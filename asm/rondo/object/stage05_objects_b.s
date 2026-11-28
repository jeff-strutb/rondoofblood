; @routine 093C:02F ItemMarkThenByState093C
; @run 70:B82F
; WHAT IT DOES: Checks contact ($B85D), the flag ($B868) and the mark ($B882); touching it ends here, else clears slot 5, sets the
; hit box offset and goes to the state's handler.
; HOW: A table of handlers at $B853.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ItemMarkThenByState093CStates = $B853 ; a handler per state
ItemMarkThenByState093C:
    jsr ContactWithPlayerBox14By20
    jsr EnterStateFourOnFlag
    jsr TouchesPlayerMark
    bcc ItemMarkThenByState093C_at0
    rts
ItemMarkThenByState093C_at0:
    jsr ProbeFromBelow093C
    jsr SetHitboxDxFromFrame
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ItemMarkThenByState093CStates,y
    sta z:zTemp00
    lda a:ItemMarkThenByState093CStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093C:0B2 ProbeFromBelow093C
; @run 70:B8B2
; WHAT IT DOES: On the first screen past state 0: probes $10 times OBJ_START_ARG below, falling, and stops on ground there.
; HOW: Y moved down for the probe and back.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ProbeFromBelow093C:
    lda a:OBJ_STATE,x
    beq ProbeFromBelow093C_at1
    lda a:OBJ_Y_HI,x
    bne ProbeFromBelow093C_at1
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_Y_LO,x
    clc
    adc z:zTemp00
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    adc #$00
    sta a:OBJ_Y_HI,x
    jsr VecApplyVelocityY
    jsr VecApplyGravity
    lda #$04
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    and #$01
    beq ProbeFromBelow093C_at0
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
ProbeFromBelow093C_at0:
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp00
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_Y_HI,x
ProbeFromBelow093C_at1:
    rts

; @routine 093C:273 BySubstate093CBA73
; @run 70:BA73
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $BA85.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate093CBA73Substates = $BA85 ; a handler per substate
BySubstate093CBA73:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate093CBA73Substates,y
    sta z:zTemp00
    lda a:BySubstate093CBA73Substates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093C:2F3 TossObjectByCount093C
; @run 70:BAF3
; WHAT IT DOES: In a free slot from $30 (none: $BB85) starts the kind the table gives for OBJ_RAIN_COUNT, thrown left at -8 or,
; with OBJ_WORK_J, at -18 facing the other way; goes to the next substate.
; HOW: The count and parent kept in the new object.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
TossObjectByCount093CKinds = $BB35 ; kind by count
TossObjectByCount093C:
    jsr VecFindFreeSlotFrom30
    bcs TossObjectByCount093C_at0
    jmp State1ByWorkJ093C
TossObjectByCount093C_at0:
    sty z:zBurstCount
    ldy a:OBJ_WORK_I,x
    lda a:TossObjectByCount093CKinds,y
    ldy z:zBurstCount
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_I,x
    sta a:OBJ_WORK_D,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    txa
    sta a:OBJ_WORK_F,y
    lda a:OBJ_WORK_J,x
    bne TossObjectByCount093C_at1
    lda #$F8
    sta a:OBJ_VX_HI,y
    bra TossObjectByCount093C_at2
TossObjectByCount093C_at1:
    lda #$EE
    sta a:OBJ_VX_HI,y
    lda a:OBJ_FACING_LEFT,y
    eor #$01
    sta a:OBJ_FACING_LEFT,y
TossObjectByCount093C_at2:
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093C:385 State1ByWorkJ093C
; @run 70:BB85
; WHAT IT DOES: State 1 with frame 1, or 4 with OBJ_WORK_J.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
State1ByWorkJ093C:
    lda #$01
    sta a:OBJ_STATE,x
    lda #$01
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_J,x
    beq State1ByWorkJ093C_at0
    lda #$04
    sta a:OBJ_FRAME,x
State1ByWorkJ093C_at0:
    rts

; @routine 093C:4DF TableThenFollow093C
; @run 70:BCDF
; WHAT IT DOES: Dispatches by state ($BCE5) and follows the parent ($BC8F).
; HOW: Two routines.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
TableThenFollow093C:
    jsr JumpByStateTable093C4E5
    jmp FollowParentYAndLeave

; @routine 093C:612 CountHitThenByState093C
; @run 70:BE12
; WHAT IT DOES: Counts OBJ_FALL_LO, bursts when hit ($BE38) and goes to the state's handler.
; HOW: A table of handlers at $BE2A.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountHitThenByState093CStates = $BE2A ; a handler per state
CountHitThenByState093C:
    inc a:OBJ_WORK_H,x
    jsr BurstWhenHit093C
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CountHitThenByState093CStates,y
    sta z:zTemp00
    lda a:CountHitThenByState093CStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093C:638 BurstWhenHit093C
; @run 70:BE38
; WHAT IT DOES: A hit outside state 6 sounds $93, bursts ($BE5D) and goes to state 6 without a hit box.
; HOW: A call and stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BurstWhenHit093C:
    lda a:OBJ_STATE,x
    cmp #$06
    beq BurstWhenHit093C_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq BurstWhenHit093C_at0
    lda #$93
    jsr VecSoundCommand
    jsr ScatterSix093C
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
BurstWhenHit093C_at0:
    rts

; @routine 093C:65D ScatterSix093C
; @run 70:BE5D
; WHAT IT DOES: Starts up to six kind $2B pieces in free slots, each with the row's waits and speeds.
; HOW: The row in zBurstCount, X kept in zClimbStepBefore.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ScatterSix093CRows = $BE93 ; four bytes per piece: OBJ_PIECE_NEXT, OBJ_PIECE_PLACE, VX high, VY high
ScatterSix093C:
    cly
ScatterSix093C_at0:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc ScatterSix093C_at1
    lda #$2B
    jsr VecStartObjectAtPlace
    stx z:zClimbStepBefore
    lda z:zBurstCount
    asl a
    asl a
    tax
    lda a:ScatterSix093CRows,x
    sta a:OBJ_WORK_D,y
    lda a:ScatterSix093CRows+1,x
    sta a:OBJ_WORK_E,y
    lda a:ScatterSix093CRows+2,x
    sta a:OBJ_VX_HI,y
    lda a:ScatterSix093CRows+3,x
    sta a:OBJ_VY_HI,y
    ldx z:zClimbStepBefore
    ldy z:zBurstCount
    iny
    cpy #$06
    bcc ScatterSix093C_at0
ScatterSix093C_at1:
    rts

; @routine 093C:767 BySubstate093CBF67
; @run 70:BF67
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $BF79.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate093CBF67Substates = $BF79 ; a handler per substate
BySubstate093CBF67:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate093CBF67Substates,y
    sta z:zTemp00
    lda a:BySubstate093CBF67Substates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093C:781 SpringAway093C
; @run 70:BF81
; WHAT IT DOES: As SpringAway07DB with this stage's speeds ($BFAA) and halving ($C016).
; HOW: PlayerDistanceSum, NegateVelocityX and Y.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SpringAway093C:
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_K,x
    jsr VecPlayerDistanceSum
    and #$0F
    sta a:OBJ_WORK_I,x
    jsr PickSpeedAndTurnFromTableC
    jsr VecNegateVelocityX
    jsr VecNegateVelocityY
    jsr HalveVelocity093D
    lda #$0C
    sta a:OBJ_WORK_D,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093D:016 HalveVelocity093D
; @run 71:C016
; WHAT IT DOES: Halves both speeds, keeping their signs.
; HOW: ASL for the sign into carry, then ROR of the high and low bytes.
; WAITS: no.
; Entry: X object. Exit: C the last bit out, N and Z from VY low.
HalveVelocity093D:
    lda a:OBJ_VX_HI,x
    asl a
    ror a:OBJ_VX_HI,x
    ror a:OBJ_VX_LO,x
    lda a:OBJ_VY_HI,x
    asl a
    ror a:OBJ_VY_HI,x
    ror a:OBJ_VY_LO,x
    rts

; @routine 093D:055 ChaseAndRise093D
; @run 71:C055
; WHAT IT DOES: As ChaseAndRise07DB with this stage's clamp ($C0C0), pictures ($BEE9), throw ($C0E5) and sound ($C0F5).
; HOW: Calls and 16-bit adds.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChaseAndRise093DPulls = $C0BC ; X acceleration words by side
ChaseAndRise093D:
    jsr VecApplyVelocity
    jsr VecSideOfPlayer
    and #$01
    sta z:zTemp00
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:ChaseAndRise093DPulls,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:ChaseAndRise093DPulls+1,y
    sta a:OBJ_VX_HI,x
    jsr ClampSpeedXToTwo093D
    lda a:OBJ_VY_LO,x
    sec
    sbc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    bmi ChaseAndRise093D_at0
    ldy #$04
    jsr StepFramesFacingPlayerC
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor z:zTemp00
    beq ChaseAndRise093D_at1
    lda #$20
    sta a:OBJ_FRAME,x
    rts
ChaseAndRise093D_at0:
    jsr ThrowUpward093D
    ldy #$04
    jsr StepFramesFacingPlayerC
    jsr SoundEveryFourthFrame
    lda a:OBJ_Y_HI,x
    bne ChaseAndRise093D_at1
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_WORK_J,x
    bcs ChaseAndRise093D_at1
    inc a:OBJ_SUBSTATE,x
ChaseAndRise093D_at1:
    rts

; @routine 093D:0C0 ClampSpeedXToTwo093D
; @run 71:C0C0
; WHAT IT DOES: Keeps the X speed within -2 and 2.
; HOW: Compares of the high byte.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ClampSpeedXToTwo093D:
    lda a:OBJ_VX_HI,x
    bmi ClampSpeedXToTwo093D_at1
    lda a:OBJ_VX_HI,x
    cmp #$02
    bcc ClampSpeedXToTwo093D_at0
    lda #$02
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
ClampSpeedXToTwo093D_at0:
    rts
ClampSpeedXToTwo093D_at1:
    lda a:OBJ_VX_HI,x
    cmp #$FE
    bcs ClampSpeedXToTwo093D_at0
    lda #$FE
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    rts

; @routine 093D:0E5 ThrowUpward093D
; @run 71:C0E5
; WHAT IT DOES: Unless already rising at -2 or less, rises at -3.
; HOW: A compare of the Y speed.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowUpward093D:
    lda a:OBJ_VY_HI,x
    cmp #$FE
    bcs ThrowUpward093D_at0
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
ThrowUpward093D_at0:
    rts

; @routine 093D:102 HoverNearTop093D
; @run 71:C102
; WHAT IT DOES: As HoverNearTop07DB with this stage's pictures, sound and halving.
; HOW: HalveVelocity093D.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HoverNearTop093D:
    ldy #$04
    jsr StepFramesFacingPlayerC
    jsr SoundEveryFourthFrame
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne HoverNearTop093D_at1
    lda a:OBJ_Y_LO,x
    cmp #$10
    bcc HoverNearTop093D_at0
    lda a:OBJ_WORK_H,x
    and #$01
    bne HoverNearTop093D_at1
    jsr HalveVelocity093D
    lda a:OBJ_VY_HI,x
    ora a:OBJ_VY_LO,x
    beq HoverNearTop093D_at0
    lda a:OBJ_VY_HI,x
    and a:OBJ_VY_LO,x
    cmp #$FF
    bne HoverNearTop093D_at1
HoverNearTop093D_at0:
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HoverNearTop093D_at1:
    rts

; @routine 093D:1AF BecomeType9CAtListEnd093D
; @run 71:C1AF
; WHAT IT DOES: Steps list 8 ($BEE9) and becomes kind $9C when it ends.
; HOW: StepFramesFacingPlayerC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BecomeType9CAtListEnd093D:
    ldy #$08
    jsr StepFramesFacingPlayerC
    bne BecomeType9CAtListEnd093D_at0
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
BecomeType9CAtListEnd093D_at0:
    rts

; @routine 093D:1BF CountThenByState093D1BF
; @run 71:C1BF
; WHAT IT DOES: Counts down to state 4 ($C1DE) and goes to the state's handler.
; HOW: A table of handlers at $C1D4.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountThenByState093D1BFStates = $C1D4 ; a handler per state
CountThenByState093D1BF:
    jsr CountdownThenState4_093D
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CountThenByState093D1BFStates,y
    sta z:zTemp00
    lda a:CountThenByState093D1BFStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093D:1DE CountdownThenState4_093D
; @run 71:C1DE
; WHAT IT DOES: Counts OBJ_PIECE_PLACE out and goes to state 4.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountdownThenState4_093D:
    dec a:OBJ_WORK_E,x
    bne CountdownThenState4_093D_at0
    lda #$04
    sta a:OBJ_STATE,x
CountdownThenState4_093D_at0:
    rts

; @routine 093D:1E9 LoadRecordStartAngle40_093D
; @run 71:C1E9
; WHAT IT DOES: Loads the record, moves, and starts the angle at $40.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadRecordStartAngle40_093D:
    jsr VecLoadObjectRecord
    jsr VecApplyVelocity
    lda #$40
    sta a:OBJ_WORK_F,x
    rts

; @routine 093D:1FE OrbitStepBack093D
; @run 71:C1FE
; WHAT IT DOES: Moves along the angle at speed 1 and turns it back by 4; at $E0 starts again at $40 and goes on.
; HOW: SetVelocityFromAngle and ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
OrbitStepBack093D:
    stz z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_F,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_F,x
    sec
    sbc #$04
    sta a:OBJ_WORK_F,x
    cmp #$E0
    bne OrbitStepBack093D_at0
    lda #$40
    sta a:OBJ_WORK_F,x
    inc a:OBJ_STATE,x
OrbitStepBack093D_at0:
    rts

; @routine 093D:223 OrbitStepForward093D
; @run 71:C223
; WHAT IT DOES: Moves along the angle at speed 1 and turns it on by 4; at $A0 starts again at $40 and goes back.
; HOW: SetVelocityFromAngle and ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
OrbitStepForward093D:
    stz z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_F,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_F,x
    clc
    adc #$04
    sta a:OBJ_WORK_F,x
    cmp #$A0
    bne OrbitStepForward093D_at0
    lda #$40
    sta a:OBJ_WORK_F,x
    dec a:OBJ_STATE,x
OrbitStepForward093D_at0:
    rts

; @routine 093D:251 ChecksThenByState093D251
; @run 71:C251
; WHAT IT DOES: Counts, faces the player in some states, takes damage ($C29B), the flag ($C2B7) and the hit box ($C2D7), then goes
; to the state's handler.
; HOW: A table of handlers at $C272.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChecksThenByState093D251States = $C272 ; a handler per state
ChecksThenByState093D251:
    inc a:OBJ_WORK_H,x
    jsr FaceThePlayerInSomeStates
    jsr DamageToState6_093D
    jsr EnterState7OnFlagOrYSignChange
    jsr SetHitboxFromFrameRowsIndirect
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ChecksThenByState093D251States,y
    sta z:zTemp00
    lda a:ChecksThenByState093D251States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093D:29B DamageToState6_093D
; @run 71:C29B
; WHAT IT DOES: With OBJ_RAIN_COUNT set and under state 6, damage that leaves zTemp00 goes to state 6.
; HOW: HitDamage0939.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DamageToState6_093D:
    lda a:OBJ_WORK_I,x
    beq DamageToState6_093D_at0
    lda a:OBJ_STATE,x
    cmp #$06
    bcs DamageToState6_093D_at0
    jsr HitDamage0939
    lda z:zTemp00
    beq DamageToState6_093D_at0
    lda #$06
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
DamageToState6_093D_at0:
    rts

; @routine 093D:316 NextStateOnFrontLanding093D
; @run 71:C316
; WHAT IT DOES: Goes to the next state on landing at the front column, height 6.
; HOW: StopOnLandingFrontColumn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NextStateOnFrontLanding093D:
    lda #$06
    jsr VecStopOnLandingFrontColumn
    bcc NextStateOnFrontLanding093D_at0
    inc a:OBJ_STATE,x
NextStateOnFrontLanding093D_at0:
    rts

; @routine 093D:321 WalkNearOrAttack093D
; @run 71:C321
; WHAT IT DOES: Walks ($C33A with Y 0), keeps the facing, goes to state 3 within $4C and checks the ground ($C3B3).
; HOW: DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkNearOrAttack093D:
    cly
    jsr WalkWhileFrameChangesB
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$4C
    bcs WalkNearOrAttack093D_at0
    lda #$03
    sta a:OBJ_STATE,x
WalkNearOrAttack093D_at0:
    jmp CheckGroundAhead093D

; @routine 093D:36E MoveTurningAtWalls093D
; @run 71:C36E
; WHAT IT DOES: Moves sideways; when the terrain ahead is solid at probes 1 and 2, turns back and moves. Carry clear when it
; turned, set when not.
; HOW: ApplyVelocityX and two ProbeTerrain with SpriteFlipMask 6.
; WAITS: no.
; Entry: X object, Y and C for the probe. Exit: C.
MoveTurningAtWalls093D:
    jsr VecApplyVelocityX
    lda #$06
    sta z:SpriteFlipMask
    lda #$01
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    tay
    lda a:TileHit+1,y
    and #$01
    beq MoveTurningAtWalls093D_at0
    lda #$06
    sta z:SpriteFlipMask
    lda #$02
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    asl a
    tay
    lda a:TileHit,y
    ora a:TileHit+1,y
    and #$02
    bne MoveTurningAtWalls093D_at0
    sec
    rts
MoveTurningAtWalls093D_at0:
    jsr VecNegateVelocityX
    jsr VecApplyVelocityX
    clc
    rts

; @routine 093D:3B3 CheckGroundAhead093D
; @run 71:C3B3
; WHAT IT DOES: Without solid ground below, or with a wall ahead on its side, goes to state 4; otherwise counts OBJ_WORK_D down
; to state 5.
; HOW: Two ProbeTerrain with SpriteFlipMask 6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CheckGroundAhead093D:
    lda #$06
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    lda a:TileHit
    and a:TileHit+1
    and a:TileHit+2
    and #$01
    beq CheckGroundAhead093D_at0
    lda #$06
    sta z:SpriteFlipMask
    lda #$22
    jsr VecProbeTerrain
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:TileHit,y
    ora a:TileHit+1,y
    and #$02
    beq CheckGroundAhead093D_at1
CheckGroundAhead093D_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts
CheckGroundAhead093D_at1:
    dec a:OBJ_WORK_D,x
    bne CheckGroundAhead093D_at2
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
CheckGroundAhead093D_at2:
    rts

; @routine 093D:404 WalkFarOrRetreat093D
; @run 71:C404
; WHAT IT DOES: Walks ($C33A with Y 2), keeps the facing reversed, goes to state 2 from $6C away and checks the ground.
; HOW: DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkFarOrRetreat093D:
    ldy #$02
    jsr WalkWhileFrameChangesB
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$6C
    bcc WalkFarOrRetreat093D_at0
    lda #$02
    sta a:OBJ_STATE,x
WalkFarOrRetreat093D_at0:
    jmp CheckGroundAhead093D

; @routine 093D:420 BySubstate093DC420
; @run 71:C420
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $C432.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate093DC420Substates = $C432 ; a handler per substate
BySubstate093DC420:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate093DC420Substates,y
    sta z:zTemp00
    lda a:BySubstate093DC420Substates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093D:467 LandThenFrame39_093D
; @run 71:C467
; WHAT IT DOES: On landing at the front column: frame $39, a wait of $C, next substate.
; HOW: StopOnLandingFrontColumn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LandThenFrame39_093D:
    lda #$06
    jsr VecStopOnLandingFrontColumn
    bcc LandThenFrame39_093D_at0
    lda #$0C
    sta a:OBJ_WORK_G,x
    lda #$39
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
LandThenFrame39_093D_at0:
    rts

; @routine 093D:47C CountThenState2_093D
; @run 71:C47C
; WHAT IT DOES: Counts OBJ_SWOOP_SPEED_HI out and goes to state 2 from its start.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountThenState2_093D:
    dec a:OBJ_WORK_G,x
    bne CountThenState2_093D_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$02
    sta a:OBJ_STATE,x
CountThenState2_093D_at0:
    rts

; @routine 093D:48D BySubstate093DC48D
; @run 71:C48D
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $C49F.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate093DC48DSubstates = $C49F ; a handler per substate
BySubstate093DC48D:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate093DC48DSubstates,y
    sta z:zTemp00
    lda a:BySubstate093DC48DSubstates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093D:4D2 SlashAlongList093D
; @run 71:C4D2
; @stop C50A
; WHAT IT DOES: Steps the list at $C50C; at its end sets OBJ_RAIN_COUNT and goes to state 3. On a new frame, frame $2B sounds
; $46; moves at the frame's speed and, turned at a wall, throws ($C51F).
; HOW: MoveTurningAtWalls093D.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SlashAlongList093DSpeeds = $C50A ; VX high by OBJ_PIECE_PLACE
SlashAlongList093DList = $C50C ; a frame list
SlashAlongList093D:
    stz a:OBJ_WORK_I,x
    lda #$C5
    sta z:zTemp11
    lda #$0C
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq SlashAlongList093D_at1
    bcc SlashAlongList093D_at2
    lda a:OBJ_FRAME,x
    cmp #$2B
    bne SlashAlongList093D_at0
    lda #$46
    jsr VecSoundCommand
SlashAlongList093D_at0:
    ldy a:OBJ_WORK_E,x
    lda a:SlashAlongList093DSpeeds,y
    sta a:OBJ_VX_HI,x
    jsr MoveTurningAtWalls093D
    bcs SpawnShotFromFrame
    rts
SlashAlongList093D_at1:
    lda #$01
    sta a:OBJ_WORK_I,x
    lda #$03
    sta a:OBJ_STATE,x
SlashAlongList093D_at2:
    rts

; @routine 093D:595 LandThenWait093DC595
; @run 71:C595
; WHAT IT DOES: Steps the list at $C5BA; on landing: sound 2, a wait of $18, frame $39, next substate.
; HOW: StopOnLandingFrontColumn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LandThenWait093DC595List = $C5BA ; a frame list
LandThenWait093DC595:
    lda #$C5
    sta z:zTemp11
    lda #$BA
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$06
    jsr VecStopOnLandingFrontColumn
    bcc LandThenWait093DC595_at0
    lda #$02
    jsr VecSoundCommand
    lda #$18
    sta a:OBJ_WORK_G,x
    lda #$39
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
LandThenWait093DC595_at0:
    rts

; @routine 093D:5D4 HopAtPlayer093D
; @run 71:C5D4
; @stop C602
; WHAT IT DOES: Substate 0 hops toward the player with frame $36; later lands ($C606).
; HOW: Speeds by side.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HopAtPlayer093DSpeeds = $C602 ; VX words by side
HopAtPlayer093D:
    lda a:OBJ_SUBSTATE,x
    bne LandThenState3_093D
    jsr VecSideOfPlayer
    and #$01
    asl a
    tay
    lda a:HopAtPlayer093DSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:HopAtPlayer093DSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    stz a:OBJ_VY_LO,x
    lda #$36
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093D:606 LandThenState3_093D
; @run 71:C606
; WHAT IT DOES: On landing at the front column, state 3 from its start.
; HOW: StopOnLandingFrontColumn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LandThenState3_093D:
    lda #$06
    jsr VecStopOnLandingFrontColumn
    bcc LandThenState3_093D_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
LandThenState3_093D_at0:
    rts

; @routine 093D:7C7 ChecksThenByState093D7C7
; @run 71:C7C7
; WHAT IT DOES: Counts, faces the player in some states, runs three checks and goes to the state's handler.
; HOW: A table of handlers at $C7E8.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChecksThenByState093D7C7States = $C7E8 ; a handler per state
ChecksThenByState093D7C7:
    inc a:OBJ_WORK_D,x
    jsr FacePlayerInSomeStates093D
    jsr GrabWhenStruckOrIdle093E
    jsr State9WhenHit093E
    jsr SetHitBoxByFrame093E
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ChecksThenByState093D7C7States,y
    sta z:zTemp00
    lda a:ChecksThenByState093D7C7States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093D:7FC FacePlayerInSomeStates093D
; @run 71:C7FC
; WHAT IT DOES: Faces the player in the states the table at $C80D marks.
; HOW: SideOfPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FacePlayerInSomeStates093DFaced = $C80D ; whether each state faces the player
FacePlayerInSomeStates093D:
    ldy a:OBJ_STATE,x
    lda a:FacePlayerInSomeStates093DFaced,y
    beq FacePlayerInSomeStates093D_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FacePlayerInSomeStates093D_at0:
    rts

; @routine 093D:1F5 CountdownNextState093D
; @run 71:C1F5
; WHAT IT DOES: Counts OBJ_PIECE_NEXT out and goes on.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountdownNextState093D:
    dec a:OBJ_WORK_D,x
    bne CountdownNextState093D_at0
    inc a:OBJ_STATE,x
CountdownNextState093D_at0:
    rts

; @routine 093D:248 ChangeToType9B_093D
; @run 71:C248
; WHAT IT DOES: Becomes kind $9B.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChangeToType9B_093D:
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts

; @routine 093E:017 GrabWhenStruckOrIdle093E
; @run 71:C817
; WHAT IT DOES: With five hits, or in a state the table at $C842 marks on the first screen: once an object to grab is near
; ($C84C), goes to state 8.
; HOW: FindNearby093E.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
GrabWhenStruckOrIdle093EGrabs = $C842 ; whether each state grabs
GrabWhenStruckOrIdle093E:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcs GrabWhenStruckOrIdle093E_at0
    ldy a:OBJ_STATE,x
    lda a:GrabWhenStruckOrIdle093EGrabs,y
    beq GrabWhenStruckOrIdle093E_at1
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne GrabWhenStruckOrIdle093E_at1
GrabWhenStruckOrIdle093E_at0:
    jsr FindNearby093E
    bcc GrabWhenStruckOrIdle093E_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$08
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
GrabWhenStruckOrIdle093E_at1:
    rts

; @routine 093E:04C FindNearby093E
; @run 71:C84C
; WHAT IT DOES: Looks for an on-screen object in slots 1-15 within $3C across (from its hit box centre) and $30 down; keeps its
; slot in OBJ_WORK_L and returns carry set, else clear.
; HOW: A walk of the slots with absolute differences.
; WAITS: no.
; Entry: X object. Exit: C found.
FindNearby093E:
    ldy #$01
FindNearby093E_at0:
    lda a:OBJ_KIND,y
    beq FindNearby093E_at4
    lda a:OBJ_X_HI,y
    ora a:OBJ_Y_HI,y
    bne FindNearby093E_at4
    lda a:OBJ_HITBOX_DX,y
    sta z:zTemp00
    lda a:OBJ_FACING_LEFT,y
    beq FindNearby093E_at1
    lda z:zTemp00
    eor #$FF
    inc a
    sta z:zTemp00
FindNearby093E_at1:
    lda a:OBJ_X_LO,y
    clc
    adc z:zTemp00
    sec
    sbc a:OBJ_X_LO,x
    bcs FindNearby093E_at2
    eor #$FF
    inc a
FindNearby093E_at2:
    cmp #$3C
    bcs FindNearby093E_at4
    lda a:OBJ_Y_LO,y
    sec
    sbc a:OBJ_Y_LO,x
    bcs FindNearby093E_at3
    eor #$FF
    inc a
FindNearby093E_at3:
    cmp #$30
    bcs FindNearby093E_at4
    tya
    sta a:OBJ_WORK_L,x
    sec
    rts
FindNearby093E_at4:
    iny
    cpy #$10
    bcc FindNearby093E_at0
    clc
    rts

; @routine 093E:09C State9WhenHit093E
; @run 71:C89C
; WHAT IT DOES: Under state 9, a hit goes to state 9 without a hit box.
; HOW: Two tests.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
State9WhenHit093E:
    lda a:OBJ_STATE,x
    cmp #$09
    bcs State9WhenHit093E_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State9WhenHit093E_at0
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$09
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State9WhenHit093E_at0:
    rts

; @routine 093E:0BC SetHitBoxByFrame093E
; @run 71:C8BC
; WHAT IT DOES: From frame $59 the hit box the frame's row names (offsets and half size); below it no size.
; HOW: A row index table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHitBoxByFrame093ERows = $CF4A ; row by frame from $59
SetHitBoxByFrame093EBoxes = $CF61 ; four bytes by row: X offset, Y offset, half width, half height
SetHitBoxByFrame093E:
    lda a:OBJ_FRAME,x
    sec
    sbc #$59
    bcs SetHitBoxByFrame093E_at0
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_HALF_H,x
    rts
SetHitBoxByFrame093E_at0:
    tay
    lda a:SetHitBoxByFrame093ERows,y
    asl a
    asl a
    tay
    lda a:SetHitBoxByFrame093EBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:SetHitBoxByFrame093EBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:SetHitBoxByFrame093EBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:SetHitBoxByFrame093EBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts
