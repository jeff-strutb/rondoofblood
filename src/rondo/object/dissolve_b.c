/*
 * A dissolve of VRAM patterns, as stages $0A to $0E and the secret stages hold it: on its frames an object makes one
 * mask of a 64 step order and ANDs it into every fourth word of the patterns, so after 64 steps they are gone. Also
 * the phase dispatcher that first sets PlayerKeepsLastPoint.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "dissolve_b.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }

/* PlayerKeepsLastPoint set, then the object's phase's handler from the table. */
static Jump keep_point_jump(uint8_t slot, uint16_t table)
{
    JumpTableLayout layout = { table };

    wr8(PlayerKeepsLastPoint, 1);
    return JumpByPhaseTableBody(slot, &layout);
}
static void ldx(uint8_t value) { cpu.x = value; abi_nz(value); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

static void add(uint8_t value, bool carry_in)
{
    Sum sum = add_bytes(cpu.a, value, carry_in);

    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

/* The 64 VRAM words from zBiosCl-Ch on: every fourth one, from the row zTemp00 names, ANDed with zTemp01-02. */
void DissolveWordsAt(uint16_t at)
{
    push8(cpu.y);
    cpu.y = 0;
    do {
        lda(cpu.y);
        lda((uint8_t)(cpu.a & 3));
        cmp(cpu.a, RD8(ZP->temp00));
        if ((cpu.p & P_Z) != 0) {
            jsr_push((uint16_t)(at + 0x0C));
            SelectVramWriteAddress();
            cpu.a = 0x00;
            jsr_pop();
            lda(RD8(ZP->bios_c.lo));
            WR8(VDC->data_lo, cpu.a);
            lda(RD8(ZP->bios_c.hi));
            WR8(VDC->data_hi, cpu.a);
            jsr_push((uint16_t)(at + 0x19));
            SelectVramReadAddress();
            lda(0x01);
            jsr_pop();
            lda(RD8(ZP->bios_c.lo));
            WR8(VDC->data_lo, cpu.a);
            lda(RD8(ZP->bios_c.hi));
            WR8(VDC->data_hi, cpu.a);
            jsr_push((uint16_t)(at + 0x26));
            SelectVramData();
            lda(0x02);
            jsr_pop();
            lda(RD8(VDC->data_lo));
            lda((uint8_t)(cpu.a & RD8(ZP->temp01)));
            WR8(VDC->data_lo, cpu.a);
            lda(RD8(VDC->data_hi));
            lda((uint8_t)(cpu.a & RD8(ZP->temp02)));
            WR8(VDC->data_hi, cpu.a);
        }
        lda(RD8(ZP->bios_c.lo));
        add(1, false);
        WR8(ZP->bios_c.lo, cpu.a);
        lda(RD8(ZP->bios_c.hi));
        add(0, (cpu.p & P_C) != 0);
        WR8(ZP->bios_c.hi, cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x40);
    } while (cpu.y < 0x40);
    cpu.y = pull8();
    abi_nz(cpu.y);
}

/* What differs between the copies of the step: the object columns, the frames it runs on and the phase's step. */
typedef struct { uint16_t clock, phase, table; uint8_t mask, step; } Dissolve;

/* On its frames: the next mask applied to the patterns zero-page $26FF to $2700 name, and the phase stepped. */
static void dissolve_step(uint8_t slot, const Dissolve *d, void (*words)(void), uint16_t at)
{
    lda(RD8_IDX(d->clock, slot));
    lda((uint8_t)(cpu.a & d->mask));
    if (cpu.a != 0) {
        return;
    }
    push8(cpu.x);
    lda(RD8_IDX(d->phase, slot));
    jsr_push((uint16_t)(at + 0x0E));
    DissolveMask();
    jsr_pop();
    ldx(rd8(WorkByteA));
    do {
        cpu.a = cpu.x;
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        cpu.y = cpu.a;
        lda(rd8((uint16_t)(d->table + cpu.y)));
        WR8(ZP->bios_c.lo, cpu.a);
        lda(rd8((uint16_t)(d->table + 1 + cpu.y)));
        WR8(ZP->bios_c.hi, cpu.a);
        jsr_push((uint16_t)(at + 0x21));
        words();
        jsr_pop();
        cpu.x = (uint8_t)(cpu.x + 1);
        cmp(cpu.x, rd8(WorkByteB));
    } while ((cpu.p & P_C) == 0);
    ldx(pull8());
    lda(RD8_IDX(d->phase, slot));
    add(d->step, false);
    lda((uint8_t)(cpu.a & 0x3F));
    WR8_IDX(d->phase, slot, cpu.a);
}

/*
 * @routine 0E0A:235 DissolveMask
 * @run 70:AA35
 * WHAT IT DOES: Makes the mask of a dissolve step from A (0 to 63): the row in zTemp00 (A's bits 4 and 5) and a word in
 * zTemp01-02 with every bit set but the one A's low four bits name.
 * HOW: A bit shifted up from 1 and the word inverted. The same bytes follow every copy of the dissolve.
 * WAITS: no.
 * Entry: A the phase. Exit: A zTemp02's byte, N and Z from it; C from the last shift; X, Y and V kept.
 */
void DissolveMask(void)
{
    uint8_t value;

    push8(cpu.a);
    abi_flag(P_C, (cpu.a & 0x08) != 0);
    lda((uint8_t)(cpu.a >> 4));
    WR8(ZP->temp00, cpu.a);
    lda(pull8());
    lda((uint8_t)(cpu.a & 0x0F));
    WR8(ZP->temp03, cpu.a);
    lda(1);
    WR8(ZP->temp01, cpu.a);
    WR8(ZP->temp02, 0);
    for (;;) {
        lda(RD8(ZP->temp03));
        if (cpu.a == 0) {
            break;
        }
        value = RD8(ZP->temp01);
        abi_flag(P_C, (value & 0x80) != 0);
        WR8(ZP->temp01, (uint8_t)(value << 1));
        value = RD8(ZP->temp02);
        WR8(ZP->temp02, (uint8_t)((value << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
        abi_flag(P_C, (value & 0x80) != 0);
        abi_nz(DEC8(ZP->temp03));
    }
    lda(RD8(ZP->temp01));
    lda((uint8_t)(cpu.a ^ 0xFF));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(ZP->temp02));
    lda((uint8_t)(cpu.a ^ 0xFF));
    WR8(ZP->temp02, cpu.a);
}

void abi_DissolveMask(void)
{
    DissolveMask();
}

/*
 * @routine 0E0A:260 DissolveWords0E0A260
 * @run 70:AA60
 * WHAT IT DOES: Applies a dissolve step's mask (Secret stage 08): of the 64 VRAM words from the address in zBiosCl-Ch, every
 * fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
 * HOW: The address stepped through zBiosCl-Ch.
 * WAITS: no.
 * Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
 */
void DissolveWords0E0A260(void)
{
    DissolveWordsAt(0xAA60);
}

void abi_DissolveWords0E0A260(void)
{
    DissolveWords0E0A260();
}

/*
 * @routine 0E0B:7ED DissolveStep0E0B7ED
 * @run 70:B7ED
 * WHAT IT DOES: One step of a dissolve (Secret stage 08): on the frames its clock allows, the next mask of the order is
 * applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
 * HOW: DissolveMask, then DissolveWords0E0A260 for each pattern address from $B821.
 * WAITS: no.
 * Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
 * bits; else A the new phase, X kept, Y twice the last pattern.
 */
void DissolveStep0E0B7ED(uint8_t slot)
{
    Dissolve dissolve = { COL(OBJ->dissolve_clock_f), COL(OBJ->dissolve_phase_k), 0xB821, 1, 0x13 };

    dissolve_step(slot, &dissolve, DissolveWords0E0A260, 0xB7ED);
}

void abi_DissolveStep0E0B7ED(void)
{
    DissolveStep0E0B7ED(cpu.x);
}

/*
 * @routine 10CA:29D DissolveWords10CA29D
 * @run 70:AA9D
 * WHAT IT DOES: Applies a dissolve step's mask (Secret stage 0A): of the 64 VRAM words from the address in zBiosCl-Ch, every
 * fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
 * HOW: The address stepped through zBiosCl-Ch.
 * WAITS: no.
 * Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
 */
void DissolveWords10CA29D(void)
{
    DissolveWordsAt(0xAA9D);
}

void abi_DissolveWords10CA29D(void)
{
    DissolveWords10CA29D();
}

/*
 * @routine 10CA:272 DissolveMask10CA272
 * @run 70:AA72
 * WHAT IT DOES: DissolveMask where Secret stage 0A's overlay holds it: the same bytes at another run address, with no
 * absolute operand inside itself, so DissolveMask's C serves it.
 * HOW: DissolveMask.
 * WAITS: no.
 * Entry and exit: as DissolveMask.
 */
void DissolveMask10CA272(void)
{
    DissolveMask();
}

void abi_DissolveMask10CA272(void)
{
    DissolveMask10CA272();
}

/*
 * @routine 10CC:5E1 DissolveStep10CC5E1
 * @run 70:BDE1
 * WHAT IT DOES: One step of a dissolve (Secret stage 0A): on the frames its clock allows, the next mask of the order is
 * applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
 * HOW: DissolveMask, then DissolveWords10CA29D for each pattern address from $BE15.
 * WAITS: no.
 * Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
 * bits; else A the new phase, X kept, Y twice the last pattern.
 */
void DissolveStep10CC5E1(uint8_t slot)
{
    Dissolve dissolve = { COL(OBJ->dissolve_clock_d), COL(OBJ->dissolve_phase_p), 0xBE15, 1, 0x13 };

    dissolve_step(slot, &dissolve, DissolveWords10CA29D, 0xBDE1);
}

void abi_DissolveStep10CC5E1(void)
{
    DissolveStep10CC5E1(cpu.x);
}

/*
 * @routine 12DA:365 DissolveWords12DA365
 * @run 70:AB65
 * WHAT IT DOES: Applies a dissolve step's mask (Stage 0C): of the 64 VRAM words from the address in zBiosCl-Ch, every
 * fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
 * HOW: The address stepped through zBiosCl-Ch.
 * WAITS: no.
 * Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
 */
void DissolveWords12DA365(void)
{
    DissolveWordsAt(0xAB65);
}

void abi_DissolveWords12DA365(void)
{
    DissolveWords12DA365();
}

/*
 * @routine 12DA:33A DissolveMask12DA33A
 * @run 70:AB3A
 * WHAT IT DOES: DissolveMask where Stage 0C's overlay holds it: the same bytes at another run address, with no
 * absolute operand inside itself, so DissolveMask's C serves it.
 * HOW: DissolveMask.
 * WAITS: no.
 * Entry and exit: as DissolveMask.
 */
void DissolveMask12DA33A(void)
{
    DissolveMask();
}

void abi_DissolveMask12DA33A(void)
{
    DissolveMask12DA33A();
}

/*
 * @routine 12DE:65E DissolveStep12DE65E
 * @run 71:CE5E
 * WHAT IT DOES: One step of a dissolve (Stage 0C): on the frames its clock allows, the next mask of the order is
 * applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
 * HOW: DissolveMask, then DissolveWords12DA365 for each pattern address from $CE92.
 * WAITS: no.
 * Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
 * bits; else A the new phase, X kept, Y twice the last pattern.
 */
void DissolveStep12DE65E(uint8_t slot)
{
    Dissolve dissolve = { COL(OBJ->dissolve_clock_d), COL(OBJ->dissolve_phase_l), 0xCE92, 1, 0x11 };

    dissolve_step(slot, &dissolve, DissolveWords12DA365, 0xCE5E);
}

void abi_DissolveStep12DE65E(void)
{
    DissolveStep12DE65E(cpu.x);
}

/*
 * @routine 13FA:365 DissolveWords13FA365
 * @run 70:AB65
 * WHAT IT DOES: Applies a dissolve step's mask (Secret stage 0C): of the 64 VRAM words from the address in zBiosCl-Ch, every
 * fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
 * HOW: The address stepped through zBiosCl-Ch.
 * WAITS: no.
 * Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
 */
void DissolveWords13FA365(void)
{
    DissolveWordsAt(0xAB65);
}

void abi_DissolveWords13FA365(void)
{
    DissolveWords13FA365();
}

/*
 * @routine 13FA:33A DissolveMask13FA33A
 * @run 70:AB3A
 * WHAT IT DOES: DissolveMask where Secret stage 0C's overlay holds it: the same bytes at another run address, with no
 * absolute operand inside itself, so DissolveMask's C serves it.
 * HOW: DissolveMask.
 * WAITS: no.
 * Entry and exit: as DissolveMask.
 */
void DissolveMask13FA33A(void)
{
    DissolveMask();
}

void abi_DissolveMask13FA33A(void)
{
    DissolveMask13FA33A();
}

/*
 * @routine 13FC:0EA DissolveStep13FC0EA
 * @run 70:B8EA
 * WHAT IT DOES: One step of a dissolve (Secret stage 0C): on the frames its clock allows, the next mask of the order is
 * applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
 * HOW: DissolveMask, then DissolveWords13FA365 for each pattern address from $B91E.
 * WAITS: no.
 * Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
 * bits; else A the new phase, X kept, Y twice the last pattern.
 */
void DissolveStep13FC0EA(uint8_t slot)
{
    Dissolve dissolve = { COL(OBJ->dissolve_clock_d), COL(OBJ->dissolve_phase_n), 0xB91E, 3, 0x13 };

    dissolve_step(slot, &dissolve, DissolveWords13FA365, 0xB8EA);
}

void abi_DissolveStep13FC0EA(void)
{
    DissolveStep13FC0EA(cpu.x);
}

/*
 * @routine 16CA:260 DissolveWords16CA260
 * @run 70:AA60
 * WHAT IT DOES: Applies a dissolve step's mask (Secret stage 0E): of the 64 VRAM words from the address in zBiosCl-Ch, every
 * fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
 * HOW: The address stepped through zBiosCl-Ch.
 * WAITS: no.
 * Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
 */
void DissolveWords16CA260(void)
{
    DissolveWordsAt(0xAA60);
}

void abi_DissolveWords16CA260(void)
{
    DissolveWords16CA260();
}

/*
 * @routine 16CB:6A4 DissolveStep16CB6A4
 * @run 70:B6A4
 * WHAT IT DOES: One step of a dissolve (Secret stage 0E): on the frames its clock allows, the next mask of the order is
 * applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
 * HOW: DissolveMask, then DissolveWords16CA260 for each pattern address from $B6D8.
 * WAITS: no.
 * Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
 * bits; else A the new phase, X kept, Y twice the last pattern.
 */
void DissolveStep16CB6A4(uint8_t slot)
{
    Dissolve dissolve = { COL(OBJ->dissolve_clock_j), COL(OBJ->dissolve_phase_n), 0xB6D8, 1, 0x13 };

    dissolve_step(slot, &dissolve, DissolveWords16CA260, 0xB6A4);
}

void abi_DissolveStep16CB6A4(void)
{
    DissolveStep16CB6A4(cpu.x);
}

/*
 * @routine 072C:35D JumpByPhaseKeepingPoint072C35D
 * @run 70:BB5D
 * WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 03).
 * HOW: One store, then JumpByPhaseTableBody with the table at $BB74.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
 */
Jump JumpByPhaseKeepingPoint072C35D(uint8_t slot)
{
    return keep_point_jump(slot, 0xBB74);
}

void abi_JumpByPhaseKeepingPoint072C35D(void)
{
    Jump jump = JumpByPhaseKeepingPoint072C35D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088B:482 JumpByPhaseKeepingPoint088B482
 * @run 70:B482
 * WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 04).
 * HOW: One store, then JumpByPhaseTableBody with the table at $B499.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
 */
Jump JumpByPhaseKeepingPoint088B482(uint8_t slot)
{
    return keep_point_jump(slot, 0xB499);
}

void abi_JumpByPhaseKeepingPoint088B482(void)
{
    Jump jump = JumpByPhaseKeepingPoint088B482(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4C:4DB JumpByPhaseKeepingPoint0B4C4DB
 * @run 70:BCDB
 * WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 06).
 * HOW: One store, then JumpByPhaseTableBody with the table at $BCF2.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
 */
Jump JumpByPhaseKeepingPoint0B4C4DB(uint8_t slot)
{
    return keep_point_jump(slot, 0xBCF2);
}

void abi_JumpByPhaseKeepingPoint0B4C4DB(void)
{
    Jump jump = JumpByPhaseKeepingPoint0B4C4DB(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAC:032 JumpByPhaseKeepingPoint0CAC032
 * @run 70:B832
 * WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 07).
 * HOW: One store, then JumpByPhaseTableBody with the table at $B849.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
 */
Jump JumpByPhaseKeepingPoint0CAC032(uint8_t slot)
{
    return keep_point_jump(slot, 0xB849);
}

void abi_JumpByPhaseKeepingPoint0CAC032(void)
{
    Jump jump = JumpByPhaseKeepingPoint0CAC032(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 16CB:61D JumpByPhaseKeepingPoint16CB61D
 * @run 70:B61D
 * WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 0E).
 * HOW: One store, then JumpByPhaseTableBody with the table at $B634.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
 */
Jump JumpByPhaseKeepingPoint16CB61D(uint8_t slot)
{
    return keep_point_jump(slot, 0xB634);
}

void abi_JumpByPhaseKeepingPoint16CB61D(void)
{
    Jump jump = JumpByPhaseKeepingPoint16CB61D(cpu.x);

    jump_shim(jump.end, jump.target);
}
