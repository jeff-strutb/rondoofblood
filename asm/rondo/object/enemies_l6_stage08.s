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
