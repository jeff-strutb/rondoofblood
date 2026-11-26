/*
 * TEMPORARY: the stage object routines of lot L6 for the overlays at sectors 16CC to 16CF (enemy kinds not yet identified), by routine. Bodies return
 * what they ended on (ObjArm, enemy_objects.h); each routine's exit shim builds A, Y and the flags from it and its abi_ wrapper applies them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/* AngleToPlayer's exit: A, N, Z, C, V and Y. */
static Ended ended_of_angle(AngleExit r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.a), r.c), r.v), r.y);
}

/* The arms the routines end on. */
enum { BC5C_STEPPED, BC5C_AIMED };

/*
 * @routine 16CC:436 StepListBC5CThenAimAtPlayer
 * @run 70:BC36
 * WHAT IT DOES: Steps the animation list at $BC5C; when it ends the object sets OBJ_3250 to 2, takes the angle to the player as OBJ_3568, sets OBJ_3510 to $80 and OBJ_34B8 to 0 and goes on to the next state.
 * HOW: StepObjectFrames, AngleToPlayer, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the step, or the new state's N and Z (A 0) when the list ended.
 */
ObjArm StepListBC5CThenAimAtPlayer(uint8_t slot, uint8_t y_entry)
{
    ObjArm r = { 0 };
    uint8_t y = y_entry;

    WR8(ZP->temp10, 0x5C);
    WR8(ZP->temp11, 0xBC);
    jsr_push(0xBC41);
    r.step = StepObjectFrames(slot, &y);
    jsr_pop();
    r.y = y;
    r.arm = BC5C_STEPPED;
    if (r.step != FRAME_LIST_ENDED) {
        return r;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    jsr_push(0xBC4B);
    r.angle = AngleToPlayer(slot);
    jsr_pop();
    WR8(OBJ->f3568[slot], r.angle.a);
    WR8(OBJ->f3510[slot], 0x80);
    WR8(OBJ->f34b8[slot], 0);
    r.arm = BC5C_AIMED;
    r.first = INC8(OBJ->state[slot]);
    return r;
}

/* The step's exit; when the list ended, LDA #2, AngleToPlayer's exit, LDA #$80, LDA #0 and the state's INC over it. */
static Ended step_list_bc5c_exit_shim(ObjArm r)
{
    Ended e = ended_of_frame_step(r.step, r.step != FRAME_HELD, r.y);

    if (r.arm == BC5C_STEPPED) {
        return e;
    }
    e = ended_then(ended_then(e, ended_load(2)), ended_of_angle(r.angle));
    return ended_then(ended_then(ended_then(e, ended_load(0x80)), ended_load(0)), ended_nz(r.first));
}

void abi_StepListBC5CThenAimAtPlayer(void)
{
    abi_ended(step_list_bc5c_exit_shim(StepListBC5CThenAimAtPlayer(cpu.x, cpu.y)));
}

/*
 * @routine 16CC:40C LoadThenRandomPlace
 * @run 70:BC0C
 * WHAT IT DOES: Loads the object's record and puts it at a random place relative to its screen: X from a random $00 to $7F plus a random $00 to $3F plus $20, Y from a random $00 to $3F plus $10 (high bytes 0).
 * HOW: LoadObjectRecord, NextRandom three times, adds.
 * WAITS: no.
 * Entry: X object. Exit: A the Y low, N, Z, C and V from the last add.
 */
ObjArm LoadThenRandomPlace(uint8_t slot)
{
    ObjArm r = { 0 };
    RandomResult random;
    uint8_t place;
    bool carry;

    jsr_push(0xBC0F);
    r.first = LoadObjectRecord(slot, &r.second, &carry);
    jsr_pop();
    r.flag = carry;
    WR8(OBJ->x_hi[slot], 0);
    jsr_push(0xBC15);
    random = NextRandom();
    jsr_pop();
    place = (uint8_t)(random.value & 0x7F);
    WR8(ZP->temp00, place);
    jsr_push(0xBC1C);
    random = NextRandom();
    jsr_pop();
    place = (uint8_t)((random.value & 0x3F) + RD8(ZP->temp00));
    WR8(OBJ->x_lo[slot], (uint8_t)(place + 0x20));
    WR8(OBJ->y_hi[slot], 0);
    jsr_push(0xBC2D);
    random = NextRandom();
    jsr_pop();
    r.third = (uint8_t)(random.value & 0x3F);
    WR8(OBJ->y_lo[slot], (uint8_t)(r.third + 0x10));
    return r;
}

/* LoadObjectRecord's Y 11 stands; A, N, Z, C and V are the last add's, CLC and ADC #$10 to the third random's low six bits. */
static Ended load_then_random_place_exit_shim(ObjArm r)
{
    return ended_then(ended_of_record(r.first, r.second, r.flag), ended_of_sum(add_bytes(r.third, 0x10, false)));
}

void abi_LoadThenRandomPlace(void)
{
    abi_ended(load_then_random_place_exit_shim(LoadThenRandomPlace(cpu.x)));
}
