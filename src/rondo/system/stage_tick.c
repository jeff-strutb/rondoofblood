/* The stage's frame tick: the opening sequence or the number blends, the health bar and the shown bytes. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    FRAME_SLOT = 86, FRAME_SLOT_NEXT = 87, PAUSED_BIT1 = 0x02, CHARACTER_STAGE_A = 0x0D, CHARACTER_STAGE_B = 0x11,
    FRAME_FOR_A = 0x72, FRAME_FOR_B = 0x71, Y_LO_DEFAULT = 0x24, Y_LO_FOR_B = 0x28, ROW_ENTRIES = 5,
    FLAG_HEALTH_SHOWN = 0x20, FLAG_SOUND_DONE = 0x40, FLAG_TAIL = 0x04, SOUND_HEALTH_UP = 0x0F, SOUND_HEALTH_DOWN = 0x54,
    SEQUENCE_RETURN = 0x8B84, STOCK_RETURN = 0x8B89, ADD_RETURN = 0x8B8C, SCORE_RETURN = 0x8B8F, WAVE_RETURN = 0x8B92,
    BAR_DOWN_RETURN = 0x8BC1, AND_RETURN = 0x8BC8, BAR_UP_RETURN = 0x8BDD, OR_RETURN = 0x8BE4, SOUND_UP_RETURN = 0x8BD7,
    SOUND_DOWN_RETURN = 0x8BF2, MAP_RETURN = 0x8BF5, BLEND_RETURN_1 = 0x8C07, BLEND_RETURN_2 = 0x8C12, BAR_RETURN = 0x8C6B,
    WAVE_SCRIPTS = 0x8C3B, FRAME_TABLE = 0x8C19, TAIL_RUN = 0x8C74, SCRIPT_SLOT = 1, BAR_FULL = 0x5E, BAR_BASE_HI = 0x65,
    SOUND_HOOK_RUN = 0x401B
};

/* The sound command the frame sends: A the command (N and Z from its load), X and Y as the code holds them, C and V as the
 * last instructions to set them left them; what Y is after it. */
static uint8_t sound_command_shim(uint8_t command, uint8_t x, uint8_t y, bool c, bool v)
{
    cpu.a = command;
    cpu.x = x;
    cpu.y = y;
    abi_nz(command);
    abi_flag(P_C, c);
    abi_flag(P_V, v);
    call_original(SOUND_HOOK_RUN);
    return cpu.y;
}

/* The jump into the code that follows the frame's end, which has no C: it gets the registers and flags and returns for this routine. */
static StageExit original_tail_shim(StageExit end, uint16_t tail_run)
{
    StageExit done = { 0, 0, 0, false, false, 0, EXIT_CPU };

    cpu.a = end.a;
    cpu.x = end.x;
    cpu.y = end.y;
    abi_nz(end.nz);
    abi_flag(P_C, end.c);
    abi_flag(P_V, end.v);
    call_original(tail_run);
    return done;
}

/*
 * @routine 024E:422 StageScriptByWave
 * @run 6B:8C22
 * WHAT IT DOES: Starts the stage's script in slot 1 that the wave counter names, when it is not
 * the one already shown.
 * HOW: The counter's low three bits, masked with RichterItemReady, index a table of eight script
 * numbers at $8C3B; when the one found differs from StageVar3566's second byte, it goes there and
 * StartScriptSlotKind4 starts it in slot 1.
 * Every stage overlay holds the same code here.
 * WAITS: no.
 * Entry: nothing. Exit: unchanged script: A the script, X the table index, Z 1, C 1;
 * else as StartScriptSlotKind4 with X 1, C as the compare left it.
 */
StageExit StageScriptByWave(void)
{
    StageExit end = { 0, 0, 0, true, false, 0, EXIT_X | EXIT_C };
    uint8_t wave = RD8(ZP->wave_counter);
    uint8_t x = (uint8_t)(wave & 0x07);
    uint8_t ready;
    uint8_t script;
    uint8_t shown;

    ready = rd8(RichterItemReady);
    x = (uint8_t)(x & ready);
    script = rd8((uint16_t)(WAVE_SCRIPTS + x));
    shown = RD8(STAGEVARS->state_3566[1]);
    end.a = script;
    end.x = x;
    if (script == shown) {
        return end;
    }
    WR8(STAGEVARS->state_3566[1], script);
    end.a = StartScriptSlotKind4(script, SCRIPT_SLOT);
    end.nz = end.a;
    end.x = SCRIPT_SLOT;
    end.y = end.a;
    end.c = script >= shown;
    end.set = EXIT_X | EXIT_Y | EXIT_C;
    return end;
}

void abi_StageScriptByWave(void)
{
    stage_exit_shim(StageScriptByWave());
}

/* Points the VDC at the health bar's cell: the bar's position from the health shown, in two words. */
static StageExit set_health_bar_address(SetBarAddress set)
{
    StageExit end = { 0, 0, 0, false, false, 0, 0 };
    uint8_t shown = RD8(ZP->field99);
    uint8_t value;
    uint8_t high;
    uint8_t low;
    bool carry;
    Sum lo_sum;
    Sum hi_sum;

    if (shown == 0) {
        return end;
    }
    shown = RD8(ZP->field99);
    value = (uint8_t)(BAR_FULL - shown);
    push8(value);
    low = (uint8_t)(value & 0x0F);
    WR8(ZP->temp00, low);
    value = pull8();
    value = (uint8_t)(value & 0xF0);
    WR8(ZP->temp01, 0);
    carry = (value & 0x80) != 0;
    value = (uint8_t)(value << 1);
    ROL8(ZP->temp01, carry);
    carry = (value & 0x80) != 0;
    value = (uint8_t)(value << 1);
    ROL8(ZP->temp01, carry);
    carry = (value & 0x80) != 0;
    value = (uint8_t)(value << 1);
    ROL8(ZP->temp01, carry);
    low = RD8(ZP->temp00);
    lo_sum = add_bytes(value, low, false);
    high = RD8(ZP->temp01);
    hi_sum = add_bytes(high, BAR_BASE_HI, lo_sum.carry);
    jsr_push(BAR_RETURN);
    set(lo_sum.value, hi_sum.value);
    jsr_pop();
    end.a = 2;
    end.nz = 2;
    end.x = lo_sum.value;
    end.y = hi_sum.value;
    end.c = hi_sum.carry;
    end.v = hi_sum.overflow;
    end.set = EXIT_X | EXIT_Y | EXIT_C | EXIT_V;
    return end;
}

/*
 * @routine 024E:443 SetHealthBarAddress
 * @run 6B:8C43
 * WHAT IT DOES: Points the VDC's VRAM addresses at the health bar's cell for the health shown
 * (zField99); nothing when none is shown.
 * HOW: $5E minus the health shown gives a cell number; its high nibble times eight plus its low
 * nibble is the offset, and the high byte is $65 plus the carry; SetVramAddresses does the rest.
 * Eleven stage overlays hold this code, the ones whose VRAM routine is at $937D.
 * WAITS: no.
 * Entry: nothing. Exit: A 2, X and Y the address, N 0, Z 0, C and V from the high byte's add;
 * with none shown A 0 and the rest kept.
 */
StageExit SetHealthBarAddress(void)
{
    return set_health_bar_address(SetVramAddresses);
}

void abi_SetHealthBarAddress(void)
{
    stage_exit_shim(SetHealthBarAddress());
}

/*
 * @routine 09DE:443 SetHealthBarAddressLong
 * @run 6B:8C43
 * WHAT IT DOES: SetHealthBarAddress for the other stage group, whose VRAM routine is at $968F.
 * HOW: The same code, calling SetVramAddressesLong.
 * WAITS: no.
 * Entry, exit: as SetHealthBarAddress.
 */
StageExit SetHealthBarAddressLong(void)
{
    return set_health_bar_address(SetVramAddressesLong);
}

void abi_SetHealthBarAddressLong(void)
{
    stage_exit_shim(SetHealthBarAddressLong());
}

static void apply_exit(StageExit end, uint8_t *x, uint8_t *y)
{
    if ((end.set & EXIT_X) != 0) {
        *x = end.x;
    }
    if ((end.set & EXIT_Y) != 0) {
        *y = end.y;
    }
}

static void apply_step(BlendStepEnd step, uint8_t *y)
{
    if (step.y_set) {
        *y = ROW_ENTRIES;
    }
}

/*
 * One tick of the stage: the sequence or the blends, the wave script, the frames, the health bar, the banks and the shown bytes.
 * v is the overflow the routine was entered with; the sound command is called with the carry and overflow the code holds there.
 */
StageExit StepStageFrameBody(uint8_t x, uint8_t y, bool v, SetBarAddress set_bar, const StepFrameLayout *layout)
{
    StageExit end = { 0, 0, 0, false, false, 0, 0 };
    StageExit other;
    StageExit bar;
    BlendStepEnd step;
    TileRowsExit blend;
    Sum map;
    bool sequence;
    bool check_sound;
    bool carry;
    uint8_t a;
    uint8_t health;
    uint8_t shown;
    uint8_t flags;

    a = RD8(OBJ->frame[FRAME_SLOT_NEXT]);
    if (a == 0) {
        return end;
    }
    if ((RD8(ZP->paused) & PAUSED_BIT1) != 0) {
        sequence = true;
    } else {
        a = RD8(STAGEVARS->state_3616[1]);
        sequence = false;
        if (a != 0) {
            a = DEC8(STAGEVARS->state_3616[1]);
            if (a == 0) {
                RMW8(ZP->paused, paused, paused | PAUSED_BIT1);
                sequence = true;
            }
        }
    }
    if (sequence) {
        jsr_push(layout->sequence_return);
        other = StepStageSequence(x, y);
        jsr_pop();
        apply_exit(other, &x, &y);
        if ((other.set & EXIT_V) != 0) {
            v = other.v;
        }
    } else {
        jsr_push(layout->stock_return);
        step = blend_stock_step_loaded(x, y);
        jsr_pop();
        apply_step(step, &y);
        jsr_push(layout->add_return);
        step = blend_score_add_step_loaded(x, y);
        jsr_pop();
        apply_step(step, &y);
        jsr_push(layout->score_return);
        step = blend_score_rows_loaded(x);
        jsr_pop();
        apply_step(step, &y);
        v = step.v;
    }
    jsr_push(layout->wave_return);
    other = StageScriptByWave();
    jsr_pop();
    apply_exit(other, &x, &y);
    WR8(OBJ->y_lo[FRAME_SLOT], Y_LO_DEFAULT);
    x = RD8(ZP->field93);
    a = rd8((uint16_t)(layout->frame_table + x));
    x = RD8(ZP->character);
    if (x != 0) {
        if (a == CHARACTER_STAGE_A) {
            a = FRAME_FOR_A;
        } else if (a == CHARACTER_STAGE_B) {
            x = Y_LO_FOR_B;
            WR8(OBJ->y_lo[FRAME_SLOT], x);
            a = FRAME_FOR_B;
        }
    }
    WR8(OBJ->frame[FRAME_SLOT], a);
    health = RD8(ZP->player_health);
    shown = RD8(ZP->field99);
    check_sound = true;
    carry = true;
    if (health == shown) {
        RMW8(ZP->field8e, flags, flags | FLAG_HEALTH_SHOWN);
    } else if (health < shown) {
        jsr_push(layout->bar_down_return);
        bar = set_health_bar_address(set_bar);
        jsr_pop();
        carry = bar.c;
        v = bar.v;
        x = 0xFF;
        y = 0xF0;
        jsr_push(layout->and_return);
        and_vram_data_loaded(x, y);
        jsr_pop();
        DEC8(ZP->field99);
    } else {
        check_sound = false;
        if ((RD8(ZP->field8e) & FLAG_HEALTH_SHOWN) != 0 && (RD8(ZP->field8e) & FLAG_SOUND_DONE) == 0) {
            jsr_push(layout->sound_up_return);
            y = sound_command_shim(layout->sound_up, x, y, true, v);
            jsr_pop();
            RMW8(ZP->field8e, flags, flags | FLAG_SOUND_DONE);
        }
        INC8(ZP->field99);
        jsr_push(layout->bar_up_return);
        (void)set_health_bar_address(set_bar);
        jsr_pop();
        x = 0;
        y = 0x0F;
        jsr_push(layout->or_return);
        or_vram_data_loaded(x, y);
        jsr_pop();
    }
    if (check_sound && (RD8(ZP->field8e) & FLAG_SOUND_DONE) != 0) {
        RMW8(ZP->field8e, flags, flags & ~FLAG_SOUND_DONE);
        jsr_push(layout->sound_down_return);
        y = sound_command_shim(layout->sound_down, x, y, carry, v);
        jsr_pop();
    }
    jsr_push(layout->map_return);
    map = map_stage_banks_loaded();
    jsr_pop();
    WR8(STAGEVARS->blend_on[0], 0);
    a = RD8(ZP->field94);
    shown = RD8(ZP->field95);
    end.a = a;
    end.x = x;
    end.y = y;
    end.c = true;
    end.v = map.overflow;
    end.nz = 0;
    end.set = EXIT_X | EXIT_Y | EXIT_C | EXIT_V;
    if (a != shown) {
        WR8(ZP->field95, a);
        jsr_push(layout->blend_return_1);
        blend = BlendStageTileRows(8, (uint8_t)(a & 0x0F), y);
        jsr_pop();
        a = RD8(ZP->field94);
        a = (uint8_t)(a >> 4);
        jsr_push(layout->blend_return_2);
        blend = BlendStageTileRows(9, a, ROW_ENTRIES);
        jsr_pop();
        end.a = blend.a;
        end.x = blend.x;
        end.y = ROW_ENTRIES;
        end.v = blend.v;
    }
    if ((RD8(ZP->field8e) & FLAG_TAIL) != 0) {
        return original_tail_shim(end, layout->tail_run);
    }
    return end;
}

/* 024E's layout of the frame step: the returns of its calls, the frame table, and the code its tail jumps on to. */
static const StepFrameLayout step_frame_layout_024e = {
    SEQUENCE_RETURN, STOCK_RETURN, ADD_RETURN, SCORE_RETURN, WAVE_RETURN, BAR_DOWN_RETURN, AND_RETURN, BAR_UP_RETURN, OR_RETURN,
    SOUND_UP_RETURN, SOUND_DOWN_RETURN, MAP_RETURN, BLEND_RETURN_1, BLEND_RETURN_2, FRAME_TABLE, TAIL_RUN, SOUND_HEALTH_UP, SOUND_HEALTH_DOWN
};

static StageExit step_stage_frame(uint8_t x, uint8_t y, bool v, SetBarAddress set_bar)
{
    return StepStageFrameBody(x, y, v, set_bar, &step_frame_layout_024e);
}

/*
 * @routine 024E:36D StepStageFrame
 * @run 6B:8B6D
 * @stop 8C19
 * WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening
 * sequence (StepStageSequence) or the three number blends, starts the wave's script, picks the
 * frame the character shows, moves the health bar one step towards the health (with a sound each
 * way), maps the stage banks and blends the two rows of the shown byte zField94 when it changed.
 * HOW: The frame object is slot 87 of the frame column. The sequence runs when paused bit 1 is
 * set or the count in StageVar3617 runs out; the health bar by SetHealthBarAddress with
 * AndVramData or OrVramData; zField94 against zField95 by BlendStageTileRows at steps 8 and 9.
 * With zField8E bit 2 set the code runs on into the next piece of the overlay, which has no C.
 * Eleven stage overlays hold this code, the ones whose VRAM routines are at $9392 and $93A1.
 * WAITS: no.
 * Entry: X and Y as the blends take them; V as the caller left it, which the health sounds can be
 * called with. Exit: as the last step left it.
 */
StageExit StepStageFrame(uint8_t x, uint8_t y, bool v)
{
    return step_stage_frame(x, y, v, SetVramAddresses);
}

void abi_StepStageFrame(void)
{
    stage_exit_shim(StepStageFrame(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}

/*
 * @routine 05BE:36D StepStageFrameLong
 * @run 6B:8B6D
 * @stop 8C19
 * WHAT IT DOES: StepStageFrame for the other stage group, whose VRAM routines are at $96A4 and
 * $96B3 and whose bank mapping is at $965B.
 * HOW: The same body as StepStageFrame with SetVramAddressesLong for the health bar; its blends at $9151, $9181
 * and $91AC, the VRAM masks and the bank mapping go to the copies loaded there (stage_copies.c), which are 05BF's
 * group's wherever these bytes are.
 * WAITS: no.
 * Entry, exit: as StepStageFrame.
 */
StageExit StepStageFrameLong(uint8_t x, uint8_t y, bool v)
{
    return step_stage_frame(x, y, v, SetVramAddressesLong);
}

void abi_StepStageFrameLong(void)
{
    stage_exit_shim(StepStageFrameLong(cpu.x, cpu.y, (cpu.p & P_V) != 0));
}
