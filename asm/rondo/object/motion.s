; @routine 0045:5D2 ApplyVelocity
; @run 69:55D2
; WHAT IT DOES: Moves an object by its velocity on both axes.
; HOW: X, then runs into ApplyVelocityY.
; Entry: X object. Exit: as ApplyVelocityY. Clobbers zTemp10.
ApplyVelocity:
    stz z:zTemp10
    lda a:OBJ_VX_HI,x
    bpl ApplyVelocity_at0
    dec z:zTemp10
ApplyVelocity_at0:
    lda a:OBJ_VX_LO,x
    clc
    adc a:OBJ_X_FRAC,x
    sta a:OBJ_X_FRAC,x
    lda a:OBJ_VX_HI,x
    adc a:OBJ_X_LO,x
    sta a:OBJ_X_LO,x
    lda z:zTemp10
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x

; @routine 0045:5F6 ApplyVelocityY
; @run 69:55F6
; WHAT IT DOES: Moves an object along Y by its Y velocity.
; HOW: See apply_axis.
; Entry: X object. Exit: as ApplyVelocityX. Clobbers zTemp10.
ApplyVelocityY:
    stz z:zTemp10
    lda a:OBJ_VY_HI,x
    bpl ApplyVelocityY_at0
    dec z:zTemp10
ApplyVelocityY_at0:
    lda a:OBJ_VY_LO,x
    clc
    adc a:OBJ_Y_FRAC,x
    sta a:OBJ_Y_FRAC,x
    lda a:OBJ_VY_HI,x
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    lda z:zTemp10
    adc a:OBJ_Y_HI,x
    sta a:OBJ_Y_HI,x
    rts

; @routine 0045:61B ApplyVelocityX
; @run 69:561B
; WHAT IT DOES: Moves an object along X by its X velocity.
; HOW: See apply_axis. The bytes repeat the head of ApplyVelocity, as its own
; entry that returns.
; Entry: X object. Exit: A the position's high byte, N, Z, C and V from its add.
; X and Y kept. Clobbers zTemp10.
ApplyVelocityX:
    stz z:zTemp10
    lda a:OBJ_VX_HI,x
    bpl ApplyVelocityX_at0
    dec z:zTemp10
ApplyVelocityX_at0:
    lda a:OBJ_VX_LO,x
    clc
    adc a:OBJ_X_FRAC,x
    sta a:OBJ_X_FRAC,x
    lda a:OBJ_VX_HI,x
    adc a:OBJ_X_LO,x
    sta a:OBJ_X_LO,x
    lda z:zTemp10
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x
    rts

; @routine 0046:32D NegateVelocityX
; @run 69:5B2D
; WHAT IT DOES: Reverses an object's X velocity.
; HOW: Two's complement of the word, low byte first.
; Entry: X object. Exit: A the high byte, N, Z, C and V from its add. X and Y kept.
NegateVelocityX:
    lda a:OBJ_VX_LO,x
    eor #$FF
    clc
    adc #$01
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    eor #$FF
    adc #$00
    sta a:OBJ_VX_HI,x
    rts

; @routine 0046:343 NegateVelocityY
; @run 69:5B43
; WHAT IT DOES: Reverses an object's Y velocity.
; HOW: Two's complement of the word, low byte first.
; Entry: X object. Exit: as NegateVelocityX.
NegateVelocityY:
    lda a:OBJ_VY_LO,x
    eor #$FF
    clc
    adc #$01
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    eor #$FF
    adc #$00
    sta a:OBJ_VY_HI,x
    rts
