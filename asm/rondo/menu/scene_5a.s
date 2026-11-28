; @routine 2703:014 Scene5ABegin
; @run 6B:8014
; WHAT IT DOES: Sub-mode 0: starts the scene: the game state wiped, the script from $8D9C, its frame counter $FFFF, the sub-mode on.
; HOW: ResetGameState, then Scene5CBegin's stores with this script.
; WAITS: yes, through ResetGameState (the vertical blank).
; Entry: X the doubled sub-mode, Y kept. Exit: A $FF, X 0, Y $FF, N and Z from the new sub-mode; C kept.
Scene5ATickCopy = $803C ; the bytes of EndingFrameTick, whose C serves them
Scene5ALengths = $8043 ; the script commands' lengths
Scene5ACommands = $8080 ; the script commands' handlers
Scene5AClearCopy = $8162 ; the bytes of EndingClearObjects, whose C serves them at this place
Scene5APlaceCopy = $816C ; the bytes of EndingPlaceObject, whose C serves them at this place
Scene5AObjectsCopy = $820B ; the bytes of Scene0CObjectsStep, whose C serves them
Scene5ABegin:
    jsr VecResetGameState
    lda #$9C
    sta z:zScriptPtr
    lda #$8D
    sta z:zScriptPtrHi
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    rts

; @routine 2703:028 Scene5AFrame
; @run 6B:8028
; WHAT IT DOES: Sub-mode 1: one frame of the scene: the frame counted, the script's command run if due, every object run.
; HOW: The copy of EndingFrameTick at $803C, the script step, the objects' loop.
; WAITS: no.
; Entry: nothing. Exit: as the objects' loop, Y and V the step's where the loop leaves them.
Scene5AFrame:
    jsr Scene5ATickCopy
    jsr Scene5AScriptStep
    jsr Scene5AObjectsCopy
    rts

; @routine 2703:054 Scene5AScriptAdvance
; @run 6B:8054
; WHAT IT DOES: Steps the script past its command and runs on into the script step.
; HOW: The command's length from this overlay's table at $8043 added to the pointer.
; WAITS: no.
; Entry: nothing. Exit: as the script step, X the command stepped past when it left X alone, V from the add when it set none.
Scene5AScriptAdvance:
    lda (zScriptPtr)
    tax
    lda a:Scene5ALengths,x
    clc
    adc z:zScriptPtr
    sta z:zScriptPtr
    bcc Scene5AScriptStep
    inc z:zScriptPtrHi

; @routine 2703:063 Scene5AScriptStep
; @run 6B:8063
; WHAT IT DOES: Runs the script's command when its two time bytes equal the frame counter.
; HOW: The times compared; the command doubled indexes this overlay's table at $8080, entered with X the index and Y 3.
; WAITS: no.
; Entry: nothing. Exit: a time byte differs: that byte, N, Z and C from comparing it, Y 1 or 2, X kept; else as the command.
Scene5AScriptStep:
    ldy #$01
    lda (zScriptPtr),y
    cmp z:zWaitTimer3
    bne Scene5AFrame+9
    iny
    lda (zScriptPtr),y
    cmp z:zWaitTimer4
    bne Scene5AFrame+9
    lda (zScriptPtr)
    asl a
    tax
    ldy #$03
    jmp (Scene5ACommands,x)

; @routine 2703:0A2 Scene5ACommandSubModeTwo
; @run 6B:80A2
; WHAT IT DOES: A script command: the sub-mode set to 2, and the script steps on.
; HOW: A store; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandSubModeTwo:
    lda #$02
    sta z:zGameSubMode
    bra Scene5AScriptAdvance

; @routine 2703:0A8 Scene5ACommandSetScroll
; @run 6B:80A8
; WHAT IT DOES: A script command: the screen's scroll X high from the operand's high four bits, Y high from its low four.
; HOW: PHA, four LSR, PLA, AND; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandSetScroll:
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
    bra Scene5AScriptAdvance

; @routine 2703:0BA Scene5ACommandStartListAndClear
; @run 6B:80BA
; WHAT IT DOES: A script command: list channel 0 started with the operand's list, the screen's state, every object and the scroll block ($EE bytes from ScrollXFrac) cleared.
; HOW: StartListChannel0Kind2, ClearScreenState, the copy of EndingClearObjects at $8162, a TAI of $EE zeros to ScrollXFrac; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandStartListAndClear:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    jsr VecClearScreenState
    jsr Scene5AClearCopy
    tai KernelZeros,ScrollXFrac,$00EE
    bra Scene5AScriptAdvance

; @routine 2703:0CE Scene5ACommandStartList
; @run 6B:80CE
; WHAT IT DOES: A script command: list channel 0 started with the operand's list.
; HOW: StartListChannel0Kind2; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandStartList:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    jmp Scene5AScriptAdvance

; @routine 2703:0D6 Scene5ACommandStartScript1
; @run 6B:80D6
; WHAT IT DOES: A script command: script slot 0 started with the operand, kind 1.
; HOW: StartScriptSlot0Kind1; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandStartScript1:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind1
    jmp Scene5AScriptAdvance

; @routine 2703:0DE Scene5ACommandStartScript2
; @run 6B:80DE
; WHAT IT DOES: A script command: script slot 0 started with the operand, kind 2.
; HOW: StartScriptSlot0Kind2; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandStartScript2:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind2
    jmp Scene5AScriptAdvance

; @routine 2703:0E6 Scene5ACommandStartScript3
; @run 6B:80E6
; WHAT IT DOES: A script command: script slot 0 started with the operand, kind 3.
; HOW: StartScriptSlot0Kind3; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandStartScript3:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind3
    jmp Scene5AScriptAdvance

; @routine 2703:0EE Scene5ACommandPlayAdpcm
; @run 6B:80EE
; WHAT IT DOES: A script command: the ADPCM sound the operand names played from the disc.
; HOW: AdpcmPlayFromDisc; the advance.
; WAITS: yes, on the drive and the ADPCM.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandPlayAdpcm:
    lda (zScriptPtr),y
    jsr VecAdpcmPlayFromDisc
    jmp Scene5AScriptAdvance

; @routine 2703:0F6 Scene5ACommandCdPlay
; @run 6B:80F6
; WHAT IT DOES: A script command: the CD track the operand names played once.
; HOW: CdPlayOnce with X the doubled command; the advance.
; WAITS: yes, on the drive.
; Entry: from the script step: X the doubled command, Y 3. Exit: as the advance.
Scene5ACommandCdPlay:
    lda (zScriptPtr),y
    jsr VecCdPlayOnce
    jmp Scene5AScriptAdvance

; @routine 2703:0FE Scene5ACommandPlaceKind
; @run 6B:80FE
; WHAT IT DOES: A script command: an object placed at the script's X and Y and given the kind that follows.
; HOW: The copy of EndingPlaceObject, then the kind; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandPlaceKind:
    lda (zScriptPtr),y
    tax
    jsr Scene5APlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_KIND,x
    jmp Scene5AScriptAdvance

; @routine 2703:10D Scene5ACommandPlaceNext
; @run 6B:810D
; WHAT IT DOES: A script command: an object placed, its start argument and sprite set from the script, kind 2.
; HOW: The copy of EndingPlaceObject, then the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandPlaceNext:
    lda (zScriptPtr),y
    tax
    jsr Scene5APlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    lda #$02
    sta a:OBJ_KIND,x
    jmp Scene5AScriptAdvance

; @routine 2703:127 Scene5ACommandPlaceFrame
; @run 6B:8127
; WHAT IT DOES: A script command: an object placed, its frame and sprite set from the script; its kind is left 0.
; HOW: The copy of EndingPlaceObject, then the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandPlaceFrame:
    lda (zScriptPtr),y
    tax
    jsr Scene5APlaceCopy
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FRAME,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    jmp Scene5AScriptAdvance

; @routine 2703:13C Scene5ACommandPlaceTimed
; @run 6B:813C
; WHAT IT DOES: A script command: an object placed, its frame, sprite set, start argument and work_d from the script, kind 3.
; HOW: The copy of EndingPlaceObject, then the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandPlaceTimed:
    lda (zScriptPtr),y
    tax
    jsr Scene5APlaceCopy
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
    jmp Scene5AScriptAdvance

; @routine 2703:188 Scene5ACommandPlaceTitleObjectA
; @run 6B:8188
; WHAT IT DOES: A script command: slot $2E cleared and started (kind 1) with the four operands as start argument and work_d to work_f, Y low $C8.
; HOW: ClearObject, INC of the kind, the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandPlaceTitleObjectA:
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
    lda #$C8
    sta a:OBJ_Y_LO,x
    jmp Scene5AScriptAdvance

; @routine 2703:1AF Scene5ACommandPlaceTitleObjectB
; @run 6B:81AF
; WHAT IT DOES: A script command: slot $2F cleared and started (kind 1) with the four operands as start argument and work_d to work_f, Y low $D8.
; HOW: ClearObject, INC of the kind, the stores; the advance.
; WAITS: no.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandPlaceTitleObjectB:
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
    jmp Scene5AScriptAdvance

; @routine 2703:1D6 Scene5ACommandLoadBanks
; @run 6B:81D6
; WHAT IT DOES: A script command: reads four banks of the ADPCM memory into the banks from zRamBaseBank + $10 on, two at a time at MPR5 and MPR6.
; HOW: As EndingCommandLoadBanks: for zTemp00 0 to 3, TAM of the two banks, the BIOS registers set, ad_read; the advance.
; WAITS: yes, through ad_read, which waits for the ADPCM to be idle.
; Entry: from the script step, Y 3. Exit: as the advance.
Scene5ACommandLoadBanks:
    stz z:zTemp00
Scene5ACommandLoadBanks_at0:
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
    bne Scene5ACommandLoadBanks_at0
    jmp Scene5AScriptAdvance
