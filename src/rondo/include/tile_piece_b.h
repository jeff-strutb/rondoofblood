/* The routines of object/tile_piece_b.c. */
#ifndef RONDO_TILE_PIECE_B_H
#define RONDO_TILE_PIECE_B_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_enemies_l7.h"

/* object/enemies_stage06_temp.c: TilePieceObject072A's body with the overlay's records, and its exit. */
PieceObjArm TilePieceObjectAt(uint8_t slot, uint16_t records);
void tile_piece_object_at_exit_shim(PieceObjArm p);

PieceObjArm TilePieceObject05CA(uint8_t slot);
void abi_TilePieceObject05CA(void);
PieceObjArm TilePieceObject088A(uint8_t slot);
void abi_TilePieceObject088A(void);
PieceObjArm TilePieceObject09EA(uint8_t slot);
void abi_TilePieceObject09EA(void);
PieceObjArm TilePieceObject0B4A(uint8_t slot);
void abi_TilePieceObject0B4A(void);
PieceObjArm TilePieceObject0CAA(uint8_t slot);
void abi_TilePieceObject0CAA(void);
PieceObjArm TilePieceObject0E0A(uint8_t slot);
void abi_TilePieceObject0E0A(void);
PieceObjArm TilePieceObject0EBA(uint8_t slot);
void abi_TilePieceObject0EBA(void);
PieceObjArm TilePieceObject0F6A(uint8_t slot);
void abi_TilePieceObject0F6A(void);
PieceObjArm TilePieceObject101A(uint8_t slot);
void abi_TilePieceObject101A(void);
PieceObjArm TilePieceObject117A(uint8_t slot);
void abi_TilePieceObject117A(void);
PieceObjArm TilePieceObject12DA(uint8_t slot);
void abi_TilePieceObject12DA(void);
PieceObjArm TilePieceObject13FA(uint8_t slot);
void abi_TilePieceObject13FA(void);
PieceObjArm TilePieceObject161A(uint8_t slot);
void abi_TilePieceObject161A(void);
PieceObjArm TilePieceObject16CA(uint8_t slot);
void abi_TilePieceObject16CA(void);
PieceObjArm TilePieceObject18DA(uint8_t slot);
void abi_TilePieceObject18DA(void);

#endif
