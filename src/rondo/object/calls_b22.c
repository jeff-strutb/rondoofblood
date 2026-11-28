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
#include "calls_b12.h"
#include "calls_b22.h"

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

static void face_by_side(uint8_t slot, uint16_t column, uint16_t ret)
{
    jsr_push(ret);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    WR8_IDX(column, slot, cpu.a);
}

static void clear_this(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    ClearObject(slot);
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
 * @routine 12DC:491 SteerAndBob12DC
 * @run 70:BC91
 * WHAT IT DOES: Turns OBJ_WORK_D toward the facing's angle and adds a cosine bob into OBJ_WORK_E-F, restarted each cycle.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SteerAndBob12DC(uint8_t slot)
{
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xBCD9 + cpu.y)));
    WR8(ZP->temp02, cpu.a);
    lda(0x08);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xBCA5);
    {
        TurnAngleResult turned = TurnAngleToward();

        turn_angle_exit_shim(turned);
    }
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    WaveAccumulate12DC(slot);
}

void abi_SteerAndBob12DC(void)
{
    SteerAndBob12DC(cpu.x);
}

/*
 * @routine 101E:1A2 SteerAndBob101E
 * @run 71:C9A2
 * WHAT IT DOES: Turns OBJ_WORK_D toward angle $40 and adds a cosine bob into OBJ_WORK_E-F; each cycle restarts it with OBJ_WORK_J $80.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SteerAndBob101E(uint8_t slot)
{
    lda(0x40);
    WR8(ZP->temp02, cpu.a);
    lda(0x08);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xC9B2);
    {
        TurnAngleResult turned = TurnAngleToward();

        turn_angle_exit_shim(turned);
    }
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(0x02);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    jsr_push(0xC9C3);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(RD8(OBJ->work_f[slot]));
    add_a(rd8(zProduct1), false);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    add_a(rd8(zProduct2), (cpu.p & P_C) != 0);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    add_a(0x04, false);
    WR8(OBJ->work_g[slot], cpu.a);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    WR8(OBJ->work_e[slot], 0);
    WR8(OBJ->work_f[slot], 0);
    lda(0x80);
    WR8(OBJ->work_j[slot], cpu.a);
}

void abi_SteerAndBob101E(void)
{
    SteerAndBob101E(cpu.x);
}

/*
 * @routine 12DB:2F4 SpiralDown12DB
 * @run 70:B2F4
 * WHAT IT DOES: Circles at half the Y speed while OBJ_WORK_D opens to $8E; off the left, placed at Y $50 for the next state.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpiralDown12DB(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, 0x8E);
    if (cpu.a < 0x8E) {
        lda(RD8(OBJ->work_d[slot]));
        add_a(0x03, false);
        WR8(OBJ->work_d[slot], cpu.a);
    }
    lda(0x80);
    WR8(ZP->temp00, cpu.a);
    lda(0x03);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    velocity_from_a(slot, 0xB312);
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
    move(slot, 0xB31F);
    lda(RD8(OBJ->x_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->x_lo[slot]));
    cmp(cpu.a, 0x10);
    if (cpu.a >= 0x10) {
        return;
    }
    WR8(OBJ->frame[slot], 0);
    lda(0x50);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SpiralDown12DB(void)
{
    SpiralDown12DB(cpu.x);
}

/*
 * @routine 0E0B:3AF ChooseAttack0E0B
 * @run 70:B3AF
 * WHAT IT DOES: Picks the next attack: by the health and a count when the player is near in height, at random when not; 0 means state 4.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChooseAttack0E0B(uint8_t slot)
{
    jsr_push(0xB3B2);
    abi_ended(ended_of_distance(DistanceYToPlayer(slot)));
    jsr_pop();
    cmp(cpu.a, 0x30);
    if (cpu.a >= 0x30) {
        random_a(0xB3B9);
        lda((uint8_t)(cpu.a & 0x01));
        add_a(0x04, false);
    } else {
        lda(RD8(OBJ->health[slot]));
        abi_flag(P_C, (cpu.a & 0x01) != 0);
        lda((uint8_t)(cpu.a >> 1));
        lda((uint8_t)(cpu.a & 0xF0));
        WR8(ZP->temp00, cpu.a);
        lda(RD8(OBJ->work_g[slot]));
        lda((uint8_t)(cpu.a & 0x0F));
        add_a(RD8(ZP->temp00), false);
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        INC8(OBJ->work_g[slot]);
        lda(rd8((uint16_t)(0xB3F8 + cpu.y)));
        if (cpu.a == 0) {
            lda(0x04);
            WR8(OBJ->state[slot], cpu.a);
            WR8(OBJ->substate[slot], 0);
            return;
        }
    }
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xB428 + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    face_by_side(slot, COL(OBJ->facing_left), 0xB3EC);
    restart_animation(slot);
}

void abi_ChooseAttack0E0B(void)
{
    ChooseAttack0E0B(cpu.x);
}

/*
 * @routine 0EC0:44A PlatformCarry0EC0
 * @run 71:DC4A
 * WHAT IT DOES: A moving platform: carries the player at its speed; the first step on it at Y $BC starts it down.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PlatformCarry0EC0(uint8_t slot)
{
    lda(0x30);
    WR8(ZP->temp08, cpu.a);
    lda(0x16);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xDC55, 0xAA35);
    if ((cpu.p & P_C) == 0) {
        lda(RD8(OBJ->work_d[slot]));
        if (cpu.a != 0) {
            lda(0x08);
            WR8(OBJ->work_e[slot], cpu.a);
        }
        WR8(OBJ->work_d[slot], 0);
        return;
    }
    lda(rd8(zPlayerPush));
    lda((uint8_t)(cpu.a | 0x10));
    lda((uint8_t)(cpu.a & 0xF9));
    wr8(zPlayerPush, cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    wr8(zPushXLo, cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    wr8(zPushXHi, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->y_lo[slot]));
        cmp(cpu.a, 0xBC);
        if (cpu.a == 0xBC) {
            lda(0x01);
            WR8(OBJ->vy_hi[slot], cpu.a);
            lda(0x40);
            WR8(OBJ->vy_lo[slot], cpu.a);
        }
    }
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_PlatformCarry0EC0(void)
{
    PlatformCarry0EC0(cpu.x);
}

/*
 * @routine 161F:5DE BreakableDoor161F
 * @run 71:D5DE
 * WHAT IT DOES: A door that first needs a hit with kind 7 in slot 5 (PlayerKind7Caught set); then a weak hit takes the route in the start argument.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BreakableDoor161F(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->hit_taken[slot]));
        if (cpu.a == 0) {
            return;
        }
        lda(RD8(OBJ->kind[5]));
        cmp(cpu.a, 0x07);
        if (cpu.a != 0x07) {
            return;
        }
        lda(0x01);
        wr8(PlayerKind7Caught, cpu.a);
        INC8(OBJ->work_f[slot]);
    }
    lda(rd8(StageDoorHeld));
    lda((uint8_t)(cpu.a ^ 0x01));
    lda((uint8_t)(cpu.a & rd8(PlayerDoorReady)));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    if (cpu.a == 0) {
        return;
    }
    cmp(cpu.a, 0x03);
    if (cpu.a >= 0x03) {
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    lda((uint8_t)(cpu.a & 0x0F));
    lda((uint8_t)(cpu.a + 1));
    WR8(ZP->field8c, cpu.a);
    lda(0x03);
    WR8(ZP->sub_step, cpu.a);
    sound(0x85, 0xD61B);
    INC8(OBJ->state[slot]);
    WR8(OBJ->work_e[slot], 0);
    lda(0x04);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_BreakableDoor161F(void)
{
    BreakableDoor161F(cpu.x);
}

/*
 * @routine 0B4D:03B WaterByLeader0B4D
 * @run 71:C03B
 * WHAT IT DOES: Rides $80 above slot $10 for OBJ_WORK_D counts, flickering, waving the water line; then the line off and cleared.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WaterByLeader0B4D(uint8_t slot)
{
    ldy(0x10);
    jsr_push(0xC040);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8(OBJ->y_lo[slot]));
    add_a((uint8_t)~0x80, true);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(RD8(OBJ->facing_left[0x10]));
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC085 + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC088 + cpu.y)));
    wr8(LevelYHi, cpu.a);
    lda(rd8((uint16_t)(0xC087 + cpu.y)));
    wr8(RasterWaterWanted, cpu.a);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    wr8(LevelYHi, 0);
    wr8(RasterWaterWanted, 0);
    ClearObject(slot);
}

void abi_WaterByLeader0B4D(void)
{
    WaterByLeader0B4D(cpu.x);
}

/*
 * @routine 0E0B:472 ListThenLeapByDistance0E0B
 * @run 70:B472
 * WHAT IT DOES: At the list's end sets the raster effect and leaps at speeds by the distance to the player.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenLeapByDistance0E0B(uint8_t slot)
{
    step_list(slot, 0xB4BC, true, 0xB47D);
    if (!list_ended()) {
        return;
    }
    lda(0x0B);
    WR8(ZP->raster_effect, cpu.a);
    lda(0xBF);
    WR8(ZP->raster_arg, cpu.a);
    near_x(slot, 0xB48A);
    for (int i = 0; i < 5; i++) {
        abi_flag(P_C, (cpu.a & 0x01) != 0);
        lda((uint8_t)(cpu.a >> 1));
    }
    word_index((uint8_t)(cpu.a << 1));
    lda(rd8((uint16_t)(0xB4CA + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB4C9 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xB4CC + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB4CB + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    restart_animation(slot);
    INC8(OBJ->substate[slot]);
    lda(RD8(OBJ->facing_left[slot]));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(0xB4BB);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
}

void abi_ListThenLeapByDistance0E0B(void)
{
    ListThenLeapByDistance0E0B(cpu.x);
}

/*
 * @routine 12DD:6F9 FlyOverWater12DD
 * @run 71:C6F9
 * WHAT IT DOES: Crosses down the screen with a raster effect, cleared below.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FlyOverWater12DD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC701);
        lda(0x04);
        WR8(OBJ->vx_hi[slot], cpu.a);
        move_x(slot, 0xC709);
        lda(0xFF);
        WR8(OBJ->y_hi[slot], cpu.a);
        lda(0x60);
        WR8(OBJ->y_lo[slot], cpu.a);
        lda(0x03);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(0x00);
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(0x0B);
        WR8(ZP->raster_effect, cpu.a);
        lda(0xBF);
        WR8(ZP->raster_arg, cpu.a);
        return;
    }
    step_list(slot, 0xC743, true, 0xC731);
    move_y(slot, 0xC734);
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0x02);
    if (cpu.a != 0x02) {
        return;
    }
    WR8(ZP->raster_effect, 0);
    WR8(ZP->raster_arg, 0);
    clear_this(slot, 0xC742);
}

void abi_FlyOverWater12DD(void)
{
    FlyOverWater12DD(cpu.x);
}

/*
 * @routine 072C:43F RiseTrailingDebris072C
 * @run 70:BC3F
 * WHAT IT DOES: Rises with debris every fourth count until falling; then not hit-tested and substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RiseTrailingDebris072C(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a == 0) {
        sound(0x23, 0xBC4B);
        cpu.a = 0;
        call_routine(0xBC4F, 0xA90B);
    }
    step_list(slot, 0xBC8A, true, 0xBC5A);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x2C, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xBC6E);
    lda(RD8(OBJ->vy_hi[slot]));
    if ((cpu.p & P_N) != 0) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    lda(0x07);
    WR8(OBJ->work_i[slot], cpu.a);
    restart_animation(slot);
    lda(0x16);
    WR8(OBJ->work_j[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_RiseTrailingDebris072C(void)
{
    RiseTrailingDebris072C(cpu.x);
}

/*
 * @routine 0E0B:302 WalkTurnOrStop0E0B
 * @run 70:B302
 * WHAT IT DOES: Walks; off the ground stops in state 3; at a wall turns round (carry set).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkTurnOrStop0E0B(uint8_t slot)
{
    move_x(slot, 0xB305);
    lda(0x04);
    WR8(ZP->product.mid, cpu.a);
    lda(0x01);
    jsr_push(0xB30E);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a == 0) {
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        abi_flag(P_C, false);
        return;
    }
    lda(0x04);
    WR8(ZP->product.mid, cpu.a);
    lda(0x22);
    jsr_push(0xB31B);
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
        abi_flag(P_C, false);
        return;
    }
    jsr_push(0xB335);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    move_x(slot, 0xB338);
    abi_flag(P_C, true);
}

void abi_WalkTurnOrStop0E0B(void)
{
    WalkTurnOrStop0E0B(cpu.x);
}

/*
 * @routine 0EBD:1D8 AmbushFromBehind0EBD
 * @run 71:C1D8
 * WHAT IT DOES: Far from the player and on one side, becomes kind $38 $34 to the right; on the other side goes back a state.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AmbushFromBehind0EBD(uint8_t slot)
{
    lda(rd8(zGameSubMode));
    cmp(cpu.a, 0x03);
    if (cpu.a == 0x03) {
        return;
    }
    jsr_push(0xC1E2);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a == 0) {
        near_x(slot, 0xC21B);
        cmp(cpu.a, 0x78);
        if (cpu.a < 0x78) {
            return;
        }
        abi_nz(DEC8(OBJ->state[slot]));
        return;
    }
    near_x(slot, 0xC1E9);
    cmp(cpu.a, 0x78);
    if (cpu.a < 0x78) {
        return;
    }
    lda(0x10);
    WR8_IDX((uint16_t)(COL(OBJ->work_d) - 1), slot, cpu.a);
    lda(0x38);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    lda(RD8(OBJ->x_lo[slot]));
    add_a(0x34, false);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0x00);
    add_a(RD8(OBJ->x_hi[slot]), (cpu.p & P_C) != 0);
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(0x60);
    WR8(OBJ->loop_flags[slot], cpu.a);
    run_routine(0xCC24);
}

void abi_AmbushFromBehind0EBD(void)
{
    AmbushFromBehind0EBD(cpu.x);
}

/*
 * @routine 13FB:5EF LeapUntilLand13FB
 * @run 70:B5EF
 * WHAT IT DOES: Falls until it lands at Y $A8; then hit-tested, turned round, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LeapUntilLand13FB(uint8_t slot)
{
    step_list(slot, 0xB63A, true, 0xB5FA);
    call_routine(0xB5FD, 0xB63F);
    move(slot, 0xB600);
    lda(0x04);
    WR8(ZP->product.mid, cpu.a);
    lda(0x22);
    jsr_push(0xB609);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x80, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    if ((cpu.p & P_N) != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xA8);
    if (cpu.a < 0xA8) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    lda((uint8_t)(cpu.a ^ 0x01));
    WR8(OBJ->facing_left[slot], cpu.a);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LeapUntilLand13FB(void)
{
    LeapUntilLand13FB(cpu.x);
}

/*
 * @routine 161C:19E BreakIntoSix161C
 * @run 70:B99E
 * WHAT IT DOES: Replaced by six kind $37 pieces, each with its waits and speeds.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BreakIntoSix161C(uint8_t slot)
{
    cpu.y = 0;
    do {
        wr8(zHitSlot, cpu.y);
        if (!find_from30(0xB9A4)) {
            break;
        }
        start_at_place(0x37, 0xB9AB);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        wr8(zSpreadSource, cpu.x);
        lda(rd8(zHitSlot));
        for (int i = 0; i < 3; i++) {
            abi_flag(P_C, (cpu.a & 0x80) != 0);
            lda((uint8_t)(cpu.a << 1));
        }
        cpu.x = cpu.a;
        abi_nz(cpu.x);
        lda(rd8((uint16_t)(0xB9E9 + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB9EA + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB9EB + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB9EC + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB9ED + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB9EE + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        cpu.x = rd8(zSpreadSource);
        abi_nz(cpu.x);
        ldy(rd8(zHitSlot));
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x06);
    } while (cpu.y < 0x06);
    ClearObject(slot);
}

void abi_BreakIntoSix161C(void)
{
    BreakIntoSix161C(cpu.x);
}

/*
 * @routine 161F:21E CrushingPlatform161F
 * @run 71:D21E
 * WHAT IT DOES: Stood on, sinks with the player, one every other frame; left, springs back to its height.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CrushingPlatform161F(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xD226);
        lda(0x26);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    lda(0x11);
    WR8(ZP->temp08, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp09, cpu.a);
    call_routine(0xD238, 0xAA35);
    if ((cpu.p & P_C) != 0) {
        lda(0x01);
        WR8(OBJ->work_e[slot], cpu.a);
        lda(rd8(zPlayerPush));
        lda((uint8_t)(cpu.a | 0x20));
        wr8(zPlayerPush, cpu.a);
        lda(rd8(zWaveCounter));
        lda((uint8_t)(cpu.a & 0x01));
        if (cpu.a != 0) {
            return;
        }
        lda(0x01);
        wr8(zPushYHi, cpu.a);
        abi_nz(DEC8(OBJ->work_d[slot]));
        return;
    }
    jsr_push(0xD256);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x02));
    if (cpu.a != 0) {
        lda(rd8(zPlayerPush));
        lda((uint8_t)(cpu.a & 0xF9));
        wr8(zPlayerPush, cpu.a);
    }
    lda(0x26);
    WR8(OBJ->work_d[slot], cpu.a);
    WR8(OBJ->work_e[slot], 0);
}

void abi_CrushingPlatform161F(void)
{
    CrushingPlatform161F(cpu.x);
}

/*
 * @routine 0E0B:120 OpenArena0E0B
 * @run 70:B120
 * WHAT IT DOES: Entered with the raster effect in A: holds the scroll at X $280; there, $80 counts, the CD track again, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void OpenArena0E0B(uint8_t slot)
{
    wr8(zRasterEffect, cpu.a);
    lda(0xBF);
    wr8(zRasterArg, cpu.a);
    lda(0x02);
    wr8(ScrollXMaxHi, cpu.a);
    lda(0x80);
    wr8(ScrollXMaxLo, cpu.a);
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a == 0) {
        lda(rd8(ScrollXHiScreen));
        cmp(cpu.a, 0x02);
        if (cpu.a != 0x02) {
            return;
        }
        lda(0x80);
        WR8(OBJ->work_d[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    jsr_push(0xB151);
    play_track_exit_shim(CdPlayAgain(cpu.x, cpu.y));
    jsr_pop();
    lda(0x48);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0xFF);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0xC0);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->frame[slot], cpu.a);
    INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
}

void abi_OpenArena0E0B(void)
{
    OpenArena0E0B(cpu.x);
}

/*
 * @routine 117E:38B ChooseState117E
 * @run 71:CB8B
 * WHAT IT DOES: Rushes (state 9) when slot 0 is in substate 1 and the player is far; else after OBJ_WORK_F counts picks the next state.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChooseState117E(uint8_t slot)
{
    lda(RD8(OBJ->substate[0]));
    cmp(cpu.a, 0x01);
    if (cpu.a == 0x01) {
        near_x(slot, 0xCB95);
        cmp(cpu.a, 0x50);
        if (cpu.a >= 0x50) {
            jsr_push(0xCB9C);
            distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
            jsr_pop();
            lda((uint8_t)(cpu.a & 0x01));
            if (cpu.a == 0) {
                restart_animation(slot);
                lda(0x09);
                WR8(OBJ->state[slot], cpu.a);
                return;
            }
        }
    }
    abi_nz(DEC8(OBJ->work_f[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    restart_animation(slot);
    jsr_push(0xCBBA);
    distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xCBE2 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->work_m[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xCBD7 + cpu.y)));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xCBDF + cpu.y)));
    WR8(OBJ->state[slot], cpu.a);
    abi_nz(INC8(OBJ->work_m[slot]));
}

void abi_ChooseState117E(void)
{
    ChooseState117E(cpu.x);
}
