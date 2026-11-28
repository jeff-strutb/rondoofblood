; @routine 224B:00B Scene02Begin
; @run 6B:800B
; WHAT IT DOES: Starts the scene of $02 and $03: bit 0 of ProgressCharacter, the script at $8383.
; HOW: ResetGameState; an ORA into ProgressCharacter; the script pointer, the frame counter $FFFF, the sub-mode up.
; WAITS: yes, through ResetGameState (the vertical blank).
; Entry: X the doubled sub-mode and Y kept. Exit: A $FF, X 0, N and Z from the new state; Y and C from the character
; where the script is picked by it, else Y $FF and C kept.
Scene02AdvanceCopy = $8049 ; the engine's routine at this place, whose bytes another overlay's C serves
Scene02Commands = $8073 ; script command handlers
Scene02ClearCopy = $8154 ; the engine's routine at this place, whose bytes another overlay's C serves
Scene02PlaceCopy = $815E ; the engine's routine at this place, whose bytes another overlay's C serves
Scene02Kinds = $821C ; object handlers by kind
Scene02JumpTableA = $8238 ; a state jump table
Scene02JumpTableB = $8286 ; a state jump table
Scene02SpeedsXLo = $8324 ; slide speeds by start argument
Scene02SpeedsXHi = $8329 ; slide speeds by start argument
Scene02SpeedsYLo = $832E ; slide speeds by start argument
Scene02SpeedsYHi = $8333 ; slide speeds by start argument
Scene02FrameLists = $8347 ; frame list pointers
Scene02Begin:
    jsr VecResetGameState
    lda a:ProgressCharacter
    ora #$01
    sta a:ProgressCharacter
    lda #$83
    sta z:zScriptPtr
    lda #$83
    sta z:zScriptPtrHi
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    rts

; @routine 224B:027 Scene02Frame
; @run 6B:8027
; WHAT IT DOES: As Scene5CFrame (25D3:01F), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CFrame.
; Entry, exit: as Scene5CFrame.
Scene02Frame:
    jsr Scene0EFrame
    jsr Scene02ScriptStep
    jsr Scene02ObjectsStep
    rts

; @routine 224B:058 Scene02ScriptStep
; @run 6B:8058
; WHAT IT DOES: As Scene5CScriptStep (25D3:050), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CScriptStep.
; Entry, exit: as Scene5CScriptStep.
Scene02ScriptStep:
    ldy #$01
    lda (zScriptPtr),y
    cmp z:zWaitTimer3
    bne Scene02Frame+9
    iny
    lda (zScriptPtr),y
    cmp z:zWaitTimer4
    bne Scene02Frame+9
    lda (zScriptPtr)
    asl a
    tax
    ldy #$03
    jmp (Scene02Commands,x)

; @routine 224B:0AD Scene02CommandStartListAndClear
; @run 6B:80AD
; WHAT IT DOES: As Scene5CCommandStartListAndClear (25D3:0A5), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandStartListAndClear.
; Entry, exit: as Scene5CCommandStartListAndClear.
Scene02CommandStartListAndClear:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    jsr VecClearScreenState
    jsr Scene02ClearCopy
    tai KernelZeros,ScrollXFrac,$00EE
    bra Scene02AdvanceCopy

; @routine 224B:0C8 Scene02CommandStartScript1
; @run 6B:80C8
; WHAT IT DOES: As Scene5CCommandStartScript1 (25D3:0C0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandStartScript1.
; Entry, exit: as Scene5CCommandStartScript1.
Scene02CommandStartScript1:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind1
    jmp Scene02AdvanceCopy

; @routine 224B:0D0 Scene02CommandStartScript2
; @run 6B:80D0
; WHAT IT DOES: As Scene5CCommandStartScript2 (25D3:0C8), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandStartScript2.
; Entry, exit: as Scene5CCommandStartScript2.
Scene02CommandStartScript2:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind2
    jmp Scene02AdvanceCopy

; @routine 224B:0D8 Scene02CommandStartScript3
; @run 6B:80D8
; WHAT IT DOES: As Scene5CCommandStartScript3 (25D3:0D0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandStartScript3.
; Entry, exit: as Scene5CCommandStartScript3.
Scene02CommandStartScript3:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind3
    jmp Scene02AdvanceCopy

; @routine 224B:0E0 Scene02CommandPlayAdpcm
; @run 6B:80E0
; WHAT IT DOES: As Scene5CCommandPlayAdpcm (25D3:0D8), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandPlayAdpcm.
; Entry, exit: as Scene5CCommandPlayAdpcm.
Scene02CommandPlayAdpcm:
    lda (zScriptPtr),y
    jsr VecAdpcmPlayFromDisc
    jmp Scene02AdvanceCopy

; @routine 224B:0E8 Scene02CommandCdPlay
; @run 6B:80E8
; WHAT IT DOES: As Scene5CCommandCdPlay (25D3:0E0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandCdPlay.
; Entry, exit: as Scene5CCommandCdPlay.
Scene02CommandCdPlay:
    lda (zScriptPtr),y
    jsr VecCdPlayOnce
    jmp Scene02AdvanceCopy

; @routine 224B:0F0 Scene02CommandPlaceKind
; @run 6B:80F0
; WHAT IT DOES: As Scene5CCommandPlaceKind (25D3:0E8), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandPlaceKind.
; Entry, exit: as Scene5CCommandPlaceKind.
Scene02CommandPlaceKind:
    lda (zScriptPtr),y
    tax
    jsr Scene02PlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_KIND,x
    jmp Scene02AdvanceCopy

; @routine 224B:0FF Scene02CommandPlaceNext
; @run 6B:80FF
; WHAT IT DOES: As Scene5CCommandPlaceNext (25D3:0F7), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandPlaceNext.
; Entry, exit: as Scene5CCommandPlaceNext.
Scene02CommandPlaceNext:
    lda (zScriptPtr),y
    tax
    jsr Scene02PlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    lda #$02
    sta a:OBJ_KIND,x
    jmp Scene02AdvanceCopy

; @routine 224B:119 Scene02CommandPlaceFrame
; @run 6B:8119
; WHAT IT DOES: As Scene5CCommandPlaceFrame (25D3:111), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandPlaceFrame.
; Entry, exit: as Scene5CCommandPlaceFrame.
Scene02CommandPlaceFrame:
    lda (zScriptPtr),y
    tax
    jsr Scene02PlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FRAME,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    jmp Scene02AdvanceCopy

; @routine 224B:12E Scene02CommandPlaceTimed
; @run 6B:812E
; WHAT IT DOES: As Scene5CCommandPlaceTimed (25D3:126), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandPlaceTimed.
; Entry, exit: as Scene5CCommandPlaceTimed.
Scene02CommandPlaceTimed:
    lda (zScriptPtr),y
    tax
    jsr Scene02PlaceCopy
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
    jmp Scene02AdvanceCopy

; @routine 224B:17A Scene02CommandPlaceTitleObjectA
; @run 6B:817A
; WHAT IT DOES: As Scene5CCommandPlaceTitleObjectA (25D3:172), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandPlaceTitleObjectA.
; Entry, exit: as Scene5CCommandPlaceTitleObjectA.
Scene02CommandPlaceTitleObjectA:
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
    jmp Scene02AdvanceCopy

; @routine 224B:1A1 Scene02CommandPlaceTitleObjectB
; @run 6B:81A1
; WHAT IT DOES: As Scene5CCommandPlaceTitleObjectB (25D3:199), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandPlaceTitleObjectB.
; Entry, exit: as Scene5CCommandPlaceTitleObjectB.
Scene02CommandPlaceTitleObjectB:
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
    jmp Scene02AdvanceCopy

; @routine 224B:1C8 Scene02CommandLoadBanks
; @run 6B:81C8
; WHAT IT DOES: As Scene5CCommandLoadBanks (25D3:1C0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CCommandLoadBanks.
; Entry, exit: as Scene5CCommandLoadBanks.
Scene02CommandLoadBanks:
    stz z:zTemp00
Scene02CommandLoadBanks_t81CA:
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
    bne Scene02CommandLoadBanks_t81CA
    jmp Scene02AdvanceCopy

; @routine 224B:1FD Scene02ObjectsStep
; @run 6B:81FD
; WHAT IT DOES: As Scene5CObjectsStep (25D3:1F5), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CObjectsStep.
; Entry, exit: as Scene5CObjectsStep.
Scene02ObjectsStep:
    clx
Scene02ObjectsStep_at1:
    lda a:OBJ_KIND,x
    beq Scene02ObjectsStep_at0
    jsr Scene02DispatchKind
Scene02ObjectsStep_at0:
    inx
    cpx #$58
    bne Scene02ObjectsStep_at1
    rts

; @routine 224B:20C Scene02DispatchKind
; @run 6B:820C
; WHAT IT DOES: As Scene5CDispatchKind (25D3:204), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($8).
; WAITS: as Scene5CDispatchKind.
; Entry, exit: as Scene5CDispatchKind.
Scene02DispatchKind:
    dec a
    asl a
    tay
    lda a:Scene02Kinds,y
    sta z:zTemp00
    lda a:Scene02Kinds+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 224B:226 Scene02StepsA
; @run 6B:8226
; @stop 8238
; WHAT IT DOES: Goes to the step its state names.
; HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene02StepsA:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:Scene02JumpTableA,y
    sta z:zTemp00
    lda a:Scene02JumpTableA+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 224B:23E Scene02Descend
; @run 6B:823E
; WHAT IT DOES: Lowers objects 0 and 2's Y by one a frame (16 bits); at object 2's Y low $B1 object $10 goes on a state, at $91 object 2's frame goes up, at $71 the object goes on.
; HOW: DECs with the borrow by compare with $FF.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene02Descend:
    dec a:OBJ_Y_LO
    lda a:OBJ_Y_LO
    cmp #$FF
    bne Scene02Descend_at0
    dec a:OBJ_Y_HI
Scene02Descend_at0:
    dec a:OBJ_Y_LO+2
    lda a:OBJ_Y_LO+2
    cmp #$FF
    bne Scene02Descend_at1
    dec a:OBJ_Y_HI+2
Scene02Descend_at1:
    lda a:OBJ_Y_LO+2
    cmp #$B1
    beq Scene02Descend_at2
    cmp #$91
    beq Scene02Descend_at3
    cmp #$71
    bne Scene02Descend_at4
    inc a:OBJ_STATE,x
    rts
Scene02Descend_at3:
    inc a:OBJ_FRAME+2
    rts
Scene02Descend_at2:
    inc a:OBJ_STATE+16
    rts
Scene02Descend_at4:
    rts

; @routine 224B:274 Scene02StepsB
; @run 6B:8274
; @stop 8286
; WHAT IT DOES: Goes to the step its state names.
; HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene02StepsB:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:Scene02JumpTableB,y
    sta z:zTemp00
    lda a:Scene02JumpTableB+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 224B:290 Scene02ScrollSet
; @run 6B:8290
; WHAT IT DOES: Puts the scroll's Y low at $FC and goes on.
; HOW: Two stores.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene02ScrollSet:
    lda #$FC
    sta a:ScrollYLo
    inc a:OBJ_STATE,x
    rts

; @routine 224B:299 Scene02ScrollUp
; @run 6B:8299
; WHAT IT DOES: Moves the scroll's Y low down by four a frame until 0, then goes on.
; HOW: SEC, SBC #4.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene02ScrollUp:
    lda a:ScrollYLo
    sec
    sbc #$04
    sta a:ScrollYLo
    beq Scene02ScrollSet+5
    rts

; @routine 224B:2A5 Scene02RaiseSecond
; @run 6B:82A5
; WHAT IT DOES: Raises object 1's Y (16 bits) until its low byte is $50.
; HOW: INC with the carry, CMP.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene02RaiseSecond:
    inc a:OBJ_Y_LO+1
    bne Scene02RaiseSecond_t82AD
    inc a:OBJ_Y_HI+1
Scene02RaiseSecond_t82AD:
    lda a:OBJ_Y_LO+1
    cmp #$50
    beq Scene02ScrollSet+5
    rts

; @routine 224B:2B5 Scene02Bob
; @run 6B:82B5
; WHAT IT DOES: Bobs object 1 on a sine of 15 around Y $50.
; HOW: ScaleBySine of 15 by work_d doubled; the product added to $50.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene02Bob:
    lda #$0F
    sta z:zTemp00
    stz z:zTemp01
    inc a:OBJ_FLASH_FRAMES,x
    lda a:OBJ_FLASH_FRAMES,x
    asl a
    jsr VecScaleBySine
    lda #$50
    clc
    adc z:SpriteFlipMask
    sta a:OBJ_Y_LO+1
    cla
    adc z:zPlaneRowsLeft
    sta a:OBJ_Y_HI+1
    rts

; @routine 224B:2DE Scene02PlayListOnce
; @run 6B:82DE
; WHAT IT DOES: Plays the start argument's frame list once.
; HOW: ScenePlayListOnce with this overlay's lists.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene02PlayListOnce:
    lda a:OBJ_STATE,x
    bne Scene02PlayListOnce_at0
    lda a:OBJ_START_ARG,x
    jsr Scene02FrameList
    bne Scene02PlayListOnce_at0
    lda a:OBJ_START_ARG,x
    bmi Scene02PlayListOnce_at0
    inc a:OBJ_STATE,x
Scene02PlayListOnce_at0:
    rts

; @routine 224B:2F4 Scene02SlideForCount
; @run 6B:82F4
; WHAT IT DOES: As the $5C overlay's slide, with this overlay's speed tables.
; HOW: SceneSlide.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene02SlideForCount:
    lda a:OBJ_STATE,x
    bne Scene02SlideForCount_t8318
    ldy a:OBJ_START_ARG,x
    lda a:Scene02SpeedsXLo,y
    sta a:OBJ_VX_LO,x
    lda a:Scene02SpeedsXHi,y
    sta a:OBJ_VX_HI,x
    lda a:Scene02SpeedsYLo,y
    sta a:OBJ_VY_LO,x
    lda a:Scene02SpeedsYHi,y
    sta a:OBJ_VY_HI,x
Scene02SlideForCount_at0:
    inc a:OBJ_STATE,x
    rts
Scene02SlideForCount_t8318:
    dec a
    bne Scene02SlideForCount_t8323
    jsr VecApplyVelocity
    dec a:OBJ_FLASH_FRAMES,x
    beq Scene02SlideForCount_at0
Scene02SlideForCount_t8323:
    rts

; @routine 224B:338 Scene02FrameList
; @run 6B:8338
; WHAT IT DOES: Steps an object through the frame list this overlay's table names for A.
; HOW: ObjectFrameListAt.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames.
Scene02FrameList:
    asl a
    tay
    lda a:Scene02FrameLists,y
    sta z:zTemp10
    lda a:Scene02FrameLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

