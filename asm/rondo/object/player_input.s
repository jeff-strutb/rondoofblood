; @routine 01BB:611 RichterReadPad
; @run 6D:B611
; WHAT IT DOES: Reads the pad for the frame: what is held, and what is newly pressed. Bit 0
; counts as pressed only when held for three frames after being up, which steadies it.
; HOW: The last four frames of the pad are kept in PadHistory.
; Entry: nothing. Exit: A the new presses, N and Z from them. X, Y, C and V kept. Clobbers
; zTemp00.
RichterReadPad:
    lda z:zPadHeld
    sta z:zPadPrevious
    lda a:PadHistory+2
    sta a:PadHistory+3
    lda a:PadHistory+1
    sta a:PadHistory+2
    lda a:PadHistory
    sta a:PadHistory+1
    lda a:BiosPadState
    sta a:PadHistory
    sta z:zPadHeld
    eor a:PadHistory+3
    and a:PadHistory+2
    and a:PadHistory+1
    and a:PadHistory
    and #$01
    sta z:zTemp00
    lda z:zPadHeld
    eor z:zPadPrevious
    and z:zPadHeld
    and #$FE
    ora z:zTemp00
    sta z:zPadPressed
    rts

; @routine 01BB:64C RichterClearPadOverrides
; @run 6D:B64C
; WHAT IT DOES: Clears the four bytes that force the pad.
; HOW: Stores zero.
; Entry: nothing. Exit: all registers and flags kept.
RichterClearPadOverrides:
    stz a:PadOverrides
    stz a:PadOverrides+1
    stz a:PadOverrides+2
    stz a:PadOverrides+3
    rts

; @routine 01BB:669 RichterAnimationTick
; @run 6D:B669
; WHAT IT DOES: Counts down the frame on show; at zero moves to the next step of the list.
; A timer of $FF means the animation has stopped.
; HOW: The timer in OBJ_2F90, the step in OBJ_FLAGS_2F38, the frame shown in OBJ_2A68.
; Entry: nothing. Exit: A the frame or $FF; Y twice the step, plus one with a frame; C and N, Z
; from what was read. X and V kept. Clobbers zTemp00-01.
RichterAnimationTick:
    lda a:OBJ_2F90
    cmp #$FF
    bne RichterAnimationTick_at0
    rts
RichterAnimationTick_at0:
    dec a:OBJ_2F90
    bne RichterSetAnimation_at0
    inc a:OBJ_FLAGS_2F38
    bra RichterSetAnimation_at1

; @routine 01BB:67B RichterSetAnimation
; @run 6D:B67B
; WHAT IT DOES: Starts an animation from its first step.
; HOW: The animation into OBJ_FIELDS_2EE0, step zero, and its first entry read.
; Entry: A animation. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
RichterSetAnimation:
    sta a:OBJ_FIELDS_2EE0
RichterSetAnimation_at4:
    stz a:OBJ_FLAGS_2F38
RichterSetAnimation_at1:
    bsr RichterSetAnimation_at2
    lda (zTemp00),y
    sta a:OBJ_2F90
    cmp #$FF
    bne RichterSetAnimation_at3
    rts
RichterSetAnimation_at3:
    cmp #$FE
    beq RichterSetAnimation_at4
    bra RichterSetAnimation_at5
RichterSetAnimation_at0:
    bsr RichterSetAnimation_at2
RichterSetAnimation_at5:
    iny
    lda (zTemp00),y
    sta a:OBJ_2A68
    rts
RichterSetAnimation_at2:
    lda a:OBJ_FIELDS_2EE0
    asl a
    tay
    lda a:AnimationLists,y
    sta z:zTemp00
    lda a:AnimationLists+1,y
    sta z:zTemp01
    lda a:OBJ_FLAGS_2F38
    asl a
    tay
    rts

; @routine 01BB:704 RichterCheckTurnWindow
; @run 6D:B704
; WHAT IT DOES: Says whether the state in play lets him turn at the screen's middle: a table
; by state, in the first step only, and only on the side of the middle he faces away from.
; HOW: PlayerByte3BD7 is the answer.
; Entry: nothing. Exit: A the last read, N and Z from it; X the state if it went that far. Y,
; C and V kept.
RichterCheckTurnWindow:
    stz a:PlayerByte3BD7
    lda a:OBJ_2800
    beq RichterCheckTurnWindow_at0
    rts
RichterCheckTurnWindow_at0:
    ldx a:OBJ_27A8
    lda a:RichterTurnStates,x
    sta a:PlayerByte3BD7
    lda a:OBJ_2AC0
    bne RichterCheckTurnWindow_at1
    lda a:OBJ_X_LO
    bmi RichterCheckTurnWindow_at2
    stz a:PlayerByte3BD7
    rts
RichterCheckTurnWindow_at1:
    lda a:OBJ_X_LO
    bpl RichterCheckTurnWindow_at2
    stz a:PlayerByte3BD7
RichterCheckTurnWindow_at2:
    rts

; @routine 01BB:7CA RichterFaceByPad
; @run 6D:B7CA
; WHAT IT DOES: Turns him to face the way the pad holds, left or right; with neither, as he is.
; HOW: Bits 7 and 5 of zPadHeld, and facing in OBJ_2AC0: 0 for right, 1 for left.
; Entry: nothing. Exit: A the bits looked at last; N and Z from them, or from the facing when it
; turns left. X, Y, C and V kept.
RichterFaceByPad:
    lda z:zPadHeld
    and #$A0
    bne RichterFaceByPad_at0
    rts
RichterFaceByPad_at0:
    stz a:OBJ_2AC0
    and #$20
    beq RichterFaceByPad_at1
    rts
RichterFaceByPad_at1:
    inc a:OBJ_2AC0
    rts

; @routine 01BB:7DD RichterNoteItemPress
; @run 6D:B7DD
; WHAT IT DOES: Notes that the item button was pressed this frame.
; HOW: Bit 2 of zPadPressed sets RichterItemRequest to 4.
; Entry: nothing. Exit: Z from the test, N and V bits 7 and 6 of the presses; A 4 if pressed.
; X, Y and C kept.
RichterNoteItemPress:
    tst #$04,z:zPadPressed
    bne RichterNoteItemPress_at0
    rts
RichterNoteItemPress_at0:
    lda #$04
    sta a:PadForcedPress
    rts

; @routine 01BB:7E9 RichterCheckItemHearts
; @run 6D:B7E9
; WHAT IT DOES: Says whether he has the hearts the item in hand costs.
; HOW: zHearts against a table by item, in BCD; RichterItemReady is $0F if so, else 0.
; Entry: nothing. Exit: X the item; A the hearts, N, Z and C from the compare, or A $0F. Y and
; V kept.
RichterCheckItemHearts:
    stz a:RichterItemReady
    ldx z:zField93
    lda z:zField94
    cmp a:RichterItemCosts,x
    bcs RichterCheckItemHearts_at0
    rts
RichterCheckItemHearts_at0:
    lda #$0F
    sta a:RichterItemReady
    rts

; @routine 01BB:909 RichterLatchButtons
; @run 6D:B909
; WHAT IT DOES: Keeps the jump and attack presses until a state acts on them.
; HOW: Bits 1 and 0 of zPadPressed ORed into RichterButtonsLatched.
; Entry: nothing. Exit: A what is latched, N and Z from it. X, Y, C and V kept.
RichterLatchButtons:
    lda z:zPadPressed
    and #$02
    bsr RichterLatchButtons_at0
    lda z:zPadPressed
    and #$01
RichterLatchButtons_at0:
    ora a:PadForcedPressB
    sta a:PadForcedPressB
    rts
