; @routine 05BD:006 StageTileReadVector9991
; @run 6B:8006
; WHAT IT DOES: Goes to the tile read at $9991 (ProbeTileRead9991).
; HOW: One jump. The twelve overlays of the group whose tile read is at $9991 hold this vector.
; WAITS: no.
; Entry, exit: as ProbeTileRead9991.
StageTileReadVector9991:
    jmp ProbeTileRead9991

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
