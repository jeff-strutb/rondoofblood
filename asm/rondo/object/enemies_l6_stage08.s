; @routine 088A:31C LoadThenSpeedF0
; @run 70:AB1C
; WHAT IT DOES: Loads the object's record and sets its X speed high to $F0.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: A $F0, N set, Z clear; C and Y from the record.
LoadThenSpeedF0:
    jsr VecLoadObjectRecord
    lda #$F0
    sta a:OBJ_VX_HI,x
    rts

; @routine 088A:325 NextStateOnScrollY
; @run 70:AB25
; WHAT IT DOES: Goes on to the next state when the scroll's Y high byte is not zero.
; HOW: A read and an INC.
; WAITS: no.
; Entry: X object. Exit: the load, or the new state's N and Z.
NextStateOnScrollY:
    lda a:ScrollYHi
    beq NextStateOnScrollY_at0
    inc a:OBJ_27A8,x
NextStateOnScrollY_at0:
    rts

; @routine 088A:41E StartListSoundThenSpawn20
; @run 70:AC1E
; WHAT IT DOES: Starts the script list $83 on channel 4, starts a kind $20 object in slot $10 at this object's place and clears this object.
; HOW: StartListChannelKind2, StartObjectAtPlace, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object. Exit: as ClearObject.
StartListSoundThenSpawn20:
    phx
    lda #$83
    ldx #$04
    jsr VecStartListChannelKind2
    plx
    ldy #$10
    lda #$20
    jsr VecStartObjectAtPlace
    jmp VecClearObject

; @routine 088A:52D CountThenNextStateF3510
; @run 70:AD2D
; WHAT IT DOES: Counts OBJ_3510 down; at zero it sets it to $30 and goes on to the next state.
; HOW: DEC, a store, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new state's N and Z (A $30).
CountThenNextStateF3510:
    dec a:OBJ_3510,x
    bne CountThenNextStateF3510_at0
    lda #$30
    sta a:OBJ_3510,x
    inc a:OBJ_27A8,x
CountThenNextStateF3510_at0:
    rts

; @routine 088A:71E PlaySoundTwo
; @run 70:AF1E
; WHAT IT DOES: Plays sound 2.
; HOW: SoundCommand.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: see abi_sound_end.
PlaySoundTwo:
    lda #$02
    jsr VecSoundCommand
    rts

; @routine 088A:724 SetFrameThenCountToNext
; @run 70:AF24
; WHAT IT DOES: Sets the object's frame (OBJ_2A68) to $0C and counts OBJ_3568 down; at zero it sets it to $10 and goes on to the next state.
; HOW: A store, DEC, a store, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new state's N and Z (A $10).
SetFrameThenCountToNext:
    lda #$0C
    sta a:OBJ_2A68,x
    dec a:OBJ_3568,x
    bne SetFrameThenCountToNext_at0
    lda #$10
    sta a:OBJ_3568,x
    inc a:OBJ_27A8,x
SetFrameThenCountToNext_at0:
    rts

; @routine 088A:765 StepListByIndex088A
; @run 70:AF65
; WHAT IT DOES: Steps the animation list whose address the table at $AF72 gives for the index in Y.
; HOW: Two table reads (bank 70), a tail jump into StepObjectFrames.
; WAITS: no.
; Entry: X object, Y the table index. Exit: as StepObjectFrames.
StepListByIndex088A:
    lda a:$AF72,y
    sta z:zTemp10
    lda a:$AF73,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 088B:084 StartFallWithFrame21
; @run 70:B084
; WHAT IT DOES: Zeroes the object's X speed, sets its Y speed high to $FD and its frame to $21 and goes on to the next OBJ_2800.
; HOW: Stores and an INC.
; WAITS: no.
; Entry: X object. Exit: A $21, N and Z from the new OBJ_2800.
StartFallWithFrame21:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    lda #$21
    sta a:OBJ_2A68,x
    inc a:OBJ_2800,x
    rts

; @routine 088B:21D TurnAroundInState9
; @run 70:B21D
; WHAT IT DOES: Sets the object's state to 9 and reverses its X speed (a tail jump into NegateVelocityX).
; HOW: A store, NegateVelocityX.
; WAITS: no.
; Entry: X object. Exit: as NegateVelocityX.
TurnAroundInState9:
    lda #$09
    sta a:OBJ_27A8,x
    jmp VecNegateVelocityX

; @routine 088B:225 FallBackInState9
; @run 70:B225
; WHAT IT DOES: Sets the object's state to 9 and reverses its Y speed (a tail jump into NegateVelocityY).
; HOW: A store, NegateVelocityY.
; WAITS: no.
; Entry: X object. Exit: as NegateVelocityY.
FallBackInState9:
    lda #$09
    sta a:OBJ_27A8,x
    jmp VecNegateVelocityY

; @routine 088B:6C5 ClearFrame
; @run 70:B6C5
; WHAT IT DOES: Sets the object's frame to 0.
; HOW: A store.
; WAITS: no.
; Entry: X object. Exit: all kept.
ClearFrame:
    stz a:OBJ_2A68,x
    rts
; @routine 088B:777 SetTimersOf12To15
; @run 70:B777
; WHAT IT DOES: Sets OBJ_34B8 of the objects 18 to 21 to 1.
; HOW: A loop of Y from 18.
; WAITS: no.
; Entry: nothing. Exit: A 1, Y 22, the compare of Y with 22.
SetTimersOf12To15:
    ldy #$12
SetTimersOf12To15_at0:
    lda #$01
    sta a:OBJ_34B8,y
    iny
    cpy #$16
    bcc SetTimersOf12To15_at0
    rts
