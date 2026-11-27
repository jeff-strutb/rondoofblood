/*
 * The terrain lookups of the thirteen stage overlays of 05BF's group, at $98BD and $991D: the same instructions as 024F's
 * group's TerrainLookup and TerrainLookupAlt at $95AB and $960B (terrain.c), with the tile read ProbeTileRead9991 at
 * $9991, so they share that body (terrain_lookup_columns, terrain_lookup_alt_columns) with this group's returns. Each
 * overlay differs only in the address of its probe table, found by comparing each overlay's operand on the disc, so
 * each has its own routine naming its table.
 *
 * The first reads, for the object, four columns' offsets from the probe's table entry and reads the map at each point,
 * leaving the answer, the depth in X and the depth in Y of each in TileHit, TileDepthX and TileDepthY; a column marked
 * $80 is not read. The second runs the first, then looks again eight pixels above each column whose answer has no bit
 * 7 and some of the bits $A1, and clears the answer's bits but $5E when that look finds bit 0.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "player_objects.h"

/*
 * @routine 05C0:0BD ProbeBoxes05C0
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 05BF: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $BCEC and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes05C0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xBCEC, &lookup_group_05bf);
}

void abi_ProbeBoxes05C0(void)
{
    lookup_exit_shim(ProbeBoxes05C0(cpu.a, cpu.x));
}

/*
 * @routine 05C0:11D ProbeBoxesFlagged05C0
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 05BF: ProbeBoxes05C0, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $BCEC.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes05C0.
 */
LookupEnd ProbeBoxesFlagged05C0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xBCEC, &lookup_group_05bf, ProbeBoxes05C0);
}

void abi_ProbeBoxesFlagged05C0(void)
{
    lookup_exit_shim(ProbeBoxesFlagged05C0(cpu.a, cpu.x));
}

/*
 * @routine 0720:0BD ProbeBoxes0720
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 071F: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $C0D4 and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes0720(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xC0D4, &lookup_group_05bf);
}

void abi_ProbeBoxes0720(void)
{
    lookup_exit_shim(ProbeBoxes0720(cpu.a, cpu.x));
}

/*
 * @routine 0720:11D ProbeBoxesFlagged0720
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 071F: ProbeBoxes0720, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $C0D4.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes0720.
 */
LookupEnd ProbeBoxesFlagged0720(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xC0D4, &lookup_group_05bf, ProbeBoxes0720);
}

void abi_ProbeBoxesFlagged0720(void)
{
    lookup_exit_shim(ProbeBoxesFlagged0720(cpu.a, cpu.x));
}

/*
 * @routine 0880:0BD ProbeBoxes0880
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 087F: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $C251 and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes0880(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xC251, &lookup_group_05bf);
}

void abi_ProbeBoxes0880(void)
{
    lookup_exit_shim(ProbeBoxes0880(cpu.a, cpu.x));
}

/*
 * @routine 0880:11D ProbeBoxesFlagged0880
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 087F: ProbeBoxes0880, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $C251.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes0880.
 */
LookupEnd ProbeBoxesFlagged0880(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xC251, &lookup_group_05bf, ProbeBoxes0880);
}

void abi_ProbeBoxesFlagged0880(void)
{
    lookup_exit_shim(ProbeBoxesFlagged0880(cpu.a, cpu.x));
}

/*
 * @routine 09E0:0BD ProbeBoxes09E0
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 09DF: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $C186 and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes09E0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xC186, &lookup_group_05bf);
}

void abi_ProbeBoxes09E0(void)
{
    lookup_exit_shim(ProbeBoxes09E0(cpu.a, cpu.x));
}

/*
 * @routine 09E0:11D ProbeBoxesFlagged09E0
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 09DF: ProbeBoxes09E0, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $C186.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes09E0.
 */
LookupEnd ProbeBoxesFlagged09E0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xC186, &lookup_group_05bf, ProbeBoxes09E0);
}

void abi_ProbeBoxesFlagged09E0(void)
{
    lookup_exit_shim(ProbeBoxesFlagged09E0(cpu.a, cpu.x));
}

/*
 * @routine 0B40:0BD ProbeBoxes0B40
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 0B3F: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $B38E and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes0B40(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xB38E, &lookup_group_05bf);
}

void abi_ProbeBoxes0B40(void)
{
    lookup_exit_shim(ProbeBoxes0B40(cpu.a, cpu.x));
}

/*
 * @routine 0B40:11D ProbeBoxesFlagged0B40
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 0B3F: ProbeBoxes0B40, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $B38E.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes0B40.
 */
LookupEnd ProbeBoxesFlagged0B40(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xB38E, &lookup_group_05bf, ProbeBoxes0B40);
}

void abi_ProbeBoxesFlagged0B40(void)
{
    lookup_exit_shim(ProbeBoxesFlagged0B40(cpu.a, cpu.x));
}

/*
 * @routine 0CA0:0BD ProbeBoxes0CA0
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 0C9F: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $C1A5 and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes0CA0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xC1A5, &lookup_group_05bf);
}

void abi_ProbeBoxes0CA0(void)
{
    lookup_exit_shim(ProbeBoxes0CA0(cpu.a, cpu.x));
}

/*
 * @routine 0CA0:11D ProbeBoxesFlagged0CA0
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 0C9F: ProbeBoxes0CA0, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $C1A5.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes0CA0.
 */
LookupEnd ProbeBoxesFlagged0CA0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xC1A5, &lookup_group_05bf, ProbeBoxes0CA0);
}

void abi_ProbeBoxesFlagged0CA0(void)
{
    lookup_exit_shim(ProbeBoxesFlagged0CA0(cpu.a, cpu.x));
}

/*
 * @routine 0E00:0BD ProbeBoxes0E00
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 0DFF: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $AF23 and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes0E00(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xAF23, &lookup_group_05bf);
}

void abi_ProbeBoxes0E00(void)
{
    lookup_exit_shim(ProbeBoxes0E00(cpu.a, cpu.x));
}

/*
 * @routine 0E00:11D ProbeBoxesFlagged0E00
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 0DFF: ProbeBoxes0E00, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $AF23.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes0E00.
 */
LookupEnd ProbeBoxesFlagged0E00(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xAF23, &lookup_group_05bf, ProbeBoxes0E00);
}

void abi_ProbeBoxesFlagged0E00(void)
{
    lookup_exit_shim(ProbeBoxesFlagged0E00(cpu.a, cpu.x));
}

/*
 * @routine 0F60:0BD ProbeBoxes0F60
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 0F5F: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $B01F and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes0F60(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xB01F, &lookup_group_05bf);
}

void abi_ProbeBoxes0F60(void)
{
    lookup_exit_shim(ProbeBoxes0F60(cpu.a, cpu.x));
}

/*
 * @routine 0F60:11D ProbeBoxesFlagged0F60
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 0F5F: ProbeBoxes0F60, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $B01F.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes0F60.
 */
LookupEnd ProbeBoxesFlagged0F60(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xB01F, &lookup_group_05bf, ProbeBoxes0F60);
}

void abi_ProbeBoxesFlagged0F60(void)
{
    lookup_exit_shim(ProbeBoxesFlagged0F60(cpu.a, cpu.x));
}

/*
 * @routine 10C0:0BD ProbeBoxes10C0
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 10BF: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $C511 and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes10C0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xC511, &lookup_group_05bf);
}

void abi_ProbeBoxes10C0(void)
{
    lookup_exit_shim(ProbeBoxes10C0(cpu.a, cpu.x));
}

/*
 * @routine 10C0:11D ProbeBoxesFlagged10C0
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 10BF: ProbeBoxes10C0, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $C511.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes10C0.
 */
LookupEnd ProbeBoxesFlagged10C0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xC511, &lookup_group_05bf, ProbeBoxes10C0);
}

void abi_ProbeBoxesFlagged10C0(void)
{
    lookup_exit_shim(ProbeBoxesFlagged10C0(cpu.a, cpu.x));
}

/*
 * @routine 12D0:0BD ProbeBoxes12D0
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 12CF: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $D09B and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes12D0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xD09B, &lookup_group_05bf);
}

void abi_ProbeBoxes12D0(void)
{
    lookup_exit_shim(ProbeBoxes12D0(cpu.a, cpu.x));
}

/*
 * @routine 12D0:11D ProbeBoxesFlagged12D0
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 12CF: ProbeBoxes12D0, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $D09B.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes12D0.
 */
LookupEnd ProbeBoxesFlagged12D0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xD09B, &lookup_group_05bf, ProbeBoxes12D0);
}

void abi_ProbeBoxesFlagged12D0(void)
{
    lookup_exit_shim(ProbeBoxesFlagged12D0(cpu.a, cpu.x));
}

/*
 * @routine 13F0:0BD ProbeBoxes13F0
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 13EF: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $C4B9 and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes13F0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xC4B9, &lookup_group_05bf);
}

void abi_ProbeBoxes13F0(void)
{
    lookup_exit_shim(ProbeBoxes13F0(cpu.a, cpu.x));
}

/*
 * @routine 13F0:11D ProbeBoxesFlagged13F0
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 13EF: ProbeBoxes13F0, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $C4B9.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes13F0.
 */
LookupEnd ProbeBoxesFlagged13F0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xC4B9, &lookup_group_05bf, ProbeBoxes13F0);
}

void abi_ProbeBoxesFlagged13F0(void)
{
    lookup_exit_shim(ProbeBoxesFlagged13F0(cpu.a, cpu.x));
}

/*
 * @routine 16C0:0BD ProbeBoxes16C0
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 16BF: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $BDA9 and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes16C0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xBDA9, &lookup_group_05bf);
}

void abi_ProbeBoxes16C0(void)
{
    lookup_exit_shim(ProbeBoxes16C0(cpu.a, cpu.x));
}

/*
 * @routine 16C0:11D ProbeBoxesFlagged16C0
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 16BF: ProbeBoxes16C0, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $BDA9.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes16C0.
 */
LookupEnd ProbeBoxesFlagged16C0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xBDA9, &lookup_group_05bf, ProbeBoxes16C0);
}

void abi_ProbeBoxesFlagged16C0(void)
{
    lookup_exit_shim(ProbeBoxesFlagged16C0(cpu.a, cpu.x));
}

/*
 * @routine 18D0:0BD ProbeBoxes18D0
 * @run 6B:98BD
 * WHAT IT DOES: TerrainLookup in the overlay at sector 18CF: reads the terrain at the four columns of a probe
 * around an object, the attribute and how far into the tile for each; a column marked as absent reads as nothing.
 * HOW: The shared lookup body with this overlay's probe table at $C285 and 05BF's group's ProbeTileRead9991.
 * WAITS: no.
 * Entry: A probe, X object. Exit: A 4, N 0, Z 1, C 1, Y 16, X kept, V from the last column's read or add.
 * Clobbers zTemp00-0D.
 */
LookupEnd ProbeBoxes18D0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_columns(probe, object, 0xC285, &lookup_group_05bf);
}

void abi_ProbeBoxes18D0(void)
{
    lookup_exit_shim(ProbeBoxes18D0(cpu.a, cpu.x));
}

/*
 * @routine 18D0:11D ProbeBoxesFlagged18D0
 * @run 6B:991D
 * WHAT IT DOES: TerrainLookupAlt in the overlay at sector 18CF: ProbeBoxes18D0, then a second look eight pixels
 * above each column that found floor; if that too is solid the floor bits come off the column's answer.
 * HOW: The shared second-look body with the same table at $C285.
 * WAITS: no.
 * Entry, exit: as ProbeBoxes18D0.
 */
LookupEnd ProbeBoxesFlagged18D0(uint8_t probe, uint8_t object)
{
    return terrain_lookup_alt_columns(probe, object, 0xC285, &lookup_group_05bf, ProbeBoxes18D0);
}

void abi_ProbeBoxesFlagged18D0(void)
{
    lookup_exit_shim(ProbeBoxesFlagged18D0(cpu.a, cpu.x));
}
