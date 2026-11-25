/*
 * TEMPORARY: the stage object routines of lot L8 that call routines of the same lot (overlays 025A, 025B and 0B4B), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/*
 * @routine 025B:0CA StartChannel81OnceThenStepScrollTable
 * @run 70:B0CA
 * WHAT IT DOES: When OBJ_3670 is 0 and the game's sub-mode is not 5 sets it 1 and starts list $81 on channel 5; then runs SetBossFixedFields and StepVerticalScrollTable.
 * HOW: Two tests, an INC, StartListChannelKind2 between a PHX and a PLX, a call and a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as StepVerticalScrollTable.
 */
Ended StartChannel81OnceThenStepScrollTable(uint8_t slot)
{
    uint8_t flag, mode;
    Ended e;

    flag = RD8(OBJ->f3670[slot]);
    e = ended_load(flag);
    if (flag == 0) {
        mode = RD8(ZP->game_sub_mode);
        e = then(e, ended_compare(mode, 5));
        if (mode != 5) {
            e = then(e, ended_nz(INC8(OBJ->f3670[slot])));
            e = then(e, l8_list_channel2(slot, 0x81, 5, 0xB0E0));
        }
    }
    jsr_push(0xB0E4);
    e = then(e, SetBossFixedFields());
    jsr_pop();
    return then(e, StepVerticalScrollTable(slot));
}

void abi_StartChannel81OnceThenStepScrollTable(void)
{
    abi_ended(StartChannel81OnceThenStepScrollTable(cpu.x));
}

/*
 * @routine 025B:0E7 NextStateThenStepScrollTable
 * @run 70:B0E7
 * WHAT IT DOES: Goes to the next state and runs StepVerticalScrollTable.
 * HOW: An INC and a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as StepVerticalScrollTable.
 */
Ended NextStateThenStepScrollTable(uint8_t slot)
{
    Ended e = ended_nz(INC8(OBJ->state[slot]));

    return then(e, StepVerticalScrollTable(slot));
}

void abi_NextStateThenStepScrollTable(void)
{
    abi_ended(NextStateThenStepScrollTable(cpu.x));
}

/*
 * @routine 025B:5E9 StepListMarksThenSound74AtC8
 * @run 70:B5E9
 * WHAT IT DOES: Steps the animation list at $B612, moves, runs StartScriptAtTimerMarks; when the object's X low byte is $C8 plays sound $74, clears the animation and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, a call, a compare, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
SoundEnd StepListMarksThenSound74AtC8(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, x;
    FrameStep step;
    AxisMove move;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e = l8_step_list(slot, 0xB612, 0xB5F4, false, &y, &step);

    jsr_push(0xB5F7);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    jsr_push(0xB5FA);
    e = then(e, StartScriptAtTimerMarks(slot));
    jsr_pop();
    y = e.y_set ? e.y : y;
    x = RD8(OBJ->x_lo[slot]);
    e = then(e, ended_compare(x, 0xC8));
    if (x != 0xC8) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0x74;
    r.before = then(e, ended_load(0x74));
    jsr_push(0xB608);
    r.call = SoundCommand(0x74, slot, y);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.end = ended_nz(INC8(OBJ->state[slot]));
    return r;
}

void abi_StepListMarksThenSound74AtC8(void)
{
    abi_sound_end(StepListMarksThenSound74AtC8(cpu.x, cpu.y));
}

/*
 * @routine 025B:6B1 SpinThenCountDownToState
 * @run 70:B6B1
 * WHAT IT DOES: Steps the animation list at $B6DA, sets slot 0's OBJ_3510 2 and OBJ_3568 0, runs SpinOnCircle and counts OBJ_3510 down; at zero sets OBJ_3510 $40 and OBJ_3670 2 and goes to the next state.
 * HOW: StepObjectFrames, SpinOnCircle, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended SpinThenCountDownToState(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB6DA, 0xB6BC, false, &y, &step);

    WR8(OBJ->f3510[0], 2);
    WR8(OBJ->f3568[0], 0);
    e = then(e, ended_load(2));
    jsr_push(0xB6C7);
    e = then(e, SpinOnCircle(slot));
    jsr_pop();
    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->f3510[slot], 0x40);
    WR8(OBJ->f3670[slot], 2);
    e = then(e, ended_load(2));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_SpinThenCountDownToState(void)
{
    abi_ended(SpinThenCountDownToState(cpu.x, cpu.y));
}

/*
 * @routine 025B:718 SpinThenCountDownToPhase80
 * @run 70:B718
 * WHAT IT DOES: As SpinThenCountDownToState, but at zero it sets OBJ_36C8 $80 and goes to the next state.
 * HOW: StepObjectFrames, SpinOnCircle, a memory decrement, a store, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended SpinThenCountDownToPhase80(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB6DA, 0xB723, false, &y, &step);

    WR8(OBJ->f3510[0], 2);
    WR8(OBJ->f3568[0], 0);
    e = then(e, ended_load(2));
    jsr_push(0xB72E);
    e = then(e, SpinOnCircle(slot));
    jsr_pop();
    timer = DEC8(OBJ->f3510[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->f36c8[slot], 0x80);
    e = then(e, ended_load(0x80));
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_SpinThenCountDownToPhase80(void)
{
    abi_ended(SpinThenCountDownToPhase80(cpu.x, cpu.y));
}

/*
 * @routine 025B:73C SpinThenCountPhaseDown
 * @run 70:B73C
 * WHAT IT DOES: Steps the animation list at $B6DA, runs SpinOnCircle, sets OBJ_3670 2 and counts OBJ_36C8 down; at zero clears the animation and OBJ_3670 and goes to the next state.
 * HOW: StepObjectFrames, SpinOnCircle, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended SpinThenCountPhaseDown(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, timer;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB6DA, 0xB747, false, &y, &step);

    jsr_push(0xB74A);
    e = then(e, SpinOnCircle(slot));
    jsr_pop();
    WR8(OBJ->f3670[slot], 2);
    e = then(e, ended_load(2));
    timer = DEC8(OBJ->f36c8[slot]);
    e = then(e, ended_nz(timer));
    if (timer != 0) {
        return e;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3670[slot], 0);
    return then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_SpinThenCountPhaseDown(void)
{
    abi_ended(SpinThenCountPhaseDown(cpu.x, cpu.y));
}

/*
 * @routine 025B:7C7 SpinThenCountWordDown
 * @run 70:B7C7
 * WHAT IT DOES: Steps the animation list at $B6DA, runs SpinOnCircle and counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero clears OBJ_3250, goes to the next state and clears OBJ_2800.
 * HOW: StepObjectFrames, SpinOnCircle, a 16-bit subtract in memory, an ORA, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
Ended SpinThenCountWordDown(uint8_t slot, uint8_t y_entry)
{
    uint8_t y = y_entry, low, high, again, joined;
    FrameStep step;
    Sum diff;
    Ended e = l8_step_list(slot, 0xB6DA, 0xB7D2, false, &y, &step);

    jsr_push(0xB7D5);
    e = then(e, SpinOnCircle(slot));
    jsr_pop();
    low = RD8(OBJ->f3510[slot]);
    diff = sub_bytes(low, 1, true);
    WR8(OBJ->f3510[slot], diff.value);
    high = RD8(OBJ->f34b8[slot]);
    diff = sub_bytes(high, 0, diff.carry);
    WR8(OBJ->f34b8[slot], diff.value);
    again = RD8(OBJ->f3510[slot]);
    joined = (uint8_t)(diff.value | again);
    e = then(then(e, ended_of_sum(diff)), ended_load(joined));
    if (joined != 0) {
        return e;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    e = then(e, ended_nz(INC8(OBJ->state[slot])));
    WR8(OBJ->f2800[slot], 0);
    return e;
}

void abi_SpinThenCountWordDown(void)
{
    abi_ended(SpinThenCountWordDown(cpu.x, cpu.y));
}

/*
 * @routine 0B4B:690 CountDownThenDrawAndSpawnRing
 * @run 70:B690
 * WHAT IT DOES: When the scroll's screen byte is 2, counts OBJ_34B8 down; at zero draws tile script 0, starts the ring of pieces (StartRingOfPieces), plays sound $56, sets the scroll's X limit to $0280 and OBJ_34B8 $60 and goes to the next state.
 * HOW: A compare, a memory decrement, DrawTileScript, StartRingOfPieces, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
ExitChain CountDownThenDrawAndSpawnRing(uint8_t slot, uint8_t y_entry)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    uint8_t screen, timer;
    TileRun run;
    Ended e, ring;
    SoundEnd r = l8_sound_none(ended_kept());

    screen = RD8(SCROLLE->x_hi_screen[0]);
    e = ended_compare(screen, 2);
    if (screen != 2) {
        chain_ended(&c, e);
        return c;
    }
    timer = DEC8(OBJ->f34b8[slot]);
    e = then(e, ended_nz(timer));
    chain_ended(&c, e);
    if (timer != 0) {
        return c;
    }
    jsr_push(0xB6A2);
    run = DrawTileScript(0, 0);
    jsr_pop();
    chain_ended(&c, ended_of_tile_run(run, 0));
    jsr_push(0xB6A5);
    ring = StartRingOfPieces(slot);
    jsr_pop();
    chain_ended(&c, ring);
    r.sounded = true;
    r.command = 0x56;
    r.before = ended_load(0x56);
    jsr_push(0xB6AA);
    r.call = SoundCommand(0x56, slot, ring.y_set ? ring.y : y_entry);
    jsr_pop();
    wr8(ScrollXMaxHi, 2);
    wr8(ScrollXMaxLo, 0x80);
    WR8(OBJ->f34b8[slot], 0x60);
    r.end = then(ended_load(0x60), ended_nz(INC8(OBJ->state[slot])));
    chain_sound(&c, r);
    return c;
}

void abi_CountDownThenDrawAndSpawnRing(void)
{
    ExitChain c = CountDownThenDrawAndSpawnRing(cpu.x, cpu.y);

    abi_chain(&c);
}
