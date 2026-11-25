; @routine 0F6C:0D5 SpawnFlashingPieceWhenRising
; @run 70:B8D5
; WHAT IT DOES: A rising object (Y speed high bit clear) shows flash flags $10; every fourth frame it starts an object of type $9B in a free slot of $20 to $3F, with Y speed $0800 (moved once) and an X speed from the table at $B90D by its own OBJ_3040 (moved once) and flash flags $10.
; HOW: A test, TST of the wave counter, FindFreeSlotFrom20, StartObjectAtPlace, ApplyVelocityY and X with X and Y swapped.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
SpawnFlashingPieceWhenRising:
    lda a:OBJ_VY_HI,x
    bmi SpawnFlashingPieceWhenRising_at0
    lda #$10
    sta a:OBJ_2B18,x
    tst #$03,z:zWaveCounter
    bne SpawnFlashingPieceWhenRising_at0
    jsr VecFindFreeSlotFrom20
    bcc SpawnFlashingPieceWhenRising_at0
    lda #$9B
    jsr VecStartObjectAtPlace
    sxy
    lda #$08
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    phy
    lda a:OBJ_3040,y
    tay
    lda a:$B90D,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    ply
    lda #$10
    sta a:OBJ_2B18,x
    sxy
SpawnFlashingPieceWhenRising_at0:
    rts

; @routine 0F6C:111 CountDownThenClearOnTimer
; @run 70:B911
; WHAT IT DOES: Counts OBJ_3510 down; at zero clears the object.
; HOW: A memory decrement and a tail jump into ClearObject.
; WAITS: no.
; Entry: X object. Exit: the decrement's N and Z.
CountDownThenClearOnTimer:
    dec a:OBJ_3510,x
    bne $B8D4
    jmp VecClearObject

; @routine 0F6C:3F0 FlashWhenHurtThenSound
; @run 70:BBF0
; WHAT IT DOES: Unless bit 7 of OBJ_31A0 is set, in the states the table at $BBB0 marks it clears its flash flags, sets them $10 when OBJ_3A90 is not 0, copies OBJ_32A8 to slot $10's, and at 5 hits plays sound $D6, doubles OBJ_3A90 twice, goes to state $0D and clears OBJ_2800.
; HOW: Tests, a table read by the state, stores, SoundCommand, two ASLs in memory.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The arm past the table test ran in no script; the draft stops there.
FlashWhenHurtThenSound:
    lda a:OBJ_31A0,x
    and #$80
    bne FlashWhenHurtThenSound_at0
    ldy a:OBJ_27A8,x
    lda a:$BBB0,y
    beq FlashWhenHurtThenSound_at0
    stz a:OBJ_2B18,x
    lda a:OBJ_3A90,x
    beq LoadThenRandomPlace
    lda #$10
    sta a:OBJ_2B18,x
    lda a:OBJ_32A8,x
    sta a:$32B8
    lda a:OBJ_33B0,x
    cmp #$05
    bcc FlashWhenHurtThenSound_at0
    lda #$D6
    jsr VecSoundCommand
    asl a:OBJ_3A90,x
    asl a:OBJ_3A90,x
    lda #$0D
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
FlashWhenHurtThenSound_at0:
    rts

; @routine 0F6C:42D EnterState0EWhenFlagged
; @run 70:BC2D
; WHAT IT DOES: When bit 7 of OBJ_31A0 is set and the state is not $0E, copies OBJ_32A8 to slot $10's, clears OBJ_3250, goes to state $0E and clears OBJ_2800.
; HOW: Two tests, stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The arm past the flag test ran in no script; the draft stops there.
EnterState0EWhenFlagged:
    lda a:OBJ_31A0,x
    and #$80
    beq EnterState0EWhenFlagged_at0
    lda a:OBJ_27A8,x
    cmp #$0E
    beq EnterState0EWhenFlagged_at0
    lda a:OBJ_32A8,x
    sta a:$32B8
    stz a:OBJ_3250,x
    lda #$0E
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
EnterState0EWhenFlagged_at0:
    rts

; @routine 0F6C:44D LoadRecordFrame24
; @run 70:BC4D
; WHAT IT DOES: Loads the object's record, sets its frame $24 and clears OBJ_3250.
; HOW: LoadObjectRecord and two stores.
; WAITS: no.
; Entry: X object. Exit: A $24, Y 11, C from the record.
LoadRecordFrame24:
    jsr VecLoadObjectRecord
    lda #$24
    sta a:OBJ_2A68,x
    stz a:OBJ_3250,x
    rts

; @routine 0F6C:459 StartTimerWhenSlot10Ready
; @run 70:BC59
; WHAT IT DOES: When the state of slot $10 is 5 or more and its OBJ_32A8 is not $5C, sets OBJ_34B8 $0A and goes to the next state.
; HOW: Two compares, a store, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
StartTimerWhenSlot10Ready:
    lda a:$27B8
    cmp #$05
    bcc StepListFallThenCountLandings
    lda a:$32B8
    cmp #$5C
    beq StepListFallThenCountLandings
    lda #$0A
    sta a:OBJ_34B8,x
    inc a:OBJ_27A8,x
    rts

; @routine 0F6C:580 HurtOnceThenAnswerYes
; @run 70:BD80
; WHAT IT DOES: Says yes (C 1) when the object has taken exactly 1 hit, the player's Y low byte is not $9A and slot 0's state is below $10: it then clears its hits and OBJ_3250, sets slot 0's OBJ_3510 $40 and OBJ_3568 $10, OBJ_34B8 $0B, restarts its animation (ClearAnimationThenStepListC585), goes to state 8. Otherwise says no (C 0).
; HOW: Compares, stores, a call.
; WAITS: no.
; Entry: X object, Y kept for the call. Exit: C 1 yes, 0 no. The yes arm ran in no script; the draft stops there.
HurtOnceThenAnswerYes:
    lda a:OBJ_33B0,x
    cmp #$01
    bne HurtOnceThenAnswerYes_at0
    lda a:OBJ_Y_LO
    cmp #$9A
    beq HurtOnceThenAnswerYes_at0
    lda a:OBJ_27A8
    cmp #$10
    bcs HurtOnceThenAnswerYes_at0
    stz a:OBJ_33B0,x
    stz a:OBJ_3250,x
    lda #$40
    sta a:OBJ_3510
    lda #$10
    sta a:OBJ_3568
    lda #$0B
    sta a:OBJ_34B8,x
    jsr ClearAnimationThenStepListC585
    lda #$08
    sta a:OBJ_27A8,x
    sec
    rts
HurtOnceThenAnswerYes_at0:
    clc
    rts

; @routine 0F6D:35F ClearAnimationThenStepListC585
; @run 71:C35F
; WHAT IT DOES: Clears the animation step and timer and runs StepListByNumberTableC585.
; HOW: Two stores, running into the next routine.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as StepListByNumberTableC585.
ClearAnimationThenStepListC585:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x

; @routine 0F6D:365 StepListByNumberTableC585
; @run 71:C365
; WHAT IT DOES: Steps the animation list that the table at $C585 names for OBJ_34B8.
; HOW: A table read of two bytes into zTemp10-11, a tail jump into StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step.
StepListByNumberTableC585:
    lda a:OBJ_34B8,x
    asl a
    tay
    lda a:$C585,y
    sta z:zTemp10
    lda a:$C586,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0F6D:377 ToggleFacing
; @run 71:C377
; WHAT IT DOES: Turns the object round.
; HOW: An EOR of bit 0 and a store.
; WAITS: no.
; Entry: X object. Exit: A the new facing, N and Z from it.
ToggleFacing:
    lda a:OBJ_2AC0,x
    eor #$01
    and #$01
    sta a:OBJ_2AC0,x
    rts

; @routine 0F6D:3F3 LoadRecordOrBurstWhenFlagged
; @run 71:C3F3
; WHAT IT DOES: In state 0 loads the record. Later, when bit 7 of OBJ_31A0 is set, plays sound $16, clears slots $52 to $54 and starts objects of type $AE in $52 and $53 (the second with OBJ_3040 1) and of type $9C in $54 at its place, then clears itself.
; HOW: LoadObjectRecord, SoundCommand, ClearObjectInY and StartObjectAtPlace three times, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The burst ran in no script; the draft stops there.
LoadRecordOrBurstWhenFlagged:
    lda a:OBJ_27A8,x
    bne LoadRecordOrBurstWhenFlagged_at0
    jsr VecLoadObjectRecord
    rts
LoadRecordOrBurstWhenFlagged_at0:
    lda a:OBJ_31A0,x
    and #$80
    beq LoadRecordOrBurstWhenFlagged_at1
    lda #$16
    jsr VecSoundCommand
    ldy #$52
    jsr VecClearObjectInY
    lda #$AE
    jsr VecStartObjectAtPlace
    iny
    jsr VecClearObjectInY
    lda #$AE
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_3040,y
    iny
    jsr VecClearObjectInY
    lda #$9C
    jsr VecStartObjectAtPlace
    jmp VecClearObject
LoadRecordOrBurstWhenFlagged_at1:
    rts
