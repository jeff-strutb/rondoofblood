/*
 * Byte copies of routines filed elsewhere, at their own places on the disc (docs/rondo/PLAN.md, section 3): each is
 * the same routine, so its C is the original's; they are filed so that their assembly is assembled there too.
 */
#include "machine.h"
#include "routines.h"
#include "front_end.h"
#include "layers.h"
#include "list_channels.h"
#include "vram_update.h"
#include "front_end_copies.h"
void abi_VramCommandsAddress(void);
VramExit VramCommandsAddress(uint8_t y, bool carry, bool overflow);
void abi_VramCommandsAddressAdd(void);
VramExit VramCommandsAddressAdd(uint8_t y, bool carry, bool overflow);
void abi_VramCommandsPairs(void);
VramExit VramCommandsPairs(uint8_t y, bool carry, bool overflow);

/*
 * @routine 0111:018 RunListChannel0111
 * @run 6C:8018
 * WHAT IT DOES: As RunListChannel00C5 (00C5:018), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as RunListChannel00C5.
 */
ChannelExit RunListChannel0111(uint8_t kind, uint8_t slot, uint8_t y, bool c, bool v)
{
    return RunListChannel00C5(kind, slot, y, c, v);
}

void abi_RunListChannel0111(void)
{
    abi_RunListChannel00C5();
}

/*
 * @routine 0111:133 AddToListWord0111
 * @run 6C:8133
 * WHAT IT DOES: As AddToListWord (00C5:133), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as AddToListWord.
 */
Sum AddToListWord0111(uint8_t amount, uint8_t entry)
{
    return AddToListWord(amount, entry);
}

void abi_AddToListWord0111(void)
{
    abi_AddToListWord();
}

/*
 * @routine 0111:151 VramListFromIndex0111
 * @run 6C:8151
 * WHAT IT DOES: As VramListFromIndex (00C5:151), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as VramListFromIndex.
 */
VramExit VramListFromIndex0111(uint8_t a, uint8_t x, bool negative, bool carry, bool overflow)
{
    return VramListFromIndex(a, x, negative, carry, overflow);
}

void abi_VramListFromIndex0111(void)
{
    abi_VramListFromIndex();
}

/*
 * @routine 0111:169 VramListRun0111
 * @run 6C:8169
 * WHAT IT DOES: As VramListRun (00C5:169), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as VramListRun.
 */
VramExit VramListRun0111(uint8_t x, uint8_t y, bool carry, bool overflow)
{
    return VramListRun(x, y, carry, overflow);
}

void abi_VramListRun0111(void)
{
    abi_VramListRun();
}

/*
 * @routine 0111:1CD VramCommandsFetch0111
 * @run 6C:81CD
 * WHAT IT DOES: As VramCommandsFetch (00C5:1CD), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as VramCommandsFetch.
 */
VramExit VramCommandsFetch0111(bool carry, bool overflow)
{
    return VramCommandsFetch(carry, overflow);
}

void abi_VramCommandsFetch0111(void)
{
    abi_VramCommandsFetch();
}

/*
 * @routine 0111:201 VramCommandsNext0111
 * @run 6C:8201
 * WHAT IT DOES: As VramCommandsNext (00C5:201), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as VramCommandsNext.
 */
VramExit VramCommandsNext0111(uint8_t y, bool carry, bool overflow)
{
    return VramCommandsNext(y, carry, overflow);
}

void abi_VramCommandsNext0111(void)
{
    abi_VramCommandsNext();
}

/*
 * @routine 0111:20E VramCommandsRepeat0111
 * @run 6C:820E
 * WHAT IT DOES: As VramCommandsRepeat (00C5:20E), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as VramCommandsRepeat.
 */
VramExit VramCommandsRepeat0111(uint8_t a, uint8_t y, bool carry, bool overflow)
{
    return VramCommandsRepeat(a, y, carry, overflow);
}

void abi_VramCommandsRepeat0111(void)
{
    abi_VramCommandsRepeat();
}

/*
 * @routine 0111:22B VramCommandsPairs0111
 * @run 6C:822B
 * WHAT IT DOES: As VramCommandsPairs (00C5:22B), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as VramCommandsPairs.
 */
VramExit VramCommandsPairs0111(uint8_t y, bool carry, bool overflow)
{
    return VramCommandsPairs(y, carry, overflow);
}

void abi_VramCommandsPairs0111(void)
{
    abi_VramCommandsPairs();
}

/*
 * @routine 0111:247 VramCommandsAddressAdd0111
 * @run 6C:8247
 * WHAT IT DOES: As VramCommandsAddressAdd (00C5:247), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as VramCommandsAddressAdd.
 */
VramExit VramCommandsAddressAdd0111(uint8_t y, bool carry, bool overflow)
{
    return VramCommandsAddressAdd(y, carry, overflow);
}

void abi_VramCommandsAddressAdd0111(void)
{
    abi_VramCommandsAddressAdd();
}

/*
 * @routine 0111:266 VramCommandsAddress0111
 * @run 6C:8266
 * WHAT IT DOES: As VramCommandsAddress (00C5:266), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as VramCommandsAddress.
 */
VramExit VramCommandsAddress0111(uint8_t y, bool carry, bool overflow)
{
    return VramCommandsAddress(y, carry, overflow);
}

void abi_VramCommandsAddress0111(void)
{
    abi_VramCommandsAddress();
}

/*
 * @routine 0111:27F CopyWordsToVdc0111
 * @run 6C:827F
 * WHAT IT DOES: As CopyWordsToVdc (00C5:27F), whose bytes these are.
 * HOW: The same instructions; filed here so that they are assembled at this origin too.
 * WAITS: no.
 * Entry, exit: as CopyWordsToVdc.
 */
Sum CopyWordsToVdc0111(uint8_t blocks)
{
    return CopyWordsToVdc(blocks);
}

void abi_CopyWordsToVdc0111(void)
{
    abi_CopyWordsToVdc();
}
