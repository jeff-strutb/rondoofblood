; @routine 01BD:7C9 ObjectTouchesPlayerBox
; @run 6E:C7C9
; WHAT IT DOES: Says whether the object lies within a box around the player: 8 either side of his X low byte and, from
; a point his Y low plus the box offset OBJ_HITBOX_DY, $14 either side. The answer is 0 for in and 1 for out.
; HOW: The first screen only (both high bytes zero); two add and compare windows of $10 and $28.
; WAITS: no.
; Entry: X object. Exit: A 0 or 1, N and Z from it, C and V from the last window's add and compare. Clobbers zTemp00.
ObjectTouchesPlayerBox:
    lda a:OBJ_Y_LO
    clc
    adc a:OBJ_HITBOX_DY
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    bne ObjectTouchesPlayerBox_at0
    lda a:OBJ_X_LO,x
    sec
    sbc a:OBJ_X_LO
    clc
    adc #$08
    cmp #$10
    bcs ObjectTouchesPlayerBox_at0
    lda a:OBJ_Y_HI,x
    bne ObjectTouchesPlayerBox_at0
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp00
    clc
    adc #$14
    cmp #$28
    bcs ObjectTouchesPlayerBox_at0
    lda #$00
    rts
ObjectTouchesPlayerBox_at0:
    lda #$01
    rts

; @routine 01BE:4E6 NudgeAngleByVelocityAndScroll
; @run 6E:CCE6
; WHAT IT DOES: Moves the object's angle pair OBJ_WORK_F and OBJ_WORK_G (and the counter OBJ_WORK_H): by two tables when it
; moves sideways (by its X speed's sign), then by the scroll speed ScrollSpeedX negated, the table at $CE79 by its sign.
; HOW: Chains of 8-bit adds with the carry carried from one to the next.
; WAITS: no.
; Entry: X object. Exit: A the angle low byte stored, N and Z from it, C and V from its add. X kept, Y the sign index.
; Tables at $CD2D, $CD2F and $CE79 have no rows yet (the old address form).
NudgeAngleByVelocityAndScroll:
    lda a:OBJ_VX_HI,x
    ora a:OBJ_VX_LO,x
    beq NudgeAngleByVelocityAndScroll_at0
    lda a:OBJ_VX_HI,x
    asl a
    rol a
    and #$01
    tay
    lda a:$CD2D,y
    clc
    adc a:OBJ_WORK_H,x
    sta a:OBJ_WORK_H,x
    lda a:$CD2F,y
    adc a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,x
    lda a:$CD2F,y
    adc a:OBJ_WORK_G,x
    sta a:OBJ_WORK_G,x
NudgeAngleByVelocityAndScroll_at0:
    cly
    lda a:ScrollSpeedX
    eor #$FF
    inc a
    bpl NudgeAngleByVelocityAndScroll_at1
    iny
NudgeAngleByVelocityAndScroll_at1:
    clc
    adc a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,x
    lda a:$CE79,y
    adc a:OBJ_WORK_G,x
    sta a:OBJ_WORK_G,x
    rts

; @routine 0720:567 WaitForField85Two
; @run 6B:9D67
; WHAT IT DOES: Moves the object on to its next state when zField85 is 2.
; HOW: CMP #2; equal increments the state.
; WAITS: no.
; Entry: X object. Exit: A zField85; N, Z, C from the compare, or N and Z from the new state. X, Y kept.
WaitForField85Two:
    lda z:zField85
    cmp #$02
    bne NextStateByKindBits_at0
    inc a:OBJ_STATE,x
    rts

; @routine 0720:571 StepFlashTimer
; @run 6B:9D71
; WHAT IT DOES: Counts the flash's frame timer down; at zero steps its list counter, and at four steps ends the flash
; (next state, bit 7 of zField8C, sound $71); before that it starts the next list.
; HOW: DEC of the timer, INC of OBJ_WORK_E read again for the CMP, flash_restart.
; WAITS: no.
; Entry: X object, Y kept. Exit: timer running: N and Z from it. Else as the flash's restart (A 2, N and Z the object's
; slot, C and V from the second add) or the sound. The end of the flash (four steps) is untraced (4 of 400 cases reach it
; and the draft stops there): written from the disc's bytes.
StepFlashTimer:
    dec a:OBJ_WORK_D,x
    bne StepFlashTimer_at0
    inc a:OBJ_WORK_E,x
    lda a:OBJ_WORK_E,x
    cmp #$04
    bne StepFlashTimer_at1
    inc a:OBJ_STATE,x
    lda z:zField8C
    ora #$80
    sta z:zField8C
    lda #$71
    jmp VecSoundCommand
StepFlashTimer_at1:
    lda #$04
    sta a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:$9CFD,y
    clc
    adc a:OBJ_WORK_E,x
    sta a:OBJ_FRAME,x
    clc
    adc a:$9CFF,y
    phx
    ldx #$05
    jsr VecStartListChannelKind2
    plx
StepFlashTimer_at0:
    rts

; @routine 0720:5AF NextStateByKindBits
; @run 6B:9DAF
; WHAT IT DOES: While zField8C bit 7 is clear, takes the object to the next state, sets its frame timer to 4, and
; takes it one state further when the low nibble of its next kind is 2 to 5, 10 or 11.
; HOW: INC of the state, one store, then compares of the nibble.
; WAITS: no.
; Entry: X object. Exit: A the last value loaded or compared, N, Z, C from the compare or N and Z from the last INC.
; The arms for the nibbles 6 to 11 and the second INC are untraced (89 of 400 cases stop the draft): written from the
; disc's bytes.
NextStateByKindBits:
    lda z:zField8C
    bmi NextStateByKindBits_at0
    inc a:OBJ_STATE,x
    lda #$04
    sta a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    and #$0F
    cmp #$02
    bcc NextStateByKindBits_at1
    cmp #$06
    bcc NextStateByKindBits_at2
    cmp #$0A
    beq NextStateByKindBits_at2
    cmp #$0B
    beq NextStateByKindBits_at2
NextStateByKindBits_at1:
    bra NextStateByKindBits_at0
NextStateByKindBits_at2:
    inc a:OBJ_STATE,x
NextStateByKindBits_at0:
    rts

; @routine 0720:5D6 StepFlashTimerDown
; @run 6B:9DD6
; WHAT IT DOES: The flash's other step: counts its frame timer down; at zero counts the list counter down; below zero
; the flash ends (next state, sound $71), else the next list starts.
; HOW: DEC of the timer, DEC of OBJ_WORK_E, flash_restart.
; WAITS: no.
; Entry: X object, Y kept. Exit: as StepFlashTimer.
StepFlashTimerDown:
    dec a:OBJ_WORK_D,x
    bne StepFlashTimerDown_at0
    dec a:OBJ_WORK_E,x
    bpl StepFlashTimerDown_at1
    inc a:OBJ_STATE,x
    lda #$71
    jmp VecSoundCommand
StepFlashTimerDown_at1:
    lda #$04
    sta a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:$9CFD,y
    clc
    adc a:OBJ_WORK_E,x
    sta a:OBJ_FRAME,x
    clc
    adc a:$9CFF,y
    phx
    ldx #$05
    jsr VecStartListChannelKind2
    plx
StepFlashTimerDown_at0:
    rts
