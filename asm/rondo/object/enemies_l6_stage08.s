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
