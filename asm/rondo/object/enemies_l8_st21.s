; @routine 0F6B:07D LoadRecordSplitIntoFourAndClear
; @run 70:B07D
; WHAT IT DOES: Sets OBJ_SPRITE_FLIP of slot 0 to $C0, loads the record and records the event, starts four objects of type $A6 in the slots after this one at its place (the second and fourth with bit 7 of OBJ_X_LO set, the third and fourth one screen on in X) and clears itself.
; HOW: LoadObjectRecord, RecordObjectEvent, four StartObjectAtPlace calls, INCs and ORs with X and Y swapped, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordSplitIntoFourAndClear:
    lda #$C0
    sta a:OBJ_SPRITE_FLIP
    jsr VecLoadObjectRecord
    jsr VecRecordObjectEvent
    txa
    inc a
    tay
    lda #$A6
    jsr VecStartObjectAtPlace
    iny
    lda #$A6
    jsr VecStartObjectAtPlace
    lda a:OBJ_X_LO,y
    ora #$80
    sta a:OBJ_X_LO,y
    iny
    lda #$A6
    jsr VecStartObjectAtPlace
    sxy
    inc a:OBJ_X_HI,x
    sxy
    iny
    lda #$A6
    jsr VecStartObjectAtPlace
    sxy
    inc a:OBJ_X_HI,x
    sxy
    lda a:OBJ_X_LO,y
    ora #$80
    sta a:OBJ_X_LO,y
    jmp VecClearObject

; @routine 0F6B:0BF LoadRecordThenFollowScroll
; @run 70:B0BF
; WHAT IT DOES: In state 0 loads the record and keeps the object's X in OBJ_WORK_D and OBJ_WORK_E. Later it sets its screen X to that spot less the scroll plus a quarter of the scroll (the scroll's pair of bytes shifted right 3 times), so it moves slowly against the scroll.
; HOW: LoadObjectRecord, two shifts through carry, two 16-bit adds and a 16-bit subtract in memory.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordThenFollowScroll:
    lda a:OBJ_STATE,x
    bne LoadRecordThenFollowScroll_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_X_LO,x
    sta a:OBJ_WORK_D,x
    lda a:OBJ_X_HI,x
    sta a:OBJ_WORK_E,x
    rts
LoadRecordThenFollowScroll_at0:
    lda a:ScrollXHiScreen
    lsr a
    lda a:ScrollXLo
    ror a
    lsr a
    lsr a
    clc
    adc a:OBJ_WORK_D,x
    sta a:OBJ_X_LO,x
    cla
    adc a:OBJ_WORK_E,x
    sta a:OBJ_X_HI,x
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc a:ScrollXHiScreen
    sta a:OBJ_X_HI,x
    rts

; @routine 0F6B:100 BreakIntoPiecesAtFifthHit
; @run 70:B100
; WHAT IT DOES: In state 0 loads the record. Once the object has taken 5 hits it records its event, plays sound $12, takes the kind of OBJ_START_ARG, sets OBJ_INVULN_TIMER $10, goes to state 0, drops $28 in Y, clears its frame and starts an object of type $9B in a free slot of $50 to $53, then four pieces (StartPieceWithSpeedFromTable for the numbers 0 to 3).
; HOW: LoadObjectRecord, RecordObjectEvent, SoundCommand, FindFreeSlotFrom50, StartObjectAtPlace, four calls.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
BreakIntoPiecesAtFifthHit:
    lda a:OBJ_STATE,x
    bne StepFacingAndJump
    jmp VecLoadObjectRecord
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc BreakIntoPiecesAtFifthHit_at0
    jsr VecRecordObjectEvent
    lda #$12
    jsr VecSoundCommand
    lda a:OBJ_START_ARG,x
    sta a:OBJ_KIND,x
    lda #$10
    sta a:OBJ_INVULN_TIMER,x
    stz a:OBJ_STATE,x
    lda a:OBJ_Y_LO,x
    clc
    adc #$D8
    sta a:OBJ_Y_LO,x
    stz a:OBJ_FRAME,x
    jsr VecFindFreeSlotFrom50
    bcc BreakIntoPiecesAtFifthHit_at0
    lda #$9B
    jsr VecStartObjectAtPlace
    stz z:zTemp00
    jsr StartPieceWithSpeedFromTable
    inc z:zTemp00
    jsr StartPieceWithSpeedFromTable
    inc z:zTemp00
    jsr StartPieceWithSpeedFromTable
    inc z:zTemp00
    jsr StartPieceWithSpeedFromTable
BreakIntoPiecesAtFifthHit_at0:
    rts

; @routine 0F6B:150 StartPieceWithSpeedFromTable
; @run 70:B150
; WHAT IT DOES: Starts an object of type $A4 in a free slot of $20 to $3F at the object's place, $28 lower, with OBJ_START_ARG the number in zTemp00 plus 3 and the speed from the table at $B18A by twice that number.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace, table reads, stores between a PHX and a PLX.
; WAITS: no.
; Entry: X object, zTemp00 the number. Exit: as the last instruction that ran.
StartPieceWithSpeedFromTable:
    jsr VecFindFreeSlotFrom20
    bcc $B14F
    lda #$A4
    jsr VecStartObjectAtPlace
    phx
    lda a:OBJ_Y_LO,y
    clc
    adc #$28
    sta a:OBJ_Y_LO,y
    lda z:zTemp00
    clc
    adc #$03
    sta a:OBJ_START_ARG,y
    lda z:zTemp00
    asl a
    tax
    lda a:$B18A,x
    sta a:OBJ_VX_LO,y
    lda a:$B18B,x
    sta a:OBJ_VX_HI,y
    lda a:$B192,x
    sta a:OBJ_VY_LO,y
    lda a:$B193,x
    sta a:OBJ_VY_HI,y
    plx
    rts

; @routine 0F6B:19A LoadRecordFlipByPlayerThenFall
; @run 70:B19A
; WHAT IT DOES: In state 0 loads the record, takes OBJ_START_ARG as its frame and, when the player is on the left side, faces left and negates (bytes inverted) its speed. Later it falls (ApplyGravity) and moves.
; HOW: LoadObjectRecord, SideOfPlayer, EORs, ApplyGravity, a tail jump into ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordFlipByPlayerThenFall:
    lda a:OBJ_STATE,x
    bne LoadRecordFlipByPlayerThenFall_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
    jsr VecSideOfPlayer
    lsr a
    bcc LoadRecordFlipByPlayerThenFall_at1
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_VX_LO,x
    eor #$FF
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    eor #$FF
    sta a:OBJ_VX_HI,x
LoadRecordFlipByPlayerThenFall_at1:
    rts
LoadRecordFlipByPlayerThenFall_at0:
    jsr VecApplyGravity
    jmp VecApplyVelocity

; @routine 0F6B:1CA LoadRecordThenClearAtSubStep3
; @run 70:B1CA
; WHAT IT DOES: In state 0 loads the record. Later, when the sub-mode's step is 3, clears OBJ_SPRITE_FLIP of slot 0 and clears the object.
; HOW: LoadObjectRecord, a compare, a store, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordThenClearAtSubStep3:
    lda a:OBJ_STATE,x
    bne LoadRecordThenClearAtSubStep3_at0
    jmp VecLoadObjectRecord
LoadRecordThenClearAtSubStep3_at0:
    lda z:zSubStep
    cmp #$03
    bne LoadRecordThenClearAtSubStep3_at1
    stz a:OBJ_SPRITE_FLIP
    jmp VecClearObject
LoadRecordThenClearAtSubStep3_at1:
    rts

; @routine 0F6B:395 SoundWhenHurtThenFlag
; @run 70:B395
; WHAT IT DOES: When bit 7 of OBJ_HIT_STATUS is clear and the object has taken 5 hits plays sound $D2 and, when OBJ_HEALTH of slot $10 is below 8, sets it to 5, sets OBJ_WORK_H to 5 and sets bit 7 of OBJ_HIT_STATUS.
; HOW: Tests, SoundCommand, stores.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The stores past the last compare ran in no script; the draft stops there.
SoundWhenHurtThenFlag:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    bne SoundWhenHurtThenFlag_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc SoundWhenHurtThenFlag_at0
    lda #$D2
    jsr VecSoundCommand
    lda a:$32B8
    cmp #$08
    bcs SoundWhenHurtThenFlag_at0
    lda #$05
    sta a:$32B8
    sta a:OBJ_WORK_H,x
    lda #$80
    ora a:OBJ_HIT_STATUS,x
    sta a:OBJ_HIT_STATUS,x
SoundWhenHurtThenFlag_at0:
    rts

; @routine 0F6B:3C0 SoundAndState6WhenFlagged
; @run 70:B3C0
; WHAT IT DOES: Outside state 6: when bit 7 of OBJ_HIT_STATUS is set plays sound $D3, goes to state 6 and copies OBJ_WORK_H to OBJ_HEALTH; otherwise copies OBJ_HEALTH to OBJ_WORK_H.
; HOW: Compares, SoundCommand, copies.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The flagged arm ran in no script; the draft stops there.
SoundAndState6WhenFlagged:
    lda a:OBJ_STATE,x
    cmp #$06
    beq SoundAndState6WhenFlagged_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq StepPhaseThenPause
    lda #$D3
    jsr VecSoundCommand
    lda #$06
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_WORK_H,x
    sta a:OBJ_HEALTH,x
    rts
    lda a:OBJ_HEALTH,x
    sta a:OBJ_WORK_H,x
SoundAndState6WhenFlagged_at0:
    rts

; @routine 0F6B:40C LoadRecordFrame0D
; @run 70:B40C
; WHAT IT DOES: Loads the object's record and sets its frame $0D.
; HOW: LoadObjectRecord and a store.
; WAITS: no.
; Entry: X object. Exit: A $0D, Y 11, C from the record.
LoadRecordFrame0D:
    jsr VecLoadObjectRecord
    lda #$0D
    sta a:OBJ_FRAME,x
    rts

; @routine 0F6B:415 ClearFourAtScreen2
; @run 70:B415
; WHAT IT DOES: When the sub-mode is 1 and the scroll's screen byte is 2, sets the scroll's X limit low byte $80 and zRasterStage 3, clears slots $40 to $43, gives the object the Y speed -$0080 and goes to the next state.
; HOW: Two compares, ClearObjectInY four times, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
ClearFourAtScreen2:
    lda z:zGameSubMode
    cmp #$01
    bne ClearFourAtScreen2_at0
    lda a:ScrollXHiScreen
    cmp #$02
    bne ClearFourAtScreen2_at0
    lda #$80
    sta a:ScrollXMaxLo
    lda #$03
    sta z:zRasterStage
    ldy #$40
    jsr VecClearObjectInY
    iny
    jsr VecClearObjectInY
    iny
    jsr VecClearObjectInY
    iny
    jsr VecClearObjectInY
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    inc a:OBJ_STATE,x
ClearFourAtScreen2_at0:
    rts

; @routine 0F6B:44A MoveUpUntilY40
; @run 70:B44A
; WHAT IT DOES: Moves the object along Y; once its Y low byte is below $40 sets OBJ_WORK_D 2 and goes to the next state.
; HOW: ApplyVelocityY, a compare, a store, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
MoveUpUntilY40:
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp #$40
    bcs MoveUpUntilY40_at0
    lda #$02
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
MoveUpUntilY40_at0:
    rts

; @routine 0F6B:50B CountDownThenCopyPhase
; @run 70:B50B
; WHAT IT DOES: Counts OBJ_WORK_E down; when it is 0 (or reaches 0) sets OBJ_SUBSTATE from OBJ_WORK_P.
; HOW: A test, a memory decrement, a copy.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
CountDownThenCopyPhase:
    lda a:OBJ_WORK_E,x
    beq CountDownThenCopyPhase_at0
    dec a:OBJ_WORK_E,x
    bne CountDownThenCopyPhase_at1
CountDownThenCopyPhase_at0:
    lda a:OBJ_WORK_P,x
    sta a:OBJ_SUBSTATE,x
CountDownThenCopyPhase_at1:
    rts

; @routine 0F6B:51C ResetPieceAndAdvance
; @run 70:B51C
; WHAT IT DOES: Clears OBJ_WORK_L, sets OBJ_WORK_M $80 and OBJ_WORK_G $E0 and goes to the next OBJ_SUBSTATE.
; HOW: Stores and an INC.
; WAITS: no.
; Entry: X object. Exit: A $E0, N and Z from the new OBJ_SUBSTATE.
ResetPieceAndAdvance:
    lda #$00
    sta a:OBJ_WORK_L,x
    lda #$80
    sta a:OBJ_WORK_M,x
    lda #$E0
    sta a:OBJ_WORK_G,x
    inc a:OBJ_SUBSTATE,x
    rts
