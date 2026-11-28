/* Byte copies filed at their own places (src/rondo/menu/front_end_copies.c). */
#ifndef RONDO_FRONT_END_COPIES_H
#define RONDO_FRONT_END_COPIES_H

#include "routines.h"
#include "front_end.h"
#include "vram_update.h"
#include "list_channels.h"
#include "layers.h"
ChannelExit RunListChannel0111(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v);
void abi_RunListChannel0111(void);
Sum AddToListWord0111(uint8_t amount, uint8_t entry);
void abi_AddToListWord0111(void);
VramExit VramListFromIndex0111(uint8_t a, uint8_t x, bool negative, bool carry, bool overflow);
void abi_VramListFromIndex0111(void);
VramExit VramListRun0111(uint8_t x, uint8_t y, bool carry, bool overflow);
void abi_VramListRun0111(void);
VramExit VramCommandsFetch0111(bool carry, bool overflow);
void abi_VramCommandsFetch0111(void);
VramExit VramCommandsNext0111(uint8_t y, bool carry, bool overflow);
void abi_VramCommandsNext0111(void);
VramExit VramCommandsRepeat0111(uint8_t a, uint8_t y, bool carry, bool overflow);
void abi_VramCommandsRepeat0111(void);
VramExit VramCommandsPairs0111(uint8_t y, bool carry, bool overflow);
void abi_VramCommandsPairs0111(void);
VramExit VramCommandsAddressAdd0111(uint8_t y, bool carry, bool overflow);
void abi_VramCommandsAddressAdd0111(void);
VramExit VramCommandsAddress0111(uint8_t y, bool carry, bool overflow);
void abi_VramCommandsAddress0111(void);
Sum CopyWordsToVdc0111(uint8_t blocks);
void abi_CopyWordsToVdc0111(void);
uint8_t Scene5CFrameTick(void);
void abi_Scene5CFrameTick(void);
void abi_VecRunListChannels0111(void);

#endif
