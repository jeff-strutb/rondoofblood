/*
 * Further small calling routines of stages $09 to $10 and the secret stages.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "calls_b3.h"

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


/*
 * @routine 161E:3D9 RecordThenClear161E
 * @run 71:CBD9
 * WHAT IT DOES: A first call only counts OBJ_WORK_E; the next records the object's event and clears it (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RecordThenClear161E(uint8_t slot)
{
    uint8_t stored, event, byte;
    bool marked;

    lda(RD8(OBJ->work_e[slot]));
    if (cpu.a == 0) {
        abi_nz(INC8(OBJ->work_e[slot]));
        return;
    }
    jsr_push(0xCBE5);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    abi_ended(l8_of_event(marked, stored, event, byte));
    jsr_pop();
    ClearObject(slot);
}

void abi_RecordThenClear161E(void)
{
    RecordThenClear161E(cpu.x);
}

/*
 * @routine 088B:630 WhenSlot20Ready088B
 * @run 70:B630
 * WHAT IT DOES: When the routine at $AF65 answers Z for slot $14: OBJ_WORK_K 128, substate on (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WhenSlot20Ready088B(uint8_t slot)
{
    cpu.y = 0x14;
    abi_nz(cpu.y);
    call_routine(0xB635, 0xAF65);
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0x80);
    WR8(OBJ->work_k[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_WhenSlot20Ready088B(void)
{
    WhenSlot20Ready088B(cpu.x);
}

/*
 * @routine 088A:750 PushByAnswer088A
 * @run 70:AF50
 * WHAT IT DOES: When the routine at $AF65 answers C for Y 0, moves along X at the table's speed for OBJ_WORK_H (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PushByAnswer088A(uint8_t slot)
{
    cpu.y = 0;
    call_routine(0xAF54, 0xAF65);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    cpu.y = RD8(OBJ->work_h[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xAF63 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    jsr_push(0xAF62);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
}

void abi_PushByAnswer088A(void)
{
    PushByAnswer088A(cpu.x);
}

/*
 * @routine 088B:031 ToState5WhenSlot2Ready088B
 * @run 70:B031
 * WHAT IT DOES: When the routine at $AF65 answers Z for slot 2: the animation restarted, state 5 (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ToState5WhenSlot2Ready088B(uint8_t slot)
{
    cpu.y = 0x02;
    abi_nz(cpu.y);
    call_routine(0xB036, 0xAF65);
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    restart_animation(slot);
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_ToState5WhenSlot2Ready088B(void)
{
    ToState5WhenSlot2Ready088B(cpu.x);
}

/*
 * @routine 088B:5FC StartWhenSlot18Ready088B
 * @run 70:B5FC
 * WHAT IT DOES: When the routine at $AF65 answers Z for slot $12: OBJ_WORK_L 1, OBJ_WORK_K 0, substate on (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartWhenSlot18Ready088B(uint8_t slot)
{
    cpu.y = 0x12;
    abi_nz(cpu.y);
    call_routine(0xB601, 0xAF65);
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0x01);
    WR8(OBJ->work_l[slot], cpu.a);
    WR8(OBJ->work_k[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_StartWhenSlot18Ready088B(void)
{
    StartWhenSlot18Ready088B(cpu.x);
}

/*
 * @routine 0EBD:0A7 ClearFirstEmptySlot0EBD
 * @run 71:C0A7
 * WHAT IT DOES: Clears the first empty slot from $15 to $1F, leaving it in Y with C set; C clear with none (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: as the last call, compare or load left them.
 */
void ClearFirstEmptySlot0EBD(void)
{
    cpu.y = 0x15;
    abi_nz(cpu.y);
    do {
        lda(RD8(OBJ->kind[cpu.y]));
        if (cpu.a == 0) {
            clear_slot_in_y(0xC0B1);
            abi_flag(P_C, true);
            return;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x20);
    } while (cpu.y != 0x20);
    abi_flag(P_C, false);
}

void abi_ClearFirstEmptySlot0EBD(void)
{
    ClearFirstEmptySlot0EBD();
}

/*
 * @routine 101E:13E HitStateWhenNear101E
 * @run 71:C93E
 * WHAT IT DOES: Rotates A, runs the routine at $C9EB; unless it leaves C set, an object within $50 dots of the player goes to state 3 (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void HitStateWhenNear101E(uint8_t slot)
{
    {
        uint8_t in = (cpu.p & P_C) != 0 ? 1 : 0;

        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)((cpu.a << 1) | in));
    }
    call_routine(0xC942, 0xC9EB);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    jsr_push(0xC947);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
    cmp(cpu.a, 0x50);
    if (cpu.a >= 0x50) {
        return;
    }
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_HitStateWhenNear101E(void)
{
    HitStateWhenNear101E(cpu.x);
}

/*
 * @routine 1020:0AA StepListByWorkF1020
 * @run 71:D8AA
 * WHAT IT DOES: Plays the frame list the table at $DA62 gives for OBJ_WORK_F (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StepListByWorkF1020(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    lda(RD8(OBJ->work_f[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xDA62 + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xDA63 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    jsr_push(0xD8BC);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
}

void abi_StepListByWorkF1020(void)
{
    StepListByWorkF1020(cpu.x);
}

/*
 * @routine 10CC:468 ListThenSubstate1_10CC
 * @run 70:BC68
 * WHAT IT DOES: Plays the frame list at $BC7C; when it ends, substate 1 (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenSubstate1_10CC(uint8_t slot)
{
    step_list(slot, 0xBC7C, true, 0xBC73);
    if (cpu.a != 0) {
        return;
    }
    lda(0x01);
    WR8(OBJ->substate[slot], cpu.a);
}

void abi_ListThenSubstate1_10CC(void)
{
    ListThenSubstate1_10CC(cpu.x);
}

/*
 * @routine 13FB:493 SpawnOddFramesAnswer13FB
 * @run 70:B493
 * WHAT IT DOES: Every other frame, starts a kind $3E object at this place in a free slot; C says whether (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnOddFramesAnswer13FB(uint8_t slot)
{
    uint8_t found_a, to;
    bool found;

    lda(RD8(ZP->wave_counter));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a == 0) {
        jsr_push(0xB49C);
        to = FindFreeSlotFrom30(&found_a, &found);
        find_slot_exit_shim(to, found_a, found);
        jsr_pop();
        if (found) {
            start_at_place(0x3E, 0xB4A3);
            abi_flag(P_C, true);
            return;
        }
    }
    abi_flag(P_C, false);
}

void abi_SpawnOddFramesAnswer13FB(void)
{
    SpawnOddFramesAnswer13FB(cpu.x);
}

/*
 * @routine 161D:630 LoadAndStartNext161D
 * @run 71:C630
 * WHAT IT DOES: Loads the record, OBJ_WORK_H $C0, and starts a kind $4E object at this place in the next slot (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAndStartNext161D(uint8_t slot)
{
    load_record(slot, 0xC633);
    lda(0xC0);
    WR8(OBJ->work_h[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xC63E);
    start_at_place(0x4E, 0xC643);
}

void abi_LoadAndStartNext161D(void)
{
    LoadAndStartNext161D(cpu.x);
}

/*
 * @routine 16CC:3E0 ListThenClearOnCount16CC
 * @run 70:BBE0
 * WHAT IT DOES: Plays the frame list at $BBD7 and counts OBJ_WORK_D; at zero the object is cleared (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenClearOnCount16CC(uint8_t slot)
{
    uint8_t left;

    step_list(slot, 0xBBD7 + 0, true, 0xBBEB);
    left = DEC8(OBJ->work_d[slot]);
    abi_nz(left);
    if (left != 0) {
        return;
    }
    ClearObject(slot);
}

void abi_ListThenClearOnCount16CC(void)
{
    ListThenClearOnCount16CC(cpu.x);
}

/*
 * @routine 12DC:02E StartKind2AIn32To43_12DC
 * @run 70:B82E
 * WHAT IT DOES: Starts kind $2A objects at this place in slots $20 to $2B (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartKind2AIn32To43_12DC(uint8_t slot)
{
    cpu.y = 0x20;
    abi_nz(cpu.y);
    do {
        clear_slot_in_y(0xB833);
        start_at_place(0x2A, 0xB838);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x2C);
    } while (cpu.y < 0x2C);
}

void abi_StartKind2AIn32To43_12DC(void)
{
    StartKind2AIn32To43_12DC(cpu.x);
}

/*
 * @routine 0B4C:7FF LoadFrame4OrFall0B4C
 * @run 70:BFFF
 * WHAT IT DOES: In state 0 loads the record with frame 4; later falls (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFrame4OrFall0B4C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC007);
        lda(0x04);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    jsr_push(0xC010);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocityY(slot));
}

void abi_LoadFrame4OrFall0B4C(void)
{
    LoadFrame4OrFall0B4C(cpu.x);
}

/*
 * @routine 0B4D:0E5 ListFallMove0B4D
 * @run 71:C0E5
 * WHAT IT DOES: Plays the frame list at $C0F6, falls and moves (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListFallMove0B4D(uint8_t slot)
{
    step_list(slot, 0xC0F6, false, 0xC0F0);
    jsr_push(0xC0F3);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_ListFallMove0B4D(void)
{
    ListFallMove0B4D(cpu.x);
}

/*
 * @routine 117C:3D8 LandOnRowThenDebris117C
 * @run 70:BBD8
 * WHAT IT DOES: When the object reaches the row at $Dx: sound $23, then on to the routine at $A90B with A 0 (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandOnRowThenDebris117C(uint8_t slot)
{
    lda(RD8(OBJ->y_lo[slot]));
    lda((uint8_t)(cpu.a & 0xF0));
    cmp(cpu.a, 0xD0);
    if (cpu.a != 0xD0) {
        return;
    }
    sound(0x23, 0xBBE6);
    cpu.a = 0;
    run_routine(0xA90B);
}

void abi_LandOnRowThenDebris117C(void)
{
    LandOnRowThenDebris117C(cpu.x);
}
