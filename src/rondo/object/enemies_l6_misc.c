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
