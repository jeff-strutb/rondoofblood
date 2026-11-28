/*
 * Her death's objects frame and her overlay's vectors to the run of her objects (bank $6E and $6D). The run of her
 * objects (MariaRunObjects, maria_objects_run.c) holds the processor, so the frame does too: each call's exit is given
 * to the registers as it returns, and the run's is the frame's.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

void MariaRunObjects(void);                                         /* object/maria_objects_run.c */
void abi_MariaRunObjects(void);                                     /* object/maria_objects_run.c */
ColumnsEnd PlayerAndCameraFrame(void);                              /* object/camera.c */

/*
 * @routine 01E1:39F MariaDeathObjects
 * @run 6E:C39F
 * WHAT IT DOES: The frame of her death's objects: her death's frame, the player's and the camera's frame, then the
 * run of her objects.
 * HOW: MariaDeathFrame, PlayerAndCameraFrame (through the kernel's vector at $40D8), MariaRunObjects.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaRunObjects.
 */
void MariaDeathObjects(void)
{
    MExit death;

    jsr_push(0xC3A2);
    death = MariaDeathFrame(cpu.x, cpu.y);
    jsr_pop();
    mx_exit(death);
    jsr_push(0xC3A5);
    columns_exit_shim(PlayerAndCameraFrame());
    jsr_pop();
    jsr_push(0xC3A8);
    MariaRunObjects();
    jsr_pop();
}

void abi_MariaDeathObjects(void)
{
    MariaDeathObjects();
}

/*
 * @routine 01DD:009 MariaVecRunObjects
 * @run 6D:A009
 * WHAT IT DOES: Her overlay's vector to the run of her objects.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaRunObjects.
 */
void abi_MariaVecRunObjects(void)
{
    abi_MariaRunObjects();
}

/*
 * @routine 01DD:00F MariaVecDeathObjects
 * @run 6D:A00F
 * WHAT IT DOES: Her overlay's vector to her death's objects frame.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaDeathObjects.
 */
void abi_MariaVecDeathObjects(void)
{
    MariaDeathObjects();
}
