; @routine 025A:6BA LoadRecordOrCapPlayerX
; @run 70:AEBA
; WHAT IT DOES: In state 0 loads the object's record; later it keeps the player's OBJ_X_LO below $C0.
; HOW: A test of the state, a tail jump into LoadObjectRecord, or a compare and a store.
; WAITS: no.
; Entry: X object. Exit: as LoadObjectRecord in state 0; otherwise the compare, A $BF when it was capped.
LoadRecordOrCapPlayerX:
    lda a:OBJ_STATE,x
    bne LoadRecordOrCapPlayerX_at0
    jmp VecLoadObjectRecord
LoadRecordOrCapPlayerX_at0:
    lda a:OBJ_X_LO
    cmp #$C0
    bcc LoadRecordOrCapPlayerX_at1
    lda #$BF
    sta a:OBJ_X_LO
LoadRecordOrCapPlayerX_at1:
    rts

; @routine 025A:6CF StartAtX120OrApplyVelocity
; @run 70:AECF
; WHAT IT DOES: In state 0 loads the object's record and sets its frame $6F, X position $0120, Y position $0038 and X speed -$0020; later it moves by its speed.
; HOW: A test of the state, a tail jump into ApplyVelocity, or LoadObjectRecord and eight stores.
; WAITS: no.
; Entry: X object. Exit: as ApplyVelocity, or A $E0 (N set, Z clear) with C and Y from LoadObjectRecord.
StartAtX120OrApplyVelocity:
    lda a:OBJ_STATE,x
    beq StartAtX120OrApplyVelocity_at0
    jmp VecApplyVelocity
StartAtX120OrApplyVelocity_at0:
    jsr VecLoadObjectRecord
    lda #$6F
    sta a:OBJ_FRAME,x
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$20
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$38
    sta a:OBJ_Y_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    lda #$E0
    sta a:OBJ_VX_LO,x
    rts

; @routine 025A:722 SetBossFixedFields
; @run 70:AF22
; WHAT IT DOES: Sets the fixed fields of slot 0: OBJ_WORK_E $10, OBJ_FRAME 0 and OBJ_WORK_F $40.
; HOW: Three stores.
; WAITS: no.
; Entry: nothing. Exit: A $40, N and Z from it.
SetBossFixedFields:
    lda #$10
    sta a:OBJ_WORK_E
    stz a:OBJ_FRAME
    lda #$40
    sta a:OBJ_WORK_F
    rts

; @routine 025A:730 StartBossWhenStageC0OrLoad
; @run 70:AF30
; WHAT IT DOES: Sets the fixed fields (SetBossFixedFields). When the requested stage zStageRequested is $C0 it starts list $84 on channel 0 and script $8E in slot 5, clears the enemy slots, clears the frames of slots $56 and $57, starts five objects of type $2B from the slots 0 to 4 into free slots of $30 to $3F (OBJ_START_ARG the number), advances ScrollXHi+1 and plays the track again (CdPlayAgain). Otherwise it loads the object's record, sets OBJ_WORK_D $80 and advances ScrollXHi+1.
; HOW: SetBossFixedFields, StartListChannelKind2, StartScriptSlotKind1, ClearEnemySlots, a loop of FindFreeSlotFrom30 and StartObjectAtPlace, CdPlayAgain, LoadObjectRecord.
; WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
; Entry: X object, Y kept for the track call. Exit: as the last instruction that ran. The $C0 arm ran in no script; the draft stops there and the arm is written from the decoded bytes.
StartBossWhenStageC0OrLoad:
    jsr SetBossFixedFields
    lda z:zStageRequested
    cmp #$C0
    bne StartBossWhenStageC0OrLoad_at0
    phx
    clx
    lda #$84
    jsr VecStartListChannelKind2
    ldx #$05
    lda #$8E
    jsr VecStartScriptSlotKind1
    jsr VecClearEnemySlots
    stz a:OBJ_FRAME+86
    stz a:OBJ_FRAME+87
    clx
StartBossWhenStageC0OrLoad_at1:
    jsr VecFindFreeSlotFrom30
    lda #$2B
    jsr VecStartObjectAtPlace
    txa
    sta a:OBJ_START_ARG,y
    inx
    cpx #$05
    bne StartBossWhenStageC0OrLoad_at1
    plx
    inc a:ScrollXHi+1
    jsr VecCdPlayAgain
    rts
StartBossWhenStageC0OrLoad_at0:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_WORK_D,x
    inc a:ScrollXHi+1
    rts

; @routine 025A:776 CountdownThenNextState
; @run 70:AF76
; WHAT IT DOES: Sets the fixed fields; counts the object's OBJ_WORK_D down; at zero clears it and goes to the next state; before that it may spawn a piece.
; HOW: SetBossFixedFields, a memory decrement, then SetTimerThenNextState with A 0 or the fall into SpawnPiece21Mask1F.
; WAITS: no.
; Entry: X object. Exit: as the routine it ends in.
CountdownThenNextState:
    jsr SetBossFixedFields
    dec a:OBJ_WORK_D,x
    bne SpawnPiece21Mask1F
    cla

; @routine 025A:77F SetTimerThenNextState
; @run 70:AF7F
; WHAT IT DOES: Sets the object's OBJ_WORK_D to A and goes to the next state.
; HOW: A store and an INC.
; WAITS: no.
; Entry: A the timer, X object. Exit: the new state's N and Z.
SetTimerThenNextState:
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
    rts

; @routine 025A:786 SpawnPiece21Mask1F
; @run 70:AF86
; WHAT IT DOES: SpawnPiece21UnlessMasked with the mask $1F.
; HOW: A load, running into the next routine.
; WAITS: no.
; Entry: X object. Exit: as SpawnPiece21UnlessMasked.
SpawnPiece21Mask1F:
    lda #$1F

; @routine 025A:788 SpawnPiece21UnlessMasked
; @run 70:AF88
; WHAT IT DOES: When A and the scroll byte ScrollXLo+5 have no bit in common, starts an object of type $21 in a free slot of $30 to $3F at the object's place.
; HOW: An AND, FindFreeSlotFrom30 and a tail jump into StartObjectAtPlace (started in Y even when none was free).
; WAITS: no.
; Entry: A the mask, X object. Exit: the AND, or the start's A and N, Z with Y and C from the slot search.
SpawnPiece21UnlessMasked:
    and a:ScrollXLo+5
    bne $AFDD
    jsr VecFindFreeSlotFrom30
    lda #$21
    jmp VecStartObjectAtPlace

; @routine 025A:795 CountdownThenSpawnPieces
; @run 70:AF95
; WHAT IT DOES: Sets the fixed fields and counts OBJ_WORK_D down; at zero sets it to $40 and goes to the next state; before that it may spawn a piece and, once in a while, a second one.
; HOW: SetBossFixedFields, SetTimerThenNextState or SpawnPiece21UnlessMasked, a compare of the scroll byte, NextRandom, FindFreeSlotFrom30 and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: the last instruction that ran.
CountdownThenSpawnPieces:
    jsr SetBossFixedFields
    dec a:OBJ_WORK_D,x
    bne CountdownThenSpawnPieces_at0
    bra SetTimerThenNextState
CountdownThenSpawnPieces_at0:
    lda #$7F
    bsr SpawnPiece21UnlessMasked
    lda a:ScrollXLo+5
    and #$7F
    cmp #$40
    bne $AFDD
    jsr VecNextRandom
    and #$03
    bne $AFDD
    jsr VecFindFreeSlotFrom30
    lda #$21
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_START_ARG,y
    rts

; @routine 025A:7C1 HoldScrollStepThenNextState
; @run 70:AFC1
; WHAT IT DOES: Sets the fixed fields, steps the scroll table (StepVerticalScrollTable), clears the scroll step ScrollStepXHi+1, and when the scroll's high byte is $FF and its low byte $80 sets the timer to $80 and goes to the next state; otherwise lowers the step by 2.
; HOW: SetBossFixedFields, StepVerticalScrollTable, loads and compares of the scroll bytes, a tail jump into SetTimerThenNextState or two memory decrements.
; WAITS: no.
; Entry: X object. Exit: the second decrement's N and Z, or as SetTimerThenNextState.
HoldScrollStepThenNextState:
    jsr SetBossFixedFields
    jsr StepVerticalScrollTable
    stz a:ScrollStepXHi+1
    lda a:ScrollXHi+1
    inc a
    bne HoldScrollStepThenNextState_at0
    lda a:ScrollXLo+1
    cmp #$80
    beq SetTimerThenNextState
HoldScrollStepThenNextState_at0:
    dec a:ScrollStepXHi+1
    dec a:ScrollStepXHi+1
    rts

; @routine 025A:7DE StepScrollThenMoveOrFadeOut
; @run 70:AFDE
; WHAT IT DOES: Runs StepVerticalScrollTable. When slots 0's state and OBJ_SUBSTATE and Y speed high byte are all 0 and the byte at $3BC3 is not 0 it acts by OBJ_WORK_I: at 0 it speeds up the object named by OBJ_START_ARG by $0010, sets ScrollStepXHi+1 to the negative of its X speed high byte, sets slot 0's OBJ_WORK_E $10 and OBJ_WORK_F 0, hands slot 0's sprite set, frame and facing to the object named by OBJ_START_ARG and clears slot 0's frame; when ScrollXHi+1 is $FE it then sets the timer $40 and goes to the next state. At 1 it clears OBJ_WORK_I, clears the object named by OBJ_START_ARG, makes it kind $31, sets slot 0's OBJ_WORK_E $10 and OBJ_WORK_F 0 and fades the CD with type $0C.
; HOW: StepVerticalScrollTable, tests, 16-bit adds in memory, ClearObjectInY, cd_fade (the card's volume fade), a tail jump into SetTimerThenNextState.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran; with the fade, as cd_fade leaves them. The byte at $3BC3 and $3BF0 keep their address form until they have names. The OBJ_WORK_I not 0 arm ran in no script; the draft stops there.
StepScrollThenMoveOrFadeOut:
    jsr StepVerticalScrollTable
    lda a:OBJ_STATE
    ora a:OBJ_SUBSTATE
    bne $AFDD
    lda a:OBJ_VY_HI
    bne $AFDD
    lda a:RichterColumnHit
    beq $AFDD
    lda a:OBJ_WORK_I,x
    bne StepScrollThenMoveOrFadeOut_at0
    phx
    lda a:OBJ_START_ARG,x
    tax
    lda #$10
    clc
    adc a:OBJ_VX_LO,x
    sta a:OBJ_VX_LO,x
    cla
    adc a:OBJ_VX_HI,x
    sta a:OBJ_VX_HI,x
    cla
    sec
    sbc a:OBJ_VX_HI,x
    sta a:ScrollStepXHi+1
    plx
    lda #$10
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_SPRITE_SET
    sta a:OBJ_SPRITE_SET,y
    lda a:OBJ_FRAME
    sta a:OBJ_FRAME,y
    lda a:OBJ_FACING_LEFT
    sta a:OBJ_FACING_LEFT,y
    stz a:OBJ_FRAME
    ldy a:ScrollXHi+1
    lda #$40
    iny
    iny
    bne StepScrollThenMoveOrFadeOut_at1
    jmp SetTimerThenNextState
StepScrollThenMoveOrFadeOut_at1:
    rts
StepScrollThenMoveOrFadeOut_at0:
    stz a:OBJ_WORK_I,x
    ldy a:OBJ_START_ARG,x
    jsr VecClearObjectInY
    lda #$31
    sta a:OBJ_KIND,y
    lda #$10
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    lda #$0C
    jmp BiosCdFade
