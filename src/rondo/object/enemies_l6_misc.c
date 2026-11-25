/*
 * TEMPORARY: the stage misc object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return how the routine's last
 * instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
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

/* The exit of the angle scales (scale_exit_shim): A, N and Z, C, V when an add ran, and Y 0. */
static Ended ended_of_scale(ScaleResult r)
{
    Ended e = ended_with_y(ended_with_carry(ended_with_a(ended_nz(r.nz), r.a), r.carry), 0);

    return r.v_set ? ended_with_overflow(e, r.overflow) : e;
}

/* AngleToPlayer's exit: A, N, Z, C, V and Y. */
static Ended ended_of_angle(AngleExit r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.a), r.c), r.v), r.y);
}

/*
 * @routine 072C:5A7 LoadRecordA06
 * @run 70:BDA7
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
Ended LoadRecordA06(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(0xBDAA);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    return ended_of_record(last, nz, carry);
}

void abi_LoadRecordA06(void)
{
    abi_ended(LoadRecordA06(cpu.x));
}

/*
 * @routine 092E:2D1 ClearHitsTaken
 * @run 6B:8AD1
 * WHAT IT DOES: Sets the object's hits taken (OBJ_33B0) to 0.
 * HOW: A store.
 * WAITS: no.
 * Entry: X object. Exit: all kept.
 */
Ended ClearHitsTaken(uint8_t slot)
{
    WR8(OBJ->hit_taken[slot], 0);
    return ended_kept();
}

void abi_ClearHitsTaken(void)
{
    abi_ended(ClearHitsTaken(cpu.x));
}

/*
 * @routine 12DF:038 RandomTimerC0
 * @run 71:D038
 * WHAT IT DOES: Sets the object's timer OBJ_34B8 to a random number with bits 6 and 7 set.
 * HOW: NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: A the timer, N set, Z clear; C and V from the random number.
 */
Ended RandomTimerC0(uint8_t slot)
{
    RandomResult random;
    uint8_t value;

    jsr_push(0xD03B);
    random = NextRandom();
    jsr_pop();
    value = (uint8_t)(random.value | 0xC0);
    WR8(OBJ->f34b8[slot], value);
    return then(ended_of_random(random), ended_load(value));
}

void abi_RandomTimerC0(void)
{
    abi_ended(RandomTimerC0(cpu.x));
}

/*
 * @routine 0E0B:5DD NextPhaseUnlessKind25
 * @run 70:B5DD
 * WHAT IT DOES: Goes on to the next OBJ_2800 unless the object in slot $50 is of kind $25.
 * HOW: A compare and an INC.
 * WAITS: no.
 * Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z.
 */
Ended NextPhaseUnlessKind25(uint8_t slot)
{
    uint8_t kind, phase;
    Ended e;

    kind = RD8(OBJ->kind[0x50]);
    e = ended_compare(kind, 0x25);
    if (kind == 0x25) {
        return e;
    }
    phase = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(phase));
}

void abi_NextPhaseUnlessKind25(void)
{
    abi_ended(NextPhaseUnlessKind25(cpu.x));
}

/*
 * @routine 0E0B:099 HalveDirectionWhenHurt
 * @run 70:B099
 * WHAT IT DOES: Unless the object is in state 6, halves its OBJ_3618 once its hits taken reach 5.
 * HOW: Two compares and a memory shift right.
 * WAITS: no.
 * Entry: X object. Exit: the last compare, or the shift's N, Z and C.
 */
Ended HalveDirectionWhenHurt(uint8_t slot)
{
    uint8_t state, hits, shifted;
    bool carry = false;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_compare(state, 6);
    if (state == 6) {
        return e;
    }
    hits = RD8(OBJ->hit_taken[slot]);
    e = ended_compare(hits, 5);
    if (hits < 5) {
        return e;
    }
    shifted = LSR8_VALUE(OBJ->f3618[slot], carry);
    return then(e, ended_with_carry(ended_nz(shifted), carry));
}

void abi_HalveDirectionWhenHurt(void)
{
    abi_ended(HalveDirectionWhenHurt(cpu.x));
}

/*
 * @routine 0E0B:10B LoadThenSpawn24
 * @run 70:B10B
 * WHAT IT DOES: Loads the object's record, sets OBJ_3618 to $80, clears the object in slot $11 and starts a kind $24 object there at this object's place.
 * HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte copied, Y $11.
 */
Ended LoadThenSpawn24(uint8_t slot)
{
    uint8_t nz, last, placed;
    bool carry;

    jsr_push(0xB10E);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->f3618[slot], 0x80);
    jsr_push(0xB118);
    ClearObjectInY(0x11);
    jsr_pop();
    placed = StartObjectAtPlace(0x24, slot, 0x11);
    return then(ended_of_record(last, nz, carry), ended_with_y(ended_load(placed), 0x11));
}

void abi_LoadThenSpawn24(void)
{
    abi_ended(LoadThenSpawn24(cpu.x));
}

/*
 * @routine 0E0B:5E8 StepListB5FEToState4
 * @run 70:B5E8
 * WHAT IT DOES: Steps the animation list at $B5FE; when it ends, the object goes to state 4 and OBJ_2800 0.
 * HOW: StepObjectFrames, then stores and an increment.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or A 4 when the list ended.
 */
Ended StepListB5FEToState4(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry;
    Ended e;

    WR8(ZP->temp10, 0xFE);
    WR8(ZP->temp11, 0xB5);
    jsr_push(0xB5F3);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->state[slot], 4);
    WR8(OBJ->f2800[slot], 0);
    return then(e, ended_load(4));
}

void abi_StepListB5FEToState4(void)
{
    abi_ended(StepListB5FEToState4(cpu.x, cpu.y));
}

/*
 * @routine 0E0B:184 FallThenStopForRaster
 * @run 70:B184
 * WHAT IT DOES: Moves the object along Y by its speed and counts OBJ_34B8 down; at zero it clears the raster effect and its argument, clears the object's Y speed and goes
 * on to the next OBJ_2800.
 * HOW: ApplyVelocityY, DEC, stores, INC.
 * WAITS: no.
 * Entry: X object. Exit: the count, or the new OBJ_2800's N and Z.
 */
Ended FallThenStopForRaster(uint8_t slot)
{
    AxisMove move;
    uint8_t left, phase;
    Ended e;

    jsr_push(0xB187);
    move = ApplyVelocityY(slot);
    jsr_pop();
    left = DEC8(OBJ->f34b8[slot]);
    e = then(ended_of_axis_move(move), ended_nz(left));
    if (left != 0) {
        return e;
    }
    WR8(ZP->raster_arg, 0);
    WR8(ZP->raster_effect, 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    phase = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(phase));
}

void abi_FallThenStopForRaster(void)
{
    abi_ended(FallThenStopForRaster(cpu.x));
}

/*
 * @routine 0E0C:128 LoadThenRiseFast
 * @run 70:B928
 * WHAT IT DOES: Loads the object's record, moves it up once by a Y speed high of $D8, then sets OBJ_3510 to $30 and the Y speed to $FA00.
 * HOW: LoadObjectRecord, ApplyVelocityY, stores.
 * WAITS: no.
 * Entry: X object. Exit: A 0, Z set.
 */
Ended LoadThenRiseFast(uint8_t slot)
{
    AxisMove move;
    uint8_t nz, last;
    bool carry;
    Ended e;

    jsr_push(0xB92B);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->vy_hi[slot], 0xD8);
    jsr_push(0xB933);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(then(e, ended_load(0xD8)), ended_of_axis_move(move));
    WR8(OBJ->f3510[slot], 0x30);
    WR8(OBJ->vy_hi[slot], 0xFA);
    WR8(OBJ->vy_lo[slot], 0);
    return then(e, ended_load(0));
}

void abi_LoadThenRiseFast(void)
{
    abi_ended(LoadThenRiseFast(cpu.x));
}

/*
 * @routine 0E0B:34D CountDownThenState6
 * @run 70:B34D
 * WHAT IT DOES: Counts OBJ_3618 down; at zero the object goes to state 6, clears OBJ_2800 and takes a new OBJ_3618 from a table at $B36B by the low two bits of OBJ_35C0.
 * HOW: DEC, stores, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: the count; at zero, the table byte as loaded (Y its index).
 */
Ended CountDownThenState6(uint8_t slot)
{
    uint8_t count, index, value;
    Ended e;

    count = RD8(OBJ->f3618[slot]);
    e = ended_load(count);
    if (count != 0) {
        count = DEC8(OBJ->f3618[slot]);
        return then(e, ended_nz(count));
    }
    WR8(OBJ->state[slot], 6);
    WR8(OBJ->f2800[slot], 0);
    index = (uint8_t)(RD8(OBJ->angle_lo[slot]) & 0x03);
    value = rd8((uint16_t)(0xB36B + index));
    WR8(OBJ->f3618[slot], value);
    return then(e, ended_with_y(ended_load(value), index));
}

void abi_CountDownThenState6(void)
{
    abi_ended(CountDownThenState6(cpu.x));
}

/*
 * @routine 0E0C:187 StepListB9B7ThenAimAtPlayer
 * @run 70:B987
 * WHAT IT DOES: Steps the animation list at $B9B7; when it ends the object takes the angle to the player as OBJ_34B8, sets its speed along it with length $0180, clears its animation step and timer, sets OBJ_3510 to $C0 and goes on to the next state.
 * HOW: StepObjectFrames, AngleToPlayer, SetVelocityFromAngle, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z (A $C0) when the list ended.
 */
Ended StepListB9B7ThenAimAtPlayer(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    AngleExit angle;
    ScaleResult scale;
    uint8_t y = y_entry, byte, state;
    Ended e;

    WR8(ZP->temp10, 0xB7);
    WR8(ZP->temp11, 0xB9);
    jsr_push(0xB992);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    jsr_push(0xB997);
    angle = AngleToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_angle(angle));
    WR8(OBJ->f34b8[slot], angle.a);
    WR8(ZP->temp00, 0x80);
    WR8(ZP->temp01, 0x01);
    byte = RD8(OBJ->f34b8[slot]);
    e = then(e, ended_load(byte));
    jsr_push(0xB9A8);
    scale = SetVelocityFromAngle(byte, slot);
    jsr_pop();
    e = then(e, ended_of_scale(scale));
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f3510[slot], 0xC0);
    state = INC8(OBJ->state[slot]);
    return then(then(e, ended_load(0xC0)), ended_nz(state));
}

void abi_StepListB9B7ThenAimAtPlayer(void)
{
    abi_ended(StepListB9B7ThenAimAtPlayer(cpu.x, cpu.y));
}

/*
 * @routine 0E0B:237 StepListB25CThenStartList
 * @run 70:B237
 * WHAT IT DOES: Steps the animation list at $B25C; when it ends it starts the script list $82 on channel 5, sets bit 2 of zField8E, sets OBJ_3250 to 2 and goes on to the next state.
 * HOW: StepObjectFrames, StartListChannelKind2, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z when the list ended.
 */
Ended StepListB25CThenStartList(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry, flags, state;
    Ended e;

    WR8(ZP->temp10, 0x5C);
    WR8(ZP->temp11, 0xB2);
    jsr_push(0xB242);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    push8(slot);
    jsr_push(0xB24C);
    StartListChannelKind2(0x82, 5);
    jsr_pop();
    slot = pull8();
    flags = RD8(ZP->field8e);
    WR8(ZP->field8e, (uint8_t)(flags | 0x04));
    WR8(OBJ->hitbox_on[slot], 2);
    state = INC8(OBJ->state[slot]);
    e = then(e, ended_load((uint8_t)(flags | 0x04)));
    return then(then(e, ended_load(2)), ended_nz(state));
}

void abi_StepListB25CThenStartList(void)
{
    abi_ended(StepListB25CThenStartList(cpu.x, cpu.y));
}

/*
 * @routine 0E0C:201 TurnTowardPlayerOrStepList
 * @run 70:BA01
 * WHAT IT DOES: While OBJ_3568 counts down the object waits, turning round when it reaches zero. At zero it compares its facing with the player's side: when they differ it sets OBJ_3568 to 5 and frame $2C, otherwise it steps the animation list at $B9FA.
 * HOW: DEC, EOR, SideOfPlayer, StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last instruction's registers.
 */
Ended TurnTowardPlayerOrStepList(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    SideResult side;
    uint8_t count, facing, bits, y = y_entry;
    Ended e;

    count = RD8(OBJ->f3568[slot]);
    e = ended_load(count);
    if (count != 0) {
        count = DEC8(OBJ->f3568[slot]);
        e = then(e, ended_nz(count));
        if (count != 0) {
            return e;
        }
        facing = (uint8_t)(RD8(OBJ->facing_left[slot]) ^ 0x01);
        WR8(OBJ->facing_left[slot], facing);
        return then(e, ended_load(facing));
    }
    jsr_push(0xBA17);
    side = SideOfPlayer(slot);
    jsr_pop();
    bits = (uint8_t)(side.sides & 0x01);
    e = then(then(e, ended_of_side(side)), ended_load(bits));
    facing = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_compare(bits, facing));
    if (bits != facing) {
        WR8(OBJ->f3568[slot], 5);
        WR8(OBJ->frame[slot], 0x2C);
        return then(e, ended_load(0x2C));
    }
    WR8(ZP->temp10, 0xFA);
    WR8(ZP->temp11, 0xB9);
    jsr_push(0xBA34);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    return then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
}

void abi_TurnTowardPlayerOrStepList(void)
{
    abi_ended(TurnTowardPlayerOrStepList(cpu.x, cpu.y));
}

/*
 * @routine 0E0B:0C2 SetHitBoxFromTable
 * @run 70:B0C2
 * WHAT IT DOES: When OBJ_3250 is not zero, takes the object's hit box (four bytes: OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38) from the table at $BEEC, four bytes a row, the row being the frame less one.
 * HOW: A pointer built in zTemp00-01 by two adds of twice the row, four reads through it.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte read, Y 3; or the test of OBJ_3250.
 */
Ended SetHitBoxFromTable(uint8_t slot)
{
    Sum row, sum;
    uint8_t on, twice, y, value, low;
    bool carry;
    Ended e;

    on = RD8(OBJ->hitbox_on[slot]);
    e = ended_load(on);
    if (on == 0) {
        return e;
    }
    WR8(ZP->temp00, 0xEC);
    WR8(ZP->temp01, 0xBE);
    row = sub_bytes(RD8(OBJ->frame[slot]), 1, true);
    twice = (uint8_t)(row.value << 1);
    WR8(ZP->temp02, twice);
    for (y = 0; y < 2; y++) {
        low = RD8(ZP->temp00);
        sum = add_bytes(low, RD8(ZP->temp02), false);
        WR8(ZP->temp00, sum.value);
        carry = sum.carry;
        low = RD8(ZP->temp01);
        sum = add_bytes(low, 0, carry);
        WR8(ZP->temp01, sum.value);
    }
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 0));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 1));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 2));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 3));
    WR8(OBJ->hitbox_half_h[slot], value);
    return then(ended_of_sum(sum), ended_with_y(ended_load(value), 3));
}

void abi_SetHitBoxFromTable(void)
{
    abi_ended(SetHitBoxFromTable(cpu.x));
}

/*
 * @routine 0E0C:412 CopyPlaceAndHitBoxOfSlot10
 * @run 70:BC12
 * WHAT IT DOES: In state 0 loads the object's record. Later it takes the place of the object in slot $10 and its facing; if that object's OBJ_3250 is not zero it takes the hit box (four bytes a row from the table at $C024, the row being that object's frame less one; the half width plus 8), else it clears its half width and half height.
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, a pointer built in zTemp00-01, four reads through it.
 * WAITS: no.
 * Entry: X object. Exit: see the code: the last read or the clears.
 */
Ended CopyPlaceAndHitBoxOfSlot10(uint8_t slot)
{
    Sum row, sum;
    uint8_t state, nz, last, on, twice, y, value, low;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return then(ended_load(state), ended_of_record(last, nz, carry));
    }
    jsr_push(0xBC1F);
    last = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    e = then(then(ended_load(state), ended_with_y(ended_load(last), 0x10)), ended_nz(last));
    value = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], value);
    e = then(e, ended_load(value));
    on = RD8(OBJ->hitbox_on[0x10]);
    e = then(e, ended_load(on));
    if (on == 0) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return e;
    }
    WR8(ZP->temp00, 0x24);
    WR8(ZP->temp01, 0xC0);
    row = sub_bytes(RD8(OBJ->frame[0x10]), 1, true);
    twice = (uint8_t)(row.value << 1);
    WR8(ZP->temp02, twice);
    for (y = 0; y < 2; y++) {
        low = RD8(ZP->temp00);
        sum = add_bytes(low, RD8(ZP->temp02), false);
        WR8(ZP->temp00, sum.value);
        carry = sum.carry;
        low = RD8(ZP->temp01);
        sum = add_bytes(low, 0, carry);
        WR8(ZP->temp01, sum.value);
    }
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 0));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 1));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 2));
    sum = add_bytes(value, 0x08, false);
    WR8(OBJ->hitbox_half_w[slot], sum.value);
    value = rd8((uint16_t)(RD16_LH2(ZP->temp00, ZP->temp01) + 3));
    WR8(OBJ->hitbox_half_h[slot], value);
    return then(then(e, ended_of_sum(sum)), ended_with_y(ended_load(value), 3));
}

void abi_CopyPlaceAndHitBoxOfSlot10(void)
{
    abi_ended(CopyPlaceAndHitBoxOfSlot10(cpu.x));
}

/*
 * @routine 0E0B:19A StepListB1BAThenSound
 * @run 70:B19A
 * WHAT IT DOES: Steps the animation list at $B1BA; when it ends it sends sound $46, sets sprite set 4 and frame 1 and goes on to the next OBJ_2800.
 * HOW: StepObjectFrames, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
SoundEnd StepListB1BAThenSound(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    FrameStep step;
    uint8_t y = y_entry, phase;
    Ended e;

    WR8(ZP->temp10, 0xBA);
    WR8(ZP->temp11, 0xB1);
    jsr_push(0xB1A5);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        r.end = e;
        return r;
    }
    r.before = then(e, ended_load(0x46));
    r.sounded = true;
    r.command = 0x46;
    jsr_push(0xB1AC);
    r.call = SoundCommand(0x46, slot, y);
    jsr_pop();
    WR8(OBJ->sprite_set[slot], 4);
    WR8(OBJ->frame[slot], 1);
    phase = INC8(OBJ->f2800[slot]);
    r.end = then(ended_load(1), ended_nz(phase));
    return r;
}

void abi_StepListB1BAThenSound(void)
{
    abi_sound_end(StepListB1BAThenSound(cpu.x, cpu.y));
}
