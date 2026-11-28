/*
 * An object flung from another (its source, in the slot OBJ_FLING_SOURCE holds), as stages $09 to $10 and the secret
 * stages hold it: while the source is still its kind, the object is placed on it and thrown at random speeds, waits,
 * plays its frames as it flies, and is thrown again. Every overlay holds its own copy at its own address, so each
 * routine is one body here taking the copy's run address, from which its tables and return addresses follow.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "fling_b.h"

/* The states' table lies this far on from the object's dispatcher, and the frame list this far on from its flight. */
enum { FLING_TABLE_AT = 0x1F, FLING_SPEEDS_X_AT = 0x58, FLING_SPEEDS_Y_AT = 0x60, FLING_LIST_AT = 0x16 };

/* The object lives while the slot it was flung from holds its source's kind; then its state's handler is jumped to. */
static Jump fling_dispatch(uint8_t slot, uint8_t kind, uint16_t at)
{
    JumpTableLayout layout = { (uint16_t)(at + FLING_TABLE_AT) };
    Jump jump = { { 0 }, 0, false };
    Ended compared;
    uint8_t source, held;

    source = RD8(OBJ->fling_source[slot]);
    held = RD8(OBJ->kind[source]);
    compared = ended_with_y(ended_then(ended_load(held), ended_compare(held, kind)), source);
    if (held != kind) {
        ClearObject(slot);
        jump.end = compared;
        return jump;
    }
    jump = JumpByStateTableBody(slot, &layout);
    jump.end = ended_then(compared, jump.end);
    return jump;
}

/* Placed on its source, nudged by a random step, then given a random wait and random speeds. */
static Ended fling_launch(uint8_t slot, uint16_t at)
{
    RandomResult random;
    uint8_t source, index, low, high, state;

    source = RD8(OBJ->fling_source[slot]);
    jsr_push((uint16_t)(at + 6));
    (void)CopyObjectPlaceBack(source, slot);
    jsr_pop();
    jsr_push((uint16_t)(at + 9));
    random = NextRandom();
    jsr_pop();
    WR8(OBJ->vx_hi[slot], (uint8_t)((random.value & 0x1F) - 0x10));
    jsr_push((uint16_t)(at + 0x14));
    random = NextRandom();
    jsr_pop();
    WR8(OBJ->vy_hi[slot], (uint8_t)((random.value & 0x1F) - 0x18));
    jsr_push((uint16_t)(at + 0x1F));
    (void)ApplyVelocity(slot);
    jsr_pop();
    jsr_push((uint16_t)(at + 0x22));
    random = NextRandom();
    jsr_pop();
    WR8(OBJ->fling_wait[slot], (uint8_t)((random.value & 3) + 1));
    jsr_push((uint16_t)(at + 0x2B));
    random = NextRandom();
    jsr_pop();
    index = (uint8_t)((random.value & 3) << 1);
    high = rd8((uint16_t)(at + FLING_SPEEDS_X_AT + 1 + index));
    WR8(OBJ->vx_hi[slot], high);
    low = rd8((uint16_t)(at + FLING_SPEEDS_X_AT + index));
    WR8(OBJ->vx_lo[slot], low);
    jsr_push((uint16_t)(at + 0x3E));
    random = NextRandom();
    jsr_pop();
    index = (uint8_t)((random.value & 3) << 1);
    high = rd8((uint16_t)(at + FLING_SPEEDS_Y_AT + 1 + index));
    WR8(OBJ->vy_hi[slot], high);
    low = rd8((uint16_t)(at + FLING_SPEEDS_Y_AT + index));
    WR8(OBJ->vy_lo[slot], low);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    state = INC8(OBJ->state[slot]);
    return ended_with_overflow(
        ended_with_carry(ended_with_y(ended_with_a(ended_nz(state), low), index), false), random.overflow);
}

/* Moves on, steps its frames, and when the list ends goes back to the launch (state 1). */
static Ended fling_fly(uint8_t slot, uint16_t at)
{
    uint16_t list = (uint16_t)(at + FLING_LIST_AT);
    AxisMove move;
    FrameStep step;
    Ended e;
    uint8_t y = 0;

    jsr_push((uint16_t)(at + 3));
    move = ApplyVelocity(slot);
    jsr_pop();
    WR8(ZP->temp11, (uint8_t)(list >> 8));
    WR8(ZP->temp10, (uint8_t)list);
    jsr_push((uint16_t)(at + 0x0E));
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    e = ended_then(ended_of_axis_move(move), ended_of_frame_step(step, step != FRAME_HELD, y));
    if (step == FRAME_LIST_ENDED) {
        WR8(OBJ->state[slot], 1);
        e = ended_then(e, ended_load(1));
    }
    return e;
}

/*
 * @routine 05CB:0B9 FlungObjectWait
 * @run 70:B0B9
 * WHAT IT DOES: A flung object waits: its wait (OBJ_FLING_WAIT) is counted down, and at zero it goes to its next state.
 * HOW: DEC, and INC of the state when it reaches zero. The same nine bytes follow every copy of the launch.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the wait left, or from the new state; A, X, Y, C and V kept.
 */
Ended FlungObjectWait(uint8_t slot)
{
    uint8_t left = DEC8(OBJ->fling_wait[slot]);

    if (left != 0) {
        return ended_nz(left);
    }
    return ended_nz(INC8(OBJ->state[slot]));
}

void abi_FlungObjectWait(void)
{
    abi_ended(FlungObjectWait(cpu.x));
}

/*
 * @routine 05CB:02A FlungObject05CB
 * @run 70:B02A
 * WHAT IT DOES: An object flung from another (Secret stage 02): cleared unless the slot it was flung from holds
 * kind $22; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $B049; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject05CB(uint8_t slot)
{
    return fling_dispatch(slot, 0x22, 0xB02A);
}

void abi_FlungObject05CB(void)
{
    Jump jump = FlungObject05CB(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 072B:05A FlungObject072B
 * @run 70:B05A
 * WHAT IT DOES: An object flung from another (Secret stage 03): cleared unless the slot it was flung from holds
 * kind $22; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $B079; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject072B(uint8_t slot)
{
    return fling_dispatch(slot, 0x22, 0xB05A);
}

void abi_FlungObject072B(void)
{
    Jump jump = FlungObject072B(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 088D:166 FlungObject088D
 * @run 71:C166
 * WHAT IT DOES: An object flung from another (Secret stage 04): cleared unless the slot it was flung from holds
 * kind $2D; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $C185; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject088D(uint8_t slot)
{
    return fling_dispatch(slot, 0x2D, 0xC166);
}

void abi_FlungObject088D(void)
{
    Jump jump = FlungObject088D(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 09EC:6AF FlungObject09EC
 * @run 70:BEAF
 * WHAT IT DOES: An object flung from another (Secret stage 05): cleared unless the slot it was flung from holds
 * kind $31; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $BECE; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject09EC(uint8_t slot)
{
    return fling_dispatch(slot, 0x31, 0xBEAF);
}

void abi_FlungObject09EC(void)
{
    Jump jump = FlungObject09EC(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 0B4B:2B5 FlungObject0B4B
 * @run 70:B2B5
 * WHAT IT DOES: An object flung from another (Secret stage 06): cleared unless the slot it was flung from holds
 * kind $21; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $B2D4; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject0B4B(uint8_t slot)
{
    return fling_dispatch(slot, 0x21, 0xB2B5);
}

void abi_FlungObject0B4B(void)
{
    Jump jump = FlungObject0B4B(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 0CAB:076 FlungObject0CAB
 * @run 70:B076
 * WHAT IT DOES: An object flung from another (Secret stage 07): cleared unless the slot it was flung from holds
 * kind $2E; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $B095; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject0CAB(uint8_t slot)
{
    return fling_dispatch(slot, 0x2E, 0xB076);
}

void abi_FlungObject0CAB(void)
{
    Jump jump = FlungObject0CAB(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 0E0A:63A FlungObject0E0A
 * @run 70:AE3A
 * WHAT IT DOES: An object flung from another (Secret stage 08): cleared unless the slot it was flung from holds
 * kind $21; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $AE59; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject0E0A(uint8_t slot)
{
    return fling_dispatch(slot, 0x21, 0xAE3A);
}

void abi_FlungObject0E0A(void)
{
    Jump jump = FlungObject0E0A(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 0F6A:766 FlungObject0F6A
 * @run 70:AF66
 * WHAT IT DOES: An object flung from another (Secret stage 09): cleared unless the slot it was flung from holds
 * kind $22; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $AF85; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject0F6A(uint8_t slot)
{
    return fling_dispatch(slot, 0x22, 0xAF66);
}

void abi_FlungObject0F6A(void)
{
    Jump jump = FlungObject0F6A(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 10CB:0C2 FlungObject10CB
 * @run 70:B0C2
 * WHAT IT DOES: An object flung from another (Secret stage 0A): cleared unless the slot it was flung from holds
 * kind $29; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $B0E1; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject10CB(uint8_t slot)
{
    return fling_dispatch(slot, 0x29, 0xB0C2);
}

void abi_FlungObject10CB(void)
{
    Jump jump = FlungObject10CB(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 13FB:084 FlungObject13FB
 * @run 70:B084
 * WHAT IT DOES: An object flung from another (Secret stage 0C): cleared unless the slot it was flung from holds
 * kind $43; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $B0A3; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject13FB(uint8_t slot)
{
    return fling_dispatch(slot, 0x43, 0xB084);
}

void abi_FlungObject13FB(void)
{
    Jump jump = FlungObject13FB(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 16CB:092 FlungObject16CB
 * @run 70:B092
 * WHAT IT DOES: An object flung from another (Secret stage 0E): cleared unless the slot it was flung from holds
 * kind $29; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $B0B1; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject16CB(uint8_t slot)
{
    return fling_dispatch(slot, 0x29, 0xB092);
}

void abi_FlungObject16CB(void)
{
    Jump jump = FlungObject16CB(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 18DB:09A FlungObject18DB
 * @run 70:B09A
 * WHAT IT DOES: An object flung from another (Stage 10): cleared unless the slot it was flung from holds
 * kind $30; else its state's handler runs (load its record, launch, wait, fly).
 * HOW: One compare, then JumpByStateTableBody with the table at $B0B9; ClearObject through its vector by a tail JMP.
 * WAITS: no.
 * Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
 * A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump FlungObject18DB(uint8_t slot)
{
    return fling_dispatch(slot, 0x30, 0xB09A);
}

void abi_FlungObject18DB(void)
{
    Jump jump = FlungObject18DB(cpu.x);

    if (jump.jumps) {
        jump_shim(jump.end, jump.target);
    } else {
        abi_ended(jump.end);
    }
}

/*
 * @routine 05CB:051 FlungObjectLaunch05CB
 * @run 70:B051
 * WHAT IT DOES: Launches a flung object (Secret stage 02): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B0A9 and $B0B1.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch05CB(uint8_t slot)
{
    return fling_launch(slot, 0xB051);
}

void abi_FlungObjectLaunch05CB(void)
{
    abi_ended(FlungObjectLaunch05CB(cpu.x));
}

/*
 * @routine 09EC:6D6 FlungObjectLaunch09EC
 * @run 70:BED6
 * WHAT IT DOES: Launches a flung object (Secret stage 05): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $BF2E and $BF36.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch09EC(uint8_t slot)
{
    return fling_launch(slot, 0xBED6);
}

void abi_FlungObjectLaunch09EC(void)
{
    abi_ended(FlungObjectLaunch09EC(cpu.x));
}

/*
 * @routine 0B4B:2DC FlungObjectLaunch0B4B
 * @run 70:B2DC
 * WHAT IT DOES: Launches a flung object (Secret stage 06): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B334 and $B33C.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch0B4B(uint8_t slot)
{
    return fling_launch(slot, 0xB2DC);
}

void abi_FlungObjectLaunch0B4B(void)
{
    abi_ended(FlungObjectLaunch0B4B(cpu.x));
}

/*
 * @routine 0CAB:09D FlungObjectLaunch0CAB
 * @run 70:B09D
 * WHAT IT DOES: Launches a flung object (Secret stage 07): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B0F5 and $B0FD.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch0CAB(uint8_t slot)
{
    return fling_launch(slot, 0xB09D);
}

void abi_FlungObjectLaunch0CAB(void)
{
    abi_ended(FlungObjectLaunch0CAB(cpu.x));
}

/*
 * @routine 0E0A:661 FlungObjectLaunch0E0A
 * @run 70:AE61
 * WHAT IT DOES: Launches a flung object (Secret stage 08): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $AEB9 and $AEC1.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch0E0A(uint8_t slot)
{
    return fling_launch(slot, 0xAE61);
}

void abi_FlungObjectLaunch0E0A(void)
{
    abi_ended(FlungObjectLaunch0E0A(cpu.x));
}

/*
 * @routine 0F6A:78D FlungObjectLaunch0F6A
 * @run 70:AF8D
 * WHAT IT DOES: Launches a flung object (Secret stage 09): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $AFE5 and $AFED.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch0F6A(uint8_t slot)
{
    return fling_launch(slot, 0xAF8D);
}

void abi_FlungObjectLaunch0F6A(void)
{
    abi_ended(FlungObjectLaunch0F6A(cpu.x));
}

/*
 * @routine 10CB:0E9 FlungObjectLaunch10CB
 * @run 70:B0E9
 * WHAT IT DOES: Launches a flung object (Secret stage 0A): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B141 and $B149.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch10CB(uint8_t slot)
{
    return fling_launch(slot, 0xB0E9);
}

void abi_FlungObjectLaunch10CB(void)
{
    abi_ended(FlungObjectLaunch10CB(cpu.x));
}

/*
 * @routine 13FB:0AB FlungObjectLaunch13FB
 * @run 70:B0AB
 * WHAT IT DOES: Launches a flung object (Secret stage 0C): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B103 and $B10B.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch13FB(uint8_t slot)
{
    return fling_launch(slot, 0xB0AB);
}

void abi_FlungObjectLaunch13FB(void)
{
    abi_ended(FlungObjectLaunch13FB(cpu.x));
}

/*
 * @routine 16CB:0B9 FlungObjectLaunch16CB
 * @run 70:B0B9
 * WHAT IT DOES: Launches a flung object (Secret stage 0E): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B111 and $B119.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch16CB(uint8_t slot)
{
    return fling_launch(slot, 0xB0B9);
}

void abi_FlungObjectLaunch16CB(void)
{
    abi_ended(FlungObjectLaunch16CB(cpu.x));
}

/*
 * @routine 18DB:0C1 FlungObjectLaunch18DB
 * @run 70:B0C1
 * WHAT IT DOES: Launches a flung object (Stage 10): placed on its source and moved a random step from it,
 * given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
 * HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B119 and $B121.
 * WAITS: no.
 * Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
 * new state.
 */
Ended FlungObjectLaunch18DB(uint8_t slot)
{
    return fling_launch(slot, 0xB0C1);
}

void abi_FlungObjectLaunch18DB(void)
{
    abi_ended(FlungObjectLaunch18DB(cpu.x));
}

/*
 * @routine 05CB:0C2 FlungObjectFly05CB
 * @run 70:B0C2
 * WHAT IT DOES: A flung object flies (Secret stage 02): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $B0D8 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly05CB(uint8_t slot)
{
    return fling_fly(slot, 0xB0C2);
}

void abi_FlungObjectFly05CB(void)
{
    abi_ended(FlungObjectFly05CB(cpu.x));
}

/*
 * @routine 072B:0FA FlungObjectFly072B
 * @run 70:B0FA
 * WHAT IT DOES: A flung object flies (Secret stage 03): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $B110 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly072B(uint8_t slot)
{
    return fling_fly(slot, 0xB0FA);
}

void abi_FlungObjectFly072B(void)
{
    abi_ended(FlungObjectFly072B(cpu.x));
}

/*
 * @routine 088D:200 FlungObjectFly088D
 * @run 71:C200
 * WHAT IT DOES: A flung object flies (Secret stage 04): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $C216 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly088D(uint8_t slot)
{
    return fling_fly(slot, 0xC200);
}

void abi_FlungObjectFly088D(void)
{
    abi_ended(FlungObjectFly088D(cpu.x));
}

/*
 * @routine 09EC:747 FlungObjectFly09EC
 * @run 70:BF47
 * WHAT IT DOES: A flung object flies (Secret stage 05): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $BF5D through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly09EC(uint8_t slot)
{
    return fling_fly(slot, 0xBF47);
}

void abi_FlungObjectFly09EC(void)
{
    abi_ended(FlungObjectFly09EC(cpu.x));
}

/*
 * @routine 0B4B:34D FlungObjectFly0B4B
 * @run 70:B34D
 * WHAT IT DOES: A flung object flies (Secret stage 06): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $B363 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly0B4B(uint8_t slot)
{
    return fling_fly(slot, 0xB34D);
}

void abi_FlungObjectFly0B4B(void)
{
    abi_ended(FlungObjectFly0B4B(cpu.x));
}

/*
 * @routine 0CAB:10E FlungObjectFly0CAB
 * @run 70:B10E
 * WHAT IT DOES: A flung object flies (Secret stage 07): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $B124 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly0CAB(uint8_t slot)
{
    return fling_fly(slot, 0xB10E);
}

void abi_FlungObjectFly0CAB(void)
{
    abi_ended(FlungObjectFly0CAB(cpu.x));
}

/*
 * @routine 0E0A:6D2 FlungObjectFly0E0A
 * @run 70:AED2
 * WHAT IT DOES: A flung object flies (Secret stage 08): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $AEE8 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly0E0A(uint8_t slot)
{
    return fling_fly(slot, 0xAED2);
}

void abi_FlungObjectFly0E0A(void)
{
    abi_ended(FlungObjectFly0E0A(cpu.x));
}

/*
 * @routine 0F6A:7FE FlungObjectFly0F6A
 * @run 70:AFFE
 * WHAT IT DOES: A flung object flies (Secret stage 09): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $B014 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly0F6A(uint8_t slot)
{
    return fling_fly(slot, 0xAFFE);
}

void abi_FlungObjectFly0F6A(void)
{
    abi_ended(FlungObjectFly0F6A(cpu.x));
}

/*
 * @routine 10CB:15A FlungObjectFly10CB
 * @run 70:B15A
 * WHAT IT DOES: A flung object flies (Secret stage 0A): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $B170 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly10CB(uint8_t slot)
{
    return fling_fly(slot, 0xB15A);
}

void abi_FlungObjectFly10CB(void)
{
    abi_ended(FlungObjectFly10CB(cpu.x));
}

/*
 * @routine 13FB:11C FlungObjectFly13FB
 * @run 70:B11C
 * WHAT IT DOES: A flung object flies (Secret stage 0C): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $B132 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly13FB(uint8_t slot)
{
    return fling_fly(slot, 0xB11C);
}

void abi_FlungObjectFly13FB(void)
{
    abi_ended(FlungObjectFly13FB(cpu.x));
}

/*
 * @routine 16CB:12A FlungObjectFly16CB
 * @run 70:B12A
 * WHAT IT DOES: A flung object flies (Secret stage 0E): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $B140 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly16CB(uint8_t slot)
{
    return fling_fly(slot, 0xB12A);
}

void abi_FlungObjectFly16CB(void)
{
    abi_ended(FlungObjectFly16CB(cpu.x));
}

/*
 * @routine 18DB:132 FlungObjectFly18DB
 * @run 70:B132
 * WHAT IT DOES: A flung object flies (Stage 10): it moves and plays its frames; when the list ends it is
 * launched again (state 1).
 * HOW: ApplyVelocity, then StepObjectFrames on the list at $B148 through zTemp10-11.
 * WAITS: no.
 * Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
 * StepObjectFrames leaves it, V from ApplyVelocity.
 */
Ended FlungObjectFly18DB(uint8_t slot)
{
    return fling_fly(slot, 0xB132);
}

void abi_FlungObjectFly18DB(void)
{
    abi_ended(FlungObjectFly18DB(cpu.x));
}
