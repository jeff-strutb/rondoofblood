/* Census copies of VramListFromTable (vram_update_census.c): declared here, not in routines.h. */
#ifndef RONDO_VRAM_UPDATE_CENSUS_H
#define RONDO_VRAM_UPDATE_CENSUS_H

#include "vram_update.h"

VramExit VramListFromTable0441(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTable0441(void);
VramExit VramListFromTable0651(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTable0651(void);
VramExit VramListFromTable1231(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTable1231(void);
VramExit VramListFromTable1A31(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTable1A31(void);
VramExit VramListFromTable1B91(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTable1B91(void);
VramExit VramListFromTable25F7(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTable25F7(void);

#endif
