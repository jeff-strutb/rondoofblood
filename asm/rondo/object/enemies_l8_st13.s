; @routine 025C:131 StepListThenStartChannel85
; @run 70:B931
; WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero starts list $85 on channel 5, sets OBJ_3670 5, OBJ_3510 $7B, the byte before the frame $2F and goes to the next OBJ_2800.
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
    inc a:OBJ_2800,x
StepListThenStartChannel85_at0:
    rts

; @routine 025C:15D StepListThenStopAnimation
; @run 70:B95D
; WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears OBJ_3670 and the animation step and timer and goes to the next OBJ_2800.
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
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_2800,x
StepListThenStopAnimation_at0:
    rts

; @routine 025C:17A StepListB995ThenWait
; @run 70:B97A
; WHAT IT DOES: Steps the animation list at $B995; when it ends sets OBJ_3670 6 and OBJ_3510 $3A and goes to the next OBJ_2800.
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
    inc a:OBJ_2800,x
StepListB995ThenWait_at0:
    rts

; @routine 025C:19C StepListThenStartType2D
; @run 70:B99C
; WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears slot $50, starts an object of type $2D there at the object's place, sets OBJ_3670 7 and OBJ_3510 $83 and goes to the next OBJ_2800.
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
    inc a:OBJ_2800,x
StepListThenStartType2D_at0:
    rts

; @routine 025C:1C4 StepListThenClearAnimation
; @run 70:B9C4
; WHAT IT DOES: Steps the animation list at $B6DA; when OBJ_3510 counts down to zero clears the animation step and timer and OBJ_3670 and goes to the next OBJ_2800.
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
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    stz a:OBJ_3670,x
    inc a:OBJ_2800,x
StepListThenClearAnimation_at0:
    rts

; @routine 025C:1E1 StepListThenStartType2E
; @run 70:B9E1
; WHAT IT DOES: Steps the animation list at $BA0E; when it ends clears OBJ_3720, sets OBJ_3670 8, OBJ_34B8 1 and OBJ_3510 $80, goes to the next OBJ_2800, clears slot $30 and starts an object of type $2E there at the object's place.
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
    inc a:OBJ_2800,x
    ldy #$30
    jsr VecClearObjectInY
    lda #$2E
    jsr VecStartObjectAtPlace
    rts

; @routine 025C:215 StepListThenSoundAndStartChannel86
; @run 70:BA15
; WHAT IT DOES: Steps the animation list at $B6DA; when the 16-bit timer OBJ_34B8:OBJ_3510 counts down to zero starts list $86 on channel 5, plays sound $76, clears the animation, OBJ_35C0, OBJ_38D8 and OBJ_3670, sets OBJ_3880 2 and goes to the next OBJ_2800.
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
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    stz a:OBJ_35C0,x
    lda #$02
    sta a:OBJ_3880,x
    stz a:OBJ_38D8,x
    stz a:OBJ_3670,x
    inc a:OBJ_2800,x
StepListThenSoundAndStartChannel86_at0:
    rts

; @routine 025C:25C SpiralOutWhileStepping
; @run 70:BA5C
; WHAT IT DOES: Steps the animation list at $BA9A while moving the object by the speed made from OBJ_35C0 and the radius OBJ_3880:OBJ_38D8; the radius shrinks by one and OBJ_35C0 turns on by 4; when OBJ_35C0 wraps it goes to the next OBJ_2800.
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
    inc a:OBJ_2800,x
SpiralOutWhileStepping_at0:
    rts

; @routine 025C:2A7 MoveXUntilPastLeftEdge
; @run 70:BAA7
; WHAT IT DOES: Moves the object along X by its speed; once its X high byte is not 0 and its low byte is below $80 sets OBJ_3510 $20 and goes to the next OBJ_2800.
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
    inc a:OBJ_2800,x
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
    lda a:OBJ_27A8,x
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
    sta a:OBJ_27A8,x
SoundWhenHurtInStatesOneToThree_at0:
    rts
SoundWhenHurtInStatesOneToThree_at2:
    lda #$43
    jsr VecSoundCommand
    lda #$05
    sta a:OBJ_27A8,x
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
