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
#include "calls_b6.h"

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

/*
 * @routine 13FC:256 LoadTurnAndRise13FC
 * @run 70:BA56
 * WHAT IT DOES: Loads the record, OBJ_WORK_K opposite the next slot's, moves 12 dots left and rises (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadTurnAndRise13FC(uint8_t slot)
{
    load_record(slot, 0xBA59);
    lda(RD8(OBJ->work_k[(uint8_t)(slot + 1)]));
    add_a(0x80, false);
    WR8(OBJ->work_k[slot], cpu.a);
    lda(0xF4);
    WR8(OBJ->vx_hi[slot], cpu.a);
    move_x(slot, 0xBA6A);
    lda(0xFE);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
}

void abi_LoadTurnAndRise13FC(void)
{
    LoadTurnAndRise13FC(cpu.x);
}

/*
 * @routine 13FC:275 ListRiseUntil13FC_275
 * @run 70:BA75
 * WHAT IT DOES: Plays the list at $BA94 while moving along Y; above $20 the animation restarted and the state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListRiseUntil13FC_275(uint8_t slot)
{
    step_list(slot, 0xBA94, true, 0xBA80);
    move_y(slot, 0xBA83);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x20);
    if (cpu.a >= 0x20) {
        return;
    }
    restart_animation(slot);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListRiseUntil13FC_275(void)
{
    ListRiseUntil13FC_275(cpu.x);
}

/*
 * @routine 13FD:15F ListRiseUntil13FD_15F
 * @run 71:C15F
 * WHAT IT DOES: Plays the list at $C17E while moving along Y; above $40 the animation restarted and the state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListRiseUntil13FD_15F(uint8_t slot)
{
    step_list(slot, 0xC17E, true, 0xC16A);
    move_y(slot, 0xC16D);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x40);
    if (cpu.a >= 0x40) {
        return;
    }
    restart_animation(slot);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListRiseUntil13FD_15F(void)
{
    ListRiseUntil13FD_15F(cpu.x);
}

/*
 * @routine 18DB:4F1 AccelerateUp18DB
 * @run 70:B4F1
 * WHAT IT DOES: Moves along Y while the speed grows upward; above Y $4C the substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AccelerateUp18DB(uint8_t slot)
{
    Sum sum;

    move_y(slot, 0xB4F4);
    lda(RD8(OBJ->vy_lo[slot]));
    sum = sub_bytes(cpu.a, 0x0C, true);
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
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x4C);
    if (cpu.a >= 0x4C) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_AccelerateUp18DB(void)
{
    AccelerateUp18DB(cpu.x);
}

/*
 * @routine 18DC:1BF SoundAndClearThree18DC
 * @run 70:B9BF
 * WHAT IT DOES: Sound $92, the hit box off, slots $11 to $13 cleared, the animation restarted, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundAndClearThree18DC(uint8_t slot)
{
    WR8(OBJ->work_j[slot], 0);
    sound(0x92, 0xB9C7);
    WR8(OBJ->hitbox_on[slot], 0);
    cpu.y = 0x11;
    abi_nz(cpu.y);
    do {
        clear_slot_in_y(0xB9CF);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x14);
    } while (cpu.y < 0x14);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundAndClearThree18DC(void)
{
    SoundAndClearThree18DC(cpu.x);
}

/*
 * @routine 072C:4B9 FallThenBecome22_072C
 * @run 70:BCB9
 * WHAT IT DOES: Adds A to the Y speed and moves; two screens down the slot becomes kind $22 (secret stage 03's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallThenBecome22_072C(uint8_t slot)
{
    add_a(0x18, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xBCC9);
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0x02);
    if (cpu.a < 0x02) {
        return;
    }
    clear_this(slot, 0xBCD3);
    lda(0x22);
    WR8(OBJ->kind[slot], cpu.a);
}

void abi_FallThenBecome22_072C(void)
{
    FallThenBecome22_072C(cpu.x);
}

/*
 * @routine 088A:5D8 ToState11WhenHit088A
 * @run 70:ADD8
 * WHAT IT DOES: In states the table allows, a hit of 5 or more: $B784, sound $54, state $11 (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ToState11WhenHit088A(uint8_t slot)
{
    cpu.y = RD8(OBJ->state[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xADF8 + cpu.y)));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    call_routine(0xADEA, 0xB784);
    sound(0x54, 0xADEF);
    lda(0x11);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_ToState11WhenHit088A(void)
{
    ToState11WhenHit088A(cpu.x);
}

/*
 * @routine 0EBE:026 ListFaceThenNear0EBE
 * @run 71:C826
 * WHAT IT DOES: Plays the list at $C846 facing the player; within $60 dots state 3 (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListFaceThenNear0EBE(uint8_t slot)
{
    step_list(slot, 0xC846, false, 0xC831);
    face_by_side(slot, COL(OBJ->facing_left), 0xC834);
    jsr_push(0xC83C);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
    cmp(cpu.a, 0x60);
    if (cpu.a >= 0x60) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ListFaceThenNear0EBE(void)
{
    ListFaceThenNear0EBE(cpu.x);
}

/*
 * @routine 101B:09C LoadAndStart24_101B
 * @run 70:B09C
 * WHAT IT DOES: Loads the record facing left, OBJ_WORK_L half the start argument, a kind $24 object in the next slot (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAndStart24_101B(uint8_t slot)
{
    load_record(slot, 0xB09F);
    lda(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    WR8(OBJ->work_l[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xB0B1);
    start_at_place(0x24, 0xB0B6);
    lda(0x30);
    WR8(OBJ->work_h[slot], cpu.a);
}

void abi_LoadAndStart24_101B(void)
{
    LoadAndStart24_101B(cpu.x);
}

/*
 * @routine 12DA:773 RunThenFall12DA
 * @run 70:AF73
 * WHAT IT DOES: The routine at $AA35 for point $10, $26, then a fall of $10 along Y (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RunThenFall12DA(uint8_t slot)
{
    lda(0x10);
    WR8(ZP->temp08, cpu.a);
    lda(0x26);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xAF7E, 0xAA35);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x10, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xAF92);
}

void abi_RunThenFall12DA(void)
{
    RunThenFall12DA(cpu.x);
}

/*
 * @routine 12DB:06D CountScriptAndChannel12DB
 * @run 70:B06D
 * WHAT IT DOES: Counts OBJ_WORK_D; at zero script $86 and list channel $A9, the count 96, state 1 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountScriptAndChannel12DB(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    push8(cpu.x);
    lda(0x86);
    cpu.x = 0x05;
    abi_nz(cpu.x);
    jsr_push(0xB07A);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
    list_channel2(0xA9, 0x01, 0xB081);
    cpu.x = pull8();
    abi_nz(cpu.x);
    lda(0x60);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_CountScriptAndChannel12DB(void)
{
    CountScriptAndChannel12DB(cpu.x);
}

/*
 * @routine 12DB:2C6 LoadRiseAndScript12DB
 * @run 70:B2C6
 * WHAT IT DOES: Loads the record 32 dots up, stops, script $8A, OBJ_WORK_E 32 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadRiseAndScript12DB(uint8_t slot)
{
    load_record(slot, 0xB2C9);
    lda(0xE0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xB2D1);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    script_kept_x(0x8A, 0x05, 0xB2DF);
    lda(0x20);
    WR8(OBJ->work_e[slot], cpu.a);
}

void abi_LoadRiseAndScript12DB(void)
{
    LoadRiseAndScript12DB(cpu.x);
}

/*
 * @routine 12DD:66A ListThenShoot12DD
 * @run 71:C66A
 * WHAT IT DOES: Plays the list at $C68A; then a rising kind $8B object in slot $30 and this one cleared (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenShoot12DD(uint8_t slot)
{
    step_list(slot, 0xC68A, true, 0xC675);
    if (!list_ended()) {
        return;
    }
    cpu.y = 0x30;
    abi_nz(cpu.y);
    clear_slot_in_y(0xC67C);
    start_at_place(0x8B, 0xC681);
    lda(0xFC);
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    clear_this(slot, 0xC689);
}

void abi_ListThenShoot12DD(void)
{
    ListThenShoot12DD(cpu.x);
}

/*
 * @routine 12DE:3F2 SoundAndNextOfCycle12DE
 * @run 71:CBF2
 * WHAT IT DOES: Sound $7C, the animation restarted, OBJ_WORK_G 40, the next substate of a cycle (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundAndNextOfCycle12DE(uint8_t slot)
{
    sound(0x7C, 0xCBF7);
    restart_animation(slot);
    lda(0x28);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xCC12 + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    abi_nz(INC8(OBJ->work_h[slot]));
}

void abi_SoundAndNextOfCycle12DE(void)
{
    SoundAndNextOfCycle12DE(cpu.x);
}

/*
 * @routine 18DC:2EB LoadSpeedByFacingAndSlot18DC
 * @run 70:BAEB
 * WHAT IT DOES: Loads the record, X speed by facing and Y speed by slot from $30, moves, sound $4D (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadSpeedByFacingAndSlot18DC(uint8_t slot)
{
    Sum sum;

    load_record(slot, 0xBAEE);
    cpu.y = RD8(OBJ->facing_left[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBB0B + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(cpu.x);
    sum = sub_bytes(cpu.a, 0x30, true);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xBB0D + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xBB05);
    sound(0x4D, 0xBB0A);
}

void abi_LoadSpeedByFacingAndSlot18DC(void)
{
    LoadSpeedByFacingAndSlot18DC(cpu.x);
}

/*
 * @routine 05CB:6A0 ClearPairAndStop05CB
 * @run 70:B6A0
 * WHAT IT DOES: Clears slots $11 and $12, stops with no hit box, OBJ_WORK_E 64, substate on (secret stage 02's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ClearPairAndStop05CB(uint8_t slot)
{
    cpu.y = 0x11;
    abi_nz(cpu.y);
    clear_slot_in_y(0xB6A5);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xB6A9);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    restart_animation(slot);
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ClearPairAndStop05CB(void)
{
    ClearPairAndStop05CB(cpu.x);
}

/*
 * @routine 0E0B:6F7 Frame44AndChannel0E0B
 * @run 70:B6F7
 * WHAT IT DOES: Frame $44, the water line off, no hit box, list channel $83, substate on (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void Frame44AndChannel0E0B(uint8_t slot)
{
    lda(0x44);
    WR8(OBJ->frame[slot], cpu.a);
    wr8(RasterWaterWanted + 1, 0);
    wr8(RasterWaterWanted, 0);
    restart_animation(slot);
    WR8(OBJ->hitbox_on[slot], 0);
    push8(cpu.x);
    list_channel2(0x83, 0x05, 0xB713);
    cpu.x = pull8();
    abi_nz(cpu.x);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_Frame44AndChannel0E0B(void)
{
    Frame44AndChannel0E0B(cpu.x);
}

/*
 * @routine 12DB:5C4 ListThenWideBox12DB
 * @run 70:B5C4
 * WHAT IT DOES: Plays the list at $B5E5; then a tall hit box, no Y speed, substate on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenWideBox12DB(uint8_t slot)
{
    step_list(slot, 0xB5E5, true, 0xB5CF);
    if (!list_ended()) {
        return;
    }
    lda(0x08);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x2C);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenWideBox12DB(void)
{
    ListThenWideBox12DB(cpu.x);
}

/*
 * @routine 12DE:4AC ListThenLeap12DE
 * @run 71:CCAC
 * WHAT IT DOES: Plays the list at $CCCD; then leaps straight up, substate on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenLeap12DE(uint8_t slot)
{
    step_list(slot, 0xCCCD, true, 0xCCB7);
    if (!list_ended()) {
        return;
    }
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0xFB);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x20);
    WR8(OBJ->vy_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenLeap12DE(void)
{
    ListThenLeap12DE(cpu.x);
}

/*
 * @routine 13FB:3FF LandAtA8_13FB
 * @run 70:B3FF
 * WHAT IT DOES: Falls to Y $A8: sound $72, held there, raster effect off, substate on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandAtA8_13FB(uint8_t slot)
{
    move_y(slot, 0xB402);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xA8);
    if (cpu.a < 0xA8) {
        return;
    }
    sound(0x72, 0xB413);
    lda(0xA8);
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(ZP->raster_effect, 0);
    WR8(ZP->raster_arg, 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LandAtA8_13FB(void)
{
    LandAtA8_13FB(cpu.x);
}

/*
 * @routine 13FB:725 CycleAndSpawn13FB
 * @run 70:B725
 * WHAT IT DOES: The next of a cycle in OBJ_WORK_I; a 3 starts a kind $42 object in slot $13; substate on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CycleAndSpawn13FB(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB746 + cpu.y)));
    WR8(OBJ->work_i[slot], cpu.a);
    cmp(cpu.a, 0x03);
    if (cpu.a == 0x03) {
        cpu.y = 0x13;
        abi_nz(cpu.y);
        clear_slot_in_y(0xB73A);
        start_at_place(0x42, 0xB73F);
    }
    INC8(OBJ->work_h[slot]);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CycleAndSpawn13FB(void)
{
    CycleAndSpawn13FB(cpu.x);
}

/*
 * @routine 0EBD:577 FallAndDropEmbers0EBD
 * @run 71:C577
 * WHAT IT DOES: Falls, cleared a screen down; every fourth count of OBJ_WORK_E drops a kind $9D object (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallAndDropEmbers0EBD(uint8_t slot)
{
    gravity(slot, 0xC57A);
    move_y(slot, 0xC57D);
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0x01);
    if (cpu.a == 0x01) {
        ClearObject(slot);
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a == 0 && find_from30(0xC591)) {
        start_at_place(0x9D, 0xC598);
    }
}

void abi_FallAndDropEmbers0EBD(void)
{
    FallAndDropEmbers0EBD(cpu.x);
}

/*
 * @routine 101C:44D SpawnAtStep101C
 * @run 70:BC4D
 * WHAT IT DOES: At animation step zTemp00: sound 6 and a kind $30 object with the facing and start argument zTemp01 (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnAtStep101C(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, RD8(ZP->temp00));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    sound(0x06, 0xBC59);
    if (!find_from30(0xBC5C)) {
        return;
    }
    start_at_place(0x30, 0xBC63);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    lda(RD8(ZP->temp01));
    WR8(OBJ->start_arg[cpu.y], cpu.a);
}

void abi_SpawnAtStep101C(void)
{
    SpawnAtStep101C(cpu.x);
}

/*
 * @routine 117F:553 ListSpeedThenCount117F
 * @run 71:D553
 * WHAT IT DOES: Plays the list at $D575, moving on new frames, until OBJ_WORK_D counts out (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSpeedThenCount117F(uint8_t slot)
{
    step_list(slot, 0xD575, true, 0xD55E);
    if ((cpu.p & P_C) != 0) {
        cpu.y = RD8(OBJ->facing_left[slot]);
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xD582 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        call_routine(0xD56C, 0xD46F);
    }
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    run_routine(0xD65D);
}

void abi_ListSpeedThenCount117F(void)
{
    ListSpeedThenCount117F(cpu.x);
}

/*
 * @routine 12DE:04C CountThenSpeed12DE
 * @run 71:C84C
 * WHAT IT DOES: Counts OBJ_WORK_E; then the state on and the X speed of the start argument, reversed when OBJ_WORK_D (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenSpeed12DE(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    INC8(OBJ->state[slot]);
    lda(RD8(OBJ->start_arg[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC86F + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC86E + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(0xC86D);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
}

void abi_CountThenSpeed12DE(void)
{
    CountThenSpeed12DE(cpu.x);
}

/*
 * @routine 13FD:2EB AimWhenBothReady13FD
 * @run 71:C2EB
 * WHAT IT DOES: Once slots $11 and $12 are in substate 9: aims at the player, state on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AimWhenBothReady13FD(uint8_t slot)
{
    AngleExit angle;

    lda(RD8(OBJ->substate[0x11]));
    cmp(cpu.a, 0x09);
    if (cpu.a != 0x09) {
        return;
    }
    lda(RD8(OBJ->substate[0x12]));
    cmp(cpu.a, 0x09);
    if (cpu.a != 0x09) {
        return;
    }
    jsr_push(0xC2FC);
    angle = AngleToPlayer(slot);
    angle_exit_shim(angle);
    jsr_pop();
    WR8(OBJ->work_j[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_AimWhenBothReady13FD(void)
{
    AimWhenBothReady13FD(cpu.x);
}

/*
 * @routine 117D:070 NextRandomMove117D
 * @run 71:C070
 * WHAT IT DOES: The next count from a cycle, the player's side in OBJ_WORK_E and substate 1 or 2 at random (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void NextRandomMove117D(uint8_t slot)
{
    INC8(OBJ->work_f[slot]);
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC093 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
    face_by_side(slot, COL(OBJ->work_e), 0xC082);
    random_a(0xC08A);
    lda((uint8_t)(cpu.a & 0x01));
    add_a(0x01, false);
    WR8(OBJ->substate[slot], cpu.a);
}

void abi_NextRandomMove117D(void)
{
    NextRandomMove117D(cpu.x);
}

/*
 * @routine 12DE:029 MoveSoundThenWait12DE
 * @run 71:C829
 * WHAT IT DOES: Moves along Y while OBJ_WORK_E counts, sounding $46 at 12; then stops, state on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void MoveSoundThenWait12DE(uint8_t slot)
{
    move_y(slot, 0xC82C);
    lda(RD8(OBJ->work_e[slot]));
    cmp(cpu.a, 0x0C);
    if (cpu.a == 0x0C) {
        sound(0x46, 0xC838);
    }
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x20);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_MoveSoundThenWait12DE(void)
{
    MoveSoundThenWait12DE(cpu.x);
}

/*
 * @routine 12DE:2BC LoadWithCompanions12DE
 * @run 71:CABC
 * WHAT IT DOES: Loads the record, a kind $3B object in slot $11, list channel $84, script $8F, OBJ_WORK_F $B0 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadWithCompanions12DE(uint8_t slot)
{
    load_record(slot, 0xCABF);
    cpu.y = 0x11;
    abi_nz(cpu.y);
    clear_slot_in_y(0xCAC4);
    start_at_place(0x3B, 0xCAC9);
    push8(cpu.x);
    list_channel2(0x84, 0x05, 0xCAD1);
    lda(0x8F);
    cpu.x = 0x04;
    abi_nz(cpu.x);
    jsr_push(0xCAD8);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
    cpu.x = pull8();
    abi_nz(cpu.x);
    lda(0xB0);
    WR8(OBJ->work_f[slot], cpu.a);
}

void abi_LoadWithCompanions12DE(void)
{
    LoadWithCompanions12DE(cpu.x);
}

/*
 * @routine 13FC:09F ListThenRaster13FC
 * @run 70:B89F
 * WHAT IT DOES: Plays the list at $B8C2; then raster effect $0B from line $BF, a slow sink, substate on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenRaster13FC(uint8_t slot)
{
    step_list(slot, 0xB8C2, true, 0xB8AA);
    if (!list_ended()) {
        return;
    }
    lda(0x0B);
    WR8(ZP->raster_effect, cpu.a);
    lda(0xBF);
    WR8(ZP->raster_arg, cpu.a);
    lda(0xC0);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->vy_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenRaster13FC(void)
{
    ListThenRaster13FC(cpu.x);
}

/*
 * @routine 13FC:0C7 DissolveSinkThenBecome43_13FC
 * @run 70:B8C7
 * WHAT IT DOES: Dissolves patterns 0 to 5 while sinking and OBJ_WORK_E counts; then the slot becomes kind $43 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DissolveSinkThenBecome43_13FC(uint8_t slot)
{
    lda(0x01);
    WR8(OBJ->work_m[slot], cpu.a);
    lda(0x00);
    wr8(WorkByteA, cpu.a);
    lda(0x06);
    wr8(WorkByteB, cpu.a);
    call_routine(0xB8D9, 0xB8EA);
    move_y(slot, 0xB8DC);
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    lda(0x43);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_DissolveSinkThenBecome43_13FC(void)
{
    DissolveSinkThenBecome43_13FC(cpu.x);
}

/*
 * @routine 13FC:12A FollowSlot16UntilLow13FC
 * @run 70:B92A
 * WHAT IT DOES: In state 0 loads the record with frame $7B; later keeps slot $10's place and is cleared below $A8 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FollowSlot16UntilLow13FC(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xB932);
        lda(0x7B);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    cpu.y = 0x10;
    abi_nz(cpu.y);
    jsr_push(0xB93D);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xA8);
    if (cpu.a < 0xA8) {
        return;
    }
    clear_this(slot, 0xB94C);
}

void abi_FollowSlot16UntilLow13FC(void)
{
    FollowSlot16UntilLow13FC(cpu.x);
}

/*
 * @routine 18DB:7EE SpawnInRow64_18DB
 * @run 70:B7EE
 * WHAT IT DOES: On even counts of OBJ_WORK_D, a kind $2E object in the first empty slot from $40, with the facing (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnInRow64_18DB(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        return;
    }
    cpu.y = 0x40;
    abi_nz(cpu.y);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (cpu.a == 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x50);
        if (cpu.y >= 0x50) {
            return;
        }
    }
    clear_slot_in_y(0xB805);
    start_at_place(0x2E, 0xB80A);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_SpawnInRow64_18DB(void)
{
    SpawnInRow64_18DB(cpu.x);
}

/*
 * @routine 0B4C:2BD SpawnWithRandom0B4C
 * @run 70:BABD
 * WHAT IT DOES: Every fourth count of OBJ_WORK_H: sound $19 and a kind $9A object with a random OBJ_WORK_E (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnWithRandom0B4C(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    sound(0x19, 0xBAC9);
    if (!find_from20(0xBACC)) {
        return;
    }
    start_at_place(0x9A, 0xBAD3);
    lda(0x01);
    WR8(OBJ->work_d[cpu.y], cpu.a);
    random_a(0xBADB);
    lda((uint8_t)(cpu.a & 0x07));
    WR8(OBJ->work_e[cpu.y], cpu.a);
}

void abi_SpawnWithRandom0B4C(void)
{
    SpawnWithRandom0B4C(cpu.x);
}

/*
 * @routine 0EBB:728 SpawnScattered0EBB
 * @run 70:B728
 * WHAT IT DOES: A kind $A9 object near this place, its work and X low random (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnScattered0EBB(uint8_t slot)
{
    if (!find_from20(0xB72B)) {
        return;
    }
    start_at_place(0xA9, 0xB732);
    random_a(0xB735);
    lda((uint8_t)(cpu.a & 0x07));
    WR8(OBJ->work_e[cpu.y], cpu.a);
    lda(0x10);
    WR8(OBJ->sprite_attr[cpu.y], cpu.a);
    random_a(0xB742);
    lda((uint8_t)(cpu.a & 0x0F));
    add_a(RD8(OBJ->x_lo[cpu.y]), false);
    WR8(OBJ->x_lo[cpu.y], cpu.a);
}

void abi_SpawnScattered0EBB(void)
{
    SpawnScattered0EBB(cpu.x);
}

/*
 * @routine 101E:5F4 SpawnInTurn101E
 * @run 71:CDF4
 * WHAT IT DOES: On this slot's frame of eight (slot less $11): sound $18 and a kind $9D object with the flip bits (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnInTurn101E(uint8_t slot)
{
    Sum sum;

    lda(RD8(ZP->wave_counter));
    lda((uint8_t)(cpu.a & 0x07));
    WR8(ZP->temp00, cpu.a);
    lda(cpu.x);
    sum = sub_bytes(cpu.a, 0x11, true);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    cmp(cpu.a, RD8(ZP->temp00));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    if (!find_from30(0xCE05)) {
        return;
    }
    sound(0x18, 0xCE0C);
    start_at_place(0x9D, 0xCE11);
    lda(RD8(OBJ->sprite_flip[slot]));
    WR8(OBJ->sprite_flip[cpu.y], cpu.a);
}

void abi_SpawnInTurn101E(void)
{
    SpawnInTurn101E(cpu.x);
}

/*
 * @routine 0EBF:648 PlaceAtRandom0EBF
 * @run 71:D648
 * WHAT IT DOES: Puts the object at a random place on the first screen from two tables (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PlaceAtRandom0EBF(uint8_t slot)
{
    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->y_hi[slot], 0);
    random_a(0xD651);
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xD66A + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    call_routine(0xD65D, 0xD676);
    random_a(0xD660);
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xD672 + cpu.y)));
    WR8(OBJ->y_lo[slot], cpu.a);
}

void abi_PlaceAtRandom0EBF(void)
{
    PlaceAtRandom0EBF(cpu.x);
}

/*
 * @routine 10CC:66C LoadOrFallList10CC
 * @run 70:BE6C
 * WHAT IT DOES: In state 0 loads the record rising; later plays the list at $BE8C while falling (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadOrFallList10CC(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xBE74);
        lda(0xF6);
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    step_list(slot, 0xBE8C, true, 0xBE85);
    gravity(slot, 0xBE88);
    move_y(slot, 0xBE8B);
}

void abi_LoadOrFallList10CC(void)
{
    LoadOrFallList10CC(cpu.x);
}
