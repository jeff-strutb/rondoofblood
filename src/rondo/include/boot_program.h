/*
 * The boot program (system/boot_program.c): sector 2, read by the boot block to $3000. Declared here, not in
 * routines.h.
 */
#ifndef RONDO_BOOT_PROGRAM_H
#define RONDO_BOOT_PROGRAM_H

#include "machine.h"
#include "routines.h"

/* How LoadBootBlocks ended: the op byte that ended the list (bit 7 set), the Y the last op left, and the last add
 * of the list pointer's high byte (or of the record pointer's, when the list was empty), whose C and V stand. */
typedef struct {
    uint8_t end;
    uint8_t y;
    Sum last;
} BootLoad;

void BootStart(void);
void abi_BootStart(void);
BootLoad LoadBootBlocks(uint8_t record);
void abi_LoadBootBlocks(void);

#endif
