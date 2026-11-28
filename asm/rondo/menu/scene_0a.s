; @routine 23F6:00B Scene0ABegin
; @run 6B:800B
; WHAT IT DOES: Starts the scene of $0A and $0B: bit 1 of ProgressCharacter, the script of the character from $802D.
; HOW: ResetGameState; an ORA into ProgressCharacter; the script pointer, the frame counter $FFFF, the sub-mode up.
; WAITS: yes, through ResetGameState (the vertical blank).
; Entry: X the doubled sub-mode and Y kept. Exit: A $FF, X 0, N and Z from the new state; Y and C from the character
; where the script is picked by it, else Y $FF and C kept.
Scene0AJumpTableA = $8370 ; a state jump table
Scene0ASpeedsXLo = $83EC ; slide speeds by start argument
Scene0ASpeedsXHi = $83F1 ; slide speeds by start argument
Scene0ASpeedsYLo = $83F6 ; slide speeds by start argument
Scene0ASpeedsYHi = $83FB ; slide speeds by start argument
Scene0AFrameLists = $840F ; frame list pointers
Scene0ABegin:
    jsr VecResetGameState
    lda a:ProgressCharacter
    ora #$02
    sta a:ProgressCharacter
    lda z:zCharacter
    asl a
    tay
    lda a:EndingScripts1E61,y
    sta z:zScriptPtr
    lda a:TitleSteps,y
    sta z:zScriptPtrHi
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    rts

; @routine 23F6:234 Scene0AOrbit
; @run 6B:8234
; WHAT IT DOES: Shows an object, swings it on a growing circle from ($CE, $B1), then after pauses flies it off.
; HOW: States 0 to 3 on SetVelocityFromAngle and ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0AOrbit:
    lda a:OBJ_STATE,x
    bne Scene0AOrbit_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$0D
    sta a:OBJ_FRAME,x
    lda #$60
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
    rts
Scene0AOrbit_at0:
    dec a
    bne Scene0AOrbit_at1
    lda #$5F
    sta z:zTemp01
    stz z:zTemp00
    inc a:OBJ_FLASH_FRAMES,x
    lda a:OBJ_FLASH_FRAMES,x
    jsr VecSetVelocityFromAngle
    lda #$CE
    clc
    adc a:OBJ_VX_HI,x
    sta a:OBJ_X_LO,x
    lda #$B1
    clc
    adc a:OBJ_VY_HI,x
    sta a:OBJ_Y_LO,x
    lda a:OBJ_FLASH_FRAMES,x
    bpl Scene0AOrbit_at2
    lda #$08
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
Scene0AOrbit_at2:
    rts
Scene0AOrbit_at1:
    dec a
    bne Scene0AOrbit_t829F
    dec a:OBJ_FLASH_FRAMES,x
    bne Scene0AOrbit_at2
    lda #$08
    sta a:OBJ_FLASH_FRAMES,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_LO,x
    lda #$FC
    sta a:OBJ_VX_HI,x
    lda #$02
    sta a:OBJ_VY_HI,x
    inc a:OBJ_STATE,x
    rts
Scene0AOrbit_t829F:
    dec a
    bne Scene0AOrbit_at2
    jsr VecApplyVelocity
    dec a:OBJ_FLASH_FRAMES,x
    bne Scene0AOrbit_at2
    inc a:OBJ_STATE,x
    rts

; @routine 23F6:2AE Scene0ASwoop
; @run 6B:82AE
; WHAT IT DOES: Shows an object and swoops it on a circle from ($90, $FF) one way, then, after a pause, back the other, until its angle count is $98.
; HOW: States 0 to 3 on SetVelocityFromAngle; the Y place sixteen bits signed.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0ASwoop:
    lda a:OBJ_STATE,x
    bne Scene0ASwoop_t82C6
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$12
    sta a:OBJ_FRAME,x
    lda #$80
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
    rts
Scene0ASwoop_t82C6:
    dec a
    bne Scene0ASwoop_t8309
    lda #$4F
    sta z:zTemp01
    stz z:zTemp00
    inc a:OBJ_FLASH_FRAMES,x
    lda a:OBJ_FLASH_FRAMES,x
    jsr VecSetVelocityFromAngle
    lda #$90
    clc
    adc a:OBJ_VX_HI,x
    sta a:OBJ_X_LO,x
    stz z:zTemp00
    lda a:OBJ_VY_HI,x
    bpl Scene0ASwoop_at0
    dec z:zTemp00
Scene0ASwoop_at0:
    lda #$FF
    clc
    adc a:OBJ_VY_HI,x
    sta a:OBJ_Y_LO,x
    cla
    adc z:zTemp00
    sta a:OBJ_Y_HI,x
    lda a:OBJ_FLASH_FRAMES,x
    cmp #$B0
    bne Scene0ASwoop_at1
    lda #$01
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
Scene0ASwoop_at1:
    rts
Scene0ASwoop_t8309:
    dec a
    bne Scene0ASwoop_at2
    dec a:OBJ_FLASH_FRAMES,x
    bne Scene0ASwoop_at1
    lda #$B0
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
    rts
Scene0ASwoop_at2:
    dec a
    bne Scene0ASwoop_at1
    lda #$4F
    sta z:zTemp01
    stz z:zTemp00
    dec a:OBJ_FLASH_FRAMES,x
    dec a:OBJ_FLASH_FRAMES,x
    dec a:OBJ_FLASH_FRAMES,x
    lda a:OBJ_FLASH_FRAMES,x
    jsr VecSetVelocityFromAngle
    lda #$90
    clc
    adc a:OBJ_VX_HI,x
    sta a:OBJ_X_LO,x
    stz z:zTemp00
    lda a:OBJ_VY_HI,x
    bpl Scene0ASwoop_at3
    dec z:zTemp00
Scene0ASwoop_at3:
    lda #$FF
    clc
    adc a:OBJ_VY_HI,x
    sta a:OBJ_Y_LO,x
    cla
    adc z:zTemp00
    sta a:OBJ_Y_HI,x
    lda a:OBJ_FLASH_FRAMES,x
    cmp #$98
    bne Scene0ASwoop_at1
    inc a:OBJ_STATE,x
    rts

; @routine 23F6:35E Scene0ASteps
; @run 6B:835E
; @stop 8370
; WHAT IT DOES: Goes to the step its state names.
; HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0ASteps:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:Scene0AJumpTableA,y
    sta z:zTemp00
    lda a:Scene0AJumpTableA+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 23F6:376 Scene0AScrollStart
; @run 6B:8376
; WHAT IT DOES: Puts the scroll's X low at $80 and goes on.
; HOW: Two stores.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0AScrollStart:
    lda #$80
    sta a:ScrollXLo
    inc a:OBJ_STATE,x
    rts

; @routine 23F6:37F Scene0AScrollRight
; @run 6B:837F
; WHAT IT DOES: Scrolls X right by a dot and a half a frame; on the carry out of the low byte the low byte goes to zero, the high byte up, and the object goes on.
; HOW: ADC of $80 to the fraction and 1 to the low byte.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0AScrollRight:
    lda #$80
    clc
    adc a:ScrollXFrac
    sta a:ScrollXFrac
    lda #$01
    adc a:ScrollXLo
    sta a:ScrollXLo
    bcc Scene0AScrollRight_at0
    stz a:ScrollXLo
    inc a:ScrollXHi
    inc a:OBJ_STATE,x
Scene0AScrollRight_at0:
    rts

; @routine 23F6:3A6 Scene0APlayListOnce
; @run 6B:83A6
; WHAT IT DOES: Plays the start argument's frame list once.
; HOW: ScenePlayListOnce with this overlay's lists.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0APlayListOnce:
    lda a:OBJ_STATE,x
    bne Scene0APlayListOnce_at0
    lda a:OBJ_START_ARG,x
    jsr Scene0AFrameList
    bne Scene0APlayListOnce_at0
    lda a:OBJ_START_ARG,x
    bmi Scene0APlayListOnce_at0
    inc a:OBJ_STATE,x
Scene0APlayListOnce_at0:
    rts

; @routine 23F6:3BC Scene0ASlideForCount
; @run 6B:83BC
; WHAT IT DOES: As the $5C overlay's slide, with this overlay's speed tables.
; HOW: SceneSlide.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0ASlideForCount:
    lda a:OBJ_STATE,x
    bne Scene0ASlideForCount_at0
    ldy a:OBJ_START_ARG,x
    lda a:Scene0ASpeedsXLo,y
    sta a:OBJ_VX_LO,x
    lda a:Scene0ASpeedsXHi,y
    sta a:OBJ_VX_HI,x
    lda a:Scene0ASpeedsYLo,y
    sta a:OBJ_VY_LO,x
    lda a:Scene0ASpeedsYHi,y
    sta a:OBJ_VY_HI,x
Scene0ASlideForCount_at2:
    inc a:OBJ_STATE,x
    rts
Scene0ASlideForCount_at0:
    dec a
    bne Scene0ASlideForCount_at1
    jsr VecApplyVelocity
    dec a:OBJ_FLASH_FRAMES,x
    beq Scene0ASlideForCount_at2
Scene0ASlideForCount_at1:
    rts

; @routine 23F6:400 Scene0AFrameList
; @run 6B:8400
; WHAT IT DOES: Steps an object through the frame list this overlay's table names for A.
; HOW: ObjectFrameListAt.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames.
Scene0AFrameList:
    asl a
    tay
    lda a:Scene0AFrameLists,y
    sta z:zTemp10
    lda a:Scene0AFrameLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

