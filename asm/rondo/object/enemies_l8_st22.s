; @routine 0F6B:45D StepListThenStartFivePieces
; @run 70:B45D
; WHAT IT DOES: Runs StepListByNumberTable; when the list ends starts objects of type $A9 in the slots $11 to $15 (each cleared first) at the object's place, numbered 0 to 4 in OBJ_3040, sets frame 2 and OBJ_3510 $C0 and goes to the next state.
; HOW: StepListByNumberTable, a loop of ClearObjectInY and StartObjectAtPlace, a counter in zTemp00.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenStartFivePieces:
    jsr StepListByNumberTable
    bne SwingByCosineFromState0B
    stz z:zTemp00
    ldy #$11
StepListThenStartFivePieces_at0:
    jsr VecClearObjectInY
    lda #$A9
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_3040,y
    inc z:zTemp00
    iny
    cpy #$16
    bne StepListThenStartFivePieces_at0
    lda #$02
    sta a:OBJ_2A68,x
    lda #$C0
    sta a:OBJ_3510,x
    inc a:OBJ_27A8,x
    rts

; @routine 0F6B:4C9 StepListThenRunPhaseTable
; @run 70:B4C9
; WHAT IT DOES: Runs StepListByNumberTable and counts OBJ_3568 down; at zero tries StartPieceABInFreeSlot and, when it started one, keeps OBJ_2800 in OBJ_38D8, clears it, sets OBJ_3510 $40 and takes OBJ_3568 from OBJ_35C0. Then runs the code the table at $B501 names for OBJ_2800 (50B, 51C, 52F, 59E, 5B1).
; HOW: Two calls, a table of two bytes to zTemp00-01, a jump through them.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
StepListThenRunPhaseTable:
    jsr StepListByNumberTable
    lda a:OBJ_3568,x
    beq PickNextPhaseFromTable
    dec a:OBJ_3568,x
    bne StepListThenRunPhaseTable_at0
    jsr StartPieceABInFreeSlot
    bcc StepListThenRunPhaseTable_at0
    lda a:OBJ_2800,x
    sta a:OBJ_38D8,x
    stz a:OBJ_2800,x
    lda #$40
    sta a:OBJ_3510,x
    lda a:OBJ_35C0,x
    sta a:OBJ_3568,x
StepListThenRunPhaseTable_at0:
    lda a:OBJ_2800,x
    asl a
    tay
    lda a:$B501,y
    sta z:zTemp00
    lda a:$B502,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0F6B:52F OrbitCosineSineThenSettle
; @run 70:B52F
; WHAT IT DOES: Moves the object on an ellipse: X speed from the cosine of OBJ_37D0 scaled by $0120, Y speed from the sine scaled by $0050; turns OBJ_37D0 on by -$0070 a frame (a 16-bit add with OBJ_3778); when OBJ_37D0 reaches $80 it is placed at Y $0040, X $B0 less the scroll and goes to the next OBJ_2800.
; HOW: ScaleByCosine, ScaleBySine, ApplyVelocityX and Y, a 16-bit add in memory, two compares.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
OrbitCosineSineThenSettle:
    lda #$20
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_37D0,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    lda #$50
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_37D0,x
    jsr VecScaleBySine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda a:OBJ_37D0,x
    sta z:zTemp00
    lda #$90
    clc
    adc a:OBJ_3778,x
    sta a:OBJ_3778,x
    lda #$FF
    adc a:OBJ_37D0,x
    sta a:OBJ_37D0,x
    cmp z:zTemp00
    beq OrbitCosineSineThenSettle_at0
    cmp #$80
    bne OrbitCosineSineThenSettle_at0
    lda #$40
    sta a:OBJ_Y_LO,x
    stz a:OBJ_Y_FRAC,x
    stz a:OBJ_3778,x
    lda #$B0
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    stz a:OBJ_X_FRAC,x
    inc a:OBJ_2800,x
OrbitCosineSineThenSettle_at0:
    rts

; @routine 0F6B:59E ResetOrbitAndAdvance
; @run 70:B59E
; WHAT IT DOES: Clears OBJ_3778, sets OBJ_37D0 and OBJ_35C0 $80 and goes to the next OBJ_2800.
; HOW: Stores and an INC.
; WAITS: no.
; Entry: X object. Exit: A $80, N and Z from the new OBJ_2800.
ResetOrbitAndAdvance:
    lda #$00
    sta a:OBJ_3778,x
    lda #$80
    sta a:OBJ_37D0,x
    lda #$80
    sta a:OBJ_35C0,x
    inc a:OBJ_2800,x
    rts

; @routine 0F6B:5B1 OrbitWithRisingSineThenSettle
; @run 70:B5B1
; WHAT IT DOES: As OrbitCosineSineThenSettle, but the Y speed comes from the sine of OBJ_3828 scaled by $0100, which turns on by $01C0 a frame; at $80 the object goes to OBJ_2800 2.
; HOW: ScaleBySine, ScaleByCosine, ApplyVelocityY and X, two 16-bit adds in memory, two compares.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
OrbitWithRisingSineThenSettle:
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_3828,x
    jsr VecScaleBySine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda #$C0
    clc
    adc a:OBJ_3880,x
    sta a:OBJ_3880,x
    lda #$01
    adc a:OBJ_3828,x
    sta a:OBJ_3828,x
    lda #$20
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_37D0,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    lda a:OBJ_37D0,x
    sta z:zTemp00
    lda #$90
    clc
    adc a:OBJ_3778,x
    sta a:OBJ_3778,x
    lda #$FF
    adc a:OBJ_37D0,x
    sta a:OBJ_37D0,x
    cmp z:zTemp00
    beq SoundEveryFourthFrameB
    cmp #$80
    bne SoundEveryFourthFrameB
    lda #$40
    sta a:OBJ_Y_LO,x
    stz a:OBJ_Y_FRAC,x
    stz a:OBJ_3778,x
    lda #$B0
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    stz a:OBJ_X_FRAC,x
    lda #$02
    sta a:OBJ_2800,x
    rts

; @routine 0F6B:70B ClearAnimationThenStepListByNumber
; @run 70:B70B
; WHAT IT DOES: Clears the animation step and timer and runs StepListByNumberTable.
; HOW: Two stores, running into the next routine.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as StepListByNumberTable.
ClearAnimationThenStepListByNumber:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x

; @routine 0F6B:711 StepListByNumberTable
; @run 70:B711
; WHAT IT DOES: Steps the animation list that the table at $BA0A names for OBJ_34B8.
; HOW: A table read of two bytes into zTemp10-11, a tail jump into StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step.
StepListByNumberTable:
    lda a:OBJ_34B8,x
    asl a
    tay
    lda a:$BA0A,y
    sta z:zTemp10
    lda a:RichterItemThirdCosts,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0F6B:723 StartPieceABInFreeSlot
; @run 70:B723
; WHAT IT DOES: Starts an object of type $AB in a free slot of $30 to $3F at the object's place and says yes (C 1); with no free slot says no (C 0).
; HOW: FindFreeSlotFrom30, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: C 1 started, 0 none.
StartPieceABInFreeSlot:
    jsr VecFindFreeSlotFrom30
    bcc StartPieceABInFreeSlot_at0
    lda #$AB
    jsr VecStartObjectAtPlace
    sec
StartPieceABInFreeSlot_at0:
    rts

; @routine 0F6B:72F LoadRecordFrameByKindThenFall
; @run 70:B72F
; WHAT IT DOES: In state 0 loads the record, sets sprite set 4 and takes the frame and OBJ_3510 from tables at $B76C and $B771 by OBJ_3040. Later, while OBJ_3510 counts down, kind 4 sets bit 4 of zField8E and plays sound $FE at zero; at zero it moves in Y and falls.
; HOW: LoadObjectRecord, table reads, a memory decrement, SoundCommand, ApplyVelocityY, ApplyGravity.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
LoadRecordFrameByKindThenFall:
    lda a:OBJ_27A8,x
    bne LoadRecordFrameByKindThenFall_at0
    jsr VecLoadObjectRecord
    lda #$04
    sta a:OBJ_2A10,x
    ldy a:OBJ_3040,x
    lda a:$B76C,y
    sta a:OBJ_2A68,x
    lda a:$B771,y
    sta a:OBJ_3510,x
    rts
LoadRecordFrameByKindThenFall_at0:
    lda a:OBJ_3510,x
    beq SoundCommandReturn
    dec a:OBJ_3510,x
    bne LoadRecordFrameByKindThenFall_at1
    lda a:OBJ_3040,x
    cmp #$04
    bne LoadRecordFrameByKindThenFall_at1
    smb4 z:zField8E
    lda #$FE
    jsr VecSoundCommand
LoadRecordFrameByKindThenFall_at1:
    rts
    jsr VecApplyVelocityY
    jsr VecApplyGravity
    rts
