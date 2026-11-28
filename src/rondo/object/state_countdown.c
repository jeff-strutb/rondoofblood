/*
 * Objects that wait out a timer and then move on: the handler of a state or step that counts the
 * object's timer down once a frame and, when it reaches zero, adds one to the state or step byte.
 * The stage overlays each carry their own copies with their own timer and state columns, so one
 * body serves them all.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "enemy_objects.h"

/* The timer is counted down; at zero the next byte is incremented. Returns the byte N and Z come from. */
static uint8_t countdown_then_advance(uint8_t slot, uint16_t timer, uint16_t advance)
{
    uint8_t left = RD8_IDX(timer, slot);
    uint8_t next;

    left = (uint8_t)(left - 1);
    WR8_IDX(timer, slot, left);
    if (left != 0) {
        return left;
    }
    next = RD8_IDX(advance, slot);
    next = (uint8_t)(next + 1);
    WR8_IDX(advance, slot, next);
    return next;
}

/*
 * @routine 051B:526 CountdownNextStepA
 * @run 70:B526
 * WHAT IT DOES: Waits out the object's timer (OBJ_34B8) and then moves it on: once the timer has
 * run down, the object's step byte (OBJ_SUBSTATE) goes up by one.
 * HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_SUBSTATE.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
 */
uint8_t CountdownNextStepA(uint8_t slot)
{
    return countdown_then_advance(slot, COL(OBJ->f34b8), COL(OBJ->substate));
}

void abi_CountdownNextStepA(void)
{
    abi_nz(CountdownNextStepA(cpu.x));
}

/*
 * @routine 05CB:209 CountdownNextStateA
 * @run 70:B209
 * WHAT IT DOES: Waits out the object's timer (OBJ_3510) and then moves it on: once the timer has
 * run down, the object's state byte (the index into its handler table) goes up by one.
 * HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
 */
uint8_t CountdownNextStateA(uint8_t slot)
{
    return countdown_then_advance(slot, COL(OBJ->f3510), COL(OBJ->f27a8));
}

void abi_CountdownNextStateA(void)
{
    abi_nz(CountdownNextStateA(cpu.x));
}

/*
 * @routine 07DB:732 CountdownNextStateB
 * @run 70:B732
 * WHAT IT DOES: Waits out the object's timer (OBJ_34B8) and then moves it on: once the timer has
 * run down, the object's state byte (the index into its handler table) goes up by one.
 * HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
 */
uint8_t CountdownNextStateB(uint8_t slot)
{
    return countdown_then_advance(slot, COL(OBJ->f34b8), COL(OBJ->f27a8));
}

void abi_CountdownNextStateB(void)
{
    abi_nz(CountdownNextStateB(cpu.x));
}

/*
 * @routine 093D:45E CountdownNextStepB
 * @run 71:C45E
 * WHAT IT DOES: Waits out the object's timer (OBJ_35C0) and then moves it on: once the timer has
 * run down, the object's step byte (OBJ_SUBSTATE) goes up by one.
 * HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_SUBSTATE.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
 */
uint8_t CountdownNextStepB(uint8_t slot)
{
    return countdown_then_advance(slot, COL(OBJ->f35c0), COL(OBJ->substate));
}

void abi_CountdownNextStepB(void)
{
    abi_nz(CountdownNextStepB(cpu.x));
}

/*
 * @routine 09EC:011 CountdownNextStateC
 * @run 70:B811
 * WHAT IT DOES: Waits out the object's timer (OBJ_34B8) and then moves it on: once the timer has
 * run down, the object's state byte (the index into its handler table) goes up by one.
 * HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
 */
uint8_t CountdownNextStateC(uint8_t slot)
{
    return countdown_then_advance(slot, COL(OBJ->f34b8), COL(OBJ->f27a8));
}

void abi_CountdownNextStateC(void)
{
    abi_nz(CountdownNextStateC(cpu.x));
}

/*
 * @routine 0CAC:6FA CountdownNextStateD
 * @run 70:BEFA
 * WHAT IT DOES: Waits out the object's timer (OBJ_35C0) and then moves it on: once the timer has
 * run down, the object's state byte (the index into its handler table) goes up by one.
 * HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
 */
uint8_t CountdownNextStateD(uint8_t slot)
{
    return countdown_then_advance(slot, COL(OBJ->f35c0), COL(OBJ->f27a8));
}

void abi_CountdownNextStateD(void)
{
    abi_nz(CountdownNextStateD(cpu.x));
}

/*
 * @routine 117C:101 CountdownNextStateE
 * @run 70:B901
 * WHAT IT DOES: Waits out the object's timer (OBJ_34B8) and then moves it on: once the timer has
 * run down, the object's state byte (the index into its handler table) goes up by one.
 * HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
 */
uint8_t CountdownNextStateE(uint8_t slot)
{
    return countdown_then_advance(slot, COL(OBJ->f34b8), COL(OBJ->f27a8));
}

void abi_CountdownNextStateE(void)
{
    abi_nz(CountdownNextStateE(cpu.x));
}

/*
 * @routine 117D:020 CountdownNextStepC
 * @run 71:C020
 * WHAT IT DOES: Waits out the object's timer (OBJ_35C0) and then moves it on: once the timer has
 * run down, the object's step byte (OBJ_SUBSTATE) goes up by one.
 * HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_SUBSTATE.
 * WAITS: no.
 * Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
 */
uint8_t CountdownNextStepC(uint8_t slot)
{
    return countdown_then_advance(slot, COL(OBJ->f35c0), COL(OBJ->substate));
}

void abi_CountdownNextStepC(void)
{
    abi_nz(CountdownNextStepC(cpu.x));
}
