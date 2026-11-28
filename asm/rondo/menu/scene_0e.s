; @routine 1E61:031 Scene0EFrame
; @run 6B:8031
; WHAT IT DOES: As Scene5CFrame (25D3:01F), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CFrame.
; Entry, exit: as Scene5CFrame.
Scene0EAdvanceCopy = $8053 ; the engine's routine at this place, whose bytes another overlay's C serves
Scene0ECommands = $807D ; script command handlers
Scene0EClearCopy = $815E ; the engine's routine at this place, whose bytes another overlay's C serves
Scene0EKinds = $8226 ; object handlers by kind
Scene0EJumpTableA = $8271 ; a state jump table
Scene0EJumpTableB = $82CC ; a state jump table
Scene0ESpeedsXLo = $843A ; slide speeds by start argument
Scene0ESpeedsYHi = $8449 ; slide speeds by start argument
Scene0EFrame:
    jsr ContinueStartRichter
    jsr Scene0EScriptStep
    jsr Scene0EObjectsStep
    rts

; @routine 1E61:062 Scene0EScriptStep
; @run 6B:8062
; WHAT IT DOES: As Scene5CScriptStep (25D3:050), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CScriptStep.
; Entry, exit: as Scene5CScriptStep.
Scene0EScriptStep:
    ldy #$01
    lda (zScriptPtr),y
    cmp z:zWaitTimer3
    bne Scene0EFrame+9
    iny
    lda (zScriptPtr),y
    cmp z:zWaitTimer4
    bne Scene0EFrame+9
    lda (zScriptPtr)
    asl a
    tax
    ldy #$03
    jmp (Scene0ECommands,x)

; @routine 1E61:0B7 Scene0ECommandStartListAndClear
; @run 6B:80B7
; WHAT IT DOES: As Scene5CCommandStartListAndClear (25D3:0A5), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandStartListAndClear.
; Entry, exit: as Scene5CCommandStartListAndClear.
Scene0ECommandStartListAndClear:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    jsr VecClearScreenState
    jsr Scene0EClearCopy
    tai KernelZeros,ScrollXFrac,$00EE
    bra Scene0EAdvanceCopy

; @routine 1E61:0D2 Scene0ECommandStartScript1
; @run 6B:80D2
; WHAT IT DOES: As Scene5CCommandStartScript1 (25D3:0C0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandStartScript1.
; Entry, exit: as Scene5CCommandStartScript1.
Scene0ECommandStartScript1:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind1
    jmp Scene0EAdvanceCopy

; @routine 1E61:0DA Scene0ECommandStartScript2
; @run 6B:80DA
; WHAT IT DOES: As Scene5CCommandStartScript2 (25D3:0C8), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandStartScript2.
; Entry, exit: as Scene5CCommandStartScript2.
Scene0ECommandStartScript2:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind2
    jmp Scene0EAdvanceCopy

; @routine 1E61:0E2 Scene0ECommandStartScript3
; @run 6B:80E2
; WHAT IT DOES: As Scene5CCommandStartScript3 (25D3:0D0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandStartScript3.
; Entry, exit: as Scene5CCommandStartScript3.
Scene0ECommandStartScript3:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind3
    jmp Scene0EAdvanceCopy

; @routine 1E61:0EA Scene0ECommandPlayAdpcm
; @run 6B:80EA
; WHAT IT DOES: As Scene5CCommandPlayAdpcm (25D3:0D8), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandPlayAdpcm.
; Entry, exit: as Scene5CCommandPlayAdpcm.
Scene0ECommandPlayAdpcm:
    lda (zScriptPtr),y
    jsr VecAdpcmPlayFromDisc
    jmp Scene0EAdvanceCopy

; @routine 1E61:0F2 Scene0ECommandCdPlay
; @run 6B:80F2
; WHAT IT DOES: As Scene5CCommandCdPlay (25D3:0E0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandCdPlay.
; Entry, exit: as Scene5CCommandCdPlay.
Scene0ECommandCdPlay:
    lda (zScriptPtr),y
    jsr VecCdPlayOnce
    jmp Scene0EAdvanceCopy

; @routine 1E61:0FA Scene0ECommandPlaceKind
; @run 6B:80FA
; WHAT IT DOES: As Scene5CCommandPlaceKind (25D3:0E8), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandPlaceKind.
; Entry, exit: as Scene5CCommandPlaceKind.
Scene0ECommandPlaceKind:
    lda (zScriptPtr),y
    tax
    jsr StageFadeTimer3CD
    iny
    lda (zScriptPtr),y
    sta a:OBJ_KIND,x
    jmp Scene0EAdvanceCopy

; @routine 1E61:109 Scene0ECommandPlaceNext
; @run 6B:8109
; WHAT IT DOES: As Scene5CCommandPlaceNext (25D3:0F7), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandPlaceNext.
; Entry, exit: as Scene5CCommandPlaceNext.
Scene0ECommandPlaceNext:
    lda (zScriptPtr),y
    tax
    jsr StageFadeTimer3CD
    iny
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    lda #$02
    sta a:OBJ_KIND,x
    jmp Scene0EAdvanceCopy

; @routine 1E61:123 Scene0ECommandPlaceFrame
; @run 6B:8123
; WHAT IT DOES: As Scene5CCommandPlaceFrame (25D3:111), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandPlaceFrame.
; Entry, exit: as Scene5CCommandPlaceFrame.
Scene0ECommandPlaceFrame:
    lda (zScriptPtr),y
    tax
    jsr StageFadeTimer3CD
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FRAME,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    jmp Scene0EAdvanceCopy

; @routine 1E61:138 Scene0ECommandPlaceTimed
; @run 6B:8138
; WHAT IT DOES: As Scene5CCommandPlaceTimed (25D3:126), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandPlaceTimed.
; Entry, exit: as Scene5CCommandPlaceTimed.
Scene0ECommandPlaceTimed:
    lda (zScriptPtr),y
    tax
    jsr StageFadeTimer3CD
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
    jmp Scene0EAdvanceCopy

; @routine 1E61:184 Scene0ECommandPlaceTitleObjectA
; @run 6B:8184
; WHAT IT DOES: As Scene5CCommandPlaceTitleObjectA (25D3:172), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandPlaceTitleObjectA.
; Entry, exit: as Scene5CCommandPlaceTitleObjectA.
Scene0ECommandPlaceTitleObjectA:
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
    jmp Scene0EAdvanceCopy

; @routine 1E61:1AB Scene0ECommandPlaceTitleObjectB
; @run 6B:81AB
; WHAT IT DOES: As Scene5CCommandPlaceTitleObjectB (25D3:199), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandPlaceTitleObjectB.
; Entry, exit: as Scene5CCommandPlaceTitleObjectB.
Scene0ECommandPlaceTitleObjectB:
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
    jmp Scene0EAdvanceCopy

; @routine 1E61:1D2 Scene0ECommandLoadBanks
; @run 6B:81D2
; WHAT IT DOES: As Scene5CCommandLoadBanks (25D3:1C0), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CCommandLoadBanks.
; Entry, exit: as Scene5CCommandLoadBanks.
Scene0ECommandLoadBanks:
    stz z:zTemp00
Scene0ECommandLoadBanks_t81D4:
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
    bne Scene0ECommandLoadBanks_t81D4
    jmp Scene0EAdvanceCopy

; @routine 1E61:207 Scene0EObjectsStep
; @run 6B:8207
; WHAT IT DOES: As Scene5CObjectsStep (25D3:1F5), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CObjectsStep.
; Entry, exit: as Scene5CObjectsStep.
Scene0EObjectsStep:
    clx
Scene0EObjectsStep_t8208:
    lda a:OBJ_KIND,x
    beq Scene0EObjectsStep_at0
    jsr Scene0EDispatchKind
Scene0EObjectsStep_at0:
    inx
    cpx #$58
    bne Scene0EObjectsStep_t8208
    rts

; @routine 1E61:216 Scene0EDispatchKind
; @run 6B:8216
; WHAT IT DOES: As Scene5CDispatchKind (25D3:204), at this overlay's place in the shared engine.
; HOW: The shared body of scene_engine.c with this overlay's shift ($12).
; WAITS: as Scene5CDispatchKind.
; Entry, exit: as Scene5CDispatchKind.
Scene0EDispatchKind:
    dec a
    asl a
    tay
    lda a:Scene0EKinds,y
    sta z:zTemp00
    lda a:Scene0EKinds+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1E61:236 Scene0ERiseScroll
; @run 6B:8236
; WHAT IT DOES: Waits a frame, then scrolls: X low up one and Y (16.8) up half a dot a frame until X low wraps.
; HOW: INC; ADC of $80 to the Y fraction and 0 to Y low.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0ERiseScroll:
    lda a:OBJ_STATE,x
    bne Scene0ERiseScroll_at0
    inc a:OBJ_STATE,x
Scene0ERiseScroll_t823E:
    rts
Scene0ERiseScroll_at0:
    dec a
    bne Scene0ERiseScroll_t823E
    inc a:ScrollXLo
    lda #$80
    clc
    adc a:ScrollYFrac
    sta a:ScrollYFrac
    cla
    adc a:ScrollYLo
    sta a:ScrollYLo
    lda a:ScrollXLo
    inc a
    bne Scene0ERiseScroll_t823E
    inc a:OBJ_STATE,x
    rts

; @routine 1E61:25F Scene0EStepsA
; @run 6B:825F
; @stop 8271
; WHAT IT DOES: Goes to the step its state names.
; HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0EStepsA:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:Scene0EJumpTableA,y
    sta z:zTemp00
    lda a:Scene0EJumpTableA+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1E61:277 Scene0EPieceStart
; @run 6B:8277
; WHAT IT DOES: Sets the work count $50 and goes on.
; HOW: Two stores.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0EPieceStart:
    lda #$50
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
    rts

; @routine 1E61:280 Scene0EPieceSwingA
; @run 6B:8280
; WHAT IT DOES: Counts down; above the limit the two pieces swing on a sine and move apart.
; HOW: DEC work_d; ScaleBySine of 2 by the count times 8 plus $C0; Y places; X of object 0 up, object 1 down.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0EPieceSwingA:
    dec a:OBJ_FLASH_FRAMES,x
    lda a:OBJ_FLASH_FRAMES,x
    cmp #$FF
    beq Scene0EPieceStart+5
    cmp #$10
    bcc Scene0EPieceSwingA_at0
    lda #$02
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_FLASH_FRAMES,x
    asl a
    asl a
    asl a
    clc
    adc #$C0
    jsr VecScaleBySine
    lda #$43
    clc
    adc z:SpriteFlipMask
    sta a:OBJ_Y_LO
    clc
    adc #$28
    sta a:OBJ_Y_LO+1
    inc a:OBJ_X_LO
    bne Scene0EPieceSwingA_at0
    inc a:OBJ_X_HI
Scene0EPieceSwingA_at0:
    dec a:OBJ_X_LO+1
    rts

; @routine 1E61:2BA Scene0EStepsB
; @run 6B:82BA
; @stop 82CC
; WHAT IT DOES: Goes to the step its state names.
; HOW: The state doubled indexes the table; the word through zTemp00 and zTemp01.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0EStepsB:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:Scene0EJumpTableB,y
    sta z:zTemp00
    lda a:Scene0EJumpTableB+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1E61:2D2 Scene0EPieceStartB
; @run 6B:82D2
; WHAT IT DOES: Sets the work count $60 and goes on.
; HOW: Two stores.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0EPieceStartB:
    lda #$60
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
    rts

; @routine 1E61:2DB Scene0EPieceSwingB
; @run 6B:82DB
; WHAT IT DOES: Counts down; above the limit the two pieces swing on a sine and move apart.
; HOW: DEC work_d; ScaleBySine of 2 by the count times 8 plus $C0; Y places; X of object 0 up, object 1 down.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0EPieceSwingB:
    dec a:OBJ_FLASH_FRAMES,x
    lda a:OBJ_FLASH_FRAMES,x
    cmp #$FF
    beq Scene0EPieceStartB+5
    cmp #$08
    bcc Scene0EPieceSwingB_at0
    lda #$02
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_FLASH_FRAMES,x
    asl a
    asl a
    asl a
    clc
    adc #$C0
    jsr VecScaleBySine
    lda #$53
    clc
    adc z:SpriteFlipMask
    sta a:OBJ_Y_LO
    sta a:OBJ_Y_LO+1
    inc a:OBJ_X_LO
    bne Scene0EPieceSwingB_at0
    inc a:OBJ_X_HI
Scene0EPieceSwingB_at0:
    dec a:OBJ_X_LO+1
    rts

; @routine 1E61:312 Scene0EBandsA
; @run 6B:8312
; WHAT IT DOES: Starts raster effect $13, then after a count pulls two raster bands apart by half a dot a frame until the lower reaches $BF.
; HOW: State 0 sets up; state 1 counts work_d and work_e; state 2 the 16.8 band scrolls.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0EBandsA:
    lda a:OBJ_STATE,x
    bne Scene0EBandsA_at0
    lda #$13
    sta z:zRasterEffect
    inc a:ScrollYHi
    lda #$47
    sta a:ScrollYLo+5
    lda #$97
    sta a:ScrollYLo+6
    inc a:ScrollXHi+2
    lda #$00
    sta a:OBJ_FLYER_WAIT,x
    lda #$FF
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
    rts
Scene0EBandsA_at0:
    dec a
    bne Scene0EBandsA_at1
    dec a:OBJ_FLASH_FRAMES,x
    bne Scene0EBandsA_at2
    dec a:OBJ_FLYER_WAIT,x
    bpl Scene0EBandsA_at2
    inc a:OBJ_STATE,x
    rts
Scene0EBandsA_at1:
    dec a
    bne Scene0EBandsA_at2
    lda a:ScrollYLo+5
    cmp #$2F
    beq Scene0EBandsA_at3
    lda a:ScrollYFrac+5
    sec
    sbc #$80
    sta a:ScrollYFrac+5
    lda a:ScrollYLo+5
    sbc #$00
    sta a:ScrollYLo+5
Scene0EBandsA_at3:
    lda a:ScrollYFrac+6
    clc
    adc #$80
    sta a:ScrollYFrac+6
    lda a:ScrollYLo+6
    adc #$00
    sta a:ScrollYLo+6
    cmp #$BF
    bne Scene0EBandsA_at2
    inc a:OBJ_STATE,x
Scene0EBandsA_at2:
    rts

; @routine 1E61:37E Scene0EBandsB
; @run 6B:837E
; WHAT IT DOES: As Scene0EBandsA with other starts and ends ($27, $C7) and a count of $170.
; HOW: As Scene0EBandsA.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0EBandsB:
    lda a:OBJ_STATE,x
    bne Scene0EBandsB_at0
    lda #$13
    sta z:zRasterEffect
    inc a:ScrollYHi
    lda #$3F
    sta a:ScrollYLo+5
    lda #$8F
    sta a:ScrollYLo+6
    inc a:ScrollXHi+2
    lda #$01
    sta a:OBJ_FLYER_WAIT,x
    lda #$70
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
    rts
Scene0EBandsB_at0:
    dec a
    bne Scene0EBandsB_t83B6
    dec a:OBJ_FLASH_FRAMES,x
    bne Scene0EBandsB_t83E9
    dec a:OBJ_FLYER_WAIT,x
    bpl Scene0EBandsB_t83E9
    inc a:OBJ_STATE,x
    rts
Scene0EBandsB_t83B6:
    dec a
    bne Scene0EBandsB_t83E9
    lda a:ScrollYLo+5
    cmp #$27
    beq Scene0EBandsB_at1
    lda a:ScrollYFrac+5
    sec
    sbc #$80
    sta a:ScrollYFrac+5
    lda a:ScrollYLo+5
    sbc #$00
    sta a:ScrollYLo+5
Scene0EBandsB_at1:
    lda a:ScrollYFrac+6
    clc
    adc #$80
    sta a:ScrollYFrac+6
    lda a:ScrollYLo+6
    adc #$00
    sta a:ScrollYLo+6
    cmp #$C7
    bne Scene0EBandsB_t83E9
    inc a:OBJ_STATE,x
Scene0EBandsB_t83E9:
    rts

; @routine 1E61:3F4 Scene0EPlayListOnce
; @run 6B:83F4
; WHAT IT DOES: Plays the start argument's frame list once.
; HOW: ScenePlayListOnce with this overlay's lists.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0EPlayListOnce:
    lda a:OBJ_STATE,x
    bne Scene0EPlayListOnce_at0
    lda a:OBJ_START_ARG,x
    jsr Scene0EFrameList
    bne Scene0EPlayListOnce_at0
    lda a:OBJ_START_ARG,x
    bmi Scene0EPlayListOnce_at0
    inc a:OBJ_STATE,x
Scene0EPlayListOnce_at0:
    rts

; @routine 1E61:40A Scene0ESlideForCount
; @run 6B:840A
; WHAT IT DOES: As the $5C overlay's slide, with this overlay's speed tables.
; HOW: SceneSlide.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene0ESlideForCount:
    lda a:OBJ_STATE,x
    bne Scene0ESlideForCount_at0
    ldy a:OBJ_START_ARG,x
    lda a:Scene0ESpeedsXLo,y
    sta a:OBJ_VX_LO,x
    lda a:WalkPlayerStepByRoute+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkStopXByRoute+4,y
    sta a:OBJ_VY_LO,x
    lda a:Scene0ESpeedsYHi,y
    sta a:OBJ_VY_HI,x
Scene0ESlideForCount_at2:
    inc a:OBJ_STATE,x
    rts
Scene0ESlideForCount_at0:
    dec a
    bne Scene0ESlideForCount_at1
    jsr VecApplyVelocity
    dec a:OBJ_FLASH_FRAMES,x
    beq Scene0ESlideForCount_at2
Scene0ESlideForCount_at1:
    rts

; @routine 1E61:44E Scene0EFrameList
; @run 6B:844E
; WHAT IT DOES: Steps an object through the frame list this overlay's table names for A.
; HOW: ObjectFrameListAt.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames.
Scene0EFrameList:
    asl a
    tay
    lda a:StartFrameByCharacter+7,y
    sta z:zTemp10
    lda a:StartFrameByCharacter+8,y
    sta z:zTemp11
    jmp VecStepObjectFrames

