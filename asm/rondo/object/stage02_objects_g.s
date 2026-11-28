; @routine 051F:4EA ChainHeadByState051B
; @run 71:D4EA
; WHAT IT DOES: Checks the head for a hit ($D734) and goes to the state's handler.
; HOW: A table of handlers at $D4FF.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChainHeadByState051BStates = $D4FF ; a handler per state
ChainHeadByState051B:
    jsr KillHeadWhenBodyHit051B
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ChainHeadByState051BStates,y
    sta z:zTemp00
    lda a:ChainHeadByState051BStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051F:509 SetUpFlyingHead051B
; @run 71:D509
; WHAT IT DOES: State 0 of the two-slot flier: loads the record, starts the body's list in the next slot, sets the angle, swing
; speed and counts, and makes the next slot kind $4A.
; HOW: LoadObjectRecord, StepObjectFrames and ClearObject on slot X+1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetUpFlyingHead051BList = $D4C4 ; a frame list
SetUpFlyingHead051B:
    jsr VecLoadObjectRecord
    inx
    lda #$C4
    sta z:zTemp10
    lda #$D4
    sta z:zTemp11
    jsr VecStepObjectFrames
    dex
    lda #$C0
    sta a:OBJ_PIECE_PLACE,x
    lda #$00
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda #$20
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda #$00
    sta a:OBJ_FALL_LO,x
    lda #$10
    sta a:OBJ_RAIN_COUNT,x
    lda #$40
    sta a:OBJ_KNOCK_FORWARD,x
    lda #$01
    sta a:OBJ_KNOCK_FORWARD,x
    inx
    jsr VecClearObject
    dex
    lda #$4A
    sta a:OBJ_KIND+1,x
    rts

; @routine 051F:547 AimFlyingHead051B
; @run 71:D547
; WHAT IT DOES: State 1: moves the parts ($D60E) unless the head is gone; with II newly pressed, copies the swing sizes to the
; next slot, aims at the player from there, sets a velocity of $0140 that way, turns it back when rising leftward and
; starts a count of $30. Then moves as $D59C.
; HOW: AngleToPlayer from slot X+1, SetVelocityFromAngle.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
AimFlyingHead051B:
    jsr FlyingHeadParts051B
    lda a:OBJ_SUBSTATE,x
    cmp #$02
    beq PlaceFlyingBody051B
    tst #$02,z:zPadPressed
    beq PlaceFlyingBody051B
    lda a:OBJ_SWOOP_SPEED_LO,x
    sta a:OBJ_SWOOP_SPEED_LO+1,x
    lda a:OBJ_SWOOP_SPEED_HI,x
    sta a:OBJ_SWOOP_SPEED_HI+1,x
    lda a:OBJ_FALL_LO,x
    sta a:OBJ_FALL_LO+1,x
    lda a:OBJ_RAIN_COUNT,x
    sta a:OBJ_RAIN_COUNT+1,x
    inx
    jsr VecAngleToPlayer
    dex
    sta a:OBJ_WORK_J,x
    lda #$40
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_J,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_VX_HI,x
    bpl AimFlyingHead051B_at0
    jsr VecNegateVelocityX
AimFlyingHead051B_at0:
    lda a:OBJ_VX_HI,x
    bpl AimFlyingHead051B_at1
    jsr VecNegateVelocityY
AimFlyingHead051B_at1:
    lda #$30
    sta a:OBJ_WORK_K,x
    inc a:OBJ_STATE,x

; @routine 051F:59C PlaceFlyingBody051B
; @run 71:D59C
; WHAT IT DOES: Places the next slot on the cosine and sine of the angle at the swing sizes from the head, after moving the head
; with the camera, and advances the angle unless the head is gone.
; HOW: ScaleByCosine and ScaleBySine into X+1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PlaceFlyingBody051B:
    inx
    jsr MoveWithScroll051B
    dex
    lda a:OBJ_SWOOP_SPEED_LO,x
    sta z:zTemp00
    lda a:OBJ_SWOOP_SPEED_HI,x
    sta z:zTemp01
    lda a:OBJ_PIECE_PLACE,x
    jsr VecScaleByCosine
    stz z:zTemp00
    lda z:zPlaneRowsLeft
    bpl PlaceFlyingBody051B_at0
    dec z:zTemp00
PlaceFlyingBody051B_at0:
    lda a:OBJ_X_LO,x
    clc
    adc z:zPlaneRowsLeft
    sta a:OBJ_X_LO+1,x
    lda a:OBJ_X_HI,x
    adc z:zTemp00
    sta a:OBJ_X_HI+1,x
    lda a:OBJ_FALL_LO,x
    sta z:zTemp00
    lda a:OBJ_RAIN_COUNT,x
    sta z:zTemp01
    lda a:OBJ_PIECE_PLACE,x
    jsr VecScaleBySine
    stz z:zTemp00
    lda z:zPlaneRowsLeft
    bpl PlaceFlyingBody051B_at1
    dec z:zTemp00
PlaceFlyingBody051B_at1:
    lda a:OBJ_Y_LO,x
    clc
    adc z:zPlaneRowsLeft
    sta a:OBJ_Y_LO+1,x
    lda a:OBJ_Y_HI,x
    adc z:zTemp00
    sta a:OBJ_Y_HI+1,x
    lda a:OBJ_SUBSTATE,x
    cmp #$02
    beq PlaceFlyingBody051B_at2
    lda a:OBJ_KNOCK_FORWARD,x
    clc
    adc a:OBJ_PIECE_NEXT,x
    sta a:OBJ_PIECE_NEXT,x
    lda a:OBJ_KNOCK_FORWARD,x
    adc a:OBJ_PIECE_PLACE,x
    sta a:OBJ_PIECE_PLACE,x
PlaceFlyingBody051B_at2:
    rts

; @routine 051F:60E FlyingHeadParts051B
; @run 71:D60E
; WHAT IT DOES: Drips ($D6E7). Substate 0 steps the body's list and, five hits taken, remembers the substate and goes to
; substate 2, speeding the angle when health is low. Substate 1 steps another list the same way. Substate 2 steps
; the third list and, at its end, goes back.
; HOW: StepObjectFrames on slot X+1; the hit count of X+1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyingHeadParts051BList0 = $D4C4 ; a frame list
FlyingHeadParts051BList1 = $D4C9 ; a frame list
FlyingHeadParts051BList2 = $D4D0 ; a frame list
FlyingHeadParts051B:
    jsr DripFromBody051B
    lda a:OBJ_SUBSTATE,x
    bne FlyingHeadParts051B_at2
    inx
    lda #$C4
    sta z:zTemp10
    lda #$D4
    sta z:zTemp11
FlyingHeadParts051B_at0:
    jsr VecStepObjectFrames
    dex
    lda a:OBJ_HIT_TAKEN+1,x
    cmp #$05
    bcc FlyingHeadParts051B_at1
    lda a:OBJ_SUBSTATE,x
    sta a:OBJ_WORK_L,x
    lda #$02
    sta a:OBJ_SUBSTATE,x
    lda a:OBJ_HEALTH+1,x
    cmp #$04
    bcs FlyingHeadParts051B_at1
    lda #$C0
    sta a:OBJ_KNOCK_FORWARD,x
    lda #$02
    sta a:OBJ_KNOCK_FORWARD,x
    lda #$01
    sta a:OBJ_WORK_L,x
FlyingHeadParts051B_at1:
    rts
FlyingHeadParts051B_at2:
    dec a
    bne FlyingHeadParts051B_at3
    inx
    lda #$C9
    sta z:zTemp10
    lda #$D4
    sta z:zTemp11
    bra FlyingHeadParts051B_at0
FlyingHeadParts051B_at3:
    inx
    lda #$D0
    sta z:zTemp10
    lda #$D4
    sta z:zTemp11
    jsr VecStepObjectFrames
    dex
    cmp #$00
    bne FlyingHeadParts051B_at4
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_ANIM_STEP,x
    lda a:OBJ_WORK_L,x
    sta a:OBJ_SUBSTATE,x
FlyingHeadParts051B_at4:
    rts

; @routine 051F:678 FlyingHeadSwell051B
; @run 71:D678
; WHAT IT DOES: State 2: moves the parts and the body; unless the head is gone, grows the swing sizes by the velocity and, its
; count over, turns the velocity back and goes on.
; HOW: 16-bit adds of VX into the two swing words.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyingHeadSwell051B:
    jsr FlyingHeadParts051B
    jsr PlaceFlyingBody051B
    lda a:OBJ_SUBSTATE,x
    cmp #$02
    beq FlyingHeadSwell051B_at0
    lda a:OBJ_VX_LO,x
    clc
    adc a:OBJ_SWOOP_SPEED_LO,x
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:OBJ_VX_HI,x
    adc a:OBJ_SWOOP_SPEED_HI,x
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_VX_LO,x
    clc
    adc a:OBJ_FALL_LO,x
    sta a:OBJ_FALL_LO,x
    lda a:OBJ_VX_HI,x
    adc a:OBJ_RAIN_COUNT,x
    sta a:OBJ_RAIN_COUNT,x
    dec a:OBJ_WORK_K,x
    bne FlyingHeadSwell051B_at0
    jsr VecNegateVelocityX
    jsr VecNegateVelocityY
    lda #$30
    sta a:OBJ_WORK_K,x
    inc a:OBJ_STATE,x
FlyingHeadSwell051B_at0:
    rts

; @routine 051F:6BF FlyingHeadShrink051B
; @run 71:D6BF
; WHAT IT DOES: State 3: as state 2 until the next, then back to state 1 with the swing sizes taken from the next slot.
; HOW: A call and four copies.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyingHeadShrink051B:
    jsr FlyingHeadSwell051B
    lda a:OBJ_STATE,x
    cmp #$04
    bcc FlyingHeadShrink051B_at0
    lda #$01
    sta a:OBJ_STATE,x
    lda a:OBJ_SWOOP_SPEED_LO+1,x
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:OBJ_SWOOP_SPEED_HI+1,x
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_FALL_LO+1,x
    sta a:OBJ_FALL_LO,x
    lda a:OBJ_RAIN_COUNT+1,x
    sta a:OBJ_RAIN_COUNT,x
FlyingHeadShrink051B_at0:
    rts

; @routine 051F:6E7 DripFromBody051B
; @run 71:D6E7
; WHAT IT DOES: Every fourth frame, a kind $4B drop beside the next slot at one of eight offsets, 12 above.
; HOW: FindFreeSlotFrom30 and NextRandom.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DripFromBody051BOffsets = $D72C ; X offset words
DripFromBody051B:
    tst #$03,z:zWaveCounter
    bne DripFromBody051B_at0
    jsr VecFindFreeSlotFrom30
    bcc DripFromBody051B_at0
    phy
    jsr VecNextRandom
    and #$07
    asl a
    tay
    lda a:DripFromBody051BOffsets,y
    sta z:zTemp00
    lda a:DripFromBody051BOffsets+1,y
    sta z:zTemp01
    ply
    lda #$4B
    sta a:OBJ_KIND,y
    lda a:OBJ_X_LO+1,x
    clc
    adc z:zTemp00
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI+1,x
    adc z:zTemp01
    sta a:OBJ_X_HI,y
    lda a:OBJ_Y_LO+1,x
    clc
    adc #$F4
    sta a:OBJ_Y_LO,y
    lda #$FF
    adc a:OBJ_Y_HI,y
    sta a:OBJ_Y_HI,y
DripFromBody051B_at0:
    rts

; @routine 051F:734 KillHeadWhenBodyHit051B
; @run 71:D734
; WHAT IT DOES: Under state 4, a hit on the next slot sounds $83, records the event and goes to state 4 with the next slot's hit
; box off.
; HOW: SoundCommand and RecordObjectEvent.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
KillHeadWhenBodyHit051B:
    lda a:OBJ_STATE,x
    cmp #$04
    bcs KillHeadWhenBodyHit051B_at0
    lda a:OBJ_HIT_STATUS+1,x
    and #$80
    beq KillHeadWhenBodyHit051B_at0
    lda #$83
    jsr VecSoundCommand
    jsr VecRecordObjectEvent
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_HITBOX_ON+1,x
KillHeadWhenBodyHit051B_at0:
    rts

; @routine 051F:756 MoveWithScroll051B
; @run 71:D756
; WHAT IT DOES: Adds the camera's signed X speed to the object's X.
; HOW: A signed 8-bit add to the 16-bit X.
; WAITS: no.
; Entry: X object. Exit: A the X high byte.
MoveWithScroll051B:
    stz z:zTemp00
    lda a:ScrollSpeedX
    bpl MoveWithScroll051B_at0
    dec z:zTemp00
MoveWithScroll051B_at0:
    clc
    adc a:OBJ_X_LO,x
    sta a:OBJ_X_LO,x
    lda z:zTemp00
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x
    rts

; @routine 051F:76F FlyingHeadFall051B
; @run 71:D76F
; WHAT IT DOES: State 4: substate 0 starts the next slot falling at $0100 without loop flag bit 0. Then drips, splashes ($D7C6),
; steps the next slot's list and moves it down, flickering; off the bottom both slots are cleared.
; HOW: ApplyVelocityY on slot X+1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyingHeadFall051BList = $D4D0 ; a frame list
FlyingHeadFall051B:
    lda a:OBJ_SUBSTATE,x
    bne FlyingHeadFall051B_at0
    stz a:OBJ_ANIM_TIMER+1,x
    stz a:OBJ_ANIM_STEP+1,x
    lda #$00
    sta a:OBJ_VY_LO+1,x
    lda #$01
    sta a:OBJ_VY_HI+1,x
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_LOOP_FLAGS+1,x
    and #$FE
    sta a:OBJ_LOOP_FLAGS+1,x
    rts
FlyingHeadFall051B_at0:
    jsr DripFromBody051B
    jsr SplashBesideBody051B
    inx
    lda #$D0
    sta z:zTemp10
    lda #$D4
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    dex
    lda #$10
    ora a:OBJ_SPRITE_ATTR+1,x
    sta a:OBJ_SPRITE_ATTR+1,x
    lda a:OBJ_Y_HI+1,x
    cmp #$01
    beq FlyingHeadFall051B_at1
    lda a:OBJ_Y_LO+1,x
    cmp #$F0
    bcs FlyingHeadFall051B_at1
    rts
FlyingHeadFall051B_at1:
    jsr VecClearObject
    inx
    jsr VecClearObject
    dex
    rts

; @routine 051F:7C6 SplashBesideBody051B
; @run 71:D7C6
; WHAT IT DOES: One frame in eight, a kind $9D splash in a free slot within 8 of the next slot each way.
; HOW: Two signed random offsets.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SplashBesideBody051B:
    lda z:zWaveCounter
    and #$07
    cmp #$06
    bne SplashBesideBody051B_at2
    jsr VecFindFreeSlotFrom30
    bcc SplashBesideBody051B_at2
    stz z:zTemp01
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$08
    sta z:zTemp00
    bpl SplashBesideBody051B_at0
    dec z:zTemp01
SplashBesideBody051B_at0:
    stz z:zTemp03
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$08
    sta z:zTemp02
    bpl SplashBesideBody051B_at1
    dec z:zTemp03
SplashBesideBody051B_at1:
    lda a:OBJ_X_LO+1,x
    clc
    adc z:zTemp00
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI+1,x
    adc z:zTemp01
    sta a:OBJ_X_HI,y
    lda a:OBJ_Y_LO+1,x
    clc
    adc z:zTemp02
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_HI+1,x
    adc z:zTemp03
    sta a:OBJ_Y_HI,y
    lda #$9D
    sta a:OBJ_KIND,y
SplashBesideBody051B_at2:
    rts

; @routine 0520:01B ClearWhenSlotBelowGone051B
; @run 71:D81B
; WHAT IT DOES: State 0 loads the record; later clears the object once the slot below holds no object.
; HOW: The kind of slot X-1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ClearWhenSlotBelowGone051B:
    lda a:OBJ_STATE,x
    bne ClearWhenSlotBelowGone051B_at0
    jmp VecLoadObjectRecord
ClearWhenSlotBelowGone051B_at0:
    lda a:OBJ_KIND-1,x
    bne ClearWhenSlotBelowGone051B_at1
    jmp VecClearObject
ClearWhenSlotBelowGone051B_at1:
    rts

; @routine 0520:02C RisingBubblePicture051B
; @run 71:D82C
; WHAT IT DOES: State 0 loads the record as sprite set 6, then steps ($D839); later rises ($D845).
; HOW: A branch by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RisingBubblePicture051B:
    lda a:OBJ_STATE,x
    bne RiseAndClearAtListEnd051B
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x

; @routine 0520:039 StepBubbleList051B
; @run 71:D839
; WHAT IT DOES: Steps the list at $D4DB.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepBubbleList051BList = $D4DB ; a frame list
StepBubbleList051B:
    lda #$DB
    sta z:zTemp10
    lda #$D4
    sta z:zTemp11
    jsr VecStepObjectFrames
    rts

; @routine 0520:045 RiseAndClearAtListEnd051B
; @run 71:D845
; WHAT IT DOES: Rises one dot, steps the list and clears the object at its end.
; HOW: StepBubbleList051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RiseAndClearAtListEnd051B:
    dec a:OBJ_Y_LO,x
    bsr StepBubbleList051B
    bne RiseAndClearAtListEnd051B_at0
    jmp VecClearObject
RiseAndClearAtListEnd051B_at0:
    rts
