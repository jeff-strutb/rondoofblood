; @routine 05BD:006 StageTileReadVector9991
; @run 6B:8006
; WHAT IT DOES: Goes to the tile read at $9991 (ProbeTileRead9991).
; HOW: One jump. The twelve overlays of the group whose tile read is at $9991 hold this vector.
; WAITS: no.
; Entry, exit: as ProbeTileRead9991.
StageTileReadVector9991:
    jmp ProbeTileRead9991

; @routine 0F6C:024 DroppingPieceStep
; @run 70:B824
; WHAT IT DOES: A piece that falls and lands: its first frame loads its record and takes its animation, facing and speeds from
; tables by the record's kind; in state 1 each frame steps its list, may start a flashing piece, moves, falls faster (Y speed +$10)
; and, once falling, reads the tile $08 below it: when that tile has bit 0 the piece is set back out of the floor, may start a
; type $9B object in a free slot of $30 to $4F, and goes to state 2 with its timer 6; other states count the timer down.
; HOW: LoadObjectRecord, StepListByNumberTable, SpawnFlashingPieceWhenRising, ApplyVelocityX and Y, the tile read vector at $8006,
; FindFreeSlotFrom30 and StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless a step sets it. Exit: as the last instruction that ran.
DroppingPieceStep:
    lda a:OBJ_27A8,x
    bne DroppingPieceStep_at0
    jsr VecLoadObjectRecord
    ldy a:OBJ_3040,x
    lda a:SoundVoicesFirst+5,y
    sta a:OBJ_34B8,x
    lda a:$B91D,y
    sta a:OBJ_2AC0,x
    lda a:$B921,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    tya
    asl a
    tay
    lda a:$B925,y
    sta a:OBJ_VX_LO,x
    lda a:$B926,y
    sta a:OBJ_VX_HI,x
    lda a:$B92D,y
    sta a:OBJ_VY_LO,x
    lda a:$B92E,y
    sta a:OBJ_VY_HI,x
    rts
DroppingPieceStep_at0:
    dec a
    beq DroppingPieceStep_at1
    jmp CountDownThenClearOnTimer
DroppingPieceStep_at1:
    jsr StepListByNumberTable
    jsr SpawnFlashingPieceWhenRising
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    clc
    adc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_VY_HI,x
    bmi DroppingPieceStep_at2
    lda a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    clc
    adc #$08
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    adc #$00
    sta z:zTemp03
    jsr StageTileReadVector9991
    and #$01
    beq DroppingPieceStep_at2
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO,x
    jsr VecFindFreeSlotFrom30
    bcc DroppingPieceStep_at3
    lda #$9B
    jsr VecStartObjectAtPlace
    lda a:OBJ_Y_LO,y
    clc
    adc #$08
    sta a:OBJ_Y_LO,y
DroppingPieceStep_at3:
    lda #$06
    sta a:OBJ_3510,x
    stz a:OBJ_3250,x
    lda #$10
    sta a:OBJ_2B18,x
    inc a:OBJ_27A8,x
DroppingPieceStep_at2:
    rts

; @routine 0F6C:4A4 WalkAndTurnAtEdge
; @run 70:BCA4
; WHAT IT DOES: A walking object's step: the leap or the hurt answer ends it (C 1); otherwise it steps its animation, moves along X and,
; when it is near the player and not at its usual height, goes on in other code; when the tile ahead of its feet is solid (bit 1) it
; turns round.
; HOW: StartLeapWhenHurtAnswerYes, HurtOnceThenAnswerYes, StepListByNumberTableC585, ApplyVelocityX, DistanceXToPlayer, ProbeTileSideBit1,
; and for the turn ToggleFacing and a tail jump into NegateVelocityX.
; WAITS: no.
; Entry: X object, Y kept unless a step sets it. Exit: as the last instruction that ran. The arms that end at the first two answers
; and the near arm ran in no script.
WalkAndTurnAtEdge:
    jsr StartLeapWhenHurtAnswerYes
    bcc WalkAndTurnAtEdge_at0
    rts
WalkAndTurnAtEdge_at0:
    jsr HurtOnceThenAnswerYes
    bcc WalkAndTurnAtEdge_at1
    rts
WalkAndTurnAtEdge_at1:
    jsr StepListByNumberTableC585
    jsr VecApplyVelocityX
    jsr VecDistanceXToPlayer
    cmp #$40
    bcs WalkAndTurnAtEdge_at2
    lda a:OBJ_Y_LO
    cmp #$9A
    beq WalkAndTurnAtEdge_at2
    jmp $BCD3
WalkAndTurnAtEdge_at2:
    jsr ProbeTileSideBit1
    bcc WalkAndTurnAtEdge_at3
    jsr ToggleFacing
    jmp VecNegateVelocityX
WalkAndTurnAtEdge_at3:
    rts

; @routine 0F6D:324 ProbeTileSideBit1
; @run 71:C324
; WHAT IT DOES: Says (C 1) whether the tile a little ahead of the object's feet has bit 1 in its attribute.
; HOW: The side of the point comes from the sign of the X speed (two offsets at $C357), $16 below the object's Y.
; The tile read is the vector at $8006.
; WAITS: no.
; Entry: X object. Exit: A the attribute's bit 1, N and Z from it, C that bit, Y and V as the tile read left them.
ProbeTileSideBit1:
    lda a:OBJ_VX_HI,x
    asl a
    rol a
    and #$01
    asl a
    tay
    lda a:OBJ_X_LO,x
    clc
    adc a:$C357,y
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:$C358,y
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    clc
    adc #$16
    sta z:zTemp02
    cla
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    jsr StageTileReadVector9991
    and #$02
    beq ProbeTileSideBit1_at0
    sec
    rts
ProbeTileSideBit1_at0:
    clc
    rts
