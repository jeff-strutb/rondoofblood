/* The routines of object/stage02_objects_a.c. */
#ifndef RONDO_STAGE02_OBJECTS_A_H
#define RONDO_STAGE02_OBJECTS_A_H

#include <stdint.h>

void BounceSpeeds051B(uint8_t slot);
void abi_BounceSpeeds051B(void);
void StepListThenFallOrLeave051B(uint8_t slot);
void abi_StepListThenFallOrLeave051B(void);
void CountCheckThenByState051B(uint8_t slot);
void abi_CountCheckThenByState051B(void);
void TurnListThenState2_051B(uint8_t slot);
void abi_TurnListThenState2_051B(void);
void SetHitBoxByFrame051B(uint8_t slot);
void abi_SetHitBoxByFrame051B(void);
void SetHitBoxStretched051B(uint8_t slot);
void abi_SetHitBoxStretched051B(void);
void SetHitBoxCrouched051B(uint8_t slot);
void abi_SetHitBoxCrouched051B(void);
void LandThenBySubstate051B(uint8_t slot);
void abi_LandThenBySubstate051B(void);
void ThrowOnAnimation051B(uint8_t slot);
void abi_ThrowOnAnimation051B(void);
void BySubstate051B678(uint8_t slot);
void abi_BySubstate051B678(void);
void BreakWhenCameraReaches051B(uint8_t slot);
void abi_BreakWhenCameraReaches051B(void);
void HitThenByState051B730(uint8_t slot);
void abi_HitThenByState051B730(void);
void StepListB819(uint8_t slot);
void abi_StepListB819(void);
void LandWithSpeedsB7CF(uint8_t slot);
void abi_LandWithSpeedsB7CF(void);
void StepThenCountOutOrLand051B(uint8_t slot);
void abi_StepThenCountOutOrLand051B(void);
void CountBoxHitByState051B871(uint8_t slot);
void abi_CountBoxHitByState051B871(void);
void SetHitBoxByFrame051B894(uint8_t slot);
void abi_SetHitBoxByFrame051B894(void);
void State3WhenHit051B(uint8_t slot);
void abi_State3WhenHit051B(void);
void LoadThenPartTwoSlotsOn051B(uint8_t slot);
void abi_LoadThenPartTwoSlotsOn051B(void);
void SwayThenState2_051B(uint8_t slot);
void abi_SwayThenState2_051B(void);
void BySubstate051B963(uint8_t slot);
void abi_BySubstate051B963(void);
void NextAttackFromList051B(uint8_t slot);
void abi_NextAttackFromList051B(void);
void SubstateThreeIfNear051B(uint8_t slot);
void abi_SubstateThreeIfNear051B(void);
void AnimateThenDropThree051B(uint8_t slot);
void abi_AnimateThenDropThree051B(void);

#endif
