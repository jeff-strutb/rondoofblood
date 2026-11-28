; @routine 0940:01C BySubstate0940D81C
; @run 71:D81C
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $D82E.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate0940D81CHandlers = $D82E ; a handler per substate
BySubstate0940D81C:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate0940D81CHandlers,y
    sta z:zTemp00
    lda a:BySubstate0940D81CHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0940:034 PickMinionKind0940
; @run 71:D834
; WHAT IT DOES: Picks one of two kinds at random and goes to the next substate.
; HOW: NextRandom.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PickMinionKind0940:
    jsr VecNextRandom
    and #$01
    sta a:OBJ_PIECE_PLACE,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0940:040 StartMinion0940
; @run 71:D840
; @stop D86D
; WHAT IT DOES: Starts the picked kind in an empty slot of $34-$37, facing this way, each keeping the other's slot; with none, state 2 ($D86F).
; HOW: A walk of the kinds.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartMinion0940Kinds = $D86D ; two kinds
StartMinion0940:
    ldy a:OBJ_PIECE_PLACE,x
    lda a:StartMinion0940Kinds,y
    sta z:zTemp00
    ldy #$34
StartMinion0940_at0:
    lda a:OBJ_KIND,y
    beq StartMinion0940_at1
    iny
    cpy #$38
    bcc StartMinion0940_at0
    bra EndSummon0940
StartMinion0940_at1:
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    txa
    sta a:OBJ_PIECE_NEXT,y
    tya
    sta a:OBJ_SWOOP_SPEED_LO,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0940:06F EndSummon0940
; @run 71:D86F
; WHAT IT DOES: State 2, substate 0.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EndSummon0940:
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 0940:078 WaitWhileMinionLives0940
; @run 71:D878
; WHAT IT DOES: Once the minion's slot no longer holds its kind, state 2.
; HOW: A compare of the kind.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WaitWhileMinionLives0940Kinds = $D86D ; two kinds
WaitWhileMinionLives0940:
    ldy a:OBJ_PIECE_PLACE,x
    lda a:WaitWhileMinionLives0940Kinds,y
    sta z:zTemp00
    ldy a:OBJ_SWOOP_SPEED_LO,x
    lda a:OBJ_KIND,y
    cmp z:zTemp00
    bne EndSummon0940
    rts

; @routine 0940:08B DripEmitter0940
; @run 71:D88B
; WHAT IT DOES: State 0 loads it off the right with a wait. Every wait starts a kind $3C drop with a random argument in an empty slot of $30-$33.
; HOW: ClearObjectInY, StartObjectAtPlace, NextRandom.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DripEmitter0940:
    lda a:OBJ_STATE,x
    bne DripEmitter0940_at0
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$20
    sta a:OBJ_X_LO,x
    lda #$20
    sta a:OBJ_PIECE_NEXT,x
    rts
DripEmitter0940_at0:
    dec a:OBJ_PIECE_NEXT,x
    bne DripEmitter0940_at2
    lda #$E0
    sta a:OBJ_PIECE_NEXT,x
    ldy #$30
DripEmitter0940_at1:
    lda a:OBJ_KIND,y
    beq DripEmitter0940_at3
    iny
    cpy #$34
    bcc DripEmitter0940_at1
DripEmitter0940_at2:
    rts
DripEmitter0940_at3:
    jsr VecClearObjectInY
    lda #$3C
    jsr VecStartObjectAtPlace
    jsr VecNextRandom
    and #$07
    sta a:OBJ_START_ARG,y
    rts

; @routine 0940:0CB SplashOfDrop0940
; @run 71:D8CB
; WHAT IT DOES: State 0 loads it. While its drop (OBJ_WORK_D) is kind $3C, steps the list at $D904 with the frame's X offset; else cleared.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SplashOfDrop0940List = $D904 ; a frame list
SplashOfDrop0940Dx = $D925 ; X offset by frame
SplashOfDrop0940:
    lda a:OBJ_STATE,x
    bne SplashOfDrop0940_at0
    jsr VecLoadObjectRecord
    lda #$02
    sta a:OBJ_HITBOX_DY,x
    rts
SplashOfDrop0940_at0:
    ldy a:OBJ_PIECE_NEXT,x
    lda a:OBJ_KIND,y
    cmp #$3C
    beq SplashOfDrop0940_at1
    jmp VecClearObject
SplashOfDrop0940_at1:
    lda #$04
    sta z:zTemp10
    lda #$D9
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne SplashOfDrop0940_at2
    jmp VecClearObject
SplashOfDrop0940_at2:
    lda a:OBJ_FRAME,x
    sec
    sbc #$8A
    tay
    lda a:SplashOfDrop0940Dx,y
    sta a:OBJ_HITBOX_DX,x
    rts

; @routine 0940:129 TrailOfDrop0940
; @run 71:D929
; WHAT IT DOES: As SplashOfDrop0940 with the list at $D963 and both offsets.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
TrailOfDrop0940List = $D963 ; a frame list
TrailOfDrop0940Offsets = $D98C ; X and Y offset by frame
TrailOfDrop0940:
    lda a:OBJ_STATE,x
    bne TrailOfDrop0940_at0
    jmp VecLoadObjectRecord
TrailOfDrop0940_at0:
    ldy a:OBJ_PIECE_NEXT,x
    lda a:OBJ_KIND,y
    cmp #$3C
    beq TrailOfDrop0940_at1
    jmp VecClearObject
TrailOfDrop0940_at1:
    lda #$63
    sta z:zTemp10
    lda #$D9
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne TrailOfDrop0940_at2
    jmp VecClearObject
TrailOfDrop0940_at2:
    lda a:OBJ_FRAME,x
    sec
    sbc #$8E
    asl a
    tay
    lda a:TrailOfDrop0940Offsets,y
    sta a:OBJ_HITBOX_DX,x
    lda a:TrailOfDrop0940Offsets+1,y
    sta a:OBJ_HITBOX_DY,x
    rts

; @routine 0940:19C ReadyDoorThenExit0940
; @run 71:D99C
; WHAT IT DOES: Sets the door ready and runs ExitWhenDoorHit093A.
; HOW: A store and a tail jump.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ReadyDoorThenExit0940:
    lda #$01
    sta a:PlayerDoorReady
    jmp ExitWhenDoorHit093A

; @routine 0940:1A4 ReadyDoorThenTrigger0940
; @run 71:D9A4
; WHAT IT DOES: Sets the door ready and runs the stage trigger object ($8AFC).
; HOW: A store and a tail jump.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ReadyDoorThenTrigger0940:
    lda #$01
    sta a:PlayerDoorReady
    jmp StageTriggerObjectB

; @routine 0940:1AC ByState0940D9AC
; @run 71:D9AC
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $D9BE.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState0940D9ACHandlers = $D9BE ; a handler per state
ByState0940D9AC:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState0940D9ACHandlers,y
    sta z:zTemp00
    lda a:ByState0940D9ACHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0940:1C6 ElevatorWaitForRider0940
; @run 71:D9C6
; WHAT IT DOES: A platform: frame 7 for odd numbers. Once the player stands on it ($30 by $16), starts a kind $33 below and: even numbers sink at $0140 with a wait; others at $DA19.
; HOW: ContactWithPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ElevatorWaitForRider0940:
    tst #$01,a:OBJ_START_ARG,x
    beq ElevatorWaitForRider0940_at0
    lda #$07
    sta a:OBJ_FRAME,x
ElevatorWaitForRider0940_at0:
    lda #$30
    sta z:zTemp08
    lda #$16
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc NextStateOfPlatform0940_at0
    ldy #$10
    jsr VecClearObjectInY
    lda #$33
    jsr VecStartObjectAtPlace
    tst #$01,a:OBJ_START_ARG,x
    bne HoldPlayerForOtherRide0940
    lda #$01
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    lda #$01
    sta a:OBJ_PIECE_NEXT,x
    lda #$80
    sta a:OBJ_PIECE_PLACE,x

; @routine 0940:202 HoldPlayerForRide0940
; @run 71:DA02
; WHAT IT DOES: The player's piece place A, start pose 2 and swoop count 0; loop flag bit 0, then on ($DA15).
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HoldPlayerForRide0940:
    sta a:OBJ_PIECE_PLACE
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_SWOOP_SPEED_LO
    lda a:OBJ_LOOP_FLAGS,x
    ora #$01
    sta a:OBJ_LOOP_FLAGS,x

; @routine 0940:215 NextStateOfPlatform0940
; @run 71:DA15
; WHAT IT DOES: Goes on.
; HOW: An increment.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NextStateOfPlatform0940:
    inc a:OBJ_STATE,x
NextStateOfPlatform0940_at0:
    rts

; @routine 0940:219 HoldPlayerForOtherRide0940
; @run 71:DA19
; WHAT IT DOES: Holds the player with $40 ($DA02), marks the other object's argument and goes on.
; HOW: HoldPlayerForRide0940.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HoldPlayerForOtherRide0940:
    lda #$40
    bsr HoldPlayerForRide0940
    lda #$01
    sta a:OBJ_START_ARG,y
    bra NextStateOfPlatform0940

; @routine 0940:224 FloatThenNext0940
; @run 71:DA24
; WHAT IT DOES: With the player's weight, floats to Y $9C; the wait over, on.
; HOW: FloatToLine0940.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FloatThenNext0940:
    lda #$30
    sta z:zTemp08
    lda #$16
    sta z:zTemp09
    jsr ContactWithPlayer
    lda #$9C
    jsr FloatToLine0940
    dec a:OBJ_PIECE_PLACE,x
    bne FloatThenNext0940_at0
    inc a:OBJ_STATE,x
FloatThenNext0940_at0:
    rts

; @routine 0940:23D RideAndCarryPlayer0940
; @run 71:DA3D
; WHAT IT DOES: Follows the scroll bands ($DB20), floats to Y $9C and drifts ($DAC6). After its wait, a standing player is pushed along, with a dip as they step on.
; HOW: ContactWithPlayer and zPlayerPush.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RideAndCarryPlayer0940:
    jsr MoveAgainstScrollBand0940
    lda #$9C
    jsr FloatToLine0940
    jsr DriftWithTheCurrent0940
    lda a:OBJ_PIECE_PLACE,x
    beq RideAndCarryPlayer0940_at0
    dec a:OBJ_PIECE_PLACE,x
    rts
RideAndCarryPlayer0940_at0:
    lda #$30
    sta z:zTemp08
    lda #$16
    sta z:zTemp09
    jsr ContactWithPlayer
    bcs RideAndCarryPlayer0940_at2
    lda a:OBJ_PIECE_NEXT,x
    beq RideAndCarryPlayer0940_at1
    lda #$08
    sta a:OBJ_PIECE_PLACE,x
RideAndCarryPlayer0940_at1:
    stz a:OBJ_PIECE_NEXT,x
    rts
RideAndCarryPlayer0940_at2:
    lda z:zPlayerPush
    ora #$10
    and #$F9
    sta z:zPlayerPush
    lda a:OBJ_VX_LO,x
    sta z:zPushXLo
    lda a:OBJ_VX_HI,x
    sta z:zPushXHi
    lda a:OBJ_PIECE_NEXT,x
    bne RideAndCarryPlayer0940_at3
    lda a:OBJ_Y_LO,x
    cmp #$9C
    bne RideAndCarryPlayer0940_at3
    lda #$01
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
RideAndCarryPlayer0940_at3:
    lda #$01
    sta a:OBJ_PIECE_NEXT,x
    rts

; @routine 0940:2C6 DriftWithTheCurrent0940
; @run 71:DAC6
; WHAT IT DOES: Steps the list at $DB15 (odd numbers' frames 6 on). At the argument's end frame turns back at -$0190; else eases toward rest from the left, and moves sideways.
; HOW: ApplyVelocityX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DriftWithTheCurrent0940List = $DB15 ; a frame list
DriftWithTheCurrent0940Ends = $DB1E ; end frame by argument
DriftWithTheCurrent0940:
    lda #$DB
    sta z:zTemp11
    lda #$15
    sta z:zTemp10
    jsr VecStepObjectFrames
    bcc DriftWithTheCurrent0940_at1
    ldy a:OBJ_START_ARG,x
    beq DriftWithTheCurrent0940_at0
    lda a:OBJ_FRAME,x
    clc
    adc #$06
    sta a:OBJ_FRAME,x
DriftWithTheCurrent0940_at0:
    lda a:OBJ_FRAME,x
    cmp a:DriftWithTheCurrent0940Ends,y
    bne DriftWithTheCurrent0940_at1
    inc a:OBJ_SWOOP_SPEED_LO,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$70
    sta a:OBJ_VX_LO,x
    bra DriftWithTheCurrent0940_at2
DriftWithTheCurrent0940_at1:
    lda a:OBJ_VX_LO,x
    clc
    adc #$04
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc #$00
    sta a:OBJ_VX_HI,x
    bmi DriftWithTheCurrent0940_at2
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
DriftWithTheCurrent0940_at2:
    jsr VecApplyVelocityX
    rts

; @routine 0940:320 MoveAgainstScrollBand0940
; @run 71:DB20
; WHAT IT DOES: Moves the object against its scroll band's movement this frame, in X and Y.
; HOW: The band's scroll less the main one, subtracted from the position.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
MoveAgainstScrollBand0940:
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    lda a:ScrollXFrac,y
    sec
    sbc a:ScrollBandFirst,y
    sta z:zTemp00
    lda a:ScrollXLo,y
    sbc a:ScrollBgxLo,y
    sta z:zTemp01
    lda a:ScrollYFrac,y
    sec
    sbc a:ScrollBandFourth,y
    sta z:zTemp02
    sta a:ScrollBandYFracStep,y
    lda a:ScrollYLo,y
    sbc a:ScrollBgyLo,y
    sta z:zTemp03
    stz z:zTemp10
    lda z:zTemp01
    bpl MoveAgainstScrollBand0940_at0
    dec z:zTemp10
MoveAgainstScrollBand0940_at0:
    lda a:OBJ_X_FRAC,x
    sec
    sbc z:zTemp00
    sta a:OBJ_X_FRAC,x
    lda a:OBJ_X_LO,x
    sbc z:zTemp01
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp10
    sta a:OBJ_X_HI,x
    stz z:zTemp10
    lda z:zTemp03
    bpl MoveAgainstScrollBand0940_at1
    dec z:zTemp10
MoveAgainstScrollBand0940_at1:
    lda a:OBJ_Y_FRAC,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_FRAC,x
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp03
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp10
    sta a:OBJ_Y_HI,x
    rts

; @routine 0940:4C6 SpawnAtScreensEnds0940
; @run 71:DCC6
; WHAT IT DOES: On map screens other than 0 and 3, every wait starts a kind $46 object in a free slot from $20; the next wait is this one's argument, or $20 before screen 3.
; HOW: FindFreeSlotFrom20.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SpawnAtScreensEnds0940:
    lda a:OBJ_STATE,x
    bne SpawnAtScreensEnds0940_at0
    jmp VecLoadObjectRecord
SpawnAtScreensEnds0940_at0:
    lda a:ScrollXHiScreen
    beq SpawnAtScreensEnds0940_at2
    cmp #$03
    beq SpawnAtScreensEnds0940_at2
    lda a:OBJ_PIECE_NEXT,x
    beq SpawnAtScreensEnds0940_at1
    dec a:OBJ_PIECE_NEXT,x
    rts
SpawnAtScreensEnds0940_at1:
    jsr VecFindFreeSlotFrom20
    bcc SpawnAtScreensEnds0940_at2
    lda #$46
    sta a:OBJ_KIND,y
    lda a:OBJ_START_ARG,x
    sta a:OBJ_PIECE_NEXT,x
    lda a:ScrollXHiScreen
    cmp #$03
    bcs SpawnAtScreensEnds0940_at2
    lda #$20
    sta a:OBJ_PIECE_NEXT,x
SpawnAtScreensEnds0940_at2:
    rts

; @routine 0940:4FD FishLeapOut0940
; @run 71:DCFD
; @stop DD3E
; WHAT IT DOES: State 0: loads it with attribute $8F at a random place, facing the player, leaping at -$0A with sound $98. Later at $DD4A.
; HOW: Position tables.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FishLeapOut0940Ys = $DD3E ; four Y
FishLeapOut0940Xs = $DD42 ; eight X
FishLeapOut0940:
    lda a:OBJ_STATE,x
    bne FishFlight0940
    jsr VecLoadObjectRecord
    lda #$8F
    sta a:OBJ_SPRITE_ATTR,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:FishLeapOut0940Ys,y
    sta a:OBJ_Y_LO,x
    jsr VecNextRandom
    and #$07
    tay
    lda a:FishLeapOut0940Xs,y
    sta a:OBJ_X_LO,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$F6
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$40
    sta a:OBJ_FALL_LO,x
    lda #$98
    jmp VecSoundCommand

; @routine 0940:54A FishFlight0940
; @run 71:DD4A
; @stop DDAD
; WHAT IT DOES: State 1: a hit turns it into sprite set 3 rising slowly; else its paling counts out, at Y $A0-$AF bursting debris, moving and falling while the list at $DDAD steps. Later at $DDC0.
; HOW: StartDebrisBurst.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FishFlight0940List = $DDAD ; a frame list
FishFlight0940:
    dec a
    bne FishSink0940
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq FishFlight0940_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_HITBOX_ON,x
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    stz a:OBJ_FRAME,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    inc a:OBJ_STATE,x
    rts
FishFlight0940_at0:
    lda a:OBJ_FALL_LO,x
    beq FishFlight0940_at1
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc #$8C
    sta a:OBJ_SPRITE_ATTR,x
    dec a:OBJ_FALL_LO,x
FishFlight0940_at1:
    lda a:OBJ_Y_LO,x
    and #$F0
    cmp #$A0
    bne FishFlight0940_at2
    lda #$23
    jsr VecSoundCommand
    cla
    jsr StartDebrisBurst
    lda #$8F
    sta a:OBJ_SPRITE_ATTR,x
FishFlight0940_at2:
    jsr VecApplyVelocity
    jsr VecApplyGravity
    lda #$AD
    sta z:zTemp10
    lda #$DD
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0940:5C0 FishSink0940
; @run 71:DDC0
; WHAT IT DOES: Palette $8B, moves down and is cleared when the list at $DDD9 ends.
; HOW: ApplyVelocityY.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FishSink0940List = $DDD9 ; a frame list
FishSink0940:
    lda #$8B
    sta a:OBJ_SPRITE_ATTR,x
    jsr VecApplyVelocityY
    lda #$D9
    sta z:zTemp10
    lda #$DD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne FishSink0940_at0
    jmp VecClearObject
FishSink0940_at0:
    rts
