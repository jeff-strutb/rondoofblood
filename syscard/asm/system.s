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

; @card 00:F02D ex_getver
; WHAT IT DOES: Gives the card's version: 3.00 on this card.
; HOW: Two bytes of the ROM.
; Entry: nothing. Exit: X the major version, Y the minor, N and Z from Y. A, C, V kept.
ex_getver:
    ldx a:CardVersionMajor
    ldy a:CardVersionMinor
    rts


; @card 00:E13A map_boot_ram
; WHAT IT DOES: Maps the five CD RAM banks the boot program runs in at $4000-$DFFF.
; HOW: The bank in CardBootRamBank to MPR2 and the four after it to MPR3 to MPR6.
; Entry: nothing. Exit: A the last bank mapped, N and Z from it. X, Y, C, V kept. Gives the last
; bank.
map_boot_ram:
    lda a:CardBootRamBank
    tam #$04
    inc a
    tam #$08
    inc a
    tam #$10
    inc a
    tam #$20
    inc a
    tam #$40
    rts


; @card 01:CB30 init_card_vectors
; WHAT IT DOES: Sets the card's vectors at boot: the soft reset to $E000, and all six interrupt
; handlers to the card's default at $E8E2.
; HOW: Writes the reset vector and CardResetMark, then the first handler, and a block move
; repeats its two bytes over the other five.
; Entry: nothing. Exit: A $E8, N 1, Z 0. X, Y, C, V kept.
init_card_vectors:
    lda #$00
    sta a:CardSoftResetLo
    lda #$E0
    sta a:CardSoftResetHi
    lda #$80
    sta z:CardResetMark
    stz z:CardZpE6
    lda #$E2
    sta a:CardIrqVectors
    lda #$E8
    sta a:CardIrqVectors+1
    tai CardIrqVectors,CardIrqVectors+2,10
    stz z:CardIrqFlags
    rts


; @card 00:E67E ex_rnd
; WHAT IT DOES: Gives a pseudo-random byte from the three bytes of state in CardRndSeed.
; HOW: The state's low byte is XORed into its middle byte, and its high byte XORed with it
; becomes the new low byte; those two then shift left one as a word, taking in bit 6 of the old
; middle byte XOR bit 0 of the old low byte. The high byte never changes. The byte given is the
; new low and middle bytes XORed. The flags the first rotate leaves are kept on the stack across the
; update and taken back.
; WAITS: no.
; Entry: CardRndSeed; the flags, which go on the stack. Exit: A the byte, N and Z from it, C the bit
; shifted out of the middle byte. X, Y, V kept.
ex_rnd:
    lda a:CardRndSeed+1
    rol a
    rol a
    rol a
    eor a:CardRndSeed
    ror a
    php
    lda a:CardRndSeed+1
    eor a:CardRndSeed
    sta a:CardRndSeed+1
    lda a:CardRndSeed+2
    eor a:CardRndSeed
    sta a:CardRndSeed
    plp
    rol a:CardRndSeed
    rol a:CardRndSeed+1
    lda a:CardRndSeed+1
    eor a:CardRndSeed
    rts
