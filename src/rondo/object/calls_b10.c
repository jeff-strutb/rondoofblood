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
#include "calls_b10.h"

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

/* A free slot looked for from $30 (or $20) and, when found, an object of a kind started in it. */
static void start_in_free_slot(bool from20, uint8_t kind, uint16_t find_ret, uint16_t start_ret)
{
    uint8_t found_a, to;
    bool found;

    jsr_push(find_ret);
    to = from20 ? FindFreeSlotFrom20(&found_a, &found) : FindFreeSlotFrom30(&found_a, &found);
    find_slot_exit_shim(to, found_a, found);
    jsr_pop();
    if (found) {
        start_at_place(kind, start_ret);
    }
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

static bool find_from20(uint16_t ret)
{
    uint8_t found_a, to;
    bool found;

    jsr_push(ret);
    to = FindFreeSlotFrom20(&found_a, &found);
    find_slot_exit_shim(to, found_a, found);
    jsr_pop();
    return found;
}

static void clear_this(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    ClearObject(slot);
    jsr_pop();
}
/* JSR FindFreeSlotFrom50's vector: true when a slot was found, in Y. */
static bool find_from50(uint16_t ret)
{
    uint8_t found_a, to;
    bool found;

    jsr_push(ret);
    to = FindFreeSlotFrom50(&found_a, &found);
    find_slot_exit_shim(to, found_a, found);
    jsr_pop();
    return found;
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

static void stop_here(uint8_t slot)
{
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
}
static void load_then(uint8_t slot, uint16_t ret) { load_record(slot, ret); }

/* LDA, ASL, TAY: a word index from an object column. */
static void word_index(uint8_t value)
{
    abi_flag(P_C, (value & 0x80) != 0);
    lda((uint8_t)(value << 1));
    cpu.y = cpu.a;
}

static void aim_at_player(uint16_t ret)
{
    AngleExit angle;

    jsr_push(ret);
    angle = AngleToPlayer(cpu.x);
    angle_exit_shim(angle);
    jsr_pop();
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
static void negate_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
}

static void record_event(uint8_t slot, uint16_t ret)
{
    uint8_t stored, event, byte;
    bool marked;

    jsr_push(ret);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    abi_ended(l8_of_event(marked, stored, event, byte));
    jsr_pop();
}

static void swap_xy(void)
{
    uint8_t keep = cpu.x;

    cpu.x = cpu.y;
    cpu.y = keep;
}
/* ROL of A through the carry. */
static void rol_a(void)
{
    bool out = (cpu.a & 0x80) != 0;

    lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
    abi_flag(P_C, out);
}

/*
 * @routine 10CC:3FA FallCountThenShoot10CC
 * @run 70:BBFA
 * WHAT IT DOES: Falls while OBJ_WORK_E counts; then sound $45 and a drop with the X speed of OBJ_WORK_O (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallCountThenShoot10CC(uint8_t slot)
{
    gravity(slot, 0xBBFD);
    move(slot, 0xBC00);
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    sound(0x45, 0xBC0A);
    lda(RD8(OBJ->work_o[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBC27 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xBC28 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_FallCountThenShoot10CC(void)
{
    FallCountThenShoot10CC(cpu.x);
}

/*
 * @routine 117F:379 LoadWithArms117F
 * @run 71:D379
 * WHAT IT DOES: Loads with a kind $35 object in the slot before and a $37 in the slot after (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadWithArms117F(uint8_t slot)
{
    load_record(slot, 0xD37C);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xD382);
    start_at_place(0x35, 0xD387);
    cpu.y = (uint8_t)(cpu.y + 1);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xD38C);
    start_at_place(0x37, 0xD391);
    lda(0x50);
    WR8(OBJ->work_e[slot], cpu.a);
    random_a(0xD399);
    WR8(OBJ->work_h[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a == 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_LoadWithArms117F(void)
{
    LoadWithArms117F(cpu.x);
}

/*
 * @routine 12DE:569 DripWhileEarly12DE
 * @run 71:CD69
 * WHAT IT DOES: In the first four animation steps, every fifth call a kind $39 object at the next of 16 X places (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DripWhileEarly12DE(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x04);
    if (cpu.a >= 0x04) {
        return;
    }
    if (!counted_out(DEC8(OBJ->work_k[slot]))) {
        return;
    }
    lda(0x05);
    WR8(OBJ->work_k[slot], cpu.a);
    if (!find_from30(0xCD7D)) {
        return;
    }
    start_at_place(0x39, 0xCD84);
    push8(cpu.x);
    lda(RD8(OBJ->work_j[slot]));
    lda((uint8_t)(cpu.a & 0x0F));
    cpu.x = cpu.a;
    lda(rd8((uint16_t)(0xCD96 + cpu.x)));
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    pull_x();
    abi_nz(INC8(OBJ->work_j[slot]));
}

void abi_DripWhileEarly12DE(void)
{
    DripWhileEarly12DE(cpu.x);
}

/*
 * @routine 13FB:5AF ListThenLeapByFacing13FB
 * @run 70:B5AF
 * WHAT IT DOES: Plays the list at $B5DC; then leaps by facing with no hit box, substate on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenLeapByFacing13FB(uint8_t slot)
{
    step_list(slot, 0xB5DC, true, 0xB5BA);
    if (!list_ended()) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    lda(RD8(OBJ->facing_left[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xB5EC + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB5EB + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0xF7);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenLeapByFacing13FB(void)
{
    ListThenLeapByFacing13FB(cpu.x);
}

/*
 * @routine 13FD:289 AfterImage13FD
 * @run 71:C289
 * WHAT IT DOES: An after-image of slot $10: shown every fourth of 24 counts with its facing and frame, then cleared (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AfterImage13FD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC291);
        lda(0x18);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    if (counted_out(DEC8(OBJ->work_d[slot]))) {
        ClearObject(slot);
        return;
    }
    WR8(OBJ->frame[slot], 0);
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->facing_left[0x10]));
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(RD8(OBJ->frame[0x10]));
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_AfterImage13FD(void)
{
    AfterImage13FD(cpu.x);
}

/*
 * @routine 16CC:1BD ListRiseFasterThenClear16CC
 * @run 70:B9BD
 * WHAT IT DOES: Rises ever faster playing the list at $B9EA, cleared once well off the screen (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListRiseFasterThenClear16CC(uint8_t slot)
{
    Sum sum;

    step_list(slot, 0xB9EA, true, 0xB9C8);
    move(slot, 0xB9CB);
    lda(RD8(OBJ->vy_lo[slot]));
    sum = sub_bytes(cpu.a, 0x20, true);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    sum = sub_bytes(cpu.a, 0x00, (cpu.p & P_C) != 0);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    if ((cpu.a & 0x80) != 0) {
        return;
    }
    ClearObject(slot);
}

void abi_ListRiseFasterThenClear16CC(void)
{
    ListRiseFasterThenClear16CC(cpu.x);
}

/*
 * @routine 18DC:21C CountThenOutburst18DC
 * @run 70:BA1C
 * WHAT IT DOES: Counts OBJ_WORK_E; then scripts, a list channel, slot $40's state on, frame $11 at Y $76 (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenOutburst18DC(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    push8(cpu.x);
    script_at(0x86, 0x05, 0xBA29);
    script_at(0x88, 0x04, 0xBA30);
    list_channel2(0x8F, 0x04, 0xBA37);
    pull_x();
    abi_nz(INC8(OBJ->state[0x40]));
    lda(0x11);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x76);
    WR8(OBJ->y_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CountThenOutburst18DC(void)
{
    CountThenOutburst18DC(cpu.x);
}

/*
 * @routine 18DC:30F ListThenWalkByFacing18DC
 * @run 70:BB0F
 * WHAT IT DOES: Plays the list at $BB3C; then walks by facing, hit-tested, state on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenWalkByFacing18DC(uint8_t slot)
{
    step_list(slot, 0xBB3C, true, 0xBB1A);
    if (!list_ended()) {
        return;
    }
    lda(RD8(OBJ->facing_left[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBB6B + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xBB6C + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListThenWalkByFacing18DC(void)
{
    ListThenWalkByFacing18DC(cpu.x);
}

/*
 * @routine 05CB:706 SinkToB8_05CB
 * @run 70:B706
 * WHAT IT DOES: Every fourth count of OBJ_WORK_D sinks with growing speed to Y $B8 (secret stage 02's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SinkToB8_05CB(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x0E, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xB721);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xB8);
    if (cpu.a < 0xB8) {
        return;
    }
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0xB8);
    WR8(OBJ->y_lo[slot], cpu.a);
}

void abi_SinkToB8_05CB(void)
{
    SinkToB8_05CB(cpu.x);
}

/*
 * @routine 0B4C:584 ListThenJumpWhen0B4C
 * @run 70:BD84
 * WHAT IT DOES: Plays the list at $BDB2 while $BDBB and $B7CC run; when C is left set, jumps by facing (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenJumpWhen0B4C(uint8_t slot)
{
    step_list(slot, 0xBDB2, false, 0xBD8F);
    lda(RD8(OBJ->work_h[slot]));
    call_routine(0xBD95, 0xBDBB);
    call_routine(0xBD98, 0xB7CC);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    cpu.y = RD8(OBJ->facing_left[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBDB9 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0xFA);
    WR8(OBJ->vy_hi[slot], cpu.a);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenJumpWhen0B4C(void)
{
    ListThenJumpWhen0B4C(cpu.x);
}

/*
 * @routine 0EBD:5BB LoadAboveRightWithPartner0EBD
 * @run 71:C5BB
 * WHAT IT DOES: Loads above and right of the screen with a kind $B0 partner in the next slot (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAboveRightWithPartner0EBD(uint8_t slot)
{
    load_record(slot, 0xC5BE);
    lda(0x18);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(0xE8);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(0xFF);
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(0x50);
    WR8(OBJ->work_p[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xC5DD);
    lda(0xB0);
    WR8(OBJ->kind[cpu.y], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->start_arg[cpu.y], cpu.a);
}

void abi_LoadAboveRightWithPartner0EBD(void)
{
    LoadAboveRightWithPartner0EBD(cpu.x);
}

/*
 * @routine 101D:7A3 SegmentBySlot101D
 * @run 71:C7A3
 * WHAT IT DOES: A piece of a row from slot $20: waits eight frames a slot after $10, its frame by slot, then falls (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SegmentBySlot101D(uint8_t slot)
{
    Sum sum;

    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC7AB);
        lda(cpu.x);
        sum = sub_bytes(cpu.a, 0x20, true);
        lda(sum.value);
        abi_flag(P_C, sum.carry);
        abi_flag(P_V, sum.overflow);
        push8(cpu.a);
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        add_a(0x10, false);
        WR8(OBJ->work_d[slot], cpu.a);
        lda(pull8());
        add_a(0x29, false);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a != 0) {
        abi_nz(DEC8(OBJ->work_d[slot]));
        return;
    }
    gravity(slot, 0xC7CD);
    move_y(slot, 0xC7D0);
}

void abi_SegmentBySlot101D(void)
{
    SegmentBySlot101D(cpu.x);
}

/*
 * @routine 161C:4DB SoundAtStep9ThenState4_161C
 * @run 70:BCDB
 * WHAT IT DOES: Plays the list at $BD09 sounding $1A at step 9; then OBJ_WORK_D 64 and state 4 (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundAtStep9ThenState4_161C(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x09);
    if (cpu.a == 0x09) {
        lda(RD8(OBJ->anim_timer[slot]));
        cmp(cpu.a, 0x03);
        if (cpu.a == 0x03) {
            lda(0x1A);
            jsr_push(0xBCEE);
            sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, cpu.x, cpu.y));
            jsr_pop();
        }
    }
    step_list(slot, 0xBD09, true, 0xBCF9);
    if (!list_ended()) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_SoundAtStep9ThenState4_161C(void)
{
    SoundAtStep9ThenState4_161C(cpu.x);
}

/*
 * @routine 16CC:3A9 ListSinkToB8Then16CC
 * @run 70:BBA9
 * WHAT IT DOES: Falls playing the list at $BBD7 to Y $B8; then attribute $10, no hit box, state on (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSinkToB8Then16CC(uint8_t slot)
{
    step_list(slot, 0xBBD7, true, 0xBBB4);
    move(slot, 0xBBB7);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xB8);
    if (cpu.a < 0xB8) {
        return;
    }
    lda(0xB8);
    WR8(OBJ->y_lo[slot], cpu.a);
    random_a(0xBBC6);
    lda(0x20);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    WR8(OBJ->hitbox_on[slot], 0);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListSinkToB8Then16CC(void)
{
    ListSinkToB8Then16CC(cpu.x);
}

/*
 * @routine 05CB:7EF FallDissolvingThenBecome22_05CB
 * @run 70:B7EF
 * WHAT IT DOES: Falls eroding patterns every fourth count; below a screen the slot becomes kind $22 (secret stage 02's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallDissolvingThenBecome22_05CB(uint8_t slot)
{
    move_y(slot, 0xB7F2);
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a == 0) {
        lda(RD8(OBJ->work_i[slot]));
        add_a(0x13, false);
        lda((uint8_t)(cpu.a & 0x3F));
        WR8(OBJ->work_i[slot], cpu.a);
        call_routine(0xB807, 0xB81E);
    }
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0x01);
    if (cpu.a != 0x01) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x10);
    if (cpu.a < 0x10) {
        return;
    }
    clear_this(slot, 0xB818);
    lda(0x22);
    WR8(OBJ->kind[slot], cpu.a);
}

void abi_FallDissolvingThenBecome22_05CB(void)
{
    FallDissolvingThenBecome22_05CB(cpu.x);
}

/*
 * @routine 0CAC:7D6 LoadOnCircle0CAC
 * @run 70:BFD6
 * WHAT IT DOES: Loads on a circle of radius $50 around the screen's centre at the angle of OBJ_WORK_D (secret stage 07's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadOnCircle0CAC(uint8_t slot)
{
    load_record(slot, 0xBFD9);
    WR8(OBJ->x_hi[slot], 0);
    lda(0x80);
    WR8(OBJ->x_lo[slot], cpu.a);
    WR8(OBJ->y_hi[slot], 0);
    lda(0x70);
    WR8(OBJ->y_lo[slot], cpu.a);
    cpu.y = RD8(OBJ->work_d[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC005 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    WR8(ZP->temp00, 0);
    lda(0x50);
    WR8(ZP->temp01, cpu.a);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    velocity_from_a(slot, 0xC001);
    move(slot, 0xC004);
}

void abi_LoadOnCircle0CAC(void)
{
    LoadOnCircle0CAC(cpu.x);
}

/*
 * @routine 0E0C:143 ListFallCountThen0E0C
 * @run 70:B943
 * WHAT IT DOES: Falls playing the list at $B972 while OBJ_WORK_E counts; then the state on, or with a start argument sound $CF and state 5 (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListFallCountThen0E0C(uint8_t slot)
{
    step_list(slot, 0xB972, true, 0xB94E);
    move_y(slot, 0xB951);
    gravity(slot, 0xB954);
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    restart_animation(slot);
    INC8(OBJ->state[slot]);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a == 0) {
        return;
    }
    sound(0xCF, 0xB96C);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ListFallCountThen0E0C(void)
{
    ListFallCountThen0E0C(cpu.x);
}

/*
 * @routine 101C:39B PickNextSubstate101C
 * @run 70:BB9B
 * WHAT IT DOES: Off screen goes on at $BC87; else the next substate and count of a cycle once frame $74 shows (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PickNextSubstate101C(uint8_t slot)
{
    restart_animation(slot);
    lda(RD8(OBJ->x_hi[slot]));
    if (cpu.a != 0) {
        run_routine(0xBC87);
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x74);
    if (cpu.a != 0x74) {
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xBBCA + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    lda(rd8((uint16_t)(0xBBD2 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    abi_nz(INC8(OBJ->work_h[slot]));
}

void abi_PickNextSubstate101C(void)
{
    PickNextSubstate101C(cpu.x);
}

/*
 * @routine 101C:545 LoadShotByArg101C
 * @run 70:BD45
 * WHAT IT DOES: Loads, steps once by facing and start argument, then takes its X speed (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadShotByArg101C(uint8_t slot)
{
    load_record(slot, 0xBD48);
    lda(0xF4);
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    cpu.y = RD8(OBJ->facing_left[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBD74 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    cpu.y = RD8(OBJ->start_arg[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBD76 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xBD62);
    lda(RD8(OBJ->facing_left[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBD79 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBD78 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
}

void abi_LoadShotByArg101C(void)
{
    LoadShotByArg101C(cpu.x);
}

/*
 * @routine 161F:1D4 LoadItemByArg161F
 * @run 71:D1D4
 * WHAT IT DOES: Loads, unpacks the start argument into OBJ_WORK_D to F and becomes kind $A5 (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadItemByArg161F(uint8_t slot)
{
    load_record(slot, 0xD1D7);
    lda(0x80);
    add_a(RD8(OBJ->hitbox_on[slot]), false);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    cpu.y = cpu.a;
    lda((uint8_t)(cpu.a & 0x0F));
    lda((uint8_t)(cpu.a | 0x10));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(cpu.y);
    rol_a();
    rol_a();
    rol_a();
    cpu.y = cpu.a;
    lda((uint8_t)(cpu.a & 0x03));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(cpu.y);
    rol_a();
    rol_a();
    lda((uint8_t)(cpu.a & 0x03));
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0xA5);
    WR8(OBJ->kind[slot], cpu.a);
}

void abi_LoadItemByArg161F(void)
{
    LoadItemByArg161F(cpu.x);
}

/*
 * @routine 117E:27C ListByAThenCheck117E
 * @run 71:CA7C
 * WHAT IT DOES: The list and X speed of A's row; on new frames $CACF and $CAE7, the speed reversed when OBJ_WORK_H (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListByAThenCheck117E(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xCAA9 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xCAAA + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    lda(rd8((uint16_t)(0xCAAB + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xCAAC + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    y = cpu.y;
    jsr_push(0xCA98);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    call_routine(0xCA9D, 0xCACF);
    lda(RD8(OBJ->work_h[slot]));
    if (cpu.a != 0) {
        negate_x(slot, 0xCAA5);
    }
    call_routine(0xCAA8, 0xCAE7);
}

void abi_ListByAThenCheck117E(void)
{
    ListByAThenCheck117E(cpu.x);
}

/*
 * @routine 088C:043 BobThenClear088C
 * @run 70:B843
 * WHAT IT DOES: A Y speed of the cosine of OBJ_WORK_E scaled by $2000, the angle stepped; cleared when OBJ_WORK_F counts out (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BobThenClear088C(uint8_t slot)
{
    lda(0x20);
    WR8(ZP->temp01, cpu.a);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    jsr_push(0xB851);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct2));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8(zProduct1));
    WR8(OBJ->vy_lo[slot], cpu.a);
    move_y(slot, 0xB85E);
    lda(RD8(OBJ->work_e[slot]));
    add_a(0x20, false);
    WR8(OBJ->work_e[slot], cpu.a);
    if (!counted_out(DEC8(OBJ->work_f[slot]))) {
        return;
    }
    ClearObject(slot);
}

void abi_BobThenClear088C(void)
{
    BobThenClear088C(cpu.x);
}
