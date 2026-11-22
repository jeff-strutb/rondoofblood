/* Lot L4: VRAM updates. Declared here, not in routines.h. */
#ifndef RONDO_VRAM_UPDATE_H
#define RONDO_VRAM_UPDATE_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"
#include "routines.h"

bool CopyTileBlockToVram(uint8_t block, bool carry_in);
void abi_CopyTileBlockToVram(void);

Sum SelectBlockTablesA(uint8_t index);
void abi_SelectBlockTablesA(void);
Sum SelectBlockTablesB(uint8_t index);
void abi_SelectBlockTablesB(void);
Sum SelectBlockTablesC(uint8_t index);
void abi_SelectBlockTablesC(void);
Sum SelectBlockTablesD(uint8_t index);
void abi_SelectBlockTablesD(void);
Sum SelectBlockTablesE(uint8_t index);
void abi_SelectBlockTablesE(void);
Sum SelectBlockTablesF(uint8_t index);
void abi_SelectBlockTablesF(void);
Sum SelectBlockTablesG(uint8_t index);
void abi_SelectBlockTablesG(void);
Sum SelectBlockTablesH(uint8_t index);
void abi_SelectBlockTablesH(void);
Sum SelectBlockTablesI(uint8_t index);
void abi_SelectBlockTablesI(void);
Sum SelectBlockTablesJ(uint8_t index);
void abi_SelectBlockTablesJ(void);
Sum SelectBlockTablesK(uint8_t index);
void abi_SelectBlockTablesK(void);
Sum SelectBlockTablesL(uint8_t index);
void abi_SelectBlockTablesL(void);
Sum SelectBlockTablesM(uint8_t index);
void abi_SelectBlockTablesM(void);
Sum SelectBlockTablesN(uint8_t index);
void abi_SelectBlockTablesN(void);
Sum SelectBlockTablesO(uint8_t index);
void abi_SelectBlockTablesO(void);
Sum SelectBlockTablesP(uint8_t index);
void abi_SelectBlockTablesP(void);

#endif
