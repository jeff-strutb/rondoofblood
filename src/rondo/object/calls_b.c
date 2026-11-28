/*
 * Small routines of stages $09 to $10 and the secret stages that call the kernel or the routine their overlay holds:
 * a record loaded and a column set, sounds, moves, frame lists.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"      /* axis_move_exit_shim */
#include "calls_b.h"

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

/* A tail JMP to it. */
static void load_record_tail(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
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

/* The record loaded, then one byte of one of the object's columns set. */
static void load_then_set(uint8_t slot, uint16_t column, uint8_t value, uint16_t ret)
{
    load_record(slot, ret);
    lda(value);
    WR8_IDX(column, slot, cpu.a);
}

/* The frame list's address into zTemp10-11, then a tail JMP to StepObjectFrames. */
static void step_list_tail(uint8_t slot, uint16_t list)
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

/*
 * @routine 10CD:331 LoadRecordTail10CD_331
 * @run 71:C331
 * WHAT IT DOES: Loads the object's record (secret stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadRecordTail10CD_331(uint8_t slot)
{
    load_record_tail(slot);
}

void abi_LoadRecordTail10CD_331(void)
{
    LoadRecordTail10CD_331(cpu.x);
}

/*
 * @routine 12DB:2A8 MoveY12DB_2A8
 * @run 70:B2A8
 * WHAT IT DOES: Moves the object along Y (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void MoveY12DB_2A8(uint8_t slot)
{
    jsr_push(0xB2AB);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
}

void abi_MoveY12DB_2A8(void)
{
    MoveY12DB_2A8(cpu.x);
}

/*
 * @routine 12DE:074 MoveX12DE_074
 * @run 71:C874
 * WHAT IT DOES: Moves the object along X (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void MoveX12DE_074(uint8_t slot)
{
    jsr_push(0xC877);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
}

void abi_MoveX12DE_074(void)
{
    MoveX12DE_074(cpu.x);
}

/*
 * @routine 101F:66E ClearAndFlag101F
 * @run 71:D66E
 * WHAT IT DOES: Sets zField92 and clears the object (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void ClearAndFlag101F(uint8_t slot)
{
    lda(1);
    WR8(ZP->field92, cpu.a);
    ClearObject(slot);
}

void abi_ClearAndFlag101F(void)
{
    ClearAndFlag101F(cpu.x);
}

/*
 * @routine 05CB:6F2 SoundThenSubstate05CB_6F2
 * @run 70:B6F2
 * WHAT IT DOES: Sound $87, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void SoundThenSubstate05CB_6F2(uint8_t slot)
{
    sound(0x87, 0xB6F7);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundThenSubstate05CB_6F2(void)
{
    SoundThenSubstate05CB_6F2(cpu.x);
}

/*
 * @routine 16CC:1FB SoundThenSubstate16CC_1FB
 * @run 70:B9FB
 * WHAT IT DOES: Sound $2A, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void SoundThenSubstate16CC_1FB(uint8_t slot)
{
    sound(0x2A, 0xBA00);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundThenSubstate16CC_1FB(void)
{
    SoundThenSubstate16CC_1FB(cpu.x);
}

/*
 * @routine 0EBF:7B5 Sound23ThenDebris0EBF
 * @run 71:D7B5
 * WHAT IT DOES: Sound $23, then on to the routine at $A90B with A 0 (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void Sound23ThenDebris0EBF(uint8_t slot)
{
    sound(0x23, 0xD7BA);
    cpu.a = 0;
    run_routine(0xA90B);
}

void abi_Sound23ThenDebris0EBF(void)
{
    Sound23ThenDebris0EBF(cpu.x);
}

/*
 * @routine 0EC0:3C3 LoadThenSet0EC0_3C3
 * @run 71:DBC3
 * WHAT IT DOES: Loads the object's record, then facing left.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadThenSet0EC0_3C3(uint8_t slot)
{
    load_then_set(slot, COL(OBJ->facing_left), 0x01, 0xDBC6);
}

void abi_LoadThenSet0EC0_3C3(void)
{
    LoadThenSet0EC0_3C3(cpu.x);
}

/*
 * @routine 101C:307 LoadThenSet101C_307
 * @run 70:BB07
 * WHAT IT DOES: Loads the object's record, then OBJ_WORK_F 64.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadThenSet101C_307(uint8_t slot)
{
    load_then_set(slot, COL(OBJ->work_f), 0x40, 0xBB0A);
}

void abi_LoadThenSet101C_307(void)
{
    LoadThenSet101C_307(cpu.x);
}

/*
 * @routine 161B:2FA LoadThenSet161B_2FA
 * @run 70:B2FA
 * WHAT IT DOES: Loads the object's record, then the hit box's Y offset $23.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadThenSet161B_2FA(uint8_t slot)
{
    load_then_set(slot, COL(OBJ->hitbox_dy), 0x23, 0xB2FD);
}

void abi_LoadThenSet161B_2FA(void)
{
    LoadThenSet161B_2FA(cpu.x);
}

/*
 * @routine 161B:37F LoadThenSet161B_37F
 * @run 70:B37F
 * WHAT IT DOES: Loads the object's record, then OBJ_WORK_K 128.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadThenSet161B_37F(uint8_t slot)
{
    load_then_set(slot, COL(OBJ->work_k), 0x80, 0xB382);
}

void abi_LoadThenSet161B_37F(void)
{
    LoadThenSet161B_37F(cpu.x);
}

/*
 * @routine 161B:63D LoadThenSet161B_63D
 * @run 70:B63D
 * WHAT IT DOES: Loads the object's record, then OBJ_WORK_H 64.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadThenSet161B_63D(uint8_t slot)
{
    load_then_set(slot, COL(OBJ->work_h), 0x40, 0xB640);
}

void abi_LoadThenSet161B_63D(void)
{
    LoadThenSet161B_63D(cpu.x);
}

/*
 * @routine 161D:080 LoadThenSet161D_080
 * @run 71:C080
 * WHAT IT DOES: Loads the object's record, then OBJ_WORK_K 240.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadThenSet161D_080(uint8_t slot)
{
    load_then_set(slot, COL(OBJ->work_k), 0xF0, 0xC083);
}

void abi_LoadThenSet161D_080(void)
{
    LoadThenSet161D_080(cpu.x);
}

/*
 * @routine 101D:2C3 LoadThenFaceByArg101D
 * @run 71:C2C3
 * WHAT IT DOES: Loads the record; the start argument is the facing (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadThenFaceByArg101D(uint8_t slot)
{
    load_record(slot, 0xC2C6);
    lda(RD8(OBJ->start_arg[slot]));
    WR8(OBJ->facing_left[slot], cpu.a);
}

void abi_LoadThenFaceByArg101D(void)
{
    LoadThenFaceByArg101D(cpu.x);
}

/*
 * @routine 12DA:736 LoadThenDraw5_12DA
 * @run 70:AF36
 * WHAT IT DOES: Loads the record and draws tile script 5 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadThenDraw5_12DA(uint8_t slot)
{
    TileRun run;

    load_record(slot, 0xAF39);
    lda(0x05);
    cpu.y = 0;
    jsr_push(0xAF3F);
    run = DrawTileScript(0x05, 0);
    abi_ended(ended_of_tile_run(run, 0x05));
    jsr_pop();
}

void abi_LoadThenDraw5_12DA(void)
{
    LoadThenDraw5_12DA(cpu.x);
}

/*
 * @routine 12DA:7B1 LoadClearFlagWait12DA
 * @run 70:AFB1
 * WHAT IT DOES: Loads the record, clears zField92, OBJ_WORK_D 160 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void LoadClearFlagWait12DA(uint8_t slot)
{
    load_record(slot, 0xAFB4);
    WR8(ZP->field92, 0);
    lda(0xA0);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_LoadClearFlagWait12DA(void)
{
    LoadClearFlagWait12DA(cpu.x);
}

/*
 * @routine 072C:6B5 FallAndGo072C
 * @run 70:BEB5
 * WHAT IT DOES: Gravity, the Y move, then the routine at $BE56 (secret stage 03's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void FallAndGo072C(uint8_t slot)
{
    jsr_push(0xBEB8);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
    jsr_push(0xBEBB);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
    call_routine(0xBEBE, 0xBE56);
}

void abi_FallAndGo072C(void)
{
    FallAndGo072C(cpu.x);
}

/*
 * @routine 161B:319 Fall161B
 * @run 70:B319
 * WHAT IT DOES: Gravity, then the Y move (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void Fall161B(uint8_t slot)
{
    jsr_push(0xB31C);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocityY(slot));
}

void abi_Fall161B(void)
{
    Fall161B(cpu.x);
}

/*
 * @routine 088B:22D State9AndTurnBack088B
 * @run 70:B22D
 * WHAT IT DOES: State 9 with both speeds reversed (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void State9AndTurnBack088B(uint8_t slot)
{
    lda(0x09);
    WR8(OBJ->state[slot], cpu.a);
    jsr_push(0xB235);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    negate_exit_shim(NegateVelocityY(slot));
}

void abi_State9AndTurnBack088B(void)
{
    State9AndTurnBack088B(cpu.x);
}

/*
 * @routine 117F:073 StateOnWhenNear117F
 * @run 71:D073
 * WHAT IT DOES: Counts the state on once the player is within 32 dots along X (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void StateOnWhenNear117F(uint8_t slot)
{
    jsr_push(0xD076);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
    cmp(cpu.a, 0x20);
    if (cpu.a < 0x20) {
        abi_nz(INC8(OBJ->state[slot]));
    }
}

void abi_StateOnWhenNear117F(void)
{
    StateOnWhenNear117F(cpu.x);
}

/*
 * @routine 161F:03A StepList161F_03A
 * @run 71:D03A
 * WHAT IT DOES: Plays the frame list at $D045 (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void StepList161F_03A(uint8_t slot)
{
    step_list_tail(slot, 0xD045);
}

void abi_StepList161F_03A(void)
{
    StepList161F_03A(cpu.x);
}

/*
 * @routine 161F:052 StepList161F_052
 * @run 71:D052
 * WHAT IT DOES: Plays the frame list at $D05D (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or load left them.
 */
void StepList161F_052(uint8_t slot)
{
    step_list_tail(slot, 0xD05D);
}

void abi_StepList161F_052(void)
{
    StepList161F_052(cpu.x);
}

/*
 * @routine 117C:76E ProbeHeight12Tail117C
 * @run 70:BF6E
 * WHAT IT DOES: Probe height 12, then on to the routine at $AB3A (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: as the last call or load left them.
 */
void ProbeHeight12Tail117C(void)
{
    lda(0x0C);
    WR8(ZP->product.mid, cpu.a);
    run_routine(0xAB3A);
}

void abi_ProbeHeight12Tail117C(void)
{
    ProbeHeight12Tail117C();
}

/*
 * @routine 117E:2E7 ProbeHeightCall117E_2E7
 * @run 71:CAE7
 * WHAT IT DOES: Probe height 16, then the routine at $AB3A (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: as the last call or load left them.
 */
void ProbeHeightCall117E_2E7(void)
{
    lda(0x10);
    WR8(ZP->product.mid, cpu.a);
    call_routine(0xCAEE, 0xAB3A);
}

void abi_ProbeHeightCall117E_2E7(void)
{
    ProbeHeightCall117E_2E7();
}

/*
 * @routine 117F:46F ProbeHeightCall117F_46F
 * @run 71:D46F
 * WHAT IT DOES: Probe height 18, then the routine at $AB3A (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: Exit: as the last call or load left them.
 */
void ProbeHeightCall117F_46F(void)
{
    lda(0x12);
    WR8(ZP->product.mid, cpu.a);
    call_routine(0xD476, 0xAB3A);
}

void abi_ProbeHeightCall117F_46F(void)
{
    ProbeHeightCall117F_46F();
}
