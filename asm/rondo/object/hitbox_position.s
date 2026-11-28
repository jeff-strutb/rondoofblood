; @routine 024F:413 ComputeHitboxPositions
; @run 6B:9413
; WHAT IT DOES: Works out where the hitbox centre of each of the first sixteen objects lies:
; its position plus the hitbox's offset, the X offset turned round when the object faces
; left. An object without a hitbox is passed over and its entry left as it was.
; HOW: Per object, the X offset (negated when facing left) is sign extended and added to X
; low and high, to HITBOX x_lo and, in the high byte, hi; then the Y offset the same way, to
; HITBOX y_lo, its sign extended high byte added to Y high and ORed into hi.
; WAITS: no.
; Entry: nothing. Exit: A the last value the loop had in A, X $10, N 0, Z 1, C 1. Y kept.
ComputeHitboxPositions:
    ldx #$00
ComputeHitboxPositions_at4:
    lda a:OBJ_HITBOX_ON,x
    beq ComputeHitboxPositions_at0
    lda a:OBJ_HITBOX_DX,x
    sta z:zTemp01
    lda a:OBJ_FACING_LEFT,x
    beq ComputeHitboxPositions_at1
    lda z:zTemp01
    eor #$FF
    inc a
    sta z:zTemp01
ComputeHitboxPositions_at1:
    stz z:zTemp00
    lda z:zTemp01
    bpl ComputeHitboxPositions_at2
    dec z:zTemp00
ComputeHitboxPositions_at2:
    clc
    adc a:OBJ_X_LO,x
    sta a:HitboxXLo,x
    lda a:OBJ_X_HI,x
    adc z:zTemp00
    sta a:HitboxHi,x
    stz z:zTemp00
    lda a:OBJ_HITBOX_DY,x
    bpl ComputeHitboxPositions_at3
    dec z:zTemp00
ComputeHitboxPositions_at3:
    lda a:OBJ_Y_LO,x
    clc
    adc a:OBJ_HITBOX_DY,x
    sta a:HitboxYLo,x
    lda a:OBJ_Y_HI,x
    adc z:zTemp00
    ora a:HitboxHi,x
    sta a:HitboxHi,x
ComputeHitboxPositions_at0:
    inx
    cpx #$10
    bcc ComputeHitboxPositions_at4
    rts

; @routine 05BF:725 ComputeHitboxPositionsLong
; @run 6B:9725
; WHAT IT DOES: ComputeHitboxPositions in 05BF's group of stage overlays: the hitbox centres of the first sixteen objects.
; HOW: The same instructions as ComputeHitboxPositions.
; WAITS: no.
; Entry: nothing. Exit: as ComputeHitboxPositions.
ComputeHitboxPositionsLong:
    ldx #$00
ComputeHitboxPositionsLong_at4:
    lda a:OBJ_HITBOX_ON,x
    beq ComputeHitboxPositionsLong_at0
    lda a:OBJ_HITBOX_DX,x
    sta z:zTemp01
    lda a:OBJ_FACING_LEFT,x
    beq ComputeHitboxPositionsLong_at1
    lda z:zTemp01
    eor #$FF
    inc a
    sta z:zTemp01
ComputeHitboxPositionsLong_at1:
    stz z:zTemp00
    lda z:zTemp01
    bpl ComputeHitboxPositionsLong_at2
    dec z:zTemp00
ComputeHitboxPositionsLong_at2:
    clc
    adc a:OBJ_X_LO,x
    sta a:HitboxXLo,x
    lda a:OBJ_X_HI,x
    adc z:zTemp00
    sta a:HitboxHi,x
    stz z:zTemp00
    lda a:OBJ_HITBOX_DY,x
    bpl ComputeHitboxPositionsLong_at3
    dec z:zTemp00
ComputeHitboxPositionsLong_at3:
    lda a:OBJ_Y_LO,x
    clc
    adc a:OBJ_HITBOX_DY,x
    sta a:HitboxYLo,x
    lda a:OBJ_Y_HI,x
    adc z:zTemp00
    ora a:HitboxHi,x
    sta a:HitboxHi,x
ComputeHitboxPositionsLong_at0:
    inx
    cpx #$10
    bcc ComputeHitboxPositionsLong_at4
    rts
