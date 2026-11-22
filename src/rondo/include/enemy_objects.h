/*
 * Lot L5: the stage-specific object code of the stage overlays, as typed C. A body takes the
 * object's slot and returns, as a plain value, the byte its last instruction left N and Z from;
 * its abi_ wrapper turns that into the flags the assembly leaves.
 */
#ifndef RONDO_ENEMY_OBJECTS_H
#define RONDO_ENEMY_OBJECTS_H

#include "machine.h"

/* object/state_countdown.c: count a timer down; on zero move on to the next state or step. */
uint8_t CountdownNextStepA(uint8_t slot);
uint8_t CountdownNextStateA(uint8_t slot);
uint8_t CountdownNextStateB(uint8_t slot);
uint8_t CountdownNextStepB(uint8_t slot);
uint8_t CountdownNextStateC(uint8_t slot);
uint8_t CountdownNextStateD(uint8_t slot);
uint8_t CountdownNextStateE(uint8_t slot);
uint8_t CountdownNextStepC(uint8_t slot);

#endif
