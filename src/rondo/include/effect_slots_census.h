/* The ending's palette effects in the census versions of its overlay (effect_slots_census.c). */
#ifndef RONDO_EFFECT_SLOTS_CENSUS_H
#define RONDO_EFFECT_SLOTS_CENSUS_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"
#include "front_end.h"

Exit EffectStep0441(uint8_t y);
void abi_EffectStep0441(void);
Exit EffectDispatch0441(uint8_t type, uint8_t slot);
void abi_EffectDispatch0441(void);
Exit EffectList0441(uint8_t slot);
void abi_EffectList0441(void);
void ClearEffectSlot0441(uint8_t slot);
void abi_ClearEffectSlot0441(void);
Exit EffectFade0441(uint8_t slot);
void abi_EffectFade0441(void);
Exit EffectShade0441(uint8_t slot);
void abi_EffectShade0441(void);
Exit EffectColour0441(uint8_t slot);
void abi_EffectColour0441(void);
Exit EffectFlash0441(uint8_t slot, uint8_t y);
void abi_EffectFlash0441(void);
Exit EffectFlashEnd0441(uint8_t slot, uint8_t y);
void abi_EffectFlashEnd0441(void);
Exit EffectStep0651(uint8_t y);
void abi_EffectStep0651(void);
Exit EffectDispatch0651(uint8_t type, uint8_t slot);
void abi_EffectDispatch0651(void);
Exit EffectList0651(uint8_t slot);
void abi_EffectList0651(void);
void ClearEffectSlot0651(uint8_t slot);
void abi_ClearEffectSlot0651(void);
Exit EffectFade0651(uint8_t slot);
void abi_EffectFade0651(void);
Exit EffectShade0651(uint8_t slot);
void abi_EffectShade0651(void);
Exit EffectColour0651(uint8_t slot);
void abi_EffectColour0651(void);
Exit EffectFlash0651(uint8_t slot, uint8_t y);
void abi_EffectFlash0651(void);
Exit EffectFlashEnd0651(uint8_t slot, uint8_t y);
void abi_EffectFlashEnd0651(void);
Exit EffectStep1231(uint8_t y);
void abi_EffectStep1231(void);
Exit EffectDispatch1231(uint8_t type, uint8_t slot);
void abi_EffectDispatch1231(void);
Exit EffectList1231(uint8_t slot);
void abi_EffectList1231(void);
void ClearEffectSlot1231(uint8_t slot);
void abi_ClearEffectSlot1231(void);
Exit EffectFade1231(uint8_t slot);
void abi_EffectFade1231(void);
Exit EffectShade1231(uint8_t slot);
void abi_EffectShade1231(void);
Exit EffectColour1231(uint8_t slot);
void abi_EffectColour1231(void);
Exit EffectFlash1231(uint8_t slot, uint8_t y);
void abi_EffectFlash1231(void);
Exit EffectFlashEnd1231(uint8_t slot, uint8_t y);
void abi_EffectFlashEnd1231(void);

#endif
