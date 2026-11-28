; @routine 025B:068 CountdownThenStartScript80
; @run 70:B068
; WHAT IT DOES: Sets fixed fields (OBJ_WORK_E slot 0 $10, the scroll bytes ScrollXLo+1 0 and ScrollXHi+1 1, a byte at $3BF0 to 2); counts OBJ_WORK_D down; at $3F starts script $80 as kind 2 in slot 5, at 0 sets the sub-step 3 and zField8C $83.
; HOW: Stores, a memory decrement, a compare, StartScriptSlotKind2 between a PHX and a PLX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The byte at $3BF0 keeps its address form until it has a name.
CountdownThenStartScript80:
    lda #$10
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    stz a:OBJ_FRAME
    stz a:ScrollXLo+1
    lda #$01
    sta a:ScrollXHi+1
    dec a:OBJ_WORK_D,x
    beq CountdownThenStartScript80_at0
    lda a:OBJ_WORK_D,x
    cmp #$3F
    bne CountdownThenStartScript80_at1
    phx
    lda #$80
    ldx #$05
    jsr VecStartScriptSlotKind2
    plx
CountdownThenStartScript80_at1:
    rts
CountdownThenStartScript80_at0:
    lda #$03
    sta z:zSubStep
    lda #$83
    sta z:zField8C
    rts

; @routine 025B:0F0 StepVerticalScrollTable
; @run 70:B0F0
; WHAT IT DOES: While OBJ_WORK_F is not 0, steps through the table at $B129 and $B133: OBJ_WORK_E takes the entry and the scroll's Y (ScrollYLo+1, ScrollYHi+1) falls by it; OBJ_WORK_F counts up and wraps to 0 at 9. When it is 0 and the scroll byte ScrollXLo+5 is 0, a random number with its top bit set starts the table (OBJ_WORK_F 1).
; HOW: A table read by OBJ_WORK_F as the index, a 16-bit subtract, and NextRandom.
; WAITS: no.
; Entry: X object. Exit: the last instruction that ran.
StepVerticalScrollTableWorkEs = $B129 ; table StepVerticalScrollTable reads
StepVerticalScrollTableScrollYHiAdds = $B133 ; table StepVerticalScrollTable reads
StepVerticalScrollTable:
    lda a:OBJ_WORK_F,x
    beq StepVerticalScrollTable_at0
    tay
    lda a:StepVerticalScrollTableWorkEs,y
    sta a:OBJ_WORK_E,x
    lda a:ScrollYLo+1
    sec
    sbc a:StepVerticalScrollTableWorkEs,y
    sta a:ScrollYLo+1
    lda a:ScrollYHi+1
    sbc a:StepVerticalScrollTableScrollYHiAdds,y
    sta a:ScrollYHi+1
    inc a:OBJ_WORK_F,x
    cpy #$09
    bne StepVerticalScrollTable_at1
    stz a:OBJ_WORK_F,x
StepVerticalScrollTable_at1:
    rts
StepVerticalScrollTable_at0:
    lda a:ScrollXLo+5
    bne StepVerticalScrollTable_at2
    jsr VecNextRandom
    asl a
    bcc StepVerticalScrollTable_at2
    inc a:OBJ_WORK_F,x
StepVerticalScrollTable_at2:
    rts

; @routine 025B:148 LoadRecordPlaceByKind
; @run 70:B148
; WHAT IT DOES: In state 0 loads the record, sets sprite set 6 and the frame from a table by OBJ_START_ARG; when the object's flags are 0 it is placed at X $0120 with a Y from a second table, and its flags take the low nibble 6.
; HOW: LoadObjectRecord, two table reads indexed by OBJ_START_ARG, stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordPlaceByKindFrames = $B17E ; table LoadRecordPlaceByKind reads
LoadRecordPlaceByKindYLos = $B180 ; table LoadRecordPlaceByKind reads
LoadRecordPlaceByKind:
    lda a:OBJ_STATE,x
    bne LoadRecordPlaceByKind_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    ldy a:OBJ_START_ARG,x
    lda a:LoadRecordPlaceByKindFrames,y
    sta a:OBJ_FRAME,x
    lda a:OBJ_SCROLL_BAND,x
    bne LoadRecordPlaceByKind_at0
    lda #$20
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_X_HI,x
    lda a:LoadRecordPlaceByKindYLos,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_SCROLL_BAND,x
    and #$F0
    ora #$06
    sta a:OBJ_SCROLL_BAND,x
LoadRecordPlaceByKind_at0:
    rts

; @routine 025B:182 LoadRecordAtPlayerPlace
; @run 70:B182
; WHAT IT DOES: In state 0 loads the record and takes slot 0's OBJ_X_LO and OBJ_Y_LO; the flags take the low nibble 1.
; HOW: LoadObjectRecord, two copies, an AND and an ORA.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordAtPlayerPlace:
    lda a:OBJ_STATE,x
    bne LoadRecordAtPlayerPlace_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_X_LO
    sta a:OBJ_X_LO,x
    lda a:OBJ_Y_LO
    sta a:OBJ_Y_LO,x
    lda a:OBJ_SCROLL_BAND,x
    and #$F0
    ora #$01
    sta a:OBJ_SCROLL_BAND,x
LoadRecordAtPlayerPlace_at0:
    rts

; @routine 025B:1A1 LoadRecordThenSinkByF3520
; @run 70:B1A1
; WHAT IT DOES: In state 0 loads the record, sets the frame 6 and moves the object one left; then adds OBJ_WORK_E of slot $10 to its OBJ_Y_LO.
; HOW: LoadObjectRecord, a memory decrement, an add.
; WAITS: no.
; Entry: X object. Exit: the add's A, N, Z, C and V; Y 11 when the record was loaded.
LoadRecordThenSinkByF3520:
    lda a:OBJ_STATE,x
    bne JumpByStateTable072B1B1
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_FRAME,x
    dec a:OBJ_X_HI,x
    lda a:OBJ_FLING_WAIT+16
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    rts

; @routine 025B:1BC LoadRecordThenStepListByKind
; @run 70:B1BC
; WHAT IT DOES: In state 0 loads the record and moves the object one left; then steps an animation list picked by OBJ_START_ARG (two bytes an entry) and, from the table's second half when slot $10's state is below 4 and OBJ_WORK_D is 0.
; HOW: LoadObjectRecord, a decrement, table reads, a tail jump into StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step.
LoadRecordThenStepListByKindLists = $B1F5 ; a list's address by index
LoadRecordThenStepListByKind:
    lda a:OBJ_STATE,x
    bne SwingAndPick
    jsr VecLoadObjectRecord
    dec a:OBJ_X_HI,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:OBJ_WORK_D,x
    bne StartSplitterLow
    lda a:OBJ_STATE+16
    cmp #$04
    bcc PickPhaseAtRandomB
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_WORK_D,x
    bra StartSplitterLow
    tya
    clc
    adc #$04
    tay
    lda a:LoadRecordThenStepListByKindLists,y
    sta z:zTemp10
    lda a:LoadRecordThenStepListByKindLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 025B:229 LoadRecordSinkThenStepListByKind
; @run 70:B229
; WHAT IT DOES: In state 0 loads the record and moves the object one left; then adds OBJ_WORK_E of slot $10 to its OBJ_Y_LO and steps the animation list picked by OBJ_START_ARG from the table at $B250.
; HOW: LoadObjectRecord, a decrement, an add, table reads, a tail jump into StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step.
LoadRecordSinkThenStepListByKindLists = $B250 ; a list's address by index
LoadRecordSinkThenStepListByKind:
    lda a:OBJ_STATE,x
    bne LoadRecordSinkThenStepListByKind_at0
    jsr VecLoadObjectRecord
    dec a:OBJ_X_HI,x
LoadRecordSinkThenStepListByKind_at0:
    lda a:OBJ_FLING_WAIT+16
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:LoadRecordSinkThenStepListByKindLists,y
    sta z:zTemp10
    lda a:LoadRecordSinkThenStepListByKindLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 025B:266 StateMachineWithSoundAndList
; @run 70:B266
; WHAT IT DOES: An object of several phases. State 0 loads the record. State 1 waits on the state of slot $10 (3 or 6): it steps the animation list at $B2FA or $B2FF, plays sound $72 when the scroll byte ScrollXLo+1 is clear, and at state 6 goes on. State 2 counts OBJ_WORK_E down and goes on. Later states fall, and once off screen to the left start a list channel and clear the object.
; HOW: Compares of OBJ_STATE and slot $10's state, SoundCommand, StepObjectFrames, StartListChannelKind2 between a PHX and a PLX, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless a step sets it. Exit: as the last instruction that ran.
StateMachineWithSoundAndList:
    lda a:OBJ_STATE,x
    bne StateMachineWithSoundAndList_at0
    jsr VecLoadObjectRecord
    dec a:OBJ_X_HI,x
    rts
StateMachineWithSoundAndList_at0:
    dec a
    bne StateMachineWithSoundAndList_at1
    lda a:OBJ_STATE+16
    cmp #$03
    bne StateMachineWithSoundAndList_at2
    lda a:OBJ_WORK_D,x
    bne StateMachineWithSoundAndList_at3
    lda a:ScrollXLo+1
    and #$7F
    beq StateMachineWithSoundAndList_at4
    bra StateMachineWithSoundAndList_at5
StateMachineWithSoundAndList_at2:
    cmp #$06
    beq StateMachineWithSoundAndList_at6
    lda a:OBJ_WORK_D,x
    bne StateMachineWithSoundAndList_at3
StateMachineWithSoundAndList_at5:
    lda #$B2
    sta z:zTemp11
    lda #$FA
    sta z:zTemp10
    jmp VecStepObjectFrames
StateMachineWithSoundAndList_at4:
    lda #$72
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_WORK_D,x
StateMachineWithSoundAndList_at3:
    lda #$B2
    sta z:zTemp11
    lda #$FF
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne StateMachineWithSoundAndList_at7
    stz a:OBJ_WORK_D,x
StateMachineWithSoundAndList_at7:
    rts
StateMachineWithSoundAndList_at1:
    dec a
    bne StateMachineWithSoundAndList_at8
    dec a:OBJ_WORK_E,x
    bne StateMachineWithSoundAndList_at7
    lda a:OBJ_SCROLL_BAND,x
    and #$F0
    ora #$05
    sta a:OBJ_SCROLL_BAND,x
    inc a:OBJ_STATE,x
    rts
StateMachineWithSoundAndList_at6:
    lda #$1C
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    lda #$0A
    sta a:OBJ_WORK_E,x
    rts
StateMachineWithSoundAndList_at8:
    inc a:OBJ_Y_LO,x
    lda a:OBJ_X_LO,x
    bmi StateMachineWithSoundAndList_at7
    lda a:OBJ_X_HI,x
    bpl StateMachineWithSoundAndList_at7
    phx
    ldx #$05
    lda #$82
    jsr VecStartListChannelKind2
    plx
    jmp VecClearObject

; @routine 025B:308 DriftRecordThenMoveWithJitter
; @run 70:B308
; WHAT IT DOES: In state 0 loads the record and sets the speed X -$18, Y $0C. When slot $10's state is 4 or more and OBJ_START_ARG is 0 it waits one round, shows frame $1D, then moves by its speed, nudging X and Y back by random amounts of up to 15 when they are off the edge.
; HOW: LoadObjectRecord, ApplyVelocity, NextRandom, adds and a subtract with a zero page temporary.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
DriftRecordThenMoveWithJitter:
    lda a:OBJ_STATE,x
    bne StartOnFirstState
    jsr VecLoadObjectRecord
    lda #$E8
    sta a:OBJ_VX_HI,x
    lda #$0C
    sta a:OBJ_VY_HI,x
    rts
    lda a:OBJ_STATE+16
    cmp #$04
    bcc DriftRecordThenMoveWithJitter_at0
    lda a:OBJ_START_ARG,x
    bne DriftRecordThenMoveWithJitter_at0
    lda a:OBJ_WORK_D,x
    bne DriftRecordThenMoveWithJitter_at1
    inc a:OBJ_WORK_D,x
    lda #$1D
    sta a:OBJ_FRAME,x
    rts
DriftRecordThenMoveWithJitter_at1:
    jsr VecApplyVelocity
    lda a:OBJ_X_LO,x
    bpl DriftRecordThenMoveWithJitter_at2
    lda a:OBJ_X_HI,x
    bpl DriftRecordThenMoveWithJitter_at2
    inc a:OBJ_X_HI,x
    jsr VecNextRandom
    and #$0F
    clc
    adc a:OBJ_X_LO,x
    sta a:OBJ_X_LO,x
DriftRecordThenMoveWithJitter_at2:
    lda a:OBJ_Y_LO,x
    bmi DriftRecordThenMoveWithJitter_at0
    lda a:OBJ_Y_HI,x
    beq DriftRecordThenMoveWithJitter_at0
    dec a:OBJ_Y_HI,x
    jsr VecNextRandom
    and #$0F
    sta z:zTemp00
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp00
    sta a:OBJ_Y_LO,x
DriftRecordThenMoveWithJitter_at0:
    rts

; @routine 025B:487 SwingByCosineFromState0B
; @run 70:B487
; WHAT IT DOES: From state $0B on, sets the Y speed from the cosine of OBJ_WORK_H scaled by $0040, moves the object in Y and turns OBJ_WORK_H on by 2.
; HOW: ScaleByCosine, two copies of the product, ApplyVelocityY, an add.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
SwingByCosineFromState0B:
    lda a:OBJ_STATE,x
    cmp #$0B
    bcc SwingByCosineFromState0B_at0
    lda #$40
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_H,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_H,x
    clc
    adc #$02
    sta a:OBJ_WORK_H,x
SwingByCosineFromState0B_at0:
    rts

; @routine 025B:4B3 StartRandomScriptWhenFlagged
; @run 70:B4B3
; WHAT IT DOES: When OBJ_WORK_K is set and OBJ_WORK_F's low five bits are 0, with a chance of three in eight starts a script from the table at $B4E9 as kind 1 in slot 5.
; HOW: Two NextRandom calls and StartScriptSlotKind1 between a PHX and a PLX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
StartRandomScriptWhenFlaggedValues = $B4E9 ; table StartRandomScriptWhenFlagged reads
StartRandomScriptWhenFlagged:
    lda a:OBJ_WORK_K,x
    beq StartRandomScriptWhenFlagged_at0
    lda a:OBJ_WORK_F,x
    and #$1F
    bne StartRandomScriptWhenFlagged_at0
    jsr VecNextRandom
    and #$07
    cmp #$03
    bcs StartRandomScriptWhenFlagged_at0
    phx
    jsr VecNextRandom
    and #$03
    tay
    lda a:StartRandomScriptWhenFlaggedValues,y
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
StartRandomScriptWhenFlagged_at0:
    rts

; @routine 025B:4ED PushPlayerWhenFlagged
; @run 70:B4ED
; WHAT IT DOES: When OBJ_WORK_L is set, pushes the player: zPlayerPush $10 and zPushX $FF60.
; HOW: A load and three stores.
; WAITS: no.
; Entry: X object. Exit: A $60 when set; otherwise N and Z from OBJ_WORK_L.
PushPlayerWhenFlagged:
    lda a:OBJ_WORK_L,x
    beq PushPlayerWhenFlagged_at0
    lda #$10
    sta z:zPlayerPush
    lda #$FF
    sta z:zPushXHi
    lda #$60
    sta z:zPushXLo
PushPlayerWhenFlagged_at0:
    rts

; @routine 025B:4FF HitSoundThenSpawnPieces
; @run 70:B4FF
; WHAT IT DOES: When the object's hits taken reach 5, plays sound $75 and sets OBJ_WORK_N to $10. While OBJ_WORK_N runs it counts down and, whenever the wave counter's low three bits are 0, starts an object of type $2F in a free slot of $20 to $3F at the object's place with OBJ_WORK_D 1.
; HOW: SoundCommand, FindFreeSlotFrom20, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
HitSoundThenSpawnPieces:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bne HitSoundThenSpawnPieces_at0
    lda #$75
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_WORK_N,x
HitSoundThenSpawnPieces_at0:
    lda a:OBJ_WORK_N,x
    beq HitSoundThenSpawnPieces_at1
    dec a:OBJ_WORK_N,x
    lda z:zWaveCounter
    and #$07
    bne HitSoundThenSpawnPieces_at1
    jsr VecFindFreeSlotFrom20
    bcc HitSoundThenSpawnPieces_at1
    lda #$2F
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,y
HitSoundThenSpawnPieces_at1:
    rts

; @routine 025B:52E EnterState0BWhenFlagged
; @run 70:B52E
; WHAT IT DOES: When bit 7 of OBJ_HIT_STATUS is set and the state is below $0B, sets the state $0B and OBJ_SUBSTATE 0.
; HOW: Two tests, two stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The stores ran in no script; the draft stops there.
EnterState0BWhenFlagged:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq EnterState0BWhenFlagged_at0
    lda a:OBJ_STATE,x
    cmp #$0B
    bcs EnterState0BWhenFlagged_at0
    lda #$0B
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
EnterState0BWhenFlagged_at0:
    rts

; @routine 025B:56F StartPieceAtTopLeft
; @run 70:B56F
; WHAT IT DOES: Loads the object's record, clears slot $18, starts an object of type $28 there at the object's place, moves the new object's source to X $003C, Y $0030 with X speed $80 (OBJ_VX_LO), OBJ_WORK_D 4 and OBJ_WORK_K 1.
; HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace, stores.
; WAITS: no.
; Entry: X object. Exit: A 1, Y $18, C from the record.
StartPieceAtTopLeft:
    jsr VecLoadObjectRecord
    ldy #$18
    jsr VecClearObjectInY
    lda #$28
    jsr VecStartObjectAtPlace
    stz a:OBJ_X_HI,x
    lda #$3C
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$30
    sta a:OBJ_Y_LO,x
    lda #$80
    sta a:OBJ_VX_LO,x
    lda #$04
    sta a:OBJ_WORK_D,x
    lda #$01
    sta a:OBJ_WORK_K,x
    rts

; @routine 025B:59C CountDownThenStartScripts
; @run 70:B59C
; WHAT IT DOES: Counts the 16-bit timer OBJ_WORK_D:OBJ_WORK_E down; at zero starts scripts $84 (slot 5) and $86 (slot 4) as kind 1, clears OBJ_WORK_K, sets OBJ_WORK_E $60 and goes to the next state.
; HOW: A 16-bit subtract in memory, an ORA, StartScriptSlotKind1 twice between a PHX and a PLX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
CountDownThenStartScripts:
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    bne StartPieceAtNextSlot
    phx
    lda #$84
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$86
    ldx #$04
    jsr VecStartScriptSlotKind1
    plx
    stz a:OBJ_WORK_K,x
    lda #$60
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
    rts

; @routine 025B:5CE CountDownWithSound77
; @run 70:B5CE
; WHAT IT DOES: Counts OBJ_WORK_E down; at $35 plays sound $77; at zero sets OBJ_WORK_E $80 and goes to the next state.
; HOW: A memory decrement, a compare, SoundCommand.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
CountDownWithSound77:
    dec a:OBJ_WORK_E,x
    beq CountDownWithSound77_at0
    lda a:OBJ_WORK_E,x
    cmp #$35
    bne StepListB5FEToState4
    lda #$77
    jsr VecSoundCommand
    rts
CountDownWithSound77_at0:
    lda #$80
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
    rts

; @routine 025B:617 StartScriptAtTimerMarks
; @run 70:B617
; WHAT IT DOES: When OBJ_WORK_E is 0 sets OBJ_WORK_K. Otherwise counts it down, and when it equals one of the eight marks at $B642 starts the script of the same index in the table at $B64A as kind 1 in slot 5.
; HOW: A loop over the marks, StartScriptSlotKind1 between a PHX and a PLX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
StartScriptAtTimerMarksLimits = $B642 ; table StartScriptAtTimerMarks reads
StartScriptAtTimerMarksValues = $B64A ; table StartScriptAtTimerMarks reads
StartScriptAtTimerMarks:
    lda a:OBJ_WORK_E,x
    bne ThrowUpward
    lda #$01
    sta a:OBJ_WORK_K,x
    rts
    cly
StartScriptAtTimerMarks_at1:
    lda a:OBJ_WORK_E,x
    cmp a:StartScriptAtTimerMarksLimits,y
    beq StartScriptAtTimerMarks_at0
    iny
    cpy #$08
    bcc StartScriptAtTimerMarks_at1
    dec a:OBJ_WORK_E,x
    rts
StartScriptAtTimerMarks_at0:
    phx
    lda a:StartScriptAtTimerMarksValues,y
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    dec a:OBJ_WORK_E,x
    rts

; @routine 025B:652 SpinOutThenLeave
; @run 70:B652
; WHAT IT DOES: Moves the object on a circle of radius $0200 by OBJ_WORK_G (turning on 4 a frame) while it steps the animation list at $B6A4; when the list ends it starts the next phase: slot $10's state up, OBJ_WORK_L 1, OBJ_HITBOX_ON 2, slot 0's OBJ_WORK_E 2, the byte at $3BF0 2, OBJ_WORK_F of slot 0 0, OBJ_WORK_G $C0, OBJ_WORK_E $20, OBJ_WORK_I 1 and the next state.
; HOW: SetVelocityFromAngle, ApplyVelocity, an add, StepObjectFrames and stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. The byte at $3BF0 keeps its address form until it has a name.
SpinOutThenLeave:
    stz z:zTemp00
    lda #$02
    sta z:zTemp01
    lda a:OBJ_WORK_G,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_G,x
    clc
    adc #$04
    sta a:OBJ_WORK_G,x
    lda #$A4
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne SoundReadCommand
    inc a:OBJ_STATE+16
    lda #$01
    sta a:OBJ_WORK_L,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$02
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    lda #$C0
    sta a:OBJ_WORK_G,x
    lda #$20
    sta a:OBJ_WORK_E,x
    lda #$01
    sta a:OBJ_WORK_I,x
    inc a:OBJ_STATE,x
    rts

; @routine 025B:6E7 SpinOnCircle
; @run 70:B6E7
; WHAT IT DOES: Moves the object on a circle of radius $0180 by OBJ_WORK_G, turning on 2 a frame; when the angle wraps to 0 it jumps to X $00D5, Y $002F with the fractions $4C and $83.
; HOW: SetVelocityFromAngle, ApplyVelocity, an add, four stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
SpinOnCircle:
    lda #$80
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_G,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_G,x
    clc
    adc #$02
    sta a:OBJ_WORK_G,x
    bne SpinOnCircle_at0
    lda #$D5
    sta a:OBJ_X_LO,x
    lda #$4C
    sta a:OBJ_X_FRAC,x
    lda #$2F
    sta a:OBJ_Y_LO,x
    lda #$83
    sta a:OBJ_Y_FRAC,x
SpinOnCircle_at0:
    rts

; @routine 025B:761 StepListThenBurstAtFrame18
; @run 70:B761
; WHAT IT DOES: Steps the animation list at $B79A. When the graphic changes and the frame is $18, plays sound $42, clears slot $50 and starts an object of type $29 there at the object's place. When the list ends it sets OBJ_WORK_D 1, OBJ_WORK_E $80, OBJ_WORK_I 3 and goes to the next state.
; HOW: StepObjectFrames, SoundCommand, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenBurstAtFrame18:
    lda #$9A
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepListThenBurstAtFrame18_at0
    bcc StepListThenBurstAtFrame18_at1
    lda a:OBJ_FRAME,x
    cmp #$18
    bne StepListThenBurstAtFrame18_at1
    lda #$42
    jsr VecSoundCommand
    ldy #$50
    jsr VecClearObjectInY
    lda #$29
    jsr VecStartObjectAtPlace
StepListThenBurstAtFrame18_at1:
    rts
StepListThenBurstAtFrame18_at0:
    lda #$01
    sta a:OBJ_WORK_D,x
    lda #$80
    sta a:OBJ_WORK_E,x
    lda #$03
    sta a:OBJ_WORK_I,x
    inc a:OBJ_STATE,x
    rts

; @routine 025B:7F5 HomeOnPointThenRingAndSound
; @run 70:B7F5
; WHAT IT DOES: Steps the animation list at $B6DA while it flies toward the point X $00E0, Y $0030 on a circle of radius $0100; once on it (within 3) plays sound $73, sets OBJ_WORK_I 4 and OBJ_WORK_E $B0, starts list $83 on channel 5, restarts the animation, and goes to state $0B with OBJ_SUBSTATE 2.
; HOW: StepObjectFrames, AngleToPoint, SetVelocityFromAngle, ApplyVelocity, two range tests, SoundCommand, StartListChannelKind2.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
HomeOnPointThenRingAndSound:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    stz z:zTemp09
    lda #$E0
    sta z:zTemp08
    stz z:zTemp0B
    lda #$30
    sta z:zTemp0A
    jsr VecAngleToPoint
    pha
    stz z:zTemp00
    lda #$01
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne HomeOnPointThenRingAndSound_at0
    lda a:OBJ_X_LO,x
    sec
    sbc #$E0
    clc
    adc #$04
    cmp #$08
    bcs HomeOnPointThenRingAndSound_at0
    lda a:OBJ_Y_LO,x
    sec
    sbc #$30
    clc
    adc #$04
    cmp #$08
    bcs HomeOnPointThenRingAndSound_at0
    lda #$73
    jsr VecSoundCommand
    lda #$04
    sta a:OBJ_WORK_I,x
    lda #$B0
    sta a:OBJ_WORK_E,x
    phx
    lda #$83
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$0B
    sta a:OBJ_STATE,x
    lda #$02
    sta a:OBJ_SUBSTATE,x
HomeOnPointThenRingAndSound_at0:
    rts
