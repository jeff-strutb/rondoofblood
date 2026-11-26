/* The stage overlay's scroll step: the overlay's banks mapped, then the stream stepped with the scroll. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum { MPR5 = 0x20, MPR6 = 0x40, FIRST_BANK_OFFSET = 8, SCROLL_STEP_RETURN = 0x8A4E, SCROLL_STEP_RUN = 0x8A36 };

/* Maps a bank and the one after it into MPR5 and MPR6, from the offset above the RAM base bank. */
static bool map_bank_pair(uint8_t offset)
{
    Sum sum = add_bytes(offset, RD8(ZP->ram_base_bank), false);

    tam(MPR5, sum.value);
    tam(MPR6, (uint8_t)(sum.value + 1));
    return sum.overflow;
}

/* Maps the first pair, the overlay's second pair, and steps the stream with the scroll. */
StageScrollEnd ScrollStepBody(uint8_t x, const ScrollStepLayout *layout)
{
    StageScrollEnd end;

    map_bank_pair(FIRST_BANK_OFFSET);
    end.v_before = map_bank_pair(rd8(layout->second_offset_at));
    jsr_push(layout->step_return);
    end.walk = StreamScrollStep(x);
    jsr_pop();
    return end;
}

/* 024E's layout: the call's return address is $8A4E. */
static StageScrollEnd scroll_step_with_banks(uint16_t second_offset_at, uint8_t x)
{
    ScrollStepLayout layout = { second_offset_at, SCROLL_STEP_RETURN };

    return ScrollStepBody(x, &layout);
}

/*
 * @routine 024E:236 StageScrollStep
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStep(uint8_t x)
{
    return scroll_step_with_banks(0xAC95, x);
}

void abi_StageScrollStep(void)
{
    StageScrollEnd end = StageScrollStep(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 050E:236 StageScrollStepB
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACCA.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepB(uint8_t x)
{
    return scroll_step_with_banks(0xACCA, x);
}

void abi_StageScrollStepB(void)
{
    StageScrollEnd end = StageScrollStepB(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 05BE:236 StageScrollStepC
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC0E.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepC(uint8_t x)
{
    return scroll_step_with_banks(0xAC0E, x);
}

void abi_StageScrollStepC(void)
{
    StageScrollEnd end = StageScrollStepC(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 071E:236 StageScrollStepD
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC16.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepD(uint8_t x)
{
    return scroll_step_with_banks(0xAC16, x);
}

void abi_StageScrollStepD(void)
{
    StageScrollEnd end = StageScrollStepD(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 07CE:236 StageScrollStepE
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $ABDC.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepE(uint8_t x)
{
    return scroll_step_with_banks(0xABDC, x);
}

void abi_StageScrollStepE(void)
{
    StageScrollEnd end = StageScrollStepE(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 087E:236 StageScrollStepF
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AB03.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepF(uint8_t x)
{
    return scroll_step_with_banks(0xAB03, x);
}

void abi_StageScrollStepF(void)
{
    StageScrollEnd end = StageScrollStepF(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 092E:236 StageScrollStepG
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC8E.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepG(uint8_t x)
{
    return scroll_step_with_banks(0xAC8E, x);
}

void abi_StageScrollStepG(void)
{
    StageScrollEnd end = StageScrollStepG(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 09DE:236 StageScrollStepH
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC5E.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepH(uint8_t x)
{
    return scroll_step_with_banks(0xAC5E, x);
}

void abi_StageScrollStepH(void)
{
    StageScrollEnd end = StageScrollStepH(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 0A8E:236 StageScrollStepI
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACE3.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepI(uint8_t x)
{
    return scroll_step_with_banks(0xACE3, x);
}

void abi_StageScrollStepI(void)
{
    StageScrollEnd end = StageScrollStepI(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 0B3E:236 StageScrollStepJ
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AB27.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepJ(uint8_t x)
{
    return scroll_step_with_banks(0xAB27, x);
}

void abi_StageScrollStepJ(void)
{
    StageScrollEnd end = StageScrollStepJ(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 0BEE:236 StageScrollStepK
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACA7.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepK(uint8_t x)
{
    return scroll_step_with_banks(0xACA7, x);
}

void abi_StageScrollStepK(void)
{
    StageScrollEnd end = StageScrollStepK(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 0C9E:236 StageScrollStepL
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC8B.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepL(uint8_t x)
{
    return scroll_step_with_banks(0xAC8B, x);
}

void abi_StageScrollStepL(void)
{
    StageScrollEnd end = StageScrollStepL(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 0D4E:236 StageScrollStepM
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD91.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepM(uint8_t x)
{
    return scroll_step_with_banks(0xAD91, x);
}

void abi_StageScrollStepM(void)
{
    StageScrollEnd end = StageScrollStepM(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 0DFE:236 StageScrollStepN
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AB70.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepN(uint8_t x)
{
    return scroll_step_with_banks(0xAB70, x);
}

void abi_StageScrollStepN(void)
{
    StageScrollEnd end = StageScrollStepN(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 0EAE:236 StageScrollStepO
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD3F.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepO(uint8_t x)
{
    return scroll_step_with_banks(0xAD3F, x);
}

void abi_StageScrollStepO(void)
{
    StageScrollEnd end = StageScrollStepO(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 0F5E:236 StageScrollStepP
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $ABA7.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepP(uint8_t x)
{
    return scroll_step_with_banks(0xABA7, x);
}

void abi_StageScrollStepP(void)
{
    StageScrollEnd end = StageScrollStepP(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 100E:236 StageScrollStepQ
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD1B.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepQ(uint8_t x)
{
    return scroll_step_with_banks(0xAD1B, x);
}

void abi_StageScrollStepQ(void)
{
    StageScrollEnd end = StageScrollStepQ(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 10BE:236 StageScrollStepR
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACC9.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepR(uint8_t x)
{
    return scroll_step_with_banks(0xACC9, x);
}

void abi_StageScrollStepR(void)
{
    StageScrollEnd end = StageScrollStepR(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 116E:236 StageScrollStepS
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD36.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepS(uint8_t x)
{
    return scroll_step_with_banks(0xAD36, x);
}

void abi_StageScrollStepS(void)
{
    StageScrollEnd end = StageScrollStepS(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 12CE:236 StageScrollStepT
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACF7.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepT(uint8_t x)
{
    return scroll_step_with_banks(0xACF7, x);
}

void abi_StageScrollStepT(void)
{
    StageScrollEnd end = StageScrollStepT(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 160E:236 StageScrollStepU
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD31.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepU(uint8_t x)
{
    return scroll_step_with_banks(0xAD31, x);
}

void abi_StageScrollStepU(void)
{
    StageScrollEnd end = StageScrollStepU(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 16BE:236 StageScrollStepV
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC65.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepV(uint8_t x)
{
    return scroll_step_with_banks(0xAC65, x);
}

void abi_StageScrollStepV(void)
{
    StageScrollEnd end = StageScrollStepV(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/*
 * @routine 18CE:236 StageScrollStepW
 * @run 6B:8A36
 * WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
 * bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
 * stream with the scroll.
 * HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
 * step is StreamScrollStep, called through its vector.
 * The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC49.
 * WAITS: it can wait where a handler's code does.
 * Entry: X the caller's. Exit: as StreamScrollStep.
 */
StageScrollEnd StageScrollStepW(uint8_t x)
{
    return scroll_step_with_banks(0xAC49, x);
}

void abi_StageScrollStepW(void)
{
    StageScrollEnd end = StageScrollStepW(cpu.x);

    cpu.y = 1;
    abi_flag(P_V, end.v_before);
    stream_walk_exit_shim(end.walk);
}

/* Each overlay's copy of the step, by where it sits on the disc. */
typedef struct {
    uint32_t disc;
    StageScrollEnd (*step)(uint8_t x);
} ScrollStepVariant;

static const ScrollStepVariant scroll_step_variants[] = {
    { DISC(0x024E, 0x236), StageScrollStep },
    { DISC(0x050E, 0x236), StageScrollStepB },
    { DISC(0x05BE, 0x236), StageScrollStepC },
    { DISC(0x071E, 0x236), StageScrollStepD },
    { DISC(0x07CE, 0x236), StageScrollStepE },
    { DISC(0x087E, 0x236), StageScrollStepF },
    { DISC(0x092E, 0x236), StageScrollStepG },
    { DISC(0x09DE, 0x236), StageScrollStepH },
    { DISC(0x0A8E, 0x236), StageScrollStepI },
    { DISC(0x0B3E, 0x236), StageScrollStepJ },
    { DISC(0x0BEE, 0x236), StageScrollStepK },
    { DISC(0x0C9E, 0x236), StageScrollStepL },
    { DISC(0x0D4E, 0x236), StageScrollStepM },
    { DISC(0x0DFE, 0x236), StageScrollStepN },
    { DISC(0x0EAE, 0x236), StageScrollStepO },
    { DISC(0x0F5E, 0x236), StageScrollStepP },
    { DISC(0x100E, 0x236), StageScrollStepQ },
    { DISC(0x10BE, 0x236), StageScrollStepR },
    { DISC(0x116E, 0x236), StageScrollStepS },
    { DISC(0x12CE, 0x236), StageScrollStepT },
    { DISC(0x160E, 0x236), StageScrollStepU },
    { DISC(0x16BE, 0x236), StageScrollStepV },
    { DISC(0x18CE, 0x236), StageScrollStepW },
};

/*
 * The step of whichever stage overlay is in RAM at $8A36. A copy no table row names is original
 * code the machine runs; the caller's X goes in and the walk's X comes back.
 */
static StageScrollEnd original_scroll_step_shim(uint8_t x)
{
    StageScrollEnd end;

    cpu.x = x;
    call_original(SCROLL_STEP_RUN);
    end.walk.how = STEP_NO_SPEED;
    end.walk.stepped = false;
    end.walk.distance_v = false;
    end.walk.cmp_a = 0;
    end.walk.cmp_b = 0;
    end.walk.last.value = 0;
    end.walk.last.carry = false;
    end.walk.last.overflow = false;
    end.walk.x = cpu.x;
    end.v_before = (cpu.p & P_V) != 0;
    return end;
}

StageScrollEnd StageScrollStepResident(uint8_t x)
{
    uint32_t disc = resident_at(SCROLL_STEP_RUN);
    unsigned i;

    for (i = 0; i < sizeof scroll_step_variants / sizeof scroll_step_variants[0]; i++) {
        if (scroll_step_variants[i].disc == disc) {
            return scroll_step_variants[i].step(x);
        }
    }
    return original_scroll_step_shim(x);
}
