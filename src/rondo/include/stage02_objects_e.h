/* The routines of object/stage02_objects_e.c. */
#ifndef RONDO_STAGE02_OBJECTS_E_H
#define RONDO_STAGE02_OBJECTS_E_H

#include <stdint.h>

void LoadPieceBySlot051B(uint8_t slot);
void abi_LoadPieceBySlot051B(void);
void LoadPictureFrame0C_051B(uint8_t slot);
void abi_LoadPictureFrame0C_051B(void);
void EmitBubbles051B(uint8_t slot);
void abi_EmitBubbles051B(void);
void BubbleRiseAndSway051B(uint8_t slot);
void abi_BubbleRiseAndSway051B(void);
void PieceObjectInit051B(uint8_t slot);
void abi_PieceObjectInit051B(void);
void BitOfY051B(uint8_t slot);
void abi_BitOfY051B(void);
void BreakPieceByHits051B(uint8_t slot);
void abi_BreakPieceByHits051B(void);
void NegateSpeedIfFacingLeft051B(uint8_t slot);
void abi_NegateSpeedIfFacingLeft051B(void);
void CopyFacingToY051B(uint8_t slot);
void abi_CopyFacingToY051B(void);
void PlaceAheadInY051B(uint8_t slot);
void abi_PlaceAheadInY051B(void);
void StartEightState2WhenHit051B(uint8_t slot);
void abi_StartEightState2WhenHit051B(void);
void FacePlayerByPlace051B(uint8_t slot);
void abi_FacePlayerByPlace051B(void);

#endif
