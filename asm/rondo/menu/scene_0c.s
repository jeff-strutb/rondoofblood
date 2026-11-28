; @routine 20BC:00B Scene0CBegin
; @run 6B:800B
; WHAT IT DOES: Starts the scene of $0C and $0D: bit 2 of ProgressCharacter, the player's health $5C, the script of the character from $8031.
; HOW: ResetGameState; an ORA into ProgressCharacter; the script pointer, the frame counter $FFFF, the sub-mode up.
; WAITS: yes, through ResetGameState (the vertical blank).
; Entry: X the doubled sub-mode and Y kept. Exit: A $FF, X 0, N and Z from the new state; Y and C from the character
; where the script is picked by it, else Y $FF and C kept.
Scene0CTickCopy = $803F ; the engine's routine at this place, whose bytes another overlay's C serves
Scene0CAdvanceCopy = $8057 ; the engine's routine at this place, whose bytes another overlay's C serves
Scene0CCommands = $8081 ; script command handlers
Scene0CClearCopy = $8162 ; the engine's routine at this place, whose bytes another overlay's C serves
Scene0CPlaceCopy = $816C ; the engine's routine at this place, whose bytes another overlay's C serves
Scene0CKinds = $822A ; object handlers by kind
Scene0CSpeedsXLo = $82D5 ; slide speeds by start argument
Scene0CSpeedsXHi = $82DA ; slide speeds by start argument
Scene0CSpeedsYLo = $82DF ; slide speeds by start argument
Scene0CSpeedsYHi = $82E4 ; slide speeds by start argument
Scene0CFrameLists = $82F8 ; frame list pointers
Scene0CBegin:
    jsr VecResetGameState
    lda a:ProgressCharacter
    ora #$04
    sta a:ProgressCharacter
    lda #$5C
    sta z:zPlayerHealth
    lda z:zCharacter
    asl a
    tay
    lda a:TitleSteps+3,y
    sta z:zScriptPtr
    lda a:TitleSteps+4,y
    sta z:zScriptPtrHi
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    rts

; @routine 20BC:035 Scene0CFrame
; @run 6B:8035
; WHAT IT DOES: As Scene5CFrame (25D3:01F), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CFrame.
; Entry, exit: as Scene5CFrame.
Scene0CFrame:
    jsr Scene0CTickCopy
    jsr Scene0CScriptStep
    jsr Scene0CObjectsStep
    rts

; @routine 20BC:066 Scene0CScriptStep
; @run 6B:8066
; WHAT IT DOES: As Scene5CScriptStep (25D3:050), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CScriptStep.
; Entry, exit: as Scene5CScriptStep.
Scene0CScriptStep:
    ldy #$01
    lda (zScriptPtr),y
    cmp z:zWaitTimer3
    bne Scene0CFrame+9
    iny
    lda (zScriptPtr),y
    cmp z:zWaitTimer4
    bne Scene0CFrame+9
    lda (zScriptPtr)
    asl a
    tax
    ldy #$03
    jmp (Scene0CCommands,x)

; @routine 20BC:0BB Scene0CCommandStartListAndClear
; @run 6B:80BB
; WHAT IT DOES: As Scene5CCommandStartListAndClear (25D3:0A5), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandStartListAndClear.
; Entry, exit: as Scene5CCommandStartListAndClear.
Scene0CCommandStartListAndClear:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    jsr VecClearScreenState
    jsr Scene0CClearCopy
    tai KernelZeros,ScrollXFrac,$00EE
    bra Scene0CAdvanceCopy

; @routine 20BC:0D6 Scene0CCommandStartScript1
; @run 6B:80D6
; WHAT IT DOES: As Scene5CCommandStartScript1 (25D3:0C0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandStartScript1.
; Entry, exit: as Scene5CCommandStartScript1.
Scene0CCommandStartScript1:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind1
    jmp Scene0CAdvanceCopy

; @routine 20BC:0DE Scene0CCommandStartScript2
; @run 6B:80DE
; WHAT IT DOES: As Scene5CCommandStartScript2 (25D3:0C8), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandStartScript2.
; Entry, exit: as Scene5CCommandStartScript2.
Scene0CCommandStartScript2:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind2
    jmp Scene0CAdvanceCopy

; @routine 20BC:0E6 Scene0CCommandStartScript3
; @run 6B:80E6
; WHAT IT DOES: As Scene5CCommandStartScript3 (25D3:0D0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandStartScript3.
; Entry, exit: as Scene5CCommandStartScript3.
Scene0CCommandStartScript3:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind3
    jmp Scene0CAdvanceCopy

; @routine 20BC:0EE Scene0CCommandPlayAdpcm
; @run 6B:80EE
; WHAT IT DOES: As Scene5CCommandPlayAdpcm (25D3:0D8), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandPlayAdpcm.
; Entry, exit: as Scene5CCommandPlayAdpcm.
Scene0CCommandPlayAdpcm:
    lda (zScriptPtr),y
    jsr VecAdpcmPlayFromDisc
    jmp Scene0CAdvanceCopy

; @routine 20BC:0F6 Scene0CCommandCdPlay
; @run 6B:80F6
; WHAT IT DOES: As Scene5CCommandCdPlay (25D3:0E0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandCdPlay.
; Entry, exit: as Scene5CCommandCdPlay.
Scene0CCommandCdPlay:
    lda (zScriptPtr),y
    jsr VecCdPlayOnce
    jmp Scene0CAdvanceCopy

; @routine 20BC:0FE Scene0CCommandPlaceKind
; @run 6B:80FE
; WHAT IT DOES: As Scene5CCommandPlaceKind (25D3:0E8), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandPlaceKind.
; Entry, exit: as Scene5CCommandPlaceKind.
Scene0CCommandPlaceKind:
    lda (zScriptPtr),y
    tax
    jsr Scene0CPlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_KIND,x
    jmp Scene0CAdvanceCopy

; @routine 20BC:10D Scene0CCommandPlaceNext
; @run 6B:810D
; WHAT IT DOES: As Scene5CCommandPlaceNext (25D3:0F7), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandPlaceNext.
; Entry, exit: as Scene5CCommandPlaceNext.
Scene0CCommandPlaceNext:
    lda (zScriptPtr),y
    tax
    jsr Scene0CPlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    lda #$02
    sta a:OBJ_KIND,x
    jmp Scene0CAdvanceCopy

; @routine 20BC:127 Scene0CCommandPlaceFrame
; @run 6B:8127
; WHAT IT DOES: As Scene5CCommandPlaceFrame (25D3:111), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandPlaceFrame.
; Entry, exit: as Scene5CCommandPlaceFrame.
Scene0CCommandPlaceFrame:
    lda (zScriptPtr),y
    tax
    jsr Scene0CPlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FRAME,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    jmp Scene0CAdvanceCopy

; @routine 20BC:13C Scene0CCommandPlaceTimed
; @run 6B:813C
; WHAT IT DOES: As Scene5CCommandPlaceTimed (25D3:126), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandPlaceTimed.
; Entry, exit: as Scene5CCommandPlaceTimed.
Scene0CCommandPlaceTimed:
    lda (zScriptPtr),y
    tax
    jsr Scene0CPlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FRAME,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FLASH_FRAMES,x
    lda #$03
    sta a:OBJ_KIND,x
    jmp Scene0CAdvanceCopy

; @routine 20BC:188 Scene0CCommandPlaceTitleObjectA
; @run 6B:8188
; WHAT IT DOES: As Scene5CCommandPlaceTitleObjectA (25D3:172), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandPlaceTitleObjectA.
; Entry, exit: as Scene5CCommandPlaceTitleObjectA.
Scene0CCommandPlaceTitleObjectA:
    ldx #$2E
    jsr VecClearObject
    inc a:OBJ_KIND,x
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FLASH_FRAMES,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FLYER_WAIT,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_CHILD_ORDER,x
    lda #$C8
    sta a:OBJ_Y_LO,x
    jmp Scene0CAdvanceCopy

; @routine 20BC:1AF Scene0CCommandPlaceTitleObjectB
; @run 6B:81AF
; WHAT IT DOES: As Scene5CCommandPlaceTitleObjectB (25D3:199), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandPlaceTitleObjectB.
; Entry, exit: as Scene5CCommandPlaceTitleObjectB.
Scene0CCommandPlaceTitleObjectB:
    ldx #$2F
    jsr VecClearObject
    inc a:OBJ_KIND,x
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FLASH_FRAMES,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FLYER_WAIT,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_CHILD_ORDER,x
    lda #$D8
    sta a:OBJ_Y_LO,x
    jmp Scene0CAdvanceCopy

; @routine 20BC:1D6 Scene0CCommandLoadBanks
; @run 6B:81D6
; WHAT IT DOES: As Scene5CCommandLoadBanks (25D3:1C0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CCommandLoadBanks.
; Entry, exit: as Scene5CCommandLoadBanks.
Scene0CCommandLoadBanks:
    stz z:zTemp00
Scene0CCommandLoadBanks_at0:
    lda z:zTemp00
    asl a
    clc
    adc #$10
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    stz z:zBiosCl
    lda z:zTemp00
    clc
    ror a
    ror a
    ror a
    sta z:zBiosCh
    stz z:zBiosDh
    stz z:zBiosBl
    lda #$A0
    sta z:zBiosBh
    stz z:zBiosAl
    lda #$40
    sta z:zBiosAh
    jsr BiosAdRead
    inc z:zTemp00
    lda z:zTemp00
    cmp #$04
    bne Scene0CCommandLoadBanks_at0
    jmp Scene0CAdvanceCopy

; @routine 20BC:20B Scene0CObjectsStep
; @run 6B:820B
; WHAT IT DOES: As Scene5CObjectsStep (25D3:1F5), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CObjectsStep.
; Entry, exit: as Scene5CObjectsStep.
Scene0CObjectsStep:
    clx
Scene0CObjectsStep_t820C:
    lda a:OBJ_KIND,x
    beq Scene0CObjectsStep_at0
    jsr Scene0CDispatchKind
Scene0CObjectsStep_at0:
    inx
    cpx #$58
    bne Scene0CObjectsStep_t820C
    rts

; @routine 20BC:21A Scene0CDispatchKind
; @run 6B:821A
; WHAT IT DOES: As Scene5CDispatchKind (25D3:204), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($16).
; WAITS: as Scene5CDispatchKind.
; Entry, exit: as Scene5CDispatchKind.
Scene0CDispatchKind:
    dec a
    asl a
    tay
    lda a:Scene0CKinds,y
    sta z:zTemp00
    lda a:Scene0CKinds+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 20BC:232 Scene0CDriftThree
; @run 6B:8232
; WHAT IT DOES: Waits 33 frames, then moves objects $11, 2 and 3 left by two and up by one a frame.
; HOW: state 0 sets work_d; state 1 counts it down, reloading below zero; three 16-bit SBCs and DECs.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0CDriftThree:
    lda a:OBJ_STATE,x
    bne Scene0CDriftThree_t8240
    lda #$20
    sta a:OBJ_FLASH_FRAMES,x
Scene0CDriftThree_at1:
    inc a:OBJ_STATE,x
Scene0CDriftThree_at0:
    rts
Scene0CDriftThree_t8240:
    dec a
    bne Scene0CDriftThree_at0
    dec a:OBJ_FLASH_FRAMES,x
    bmi Scene0CDriftThree_at1
    lda a:OBJ_X_LO+17
    sec
    sbc #$02
    sta a:OBJ_X_LO+17
    lda a:OBJ_X_HI+17
    sbc #$00
    sta a:OBJ_X_HI+17
    dec a:OBJ_Y_LO+17
    lda a:OBJ_X_LO+2
    sec
    sbc #$02
    sta a:OBJ_X_LO+2
    lda a:OBJ_X_HI+2
    sbc #$00
    sta a:OBJ_X_HI+2
    dec a:OBJ_Y_LO+2
    lda a:OBJ_X_LO+3
    sec
    sbc #$02
    sta a:OBJ_X_LO+3
    lda a:OBJ_X_HI+3
    sbc #$00
    sta a:OBJ_X_HI+3
    dec a:OBJ_Y_LO+3
    rts

; @routine 20BC:28F Scene0CPlayListOnce
; @run 6B:828F
; WHAT IT DOES: Plays the start argument's frame list once.
; HOW: ScenePlayListOnce with this overlay's lists.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0CPlayListOnce:
    lda a:OBJ_STATE,x
    bne Scene0CPlayListOnce_t82A4
    lda a:OBJ_START_ARG,x
    jsr Scene0CFrameList
    bne Scene0CPlayListOnce_t82A4
    lda a:OBJ_START_ARG,x
    bmi Scene0CPlayListOnce_t82A4
    inc a:OBJ_STATE,x
Scene0CPlayListOnce_t82A4:
    rts

; @routine 20BC:2A5 Scene0CSlideForCount
; @run 6B:82A5
; WHAT IT DOES: As the $5C overlay's slide, with this overlay's speed tables.
; HOW: SceneSlide.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0CSlideForCount:
    lda a:OBJ_STATE,x
    bne Scene0CSlideForCount_at0
    ldy a:OBJ_START_ARG,x
    lda a:Scene0CSpeedsXLo,y
    sta a:OBJ_VX_LO,x
    lda a:Scene0CSpeedsXHi,y
    sta a:OBJ_VX_HI,x
    lda a:Scene0CSpeedsYLo,y
    sta a:OBJ_VY_LO,x
    lda a:Scene0CSpeedsYHi,y
    sta a:OBJ_VY_HI,x
Scene0CSlideForCount_at2:
    inc a:OBJ_STATE,x
    rts
Scene0CSlideForCount_at0:
    dec a
    bne Scene0CSlideForCount_at1
    jsr VecApplyVelocity
    dec a:OBJ_FLASH_FRAMES,x
    beq Scene0CSlideForCount_at2
Scene0CSlideForCount_at1:
    rts

; @routine 20BC:2E9 Scene0CFrameList
; @run 6B:82E9
; WHAT IT DOES: Steps an object through the frame list this overlay's table names for A.
; HOW: ObjectFrameListAt.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames.
Scene0CFrameList:
    asl a
    tay
    lda a:Scene0CFrameLists,y
    sta z:zTemp10
    lda a:Scene0CFrameLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

