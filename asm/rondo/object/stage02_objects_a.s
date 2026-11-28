; @routine 051B:195 BounceSpeeds051B
; @run 70:B195
; WHAT IT DOES: In state 0 loads the record, sets sprite set 4 and picks one of eight speed pairs at random; else goes on at $B203.
; HOW: A table row of four bytes by NextRandom's low three bits.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BounceSpeeds051BTable = $B1C3 ; eight rows: VX low, high, VY low, high
BounceSpeeds051B:
    lda a:OBJ_STATE,x
    bne StepListThenFallOrLeave051B
    jsr VecLoadObjectRecord
    lda #$04
    sta a:OBJ_SPRITE_SET,x
    jsr VecNextRandom
    and #$07
    asl a
    asl a
    tay
    lda a:BounceSpeeds051BTable+1,y
    sta a:OBJ_VX_HI,x
    lda a:BounceSpeeds051BTable,y
    sta a:OBJ_VX_LO,x
    lda a:BounceSpeeds051BTable+3,y
    sta a:OBJ_VY_HI,x
    lda a:BounceSpeeds051BTable+2,y
    sta a:OBJ_VY_LO,x
    rts

; @routine 051B:203 StepListThenFallOrLeave051B
; @run 70:B203
; WHAT IT DOES: Steps the list at $B21E; on the first screen below its top (Y high 0 or negative) falls under gravity, else the slot is cleared.
; HOW: StepObjectFrames, then ApplyGravityUnlimited and ApplyVelocity, or ClearObject.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepListThenFallOrLeave051B:
    lda #$1E
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_Y_HI,x
    beq StepListThenFallOrLeave051B_at0
    bmi StepListThenFallOrLeave051B_at0
    jmp VecClearObject
StepListThenFallOrLeave051B_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 051B:2E7 CountCheckThenByState051B
; @run 70:B2E7
; WHAT IT DOES: Counts OBJ_WORK_F up, runs the hit check at $B30D and goes to the state's handler.
; HOW: A table of handlers at $B2FF.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountCheckThenByState051BStates = $B2FF ; a handler per state
CountCheckThenByState051B:
    inc a:OBJ_SWOOP_SPEED_LO,x
    jsr StateSixOnStatusBit7
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CountCheckThenByState051BStates,y
    sta z:zTemp00
    lda a:CountCheckThenByState051BStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051B:475 TurnListThenState2_051B
; @run 70:B475
; WHAT IT DOES: Runs $B4B0 for the hit box, steps the list picked by the swoop speed and facing, and when it ends sets the box and state 2.
; HOW: A table of list addresses at $B498.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
TurnListThenState2_051BLists = $B498 ; list addresses by OBJ_SWOOP_SPEED_HI xor OBJ_FACING_LEFT
TurnListThenState2_051B:
    jsr SetHitBoxByFrame051B
    lda a:OBJ_SWOOP_SPEED_HI,x
    eor a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:TurnListThenState2_051BLists,y
    sta z:zTemp10
    lda a:TurnListThenState2_051BLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne TurnListThenState2_051B_at0
    jsr SetHitBox0C18
    lda #$02
    sta a:OBJ_STATE,x
TurnListThenState2_051B_at0:
    rts

; @routine 051B:4B0 SetHitBoxByFrame051B
; @run 70:B4B0
; WHAT IT DOES: Sets the hit box by the frame: $0A and $0B stretched ($B4CD), $0D crouched ($B4E2), else the standing box ($B4BF).
; HOW: Compares, then goes to the arm.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHitBoxByFrame051B:
    lda a:OBJ_FRAME,x
    cmp #$0A
    beq SetHitBoxStretched051B
    cmp #$0B
    beq SetHitBoxStretched051B
    cmp #$0D
    beq SetHitBoxCrouched051B

; @routine 051B:4CD SetHitBoxStretched051B
; @run 70:B4CD
; WHAT IT DOES: The stretched hit box: offset -8, 4; half size $14 by $14.
; HOW: Four stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHitBoxStretched051B:
    lda #$F8
    sta a:OBJ_HITBOX_DX,x
    lda #$04
    sta a:OBJ_HITBOX_DY,x
    lda #$14
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$14
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 051B:4E2 SetHitBoxCrouched051B
; @run 70:B4E2
; WHAT IT DOES: The crouched hit box: offset 9, 2; half size $0C by $17.
; HOW: Four stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHitBoxCrouched051B:
    lda #$09
    sta a:OBJ_HITBOX_DX,x
    lda #$02
    sta a:OBJ_HITBOX_DY,x
    lda #$0C
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$17
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 051B:4F7 LandThenBySubstate051B
; @run 70:B4F7
; WHAT IT DOES: Asks StopOnLanding with height 4, then goes to the substate's handler.
; HOW: A table of handlers at $B50E.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LandThenBySubstate051BSubstates = $B50E ; a handler per substate
LandThenBySubstate051B:
    lda #$04
    jsr VecStopOnLanding
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:LandThenBySubstate051BSubstates,y
    sta z:zTemp00
    lda a:LandThenBySubstate051BSubstates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051B:52F ThrowOnAnimation051B
; @run 70:B52F
; WHAT IT DOES: Steps the list at $B57A and clears the object when it ends; at frame $1D or later starts a thrown object in a free slot
; from $30, its kind and speeds by OBJ_FALL_LO, which counts the throws.
; HOW: FindFreeSlotFrom30, StartObjectAtPlace, speeds from a table, one step of movement for the new object.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowOnAnimation051BKinds = $B59D ; the thrown kind by throw
ThrowOnAnimation051BSpeeds = $B5A5 ; X high and Y high by throw
ThrowOnAnimation051B:
    lda #$7A
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ThrowOnAnimation051B_at0
    jmp VecClearObject
ThrowOnAnimation051B_at0:
    bcc ThrowOnAnimation051B_at1
    lda a:OBJ_FRAME,x
    cmp #$1D
    bcc ThrowOnAnimation051B_at1
    jsr VecFindFreeSlotFrom30
    bcc ThrowOnAnimation051B_at1
    lda #$15
    jsr VecSoundCommand
    phy
    lda a:OBJ_FALL_LO,x
    tay
    lda a:ThrowOnAnimation051BKinds,y
    ply
    jsr VecStartObjectAtPlace
    phx
    lda a:OBJ_FALL_LO,x
    asl a
    tax
    lda a:ThrowOnAnimation051BSpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:ThrowOnAnimation051BSpeeds+1,x
    sta a:OBJ_VY_HI,y
    plx
    sxy
    jsr VecApplyVelocity
    sxy
    inc a:OBJ_FALL_LO,x
ThrowOnAnimation051B_at1:
    rts

; @routine 051B:678 BySubstate051B678
; @run 70:B678
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $B68A.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate051B678Substates = $B68A ; a handler per substate
BySubstate051B678:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate051B678Substates,y
    sta z:zTemp00
    lda a:BySubstate051B678Substates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051B:6A1 BreakWhenCameraReaches051B
; @run 70:B6A1
; WHAT IT DOES: On the first screen, once its X reaches the column in OBJ_START_ARG's low digit: draws the tile script the high digit
; names, clears the next slot, starts four pieces ($B6ED), records the event, sound $81, and goes on as the broken thing.
; HOW: A compare of X high and X low / 16, then the calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BreakWhenCameraReaches051B:
    lda a:OBJ_X_HI,x
    bne BreakWhenCameraReaches051B_at0
    lda a:OBJ_START_ARG,x
    and #$0F
    sta z:zTemp00
    lda a:OBJ_X_LO,x
    lsr a
    lsr a
    lsr a
    lsr a
    cmp z:zTemp00
    bcs BreakWhenCameraReaches051B_at0
    lda a:OBJ_START_ARG,x
    and #$70
    lsr a
    lsr a
    lsr a
    lsr a
    inc a
    cly
    jsr VecDrawTileScript
    txa
    tay
    iny
    jsr VecClearObjectInY
    jsr StartFourPiecesWithCount
    jsr VecRecordObjectEvent
    lda #$81
    jsr VecSoundCommand
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_LOOP_FLAGS,x
    ora #$40
    sta a:OBJ_LOOP_FLAGS,x
    lda #$0E
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
BreakWhenCameraReaches051B_at0:
    rts

; @routine 051B:730 HitThenByState051B730
; @run 70:B730
; WHAT IT DOES: Under state 4, a hit (OBJ_HIT_STATUS bit 7) restarts the animation, sets OBJ_PIECE_PLACE $20 and state 4; then goes
; to the state's handler.
; HOW: A table of handlers at $B760.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HitThenByState051B730States = $B760 ; a handler per state
HitThenByState051B730:
    lda a:OBJ_STATE,x
    cmp #$04
    bcs HitThenByState051B730_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitThenByState051B730_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$20
    sta a:OBJ_PIECE_PLACE,x
    lda #$04
    sta a:OBJ_STATE,x
HitThenByState051B730_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:HitThenByState051B730States,y
    sta z:zTemp00
    lda a:HitThenByState051B730States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051B:7C4 StepListB819
; @run 70:B7C4
; WHAT IT DOES: Steps the list at $B819 and goes on at $B7CF.
; HOW: StepObjectFrames, then into LandWithSpeedsB7CF.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as LandWithSpeedsB7CF.
StepListB819:
    lda #$19
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames

; @routine 051B:7CF LandWithSpeedsB7CF
; @run 70:B7CF
; WHAT IT DOES: Asks StopOnLanding with height 6. Against a wall sets OBJ_HIT_STATUS bit 7. On landing takes speeds from the row of
; four at $B83A that OBJ_WORK_D picks and, while it is under 5, counts it up, with sound $2C the first time.
; HOW: StopOnLanding with its flags kept by PHP and PLP across the wall test.
; WAITS: no.
; Entry: X object, Y and C as the frame step left them. Exit: as the last instruction left them.
LandWithSpeedsB7CFSpeeds = $B83A ; five rows: VX low, high, VY low, high
LandWithSpeedsB7CF:
    lda #$06
    jsr VecStopOnLanding
    php
    and #$02
    beq LandWithSpeedsB7CF_at0
    plp
    lda a:OBJ_HIT_STATUS,x
    ora #$80
    sta a:OBJ_HIT_STATUS,x
    rts
LandWithSpeedsB7CF_at0:
    plp
    bcc LandWithSpeedsB7CF_at2
    lda a:OBJ_PIECE_NEXT,x
    asl a
    asl a
    tay
    lda a:LandWithSpeedsB7CFSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LandWithSpeedsB7CFSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:LandWithSpeedsB7CFSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:LandWithSpeedsB7CFSpeeds+2,y
    sta a:OBJ_VY_LO,x
    lda a:OBJ_PIECE_NEXT,x
    cmp #$05
    bcs LandWithSpeedsB7CF_at2
    lda a:OBJ_PIECE_NEXT,x
    bne LandWithSpeedsB7CF_at1
    lda #$2C
    jsr VecSoundCommand
LandWithSpeedsB7CF_at1:
    inc a:OBJ_PIECE_NEXT,x
LandWithSpeedsB7CF_at2:
    rts

; @routine 051C:052 StepThenCountOutOrLand051B
; @run 70:B852
; WHAT IT DOES: Steps the list at $B86C; counts OBJ_PIECE_PLACE down and clears the object at zero, else goes on at $B7CF.
; HOW: No hit box meanwhile.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepThenCountOutOrLand051B:
    stz a:OBJ_HITBOX_ON,x
    lda #$6C
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_PIECE_PLACE,x
    bne StepThenCountOutOrLand051B_at0
    jmp VecClearObject
StepThenCountOutOrLand051B_at0:
    jmp LandWithSpeedsB7CF

; @routine 051C:071 CountBoxHitByState051B871
; @run 70:B871
; WHAT IT DOES: Counts OBJ_WORK_J up, sets the hit box ($B894), checks for a hit ($B8E7) and goes to the state's handler.
; HOW: A table of handlers at $B88C.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountBoxHitByState051B871States = $B88C ; a handler per state
CountBoxHitByState051B871:
    inc a:OBJ_WORK_J,x
    jsr SetHitBoxByFrame051B894
    jsr State3WhenHit051B
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CountBoxHitByState051B871States,y
    sta z:zTemp00
    lda a:CountBoxHitByState051B871States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051C:094 SetHitBoxByFrame051B894
; @run 70:B894
; WHAT IT DOES: The hit box by the frame: none at frame 0, a long one at 5 and a longer one at 6, else the standing one.
; HOW: Compares and stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHitBoxByFrame051B894:
    lda a:OBJ_FRAME,x
    beq SetHitBoxByFrame051B894_at2
    cmp #$05
    beq SetHitBoxByFrame051B894_at0
    cmp #$06
    beq SetHitBoxByFrame051B894_at1
    lda #$F4
    sta a:OBJ_HITBOX_DX,x
    lda #$F0
    sta a:OBJ_HITBOX_DY,x
    lda #$1C
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$28
    sta a:OBJ_HITBOX_HALF_H,x
    rts
SetHitBoxByFrame051B894_at0:
    lda #$D4
    sta a:OBJ_HITBOX_DX,x
    lda #$F8
    sta a:OBJ_HITBOX_DY,x
    lda #$3C
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$10
    sta a:OBJ_HITBOX_HALF_H,x
    rts
SetHitBoxByFrame051B894_at1:
    lda #$C8
    sta a:OBJ_HITBOX_DX,x
    lda #$F8
    sta a:OBJ_HITBOX_DY,x
    lda #$48
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$10
    sta a:OBJ_HITBOX_HALF_H,x
    rts
SetHitBoxByFrame051B894_at2:
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 051C:0E7 State3WhenHit051B
; @run 70:B8E7
; WHAT IT DOES: Under state 3, a hit (OBJ_HIT_STATUS bit 7) goes to state 3, substate 0.
; HOW: Two tests.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
State3WhenHit051B:
    lda a:OBJ_STATE,x
    cmp #$03
    bcs State3WhenHit051B_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State3WhenHit051B_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State3WhenHit051B_at0:
    rts

; @routine 051C:0FE LoadThenPartTwoSlotsOn051B
; @run 70:B8FE
; WHAT IT DOES: Loads the record, clears the slot two on and starts a kind $25 object there at this one's place.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadThenPartTwoSlotsOn051B:
    jsr VecLoadObjectRecord
    txa
    tay
    iny
    iny
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    rts

; @routine 051C:10E SwayThenState2_051B
; @run 70:B90E
; WHAT IT DOES: Every $40 counts of OBJ_WORK_J goes to state 2. Meanwhile steps the list at $B950 and, on frames that step, moves
; sideways by the speed OBJ_FALL_LO picks, turning every $20 moves.
; HOW: StepObjectFrames, ApplyVelocityX, a count in OBJ_SWOOP_SPEED_HI.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SwayThenState2_051BSpeeds = $B961 ; the X speed high byte by direction
SwayThenState2_051B:
    lda a:OBJ_WORK_J,x
    and #$3F
    bne SwayThenState2_051B_at0
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts
SwayThenState2_051B_at0:
    lda #$50
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc SwayThenState2_051B_at1
    ldy a:OBJ_FALL_LO,x
    lda a:SwayThenState2_051BSpeeds,y
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    inc a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_SWOOP_SPEED_HI,x
    cmp #$20
    bcc SwayThenState2_051B_at1
    lda a:OBJ_FALL_LO,x
    eor #$01
    sta a:OBJ_FALL_LO,x
    stz a:OBJ_SWOOP_SPEED_HI,x
SwayThenState2_051B_at1:
    rts

; @routine 051C:163 BySubstate051B963
; @run 70:B963
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $B975.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate051B963Substates = $B975 ; a handler per substate
BySubstate051B963:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate051B963Substates,y
    sta z:zTemp00
    lda a:BySubstate051B963Substates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051C:17F NextAttackFromList051B
; @run 70:B97F
; WHAT IT DOES: Takes the next substate from the eight at $B99D, counted by OBJ_SWOOP_SPEED_LO; substate 0 means state 1.
; HOW: A table read; the animation restarts for another substate.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NextAttackFromList051BOrder = $B99D ; eight substates in turn
NextAttackFromList051B:
    lda a:OBJ_SWOOP_SPEED_LO,x
    and #$07
    tay
    inc a:OBJ_SWOOP_SPEED_LO,x
    lda a:NextAttackFromList051BOrder,y
    sta a:OBJ_SUBSTATE,x
    bne NextAttackFromList051B_at0
    lda #$01
    sta a:OBJ_STATE,x
    rts
NextAttackFromList051B_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    rts

; @routine 051C:1A5 SubstateThreeIfNear051B
; @run 70:B9A5
; WHAT IT DOES: Half the time, with the player within $70, goes to substate 3; else to the next substate.
; HOW: NextRandom bit 0, DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SubstateThreeIfNear051B:
    jsr VecNextRandom
    and #$01
    beq SubstateThreeIfNear051B_at0
    jsr VecDistanceXToPlayer
    cmp #$70
    bcs SubstateThreeIfNear051B_at0
    lda #$03
    sta a:OBJ_SUBSTATE,x
    rts
SubstateThreeIfNear051B_at0:
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 051C:1BD AnimateThenDropThree051B
; @run 70:B9BD
; WHAT IT DOES: Steps the list at $BA06. At its end goes to state 1. At step $0F sounds $25 and starts up to three kind $43 objects
; in free slots from $51, numbered 0 to 2 in OBJ_START_ARG.
; HOW: A walk over slots $51-$56 for empty ones.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
AnimateThenDropThree051B:
    lda #$06
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq AnimateThenDropThree051B_at2
    lda a:OBJ_ANIM_STEP,x
    cmp #$0F
    bne AnimateThenDropThree051B_at3
    lda #$25
    jsr VecSoundCommand
    stz z:zTemp00
    ldy #$51
AnimateThenDropThree051B_at0:
    lda a:OBJ_KIND,y
    bne AnimateThenDropThree051B_at1
    jsr VecClearObjectInY
    lda #$43
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    inc z:zTemp00
    lda z:zTemp00
    cmp #$03
    beq AnimateThenDropThree051B_at3
AnimateThenDropThree051B_at1:
    iny
    cpy #$57
    bcc AnimateThenDropThree051B_at0
    rts
AnimateThenDropThree051B_at2:
    stz a:OBJ_PIECE_PLACE,x
    lda #$01
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
AnimateThenDropThree051B_at3:
    rts
