/* The routines of object/stage0608_entries_c.c. */
#ifndef RONDO_STAGE0608_ENTRIES_C_H
#define RONDO_STAGE0608_ENTRIES_C_H

#include <stdint.h>

void ReloadStageEntryAndScroll0A8E(void);
void abi_ReloadStageEntryAndScroll0A8E(void);
void DebrisFlightStep0A9A(void);
void abi_DebrisFlightStep0A9A(void);
void LoadWaitAndClearWork0A9E(uint8_t slot);
void abi_LoadWaitAndClearWork0A9E(void);
void DebrisFlightStep0BFA(void);
void abi_DebrisFlightStep0BFA(void);
void FollowYWithOffset0D5C(uint8_t slot);
void abi_FollowYWithOffset0D5C(void);

#endif
