; @routine 025B:0CA StartChannel81OnceThenStepScrollTable
; @run 70:B0CA
; WHAT IT DOES: When OBJ_3670 is 0 and the game's sub-mode is not 5 sets it 1 and starts list $81 on channel 5; then runs SetBossFixedFields and StepVerticalScrollTable.
; HOW: Two tests, an INC, StartListChannelKind2 between a PHX and a PLX, a call and a tail jump.
; WAITS: no.
; Entry: X object. Exit: as StepVerticalScrollTable.
StartChannel81OnceThenStepScrollTable:
    lda a:OBJ_3670,x
    bne StartChannel81OnceThenStepScrollTable_at0
    lda z:zGameSubMode
    cmp #$05
    beq StartChannel81OnceThenStepScrollTable_at0
    inc a:OBJ_3670,x
    phx
    ldx #$05
    lda #$81
    jsr VecStartListChannelKind2
    plx
StartChannel81OnceThenStepScrollTable_at0:
    jsr SetBossFixedFields
    jmp StepVerticalScrollTable

; @routine 025B:0E7 NextStateThenStepScrollTable
; @run 70:B0E7
; WHAT IT DOES: Goes to the next state and runs StepVerticalScrollTable.
; HOW: An INC and a tail jump.
; WAITS: no.
; Entry: X object. Exit: as StepVerticalScrollTable.
NextStateThenStepScrollTable:
    inc a:OBJ_STATE,x
    jmp StepVerticalScrollTable

; @routine 025B:5E9 StepListMarksThenSound74AtC8
; @run 70:B5E9
; WHAT IT DOES: Steps the animation list at $B612, moves, runs StartScriptAtTimerMarks; when the object's X low byte is $C8 plays sound $74, clears the animation and goes to the next state.
; HOW: StepObjectFrames, ApplyVelocity, a call, a compare, SoundCommand, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListMarksThenSound74AtC8:
    lda #$12
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    jsr StartScriptAtTimerMarks
    lda a:OBJ_X_LO,x
    cmp #$C8
    bcc RichterReadPad
    bne RichterReadPad
    lda #$74
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 025B:6B1 SpinThenCountDownToState
; @run 70:B6B1
; WHAT IT DOES: Steps the animation list at $B6DA, sets slot 0's OBJ_3510 2 and OBJ_3568 0, runs SpinOnCircle and counts OBJ_3510 down; at zero sets OBJ_3510 $40 and OBJ_3670 2 and goes to the next state.
; HOW: StepObjectFrames, SpinOnCircle, a memory decrement, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
SpinThenCountDownToState:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$02
    sta a:OBJ_3510
    stz a:OBJ_3568
    jsr SpinOnCircle
    dec a:OBJ_3510,x
    bne SpinThenCountDownToState_at0
    lda #$40
    sta a:OBJ_3510,x
    lda #$02
    sta a:OBJ_3670,x
    inc a:OBJ_STATE,x
SpinThenCountDownToState_at0:
    rts

; @routine 025B:718 SpinThenCountDownToPhase80
; @run 70:B718
; WHAT IT DOES: As SpinThenCountDownToState, but at zero it sets OBJ_36C8 $80 and goes to the next state.
; HOW: StepObjectFrames, SpinOnCircle, a memory decrement, a store, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
SpinThenCountDownToPhase80:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$02
    sta a:OBJ_3510
    stz a:OBJ_3568
    jsr SpinOnCircle
    dec a:OBJ_3510,x
    bne OrbitStepBack
    lda #$80
    sta a:OBJ_36C8,x
    inc a:OBJ_STATE,x
    rts

; @routine 025B:73C SpinThenCountPhaseDown
; @run 70:B73C
; WHAT IT DOES: Steps the animation list at $B6DA, runs SpinOnCircle, sets OBJ_3670 2 and counts OBJ_36C8 down; at zero clears the animation and OBJ_3670 and goes to the next state.
; HOW: StepObjectFrames, SpinOnCircle, a memory decrement, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
SpinThenCountPhaseDown:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr SpinOnCircle
    lda #$02
    sta a:OBJ_3670,x
    dec a:OBJ_36C8,x
    bne OrbitStepForward
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_3670,x
    inc a:OBJ_STATE,x
    rts

; @routine 025B:7C7 SpinThenCountWordDown
; @run 70:B7C7
; WHAT IT DOES: Steps the animation list at $B6DA, runs SpinOnCircle and counts the 16-bit timer OBJ_34B8:OBJ_3510 down; at zero clears OBJ_3250, goes to the next state and clears OBJ_SUBSTATE.
; HOW: StepObjectFrames, SpinOnCircle, a 16-bit subtract in memory, an ORA, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
SpinThenCountWordDown:
    lda #$DA
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr SpinOnCircle
    lda a:OBJ_3510,x
    sec
    sbc #$01
    sta a:OBJ_3510,x
    lda a:OBJ_34B8,x
    sbc #$00
    sta a:OBJ_34B8,x
    ora a:OBJ_3510,x
    bne SpinThenCountWordDown_at0
    stz a:OBJ_3250,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SpinThenCountWordDown_at0:
    rts

; @routine 0B4B:690 CountDownThenDrawAndSpawnRing
; @run 70:B690
; WHAT IT DOES: When the scroll's screen byte is 2, counts OBJ_34B8 down; at zero draws tile script 0, starts the ring of pieces (StartRingOfPieces), plays sound $56, sets the scroll's X limit to $0280 and OBJ_34B8 $60 and goes to the next state.
; HOW: A compare, a memory decrement, DrawTileScript, StartRingOfPieces, SoundCommand, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
CountDownThenDrawAndSpawnRing:
    lda a:ScrollXHiScreen
    cmp #$02
    bne CountDownThenDrawAndSpawnRing_at0
    dec a:OBJ_34B8,x
    bne CountDownThenDrawAndSpawnRing_at0
    lda #$00
    cly
    jsr VecDrawTileScript
    jsr StartRingOfPieces
    lda #$56
    jsr VecSoundCommand
    lda #$02
    sta a:ScrollXMaxHi
    lda #$80
    sta a:ScrollXMaxLo
    lda #$60
    sta a:OBJ_34B8,x
    inc a:OBJ_STATE,x
CountDownThenDrawAndSpawnRing_at0:
    rts
