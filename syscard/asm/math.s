; @card 00:FDA9 map_math_bank
; WHAT IT DOES: Maps the math routines at $C000, keeping what was there.
; HOW: MPR6 goes to CardSavedMpr6, then bank 3 to MPR6.
; Entry: nothing. Exit: A kept, N and Z from it. X, Y, C, V kept.
map_math_bank:
    pha
    tma #$40
    sta a:CardSavedMpr6
    lda #$03
    tam #$40
    pla
    rts

; @card 00:FDB5 unmap_math_bank
; WHAT IT DOES: Puts back what map_math_bank kept.
; HOW: CardSavedMpr6 to MPR6, with A and the flags saved around it.
; Entry: nothing. Exit: all registers and flags kept.
unmap_math_bank:
    php
    pha
    lda a:CardSavedMpr6
    tam #$40
    pla
    plp
    rts

; @card 03:CD3A mul_u8
; WHAT IT DOES: Multiplies two unsigned bytes for a 16-bit product in _CX.
; HOW: Shift and add over eight rounds in X:A, the multiplier in X moving out as the
; product moves in.
; Entry: _AL, _BL. Exit: A the low byte, X the high, Y 0, N 0, Z 1; C and V from the add
; of the lowest set bit of _BL, C clear if that was not bit 0, V kept if _BL is zero.
mul_u8:
    lda z:CardAl
    ldx z:CardBl
    sta z:CardCl
    cla
    ldy #$08
mul_u8_at0:
    asl a
    sax
    rol a
    sax
    bcc mul_u8_at1
    clc
    adc z:CardCl
    bcc mul_u8_at1
    inx
mul_u8_at1:
    dey
    bne mul_u8_at0
    stx z:CardCh
    sta z:CardCl
    rts

; @card 00:FDC6 ma_mul8u
; WHAT IT DOES: The BIOS's unsigned byte multiply.
; HOW: Maps the math bank, mul_u8, puts it back.
; Entry: _AL, _BL. Exit: as mul_u8.
ma_mul8u:
    bsr map_math_bank
    jsr mul_u8
    bra unmap_math_bank
