/*
 * TEMPORARY: the stage object routines of lot L6 for the overlays at sectors 16C9 to 16CF (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/*
 * @routine 16CB:3EE StepListB404ThenTimer
 * @run 70:B3EE
 * WHAT IT DOES: Steps the animation list at $B404; when it ends, OBJ_34B8 becomes $48 and the object goes to the next OBJ_2800.
 * HOW: StepObjectFrames, a store, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z (A $48) when the list ended.
 */
Ended StepListB404ThenTimer(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry, phase;
    Ended e;

    WR8(ZP->temp10, 0x04);
    WR8(ZP->temp11, 0xB4);
    jsr_push(0xB3F9);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->f34b8[slot], 0x48);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(0x48)), ended_nz(phase));
}

void abi_StepListB404ThenTimer(void)
{
    abi_ended(StepListB404ThenTimer(cpu.x, cpu.y));
}

/*
 * @routine 16CB:20E LoadThenStartRise
 * @run 70:B20E
 * WHAT IT DOES: Loads the object's record, sets its flip byte (OBJ_2FE8) to $80, its Y speed to $FE80 and OBJ_3720 to $E0.
 * HOW: LoadObjectRecord, stores.
 * WAITS: no.
 * Entry: X object. Exit: A $E0, N set, Z clear; C and Y from the record.
 */
Ended LoadThenStartRise(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;
    Ended e;

    jsr_push(0xB211);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->sprite_flip[slot], 0x80);
    WR8(OBJ->vy_hi[slot], 0xFE);
    WR8(OBJ->vy_lo[slot], 0x80);
    WR8(OBJ->f3720[slot], 0xE0);
    return then(e, ended_load(0xE0));
}

void abi_LoadThenStartRise(void)
{
    abi_ended(LoadThenStartRise(cpu.x));
}

/*
 * @routine 16CB:24C ClearSlotsWhenScrollZero
 * @run 70:B24C
 * WHAT IT DOES: When the level's X position high byte and the scroll X low are both zero, clears the objects in slots $11 to $4E, sets OBJ_34B8 to $60 and goes on to the next OBJ_2800.
 * HOW: Two reads, a loop of ClearObjectInY, a store, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the OR, or the new OBJ_2800's N and Z.
 */
Ended ClearSlotsWhenScrollZero(uint8_t slot)
{
    uint8_t high, low, y, phase;
    Ended e;

    high = rd8(ScrollXHiScreen);
    low = rd8(ScrollXLo);
    e = ended_load((uint8_t)(high | low));
    if ((high | low) != 0) {
        return e;
    }
    for (y = 0x11; y < 0x4F; y++) {
        jsr_push(0xB259);
        ClearObjectInY(y);
        jsr_pop();
    }
    WR8(OBJ->f34b8[slot], 0x60);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_with_y(ended_with_carry(ended_nz(0), true), 0x4F)), then(ended_load(0x60), ended_nz(phase)));
}

void abi_ClearSlotsWhenScrollZero(void)
{
    abi_ended(ClearSlotsWhenScrollZero(cpu.x));
}

/*
 * @routine 16CB:2F6 StepListB36AThenCount
 * @run 70:B2F6
 * WHAT IT DOES: Steps the animation list at $B36A and moves the object along Y; counts OBJ_34B8 down and at zero clears OBJ_2FE8, sets OBJ_3510 to $40 and goes on to the next OBJ_2800.
 * HOW: StepObjectFrames, ApplyVelocityY, DEC, stores, INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the count, or the new OBJ_2800's N and Z (A $40).
 */
Ended StepListB36AThenCount(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    AxisMove move;
    uint8_t y = y_entry, left, phase;
    Ended e;

    WR8(ZP->temp10, 0x6A);
    WR8(ZP->temp11, 0xB3);
    jsr_push(0xB301);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    jsr_push(0xB304);
    move = ApplyVelocityY(slot);
    jsr_pop();
    left = DEC8(OBJ->f34b8[slot]);
    e = then(then(e, ended_of_axis_move(move)), ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(OBJ->sprite_flip[slot], 0);
    WR8(OBJ->f3510[slot], 0x40);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(0x40)), ended_nz(phase));
}

void abi_StepListB36AThenCount(void)
{
    abi_ended(StepListB36AThenCount(cpu.x, cpu.y));
}

/*
 * @routine 16CB:54F StartKindFromTableAtSlot16
 * @run 70:B54F
 * WHAT IT DOES: Takes a kind from the table at $B56B by OBJ_3618, clears the object in slot $16, starts an object of that kind there at this object's place, counts OBJ_3618 up, sets OBJ_34B8 to $40 and goes on to the next OBJ_2800.
 * HOW: A table read (bank 70), ClearObjectInY, StartObjectAtPlace, two INCs.
 * WAITS: no.
 * Entry: X object. Exit: A $40, Y $16, N and Z from the new OBJ_2800.
 */
Ended StartKindFromTableAtSlot16(uint8_t slot)
{
    uint8_t index, kind, phase;
    Ended e;

    index = RD8(OBJ->f3618[slot]);
    kind = rd8((uint16_t)(0xB56B + index));
    e = ended_with_y(ended_load(kind), index);
    push8(kind);
    jsr_push(0xB55B);
    ClearObjectInY(0x16);
    jsr_pop();
    kind = pull8();
    e = then(e, ended_with_y(ended_load(kind), 0x16));
    jsr_push(0xB55F);
    StartObjectAtPlace(kind, slot, 0x16);
    jsr_pop();
    INC8(OBJ->f3618[slot]);
    WR8(OBJ->f34b8[slot], 0x40);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(0x40)), ended_nz(phase));
}

void abi_StartKindFromTableAtSlot16(void)
{
    abi_ended(StartKindFromTableAtSlot16(cpu.x));
}

/*
 * @routine 16CB:3C8 CountThenJumpRight
 * @run 70:B3C8
 * WHAT IT DOES: Sets the object's frame to 1 and counts OBJ_34B8 down; at zero it gives the object an X speed high of $12 and a Y speed high of $E0, moves it once, clears its animation step and timer, sets frame 3 and goes on to the next OBJ_2800.
 * HOW: DEC, stores, ApplyVelocity, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new OBJ_2800's N and Z (A 3).
 */
Ended CountThenJumpRight(uint8_t slot)
{
    AxisMove move;
    uint8_t left, phase;
    Ended e;

    WR8(OBJ->frame[slot], 1);
    left = DEC8(OBJ->f34b8[slot]);
    e = then(ended_load(1), ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(OBJ->vx_hi[slot], 0x12);
    WR8(OBJ->vy_hi[slot], 0xE0);
    e = then(e, ended_load(0xE0));
    jsr_push(0xB3DF);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->frame[slot], 3);
    phase = INC8(OBJ->f2800[slot]);
    return then(then(e, ended_load(3)), ended_nz(phase));
}

void abi_CountThenJumpRight(void)
{
    abi_ended(CountThenJumpRight(cpu.x));
}
