/*
 * More small calling routines of stages $09 to $10 and the secret stages: frame lists ending in a state step, sounds,
 * spawns in free slots, turns to the player, per-pattern loops.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "calls_b2.h"

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

/* Under the table byte for its state, nothing; else the object turned by the side the player is on. */
static void turn_by_state_table(uint8_t slot, uint16_t table, uint16_t ret)
{
    cpu.y = RD8(OBJ->state[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(table + cpu.y)));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(ret);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    WR8(OBJ->facing_left[slot], cpu.a);
}

/* PHX, JSR the first routine, then the second for X from 0 up to count, PLX. */
static void each_of(uint16_t first, uint16_t first_ret, uint16_t each, uint16_t each_ret, uint8_t count)
{
    push8(cpu.x);
    call_routine(first_ret, first);
    cpu.x = 0;
    do {
        call_routine(each_ret, each);
        cpu.x = (uint8_t)(cpu.x + 1);
        cmp(cpu.x, count);
    } while (cpu.x < count);
    cpu.x = pull8();
    abi_nz(cpu.x);
}

/*
 * @routine 0CAC:059 ListThenStep0CAC_059
 * @run 70:B859
 * WHAT IT DOES: Plays the frame list at $B86A; when it ends, the substate stepped.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
 */
void ListThenStep0CAC_059(uint8_t slot)
{
    step_list(slot, 0xB86A, true, 0xB864);
    if (cpu.a != 0) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenStep0CAC_059(void)
{
    ListThenStep0CAC_059(cpu.x);
}

/*
 * @routine 0CAC:178 ListThenStep0CAC_178
 * @run 70:B978
 * WHAT IT DOES: Plays the frame list at $B989; when it ends, the substate stepped.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
 */
void ListThenStep0CAC_178(uint8_t slot)
{
    step_list(slot, 0xB989, true, 0xB983);
    if (cpu.a != 0) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenStep0CAC_178(void)
{
    ListThenStep0CAC_178(cpu.x);
}

/*
 * @routine 0EBD:536 ListThenStep0EBD_536
 * @run 71:C536
 * WHAT IT DOES: Plays the frame list at $C56E; when it ends, the state stepped.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
 */
void ListThenStep0EBD_536(uint8_t slot)
{
    step_list(slot, 0xC56E, true, 0xC541);
    if (cpu.a != 0) {
        return;
    }
    abi_nz(DEC8(OBJ->state[slot]));
}

void abi_ListThenStep0EBD_536(void)
{
    ListThenStep0EBD_536(cpu.x);
}

/*
 * @routine 161B:388 ListThenStep161B_388
 * @run 70:B388
 * WHAT IT DOES: Plays the frame list at $B399; when it ends, the state stepped.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
 */
void ListThenStep161B_388(uint8_t slot)
{
    step_list(slot, 0xB399, true, 0xB393);
    if (cpu.a != 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListThenStep161B_388(void)
{
    ListThenStep161B_388(cpu.x);
}

/*
 * @routine 161C:400 ListThenStep161C_400
 * @run 70:BC00
 * WHAT IT DOES: Plays the frame list at $BC11; when it ends, the state stepped.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
 */
void ListThenStep161C_400(uint8_t slot)
{
    step_list(slot, 0xBC11, true, 0xBC0B);
    if (cpu.a != 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListThenStep161C_400(void)
{
    ListThenStep161C_400(cpu.x);
}

/*
 * @routine 0EBE:7C2 ListThenStep0EBE_7C2
 * @run 71:CFC2
 * WHAT IT DOES: Plays the frame list at $CFD3; when it ends, the substate cleared.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
 */
void ListThenStep0EBE_7C2(uint8_t slot)
{
    step_list(slot, 0xCFD3, true, 0xCFCD);
    if (cpu.a != 0) {
        return;
    }
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenStep0EBE_7C2(void)
{
    ListThenStep0EBE_7C2(cpu.x);
}

/*
 * @routine 0EBE:7DC ListThenStep0EBE_7DC
 * @run 71:CFDC
 * WHAT IT DOES: Plays the frame list at $CFED; when it ends, the substate cleared.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
 */
void ListThenStep0EBE_7DC(uint8_t slot)
{
    step_list(slot, 0xCFED, true, 0xCFE7);
    if (cpu.a != 0) {
        return;
    }
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenStep0EBE_7DC(void)
{
    ListThenStep0EBE_7DC(cpu.x);
}

/*
 * @routine 10CC:001 ListThenSubstate3_10CC
 * @run 70:B801
 * WHAT IT DOES: Plays the frame list at $B814; when it ends, substate 3 (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenSubstate3_10CC(uint8_t slot)
{
    step_list(slot, 0xB814, true, 0xB80C);
    if (cpu.a != 0) {
        return;
    }
    lda(0x03);
    WR8(OBJ->substate[slot], cpu.a);
}

void abi_ListThenSubstate3_10CC(void)
{
    ListThenSubstate3_10CC(cpu.x);
}

/*
 * @routine 117E:10F ListThenMove117E_10F
 * @run 71:C90F
 * WHAT IT DOES: Plays the frame list at $C8BE, then moves the object.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenMove117E_10F(uint8_t slot)
{
    step_list(slot, 0xC8BE, false, 0xC91A);
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_ListThenMove117E_10F(void)
{
    ListThenMove117E_10F(cpu.x);
}

/*
 * @routine 12DD:1FB ListThenMove12DD_1FB
 * @run 71:C1FB
 * WHAT IT DOES: Plays the frame list at $C20A, then moves the object.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenMove12DD_1FB(uint8_t slot)
{
    step_list(slot, 0xC20A, true, 0xC206);
    axis_move_exit_shim(ApplyVelocityX(slot));
}

void abi_ListThenMove12DD_1FB(void)
{
    ListThenMove12DD_1FB(cpu.x);
}

/*
 * @routine 0CAB:759 SoundRestartSubstate0CAB_759
 * @run 70:B759
 * WHAT IT DOES: Sound $11, the animation restarted, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundRestartSubstate0CAB_759(uint8_t slot)
{
    sound(0x11, 0xB75E);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundRestartSubstate0CAB_759(void)
{
    SoundRestartSubstate0CAB_759(cpu.x);
}

/*
 * @routine 10CC:455 SoundRestartSubstate10CC_455
 * @run 70:BC55
 * WHAT IT DOES: Sound $44, the animation restarted, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundRestartSubstate10CC_455(uint8_t slot)
{
    sound(0x44, 0xBC5A);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundRestartSubstate10CC_455(void)
{
    SoundRestartSubstate10CC_455(cpu.x);
}

/*
 * @routine 12DB:5B5 SoundRestartSubstate12DB_5B5
 * @run 70:B5B5
 * WHAT IT DOES: Sound $75, the animation restarted, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundRestartSubstate12DB_5B5(uint8_t slot)
{
    sound(0x75, 0xB5BA);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundRestartSubstate12DB_5B5(void)
{
    SoundRestartSubstate12DB_5B5(cpu.x);
}

/*
 * @routine 12DB:2E6 CountThenSound73_12DB
 * @run 70:B2E6
 * WHAT IT DOES: Counts OBJ_WORK_E; at zero sound $73 and the state on (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenSound73_12DB(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->work_e[slot]);

    abi_nz(left);
    if (left != 0) {
        return;
    }
    sound(0x73, 0xB2F0);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_CountThenSound73_12DB(void)
{
    CountThenSound73_12DB(cpu.x);
}

/*
 * @routine 117D:496 FacePlayerInStates117D_496
 * @run 71:C496
 * WHAT IT DOES: In the states the table at $C4A7 marks, turns the object by the player's side (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FacePlayerInStates117D_496(uint8_t slot)
{
    turn_by_state_table(slot, 0xC4A7, 0xC4A1);
}

void abi_FacePlayerInStates117D_496(void)
{
    FacePlayerInStates117D_496(cpu.x);
}

/*
 * @routine 117E:1AF FacePlayerInStates117E_1AF
 * @run 71:C9AF
 * WHAT IT DOES: In the states the table at $C9C0 marks, turns the object by the player's side (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FacePlayerInStates117E_1AF(uint8_t slot)
{
    turn_by_state_table(slot, 0xC9C0, 0xC9BA);
}

void abi_FacePlayerInStates117E_1AF(void)
{
    FacePlayerInStates117E_1AF(cpu.x);
}

/*
 * @routine 117F:296 FacePlayerInStates117F_296
 * @run 71:D296
 * WHAT IT DOES: In the states the table at $D2A7 marks, turns the object by the player's side (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FacePlayerInStates117F_296(uint8_t slot)
{
    turn_by_state_table(slot, 0xD2A7, 0xD2A1);
}

void abi_FacePlayerInStates117F_296(void)
{
    FacePlayerInStates117F_296(cpu.x);
}

/*
 * @routine 101D:45C LoadAndStartC0_101D
 * @run 71:C45C
 * WHAT IT DOES: Loads the record and starts a kind $C0 object at its place in slot $1F (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAndStartC0_101D(uint8_t slot)
{
    load_record(slot, 0xC45F);
    cpu.y = 0x1F;
    abi_nz(cpu.y);
    clear_slot_in_y(0xC464);
    start_at_place(0xC0, 0xC469);
}

void abi_LoadAndStartC0_101D(void)
{
    LoadAndStartC0_101D(cpu.x);
}

/*
 * @routine 13FB:3BC StartKind3DAt17_13FB
 * @run 70:B3BC
 * WHAT IT DOES: Starts a kind $3D object at this place in slot $11, substate on (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartKind3DAt17_13FB(uint8_t slot)
{
    cpu.y = 0x11;
    abi_nz(cpu.y);
    clear_slot_in_y(0xB3C1);
    start_at_place(0x3D, 0xB3C6);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartKind3DAt17_13FB(void)
{
    StartKind3DAt17_13FB(cpu.x);
}

/*
 * @routine 072C:3C7 SpawnEveryFew072C_3C7
 * @run 70:BBC7
 * WHAT IT DOES: On some counts of OBJ_WORK_D, starts a kind $2E object at this place (secret stage 03's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnEveryFew072C_3C7(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    if (cpu.a != 0) {
        return;
    }
    start_in_free_slot(true, 0x2E, 0xBBD1, 0xBBD8);
}

void abi_SpawnEveryFew072C_3C7(void)
{
    SpawnEveryFew072C_3C7(cpu.x);
}

/*
 * @routine 09EB:3C4 SpawnEveryFew09EB_3C4
 * @run 70:B3C4
 * WHAT IT DOES: On some counts of OBJ_WORK_G, starts a kind $30 object at this place (secret stage 05's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnEveryFew09EB_3C4(uint8_t slot)
{
    lda(RD8(OBJ->work_g[slot]));
    lda((uint8_t)(cpu.a & 0x3F));
    if (cpu.a != 0) {
        return;
    }
    start_in_free_slot(false, 0x30, 0xB3CE, 0xB3D5);
}

void abi_SpawnEveryFew09EB_3C4(void)
{
    SpawnEveryFew09EB_3C4(cpu.x);
}

/*
 * @routine 13FB:63F SpawnEveryFew13FB_63F
 * @run 70:B63F
 * WHAT IT DOES: On some counts of OBJ_WORK_D, starts a kind $41 object at this place (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnEveryFew13FB_63F(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        return;
    }
    start_in_free_slot(false, 0x41, 0xB649, 0xB650);
}

void abi_SpawnEveryFew13FB_63F(void)
{
    SpawnEveryFew13FB_63F(cpu.x);
}

/*
 * @routine 12DB:297 SpawnOnOddFrames12DB
 * @run 70:B297
 * WHAT IT DOES: Every other frame starts a kind $26 object at this place (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnOnOddFrames12DB(uint8_t slot)
{
    lda(RD8(ZP->wave_counter));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        return;
    }
    start_in_free_slot(false, 0x26, 0xB2A0, 0xB2A7);
}

void abi_SpawnOnOddFrames12DB(void)
{
    SpawnOnOddFrames12DB(cpu.x);
}

/*
 * @routine 16CC:583 SpawnAtRandom16CC
 * @run 70:BD83
 * WHAT IT DOES: One frame in 16 starts a kind $9B object at this place (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnAtRandom16CC(uint8_t slot)
{
    jsr_push(0xBD86);
    random_exit_shim(NextRandom());
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x0F));
    if (cpu.a != 0) {
        return;
    }
    start_in_free_slot(false, 0x9B, 0xBD8D, 0xBD94);
}

void abi_SpawnAtRandom16CC(void)
{
    SpawnAtRandom16CC(cpu.x);
}

/*
 * @routine 0CAC:21E SpawnWithSound0CAC
 * @run 70:BA1E
 * WHAT IT DOES: Sound $18 and a kind $2D object at this place, when a slot is free (secret stage 07's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnWithSound0CAC(uint8_t slot)
{
    uint8_t found_a, to;
    bool found;

    jsr_push(0xBA21);
    to = FindFreeSlotFrom20(&found_a, &found);
    find_slot_exit_shim(to, found_a, found);
    jsr_pop();
    if (!found) {
        return;
    }
    sound(0x18, 0xBA28);
    start_at_place(0x2D, 0xBA2D);
}

void abi_SpawnWithSound0CAC(void)
{
    SpawnWithSound0CAC(cpu.x);
}

/*
 * @routine 0B4D:115 LoadFrame4DWait0B4D_115
 * @run 71:C115
 * WHAT IT DOES: Loads the record, frame $4D, OBJ_WORK_D 192 (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFrame4DWait0B4D_115(uint8_t slot)
{
    load_record(slot, 0xC118);
    lda(0x4D);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0xC0);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_LoadFrame4DWait0B4D_115(void)
{
    LoadFrame4DWait0B4D_115(cpu.x);
}

/*
 * @routine 12DB:451 LoadThenWaits12DB
 * @run 70:B451
 * WHAT IT DOES: Loads the record, OBJ_WORK_E $70 and OBJ_WORK_H $80 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadThenWaits12DB(uint8_t slot)
{
    load_record(slot, 0xB454);
    lda(0x70);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->work_h[slot], cpu.a);
}

void abi_LoadThenWaits12DB(void)
{
    LoadThenWaits12DB(cpu.x);
}

/*
 * @routine 0EBB:083 LoadFrameByStep0EBB
 * @run 70:B083
 * WHAT IT DOES: Loads the record; the frame by the step through the stage, sprite set 6 (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFrameByStep0EBB(uint8_t slot)
{
    load_record(slot, 0xB086);
    cpu.y = RD8(ZP->field82);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xB094 + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x06);
    WR8(OBJ->sprite_set[slot], cpu.a);
}

void abi_LoadFrameByStep0EBB(void)
{
    LoadFrameByStep0EBB(cpu.x);
}

/*
 * @routine 12DD:059 LoadFrameByWorkD12DD
 * @run 71:C059
 * WHAT IT DOES: Loads the record; the frame by OBJ_WORK_D, OBJ_WORK_I 8 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFrameByWorkD12DD(uint8_t slot)
{
    load_record(slot, 0xC05C);
    cpu.y = RD8(OBJ->work_d[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC06B + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x08);
    WR8(OBJ->work_i[slot], cpu.a);
}

void abi_LoadFrameByWorkD12DD(void)
{
    LoadFrameByWorkD12DD(cpu.x);
}

/*
 * @routine 12D0:4ED LoadFrameByArg12D0
 * @run 6B:9CED
 * WHAT IT DOES: Loads the record; the frame by the start argument's bit 0 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFrameByArg12D0(uint8_t slot)
{
    load_record(slot, 0x9CF0);
    lda(RD8(OBJ->start_arg[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0x9CFD + cpu.y)));
    WR8(OBJ->frame[slot], cpu.a);
}

void abi_LoadFrameByArg12D0(void)
{
    LoadFrameByArg12D0(cpu.x);
}

/*
 * @routine 0EBE:015 LoadAttrByArg0EBE
 * @run 71:C815
 * WHAT IT DOES: Loads the record, attribute $8C; a start argument counts the state on (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAttrByArg0EBE(uint8_t slot)
{
    load_record(slot, 0xC818);
    lda(0x8C);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a == 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_LoadAttrByArg0EBE(void)
{
    LoadAttrByArg0EBE(cpu.x);
}

/*
 * @routine 101B:0BC HitBoxWhenNear101B
 * @run 70:B0BC
 * WHAT IT DOES: Within $60 dots of the player along X: hit-tested, state on (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void HitBoxWhenNear101B(uint8_t slot)
{
    jsr_push(0xB0BF);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
    cmp(cpu.a, 0x60);
    if (cpu.a >= 0x60) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_HitBoxWhenNear101B(void)
{
    HitBoxWhenNear101B(cpu.x);
}

/*
 * @routine 161D:644 StateOnWhenHitOrNear161D
 * @run 71:C644
 * WHAT IT DOES: Counts the state on once hit, or once the player is within $68 dots (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StateOnWhenHitOrNear161D(uint8_t slot)
{
    lda(RD8(OBJ->hit_taken[slot]));
    if (cpu.a == 0) {
        jsr_push(0xC64C);
        abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
        jsr_pop();
        cmp(cpu.a, 0x68);
        if (cpu.a >= 0x68) {
            return;
        }
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_StateOnWhenHitOrNear161D(void)
{
    StateOnWhenHitOrNear161D(cpu.x);
}

/*
 * @routine 101D:2CD StateOnWhenClose101D
 * @run 71:C2CD
 * WHAT IT DOES: Counts the state on once the player is within $70 along X and $30 along Y (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StateOnWhenClose101D(uint8_t slot)
{
    jsr_push(0xC2D0);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
    cmp(cpu.a, 0x70);
    if (cpu.a >= 0x70) {
        return;
    }
    jsr_push(0xC2D7);
    abi_ended(ended_of_distance(DistanceYToPlayer(slot)));
    jsr_pop();
    cmp(cpu.a, 0x30);
    if (cpu.a >= 0x30) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_StateOnWhenClose101D(void)
{
    StateOnWhenClose101D(cpu.x);
}

/*
 * @routine 0EBE:312 StopOnLandingThen0EBE_312
 * @run 71:CB12
 * WHAT IT DOES: Falls with height 4; on landing the state on with substate 0.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StopOnLandingThen0EBE_312(uint8_t slot)
{
    lda(0x04);
    jsr_push(0xCB17);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
    WR8(OBJ->substate[slot], 0);
}

void abi_StopOnLandingThen0EBE_312(void)
{
    StopOnLandingThen0EBE_312(cpu.x);
}

/*
 * @routine 117E:24B StopOnLandingThen117E_24B
 * @run 71:CA4B
 * WHAT IT DOES: Falls with height 16; on landing the animation restarted and the state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StopOnLandingThen117E_24B(uint8_t slot)
{
    lda(0x10);
    jsr_push(0xCA50);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    restart_animation(slot);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_StopOnLandingThen117E_24B(void)
{
    StopOnLandingThen117E_24B(cpu.x);
}

/*
 * @routine 161D:3D2 MoveAndCount161D
 * @run 71:C3D2
 * WHAT IT DOES: Moves; every fourth count of OBJ_WORK_D the substate on (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void MoveAndCount161D(uint8_t slot)
{
    uint8_t left;

    jsr_push(0xC3D5);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
    left = DEC8(OBJ->work_d[slot]);
    abi_nz(left);
    if (left != 0) {
        return;
    }
    lda(0x04);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_MoveAndCount161D(void)
{
    MoveAndCount161D(cpu.x);
}

/*
 * @routine 05CC:01E EachOfPatterns05CC_01E
 * @run 70:B81E
 * WHAT IT DOES: Runs the routine at $B82D, then the one at $B858 for X 0 to 26, X restored.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: X restored, N and Z from it; C set; else as the last call.
 */
void EachOfPatterns05CC_01E(void)
{
    each_of(0xB82D, 0xB822, 0xB858, 0xB826, 27);
}

void abi_EachOfPatterns05CC_01E(void)
{
    EachOfPatterns05CC_01E();
}

/*
 * @routine 072C:4E6 EachOfPatterns072C_4E6
 * @run 70:BCE6
 * WHAT IT DOES: Runs the routine at $BCF5, then the one at $BD20 for X 0 to 7, X restored.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: X restored, N and Z from it; C set; else as the last call.
 */
void EachOfPatterns072C_4E6(void)
{
    each_of(0xBCF5, 0xBCEA, 0xBD20, 0xBCEE, 8);
}

void abi_EachOfPatterns072C_4E6(void)
{
    EachOfPatterns072C_4E6();
}

/*
 * @routine 0B4D:164 EachOfPatterns0B4D_164
 * @run 71:C164
 * WHAT IT DOES: Runs the routine at $C173, then the one at $C19E for X 0 to 6, X restored.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: X restored, N and Z from it; C set; else as the last call.
 */
void EachOfPatterns0B4D_164(void)
{
    each_of(0xC173, 0xC168, 0xC19E, 0xC16C, 7);
}

void abi_EachOfPatterns0B4D_164(void)
{
    EachOfPatterns0B4D_164();
}
