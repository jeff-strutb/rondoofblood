/* The VDC: register selection, the display switch, scroll, reset. */
#include "machine.h"
#include "ram.h"

/* VDC registers. */
enum {
    VDC_REG_MAWR = 0x00, VDC_REG_MARR = 0x01, VDC_REG_VRAM = 0x02,
    VDC_REG_CR = 0x05, VDC_REG_RCR = 0x06, VDC_REG_BXR = 0x07,
    VDC_REG_BYR = 0x08, VDC_REG_HSR = 0x0A, VDC_REG_HDR = 0x0B
};

enum { VDC_STATUS_VBLANK = 0x20, VRAM_BLANK_TILE_LO = 0x10, VRAM_BLANK_TILE_HI = 0x01 };

/*
 * @routine 0046:366 SelectVdcRegister
 * @run 69:5B66
 * WHAT IT DOES: Selects a VDC register, remembering which so the frame
 * interrupt can put it back.
 * HOW: Stores it in zVdcReg, then writes it to the address port.
 * Entry: A register. Exit: A, X, Y and flags kept.
 */
void SelectVdcRegister(uint8_t reg)
{
    wr8(zVdcReg, reg);
    wr8(VDC_ADDR, reg);
}

void abi_SelectVdcRegister(void)
{
    SelectVdcRegister(cpu.a);
}

/*
 * @routine 0046:364 SelectVdcControl
 * @run 69:5B64
 * WHAT IT DOES: Selects the VDC's control register.
 * HOW: Register 5 into SelectVdcRegister.
 * Entry: nothing. Exit: A 5, N 0, Z 0. X and Y kept.
 */
void SelectVdcControl(void)
{
    SelectVdcRegister(VDC_REG_CR);
}

void abi_SelectVdcControl(void)
{
    SelectVdcControl();
    cpu.a = VDC_REG_CR;
    abi_nz(VDC_REG_CR);
}

/*
 * @routine 0046:361 SelectVramWriteAddress
 * @run 69:5B61
 * WHAT IT DOES: Selects the VDC's VRAM write address.
 * HOW: Register 0 into SelectVdcRegister. CLA sets no flags.
 * Entry: nothing. Exit: A 0. Flags, X and Y kept.
 */
void SelectVramWriteAddress(void)
{
    SelectVdcRegister(VDC_REG_MAWR);
}

void abi_SelectVramWriteAddress(void)
{
    SelectVramWriteAddress();
    cpu.a = VDC_REG_MAWR;
}

/*
 * @routine 0046:35D SelectVramReadAddress
 * @run 69:5B5D
 * WHAT IT DOES: Selects the VDC's VRAM read address.
 * HOW: Register 1 into SelectVdcRegister.
 * Entry: nothing. Exit: A 1, N 0, Z 0. X and Y kept.
 */
void SelectVramReadAddress(void)
{
    SelectVdcRegister(VDC_REG_MARR);
}

void abi_SelectVramReadAddress(void)
{
    SelectVramReadAddress();
    cpu.a = VDC_REG_MARR;
    abi_nz(VDC_REG_MARR);
}

/*
 * @routine 0046:359 SelectVramData
 * @run 69:5B59
 * WHAT IT DOES: Selects the VDC's VRAM data port.
 * HOW: Register 2 into SelectVdcRegister.
 * Entry: nothing. Exit: A 2, N 0, Z 0. X and Y kept.
 */
void SelectVramData(void)
{
    SelectVdcRegister(VDC_REG_VRAM);
}

void abi_SelectVramData(void)
{
    SelectVramData();
    cpu.a = VDC_REG_VRAM;
    abi_nz(VDC_REG_VRAM);
}

/*
 * @routine 0046:36C ClearVdcControlHigh
 * @run 69:5B6C
 * WHAT IT DOES: Zeroes the high byte of the VDC's control register.
 * HOW: Selects the register and writes zero to its high data port.
 * Entry: nothing. Exit: A 0, N 0, Z 1. X and Y kept.
 */
void ClearVdcControlHigh(void)
{
    SelectVdcControl();
    wr8(VDC_DATA_HI, 0);
}

void abi_ClearVdcControlHigh(void)
{
    ClearVdcControlHigh();
    cpu.a = 0;
    abi_nz(0);
}

/*
 * @routine 0043:3D8 WaitVblankDisplayOff
 * @run 69:43D8
 * WHAT IT DOES: Turns the display off at the next vertical blank, and with it
 * any raster effect.
 * HOW: Clears the status the interrupt keeps and waits until the interrupt has
 * seen vblank; then clears the frame's raster effect and writes the control
 * value the game wants, with its sprite and background enables off, without
 * changing what the game wants.
 * Entry: interrupts on. Exit: A the value written, N and Z from it. X and Y
 * kept.
 */
uint8_t WaitVblankDisplayOff(void)
{
    uint8_t control;

    wr8(zVdcStatus, 0);
    while (!(rd8(zVdcStatus) & VDC_STATUS_VBLANK))
        wait_interrupt();
    wr8(RasterEffectNow, 0);
    SelectVdcControl();
    control = rd8(zVdcCrLo) & 0x3F;
    wr8(VdcCrShadow, control);
    wr8(VDC_DATA_LO, control);
    return control;
}

void abi_WaitVblankDisplayOff(void)
{
    cpu.a = WaitVblankDisplayOff();
    abi_nz(cpu.a);
}

/*
 * @routine 0043:3EE WriteVdcControl
 * @run 69:43EE
 * WHAT IT DOES: Writes the VDC control value the game wants.
 * HOW: Selects the register and writes both bytes.
 * Entry: nothing. Exit: A the high byte, N and Z from it. X and Y kept.
 */
uint8_t WriteVdcControl(void)
{
    uint8_t byte;

    SelectVdcControl();
    byte = rd8(zVdcCrLo);
    wr8(VDC_DATA_LO, byte);
    byte = rd8(zVdcCrHi);
    wr8(VDC_DATA_HI, byte);
    return byte;
}

void abi_WriteVdcControl(void)
{
    cpu.a = WriteVdcControl();
    abi_nz(cpu.a);
}

/*
 * @routine 0043:2D4 ResetDisplayControl
 * @run 69:42D4
 * WHAT IT DOES: Sets up the display to come on: the game's control value becomes
 * sprites and background on with the vblank interrupt enabled, scroll zero, no
 * raster effect, and a four frame delay. Only the interrupt enable reaches the
 * VDC now; the frame interrupt turns the display on when the delay runs out.
 * HOW: Writes $C8 as the wanted value and its low nibble to the VDC.
 * Entry: nothing. Exit: A 4, N 0, Z 0. X and Y kept.
 */
void ResetDisplayControl(void)
{
    SelectVdcControl();
    wr8(zVdcCrLo, 0xC8);
    wr8(VDC_DATA_LO, 0xC8 & 0x0F);
    wr8(zVdcCrHi, 0);
    wr8(VDC_DATA_HI, 0);
    wr8(ScrollXLo, 0);
    wr8(ScrollXHi, 0);
    wr8(ScrollYLo, 0);
    wr8(ScrollYHi, 0);
    wr8(zRasterEffect, 0);
    wr8(zDisplayDelay, 4);
}

void abi_ResetDisplayControl(void)
{
    ResetDisplayControl();
    cpu.a = 4;
    abi_nz(4);
}

/*
 * @routine 0043:2F9 WriteBandScroll
 * @run 69:42F9
 * WHAT IT DOES: Writes one raster band's background scroll to the VDC.
 * HOW: BXR then BYR, from the bands' parallel byte arrays.
 * Entry: X band. Exit: A BYR's high byte, N and Z from it. X and Y kept.
 */
uint8_t WriteBandScroll(uint8_t band)
{
    uint8_t byte;

    SelectVdcRegister(VDC_REG_BXR);
    byte = rd8((uint16_t)(ScrollBgxLo + band));
    wr8(VDC_DATA_LO, byte);
    byte = rd8((uint16_t)(ScrollBgxHi + band));
    wr8(VDC_DATA_HI, byte);
    SelectVdcRegister(VDC_REG_BYR);
    byte = rd8((uint16_t)(ScrollBgyLo + band));
    wr8(VDC_DATA_LO, byte);
    byte = rd8((uint16_t)(ScrollBgyHi + band));
    wr8(VDC_DATA_HI, byte);
    return byte;
}

void abi_WriteBandScroll(void)
{
    cpu.a = WriteBandScroll(cpu.x);
    abi_nz(cpu.a);
}

/*
 * @routine 0043:31C WriteScreenTiming
 * @run 69:431C
 * WHAT IT DOES: Sets the VDC's horizontal timing for a screen width.
 * HOW: HSR and HDR, each a word from a table by timing index times two.
 * Entry: Y index. Exit: A HDR's high byte, N and Z from it. X and Y kept.
 */
uint8_t WriteScreenTiming(uint8_t index)
{
    uint8_t byte;

    SelectVdcRegister(VDC_REG_HSR);
    byte = rd8((uint16_t)(VdcHsrTable + index));
    wr8(VDC_DATA_LO, byte);
    byte = rd8((uint16_t)(VdcHsrTable + 1 + index));
    wr8(VDC_DATA_HI, byte);
    SelectVdcRegister(VDC_REG_HDR);
    byte = rd8((uint16_t)(VdcHdrTable + index));
    wr8(VDC_DATA_LO, byte);
    byte = rd8((uint16_t)(VdcHdrTable + 1 + index));
    wr8(VDC_DATA_HI, byte);
    return byte;
}

void abi_WriteScreenTiming(void)
{
    cpu.a = WriteScreenTiming(cpu.y);
    abi_nz(cpu.a);
}

/*
 * @routine 0046:3A0 ClearScreenState
 * @run 69:5BA0
 * WHAT IT DOES: Blanks the screen for a new scene: display off, no raster
 * effect, the scroll and band tables zero, and the whole background map filled
 * with the blank tile.
 * HOW: Waits for vblank with the display off, block-fills $3E00 with zero, sets
 * the VRAM write address to 0 and writes tile $0110 $1000 times.
 * Entry: interrupts on. Exit: A 2, X 0, Y $FF, N 1, Z 0.
 */
void ClearScreenState(void)
{
    WaitVblankDisplayOff();
    wr8(zDisplayDelay, 4);
    wr8(zRasterEffect, 0);
    wr8(zVideoMode, 0);
    tai(KernelZeros, ScrollWanted, 0x00EE);
    ClearVdcControlHigh();
    SelectVramWriteAddress();
    st1(0);
    st2(0);
    SelectVramData();
    for (int count = 0; count < 0x1000; count++) {
        st1(VRAM_BLANK_TILE_LO);
        st2(VRAM_BLANK_TILE_HI);
    }
}

void abi_ClearScreenState(void)
{
    ClearScreenState();
    cpu.a = VDC_REG_VRAM;
    cpu.x = 0;
    cpu.y = 0xFF;
    abi_nz(0xFF);
}

/*
 * @routine 0046:375 ResetGameState
 * @run 69:5B75
 * WHAT IT DOES: Starts over from nothing: display off, the VCE's first colours
 * black, the level pointer and all game RAM zero, then a blank screen.
 * HOW: Waits for vblank with the display off, zeroes colours 0 and $100,
 * block-fills $26D0 and $26F3 up with zero, then runs into ClearScreenState.
 * Entry: interrupts on. Exit: as ClearScreenState.
 */
void ResetGameState(void)
{
    WaitVblankDisplayOff();
    wr8(VCE_ADDR_LO, 0);
    wr8(VCE_ADDR_HI, 0);
    wr8(VCE_DATA_LO, 0);
    wr8(VCE_DATA_HI, 0);
    wr8(VCE_ADDR_LO, 0);
    wr8(VCE_ADDR_HI, 1);
    wr8(VCE_DATA_LO, 0);
    wr8(VCE_DATA_HI, 0);
    tai(KernelZeros, LevelStreamLo, 0x0003);
    tai(KernelZeros, GameRamStart, 0x164D);
    ClearScreenState();
}

void abi_ResetGameState(void)
{
    ResetGameState();
    cpu.a = VDC_REG_VRAM;
    cpu.x = 0;
    cpu.y = 0xFF;
    abi_nz(0xFF);
}
