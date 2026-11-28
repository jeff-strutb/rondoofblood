; @routine 088A:610 StateSixteenWhenHit088A
; @run 70:AE10
; WHAT IT DOES: Hit outside states 16 and 18: sound $54, runs $B791, stops and goes to state 16.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StateSixteenWhenHit088A:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq StateSixteenWhenHit088A_at0
    lda a:OBJ_STATE,x
    cmp #$10
    beq StateSixteenWhenHit088A_at0
    cmp #$12
    beq StateSixteenWhenHit088A_at0
    lda #$54
    jsr VecSoundCommand
    jsr TurnOrClearSlots12to15_088B
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$10
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StateSixteenWhenHit088A_at0:
    rts

; @routine 088A:6DF FallOrRun088A
; @run 70:AEDF
; WHAT IT DOES: Frame $0A; first falls until it is on the first screen (Y high byte 0), then runs $AF03 each time.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallOrRun088A:
    lda #$0A
    sta a:OBJ_FRAME,x
    lda a:OBJ_SUBSTATE,x
    bne FallOrRun088A_at0
    jsr VecApplyGravity
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne FallOrRun088A_at1
    inc a:OBJ_SUBSTATE,x
FallOrRun088A_at1:
    rts
FallOrRun088A_at0:
    jsr FrameLandCall088A
    lda a:OBJ_STATE,x
    cmp #$01
    beq FallOrRun088A_at1
    rts

; @routine 0E0B:781 CountThenStartRow0E0B
; @run 70:B781
; WHAT IT DOES: When OBJ_WORK_D runs out: runs $B795, frame 0, a new count of $40, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenStartRow0E0B:
    dec a:OBJ_WORK_D,x
    bne CountThenStartRow0E0B_at0
    jsr StartFifteenKind2B_0E0B
    stz a:OBJ_FRAME,x
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
CountThenStartRow0E0B_at0:
    rts

; @routine 0EBC:61F FallWithoutPrevious0EBC
; @run 70:BE1F
; WHAT IT DOES: With the previous slot empty falls at 4; else runs $BDFF and the state's handler.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallWithoutPrevious0EBCStates = $BE41 ; the handlers by state
FallWithoutPrevious0EBC:
    lda a:OBJ_KIND-1,x
    bne FallWithoutPrevious0EBC_at0
    lda #$04
    sta a:OBJ_VY_HI,x
    jmp VecApplyVelocityY
FallWithoutPrevious0EBC_at0:
    jsr ToState6WhenHitOrAway0EBC
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FallWithoutPrevious0EBCStates,y
    sta z:zTemp00
    lda a:FallWithoutPrevious0EBCStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBD:27C DoorRouteByHit0EBD
; @run 71:C27C
; WHAT IT DOES: A flagged door: while it is not held and the player is ready, a weak hit takes the route in the start argument.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DoorRouteByHit0EBD:
    lda a:OBJ_STATE,x
    bne DoorRouteByHit0EBD_at0
    jmp VecLoadObjectRecord
DoorRouteByHit0EBD_at0:
    lda a:StageDoorHeld
    eor #$01
    and a:PlayerDoorReady
    beq DoorRouteByHit0EBD_at1
    lda a:OBJ_HIT_TAKEN,x
    beq DoorRouteByHit0EBD_at1
    cmp #$03
    bcs DoorRouteByHit0EBD_at1
    lda #$80
    ora a:OBJ_START_ARG,x
    sta z:zField8C
    lda #$02
    sta z:zField82
    lda #$03
    sta z:zSubStep
DoorRouteByHit0EBD_at1:
    rts

; @routine 0EBE:4B6 AnimateThenLeapAcross0EBE
; @run 71:CCB6
; WHAT IT DOES: At the end of the list leaps toward the farther half of the screen, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AnimateThenLeapAcross0EBESpeeds = $CCE5 ; vx high byte by which half of the screen
AnimateThenLeapAcross0EBE:
    ldy #$04
    jsr FaceAndAnimate0EBE
    bne AnimateThenLeapAcross0EBE_at0
    lda a:OBJ_X_LO,x
    rol a
    rol a
    and #$01
    eor #$01
    sta z:zBurstCount
    ldy z:zBurstCount
    lda a:AnimateThenLeapAcross0EBESpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
AnimateThenLeapAcross0EBE_at0:
    rts

; @routine 0EBF:234 TurnBackAtRandom0EBF
; @run 71:D234
; WHAT IT DOES: Keeps the facing, picks OBJ_WORK_I from the distance to the player, runs $D25D, turns the velocity back, runs $D2C9; substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnBackAtRandom0EBFStepD2C9 = $D2C9 ; a routine of this overlay
TurnBackAtRandom0EBF:
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_K,x
    jsr VecPlayerDistanceSum
    and #$0F
    sta a:OBJ_WORK_I,x
    jsr SpeedRowByWorkI0EBF
    jsr VecNegateVelocityX
    jsr VecNegateVelocityY
    jsr TurnBackAtRandom0EBFStepD2C9
    lda #$0C
    sta a:OBJ_WORK_D,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0EBF:2DE RunThenNewCourse0EBF
; @run 71:D2DE
; WHAT IT DOES: Animates and moves for OBJ_WORK_D counts; then sound $C7, $D25D, a new count of $18-$97 in OBJ_WORK_J, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RunThenNewCourse0EBF:
    ldy #$04
    jsr FaceAndAnimate0EBF
    jsr VecApplyVelocity
    dec a:OBJ_WORK_D,x
    bne RunThenNewCourse0EBF_at0
    lda #$C7
    jsr VecSoundCommand
    jsr SpeedRowByWorkI0EBF
    jsr VecNextRandom
    and #$7F
    clc
    adc #$18
    sta a:OBJ_WORK_J,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
RunThenNewCourse0EBF_at0:
    rts

; @routine 0EC0:41D PointCallThenCount0EC0
; @run 71:DC1D
; WHAT IT DOES: Calls $AA35 with the point $30,$16 and $DC93 with A $BC; after OBJ_WORK_E counts the state goes on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PointCallThenCount0EC0StepDC93 = $DC93 ; a routine of this overlay
PointCallThenCount0EC0:
    lda #$30
    sta z:zTemp08
    lda #$16
    sta z:zTemp09
    jsr ContactWithPlayer
    lda #$BC
    jsr PointCallThenCount0EC0StepDC93
    dec a:OBJ_WORK_E,x
    bne PointCallThenCount0EC0_at0
    inc a:OBJ_STATE,x
PointCallThenCount0EC0_at0:
    rts

; @routine 0F6C:770 FaceHopStateTen0F6C
; @run 70:BF70
; WHAT IT DOES: Faces the player, starts a hop, runs $BF97, OBJ_WORK_D $0E, runs $C35F and goes to state 10.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FaceHopStateTen0F6C:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_SUBSTATE,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    jsr SpeedByDistance0F6C
    lda #$0E
    sta a:OBJ_WORK_D,x
    jsr ClearAnimationThenStepListC585
    lda #$0A
    sta a:OBJ_STATE,x
    rts

; @routine 101C:3DA ListSpawnTwice101C
; @run 70:BBDA
; WHAT IT DOES: Runs the list at $BBFC; on each new frame calls $BC4D twice, with 3,1 and with $18,0 in zTemp00-01.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSpawnTwice101C:
    lda #$FC
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc ListSpawnTwice101C_at0
    lda #$03
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    jsr SpawnAtStep101C
    lda #$18
    sta z:zTemp00
    stz z:zTemp01
    jsr SpawnAtStep101C
ListSpawnTwice101C_at0:
    rts

; @routine 101D:480 CloseDoorOnScreen7_101D
; @run 71:C480
; WHAT IT DOES: On screen 7: holds the scroll there, starts the list channels and script $8A, holds the door, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CloseDoorOnScreen7_101D:
    lda a:ScrollXHiScreen
    cmp #$07
    bne CloseDoorOnScreen7_101D_at0
    lda #$07
    sta a:ScrollXMinHi
    phx
    lda #$86
    ldx #$05
    jsr VecStartListChannelKind2
    lda z:zCharacter
    beq CloseDoorOnScreen7_101D_at1
    lda #$87
    ldx #$04
    jsr VecStartListChannelKind2
CloseDoorOnScreen7_101D_at1:
    lda #$8A
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$01
    sta a:StageDoorHeld
    plx
    inc a:OBJ_SUBSTATE,x
CloseDoorOnScreen7_101D_at0:
    rts

; @routine 101D:7DA LeadTrail101D
; @run 71:C7DA
; WHAT IT DOES: Counts OBJ_WORK_H, runs three of the overlay's steps, moves the $80-byte trail at $C7FC along one and puts OBJ_WORK_E plus OBJ_WORK_D at its head.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LeadTrail101DBefore = $C7FB ; one byte before the trail
LeadTrail101DTrail = $C7FC ; the trail, newest first
LeadTrail101D:
    inc a:OBJ_WORK_H,x
    jsr HalveWorkKOnHit101E
    jsr HitToState5_101E
    jsr JumpByStateTable101E094
    ldy #$7F
LeadTrail101D_at0:
    lda a:LeadTrail101DBefore,y
    sta a:LeadTrail101DBefore+1,y
    dey
    bne LeadTrail101D_at0
    lda a:OBJ_WORK_E,x
    clc
    adc a:OBJ_WORK_D,x
    sta a:LeadTrail101DBefore+1
    rts

; @routine 101E:3E5 OrbitByTrail101E
; @run 71:CBE5
; WHAT IT DOES: Runs $CC2B, then out at the angle the trail holds at this piece's lag; slot $10's OBJ_WORK_M counts the state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
OrbitByTrail101ETrail = $C7FC ; the leader's angles
OrbitByTrail101ELags = $CC1B ; the lag in the trail by OBJ_WORK_D
OrbitByTrail101E:
    jsr FollowPreviousWithSlot16_101E
    ldy a:OBJ_WORK_D,x
    lda a:OrbitByTrail101ELags,y
    tay
    stz z:zTemp00
    lda a:OBJ_WORK_I,x
    sta z:zTemp01
    lda a:OrbitByTrail101ETrail,y
    sta a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_M+$10
    beq OrbitByTrail101E_at0
    inc a:OBJ_STATE,x
OrbitByTrail101E_at0:
    rts

; @routine 117B:300 TouchThenTurn117B
; @run 70:B300
; WHAT IT DOES: Calls $AA35 with the point $1C,$1A, taking the push off again; on contact with slot 0 rising or falling, faces the player and goes on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TouchThenTurn117B:
    lda #$83
    sta a:OBJ_SPRITE_ATTR,x
    lda #$1C
    sta z:zTemp08
    lda #$1A
    sta z:zTemp09
    jsr ContactWithPlayer
    php
    lda z:zPlayerPush
    and #$F9
    sta z:zPlayerPush
    plp
    bcc TouchThenTurn117B_at0
    lda a:OBJ_VY_HI
    beq TouchThenTurn117B_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_WORK_E,x
    lda #$05
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
TouchThenTurn117B_at0:
    rts

; @routine 117E:25C PatrolList0_117E
; @run 71:CA5C
; WHAT IT DOES: Runs $CA7C with A 0 and $CAEF facing as OBJ_WORK_H says; every $B0 counts that way turns round.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PatrolList0_117EStepCAEF = $CAEF ; a routine of this overlay
PatrolList0_117E:
    cla
    jsr ListByAThenCheck117E
    jsr PatrolList0_117EStepCAEF
    lda a:OBJ_WORK_H,x
    sta a:OBJ_FACING_LEFT,x
    dec a:OBJ_WORK_E,x
    bne PatrolList0_117E_at0
    lda #$B0
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_H,x
    eor #$01
    sta a:OBJ_WORK_H,x
PatrolList0_117E_at0:
    rts

; @routine 117E:31F PatrolList3_117E
; @run 71:CB1F
; WHAT IT DOES: Runs $CA7C with A 3 and $CAEF facing as OBJ_WORK_H says; every $B0 counts that way turns round.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PatrolList3_117E:
    lda #$03
    jsr ListByAThenCheck117E
    jsr PatrolList0_117EStepCAEF
    lda a:OBJ_WORK_H,x
    sta a:OBJ_FACING_LEFT,x
    dec a:OBJ_WORK_E,x
    bne PatrolList3_117E_at0
    lda #$B0
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_H,x
    eor #$01
    sta a:OBJ_WORK_H,x
PatrolList3_117E_at0:
    rts

; @routine 117E:425 TurnToAttack117E
; @run 71:CC25
; WHAT IT DOES: Runs $CA7C with A 2, keeps the facing in OBJ_WORK_H, runs $CC46; with the player at least $48 across, state 3.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnToAttack117EStepCC46 = $CC46 ; a routine of this overlay
TurnToAttack117E:
    lda #$02
    jsr ListByAThenCheck117E
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_H,x
    jsr TurnToAttack117EStepCC46
    jsr VecDistanceXToPlayer
    cmp #$48
    bcc TurnToAttack117E_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
TurnToAttack117E_at0:
    rts

; @routine 12DC:21F LeadTrail12DC
; @run 70:BA1F
; WHAT IT DOES: Counts OBJ_WORK_H, runs three of the overlay's steps, moves the $F0-byte trail at $BA41 along one and puts OBJ_WORK_E plus OBJ_WORK_D at its head.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LeadTrail12DCBefore = $BA40 ; one byte before the trail
LeadTrail12DCTrail = $BA41 ; the trail, newest first
LeadTrail12DC:
    inc a:OBJ_WORK_H,x
    jsr ParryWhenHit12DC
    jsr HitStopToState6_12DC
    jsr JumpByStateTable12DC388
    ldy #$EF
LeadTrail12DC_at0:
    lda a:LeadTrail12DCBefore,y
    sta a:LeadTrail12DCBefore+1,y
    dey
    bne LeadTrail12DC_at0
    lda a:OBJ_WORK_E,x
    clc
    adc a:OBJ_WORK_D,x
    sta a:LeadTrail12DCBefore+1
    rts

; @routine 12DC:3DC LandThenHittable12DC
; @run 70:BBDC
; WHAT IT DOES: Runs $BCA8; every fourth count of OBJ_WORK_H runs $BBF4, and carry from it makes the object hittable.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandThenHittable12DC:
    jsr WaveAccumulate12DC
    lda a:OBJ_WORK_H,x
    and #$03
    bne LandThenHittable12DC_at0
    jsr FallToFloorA4_12DC
    bcc LandThenHittable12DC_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
LandThenHittable12DC_at0:
    rts

; @routine 12DC:790 FallListThenWait12DC
; @run 70:BF90
; WHAT IT DOES: Runs $BBF4 and the list at $BFA9; at its end a wait of $40, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallListThenWait12DC:
    jsr FallToFloorA4_12DC
    lda #$A9
    sta z:zTemp10
    lda #$BF
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne FallListThenWait12DC_at0
    lda #$40
    sta a:OBJ_WORK_L,x
    inc a:OBJ_SUBSTATE,x
FallListThenWait12DC_at0:
    rts

; @routine 12DE:078 AnimateByWorkDThenState12DE
; @run 71:C878
; WHAT IT DOES: Runs the frame list bits 4-6 of OBJ_WORK_D halved pick, then the state's handler.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AnimateByWorkDThenState12DEStates = $C8A3 ; the handlers by state
AnimateByWorkDThenState12DELists = $C9B9 ; the frame lists
AnimateByWorkDThenState12DE:
    lda a:OBJ_WORK_D,x
    and #$70
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:AnimateByWorkDThenState12DELists,y
    sta z:zTemp10
    lda a:AnimateByWorkDThenState12DELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:AnimateByWorkDThenState12DEStates,y
    sta z:zTemp00
    lda a:AnimateByWorkDThenState12DEStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DE:2DF LandThenHittable12DE
; @run 71:CADF
; WHAT IT DOES: Every fourth count of OBJ_WORK_D runs $CAF4; carry from it makes the object hittable, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandThenHittable12DE:
    lda a:OBJ_WORK_D,x
    and #$03
    bne LandThenHittable12DE_at0
    jsr FallToFloor93_12DE
    bcc LandThenHittable12DE_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
LandThenHittable12DE_at0:
    rts

; @routine 12DE:600 FallListThenWait12DE
; @run 71:CE00
; WHAT IT DOES: Runs $CAF4 and the list at $CE19; at its end a wait of $80, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallListThenWait12DE:
    jsr FallToFloor93_12DE
    lda #$19
    sta z:zTemp10
    lda #$CE
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne FallListThenWait12DE_at0
    lda #$80
    sta a:OBJ_WORK_G,x
    inc a:OBJ_SUBSTATE,x
FallListThenWait12DE_at0:
    rts

; @routine 161C:14C AnimateThenLeapAcross161C
; @run 70:B94C
; WHAT IT DOES: At the end of the list leaps toward the farther half of the screen, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AnimateThenLeapAcross161CSpeeds = $B97B ; vx high byte by which half of the screen
AnimateThenLeapAcross161C:
    ldy #$04
    jsr FaceAndAnimate161C
    bne AnimateThenLeapAcross161C_at0
    lda a:OBJ_X_LO,x
    rol a
    rol a
    and #$01
    eor #$01
    sta z:zBurstCount
    ldy z:zBurstCount
    lda a:AnimateThenLeapAcross161CSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
AnimateThenLeapAcross161C_at0:
    rts

; @routine 18DB:153 ChannelByFrame18DB
; @run 70:B153
; WHAT IT DOES: Counts OBJ_WORK_D, runs $B183 and $B1AE; while OBJ_WORK_J is 0, a frame from $52 on starts its list channel in slot 4.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChannelByFrame18DBChannels = $B175 ; list channel numbers by frame
ChannelByFrame18DB:
    inc a:OBJ_WORK_D,x
    jsr HitInStates5To7_18DB
    jsr JumpByStateTable18DB1AE
    lda a:OBJ_WORK_J,x
    bne ChannelByFrame18DB_at0
    lda a:OBJ_FRAME,x
    sec
    sbc #$52
    bcc ChannelByFrame18DB_at0
    tay
    lda a:ChannelByFrame18DBChannels,y
    phx
    ldx #$04
    jsr VecStartListChannelKind2
    plx
ChannelByFrame18DB_at0:
    rts

; @routine 18DB:296 FaceThenSubstate18DB
; @run 70:B296
; WHAT IT DOES: Faces the player and runs $B2C4; unless that sets carry, the substate's handler.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FaceThenSubstate18DBSubstates = $B2B6 ; the handlers by substate
FaceThenSubstate18DB:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jsr CountStateWhenHit18DB
    bcc FaceThenSubstate18DB_at0
    rts
FaceThenSubstate18DB_at0:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:FaceThenSubstate18DBSubstates,y
    sta z:zTemp00
    lda a:FaceThenSubstate18DBSubstates+1,y
    sta z:zTemp01
    jmp (ZeroPage)
