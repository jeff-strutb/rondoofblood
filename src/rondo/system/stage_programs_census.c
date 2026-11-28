/*
 * Routines of the stage programs no read the game makes loads, at sectors 026D, 031D, 043D, 064D, 10DD, 122D, 1A2D,
 * 1B8D and 1CED: other builds of the stage overlay at 024D. Each routine here is a loaded stage routine with its own
 * program's operands; its C calls the loaded routine's body with them.
 */
#include <stddef.h>
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_regs.h"
#include "front_end.h"
#include "stage_programs_census.h"
#include "stage_gauge.h"

/* The routines below serve the same bytes in several programs, so a call or jump into the overlay's own window goes
 * to the C of the routine loaded there (routine_at), not to one program's function. */

/* The VRAM address setter loaded at run, with A as the caller holds it (N and Z from it), X the low byte and Y the
 * high; the frame, if any, is the caller's. */
static void resident_set_vram_shim(uint16_t run, uint8_t a, uint8_t lo, uint8_t hi)
{
    cpu.a = a;
    abi_nz(a);
    cpu.x = lo;
    cpu.y = hi;
    routine_at(run)();
}

/* For SetHealthBarAddress's JSR: A the high byte (TAY after the ADC). */
static void resident_bar_vram_933b(uint8_t lo, uint8_t hi)
{
    resident_set_vram_shim(0x933B, hi, lo, hi);
}

/* For SetStageTileVramAddress's JMP: A the low byte (TAX from it). */
static void resident_tile_vram_933b(uint8_t lo, uint8_t hi)
{
    resident_set_vram_shim(0x933B, lo, lo, hi);
}

/* For SetHealthBarAddress's JSR: A the high byte (TAY after the ADC). */
static void resident_bar_vram_961c(uint8_t lo, uint8_t hi)
{
    resident_set_vram_shim(0x961C, hi, lo, hi);
}

/* For SetStageTileVramAddress's JMP: A the low byte (TAX from it). */
static void resident_tile_vram_961c(uint8_t lo, uint8_t hi)
{
    resident_set_vram_shim(0x961C, lo, lo, hi);
}

/* The row spawn loaded at run, as a SpawnRow: X the row; the slot comes back in Y and the Y low byte in A. */
static SpawnedObject resident_spawn_shim(uint16_t run, uint8_t row)
{
    SpawnedObject out;

    cpu.x = row;
    abi_nz(row);
    routine_at(run)();
    out.slot = cpu.y;
    out.y_lo = cpu.a;
    return out;
}

static SpawnedObject resident_spawn_936e(uint8_t row)
{
    return resident_spawn_shim(0x936E, row);
}

static SpawnedObject resident_spawn_964f(uint8_t row)
{
    return resident_spawn_shim(0x964F, row);
}

/* LDA, CLC, ADC, TAM, INC A, TAM as they leave the registers, then the JMP to $A000. */
static void resident_jump_shim(Sum bank)
{
    cpu.a = (uint8_t)(bank.value + 1);
    abi_nz(cpu.a);
    abi_flag(P_C, bank.carry);
    abi_flag(P_V, bank.overflow);
    run_routine(0xA000);
}

/* StartStageLists with a program's own tables, list expansion, script slot 7 and return addresses. */
static void list_channel_registers_shim(uint8_t a, uint8_t x);

static StageStartEnd start_lists(uint16_t lists_at, uint16_t scripts_at, StageListEnd (*expand)(void))
{
    StageStartEnd end;
    StageListEnd lists;
    uint8_t section;
    uint8_t list;
    uint8_t script;
    uint8_t x;

    jsr_push(0x886F);
    ClearListChannels();
    jsr_pop();
    x = RD8(ZP->field82);
    list = rd8((uint16_t)(lists_at + x));
    jsr_push(0x8877);
    list_channel_registers_shim(StartListChannel0Kind2(list), 0);
    jsr_pop();
    jsr_push(0x887A);
    lists = expand();
    lists_exit_shim(lists);
    jsr_pop();
    section = RD8(ZP->field82);
    end.index = add_bytes((uint8_t)(section << 1), RD8(ZP->character), false);
    script = rd8((uint16_t)(scripts_at + end.index.value));
    end.script = StartScriptSlotKind1(script, 7);
    return end;
}

/* The registers the list channel start leaves, which the list expansion's block move pushes. */
static void list_channel_registers_shim(uint8_t a, uint8_t x)
{
    cpu.a = a;
    cpu.x = x;
}

static void start_lists_exit_shim(StageStartEnd end)
{
    script_slot_exit_shim(end.script);
    cpu.x = 7;
    abi_flag(P_C, end.index.carry);
    abi_flag(P_V, end.index.overflow);
}

/* A row of a program's five spawn tables (slot, frame, sprite set, X low, Y low): SpawnStageObject's body. */
static SpawnedObject spawn_row(uint8_t row, const uint16_t tables[5], uint16_t ret)
{
    SpawnedObject out;
    uint8_t byte;

    out.slot = rd8((uint16_t)(tables[0] + row));
    jsr_push(ret);
    ClearObjectInY(out.slot);
    jsr_pop();
    byte = rd8((uint16_t)(tables[1] + row));
    WR8(OBJ->frame[out.slot], byte);
    byte = rd8((uint16_t)(tables[2] + row));
    WR8(OBJ->f2a10[out.slot], byte);
    byte = rd8((uint16_t)(tables[3] + row));
    WR8(OBJ->x_lo[out.slot], byte);
    byte = rd8((uint16_t)(tables[4] + row));
    WR8(OBJ->y_lo[out.slot], byte);
    out.y_lo = byte;
    return out;
}

/* The body of StageSectionNext (stage_flow.c) with a program's own section table. */
static StageRegs section_next(StageRegs r, uint16_t table)
{
    INC8(ZP->field82);
    r.y = RD8(ZP->field82);
    r = regs_loaded(r, rd8((uint16_t)(table + r.y)));
    WR8(ZP->field83, r.a);
    return r;
}

/* MaskStageRowCopy and MaskStageRow with a program's own offset table; from is 2 or 0. */
static MaskExit mask_row(uint8_t x, uint8_t y, uint16_t offsets, uint8_t from)
{
    MaskExit out = { 0, { 0, false, false } };

    WR8(ZP->temp08, x);
    push8(x);
    push8(y);
    for (int step = 7; step >= 0; step--) {
        uint8_t offset = rd8((uint16_t)(offsets + step));
        uint8_t base = RD8(ZP->temp08);
        uint8_t cell;
        uint8_t mask;

        out.last = add_bytes(offset, base, false);
        cell = RD8(STAGEWORK->mask_buffer[(uint16_t)(out.last.value + from)]);
        mask = RD8(ZP->temp00);
        out.a = (uint8_t)(cell & mask);
        WR8(STAGEWORK->mask_buffer[out.last.value], out.a);
    }
    pull8();
    pull8();
    return out;
}

static void mask_row_exit_shim(MaskExit out, uint8_t x)
{
    cpu.a = out.a;
    abi_nz(x);
    abi_flag(P_C, out.last.carry);
    abi_flag(P_V, out.last.overflow);
}

/* The body of SetHealthBarAddress (stage_tick.c) with a copy's address setter and return address. */
static StageExit health_bar_address(void (*set)(uint8_t, uint8_t), uint16_t ret)
{
    StageExit end = { 0, 0, 0, false, false, 0, 0 };
    uint8_t value;
    uint8_t low;
    uint8_t high;
    Sum lo_sum;
    Sum hi_sum;

    value = RD8(ZP->field99);
    if (value == 0) {
        return end;
    }
    value = (uint8_t)(0x5E - RD8(ZP->field99));
    push8(value);
    WR8(ZP->temp00, (uint8_t)(value & 0x0F));
    value = (uint8_t)(pull8() & 0xF0);
    WR8(ZP->temp01, 0);
    for (int bit = 0; bit < 3; bit++) {
        bool carry = (value & 0x80) != 0;

        value = (uint8_t)(value << 1);
        ROL8(ZP->temp01, carry);
    }
    low = RD8(ZP->temp00);
    lo_sum = add_bytes(value, low, false);
    high = RD8(ZP->temp01);
    hi_sum = add_bytes(high, 0x65, lo_sum.carry);
    jsr_push(ret);
    set(lo_sum.value, hi_sum.value);
    jsr_pop();
    end.a = 2;
    end.nz = 2;
    end.x = lo_sum.value;
    end.y = hi_sum.value;
    end.c = hi_sum.carry;
    end.v = hi_sum.overflow;
    end.set = EXIT_X | EXIT_Y | EXIT_C | EXIT_V;
    return end;
}

/*
 * @routine 026D:0B4 ResetStageVariables026D
 * @run 6B:80B4
 * WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $D541.
 * HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D5 and N set. X, Y, C, V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm ResetStageVariables026D(void)
{
    return reset_stage_variables(0x41, 0xD5);
}

void abi_ResetStageVariables026D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables026D()));
}

/*
 * @routine 026E:03D ExpandStageVramRuns026E
 * @run 6B:883D
 * WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9C52.
 * HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRunsEnd ExpandStageVramRuns026E(void)
{
    return expand_vram_runs(0x9C52);
}

void abi_ExpandStageVramRuns026E(void)
{
    runs_exit_shim(ExpandStageVramRuns026E());
}

/*
 * @routine 026F:235 ExpandStageLists026F
 * @run 6B:9235
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9C4A.
 * HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageListEnd ExpandStageLists026F(void)
{
    return expand_stage_lists(0x9C4A);
}

void abi_ExpandStageLists026F(void)
{
    lists_exit_shim(ExpandStageLists026F());
}

/*
 * @routine 031D:0B4 ResetStageVariables031D
 * @run 6B:80B4
 * WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $BF1E.
 * HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $BF and N set. X, Y, C, V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm ResetStageVariables031D(void)
{
    return reset_stage_variables(0x1E, 0xBF);
}

void abi_ResetStageVariables031D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables031D()));
}

/*
 * @routine 031E:03D ExpandStageVramRuns031E
 * @run 6B:883D
 * WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9E19.
 * HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRunsEnd ExpandStageVramRuns031E(void)
{
    return expand_vram_runs(0x9E19);
}

void abi_ExpandStageVramRuns031E(void)
{
    runs_exit_shim(ExpandStageVramRuns031E());
}

/*
 * @routine 031F:229 ExpandStageLists031F
 * @run 6B:9229
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9E11.
 * HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageListEnd ExpandStageLists031F(void)
{
    return expand_stage_lists(0x9E11);
}

void abi_ExpandStageLists031F(void)
{
    lists_exit_shim(ExpandStageLists031F());
}

/*
 * @routine 043D:0B4 ResetStageVariables043D
 * @run 6B:80B4
 * WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $C340.
 * HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C3 and N set. X, Y, C, V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm ResetStageVariables043D(void)
{
    return reset_stage_variables(0x40, 0xC3);
}

void abi_ResetStageVariables043D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables043D()));
}

/*
 * @routine 043E:03D ExpandStageVramRuns043E
 * @run 6B:883D
 * WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9EDD.
 * HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRunsEnd ExpandStageVramRuns043E(void)
{
    return expand_vram_runs(0x9EDD);
}

void abi_ExpandStageVramRuns043E(void)
{
    runs_exit_shim(ExpandStageVramRuns043E());
}

/*
 * @routine 043F:229 ExpandStageLists043F
 * @run 6B:9229
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9ED5.
 * HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageListEnd ExpandStageLists043F(void)
{
    return expand_stage_lists(0x9ED5);
}

void abi_ExpandStageLists043F(void)
{
    lists_exit_shim(ExpandStageLists043F());
}

/*
 * @routine 064D:0B4 ResetStageVariables064D
 * @run 6B:80B4
 * WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $DC6A.
 * HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DC and N set. X, Y, C, V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm ResetStageVariables064D(void)
{
    return reset_stage_variables(0x6A, 0xDC);
}

void abi_ResetStageVariables064D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables064D()));
}

/*
 * @routine 064E:03D ExpandStageVramRuns064E
 * @run 6B:883D
 * WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9D01.
 * HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRunsEnd ExpandStageVramRuns064E(void)
{
    return expand_vram_runs(0x9D01);
}

void abi_ExpandStageVramRuns064E(void)
{
    runs_exit_shim(ExpandStageVramRuns064E());
}

/*
 * @routine 064F:235 ExpandStageLists064F
 * @run 6B:9235
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9CFB.
 * HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageListEnd ExpandStageLists064F(void)
{
    return expand_stage_lists(0x9CFB);
}

void abi_ExpandStageLists064F(void)
{
    lists_exit_shim(ExpandStageLists064F());
}

/*
 * @routine 10DD:0B4 ResetStageVariables10DD
 * @run 6B:80B4
 * WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $C6F1.
 * HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C6 and N set. X, Y, C, V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm ResetStageVariables10DD(void)
{
    return reset_stage_variables(0xF1, 0xC6);
}

void abi_ResetStageVariables10DD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables10DD()));
}

/*
 * @routine 10DE:03D ExpandStageVramRuns10DE
 * @run 6B:883D
 * WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9ED7.
 * HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRunsEnd ExpandStageVramRuns10DE(void)
{
    return expand_vram_runs(0x9ED7);
}

void abi_ExpandStageVramRuns10DE(void)
{
    runs_exit_shim(ExpandStageVramRuns10DE());
}

/*
 * @routine 10DF:229 ExpandStageLists10DF
 * @run 6B:9229
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9ECF.
 * HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageListEnd ExpandStageLists10DF(void)
{
    return expand_stage_lists(0x9ECF);
}

void abi_ExpandStageLists10DF(void)
{
    lists_exit_shim(ExpandStageLists10DF());
}

/*
 * @routine 122D:0B4 ResetStageVariables122D
 * @run 6B:80B4
 * WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $D712.
 * HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D7 and N set. X, Y, C, V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm ResetStageVariables122D(void)
{
    return reset_stage_variables(0x12, 0xD7);
}

void abi_ResetStageVariables122D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables122D()));
}

/*
 * @routine 122E:03D ExpandStageVramRuns122E
 * @run 6B:883D
 * WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9CD0.
 * HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRunsEnd ExpandStageVramRuns122E(void)
{
    return expand_vram_runs(0x9CD0);
}

void abi_ExpandStageVramRuns122E(void)
{
    runs_exit_shim(ExpandStageVramRuns122E());
}

/*
 * @routine 122F:235 ExpandStageLists122F
 * @run 6B:9235
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9CCA.
 * HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageListEnd ExpandStageLists122F(void)
{
    return expand_stage_lists(0x9CCA);
}

void abi_ExpandStageLists122F(void)
{
    lists_exit_shim(ExpandStageLists122F());
}

/*
 * @routine 1A2D:0B4 ResetStageVariables1A2D
 * @run 6B:80B4
 * WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $DCA3.
 * HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DC and N set. X, Y, C, V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm ResetStageVariables1A2D(void)
{
    return reset_stage_variables(0xA3, 0xDC);
}

void abi_ResetStageVariables1A2D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables1A2D()));
}

/*
 * @routine 1A2E:03D ExpandStageVramRuns1A2E
 * @run 6B:883D
 * WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9CCD.
 * HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRunsEnd ExpandStageVramRuns1A2E(void)
{
    return expand_vram_runs(0x9CCD);
}

void abi_ExpandStageVramRuns1A2E(void)
{
    runs_exit_shim(ExpandStageVramRuns1A2E());
}

/*
 * @routine 1A2F:235 ExpandStageLists1A2F
 * @run 6B:9235
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9CC5.
 * HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageListEnd ExpandStageLists1A2F(void)
{
    return expand_stage_lists(0x9CC5);
}

void abi_ExpandStageLists1A2F(void)
{
    lists_exit_shim(ExpandStageLists1A2F());
}

/*
 * @routine 1B8D:0B4 ResetStageVariables1B8D
 * @run 6B:80B4
 * WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $D9C4.
 * HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D9 and N set. X, Y, C, V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm ResetStageVariables1B8D(void)
{
    return reset_stage_variables(0xC4, 0xD9);
}

void abi_ResetStageVariables1B8D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables1B8D()));
}

/*
 * @routine 1B8E:03D ExpandStageVramRuns1B8E
 * @run 6B:883D
 * WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9D15.
 * HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRunsEnd ExpandStageVramRuns1B8E(void)
{
    return expand_vram_runs(0x9D15);
}

void abi_ExpandStageVramRuns1B8E(void)
{
    runs_exit_shim(ExpandStageVramRuns1B8E());
}

/*
 * @routine 1B8F:235 ExpandStageLists1B8F
 * @run 6B:9235
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9D0D.
 * HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageListEnd ExpandStageLists1B8F(void)
{
    return expand_stage_lists(0x9D0D);
}

void abi_ExpandStageLists1B8F(void)
{
    lists_exit_shim(ExpandStageLists1B8F());
}

/*
 * @routine 1CED:0B4 ResetStageVariables1CED
 * @run 6B:80B4
 * WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $DC06.
 * HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DC and N set. X, Y, C, V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm ResetStageVariables1CED(void)
{
    return reset_stage_variables(0x06, 0xDC);
}

void abi_ResetStageVariables1CED(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables1CED()));
}

/*
 * @routine 1CEE:03D ExpandStageVramRuns1CEE
 * @run 6B:883D
 * WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9CAB.
 * HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRunsEnd ExpandStageVramRuns1CEE(void)
{
    return expand_vram_runs(0x9CAB);
}

void abi_ExpandStageVramRuns1CEE(void)
{
    runs_exit_shim(ExpandStageVramRuns1CEE());
}

/*
 * @routine 1CEF:235 ExpandStageLists1CEF
 * @run 6B:9235
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9C9F.
 * HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageListEnd ExpandStageLists1CEF(void)
{
    return expand_stage_lists(0x9C9F);
}

void abi_ExpandStageLists1CEF(void)
{
    lists_exit_shim(ExpandStageLists1CEF());
}

/*
 * @routine 026F:33B SetVramAddresses026F
 * @run 6B:933B
 * WHAT IT DOES: Points the VDC's read and write addresses at X and Y and selects the data register: this program's SetVramAddresses.
 * HOW: set_vram_addresses (map_draw.c), the body of 024F:37D, with this copy's return addresses.
 * WAITS: no.
 * Entry: X the address low, Y the high. Exit: A 2, N and Z from it. X, Y kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
void SetVramAddresses026F(uint8_t lo, uint8_t hi)
{
    set_vram_addresses(lo, hi, 0x933E, 0x9347);
}

void abi_SetVramAddresses026F(void)
{
    SetVramAddresses026F(cpu.x, cpu.y);
    cpu.a = 2;
    abi_nz(2);
}

/*
 * @routine 031F:61C SetVramAddresses031F
 * @run 6B:961C
 * WHAT IT DOES: Points the VDC's read and write addresses at X and Y and selects the data register: this program's SetVramAddresses.
 * HOW: set_vram_addresses (map_draw.c), the body of 024F:37D, with this copy's return addresses.
 * WAITS: no.
 * Entry: X the address low, Y the high. Exit: A 2, N and Z from it. X, Y kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
void SetVramAddresses031F(uint8_t lo, uint8_t hi)
{
    set_vram_addresses(lo, hi, 0x961F, 0x9628);
}

void abi_SetVramAddresses031F(void)
{
    SetVramAddresses031F(cpu.x, cpu.y);
    cpu.a = 2;
    abi_nz(2);
}

/*
 * @routine 026D:634 StageSectionNext026D
 * @run 6B:8634
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
 * HOW: As StageSectionNext (024D:66A), with this program's table.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSectionNext026D(StageRegs r)
{
    return section_next(r, 0x9C42);
}

void abi_StageSectionNext026D(void)
{
    stage_regs_load_shim(StageSectionNext026D(stage_regs_entry_shim()));
}

/*
 * @routine 031D:634 StageSectionNext031D
 * @run 6B:8634
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
 * HOW: As StageSectionNext (024D:66A), with this program's table.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSectionNext031D(StageRegs r)
{
    return section_next(r, 0x9E09);
}

void abi_StageSectionNext031D(void)
{
    stage_regs_load_shim(StageSectionNext031D(stage_regs_entry_shim()));
}

/*
 * @routine 043D:634 StageSectionNext043D
 * @run 6B:8634
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
 * HOW: As StageSectionNext (024D:66A), with this program's table.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSectionNext043D(StageRegs r)
{
    return section_next(r, 0x9ECD);
}

void abi_StageSectionNext043D(void)
{
    stage_regs_load_shim(StageSectionNext043D(stage_regs_entry_shim()));
}

/*
 * @routine 064D:634 StageSectionNext064D
 * @run 6B:8634
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
 * HOW: As StageSectionNext (024D:66A), with this program's table.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSectionNext064D(StageRegs r)
{
    return section_next(r, 0x9CF3);
}

void abi_StageSectionNext064D(void)
{
    stage_regs_load_shim(StageSectionNext064D(stage_regs_entry_shim()));
}

/*
 * @routine 10DD:634 StageSectionNext10DD
 * @run 6B:8634
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
 * HOW: As StageSectionNext (024D:66A), with this program's table.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSectionNext10DD(StageRegs r)
{
    return section_next(r, 0x9EC7);
}

void abi_StageSectionNext10DD(void)
{
    stage_regs_load_shim(StageSectionNext10DD(stage_regs_entry_shim()));
}

/*
 * @routine 122D:634 StageSectionNext122D
 * @run 6B:8634
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
 * HOW: As StageSectionNext (024D:66A), with this program's table.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSectionNext122D(StageRegs r)
{
    return section_next(r, 0x9CC7);
}

void abi_StageSectionNext122D(void)
{
    stage_regs_load_shim(StageSectionNext122D(stage_regs_entry_shim()));
}

/*
 * @routine 1A2D:634 StageSectionNext1A2D
 * @run 6B:8634
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
 * HOW: As StageSectionNext (024D:66A), with this program's table.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSectionNext1A2D(StageRegs r)
{
    return section_next(r, 0x9CBD);
}

void abi_StageSectionNext1A2D(void)
{
    stage_regs_load_shim(StageSectionNext1A2D(stage_regs_entry_shim()));
}

/*
 * @routine 1B8D:634 StageSectionNext1B8D
 * @run 6B:8634
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
 * HOW: As StageSectionNext (024D:66A), with this program's table.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSectionNext1B8D(StageRegs r)
{
    return section_next(r, 0x9D09);
}

void abi_StageSectionNext1B8D(void)
{
    stage_regs_load_shim(StageSectionNext1B8D(stage_regs_entry_shim()));
}

/*
 * @routine 1CED:634 StageSectionNext1CED
 * @run 6B:8634
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
 * HOW: As StageSectionNext (024D:66A), with this program's table.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSectionNext1CED(StageRegs r)
{
    return section_next(r, 0x9C99);
}

void abi_StageSectionNext1CED(void)
{
    stage_regs_load_shim(StageSectionNext1CED(stage_regs_entry_shim()));
}

/*
 * @routine 026F:0D1 MaskStageRowCopy026F
 * @run 6B:90D1
 * WHAT IT DOES: Masks one row of the stage's mask buffer from the cells two on: each of the row's eight cells ANDed with the mask in zTemp00.
 * HOW: As MaskStageRowCopy (024F:113), with this program's offset table.
 * WAITS: no.
 * Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last cell written, N and Z from X, C and V from the last offset add. X and Y kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
MaskExit MaskStageRowCopy026F(uint8_t x, uint8_t y)
{
    return mask_row(x, y, 0x9107, 2);
}

void abi_MaskStageRowCopy026F(void)
{
    uint8_t x = cpu.x;

    mask_row_exit_shim(MaskStageRowCopy026F(x, cpu.y), x);
}

/*
 * @routine 026F:0EC MaskStageRow026F
 * @run 6B:90EC
 * WHAT IT DOES: Masks one row of the stage's mask buffer from the cells in place: each of the row's eight cells ANDed with the mask in zTemp00.
 * HOW: As MaskStageRow (024F:12E), with this program's offset table.
 * WAITS: no.
 * Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last cell written, N and Z from X, C and V from the last offset add. X and Y kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
MaskExit MaskStageRow026F(uint8_t x, uint8_t y)
{
    return mask_row(x, y, 0x9107, 0);
}

void abi_MaskStageRow026F(void)
{
    uint8_t x = cpu.x;

    mask_row_exit_shim(MaskStageRow026F(x, cpu.y), x);
}

/*
 * @routine 031F:0C5 MaskStageRowCopy031F
 * @run 6B:90C5
 * WHAT IT DOES: Masks one row of the stage's mask buffer from the cells two on: each of the row's eight cells ANDed with the mask in zTemp00.
 * HOW: As MaskStageRowCopy (024F:113), with this program's offset table.
 * WAITS: no.
 * Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last cell written, N and Z from X, C and V from the last offset add. X and Y kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
MaskExit MaskStageRowCopy031F(uint8_t x, uint8_t y)
{
    return mask_row(x, y, 0x90FB, 2);
}

void abi_MaskStageRowCopy031F(void)
{
    uint8_t x = cpu.x;

    mask_row_exit_shim(MaskStageRowCopy031F(x, cpu.y), x);
}

/*
 * @routine 031F:0E0 MaskStageRow031F
 * @run 6B:90E0
 * WHAT IT DOES: Masks one row of the stage's mask buffer from the cells in place: each of the row's eight cells ANDed with the mask in zTemp00.
 * HOW: As MaskStageRow (024F:12E), with this program's offset table.
 * WAITS: no.
 * Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last cell written, N and Z from X, C and V from the last offset add. X and Y kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
MaskExit MaskStageRow031F(uint8_t x, uint8_t y)
{
    return mask_row(x, y, 0x90FB, 0);
}

void abi_MaskStageRow031F(void)
{
    uint8_t x = cpu.x;

    mask_row_exit_shim(MaskStageRow031F(x, cpu.y), x);
}

/*
 * @routine 026E:63D SetStageTileVramAddress026E
 * @run 6B:8E3D
 * WHAT IT DOES: Sets the VRAM address of the tile pair zTemp04 names, from this program's table of addresses.
 * HOW: As SetStageTileVramAddress (024E:687): zTemp04 less bit 0 indexes the table; a tail JMP to the setter loaded at that address (several programs hold these bytes).
 * WAITS: no.
 * Entry: zTemp04. Exit: A 2, N and Z from it, X the address low, Y the high.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
uint16_t SetStageTileVramAddress026E(void)
{
    uint8_t index = (uint8_t)(RD8(ZP->temp04) & 0xFE);
    uint8_t lo = rd8((uint16_t)(0x8E4C + index));
    uint8_t hi = rd8((uint16_t)(0x8E4D + index));

    resident_tile_vram_933b(lo, hi);    /* JMP */
    return (uint16_t)(lo | hi << 8);
}

void abi_SetStageTileVramAddress026E(void)
{
    (void)SetStageTileVramAddress026E();
}

/*
 * @routine 026E:3F9 SetHealthBarAddress026E
 * @run 6B:8BF9
 * WHAT IT DOES: While zField99 is not 0, sets the VRAM address of the bar at the cell $5E less it counts.
 * HOW: As SetHealthBarAddress (024E:443), calling the setter loaded at its JSR's address (several programs hold these bytes).
 * WAITS: no.
 * Entry: nothing. Exit, zField99 0: A 0, Z set. Else: A 2, X and Y the address, C and V from the last add.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit SetHealthBarAddress026E(void)
{
    return health_bar_address(resident_bar_vram_933b, 0x8C21);
}

void abi_SetHealthBarAddress026E(void)
{
    stage_exit_shim(SetHealthBarAddress026E());
}

/*
 * @routine 031E:63D SetStageTileVramAddress031E
 * @run 6B:8E3D
 * WHAT IT DOES: Sets the VRAM address of the tile pair zTemp04 names, from this program's table of addresses.
 * HOW: As SetStageTileVramAddress (024E:687): zTemp04 less bit 0 indexes the table; a tail JMP to the setter loaded at that address (several programs hold these bytes).
 * WAITS: no.
 * Entry: zTemp04. Exit: A 2, N and Z from it, X the address low, Y the high.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
uint16_t SetStageTileVramAddress031E(void)
{
    uint8_t index = (uint8_t)(RD8(ZP->temp04) & 0xFE);
    uint8_t lo = rd8((uint16_t)(0x8E4C + index));
    uint8_t hi = rd8((uint16_t)(0x8E4D + index));

    resident_tile_vram_961c(lo, hi);    /* JMP */
    return (uint16_t)(lo | hi << 8);
}

void abi_SetStageTileVramAddress031E(void)
{
    (void)SetStageTileVramAddress031E();
}

/*
 * @routine 031E:3F9 SetHealthBarAddress031E
 * @run 6B:8BF9
 * WHAT IT DOES: While zField99 is not 0, sets the VRAM address of the bar at the cell $5E less it counts.
 * HOW: As SetHealthBarAddress (024E:443), calling the setter loaded at its JSR's address (several programs hold these bytes).
 * WAITS: no.
 * Entry: nothing. Exit, zField99 0: A 0, Z set. Else: A 2, X and Y the address, C and V from the last add.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit SetHealthBarAddress031E(void)
{
    return health_bar_address(resident_bar_vram_961c, 0x8C21);
}

void abi_SetHealthBarAddress031E(void)
{
    stage_exit_shim(SetHealthBarAddress031E());
}

/*
 * @routine 026E:06C StartStageLists026E
 * @run 6B:886C
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
 * HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists026F, script slot 7 (the loaded routine: 6) and its return addresses.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageStartEnd StartStageLists026E(void)
{
    return start_lists(0x9C34, 0x9C3C, ExpandStageLists026F);
}

void abi_StartStageLists026E(void)
{
    start_lists_exit_shim(StartStageLists026E());
}

/*
 * @routine 031E:06C StartStageLists031E
 * @run 6B:886C
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
 * HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists031F, script slot 7 (the loaded routine: 6) and its return addresses.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageStartEnd StartStageLists031E(void)
{
    return start_lists(0x9DFF, 0x9E07, ExpandStageLists031F);
}

void abi_StartStageLists031E(void)
{
    start_lists_exit_shim(StartStageLists031E());
}

/*
 * @routine 043E:06C StartStageLists043E
 * @run 6B:886C
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
 * HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists043F, script slot 7 (the loaded routine: 6) and its return addresses.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageStartEnd StartStageLists043E(void)
{
    return start_lists(0x9EC3, 0x9ECB, ExpandStageLists043F);
}

void abi_StartStageLists043E(void)
{
    start_lists_exit_shim(StartStageLists043E());
}

/*
 * @routine 064E:06C StartStageLists064E
 * @run 6B:886C
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
 * HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists064F, script slot 7 (the loaded routine: 6) and its return addresses.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageStartEnd StartStageLists064E(void)
{
    return start_lists(0x9CE5, 0x9CED, ExpandStageLists064F);
}

void abi_StartStageLists064E(void)
{
    start_lists_exit_shim(StartStageLists064E());
}

/*
 * @routine 10DE:06C StartStageLists10DE
 * @run 6B:886C
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
 * HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists10DF, script slot 7 (the loaded routine: 6) and its return addresses.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageStartEnd StartStageLists10DE(void)
{
    return start_lists(0x9EB9, 0x9EC1, ExpandStageLists10DF);
}

void abi_StartStageLists10DE(void)
{
    start_lists_exit_shim(StartStageLists10DE());
}

/*
 * @routine 122E:06C StartStageLists122E
 * @run 6B:886C
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
 * HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists122F, script slot 7 (the loaded routine: 6) and its return addresses.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageStartEnd StartStageLists122E(void)
{
    return start_lists(0x9CBE, 0x9CC1, ExpandStageLists122F);
}

void abi_StartStageLists122E(void)
{
    start_lists_exit_shim(StartStageLists122E());
}

/*
 * @routine 1A2E:06C StartStageLists1A2E
 * @run 6B:886C
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
 * HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists1A2F, script slot 7 (the loaded routine: 6) and its return addresses.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageStartEnd StartStageLists1A2E(void)
{
    return start_lists(0x9CAD, 0x9CB5, ExpandStageLists1A2F);
}

void abi_StartStageLists1A2E(void)
{
    start_lists_exit_shim(StartStageLists1A2E());
}

/*
 * @routine 1B8E:06C StartStageLists1B8E
 * @run 6B:886C
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
 * HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists1B8F, script slot 7 (the loaded routine: 6) and its return addresses.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageStartEnd StartStageLists1B8E(void)
{
    return start_lists(0x9CFD, 0x9D01, ExpandStageLists1B8F);
}

void abi_StartStageLists1B8E(void)
{
    start_lists_exit_shim(StartStageLists1B8E());
}

/*
 * @routine 1CEE:06C StartStageLists1CEE
 * @run 6B:886C
 * WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
 * HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists1CEF, script slot 7 (the loaded routine: 6) and its return addresses.
 * WAITS: no.
 * Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageStartEnd StartStageLists1CEE(void)
{
    return start_lists(0x9C87, 0x9C8D, ExpandStageLists1CEF);
}

void abi_StartStageLists1CEE(void)
{
    start_lists_exit_shim(StartStageLists1CEE());
}

/*
 * @routine 026F:36E SpawnStageObject026F
 * @run 6B:936E
 * WHAT IT DOES: Puts a new object in the slot a row of this program's spawn tables names, with the row's frame, sprite set and X and Y low bytes.
 * HOW: As SpawnStageObject (024F:3B0), with this program's tables and return address.
 * WAITS: no.
 * Entry: X the row. Exit: A the Y low byte, N and Z from it, Y the slot. X kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
SpawnedObject SpawnStageObject026F(uint8_t row)
{
    static const uint16_t tables[5] = { 0x938E, 0x9396, 0x939E, 0x93A6, 0x93AE };

    return spawn_row(row, tables, 0x9375);
}

void abi_SpawnStageObject026F(void)
{
    SpawnedObject out = SpawnStageObject026F(cpu.x);

    cpu.a = out.y_lo;
    cpu.y = out.slot;
    abi_nz(cpu.a);
}

/*
 * @routine 031F:64F SpawnStageObject031F
 * @run 6B:964F
 * WHAT IT DOES: Puts a new object in the slot a row of this program's spawn tables names, with the row's frame, sprite set and X and Y low bytes.
 * HOW: As SpawnStageObject (024F:3B0), with this program's tables and return address.
 * WAITS: no.
 * Entry: X the row. Exit: A the Y low byte, N and Z from it, Y the slot. X kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
SpawnedObject SpawnStageObject031F(uint8_t row)
{
    static const uint16_t tables[5] = { 0x966F, 0x9677, 0x967F, 0x9687, 0x968F };

    return spawn_row(row, tables, 0x9656);
}

void abi_SpawnStageObject031F(void)
{
    SpawnedObject out = SpawnStageObject031F(cpu.x);

    cpu.a = out.y_lo;
    cpu.y = out.slot;
    abi_nz(cpu.a);
}

/*
 * @routine 026E:75A SpawnStageObjectsRows34026E
 * @run 6B:8F5A
 * WHAT IT DOES: Unless zField8E bit 0 is set, spawns rows 3 and 4 and sets up the two anchor objects and the player byte.
 * HOW: SpawnSecondPairBody (stage_spawn.c), the body of 024E:798, spawning with the row spawn loaded at its JSR's address (several programs hold these bytes), with this copy's return addresses.
 * WAITS: no.
 * Entry: nothing. Exit when it spawned: A 2, X 4, Y the second slot, N and Z from A; else as entered.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageSpawnEnd SpawnStageObjectsRows34026E(void)
{
    static const SpawnPairLayout layout = { 0, 0, 0x8F63, 0x8F67, 0 };

    return SpawnSecondPairBody(resident_spawn_936e, &layout);
}

void abi_SpawnStageObjectsRows34026E(void)
{
    StageSpawnEnd end = SpawnStageObjectsRows34026E();

    if (end.ran) {
        cpu.a = end.a;
        cpu.x = end.x;
        cpu.y = end.y;
        abi_nz(end.a);
    }
}

/*
 * @routine 031E:74E SpawnStageObjectsRows34031E
 * @run 6B:8F4E
 * WHAT IT DOES: Unless zField8E bit 0 is set, spawns rows 3 and 4 and sets up the two anchor objects and the player byte.
 * HOW: SpawnSecondPairBody (stage_spawn.c), the body of 024E:798, spawning with the row spawn loaded at its JSR's address (several programs hold these bytes), with this copy's return addresses.
 * WAITS: no.
 * Entry: nothing. Exit when it spawned: A 2, X 4, Y the second slot, N and Z from A; else as entered.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageSpawnEnd SpawnStageObjectsRows34031E(void)
{
    static const SpawnPairLayout layout = { 0, 0, 0x8F57, 0x8F5B, 0 };

    return SpawnSecondPairBody(resident_spawn_964f, &layout);
}

void abi_SpawnStageObjectsRows34031E(void)
{
    StageSpawnEnd end = SpawnStageObjectsRows34031E();

    if (end.ran) {
        cpu.a = end.a;
        cpu.x = end.x;
        cpu.y = end.y;
        abi_nz(end.a);
    }
}

/*
 * @routine 026F:3DF FillMarkedTiles026F
 * @run 6B:93DF
 * WHAT IT DOES: For each non-zero byte of a 256-byte table from index Y on, fills one 16-word VRAM tile with $55AA: the
 * tile at address X*$100 + index*16.
 * HOW: The table's address (A its high byte) in zTemp00-01. For a marked index: the write address selected, the address
 * set (the index shifted four times through zTemp03, X ORed into the high byte), the data register selected, sixteen
 * words by ST1 and ST2.
 * WAITS: no.
 * Entry: A the table's page, X the VRAM address high bits, Y the first index. Exit: Y 0, N 0, Z 1. If any tile was
 * filled: X 0, C 0; else X and C kept. A 2 if index $FF was filled, else 0. V kept. Clobbers zTemp00-03.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
TileFillEnd FillMarkedTiles026F(uint8_t page, uint8_t high, uint8_t y)
{
    TileFillEnd end = { 0, false };

    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, page);
    WR8(ZP->temp02, high);
    do {
        end.a = RD8_PTR_Y(ZP->temp00, y);
        if (end.a != 0) {
            uint8_t value = y;
            uint8_t other;

            jsr_push(0x93EC);
            SelectVramWriteAddress();
            jsr_pop();
            WR8(ZP->temp03, 0);
            for (int bit = 0; bit < 4; bit++) {
                bool carry = (value & 0x80) != 0;

                value = (uint8_t)(value << 1);
                ROL8(ZP->temp03, carry);
            }
            WR8(VDC->data_lo, value);
            value = RD8(ZP->temp03);
            other = RD8(ZP->temp02);
            WR8(VDC->data_hi, (uint8_t)(value | other));
            jsr_push(0x9408);
            SelectVramData();
            jsr_pop();
            for (int word = 0; word < 16; word++) {
                st1(0xAA);
                st2(0x55);
            }
            end.a = 2;
            end.filled = true;
        }
        y++;
    } while (y != 0);
    return end;
}

void abi_FillMarkedTiles026F(void)
{
    TileFillEnd end = FillMarkedTiles026F(cpu.a, cpu.x, cpu.y);

    cpu.a = end.a;
    cpu.y = 0;
    if (end.filled) {
        cpu.x = 0;
        abi_flag(P_C, false);
    }
    abi_nz(0);
}

/*
 * @routine 1531:2FC ClearEffectSlot1531
 * @run 6B:82FC
 * WHAT IT DOES: Empties a palette effect slot of the list program at sector 1531: its eight fields, 8 bytes apart from $3C00.
 * HOW: Eight STZ of the slot's columns, the kind first. This build has 8 slots where the later ones have 12.
 * WAITS: no.
 * Entry: X the slot. Exit: A, X, Y and the flags kept.
 * Nothing the game does loads sector 1531, and nothing is known to reach this routine.
 */
void ClearEffectSlot1531(uint8_t slot)
{
    for (int field = 0; field < 8; field++) {
        wr8((uint16_t)(ADDR_OF8(SLOTS->kind[0]) + 8 * field + slot), 0);
    }
}

void abi_ClearEffectSlot1531(void)
{
    ClearEffectSlot1531(cpu.x);
}

/*
 * @routine 1531:55C EffectRestoreColours1531
 * @run 6B:855C
 * WHAT IT DOES: Ends a palette effect of the list program at sector 1531: clears zFadeActive, sends the program's 256 background and 128 sprite colours to the VCE, and empties the slot.
 * HOW: Two TIA, from $8632 to colour 0 and from $8832 to colour $180, then a jump to ClearEffectSlot1531.
 * WAITS: no.
 * Entry: X the slot; Y pushed by the block moves. Exit: A $80, N set, Z clear. X, Y, C, V kept.
 * Nothing the game does loads sector 1531, and nothing is known to reach this routine.
 */
void EffectRestoreColours1531(uint8_t slot, uint8_t y)
{
    WR8(ZP->fade_active, 0);
    WR8(VCE->addr_hi, 0);
    WR8(VCE->addr_lo, 0);
    block_move_tia(0x8632, ADDR_OF(VCE->data_lo), 0x0200, 0x00, slot, y);
    WR8(VCE->addr_hi, 0x01);
    WR8(VCE->addr_lo, 0x80);
    block_move_tia(0x8832, ADDR_OF(VCE->data_lo), 0x0100, 0x80, slot, y);
    ClearEffectSlot1531(slot);
}

void abi_EffectRestoreColours1531(void)
{
    EffectRestoreColours1531(cpu.x, cpu.y);
    cpu.a = 0x80;
    abi_nz(0x80);
}

/*
 * @routine 026E:205 StepScrollAndRunObjects026E
 * @run 6B:8A05
 * WHAT IT DOES: Steps the stage scroll, maps the banks eight and nine above zRamBaseBank at $A000 and $C000, and goes on into the overlay at $A000.
 * HOW: A JSR to the scroll step at $89EC (the one loaded there: the nine programs hold these bytes), an add, two TAM, and a JMP to $A000.
 * WAITS: as the code at $A000.
 * Entry: X as the scroll step takes it. Exit: as the code at $A000, entered with A the second bank, N and Z from it, C and V from the add.
 * Nothing the game does loads these programs (codemap.unloaded_programs), so nothing is known to reach it.
 */
void StepScrollAndRunObjects026E(void)
{
    Sum bank;

    call_routine(0x8A08, 0x89EC);
    bank = add_bytes(0x08, RD8(ZP->ram_base_bank), false);
    tam(0x20, bank.value);
    tam(0x40, (uint8_t)(bank.value + 1));
    resident_jump_shim(bank);
}

void abi_StepScrollAndRunObjects026E(void)
{
    StepScrollAndRunObjects026E();
}

/*
 * @routine 026D:18B StagePauseToggle026D
 * @run 6B:818B
 * WHAT IT DOES: Pauses the stage when RUN is pressed, unless the fade is running, the display is still off, or
 * zPaused's bit 1 holds the pause (as StagePauseToggle, 024D:1A2). Programs 064D, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StagePauseToggleAt (stage_flow.c), the body of 024D:1A2: this program's JSR at $819F goes to $8E96, which
 * holds StagePauseSet's bytes (026E:696, the same as 024E:6D4), so its C is StagePauseSet's.
 * WAITS: no.
 * Entry: nothing. Exit: A the last value loaded or computed, N and Z from it; or as StagePauseSet. X, Y, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StagePauseToggle026D(StageRegs r)
{
    return StagePauseToggleAt(r, 0x81A2);
}

void abi_StagePauseToggle026D(void)
{
    stage_regs_load_shim(StagePauseToggle026D(stage_regs_entry_shim()));
}

/*
 * @routine 031D:18B StagePauseToggle031D
 * @run 6B:818B
 * WHAT IT DOES: Pauses the stage when RUN is pressed, unless the fade is running, the display is still off, or
 * zPaused's bit 1 holds the pause (as StagePauseToggle, 024D:1A2). Programs 043D and 10DD hold the same bytes.
 * HOW: StagePauseToggleAt (stage_flow.c), the body of 024D:1A2: this program's JSR at $819F goes to $8E8A, which
 * holds StagePauseSet's bytes (031E:68A, the same as 024E:6D4), so its C is StagePauseSet's.
 * WAITS: no.
 * Entry: nothing. Exit: A the last value loaded or computed, N and Z from it; or as StagePauseSet. X, Y, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StagePauseToggle031D(StageRegs r)
{
    return StagePauseToggleAt(r, 0x81A2);
}

void abi_StagePauseToggle031D(void)
{
    stage_regs_load_shim(StagePauseToggle031D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:0A8 StageCameraStep026D
 * @run 6B:80A8
 * WHAT IT DOES: The camera's frame with the overlay's bank 7 mapped (as StageCameraStep, 024D:0BF). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same
 * bytes.
 * HOW: StageCameraStepAt (stage_flow.c), the body of 024D:0BF: the JSR goes to $80A0, which holds MapOverlayBank7's bytes
 * (024D:0B7) in all nine programs, so its C is MapOverlayBank7's; then the tail JMP to $A08D, the C of the routine
 * loaded there.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as CameraStep, entered with A the bank, N, Z, C and V from the map's add.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageCameraStep026D(StageRegs r)
{
    return StageCameraStepAt(r, 0x80AB);
}

void abi_StageCameraStep026D(void)
{
    stage_regs_load_shim(StageCameraStep026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:0AE StageScrollSpeedsOff026D
 * @run 6B:80AE
 * WHAT IT DOES: Stops the layers' scroll: maps the overlay's bank 7 and runs its ClearScrollSpeeds (as StageScrollSpeedsOff, 024D:0C5). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same
 * bytes.
 * HOW: StageScrollSpeedsOffAt (stage_flow.c), the body of 024D:0C5: the JSR goes to $80A0, which holds MapOverlayBank7's bytes
 * (024D:0B7) in all nine programs, so its C is MapOverlayBank7's; then the tail JMP to $A067, the C of the routine
 * loaded there.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as ClearScrollSpeeds leaves them, entered with A the bank and N, Z, C and V from the map's add.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageScrollSpeedsOff026D(StageRegs r)
{
    return StageScrollSpeedsOffAt(r, 0x80B1);
}

void abi_StageScrollSpeedsOff026D(void)
{
    stage_regs_load_shim(StageScrollSpeedsOff026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:249 StageChooseClearKind026D
 * @run 6B:8249
 * WHAT IT DOES: Chooses how the cleared stage plays out (zClearKind) from zField8C: 0 gives 2, negative 4; else 1 for 1, 2, 7 to
 * $0A, $0D and $0E, and 3 for 3 to 6, $0B, $0C and $0F up; then sub-mode 3 with the scroll stopped and the forced
 * presses and time-slowing cleared (as StageChooseClearKind, 024D:260). Programs 031D, 043D, 064D,
 * 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageChooseClearKindAt (stage_flow.c), the body of 024D:260, with this program's JSR at $827E to its
 * StageScrollSpeedsOff026D at $80AE, the C of the routine loaded there.
 * WAITS: no.
 * Entry: nothing. Exit: as StageScrollSpeedsOff026D and the stores leave them; A, N and Z as that left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageChooseClearKind026D(StageRegs r)
{
    return StageChooseClearKindAt(r, 0x8281, 0x80AE);
}

void abi_StageChooseClearKind026D(void)
{
    stage_regs_load_shim(StageChooseClearKind026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:226 StageSubStepDeath026D
 * @run 6B:8226
 * WHAT IT DOES: Sub-step for the player's death: sub-mode 2 with zField80 and zField89 cleared, the scroll stopped, the forced
 * presses and the fall's step cleared; when the time-slowing mode is set it is cleared and the fade starts again from
 * 2 (as StageSubStepDeath, 024D:23D). Programs 031D, 043D, 064D,
 * 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageSubStepDeathAt (stage_flow.c), the body of 024D:23D, with this program's JSR at $822E to its
 * StageScrollSpeedsOff026D at $80AE, the C of the routine loaded there. Its tail JMP goes to $8161, which holds
 * StageFadeTimer's bytes (024D:178) in all nine programs, so its C is StageFadeTimer's.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: A TimeSlowMode (0), N and Z from it, the rest as StageScrollSpeedsOff026D left
 * them; else as StageFadeTimer.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSubStepDeath026D(StageRegs r)
{
    return StageSubStepDeathAt(r, 0x8231, 0x80AE);
}

void abi_StageSubStepDeath026D(void)
{
    stage_regs_load_shim(StageSubStepDeath026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:2A3 StageSubStepTally026D
 * @run 6B:82A3
 * WHAT IT DOES: Sub-step to the tally: sub-mode 6 with zField80 and zField89 cleared, the scroll stopped, the forced presses and
 * the fall's step cleared, the player's work bytes F cleared and E $40; when the time-slowing mode is set it is cleared
 * and the fade starts again from 2 (as StageSubStepTally, 024D:2BA). Programs 031D, 043D, 064D,
 * 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageSubStepTallyAt (stage_flow.c), the body of 024D:2BA, with this program's JSR at $82AB to its
 * StageScrollSpeedsOff026D at $80AE, the C of the routine loaded there. Its tail JMP goes to $8161, which holds
 * StageFadeTimer's bytes (024D:178) in all nine programs, so its C is StageFadeTimer's.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: A TimeSlowMode (0), N and Z from it, the rest as StageScrollSpeedsOff026D left
 * them; else as StageFadeTimer.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSubStepTally026D(StageRegs r)
{
    return StageSubStepTallyAt(r, 0x82AE, 0x80AE);
}

void abi_StageSubStepTally026D(void)
{
    stage_regs_load_shim(StageSubStepTally026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:28B StageSubStepFadeOut026D
 * @run 6B:828B
 * WHAT IT DOES: Sub-step to the fade: sub-mode 4 from its first step, zField89 cleared, the scroll stopped (as StageSubStepFadeOut, 024D:2A2). Programs 031D, 043D, 064D,
 * 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageSubStepFadeOutAt (stage_flow.c), the body of 024D:2A2, with this program's tail JMP at $8293 to its
 * StageScrollSpeedsOff026D at $80AE, the C of the routine loaded there.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as StageScrollSpeedsOff026D.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSubStepFadeOut026D(StageRegs r)
{
    return StageSubStepFadeOutAt(r, 0x8293, 0x80AE);
}

void abi_StageSubStepFadeOut026D(void)
{
    stage_regs_load_shim(StageSubStepFadeOut026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:296 StageSubStepFadeOutLate026D
 * @run 6B:8296
 * WHAT IT DOES: Sub-step to the fade at its step 5: sub-mode 4, zField80 5, zField89 cleared, the scroll stopped (as StageSubStepFadeOutLate, 024D:2AD). Programs 031D, 043D, 064D,
 * 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageSubStepFadeOutLateAt (stage_flow.c), the body of 024D:2AD, with this program's tail JMP at $82A0 to its
 * StageScrollSpeedsOff026D at $80AE, the C of the routine loaded there.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as StageScrollSpeedsOff026D.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageSubStepFadeOutLate026D(StageRegs r)
{
    return StageSubStepFadeOutLateAt(r, 0x82A0, 0x80AE);
}

void abi_StageSubStepFadeOutLate026D(void)
{
    stage_regs_load_shim(StageSubStepFadeOutLate026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:2F5 StageForcedWalk026D
 * @run 6B:82F5
 * WHAT IT DOES: A sub-mode 3 start: the stage set up (InitStage026E), sub-mode 1, and the player walked by a forced pad away from
 * the side he faces: $18 frames when zField8C is set (it is cleared). As StageForcedWalk, 024D:31D. Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageForcedWalkAt (stage_flow.c), the body of 024D:31D, with this program's JSR at $82F5 to InitStage026E at
 * $88A2 (the C of the routine loaded there) and its table of pads at $8316.
 * WAITS: where InitStage026E waits.
 * Entry: nothing. Exit: A the pad, N and Z from it, X the table index; Y, C and V as InitStage026E left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageForcedWalk026D(StageRegs r)
{
    return StageForcedWalkAt(r, 0x82F8, 0x88A2, 0x8316);
}

void abi_StageForcedWalk026D(void)
{
    stage_regs_load_shim(StageForcedWalk026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:318 StageForcedWalkB026D
 * @run 6B:8318
 * WHAT IT DOES: A sub-mode 3 start: the stage set up (InitStage026E), sub-mode 1, and the player walked by a forced pad away from
 * the side he faces: $18 frames when zField8C is set (it is cleared). As StageForcedWalk, 024D:31D. These bytes are 026D:2F5's at
 * $8318; their C is filed again because the JSR's return address is its own. Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageForcedWalkAt (stage_flow.c), the body of 024D:31D, with this program's JSR at $8318 to InitStage026E at
 * $88A2 (the C of the routine loaded there) and its table of pads at $8316.
 * WAITS: where InitStage026E waits.
 * Entry: nothing. Exit: A the pad, N and Z from it, X the table index; Y, C and V as InitStage026E left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageForcedWalkB026D(StageRegs r)
{
    return StageForcedWalkAt(r, 0x831B, 0x88A2, 0x8316);
}

void abi_StageForcedWalkB026D(void)
{
    stage_regs_load_shim(StageForcedWalkB026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:358 StageFallRightLand026D
 * @run 6B:8358
 * WHAT IT DOES: The fall to the right's middle step: the player idle, his animation sound, and until his X passes $10 on a
 * screen past the first the step counts on; else his landing (as StageFallRightLand, 024D:380). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageFallRightLandAt (stage_flow.c), the body of 024D:380, at $8358. Its JSR to $8089 goes to
 * MapOverlayBanks5And6's bytes (024D:0A0) in all nine programs, so its C is MapOverlayBanks5And6's.
 * WAITS: no.
 * Entry: nothing. Exit: counting: N and Z from the new step; else as $A01E.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageFallRightLand026D(StageRegs r)
{
    return StageFallRightLandAt(r, 0x8358);
}

void abi_StageFallRightLand026D(void)
{
    stage_regs_load_shim(StageFallRightLand026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:34B StageFallPose026D
 * @run 6B:834B
 * WHAT IT DOES: The fall's first step: the player's animation 0 set and its sound, then the step counted on (as StageFallPose,
 * 024D:373). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageFallPoseAt (stage_flow.c), the body of 024D:373, at $834B. Its JSR to $8089 goes to MapOverlayBanks5And6's
 * bytes (024D:0A0) in all nine programs, so its C is MapOverlayBanks5And6's; the BRA goes to the INC of zFallStep in
 * StageFallRightLand026D.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the new step; the rest as $A012 left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageFallPose026D(StageRegs r)
{
    return StageFallPoseAt(r, 0x834B);
}

void abi_StageFallPose026D(void)
{
    stage_regs_load_shim(StageFallPose026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:374 StageFallRightDone026D
 * @run 6B:8374
 * WHAT IT DOES: The last step of the fall to the right: the stage set up again and the player walked on by pad $20 (as
 * StageFallRightDone, 024D:39C). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageFallRightDoneAt (stage_flow.c), the body of 024D:39C, with this program's JSR at $8374 to InitStage026E at
 * $88A2, the C of the routine loaded there.
 * WAITS: where InitStage026E waits.
 * Entry: nothing. Exit: A $20, N and Z from it; X, Y, C and V as InitStage026E left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageFallRightDone026D(StageRegs r)
{
    return StageFallRightDoneAt(r, 0x8377, 0x88A2);
}

void abi_StageFallRightDone026D(void)
{
    stage_regs_load_shim(StageFallRightDone026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:3C4 StageFallLeftDone026D
 * @run 6B:83C4
 * WHAT IT DOES: The last step of the fall to the left: the stage set up again and the player walked on by pad $80 (as
 * StageFallLeftDone, 024D:3EC). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageFallLeftDoneAt (stage_flow.c), the body of 024D:3EC, with this program's JSR at $83C4 to InitStage026E at
 * $88A2, the C of the routine loaded there.
 * WAITS: where InitStage026E waits.
 * Entry: nothing. Exit: A $80, N and Z from it; X, Y, C and V as InitStage026E left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageFallLeftDone026D(StageRegs r)
{
    return StageFallLeftDoneAt(r, 0x83C7, 0x88A2);
}

void abi_StageFallLeftDone026D(void)
{
    stage_regs_load_shim(StageFallLeftDone026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:3A8 StageFallLeftLand026D
 * @run 6B:83A8
 * WHAT IT DOES: The fall to the left's middle step: the player idle, his animation sound, and while his X is below $F0 on a
 * screen past the first the step counts on; else his landing (as StageFallLeftLand, 024D:3D0). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageFallLeftLandAt (stage_flow.c), the body of 024D:3D0, at $83A8. Its JSR to $8089 goes to
 * MapOverlayBanks5And6's bytes (024D:0A0) in all nine programs, so its C is MapOverlayBanks5And6's; its branch to
 * $83C1 goes to StageFallStepNext's bytes (024D:3E9), whose C is StageFallStepNext's.
 * WAITS: no.
 * Entry: nothing. Exit: counting: N and Z from the new step; else as $A01E.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageFallLeftLand026D(StageRegs r)
{
    return StageFallLeftLandAt(r, 0x83A8);
}

void abi_StageFallLeftLand026D(void)
{
    stage_regs_load_shim(StageFallLeftLand026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:339 StageFallRight026D
 * @run 6B:8339
 * @stop 834B
 * WHAT IT DOES: The fall to the right, a step a frame: the first sets the player's speed $0140 and runs into
 * StageFallPose026D; the second StageFallRightLand026D; the third StageFallRightDone026D (as StageFallRight,
 * 024D:361). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageFallRightAt (stage_flow.c), the body of 024D:361, with this program's steps.
 * WAITS: as the step taken.
 * Entry: nothing. Exit: as the step taken.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageFallRight026D(StageRegs r)
{
    return StageFallRightAt(r, StageFallRightLand026D, StageFallRightDone026D, StageFallPose026D);
}

void abi_StageFallRight026D(void)
{
    stage_regs_load_shim(StageFallRight026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:388 StageFallLeft026D
 * @run 6B:8388
 * @stop 83A8
 * WHAT IT DOES: The fall to the left: the second step StageFallLeftLand026D, the third StageFallLeftDone026D; the first, with
 * the object in slot $50 of kind $30, counts the step on twice, else sets the player's speed -$0140, poses him
 * (StageFallPose026D) and runs into StageFallLeftLand026D (as StageFallLeft, 024D:3B0). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageFallLeftAt (stage_flow.c), the body of 024D:3B0, with this program's steps; its BSR and BRA go to
 * StageFallStepNext's bytes at $83C1, whose C is StageFallStepNext's.
 * WAITS: as the step taken.
 * Entry: nothing. Exit: as the step taken.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageFallLeft026D(StageRegs r)
{
    return StageFallLeftAt(r, StageFallLeftLand026D, StageFallLeftDone026D, StageFallPose026D, 0x8399, 0x83A8);
}

void abi_StageFallLeft026D(void)
{
    stage_regs_load_shim(StageFallLeft026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:3EB StageRestartInPlace026D
 * @run 6B:83EB
 * WHAT IT DOES: Sets the stage up anew in place: the screen cleared, its lists, VRAM runs, set-up, objects and the player's start,
 * then sub-mode 1 with zField86 from zField83 (as StageRestartInPlace, 024D:413). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes, their calls going to the routines
 * each loads at the same addresses.
 * HOW: StageRestartInPlaceAt (stage_flow.c), the body of 024D:413, at $83EB, with this program's calls to $886C, $883D,
 * $88A2, $8B09 and $8A15, each the C of the routine loaded there; its JSR to $8089 goes to MapOverlayBanks5And6's
 * bytes (024D:0A0) in all nine programs, so its C is MapOverlayBanks5And6's.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: A zField83, N and Z from it; X, Y, C and V as SetUpStageObjects026E left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageRestartInPlace026D(StageRegs r)
{
    return StageRestartInPlaceAt(r, 0x83EB, 0x886C, 0x883D, 0x88A2, 0x8B09, 0x8A15);
}

void abi_StageRestartInPlace026D(void)
{
    stage_regs_load_shim(StageRestartInPlace026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:420 StageShowStartFrame026D
 * @run 6B:8420
 * WHAT IT DOES: The player's frame by character set, then his animation sound (as StageShowStartFrame, 024D:448). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageShowStartFrameAt (stage_flow.c), the body of 024D:448, at $8420 with this program's table of frames at
 * $842E; its JSR to $8089 goes to MapOverlayBanks5And6's bytes (024D:0A0) in all nine programs, so its C is
 * MapOverlayBanks5And6's.
 * WAITS: no.
 * Entry: nothing. Exit: as $A012, entered with X zCharacter, A the frame, N and Z from it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageShowStartFrame026D(StageRegs r)
{
    return StageShowStartFrameAt(r, 0x8420, 0x842E);
}

void abi_StageShowStartFrame026D(void)
{
    stage_regs_load_shim(StageShowStartFrame026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:49F StageClearedWaitScroll026D
 * @run 6B:849F
 * WHAT IT DOES: A cleared stage's step: the scroll and the objects run, and once zField8C's bit 7 is set the next step comes (as
 * StageClearedWaitScroll, 024D:4C7). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageClearedWaitScrollAt (stage_flow.c), the body of 024D:4C7, with this program's JSR at $849F to
 * StepScrollAndRunObjects026E at $8A05, the C of the routine loaded there.
 * WAITS: as StepScrollAndRunObjects026E.
 * Entry: X the caller's. Exit: A zField8C, N and Z from it or from zField85's INC; X, Y, C and V as the objects left
 * them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageClearedWaitScroll026D(StageRegs r)
{
    return StageClearedWaitScrollAt(r, 0x84A2, 0x8A05);
}

void abi_StageClearedWaitScroll026D(void)
{
    stage_regs_load_shim(StageClearedWaitScroll026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:65F StageClearedScrollOn026D
 * @run 6B:865F
 * WHAT IT DOES: A cleared stage's step: the scroll and the objects run, zField8C's bit 7 is cleared and the next step comes (as
 * StageClearedScrollOn, 024D:695). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageClearedScrollOnAt (stage_flow.c), the body of 024D:695, with this program's JSR at $865F to
 * StepScrollAndRunObjects026E at $8A05, the C of the routine loaded there.
 * WAITS: as StepScrollAndRunObjects026E.
 * Entry: X the caller's. Exit: N and Z from zField85's INC; A, X, Y, C and V as the objects left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageClearedScrollOn026D(StageRegs r)
{
    return StageClearedScrollOnAt(r, 0x8662, 0x8A05);
}

void abi_StageClearedScrollOn026D(void)
{
    stage_regs_load_shim(StageClearedScrollOn026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:465 StageWalkOutScroll026D
 * @run 6B:8465
 * WHAT IT DOES: A frame of walking the player out of a cleared room: the scroll stepped by the route's direction, the player
 * moved with it, the camera and objects run; at the route's stop the scroll halts and the next step comes (as
 * StageWalkOutScroll, 024D:48D). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageWalkOutScrollAt (stage_steps.c), the body of 024D:48D, at $8465 with this program's tables at $8414, $8416
 * and $8418 and its calls to $80A8, $8A05 and $80AE, each the C of the routine loaded there.
 * WAITS: as the calls.
 * Entry: nothing. Exit: walking: A the player's X, N, Z and C from the compare, Y the route; stopped: N and Z from
 * zField85's INC.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageWalkOutScroll026D(StageRegs r)
{
    return StageWalkOutScrollAt(r, 0x8465, 0x8414, 0x8416, 0x8418, 0x80A8, 0x8A05, 0x80AE);
}

void abi_StageWalkOutScroll026D(void)
{
    stage_regs_load_shim(StageWalkOutScroll026D(stage_regs_entry_shim()));
}

/*
 * @routine 026D:656 StageStopAndStep026D
 * @run 6B:8656
 * WHAT IT DOES: The scroll stopped, a frame of the objects, the next step (as StageStopAndStep, 024D:68C). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
 * HOW: StageStopAndStepAt (stage_steps.c), the body of 024D:68C, at $8656 with its calls to $80AE and $8A05, each the
 * C of the routine loaded there.
 * WAITS: as the calls.
 * Entry: nothing. Exit: N and Z from zField85's INC; the rest as the objects left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs StageStopAndStep026D(StageRegs r)
{
    return StageStopAndStepAt(r, 0x8656, 0x80AE, 0x8A05);
}

void abi_StageStopAndStep026D(void)
{
    stage_regs_load_shim(StageStopAndStep026D(stage_regs_entry_shim()));
}

/*
 * @routine 026E:3D8 StageScriptByWave026E
 * @run 6B:8BD8
 * WHAT IT DOES: Starts the stage's script in slot 1 that the wave counter names, when it is not the one already shown (as
 * StageScriptByWave, 024E:422). Programs 031E, 043E, 064E, 10DE, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
 * HOW: StageScriptByWaveAt (stage_tick.c), the body of 024E:422, with this program's table of scripts at $8BF1.
 * WAITS: no.
 * Entry: nothing. Exit: unchanged script: A the script, X the table index, Z 1, C 1; else as StartScriptSlotKind4 with
 * X 1, C as the compare left it.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit StageScriptByWave026E(void)
{
    return StageScriptByWaveAt(0x8BF1);
}

void abi_StageScriptByWave026E(void)
{
    stage_exit_shim(StageScriptByWave026E());
}

/*
 * @routine 026E:51A BossFlashMaskCell026E
 * @run 6B:8D1A
 * WHAT IT DOES: Masks one cell of VRAM, $40 words, with random bits and a pattern by the flash's step (as BossFlashMaskCell,
 * 024E:564). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
 * HOW: BossFlashMaskCellAt (stage_boss_flash.c), the body of 024E:564, at $8D1A with this program's table of masks at
 * $8CD3; its JSR to $933B is the C of the routine loaded there (SetVramAddresses026F), and its JSR to $9350 goes to
 * AndVramData's bytes (024F:392), the C of the routine loaded there.
 * WAITS: no.
 * Entry: X and Y the address. Exit: X 0, N 0, Z 1; A, Y, C and V as the last AndVramData left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs BossFlashMaskCell026E(StageRegs r)
{
    return BossFlashMaskCellAt(r, 0x8D1A, 0x933B, 0x9350, 0x8CD3);
}

void abi_BossFlashMaskCell026E(void)
{
    stage_regs_load_shim(BossFlashMaskCell026E(stage_regs_entry_shim()));
}

/*
 * @routine 031E:51A BossFlashMaskCell031E
 * @run 6B:8D1A
 * WHAT IT DOES: Masks one cell of VRAM, $40 words, with random bits and a pattern by the flash's step (as BossFlashMaskCell,
 * 024E:564). Programs 043E and 10DE hold the same bytes.
 * HOW: BossFlashMaskCellAt (stage_boss_flash.c), the body of 024E:564, at $8D1A with this program's table of masks at
 * $8CD3; its JSR to $961C is the C of the routine loaded there (SetVramAddresses031F), and its JSR to $9631 goes to
 * AndVramData's bytes (024F:392), the C of the routine loaded there.
 * WAITS: no.
 * Entry: X and Y the address. Exit: X 0, N 0, Z 1; A, Y, C and V as the last AndVramData left them.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs BossFlashMaskCell031E(StageRegs r)
{
    return BossFlashMaskCellAt(r, 0x8D1A, 0x961C, 0x9631, 0x8CD3);
}

void abi_BossFlashMaskCell031E(void)
{
    stage_regs_load_shim(BossFlashMaskCell031E(stage_regs_entry_shim()));
}

/* The gauge of the 026D and 031D programs: the loaded one's code moved down $4A ($8C39 to $8D19 where the loaded
 * overlays have $8C83 to $8D63), drawing through each program's own VRAM routines. */
static const GaugeVram GAUGE_026E = { SetVramAddresses026F, AndVramData, OrVramData, (uint16_t)-0x4A };
static const GaugeVram GAUGE_031E = { SetVramAddresses031F, AndVramData, OrVramData, (uint16_t)-0x4A };

/*
 * @routine 026E:439 GaugeTimerCheckB026E
 * @run 6B:8C39
 * @stop 8C41
 * WHAT IT DOES: While the life timer of slot 85 is zero, the gauge steps (GaugeStepB026E); else the boss gauge's flash
 * at $8CDB (as GaugeTimerCheckB, 024E:483). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
 * HOW: GaugeTimerCheckAt (stage_gauge.c), the body of 024E:483, its jumps $4A below the loaded overlays', each to the
 * C of the routine loaded there.
 * WAITS: no.
 * Entry: nothing. Exit: as the step or the flash taken.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
void abi_GaugeTimerCheckB026E(void)
{
    GaugeTimerCheckAt((uint16_t)-0x4A);
}

/*
 * @routine 026E:441 GaugeStepB026E
 * @run 6B:8C41
 * @stop 8C6D
 * WHAT IT DOES: Moves the gauge one step towards its target, as GaugeStep with this program's VRAM routines (024E:48B). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
 * HOW: GaugeStepAt (stage_gauge.c), the body of 024E:48B, with this program's VRAM routines ($933B, and the bytes of
 * AndVramData and OrVramData at $9350 and $935F) and its code $4A below the loaded overlays'.
 * WAITS: no.
 * Entry: nothing. Exit: as GaugeStep.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit GaugeStepB026E(void)
{
    return GaugeStepAt(&GAUGE_026E);
}

void abi_GaugeStepB026E(void)
{
    stage_exit_shim(GaugeStepB026E());
}

/*
 * @routine 026E:46D GaugeStepUpB026E
 * @run 6B:8C6D
 * @stop 8C7A
 * WHAT IT DOES: Grows the gauge one, as GaugeStepUp with this program's VRAM routines (024E:4B7). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
 * HOW: GaugeStepUpAt (stage_gauge.c), the body of 024E:4B7, with this program's VRAM routines ($933B, and the bytes of
 * AndVramData and OrVramData at $9350 and $935F) and its code $4A below the loaded overlays'.
 * WAITS: no.
 * Entry: nothing. Exit: as GaugeStepUp.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit GaugeStepUpB026E(void)
{
    return GaugeStepUpAt(&GAUGE_026E);
}

void abi_GaugeStepUpB026E(void)
{
    stage_exit_shim(GaugeStepUpB026E());
}

/*
 * @routine 026E:47A SetGaugeAddressB026E
 * @run 6B:8C7A
 * WHAT IT DOES: Points the VDC at the gauge's cell for its value, as SetGaugeAddress with this program's VRAM routine (024E:4C4). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
 * HOW: SetGaugeAddressAt (stage_gauge.c), the body of 024E:4C4, with this program's VRAM routines ($933B, and the bytes of
 * AndVramData and OrVramData at $9350 and $935F) and its code $4A below the loaded overlays'.
 * WAITS: no.
 * Entry: nothing. Exit: as SetGaugeAddress.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit SetGaugeAddressB026E(void)
{
    return SetGaugeAddressAt(&GAUGE_026E);
}

void abi_SetGaugeAddressB026E(void)
{
    stage_exit_shim(SetGaugeAddressB026E());
}

/*
 * @routine 031E:439 GaugeTimerCheckB031E
 * @run 6B:8C39
 * @stop 8C41
 * WHAT IT DOES: While the life timer of slot 85 is zero, the gauge steps (GaugeStepB031E); else the boss gauge's flash
 * at $8CDB (as GaugeTimerCheckB, 024E:483). Programs 043E and 10DE hold the same bytes.
 * HOW: GaugeTimerCheckAt (stage_gauge.c), the body of 024E:483, its jumps $4A below the loaded overlays', each to the
 * C of the routine loaded there.
 * WAITS: no.
 * Entry: nothing. Exit: as the step or the flash taken.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
void abi_GaugeTimerCheckB031E(void)
{
    GaugeTimerCheckAt((uint16_t)-0x4A);
}

/*
 * @routine 031E:441 GaugeStepB031E
 * @run 6B:8C41
 * @stop 8C6D
 * WHAT IT DOES: Moves the gauge one step towards its target, as GaugeStep with this program's VRAM routines (024E:48B). Programs 043E and 10DE hold the same bytes.
 * HOW: GaugeStepAt (stage_gauge.c), the body of 024E:48B, with this program's VRAM routines ($961C, and the bytes of
 * AndVramData and OrVramData at $9631 and $9640) and its code $4A below the loaded overlays'.
 * WAITS: no.
 * Entry: nothing. Exit: as GaugeStep.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit GaugeStepB031E(void)
{
    return GaugeStepAt(&GAUGE_031E);
}

void abi_GaugeStepB031E(void)
{
    stage_exit_shim(GaugeStepB031E());
}

/*
 * @routine 031E:46D GaugeStepUpB031E
 * @run 6B:8C6D
 * @stop 8C7A
 * WHAT IT DOES: Grows the gauge one, as GaugeStepUp with this program's VRAM routines (024E:4B7). Programs 043E and 10DE hold the same bytes.
 * HOW: GaugeStepUpAt (stage_gauge.c), the body of 024E:4B7, with this program's VRAM routines ($961C, and the bytes of
 * AndVramData and OrVramData at $9631 and $9640) and its code $4A below the loaded overlays'.
 * WAITS: no.
 * Entry: nothing. Exit: as GaugeStepUp.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit GaugeStepUpB031E(void)
{
    return GaugeStepUpAt(&GAUGE_031E);
}

void abi_GaugeStepUpB031E(void)
{
    stage_exit_shim(GaugeStepUpB031E());
}

/*
 * @routine 031E:47A SetGaugeAddressB031E
 * @run 6B:8C7A
 * WHAT IT DOES: Points the VDC at the gauge's cell for its value, as SetGaugeAddress with this program's VRAM routine (024E:4C4). Programs 043E and 10DE hold the same bytes.
 * HOW: SetGaugeAddressAt (stage_gauge.c), the body of 024E:4C4, with this program's VRAM routines ($961C, and the bytes of
 * AndVramData and OrVramData at $9631 and $9640) and its code $4A below the loaded overlays'.
 * WAITS: no.
 * Entry: nothing. Exit: as SetGaugeAddress.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit SetGaugeAddressB031E(void)
{
    return SetGaugeAddressAt(&GAUGE_031E);
}

void abi_SetGaugeAddressB031E(void)
{
    stage_exit_shim(SetGaugeAddressB031E());
}

/*
 * @routine 026E:4DB BossGaugeFlash026E
 * @run 6B:8CDB
 * WHAT IT DOES: A frame of the boss gauge's flash: masks four cells, counts the flash's step, makes the object in slot 85 fall
 * faster and move, and at the bottom hides it and clears bit 2 of zField8E (as BossGaugeFlash, 024E:525). These are
 * 024E:525's instructions $4A lower, every operand into the code moved alike; their return addresses are their own. Programs 031E, 043E, 064E, 10DE, 122E, 1A2E,
 * 1B8E and 1CEE hold the same bytes, their call at $8CEC going to their own routine there.
 * HOW: BossGaugeFlashAt (stage_boss_flash.c), the body of 024E:525, at $8CDB with this program's table of cells at $8CC3;
 * each cell through the C of the routine loaded at $8D1A (it differs between these programs).
 * WAITS: no.
 * Entry: nothing. Exit: A the object's Y high, N and Z from it (RMB sets none); X 85.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageRegs BossGaugeFlash026E(StageRegs r)
{
    return BossGaugeFlashAt(r, 0x8CDB, 0x8CC3, 0x8D1A);
}

void abi_BossGaugeFlash026E(void)
{
    stage_regs_load_shim(BossGaugeFlash026E(stage_regs_entry_shim()));
}

/* The JSR to the tile address setter loaded at $8E3D (it differs between these programs): the C of the routine there. */
static void resident_tile_address_8e3d(void)
{
    run_routine(0x8E3D);
}

/*
 * @routine 026E:53A BlendStageTileRows026E
 * @run 6B:8D3A
 * WHAT IT DOES: Writes five tile colour values into the VRAM words of one tile row step of the stage, from one row of
 * StageTileRows or, while a blend runs, from that row and a second one (as BlendStageTileRows, 024E:584). These are
 * 024E:584's instructions $4A lower. Programs 031E, 043E, 064E, 10DE, 122E, 1A2E, 1B8E
 * and 1CEE hold the same bytes, their tile address setter at $8E3D their own.
 * HOW: BlendStageTileRowsAt (stage_tile_rows.c), the body of 024E:584, its code $4A below the loaded overlays'; its two
 * JSRs to $8E3D go to the C of the tile address setter loaded there.
 * WAITS: no.
 * Entry: X the step (zTemp04), A the first row, Y the second row. Exit: A the last byte written, X the blend step, Y 5,
 * N 0, Z 1, C 1, V from the last add.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
TileRowsExit BlendStageTileRows026E(uint8_t phase, uint8_t row_a, uint8_t row_b)
{
    return BlendStageTileRowsAt(phase, row_a, row_b, (uint16_t)-0x4A, resident_tile_address_8e3d);
}

void abi_BlendStageTileRows026E(void)
{
    blend_exit_shim(BlendStageTileRows026E(cpu.x, cpu.a, cpu.y));
}

/*
 * @routine 026F:224 BlendStageRowPair026F
 * @run 6B:9224
 * WHAT IT DOES: Blends the two rows a byte names: its low nibble's row at the step X, its high nibble's row at the step after
 * (as BlendStageRowPair, 024F:266). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: BlendStageRowPairAt (stage_tile_rows.c), the body of 024F:266, at $9224, with BlendStageTileRows026E at $8D3A
 * (the bytes of the tile row blend in all these programs).
 * WAITS: no.
 * Entry: A the byte, X the step, Y the second row of the first blend. Exit: as BlendStageTileRows026E.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
TileRowsExit BlendStageRowPair026F(uint8_t x, uint8_t a, uint8_t y)
{
    return BlendStageRowPairAt(x, a, y, 0x9224, BlendStageTileRows026E);
}

void abi_BlendStageRowPair026F(void)
{
    blend_exit_shim(BlendStageRowPair026F(cpu.x, cpu.a, cpu.y));
}

/*
 * @routine 031F:218 BlendStageRowPair031F
 * @run 6B:9218
 * WHAT IT DOES: Blends the two rows a byte names: its low nibble's row at the step X, its high nibble's row at the step after
 * (as BlendStageRowPair, 024F:266). Programs 043F and 10DF hold the same bytes; these are 026F:224's at $9218, filed again
 * because the JSR's return address is its own.
 * HOW: BlendStageRowPairAt (stage_tile_rows.c), the body of 024F:266, at $9218, with BlendStageTileRows026E at $8D3A
 * (the bytes of the tile row blend in all these programs).
 * WAITS: no.
 * Entry: A the byte, X the step, Y the second row of the first blend. Exit: as BlendStageTileRows026E.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
TileRowsExit BlendStageRowPair031F(uint8_t x, uint8_t a, uint8_t y)
{
    return BlendStageRowPairAt(x, a, y, 0x9218, BlendStageTileRows026E);
}

void abi_BlendStageRowPair031F(void)
{
    blend_exit_shim(BlendStageRowPair031F(cpu.x, cpu.a, cpu.y));
}

/*
 * @routine 026F:1F4 BlendStageRowChanges026F
 * @run 6B:91F4
 * WHAT IT DOES: Blends the rows a byte changed: for each nibble of the old byte (zBiosAl) that differs from the new (A), the old
 * nibble's row is blended in at the step X or the step after (as BlendStageRowChanges, 024F:236). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: BlendStageRowChangesAt (stage_tile_rows.c), the body of 024F:236, at $91F4, with BlendStageTileRows026E at $8D3A
 * (the bytes of the tile row blend in all these programs).
 * WAITS: no.
 * Entry: A the new byte, zBiosAl the old, X the step. Exit: as BlendStageRowChanges.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
RowChangesExit BlendStageRowChanges026F(uint8_t x, uint8_t new_byte)
{
    return BlendStageRowChangesAt(x, new_byte, 0x91F4, BlendStageTileRows026E);
}

void abi_BlendStageRowChanges026F(void)
{
    row_changes_exit_shim(BlendStageRowChanges026F(cpu.x, cpu.a));
}

/*
 * @routine 031F:1E8 BlendStageRowChanges031F
 * @run 6B:91E8
 * WHAT IT DOES: Blends the rows a byte changed: for each nibble of the old byte (zBiosAl) that differs from the new (A), the old
 * nibble's row is blended in at the step X or the step after (as BlendStageRowChanges, 024F:236). Programs 043F and 10DF hold the same bytes; these are 026F:1F4's at $91E8, filed again
 * because the JSRs' return addresses are their own.
 * HOW: BlendStageRowChangesAt (stage_tile_rows.c), the body of 024F:236, at $91E8, with BlendStageTileRows026E at $8D3A
 * (the bytes of the tile row blend in all these programs).
 * WAITS: no.
 * Entry: A the new byte, zBiosAl the old, X the step. Exit: as BlendStageRowChanges.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
RowChangesExit BlendStageRowChanges031F(uint8_t x, uint8_t new_byte)
{
    return BlendStageRowChangesAt(x, new_byte, 0x91E8, BlendStageTileRows026E);
}

void abi_BlendStageRowChanges031F(void)
{
    row_changes_exit_shim(BlendStageRowChanges031F(cpu.x, cpu.a));
}

/*
 * @routine 026F:10F BlendStageStockRows026F
 * @run 6B:910F
 * WHAT IT DOES: Shows the player's stock on the stage's tile rows when it changed (as BlendStageStockRows, 024F:151). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: blend_stock_rows_at (stock_display.c), the body of 024F:151, at $910F, with BlendStageRowPair026F; its JSR to $92FD
 * and its jumps to $9312 go to SaveAndMapStageBanks' and RestoreStageBanks' bytes (024F:33F and 024F:354), so
 * their C is theirs.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as BlendStageStockRows.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm BlendStageStockRows026F(uint8_t slot, uint8_t y)
{
    return blend_stock_rows_at(slot, y, &stage_banks_024f, 0x910F, BlendStageRowPair026F);
}

void abi_BlendStageStockRows026F(void)
{
    abi_ended(blend_stock_rows_exit_shim(BlendStageStockRows026F(cpu.x, cpu.y)));
}

/*
 * @routine 031F:103 BlendStageStockRows031F
 * @run 6B:9103
 * WHAT IT DOES: Shows the player's stock on the stage's tile rows when it changed (as BlendStageStockRows, 024F:151). Programs 043F and 10DF hold the same bytes.
 * HOW: blend_stock_rows_at (stock_display.c), the body of 024F:151, at $9103, with BlendStageRowPair031F; its JSR to $95DE
 * and its jumps to $95F3 go to SaveAndMapStageBanks' and RestoreStageBanks' bytes (024F:33F and 024F:354), so
 * their C is theirs.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as BlendStageStockRows.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm BlendStageStockRows031F(uint8_t slot, uint8_t y)
{
    return blend_stock_rows_at(slot, y, &stage_banks_024f, 0x9103, BlendStageRowPair031F);
}

void abi_BlendStageStockRows031F(void)
{
    abi_ended(blend_stock_rows_exit_shim(BlendStageStockRows031F(cpu.x, cpu.y)));
}

/*
 * @routine 026F:13F BlendStageScoreAddRows026F
 * @run 6B:913F
 * WHAT IT DOES: Shows the score being added on the stage's tile rows while the blend is off (as BlendStageScoreAddRows,
 * 024F:181). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: blend_rows_of_player_at (stage_blend_long.c), the body of 024F:181, at $913F, with BlendStageRowPair026F; its JSR
 * to $92FD and its jumps to $9312 go to SaveAndMapStageBanks' and RestoreStageBanks' bytes (024F:33F and 024F:354),
 * so their C is theirs.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as BlendStageScoreAddRows.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm BlendStageScoreAddRows026F(uint8_t slot, uint8_t y)
{
    return blend_rows_of_player_at(slot, y, &stage_banks_024f, 0x913F, BlendStageRowPair026F);
}

void abi_BlendStageScoreAddRows026F(void)
{
    abi_ended(blend_stage_rows_of_player_exit_shim(BlendStageScoreAddRows026F(cpu.x, cpu.y)));
}

/*
 * @routine 031F:133 BlendStageScoreAddRows031F
 * @run 6B:9133
 * WHAT IT DOES: Shows the score being added on the stage's tile rows while the blend is off (as BlendStageScoreAddRows,
 * 024F:181). Programs 043F and 10DF hold the same bytes.
 * HOW: blend_rows_of_player_at (stage_blend_long.c), the body of 024F:181, at $9133, with BlendStageRowPair031F; its JSR
 * to $95DE and its jumps to $95F3 go to SaveAndMapStageBanks' and RestoreStageBanks' bytes (024F:33F and 024F:354),
 * so their C is theirs.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as BlendStageScoreAddRows.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
ObjArm BlendStageScoreAddRows031F(uint8_t slot, uint8_t y)
{
    return blend_rows_of_player_at(slot, y, &stage_banks_024f, 0x9133, BlendStageRowPair031F);
}

void abi_BlendStageScoreAddRows031F(void)
{
    abi_ended(blend_stage_rows_of_player_exit_shim(BlendStageScoreAddRows031F(cpu.x, cpu.y)));
}

/*
 * @routine 026F:16A BlendStageScoreRows026F
 * @run 6B:916A
 * WHAT IT DOES: Shows the score on the stage's tile rows, a row of changes a step, when it changed (as BlendStageScoreRows,
 * 024F:1AC). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: BlendStageScoreRowsAt (stage_blend_steps.c), the body of 024F:1AC, at $916A, with BlendStageRowChanges026F; its
 * JSR to $92FD and its jumps to $9312 go to SaveAndMapStageBanks' and RestoreStageBanks' bytes (024F:33F and
 * 024F:354), so their C is theirs.
 * WAITS: no.
 * Entry: X the caller's. Exit: as BlendStageScoreRows.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
BlendStepEnd BlendStageScoreRows026F(uint8_t x)
{
    return BlendStageScoreRowsAt(x, &stage_banks_024f, 0x916A, BlendStageRowChanges026F);
}

void abi_BlendStageScoreRows026F(void)
{
    blend_step_exit_shim(BlendStageScoreRows026F(cpu.x));
}

/*
 * @routine 031F:15E BlendStageScoreRows031F
 * @run 6B:915E
 * WHAT IT DOES: Shows the score on the stage's tile rows, a row of changes a step, when it changed (as BlendStageScoreRows,
 * 024F:1AC). Programs 043F and 10DF hold the same bytes.
 * HOW: BlendStageScoreRowsAt (stage_blend_steps.c), the body of 024F:1AC, at $915E, with BlendStageRowChanges031F; its
 * JSR to $95DE and its jumps to $95F3 go to SaveAndMapStageBanks' and RestoreStageBanks' bytes (024F:33F and
 * 024F:354), so their C is theirs.
 * WAITS: no.
 * Entry: X the caller's. Exit: as BlendStageScoreRows.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
BlendStepEnd BlendStageScoreRows031F(uint8_t x)
{
    return BlendStageScoreRowsAt(x, &stage_banks_024f, 0x915E, BlendStageRowChanges031F);
}

void abi_BlendStageScoreRows031F(void)
{
    blend_step_exit_shim(BlendStageScoreRows031F(cpu.x));
}

/*
 * @routine 026F:06F MaskVramRowsRandom026F
 * @run 6B:906F
 * WHAT IT DOES: Masks a region of VRAM with random bits: copied into the mask buffer, masked row by row, and written back (as
 * MaskVramRowsRandom, 024F:0B1). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: MaskVramRowsRandomAt (stage_vram_mask.c), the body of 024F:0B1, at $906F with this program's
 * SetVramAddresses026F, MaskStageRowCopy026F and MaskStageRow026F.
 * WAITS: no.
 * Entry: zTemp10 and zTemp11 the VRAM address. Exit: as MaskVramRowsRandom.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
VramMaskExit MaskVramRowsRandom026F(void)
{
    return MaskVramRowsRandomAt(0x906F, SetVramAddresses026F, MaskStageRowCopy026F, MaskStageRow026F);
}

void abi_MaskVramRowsRandom026F(void)
{
    mask_vram_rows_exit_shim(MaskVramRowsRandom026F());
}

/*
 * @routine 031F:063 MaskVramRowsRandom031F
 * @run 6B:9063
 * WHAT IT DOES: Masks a region of VRAM with random bits: copied into the mask buffer, masked row by row, and written back (as
 * MaskVramRowsRandom, 024F:0B1). Programs 043F and 10DF hold the same bytes.
 * HOW: MaskVramRowsRandomAt (stage_vram_mask.c), the body of 024F:0B1, at $9063 with this program's
 * SetVramAddresses031F, MaskStageRowCopy031F and MaskStageRow031F.
 * WAITS: no.
 * Entry: zTemp10 and zTemp11 the VRAM address. Exit: as MaskVramRowsRandom.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
VramMaskExit MaskVramRowsRandom031F(void)
{
    return MaskVramRowsRandomAt(0x9063, SetVramAddresses031F, MaskStageRowCopy031F, MaskStageRow031F);
}

void abi_MaskVramRowsRandom031F(void)
{
    mask_vram_rows_exit_shim(MaskVramRowsRandom031F());
}

/*
 * @routine 026F:29D ClearStageBitRows026F
 * @run 6B:929D
 * WHAT IT DOES: Clears the stage's bit in four rows of VRAM words (as ClearStageBitRows, 024F:2DF). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: ClearStageBitRowsAt (stage_clear_bits.c), the body of 024F:2DF, at $929D with SetVramAddresses026F and this
 * program's tables at $92E5 and $92ED.
 * WAITS: no.
 * Entry: X kept for the caller; zTemp08-09 the base, zTemp0F the place. Exit: as ClearStageBitRows.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit ClearStageBitRows026F(uint8_t x)
{
    return ClearStageBitRowsAt(x, SetVramAddresses026F, 0x929D, 0x92E5, 0x92ED);
}

void abi_ClearStageBitRows026F(void)
{
    stage_exit_shim(ClearStageBitRows026F(cpu.x));
}

/*
 * @routine 031F:291 ClearStageBitRows031F
 * @run 6B:9291
 * WHAT IT DOES: Clears the stage's bit in four rows of VRAM words (as ClearStageBitRows, 024F:2DF). Programs 043F and 10DF hold the same bytes.
 * HOW: ClearStageBitRowsAt (stage_clear_bits.c), the body of 024F:2DF, at $9291 with SetVramAddresses031F and this
 * program's tables at $92D9 and $92E1.
 * WAITS: no.
 * Entry: X kept for the caller; zTemp08-09 the base, zTemp0F the place. Exit: as ClearStageBitRows.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit ClearStageBitRows031F(uint8_t x)
{
    return ClearStageBitRowsAt(x, SetVramAddresses031F, 0x9291, 0x92D9, 0x92E1);
}

void abi_ClearStageBitRows031F(void)
{
    stage_exit_shim(ClearStageBitRows031F(cpu.x));
}

/*
 * @routine 026F:297 ClearStageBitRowsFour026F
 * @run 6B:9297
 * WHAT IT DOES: Clears the stage's bit in four rows of VRAM words, four times over (as ClearStageBitRowsFour, 024F:2D9). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: ClearStageBitRowsFourAt (stage_clear_bits.c), the body of 024F:2D9, at $9297 with ClearStageBitRows026F, three
 * BSRs and the fall into it.
 * WAITS: no.
 * Entry: as ClearStageBitRows026F. Exit: as the last ClearStageBitRows026F.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit ClearStageBitRowsFour026F(uint8_t x)
{
    return ClearStageBitRowsFourAt(x, 0x9297, ClearStageBitRows026F);
}

void abi_ClearStageBitRowsFour026F(void)
{
    stage_exit_shim(ClearStageBitRowsFour026F(cpu.x));
}

/*
 * @routine 031F:28B ClearStageBitRowsFour031F
 * @run 6B:928B
 * WHAT IT DOES: Clears the stage's bit in four rows of VRAM words, four times over (as ClearStageBitRowsFour, 024F:2D9). Programs 043F and 10DF hold the same bytes.
 * HOW: ClearStageBitRowsFourAt (stage_clear_bits.c), the body of 024F:2D9, at $928B with ClearStageBitRows031F, three
 * BSRs and the fall into it.
 * WAITS: no.
 * Entry: as ClearStageBitRows031F. Exit: as the last ClearStageBitRows031F.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit ClearStageBitRowsFour031F(uint8_t x)
{
    return ClearStageBitRowsFourAt(x, 0x928B, ClearStageBitRows031F);
}

void abi_ClearStageBitRowsFour031F(void)
{
    stage_exit_shim(ClearStageBitRowsFour031F(cpu.x));
}

/*
 * @routine 026F:560 HitStoreHandler026F
 * @run 6B:9560
 * WHAT IT DOES: The hit store handler of the stage programs (as HitStoreHandler, 024F:543): the hit slot's value into
 * the object's hit-taken byte; when the first object's hit-taken byte is empty and the byte at $2745 is 0, the first
 * object's hit-taken byte and health take the object's two stored bytes. Programs 064F, 122F, 1A2F, 1B8F and 1CEF
 * hold the same bytes, and 031F, 043F and 10DF at $97E2 (none of its operands points into it, so it runs the same
 * there).
 * HOW: HitStoreHandlerAt (hit_test.c), the body of 024F:543, with the loaded columns and the gate at $2745.
 * WAITS: no.
 * Entry: X the object, Y the hit slot. Exit: A the last byte loaded, N and Z from it. X, Y, C and V kept.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit HitStoreHandler026F(uint8_t x, uint8_t y)
{
    const HitStoreColumns c = { COL(OBJ->work_n), COL(OBJ->hit_taken), COL(OBJ->work_c), COL(OBJ->work_b),
                                ADDR_OF8(OBJ->health[0]), HitStoreGate };

    return HitStoreHandlerAt(x, y, &c);
}

void abi_HitStoreHandler026F(void)
{
    stage_exit_shim(HitStoreHandler026F(cpu.x, cpu.y));
}

/*
 * @routine 026F:483 HitTestObject026F
 * @run 6B:9483
 * @stop 9542
 * WHAT IT DOES: The stage programs' hit test (as HitTestObject, 024F:466): the object X's hitbox against the other
 * side's slots and, on the first overlap, the handler its kind names in this program's table at $9542. Programs 064F,
 * 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: HitTestObjectAt (hit_test.c), the body of 024F:466, with the table at $9542; every handler it names is the
 * routine loaded at its address (the tables of the programs holding these bytes are their own).
 * WAITS: no.
 * Entry: X the object. Exit: with no hit A the last value, Y $FF, N set; with one, as the handler.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit HitTestObject026F(uint8_t x)
{
    return HitTestObjectAt(x, 0x9542, NULL);
}

void abi_HitTestObject026F(void)
{
    stage_exit_shim(HitTestObject026F(cpu.x));
}

/*
 * @routine 031F:705 HitTestObject031F
 * @run 6B:9705
 * @stop 97C4
 * WHAT IT DOES: The hit test of programs 031F, 043F and 10DF (as HitTestObject, 024F:466), with the table at $97C4.
 * HOW: HitTestObjectAt (hit_test.c), the body of 024F:466, with the table at $97C4; every handler it names is the
 * routine loaded at its address.
 * WAITS: no.
 * Entry, exit: as HitTestObject026F.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit HitTestObject031F(uint8_t x)
{
    return HitTestObjectAt(x, 0x97C4, NULL);
}

void abi_HitTestObject031F(void)
{
    stage_exit_shim(HitTestObject031F(cpu.x));
}

/* The JSR to the hitbox positions loaded at run (ComputeHitboxPositions' bytes in these programs): what it leaves. */
static HitboxEnd loaded_hitbox_positions(uint16_t run)
{
    HitboxEnd end;

    routine_at(run)();
    end.a = cpu.a;
    end.v_seen = true;
    end.v = (cpu.p & P_V) != 0;
    return end;
}

static HitboxEnd hitbox_positions_9430(void)
{
    return loaded_hitbox_positions(0x9430);
}

static HitboxEnd hitbox_positions_96b2(void)
{
    return loaded_hitbox_positions(0x96B2);
}

/* The JSR to the hit test loaded at run, X the object; the registers and flags it leaves are the machine's. */
static StageExit loaded_hit_test(uint16_t run, uint8_t x)
{
    StageExit end = { 0, 0, 0, false, false, 0, EXIT_CPU };

    cpu.x = x;
    routine_at(run)();
    return end;
}

static StageExit hit_test_9483(uint8_t x)
{
    return loaded_hit_test(0x9483, x);
}

static StageExit hit_test_9705(uint8_t x)
{
    return loaded_hit_test(0x9705, x);
}

/*
 * @routine 026F:415 HitTestAllObjects026F
 * @run 6B:9415
 * WHAT IT DOES: One frame of hit testing in the stage programs (as HitTestAllObjects, 024F:3F8): places the first
 * sixteen hitboxes and tests the objects from $57 down to $10, clearing the hit-taken byte of each whose timer is
 * running and counting that timer down. Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
 * HOW: HitTestAllObjectsAt (hit_test.c), the body of 024F:3F8, with the routines loaded at $9430 (the hitbox
 * positions) and $9483 (the hit test) and its JSRs' returns at $9419 and $941E.
 * WAITS: no.
 * Entry: X kept on the stack. Exit: as the last object left it; X kept, C 0.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit HitTestAllObjects026F(uint8_t x)
{
    return HitTestAllObjectsAt(x, hitbox_positions_9430, hit_test_9483, 0x9419, 0x941E);
}

void abi_HitTestAllObjects026F(void)
{
    stage_exit_shim(HitTestAllObjects026F(cpu.x));
}

/*
 * @routine 031F:697 HitTestAllObjects031F
 * @run 6B:9697
 * WHAT IT DOES: One frame of hit testing in programs 031F, 043F and 10DF (as HitTestAllObjects, 024F:3F8).
 * HOW: HitTestAllObjectsAt (hit_test.c), the body of 024F:3F8, with the routines loaded at $96B2 and $9705 and its
 * JSRs' returns at $969B and $96A0.
 * WAITS: no.
 * Entry, exit: as HitTestAllObjects026F.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit HitTestAllObjects031F(uint8_t x)
{
    return HitTestAllObjectsAt(x, hitbox_positions_96b2, hit_test_9705, 0x969B, 0x96A0);
}

void abi_HitTestAllObjects031F(void)
{
    stage_exit_shim(HitTestAllObjects031F(cpu.x));
}

/*
 * @routine 026F:6A1 TerrainTileRead026F
 * @run 6B:96A1
 * WHAT IT DOES: The terrain tile read of the stage programs (as TerrainTileRead, 050F:67F): the attribute of the
 * background tile at a point, 0 if its page has none, and for the attributes $80-$9B the answer of the handler this
 * program's table at $973C names. Programs 064F, 122F, 1A2F, 1B8F, 1CEF hold the same bytes.
 * HOW: TerrainTileReadAt (terrain.c), the body of 050F:67F, with its JSRs' returns $96F0 and $96FD,
 * the table at $973C, and every handler going to the routine loaded at its address.
 * WAITS: no.
 * Entry: zTemp00-01 X, zTemp02-03 Y. Exit: as TerrainTileRead.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
TileRead TerrainTileRead026F(void)
{
    const TileReadBuild build = { 0x96F0, 0x96FD, 0x973C, NULL };

    return TerrainTileReadAt(&build);
}

void abi_TerrainTileRead026F(void)
{
    terrain_read_exit_shim(TerrainTileRead026F());
}

/* The JSR to the read loaded at $96A1: its answer in A, the rest as it leaves the machine. */
static TileRead loaded_tile_read_96a1(void)
{
    TileRead read = { TILE_BY_ORIGINAL, 0, 0, 0, 0, 0, { false, 0, false, false, false } };

    routine_at(0x96A1)();
    read.attribute = cpu.a;
    return read;
}

static const LookupGroup lookup_group_026f = { 0x9608, 0x9631, 0x9681, loaded_tile_read_96a1 };

/* The JSR to the lookup loaded at $95CD: what it leaves is the machine's, its last read the routine's. */
static LookupEnd loaded_lookup_95cd(uint8_t probe, uint8_t object)
{
    LookupEnd end = { false, false, 0, 0, true, { TILE_BY_ORIGINAL, 0, 0, 0, 0, 0, { false, 0, false, false, false } }, 0 };

    cpu.a = probe;
    cpu.x = object;
    routine_at(0x95CD)();
    end.last.attribute = cpu.a;
    return end;
}

/*
 * @routine 026F:5CD TerrainLookup026F
 * @run 6B:95CD
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB): the
 * attribute, and how far into the tile, for each, from this program's probe table at $D2BB.
 * HOW: terrain_lookup_columns (terrain.c), the body of 024F:5AB, with the table at $D2BB, the read loaded at
 * $96A1 and its JSR's return $9608.
 * WAITS: no.
 * Entry: A probe, X object. Exit: as TerrainLookupB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookup026F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xD2BB, &lookup_group_026f);
}

void abi_TerrainLookup026F(void)
{
    lookup_exit_shim(TerrainLookup026F(cpu.a, cpu.x));
}

/*
 * @routine 026F:62D TerrainLookupAlt026F
 * @run 6B:962D
 * WHAT IT DOES: The lookup at $95CD, then a second look at each column that found floor (as TerrainLookupAltB,
 * 024F:60B), from this program's probe table at $D2BB.
 * HOW: terrain_lookup_alt_columns (terrain.c), the body of 024F:60B, with the table at $D2BB, the lookup loaded at
 * $95CD and the read loaded at $96A1, and its JSRs' returns $9631 and $9681.
 * WAITS: no.
 * Entry and exit: as TerrainLookup026F.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookupAlt026F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xD2BB, &lookup_group_026f, loaded_lookup_95cd);
}

void abi_TerrainLookupAlt026F(void)
{
    lookup_exit_shim(TerrainLookupAlt026F(cpu.a, cpu.x));
}

/*
 * @routine 064F:5CD TerrainLookup064F
 * @run 6B:95CD
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB): the
 * attribute, and how far into the tile, for each, from this program's probe table at $B63B.
 * HOW: terrain_lookup_columns (terrain.c), the body of 024F:5AB, with the table at $B63B, the read loaded at
 * $96A1 and its JSR's return $9608.
 * WAITS: no.
 * Entry: A probe, X object. Exit: as TerrainLookupB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookup064F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xB63B, &lookup_group_026f);
}

void abi_TerrainLookup064F(void)
{
    lookup_exit_shim(TerrainLookup064F(cpu.a, cpu.x));
}

/*
 * @routine 064F:62D TerrainLookupAlt064F
 * @run 6B:962D
 * WHAT IT DOES: The lookup at $95CD, then a second look at each column that found floor (as TerrainLookupAltB,
 * 024F:60B), from this program's probe table at $B63B.
 * HOW: terrain_lookup_alt_columns (terrain.c), the body of 024F:60B, with the table at $B63B, the lookup loaded at
 * $95CD and the read loaded at $96A1, and its JSRs' returns $9631 and $9681.
 * WAITS: no.
 * Entry and exit: as TerrainLookup064F.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookupAlt064F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xB63B, &lookup_group_026f, loaded_lookup_95cd);
}

void abi_TerrainLookupAlt064F(void)
{
    lookup_exit_shim(TerrainLookupAlt064F(cpu.a, cpu.x));
}

/*
 * @routine 122F:5CD TerrainLookup122F
 * @run 6B:95CD
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB): the
 * attribute, and how far into the tile, for each, from this program's probe table at $D4C4.
 * HOW: terrain_lookup_columns (terrain.c), the body of 024F:5AB, with the table at $D4C4, the read loaded at
 * $96A1 and its JSR's return $9608.
 * WAITS: no.
 * Entry: A probe, X object. Exit: as TerrainLookupB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookup122F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xD4C4, &lookup_group_026f);
}

void abi_TerrainLookup122F(void)
{
    lookup_exit_shim(TerrainLookup122F(cpu.a, cpu.x));
}

/*
 * @routine 122F:62D TerrainLookupAlt122F
 * @run 6B:962D
 * WHAT IT DOES: The lookup at $95CD, then a second look at each column that found floor (as TerrainLookupAltB,
 * 024F:60B), from this program's probe table at $D4C4.
 * HOW: terrain_lookup_alt_columns (terrain.c), the body of 024F:60B, with the table at $D4C4, the lookup loaded at
 * $95CD and the read loaded at $96A1, and its JSRs' returns $9631 and $9681.
 * WAITS: no.
 * Entry and exit: as TerrainLookup122F.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookupAlt122F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xD4C4, &lookup_group_026f, loaded_lookup_95cd);
}

void abi_TerrainLookupAlt122F(void)
{
    lookup_exit_shim(TerrainLookupAlt122F(cpu.a, cpu.x));
}

/*
 * @routine 1A2F:5CD TerrainLookup1A2F
 * @run 6B:95CD
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB): the
 * attribute, and how far into the tile, for each, from this program's probe table at $BCDF.
 * HOW: terrain_lookup_columns (terrain.c), the body of 024F:5AB, with the table at $BCDF, the read loaded at
 * $96A1 and its JSR's return $9608.
 * WAITS: no.
 * Entry: A probe, X object. Exit: as TerrainLookupB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookup1A2F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xBCDF, &lookup_group_026f);
}

void abi_TerrainLookup1A2F(void)
{
    lookup_exit_shim(TerrainLookup1A2F(cpu.a, cpu.x));
}

/*
 * @routine 1A2F:62D TerrainLookupAlt1A2F
 * @run 6B:962D
 * WHAT IT DOES: The lookup at $95CD, then a second look at each column that found floor (as TerrainLookupAltB,
 * 024F:60B), from this program's probe table at $BCDF.
 * HOW: terrain_lookup_alt_columns (terrain.c), the body of 024F:60B, with the table at $BCDF, the lookup loaded at
 * $95CD and the read loaded at $96A1, and its JSRs' returns $9631 and $9681.
 * WAITS: no.
 * Entry and exit: as TerrainLookup1A2F.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookupAlt1A2F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xBCDF, &lookup_group_026f, loaded_lookup_95cd);
}

void abi_TerrainLookupAlt1A2F(void)
{
    lookup_exit_shim(TerrainLookupAlt1A2F(cpu.a, cpu.x));
}

/*
 * @routine 1B8F:5CD TerrainLookup1B8F
 * @run 6B:95CD
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB): the
 * attribute, and how far into the tile, for each, from this program's probe table at $D85F.
 * HOW: terrain_lookup_columns (terrain.c), the body of 024F:5AB, with the table at $D85F, the read loaded at
 * $96A1 and its JSR's return $9608.
 * WAITS: no.
 * Entry: A probe, X object. Exit: as TerrainLookupB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookup1B8F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xD85F, &lookup_group_026f);
}

void abi_TerrainLookup1B8F(void)
{
    lookup_exit_shim(TerrainLookup1B8F(cpu.a, cpu.x));
}

/*
 * @routine 1B8F:62D TerrainLookupAlt1B8F
 * @run 6B:962D
 * WHAT IT DOES: The lookup at $95CD, then a second look at each column that found floor (as TerrainLookupAltB,
 * 024F:60B), from this program's probe table at $D85F.
 * HOW: terrain_lookup_alt_columns (terrain.c), the body of 024F:60B, with the table at $D85F, the lookup loaded at
 * $95CD and the read loaded at $96A1, and its JSRs' returns $9631 and $9681.
 * WAITS: no.
 * Entry and exit: as TerrainLookup1B8F.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookupAlt1B8F(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xD85F, &lookup_group_026f, loaded_lookup_95cd);
}

void abi_TerrainLookupAlt1B8F(void)
{
    lookup_exit_shim(TerrainLookupAlt1B8F(cpu.a, cpu.x));
}

/*
 * @routine 1CEF:5CD TerrainLookup1CEF
 * @run 6B:95CD
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB): the
 * attribute, and how far into the tile, for each, from this program's probe table at $DA45.
 * HOW: terrain_lookup_columns (terrain.c), the body of 024F:5AB, with the table at $DA45, the read loaded at
 * $96A1 and its JSR's return $9608.
 * WAITS: no.
 * Entry: A probe, X object. Exit: as TerrainLookupB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookup1CEF(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xDA45, &lookup_group_026f);
}

void abi_TerrainLookup1CEF(void)
{
    lookup_exit_shim(TerrainLookup1CEF(cpu.a, cpu.x));
}

/*
 * @routine 1CEF:62D TerrainLookupAlt1CEF
 * @run 6B:962D
 * WHAT IT DOES: The lookup at $95CD, then a second look at each column that found floor (as TerrainLookupAltB,
 * 024F:60B), from this program's probe table at $DA45.
 * HOW: terrain_lookup_alt_columns (terrain.c), the body of 024F:60B, with the table at $DA45, the lookup loaded at
 * $95CD and the read loaded at $96A1, and its JSRs' returns $9631 and $9681.
 * WAITS: no.
 * Entry and exit: as TerrainLookup1CEF.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookupAlt1CEF(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xDA45, &lookup_group_026f, loaded_lookup_95cd);
}

void abi_TerrainLookupAlt1CEF(void)
{
    lookup_exit_shim(TerrainLookupAlt1CEF(cpu.a, cpu.x));
}

/*
 * @routine 0320:123 TerrainTileRead0320
 * @run 6B:9923
 * WHAT IT DOES: The terrain tile read of the stage programs (as TerrainTileRead, 050F:67F): the attribute of the
 * background tile at a point, 0 if its page has none, and for the attributes $80-$9B the answer of the handler this
 * program's table at $99BE names. Programs 0440, 10E0 hold the same bytes.
 * HOW: TerrainTileReadAt (terrain.c), the body of 050F:67F, with its JSRs' returns $9972 and $997F,
 * the table at $99BE, and every handler going to the routine loaded at its address.
 * WAITS: no.
 * Entry: zTemp00-01 X, zTemp02-03 Y. Exit: as TerrainTileRead.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
TileRead TerrainTileRead0320(void)
{
    const TileReadBuild build = { 0x9972, 0x997F, 0x99BE, NULL };

    return TerrainTileReadAt(&build);
}

void abi_TerrainTileRead0320(void)
{
    terrain_read_exit_shim(TerrainTileRead0320());
}

/* The JSR to the read loaded at $9923: its answer in A, the rest as it leaves the machine. */
static TileRead loaded_tile_read_9923(void)
{
    TileRead read = { TILE_BY_ORIGINAL, 0, 0, 0, 0, 0, { false, 0, false, false, false } };

    routine_at(0x9923)();
    read.attribute = cpu.a;
    return read;
}

static const LookupGroup lookup_group_0320 = { 0x988A, 0x98B3, 0x9903, loaded_tile_read_9923 };

/* The JSR to the lookup loaded at $984F: what it leaves is the machine's, its last read the routine's. */
static LookupEnd loaded_lookup_984f(uint8_t probe, uint8_t object)
{
    LookupEnd end = { false, false, 0, 0, true, { TILE_BY_ORIGINAL, 0, 0, 0, 0, 0, { false, 0, false, false, false } }, 0 };

    cpu.a = probe;
    cpu.x = object;
    routine_at(0x984F)();
    end.last.attribute = cpu.a;
    return end;
}

/*
 * @routine 0320:04F TerrainLookup0320
 * @run 6B:984F
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB): the
 * attribute, and how far into the tile, for each, from this program's probe table at $BDF5.
 * HOW: terrain_lookup_columns (terrain.c), the body of 024F:5AB, with the table at $BDF5, the read loaded at
 * $9923 and its JSR's return $988A.
 * WAITS: no.
 * Entry: A probe, X object. Exit: as TerrainLookupB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookup0320(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xBDF5, &lookup_group_0320);
}

void abi_TerrainLookup0320(void)
{
    lookup_exit_shim(TerrainLookup0320(cpu.a, cpu.x));
}

/*
 * @routine 0320:0AF TerrainLookupAlt0320
 * @run 6B:98AF
 * WHAT IT DOES: The lookup at $984F, then a second look at each column that found floor (as TerrainLookupAltB,
 * 024F:60B), from this program's probe table at $BDF5.
 * HOW: terrain_lookup_alt_columns (terrain.c), the body of 024F:60B, with the table at $BDF5, the lookup loaded at
 * $984F and the read loaded at $9923, and its JSRs' returns $98B3 and $9903.
 * WAITS: no.
 * Entry and exit: as TerrainLookup0320.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookupAlt0320(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xBDF5, &lookup_group_0320, loaded_lookup_984f);
}

void abi_TerrainLookupAlt0320(void)
{
    lookup_exit_shim(TerrainLookupAlt0320(cpu.a, cpu.x));
}

/*
 * @routine 0440:04F TerrainLookup0440
 * @run 6B:984F
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB): the
 * attribute, and how far into the tile, for each, from this program's probe table at $C1C6.
 * HOW: terrain_lookup_columns (terrain.c), the body of 024F:5AB, with the table at $C1C6, the read loaded at
 * $9923 and its JSR's return $988A.
 * WAITS: no.
 * Entry: A probe, X object. Exit: as TerrainLookupB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookup0440(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xC1C6, &lookup_group_0320);
}

void abi_TerrainLookup0440(void)
{
    lookup_exit_shim(TerrainLookup0440(cpu.a, cpu.x));
}

/*
 * @routine 0440:0AF TerrainLookupAlt0440
 * @run 6B:98AF
 * WHAT IT DOES: The lookup at $984F, then a second look at each column that found floor (as TerrainLookupAltB,
 * 024F:60B), from this program's probe table at $C1C6.
 * HOW: terrain_lookup_alt_columns (terrain.c), the body of 024F:60B, with the table at $C1C6, the lookup loaded at
 * $984F and the read loaded at $9923, and its JSRs' returns $98B3 and $9903.
 * WAITS: no.
 * Entry and exit: as TerrainLookup0440.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookupAlt0440(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xC1C6, &lookup_group_0320, loaded_lookup_984f);
}

void abi_TerrainLookupAlt0440(void)
{
    lookup_exit_shim(TerrainLookupAlt0440(cpu.a, cpu.x));
}

/*
 * @routine 10E0:04F TerrainLookup10E0
 * @run 6B:984F
 * WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB): the
 * attribute, and how far into the tile, for each, from this program's probe table at $C55F.
 * HOW: terrain_lookup_columns (terrain.c), the body of 024F:5AB, with the table at $C55F, the read loaded at
 * $9923 and its JSR's return $988A.
 * WAITS: no.
 * Entry: A probe, X object. Exit: as TerrainLookupB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookup10E0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xC55F, &lookup_group_0320);
}

void abi_TerrainLookup10E0(void)
{
    lookup_exit_shim(TerrainLookup10E0(cpu.a, cpu.x));
}

/*
 * @routine 10E0:0AF TerrainLookupAlt10E0
 * @run 6B:98AF
 * WHAT IT DOES: The lookup at $984F, then a second look at each column that found floor (as TerrainLookupAltB,
 * 024F:60B), from this program's probe table at $C55F.
 * HOW: terrain_lookup_alt_columns (terrain.c), the body of 024F:60B, with the table at $C55F, the lookup loaded at
 * $984F and the read loaded at $9923, and its JSRs' returns $98B3 and $9903.
 * WAITS: no.
 * Entry and exit: as TerrainLookup10E0.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
LookupEnd TerrainLookupAlt10E0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xC55F, &lookup_group_0320, loaded_lookup_984f);
}

void abi_TerrainLookupAlt10E0(void)
{
    lookup_exit_shim(TerrainLookupAlt10E0(cpu.a, cpu.x));
}

/*
 * @routine 026F:774 TileStepC026F
 * @run 6B:9774
 * WHAT IT DOES: A slope tile: moves the point's row by the table at $980F on its column (as TileStepC, 050F:752).
 * HOW: TileStepAt (terrain.c), the body of 050F:752, with this program's table at $980F.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileStepC026F(void)
{
    return TileStepAt(0x980F);
}

void abi_TileStepC026F(void)
{
    handler_exit_shim(TileStepC026F());
}

/*
 * @routine 026F:781 TileRowA026F
 * @run 6B:9781
 * WHAT IT DOES: A slope tile: the point's new row from the eight by eight table at $9817 on its row and column (as TileRowA, 050F:75F).
 * HOW: TileRowAt (terrain.c), the body of 050F:75F, with this program's table at $9817.
 * WAITS: no.
 * Entry and exit: as TileRowA.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowA026F(void)
{
    return TileRowAt(0x9817);
}

void abi_TileRowA026F(void)
{
    handler_exit_shim(TileRowA026F());
}

/*
 * @routine 026F:792 TileRowB026F
 * @run 6B:9792
 * WHAT IT DOES: As TileRowB (050F:770), with the table at $9857.
 * HOW: TileRowAt (terrain.c), the body of 050F:770, with this program's table at $9857.
 * WAITS: no.
 * Entry and exit: as TileRowB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowB026F(void)
{
    return TileRowAt(0x9857);
}

void abi_TileRowB026F(void)
{
    handler_exit_shim(TileRowB026F());
}

/*
 * @routine 026F:7A3 TileHalfStepA026F
 * @run 6B:97A3
 * WHAT IT DOES: The tiles $87-$8A: a step from the pairs at $97FF that differs between the tile's halves (as TileHalfStepA, 050F:781).
 * HOW: TileHalfStepAt (terrain.c), the body of 050F:781, with this program's table at $97FF.
 * WAITS: no.
 * Entry and exit: as TileHalfStepA.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileHalfStepA026F(uint8_t tile)
{
    return TileHalfStepAt(0x97FF, 0x83, tile, true);
}

void abi_TileHalfStepA026F(void)
{
    handler_exit_shim(TileHalfStepA026F(cpu.a));
}

/*
 * @routine 026F:7B7 TileHalfStepB026F
 * @run 6B:97B7
 * WHAT IT DOES: The tiles $8C-$8F: a step from the pairs at $9807 that differs between the tile's halves (as TileHalfStepB, 050F:795).
 * HOW: TileHalfStepAt (terrain.c), the body of 050F:795, with this program's table at $9807.
 * WAITS: no.
 * Entry and exit: as TileHalfStepB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileHalfStepB026F(uint8_t tile)
{
    return TileHalfStepAt(0x9807, 0x8C, tile, false);
}

void abi_TileHalfStepB026F(void)
{
    handler_exit_shim(TileHalfStepB026F(cpu.a));
}

/*
 * @routine 026F:7D0 TileStepD026F
 * @run 6B:97D0
 * WHAT IT DOES: As TileStepD (050F:7AE), with the table at $9897.
 * HOW: TileStepAt (terrain.c), the body of 050F:7AE, with this program's table at $9897.
 * WAITS: no.
 * Entry and exit: as TileStepD.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileStepD026F(void)
{
    return TileStepAt(0x9897);
}

void abi_TileStepD026F(void)
{
    handler_exit_shim(TileStepD026F());
}

/*
 * @routine 026F:7DD TileRowC026F
 * @run 6B:97DD
 * WHAT IT DOES: As TileRowC (050F:7BB), with the table at $989F.
 * HOW: TileRowAt (terrain.c), the body of 050F:7BB, with this program's table at $989F.
 * WAITS: no.
 * Entry and exit: as TileRowC.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowC026F(void)
{
    return TileRowAt(0x989F);
}

void abi_TileRowC026F(void)
{
    handler_exit_shim(TileRowC026F());
}

/*
 * @routine 026F:7EE TileRowD026F
 * @run 6B:97EE
 * WHAT IT DOES: As TileRowD (050F:7CC), with the table at $98DF.
 * HOW: TileRowAt (terrain.c), the body of 050F:7CC, with this program's table at $98DF.
 * WAITS: no.
 * Entry and exit: as TileRowD.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowD026F(void)
{
    return TileRowAt(0x98DF);
}

void abi_TileRowD026F(void)
{
    handler_exit_shim(TileRowD026F());
}

/*
 * @routine 0320:1F6 TileStepC0320
 * @run 6B:99F6
 * WHAT IT DOES: A slope tile: moves the point's row by the table at $9A91 on its column (as TileStepC, 050F:752).
 * HOW: TileStepAt (terrain.c), the body of 050F:752, with this program's table at $9A91.
 * WAITS: no.
 * Entry and exit: as TileStepC.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileStepC0320(void)
{
    return TileStepAt(0x9A91);
}

void abi_TileStepC0320(void)
{
    handler_exit_shim(TileStepC0320());
}

/*
 * @routine 0320:203 TileRowA0320
 * @run 6B:9A03
 * WHAT IT DOES: A slope tile: the point's new row from the eight by eight table at $9A99 on its row and column (as TileRowA, 050F:75F).
 * HOW: TileRowAt (terrain.c), the body of 050F:75F, with this program's table at $9A99.
 * WAITS: no.
 * Entry and exit: as TileRowA.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowA0320(void)
{
    return TileRowAt(0x9A99);
}

void abi_TileRowA0320(void)
{
    handler_exit_shim(TileRowA0320());
}

/*
 * @routine 0320:214 TileRowB0320
 * @run 6B:9A14
 * WHAT IT DOES: As TileRowB (050F:770), with the table at $9AD9.
 * HOW: TileRowAt (terrain.c), the body of 050F:770, with this program's table at $9AD9.
 * WAITS: no.
 * Entry and exit: as TileRowB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowB0320(void)
{
    return TileRowAt(0x9AD9);
}

void abi_TileRowB0320(void)
{
    handler_exit_shim(TileRowB0320());
}

/*
 * @routine 0320:225 TileHalfStepA0320
 * @run 6B:9A25
 * WHAT IT DOES: The tiles $87-$8A: a step from the pairs at $9A81 that differs between the tile's halves (as TileHalfStepA, 050F:781).
 * HOW: TileHalfStepAt (terrain.c), the body of 050F:781, with this program's table at $9A81.
 * WAITS: no.
 * Entry and exit: as TileHalfStepA.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileHalfStepA0320(uint8_t tile)
{
    return TileHalfStepAt(0x9A81, 0x83, tile, true);
}

void abi_TileHalfStepA0320(void)
{
    handler_exit_shim(TileHalfStepA0320(cpu.a));
}

/*
 * @routine 0320:239 TileHalfStepB0320
 * @run 6B:9A39
 * WHAT IT DOES: The tiles $8C-$8F: a step from the pairs at $9A89 that differs between the tile's halves (as TileHalfStepB, 050F:795).
 * HOW: TileHalfStepAt (terrain.c), the body of 050F:795, with this program's table at $9A89.
 * WAITS: no.
 * Entry and exit: as TileHalfStepB.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileHalfStepB0320(uint8_t tile)
{
    return TileHalfStepAt(0x9A89, 0x8C, tile, false);
}

void abi_TileHalfStepB0320(void)
{
    handler_exit_shim(TileHalfStepB0320(cpu.a));
}

/*
 * @routine 0320:252 TileStepD0320
 * @run 6B:9A52
 * WHAT IT DOES: As TileStepD (050F:7AE), with the table at $9B19.
 * HOW: TileStepAt (terrain.c), the body of 050F:7AE, with this program's table at $9B19.
 * WAITS: no.
 * Entry and exit: as TileStepD.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileStepD0320(void)
{
    return TileStepAt(0x9B19);
}

void abi_TileStepD0320(void)
{
    handler_exit_shim(TileStepD0320());
}

/*
 * @routine 0320:25F TileRowC0320
 * @run 6B:9A5F
 * WHAT IT DOES: As TileRowC (050F:7BB), with the table at $9B21.
 * HOW: TileRowAt (terrain.c), the body of 050F:7BB, with this program's table at $9B21.
 * WAITS: no.
 * Entry and exit: as TileRowC.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowC0320(void)
{
    return TileRowAt(0x9B21);
}

void abi_TileRowC0320(void)
{
    handler_exit_shim(TileRowC0320());
}

/*
 * @routine 0320:270 TileRowD0320
 * @run 6B:9A70
 * WHAT IT DOES: As TileRowD (050F:7CC), with the table at $9B61.
 * HOW: TileRowAt (terrain.c), the body of 050F:7CC, with this program's table at $9B61.
 * WAITS: no.
 * Entry and exit: as TileRowD.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowD0320(void)
{
    return TileRowAt(0x9B61);
}

void abi_TileRowD0320(void)
{
    handler_exit_shim(TileRowD0320());
}

/*
 * @routine 0270:11F TileStepTwice026F
 * @run 6B:991F
 * WHAT IT DOES: As TileStepTwice (0510:0FD), with this program's tables.
 * HOW: TileStepTwiceAt (terrain.c), the body of 0510:0FD, with the offsets table at $998A.
 * WAITS: no.
 * Entry: as TileStepTwice.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileStepTwice026F(void)
{
    return TileStepTwiceAt(0x998A);
}

void abi_TileStepTwice026F(void)
{
    handler_exit_shim(TileStepTwice026F());
}

/*
 * @routine 0270:128 TileRowOffset026F
 * @run 6B:9928
 * WHAT IT DOES: As TileRowOffset (0510:106), with this program's tables.
 * HOW: TileRowOffsetAt (terrain.c), the body of 0510:106, with the tables at $998A and $9952.
 * WAITS: no.
 * Entry: as TileRowOffset.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowOffset026F(void)
{
    return TileRowOffsetAt(0x998A, 0x9952);
}

void abi_TileRowOffset026F(void)
{
    handler_exit_shim(TileRowOffset026F());
}

/*
 * @routine 0270:136 TileStepOffset026F
 * @run 6B:9936
 * WHAT IT DOES: As TileStepOffset (0510:114), with this program's tables.
 * HOW: TileStepOffsetAt (terrain.c), the body of 0510:114, with the offsets table at $998A.
 * WAITS: no.
 * Entry: as TileStepOffset.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileStepOffset026F(void)
{
    return TileStepOffsetAt(0x998A);
}

void abi_TileStepOffset026F(void)
{
    handler_exit_shim(TileStepOffset026F());
}

/*
 * @routine 0270:141 TileRowE026F
 * @run 6B:9941
 * WHAT IT DOES: As TileRowE (0510:11F), with this program's tables.
 * HOW: TileRowAt (terrain.c), the body of 0510:11F, with the table at $9952.
 * WAITS: no.
 * Entry: as TileRowE.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowE026F(void)
{
    return TileRowAt(0x9952);
}

void abi_TileRowE026F(void)
{
    handler_exit_shim(TileRowE026F());
}

/*
 * @routine 0320:3A1 TileStepTwice0320
 * @run 6B:9BA1
 * WHAT IT DOES: As TileStepTwice (0510:0FD), with this program's tables.
 * HOW: TileStepTwiceAt (terrain.c), the body of 0510:0FD, with the offsets table at $9C0C.
 * WAITS: no.
 * Entry: as TileStepTwice.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileStepTwice0320(void)
{
    return TileStepTwiceAt(0x9C0C);
}

void abi_TileStepTwice0320(void)
{
    handler_exit_shim(TileStepTwice0320());
}

/*
 * @routine 0320:3AA TileRowOffset0320
 * @run 6B:9BAA
 * WHAT IT DOES: As TileRowOffset (0510:106), with this program's tables.
 * HOW: TileRowOffsetAt (terrain.c), the body of 0510:106, with the tables at $9C0C and $9BD4.
 * WAITS: no.
 * Entry: as TileRowOffset.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowOffset0320(void)
{
    return TileRowOffsetAt(0x9C0C, 0x9BD4);
}

void abi_TileRowOffset0320(void)
{
    handler_exit_shim(TileRowOffset0320());
}

/*
 * @routine 0320:3B8 TileStepOffset0320
 * @run 6B:9BB8
 * WHAT IT DOES: As TileStepOffset (0510:114), with this program's tables.
 * HOW: TileStepOffsetAt (terrain.c), the body of 0510:114, with the offsets table at $9C0C.
 * WAITS: no.
 * Entry: as TileStepOffset.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileStepOffset0320(void)
{
    return TileStepOffsetAt(0x9C0C);
}

void abi_TileStepOffset0320(void)
{
    handler_exit_shim(TileStepOffset0320());
}

/*
 * @routine 0320:3C3 TileRowE0320
 * @run 6B:9BC3
 * WHAT IT DOES: As TileRowE (0510:11F), with this program's tables.
 * HOW: TileRowAt (terrain.c), the body of 0510:11F, with the table at $9BD4.
 * WAITS: no.
 * Entry: as TileRowE.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
HandlerExit TileRowE0320(void)
{
    return TileRowAt(0x9BD4);
}

void abi_TileRowE0320(void)
{
    handler_exit_shim(TileRowE0320());
}
