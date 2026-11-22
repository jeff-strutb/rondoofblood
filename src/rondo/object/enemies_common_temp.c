/*
 * TEMPORARY: object routines every stage overlay carries its own copy of (they differ only in the
 * disc tables they read), kept together here until each has a home.
 * The disc tables are named here by their overlay address until the typed data layer reads them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/* Whether an item drops when the object is hit hard enough: see DropItemWhenHitA. */
static Ended drop_item_when_hit(uint8_t slot, uint16_t limits, uint16_t items)
{
    uint8_t hit = RD8(OBJ->hit_taken[slot]);
    uint8_t code, bits, y, limit, item, index, type, last, last2;
    bool found;
    uint8_t found_slot;
    RandomResult random;
    Sum sum, added;
    Ended e;

    e = ended_compare(hit, 5);
    if (hit < 5) {
        return e;
    }
    code = RD8(OBJ->f31f8[slot]);
    bits = (uint8_t)((code >> 2) & 7);
    e = ended_then(e, ended_with_carry(ended_load(bits), (code & 2) != 0));
    if (bits == 0) {
        return e;
    }
    y = (uint8_t)(bits - 1);
    e = ended_then(e, ended_with_y(ended_kept(), y));
    WR8(ZP->temp00, (uint8_t)(y << 4));
    jsr_push(0xA2E0);
    random = NextRandom();
    jsr_pop();
    limit = rd8((uint16_t)(limits + y));
    e = ended_then(ended_then(e, ended_of_random(random)), ended_compare(random.value, limit));
    if (random.value >= limit) {
        return e;
    }
    jsr_push(0xA2E8);
    sum = PlayerDistanceSum(slot, false);
    jsr_pop();
    e = ended_then(e, ended_of_sum(sum));
    index = (uint8_t)(sum.value & 0x0F);
    added = add_bytes(index, RD8(ZP->temp00), false);
    y = added.value;
    item = rd8((uint16_t)(items + y));
    WR8(ZP->temp00, item);
    e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(item), added.carry),
                                                          added.overflow), y));
    jsr_push(0xA2F6);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = ended_then(e, ended_with_carry(ended_with_y(ended_load(last), found_slot), found));
    e = ended_then(e, ended_nz(0));
    if (!found) {
        return e;
    }
    type = RD8(ZP->temp00);
    e = ended_then(e, ended_load(type));
    jsr_push(0xA2FD);
    last2 = StartObjectAtPlace(type, slot, found_slot);
    jsr_pop();
    return ended_then(e, ended_load(last2));
}

/*
 * @routine 07D9:2C5 DropItemWhenHitA
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $AB3D
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AB45,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitA(uint8_t slot)
{
    enum { LIMITS = 0xAB3D, ITEMS = 0xAB45 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitA(void)
{
    abi_ended(DropItemWhenHitA(cpu.x));
}

/*
 * @routine 0939:2C5 DropItemWhenHitB
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $ABFD
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AC05,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitB(uint8_t slot)
{
    enum { LIMITS = 0xABFD, ITEMS = 0xAC05 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitB(void)
{
    abi_ended(DropItemWhenHitB(cpu.x));
}

/*
 * @routine 0BF9:2C5 DropItemWhenHitC
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $AC08
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AC10,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitC(uint8_t slot)
{
    enum { LIMITS = 0xAC08, ITEMS = 0xAC10 };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitC(void)
{
    abi_ended(DropItemWhenHitC(cpu.x));
}

/*
 * @routine 0D59:2C5 DropItemWhenHitD
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $ACD2
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $ACDA,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitD(uint8_t slot)
{
    enum { LIMITS = 0xACD2, ITEMS = 0xACDA };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitD(void)
{
    abi_ended(DropItemWhenHitD(cpu.x));
}

/*
 * @routine 1179:2C5 DropItemWhenHitE
 * @run 70:A2C5
 * WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
 * zero, may drop an item: a random number under the limit in a table at overlay address $AC56
 * (indexed by those bits minus one) picks the chance; the item comes from a table at $AC5E,
 * indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
 * a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
 * HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
 * reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
 * coverage (PROCEDURE section 6).
 * WAITS: no.
 * Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
 * free slot (or the table index); X kept. Clobbers zTemp00.
 */
Ended DropItemWhenHitE(uint8_t slot)
{
    enum { LIMITS = 0xAC56, ITEMS = 0xAC5E };

    return drop_item_when_hit(slot, LIMITS, ITEMS);
}

void abi_DropItemWhenHitE(void)
{
    abi_ended(DropItemWhenHitE(cpu.x));
}

/*
 * @routine 088A:5AF FaceThePlayerInSomeStatesC
 * @run 70:ADAF
 * WHAT IT DOES: FaceThePlayerInSomeStates in the overlay at sector 088A, with its table at $ADC0.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as FaceThePlayerInSomeStates.
 */
SideEnd FaceThePlayerInSomeStatesC(uint8_t slot)
{
    return face_player_in_states(slot, 0xADC0, 0xADBA);
}

void abi_FaceThePlayerInSomeStatesC(void)
{
    abi_side_end(FaceThePlayerInSomeStatesC(cpu.x));
}

/*
 * @routine 0A9C:7FE FaceThePlayerInSomeStatesD
 * @run 71:BFFE
 * WHAT IT DOES: FaceThePlayerInSomeStates in the overlay at sector 0A9C, with its table at $C00F.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as FaceThePlayerInSomeStates.
 */
SideEnd FaceThePlayerInSomeStatesD(uint8_t slot)
{
    return face_player_in_states(slot, 0xC00F, 0xC009);
}

void abi_FaceThePlayerInSomeStatesD(void)
{
    abi_side_end(FaceThePlayerInSomeStatesD(cpu.x));
}

/*
 * @routine 0A9E:591 ThrowByNextKindB
 * @run 71:CD91
 * WHAT IT DOES: ThrowByNextKind in the overlay at sector 0A9E, with its X speed table at $CDB4.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as ThrowByNextKind.
 */
Ended ThrowByNextKindB(uint8_t slot)
{
    return throw_by_next_kind(slot, 0xCDB4, 0xCDAD);
}

void abi_ThrowByNextKindB(void)
{
    abi_ended(ThrowByNextKindB(cpu.x));
}

/*
 * @routine 0A9E:5B6 PickTimersAtRandomB
 * @run 71:CDB6
 * WHAT IT DOES: PickTimersAtRandom in the overlay at sector 0A9E, with its table at $CDCA.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as PickTimersAtRandom.
 */
RandomEnd PickTimersAtRandomB(uint8_t slot)
{
    return pick_pair_at_random(slot, 0xCDCA, 0xCDB9, false);
}

void abi_PickTimersAtRandomB(void)
{
    abi_random_end(PickTimersAtRandomB(cpu.x));
}

/*
 * @routine 0A9E:5D2 PickAngleAtRandomB
 * @run 71:CDD2
 * WHAT IT DOES: PickAngleAtRandom in the overlay at sector 0A9E, with its table at $CDE6.
 * HOW: The shared body.
 * WAITS: no.
 * Entry, exit: as PickAngleAtRandom.
 */
RandomEnd PickAngleAtRandomB(uint8_t slot)
{
    return pick_pair_at_random(slot, 0xCDE6, 0xCDD5, true);
}

void abi_PickAngleAtRandomB(void)
{
    abi_random_end(PickAngleAtRandomB(cpu.x));
}

/*
 * @routine 117C:73A WalkWhileFrameChangesD
 * @run 70:BF3A
 * WHAT IT DOES: WalkWhileFrameChangesB in the overlay at sector 117C: lists at $BF56 plus Y, X
 * speeds at $BF5A by f3510.
 * HOW: StepObjectFrames, then ApplyVelocityX.
 * WAITS: no.
 * Entry, exit: as WalkWhileFrameChangesB.
 */
Ended WalkWhileFrameChangesD(uint8_t slot, uint8_t index)
{
    enum { LISTS = 0xBF56, SPEEDS = 0xBF5A };
    uint8_t exit_y = index, phase;
    FrameStep step;
    AxisMove move;
    Ended e;

    WR8(ZP->temp10, rd8((uint16_t)(LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(LISTS + 1 + index)));
    jsr_push(0xBF47);
    step = StepObjectFrames(slot, &exit_y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, exit_y);
    if (step != FRAME_CHANGED) {
        return e;
    }
    phase = RD8(OBJ->f3510[slot]);
    WR8(OBJ->vx_hi[slot], rd8((uint16_t)(SPEEDS + phase)));
    jsr_push(0xBF55);
    move = ApplyVelocityX(slot);
    jsr_pop();
    return ended_then(ended_then(e, ended_with_y(ended_kept(), phase)), ended_of_axis_move(move));
}

void abi_WalkWhileFrameChangesD(void)
{
    abi_ended(WalkWhileFrameChangesD(cpu.x, cpu.y));
}
