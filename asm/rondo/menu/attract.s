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
    sta z:$7E
    lda #$8F
    sta z:$7F
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
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
    stz a:OBJ_2750+8
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
    lda ($7E)
    tax
    lda a:$87F0,x
    clc
    adc z:$7E
    sta z:$7E
    bcc ScriptStep
    inc z:$7F

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
    lda ($7E),y
    cmp z:zWaitTimer3
    bne $87DC
    iny
    lda ($7E),y
    cmp z:zWaitTimer4
    bne $87DC
    lda ($7E)
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
; Entry: from ScriptStep. Exit: as ScriptAdvance.
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
    lda ($7E),y
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
    lda ($7E),y
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
    lda ($7E),y
    jsr VecStartListChannel0Kind2
    bra ScriptAdvance

; @routine 00C2:06D ScriptStartSlotKind1
; @run 6B:886D
; WHAT IT DOES: A script command: starts script slot 0 as kind 1 with the operand's script.
; HOW: StartScriptSlot0Kind1; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptStartSlotKind1:
    lda ($7E),y
    jsr VecStartScriptSlot0Kind1
    bra ScriptAdvance

; @routine 00C2:074 ScriptStartSlotKind2
; @run 6B:8874
; WHAT IT DOES: A script command: starts script slot 0 as kind 2 with the operand's script.
; HOW: StartScriptSlot0Kind2; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptStartSlotKind2:
    lda ($7E),y
    jsr VecStartScriptSlot0Kind2
    bra ScriptAdvance

; @routine 00C2:07B ScriptStartSlotKind3
; @run 6B:887B
; WHAT IT DOES: A script command: starts script slot 0 as kind 3 with the operand's script.
; HOW: StartScriptSlot0Kind3; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptStartSlotKind3:
    lda ($7E),y
    jsr VecStartScriptSlot0Kind3
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
    lda ($7E),y
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
    lda ($7E),y
    tax
    jsr ScriptPlaceObject
    iny
    lda ($7E),y
    sta a:OBJ_2750,x
    jmp ScriptAdvance

; @routine 00C2:0A2 ScriptPlaceObjectKind2
; @run 6B:88A2
; WHAT IT DOES: A script command: places an object, gives it the next two operands (OBJ->f3040
; and OBJ->f2a10) and starts it in state 2.
; HOW: ScriptPlaceObject, two bytes, a 2 in OBJ->f2750; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptPlaceObjectKind2:
    lda ($7E),y
    tax
    jsr ScriptPlaceObject
    iny
    lda ($7E),y
    sta a:OBJ_3040,x
    iny
    lda ($7E),y
    sta a:OBJ_2A10,x
    lda #$02
    sta a:OBJ_2750,x
    jmp ScriptAdvance

; @routine 00C2:0BC ScriptPlaceObjectFrame
; @run 6B:88BC
; WHAT IT DOES: A script command: places an object and gives it a picture frame and a kind of
; picture (OBJ->f2a68 and OBJ->f2a10) from the next two operands.
; HOW: ScriptPlaceObject, two bytes; back to ScriptAdvance.
; WAITS: no.
; Entry: from ScriptStep, Y 3. Exit: as ScriptAdvance.
ScriptPlaceObjectFrame:
    lda ($7E),y
    tax
    jsr ScriptPlaceObject
    iny
    lda ($7E),y
    sta a:OBJ_2A68,x
    iny
    lda ($7E),y
    sta a:OBJ_2A10,x
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
    inc a:OBJ_2750,x
    lda ($7E),y
    sta a:OBJ_3040,x
    iny
    lda ($7E),y
    sta a:OBJ_34B8,x
    iny
    lda ($7E),y
    sta a:OBJ_3510,x
    iny
    lda ($7E),y
    sta a:OBJ_3568,x
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
    inc a:OBJ_2750,x
    lda ($7E),y
    sta a:OBJ_3040,x
    iny
    lda ($7E),y
    sta a:OBJ_34B8,x
    iny
    lda ($7E),y
    sta a:OBJ_3510,x
    iny
    lda ($7E),y
    sta a:OBJ_3568,x
    lda #$D8
    sta a:OBJ_Y_LO,x
    jmp ScriptAdvance
