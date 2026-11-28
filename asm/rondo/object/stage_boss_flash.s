; @routine 024E:525 BossGaugeFlash
; @run 6B:8D25
; WHAT IT DOES: A frame of the boss gauge's flash: masks four cells, counts the flash's step, makes the object in
; slot 85 fall faster and move, and at the bottom hides it and clears bit 2 of zField8E.
; HOW: The cell's address from BossFlashAddresses by zTemp01 (from the wave count's bit 0, two at a time below 8);
; OBJ_VY of slot 85 plus $0C; ApplyVelocity.
; WAITS: no.
; Entry: nothing. Exit: A the object's Y high, N and Z from it (RMB sets none); X 85.
BossGaugeFlash:
    lda z:zWaveCounter
    and #$01
    sta z:zTemp01
BossGaugeFlash_at0:
    lda z:zTemp01
    asl a
    tax
    ldy a:BossFlashAddresses+1,x
    lda a:BossFlashAddresses,x
    tax
    jsr BossFlashMaskCell
    inc z:zTemp01
    inc z:zTemp01
    lda z:zTemp01
    cmp #$08
    bcc BossGaugeFlash_at0
    inc a:BossFlashStep
    lda a:OBJ_VY_LO+85
    clc
    adc #$0C
    sta a:OBJ_VY_LO+85
    bcc BossGaugeFlash_at1
    inc a:OBJ_VY_HI+85
BossGaugeFlash_at1:
    ldx #$55
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI+85
    beq SetGaugeAddressB_ret
    stz a:OBJ_FRAME+85
    rmb2 z:zField8E
    rts

; @routine 024E:564 BossFlashMaskCell
; @run 6B:8D64
; WHAT IT DOES: Masks one cell of VRAM, $40 words, with random bits and a pattern by the flash's step.
; HOW: The VRAM address from X and Y ($937D); for each word, BossFlashMasks by the step's low three bits OR a random
; byte OR zRandom2 OR zRandom1 is both masks of AndVramData, X kept on the stack.
; WAITS: no.
; Entry: X and Y the address. Exit: X 0, N 0, Z 1; A, Y, C and V as the last AndVramData left them.
BossFlashMaskCell:
    jsr SetVramAddresses
    ldx #$40
BossFlashMaskCell_at0:
    lda a:BossFlashStep
    and #$07
    tay
    jsr VecNextRandom
    ora z:zRandom2
    ora z:zRandom1
    ora a:BossFlashMasks,y
    phx
    tax
    tay
    jsr AndVramData
    plx
    dex
    bne BossFlashMaskCell_at0
    rts

