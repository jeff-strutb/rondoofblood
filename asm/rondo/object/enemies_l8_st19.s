; @routine 0B4B:01E LoadRecordInStateZero
; @run 70:B01E
; WHAT IT DOES: In state 0 loads the object's record; later nothing.
; HOW: A test and LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: the state's N and Z, or as LoadObjectRecord.
LoadRecordInStateZero:
    lda a:OBJ_STATE,x
    bne LoadRecordInStateZero_at0
    jsr VecLoadObjectRecord
LoadRecordInStateZero_at0:
    rts

; @routine 0B4B:57E FaceThePlayerByStateTable
; @run 70:B57E
; WHAT IT DOES: Every eighth frame of OBJ_WORK_H, in the states the table at $B596 marks, the object takes the facing of the player's side.
; HOW: An AND, a table read by the state, SideOfPlayer, a store.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
FaceThePlayerByStateTable:
    lda a:OBJ_WORK_H,x
    and #$07
    bne FaceThePlayerByStateTable_at0
    ldy a:OBJ_STATE,x
    lda a:$B596,y
    beq FaceThePlayerByStateTable_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FaceThePlayerByStateTable_at0:
    rts

; @routine 0B4B:5AE StateTableHurtFlag
; @run 70:B5AE
; WHAT IT DOES: Sets OBJ_2EE0 from the table at $B5D2 by the state; in the states the table at $B5E2 marks, an object that has taken 5 hits goes to state $0E and clears its OBJ_SUBSTATE and animation.
; HOW: Two table reads by the state, a compare, stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
StateTableHurtFlag:
    ldy a:OBJ_STATE,x
    lda a:$B5D2,y
    sta a:OBJ_WORK_A,x
    lda a:$B5E2,y
    beq StateTableHurtFlag_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc StateTableHurtFlag_at0
    lda #$0E
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
StateTableHurtFlag_at0:
    rts

; @routine 0B4B:5F2 EnterState0FWhenFlagged
; @run 70:B5F2
; WHAT IT DOES: When bit 7 of OBJ_HIT_STATUS is set and the state is not $0F, plays sound $B7, clears the animation and the speed and goes to state $0F with OBJ_SUBSTATE 0.
; HOW: Two tests, SoundCommand, stores.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The arm past the flag test ran in no script; the draft stops there.
EnterState0FWhenFlagged:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq EnterState0FWhenFlagged_at0
    lda a:OBJ_STATE,x
    cmp #$0F
    beq EnterState0FWhenFlagged_at0
    lda #$B7
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$0F
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
EnterState0FWhenFlagged_at0:
    rts

; @routine 0B4B:620 HitBoxByFrameTable
; @run 70:B620
; WHAT IT DOES: When OBJ_HITBOX_ON is set, sets it 2 and takes the hit box (OBJ_HITBOX_DX, OBJ_HITBOX_DY, OBJ_HITBOX_HALF_W, OBJ_HITBOX_HALF_H) from a four byte row of the table at $C207 picked by the frame less 1.
; HOW: A test, a subtract, two shifts, four table reads, stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
HitBoxByFrameTable:
    lda a:OBJ_HITBOX_ON,x
    beq HitBoxByFrameTable_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_FRAME,x
    sec
    sbc #$01
    asl a
    asl a
    tay
    lda a:$C207,y
    sta a:OBJ_HITBOX_DX,x
    lda a:$C208,y
    sta a:OBJ_HITBOX_DY,x
    lda a:$C209,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:$C20A,y
    sta a:OBJ_HITBOX_HALF_H,x
HitBoxByFrameTable_at0:
    rts

; @routine 0B4B:67E LoadRecordStartPieceAtPlace
; @run 70:B67E
; WHAT IT DOES: Loads the record, sets OBJ_WORK_D $60, clears slot $11 and starts an object of type $26 there at the object's place.
; HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A the start's last byte, Y $11.
LoadRecordStartPieceAtPlace:
    jsr VecLoadObjectRecord
    lda #$60
    sta a:OBJ_WORK_D,x
    ldy #$11
    jsr VecClearObjectInY
    lda #$26
    jmp VecStartObjectAtPlace

; @routine 0B4B:6BD CountDownThenMusicAgain
; @run 70:B6BD
; WHAT IT DOES: Counts OBJ_WORK_D down; at zero clears bits 3 and 4 of zField8E, plays the track again (CdPlayAgain), sets OBJ_WORK_I 1 and OBJ_WORK_D 1 and goes to the next state.
; HOW: A memory decrement, two RMWs of zField8E, CdPlayAgain, stores, an INC.
; WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
; Entry: X object, Y kept for the track call. Exit: as the last instruction that ran.
CountDownThenMusicAgain:
    dec a:OBJ_WORK_D,x
    bne CountDownThenMusicAgain_at0
    rmb3 z:zField8E
    rmb4 z:zField8E
    jsr VecCdPlayAgain
    lda #$01
    sta a:OBJ_WORK_I,x
    lda #$01
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
CountDownThenMusicAgain_at0:
    rts

; @routine 0B4B:6D7 StartScriptsFromTable
; @run 70:B6D7
; WHAT IT DOES: Counts OBJ_WORK_D down; at zero steps OBJ_WORK_J and takes OBJ_WORK_D from the table at $B72B. Each frame it takes the frame and the script from the four byte row of the table at $B729 picked by OBJ_WORK_J and starts the script as kind 1 in slot 5; at the marker $FF it sets bit 2 of zField8E, frame $2D, the speed X $0100 and Y -$0600, OBJ_HITBOX_ON 2, OBJ_WORK_D $30 and goes to the next state.
; HOW: A memory decrement, table reads, StartScriptSlotKind1 between a PHX and a PLX, stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
StartScriptsFromTable:
    dec a:OBJ_WORK_D,x
    bne StartScriptsFromTable_at0
    inc a:OBJ_WORK_J,x
    lda a:OBJ_WORK_J,x
    asl a
    asl a
    tay
    lda a:$B72B,y
    sta a:OBJ_WORK_D,x
StartScriptsFromTable_at0:
    lda a:OBJ_WORK_J,x
    asl a
    asl a
    tay
    lda a:$B729,y
    cmp #$FF
    beq StartScriptsFromTable_at1
    sta a:OBJ_FRAME,x
    phx
    lda a:$B72A,y
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    rts
StartScriptsFromTable_at1:
    lda z:zField8E
    ora #$04
    sta z:zField8E
    lda #$2D
    sta a:OBJ_FRAME,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$FA
    sta a:OBJ_VY_HI,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$30
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
    rts

; @routine 0B4B:76E FallThenSoundAtGround
; @run 70:B76E
; WHAT IT DOES: Falls (FallToGroundAt98); on landing plays sound $4A and goes to the next state.
; HOW: FallToGroundAt98, SoundCommand, an INC.
; WAITS: no.
; Entry: X object, Y the probe column, C into the probe. Exit: as the last instruction that ran.
FallThenSoundAtGround:
    jsr FallToGroundAt98
    bcc FallThenSoundAtGround_at0
    lda #$4A
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
FallThenSoundAtGround_at0:
    rts

; @routine 0B4B:77C FallToGroundAt98
; @run 70:B77C
; WHAT IT DOES: Probes the terrain below the object (request $22, probe height 4), moves it by its speed and under gravity; at Y low $98 or more it stops there and says yes (C 1), else no (C 0).
; HOW: ProbeTerrain, ApplyVelocity, ApplyGravity, a compare.
; WAITS: no.
; Entry: X object, Y the probe column, C into the probe. Exit: C 1 landed, 0 not; the compare's A, N and Z.
FallToGroundAt98:
    lda #$04
    sta z:SpriteFlipMask
    lda #$22
    jsr VecProbeTerrain
    jsr VecApplyVelocity
    jsr VecApplyGravity
    lda a:OBJ_Y_LO,x
    cmp #$98
    bcc FallToGroundAt98_at0
    lda #$98
    sta a:OBJ_Y_LO,x
    sec
    rts
FallToGroundAt98_at0:
    clc
    rts

; @routine 0B4B:79B CountDownThenHopTowardPlayer
; @run 70:B79B
; WHAT IT DOES: Runs CountDownOrPickNextTimer; when it says no (C 0) runs StepListThenWallBounce, copies the facing to OBJ_WORK_F and, when the player is within $28 in X, goes to state 6.
; HOW: Two calls, DistanceXToPlayer, a compare, a store.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
CountDownThenHopTowardPlayer:
    jsr CountDownOrPickNextTimer
    bcs CountDownThenHopTowardPlayer_at0
    jsr StepListThenWallBounce
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_F,x
    jsr VecDistanceXToPlayer
    cmp #$28
    bcs CountDownThenHopTowardPlayer_at0
    lda #$06
    sta a:OBJ_STATE,x
CountDownThenHopTowardPlayer_at0:
    rts

; @routine 0B4B:7B6 StepListThenWallBounce
; @run 70:B7B6
; WHAT IT DOES: Steps the animation list at $B7F8; when the graphic changes it sets the X speed from the table at $B805 by OBJ_WORK_F and goes on to MoveXAndProbeWallBounce; otherwise says no (C 0).
; HOW: StepObjectFrames, a table read, a store, a fall into the next routine.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: C 1 bounced, 0 not.
StepListThenWallBounce:
    lda #$B7
    sta z:zTemp11
    lda #$F8
    sta z:zTemp10
    jsr VecStepObjectFrames
    bcc HomeOnPointThenRingAndSound
    ldy a:OBJ_WORK_F,x
    lda a:$B805,y
    sta a:OBJ_VX_HI,x

; @routine 0B4B:7CC MoveXAndProbeWallBounce
; @run 70:B7CC
; WHAT IT DOES: Moves the object along X, probes the walls on the side it moves to (request 2, probe height 4) and, when the probe's attribute bytes at $26F3 and $26F4 for that side show bit 1, reverses the speed and moves again and says yes (C 1); otherwise no (C 0).
; HOW: ApplyVelocityX, ProbeTerrain, two ROLs, a table read, NegateVelocityX, ApplyVelocityX. The bytes at $26F3 and $26F4 keep their address form until they have names.
; WAITS: no.
; Entry: X object, Y the probe column, C into the move. Exit: C 1 bounced, 0 not. The bounce ran in no script; the draft stops there.
MoveXAndProbeWallBounce:
    jsr VecApplyVelocityX
    lda #$04
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
    beq HomeOnPointThenRingAndSound
    jsr VecNegateVelocityX
    jsr VecApplyVelocityX
    sec
    rts
    clc
    rts

; @routine 0B4C:034 CountDownThenHopAwayFromPlayer
; @run 70:B834
; WHAT IT DOES: Runs CountDownOrPickNextTimer; when it says no (C 0) runs StepListThenWallBounce, sets OBJ_WORK_F to the opposite of the facing and, when the player is $40 or more away in X, goes to state 5.
; HOW: Two calls, DistanceXToPlayer, a compare, stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
CountDownThenHopAwayFromPlayer:
    jsr CountDownOrPickNextTimer
    bcs CountDownThenHopAwayFromPlayer_at0
    jsr StepListThenWallBounce
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_F,x
    jsr VecDistanceXToPlayer
    cmp #$40
    bcc CountDownThenHopAwayFromPlayer_at0
    lda #$05
    sta a:OBJ_STATE,x
CountDownThenHopAwayFromPlayer_at0:
    rts

; @routine 0B4C:15F StepListFallThenSoundAtGround
; @run 70:B95F
; WHAT IT DOES: Steps the animation list at $B97D and falls (FallToGroundAt98); on landing plays sound $4A, sets OBJ_WORK_E $28 and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, FallToGroundAt98, SoundCommand, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListFallThenSoundAtGround:
    lda #$B9
    sta z:zTemp11
    lda #$7D
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr FallToGroundAt98
    bcc StepListFallThenSoundAtGround_at0
    lda #$4A
    jsr VecSoundCommand
    lda #$28
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
StepListFallThenSoundAtGround_at0:
    rts

; @routine 0B4C:304 StepListThenTileWriteAndSpawn
; @run 70:BB04
; WHAT IT DOES: Steps the animation list at $BB1F; when it ends goes to state 5; when the graphic changes runs SoundAndTileWriteAtFrame24 and SoundAndSpawnFiveAtFrame27.
; HOW: StepObjectFrames, two calls, the second a tail jump.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenTileWriteAndSpawn:
    lda #$BB
    sta z:zTemp11
    lda #$1F
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq StepListThenTileWriteAndSpawn_at0
    bcc StepListThenTileWriteAndSpawn_at1
    jsr SoundAndTileWriteAtFrame24
    jmp SoundAndSpawnFiveAtFrame27
StepListThenTileWriteAndSpawn_at0:
    lda #$05
    sta a:OBJ_STATE,x
StepListThenTileWriteAndSpawn_at1:
    rts

; @routine 0B4C:3B8 WaitThenStepListWithSoundAndBounce
; @run 70:BBB8
; WHAT IT DOES: In OBJ_SUBSTATE 0 goes to state 9 when the player is $58 or more away in X, and on to the next OBJ_SUBSTATE. Later it steps the animation list at $BC00: when it ends it clears the animation and goes to state 5; at frame $12 it plays sound $2C, sets the X speed from the table at $BC07 by its facing and goes on to MoveXAndProbeWallBounce.
; HOW: DistanceXToPlayer, StepObjectFrames, SoundCommand, a table read, a tail jump.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
WaitThenStepListWithSoundAndBounce:
    lda a:OBJ_SUBSTATE,x
    bne WaitThenStepListWithSoundAndBounce_at0
    jsr VecDistanceXToPlayer
    cmp #$58
    bcc WaitThenStepListWithSoundAndBounce_at1
    lda #$09
    sta a:OBJ_STATE,x
WaitThenStepListWithSoundAndBounce_at1:
    inc a:OBJ_SUBSTATE,x
    rts
WaitThenStepListWithSoundAndBounce_at0:
    lda #$BC
    sta z:zTemp11
    lda #$00
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq WaitThenStepListWithSoundAndBounce_at2
    bcc JumpByStateTable093C3FF
    lda a:OBJ_FRAME,x
    cmp #$12
    bne JumpByStateTable093C3FF
    lda #$2C
    jsr VecSoundCommand
    ldy a:OBJ_FACING_LEFT,x
    lda a:$BC07,y
    sta a:OBJ_VX_HI,x
    jmp MoveXAndProbeWallBounce
WaitThenStepListWithSoundAndBounce_at2:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
    rts

; @routine 0B4C:46F StepListFallThenCountLandings
; @run 70:BC6F
; WHAT IT DOES: Steps the animation list at $BC9B and falls (FallToGroundAt98); on landing clears the animation, goes to state 6, halves OBJ_WORK_D and counts OBJ_WORK_L up; at 3 clears OBJ_WORK_D.
; HOW: StepObjectFrames, FallToGroundAt98, an LSR and an INC in memory, a compare.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. Clearing OBJ_WORK_D ran in no script; the draft stops there.
StepListFallThenCountLandings:
    lda #$BC
    sta z:zTemp11
    lda #$9B
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr FallToGroundAt98
    bcc StepListFallThenCountLandings_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
    lsr a:OBJ_WORK_D,x
    inc a:OBJ_WORK_L,x
    lda a:OBJ_WORK_L,x
    cmp #$03
    bcc StepListFallThenCountLandings_at0
    stz a:OBJ_WORK_D,x
StepListFallThenCountLandings_at0:
    rts

; @routine 0B4C:755 LoadRecordLeapByNumberThenFall
; @run 70:BF55
; WHAT IT DOES: In state 0 loads the record, gives it the X speed from the table at $BF9B by its facing and the Y speed high $E0, moves it once and sets the speed from a four byte row of the table at $BF9D by OBJ_WORK_D; facing left reverses the X speed. Later it falls (ApplyGravityUnlimited) and moves.
; HOW: LoadObjectRecord, ApplyVelocity, table reads, NegateVelocityX, ApplyGravityUnlimited.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. Reversing the speed ran in no script; the draft stops there.
LoadRecordLeapByNumberThenFall:
    lda a:OBJ_STATE,x
    bne LoadRecordLeapByNumberThenFall_at0
    jsr VecLoadObjectRecord
    ldy a:OBJ_FACING_LEFT,x
    lda a:$BF9B,y
    sta a:OBJ_VX_HI,x
    lda #$E0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    tay
    lda a:$BF9E,y
    sta a:OBJ_VX_HI,x
    lda a:$BF9D,y
    sta a:OBJ_VX_LO,x
    lda a:$BFA0,y
    sta a:OBJ_VY_HI,x
    lda a:$BF9F,y
    sta a:OBJ_VY_LO,x
    lda a:OBJ_FACING_LEFT,x
    beq LoadRecordLeapByNumberThenFall_at1
    jsr VecNegateVelocityX
LoadRecordLeapByNumberThenFall_at1:
    rts
LoadRecordLeapByNumberThenFall_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0B4C:7B1 FollowObject10HitBoxByFrame
; @run 70:BFB1
; WHAT IT DOES: Counts OBJ_WORK_D up. In state 0 loads the record. Later, when OBJ_WORK_I of slot $10 is 0 it clears its hit box width and height; otherwise it takes the place and facing of slot $10 and the hit box from a four byte row of the table at $C33B by slot $10's frame; a slot $10 that is not kind $24 clears the object.
; HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads, stores, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. Clearing the object ran in no script; the draft stops there.
FollowObject10HitBoxByFrame:
    inc a:OBJ_WORK_D,x
    lda a:OBJ_STATE,x
    bne JumpByStateTable
    jmp VecLoadObjectRecord
    lda a:$3680
    bne FollowObject10HitBoxByFrame_at0
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_HALF_H,x
    rts
FollowObject10HitBoxByFrame_at0:
    ldy #$10
    jsr VecCopyObjectPlaceBack
    lda a:$2A78
    sec
    sbc #$01
    asl a
    asl a
    tay
    lda a:$C33B,y
    sta a:OBJ_HITBOX_DX,x
    lda a:$C33C,y
    sta a:OBJ_HITBOX_DY,x
    lda a:$C33D,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:$C33E,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:$2AD0
    sta a:OBJ_FACING_LEFT,x
    lda a:$2760
    cmp #$24
    beq FaceThePlayerInSomeStatesD
    jmp VecClearObject
    rts
