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
    inc a:OBJ_27A8,x
    rts

; @routine 0720:571 StepFlashTimer
; @run 6B:9D71
; WHAT IT DOES: Counts the flash's frame timer down; at zero steps its list counter, and at four steps ends the flash
; (next state, bit 7 of zField8C, sound $71); before that it starts the next list.
; HOW: DEC of the timer, INC of OBJ_3510 read again for the CMP, flash_restart.
; WAITS: no.
; Entry: X object, Y kept. Exit: timer running: N and Z from it. Else as the flash's restart (A 2, N and Z the object's
; slot, C and V from the second add) or the sound. The end of the flash (four steps) is untraced (4 of 400 cases reach it
; and the draft stops there): written from the disc's bytes.
StepFlashTimer:
    dec a:OBJ_34B8,x
    bne StepFlashTimer_at0
    inc a:OBJ_3510,x
    lda a:OBJ_3510,x
    cmp #$04
    bne StepFlashTimer_at1
    inc a:OBJ_27A8,x
    lda z:zField8C
    ora #$80
    sta z:zField8C
    lda #$71
    jmp VecSoundCommand
StepFlashTimer_at1:
    lda #$04
    sta a:OBJ_34B8,x
    lda a:OBJ_3040,x
    and #$01
    tay
    lda a:$9CFD,y
    clc
    adc a:OBJ_3510,x
    sta a:OBJ_2A68,x
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
    inc a:OBJ_27A8,x
    lda #$04
    sta a:OBJ_34B8,x
    lda a:OBJ_3040,x
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
    inc a:OBJ_27A8,x
NextStateByKindBits_at0:
    rts

; @routine 0720:5D6 StepFlashTimerDown
; @run 6B:9DD6
; WHAT IT DOES: The flash's other step: counts its frame timer down; at zero counts the list counter down; below zero
; the flash ends (next state, sound $71), else the next list starts.
; HOW: DEC of the timer, DEC of OBJ_3510, flash_restart.
; WAITS: no.
; Entry: X object, Y kept. Exit: as StepFlashTimer.
StepFlashTimerDown:
    dec a:OBJ_34B8,x
    bne StepFlashTimerDown_at0
    dec a:OBJ_3510,x
    bpl StepFlashTimerDown_at1
    inc a:OBJ_27A8,x
    lda #$71
    jmp VecSoundCommand
StepFlashTimerDown_at1:
    lda #$04
    sta a:OBJ_34B8,x
    lda a:OBJ_3040,x
    and #$01
    tay
    lda a:$9CFD,y
    clc
    adc a:OBJ_3510,x
    sta a:OBJ_2A68,x
    clc
    adc a:$9CFF,y
    phx
    ldx #$05
    jsr VecStartListChannelKind2
    plx
StepFlashTimerDown_at0:
    rts
