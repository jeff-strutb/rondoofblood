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
#include "stage_programs_census.h"

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
