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

/* BNE after StepObjectFrames: true when the list ended. */
static bool list_ended(void)
{
    return (cpu.p & P_Z) != 0;
}
static void move(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
}
/* LDA #list, LDX #slot, JSR StartListChannelKind2. */
static void list_channel2(uint8_t number, uint8_t at_slot, uint16_t ret)
{
    lda(number);
    cpu.x = at_slot;
    abi_nz(at_slot);
    jsr_push(ret);
    list_channel_exit_shim(StartListChannelKind2(cpu.a, cpu.x));
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

static void script_at(uint8_t number, uint8_t at_slot, uint16_t ret)
{
    lda(number);
    cpu.x = at_slot;
    abi_nz(at_slot);
    jsr_push(ret);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
}

static void pull_x(void)
{
    cpu.x = pull8();
    abi_nz(cpu.x);
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
/* A tail JMP to LoadObjectRecord's vector. */
static void load_record_tail_(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
}

/*
 * @routine 12DC:50A FaceIfThenAttack12DC
 * @run 70:BD0A
 * WHAT IT DOES: Entered with Z set faces the player first; runs $BCDB; with carry clear and the player at least $70 across, state 2; the rest is FaceByBitThenNear12DC's code.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FaceIfThenAttack12DC(uint8_t slot)
{
    if ((cpu.p & P_Z) != 0) {
        face_by_side(slot, COL(OBJ->facing_left), 0xBD0F);
    }
    call_routine(0xBD17, 0xBCDB);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    near_x(slot, 0xBD1C);
    cmp(cpu.a, 0x70);
    if (cpu.a < 0x70) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_FaceIfThenAttack12DC(void)
{
    FaceIfThenAttack12DC(cpu.x);
}

/*
 * @routine 101F:007 StartMarkerLeft101F
 * @run 71:D007
 * WHAT IT DOES: Entered with Z clear: starts kind $BB two slots below, $50 to the left.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartMarkerLeft101F(uint8_t slot)
{
    if ((cpu.p & P_Z) != 0) {
        return;
    }
    lda(cpu.x);
    lda((uint8_t)(cpu.a - 1));
    lda((uint8_t)(cpu.a - 1));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    clear_slot_in_y(0xD010);
    start_at_place(0xBB, 0xD015);
    lda(RD8(OBJ->x_lo[cpu.y]));
    add_a((uint8_t)~0x50, true);
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    lda(RD8(OBJ->x_hi[cpu.y]));
    add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->x_hi[cpu.y], cpu.a);
}

void abi_StartMarkerLeft101F(void)
{
    StartMarkerLeft101F(cpu.x);
}

/*
 * @routine 161E:6B8 PushWithin161E
 * @run 71:CEB8
 * WHAT IT DOES: Within $14 across and touching: pushes the player along, X by the start argument, Y by the object's speed.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PushWithin161E(uint8_t slot)
{
    near_x(slot, 0xCEBB);
    cmp(cpu.a, 0x14);
    if (cpu.a >= 0x14) {
        return;
    }
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xCEEC + cpu.y)));
    add_a(0x12, false);
    WR8(ZP->temp09, cpu.a);
    lda(0x17);
    WR8(ZP->temp08, cpu.a);
    call_routine(0xCECF, 0xAA35);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(rd8(zPlayerPush));
    lda((uint8_t)(cpu.a | 0x30));
    wr8(zPlayerPush, cpu.a);
    ldy(RD8(OBJ->start_arg[slot]));
    lda(rd8((uint16_t)(0xCEEA + cpu.y)));
    wr8(zPushXHi, cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    wr8(zPushYHi, cpu.a);
    lda(RD8(OBJ->vy_lo[slot]));
    wr8(zPushYLo, cpu.a);
}

void abi_PushWithin161E(void)
{
    PushWithin161E(cpu.x);
}

/*
 * @routine 18DD:1AD TakeFromPool18DD
 * @run 71:C1AD
 * WHAT IT DOES: Takes one marked entry (bit 7) of the 112 at $3F90, 16 random tries and then a search; with none left a count of $40 in OBJ_WORK_E, the raster effect off, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TakeFromPool18DD(uint8_t slot)
{
    lda(0x10);
    WR8(ZP->temp00, cpu.a);
    do {
        do {
            random_a(0xC1B4);
            lda((uint8_t)(cpu.a & 0x7F));
            cmp(cpu.a, 0x70);
        } while (cpu.a >= 0x70);
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0x3F90 + cpu.y)));
        if ((cpu.p & P_N) != 0) {
            goto take;
        }
        abi_nz(DEC8(ZP->temp00));
    } while ((cpu.p & P_Z) == 0);
    cpu.y = 0;
    do {
        lda(rd8((uint16_t)(0x3F90 + cpu.y)));
        if ((cpu.p & P_N) != 0) {
            goto take;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x70);
    } while (cpu.y != 0x70);
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    wr8(zRasterEffect, 0);
    abi_nz(INC8(OBJ->state[slot]));
    return;
take:
    cpu.a = 0;
    wr8((uint16_t)(0x3F90 + cpu.y), cpu.a);
}

void abi_TakeFromPool18DD(void)
{
    TakeFromPool18DD(cpu.x);
}

/*
 * @routine 101F:166 WakeOnRoute101F
 * @run 71:D166
 * WHAT IT DOES: State 1 waits for the route to be set: then list channel $83, script $87; the wave's phase is cleared otherwise and after.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WakeOnRoute101F(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    lda((uint8_t)(cpu.a - 1));
    if (cpu.a == 0) {
        lda(rd8(zField8C));
        if (cpu.a == 0) {
            return;
        }
        lda(rd8(CameraWavePhase));
        if (cpu.a != 0) {
            return;
        }
        lda(RD8(OBJ->kind[slot]));
        lda((uint8_t)(cpu.a | 0x80));
        WR8(OBJ->kind[slot], cpu.a);
        INC8(OBJ->state[slot]);
        lda(0x83);
        push8(cpu.x);
        list_channel2(cpu.a, 0x06, 0xD18D);
        script_at(0x87, 0x05, 0xD194);
        pull_x();
    }
    wr8(CameraWavePhase, 0);
}

void abi_WakeOnRoute101F(void)
{
    WakeOnRoute101F(cpu.x);
}

/*
 * @routine 0E0C:4FA ListThenScatter0E0C
 * @run 70:BCFA
 * WHAT IT DOES: At the end of the list at $BD2F starts the pieces, sound $25, and goes on, still and not hit-tested.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenScatter0E0C(uint8_t slot)
{
    step_list(slot, 0xBD2F, true, 0xBD05);
    if (!list_ended()) {
        return;
    }
    call_routine(0xBD0A, 0xBD6E);
    sound(0x25, 0xBD0F);
    WR8(OBJ->hitbox_on[slot], 0);
    restart_animation(slot);
    lda(RD8(OBJ->sprite_attr[slot]));
    lda((uint8_t)(cpu.a | 0x10));
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(0x60);
    WR8(OBJ->work_e[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListThenScatter0E0C(void)
{
    ListThenScatter0E0C(cpu.x);
}

/*
 * @routine 12DD:073 OrbitByTrail12DD
 * @run 71:C073
 * WHAT IT DOES: Runs $C0B3, then out at the angle the trail holds at this piece's lag, runs $C0FF; slot $10's OBJ_WORK_M counts the state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void OrbitByTrail12DD(uint8_t slot)
{
    call_routine(0xC076, 0xC0B3);
    ldy(RD8(OBJ->work_d[slot]));
    lda(rd8((uint16_t)(0xC0A9 + cpu.y)));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    WR8(ZP->temp00, 0);
    lda(RD8(OBJ->work_i[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(rd8((uint16_t)(0xBA41 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    velocity_from_a(slot, 0xC08D);
    move(slot, 0xC090);
    lda(0x08);
    WR8(ZP->temp00, cpu.a);
    lda(0xFF);
    WR8(ZP->temp01, cpu.a);
    call_routine(0xC09B, 0xC0FF);
    lda(RD8(OBJ->work_m[0x10]));
    if (cpu.a == 0) {
        return;
    }
    lda(0x18);
    WR8(OBJ->work_h[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_OrbitByTrail12DD(void)
{
    OrbitByTrail12DD(cpu.x);
}

/*
 * @routine 161E:07F BoxByPreviousFrame161E
 * @run 71:C87F
 * WHAT IT DOES: Takes the previous slot's place and facing, and its hit box by that slot's frame (from $57).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BoxByPreviousFrame161E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    jsr_push(0xC88D);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8_IDX((uint16_t)(COL(OBJ->facing_left) - 1), slot));
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(RD8_IDX((uint16_t)(COL(OBJ->frame) - 1), slot));
    add_a((uint8_t)~0x57, true);
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC8B5 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xC8B6 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xC8B7 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xC8B8 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_BoxByPreviousFrame161E(void)
{
    BoxByPreviousFrame161E(cpu.x);
}

/*
 * @routine 0EBB:74C TumbleThenGone0EBB
 * @run 70:B74C
 * WHAT IT DOES: First knocked up with its X speed halved; then $A92E each count until OBJ_WORK_L runs out, and cleared.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TumbleThenGone0EBB(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        uint8_t old;

        call_routine(0xB754, 0xA92E);
        WR8(OBJ->hitbox_on[slot], 0);
        lda(RD8(OBJ->vy_lo[slot]));
        add_a(0x00, false);
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(RD8(OBJ->vy_hi[slot]));
        add_a(0x03, (cpu.p & P_C) != 0);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(RD8(OBJ->vx_hi[slot]));
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        old = RD8(OBJ->vx_hi[slot]);
        WR8(OBJ->vx_hi[slot], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_flag(P_C, (old & 0x01) != 0);
        old = RD8(OBJ->vx_lo[slot]);
        WR8(OBJ->vx_lo[slot], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_nz((uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_flag(P_C, (old & 0x01) != 0);
        lda(0x18);
        WR8(OBJ->work_l[slot], cpu.a);
        return;
    }
    abi_nz(DEC8(OBJ->work_l[slot]));
    if ((cpu.p & P_Z) != 0) {
        ClearObject(slot);
        return;
    }
    run_routine(0xA92E);
}

void abi_TumbleThenGone0EBB(void)
{
    TumbleThenGone0EBB(cpu.x);
}

/*
 * @routine 117F:584 ChargeList117F
 * @run 71:D584
 * WHAT IT DOES: Runs the list at $D5BD; on each new frame moves at the facing's speed (sounds at step 9); at its end substate 2.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChargeList117F(uint8_t slot)
{
    step_list(slot, 0xD5BD, true, 0xD58F);
    if (list_ended()) {
        lda(0x40);
        WR8(OBJ->work_d[slot], cpu.a);
        lda(0x02);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x09);
    if (cpu.a == 0x09) {
        sound(0xE0, 0xD59F);
        sound(0x07, 0xD5A4);
    }
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xD5BB + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    run_routine(0xD46F);
}

void abi_ChargeList117F(void)
{
    ChargeList117F(cpu.x);
}

/*
 * @routine 12DE:458 StrikeList12DE
 * @run 71:CC58
 * WHAT IT DOES: Runs the list at $CC8F, sound $1A at step 4; at its end, or at frame $75 with slot $11 free to, state 3.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StrikeList12DE(uint8_t slot)
{
    step_list(slot, 0xCC8F, true, 0xCC63);
    if (!list_ended()) {
        if ((cpu.p & P_C) == 0) {
            return;
        }
        lda(RD8(OBJ->anim_step[slot]));
        cmp(cpu.a, 0x04);
        if (cpu.a == 0x04) {
            sound(0x1A, 0xCC73);
            return;
        }
        lda(RD8(OBJ->frame[slot]));
        cmp(cpu.a, 0x75);
        if (cpu.a != 0x75) {
            return;
        }
        lda(RD8(OBJ->work_d[0x11]));
        if (cpu.a != 0) {
            return;
        }
    }
    restart_animation(slot);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_StrikeList12DE(void)
{
    StrikeList12DE(cpu.x);
}

/*
 * @routine 16CC:0CA RiseListSlowing16CC
 * @run 70:B8CA
 * WHAT IT DOES: Moves along Y taking $10 off the speed; when it turns negative: sound $8F, a wait of $40, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RiseListSlowing16CC(uint8_t slot)
{
    call_routine(0xB8CD, 0xB912);
    step_list(slot, 0xB901, true, 0xB8D8);
    move_y(slot, 0xB8DB);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->vy_lo[slot]));
    add_a((uint8_t)~0x10, true);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    sound(0x8F, 0xB8F8);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_RiseListSlowing16CC(void)
{
    RiseListSlowing16CC(cpu.x);
}

/*
 * @routine 18DB:3B1 SpitByStep18DB
 * @run 70:B3B1
 * WHAT IT DOES: At animation step 7 starts kind $27 in slot $33; at step 8 sound $26 and kind $28 in slots $30-$32.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpitByStep18DB(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x07);
    if (cpu.a == 0x07) {
        ldy(0x33);
        clear_slot_in_y(0xB3DC);
        start_at_place(0x27, 0xB3E1);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        return;
    }
    cmp(cpu.a, 0x08);
    if (cpu.a != 0x08) {
        return;
    }
    sound(0x26, 0xB3C1);
    ldy(0x30);
    do {
        clear_slot_in_y(0xB3C6);
        start_at_place(0x28, 0xB3CB);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x33);
    } while (cpu.y < 0x33);
}

void abi_SpitByStep18DB(void)
{
    SpitByStep18DB(cpu.x);
}

/*
 * @routine 088C:070 LoadSpeedThenFall088C
 * @run 70:B870
 * WHAT IT DOES: First loads with the speeds OBJ_WORK_D picks; then animates, falling without limit.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadSpeedThenFall088C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xB878);
        lda(RD8(OBJ->work_d[slot]));
        word_index((uint8_t)(cpu.a << 1));
        lda(rd8((uint16_t)(0xB8A9 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xB8A8 + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(rd8((uint16_t)(0xB8AB + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xB8AA + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        return;
    }
    step_list(slot, 0xB8E8, false, 0xB8A2);
    jsr_push(0xB8A5);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_LoadSpeedThenFall088C(void)
{
    LoadSpeedThenFall088C(cpu.x);
}

/*
 * @routine 0E0C:689 BounceOnFloor0E0C
 * @run 70:BE89
 * WHAT IT DOES: Every other frame falls; on the floor at Y $B9 bounces three times, then the state goes on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BounceOnFloor0E0C(uint8_t slot)
{
    call_routine(0xBE8C, 0xBEC1);
    lda(rd8(zWaveCounter));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        return;
    }
    gravity(slot, 0xBE95);
    move(slot, 0xBE98);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xB9);
    if (cpu.a < 0xB9) {
        return;
    }
    lda(0xB9);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, 0x03);
    if (cpu.a >= 0x03) {
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBEE7 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBEE6 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->work_d[slot]));
}

void abi_BounceOnFloor0E0C(void)
{
    BounceOnFloor0E0C(cpu.x);
}

/*
 * @routine 0F6C:53E ChooseNextAct0F6C
 * @run 70:BD3E
 * WHAT IT DOES: Runs the overlay's checks and list; at the list's end goes to $BC84; on marked animation steps runs $C377.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChooseNextAct0F6C(uint8_t slot)
{
    call_routine(0xBD41, 0xBEF7);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    call_routine(0xBD47, 0xBD80);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    call_routine(0xBD4D, 0xC365);
    if (list_ended()) {
        run_routine(0xBC84);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    ldy(RD8(OBJ->anim_step[slot]));
    lda(rd8((uint16_t)(0xBD60 + cpu.y)));
    if (cpu.a == 0) {
        return;
    }
    call_routine(0xBD5C, 0xC377);
}

void abi_ChooseNextAct0F6C(void)
{
    ChooseNextAct0F6C(cpu.x);
}
