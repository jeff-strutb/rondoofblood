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
#include "calls_b9.h"

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

static void load_record_tail_(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
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

/*
 * @routine 117F:23E LoadOrAimFromPrevious117F
 * @run 71:D23E
 * WHAT IT DOES: In state 0 loads the record; later keeps the previous slot's place and attribute and shows the frame aiming at the player.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadOrAimFromPrevious117F(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    lda(RD8_IDX((uint16_t)(COL(OBJ->sprite_attr) - 1), slot));
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    swap_xy();
    jsr_push(0xD253);
    copy_place_exit_shim(CopyObjectPlace(cpu.x, cpu.y));
    jsr_pop();
    swap_xy();
    aim_at_player(0xD257);
    add_a(0x10, false);
    {
        int i;

        for (i = 0; i < 5; i++) {
            abi_flag(P_C, (cpu.a & 0x01) != 0);
            lda((uint8_t)(cpu.a >> 1));
        }
    }
    add_a(0x75, false);
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_LoadOrAimFromPrevious117F(void)
{
    LoadOrAimFromPrevious117F(cpu.x);
}

/*
 * @routine 161F:269 RiseWithRoute161F
 * @run 71:D269
 * WHAT IT DOES: Loads; once zField8C is set rises two dots a frame until Y low passes $80 and is cleared (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RiseWithRoute161F(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xD271);
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x44);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    cpu.a = (uint8_t)(cpu.a - 1);
    abi_nz(cpu.a);
    if (cpu.a == 0) {
        lda(RD8(ZP->field8c));
        if (cpu.a == 0) {
            return;
        }
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    DEC8(OBJ->y_lo[slot]);
    abi_nz(DEC8(OBJ->y_lo[slot]));
    if ((cpu.p & P_N) == 0) {
        return;
    }
    ClearObject(slot);
}

void abi_RiseWithRoute161F(void)
{
    RiseWithRoute161F(cpu.x);
}

/*
 * @routine 0EBD:502 WalkListThenState0EBD
 * @run 71:C502
 * WHAT IT DOES: Walks playing the list at $C52D; OBJ_WORK_G 1 means state 3, bit 7 a random state step (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkListThenState0EBD(uint8_t slot)
{
    move_x(slot, 0xC505);
    step_list(slot, 0xC52D, true, 0xC510);
    lda(RD8(OBJ->work_g[slot]));
    cmp(cpu.a, 0x01);
    if (cpu.a == 0x01) {
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->work_g[slot]));
    if ((cpu.a & 0x80) == 0) {
        return;
    }
    random_a(0xC51F);
    lda((uint8_t)(cpu.a & 0x7F));
    if (cpu.a != 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_WalkListThenState0EBD(void)
{
    WalkListThenState0EBD(cpu.x);
}

/*
 * @routine 101C:620 ArenaLimits101C
 * @run 70:BE20
 * WHAT IT DOES: Holds the scroll at screen 6 until slots $18, $1A and $1C are empty, then frees it to screen 8 (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ArenaLimits101C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        lda(rd8(ScrollXHiScreen));
        cmp(cpu.a, 0x06);
        if (cpu.a != 0x06) {
            return;
        }
        lda(0x06);
        wr8(ScrollXMinHi, cpu.a);
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    lda(RD8(OBJ->kind[0x18]));
    lda((uint8_t)(cpu.a | RD8(OBJ->kind[0x1A])));
    lda((uint8_t)(cpu.a | RD8(OBJ->kind[0x1C])));
    if (cpu.a != 0) {
        return;
    }
    lda(0x08);
    wr8(ScrollXMaxHi, cpu.a);
    record_event(slot, 0xBE48);
    ClearObject(slot);
}

void abi_ArenaLimits101C(void)
{
    ArenaLimits101C(cpu.x);
}

/*
 * @routine 101D:2DF ShootAtFrameAB_101D
 * @run 71:C2DF
 * WHAT IT DOES: Plays the list at $C30A; at frame $AB sound $27 and a kind $BA object in the next slot (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ShootAtFrameAB_101D(uint8_t slot)
{
    step_list(slot, 0xC30A, true, 0xC2EA);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0xAB);
    if (cpu.a != 0xAB) {
        return;
    }
    sound(0x27, 0xC2F8);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xC2FE);
    start_at_place(0xBA, 0xC303);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_ShootAtFrameAB_101D(void)
{
    ShootAtFrameAB_101D(cpu.x);
}

/*
 * @routine 101E:0F0 LandThenWake101E
 * @run 71:C8F0
 * WHAT IT DOES: Falls playing the list at $C91B; once hit or the player within $70 dots, hit-tested and the state on (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandThenWake101E(uint8_t slot)
{
    step_list(slot, 0xC91B, true, 0xC8FB);
    lda(0x08);
    jsr_push(0xC900);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    lda(RD8(OBJ->hit_taken[slot]));
    if (cpu.a == 0) {
        near_x(slot, 0xC908);
        cmp(cpu.a, 0x70);
        if (cpu.a >= 0x70) {
            return;
        }
    }
    restart_animation(slot);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_LandThenWake101E(void)
{
    LandThenWake101E(cpu.x);
}

/*
 * @routine 12DC:331 ParryWhenHit12DC
 * @run 70:BB31
 * WHAT IT DOES: In states 2 and 3 a hit of 5 or more: sound $78, turn to the player, state 5 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ParryWhenHit12DC(uint8_t slot)
{
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x02);
    if (cpu.a != 0x02) {
        cmp(cpu.a, 0x03);
        if (cpu.a != 0x03) {
            return;
        }
    }
    sound(0x78, 0xBB48);
    face_by_side(slot, COL(OBJ->facing_left), 0xBB4B);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    restart_animation(slot);
}

void abi_ParryWhenHit12DC(void)
{
    ParryWhenHit12DC(cpu.x);
}

/*
 * @routine 13FC:422 ListThenHitTest13FC
 * @run 70:BC22
 * WHAT IT DOES: Plays the list at $BC4D; then hit-tested with fresh work bytes, substate on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenHitTest13FC(uint8_t slot)
{
    step_list(slot, 0xBC4D, true, 0xBC2D);
    if (!list_ended()) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(0x12);
    WR8(OBJ->work_c[slot], cpu.a);
    WR8(OBJ->work_e[slot], 0);
    WR8(OBJ->work_f[slot], 0);
    lda(0x01);
    WR8(OBJ->work_h[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->work_i[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenHitTest13FC(void)
{
    ListThenHitTest13FC(cpu.x);
}

/*
 * @routine 16CC:204 ListSpawnWhileCount16CC
 * @run 70:BA04
 * WHAT IT DOES: Plays the list at $B9EA starting a kind $26 object every eighth count of OBJ_WORK_D; then stops, substate on (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSpawnWhileCount16CC(uint8_t slot)
{
    step_list(slot, 0xB9EA, true, 0xBA0F);
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    if (cpu.a == 0 && find_from30(0xBA19)) {
        start_at_place(0x26, 0xBA20);
    }
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListSpawnWhileCount16CC(void)
{
    ListSpawnWhileCount16CC(cpu.x);
}

/*
 * @routine 0CAC:1E1 FallThenBecome2E_0CAC
 * @run 70:B9E1
 * WHAT IT DOES: Falls playing the list at $BA0D; five screens down becomes kind $2E and clears slot $11 (secret stage 07's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallThenBecome2E_0CAC(uint8_t slot)
{
    step_list(slot, 0xBA0D, true, 0xB9EC);
    call_routine(0xB9EF, 0xBA1E);
    gravity(slot, 0xB9F2);
    move_y(slot, 0xB9F5);
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    clear_this(slot, 0xB9FF);
    lda(0x2E);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    ldy(0x11);
    clear_slot_in_y(0xBA0C);
}

void abi_FallThenBecome2E_0CAC(void)
{
    FallThenBecome2E_0CAC(cpu.x);
}

/*
 * @routine 0CAD:03A CircleFromCentre0CAD
 * @run 71:C03A
 * WHAT IT DOES: Spirals in from the screen's centre: a place at angle OBJ_WORK_E, radius OBJ_WORK_G, until the radius is gone.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CircleFromCentre0CAD(uint8_t slot)
{
    WR8(OBJ->x_hi[slot], 0);
    lda(0x80);
    WR8(OBJ->x_lo[slot], cpu.a);
    WR8(OBJ->y_hi[slot], 0);
    lda(0x70);
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(ZP->temp00, 0);
    lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    velocity_from_a(slot, 0xC057);
    move(slot, 0xC05A);
    INC8(OBJ->work_e[slot]);
    if (!counted_out(DEC8(OBJ->work_g[slot]))) {
        return;
    }
    ClearObject(slot);
}

void abi_CircleFromCentre0CAD(void)
{
    CircleFromCentre0CAD(cpu.x);
}

/*
 * @routine 0EBF:25D SpeedRowByWorkI0EBF
 * @run 71:D25D
 * WHAT IT DOES: The speeds of row OBJ_WORK_I, OBJ_WORK_E counted, X reversed toward the player (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpeedRowByWorkI0EBF(uint8_t slot)
{
    lda(RD8(OBJ->work_i[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xD28A + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xD289 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xD28C + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xD28B + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    INC8(OBJ->work_e[slot]);
    jsr_push(0xD281);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a == 0) {
        return;
    }
    negate_exit_shim(NegateVelocityX(slot));
}

void abi_SpeedRowByWorkI0EBF(void)
{
    SpeedRowByWorkI0EBF(cpu.x);
}

/*
 * @routine 10CD:09A SpiralThenSound10CD
 * @run 71:C09A
 * WHAT IT DOES: Moves at angle OBJ_WORK_D while OBJ_WORK_G counts; then sound $1D on screen, else cleared (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpiralThenSound10CD(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    velocity_from_a(slot, 0xC0AA);
    move(slot, 0xC0AD);
    if (!counted_out(DEC8(OBJ->work_g[slot]))) {
        return;
    }
    lda(RD8(OBJ->x_hi[slot]));
    lda((uint8_t)(cpu.a | RD8(OBJ->y_hi[slot])));
    if (cpu.a != 0) {
        ClearObject(slot);
        return;
    }
    sound(0x1D, 0xC0C2);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SpiralThenSound10CD(void)
{
    SpiralThenSound10CD(cpu.x);
}

/*
 * @routine 117B:1D4 RiseToLine117B
 * @run 70:B1D4
 * WHAT IT DOES: While below the line OBJ_WORK_E, rises with growing speed; at the line it stops there (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RiseToLine117B(uint8_t slot)
{
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, RD8(OBJ->work_e[slot]));
    if ((cpu.p & P_C) != 0 && (cpu.p & P_Z) == 0) {
        Sum sum;

        move_y(slot, 0xB1E1);
        lda(RD8(OBJ->vy_lo[slot]));
        sum = sub_bytes(cpu.a, 0x30, true);
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
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
}

void abi_RiseToLine117B(void)
{
    RiseToLine117B(cpu.x);
}

/*
 * @routine 117D:102 LeapByDistance117D
 * @run 71:C102
 * WHAT IT DOES: A leap whose X speed is by the player's distance, reversed when OBJ_WORK_E (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LeapByDistance117D(uint8_t slot)
{
    int i;

    INC8(OBJ->substate[slot]);
    near_x(slot, 0xC108);
    for (i = 0; i < 5; i++) {
        abi_flag(P_C, (cpu.a & 0x01) != 0);
        lda((uint8_t)(cpu.a >> 1));
    }
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC12F + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC12E + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0xFB);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    if (cpu.a == 0) {
        return;
    }
    negate_x(slot, 0xC12D);
}

void abi_LeapByDistance117D(void)
{
    LeapByDistance117D(cpu.x);
}

/*
 * @routine 12DB:13C WaitForScreen3_12DB
 * @run 70:B13C
 * WHAT IT DOES: Holds the player's pad bytes while playing the list at $B168 until screen 3; then records the event, state on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WaitForScreen3_12DB(uint8_t slot)
{
    lda(0x03);
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x02);
    wr8(PlayerStartPose, cpu.a);
    WR8(OBJ->work_f[0], 0);
    step_list(slot, 0xB168, true, 0xB154);
    lda(rd8(ScrollXHiScreen));
    cmp(cpu.a, 0x03);
    if (cpu.a < 0x03) {
        return;
    }
    record_event(slot, 0xB15E);
    restart_animation(slot);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_WaitForScreen3_12DB(void)
{
    WaitForScreen3_12DB(cpu.x);
}

/*
 * @routine 12DC:603 SpawnThreeAtStep12DC
 * @run 70:BE03
 * WHAT IT DOES: At animation step 10: sound $25 and three numbered kind $30 objects with the facing (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnThreeAtStep12DC(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x0A);
    if (cpu.a != 0x0A) {
        return;
    }
    sound(0x25, 0xBE0F);
    WR8(ZP->temp00, 0);
    do {
        if (!find_from30(0xBE14)) {
            return;
        }
        start_at_place(0x30, 0xBE1B);
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        INC8(ZP->temp00);
        lda(RD8(ZP->temp00));
        cmp(cpu.a, 0x03);
    } while (cpu.a < 0x03);
}

void abi_SpawnThreeAtStep12DC(void)
{
    SpawnThreeAtStep12DC(cpu.x);
}

/*
 * @routine 12DD:0B3 FollowSlot16Piece12DD_0B3
 * @run 71:C0B3
 * WHAT IT DOES: Follows the previous slot at an offset from a table by OBJ_WORK_D and slot $10's facing, with its attribute and hit-test.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FollowSlot16Piece12DD_0B3(uint8_t slot)
{
    lda(RD8(OBJ->sprite_attr[0x10]));
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(RD8(OBJ->hitbox_on[0x10]));
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    jsr_push(0xC0C5);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8(OBJ->work_d[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    add_a(RD8(OBJ->facing_left[0x10]), false);
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC0DF + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC0E1 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xC0DE);
}

void abi_FollowSlot16Piece12DD_0B3(void)
{
    FollowSlot16Piece12DD_0B3(cpu.x);
}

/*
 * @routine 12DD:14D FollowSlot16Piece12DD_14D
 * @run 71:C14D
 * WHAT IT DOES: Follows the previous slot at an offset from a table by OBJ_WORK_D and slot $10's facing, with its attribute and hit-test.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FollowSlot16Piece12DD_14D(uint8_t slot)
{
    lda(RD8(OBJ->sprite_attr[0x10]));
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(RD8(OBJ->hitbox_on[0x10]));
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    jsr_push(0xC15F);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8(OBJ->work_d[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    add_a(RD8(OBJ->facing_left[0x10]), false);
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC179 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC17B + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xC178);
}

void abi_FollowSlot16Piece12DD_14D(void)
{
    FollowSlot16Piece12DD_14D(cpu.x);
}

/*
 * @routine 18DB:510 DiveThenSound18DB
 * @run 70:B510
 * WHAT IT DOES: Rises slowing under gravity $18; at the top sound $93, slot $40 to state 2, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DiveThenSound18DB(uint8_t slot)
{
    move_y(slot, 0xB513);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x18, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    if ((cpu.p & P_N) != 0) {
        return;
    }
    sound(0x93, 0xB52B);
    lda(0x02);
    WR8(OBJ->state[0x40], cpu.a);
    lda(0x80);
    WR8(OBJ->work_e[slot], cpu.a);
    WR8(OBJ->work_g[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_DiveThenSound18DB(void)
{
    DiveThenSound18DB(cpu.x);
}

/*
 * @routine 09EB:280 CountThenErupt09EB
 * @run 70:B280
 * WHAT IT DOES: Counts OBJ_WORK_D; then a kind $2E object in slot $11, sound $A2, rising unseen with the raster effect (secret stage 05's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenErupt09EB(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    ldy(0x11);
    clear_slot_in_y(0xB28A);
    start_at_place(0x2E, 0xB28F);
    sound(0xA2, 0xB294);
    lda(0xFF);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    WR8(OBJ->frame[slot], 0);
    lda(0x0B);
    WR8(ZP->raster_effect, cpu.a);
    lda(0xC0);
    WR8(ZP->raster_arg, cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_CountThenErupt09EB(void)
{
    CountThenErupt09EB(cpu.x);
}

/*
 * @routine 101B:17B ScatterEightThenClearPair101B
 * @run 70:B17B
 * WHAT IT DOES: Starts eight numbered kind $25 objects, then clears this slot and the next (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ScatterEightThenClearPair101B(uint8_t slot)
{
    WR8(ZP->temp00, 0);
    do {
        if (!find_from30(0xB180)) {
            break;
        }
        start_at_place(0x25, 0xB187);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        lda(0x17);
        WR8(OBJ->work_e[cpu.y], cpu.a);
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        INC8(ZP->temp00);
        lda(RD8(ZP->temp00));
        cmp(cpu.a, 0x08);
    } while (cpu.a < 0x08);
    clear_this(cpu.x, 0xB1A2);
    cpu.x = (uint8_t)(cpu.x + 1);
    abi_nz(cpu.x);
    clear_this(cpu.x, 0xB1A6);
    cpu.x = (uint8_t)(cpu.x - 1);
    abi_nz(cpu.x);
}

void abi_ScatterEightThenClearPair101B(void)
{
    ScatterEightThenClearPair101B(cpu.x);
}

/*
 * @routine 101C:6C0 FlyOffWhenSourceGone101C
 * @run 70:BEC0
 * WHAT IT DOES: Once the slot the start argument names is empty, flies off up and back with frame $81 (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FlyOffWhenSourceGone101C(uint8_t slot)
{
    cpu.y = RD8(OBJ->start_arg[slot]);
    abi_nz(cpu.y);
    lda(RD8(OBJ->kind[cpu.y]));
    if (cpu.a != 0) {
        return;
    }
    lda(0x00);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0xFF);
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    if (cpu.a != 0) {
        negate_x(slot, 0xBEE4);
    }
    lda(0x81);
    WR8(OBJ->frame[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_FlyOffWhenSourceGone101C(void)
{
    FlyOffWhenSourceGone101C(cpu.x);
}

/*
 * @routine 0F6C:797 SpeedByDistance0F6C
 * @run 70:BF97
 * WHAT IT DOES: An X speed by how far the player is (four bands), reversed when facing left (secret stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpeedByDistance0F6C(uint8_t slot)
{
    near_x(slot, 0xBF9A);
    cpu.y = 0;
    cmp(cpu.a, 0x48);
    if (cpu.a >= 0x48) {
        cpu.y = (uint8_t)(cpu.y + 2);
        abi_nz(cpu.y);
        cmp(cpu.a, 0x58);
        if (cpu.a >= 0x58) {
            cpu.y = (uint8_t)(cpu.y + 2);
            abi_nz(cpu.y);
            cmp(cpu.a, 0x68);
            if (cpu.a >= 0x68) {
                cpu.y = (uint8_t)(cpu.y + 2);
                abi_nz(cpu.y);
            }
        }
    }
    lda(rd8((uint16_t)(0xBFC2 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xBFC3 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    if (cpu.a == 0) {
        return;
    }
    negate_exit_shim(NegateVelocityX(slot));
}

void abi_SpeedByDistance0F6C(void)
{
    SpeedByDistance0F6C(cpu.x);
}

/*
 * @routine 0EBB:490 DebrisBelow0EBB
 * @run 70:B490
 * WHAT IT DOES: Sound $23 and the debris at $A90B eight dots below, then stops at Y $9A (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DebrisBelow0EBB(uint8_t slot)
{
    lda(0x9A);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    push8(cpu.a);
    add_a(0x08, false);
    WR8(OBJ->y_lo[slot], cpu.a);
    sound(0x23, 0xB4A4);
    cpu.a = 0;
    call_routine(0xB4A8, 0xA90B);
    lda(pull8());
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->work_k[slot], 0);
}

void abi_DebrisBelow0EBB(void)
{
    DebrisBelow0EBB(cpu.x);
}
