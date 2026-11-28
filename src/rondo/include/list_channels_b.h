/* RunListChannel in the stage overlays of stages $09, $0A, $0B, $10 and the secret stages (system/list_channels_b.c). */
#ifndef RONDO_LIST_CHANNELS_B_H
#define RONDO_LIST_CHANNELS_B_H

#include "list_channels_census.h"

ChannelExit RunListChannel05C1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel05C1(void);
ChannelExit RunListChannel0721(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0721(void);
ChannelExit RunListChannel0881(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0881(void);
ChannelExit RunListChannel0E01(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0E01(void);
ChannelExit RunListChannel0EB1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0EB1(void);
ChannelExit RunListChannel0F61(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0F61(void);
ChannelExit RunListChannel1011(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel1011(void);
ChannelExit RunListChannel10C1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel10C1(void);
ChannelExit RunListChannel1171(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel1171(void);
ChannelExit RunListChannel16C1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel16C1(void);
ChannelExit RunListChannel18D1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel18D1(void);

#endif
