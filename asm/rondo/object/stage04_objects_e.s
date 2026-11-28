; @routine 07DD:552 SparkOnFirstScreen07DD
; @run 71:C552
; WHAT IT DOES: Clears the object off the first screen or without an argument, and when the list at $C573 ends.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SparkOnFirstScreen07DAList = $C573 ; a frame list
SparkOnFirstScreen07DD:
    lda a:OBJ_X_HI,x
    beq SparkOnFirstScreen07DD_at0
    jmp VecClearObject
SparkOnFirstScreen07DD_at0:
    lda a:OBJ_START_ARG,x
    bne SparkOnFirstScreen07DD_at1
    jmp VecClearObject
SparkOnFirstScreen07DD_at1:
    lda #$73
    sta z:zTemp10
    lda #$C5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne SparkOnFirstScreen07DD_at2
    jmp VecClearObject
SparkOnFirstScreen07DD_at2:
    rts

; @routine 07DD:586 EmitAfterWait07DD
; @run 71:C586
; WHAT IT DOES: State 0 loads the record with a wait of $40. When the 16-bit wait is out, starts a kind $31 object (number 0 in a
; free slot, others in an empty one of $43-$4D) and picks the next wait.
; HOW: A 16-bit countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EmitAfterWait07DAWaits = $C5E7 ; wait words by number
EmitAfterWait07DD:
    lda a:OBJ_STATE,x
    bne EmitAfterWait07DD_at0
    jsr VecLoadObjectRecord
    lda #$40
    sta a:OBJ_PIECE_PLACE,x
    rts
EmitAfterWait07DD_at0:
    lda a:OBJ_PIECE_PLACE,x
    ora a:OBJ_PIECE_NEXT,x
    beq EmitAfterWait07DD_at1
    lda a:OBJ_PIECE_PLACE,x
    sec
    sbc #$01
    sta a:OBJ_PIECE_PLACE,x
    lda a:OBJ_PIECE_NEXT,x
    sbc #$00
    sta a:OBJ_PIECE_NEXT,x
    rts
EmitAfterWait07DD_at1:
    lda a:OBJ_START_ARG,x
    beq EmitAfterWait07DD_at3
    ldy #$43
EmitAfterWait07DD_at2:
    lda a:OBJ_KIND,y
    beq EmitAfterWait07DD_at4
    iny
    cpy #$4E
    bcc EmitAfterWait07DD_at2
    rts
EmitAfterWait07DD_at3:
    jsr VecFindFreeSlotFrom30
    bcc EmitAfterWait07DD_at5
EmitAfterWait07DD_at4:
    jsr VecClearObjectInY
    lda #$31
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    sta a:OBJ_START_ARG,y
    lda a:OBJ_START_ARG,x
    and #$03
    asl a
    tay
    lda a:EmitAfterWait07DAWaits,y
    sta a:OBJ_PIECE_PLACE,x
    lda a:EmitAfterWait07DAWaits+1,y
    sta a:OBJ_PIECE_NEXT,x
EmitAfterWait07DD_at5:
    rts

; @routine 07DD:5EB StepThenByState07DDC5EB
; @run 71:C5EB
; WHAT IT DOES: Steps the list at $C610 ($C604) and goes to the state's handler.
; HOW: A table of handlers at $C600.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepThenByState07DDC5EBStates = $C600 ; a handler per state
StepThenByState07DDC5EB:
    jsr StepList07DDC610
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:StepThenByState07DDC5EBStates,y
    sta z:zTemp00
    lda a:StepThenByState07DDC5EBStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DD:604 StepList07DDC610
; @run 71:C604
; WHAT IT DOES: Steps the list at $C610.
; HOW: A tail jump to StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepList07DDC610List = $C610 ; a frame list
StepList07DDC610:
    lda #$10
    sta z:zTemp10
    lda #$C6
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 07DD:619 ScrollShortOfRightEnd07DD
; @run 71:C619
; WHAT IT DOES: As ScrollShortOfRightEnd051B: carry set when the camera is two screens or more short of the right end, or one
; short with its low byte under $E0.
; HOW: The same instructions at this address.
; WAITS: no.
; Entry: none. Exit: C the answer, A the difference or ScrollXLo.
ScrollShortOfRightEnd07DD:
    lda a:ScrollXMaxHi
    sec
    sbc a:ScrollXHiScreen
    cmp #$02
    bcs ScrollShortOfRightEnd07DD_at0
    cmp #$00
    beq ScrollShortOfRightEnd07DD_at1
    lda a:ScrollXLo
    cmp #$E0
    bcs ScrollShortOfRightEnd07DD_at1
    sec
ScrollShortOfRightEnd07DD_at0:
    rts
ScrollShortOfRightEnd07DD_at1:
    clc
    rts

; @routine 07DD:633 EnterBySideOrFloat07DD
; @run 71:C633
; WHAT IT DOES: With an argument: placed off the right ($C696), then hidden at a random height ($48-$96) with loop flags $20.
; Without: placed at the right or the left of the screen by the player's side, as EnterFromEdgeBySide051B.
; HOW: LoadObjectRecord, two NextRandom, ScrollShortOfRightEnd07DD.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EnterBySideOrFloat07DD:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    beq EnterBySideOrFloat07DD_at0
    jsr PlaceRightWalkingLeft07DD
    stz a:OBJ_HITBOX_ON,x
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
    stz a:OBJ_Y_HI,x
    jsr VecNextRandom
    and #$3F
    clc
    adc #$48
    sta z:zTemp00
    jsr VecNextRandom
    and #$0F
    clc
    adc z:zTemp00
    sta a:OBJ_Y_LO,x
    lda #$20
    sta a:OBJ_LOOP_FLAGS,x
    stz a:OBJ_HIT_REACTION,x
    rts
EnterBySideOrFloat07DD_at0:
    lda a:OBJ_X_LO
    cmp #$40
    bcc PlaceRightWalkingLeft07DD
    cmp #$C0
    bcs EnterBySideOrFloat07DD_at1
    bsr ScrollShortOfRightEnd07DD
    bcc EnterBySideOrFloat07DD_at1
    lda a:OBJ_FACING_LEFT
    beq PlaceRightWalkingLeft07DD
EnterBySideOrFloat07DD_at1:
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$FF
    sta a:OBJ_X_HI,x
    lda #$F0
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
    bra PlaceAbovePlayer07DD

; @routine 07DD:696 PlaceRightWalkingLeft07DD
; @run 71:C696
; WHAT IT DOES: At X $0110 with X speed $FEC0, then Y from the player's ($C6AA).
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PlaceRightWalkingLeft07DD:
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$10
    sta a:OBJ_X_LO,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x

; @routine 07DD:6AA PlaceAbovePlayer07DD
; @run 71:C6AA
; WHAT IT DOES: Y the player's less $10.
; HOW: A 16-bit subtraction.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PlaceAbovePlayer07DD:
    lda a:OBJ_Y_LO
    sec
    sbc #$10
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    sbc #$00
    sta a:OBJ_Y_HI,x
    rts

; @routine 07DD:6BC BobOnCosine07DD
; @run 71:C6BC
; WHAT IT DOES: As BobOnCosine051B: Y speed from the cosine of OBJ_WORK_D, magnitude 1.0; moves, and advances the angle by 2.
; HOW: ScaleByCosine and ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BobOnCosine07DD:
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_PIECE_NEXT,x
    jsr VecScaleByCosine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_PIECE_NEXT,x
    clc
    adc #$02
    sta a:OBJ_PIECE_NEXT,x
    rts

; @routine 07DD:6E1 HitThenByState07DDC6E1
; @run 71:C6E1
; WHAT IT DOES: Breaks the chain on a hit ($C702) and goes to the state's handler.
; HOW: A table of handlers at $C6F6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HitThenByState07DDC6E1States = $C6F6 ; a handler per state
HitThenByState07DDC6E1:
    jsr BreakChainOnHit07DD
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:HitThenByState07DDC6E1States,y
    sta z:zTemp00
    lda a:HitThenByState07DDC6E1States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DD:702 BreakChainOnHit07DD
; @run 71:C702
; WHAT IT DOES: A hit outside state 4: the next slot becomes kind $9C, the six after it lose their hit boxes and pace bits and
; take attack powers from the table; this one goes to state 4 without a hit box or picture.
; HOW: A loop over slots X+2 to X+7.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BreakChainOnHit07DAPowers = $C740 ; OBJ_ATTACK_POWER by link
BreakChainOnHit07DD:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq BreakChainOnHit07DD_at1
    lda a:OBJ_STATE,x
    cmp #$04
    beq BreakChainOnHit07DD_at1
    phx
    inx
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    inx
    cly
BreakChainOnHit07DD_at0:
    lda a:BreakChainOnHit07DAPowers,y
    sta a:OBJ_ATTACK_POWER,x
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_PACE,x
    and #$87
    sta a:OBJ_PACE,x
    inx
    iny
    cpy #$06
    bcc BreakChainOnHit07DD_at0
    plx
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_FRAME,x
    lda #$04
    sta a:OBJ_STATE,x
BreakChainOnHit07DD_at1:
    rts

; @routine 07DD:746 LoadWithRider07DD
; @run 71:C746
; WHAT IT DOES: Loads the record with a wait of $60 and starts a kind $34 object in the next slot; argument bit 7 goes to state 5,
; and $FF to substate 2.
; HOW: ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadWithRider07DD:
    jsr VecLoadObjectRecord
    lda #$60
    sta a:OBJ_PIECE_PLACE,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$34
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    bpl LoadWithRider07DD_at0
    lda #$05
    sta a:OBJ_STATE,x
    lda a:OBJ_START_ARG,x
    cmp #$FF
    bne LoadWithRider07DD_at0
    lda #$02
    sta a:OBJ_SUBSTATE,x
LoadWithRider07DD_at0:
    rts

; @routine 07DD:770 BySubstate07DDC770
; @run 71:C770
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $C782.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate07DDC770Substates = $C782 ; a handler per substate
BySubstate07DDC770:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate07DDC770Substates,y
    sta z:zTemp00
    lda a:BySubstate07DDC770Substates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DD:788 StopAtPlaceOfRider07DD
; @run 71:C788
; WHAT IT DOES: On the first screen, once left of the X the table gives for slot Y's argument, slows to $0020 and goes on.
; HOW: Y is the caller's; the argument's low seven bits index the table.
; WAITS: no.
; Entry: X object, Y a slot. Exit: as the last instruction left them.
StopAtPlaceOfRider07DAPlaces = $C7A9 ; X by argument
StopAtPlaceOfRider07DD:
    lda a:OBJ_X_HI,x
    bne StopAtPlaceOfRider07DD_at0
    lda a:OBJ_START_ARG,y
    and #$7F
    tay
    lda a:OBJ_X_LO,x
    cmp a:StopAtPlaceOfRider07DAPlaces,y
    bcs StopAtPlaceOfRider07DD_at0
    lda #$00
    sta a:OBJ_VX_HI,x
    lda #$20
    sta a:OBJ_VX_LO,x
    inc a:OBJ_SUBSTATE,x
StopAtPlaceOfRider07DD_at0:
    rts

; @routine 07DD:7AD BounceToRest07DD
; @run 71:C7AD
; WHAT IT DOES: Falls; landing at Y $B8 on the first screen, bounces up to three times by the table, then goes to state 2.
; HOW: ApplyGravity and ApplyVelocity; OBJ_WORK_J counts the bounces.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BounceToRest07DABounces = $C7E3 ; VY words by bounce
BounceToRest07DD:
    jsr VecApplyGravity
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne StartState2_07DD_at0
    lda a:OBJ_Y_LO,x
    cmp #$B8
    bcc StartState2_07DD_at0
    lda #$B8
    sta a:OBJ_Y_LO,x
    lda a:OBJ_WORK_J,x
    cmp #$03
    bcs StartState2_07DD
    asl a
    tay
    lda a:BounceToRest07DABounces,y
    sta a:OBJ_VY_LO,x
    lda a:BounceToRest07DABounces+1,y
    sta a:OBJ_VY_HI,x
    inc a:OBJ_WORK_J,x
    rts

; @routine 07DD:7DD StartState2_07DD
; @run 71:C7DD
; WHAT IT DOES: Goes to state 2.
; HOW: A store.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartState2_07DD:
    lda #$02
    sta a:OBJ_STATE,x
StartState2_07DD_at0:
    rts

; @routine 07DD:7EB DriftAwayWhenFar07DD
; @run 71:C7EB
; WHAT IT DOES: Within $30 of the player, state 2; else drifts left at $FFC0 and bobs ($C7FF).
; HOW: DistanceXToPlayer, ApplyVelocityX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DriftAwayWhenFar07DD:
    jsr VecDistanceXToPlayer
    cmp #$30
    bcc StartState2_07DD
    lda #$FF
    sta a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX

; @routine 07DD:7FF BobSlowlyOnCosine07DD
; @run 71:C7FF
; WHAT IT DOES: Y speed from the cosine of OBJ_WORK_D at $20, moves down it and advances the angle by 2; each turn resets Y to $CE.
; HOW: ScaleByCosine and ApplyVelocityY.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BobSlowlyOnCosine07DD:
    lda #$20
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_PIECE_NEXT,x
    jsr VecScaleByCosine
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_PIECE_NEXT,x
    clc
    adc #$02
    sta a:OBJ_PIECE_NEXT,x
    bcc BobSlowlyOnCosine07DD_at0
    lda #$CE
    sta a:OBJ_Y_LO,x
BobSlowlyOnCosine07DD_at0:
    rts
