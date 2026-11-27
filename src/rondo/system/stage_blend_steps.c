/* The stage's number blends: the stock, the score added and the score, blended in a row at a time. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"
#include "stage_enemies_l7.h"

enum {
    SCORE_SAVE_RETURN = 0x91AF, SCORE_CHANGES_RETURN_1 = 0x91FE, SCORE_CHANGES_RETURN_2 = 0x920A,
    SCORE_CHANGES_RETURN_3 = 0x9216, ROW_ENTRIES = 5, MODE_SIX = 6,
    PAUSED_NEXT_BIT = 0x02, STEPS = 6, FIRST_STEP = 1, SECOND_STEP = 3, THIRD_STEP = 5, STEP_RESET = 5,
    SCORE_COPY_LO = 0x371E, SCORE_COPY_MID = 0x3776, SCORE_COPY_HI = 0x37CE
};

/* What a step leaves: A the restored bank, C and V as the instructions left them, Y 5 once a blend ran. */
static BlendStepEnd step_end(uint8_t bank, bool carry, bool overflow, bool y_set)
{
    BlendStepEnd end;

    end.a = bank;
    end.c = carry;
    end.v = overflow;
    end.y_set = y_set;
    return end;
}

static void step_exit_shim(BlendStepEnd end)
{
    cpu.a = end.a;
    abi_nz(end.a);
    abi_flag(P_C, end.c);
    abi_flag(P_V, end.v);
    if (end.y_set) {
        cpu.y = ROW_ENTRIES;
    }
}

/*
 * The score's blend: with a blend running, steps its counter and blends the three score bytes'
 * rows against the ones shown (BlendStageRowChanges at steps 1, 3 and 5) until the counter
 * reaches 6, when the shown bytes become the score's. Starts the counter at 5 when the game is
 * at mode 6 or paused with bit 1; when the shown bytes already equal the score, nothing blends.
 */
static BlendStepEnd blend_score_rows(uint8_t x, const StageBanks *banks)
{
    Sum map;
    RowChangesExit changes;
    uint8_t on;
    uint8_t step;
    uint8_t mode;
    uint8_t shown;
    uint8_t score;
    bool carry;
    bool overflow;
    bool y_set = false;
    bool blend = false;

    jsr_push(SCORE_SAVE_RETURN);
    map = banks->save_and_map();
    jsr_pop();
    carry = map.carry;
    overflow = map.overflow;
    on = RD8(STAGEVARS->blend_on[1]);
    if (on == 0) {
        return step_end(banks->restore(), carry, overflow, false);
    }
    WR8(STAGEVARS->blend_on[0], on);
    step = RD8(STAGEVARS->blend_step[0]);
    if (step == 0) {
        mode = RD8(ZP->game_sub_mode);
        if (mode == MODE_SIX || (RD8(ZP->paused) & PAUSED_NEXT_BIT) != 0) {
            WR8(STAGEVARS->blend_step[0], STEP_RESET);
        }
        shown = RD8(STAGEVARS->state_350e[0]);
        score = RD8(ZP->score.lo);
        blend = shown != score;
        if (!blend) {
            shown = RD8(STAGEVARS->state_3566[0]);
            score = RD8(ZP->score.mid);
            blend = shown != score;
        }
        if (!blend) {
            shown = RD8(STAGEVARS->state_35be[0]);
            score = RD8(ZP->score.hi);
            if (shown == score) {
                return step_end(banks->restore(), true, overflow, false);
            }
            INC8(STAGEVARS->blend_step[0]);
            score = RD8(ZP->score.lo);
            wr8(SCORE_COPY_LO, score);
            score = RD8(ZP->score.mid);
            wr8(SCORE_COPY_MID, score);
            score = RD8(ZP->score.hi);
            wr8(SCORE_COPY_HI, score);
        }
    }
    push8(x);
    shown = RD8(STAGEVARS->state_350e[0]);
    WR8(ZP->bios_a.lo, shown);
    score = RD8(ZP->score.lo);
    jsr_push(SCORE_CHANGES_RETURN_1);
    changes = BlendStageRowChanges(FIRST_STEP, score);
    jsr_pop();
    y_set = y_set || changes.y_set;
    if (changes.v_set) {
        overflow = changes.v;
    }
    shown = RD8(STAGEVARS->state_3566[0]);
    WR8(ZP->bios_a.lo, shown);
    score = RD8(ZP->score.mid);
    jsr_push(SCORE_CHANGES_RETURN_2);
    changes = BlendStageRowChanges(SECOND_STEP, score);
    jsr_pop();
    y_set = y_set || changes.y_set;
    if (changes.v_set) {
        overflow = changes.v;
    }
    shown = RD8(STAGEVARS->state_35be[0]);
    WR8(ZP->bios_a.lo, shown);
    score = RD8(ZP->score.hi);
    jsr_push(SCORE_CHANGES_RETURN_3);
    changes = BlendStageRowChanges(THIRD_STEP, score);
    jsr_pop();
    y_set = y_set || changes.y_set;
    if (changes.v_set) {
        overflow = changes.v;
    }
    x = pull8();
    step = INC8(STAGEVARS->blend_step[0]);
    step = RD8(STAGEVARS->blend_step[0]);
    carry = step >= STEPS;
    if (step == STEPS) {
        WR8(STAGEVARS->blend_step[0], 0);
        score = RD8(ZP->score.lo);
        WR8(STAGEVARS->state_350e[0], score);
        score = RD8(ZP->score.mid);
        WR8(STAGEVARS->state_3566[0], score);
        score = RD8(ZP->score.hi);
        WR8(STAGEVARS->state_35be[0], score);
    }
    return step_end(banks->restore(), carry, overflow, y_set);
}

/*
 * @routine 024F:151 BlendStageStockRows
 * @run 6B:9151
 * WHAT IT DOES: Blends the stock number into the stage: with no blend running, its rows from step 0; with one running, marks the shown stock as done when the game is at mode 1.
 * HOW: Saves and maps the stage banks, then by the blend flag either blends the stock's two rows with BlendStageRowPair or checks the stock shown against the stock; RestoreStageBanks ends it.
 * The same instructions as 05BF's group's BlendStockRows, calling the bank routines at $933F and $9354: its body,
 * blend_stock_rows_with (stock_display.c), with this group's bank routines.
 * WAITS: no.
 * Entry: X kept on the stack, Y the second row of the first blend. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
 * else kept; X kept.
 */
ObjArm BlendStageStockRows(uint8_t slot, uint8_t y)
{
    return blend_stock_rows_with(slot, y, &stage_banks_024f);
}

void abi_BlendStageStockRows(void)
{
    abi_ended(blend_stock_rows_exit_shim(BlendStageStockRows(cpu.x, cpu.y)));
}

/*
 * @routine 024F:181 BlendStageScoreAddRows
 * @run 6B:9181
 * WHAT IT DOES: Blends the two bytes of the score being added into the stage (steps 3 and 5), when no blend is running.
 * HOW: Saves and maps the stage banks, and when no blend is running calls BlendStageRowPair for each of the two bytes of the added score; RestoreStageBanks ends it.
 * The same instructions as 05BF's group's BlendStageRowsOfPlayer, calling the bank routines at $933F and $9354: its body,
 * blend_rows_of_player_with (stage_blend_long.c), with this group's bank routines.
 * WAITS: no.
 * Entry: X kept on the stack, Y the second row of the first blend. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
 * else kept; X kept.
 */
ObjArm BlendStageScoreAddRows(uint8_t slot, uint8_t y)
{
    return blend_rows_of_player_with(slot, y, &stage_banks_024f);
}

void abi_BlendStageScoreAddRows(void)
{
    abi_ended(blend_stage_rows_of_player_exit_shim(BlendStageScoreAddRows(cpu.x, cpu.y)));
}

/*
 * @routine 024F:1AC BlendStageScoreRows
 * @run 6B:91AC
 * WHAT IT DOES: Steps the score blend: with one running, blends the three score bytes' rows against the ones shown until six steps are made, when the shown bytes become the score's.
 * HOW: Saves and maps the stage banks, steps the blend counter and calls BlendStageRowChanges for each of the three score bytes against the bytes shown; RestoreStageBanks ends it.
 * WAITS: no.
 * Entry: X kept on the stack. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
 * else kept; X kept.
 */
BlendStepEnd BlendStageScoreRows(uint8_t x)
{
    return blend_score_rows(x, &stage_banks_024f);
}

void abi_BlendStageScoreRows(void)
{
    step_exit_shim(BlendStageScoreRows(cpu.x));
}

/*
 * @routine 05BF:1AC BlendStageScoreRowsLong
 * @run 6B:91AC
 * WHAT IT DOES: BlendStageScoreRows in 05BF's group of stage overlays, which keeps its bank routines at $9651 and $9666.
 * HOW: The same instructions, calling SaveAndMapStageBanksLong and RestoreStageBanksLong.
 * WAITS: no.
 * Entry: X kept on the stack. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
 * else kept; X kept.
 */
BlendStepEnd BlendStageScoreRowsLong(uint8_t x)
{
    return blend_score_rows(x, &stage_banks_05bf);
}

void abi_BlendStageScoreRowsLong(void)
{
    step_exit_shim(BlendStageScoreRowsLong(cpu.x));
}
