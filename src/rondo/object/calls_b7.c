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
#include "calls_b7.h"

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

/*
 * @routine 117A:6FC RasterByLevelX117A
 * @run 70:AEFC
 * WHAT IT DOES: In state 0 loads the record; later raster effect $1D below level X $3A0, none above (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RasterByLevelX117A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    lda(rd8(ScrollXHiScreen));
    cmp(cpu.a, 0x03);
    if (cpu.a >= 0x03) {
        cmp(cpu.a, 0x04);
        if (cpu.a >= 0x04) {
            lda(0x00);
            WR8(ZP->raster_effect, cpu.a);
            return;
        }
        lda(rd8(ScrollXLo));
        cmp(cpu.a, 0xA0);
        if (cpu.a >= 0xA0) {
            lda(0x00);
            WR8(ZP->raster_effect, cpu.a);
            return;
        }
    }
    lda(0x1D);
    WR8(ZP->raster_effect, cpu.a);
}

void abi_RasterByLevelX117A(void)
{
    RasterByLevelX117A(cpu.x);
}

/*
 * @routine 117D:563 ListSpeedProbe117D
 * @run 71:C563
 * WHAT IT DOES: Plays the list at $C58B, the X speed by OBJ_WORK_F, and probes with height 6 (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSpeedProbe117D(uint8_t slot)
{
    step_list(slot, 0xC58B, false, 0xC56E);
    lda(RD8(OBJ->work_f[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC588 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC587 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0x06);
    WR8(ZP->product.mid, cpu.a);
    call_routine(0xC586, 0xAB3A);
}

void abi_ListSpeedProbe117D(void)
{
    ListSpeedProbe117D(cpu.x);
}

/*
 * @routine 161D:66F ListSpeedProbe161D
 * @run 71:C66F
 * WHAT IT DOES: Plays the list at $C695, the X speed by OBJ_WORK_E, and probes with height 12 (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSpeedProbe161D(uint8_t slot)
{
    step_list(slot, 0xC695, true, 0xC67A);
    lda(RD8(OBJ->work_e[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xC6A3 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC6A2 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0x0C);
    WR8(ZP->product.mid, cpu.a);
    call_routine(0xC692, 0xAB3A);
}

void abi_ListSpeedProbe161D(void)
{
    ListSpeedProbe161D(cpu.x);
}

/*
 * @routine 117F:156 WaitOnLeader117F
 * @run 71:D156
 * WHAT IT DOES: Keeps the place of the slot OBJ_WORK_L names until it reaches state 3 and OBJ_WORK_D counts out (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WaitOnLeader117F(uint8_t slot)
{
    cpu.y = RD8(OBJ->work_l[slot]);
    abi_nz(cpu.y);
    jsr_push(0xD15C);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8(OBJ->state[cpu.y]));
    cmp(cpu.a, 0x03);
    if (cpu.a < 0x03) {
        return;
    }
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    cpu.y = RD8(OBJ->work_m[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xD17A + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_WaitOnLeader117F(void)
{
    WaitOnLeader117F(cpu.x);
}

/*
 * @routine 12DE:71E StartEightAtBE_12DE
 * @run 71:CF1E
 * WHAT IT DOES: Sound $19 and eight kind $2B objects numbered at Y $BE (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: as the last call, compare or load left them.
 */
void StartEightAtBE_12DE(void)
{
    sound(0x19, 0xCF23);
    WR8(ZP->temp00, 0);
    do {
        if (!find_from30(0xCF28)) {
            return;
        }
        start_at_place(0x2B, 0xCF2F);
        lda(0xBE);
        WR8(OBJ->y_lo[cpu.y], cpu.a);
        lda(RD8(ZP->temp00));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        INC8(ZP->temp00);
        lda(RD8(ZP->temp00));
        cmp(cpu.a, 0x08);
    } while (cpu.a < 0x08);
}

void abi_StartEightAtBE_12DE(void)
{
    StartEightAtBE_12DE();
}

/*
 * @routine 13FB:6CC FallProbeToA8_13FB
 * @run 70:B6CC
 * WHAT IT DOES: Moves and falls with a terrain probe; at Y $A8 it stops there with C set (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallProbeToA8_13FB(uint8_t slot)
{
    move(slot, 0xB6CF);
    gravity(slot, 0xB6D2);
    lda(0x04);
    WR8(ZP->product.mid, cpu.a);
    lda(0x22);
    jsr_push(0xB6DB);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xA8);
    if (cpu.a < 0xA8) {
        return;
    }
    lda(0xA8);
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    abi_flag(P_C, true);
}

void abi_FallProbeToA8_13FB(void)
{
    FallProbeToA8_13FB(cpu.x);
}

/*
 * @routine 13FD:420 SwingFallToBC_13FD
 * @run 71:C420
 * WHAT IT DOES: Swings and falls; from Y $BC zRasterStage 0 and the object cleared (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwingFallToBC_13FD(uint8_t slot)
{
    call_routine(0xC423, 0xC3F2);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x0C, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xC437);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xBC);
    if (cpu.a < 0xBC) {
        return;
    }
    wr8(zRasterStage, 0);
    jsr_push(0xC443);
    ClearObject(slot);
    jsr_pop();
}

void abi_SwingFallToBC_13FD(void)
{
    SwingFallToBC_13FD(cpu.x);
}

/*
 * @routine 16CC:385 LoadTossFromSlot17_16CC
 * @run 70:BB85
 * WHAT IT DOES: Loads the record at slot $11's place with a random X speed and a falling Y speed (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadTossFromSlot17_16CC(uint8_t slot)
{
    load_record(slot, 0xBB88);
    ldy(0x11);
    jsr_push(0xBB8D);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    random_a(0xBB90);
    lda((uint8_t)(cpu.a & 0x03));
    add_a(0xFD, false);
    WR8(OBJ->vx_hi[slot], cpu.a);
    random_a(0xBB9B);
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0xC0);
    WR8(OBJ->vy_lo[slot], cpu.a);
}

void abi_LoadTossFromSlot17_16CC(void)
{
    LoadTossFromSlot17_16CC(cpu.x);
}

/*
 * @routine 0B4D:13F CountThenStart21_0B4D
 * @run 71:C13F
 * WHAT IT DOES: Counts OBJ_WORK_D: every other count steps a phase and runs $C164; at zero a kind $21 object in slot $11 and cleared (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenStart21_0B4D(uint8_t slot)
{
    if (counted_out(DEC8(OBJ->work_d[slot]))) {
        lda(0x21);
        ldy(0x11);
        jsr_push(0xC14B);
        copy_place_exit_shim(StartObjectAtPlace(cpu.a, slot, cpu.y));
        jsr_pop();
        ClearObject(slot);
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    add_a(0x13, false);
    lda((uint8_t)(cpu.a & 0x3F));
    WR8(OBJ->work_e[slot], cpu.a);
    call_routine(0xC163, 0xC164);
}

void abi_CountThenStart21_0B4D(void)
{
    CountThenStart21_0B4D(cpu.x);
}

/*
 * @routine 0E0B:735 ListAtFrame40Spawn0E0B
 * @run 70:B735
 * WHAT IT DOES: Plays the list at $B75A; at its frame $40 a kind $2A object in slot $50; when it ends, substate on (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListAtFrame40Spawn0E0B(uint8_t slot)
{
    step_list(slot, 0xB75A, true, 0xB740);
    if (list_ended()) {
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x40);
    if (cpu.a != 0x40) {
        return;
    }
    ldy(0x50);
    clear_slot_in_y(0xB750);
    start_at_place(0x2A, 0xB755);
}

void abi_ListAtFrame40Spawn0E0B(void)
{
    ListAtFrame40Spawn0E0B(cpu.x);
}

/*
 * @routine 10CC:42B ListLandScript10CC
 * @run 70:BC2B
 * WHAT IT DOES: Plays the list at $BC50 falling; on landing script $AF and substate 1 (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListLandScript10CC(uint8_t slot)
{
    step_list(slot, 0xBC50, true, 0xBC36);
    call_routine(0xBC39, 0xBA02);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    push8(cpu.x);
    script_at(0xAF, 0x08, 0xBC43);
    pull_x();
    restart_animation(slot);
    lda(0x01);
    WR8(OBJ->substate[slot], cpu.a);
}

void abi_ListLandScript10CC(void)
{
    ListLandScript10CC(cpu.x);
}

/*
 * @routine 12DB:581 ListCountSoundSpawn12DB
 * @run 70:B581
 * WHAT IT DOES: Plays the list at $B5A6 while OBJ_WORK_E counts; then sound $25, a kind $29 object, state 2 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListCountSoundSpawn12DB(uint8_t slot)
{
    step_list(slot, 0xB5A6, true, 0xB58C);
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    sound(0x25, 0xB596);
    if (!find_from50(0xB599)) {
        return;
    }
    start_at_place(0x29, 0xB5A0);
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ListCountSoundSpawn12DB(void)
{
    ListCountSoundSpawn12DB(cpu.x);
}

/*
 * @routine 12DD:57C ListSoundAt48_12DD
 * @run 71:C57C
 * WHAT IT DOES: Plays the list at $C5A1 with sound $79 at frame $48; when it ends, state 3 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSoundAt48_12DD(uint8_t slot)
{
    step_list(slot, 0xC5A1, true, 0xC587);
    if (list_ended()) {
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x48);
    if (cpu.a != 0x48) {
        return;
    }
    sound(0x79, 0xC597);
}

void abi_ListSoundAt48_12DD(void)
{
    ListSoundAt48_12DD(cpu.x);
}

/*
 * @routine 12DE:5A6 ListThenShoot13_12DE
 * @run 71:CDA6
 * WHAT IT DOES: Plays the list at $CDCB; then sound $21, a kind $3A object in slot $13, state 3 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenShoot13_12DE(uint8_t slot)
{
    step_list(slot, 0xCDCB, true, 0xCDB1);
    if (!list_ended()) {
        return;
    }
    sound(0x21, 0xCDB8);
    ldy(0x13);
    clear_slot_in_y(0xCDBD);
    start_at_place(0x3A, 0xCDC2);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenShoot13_12DE(void)
{
    ListThenShoot13_12DE(cpu.x);
}

/*
 * @routine 16CB:64A CountScriptSpawn16CB
 * @run 70:B64A
 * WHAT IT DOES: Counts OBJ_WORK_D; then script $96 and a kind $2B object; with no free slot the count stays out (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountScriptSpawn16CB(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    push8(cpu.x);
    script_at(0x96, 0x05, 0xB657);
    pull_x();
    if (!find_from30(0xB65B)) {
        WR8(OBJ->work_d[slot], 0);
        return;
    }
    start_at_place(0x2B, 0xB662);
    lda(0xC0);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CountScriptSpawn16CB(void)
{
    CountScriptSpawn16CB(cpu.x);
}

/*
 * @routine 18DB:22A ListThenSpawn26_18DB
 * @run 70:B22A
 * WHAT IT DOES: Plays the list at $B24F; then a kind $26 object in slot $18 at Y $5E with sound $91, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenSpawn26_18DB(uint8_t slot)
{
    step_list(slot, 0xB24F, true, 0xB235);
    if (!list_ended()) {
        return;
    }
    ldy(0x18);
    clear_slot_in_y(0xB23C);
    start_at_place(0x26, 0xB241);
    sound(0x91, 0xB246);
    lda(0x5E);
    WR8(OBJ->y_lo[cpu.y], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenSpawn26_18DB(void)
{
    ListThenSpawn26_18DB(cpu.x);
}

/*
 * @routine 0B4C:4FC StartPairFacing0B4C
 * @run 70:BCFC
 * WHAT IT DOES: Faces the player and starts a kind $2D object in slot $51 and a $2E in slot $11, substate on (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartPairFacing0B4C(uint8_t slot)
{
    face_by_side(slot, COL(OBJ->facing_left), 0xBCFF);
    ldy(0x51);
    clear_slot_in_y(0xBD09);
    start_at_place(0x2D, 0xBD0E);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    ldy(0x11);
    clear_slot_in_y(0xBD19);
    start_at_place(0x2E, 0xBD1E);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartPairFacing0B4C(void)
{
    StartPairFacing0B4C(cpu.x);
}

/*
 * @routine 0E0B:4E9 ListLandSounds0E0B
 * @run 70:B4E9
 * WHAT IT DOES: Plays the list at $B50F while falling; on landing sounds $CE and $4A, substate on (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListLandSounds0E0B(uint8_t slot)
{
    step_list(slot, 0xB50F, true, 0xB4F4);
    lda(0x04);
    jsr_push(0xB4F9);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    sound(0xCE, 0xB500);
    sound(0x4A, 0xB505);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListLandSounds0E0B(void)
{
    ListLandSounds0E0B(cpu.x);
}

/*
 * @routine 0EBF:1E4 StateByPlace0EBF
 * @run 71:D1E4
 * WHAT IT DOES: State 3 off the screen along X, 4 along Y, else by the player's distance from a table (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StateByPlace0EBF(uint8_t slot)
{
    WR8(OBJ->substate[slot], 0);
    lda(RD8(OBJ->x_hi[slot]));
    if (cpu.a != 0) {
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        lda(0x04);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    jsr_push(0xD200);
    distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x0F));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xD20A + cpu.y)));
    WR8(OBJ->state[slot], cpu.a);
}

void abi_StateByPlace0EBF(void)
{
    StateByPlace0EBF(cpu.x);
}

/*
 * @routine 101C:46F ListSpawnOrEnd101C
 * @run 70:BC6F
 * WHAT IT DOES: Plays the list at $BC95; on new frames the spawn at $BC4D for step 8; when it ends, state 1 (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSpawnOrEnd101C(uint8_t slot)
{
    step_list(slot, 0xBC95, true, 0xBC7A);
    if (list_ended()) {
        lda(0x74);
        WR8(OBJ->frame[slot], cpu.a);
        lda(0x01);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(0x08);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    run_routine(0xBC4D);
}

void abi_ListSpawnOrEnd101C(void)
{
    ListSpawnOrEnd101C(cpu.x);
}

/*
 * @routine 101E:553 StartSix9A_101E
 * @run 71:CD53
 * WHAT IT DOES: Starts up to six kind $9A objects numbered in OBJ_WORK_E, Y kept (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartSix9A_101E(uint8_t slot)
{
    push8(cpu.y);
    cpu.y = 0;
    do {
        WR8(ZP->spread_count, cpu.y);
        if (!find_from30(0xCD5A)) {
            break;
        }
        lda(0x01);
        WR8(OBJ->work_d[cpu.y], cpu.a);
        start_at_place(0x9A, 0xCD66);
        lda(0x80);
        WR8(OBJ->sprite_flip[cpu.y], cpu.a);
        lda(RD8(ZP->spread_count));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        cpu.y = RD8(ZP->spread_count);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x06);
    } while (cpu.y < 0x06);
    cpu.y = pull8();
    abi_nz(cpu.y);
}

void abi_StartSix9A_101E(void)
{
    StartSix9A_101E(cpu.x);
}

/*
 * @routine 161E:773 LoadDriftOrFall161E
 * @run 71:CF73
 * WHAT IT DOES: In state 0 loads and tosses; later falls, moves and runs on at $D03A (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadDriftOrFall161E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xCF7B);
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x36);
        WR8(OBJ->frame[slot], cpu.a);
        lda(0xFE);
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(0xFE);
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    gravity(slot, 0xCF93);
    move(slot, 0xCF96);
    run_routine(0xD03A);
}

void abi_LoadDriftOrFall161E(void)
{
    LoadDriftOrFall161E(cpu.x);
}

/*
 * @routine 18DB:43D StartByAnimationStep18DB
 * @run 70:B43D
 * WHAT IT DOES: At animation steps 7 and 8, a kind $29 object in slot $30 or $31 with the facing and sound $4D (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartByAnimationStep18DB(uint8_t slot)
{
    Sum sum;

    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x07);
    if (cpu.a != 0x07) {
        cmp(cpu.a, 0x08);
        if (cpu.a != 0x08) {
            return;
        }
    }
    sum = sub_bytes(cpu.a, 0x07, true);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    add_a(0x30, false);
    cpu.y = cpu.a;
    clear_slot_in_y(0xB452);
    start_at_place(0x29, 0xB457);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    sound(0x4D, 0xB462);
}

void abi_StartByAnimationStep18DB(void)
{
    StartByAnimationStep18DB(cpu.x);
}

/*
 * @routine 18DC:1DE ListThenTwoScripts18DC
 * @run 70:B9DE
 * WHAT IT DOES: Plays the list at $BA04; then scripts $85 and $87, OBJ_WORK_E 64, substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenTwoScripts18DC(uint8_t slot)
{
    step_list(slot, 0xBA04, true, 0xB9E9);
    if (!list_ended()) {
        return;
    }
    push8(cpu.x);
    script_at(0x85, 0x04, 0xB9F3);
    script_at(0x87, 0x05, 0xB9FA);
    pull_x();
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenTwoScripts18DC(void)
{
    ListThenTwoScripts18DC(cpu.x);
}

/*
 * @routine 088B:5D5 LandAtB5_088B
 * @run 70:B5D5
 * WHAT IT DOES: Falls until landing or Y $B5, then sound $9C and substate on (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandAtB5_088B(uint8_t slot)
{
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xB5);
    if (cpu.a < 0xB5) {
        lda(0x0A);
        WR8(OBJ->frame[slot], cpu.a);
        lda(0x04);
        jsr_push(0xB5E6);
        mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
        jsr_pop();
        if ((cpu.p & P_C) == 0) {
            return;
        }
    }
    sound(0x9C, 0xB5ED);
    restart_animation(slot);
    lda(0xB5);
    WR8(OBJ->y_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LandAtB5_088B(void)
{
    LandAtB5_088B(cpu.x);
}

/*
 * @routine 09EC:3D9 LoadFaceOrListClear09EC
 * @run 70:BBD9
 * WHAT IT DOES: In state 0 loads with a random facing; later follows slot $10 playing the list at $BC00, then cleared (secret stage 05's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFaceOrListClear09EC(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xBBE1);
        random_a(0xBBE4);
        lda((uint8_t)(cpu.a & 0x01));
        WR8(OBJ->facing_left[slot], cpu.a);
        return;
    }
    ldy(0x10);
    jsr_push(0xBBEF);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    step_list(slot, 0xBC00, true, 0xBBFA);
    if (!list_ended()) {
        return;
    }
    ClearObject(slot);
}

void abi_LoadFaceOrListClear09EC(void)
{
    LoadFaceOrListClear09EC(cpu.x);
}

/*
 * @routine 0CAB:768 ListByFacingThenState3_0CAB
 * @run 70:B768
 * WHAT IT DOES: Plays the facing's frame list; then state 3 (secret stage 07's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListByFacingThenState3_0CAB(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    lda(RD8(OBJ->facing_left[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB78F + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xB790 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    jsr_push(0xB77A);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
    if (!list_ended()) {
        return;
    }
    lda(0x02);
    WR8(OBJ->work_a[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListByFacingThenState3_0CAB(void)
{
    ListByFacingThenState3_0CAB(cpu.x);
}

/*
 * @routine 0E0B:7C6 DissolveThenBecome21_0E0B
 * @run 70:B7C6
 * WHAT IT DOES: Dissolves patterns 0 to $11 while OBJ_WORK_D counts; then the slot becomes kind $21 (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DissolveThenBecome21_0E0B(uint8_t slot)
{
    lda(0x00);
    wr8(WorkByteA, cpu.a);
    lda(0x12);
    wr8(WorkByteB, cpu.a);
    call_routine(0xB7D3, 0xB7ED);
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    stop_here(slot);
    lda(0x21);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_DissolveThenBecome21_0E0B(void)
{
    DissolveThenBecome21_0E0B(cpu.x);
}

/*
 * @routine 101C:504 StartRowThenClear101C
 * @run 70:BD04
 * WHAT IT DOES: Starts eight kind $25 objects numbered in their start arguments, then clears itself (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartRowThenClear101C(uint8_t slot)
{
    WR8(ZP->temp00, 0);
    do {
        if (!find_from30(0xBD09)) {
            break;
        }
        start_at_place(0x25, 0xBD10);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        lda(0x8C);
        WR8(OBJ->work_e[cpu.y], cpu.a);
        INC8(ZP->temp00);
        lda(RD8(ZP->temp00));
        cmp(cpu.a, 0x08);
    } while (cpu.a < 0x08);
    ClearObject(slot);
}

void abi_StartRowThenClear101C(void)
{
    StartRowThenClear101C(cpu.x);
}

/*
 * @routine 101C:5F9 FallAndScatter101C
 * @run 70:BDF9
 * WHAT IT DOES: Falls and every fourth frame throws a kind $9B object to either side (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallAndScatter101C(uint8_t slot)
{
    uint8_t swap;

    gravity(slot, 0xBDFC);
    move(slot, 0xBDFF);
    lda(RD8(ZP->wave_counter));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    if (!find_from30(0xBE08)) {
        return;
    }
    start_at_place(0x9B, 0xBE0F);
    random_a(0xBE12);
    lda((uint8_t)(cpu.a & 0x0F));
    {
        Sum sum = sub_bytes(cpu.a, 0x08, true);

        lda(sum.value);
        abi_flag(P_C, sum.carry);
        abi_flag(P_V, sum.overflow);
    }
    WR8(OBJ->vx_hi[cpu.y], cpu.a);
    swap = cpu.x;
    cpu.x = cpu.y;
    cpu.y = swap;
    jsr_push(0xBE1E);
    axis_move_exit_shim(ApplyVelocityX(cpu.x));
    jsr_pop();
    swap = cpu.x;
    cpu.x = cpu.y;
    cpu.y = swap;
}

void abi_FallAndScatter101C(void)
{
    FallAndScatter101C(cpu.x);
}

/*
 * @routine 101E:3AF LoadFrameByWorkD101E
 * @run 71:CBAF
 * WHAT IT DOES: Loads the record with slot $10's flip; the frame by OBJ_WORK_D, hit-tested only for piece $E (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFrameByWorkD101E(uint8_t slot)
{
    load_record(slot, 0xCBB2);
    lda(RD8(OBJ->sprite_flip[0x10]));
    WR8(OBJ->sprite_flip[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, 0x0E);
    if (cpu.a != 0x0E) {
        WR8(OBJ->hitbox_on[slot], 0);
        lda(0x10);
        WR8(OBJ->sprite_attr[slot], cpu.a);
    }
    cpu.y = RD8(OBJ->work_d[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xCBD6 + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x05);
    WR8(OBJ->work_i[slot], cpu.a);
}

void abi_LoadFrameByWorkD101E(void)
{
    LoadFrameByWorkD101E(cpu.x);
}

/*
 * @routine 10CC:512 LandThenStartPair10CC
 * @run 70:BD12
 * WHAT IT DOES: On landing, frame $9C and kind $2F and $30 objects in slots $11 and $12, substate on (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandThenStartPair10CC(uint8_t slot)
{
    call_routine(0xBD15, 0xBA02);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(0x9C);
    WR8(OBJ->frame[slot], cpu.a);
    ldy(0x11);
    clear_slot_in_y(0xBD21);
    start_at_place(0x2F, 0xBD26);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xBD30);
    start_at_place(0x30, 0xBD35);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LandThenStartPair10CC(void)
{
    LandThenStartPair10CC(cpu.x);
}

/*
 * @routine 10CC:5BA DissolveThenBecome29_10CC
 * @run 70:BDBA
 * WHAT IT DOES: Dissolves patterns 0 to $D while OBJ_WORK_E counts; then the slot becomes kind $29 (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DissolveThenBecome29_10CC(uint8_t slot)
{
    lda(0x00);
    wr8(WorkByteA, cpu.a);
    lda(0x0E);
    wr8(WorkByteB, cpu.a);
    call_routine(0xBDC7, 0xBDE1);
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    stop_here(slot);
    lda(0x29);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_DissolveThenBecome29_10CC(void)
{
    DissolveThenBecome29_10CC(cpu.x);
}

/*
 * @routine 10CD:217 LoadOrChannelWhenFrame71_10CD
 * @run 71:C217
 * WHAT IT DOES: In state 0 loads with scroll band 1 and frame $71; cleared once slot $10 shows it, else list channel $9C (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadOrChannelWhenFrame71_10CD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC21F);
        lda(0x01);
        WR8(OBJ->scroll_band[slot], cpu.a);
        lda(0x71);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->frame[0x10]));
    cmp(cpu.a, 0x71);
    if (cpu.a == 0x71) {
        ClearObject(slot);
        return;
    }
    push8(cpu.x);
    list_channel2(0x9C, 0x04, 0xC23C);
    pull_x();
}

void abi_LoadOrChannelWhenFrame71_10CD(void)
{
    LoadOrChannelWhenFrame71_10CD(cpu.x);
}

/*
 * @routine 10CB:383 ScriptByFrameMatch10CB
 * @run 70:B383
 * WHAT IT DOES: Counts OBJ_WORK_F; when the count is in the table, script $83 plus its place (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ScriptByFrameMatch10CB(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->work_f[slot]));
    if ((cpu.a & 0x80) != 0) {
        return;
    }
    INC8(OBJ->work_f[slot]);
    cpu.y = 0;
    for (;;) {
        cmp(cpu.a, rd8((uint16_t)(0xB3A8 + cpu.y)));
        if ((cpu.p & P_Z) != 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x16);
        if (cpu.y >= 0x16) {
            return;
        }
    }
    lda(cpu.y);
    add_a(0x83, false);
    push8(cpu.x);
    cpu.x = 0x05;
    abi_nz(cpu.x);
    jsr_push(0xB3A6);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
    pull_x();
}

void abi_ScriptByFrameMatch10CB(void)
{
    ScriptByFrameMatch10CB(cpu.x);
}
