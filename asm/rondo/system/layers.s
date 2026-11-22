; @routine 0515:12F HalveScrollStepFirstToSecond
; @run 6F:A12F
; WHAT IT DOES: Sets entry 1 of this frame's X scroll step to half of entry 0's.
; HOW: Entry 0 to entry 1 through HalveScrollStep. The three bytes load Y and clear X, then
; run on into it.
; WAITS: no.
; Entry: nothing. Exit: as HalveScrollStep, X 0 and Y 1.
HalveScrollStepFirstToSecond:
    ldy #$01
    clx

; @routine 0515:132 HalveScrollStep
; @run 6F:A132
; WHAT IT DOES: Sets one entry of this frame's X scroll step to half another's, so a layer
; behind the first moves at half its speed.
; HOW: A signed 16-bit shift right: the high byte keeps its sign (the first read of it
; only gives the sign, the second is shifted), and the bit it loses goes into the low byte's
; top. The high byte is written before the low byte is read.
; WAITS: no.
; Entry: X the entry to halve, Y the entry that gets the result. Exit: A the low byte written, N
; and Z from it, C the low bit shifted out. X and Y kept.
HalveScrollStep:
    lda a:ScrollStepXHi,x
    asl a
    lda a:ScrollStepXHi,x
    ror a
    sta a:ScrollStepXHi,y
    lda a:ScrollStepXLo,x
    ror a
    sta a:ScrollStepXLo,y
    rts

; @routine 07D5:131 HalveScrollStepFromFirst
; @run 6F:A131
; WHAT IT DOES: Sets entry Y of this frame's X scroll step to half of entry 0's.
; HOW: Entry 0 into HalveScrollStep. The one byte is a CLX that runs on into it.
; WAITS: no.
; Entry: Y the entry that gets the result. Exit: as HalveScrollStep, X 0.
HalveScrollStepFromFirst:
    clx
