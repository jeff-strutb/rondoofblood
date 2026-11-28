; @routine 07DF:029 BatSwoop07DF
; @run 71:D029
; WHAT IT DOES: Steps the list at $D08F, bobbing on the cosine and flying. Within $60 of the player, one time in 128, takes a
; random dive speed (turned by facing) and goes on.
; HOW: ScaleByCosine, NextRandom.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BatSwoop07DFList = $D08F ; a frame list
BatSwoop07DFDives = $D098 ; VX word and VY word, eight dives
BatSwoop07DF:
    lda #$D0
    sta z:zTemp11
    lda #$8F
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$40
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_PIECE_NEXT,x
    jsr VecScaleByCosine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    inc a:OBJ_PIECE_NEXT,x
    jsr VecApplyVelocityX
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs BatSwoop07DF_at1
    jsr VecNextRandom
    cmp #$02
    bcs BatSwoop07DF_at1
    jsr VecNextRandom
    and #$07
    asl a
    asl a
    tay
    lda a:BatSwoop07DFDives,y
    sta a:OBJ_VX_LO,x
    lda a:BatSwoop07DFDives+1,y
    sta a:OBJ_VX_HI,x
    lda a:BatSwoop07DFDives+2,y
    sta a:OBJ_VY_LO,x
    lda a:BatSwoop07DFDives+3,y
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT,x
    beq BatSwoop07DF_at0
    jsr VecNegateVelocityX
BatSwoop07DF_at0:
    inc a:OBJ_STATE,x
BatSwoop07DF_at1:
    rts

; @routine 07DF:0B8 FallBurstAtLine07DF
; @run 71:D0B8
; WHAT IT DOES: Steps the list at $D08F and falls, bursting debris at Y $D0-$DF.
; HOW: DebrisWithSound07DE, ApplyGravityUnlimited.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallBurstAtLine07DFList = $D08F ; a frame list
FallBurstAtLine07DF:
    lda #$D0
    sta z:zTemp11
    lda #$8F
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_Y_LO,x
    and #$F0
    cmp #$D0
    bne FallBurstAtLine07DF_at0
    jsr DebrisWithSound07DE
FallBurstAtLine07DF_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 07DF:2E4 CandleBox07DF
; @run 71:D2E4
; WHAT IT DOES: State 0 loads the record as frame 1; later steps the list at $D344 and checks for breaking ($D2FD).
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CandleBox07DFList = $D344 ; a frame list
CandleBox07DF:
    lda a:OBJ_STATE,x
    bne CandleBox07DF_at0
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_FRAME,x
    rts
CandleBox07DF_at0:
    lda #$D3
    sta z:zTemp11
    lda #$44
    sta z:zTemp10
    jsr VecStepObjectFrames

; @routine 07DF:2FD BreakWhenHitOrPassed07DF
; @run 71:D2FD
; WHAT IT DOES: Passed by the object in slot $4D within $50, or five hits taken: records the event, becomes the kind
; OBJ_START_ARG holds, and starts in a free slot from $50 a kind $9C rising (kind $38) or a kind $9B.
; HOW: RightOfObject4DWithin, FindFreeSlotFrom50.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BreakWhenHitOrPassed07DF:
    lda #$50
    jsr RightOfObject4DWithin
    bcs BreakWhenHitOrPassed07DF_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc StartTypeAndLaunchUp_end
BreakWhenHitOrPassed07DF_at0:
    jsr VecRecordObjectEvent
    lda a:OBJ_KIND,x
    sta z:zTemp00
    lda a:OBJ_START_ARG,x
    sta a:OBJ_KIND,x
    lda #$10
    sta a:OBJ_INVULN_TIMER,x
    stz a:OBJ_STATE,x
    jsr VecFindFreeSlotFrom50
    bcc StartTypeAndLaunchUp_end
    lda z:zTemp00
    cmp #$38
    beq StartTypeAndLaunchUp
    lda #$9B
    jmp VecStartObjectAtPlace

; @routine 07DF:36D StandWithBox07DF
; @run 71:D36D
; WHAT IT DOES: State 0 loads the record as sprite set 6, half height $14; later steps the list at $D38E and checks for
; breaking ($D2FD).
; HOW: StepObjectFrames, then into BreakWhenHitOrPassed07DF.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StandWithBox07DFList = $D38E ; a frame list
StandWithBox07DF:
    lda a:OBJ_STATE,x
    bne StandWithBox07DF_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$14
    sta a:OBJ_HITBOX_HALF_H,x
    rts
StandWithBox07DF_at0:
    lda #$D3
    sta z:zTemp11
    lda #$8E
    sta z:zTemp10
    jsr VecStepObjectFrames
    jmp BreakWhenHitOrPassed07DF

; @routine 07DF:402 ByState07DFD402
; @run 71:D402
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $D414.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DFD402States = $D414 ; a handler per state
ByState07DFD402:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DFD402States,y
    sta z:zTemp00
    lda a:ByState07DFD402States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DF:41A LoadBreakable07DF
; @run 71:D41A
; WHAT IT DOES: Loads the record as frame 5, the hit box and wait the argument picks.
; HOW: Tables by argument.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadBreakable07DFWaits = $D3EE ; wait by argument
LoadBreakable07DFSizes = $D3F2 ; half width and height by argument
LoadBreakable07DFOffsets = $D3F6 ; X and Y offsets by argument
LoadBreakable07DF:
    jsr VecLoadObjectRecord
    lda #$05
    sta a:OBJ_FRAME,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:LoadBreakable07DFSizes,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:LoadBreakable07DFSizes+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:LoadBreakable07DFOffsets,y
    sta a:OBJ_HITBOX_DX,x
    lda a:LoadBreakable07DFOffsets+1,y
    sta a:OBJ_HITBOX_DY,x
    ldy a:OBJ_START_ARG,x
    lda a:LoadBreakable07DFWaits,y
    sta a:OBJ_PIECE_NEXT,x
    sta a:OBJ_PIECE_PLACE,x
    rts

; @routine 07DF:44C CountWhileHit07DF
; @run 71:D44C
; WHAT IT DOES: While hit ($D4E1), counts OBJ_PIECE_PLACE down and at zero goes on; otherwise reloads it.
; HOW: TakeHitByArgument07DF.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountWhileHit07DF:
    jsr TakeHitByArgument07DF
    bcc CountWhileHit07DF_at1
    lda a:OBJ_PIECE_PLACE,x
    beq CountWhileHit07DF_at0
    dec a:OBJ_PIECE_PLACE,x
    rts
CountWhileHit07DF_at0:
    inc a:OBJ_STATE,x
    rts
CountWhileHit07DF_at1:
    lda a:OBJ_PIECE_NEXT,x
    sta a:OBJ_PIECE_PLACE,x
    rts

; @routine 07DF:465 ByArgument07DFD465
; @run 71:D465
; WHAT IT DOES: Goes to the handler for OBJ_START_ARG.
; HOW: A table of handlers at $D3FA.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByArgument07DFD465Handlers = $D3FA ; a handler per argument
ByArgument07DFD465:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:ByArgument07DFD465Handlers,y
    sta z:zTemp00
    lda a:ByArgument07DFD465Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DF:477 BySubstate07DFD477
; @run 71:D477
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $D489.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate07DFD477Substates = $D489 ; a handler per substate
BySubstate07DFD477:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate07DFD477Substates,y
    sta z:zTemp00
    lda a:BySubstate07DFD477Substates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DF:48D StartBreakingByArgument07DF
; @run 71:D48D
; WHAT IT DOES: The frame and wait the argument picks, sound $3A, next substate.
; HOW: Tables.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartBreakingByArgument07DFFrames = $D3EA ; frame by argument
StartBreakingByArgument07DFWaits = $D3EC ; wait by argument
StartBreakingByArgument07DF:
    ldy a:OBJ_START_ARG,x
    lda a:StartBreakingByArgument07DFFrames,y
    sta a:OBJ_FRAME,x
    lda a:StartBreakingByArgument07DFWaits,y
    sta a:OBJ_PIECE_PLACE,x
    lda #$3A
    jsr VecSoundCommand
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 07DF:4A5 RiseThenBecomeByArgument07DF
; @run 71:D4A5
; WHAT IT DOES: Kind $88 results step the list at $D524. Rises a dot a frame while the wait counts, then records the event and
; becomes the kind the argument picks at rest.
; HOW: A table of kinds.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RiseThenBecomeByArgument07DFKinds = $D3F0 ; kind by argument
RiseThenBecomeByArgument07DFList = $D524 ; a frame list
RiseThenBecomeByArgument07DF:
    ldy a:OBJ_START_ARG,x
    lda a:RiseThenBecomeByArgument07DFKinds,y
    cmp #$88
    bne RiseThenBecomeByArgument07DF_at0
    lda #$D5
    sta z:zTemp11
    lda #$24
    sta z:zTemp10
    jsr VecStepObjectFrames
RiseThenBecomeByArgument07DF_at0:
    dec a:OBJ_Y_LO,x
    dec a:OBJ_PIECE_PLACE,x
    bne RiseThenBecomeByArgument07DF_at1
    jsr VecRecordObjectEvent
    ldy a:OBJ_START_ARG,x
    lda a:RiseThenBecomeByArgument07DFKinds,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_HITBOX_DX,x
    stz a:OBJ_HITBOX_DY,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
RiseThenBecomeByArgument07DF_at1:
    rts

; @routine 07DF:4E1 TakeHitByArgument07DF
; @run 71:D4E1
; WHAT IT DOES: Clears the invulnerability; a hit type of 1-4 goes to the handler for OBJ_START_ARG, else carry clear.
; HOW: A table of handlers at $D3FE.
; WAITS: no.
; Entry: X object. Exit: C as the handler or clear.
TakeHitByArgument07DFHandlers = $D3FE ; a handler per argument
TakeHitByArgument07DF:
    stz a:OBJ_INVULN_TIMER,x
    lda a:OBJ_HIT_TAKEN,x
    beq TakeHitByArgument07DF_at0
    cmp #$05
    bcs TakeHitByArgument07DF_at0
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:TakeHitByArgument07DFHandlers,y
    sta z:zTemp00
    lda a:TakeHitByArgument07DFHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)
TakeHitByArgument07DF_at0:
    clc
    rts

; @routine 07DF:501 PlayerInState2_07DF
; @run 71:D501
; WHAT IT DOES: Carry set when the player is in state 2.
; HOW: A compare of slot 0's state.
; WAITS: no.
; Entry: none. Exit: C the answer, A the state.
PlayerInState2_07DF:
    lda a:OBJ_STATE
    cmp #$02
    beq PlayerInState2_07DF_at0
    clc
    rts
PlayerInState2_07DF_at0:
    sec
    rts

; @routine 07DF:52D ByState07DFD52D
; @run 71:D52D
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $D53F.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DFD52DStates = $D53F ; a handler per state
ByState07DFD52D:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DFD52DStates,y
    sta z:zTemp00
    lda a:ByState07DFD52DStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DF:549 LeaveWhenClearedAtEnd07DF
; @run 71:D549
; WHAT IT DOES: With the door ready, the camera past $06B0 and no object of kind $25 in slots $10-$57: sub-step 3, attack flags
; 2, and on.
; HOW: A walk of the kinds.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LeaveWhenClearedAtEnd07DF:
    lda a:PlayerDoorReady
    beq LeaveWhenClearedAtEnd07DF_at2
    lda a:ScrollXHi
    cmp #$06
    bcc LeaveWhenClearedAtEnd07DF_at2
    bne LeaveWhenClearedAtEnd07DF_at0
    lda a:ScrollXLo
    cmp #$B0
    bcc LeaveWhenClearedAtEnd07DF_at2
LeaveWhenClearedAtEnd07DF_at0:
    ldy #$10
LeaveWhenClearedAtEnd07DF_at1:
    lda a:OBJ_KIND,y
    cmp #$25
    beq LeaveWhenClearedAtEnd07DF_at2
    iny
    cpy #$58
    bcc LeaveWhenClearedAtEnd07DF_at1
    lda #$03
    sta z:zSubStep
    lda #$02
    sta a:HitAttackFlags
    inc a:OBJ_STATE,x
LeaveWhenClearedAtEnd07DF_at2:
    rts

; @routine 07E0:065 FallingSpikeTrap07E0
; @run 71:D865
; WHAT IT DOES: State 0 loads the record. State 1 waits for the camera, then counts $20-$3F; state 2 counts and starts script
; $82 in the slot OBJ_START_ARG names with a count of $10; state 3 counts back to state 1 with sound $4F.
; HOW: Branches by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallingSpikeTrap07E0:
    lda a:OBJ_STATE,x
    bne FallingSpikeTrap07E0_at0
    jmp VecLoadObjectRecord
FallingSpikeTrap07E0_at0:
    dec a
    bne FallingSpikeTrap07E0_at2
    lda a:OBJ_X_LO,x
    bne FallingSpikeTrap07E0_at1
    lda z:zWaveCounter
    and #$1F
    clc
    adc #$20
    sta a:OBJ_PIECE_NEXT,x
    inc a:OBJ_STATE,x
FallingSpikeTrap07E0_at1:
    rts
FallingSpikeTrap07E0_at2:
    dec a
    bne FallingSpikeTrap07E0_at3
    dec a:OBJ_PIECE_NEXT,x
    bne FallingSpikeTrap07E0_at1
    phx
    lda a:OBJ_START_ARG,x
    tax
    lda #$82
    jsr VecStartScriptSlotKind1
    plx
    lda #$10
    sta a:OBJ_PIECE_NEXT,x
    inc a:OBJ_STATE,x
    rts
FallingSpikeTrap07E0_at3:
    dec a:OBJ_PIECE_NEXT,x
    bne FallingSpikeTrap07E0_at1
    lda #$01
    sta a:OBJ_STATE,x
    lda #$4F
    jmp VecSoundCommand

; @routine 07E0:0AE FlickeringTorch07E0
; @run 71:D8AE
; WHAT IT DOES: State 0 loads the record as sprite set 6. Then keeps the script of its slot running (one of two by the wave
; counter) and steps the list at $D8DF.
; HOW: StartScriptSlotKind1 when the slot is empty.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlickeringTorch07E0List = $D8DF ; a frame list
FlickeringTorch07E0Scripts = $D8EA ; two scripts
FlickeringTorch07E0:
    lda a:OBJ_STATE,x
    bne FlickeringTorch07E0_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    rts
FlickeringTorch07E0_at0:
    phx
    lda a:OBJ_START_ARG,x
    tax
    lda a:ScriptSlotKind,x
    bne FlickeringTorch07E0_at1
    lda z:zWaveCounter
    lsr a
    lsr a
    and #$01
    tay
    lda a:FlickeringTorch07E0Scripts,y
    jsr VecStartScriptSlotKind1
FlickeringTorch07E0_at1:
    plx
    lda #$D8
    sta z:zTemp11
    lda #$DF
    sta z:zTemp10
    jmp VecStepObjectFrames

; @routine 07E0:0EC RaisedWhenFlag07E0
; @run 71:D8EC
; WHAT IT DOES: State 0 loads the record as sprite set 6, frame $1D. Once zField8C is set, rises 2 a frame until off the top.
; HOW: Branches by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RaisedWhenFlag07E0:
    lda a:OBJ_STATE,x
    bne RaisedWhenFlag07E0_at1
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$1D
    sta a:OBJ_FRAME,x
RaisedWhenFlag07E0_at0:
    rts
RaisedWhenFlag07E0_at1:
    dec a
    bne RaisedWhenFlag07E0_at2
    lda z:zField8C
    beq RaisedWhenFlag07E0_at0
    inc a:OBJ_STATE,x
    rts
RaisedWhenFlag07E0_at2:
    dec a:OBJ_Y_LO,x
    dec a:OBJ_Y_LO,x
    bpl RaisedWhenFlag07E0_at0
    jmp VecClearObject

; @routine 07E0:115 PictureByArgument07E0
; @run 71:D915
; WHAT IT DOES: State 0 loads the record with the frame OBJ_START_ARG, sprite set 6.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PictureByArgument07E0:
    lda a:OBJ_STATE,x
    bne PictureByArgument07E0_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
PictureByArgument07E0_at0:
    rts

; @routine 07E0:129 ByState07E0D929
; @run 71:D929
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $D93B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07E0D929States = $D93B ; a handler per state
ByState07E0D929:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07E0D929States,y
    sta z:zTemp00
    lda a:ByState07E0D929States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07E0:147 ExitDoorWhenHit07E0
; @run 71:D947
; WHAT IT DOES: A first hit with kind 7 in slot 5 marks it caught. With the door ready and not held, a hit of 1 or 2 takes the
; exit OBJ_START_ARG's low digit plus one: sub-step 3, sound $90, a count of 4.
; HOW: Tests of the door and hit.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ExitDoorWhenHit07E0:
    lda a:OBJ_SWOOP_SPEED_LO,x
    bne ExitDoorWhenHit07E0_at0
    lda a:OBJ_HIT_TAKEN,x
    beq ExitDoorWhenHit07E0_at1
    lda a:OBJ_KIND+5
    cmp #$07
    bne ExitDoorWhenHit07E0_at1
    lda #$01
    sta a:PlayerKind7Caught
    inc a:OBJ_SWOOP_SPEED_LO,x
ExitDoorWhenHit07E0_at0:
    lda a:StageDoorHeld
    eor #$01
    and a:PlayerDoorReady
    beq ExitDoorWhenHit07E0_at1
    lda a:OBJ_HIT_TAKEN,x
    beq ExitDoorWhenHit07E0_at1
    cmp #$03
    bcs ExitDoorWhenHit07E0_at1
    lda a:OBJ_START_ARG,x
    and #$0F
    inc a
    sta z:zField8C
    lda #$03
    sta z:zSubStep
    lda #$90
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
    stz a:OBJ_PIECE_PLACE,x
    lda #$04
    sta a:OBJ_PIECE_NEXT,x
ExitDoorWhenHit07E0_at1:
    rts

; @routine 07E0:190 NextWhenPart2_07E0
; @run 71:D990
; WHAT IT DOES: Goes on once zField85 is 2.
; HOW: A compare.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NextWhenPart2_07E0:
    lda z:zField85
    cmp #$02
    bne SetExitWhenFlagCleared07E0_at1
    inc a:OBJ_STATE,x
    rts

; @routine 07E0:19A FlagAfterFourWaits07E0
; @run 71:D99A
; WHAT IT DOES: After four waits of OBJ_PIECE_NEXT, goes on and sets zField8C bit 7.
; HOW: A count in OBJ_PIECE_PLACE.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlagAfterFourWaits07E0:
    dec a:OBJ_PIECE_NEXT,x
    bne FlagAfterFourWaits07E0_at1
    inc a:OBJ_PIECE_PLACE,x
    lda a:OBJ_PIECE_PLACE,x
    cmp #$04
    bne FlagAfterFourWaits07E0_at0
    inc a:OBJ_STATE,x
    lda z:zField8C
    ora #$80
    sta z:zField8C
    rts
FlagAfterFourWaits07E0_at0:
    lda #$04
    sta a:OBJ_PIECE_NEXT,x
FlagAfterFourWaits07E0_at1:
    rts

; @routine 07E0:1B9 SetExitWhenFlagCleared07E0
; @run 71:D9B9
; WHAT IT DOES: Once zField8C bit 7 is clear, goes on; Richter's progress bit 0 sets this argument's high digit to $A. zField8C
; becomes the argument plus one.
; HOW: Reads of ProgressCharacter.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetExitWhenFlagCleared07E0:
    lda z:zField8C
    bmi SetExitWhenFlagCleared07E0_at1
    inc a:OBJ_STATE,x
    lda #$04
    sta a:OBJ_PIECE_NEXT,x
    lda a:ProgressCharacter
    and #$01
    beq SetExitWhenFlagCleared07E0_at0
    lda a:OBJ_START_ARG,x
    and #$0F
    ora #$A0
    sta a:OBJ_START_ARG,x
SetExitWhenFlagCleared07E0_at0:
    lda a:OBJ_START_ARG,x
    inc a
    sta z:zField8C
SetExitWhenFlagCleared07E0_at1:
    rts
