; @routine 051C:229 SoundThenNextSubstate051B
; @run 70:BA29
; WHAT IT DOES: Sound $1F when on the first screen, then the next substate.
; HOW: SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SoundThenNextSubstate051B:
    lda #$1F
    jsr VecSoundIfOnFirstScreen
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 051C:232 SoundAtStepNine051B
; @run 70:BA32
; WHAT IT DOES: Steps the list at $BA59; at its end goes to state 1; at step 9 sounds $1A.
; HOW: StepObjectFrames and a test of OBJ_ANIM_STEP.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SoundAtStepNine051B:
    lda #$59
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq SoundAtStepNine051B_at0
    bcc SoundAtStepNine051B_at1
    lda a:OBJ_ANIM_STEP,x
    cmp #$09
    bne SoundAtStepNine051B_at1
    lda #$1A
    jmp VecSoundCommand
SoundAtStepNine051B_at0:
    stz a:OBJ_WORK_E,x
    lda #$01
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SoundAtStepNine051B_at1:
    rts

; @routine 051C:27A BySubstate051BBA7A
; @run 70:BA7A
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $BA8C.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate051BBA7ASubstates = $BA8C ; a handler per substate
BySubstate051BBA7A:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate051BBA7ASubstates,y
    sta z:zTemp00
    lda a:BySubstate051BBA7ASubstates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051C:290 StartFortyCountAndCountNext051B
; @run 70:BA90
; WHAT IT DOES: No hit box, the animation from its start, a count of $40, the state of the slot two on counted up, next substate.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartFortyCountAndCountNext051B:
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE+2,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 051C:2A5 SinkWhileCounting051B
; @run 70:BAA5
; WHAT IT DOES: Counts OBJ_WORK_D out and clears the object at zero. Until then draws it with attribute bit 4, moves it down, adds
; 2 to its Y speed, moves it again and drops water drops ($BAD1).
; HOW: ApplyVelocityY twice around the speed's increase.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SinkWhileCounting051B:
    dec a:OBJ_WORK_D,x
    bne SinkWhileCounting051B_at0
    jsr VecClearObject
    rts
SinkWhileCounting051B_at0:
    lda a:OBJ_SPRITE_ATTR,x
    ora #$10
    sta a:OBJ_SPRITE_ATTR,x
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    clc
    adc #$02
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    jsr DripByWaveCounter051B
    rts

; @routine 051C:2D1 DripByWaveCounter051B
; @run 70:BAD1
; WHAT IT DOES: When the wave counter's low six bits match one of the eight values at $BB11, starts a kind $9E object in a free slot
; from $30 with the speeds of that value's row, sound $15 on the first screen, and moves it one step.
; HOW: A search of the table; X kept in zClimbStepBefore and the row in zBurstCount.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DripByWaveCounter051BTimes = $BB11 ; eight values of the wave counter
DripByWaveCounter051BSpeeds = $BB19 ; X high and Y high by row
DripByWaveCounter051B:
    cly
DripByWaveCounter051B_at0:
    lda z:zWaveCounter
    and #$3F
    cmp a:DripByWaveCounter051BTimes,y
    beq DripByWaveCounter051B_at1
    iny
    cpy #$08
    bcc DripByWaveCounter051B_at0
    rts
DripByWaveCounter051B_at1:
    stx z:zClimbStepBefore
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc DripByWaveCounter051B_at3
    lda a:OBJ_X_HI,x
    bne DripByWaveCounter051B_at2
    lda #$15
    jsr VecSoundIfOnFirstScreen
DripByWaveCounter051B_at2:
    lda #$9E
    jsr VecStartObjectAtPlace
    lda z:zBurstCount
    asl a
    tax
    lda a:DripByWaveCounter051BSpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:DripByWaveCounter051BSpeeds+1,x
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zClimbStepBefore
DripByWaveCounter051B_at3:
    rts

; @routine 051C:329 ByState051BBB29
; @run 70:BB29
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $BB3B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState051BBB29States = $BB3B ; a handler per state
ByState051BBB29:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState051BBB29States,y
    sta z:zTemp00
    lda a:ByState051BBB29States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051C:341 LoadWithBoxAndCount051BBB41
; @run 70:BB41
; WHAT IT DOES: Loads the record; a count of $40 and the hit box offset -4, $20.
; HOW: LoadObjectRecord and stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadWithBoxAndCount051BBB41:
    jsr VecLoadObjectRecord
    lda #$40
    sta a:OBJ_WORK_D,x
    lda #$FC
    sta a:OBJ_HITBOX_DX,x
    lda #$20
    sta a:OBJ_HITBOX_DY,x
    rts

; @routine 051C:354 FollowTwoBelowAndStep051B
; @run 70:BB54
; WHAT IT DOES: Takes the place of the object two slots down and steps the list at $BB67.
; HOW: CopyObjectPlaceBack and StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FollowTwoBelowAndStep051B:
    txa
    tay
    dey
    dey
    jsr VecCopyObjectPlaceBack
    lda #$67
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    rts

; @routine 051C:378 RiseAndExpire051B
; @run 70:BB78
; WHAT IT DOES: No hit box; moves by $BB91, draws with attribute bit 4 and clears the object when its count runs out.
; HOW: A call and a countdown of OBJ_WORK_D.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RiseAndExpire051B:
    stz a:OBJ_HITBOX_ON,x
    jsr WaterLevelByDistance051B
    lda a:OBJ_SPRITE_ATTR,x
    ora #$10
    sta a:OBJ_SPRITE_ATTR,x
    dec a:OBJ_WORK_D,x
    bne RiseAndExpire051B_at0
    jmp VecClearObject
RiseAndExpire051B_at0:
    rts

; @routine 051C:391 WaterLevelByDistance051B
; @run 70:BB91
; WHAT IT DOES: Sets the water level from the table at $BBB1, by the player's distance and the count in OBJ_WORK_J.
; HOW: A word table indexed by distance / 4 in steps of 16, plus the count's low bits doubled.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WaterLevelByDistance051BLevels = $BBB1 ; water level words
WaterLevelByDistance051B:
    lda a:OBJ_WORK_J,x
    and #$07
    asl a
    sta z:zTemp00
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    and #$F0
    clc
    adc z:zTemp00
    tay
    lda a:WaterLevelByDistance051BLevels,y
    sta a:RasterWaterWanted
    lda a:WaterLevelByDistance051BLevels+1,y
    sta a:LevelYHi
    rts

; @routine 051C:3F1 ByState051BBBF1
; @run 70:BBF1
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $BC03.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState051BBBF1States = $BC03 ; a handler per state
ByState051BBBF1:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState051BBBF1States,y
    sta z:zTemp00
    lda a:ByState051BBBF1States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051C:409 LaunchByStartArgAngle051B
; @run 70:BC09
; WHAT IT DOES: Loads the record, moves one step by speed high bytes $F4, $DA, then sets the velocity from the angle the table at
; $BC2B holds for OBJ_START_ARG, magnitude $0200.
; HOW: ApplyVelocity and SetVelocityFromAngle.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LaunchByStartArgAngle051BAngles = $BC2B ; an angle per start argument
LaunchByStartArgAngle051B:
    jsr VecLoadObjectRecord
    lda #$F4
    sta a:OBJ_VX_HI,x
    lda #$DA
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda #$00
    sta z:zTemp00
    lda #$02
    sta z:zTemp01
    ldy a:OBJ_START_ARG,x
    lda a:LaunchByStartArgAngle051BAngles,y
    jsr VecSetVelocityFromAngle
    rts

; @routine 051C:42E FlyUntilTerrain051B
; @run 70:BC2E
; WHAT IT DOES: Steps the list at $BC6C and moves; on terrain stops the sideways move, rises at -1 and goes to the next state as
; sprite set 3.
; HOW: ProbeTerrain request $2B with SpriteFlipMask 8.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyUntilTerrain051B:
    lda #$6C
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda #$08
    sta z:SpriteFlipMask
    lda #$2B
    jsr VecProbeTerrain
    and #$03
    beq FlyUntilTerrain051B_at0
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    stz a:OBJ_FRAME,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
FlyUntilTerrain051B_at0:
    rts

; @routine 051C:475 StepThenClear051BBC75
; @run 70:BC75
; WHAT IT DOES: Steps the list at $BC86 and clears the object when it ends.
; HOW: StepObjectFrames, ClearObject.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepThenClear051BBC75:
    lda #$86
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenClear051BBC75_at0
    jmp VecClearObject
StepThenClear051BBC75_at0:
    rts

; @routine 051C:495 SpawnWhileSlot16Absent051B
; @run 70:BC95
; WHAT IT DOES: State 0 loads the record. Then, unless the object in slot $10 is kind $24, counts the 16-bit wait in
; OBJ_PIECE_NEXT:PIECE_PLACE down and at zero starts a kind $46 object in a free slot from $30, waiting $C0 again.
; HOW: A 16-bit decrement and FindFreeSlotFrom30.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SpawnWhileSlot16Absent051B:
    lda a:OBJ_STATE,x
    bne SpawnWhileSlot16Absent051B_at0
    jmp VecLoadObjectRecord
SpawnWhileSlot16Absent051B_at0:
    lda a:OBJ_KIND+16
    cmp #$24
    beq SpawnWhileSlot16Absent051B_at2
    lda a:OBJ_WORK_E,x
    ora a:OBJ_WORK_D,x
    beq SpawnWhileSlot16Absent051B_at1
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    rts
SpawnWhileSlot16Absent051B_at1:
    jsr VecFindFreeSlotFrom30
    bcc SpawnWhileSlot16Absent051B_at2
    lda #$46
    jsr VecStartObjectAtPlace
    stz a:OBJ_WORK_D,x
    lda #$C0
    sta a:OBJ_WORK_E,x
SpawnWhileSlot16Absent051B_at2:
    rts

; @routine 051C:4D9 StepThenByState051BBCD9
; @run 70:BCD9
; WHAT IT DOES: Steps the list at $BCFE ($BCF2) and goes to the state's handler.
; HOW: A table of handlers at $BCEE.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepThenByState051BBCD9States = $BCEE ; a handler per state
StepThenByState051BBCD9:
    jsr StepListBCFE051B
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:StepThenByState051BBCD9States,y
    sta z:zTemp00
    lda a:StepThenByState051BBCD9States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051C:4F2 StepListBCFE051B
; @run 70:BCF2
; WHAT IT DOES: Steps the list at $BCFE.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepListBCFE051B:
    lda #$FE
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    rts

; @routine 051C:507 ScrollShortOfRightEnd051B
; @run 70:BD07
; WHAT IT DOES: Carry set when the camera is two screens or more short of the right end, or one short with its low byte under ;
; clear otherwise.
; HOW: ScrollXMaxHi less ScrollXHiScreen, and ScrollXLo.
; WAITS: no.
; Entry: none. Exit: C the answer, A the difference or ScrollXLo.
ScrollShortOfRightEnd051B:
    lda a:ScrollXMaxHi
    sec
    sbc a:ScrollXHiScreen
    cmp #$02
    bcs ScrollShortOfRightEnd051B_at0
    cmp #$00
    beq ScrollShortOfRightEnd051B_at1
    lda a:ScrollXLo
    cmp #$E0
    bcs ScrollShortOfRightEnd051B_at1
    sec
ScrollShortOfRightEnd051B_at0:
    rts
ScrollShortOfRightEnd051B_at1:
    clc
    rts

; @routine 051C:521 EnterFromEdgeBySide051B
; @run 70:BD21
; WHAT IT DOES: Loads the record and places the object at X $FFE0 with X speed $0140 and OBJ_FACING_LEFT 1, or at X $0120 with
; speed $FEC0: the former when the player's X low is $C0 or more, or between $40 and $C0 with the camera near its
; right end or the player facing left. Y is the player's less $10.
; HOW: The player's X low and facing; ScrollShortOfRightEnd051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EnterFromEdgeBySide051B:
    jsr VecLoadObjectRecord
    lda a:OBJ_X_LO
    cmp #$40
    bcc EnterFromEdgeBySide051B_at1
    cmp #$C0
    bcs EnterFromEdgeBySide051B_at0
    bsr ScrollShortOfRightEnd051B
    bcc EnterFromEdgeBySide051B_at0
    lda a:OBJ_FACING_LEFT
    beq EnterFromEdgeBySide051B_at1
EnterFromEdgeBySide051B_at0:
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$FF
    sta a:OBJ_X_HI,x
    lda #$E0
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
    bra EnterFromEdgeBySide051B_at2
EnterFromEdgeBySide051B_at1:
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$20
    sta a:OBJ_X_LO,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x
EnterFromEdgeBySide051B_at2:
    lda a:OBJ_Y_LO
    sec
    sbc #$10
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    sbc #$00
    sta a:OBJ_Y_HI,x
    rts

; @routine 051C:579 BobOnCosine051B
; @run 70:BD79
; WHAT IT DOES: Y speed from the cosine of OBJ_WORK_D, magnitude 1.0; moves, and advances the angle by 2.
; HOW: ScaleByCosine and ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BobOnCosine051B:
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecScaleByCosine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    clc
    adc #$02
    sta a:OBJ_WORK_D,x
    rts

; @routine 051C:59E CountHitThenByState051BBD9E
; @run 70:BD9E
; WHAT IT DOES: Counts OBJ_WORK_D up, checks for a hit ($BDCE) and goes to the state's handler.
; HOW: A table of handlers at $BDB6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountHitThenByState051BBD9EStates = $BDB6 ; a handler per state
CountHitThenByState051BBD9E:
    inc a:OBJ_WORK_D,x
    jsr State11WhenHit051BBDCE
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CountHitThenByState051BBD9EStates,y
    sta z:zTemp00
    lda a:CountHitThenByState051BBD9EStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051C:5CE State11WhenHit051BBDCE
; @run 70:BDCE
; WHAT IT DOES: Under state $0B, a hit (OBJ_HIT_STATUS bit 7) goes to state $0B, substate 0.
; HOW: Two tests.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
State11WhenHit051BBDCE:
    lda a:OBJ_STATE,x
    cmp #$0B
    bcs State11WhenHit051BBDCE_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State11WhenHit051BBDCE_at0
    lda #$0B
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State11WhenHit051BBDCE_at0:
    rts
