; @routine 25D3:000 Scene5CStep
; @run 6B:8000
; @stop 8007
; WHAT IT DOES: One call of the scene: the handler of the sub-mode in the table at $8007 (0 begins, 1 runs a frame).
; HOW: zGameSubMode doubled indexes the table and is jumped through.
; WAITS: yes, through Scene5CBegin.
; Entry: nothing. Exit: as the handler leaves them.
Scene5CSteps = $8007 ; the sub-modes' handlers
Scene5CCommands = $806B ; the script commands' handlers
Scene5CKinds = $8214 ; object handlers by kind
Scene5CDrifters = $8284 ; the five drifting slots
Scene5CSpeedsXL = $82D9 ; slide speeds
Scene5CSpeedsXH = $82DE ; slide speeds
Scene5CSpeedsYL = $82E3 ; slide speeds
Scene5CSpeedsYH = $82E8 ; slide speeds
Scene5CFrameLists = $82FC ; frame list pointers
Scene5CStep:
    lda z:zGameSubMode
    asl a
    tax
    jmp (Scene5CSteps,x)

; @routine 25D3:00B Scene5CBegin
; @run 6B:800B
; WHAT IT DOES: Starts the scene: the game state wiped, the script from $838D, its frame counter $FFFF, the state goes on.
; HOW: As EndingBegin, with this scene's script.
; WAITS: yes, through ResetGameState (the vertical blank).
; Entry: X the doubled sub-mode and Y kept. Exit: A $FF, X 0, Y $FF, N and Z from the new state; C kept.
Scene5CBegin:
    jsr VecResetGameState
    lda #$8D
    sta z:zScriptPtr
    lda #$83
    sta z:zScriptPtrHi
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    rts

; @routine 25D3:01F Scene5CFrame
; @run 6B:801F
; WHAT IT DOES: One frame of the scene: the frame counted, the script's command run if due, every object run.
; HOW: Scene5CFrameTick (the bytes of EndingFrameTick), Scene5CScriptStep, Scene5CObjectsStep.
; WAITS: no.
; Entry: nothing. Exit: as Scene5CObjectsStep.
Scene5CFrame:
    jsr EndingFrameTick
    jsr Scene5CScriptStep
    jsr Scene5CObjectsStep
    rts

; @routine 25D3:041 Scene5CScriptAdvance
; @run 6B:8041
; WHAT IT DOES: Steps the script past its command and runs on into Scene5CScriptStep.
; HOW: The command's length from $8030 added to the pointer.
; WAITS: no.
; Entry: nothing. Exit: as Scene5CScriptStep, X the command stepped past when it left X alone.
Scene5CScriptAdvance:
    lda (zScriptPtr)
    tax
    lda a:TitleSteps+2,x
    clc
    adc z:zScriptPtr
    sta z:zScriptPtr
    bcc Scene5CScriptStep
    inc z:zScriptPtrHi

; @routine 25D3:050 Scene5CScriptStep
; @run 6B:8050
; WHAT IT DOES: Runs the script's command when its two time bytes equal the frame counter.
; HOW: The times compared; the command doubled indexes the table at $806B, entered with X the index and Y 3.
; WAITS: no.
; Entry: nothing. Exit: a time byte differs: that byte, N, Z and C from comparing it, Y 1 or 2, X kept; else as the command.
Scene5CScriptStep:
    ldy #$01
    lda (zScriptPtr),y
    cmp z:zWaitTimer3
    bne Scene5CFrame+9
    iny
    lda (zScriptPtr),y
    cmp z:zWaitTimer4
    bne Scene5CFrame+9
    lda (zScriptPtr)
    asl a
    tax
    ldy #$03
    jmp (Scene5CCommands,x)
    dec z:zModeDone
    rts

; @routine 25D3:08D Scene5CCommandSubModeTwo
; @run 6B:808D
; WHAT IT DOES: A script command: sub-mode 2.
; HOW: LDA #2, STA zGameSubMode.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandSubModeTwo:
    lda #$02
    sta z:zGameSubMode
    bra Scene5CScriptAdvance

; @routine 25D3:093 Scene5CCommandSetScroll
; @run 6B:8093
; WHAT IT DOES: A script command: sets the scroll's high bytes from the operand's two nibbles.
; HOW: The high nibble to ScrollXHi, the low to ScrollYHi, with PHA and PLA.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandSetScroll:
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
    bra Scene5CScriptAdvance

; @routine 25D3:0A5 Scene5CCommandStartListAndClear
; @run 6B:80A5
; WHAT IT DOES: A script command: starts the operand's list, clears the screen's state, the object slots and the scroll block.
; HOW: StartListChannel0Kind2, ClearScreenState, Scene5CClearObjects, a TAI to $3E00.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandStartListAndClear:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    jsr VecClearScreenState
    jsr Scene5CClearObjects
    tai KernelZeros,ScrollXFrac,$00EE
    bra Scene5CScriptAdvance

; @routine 25D3:0B9 Scene5CCommandStartList
; @run 6B:80B9
; WHAT IT DOES: A script command: starts list channel 0 with the operand's list.
; HOW: StartListChannel0Kind2.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandStartList:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    bra Scene5CScriptAdvance

; @routine 25D3:0C0 Scene5CCommandStartScript1
; @run 6B:80C0
; WHAT IT DOES: A script command: starts script slot 0 as kind 1 with the operand's script.
; HOW: StartScriptSlot0Kind1.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandStartScript1:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind1
    jmp Scene5CScriptAdvance

; @routine 25D3:0C8 Scene5CCommandStartScript2
; @run 6B:80C8
; WHAT IT DOES: A script command: starts script slot 0 as kind 2 with the operand's script.
; HOW: StartScriptSlot0Kind2.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandStartScript2:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind2
    jmp Scene5CScriptAdvance

; @routine 25D3:0D0 Scene5CCommandStartScript3
; @run 6B:80D0
; WHAT IT DOES: A script command: starts script slot 0 as kind 3 with the operand's script.
; HOW: StartScriptSlot0Kind3.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandStartScript3:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind3
    jmp Scene5CScriptAdvance

; @routine 25D3:0D8 Scene5CCommandPlayAdpcm
; @run 6B:80D8
; WHAT IT DOES: A script command: plays the operand's sound from the disc as ADPCM.
; HOW: AdpcmPlayFromDisc.
; WAITS: yes, on the drive and the ADPCM.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandPlayAdpcm:
    lda (zScriptPtr),y
    jsr VecAdpcmPlayFromDisc
    jmp Scene5CScriptAdvance

; @routine 25D3:0E0 Scene5CCommandCdPlay
; @run 6B:80E0
; WHAT IT DOES: A script command: plays the operand's CD track once.
; HOW: CdPlayOnce.
; WAITS: yes, on the drive.
; Entry: from Scene5CScriptStep, X the doubled command, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandCdPlay:
    lda (zScriptPtr),y
    jsr VecCdPlayOnce
    jmp Scene5CScriptAdvance

; @routine 25D3:0E8 Scene5CCommandPlaceKind
; @run 6B:80E8
; WHAT IT DOES: A script command: places an object and sets its kind.
; HOW: Scene5CPlaceObject, then the operand bytes.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandPlaceKind:
    lda (zScriptPtr),y
    tax
    jsr Scene5CPlaceObject
    iny
    lda (zScriptPtr),y
    sta a:OBJ_KIND,x
    jmp Scene5CScriptAdvance

; @routine 25D3:0F7 Scene5CCommandPlaceNext
; @run 6B:80F7
; WHAT IT DOES: A script command: places an object, sets its start argument and picture set, and its kind 2.
; HOW: Scene5CPlaceObject, then the operand bytes.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandPlaceNext:
    lda (zScriptPtr),y
    tax
    jsr Scene5CPlaceObject
    iny
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    lda #$02
    sta a:OBJ_KIND,x
    jmp Scene5CScriptAdvance

; @routine 25D3:111 Scene5CCommandPlaceFrame
; @run 6B:8111
; WHAT IT DOES: A script command: places an object and sets its frame and picture set.
; HOW: Scene5CPlaceObject, then the operand bytes.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandPlaceFrame:
    lda (zScriptPtr),y
    tax
    jsr Scene5CPlaceObject
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FRAME,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    jmp Scene5CScriptAdvance

; @routine 25D3:126 Scene5CCommandPlaceTimed
; @run 6B:8126
; WHAT IT DOES: A script command: places an object, sets its frame, picture set, start argument and work byte D, and its kind 3.
; HOW: Scene5CPlaceObject, then the operand bytes.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandPlaceTimed:
    lda (zScriptPtr),y
    tax
    jsr Scene5CPlaceObject
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
    jmp Scene5CScriptAdvance

; @routine 25D3:14C Scene5CClearObjects
; @run 6B:814C
; WHAT IT DOES: Empties all 88 object slots.
; HOW: A loop of ClearObject.
; WAITS: no.
; Entry: nothing. Exit: X $58, C and Z set, N clear; A and Y kept.
Scene5CClearObjects:
    clx
Scene5CClearObjects_at0:
    jsr VecClearObject
    inx
    cpx #$58
    bne Scene5CClearObjects_at0
    rts

; @routine 25D3:156 Scene5CPlaceObject
; @run 6B:8156
; WHAT IT DOES: Empties an object and gives it the place four script bytes name.
; HOW: ClearObject; X low and high, Y low and high from the bytes at Y plus 1 to 4.
; WAITS: no.
; Entry: X the object, Y the byte before the four. Exit: A the last byte, Y plus 4, N and Z from A; X and C kept.
Scene5CPlaceObject:
    jsr VecClearObject
    iny
    lda (zScriptPtr),y
    sta a:OBJ_X_LO,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_X_HI,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_Y_LO,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_Y_HI,x
    rts

; @routine 25D3:172 Scene5CCommandPlaceTitleObjectA
; @run 6B:8172
; WHAT IT DOES: A script command: object $2E with four operands and its Y at $C8.
; HOW: ClearObject, one added to the kind, four operands.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandPlaceTitleObjectA:
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
    sta a:OBJ_TALLY_WAIT,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_TALLY_STEP,x
    lda #$C8
    sta a:OBJ_Y_LO,x
    jmp Scene5CScriptAdvance

; @routine 25D3:199 Scene5CCommandPlaceTitleObjectB
; @run 6B:8199
; WHAT IT DOES: A script command: object $2F with four operands and its Y at $D8.
; HOW: ClearObject, one added to the kind, four operands.
; WAITS: no.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandPlaceTitleObjectB:
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
    sta a:OBJ_TALLY_WAIT,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_TALLY_STEP,x
    lda #$D8
    sta a:OBJ_Y_LO,x
    jmp Scene5CScriptAdvance

; @routine 25D3:1C0 Scene5CCommandLoadBanks
; @run 6B:81C0
; WHAT IT DOES: A script command: reads four banks of the ADPCM memory into the banks at MPR5 and MPR6.
; HOW: As EndingCommandLoadBanks.
; WAITS: yes, through ad_read, which waits for the ADPCM to be idle.
; Entry: from Scene5CScriptStep, Y 3. Exit: as Scene5CScriptAdvance.
Scene5CCommandLoadBanks:
    stz z:zTemp00
Scene5CCommandLoadBanks_at0:
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
    bne Scene5CCommandLoadBanks_at0
    jmp Scene5CScriptAdvance

; @routine 25D3:1F5 Scene5CObjectsStep
; @run 6B:81F5
; WHAT IT DOES: Runs every object with a kind.
; HOW: A loop over the 88 slots through Scene5CDispatchKind.
; WAITS: no.
; Entry: nothing. Exit: X $58, Z and C set; A the last kind or what its handler left; Y and V as the last handler left them.
Scene5CObjectsStep:
    clx
Scene5CObjectsStep_at1:
    lda a:OBJ_KIND,x
    beq Scene5CObjectsStep_at0
    jsr Scene5CDispatchKind
Scene5CObjectsStep_at0:
    inx
    cpx #$58
    bne Scene5CObjectsStep_at1
    rts

; @routine 25D3:204 Scene5CDispatchKind
; @run 6B:8204
; WHAT IT DOES: Sends an object to the handler its kind names: the table at $8214, kind 1 first.
; HOW: Kind less one doubled; the word through zTemp00 and zTemp01.
; WAITS: no.
; Entry: A the kind, X the object. Exit: as the handler leaves them.
Scene5CDispatchKind:
    dec a
    asl a
    tay
    lda a:Scene5CKinds,y
    sta z:zTemp00
    lda a:Scene5CKinds+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 25D3:220 Scene5CShowFirstObject
; @run 6B:8220
; WHAT IT DOES: Gives object 0 attribute $10.
; HOW: One store.
; WAITS: no.
; Entry: X the object. Exit: A $10, N and Z from it; X, Y and C kept.
Scene5CShowFirstObject:
    lda #$10
    sta a:OBJ_SPRITE_ATTR
    rts

; @routine 25D3:226 Scene5CScrollUpAndClear
; @run 6B:8226
; WHAT IT DOES: Scrolls the screen's X by eight a frame for 32 frames, then puts the scroll's high byte back and clears the object.
; HOW: OBJ->state; ScrollXLo; work_d counted down; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
Scene5CScrollUpAndClear:
    lda a:OBJ_STATE,x
    bne Scene5CScrollUpAndClear_at0
    lda #$20
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
    rts
Scene5CScrollUpAndClear_at0:
    lda a:ScrollXLo
    clc
    adc #$08
    sta a:ScrollXLo
    dec a:OBJ_FLASH_FRAMES,x
    beq Scene5CScrollUpAndClear_at1
    rts
Scene5CScrollUpAndClear_at1:
    stz a:ScrollXHi
    jmp VecClearObject

; @routine 25D3:249 Scene5CDriftFiveObjects
; @run 6B:8249
; WHAT IT DOES: Drifts objects 0 to 3 and 15 left at a quarter dot over a scrolling screen for 96 frames, then puts the scroll back and clears itself.
; HOW: OBJ->state; ScrollXLo; the five slots from $8284 given their X speed and moved by ApplyVelocityX; work_d.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
Scene5CDriftFiveObjects:
    lda a:OBJ_STATE,x
    bne Scene5CDriftFiveObjects_at0
    lda #$A0
    sta a:ScrollXLo
    lda #$60
    sta a:OBJ_FLASH_FRAMES,x
    inc a:OBJ_STATE,x
    rts
Scene5CDriftFiveObjects_at0:
    inc a:ScrollXLo
    phx
    cly
Scene5CDriftFiveObjects_at1:
    lda a:Scene5CDrifters,y
    tax
    lda #$A0
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    iny
    cpy #$05
    bne Scene5CDriftFiveObjects_at1
    plx
    dec a:OBJ_FLASH_FRAMES,x
    beq Scene5CDriftFiveObjects_at2
    rts
Scene5CDriftFiveObjects_at2:
    stz a:ScrollXHi
    jmp VecClearObject

; @routine 25D3:293 Scene5CPlayListOnce
; @run 6B:8293
; WHAT IT DOES: Plays the object's start argument's frame list until it ends, then moves it on a state unless the argument is negative.
; HOW: As PlayFrameListOnce, with this overlay's lists.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
Scene5CPlayListOnce:
    lda a:OBJ_STATE,x
    bne Scene5CPlayListOnce_at0
    lda a:OBJ_START_ARG,x
    jsr Scene5CFrameList
    bne Scene5CPlayListOnce_at0
    lda a:OBJ_START_ARG,x
    bmi Scene5CPlayListOnce_at0
    inc a:OBJ_STATE,x
Scene5CPlayListOnce_at0:
    rts

; @routine 25D3:2A9 Scene5CSlideForCount
; @run 6B:82A9
; WHAT IT DOES: As SlideForCount, with the speeds from this overlay's four tables by the start argument.
; HOW: OBJ->state; tables at $82D9, $82DE, $82E3, $82E8; ApplyVelocity; work_d.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
Scene5CSlideForCount:
    lda a:OBJ_STATE,x
    bne Scene5CSlideForCount_at0
    ldy a:OBJ_START_ARG,x
    lda a:Scene5CSpeedsXL,y
    sta a:OBJ_VX_LO,x
    lda a:Scene5CSpeedsXH,y
    sta a:OBJ_VX_HI,x
    lda a:Scene5CSpeedsYL,y
    sta a:OBJ_VY_LO,x
    lda a:Scene5CSpeedsYH,y
    sta a:OBJ_VY_HI,x
Scene5CSlideForCount_at1:
    inc a:OBJ_STATE,x
    rts
Scene5CSlideForCount_at0:
    dec a
    bne Scene5CSlideForCount_t82D8
    jsr VecApplyVelocity
    dec a:OBJ_FLASH_FRAMES,x
    beq Scene5CSlideForCount_at1
Scene5CSlideForCount_t82D8:
    rts

; @routine 25D3:2ED Scene5CFrameList
; @run 6B:82ED
; WHAT IT DOES: Steps an object through the frame list the table at $82FC names for A.
; HOW: As ObjectFrameList with this table.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames.
Scene5CFrameList:
    asl a
    tay
    lda a:Scene5CFrameLists,y
    sta z:zTemp10
    lda a:Scene5CFrameLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames
