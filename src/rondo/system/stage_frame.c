/* The end of a stage's frame set-up: the scroll stepped, the object banks mapped, the objects run. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum { MPR5 = 0x20, MPR6 = 0x40, OBJECT_BANK_OFFSET = 8, STEP_RETURN = 0x8A52, OBJECTS_RUN = 0xA000 };

void abi_RunObjects(void);
void abi_RichterVecFrame(void);
void abi_CopyTileBlockToVram(void);

/*
 * The jump into whatever the overlay holds at $A000: RunObjects when that is loaded (it
 * takes X and nothing else), the player's frame or the tile block copy when those are, otherwise original code, which gets the registers the mapping add
 * and the increment left, with X as the scroll step left it and Y 1, which every scroll step leaves.
 */
static void run_resident_objects_shim(uint8_t x, Sum sum)
{
    cpu.x = x;
    cpu.y = 1;
    cpu.a = (uint8_t)(sum.value + 1);
    abi_nz(cpu.a);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    uint32_t resident = resident_at(OBJECTS_RUN);

    if (resident == DISC(0x0259, 0x000)) {
        abi_RunObjects();
    } else if (resident == DISC(0x01B9, 0x000)) {
        abi_RichterVecFrame();
    } else if (resident == DISC(0x0255, 0x000)) {
        abi_CopyTileBlockToVram();
    } else {
        call_original(OBJECTS_RUN);
    }
}

/*
 * @routine 024E:24F StepScrollAndRunObjects
 * @run 6B:8A4F
 * WHAT IT DOES: Finishes a stage's frame set-up: steps the stream with the scroll, maps the
 * object banks, and runs the stage's objects.
 * HOW: Calls the overlay's scroll step ($8A36), maps the bank pair 8 above the RAM base bank
 * into MPR5 and MPR6, and jumps to the code at $A000 (RunObjects), which returns to the caller.
 * Every stage overlay holds the same code here.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as RunObjects.
 */
void StepScrollAndRunObjects(uint8_t x)
{
    StageScrollEnd scroll;
    Sum sum;

    jsr_push(STEP_RETURN);
    scroll = StageScrollStepResident(x);
    jsr_pop();
    sum = add_bytes(OBJECT_BANK_OFFSET, RD8(ZP->ram_base_bank), false);
    tam(MPR5, sum.value);
    tam(MPR6, (uint8_t)(sum.value + 1));
    run_resident_objects_shim(scroll.walk.x, sum);
}

void abi_StepScrollAndRunObjects(void)
{
    StepScrollAndRunObjects(cpu.x);
}
