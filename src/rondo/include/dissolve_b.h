/* The routines of object/dissolve_b.c. */
#ifndef RONDO_DISSOLVE_B_H
#define RONDO_DISSOLVE_B_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

void DissolveMask(void);
void abi_DissolveMask(void);
void DissolveWords0E0A260(void);
void abi_DissolveWords0E0A260(void);
void DissolveStep0E0B7ED(uint8_t slot);
void abi_DissolveStep0E0B7ED(void);
void DissolveWords10CA29D(void);
void abi_DissolveWords10CA29D(void);
void DissolveMask10CA272(void);
void abi_DissolveMask10CA272(void);
void DissolveStep10CC5E1(uint8_t slot);
void abi_DissolveStep10CC5E1(void);
void DissolveWords12DA365(void);
void abi_DissolveWords12DA365(void);
void DissolveMask12DA33A(void);
void abi_DissolveMask12DA33A(void);
void DissolveStep12DE65E(uint8_t slot);
void abi_DissolveStep12DE65E(void);
void DissolveWords13FA365(void);
void abi_DissolveWords13FA365(void);
void DissolveMask13FA33A(void);
void abi_DissolveMask13FA33A(void);
void DissolveStep13FC0EA(uint8_t slot);
void abi_DissolveStep13FC0EA(void);
void DissolveWords16CA260(void);
void abi_DissolveWords16CA260(void);
void DissolveStep16CB6A4(uint8_t slot);
void abi_DissolveStep16CB6A4(void);
Jump JumpByPhaseKeepingPoint072C35D(uint8_t slot);
void abi_JumpByPhaseKeepingPoint072C35D(void);
Jump JumpByPhaseKeepingPoint088B482(uint8_t slot);
void abi_JumpByPhaseKeepingPoint088B482(void);
Jump JumpByPhaseKeepingPoint0B4C4DB(uint8_t slot);
void abi_JumpByPhaseKeepingPoint0B4C4DB(void);
Jump JumpByPhaseKeepingPoint0CAC032(uint8_t slot);
void abi_JumpByPhaseKeepingPoint0CAC032(void);
Jump JumpByPhaseKeepingPoint16CB61D(uint8_t slot);
void abi_JumpByPhaseKeepingPoint16CB61D(void);

#endif
