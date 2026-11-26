/*
 * TEMPORARY: the stage object routines of lot L8 that call routines of the same lot (overlays 025A, 025B and 0B4B), by routine. Bodies return the arm
 * they ended on and the values behind it; an exit shim per routine builds how the routine's last instructions left A, Y and the flags (Ended,
 * enemy_objects.h) and the abi_ wrappers show it.
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

/* The arms the routines end on. */
enum { ST24_START_FLAG, ST24_START_MODE, ST24_START_STARTED };
enum { ST24_MARKS_NONE, ST24_MARKS_SOUND };
enum { ST24_SPIN_COUNTING, ST24_SPIN_DONE };
enum { ST24_RING_OTHER_SCREEN, ST24_RING_COUNTING, ST24_RING_STARTED };

/*
 * @routine 025B:0CA StartChannel81OnceThenStepScrollTable
 * @run 70:B0CA
 * WHAT IT DOES: When OBJ_3670 is 0 and the game's sub-mode is not 5 sets it 1 and starts list $81 on channel 5; then runs SetBossFixedFields and StepVerticalScrollTable.
 * HOW: Two tests, an INC, StartListChannelKind2 between a PHX and a PLX, a call and a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as StepVerticalScrollTable (start_channel81_exit_shim).
 */
L8St24StartArm StartChannel81OnceThenStepScrollTable(uint8_t slot)
{
    L8St24StartArm r = { 0 };

    r.flag = RD8(OBJ->f3670[slot]);
    r.arm = ST24_START_FLAG;
    if (r.flag == 0) {
        r.mode = RD8(ZP->game_sub_mode);
        r.arm = ST24_START_MODE;
        if (r.mode != 5) {
            r.counted = INC8(OBJ->f3670[slot]);
            r.x = l8_channel2(slot, 0x81, 5, 0xB0E0);
            r.arm = ST24_START_STARTED;
        }
    }
    jsr_push(0xB0E4);
    r.fixed = SetBossFixedFields();
    jsr_pop();
    r.table = StepVerticalScrollTable(slot);
    return r;
}

/* What StartChannel81OnceThenStepScrollTable leaves in A, Y and the flags, from the arm it ended on. */
static Ended start_channel81_exit_shim(L8St24StartArm r)
{
    Ended e = ended_load(r.flag);

    if (r.arm != ST24_START_FLAG) {
        e = then(e, ended_compare(r.mode, 5));
    }
    if (r.arm == ST24_START_STARTED) {
        e = then(e, ended_nz(r.counted));
        e = then(e, l8_channel2_ended(r.x));
    }
    e = then(e, l8_boss_fixed_exit_shim(r.fixed));
    return then(e, l8_vertical_scroll_table_exit_shim(r.table));
}

void abi_StartChannel81OnceThenStepScrollTable(void)
{
    abi_ended(start_channel81_exit_shim(StartChannel81OnceThenStepScrollTable(cpu.x)));
}

/*
 * @routine 025B:0E7 NextStateThenStepScrollTable
 * @run 70:B0E7
 * WHAT IT DOES: Goes to the next state and runs StepVerticalScrollTable.
 * HOW: An INC and a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as StepVerticalScrollTable (next_state_exit_shim).
 */
L8St24NextArm NextStateThenStepScrollTable(uint8_t slot)
{
    L8St24NextArm r = { 0 };

    r.state = INC8(OBJ->state[slot]);
    r.table = StepVerticalScrollTable(slot);
    return r;
}

/* What NextStateThenStepScrollTable leaves in A, Y and the flags. */
static Ended next_state_exit_shim(L8St24NextArm r)
{
    return then(ended_nz(r.state), l8_vertical_scroll_table_exit_shim(r.table));
}

void abi_NextStateThenStepScrollTable(void)
{
    abi_ended(next_state_exit_shim(NextStateThenStepScrollTable(cpu.x)));
}

/* What the step, the move and the marks of StepListMarksThenSound74AtC8 leave before the X compare. */
static Ended marks_before(const L8St24MarksArm *r)
{
    Ended e = then(l8_step_ended(r->step), ended_of_axis_move(r->move));

    return then(e, l8_timer_marks_exit_shim(r->marks));
}

/* The Y the sound call takes: the one the marks or the move left, else the one the list step left. */
static uint8_t marks_y(const L8St24MarksArm *r)
{
    Ended e = marks_before(r);

    return e.y_set ? e.y : r->step.y;
}

/*
 * @routine 025B:5E9 StepListMarksThenSound74AtC8
 * @run 70:B5E9
 * WHAT IT DOES: Steps the animation list at $B612, moves, runs StartScriptAtTimerMarks; when the object's X low byte is $C8 plays sound $74, clears the animation and goes to the next state.
 * HOW: StepObjectFrames, ApplyVelocity, a call, a compare, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (marks_exit_shim).
 */
L8St24MarksArm StepListMarksThenSound74AtC8(uint8_t slot, uint8_t y_entry)
{
    L8St24MarksArm r = { 0 };

    r.step = l8_list_step(slot, 0xB612, 0xB5F4, false, y_entry);
    jsr_push(0xB5F7);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xB5FA);
    r.marks = StartScriptAtTimerMarks(slot);
    jsr_pop();
    r.x = RD8(OBJ->x_lo[slot]);
    if (r.x != 0xC8) {
        r.arm = ST24_MARKS_NONE;
        return r;
    }
    jsr_push(0xB608);
    r.call = SoundCommand(0x74, slot, marks_y(&r));
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.state = INC8(OBJ->state[slot]);
    r.arm = ST24_MARKS_SOUND;
    return r;
}

/* What StepListMarksThenSound74AtC8 leaves in A, Y and the flags and the sound call it made, from the arm it ended on. */
static SoundEnd marks_exit_shim(L8St24MarksArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = then(marks_before(&r), ended_compare(r.x, 0xC8));

    if (r.arm == ST24_MARKS_NONE) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0x74;
    s.before = then(e, ended_load(0x74));
    s.call = r.call;
    s.end = ended_nz(r.state);
    return s;
}

void abi_StepListMarksThenSound74AtC8(void)
{
    abi_sound_end(marks_exit_shim(StepListMarksThenSound74AtC8(cpu.x, cpu.y)));
}

/*
 * @routine 025B:6B1 SpinThenCountDownToState
 * @run 70:B6B1
 * WHAT IT DOES: Steps the animation list at $B6DA, sets slot 0's OBJ_3510 2 and OBJ_3568 0, runs SpinOnCircle and counts OBJ_3510 down; at zero sets OBJ_3510 $40 and OBJ_3670 2 and goes to the next state.
 * HOW: StepObjectFrames, SpinOnCircle, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (spin_state_exit_shim).
 */
L8St24SpinArm SpinThenCountDownToState(uint8_t slot, uint8_t y_entry)
{
    L8St24SpinArm r = { 0 };

    r.step = l8_list_step(slot, 0xB6DA, 0xB6BC, false, y_entry);
    WR8(OBJ->f3510[0], 2);
    WR8(OBJ->f3568[0], 0);
    jsr_push(0xB6C7);
    r.spin = SpinOnCircle(slot);
    jsr_pop();
    r.timer = DEC8(OBJ->f3510[slot]);
    if (r.timer != 0) {
        r.arm = ST24_SPIN_COUNTING;
        return r;
    }
    WR8(OBJ->f3510[slot], 0x40);
    WR8(OBJ->f3670[slot], 2);
    r.state = INC8(OBJ->state[slot]);
    r.arm = ST24_SPIN_DONE;
    return r;
}

/* What SpinThenCountDownToState leaves in A, Y and the flags, from the arm it ended on. */
static Ended spin_state_exit_shim(L8St24SpinArm r)
{
    Ended e = then(l8_step_ended(r.step), ended_load(2));

    e = then(e, l8_spin_on_circle_exit_shim(r.spin));
    e = then(e, ended_nz(r.timer));
    if (r.arm == ST24_SPIN_COUNTING) {
        return e;
    }
    e = then(e, ended_load(2));
    return then(e, ended_nz(r.state));
}

void abi_SpinThenCountDownToState(void)
{
    abi_ended(spin_state_exit_shim(SpinThenCountDownToState(cpu.x, cpu.y)));
}

/*
 * @routine 025B:718 SpinThenCountDownToPhase80
 * @run 70:B718
 * WHAT IT DOES: As SpinThenCountDownToState, but at zero it sets OBJ_36C8 $80 and goes to the next state.
 * HOW: StepObjectFrames, SpinOnCircle, a memory decrement, a store, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (spin_phase80_exit_shim).
 */
L8St24SpinArm SpinThenCountDownToPhase80(uint8_t slot, uint8_t y_entry)
{
    L8St24SpinArm r = { 0 };

    r.step = l8_list_step(slot, 0xB6DA, 0xB723, false, y_entry);
    WR8(OBJ->f3510[0], 2);
    WR8(OBJ->f3568[0], 0);
    jsr_push(0xB72E);
    r.spin = SpinOnCircle(slot);
    jsr_pop();
    r.timer = DEC8(OBJ->f3510[slot]);
    if (r.timer != 0) {
        r.arm = ST24_SPIN_COUNTING;
        return r;
    }
    WR8(OBJ->f36c8[slot], 0x80);
    r.state = INC8(OBJ->state[slot]);
    r.arm = ST24_SPIN_DONE;
    return r;
}

/* What SpinThenCountDownToPhase80 leaves in A, Y and the flags, from the arm it ended on. */
static Ended spin_phase80_exit_shim(L8St24SpinArm r)
{
    Ended e = then(l8_step_ended(r.step), ended_load(2));

    e = then(e, l8_spin_on_circle_exit_shim(r.spin));
    e = then(e, ended_nz(r.timer));
    if (r.arm == ST24_SPIN_COUNTING) {
        return e;
    }
    e = then(e, ended_load(0x80));
    return then(e, ended_nz(r.state));
}

void abi_SpinThenCountDownToPhase80(void)
{
    abi_ended(spin_phase80_exit_shim(SpinThenCountDownToPhase80(cpu.x, cpu.y)));
}

/*
 * @routine 025B:73C SpinThenCountPhaseDown
 * @run 70:B73C
 * WHAT IT DOES: Steps the animation list at $B6DA, runs SpinOnCircle, sets OBJ_3670 2 and counts OBJ_36C8 down; at zero clears the animation and OBJ_3670 and goes to the next state.
 * HOW: StepObjectFrames, SpinOnCircle, a memory decrement, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (spin_phase_down_exit_shim).
 */
L8St24SpinArm SpinThenCountPhaseDown(uint8_t slot, uint8_t y_entry)
{
    L8St24SpinArm r = { 0 };

    r.step = l8_list_step(slot, 0xB6DA, 0xB747, false, y_entry);
    jsr_push(0xB74A);
    r.spin = SpinOnCircle(slot);
    jsr_pop();
    WR8(OBJ->f3670[slot], 2);
    r.timer = DEC8(OBJ->f36c8[slot]);
    if (r.timer != 0) {
        r.arm = ST24_SPIN_COUNTING;
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3670[slot], 0);
    r.state = INC8(OBJ->state[slot]);
    r.arm = ST24_SPIN_DONE;
    return r;
}

/* What SpinThenCountPhaseDown leaves in A, Y and the flags, from the arm it ended on. */
static Ended spin_phase_down_exit_shim(L8St24SpinArm r)
{
    Ended e = then(l8_step_ended(r.step), l8_spin_on_circle_exit_shim(r.spin));

    e = then(e, ended_load(2));
    e = then(e, ended_nz(r.timer));
    if (r.arm == ST24_SPIN_COUNTING) {
        return e;
    }
    return then(e, ended_nz(r.state));
}

void abi_SpinThenCountPhaseDown(void)
{
    abi_ended(spin_phase_down_exit_shim(SpinThenCountPhaseDown(cpu.x, cpu.y)));
}

/*
 * @routine 025B:7C7 SpinThenCountWordDown
 * @run 70:B7C7
 * WHAT IT DOES: Steps the animation list at $B6DA, runs SpinOnCircle and counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero clears OBJ_3250, goes to the next state and clears OBJ_2800.
 * HOW: StepObjectFrames, SpinOnCircle, a 16-bit subtract in memory, an ORA, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (spin_word_exit_shim).
 */
L8St24SpinArm SpinThenCountWordDown(uint8_t slot, uint8_t y_entry)
{
    L8St24SpinArm r = { 0 };
    uint8_t low, high, again;
    Sum diff;

    r.step = l8_list_step(slot, 0xB6DA, 0xB7D2, false, y_entry);
    jsr_push(0xB7D5);
    r.spin = SpinOnCircle(slot);
    jsr_pop();
    low = RD8(OBJ->f3510[slot]);
    diff = sub_bytes(low, 1, true);
    WR8(OBJ->f3510[slot], diff.value);
    high = RD8(OBJ->f34b8[slot]);
    r.diff = sub_bytes(high, 0, diff.carry);
    WR8(OBJ->f34b8[slot], r.diff.value);
    again = RD8(OBJ->f3510[slot]);
    r.joined = (uint8_t)(r.diff.value | again);
    if (r.joined != 0) {
        r.arm = ST24_SPIN_COUNTING;
        return r;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    r.state = INC8(OBJ->state[slot]);
    WR8(OBJ->f2800[slot], 0);
    r.arm = ST24_SPIN_DONE;
    return r;
}

/* What SpinThenCountWordDown leaves in A, Y and the flags, from the arm it ended on. */
static Ended spin_word_exit_shim(L8St24SpinArm r)
{
    Ended e = then(l8_step_ended(r.step), l8_spin_on_circle_exit_shim(r.spin));

    e = then(then(e, ended_of_sum(r.diff)), ended_load(r.joined));
    if (r.arm == ST24_SPIN_COUNTING) {
        return e;
    }
    return then(e, ended_nz(r.state));
}

void abi_SpinThenCountWordDown(void)
{
    abi_ended(spin_word_exit_shim(SpinThenCountWordDown(cpu.x, cpu.y)));
}

/* The Y the sound call takes: the one the ring start left, else the one the routine was entered with. */
static uint8_t ring_y(const L8St24RingArm *r, uint8_t y_entry)
{
    Ended e = l8_ring_exit_shim(r->ring);

    return e.y_set ? e.y : y_entry;
}

/*
 * @routine 0B4B:690 CountDownThenDrawAndSpawnRing
 * @run 70:B690
 * WHAT IT DOES: When the scroll's screen byte is 2, counts OBJ_34B8 down; at zero draws tile script 0, starts the ring of pieces (StartRingOfPieces), plays sound $56, sets the scroll's X limit to $0280 and OBJ_34B8 $60 and goes to the next state.
 * HOW: A compare, a memory decrement, DrawTileScript, StartRingOfPieces, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (ring_exit_shim).
 */
L8St24RingArm CountDownThenDrawAndSpawnRing(uint8_t slot, uint8_t y_entry)
{
    L8St24RingArm r = { 0 };

    r.screen = RD8(SCROLLE->x_hi_screen[0]);
    if (r.screen != 2) {
        r.arm = ST24_RING_OTHER_SCREEN;
        return r;
    }
    r.timer = DEC8(OBJ->f34b8[slot]);
    if (r.timer != 0) {
        r.arm = ST24_RING_COUNTING;
        return r;
    }
    jsr_push(0xB6A2);
    r.run = DrawTileScript(0, 0);
    jsr_pop();
    jsr_push(0xB6A5);
    r.ring = StartRingOfPieces(slot);
    jsr_pop();
    jsr_push(0xB6AA);
    r.call = SoundCommand(0x56, slot, ring_y(&r, y_entry));
    jsr_pop();
    wr8(ScrollXMaxHi, 2);
    wr8(ScrollXMaxLo, 0x80);
    WR8(OBJ->f34b8[slot], 0x60);
    r.state = INC8(OBJ->state[slot]);
    r.arm = ST24_RING_STARTED;
    return r;
}

/* What CountDownThenDrawAndSpawnRing leaves, step by step: the exits and the sound call, from the arm it ended on. */
static ExitChain ring_exit_shim(L8St24RingArm r)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    Ended e = ended_compare(r.screen, 2);
    SoundEnd s = l8_sound_none(ended_kept());

    if (r.arm == ST24_RING_OTHER_SCREEN) {
        chain_ended(&c, e);
        return c;
    }
    e = then(e, ended_nz(r.timer));
    chain_ended(&c, e);
    if (r.arm == ST24_RING_COUNTING) {
        return c;
    }
    chain_ended(&c, ended_of_tile_run(r.run, 0));
    chain_ended(&c, l8_ring_exit_shim(r.ring));
    s.sounded = true;
    s.command = 0x56;
    s.before = ended_load(0x56);
    s.call = r.call;
    s.end = then(ended_load(0x60), ended_nz(r.state));
    chain_sound(&c, s);
    return c;
}

void abi_CountDownThenDrawAndSpawnRing(void)
{
    ExitChain c = ring_exit_shim(CountDownThenDrawAndSpawnRing(cpu.x, cpu.y));

    abi_chain(&c);
}
