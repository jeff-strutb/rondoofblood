/* The routines of object/stage0607_ram_c.c. */
#ifndef RONDO_STAGE0607_RAM_C_H
#define RONDO_STAGE0607_RAM_C_H

#include <stdint.h>

void FillSevenWithThreeThenClear0A9C(void);
void abi_FillSevenWithThreeThenClear0A9C(void);
void FillEightWithArgThenClear0BFF(uint8_t slot);
void abi_FillEightWithArgThenClear0BFF(void);
void TileGateBySaveByte0BFF(void);
void abi_TileGateBySaveByte0BFF(void);

#endif
