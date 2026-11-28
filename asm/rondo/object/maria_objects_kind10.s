; @routine 01E2:5B0 MariaItem4Extra
; @run 6E:CDB0
; WHAT IT DOES: The handler of kind 12, item 4's second object: its state's routine from MariaItem4ExtraStates.
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
MariaItem4Extra:
    jsr MariaStateDoubled
    jmp (MariaItem4ExtraStates,x)

; @routine 01E2:5BC MariaItem4ExtraStart
; @run 6E:CDBC
; WHAT IT DOES: State 0 of kind 12: while she is in item 4's state on the screen, it waits for her step 1, then takes
; her X and starts falling ($0038 a frame) with animation 7. Out of that state, or with her below the screen, it is
; cleared.
; HOW: Her OBJ_STATE, OBJ_Y_HI and OBJ_SUBSTATE; the JMP to MariaObjectFallStart7, or to ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: waiting, A her step, N, Z and C from comparing it with 1; else as the routine
; jumped to.
MariaItem4ExtraStart:
    ldx z:zClimbStepBefore
    lda a:OBJ_STATE
    cmp #$10
    bne MariaItem4ExtraStart_at0
    lda a:OBJ_Y_HI
    dec a
    beq MariaItem4ExtraStart_at0
    lda a:OBJ_SUBSTATE
    cmp #$01
    bne MariaItem4ExtraStart_at1
    lda a:OBJ_X_LO
    sta a:OBJ_X_LO,x
    jmp MariaObjectFallStart7
MariaItem4ExtraStart_at1:
    rts
MariaItem4ExtraStart_at0:
MariaItem4ExtraStart_clear:
    jmp VecClearObject

; @routine 01E2:5DF MariaItem4ExtraFollow
; @run 6E:CDDF
; WHAT IT DOES: State 1 of kind 12: from her (facing and flip), animates, moves, takes her X and falls; below the
; screen it is cleared. While she is in item 4's state and it is near her, it goes above her, keeps her X less its own
; in OBJ_HELD_X_OFFSET, and goes to its next state.
; HOW: MariaObjectFromPlayer, MariaObjectAnimationTick, MariaObjectMove, MariaObjectFallCapped, MariaObjectNearPlayer's
; carry, a BSR to MariaObjectAbovePlayer, the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step or the routine jumped to.
MariaItem4ExtraFollow:
    ldx z:zClimbStepBefore
    jsr MariaObjectFromPlayer
    jsr MariaObjectAnimationTick
    jsr MariaObjectMove
    lda a:OBJ_X_LO
    sta a:OBJ_X_LO,x
    jsr MariaObjectFallCapped
    lda a:OBJ_Y_HI,x
    cmp #$01
    beq MariaItem4ExtraStart_clear
    lda a:OBJ_STATE
    cmp #$10
    bne MariaItem4ExtraFollow_at0
    jsr MariaObjectNearPlayer
    bcc MariaItem4ExtraFollow_at0
    bsr MariaObjectAbovePlayer
    lda a:OBJ_X_LO
    sec
    sbc a:OBJ_X_LO,x
    sta a:OBJ_HELD_X_OFFSET,x
    jmp MariaObjectNextState
MariaItem4ExtraFollow_at0:
    rts

; @routine 01E2:641 MariaItem4ExtraHeld
; @run 6E:CE41
; WHAT IT DOES: State 2 of kind 12, held above her: from her; it animates unless her frame is $42; it stays above
; her at her X less OBJ_HELD_X_OFFSET; when she leaves item 4's state, or is below the screen, it is cleared.
; HOW: MariaObjectFromPlayer, MariaObjectAnimationTick, MariaObjectAbovePlayer; the tail JMPs to ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: held, A her Y's high byte less one, N and Z from it; else as ClearObject.
MariaItem4ExtraHeld:
    ldx z:zClimbStepBefore
    jsr MariaObjectFromPlayer
    lda a:OBJ_FRAME
    cmp #$42
    beq MariaItem4ExtraHeld_at0
    jsr MariaObjectAnimationTick
MariaItem4ExtraHeld_at0:
    jsr MariaObjectAbovePlayer
    lda a:OBJ_X_LO
    sec
    sbc a:OBJ_HELD_X_OFFSET,x
    sta a:OBJ_X_LO,x
    lda a:OBJ_STATE
    cmp #$10
    beq MariaItem4ExtraHeld_at1
    jmp VecClearObject
MariaItem4ExtraHeld_at1:
    lda a:OBJ_Y_HI
    dec a
    bne MariaItem4ExtraHeld_at2
    jmp VecClearObject
MariaItem4ExtraHeld_at2:
    rts

; @routine 01E2:671 MariaSequenceObject
; @run 6E:CE71
; WHAT IT DOES: The handler of kind 10, the object of her pad sequence's move: its state's routine from
; MariaSequenceObjectStates (start, rise, turn, fall).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
MariaSequenceObject:
    jsr MariaStateDoubled
    jmp (MariaSequenceObjectStates,x)

; @routine 01E2:67F MariaSequenceObjectStart
; @run 6E:CE7F
; WHAT IT DOES: State 0 of kind 10: from her (facing and flip) at her place, X speed entry 3 the way it faces,
; OBJ_WORK_N $0D, a hitbox $1E by $0C at 15 across and 3 down, live; animation 0 and its next state; frame $11.
; HOW: MariaObjectFromPlayer, MariaObjectAtPlayerPlace, MariaSetVelocityXFromTable, MariaObjectFacingVelocity,
; MariaObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: A $11, N and Z from it; the rest as MariaObjectStartAnimation left it.
MariaSequenceObjectStart:
    ldx z:zClimbStepBefore
    jsr MariaObjectFromPlayer
    jsr MariaObjectAtPlayerPlace
    lda #$03
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$0D
    sta a:OBJ_WORK_N,x
    lda #$06
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$0F
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$03
    sta a:OBJ_HITBOX_DY,x
    lda #$0F
    sta a:OBJ_HITBOX_DX,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    cla
    jsr MariaObjectStartAnimation
    lda #$11
    sta a:OBJ_FRAME,x
    rts

; @routine 01E2:6B7 MariaSequenceObjectRise
; @run 6E:CEB7
; WHAT IT DOES: State 1 of kind 10: while she is in the sequence's state it moves; at her frame $47 it stops along X
; and goes to its next state; either way its frame follows hers (MariaObjectFrameFromPlayer). Out of that state it is
; cleared.
; HOW: Her OBJ_STATE and OBJ_FRAME; MariaObjectMove, MariaObjectStopX, MariaObjectNextState, on into
; MariaObjectFrameFromPlayer; the branch to MariaSequenceObjectClear.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectFrameFromPlayer, or ClearObject.
MariaSequenceObjectRise:
    ldx z:zClimbStepBefore
    lda a:OBJ_STATE
    cmp #$11
    bne MariaSequenceObjectClear
    jsr MariaObjectMove
    lda a:OBJ_FRAME
    cmp #$47
    bne MariaObjectFrameFromPlayer
    jsr MariaObjectStopX
    jsr MariaObjectNextState

; @routine 01E2:6E7 MariaSequenceObjectClear
; @run 6E:CEE7
; WHAT IT DOES: Clears kind 10's object when she has left the sequence's state.
; HOW: The tail JMP to ClearObject through the kernel's vector.
; WAITS: no.
; Entry: X the object. Exit: all registers and flags kept.
MariaSequenceObjectClear:
    jmp VecClearObject

; @routine 01E2:6EA MariaSequenceObjectTurn
; @run 6E:CEEA
; WHAT IT DOES: State 2 of kind 10: while she is in the sequence's state it moves with its frame following hers; at
; her frame $46 it sets off: timer $20, X speed entry 5 the way it faces, Y speed entry 3, a fall of $0024 a frame, no
; hitbox, and its next state. Out of that state it is cleared.
; HOW: MariaObjectMove, a BSR to MariaObjectFrameFromPlayer; the velocity tables, MariaObjectFacingVelocity; the tail
; JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $46; else as the routine
; jumped to.
MariaSequenceObjectTurn:
    ldx z:zClimbStepBefore
    lda a:OBJ_STATE
    cmp #$11
    bne MariaSequenceObjectClear
    jsr MariaObjectMove
    bsr MariaObjectFrameFromPlayer
    lda a:OBJ_FRAME
    cmp #$46
    beq MariaSequenceObjectTurn_at0
    rts
MariaSequenceObjectTurn_at0:
    lda #$20
    sta a:OBJ_HURT_TIMER,x
    lda #$05
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$03
    jsr MariaSetVelocityYFromTable
    lda #$24
    sta a:OBJ_FALL_LO,x
    lda #$00
    sta a:OBJ_FALL_HI,x
    stz a:OBJ_HITBOX_ON,x
    jmp MariaObjectNextState

; @routine 01E2:722 MariaSequenceObjectFall
; @run 6E:CF22
; WHAT IT DOES: State 3 of kind 10: until its timer runs out it moves and falls, shows frame $12 and keeps its mark
; on; then it is cleared.
; HOW: DEC of OBJ_HURT_TIMER; MariaObjectMove, MariaObjectFallCapped, MariaObjectMarkOn; the tail JMP to ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectMarkOn, or ClearObject.
MariaSequenceObjectFall:
    ldx z:zClimbStepBefore
    dec a:OBJ_HURT_TIMER,x
    beq MariaSequenceObjectFall_at0
    jsr MariaObjectMove
    jsr MariaObjectFallCapped
    lda #$12
    sta a:OBJ_FRAME,x
    jsr MariaObjectMarkOn
    rts
MariaSequenceObjectFall_at0:
    jmp VecClearObject
