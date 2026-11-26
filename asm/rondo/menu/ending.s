; @routine 010D:000 EndingStep
; @run 6B:8000
; WHAT IT DOES: One call of the ending's mode: the handler of the sub-mode zGameSubMode names in the table at $8007 (0 begins the ending,
; 1 runs a frame of it).
; HOW: Reads zGameSubMode, doubles it and jumps through the table. A word this overlay does not hold runs as the original code.
; WAITS: yes, through EndingBegin, which waits for the vertical blank.
; Entry: nothing. Exit: as the handler leaves them.
; not checked: one case of 400 jumps to a table word outside this overlay's C.
EndingStep:
    lda z:zGameSubMode
    asl a
    tax
    jmp (FrameOverlay8006+1,x)

; @routine 010D:00B EndingBegin
; @run 6B:800B
; WHAT IT DOES: Starts the ending: the game state is wiped, the script is set to run from $8AA3, its frame counter to $FFFF and the
; state goes on.
; HOW: ResetGameState; the script pointer and the counter are written; zGameSubMode counted up.
; WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off; the stack there is this routine's
; return address and the call's.
; Entry: X the doubled sub-mode and Y kept (ResetGameState saves them). Exit: A $FF, X 0, Y $FF, N and Z from the new state; C kept.
EndingBegin:
    jsr VecResetGameState
    lda #$A3
    sta z:$7E
    lda #$8A
    sta z:$7F
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    rts

; @routine 010D:01F EndingFrame
; @run 6B:801F
; WHAT IT DOES: One frame of the ending: the frame counted, the script's next command run if it is due, and every object run.
; HOW: EndingFrameTick, EndingScriptStep, EndingObjectsStep.
; WAITS: no.
; Entry: nothing. Exit: as EndingObjectsStep.
; not checked: as EndingObjectsStep: only the idle loop is compared.
EndingFrame:
    jsr EndingFrameTick
    jsr EndingScriptStep
    jsr EndingObjectsStep
    rts

; @routine 010D:029 EndingFrameTick
; @run 6B:8029
; WHAT IT DOES: Counts one frame of the ending scene on its 16-bit frame counter.
; HOW: Adds one to zWaitTimer3, and to zWaitTimer4 when it wrapped to zero.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the byte last incremented, C and the registers kept.
EndingFrameTick:
    inc z:zWaitTimer3
    bne EndingFrameTick_at0
    inc z:zWaitTimer4
EndingFrameTick_at0:
    rts

; @routine 010D:040 EndingScriptAdvance
; @run 6B:8040
; WHAT IT DOES: Steps the ending's script past the command it is at, and runs on into EndingScriptStep.
; HOW: The command byte indexes a table of lengths, which is added to the pointer (the high byte goes up on a carry).
; WAITS: no.
; Entry: nothing. Exit: as EndingScriptStep, X the command stepped past (when EndingScriptStep left X alone), V from the pointer's add
; (when it set none).
EndingScriptAdvance:
    lda ($7E)
    tax
    lda a:TitleSteps+2,x
    clc
    adc z:$7E
    sta z:$7E
    bcc EndingScriptStep
    inc z:$7F

; @routine 010D:04F EndingScriptStep
; @run 6B:804F
; WHAT IT DOES: Runs the ending's script command if its time has come: the two time bytes after the command byte equal the frame counter.
; HOW: Compares the first time byte with zWaitTimer3 and the second with zWaitTimer4 and returns on a difference; otherwise the
; command, doubled, indexes the table at $806A and it is jumped to with X the doubled command and Y 3.
; WAITS: no.
; Entry: nothing. Exit, a time byte differs: A that byte, N, Z and C from comparing it, Y 1 or 2, X kept. Otherwise as the command leaves them.
EndingScriptStep:
    ldy #$01
    lda ($7E),y
    cmp z:zWaitTimer3
    bne $8028
    iny
    lda ($7E),y
    cmp z:zWaitTimer4
    bne $8028
    lda ($7E)
    asl a
    tax
    ldy #$03
    jmp ($806A,x)

; @routine 010D:067 EndingModeDoneBack
; @run 6B:8067
; WHAT IT DOES: Takes one back off zModeDone.
; HOW: One decrement.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the new value, C and the registers kept.
EndingModeDoneBack:
    dec z:zModeDone
    rts

; @routine 010D:090 EndingCommandStartList
; @run 6B:8090
; WHAT IT DOES: A script command: starts list channel 0 with the operand's list, clears the screen's state and all the object slots, and
; clears the scroll block again.
; HOW: StartListChannel0Kind2, ClearScreenState, EndingClearObjects, a TAI from $4003 to $3E00; back to EndingScriptAdvance.
; WAITS: no.
; Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
EndingCommandStartList:
    lda ($7E),y
    jsr VecStartListChannel0Kind2
    jsr VecClearScreenState
    jsr EndingClearObjects
    tai KernelZeros,ScrollXFrac,$00EE
    bra EndingScriptAdvance

; @routine 010D:0AB EndingCommandStartScript1
; @run 6B:80AB
; WHAT IT DOES: A script command: starts script slot 0 as kind 1 with the operand's script.
; HOW: StartScriptSlot0Kind1; back to EndingScriptAdvance.
; WAITS: no.
; Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
EndingCommandStartScript1:
    lda ($7E),y
    jsr VecStartScriptSlot0Kind1
    bra EndingScriptAdvance

; @routine 010D:0B2 EndingCommandStartScript2
; @run 6B:80B2
; WHAT IT DOES: A script command: starts script slot 0 as kind 2 with the operand's script.
; HOW: StartScriptSlot0Kind2; back to EndingScriptAdvance.
; WAITS: no.
; Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
EndingCommandStartScript2:
    lda ($7E),y
    jsr VecStartScriptSlot0Kind2
    jmp EndingScriptAdvance

; @routine 010D:0C5 EndingCommandCdPlay
; @run 6B:80C5
; WHAT IT DOES: A script command: plays the CD track the operand names, once.
; HOW: CdPlayOnce; back to EndingScriptAdvance.
; WAITS: no.
; Entry: from EndingScriptStep, X the doubled command, Y 3. Exit: as EndingScriptAdvance.
EndingCommandCdPlay:
    lda ($7E),y
    jsr VecCdPlayOnce
    jmp EndingScriptAdvance

; @routine 010D:0CD EndingCommandPlaceKind
; @run 6B:80CD
; WHAT IT DOES: A script command: places an object (empties it and gives it the four place bytes) and sets its kind from the byte after.
; HOW: ScriptPlaceObject; one store; back to EndingScriptAdvance.
; WAITS: no.
; Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
EndingCommandPlaceKind:
    lda ($7E),y
    tax
    jsr EndingPlaceObject
    iny
    lda ($7E),y
    sta a:OBJ_2750,x
    jmp EndingScriptAdvance

; @routine 010D:0DC EndingCommandPlaceNext
; @run 6B:80DC
; WHAT IT DOES: A script command: places an object, sets its next kind and picture kind from the two bytes after, and its kind to 2.
; HOW: ScriptPlaceObject; three stores; back to EndingScriptAdvance.
; WAITS: no.
; Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
EndingCommandPlaceNext:
    lda ($7E),y
    tax
    jsr EndingPlaceObject
    iny
    lda ($7E),y
    sta a:OBJ_3040,x
    iny
    lda ($7E),y
    sta a:OBJ_2A10,x
    lda #$02
    sta a:OBJ_2750,x
    jmp EndingScriptAdvance

; @routine 010D:0F6 EndingCommandPlaceFrame
; @run 6B:80F6
; WHAT IT DOES: A script command: places an object and sets its frame and picture kind from the two bytes after.
; HOW: ScriptPlaceObject; two stores; back to EndingScriptAdvance.
; WAITS: no.
; Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
EndingCommandPlaceFrame:
    lda ($7E),y
    tax
    jsr EndingPlaceObject
    iny
    lda ($7E),y
    sta a:OBJ_2A68,x
    iny
    lda ($7E),y
    sta a:OBJ_2A10,x
    jmp EndingScriptAdvance

; @routine 010D:10B EndingCommandPlaceTimed
; @run 6B:810B
; WHAT IT DOES: A script command: places an object and sets its frame, picture kind, next kind and hold from the four bytes after, and its kind to 3.
; HOW: ScriptPlaceObject; five stores; back to EndingScriptAdvance.
; WAITS: no.
; Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
EndingCommandPlaceTimed:
    lda ($7E),y
    tax
    jsr EndingPlaceObject
    iny
    lda ($7E),y
    sta a:OBJ_2A68,x
    iny
    lda ($7E),y
    sta a:OBJ_2A10,x
    iny
    lda ($7E),y
    sta a:OBJ_3040,x
    iny
    lda ($7E),y
    sta a:OBJ_34B8,x
    lda #$03
    sta a:OBJ_2750,x
    jmp EndingScriptAdvance

; @routine 010D:131 EndingClearObjects
; @run 6B:8131
; WHAT IT DOES: Empties all 88 object slots.
; HOW: Sets X to 0 and runs into a loop of ClearObject.
; WAITS: no.
; Entry: nothing. Exit: X $58, C and Z set, N clear; A and Y kept.
EndingClearObjects:
    clx
EndingClearObjects_at0:
    jsr VecClearObject
    inx
    cpx #$58
    bne EndingClearObjects_at0
    rts

; @routine 010D:13B EndingPlaceObject
; @run 6B:813B
; WHAT IT DOES: Empties an object and gives it the place four script bytes name (the copy of ScriptPlaceObject in this overlay).
; HOW: ClearObject; then the bytes at Y plus 1 to 4 go to the object's X low, X high, Y low and Y high.
; WAITS: no.
; Entry: X the object, Y the script byte before the four. Exit: A the last byte read, Y plus 4, N and Z from A; X and C kept.
EndingPlaceObject:
    jsr VecClearObject
    iny
    lda ($7E),y
    sta a:OBJ_X_LO,x
    iny
    lda ($7E),y
    sta a:OBJ_X_HI,x
    iny
    lda ($7E),y
    sta a:OBJ_Y_LO,x
    iny
    lda ($7E),y
    sta a:OBJ_Y_HI,x
    rts

; @routine 010D:1A5 EndingCommandLoadBanks
; @run 6B:81A5
; WHAT IT DOES: A script command: reads four banks of the ADPCM memory into the RAM banks at MPR5 and MPR6, one after another.
; HOW: For each of four numbers the two banks are mapped, the ADPCM address and the length set, and ad_read called; back to EndingScriptAdvance.
; WAITS: yes, through ad_read, which waits for the ADPCM to be idle.
; Entry: from EndingScriptStep, Y 3. Exit: as EndingScriptAdvance.
; not checked: differs inside the card's ad_read, whose draft pushes frames its C does not.
EndingCommandLoadBanks:
    stz z:zTemp00
EndingCommandLoadBanks_at0:
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
    jsr $E036
    inc z:zTemp00
    lda z:zTemp00
    cmp #$04
    bne EndingCommandLoadBanks_at0
    jmp EndingScriptAdvance

; @routine 010D:1DA EndingObjectsStep
; @run 6B:81DA
; WHAT IT DOES: Runs every object of the ending scene that has a kind: a loop over the 88 slots, each with a kind sent to its handler.
; HOW: OBJ->kind read for each slot; EndingDispatchKind for the ones that are not zero.
; WAITS: no.
; Entry: nothing. Exit: X $58, N and Z clear... Z set, C set (the loop's last compare); A the last slot's kind or what its handler left;
; Y and V as the last handler left them.
; not checked: the jump to a handler is computed and through mode cannot follow it; only the idle loop is compared (curated/ending_objects_idle.txt).
EndingObjectsStep:
    clx
EndingObjectsStep_at1:
    lda a:OBJ_2750,x
    beq EndingObjectsStep_at0
    jsr EndingDispatchKind
EndingObjectsStep_at0:
    inx
    cpx #$58
    bne EndingObjectsStep_at1
    rts

; @routine 010D:1E9 EndingDispatchKind
; @run 6B:81E9
; WHAT IT DOES: Sends an object to the handler its kind names: the table at $81F9 holds one address per kind, kind 1 first.
; HOW: Kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01; the jump is made with A the word's high
; byte, Y the index, C bit 7 of kind less one, X the object.
; WAITS: no.
; Entry: A the kind (not zero), X the object. Exit: as the handler leaves them.
EndingDispatchKind:
    dec a
    asl a
    tay
    lda a:$81F9,y
    sta z:zTemp00
    lda a:$81FA,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 010D:239 EndingCreditScrollStart
; @run 6B:8239
; WHAT IT DOES: An object handler: every frame starts list $18 plus the frame count's two low bits on channel 7 as kind 2. In its first
; state it also starts script $1A on slot 0, holds the object 6 frames, moves the first scroll's X high byte and Y high byte and
; Y low byte, and moves on. In the next states, until state $0A, once the hold has run out it takes the first scroll's X and Y high
; bytes from a table at $8284 by state, holds 6 frames and moves on.
; HOW: OBJ->state; StartListChannelKind2; StartScriptSlot0Kind1; EndingHoldTimer; SCROLL.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingCreditScrollStart:
    phx
    lda z:zFrameTick
    and #$03
    clc
    adc #$18
    ldx #$07
    jsr VecStartListChannelKind2
    plx
    lda a:OBJ_27A8,x
    bne ModeDoneAdd
    phx
    lda #$1A
    jsr VecStartScriptSlot0Kind1
    plx
    lda #$06
    sta a:OBJ_34B8,x
    inc a:ScrollXHi
    inc a:ScrollYHi
    dec a:ScrollYLo
    inc a:OBJ_27A8,x
    rts
    cmp #$0A
    beq EndingCreditScrollStart_at0
    jsr EndingHoldTimer
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$8284,y
    sta a:ScrollXHi
    lda a:$8285,y
    sta a:ScrollYHi
    lda #$06
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
EndingCreditScrollStart_at0:
    rts

; @routine 010D:298 EndingFallingPiece
; @run 6B:8298
; WHAT IT DOES: An object handler of two stages: the first takes its Y speed from a table at $82D5 by its number, shows picture kind 3
; and frame $72 plus its number and moves on; after that each frame it sets flash flags from a table at $82DA by bit 2 of its number
; plus the frame count, moves, and clears the object's Y high byte to 1 once it is negative with a non-negative Y low byte.
; HOW: OBJ->state; OBJ->flash_flags; ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingFallingPiece:
    lda a:OBJ_27A8,x
    bne EndingFallingPiece_at0
    lda a:$82D5,x
    sta a:OBJ_VY_HI,x
    lda #$03
    sta a:OBJ_2A10,x
    txa
    clc
    adc #$72
    sta a:OBJ_2A68,x
    inc a:OBJ_27A8,x
    rts
EndingFallingPiece_at0:
    txa
    clc
    adc z:zFrameTick
    lsr a
    lsr a
    and #$01
    tay
    lda a:$82DA,y
    sta a:OBJ_2B18,x
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bpl EndingFallingPiece_at1
    lda a:OBJ_Y_LO,x
    bmi EndingFallingPiece_at1
    lda #$01
    sta a:OBJ_Y_HI,x
EndingFallingPiece_at1:
    rts

; @routine 010D:2DC EndingClearAfterScrollX
; @run 6B:82DC
; WHAT IT DOES: An object handler: counts the first scroll's X high byte up and clears the object.
; HOW: One increment; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: N and Z from the new byte; A, X, Y and C kept.
EndingClearAfterScrollX:
    inc a:ScrollXHi
    jmp VecClearObject

; @routine 010D:2E2 EndingClearAfterScrollY
; @run 6B:82E2
; WHAT IT DOES: An object handler: counts the first scroll's Y high byte up and clears the object.
; HOW: One increment; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: N and Z from the new byte; A, X, Y and C kept.
EndingClearAfterScrollY:
    inc a:ScrollYHi
    jmp VecClearObject

; @routine 010D:2E8 EndingPieceSpawn
; @run 6B:82E8
; WHAT IT DOES: An object handler of two stages: the first waits for object 1's X to be $0078 (high byte 0 and low $78) and moves on;
; the second makes objects $10 to $2E: each cleared, shown as picture kind 3, kind $1A, frame $40 plus its number, X low and
; Y low from tables at $831C and $833B; then clears object 1's frame and the object itself.
; HOW: OBJ->state; ClearObject; two table reads a piece.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingPieceSpawn:
    lda a:OBJ_27A8,x
    bne EndingPieceSpawn_at0
    lda a:OBJ_X_HI+1
    bne EndingPieceSpawn_at1
    lda a:OBJ_X_LO+1
    cmp #$78
    bne EndingPieceSpawn_at1
    inc a:OBJ_27A8,x
EndingPieceSpawn_at1:
    rts
EndingPieceSpawn_at0:
    phx
    ldx #$10
EndingPieceSpawn_at2:
    jsr VecClearObject
    lda #$03
    sta a:OBJ_2A10,x
    lda #$1A
    sta a:OBJ_2750,x
    txa
    clc
    adc #$40
    sta a:OBJ_2A68,x
    lda a:$831C,x
    sta a:OBJ_X_LO,x
    lda a:$833B,x
    sta a:OBJ_Y_LO,x
    inx
    cpx #$2F
    bne EndingPieceSpawn_at2
    plx
    stz a:OBJ_2A68+1
    jmp VecClearObject

; @routine 010D:36A EndingAimAtPlace
; @run 6B:836A
; WHAT IT DOES: An object handler of two stages: the first makes the object's speed the distance to the place (X $70, Y $58)
; times 8 and moves to the next state; the second adds $18 to the downward speed and moves.
; HOW: OBJ->state; subtracts and three shifts of the 16-bit speeds; ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingAimAtPlace:
    lda a:OBJ_27A8,x
    bne EndingAimAtPlace_at0
    lda a:OBJ_X_LO,x
    sec
    sbc #$70
    sta a:OBJ_VX_LO,x
    lda a:OBJ_X_HI,x
    sbc #$00
    sta a:OBJ_VX_HI,x
    lda a:OBJ_Y_LO,x
    sec
    sbc #$58
    sta a:OBJ_VY_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    asl a:OBJ_VX_LO,x
    rol a:OBJ_VX_HI,x
    asl a:OBJ_VY_LO,x
    rol a:OBJ_VY_HI,x
    asl a:OBJ_VX_LO,x
    rol a:OBJ_VX_HI,x
    asl a:OBJ_VY_LO,x
    rol a:OBJ_VY_HI,x
    asl a:OBJ_VX_LO,x
    rol a:OBJ_VX_HI,x
    asl a:OBJ_VY_LO,x
    rol a:OBJ_VY_HI,x
    inc a:OBJ_27A8,x
    rts
EndingAimAtPlace_at0:
    dec a
    bne EndingAimAtPlace_at1
    lda #$18
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,x
    bcc EndingAimAtPlace_at2
    inc a:OBJ_VY_HI,x
EndingAimAtPlace_at2:
    jsr VecApplyVelocity
EndingAimAtPlace_at1:
    rts

; @routine 010D:3CE EndingScrollPull
; @run 6B:83CE
; WHAT IT DOES: An object handler of two states: the first sets the first scroll's X low byte to $80; the second
; takes one off the X of objects 1, 2 and 3 a frame (with the borrow into the high bytes of objects 1 and 3)
; and counts the first scroll's X low byte down, moving on when it reaches 0.
; HOW: OBJ->state; decrements with a test for $FF.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollPull:
    lda a:OBJ_27A8,x
    bne EndingScrollPull_at0
    lda #$80
    sta a:ScrollXLo
    inc a:OBJ_27A8,x
    rts
EndingScrollPull_at0:
    dec a
    bne EndingScrollPull_at1
    dec a:OBJ_X_LO+1
    lda a:OBJ_X_LO+1
    cmp #$FF
    bne EndingScrollPull_at2
    dec a:OBJ_X_HI+1
EndingScrollPull_at2:
    dec a:OBJ_X_LO+2
    dec a:OBJ_X_LO+3
    lda a:OBJ_X_LO+3
    cmp #$FF
    bne EndingScrollPull_at3
    dec a:OBJ_X_HI+3
EndingScrollPull_at3:
    dec a:ScrollXLo
    bne EndingScrollPull_at1
    inc a:OBJ_27A8,x
EndingScrollPull_at1:
    rts

; @routine 010D:405 EndingScrollRunUp
; @run 6B:8405
; WHAT IT DOES: An object handler: in its first state counts the first scroll's X low byte up each frame until
; it reaches $80, then moves the object to its next state.
; HOW: OBJ->state; one increment of SCROLL->x_lo[0] and a compare.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollRunUp:
    lda a:OBJ_27A8,x
    bne EndingScrollRunUp_at0
    inc a:ScrollXLo
    lda a:ScrollXLo
    cmp #$80
    bne EndingScrollRunUp_at0
    inc a:OBJ_27A8,x
EndingScrollRunUp_at0:
    rts

; @routine 010D:418 EndingPlayList10
; @run 6B:8418
; WHAT IT DOES: An object handler: shows picture kind 3, waits a frame, then plays the ending's list $10 until it
; ends and moves to the next state.
; HOW: OBJ->state and OBJ->sprite_set; EndingFrameList.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingPlayList10:
    lda a:OBJ_27A8,x
    bne EndingPlayList10_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
    rts
EndingPlayList10_at0:
    dec a
    bne EndingPlayList10_at1
EndingPlayList10_at2:
    rts
EndingPlayList10_at1:
    dec a
    bne EndingPlayList10_at2
    lda #$10
    jsr EndingFrameList
    bne EndingPlayList10_at2
    inc a:OBJ_27A8,x
    rts

; @routine 010D:438 EndingScrollWindowSteps
; @run 6B:8438
; WHAT IT DOES: An object handler of four states: the first starts the raster effect $13, sets scroll slots 1 and 5 to Y
; $1F and 2 and 6 to $AF and holds the object $20 frames; the second waits the hold out; the third moves on; the
; fourth moves scroll slot 1's X on by 4 a frame and, when it wraps to 0, moves the first object and scroll slot 1's
; X high byte on and the object to its next state.
; HOW: OBJ->state; EndingHoldTimer; SCROLL->x_lo and x_hi.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingScrollWindowSteps:
    lda a:OBJ_27A8,x
    bne EndingScrollWindowSteps_at0
    lda #$13
    sta z:zRasterEffect
    lda #$1F
    sta a:ScrollYLo+5
    sta a:ScrollYLo+1
    lda #$AF
    sta a:ScrollYLo+6
    sta a:ScrollYLo+2
    lda #$20
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts
EndingScrollWindowSteps_at0:
    dec a
    bne EndingScrollWindowSteps_at1
    jsr EndingHoldTimer
    inc a:OBJ_27A8,x
    rts
EndingScrollWindowSteps_at1:
    dec a
    bne EndingScrollWindowSteps_at2
    inc a:OBJ_27A8,x
    rts
EndingScrollWindowSteps_at2:
    dec a
    bne EndingScrollWindowSteps_at3
    lda a:ScrollXLo+1
    clc
    adc #$04
    sta a:ScrollXLo+1
    bne EndingScrollWindowSteps_at4
    inc a:OBJ_27A8
    inc a:ScrollXHi+1
    inc a:OBJ_27A8,x
EndingScrollWindowSteps_at4:
    rts
EndingScrollWindowSteps_at3:
    rts

; @routine 010D:484 EndingScrollWindowA
; @run 6B:8484
; WHAT IT DOES: An object handler: in its first state starts the raster effect $13 and sets scroll slots 1 and 5 to Y
; $1F and 2 and 6 to $BF, then moves to the next state.
; HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollWindowA:
    lda a:OBJ_27A8,x
    bne EndingScrollWindowA_at0
    lda #$13
    sta z:zRasterEffect
    lda #$1F
    sta a:ScrollYLo+5
    sta a:ScrollYLo+1
    lda #$BF
    sta a:ScrollYLo+6
    sta a:ScrollYLo+2
    inc a:OBJ_27A8,x
EndingScrollWindowA_at0:
    rts

; @routine 010D:4A1 EndingScrollWindowB
; @run 6B:84A1
; WHAT IT DOES: As EndingScrollWindowA with the Y values $27 and $C7.
; HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollWindowB:
    lda a:OBJ_27A8,x
    bne EndingScrollWindowA_at0
    lda #$13
    sta z:zRasterEffect
    lda #$27
    sta a:ScrollYLo+5
    sta a:ScrollYLo+1
    lda #$C7
    sta a:ScrollYLo+6
    sta a:ScrollYLo+2
    inc a:OBJ_27A8,x
    rts

; @routine 010D:4BE EndingPlayList0F
; @run 6B:84BE
; WHAT IT DOES: As EndingPlayList10, with the ending's list $0F.
; HOW: OBJ->state and OBJ->sprite_set; EndingFrameList.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingPlayList0F:
    lda a:OBJ_27A8,x
    bne EndingPlayList0F_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
    rts
EndingPlayList0F_at0:
    dec a
    bne EndingPlayList0F_at1
EndingPlayList0F_at2:
    rts
EndingPlayList0F_at1:
    dec a
    bne EndingPlayList0F_at2
    lda #$0F
    jsr EndingFrameList
    bne EndingPlayList0F_at2
    inc a:OBJ_27A8,x
    rts

; @routine 010D:4DE EndingRiseToY40
; @run 6B:84DE
; WHAT IT DOES: An object handler: shows picture kind 3, waits a frame, then shows frame $0D and moves the object up
; a line each frame until its Y low byte is $40, then sets flash flags $8D and moves to the next state.
; HOW: OBJ->state, OBJ->sprite_set, OBJ->frame, OBJ->y_lo and OBJ->flash_flags.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingRiseToY40:
    lda a:OBJ_27A8,x
    bne EndingRiseToY40_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
    rts
EndingRiseToY40_at0:
    dec a
    bne EndingRiseToY40_at1
EndingRiseToY40_at3:
    rts
EndingRiseToY40_at1:
    dec a
    bne EndingRiseToY40_at2
    lda #$0D
    sta a:OBJ_2A68,x
    dec a:OBJ_Y_LO,x
    lda a:OBJ_Y_LO,x
    cmp #$40
    bne EndingRiseToY40_at3
    lda #$8D
    sta a:OBJ_2B18,x
    inc a:OBJ_27A8,x
    rts
EndingRiseToY40_at2:
    rts

; @routine 010D:50C EndingPlayList0EThenClear
; @run 6B:850C
; WHAT IT DOES: An object handler: shows picture kind 3, waits a frame, then plays the ending's list $0E until it
; ends and clears the object.
; HOW: OBJ->state and OBJ->sprite_set; EndingFrameList; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingPlayList0EThenClear:
    lda a:OBJ_27A8,x
    bne EndingPlayList0EThenClear_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
    rts
EndingPlayList0EThenClear_at0:
    dec a
    bne EndingPlayList0EThenClear_at1
EndingPlayList0EThenClear_at2:
    rts
EndingPlayList0EThenClear_at1:
    lda #$0E
    jsr EndingFrameList
    bne EndingPlayList0EThenClear_at2
    jmp VecClearObject

; @routine 010D:548 EndingScrollWindowHold
; @run 6B:8548
; WHAT IT DOES: An object handler: starts the raster effect $13, sets scroll slots 1 and 5 to Y $0F and 2 and 6 to $6F,
; holds the object for $40 frames and moves to the next state.
; HOW: ZP->raster_effect; SCROLL->y_lo; OBJ->life_timer.
; WAITS: no.
; Entry: X the object. Exit: A $40, N and Z from the new state; C and the registers kept.
EndingScrollWindowHold:
    lda #$13
    sta z:zRasterEffect
    lda #$0F
    sta a:ScrollYLo+5
    sta a:ScrollYLo+1
    lda #$6F
    sta a:ScrollYLo+6
    sta a:ScrollYLo+2
    lda #$40
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts

; @routine 010D:565 EndingStartList3
; @run 6B:8565
; WHAT IT DOES: An object handler: once its hold has run out starts list 0 on channel 7 as kind 3, holds the object $20
; frames and moves to the next state.
; HOW: EndingHoldTimer (which returns to the dispatch while the hold lasts); StartListChannelKind3; OBJ->life_timer.
; WAITS: no.
; Entry: X the object, A as the dispatch left it. Exit: A $20, N and Z from the new state; C and the registers kept.
; While the hold lasts: A kept, N and Z from the timer.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingStartList3:
    jsr EndingHoldTimer
    phx
    cla
    ldx #$07
    jsr VecStartListChannelKind3
EndingStartList3_at0:
    plx
    lda #$20
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts

; @routine 010D:579 EndingStartScript18
; @run 6B:8579
; WHAT IT DOES: An object handler: once its hold has run out starts script $18 on slot 7 as kind 1 and moves object 2's frame,
; object 3's X, scroll slot 2's X high byte and low byte, and the object's state on.
; HOW: EndingHoldTimer; StartScriptSlotKind1; five counts.
; WAITS: no.
; Entry: X the object, A as the dispatch left it. Exit: A and Y $18, N and Z from the new state; C and X kept.
; While the hold lasts: A kept, N and Z from the timer.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingStartScript18:
    jsr EndingHoldTimer
    phx
    lda #$18
    ldx #$07
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_2A68+2
    inc a:OBJ_X_LO+3
    inc a:OBJ_X_LO+3
    inc a:ScrollXHi+2
    dec a:ScrollXLo+2
    inc a:OBJ_27A8,x
    rts

; @routine 010D:598 EndingScrollSlide
; @run 6B:8598
; WHAT IT DOES: An object handler: takes 4 off scroll slot 2's X low byte; while the result is $80 or more it is stored and
; nothing else happens; below that it counts scroll slot 6's Y high byte up, holds the object $30 frames, counts object
; 4's state, starts script $17 on slot 7 as kind 1, moves the object on and stores $80.
; HOW: SCROLL->x_lo; StartScriptSlotKind1.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollSlide:
    lda a:ScrollXLo+2
    sec
    sbc #$04
    cmp #$80
    bcs EndingScrollSlide_at0
    inc a:ScrollYHi+6
    lda #$30
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8+4
    phx
    lda #$17
    ldx #$07
    jsr VecStartScriptSlotKind1
    plx
    lda #$80
    inc a:OBJ_27A8,x
EndingScrollSlide_at0:
    sta a:ScrollXLo+2
    rts

; @routine 010D:5BF EndingSplitScrolls
; @run 6B:85BF
; WHAT IT DOES: An object handler: once its hold has run out resets scroll slots 2 and 6 and moves slots 1 and 2's Y high bytes on,
; sets slot 2's Y low byte to $6F, holds the object $20 frames and moves objects 1 and this one to their next states.
; HOW: EndingHoldTimer; stores and counts on SCROLL.
; WAITS: no.
; Entry: X the object, A as the dispatch left it. Exit: A $20, N and Z from the new state; C and the registers kept.
; While the hold lasts: A kept, N and Z from the timer.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingSplitScrolls:
    jsr EndingHoldTimer
    stz a:ScrollYHi+6
    inc a:ScrollYHi+1
    inc a:ScrollYHi+2
    lda #$6F
    sta a:ScrollYLo+2
    stz a:ScrollXHi+2
    stz a:ScrollXLo+2
    lda #$20
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8+1
    inc a:OBJ_27A8,x
    rts

; @routine 010D:5E2 EndingScrollsOn
; @run 6B:85E2
; WHAT IT DOES: An object handler: once its hold has run out counts scroll slot 6's Y high byte and slot 2's X high byte up,
; counts object 6's state and moves this object to its next state.
; HOW: EndingHoldTimer; four counts.
; WAITS: no.
; Entry: X the object, A as the dispatch left it. Exit: A kept, N and Z from the new state; C and the registers kept.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingScrollsOn:
    jsr EndingHoldTimer
    inc a:ScrollYHi+6
    inc a:ScrollXHi+2
    inc a:OBJ_27A8+6
    inc a:OBJ_27A8,x
    rts

; @routine 010D:5F2 EndingNothing
; @run 6B:85F2
; WHAT IT DOES: Returns at once.
; HOW: An RTS.
; WAITS: no.
; Entry: nothing. Exit: everything kept.
EndingNothing:
    rts

; @routine 010D:5F3 EndingHoldTimer
; @run 6B:85F3
; WHAT IT DOES: Counts down an object's hold timer; while it has not run out the caller is not returned to:
; the routine returns to the caller's caller instead.
; HOW: OBJ->life_timer counted down; when it is not zero the return address of the call is pulled off the
; stack (two PLA) before the RTS.
; WAITS: no.
; Entry: X the object, the caller's frame on the stack. Exit: while the hold lasts A the high byte of the caller's return
; address (the last PLA), N and Z from it; when it has run out N and Z from the timer (zero). X, Y and C kept.
; not checked: it pulls its caller's frame and cannot run alone; its callers compare it.
EndingHoldTimer:
    dec a:OBJ_34B8,x
    beq EndingHoldTimer_at0
    pla
    pla
EndingHoldTimer_at0:
    rts

; @routine 010D:5FB EndingScrollSteps
; @run 6B:85FB
; WHAT IT DOES: An object handler of five stages: the first counts the first scroll's X and Y high bytes up; the next three each
; take 8 off the first scroll's X low byte until it is $F8, then clear its X high byte, start the ending's list $0D on this channel
; and count its Y high byte up; the fifth adds $10 to object 1's X (with the carry into its high byte) and takes 8 off the scroll's
; X low byte until it is 0.
; HOW: OBJ->state; EndingScrollBack8; StartListChannelKind2.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollSteps:
    lda a:OBJ_27A8,x
    bne EndingScrollSteps_at0
    inc a:ScrollXHi
    inc a:ScrollYHi
    inc a:OBJ_27A8,x
EndingScrollSteps_at2:
    rts
EndingScrollSteps_at0:
    dec a
    bne EndingScrollSteps_at1
    jsr EndingScrollBack8
    cmp #$F8
    bne EndingScrollSteps_at2
    stz a:ScrollXHi
    inc a:OBJ_27A8,x
    rts
EndingScrollSteps_at1:
    dec a
    bne EndingScrollSteps_at3
    jsr EndingScrollBack8
    cmp #$F8
    bne EndingScrollSteps_at2
    stz a:ScrollYHi
    phx
    lda #$0D
    jsr VecStartListChannelKind2
    plx
    inc a:OBJ_27A8,x
    rts
EndingScrollSteps_at3:
    dec a
    bne EndingScrollSteps_at4
    jsr EndingScrollBack8
    cmp #$F8
    bne EndingScrollSteps_at2
    inc a:ScrollYHi
    inc a:OBJ_27A8,x
    rts
EndingScrollSteps_at4:
    dec a
    bne EndingScrollSteps_at2
    lda a:OBJ_X_LO+1
    clc
    adc #$10
    sta a:OBJ_X_LO+1
    bcc SoundNotePitch
    inc a:OBJ_X_HI+1
    jsr EndingScrollBack8
    bne EndingScrollSteps_at2
    inc a:OBJ_27A8,x
    rts

; @routine 010D:65E EndingScrollBack8
; @run 6B:865E
; WHAT IT DOES: Takes 8 off the low byte of the first scroll X.
; HOW: A subtract of 8 from SCROLL->x_lo[0].
; WAITS: no.
; Entry: nothing. Exit: A the new byte, N and Z from it, C and V from the subtract; X and Y kept.
EndingScrollBack8:
    lda a:ScrollXLo
    sec
    sbc #$08
    sta a:ScrollXLo
    rts

; @routine 010D:668 EndingScrollRight6
; @run 6B:8668
; WHAT IT DOES: An object handler of two stages: adds 6 to the first scroll's X low byte; in the first stage a carry counts
; the first scroll's Y high byte up and moves to the next state; in the second the byte is counted up once more and a
; carry moves the X high byte on, clears the low byte and moves to the next state.
; HOW: OBJ->state; SCROLL->x_lo[0], x_hi[0] and y_hi[0].
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollRight6:
    lda a:OBJ_27A8,x
    bne EndingScrollRight6_at0
    lda a:ScrollXLo
    clc
    adc #$06
    sta a:ScrollXLo
    bcc EndingScrollRight6_at1
    inc a:ScrollYHi
    inc a:OBJ_27A8,x
EndingScrollRight6_at1:
    rts
EndingScrollRight6_at0:
    dec a
    bne EndingScrollRight6_at1
    lda a:ScrollXLo
    clc
    adc #$06
    sta a:ScrollXLo
    inc a:ScrollXLo
    bcc EndingScrollRight6_at1
    inc a:ScrollXHi
    stz a:ScrollXLo
    inc a:OBJ_27A8,x
    rts

; @routine 010D:69A EndingScrollDown
; @run 6B:869A
; WHAT IT DOES: An object handler of three stages: the first starts the raster effect $13, sets scroll slots 5 and 6 to Y $1F and
; $BF, slot 1 to $00, and counts slot 0's Y high byte and slot 2's X high byte up; the second, on odd frames, counts slot 1's Y
; up and the Y low bytes of the two objects after this one down until slot 1's Y is $1F, then clears the frame of the object
; two on and moves to the next state; the third plays the ending's list $0B on the next object.
; HOW: OBJ->state; ZP->frame_tick; SCROLL; EndingFrameList.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollDown:
    lda a:OBJ_27A8,x
    bne EndingScrollDown_at0
    lda #$13
    sta z:zRasterEffect
    lda #$1F
    sta a:ScrollYLo+5
    lda #$00
    sta a:ScrollYLo+1
    inc a:ScrollYHi
    lda #$BF
    sta a:ScrollYLo+6
    inc a:ScrollXHi+2
    inc a:OBJ_27A8,x
EndingScrollDown_at2:
    rts
EndingScrollDown_at0:
    dec a
    bne EndingScrollDown_at1
    lda z:zFrameTick
    lsr a
    bcc EndingScrollDown_at2
    inc a:ScrollYLo+1
    dec a:OBJ_Y_LO+1,x
    dec a:OBJ_Y_LO+2,x
    lda a:ScrollYLo+1
    cmp #$1F
    bne EndingScrollDown_at2
    inc a:OBJ_27A8,x
    stz a:OBJ_2A68+2,x
    rts
EndingScrollDown_at1:
    inx
    lda #$0B
    jsr EndingFrameList
    dex
    rts

; @routine 010D:6E3 EndingScrollUp
; @run 6B:86E3
; WHAT IT DOES: An object handler of two stages: the first starts the raster effect $13 and sets scroll slots 1, 5 and 6 to Y
; $0F, $2F and $BF, slot 1's Y high byte up by one and slot 2's X high byte up by one; the second counts slot 1's Y down (with
; the borrow) until it is $2F and moves to the next state.
; HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo and y_hi.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollUp:
    lda a:OBJ_27A8,x
    bne EndingScrollUp_at0
    lda #$13
    sta z:zRasterEffect
    lda #$0F
    sta a:ScrollYLo+1
    inc a:ScrollYHi+1
    lda #$2F
    sta a:ScrollYLo+5
    lda #$BF
    sta a:ScrollYLo+6
    inc a:ScrollXHi+2
    inc a:OBJ_27A8,x
    rts
EndingScrollUp_at0:
    dec a
    bne EndingScrollUp_at1
    dec a:ScrollYLo+1
    lda a:ScrollYLo+1
    cmp #$FF
    bne EndingScrollUp_at2
    dec a:ScrollYHi+1
EndingScrollUp_at2:
    lda a:ScrollYLo+1
    cmp #$2F
    bne EndingScrollUp_at1
    inc a:OBJ_27A8,x
EndingScrollUp_at1:
    rts

; @routine 010D:720 EndingRiseFrame31
; @run 6B:8720
; WHAT IT DOES: An object handler of three stages: shows picture kind 3 and frame $31, slows its upward speed by 2 and holds
; $B0 frames; then waits the hold out; then moves until its Y low byte is $48 and moves to the next state.
; HOW: OBJ->state; EndingHoldTimer; ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingRiseFrame31:
    lda a:OBJ_27A8,x
    bne EndingRiseFrame31_at0
    lda #$03
    sta a:OBJ_2A10,x
    lda #$31
    sta a:OBJ_2A68,x
    dec a:OBJ_VY_HI,x
    dec a:OBJ_VY_HI,x
    lda #$B0
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts
EndingRiseFrame31_at0:
    dec a
    bne EndingRiseFrame31_at1
    jsr EndingHoldTimer
    inc a:OBJ_27A8,x
    rts
EndingRiseFrame31_at1:
    dec a
    bne EndingRiseFrame31_at2
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$48
    bne EndingRiseFrame31_at2
    inc a:OBJ_27A8,x
EndingRiseFrame31_at2:
    rts

; @routine 010D:759 EndingRiseFrame32A
; @run 6B:8759
; WHAT IT DOES: An object handler of three stages: shows picture kind 3, sets its downward speed $60 and slows its upward speed
; by 2, holds $90 frames and adds $10 to the byte at $345F plus its number; then waits the hold out and shows frame $32; then
; moves until its Y low byte is $60 and moves to the next state.
; HOW: OBJ->state; EndingHoldTimer; ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingRiseFrame32A:
    lda a:OBJ_27A8,x
    bne EndingRiseFrame32A_at0
    lda #$03
    sta a:OBJ_2A10,x
    lda #$60
    sta a:OBJ_VY_LO,x
    dec a:OBJ_VY_HI,x
    dec a:OBJ_VY_HI,x
    lda #$90
    sta a:OBJ_34B8,x
    lda a:$345F,x
    clc
    adc #$10
    sta a:$345F,x
    inc a:OBJ_27A8,x
    rts
EndingRiseFrame32A_at0:
    dec a
    bne EndingRiseFrame32A_at1
    jsr EndingHoldTimer
    lda #$32
    sta a:OBJ_2A68,x
    inc a:OBJ_27A8,x
    rts
EndingRiseFrame32A_at1:
    dec a
    bne EndingRiseFrame32A_at2
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$60
    bne $8758
    inc a:OBJ_27A8,x
EndingRiseFrame32A_at2:
    rts

; @routine 010D:7A0 EndingRiseWithList
; @run 6B:87A0
; WHAT IT DOES: An object handler of three stages: shows picture kind 3, slows its upward speed by 1 and holds $80 frames; then
; waits the hold out; then plays the ending's list 7 and moves until its Y low byte is $68 and moves to the next state.
; HOW: OBJ->state; EndingHoldTimer; EndingFrameList; ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingRiseWithList:
    lda a:OBJ_27A8,x
    bne EndingRiseWithList_at0
    lda #$03
    sta a:OBJ_2A10,x
    dec a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts
EndingRiseWithList_at0:
    dec a
    bne EndingRiseWithList_at1
    jsr EndingHoldTimer
    inc a:OBJ_27A8,x
    rts
EndingRiseWithList_at1:
    dec a
    bne EndingRiseWithList_at2
    lda #$07
    jsr EndingFrameList
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$68
    bne $8758
    inc a:OBJ_27A8,x
EndingRiseWithList_at2:
    rts

; @routine 010D:7D6 EndingRiseFrame32B
; @run 6B:87D6
; WHAT IT DOES: As EndingRiseFrame32A, holding $A0 frames and adding $20.
; HOW: OBJ->state; EndingHoldTimer; ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingRiseFrame32B:
    lda a:OBJ_27A8,x
    bne EndingRiseFrame32B_at0
    lda #$03
    sta a:OBJ_2A10,x
    lda #$60
    sta a:OBJ_VY_LO,x
    dec a:OBJ_VY_HI,x
    dec a:OBJ_VY_HI,x
    lda #$A0
    sta a:OBJ_34B8,x
    lda a:$345F,x
    clc
    adc #$20
    sta a:$345F,x
    inc a:OBJ_27A8,x
    rts
EndingRiseFrame32B_at0:
    dec a
    bne EndingRiseFrame32B_at1
    jsr EndingHoldTimer
    lda #$32
    sta a:OBJ_2A68,x
    inc a:OBJ_27A8,x
    rts
EndingRiseFrame32B_at1:
    dec a
    bne EndingRiseFrame32B_at2
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$60
    bne EndingRiseFrame32B_at2
    inc a:OBJ_27A8,x
EndingRiseFrame32B_at2:
    rts

; @routine 010E:01D EndingHoldThenScript19
; @run 6B:881D
; WHAT IT DOES: An object handler of two stages: the first starts the raster effect $13, sets scroll slots 1 and 5 to Y $2F and
; 2 and 6 to $BF and holds the object $40 frames; the second, once the hold has run out, starts script $19 on slot 7 as kind 1
; and moves to the next state.
; HOW: OBJ->state; EndingHoldTimer; StartScriptSlotKind1.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
; not checked: the hold timer's early return drops the caller's frame; the draft goes on regardless (FRAME DROPPED), so only the arm where the timer has run out is compared.
EndingHoldThenScript19:
    lda a:OBJ_27A8,x
    bne EndingHoldThenScript19_at0
    lda #$13
    sta z:zRasterEffect
    lda #$2F
    sta a:ScrollYLo+1
    sta a:ScrollYLo+5
    lda #$BF
    sta a:ScrollYLo+2
    sta a:ScrollYLo+6
    lda #$40
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts
EndingHoldThenScript19_at0:
    dec a
    bne ScriptSound
    jsr EndingHoldTimer
    phx
    lda #$19
    ldx #$07
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_27A8,x
    rts
    rts

; @routine 010E:053 EndingLinkAnimationA
; @run 6B:8853
; WHAT IT DOES: An object handler: shows picture kind 3 and plays the ending's list 4; when a frame changes to a non-zero picture
; it starts script $0D on slot 1 as kind 1.
; HOW: OBJ->state and OBJ->sprite_set; EndingFrameList; StartScriptSlotKind1.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingLinkAnimationA:
    lda a:OBJ_27A8,x
    bne EndingLinkAnimationA_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
EndingLinkAnimationA_at0:
    lda #$04
    jsr EndingFrameList
    bcc EndingLinkAnimationA_at1
    lda a:OBJ_2A68,x
    beq EndingLinkAnimationA_at1
    phx
    ldx #$01
    lda #$0D
    jsr VecStartScriptSlotKind1
    plx
EndingLinkAnimationA_at1:
    rts

; @routine 010E:076 EndingLinkAnimationB
; @run 6B:8876
; WHAT IT DOES: As EndingLinkAnimationA, with the ending's list 5.
; HOW: OBJ->state and OBJ->sprite_set; EndingFrameList; StartScriptSlotKind1.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingLinkAnimationB:
    lda a:OBJ_27A8,x
    bne EndingLinkAnimationB_at0
    lda #$03
    sta a:OBJ_2A10,x
    inc a:OBJ_27A8,x
EndingLinkAnimationB_at0:
    lda #$05
    bra $8862

; @routine 010E:087 EndingScrollSqueeze
; @run 6B:8887
; WHAT IT DOES: An object handler of four stages: the first starts the raster effect $14 and sets the scroll's Y slots (1 and 5 to
; $2F, 2 and 6 to $BF, 0 to $5F, 4 to $A7) and holds the object $40 frames; the second waits the hold out; the third, on odd frames,
; moves slots 5 and 6 towards each other until slot 5 is $78; every stage but the first moves scroll slots 3 and 4 on.
; HOW: OBJ->state; SCROLL->y_lo; EndingScrollSlots.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingScrollSqueeze:
    lda a:OBJ_27A8,x
    bne EndingScrollSqueeze_at0
    lda #$14
    sta z:zRasterEffect
    inc a:ScrollYHi+1
    lda #$2F
    sta a:ScrollYLo+1
    sta a:ScrollYLo+5
    lda #$BF
    sta a:ScrollYLo+2
    sta a:ScrollYLo+6
    lda #$5F
    sta a:ScrollYLo
    lda #$A7
    sta a:ScrollYLo+4
    lda #$40
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
EndingScrollSqueeze_at2:
    rts
EndingScrollSqueeze_at0:
    dec a
    bne EndingScrollSqueeze_at1
    dec a:OBJ_34B8,x
    bne EndingScrollSqueeze_at2
    inc a:OBJ_27A8,x
    rts
EndingScrollSqueeze_at1:
    dec a
    bne InitStage
    jsr EndingScrollSlots
    bbr0 zFrameTick,EndingScrollSqueeze_at2
    inc a:ScrollYLo+5
    dec a:ScrollYLo+6
    lda a:ScrollYLo+5
    cmp #$78
    bne EndingScrollSqueeze_at3
    inc a:OBJ_27A8,x
EndingScrollSqueeze_at3:
    lda a:ScrollYLo+5
    sta a:ScrollYLo+1
    lda a:ScrollYLo+6
    cmp #$A7
    bcc EndingScrollSqueeze_at4
    sta a:ScrollYLo+4
EndingScrollSqueeze_at4:
    rts
    jmp EndingScrollSlots

; @routine 010E:0EF EndingScrollSlots
; @run 6B:88EF
; WHAT IT DOES: Moves the scroll slots 3 and 4 on: 4 added to the low X of slot 3, 8 to the low X of slot 4.
; HOW: Two adds.
; WAITS: no.
; Entry: nothing. Exit: A the second sum, N and Z from it, C and V from its add; X and Y kept.
EndingScrollSlots:
    lda a:ScrollXLo+3
    clc
    adc #$04
    sta a:ScrollXLo+3
    lda a:ScrollXLo+4
    clc
    adc #$08
    sta a:ScrollXLo+4
    rts

; @routine 010E:102 EndingDropAndBounce
; @run 6B:8902
; WHAT IT DOES: An object handler of three stages: the first starts the raster effect $13, sets the scroll's Y slots (1 and 5 to
; $2F, 2 and 6 to $AF), picture kind 3, frame 1 and X speed $C0; the second adds $60 to the downward speed each frame and moves
; the object until its Y is $60 or more, then fixes it at $60 (Y and X); the third adds $10 to the hold byte each frame and bounces
; the object by the sine of it ($07FF scaled) while the byte is not negative, shifting scroll slots 1 and 2 against it;
; when it goes negative it fixes the object at Y $60, the slots at $2F and $AF, and moves to the next state.
; HOW: OBJ->state; ApplyVelocity; ScaleBySine with zTemp00 and zTemp01 set to $07FF; ZP->product.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingDropAndBounce:
    lda a:OBJ_27A8,x
    bne EndingDropAndBounce_at0
    lda #$13
    sta z:zRasterEffect
    stz z:zRasterArg
    lda #$2F
    sta a:ScrollYLo+1
    sta a:ScrollYLo+5
    lda #$AF
    sta a:ScrollYLo+2
    sta a:ScrollYLo+6
    lda #$03
    sta a:OBJ_2A10,x
    lda #$01
    sta a:OBJ_2A68,x
    lda #$C0
    sta a:OBJ_VX_LO,x
EndingDropAndBounce_at3:
    inc a:OBJ_27A8,x
    rts
EndingDropAndBounce_at0:
    dec a
    bne EndingDropAndBounce_at1
    lda #$60
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,x
    cla
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$60
    bcc EndingDropAndBounce_at2
    lda #$60
    sta a:OBJ_Y_LO,x
    sta a:OBJ_X_LO,x
    bra EndingDropAndBounce_at3
EndingDropAndBounce_at1:
    dec a
    bne EndingDropAndBounce_at2
    lda #$FF
    sta z:zTemp00
    lda #$07
    sta z:zTemp01
    lda a:OBJ_34B8,x
    clc
    adc #$10
    sta a:OBJ_34B8,x
    bmi EndingDropAndBounce_at4
    jsr VecScaleBySine
    lda z:zProduct2
    clc
    adc #$60
    sta a:OBJ_Y_LO,x
    lda #$2F
    sec
    sbc z:zProduct2
    sta a:ScrollYLo+1
    lda #$AF
    sec
    sbc z:zProduct2
    sta a:ScrollYLo+2
EndingDropAndBounce_at2:
    rts
EndingDropAndBounce_at4:
    lda #$60
    sta a:OBJ_Y_LO,x
    inc a:OBJ_27A8,x
    lda #$2F
    sta a:ScrollYLo+1
    lda #$AF
    sta a:ScrollYLo+2
    rts

; @routine 010E:19C EndingWaitForList
; @run 6B:899C
; WHAT IT DOES: An object handler: in its first state plays the ending's list its next kind names, and when the list has ended
; and the kind is not negative moves to the next state.
; HOW: OBJ->state and OBJ->next_kind; EndingFrameList.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingWaitForList:
    lda a:OBJ_27A8,x
    bne EndingWaitForList_at0
    lda a:OBJ_3040,x
    jsr EndingFrameList
    bne EndingWaitForList_at0
    lda a:OBJ_3040,x
    bmi EndingWaitForList_at0
    inc a:OBJ_27A8,x
EndingWaitForList_at0:
    rts

; @routine 010E:1B2 EndingFlyOff
; @run 6B:89B2
; WHAT IT DOES: An object handler of two stages: the first takes its speed from four tables at $89E2 by its next kind; the second
; moves the object until its hold has run out.
; HOW: OBJ->state and OBJ->next_kind; ApplyVelocity; OBJ->life_timer.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
EndingFlyOff:
    lda a:OBJ_27A8,x
    bne EndingFlyOff_at0
    ldy a:OBJ_3040,x
    lda a:$89E2,y
    sta a:OBJ_VX_LO,x
    lda a:$89E7,y
    sta a:OBJ_VX_HI,x
    lda a:$89EC,y
    sta a:OBJ_VY_LO,x
    lda a:$89F1,y
    sta a:OBJ_VY_HI,x
EndingFlyOff_at2:
    inc a:OBJ_27A8,x
    rts
EndingFlyOff_at0:
    dec a
    bne EndingFlyOff_at1
    jsr VecApplyVelocity
    dec a:OBJ_34B8,x
    beq EndingFlyOff_at2
EndingFlyOff_at1:
    rts

; @routine 010E:1F6 EndingFrameList
; @run 6B:89F6
; WHAT IT DOES: Steps an object's animation along the list the ending's table names: the list's address is
; read from the table at $8A05 by A, and StepObjectFrames runs on it.
; HOW: A doubled indexes the table; its two bytes go to zTemp10 and zTemp11; StepObjectFrames as a tail jump.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames; X kept.
EndingFrameList:
    asl a
    tay
    lda a:$8A05,y
    sta z:zTemp10
    lda a:$8A06,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0111:003 EndingEffectEntry
; @run 6C:8003
; WHAT IT DOES: The palette effects' entry for a frame: runs EndingEffectStep.
; HOW: A jump to EndingEffectStep.
; WAITS: no.
; Entry: nothing. Exit: as EndingEffectStep.
; not checked: as EndingEffectStep.
EndingEffectEntry:
    jmp EndingEffectStep

; @routine 0111:2AE EndingEffectStep
; @run 6C:82AE
; WHAT IT DOES: One frame of the ending's palette effects: while the game is paused (bit 0 of zPaused) only slots 2 and $0B are run; otherwise,
; unless slot 3 holds type 5 (which runs as the original code with X 3), every slot from $0B down to 0 that has a type is run through
; EndingEffectDispatch.
; HOW: zPaused; the types at $3C00; the slot kept in zRichterObject's byte ($20D9) across each dispatch.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear; A the last type loaded or what the last handler left; C, Y and V as the last handler left them.
; not checked: the paused path is untraced in the draft, and types outside the table jump to other overlays.
EndingEffectStep:
    bbr0 zPaused,EndingEffectStep_at0
    ldx #$02
    bsr EndingEffectStep_at1
    ldx #$0B
EndingEffectStep_at1:
    lda a:ScriptSlotKind,x
    bne EndingEffectDispatch
    rts
EndingEffectStep_at2:
    ldx #$03
    jmp EndingStartList3_at0
EndingEffectStep_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EndingEffectStep_at2
    ldx #$0B
EndingEffectStep_at4:
    lda a:ScriptSlotKind,x
    beq EndingEffectStep_at3
    stx z:zRichterObject
    bsr EndingEffectDispatch
    ldx z:zRichterObject
EndingEffectStep_at3:
    dex
    bpl EndingEffectStep_at4
    rts

; @routine 0111:2DA EndingEffectDispatch
; @run 6C:82DA
; WHAT IT DOES: Sends a palette effect slot to the handler its type names: the table at $82EA holds one address per type, type 1 first.
; HOW: Type less one doubled indexes the table; the word goes through zTemp00 and zTemp01; the jump is made with A the word's high byte,
; Y the index, C bit 7 of type less one, X the slot. A handler this overlay's C does not hold runs as the original code.
; WAITS: no.
; Entry: A the type (not zero), X the slot. Exit: as the handler leaves them.
; not checked: 2 of 400 cases jump to a type outside the table.
EndingEffectDispatch:
    dec a
    asl a
    tay
    lda a:$82EA,y
    sta z:zTemp00
    lda a:$82EB,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0111:2F8 EndingEffectList
; @run 6C:82F8
; WHAT IT DOES: A palette effect that plays a list of palette loads: once its timer has run down it takes the list's palettes one a
; step (each loaded by LoadPackedPaletteC), the timer set from the list's first byte; when the list ends it counts a round and starts
; the list again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at $89D2).
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N, Z and C from
; comparing it with the rounds played, Y 1; X kept.
EndingEffectList:
    lda a:ScriptSlotTimer,x
    beq EndingEffectList_at0
    dec a:ScriptSlotTimer,x
    rts
EndingEffectList_at0:
    lda a:ScriptSlotScript,x
    bpl EndingEffectList_at1
    asl a
    tay
    lda a:$89D2,y
    sta z:zTemp00
    lda a:$89D3,y
    sta z:zTemp01
    bra EndingEffectList_at2
EndingEffectList_at1:
    asl a
    tay
    lda a:$9400
    sta z:zBiosAl
    lda a:$9401
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EndingEffectList_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EndingEffectList_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EndingEffectList_at3
    jsr LoadPackedPaletteC
    inc a:ScriptSlotField18,x
    bra EndingEffectList
EndingEffectList_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EndingEffectList_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EndingEffectList_at4
EndingEffectList_at5:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 0111:371 EndingEffectFade
; @run 6C:8371
; WHAT IT DOES: A palette effect that fades palettes down: once its timer has run down it counts a round, gives the slot back after the
; eighth, and otherwise takes the list's palettes (an address of the VCE and a count each) and darkens every colour of them by one step
; in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402 (or a table at $9442); the VCE read into $3C60 by a TAI,
; changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X $20 when a palette
; was changed (else the slot), Y the list's place.
EndingEffectFade:
    lda a:ScriptSlotTimer,x
    beq EndingEffectFade_at0
    dec a:ScriptSlotTimer,x
    rts
EndingEffectFade_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EndingEffectFade_at1
    jmp EndingEffectList_at5
EndingEffectFade_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EndingEffectFade_at2
    lda a:$9442,y
    sta z:zTemp00
    lda a:$9443,y
    sta z:zTemp01
    bra EndingEffectFade_at3
EndingEffectFade_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EndingEffectFade_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EndingEffectFade_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EndingEffectFade_at4
    rts
EndingEffectFade_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EndingEffectFade_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EndingEffectFade_at5
    dec a
EndingEffectFade_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EndingEffectFade_at6
    sec
    sbc #$08
EndingEffectFade_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EndingEffectFade_at7
    sec
    sbc #$20
EndingEffectFade_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EndingEffectFade_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EndingEffectFade_at9
