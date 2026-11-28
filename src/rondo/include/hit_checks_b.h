/* The routines of object/hit_checks_b.c. */
#ifndef RONDO_HIT_CHECKS_B_H
#define RONDO_HIT_CHECKS_B_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"

void HitToState4Frame0EBD(uint8_t slot);
void abi_HitToState4Frame0EBD(void);
void HitToState2Frame101E(uint8_t slot);
void abi_HitToState2Frame101E(void);
void HitToState4_101B(uint8_t slot);
void abi_HitToState4_101B(void);
void HitToState11_101B(uint8_t slot);
void abi_HitToState11_101B(void);
void HitToState4_12DB(uint8_t slot);
void abi_HitToState4_12DB(void);
void HitSpawn92_161B(uint8_t slot);
void abi_HitSpawn92_161B(void);
void HitHold32_161B(uint8_t slot);
void abi_HitHold32_161B(void);
void MarkWhenPlayerNear0EBD(uint8_t slot);
void abi_MarkWhenPlayerNear0EBD(void);
void HitSoundToState4_101C(uint8_t slot);
void abi_HitSoundToState4_101C(void);
void HitBoxSizeByFrame101C(uint8_t slot);
void abi_HitBoxSizeByFrame101C(void);
void CycleWhileUnderState2_101C(uint8_t slot);
void abi_CycleWhileUnderState2_101C(void);
void ToState4WhenPlayerClose101D(uint8_t slot);
void abi_ToState4WhenPlayerClose101D(void);
void HitToState5_101D(uint8_t slot);
void abi_HitToState5_101D(void);
void HitBringDownSlots101C(uint8_t slot);
void abi_HitBringDownSlots101C(void);
void DropAndShake12DD(uint8_t slot);
void abi_DropAndShake12DD(void);
void HalveWhenHit12DE(uint8_t slot);
void abi_HalveWhenHit12DE(void);
void HitToState7_161C(uint8_t slot);
void abi_HitToState7_161C(void);
void HitStopAll12DD(uint8_t slot);
void abi_HitStopAll12DD(void);
void HitStopAll12DE(uint8_t slot);
void abi_HitStopAll12DE(void);
void SpawnAtRandom18DB(uint8_t slot);
void abi_SpawnAtRandom18DB(void);
void HealthByFirstHit18DB(uint8_t slot);
void abi_HealthByFirstHit18DB(void);
void StartSixAt0EBF(uint8_t slot);
void abi_StartSixAt0EBF(void);
void HitSoundAndSix0EBF(uint8_t slot);
void abi_HitSoundAndSix0EBF(void);

#endif
