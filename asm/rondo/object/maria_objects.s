; @routine 01E1:30D MariaObjectStopX
; @run 6E:C30D
; WHAT IT DOES: RichterObjectStopX at her address, the same bytes: stops an object along X.
; HOW: RichterObjectStopX's body.
; WAITS: no.
; Entry: X the object. Exit: all registers and flags kept.
MariaObjectStopX:
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VX_HI,x
    rts

; @routine 01E1:314 MariaObjectStop
; @run 6E:C314
; WHAT IT DOES: Stops an object along X and Y.
; HOW: A BSR to MariaObjectStopX, then on into MariaObjectStopY.
; WAITS: no.
; Entry: X the object. Exit: all registers and flags kept.
MariaObjectStop:
    bsr MariaObjectStopX

; @routine 01E1:316 MariaObjectStopY
; @run 6E:C316
; WHAT IT DOES: RichterObjectStopY at her address, the same bytes: stops an object along Y.
; HOW: RichterObjectStopY's body.
; WAITS: no.
; Entry: X the object. Exit: all registers and flags kept.
MariaObjectStopY:
    stz a:OBJ_VY_LO,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 01E1:375 MariaObjectMarkOff
; @run 6E:C375
; WHAT IT DOES: Clears bit 4 of an object's sprite attribute (the bit MariaObjectMarkOn sets).
; HOW: The AND, then a branch to MariaObjectMarkOn's store.
; WAITS: no.
; Entry: X the object. Exit: A the new attribute, N and Z from it. X, Y, C and V kept.
MariaObjectMarkOff:
    lda a:OBJ_SPRITE_ATTR,x
    and #$EF
    bra MariaObjectMarkOn+5

; @routine 01E1:4BE MariaClearObjectsBut
; @run 6E:C4BE
; WHAT IT DOES: Empties object slots 1 to 15, except an object of kind 4 and one of kind $0C in state 2 (item 4's);
; time back to normal and the pad's forced presses off.
; HOW: OBJ_KIND, and OBJ_STATE for kind $0C; ClearObject.
; WAITS: no.
; Entry: nothing. Exit: A the last byte read (slot 15's kind, or its state), X $10, Z and C set, N clear. Y and V
; kept.
MariaClearObjectsBut:
    ldx #$01
MariaClearObjectsBut_at2:
    lda a:OBJ_KIND,x
    cmp #$04
    beq MariaClearObjectsBut_at0
    cmp #$0C
    bne MariaClearObjectsBut_at1
    lda a:OBJ_STATE,x
    cmp #$02
    beq MariaClearObjectsBut_at0
MariaClearObjectsBut_at1:
    jsr VecClearObject
MariaClearObjectsBut_at0:
    inx
    cpx #$10
    bne MariaClearObjectsBut_at2
    stz a:TimeSlowMode
    stz a:PadForcedPress
    stz a:PadForcedPressB
    rts

; @routine 01E1:6A5 MariaObjectPlaceByState
; @run 6E:C6A5
; WHAT IT DOES: Puts a companion (slot 1 or 2) at its place beside her for the state she is in: the offsets from
; MariaSlotPlaces, the pair picked by MariaSlotPlaceByState and the slot.
; HOW: (slot - 1) * 2 plus the state's entry; the offsets to zTemp04 and zTemp06; the tail JMP to MariaObjectAtPlayer.
; WAITS: no.
; Entry: X the object. Exit: Y the pair's index; as MariaObjectAtPlayer. X kept.
MariaObjectPlaceByState:
    txa
    dec a
    asl a
    ldy a:OBJ_STATE
    clc
    adc a:MariaSlotPlaceByState,y
    tay
    lda a:MariaSlotPlaces,y
    sta z:zTemp04
    lda a:MariaSlotPlaces+1,y
    sta z:zTemp06
    jmp MariaObjectAtPlayer

; @routine 01E1:706 MariaClaimKind3Slot
; @run 6E:C706
; WHAT IT DOES: Makes sure a companion of kind 3 is out: slot 1 when it is empty; nothing when slot 1 holds kind 3;
; else slot 2 when it is empty.
; HOW: OBJ_KIND; ClearObject, kind 3. X is kept on the stack.
; WAITS: no.
; Entry: X kept. Exit: A the last kind read or 3, N and Z from X; C from comparing slot 1's kind with 3 when it was
; out, else kept. Y and V kept.
MariaClaimKind3Slot:
    phx
    ldx #$01
    lda a:OBJ_KIND,x
    bne MariaClaimKind3Slot_at0
    bra MariaClaimKind3Slot_at1
MariaClaimKind3Slot_at0:
    cmp #$03
    beq MariaClaimKind3Slot_at2
    inx
    lda a:OBJ_KIND,x
    bne MariaClaimKind3Slot_at2
MariaClaimKind3Slot_at1:
    jsr VecClearObject
    lda #$03
    sta a:OBJ_KIND,x
MariaClaimKind3Slot_at2:
    plx
    rts

; @routine 01E1:7CF MariaObjectStartAnimation
; @run 6E:C7CF
; WHAT IT DOES: Starts an object's animation, sprite set 1, and moves it to its next state.
; HOW: MariaObjectSetAnimation, the store, the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: A the animation, X the object. Exit: A 1, N and Z from the new state; Y and C as MariaObjectSetAnimation left
; them. X and V kept.
MariaObjectStartAnimation:
    jsr MariaObjectSetAnimation
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    jmp MariaObjectNextState

; @routine 01E2:40B MariaObjectHitByChecked
; @run 6E:CC0B
; WHAT IT DOES: Whether an object takes the hit its hit test found: the object that hit it (OBJ_HIT_BY_SLOT) must be
; slot 16 or more, hitting (OBJ_HITBOX_ON 2) and of health not below zero; then its hitbox goes off and its
; OBJ_WORK_D counts one down (C set). Otherwise its OBJ_WORK_P is cleared (C clear). OBJ_HIT_BY_SLOT is cleared
; either way.
; HOW: The hitter's slot in Y; its columns read by Y.
; WAITS: no.
; Entry: X the object. Exit: Y the hitter's slot. Taken, A the hitter's health, N and Z from the new OBJ_WORK_D, C
; set. Not, C clear, with A and N, Z from the last read or compare (A kept when the slot was below 16). X and V kept.
MariaObjectHitByChecked:
    ldy a:OBJ_WORK_O,x
    cpy #$10
    bcc MariaObjectHitByChecked_at0
    lda a:OBJ_HITBOX_ON,y
    cmp #$02
    bne MariaObjectHitByChecked_at0
    lda a:OBJ_HEALTH,y
    bmi MariaObjectHitByChecked_at0
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_WORK_O,x
    dec a:OBJ_WORK_D,x
    sec
    rts
MariaObjectHitByChecked_at0:
    stz a:OBJ_WORK_O,x
    stz a:OBJ_WORK_P,x
    clc
    rts

; @routine 01E2:616 MariaObjectAbovePlayer
; @run 6E:CE16
; WHAT IT DOES: Puts an object's Y ten rows above hers, and three more unless her frame is $42.
; HOW: 16-bit adds of $FFF6 and $FFFD.
; WAITS: no.
; Entry: X the object. Exit: frame $42, A $42, Z and C set. Else A the object's Y high byte, N and Z from it, C and
; V from the add. X and Y kept.
MariaObjectAbovePlayer:
    lda a:OBJ_Y_LO
    clc
    adc #$F6
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    adc #$FF
    sta a:OBJ_Y_HI,x
    lda a:OBJ_FRAME
    cmp #$42
    bne MariaObjectAbovePlayer_at0
    rts
MariaObjectAbovePlayer_at0:
    lda a:OBJ_Y_LO,x
    clc
    adc #$FD
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    adc #$FF
    sta a:OBJ_Y_HI,x
    rts

; @routine 01E2:6D0 MariaObjectFrameFromPlayer
; @run 6E:CED0
; WHAT IT DOES: Gives an object the frame that goes with hers (her frame less $45, plus $11), shown on odd counts of
; zWaveCounter only (frame 0 on even ones).
; HOW: A subtract and an add; bit 0 of zWaveCounter.
; WAITS: no.
; Entry: X the object. Exit: A bit 0 of zWaveCounter, N and Z from it; C and V from the add. X and Y kept.
MariaObjectFrameFromPlayer:
    lda a:OBJ_FRAME
    sec
    sbc #$45
    clc
    adc #$11
    sta a:OBJ_FRAME,x
    lda z:zWaveCounter
    and #$01
    beq MariaObjectFrameFromPlayer_at0
    rts
MariaObjectFrameFromPlayer_at0:
    stz a:OBJ_FRAME,x
    rts

; @routine 01E2:7B5 MariaObjectVelocityFromWork
; @run 6E:CFB5
; WHAT IT DOES: Points an object along the angle in its OBJ_WORK_D at the speed in OBJ_WORK_E (low) and OBJ_WORK_F
; (high).
; HOW: The speed to zTemp00-01, then SetVelocityFromAngle (through the kernel's vector at $4048).
; WAITS: no.
; Entry: X the object. Exit: as SetVelocityFromAngle.
MariaObjectVelocityFromWork:
    lda a:OBJ_WORK_E,x
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    rts

; @routine 01E3:26B MariaStartScriptSlot3
; @run 6E:D26B
; WHAT IT DOES: Starts a script in script slot 3 as kind 5, unless slot 3 is in use.
; HOW: StartSoleScriptSlot (through the kernel's vector at $4042) with X 3; X kept on the stack.
; WAITS: no.
; Entry: A the script, X kept. Exit: A, Y and C as StartSoleScriptSlot left them, N and Z from X. V kept.
MariaStartScriptSlot3:
    phx
    ldx #$03
    jsr VecStartSoleScriptSlot
    plx
    rts

; @routine 01E3:33F MariaObjectFarOffScreen
; @run 6E:D33F
; WHAT IT DOES: Says whether an object is more than $30 pixels off the screen: 1 when it is, 0 when it is on the
; screen (X high byte 0) or near its edge.
; HOW: OBJ_X_HI 0 is on the screen, $FF the screen to the left (off when X is below $D0), any other past the right
; (off from $30 on).
; WAITS: no.
; Entry: X the object. Exit: A 0 or 1, N and Z from it; C from the compare when one was made. X, Y and V kept.
MariaObjectFarOffScreen:
    lda a:OBJ_X_HI,x
    beq MariaObjectFarOffScreen_at0
    inc a
    beq MariaObjectFarOffScreen_at1
    lda a:OBJ_X_LO,x
    cmp #$30
    bcs MariaObjectFarOffScreen_at2
    bra MariaObjectFarOffScreen_at0
MariaObjectFarOffScreen_at1:
    lda a:OBJ_X_LO,x
    cmp #$D0
    bcc MariaObjectFarOffScreen_at2
MariaObjectFarOffScreen_at0:
    lda #$00
    rts
MariaObjectFarOffScreen_at2:
    lda #$01
    rts

; @routine 01E3:35D MariaObjectFrame24Next
; @run 6E:D35D
; WHAT IT DOES: Gives an object frame $24, clears zFieldD2, and moves the object to its next state.
; HOW: The stores, the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: X the object. Exit: A $24, N and Z from the new state. X, Y, C and V kept.
MariaObjectFrame24Next:
    lda #$24
    sta a:OBJ_FRAME,x
    stz z:zFieldD2
    jmp MariaObjectNextState

; @routine 01E3:38C MariaObjectFrameByPlayerFrame
; @run 6E:D38C
; WHAT IT DOES: Gives an object the frame MariaFrameByPlayerFrame pairs with hers (from frame $53).
; HOW: Her frame less $53 indexes the table.
; WAITS: no.
; Entry: X the object. Exit: A the frame, N and Z from it; Y the index; C and V from the subtract. X kept.
MariaObjectFrameByPlayerFrame:
    lda a:OBJ_FRAME
    sec
    sbc #$53
    tay
    lda a:MariaFrameByPlayerFrame,y
    sta a:OBJ_FRAME,x
    rts

; @routine 01E3:3BE MariaObjectFrame25
; @run 6E:D3BE
; WHAT IT DOES: Gives an object frame $25.
; HOW: One store.
; WAITS: no.
; Entry: X the object. Exit: A $25, N and Z from it. X, Y, C and V kept.
MariaObjectFrame25:
    lda #$25
    sta a:OBJ_FRAME,x
    rts
