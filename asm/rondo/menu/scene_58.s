; @routine 267B:000 Scene58Step
; @run 6B:8000
; @stop 800E
; WHAT IT DOES: One call of the scene: bank zRamBaseBank + $0A mapped at $A000, then the handler of the sub-mode in
; the table at $800E (0 begins, 1 runs a frame, 2 waits for RUN).
; HOW: ADC and TAM to MPR5; zGameSubMode doubled indexes the table and is jumped through, to the routine loaded
; there: the $5A overlay holds these bytes with its own handlers at the same places.
; WAITS: yes, through the begin handler.
; Entry: nothing. Exit: as the handler leaves them; it is entered with A and X the doubled sub-mode, N and Z from it,
; C from the doubling and V from the bank's add.
Scene58Steps = $800E ; the sub-modes' handlers
Scene58TickCopy = $803C ; the bytes of EndingFrameTick, whose C serves them
Scene58Lengths = $8043 ; the script commands' lengths
Scene58Commands = $8082 ; the script commands' handlers
Scene58ClearCopy = $818E ; the bytes of EndingClearObjects, whose C serves them at this place
Scene58PlaceCopy = $8198 ; the bytes of EndingPlaceObject, whose C serves them at this place
Scene58Kinds = $826F ; object handlers by kind
Scene58Step:
    lda #$0A
    clc
    adc z:zRamBaseBank
    tam #$20
    lda z:zGameSubMode
    asl a
    tax
    jmp (Scene58Steps,x)

; @routine 267B:014 Scene58Begin
; @run 6B:8014
; WHAT IT DOES: Sub-mode 0: starts the scene: the game state wiped, the script from $8E8A, its frame counter $FFFF, the sub-mode on.
; HOW: ResetGameState, then Scene5CBegin's stores with this script.
; WAITS: yes, through ResetGameState (the vertical blank).
; Entry: X the doubled sub-mode, Y kept. Exit: A $FF, X 0, Y $FF, N and Z from the new sub-mode; C kept.
Scene58Begin:
    jsr VecResetGameState
    lda #$8A
    sta z:zScriptPtr
    lda #$8E
    sta z:zScriptPtrHi
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    rts

; @routine 267B:028 Scene58Frame
; @run 6B:8028
; WHAT IT DOES: Sub-mode 1: one frame of the scene: the frame counted, the script's command run if due, every object run.
; HOW: The copy of EndingFrameTick at $803C, the script step, the objects' loop.
; WAITS: no.
; Entry: nothing. Exit: as the objects' loop, Y and V the step's where the loop leaves them.
Scene58Frame:
    jsr Scene58TickCopy
    jsr Scene58ScriptStep
    jsr Scene58ObjectsStep
    rts

; @routine 267B:032 Scene58WaitForRun
; @run 6B:8032
; WHAT IT DOES: Sub-mode 2, after the script ends: the mode is done when RUN is newly pressed.
; HOW: BiosJoyTrigger's RUN bit; INC of zModeDone.
; WAITS: no.
; Entry: nothing. Exit: A the RUN bit, N and Z from it or from zModeDone; X, Y, C and V kept.
Scene58WaitForRun:
    lda a:BiosJoyTrigger
    and #$08
    beq Scene58Frame+9
    inc z:zModeDone
    rts

; @routine 267B:056 Scene58ScriptAdvance
; @run 6B:8056
; WHAT IT DOES: Steps the script past its command and runs on into the script step.
; HOW: The command's length from this overlay's table at $8043 added to the pointer.
; WAITS: no.
; Entry: nothing. Exit: as the script step, X the command stepped past when it left X alone, V from the add when it set none.
Scene58ScriptAdvance:
    lda (zScriptPtr)
    tax
    lda a:Scene58Lengths,x
    clc
    adc z:zScriptPtr
    sta z:zScriptPtr
    bcc Scene58ScriptStep
    inc z:zScriptPtrHi

; @routine 267B:065 Scene58ScriptStep
; @run 6B:8065
; WHAT IT DOES: Runs the script's command when its two time bytes equal the frame counter.
; HOW: The times compared; the command doubled indexes this overlay's table at $8082, entered with X the index and Y 3.
; WAITS: no.
; Entry: nothing. Exit: a time byte differs: that byte, N, Z and C from comparing it, Y 1 or 2, X kept; else as the command.
Scene58ScriptStep:
    ldy #$01
    lda (zScriptPtr),y
    cmp z:zWaitTimer3
    bne Scene58Frame+9
    iny
    lda (zScriptPtr),y
    cmp z:zWaitTimer4
    bne Scene58Frame+9
    lda (zScriptPtr)
    asl a
    tax
    ldy #$03
    jmp (Scene58Commands,x)

; @routine 267B:07D Scene58CommandEnd
; @run 6B:807D
; WHAT IT DOES: A script command: ends the script, the sub-mode set to 2 (wait for RUN); the script is not stepped.
; HOW: A store; RTS.
; WAITS: no.
; Entry: from the script step: X the doubled command, Y 3. Exit: A 2, N and Z from it; X, Y and C kept.
Scene58CommandEnd:
    lda #$02
    sta z:zGameSubMode
    rts

; @routine 267B:0A8 Scene58CommandSubModeTwo
; @run 6B:80A8
; WHAT IT DOES: A script command: the sub-mode set to 2, and the script steps on.
; HOW: A store; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandSubModeTwo:
    lda #$02
    sta z:zGameSubMode
    bra Scene58ScriptAdvance

; @routine 267B:0AE Scene58CommandSetScroll
; @run 6B:80AE
; WHAT IT DOES: A script command: the screen's scroll X high from the operand's high four bits, Y high from its low four.
; HOW: PHA, four LSR, PLA, AND; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandSetScroll:
    lda (zScriptPtr),y
    pha
    lsr a
    lsr a
    lsr a
    lsr a
    sta a:ScrollXHi
    pla
    and #$0F
    sta a:ScrollYHi
    bra Scene58ScriptAdvance

; @routine 267B:0C0 Scene58CommandStartListAndClear
; @run 6B:80C0
; WHAT IT DOES: A script command: list channel 0 started with the operand's list, the screen's state, every object and the scroll block ($EE bytes from ScrollXFrac) cleared.
; HOW: StartListChannel0Kind2, ClearScreenState, the copy of EndingClearObjects at $818E, a TAI of $EE zeros to ScrollXFrac; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandStartListAndClear:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    jsr VecClearScreenState
    jsr Scene58ClearCopy
    tai KernelZeros,ScrollXFrac,$00EE
    bra Scene58ScriptAdvance

; @routine 267B:0D4 Scene58CommandStartList
; @run 6B:80D4
; WHAT IT DOES: A script command: list channel 0 started with the operand's list.
; HOW: StartListChannel0Kind2; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandStartList:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    jmp Scene58ScriptAdvance

; @routine 267B:0DC Scene58CommandStartScript1
; @run 6B:80DC
; WHAT IT DOES: A script command: script slot 0 started with the operand, kind 1.
; HOW: StartScriptSlot0Kind1; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandStartScript1:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind1
    jmp Scene58ScriptAdvance

; @routine 267B:0E4 Scene58CommandStartScript2
; @run 6B:80E4
; WHAT IT DOES: A script command: script slot 0 started with the operand, kind 2.
; HOW: StartScriptSlot0Kind2; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandStartScript2:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind2
    jmp Scene58ScriptAdvance

; @routine 267B:0EC Scene58CommandStartScript3
; @run 6B:80EC
; WHAT IT DOES: A script command: script slot 0 started with the operand, kind 3.
; HOW: StartScriptSlot0Kind3; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandStartScript3:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind3
    jmp Scene58ScriptAdvance

; @routine 267B:0F4 Scene58CommandPlayAdpcm
; @run 6B:80F4
; WHAT IT DOES: A script command: the ADPCM sound the operand names played from the disc.
; HOW: AdpcmPlayFromDisc; the advance.
; WAITS: yes, on the drive and the ADPCM.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandPlayAdpcm:
    lda (zScriptPtr),y
    jsr VecAdpcmPlayFromDisc
    jmp Scene58ScriptAdvance

; @routine 267B:0FC Scene58CommandCdPlay
; @run 6B:80FC
; WHAT IT DOES: A script command: the CD track the operand names played once.
; HOW: CdPlayOnce with X the doubled command; the advance.
; WAITS: yes, on the drive.
; Entry: from the script step: X the doubled command, Y 3. Exit: as the advance.
Scene58CommandCdPlay:
    lda (zScriptPtr),y
    jsr VecCdPlayOnce
    jmp Scene58ScriptAdvance

; @routine 267B:104 Scene58CommandPlaceKind
; @run 6B:8104
; WHAT IT DOES: A script command: an object placed at the script's X and Y and given the kind that follows.
; HOW: The copy of EndingPlaceObject, then the kind; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandPlaceKind:
    lda (zScriptPtr),y
    tax
    jsr Scene58PlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_KIND,x
    jmp Scene58ScriptAdvance

; @routine 267B:113 Scene58CommandPlaceNext
; @run 6B:8113
; WHAT IT DOES: A script command: an object placed, its start argument and sprite set from the script, kind 2.
; HOW: The copy of EndingPlaceObject, then the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandPlaceNext:
    lda (zScriptPtr),y
    tax
    jsr Scene58PlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    lda #$02
    sta a:OBJ_KIND,x
    jmp Scene58ScriptAdvance

; @routine 267B:12D Scene58CommandPlaceFrame
; @run 6B:812D
; WHAT IT DOES: A script command: an object placed, its frame and sprite set from the script; its kind is left 0.
; HOW: The copy of EndingPlaceObject, then the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandPlaceFrame:
    lda (zScriptPtr),y
    tax
    jsr Scene58PlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FRAME,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    jmp Scene58ScriptAdvance

; @routine 267B:142 Scene58CommandPlaceTimed
; @run 6B:8142
; WHAT IT DOES: A script command: an object placed, its frame, sprite set, start argument and work_d from the script, kind 3.
; HOW: The copy of EndingPlaceObject, then the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandPlaceTimed:
    lda (zScriptPtr),y
    tax
    jsr Scene58PlaceCopy
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
    sta a:OBJ_WORK_D,x
    lda #$03
    sta a:OBJ_KIND,x
    jmp Scene58ScriptAdvance

; @routine 267B:168 Scene58CommandPlaceWorked
; @run 6B:8168
; WHAT IT DOES: A script command: an object placed, its work_e, sprite set, start argument and work_d from the script, kind 4.
; HOW: The copy of EndingPlaceObject, then the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandPlaceWorked:
    lda (zScriptPtr),y
    tax
    jsr Scene58PlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_WORK_E,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_WORK_D,x
    lda #$04
    sta a:OBJ_KIND,x
    jmp Scene58ScriptAdvance

; @routine 267B:1B4 Scene58CommandPlaceTitleObjectALow
; @run 6B:81B4
; WHAT IT DOES: A script command: slot $2E cleared and started (kind 1) with the four operands as start argument and work_d to work_f, Y low $40 and X high stepped down one.
; HOW: ClearObject, INC of the kind, the stores, DEC of X high; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandPlaceTitleObjectALow:
    ldx #$2E
    jsr VecClearObject
    inc a:OBJ_KIND,x
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_WORK_D,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_WORK_E,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_WORK_F,x
    lda #$40
    sta a:OBJ_Y_LO,x
    dec a:OBJ_X_HI,x
    jmp Scene58ScriptAdvance

; @routine 267B:1DE Scene58CommandPlaceTitleObjectB
; @run 6B:81DE
; WHAT IT DOES: A script command: slot $2F cleared and started (kind 1) with the four operands as start argument and work_d to work_f, Y low $D8.
; HOW: ClearObject, INC of the kind, the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandPlaceTitleObjectB:
    ldx #$2F
    jsr VecClearObject
    inc a:OBJ_KIND,x
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_WORK_D,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_WORK_E,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_WORK_F,x
    lda #$D8
    sta a:OBJ_Y_LO,x
Scene58CommandPlaceTitleObjectB_at0:
    jmp Scene58ScriptAdvance

; @routine 267B:205 Scene58CommandBranchOnProgress
; @run 6B:8205
; WHAT IT DOES: A script command: when the low four bits of ProgressCharacter are all set, the script goes on at $9225 and zRasterStage is cleared.
; HOW: AND and CMP of $0F; the pointer set and STZ; the advance either way (the step past the command at the pointer).
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandBranchOnProgress:
    lda a:ProgressCharacter
    and #$0F
    cmp #$0F
    bne Scene58CommandPlaceTitleObjectB_at0
    lda #$25
    sta z:zScriptPtr
    lda #$92
    sta z:zScriptPtrHi
    stz z:zRasterStage
    jmp Scene58ScriptAdvance

; @routine 267B:21B Scene58CommandLoadBanks
; @run 6B:821B
; WHAT IT DOES: A script command: reads four banks of the ADPCM memory into the banks from zRamBaseBank + $10 on, two at a time at MPR5 and MPR6.
; HOW: As EndingCommandLoadBanks: for zTemp00 0 to 3, TAM of the two banks, the BIOS registers set, ad_read; the advance.
; WAITS: yes, through ad_read, which waits for the ADPCM to be idle.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene58CommandLoadBanks:
    stz z:zTemp00
Scene58CommandLoadBanks_at0:
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
    bne Scene58CommandLoadBanks_at0
    jmp Scene58ScriptAdvance

; @routine 267B:250 Scene58ObjectsStep
; @run 6B:8250
; WHAT IT DOES: Runs every object: each slot with a kind through this overlay's handler for it.
; HOW: X 0 to $57; the dispatch for each kind not 0.
; WAITS: no.
; Entry: nothing. Exit: A the last kind or handler's A, X $58, N and Z from the CPX, C set; Y and V as the last handler left them.
Scene58ObjectsStep:
    clx
Scene58ObjectsStep_at1:
    lda a:OBJ_KIND,x
    beq Scene58ObjectsStep_at0
    jsr Scene58DispatchKind
Scene58ObjectsStep_at0:
    inx
    cpx #$58
    bne Scene58ObjectsStep_at1
    rts

; @routine 267B:25F Scene58DispatchKind
; @run 6B:825F
; @stop 826F
; WHAT IT DOES: Jumps to the handler of the object's kind, from this overlay's table at $826F.
; HOW: The kind less one, doubled, indexes the table; the word through zTemp00 and zTemp01.
; WAITS: no.
; Entry: A the kind (not 0), X the object. Exit: as the handler, entered with A the handler's high byte and Y the index.
Scene58DispatchKind:
    dec a
    asl a
    tay
    lda a:Scene58Kinds,y
    sta z:zTemp00
    lda a:Scene58Kinds+1,y
    sta z:zTemp01
    jmp (ZeroPage)
; @routine 267B:63D Scene58CycleLists
; @run 6B:863D
; WHAT IT DOES: An object handler: starts list channel 0 on lists $0C, $0D and $0E in turn, one every nine frames.
; HOW: SceneBCycleLists: work_e the list, work_d the count; StartListChannel0Kind2.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them; X the object, C from the list's compare after a start.
Scene58ShakeX = $8686 ; scroll X high by shake step
Scene58ShakeY = $868A ; scroll Y high by shake step
Scene58SpeedsXLo = $86E7 ; slide speeds by start argument
Scene58SpeedsXHi = $86F2 ; slide speeds by start argument
Scene58SpeedsYLo = $86FD ; slide speeds by start argument
Scene58SpeedsYHi = $8708 ; slide speeds by start argument
Scene58FrameLists = $8722 ; frame list pointers
Scene58CycleLists:
    lda a:OBJ_STATE,x
    bne Scene58CycleLists_at0
    inc a:OBJ_STATE,x
    lda #$0C
    sta a:OBJ_WORK_E,x
Scene58CycleLists_at3:
    lda #$09
    sta a:OBJ_WORK_D,x
Scene58CycleLists_at1:
    rts
Scene58CycleLists_at0:
    dec a:OBJ_WORK_D,x
    bne Scene58CycleLists_at1
    phx
    inc a:OBJ_WORK_E,x
    lda a:OBJ_WORK_E,x
    cmp #$0F
    bne Scene58CycleLists_at2
    lda #$0C
    sta a:OBJ_WORK_E,x
Scene58CycleLists_at2:
    jsr VecStartListChannel0Kind2
    plx
    bra Scene58CycleLists_at3

; @routine 267B:66B Scene58Shake
; @run 6B:866B
; @stop 8686
; WHAT IT DOES: An object handler: shakes the screen, every eighth frame setting the scroll's X high and Y high from
; the tables at $8686 and $868A by bits 3 and 4 of zFrameTick.
; HOW: SceneBShake.
; WAITS: no.
; Entry: X the object. Exit: A the last value, N and Z from it; Y the index and C clear after a store; X kept.
Scene58Shake:
    lda z:zFrameTick
    and #$07
    bne Scene58Shake_atR
    lda z:zFrameTick
    and #$18
    lsr a
    lsr a
    lsr a
    tay
    lda a:Scene58ShakeX,y
    sta a:ScrollXHi
    lda a:Scene58ShakeY,y
    sta a:ScrollYHi
Scene58Shake_atR:
    rts

; @routine 267B:698 Scene58SlideThenList
; @run 6B:8698
; WHAT IT DOES: An object handler: Scene58Slide, then the frame list work_e names.
; HOW: JSR Scene58Slide; JMP Scene58FrameList with A work_e.
; WAITS: no.
; Entry: X the object. Exit: as Scene58FrameList, V as the slide left it.
Scene58SlideThenList:
    jsr Scene58Slide
    lda a:OBJ_WORK_E,x
    jmp Scene58FrameList

; @routine 267B:6A1 Scene58PlayListOnce
; @run 6B:86A1
; WHAT IT DOES: An object handler: plays the start argument's frame list once, then moves on a state unless the
; argument is negative.
; HOW: ScenePlayListOnce with this overlay's lists ($8722).
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene58PlayListOnce:
    lda a:OBJ_STATE,x
    bne Scene58PlayListOnce_at0
    lda a:OBJ_START_ARG,x
    jsr Scene58FrameList
    bne Scene58PlayListOnce_at0
    lda a:OBJ_START_ARG,x
    bmi Scene58PlayListOnce_at0
    inc a:OBJ_STATE,x
Scene58PlayListOnce_at0:
    rts

; @routine 267B:6B7 Scene58Slide
; @run 6B:86B7
; @stop 86E7
; WHAT IT DOES: An object handler: takes the speeds its start argument names from the tables at $86E7, $86F2,
; $86FD and $8708, then moves for work_d frames and moves on a state.
; HOW: SceneSlide.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene58Slide:
    lda a:OBJ_STATE,x
    bne Scene58Slide_at0
    ldy a:OBJ_START_ARG,x
    lda a:Scene58SpeedsXLo,y
    sta a:OBJ_VX_LO,x
    lda a:Scene58SpeedsXHi,y
    sta a:OBJ_VX_HI,x
    lda a:Scene58SpeedsYLo,y
    sta a:OBJ_VY_LO,x
    lda a:Scene58SpeedsYHi,y
    sta a:OBJ_VY_HI,x
Scene58Slide_at2:
    inc a:OBJ_STATE,x
    rts
Scene58Slide_at0:
    dec a
    bne Scene58Slide_at1
    jsr VecApplyVelocity
    dec a:OBJ_WORK_D,x
    beq Scene58Slide_at2
Scene58Slide_at1:
    rts

; @routine 267B:713 Scene58FrameList
; @run 6B:8713
; @stop 8722
; WHAT IT DOES: Steps an object through the frame list this overlay's table at $8722 names for A.
; HOW: ObjectFrameListAt.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames.
Scene58FrameList:
    asl a
    tay
    lda a:Scene58FrameLists,y
    sta z:zTemp10
    lda a:Scene58FrameLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames
; @routine 267B:28B Scene58SpawnCycleA
; @run 6B:828B
; @stop 82E8
; WHAT IT DOES: An object handler: spawns objects in a cycle of six slots (the table at $82E8), each as kind 1 at Y
; $170, their work_d and the waits between them from the step table at $82EE; at the table's end it clears itself.
; HOW: SceneBSpawnCycle: work_e the wait, work_f the slot, work_d the step; ClearObjectInY.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene58SpawnSlotsA = $82E8 ; the six slots Scene58SpawnCycleA spawns into
Scene58SpawnStepsA = $82EE ; its steps: the wait, then the spawned work_d
Scene58SpawnSlotsB = $84D3 ; the six slots Scene58SpawnCycleB spawns into
Scene58SpawnStepsB = $84D9 ; its steps: the wait, then the spawned work_d
Scene58PatternRow0 = $B980 ; pattern rows in the bank Scene58Step maps at $A000
Scene58PatternRow1 = $B380
Scene58PatternRow2 = $B200
Scene58PatternRow3 = $B680
Scene58PatternRow4 = $B180
Scene58SpawnCycleA:
    lda a:OBJ_STATE,x
    bne Scene58SpawnCycleA_at0
    inc a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
Scene58SpawnCycleA_at8296:
    rts
Scene58SpawnCycleA_at0:
    dec a:OBJ_WORK_E,x
    bne Scene58SpawnCycleA_at8296
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_F,x
    cmp #$06
    bne Scene58SpawnCycleA_at1
    cla
    sta a:OBJ_WORK_F,x
Scene58SpawnCycleA_at1:
    tay
    lda a:Scene58SpawnSlotsA,y
    tay
    jsr VecClearObjectInY
    phx
    lda a:OBJ_WORK_D,x
    asl a
    tax
    lda a:Scene58SpawnStepsA+1,x
    sta a:OBJ_WORK_D,y
    plx
    phx
    lda a:OBJ_WORK_F,x
    sta a:OBJ_START_ARG,y
    lda #$01
    sta a:OBJ_KIND,y
    sta a:OBJ_Y_HI,y
    lda #$70
    sta a:OBJ_Y_LO,y
    plx
    inc a:OBJ_WORK_D,x
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:Scene58SpawnStepsA,y
    beq Scene58SpawnCycleA_at82E5
    sta a:OBJ_WORK_E,x
    rts
Scene58SpawnCycleA_at82E5:
    jmp VecClearObject

; @routine 267B:389 Scene58LoadPatterns
; @run 6B:8389
; WHAT IT DOES: An object handler: copies five rows of $80 bytes from $B980, $B380, $B200, $B680 and $B180 (the bank
; Scene58Step maps at $A000) to VRAM from $7000, then clears itself.
; HOW: SelectVramWriteAddress, ST1 and ST2, SelectVramData, five TIA to the VDC's data port; ClearObject.
; WAITS: no.
; Entry: X the object, Y the dispatch's index ($18: the handler of kind 13). Exit: A 2, N and Z from it; X, Y and C
; kept.
Scene58LoadPatterns:
    jsr VecSelectVramWriteAddress
    st1 #$00
    st2 #$70
    jsr VecSelectVramData
    tia Scene58PatternRow0,VDC_DATA_LO,$0080
    tia Scene58PatternRow1,VDC_DATA_LO,$0080
    tia Scene58PatternRow2,VDC_DATA_LO,$0080
    tia Scene58PatternRow3,VDC_DATA_LO,$0080
    tia Scene58PatternRow4,VDC_DATA_LO,$0080
    jmp VecClearObject

; @routine 267B:3B9 Scene58Swing
; @run 6B:83B9
; WHAT IT DOES: An object handler: swings object $10 left, right, left, right and left (X speed -$10 and +$10, turning
; below X low $40 and at $80), then sends it off left at -$60.
; HOW: SceneBSwing; ApplyVelocityX.
; WAITS: no.
; Entry: X the object. Exit: as the last compare left them, V from the move; from state 5 on, A and C from the move.
Scene58Swing:
    lda a:OBJ_STATE,x
    bne Scene58Swing_at0
    phx
    ldx #$10
    lda #$F0
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda a:OBJ_X_LO,x
    plx
    cmp #$40
    bcs Scene58Swing_at1
    inc a:OBJ_STATE,x
Scene58Swing_at1:
    rts
Scene58Swing_at0:
    dec a
    bne Scene58Swing_at2
    phx
    ldx #$10
    lda #$10
    sta a:OBJ_VX_LO,x
    lda #$00
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda a:OBJ_X_LO,x
    plx
    cmp #$80
    bcc Scene58Swing_at3
    inc a:OBJ_STATE,x
Scene58Swing_at3:
    rts
Scene58Swing_at2:
    dec a
    bne Scene58Swing_at8418
    phx
    ldx #$10
    lda #$F0
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda a:OBJ_X_LO,x
    plx
    cmp #$40
    bcs Scene58Swing_at4
    inc a:OBJ_STATE,x
Scene58Swing_at4:
    rts
Scene58Swing_at8418:
    dec a
    bne Scene58Swing_at5
    phx
    ldx #$10
    lda #$10
    sta a:OBJ_VX_LO,x
    lda #$00
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda a:OBJ_X_LO,x
    plx
    cmp #$80
    bcc Scene58Swing_at8436
    inc a:OBJ_STATE,x
Scene58Swing_at8436:
    rts
Scene58Swing_at5:
    dec a
    bne Scene58Swing_at8456
    phx
    ldx #$10
    lda #$F0
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda a:OBJ_X_LO,x
    plx
    cmp #$40
    bcs Scene58Swing_at8455
    inc a:OBJ_STATE,x
Scene58Swing_at8455:
    rts
Scene58Swing_at8456:
    phx
    ldx #$10
    lda #$A0
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    plx
    rts

; @routine 267B:468 Scene58ShowObjectsAndClear
; @run 6B:8468
; WHAT IT DOES: An object handler: gives objects $13, $11 and $0F sprite attribute $10, then clears itself.
; HOW: Three stores; ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A $10, N and Z from it; X, Y and C kept.
Scene58ShowObjectsAndClear:
    lda #$10
    sta a:OBJ_SPRITE_ATTR+19
    sta a:OBJ_SPRITE_ATTR+17
    sta a:OBJ_SPRITE_ATTR+15
    jmp VecClearObject

; @routine 267B:476 Scene58SpawnCycleB
; @run 6B:8476
; @stop 84D3
; WHAT IT DOES: An object handler: as Scene58SpawnCycleA with the slots at $84D3 and the steps at $84D9.
; HOW: SceneBSpawnCycle.
; WAITS: no.
; Entry: X the object. Exit: as the last test or store left them.
Scene58SpawnCycleB:
    lda a:OBJ_STATE,x
    bne Scene58SpawnCycleB_at0
    inc a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
Scene58SpawnCycleB_at1:
    rts
Scene58SpawnCycleB_at0:
    dec a:OBJ_WORK_E,x
    bne Scene58SpawnCycleB_at1
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_F,x
    cmp #$06
    bne Scene58SpawnCycleB_at2
    cla
    sta a:OBJ_WORK_F,x
Scene58SpawnCycleB_at2:
    tay
    lda a:Scene58SpawnSlotsB,y
    tay
    jsr VecClearObjectInY
    phx
    lda a:OBJ_WORK_D,x
    asl a
    tax
    lda a:Scene58SpawnStepsB+1,x
    sta a:OBJ_WORK_D,y
    plx
    phx
    lda a:OBJ_WORK_F,x
    sta a:OBJ_START_ARG,y
    lda #$01
    sta a:OBJ_KIND,y
    sta a:OBJ_Y_HI,y
    lda #$70
    sta a:OBJ_Y_LO,y
    plx
    inc a:OBJ_WORK_D,x
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:Scene58SpawnStepsB,y
    beq Scene58SpawnCycleB_at3
    sta a:OBJ_WORK_E,x
    rts
Scene58SpawnCycleB_at3:
    jmp VecClearObject
