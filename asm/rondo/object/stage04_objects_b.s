; @routine 07DB:4A4 BySubstate07DAB4A4
; @run 70:B4A4
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $B4B6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate07DAB4A4Substates = $B4B6 ; a handler per substate
BySubstate07DAB4A4:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate07DAB4A4Substates,y
    sta z:zTemp00
    lda a:BySubstate07DAB4A4Substates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DB:4BE SpringAway07DB
; @run 70:B4BE
; WHAT IT DOES: Keeps the facing, picks a count from the distance to the player and a speed ($B4E7), turns both speeds back,
; halves them ($B553), waits $0C and goes to the next substate.
; HOW: PlayerDistanceSum, NegateVelocityX and Y.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SpringAway07DB:
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_K,x
    jsr VecPlayerDistanceSum
    and #$0F
    sta a:OBJ_RAIN_COUNT,x
    jsr PickSpeedAndTurnFromTableB
    jsr VecNegateVelocityX
    jsr VecNegateVelocityY
    jsr HalveVelocity07DB
    lda #$0C
    sta a:OBJ_PIECE_NEXT,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 07DB:553 HalveVelocity07DB
; @run 70:B553
; WHAT IT DOES: Halves both speeds, keeping their signs.
; HOW: ASL for the sign into carry, then ROR of the high and low bytes.
; WAITS: no.
; Entry: X object. Exit: C the last bit out, N and Z from VY low.
HalveVelocity07DB:
    lda a:OBJ_VX_HI,x
    asl a
    ror a:OBJ_VX_HI,x
    ror a:OBJ_VX_LO,x
    lda a:OBJ_VY_HI,x
    asl a
    ror a:OBJ_VY_HI,x
    ror a:OBJ_VY_LO,x
    rts

; @routine 07DB:592 ChaseAndRise07DB
; @run 70:B592
; WHAT IT DOES: Moves, accelerates toward the player (clamped to 2), and rises faster. While moving down, steps the list and
; shows frame $4A when moving toward the player; once moving up, throws upward and sounds, and at the height in
; OBJ_WORK_J goes to the next substate.
; HOW: Calls and 16-bit adds.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChaseAndRise07DAPulls = $B5F9 ; X acceleration words by side
ChaseAndRise07DB:
    jsr VecApplyVelocity
    jsr VecSideOfPlayer
    and #$01
    sta z:zTemp00
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:ChaseAndRise07DAPulls,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:ChaseAndRise07DAPulls+1,y
    sta a:OBJ_VX_HI,x
    jsr ClampSpeedXToTwo
    lda a:OBJ_VY_LO,x
    sec
    sbc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    bmi ChaseAndRise07DB_at0
    ldy #$04
    jsr StepFramesFacingPlayer
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor z:zTemp00
    beq ChaseAndRise07DB_at1
    lda #$4A
    sta a:OBJ_FRAME,x
    rts
ChaseAndRise07DB_at0:
    jsr ThrowUpward
    ldy #$04
    jsr StepFramesFacingPlayer
    jsr SoundEveryFourthFrameB
    lda a:OBJ_Y_HI,x
    bne ChaseAndRise07DB_at1
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_WORK_J,x
    bcs ChaseAndRise07DB_at1
    inc a:OBJ_SUBSTATE,x
ChaseAndRise07DB_at1:
    rts

; @routine 07DB:63F HoverNearTop07DB
; @run 70:B63F
; WHAT IT DOES: Steps the list with its sound and moves. On the first screen: high up, goes to state 5; otherwise every other
; count halves the speeds, and once they have died away (0 or -1) goes to state 5.
; HOW: HalveVelocity07DB.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HoverNearTop07DB:
    ldy #$04
    jsr StepFramesFacingPlayer
    jsr SoundEveryFourthFrameB
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne HoverNearTop07DB_at1
    lda a:OBJ_Y_LO,x
    cmp #$10
    bcc HoverNearTop07DB_at0
    lda a:OBJ_FALL_LO,x
    and #$01
    bne HoverNearTop07DB_at1
    jsr HalveVelocity07DB
    lda a:OBJ_VY_HI,x
    ora a:OBJ_VY_LO,x
    beq HoverNearTop07DB_at0
    lda a:OBJ_VY_HI,x
    and a:OBJ_VY_LO,x
    cmp #$FF
    bne HoverNearTop07DB_at1
HoverNearTop07DB_at0:
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HoverNearTop07DB_at1:
    rts

; @routine 07DB:6FC CountThenByState07DB6FC
; @run 70:B6FC
; WHAT IT DOES: Counts down to state 4 ($B71B) and goes to the state's handler.
; HOW: A table of handlers at $B711.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountThenByState07DB6FCStates = $B711 ; a handler per state
CountThenByState07DB6FC:
    jsr CountdownThenState4
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CountThenByState07DB6FCStates,y
    sta z:zTemp00
    lda a:CountThenByState07DB6FCStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DB:78E FlagThenByState07DB78E
; @run 70:B78E
; WHAT IT DOES: Enters state 6 if flagged ($B7B1) and goes to the state's handler.
; HOW: A table of handlers at $B7A3.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlagThenByState07DB78EStates = $B7A3 ; a handler per state
FlagThenByState07DB78E:
    jsr EnterState6IfFlagged
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlagThenByState07DB78EStates,y
    sta z:zTemp00
    lda a:FlagThenByState07DB78EStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DB:7C7 LandUnlessSlot54_07DB
; @run 70:B7C7
; WHAT IT DOES: Unless the object in slot $54 shows frame 4, goes on when it lands.
; HOW: StopOnLanding with height 4.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LandUnlessSlot54_07DB:
    lda a:OBJ_FRAME+84
    cmp #$04
    beq LandUnlessSlot54_07DB_at0
    lda #$04
    jsr VecStopOnLanding
    bcc LandUnlessSlot54_07DB_at0
    inc a:OBJ_STATE,x
LandUnlessSlot54_07DB_at0:
    rts

; @routine 07DB:7D9 WalkNearOrAttack07DB
; @run 70:B7D9
; WHAT IT DOES: Walks ($B7F1), keeps the facing in OBJ_PIECE_PLACE and, within $4C, goes to state 3; then checks the ground
; ($B878).
; HOW: DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkNearOrAttack07DB:
    jsr WalkWhileFrameChanges
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_PIECE_PLACE,x
    jsr VecDistanceXToPlayer
    cmp #$4C
    bcs WalkNearOrAttack07DB_at0
    lda #$03
    sta a:OBJ_STATE,x
WalkNearOrAttack07DB_at0:
    jmp CheckGroundAhead07DB

; @routine 07DC:078 CheckGroundAhead07DB
; @run 70:B878
; WHAT IT DOES: Without solid ground ahead and below, goes to state 5; otherwise counts OBJ_WORK_D down to state 4.
; HOW: ProbeTerrain request 9 with SpriteFlipMask 4; the three TileHit answers.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CheckGroundAhead07DB:
    lda #$04
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    lda a:TileHit
    and a:TileHit+1
    and a:TileHit+2
    and #$01
    bne CountdownStopThenState4
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    lda #$05
    sta a:OBJ_STATE,x
    rts

; @routine 07DC:0AE WalkFarOrRetreat07DB
; @run 70:B8AE
; WHAT IT DOES: Walks, keeps the facing reversed in OBJ_PIECE_PLACE and, from $6C away, goes to state 2; then checks the ground.
; HOW: DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkFarOrRetreat07DB:
    jsr WalkWhileFrameChanges
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_PIECE_PLACE,x
    jsr VecDistanceXToPlayer
    cmp #$6C
    bcc WalkFarOrRetreat07DB_at0
    lda #$02
    sta a:OBJ_STATE,x
WalkFarOrRetreat07DB_at0:
    jmp CheckGroundAhead07DB

; @routine 07DC:10B BySubstate07DAB90B
; @run 70:B90B
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $B91D.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate07DAB90BSubstates = $B91D ; a handler per substate
BySubstate07DAB90B:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate07DAB90BSubstates,y
    sta z:zTemp00
    lda a:BySubstate07DAB90BSubstates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DC:123 StartHopAtListEnd07DB
; @run 70:B923
; WHAT IT DOES: Steps the list ($B817); at its end jumps the way OBJ_PIECE_PLACE says, rising at -3.
; HOW: A table of speeds by direction.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartHopAtListEnd07DASpeeds = $B948 ; VX high by direction
StartHopAtListEnd07DB:
    ldy #$04
    jsr StepFramesFacingPlayerAlways
    bne StartHopAtListEnd07DB_at0
    ldy a:OBJ_PIECE_PLACE,x
    lda a:StartHopAtListEnd07DASpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
StartHopAtListEnd07DB_at0:
    rts

; @routine 07DC:14A NextSubstateOnLanding07DB
; @run 70:B94A
; WHAT IT DOES: Goes to the next substate on landing.
; HOW: StopOnLanding with height 4.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NextSubstateOnLanding07DB:
    lda #$04
    jsr VecStopOnLanding
    bcc NextSubstateOnLanding07DB_at0
    inc a:OBJ_SUBSTATE,x
NextSubstateOnLanding07DB_at0:
    rts

; @routine 07DC:155 State3AtListEnd07DB
; @run 70:B955
; WHAT IT DOES: Steps the list ($B817); at its end goes to state 3.
; HOW: StepFramesFacingPlayerAlways with list 6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
State3AtListEnd07DB:
    ldy #$06
    jsr StepFramesFacingPlayerAlways
    bne State3AtListEnd07DB_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
State3AtListEnd07DB_at0:
    rts

; @routine 07DC:291 ThrowFromHandAndMove07DB
; @run 70:BA91
; WHAT IT DOES: Counts, picks a speed at random toward the player at state starts ($BAE3), handles a hit ($BAFA), sets the hit
; box ($BB32) and goes to the state's handler ($BB44). While the slot below holds kind $27 in a state under 2,
; places it at the hand with the speed the frame picks, mirrored by facing, and moves it.
; HOW: CopyObjectPlace to the slot below.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowFromHandAndMove07DASpeeds = $BFCC ; VX high and VY high by frame from 1
ThrowFromHandAndMove07DB:
    inc a:OBJ_SWOOP_SPEED_HI,x
    jsr FacePlayerInSomeStates07DB
    jsr DropThrownAndRecoil07DB
    jsr SetHitBoxByFrame12_07DB
    jsr ByState07DABB44
    lda a:OBJ_KIND-1,x
    cmp #$27
    bne ThrowFromHandAndMove07DB_at1
    lda a:OBJ_STATE-1,x
    cmp #$02
    bcs ThrowFromHandAndMove07DB_at1
    txa
    tay
    dey
    jsr VecCopyObjectPlace
    stx z:zClimbStepBefore
    lda a:OBJ_FRAME,x
    sec
    sbc #$01
    asl a
    tax
    lda a:ThrowFromHandAndMove07DASpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:ThrowFromHandAndMove07DASpeeds+1,x
    sta a:OBJ_VY_HI,y
    ldx z:zClimbStepBefore
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT-1,x
    beq ThrowFromHandAndMove07DB_at0
    lda a:OBJ_VX_HI,y
    eor #$FF
    inc a
    sta a:OBJ_VX_HI,y
ThrowFromHandAndMove07DB_at0:
    sxy
    jsr VecApplyVelocity
    sxy
ThrowFromHandAndMove07DB_at1:
    rts

; @routine 07DC:2E3 FacePlayerInSomeStates07DB
; @run 70:BAE3
; WHAT IT DOES: Faces the player in the states the table at $BAF4 marks.
; HOW: SideOfPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FacePlayerInSomeStates07DAFaced = $BAF4 ; whether each state faces the player
FacePlayerInSomeStates07DB:
    ldy a:OBJ_STATE,x
    lda a:FacePlayerInSomeStates07DAFaced,y
    beq FacePlayerInSomeStates07DB_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FacePlayerInSomeStates07DB_at0:
    rts

; @routine 07DC:2FA DropThrownAndRecoil07DB
; @run 70:BAFA
; WHAT IT DOES: A hit outside state 5 drops the object below and sends this one up and back for $40 frames in state 5.
; HOW: ClearObjectInY and stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DropThrownAndRecoil07DB:
    lda a:OBJ_STATE,x
    cmp #$05
    beq DropThrownAndRecoil07DB_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq DropThrownAndRecoil07DB_at0
    stz a:OBJ_HITBOX_ON,x
    txa
    tay
    dey
    jsr VecClearObjectInY
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    lda #$40
    sta a:OBJ_PIECE_NEXT,x
    lda #$05
    sta a:OBJ_STATE,x
DropThrownAndRecoil07DB_at0:
    rts

; @routine 07DC:332 SetHitBoxByFrame12_07DB
; @run 70:BB32
; WHAT IT DOES: Half width 8, or $13 at frame $12.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHitBoxByFrame12_07DB:
    lda #$08
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:OBJ_FRAME,x
    cmp #$12
    bne SetHitBoxByFrame12_07DB_at0
    lda #$13
    sta a:OBJ_HITBOX_HALF_W,x
SetHitBoxByFrame12_07DB_at0:
    rts

; @routine 07DC:344 ByState07DABB44
; @run 70:BB44
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $BB56.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DABB44States = $BB56 ; a handler per state
ByState07DABB44:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DABB44States,y
    sta z:zTemp00
    lda a:ByState07DABB44States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DC:362 LoadThrowerWithObject07DB
; @run 70:BB62
; WHAT IT DOES: Loads the record facing the player with a wait of $40 and starts the kind $27 object it holds in the slot below;
; number 2 draws behind (attribute $87).
; HOW: ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadThrowerWithObject07DB:
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$40
    sta a:OBJ_PIECE_NEXT,x
    txa
    tay
    dey
    jsr VecClearObjectInY
    lda #$27
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    cmp #$02
    bne LoadThrowerWithObject07DB_at0
    lda #$87
    sta a:OBJ_SPRITE_ATTR,x
LoadThrowerWithObject07DB_at0:
    rts

; @routine 07DC:38A NextStateOnLanding07DC38A
; @run 70:BB8A
; WHAT IT DOES: Goes to the next state on landing, with height 6.
; HOW: StopOnLanding.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NextStateOnLanding07DC38A:
    lda #$06
    jsr VecStopOnLanding
    bcc NextStateOnLanding07DC38A_at0
    inc a:OBJ_STATE,x
NextStateOnLanding07DC38A_at0:
    rts

; @routine 07DC:395 WalkUntilNear07DB
; @run 70:BB95
; WHAT IT DOES: Walks ($BBB0) the way it faces; unless an attack starts ($BBE5), goes to state 3 within $4E.
; HOW: DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkUntilNear07DB:
    jsr StepAndWalk07DB
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_SWOOP_SPEED_LO,x
    jsr AttackWhenWaitOver07DB
    bcs WalkUntilNear07DB_at0
    jsr VecDistanceXToPlayer
    cmp #$4E
    bcs WalkUntilNear07DB_at0
    lda #$03
    sta a:OBJ_STATE,x
WalkUntilNear07DB_at0:
    rts

; @routine 07DC:3B0 StepAndWalk07DB
; @run 70:BBB0
; WHAT IT DOES: Steps the list at $BBD8 and walks the way OBJ_WORK_F says, bouncing off walls.
; HOW: StepObjectFrames, then BounceOffWallsHigh07DA with SpriteFlipMask 6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepAndWalk07DASpeeds = $BBD4 ; VX words by direction
StepAndWalk07DAList = $BBD8 ; a frame list
StepAndWalk07DB:
    lda #$BB
    sta z:zTemp11
    lda #$D8
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_SWOOP_SPEED_LO,x
    asl a
    tay
    lda a:StepAndWalk07DASpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:StepAndWalk07DASpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$06
    sta z:SpriteFlipMask
    jsr BounceOffWallsHigh07DA
    rts

; @routine 07DC:3E5 AttackWhenWaitOver07DB
; @run 70:BBE5
; WHAT IT DOES: While OBJ_WORK_D counts, carry clear. At zero picks the next wait from the table by number and OBJ_RAIN_COUNT,
; starts state 4 and returns carry set.
; HOW: Eight waits per number.
; WAITS: no.
; Entry: X object. Exit: C whether an attack starts.
AttackWhenWaitOver07DAWaits = $BC16 ; eight waits per number
AttackWhenWaitOver07DB:
    lda a:OBJ_PIECE_NEXT,x
    beq AttackWhenWaitOver07DB_at0
    dec a:OBJ_PIECE_NEXT,x
    clc
    rts
AttackWhenWaitOver07DB_at0:
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_RAIN_COUNT,x
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:AttackWhenWaitOver07DAWaits,y
    sta a:OBJ_PIECE_NEXT,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    sec
    rts

; @routine 07DC:42E WalkAwayUntilFar07DB
; @run 70:BC2E
; WHAT IT DOES: Walks away from the way it faces; unless an attack starts, goes to state 2 from $64 away.
; HOW: DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkAwayUntilFar07DB:
    jsr StepAndWalk07DB
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_SWOOP_SPEED_LO,x
    jsr AttackWhenWaitOver07DB
    bcs WalkAwayUntilFar07DB_at0
    jsr VecDistanceXToPlayer
    cmp #$64
    bcc WalkAwayUntilFar07DB_at0
    lda #$02
    sta a:OBJ_STATE,x
WalkAwayUntilFar07DB_at0:
    rts

; @routine 07DC:44B BySubstate07DABC4B
; @run 70:BC4B
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $BC5D.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate07DABC4BSubstates = $BC5D ; a handler per substate
BySubstate07DABC4B:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate07DABC4BSubstates,y
    sta z:zTemp00
    lda a:BySubstate07DABC4BSubstates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DC:467 ChooseNextMove07DB
; @run 70:BC67
; WHAT IT DOES: Off the first screen, state 3. With the player within $18 in height, takes the substate from the table by number
; and count; further, every fourth count goes to substate 3, else state 3.
; HOW: DistanceYToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChooseNextMove07DAMoves = $BCA5 ; sixteen substates per number
ChooseNextMove07DB:
    lda a:OBJ_X_HI,x
    bne ChooseNextMove07DB_at0
    jsr VecDistanceYToPlayer
    cmp #$18
    bcc ChooseNextMove07DB_at2
    inc a:OBJ_RAIN_COUNT,x
    lda a:OBJ_RAIN_COUNT,x
    and #$03
    beq ChooseNextMove07DB_at1
ChooseNextMove07DB_at0:
    lda #$03
    sta a:OBJ_STATE,x
    rts
ChooseNextMove07DB_at1:
    lda #$03
    sta a:OBJ_SUBSTATE,x
    rts
ChooseNextMove07DB_at2:
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_RAIN_COUNT,x
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:ChooseNextMove07DAMoves,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_RAIN_COUNT,x
    rts

; @routine 07DC:4D5 ThrowAlongList07DB4D5
; @run 70:BCD5
; WHAT IT DOES: Steps the list at $BCF7: at its end starts state 3 ($BCE8); on a new frame throws ($BD04).
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowAlongList07DB4D5List = $BCF7 ; a frame list
ThrowAlongList07DB4D5:
    lda #$F7
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq RestartInState3_07DB
    bcc ThrowAlongList07DB4D5_at0
    jmp ThrowAtFrameFive07DB
ThrowAlongList07DB4D5_at0:
    rts

; @routine 07DC:4E8 RestartInState3_07DB
; @run 70:BCE8
; WHAT IT DOES: State 3 from its start.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RestartInState3_07DB:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 07DC:504 ThrowAtFrameFive07DB
; @run 70:BD04
; WHAT IT DOES: At animation step 5, sound 6 and a kind $26 object with this facing, its OBJ_PIECE_PLACE the substate less one.
; HOW: FindFreeSlotFrom30 and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowAtFrameFive07DB:
    lda a:OBJ_ANIM_STEP,x
    cmp #$05
    bne ThrowAtFrameFive07DB_at0
    lda #$06
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom30
    bcc ThrowAtFrameFive07DB_at0
    lda #$26
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda a:OBJ_SUBSTATE,x
    dec a
    sta a:OBJ_PIECE_PLACE,y
ThrowAtFrameFive07DB_at0:
    rts

; @routine 07DC:528 ThrowAlongList07DB528
; @run 70:BD28
; WHAT IT DOES: As ThrowAlongList07DB4D5 with the list at $BD3B.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowAlongList07DB528List = $BD3B ; a frame list
ThrowAlongList07DB528:
    lda #$3B
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq RestartInState3_07DB
    bcc ThrowAlongList07DB528_at0
    jmp ThrowAtFrameFive07DB
ThrowAlongList07DB528_at0:
    rts

; @routine 07DC:546 ThrowAlongList07DB546
; @run 70:BD46
; WHAT IT DOES: As ThrowAlongList07DB4D5 with the list at $BD59.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowAlongList07DB546List = $BD59 ; a frame list
ThrowAlongList07DB546:
    lda #$59
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq RestartInState3_07DB
    bcc ThrowAlongList07DB546_at0
    jmp ThrowAtFrameFive07DB
ThrowAlongList07DB546_at0:
    rts

; @routine 07DC:568 DashAlongList07DB
; @run 70:BD68
; WHAT IT DOES: Steps the list at $BD98 (state 3 at its end), moving the way it faces and, from step 2, bouncing off walls.
; HOW: StepObjectFrames, BounceOffWallsHigh07DA with SpriteFlipMask 6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DashAlongList07DAList = $BD98 ; a frame list
DashAlongList07DASpeeds = $BDB1 ; VX words by facing
DashAlongList07DB:
    lda #$98
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne DashAlongList07DB_at0
    jmp RestartInState3_07DB
DashAlongList07DB_at0:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:DashAlongList07DASpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:DashAlongList07DASpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_ANIM_STEP,x
    cmp #$02
    bcc DashAlongList07DB_at1
    lda #$06
    sta z:SpriteFlipMask
    jsr BounceOffWallsHigh07DA
DashAlongList07DB_at1:
    rts

; @routine 07DC:5B5 BurningDebris07DB
; @run 70:BDB5
; WHAT IT DOES: Steps the list at $BE03, counts out and clears; drips ($BE0C); on landing at frame $1B bounces by the table,
; smaller each time, six times.
; HOW: StepObjectFrames, StopOnLanding with height 6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BurningDebris07DABounces = $BDF5 ; VY words by bounce
BurningDebris07DAList = $BE03 ; a frame list
BurningDebris07DB:
    lda #$BE
    sta z:zTemp11
    lda #$03
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PIECE_NEXT,x
    bne BurningDebris07DB_at0
    jmp VecClearObject
BurningDebris07DB_at0:
    jsr SparkByCount07DB
    lda #$06
    jsr VecStopOnLanding
    bcc BurningDebris07DB_at1
    lda a:OBJ_FRAME,x
    cmp #$1B
    bne BurningDebris07DB_at1
    lda a:OBJ_FALL_LO,x
    asl a
    tay
    lda a:BurningDebris07DABounces+1,y
    sta a:OBJ_VY_HI,x
    lda a:BurningDebris07DABounces,y
    sta a:OBJ_VY_LO,x
    lda a:OBJ_FALL_LO,x
    cmp #$06
    beq BurningDebris07DB_at1
    inc a:OBJ_FALL_LO,x
BurningDebris07DB_at1:
    rts

; @routine 07DC:60C SparkByCount07DB
; @run 70:BE0C
; WHAT IT DOES: When OBJ_WORK_D matches one of eight values at $BE46, starts a kind $9C spark with that row's speed, sound $15,
; and moves it once.
; HOW: As DripByWaveCounter051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SparkByCount07DATimes = $BE46 ; eight counts
SparkByCount07DASpeeds = $BE4E ; VX high and VY high by row
SparkByCount07DB:
    cly
SparkByCount07DB_at0:
    lda a:OBJ_PIECE_NEXT,x
    cmp a:SparkByCount07DATimes,y
    beq SparkByCount07DB_at1
    iny
    cpy #$08
    bcc SparkByCount07DB_at0
    rts
SparkByCount07DB_at1:
    stx z:zClimbStepBefore
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc SparkByCount07DB_at2
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$15
    jsr VecSoundIfOnFirstScreen
    lda z:zBurstCount
    asl a
    tax
    lda a:SparkByCount07DASpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:SparkByCount07DASpeeds+1,x
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zClimbStepBefore
SparkByCount07DB_at2:
    rts
