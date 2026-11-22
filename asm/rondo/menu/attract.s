; @routine 00C1:265 ModeDoneAdd
; @run 6B:8265
; WHAT IT DOES: Adds one to zModeDone, the count the mode loop reads to learn that the mode has
; finished.
; HOW: One increment.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the new value, C and the registers kept. No script reaches
; it; it is compared with the draft only.
ModeDoneAdd:
    inc z:zModeDone
    rts

; @routine 00C1:71A AttractBegin
; @run 6B:871A
; WHAT IT DOES: Starts the attract scene: the game state is wiped, the scene's script is set to
; run from $8F98, its frame counter to $FFFF (so that the first tick brings it to 0) and the
; mode goes on to its next state.
; HOW: ResetGameState; the script pointer (the two bytes at $7E) and the frame counter are
; written; zGameSubMode goes up by one.
; WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off;
; the stack there is this routine's return address and the call's.
; Entry: nothing. Exit: A $FF, X and Y as ResetGameState leaves them, N and Z from the new state,
; C as ResetGameState leaves it. The comparison with the draft shows its first difference
; inside ResetGameState (its C does not yet place the frame of its own call to the vblank wait);
; this routine's own accesses agree up to that call.
AttractBegin:
    jsr VecResetGameState
    lda #$98
    sta z:$7E
    lda #$8F
    sta z:$7F
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    rts

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
; shows its first difference inside ResetGameState (its C does not yet place the frame of its
; own call to the vblank wait), not in this routine's own accesses: its own are the frame, the
; call and a decrement of zModeDone, and agree up to that point.
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
