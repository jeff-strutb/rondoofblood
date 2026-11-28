; @routine 0A9A:33A BounceAtWalls
; @run 70:AB3A
; WHAT IT DOES: Moves the object along X and asks the floor (request 9); when the floor under the side it is moving to has no wall bit, or the wall
; ahead (request 2) has none, it goes on; otherwise it turns round (NegateVelocityX) and moves once more.
; HOW: ApplyVelocityX, ProbeTerrain twice, NegateVelocityX; the answer is C: set when it turned.
; WAITS: no.
; Entry: X object, Y the probe's column. Exit: C set when the object turned round; A and the flags as the last instructions left them.
BounceAtWalls:
    jsr VecApplyVelocityX
    lda #$09
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    tay
    lda a:TileHit+1,y
    and #$01
    beq BounceAtWalls_at0
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
    bne BounceAtWalls_at0
    clc
    rts
BounceAtWalls_at0:
    jsr VecNegateVelocityX
    jsr VecApplyVelocityX
    sec
    rts

; @routine 0A9C:7F2 FaceBreakAndRun
; @run 70:BFF2
; WHAT IT DOES: One frame of the cave object: its life timer goes up, it faces the player in some states (FaceThePlayerInSomeStatesD), may break (BreakWhenHit) and then the code its state names runs.
; HOW: INC, two calls, a tail jump through the state table (JumpByStateTableC).
; WAITS: no.
; Entry: X object. Exit: as the code jumped to.
FaceBreakAndRun:
    inc a:OBJ_WORK_D,x
    jsr FaceThePlayerInSomeStatesD
    jsr BreakWhenHit
    jmp JumpByStateTableC

; @routine 0A9D:01F BreakWhenHit
; @run 71:C01F
; WHAT IT DOES: An object in a state below $0B whose status bit 7 is set clears the object after it, stops its hitbox and animation,
; sets OBJ_WORK_L to $40 and enters state $0B.
; HOW: Reads the state and the status flags; ClearObject on slot plus one.
; WAITS: no.
; Entry: X object. Exit: A $0B, N and Z from it, C 0.
BreakWhenHit:
    lda a:OBJ_STATE,x
    cmp #$0B
    bcs BreakWhenHit_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq BreakWhenHit_at0
    inx
    jsr VecClearObject
    dex
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$40
    sta a:OBJ_WORK_L,x
    lda #$0B
    sta a:OBJ_STATE,x
BreakWhenHit_at0:
    rts

; @routine 0A9D:046 JumpByStateTableC
; @run 71:C046
; WHAT IT DOES: Goes to the code a table at $C058 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTableC:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$C058,y
    sta z:zTemp00
    lda a:$C059,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9D:070 StartWithPartner
; @run 71:C070
; WHAT IT DOES: Loads the object's record, sets OBJ_WORK_E to $B0 and its angle byte to $40, and starts a kind $25 object at its place in the slot
; after it.
; HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A the last byte copied, Y the slot plus one, C from the record.
StartWithPartner:
    jsr VecLoadObjectRecord
    lda #$B0
    sta a:OBJ_WORK_E,x
    lda #$40
    sta a:OBJ_WORK_F,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$25
    jmp VecStartObjectAtPlace

; @routine 0A9D:088 StopWhenLanded
; @run 71:C088
; WHAT IT DOES: Unless slot $54's frame is 4, asks StopOnLanding with a probe height of 4; on landing the object's animation is reset and it enters
; its next state.
; HOW: StopOnLanding, two stores, an INC.
; Untraced: one byte.
; WAITS: no.
; Entry: X object, Y the probe's column, C passed on. Exit: the mover's registers, then N and Z of the new state when landed.
StopWhenLanded:
    lda a:$2ABC
    cmp #$04
    beq StopWhenLanded_at0
    lda #$04
    jsr VecStopOnLanding
    bcc StopWhenLanded_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
StopWhenLanded_at0:
    rts

; @routine 0A9D:0A0 WalkAndAttackA
; @run 71:C0A0
; WHAT IT DOES: WalkByPhase 0 and ChooseAttackByDistance; the object faces its walking direction, counts OBJ_WORK_E down and at zero resets it to $B0
; and turns the direction round.
; HOW: attack_walk with phase 0.
; WAITS: no.
; Entry: X object, Y the probe's column. Exit: see attack_walk_exit_shim.
WalkAndAttackA:
    cla
    jsr WalkByPhase
    jsr ChooseAttackByDistance
    lda a:OBJ_WORK_H,x
    sta a:OBJ_FACING_LEFT,x
    dec a:OBJ_WORK_E,x
    bne WalkAndAttackA_at0
    lda #$B0
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_H,x
    eor #$01
    sta a:OBJ_WORK_H,x
WalkAndAttackA_at0:
    rts

; @routine 0A9D:0C0 WalkByPhase
; @run 71:C0C0
; WHAT IT DOES: A walking step for the phase in A (0 to 3): the phase times four picks an animation list pointer and a speed word from the table at $C0ED;
; the list is stepped; when a new frame comes the object slows toward the player (SlowToPlayer), turns its speed round when its direction byte OBJ_WORK_H says,
; and moves with BounceWithHeight4.
; HOW: StepObjectFrames, SlowToPlayer, NegateVelocityX, BounceWithHeight4.
; WAITS: no.
; Entry: X object, A the phase, Y the probe's column. Exit: the step's flags when no new frame; else as BounceAtWalls.
WalkByPhase:
    asl a
    asl a
    tay
    lda a:$C0ED,y
    sta z:zTemp10
    lda a:$C0EE,y
    sta z:zTemp11
    lda a:$C0EF,y
    sta a:OBJ_VX_LO,x
    lda a:$C0F0,y
    sta a:OBJ_VX_HI,x
    jsr VecStepObjectFrames
    bcc WalkByPhase_at0
    jsr SlowToPlayer
    lda a:OBJ_WORK_H,x
    beq WalkByPhase_at1
    jsr VecNegateVelocityX
WalkByPhase_at1:
    jsr BounceWithHeight4
WalkByPhase_at0:
    rts

; @routine 0A9D:113 SlowToPlayer
; @run 71:C113
; WHAT IT DOES: When the player is within 5 along X, the object's X speed becomes that distance and its direction byte OBJ_WORK_H the
; side the player is on (inverted).
; HOW: DistanceXToPlayer, SideOfPlayer.
; WAITS: no.
; Entry: X object. Exit: the compare with 5, or A the flipped side bit.
SlowToPlayer:
    jsr VecDistanceXToPlayer
    cmp #$05
    bcs SlowToPlayer_at0
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    jsr VecSideOfPlayer
    and #$01
    eor #$01
    sta a:OBJ_WORK_H,x
SlowToPlayer_at0:
    rts

; @routine 0A9D:12B BounceWithHeight4
; @run 71:C12B
; WHAT IT DOES: BounceAtWalls with a probe height of 4.
; HOW: A store into zProduct1, then the call.
; WAITS: no.
; Entry: X object, Y the probe's column. Exit: as BounceAtWalls.
BounceWithHeight4:
    lda #$04
    sta z:SpriteFlipMask
    jsr BounceAtWalls
    rts

; @routine 0A9D:133 ChooseAttackByDistance
; @run 71:C133
; WHAT IT DOES: When the player is within $20 vertically the object resets its animation, sets OBJ_WORK_K 1 and enters state 4; else when within
; $18 along X it resets, sets OBJ_WORK_K 2 and enters state 7.
; HOW: DistanceYToPlayer, DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: the last compare or load.
ChooseAttackByDistance:
    jsr VecDistanceYToPlayer
    cmp #$20
    bcs ChooseAttackByDistance_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$01
    sta a:OBJ_WORK_K,x
    lda #$04
    sta a:OBJ_STATE,x
    rts
ChooseAttackByDistance_at0:
    jsr VecDistanceXToPlayer
    cmp #$18
    bcs ChooseAttackByDistance_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$02
    sta a:OBJ_WORK_K,x
    lda #$07
    sta a:OBJ_STATE,x
ChooseAttackByDistance_at1:
    rts

; @routine 0A9D:163 WalkAndAttackB
; @run 71:C163
; WHAT IT DOES: As WalkAndAttackA with phase 3.
; HOW: attack_walk with phase 3.
; WAITS: no.
; Entry and exit: as WalkAndAttackA.
WalkAndAttackB:
    lda #$03
    jsr WalkByPhase
    jsr ChooseAttackByDistance
    lda a:OBJ_WORK_H,x
    sta a:OBJ_FACING_LEFT,x
    dec a:OBJ_WORK_E,x
    bne WalkAndAttackB_at0
    lda #$B0
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_H,x
    eor #$01
    sta a:OBJ_WORK_H,x
WalkAndAttackB_at0:
    rts

; @routine 0A9D:184 ChasePlayerA
; @run 71:C184
; WHAT IT DOES: Enters state 5 when the player is within $64 along X, walks (phase 1), takes its facing as its direction, chooses its next move, and goes to state 3
; when the player is $40 or more away vertically.
; HOW: DistanceXToPlayer, WalkByPhase, ChooseNextMove, DistanceYToPlayer.
; WAITS: no.
; Entry: X object, Y the probe's column, C passed on. Exit: see chase_exit_shim.
ChasePlayerA:
    jsr VecDistanceXToPlayer
    cmp #$64
    bcs ChasePlayerA_at0
    lda #$05
    sta a:OBJ_STATE,x
ChasePlayerA_at0:
    lda #$01
    jsr WalkByPhase
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_H,x
    jsr ChooseNextMove
    jsr VecDistanceYToPlayer
    cmp #$40
    bcc ChasePlayerA_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
ChasePlayerA_at1:
    rts

; @routine 0A9D:1B1 ChasePlayerB
; @run 71:C1B1
; WHAT IT DOES: As ChasePlayerA, but it enters state 4 when the player is $78 or more away along X and its direction is its facing turned round.
; HOW: As ChasePlayerA.
; WAITS: no.
; Entry and exit: as ChasePlayerA.
ChasePlayerB:
    jsr VecDistanceXToPlayer
    cmp #$78
    bcc ChasePlayerB_at0
    lda #$04
    sta a:OBJ_STATE,x
ChasePlayerB_at0:
    lda #$01
    jsr WalkByPhase
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_H,x
    jsr ChooseNextMove
    bra $C19E

; @routine 0A9D:1CF ChooseNextMove
; @run 71:C1CF
; WHAT IT DOES: While the object in slot 0 is in OBJ_SUBSTATE state 1 and the player is $50 or more away along X and the distance sum is even, the
; object resets its animation and enters state 9; otherwise its wait OBJ_WORK_F counts down and at zero it resets, picks a new wait from the table
; at $C226 by the distance sum and its next state from the table at $C223 by a counter OBJ_WORK_M, which goes up one.
; HOW: DistanceXToPlayer, PlayerDistanceSum (twice, with the carry the compares left), table reads (bank 71).
; WAITS: no.
; Entry: X object, C entry (it reaches PlayerDistanceSum when a compare did not set it). Exit: the last compare, AND, DEC or INC.
ChooseNextMove:
    lda a:OBJ_SUBSTATE
    cmp #$01
    bne ChooseNextMove_at0
    jsr VecDistanceXToPlayer
    cmp #$50
    bcc ChooseNextMove_at0
    jsr VecPlayerDistanceSum
    and #$01
    bne ChooseNextMove_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$09
    sta a:OBJ_STATE,x
    rts
ChooseNextMove_at0:
    dec a:OBJ_WORK_F,x
    bne ChooseNextMove_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    jsr VecPlayerDistanceSum
    and #$03
    tay
    lda a:$C226,y
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_M,x
    and #$07
    tay
    lda a:$C21B,y
    tay
    lda a:$C223,y
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_M,x
ChooseNextMove_at1:
    rts

; @routine 0A9D:22A StepAndLunge
; @run 71:C22A
; WHAT IT DOES: Steps the animation list at $C260; when it ends the object resets its animation and enters state 4; on the new frame 7 it plays sound $1F
; (on the first screen), takes an X speed from a table at $C25E by its facing and moves with BounceWithHeight4.
; HOW: StepObjectFrames, SoundIfOnFirstScreen, BounceWithHeight4.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
StepAndLunge:
    lda #$C2
    sta z:zTemp11
    lda #$60
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq StepAndLunge_at0
    bcc StepAndLunge_at1
    lda a:OBJ_FRAME,x
    cmp #$07
    bne StepAndLunge_at1
    lda #$1F
    jsr VecSoundIfOnFirstScreen
    ldy a:OBJ_FACING_LEFT,x
    lda a:$C25E,y
    sta a:OBJ_VX_HI,x
    jsr BounceWithHeight4
StepAndLunge_at1:
    rts
StepAndLunge_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    rts

; @routine 0A9D:269 WalkThenTurn
; @run 71:C269
; WHAT IT DOES: Walks by phase 2, takes its facing as its direction, runs TurnWhenWaitEnds, and when the player is $48 or more away along X resets its
; animation and enters state 3.
; HOW: WalkByPhase, TurnWhenWaitEnds, DistanceXToPlayer.
; WAITS: no.
; Entry: X object, Y the probe's column. Exit: the walk's, then the last compare or load.
WalkThenTurn:
    lda #$02
    jsr WalkByPhase
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_H,x
    jsr TurnWhenWaitEnds
    jsr VecDistanceXToPlayer
    cmp #$48
    bcc WalkThenTurn_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
WalkThenTurn_at0:
    rts

; @routine 0A9D:28A TurnWhenWaitEnds
; @run 71:C28A
; WHAT IT DOES: Counts the angle byte down; at zero resets the animation, takes OBJ_WORK_I from the player's side (bit 1, vertical) and enters state 8.
; HOW: DEC, SideOfPlayer and a shift.
; WAITS: no.
; Entry: X object. Exit: wait running: N and Z from the count; at zero A $08, C bit 0 of the sides.
TurnWhenWaitEnds:
    dec a:OBJ_WORK_F,x
    bne TurnWhenWaitEnds_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    jsr VecSideOfPlayer
    lsr a
    sta a:OBJ_WORK_I,x
    lda #$08
    sta a:OBJ_STATE,x
TurnWhenWaitEnds_at0:
    rts

; @routine 0A9D:2A2 StepAndShout
; @run 71:C2A2
; WHAT IT DOES: Steps the animation list a table at $C2E4 names for OBJ_WORK_I; when the list ends it resets, enters state 7 and picks a wait
; from the table at $C2E0 by the distance sum; on a new frame $26 or $2D it plays sound $1F if the object is on the first screen.
; HOW: StepObjectFrames, PlayerDistanceSum, a tail jump into SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
StepAndShout:
    lda a:OBJ_WORK_I,x
    asl a
    tay
    lda a:$C2E4,y
    sta z:zTemp10
    lda a:$C2E5,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepAndShout_at0
    bcc StepAndShout_at1
    lda a:OBJ_FRAME,x
    cmp #$26
    beq StepAndShout_at2
    cmp #$2D
    bne StepAndShout_at1
StepAndShout_at2:
    lda #$1F
    jmp VecSoundIfOnFirstScreen
StepAndShout_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$07
    sta a:OBJ_STATE,x
    jsr VecPlayerDistanceSum
    and #$03
    tay
    lda a:$C2E0,y
    sta a:OBJ_WORK_F,x
StepAndShout_at1:
    rts

; @routine 0A9D:31E StepWithArmor
; @run 71:C31E
; WHAT IT DOES: Sets bits 0 and 1 of the object's OBJ_PACE while it steps the animation list at $C347; when the list ends it clears them, resets
; the animation and enters state 5.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the step; at the end A 5.
StepWithArmor:
    lda a:OBJ_PACE,x
    ora #$03
    sta a:OBJ_PACE,x
    lda #$C3
    sta z:zTemp11
    lda #$47
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne StepWithArmor_at0
    lda a:OBJ_PACE,x
    and #$FC
    sta a:OBJ_PACE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
StepWithArmor_at0:
    rts

; @routine 0A9D:366 StepAndPounce
; @run 71:C366
; WHAT IT DOES: Sets OBJ_3359 3 and steps the animation list at $C3C5; when it ends OBJ_3359 becomes 2, the animation resets and the object enters state 4; on
; the new frame $0E it plays sound $1A; on frames $0C and $0D it takes an X speed from a table at $C3BD by the frame and its facing and moves with
; BounceWithHeight4.
; HOW: StepObjectFrames, SoundIfOnFirstScreen, BounceWithHeight4.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
StepAndPounce:
    lda #$03
    sta a:OBJ_WORK_C+1,x
    lda #$C3
    sta z:zTemp11
    lda #$C5
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq StepAndPounce_at0
    bcc StepAndPounce_at1
    lda a:OBJ_FRAME,x
    cmp #$0E
    bne StepAndPounce_at2
    lda #$1A
    jsr VecSoundIfOnFirstScreen
StepAndPounce_at2:
    lda a:OBJ_FRAME,x
    sec
    sbc #$0C
    cmp #$02
    bcs StepAndPounce_at1
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_FACING_LEFT,x
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:$C3BE,y
    sta a:OBJ_VX_HI,x
    lda a:$C3BD,y
    sta a:OBJ_VX_LO,x
    jsr BounceWithHeight4
StepAndPounce_at1:
    rts
StepAndPounce_at0:
    lda #$02
    sta a:OBJ_WORK_C+1,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    rts

; @routine 0A9D:3DA FlashAndCount
; @run 71:C3DA
; @stop C401
; WHAT IT DOES: Flashes the object ($8A), steps the animation list at $C401, counts OBJ_WORK_L down and clears the object at zero; while it runs, when
; the count equals one of eight cue values (table at $C43B) it shoots an arrow with that cue's index.
; HOW: StepObjectFrames, DEC, a scan of the table, ShootArrow (reached by a branch).
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
FlashAndCount:
    lda #$8A
    sta a:OBJ_SPRITE_ATTR,x
    lda #$01
    sta z:zTemp10
    lda #$C4
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_L,x
    bne FlashAndCount_at0
    jmp VecClearObject
FlashAndCount_at0:
    cly
FlashAndCount_at1:
    lda a:OBJ_WORK_L,x
    cmp a:$C43B,y
    beq ShootArrow
    iny
    cpy #$08
    bcc FlashAndCount_at1
    rts

; @routine 0A9D:410 ShootArrow
; @run 71:C410
; WHAT IT DOES: Starts a kind $9C object at this object's place in the first free slot from $30, plays sound $15 if it is on the first screen,
; gives the new object an X and Y speed from a table at $C443 (two bytes by the cue's index doubled) and moves it once.
; HOW: The slot and the index are kept in zRichterObject and zHitSlot; FindFreeSlotFrom30, StartObjectAtPlace, SoundIfOnFirstScreen,
; ApplyVelocity.
; Untraced: one byte.
; WAITS: no.
; Entry: X object, Y the cue's index. Exit: see the wrapper.
ShootArrow:
    stx z:zRichterObject
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom30
    bcc LaunchHopByF3510
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$15
    jsr VecSoundIfOnFirstScreen
    lda z:zHitSlot
    asl a
    tax
    lda a:$C443,x
    sta a:OBJ_VX_HI,y
    lda a:$C444,x
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zRichterObject
    rts
; @routine 0A9D:453 FollowPartner
; @run 71:C453
; WHAT IT DOES: In state 0 loads the object's record; later, when it has taken a hit of 2 or more it plays sound $12 and sets its timer to 5; it takes the place of the
; object before it, sets its hitbox from a four-byte row of the table at $C4AA by that object's frame less 1, copies its facing and hitbox flag, and is cleared
; if the object before it is not kind $24.
; HOW: LoadObjectRecord (a tail jump), SoundIfOnFirstScreen, CopyObjectPlaceBack, table reads (bank 71), ClearObject.
; Untraced: the clear (C4A6).
; Curated cases: tools/rondo/curated/follow_partner.txt (the traced arms with the partner of kind $24).
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
FollowPartner:
    lda a:OBJ_STATE,x
    bne FollowPartner_at0
    jmp VecLoadObjectRecord
FollowPartner_at0:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$02
    bcc FollowPartner_at1
    lda #$12
    jsr VecSoundIfOnFirstScreen
    lda #$05
    sta a:OBJ_INVULN_TIMER,x
FollowPartner_at1:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:$2A67,x
    sec
    sbc #$01
    asl a
    asl a
    tay
    lda a:$C4AA,y
    sta a:OBJ_HITBOX_DX,x
    lda a:$C4AB,y
    sta a:OBJ_HITBOX_DY,x
    lda a:$C4AC,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:$C4AD,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:$2ABF,x
    sta a:OBJ_FACING_LEFT,x
    lda a:$324F,x
    sta a:OBJ_HITBOX_ON,x
    lda a:$274F,x
    cmp #$24
    beq FollowPartner_at2
    jmp VecClearObject
FollowPartner_at2:
    rts

; @routine 0A9E:4C3 HopAndSettle
; @run 71:CCC3
; WHAT IT DOES: An object with a state and on the first screen row is lifted by sixteen times its next kind, moved by its Y speed, given gravity and asked for
; the floor (request 9, probe height 10): on floor its Y speed is stopped; then it is put back down by the same lift.
; HOW: Two 16-bit adds of the lift, ApplyVelocityY, ApplyGravity, ProbeTerrain.
; WAITS: no.
; Entry: X object, Y the probe's column, C passed to the lookup. Exit: see the wrapper.
HopAndSettle:
    lda a:OBJ_STATE,x
    beq HopAndSettle_at0
    lda a:OBJ_Y_HI,x
    bne HopAndSettle_at0
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
    lda #$0A
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    and #$01
    beq HopAndSettle_at1
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
HopAndSettle_at1:
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
HopAndSettle_at0:
    rts

; @routine 0A9E:51B SetHitboxByFrame
; @run 71:CD1B
; WHAT IT DOES: Turns the object's hitbox on and sets its X offset from a table at $CD39 by its frame less $80; from frame $86 on, the hitbox goes off and the
; offset 0.
; HOW: A compare of the frame and a table read (bank 71).
; Untraced: frames $86 and over (CD32).
; WAITS: no.
; Entry: X object. Exit: frame under $86: A the table byte, Y the index, C and V from the subtract; else A the frame, the compare's flags.
SetHitboxByFrame:
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_FRAME,x
    cmp #$86
    bcs SetHitboxByFrame_at0
    sec
    sbc #$80
    tay
    lda a:$CD39,y
    sta a:OBJ_HITBOX_DX,x
    rts
SetHitboxByFrame_at0:
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_HITBOX_DX,x
    rts

; @routine 0A9E:53F StartKnifeThrower
; @run 71:CD3F
; WHAT IT DOES: Loads the object's record. If its next kind is a count (bit 7 clear) it starts that many kind $30 objects after it, each numbered from $81 up, each
; remembering this object's slot and its slot plus count; then the count loses bit 7, its low bit decides the facing, and the throw, the timers and the angle are
; picked (ThrowByNextKindB, PickTimersAtRandomB, PickAngleAtRandomB).
; HOW: LoadObjectRecord, a loop of ClearObjectInY and StartObjectAtPlace.
; Untraced: one byte.
; WAITS: no.
; Entry: X object. Exit: see the wrapper (the three picks, one after the other).
StartKnifeThrower:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    bmi StartKnifeThrower_at0
    txa
    sta a:OBJ_WORK_M,x
    clc
    adc a:OBJ_START_ARG,x
    sta a:OBJ_WORK_N,x
    txa
    tay
    iny
    lda #$81
    sta z:zTemp00
StartKnifeThrower_at1:
    jsr VecClearObjectInY
    lda #$30
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    txa
    sta a:OBJ_WORK_M,y
    lda a:OBJ_WORK_N,x
    sta a:OBJ_WORK_N,y
    inc z:zTemp00
    iny
    dec a:OBJ_START_ARG,x
    bne StartKnifeThrower_at1
StartKnifeThrower_at0:
    lda a:OBJ_START_ARG,x
    and #$7F
    sta a:OBJ_START_ARG,x
    and #$01
    eor #$01
    sta a:OBJ_FACING_LEFT,x
    jsr ThrowByNextKindB
    jsr PickTimersAtRandomB
    jsr PickAngleAtRandomB
    rts

; @routine 0A9E:5EE CountBothWaits
; @run 71:CDEE
; WHAT IT DOES: Runs both waits, CountDownThenAdvance and CountDownAngleThenAdvance.
; HOW: Two calls.
; WAITS: no.
; Entry: X object. Exit: as the second.
CountBothWaits:
    jsr CountDownThenAdvance
    jsr CountDownAngleThenAdvance
    rts

; @routine 0A9E:5F5 CountDownThenAdvance
; @run 71:CDF5
; WHAT IT DOES: A 16-bit wait (OBJ_WORK_E low, the life timer high): while it is not zero it counts down; at zero it picks new timers (PickTimersAtRandomB),
; stops the animation and enters state 2.
; HOW: An ORA of the two bytes, a 16-bit subtract.
; Untraced: the arm at zero (CDFD to CE0B).
; WAITS: no.
; Entry: X object. Exit: see the wrapper.
CountDownThenAdvance:
    lda a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    bne CountDownThenAdvance_at0
    jsr PickTimersAtRandomB
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$02
    sta a:OBJ_STATE,x
    rts
CountDownThenAdvance_at0:
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    rts

; @routine 0A9E:61E CountDownAngleThenAdvance
; @run 71:CE1E
; WHAT IT DOES: With the object's X high byte 0, a 16-bit wait (OBJ_WORK_F high, OBJ_WORK_G low) counts down; at zero it picks a new angle
; (PickAngleAtRandomB), stops the animation, clears OBJ_SUBSTATE and enters state 3.
; HOW: An ORA of the two bytes, a 16-bit subtract.
; Untraced: everything past the X high byte test.
; WAITS: no.
; Entry: X object. Exit: see the wrapper.
CountDownAngleThenAdvance:
    lda a:OBJ_X_HI,x
    bne CountDownAngleThenAdvance_at0
    lda a:OBJ_WORK_F,x
    ora a:OBJ_WORK_G,x
    bne CountDownAngleThenAdvance_at1
    jsr PickAngleAtRandomB
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    lda #$03
    sta a:OBJ_STATE,x
    rts
CountDownAngleThenAdvance_at1:
    lda a:OBJ_WORK_G,x
    sec
    sbc #$01
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    sbc #$00
    sta a:OBJ_WORK_F,x
CountDownAngleThenAdvance_at0:
    rts
