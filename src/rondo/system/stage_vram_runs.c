/* The stage overlay's VRAM runs: a table of byte runs expanded into the overlay's VRAM copy buffer. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum { BUFFER_BYTES = 0x00FF, RUN_END = 0xFF, VALUE_MASK = 0x7F, END_OF_RUN_BIT = 0x01 };

/*
 * Clears the buffer, then walks the run list the stage's table points at. A run starts with the
 * buffer index it fills from; its bytes follow, each stored with bit 7 dropped, and bit 7 set on
 * the run's last byte. An $FF where a byte is expected ends the list. Returns the buffer index
 * and the list position the walk stopped at.
 */
static StageRunsEnd expand_vram_runs(uint16_t table)
{
    uint8_t y;
    uint8_t x;
    uint8_t value;
    bool run_done;
    StageRunsEnd end;

    tai(KernelZeros, StageVramBuffer, BUFFER_BYTES);
    y = (uint8_t)(RD8(ZP->field82) << 1);
    WR8(ZP->temp00, rd8((uint16_t)(table + y)));
    WR8(ZP->temp01, rd8((uint16_t)(table + 1 + y)));
    y = 0;
    for (;;) {
        x = RD8_PTR_Y(ZP->temp00, y);
        y++;
        do {
            value = RD8_PTR_Y(ZP->temp00, y);
            y++;
            if (value == RUN_END) {
                end.x = x;
                end.y = y;
                return end;
            }
            ROL8(ZP->temp02, (value & 0x80) != 0);
            WR8(STAGEVRAM->buffer[x], (uint8_t)(value & VALUE_MASK));
            x++;
            run_done = (RD8(ZP->temp02) & END_OF_RUN_BIT) != 0;
        } while (!run_done);
    }
}

/* The exit the list's end leaves: A $FF compared equal to $FF (N 0, Z 1, C 1), X and Y as stopped. */
static void runs_exit_shim(StageRunsEnd end)
{
    cpu.a = RUN_END;
    cpu.x = end.x;
    cpu.y = end.y;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 024E:082 ExpandStageVramRuns
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRuns(void)
{
    return expand_vram_runs(0x9BA0);
}

void abi_ExpandStageVramRuns(void)
{
    runs_exit_shim(ExpandStageVramRuns());
}

/*
 * @routine 050E:082 ExpandStageVramRunsB
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9C30.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsB(void)
{
    return expand_vram_runs(0x9C30);
}

void abi_ExpandStageVramRunsB(void)
{
    runs_exit_shim(ExpandStageVramRunsB());
}

/*
 * @routine 05BE:082 ExpandStageVramRunsC
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E87.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsC(void)
{
    return expand_vram_runs(0x9E87);
}

void abi_ExpandStageVramRunsC(void)
{
    runs_exit_shim(ExpandStageVramRunsC());
}

/*
 * @routine 071E:082 ExpandStageVramRunsD
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9F4B.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsD(void)
{
    return expand_vram_runs(0x9F4B);
}

void abi_ExpandStageVramRunsD(void)
{
    runs_exit_shim(ExpandStageVramRunsD());
}

/*
 * @routine 07CE:082 ExpandStageVramRunsE
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CE2.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsE(void)
{
    return expand_vram_runs(0x9CE2);
}

void abi_ExpandStageVramRunsE(void)
{
    runs_exit_shim(ExpandStageVramRunsE());
}

/*
 * @routine 087E:082 ExpandStageVramRunsF
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E67.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsF(void)
{
    return expand_vram_runs(0x9E67);
}

void abi_ExpandStageVramRunsF(void)
{
    runs_exit_shim(ExpandStageVramRunsF());
}

/*
 * @routine 092E:082 ExpandStageVramRunsG
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CDF.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsG(void)
{
    return expand_vram_runs(0x9CDF);
}

void abi_ExpandStageVramRunsG(void)
{
    runs_exit_shim(ExpandStageVramRunsG());
}

/*
 * @routine 09DE:082 ExpandStageVramRunsH
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9EAE.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsH(void)
{
    return expand_vram_runs(0x9EAE);
}

void abi_ExpandStageVramRunsH(void)
{
    runs_exit_shim(ExpandStageVramRunsH());
}

/*
 * @routine 0A8E:082 ExpandStageVramRunsI
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CB8.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsI(void)
{
    return expand_vram_runs(0x9CB8);
}

void abi_ExpandStageVramRunsI(void)
{
    runs_exit_shim(ExpandStageVramRunsI());
}

/*
 * @routine 0B3E:082 ExpandStageVramRunsJ
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E79.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsJ(void)
{
    return expand_vram_runs(0x9E79);
}

void abi_ExpandStageVramRunsJ(void)
{
    runs_exit_shim(ExpandStageVramRunsJ());
}

/*
 * @routine 0BEE:082 ExpandStageVramRunsK
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CCD.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsK(void)
{
    return expand_vram_runs(0x9CCD);
}

void abi_ExpandStageVramRunsK(void)
{
    runs_exit_shim(ExpandStageVramRunsK());
}

/*
 * @routine 0C9E:082 ExpandStageVramRunsL
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E7C.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsL(void)
{
    return expand_vram_runs(0x9E7C);
}

void abi_ExpandStageVramRunsL(void)
{
    runs_exit_shim(ExpandStageVramRunsL());
}

/*
 * @routine 0D4E:082 ExpandStageVramRunsM
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CF3.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsM(void)
{
    return expand_vram_runs(0x9CF3);
}

void abi_ExpandStageVramRunsM(void)
{
    runs_exit_shim(ExpandStageVramRunsM());
}

/*
 * @routine 0DFE:082 ExpandStageVramRunsN
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9EAA.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsN(void)
{
    return expand_vram_runs(0x9EAA);
}

void abi_ExpandStageVramRunsN(void)
{
    runs_exit_shim(ExpandStageVramRunsN());
}

/*
 * @routine 0EAE:082 ExpandStageVramRunsO
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E04.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsO(void)
{
    return expand_vram_runs(0x9E04);
}

void abi_ExpandStageVramRunsO(void)
{
    runs_exit_shim(ExpandStageVramRunsO());
}

/*
 * @routine 0F5E:082 ExpandStageVramRunsP
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9F1F.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsP(void)
{
    return expand_vram_runs(0x9F1F);
}

void abi_ExpandStageVramRunsP(void)
{
    runs_exit_shim(ExpandStageVramRunsP());
}

/*
 * @routine 100E:082 ExpandStageVramRunsQ
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9C7E.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsQ(void)
{
    return expand_vram_runs(0x9C7E);
}

void abi_ExpandStageVramRunsQ(void)
{
    runs_exit_shim(ExpandStageVramRunsQ());
}

/*
 * @routine 10BE:082 ExpandStageVramRunsR
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9F09.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsR(void)
{
    return expand_vram_runs(0x9F09);
}

void abi_ExpandStageVramRunsR(void)
{
    runs_exit_shim(ExpandStageVramRunsR());
}

/*
 * @routine 116E:082 ExpandStageVramRunsS
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9C89.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsS(void)
{
    return expand_vram_runs(0x9C89);
}

void abi_ExpandStageVramRunsS(void)
{
    runs_exit_shim(ExpandStageVramRunsS());
}

/*
 * @routine 12CE:082 ExpandStageVramRunsT
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9F45.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsT(void)
{
    return expand_vram_runs(0x9F45);
}

void abi_ExpandStageVramRunsT(void)
{
    runs_exit_shim(ExpandStageVramRunsT());
}

/*
 * @routine 160E:082 ExpandStageVramRunsU
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CAE.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsU(void)
{
    return expand_vram_runs(0x9CAE);
}

void abi_ExpandStageVramRunsU(void)
{
    runs_exit_shim(ExpandStageVramRunsU());
}

/*
 * @routine 18CE:082 ExpandStageVramRunsV
 * @run 6B:8882
 * WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
 * clears it, then fills the runs the list for the current stage section (zField82) holds.
 * HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
 * two bytes per section. Each run gives its first buffer index, then its bytes with the last
 * one marked by bit 7; an $FF byte ends the list.
 * The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9EA6.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
 * position, N 0, Z 1, C 1. Clobbers zTemp00-02.
 */
StageRunsEnd ExpandStageVramRunsV(void)
{
    return expand_vram_runs(0x9EA6);
}

void abi_ExpandStageVramRunsV(void)
{
    runs_exit_shim(ExpandStageVramRunsV());
}
