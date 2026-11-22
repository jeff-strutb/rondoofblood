/*
 * The file menu and the name menu, which share one screen of objects: the page and step the
 * menu is in (zGameSubMode and zField80, the page a table of steps is chosen by), the cursor
 * objects that glide to the place the cursor value names, and the shift of the menu's objects when
 * the screen scrolls.
 *
 * Fields named by their address, and tables read by their address, are placeholders until the
 * names for them land (front_end.h lists them): the cursor value of object 0 and 1 is OBJ->f34b8,
 * the target place of an object is OBJ->f3568 (X) and OBJ->f35c0 (Y), and OBJ->f3510 says it is
 * on its way.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"

/*
 * @routine 00C4:60D SetMenuStep
 * @run 6B:9E0D
 * WHAT IT DOES: Puts the menu in step A of its page.
 * HOW: One store into zField80.
 * WAITS: no.
 * Entry: A the step. Exit: A and the flags kept.
 */
void SetMenuStep(uint8_t step)
{
    WR8(ZP->field80, step);
}

void abi_SetMenuStep(void)
{
    SetMenuStep(cpu.a);
}

/*
 * @routine 00C4:60A MenuStepAdvance
 * @run 6B:9E0A
 * WHAT IT DOES: Moves the menu on to the next step of its page.
 * HOW: Reads zField80, adds one and runs on into SetMenuStep.
 * WAITS: no.
 * Entry: nothing. Exit: A the new step, N and Z from it, C kept.
 */
uint8_t MenuStepAdvance(void)
{
    uint8_t step;

    step = RD8(ZP->field80);
    step = (uint8_t)(step + 1);
    SetMenuStep(step);
    return step;
}

void abi_MenuStepAdvance(void)
{
    uint8_t step = MenuStepAdvance();

    cpu.a = step;
    abi_nz(step);
}

/*
 * @routine 00C4:605 SetMenuPage
 * @run 6B:9E05
 * WHAT IT DOES: Puts the menu on page A, at its first step.
 * HOW: Stores A in zGameSubMode, then runs on into SetMenuStep with the step 0.
 * WAITS: no.
 * Entry: A the page. Exit: A 0, the flags kept.
 */
void SetMenuPage(uint8_t page)
{
    WR8(ZP->game_sub_mode, page);
    SetMenuStep(0);
}

void abi_SetMenuPage(void)
{
    SetMenuPage(cpu.a);
    cpu.a = 0;
}

/*
 * @routine 00C4:602 MenuPageAdvance
 * @run 6B:9E02
 * WHAT IT DOES: Moves the menu on to the first step of the next page.
 * HOW: Reads zGameSubMode, adds one and runs on into SetMenuPage.
 * WAITS: no.
 * Entry: nothing. Exit: A 0, N and Z from the new page, C kept.
 */
uint8_t MenuPageAdvance(void)
{
    uint8_t page;

    page = RD8(ZP->game_sub_mode);
    page = (uint8_t)(page + 1);
    SetMenuPage(page);
    return page;
}

void abi_MenuPageAdvance(void)
{
    uint8_t page = MenuPageAdvance();

    cpu.a = 0;
    abi_nz(page);
}

/*
 * @routine 00C4:5E9 ObjectFrameFromTick
 * @run 6B:9DE9
 * WHAT IT DOES: Gives an object the picture frame (1 to 8) that the frame counter says: a
 * frame lasts four ticks, and the eight frames go round.
 * HOW: Reads zFrameTick, shifts it right twice, keeps three bits, adds one, stores it in the
 * object's frame byte.
 * WAITS: no.
 * Entry: X the object. Exit: A the frame, N and Z clear, C bit 1 of the tick. X and Y kept.
 */
ValueCarry ObjectFrameFromTick(uint8_t object)
{
    ValueCarry result;
    uint8_t tick;

    tick = RD8(ZP->frame_tick);
    result.carry = (tick & 0x02) != 0;
    result.value = (uint8_t)(((tick >> 2) & 0x07) + 1);
    WR8(OBJ->f2a68[object], result.value);
    return result;
}

void abi_ObjectFrameFromTick(void)
{
    ValueCarry result = ObjectFrameFromTick(cpu.x);

    cpu.a = result.value;
    abi_nz(result.value);
    abi_flag(P_C, result.carry);
}

/*
 * @routine 00C4:5D4 HalveObjectVelocity
 * @run 6B:9DD4
 * WHAT IT DOES: Halves an object's speed along both axes, keeping the sign.
 * HOW: The X speed (high byte, then low byte) and then the Y speed, each as a 16-bit
 * arithmetic shift right: the sign is copied into the carry first so the high byte keeps it.
 * WAITS: no.
 * Entry: X the object. Exit: A the Y speed's high byte shifted left once, N and Z from the Y
 * speed's new low byte, C the bit it lost. X and Y kept.
 */
Alu HalveObjectVelocity(uint8_t object)
{
    Alu result = alu_none();
    uint8_t high, old, shifted;
    bool carry;

    high = RD8(OBJ->vx_hi[object]);
    carry = (high & 0x80) != 0;
    carry = ROR8(OBJ->vx_hi[object], carry);
    ROR8(OBJ->vx_lo[object], carry);
    high = RD8(OBJ->vy_hi[object]);
    result.a = (uint8_t)(high << 1);
    carry = (high & 0x80) != 0;
    carry = ROR8(OBJ->vy_hi[object], carry);
    old = RD8(OBJ->vy_lo[object]);
    shifted = (uint8_t)((old >> 1) | (carry ? 0x80 : 0));
    WR8(OBJ->vy_lo[object], shifted);
    result.nz = shifted;
    result.c = (old & 0x01) != 0;
    result.c_kept = false;
    return result;
}

void abi_HalveObjectVelocity(void)
{
    abi_alu(HalveObjectVelocity(cpu.x));
}

/*
 * @routine 00C4:5BA SetCursorTarget
 * @run 6B:9DBA
 * WHAT IT DOES: Sends a cursor object to the place its cursor value names: the target is
 * read from a table of places, one pair for each value and each of the two cursors.
 * HOW: The object's cursor value (OBJ->f34b8) doubled with the object's low bit under it, doubled
 * again, is the table index; the pair at that index goes to the object's target (X and Y) and
 * the object is marked as on its way (OBJ->f3510 is 1).
 * WAITS: no.
 * Entry: X the cursor object (0 or 1). Exit: A 1, N and Z clear, C bit 6 of the cursor value, Y
 * the table index. X kept.
 */
CursorTarget SetCursorTarget(uint8_t object)
{
    CursorTarget result;
    uint8_t cursor, doubled, target;

    cursor = RD8(OBJ->f34b8[object]);
    doubled = (uint8_t)((cursor << 1) | (object & 1));
    result.carry = (doubled & 0x80) != 0;
    result.index = (uint8_t)(doubled << 1);
    target = rd8((uint16_t)(CURSOR_PLACES + result.index));
    WR8(OBJ->f3568[object], target);
    target = rd8((uint16_t)(CURSOR_PLACES + 1 + result.index));
    WR8(OBJ->f35c0[object], target);
    WR8(OBJ->f3510[object], 1);
    return result;
}

void abi_SetCursorTarget(void)
{
    CursorTarget result = SetCursorTarget(cpu.x);

    cpu.a = 1;
    cpu.y = result.index;
    abi_nz(1);
    abi_flag(P_C, result.carry);
}

/*
 * @routine 00C4:5B6 SetCursorTargets
 * @run 6B:9DB6
 * WHAT IT DOES: SetCursorTarget for both cursor objects, 0 then 1.
 * HOW: A call for object 0; the one byte after it, INX, runs on into SetCursorTarget for 1.
 * WAITS: no.
 * Entry: nothing. Exit: as SetCursorTarget for object 1, X 1.
 */
CursorTarget SetCursorTargets(void)
{
    jsr_push(0x9DB9);
    SetCursorTarget(0);
    jsr_pop();
    return SetCursorTarget(1);
}

void abi_SetCursorTargets(void)
{
    CursorTarget result = SetCursorTargets();

    cpu.x = 1;
    cpu.a = 1;
    cpu.y = result.index;
    abi_nz(1);
    abi_flag(P_C, result.carry);
}

/*
 * @routine 00C4:46A SetCursorValue
 * @run 6B:9C6A
 * WHAT IT DOES: Puts both cursors on the cursor value A and sends them to its place.
 * HOW: Stores A in both cursor objects' value and goes to SetCursorTargets.
 * WAITS: no.
 * Entry: A the cursor value. Exit: as SetCursorTargets.
 */
CursorTarget SetCursorValue(uint8_t value)
{
    WR8(OBJ->f34b8[0], value);
    WR8(OBJ->f34b8[1], value);
    return SetCursorTargets();
}

void abi_SetCursorValue(void)
{
    CursorTarget result = SetCursorValue(cpu.a);

    cpu.x = 1;
    cpu.a = 1;
    cpu.y = result.index;
    abi_nz(1);
    abi_flag(P_C, result.carry);
}

/*
 * @routine 00C4:4D0 PlaceCursorObject
 * @run 6B:9CD0
 * WHAT IT DOES: Makes a cursor object afresh at the place its cursor value names.
 * HOW: Keeps A on the stack across ClearObject, gives the object the value, frame 4 and the
 * place from the same table SetCursorTarget reads (index as there).
 * WAITS: no.
 * Entry: A the cursor value, X the object (0 or 1). Exit: A the Y place, N and Z from it, C
 * bit 6 of the cursor value, Y the table index. X kept.
 */
CursorTarget PlaceCursorObject(uint8_t value, uint8_t object)
{
    CursorTarget result;
    uint8_t pulled, doubled;

    push8(value);
    jsr_push(0x9CD4);
    ClearObject(object);
    jsr_pop();
    WR8(OBJ->f2a10[object], 4);
    pulled = pull8();
    WR8(OBJ->f34b8[object], pulled);
    doubled = (uint8_t)((pulled << 1) | (object & 1));
    result.carry = (doubled & 0x80) != 0;
    result.index = (uint8_t)(doubled << 1);
    result.value = rd8((uint16_t)(CURSOR_PLACES + result.index));
    WR8(OBJ->x_lo[object], result.value);
    result.value = rd8((uint16_t)(CURSOR_PLACES + 1 + result.index));
    WR8(OBJ->y_lo[object], result.value);
    return result;
}

void abi_PlaceCursorObject(void)
{
    CursorTarget result = PlaceCursorObject(cpu.a, cpu.x);

    cpu.a = result.value;
    cpu.y = result.index;
    abi_nz(result.value);
    abi_flag(P_C, result.carry);
}

/*
 * @routine 00C4:4CA PlaceCursorObjects
 * @run 6B:9CCA
 * WHAT IT DOES: PlaceCursorObject for both cursor objects, 0 then 1, with the same value.
 * HOW: Keeps A on the stack; a call for object 0; the PLA and INX after it run on into
 * PlaceCursorObject for object 1.
 * WAITS: no.
 * Entry: A the cursor value. Exit: as PlaceCursorObject for object 1, X 1.
 */
CursorTarget PlaceCursorObjects(uint8_t value)
{
    uint8_t pulled;

    push8(value);
    jsr_push(0x9CCE);
    PlaceCursorObject(value, 0);
    jsr_pop();
    pulled = pull8();
    return PlaceCursorObject(pulled, 1);
}

void abi_PlaceCursorObjects(void)
{
    CursorTarget result = PlaceCursorObjects(cpu.a);

    cpu.x = 1;
    cpu.a = result.value;
    cpu.y = result.index;
    abi_nz(result.value);
    abi_flag(P_C, result.carry);
}

/*
 * @routine 00C4:3F6 ShiftObjectsByScrollChange
 * @run 6B:9BF6
 * WHAT IT DOES: Moves the menu's objects 16 to 63 sideways by what the menu's scroll has moved
 * since the last time, so they stay where they are on the screen.
 * HOW: The X scroll word kept last time (the menu keeps it in the first X and Y speed bytes,
 * which it has no other use for) less the scroll's X word now is kept in their place as the shift, and added, low byte then high byte, to the X
 * place of each of the objects.
 * WAITS: no.
 * Entry: nothing. Exit: A the last high byte sum, V from that sum, X $40, Z and C set, N clear.
 */
Alu ShiftObjectsByScrollChange(void)
{
    Alu low, high, step;
    uint8_t object, value, shift;

    value = RD8(SCROLLE->speed_x[0]);
    shift = RD8(SCROLL->x_lo[0]);
    low = sbc8(value, shift, true);
    WR8(SCROLLE->speed_x[0], low.a);
    value = RD8(SCROLLE->speed_y[0]);
    shift = RD8(SCROLL->x_hi[0]);
    high = sbc8(value, shift, low.c);
    WR8(SCROLLE->speed_y[0], high.a);
    step = alu_none();
    for (object = 0x10; object != 0x40; object++) {
        value = RD8(OBJ->x_lo[object]);
        shift = RD8(SCROLLE->speed_x[0]);
        low = adc8(value, shift, false);
        WR8(OBJ->x_lo[object], low.a);
        value = RD8(OBJ->x_hi[object]);
        shift = RD8(SCROLLE->speed_y[0]);
        step = adc8(value, shift, low.c);
        WR8(OBJ->x_hi[object], step.a);
    }
    step.nz = 0;
    step.c = true;
    return step;
}

void abi_ShiftObjectsByScrollChange(void)
{
    abi_alu(ShiftObjectsByScrollChange());
    cpu.x = 0x40;
}
