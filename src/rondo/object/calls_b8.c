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
#include "calls_b8.h"

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

/*
 * @routine 161D:211 ListThenAim161D
 * @run 71:C211
 * WHAT IT DOES: Moves while the list at $C238 plays; then aims at the player at speed 4, OBJ_WORK_D 48, substate on (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenAim161D(uint8_t slot)
{
    move_y(slot, 0xC214);
    step_list(slot, 0xC238, true, 0xC21F);
    if (!list_ended()) {
        return;
    }
    aim_at_player(0xC224);
    push8(cpu.a);
    WR8(ZP->temp00, 0);
    lda(0x04);
    WR8(ZP->temp01, cpu.a);
    lda(pull8());
    velocity_from_a(slot, 0xC22F);
    lda(0x30);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenAim161D(void)
{
    ListThenAim161D(cpu.x);
}

/*
 * @routine 18DB:2DE CountThenDrop26_18DB
 * @run 70:B2DE
 * WHAT IT DOES: Counts OBJ_WORK_E; then moves to a random X, starts a kind $26 object in slot $18, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenDrop26_18DB(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    random_a(0xB2E6);
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB305 + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0x76);
    WR8(OBJ->y_lo[slot], cpu.a);
    ldy(0x18);
    clear_slot_in_y(0xB2F9);
    start_at_place(0x26, 0xB2FE);
    INC8(OBJ->work_f[slot]);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CountThenDrop26_18DB(void)
{
    CountThenDrop26_18DB(cpu.x);
}

/*
 * @routine 072C:39F FallUntilScreen4_072C
 * @run 70:BB9F
 * WHAT IT DOES: Falls four screens, running $BBC7; then new counts and the substate on (secret stage 03's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallUntilScreen4_072C(uint8_t slot)
{
    lda(0x05);
    WR8(OBJ->work_i[slot], cpu.a);
    gravity(slot, 0xBBA7);
    move_y(slot, 0xBBAA);
    call_routine(0xBBAD, 0xBBC7);
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0x04);
    if (cpu.a != 0x04) {
        return;
    }
    lda(0x00);
    WR8(OBJ->work_n[slot], cpu.a);
    lda(0x06);
    WR8(OBJ->work_i[slot], cpu.a);
    lda(0x30);
    WR8(OBJ->work_j[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_FallUntilScreen4_072C(void)
{
    FallUntilScreen4_072C(cpu.x);
}

/*
 * @routine 117C:5E1 ThrowThenLandRow117C
 * @run 70:BDE1
 * WHAT IT DOES: Throws the object up; then it falls, running $BBD8 on the $Dx row and $BBB1 after (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ThrowThenLandRow117C(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a == 0) {
        lda(0xFC);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(0x00);
        WR8(OBJ->vy_lo[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    lda((uint8_t)(cpu.a & 0xF0));
    cmp(cpu.a, 0xD0);
    if (cpu.a == 0xD0) {
        call_routine(0xBE00, 0xBBD8);
    }
    jsr_push(0xBE03);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    move(slot, 0xBE06);
    run_routine(0xBBB1);
}

void abi_ThrowThenLandRow117C(void)
{
    ThrowThenLandRow117C(cpu.x);
}

/*
 * @routine 117D:15A ListLandThen117D
 * @run 71:C15A
 * WHAT IT DOES: Plays the list at $C182 falling; on landing sound 2, OBJ_WORK_G 24, frame $3B, substate on (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListLandThen117D(uint8_t slot)
{
    step_list(slot, 0xC182, false, 0xC165);
    call_routine(0xC168, 0xBBD8);
    lda(0x0C);
    jsr_push(0xC16D);
    mover_exit_shim(StopOnLandingFrontColumn(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    sound(0x02, 0xC174);
    lda(0x18);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(0x3B);
    WR8(OBJ->frame[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListLandThen117D(void)
{
    ListLandThen117D(cpu.x);
}

/*
 * @routine 117D:515 LoadFacingWithPrevious117D
 * @run 71:C515
 * WHAT IT DOES: Loads facing the player with a kind $33 object in the slot before; start argument 2 sets attribute $87 (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFacingWithPrevious117D(uint8_t slot)
{
    load_record(slot, 0xC518);
    face_by_side(slot, COL(OBJ->facing_left), 0xC51B);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xC52B);
    start_at_place(0x33, 0xC530);
    lda(RD8(OBJ->start_arg[slot]));
    cmp(cpu.a, 0x02);
    if (cpu.a != 0x02) {
        return;
    }
    lda(0x87);
    WR8(OBJ->sprite_attr[slot], cpu.a);
}

void abi_LoadFacingWithPrevious117D(void)
{
    LoadFacingWithPrevious117D(cpu.x);
}

/*
 * @routine 117F:416 ListDashThenState117F
 * @run 71:D416
 * WHAT IT DOES: Dashes left on the list's new frames until X under $C0 on the first screen; then the state on (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListDashThenState117F(uint8_t slot)
{
    step_list(slot, 0xD479, true, 0xD421);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(0xFB);
    WR8(OBJ->vx_hi[slot], cpu.a);
    move_x(slot, 0xD42B);
    lda(RD8(OBJ->x_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->x_lo[slot]));
    cmp(cpu.a, 0xC0);
    if (cpu.a >= 0xC0) {
        return;
    }
    INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListDashThenState117F(void)
{
    ListDashThenState117F(cpu.x);
}

/*
 * @routine 12DC:45C MoveBounceOffWall12DC
 * @run 70:BC5C
 * WHAT IT DOES: Moves along X; a wall met on the way it goes turns it back (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void MoveBounceOffWall12DC(uint8_t slot)
{
    move_x(slot, 0xBC5F);
    lda(0x04);
    WR8(ZP->product.mid, cpu.a);
    lda(0x02);
    jsr_push(0xBC68);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    lda(RD8(OBJ->vx_hi[slot]));
    {
        bool out = (cpu.a & 0x80) != 0;

        lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
        abi_flag(P_C, out);
        out = (cpu.a & 0x80) != 0;
        lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
        abi_flag(P_C, out);
    }
    lda((uint8_t)(cpu.a & 0x01));
    lda((uint8_t)(cpu.a ^ 0x01));
    word_index(cpu.a);
    lda(rd8((uint16_t)(TileHit + cpu.y)));
    lda((uint8_t)(cpu.a | rd8((uint16_t)(TileHit + 1 + cpu.y))));
    lda((uint8_t)(cpu.a & 0x02));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(0xBC81);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocityX(slot));
}

void abi_MoveBounceOffWall12DC(void)
{
    MoveBounceOffWall12DC(cpu.x);
}

/*
 * @routine 161C:437 ListByStateThenProbe161C
 * @run 70:BC37
 * WHAT IT DOES: The list for the state from 4; on new frames the X speed by OBJ_WORK_E and a probe (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListByStateThenProbe161C(uint8_t slot)
{
    uint8_t y;
    FrameStep step;
    Sum sum;

    lda(RD8(OBJ->state[slot]));
    sum = sub_bytes(cpu.a, 0x04, true);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBC61 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xBC62 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    jsr_push(0xBC4C);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    cpu.y = RD8(OBJ->work_e[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBC5F + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x0A);
    WR8(ZP->product.mid, cpu.a);
    call_routine(0xBC5E, 0xAB3A);
}

void abi_ListByStateThenProbe161C(void)
{
    ListByStateThenProbe161C(cpu.x);
}

/*
 * @routine 10CC:273 SpawnEvery48_10CC
 * @run 70:BA73
 * WHAT IT DOES: Every 49th call, a kind $26 object in the first empty of slots $18 to $1A (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnEvery48_10CC(uint8_t slot)
{
    lda(RD8(OBJ->work_m[slot]));
    if (cpu.a != 0) {
        abi_nz(DEC8(OBJ->work_m[slot]));
        return;
    }
    lda(0x30);
    WR8(OBJ->work_m[slot], cpu.a);
    ldy(0x18);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (cpu.a == 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x1B);
        if (cpu.y >= 0x1B) {
            return;
        }
    }
    clear_slot_in_y(0xBA91);
    start_at_place(0x26, 0xBA96);
    lda(0x01);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
}

void abi_SpawnEvery48_10CC(void)
{
    SpawnEvery48_10CC(cpu.x);
}

/*
 * @routine 117B:68D TurnAround117B
 * @run 70:B68D
 * WHAT IT DOES: Turns the object around with a delay by the player's distance, substate on (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnAround117B(uint8_t slot)
{
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->work_k[slot], cpu.a);
    jsr_push(0xB696);
    distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x0F));
    WR8(OBJ->work_i[slot], cpu.a);
    call_routine(0xB69E, 0xB6B6);
    jsr_push(0xB6A1);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    jsr_push(0xB6A4);
    negate_exit_shim(NegateVelocityY(slot));
    jsr_pop();
    call_routine(0xB6A7, 0xB722);
    lda(0x0C);
    WR8(OBJ->work_d[slot], cpu.a);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_TurnAround117B(void)
{
    TurnAround117B(cpu.x);
}

/*
 * @routine 117E:4D9 ListKeepingPace117E
 * @run 71:CCD9
 * WHAT IT DOES: Plays the list at $CD02 with OBJ_PACE bits 0-1 set; then clears them, state 5 (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListKeepingPace117E(uint8_t slot)
{
    lda(RD8(OBJ->pace[slot]));
    lda((uint8_t)(cpu.a | 0x03));
    WR8(OBJ->pace[slot], cpu.a);
    step_list(slot, 0xCD02, false, 0xCCEC);
    if (!list_ended()) {
        return;
    }
    lda(RD8(OBJ->pace[slot]));
    lda((uint8_t)(cpu.a & 0xFC));
    WR8(OBJ->pace[slot], cpu.a);
    restart_animation(slot);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ListKeepingPace117E(void)
{
    ListKeepingPace117E(cpu.x);
}

/*
 * @routine 12DC:7B6 ScriptsAtCounts12DC
 * @run 70:BFB6
 * WHAT IT DOES: While OBJ_WORK_L counts, scripts $8B-$8E at the table's counts; at zero 128 and substate on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ScriptsAtCounts12DC(uint8_t slot)
{
    if (counted_out(DEC8(OBJ->work_l[slot]))) {
        lda(0x80);
        WR8(OBJ->work_l[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    cpu.y = 0;
    for (;;) {
        lda(RD8(OBJ->work_l[slot]));
        cmp(cpu.a, rd8((uint16_t)(0xBFDF + cpu.y)));
        if ((cpu.p & P_Z) != 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x04);
        if (cpu.y >= 0x04) {
            return;
        }
    }
    lda(cpu.y);
    add_a(0x8B, false);
    push8(cpu.x);
    cpu.x = 0x05;
    abi_nz(cpu.x);
    jsr_push(0xBFD4);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
    pull_x();
}

void abi_ScriptsAtCounts12DC(void)
{
    ScriptsAtCounts12DC(cpu.x);
}

/*
 * @routine 18DB:3F9 ListWithChecksThenSpawn18DB
 * @run 70:B3F9
 * WHAT IT DOES: Plays the list at $B422, on new frames setting the hit box and starting; when it ends a kind $26 object, substate back (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListWithChecksThenSpawn18DB(uint8_t slot)
{
    step_list(slot, 0xB422, true, 0xB404);
    if (!list_ended()) {
        if ((cpu.p & P_C) == 0) {
            return;
        }
        call_routine(0xB40B, 0xB394);
        call_routine(0xB40E, 0xB43D);
        return;
    }
    ldy(0x18);
    clear_slot_in_y(0xB414);
    start_at_place(0x26, 0xB419);
    lda(0x01);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    abi_nz(DEC8(OBJ->substate[slot]));
}

void abi_ListWithChecksThenSpawn18DB(void)
{
    ListWithChecksThenSpawn18DB(cpu.x);
}

/*
 * @routine 18DC:082 ListThenSpawn27_18DC
 * @run 70:B882
 * WHAT IT DOES: Plays the list at $B8AB; then a kind $27 object in slot $18 with the facing, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenSpawn27_18DC(uint8_t slot)
{
    step_list(slot, 0xB8AB, true, 0xB88D);
    if (!list_ended()) {
        return;
    }
    ldy(0x18);
    clear_slot_in_y(0xB894);
    start_at_place(0x27, 0xB899);
    lda(0x01);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    WR8(OBJ->work_j[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenSpawn27_18DC(void)
{
    ListThenSpawn27_18DC(cpu.x);
}

/*
 * @routine 18DC:0B6 SpawnWhenSlot24Gone18DC
 * @run 70:B8B6
 * WHAT IT DOES: Once slot $18 is empty: sound $26, a kind $2C object in slot $20, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnWhenSlot24Gone18DC(uint8_t slot)
{
    lda(RD8(OBJ->kind[0x18]));
    if (cpu.a != 0) {
        return;
    }
    sound(0x26, 0xB8C0);
    ldy(0x20);
    clear_slot_in_y(0xB8C5);
    start_at_place(0x2C, 0xB8CA);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    lda(RD8(OBJ->work_i[slot]));
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    lda(0x38);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SpawnWhenSlot24Gone18DC(void)
{
    SpawnWhenSlot24Gone18DC(cpu.x);
}

/*
 * @routine 09EB:3F2 RunListThenState13_09EB
 * @run 70:B3F2
 * WHAT IT DOES: The routine at $AA35 and the list at $B41C; when it ends state $D, at least 3 frames not hit-tested (secret stage 05's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RunListThenState13_09EB(uint8_t slot)
{
    lda(0x20);
    WR8(ZP->temp08, cpu.a);
    lda(0x2C);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB3FD, 0xAA35);
    step_list(slot, 0xB41C, false, 0xB408);
    if (!list_ended()) {
        return;
    }
    lda(0x0D);
    WR8(OBJ->state[slot], cpu.a);
    lda(RD8(OBJ->invuln_timer[slot]));
    cmp(cpu.a, 0x03);
    if (cpu.a >= 0x03) {
        return;
    }
    lda(0x03);
    WR8(OBJ->invuln_timer[slot], cpu.a);
}

void abi_RunListThenState13_09EB(void)
{
    RunListThenState13_09EB(cpu.x);
}

/*
 * @routine 0E0B:5A8 ListShootAtFrame3B_0E0B
 * @run 70:B5A8
 * WHAT IT DOES: Plays the list at $B5D2; at frame $3B sound $CF and a kind $25 object; when it ends, substate on (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListShootAtFrame3B_0E0B(uint8_t slot)
{
    step_list(slot, 0xB5D2, true, 0xB5B3);
    if (list_ended()) {
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x3B);
    if (cpu.a != 0x3B) {
        return;
    }
    sound(0xCF, 0xB5C3);
    ldy(0x50);
    clear_slot_in_y(0xB5C8);
    start_at_place(0x25, 0xB5CD);
}

void abi_ListShootAtFrame3B_0E0B(void)
{
    ListShootAtFrame3B_0E0B(cpu.x);
}

/*
 * @routine 0E0B:661 DripAtFrame26_0E0B
 * @run 70:B661
 * WHAT IT DOES: At frame $26, every fourth count, a kind $28 object at the next of 16 X places (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DripAtFrame26_0E0B(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x26);
    if (cpu.a != 0x26) {
        return;
    }
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    if (!find_from30(0xB672)) {
        return;
    }
    start_at_place(0x28, 0xB679);
    push8(cpu.x);
    lda(RD8(OBJ->work_j[slot]));
    lda((uint8_t)(cpu.a & 0x0F));
    cpu.x = cpu.a;
    lda(rd8((uint16_t)(0xB68B + cpu.x)));
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    pull_x();
    abi_nz(INC8(OBJ->work_j[slot]));
}

void abi_DripAtFrame26_0E0B(void)
{
    DripAtFrame26_0E0B(cpu.x);
}

/*
 * @routine 0EBD:1AE ListThenWhenNear0EBD
 * @run 71:C1AE
 * WHAT IT DOES: Outside sub-mode 3: hit-tested, playing the list at $C223; within $70 dots the state on (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenWhenNear0EBD(uint8_t slot)
{
    lda(RD8(ZP->game_sub_mode));
    cmp(cpu.a, 0x03);
    if (cpu.a == 0x03) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    WR8(OBJ->loop_flags[slot], 0);
    step_list(slot, 0xC223, true, 0xC1C8);
    if (!list_ended()) {
        return;
    }
    near_x(slot, 0xC1CD);
    cmp(cpu.a, 0x70);
    if (cpu.a >= 0x70) {
        return;
    }
    INC8(OBJ->state[slot]);
    WR8(OBJ->hitbox_on[slot], 0);
}

void abi_ListThenWhenNear0EBD(void)
{
    ListThenWhenNear0EBD(cpu.x);
}

/*
 * @routine 0EBE:04B RideSourceOrState5_0EBE
 * @run 71:C84B
 * WHAT IT DOES: Rides 20 dots below the slot the start argument names while it is kind $2C; else stops, state 5 (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RideSourceOrState5_0EBE(uint8_t slot)
{
    cpu.y = RD8(OBJ->start_arg[slot]);
    abi_nz(cpu.y);
    if (cpu.y != 0) {
        lda(RD8(OBJ->kind[cpu.y]));
        cmp(cpu.a, 0x2C);
        if (cpu.a == 0x2C) {
            jsr_push(0xC85A);
            copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
            jsr_pop();
            lda(0x14);
            WR8(OBJ->vy_hi[slot], cpu.a);
            move_y(slot, 0xC862);
            return;
        }
    }
    stop_here(slot);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_RideSourceOrState5_0EBE(void)
{
    RideSourceOrState5_0EBE(cpu.x);
}

/*
 * @routine 13FB:678 ListThenHop13FB
 * @run 70:B678
 * WHAT IT DOES: Plays the list at $B6A2; then hops by facing, substate on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenHop13FB(uint8_t slot)
{
    step_list(slot, 0xB6A2, true, 0xB683);
    if (!list_ended()) {
        return;
    }
    lda(RD8(OBJ->facing_left[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xB6A8 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB6A7 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenHop13FB(void)
{
    ListThenHop13FB(cpu.x);
}

/*
 * @routine 13FC:070 ProbeSpawnListWhenAlone13FC
 * @run 70:B870
 * WHAT IT DOES: Falls ($B6CC), spawns ($B493), plays the list at $B89A until slots $11 and $12 are empty (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ProbeSpawnListWhenAlone13FC(uint8_t slot)
{
    call_routine(0xB873, 0xB6CC);
    call_routine(0xB876, 0xB493);
    if ((cpu.p & P_C) != 0) {
        lda(0x01);
        WR8(OBJ->start_arg[cpu.y], cpu.a);
    }
    step_list(slot, 0xB89A, true, 0xB888);
    lda(RD8(OBJ->kind[0x11]));
    lda((uint8_t)(cpu.a | RD8(OBJ->kind[0x12])));
    if (cpu.a != 0) {
        return;
    }
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ProbeSpawnListWhenAlone13FC(void)
{
    ProbeSpawnListWhenAlone13FC(cpu.x);
}

/*
 * @routine 18DC:249 ListSoundThenSpawn30_18DC
 * @run 70:BA49
 * WHAT IT DOES: Plays the list at $BA73, sound $90 at step 2; when it ends a kind $30 object in slot $30, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSoundThenSpawn30_18DC(uint8_t slot)
{
    step_list(slot, 0xBA73, true, 0xBA54);
    if (list_ended()) {
        ldy(0x30);
        clear_slot_in_y(0xBA6A);
        start_at_place(0x30, 0xBA6F);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x02);
    if (cpu.a != 0x02) {
        return;
    }
    sound(0x90, 0xBA64);
}

void abi_ListSoundThenSpawn30_18DC(void)
{
    ListSoundThenSpawn30_18DC(cpu.x);
}

/*
 * @routine 161E:5E5 LoadThenListClear161E
 * @run 71:CDE5
 * WHAT IT DOES: In state 0 loads rising; later moves playing the list at $CE0E and is cleared when it ends (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadThenListClear161E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xCDED);
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x0D);
        WR8(OBJ->frame[slot], cpu.a);
        abi_nz(DEC8(OBJ->vy_hi[slot]));
        return;
    }
    move(slot, 0xCDFE);
    step_list(slot, 0xCE0E, true, 0xCE09);
    if (!list_ended()) {
        return;
    }
    ClearObject(slot);
}

void abi_LoadThenListClear161E(void)
{
    LoadThenListClear161E(cpu.x);
}

/*
 * @routine 088B:665 MoveOrAimShot088B
 * @run 70:B665
 * WHAT IT DOES: Moving: moves; else loads, aims at the player at speed 6 with the frame of that angle, sound $2E (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void MoveOrAimShot088B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a != 0) {
        axis_move_exit_shim(ApplyVelocity(slot));
        return;
    }
    load_record(slot, 0xB670);
    aim_at_player(0xB673);
    push8(cpu.a);
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    add_a(0x01, false);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x06);
    WR8(ZP->temp01, cpu.a);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(pull8());
    velocity_from_a(slot, 0xB689);
    sound(0x2E, 0xB68E);
}

void abi_MoveOrAimShot088B(void)
{
    MoveOrAimShot088B(cpu.x);
}

/*
 * @routine 117E:221 LoadAndStart30_117E
 * @run 71:CA21
 * WHAT IT DOES: Loads with a kind $30 object in the next slot; a start argument takes the player's side and state 8 (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAndStart30_117E(uint8_t slot)
{
    load_record(slot, 0xCA24);
    lda(0xB0);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xCA34);
    start_at_place(0x30, 0xCA39);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(0xCA41);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    WR8(OBJ->work_i[slot], cpu.a);
    lda(0x08);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_LoadAndStart30_117E(void)
{
    LoadAndStart30_117E(cpu.x);
}
