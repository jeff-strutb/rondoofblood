; @routine 010D:029 EndingFrameTick
; @run 6B:8029
; WHAT IT DOES: Counts one frame of the ending scene on its 16-bit frame counter.
; HOW: Adds one to zWaitTimer3, and to zWaitTimer4 when it wrapped to zero.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the byte last incremented, C and the registers kept.
EndingFrameTick:
    inc z:zWaitTimer3
    bne EndingFrameTick_at0
    inc z:zWaitTimer4
EndingFrameTick_at0:
    rts

; @routine 010D:067 EndingModeDoneBack
; @run 6B:8067
; WHAT IT DOES: Takes one back off zModeDone.
; HOW: One decrement.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the new value, C and the registers kept.
EndingModeDoneBack:
    dec z:zModeDone
    rts

; @routine 010D:2DC EndingClearAfterScrollX
; @run 6B:82DC
; WHAT IT DOES: An object handler: counts the first scroll's X high byte up and clears the object.
; HOW: One increment; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: N and Z from the new byte; A, X, Y and C kept.
EndingClearAfterScrollX:
    inc a:ScrollXHi
    jmp VecClearObject

; @routine 010D:2E2 EndingClearAfterScrollY
; @run 6B:82E2
; WHAT IT DOES: An object handler: counts the first scroll's Y high byte up and clears the object.
; HOW: One increment; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: N and Z from the new byte; A, X, Y and C kept.
EndingClearAfterScrollY:
    inc a:ScrollYHi
    jmp VecClearObject

; @routine 010D:3CE EndingScrollPull
; @run 6B:83CE
; WHAT IT DOES: An object handler of two states: the first sets the first scroll's X low byte to $80; the second
; takes one off the X of objects 1, 2 and 3 a frame (with the borrow into the high bytes of objects 1 and 3)
; and counts the first scroll's X low byte down, moving on when it reaches 0.
; HOW: OBJ->state; decrements with a test for $FF.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollPull:
    lda a:OBJ_27A8,x
    bne EndingScrollPull_at0
    lda #$80
    sta a:ScrollXLo
    inc a:OBJ_27A8,x
    rts
EndingScrollPull_at0:
    dec a
    bne EndingScrollPull_at1
    dec a:OBJ_X_LO+1
    lda a:OBJ_X_LO+1
    cmp #$FF
    bne EndingScrollPull_at2
    dec a:OBJ_X_HI+1
EndingScrollPull_at2:
    dec a:OBJ_X_LO+2
    dec a:OBJ_X_LO+3
    lda a:OBJ_X_LO+3
    cmp #$FF
    bne EndingScrollPull_at3
    dec a:OBJ_X_HI+3
EndingScrollPull_at3:
    dec a:ScrollXLo
    bne EndingScrollPull_at1
    inc a:OBJ_27A8,x
EndingScrollPull_at1:
    rts

; @routine 010D:405 EndingScrollRunUp
; @run 6B:8405
; WHAT IT DOES: An object handler: in its first state counts the first scroll's X low byte up each frame until
; it reaches $80, then moves the object to its next state.
; HOW: OBJ->state; one increment of SCROLL->x_lo[0] and a compare.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollRunUp:
    lda a:OBJ_27A8,x
    bne EndingScrollRunUp_at0
    inc a:ScrollXLo
    lda a:ScrollXLo
    cmp #$80
    bne EndingScrollRunUp_at0
    inc a:OBJ_27A8,x
EndingScrollRunUp_at0:
    rts

; @routine 010D:5F2 EndingNothing
; @run 6B:85F2
; WHAT IT DOES: Returns at once.
; HOW: An RTS.
; WAITS: no.
; Entry: nothing. Exit: everything kept.
EndingNothing:
    rts

; @routine 010D:5F3 EndingHoldTimer
; @run 6B:85F3
; WHAT IT DOES: Counts down an object's hold timer; while it has not run out the caller is not returned to:
; the routine returns to the caller's caller instead.
; HOW: OBJ->life_timer counted down; when it is not zero the return address of the call is pulled off the
; stack (two PLA) before the RTS.
; WAITS: no.
; Entry: X the object, the caller's frame on the stack. Exit: N and Z from the timer, Z set when it has run out;
; A, X, Y and C kept.
EndingHoldTimer:
    dec a:OBJ_34B8,x
    beq EndingHoldTimer_at0
    pla
    pla
EndingHoldTimer_at0:
    rts

; @routine 010D:65E EndingScrollBack8
; @run 6B:865E
; WHAT IT DOES: Takes 8 off the low byte of the first scroll X.
; HOW: A subtract of 8 from SCROLL->x_lo[0].
; WAITS: no.
; Entry: nothing. Exit: A the new byte, N and Z from it, C and V from the subtract; X and Y kept.
EndingScrollBack8:
    lda a:ScrollXLo
    sec
    sbc #$08
    sta a:ScrollXLo
    rts

; @routine 010E:0EF EndingScrollSlots
; @run 6B:80EF
; WHAT IT DOES: Moves the scroll slots 3 and 4 on: 4 added to the low X of slot 3, 8 to the low X of slot 4.
; HOW: Two adds.
; WAITS: no.
; Entry: nothing. Exit: A the second sum, N and Z from it, C and V from its add; X and Y kept.
EndingScrollSlots:
    lda a:ScrollXLo+3
    clc
    adc #$04
    sta a:ScrollXLo+3
    lda a:ScrollXLo+4
    clc
    adc #$08
    sta a:ScrollXLo+4
    rts

; @routine 010E:1F6 EndingFrameList
; @run 6B:81F6
; WHAT IT DOES: Steps an object's animation along the list the ending's table names: the list's address is
; read from the table at $8A05 by A, and StepObjectFrames runs on it.
; HOW: A doubled indexes the table; its two bytes go to zTemp10 and zTemp11; StepObjectFrames as a tail jump.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames; X kept.
EndingFrameList:
    asl a
    tay
    lda a:$8A05,y
    sta z:zTemp10
    lda a:$8A06,y
    sta z:zTemp11
    jmp VecStepObjectFrames
