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

; @routine 010D:418 EndingPlayList10
; @run 6B:8418
; WHAT IT DOES: An object handler: shows picture kind 3, waits a frame, then plays the ending's list $10 until it
; ends and moves to the next state.
; HOW: OBJ->state and OBJ->sprite_set; EndingFrameList.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingPlayList10:
    lda a:OBJ_27A8,x
    bne EndingPlayList10_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
    rts
EndingPlayList10_at0:
    dec a
    bne EndingPlayList10_at1
EndingPlayList10_at2:
    rts
EndingPlayList10_at1:
    dec a
    bne EndingPlayList10_at2
    lda #$10
    jsr EndingFrameList
    bne EndingPlayList10_at2
    inc a:OBJ_27A8,x
    rts

; @routine 010D:438 EndingScrollWindowSteps
; @run 6B:8438
; WHAT IT DOES: An object handler of four states: the first starts the raster effect $13, sets scroll slots 1 and 5 to Y
; $1F and 2 and 6 to $AF and holds the object $20 frames; the second waits the hold out; the third moves on; the
; fourth moves scroll slot 1's X on by 4 a frame and, when it wraps to 0, moves the first object and scroll slot 1's
; X high byte on and the object to its next state.
; HOW: OBJ->state; EndingHoldTimer; SCROLL->x_lo and x_hi.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollWindowSteps:
    lda a:OBJ_27A8,x
    bne EndingScrollWindowSteps_at0
    lda #$13
    sta z:zRasterEffect
    lda #$1F
    sta a:ScrollYLo+5
    sta a:ScrollYLo+1
    lda #$AF
    sta a:ScrollYLo+6
    sta a:ScrollYLo+2
    lda #$20
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts
EndingScrollWindowSteps_at0:
    dec a
    bne EndingScrollWindowSteps_at1
    jsr EndingHoldTimer
    inc a:OBJ_27A8,x
    rts
EndingScrollWindowSteps_at1:
    dec a
    bne EndingScrollWindowSteps_at2
    inc a:OBJ_27A8,x
    rts
EndingScrollWindowSteps_at2:
    dec a
    bne EndingScrollWindowSteps_at3
    lda a:ScrollXLo+1
    clc
    adc #$04
    sta a:ScrollXLo+1
    bne EndingScrollWindowSteps_at4
    inc a:OBJ_27A8
    inc a:ScrollXHi+1
    inc a:OBJ_27A8,x
EndingScrollWindowSteps_at4:
    rts
EndingScrollWindowSteps_at3:
    rts

; @routine 010D:484 EndingScrollWindowA
; @run 6B:8484
; WHAT IT DOES: An object handler: in its first state starts the raster effect $13 and sets scroll slots 1 and 5 to Y
; $1F and 2 and 6 to $BF, then moves to the next state.
; HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollWindowA:
    lda a:OBJ_27A8,x
    bne EndingScrollWindowA_at0
    lda #$13
    sta z:zRasterEffect
    lda #$1F
    sta a:ScrollYLo+5
    sta a:ScrollYLo+1
    lda #$BF
    sta a:ScrollYLo+6
    sta a:ScrollYLo+2
    inc a:OBJ_27A8,x
EndingScrollWindowA_at0:
    rts

; @routine 010D:4A1 EndingScrollWindowB
; @run 6B:84A1
; WHAT IT DOES: As EndingScrollWindowA with the Y values $27 and $C7.
; HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollWindowB:
    lda a:OBJ_27A8,x
    bne EndingScrollWindowA_at0
    lda #$13
    sta z:zRasterEffect
    lda #$27
    sta a:ScrollYLo+5
    sta a:ScrollYLo+1
    lda #$C7
    sta a:ScrollYLo+6
    sta a:ScrollYLo+2
    inc a:OBJ_27A8,x
    rts

; @routine 010D:4BE EndingPlayList0F
; @run 6B:84BE
; WHAT IT DOES: As EndingPlayList10, with the ending's list $0F.
; HOW: OBJ->state and OBJ->sprite_set; EndingFrameList.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingPlayList0F:
    lda a:OBJ_27A8,x
    bne EndingPlayList0F_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
    rts
EndingPlayList0F_at0:
    dec a
    bne EndingPlayList0F_at1
EndingPlayList0F_at2:
    rts
EndingPlayList0F_at1:
    dec a
    bne EndingPlayList0F_at2
    lda #$0F
    jsr EndingFrameList
    bne EndingPlayList0F_at2
    inc a:OBJ_27A8,x
    rts

; @routine 010D:4DE EndingRiseToY40
; @run 6B:84DE
; WHAT IT DOES: An object handler: shows picture kind 3, waits a frame, then shows frame $0D and moves the object up
; a line each frame until its Y low byte is $40, then sets flash flags $8D and moves to the next state.
; HOW: OBJ->state, OBJ->sprite_set, OBJ->frame, OBJ->y_lo and OBJ->flash_flags.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingRiseToY40:
    lda a:OBJ_27A8,x
    bne EndingRiseToY40_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
    rts
EndingRiseToY40_at0:
    dec a
    bne EndingRiseToY40_at1
EndingRiseToY40_at3:
    rts
EndingRiseToY40_at1:
    dec a
    bne EndingRiseToY40_at2
    lda #$0D
    sta a:OBJ_2A68,x
    dec a:OBJ_Y_LO,x
    lda a:OBJ_Y_LO,x
    cmp #$40
    bne EndingRiseToY40_at3
    lda #$8D
    sta a:OBJ_2B18,x
    inc a:OBJ_27A8,x
    rts
EndingRiseToY40_at2:
    rts

; @routine 010D:50C EndingPlayList0EThenClear
; @run 6B:850C
; WHAT IT DOES: An object handler: shows picture kind 3, waits a frame, then plays the ending's list $0E until it
; ends and clears the object.
; HOW: OBJ->state and OBJ->sprite_set; EndingFrameList; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingPlayList0EThenClear:
    lda a:OBJ_27A8,x
    bne EndingPlayList0EThenClear_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
    rts
EndingPlayList0EThenClear_at0:
    dec a
    bne EndingPlayList0EThenClear_at1
EndingPlayList0EThenClear_at2:
    rts
EndingPlayList0EThenClear_at1:
    lda #$0E
    jsr EndingFrameList
    bne EndingPlayList0EThenClear_at2
    jmp VecClearObject

; @routine 010D:548 EndingScrollWindowHold
; @run 6B:8548
; WHAT IT DOES: An object handler: starts the raster effect $13, sets scroll slots 1 and 5 to Y $0F and 2 and 6 to $6F,
; holds the object for $40 frames and moves to the next state.
; HOW: ZP->raster_effect; SCROLL->y_lo; OBJ->life_timer.
; WAITS: no.
; Entry: X the object. Exit: A $40, N and Z from the new state; C and the registers kept.
EndingScrollWindowHold:
    lda #$13
    sta z:zRasterEffect
    lda #$0F
    sta a:ScrollYLo+5
    sta a:ScrollYLo+1
    lda #$6F
    sta a:ScrollYLo+6
    sta a:ScrollYLo+2
    lda #$40
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts

; @routine 010D:565 EndingStartList3
; @run 6B:8565
; WHAT IT DOES: An object handler: once its hold has run out starts list 0 on channel 7 as kind 3, holds the object $20
; frames and moves to the next state.
; HOW: EndingHoldTimer (which returns to the dispatch while the hold lasts); StartListChannelKind3; OBJ->life_timer.
; WAITS: no.
; Entry: X the object, A as the dispatch left it. Exit: A $20, N and Z from the new state; C and the registers kept.
; While the hold lasts: A kept, N and Z from the timer.
EndingStartList3:
    jsr EndingHoldTimer
    phx
    cla
    ldx #$07
    jsr VecStartListChannelKind3
    plx
    lda #$20
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts

; @routine 010D:579 EndingStartScript18
; @run 6B:8579
; WHAT IT DOES: An object handler: once its hold has run out starts script $18 on slot 7 as kind 1 and moves object 2's frame,
; object 3's X, scroll slot 2's X high byte and low byte, and the object's state on.
; HOW: EndingHoldTimer; StartScriptSlotKind1; five counts.
; WAITS: no.
; Entry: X the object, A as the dispatch left it. Exit: A and Y $18, N and Z from the new state; C and X kept.
; While the hold lasts: A kept, N and Z from the timer.
EndingStartScript18:
    jsr EndingHoldTimer
    phx
    lda #$18
    ldx #$07
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_2A68+2
    inc a:OBJ_X_LO+3
    inc a:OBJ_X_LO+3
    inc a:ScrollXHi+2
    dec a:ScrollXLo+2
    inc a:OBJ_27A8,x
    rts

; @routine 010D:598 EndingScrollSlide
; @run 6B:8598
; WHAT IT DOES: An object handler: takes 4 off scroll slot 2's X low byte; while the result is $80 or more it is stored and
; nothing else happens; below that it counts scroll slot 6's Y high byte up, holds the object $30 frames, counts object
; 4's state, starts script $17 on slot 7 as kind 1, moves the object on and stores $80.
; HOW: SCROLL->x_lo; StartScriptSlotKind1.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollSlide:
    lda a:ScrollXLo+2
    sec
    sbc #$04
    cmp #$80
    bcs EndingScrollSlide_at0
    inc a:ScrollYHi+6
    lda #$30
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8+4
    phx
    lda #$17
    ldx #$07
    jsr VecStartScriptSlotKind1
    plx
    lda #$80
    inc a:OBJ_27A8,x
EndingScrollSlide_at0:
    sta a:ScrollXLo+2
    rts

; @routine 010D:5BF EndingSplitScrolls
; @run 6B:85BF
; WHAT IT DOES: An object handler: once its hold has run out resets scroll slots 2 and 6 and moves slots 1 and 2's Y high bytes on,
; sets slot 2's Y low byte to $6F, holds the object $20 frames and moves objects 1 and this one to their next states.
; HOW: EndingHoldTimer; stores and counts on SCROLL.
; WAITS: no.
; Entry: X the object, A as the dispatch left it. Exit: A $20, N and Z from the new state; C and the registers kept.
; While the hold lasts: A kept, N and Z from the timer.
EndingSplitScrolls:
    jsr EndingHoldTimer
    stz a:ScrollYHi+6
    inc a:ScrollYHi+1
    inc a:ScrollYHi+2
    lda #$6F
    sta a:ScrollYLo+2
    stz a:ScrollXHi+2
    stz a:ScrollXLo+2
    lda #$20
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8+1
    inc a:OBJ_27A8,x
    rts

; @routine 010D:5E2 EndingScrollsOn
; @run 6B:85E2
; WHAT IT DOES: An object handler: once its hold has run out counts scroll slot 6's Y high byte and slot 2's X high byte up,
; counts object 6's state and moves this object to its next state.
; HOW: EndingHoldTimer; four counts.
; WAITS: no.
; Entry: X the object, A as the dispatch left it. Exit: A kept, N and Z from the new state; C and the registers kept.
EndingScrollsOn:
    jsr EndingHoldTimer
    inc a:ScrollYHi+6
    inc a:ScrollXHi+2
    inc a:OBJ_27A8+6
    inc a:OBJ_27A8,x
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
; Entry: X the object, the caller's frame on the stack. Exit: N and Z from the timer (it returns the timer), Z set when
; it has run out; A, X, Y and C kept.
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
; @run 6B:88EF
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
; @run 6B:89F6
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
