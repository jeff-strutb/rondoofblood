/*
 * The attract mode's scene: the state handlers the attract mode runs one a frame (zGameSubMode
 * is the state), and the scene script's own helpers. The scene's frame counter is the word
 * zWaitTimer3 (low) and zWaitTimer4 (high).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"

enum { OBJECT_SLOTS = 0x58 };

/*
 * @routine 00C1:7CC AttractFrameTick
 * @run 6B:87CC
 * WHAT IT DOES: Counts one frame of the attract scene on its 16-bit frame counter.
 * HOW: Adds one to the low byte, and to the high byte when the low byte wrapped to zero.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the byte last incremented (the low byte, or the high byte
 * when the low one wrapped), C and the registers kept.
 */
uint8_t AttractFrameTick(void)
{
    uint8_t low;

    low = INC8(ZP->wait_timer3);
    if (low != 0) {
        return low;
    }
    return INC8(ZP->wait_timer4);
}

void abi_AttractFrameTick(void)
{
    abi_nz(AttractFrameTick());
}

/*
 * @routine 00C1:7D3 AttractEndCountdown
 * @run 6B:87D3
 * WHAT IT DOES: The attract scene's last state: counts the frame counter down, and when it
 * runs out wipes the game state and takes the scene's done mark back off.
 * HOW: Subtracts one from the low byte of the frame counter and returns while it is not zero.
 * At zero it calls ResetGameState and subtracts one from zModeDone.
 * WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off;
 * the stack there is this routine's return address and the call's. The comparison with the draft
 * shows its first difference inside ResetGameState (its C does not yet place the frame of its
 * own call to the vblank wait), not in this routine's own accesses: its own are the frame, the
 * call and a decrement of zModeDone, and agree up to that point.
 * Entry: nothing. Exit: N and Z from the byte last decremented, C and the registers as
 * ResetGameState leaves them (kept when the count has not run out).
 */
AttractEnd AttractEndCountdown(void)
{
    AttractEnd end;

    end.value = DEC8(ZP->wait_timer3);
    end.reset = false;
    if (end.value != 0) {
        return end;
    }
    jsr_push(0x87DA);
    ResetGameState();
    jsr_pop();
    end.value = DEC8(ZP->mode_done);
    end.reset = true;
    return end;
}

void abi_AttractEndCountdown(void)
{
    AttractEnd end = AttractEndCountdown();

    if (end.reset) {
        reset_game_state_exit_shim();
    }
    abi_nz(end.value);
}

/*
 * @routine 00C2:0F7 ClearAllObjects
 * @run 6B:88F7
 * WHAT IT DOES: Empties all 88 object slots, the scene script's way of clearing the scene.
 * HOW: A loop over the slots from 0 up, each through ClearObject.
 * WAITS: no.
 * Entry: nothing. Exit: X $58, C and Z set, N clear (the loop's last compare); A and Y kept.
 */
void ClearAllObjects(void)
{
    uint8_t slot;

    for (slot = 0; slot != OBJECT_SLOTS; slot++) {
        jsr_push(0x88FB);
        ClearObject(slot);
        jsr_pop();
    }
}

void abi_ClearAllObjects(void)
{
    ClearAllObjects();
    cpu.x = OBJECT_SLOTS;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 00C1:71A AttractBegin
 * @run 6B:871A
 * WHAT IT DOES: Starts the attract scene: the game state is wiped, the scene's script is set to
 * run from $8F98, its frame counter to $FFFF (so that the first tick brings it to 0) and the
 * mode goes on to its next state.
 * HOW: ResetGameState; the script pointer (the two bytes at $7E) and the frame counter are
 * written; zGameSubMode goes up by one.
 * WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off;
 * the stack there is this routine's return address and the call's.
 * Entry: nothing. Exit: A $FF, X and Y as ResetGameState leaves them, N and Z from the new state,
 * C as ResetGameState leaves it. The comparison with the draft shows its first difference
 * inside ResetGameState (its C does not yet place the frame of its own call to the vblank wait);
 * this routine's own accesses agree up to that call.
 */
uint8_t AttractBegin(void)
{
    jsr_push(0x871D);
    ResetGameState();
    jsr_pop();
    WR8(ZP->_gap_007C[2], 0x98);
    WR8(ZP->_gap_007C[3], 0x8F);
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    return INC8(ZP->game_sub_mode);
}

void abi_AttractBegin(void)
{
    uint8_t state = AttractBegin();

    reset_game_state_exit_shim();
    cpu.a = 0xFF;
    abi_nz(state);
}

/*
 * @routine 00C1:265 ModeDoneAdd
 * @run 6B:8265
 * WHAT IT DOES: Adds one to zModeDone, the count the mode loop reads to learn that the mode has
 * finished.
 * HOW: One increment.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the new value, C and the registers kept. No script reaches
 * it; it is compared with the draft only.
 */
uint8_t ModeDoneAdd(void)
{
    return INC8(ZP->mode_done);
}

void abi_ModeDoneAdd(void)
{
    abi_nz(ModeDoneAdd());
}
