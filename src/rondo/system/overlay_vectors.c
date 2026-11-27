/*
 * Overlay entry jumps: each overlay begins with a table of JMPs that the kernel and the dispatchers call
 * through. Each is one JMP; its registers and flags are its target's. The same bytes stand in other overlays
 * (each stage's copy), and the target is that overlay's own routine, so the C goes to the C of the routine
 * loaded at the target (run_routine), not to one overlay's.
 */
#include "machine.h"

/*
 * @routine 050D:000 StageVec8000_050D
 * @run 6B:8000
 * WHAT IT DOES: Goes to the stage overlay's frame at $8018.
 * HOW: One jump.
 * WAITS: where the routine at $8018 waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $8018.
 */
void abi_StageVec8000_050D(void)
{
    run_routine(0x8018);
}

/*
 * @routine 050D:006 StageVec8006_050D
 * @run 6B:8006
 * WHAT IT DOES: Goes to TerrainTileRead.
 * HOW: One jump.
 * WAITS: where the routine at $967F waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $967F.
 */
void abi_StageVec8006_050D(void)
{
    run_routine(0x967F);
}

/*
 * @routine 050D:00C StageVec800C_050D
 * @run 6B:800C
 * WHAT IT DOES: Goes to TerrainLookup.
 * HOW: One jump.
 * WAITS: where the routine at $95AB waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $95AB.
 */
void abi_StageVec800C_050D(void)
{
    run_routine(0x95AB);
}

/*
 * @routine 050D:00F StageVec800F_050D
 * @run 6B:800F
 * WHAT IT DOES: Goes to TerrainLookupAlt.
 * HOW: One jump.
 * WAITS: where the routine at $960B waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $960B.
 */
void abi_StageVec800F_050D(void)
{
    run_routine(0x960B);
}

/*
 * @routine 071D:006 StageVec8006_071D
 * @run 6B:8006
 * WHAT IT DOES: Goes to the overlay's tile read at $9991.
 * HOW: One jump.
 * WAITS: where the routine at $9991 waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $9991.
 */
void abi_StageVec8006_071D(void)
{
    run_routine(0x9991);
}

/*
 * @routine 071D:00C StageVec800C_071D
 * @run 6B:800C
 * WHAT IT DOES: Goes to ProbeBoxes0720.
 * HOW: One jump.
 * WAITS: where the routine at $98BD waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $98BD.
 */
void abi_StageVec800C_071D(void)
{
    run_routine(0x98BD);
}

/*
 * @routine 071D:00F StageVec800F_071D
 * @run 6B:800F
 * WHAT IT DOES: Goes to ProbeBoxesFlagged0720.
 * HOW: One jump.
 * WAITS: where the routine at $991D waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $991D.
 */
void abi_StageVec800F_071D(void)
{
    run_routine(0x991D);
}

/*
 * @routine 0111:006 SceneVec8006_0111
 * @run 6C:8006
 * WHAT IT DOES: Goes to the scene overlay's call at $8169.
 * HOW: One jump.
 * WAITS: where the routine at $8169 waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $8169.
 */
void abi_SceneVec8006_0111(void)
{
    run_routine(0x8169);
}

/*
 * @routine 00C1:00C StageLoadVec800C_00C1
 * @run 6B:800C
 * WHAT IT DOES: Goes to the stage-load overlay's code at $91E2.
 * HOW: One jump.
 * WAITS: where the routine at $91E2 waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $91E2.
 */
void abi_StageLoadVec800C_00C1(void)
{
    run_routine(0x91E2);
}

/*
 * @routine 072C:66E JumpToFollowPathAndShow
 * @run 70:BE6E
 * WHAT IT DOES: Goes to FollowPathAndShow.
 * HOW: One jump.
 * WAITS: where the routine at $BDD0 waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $BDD0.
 */
void abi_JumpToFollowPathAndShow(void)
{
    run_routine(0xBDD0);
}
