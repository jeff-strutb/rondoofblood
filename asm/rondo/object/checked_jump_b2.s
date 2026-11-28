; @routine 13FC:1C2 CheckedJump13FC1C2
; @run 70:B9C2
; WHAT IT DOES: An object's dispatcher (Secret stage 0C): WORK_K is counted on, the loaded overlay's routines at $B9EA, $BA18, $BA2F are run,
; then the handler of its state from the table at $B9E0.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump13FC1C2Handlers = $B9E0 ; the handlers by state
CheckedJump13FC1C2:
    inc a:OBJ_WORK_K,x
    jsr KeepNearScreen13FC
    jsr ToState4WhenSlot16Marked13FC
    jsr HitBoxByFrameRange13FC
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump13FC1C2Handlers,y
    sta z:zTemp00
    lda a:CheckedJump13FC1C2Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FD:11C CheckedJump13FD11C
; @run 71:C11C
; WHAT IT DOES: An object's dispatcher (Secret stage 0C): WORK_K is counted on, the loaded overlay's routines at $B9EA, $BA18, $BA2F are run,
; then the handler of its state from the table at $C13A.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump13FD11CHandlers = $C13A ; the handlers by state
CheckedJump13FD11C:
    inc a:OBJ_WORK_K,x
    jsr KeepNearScreen13FC
    jsr ToState4WhenSlot16Marked13FC
    jsr HitBoxByFrameRange13FC
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump13FD11CHandlers,y
    sta z:zTemp00
    lda a:CheckedJump13FD11CHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161D:5A4 CheckedJump161D5A4
; @run 71:C5A4
; WHAT IT DOES: An object's dispatcher (Stage 0E): WORK_F is counted on, the loaded overlay's routines at $C5CE, $C5F8, $C60A are run,
; then the handler of its state from the table at $C5C2.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump161D5A4Handlers = $C5C2 ; the handlers by state
CheckedJump161D5A4:
    inc a:OBJ_WORK_F,x
    jsr FaceThePlayerEvery8_161D
    jsr HalveWhenHit161D
    jsr HitStopAndClearNext161D
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump161D5A4Handlers,y
    sta z:zTemp00
    lda a:CheckedJump161D5A4Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)
