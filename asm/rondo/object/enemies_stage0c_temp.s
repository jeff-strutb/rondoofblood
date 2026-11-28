; @routine 0CAB:147 StartTileScriptByKind
; @run 70:B147
; WHAT IT DOES: Loads the object's record and sets its timer from a table at $B15D by its next kind; for kind 0 it draws the tile script 6 of group 0 (DrawTileScript).
; HOW: LoadObjectRecord, a table read (bank 70), DrawTileScript.
; WAITS: no.
; Entry: X object. Exit: kind 0: as DrawTileScript's; else the compare with 0.
StartTileScriptByKind:
    jsr VecLoadObjectRecord
    ldy a:OBJ_START_ARG,x
    lda a:$B15D,y
    sta a:OBJ_WORK_D,x
    cpy #$00
    bne StartTileScriptByKind_at0
    lda #$06
    jsr VecDrawTileScript
StartTileScriptByKind_at0:
    rts

; @routine 0CAB:161 CountThenTileScript
; @run 70:B161
; WHAT IT DOES: Counts the object's timer down; at zero it draws the tile script of group 0 numbered the next kind plus 7 (DrawTileScript) and goes on to the next state.
; HOW: DEC, DrawTileScript, INC.
; WAITS: no.
; Entry: X object. Exit: the count; at zero, the state's count.
CountThenTileScript:
    dec a:OBJ_WORK_D,x
    bne CountThenTileScript_at0
    lda a:OBJ_START_ARG,x
    clc
    adc #$06
    inc a
    cly
    jsr VecDrawTileScript
    inc a:OBJ_STATE,x
CountThenTileScript_at0:
    rts

; @routine 0CAB:175 FallingWithGravity
; @run 70:B175
; WHAT IT DOES: Steps the animation list at $B187, applies unlimited gravity and moves the object.
; HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity, Y as the step left it.
FallingWithGravity:
    lda #$87
    sta z:zTemp10
    lda #$B1
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocity
    rts

; @routine 0CAB:18E StepWithGuardTable
; @run 70:B18E
; WHAT IT DOES: One frame of an object with the guard states: OBJ_WORK_H goes up, StartWhenHurt, EnterStateSevenOnFlag, EnterStateFiveOnPlayerState9 and SetHitboxFromFrame run, then
; the code the state table at $B2F4 names (JumpByStateTableD); afterwards a sound list from a table at $B1B3 by the frame, when not 0, starts on channel 5 (StartListChannelKind2, list plus $81).
; HOW: INC, five calls, a call through the state table, a table read (bank 70), StartListChannelKind2.
; WAITS: no.
; Entry: X object. Exit: the list start's, or the table load's.
StepWithGuardTable:
    inc a:OBJ_WORK_H,x
    jsr StartWhenHurt
    jsr EnterStateSevenOnFlag
    jsr EnterStateFiveOnPlayerState9
    jsr SetHitboxFromFrame
    jsr JumpByStateTableD
    ldy a:OBJ_FRAME,x
    lda a:$B1B3,y
    beq StepWithGuardTable_at0
    phx
    clc
    adc #$81
    ldx #$05
    jsr VecStartListChannelKind2
    plx
StepWithGuardTable_at0:
    rts

; @routine 0CAB:203 StartWhenHurt
; @run 70:B203
; WHAT IT DOES: An object in state 3 that has taken a hit of 5 or more resets its animation, enters state 6 and clears OBJ_SUBSTATE.
; HOW: Two compares.
; WAITS: no.
; Entry: X object. Exit: the last compare or load.
StartWhenHurt:
    lda a:OBJ_STATE,x
    cmp #$03
    bne StartWhenHurt_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc StartWhenHurt_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StartWhenHurt_at0:
    rts

; @routine 0CAB:220 EnterStateSevenOnFlag
; @run 70:B220
; WHAT IT DOES: When bit 7 of the object's status flags is set and its state is below 7, it faces right, resets its animation, enters state 7 and clears OBJ_SUBSTATE.
; HOW: Two compares.
; Untraced: every arm past the flag test (B227 to B23E).
; WAITS: no.
; Entry: X object. Exit: the last AND, compare or load.
EnterStateSevenOnFlag:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq EnterStateSevenOnFlag_at0
    lda a:OBJ_STATE,x
    cmp #$07
    bcs EnterStateSevenOnFlag_at0
    stz a:OBJ_FACING_LEFT,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$07
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
EnterStateSevenOnFlag_at0:
    rts

; @routine 0CAB:2E2 JumpByStateTableD
; @run 70:B2E2
; WHAT IT DOES: Goes to the code a table at $B2F4 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTableD:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$B2F4,y
    sta z:zTemp00
    lda a:$B2F5,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAB:304 StartWithGuard
; @run 70:B304
; WHAT IT DOES: Loads the object's record and starts a kind $25 object at its place in slot $11; its timer ends as $80.
; HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A $80, Y $11, C from the record, N set.
StartWithGuard:
    jsr VecLoadObjectRecord
    ldy #$11
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    lda #$E0
    sta a:OBJ_WORK_D,x
    lda #$80
    sta a:OBJ_WORK_D,x
    rts

; @routine 0CAB:31C CountThenCue
; @run 70:B31C
; WHAT IT DOES: Counts the object's timer down; at zero it clears bits 3 and 4 of zField8E, plays the music again (CdPlayAgain), sends sound $BF, sets the angle word to
; $8000 (OBJ_WORK_F $80, OBJ_WORK_G 0) and enters its next state.
; HOW: DEC, two memory bit clears, CdPlayAgain, SoundCommand, INC.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
CountThenCue:
    dec a:OBJ_WORK_D,x
    bne CountThenCue_at0
    rmb3 z:zField8E
    rmb4 z:zField8E
    jsr VecCdPlayAgain
    lda #$BF
    jsr VecSoundCommand
    lda #$80
    sta a:OBJ_WORK_F,x
    stz a:OBJ_WORK_G,x
    inc a:OBJ_STATE,x
CountThenCue_at0:
    rts

; @routine 0CAB:339 SlideTurnByTable
; @run 70:B339
; WHAT IT DOES: Steps the animation list at $B3E0, sets the object's Y to $0070 and its Y speed to the cosine of the angle OBJ_WORK_E scaled by the 16-bit
; length OBJ_WORK_F:OBJ_WORK_G, moves it in Y, turns the angle half a circle, and runs the code a table at $B383 names for OBJ_SUBSTATE (SlideAngleDown,
; SlideAngleUp, TurnAngleBack).
; HOW: StepObjectFrames, ScaleByCosine, ApplyVelocityY, then a jump through the table.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; Y the doubled OBJ_SUBSTATE.
SlideTurnByTable:
    lda #$E0
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    stz a:OBJ_Y_HI,x
    lda #$70
    sta a:OBJ_Y_LO,x
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_E,x
    eor #$80
    sta a:OBJ_WORK_E,x
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B383,y
    sta z:zTemp00
    lda a:$B384,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAB:389 SlideAngleDown
; @run 70:B389
; WHAT IT DOES: Takes one off the 16-bit angle word (OBJ_WORK_F high, OBJ_WORK_G low, in steps of $0100 less 0 on the low byte as the code subtracts $0100); when the high byte
; reaches 0 the object's OBJ_SUBSTATE goes up one.
; HOW: A 16-bit subtract.
; WAITS: no.
; Entry: X object. Exit: the last subtract or INC.
SlideAngleDown:
    lda a:OBJ_WORK_G,x
    sec
    sbc #$00
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    sbc #$01
    sta a:OBJ_WORK_F,x
    bne SlideAngleDown_at0
    inc a:OBJ_SUBSTATE,x
SlideAngleDown_at0:
    rts

; @routine 0CAB:3A0 SlideAngleUp
; @run 70:B3A0
; WHAT IT DOES: Adds $00C0 to the 16-bit angle word (OBJ_WORK_F high, OBJ_WORK_G low); once the high byte reaches $10 the object's OBJ_SUBSTATE goes up one.
; HOW: A 16-bit add and a compare.
; WAITS: no.
; Entry: X object. Exit: the compare or the INC.
SlideAngleUp:
    lda a:OBJ_WORK_G,x
    clc
    adc #$C0
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    adc #$00
    sta a:OBJ_WORK_F,x
    cmp #$10
    bcc SlideAngleUp_at0
    inc a:OBJ_SUBSTATE,x
SlideAngleUp_at0:
    rts
; @routine 0CAB:3E5 StepPhaseThenPause
; @run 70:B3E5
; WHAT IT DOES: Runs the code OBJ_SUBSTATE names (JumpByPhaseTable); afterwards a pause counter OBJ_WORK_J counts down, and at zero it is set from a table at $B406 picked at random, the state is 4
; and OBJ_SUBSTATE is 0.
; HOW: JumpByPhaseTable, NextRandom, a table read (bank 70).
; WAITS: no.
; Entry: X object. Exit: the count; at zero, the OBJ_SUBSTATE clear.
StepPhaseThenPause:
    jsr JumpByPhaseTable
    lda a:OBJ_WORK_J,x
    beq StepPhaseThenPause_at0
    dec a:OBJ_WORK_J,x
    rts
StepPhaseThenPause_at0:
    jsr VecNextRandom
    and #$03
    tay
    lda a:$B406,y
    sta a:OBJ_WORK_J,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 0CAB:40A JumpByPhaseTable
; @run 70:B40A
; WHAT IT DOES: Goes to the code a table at $B41C names for OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B41C,y
    sta z:zTemp00
    lda a:$B41D,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAB:422 AimAtPlayerThenArm
; @run 70:B422
; WHAT IT DOES: Takes OBJ_WORK_E as the angle to the player (a random one on three counts out of four, by OBJ_WORK_I and 3), sets the angle word from a table at $B44F by
; OBJ_WORK_I and 3, and moves OBJ_WORK_I and OBJ_SUBSTATE on.
; HOW: AngleToPlayer, NextRandom, table reads (bank 70).
; WAITS: no.
; Entry: X object. Exit: N and Z of the new OBJ_SUBSTATE; A the table's low byte, Y the table index, C 0.
AimAtPlayerThenArm:
    jsr VecAngleToPlayer
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_I,x
    and #$03
    beq AimAtPlayerThenArm_at0
    jsr VecNextRandom
    sta a:OBJ_WORK_E,x
AimAtPlayerThenArm_at0:
    lda a:OBJ_WORK_I,x
    and #$03
    asl a
    tay
    lda a:$B450,y
    sta a:OBJ_WORK_F,x
    lda a:$B44F,y
    sta a:OBJ_WORK_G,x
    inc a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0CAB:457 MoveAlongAndSlow
; @run 70:B457
; WHAT IT DOES: Moves the object along its angle OBJ_WORK_E by the 16-bit length OBJ_WORK_F:OBJ_WORK_G. Outside the screen box (InsideScreenBox) it clears OBJ_WORK_I
; and OBJ_SUBSTATE and stops. Inside, it takes a byte of a table at $B4A0 (picked by OBJ_WORK_I and 7) off the length, and when the length runs out
; it sets the timer from a table at $B4A8 picked at random and goes on to the next OBJ_SUBSTATE.
; HOW: SetVelocityFromAngle, ApplyVelocity, InsideScreenBox, a 16-bit subtract, NextRandom, table reads (bank 70).
; Untraced: the outside arm (B46F).
; WAITS: no.
; Entry: X object. Exit: the last count, subtract or load.
MoveAlongAndSlow:
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    jsr InsideScreenBox
    bcc MoveAlongAndSlow_at0
    stz a:OBJ_WORK_I,x
    stz a:OBJ_SUBSTATE,x
    rts
MoveAlongAndSlow_at0:
    lda a:OBJ_WORK_I,x
    and #$07
    tay
    lda a:OBJ_WORK_G,x
    sec
    sbc a:$B4A0,y
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    sbc #$00
    sta a:OBJ_WORK_F,x
    bcs MoveAlongAndSlow_at1
    jsr VecNextRandom
    and #$03
    tay
    lda a:$B4A8,y
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
MoveAlongAndSlow_at1:
    rts

; @routine 0CAB:4AC InsideScreenBox
; @run 70:B4AC
; WHAT IT DOES: Says whether the object is inside the box X $20 to $DF, Y $20 to $BF on the first screen: C clear when inside, set when not.
; HOW: The position's high bytes ORed, then four compares.
; Untraced: the outside answer (B4CC).
; Curated cases: tools/rondo/curated/inside_screen_box.txt (150 inside cases; the draft reaches the untraced outside answer on the seeded ones).
; WAITS: no.
; Entry: X object. Exit: C clear when inside; A and the flags as the last compare or load left them.
InsideScreenBox:
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne InsideScreenBox_at0
    lda a:OBJ_X_LO,x
    cmp #$20
    bcc InsideScreenBox_at0
    cmp #$E0
    bcs InsideScreenBox_at0
    lda a:OBJ_Y_LO,x
    cmp #$20
    bcc InsideScreenBox_at0
    cmp #$C0
    bcs InsideScreenBox_at0
    clc
    rts
InsideScreenBox_at0:
    sec
    rts

; @routine 0CAB:4CE CountThenStop
; @run 70:B4CE
; WHAT IT DOES: Counts the object's timer down; at zero it clears OBJ_SUBSTATE.
; HOW: DEC and a store.
; WAITS: no.
; Entry: X object. Exit: N and Z from the count.
CountThenStop:
    dec a:OBJ_WORK_D,x
    bne CountThenStop_at0
    stz a:OBJ_SUBSTATE,x
CountThenStop_at0:
    rts

; @routine 0CAB:509 PickNextMoveBySide
; @run 70:B509
; WHAT IT DOES: Faces the player (bit 0 of SideOfPlayer), picks OBJ_SUBSTATE from a table at $B52C by the object's hit points halved (the high nibble) and a counter OBJ_WORK_K (the low
; nibble), and counts the counter up.
; HOW: SideOfPlayer, table read (bank 70).
; WAITS: no.
; Entry: X object. Exit: N and Z of the new counter.
PickNextMoveBySide:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_HEALTH,x
    lsr a
    and #$F0
    sta z:zTemp00
    lda a:OBJ_WORK_K,x
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$B52C,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_K,x
    rts

; @routine 0CAB:55C StepThenSpawnShot
; @run 70:B55C
; WHAT IT DOES: Steps the animation list at $B582; when it ends, starts a kind $28 object at its place in slot $12 with a timer of 1 and its facing, and moves OBJ_SUBSTATE on.
; HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_SUBSTATE.
StepThenSpawnShot:
    lda #$82
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenSpawnShot_at0
    ldy #$12
    jsr VecClearObjectInY
    lda #$28
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    inc a:OBJ_SUBSTATE,x
StepThenSpawnShot_at0:
    rts

; @routine 0CAB:591 StartShotsUnlessThere
; @run 70:B591
; WHAT IT DOES: Unless slot $12's kind is $28, starts the three shots (StartThreeShots) and plays sound $2E; then goes on as ResetToStateThree.
; HOW: A compare, StartThreeShots, SoundCommand, the next routine's bytes.
; WAITS: no.
; Entry: X object. Exit: as ResetToStateThree, or the compare's flags when the shots are already there.
StartShotsUnlessThere:
    lda a:$2762
    cmp #$28
    beq ResetToStateThree_at0
    jsr StartThreeShots
    lda #$2E
    jsr VecSoundCommand

; @routine 0CAB:5A0 ResetToStateThree
; @run 70:B5A0
; WHAT IT DOES: Sets the object's frame to 1, its state to 3 and clears OBJ_SUBSTATE.
; HOW: Three stores.
; WAITS: no.
; Entry: X object. Exit: A 3, N and Z from it.
ResetToStateThree:
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ResetToStateThree_at0:
    rts

; @routine 0CAB:5AE StartThreeShots
; @run 70:B5AE
; WHAT IT DOES: Starts three kind $26 objects at this object's place in slots $13 to $15, each with this object's facing and its number 0 to 2 as its timer.
; HOW: A loop of ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A the last number, Y $16, N, Z and C from the compare of Y with $16.
StartThreeShots:
    stz z:zTemp00
    ldy #$13
StartThreeShots_at0:
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda z:zTemp00
    sta a:OBJ_WORK_D,y
    inc z:zTemp00
    iny
    cpy #$16
    bcc StartThreeShots_at0
    rts
; @routine 0CAB:5CD StepThenWaitLonger
; @run 70:B5CD
; WHAT IT DOES: Steps the animation list at $B5E6; when it ends the object's frame is 0, its timer $30 and OBJ_SUBSTATE goes on.
; HOW: StepObjectFrames and stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_SUBSTATE.
StepThenWaitLonger:
    lda #$E6
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenWaitLonger_at0
    stz a:OBJ_FRAME,x
    lda #$30
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
StepThenWaitLonger_at0:
    rts

; @routine 0CAB:5F1 FlickerAndStartPieces
; @run 70:B5F1
; WHAT IT DOES: Counts the object's timer down; while it runs it starts up to two pieces (StartTwoPieces) and, when the timer is $18, moves the object to a random
; place (X and Y low bytes from a table at $B623, high bytes 0); at zero OBJ_SUBSTATE moves on.
; HOW: DEC, StartTwoPieces, NextRandom twice, table reads (bank 70).
; WAITS: no.
; Entry: X object. Exit: the last compare, load or INC.
FlickerAndStartPieces:
    dec a:OBJ_WORK_D,x
    beq FlickerAndStartPieces_at0
    jsr StartTwoPieces
    lda a:OBJ_WORK_D,x
    cmp #$18
    bne ThrowUpward
    stz a:OBJ_X_HI,x
    jsr VecNextRandom
    and #$07
    tay
    lda a:$B623,y
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    jsr VecNextRandom
    and #$07
    tay
    lda a:$B623,y
    sta a:OBJ_Y_LO,x
    rts
FlickerAndStartPieces_at0:
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0CAB:62B StartTwoPieces
; @run 70:B62B
; WHAT IT DOES: Starts up to two kind $29 objects at this object's place in free slots from $20 (each only if a slot is free).
; HOW: FindFreeSlotFrom20 and StartObjectAtPlace, twice.
; WAITS: no.
; Entry: X object. Exit: as the last search or start.
StartTwoPieces:
    jsr VecFindFreeSlotFrom20
    bcc StartTwoPieces_at0
    lda #$29
    jsr VecStartObjectAtPlace
    jsr VecFindFreeSlotFrom20
    bcc StartTwoPieces_at0
    lda #$29
    jsr VecStartObjectAtPlace
StartTwoPieces_at0:
    rts

; @routine 0CAB:640 StepThenRest
; @run 70:B640
; WHAT IT DOES: Steps the animation list at $B65B; when it ends the object shows frame 1, enters state 3 and clears OBJ_SUBSTATE.
; HOW: StepObjectFrames and three stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: A 3.
StepThenRest:
    lda #$5B
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenRest_at0
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StepThenRest_at0:
    rts

; @routine 0CAB:666 StepThenThrowShot
; @run 70:B666
; WHAT IT DOES: Steps the animation list at $B582; when it ends, starts a kind $28 object at its place in slot $12 with its facing and moves OBJ_SUBSTATE on.
; HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_SUBSTATE.
StepThenThrowShot:
    lda #$82
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenThrowShot_at0
    ldy #$12
    jsr VecClearObjectInY
    lda #$28
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    inc a:OBJ_SUBSTATE,x
StepThenThrowShot_at0:
    rts

; @routine 0CAB:687 SinkWhenPartnerGone
; @run 70:B687
; WHAT IT DOES: When slot $12's kind is not $28 the object sinks (SinkToLimit), its timer becomes $30 and OBJ_SUBSTATE moves on.
; HOW: A compare, SinkToLimit, two stores.
; WAITS: no.
; Entry: X object. Exit: the compare's flags, or N and Z of the new OBJ_SUBSTATE.
SinkWhenPartnerGone:
    lda a:$2762
    cmp #$28
    beq SinkWhenPartnerGone_at0
    jsr SinkToLimit
    lda #$30
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
SinkWhenPartnerGone_at0:
    rts

; @routine 0CAB:69A SinkToLimit
; @run 70:B69A
; WHAT IT DOES: Takes 3 off the object's Y low byte while it is $30 or more.
; HOW: A compare and a subtract.
; WAITS: no.
; Entry: X object. Exit: under $30: the compare's flags; else the subtract's.
SinkToLimit:
    lda a:OBJ_Y_LO,x
    cmp #$30
    bcc SinkToLimit_at0
    lda a:OBJ_Y_LO,x
    sec
    sbc #$03
    sta a:OBJ_Y_LO,x
SinkToLimit_at0:
    rts

; @routine 0CAB:6AB SinkAndShootThenRestart
; @run 70:B6AB
; WHAT IT DOES: ShootEveryFourth and SinkToLimit; the timer counts down and at zero the object shows frame 1, enters state 3 and clears OBJ_SUBSTATE.
; HOW: The two calls, a DEC and three stores.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: see the wrapper.
SinkAndShootThenRestart:
    jsr ShootEveryFourth
    jsr SinkToLimit
    dec a:OBJ_WORK_D,x
    bne SinkAndShootThenRestart_at0
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SinkAndShootThenRestart_at0:
    rts

; @routine 0CAB:6C4 ShootEveryFourth
; @run 70:B6C4
; WHAT IT DOES: Every fourth count of OBJ_WORK_H, while Y is under $A0, starts a kind $27 object at its place in the first free slot from $20 (sound $26 first), with the
; object's facing and its OBJ_WORK_L count (low three bits) as its next kind; the count goes up.
; HOW: FindFreeSlotFrom20, SoundCommand, StartObjectAtPlace.
; Untraced: two bytes.
; WAITS: no.
; Entry: X object, Y pushed by the sound driver. Exit: see the wrapper.
ShootEveryFourth:
    lda a:OBJ_WORK_H,x
    and #$03
    bne ShootEveryFourth_at0
    lda a:OBJ_Y_LO,x
    cmp #$A0
    bcs ShootEveryFourth_at0
    jsr VecFindFreeSlotFrom20
    bcc ShootEveryFourth_at0
    lda #$26
    jsr VecSoundCommand
    lda #$27
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda a:OBJ_WORK_L,x
    and #$07
    sta a:OBJ_START_ARG,y
    inc a:OBJ_WORK_L,x
ShootEveryFourth_at0:
    rts

; @routine 0CAB:6F3 StepThenThrowSlowShot
; @run 70:B6F3
; WHAT IT DOES: As StepThenThrowShot, but the shot's timer is 2.
; HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry and exit: as StepThenThrowShot.
StepThenThrowSlowShot:
    lda #$82
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenThrowSlowShot_at0
    ldy #$12
    jsr VecClearObjectInY
    lda #$28
    jsr VecStartObjectAtPlace
    lda #$02
    sta a:OBJ_WORK_D,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    inc a:OBJ_SUBSTATE,x
StepThenThrowSlowShot_at0:
    rts

; @routine 0CAB:719 SetLongWait
; @run 70:B719
; WHAT IT DOES: Sets the object's timer to $40 and moves OBJ_SUBSTATE on.
; HOW: A store and an INC.
; WAITS: no.
; Entry: X object. Exit: N and Z from the new OBJ_SUBSTATE, A $40.
SetLongWait:
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0CAB:722 SinkThenRestart
; @run 70:B722
; WHAT IT DOES: Drops a shot when a slot is free (DropShotWhenSlotFree), counts its timer down and at zero shows frame 1, enters state 3 and clears OBJ_SUBSTATE.
; HOW: DropShotWhenSlotFree, DEC, three stores.
; WAITS: no.
; Entry: X object. Exit: the count's N and Z, or A 3 at zero.
SinkThenRestart:
    jsr DropShotWhenSlotFree
    dec a:OBJ_WORK_D,x
    bne SinkThenRestart_at0
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SinkThenRestart_at0:
    rts

; @routine 0CAB:738 DropShotWhenSlotFree
; @run 70:B738
; WHAT IT DOES: Every 16th count of OBJ_WORK_H, searches slots $30 to $35 for a free one and starts a kind $2A object there at its place.
; HOW: A search by kind, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: the last compare or load.
DropShotWhenSlotFree:
    lda a:OBJ_WORK_H,x
    and #$0F
    bne DropShotWhenSlotFree_at0
    ldy #$30
DropShotWhenSlotFree_at2:
    lda a:OBJ_KIND,y
    beq DropShotWhenSlotFree_at1
    iny
    cpy #$36
    bcc DropShotWhenSlotFree_at2
    rts
DropShotWhenSlotFree_at1:
    jsr VecClearObjectInY
    lda #$2A
    jsr VecStartObjectAtPlace
DropShotWhenSlotFree_at0:
    rts

; @routine 0CAB:7A9 StepThenRestB
; @run 70:B7A9
; WHAT IT DOES: As StepThenRest with the list at $B7C4.
; HOW: As StepThenRest.
; WAITS: no.
; Entry and exit: as StepThenRest.
StepThenRestB:
    lda #$C4
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenRestB_at0
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StepThenRestB_at0:
    rts
; @routine 0CAB:7CD LaunchThenFall
; @run 70:B7CD
; WHAT IT DOES: State 0: plays sound $C0, quarters OBJ_WORK_J, takes the angle to the player plus $80 as the object's angle with the length $0080 and goes on to the next
; OBJ_SUBSTATE. Later: when it has been hit five times (OBJ_HIT_TAKEN) it sets OBJ_2EE0 to 3, its state to 4 and OBJ_SUBSTATE to 9; else it moves and steps the animation list at $B829, and when the
; list ends it sets its frame to 1, its state to 3 and OBJ_SUBSTATE to 0.
; HOW: SoundCommand, LSR (twice), AngleToPlayer, SetVelocityFromAngle, ApplyVelocity, StepObjectFrames.
; Untraced: the hit arm (B7FB to B80A).
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see abi_sound_end.
LaunchThenFall:
    lda a:OBJ_SUBSTATE,x
    bne LaunchThenFall_at0
    lda #$C0
    jsr VecSoundCommand
    lsr a:OBJ_WORK_J,x
    lsr a:OBJ_WORK_J,x
    jsr VecAngleToPlayer
    clc
    adc #$80
    pha
    lda #$80
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    inc a:OBJ_SUBSTATE,x
    rts
LaunchThenFall_at0:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc LaunchThenFall_at1
    lda #$03
    sta a:OBJ_WORK_A,x
    lda #$04
    sta a:OBJ_STATE,x
    lda #$09
    sta a:OBJ_SUBSTATE,x
    rts
LaunchThenFall_at1:
    jsr VecApplyVelocity
    lda #$29
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LaunchThenFall_at2
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
LaunchThenFall_at2:
    rts

; @routine 0CAC:282 FollowAndFacePlayer
; @run 70:BA82
; WHAT IT DOES: In state 0 loads the object's record; later it sets its frame to 0 and, when a table at $BAAF by slot $10's frame says so, copies slot $10's flash flag, takes
; its place and shows the frame $29 plus the angle to the player in thirty-seconds.
; HOW: LoadObjectRecord (a tail jump), CopyObjectPlaceBack, AngleToPlayer, table read (bank 70).
; WAITS: no.
; Entry: X object. Exit: state 0: as LoadObjectRecord; else the last load or add.
FollowAndFacePlayer:
    lda a:OBJ_STATE,x
    bne FollowAndFacePlayer_at0
    jmp VecLoadObjectRecord
FollowAndFacePlayer_at0:
    stz a:OBJ_FRAME,x
    ldy a:$2A78
    lda a:$BAAF,y
    beq FollowAndFacePlayer_at1
    lda a:$2B28
    sta a:OBJ_SPRITE_ATTR,x
    ldy #$10
    jsr VecCopyObjectPlaceBack
    jsr VecAngleToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc #$29
    sta a:OBJ_FRAME,x
FollowAndFacePlayer_at1:
    rts

; @routine 0CAC:2EF StartSoundedPiece
; @run 70:BAEF
; @stop BB10
; WHAT IT DOES: In state 0 loads the object's record, plays the sound a table at $BB12 names for its timer-index, takes an X speed from a table at $BB10 by its facing, moves
; along X once and faces right; in any other state it goes on as StepOrClearByKind (the code after the table).
; HOW: LoadObjectRecord, SoundCommand, ApplyVelocityX.
; Untraced: the tables at $BB10 are data the run only read.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: see the wrapper.
StartSoundedPiece:
    lda a:OBJ_STATE,x
    bne StepOrClearByKind
    jsr VecLoadObjectRecord
    ldy a:OBJ_WORK_D,x
    lda a:$BB12,y
    jsr VecSoundCommand
    ldy a:OBJ_FACING_LEFT,x
    lda a:$BB10,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    stz a:OBJ_FACING_LEFT,x
    rts

; @routine 0CAC:315 StepOrClearByKind
; @run 70:BB15
; WHAT IT DOES: Steps the animation list that a table at $BB2D names for the object's timer-index (OBJ_WORK_D doubled); when it ends the object is cleared.
; HOW: A table read (bank 70), StepObjectFrames, a tail jump into ClearObject. (The part of the routine at $BAEF that comes after its table.)
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the step.
StepOrClearByKind:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:$BB2D,y
    sta z:zTemp10
    lda a:$BB2E,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepOrClearByKind_at0
    jmp VecClearObject
StepOrClearByKind_at0:
    rts

; @routine 0CAC:34E StartSideDrifter
; @run 70:BB4E
; @stop BB89
; WHAT IT DOES: In state 0 loads the object's record, takes an X speed from a table at $BB89 by its facing, moves along X once, takes a four-byte speed row from the table at $BB8B by its
; timer-index and reverses its X speed when it faces left; in any other state it goes on as StepListAndMove.
; HOW: LoadObjectRecord, ApplyVelocityX, NegateVelocityX, table reads (bank 70).
; Untraced: the tables at $BB89 and $BB8B are data the run only read.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: state 0: the last load or negate; else as StepListAndMove.
StartSideDrifter:
    lda a:OBJ_STATE,x
    bne $BB97
    jsr VecLoadObjectRecord
    ldy a:OBJ_FACING_LEFT,x
    lda a:$BB89,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    tay
    lda a:$BB8C,y
    sta a:OBJ_VX_HI,x
    lda a:$BB8B,y
    sta a:OBJ_VX_LO,x
    lda a:$BB8E,y
    sta a:OBJ_VY_HI,x
    lda a:$BB8D,y
    sta a:OBJ_VY_LO,x
    lda a:OBJ_FACING_LEFT,x
    beq StartSideDrifter_at0
    jsr VecNegateVelocityX
StartSideDrifter_at0:
    rts

; @routine 0CAC:399 StepListAndMove
; @run 70:BB99
; WHAT IT DOES: Steps the animation list that a table at $BBAC names for the object's timer-index (doubled) and moves the object (the part of the routine at $BB4E after its table).
; HOW: A table read (bank 70), StepObjectFrames, a tail jump into ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity, Y from the step.
StepListAndMove:
    bit z:zTemp0A,x
    tay
    lda a:$BBAC,y
    sta z:zTemp10
    lda a:$BBAD,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jmp VecApplyVelocity

; @routine 0CAC:3C1 StartScatteringPiece
; @run 70:BBC1
; WHAT IT DOES: In any state but 0 it goes on as StepMoveOrClear. In state 0 it loads its record, takes a random X and Y speed from a table at $BC1C (sixteen bytes: eight
; words), moves once, then takes a second random X speed from $BC2C and Y speed from $BC3C.
; HOW: LoadObjectRecord, NextRandom four times, ApplyVelocity, table reads (bank 70).
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: the last random's pick and table read.
StartScatteringPiece:
    lda a:OBJ_STATE,x
    beq StartScatteringPiece_at0
    jmp StepMoveOrClear
StartScatteringPiece_at0:
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    and #$07
    asl a
    tay
    lda a:AnimationLists+2,y
    sta a:OBJ_VX_HI,x
    lda a:AnimationLists+1,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$07
    asl a
    tay
    lda a:AnimationLists+2,y
    sta a:OBJ_VY_HI,x
    lda a:AnimationLists+1,y
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$07
    asl a
    tay
    lda a:$BC2D,y
    sta a:OBJ_VX_HI,x
    lda a:$BC2C,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$07
    asl a
    tay
    lda a:$BC3D,y
    sta a:OBJ_VY_HI,x
    lda a:$BC3C,y
    sta a:OBJ_VY_LO,x
    rts

; @routine 0CAC:45C StepMoveOrClear
; @run 70:BC5C
; WHAT IT DOES: Steps the animation list at $BC6F; while it runs the object moves by its velocity, and when it ends the object is cleared.
; HOW: StepObjectFrames, then a tail jump into ApplyVelocity or ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: running: as ApplyVelocity; ended: as the step.
StepMoveOrClear:
    lda #$BC
    sta z:zTemp11
    lda #$6F
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne StepMoveOrClear_at0
    jmp VecClearObject
StepMoveOrClear_at0:
    jmp VecApplyVelocity

; @routine 0CAC:492 StartThrownPiece
; @run 70:BC92
; WHAT IT DOES: Loads the object's record, turns on bit 7 of its hitbox flag, gives it an X speed from a table at $BCD1 by its facing and a Y speed of $18, moves it, then picks
; a random length from a table at $BCD3 and sets its speed from the angle a table at $BCE3 names for its next kind (SetVelocityFromAngle); a left-facing object is reversed.
; HOW: LoadObjectRecord, ApplyVelocity, NextRandom, SetVelocityFromAngle.
; Untraced: the reversal (BCCD).
; Curated cases: tools/rondo/curated/start_thrown_piece.txt (right-facing objects, the traced arm).
; WAITS: no.
; Entry: X object. Exit: the last load; C, V and Y as the angle scale left them.
StartThrownPiece:
    jsr VecLoadObjectRecord
    lda a:OBJ_HITBOX_ON,x
    ora #$80
    sta a:OBJ_HITBOX_ON,x
    ldy a:OBJ_FACING_LEFT,x
    lda a:$BCD1,y
    sta a:OBJ_VX_HI,x
    lda #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$07
    asl a
    tay
    lda a:$BCD3,y
    sta z:zTemp00
    lda a:$BCD4,y
    sta z:zTemp01
    ldy a:OBJ_START_ARG,x
    lda a:$BCE3,y
    jsr VecSetVelocityFromAngle
    lda a:OBJ_FACING_LEFT,x
    beq StartThrownPiece_at0
    jsr VecNegateVelocityX
StartThrownPiece_at0:
    rts

; @routine 0CAC:4EB BounceSparkOffWalls
; @run 70:BCEB
; WHAT IT DOES: Steps the animation list at $BD51; an object not on the first Y page is cleared; its hitbox half height comes from a table at $BD78 by the frame less $17; it moves, $0008 is added to
; its Y speed (gravity), and it asks the terrain: a wall (request $22, height 4) halves its X speed and reverses it (the shift of the 16-bit speed through the carry, then NegateVelocityX); else a floor
; (request 9) stops its X speed, sets its timer to $40 and moves OBJ_SUBSTATE on.
; HOW: StepObjectFrames, ApplyVelocity, ProbeTerrain twice, table read (bank 70), a tail jump into NegateVelocityX or ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see abi_chain.
BounceSparkOffWalls:
    lda #$51
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_Y_HI,x
    beq BounceSparkOffWalls_at0
    jmp VecClearObject
BounceSparkOffWalls_at0:
    lda a:OBJ_FRAME,x
    sec
    sbc #$17
    tay
    lda a:$BD78,y
    sta a:OBJ_HITBOX_HALF_H,x
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    clc
    adc #$08
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    lda #$04
    sta z:SpriteFlipMask
    lda #$22
    jsr VecProbeTerrain
    and #$02
    beq BounceSparkOffWalls_at1
    lda a:OBJ_VX_HI,x
    asl a
    ror a:OBJ_VX_HI,x
    ror a:OBJ_VX_LO,x
    jmp VecNegateVelocityX
BounceSparkOffWalls_at1:
    lda #$09
    jsr VecProbeTerrain
    and #$01
    beq BounceSparkOffWalls_at2
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
BounceSparkOffWalls_at2:
    rts

; @routine 0CAC:57C StepCountClear
; @run 70:BD7C
; WHAT IT DOES: Steps the animation list at $BD51; counts the object's timer down and clears it at zero; until then its hitbox half height comes from a table at $BD78 by the
; frame less $17.
; HOW: StepObjectFrames, DEC, a table read (bank 70).
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: cleared: N and Z of the count; else A the table byte, Y the index, C and V from the subtract.
StepCountClear:
    lda #$51
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_D,x
    bne StepCountClear_at0
    jmp VecClearObject
StepCountClear_at0:
    lda a:OBJ_FRAME,x
    sec
    sbc #$17
    tay
    lda a:$BD78,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0CAC:5B5 StartFallingSpark
; @run 70:BDB5
; WHAT IT DOES: Loads the object's record, gives OBJ_WORK_E a random byte, its angle low byte one of four from a table at $BDE1, a random X low byte, X high 0, Y $0100 and a
; timer of $C0.
; HOW: LoadObjectRecord, NextRandom three times, a table read (bank 70).
; WAITS: no.
; Entry: X object. Exit: A $C0, N set, Z clear; Y the table index, C and V as the last random left them.
StartFallingSpark:
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    sta a:OBJ_WORK_E,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:$BDE1,y
    sta a:OBJ_WORK_G,x
    stz a:OBJ_X_HI,x
    jsr VecNextRandom
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_Y_HI,x
    stz a:OBJ_Y_LO,x
    lda #$C0
    sta a:OBJ_WORK_D,x
    rts

; @routine 0CAC:5E5 TrackThenAimAtPlayer
; @run 70:BDE5
; WHAT IT DOES: Runs TrackPointOrLand, adds the speed OBJ_WORK_G to the angle OBJ_WORK_E and counts the timer down; at zero it takes the angle to the player and
; the speed OBJ_WORK_G along it, and goes on to the next state.
; HOW: TrackPointOrLand, AngleToPlayer, SetVelocityFromAngle.
; WAITS: no.
; Entry: X object. Exit: the timer's count, or the state's count after the step.
TrackThenAimAtPlayer:
    jsr TrackPointOrLand
    lda a:OBJ_WORK_E,x
    clc
    adc a:OBJ_WORK_G,x
    sta a:OBJ_WORK_E,x
    dec a:OBJ_WORK_D,x
    bne FlashFallAnimateThenRemove
    jsr VecAngleToPlayer
    pha
    stz z:zTemp01
    lda a:OBJ_WORK_G,x
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    inc a:OBJ_STATE,x
    rts

; @routine 0CAC:60A TrackPointOrLand
; @run 70:BE0A
; WHAT IT DOES: Puts object $16 at the place of object $10, one length $30 along the object's angle OBJ_WORK_E, and moves it. If the object has landed
; (OBJ_WORK_H set), or is within 8 of object $16 on both axes, it lands: frame 2, OBJ_WORK_B $12, flags OBJ_2EE0 and OBJ_WORK_H set, and object $16
; is placed and moved again along the angle. Otherwise it takes the angle to object $16 and moves at the speed OBJ_WORK_G along it.
; HOW: CopyObjectPlaceBack, SetVelocityFromAngle, ApplyVelocity, AngleToPoint, 16-bit subtract and add.
; WAITS: no.
; Entry: X object. Exit: as ApplyVelocity.
TrackPointOrLand:
    lda a:OBJ_WORK_E,x
    sta z:zTemp02
    phx
    ldx #$16
    ldy #$10
    jsr VecCopyObjectPlaceBack
    stz z:zTemp00
    lda #$30
    sta z:zTemp01
    lda z:zTemp02
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    plx
    lda a:OBJ_WORK_H,x
    bne TrackPointOrLand_at0
    lda a:OBJ_X_LO,x
    sec
    sbc a:$2BDE
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    sbc a:$2B86
    sta z:zTemp01
    lda z:zTemp00
    clc
    adc #$08
    sta z:zTemp00
    lda z:zTemp01
    adc #$00
    bne TrackPointOrLand_at1
    lda z:zTemp00
    cmp #$10
    bcs TrackPointOrLand_at1
    lda a:OBJ_Y_LO,x
    sec
    sbc a:$2CE6
    sta z:zTemp00
    lda a:OBJ_Y_HI,x
    sbc a:$2C8E
    sta z:zTemp01
    lda z:zTemp00
    clc
    adc #$08
    sta z:zTemp00
    lda z:zTemp01
    adc #$00
    bne TrackPointOrLand_at1
    lda z:zTemp00
    cmp #$10
    bcs TrackPointOrLand_at1
TrackPointOrLand_at0:
    lda #$02
    sta a:OBJ_WORK_C,x
    lda #$12
    sta a:OBJ_WORK_B,x
    lda #$01
    sta a:OBJ_WORK_A,x
    lda #$01
    sta a:OBJ_WORK_H,x
    ldy #$10
    jsr VecCopyObjectPlaceBack
    stz z:zTemp00
    lda #$30
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts
TrackPointOrLand_at1:
    lda a:$2B86
    sta z:zTemp09
    lda a:$2BDE
    sta z:zTemp08
    lda a:$2C8E
    sta z:zTemp0B
    lda a:$2CE6
    sta z:zTemp0A
    jsr VecAngleToPoint
    pha
    stz z:zTemp00
    lda a:OBJ_WORK_G,x
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 0CAC:6DD StartFromTable
; @run 70:BEDD
; WHAT IT DOES: Loads the object's record, sets its angle low byte to $80 and its angle high byte from a table at $BEF2 by its next kind, less $A0.
; HOW: LoadObjectRecord and a subtract.
; WAITS: no.
; Entry: X object. Exit: A the angle high byte, Y the next kind, C and V from the subtract.
StartFromTable:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_WORK_G,x
    ldy a:OBJ_START_ARG,x
    lda a:$BEF2,y
    sec
    sbc #$A0
    sta a:OBJ_WORK_F,x
    rts

; @routine 0CAC:703 OrbitThenLeave
; @run 70:BF03
; WHAT IT DOES: Steps the animation list that a table at $BF66 names for the object's next kind, puts the object at X $0080, Y $0070 relative
; to its screen and moves it along its angle OBJ_WORK_F by the length held in OBJ_WORK_E and the timer; then adds $80 to that 16-bit length,
; turns the angle by one and, once the high byte of the length reaches $50, takes the speed from the turned-about angle with length $0400 and goes on to the next state.
; HOW: A table read (bank 70), StepObjectFrames, SetVelocityFromAngle, ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: the state's count when it goes on, else the compare.
OrbitThenLeave:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:$BF66,y
    sta z:zTemp10
    lda a:$BF67,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    lda #$70
    sta a:OBJ_Y_LO,x
    lda a:OBJ_WORK_E,x
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    lda a:OBJ_WORK_F,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_E,x
    clc
    adc #$80
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    adc #$00
    sta a:OBJ_WORK_D,x
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_D,x
    cmp #$50
    bcc OrbitThenLeave_at0
    stz z:zTemp00
    lda #$04
    sta z:zTemp01
    lda a:OBJ_WORK_F,x
    clc
    adc #$80
    jsr VecSetVelocityFromAngle
    inc a:OBJ_STATE,x
OrbitThenLeave_at0:
    rts

; @routine 0CAD:0BC ShatterOnHits
; @run 71:C0BC
; @stop C11F
; WHAT IT DOES: An object that breaks into a piece when hit often. State 0 loads its record and sets sprite set 6, frame 1 and X speed high 4. Once its hits taken reach 5 it
; records its event, takes a kind from the table at $C160 by the player's stock (at most 4), clears its state, speeds and parts, sets Y speed high $FB and its timer
; to $16 and starts a kind $9B object at its place from a free slot from $50. Before that it steps the part list (StepPartList) in state 1 and
; goes on at $C11F once the scroll X low is $80 or more; other states go on at $C135 (neither ever ran in a script).
; HOW: LoadObjectRecord, RecordObjectEvent, FindFreeSlotFrom50, StartObjectAtPlace, StepPartList.
; WAITS: no.
; Entry: X object. Exit: see the wrapper.
ShatterOnHits:
    lda a:OBJ_STATE,x
    bne ShatterOnHits_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$04
    sta a:OBJ_VX_HI,x
    rts
ShatterOnHits_at0:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc ShatterOnHits_at1
    jsr VecRecordObjectEvent
    ldy z:zStock
    cpy #$04
    bcc ShatterOnHits_at2
    ldy #$04
ShatterOnHits_at2:
    lda a:$C160,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$16
    sta a:OBJ_INVULN_TIMER,x
    jsr VecFindFreeSlotFrom50
    bcc DoNothing
    lda #$9B
    jmp VecStartObjectAtPlace
ShatterOnHits_at1:
    lda a:OBJ_STATE,x
    dec a
    bne $C135
    bsr StepPartList
    lda a:ScrollXLo
    bpl DoNothing

; @routine 0CAD:12A StepPartList
; @run 71:C12A
; WHAT IT DOES: Steps the animation list at $C196.
; HOW: The list's address goes to zTemp10-11; a tail jump into StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as StepObjectFrames.
StepPartList:
    lda #$96
    sta z:zTemp10
    lda #$C1
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0CAD:15F DoNothing
; @run 71:C15F
; WHAT IT DOES: Returns at once (a state handler that has nothing to do).
; HOW: RTS.
; WAITS: no.
; Entry: nothing. Exit: every register and flag kept.
DoNothing:
    rts

; @routine 0CAD:165 WatchVramForMark
; @run 71:C165
; WHAT IT DOES: In state 0 loads the object's record and sets its sprite set to 6 and its frame to 7. Each frame it reads the VDC's video RAM at $05FB and, unless the
; word is a $43 byte, records the object's event and clears the object.
; HOW: LoadObjectRecord, SelectVramReadAddress, two data writes, SelectVramData, two reads of the VDC's data port, RecordObjectEvent, ClearObject.
; WAITS: no.
; Entry: X object. Exit: see the wrapper.
WatchVramForMark:
    lda a:OBJ_STATE,x
    bne WatchVramForMark_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$07
    sta a:OBJ_FRAME,x
WatchVramForMark_at0:
    jsr VecSelectVramReadAddress
    st1 #$FB
    st2 #$05
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    ldy a:VDC_DATA_HI
    cmp #$43
    beq DoNothing
    jsr VecRecordObjectEvent
    jmp VecClearObject
