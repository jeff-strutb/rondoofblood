/*
 * The flash the boss's gauge goes on to once its timer is set (sector 024E, the overlays whose VRAM routines are at
 * $937D): masks eight places of VRAM with random bits, drops the object in slot 85 and removes it at the bottom.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "stage_regs.h"

typedef StageRegs Regs;

enum { BOSS_SLOT = 85, PASS_WORDS = 0x40 };

/* A JSR to the routine loaded at an overlay address, with the registers as they stand. */
static Regs call_to(Regs r, uint16_t ret, uint16_t run)
{
    stage_regs_load_shim(r);
    call_routine(ret, run);
    return stage_regs_from_cpu_shim(r);
}

/*
 * @routine 024E:564 BossFlashMaskCell
 * @run 6B:8D64
 * WHAT IT DOES: Masks one cell of VRAM, $40 words, with random bits and a pattern by the flash's step.
 * HOW: The VRAM address from X and Y ($937D); for each word, BossFlashMasks by the step's low three bits OR a random
 * byte OR zRandom2 OR zRandom1 is both masks of AndVramData, X kept on the stack.
 * WAITS: no.
 * Entry: X and Y the address. Exit: X 0, N 0, Z 1; A, Y, C and V as the last AndVramData left them.
 */
StageRegs BossFlashMaskCell(StageRegs r)
{
    return BossFlashMaskCellAt(r, 0x8D64, 0x937D, 0x9392, BossFlashMasks);
}

/* BossFlashMaskCell's body, at the address it runs at (its JSRs return to at + 3, + $0E and + $1B), with where its
 * VRAM address and AND calls go and its table of masks: the builds of the overlay that hold these instructions at
 * another address (stage_programs_census.c) pass their own. */
StageRegs BossFlashMaskCellAt(StageRegs r, uint16_t at, uint16_t point, uint16_t and_bits, uint16_t masks)
{
    RandomResult random;
    uint8_t step, value;

    r = call_to(r, (uint16_t)(at + 3), point);
    r.x = PASS_WORDS;
    r.nz = PASS_WORDS;
    do {
        step = (uint8_t)(rd8(BossFlashStep) & 0x07);
        r.y = step;
        r = regs_loaded(r, step);
        jsr_push((uint16_t)(at + 0x0E));
        random = NextRandom();
        jsr_pop();
        stage_regs_load_shim(r);
        random_exit_shim(random);
        r = stage_regs_from_cpu_shim(r);
        value = (uint8_t)(r.a | RD8(ZP->random2));
        value = (uint8_t)(value | RD8(ZP->random1));
        value = (uint8_t)(value | rd8((uint16_t)(masks + r.y)));
        r = regs_loaded(r, value);
        push8(r.x);
        r.x = value;
        r.y = value;
        r.nz = value;
        r = call_to(r, (uint16_t)(at + 0x1B), and_bits);
        r.x = pull8();
        r.x--;
        r.nz = r.x;
    } while (r.x != 0);
    return r;
}

void abi_BossFlashMaskCell(void)
{
    stage_regs_load_shim(BossFlashMaskCell(regs_entry_shim()));
}

/*
 * @routine 024E:525 BossGaugeFlash
 * @run 6B:8D25
 * WHAT IT DOES: A frame of the boss gauge's flash: masks four cells, counts the flash's step, makes the object in
 * slot 85 fall faster and move, and at the bottom hides it and clears bit 2 of zField8E.
 * HOW: Each cell through the C of the routine loaded at $8D64 (it differs between the overlays). The cell's address from BossFlashAddresses by zTemp01 (from the wave count's bit 0, two at a time below 8);
 * OBJ_VY of slot 85 plus $0C; ApplyVelocity.
 * WAITS: no.
 * Entry: nothing. Exit: A the object's Y high, N and Z from it (RMB sets none); X 85.
 */
StageRegs BossGaugeFlash(StageRegs r)
{
    return BossGaugeFlashAt(r, 0x8D25, BossFlashAddresses, 0x8D64);
}

/* BossGaugeFlash's body, at the address it runs at (its JSRs return to at + $14 and + $34), with its table of cells
 * and where its call to the mask goes: the builds of the overlay that hold these instructions at another address
 * (stage_programs_census.c) pass their own. */
StageRegs BossGaugeFlashAt(StageRegs r, uint16_t at, uint16_t addresses, uint16_t mask_cell)
{
    AxisMove moved;
    uint8_t value;
    Sum sum;

    r = regs_loaded(r, (uint8_t)(RD8(ZP->wave_counter) & 0x01));
    WR8(ZP->temp01, r.a);
    do {
        value = RD8(ZP->temp01);
        r.c = (value & 0x80) != 0;
        r = regs_loaded(r, (uint8_t)(value << 1));
        r.x = r.a;
        r.y = rd8((uint16_t)(addresses + 1 + r.x));
        r = regs_loaded(r, rd8((uint16_t)(addresses + r.x)));
        r.x = r.a;
        r = call_to(r, (uint16_t)(at + 0x14), mask_cell);
        INC8(ZP->temp01);
        INC8(ZP->temp01);
        r = regs_loaded(r, RD8(ZP->temp01));
        r = regs_compared(r, r.a, 0x08);
    } while (!r.c);
    value = (uint8_t)(rd8(BossFlashStep) + 1);
    wr8(BossFlashStep, value);
    sum = add_bytes(RD8(OBJ->vy_lo[BOSS_SLOT]), 0x0C, false);
    WR8(OBJ->vy_lo[BOSS_SLOT], sum.value);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    if (sum.carry) {
        r.nz = INC8(OBJ->vy_hi[BOSS_SLOT]);
    }
    r.x = BOSS_SLOT;
    r.nz = BOSS_SLOT;
    jsr_push((uint16_t)(at + 0x34));
    moved = ApplyVelocity(BOSS_SLOT);
    jsr_pop();
    stage_regs_load_shim(r);
    cpu.a = axis_move_flags_shim(moved);
    abi_nz(cpu.a);
    r = stage_regs_from_cpu_shim(r);
    r = regs_loaded(r, RD8(OBJ->y_hi[BOSS_SLOT]));
    if (r.a == 0) {
        return r;
    }
    WR8(OBJ->frame[BOSS_SLOT], 0);
    RMW8(ZP->field8e, old, old & ~0x04);
    return r;
}

void abi_BossGaugeFlash(void)
{
    stage_regs_load_shim(BossGaugeFlash(regs_entry_shim()));
}
