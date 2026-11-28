; @routine 07DE:492 FallPaleAndLand07DE
; @run 71:CC92
; WHAT IT DOES: While OBJ_WORK_H counts down, a palette from $8C by it. At Y $D0-$DF bursts debris ($CCE7). Lands (height 8) to
; the next state; then shows frame $79 or steps ($CCC0).
; HOW: StopOnLanding with SpriteFlipMask 8.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallPaleAndLand07DE:
    lda a:OBJ_FALL_LO,x
    beq FallPaleAndLand07DE_at0
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    clc
    adc #$8C
    sta a:OBJ_SPRITE_ATTR,x
    dec a:OBJ_FALL_LO,x
FallPaleAndLand07DE_at0:
    lda a:OBJ_Y_LO,x
    and #$F0
    cmp #$D0
    bne FallPaleAndLand07DE_at1
    jsr DebrisWithSound07DE
FallPaleAndLand07DE_at1:
    lda #$08
    sta z:SpriteFlipMask
    jsr VecStopOnLanding
    bcc PictureByRise07DE
    stz a:OBJ_SUBSTATE,x
    inc a:OBJ_STATE,x

; @routine 07DE:4C0 PictureByRise07DE
; @run 71:CCC0
; WHAT IT DOES: Frame $79 while falling; while rising, steps the list at $CCD6.
; HOW: The sign of the Y speed.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PictureByRise07DEList = $CCD6 ; a frame list
PictureByRise07DE:
    lda a:OBJ_VY_HI,x
    bmi PictureByRise07DE_at0
    lda #$79
    sta a:OBJ_FRAME,x
    rts
PictureByRise07DE_at0:
    lda #$D6
    sta z:zTemp10
    lda #$CC
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 07DE:4E7 DebrisWithSound07DE
; @run 71:CCE7
; WHAT IT DOES: Sound $23 on the first screen and a debris burst with count 0.
; HOW: A tail jump to StartDebrisBurst.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DebrisWithSound07DE:
    lda #$23
    jsr VecSoundIfOnFirstScreen
    cla
    jmp StartDebrisBurst

; @routine 07DE:4F0 RestThenRise07DE
; @run 71:CCF0
; WHAT IT DOES: Substate 0: stops with a wait of $20 and clears the object far off the screen. Then steps the list at $CD4C and,
; the wait over, faces the player and goes on; checks the ground ($CD55).
; HOW: Substate branches.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RestThenRise07DEList = $CD4C ; a frame list
RestThenRise07DE:
    lda a:OBJ_SUBSTATE,x
    bne RestThenRise07DE_at3
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$20
    sta a:OBJ_PIECE_PLACE,x
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_X_HI,x
    beq RestThenRise07DE_at2
    bmi RestThenRise07DE_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcc RestThenRise07DE_at0
    jmp VecClearObject
RestThenRise07DE_at0:
    rts
RestThenRise07DE_at1:
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcs RestThenRise07DE_at2
    jmp VecClearObject
RestThenRise07DE_at2:
    rts
RestThenRise07DE_at3:
    lda #$4C
    sta z:zTemp10
    lda #$CD
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_PIECE_PLACE,x
    bne RestThenRise07DE_at4
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$40
    sta a:OBJ_PIECE_PLACE,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
RestThenRise07DE_at4:
    jmp RestOffEdge07DE

; @routine 07DE:555 RestOffEdge07DE
; @run 71:CD55
; WHAT IT DOES: Without ground below (probe $2B), stops and goes to state 3.
; HOW: ProbeTerrain with SpriteFlipMask 8.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RestOffEdge07DE:
    lda #$08
    sta z:SpriteFlipMask
    lda #$2B
    jsr VecProbeTerrain
    and #$01
    bne RestOffEdge07DE_at0
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$03
    sta a:OBJ_STATE,x
RestOffEdge07DE_at0:
    rts

; @routine 07DE:574 WalkThenChoose07DE
; @run 71:CD74
; @stop CD9B
; WHAT IT DOES: Steps the list at $CD4C and walks the way it faces while OBJ_PIECE_PLACE counts; at zero chooses ($CD9F).
; HOW: A speed table by facing; then the ground check.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkThenChoose07DEList = $CD4C ; a frame list
WalkThenChoose07DESpeeds = $CD9B ; VX words by facing
WalkThenChoose07DE:
    lda #$4C
    sta z:zTemp10
    lda #$CD
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_PIECE_PLACE,x
    beq ChooseGhoulAttack07DE
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:WalkThenChoose07DESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkThenChoose07DESpeeds,y
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    jmp RestOffEdge07DE

; @routine 07DE:59F ChooseGhoulAttack07DE
; @run 71:CD9F
; WHAT IT DOES: Faces the player, stops, and by a random table goes to state 6, to state 4, or leaps (state 7) walking on
; ($CDF2).
; HOW: NextRandom's high bits.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChooseGhoulAttack07DEChoices = $CE04 ; sixteen choices
ChooseGhoulAttack07DE:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    jsr VecNextRandom
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    lda a:ChooseGhoulAttack07DEChoices,y
    beq ChooseGhoulAttack07DE_at0
    dec a
    beq ChooseGhoulAttack07DE_at1
    bra ChooseGhoulAttack07DE_at2
ChooseGhoulAttack07DE_at0:
    lda #$06
    sta a:OBJ_STATE,x
    rts
ChooseGhoulAttack07DE_at1:
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts
ChooseGhoulAttack07DE_at2:
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    lda #$16
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$08
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$74
    sta a:OBJ_FRAME,x
    stz a:OBJ_WORK_K,x
    lda #$07
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x

; @routine 07DE:5F2 WalkSpeedByFacing07DE
; @run 71:CDF2
; WHAT IT DOES: The X speed the table gives for the facing.
; HOW: Two loads.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkSpeedByFacing07DESpeeds = $CE14 ; VX words by facing
WalkSpeedByFacing07DE:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:WalkSpeedByFacing07DESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkSpeedByFacing07DESpeeds,y
    sta a:OBJ_VX_LO,x
    rts

; @routine 07DE:618 ThrowAtListEnd07DE
; @run 71:CE18
; WHAT IT DOES: Steps the list at $CE40; at its end starts a kind $2F object facing the same way and goes to state 4. Then the
; ground check.
; HOW: FindFreeSlotFrom30 and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowAtListEnd07DEList = $CE40 ; a frame list
ThrowAtListEnd07DE:
    lda #$40
    sta z:zTemp10
    lda #$CE
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ThrowAtListEnd07DE_at1
    jsr VecFindFreeSlotFrom30
    bcc ThrowAtListEnd07DE_at0
    lda #$2F
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
ThrowAtListEnd07DE_at0:
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ThrowAtListEnd07DE_at1:
    jmp RestOffEdge07DE

; @routine 07DE:64D LeapAndBounce07DE
; @run 71:CE4D
; @stop CE7C
; WHAT IT DOES: Substate 0: on landing takes the walking speed and bounces up to twice by the table; then substate 1 ($CE80).
; HOW: StopOnLanding with height $0A.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LeapAndBounce07DEBounces = $CE7C ; VY words by bounce
LeapAndBounce07DE:
    lda a:OBJ_SUBSTATE,x
    bne SlowAndLandAfterLeap07DE
    lda #$0A
    jsr VecStopOnLanding
    bcc LeapAndBounce07DE_at0
    jsr WalkSpeedByFacing07DE
    lda a:OBJ_WORK_K,x
    cmp #$02
    bcs LeapAndBounce07DE_at1
    lda a:OBJ_WORK_K,x
    asl a
    tay
    lda a:LeapAndBounce07DEBounces+1,y
    sta a:OBJ_VY_HI,x
    lda a:LeapAndBounce07DEBounces,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_WORK_K,x
LeapAndBounce07DE_at0:
    rts
LeapAndBounce07DE_at1:
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 07DE:680 SlowAndLandAfterLeap07DE
; @run 71:CE80
; WHAT IT DOES: Slows the X speed and moves with the ground. Stopped, hit a wall or fallen off: hops up in state 3 ($CEA9) or
; rests ($CECD).
; HOW: MoveAndLand with height $0A.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SlowAndLandAfterLeap07DESlows = $CEE2 ; X words by facing
SlowAndLandAfterLeap07DE:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:SlowAndLandAfterLeap07DESlows,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:SlowAndLandAfterLeap07DESlows+1,y
    sta a:OBJ_VX_HI,x
    ora a:OBJ_VX_LO,x
    beq SlowAndLandAfterLeap07DE_at0
    lda #$0A
    jsr VecMoveAndLand
    bcs SlowAndLandAfterLeap07DE_at0
    and #$01
    beq SlowAndLandAfterLeap07DE_at1
    rts
SlowAndLandAfterLeap07DE_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$0C
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$10
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$03
    sta a:OBJ_STATE,x
    rts
SlowAndLandAfterLeap07DE_at1:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 07DE:6E6 DieRisingThenFall07DE
; @run 71:CEE6
; WHAT IT DOES: Substate 0 starts a rise of -4; later bursts at Y $D0-$DF, falls, and shows the falling picture.
; HOW: Into PictureByRise07DE.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DieRisingThenFall07DE:
    lda a:OBJ_SUBSTATE,x
    bne DieRisingThenFall07DE_at0
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
    rts
DieRisingThenFall07DE_at0:
    lda a:OBJ_Y_LO,x
    and #$F0
    cmp #$D0
    bne DieRisingThenFall07DE_at1
    jsr DebrisWithSound07DE
DieRisingThenFall07DE_at1:
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocity
    jmp PictureByRise07DE

; @routine 07DE:70E SinkPaleAndClear07DE
; @run 71:CF0E
; WHAT IT DOES: Palette $8B, moves down the Y speed and clears the object when the list at $CF27 ends.
; HOW: ApplyVelocityY and StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SinkPaleAndClear07DEList = $CF27 ; a frame list
SinkPaleAndClear07DE:
    lda #$8B
    sta a:OBJ_SPRITE_ATTR,x
    jsr VecApplyVelocityY
    lda #$27
    sta z:zTemp10
    lda #$CF
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne SinkPaleAndClear07DE_at0
    jmp VecClearObject
SinkPaleAndClear07DE_at0:
    rts

; @routine 07DE:758 DroppedBoneRise07DE
; @run 71:CF58
; WHAT IT DOES: State 0: sound $25, loads the record, rises one step and takes the X speed by facing. Later steps the list at
; $CF91 and moves sideways.
; HOW: Stores and tail jumps.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DroppedBoneRise07DESpeeds = $CF8D ; VX words by facing
DroppedBoneRise07DEList = $CF91 ; a frame list
DroppedBoneRise07DE:
    lda a:OBJ_STATE,x
    bne DroppedBoneRise07DE_at0
    lda #$25
    jsr VecSoundIfOnFirstScreen
    jsr VecLoadObjectRecord
    lda #$F5
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:DroppedBoneRise07DESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:DroppedBoneRise07DESpeeds,y
    sta a:OBJ_VX_LO,x
    rts
DroppedBoneRise07DE_at0:
    lda #$91
    sta z:zTemp10
    lda #$CF
    sta z:zTemp11
    jsr VecStepObjectFrames
    jmp VecApplyVelocityX

; @routine 07DE:796 DieCheckThenByState07DE
; @run 71:CF96
; WHAT IT DOES: Takes a killing hit ($CFB3) and goes to the state's handler.
; HOW: A table of handlers at $CFAB.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DieCheckThenByState07DEStates = $CFAB ; a handler per state
DieCheckThenByState07DE:
    jsr DieWhenHit07DECFB3
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:DieCheckThenByState07DEStates,y
    sta z:zTemp00
    lda a:DieCheckThenByState07DEStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DE:7B3 DieWhenHit07DECFB3
; @run 71:CFB3
; WHAT IT DOES: A hit under state 3 turns it into sprite set 3 rising slowly, without a hit box, in state 3.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DieWhenHit07DECFB3:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq DieWhenHit07DECFB3_at0
    lda a:OBJ_STATE,x
    cmp #$03
    bcs DieWhenHit07DECFB3_at0
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
    lda #$03
    sta a:OBJ_STATE,x
DieWhenHit07DECFB3_at0:
    rts

; @routine 07DE:7E2 EnterAtRandomEdge07DE
; @run 71:CFE2
; WHAT IT DOES: Loads the record; a random speed and X from one of two sides, a random Y, facing the player.
; HOW: Tables by NextRandom.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EnterAtRandomEdge07DESides = $D01D ; VX word and X word by side
EnterAtRandomEdge07DERows = $D025 ; four Y
EnterAtRandomEdge07DE:
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    sta a:OBJ_PIECE_NEXT,x
    and #$01
    asl a
    asl a
    tay
    lda a:EnterAtRandomEdge07DESides+1,y
    sta a:OBJ_VX_HI,x
    lda a:EnterAtRandomEdge07DESides,y
    sta a:OBJ_VX_LO,x
    lda a:EnterAtRandomEdge07DESides+3,y
    sta a:OBJ_X_HI,x
    lda a:EnterAtRandomEdge07DESides+2,y
    sta a:OBJ_X_LO,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:EnterAtRandomEdge07DERows,y
    sta a:OBJ_Y_LO,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    rts
