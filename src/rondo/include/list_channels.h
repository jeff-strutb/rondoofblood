/* The loaded versions of RunListChannel (system/list_channels.c): declared here, not in routines.h. */
#ifndef RONDO_LIST_CHANNELS_H
#define RONDO_LIST_CHANNELS_H

#include "list_channels_census.h"

ChannelExit RunListChannel00C5(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel00C5(void);
ChannelExit RunListChannel07D1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel07D1(void);
ChannelExit RunListChannel0931(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0931(void);
ChannelExit RunListChannel09E1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel09E1(void);
ChannelExit RunListChannel0B41(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0B41(void);
ChannelExit RunListChannel1611(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel1611(void);
ChannelExit RunListChannel12D1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel12D1(void);

#endif
