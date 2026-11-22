/* Lot L4: VRAM updates. Declared here, not in routines.h. */
#ifndef RONDO_VRAM_UPDATE_H
#define RONDO_VRAM_UPDATE_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"

bool CopyTileBlockToVram(uint8_t block, bool carry_in);
void abi_CopyTileBlockToVram(void);

Alu SelectBlockTablesA(uint8_t index);
void abi_SelectBlockTablesA(void);
Alu SelectBlockTablesB(uint8_t index);
void abi_SelectBlockTablesB(void);
Alu SelectBlockTablesC(uint8_t index);
void abi_SelectBlockTablesC(void);
Alu SelectBlockTablesD(uint8_t index);
void abi_SelectBlockTablesD(void);
Alu SelectBlockTablesE(uint8_t index);
void abi_SelectBlockTablesE(void);
Alu SelectBlockTablesF(uint8_t index);
void abi_SelectBlockTablesF(void);
Alu SelectBlockTablesG(uint8_t index);
void abi_SelectBlockTablesG(void);
Alu SelectBlockTablesH(uint8_t index);
void abi_SelectBlockTablesH(void);
Alu SelectBlockTablesI(uint8_t index);
void abi_SelectBlockTablesI(void);
Alu SelectBlockTablesJ(uint8_t index);
void abi_SelectBlockTablesJ(void);
Alu SelectBlockTablesK(uint8_t index);
void abi_SelectBlockTablesK(void);
Alu SelectBlockTablesL(uint8_t index);
void abi_SelectBlockTablesL(void);
Alu SelectBlockTablesM(uint8_t index);
void abi_SelectBlockTablesM(void);
Alu SelectBlockTablesN(uint8_t index);
void abi_SelectBlockTablesN(void);
Alu SelectBlockTablesO(uint8_t index);
void abi_SelectBlockTablesO(void);
Alu SelectBlockTablesP(uint8_t index);
void abi_SelectBlockTablesP(void);

#endif
