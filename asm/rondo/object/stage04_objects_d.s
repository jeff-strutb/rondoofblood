; @routine 07DD:25F SwingAroundNextAndSound07DD
; @run 71:C25F
; WHAT IT DOES: Steps the list at $C21A and circles the object in the next slot, its height halved; each full turn sounds $4A and
; goes back a state.
; HOW: CopyObjectPlaceBack, ApplyVelocityY $23, SetVelocityFromAngle magnitude $1000.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SwingAroundNextAndSound07DAList = $C21A ; a frame list
SwingAroundNextAndSound07DD:
    lda #$C2
    sta z:zTemp11
    lda #$1A
    sta z:zTemp10
    jsr VecStepObjectFrames
    txa
    tay
    iny
    jsr VecCopyObjectPlaceBack
    lda #$23
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$10
    sta z:zTemp01
    lda #$00
    sta z:zTemp00
    lda a:OBJ_PIECE_NEXT,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_VY_HI,x
    asl a
    ror a:OBJ_VY_HI,x
    ror a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_PIECE_NEXT,x
    clc
    adc #$08
    sta a:OBJ_PIECE_NEXT,x
    bcc SwingAroundNextAndSound07DD_at0
    lda #$4A
    jsr VecSoundCommand
    dec a:OBJ_STATE,x
SwingAroundNextAndSound07DD_at0:
    rts

; @routine 07DD:2AB CountHitsThenByState07DD
; @run 71:C2AB
; WHAT IT DOES: Counts, takes the hurt time ($C2D0), drops without the arm ($C2E2) and goes to the state's handler.
; HOW: A table of handlers at $C2C6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountHitsThenByState07DAStates = $C2C6 ; a handler per state
CountHitsThenByState07DD:
    inc a:OBJ_PIECE_NEXT,x
    jsr HurtTimeByHit07DD
    jsr DropWithoutBase07DD
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CountHitsThenByState07DAStates,y
    sta z:zTemp00
    lda a:CountHitsThenByState07DAStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DD:2D0 HurtTimeByHit07DD
; @run 71:C2D0
; WHAT IT DOES: Sets the hurt time the table gives for a hit's type and $18 frames of invulnerability.
; HOW: OBJ_HIT_TAKEN indexes the table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HurtTimeByHit07DATimes = $A27D ; hurt time by hit type
HurtTimeByHit07DD:
    lda a:OBJ_HIT_TAKEN,x
    beq HurtTimeByHit07DD_at0
    tay
    lda a:HurtTimeByHit07DATimes,y
    sta a:OBJ_HURT_TIMER,x
    lda #$18
    sta a:OBJ_INVULN_TIMER,x
HurtTimeByHit07DD_at0:
    rts

; @routine 07DD:2E2 DropWithoutBase07DD
; @run 71:C2E2
; WHAT IT DOES: Under state 2, once the slot below no longer holds kind $AB, goes to state 2 with pace bits 3-6 cleared, the
; animation restarted, no hit box and a wait of $30.
; HOW: The kind of slot X-1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DropWithoutBase07DD:
    lda a:OBJ_STATE,x
    cmp #$02
    bcs DropWithoutBase07DD_at0
    lda a:OBJ_KIND-1,x
    cmp #$AB
    beq DropWithoutBase07DD_at0
    lda #$02
    sta a:OBJ_STATE,x
    lda a:OBJ_PACE,x
    and #$87
    sta a:OBJ_PACE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_HITBOX_ON,x
    lda #$30
    sta a:OBJ_PIECE_PLACE,x
DropWithoutBase07DD_at0:
    rts

; @routine 07DD:30C TailByBodyState07DD
; @run 71:C30C
; WHAT IT DOES: Steps the list at $C366; frame $59 unless the body two slots below is alive. While the slot below is in state 1,
; bobs and drifts by screen side; else sits at Y $8E.
; HOW: Reads of slots X-2 and X-1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
TailByBodyState07DABobs = $C360 ; Y step by phase
TailByBodyState07DASpeeds = $C362 ; VX words by screen side
TailByBodyState07DAList = $C366 ; a frame list
TailByBodyState07DD:
    lda #$C3
    sta z:zTemp11
    lda #$66
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_KIND-2,x
    cmp #$AA
    bne TailByBodyState07DD_at0
    lda a:OBJ_STATE-2,x
    cmp #$02
    bcc TailByBodyState07DD_at1
TailByBodyState07DD_at0:
    lda #$59
    sta a:OBJ_FRAME,x
TailByBodyState07DD_at1:
    lda a:OBJ_STATE-1,x
    cmp #$01
    bne TailByBodyState07DD_at2
    lda a:OBJ_PIECE_NEXT,x
    lsr a
    lsr a
    and #$01
    tay
    lda a:OBJ_Y_LO,x
    clc
    adc a:TailByBodyState07DABobs,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_X_HI,x
    and #$01
    asl a
    tay
    lda a:TailByBodyState07DASpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:TailByBodyState07DASpeeds,y
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    rts
TailByBodyState07DD_at2:
    lda #$8E
    sta a:OBJ_Y_LO,x
    rts

; @routine 07DD:36B NextStateAfterWait30_07DD
; @run 71:C36B
; WHAT IT DOES: Counts OBJ_PIECE_PLACE out, reloads $30 and goes on.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NextStateAfterWait30_07DD:
    dec a:OBJ_PIECE_PLACE,x
    bne NextStateAfterWait30_07DD_at0
    lda #$30
    sta a:OBJ_PIECE_PLACE,x
    inc a:OBJ_STATE,x
NextStateAfterWait30_07DD_at0:
    rts

; @routine 07DD:379 BurningWaterAtCounts07DD
; @run 71:C379
; @stop C3B4
; WHAT IT DOES: Steps the list at $C3B4; at its end clears the water level and the object. Meanwhile flickers the water level,
; counts down, and at eight counts throws embers ($C3D1).
; HOW: A search of the counts.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BurningWaterAtCounts07DAList = $C3B4 ; a frame list
BurningWaterAtCounts07DALevels = $C3C1 ; water level words
BurningWaterAtCounts07DATimes = $C3F5 ; eight counts
BurningWaterAtCounts07DD:
    lda #$C3
    sta z:zTemp11
    lda #$B4
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne BurningWaterAtCounts07DD_at0
    stz a:RasterWaterWanted
    stz a:LevelYHi
    jmp VecClearObject
BurningWaterAtCounts07DD_at0:
    lda a:OBJ_PIECE_PLACE,x
    and #$07
    asl a
    tay
    lda a:BurningWaterAtCounts07DALevels,y
    sta a:RasterWaterWanted
    lda a:BurningWaterAtCounts07DALevels+1,y
    sta a:LevelYHi
    dec a:OBJ_PIECE_PLACE,x
    cly
BurningWaterAtCounts07DD_at1:
    lda a:OBJ_PIECE_PLACE,x
    cmp a:BurningWaterAtCounts07DATimes,y
    beq ThrowEmberOfRow07DD
    iny
    cpy #$08
    bcc BurningWaterAtCounts07DD_at1
    rts

; @routine 07DD:3D1 ThrowEmberOfRow07DD
; @run 71:C3D1
; WHAT IT DOES: Starts a kind $9E ember with the speed pair Y picks and moves it.
; HOW: The row doubled kept in zBurstCount.
; WAITS: no.
; Entry: X object, Y the row. Exit: as the last instruction left them.
ThrowEmberOfRow07DASpeeds = $C3FD ; VX high and VY high by row
ThrowEmberOfRow07DD:
    tya
    asl a
    sta z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc ThrowEmberOfRow07DD_at0
    lda #$9E
    jsr VecStartObjectAtPlace
    phx
    ldx z:zBurstCount
    lda a:ThrowEmberOfRow07DASpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:ThrowEmberOfRow07DASpeeds+1,x
    sta a:OBJ_VY_HI,y
    plx
    sxy
    jsr VecApplyVelocity
    sxy
ThrowEmberOfRow07DD_at0:
    rts

; @routine 07DD:40D SpawnWhenSlotsFree07DD
; @run 71:C40D
; WHAT IT DOES: Counts OBJ_WORK_D out, then, a slot of $30-$34 free, starts a kind $29 object there with this argument and picks
; the next wait from the table.
; HOW: FindEmptyOf30to34_07DD and NextRandom.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SpawnWhenSlotsFree07DAWaits = $C445 ; four waits per argument
SpawnWhenSlotsFree07DD:
    lda a:OBJ_STATE,x
    bne SpawnWhenSlotsFree07DD_at0
    jmp VecLoadObjectRecord
SpawnWhenSlotsFree07DD_at0:
    lda a:OBJ_PIECE_NEXT,x
    beq SpawnWhenSlotsFree07DD_at1
    dec a:OBJ_PIECE_NEXT,x
    rts
SpawnWhenSlotsFree07DD_at1:
    jsr FindEmptyOf30to34_07DD
    bcc SpawnWhenSlotsFree07DD_at2
    jsr VecClearObjectInY
    lda #$29
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    sta a:OBJ_START_ARG,y
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    and #$03
    clc
    adc z:zTemp00
    tay
    lda a:SpawnWhenSlotsFree07DAWaits,y
    sta a:OBJ_PIECE_NEXT,x
SpawnWhenSlotsFree07DD_at2:
    rts

; @routine 07DD:44D FindEmptyOf30to34_07DD
; @run 71:C44D
; WHAT IT DOES: Y the first of slots $30-$34 holding no object, carry set; carry clear when none is.
; HOW: A walk of the kinds.
; WAITS: no.
; Entry: none. Exit: C found, Y the slot or $35.
FindEmptyOf30to34_07DD:
    ldy #$30
FindEmptyOf30to34_07DD_at0:
    lda a:OBJ_KIND,y
    beq FindEmptyOf30to34_07DD_at1
    iny
    cpy #$35
    bcc FindEmptyOf30to34_07DD_at0
    clc
    rts
FindEmptyOf30to34_07DD_at1:
    sec
    rts

; @routine 07DD:45D ByState07DAC45D
; @run 71:C45D
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $C46F.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DAC45DStates = $C46F ; a handler per state
ByState07DAC45D:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DAC45DStates,y
    sta z:zTemp00
    lda a:ByState07DAC45DStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DD:477 EnterFromRandomSide07DD
; @run 71:C477
; WHAT IT DOES: Loads the record; number 0 faces right, others at random, but half the time not at all while one of slots $14,
; $1C or $24 holds kind $35 (then cleared). Takes the X for number and facing.
; HOW: NextRandom and the kinds of three slots.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EnterFromRandomSide07DAPlaces = $C4C6 ; X words by number and facing
EnterFromRandomSide07DD:
    jsr VecLoadObjectRecord
    cla
    lda a:OBJ_START_ARG,x
    beq EnterFromRandomSide07DD_at3
    jsr VecNextRandom
    and #$01
    beq EnterFromRandomSide07DD_at2
    lda a:OBJ_KIND+20
    cmp #$35
    bne EnterFromRandomSide07DD_at0
    jmp VecClearObject
EnterFromRandomSide07DD_at0:
    lda a:OBJ_KIND+28
    cmp #$35
    bne EnterFromRandomSide07DD_at1
    jmp VecClearObject
EnterFromRandomSide07DD_at1:
    lda a:OBJ_KIND+36
    cmp #$35
    bne EnterFromRandomSide07DD_at2
    jmp VecClearObject
EnterFromRandomSide07DD_at2:
    jsr VecNextRandom
    and #$01
EnterFromRandomSide07DD_at3:
    sta a:OBJ_FACING_LEFT,x
    asl a
    sta z:zTemp00
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:EnterFromRandomSide07DAPlaces+1,y
    sta a:OBJ_X_HI,x
    lda a:EnterFromRandomSide07DAPlaces,y
    sta a:OBJ_X_LO,x
    rts

; @routine 07DD:4CE WalkOnLanding07DD
; @run 71:C4CE
; WHAT IT DOES: On landing, takes the walking speed for the facing and goes on.
; HOW: StopOnLanding with height $0C.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkOnLanding07DASpeeds = $C4EA ; VX words by facing
WalkOnLanding07DD:
    lda #$0C
    jsr VecStopOnLanding
    bcc WalkOnLanding07DD_at0
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:WalkOnLanding07DASpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkOnLanding07DASpeeds,y
    sta a:OBJ_VX_LO,x
    inc a:OBJ_STATE,x
WalkOnLanding07DD_at0:
    rts

; @routine 07DD:4EE WalkUntilNearSlot4D07DD
; @run 71:C4EE
; WHAT IT DOES: Within $60 of the object in slot $4D becomes kind $9C with sound $8E. Else steps the list for its argument,
; moves and lands: a wall sends it to state 3, off the ground to state 1.
; HOW: RightOfObject4DWithin, MoveAndLand with height $0C.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkUntilNearSlot4D07DALists = $C544 ; list addresses
WalkUntilNearSlot4D07DD:
    lda #$60
    jsr RightOfObject4DWithin
    bcc WalkUntilNearSlot4D07DD_at0
    lda #$8E
    jsr VecSoundCommand
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
WalkUntilNearSlot4D07DD_at0:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:WalkUntilNearSlot4D07DALists,y
    sta z:zTemp10
    lda a:WalkUntilNearSlot4D07DALists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$0C
    jsr VecMoveAndLand
    bcs WalkUntilNearSlot4D07DD_at2
    and #$01
    bne WalkUntilNearSlot4D07DD_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$01
    sta a:OBJ_STATE,x
WalkUntilNearSlot4D07DD_at1:
    rts
WalkUntilNearSlot4D07DD_at2:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_SUBSTATE,x
    lda #$03
    sta a:OBJ_STATE,x
    rts
