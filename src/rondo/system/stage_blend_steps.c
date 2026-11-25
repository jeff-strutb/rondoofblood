/* The stage's number blends: the stock, the score added and the score, blended in a row at a time. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    STOCK_PAIR_RETURN = 0x917D, STOCK_SAVE_RETURN = 0x9154,
    ADD_SAVE_RETURN = 0x9184, ADD_PAIR_RETURN_1 = 0x9194, ADD_PAIR_RETURN_2 = 0x919B,
    SCORE_SAVE_RETURN = 0x91AF, SCORE_CHANGES_RETURN_1 = 0x91FE, SCORE_CHANGES_RETURN_2 = 0x920A,
    SCORE_CHANGES_RETURN_3 = 0x9216, ROW_ENTRIES = 5, MODE_GAME_OVER_FADE = 1, MODE_SIX = 6,
    PAUSED_STEP_BIT = 0x02, PAUSED_NEXT_BIT = 0x02, STEPS = 6, FIRST_STEP = 1, SECOND_STEP = 3, THIRD_STEP = 5,
    PAIR_STEP_A = 3, PAIR_STEP_B = 5, STEP_RESET = 5, MARK_AFTER_STOCK = 0xC0,
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
 * The stock's blend: with no blend running, starts the stock's rows (BlendStageRowPair from
 * step 0 with the stock as the byte); with one running, when the stock shown (StageVar3616)
 * differs from the stock and the game is at mode 1, marks the shown one as $C0 and stops.
 */
static BlendStepEnd blend_stock_rows(uint8_t x, uint8_t y)
{
    Sum map;
    TileRowsExit pair;
    uint8_t on;
    uint8_t shown;
    uint8_t stock;
    uint8_t mode;
    bool carry;
    bool y_set = false;
    bool overflow;

    jsr_push(STOCK_SAVE_RETURN);
    map = SaveAndMapStageBanks();
    jsr_pop();
    carry = map.carry;
    overflow = map.overflow;
    on = RD8(STAGEVARS->blend_on[1]);
    if (on == 0) {
        push8(x);
        WR8(STAGEVARS->blend_on[0], 0);
        stock = RD8(ZP->stock);
        WR8(STAGEVARS->state_3616[0], stock);
        jsr_push(STOCK_PAIR_RETURN);
        pair = BlendStageRowPair(0, stock, y);
        jsr_pop();
        x = pull8();
        carry = true;
        overflow = pair.v;
        y_set = true;
    } else {
        shown = RD8(STAGEVARS->state_3616[0]);
        stock = RD8(ZP->stock);
        carry = shown >= stock;
        if (shown != stock) {
            mode = RD8(ZP->game_sub_mode);
            carry = mode >= MODE_GAME_OVER_FADE;
            if (mode == MODE_GAME_OVER_FADE) {
                RMW8(ZP->paused, paused, paused | PAUSED_STEP_BIT);
                WR8(STAGEVARS->state_3616[1], MARK_AFTER_STOCK);
                carry = true;
            }
        }
    }
    return step_end(RestoreStageBanks(), carry, overflow, y_set);
}

/*
 * The added score's blend: with no blend running, blends the rows of the two bytes of the
 * score being added (steps 3 and 5).
 */
static BlendStepEnd blend_score_add_rows(uint8_t x, uint8_t y)
{
    Sum map;
    TileRowsExit pair;
    uint8_t on;
    uint8_t digits;
    bool carry;
    bool overflow;
    bool y_set = false;

    jsr_push(ADD_SAVE_RETURN);
    map = SaveAndMapStageBanks();
    jsr_pop();
    carry = map.carry;
    overflow = map.overflow;
    on = RD8(STAGEVARS->blend_on[1]);
    if (on == 0) {
        push8(x);
        WR8(STAGEVARS->blend_on[0], 0);
        digits = RD8(ZP->score_add.lo);
        jsr_push(ADD_PAIR_RETURN_1);
        pair = BlendStageRowPair(PAIR_STEP_A, digits, y);
        jsr_pop();
        digits = RD8(ZP->score_add.hi);
        jsr_push(ADD_PAIR_RETURN_2);
        pair = BlendStageRowPair(PAIR_STEP_B, digits, ROW_ENTRIES);
        jsr_pop();
        x = pull8();
        carry = true;
        overflow = pair.v;
        y_set = true;
    }
    return step_end(RestoreStageBanks(), carry, overflow, y_set);
}

/*
 * The score's blend: with a blend running, steps its counter and blends the three score bytes'
 * rows against the ones shown (BlendStageRowChanges at steps 1, 3 and 5) until the counter
 * reaches 6, when the shown bytes become the score's. Starts the counter at 5 when the game is
 * at mode 6 or paused with bit 1; when the shown bytes already equal the score, nothing blends.
 */
static BlendStepEnd blend_score_rows(uint8_t x)
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
    map = SaveAndMapStageBanks();
    jsr_pop();
    carry = map.carry;
    overflow = map.overflow;
    on = RD8(STAGEVARS->blend_on[1]);
    if (on == 0) {
        return step_end(RestoreStageBanks(), carry, overflow, false);
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
                return step_end(RestoreStageBanks(), true, overflow, false);
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
    return step_end(RestoreStageBanks(), carry, overflow, y_set);
}

/*
 * @routine 024F:151 BlendStageStockRows
 * @run 6B:9151
 * WHAT IT DOES: Blends the stock number into the stage: with no blend running, its rows from step 0; with one running, marks the shown stock as done when the game is at mode 1.
 * HOW: Saves and maps the stage banks, then by the blend flag either blends the stock's two rows with BlendStageRowPair or checks the stock shown against the stock; RestoreStageBanks ends it.
 * WAITS: no.
 * Entry: X kept on the stack, Y the second row of the first blend. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
 * else kept; X kept.
 */
BlendStepEnd BlendStageStockRows(uint8_t x, uint8_t y)
{
    return blend_stock_rows(x, y);
}

void abi_BlendStageStockRows(void)
{
    step_exit_shim(BlendStageStockRows(cpu.x, cpu.y));
}

/*
 * @routine 05BF:151 BlendStageStockRowsLong
 * @run 6B:9151
 * WHAT IT DOES: The same code for the other stage group, which keeps its bank routines at $9651 and $9666.
 * HOW: The same code, calling the bank routines at their other run addresses.
 * WAITS: no.
 * Entry: X kept on the stack, Y the second row of the first blend. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
 * else kept; X kept.
 */
BlendStepEnd BlendStageStockRowsLong(uint8_t x, uint8_t y)
{
    return blend_stock_rows(x, y);
}

void abi_BlendStageStockRowsLong(void)
{
    step_exit_shim(BlendStageStockRowsLong(cpu.x, cpu.y));
}

/*
 * @routine 024F:181 BlendStageScoreAddRows
 * @run 6B:9181
 * WHAT IT DOES: Blends the two bytes of the score being added into the stage (steps 3 and 5), when no blend is running.
 * HOW: Saves and maps the stage banks, and when no blend is running calls BlendStageRowPair for each of the two bytes of the added score; RestoreStageBanks ends it.
 * WAITS: no.
 * Entry: X kept on the stack, Y the second row of the first blend. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
 * else kept; X kept.
 */
BlendStepEnd BlendStageScoreAddRows(uint8_t x, uint8_t y)
{
    return blend_score_add_rows(x, y);
}

void abi_BlendStageScoreAddRows(void)
{
    step_exit_shim(BlendStageScoreAddRows(cpu.x, cpu.y));
}

/*
 * @routine 05BF:181 BlendStageScoreAddRowsLong
 * @run 6B:9181
 * WHAT IT DOES: The same code for the other stage group, which keeps its bank routines at $9651 and $9666.
 * HOW: The same code, calling the bank routines at their other run addresses.
 * WAITS: no.
 * Entry: X kept on the stack, Y the second row of the first blend. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
 * else kept; X kept.
 */
BlendStepEnd BlendStageScoreAddRowsLong(uint8_t x, uint8_t y)
{
    return blend_score_add_rows(x, y);
}

void abi_BlendStageScoreAddRowsLong(void)
{
    step_exit_shim(BlendStageScoreAddRowsLong(cpu.x, cpu.y));
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
    return blend_score_rows(x);
}

void abi_BlendStageScoreRows(void)
{
    step_exit_shim(BlendStageScoreRows(cpu.x));
}

/*
 * @routine 05BF:1AC BlendStageScoreRowsLong
 * @run 6B:91AC
 * WHAT IT DOES: The same code for the other stage group, which keeps its bank routines at $9651 and $9666.
 * HOW: The same code, calling the bank routines at their other run addresses.
 * WAITS: no.
 * Entry: X kept on the stack. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
 * else kept; X kept.
 */
BlendStepEnd BlendStageScoreRowsLong(uint8_t x)
{
    return blend_score_rows(x);
}

void abi_BlendStageScoreRowsLong(void)
{
    step_exit_shim(BlendStageScoreRowsLong(cpu.x));
}
