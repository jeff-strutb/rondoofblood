/* Lot L4: VRAM updates. Declared here, not in routines.h. */
#ifndef RONDO_VRAM_UPDATE_H
#define RONDO_VRAM_UPDATE_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"
#include "routines.h"

bool CopyTileBlockToVram(uint8_t block, bool carry_in);
/* With its camera module `moved` on from $A000. */
bool CopyTileBlockToVramMoved(uint8_t block, bool carry_in, uint16_t moved);
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


/* What the VRAM list routines leave: A, X, Y, C and V (N clear and Z set when the A is $FF). */
typedef struct {
    uint8_t a, x, y;
    bool carry, overflow;
    bool left; /* the call went on into original code: the registers are as that code left them */
} VramExit;

VramExit VramListFromTableA(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableA(void);
VramExit VramListFromTableB(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableB(void);
VramExit VramListFromTableC(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableC(void);
VramExit VramListFromTableD(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableD(void);
VramExit VramListFromTableE(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableE(void);
VramExit VramListFromTableF(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableF(void);
VramExit VramListFromTableG(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableG(void);
VramExit VramListFromTableH(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableH(void);
VramExit VramListFromTableI(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableI(void);
VramExit VramListFromTableJ(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableJ(void);
VramExit VramListFromTableK(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableK(void);
VramExit VramListFromTableL(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableL(void);
VramExit VramListFromTableM(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableM(void);
VramExit VramListFromTableN(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableN(void);
VramExit VramListFromTableO(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableO(void);
VramExit VramListFromTableP(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableP(void);
VramExit VramListFromTableQ(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableQ(void);
VramExit VramListFromTableR(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableR(void);
VramExit VramListFromTableS(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableS(void);
VramExit VramListFromTableT(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableT(void);
VramExit VramListFromTableU(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableU(void);
VramExit VramListFromTableV(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableV(void);
VramExit VramListFromTableW(uint8_t a, uint8_t x, bool overflow);
void abi_VramListFromTableW(void);
VramExit VramListFromIndex(uint8_t a, uint8_t x, bool negative, bool carry, bool overflow);
VramExit VramListRun(uint8_t x, uint8_t y, bool carry, bool overflow);
VramExit VramCommandsFetch(bool carry, bool overflow);
VramExit VramCommandsNext(uint8_t y, bool carry, bool overflow);
VramExit VramCommandsRepeat(uint8_t a, uint8_t y, bool carry, bool overflow);
void abi_VramListFromIndex(void);
void abi_VramListRun(void);
void abi_VramCommandsFetch(void);
void abi_VramCommandsNext(void);
void abi_VramCommandsRepeat(void);


typedef struct {
    Sum sum;
    uint8_t left;
} PaletteLoad;

PaletteLoad LoadPackedPalette(uint8_t y);
PaletteLoad LoadPackedPaletteB(uint8_t y);
PaletteLoad LoadPackedPaletteC(uint8_t y);
void abi_LoadPackedPalette(void);
void abi_LoadPackedPaletteB(void);
void abi_LoadPackedPaletteC(void);

/* The copies in the programs at 10C5 and 12D5. */
bool CopyTileBlockToVram10C5(uint8_t block, bool carry_in);
void abi_CopyTileBlockToVram10C5(void);
bool CopyTileBlockToVram12D5(uint8_t block, bool carry_in);
void abi_CopyTileBlockToVram12D5(void);

#endif
