; @routine 0EBD:351 LoadFromSide0EBD
; @run 71:C351
; WHAT IT DOES: Comes in from behind the player at its height, from screen 2 until the scroll's limit; other states go to $C3C9.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFromSide0EBDWaits = $C420 ; OBJ_WORK_H
LoadFromSide0EBDRises = $C424 ; vy high byte
LoadFromSide0EBDLater = $C3C9 ; the other states' code of this overlay
LoadFromSide0EBD:
    lda a:OBJ_STATE,x
    beq LoadFromSide0EBD_at0
    jmp LoadFromSide0EBDLater
LoadFromSide0EBD_at0:
    jsr VecLoadObjectRecord
    lda a:ScrollXHiScreen
    cmp a:ScrollXMaxHi
    bne LoadFromSide0EBD_at1
    jmp VecClearObject
LoadFromSide0EBD_at1:
    lda a:ScrollXHiScreen
    cmp #$02
    bcs LoadFromSide0EBD_at2
    jmp VecClearObject
LoadFromSide0EBD_at2:
    lda a:OBJ_FACING_LEFT
    beq LoadFromSide0EBD_at3
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$FF
    sta a:OBJ_X_HI,x
    lda #$C0
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
    bra LoadFromSide0EBD_at4
LoadFromSide0EBD_at3:
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$40
    sta a:OBJ_X_LO,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x
LoadFromSide0EBD_at4:
    ldy a:OBJ_WORK_G,x
    lda a:LoadFromSide0EBDWaits,y
    sta a:OBJ_WORK_H,x
    inc a:OBJ_WORK_F,x
    lda a:OBJ_Y_HI
    sta a:OBJ_Y_HI,x
    lda a:OBJ_Y_LO
    sta a:OBJ_Y_LO,x
    ldy a:OBJ_WORK_I,x
    lda a:LoadFromSide0EBDRises,y
    sta a:OBJ_VY_HI,x
    jmp VecApplyVelocityY

; @routine 0EBF:6CA WeaveThenDive0EBF
; @run 71:D6CA
; WHAT IT DOES: Weaves for OBJ_WORK_G counts; unless the player is near then, leaps (sound $CB) and goes on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WeaveThenDive0EBFWaits = $D747 ; pairs of waits
WeaveThenDive0EBF:
    lda #$57
    sta z:zTemp10
    lda #$D7
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$80
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_I,x
    jsr VecScaleBySine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:zProduct1
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda #$A0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_J,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:zProduct1
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    inc a:OBJ_WORK_I,x
    inc a:OBJ_WORK_J,x
    dec a:OBJ_WORK_G,x
    beq WeaveThenDive0EBF_at0
    jsr VecDistanceXToPlayer
    cmp #$38
    bcc WeaveThenDive0EBF_at0
    lda #$F6
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$40
    sta a:OBJ_WORK_H,x
    jsr VecNextRandom
    and #$0E
    tay
    lda a:WeaveThenDive0EBFWaits+1,y
    sta a:OBJ_WORK_F,x
    lda a:WeaveThenDive0EBFWaits,y
    sta a:OBJ_WORK_G,x
    lda #$CB
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
WeaveThenDive0EBF_at0:
    rts

; @routine 0F6C:135 ShardByArgArc0F6C
; @run 70:B935
; WHAT IT DOES: A shard flung by its start argument, falling at most 3 a count.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ShardByArgArc0F6CPlacesX = $B9D9 ; first step
ShardByArgArc0F6CPlacesY = $B9E0 ; first step
ShardByArgArc0F6CSpeedsX = $B9E7 ; vx by argument
ShardByArgArc0F6CSpeedsY = $B9F5 ; vy by argument
ShardByArgArc0F6CFrames = $BA03 ; frame by argument
ShardByArgArc0F6C:
    lda a:OBJ_STATE,x
    bne ShardByArgArc0F6C_at0
    jsr VecLoadObjectRecord
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    ldy a:OBJ_START_ARG,x
    lda a:ShardByArgArc0F6CFrames,y
    sta a:OBJ_FRAME,x
    lda a:ShardByArgArc0F6CPlacesX,y
    sta a:OBJ_VX_HI,x
    lda a:ShardByArgArc0F6CPlacesY,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:ShardByArgArc0F6CSpeedsX,y
    sta a:OBJ_VX_LO,x
    lda a:ShardByArgArc0F6CSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:ShardByArgArc0F6CSpeedsY,y
    sta a:OBJ_VY_LO,x
    lda a:ShardByArgArc0F6CSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    rts
ShardByArgArc0F6C_at0:
    lda a:OBJ_VY_HI,x
    bmi ShardByArgArc0F6C_at1
    cmp #$01
    bmi ShardByArgArc0F6C_at1
    jsr ThrowOnTurn0F6C
ShardByArgArc0F6C_at1:
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    clc
    adc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    bmi ShardByArgArc0F6C_at2
    cmp #$03
    bcc ShardByArgArc0F6C_at2
    stz a:OBJ_VY_LO,x
ShardByArgArc0F6C_at2:
    rts

; @routine 0F6D:42D PieceArcThenRest0F6D
; @run 71:C42D
; WHAT IT DOES: A piece arcing by its start argument toward the player's side; landing, leaves a kind $9B object and goes.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PieceArcThenRest0F6DFrames = $C4AA ; frame by argument
PieceArcThenRest0F6DSpeeds = $C4AC ; vx by argument
PieceArcThenRest0F6DRises = $C4B0 ; vy by argument
PieceArcThenRest0F6DFloors = $C4B4 ; floor by argument
PieceArcThenRest0F6D:
    lda a:OBJ_STATE,x
    bne PieceArcThenRest0F6D_at0
    jsr VecLoadObjectRecord
    stz a:OBJ_HITBOX_ON,x
    ldy a:OBJ_START_ARG,x
    lda a:PieceArcThenRest0F6DFrames,y
    sta a:OBJ_FRAME,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:PieceArcThenRest0F6DSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:PieceArcThenRest0F6DSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:PieceArcThenRest0F6DRises,y
    sta a:OBJ_VY_LO,x
    lda a:PieceArcThenRest0F6DRises+1,y
    sta a:OBJ_VY_HI,x
    jsr VecSideOfPlayer
    and #$01
    bne PieceArcThenRest0F6D_at1
    rts
PieceArcThenRest0F6D_at1:
    jmp VecNegateVelocityX
PieceArcThenRest0F6D_at0:
    dec a
    bne PieceArcThenRest0F6D_at2
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    jsr VecApplyGravity
    lda a:OBJ_VY_HI,x
    bmi PieceArcThenRest0F6D_at3
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_Y_LO,x
    cmp a:PieceArcThenRest0F6DFloors,y
    bcc PieceArcThenRest0F6D_at3
    lda a:PieceArcThenRest0F6DFloors,y
    sta a:OBJ_Y_LO,x
    lda #$10
    sta a:OBJ_WORK_D,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    inc a:OBJ_STATE,x
    jsr VecFindFreeSlotFrom20
    lda #$9B
    jsr VecStartObjectAtPlace
PieceArcThenRest0F6D_at3:
    rts
PieceArcThenRest0F6D_at2:
    dec a:OBJ_WORK_D,x
    bne PieceArcThenRest0F6D_at4
    jmp VecClearObject
PieceArcThenRest0F6D_at4:
    rts

; @routine 12DC:16F DebrisFallThenGround12DC
; @run 70:B96F
; WHAT IT DOES: Debris spreading by slot until its time or the top of the screen; then kind $9C, with a kind $83 left there.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DebrisFallThenGround12DCSteps = $B9EF ; vx step by slot
DebrisFallThenGround12DCLifts = $BA07 ; vy step by slot
DebrisFallThenGround12DC:
    lda #$EA
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_E,x
    beq DebrisFallThenGround12DC_at0
    lda a:OBJ_Y_HI,x
    bmi DebrisFallThenGround12DC_at0
    bne DebrisFallThenGround12DC_at1
    lda a:OBJ_Y_LO,x
    cmp #$18
    bcc DebrisFallThenGround12DC_at0
DebrisFallThenGround12DC_at1:
    lda a:OBJ_WORK_D,x
    and #$01
    bne DebrisFallThenGround12DC_at2
    jsr VecApplyVelocity
    txa
    sec
    sbc #$20
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:DebrisFallThenGround12DCSteps,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:DebrisFallThenGround12DCSteps+1,y
    sta a:OBJ_VX_HI,x
    lda a:OBJ_VY_LO,x
    sec
    sbc a:DebrisFallThenGround12DCLifts,y
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
DebrisFallThenGround12DC_at2:
    rts
DebrisFallThenGround12DC_at0:
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    jsr VecFindFreeSlotFrom30
    bcc DebrisFallThenGround12DC_at3
    lda a:OBJ_Y_HI,x
    bne DebrisFallThenGround12DC_at4
    lda a:OBJ_Y_LO,x
    cmp #$18
    bcs DebrisFallThenGround12DC_at5
DebrisFallThenGround12DC_at4:
    stz a:OBJ_Y_HI,x
    lda #$18
    sta a:OBJ_Y_LO,x
DebrisFallThenGround12DC_at5:
    lda #$83
    jsr VecStartObjectAtPlace
DebrisFallThenGround12DC_at3:
    rts

; @routine 161B:1DE LoadFlyInByArg161B
; @run 70:B1DE
; WHAT IT DOES: Flies in from the right, or now and then from the left, above slot 0, sometimes with a kind $2D rider.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFlyInByArg161B:
    jsr VecLoadObjectRecord
    lda #$04
    sta a:OBJ_HITBOX_DY,x
    lda #$F0
    sta a:OBJ_WORK_F,x
    lda a:OBJ_START_ARG,x
    and #$03
    bne LoadFlyInByArg161B_at0
    jsr VecNextRandom
    and #$03
    beq LoadFlyInByArg161B_at0
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$FF
    sta a:OBJ_X_HI,x
    lda #$F0
    sta a:OBJ_X_LO,x
    lda #$00
    sta a:OBJ_VX_HI,x
    lda #$10
    sta a:OBJ_VX_LO,x
    bra LoadFlyInByArg161B_at1
LoadFlyInByArg161B_at0:
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$10
    sta a:OBJ_X_LO,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
LoadFlyInByArg161B_at1:
    lda a:OBJ_Y_LO
    sec
    sbc #$10
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    sbc #$00
    sta a:OBJ_Y_HI,x
    jsr VecNextRandom
    and #$03
    bne LoadFlyInByArg161B_at2
    jsr VecFindFreeSlotFrom30
    bcc LoadFlyInByArg161B_at2
    lda #$2D
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    txa
    sta a:OBJ_WORK_D,y
LoadFlyInByArg161B_at2:
    rts

; @routine 18DB:5C7 RaiseArmed18DB
; @run 70:B5C7
; WHAT IT DOES: Arms itself with its hit box and three numbered kind $25 parts, two scripts, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RaiseArmed18DB:
    lda #$59
    sta a:OBJ_FRAME,x
    lda #$5C
    sta a:OBJ_HEALTH,x
    stz a:OBJ_PACE,x
    lda #$03
    sta a:OBJ_WORK_A,x
    stz a:OBJ_HIT_STATUS,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$E8
    sta a:OBJ_HITBOX_DX,x
    lda #$F0
    sta a:OBJ_HITBOX_DY,x
    lda #$10
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$10
    sta a:OBJ_HITBOX_HALF_H,x
    ldy #$11
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_START_ARG,y
    iny
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    lda #$02
    sta a:OBJ_START_ARG,y
    iny
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    lda #$03
    sta a:OBJ_START_ARG,y
    phx
    lda #$86
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$84
    ldx #$04
    jsr VecStartScriptSlotKind1
    plx
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
    rts
