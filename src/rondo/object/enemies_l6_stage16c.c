/*
 * TEMPORARY: the stage object routines of lot L6 for the overlays at sectors 16CC to 16CF (enemy kinds not yet identified), by routine. Bodies return how the
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

/* AngleToPlayer's exit: A, N, Z, C, V and Y. */
static Ended ended_of_angle(AngleExit r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.a), r.c), r.v), r.y);
}

/*
 * @routine 16CC:436 StepListBC5CThenAimAtPlayer
 * @run 70:BC36
 * WHAT IT DOES: Steps the animation list at $BC5C; when it ends the object sets OBJ_3250 to 2, takes the angle to the player as OBJ_3568, sets OBJ_3510 to $80 and OBJ_34B8 to 0 and goes on to the next state.
 * HOW: StepObjectFrames, AngleToPlayer, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z (A 0) when the list ended.
 */
Ended StepListBC5CThenAimAtPlayer(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    AngleExit angle;
    uint8_t y = y_entry, state;
    Ended e;

    WR8(ZP->temp10, 0x5C);
    WR8(ZP->temp11, 0xBC);
    jsr_push(0xBC41);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    jsr_push(0xBC4B);
    angle = AngleToPlayer(slot);
    jsr_pop();
    e = then(then(e, ended_load(2)), ended_of_angle(angle));
    WR8(OBJ->f3568[slot], angle.a);
    WR8(OBJ->f3510[slot], 0x80);
    WR8(OBJ->f34b8[slot], 0);
    state = INC8(OBJ->state[slot]);
    return then(then(then(e, ended_load(0x80)), ended_load(0)), ended_nz(state));
}

void abi_StepListBC5CThenAimAtPlayer(void)
{
    abi_ended(StepListBC5CThenAimAtPlayer(cpu.x, cpu.y));
}

/*
 * @routine 16CC:40C LoadThenRandomPlace
 * @run 70:BC0C
 * WHAT IT DOES: Loads the object's record and puts it at a random place relative to its screen: X from a random $00 to $7F plus a random $00 to $3F plus $20, Y from a random $00 to $3F plus $10 (high bytes 0).
 * HOW: LoadObjectRecord, NextRandom three times, adds.
 * WAITS: no.
 * Entry: X object. Exit: A the Y low, N, Z, C and V from the last add.
 */
Ended LoadThenRandomPlace(uint8_t slot)
{
    RandomResult random;
    Sum sum;
    uint8_t nz, last, first, second;
    bool carry;
    Ended e;

    jsr_push(0xBC0F);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->x_hi[slot], 0);
    jsr_push(0xBC15);
    random = NextRandom();
    jsr_pop();
    first = (uint8_t)(random.value & 0x7F);
    e = then(then(e, ended_of_random(random)), ended_load(first));
    WR8(ZP->temp00, first);
    jsr_push(0xBC1C);
    random = NextRandom();
    jsr_pop();
    second = (uint8_t)(random.value & 0x3F);
    e = then(then(e, ended_of_random(random)), ended_load(second));
    sum = add_bytes(second, RD8(ZP->temp00), false);
    e = then(e, ended_of_sum(sum));
    sum = add_bytes(sum.value, 0x20, false);
    WR8(OBJ->x_lo[slot], sum.value);
    e = then(e, ended_of_sum(sum));
    WR8(OBJ->y_hi[slot], 0);
    jsr_push(0xBC2D);
    random = NextRandom();
    jsr_pop();
    second = (uint8_t)(random.value & 0x3F);
    e = then(then(e, ended_of_random(random)), ended_load(second));
    sum = add_bytes(second, 0x10, false);
    WR8(OBJ->y_lo[slot], sum.value);
    return then(e, ended_of_sum(sum));
}

void abi_LoadThenRandomPlace(void)
{
    abi_ended(LoadThenRandomPlace(cpu.x));
}
