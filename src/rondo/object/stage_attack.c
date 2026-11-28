/*
 * Stage object pieces of the first stage's overlay (024F, 0250): a row fill of blank VRAM words, an attack's report
 * onto the object it hit, and the door the player walks into.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "syscard.h"
#include "stage_map.h"
#include "stage_regs.h"

typedef StageRegs Regs;

enum { ROW_WORDS = 0x40, DOOR_SOUND_TYPE = 0x0C };

/*
 * @routine 024F:364 StageClearVramRowsOn
 * @run 6B:9364
 * WHAT IT DOES: Writes rows of $40 zero words to the VDC from where it points, zTemp00 rows.
 * HOW: ST1 and ST2 of zero, DEC zTemp00 until it is zero (a count of 0 writes 256 rows).
 * WAITS: no.
 * Entry: zTemp00 rows, the VDC's write address set. Exit: X 0, N 0, Z 1. A, Y, C and V kept.
 */
StageRegs StageClearVramRowsOn(StageRegs r)
{
    do {
        for (uint8_t words = ROW_WORDS; words != 0; words--) {
            st1(0x00);
            st2(0x00);
        }
        r.nz = DEC8(ZP->temp00);
    } while (r.nz != 0);
    r.x = 0;
    return r;
}

void abi_StageClearVramRowsOn(void)
{
    stage_regs_load_shim(StageClearVramRowsOn(regs_entry_shim()));
}

/* HitReportHandler at $9568, run into or reached by a branch: A the power, N and Z from it; X, Y, C and V kept. */
static Regs report_keep(Regs r)
{
    return stage_regs_after(HitReportHandler(r.x, r.y), r);
}

/* From $9564: side 2 reports nothing; else the report. */
static Regs report_side(Regs r)
{
    r = regs_compared(r, r.a, 0x02);
    if (r.a == 0x02) {
        return r;
    }
    return report_keep(r);
}

/*
 * @routine 024F:55B StageHitReportLow
 * @run 6B:955B
 * @stop 9568
 * WHAT IT DOES: Records an attack's hit: the side from OBJ_PACE bit 0 into the attack's report byte; unless it is 2,
 * the hit report kept and the attack's power taken as the object's hit.
 * HOW: AND, INC; the shared end at $9564.
 * WAITS: no.
 * Entry: X the object, Y the attack. Exit: side 2: A 2, Z 1, C 1; else A the power, N and Z from it. X, Y, V kept.
 */
StageRegs StageHitReportLow(StageRegs r)
{
    r = regs_loaded(r, (uint8_t)((uint8_t)(RD8(OBJ->pace[r.x]) & 0x01) + 1));
    WR8(OBJ->attack_side[r.y], r.a);
    return report_side(r);
}

void abi_StageHitReportLow(void)
{
    stage_regs_load_shim(StageHitReportLow(regs_entry_shim()));
}

/*
 * @routine 024F:587 StageHitReportHigh
 * @run 6B:9587
 * WHAT IT DOES: As StageHitReportLow with the side from OBJ_PACE bit 1.
 * HOW: AND, LSR, INC; BRA to $9564.
 * WAITS: no.
 * Entry, exit: as StageHitReportLow.
 */
StageRegs StageHitReportHigh(StageRegs r)
{
    uint8_t bits = (uint8_t)(RD8(OBJ->pace[r.x]) & 0x02);

    r = regs_loaded(r, (uint8_t)((bits >> 1) + 1));
    r.c = false;
    WR8(OBJ->attack_side[r.y], r.a);
    return report_side(r);
}

void abi_StageHitReportHigh(void)
{
    stage_regs_load_shim(StageHitReportHigh(regs_entry_shim()));
}

/*
 * @routine 024F:593 StageHitReportTop
 * @run 6B:9593
 * WHAT IT DOES: As StageHitReportLow with the side from OBJ_PACE bit 7; side 2 gives the attack power 5 and is
 * reported all the same.
 * HOW: AND, two ROLs and AND (bit 7 to bit 0), INC; the report from $9568.
 * WAITS: no.
 * Entry: X the object, Y the attack. Exit: A the power, N and Z from it; C 0, or 1 when side 2. X, Y, V kept.
 */
StageRegs StageHitReportTop(StageRegs r)
{
    return StageHitReportTopAt(r, 0x05);
}

/* StageHitReportTop's body, for the builds of it that give side 2 another attack power. */
StageRegs StageHitReportTopAt(StageRegs r, uint8_t side_two_power)
{
    uint8_t side = (uint8_t)((RD8(OBJ->pace[r.x]) >> 7) + 1);

    r = regs_loaded(r, side);
    r.c = false;
    WR8(OBJ->attack_side[r.y], side);
    r = regs_compared(r, side, 0x02);
    if (side == 0x02) {
        r = regs_loaded(r, side_two_power);
        WR8(OBJ->attack_power[r.y], side_two_power);
    }
    return report_keep(r);
}

void abi_StageHitReportTop(void)
{
    stage_regs_load_shim(StageHitReportTop(regs_entry_shim()));
}

/*
 * @routine 0250:1EF StageDoorTouched
 * @run 6B:99EF
 * WHAT IT DOES: A door the player can go through: while he is ready for it and the door is not held, a hit of 1 or 2
 * takes him: the door's argument gives zField8C, the camera modes 1 and $18 reset, sub-step 3, the door's opening
 * started; an argument with a high nibble also stops the raster effect and, but for $F0, starts a list on channel 6;
 * one with a zero high nibble and 2 to 5 fades the CD.
 * HOW: StageDoorHeld EOR 1 AND PlayerDoorReady; CPs of OBJ_HIT_TAKEN and OBJ_START_ARG; StartListChannelKind2 with X
 * kept; the tail JMP to cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: as the path it took leaves them, X kept.
 */
StageRegs StageDoorTouched(StageRegs r)
{
    uint8_t value, arg;

    value = (uint8_t)(rd8(StageDoorHeld) ^ 0x01);
    r = regs_loaded(r, (uint8_t)(value & rd8(PlayerDoorReady)));
    if (r.a == 0) {
        return r;
    }
    r = regs_loaded(r, RD8(OBJ->hit_taken[r.x]));
    if (r.a == 0) {
        return r;
    }
    r = regs_compared(r, r.a, 0x03);
    if (r.c) {
        return r;
    }
    r = regs_loaded(r, (uint8_t)((uint8_t)(RD8(OBJ->start_arg[r.x]) & 0x0F) + 1));
    WR8(ZP->field8c, r.a);
    r = regs_loaded(r, RD8(SCROLLLIM->camera_mode));
    r = regs_compared(r, r.a, 0x01);
    if (r.a != 0x01) {
        r = regs_compared(r, r.a, 0x18);
    }
    if (r.a == 0x01 || r.a == 0x18) {
        WR8(SCROLLLIM->camera_mode, 0);
    }
    r = regs_loaded(r, 0x03);
    WR8(ZP->sub_step, 0x03);
    r.nz = INC8(OBJ->state[r.x]);
    WR8(OBJ->gate_step[r.x], 0);
    r = regs_loaded(r, 0x04);
    WR8(OBJ->gate_wait[r.x], 0x04);
    arg = (uint8_t)(RD8(OBJ->start_arg[r.x]) & 0xF0);
    r = regs_loaded(r, arg);
    if (arg != 0) {
        WR8(ZP->raster_effect, 0);
        WR8(SCROLLLIM->camera_mode, 0);
        r = regs_compared(r, arg, 0xF0);
        if (arg == 0xF0) {
            return r;
        }
        r.c = (arg & 0x08) != 0;
        r = regs_loaded(r, (uint8_t)((arg >> 4) | 0x80));
        push8(r.x);
        r.x = 6;
        r.nz = 6;
        jsr_push(0x9A43);
        r.a = StartListChannelKind2(r.a, 6);
        jsr_pop();
        r.x = pull8();
        r.nz = r.x;
        return r;
    }
    r = regs_loaded(r, RD8(OBJ->start_arg[r.x]));
    r = regs_compared(r, r.a, 0x02);
    if (!r.c) {
        return r;
    }
    r = regs_compared(r, r.a, 0x06);
    if (r.c) {
        return r;
    }
    r = regs_loaded(r, DOOR_SOUND_TYPE);
    return regs_loaded(r, vec_cd_fade(DOOR_SOUND_TYPE));
}

void abi_StageDoorTouched(void)
{
    stage_regs_load_shim(StageDoorTouched(regs_entry_shim()));
}
