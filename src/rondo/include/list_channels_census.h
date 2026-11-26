/* The list channels of the census versions of the ending's effect overlay (list_channels_census.c). */
#ifndef RONDO_LIST_CHANNELS_CENSUS_H
#define RONDO_LIST_CHANNELS_CENSUS_H

#include <stdbool.h>
#include <stdint.h>
#include "machine.h"

/* What a list channel leaves: A, X, Y and the flags (N and Z from nz), each register or flag marked kept when the
 * path does not set it; left: a VRAM list went on into original code, and A, Y, C and V are as it left them. */
typedef struct {
    uint8_t a, x, y, nz;
    bool c, v;
    bool y_set, c_set, v_set, left;
} ChannelExit;

ChannelExit RunListChannel0441(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0441(void);
ChannelExit RunListChannel0651(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0651(void);
ChannelExit RunListChannel1231(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel1231(void);
ChannelExit RunListChannel1A31(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel1A31(void);
ChannelExit RunListChannel1B91(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel1B91(void);
ChannelExit RunListChannel25F7(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel25F7(void);
ChannelExit RunListChannel1CF1(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel1CF1(void);

#endif
