; @routine 051E:754 SetUpSegmentChain051B
; @run 71:CF54
; WHAT IT DOES: The eight segments in slots $10-$17. First checks for the hit that starts them ($CF11); in state 0, slot $10
; copies itself as kind $35 into slots $11-$17, each loads its record, takes its hit box (slot $10), flip, facing,
; piece number and frame; slot $17 gets $10 health. Later states run $CFE6.
; HOW: ClearObject and a copy of the place per slot.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetUpSegmentChain051BPieces = $CE67 ; piece number by segment
SetUpSegmentChain051BFrames = $CE6F ; frame by segment
SetUpSegmentChain051B:
    bsr StartEightState2WhenHit051B
    lda a:OBJ_STATE,x
    beq SetUpSegmentChain051B_at0
    jmp SegmentChainFollow051B
SetUpSegmentChain051B_at0:
    cpx #$10
    bne SetUpSegmentChain051B_at2
    lda #$03
    sta z:zRasterStage
    phx
    lda a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    sta z:zTemp03
SetUpSegmentChain051B_at1:
    inx
    jsr VecClearObject
    lda #$35
    sta a:OBJ_KIND,x
    lda z:zTemp00
    sta a:OBJ_X_LO,x
    lda z:zTemp01
    sta a:OBJ_X_HI,x
    lda z:zTemp02
    sta a:OBJ_Y_LO,x
    lda z:zTemp03
    sta a:OBJ_Y_HI,x
    cpx #$17
    bne SetUpSegmentChain051B_at1
    plx
SetUpSegmentChain051B_at2:
    jsr VecLoadObjectRecord
    cpx #$10
    bne SetUpSegmentChain051B_at3
    lda #$10
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$16
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$EC
    sta a:OBJ_HITBOX_DY,x
    lda #$10
    sta a:OBJ_HITBOX_DY,x
SetUpSegmentChain051B_at3:
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
    jsr FacePlayerByPlace051B
    txa
    sec
    sbc #$10
    tay
    lda a:SetUpSegmentChain051BPieces,y
    sta a:OBJ_PIECE_PLACE,x
    lda #$00
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda #$01
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:SetUpSegmentChain051BFrames,y
    sta a:OBJ_FRAME,x
    cpx #$17
    beq SetUpSegmentChain051B_at4
    rts
SetUpSegmentChain051B_at4:
    lda #$10
    sta a:OBJ_HEALTH,x
    rts

; @routine 051E:7E6 SegmentChainFollow051B
; @run 71:CFE6
; WHAT IT DOES: State 1: the last segment runs the head's moves ($D0A8). Every segment moves, then places the next segment
; at the cosine and sine of its angle from it, less the camera's movement, and swings its angle between $A0 and $F0
; either way. Later states go to $D1BF.
; HOW: SetVelocityFromAngle into the next slot, a 16-bit swing.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SegmentChainFollow051B:
    dec a
    beq SegmentChainFollow051B_at0
    jmp SegmentDies051B
SegmentChainFollow051B_at0:
    cpx #$17
    bne SegmentChainFollow051B_at1
    jsr HeadAttacks051B
SegmentChainFollow051B_at1:
    jsr NegateSpeedIfFacingLeft051B
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    lda #$00
    sta z:zTemp00
    lda #$08
    sta z:zTemp01
    lda a:OBJ_PIECE_PLACE,x
    inx
    jsr VecSetVelocityFromAngle
    dex
    stz z:zTemp00
    stz z:zTemp01
    lda a:ScrollSpeedX
    bpl SegmentChainFollow051B_at2
    dec z:zTemp00
SegmentChainFollow051B_at2:
    lda a:ScrollSpeedY
    bpl SegmentChainFollow051B_at3
    dec z:zTemp01
SegmentChainFollow051B_at3:
    lda a:OBJ_X_FRAC,x
    sta a:OBJ_X_FRAC+1,x
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollSpeedX
    sta a:OBJ_X_LO+1,x
    lda a:OBJ_X_HI,x
    adc z:zTemp00
    sta a:OBJ_X_HI+1,x
    lda a:OBJ_Y_FRAC,x
    sta a:OBJ_Y_FRAC+1,x
    lda a:OBJ_Y_LO,x
    clc
    adc a:ScrollSpeedY
    sta a:OBJ_Y_LO+1,x
    lda a:OBJ_Y_HI,x
    adc z:zTemp01
    sta a:OBJ_Y_HI+1,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    bmi SegmentChainFollow051B_at7
    lda a:OBJ_SWOOP_SPEED_HI,x
    clc
    adc a:OBJ_PIECE_NEXT,x
    sta a:OBJ_PIECE_NEXT,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    clc
    adc a:OBJ_PIECE_PLACE,x
    sta a:OBJ_PIECE_PLACE,x
    cmp #$F0
    bcs SegmentChainFollow051B_at4
    rts
SegmentChainFollow051B_at4:
    lda #$F0
SegmentChainFollow051B_at5:
    sta a:OBJ_PIECE_PLACE,x
    stz a:OBJ_PIECE_NEXT,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    eor #$FF
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:OBJ_SWOOP_SPEED_HI,x
    eor #$FF
    inc a
    sta a:OBJ_SWOOP_SPEED_HI,x
    bne SegmentChainFollow051B_at6
    inc a:OBJ_SWOOP_SPEED_LO,x
SegmentChainFollow051B_at6:
    rts
SegmentChainFollow051B_at7:
    lda a:OBJ_SWOOP_SPEED_HI,x
    clc
    adc a:OBJ_PIECE_NEXT,x
    sta a:OBJ_PIECE_NEXT,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    clc
    adc a:OBJ_PIECE_PLACE,x
    sta a:OBJ_PIECE_PLACE,x
    cmp #$A0
    bcc SegmentChainFollow051B_at8
    rts
SegmentChainFollow051B_at8:
    lda #$A0
    bra SegmentChainFollow051B_at5

; @routine 051F:0A8 HeadAttacks051B
; @run 71:D0A8
; WHAT IT DOES: The head. Substate 0: its box ($D195) and list; when the list ends throws ($D12A); under 8 health moves on with
; list OBJ_FALL_LO 1. Substate 1: steps that list and turns the swings about ($D105). Substate 2: its wider box
; ($D1AA) and list, throwing again ($D158) when the animation timer is 7.
; HOW: Branches by substate.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HeadAttacks051B:
    lda a:OBJ_SUBSTATE,x
    beq HeadAttacks051B_at1
    dec a
    beq HeadAttacks051B_at4
    jsr SetHeadHitBoxOpen051B
    bsr StepHeadList051B
    lda a:OBJ_ANIM_TIMER,x
    cmp #$07
    bne HeadAttacks051B_at0
    jsr ThrowFromSlotPool051B
HeadAttacks051B_at0:
    rts
HeadAttacks051B_at1:
    jsr SetHeadHitBox051B
    bsr StepHeadList051B
    bne HeadAttacks051B_at2
    jsr ThrowFromMouth051B
HeadAttacks051B_at2:
    lda a:OBJ_HEALTH,x
    cmp #$08
    bcs HeadAttacks051B_at3
    inc a:OBJ_FALL_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
HeadAttacks051B_at3:
    rts
HeadAttacks051B_at4:
    jsr StepHeadList051B
    bne HeadAttacks051B_at5
    inc a:OBJ_FALL_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    bsr SpeedSwings051B
    inc a:OBJ_SUBSTATE,x
HeadAttacks051B_at5:
    rts

; @routine 051F:0F2 StepHeadList051B
; @run 71:D0F2
; WHAT IT DOES: Steps the list OBJ_FALL_LO picks from the table at $CEBB.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepHeadList051BLists = $CEBB ; list addresses
StepHeadList051B:
    lda a:OBJ_FALL_LO,x
    asl a
    tay
    lda a:StepHeadList051BLists,y
    sta z:zTemp10
    lda a:StepHeadList051BLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    rts

; @routine 051F:105 SpeedSwings051B
; @run 71:D105
; WHAT IT DOES: Gives slots $10-$17 a new swing speed: $01C0 for those swinging forward, $FE40 for those swinging back.
; HOW: A loop over the eight.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SpeedSwings051B:
    phx
    ldx #$10
SpeedSwings051B_at0:
    lda a:OBJ_SWOOP_SPEED_LO,x
    bmi SpeedSwings051B_at1
    lda #$C0
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda #$01
    sta a:OBJ_SWOOP_SPEED_LO,x
    bra SpeedSwings051B_at2
SpeedSwings051B_at1:
    lda #$40
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda #$FE
    sta a:OBJ_SWOOP_SPEED_LO,x
SpeedSwings051B_at2:
    inx
    cpx #$18
    bne SpeedSwings051B_at0
    plx
    rts

; @routine 051F:12A ThrowFromMouth051B
; @run 71:D12A
; WHAT IT DOES: Starts a kind $3A object $28 ahead and 6 above, in a free slot from $30 on screen.
; HOW: FindFreeSlotFrom30, CopyFacingToY051B, PlaceAheadInY051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowFromMouth051B:
    jsr VecFindFreeSlotFrom30
    bcc ThrowFromMouth051B_at0
    jsr CopyFacingToY051B
    lda #$28
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    jsr PlaceAheadInY051B
    bne ThrowFromMouth051B_at0
    lda a:OBJ_Y_LO,x
    clc
    adc #$FA
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_HI,x
    adc #$FF
    sta a:OBJ_Y_HI,y
    bne ThrowFromMouth051B_at0
    lda #$3A
    sta a:OBJ_KIND,y
ThrowFromMouth051B_at0:
    rts

; @routine 051F:158 ThrowFromSlotPool051B
; @run 71:D158
; WHAT IT DOES: Takes the first empty slot of $19-$28, clears it and starts a kind $3B object $0C ahead, on screen.
; HOW: As ThrowFromMouth051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowFromSlotPool051B:
    ldy #$19
ThrowFromSlotPool051B_at0:
    lda a:OBJ_KIND,y
    beq ThrowFromSlotPool051B_at1
    iny
    cpy #$29
    bne ThrowFromSlotPool051B_at0
    rts
ThrowFromSlotPool051B_at1:
    phx
    sxy
    jsr VecClearObject
    sxy
    plx
    jsr CopyFacingToY051B
    lda #$0C
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    jsr PlaceAheadInY051B
    bne ThrowFromSlotPool051B_at2
    lda a:OBJ_Y_LO,x
    clc
    adc #$00
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_HI,x
    adc #$00
    sta a:OBJ_Y_HI,y
    bne ThrowFromSlotPool051B_at2
    lda #$3B
    sta a:OBJ_KIND,y
ThrowFromSlotPool051B_at2:
    rts

; @routine 051F:195 SetHeadHitBox051B
; @run 71:D195
; WHAT IT DOES: The head's hit box: half size $14 by $0C, offset $14, 0.
; HOW: Four stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHeadHitBox051B:
    lda #$14
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$0C
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$14
    sta a:OBJ_HITBOX_DX,x
    lda #$00
    sta a:OBJ_HITBOX_DY,x
    rts

; @routine 051F:1AA SetHeadHitBoxOpen051B
; @run 71:D1AA
; WHAT IT DOES: The head's open-mouth hit box: half size $14 by $14, offset $14, 0.
; HOW: Four stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHeadHitBoxOpen051B:
    lda #$14
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$14
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$14
    sta a:OBJ_HITBOX_DX,x
    lda #$00
    sta a:OBJ_HITBOX_DY,x
    rts

; @routine 051F:1BF SegmentDies051B
; @run 71:D1BF
; WHAT IT DOES: Substate 0: no hit box or loop flag bit 0, a wait by segment, attribute bit 4. Then, the wait over, slot $10
; throws the pieces ($D239); another (on screen with its sound) is cleared but for its place and becomes the
; segment's debris kind.
; HOW: Tables by segment; the place kept on the stack across ClearObject.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SegmentDies051BWaits = $D2A6 ; wait by segment
SegmentDies051BKinds = $D2AE ; debris kind by segment
SegmentDies051BSounds = $D2D6 ; sound by segment
SegmentDies051B:
    lda a:OBJ_SUBSTATE,x
    bne SegmentDies051B_at0
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_LOOP_FLAGS,x
    and #$FE
    sta a:OBJ_LOOP_FLAGS,x
    txa
    sec
    sbc #$10
    tay
    lda a:SegmentDies051BWaits,y
    sta a:OBJ_ANIM_TIMER,x
    lda #$10
    ora a:OBJ_SPRITE_ATTR,x
    sta a:OBJ_SPRITE_ATTR,x
    inc a:OBJ_SUBSTATE,x
    rts
SegmentDies051B_at0:
    lda a:OBJ_ANIM_TIMER,x
    beq SegmentDies051B_at1
    dec a:OBJ_ANIM_TIMER,x
    beq SegmentDies051B_at1
    rts
SegmentDies051B_at1:
    cpx #$10
    beq ThrowRemains051B
    lda a:OBJ_X_HI,x
    bne SegmentDies051B_at2
    txa
    sec
    sbc #$10
    tay
    lda a:SegmentDies051BSounds,y
    jsr VecSoundCommand
SegmentDies051B_at2:
    lda a:OBJ_X_LO,x
    pha
    lda a:OBJ_X_HI,x
    pha
    lda a:OBJ_Y_LO,x
    pha
    lda a:OBJ_Y_HI,x
    pha
    jsr VecClearObject
    pla
    sta a:OBJ_Y_HI,x
    pla
    sta a:OBJ_Y_LO,x
    pla
    sta a:OBJ_X_HI,x
    pla
    sta a:OBJ_X_LO,x
    txa
    sec
    sbc #$10
    tay
    lda a:SegmentDies051BKinds,y
    sta a:OBJ_KIND,x
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
    rts

; @routine 051F:239 ThrowRemains051B
; @run 71:D239
; WHAT IT DOES: Every fourth frame starts a kind $9E piece at one of eight places around it, sound $15; after eight, clears itself.
; HOW: A table of offset words indexed by OBJ_WORK_K.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowRemains051BPlaces = $D2B6 ; X word, Y word by piece
ThrowRemains051B:
    lda a:OBJ_RAIN_COUNT,x
    cmp #$08
    bne ThrowRemains051B_at0
    jmp VecClearObject
ThrowRemains051B_at0:
    tst #$03,z:zWaveCounter
    bne ThrowRemains051B_at1
    jsr VecFindFreeSlotFrom30
    bcc ThrowRemains051B_at1
    phx
    lda #$9E
    sta a:OBJ_KIND,y
    lda #$80
    sta a:OBJ_SPRITE_FLIP,y
    lda a:OBJ_RAIN_COUNT,x
    asl a
    asl a
    tax
    lda a:ThrowRemains051BPlaces,x
    sta a:OBJ_X_LO,y
    lda a:ThrowRemains051BPlaces+1,x
    sta a:OBJ_X_HI,y
    lda a:ThrowRemains051BPlaces+2,x
    sta a:OBJ_Y_LO,y
    lda a:ThrowRemains051BPlaces+3,x
    sta a:OBJ_Y_HI,y
    plx
    lda a:OBJ_X_LO,x
    clc
    adc a:OBJ_X_LO,y
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI,x
    adc a:OBJ_X_HI,y
    sta a:OBJ_X_HI,y
    lda a:OBJ_Y_LO,x
    clc
    adc a:OBJ_Y_LO,y
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_HI,x
    adc a:OBJ_Y_HI,y
    sta a:OBJ_Y_HI,y
    lda #$15
    jsr VecSoundCommand
    inc a:OBJ_RAIN_COUNT,x
ThrowRemains051B_at1:
    rts

; @routine 051F:2DE BouncingShot051B
; @run 71:D2DE
; WHAT IT DOES: State 0 loads it flying at 4 the facing way. State 1, at terrain ($D367), turns it to rise away slowly; else it
; flies falling by 9 and is cleared off the first screen. Later it falls by $18 ($D34E).
; HOW: Branches by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BouncingShot051B:
    lda a:OBJ_STATE,x
    bne BouncingShot051B_at0
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
    lda #$19
    sta a:OBJ_FRAME,x
    lda #$00
    sta a:OBJ_VX_LO,x
    lda #$04
    sta a:OBJ_VX_HI,x
    jsr NegateSpeedIfFacingLeft051B
    lda #$00
    sta a:OBJ_VY_LO,x
    lda #$00
    sta a:OBJ_VY_HI,x
    rts
BouncingShot051B_at0:
    dec a
    bne FallingShot051B
    jsr TerrainAtPlace051B
    bcc BouncingShot051B_at1
    lda #$00
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    jsr NegateSpeedIfFacingLeft051B
    lda #$00
    sta a:OBJ_VY_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    inc a:OBJ_STATE,x
    rts
BouncingShot051B_at1:
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    clc
    adc #$09
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x

; @routine 051F:342 ClearOffFirstScreen051B
; @run 71:D342
; WHAT IT DOES: Clears the object once X or Y is off the first screen.
; HOW: The high bytes together.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ClearOffFirstScreen051B:
    lda a:OBJ_Y_HI,x
    ora a:OBJ_X_HI,x
    bne ClearOffFirstScreen051B_at0
    rts
ClearOffFirstScreen051B_at0:
    jmp VecClearObject

; @routine 051F:34E FallingShot051B
; @run 71:D34E
; WHAT IT DOES: Moves, falls by $18 and is cleared off the first screen.
; HOW: Into ClearOffFirstScreen051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallingShot051B:
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    bra ClearOffFirstScreen051B

; @routine 051F:367 TerrainAtPlace051B
; @run 71:D367
; WHAT IT DOES: Carry set when the terrain at the object's place has bit 0 or 1.
; HOW: TerrainTileRead at X zTemp00-01, Y zTemp02-03.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
TerrainAtPlace051B:
    lda a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    sta z:zTemp03
    jsr TerrainTileRead
    and #$03
    bne TerrainAtPlace051B_at0
    clc
    rts
TerrainAtPlace051B_at0:
    sec
    rts

; @routine 051F:38E WobblingShot051B
; @run 71:D38E
; WHAT IT DOES: A shot at 1 the facing way, drifting up or down by slot and wobbling on a sine; cleared at terrain or off screen.
; HOW: Tables by slot's low bits; ScaleBySine of a stepping angle.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WobblingShot051BList = $CE9F ; a frame list
WobblingShot051BDrift = $D386 ; Y drift words by slot
WobblingShot051B:
    lda a:OBJ_STATE,x
    bne WobblingShot051B_at0
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
    lda #$00
    sta a:OBJ_VX_LO,x
    lda #$01
    sta a:OBJ_VX_HI,x
    jsr NegateSpeedIfFacingLeft051B
    txa
    and #$03
    asl a
    tay
    lda a:WobblingShot051BDrift,y
    sta a:OBJ_PIECE_NEXT,x
    lda a:WobblingShot051BDrift+1,y
    sta a:OBJ_PIECE_PLACE,x
    jsr VecNextRandom
    sta a:OBJ_SWOOP_SPEED_HI,x
    rts
WobblingShot051B_at0:
    lda #$9F
    sta z:zTemp10
    lda #$CE
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityX
    stz z:zTemp00
    lda a:OBJ_PIECE_PLACE,x
    bpl WobblingShot051B_at1
    dec z:zTemp00
WobblingShot051B_at1:
    lda a:OBJ_Y_FRAC,x
    clc
    adc a:OBJ_PIECE_NEXT,x
    sta a:OBJ_Y_FRAC,x
    lda a:OBJ_Y_LO,x
    adc a:OBJ_PIECE_PLACE,x
    sta a:OBJ_Y_LO,x
    lda z:zTemp00
    adc a:OBJ_Y_HI,x
    sta a:OBJ_Y_HI,x
    jsr VecApplyVelocityY
    lda #$30
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_SWOOP_SPEED_HI,x
    jsr VecScaleBySine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda a:OBJ_SWOOP_SPEED_LO,x
    clc
    adc #$80
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:OBJ_SWOOP_SPEED_HI,x
    clc
    adc #$02
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    sta z:zTemp03
    jsr TerrainTileRead
    and #$03
    bne WobblingShot051B_at2
    jmp ClearOffFirstScreen051B
WobblingShot051B_at2:
    jmp VecClearObject
