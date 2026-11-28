/*
 * Routines of the stage programs no read the game makes loads, at sectors 026D, 031D, 043D, 064D, 10DD, 122D, 1A2D,
 * 1B8D and 1CED: other builds of the stage overlay at 024D. Each routine here is a loaded stage routine with its own
 * program's operands; its C calls the loaded routine's body with them.
 */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"
#include "enemy_objects.h"
#include "stage_regs.h"
#include "stage_programs_census.h"

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
 * HOW: As SetStageTileVramAddress (024E:687): zTemp04 less bit 0 indexes the table; a tail JMP to SetVramAddresses026F.
 * WAITS: no.
 * Entry: zTemp04. Exit: A 2, N and Z from it, X the address low, Y the high.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
uint16_t SetStageTileVramAddress026E(void)
{
    uint8_t index = (uint8_t)(RD8(ZP->temp04) & 0xFE);
    uint8_t lo = rd8((uint16_t)(0x8E4C + index));
    uint8_t hi = rd8((uint16_t)(0x8E4D + index));

    SetVramAddresses026F(lo, hi);
    return (uint16_t)(lo | hi << 8);
}

void abi_SetStageTileVramAddress026E(void)
{
    uint16_t address = SetStageTileVramAddress026E();

    cpu.x = (uint8_t)address;
    cpu.y = (uint8_t)(address >> 8);
    cpu.a = 2;
    abi_nz(2);
}

/*
 * @routine 026E:3F9 SetHealthBarAddress026E
 * @run 6B:8BF9
 * WHAT IT DOES: While zField99 is not 0, sets the VRAM address of the bar at the cell $5E less it counts.
 * HOW: As SetHealthBarAddress (024E:443), calling SetVramAddresses026F.
 * WAITS: no.
 * Entry: nothing. Exit, zField99 0: A 0, Z set. Else: A 2, X and Y the address, C and V from the last add.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit SetHealthBarAddress026E(void)
{
    return health_bar_address(SetVramAddresses026F, 0x8C21);
}

void abi_SetHealthBarAddress026E(void)
{
    stage_exit_shim(SetHealthBarAddress026E());
}

/*
 * @routine 031E:63D SetStageTileVramAddress031E
 * @run 6B:8E3D
 * WHAT IT DOES: Sets the VRAM address of the tile pair zTemp04 names, from this program's table of addresses.
 * HOW: As SetStageTileVramAddress (024E:687): zTemp04 less bit 0 indexes the table; a tail JMP to SetVramAddresses031F.
 * WAITS: no.
 * Entry: zTemp04. Exit: A 2, N and Z from it, X the address low, Y the high.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
uint16_t SetStageTileVramAddress031E(void)
{
    uint8_t index = (uint8_t)(RD8(ZP->temp04) & 0xFE);
    uint8_t lo = rd8((uint16_t)(0x8E4C + index));
    uint8_t hi = rd8((uint16_t)(0x8E4D + index));

    SetVramAddresses031F(lo, hi);
    return (uint16_t)(lo | hi << 8);
}

void abi_SetStageTileVramAddress031E(void)
{
    uint16_t address = SetStageTileVramAddress031E();

    cpu.x = (uint8_t)address;
    cpu.y = (uint8_t)(address >> 8);
    cpu.a = 2;
    abi_nz(2);
}

/*
 * @routine 031E:3F9 SetHealthBarAddress031E
 * @run 6B:8BF9
 * WHAT IT DOES: While zField99 is not 0, sets the VRAM address of the bar at the cell $5E less it counts.
 * HOW: As SetHealthBarAddress (024E:443), calling SetVramAddresses031F.
 * WAITS: no.
 * Entry: nothing. Exit, zField99 0: A 0, Z set. Else: A 2, X and Y the address, C and V from the last add.
 * Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
 */
StageExit SetHealthBarAddress031E(void)
{
    return health_bar_address(SetVramAddresses031F, 0x8C21);
}

void abi_SetHealthBarAddress031E(void)
{
    stage_exit_shim(SetHealthBarAddress031E());
}
