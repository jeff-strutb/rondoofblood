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
#include "calls_b20.h"

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
 * @routine 161C:732 BoxByPreviousFrame161C
 * @run 70:BF32
 * WHAT IT DOES: Hit-tested while the previous slot shows frames $10-$16, in its place and facing, with that frame's box offset.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BoxByPreviousFrame161C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    lda(RD8_IDX((uint16_t)(COL(OBJ->frame) - 1), slot));
    cmp(cpu.a, 0x10);
    if (cpu.a >= 0x10) {
        cmp(cpu.a, 0x17);
        if (cpu.a < 0x17) {
            lda(0x02);
            WR8(OBJ->hitbox_on[slot], cpu.a);
            lda(cpu.x);
            cpu.y = cpu.a;
            cpu.y = (uint8_t)(cpu.y - 1);
            abi_nz(cpu.y);
            jsr_push(0xBF50);
            copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
            jsr_pop();
            lda(RD8_IDX((uint16_t)(COL(OBJ->facing_left) - 1), slot));
            WR8(OBJ->facing_left[slot], cpu.a);
            lda(RD8_IDX((uint16_t)(COL(OBJ->frame) - 1), slot));
            add_a((uint8_t)~0x10, true);
            word_index(cpu.a);
            lda(rd8((uint16_t)(0xBF6F + cpu.y)));
            WR8(OBJ->hitbox_dx[slot], cpu.a);
            lda(rd8((uint16_t)(0xBF70 + cpu.y)));
            WR8(OBJ->hitbox_dy[slot], cpu.a);
            return;
        }
    }
    WR8(OBJ->hitbox_on[slot], 0);
}

void abi_BoxByPreviousFrame161C(void)
{
    BoxByPreviousFrame161C(cpu.x);
}

/*
 * @routine 0B4C:70C FlyFromSlot10_0B4C
 * @run 70:BF0C
 * WHAT IT DOES: Held a step ahead of slot $10 by the facing, its box sized by its frame.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FlyFromSlot10_0B4C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    step_list(slot, 0xBF4C, false, 0xBF1F);
    ldy(0x10);
    jsr_push(0xBF24);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xBF4A + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x08);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xBF35);
    lda(RD8(OBJ->frame[slot]));
    add_a((uint8_t)~0x01, true);
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBF51 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xBF52 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_FlyFromSlot10_0B4C(void)
{
    FlyFromSlot10_0B4C(cpu.x);
}

/*
 * @routine 101E:0B2 StartLeaderAndTrail101E
 * @run 71:C8B2
 * WHAT IT DOES: Loads, fills the 128-byte trail with $40 and starts fifteen numbered kind $C2 pieces in the next slots.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartLeaderAndTrail101E(uint8_t slot)
{
    load_record(slot, 0xC8B5);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a != 0) {
        lda(0x80);
        WR8(OBJ->sprite_flip[slot], cpu.a);
    }
    lda(0x48);
    WR8(OBJ->work_k[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    cpu.y = 0;
    do {
        wr8((uint16_t)(0xC7FC + cpu.y), cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x80);
    } while (cpu.y < 0x80);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    lda(0x0F);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    do {
        clear_slot_in_y(0xC8DE);
        start_at_place(0xC2, 0xC8E3);
        lda(RD8(ZP->temp01));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        abi_nz(cpu.y);
        INC8(ZP->temp01);
        abi_nz(DEC8(ZP->temp00));
    } while ((cpu.p & P_Z) == 0);
}

void abi_StartLeaderAndTrail101E(void)
{
    StartLeaderAndTrail101E(cpu.x);
}

/*
 * @routine 117F:3D8 HoldThenDrop117F
 * @run 71:D3D8
 * WHAT IT DOES: Holds the player (forced pad) for OBJ_WORK_D counts, then a list channel and a kind $34 object off the left.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void HoldThenDrop117F(uint8_t slot)
{
    lda(0x10);
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x02);
    wr8(PlayerStartPose, cpu.a);
    WR8(OBJ->work_f[0], 0);
    wr8(HitAttackFlags, 0);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    push8(cpu.x);
    list_channel2(0x83, 0x05, 0xD3F5);
    pull_x();
    lda(cpu.x);
    add_a(0x03, false);
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    clear_slot_in_y(0xD3FE);
    start_at_place(0x34, 0xD403);
    lda(0xFF);
    WR8(OBJ->x_hi[cpu.y], cpu.a);
    lda(0xE0);
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    lda(0x01);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_HoldThenDrop117F(void)
{
    HoldThenDrop117F(cpu.x);
}

/*
 * @routine 18DC:373 DriftSpitting18DC
 * @run 70:BB73
 * WHAT IT DOES: Drifts, speeding by the facing, leaving a kind $2A object every fourth count of OBJ_WORK_D.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DriftSpitting18DC(uint8_t slot)
{
    step_list(slot, 0xBBB1, true, 0xBB7E);
    move(slot, 0xBB81);
    lda(RD8(OBJ->facing_left[slot]));
    word_index(cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    add_a(rd8((uint16_t)(0xBB6F + cpu.y)), false);
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    add_a(rd8((uint16_t)(0xBB70 + cpu.y)), (cpu.p & P_C) != 0);
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    if (!find_from30(0xBBA3)) {
        return;
    }
    start_at_place(0x2A, 0xBBAA);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_DriftSpitting18DC(void)
{
    DriftSpitting18DC(cpu.x);
}

/*
 * @routine 12DB:643 SinkToB0_12DB
 * @run 70:B643
 * WHAT IT DOES: Sinks faster each count; at Y $B0 stops, a wait of $20, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SinkToB0_12DB(uint8_t slot)
{
    step_list(slot, 0xB682, true, 0xB64E);
    move(slot, 0xB651);
    call_routine(0xB654, 0xB687);
    call_routine(0xB657, 0xB6A1);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x18, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xB0);
    if (cpu.a < 0xB0) {
        return;
    }
    lda(0xB0);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(0x20);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SinkToB0_12DB(void)
{
    SinkToB0_12DB(cpu.x);
}

/*
 * @routine 09EC:768 RainKind9D_09EC
 * @run 70:BF68
 * WHAT IT DOES: Moves to a random X; every other count starts a kind $9D object (a quarter of them in the other slots).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RainKind9D_09EC(uint8_t slot)
{
    INC8(OBJ->work_d[slot]);
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xBF73);
        WR8(OBJ->x_hi[slot], 0);
        WR8(OBJ->y_hi[slot], 0);
        lda(0xC0);
        WR8(OBJ->y_lo[slot], cpu.a);
        return;
    }
    random_a(0xBF82);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        return;
    }
    random_a(0xBF8F);
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        if (!find_from30(0xBF96)) {
            return;
        }
    } else if (!find_from50(0xBFA0)) {
        return;
    }
    lda(0x9D);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
}

void abi_RainKind9D_09EC(void)
{
    RainKind9D_09EC(cpu.x);
}

/*
 * @routine 101B:1A8 BoxBesidePrevious101B
 * @run 70:B1A8
 * WHAT IT DOES: In the previous slot's place; a box of $0C while that slot shows frames $06-$15.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BoxBesidePrevious101B(uint8_t slot)
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
    jsr_push(0xB1B6);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8_IDX((uint16_t)(COL(OBJ->facing_left) - 1), slot));
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(RD8_IDX((uint16_t)(COL(OBJ->frame) - 1), slot));
    cmp(cpu.a, 0x06);
    if (cpu.a >= 0x06) {
        cmp(cpu.a, 0x16);
        if (cpu.a < 0x16) {
            add_a((uint8_t)~0x06, true);
            word_index(cpu.a);
            lda(rd8((uint16_t)(0xB1E8 + cpu.y)));
            WR8(OBJ->hitbox_dx[slot], cpu.a);
            lda(rd8((uint16_t)(0xB1E9 + cpu.y)));
            WR8(OBJ->hitbox_dy[slot], cpu.a);
            lda(0x0C);
            WR8(OBJ->hitbox_half_w[slot], cpu.a);
            WR8(OBJ->hitbox_half_h[slot], cpu.a);
            return;
        }
    }
    WR8(OBJ->hitbox_half_w[slot], 0);
    WR8(OBJ->hitbox_half_h[slot], 0);
}

void abi_BoxBesidePrevious101B(void)
{
    BoxBesidePrevious101B(cpu.x);
}

/*
 * @routine 101D:025 LoadSegmentByArg101D
 * @run 71:C025
 * WHAT IT DOES: With the player on one side only: loads with its box and delay by the start argument.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadSegmentByArg101D(uint8_t slot)
{
    jsr_push(0xC028);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        ClearObject(slot);
        return;
    }
    load_record(slot, 0xC032);
    lda(RD8(OBJ->start_arg[slot]));
    for (int i = 0; i < 4; i++) {
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
    }
    WR8(OBJ->work_k[slot], cpu.a);
    lda(0xA0);
    WR8(OBJ->work_i[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    word_index((uint8_t)(cpu.a << 1));
    lda(rd8((uint16_t)(0xC065 + cpu.y)));
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(rd8((uint16_t)(0xC066 + cpu.y)));
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(rd8((uint16_t)(0xC067 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xC068 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    lda(0x03);
    WR8(OBJ->work_a[0x11], cpu.a);
}

void abi_LoadSegmentByArg101D(void)
{
    LoadSegmentByArg101D(cpu.x);
}

/*
 * @routine 12DE:743 FallingBit12DE
 * @run 71:CF43
 * WHAT IT DOES: Starts above the screen with one of two frames and falls until below the first screen.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallingBit12DE(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xCF4B);
        WR8(OBJ->x_hi[slot], 0);
        lda(0xFF);
        WR8(OBJ->y_hi[slot], cpu.a);
        lda(0xF0);
        WR8(OBJ->y_lo[slot], cpu.a);
        random_a(0xCF5B);
        lda((uint8_t)(cpu.a & 0x01));
        add_a(0x8D, false);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x18, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xCF78);
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0x01);
    if (cpu.a != 0x01) {
        return;
    }
    ClearObject(slot);
}

void abi_FallingBit12DE(void)
{
    FallingBit12DE(cpu.x);
}

/*
 * @routine 161C:34B LoadWithPartner161C
 * @run 70:BB4B
 * WHAT IT DOES: Loads with a kind $48 partner, health by the start argument; an odd one starts in state 4.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadWithPartner161C(uint8_t slot)
{
    load_record(slot, 0xBB4E);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xBB54);
    start_at_place(0x48, 0xBB59);
    face_by_side(slot, COL(OBJ->facing_left), 0xBB5C);
    lda(0x50);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    for (int i = 0; i < 4; i++) {
        abi_flag(P_C, (cpu.a & 0x01) != 0);
        lda((uint8_t)(cpu.a >> 1));
    }
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBB8B + cpu.y)));
    WR8(OBJ->health[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a == 0) {
        return;
    }
    lda(0x01);
    WR8(OBJ->work_k[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_LoadWithPartner161C(void)
{
    LoadWithPartner161C(cpu.x);
}

/*
 * @routine 18DC:039 ChooseByEdge18DC
 * @run 70:B839
 * WHAT IT DOES: Faces the player; short of the edge picks the next substate, else state 6 with OBJ_WORK_F one less.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChooseByEdge18DC(uint8_t slot)
{
    face_by_side(slot, COL(OBJ->facing_left), 0xB83C);
    lda(RD8(OBJ->x_hi[slot]));
    if (cpu.a == 0) {
        bool inside;

        lda(RD8(OBJ->facing_left[slot]));
        if (cpu.a == 0) {
            lda(RD8(OBJ->x_lo[slot]));
            cmp(cpu.a, 0x30);
            inside = cpu.a < 0x30;
        } else {
            lda(RD8(OBJ->x_lo[slot]));
            cmp(cpu.a, 0xD0);
            inside = cpu.a < 0xD0;
        }
        if (inside) {
            WR8(OBJ->work_i[slot], 0);
            lda(RD8(OBJ->work_h[slot]));
            lda((uint8_t)(cpu.a & 0x07));
            cpu.y = cpu.a;
            abi_nz(cpu.y);
            lda(rd8((uint16_t)(0xB87A + cpu.y)));
            WR8(OBJ->substate[slot], cpu.a);
            abi_nz(INC8(OBJ->work_h[slot]));
            return;
        }
    }
    lda(0x06);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    abi_nz(DEC8(OBJ->work_f[slot]));
}

void abi_ChooseByEdge18DC(void)
{
    ChooseByEdge18DC(cpu.x);
}

/*
 * @routine 0B4C:1F6 ChargeThenBrake0B4C
 * @run 70:B9F6
 * WHAT IT DOES: Runs the list at $BA3C moving two steps a frame; at its end sound $B6, a slower speed, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChargeThenBrake0B4C(uint8_t slot)
{
    step_list(slot, 0xBA3C, false, 0xBA01);
    if (list_ended()) {
        sound(0xB6, 0xBA19);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->work_f[slot], cpu.a);
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xBA3A + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        WR8(OBJ->vx_lo[slot], 0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->vy_lo[slot], 0);
        lda(0x19);
        WR8(OBJ->frame[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xBA38 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    call_routine(0xBA11, 0xB7CC);
    run_routine(0xB7CC);
}

void abi_ChargeThenBrake0B4C(void)
{
    ChargeThenBrake0B4C(cpu.x);
}

/*
 * @routine 10CC:0C9 WrapToTop10CC
 * @run 70:B8C9
 * WHAT IT DOES: Once above the screen before, comes back at the top at one of two places, state 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WrapToTop10CC(uint8_t slot)
{
    move_y(slot, 0xB8CC);
    lda(RD8(OBJ->y_hi[slot]));
    cmp(cpu.a, 0xFE);
    if (cpu.a != 0xFE) {
        return;
    }
    lda(0x80);
    WR8(OBJ->scroll_band[slot], cpu.a);
    wr8(zRasterStage, 0);
    WR8(OBJ->x_hi[slot], 0);
    lda(rd8(ScrollXHiScreen));
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
    lda((uint8_t)(cpu.a & 0x01));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xB90B + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0xFF);
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(0xC0);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(0x98);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    wr8(zRasterStage, 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->substate[slot], 0);
}

void abi_WrapToTop10CC(void)
{
    WrapToTop10CC(cpu.x);
}

/*
 * @routine 10CC:1C0 LandStartShield10CC
 * @run 70:B9C0
 * WHAT IT DOES: On landing: two kind $2D objects in slots $11-$12, hit-tested and armed, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandStartShield10CC(uint8_t slot)
{
    call_routine(0xB9C3, 0xBA02);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    ldy(0x11);
    clear_slot_in_y(0xB9CA);
    start_at_place(0x2D, 0xB9CF);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xB9D3);
    start_at_place(0x2D, 0xB9D8);
    lda(0x01);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    lda(0x03);
    WR8(OBJ->work_a[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->work_c[slot], cpu.a);
    lda(0x16);
    WR8(OBJ->work_b[slot], cpu.a);
    lda(0x87);
    WR8(OBJ->pace[slot], cpu.a);
    WR8(OBJ->work_g[slot], 0);
    lda(0x80);
    WR8(OBJ->work_f[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LandStartShield10CC(void)
{
    LandStartShield10CC(cpu.x);
}

/*
 * @routine 117F:600 ListThrow117F
 * @run 71:D600
 * WHAT IT DOES: Throws kind $36 at frame $25, OBJ_WORK_K more times, then goes to $D65D.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThrow117F(uint8_t slot)
{
    step_list(slot, 0xD642, true, 0xD60B);
    if (list_ended()) {
        lda(RD8(OBJ->work_k[slot]));
        if (cpu.a == 0) {
            run_routine(0xD65D);
            return;
        }
        DEC8(OBJ->work_k[slot]);
        lda(0x06);
        WR8(OBJ->anim_step[slot], cpu.a);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x25);
    if (cpu.a != 0x25) {
        return;
    }
    sound(0x1B, 0xD61B);
    sound(0xE1, 0xD620);
    if (!find_from30(0xD623)) {
        return;
    }
    start_at_place(0x36, 0xD62A);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_ListThrow117F(void)
{
    ListThrow117F(cpu.x);
}

/*
 * @routine 161B:255 BobUntilRandom161B
 * @run 70:B255
 * WHAT IT DOES: Bobs by the cosine of OBJ_WORK_D until a 1-in-256 chance: then a wait of 8-39, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BobUntilRandom161B(uint8_t slot)
{
    step_list(slot, 0xB298, true, 0xB260);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xB26E);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct2));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8(zProduct1));
    WR8(OBJ->vy_lo[slot], cpu.a);
    move(slot, 0xB27B);
    lda(RD8(OBJ->work_d[slot]));
    add_a(0x02, false);
    WR8(OBJ->work_d[slot], cpu.a);
    random_a(0xB287);
    if (cpu.a != 0) {
        return;
    }
    random_a(0xB28C);
    lda((uint8_t)(cpu.a & 0x1F));
    add_a(0x08, false);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_BobUntilRandom161B(void)
{
    BobUntilRandom161B(cpu.x);
}

/*
 * @routine 0EBA:7D5 ExitByDoor0EBA
 * @run 70:AFD5
 * WHAT IT DOES: Waits for the player within $60, fades the CD music; 128 counts on asks the mode for what comes next (zFieldA3 9).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ExitByDoor0EBA(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xAFDD);
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x03);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    lda((uint8_t)(cpu.a - 1));
    if (cpu.a == 0) {
        near_x(slot, 0xAFEE);
        cmp(cpu.a, 0x60);
        if (cpu.a >= 0x60) {
            return;
        }
        push8(cpu.x);
        lda(0x0C);
        jsr_push(0xAFF8);
        (void)vec_cd_fade(0x0C);
        cd_fade_exit_shim(0x0C);
        jsr_pop();
        pull_x();
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    DEC8(OBJ->work_d[slot]);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(rd8(0x26EF));
    lda((uint8_t)(cpu.a & 0xFC));
    wr8(0x26EF, cpu.a);
    wr8(zModeEvents, (uint8_t)(rd8(zModeEvents) | 0x02));
    lda(0x09);
    wr8(zFieldA3, cpu.a);
}

void abi_ExitByDoor0EBA(void)
{
    ExitByDoor0EBA(cpu.x);
}

/*
 * @routine 161F:675 ExitByDoor161F
 * @run 71:D675
 * WHAT IT DOES: Waits for the player within $78, fades the CD music; 128 counts on asks the mode for what comes next (zFieldA3 8).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ExitByDoor161F(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xD67D);
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x37);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    lda((uint8_t)(cpu.a - 1));
    if (cpu.a == 0) {
        near_x(slot, 0xD68E);
        cmp(cpu.a, 0x78);
        if (cpu.a >= 0x78) {
            return;
        }
        push8(cpu.x);
        lda(0x0C);
        jsr_push(0xD698);
        (void)vec_cd_fade(0x0C);
        cd_fade_exit_shim(0x0C);
        jsr_pop();
        pull_x();
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    DEC8(OBJ->work_d[slot]);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(rd8(0x26EF));
    lda((uint8_t)(cpu.a & 0xF8));
    wr8(0x26EF, cpu.a);
    wr8(zModeEvents, (uint8_t)(rd8(zModeEvents) | 0x02));
    lda(0x08);
    wr8(zFieldA3, cpu.a);
}

void abi_ExitByDoor161F(void)
{
    ExitByDoor161F(cpu.x);
}
