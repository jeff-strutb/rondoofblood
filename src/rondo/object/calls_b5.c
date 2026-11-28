/*
 * More calling routines of stages $09 to $10 and the secret stages.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "calls_b.h"
#include "calls_b3.h"
#include "calls_b5.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

/* JSR LoadObjectRecord's vector. */
static void load_record(uint8_t slot, uint16_t ret)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(ret);
    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
    jsr_pop();
}

/* LDA #command and JSR SoundCommand's vector. */
static void sound(uint8_t command, uint16_t ret)
{
    CommandCall call;

    lda(command);
    jsr_push(ret);
    call = SoundCommand(command, cpu.x, cpu.y);
    sound_command_exit_shim(command, call);
    jsr_pop();
}

/* The frame list's address into zTemp11 then zTemp10 (or the other way round), and JSR StepObjectFrames. */
static void step_list(uint8_t slot, uint16_t list, bool low_first, uint16_t ret)
{
    uint8_t y = cpu.y;
    FrameStep step;

    if (low_first) {
        lda((uint8_t)list);
        WR8(ZP->temp10, cpu.a);
        lda((uint8_t)(list >> 8));
        WR8(ZP->temp11, cpu.a);
    } else {
        lda((uint8_t)(list >> 8));
        WR8(ZP->temp11, cpu.a);
        lda((uint8_t)list);
        WR8(ZP->temp10, cpu.a);
    }
    jsr_push(ret);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
}

static void restart_animation(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

static void clear_slot_in_y(uint16_t ret)
{
    jsr_push(ret);
    ClearObjectInY(cpu.y);
    jsr_pop();
}

/* LDA #kind and JSR StartObjectAtPlace: the object at X's place, in slot Y. */
static void start_at_place(uint8_t kind, uint16_t ret)
{
    lda(kind);
    jsr_push(ret);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
}

/* LDA #script, LDX #slot, JSR StartScriptSlotKind1, between PHX and PLX. */
static void script_kept_x(uint8_t number, uint8_t at_slot, uint16_t ret)
{
    push8(cpu.x);
    lda(number);
    cpu.x = at_slot;
    abi_nz(at_slot);
    jsr_push(ret);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
    cpu.x = pull8();
    abi_nz(cpu.x);
}

static void move_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
}

static void move_y(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
}

static void gravity(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
}

static void step_list_tail_(uint8_t slot, uint16_t list)
{
    uint8_t y = cpu.y;
    FrameStep step;

    lda((uint8_t)list);
    WR8(ZP->temp10, cpu.a);
    lda((uint8_t)(list >> 8));
    WR8(ZP->temp11, cpu.a);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
}

static uint8_t INC8_IDX_(uint16_t column, uint8_t slot)
{
    uint8_t value = (uint8_t)(RD8_IDX(column, slot) + 1);

    WR8_IDX(column, slot, value);
    return value;
}

/* DEC of a column: true when it reached zero. */
static bool counted_out(uint8_t left)
{
    abi_nz(left);
    return left == 0;
}

/* BNE after StepObjectFrames: true when the list ended. */
static bool list_ended(void)
{
    return (cpu.p & P_Z) != 0;
}
/* JSR FindFreeSlotFrom30's vector: true when a slot was found, in Y. */
static bool find_from30(uint16_t ret)
{
    uint8_t found_a, to;
    bool found;

    jsr_push(ret);
    to = FindFreeSlotFrom30(&found_a, &found);
    find_slot_exit_shim(to, found_a, found);
    jsr_pop();
    return found;
}

static void move(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
}

/*
 * @routine 10CC:202 LandBelow90OrFall10CC
 * @run 70:BA02
 * WHAT IT DOES: Low on the first screen lands with height 4; else falls and moves, C clear (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandBelow90OrFall10CC(uint8_t slot)
{
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->y_lo[slot]));
        cmp(cpu.a, 0x90);
        if (cpu.a >= 0x90) {
            lda(0x04);
            jsr_push(0xBA13);
            mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
            jsr_pop();
            return;
        }
    }
    gravity(slot, 0xBA17);
    move(slot, 0xBA1A);
    abi_flag(P_C, false);
}

void abi_LandBelow90OrFall10CC(void)
{
    LandBelow90OrFall10CC(cpu.x);
}

/*
 * @routine 117B:34A ListByWorkEThenState1_117B
 * @run 70:B34A
 * WHAT IT DOES: Plays the list the table gives for OBJ_WORK_E; when it ends, state 1 (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListByWorkEThenState1_117B(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    lda(RD8(OBJ->work_e[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB364 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xB365 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    jsr_push(0xB35C);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
    if (!list_ended()) {
        return;
    }
    lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ListByWorkEThenState1_117B(void)
{
    ListByWorkEThenState1_117B(cpu.x);
}

/*
 * @routine 161B:71E BreakIntoFive161B
 * @run 70:B71E
 * WHAT IT DOES: Starts up to five kind $34 objects numbered 5 down at this place, then clears itself (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BreakIntoFive161B(uint8_t slot)
{
    lda(0x05);
    WR8(ZP->temp00, cpu.a);
    do {
        if (!find_from30(0xB725)) {
            break;
        }
        start_at_place(0x34, 0xB72C);
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        abi_nz(DEC8(ZP->temp00));
    } while ((cpu.p & P_Z) == 0);
    ClearObject(slot);
}

void abi_BreakIntoFive161B(void)
{
    BreakIntoFive161B(cpu.x);
}

/*
 * @routine 0E0C:4C5 LoadAndPop0E0C
 * @run 70:BCC5
 * WHAT IT DOES: Loads the record, moves up 40 dots at once, then rises at -6 with OBJ_WORK_E 40 (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAndPop0E0C(uint8_t slot)
{
    load_record(slot, 0xBCC8);
    lda(0xD8);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xBCD0);
    lda(0x28);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0xFA);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->vy_lo[slot], cpu.a);
}

void abi_LoadAndPop0E0C(void)
{
    LoadAndPop0E0C(cpu.x);
}

/*
 * @routine 13FB:48C SpawnAtStep18_13FB
 * @run 70:B48C
 * WHAT IT DOES: At animation step $12 runs on into SpawnOddFramesAnswer13FB, else leaves C clear (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnAtStep18_13FB(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x12);
    if (cpu.a == 0x12) {
        SpawnOddFramesAnswer13FB(slot);
        return;
    }
    abi_flag(P_C, false);
}

void abi_SpawnAtStep18_13FB(void)
{
    SpawnAtStep18_13FB(cpu.x);
}

/*
 * @routine 13FC:4FA KnockAwayWhenHit13FC
 * @run 70:BCFA
 * WHAT IT DOES: A hit of 5 or more: OBJ_WORK_D the angle away from the player, OBJ_WORK_E 2, OBJ_WORK_F $80 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void KnockAwayWhenHit13FC(uint8_t slot)
{
    AngleExit angle;
    Sum sum;

    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    jsr_push(0xBD04);
    angle = AngleToPlayer(slot);
    angle_exit_shim(angle);
    jsr_pop();
    sum = add_bytes(cpu.a, 0x80, false);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->work_f[slot], cpu.a);
}

void abi_KnockAwayWhenHit13FC(void)
{
    KnockAwayWhenHit13FC(cpu.x);
}

/*
 * @routine 13FD:144 LoadAndToss13FD
 * @run 71:C144
 * WHAT IT DOES: Loads the record, jumps 12 dots right and rises at -1.5 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAndToss13FD(uint8_t slot)
{
    load_record(slot, 0xC147);
    lda(0x01);
    WR8(OBJ->start_arg[slot], cpu.a);
    lda(0x0C);
    WR8(OBJ->vx_hi[slot], cpu.a);
    move_x(slot, 0xC154);
    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
}

void abi_LoadAndToss13FD(void)
{
    LoadAndToss13FD(cpu.x);
}

/*
 * @routine 18DB:4AF MoveThenStartFour18DB
 * @run 70:B4AF
 * WHAT IT DOES: Moves while OBJ_WORK_E counts; then starts kind $2F objects in slots $11 to $14 (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void MoveThenStartFour18DB(uint8_t slot)
{
    move_x(slot, 0xB4B2);
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    cpu.y = 0x11;
    abi_nz(cpu.y);
    do {
        clear_slot_in_y(0xB4BC);
        start_at_place(0x2F, 0xB4C1);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x15);
    } while (cpu.y < 0x15);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_MoveThenStartFour18DB(void)
{
    MoveThenStartFour18DB(cpu.x);
}

/*
 * @routine 09EB:3D6 SoundAtCount09EB
 * @run 70:B3D6
 * WHAT IT DOES: Sound $A3 each time OBJ_WORK_I reaches OBJ_WORK_L, which then moves on by $40 (secret stage 05's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundAtCount09EB(uint8_t slot)
{
    Sum sum;

    lda(RD8(OBJ->work_l[slot]));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->work_i[slot]));
    cmp(cpu.a, RD8(OBJ->work_l[slot]));
    if ((cpu.p & P_C) == 0) {
        return;
    }
    sound(0xA3, 0xB3E8);
    lda(RD8(OBJ->work_l[slot]));
    sum = add_bytes(cpu.a, 0x40, false);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->work_l[slot], cpu.a);
}

void abi_SoundAtCount09EB(void)
{
    SoundAtCount09EB(cpu.x);
}

/*
 * @routine 09EB:449 SoundA0AndSink09EB
 * @run 70:B449
 * WHAT IT DOES: Sound $A0, the hit box off, sinking half a dot, raster effect $0B from line $C0, substate on (secret stage 05's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundA0AndSink09EB(uint8_t slot)
{
    sound(0xA0, 0xB44E);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x0B);
    WR8(ZP->raster_effect, cpu.a);
    lda(0xC0);
    WR8(ZP->raster_arg, cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundA0AndSink09EB(void)
{
    SoundA0AndSink09EB(cpu.x);
}

/*
 * @routine 0B4C:2E1 ListThenState5When0B4C
 * @run 70:BAE1
 * WHAT IT DOES: Plays the list at $BAFD; when $B77C leaves C set, the animation restarted and state 5 (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenState5When0B4C(uint8_t slot)
{
    step_list(slot, 0xBAFD, false, 0xBAEC);
    call_routine(0xBAEF, 0xB77C);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    restart_animation(slot);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ListThenState5When0B4C(void)
{
    ListThenState5When0B4C(cpu.x);
}

/*
 * @routine 0E0C:2E0 ListThenHover0E0C
 * @run 70:BAE0
 * WHAT IT DOES: Plays the list at $BAFC; when it ends, OBJ_WORK_E 23, no Y speed, state on (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenHover0E0C(uint8_t slot)
{
    step_list(slot, 0xBAFC, true, 0xBAEB);
    if (!list_ended()) {
        return;
    }
    lda(0x17);
    WR8(OBJ->work_e[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListThenHover0E0C(void)
{
    ListThenHover0E0C(cpu.x);
}

/*
 * @routine 0EBD:619 ListCountThenNext0EBD
 * @run 71:C619
 * WHAT IT DOES: Plays the list at $C56E while OBJ_WORK_D counts; at zero this state, and with a start argument the next slot's, count on (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListCountThenNext0EBD(uint8_t slot)
{
    step_list(slot, 0xC56E, true, 0xC624);
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    INC8(OBJ->state[slot]);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a == 0) {
        return;
    }
    abi_nz(INC8_IDX_(COL(OBJ->state) + 1, slot));
}

void abi_ListCountThenNext0EBD(void)
{
    ListCountThenNext0EBD(cpu.x);
}

/*
 * @routine 101B:4D4 LoadAndWait101B
 * @run 70:B4D4
 * WHAT IT DOES: Loads the record, facing left, no hit box, bit 6 of OBJ_LOOP_FLAGS off, state 4 (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAndWait101B(uint8_t slot)
{
    load_record(slot, 0xB4D7);
    lda(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    WR8(OBJ->hitbox_on[slot], 0);
    lda(RD8(OBJ->loop_flags[slot]));
    lda((uint8_t)(cpu.a & 0xBF));
    WR8(OBJ->loop_flags[slot], cpu.a);
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_LoadAndWait101B(void)
{
    LoadAndWait101B(cpu.x);
}

/*
 * @routine 101E:6B4 LoadAtRandomTop101E
 * @run 71:CEB4
 * WHAT IT DOES: Loads the record and puts the object above the screen at a random X (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAtRandomTop101E(uint8_t slot)
{
    Sum sum;

    load_record(slot, 0xCEB7);
    WR8(OBJ->x_hi[slot], 0);
    jsr_push(0xCEBD);
    random_exit_shim(NextRandom());
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x7F));
    sum = add_bytes(cpu.a, 0xC0, false);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0xFF);
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(0xF0);
    WR8(OBJ->y_lo[slot], cpu.a);
}

void abi_LoadAtRandomTop101E(void)
{
    LoadAtRandomTop101E(cpu.x);
}

/*
 * @routine 1020:047 StartCopyOfWorks1020_047
 * @run 71:D847
 * WHAT IT DOES: Starts a kind $B3 object in a free slot with this object's work_f and work_e; zTemp00 counted.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartCopyOfWorks1020_047(uint8_t slot)
{
    if (!find_from30(0xD84A)) {
        return;
    }
    clear_slot_in_y(0xD84F);
    start_at_place(0xB3, 0xD854);
    lda(RD8(OBJ->work_f[slot]));
    WR8(OBJ->work_g[cpu.y], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    WR8(OBJ->work_h[cpu.y], cpu.a);
    abi_nz(INC8(ZP->temp00));
}

void abi_StartCopyOfWorks1020_047(void)
{
    StartCopyOfWorks1020_047(cpu.x);
}

/*
 * @routine 10CD:3D9 StartCopyOfWorks10CD_3D9
 * @run 71:C3D9
 * WHAT IT DOES: Starts a kind $A4 object in a free slot with this object's work_f and start_arg; zTemp00 counted.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartCopyOfWorks10CD_3D9(uint8_t slot)
{
    if (!find_from30(0xC3DC)) {
        return;
    }
    clear_slot_in_y(0xC3E1);
    start_at_place(0xA4, 0xC3E6);
    lda(RD8(OBJ->work_f[slot]));
    WR8(OBJ->work_g[cpu.y], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    abi_nz(INC8(ZP->temp00));
}

void abi_StartCopyOfWorks10CD_3D9(void)
{
    StartCopyOfWorks10CD_3D9(cpu.x);
}

/*
 * @routine 117E:06C SpeedByWorkEAndMove117E
 * @run 71:C86C
 * WHAT IT DOES: The speeds the table gives for OBJ_WORK_E, X reversed when facing left, then a move (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpeedByWorkEAndMove117E(uint8_t slot)
{
    lda(RD8(OBJ->work_e[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC888 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC889 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    if (cpu.a != 0) {
        jsr_push(0xC885);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_SpeedByWorkEAndMove117E(void)
{
    SpeedByWorkEAndMove117E(cpu.x);
}

/*
 * @routine 12DA:7BC SoundScriptEvery180_12DA
 * @run 70:AFBC
 * WHAT IT DOES: Counts OBJ_WORK_D; at zero sound $47, script $87, the count 180 and the state on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundScriptEvery180_12DA(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    sound(0x47, 0xAFC6);
    script_kept_x(0x87, 0x05, 0xAFCE);
    lda(0xB4);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SoundScriptEvery180_12DA(void)
{
    SoundScriptEvery180_12DA(cpu.x);
}

/*
 * @routine 13FB:384 LoadAboveAndScript13FB
 * @run 70:B384
 * WHAT IT DOES: Loads the record above the screen, OBJ_WORK_G $80, script $90 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAboveAndScript13FB(uint8_t slot)
{
    load_record(slot, 0xB387);
    lda(0xFF);
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(0xC0);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->work_g[slot], cpu.a);
    script_kept_x(0x90, 0x04, 0xB39E);
}

void abi_LoadAboveAndScript13FB(void)
{
    LoadAboveAndScript13FB(cpu.x);
}

/*
 * @routine 09EC:25F FallThenClearOffScreen09EC
 * @run 70:BA5F
 * WHAT IT DOES: Gravity of $18, a move, cleared once off the first screen along Y (secret stage 05's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallThenClearOffScreen09EC(uint8_t slot)
{
    Sum sum;

    lda(RD8(OBJ->vy_lo[slot]));
    sum = add_bytes(cpu.a, 0x18, false);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    sum = add_bytes(cpu.a, 0x00, (cpu.p & P_C) != 0);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xBA73);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        ClearObject(slot);
    }
}

void abi_FallThenClearOffScreen09EC(void)
{
    FallThenClearOffScreen09EC(cpu.x);
}

/*
 * @routine 0CAD:00D ListByWorkDThenSlot16_0CAD
 * @run 71:C00D
 * WHAT IT DOES: Plays the list for OBJ_WORK_D; once slot $10 is in substate 4 the state counts on (secret stage 07's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListByWorkDThenSlot16_0CAD(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    lda(RD8(OBJ->work_d[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC02A + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xC02B + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    jsr_push(0xC01F);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
    lda(RD8(OBJ->substate[0x10]));
    cmp(cpu.a, 0x04);
    if (cpu.a != 0x04) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListByWorkDThenSlot16_0CAD(void)
{
    ListByWorkDThenSlot16_0CAD(cpu.x);
}

/*
 * @routine 0E0B:718 ListChannelThenSlot80_0E0B
 * @run 70:B718
 * WHAT IT DOES: List channel $84; substate 3 while slot $50 is taken, else sound $CF and substate on (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListChannelThenSlot80_0E0B(uint8_t slot)
{
    push8(cpu.x);
    lda(0x84);
    cpu.x = 0x05;
    abi_nz(cpu.x);
    jsr_push(0xB720);
    list_channel_exit_shim(StartListChannelKind2(cpu.a, cpu.x));
    jsr_pop();
    cpu.x = pull8();
    abi_nz(cpu.x);
    lda(RD8(OBJ->kind[0x50]));
    if (cpu.a != 0) {
        lda(0x03);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    sound(0xCF, 0xB731);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListChannelThenSlot80_0E0B(void)
{
    ListChannelThenSlot80_0E0B(cpu.x);
}

/*
 * @routine 0EBD:191 LoadSetBoxAndList0EBD
 * @run 71:C191
 * WHAT IT DOES: Loads the record, sprite set 4, a narrow hit box, and plays the list at $C223 (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadSetBoxAndList0EBD(uint8_t slot)
{
    load_record(slot, 0xC194);
    lda(0x04);
    WR8(OBJ->sprite_set[slot], cpu.a);
    lda(0x04);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x0F);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    step_list_tail_(slot, 0xC223);
}

void abi_LoadSetBoxAndList0EBD(void)
{
    LoadSetBoxAndList0EBD(cpu.x);
}

/*
 * @routine 101D:6D6 BurstThenClearPair101D
 * @run 71:C6D6
 * WHAT IT DOES: Counts OBJ_WORK_G; then kind $BF objects in slots $20 to $31 and this object and the next cleared (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BurstThenClearPair101D(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_g[slot]))) {
        return;
    }
    cpu.y = 0x20;
    abi_nz(cpu.y);
    do {
        clear_slot_in_y(0xC6E0);
        start_at_place(0xBF, 0xC6E5);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x32);
    } while (cpu.y < 0x32);
    jsr_push(0xC6ED);
    ClearObject(cpu.x);
    jsr_pop();
    cpu.x = (uint8_t)(cpu.x + 1);
    abi_nz(cpu.x);
    jsr_push(0xC6F1);
    ClearObject(cpu.x);
    jsr_pop();
    cpu.x = (uint8_t)(cpu.x - 1);
    abi_nz(cpu.x);
}

void abi_BurstThenClearPair101D(void)
{
    BurstThenClearPair101D(cpu.x);
}

/*
 * @routine 101F:721 StartInNextThree101F
 * @run 71:D721
 * WHAT IT DOES: Starts a kind $B7 object in the first empty of the next three slots; zTemp00 counted (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartInNextThree101F(uint8_t slot)
{
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    lda(0x03);
    WR8(ZP->temp01, cpu.a);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (cpu.a == 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        abi_nz(cpu.y);
        abi_nz(DEC8(ZP->temp01));
        if ((cpu.p & P_Z) != 0) {
            return;
        }
    }
    clear_slot_in_y(0xD736);
    start_at_place(0xB7, 0xD73B);
    abi_nz(INC8(ZP->temp00));
}

void abi_StartInNextThree101F(void)
{
    StartInNextThree101F(cpu.x);
}

/*
 * @routine 12DB:746 ListThenBox12DB
 * @run 70:B746
 * WHAT IT DOES: Plays the list at $B763; when it ends, a wide low hit box and state 2 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenBox12DB(uint8_t slot)
{
    step_list(slot, 0xB763, true, 0xB751);
    if (!list_ended()) {
        return;
    }
    lda(0x1C);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ListThenBox12DB(void)
{
    ListThenBox12DB(cpu.x);
}

/*
 * @routine 13FD:3D5 SwingAndCountWord13FD
 * @run 71:C3D5
 * WHAT IT DOES: Swings (the routine at $C3F2) and counts the word OBJ_WORK_D-E down; at zero the state on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwingAndCountWord13FD(uint8_t slot)
{
    Sum sum;

    call_routine(0xC3D8, 0xC3F2);
    lda(RD8(OBJ->work_e[slot]));
    sum = sub_bytes(cpu.a, 0x01, true);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    sum = sub_bytes(cpu.a, 0x00, (cpu.p & P_C) != 0);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->work_d[slot], cpu.a);
    lda((uint8_t)(cpu.a | RD8(OBJ->work_e[slot])));
    if (cpu.a != 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SwingAndCountWord13FD(void)
{
    SwingAndCountWord13FD(cpu.x);
}

/*
 * @routine 0EBE:15E FrameByRiseThenLand0EBE
 * @run 71:C95E
 * WHAT IT DOES: A rising or falling frame; on landing state 3 (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FrameByRiseThenLand0EBE(uint8_t slot)
{
    bool out;

    lda(RD8(OBJ->vy_hi[slot]));
    out = (cpu.a & 0x80) != 0;
    lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
    abi_flag(P_C, out);
    out = (cpu.a & 0x80) != 0;
    lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
    abi_flag(P_C, out);
    lda((uint8_t)(cpu.a & 0x01));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC97C + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x06);
    jsr_push(0xC971);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_FrameByRiseThenLand0EBE(void)
{
    FrameByRiseThenLand0EBE(cpu.x);
}

/*
 * @routine 117F:459 ListThenProbe117F
 * @run 71:D459
 * WHAT IT DOES: Plays the list at $D479; on a new frame, the X speed by OBJ_WORK_F, running on into ProbeHeightCall117F_46F (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenProbe117F(uint8_t slot)
{
    step_list(slot, 0xD479, true, 0xD464);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    cpu.y = RD8(OBJ->work_f[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xD477 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    ProbeHeightCall117F_46F();
}

void abi_ListThenProbe117F(void)
{
    ListThenProbe117F(cpu.x);
}

/*
 * @routine 117F:4DA NextOfCycle117F_4DA
 * @run 71:D4DA
 * WHAT IT DOES: The next substate of a cycle of eight (OBJ_WORK_H), the animation restarted.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void NextOfCycle117F_4DA(uint8_t slot)
{
    restart_animation(slot);
    jsr_push(0xD4E3);
    random_exit_shim(NextRandom());
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x03));
    WR8(OBJ->work_k[slot], cpu.a);
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xD4F8 + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    abi_nz(INC8(OBJ->work_h[slot]));
}

void abi_NextOfCycle117F_4DA(void)
{
    NextOfCycle117F_4DA(cpu.x);
}

/*
 * @routine 12DD:552 NextOfCycle12DD_552
 * @run 71:C552
 * WHAT IT DOES: The next substate of a cycle of eight (OBJ_WORK_H), the animation restarted.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void NextOfCycle12DD_552(uint8_t slot)
{
    jsr_push(0xC555);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    WR8(OBJ->facing_left[slot], cpu.a);
    restart_animation(slot);
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC570 + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    abi_nz(INC8(OBJ->work_h[slot]));
}

void abi_NextOfCycle12DD_552(void)
{
    NextOfCycle12DD_552(cpu.x);
}

/*
 * @routine 12DE:62A DissolveThenWait12DE
 * @run 71:CE2A
 * WHAT IT DOES: Dissolves patterns 0 to 3 while OBJ_WORK_G counts; then the count 128, substate on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DissolveThenWait12DE(uint8_t slot)
{
    lda(0x00);
    wr8(WorkByteA, cpu.a);
    lda(0x04);
    wr8(WorkByteB, cpu.a);
    call_routine(0xCE37, 0xCE5E);
    if (!counted_out(DEC8(OBJ->work_g[slot]))) {
        return;
    }
    lda(0x80);
    WR8(OBJ->work_g[slot], cpu.a);
    WR8(OBJ->work_l[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_DissolveThenWait12DE(void)
{
    DissolveThenWait12DE(cpu.x);
}

/*
 * @routine 16CB:686 DissolveThenBecome29_16CB
 * @run 70:B686
 * WHAT IT DOES: Dissolves patterns 0 to 9 while OBJ_WORK_D counts; then the slot becomes kind $29 (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DissolveThenBecome29_16CB(uint8_t slot)
{
    lda(0x00);
    wr8(WorkByteA, cpu.a);
    lda(0x0A);
    wr8(WorkByteB, cpu.a);
    call_routine(0xB693, 0xB6A4);
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    jsr_push(0xB69B);
    ClearObject(slot);
    jsr_pop();
    lda(0x29);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_DissolveThenBecome29_16CB(void)
{
    DissolveThenBecome29_16CB(cpu.x);
}

/*
 * @routine 18DC:172 StartKind2DAt48_18DC
 * @run 70:B972
 * WHAT IT DOES: Once slot $33 is empty, a kind $2D object in slot $30, OBJ_WORK_E 56, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartKind2DAt48_18DC(uint8_t slot)
{
    lda(RD8(OBJ->kind[0x33]));
    if (cpu.a != 0) {
        return;
    }
    cpu.y = 0x30;
    abi_nz(cpu.y);
    clear_slot_in_y(0xB97C);
    start_at_place(0x2D, 0xB981);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    lda(0x38);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartKind2DAt48_18DC(void)
{
    StartKind2DAt48_18DC(cpu.x);
}

/*
 * @routine 0EC0:386 ListThenBurstOnRow0EC0
 * @run 71:DB86
 * WHAT IT DOES: Plays the list at $DB5D, runs $D7B5 on the $Dx row, falls and moves (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenBurstOnRow0EC0(uint8_t slot)
{
    step_list(slot, 0xDB5D, false, 0xDB91);
    lda(RD8(OBJ->y_lo[slot]));
    lda((uint8_t)(cpu.a & 0xF0));
    cmp(cpu.a, 0xD0);
    if (cpu.a == 0xD0) {
        call_routine(0xDB9D, 0xD7B5);
    }
    jsr_push(0xDBA0);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_ListThenBurstOnRow0EC0(void)
{
    ListThenBurstOnRow0EC0(cpu.x);
}
