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

/*
 * @routine 00DE:201 EventBitMask
 * @run 6F:AA01
 * WHAT IT DOES: Turns an event number into the byte of the event flags that holds it and the bit
 * in that byte.
 * HOW: The number is kept on the stack while its byte (over 8) is worked out, then pulled; the bit
 * is 1 shifted left by the number's low three bits. The original counts the shifts down in $FE (one more than the bits, then built one place too
 * high and rotated back so that 7 ends on $80); the counter is left at zero.
 * WAITS: no.
 * Entry: A the event number. Exit: A the bit, N and Z from it, C clear, Y the byte; X kept;
 * zBiosDl zero.
 */
ByteIndex EventBitMask(uint8_t event)
{
    ByteIndex result;
    uint8_t low, left;

    push8(event);
    result.index = (uint8_t)(event >> 3);
    event = pull8();
    low = (uint8_t)(event & 0x07);
    WR8(ZP->bios_d.lo, low);
    INC8(ZP->bios_d.lo);
    do {
        left = DEC8(ZP->bios_d.lo);
    } while (left != 0);
    result.value = (uint8_t)(1u << low);
    return result;
}

void abi_EventBitMask(void)
{
    ByteIndex result = EventBitMask(cpu.a);

    cpu.a = result.value;
    cpu.y = result.index;
    abi_nz(result.value);
    abi_flag(P_C, false);
}

/* The exit of a routine that ends on a read of the flags through the bit's byte. */
static void event_exit_shim(ByteIndex result)
{
    cpu.a = result.value;
    cpu.y = result.index;
    abi_nz(result.value);
    abi_flag(P_C, false);
}

/*
 * @routine 00DE:1E1 EventBitClear
 * @run 6F:A9E1
 * WHAT IT DOES: Takes one event out of the flags the save remembers.
 * HOW: EventBitMask gives the byte and bit; the byte is read, ANDed with the bit's complement and
 * written back.
 * WAITS: no.
 * Entry: A the event number. Exit: A the byte written, N and Z from it, C clear, Y the flag byte
 * index; X kept.
 */
ByteIndex EventBitClear(uint8_t event)
{
    ByteIndex bit, result;
    uint8_t flags;

    jsr_push(0xA9E3);
    bit = EventBitMask(event);
    jsr_pop();
    flags = rd8((uint16_t)(EVENT_FLAGS + bit.index));
    result.value = (uint8_t)(flags & ~bit.value);
    result.index = bit.index;
    wr8((uint16_t)(EVENT_FLAGS + bit.index), result.value);
    return result;
}

void abi_EventBitClear(void)
{
    event_exit_shim(EventBitClear(cpu.a));
}

/*
 * @routine 00DE:1EE EventBitSet
 * @run 6F:A9EE
 * WHAT IT DOES: Puts one event into the flags the save remembers.
 * HOW: EventBitMask gives the byte and bit; the byte is read, ORed with the bit and written back.
 * WAITS: no.
 * Entry: A the event number. Exit: as EventBitClear.
 */
ByteIndex EventBitSet(uint8_t event)
{
    ByteIndex bit, result;
    uint8_t flags;

    jsr_push(0xA9F0);
    bit = EventBitMask(event);
    jsr_pop();
    flags = rd8((uint16_t)(EVENT_FLAGS + bit.index));
    result.value = (uint8_t)(flags | bit.value);
    result.index = bit.index;
    wr8((uint16_t)(EVENT_FLAGS + bit.index), result.value);
    return result;
}

void abi_EventBitSet(void)
{
    event_exit_shim(EventBitSet(cpu.a));
}

/*
 * @routine 00DE:1F9 EventBitTest
 * @run 6F:A9F9
 * WHAT IT DOES: Tells whether one event is in the flags the save remembers.
 * HOW: EventBitMask gives the byte and bit; the byte is read and ANDed with the bit.
 * WAITS: no.
 * Entry: A the event number. Exit: A the bit if the event is set, else 0, N and Z from it, C
 * clear, Y the flag byte index; X kept.
 */
ByteIndex EventBitTest(uint8_t event)
{
    ByteIndex bit, result;
    uint8_t flags;

    jsr_push(0xA9FB);
    bit = EventBitMask(event);
    jsr_pop();
    flags = rd8((uint16_t)(EVENT_FLAGS + bit.index));
    result.value = (uint8_t)(bit.value & flags);
    result.index = bit.index;
    return result;
}

void abi_EventBitTest(void)
{
    event_exit_shim(EventBitTest(cpu.a));
}

/*
 * @routine 00DE:1D8 EventFlagsClear
 * @run 6F:A9D8
 * WHAT IT DOES: Forgets every event: the 32 bytes of event flags go to zero.
 * HOW: A loop from byte 31 down to 0.
 * WAITS: no.
 * Entry: nothing. Exit: X $FF, N set, Z clear; A, Y and C kept.
 */
void EventFlagsClear(void)
{
    uint8_t index;

    index = 0x1F;
    do {
        wr8((uint16_t)(EVENT_FLAGS + index), 0);
        index--;
    } while (index != 0xFF);
}

void abi_EventFlagsClear(void)
{
    EventFlagsClear();
    cpu.x = 0xFF;
    abi_nz(0xFF);
}

/*
 * @routine 00DE:26D CountSetBits
 * @run 6F:AA6D
 * WHAT IT DOES: Counts the bits set in a byte.
 * HOW: Shifts the byte right eight times, counting each carry out.
 * WAITS: no. The arm that counts a set bit (an INY) is in no script's trace: the draft stops
 * there, so the comparison does not reach it.
 * Entry: A the byte, X kept on the stack across it (a typed caller passes its own X). Exit: A and
 * Y the count, N and Z from X (the PLX that gives it back is the last to set them), C the byte's
 * bit 7; X kept.
 */
CountBits CountSetBits(uint8_t byte, uint8_t kept_x)
{
    CountBits result;
    uint8_t shifted, count;

    push8(kept_x);
    count = 0;
    shifted = byte;
    result.carry = false;
    for (uint8_t step = 0; step != 8; step++) {
        result.carry = (shifted & 0x01) != 0;
        shifted = (uint8_t)(shifted >> 1);
        if (result.carry) {
            count++;
        }
    }
    result.count = count;
    result.x = pull8();
    return result;
}

void abi_CountSetBits(void)
{
    CountBits result = CountSetBits(cpu.a, cpu.x);

    cpu.a = result.count;
    cpu.y = result.count;
    cpu.x = result.x;
    abi_nz(result.x);
    abi_flag(P_C, result.carry);
}

/*
 * @routine 00DE:171 WriteHexByteTiles
 * @run 6F:A971
 * WHAT IT DOES: Writes a byte to the screen as two hex digit tiles, high digit first.
 * HOW: Keeps A on the stack, writes the high nibble as a digit tile, pulls A and runs on into
 * WriteDigitTile for the low nibble.
 * WAITS: no.
 * Entry: A the byte; the VDC's write address set. Exit: A the low digit's tile number, the flags as the last OR left them (N and Z clear), C bit 3 of the byte (the last
 * shift's); X and Y kept.
 */
void WriteHexByteTiles(uint8_t byte)
{
    push8(byte);
    jsr_push(0xA978);
    WriteDigitTile((uint8_t)(byte >> 4));
    jsr_pop();
    byte = pull8();
    WriteLowDigitTile(byte);
}

void abi_WriteHexByteTiles(void)
{
    uint8_t byte = cpu.a;

    WriteHexByteTiles(byte);
    cpu.a = (uint8_t)((byte & 0x0F) | 0x10);
    abi_nz(cpu.a);
    abi_flag(P_C, (byte & 0x08) != 0);
}

/*
 * @routine 00DE:179 WriteLowDigitTile
 * @run 6F:A979
 * WHAT IT DOES: Writes the low nibble of A to the screen as a digit tile.
 * HOW: Masks A with $0F and runs on into WriteDigitTile.
 * WAITS: no.
 * Entry: A the value; the VDC's write address set. Exit: as WriteDigitTile.
 */
void WriteLowDigitTile(uint8_t value)
{
    WriteDigitTile((uint8_t)(value & 0x0F));
}

void abi_WriteLowDigitTile(void)
{
    uint8_t value = cpu.a;

    WriteLowDigitTile(value);
    cpu.a = (uint8_t)((value & 0x0F) | 0x10);
    abi_nz(cpu.a);
}

/*
 * @routine 00DE:17B WriteDigitTile
 * @run 6F:A97B
 * WHAT IT DOES: Writes one digit tile to the screen: the digit's tile number is $10 over the
 * digit, with palette 1 in the high byte.
 * HOW: ORs $10 into A, writes it to the VDC's data port and then $11 to its high byte.
 * WAITS: no.
 * Entry: A the digit (0 to $F, or any value to be ORed with $10); the VDC's write address set.
 * Exit: A the value ORed with $10, N and Z from it, C kept, X and Y kept.
 */
void WriteDigitTile(uint8_t digit)
{
    WR8(VDC->data_lo, (uint8_t)(digit | 0x10));
    st2(0x11);
}

void abi_WriteDigitTile(void)
{
    uint8_t tile = (uint8_t)(cpu.a | 0x10);

    WriteDigitTile(cpu.a);
    cpu.a = tile;
    abi_nz(tile);
}

/*
 * @routine 00DE:216 SlotCompletionPercent
 * @run 6F:AA16
 * WHAT IT DOES: Works out how complete a save slot is, as a decimal number for the file menu: two
 * for each bit set in the slot's first byte, and four for each bit set in the OR of its five
 * pairs of bytes at offsets $0B and $10.
 * HOW: Decimal mode on. The slot's offset into the backup file is read from a table; each byte's
 * set bits are counted by CountSetBits; the sum is built in zTemp08 (low) and zTemp09 (high) with
 * the count added once for the first byte twice, and four times for each of the five pairs (the
 * pair's count kept in zTemp0B between the four adds, five pairs counted in zTemp0A). Decimal
 * mode off.
 * WAITS: no.
 * Entry: Y the slot (0 to 2). Exit: A zTemp09, C from the last decimal add, N clear and Z set (the
 * pair counter reached zero), V as the caller left it, D clear; X the slot's offset plus 5; Y the
 * last count.
 */
SlotSum SlotCompletionPercent(uint8_t slot)
{
    SlotSum result;
    CountBits count;
    Alu sum;
    uint8_t offset, value, other, left, again;

    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    value = rd8((uint16_t)(SAVE_FILE + offset));
    jsr_push(0xAA20);
    count = CountSetBits(value, offset);
    jsr_pop();
    WR8(ZP->temp08, count.count);
    other = RD8(ZP->temp08);
    sum = adc8_bcd(count.count, other, false);
    WR8(ZP->temp08, sum.a);
    WR8(ZP->temp09, 0);
    WR8(ZP->temp0_a, 5);
    do {
        value = rd8((uint16_t)(SAVE_FILE + 0x0B + offset));
        other = rd8((uint16_t)(SAVE_FILE + 0x10 + offset));
        jsr_push(0xAA36);
        count = CountSetBits((uint8_t)(value | other), offset);
        jsr_pop();
        WR8(ZP->temp0_b, count.count);
        value = count.count;
        for (again = 0; again != 4; again++) {
            if (again != 0) {
                value = RD8(ZP->temp0_b);
            }
            other = RD8(ZP->temp08);
            sum = adc8_bcd(value, other, false);
            WR8(ZP->temp08, sum.a);
            other = RD8(ZP->temp09);
            sum = adc8_bcd(0, other, sum.c);
            WR8(ZP->temp09, sum.a);
        }
        offset++;
        left = DEC8(ZP->temp0_a);
    } while (left != 0);
    result.a = sum.a;
    result.carry = sum.c;
    result.x = offset;
    result.y = count.count;
    return result;
}

void abi_SlotCompletionPercent(void)
{
    SlotSum result = SlotCompletionPercent(cpu.y);

    cpu.a = result.a;
    cpu.x = result.x;
    cpu.y = result.y;
    abi_nz(0);
    abi_flag(P_C, result.carry);
    abi_flag(P_D, false);
}

/*
 * @routine 00DE:7B1 SlotClearedFlags
 * @run 6F:AFB1
 * WHAT IT DOES: For the chosen save slot, marks which of 13 things it has: a byte for each, in a
 * list that ends in a zero byte, set when the slot has any of the bits a table names for it.
 * HOW: For each of the 13, clears its byte, reads the two slot bytes the table's offset names (at
 * $01 and $06 into the slot), ORs them, ANDs the table's mask and adds one to the byte if
 * anything is left; then clears the byte after the last.
 * WAITS: no. The arm that adds one to a byte (an INC) is in no script's trace: the draft stops
 * there, so the comparison does not reach it (it is compared with every mask zero).
 * Entry: nothing. Exit: A the last AND, X 13, Y the last slot byte index, Z and C set, N clear, V
 * from the last offset add (the slot's offset plus the table's).
 */
SlotFlags SlotClearedFlags(void)
{
    SlotFlags result;
    uint8_t item, slot, offset, index, first, second, masked, flag;
    Alu add;

    masked = 0;
    index = 0;
    add = alu_none();
    for (item = 0; item != 13; item++) {
        wr8((uint16_t)(CLEARED_FLAGS + item), 0);
        slot = RD8(ZP->save_slot);
        offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
        first = rd8((uint16_t)(CLEARED_TABLE_OFFSETS + item));
        add = adc8(offset, first, false);
        index = add.a;
        first = rd8((uint16_t)(SAVE_FILE + 0x01 + index));
        second = rd8((uint16_t)(SAVE_FILE + 0x06 + index));
        masked = (uint8_t)(first | second);
        flag = rd8((uint16_t)(CLEARED_TABLE_MASKS + item));
        masked = (uint8_t)(masked & flag);
        if (masked != 0) {
            flag = rd8((uint16_t)(CLEARED_FLAGS + item));
            wr8((uint16_t)(CLEARED_FLAGS + item), (uint8_t)(flag + 1));
        }
    }
    wr8((uint16_t)(CLEARED_FLAGS + 13), 0);
    result.a = masked;
    result.y = index;
    result.v = add.v;
    return result;
}

void abi_SlotClearedFlags(void)
{
    SlotFlags result = SlotClearedFlags();

    cpu.a = result.a;
    cpu.x = 13;
    cpu.y = result.y;
    abi_nz(0);
    abi_flag(P_C, true);
    abi_flag(P_V, result.v);
}

/*
 * @routine 00DE:7A3 SlotLastClearedFlag
 * @run 6F:AFA3
 * WHAT IT DOES: Finds the highest of the 13 things SlotClearedFlags marks that the chosen slot
 * has, or 0 if it has none.
 * HOW: SlotClearedFlags, then a scan of its bytes from the 13th down for the first that is not
 * zero.
 * WAITS: no.
 * Entry: nothing. Exit: X the number found (0 to 12; 0 too when none), A the byte read last (zero when
 * none), N and Z from the read (from the last decrement of X when none), C set, V from
 * SlotClearedFlags; Y as it left it.
 */
SlotFound SlotLastClearedFlag(void)
{
    SlotFound result;
    SlotFlags flags;
    uint8_t item, value;

    jsr_push(0xAFA5);
    flags = SlotClearedFlags();
    jsr_pop();
    result.v = flags.v;
    result.y = flags.y;
    result.found = false;
    value = 0;
    item = 0x0C;
    for (;;) {
        value = rd8((uint16_t)(CLEARED_FLAGS + item));
        if (value != 0) {
            result.found = true;
            break;
        }
        item--;
        if (item >= 0x80) {
            item = 0;
            break;
        }
    }
    result.a = value;
    result.x = item;
    return result;
}

void abi_SlotLastClearedFlag(void)
{
    SlotFound result = SlotLastClearedFlag();

    cpu.a = result.a;
    cpu.x = result.x;
    cpu.y = result.y;
    abi_flag(P_C, true);
    abi_flag(P_V, result.v);
    if (result.found) {
        abi_nz(result.a);
    } else {
        abi_nz(0xFF);
    }
}

/*
 * @routine 00DD:699 SlotToWorkArea
 * @run 6F:A699
 * WHAT IT DOES: Copies the chosen save slot's bytes that the file menu shows into a work area
 * ($3F40 up): the first byte, the two at $1D and $1E, and four groups of five bytes (at $01,
 * $06, $0B and $10) as four runs of five.
 * HOW: The slot's offset into the backup file comes from a table; the three single bytes are
 * copied, then a loop of five takes one byte from each of the four groups at a time.
 * WAITS: no.
 * Entry: nothing. Exit: A the last byte copied, X the slot offset plus 5, Y 5, Z and C set, N
 * clear.
 */
SlotCopy SlotToWorkArea(void)
{
    SlotCopy result;
    uint8_t slot, offset, index, value;

    slot = RD8(ZP->save_slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    value = rd8((uint16_t)(SAVE_FILE + offset));
    wr8(SLOT_WORK, value);
    value = rd8((uint16_t)(SAVE_FILE + 0x1D + offset));
    wr8(SLOT_WORK + 1, value);
    value = rd8((uint16_t)(SAVE_FILE + 0x1E + offset));
    wr8(SLOT_WORK + 2, value);
    for (index = 0; index != 5; index++) {
        value = rd8((uint16_t)(SAVE_FILE + 0x01 + offset));
        wr8((uint16_t)(SLOT_WORK + 3 + index), value);
        value = rd8((uint16_t)(SAVE_FILE + 0x06 + offset));
        wr8((uint16_t)(SLOT_WORK + 8 + index), value);
        value = rd8((uint16_t)(SAVE_FILE + 0x0B + offset));
        wr8((uint16_t)(SLOT_WORK + 13 + index), value);
        value = rd8((uint16_t)(SAVE_FILE + 0x10 + offset));
        wr8((uint16_t)(SLOT_WORK + 18 + index), value);
        offset++;
    }
    result.a = value;
    result.x = offset;
    return result;
}

void abi_SlotToWorkArea(void)
{
    SlotCopy result = SlotToWorkArea();

    cpu.a = result.a;
    cpu.x = result.x;
    cpu.y = 5;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 00C4:025 PadRepeatUpdate
 * @run 6B:9825
 * WHAT IT DOES: Gives the menu a pad with key repeat: for each of the eight pad bits, a bit in
 * $7DFA that is set on the frame the button goes down and then every 12 frames after 24 held.
 * HOW: The new presses (the card's trigger byte) and the held bits (the card's pad byte) are
 * shifted out a bit at a time. A new press loads the bit's counter (the eight bytes from $7DF2)
 * with 24 and sets the bit. A held button counts the counter down; at zero it loads 12 and sets
 * the bit. A released button clears the counter. Each bit is rotated into $7DFA, so that button 0
 * ends on bit 0.
 * WAITS: no. The arm that reloads a counter that has run down to zero (LDA #$0C) is in no script's
 * trace: the draft stops there, so the comparison does not reach it.
 * Entry: nothing. Exit: A the last value the counter loop loaded (24, 12, 0, or the held byte if
 * no counter was loaded), X 8, Z and C set, N clear. Clobbers zTemp00 and zTemp01.
 */
uint8_t PadRepeatUpdate(void)
{
    uint8_t a, button, count;
    bool event;

    a = rd8(BiosJoyTrigger);
    WR8(ZP->temp00, a);
    a = rd8(BiosPadState);
    WR8(ZP->temp01, a);
    for (button = 0; button != 8; button++) {
        bool store = true;

        if (LSR8(ZP->temp00)) {
            a = 0x18;
            LSR8(ZP->temp01);
            event = true;
        } else if (LSR8(ZP->temp01)) {
            event = false;
            count = rd8((uint16_t)(PAD_REPEAT + button));
            count = (uint8_t)(count - 1);
            wr8((uint16_t)(PAD_REPEAT + button), count);
            if (count == 0) {
                a = 0x0C;
                event = true;
            } else {
                store = false;
            }
        } else {
            a = 0;
            event = false;
        }
        if (store) {
            wr8((uint16_t)(PAD_REPEAT + button), a);
        }
        count = rd8(PAD_REPEAT_BITS);
        wr8(PAD_REPEAT_BITS, (uint8_t)((count >> 1) | (event ? 0x80 : 0)));
    }
    return a;
}

void abi_PadRepeatUpdate(void)
{
    cpu.a = PadRepeatUpdate();
    cpu.x = 8;
    abi_nz(0);
    abi_flag(P_C, true);
}

/* The digit tile both draw: the value plus $10 to the VDC's data port, the palette byte after it. */
static Alu draw_menu_digit(uint8_t value)
{
    Alu sum = adc8(value, 0x10, false);

    WR8(VDC->data_lo, sum.a);
    st2(0x11);
    return sum;
}

/*
 * @routine 00C4:0CC DrawSelectedFileDigit
 * @run 6B:98CC
 * WHAT IT DOES: Draws the number of the selected file as a digit tile at the screen place $00CE.
 * HOW: Sets the VDC's write address to $00CE and writes the digit tile for the value in $7DF1
 * (its value plus $10, palette 1).
 * WAITS: no.
 * Entry: nothing. Exit: A the value plus $10, N, Z, C and V from that add; X and Y kept.
 */
Alu DrawSelectedFileDigit(void)
{
    jsr_push(0x98CF);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0xCE);
    st2(0x00);
    jsr_push(0x98D6);
    SelectVramData();
    jsr_pop();
    return draw_menu_digit(rd8(SELECTED_FILE));
}

void abi_DrawSelectedFileDigit(void)
{
    Alu result = DrawSelectedFileDigit();

    cpu.a = result.a;
    abi_alu(result);
}

/*
 * @routine 00C4:0E2 DrawCursorRowDigit
 * @run 6B:98E2
 * WHAT IT DOES: Draws the number of the cursor's row as a digit tile at the screen place $012B.
 * HOW: Sets the VDC's write address to $012B and runs into DrawSelectedFileDigit's last
 * part with the value in $7DFF.
 * WAITS: no.
 * Entry: nothing. Exit: as DrawSelectedFileDigit.
 */
Alu DrawCursorRowDigit(void)
{
    jsr_push(0x98E5);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0x2B);
    st2(0x01);
    jsr_push(0x98EC);
    SelectVramData();
    jsr_pop();
    return draw_menu_digit(rd8(CURSOR_ROW));
}

void abi_DrawCursorRowDigit(void)
{
    Alu result = DrawCursorRowDigit();

    cpu.a = result.a;
    abi_alu(result);
}
