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
 * WAITS: no. CountSetBits's arm that counts a bit is in no trace and the draft stops there, so the
 * comparison now runs with every counted byte zero (curated); the sums with counts were compared
 * when the harness still ran the lifted CountSetBits on both sides: 400 of 400, decimal and not.
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

/*
 * @routine 00C3:2E7 PadRepeatReset
 * @run 6B:92E7
 * WHAT IT DOES: Starts the menu's key repeat afresh: no button is down, and each of the eight
 * buttons waits the full first delay.
 * HOW: Zeroes the repeat bits ($7DFA), then loads each of the eight repeat counters (from $7DF2)
 * with 24, from the last to the first.
 * WAITS: no.
 * Entry: nothing. Exit: A $18, X $FF, N set, Z clear; Y and C kept.
 */
void PadRepeatReset(void)
{
    uint8_t index;

    wr8(PAD_REPEAT_BITS, 0);
    index = 7;
    do {
        wr8((uint16_t)(PAD_REPEAT + index), 0x18);
        index--;
    } while (index != 0xFF);
}

void abi_PadRepeatReset(void)
{
    PadRepeatReset();
    cpu.a = 0x18;
    cpu.x = 0xFF;
    abi_nz(0xFF);
}

/*
 * @routine 00C3:31B MenuBackOnButtonII
 * @run 6B:931B
 * WHAT IT DOES: Takes the menu back to its page 8 when button II has just been pressed.
 * HOW: ANDs the card's trigger byte with 2 and returns when it is zero; otherwise runs on into
 * MenuBackToPageEight.
 * WAITS: no.
 * Entry: nothing. Exit: not pressed: A 0, Z set, N clear, C kept. Pressed: as
 * MenuBackToPageEight with N and Z clear.
 */
bool MenuBackOnButtonII(void)
{
    uint8_t trigger;

    trigger = rd8(BiosJoyTrigger);
    if ((trigger & 0x02) == 0) {
        return false;
    }
    MenuBackToPageEight();
    return true;
}

void abi_MenuBackOnButtonII(void)
{
    bool pressed = MenuBackOnButtonII();

    cpu.a = 0;
    abi_nz(pressed ? 8 : 0);
}

/*
 * @routine 00C3:322 MenuBackToPageEight
 * @run 6B:9322
 * WHAT IT DOES: Puts the menu on its page 8, at its first step.
 * HOW: Loads 8 and goes to SetMenuPage.
 * WAITS: no.
 * Entry: nothing. Exit: A 0, N and Z clear (the 8 loaded), C kept.
 */
void MenuBackToPageEight(void)
{
    SetMenuPage(8);
}

void abi_MenuBackToPageEight(void)
{
    MenuBackToPageEight();
    cpu.a = 0;
    abi_nz(8);
}

/*
 * @routine 00C1:228 IncrementTempWord
 * @run 6B:8228
 * WHAT IT DOES: Adds one to the 16-bit pointer in zTemp00 and zTemp01, the place the text
 * printer reads its next byte from.
 * HOW: Adds one to the low byte, and to the high byte when the low byte wrapped to zero.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the byte last incremented, C and the registers kept.
 */
uint8_t IncrementTempWord(void)
{
    uint8_t low;

    low = INC8(ZP->temp00);
    if (low != 0) {
        return low;
    }
    return INC8(ZP->temp01);
}

void abi_IncrementTempWord(void)
{
    abi_nz(IncrementTempWord());
}

/*
 * @routine 00C1:5CF WriteGlyphRows
 * @run 6B:85CF
 * WHAT IT DOES: Writes eight rows of a glyph's tile to the VDC, from the glyph buffer at $3B40:
 * every other byte from the buffer is the low byte of a row's word, $FF the high byte.
 * HOW: Eight times, the buffer byte at X goes to the VDC data port and $FF to its high byte; X
 * goes up by two each row.
 * WAITS: no.
 * Entry: X the buffer offset; the VDC's write address set. Exit: A the last byte written, X plus
 * 16, Y 8, Z and C set, N clear.
 */
GlyphRows WriteGlyphRows(uint8_t offset)
{
    GlyphRows result;
    uint8_t row;

    result.a = 0;
    for (row = 0; row != 8; row++) {
        result.a = rd8((uint16_t)(GLYPH_BUFFER + offset));
        WR8(VDC->data_lo, result.a);
        st2(0xFF);
        offset = (uint8_t)(offset + 2);
    }
    result.x = offset;
    return result;
}

void abi_WriteGlyphRows(void)
{
    GlyphRows result = WriteGlyphRows(cpu.x);

    cpu.a = result.a;
    cpu.x = result.x;
    cpu.y = 8;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 00C1:5CA WriteGlyphRowsTwice
 * @run 6B:85CA
 * WHAT IT DOES: Writes the same eight glyph rows twice, so a glyph fills two tiles' worth.
 * HOW: Keeps X on the stack across one call of WriteGlyphRows, then runs on into it with X as it
 * was.
 * WAITS: no.
 * Entry, exit: as WriteGlyphRows.
 */
GlyphRows WriteGlyphRowsTwice(uint8_t offset)
{
    uint8_t pulled;

    push8(offset);
    jsr_push(0x85CE);
    WriteGlyphRows(offset);
    jsr_pop();
    pulled = pull8();
    return WriteGlyphRows(pulled);
}

void abi_WriteGlyphRowsTwice(void)
{
    GlyphRows result = WriteGlyphRowsTwice(cpu.x);

    cpu.a = result.a;
    cpu.x = result.x;
    cpu.y = 8;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 00DE:05D BlankTextRow
 * @run 6F:A85D
 * WHAT IT DOES: Blanks a row of eight tiles of the menu's text: each tile's number becomes $3D
 * and its palette byte stays.
 * HOW: The row number doubled picks the row's VRAM address from a table, which is set as both
 * the write and the read address; eight times the tile's low byte is read, $3D is written in its
 * place, the high byte is read and written back.
 * WAITS: no.
 * Entry: A the row, X kept on the stack across it (a typed caller passes its own). Exit: A the
 * last high byte, Y the row doubled, X kept with N and Z from it (the PLX), C set.
 */
Exit BlankTextRow(uint8_t row, uint8_t kept_x)
{
    Exit exit = { 0 };
    uint8_t index, value, count;

    push8(kept_x);
    index = (uint8_t)(row << 1);
    jsr_push(0xA863);
    SelectVramWriteAddress();
    jsr_pop();
    value = rd8((uint16_t)(TEXT_ROW_ADDRESSES + index));
    WR8(VDC->data_lo, value);
    value = rd8((uint16_t)(TEXT_ROW_ADDRESSES + 1 + index));
    WR8(VDC->data_hi, value);
    jsr_push(0xA872);
    SelectVramReadAddress();
    jsr_pop();
    value = rd8((uint16_t)(TEXT_ROW_ADDRESSES + index));
    WR8(VDC->data_lo, value);
    value = rd8((uint16_t)(TEXT_ROW_ADDRESSES + 1 + index));
    WR8(VDC->data_hi, value);
    jsr_push(0xA881);
    SelectVramData();
    jsr_pop();
    value = 0;
    for (count = 0; count != 8; count++) {
        RD8(VDC->data_lo);
        st1(0x3D);
        value = RD8(VDC->data_hi);
        WR8(VDC->data_hi, value);
    }
    exit.a = value;
    exit.y = index;
    exit.x = pull8();
    exit.nz = exit.x;
    exit.c = true;
    exit.v_set = false;
    return exit;
}

void abi_BlankTextRow(void)
{
    exit_shim(BlankTextRow(cpu.a, cpu.x));
}

/*
 * @routine 00DE:0B2 RecolorTextRun
 * @run 6F:A8B2
 * WHAT IT DOES: Gives a run of the menu's text a palette: from the row's VRAM address on, each tile
 * that has a number is rewritten with the palette byte, until a tile whose high nibble is 3 ends
 * the run; tiles whose high nibble is 0 are rewritten as they are.
 * HOW: The palette byte is looked up by Y and kept in $F8. The row (doubled) picks the VRAM
 * address, set as read and write address. Each pass reads a word, keeps its high byte in $F9, and
 * writes the word back with its own high byte, or with $F8 as the high byte, or stops.
 * WAITS: no.
 * Entry: A the row, Y the palette index. Exit: A $30, Y the last tile's low byte, Z and C set, N
 * clear; X kept. Clobbers zBiosAl and zBiosAh.
 */
Exit RecolorTextRun(uint8_t row, uint8_t palette_index)
{
    Exit exit = { 0 };
    uint8_t value, index, low, high, masked, kept;

    push8(row);
    value = rd8((uint16_t)(TEXT_PALETTES + palette_index));
    WR8(ZP->bios_a.lo, value);
    row = pull8();
    index = (uint8_t)(row << 1);
    jsr_push(0xA8BE);
    SelectVramReadAddress();
    jsr_pop();
    value = rd8((uint16_t)(TEXT_COLOR_ADDRESSES + index));
    WR8(VDC->data_lo, value);
    value = rd8((uint16_t)(TEXT_COLOR_ADDRESSES + 1 + index));
    WR8(VDC->data_hi, value);
    jsr_push(0xA8CD);
    SelectVramWriteAddress();
    jsr_pop();
    value = rd8((uint16_t)(TEXT_COLOR_ADDRESSES + index));
    WR8(VDC->data_lo, value);
    value = rd8((uint16_t)(TEXT_COLOR_ADDRESSES + 1 + index));
    WR8(VDC->data_hi, value);
    jsr_push(0xA8DC);
    SelectVramData();
    jsr_pop();
    for (;;) {
        low = RD8(VDC->data_lo);
        high = RD8(VDC->data_hi);
        WR8(ZP->bios_a.hi, high);
        masked = (uint8_t)(high & 0xF0);
        if (masked == 0) {
            kept = RD8(ZP->bios_a.hi);
            WR8(VDC->data_lo, low);
            WR8(VDC->data_hi, kept);
            continue;
        }
        if (masked == 0x30) {
            break;
        }
        kept = RD8(ZP->bios_a.lo);
        WR8(VDC->data_lo, low);
        WR8(VDC->data_hi, kept);
    }
    exit.a = 0x30;
    exit.y = low;
    exit.nz = 0;
    exit.c = true;
    exit.v_set = false;
    exit.x = 0;
    exit.x_kept = true;
    return exit;
}

void abi_RecolorTextRun(void)
{
    exit_shim(RecolorTextRun(cpu.a, cpu.y));
}

/*
 * @routine 00DE:006 DrawSlotPercent
 * @run 6F:A806
 * WHAT IT DOES: Writes the completion of a save slot as a number of up to three digits at the
 * place the menu has for the row: "100" or a blank and two digits.
 * HOW: The row is kept doubled (the place table's index) and reduced by repeated subtraction of 3
 * to the slot (0 to 2); the VRAM write address is set from the table, SlotCompletionPercent
 * works the number out in zTemp08 and zTemp09, and a 1 digit tile, or a blank tile, and the low
 * byte as two hex digits are written.
 * WAITS: no.
 * Entry: A the row, X kept on the stack across it (a typed caller passes its own). Exit: A the
 * last digit tile, N and Z from X, C bit 3 of the low byte written, V as the last subtraction left it
 * (kept when the row is under 3), X kept, Y the slot.
 */
Exit DrawSlotPercent(uint8_t row, uint8_t kept_x)
{
    Exit exit = { 0 };
    SlotSum sum;
    Alu minus;
    uint8_t index, slot, value, high;
    bool v_set = false, v = false;

    push8(kept_x);
    push8(row);
    index = (uint8_t)(row << 1);
    row = pull8();
    while (row >= 3) {
        minus = sbc8(row, 3, true);
        row = minus.a;
        v_set = true;
        v = minus.v;
    }
    slot = row;
    jsr_push(0xA817);
    SelectVramWriteAddress();
    jsr_pop();
    value = rd8((uint16_t)(PERCENT_ADDRESSES + index));
    WR8(VDC->data_lo, value);
    value = rd8((uint16_t)(PERCENT_ADDRESSES + 1 + index));
    WR8(VDC->data_hi, value);
    jsr_push(0xA826);
    SelectVramData();
    jsr_pop();
    push8(index);
    push8(slot);
    jsr_push(0xA82C);
    sum = SlotCompletionPercent(slot);
    jsr_pop();
    exit.y = pull8();
    exit.x = pull8();
    high = RD8(ZP->temp09);
    if (high != 0) {
        jsr_push(0xA837);
        WriteDigitTile(1);
        jsr_pop();
        value = 0;
    } else {
        st1(0x20);
        st2(0x11);
        value = RD8(ZP->temp08);
    }
    jsr_push(0xA843);
    WriteHexByteTiles(value);
    jsr_pop();
    exit.a = (uint8_t)((value & 0x0F) | 0x10);
    exit.c = (value & 0x08) != 0;
    exit.x = pull8();
    exit.nz = exit.x;
    exit.v = v;
    exit.v_set = v_set;
    return exit;
}

void abi_DrawSlotPercent(void)
{
    exit_shim(DrawSlotPercent(cpu.a, cpu.x));
}

/*
 * @routine 00DD:133 OrBufferIntoVramLow
 * @run 6F:A133
 * WHAT IT DOES: Merges the glyph buffer into the VRAM words from the read address on: the low
 * byte of each of 16 words has the buffer's bytes ORed in (every other byte, from $3B40).
 * HOW: Sixteen times the low byte is read, ORed with the buffer byte and written, then the high
 * byte is read and written back.
 * WAITS: no.
 * Entry: X kept on the stack across it (a typed caller passes its own); the VDC's read and write
 * addresses set. Exit: A the last high byte, X kept with N and Z from it, C set.
 */
Exit OrBufferIntoVramLow(uint8_t kept_x)
{
    Exit exit = { 0 };
    uint8_t offset, low, high, byte;

    push8(kept_x);
    high = 0;
    for (offset = 0; offset != 0x20; offset = (uint8_t)(offset + 2)) {
        low = RD8(VDC->data_lo);
        byte = rd8((uint16_t)(GLYPH_BUFFER + offset));
        WR8(VDC->data_lo, (uint8_t)(low | byte));
        high = RD8(VDC->data_hi);
        WR8(VDC->data_hi, high);
    }
    exit.a = high;
    exit.x = pull8();
    exit.nz = exit.x;
    exit.c = true;
    exit.v_set = false;
    exit.y_kept = true;
    return exit;
}

void abi_OrBufferIntoVramLow(void)
{
    exit_shim(OrBufferIntoVramLow(cpu.x));
}

/*
 * @routine 00DD:17B ShiftGlyphBufferRight
 * @run 6F:A17B
 * WHAT IT DOES: Shifts the glyph buffer's twelve words right by one bit, to move a glyph a pixel
 * along, and sets the column offset zTemp08 to $10.
 * HOW: Twelve times the high byte (at $3B44) is shifted right and the carry rotated into the low
 * byte after it; then $10 is stored in zTemp08.
 * WAITS: no.
 * Entry: X kept on the stack across it (a typed caller passes its own). Exit: A $10, N and Z
 * clear, C set; X kept.
 */
Exit ShiftGlyphBufferRight(uint8_t kept_x)
{
    Exit exit = { 0 };
    uint8_t offset;
    bool carry;

    push8(kept_x);
    for (offset = 0; offset != 0x18; offset = (uint8_t)(offset + 2)) {
        carry = lsr_mem((uint16_t)(GLYPH_BUFFER + 4 + offset));
        ror_mem((uint16_t)(GLYPH_BUFFER + 5 + offset), carry);
    }
    exit.x = pull8();
    WR8(ZP->temp08, 0x10);
    exit.a = 0x10;
    exit.nz = 0x10;
    exit.c = true;
    exit.v_set = false;
    exit.y_kept = true;
    return exit;
}

void abi_ShiftGlyphBufferRight(void)
{
    exit_shim(ShiftGlyphBufferRight(cpu.x));
}

/*
 * @routine 00DD:1A3 SetGlyphVramPlace
 * @run 6F:A1A3
 * WHAT IT DOES: Points the VDC's read and write addresses at a glyph's place in VRAM: the
 * object's base place (from a table by the object's kind), plus the column offset zBiosAl, plus
 * the glyph number times 64 (with the half-column in zTemp08 ORed in).
 * HOW: The glyph number is shifted right twice into zTemp01 with the two bits shifted out kept
 * as the top of zTemp00, which zTemp08 is ORed into. The base place is added to zBiosAl and
 * zTemp00 (the carry of the first add is dropped, as in the original) and zTemp01, written to the
 * write address, and then the same word to the read address; the data port is selected.
 * WAITS: no.
 * Entry: A the glyph number, X the object, zBiosAl, zTemp08. Exit: A 2, N and Z clear, C and V from
 * the high byte add, Y the kind doubled; X kept. Clobbers zTemp00 to zTemp03.
 */
Exit SetGlyphVramPlace(uint8_t glyph, uint8_t object)
{
    Exit exit = { 0 };
    Alu add_low, add_more, add_high;
    uint8_t index, kind, base, low, high, carry_in;
    bool carry;

    WR8(ZP->temp00, 0);
    carry = (glyph & 0x01) != 0;
    glyph = (uint8_t)(glyph >> 1);
    ROR8(ZP->temp00, carry);
    carry = (glyph & 0x01) != 0;
    glyph = (uint8_t)(glyph >> 1);
    ROR8(ZP->temp00, carry);
    WR8(ZP->temp01, glyph);
    low = RD8(ZP->temp00);
    high = RD8(ZP->temp08);
    WR8(ZP->temp00, (uint8_t)(low | high));
    kind = RD8(OBJ->f3040[object]);
    index = (uint8_t)(kind << 1);
    jsr_push(0xA1BB);
    SelectVramWriteAddress();
    jsr_pop();
    base = rd8((uint16_t)(GLYPH_PLACES + index));
    low = RD8(ZP->bios_a.lo);
    add_low = adc8(base, low, false);
    high = RD8(ZP->temp00);
    add_more = adc8(add_low.a, high, false);
    WR8(ZP->temp02, add_more.a);
    WR8(VDC->data_lo, add_more.a);
    base = rd8((uint16_t)(GLYPH_PLACES + 1 + index));
    carry_in = RD8(ZP->temp01);
    add_high = adc8(base, carry_in, add_more.c);
    WR8(ZP->temp03, add_high.a);
    WR8(VDC->data_hi, add_high.a);
    jsr_push(0xA1D6);
    SelectVramReadAddress();
    jsr_pop();
    low = RD8(ZP->temp02);
    WR8(VDC->data_lo, low);
    high = RD8(ZP->temp03);
    WR8(VDC->data_hi, high);
    SelectVramData();
    exit.a = 2;
    exit.y = index;
    exit.nz = 2;
    exit.c = add_high.c;
    exit.v = add_high.v;
    exit.v_set = true;
    exit.x_kept = true;
    return exit;
}

void abi_SetGlyphVramPlace(void)
{
    exit_shim(SetGlyphVramPlace(cpu.a, cpu.x));
}

/*
 * @routine 00DD:104 OverlayGlyphRows
 * @run 6F:A104
 * WHAT IT DOES: Draws a glyph into the tiles of an object's row by overlaying it on what is in
 * VRAM: two places one glyph number apart are set, the first has the glyph buffer's low bytes
 * ORed in, the second its high bytes.
 * HOW: The object's step (OBJ->f35c0) picks a glyph number from a table. SetGlyphVramPlace for
 * it, OrBufferIntoVramLow, then SetGlyphVramPlace for the number plus one and a loop of 16 words
 * that reads each word, writes it back and ORs the buffer's odd bytes (from $3B41) into the high
 * byte.
 * WAITS: no.
 * Entry: X the object. Exit: A the last high byte written, X kept (N and Z from it), C set, Y the
 * object's kind doubled, V from SetGlyphVramPlace.
 */
Exit OverlayGlyphRows(uint8_t object)
{
    Exit exit = { 0 };
    Exit place;
    uint8_t step, glyph, offset, low, high, byte;

    step = RD8(OBJ->f35c0[object]);
    glyph = rd8((uint16_t)(GLYPH_NUMBERS + step));
    jsr_push(0xA10D);
    SetGlyphVramPlace(glyph, object);
    jsr_pop();
    jsr_push(0xA110);
    OrBufferIntoVramLow(object);
    jsr_pop();
    step = RD8(OBJ->f35c0[object]);
    glyph = rd8((uint16_t)(GLYPH_NUMBERS + step));
    glyph = (uint8_t)(glyph + 1);
    jsr_push(0xA11A);
    place = SetGlyphVramPlace(glyph, object);
    jsr_pop();
    push8(object);
    high = 0;
    for (offset = 0; offset != 0x20; offset = (uint8_t)(offset + 2)) {
        low = RD8(VDC->data_lo);
        WR8(VDC->data_lo, low);
        high = RD8(VDC->data_hi);
        byte = rd8((uint16_t)(GLYPH_BUFFER + 1 + offset));
        high = (uint8_t)(high | byte);
        WR8(VDC->data_hi, high);
    }
    exit.a = high;
    exit.x = pull8();
    exit.y = place.y;
    exit.nz = exit.x;
    exit.c = true;
    exit.v = place.v;
    exit.v_set = true;
    return exit;
}

void abi_OverlayGlyphRows(void)
{
    exit_shim(OverlayGlyphRows(cpu.x));
}

/*
 * @routine 00DD:0FA OverlayGlyphPass
 * @run 6F:A0FA
 * WHAT IT DOES: One pass of the object's glyph draw: OverlayGlyphRows with the column offset 0,
 * then the glyph buffer shifted right a bit and OverlayGlyphRows again with the offset 1.
 * HOW: Sets zBiosAl to 0, calls OverlayGlyphRows, adds one to zBiosAl, calls
 * ShiftGlyphBufferRight, and runs on into OverlayGlyphRows.
 * WAITS: no.
 * Entry: X the object. Exit: as OverlayGlyphRows.
 */
Exit OverlayGlyphPass(uint8_t object)
{
    WR8(ZP->bios_a.lo, 0);
    jsr_push(0xA0FF);
    OverlayGlyphRows(object);
    jsr_pop();
    INC8(ZP->bios_a.lo);
    jsr_push(0xA104);
    ShiftGlyphBufferRight(object);
    jsr_pop();
    return OverlayGlyphRows(object);
}

void abi_OverlayGlyphPass(void)
{
    exit_shim(OverlayGlyphPass(cpu.x));
}

/*
 * @routine 00DD:156 OverlayGlyphRowsBoth
 * @run 6F:A156
 * WHAT IT DOES: Draws a glyph into the tiles of an object's row by overlaying it on what is in
 * VRAM, both bytes of each word: the low bytes from the buffer's even bytes and the high bytes
 * from the odd ones.
 * HOW: The object's step (OBJ->f35c0) picks a glyph number from a table, SetGlyphVramPlace sets the
 * place, then 16 words are read and written back with the buffer ORed in.
 * WAITS: no.
 * Entry: X the object. Exit: A the last high byte written, X kept (N and Z from it), C set, Y the
 * object's kind doubled, V from SetGlyphVramPlace.
 */
Exit OverlayGlyphRowsBoth(uint8_t object)
{
    Exit exit = { 0 };
    Exit place;
    uint8_t step, glyph, offset, low, high, byte;

    step = RD8(OBJ->f35c0[object]);
    glyph = rd8((uint16_t)(GLYPH_NUMBERS + step));
    jsr_push(0xA15F);
    place = SetGlyphVramPlace(glyph, object);
    jsr_pop();
    push8(object);
    high = 0;
    for (offset = 0; offset != 0x20; offset = (uint8_t)(offset + 2)) {
        low = RD8(VDC->data_lo);
        byte = rd8((uint16_t)(GLYPH_BUFFER + 1 + offset));
        WR8(VDC->data_lo, (uint8_t)(low | byte));
        high = RD8(VDC->data_hi);
        byte = rd8((uint16_t)(GLYPH_BUFFER + offset));
        WR8(VDC->data_hi, (uint8_t)(high | byte));
    }
    exit.a = (uint8_t)(high | byte);
    exit.x = pull8();
    exit.y = place.y;
    exit.nz = exit.x;
    exit.c = true;
    exit.v = place.v;
    exit.v_set = true;
    return exit;
}

void abi_OverlayGlyphRowsBoth(void)
{
    exit_shim(OverlayGlyphRowsBoth(cpu.x));
}

/*
 * @routine 00DD:14C OverlayGlyphPassBoth
 * @run 6F:A14C
 * WHAT IT DOES: One pass of the object's glyph draw with both bytes overlaid: as OverlayGlyphPass
 * with OverlayGlyphRowsBoth in place of OverlayGlyphRows.
 * HOW: Sets zBiosAl to 0, calls OverlayGlyphRowsBoth, adds one to zBiosAl, calls
 * ShiftGlyphBufferRight, and runs on into OverlayGlyphRowsBoth.
 * WAITS: no.
 * Entry: X the object. Exit: as OverlayGlyphRowsBoth.
 */
Exit OverlayGlyphPassBoth(uint8_t object)
{
    WR8(ZP->bios_a.lo, 0);
    jsr_push(0xA151);
    OverlayGlyphRowsBoth(object);
    jsr_pop();
    INC8(ZP->bios_a.lo);
    jsr_push(0xA156);
    ShiftGlyphBufferRight(object);
    jsr_pop();
    return OverlayGlyphRowsBoth(object);
}

void abi_OverlayGlyphPassBoth(void)
{
    exit_shim(OverlayGlyphPassBoth(cpu.x));
}

/*
 * @routine 00DD:01C ClearVramBlock
 * @run 6F:A01C
 * WHAT IT DOES: Blanks 1024 words of VRAM at the place the object's kind names, and moves the
 * object on to its next state.
 * HOW: The kind doubled picks the place from a table, set as the write address; $0000 is
 * written 1024 times (four rounds of 256) and the object's state (OBJ->f27a8) goes up by one.
 * WAITS: no.
 * Entry: X the object. Exit: A 4, X kept, N and Z from the new state, C bit 7 of the kind, Y the
 * kind doubled. Clobbers zTemp00.
 */
Exit ClearVramBlock(uint8_t object)
{
    Exit exit = { 0 };
    uint8_t kind, index, value, rounds, count, state;

    kind = RD8(OBJ->f3040[object]);
    index = (uint8_t)(kind << 1);
    jsr_push(0xA024);
    SelectVramWriteAddress();
    jsr_pop();
    value = rd8((uint16_t)(GLYPH_PLACES + index));
    WR8(VDC->data_lo, value);
    value = rd8((uint16_t)(GLYPH_PLACES + 1 + index));
    WR8(VDC->data_hi, value);
    jsr_push(0xA033);
    SelectVramData();
    jsr_pop();
    push8(object);
    WR8(ZP->temp00, 4);
    count = 0;
    do {
        do {
            st1(0x00);
            st2(0x00);
            count = (uint8_t)(count - 1);
        } while (count != 0);
        rounds = DEC8(ZP->temp00);
    } while (rounds != 0);
    object = pull8();
    state = INC8(OBJ->f27a8[object]);
    exit.a = 4;
    exit.x_kept = true;
    exit.y = index;
    exit.nz = state;
    exit.c = (kind & 0x80) != 0;
    return exit;
}

void abi_ClearVramBlock(void)
{
    exit_shim(ClearVramBlock(cpu.x));
}

/*
 * @routine 00DD:1E3 GlideObjectUp
 * @run 6F:A1E3
 * WHAT IT DOES: One frame of a short slide: on every other frame the object moves up one pixel
 * (its Y goes down by one), the step counter counts down, and at zero the object goes on to its
 * next state.
 * HOW: The counter (OBJ->f35c0) read and shifted right gives the odd or even frame; on an even
 * one OBJ->y_lo is decremented; the counter is decremented and at zero OBJ->f27a8 is incremented.
 * WAITS: no.
 * Entry: X the object. Exit: A the counter shifted right, X kept, C the counter's bit 0, N and Z
 * from the counter after the decrement (from the new state when it reached zero).
 */
Exit GlideObjectUp(uint8_t object)
{
    Exit exit = { 0 };
    uint8_t counter, left, state;

    counter = RD8(OBJ->f35c0[object]);
    exit.c = (counter & 0x01) != 0;
    exit.a = (uint8_t)(counter >> 1);
    if (!exit.c) {
        DEC8(OBJ->y_lo[object]);
    }
    left = DEC8(OBJ->f35c0[object]);
    exit.nz = left;
    if (left == 0) {
        state = INC8(OBJ->f27a8[object]);
        exit.nz = state;
    }
    exit.x_kept = true;
    exit.y_kept = true;
    return exit;
}

void abi_GlideObjectUp(void)
{
    exit_shim(GlideObjectUp(cpu.x));
}

/*
 * @routine 00DD:215 GlideObjectDown
 * @run 6F:A215
 * WHAT IT DOES: One frame of a short slide down: on every other frame the object moves down one
 * pixel, the step counter counts down, and at zero the object is cleared away.
 * HOW: The counter (OBJ->f35c0) read and shifted right gives the odd or even frame; on an even
 * one OBJ->y_lo is incremented; the counter is decremented and at zero ClearObject empties the
 * slot.
 * WAITS: no.
 * Entry: X the object. Exit: A the counter shifted right, X kept, C the counter's bit 0, N and Z
 * from the counter after the decrement, Y kept.
 */
Exit GlideObjectDown(uint8_t object)
{
    Exit exit = { 0 };
    uint8_t counter, left;

    counter = RD8(OBJ->f35c0[object]);
    exit.c = (counter & 0x01) != 0;
    exit.a = (uint8_t)(counter >> 1);
    if (!exit.c) {
        INC8(OBJ->y_lo[object]);
    }
    left = DEC8(OBJ->f35c0[object]);
    exit.nz = left;
    exit.x_kept = true;
    exit.y_kept = true;
    if (left == 0) {
        ClearObject(object);
    }
    return exit;
}

void abi_GlideObjectDown(void)
{
    exit_shim(GlideObjectDown(cpu.x));
}

/*
 * @routine 00DD:0DC GlyphPassByStep
 * @run 6F:A0DC
 * WHAT IT DOES: Chooses how the object's glyph is drawn this step: the object's step (OBJ->f35c0)
 * low two bits pick OverlayGlyphPass (steps 0 and 3) or OverlayGlyphPassBoth (steps 1 and 2).
 * HOW: The two bits doubled index a table of handler addresses; the address goes to zTemp00 and
 * zTemp01 and is jumped through; zTemp08 is cleared first.
 * WAITS: no.
 * Entry: X the object. Exit: as the handler chosen.
 */
Exit GlyphPassByStep(uint8_t object)
{
    uint8_t step, index, value;
    uint16_t target;
    Exit exit = { 0 };

    step = RD8(OBJ->f35c0[object]);
    index = (uint8_t)((step & 0x03) << 1);
    value = rd8((uint16_t)(GLYPH_PASSES + index));
    WR8(ZP->temp00, value);
    value = rd8((uint16_t)(GLYPH_PASSES + 1 + index));
    WR8(ZP->temp01, value);
    /* the two addresses in the table are the two typed passes; a jump to any other is entered with
     * the registers as the original has them at its JMP */
    WR8(ZP->temp08, 0);
    target = RD16_LH2(ZP->temp00, ZP->temp01);
    if (target == 0xA0FA) {
        return OverlayGlyphPass(object);
    }
    if (target == 0xA14C) {
        return OverlayGlyphPassBoth(object);
    }
    cpu.a = value;
    cpu.y = index;
    abi_nz(value);
    abi_flag(P_C, false);
    call_original(target);
    exit.from_cpu = true;
    return exit;
}

void abi_GlyphPassByStep(void)
{
    exit_shim(GlyphPassByStep(cpu.x));
}

/*
 * @routine 00DD:50A PlaceObjectList
 * @run 6F:A50A
 * WHAT IT DOES: Puts a list of scene objects on the screen: each is cleared and given a place and
 * a picture frame, all the same X high byte, in consecutive slots from the one the list names.
 * HOW: A (doubled) picks the list's address from a table. The list's first byte is the first slot
 * (kept in X), its second the common X high byte (in zTemp08); then entries of three bytes (X low,
 * Y low, frame) until an X low of zero. The pointer is zTemp00 and zTemp01; ClearObject empties
 * each slot, whose kind of picture (OBJ->f2a10) is set to 4.
 * WAITS: no.
 * Entry: A the list. Exit: A 0, Z set, N clear; X the slot after the last placed; Y 2 (1 for an
 * empty list); C and V from the last pointer add.
 */
Exit PlaceObjectList(uint8_t list)
{
    Exit exit = { 0 };
    Alu sum;
    uint8_t index, slot, value, low, y;

    index = (uint8_t)(list << 1);
    value = rd8((uint16_t)(OBJECT_LISTS + index));
    WR8(ZP->temp00, value);
    value = rd8((uint16_t)(OBJECT_LISTS + 1 + index));
    WR8(ZP->temp01, value);
    slot = RD8_PTR(ZP->temp00);
    y = 1;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(ZP->temp08, value);
    low = RD8(ZP->temp00);
    sum = adc8(low, 2, false);
    WR8(ZP->temp00, sum.a);
    if (sum.c) {
        INC8(ZP->temp01);
    }
    for (;;) {
        jsr_push(0xA52D);
        ClearObject(slot);
        jsr_pop();
        value = RD8_PTR(ZP->temp00);
        if (value == 0) {
            break;
        }
        WR8(OBJ->x_lo[slot], value);
        value = RD8(ZP->temp08);
        WR8(OBJ->x_hi[slot], value);
        y = 1;
        value = RD8_PTR_Y(ZP->temp00, y);
        WR8(OBJ->y_lo[slot], value);
        y = 2;
        value = RD8_PTR_Y(ZP->temp00, y);
        WR8(OBJ->f2a68[slot], value);
        WR8(OBJ->f2a10[slot], 4);
        low = RD8(ZP->temp00);
        sum = adc8(low, 3, false);
        WR8(ZP->temp00, sum.a);
        if (sum.c) {
            INC8(ZP->temp01);
        }
        slot++;
    }
    exit.a = 0;
    exit.x = slot;
    exit.y = y;
    exit.nz = 0;
    exit.c = sum.c;
    exit.v = sum.v;
    exit.v_set = true;
    return exit;
}

void abi_PlaceObjectList(void)
{
    exit_shim(PlaceObjectList(cpu.a));
}

/*
 * @routine 00DD:7D2 DrawSlotName
 * @run 6F:A7D2
 * WHAT IT DOES: Writes the eight characters of a save slot's name to the screen at the place the
 * menu has for the row.
 * HOW: The row (doubled) picks the place from a table; the row is reduced by repeated subtraction
 * of 3 to the slot; the VDC's write address is set; the eight characters from the slot's name
 * (at $15 into the slot) go out as tiles with palette 1.
 * WAITS: no.
 * Entry: A the row, X kept on the stack across it (a typed caller passes its own). Exit: A the
 * last character, Y the slot's offset plus 8, X kept (N and Z from it), C set, V as the last
 * subtraction left it (kept when the row is under 3).
 */
Exit DrawSlotName(uint8_t row, uint8_t kept_x)
{
    Exit exit = { 0 };
    Alu minus;
    uint8_t index, slot, value, offset, count;
    bool v_set = false, v = false;

    push8(kept_x);
    push8(row);
    index = (uint8_t)(row << 1);
    row = pull8();
    while (row >= 3) {
        minus = sbc8(row, 3, true);
        row = minus.a;
        v_set = true;
        v = minus.v;
    }
    slot = row;
    jsr_push(0xA7E3);
    SelectVramWriteAddress();
    jsr_pop();
    value = rd8((uint16_t)(TEXT_ROW_ADDRESSES + index));
    WR8(VDC->data_lo, value);
    value = rd8((uint16_t)(TEXT_ROW_ADDRESSES + 1 + index));
    WR8(VDC->data_hi, value);
    jsr_push(0xA7F2);
    SelectVramData();
    jsr_pop();
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    for (count = 0; count != 8; count++) {
        value = rd8((uint16_t)(SAVE_FILE + 0x15 + offset));
        WR8(VDC->data_lo, value);
        st2(0x11);
        offset++;
    }
    exit.a = value;
    exit.y = offset;
    exit.x = pull8();
    exit.nz = exit.x;
    exit.c = true;
    exit.v = v;
    exit.v_set = v_set;
    return exit;
}

void abi_DrawSlotName(void)
{
    exit_shim(DrawSlotName(cpu.a, cpu.x));
}

/*
 * @routine 00DE:183 DrawSlotContinuesAndFace
 * @run 6F:A983
 * WHAT IT DOES: Writes the chosen save slot's continues left (two digits, from its $1D and $1E
 * bytes) at the screen place $06AD, and the character's face tile pair, at $0229.
 * HOW: The write address is set to $06AD; the byte at $1E goes out as one digit and the byte at $1D
 * as two; the write address is set to $0229 and the character (at $7DE3 up, by the slot) picks a
 * pair of tile numbers from a table, written with palette 1.
 * WAITS: no.
 * Entry: nothing. Exit: A the second tile number, X the slot's offset, Y the table index, N and Z
 * from A, C bit 7 of the character.
 */
Exit DrawSlotContinuesAndFace(void)
{
    Exit exit = { 0 };
    uint8_t slot, offset, value, face, index;

    jsr_push(0xA986);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0xAD);
    st2(0x06);
    jsr_push(0xA98D);
    SelectVramData();
    jsr_pop();
    slot = RD8(ZP->save_slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    value = rd8((uint16_t)(SAVE_FILE + 0x1E + offset));
    jsr_push(0xA997);
    WriteLowDigitTile(value);
    jsr_pop();
    value = rd8((uint16_t)(SAVE_FILE + 0x1D + offset));
    jsr_push(0xA99C);
    WriteHexByteTiles(value);
    jsr_pop();
    jsr_push(0xA99F);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0x29);
    st2(0x02);
    jsr_push(0xA9A6);
    SelectVramData();
    jsr_pop();
    slot = RD8(ZP->save_slot);
    face = rd8((uint16_t)(SLOT_FACES + slot));
    index = (uint8_t)(face << 1);
    value = rd8((uint16_t)(FACE_TILES + index));
    WR8(VDC->data_lo, value);
    st2(0x11);
    value = rd8((uint16_t)(FACE_TILES + 1 + index));
    WR8(VDC->data_lo, value);
    st2(0x11);
    exit.a = value;
    exit.x = offset;
    exit.y = index;
    exit.nz = value;
    exit.c = (face & 0x80) != 0;
    return exit;
}

void abi_DrawSlotContinuesAndFace(void)
{
    exit_shim(DrawSlotContinuesAndFace());
}

/*
 * @routine 00DE:153 DrawSlotDetails
 * @run 6F:A953
 * WHAT IT DOES: Writes the chosen save slot's continues and face (DrawSlotContinuesAndFace) and
 * its score total, six digits, at the screen place $022E.
 * HOW: DrawSlotContinuesAndFace; the write address set to $022E; the three score bytes (at $1F to
 * $21 into the slot, highest first) as hex digit pairs, the last by running on into
 * WriteHexByteTiles.
 * WAITS: no.
 * Entry: nothing. Exit: A the last digit tile, N and Z from it, C bit 3 of the last score byte, X the
 * slot's offset, Y the slot.
 */
Exit DrawSlotDetails(void)
{
    Exit exit = { 0 };
    uint8_t slot, offset, value;

    jsr_push(0xA955);
    DrawSlotContinuesAndFace();
    jsr_pop();
    jsr_push(0xA958);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0x2E);
    st2(0x02);
    jsr_push(0xA95F);
    SelectVramData();
    jsr_pop();
    slot = RD8(ZP->save_slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    value = rd8((uint16_t)(SAVE_FILE + 0x21 + offset));
    jsr_push(0xA969);
    WriteHexByteTiles(value);
    jsr_pop();
    value = rd8((uint16_t)(SAVE_FILE + 0x20 + offset));
    jsr_push(0xA96E);
    WriteHexByteTiles(value);
    jsr_pop();
    value = rd8((uint16_t)(SAVE_FILE + 0x1F + offset));
    WriteHexByteTiles(value);
    exit.a = (uint8_t)((value & 0x0F) | 0x10);
    exit.x = offset;
    exit.y = slot;
    exit.nz = exit.a;
    exit.c = (value & 0x08) != 0;
    return exit;
}

void abi_DrawSlotDetails(void)
{
    exit_shim(DrawSlotDetails());
}
