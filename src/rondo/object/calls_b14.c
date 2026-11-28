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
#include "calls_b14.h"

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

static void gravity(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    gravity_exit_shim(ApplyGravity(slot));
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
static void add_a(uint8_t value, bool carry_in)
{
    Sum sum = add_bytes(cpu.a, value, carry_in);

    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
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

static void near_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
}
/*
 * @routine 0EBE:386 FaceAndAnimate0EBE
 * @run 71:CB86
 * WHAT IT DOES: Faces the player and runs that side's frame list.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FaceAndAnimate0EBE(uint8_t slot)
{
    face_by_side(slot, COL(OBJ->facing_left), 0xCB89);
    lda(rd8((uint16_t)(0xCB9B + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xCB9C + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step = StepObjectFrames(slot, &y);

        step_frames_exit_shim(step, y);
    }
}

void abi_FaceAndAnimate0EBE(void)
{
    FaceAndAnimate0EBE(cpu.x);
}

/*
 * @routine 0EBF:19C FaceAndAnimate0EBF
 * @run 71:D19C
 * WHAT IT DOES: Faces the player and runs that side's frame list.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FaceAndAnimate0EBF(uint8_t slot)
{
    face_by_side(slot, COL(OBJ->facing_left), 0xD19F);
    lda(rd8((uint16_t)(0xD1B1 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xD1B2 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step = StepObjectFrames(slot, &y);

        step_frames_exit_shim(step, y);
    }
}

void abi_FaceAndAnimate0EBF(void)
{
    FaceAndAnimate0EBF(cpu.x);
}

/*
 * @routine 161C:01C FaceAndAnimate161C
 * @run 70:B81C
 * WHAT IT DOES: Faces the player and runs that side's frame list.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FaceAndAnimate161C(uint8_t slot)
{
    face_by_side(slot, COL(OBJ->facing_left), 0xB81F);
    lda(rd8((uint16_t)(0xB831 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xB832 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step = StepObjectFrames(slot, &y);

        step_frames_exit_shim(step, y);
    }
}

void abi_FaceAndAnimate161C(void)
{
    FaceAndAnimate161C(cpu.x);
}

/*
 * @routine 05CC:4D8 LoadFrameByArg05CC
 * @run 70:BCD8
 * WHAT IT DOES: In state 0 loads, with the start argument as the frame, in sprite set 6.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFrameByArg05CC(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a != 0) {
        return;
    }
    load_record(slot, 0xBCE0);
    lda(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x06);
    WR8(OBJ->sprite_set[slot], cpu.a);
}

void abi_LoadFrameByArg05CC(void)
{
    LoadFrameByArg05CC(cpu.x);
}

/*
 * @routine 13FC:04B ListThenSubstate13FC
 * @run 70:B84B
 * WHAT IT DOES: Runs $B6CC and the list at $B85F; at its end the substate counts on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenSubstate13FC(uint8_t slot)
{
    call_routine(0xB84E, 0xB6CC);
    step_list(slot, 0xB85F, true, 0xB859);
    if (!list_ended()) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenSubstate13FC(void)
{
    ListThenSubstate13FC(cpu.x);
}

/*
 * @routine 088C:3E6 LoadScriptInStageFiveClear088C
 * @run 70:BBE6
 * WHAT IT DOES: Loads and clears the object, with script $92 first when the stage asked for is 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadScriptInStageFiveClear088C(uint8_t slot)
{
    load_record(slot, 0xBBE9);
    lda(rd8(zStageRequested));
    cmp(cpu.a, 0x05);
    if (cpu.a == 0x05) {
        push8(cpu.x);
        script_at(0x92, 0x05, 0xBBF7);
        pull_x();
    }
    ClearObject(slot);
}

void abi_LoadScriptInStageFiveClear088C(void)
{
    LoadScriptInStageFiveClear088C(cpu.x);
}

/*
 * @routine 101B:281 StartKind9BOnBeat101B
 * @run 70:B281
 * WHAT IT DOES: When bits 1-3 of the wave counter are the start argument, starts kind $9B here in a free slot.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartKind9BOnBeat101B(uint8_t slot)
{
    lda(rd8(zWaveCounter));
    lda((uint8_t)(cpu.a >> 1));
    abi_flag(P_C, (rd8(zWaveCounter) & 1) != 0);
    lda((uint8_t)(cpu.a & 0x07));
    cmp(cpu.a, RD8(OBJ->start_arg[slot]));
    if (cpu.a != RD8(OBJ->start_arg[slot])) {
        return;
    }
    if (!find_from30(0xB28E)) {
        return;
    }
    start_at_place(0x9B, 0xB295);
}

void abi_StartKind9BOnBeat101B(void)
{
    StartKind9BOnBeat101B(cpu.x);
}

/*
 * @routine 101B:7D8 RestartIntoStateTwo101B
 * @run 70:B7D8
 * WHAT IT DOES: Clears OBJ_WORK_E, runs $B73B and starts state 2 with its animation from the start.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RestartIntoStateTwo101B(uint8_t slot)
{
    WR8(OBJ->work_e[slot], 0);
    call_routine(0xB7DE, 0xB73B);
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
    restart_animation(slot);
}

void abi_RestartIntoStateTwo101B(void)
{
    RestartIntoStateTwo101B(cpu.x);
}

/*
 * @routine 101C:6ED LandThenFrame74_101C
 * @run 70:BEED
 * WHAT IT DOES: On landing shows frame $74 for 6 counts, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandThenFrame74_101C(uint8_t slot)
{
    lda(0x06);
    jsr_push(0xBEF2);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(0x74);
    WR8(OBJ->frame[slot], cpu.a);
    INC8(OBJ->substate[slot]);
    lda(0x06);
    WR8(OBJ->anim_timer[slot], cpu.a);
}

void abi_LandThenFrame74_101C(void)
{
    LandThenFrame74_101C(cpu.x);
}

/*
 * @routine 101F:698 WakeWhenNear101F
 * @run 71:D698
 * WHAT IT DOES: With the player nearer than $50 across, the state counts on; OBJ_WORK_D $80, sprite attribute $10.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WakeWhenNear101F(uint8_t slot)
{
    near_x(slot, 0xD69B);
    cmp(cpu.a, 0x50);
    if (cpu.a >= 0x50) {
        return;
    }
    INC8(OBJ->state[slot]);
    lda(0x80);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->sprite_attr[slot], cpu.a);
}

void abi_WakeWhenNear101F(void)
{
    WakeWhenNear101F(cpu.x);
}

/*
 * @routine 10CC:4FD SoundHitOffWait10CC
 * @run 70:BCFD
 * WHAT IT DOES: Sound $E6, hitbox off, raster stage 3, a wait of $40 in OBJ_WORK_E, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundHitOffWait10CC(uint8_t slot)
{
    sound(0xE6, 0xBD02);
    WR8(OBJ->hitbox_on[slot], 0);
    lda(0x03);
    wr8(zRasterStage, cpu.a);
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundHitOffWait10CC(void)
{
    SoundHitOffWait10CC(cpu.x);
}

/*
 * @routine 16CC:25E LoadByPieceSlot16CC
 * @run 70:BA5E
 * WHAT IT DOES: Loads; OBJ_WORK_E and the frame by the slot, counted from $11.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadByPieceSlot16CC(uint8_t slot)
{
    load_record(slot, 0xBA61);
    lda(cpu.x);
    add_a((uint8_t)~0x11, true);
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBA73 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(rd8((uint16_t)(0xBA78 + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_LoadByPieceSlot16CC(void)
{
    LoadByPieceSlot16CC(cpu.x);
}

/*
 * @routine 09EC:23B LoadByPieceSlot09EC
 * @run 70:BA3B
 * WHAT IT DOES: Loads; OBJ_WORK_D by the slot counted from $20, frame $38 on from it.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadByPieceSlot09EC(uint8_t slot)
{
    load_record(slot, 0xBA3E);
    lda(cpu.x);
    add_a((uint8_t)~0x20, true);
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBA51 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
    lda(cpu.y);
    add_a(0x38, false);
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_LoadByPieceSlot09EC(void)
{
    LoadByPieceSlot09EC(cpu.x);
}

/*
 * @routine 0B4C:6F3 ScriptByFrame0B4C
 * @run 70:BEF3
 * WHAT IT DOES: On frames $48-$4A starts that frame's script in slot 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ScriptByFrame0B4C(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    add_a((uint8_t)~0x48, true);
    cmp(cpu.a, 0x03);
    if (cpu.a >= 0x03) {
        return;
    }
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    push8(cpu.x);
    script_at(rd8((uint16_t)(0xBF09 + cpu.y)), 0x05, 0xBF07);
    pull_x();
}

void abi_ScriptByFrame0B4C(void)
{
    ScriptByFrame0B4C(cpu.x);
}

/*
 * @routine 101B:124 ListThenStateTwo101B
 * @run 70:B124
 * WHAT IT DOES: Runs the list at $B13A; at its end state 2.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenStateTwo101B(uint8_t slot)
{
    step_list(slot, 0xB13A, true, 0xB12F);
    if (!list_ended()) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenStateTwo101B(void)
{
    ListThenStateTwo101B(cpu.x);
}

/*
 * @routine 0EBC:0BE CallWhenLowAndDue0EBC
 * @run 70:B8BE
 * WHAT IT DOES: With OBJ_WORK_G 0 and OBJ_WORK_F at least $B8 runs $B3E4.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CallWhenLowAndDue0EBC(uint8_t slot)
{
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xD8);
    lda(RD8(OBJ->work_g[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->work_f[slot]));
    cmp(cpu.a, 0xB8);
    if (cpu.a < 0xB8) {
        return;
    }
    call_routine(0xB8D4, 0xB3E4);
}

void abi_CallWhenLowAndDue0EBC(void)
{
    CallWhenLowAndDue0EBC(cpu.x);
}

/*
 * @routine 12DB:08D ClearUnlessScriptFive12DB
 * @run 70:B08D
 * WHAT IT DOES: Once script slot 3's kind is not 5: sets slot 0's OBJ_WORK_E and F, asks the mode (zModeEvents bit 0) and clears.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ClearUnlessScriptFive12DB(uint8_t slot)
{
    lda(rd8((uint16_t)(ScriptSlotKind + 3)));
    cmp(cpu.a, 0x05);
    if (cpu.a == 0x05) {
        return;
    }
    lda(0x04);
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x10);
    WR8(OBJ->work_f[0], cpu.a);
    wr8(zModeEvents, (uint8_t)(rd8(zModeEvents) | 0x01));
    ClearObject(slot);
}

void abi_ClearUnlessScriptFive12DB(void)
{
    ClearUnlessScriptFive12DB(cpu.x);
}

/*
 * @routine 16CB:267 WaitThenScriptSound16CB
 * @run 70:B267
 * WHAT IT DOES: When OBJ_WORK_D runs out: script $82 in slot 5, substate on, sound $25.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WaitThenScriptSound16CB(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    push8(cpu.x);
    script_at(0x82, 0x05, 0xB274);
    pull_x();
    INC8(OBJ->substate[slot]);
    sound(0x25, 0xB27D);
}

void abi_WaitThenScriptSound16CB(void)
{
    WaitThenScriptSound16CB(cpu.x);
}

/*
 * @routine 05CB:734 ListThenWait05CB
 * @run 70:B734
 * WHAT IT DOES: Runs $B706 and the list at $B74D; at its end a wait of $F0, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenWait05CB(uint8_t slot)
{
    call_routine(0xB737, 0xB706);
    step_list(slot, 0xB74D, true, 0xB742);
    if (!list_ended()) {
        return;
    }
    lda(0xF0);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenWait05CB(void)
{
    ListThenWait05CB(cpu.x);
}

/*
 * @routine 088B:4DD FallThenLandCall088B
 * @run 70:B4DD
 * WHAT IT DOES: Moves and falls; above Y $18, or no longer rising, runs $B784 and the substate counts on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallThenLandCall088B(uint8_t slot)
{
    move(slot, 0xB4E0);
    gravity(slot, 0xB4E3);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x18);
    if (cpu.a >= 0x18) {
        lda(RD8(OBJ->vy_hi[slot]));
        if ((cpu.p & P_N) != 0) {
            return;
        }
    }
    call_routine(0xB4F2, 0xB784);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_FallThenLandCall088B(void)
{
    FallThenLandCall088B(cpu.x);
}

/*
 * @routine 09EC:31A LoadThenRideSlot10_09EC
 * @run 70:BB1A
 * WHAT IT DOES: Loads in state 0; after it stays at slot $10's place running the list at $BB33.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadThenRideSlot10_09EC(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xBB22);
        return;
    }
    ldy(0x10);
    jsr_push(0xBB28);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(0x33);
    WR8(ZP->temp10, cpu.a);
    lda(0xBB);
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
}

void abi_LoadThenRideSlot10_09EC(void)
{
    LoadThenRideSlot10_09EC(cpu.x);
}

/*
 * @routine 0E0C:56E StartSevenKind26_0E0C
 * @run 70:BD6E
 * WHAT IT DOES: Starts numbered kind $26 objects in slots $18-$1E.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartSevenKind26_0E0C(uint8_t slot)
{
    ldy(0x18);
    WR8(ZP->temp00, 0);
    do {
        clear_slot_in_y(0xBD75);
        start_at_place(0x26, 0xBD7A);
        lda(RD8(ZP->temp00));
        WR8(OBJ->start_arg[cpu.y], cpu.a);
        INC8(ZP->temp00);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x1F);
    } while (cpu.y < 0x1F);
}

void abi_StartSevenKind26_0E0C(void)
{
    StartSevenKind26_0E0C(cpu.x);
}

/*
 * @routine 0F6C:7FD FaceCallStateTwelve0F6C
 * @run 70:BFFD
 * WHAT IT DOES: Faces the player, OBJ_WORK_D 1, runs $C35F and goes to state 12.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FaceCallStateTwelve0F6C(uint8_t slot)
{
    face_by_side(slot, COL(OBJ->facing_left), 0xC000);
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
    call_routine(0xC00D, 0xC35F);
    lda(0x0C);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_FaceCallStateTwelve0F6C(void)
{
    FaceCallStateTwelve0F6C(cpu.x);
}

/*
 * @routine 0EBF:57F ClearOneEmptyOf20to27_0EBF
 * @run 71:D57F
 * WHAT IT DOES: Finds an empty slot in $20-$27 and clears it; carry set when there is one.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: as the last call, compare or load left them.
 */
void ClearOneEmptyOf20to27_0EBF(void)
{
    ldy(0x20);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (cpu.a == 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x28);
        if (cpu.y >= 0x28) {
            abi_flag(P_C, false);
            return;
        }
    }
    clear_slot_in_y(0xD590);
    abi_flag(P_C, true);
}

void abi_ClearOneEmptyOf20to27_0EBF(void)
{
    ClearOneEmptyOf20to27_0EBF();
}
