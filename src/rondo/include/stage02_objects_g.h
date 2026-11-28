/* The routines of object/stage02_objects_g.c. */
#ifndef RONDO_STAGE02_OBJECTS_G_H
#define RONDO_STAGE02_OBJECTS_G_H

#include <stdint.h>

void ChainHeadByState051B(uint8_t slot);
void abi_ChainHeadByState051B(void);
void SetUpFlyingHead051B(uint8_t slot);
void abi_SetUpFlyingHead051B(void);
void AimFlyingHead051B(uint8_t slot);
void abi_AimFlyingHead051B(void);
void PlaceFlyingBody051B(uint8_t slot);
void abi_PlaceFlyingBody051B(void);
void FlyingHeadParts051B(uint8_t slot);
void abi_FlyingHeadParts051B(void);
void FlyingHeadSwell051B(uint8_t slot);
void abi_FlyingHeadSwell051B(void);
void FlyingHeadShrink051B(uint8_t slot);
void abi_FlyingHeadShrink051B(void);
void DripFromBody051B(uint8_t slot);
void abi_DripFromBody051B(void);
void KillHeadWhenBodyHit051B(uint8_t slot);
void abi_KillHeadWhenBodyHit051B(void);
void MoveWithScroll051B(uint8_t slot);
void abi_MoveWithScroll051B(void);
void FlyingHeadFall051B(uint8_t slot);
void abi_FlyingHeadFall051B(void);
void SplashBesideBody051B(uint8_t slot);
void abi_SplashBesideBody051B(void);
void ClearWhenSlotBelowGone051B(uint8_t slot);
void abi_ClearWhenSlotBelowGone051B(void);
void RisingBubblePicture051B(uint8_t slot);
void abi_RisingBubblePicture051B(void);
void StepBubbleList051B(uint8_t slot);
void abi_StepBubbleList051B(void);
void RiseAndClearAtListEnd051B(uint8_t slot);
void abi_RiseAndClearAtListEnd051B(void);

#endif
