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
#include "calls_b11.h"

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

static void negate_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    negate_exit_shim(NegateVelocityX(slot));
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
 * @routine 18DB:183 HitInStates5To7_18DB
 * @run 70:B183
 * WHAT IT DOES: In states 5 to 7, once hit: state 8 and $500 points; a carry from the score runs $A6EC (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void HitInStates5To7_18DB(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    cmp(cpu.a, 0x08);
    if (cpu.a >= 0x08) {
        return;
    }
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    if (cpu.a == 0) {
        return;
    }
    lda(0x08);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(0x05);
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xB1A8);
    score_exit_shim(AddScoreWord());
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    call_routine(0xB1AD, 0xA6EC);
}

void abi_HitInStates5To7_18DB(void)
{
    HitInStates5To7_18DB(cpu.x);
}

/*
 * @routine 101D:3B4 FireWhenFacingPlayer101D
 * @run 71:C3B4
 * WHAT IT DOES: Cleared unless facing the player; else loads, jumps by facing and lasts two counts (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FireWhenFacingPlayer101D(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a != 0) {
        if (!counted_out(DEC8(OBJ->work_d[slot]))) {
            return;
        }
        ClearObject(slot);
        return;
    }
    jsr_push(0xC3BC);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    cmp(cpu.a, RD8(OBJ->facing_left[slot]));
    if ((cpu.p & P_Z) == 0) {
        ClearObject(slot);
        return;
    }
    load_record(slot, 0xC3C9);
    cpu.y = RD8(OBJ->facing_left[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC3E0 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0xF6);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xC3DA);
    lda(0x02);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_FireWhenFacingPlayer101D(void)
{
    FireWhenFacingPlayer101D(cpu.x);
}

/*
 * @routine 16CC:46B ListSpiralOut16CC
 * @run 70:BC6B
 * WHAT IT DOES: Moves at angle OBJ_WORK_F with a growing speed (OBJ_WORK_D-E), playing the list at $BC9A (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSpiralOut16CC(uint8_t slot)
{
    step_list(slot, 0xBC9A, true, 0xBC76);
    lda(RD8(OBJ->work_e[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    velocity_from_a(slot, 0xBC86);
    lda(RD8(OBJ->work_e[slot]));
    add_a(0x08, false);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->work_d[slot], cpu.a);
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_ListSpiralOut16CC(void)
{
    ListSpiralOut16CC(cpu.x);
}

/*
 * @routine 18DB:6AA ChecksListThenNextState18DB
 * @run 70:B6AA
 * WHAT IT DOES: Runs $B6FE and $B71B and the list at $B6E9; when OBJ_WORK_E counts out, the next state of a cycle (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChecksListThenNextState18DB(uint8_t slot)
{
    call_routine(0xB6AD, 0xB6FE);
    call_routine(0xB6B0, 0xB71B);
    step_list(slot, 0xB6E9, true, 0xB6BB);
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    restart_animation(slot);
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a & 0x0F));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB6D9 + cpu.y)));
    WR8(OBJ->state[slot], cpu.a);
    INC8(OBJ->work_f[slot]);
    WR8(OBJ->substate[slot], 0);
}

void abi_ChecksListThenNextState18DB(void)
{
    ChecksListThenNextState18DB(cpu.x);
}

/*
 * @routine 088C:4DB AndVramWithMask088C
 * @run 70:BCDB
 * WHAT IT DOES: ANDs the VRAM word at zBiosCl-Ch with StageMaskBuffer's byte X on both halves (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: A the new high byte, N and Z from it; X and Y kept.
 */
void AndVramWithMask088C(void)
{
    jsr_push(0xBCDE);
    SelectVramWriteAddress();
    cpu.a = 0x00;
    jsr_pop();
    lda(RD8(ZP->bios_c.lo));
    WR8(VDC->data_lo, cpu.a);
    lda(RD8(ZP->bios_c.hi));
    WR8(VDC->data_hi, cpu.a);
    jsr_push(0xBCEB);
    SelectVramReadAddress();
    lda(0x01);
    jsr_pop();
    lda(RD8(ZP->bios_c.lo));
    WR8(VDC->data_lo, cpu.a);
    lda(RD8(ZP->bios_c.hi));
    WR8(VDC->data_hi, cpu.a);
    jsr_push(0xBCF8);
    SelectVramData();
    lda(0x02);
    jsr_pop();
    lda(RD8(VDC->data_lo));
    lda((uint8_t)(cpu.a & rd8((uint16_t)(StageMaskBuffer + cpu.x))));
    WR8(VDC->data_lo, cpu.a);
    lda(RD8(VDC->data_hi));
    lda((uint8_t)(cpu.a & rd8((uint16_t)(StageMaskBuffer + cpu.x))));
    WR8(VDC->data_hi, cpu.a);
}

void abi_AndVramWithMask088C(void)
{
    AndVramWithMask088C();
}

/*
 * @routine 0B4C:243 MoveStopAtWall0B4C
 * @run 70:BA43
 * WHAT IT DOES: Moves along X; at a wall OBJ_WORK_E 96, the animation restarted, substate on (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void MoveStopAtWall0B4C(uint8_t slot)
{
    move_x(slot, 0xBA46);
    lda(0x06);
    WR8(ZP->product.mid, cpu.a);
    lda(0x22);
    jsr_push(0xBA4F);
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
    lda(0x60);
    WR8(OBJ->work_e[slot], cpu.a);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_MoveStopAtWall0B4C(void)
{
    MoveStopAtWall0B4C(cpu.x);
}

/*
 * @routine 0CAC:109 StartAtCountWithSound0CAC
 * @run 70:B909
 * WHAT IT DOES: At the table's counts of OBJ_WORK_E: a sound and a kind $2C object in slot $18 plus its place (secret stage 07's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartAtCountWithSound0CAC(uint8_t slot)
{
    cpu.y = 0;
    for (;;) {
        lda(RD8(OBJ->work_e[slot]));
        cmp(cpu.a, rd8((uint16_t)(0xB939 + cpu.y)));
        if ((cpu.p & P_Z) != 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x08);
        if (cpu.y >= 0x08) {
            return;
        }
    }
    push8(cpu.y);
    lda(cpu.y);
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB941 + cpu.y)));
    jsr_push(0xB923);
    {
        CommandCall call = SoundCommand(cpu.a, cpu.x, cpu.y);

        sound_command_exit_shim(cpu.a, call);
    }
    jsr_pop();
    cpu.y = pull8();
    abi_nz(cpu.y);
    WR8(ZP->spread_count, cpu.y);
    lda(cpu.y);
    add_a(0x18, false);
    cpu.y = cpu.a;
    clear_slot_in_y(0xB92E);
    start_at_place(0x2C, 0xB933);
    lda(RD8(ZP->spread_count));
    WR8(OBJ->work_d[cpu.y], cpu.a);
}

void abi_StartAtCountWithSound0CAC(void)
{
    StartAtCountWithSound0CAC(cpu.x);
}

/*
 * @routine 0EBF:610 LoadRiser0EBF
 * @run 71:D610
 * WHAT IT DOES: Loads at a random place rising slowly, facing the player, with random work bytes (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadRiser0EBF(uint8_t slot)
{
    load_record(slot, 0xD613);
    lda(0x8F);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    call_routine(0xD61B, 0xD648);
    random_a(0xD61E);
    WR8(OBJ->work_j[slot], cpu.a);
    random_a(0xD624);
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xD640 + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    face_by_side(slot, COL(OBJ->facing_left), 0xD630);
    lda(0xFF);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->vy_lo[slot], cpu.a);
}

void abi_LoadRiser0EBF(void)
{
    LoadRiser0EBF(cpu.x);
}

/*
 * @routine 101B:0CC WalkAndTurnEvery101B
 * @run 70:B0CC
 * WHAT IT DOES: Walks a step on each new frame, turning after as many steps as the start argument (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkAndTurnEvery101B(uint8_t slot)
{
    step_list(slot, 0xB0FE, true, 0xB0D7);
    if ((cpu.p & P_C) != 0) {
        cpu.y = RD8(OBJ->work_e[slot]);
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xB0FC + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        move_x(slot, 0xB0E5);
        if (counted_out(DEC8(OBJ->work_l[slot]))) {
            lda(RD8(OBJ->start_arg[slot]));
            WR8(OBJ->work_l[slot], cpu.a);
            lda(RD8(OBJ->work_e[slot]));
            lda((uint8_t)(cpu.a ^ 0x01));
            WR8(OBJ->work_e[slot], cpu.a);
        }
    }
    call_routine(0xB0FB, 0xB107);
}

void abi_WalkAndTurnEvery101B(void)
{
    WalkAndTurnEvery101B(cpu.x);
}

/*
 * @routine 10CC:3CA SoundScriptAndPose10CC
 * @run 70:BBCA
 * WHAT IT DOES: Sound $E8, script $B0, then frame $98 shot upward, substate on (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundScriptAndPose10CC(uint8_t slot)
{
    sound(0xE8, 0xBBCF);
    push8(cpu.x);
    script_at(0xB0, 0x08, 0xBBD7);
    pull_x();
    restart_animation(slot);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0xF8);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x20);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x98);
    WR8(OBJ->frame[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundScriptAndPose10CC(void)
{
    SoundScriptAndPose10CC(cpu.x);
}

/*
 * @routine 117E:08E ListFlyThenFall117E
 * @run 71:C88E
 * WHAT IT DOES: Flies playing the list at $C8BE; type 2 falls, others count OBJ_WORK_F to the next state (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListFlyThenFall117E(uint8_t slot)
{
    step_list(slot, 0xC8BE, false, 0xC899);
    move(slot, 0xC89C);
    lda(RD8(OBJ->work_e[slot]));
    cmp(cpu.a, 0x02);
    if (cpu.a == 0x02) {
        lda(RD8(OBJ->vy_lo[slot]));
        add_a(0x20, false);
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(RD8(OBJ->vy_hi[slot]));
        add_a(0x00, (cpu.p & P_C) != 0);
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    if (!counted_out(DEC8(OBJ->work_f[slot]))) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListFlyThenFall117E(void)
{
    ListFlyThenFall117E(cpu.x);
}

/*
 * @routine 12DB:6A1 DripWhenLow12DB
 * @run 70:B6A1
 * WHAT IT DOES: Low on the screen, every fourth count: sound $19 and a kind $2B drop at Y $BE (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DripWhenLow12DB(uint8_t slot)
{
    uint8_t found_a, to;
    bool found;

    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xA4);
    if (cpu.a < 0xA4) {
        return;
    }
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    jsr_push(0xB6B7);
    to = FindFreeSlotFrom30(&found_a, &found);
    find_slot_exit_shim(to, found_a, found);
    jsr_pop();
    if (!found) {
        return;
    }
    sound(0x19, 0xB6BE);
    start_at_place(0x2B, 0xB6C3);
    lda(0xBE);
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    random_a(0xB6CB);
    lda((uint8_t)(cpu.a & 0x0F));
    WR8(OBJ->work_d[cpu.y], cpu.a);
}

void abi_DripWhenLow12DB(void)
{
    DripWhenLow12DB(cpu.x);
}

/*
 * @routine 12DD:415 WaitForSlot18ThenList12DD
 * @run 71:C415
 * WHAT IT DOES: Waits for slot $12 to drop past Y $50 (sound $7B), then the list at $C445, hit-tested, state on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WaitForSlot18ThenList12DD(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->y_hi[0x12]));
        if (cpu.a != 0) {
            return;
        }
        lda(RD8(OBJ->y_lo[0x12]));
        cmp(cpu.a, 0x50);
        if (cpu.a < 0x50) {
            return;
        }
        sound(0x7B, 0xC42B);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    step_list(slot, 0xC445, true, 0xC43A);
    if (!list_ended()) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_WaitForSlot18ThenList12DD(void)
{
    WaitForSlot18ThenList12DD(cpu.x);
}

/*
 * @routine 12DE:173 HomeOnAngle12DE
 * @run 71:C973
 * WHAT IT DOES: Moves at angle OBJ_WORK_D turning toward OBJ_WORK_H; there, the state on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void HomeOnAngle12DE(uint8_t slot)
{
    TurnAngleResult turned;

    lda(RD8(OBJ->work_g[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    velocity_from_a(slot, 0xC983);
    move(slot, 0xC986);
    lda(RD8(OBJ->work_h[slot]));
    WR8(ZP->temp02, cpu.a);
    lda(0x02);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xC997);
    turned = TurnAngleToward();
    turn_angle_exit_shim(turned);
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    cmp(cpu.a, RD8(OBJ->work_h[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_HomeOnAngle12DE(void)
{
    HomeOnAngle12DE(cpu.x);
}

/*
 * @routine 161D:43A LoadAimThenList161D
 * @run 71:C43A
 * WHAT IT DOES: In state 0 appears 32 dots lower aimed at the player; later flies playing the list at $C46A (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAimThenList161D(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC442);
        lda(0x20);
        WR8(OBJ->vy_hi[slot], cpu.a);
        move_y(slot, 0xC44A);
        aim_at_player(0xC44D);
        push8(cpu.a);
        lda(0x80);
        WR8(ZP->temp00, cpu.a);
        lda(0x03);
        WR8(ZP->temp01, cpu.a);
        lda(pull8());
        velocity_from_a(slot, 0xC45A);
        return;
    }
    step_list(slot, 0xC46A, true, 0xC466);
    move(slot, 0xC469);
}

void abi_LoadAimThenList161D(void)
{
    LoadAimThenList161D(cpu.x);
}

/*
 * @routine 18DC:139 ListThenLaunch27_18DC
 * @run 70:B939
 * WHAT IT DOES: Plays the list at $B969; then sound $4C and a kind $27 object in slot $33, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenLaunch27_18DC(uint8_t slot)
{
    step_list(slot, 0xB969, true, 0xB944);
    if (!list_ended()) {
        return;
    }
    sound(0x4C, 0xB94B);
    ldy(0x33);
    clear_slot_in_y(0xB950);
    start_at_place(0x27, 0xB955);
    lda(0x01);
    WR8(OBJ->work_j[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenLaunch27_18DC(void)
{
    ListThenLaunch27_18DC(cpu.x);
}

/*
 * @routine 0F6C:1A8 ThrowOnTurn0F6C
 * @run 70:B9A8
 * WHAT IT DOES: On this object's turn of four, a kind $9B object eight dots down and right (secret stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ThrowOnTurn0F6C(uint8_t slot)
{
    lda(RD8(ZP->wave_counter));
    lda((uint8_t)(cpu.a & 0x03));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    cmp(cpu.a, RD8(ZP->temp00));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    if (!find_from20(0xB9BA)) {
        return;
    }
    start_at_place(0x9B, 0xB9C1);
    swap_xy();
    lda(0x08);
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    move_x(cpu.x, 0xB9CA);
    lda(0x08);
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    move_y(cpu.x, 0xB9D2);
    lda(0x10);
    WR8(OBJ->sprite_attr[cpu.x], cpu.a);
    swap_xy();
}

void abi_ThrowOnTurn0F6C(void)
{
    ThrowOnTurn0F6C(cpu.x);
}

/*
 * @routine 101E:42B FollowPreviousWithSlot16_101E
 * @run 71:CC2B
 * WHAT IT DOES: Keeps the previous slot's place; piece 0 offsets it by slot $10's frame and facing (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FollowPreviousWithSlot16_101E(uint8_t slot)
{
    lda(RD8(OBJ->hitbox_on[0x10]));
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    jsr_push(0xCC37);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->frame[0x10]));
    {
        Sum sum = sub_bytes(cpu.a, 0x01, true);

        lda(sum.value);
        abi_flag(P_C, sum.carry);
        abi_flag(P_V, sum.overflow);
    }
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xCC5C + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xCC5D + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->facing_left[0x10]));
    if (cpu.a != 0) {
        negate_x(slot, 0xCC58);
    }
    move(slot, 0xCC5B);
}

void abi_FollowPreviousWithSlot16_101E(void)
{
    FollowPreviousWithSlot16_101E(cpu.x);
}

/*
 * @routine 117F:72F RaiseSlot67_117F
 * @run 71:D72F
 * WHAT IT DOES: Once the slot three on is empty, raises slot $43 two dots a call; at Y $9F cleared (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RaiseSlot67_117F(uint8_t slot)
{
    Sum sum;

    lda(RD8_IDX((uint16_t)(COL(OBJ->kind) + 3), slot));
    if (cpu.a != 0) {
        return;
    }
    lda(0x80);
    WR8(OBJ->sprite_flip[0x43], cpu.a);
    WR8(OBJ->hit_taken[0x43], 0);
    lda(RD8(OBJ->y_lo[0x43]));
    sum = sub_bytes(cpu.a, 0x02, true);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->y_lo[0x43], cpu.a);
    lda(RD8(OBJ->y_hi[0x43]));
    sum = sub_bytes(cpu.a, 0x00, (cpu.p & P_C) != 0);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    WR8(OBJ->y_hi[0x43], cpu.a);
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[0x43]));
    cmp(cpu.a, 0x9F);
    if (cpu.a >= 0x9F) {
        return;
    }
    lda(0x9F);
    WR8(OBJ->y_lo[0x43], cpu.a);
    ClearObject(slot);
}

void abi_RaiseSlot67_117F(void)
{
    RaiseSlot67_117F(cpu.x);
}

/*
 * @routine 10CB:17B PieceChannelByFrame10CB
 * @run 70:B17B
 * WHAT IT DOES: Counts OBJ_WORK_D, runs $B256 and $B26D; a kind $A5 piece shows the list channel its frame names (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PieceChannelByFrame10CB(uint8_t slot)
{
    INC8(OBJ->work_d[slot]);
    call_routine(0xB181, 0xB256);
    call_routine(0xB184, 0xB26D);
    push8(cpu.x);
    lda(RD8(OBJ->kind[slot]));
    cmp(cpu.a, 0xA5);
    if (cpu.a == 0xA5) {
        lda(RD8(OBJ->work_l[slot]));
        if (cpu.a == 0) {
            lda(RD8(OBJ->frame[slot]));
            if (cpu.a != 0) {
                Sum sum = sub_bytes(cpu.a, 0x01, true);

                lda(sum.value);
                abi_flag(P_C, sum.carry);
                abi_flag(P_V, sum.overflow);
                cpu.y = cpu.a;
                lda(rd8((uint16_t)(0xB1A9 + cpu.y)));
                if (cpu.a != 0) {
                    add_a(0x81, false);
                    list_channel2(cpu.a, 0x05, 0xB1A7);
                }
            }
        }
    }
    pull_x();
}

void abi_PieceChannelByFrame10CB(void)
{
    PieceChannelByFrame10CB(cpu.x);
}
