; @routine 00C1:265 ModeDoneAdd
; @run 6B:8265
; WHAT IT DOES: Adds one to zModeDone, the count the mode loop reads to learn that the mode has
; finished.
; HOW: One increment.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the new value, C and the registers kept. No script reaches
; it; it is compared with the draft only.
ModeDoneAdd:
    inc z:zModeDone
    rts

; @routine 00C1:71A AttractBegin
; @run 6B:871A
; WHAT IT DOES: Starts the attract scene: the game state is wiped, the scene's script is set to
; run from $8F98, its frame counter to $FFFF (so that the first tick brings it to 0) and the
; mode goes on to its next state.
; HOW: ResetGameState; the script pointer (the two bytes at $7E) and the frame counter are
; written; zGameSubMode goes up by one.
; WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off;
; the stack there is this routine's return address and the call's.
; Entry: nothing. Exit: A $FF, X and Y as ResetGameState leaves them, N and Z from the new state,
; C as ResetGameState leaves it. The comparison with the draft shows its first difference
; inside ResetGameState (its C does not yet place the frame of its own call to the vblank wait);
; this routine's own accesses agree up to that call.
AttractBegin:
    jsr VecResetGameState
    lda #$98
    sta z:zScriptPtr
    lda #$8F
    sta z:zScriptPtrHi
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    rts

; @routine 00C1:72E AttractRestart
; @run 6B:872E
; WHAT IT DOES: The attract mode's sub-mode 3: the game state is wiped, the scene's script goes back to $9192 (where
; AttractWaitStart sets it when start is pressed), its frame counter to $FFFF, and the mode steps back to sub-mode 2
; (AttractStartCheck). No script or routine found sets sub-mode 3.
; HOW: ResetGameState; the script pointer and the frame counter are written; zGameSubMode goes down by one.
; WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off; the stack there is this
; routine's return address and the call's.
; Entry: X the doubled sub-mode and Y kept (ResetGameState sees them). Exit: A $FF, X 0, Y $FF (ResetGameState's), N and
; Z from the new sub-mode, C kept.
AttractRestart:
    jsr VecResetGameState
    lda #$92
    sta z:zScriptPtr
    lda #$91
    sta z:zScriptPtrHi
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    dec z:zGameSubMode
    rts

; @routine 00C1:7CC AttractFrameTick
; @run 6B:87CC
; WHAT IT DOES: Counts one frame of the attract scene on its 16-bit frame counter.
; HOW: Adds one to the low byte, and to the high byte when the low byte wrapped to zero.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the byte last incremented (the low byte, or the high byte
; when the low one wrapped), C and the registers kept.
AttractFrameTick:
    inc z:zWaitTimer3
    bne AttractFrameTick_at0
    inc z:zWaitTimer4
AttractFrameTick_at0:
    rts

; @routine 00C1:7D3 AttractEndCountdown
; @run 6B:87D3
; WHAT IT DOES: The attract scene's last state: counts the frame counter down, and when it
; runs out wipes the game state and takes the scene's done mark back off.
; HOW: Subtracts one from the low byte of the frame counter and returns while it is not zero.
; At zero it calls ResetGameState and subtracts one from zModeDone.
; WAITS: yes, through ResetGameState, which waits for the vertical blank with the display off;
; the stack there is this routine's return address and the call's. The comparison with the draft
; shows its first difference inside ResetGameState (its C does not yet place the frame of its
; own call to the vblank wait), not in this routine's own accesses: its own are the frame, the
; call and a decrement of zModeDone, and agree up to that point.
; Entry: nothing. Exit: N and Z from the byte last decremented, C and the registers as
; ResetGameState leaves them (kept when the count has not run out).
AttractEndCountdown:
    dec z:zWaitTimer3
    bne AttractEndCountdown_at0
    jsr VecResetGameState
    dec z:zModeDone
AttractEndCountdown_at0:
    rts

; @routine 00C1:7DD AttractHoldBeforeEnd
; @run 6B:87DD
; WHAT IT DOES: The attract script's closing command: the scene goes to state 4, its counter to $40,
; two object state bytes are cleared, and script slot 0 starts script 7.
; HOW: Stores, then StartScriptSlot0Kind2 with 7 as a tail jump.
; WAITS: no.
; Entry: from ScriptStep. Exit: as StartScriptSlot0Kind2: A and Y 7, X 0, N and Z clear.
AttractHoldBeforeEnd:
    lda #$04
    sta z:zGameSubMode
    lda #$40
    sta z:zWaitTimer3
    stz a:OBJ_KIND+8
    stz a:$2759
    lda #$07
    jmp VecStartScriptSlot0Kind2

; @routine 00C2:000 ScriptAdvance
; @run 6B:8800
; WHAT IT DOES: Steps the scene script past the command it is at, and runs on into ScriptStep.
; HOW: The command byte indexes a table of lengths, which is added to the pointer (the high byte
; goes up on a carry); then it runs into ScriptStep.
; WAITS: no.
; Entry: nothing. Exit: as ScriptStep, X the command that was stepped past (when ScriptStep left
; X alone), V from the pointer's add (when it set none).
ScriptAdvance:
    lda (zScriptPtr)
    tax
    lda a:$87F0,x
    clc
    adc z:zScriptPtr
    sta z:zScriptPtr
    bcc ScriptStep
    inc z:zScriptPtrHi

; @routine 00C2:00F ScriptStep
; @run 6B:880F
; WHAT IT DOES: Runs the scene script's next command if its time has come: the two time bytes
; after the command byte equal the scene's frame counter.
; HOW: Compares the first time byte with zWaitTimer3 and the second with zWaitTimer4, and
; returns on a difference; otherwise the command, doubled, indexes a table of handlers and it
; is jumped to with X the doubled command and Y 3.
; WAITS: no.
; Entry: nothing. Exit, a time byte differs: A that byte, N, Z and C from comparing it, Y 1 or 2,
; X kept. Otherwise as the handler leaves them.
ScriptStep:
    ldy #$01
    lda (zScriptPtr),y
    cmp z:zWaitTimer3
    bne $87DC
    iny
    lda (zScriptPtr),y
    cmp z:zWaitTimer4
    bne $87DC
    lda (zScriptPtr)
    asl a
    tax
    ldy #$03
    jmp ($8827,x)

; @routine 00C2:047 ScriptSaveReadAndState
; @run 6B:8847
; WHAT IT DOES: A script command: marks that the title has read the save ($3F83 is set to 1) and
; puts the mode in its state 2.
; HOW: Two stores, then back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptSaveReadAndState:
    lda #$01
    sta a:TitleSaveRead
    lda #$02
    sta z:zGameSubMode
    bra ScriptAdvance

; @routine 00C2:052 ScriptSound
; @run 6B:8852
; WHAT IT DOES: A script command: gives the sound driver the command's operand.
; HOW: The byte at Y 3 goes to SoundCommand; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptSound:
    lda (zScriptPtr),y
    jsr VecSoundCommand
    bra ScriptAdvance

; @routine 00C2:059 ScriptStartListAndClear
; @run 6B:8859
; WHAT IT DOES: A script command: starts list channel 0 with the operand's list, clears the
; screen's state and all the object slots.
; HOW: StartListChannel0Kind2, ClearScreenState, ClearAllObjects; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptStartListAndClear:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    jsr VecClearScreenState
    jsr ClearAllObjects
    bra ScriptAdvance

; @routine 00C2:066 ScriptStartList
; @run 6B:8866
; WHAT IT DOES: A script command: starts list channel 0 with the operand's list.
; HOW: StartListChannel0Kind2; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptStartList:
    lda (zScriptPtr),y
    jsr VecStartListChannel0Kind2
    bra ScriptAdvance

; @routine 00C2:06D ScriptStartSlotKind1
; @run 6B:886D
; WHAT IT DOES: A script command: starts script slot 0 as kind 1 with the operand's script.
; HOW: StartScriptSlot0Kind1; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptStartSlotKind1:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind1
    bra ScriptAdvance

; @routine 00C2:074 ScriptStartSlotKind2
; @run 6B:8874
; WHAT IT DOES: A script command: starts script slot 0 as kind 2 with the operand's script.
; HOW: StartScriptSlot0Kind2; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptStartSlotKind2:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind2
    bra ScriptAdvance

; @routine 00C2:07B ScriptStartSlotKind3
; @run 6B:887B
; WHAT IT DOES: A script command: starts script slot 0 as kind 3 with the operand's script.
; HOW: StartScriptSlot0Kind3; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptStartSlotKind3:
    lda (zScriptPtr),y
    jsr VecStartScriptSlot0Kind3
    jmp ScriptAdvance

; @routine 00C2:083 ScriptPlayAdpcm
; @run 6B:8883
; WHAT IT DOES: A script command: plays the operand's sound straight from the disc as ADPCM.
; HOW: AdpcmPlayFromDisc; back to ScriptAdvance.
; WAITS: yes, on the drive and the ADPCM, through AdpcmPlayFromDisc; the stack there is the caller's return and this
; call's two bytes.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptPlayAdpcm:
    lda (zScriptPtr),y
    jsr VecAdpcmPlayFromDisc
    jmp ScriptAdvance

; @routine 00C2:08B ScriptPlayTrack
; @run 6B:888B
; WHAT IT DOES: A script command: plays the operand's CD track once.
; HOW: CdPlayOnce, with X the doubled command and Y 3 as ScriptStep enters it, which it keeps on
; the stack; back to ScriptAdvance.
; WAITS: yes, on the drive, through CdPlayOnce; the stack there is the caller's return, the
; call's two bytes and X and Y pushed (2).
; Entry: from ScriptStep, X the doubled command, Y 3. Exit: as ScriptAdvance.
ScriptPlayTrack:
    lda (zScriptPtr),y
    jsr VecCdPlayOnce
    jmp ScriptAdvance

; @routine 00C2:093 ScriptPlaceObjectState
; @run 6B:8893
; WHAT IT DOES: A script command: places an object (the operand's slot) and gives it the state
; the next operand names.
; HOW: ScriptPlaceObject, then one more byte to the object's first state byte (OBJ->f2750); back
; to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptPlaceObjectState:
    lda (zScriptPtr),y
    tax
    jsr ScriptPlaceObject
    iny
    lda (zScriptPtr),y
    sta a:OBJ_KIND,x
    jmp ScriptAdvance

; @routine 00C2:0A2 ScriptPlaceObjectKind2
; @run 6B:88A2
; WHAT IT DOES: A script command: places an object, gives it the next two operands (OBJ->f3040
; and OBJ->f2a10) and starts it in state 2.
; HOW: ScriptPlaceObject, two bytes, a 2 in OBJ->f2750; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptPlaceObjectKind2:
    lda (zScriptPtr),y
    tax
    jsr ScriptPlaceObject
    iny
    lda (zScriptPtr),y
    sta a:OBJ_START_ARG,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    lda #$02
    sta a:OBJ_KIND,x
    jmp ScriptAdvance

; @routine 00C2:0BC ScriptPlaceObjectFrame
; @run 6B:88BC
; WHAT IT DOES: A script command: places an object and gives it a picture frame and a kind of
; picture (OBJ->f2a68 and OBJ->f2a10) from the next two operands.
; HOW: ScriptPlaceObject, two bytes; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptPlaceObjectFrame:
    lda (zScriptPtr),y
    tax
    jsr ScriptPlaceObject
    iny
    lda (zScriptPtr),y
    sta a:OBJ_FRAME,x
    iny
    lda (zScriptPtr),y
    sta a:OBJ_SPRITE_SET,x
    jmp ScriptAdvance

; @routine 00C2:0D1 ScriptPlaceObjectTimed
; @run 6B:88D1
; WHAT IT DOES: A script command: places an object, gives it a frame, a kind of picture, a start argument and a work
; byte (OBJ->frame, sprite_set, start_arg and work_d) from the next four operands, and its kind 3.
; HOW: ScriptPlaceObject, four bytes, a 3 in OBJ->kind; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptPlaceObjectTimed:
    lda (zScriptPtr),y
    tax
    jsr ScriptPlaceObject
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
    jmp ScriptAdvance

; @routine 00C2:0F7 ClearAllObjects
; @run 6B:88F7
; WHAT IT DOES: Empties all 88 object slots, the scene script's way of clearing the scene.
; HOW: A loop over the slots from 0 up, each through ClearObject.
; WAITS: no.
; Entry: nothing. Exit: X $58, C and Z set, N clear (the loop's last compare); A and Y kept.
ClearAllObjects:
    clx
ClearAllObjects_at0:
    jsr VecClearObject
    inx
    cpx #$58
    bne ClearAllObjects_at0
    rts

; @routine 00C2:101 ScriptPlaceObject
; @run 6B:8901
; WHAT IT DOES: Empties an object and gives it the place four script bytes name.
; HOW: ClearObject; then the bytes at Y plus 1 to 4 go to the object's X low, X high, Y low and Y
; high.
; WAITS: no.
; Entry: X the object, Y the script byte before the four. Exit: A the last byte read, Y plus 4,
; N and Z from A; X and C kept.
ScriptPlaceObject:
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

; @routine 00C2:11D ScriptPlaceTitleObjectA
; @run 6B:891D
; WHAT IT DOES: A script command: starts object $2E (a title object) with the next four operands
; and its Y at $C8.
; HOW: ClearObject, one added to its state, the four bytes to OBJ->f3040, f34b8, f3510 and
; f3568, $C8 to its Y low; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptPlaceTitleObjectA:
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
    jmp ScriptAdvance

; @routine 00C2:144 ScriptPlaceTitleObjectB
; @run 6B:8944
; WHAT IT DOES: As ScriptPlaceTitleObjectA for object $2F, its Y at $D8.
; HOW: The same body, shared through script_place_title_object.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptPlaceTitleObjectB:
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
    jmp ScriptAdvance

; @routine 00C2:1B0 PlayFrameListOnce
; @run 6B:89B0
; WHAT IT DOES: An object handler: once the object has no state yet, plays the animation the
; object's next kind names until it ends, then moves the object to its next state.
; HOW: OBJ->state nonzero returns; ObjectFrameList with OBJ->start_arg; at its end (A 0), state +1.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
PlayFrameListOnce:
    lda a:OBJ_STATE,x
    bne PlayFrameListOnce_at0
    lda a:OBJ_START_ARG,x
    jsr ObjectFrameList
    bne PlayFrameListOnce_at0
    inc a:OBJ_STATE,x
PlayFrameListOnce_at0:
    rts

; @routine 00C2:1C1 SlideForCount
; @run 6B:89C1
; WHAT IT DOES: An object handler: on its first frame the object is given a speed of one dot a frame, up for a start
; argument of 0 and left for 1; then it moves for as many frames as OBJ->work_d holds, and goes to its next state.
; HOW: OBJ->state 0: OBJ->start_arg indexes the speeds at $89E8 (OBJ->vx_hi) and $89EA (OBJ->vy_hi), state +1. State 1:
; ApplyVelocity, OBJ->work_d counted down, state +1 at zero. Any other state returns.
; WAITS: no.
; Entry: X the object. Exit, state 0: A the Y speed, Y the start argument, N and Z from the new state; X and C kept.
; State 1: A, C and V from ApplyVelocity, N and Z from the count or the new state; X and Y kept. Other states: A the
; state less one, N and Z from it; X, Y and C kept.
SlideSpeedsX = $89E8 ; the X speed's high byte by start argument (0 up, 1 left)
SlideSpeedsY = $89EA ; the Y speed's high byte by start argument
SlideForCount:
    lda a:OBJ_STATE,x
    bne SlideForCount_at0
    ldy a:OBJ_START_ARG,x
    lda a:SlideSpeedsX,y
    sta a:OBJ_VX_HI,x
    lda a:SlideSpeedsY,y
    sta a:OBJ_VY_HI,x
    inc a:OBJ_STATE,x
    rts
SlideForCount_at0:
    dec a
    bne SlideForCount_at1
    jsr VecApplyVelocity
    dec a:OBJ_WORK_D,x
    bne SlideForCount_at1
    inc a:OBJ_STATE,x
SlideForCount_at1:
    rts

; @routine 00C2:1EC AnimateAndCueSeven
; @run 6B:89EC
; WHAT IT DOES: An object handler: on its first frame gives the object picture kind 3 and state 1;
; then steps frame list 0, and when the picture changes to a nonzero frame starts script 7 on
; slot 1.
; HOW: OBJ->state, OBJ->sprite_set; ObjectFrameList(0); OBJ->frame; StartScriptSlotKind1 with X and
; A kept on the stack.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
AnimateAndCueSeven:
    lda a:OBJ_STATE,x
    bne AnimateAndCueSeven_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    inc a:OBJ_STATE,x
AnimateAndCueSeven_at0:
    cla
    jsr ObjectFrameList
    bcc AnimateAndCueSeven_at1
    lda a:OBJ_FRAME,x
    beq AnimateAndCueSeven_at1
    phx
    ldx #$01
    lda #$07
    jsr VecStartScriptSlotKind1
    plx
AnimateAndCueSeven_at1:
    rts

; @routine 00C2:20E AnimateList1AndCueSeven
; @run 6B:8A0E
; WHAT IT DOES: As AnimateAndCueSeven with frame list 1.
; HOW: The same first-frame set-up, then frame_list_then_cue with A 1.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
AnimateList1AndCueSeven:
    lda a:OBJ_STATE,x
    bne AnimateList1AndCueSeven_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    inc a:OBJ_STATE,x
AnimateList1AndCueSeven_at0:
    lda #$01
    bra $89FA

; @routine 00C2:21F AnimateAndCueNine
; @run 6B:8A1F
; WHAT IT DOES: An object handler: on its first frame gives the object picture kind 3 and state 1;
; then, while the scroll is still ($3E10, $3E28 and $3E20 all zero), steps frame list 8 and, when
; the picture changes to a nonzero frame and script slot 0 is idle, starts script 9 on slot 1.
; HOW: OBJ->state, OBJ->sprite_set; the three scroll bytes ORed; ObjectFrameList(8);
; OBJ->frame; SLOTS->kind[0]; StartScriptSlotKind1.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
AnimateAndCueNine:
    lda a:OBJ_STATE,x
    bne AnimateAndCueNine_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    inc a:OBJ_STATE,x
AnimateAndCueNine_at0:
    lda a:ScrollXHi
    ora a:ScrollYHi
    ora a:ScrollYLo
    bne AnimateAndCueNine_at1
    lda #$08
    jsr ObjectFrameList
    bcc AnimateAndCueNine_at1
    lda a:OBJ_FRAME,x
    beq AnimateAndCueNine_at1
    lda a:ScriptSlotKind
    bne AnimateAndCueNine_at1
    phx
    ldx #$01
    lda #$09
    jsr VecStartScriptSlotKind1
    plx
AnimateAndCueNine_at1:
    rts

; @routine 00C2:252 AnimateList9AndCueNine
; @run 6B:8A52
; WHAT IT DOES: As AnimateAndCueNine with frame list 9.
; HOW: The same body, shared through scroll_idle_cue.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
AnimateList9AndCueNine:
    lda a:OBJ_STATE,x
    bne AnimateList9AndCueNine_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    inc a:OBJ_STATE,x
AnimateList9AndCueNine_at0:
    lda a:ScrollXHi
    ora a:ScrollYHi
    ora a:ScrollYLo
    bne AnimateAndCueNine_at1
    lda #$09
    bra $8A39

; @routine 00C2:26E BlinkScrollFlag
; @run 6B:8A6E
; WHAT IT DOES: An object handler: counts frames in OBJ->f34b8 and sets the scroll's X high byte
; to 1 once the count's low seven bits reach $78, else 0.
; HOW: OBJ->state set to 1 on the first frame; OBJ->f34b8 counted; the compare's carry rotated in.
; WAITS: no.
; Entry: X the object. Exit: A 0 or 1, N and Z from it, C clear.
BlinkScrollFlag:
    lda a:OBJ_STATE,x
    bne BlinkScrollFlag_at0
    inc a:OBJ_STATE,x
BlinkScrollFlag_at0:
    inc a:OBJ_WORK_D,x
    lda a:OBJ_WORK_D,x
    and #$7F
    cmp #$78
    cla
    rol a
    sta a:ScrollXHi
    rts

; @routine 00C2:286 MirrorVramBlock
; @run 6B:8A86
; WHAT IT DOES: An object handler: on its second frame mirrors a block of VRAM left to right: each
; byte of $1C blocks of $70 words (the first block $70, the rest 256) from $1170 is written reversed
; to $2CE0 on, and the object is cleared.
; HOW: OBJ->state counts the first frame; then the read and write addresses are set, and each
; word's low and high bytes are read, their bits rotated into zTemp00 one by one and written
; back; ClearObject ends it.
; WAITS: no.
; Entry: X the object. Exit: A the last high byte reversed, Z set, N clear, C the last bit rotated
; out, Y 0; X kept.
MirrorVramBlock:
    lda a:OBJ_STATE,x
    bne MirrorVramBlock_at0
    inc a:OBJ_STATE,x
    rts
MirrorVramBlock_at0:
    jsr VecSelectVramReadAddress
    st1 #$70
    st2 #$11
    jsr VecSelectVramWriteAddress
    st1 #$E0
    st2 #$2C
    jsr VecSelectVramData
    lda #$1C
    sta z:zTemp01
    ldy #$70
MirrorVramBlock_at1:
    lda a:VDC_DATA_LO
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    lda z:zTemp00
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    asl a
    ror z:zTemp00
    lda z:zTemp00
    sta a:VDC_DATA_HI
    dey
    bne MirrorVramBlock_at1
    dec z:zTemp01
    bne MirrorVramBlock_at1
    jmp VecClearObject

; @routine 00C2:2F0 NudgeScrollIn
; @run 6B:8AF0
; WHAT IT DOES: An object handler with three states: the first frame only advances; state 1 adds one
; to the scroll's X low byte and, when it wraps, one to the Y high byte and goes on; state 2 adds one
; a frame until it reaches $80 and goes on.
; HOW: OBJ->state decremented by the original's DEC A to tell the states apart; the scroll bytes by
; INC.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
NudgeScrollIn:
    lda a:OBJ_STATE,x
    bne NudgeScrollIn_at0
    inc a:OBJ_STATE,x
    rts
NudgeScrollIn_at0:
    dec a
    bne NudgeScrollIn_at1
    inc a:ScrollXLo
    bne NudgeScrollIn_at2
    inc a:ScrollYHi
    inc a:OBJ_STATE,x
NudgeScrollIn_at2:
    rts
NudgeScrollIn_at1:
    dec a
    bne NudgeScrollIn_at3
    inc a:ScrollXLo
    lda a:ScrollXLo
    cmp #$80
    bne NudgeScrollIn_at2
    inc a:OBJ_STATE,x
NudgeScrollIn_at3:
    rts

; @routine 00C2:319 DropAndShakeScreen
; @run 6B:8B19
; WHAT IT DOES: An object handler with three states: the first frame gives the object a downward
; speed and the scroll's Y low byte $FF; state 1 speeds the object up and moves the scroll and
; the object down until the scroll runs out, then cues script 10 and waits $10 frames; state 2
; shakes the scroll sideways by a random amount each frame until the wait ends, then clears it.
; HOW: OBJ->state; OBJ->vy and OBJ->y_frac; SCROLL->y_lo, x_lo and x_hi; StartScriptSlotKind1;
; NextRandom; OBJ->f34b8 the wait.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
DropAndShakeScreen:
    lda a:OBJ_STATE,x
    bne DropAndShakeScreen_at0
    inc a:OBJ_STATE,x
    lda #$02
    sta a:OBJ_VY_HI,x
    lda #$FF
    sta a:ScrollYLo
    rts
DropAndShakeScreen_at0:
    dec a
    bne DropAndShakeScreen_at1
    lda a:OBJ_VY_LO,x
    clc
    adc #$40
    sta a:OBJ_VY_LO,x
    bcc DropAndShakeScreen_at2
    inc a:OBJ_VY_HI,x
DropAndShakeScreen_at2:
    lda a:OBJ_Y_FRAC,x
    sec
    sbc a:OBJ_VY_LO,x
    sta a:OBJ_Y_FRAC,x
    lda a:ScrollYLo
    sbc a:OBJ_VY_HI,x
    sta a:ScrollYLo
    bcs DropAndShakeScreen_at3
    stz a:ScrollYLo
    phx
    ldx #$01
    lda #$0A
    jsr VecStartScriptSlotKind1
    plx
    lda #$10
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
DropAndShakeScreen_at3:
    rts
DropAndShakeScreen_at1:
    dec a
    bne DropAndShakeScreen_at3
    jsr VecNextRandom
    and #$07
    asl a
    sta a:ScrollXLo
    stz a:ScrollXHi
    dec a:OBJ_WORK_D,x
    bne DropAndShakeScreen_at3
    stz a:ScrollXLo
    stz a:ScrollXHi
    inc a:OBJ_STATE,x
    rts

; @routine 00C2:385 BreakApartOnFrame
; @run 6B:8B85
; WHAT IT DOES: An object handler: on its first frame sets picture kind 3; then steps frame list 2
; and when the picture reaches frame $0D starts script 8 on slot 1 and scatters debris.
; HOW: OBJ->state; ObjectFrameList(2); OBJ->frame; StartScriptSlotKind1; ScatterDebris.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
BreakApartOnFrame:
    lda a:OBJ_STATE,x
    bne BreakApartOnFrame_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
BreakApartOnFrame_at2:
    inc a:OBJ_STATE,x
    rts
BreakApartOnFrame_at0:
    dec a
    bne BreakApartOnFrame_at1
    lda #$02
    jsr ObjectFrameList
    beq BreakApartOnFrame_at2
    bcc BreakApartOnFrame_at1
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne BreakApartOnFrame_at1
    phx
    ldx #$01
    lda #$08
    jsr VecStartScriptSlotKind1
    plx
    jmp ScatterDebris
BreakApartOnFrame_at1:
    rts

; @routine 00C2:3B3 ScatterDebris
; @run 6B:8BB3
; WHAT IT DOES: Fills the objects from 4 to $1E with pieces flying out, each from a table of four
; bytes: its X place and speed, its Y place and speed, a random Y fraction, a frame.
; HOW: Per object, from $1E down to 4: ClearObject, the table's bytes into OBJ->x_lo, vx, y_lo, vy_hi,
; the frame; NextRandom into OBJ->vy_lo; the object's picture kind 3 and kind 8.
; WAITS: no.
; Entry: X kept on the stack across it (a typed caller passes its own). Exit: A 8, Z clear, N
; clear, C as the last compare left it (set), Y 4 less than the table's last index; X kept.
ScatterDebris:
    phx
    ldx #$1E
    cly
ScatterDebris_at1:
    jsr VecClearObject
    lda a:$8BFE,y
    sta a:OBJ_X_LO,x
    sec
    sbc #$80
    sta a:OBJ_VX_LO,x
    bpl ScatterDebris_at0
    dec a:OBJ_VX_HI,x
ScatterDebris_at0:
    asl a:OBJ_VX_LO,x
    rol a:OBJ_VX_HI,x
    iny
    lda a:$8BFE,y
    sta a:OBJ_Y_LO,x
    iny
    lda a:$8BFE,y
    sta a:OBJ_VY_HI,x
    jsr VecNextRandom
    sta a:OBJ_VY_LO,x
    iny
    lda a:$8BFE,y
    sta a:OBJ_FRAME,x
    iny
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$08
    sta a:OBJ_KIND,x
    dex
    cpx #$03
    bne ScatterDebris_at1
    plx
    rts

; @routine 00C2:46A FallAndClear
; @run 6B:8C6A
; WHAT IT DOES: An object handler: after its first frame the object moves by its speed, its Y speed
; grows by $20 a frame, and once it is below the screen's middle, past $80, the object is cleared.
; HOW: OBJ->state; ApplyVelocity; OBJ->vy; OBJ->y_hi and OBJ->y_lo; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
FallAndClear:
    lda a:OBJ_STATE,x
    bne FallAndClear_at0
    inc a:OBJ_STATE,x
    rts
FallAndClear_at0:
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    clc
    adc #$20
    sta a:OBJ_VY_LO,x
    bcc FallAndClear_at1
    inc a:OBJ_VY_HI,x
FallAndClear_at1:
    lda a:OBJ_Y_HI,x
    bmi SoundCommandTranspose
    beq SoundCommandTranspose
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcc SoundCommandTranspose
    jmp VecClearObject

; @routine 00C2:495 IntroSceneObject
; @run 6B:8C95
; WHAT IT DOES: An object handler of five states: it starts script 11 on slot 2 and script 6 on slot 1
; and swaps two script slots' scripts, then rises, steps frame list 3, and eases the scroll in two
; stages by $E steps.
; HOW: OBJ->state; StartScriptSlotKind1 and StartScriptSlotKind4 with X on the stack;
; ApplyVelocity; ObjectFrameList(3); SCROLL->y_lo counted down by two.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
IntroSceneObject:
    lda a:OBJ_STATE,x
    bne IntroSceneObject_at0
    inc a:OBJ_STATE,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$2F
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    inc a:ScrollXHi
    inc a:ScrollYHi
    phx
    ldx #$02
    lda #$0B
    jsr VecStartScriptSlotKind1
    ldx #$01
    lda #$06
    jsr VecStartScriptSlotKind4
    lda a:ScriptSlotScript+2
    ldx a:ScriptSlotScript
    stx a:ScriptSlotScript+2
    sta a:ScriptSlotScript
    plx
    rts
IntroSceneObject_at0:
    dec a
    bne IntroSceneObject_at1
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    bmi IntroSceneObject_at2
    inc a:OBJ_STATE,x
    rts
IntroSceneObject_at1:
    dec a
    bne SoundCommandRepeatMark
    lda #$03
    jsr ObjectFrameList
    bne IntroSceneObject_at2
    inc a:OBJ_STATE,x
    rts
    dec a
    bne IntroSceneObject_at3
    jsr VecApplyVelocity
    stz a:ScrollYHi
    dec a:ScrollYLo
    dec a:ScrollYLo
    bne IntroSceneObject_at2
    dec a:ScrollXHi
    inc a:ScrollYHi
    inc a:OBJ_STATE,x
IntroSceneObject_at2:
    rts
IntroSceneObject_at3:
    dec a
    bne IntroSceneObject_at2
    jsr VecApplyVelocity
    stz a:ScrollYHi
    dec a:ScrollYLo
    dec a:ScrollYLo
    bne IntroSceneObject_at2
    inc a:OBJ_STATE,x
    rts

; @routine 00C2:51C RiseAndFallObject
; @run 6B:8D1C
; WHAT IT DOES: An object handler of five stages: it shows picture kind 3 and moves the scroll in,
; waits $10 frames, steps frame list 4 and then 5 (stopping to set a cue), and from then on steps
; list 6 and moves the object until it is above the screen, when it is cleared.
; HOW: OBJ->state; OBJ->life_timer; ObjectFrameList; ApplyVelocity; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
RiseAndFallObject:
    lda a:OBJ_STATE,x
    bne RiseAndFallObject_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    inc a:ScrollXHi
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$10
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
    rts
RiseAndFallObject_at0:
    dec a
    bne RiseAndFallObject_at1
    dec a:OBJ_WORK_D,x
    bne RiseAndFallObject_at2
    lda #$04
    sta a:OBJ_WORK_D,x
    stz a:ScrollXHi
RiseAndFallObject_at4:
    inc a:OBJ_STATE,x
RiseAndFallObject_at2:
    rts
RiseAndFallObject_at1:
    dec a
    bne RiseAndFallObject_at3
    lda #$04
    jsr ObjectFrameList
    beq RiseAndFallObject_at4
    rts
RiseAndFallObject_at3:
    dec a
    bne RiseAndFallObject_at5
    lda #$05
    jsr ObjectFrameList
    bne RiseAndFallObject_at2
    dec a:OBJ_WORK_D,x
    bne RiseAndFallObject_at2
    lda #$0B
    sta a:$276E
    inc a:OBJ_STATE,x
    rts
RiseAndFallObject_at5:
    dec a
    bne RiseAndFallObject_at5
    lda #$06
    jsr ObjectFrameList
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bpl RiseAndFallObject_at2
    jmp VecClearObject

; @routine 00C2:581 DriftAndClearObject
; @run 6B:8D81
; WHAT IT DOES: An object handler: on its first frame it sets a slow rightward speed; after that it
; steps frame list 6 (7 once it is past $50 down), moves, slows its upward speed by 4 a frame,
; and clears itself once above the screen.
; HOW: OBJ->state; OBJ->y_lo; ObjectFrameList; ApplyVelocity; OBJ->vy; ClearObject as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
DriftAndClearObject:
    lda a:OBJ_STATE,x
    bne DriftAndClearObject_at0
    inc a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
    inc a:OBJ_STATE,x
DriftAndClearObject_at4:
    rts
DriftAndClearObject_at0:
    lda a:OBJ_Y_LO,x
    cmp #$50
    bcs DriftAndClearObject_at1
    lda #$06
    bra DriftAndClearObject_at2
DriftAndClearObject_at1:
    lda #$07
DriftAndClearObject_at2:
    jsr ObjectFrameList
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    sec
    sbc #$04
    sta a:OBJ_VY_LO,x
    bcs DriftAndClearObject_at3
    dec a:OBJ_VY_HI,x
DriftAndClearObject_at3:
    lda a:OBJ_Y_HI,x
    bpl DriftAndClearObject_at4
    jmp VecClearObject

; @routine 00C2:5BB SplitScreenObject
; @run 6B:8DBB
; WHAT IT DOES: An object handler of four states: it starts a raster effect and the two halves of
; the scene (objects $10 and $11) drifting apart, waits for them, then moves both, counts the effect
; and the scroll down, clears both and moves the scroll, and last counts the scroll out.
; HOW: OBJ->state; ZP->raster_effect; SCROLL->y_lo and x_hi; ApplyVelocity and ClearObject on $10
; and $11 with X on the stack.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
SplitScreenObject:
    lda a:OBJ_STATE,x
    bne SplitScreenObject_at0
    inc a:OBJ_STATE,x
    dec a:$2D90
    inc a:$2D91
    lda #$40
    sta a:OBJ_WORK_D,x
    lda #$12
    sta z:zRasterEffect
    stz z:zRasterArg
    lda #$38
    sta a:ScrollYLo+2
    inc a:ScrollXHi
    lda #$68
    sta a:ScrollYLo
    rts
SplitScreenObject_at0:
    dec a
    bne SplitScreenObject_at1
    dec a:OBJ_WORK_D,x
    bne SplitScreenObject_at2
    inc a:OBJ_STATE,x
    rts
SplitScreenObject_at1:
    dec a
    bne SplitScreenObject_at3
    phx
    ldx #$10
    jsr VecApplyVelocity
    inx
    jsr VecApplyVelocity
    plx
    lda z:zRasterEffect
    beq SplitScreenObject_at4
    dec a:ScrollYLo+2
    bne SplitScreenObject_at4
    stz z:zRasterEffect
SplitScreenObject_at4:
    dec a:ScrollYLo
    bne SplitScreenObject_at2
    phx
    ldx #$10
    jsr VecClearObject
    inx
    jsr VecClearObject
    plx
    stz a:ScrollXHi
    inc a:ScrollYHi
    inc a:OBJ_STATE,x
SplitScreenObject_at2:
    rts
SplitScreenObject_at3:
    dec a
    bne SplitScreenObject_at5
    stz a:ScrollYHi
    dec a:ScrollYLo
    bne SplitScreenObject_at2
    inc a:OBJ_STATE,x
    rts
SplitScreenObject_at5:
    dec a
    bne SplitScreenObject_at5
    rts

; @routine 00C2:634 FlingObjectRight
; @run 6B:8E34
; WHAT IT DOES: An object handler: it shows picture kind 3 with a rightward speed, then moves while
; stepping frame lists $0A and $0B in turn, flicks its speed to a hard upward one, and ends on
; list $0D.
; HOW: OBJ->state; ApplyVelocity; ObjectFrameList; OBJ->life_timer counted down to 0 between lists.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
FlingObjectRight:
    lda a:OBJ_STATE,x
    bne FlingObjectRight_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$70
    sta a:OBJ_VX_LO,x
FlingObjectRight_at5:
    lda #$08
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
FlingObjectRight_at2:
    rts
FlingObjectRight_at0:
    dec a
    bne FlingObjectRight_at1
    jsr VecApplyVelocity
    lda #$0A
    jsr ObjectFrameList
    bne FlingObjectRight_at2
    dec a:OBJ_WORK_D,x
    beq FlingObjectLeft_at0
    rts
FlingObjectRight_at1:
    dec a
    bne FlingObjectRight_at3
    jsr VecApplyVelocity
    lda #$0B
    jsr ObjectFrameList
    bne FlingObjectRight_at2
    dec a:OBJ_WORK_D,x
    beq FlingObjectRight_at4
    rts
FlingObjectRight_at3:
    lda #$0D
    jmp ObjectFrameList
FlingObjectRight_at4:
    lda #$01
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VY_LO,x
    sta a:OBJ_VY_HI,x
    bra FlingObjectRight_at5

; @routine 00C2:686 FlingObjectLeft
; @run 6B:8E86
; WHAT IT DOES: The mirror of FlingObjectRight: leftward, with frame lists $0A, $0C and $0D.
; HOW: OBJ->state; ApplyVelocity; ObjectFrameList; OBJ->life_timer counted down to 0 between lists.
; WAITS: no.
; Entry: X the object. Exit: as the last test left them.
FlingObjectLeft:
    lda a:OBJ_STATE,x
    bne FlingObjectLeft_at1
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$90
    sta a:OBJ_VX_LO,x
    dec a:OBJ_VX_HI,x
FlingObjectLeft_at2:
    lda #$08
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
FlingObjectLeft_at4:
    rts
FlingObjectLeft_at0:
    lda #$B0
    sta a:OBJ_VY_LO,x
    dec a:OBJ_VY_HI,x
    bra FlingObjectLeft_at2
FlingObjectLeft_at1:
    dec a
    bne FlingObjectLeft_at3
    jsr VecApplyVelocity
    lda #$0A
    jsr ObjectFrameList
    bne FlingObjectLeft_at4
    dec a:OBJ_WORK_D,x
    beq FlingObjectLeft_at0
    rts
FlingObjectLeft_at3:
    dec a
    bne $8E72
    jsr VecApplyVelocity
    lda #$0C
    jsr ObjectFrameList
    bne FlingObjectLeft_at4
    dec a:OBJ_WORK_D,x
    beq FlingObjectLeft_at5
    rts
FlingObjectLeft_at5:
    lda #$FF
    sta a:OBJ_VX_LO,x
    sta a:OBJ_VY_LO,x
    sta a:OBJ_VY_HI,x
    bra FlingObjectLeft_at2

; @routine 00C2:6DE ObjectFrameList
; @run 6B:8EDE
; WHAT IT DOES: Steps an object's animation along the list a scene table names: the list's
; address is read from a table by A, and StepObjectFrames runs on it.
; HOW: A doubled indexes the table; its two bytes go to zTemp10 and zTemp11; StepObjectFrames as a
; tail jump.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames; X kept.
ObjectFrameList:
    asl a
    tay
    lda a:$8EED,y
    sta z:zTemp10
    lda a:$8EEE,y
    sta z:zTemp11
    jmp VecStepObjectFrames
