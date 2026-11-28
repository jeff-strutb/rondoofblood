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
