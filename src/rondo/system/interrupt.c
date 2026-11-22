/*
 * The kernel's start and the frame interrupt, which runs the whole game: after
 * start the main program only idles, and everything happens from vblank.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"

enum {
    STATUS_RASTER = 0x04, STATUS_VBLANK = 0x20, CR_RASTER_ENABLE = 0x04,
    CR_VBLANK_ENABLE = 0x08, MPR3 = 0x08, MPR4 = 0x10, MPR5 = 0x20, MPR6 = 0x40,
    RCR_LINE_BASE = 0x40, VDC_REG_RCR = 0x06, VDC_REG_VSYNC_DMA = 0x13,
    SCREEN_COLUMNS = 0x20, SCREEN_ROWS = 0x1E, SCENE_MENU_A = 4, SCENE_MENU_B = 6, ADPCM_MENU = 0x24,
    VECTOR_VDC = 1, VECTOR_TIMER = 2,
    KERNEL_RUN = 0x4000, FRAME_INTERRUPT_RUN = 0x4177, IGNORED_INTERRUPT_RUN = 0x4297
};

void abi_KernelIdle(void);

static void map_from_base(uint8_t mpr_bit, uint8_t offset)
{
    uint8_t base = rd8(zRamBaseBank);

    tam(mpr_bit, (uint8_t)(offset + base));
}

/* Sets the raster compare line from a RAM copy and enables its interrupt. */
static void write_raster_line(void)
{
    Sum low;

    wr8(RasterStep, 0);
    SelectVdcRegister(VDC_REG_RCR);
    low = add_bytes(rd8(RasterLine), RCR_LINE_BASE, false);
    wr8(VDC_DATA_LO, low.value);
    wr8(VDC_DATA_HI, low.carry ? 1 : 0);
}

static void set_raster_enable(void)
{
    uint8_t control = rd8(zVdcCrLo);

    wr8(zVdcCrLo, (uint8_t)(control | CR_RASTER_ENABLE));
}

static void clear_raster_enable(void)
{
    uint8_t control = rd8(zVdcCrLo);

    wr8(zVdcCrLo, (uint8_t)(control & ~CR_RASTER_ENABLE));
}

/*
 * @routine 0043:298 LateFrame
 * @run 69:4298
 * WHAT IT DOES: A frame that began while the last was still running: only video and
 * sound are kept up, and the game's logic loses the frame.
 * HOW: Rewrites the raster line, the display control and the first band's scroll
 * from their RAM copies, ticks the sound driver, and joins FrameInterrupt's exit.
 * Entry: from FrameInterrupt, its saves on the stack. Exit: through its RTI.
 */
static void late_frame(void)
{
    uint8_t byte;

    clear_raster_enable();
    byte = rd8(RasterEffectNow);
    if (byte != 0) {
        write_raster_line();
        set_raster_enable();
    }
    SelectVdcControl();
    byte = rd8(VdcCrShadow);
    wr8(VDC_DATA_LO, (uint8_t)(byte | CR_VBLANK_ENABLE));
    WriteBandScroll(0);
    SoundFrameTick();
}

/* A frame on time: video, then the game's logic with interrupts on. */
static void full_frame(void)
{
    static const uint16_t wanted[] = {
        ScrollXFrac, ScrollXLo, ScrollXHi, ScrollYFrac, ScrollYLo, ScrollYHi
    };
    static const uint16_t bands[] = {
        ScrollBandFirst, ScrollBgxLo, ScrollBgxHi, ScrollBandFourth, ScrollBgyLo, ScrollBgyHi
    };
    uint8_t byte;
    uint8_t mode;
    uint8_t delay;

    byte = (uint8_t)(rd8(zFrameBusy) + 1);
    wr8(zFrameBusy, byte);
    mode = rd8(zVideoMode);
    byte = rd8((uint16_t)(VceControlTable + mode));
    wr8(VCE_CONTROL, byte ^ 0x04);
    WriteScreenTiming((uint8_t)(mode << 1));
    clear_raster_enable();
    byte = rd8(zRasterEffect);
    wr8(RasterEffectNow, byte);
    if (byte != 0) {
        uint8_t effect;
        uint8_t last;

        byte = rd8(zRasterArg);
        wr8(RasterLine, byte);
        write_raster_line();
        RasterStart(&effect, &last);
        set_raster_enable();
    }
    byte = rd8(zVdcCrLo);
    delay = rd8(zDisplayDelay);
    if (delay != 0) {
        delay = (uint8_t)(rd8(zDisplayDelay) - 1);
        wr8(zDisplayDelay, delay);
        if (delay != 0)
            byte &= 0x3F;
    }
    byte |= CR_VBLANK_ENABLE;
    SelectVdcControl();
    wr8(VdcCrShadow, byte);
    wr8(VDC_DATA_LO, byte);
    for (unsigned i = 0; i < sizeof wanted / sizeof wanted[0]; i++) {
        byte = rd8(wanted[i]);
        wr8(bands[i], byte);
    }
    WriteBandScroll(0);
    map_from_base(MPR4, 0x04);
    map_from_base(MPR5, 0x07);
    map_from_base(MPR6, 0x0C);
    call_original(FrameOverlay8003);
    call_original(FrameOverlay8000);
    SoundFrameTick();
    call_original(BiosE063);
    NextRandom();
    map_from_base(MPR4, 0x03);
    abi_FrameModeDispatch();
    map_from_base(MPR3, 0x00);
    map_from_base(MPR4, 0x0A);
    map_from_base(MPR5, 0x07);
    call_original(FrameOverlay8000);
    map_from_base(MPR3, 0x02);
    SelectVdcRegister(VDC_REG_VSYNC_DMA);
    st2(0x10);
    wr8(zFrameBusy, 0);
    wr8(zSoundBusy, 0);
    byte = rd8(ScreenWideFlag);
    if (byte != 0)
        WriteScreenTiming(2);
}

/*
 * @routine 0043:177 FrameInterrupt
 * @run 69:4177
 * WHAT IT DOES: The VDC's interrupt. A raster interrupt goes to the raster handler.
 * A vblank runs a frame: all of video first, then with interrupts on the game's
 * objects, sound, random numbers and mode logic. If the last frame is still
 * running, only video and sound are kept up, and the frame is dropped.
 * HOW: Saves everything including three MPRs and the selected VDC register, reads
 * the VDC's status to zVdcStatus, and branches on its bits.
 * Entry: interrupt. Exit: RTI; every register, flag, MPR and the VDC selection as
 * they were. Clobbers what the frame's work does.
 */
void FrameInterrupt(void)
{
    uint8_t mpr4 = tma(MPR4);
    uint8_t mpr5 = tma(MPR5);
    uint8_t mpr6 = tma(MPR6);
    uint8_t reg = rd8(zVdcReg);
    uint8_t status = rd8(VDC_ADDR);
    uint8_t byte;

    wr8(zVdcStatus, status);
    if (status & STATUS_RASTER) {
        RasterInterrupt();
    } else if (status & STATUS_VBLANK) {
        cli();
        byte = (uint8_t)(rd8(zFrameCount) + 1);
        wr8(zFrameCount, byte);
        byte = rd8(zFrameBusy);
        if (byte == 0)
            full_frame();
        else
            late_frame();
    }
    SelectVdcRegister(reg);
    tam(MPR6, mpr6);
    tam(MPR5, mpr5);
    tam(MPR4, mpr4);
}

void abi_FrameInterrupt(void)
{
    Cpu saved = cpu;

    FrameInterrupt();
    cpu = saved;
}

/*
 * @routine 0043:297 IgnoredInterrupt
 * @run 69:4297
 * WHAT IT DOES: The handler for the interrupt the game has no use for: returns.
 * HOW: RTI.
 * Entry: interrupt. Exit: everything as it was.
 */
void abi_IgnoredInterrupt(void)
{
}

/*
 * @routine 0043:0F9 KernelStart
 * @run 69:40F9
 * WHAT IT DOES: Where the loader lands: zeroes RAM, starts the System Card's pad and
 * display services and the sound driver, sets the display and first palette, installs
 * the interrupts, and then idles forever, the game running from the frame interrupt.
 * HOW: Block fills of zero page and game RAM, then BIOS and kernel calls in order.
 * WAITS: KernelIdle, a jump to itself, entered by this routine's last JMP; this routine has
 * pushed nothing there, so the stack is as the loader left it.
 * Entry: from the loader, zRamBaseBank set, MPR2-3 the kernel. Exit: never.
 */
void KernelStart(void)
{
    uint8_t base;
    Alu sum;

    tai(KernelZeros, ADDR_OF(ZP->temp00), ADDR_OF(ZP->ram_base_bank) - ADDR_OF(ZP->temp00));
    tai(KernelZeros, ADDR_OF(SNDS->command_in), KERNEL_RUN - ADDR_OF(SNDS->command_in));
    /* The calls below are made to code with no typed C yet, each with the registers and flags the assembly has
     * at its JSR (call_orig); SelectVdcRegister is typed C and is called with its frame. */
    cpu.a = 0;
    cpu.x = SCREEN_COLUMNS;
    cpu.y = SCREEN_ROWS;
    abi_nz(SCREEN_ROWS);
    call_orig(0x410F, 0xE06F);                      /* ex_scrmod */
    cpu.a = 0;
    call_orig(0x4113, 0xE06C);                      /* ex_dotmod */
    WR16_LH(BIOS->screen, 0x1000);
    cpu.a = 0x10;
    abi_nz(0x10);
    call_orig(0x411E, 0xE0A2);                      /* ex_satclr */
    WR8(ZP->game_mode, 0);
    sum = adc8(0x0D, RD8(ZP->ram_base_bank), false);
    base = sum.a;
    tam(MPR4, base);
    tam(MPR5, (uint8_t)(base + 1));
    tam(MPR6, (uint8_t)(base + 2));
    cpu.a = (uint8_t)(base + 2);
    abi_nz(cpu.a);
    abi_flag(P_C, sum.c);
    abi_flag(P_V, sum.v);
    call_orig(0x4132, 0xB233);                      /* SoundDrvInit */
    call_orig(0x4135, 0x42D4);                      /* ResetDisplayControl */
    abi_nz(0x0D);
    jsr_push(0x413A);
    SelectVdcRegister(0x0D);
    jsr_pop();
    WR8(VDC->data_lo, 0xE0);
    cpu.a = SCENE_MENU_A;
    abi_nz(SCENE_MENU_A);
    call_orig(0x4144, 0x6FD8);                      /* LoadSceneScript */
    cpu.a = SCENE_MENU_B;
    abi_nz(SCENE_MENU_B);
    call_orig(0x4149, 0x6FD8);
    cpu.a = ADPCM_MENU;
    abi_nz(ADPCM_MENU);
    call_orig(0x414E, 0x6E43);                      /* AdpcmLoad */
    WR8(ZP->music_wanted, 0xFF);
    cpu.a = VECTOR_VDC;
    cpu.x = (uint8_t)FRAME_INTERRUPT_RUN;
    cpu.y = (uint8_t)(FRAME_INTERRUPT_RUN >> 8);
    abi_nz(cpu.y);
    call_orig(0x415B, 0xE05D);                      /* ex_setvec */
    cpu.a = VECTOR_TIMER;
    cpu.x = (uint8_t)IGNORED_INTERRUPT_RUN;
    cpu.y = (uint8_t)(IGNORED_INTERRUPT_RUN >> 8);
    abi_nz(cpu.y);
    call_orig(0x4164, 0xE05D);
    WR8(ZP->bios_irq_disable, 0x06);
    sum = adc8(0x03, RD8(ZP->ram_base_bank), false);
    tam(MPR4, sum.a);
    wr8(zIdleFlag, 0);
    abi_KernelIdle();                               /* JMP KernelIdle */
}

void abi_KernelStart(void)
{
    KernelStart();
}

/*
 * @routine 0043:174 KernelIdle
 * @run 69:4174
 * WHAT IT DOES: The main program's whole life after start: nothing, forever.
 * HOW: A jump to itself; interrupts do the work.
 * Entry: interrupts on. Exit: never.
 */
void abi_KernelIdle(void)
{
    for (;;)
        wait_interrupt();
}

void abi_LateFrame(void)
{
    late_frame();
}
