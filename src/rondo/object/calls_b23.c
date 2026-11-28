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
#include "calls_b17.h"
#include "calls_b23.h"

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
static void add_a(uint8_t value, bool carry_in)
{
    Sum sum = add_bytes(cpu.a, value, carry_in);

    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

static void random_a(uint16_t ret)
{
    jsr_push(ret);
    random_exit_shim(NextRandom());
    jsr_pop();
}

static void ldy(uint8_t value)
{
    cpu.y = value;
    abi_nz(value);
}

/* LDA, ASL, TAY: a word index from an object column. */
static void word_index(uint8_t value)
{
    abi_flag(P_C, (value & 0x80) != 0);
    lda((uint8_t)(value << 1));
    cpu.y = cpu.a;
}

static void velocity_from_a(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
}

static void near_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
}
/* ROL of A through the carry. */
static void rol_a(void)
{
    bool out = (cpu.a & 0x80) != 0;

    lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
    abi_flag(P_C, out);
}

/*
 * @routine 117A:0BC BounceThenOpen117A
 * @run 70:A8BC
 * WHAT IT DOES: A hit piece bounces a few times (sound $19), then notes itself broken in zField8F and becomes what it holds.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BounceThenOpen117A(uint8_t slot)
{
    sound(0x19, 0xA8C1);
    lda(0x01);
    call_routine(0xA8C6, 0xA90B);
    ldy(RD8(OBJ->work_e[slot]));
    INC8(OBJ->work_d[slot]);
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xB072 + cpu.y)));
    if (cpu.a < rd8((uint16_t)(0xB072 + cpu.y))) {
        lda(RD8(OBJ->work_d[slot]));
        lda((uint8_t)(cpu.a - 1));
        lda((uint8_t)(cpu.a << 1));
        add_a(RD8(OBJ->work_e[slot]), false);
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xB078 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xB079 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        move(slot, 0xA8ED);
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    call_routine(0xA8F4, 0xA855);
    lda((uint8_t)(cpu.a | RD8(ZP->field8f)));
    WR8(ZP->field8f, cpu.a);
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB077 + cpu.y)));
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
}

void abi_BounceThenOpen117A(void)
{
    BounceThenOpen117A(cpu.x);
}

/*
 * @routine 161A:0BC BounceThenOpen161A
 * @run 70:A8BC
 * WHAT IT DOES: A hit piece bounces a few times (sound $19), then notes itself broken in zField8F and becomes what it holds.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BounceThenOpen161A(uint8_t slot)
{
    sound(0x19, 0xA8C1);
    lda(0x01);
    call_routine(0xA8C6, 0xA90B);
    ldy(RD8(OBJ->work_e[slot]));
    INC8(OBJ->work_d[slot]);
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xB0A2 + cpu.y)));
    if (cpu.a < rd8((uint16_t)(0xB0A2 + cpu.y))) {
        lda(RD8(OBJ->work_d[slot]));
        lda((uint8_t)(cpu.a - 1));
        lda((uint8_t)(cpu.a << 1));
        add_a(RD8(OBJ->work_e[slot]), false);
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xB0A8 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xB0A9 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        move(slot, 0xA8ED);
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    call_routine(0xA8F4, 0xA855);
    lda((uint8_t)(cpu.a | RD8(ZP->field8f)));
    WR8(ZP->field8f, cpu.a);
    ldy(RD8(OBJ->work_e[slot]));
    lda(rd8((uint16_t)(0xB0A7 + cpu.y)));
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
}

void abi_BounceThenOpen161A(void)
{
    BounceThenOpen161A(cpu.x);
}

/*
 * @routine 101E:4A5 LeadOrLaunch101E
 * @run 71:CCA5
 * WHAT IT DOES: While slot $10's OBJ_WORK_M is 1 follows the trail at its lag; else launches on its own angle, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LeadOrLaunch101E(uint8_t slot)
{
    call_routine(0xCCA8, 0xCC2B);
    lda(RD8(OBJ->work_m[0x10]));
    cmp(cpu.a, 0x01);
    if (cpu.a != 0x01) {
        WR8(OBJ->work_j[slot], 0);
        WR8(ZP->temp00, 0);
        lda(RD8(OBJ->work_i[slot]));
        WR8(ZP->temp01, cpu.a);
        lda(RD8(OBJ->work_e[slot]));
        velocity_from_a(slot, 0xCCBF);
        lda(RD8(OBJ->vy_hi[slot]));
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        {
            uint8_t old = RD8(OBJ->vy_hi[slot]);

            WR8(OBJ->vy_hi[slot], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
            abi_flag(P_C, (old & 0x01) != 0);
            old = RD8(OBJ->vy_lo[slot]);
            WR8(OBJ->vy_lo[slot], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
            abi_nz((uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
            abi_flag(P_C, (old & 0x01) != 0);
        }
        move(slot, 0xCCCC);
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    ldy(RD8(OBJ->work_d[slot]));
    lda(rd8((uint16_t)(0xCC1B + cpu.y)));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    WR8(ZP->temp00, 0);
    lda(RD8(OBJ->work_i[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(rd8((uint16_t)(0xC7FC + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    velocity_from_a(slot, 0xCCE7);
    lda(RD8(OBJ->vy_hi[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    {
        uint8_t old = RD8(OBJ->vy_hi[slot]);

        WR8(OBJ->vy_hi[slot], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_flag(P_C, (old & 0x01) != 0);
        old = RD8(OBJ->vy_lo[slot]);
        WR8(OBJ->vy_lo[slot], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_nz((uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_flag(P_C, (old & 0x01) != 0);
    }
    move(slot, 0xCCF4);
}

void abi_LeadOrLaunch101E(void)
{
    LeadOrLaunch101E(cpu.x);
}

/*
 * @routine 12DD:482 WalkByStateList12DD
 * @run 71:C482
 * WHAT IT DOES: On each new frame of the state's list walks at OBJ_WORK_E's speed, turning at walls.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkByStateList12DD(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    lda(RD8(OBJ->state[slot]));
    add_a((uint8_t)~0x02, true);
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC4D6 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xC4D7 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    jsr_push(0xC497);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC4D3 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC4D2 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    move_x(slot, 0xC4AD);
    lda(0x06);
    WR8(ZP->product.mid, cpu.a);
    lda(0x02);
    jsr_push(0xC4B6);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    lda(RD8(OBJ->vx_hi[slot]));
    rol_a();
    rol_a();
    lda((uint8_t)(cpu.a & 0x01));
    lda((uint8_t)(cpu.a ^ 0x01));
    word_index(cpu.a);
    lda(rd8((uint16_t)(TileHit + cpu.y)));
    lda((uint8_t)(cpu.a | rd8((uint16_t)(TileHit + 1 + cpu.y))));
    lda((uint8_t)(cpu.a & 0x02));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(0xC4CF);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocityX(slot));
}

void abi_WalkByStateList12DD(void)
{
    WalkByStateList12DD(cpu.x);
}

/*
 * @routine 16CC:4A5 DropThenScatter16CC
 * @run 70:BCA5
 * WHAT IT DOES: Falls to Y $B8 and breaks into six numbered kind $2C pieces.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DropThenScatter16CC(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xBCAD);
        lda(0x10);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        lda(0x1F);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x18, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xBCCC);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xB8);
    if (cpu.a < 0xB8) {
        return;
    }
    lda(0xB8);
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(ZP->temp00, 0);
    do {
        if (!find_from30(0xBCDD)) {
            break;
        }
        start_at_place(0x2C, 0xBCE4);
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        INC8(ZP->temp00);
        lda(RD8(ZP->temp00));
        cmp(cpu.a, 0x06);
    } while (cpu.a < 0x06);
    ClearObject(slot);
}

void abi_DropThenScatter16CC(void)
{
    DropThenScatter16CC(cpu.x);
}

/*
 * @routine 0EC0:3CC SwitchPlatform0EC0
 * @run 71:DBCC
 * WHAT IT DOES: Stood on: starts a kind $4A object with a count, goes down, holds the player, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwitchPlatform0EC0(uint8_t slot)
{
    lda(0x30);
    WR8(ZP->temp08, cpu.a);
    lda(0x16);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xDBD7, 0xAA35);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    ldy(0x10);
    clear_slot_in_y(0xDBDE);
    start_at_place(0x4A, 0xDBE3);
    lda(RD8(OBJ->work_g[slot]));
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    cmp(cpu.a, 0x03);
    if (cpu.a < 0x03) {
        INC8(OBJ->work_g[slot]);
    }
    lda(0x01);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0xC0);
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x02);
    wr8(PlayerStartPose, cpu.a);
    WR8(OBJ->work_f[0], 0);
    lda(RD8(OBJ->loop_flags[slot]));
    lda((uint8_t)(cpu.a | 0x01));
    WR8(OBJ->loop_flags[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SwitchPlatform0EC0(void)
{
    SwitchPlatform0EC0(cpu.x);
}

/*
 * @routine 0EC0:59F ReleaseOnStateChange0EC0
 * @run 71:DD9F
 * WHAT IT DOES: When $DC1D changes the state, throws the item for OBJ_WORK_G (by zField93 for the last).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ReleaseOnStateChange0EC0(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    push8(cpu.a);
    call_routine(0xDDA6, 0xDC1D);
    lda(pull8());
    cmp(cpu.a, RD8(OBJ->state[slot]));
    if ((cpu.p & P_Z) != 0) {
        return;
    }
    ldy(RD8(OBJ->work_g[slot]));
    lda(rd8((uint16_t)(0xDDF0 + cpu.y)));
    cmp(cpu.a, 0xFF);
    if (cpu.a == 0xFF) {
        cpu.y = 0;
        lda(RD8(ZP->field93));
        cmp(cpu.a, 0x07);
        if (cpu.a == 0x07) {
            cpu.y = (uint8_t)(cpu.y + 1);
            abi_nz(cpu.y);
        }
        lda(rd8((uint16_t)(0xDDF4 + cpu.y)));
    }
    ldy(0x11);
    clear_slot_in_y(0xDDC6);
    jsr_push(0xDDC9);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    lda(0xE8);
    add_a(RD8(OBJ->y_lo[cpu.y]), false);
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    lda(0xF0);
    add_a(RD8(OBJ->x_lo[cpu.y]), false);
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    lda(0xFA);
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    lda(0x01);
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    lda(0x80);
    WR8(OBJ->vx_lo[cpu.y], cpu.a);
    lda(0x50);
    WR8(OBJ->invuln_timer[cpu.y], cpu.a);
}

void abi_ReleaseOnStateChange0EC0(void)
{
    ReleaseOnStateChange0EC0(cpu.x);
}

/*
 * @routine 117E:005 LoadLeapByKind117E
 * @run 71:C805
 * WHAT IT DOES: Loads and leaps: by OBJ_WORK_E, or for 2 by the distance to the player; mirrored by facing.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadLeapByKind117E(uint8_t slot)
{
    load_record(slot, 0xC808);
    call_routine(0xC80B, 0xC86C);
    lda(0x38);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    cmp(cpu.a, 0x02);
    if (cpu.a != 0x02) {
        word_index(cpu.a);
        lda(rd8((uint16_t)(0xC857 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xC856 + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
    } else {
        near_x(slot, 0xC83C);
        for (int i = 0; i < 5; i++) {
            abi_flag(P_C, (cpu.a & 0x01) != 0);
            lda((uint8_t)(cpu.a >> 1));
        }
        word_index(cpu.a);
        lda(rd8((uint16_t)(0xC85D + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xC85C + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(0xFB);
        WR8(OBJ->vy_hi[slot], cpu.a);
    }
    lda(RD8(OBJ->facing_left[slot]));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(0xC833);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_LoadLeapByKind117E(void)
{
    LoadLeapByKind117E(cpu.x);
}

/*
 * @routine 161E:33B LoadColumnOfFour161E
 * @run 71:CB3B
 * WHAT IT DOES: The first of four starts three more, $20 apart to the left; the fourth shows frame 4.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadColumnOfFour161E(uint8_t slot)
{
    load_record(slot, 0xCB3E);
    lda(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->work_g[slot], cpu.a);
    lda((uint8_t)(cpu.a & 0x0F));
    WR8(OBJ->start_arg[slot], cpu.a);
    lda((uint8_t)(cpu.a & 0x03));
    WR8(OBJ->work_d[slot], cpu.a);
    if (cpu.a == 0) {
        lda(0x03);
        WR8(ZP->temp00, cpu.a);
        lda(cpu.x);
        lda((uint8_t)(cpu.a + 1));
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        do {
            clear_slot_in_y(0xCB5A);
            start_at_place(0xA1, 0xCB5F);
            lda(RD8_IDX((uint16_t)(COL(OBJ->work_g) - 1), cpu.y));
            lda((uint8_t)(cpu.a + 1));
            WR8(OBJ->start_arg[cpu.y], cpu.a);
            WR8(OBJ->work_g[cpu.y], cpu.a);
            lda(RD8_IDX((uint16_t)(COL(OBJ->x_lo) - 1), cpu.y));
            add_a((uint8_t)~0x20, true);
            WR8(OBJ->x_lo[cpu.y], cpu.a);
            lda(RD8_IDX((uint16_t)(COL(OBJ->x_hi) - 1), cpu.y));
            add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
            WR8(OBJ->x_hi[cpu.y], cpu.a);
            cpu.y = (uint8_t)(cpu.y + 1);
            abi_nz(cpu.y);
            abi_nz(DEC8(ZP->temp00));
        } while ((cpu.p & P_Z) == 0);
    }
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, 0x03);
    if (cpu.a != 0x03) {
        return;
    }
    lda(0x04);
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_LoadColumnOfFour161E(void)
{
    LoadColumnOfFour161E(cpu.x);
}

/*
 * @routine 101E:7F5 GateStates101E
 * @run 71:CFF5
 * WHAT IT DOES: In state 0 loads the gate and starts its marker (the code of StartMarkerLeft101F); other states go to $D027.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void GateStates101E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a != 0) {
        run_routine(0xD027);
        return;
    }
    load_record(slot, 0xCFFD);
    lda(0x12);
    WR8(OBJ->frame[slot], cpu.a);
    lda(RD8_IDX((uint16_t)(COL(OBJ->kind) - 2), slot));
    cmp(cpu.a, 0xBB);
    StartMarkerLeft101F(slot);
}

void abi_GateStates101E(void)
{
    GateStates101E(cpu.x);
}

/*
 * @routine 10CD:035 LoadDelaysAtRandom10CD
 * @run 71:C035
 * WHAT IT DOES: Loads random waits and speeds, by the start argument or the player's side.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadDelaysAtRandom10CD(uint8_t slot)
{
    load_record(slot, 0xC038);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a != 0) {
        random_a(0xC040);
        lda((uint8_t)(cpu.a & 0x3F));
        add_a(0xA0, false);
        WR8(OBJ->work_d[slot], cpu.a);
        random_a(0xC04B);
        lda((uint8_t)(cpu.a & 0x03));
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xC08E + cpu.y)));
        WR8(OBJ->work_g[slot], cpu.a);
    } else {
        jsr_push(0xC059);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        lda((uint8_t)(cpu.a & 0x01));
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        random_a(0xC05F);
        lda((uint8_t)(cpu.a & 0x7F));
        add_a(rd8((uint16_t)(0xC088 + cpu.y)), false);
        WR8(OBJ->work_d[slot], cpu.a);
        random_a(0xC06B);
        lda((uint8_t)(cpu.a & 0x03));
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xC08A + cpu.y)));
        WR8(OBJ->work_g[slot], cpu.a);
    }
    random_a(0xC077);
    lda((uint8_t)(cpu.a & 0x03));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC092 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(rd8((uint16_t)(0xC093 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_LoadDelaysAtRandom10CD(void)
{
    LoadDelaysAtRandom10CD(cpu.x);
}

/*
 * @routine 1020:1EA DriftingBackground1020
 * @run 71:D9EA
 * WHAT IT DOES: A drifting piece by slot and speed, wrapped from the left edge to the right.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DriftingBackground1020(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xD9F2);
        lda(0x10);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        lda(0x80);
        WR8(OBJ->sprite_flip[slot], cpu.a);
        lda(cpu.x);
        lda((uint8_t)(cpu.a & 0x03));
        WR8(OBJ->work_d[slot], cpu.a);
        lda(cpu.x);
        add_a((uint8_t)~0x48, true);
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xD9E2 + cpu.y)));
        WR8(OBJ->y_lo[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->work_d[slot]));
        lda((uint8_t)(cpu.a & 0x03));
        word_index(cpu.a);
        lda(rd8((uint16_t)(0xDA5A + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(rd8((uint16_t)(0xDA5B + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        move_x(slot, 0xDA29);
    }
    call_routine(0xDA2C, 0xDA3E);
    lda(RD8(OBJ->x_hi[slot]));
    cmp(cpu.a, 0xFF);
    if (cpu.a != 0xFF) {
        return;
    }
    lda(RD8(OBJ->x_lo[slot]));
    if ((cpu.p & P_N) != 0) {
        return;
    }
    lda(0x01);
    WR8(OBJ->x_hi[slot], cpu.a);
}

void abi_DriftingBackground1020(void)
{
    DriftingBackground1020(cpu.x);
}

/*
 * @routine 072D:039 SparkFromSlot10_072D
 * @run 71:C039
 * WHAT IT DOES: A spark thrown from slot $10, falling until its list ends.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SparkFromSlot10_072D(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC041);
        lda(RD8(OBJ->sprite_flip[0x10]));
        WR8(OBJ->sprite_flip[slot], cpu.a);
        random_a(0xC04A);
        lda((uint8_t)(cpu.a & 0x0F));
        add_a((uint8_t)~0x08, true);
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(0xE0);
        WR8(OBJ->vy_hi[slot], cpu.a);
        move(slot, 0xC05A);
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        lda(0x02);
        WR8(OBJ->vy_hi[slot], cpu.a);
        WR8(OBJ->vy_lo[slot], 0);
        return;
    }
    step_list(slot, 0xC08E, true, 0xC074);
    if (list_ended()) {
        ClearObject(slot);
        return;
    }
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x18, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xC08D);
}

void abi_SparkFromSlot10_072D(void)
{
    SparkFromSlot10_072D(cpu.x);
}

/*
 * @routine 10CC:2F9 SlideWithCosine10CC
 * @run 70:BAF9
 * WHAT IT DOES: Slides at the facing's speed, with a cosine sway while OBJ_WORK_N is 0, until it lands.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SlideWithCosine10CC(uint8_t slot)
{
    lda(RD8(OBJ->work_n[slot]));
    if (cpu.a == 0) {
        lda(0x50);
        WR8(ZP->temp00, cpu.a);
        lda(0x00);
        WR8(ZP->temp01, cpu.a);
        lda(RD8(OBJ->work_g[slot]));
        jsr_push(0xBB0C);
        scale_exit_shim(ScaleByCosine(cpu.a));
        jsr_pop();
        lda(RD8(OBJ->work_g[slot]));
        add_a(0x02, false);
        WR8(OBJ->work_g[slot], cpu.a);
        lda(RD8(OBJ->facing_left[slot]));
        word_index(cpu.a);
        lda(rd8(zProduct1));
        add_a(rd8((uint16_t)(0xBB48 + cpu.y)), false);
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(rd8(zProduct2));
        add_a(rd8((uint16_t)(0xBB49 + cpu.y)), (cpu.p & P_C) != 0);
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(0x04);
        jsr_push(0xBB30);
        mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
        jsr_pop();
        return;
    }
    lda(RD8(OBJ->facing_left[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBB48 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xBB49 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x04);
    jsr_push(0xBB47);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
}

void abi_SlideWithCosine10CC(void)
{
    SlideWithCosine10CC(cpu.x);
}
