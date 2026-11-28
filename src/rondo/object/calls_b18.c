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
#include "calls_b18.h"

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

/*
 * @routine 18DB:260 StopMusicOnStep18DB
 * @run 70:B260
 * WHAT IT DOES: At slot $18's animation step 7: list channel $81, the sound stopped, the CD track again, state on; before it, attribute $10.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StopMusicOnStep18DB(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[0x18]));
    if (cpu.a == 0) {
        return;
    }
    cmp(cpu.a, 0x07);
    if (cpu.a != 0x07) {
        lda(0x10);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        return;
    }
    WR8(OBJ->frame[slot], 0);
    push8(cpu.x);
    list_channel2(0x81, 0x05, 0xB27A);
    RMW8(ZP->field8e, old, old | 0x10);
    sound(0xFE, 0xB281);
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    jsr_push(0xB288);
    play_track_exit_shim(CdPlayAgain(cpu.x, cpu.y));
    jsr_pop();
    pull_x();
    lda(RD8(ZP->field8e));
    lda((uint8_t)(cpu.a | 0x04));
    WR8(ZP->field8e, cpu.a);
    INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
}

void abi_StopMusicOnStep18DB(void)
{
    StopMusicOnStep18DB(cpu.x);
}

/*
 * @routine 0EBD:635 ArcAndSpin0EBD
 * @run 71:C635
 * WHAT IT DOES: Moves out at angle OBJ_WORK_P, which turns by $60/256 each count; at $F0 cleared.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ArcAndSpin0EBD(uint8_t slot)
{
    step_list(slot, 0xC52D, true, 0xC640);
    lda(0x80);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_p[slot]));
    velocity_from_a(slot, 0xC64E);
    lda(RD8(OBJ->work_o[slot]));
    add_a(0x60, false);
    WR8(OBJ->work_o[slot], cpu.a);
    lda(RD8(OBJ->work_p[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->work_p[slot], cpu.a);
    cmp(cpu.a, 0xF0);
    if (cpu.a >= 0xF0) {
        ClearObject(slot);
        return;
    }
    move_x(slot, 0xC669);
    move_y(slot, 0xC66C);
}

void abi_ArcAndSpin0EBD(void)
{
    ArcAndSpin0EBD(cpu.x);
}

/*
 * @routine 117D:4AD KnockedLoose117D
 * @run 71:C4AD
 * WHAT IT DOES: Hit outside state 5: not hit-tested, the previous slot cleared, knocked upward, state 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void KnockedLoose117D(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a == 0x05) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xC4C4);
    restart_animation(slot);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_KnockedLoose117D(void)
{
    KnockedLoose117D(cpu.x);
}

/*
 * @routine 117E:0CF AccelerateToSpeed117E
 * @run 71:C8CF
 * WHAT IT DOES: Animates and moves, adding to the X speed until it reaches the target for OBJ_WORK_D; then the state goes on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AccelerateToSpeed117E(uint8_t slot)
{
    step_list(slot, 0xC8BE, false, 0xC8DA);
    move(slot, 0xC8DD);
    lda(RD8(OBJ->work_d[slot]));
    word_index((uint8_t)(cpu.a << 1));
    lda(RD8(OBJ->vx_lo[slot]));
    add_a(rd8((uint16_t)(0xC907 + cpu.y)), false);
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    add_a(rd8((uint16_t)(0xC908 + cpu.y)), (cpu.p & P_C) != 0);
    WR8(OBJ->vx_hi[slot], cpu.a);
    cmp(cpu.a, rd8((uint16_t)(0xC90A + cpu.y)));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(RD8(OBJ->vx_lo[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xC909 + cpu.y)));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_AccelerateToSpeed117E(void)
{
    AccelerateToSpeed117E(cpu.x);
}

/*
 * @routine 117F:500 ChargeUntilListEnds117F
 * @run 71:D500
 * WHAT IT DOES: Runs the list at $D538, moving at the facing's speed on each new frame (sounds at step 8); at its end substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChargeUntilListEnds117F(uint8_t slot)
{
    step_list(slot, 0xD538, true, 0xD50B);
    if (list_ended()) {
        lda(0x40);
        WR8(OBJ->work_d[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x08);
    if (cpu.a == 0x08) {
        sound(0xE0, 0xD51E);
        sound(0x1A, 0xD523);
    }
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xD551 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    run_routine(0xD46F);
}

void abi_ChargeUntilListEnds117F(void)
{
    ChargeUntilListEnds117F(cpu.x);
}

/*
 * @routine 12DE:122 TurnQuarterAlt12DE
 * @run 71:C922
 * WHAT IT DOES: TurnQuarter12DE with OBJ_WORK_G as the speed's low byte (zTemp00): the entry five bytes before it.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnQuarterAlt12DE(uint8_t slot)
{
    lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, cpu.a);
    TurnQuarter12DE(slot);
}

void abi_TurnQuarterAlt12DE(void)
{
    TurnQuarterAlt12DE(cpu.x);
}

/*
 * @routine 161B:2A1 BobAndCount161B
 * @run 70:B2A1
 * WHAT IT DOES: Bobs by the cosine of OBJ_WORK_D; after OBJ_WORK_E counts the state goes back.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BobAndCount161B(uint8_t slot)
{
    step_list(slot, 0xB2D9, true, 0xB2AC);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xB2BA);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct2));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8(zProduct1));
    WR8(OBJ->vy_lo[slot], cpu.a);
    move_y(slot, 0xB2C7);
    lda(RD8(OBJ->work_d[slot]));
    add_a(0x02, false);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    abi_nz(DEC8(OBJ->state[slot]));
}

void abi_BobAndCount161B(void)
{
    BobAndCount161B(cpu.x);
}

/*
 * @routine 161E:277 CountDownSpawning161E
 * @run 71:CA77
 * WHAT IT DOES: Counts OBJ_WORK_E down; from $20 on, every eighth count a kind $A2 object at a height by the count; past 0, frame 5 and state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountDownSpawning161E(uint8_t slot)
{
    DEC8(OBJ->work_e[slot]);
    lda(RD8(OBJ->work_e[slot]));
    cmp(cpu.a, 0xFF);
    if (cpu.a == 0xFF) {
        lda(0x05);
        WR8(OBJ->frame[slot], cpu.a);
        INC8(OBJ->state[slot]);
        WR8(OBJ->work_e[slot], 0);
        return;
    }
    cmp(cpu.a, 0x21);
    if (cpu.a >= 0x21) {
        return;
    }
    lda((uint8_t)(cpu.a & 0x07));
    if (cpu.a != 0) {
        return;
    }
    if (!find_from30(0xCA8C)) {
        return;
    }
    start_at_place(0xA2, 0xCA93);
    lda(RD8(OBJ->work_e[slot]));
    for (int i = 0; i < 3; i++) {
        abi_flag(P_C, (cpu.a & 0x01) != 0);
        lda((uint8_t)(cpu.a >> 1));
    }
    push8(cpu.x);
    cpu.x = cpu.a;
    abi_nz(cpu.x);
    lda(rd8((uint16_t)(0xCBD4 + cpu.x)));
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    pull_x();
}

void abi_CountDownSpawning161E(void)
{
    CountDownSpawning161E(cpu.x);
}

/*
 * @routine 072C:6C4 LaunchByPattern072C
 * @run 70:BEC4
 * WHAT IT DOES: Animates, hit-tested, moving at the speeds the pattern for OBJ_WORK_D picks, then $BE56.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LaunchByPattern072C(uint8_t slot)
{
    step_list(slot, 0xBEFD, true, 0xBECF);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    ldy(RD8(OBJ->work_d[slot]));
    lda(rd8((uint16_t)(0xBE24 + cpu.y)));
    add_a(0x04, false);
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xB46E + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB52E + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xB5EE + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB6AE + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    move(slot, 0xBEF9);
    call_routine(0xBEFC, 0xBE56);
}

void abi_LaunchByPattern072C(void)
{
    LaunchByPattern072C(cpu.x);
}

/*
 * @routine 0EBE:1AE LoadAtTopOrFace0EBE
 * @run 71:C9AE
 * WHAT IT DOES: Loads; with no start argument placed at one of four spots along the top, cleared if $C9EF says so, else facing the player.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAtTopOrFace0EBE(uint8_t slot)
{
    load_record(slot, 0xC9B1);
    lda(0x50);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a != 0) {
        return;
    }
    random_a(0xC9BE);
    lda((uint8_t)(cpu.a & 0x03));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC9E8 + cpu.y)));
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC9E7 + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    WR8(OBJ->y_hi[slot], 0);
    lda(0x02);
    WR8(OBJ->y_lo[slot], cpu.a);
    call_routine(0xC9D9, 0xC9EF);
    if ((cpu.p & P_C) != 0) {
        ClearObject(slot);
        return;
    }
    face_by_side(slot, COL(OBJ->facing_left), 0xC9E1);
}

void abi_LoadAtTopOrFace0EBE(void)
{
    LoadAtTopOrFace0EBE(cpu.x);
}

/*
 * @routine 0EBE:611 TossThenBecome9B0EBE
 * @run 71:CE11
 * WHAT IT DOES: Loads moving away from or toward the player; falls animated for OBJ_WORK_E counts and then becomes kind $9B.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TossThenBecome9B0EBE(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xCE19);
        jsr_push(0xCE1C);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        lda((uint8_t)(cpu.a & 0x01));
        if (cpu.a != 0) {
            jsr_push(0xCE23);
            negate_exit_shim(NegateVelocityX(slot));
            jsr_pop();
        }
        return;
    }
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) != 0) {
        lda(0x9B);
        WR8(OBJ->kind[slot], cpu.a);
        WR8(OBJ->state[slot], 0);
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xCE4A + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xCE4B + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    jsr_push(0xCE44);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
    jsr_push(0xCE47);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_TossThenBecome9B0EBE(void)
{
    TossThenBecome9B0EBE(cpu.x);
}

/*
 * @routine 117B:4DC DoorByHit117B
 * @run 70:B4DC
 * WHAT IT DOES: A flagged door: a weak hit while it is open takes the route; start arguments other than 0 and 2 set zField82 to 3.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DoorByHit117B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xB4E4);
        lda(0x0C);
        WR8(OBJ->hitbox_half_w[slot], cpu.a);
        return;
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
    if (cpu.a != 0) {
        cmp(cpu.a, 0x02);
        if (cpu.a != 0x02) {
            lda(0x03);
            WR8(ZP->field82, cpu.a);
        }
    }
    lda(0x80);
    WR8(ZP->field8c, cpu.a);
    lda(0x03);
    WR8(ZP->sub_step, cpu.a);
}

void abi_DoorByHit117B(void)
{
    DoorByHit117B(cpu.x);
}

/*
 * @routine 117D:5FF NextSubstateByHeight117D
 * @run 71:C5FF
 * WHAT IT DOES: Far above or below: state 3 (every fourth time substate 3); near: the next substate from the list for the start argument.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void NextSubstateByHeight117D(uint8_t slot)
{
    jsr_push(0xC602);
    abi_ended(ended_of_distance(DistanceYToPlayer(slot)));
    jsr_pop();
    cmp(cpu.a, 0x18);
    if (cpu.a >= 0x18) {
        INC8(OBJ->work_i[slot]);
        lda(RD8(OBJ->work_i[slot]));
        lda((uint8_t)(cpu.a & 0x03));
        if (cpu.a == 0) {
            lda(0x03);
            WR8(OBJ->substate[slot], cpu.a);
            return;
        }
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    for (int i = 0; i < 4; i++) {
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
    }
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_i[slot]));
    lda((uint8_t)(cpu.a & 0x0F));
    add_a(RD8(ZP->temp00), false);
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC638 + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    abi_nz(INC8(OBJ->work_i[slot]));
}

void abi_NextSubstateByHeight117D(void)
{
    NextSubstateByHeight117D(cpu.x);
}
