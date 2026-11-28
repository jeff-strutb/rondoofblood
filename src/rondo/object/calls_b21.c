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
#include "calls_b19.h"
#include "calls_b21.h"

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

static void record_event(uint8_t slot, uint16_t ret)
{
    uint8_t stored, event, byte;
    bool marked;

    jsr_push(ret);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    abi_ended(l8_of_event(marked, stored, event, byte));
    jsr_pop();
}

/* ROL of A through the carry. */
static void rol_a(void)
{
    bool out = (cpu.a & 0x80) != 0;

    lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
    abi_flag(P_C, out);
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
 * @routine 101E:151 WalkByTable101E
 * @run 71:C951
 * WHAT IT DOES: Animates and moves across at the speed OBJ_WORK_I picks, turning round at a wall.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkByTable101E(uint8_t slot)
{
    step_list(slot, 0xC999, true, 0xC95C);
    lda(RD8(OBJ->work_i[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC996 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC995 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    move_x(slot, 0xC970);
    lda(0x08);
    WR8(ZP->product.mid, cpu.a);
    lda(0x02);
    jsr_push(0xC979);
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
    jsr_push(0xC992);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocityX(slot));
}

void abi_WalkByTable101E(void)
{
    WalkByTable101E(cpu.x);
}

/*
 * @routine 12DC:440 WalkByTable12DC
 * @run 70:BC40
 * WHAT IT DOES: Animates and moves across at the speed OBJ_WORK_I picks, turning round at a wall.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkByTable12DC(uint8_t slot)
{
    step_list(slot, 0xBC88, true, 0xBC4B);
    lda(RD8(OBJ->work_i[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBC85 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBC84 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    MoveBounceOffWall12DC(slot);
}

void abi_WalkByTable12DC(void)
{
    WalkByTable12DC(cpu.x);
}

/*
 * @routine 12DE:33B WalkByTable12DE
 * @run 71:CB3B
 * WHAT IT DOES: On each new frame of the list moves across at the speed OBJ_WORK_E picks, turning round at a wall.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkByTable12DE(uint8_t slot)
{
    step_list(slot, 0xCB85, true, 0xCB46);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    MoveBounceByTable12DE(slot);
}

void abi_WalkByTable12DE(void)
{
    WalkByTable12DE(cpu.x);
}

/*
 * @routine 13FB:525 WalkByTable13FB
 * @run 70:B525
 * WHAT IT DOES: On each new frame of the list moves across at the speed OBJ_WORK_J picks, turning round at a wall.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkByTable13FB(uint8_t slot)
{
    step_list(slot, 0xB56C, true, 0xB530);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->work_j[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xB57A + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB579 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    move_x(slot, 0xB546);
    lda(0x04);
    WR8(ZP->product.mid, cpu.a);
    lda(0x02);
    jsr_push(0xB54F);
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
    jsr_push(0xB568);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocityX(slot));
}

void abi_WalkByTable13FB(void)
{
    WalkByTable13FB(cpu.x);
}

/*
 * @routine 10CB:7A2 ThrowByFrame10CB
 * @run 70:B7A2
 * WHAT IT DOES: Runs the list at $B7E6, throwing kind $2B at frame $58 and $2C (with sound $E5) at $5C; at its end substate 6.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ThrowByFrame10CB(uint8_t slot)
{
    step_list(slot, 0xB7E6, true, 0xB7AD);
    if (list_ended()) {
        WR8(OBJ->vy_lo[slot], 0);
        WR8(OBJ->vy_hi[slot], 0);
        WR8(OBJ->work_g[slot], 0);
        lda(0x06);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x58);
    if (cpu.a == 0x58) {
        ldy(0x30);
        clear_slot_in_y(0xB7D1);
        start_at_place(0x2B, 0xB7D6);
        return;
    }
    cmp(cpu.a, 0x5C);
    if (cpu.a != 0x5C) {
        return;
    }
    sound(0xE5, 0xB7C1);
    ldy(0x11);
    clear_slot_in_y(0xB7C6);
    start_at_place(0x2C, 0xB7CB);
}

void abi_ThrowByFrame10CB(void)
{
    ThrowByFrame10CB(cpu.x);
}

/*
 * @routine 117F:07E StartSixSegments117F
 * @run 71:D07E
 * WHAT IT DOES: Health 3; starts six numbered kind $3E segments two slots on, lags 8 apart.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartSixSegments117F(uint8_t slot)
{
    restart_animation(slot);
    INC8(OBJ->state[slot]);
    lda(0xF0);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x03);
    WR8(OBJ->health[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    lda(0x06);
    WR8(ZP->temp00, cpu.a);
    lda(0x03);
    WR8(ZP->temp01, cpu.a);
    WR8(ZP->temp02, 0);
    do {
        clear_slot_in_y(0xD0A2);
        start_at_place(0x3E, 0xD0A7);
        lda(cpu.x);
        WR8(OBJ->work_l[cpu.y], cpu.a);
        lda(RD8(ZP->temp01));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        add_a(0x08, false);
        WR8(ZP->temp01, cpu.a);
        lda(RD8(ZP->temp02));
        WR8(OBJ->work_m[cpu.y], cpu.a);
        INC8(ZP->temp02);
        cpu.y = (uint8_t)(cpu.y + 1);
        abi_nz(cpu.y);
        abi_nz(DEC8(ZP->temp00));
    } while ((cpu.p & P_Z) == 0);
}

void abi_StartSixSegments117F(void)
{
    StartSixSegments117F(cpu.x);
}

/*
 * @routine 12DD:69D BoxByLeader12DD
 * @run 71:C69D
 * WHAT IT DOES: Rides slot $10 with a box by its frame ($43-$48); no box otherwise.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BoxByLeader12DD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    lda(RD8(OBJ->frame[0x10]));
    cmp(cpu.a, 0x43);
    if (cpu.a >= 0x43) {
        cmp(cpu.a, 0x49);
        if (cpu.a < 0x49) {
            add_a((uint8_t)~0x43, true);
            word_index((uint8_t)(cpu.a << 1));
            lda(rd8((uint16_t)(0xC6E1 + cpu.y)));
            WR8(OBJ->hitbox_dx[slot], cpu.a);
            lda(rd8((uint16_t)(0xC6E2 + cpu.y)));
            WR8(OBJ->hitbox_dy[slot], cpu.a);
            lda(rd8((uint16_t)(0xC6E3 + cpu.y)));
            WR8(OBJ->hitbox_half_w[slot], cpu.a);
            lda(rd8((uint16_t)(0xC6E4 + cpu.y)));
            WR8(OBJ->hitbox_half_h[slot], cpu.a);
            ldy(0x10);
            jsr_push(0xC6D3);
            copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
            jsr_pop();
            lda(RD8(OBJ->facing_left[0x10]));
            WR8(OBJ->facing_left[slot], cpu.a);
            return;
        }
    }
    WR8(OBJ->hitbox_half_w[slot], 0);
    WR8(OBJ->hitbox_half_h[slot], 0);
}

void abi_BoxByLeader12DD(void)
{
    BoxByLeader12DD(cpu.x);
}

/*
 * @routine 16CC:086 LoadWithLimbs16CC
 * @run 70:B886
 * WHAT IT DOES: Loads above the screen, counted in slot $10, with five kind $24 and two kind $25 limbs.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadWithLimbs16CC(uint8_t slot)
{
    load_record(slot, 0xB889);
    INC8(OBJ->work_l[0x10]);
    WR8(OBJ->x_hi[slot], 0);
    lda(0xA0);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0xFF);
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(0xA0);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x60);
    WR8(OBJ->vy_lo[slot], cpu.a);
    ldy(0x11);
    do {
        clear_slot_in_y(0xB8AD);
        start_at_place(0x24, 0xB8B2);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x16);
    } while (cpu.y < 0x16);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xB8BB);
    start_at_place(0x25, 0xB8C0);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xB8C4);
    start_at_place(0x25, 0xB8C9);
}

void abi_LoadWithLimbs16CC(void)
{
    LoadWithLimbs16CC(cpu.x);
}

/*
 * @routine 088A:56A RunAndRecordPath088A
 * @run 70:AD6A
 * WHAT IT DOES: Runs the overlay's checks and state, records the place in two 64-byte paths, and starts the list channel for the frame.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RunAndRecordPath088A(uint8_t slot)
{
    INC8(OBJ->work_j[slot]);
    call_routine(0xAD70, 0xADAF);
    call_routine(0xAD73, 0xADD8);
    call_routine(0xAD76, 0xAE10);
    call_routine(0xAD79, 0xAE3F);
    call_routine(0xAD7C, 0xAE61);
    ldy(0x3F);
    do {
        lda(rd8((uint16_t)(0xBB65 + cpu.y)));
        wr8((uint16_t)(0xBB66 + cpu.y), cpu.a);
        lda(rd8((uint16_t)(0xBBA5 + cpu.y)));
        wr8((uint16_t)(0xBBA6 + cpu.y), cpu.a);
        cpu.y = (uint8_t)(cpu.y - 1);
        abi_nz(cpu.y);
    } while (cpu.y != 0);
    lda(RD8(OBJ->x_lo[slot]));
    wr8(0xBB66, cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    wr8(0xBBA6, cpu.a);
    lda(RD8(OBJ->work_l[slot]));
    if (cpu.a != 0) {
        return;
    }
    push8(cpu.x);
    lda(RD8(OBJ->frame[slot]));
    add_a((uint8_t)~0x01, true);
    add_a(0x85, false);
    list_channel2(cpu.a, 0x05, 0xADAD);
    pull_x();
}

void abi_RunAndRecordPath088A(void)
{
    RunAndRecordPath088A(cpu.x);
}

/*
 * @routine 101B:483 SplitInTwo101B
 * @run 70:B483
 * WHAT IT DOES: Replaced by two kind $2D halves, each with its argument, wait and speeds.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SplitInTwo101B(uint8_t slot)
{
    cpu.y = 0;
    do {
        wr8(zHitSlot, cpu.y);
        if (!find_from30(0xB489)) {
            break;
        }
        start_at_place(0x2D, 0xB490);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        push8(cpu.x);
        cpu.x = rd8(zHitSlot);
        abi_nz(cpu.x);
        lda(rd8((uint16_t)(0xB4C8 + cpu.x)));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB4CA + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB4CC + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB4CE + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB4D0 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xB4D2 + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        pull_x();
        ldy(rd8(zHitSlot));
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x02);
    } while (cpu.y < 0x02);
    ClearObject(slot);
}

void abi_SplitInTwo101B(void)
{
    SplitInTwo101B(cpu.x);
}

/*
 * @routine 101B:783 WakeBeforeLevelX101B
 * @run 70:B783
 * WHAT IT DOES: When the player's level X is in $200-$2ED: wakes, hit-tested, leaping by the side the player is on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WakeBeforeLevelX101B(uint8_t slot)
{
    lda(RD8(OBJ->x_lo[0]));
    add_a(rd8(ScrollXLo), false);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->x_hi[0]));
    add_a(rd8(ScrollXHiScreen), (cpu.p & P_C) != 0);
    cmp(cpu.a, 0x02);
    if (cpu.a != 0x02) {
        return;
    }
    lda(RD8(ZP->temp00));
    cmp(cpu.a, 0xEE);
    if (cpu.a >= 0xEE) {
        return;
    }
    record_event(slot, 0xB79F);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(RD8(OBJ->loop_flags[slot]));
    lda((uint8_t)(cpu.a | 0x40));
    WR8(OBJ->loop_flags[slot], cpu.a);
    jsr_push(0xB7AF);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xB7C8 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xB7C9 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_WakeBeforeLevelX101B(void)
{
    WakeBeforeLevelX101B(cpu.x);
}

/*
 * @routine 072C:708 FlyPatternUnlessHeld072C
 * @run 70:BF08
 * WHAT IT DOES: Not hit-tested; while slot $10 allows, flies the pattern for OBJ_WORK_D, cleared below the first screen.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FlyPatternUnlessHeld072C(uint8_t slot)
{
    WR8(OBJ->hitbox_on[slot], 0);
    lda(RD8(OBJ->work_d[0x10]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    step_list(slot, 0xBF4E, true, 0xBF1D);
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
    move(slot, 0xBF42);
    call_routine(0xBF45, 0xBE56);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        return;
    }
    ClearObject(slot);
}

void abi_FlyPatternUnlessHeld072C(void)
{
    FlyPatternUnlessHeld072C(cpu.x);
}

/*
 * @routine 09EB:239 StartParts09EB
 * @run 70:B239
 * WHAT IT DOES: Starts five kind $2B and eight kind $2C parts, places itself below the screen, waits $E0, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartParts09EB(uint8_t slot)
{
    ldy(0x20);
    do {
        clear_slot_in_y(0xB23E);
        start_at_place(0x2B, 0xB243);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x25);
    } while (cpu.y < 0x25);
    do {
        clear_slot_in_y(0xB251);
        start_at_place(0x2C, 0xB256);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x2D);
    } while (cpu.y < 0x2D);
    restart_animation(slot);
    WR8(OBJ->x_hi[slot], 0);
    lda(0xC0);
    add_a((uint8_t)~rd8(ScrollXLo), true);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(0xE0);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_StartParts09EB(void)
{
    StartParts09EB(cpu.x);
}

/*
 * @routine 10CC:550 ListSeedFreeSlot10CC
 * @run 70:BD50
 * WHAT IT DOES: At step 1 of every fourth count starts a kind $28 object in a free slot of $13-$1F; at the list's end substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSeedFreeSlot10CC(uint8_t slot)
{
    step_list(slot, 0xBD97, true, 0xBD5B);
    if (list_ended()) {
        lda(0x01);
        WR8(OBJ->work_l[slot], cpu.a);
        lda(0xB0);
        WR8(OBJ->work_e[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x01);
    if (cpu.a != 0x01) {
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    ldy(0x13);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (cpu.a == 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x20);
        if (cpu.y >= 0x20) {
            return;
        }
    }
    start_at_place(0x28, 0xBD7D);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    lda(0x01);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
}

void abi_ListSeedFreeSlot10CC(void)
{
    ListSeedFreeSlot10CC(cpu.x);
}

/*
 * @routine 12DB:16D HoldThenShoot12DB
 * @run 70:B16D
 * WHAT IT DOES: Holds the player and the scroll's limit at X $380; throws kind $25 at step 9; at the list's end a wait of $40 and state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void HoldThenShoot12DB(uint8_t slot)
{
    lda(0x03);
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x02);
    wr8(PlayerStartPose, cpu.a);
    WR8(OBJ->work_f[0], 0);
    lda(0x03);
    wr8(ScrollXMaxHi, cpu.a);
    lda(0x80);
    wr8(ScrollXMaxLo, cpu.a);
    step_list(slot, 0xB1B4, true, 0xB18F);
    if (list_ended()) {
        restart_animation(slot);
        lda(0x40);
        WR8(OBJ->work_d[slot], cpu.a);
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x09);
    if (cpu.a != 0x09) {
        return;
    }
    ldy(0x50);
    clear_slot_in_y(0xB19F);
    start_at_place(0x25, 0xB1A4);
}

void abi_HoldThenShoot12DB(void)
{
    HoldThenShoot12DB(cpu.x);
}

/*
 * @routine 12DD:7D6 LoadShotByArg12DD
 * @run 71:C7D6
 * WHAT IT DOES: Loads at one of the places the start argument names, X mirrored while OBJ_WORK_D is set.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadShotByArg12DD(uint8_t slot)
{
    load_record(slot, 0xC7D9);
    lda(RD8(OBJ->hitbox_on[slot]));
    lda((uint8_t)(cpu.a | 0x80));
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x24);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    word_index((uint8_t)(cpu.a << 1));
    lda(rd8((uint16_t)(0xC81E + cpu.y)));
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC81D + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xC820 + cpu.y)));
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC81F + cpu.y)));
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->x_lo[slot]));
    lda((uint8_t)(cpu.a ^ 0xFF));
    lda((uint8_t)(cpu.a + 1));
    WR8(OBJ->x_lo[slot], cpu.a);
}

void abi_LoadShotByArg12DD(void)
{
    LoadShotByArg12DD(cpu.x);
}

/*
 * @routine 161B:646 SpawnWhileWaiting161B
 * @run 70:B646
 * WHAT IT DOES: Throws kind $34 pieces while OBJ_WORK_H counts, then shows itself, waits again and becomes hit-tested, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnWhileWaiting161B(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a != 0) {
        abi_nz(DEC8(OBJ->work_h[slot]));
        if ((cpu.p & P_Z) == 0) {
            return;
        }
        lda(0x02);
        WR8(OBJ->hitbox_on[slot], cpu.a);
        WR8(OBJ->sprite_attr[slot], 0);
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    if (cpu.a == 0) {
        if (find_from30(0xB655)) {
            start_at_place(0x34, 0xB65C);
            random_a(0xB65F);
            lda((uint8_t)(cpu.a & 0x07));
            WR8(OBJ->start_arg[cpu.y], cpu.a);
        }
    }
    abi_nz(DEC8(OBJ->work_h[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0x20);
    WR8(OBJ->work_h[slot], cpu.a);
    lda(0x0E);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SpawnWhileWaiting161B(void)
{
    SpawnWhileWaiting161B(cpu.x);
}

/*
 * @routine 161D:71F ListLunge161D
 * @run 71:C71F
 * WHAT IT DOES: Lunges during steps 7-10 of the list (sounds at 8); at its end state 3.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListLunge161D(uint8_t slot)
{
    step_list(slot, 0xC766, true, 0xC72A);
    if (list_ended()) {
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x07);
    if (cpu.a < 0x07) {
        return;
    }
    cmp(cpu.a, 0x0B);
    if (cpu.a >= 0x0B) {
        return;
    }
    cmp(cpu.a, 0x08);
    if (cpu.a == 0x08) {
        sound(0x87, 0xC740);
        lda(0x1A);
        jsr_push(0xC745);
        sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, cpu.x, cpu.y));
        jsr_pop();
    }
    lda(RD8(OBJ->facing_left[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC78A + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC789 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0x0C);
    WR8(ZP->product.mid, cpu.a);
    run_routine(0xAB3A);
}

void abi_ListLunge161D(void)
{
    ListLunge161D(cpu.x);
}

/*
 * @routine 10CB:72F SeedWhileLeader10CB
 * @run 70:B72F
 * WHAT IT DOES: Runs the list at $B777 throwing kind $27 at frame $4F; when slot 0 reaches state 9, substate 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SeedWhileLeader10CB(uint8_t slot)
{
    lda(RD8(OBJ->state[0]));
    cmp(cpu.a, 0x09);
    if (cpu.a == 0x09) {
        restart_animation(slot);
        lda(0x05);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    step_list(slot, 0xB777, true, 0xB74D);
    if (list_ended()) {
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x4F);
    if (cpu.a != 0x4F) {
        return;
    }
    ldy(0x30);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (cpu.a == 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x34);
        if (cpu.y >= 0x34) {
            return;
        }
    }
    clear_slot_in_y(0xB768);
    start_at_place(0x27, 0xB76D);
    sound(0x1E, 0xB772);
}

void abi_SeedWhileLeader10CB(void)
{
    SeedWhileLeader10CB(cpu.x);
}
