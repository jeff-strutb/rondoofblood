/*
 * The first routine of each overlay's start, lot L2: 23 copies that differ only in the address they leave in the pointer
 * at $207E (the overlay's own table). While the game runs (zGameRunning not zero) the routine clears the random bytes,
 * a work byte, a flag and four bytes of an unnamed block, then sets the pointer.
 *
 * The stores run only while the game runs. Twelve of the 23 copies have their stores untraced (no script ran them with the
 * game running), so their drafts stop there (liftcmp counts 360 untraced cases and one unreached line each): the shared body is
 * compared in the eleven whose stores a script ran, and the two immediates of each copy are the disc's bytes 21 and 25.
 *
 * $207C-$207D are the demo's pad bytes (zDemoPadLast, zDemoPadFrames) and $207E the pointer its record is read through
 * (zScriptPtr): the reset starts the demo's record at the overlay's own table.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"

/* The one test of zGameRunning, and when the game runs the stores: what the routine ended on. */
ObjArm reset_stage_variables(uint8_t pointer_lo, uint8_t pointer_hi)
{
    ObjArm r = { 0 };

    r.first = RD8(ZP->game_running);
    if (r.first == 0) {
        return r;
    }
    WR8(ZP->random0, 0);
    WR8(ZP->random1, 0);
    WR8(ZP->random2, 0);
    WR8(ZP->random3, 0);
    WR8(ZP->wave_counter, 0);
    WR8(ZP->frame_tick, 0);
    WR8(ZP->demo_pad_last, 0);
    WR8(ZP->demo_pad_frames, 0);
    WR8(ZP->script_ptr, pointer_lo);
    WR8(ZP->script_ptr_hi, pointer_hi);
    r.second = pointer_hi;
    r.flag = true;
    return r;
}

/* A when the game was not running, else the pointer's high byte. */
Ended reset_stage_variables_exit_shim(ObjArm r)
{
    return ended_load(r.flag ? r.second : r.first);
}

/*
 * @routine 024D:0CB ResetStageVariables024D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C20D.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C2 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables024D(void)
{
    return reset_stage_variables(0x0d, 0xc2);
}

void abi_ResetStageVariables024D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables024D()));
}

/*
 * @routine 050D:0CB ResetStageVariables050D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $CE66.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $CE and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables050D(void)
{
    return reset_stage_variables(0x66, 0xce);
}

void abi_ResetStageVariables050D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables050D()));
}

/*
 * @routine 05BD:0CB ResetStageVariables05BD
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $BE15.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $BE and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables05BD(void)
{
    return reset_stage_variables(0x15, 0xbe);
}

void abi_ResetStageVariables05BD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables05BD()));
}

/*
 * @routine 071D:0CB ResetStageVariables071D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C24E.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C2 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables071D(void)
{
    return reset_stage_variables(0x4e, 0xc2);
}

void abi_ResetStageVariables071D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables071D()));
}

/*
 * @routine 07CD:0CB ResetStageVariables07CD
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DC50.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DC and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables07CD(void)
{
    return reset_stage_variables(0x50, 0xdc);
}

void abi_ResetStageVariables07CD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables07CD()));
}

/*
 * @routine 087D:0CB ResetStageVariables087D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C3AA.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C3 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables087D(void)
{
    return reset_stage_variables(0xaa, 0xc3);
}

void abi_ResetStageVariables087D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables087D()));
}

/*
 * @routine 092D:0CB ResetStageVariables092D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DB8F.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DB and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables092D(void)
{
    return reset_stage_variables(0x8f, 0xdb);
}

void abi_ResetStageVariables092D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables092D()));
}

/*
 * @routine 09DD:0CB ResetStageVariables09DD
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C274.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C2 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables09DD(void)
{
    return reset_stage_variables(0x74, 0xc2);
}

void abi_ResetStageVariables09DD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables09DD()));
}

/*
 * @routine 0A8D:0CB ResetStageVariables0A8D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DDD7.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DD and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables0A8D(void)
{
    return reset_stage_variables(0xd7, 0xdd);
}

void abi_ResetStageVariables0A8D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables0A8D()));
}

/*
 * @routine 0B3D:0CB ResetStageVariables0B3D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D46F.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D4 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables0B3D(void)
{
    return reset_stage_variables(0x6f, 0xd4);
}

void abi_ResetStageVariables0B3D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables0B3D()));
}

/*
 * @routine 0BED:0CB ResetStageVariables0BED
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DA06.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DA and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables0BED(void)
{
    return reset_stage_variables(0x06, 0xda);
}

void abi_ResetStageVariables0BED(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables0BED()));
}

/*
 * @routine 0C9D:0CB ResetStageVariables0C9D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C31A.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C3 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables0C9D(void)
{
    return reset_stage_variables(0x1a, 0xc3);
}

void abi_ResetStageVariables0C9D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables0C9D()));
}

/*
 * @routine 0D4D:0CB ResetStageVariables0D4D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D937.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D9 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables0D4D(void)
{
    return reset_stage_variables(0x37, 0xd9);
}

void abi_ResetStageVariables0D4D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables0D4D()));
}

/*
 * @routine 0DFD:0CB ResetStageVariables0DFD
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C15C.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C1 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables0DFD(void)
{
    return reset_stage_variables(0x5c, 0xc1);
}

void abi_ResetStageVariables0DFD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables0DFD()));
}

/*
 * @routine 0EAD:0CB ResetStageVariables0EAD
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $B389.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $B3 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables0EAD(void)
{
    return reset_stage_variables(0x89, 0xb3);
}

void abi_ResetStageVariables0EAD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables0EAD()));
}

/*
 * @routine 0F5D:0CB ResetStageVariables0F5D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $B1F3.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $B1 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables0F5D(void)
{
    return reset_stage_variables(0xf3, 0xb1);
}

void abi_ResetStageVariables0F5D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables0F5D()));
}

/*
 * @routine 100D:0CB ResetStageVariables100D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D556.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D5 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables100D(void)
{
    return reset_stage_variables(0x56, 0xd5);
}

void abi_ResetStageVariables100D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables100D()));
}

/*
 * @routine 10BD:0CB ResetStageVariables10BD
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C58C.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C5 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables10BD(void)
{
    return reset_stage_variables(0x8c, 0xc5);
}

void abi_ResetStageVariables10BD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables10BD()));
}

/*
 * @routine 116D:0CB ResetStageVariables116D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DB63.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DB and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables116D(void)
{
    return reset_stage_variables(0x63, 0xdb);
}

void abi_ResetStageVariables116D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables116D()));
}

/*
 * @routine 12CD:0CB ResetStageVariables12CD
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D25A.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D2 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables12CD(void)
{
    return reset_stage_variables(0x5a, 0xd2);
}

void abi_ResetStageVariables12CD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables12CD()));
}

/*
 * @routine 160D:0CB ResetStageVariables160D
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D916.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D9 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables160D(void)
{
    return reset_stage_variables(0x16, 0xd9);
}

void abi_ResetStageVariables160D(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables160D()));
}

/*
 * @routine 16BD:0CB ResetStageVariables16BD
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $BF26.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $BF and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables16BD(void)
{
    return reset_stage_variables(0x26, 0xbf);
}

void abi_ResetStageVariables16BD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables16BD()));
}

/*
 * @routine 18CD:0CB ResetStageVariables18CD
 * @run 6B:80CB
 * WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C373.
 * HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
 * WAITS: no.
 * Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C3 and N set. X, Y, C, V kept.
 */
ObjArm ResetStageVariables18CD(void)
{
    return reset_stage_variables(0x73, 0xc3);
}

void abi_ResetStageVariables18CD(void)
{
    abi_ended(reset_stage_variables_exit_shim(ResetStageVariables18CD()));
}
