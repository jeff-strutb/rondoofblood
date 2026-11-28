; @routine 05CB:0E3 LoadRecordThenRecordEventAtFifth
; @run 70:B0E3
; WHAT IT DOES: In state 0 loads the record and sets sprite set 7, frame $12 and the hit height $14. Later it steps the animation list at $B136; once the object has taken 5 hits it records its event, takes the kind of OBJ_START_ARG, sets OBJ_INVULN_TIMER $10, goes to state 0 and starts an object of type $9C from a free slot of $50 to $53 with the Y speed -$0600, moved once.
; HOW: LoadObjectRecord, StepObjectFrames, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace, ApplyVelocityY with X and Y swapped.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
LoadRecordThenRecordEventAtFifth:
    lda a:OBJ_STATE,x
    bne LoadRecordThenRecordEventAtFifth_at0
    jsr VecLoadObjectRecord
    lda #$07
    sta a:OBJ_SPRITE_SET,x
    lda #$12
    sta a:OBJ_FRAME,x
    lda #$14
    sta a:OBJ_HITBOX_HALF_H,x
    rts
LoadRecordThenRecordEventAtFifth_at0:
    lda #$B1
    sta z:zTemp11
    lda #$36
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc StepListByTimerWithGravity
    jsr VecRecordObjectEvent
    lda a:OBJ_START_ARG,x
    sta a:OBJ_KIND,x
    lda #$10
    sta a:OBJ_INVULN_TIMER,x
    stz a:OBJ_STATE,x
    jsr VecFindFreeSlotFrom50
    bcc StepListByTimerWithGravity
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$FA
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocityY
    stz a:OBJ_VY_HI,x
    sxy
    rts

; @routine 05CB:1A9 FlagThenHitsCompare
; @run 70:B1A9
; WHAT IT DOES: Tests bit 7 of OBJ_HIT_STATUS and, when it is clear, compares the hits taken with 5; nothing is stored.
; HOW: An AND and a compare.
; WAITS: no.
; Entry: X object. Exit: the AND, or the compare.
FlagThenHitsCompare:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    bne FlagThenHitsCompare_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc FlagThenHitsCompare_at0
FlagThenHitsCompare_at0:
    rts

; @routine 05CB:1B8 EnterState5WhenFlagged
; @run 70:B1B8
; WHAT IT DOES: When bit 7 of OBJ_HIT_STATUS is set and the state is below 5, sets the state 5 and OBJ_SUBSTATE 0.
; HOW: Two tests, two stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The stores ran in no script; the draft stops there.
EnterState5WhenFlagged:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq PickPhaseAtRandomA
    lda a:OBJ_STATE,x
    cmp #$05
    bcs PickPhaseAtRandomA
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 05CB:1ED LoadRecordStartTwoPieces
; @run 70:B1ED
; WHAT IT DOES: Loads the object's record, clears slots $11 and $12 and starts an object of type $26 in each at the object's place; OBJ_3510 $40.
; HOW: LoadObjectRecord, ClearObjectInY and StartObjectAtPlace twice, a store.
; WAITS: no.
; Entry: X object. Exit: A $40, Y $12, C from the record.
LoadRecordStartTwoPieces:
    jsr VecLoadObjectRecord
    ldy #$11
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    iny
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    lda #$40
    sta a:OBJ_3510,x
    rts

; @routine 05CB:232 SetUpEntryFromLeft
; @run 70:B232
; WHAT IT DOES: Places the object at X $FFF0, Y $0030 with the speed X $00C0 and goes to the next OBJ_SUBSTATE.
; HOW: Stores and an INC.
; WAITS: no.
; Entry: X object. Exit: A $C0, N and Z from the new OBJ_SUBSTATE.
SetUpEntryFromLeft:
    lda #$FF
    sta a:OBJ_X_HI,x
    lda #$F0
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$30
    sta a:OBJ_Y_LO,x
    stz a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 05CB:250 StepListMoveXThenTurnUp
; @run 70:B250
; WHAT IT DOES: Steps the animation list at $B289 and moves the object along X; once it is at X $0120 or more it takes frame 0, the speed X -$0400 and Y $0500, clears the animation and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, ApplyVelocityX, two compares, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListMoveXThenTurnUp:
    lda #$89
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityX
    lda a:OBJ_X_HI,x
    cmp #$01
    bne StepListMoveXThenTurnUp_at0
    lda a:OBJ_X_LO,x
    cmp #$20
    bcc StepListMoveXThenTurnUp_at0
    stz a:OBJ_FRAME,x
    lda #$FC
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$05
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
StepListMoveXThenTurnUp_at0:
    rts

; @routine 05CB:296 WaitForScreenByte2
; @run 70:B296
; WHAT IT DOES: When the scroll byte ScrollXHiScreen is 2 sets OBJ_3510 $60 and goes to the next OBJ_SUBSTATE.
; HOW: A compare, a store, an INC.
; WAITS: no.
; Entry: X object. Exit: the compare, or the new OBJ_SUBSTATE's N and Z (A $60).
WaitForScreenByte2:
    lda a:ScrollXHiScreen
    cmp #$02
    bne WaitForScreenByte2_at0
    lda #$60
    sta a:OBJ_3510,x
    inc a:OBJ_SUBSTATE,x
WaitForScreenByte2_at0:
    rts

; @routine 05CB:2A6 CountDownThenSound84AndAppear
; @run 70:B2A6
; WHAT IT DOES: Counts OBJ_3510 down; at zero plays sound $84, places the object at X $0110, Y $0010 and goes to the next OBJ_SUBSTATE.
; HOW: A memory decrement, SoundCommand, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
CountDownThenSound84AndAppear:
    dec a:OBJ_3510,x
    bne CountDownThenSound84AndAppear_at0
    lda #$84
    jsr VecSoundCommand
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$10
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$10
    sta a:OBJ_Y_LO,x
    inc a:OBJ_SUBSTATE,x
CountDownThenSound84AndAppear_at0:
    rts

; @routine 05CB:2C6 StepListFallAndSwing
; @run 70:B2C6
; WHAT IT DOES: Steps the animation list at $B30B and moves the object; its X speed gains $0010 and its Y speed loses $0020 a frame; once its Y high byte is not 0 and its Y low byte is below $C0 it sets OBJ_3510 $40 and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, ApplyVelocity, two 16-bit adds in memory, two tests.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListFallAndSwing:
    lda #$0B
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_VX_LO,x
    clc
    adc #$10
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc #$00
    sta a:OBJ_VX_HI,x
    lda a:OBJ_VY_LO,x
    sec
    sbc #$20
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_HI,x
    beq StepListFallAndSwing_at0
    lda a:OBJ_Y_LO,x
    cmp #$C0
    bcs StepListFallAndSwing_at0
    lda #$40
    sta a:OBJ_3510,x
    inc a:OBJ_SUBSTATE,x
StepListFallAndSwing_at0:
    rts

; @routine 05CB:316 CountDownThenLeaveRight
; @run 70:B316
; WHAT IT DOES: Counts OBJ_3510 down; at zero sets OBJ_3250 2, places the object at X $00F0, gives it the speed X -$00E0 and Y $0200, clears the animation and goes to the next OBJ_SUBSTATE.
; HOW: A memory decrement, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
CountDownThenLeaveRight:
    dec a:OBJ_3510,x
    bne CountDownThenLeaveRight_at0
    lda #$02
    sta a:OBJ_3250,x
    stz a:OBJ_X_HI,x
    lda #$F0
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    lda #$20
    sta a:OBJ_VX_LO,x
    lda #$02
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
CountDownThenLeaveRight_at0:
    rts

; @routine 05CB:344 StepListFallThenMusicAgain
; @run 70:B344
; WHAT IT DOES: Plays the sound every 32 frames (SoundEvery32Frames), steps the animation list at $B375 and moves; once the Y high byte is 0 and the Y low byte is $40 or more it clears bits 3 and 4 of zField8E, plays the track again (CdPlayAgain), sets bit 2 of zField8E, goes to the next state and clears OBJ_SUBSTATE.
; HOW: SoundEvery32Frames, StepObjectFrames, ApplyVelocity, RMW of zField8E, CdPlayAgain.
; WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListFallThenMusicAgain:
    jsr SoundEvery32Frames
    lda #$75
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne StepListFallThenMusicAgain_at0
    lda a:OBJ_Y_LO,x
    cmp #$40
    bcc StepListFallThenMusicAgain_at0
    rmb3 z:zField8E
    rmb4 z:zField8E
    jsr VecCdPlayAgain
    lda z:zField8E
    ora #$04
    sta z:zField8E
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StepListFallThenMusicAgain_at0:
    rts

; @routine 05CB:3A1 TurnTowardCenterOrWait
; @run 70:B3A1
; WHAT IT DOES: Clears the animation, goes to the next OBJ_SUBSTATE, sets OBJ_3510 to 2 to 5 at random and OBJ_3618 to 0 or 1 by whether it is outside the center band; when the object's facing already matches FacingByX goes on once more.
; HOW: NextRandom, InsideCenterBand, FacingByX, a ROL, a compare.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
TurnTowardCenterOrWait:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    jsr VecNextRandom
    and #$03
    clc
    adc #$02
    sta a:OBJ_3510,x
    jsr InsideCenterBand
    rol a
    and #$01
    sta a:OBJ_3618,x
    jsr FacingByX
    cmp a:OBJ_FACING_LEFT,x
    bne TurnTowardCenterOrWait_at0
    inc a:OBJ_SUBSTATE,x
TurnTowardCenterOrWait_at0:
    rts

; @routine 05CB:3CA InsideCenterBand
; @run 70:B3CA
; WHAT IT DOES: Says whether the object's X is inside the band from $0038 to $00C7 on the first screen: C 0 when it is, C 1 when not.
; HOW: A test of the high byte and two compares.
; WAITS: no.
; Entry: X object. Exit: C 0 inside, 1 outside; A and N, Z from the last load or compare.
InsideCenterBand:
    lda a:OBJ_X_HI,x
    bne EnterState4IfFlagged
    lda a:OBJ_X_LO,x
    cmp #$38
    bcc EnterState4IfFlagged
    cmp #$C8
    bcs EnterState4IfFlagged
    clc
    rts
    sec
    rts

; @routine 05CB:3DE FacingByX
; @run 70:B3DE
; WHAT IT DOES: A 1 when the object's X (first screen) is below $80, a 0 when it is $80 or more; off the first screen it gives the side of the player's bit 0 instead.
; HOW: Two ROLs and an EOR on the X low byte, or SideOfPlayer and an AND.
; WAITS: no.
; Entry: X object, C kept into the first ROL. Exit: A the answer; C bit 6 of the X low byte on the first screen.
FacingByX:
    lda a:OBJ_X_HI,x
    bne StepFramesFacingPlayer
    lda a:OBJ_X_LO,x
    rol a
    rol a
    and #$01
    eor #$01
    rts
    jsr VecSideOfPlayer
    and #$01
    rts

; @routine 05CB:3F3 StepListThenTurnAround
; @run 70:B3F3
; WHAT IT DOES: Steps the animation list at $B411; when it ends sets frame 5, turns the object round and goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, a store, an EOR, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenTurnAround:
    lda #$11
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListThenTurnAround_at0
    lda #$05
    sta a:OBJ_FRAME,x
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_FACING_LEFT,x
    inc a:OBJ_SUBSTATE,x
StepListThenTurnAround_at0:
    rts

; @routine 05CB:41C StepSideToSideWithCenterCheck
; @run 70:B41C
; WHAT IT DOES: Sets the X speed from a table at $B462 by the object's facing and moves along X while it steps the animation list at $B375; when the list ends it asks EdgeCodeByDirection: 0 leaves it, 1 sets OBJ_3510 1 and advances OBJ_SUBSTATE (and once more when the side of the player matches the facing), 2 counts OBJ_3510 down first.
; HOW: Table reads, ApplyVelocityX, StepObjectFrames, EdgeCodeByDirection, SideOfPlayer.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepSideToSideWithCenterCheck:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:$B462,y
    sta a:OBJ_VX_HI,x
    lda a:$B462,y
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    lda #$75
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepSideToSideWithCenterCheck_at0
    jsr EdgeCodeByDirection
    beq StepSideToSideWithCenterCheck_at0
    cmp #$01
    beq StepSideToSideWithCenterCheck_at1
    dec a:OBJ_3510,x
    bne StepSideToSideWithCenterCheck_at0
StepSideToSideWithCenterCheck_at1:
    lda #$01
    sta a:OBJ_3510,x
    inc a:OBJ_SUBSTATE,x
    jsr VecSideOfPlayer
    and #$01
    cmp a:OBJ_FACING_LEFT,x
    bne StepSideToSideWithCenterCheck_at0
    inc a:OBJ_SUBSTATE,x
StepSideToSideWithCenterCheck_at0:
    rts

; @routine 05CB:465 EdgeCodeByDirection
; @run 70:B465
; WHAT IT DOES: Gives 2 when the object is inside the center band (InsideCenterBand says C 0), else 1 when OBJ_3618 is 0 and 0 when it is not.
; HOW: A load and a call to InsideCenterBand, a load of the answer.
; WAITS: no.
; Entry: X object. Exit: A the answer, N and Z as the load. The arm for OBJ_3618 not 0 outside the band ran in no script.
EdgeCodeByDirection:
    lda a:OBJ_3618,x
    beq EdgeCodeByDirection_at0
    jsr InsideCenterBand
    bcs EdgeCodeByDirection_at1
    lda #$02
    rts
EdgeCodeByDirection_at1:
    cla
    rts
EdgeCodeByDirection_at0:
    jsr InsideCenterBand
    bcs EdgeCodeByDirection_at2
    lda #$02
    rts
EdgeCodeByDirection_at2:
    lda #$01
    rts

; @routine 05CB:49D StepListThenCountDownToState4
; @run 70:B49D
; WHAT IT DOES: Steps the animation list at $B375; when it ends counts OBJ_3510 down and at zero goes to state 4 with OBJ_SUBSTATE 0.
; HOW: StepObjectFrames, a memory decrement, two stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenCountDownToState4:
    lda #$75
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListThenCountDownToState4_at0
    dec a:OBJ_3510,x
    bne StepListThenCountDownToState4_at0
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StepListThenCountDownToState4_at0:
    rts

; @routine 05CB:4D6 PickNextPhaseFromTable
; @run 70:B4D6
; WHAT IT DOES: Clears the animation and sets OBJ_SUBSTATE from the table at $B4EC by the low three bits of OBJ_3568, then advances OBJ_3568.
; HOW: Stores, a table read, an INC.
; WAITS: no.
; Entry: X object. Exit: A the table byte, Y the index, N and Z from the new OBJ_3568.
PickNextPhaseFromTable:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_3568,x
    and #$07
    tay
    lda a:$B4EC,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_3568,x
    rts

; @routine 05CB:4F4 StepListThenLeapWithSound
; @run 70:B4F4
; WHAT IT DOES: Steps the animation list at $B525; when it ends plays sound $85 and gives the object a leap: the X speed from the table at $B532 by its facing and the Y speed $0600; goes to the next OBJ_SUBSTATE.
; HOW: StepObjectFrames, SoundCommand, table reads, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenLeapWithSound:
    lda #$25
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListThenLeapWithSound_at0
    lda #$85
    jsr VecSoundCommand
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:$B533,y
    sta a:OBJ_VX_HI,x
    lda a:$B532,y
    sta a:OBJ_VX_LO,x
    lda #$06
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
StepListThenLeapWithSound_at0:
    rts

; @routine 05CB:536 FlyAndSagWithTwoLists
; @run 70:B536
; WHAT IT DOES: Clears OBJ_3358 and steps one of two animation lists (at $B575, by the sign of the Y speed) while moving; the Y speed loses $0030 a frame; when it is negative and the Y low byte is below $78 sets OBJ_3358 2 and goes to the next OBJ_SUBSTATE.
; HOW: A store, two ROLs, a table read, StepObjectFrames, ApplyVelocity, a 16-bit subtract in memory, a compare.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
FlyAndSagWithTwoLists:
    stz a:OBJ_3358,x
    lda a:OBJ_VY_HI,x
    rol a
    rol a
    and #$01
    asl a
    tay
    lda a:$B575,y
    sta z:zTemp10
    lda a:$B576,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    sec
    sbc #$30
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    bpl FlyAndSagWithTwoLists_at0
    lda a:OBJ_Y_LO,x
    cmp #$78
    bcs FlyAndSagWithTwoLists_at0
    lda #$02
    sta a:OBJ_3358,x
    inc a:OBJ_SUBSTATE,x
FlyAndSagWithTwoLists_at0:
    rts

; @routine 05CB:58D SoundEvery32Frames
; @run 70:B58D
; WHAT IT DOES: When the low five bits of OBJ_34B8 are 0 plays sound $29.
; HOW: An AND, SoundCommand.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: the AND, or the sound call's.
SoundEvery32Frames:
    lda a:OBJ_34B8,x
    and #$1F
    bne SoundEvery32Frames_at0
    lda #$29
    jsr VecSoundCommand
SoundEvery32Frames_at0:
    rts

; @routine 05CB:59A StepListSlowDownThenPickRandom
; @run 70:B59A
; WHAT IT DOES: Steps the animation list at $B580 and moves the object; every eighth frame of OBJ_34B8 halves its speed and, once both speed bytes are 0 or both $FF, goes on to PickRandomThenState3.
; HOW: StepObjectFrames, ApplyVelocity, HalveObjectVelocity, tests of OBJ_VY_HI and OBJ_VY_LO.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListSlowDownThenPickRandom:
    lda #$80
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_34B8,x
    and #$07
    bne $B5D8
    jsr $B5D9
    lda a:OBJ_VY_HI,x
    ora a:OBJ_VY_LO,x
    beq PickRandomThenState3
    lda a:OBJ_VY_HI,x
    and a:OBJ_VY_LO,x
    cmp #$FF
    bne $B5D8

; @routine 05CB:5C4 PickRandomThenState3
; @run 70:B5C4
; WHAT IT DOES: Takes a random number (kept in Y as 0 to 3), clears OBJ_SUBSTATE and the animation and goes to state 3.
; HOW: NextRandom, an AND, stores.
; WAITS: no.
; Entry: X object. Exit: A 3, Y the random number masked, C and V from NextRandom.
PickRandomThenState3:
    jsr VecNextRandom
    and #$03
    tay
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    rts

; @routine 05CB:5EE StepListThenSoundAndSpawn
; @run 70:B5EE
; WHAT IT DOES: Steps the animation list at $B62B; when it ends goes to PickRandomThenState3; when the graphic changes at animation step $0A plays sound $2A; from step $0A on, every fourth frame of OBJ_34B8, starts an object of type $25 in a free slot of $20 to $3F facing as this one.
; HOW: StepObjectFrames, SoundCommand, FindFreeSlotFrom20, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenSoundAndSpawn:
    lda #$2B
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListThenSoundAndSpawn_at0
    jmp PickRandomThenState3
StepListThenSoundAndSpawn_at0:
    bcc StepListThenSoundAndSpawn_at1
    lda a:OBJ_ANIM_STEP,x
    cmp #$0A
    bne StepListThenSoundAndSpawn_at1
    lda #$2A
    jsr VecSoundCommand
StepListThenSoundAndSpawn_at1:
    lda a:OBJ_ANIM_STEP,x
    cmp #$0A
    bcc StepListThenSoundAndSpawn_at2
    lda a:OBJ_34B8,x
    and #$03
    bne StepListThenSoundAndSpawn_at2
    jsr VecFindFreeSlotFrom20
    bcc StepListThenSoundAndSpawn_at2
    lda #$25
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
StepListThenSoundAndSpawn_at2:
    rts

; @routine 05CB:648 StepListThenBurstAtFrame14
; @run 70:B648
; WHAT IT DOES: Steps the animation list at $B675; when it ends goes to PickRandomThenState3; when the graphic changes and the frame is $14 plays sound $86 and starts an object of type $28 in slot $13 facing as this one.
; HOW: StepObjectFrames, SoundCommand, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenBurstAtFrame14:
    lda #$75
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepListThenBurstAtFrame14_at0
    bcc StepListThenBurstAtFrame14_at1
    lda a:OBJ_FRAME,x
    cmp #$14
    bne StepListThenBurstAtFrame14_at1
    lda #$86
    jsr VecSoundCommand
    ldy #$13
    lda #$28
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    rts
StepListThenBurstAtFrame14_at0:
    jmp PickRandomThenState3
StepListThenBurstAtFrame14_at1:
    rts
