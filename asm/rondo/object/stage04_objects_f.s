; @routine 07DE:029 AdvanceWhenRiderHit07DE
; @run 71:C829
; WHAT IT DOES: On the first screen follows its rider ($C83D); once the object OBJ_START_ARG names is hit, goes on.
; HOW: A read of that object's hit type.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
AdvanceWhenRiderHit07DE:
    lda a:OBJ_X_HI,x
    bne AdvanceWhenRiderHit07DE_at0
    jsr FollowCarrier07DE
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_HIT_TAKEN,y
    beq AdvanceWhenRiderHit07DE_at0
    inc a:OBJ_STATE,x
AdvanceWhenRiderHit07DE_at0:
    rts

; @routine 07DE:03D FollowCarrier07DE
; @run 71:C83D
; WHAT IT DOES: With an argument under $80, on the first screen: takes the place of the object it names and moves down 4.
; HOW: CopyObjectPlaceBack and a tail jump to ApplyVelocityY.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FollowCarrier07DE:
    lda a:OBJ_START_ARG,x
    bmi FollowCarrier07DE_at0
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne FollowCarrier07DE_at0
    ldy a:OBJ_START_ARG,x
    jsr VecCopyObjectPlaceBack
    lda #$04
    sta a:OBJ_VY_HI,x
    jmp VecApplyVelocityY
FollowCarrier07DE_at0:
    rts

; @routine 07DE:059 RaiseSixSegments07DE
; @run 71:C859
; WHAT IT DOES: Follows its rider (argument $FF bobs first) and steps the list at $C8BC. At its end: next state with a wait of $F0,
; a hit box and 3 health, and six kind $35 segments two slots on, with their numbers, waits $03 up by 8 and parent.
; HOW: A loop of ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RaiseSixSegments07DEList = $C8BC ; a frame list
RaiseSixSegments07DE:
    jsr FollowCarrier07DE
    lda a:OBJ_START_ARG,x
    cmp #$FF
    bne RaiseSixSegments07DE_at0
    jsr BobSlowlyOnCosine07DD
RaiseSixSegments07DE_at0:
    lda #$BC
    sta z:zTemp10
    lda #$C8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne RaiseSixSegments07DE_at2
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    lda #$F0
    sta a:OBJ_PIECE_NEXT,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$03
    sta a:OBJ_HEALTH,x
    txa
    tay
    iny
    iny
    lda #$06
    sta z:zTemp00
    lda #$03
    sta z:zTemp01
    stz z:zTemp02
RaiseSixSegments07DE_at1:
    jsr VecClearObjectInY
    lda #$35
    jsr VecStartObjectAtPlace
    txa
    sta a:OBJ_WORK_L,y
    lda z:zTemp01
    sta a:OBJ_PIECE_NEXT,y
    clc
    adc #$08
    sta z:zTemp01
    lda z:zTemp02
    sta a:OBJ_KNOCK_FORWARD,y
    inc z:zTemp02
    iny
    dec z:zTemp00
    bne RaiseSixSegments07DE_at1
RaiseSixSegments07DE_at2:
    rts

; @routine 07DE:0C9 HomingOnPlayer07DE
; @run 71:C8C9
; WHAT IT DOES: Steps the list at $C922, turns its angle toward the player 32 dots up by one, and moves at a speed that pulses on
; the cosine of OBJ_FALL_LO ($0140 plus).
; HOW: AngleToPlayerOffset, TurnAngleToward, ScaleByCosine, SetVelocityFromAngle.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HomingOnPlayer07DEList = $C922 ; a frame list
HomingOnPlayer07DE:
    lda #$22
    sta z:zTemp10
    lda #$C9
    sta z:zTemp11
    jsr VecStepObjectFrames
    stz z:zTemp08
    stz z:zTemp09
    lda #$E0
    sta z:zTemp0A
    lda #$FF
    sta z:zTemp0B
    jsr VecAngleToPlayerOffset
    sta z:zTemp02
    lda #$01
    sta z:zTemp00
    lda a:OBJ_PIECE_NEXT,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_PIECE_NEXT,x
    lda #$C0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_FALL_LO,x
    jsr VecScaleByCosine
    lda a:OBJ_FALL_LO,x
    clc
    adc #$02
    sta a:OBJ_FALL_LO,x
    lda z:SpriteFlipMask
    clc
    adc #$40
    sta z:zTemp00
    lda z:zPlaneRowsLeft
    adc #$01
    sta z:zTemp01
    lda a:OBJ_PIECE_NEXT,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 07DE:12B CountThenClear07DE
; @run 71:C92B
; WHAT IT DOES: Counts OBJ_PIECE_PLACE out and clears the object.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountThenClear07DE:
    dec a:OBJ_PIECE_PLACE,x
    bne CountThenClear07DE_at0
    jmp VecClearObject
CountThenClear07DE_at0:
    rts

; @routine 07DE:134 ByState07DEC934
; @run 71:C934
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $C946.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DEC934States = $C946 ; a handler per state
ByState07DEC934:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DEC934States,y
    sta z:zTemp00
    lda a:ByState07DEC934States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DE:14C SegmentWaitForBody07DE
; @run 71:C94C
; WHAT IT DOES: Follows its body; once the body is in state 3, counts OBJ_PIECE_NEXT out, takes its frame and a wait of $40.
; HOW: The body's slot in OBJ_WORK_L.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SegmentWaitForBody07DEFrames = $C970 ; frames by segment
SegmentWaitForBody07DE:
    ldy a:OBJ_WORK_L,x
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_STATE,y
    cmp #$03
    bcc SegmentWaitForBody07DE_at0
    dec a:OBJ_PIECE_NEXT,x
    bne SegmentWaitForBody07DE_at0
    ldy a:OBJ_KNOCK_FORWARD,x
    lda a:SegmentWaitForBody07DEFrames,y
    sta a:OBJ_FRAME,x
    lda #$40
    sta a:OBJ_PIECE_PLACE,x
    inc a:OBJ_STATE,x
SegmentWaitForBody07DE_at0:
    rts

; @routine 07DE:176 SegmentSwing07DE
; @run 71:C976
; WHAT IT DOES: Takes the body's attribute and its place along the chain ($C9FE); its attack power out, becomes kind $9B with
; sound $10. Else swings: the radius grows to six steps, and the angle swings by a pendulum of two accelerations.
; HOW: ScaleByCosine; a 16-bit angle and speed.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SegmentSwing07DEPulls = $C9F6 ; acceleration words by half
SegmentSwing07DE:
    ldy a:OBJ_WORK_L,x
    lda a:OBJ_SPRITE_ATTR,y
    sta a:OBJ_SPRITE_ATTR,x
    jsr PlaceAlongChain07DE
    lda a:OBJ_ATTACK_POWER,x
    beq SegmentSwing07DE_at0
    dec a:OBJ_ATTACK_POWER,x
    bne SegmentSwing07DE_at0
    lda #$10
    jsr VecSoundIfOnFirstScreen
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
SegmentSwing07DE_at0:
    lda a:OBJ_WORK_J,x
    cmp #$06
    bcs SegmentSwing07DE_at1
    lda a:OBJ_WORK_K,x
    clc
    adc #$08
    sta a:OBJ_WORK_K,x
    bcc SegmentSwing07DE_at1
    inc a:OBJ_WORK_J,x
SegmentSwing07DE_at1:
    lda #$04
    sta z:zTemp01
    lda #$00
    sta z:zTemp00
    lda a:OBJ_SWOOP_SPEED_HI,x
    jsr VecScaleByCosine
    lda a:OBJ_SWOOP_SPEED_LO,x
    clc
    adc z:SpriteFlipMask
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:OBJ_PIECE_PLACE,x
    adc z:zPlaneRowsLeft
    sta a:OBJ_PIECE_PLACE,x
    lda a:OBJ_RAIN_COUNT,x
    and #$03
    asl a
    tay
    lda a:OBJ_FALL_LO,x
    clc
    adc a:SegmentSwing07DEPulls,y
    sta a:OBJ_FALL_LO,x
    lda a:OBJ_SWOOP_SPEED_HI,x
    adc a:SegmentSwing07DEPulls+1,y
    sta a:OBJ_SWOOP_SPEED_HI,x
    bcc SegmentSwing07DE_at2
    lda #$40
    sta a:OBJ_PIECE_PLACE,x
    stz a:OBJ_SWOOP_SPEED_LO,x
    inc a:OBJ_RAIN_COUNT,x
SegmentSwing07DE_at2:
    rts

; @routine 07DE:1FE PlaceAlongChain07DE
; @run 71:C9FE
; WHAT IT DOES: Places itself at radius OBJ_WORK_J-K and angle OBJ_PIECE_PLACE from the segment before: the first from two slots
; before and 4 down, others from the one before.
; HOW: CopyObjectPlaceBack, SetVelocityFromAngle, ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PlaceAlongChain07DE:
    txa
    tay
    dey
    lda a:OBJ_KNOCK_FORWARD,x
    beq PlaceAlongChain07DE_at0
    jsr VecCopyObjectPlaceBack
    bra PlaceAlongChain07DE_at1
PlaceAlongChain07DE_at0:
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_Y_LO,x
    clc
    adc #$04
    sta a:OBJ_Y_LO,x
    bcc PlaceAlongChain07DE_at1
    inc a:OBJ_Y_HI,x
PlaceAlongChain07DE_at1:
    lda a:OBJ_WORK_K,x
    sta z:zTemp00
    lda a:OBJ_WORK_J,x
    sta z:zTemp01
    lda a:OBJ_PIECE_PLACE,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 07DE:231 ByState07DECA31
; @run 71:CA31
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $CA43.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DECA31States = $CA43 ; a handler per state
ByState07DECA31:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DECA31States,y
    sta z:zTemp00
    lda a:ByState07DECA31States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DE:247 FaceFrameTowardPlayer07DE
; @run 71:CA47
; WHAT IT DOES: Takes the attribute and place of the slot below and the frame for the angle to the player.
; HOW: AngleToPlayer in eighths from frame $85.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FaceFrameTowardPlayer07DE:
    lda a:OBJ_SPRITE_ATTR-1,x
    sta a:OBJ_SPRITE_ATTR,x
    txa
    tay
    dey
    sxy
    jsr VecCopyObjectPlace
    sxy
    jsr VecAngleToPlayer
    clc
    adc #$10
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc #$85
    sta a:OBJ_FRAME,x
    rts

; @routine 07DE:267 EmitFourThenWait07DE
; @run 71:CA67
; WHAT IT DOES: State 0: loads the record, sets raster effect $19 with argument $C8 and emits four times ($CA8C). Later counts
; OBJ_PIECE_NEXT out between emissions.
; HOW: A loop with Y on the stack.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EmitFourThenWait07DE:
    lda a:OBJ_STATE,x
    bne EmitFourThenWait07DE_at1
    jsr VecLoadObjectRecord
    lda #$19
    sta z:zRasterEffect
    lda #$C8
    sta z:zRasterArg
    cly
EmitFourThenWait07DE_at0:
    phy
    jsr EmitOneOfEight07DE
    ply
    iny
    cpy #$04
    bcc EmitFourThenWait07DE_at0
    rts
EmitFourThenWait07DE_at1:
    lda a:OBJ_PIECE_NEXT,x
    beq EmitOneOfEight07DE
    dec a:OBJ_PIECE_NEXT,x
    rts

; @routine 07DE:28C EmitOneOfEight07DE
; @run 71:CA8C
; WHAT IT DOES: Counts OBJ_PIECE_PLACE up and, a slot of $20-$27 free, starts the kind the table gives there with the wait
; OBJ_START_ARG.
; HOW: FindEmptyOf20to27_07DE.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EmitOneOfEight07DEKinds = $CAA9 ; eight kinds
EmitOneOfEight07DE:
    inc a:OBJ_PIECE_PLACE,x
    jsr FindEmptyOf20to27_07DE
    bcc EmitOneOfEight07DE_at0
    phy
    lda a:OBJ_PIECE_PLACE,x
    and #$07
    tay
    lda a:EmitOneOfEight07DEKinds,y
    ply
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    sta a:OBJ_PIECE_NEXT,x
EmitOneOfEight07DE_at0:
    rts

; @routine 07DE:2B1 FindEmptyOf20to27_07DE
; @run 71:CAB1
; WHAT IT DOES: Clears and gives in Y the first empty slot of $20-$27, carry set; carry clear when none is.
; HOW: A walk of the kinds, then ClearObjectInY.
; WAITS: no.
; Entry: none. Exit: C found, Y the slot.
FindEmptyOf20to27_07DE:
    ldy #$20
FindEmptyOf20to27_07DE_at0:
    lda a:OBJ_KIND,y
    beq FindEmptyOf20to27_07DE_at1
    iny
    cpy #$28
    bcc FindEmptyOf20to27_07DE_at0
    clc
    rts
FindEmptyOf20to27_07DE_at1:
    jsr VecClearObjectInY
    sec
    rts

; @routine 07DE:2C4 AscendHitThenByState07DE
; @run 71:CAC4
; WHAT IT DOES: Counts the rise down ($CAF0), takes a hit ($CB13) and goes to the state's handler.
; HOW: A table of handlers at $CADC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
AscendHitThenByState07DEStates = $CADC ; a handler per state
AscendHitThenByState07DE:
    jsr CountRiseTime07DE
    jsr DieWhenHit07DE
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:AscendHitThenByState07DEStates,y
    sta z:zTemp00
    lda a:AscendHitThenByState07DEStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DE:2F0 CountRiseTime07DE
; @run 71:CAF0
; WHAT IT DOES: In state 5 counts the 16-bit OBJ_SWOOP_SPEED out and goes to state 8.
; HOW: A 16-bit decrement.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountRiseTime07DE:
    lda a:OBJ_STATE,x
    cmp #$05
    bne CountRiseTime07DE_at0
    lda a:OBJ_SWOOP_SPEED_HI,x
    sec
    sbc #$01
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    sbc #$00
    sta a:OBJ_SWOOP_SPEED_LO,x
    ora a:OBJ_SWOOP_SPEED_HI,x
    bne CountRiseTime07DE_at0
    lda #$08
    sta a:OBJ_STATE,x
CountRiseTime07DE_at0:
    rts

; @routine 07DE:313 DieWhenHit07DE
; @run 71:CB13
; WHAT IT DOES: A hit under state 9 turns it into sprite set 3 rising slowly, without a hit box, in state 9.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DieWhenHit07DE:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq DieWhenHit07DE_at0
    lda a:OBJ_STATE,x
    cmp #$09
    bcs DieWhenHit07DE_at0
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    stz a:OBJ_FRAME,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$FE
    sta a:OBJ_VY_LO,x
    lda #$09
    sta a:OBJ_STATE,x
DieWhenHit07DE_at0:
    rts

; @routine 07DE:342 SetUpGhost07DE
; @run 71:CB42
; WHAT IT DOES: Loads the record with attribute $8F, a random place ($CB7A), random bob phase and life, facing the player, rising
; at -0.75.
; HOW: NextRandom and a table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetUpGhost07DELives = $CB72 ; lives
SetUpGhost07DE:
    jsr VecLoadObjectRecord
    lda #$8F
    sta a:OBJ_SPRITE_ATTR,x
    jsr RandomPlaceByScroll07DE
    jsr VecNextRandom
    sta a:OBJ_WORK_J,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:SetUpGhost07DELives,y
    sta a:OBJ_SWOOP_SPEED_HI,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    rts

; @routine 07DE:37A RandomPlaceByScroll07DE
; @run 71:CB7A
; WHAT IT DOES: Puts the object at a random X and Y from the tables, shifted to the screen edge the camera moves toward.
; HOW: Two NextRandom and ShiftByScrollDirection07DE.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RandomPlaceByScroll07DEX = $CB9C ; eight X
RandomPlaceByScroll07DEY = $CBA4 ; four Y
RandomPlaceByScroll07DE:
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    jsr VecNextRandom
    and #$07
    tay
    lda a:RandomPlaceByScroll07DEX,y
    sta a:OBJ_X_LO,x
    jsr ShiftByScrollDirection07DE
    jsr VecNextRandom
    and #$03
    tay
    lda a:RandomPlaceByScroll07DEY,y
    sta a:OBJ_Y_LO,x
    rts

; @routine 07DE:3A8 ShiftByScrollDirection07DE
; @run 71:CBA8
; WHAT IT DOES: When the camera scrolls sideways, moves X by the table's step for its direction.
; HOW: The sign of ScrollSpeedX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ShiftByScrollDirection07DESteps = $CBC7 ; X words by direction
ShiftByScrollDirection07DE:
    lda a:ScrollSpeedX
    beq ShiftByScrollDirection07DE_at0
    rol a
    rol a
    and #$01
    asl a
    tay
    lda a:OBJ_X_LO,x
    clc
    adc a:ShiftByScrollDirection07DESteps,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    adc a:ShiftByScrollDirection07DESteps+1,y
    sta a:OBJ_X_HI,x
ShiftByScrollDirection07DE_at0:
    rts

; @routine 07DE:3CB SwayAndRise07DE
; @run 71:CBCB
; WHAT IT DOES: Sways sideways on the cosine of OBJ_WORK_J and rises; above Y $E0 on the first screen goes on.
; HOW: ScaleByCosine, ApplyVelocityX and ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SwayAndRise07DE:
    lda #$A0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_J,x
    jsr VecScaleByCosine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VX_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    inc a:OBJ_WORK_J,x
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne SwayAndRise07DE_at0
    lda a:OBJ_Y_LO,x
    cmp #$E0
    bcs SwayAndRise07DE_at0
    inc a:OBJ_STATE,x
SwayAndRise07DE_at0:
    rts

; @routine 07DE:3FC GhostDrift07DE
; @run 71:CBFC
; WHAT IT DOES: Steps the list at $CC89, wandering on a sine and cosine and counting its life. While the life lasts, once $38 or
; more from the player, leaps with a random speed, sound $8F, and goes on.
; HOW: ScaleBySine and ScaleByCosine.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
GhostDrift07DELeaps = $CC79 ; X speed words
GhostDrift07DEList = $CC89 ; a frame list
GhostDrift07DE:
    lda #$89
    sta z:zTemp10
    lda #$CC
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$80
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_RAIN_COUNT,x
    jsr VecScaleBySine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda #$A0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_J,x
    jsr VecScaleByCosine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VX_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    inc a:OBJ_RAIN_COUNT,x
    inc a:OBJ_WORK_J,x
    dec a:OBJ_SWOOP_SPEED_HI,x
    beq GhostDrift07DE_at0
    jsr VecDistanceXToPlayer
    cmp #$38
    bcc GhostDrift07DE_at0
    lda #$F6
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$40
    sta a:OBJ_FALL_LO,x
    jsr VecNextRandom
    and #$0E
    tay
    lda a:GhostDrift07DELeaps+1,y
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:GhostDrift07DELeaps,y
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda #$8F
    jsr VecSoundIfOnFirstScreen
    inc a:OBJ_STATE,x
GhostDrift07DE_at0:
    rts
