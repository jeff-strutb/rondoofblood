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
 * @routine 12DD:3E4 LoadWithArmsAndScript12DD
 * @run 71:C3E4
 * WHAT IT DOES: Loads with kind $32 and $33 objects in slots $11 and $12, list channel $83 and script $8F (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadWithArmsAndScript12DD(uint8_t slot)
{
    load_record(slot, 0xC3E7);
    lda(0xA4);
    WR8(OBJ->y_lo[slot], cpu.a);
    ldy(0x11);
    clear_slot_in_y(0xC3F1);
    start_at_place(0x32, 0xC3F6);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xC3FA);
    start_at_place(0x33, 0xC3FF);
    push8(cpu.x);
    list_channel2(0x83, 0x05, 0xC407);
    script_at(0x8F, 0x04, 0xC40E);
    pull_x();
    lda(0x30);
    WR8(OBJ->work_f[slot], cpu.a);
}

void abi_LoadWithArmsAndScript12DD(void)
{
    LoadWithArmsAndScript12DD(cpu.x);
}

/*
 * @routine 0B4C:273 SpitThenLeapByFacing0B4C
 * @run 70:BA73
 * WHAT IT DOES: Spits ($BABD) playing the facing's list while OBJ_WORK_E counts, then leaps (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpitThenLeapByFacing0B4C(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    call_routine(0xBA76, 0xBABD);
    lda(RD8(OBJ->facing_left[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBAA5 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xBAA6 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    jsr_push(0xBA88);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    cpu.y = RD8(OBJ->facing_left[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBABB + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0xF9);
    WR8(OBJ->vy_hi[slot], cpu.a);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SpitThenLeapByFacing0B4C(void)
{
    SpitThenLeapByFacing0B4C(cpu.x);
}

/*
 * @routine 101C:23E LoadSoundThenListClear101C
 * @run 70:BA3E
 * WHAT IT DOES: Appears with sound $13 rising; plays the list at $BA70 and is cleared when it ends (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadSoundThenListClear101C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xBA46);
        sound(0x13, 0xBA4B);
        restart_animation(slot);
        lda(0xFD);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(0x00);
        WR8(OBJ->vy_lo[slot], cpu.a);
        return;
    }
    move_y(slot, 0xBA5F);
    step_list(slot, 0xBA70, true, 0xBA6A);
    if (!list_ended()) {
        return;
    }
    ClearObject(slot);
}

void abi_LoadSoundThenListClear101C(void)
{
    LoadSoundThenListClear101C(cpu.x);
}

/*
 * @routine 13FB:4A7 FireTwiceAtStep13_13FB
 * @run 70:B4A7
 * WHAT IT DOES: At animation step $13: at timer $30 a kind $40 object in slot $12, at $10 a kind $3F in slot $11, each with sound $46 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FireTwiceAtStep13_13FB(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x13);
    if (cpu.a != 0x13) {
        return;
    }
    lda(RD8(OBJ->anim_timer[slot]));
    cmp(cpu.a, 0x10);
    if (cpu.a == 0x10) {
        sound(0x46, 0xB4CE);
        ldy(0x11);
        clear_slot_in_y(0xB4D3);
        start_at_place(0x3F, 0xB4D8);
        return;
    }
    cmp(cpu.a, 0x30);
    if (cpu.a != 0x30) {
        return;
    }
    sound(0x46, 0xB4BE);
    ldy(0x12);
    clear_slot_in_y(0xB4C3);
    start_at_place(0x40, 0xB4C8);
}

void abi_FireTwiceAtStep13_13FB(void)
{
    FireTwiceAtStep13_13FB(cpu.x);
}

/*
 * @routine 161D:2A3 ListThenThreeShots161D
 * @run 71:C2A3
 * WHAT IT DOES: Plays the list at $C238; then kind $4C shots in slots $12 to $14 with sound $2E each, state 2 (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenThreeShots161D(uint8_t slot)
{
    step_list(slot, 0xC238, true, 0xC2AE);
    if (!list_ended()) {
        return;
    }
    ldy(0x12);
    do {
        clear_slot_in_y(0xC2B5);
        start_at_place(0x4C, 0xC2BA);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        push8(cpu.y);
        sound(0x2E, 0xC2C6);
        cpu.y = pull8();
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x15);
    } while (cpu.y < 0x15);
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenThreeShots161D(void)
{
    ListThenThreeShots161D(cpu.x);
}

/*
 * @routine 16CB:1C2 SpitDebrisEvery8_16CB
 * @run 70:B1C2
 * WHAT IT DOES: Every eighth count of OBJ_WORK_J throws a kind $9C object at random speeds with sound $13 (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpitDebrisEvery8_16CB(uint8_t slot)
{
    lda(RD8(OBJ->work_j[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    if (cpu.a != 0) {
        return;
    }
    if (!find_from30(0xB1CC)) {
        return;
    }
    start_at_place(0x9C, 0xB1D3);
    swap_xy();
    random_a(0xB1D7);
    lda((uint8_t)(cpu.a & 0x1F));
    {
        Sum sum = sub_bytes(cpu.a, 0x10, true);

        lda(sum.value);
        abi_flag(P_C, sum.carry);
        abi_flag(P_V, sum.overflow);
    }
    WR8(OBJ->vx_hi[cpu.x], cpu.a);
    random_a(0xB1E2);
    lda((uint8_t)(cpu.a & 0x1F));
    {
        Sum sum = sub_bytes(cpu.a, 0x20, true);

        lda(sum.value);
        abi_flag(P_C, sum.carry);
        abi_flag(P_V, sum.overflow);
    }
    WR8(OBJ->vy_hi[cpu.x], cpu.a);
    move(cpu.x, 0xB1ED);
    swap_xy();
    sound(0x13, 0xB1F3);
}

void abi_SpitDebrisEvery8_16CB(void)
{
    SpitDebrisEvery8_16CB(cpu.x);
}

/*
 * @routine 117B:199 PaceBackAndForth117B
 * @run 70:B199
 * WHAT IT DOES: Moves along X counting the word OBJ_WORK_H-G; at zero turns (OBJ_WORK_F) and reloads the count by start argument (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PaceBackAndForth117B(uint8_t slot)
{
    move_x(slot, 0xB19C);
    lda(RD8(OBJ->work_h[slot]));
    {
        Sum sum = sub_bytes(cpu.a, 0x01, true);

        lda(sum.value);
        abi_flag(P_C, sum.carry);
        abi_flag(P_V, sum.overflow);
    }
    WR8(OBJ->work_h[slot], cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    {
        Sum sum = sub_bytes(cpu.a, 0x00, (cpu.p & P_C) != 0);

        lda(sum.value);
        abi_flag(P_C, sum.carry);
        abi_flag(P_V, sum.overflow);
    }
    WR8(OBJ->work_g[slot], cpu.a);
    lda((uint8_t)(cpu.a | RD8(OBJ->work_h[slot])));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a ^ 0x01));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xB122 + cpu.y)));
    WR8(OBJ->work_h[slot], cpu.a);
    lda(rd8((uint16_t)(0xB123 + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
}

void abi_PaceBackAndForth117B(void)
{
    PaceBackAndForth117B(cpu.x);
}

/*
 * @routine 12DA:740 ArenaStartOnScreen3_12DA
 * @run 70:AF40
 * WHAT IT DOES: From screen 3, holds the player until slot $50 reaches state 3; then tile script 6, the event recorded, state on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ArenaStartOnScreen3_12DA(uint8_t slot)
{
    TileRun run;

    lda(rd8(ScrollXHiScreen));
    cmp(cpu.a, 0x03);
    if (cpu.a < 0x03) {
        return;
    }
    lda(0x03);
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x02);
    wr8(PlayerStartPose, cpu.a);
    WR8(OBJ->work_f[0], 0);
    lda(0x10);
    WR8(ZP->temp08, cpu.a);
    lda(0x26);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xAF5F, 0xAA35);
    lda(RD8(OBJ->state[0x50]));
    cmp(cpu.a, 0x03);
    if (cpu.a < 0x03) {
        return;
    }
    lda(0x06);
    cpu.y = 0;
    jsr_push(0xAF6C);
    run = DrawTileScript(0x06, 0);
    abi_ended(ended_of_tile_run(run, 0x06));
    jsr_pop();
    record_event(slot, 0xAF6F);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ArenaStartOnScreen3_12DA(void)
{
    ArenaStartOnScreen3_12DA(cpu.x);
}

/*
 * @routine 12DE:127 TurnQuarter12DE
 * @run 71:C927
 * WHAT IT DOES: Moves at angle OBJ_WORK_D turning toward the half-turn OBJ_WORK_I names; there, the state on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TurnQuarter12DE(uint8_t slot)
{
    TurnAngleResult turned;
    bool out;

    lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    velocity_from_a(slot, 0xC932);
    move(slot, 0xC935);
    lda(RD8(OBJ->work_i[slot]));
    lda((uint8_t)(cpu.a ^ 0x01));
    out = (cpu.a & 0x01) != 0;
    lda((uint8_t)((cpu.a >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
    abi_flag(P_C, out);
    out = (cpu.a & 0x01) != 0;
    lda((uint8_t)((cpu.a >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
    abi_flag(P_C, out);
    lda((uint8_t)(cpu.a & 0x80));
    push8(cpu.a);
    WR8(ZP->temp02, cpu.a);
    lda(0x01);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xC94D);
    turned = TurnAngleToward();
    turn_angle_exit_shim(turned);
    jsr_pop();
    WR8(OBJ->work_d[slot], cpu.a);
    lda(pull8());
    cmp(cpu.a, RD8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_TurnQuarter12DE(void)
{
    TurnQuarter12DE(cpu.x);
}

/*
 * @routine 117E:3E6 ListStrikeThenState4_117E
 * @run 71:CBE6
 * WHAT IT DOES: Plays the list at $CC1C, lunging at frame 7 with sound $1F; when it ends, state 4 (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListStrikeThenState4_117E(uint8_t slot)
{
    step_list(slot, 0xCC1C, false, 0xCBF1);
    if (list_ended()) {
        restart_animation(slot);
        lda(0x04);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x07);
    if (cpu.a != 0x07) {
        return;
    }
    sound(0x1F, 0xCC01);
    cpu.y = RD8(OBJ->facing_left[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xCC1A + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    call_routine(0xCC0D, 0xCAE7);
}

void abi_ListStrikeThenState4_117E(void)
{
    ListStrikeThenState4_117E(cpu.x);
}

/*
 * @routine 0B4C:522 ListSoundThenTurnLeap0B4C
 * @run 70:BD22
 * WHAT IT DOES: Plays the list at $BD57 with sound $19 at frame $3E; then faces the player and starts walking (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSoundThenTurnLeap0B4C(uint8_t slot)
{
    call_routine(0xBD25, 0xB77C);
    step_list(slot, 0xBD57, false, 0xBD30);
    if (!list_ended()) {
        if ((cpu.p & P_C) == 0) {
            return;
        }
        lda(RD8(OBJ->frame[slot]));
        cmp(cpu.a, 0x3E);
        if (cpu.a != 0x3E) {
            return;
        }
        sound(0x19, 0xBD40);
        return;
    }
    face_by_side(slot, COL(OBJ->facing_left), 0xBD44);
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xBDB7 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    WR8(OBJ->vx_lo[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListSoundThenTurnLeap0B4C(void)
{
    ListSoundThenTurnLeap0B4C(cpu.x);
}

/*
 * @routine 0B4C:5E5 FallListThenRest0B4C
 * @run 70:BDE5
 * WHAT IT DOES: Every fourth count falls playing the list at $BE1A; once falling again, stops with no hit box (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallListThenRest0B4C(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    step_list(slot, 0xBE1A, false, 0xBDF7);
    jsr_push(0xBDFA);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    move_y(slot, 0xBDFD);
    lda(RD8(OBJ->vy_hi[slot]));
    if ((cpu.a & 0x80) != 0) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    restart_animation(slot);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_FallListThenRest0B4C(void)
{
    FallListThenRest0B4C(cpu.x);
}

/*
 * @routine 0EC0:226 LoadHopThenWalk0EC0
 * @run 71:DA26
 * WHAT IT DOES: Appears hopping with sound $25 and its facing's speed; later walks playing the list at $DA5F (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadHopThenWalk0EC0(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        sound(0x25, 0xDA30);
        load_record(slot, 0xDA33);
        lda(0xF5);
        WR8(OBJ->vy_hi[slot], cpu.a);
        move_y(slot, 0xDA3B);
        lda(RD8(OBJ->facing_left[slot]));
        word_index(cpu.a);
        lda(rd8((uint16_t)(0xDA5C + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xDA5B + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        return;
    }
    step_list(slot, 0xDA5F, true, 0xDA58);
    axis_move_exit_shim(ApplyVelocityX(slot));
}

void abi_LoadHopThenWalk0EC0(void)
{
    LoadHopThenWalk0EC0(cpu.x);
}

/*
 * @routine 161C:5B9 StartRowByWorkK161C
 * @run 70:BDB9
 * WHAT IT DOES: Starts kind $51 objects numbered over the range OBJ_WORK_K names, then clears itself (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartRowByWorkK161C(uint8_t slot)
{
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xBDBF);
    lda(RD8(OBJ->work_k[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBDEE + cpu.y)));
    WR8(ZP->temp00, cpu.a);
    lda(rd8((uint16_t)(0xBDEF + cpu.y)));
    WR8(ZP->temp01, cpu.a);
    do {
        if (!find_from30(0xBDD1)) {
            break;
        }
        start_at_place(0x51, 0xBDD8);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        INC8(ZP->temp00);
        lda(RD8(ZP->temp00));
        cmp(cpu.a, RD8(ZP->temp01));
    } while ((cpu.p & P_C) == 0);
    ClearObject(slot);
}

void abi_StartRowByWorkK161C(void)
{
    StartRowByWorkK161C(cpu.x);
}

/*
 * @routine 161E:035 ListThenBurstSix161E
 * @run 71:C835
 * WHAT IT DOES: Plays the list at $C86A; then sound $88, six numbered kind $4F objects, and cleared (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenBurstSix161E(uint8_t slot)
{
    step_list(slot, 0xC86A, true, 0xC840);
    if (!list_ended()) {
        return;
    }
    sound(0x88, 0xC847);
    WR8(ZP->temp00, 0);
    do {
        if (!find_from30(0xC84C)) {
            break;
        }
        start_at_place(0x4F, 0xC853);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        INC8(ZP->temp00);
        lda(RD8(ZP->temp00));
        cmp(cpu.a, 0x06);
    } while (cpu.a < 0x06);
    ClearObject(slot);
}

void abi_ListThenBurstSix161E(void)
{
    ListThenBurstSix161E(cpu.x);
}

/*
 * @routine 12DE:0AD LoadByArg12DE
 * @run 71:C8AD
 * WHAT IT DOES: Loads 16 dots up with angle and speed from tables by the start argument (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadByArg12DE(uint8_t slot)
{
    load_record(slot, 0xC8B0);
    lda(0xF0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xC8B8);
    cpu.y = RD8(OBJ->start_arg[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC912 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(rd8((uint16_t)(0xC8E0 + cpu.y)));
    cpu.y = RD8(OBJ->work_i[slot]);
    abi_nz(cpu.y);
    add_a(rd8((uint16_t)(0xC8F0 + cpu.y)), false);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC8F3 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(rd8((uint16_t)(0xC8F2 + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
}

void abi_LoadByArg12DE(void)
{
    LoadByArg12DE(cpu.x);
}

/*
 * @routine 12DC:4A8 WaveAccumulate12DC
 * @run 70:BCA8
 * WHAT IT DOES: Adds the cosine of OBJ_WORK_G scaled by $C0 into OBJ_WORK_E-F, the angle stepped; a full turn clears the sum (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WaveAccumulate12DC(uint8_t slot)
{
    lda(0xC0);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    jsr_push(0xBCB6);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(RD8(OBJ->work_f[slot]));
    add_a(rd8(zProduct1), false);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    add_a(rd8(zProduct2), (cpu.p & P_C) != 0);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_g[slot]));
    add_a(0x04, false);
    WR8(OBJ->work_g[slot], cpu.a);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    WR8(OBJ->work_e[slot], 0);
    WR8(OBJ->work_f[slot], 0);
}

void abi_WaveAccumulate12DC(void)
{
    WaveAccumulate12DC(cpu.x);
}
