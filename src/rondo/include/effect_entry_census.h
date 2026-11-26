/* The entries and channel loop of the census versions of the ending's effect overlay (effect_entry_census.c). */
#ifndef RONDO_EFFECT_ENTRY_CENSUS_H
#define RONDO_EFFECT_ENTRY_CENSUS_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"
#include "front_end.h"
#include "vram_update.h"
#include "list_channels_census.h"

ChannelExit ListEntry0441(uint8_t y, bool c, bool v);
void abi_ListEntry0441(void);
Exit EffectEntry0441(uint8_t y);
void abi_EffectEntry0441(void);
VramExit VramEntry0441(uint8_t x, uint8_t y, bool carry, bool overflow);
void abi_VramEntry0441(void);
ChannelExit RunAllListChannels0441(uint8_t y, bool c, bool v);
void abi_RunAllListChannels0441(void);
ChannelExit ListEntry0651(uint8_t y, bool c, bool v);
void abi_ListEntry0651(void);
Exit EffectEntry0651(uint8_t y);
void abi_EffectEntry0651(void);
VramExit VramEntry0651(uint8_t x, uint8_t y, bool carry, bool overflow);
void abi_VramEntry0651(void);
ChannelExit RunAllListChannels0651(uint8_t y, bool c, bool v);
void abi_RunAllListChannels0651(void);
ChannelExit ListEntry1231(uint8_t y, bool c, bool v);
void abi_ListEntry1231(void);
Exit EffectEntry1231(uint8_t y);
void abi_EffectEntry1231(void);
VramExit VramEntry1231(uint8_t x, uint8_t y, bool carry, bool overflow);
void abi_VramEntry1231(void);
ChannelExit RunAllListChannels1231(uint8_t y, bool c, bool v);
void abi_RunAllListChannels1231(void);
ChannelExit ListEntry1A31(uint8_t y, bool c, bool v);
void abi_ListEntry1A31(void);
Exit EffectEntry1A31(uint8_t y);
void abi_EffectEntry1A31(void);
VramExit VramEntry1A31(uint8_t x, uint8_t y, bool carry, bool overflow);
void abi_VramEntry1A31(void);
ChannelExit RunAllListChannels1A31(uint8_t y, bool c, bool v);
void abi_RunAllListChannels1A31(void);
ChannelExit ListEntry1B91(uint8_t y, bool c, bool v);
void abi_ListEntry1B91(void);
Exit EffectEntry1B91(uint8_t y);
void abi_EffectEntry1B91(void);
VramExit VramEntry1B91(uint8_t x, uint8_t y, bool carry, bool overflow);
void abi_VramEntry1B91(void);
ChannelExit RunAllListChannels1B91(uint8_t y, bool c, bool v);
void abi_RunAllListChannels1B91(void);
ChannelExit ListEntry25F7(uint8_t y, bool c, bool v);
void abi_ListEntry25F7(void);
Exit EffectEntry25F7(uint8_t y);
void abi_EffectEntry25F7(void);
VramExit VramEntry25F7(uint8_t x, uint8_t y, bool carry, bool overflow);
void abi_VramEntry25F7(void);
ChannelExit RunAllListChannels25F7(uint8_t y, bool c, bool v);
void abi_RunAllListChannels25F7(void);
ChannelExit ListEntry1CF1(uint8_t y, bool c, bool v);
void abi_ListEntry1CF1(void);
Exit EffectEntry1CF1(uint8_t y);
void abi_EffectEntry1CF1(void);
VramExit VramEntry1CF1(uint8_t x, uint8_t y, bool carry, bool overflow);
void abi_VramEntry1CF1(void);
ChannelExit RunAllListChannels1CF1(uint8_t y, bool c, bool v);
void abi_RunAllListChannels1CF1(void);

#endif
