; @card 00:FE92 ex_memopen
; WHAT IT DOES: Finds the card's extra RAM, and the bank it starts at.
; HOW: Looks for a signature at $18C5, then at $18C1, whose bank it marks with bit 7.
; Entry: nothing. Exit, found: X the bank, A the card's byte at $FFF4, N and Z from it, C
; clear. Exit, not found: X 0, C set, A the byte last read, N and Z from its compare. Y, V
; kept.
ex_memopen:
    lda a:ArcadeRamSigA
    cmp #$AA
    bne ex_memopen_at0
    lda a:ArcadeRamSigB
    cmp #$55
    bne ex_memopen_at0
    ldx a:ArcadeRamBank
    bra ex_memopen_at1
ex_memopen_at0:
    lda a:SuperRamSigA
    cmp #$AA
    bne ex_memopen_at2
    lda a:SuperRamSigB
    cmp #$55
    bne ex_memopen_at2
    ldx a:SuperRamBank
    sax
    ora #$80
    sax
ex_memopen_at1:
    lda a:CardVersionByte
    clc
    rts
ex_memopen_at2:
    clx
    sec
    rts

; @card 00:F034 ex_setvec
; WHAT IT DOES: Sets an interrupt handler the card calls.
; HOW: A word in the table at $2200 by which; which 6 is the soft reset's, kept apart.
; Entry: A which, X and Y the address. Exit: flags as they were, A and Y kept; X the
; table's offset of the high byte unless which is 6.
ex_setvec:
    php
    sei
    cmp #$06
    bne ex_setvec_at0
    stx a:CardSoftResetLo
    sty a:CardSoftResetHi
    bra ex_setvec_at1
ex_setvec_at0:
    pha
    phy
    phx
    asl a
    tax
    pla
    sta a:CardIrqVectors,x
    inx
    pla
    sta a:CardIrqVectors,x
    pla
ex_setvec_at1:
    plp
    rts
