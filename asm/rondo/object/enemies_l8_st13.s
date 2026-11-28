; @routine 025C:131 StepListThenStartChannel85
; @run 70:B931
; WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero starts list $85 on channel 5, sets OBJ_3670 5, OBJ_3510 $7B, the byte before the frame $2F and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, a memory decrement, StartListChannelKind2 between a PHX and a PLX, stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenStartChannel85:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_3510,x
    bne StepListThenStartChannel85_at0
    phx
    lda #$85
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    lda #$05
    sta a:OBJ_3670,x
    lda #$7B
    sta a:OBJ_3510,x
    lda #$2F
    sta a:$2A67,x
    inc a:OBJ_SUBSTATE,x
StepListThenStartChannel85_at0:
    rts

; @routine 025C:15D StepListThenStopAnimation
; @run 70:B95D
; WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears OBJ_3670 and the animation step and timer and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, a memory decrement, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenStopAnimation:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_3510,x
    bne StepListThenStopAnimation_at0
    stz a:OBJ_3670,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
StepListThenStopAnimation_at0:
    rts

; @routine 025C:17A StepListB995ThenWait
; @run 70:B97A
; WHAT IT DOES: Steps the animation list at $B995; when it ends sets OBJ_3670 6 and OBJ_3510 $3A and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListB995ThenWait:
    lda #$95
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB995ThenWait_at0
    lda #$06
    sta a:OBJ_3670,x
    lda #$3A
    sta a:OBJ_3510,x
    inc a:OBJ_SUBSTATE,x
StepListB995ThenWait_at0:
    rts

; @routine 025C:19C StepListThenStartType2D
; @run 70:B99C
; WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears slot $50, starts an object of type $2D there at the object's place, sets OBJ_3670 7 and OBJ_3510 $83 and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, a memory decrement, ClearObjectInY, StartObjectAtPlace, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenStartType2D:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_3510,x
    bne StepListThenStartType2D_at0
    ldy #$50
    jsr VecClearObjectInY
    lda #$2D
    jsr VecStartObjectAtPlace
    lda #$07
    sta a:OBJ_3670,x
    lda #$83
    sta a:OBJ_3510,x
    inc a:OBJ_SUBSTATE,x
StepListThenStartType2D_at0:
    rts

; @routine 025C:1C4 StepListThenClearAnimation
; @run 70:B9C4
; WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears the animation step and timer and OBJ_3670 and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, a memory decrement, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenClearAnimation:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_3510,x
    bne StepListThenClearAnimation_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_3670,x
    inc a:OBJ_SUBSTATE,x
StepListThenClearAnimation_at0:
    rts

; @routine 025C:1E1 StepListThenStartType2E
; @run 70:B9E1
; WHAT IT DOES: Steps the animation list at $BA0E; when it ends clears OBJ_3720, sets OBJ_3670 8, OBJ_34B8 1 and OBJ_3510 $80, goes to the next OBJ_SUBSTATE, clears slot $30 and starts an object of type $2E there at the object's place.
; HOW: StepObjectFrames, stores, an INC, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenStartType2E:
    lda #$0E
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne CountAnglePairDownOrRepick
    stz a:OBJ_3720,x
    lda #$08
    sta a:OBJ_3670,x
    lda #$01
    sta a:OBJ_34B8,x
    lda #$80
    sta a:OBJ_3510,x
    inc a:OBJ_SUBSTATE,x
    ldy #$30
    jsr VecClearObjectInY
    lda #$2E
    jsr VecStartObjectAtPlace
    rts

; @routine 025C:215 StepListThenSoundAndStartChannel86
; @run 70:BA15
; WHAT IT DOES: Steps the animation list at $B6DA; when the 16-bit timer OBJ_34B8:OBJ_3510 counts down to zero starts list $86 on channel 5, plays sound $76, clears the animation, OBJ_35C0, OBJ_38D8 and OBJ_3670, sets OBJ_3880 2 and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, a 16-bit subtract in memory, StartListChannelKind2 between a PHX and a PLX, SoundCommand, stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenSoundAndStartChannel86:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_3510,x
    sec
    sbc #$01
    sta a:OBJ_3510,x
    lda a:OBJ_34B8,x
    sbc #$00
    sta a:OBJ_34B8,x
    ora a:OBJ_3510,x
    bne StepListThenSoundAndStartChannel86_at0
    phx
    lda #$86
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    lda #$76
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_35C0,x
    lda #$02
    sta a:OBJ_3880,x
    stz a:OBJ_38D8,x
    stz a:OBJ_3670,x
    inc a:OBJ_SUBSTATE,x
StepListThenSoundAndStartChannel86_at0:
    rts

; @routine 025C:25C SpiralOutWhileStepping
; @run 70:BA5C
; WHAT IT DOES: Steps the animation list at $BA9A while moving the object by the speed made from OBJ_35C0 and the radius OBJ_3880:OBJ_38D8; the radius shrinks by one and OBJ_35C0 turns on by 4; when OBJ_35C0 wraps it goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, SetVelocityFromAngle, ApplyVelocity, a 16-bit subtract in memory, an add.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
SpiralOutWhileStepping:
    lda #$9A
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_38D8,x
    sta z:zTemp00
    lda a:OBJ_3880,x
    sta z:zTemp01
    lda a:OBJ_35C0,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_38D8,x
    sec
    sbc #$01
    sta a:OBJ_38D8,x
    lda a:OBJ_3880,x
    sbc #$00
    sta a:OBJ_3880,x
    lda a:OBJ_35C0,x
    clc
    adc #$04
    sta a:OBJ_35C0,x
    bcc SpiralOutWhileStepping_at0
    inc a:OBJ_SUBSTATE,x
SpiralOutWhileStepping_at0:
    rts

; @routine 025C:2A7 MoveXUntilPastLeftEdge
; @run 70:BAA7
; WHAT IT DOES: Moves the object along X by its speed; once its X high byte is not 0 and its low byte is below $80 sets OBJ_3510 $20 and goes to the next OBJ_SUBSTATE.
; HOW: ApplyVelocityX, two tests, a store, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
MoveXUntilPastLeftEdge:
    jsr VecApplyVelocityX
    lda a:OBJ_X_HI,x
    beq RichterFloorOddScreen
    lda a:OBJ_X_LO,x
    bpl RichterFloorOddScreen
    lda #$20
    sta a:OBJ_3510,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 025C:2BD CountDownThenClearAndAdvanceSlot10
; @run 70:BABD
; WHAT IT DOES: Counts OBJ_3510 down; at zero goes to the next state of slot $10 and clears the object.
; HOW: A memory decrement, an INC, ClearObject.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
CountDownThenClearAndAdvanceSlot10:
    dec a:OBJ_3510,x
    bne CountDownThenClearAndAdvanceSlot10_at0
    inc a:$27B8
    jsr VecClearObject
CountDownThenClearAndAdvanceSlot10_at0:
    rts

; @routine 025C:2EA SoundWhenHurtInStatesOneToThree
; @run 70:BAEA
; WHAT IT DOES: In states 1 to 3, when OBJ_3568 is 1 or 2 or bit 7 of OBJ_31A0 is set, plays sound $43 and goes to state 4 (state 5 when OBJ_3568 is 2).
; HOW: Compares, SoundCommand, a store.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The OBJ_3568 = 2 arm ran in no script; the draft stops there.
SoundWhenHurtInStatesOneToThree:
    lda a:OBJ_STATE,x
    beq SoundWhenHurtInStatesOneToThree_at0
    cmp #$04
    bcs SoundWhenHurtInStatesOneToThree_at0
    lda a:OBJ_3568,x
    cmp #$01
    beq SoundWhenHurtInStatesOneToThree_at1
    cmp #$02
    beq SoundWhenHurtInStatesOneToThree_at2
    lda a:OBJ_31A0,x
    and #$80
    beq SoundWhenHurtInStatesOneToThree_at0
SoundWhenHurtInStatesOneToThree_at1:
    lda #$43
    jsr VecSoundCommand
    lda #$04
    sta a:OBJ_STATE,x
SoundWhenHurtInStatesOneToThree_at0:
    rts
SoundWhenHurtInStatesOneToThree_at2:
    lda #$43
    jsr VecSoundCommand
    lda #$05
    sta a:OBJ_STATE,x
    rts

; @routine 025C:31B LoadRecordAimAtPlayer
; @run 70:BB1B
; WHAT IT DOES: Loads the object's record, sets X speed -$20 and moves it, finds the angle to the point $FF00:$00F0 from the player's side and sets the speed from it with a radius of $0280; OBJ_3510 $40.
; HOW: LoadObjectRecord, ApplyVelocityX, AngleToPlayerOffset, SetVelocityFromAngle.
; WAITS: no.
; Entry: X object. Exit: A $40.
LoadRecordAimAtPlayer:
    jsr VecLoadObjectRecord
    lda #$E0
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    stz z:zTemp08
    stz z:zTemp09
    lda #$F0
    sta z:zTemp0A
    lda #$FF
    sta z:zTemp0B
    jsr VecAngleToPlayerOffset
    pha
    lda #$80
    sta z:zTemp00
    lda #$02
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    lda #$40
    sta a:OBJ_3510,x
    rts

; @routine 025C:348 StepListThenCountdownNextState
; @run 70:BB48
; WHAT IT DOES: Steps the animation list at $BB69, sets OBJ_3250 2, moves the object and counts OBJ_3510 down; at zero sets it $40 and goes to the next state.
; HOW: StepObjectFrames, ApplyVelocity, a memory decrement, a store, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenCountdownNextState:
    lda #$69
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$02
    sta a:OBJ_3250,x
    jsr VecApplyVelocity
    dec a:OBJ_3510,x
    bne StepListThenCountdownNextState_at0
    lda #$40
    sta a:OBJ_3510,x
    inc a:OBJ_STATE,x
StepListThenCountdownNextState_at0:
    rts

; @routine 025C:376 StepListThenSound43AtZero
; @run 70:BB76
; WHAT IT DOES: Steps the animation list at $BB69; when OBJ_3510 counts down to zero plays sound $43 and goes to the next state.
; HOW: StepObjectFrames, a memory decrement, SoundCommand, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenSound43AtZero:
    lda #$69
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_3510,x
    bne StepListThenSound43AtZero_at0
    lda #$43
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
StepListThenSound43AtZero_at0:
    rts

; @routine 025C:38F SeekObject19ThenHitAndClear
; @run 70:BB8F
; WHAT IT DOES: Steps the animation list at $BB69 while flying toward slot $19 with the radius $0280; when near it (NearObject19) sets the state of slot $19 to 7, its OBJ_3670 2, the frame of slot $18 1 and its animation clear, picks OBJ_36C8 of slot $19 from the table at $BBE5 by a random number and clears the object.
; HOW: StepObjectFrames, AngleToPoint, SetVelocityFromAngle, ApplyVelocity, NearObject19, NextRandom, ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
SeekObject19ThenHitAndClear:
    lda #$69
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    stz z:zTemp09
    lda a:$2BE1
    sta z:zTemp08
    stz z:zTemp0B
    lda a:$2CE9
    sta z:zTemp0A
    jsr VecAngleToPoint
    pha
    lda #$80
    sta z:zTemp00
    lda #$02
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    jsr NearObject19
    bcc SeekObject19ThenHitAndClear_at0
    lda #$07
    sta a:$27C1
    lda #$02
    sta a:$3689
    lda #$01
    sta a:$2A80
    stz a:$3420
    stz a:$3478
    jsr VecNextRandom
    and #$03
    tay
    lda a:$BBE5,y
    sta a:$36E1
    jsr VecClearObject
SeekObject19ThenHitAndClear_at0:
    rts

; @routine 025C:3E9 NearObject19
; @run 70:BBE9
; WHAT IT DOES: Says whether the object is within 7 of the object in slot $19 on both axes (low bytes).
; HOW: Two absolute differences and compares; C 1 when near.
; WAITS: no.
; Entry: X object. Exit: C 1 near, 0 not; A the last difference, N and Z from the compare, V from the last subtract.
NearObject19:
    lda a:OBJ_X_LO,x
    sec
    sbc a:$2BE1
    bcs NearObject19_at0
    eor #$FF
    inc a
NearObject19_at0:
    cmp #$08
    bcs NearObject19_at1
    lda a:OBJ_Y_LO,x
    sec
    sbc a:$2CE9
    bcs NearObject19_at2
    eor #$FF
    inc a
NearObject19_at2:
    cmp #$08
    bcs NearObject19_at1
    sec
    rts
NearObject19_at1:
    clc
    rts

; @routine 025C:425 StartRiseAndSpawnRing
; @run 70:BC25
; WHAT IT DOES: Starts the ring of seven objects (SpawnRingOfSeven), sets OBJ_37D0 of slot $19 to 1, clears the object's OBJ_3250, animation and X speed, gives it the Y speed -$0400 and goes to the next OBJ_SUBSTATE.
; HOW: A call, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: A $FC, N and Z from the new OBJ_SUBSTATE.
StartRiseAndSpawnRing:
    jsr SpawnRingOfSeven
    lda #$01
    sta a:$37E9
    stz a:OBJ_3250,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 025C:448 SpawnRingOfSeven
; @run 70:BC48
; WHAT IT DOES: Clears the slots $20 to $26 and starts an object of type $2A in each at the object's place, giving each its number 0 to 6 in OBJ_34B8.
; HOW: A loop of ClearObjectInY, StartObjectAtPlace and a counter in zTemp00.
; WAITS: no.
; Entry: X object. Exit: Y $27, C 1, N and Z from the compare, A the last number.
SpawnRingOfSeven:
    stz z:zTemp00
    ldy #$20
SpawnRingOfSeven_at0:
    jsr VecClearObjectInY
    lda #$2A
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_34B8,y
    inc z:zTemp00
    iny
    cpy #$27
    bcc SpawnRingOfSeven_at0
    rts

; @routine 025C:461 FallAlongListUntilGround
; @run 70:BC61
; WHAT IT DOES: Steps the animation list at $BC98 while the object falls (ApplyGravityUnlimited, ApplyVelocityY); once its Y low byte reaches $B8 it takes sprite set 3, frame 0, the speed X -$0400, Y -$0200, clears the animation and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocityY, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
FallAlongListUntilGround:
    lda #$98
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp #$B8
    bcc FallAlongListUntilGround_at0
    lda #$03
    sta a:OBJ_2A10,x
    stz a:OBJ_FRAME,x
    lda #$FC
    sta a:OBJ_VX_HI,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
FallAlongListUntilGround_at0:
    rts

; @routine 025C:4B9 StepListCCThenMoveOrClear
; @run 70:BCB9
; WHAT IT DOES: Steps the animation list at $BCCC; while it runs the object moves by its speed, when it ends the object is cleared.
; HOW: StepObjectFrames, then a tail jump into ClearObject or ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the step and then ClearObject (all kept), or as ApplyVelocity.
StepListCCThenMoveOrClear:
    lda #$CC
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListCCThenMoveOrClear_at0
    jmp VecClearObject
StepListCCThenMoveOrClear_at0:
    jmp VecApplyVelocity

; @routine 025C:534 LoadRecordThrowByNumber
; @run 70:BD34
; WHAT IT DOES: Loads the object's record, then gives it the speed (both axes) from a four byte entry of the table at $BD66 picked by its number OBJ_34B8, the timer OBJ_3510 from a table at $BD86 and the frame $28 plus the number.
; HOW: LoadObjectRecord, table reads by the number times 4, an add.
; WAITS: no.
; Entry: X object. Exit: A the frame, C and V from the add, Y the number.
LoadRecordThrowByNumber:
    jsr VecLoadObjectRecord
    lda a:OBJ_34B8,x
    asl a
    asl a
    tay
    lda a:$BD67,y
    sta a:OBJ_VX_HI,x
    lda a:$BD66,y
    sta a:OBJ_VX_LO,x
    lda a:$BD69,y
    sta a:OBJ_VY_HI,x
    lda a:$BD68,y
    sta a:OBJ_VY_LO,x
    ldy a:OBJ_34B8,x
    lda a:$BD86,y
    sta a:OBJ_3510,x
    tya
    clc
    adc #$28
    sta a:OBJ_FRAME,x
    rts

; @routine 025C:58E FallThenTurnAtTimerEnd
; @run 70:BD8E
; WHAT IT DOES: The object falls and moves; when OBJ_3510 counts down to zero it takes sprite set 3, frame 0, the speed X -$0400, Y -$0100, clears the animation and goes to the next state.
; HOW: ApplyGravityUnlimited, ApplyVelocity, a memory decrement, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
FallThenTurnAtTimerEnd:
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocity
    dec a:OBJ_3510,x
    bne FallThenTurnAtTimerEnd_at0
    lda #$03
    sta a:OBJ_2A10,x
    stz a:OBJ_FRAME,x
    lda #$FC
    sta a:OBJ_VX_HI,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
FallThenTurnAtTimerEnd_at0:
    rts

; @routine 025C:5B8 StepListCBThenMoveOrClear
; @run 70:BDB8
; WHAT IT DOES: Steps the animation list at $BDCB; while it runs the object moves by its speed, when it ends the object is cleared.
; HOW: StepObjectFrames, then a tail jump into ClearObject or ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the step and then ClearObject (all kept), or as ApplyVelocity.
StepListCBThenMoveOrClear:
    lda #$CB
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListCBThenMoveOrClear_at0
    jmp VecClearObject
StepListCBThenMoveOrClear_at0:
    jmp VecApplyVelocity

; @routine 025C:5F8 FollowObject19ThenStepListByMode
; @run 70:BDF8
; WHAT IT DOES: In state 0 loads the record. Later it takes the place of slot $19, restarts its animation when OBJ_3670 of slot $19 changed, and shows frame by it: from a table at $BE33 when the entry is below $80, otherwise by stepping the list the entry names (low nibble times two, at $BE3C).
; HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads, a tail jump into StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
FollowObject19ThenStepListByMode:
    lda a:OBJ_STATE,x
    bne FollowObject19ThenStepListByMode_at0
    jmp VecLoadObjectRecord
FollowObject19ThenStepListByMode_at0:
    ldy #$19
    jsr VecCopyObjectPlaceBack
    lda a:$3689
    cmp a:OBJ_34B8,x
    beq FollowObject19ThenStepListByMode_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
FollowObject19ThenStepListByMode_at1:
    sta a:OBJ_34B8,x
    ldy a:$3689
    lda a:$BE33,y
    bmi FollowObject19ThenStepListByMode_at2
    sta a:OBJ_FRAME,x
    rts
FollowObject19ThenStepListByMode_at2:
    and #$0F
    asl a
    tay
    lda a:$BE3C,y
    sta z:zTemp10
    lda a:$BE3D,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 025C:6D6 StartScriptByWaveBit
; @run 70:BED6
; WHAT IT DOES: Starts script $8F or $90 (by bit 0 of the wave counter) as kind 1 in slot 4.
; HOW: StartScriptSlotKind1 between a PHX and a PLX.
; WAITS: no.
; Entry: X object. Exit: A and Y the script, N and Z from X, C and V from the add.
StartScriptByWaveBit:
    phx
    lda z:zWaveCounter
    and #$01
    clc
    adc #$8F
    ldx #$04
    jsr VecStartScriptSlotKind1
    plx
    rts

; @routine 025C:6E5 LoadRecordWithSound1C
; @run 70:BEE5
; WHAT IT DOES: Loads the object's record and plays sound $1C.
; HOW: LoadObjectRecord, SoundCommand.
; WAITS: no.
; Entry: X object. Exit: as the sound call.
LoadRecordWithSound1C:
    jsr VecLoadObjectRecord
    lda #$1C
    jsr VecSoundCommand
    rts

; @routine 025C:6EE FollowObject19ThenLaunch
; @run 70:BEEE
; WHAT IT DOES: Takes the place of slot $19 and steps the animation list at $BF16; when it ends sets the speed from the angle $68 with the radius $0400, OBJ_3250 2 and goes to the next state.
; HOW: CopyObjectPlaceBack, StepObjectFrames, SetVelocityFromAngle, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
FollowObject19ThenLaunch:
    ldy #$19
    jsr VecCopyObjectPlaceBack
    lda #$16
    sta z:zTemp10
    lda #$BF
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne FollowObject19ThenLaunch_at0
    lda #$00
    sta z:zTemp00
    lda #$04
    sta z:zTemp01
    lda #$68
    jsr VecSetVelocityFromAngle
    lda #$02
    sta a:OBJ_3250,x
    inc a:OBJ_STATE,x
FollowObject19ThenLaunch_at0:
    rts

; @routine 025C:757 StepListThenSpawnUntilHurt
; @run 70:BF57
; WHAT IT DOES: Steps the animation list at $BF8E, moves the object and may spawn a piece (SpawnType30EveryFourthFrame); once bit 7 of OBJ_31A0 is set it takes the speed X $00C0, Y -$0300, clears OBJ_3250 and the animation and goes to the next state.
; HOW: StepObjectFrames, ApplyVelocity, a call, a test, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenSpawnUntilHurt:
    lda #$8E
    sta z:zTemp10
    lda #$BF
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    jsr SpawnType30EveryFourthFrame
    lda a:OBJ_31A0,x
    and #$80
    beq StepListThenSpawnUntilHurt_at0
    lda #$00
    sta a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_3250,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
StepListThenSpawnUntilHurt_at0:
    rts

; @routine 025C:797 SpawnType30EveryFourthFrame
; @run 70:BF97
; WHAT IT DOES: When the wave counter's low two bits are 0, starts an object of type $30 in a free slot of $20 to $3F at the object's place.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
SpawnType30EveryFourthFrame:
    lda z:zWaveCounter
    and #$03
    bne SpawnType30EveryFourthFrame_at0
    jsr VecFindFreeSlotFrom20
    bcc SpawnType30EveryFourthFrame_at0
    lda #$30
    jsr VecStartObjectAtPlace
SpawnType30EveryFourthFrame_at0:
    rts

; @routine 025C:7A8 StepListSlowSideways
; @run 70:BFA8
; WHAT IT DOES: Steps the animation list at $BFDC; the X speed loses $0008 and the Y speed gains $0020 a frame; the object moves and may spawn a piece (SpawnType2FAtWaveMarks).
; HOW: StepObjectFrames, two 16-bit adds in memory, ApplyVelocity, a call.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the call.
StepListSlowSideways:
    lda #$DC
    sta z:zTemp10
    lda #$BF
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_VX_LO,x
    sec
    sbc #$08
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    sbc #$00
    sta a:OBJ_VX_HI,x
    lda a:OBJ_VY_LO,x
    clc
    adc #$20
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr SpawnType2FAtWaveMarks
    rts

; @routine 025C:7ED SpawnType2FAtWaveMarks
; @run 70:BFED
; WHAT IT DOES: When the wave counter (low six bits) equals one of the eight marks at $C028, starts an object of type $2F in a free slot of $20 to $3F, plays sound $18 and gives the new object the speed from the table at $C030 (two bytes a mark), then moves it once.
; HOW: A loop over the marks, FindFreeSlotFrom20, StartObjectAtPlace, SoundCommand, ApplyVelocity with the slots swapped.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. zD9 and zDA keep their address form until they have names.
SpawnType2FAtWaveMarks:
    lda z:zWaveCounter
    and #$3F
    cly
    cmp a:$C028,y
    beq SpawnType2FAtWaveMarks_at0
    iny
    cpy #$08
    bcc SelectRasterStage1
    rts
SpawnType2FAtWaveMarks_at0:
    stx z:zRichterObject
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom20
    bcc SpawnType2FAtWaveMarks_at1
    lda #$2F
    jsr VecStartObjectAtPlace
    lda #$18
    jsr VecSoundCommand
    lda z:zHitSlot
    asl a
    tax
    lda a:$C030,x
    sta a:OBJ_VX_HI,y
    lda a:$C031,x
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zRichterObject
SpawnType2FAtWaveMarks_at1:
    rts
