; @card 01:CC17 msf_to_lba
; WHAT IT DOES: Makes the disc position in CardCdResult, as BCD minutes, seconds and frames,
; into a sector number in _AL, _AH, _BL, high first.
; HOW: Each byte goes to binary in place (ex_bcdbin, which leaves a bad byte as it was); then
; ((minutes times 60 plus seconds) times 75 plus frames) less 150, the frames before sector 0,
; by shifts and adds of the term in CardPointerLo, in 24 bits.
; Entry: CardCdResult. Exit: A the high byte, N, Z, C and V from adding the last term's high
; byte. X, Y kept. The term is left as the frames less 150.
msf_to_lba:
    lda a:CardCdResult
    jsr ex_bcdbin
    sta a:CardCdResult
    lda a:CardCdResult+1
    jsr ex_bcdbin
    sta a:CardCdResult+1
    lda a:CardCdResult+2
    jsr ex_bcdbin
    sta a:CardCdResult+2
    stz z:CardAl
    stz z:CardAh
    stz z:CardBl
    lda a:CardCdResult
    bsr lba_load_term
    bsr lba_double_term
    bsr lba_double_and_add
    bsr lba_double_and_add
    bsr lba_double_and_add
    bsr lba_double_and_add
    lda a:CardCdResult+1
    bsr lba_load_term
    bsr lba_add_term
    lda z:CardAl
    sta z:CardPointerLo
    lda z:CardAh
    sta z:CardPointerHi
    lda z:CardBl
    sta z:CardCopyLo
    bsr lba_double_and_add
    bsr lba_double_term
    bsr lba_double_and_add
    bsr lba_double_term
    bsr lba_double_term
    bsr lba_double_and_add
    lda a:CardCdResult+2
    sec
    sbc #$96
    sta z:CardCopyLo
    cla
    sbc #$00
    sta z:CardPointerHi
    cla
    sbc #$00
    sta z:CardPointerLo
    bra lba_add_term

; @card 01:CC7A lba_double_term
; WHAT IT DOES: Doubles the term.
; HOW: A shift left through its three bytes.
; Entry: the term. Exit: C its old top bit, N its new top bit, Z if its new high byte is zero.
; A, X, Y, V kept. Gives the old top bit.
lba_double_term:
    asl z:CardCopyLo
    rol z:CardPointerHi
    rol z:CardPointerLo
    rts

; @card 01:CC81 lba_double_and_add
; WHAT IT DOES: Doubles the term, then adds it to the sum.
; HOW: lba_double_term, then lba_add_term.
; Entry: the sum, the term. Exit: as lba_add_term.
lba_double_and_add:
    bsr lba_double_term

; @card 01:CC83 lba_add_term
; WHAT IT DOES: Adds the term to the sum.
; HOW: A three-byte add, low byte first.
; Entry: the sum, the term. Exit: A the sum's high byte, N, Z, C and V from adding the high
; bytes. X, Y kept.
lba_add_term:
    lda z:CardBl
    clc
    adc z:CardCopyLo
    sta z:CardBl
    lda z:CardAh
    adc z:CardPointerHi
    sta z:CardAh
    lda z:CardAl
    adc z:CardPointerLo
    sta z:CardAl
    rts

; @card 01:CC97 lba_load_term
; WHAT IT DOES: Makes a byte the term to add.
; HOW: The byte to the term's low byte, its upper two bytes cleared.
; Entry: A. Exit: all registers and flags kept.
lba_load_term:
    sta z:CardCopyLo
    stz z:CardPointerHi
    stz z:CardPointerLo
    rts

