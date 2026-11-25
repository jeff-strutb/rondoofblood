/* The stage entry: the section's record loaded into the scroll state, the camera limits and the player's start. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    SCROLL_COPIES = 8, SECTION_SHIFT = 1, KIND_FIRST = 5, KIND_SPAN = 4, SKIP_BIT = 0x02, CHARACTER_OFFSETS = 0x8A32,
    X_LO_SLOT = 0, FIELD_RIGHT = 0x80
};

/*
 * Reads the section's record, a pointer the overlay's table gives by zField83, and puts it in
 * place: the first byte the X screen of every scroll slot, the next the Y screen and Y high,
 * then the scroll positions and the camera limits, and last the player's start.
 */
static StageExit load_stage_entry(uint16_t table)
{
    StageExit end = { 0, 0, 0, false, false, 0, EXIT_Y };
    uint8_t y;
    uint8_t a;
    uint8_t value;
    uint8_t slot;
    uint8_t kind;
    Sum sum;

    WR8(ZP->raster_stage, 0);
    a = RD8(ZP->field83);
    end.c = (a & 0x80) != 0;
    end.set |= EXIT_C;
    y = (uint8_t)(a << SECTION_SHIFT);
    value = rd8((uint16_t)(table + y));
    WR8(ZP->temp00, value);
    value = rd8((uint16_t)(table + 1 + y));
    WR8(ZP->temp01, value);
    y = 0;
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    for (slot = 0; slot < SCROLL_COPIES; slot++) {
        WR8(SCROLLE->x_hi_screen[slot], a);
    }
    value = (uint8_t)(a & 0x01);
    for (slot = 0; slot < SCROLL_COPIES; slot++) {
        WR8(SCROLL->x_hi[slot], value);
    }
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(SCROLLE->y_hi_screen[0], a);
    value = (uint8_t)(a & 0x01);
    WR8(SCROLL->y_hi[0], value);
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(SCROLL->x_lo[0], a);
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(SCROLL->y_lo[0], a);
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(SCROLLLIM->camera_mode, a);
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(SCROLLE->f3e66[0], a);
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(SCROLLLIM->x_min_hi, a);
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(SCROLLLIM->x_max.hi, a);
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(SCROLLLIM->y_min_hi, a);
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(SCROLLLIM->y_max.hi, a);
    if ((RD8(ZP->field8e) & SKIP_BIT) != 0) {
        /* Untraced: the record's next two bytes are skipped and the start is not changed. */
        RMW8(ZP->field8e, flags, flags & ~SKIP_BIT);
        y = (uint8_t)(y + 2);
    } else {
        a = RD8_PTR_Y(ZP->temp00, y);
        if (a != 0) {
            y++;
            WR8(OBJ->x_lo[X_LO_SLOT], a);
        } else {
            /* Untraced: a zero byte takes the saved scroll in its place. */
            a = RD8(ZP->bios_a.lo);
            WR8(SCROLLE->x_hi_screen[0], a);
            value = (uint8_t)(a & 0x01);
            WR8(SCROLL->x_hi[0], value);
            a = RD8(ZP->bios_a.hi);
            WR8(SCROLL->x_lo[0], a);
            y++;
        }
        a = RD8_PTR_Y(ZP->temp00, y);
        y++;
        WR8(OBJ->y_lo[X_LO_SLOT], a);
        WR8(OBJ->y_hi[X_LO_SLOT], 0);
        WR8(OBJ->x_hi[X_LO_SLOT], 0);
        a = RD8(ZP->character);
        if (a != 0) {
            /* Untraced: the second character's start is moved by the kind of the first object. */
            kind = RD8(OBJ->f27a8[0]);
            end.c = kind >= KIND_FIRST;
            end.set |= EXIT_C;
            if (kind >= KIND_FIRST) {
                sum = sub_bytes(kind, KIND_FIRST, true);
                end.c = sum.value >= KIND_SPAN;
                if (sum.value < KIND_SPAN) {
                    value = RD8(OBJ->x_lo[X_LO_SLOT]);
                    a = rd8((uint16_t)(CHARACTER_OFFSETS + sum.value));
                    sum = add_bytes(a, value, false);
                    WR8(OBJ->x_lo[X_LO_SLOT], sum.value);
                    end.c = sum.carry;
                    end.v = sum.overflow;
                    end.set |= EXIT_C | EXIT_V;
                }
            }
        }
    }
    a = RD8_PTR_Y(ZP->temp00, y);
    y++;
    WR8(ZP->raster_effect, a);
    WR8(ZP->raster_arg, 0);
    a = RD8_PTR_Y(ZP->temp00, y);
    if ((a & FIELD_RIGHT) == 0) {
        y++;
        WR8(OBJ->facing_left[X_LO_SLOT], a);
    } else {
        y++;
    }
    end.a = a;
    end.y = y;
    end.nz = y;
    return end;
}

/*
 * @routine 024E:152 LoadStageEntry
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntry(void)
{
    return load_stage_entry(0x9BC2);
}

void abi_LoadStageEntry(void)
{
    stage_exit_shim(LoadStageEntry());
}

/*
 * @routine 050E:152 LoadStageEntryB
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9C71.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryB(void)
{
    return load_stage_entry(0x9C71);
}

void abi_LoadStageEntryB(void)
{
    stage_exit_shim(LoadStageEntryB());
}

/*
 * @routine 05BE:152 LoadStageEntryC
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9EAD.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryC(void)
{
    return load_stage_entry(0x9EAD);
}

void abi_LoadStageEntryC(void)
{
    stage_exit_shim(LoadStageEntryC());
}

/*
 * @routine 071E:152 LoadStageEntryD
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9F70.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryD(void)
{
    return load_stage_entry(0x9F70);
}

void abi_LoadStageEntryD(void)
{
    stage_exit_shim(LoadStageEntryD());
}

/*
 * @routine 07CE:152 LoadStageEntryE
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9D29.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryE(void)
{
    return load_stage_entry(0x9D29);
}

void abi_LoadStageEntryE(void)
{
    stage_exit_shim(LoadStageEntryE());
}

/*
 * @routine 087E:152 LoadStageEntryF
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9E70.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryF(void)
{
    return load_stage_entry(0x9E70);
}

void abi_LoadStageEntryF(void)
{
    stage_exit_shim(LoadStageEntryF());
}

/*
 * @routine 092E:152 LoadStageEntryG
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9D6C.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryG(void)
{
    return load_stage_entry(0x9D6C);
}

void abi_LoadStageEntryG(void)
{
    stage_exit_shim(LoadStageEntryG());
}

/*
 * @routine 09DE:152 LoadStageEntryH
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9EB6.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryH(void)
{
    return load_stage_entry(0x9EB6);
}

void abi_LoadStageEntryH(void)
{
    stage_exit_shim(LoadStageEntryH());
}

/*
 * @routine 0A8E:152 LoadStageEntryI
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9D50.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryI(void)
{
    return load_stage_entry(0x9D50);
}

void abi_LoadStageEntryI(void)
{
    stage_exit_shim(LoadStageEntryI());
}

/*
 * @routine 0B3E:152 LoadStageEntryJ
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9E81.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryJ(void)
{
    return load_stage_entry(0x9E81);
}

void abi_LoadStageEntryJ(void)
{
    stage_exit_shim(LoadStageEntryJ());
}

/*
 * @routine 0BEE:152 LoadStageEntryK
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9D05.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryK(void)
{
    return load_stage_entry(0x9D05);
}

void abi_LoadStageEntryK(void)
{
    stage_exit_shim(LoadStageEntryK());
}

/*
 * @routine 0C9E:152 LoadStageEntryL
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9E84.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryL(void)
{
    return load_stage_entry(0x9E84);
}

void abi_LoadStageEntryL(void)
{
    stage_exit_shim(LoadStageEntryL());
}

/*
 * @routine 0D4E:152 LoadStageEntryM
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9D41.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryM(void)
{
    return load_stage_entry(0x9D41);
}

void abi_LoadStageEntryM(void)
{
    stage_exit_shim(LoadStageEntryM());
}

/*
 * @routine 0DFE:152 LoadStageEntryN
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9EB3.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryN(void)
{
    return load_stage_entry(0x9EB3);
}

void abi_LoadStageEntryN(void)
{
    stage_exit_shim(LoadStageEntryN());
}

/*
 * @routine 0EAE:152 LoadStageEntryO
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9E76.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryO(void)
{
    return load_stage_entry(0x9E76);
}

void abi_LoadStageEntryO(void)
{
    stage_exit_shim(LoadStageEntryO());
}

/*
 * @routine 0F5E:152 LoadStageEntryP
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9F27.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryP(void)
{
    return load_stage_entry(0x9F27);
}

void abi_LoadStageEntryP(void)
{
    stage_exit_shim(LoadStageEntryP());
}

/*
 * @routine 100E:152 LoadStageEntryQ
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9CBA.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryQ(void)
{
    return load_stage_entry(0x9CBA);
}

void abi_LoadStageEntryQ(void)
{
    stage_exit_shim(LoadStageEntryQ());
}

/*
 * @routine 10BE:152 LoadStageEntryR
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9F1A.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryR(void)
{
    return load_stage_entry(0x9F1A);
}

void abi_LoadStageEntryR(void)
{
    stage_exit_shim(LoadStageEntryR());
}

/*
 * @routine 116E:152 LoadStageEntryS
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9CD4.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryS(void)
{
    return load_stage_entry(0x9CD4);
}

void abi_LoadStageEntryS(void)
{
    stage_exit_shim(LoadStageEntryS());
}

/*
 * @routine 12CE:152 LoadStageEntryT
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9F5E.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryT(void)
{
    return load_stage_entry(0x9F5E);
}

void abi_LoadStageEntryT(void)
{
    stage_exit_shim(LoadStageEntryT());
}

/*
 * @routine 160E:152 LoadStageEntryU
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9CE6.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryU(void)
{
    return load_stage_entry(0x9CE6);
}

void abi_LoadStageEntryU(void)
{
    stage_exit_shim(LoadStageEntryU());
}

/*
 * @routine 16BE:152 LoadStageEntryV
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9F4D.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryV(void)
{
    return load_stage_entry(0x9F4D);
}

void abi_LoadStageEntryV(void)
{
    stage_exit_shim(LoadStageEntryV());
}

/*
 * @routine 18CE:152 LoadStageEntryW
 * @run 6B:8952
 * WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
 * the camera mode and limits, the player's start position and the raster effect.
 * HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
 * state, SCROLLLIM and the player's slot in the order the record keeps them.
 * The same code as LoadStageEntry; this overlay's table of section records is at $9EC1.
 * WAITS: no.
 * Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
 * Clobbers zTemp00-01.
 */
StageExit LoadStageEntryW(void)
{
    return load_stage_entry(0x9EC1);
}

void abi_LoadStageEntryW(void)
{
    stage_exit_shim(LoadStageEntryW());
}
