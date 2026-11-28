/*
 * The camera and scroll setup of the programs at 10C5 and 12D5: stage_camera.c's and stage_setup_scroll.c's module
 * loaded $390 and $389 on. Each routine is its original's body, told how far the module is moved where it matters.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "stage_regs.h"
#include "stage_camera_moved.h"

/*
 * @routine 10C5:41D CameraStep10C5
 * @run 6F:A41D
 * WHAT IT DOES: CameraStep's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraStep's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraStep.
 */
StageRegs CameraStep10C5(StageRegs r)
{
    return CameraStepMoved(r, 0x390);
}

void abi_CameraStep10C5(void)
{
    stage_regs_load_shim(CameraStep10C5(regs_entry_shim()));
}

/*
 * @routine 10C5:46B StageCameraModeDispatch10C5
 * @run 6F:A46B
 * @stop A476
 * WHAT IT DOES: StageCameraModeDispatch's copy in the program at 10C5, its camera module $390 on.
 * HOW: StageCameraModeDispatch's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as StageCameraModeDispatch.
 */
StageRegs StageCameraModeDispatch10C5(StageRegs r)
{
    return StageCameraModeDispatchMoved(r, 0x390);
}

void abi_StageCameraModeDispatch10C5(void)
{
    stage_regs_load_shim(StageCameraModeDispatch10C5(regs_entry_shim()));
}

/*
 * @routine 10C5:4BF ScrollStepXHalfOf0To110C5
 * @run 6F:A4BF
 * WHAT IT DOES: ScrollStepXHalfOf0To1's copy in the program at 10C5, its camera module $390 on.
 * HOW: ScrollStepXHalfOf0To1's body.
 * WAITS: no.
 * Entry, exit: as ScrollStepXHalfOf0To1.
 */
StageRegs ScrollStepXHalfOf0To110C5(StageRegs r)
{
    return ScrollStepXHalfOf0To1(r);
}

void abi_ScrollStepXHalfOf0To110C5(void)
{
    stage_regs_load_shim(ScrollStepXHalfOf0To110C5(regs_entry_shim()));
}

/*
 * @routine 10C5:4C1 ScrollStepXHalfOf010C5
 * @run 6F:A4C1
 * WHAT IT DOES: ScrollStepXHalfOf0's copy in the program at 10C5, its camera module $390 on.
 * HOW: ScrollStepXHalfOf0's body.
 * WAITS: no.
 * Entry, exit: as ScrollStepXHalfOf0.
 */
StageRegs ScrollStepXHalfOf010C5(StageRegs r)
{
    return ScrollStepXHalfOf0(r);
}

void abi_ScrollStepXHalfOf010C5(void)
{
    stage_regs_load_shim(ScrollStepXHalfOf010C5(regs_entry_shim()));
}

/*
 * @routine 10C5:4C2 ScrollStepXHalf10C5
 * @run 6F:A4C2
 * WHAT IT DOES: ScrollStepXHalf's copy in the program at 10C5, its camera module $390 on.
 * HOW: ScrollStepXHalf's body.
 * WAITS: no.
 * Entry, exit: as ScrollStepXHalf.
 */
StageRegs ScrollStepXHalf10C5(StageRegs r)
{
    return ScrollStepXHalf(r);
}

void abi_ScrollStepXHalf10C5(void)
{
    stage_regs_load_shim(ScrollStepXHalf10C5(regs_entry_shim()));
}

/*
 * @routine 10C5:4D5 CameraMode1B10C5
 * @run 6F:A4D5
 * WHAT IT DOES: CameraMode1B's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode1B's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode1B.
 */
StageRegs CameraMode1B10C5(StageRegs r)
{
    return CameraMode1BMoved(r, 0x390);
}

void abi_CameraMode1B10C5(void)
{
    stage_regs_load_shim(CameraMode1B10C5(regs_entry_shim()));
}

/*
 * @routine 10C5:4F7 CameraMode1910C5
 * @run 6F:A4F7
 * WHAT IT DOES: CameraMode19's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode19's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode19.
 */
StageRegs CameraMode1910C5(StageRegs r)
{
    return CameraMode19Moved(r, 0x390);
}

void abi_CameraMode1910C5(void)
{
    stage_regs_load_shim(CameraMode1910C5(regs_entry_shim()));
}

/*
 * @routine 10C5:58B CameraMode1710C5
 * @run 6F:A58B
 * WHAT IT DOES: CameraMode17's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode17's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode17.
 */
StageRegs CameraMode1710C5(StageRegs r)
{
    return CameraMode17Moved(r, 0x390);
}

void abi_CameraMode1710C5(void)
{
    stage_regs_load_shim(CameraMode1710C5(regs_entry_shim()));
}

/*
 * @routine 10C5:5A0 CameraMode1610C5
 * @run 6F:A5A0
 * WHAT IT DOES: CameraMode16's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode16's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode16.
 */
StageRegs CameraMode1610C5(StageRegs r)
{
    return CameraMode16Moved(r, 0x390);
}

void abi_CameraMode1610C5(void)
{
    stage_regs_load_shim(CameraMode1610C5(regs_entry_shim()));
}

/*
 * @routine 10C5:5B4 CameraMode1510C5
 * @run 6F:A5B4
 * WHAT IT DOES: CameraMode15's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode15's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode15.
 */
StageRegs CameraMode1510C5(StageRegs r)
{
    return CameraMode15Moved(r, 0x390);
}

void abi_CameraMode1510C5(void)
{
    stage_regs_load_shim(CameraMode1510C5(regs_entry_shim()));
}

/*
 * @routine 10C5:5CD CameraWaterWave10C5
 * @run 6F:A5CD
 * WHAT IT DOES: CameraWaterWave's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraWaterWave's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraWaterWave.
 */
StageRegs CameraWaterWave10C5(StageRegs r)
{
    return CameraWaterWaveMoved(r, 0x390);
}

void abi_CameraWaterWave10C5(void)
{
    stage_regs_load_shim(CameraWaterWave10C5(regs_entry_shim()));
}

/*
 * @routine 10C5:5FF CameraMode1410C5
 * @run 6F:A5FF
 * WHAT IT DOES: CameraMode14's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode14's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode14.
 */
StageRegs CameraMode1410C5(StageRegs r)
{
    return CameraMode14Moved(r, 0x390);
}

void abi_CameraMode1410C5(void)
{
    stage_regs_load_shim(CameraMode1410C5(regs_entry_shim()));
}

/*
 * @routine 10C5:615 CameraMode1310C5
 * @run 6F:A615
 * WHAT IT DOES: CameraMode13's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode13's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode13.
 */
StageRegs CameraMode1310C5(StageRegs r)
{
    return CameraMode13Moved(r, 0x390);
}

void abi_CameraMode1310C5(void)
{
    stage_regs_load_shim(CameraMode1310C5(regs_entry_shim()));
}

/*
 * @routine 10C5:685 CameraMode1210C5
 * @run 6F:A685
 * WHAT IT DOES: CameraMode12's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode12's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode12.
 */
StageRegs CameraMode1210C5(StageRegs r)
{
    return CameraMode12Moved(r, 0x390);
}

void abi_CameraMode1210C5(void)
{
    stage_regs_load_shim(CameraMode1210C5(regs_entry_shim()));
}

/*
 * @routine 10C5:71D CameraMode1110C5
 * @run 6F:A71D
 * WHAT IT DOES: CameraMode11's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode11's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode11.
 */
StageRegs CameraMode1110C5(StageRegs r)
{
    return CameraMode11Moved(r, 0x390);
}

void abi_CameraMode1110C5(void)
{
    stage_regs_load_shim(CameraMode1110C5(regs_entry_shim()));
}

/*
 * @routine 10C5:724 CameraMode1010C5
 * @run 6F:A724
 * WHAT IT DOES: CameraMode10's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode10's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode10.
 */
StageRegs CameraMode1010C5(StageRegs r)
{
    return CameraMode10Moved(r, 0x390);
}

void abi_CameraMode1010C5(void)
{
    stage_regs_load_shim(CameraMode1010C5(regs_entry_shim()));
}

/*
 * @routine 10C5:755 CameraMode0F10C5
 * @run 6F:A755
 * WHAT IT DOES: CameraMode0F's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode0F's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0F.
 */
StageRegs CameraMode0F10C5(StageRegs r)
{
    return CameraMode0FMoved(r, 0x390);
}

void abi_CameraMode0F10C5(void)
{
    stage_regs_load_shim(CameraMode0F10C5(regs_entry_shim()));
}

/*
 * @routine 10C5:77B CameraMode0D10C5
 * @run 6F:A77B
 * WHAT IT DOES: CameraMode0D's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode0D's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0D.
 */
StageRegs CameraMode0D10C5(StageRegs r)
{
    return CameraMode0DMoved(r, 0x390);
}

void abi_CameraMode0D10C5(void)
{
    stage_regs_load_shim(CameraMode0D10C5(regs_entry_shim()));
}

/*
 * @routine 10C5:786 CameraMode0310C5
 * @run 6F:A786
 * WHAT IT DOES: CameraMode03's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode03's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode03.
 */
StageRegs CameraMode0310C5(StageRegs r)
{
    return CameraMode03Moved(r, 0x390);
}

void abi_CameraMode0310C5(void)
{
    stage_regs_load_shim(CameraMode0310C5(regs_entry_shim()));
}

/*
 * @routine 10C5:7DB CameraMode1C10C5
 * @run 6F:A7DB
 * WHAT IT DOES: CameraMode1C's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode1C's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode1C.
 */
StageRegs CameraMode1C10C5(StageRegs r)
{
    return CameraMode1CMoved(r, 0x390);
}

void abi_CameraMode1C10C5(void)
{
    stage_regs_load_shim(CameraMode1C10C5(regs_entry_shim()));
}

/*
 * @routine 10C5:7F7 CameraMode0410C5
 * @run 6F:A7F7
 * WHAT IT DOES: CameraMode04's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode04's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode04.
 */
StageRegs CameraMode0410C5(StageRegs r)
{
    return CameraMode04Moved(r, 0x390);
}

void abi_CameraMode0410C5(void)
{
    stage_regs_load_shim(CameraMode0410C5(regs_entry_shim()));
}

/*
 * @routine 10C6:007 CameraMode0610C5
 * @run 6F:A807
 * WHAT IT DOES: CameraMode06's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode06's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode06.
 */
StageRegs CameraMode0610C5(StageRegs r)
{
    return CameraMode06Moved(r, 0x390);
}

void abi_CameraMode0610C5(void)
{
    stage_regs_load_shim(CameraMode0610C5(regs_entry_shim()));
}

/*
 * @routine 10C6:022 CameraMode0710C5
 * @run 6F:A822
 * @stop A859
 * WHAT IT DOES: CameraMode07's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode07's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode07.
 */
StageRegs CameraMode0710C5(StageRegs r)
{
    return CameraMode07Moved(r, 0x390);
}

void abi_CameraMode0710C5(void)
{
    stage_regs_load_shim(CameraMode0710C5(regs_entry_shim()));
}

/*
 * @routine 10C6:059 ScrollYHeldByX10C5
 * @run 6F:A859
 * WHAT IT DOES: ScrollYHeldByX's copy in the program at 10C5, its camera module $390 on.
 * HOW: ScrollYHeldByX's body.
 * WAITS: no.
 * Entry, exit: as ScrollYHeldByX.
 */
StageRegs ScrollYHeldByX10C5(StageRegs r)
{
    return ScrollYHeldByX(r);
}

void abi_ScrollYHeldByX10C5(void)
{
    stage_regs_load_shim(ScrollYHeldByX10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:060 ScrollYAtTop10C5
 * @run 6F:A860
 * WHAT IT DOES: ScrollYAtTop's copy in the program at 10C5, its camera module $390 on.
 * HOW: ScrollYAtTop's body.
 * WAITS: no.
 * Entry, exit: as ScrollYAtTop.
 */
StageRegs ScrollYAtTop10C5(StageRegs r)
{
    return ScrollYAtTop(r);
}

void abi_ScrollYAtTop10C5(void)
{
    stage_regs_load_shim(ScrollYAtTop10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:06D ScrollYSlideDown10C5
 * @run 6F:A86D
 * WHAT IT DOES: ScrollYSlideDown's copy in the program at 10C5, its camera module $390 on.
 * HOW: ScrollYSlideDown's body.
 * WAITS: no.
 * Entry, exit: as ScrollYSlideDown.
 */
StageRegs ScrollYSlideDown10C5(StageRegs r)
{
    return ScrollYSlideDown(r);
}

void abi_ScrollYSlideDown10C5(void)
{
    stage_regs_load_shim(ScrollYSlideDown10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:07E ScrollYAtBottom10C5
 * @run 6F:A87E
 * WHAT IT DOES: ScrollYAtBottom's copy in the program at 10C5, its camera module $390 on.
 * HOW: ScrollYAtBottom's body.
 * WAITS: no.
 * Entry, exit: as ScrollYAtBottom.
 */
StageRegs ScrollYAtBottom10C5(StageRegs r)
{
    return ScrollYAtBottom(r);
}

void abi_ScrollYAtBottom10C5(void)
{
    stage_regs_load_shim(ScrollYAtBottom10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:08D ScrollYSlideBy8410C5
 * @run 6F:A88D
 * WHAT IT DOES: ScrollYSlideBy84's copy in the program at 10C5, its camera module $390 on.
 * HOW: ScrollYSlideBy84's body.
 * WAITS: no.
 * Entry, exit: as ScrollYSlideBy84.
 */
StageRegs ScrollYSlideBy8410C5(StageRegs r)
{
    return ScrollYSlideBy84(r);
}

void abi_ScrollYSlideBy8410C5(void)
{
    stage_regs_load_shim(ScrollYSlideBy8410C5(regs_entry_shim()));
}

/*
 * @routine 10C6:0A5 ScrollYSlideBy410C5
 * @run 6F:A8A5
 * WHAT IT DOES: ScrollYSlideBy4's copy in the program at 10C5, its camera module $390 on.
 * HOW: ScrollYSlideBy4's body.
 * WAITS: no.
 * Entry, exit: as ScrollYSlideBy4.
 */
StageRegs ScrollYSlideBy410C5(StageRegs r)
{
    return ScrollYSlideBy4(r);
}

void abi_ScrollYSlideBy410C5(void)
{
    stage_regs_load_shim(ScrollYSlideBy410C5(regs_entry_shim()));
}

/*
 * @routine 10C6:0B9 CameraMode1A10C5
 * @run 6F:A8B9
 * WHAT IT DOES: CameraMode1A's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode1A's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode1A.
 */
StageRegs CameraMode1A10C5(StageRegs r)
{
    return CameraMode1AMoved(r, 0x390);
}

void abi_CameraMode1A10C5(void)
{
    stage_regs_load_shim(CameraMode1A10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:0DA CameraMode00KeepY10C5
 * @run 6F:A8DA
 * WHAT IT DOES: CameraMode00KeepY's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode00KeepY's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode00KeepY.
 */
StageRegs CameraMode00KeepY10C5(StageRegs r)
{
    return CameraMode00KeepYMoved(r, 0x390);
}

void abi_CameraMode00KeepY10C5(void)
{
    stage_regs_load_shim(CameraMode00KeepY10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:0F6 CameraMode0810C5
 * @run 6F:A8F6
 * WHAT IT DOES: CameraMode08's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode08's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode08.
 */
StageRegs CameraMode0810C5(StageRegs r)
{
    return CameraMode08Moved(r, 0x390);
}

void abi_CameraMode0810C5(void)
{
    stage_regs_load_shim(CameraMode0810C5(regs_entry_shim()));
}

/*
 * @routine 10C6:134 CameraMode1D10C5
 * @run 6F:A934
 * WHAT IT DOES: CameraMode1D's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode1D's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode1D.
 */
StageRegs CameraMode1D10C5(StageRegs r)
{
    return CameraMode1DMoved(r, 0x390);
}

void abi_CameraMode1D10C5(void)
{
    stage_regs_load_shim(CameraMode1D10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:159 CameraMode0E10C5
 * @run 6F:A959
 * WHAT IT DOES: CameraMode0E's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode0E's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0E.
 */
StageRegs CameraMode0E10C5(StageRegs r)
{
    return CameraMode0EMoved(r, 0x390);
}

void abi_CameraMode0E10C5(void)
{
    stage_regs_load_shim(CameraMode0E10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:180 CameraMode0910C5
 * @run 6F:A980
 * WHAT IT DOES: CameraMode09's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode09's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode09.
 */
StageRegs CameraMode0910C5(StageRegs r)
{
    return CameraMode09Moved(r, 0x390);
}

void abi_CameraMode0910C5(void)
{
    stage_regs_load_shim(CameraMode0910C5(regs_entry_shim()));
}

/*
 * @routine 10C6:1B3 CameraMode0C10C5
 * @run 6F:A9B3
 * WHAT IT DOES: CameraMode0C's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode0C's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0C.
 */
StageRegs CameraMode0C10C5(StageRegs r)
{
    return CameraMode0CMoved(r, 0x390);
}

void abi_CameraMode0C10C5(void)
{
    stage_regs_load_shim(CameraMode0C10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:1BB CameraMode0B10C5
 * @run 6F:A9BB
 * WHAT IT DOES: CameraMode0B's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode0B's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0B.
 */
StageRegs CameraMode0B10C5(StageRegs r)
{
    return CameraMode0BMoved(r, 0x390);
}

void abi_CameraMode0B10C5(void)
{
    stage_regs_load_shim(CameraMode0B10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:1BF CameraMode0A10C5
 * @run 6F:A9BF
 * WHAT IT DOES: CameraMode0A's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode0A's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0A.
 */
StageRegs CameraMode0A10C5(StageRegs r)
{
    return CameraMode0AMoved(r, 0x390);
}

void abi_CameraMode0A10C5(void)
{
    stage_regs_load_shim(CameraMode0A10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:1C1 CameraBandsBlocks10C5
 * @run 6F:A9C1
 * WHAT IT DOES: CameraBandsBlocks's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraBandsBlocks's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraBandsBlocks.
 */
StageRegs CameraBandsBlocks10C5(StageRegs r)
{
    return CameraBandsBlocksMoved(r, 0x390);
}

void abi_CameraBandsBlocks10C5(void)
{
    stage_regs_load_shim(CameraBandsBlocks10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:1C6 CameraBandsY10C5
 * @run 6F:A9C6
 * WHAT IT DOES: CameraBandsY's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraBandsY's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraBandsY.
 */
StageRegs CameraBandsY10C5(StageRegs r)
{
    return CameraBandsYMoved(r, 0x390);
}

void abi_CameraBandsY10C5(void)
{
    stage_regs_load_shim(CameraBandsY10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:1D4 CameraParallaxSteps10C5
 * @run 6F:A9D4
 * WHAT IT DOES: CameraParallaxSteps's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraParallaxSteps's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraParallaxSteps.
 */
StageRegs CameraParallaxSteps10C5(StageRegs r)
{
    return CameraParallaxStepsMoved(r, 0x390);
}

void abi_CameraParallaxSteps10C5(void)
{
    stage_regs_load_shim(CameraParallaxSteps10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:215 CameraParallaxAdvance10C5
 * @run 6F:AA15
 * WHAT IT DOES: CameraParallaxAdvance's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraParallaxAdvance's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraParallaxAdvance.
 */
StageRegs CameraParallaxAdvance10C5(StageRegs r)
{
    return CameraParallaxAdvanceMoved(r, 0x390);
}

void abi_CameraParallaxAdvance10C5(void)
{
    stage_regs_load_shim(CameraParallaxAdvance10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:22E CameraMode00Set10C5
 * @run 6F:AA2E
 * WHAT IT DOES: CameraMode00Set's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode00Set's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode00Set.
 */
StageRegs CameraMode00Set10C5(StageRegs r)
{
    return CameraMode00SetMoved(r, 0x390);
}

void abi_CameraMode00Set10C5(void)
{
    stage_regs_load_shim(CameraMode00Set10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:234 CameraMode0010C5
 * @run 6F:AA34
 * @stop AA3F
 * WHAT IT DOES: CameraMode00's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode00's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode00.
 */
StageRegs CameraMode0010C5(StageRegs r)
{
    return CameraMode00Moved(r, 0x390);
}

void abi_CameraMode0010C5(void)
{
    stage_regs_load_shim(CameraMode0010C5(regs_entry_shim()));
}

/*
 * @routine 10C6:23F CameraMode0110C5
 * @run 6F:AA3F
 * WHAT IT DOES: CameraMode01's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode01's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode01.
 */
StageRegs CameraMode0110C5(StageRegs r)
{
    return CameraMode01Moved(r, 0x390);
}

void abi_CameraMode0110C5(void)
{
    stage_regs_load_shim(CameraMode0110C5(regs_entry_shim()));
}

/*
 * @routine 10C6:24A CameraMode0210C5
 * @run 6F:AA4A
 * WHAT IT DOES: CameraMode02's copy in the program at 10C5, its camera module $390 on.
 * HOW: CameraMode02's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode02.
 */
StageRegs CameraMode0210C5(StageRegs r)
{
    return CameraMode02Moved(r, 0x390);
}

void abi_CameraMode0210C5(void)
{
    stage_regs_load_shim(CameraMode0210C5(regs_entry_shim()));
}

/*
 * @routine 10C6:314 ColumnDueLeft10C5
 * @run 6F:AB14
 * WHAT IT DOES: ColumnDueLeft's copy in the program at 10C5, its camera module $390 on.
 * HOW: ColumnDueLeft's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as ColumnDueLeft.
 */
StageRegs ColumnDueLeft10C5(StageRegs r)
{
    return ColumnDueLeftMoved(r, 0x390);
}

void abi_ColumnDueLeft10C5(void)
{
    stage_regs_load_shim(ColumnDueLeft10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:327 ColumnDue10C5
 * @run 6F:AB27
 * WHAT IT DOES: ColumnDue's copy in the program at 10C5, its camera module $390 on.
 * HOW: ColumnDue's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as ColumnDue.
 */
StageRegs ColumnDue10C5(StageRegs r)
{
    return ColumnDueMoved(r, 0x390);
}

void abi_ColumnDue10C5(void)
{
    stage_regs_load_shim(ColumnDue10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:4DC StageSetUpScroll10C5
 * @run 6F:ACDC
 * WHAT IT DOES: StageSetUpScroll's copy in the program at 10C5, its camera module $390 on.
 * HOW: StageSetUpScroll's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as StageSetUpScroll.
 */
StageRegs StageSetUpScroll10C5(StageRegs r)
{
    return StageSetUpScrollMoved(r, 0x390);
}

void abi_StageSetUpScroll10C5(void)
{
    stage_regs_load_shim(StageSetUpScroll10C5(regs_entry_shim()));
}

/*
 * @routine 10C6:541 StageSetUpScrollByEffect10C5
 * @run 6F:AD41
 * WHAT IT DOES: StageSetUpScrollByEffect's copy in the program at 10C5, its camera module $390 on.
 * HOW: StageSetUpScrollByEffect's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as StageSetUpScrollByEffect.
 */
StageRegs StageSetUpScrollByEffect10C5(StageRegs r)
{
    return StageSetUpScrollByEffectMoved(r, 0x390);
}

void abi_StageSetUpScrollByEffect10C5(void)
{
    stage_regs_load_shim(StageSetUpScrollByEffect10C5(regs_entry_shim()));
}

/*
 * @routine 12D5:416 CameraStep12D5
 * @run 6F:A416
 * WHAT IT DOES: CameraStep's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraStep's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraStep.
 */
StageRegs CameraStep12D5(StageRegs r)
{
    return CameraStepMoved(r, 0x389);
}

void abi_CameraStep12D5(void)
{
    stage_regs_load_shim(CameraStep12D5(regs_entry_shim()));
}

/*
 * @routine 12D5:464 StageCameraModeDispatch12D5
 * @run 6F:A464
 * @stop A46F
 * WHAT IT DOES: StageCameraModeDispatch's copy in the program at 12D5, its camera module $389 on.
 * HOW: StageCameraModeDispatch's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as StageCameraModeDispatch.
 */
StageRegs StageCameraModeDispatch12D5(StageRegs r)
{
    return StageCameraModeDispatchMoved(r, 0x389);
}

void abi_StageCameraModeDispatch12D5(void)
{
    stage_regs_load_shim(StageCameraModeDispatch12D5(regs_entry_shim()));
}

/*
 * @routine 12D5:4B8 ScrollStepXHalfOf0To112D5
 * @run 6F:A4B8
 * WHAT IT DOES: ScrollStepXHalfOf0To1's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScrollStepXHalfOf0To1's body.
 * WAITS: no.
 * Entry, exit: as ScrollStepXHalfOf0To1.
 */
StageRegs ScrollStepXHalfOf0To112D5(StageRegs r)
{
    return ScrollStepXHalfOf0To1(r);
}

void abi_ScrollStepXHalfOf0To112D5(void)
{
    stage_regs_load_shim(ScrollStepXHalfOf0To112D5(regs_entry_shim()));
}

/*
 * @routine 12D5:4BA ScrollStepXHalfOf012D5
 * @run 6F:A4BA
 * WHAT IT DOES: ScrollStepXHalfOf0's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScrollStepXHalfOf0's body.
 * WAITS: no.
 * Entry, exit: as ScrollStepXHalfOf0.
 */
StageRegs ScrollStepXHalfOf012D5(StageRegs r)
{
    return ScrollStepXHalfOf0(r);
}

void abi_ScrollStepXHalfOf012D5(void)
{
    stage_regs_load_shim(ScrollStepXHalfOf012D5(regs_entry_shim()));
}

/*
 * @routine 12D5:4BB ScrollStepXHalf12D5
 * @run 6F:A4BB
 * WHAT IT DOES: ScrollStepXHalf's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScrollStepXHalf's body.
 * WAITS: no.
 * Entry, exit: as ScrollStepXHalf.
 */
StageRegs ScrollStepXHalf12D5(StageRegs r)
{
    return ScrollStepXHalf(r);
}

void abi_ScrollStepXHalf12D5(void)
{
    stage_regs_load_shim(ScrollStepXHalf12D5(regs_entry_shim()));
}

/*
 * @routine 12D5:4CE CameraMode1B12D5
 * @run 6F:A4CE
 * WHAT IT DOES: CameraMode1B's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode1B's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode1B.
 */
StageRegs CameraMode1B12D5(StageRegs r)
{
    return CameraMode1BMoved(r, 0x389);
}

void abi_CameraMode1B12D5(void)
{
    stage_regs_load_shim(CameraMode1B12D5(regs_entry_shim()));
}

/*
 * @routine 12D5:4F0 CameraMode1912D5
 * @run 6F:A4F0
 * WHAT IT DOES: CameraMode19's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode19's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode19.
 */
StageRegs CameraMode1912D5(StageRegs r)
{
    return CameraMode19Moved(r, 0x389);
}

void abi_CameraMode1912D5(void)
{
    stage_regs_load_shim(CameraMode1912D5(regs_entry_shim()));
}

/*
 * @routine 12D5:584 CameraMode1712D5
 * @run 6F:A584
 * WHAT IT DOES: CameraMode17's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode17's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode17.
 */
StageRegs CameraMode1712D5(StageRegs r)
{
    return CameraMode17Moved(r, 0x389);
}

void abi_CameraMode1712D5(void)
{
    stage_regs_load_shim(CameraMode1712D5(regs_entry_shim()));
}

/*
 * @routine 12D5:599 CameraMode1612D5
 * @run 6F:A599
 * WHAT IT DOES: CameraMode16's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode16's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode16.
 */
StageRegs CameraMode1612D5(StageRegs r)
{
    return CameraMode16Moved(r, 0x389);
}

void abi_CameraMode1612D5(void)
{
    stage_regs_load_shim(CameraMode1612D5(regs_entry_shim()));
}

/*
 * @routine 12D5:5AD CameraMode1512D5
 * @run 6F:A5AD
 * WHAT IT DOES: CameraMode15's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode15's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode15.
 */
StageRegs CameraMode1512D5(StageRegs r)
{
    return CameraMode15Moved(r, 0x389);
}

void abi_CameraMode1512D5(void)
{
    stage_regs_load_shim(CameraMode1512D5(regs_entry_shim()));
}

/*
 * @routine 12D5:5C6 CameraWaterWave12D5
 * @run 6F:A5C6
 * WHAT IT DOES: CameraWaterWave's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraWaterWave's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraWaterWave.
 */
StageRegs CameraWaterWave12D5(StageRegs r)
{
    return CameraWaterWaveMoved(r, 0x389);
}

void abi_CameraWaterWave12D5(void)
{
    stage_regs_load_shim(CameraWaterWave12D5(regs_entry_shim()));
}

/*
 * @routine 12D5:5F8 CameraMode1412D5
 * @run 6F:A5F8
 * WHAT IT DOES: CameraMode14's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode14's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode14.
 */
StageRegs CameraMode1412D5(StageRegs r)
{
    return CameraMode14Moved(r, 0x389);
}

void abi_CameraMode1412D5(void)
{
    stage_regs_load_shim(CameraMode1412D5(regs_entry_shim()));
}

/*
 * @routine 12D5:60E CameraMode1312D5
 * @run 6F:A60E
 * WHAT IT DOES: CameraMode13's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode13's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode13.
 */
StageRegs CameraMode1312D5(StageRegs r)
{
    return CameraMode13Moved(r, 0x389);
}

void abi_CameraMode1312D5(void)
{
    stage_regs_load_shim(CameraMode1312D5(regs_entry_shim()));
}

/*
 * @routine 12D5:67E CameraMode1212D5
 * @run 6F:A67E
 * WHAT IT DOES: CameraMode12's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode12's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode12.
 */
StageRegs CameraMode1212D5(StageRegs r)
{
    return CameraMode12Moved(r, 0x389);
}

void abi_CameraMode1212D5(void)
{
    stage_regs_load_shim(CameraMode1212D5(regs_entry_shim()));
}

/*
 * @routine 12D5:716 CameraMode1112D5
 * @run 6F:A716
 * WHAT IT DOES: CameraMode11's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode11's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode11.
 */
StageRegs CameraMode1112D5(StageRegs r)
{
    return CameraMode11Moved(r, 0x389);
}

void abi_CameraMode1112D5(void)
{
    stage_regs_load_shim(CameraMode1112D5(regs_entry_shim()));
}

/*
 * @routine 12D5:71D CameraMode1012D5
 * @run 6F:A71D
 * WHAT IT DOES: CameraMode10's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode10's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode10.
 */
StageRegs CameraMode1012D5(StageRegs r)
{
    return CameraMode10Moved(r, 0x389);
}

void abi_CameraMode1012D5(void)
{
    stage_regs_load_shim(CameraMode1012D5(regs_entry_shim()));
}

/*
 * @routine 12D5:74E CameraMode0F12D5
 * @run 6F:A74E
 * WHAT IT DOES: CameraMode0F's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode0F's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0F.
 */
StageRegs CameraMode0F12D5(StageRegs r)
{
    return CameraMode0FMoved(r, 0x389);
}

void abi_CameraMode0F12D5(void)
{
    stage_regs_load_shim(CameraMode0F12D5(regs_entry_shim()));
}

/*
 * @routine 12D5:774 CameraMode0D12D5
 * @run 6F:A774
 * WHAT IT DOES: CameraMode0D's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode0D's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0D.
 */
StageRegs CameraMode0D12D5(StageRegs r)
{
    return CameraMode0DMoved(r, 0x389);
}

void abi_CameraMode0D12D5(void)
{
    stage_regs_load_shim(CameraMode0D12D5(regs_entry_shim()));
}

/*
 * @routine 12D5:77F CameraMode0312D5
 * @run 6F:A77F
 * WHAT IT DOES: CameraMode03's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode03's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode03.
 */
StageRegs CameraMode0312D5(StageRegs r)
{
    return CameraMode03Moved(r, 0x389);
}

void abi_CameraMode0312D5(void)
{
    stage_regs_load_shim(CameraMode0312D5(regs_entry_shim()));
}

/*
 * @routine 12D5:7D4 CameraMode1C12D5
 * @run 6F:A7D4
 * WHAT IT DOES: CameraMode1C's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode1C's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode1C.
 */
StageRegs CameraMode1C12D5(StageRegs r)
{
    return CameraMode1CMoved(r, 0x389);
}

void abi_CameraMode1C12D5(void)
{
    stage_regs_load_shim(CameraMode1C12D5(regs_entry_shim()));
}

/*
 * @routine 12D5:7F0 CameraMode0412D5
 * @run 6F:A7F0
 * WHAT IT DOES: CameraMode04's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode04's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode04.
 */
StageRegs CameraMode0412D5(StageRegs r)
{
    return CameraMode04Moved(r, 0x389);
}

void abi_CameraMode0412D5(void)
{
    stage_regs_load_shim(CameraMode0412D5(regs_entry_shim()));
}

/*
 * @routine 12D6:000 CameraMode0612D5
 * @run 6F:A800
 * WHAT IT DOES: CameraMode06's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode06's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode06.
 */
StageRegs CameraMode0612D5(StageRegs r)
{
    return CameraMode06Moved(r, 0x389);
}

void abi_CameraMode0612D5(void)
{
    stage_regs_load_shim(CameraMode0612D5(regs_entry_shim()));
}

/*
 * @routine 12D6:01B CameraMode0712D5
 * @run 6F:A81B
 * @stop A852
 * WHAT IT DOES: CameraMode07's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode07's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode07.
 */
StageRegs CameraMode0712D5(StageRegs r)
{
    return CameraMode07Moved(r, 0x389);
}

void abi_CameraMode0712D5(void)
{
    stage_regs_load_shim(CameraMode0712D5(regs_entry_shim()));
}

/*
 * @routine 12D6:052 ScrollYHeldByX12D5
 * @run 6F:A852
 * WHAT IT DOES: ScrollYHeldByX's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScrollYHeldByX's body.
 * WAITS: no.
 * Entry, exit: as ScrollYHeldByX.
 */
StageRegs ScrollYHeldByX12D5(StageRegs r)
{
    return ScrollYHeldByX(r);
}

void abi_ScrollYHeldByX12D5(void)
{
    stage_regs_load_shim(ScrollYHeldByX12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:059 ScrollYAtTop12D5
 * @run 6F:A859
 * WHAT IT DOES: ScrollYAtTop's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScrollYAtTop's body.
 * WAITS: no.
 * Entry, exit: as ScrollYAtTop.
 */
StageRegs ScrollYAtTop12D5(StageRegs r)
{
    return ScrollYAtTop(r);
}

void abi_ScrollYAtTop12D5(void)
{
    stage_regs_load_shim(ScrollYAtTop12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:066 ScrollYSlideDown12D5
 * @run 6F:A866
 * WHAT IT DOES: ScrollYSlideDown's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScrollYSlideDown's body.
 * WAITS: no.
 * Entry, exit: as ScrollYSlideDown.
 */
StageRegs ScrollYSlideDown12D5(StageRegs r)
{
    return ScrollYSlideDown(r);
}

void abi_ScrollYSlideDown12D5(void)
{
    stage_regs_load_shim(ScrollYSlideDown12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:077 ScrollYAtBottom12D5
 * @run 6F:A877
 * WHAT IT DOES: ScrollYAtBottom's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScrollYAtBottom's body.
 * WAITS: no.
 * Entry, exit: as ScrollYAtBottom.
 */
StageRegs ScrollYAtBottom12D5(StageRegs r)
{
    return ScrollYAtBottom(r);
}

void abi_ScrollYAtBottom12D5(void)
{
    stage_regs_load_shim(ScrollYAtBottom12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:086 ScrollYSlideBy8412D5
 * @run 6F:A886
 * WHAT IT DOES: ScrollYSlideBy84's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScrollYSlideBy84's body.
 * WAITS: no.
 * Entry, exit: as ScrollYSlideBy84.
 */
StageRegs ScrollYSlideBy8412D5(StageRegs r)
{
    return ScrollYSlideBy84(r);
}

void abi_ScrollYSlideBy8412D5(void)
{
    stage_regs_load_shim(ScrollYSlideBy8412D5(regs_entry_shim()));
}

/*
 * @routine 12D6:09E ScrollYSlideBy412D5
 * @run 6F:A89E
 * WHAT IT DOES: ScrollYSlideBy4's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScrollYSlideBy4's body.
 * WAITS: no.
 * Entry, exit: as ScrollYSlideBy4.
 */
StageRegs ScrollYSlideBy412D5(StageRegs r)
{
    return ScrollYSlideBy4(r);
}

void abi_ScrollYSlideBy412D5(void)
{
    stage_regs_load_shim(ScrollYSlideBy412D5(regs_entry_shim()));
}

/*
 * @routine 12D6:0B2 CameraMode1A12D5
 * @run 6F:A8B2
 * WHAT IT DOES: CameraMode1A's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode1A's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode1A.
 */
StageRegs CameraMode1A12D5(StageRegs r)
{
    return CameraMode1AMoved(r, 0x389);
}

void abi_CameraMode1A12D5(void)
{
    stage_regs_load_shim(CameraMode1A12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:0D3 CameraMode00KeepY12D5
 * @run 6F:A8D3
 * WHAT IT DOES: CameraMode00KeepY's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode00KeepY's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode00KeepY.
 */
StageRegs CameraMode00KeepY12D5(StageRegs r)
{
    return CameraMode00KeepYMoved(r, 0x389);
}

void abi_CameraMode00KeepY12D5(void)
{
    stage_regs_load_shim(CameraMode00KeepY12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:0EF CameraMode0812D5
 * @run 6F:A8EF
 * WHAT IT DOES: CameraMode08's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode08's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode08.
 */
StageRegs CameraMode0812D5(StageRegs r)
{
    return CameraMode08Moved(r, 0x389);
}

void abi_CameraMode0812D5(void)
{
    stage_regs_load_shim(CameraMode0812D5(regs_entry_shim()));
}

/*
 * @routine 12D6:12D CameraMode1D12D5
 * @run 6F:A92D
 * WHAT IT DOES: CameraMode1D's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode1D's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode1D.
 */
StageRegs CameraMode1D12D5(StageRegs r)
{
    return CameraMode1DMoved(r, 0x389);
}

void abi_CameraMode1D12D5(void)
{
    stage_regs_load_shim(CameraMode1D12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:152 CameraMode0E12D5
 * @run 6F:A952
 * WHAT IT DOES: CameraMode0E's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode0E's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0E.
 */
StageRegs CameraMode0E12D5(StageRegs r)
{
    return CameraMode0EMoved(r, 0x389);
}

void abi_CameraMode0E12D5(void)
{
    stage_regs_load_shim(CameraMode0E12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:179 CameraMode0912D5
 * @run 6F:A979
 * WHAT IT DOES: CameraMode09's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode09's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode09.
 */
StageRegs CameraMode0912D5(StageRegs r)
{
    return CameraMode09Moved(r, 0x389);
}

void abi_CameraMode0912D5(void)
{
    stage_regs_load_shim(CameraMode0912D5(regs_entry_shim()));
}

/*
 * @routine 12D6:1AC CameraMode0C12D5
 * @run 6F:A9AC
 * WHAT IT DOES: CameraMode0C's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode0C's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0C.
 */
StageRegs CameraMode0C12D5(StageRegs r)
{
    return CameraMode0CMoved(r, 0x389);
}

void abi_CameraMode0C12D5(void)
{
    stage_regs_load_shim(CameraMode0C12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:1B4 CameraMode0B12D5
 * @run 6F:A9B4
 * WHAT IT DOES: CameraMode0B's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode0B's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0B.
 */
StageRegs CameraMode0B12D5(StageRegs r)
{
    return CameraMode0BMoved(r, 0x389);
}

void abi_CameraMode0B12D5(void)
{
    stage_regs_load_shim(CameraMode0B12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:1B8 CameraMode0A12D5
 * @run 6F:A9B8
 * WHAT IT DOES: CameraMode0A's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode0A's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode0A.
 */
StageRegs CameraMode0A12D5(StageRegs r)
{
    return CameraMode0AMoved(r, 0x389);
}

void abi_CameraMode0A12D5(void)
{
    stage_regs_load_shim(CameraMode0A12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:1BA CameraBandsBlocks12D5
 * @run 6F:A9BA
 * WHAT IT DOES: CameraBandsBlocks's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraBandsBlocks's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraBandsBlocks.
 */
StageRegs CameraBandsBlocks12D5(StageRegs r)
{
    return CameraBandsBlocksMoved(r, 0x389);
}

void abi_CameraBandsBlocks12D5(void)
{
    stage_regs_load_shim(CameraBandsBlocks12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:1BF CameraBandsY12D5
 * @run 6F:A9BF
 * WHAT IT DOES: CameraBandsY's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraBandsY's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraBandsY.
 */
StageRegs CameraBandsY12D5(StageRegs r)
{
    return CameraBandsYMoved(r, 0x389);
}

void abi_CameraBandsY12D5(void)
{
    stage_regs_load_shim(CameraBandsY12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:1CD CameraParallaxSteps12D5
 * @run 6F:A9CD
 * WHAT IT DOES: CameraParallaxSteps's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraParallaxSteps's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraParallaxSteps.
 */
StageRegs CameraParallaxSteps12D5(StageRegs r)
{
    return CameraParallaxStepsMoved(r, 0x389);
}

void abi_CameraParallaxSteps12D5(void)
{
    stage_regs_load_shim(CameraParallaxSteps12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:20E CameraParallaxAdvance12D5
 * @run 6F:AA0E
 * WHAT IT DOES: CameraParallaxAdvance's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraParallaxAdvance's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraParallaxAdvance.
 */
StageRegs CameraParallaxAdvance12D5(StageRegs r)
{
    return CameraParallaxAdvanceMoved(r, 0x389);
}

void abi_CameraParallaxAdvance12D5(void)
{
    stage_regs_load_shim(CameraParallaxAdvance12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:227 CameraMode00Set12D5
 * @run 6F:AA27
 * WHAT IT DOES: CameraMode00Set's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode00Set's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode00Set.
 */
StageRegs CameraMode00Set12D5(StageRegs r)
{
    return CameraMode00SetMoved(r, 0x389);
}

void abi_CameraMode00Set12D5(void)
{
    stage_regs_load_shim(CameraMode00Set12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:22D CameraMode0012D5
 * @run 6F:AA2D
 * @stop AA38
 * WHAT IT DOES: CameraMode00's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode00's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode00.
 */
StageRegs CameraMode0012D5(StageRegs r)
{
    return CameraMode00Moved(r, 0x389);
}

void abi_CameraMode0012D5(void)
{
    stage_regs_load_shim(CameraMode0012D5(regs_entry_shim()));
}

/*
 * @routine 12D6:238 CameraMode0112D5
 * @run 6F:AA38
 * WHAT IT DOES: CameraMode01's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode01's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode01.
 */
StageRegs CameraMode0112D5(StageRegs r)
{
    return CameraMode01Moved(r, 0x389);
}

void abi_CameraMode0112D5(void)
{
    stage_regs_load_shim(CameraMode0112D5(regs_entry_shim()));
}

/*
 * @routine 12D6:243 CameraMode0212D5
 * @run 6F:AA43
 * WHAT IT DOES: CameraMode02's copy in the program at 12D5, its camera module $389 on.
 * HOW: CameraMode02's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as CameraMode02.
 */
StageRegs CameraMode0212D5(StageRegs r)
{
    return CameraMode02Moved(r, 0x389);
}

void abi_CameraMode0212D5(void)
{
    stage_regs_load_shim(CameraMode0212D5(regs_entry_shim()));
}

/*
 * @routine 12D6:263 ScreenMapAtEntry12D5
 * @run 6F:AA63
 * WHAT IT DOES: ScreenMapAtEntry's copy in the program at 12D5, its camera module $389 on.
 * HOW: ScreenMapAtEntry's body.
 * WAITS: no.
 * Entry, exit: as ScreenMapAtEntry.
 */
StageRegs ScreenMapAtEntry12D5(StageRegs r)
{
    return ScreenMapAtEntry(r);
}

void abi_ScreenMapAtEntry12D5(void)
{
    stage_regs_load_shim(ScreenMapAtEntry12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:30D ColumnDueLeft12D5
 * @run 6F:AB0D
 * WHAT IT DOES: ColumnDueLeft's copy in the program at 12D5, its camera module $389 on.
 * HOW: ColumnDueLeft's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as ColumnDueLeft.
 */
StageRegs ColumnDueLeft12D5(StageRegs r)
{
    return ColumnDueLeftMoved(r, 0x389);
}

void abi_ColumnDueLeft12D5(void)
{
    stage_regs_load_shim(ColumnDueLeft12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:320 ColumnDue12D5
 * @run 6F:AB20
 * WHAT IT DOES: ColumnDue's copy in the program at 12D5, its camera module $389 on.
 * HOW: ColumnDue's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as ColumnDue.
 */
StageRegs ColumnDue12D5(StageRegs r)
{
    return ColumnDueMoved(r, 0x389);
}

void abi_ColumnDue12D5(void)
{
    stage_regs_load_shim(ColumnDue12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:4D5 StageSetUpScroll12D5
 * @run 6F:ACD5
 * WHAT IT DOES: StageSetUpScroll's copy in the program at 12D5, its camera module $389 on.
 * HOW: StageSetUpScroll's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as StageSetUpScroll.
 */
StageRegs StageSetUpScroll12D5(StageRegs r)
{
    return StageSetUpScrollMoved(r, 0x389);
}

void abi_StageSetUpScroll12D5(void)
{
    stage_regs_load_shim(StageSetUpScroll12D5(regs_entry_shim()));
}

/*
 * @routine 12D6:53A StageSetUpScrollByEffect12D5
 * @run 6F:AD3A
 * WHAT IT DOES: StageSetUpScrollByEffect's copy in the program at 12D5, its camera module $389 on.
 * HOW: StageSetUpScrollByEffect's body with the module's addresses and returns moved.
 * WAITS: no.
 * Entry, exit: as StageSetUpScrollByEffect.
 */
StageRegs StageSetUpScrollByEffect12D5(StageRegs r)
{
    return StageSetUpScrollByEffectMoved(r, 0x389);
}

void abi_StageSetUpScrollByEffect12D5(void)
{
    stage_regs_load_shim(StageSetUpScrollByEffect12D5(regs_entry_shim()));
}
