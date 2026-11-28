/* The routines of object/stage04_objects_d.c. */
#ifndef RONDO_STAGE04_OBJECTS_D_H
#define RONDO_STAGE04_OBJECTS_D_H

#include <stdint.h>

void SwingAroundNextAndSound07DD(uint8_t slot);
void abi_SwingAroundNextAndSound07DD(void);
void CountHitsThenByState07DD(uint8_t slot);
void abi_CountHitsThenByState07DD(void);
void HurtTimeByHit07DD(uint8_t slot);
void abi_HurtTimeByHit07DD(void);
void DropWithoutBase07DD(uint8_t slot);
void abi_DropWithoutBase07DD(void);
void TailByBodyState07DD(uint8_t slot);
void abi_TailByBodyState07DD(void);
void NextStateAfterWait30_07DD(uint8_t slot);
void abi_NextStateAfterWait30_07DD(void);
void BurningWaterAtCounts07DD(uint8_t slot);
void abi_BurningWaterAtCounts07DD(void);
void ThrowEmberOfRow07DD(uint8_t slot);
void abi_ThrowEmberOfRow07DD(void);
void SpawnWhenSlotsFree07DD(uint8_t slot);
void abi_SpawnWhenSlotsFree07DD(void);
void FindEmptyOf30to34_07DD(uint8_t slot);
void abi_FindEmptyOf30to34_07DD(void);
void ByState07DAC45D(uint8_t slot);
void abi_ByState07DAC45D(void);
void EnterFromRandomSide07DD(uint8_t slot);
void abi_EnterFromRandomSide07DD(void);
void WalkOnLanding07DD(uint8_t slot);
void abi_WalkOnLanding07DD(void);
void WalkUntilNearSlot4D07DD(uint8_t slot);
void abi_WalkUntilNearSlot4D07DD(void);

#endif
