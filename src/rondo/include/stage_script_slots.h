/* The routines of system/stage_script_slots.c. */
#ifndef RONDO_STAGE_SCRIPT_SLOTS_H
#define RONDO_STAGE_SCRIPT_SLOTS_H

#include <stdint.h>

void RunScriptSlotsByPause(void);
void abi_RunScriptSlotsByPause(void);
void RunScriptSlotIfUsed(void);
void abi_RunScriptSlotIfUsed(void);
void RunScriptSlot3Flash(void);
void abi_RunScriptSlot3Flash(void);
void RunAllScriptSlots(void);
void abi_RunAllScriptSlots(void);
void RunScriptSlotByKind(void);
void abi_RunScriptSlotByKind(void);

#endif
