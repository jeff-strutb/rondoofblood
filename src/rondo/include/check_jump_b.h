/* The routines of object/check_jump_b.c. */
#ifndef RONDO_CHECK_JUMP_B_H
#define RONDO_CHECK_JUMP_B_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

void ToState6WhenHitBit7(uint8_t slot);
void abi_ToState6WhenHitBit7(void);
void HitBoxByFrame10CC(uint8_t slot);
void abi_HitBoxByFrame10CC(void);
void RiseWhenHitBit7_0EC0(uint8_t slot);
void abi_RiseWhenHitBit7_0EC0(void);
void HoldWhenHitBit7_101D(uint8_t slot);
void abi_HoldWhenHitBit7_101D(void);
void StartPiecesWhenHit117F(uint8_t slot);
void abi_StartPiecesWhenHit117F(void);
void HoldNearLevelX13FB(uint8_t slot);
void abi_HoldNearLevelX13FB(void);
void PlaceChain16CC(uint8_t slot);
void abi_PlaceChain16CC(void);
Jump CheckThenJump05CB382(uint8_t slot);
void abi_CheckThenJump05CB382(void);
Jump CheckThenJump09EC03C(uint8_t slot);
void abi_CheckThenJump09EC03C(void);
Jump CheckThenJump0E0C0CC(uint8_t slot);
void abi_CheckThenJump0E0C0CC(void);
Jump CheckThenJump0EBE17E(uint8_t slot);
void abi_CheckThenJump0EBE17E(void);
Jump CheckThenJump0EBE652(uint8_t slot);
void abi_CheckThenJump0EBE652(void);
Jump CheckThenJump0EBF472(uint8_t slot);
void abi_CheckThenJump0EBF472(void);
Jump CheckThenJump0EC0264(uint8_t slot);
void abi_CheckThenJump0EC0264(void);
Jump CheckThenJump101D285(uint8_t slot);
void abi_CheckThenJump101D285(void);
Jump CheckThenJump10CC10D(uint8_t slot);
void abi_CheckThenJump10CC10D(void);
Jump CheckThenJump117C0CB(uint8_t slot);
void abi_CheckThenJump117C0CB(void);
Jump CheckThenJump117D39D(uint8_t slot);
void abi_CheckThenJump117D39D(void);
Jump CheckThenJump117E7F7(uint8_t slot);
void abi_CheckThenJump117E7F7(void);
Jump CheckThenJump13FB7AF(uint8_t slot);
void abi_CheckThenJump13FB7AF(void);
Jump CheckThenJump161B7A3(uint8_t slot);
void abi_CheckThenJump161B7A3(void);
Jump CheckThenJump16CC165(uint8_t slot);
void abi_CheckThenJump16CC165(void);

#endif
