; @routine 01E2:14B MariaKind2
; @run 6E:C94B
; WHAT IT DOES: The handler of kind 2: its state's routine from MariaKind2States (start, walk, the edge of the
; screen, in the air twice, the step, walk again).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
MariaKind2:
    jsr MariaStateDoubled
    jmp (MariaKind2States,x)

; @routine 01E2:15F MariaKind2Start
; @run 6E:C95F
; WHAT IT DOES: State 0 of kind 2: from her (facing and flip), 2 across and 2 up from her, a hitbox 12 wide and 5
; high, live; X speed entry 1 the way it faces; a fall of $0024 a frame (OBJ_FALL_LO and HI), OBJ_WORK_N $0C, three
; OBJ_WALL_TURNS; animation 0 and its next state; frame $0B.
; HOW: MariaObjectFromPlayer, MariaObjectAtPlayer, MariaSetVelocityXFromTable, MariaObjectFacingVelocity,
; MariaObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: A $0B, N and Z from it; the rest as MariaObjectStartAnimation left it.
MariaKind2Start:
    ldx z:zRichterObject
    jsr MariaObjectFromPlayer
    lda #$02
    sta z:zTemp04
    lda #$FE
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    lda #$05
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$0C
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$01
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$24
    sta a:OBJ_FALL_LO,x
    lda #$00
    sta a:OBJ_FALL_HI,x
    lda #$0C
    sta a:OBJ_WORK_N,x
    lda #$03
    sta a:OBJ_WALL_TURNS,x
    cla
    jsr MariaObjectStartAnimation
    lda #$0B
    sta a:OBJ_FRAME,x
    rts

; @routine 01E2:1A4 MariaKind2Walk
; @run 6E:C9A4
; WHAT IT DOES: States 1 and 6 of kind 2, walking: hit, it turns (MariaKind2Turn); it moves and falls, animates while
; it has an animation; below the screen it is cleared. On the way down, a mark of bit 5 under it leaps it
; (MariaKind2Leap) and solid ground lands it (MariaKind2Land); then a bit-5 mark at its feet leaps it, bit 3 a step
; ahead climbs it (MariaKind2Step); with nothing of bit 1 ahead it goes to the edge check (MariaKind2Edge); else a
; wall: its turns counted down, a turn (state 3) while some are left, else state 4.
; HOW: MariaObjectHitByChecked's carry; MariaObjectMove, MariaObjectFallCapped, MariaObjectAnimationTick; the probes
; below and at offsets (MariaObjectProbeBelowAny, MariaObjectProbeBelow, MariaObjectProbeAt, MariaObjectProbeAtBit1).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaKind2Walk:
    ldx z:zRichterObject
    jsr MariaObjectHitByChecked
    bcs MariaKind2Turn
    jsr MariaObjectMove
    jsr MariaObjectFallCapped
    lda a:OBJ_WORK_A,x
    beq MariaKind2Walk_at0
    jsr MariaObjectAnimationTick
MariaKind2Walk_at0:
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne MariaKind2Walk_at1
    jmp MariaKind2Clear
MariaKind2Walk_at1:
    cmp #$FF
    beq MariaKind2Walk_at2
    lda a:OBJ_VY_HI,x
    bmi MariaKind2Walk_at2
    lda #$08
    jsr MariaObjectProbeBelowAny
    beq MariaKind2Walk_at2
    and #$20
    beq MariaKind2Walk_at3
    jmp MariaKind2Leap
MariaKind2Walk_at3:
    lda #$00
    jsr MariaObjectProbeBelow
    beq MariaKind2Land
MariaKind2Walk_at2:
    lda #$00
    jsr MariaObjectProbeBelowAny
    and #$20
    beq MariaKind2Walk_at4
    jmp MariaKind2Leap
MariaKind2Walk_at4:
    lda #$00
    ldy #$08
    jsr MariaObjectProbeAt
    and #$08
    beq MariaKind2Walk_at5
    jmp MariaKind2Step
MariaKind2Walk_at5:
    ldy a:OBJ_FACING_LEFT,x
    lda a:MariaKind2WallAhead,y
    cly
    jsr MariaObjectProbeAtBit1
    beq MariaKind2Edge
    dec a:OBJ_WALL_TURNS,x
    bne MariaKind2Turn
MariaKind2Walk_at6:
    lda #$04
    bra MariaKind2Turn_at0

; @routine 01E2:210 MariaKind2Turn
; @run 6E:CA10
; WHAT IT DOES: Turns kind 2 from a wall or a hit: state 3 while it has turns left in OBJ_WALL_TURNS, else state 4; Y
; speed entry 3, X speed entry 2 the way it faces, frame $0C.
; HOW: The velocity tables, MariaObjectFacingVelocity.
; WAITS: no.
; Entry: X the object. Exit: A $0C, N and Z from it; the rest as MariaObjectFacingVelocity left it.
MariaKind2Turn:
    lda a:OBJ_WALL_TURNS,x
    beq MariaKind2Walk_at6
    lda #$03
MariaKind2Turn_at0:
    sta a:OBJ_STATE,x
    lda #$03
    jsr MariaSetVelocityYFromTable
    lda #$02
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$0C
    sta a:OBJ_FRAME,x
    rts

; @routine 01E2:22D MariaKind2Edge
; @run 6E:CA2D
; WHAT IT DOES: Kind 2 with nothing ahead of it: once its Y speed's high byte is $FF (rising), a mark of bit 5 under
; it, 8 rows below or at its feet, leaps it.
; HOW: MariaObjectProbeBelowAny twice; the JMP to MariaKind2Leap.
; WAITS: no.
; Entry: X the object. Exit: as the last test or MariaKind2Leap.
MariaKind2Edge:
    lda a:OBJ_VY_HI,x
    cmp #$FF
    beq MariaKind2Edge_at0
    rts
MariaKind2Edge_at0:
    lda #$08
    jsr MariaObjectProbeBelowAny
    and #$20
    beq MariaKind2Edge_at1
    jmp MariaKind2Leap
MariaKind2Edge_at1:
    lda #$00
    jsr MariaObjectProbeBelowAny
    and #$20
    beq MariaKind2Edge_at2
    jmp MariaKind2Leap
MariaKind2Edge_at2:
    rts

; @routine 01E2:24E MariaKind2Land
; @run 6E:CA4E
; WHAT IT DOES: Lands kind 2 on the ground 8 rows below it: its Y up by the row the probe left in zTemp02, no Y speed
; or fraction, animation 4, state 2.
; HOW: MariaObjectProbeBelowAny, the subtract, MariaObjectStopY, MariaObjectStartAnimation.
; WAITS: no.
; Entry: X the object. Exit: A 2, N and Z from it; the rest as MariaObjectStartAnimation left it.
MariaKind2Land:
    lda #$08
    jsr MariaObjectProbeBelowAny
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO,x
    jsr MariaObjectStopY
    stz a:OBJ_Y_FRAC,x
    lda #$04
    jsr MariaObjectStartAnimation
    lda #$02
    sta a:OBJ_STATE,x
    rts

; @routine 01E2:26F MariaKind2State2
; @run 6E:CA6F
; WHAT IT DOES: State 2 of kind 2: on a screen past the first (OBJ_X_HI not zero) with its X from $10 to $EF there,
; it is cleared; else it walks on (MariaKind2Walk2).
; HOW: OBJ_X_HI and OBJ_X_LO; on into MariaKind2Clear, or a branch to MariaKind2Walk2.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaKind2Clear or MariaKind2Walk2.
MariaKind2State2:
    ldx z:zRichterObject
    lda a:OBJ_X_HI,x
    beq MariaKind2Walk2
    lda a:OBJ_X_LO,x
    cmp #$F0
    bcs MariaKind2Walk2
    cmp #$10
    bcc MariaKind2Walk2

; @routine 01E2:281 MariaKind2Clear
; @run 6E:CA81
; WHAT IT DOES: Clears kind 2's object.
; HOW: The tail JMP to ClearObject through the kernel's vector.
; WAITS: no.
; Entry: X the object. Exit: all registers and flags kept.
MariaKind2Clear:
    jmp VecClearObject

; @routine 01E2:284 MariaKind2Walk2
; @run 6E:CA84
; @stop CAD4
; WHAT IT DOES: Kind 2 walking in state 2: hit, it turns; it animates and moves; 8 rows below it, bit 7 of the answer
; lifts it by the row in zTemp02. Otherwise with nothing solid ahead and below, or something of bit 1 ahead, it drops
; (MariaKind2Drop); under bit 4 straight below, Y speed entry $0E.
; HOW: MariaObjectHitByChecked, MariaObjectAnimationTick, MariaObjectMove; MariaObjectProbe, MariaObjectProbeAtSolid,
; MariaObjectProbeAtBit1; MariaKind2FloorAhead and MariaKind2DropAhead by the facing.
; WAITS: no.
; Entry: X the object. Exit: the processor as the last step left it.
MariaKind2Walk2:
    jsr MariaObjectHitByChecked
    bcs MariaKind2Turn
    jsr MariaObjectAnimationTick
    jsr MariaObjectMove
    lda #$08
    sta z:zTemp06
    stz z:zTemp07
    stz z:zTemp04
    stz z:zTemp05
    jsr MariaObjectProbe
    and #$80
    bne MariaKind2Walk2_at0
    ldy a:OBJ_FACING_LEFT,x
    lda a:MariaKind2FloorAhead,y
    ldy #$08
    jsr MariaObjectProbeAtSolid
    beq MariaKind2Drop
    ldy a:OBJ_FACING_LEFT,x
    lda a:MariaKind2DropAhead,y
    ldy #$02
    jsr MariaObjectProbeAtBit1
    bne MariaKind2Drop
    ldy #$08
    cla
    jsr MariaObjectProbeAtSolid
    and #$10
    beq MariaKind2Walk2_at1
    lda #$0E
    jsr MariaSetVelocityYFromTable
MariaKind2Walk2_at1:
    rts
MariaKind2Walk2_at0:
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO,x
    rts

; @routine 01E2:2D8 MariaKind2Leap
; @run 6E:CAD8
; WHAT IT DOES: Kind 2 leaps: Y speed entry 3, X speed entry $0A the way it faces; animation 4 unless it is in state 6
; already; state 6.
; HOW: The velocity tables, MariaObjectFacingVelocity, MariaObjectStartAnimation.
; WAITS: no.
; Entry: X the object. Exit: A 6, N and Z from it; the rest as the last call left it.
MariaKind2Leap:
    lda #$03
    jsr MariaSetVelocityYFromTable
    lda #$0A
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda a:OBJ_STATE,x
    cmp #$06
    beq MariaKind2Leap_at0
    lda #$04
    jsr MariaObjectStartAnimation
MariaKind2Leap_at0:
    lda #$06
    sta a:OBJ_STATE,x
    rts

; @routine 01E2:2F7 MariaKind2Drop
; @run 6E:CAF7
; WHAT IT DOES: Kind 2 drops: Y speed entry 3, animation 5, state 1.
; HOW: MariaSetVelocityYFromTable, MariaObjectStartAnimation.
; WAITS: no.
; Entry: X the object. Exit: A 1, N and Z from it; the rest as MariaObjectStartAnimation left it.
MariaKind2Drop:
    lda #$03
    jsr MariaSetVelocityYFromTable
    lda #$05
    jsr MariaObjectStartAnimation
    lda #$01
    sta a:OBJ_STATE,x
    rts

; @routine 01E2:307 MariaKind2Air
; @run 6E:CB07
; WHAT IT DOES: States 3 and 4 of kind 2, in the air: it moves and falls; below the screen it is cleared. In state 3
; on the way down, a bit-5 mark under it leaps it and solid ground lands it (then walks, state 2, live); with nothing
; below, bit 3 at its feet climbs it; otherwise the edge check.
; HOW: MariaObjectMove, MariaObjectFallCapped; MariaObjectProbeBelowAny, MariaObjectProbeBelow, MariaObjectProbeAt;
; MariaKind2Land, the velocity table, MariaObjectFacingVelocity.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaKind2Air:
    ldx z:zRichterObject
    jsr MariaObjectMove
    jsr MariaObjectFallCapped
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne MariaKind2Air_at0
    jmp MariaKind2Clear
MariaKind2Air_at0:
    cmp #$FF
    beq MariaKind2Air_at1
    lda a:OBJ_STATE,x
    cmp #$04
    beq MariaKind2Air_at1
    lda a:OBJ_VY_HI,x
    bmi MariaKind2Air_at1
    lda #$08
    jsr MariaObjectProbeBelowAny
    beq MariaKind2Air_at2
    and #$20
    beq MariaKind2Air_at3
    jmp MariaKind2Leap
MariaKind2Air_at3:
    lda #$00
    jsr MariaObjectProbeBelow
    beq MariaKind2Air_at4
MariaKind2Air_at1:
    jmp MariaKind2Edge
MariaKind2Air_at4:
    jsr MariaKind2Land
    lda #$01
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$02
    sta a:OBJ_STATE,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    rts
MariaKind2Air_at2:
    lda #$00
    ldy #$08
    jsr MariaObjectProbeAt
    and #$08
    beq MariaKind2Air_at1

; @routine 01E2:362 MariaKind2Step
; @run 6E:CB62
; WHAT IT DOES: Kind 2 climbs a step: its Y up by the row in zTemp02 and down 6, no Y speed, X speed entry 6 the way
; it faces, state 5, animation $0A.
; HOW: The subtract and add, MariaObjectStopY, the velocity table, MariaObjectFacingVelocity, the tail JMP to
; MariaObjectSetAnimation.
; WAITS: no.
; Entry: X the object. Exit: as MariaObjectSetAnimation.
MariaKind2Step:
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO,x
    lda #$06
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    jsr MariaObjectStopY
    lda #$06
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$05
    sta a:OBJ_STATE,x
    lda #$0A
    jmp MariaObjectSetAnimation

; @routine 01E2:389 MariaKind2State5
; @run 6E:CB89
; WHAT IT DOES: State 5 of kind 2, on the step: on a screen past the first with its X from $10 to $EF it is cleared; hit, it turns; it
; animates; in camera mode $17 its speed follows the scroll (MariaKind2Special); it moves; with something of bit 1
; ahead it goes up 6 rows and drops on with X speed entry 7 and Y speed entry 0.
; HOW: OBJ_X_HI and OBJ_X_LO, the tail JMP to ClearObject; MariaObjectHitByChecked, MariaObjectAnimationTick,
; MariaObjectMove, MariaObjectProbeAtBit1 with MariaKind2StepAhead by the facing; MariaKind2Drop and the velocity tables.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaKind2State5:
    ldx z:zRichterObject
    lda a:OBJ_X_HI,x
    beq MariaKind2State5_at0
    lda a:OBJ_X_LO,x
    cmp #$F0
    bcs MariaKind2State5_at0
    cmp #$10
    bcc MariaKind2State5_at0
    jmp VecClearObject
MariaKind2State5_at0:
    jsr MariaObjectHitByChecked
    bcc MariaKind2State5_at1
    jmp MariaKind2Turn
MariaKind2State5_at1:
    jsr MariaObjectAnimationTick
    lda a:CameraMode
    cmp #$17
    bne MariaKind2State5_at2
    jsr MariaKind2Special
MariaKind2State5_at2:
    jsr MariaObjectMove
    ldy a:OBJ_FACING_LEFT,x
    lda a:MariaKind2StepAhead,y
    ldy #$02
    jsr MariaObjectProbeAtBit1
    beq MariaKind2State5_at3
    lda #$FA
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    jsr MariaKind2Drop
    lda #$07
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$00
    jsr MariaSetVelocityYFromTable
MariaKind2State5_at3:
    rts

; @routine 01E2:3DF MariaKind2Special
; @run 6E:CBDF
; WHAT IT DOES: Kind 2 in camera mode $17: off screen 0 its Y speed is $0080 and it moves against the scroll
; (MariaObjectAgainstScroll); on the last screen ($0E) from X $58 on, Y speed $0480 and nothing more.
; HOW: ScrollXHiScreen (read twice); the stores; MariaObjectAgainstScroll.
; WAITS: no.
; Entry: X the object. Exit: screen 0, A 0, Z set; else as MariaObjectAgainstScroll, or A 4 with N and Z from it.
MariaKind2Special:
    lda a:ScrollXHiScreen
    beq MariaKind2Special_at0
    lda a:ScrollXHiScreen
    cmp #$0E
    beq MariaKind2Special_at1
MariaKind2Special_at2:
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$00
    sta a:OBJ_VY_HI,x
    jsr MariaObjectAgainstScroll
MariaKind2Special_at0:
    rts
MariaKind2Special_at1:
    lda a:OBJ_X_LO,x
    cmp #$58
    bcc MariaKind2Special_at2
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$04
    sta a:OBJ_VY_HI,x
    rts
