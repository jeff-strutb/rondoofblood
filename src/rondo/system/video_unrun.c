/* The VDC: kernel routines no gameplay script runs, covered by unit calls (tools/rondo/scripts/unrun_kernel.txt). */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

enum {
    VRAM_LIST_END = 0xFF, VRAM_LIST_ADDRESS = 0xFE, VRAM_LIST_HIGH = 0xFD,
    VRAM_BLANK_TILE_LO = 0x10, VRAM_BLANK_TILE_HI = 0x01
};

typedef struct {
    uint8_t high, index;
} VramListEnd;

/*
 * @routine 0046:3CD WriteVramList
 * @run 69:5BCD
 * WHAT IT DOES: Writes a list of VRAM words: a write address, then words whose low byte is a
 * list byte and whose high byte is held; with carry set at entry every word is the blank tile
 * ($0110) instead. No JSR, JMP or table on the disc names it: the census found it after the
 * return of the routine before it, and no input reaches it, so its script runs it by unit calls.
 * HOW: Keeps the carry in bit 0 of zTemp02 (tested again for every word). Clears the control
 * register's high byte, then from the list at the pointer in zTemp00: two bytes of VRAM write
 * address, then bytes, each one word; $FF ends the list, $FE is followed by a new address, $FD
 * by a new high byte.
 * Entry: zTemp00-01 the list, X the first high byte, C erase. The list is indexed by Y from 0,
 * so it is at most 256 bytes.
 * Exit: A $FF, N 0, Z 1, C 1, V kept; X the high byte last held, Y the index past the end.
 * WAITS: no.
 */
VramListEnd WriteVramList(uint8_t high, bool erase)
{
    VramListEnd end;
    uint8_t index = 0;
    uint8_t code;

    WR8(ZP->temp02, 0);
    ROL8(ZP->temp02, erase);
    jsr_push(0x5BD4);
    ClearVdcControlHigh();
    jsr_pop();
    for (;;) {
        jsr_push(0x5BD8);
        SelectVramWriteAddress();
        jsr_pop();
        WR8(VDC->data_lo, RD8_PTR_Y(ZP->temp00, index));
        index++;
        WR8(VDC->data_hi, RD8_PTR_Y(ZP->temp00, index));
        index++;
        jsr_push(0x5BE7);
        SelectVramData();
        jsr_pop();
        for (;;) {
            code = RD8_PTR_Y(ZP->temp00, index);
            index++;
            if (code == VRAM_LIST_END) {
                end.high = high;
                end.index = index;
                return end;
            }
            if (code == VRAM_LIST_ADDRESS) {
                break;
            }
            if (code == VRAM_LIST_HIGH) {
                high = RD8_PTR_Y(ZP->temp00, index);
                index++;
                continue;
            }
            if (RD8(ZP->temp02) & 0x01) {
                code = VRAM_BLANK_TILE_LO;
                high = VRAM_BLANK_TILE_HI;
            }
            WR8(VDC->data_lo, code);
            WR8(VDC->data_hi, high);
        }
    }
}

void abi_WriteVramList(void)
{
    VramListEnd end = WriteVramList(cpu.x, (cpu.p & P_C) != 0);

    cpu.a = VRAM_LIST_END;
    cpu.x = end.high;
    cpu.y = end.index;
    abi_nz(0);                          /* CMP #$FF on $FF: equal */
    abi_flag(P_C, true);
}
