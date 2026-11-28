/* The routines of object/stage04_objects_f.c. */
#ifndef RONDO_STAGE04_OBJECTS_F_H
#define RONDO_STAGE04_OBJECTS_F_H

#include <stdint.h>

void AdvanceWhenRiderHit07DE(uint8_t slot);
void abi_AdvanceWhenRiderHit07DE(void);
void FollowCarrier07DE(uint8_t slot);
void abi_FollowCarrier07DE(void);
void RaiseSixSegments07DE(uint8_t slot);
void abi_RaiseSixSegments07DE(void);
void HomingOnPlayer07DE(uint8_t slot);
void abi_HomingOnPlayer07DE(void);
void CountThenClear07DE(uint8_t slot);
void abi_CountThenClear07DE(void);
void ByState07DEC934(uint8_t slot);
void abi_ByState07DEC934(void);
void SegmentWaitForBody07DE(uint8_t slot);
void abi_SegmentWaitForBody07DE(void);
void SegmentSwing07DE(uint8_t slot);
void abi_SegmentSwing07DE(void);
void PlaceAlongChain07DE(uint8_t slot);
void abi_PlaceAlongChain07DE(void);
void ByState07DECA31(uint8_t slot);
void abi_ByState07DECA31(void);
void FaceFrameTowardPlayer07DE(uint8_t slot);
void abi_FaceFrameTowardPlayer07DE(void);
void EmitFourThenWait07DE(uint8_t slot);
void abi_EmitFourThenWait07DE(void);
void EmitOneOfEight07DE(uint8_t slot);
void abi_EmitOneOfEight07DE(void);
void FindEmptyOf20to27_07DE(uint8_t slot);
void abi_FindEmptyOf20to27_07DE(void);
void AscendHitThenByState07DE(uint8_t slot);
void abi_AscendHitThenByState07DE(void);
void CountRiseTime07DE(uint8_t slot);
void abi_CountRiseTime07DE(void);
void DieWhenHit07DE(uint8_t slot);
void abi_DieWhenHit07DE(void);
void SetUpGhost07DE(uint8_t slot);
void abi_SetUpGhost07DE(void);
void RandomPlaceByScroll07DE(uint8_t slot);
void abi_RandomPlaceByScroll07DE(void);
void ShiftByScrollDirection07DE(uint8_t slot);
void abi_ShiftByScrollDirection07DE(void);
void SwayAndRise07DE(uint8_t slot);
void abi_SwayAndRise07DE(void);
void GhostDrift07DE(uint8_t slot);
void abi_GhostDrift07DE(void);

#endif
