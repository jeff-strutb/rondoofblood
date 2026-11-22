; @routine 00C1:7CC AttractFrameTick
; @run 6B:87CC
; WHAT IT DOES: Counts one frame of the attract scene on its 16-bit frame counter.
; HOW: Adds one to the low byte, and to the high byte when the low byte wrapped to zero.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the byte last incremented (the low byte, or the high byte
; when the low one wrapped), C and the registers kept.
AttractFrameTick:
    inc z:zWaitTimer3
    bne AttractFrameTick_at0
    inc z:zWaitTimer4
AttractFrameTick_at0:
    rts

; @routine 00C1:7D3 AttractEndCountdown
; @run 6B:87D3
; WHAT IT DOES: The attract scene's last state: counts the frame counter down, and when it
; runs out wipes the game state and takes the scene's done mark back off.
; HOW: Subtracts one from the low byte of the frame counter and returns while it is not zero.
; At zero it calls ResetGameState and subtracts one from zModeDone.
; WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off;
; the stack there is this routine's return address and the call's. The comparison with the draft
; does not reach the arm that calls it, since the harness stops at a wait (the draft stops there
; too); that arm is one call and a decrement of zModeDone.
; Entry: nothing. Exit: N and Z from the byte last decremented, C and the registers as
; ResetGameState leaves them (kept when the count has not run out).
AttractEndCountdown:
    dec z:zWaitTimer3
    bne AttractEndCountdown_at0
    jsr VecResetGameState
    dec z:zModeDone
AttractEndCountdown_at0:
    rts

; @routine 00C2:0F7 ClearAllObjects
; @run 6B:88F7
; WHAT IT DOES: Empties all 88 object slots, the scene script's way of clearing the scene.
; HOW: A loop over the slots from 0 up, each through ClearObject.
; WAITS: no.
; Entry: nothing. Exit: X $58, C and Z set, N clear (the loop's last compare); A and Y kept.
ClearAllObjects:
    clx
ClearAllObjects_at0:
    jsr VecClearObject
    inx
    cpx #$58
    bne ClearAllObjects_at0
    rts
