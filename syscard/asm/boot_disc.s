; @card 00:E14C check_boot_disc
; WHAT IT DOES: Finds the disc's first data track, reads its first two sectors to $2800, checks them against the card's own
; copy of the title and that the next $4A bytes are zero, and runs the program there.
; HOW: cd_dinfo for each track from CardDiscFirstTrack on, up to the last, until one's answer has the data bit; its number
; is kept and msf_to_lba makes its start a sector, which is saved twice in the base table; cd_read two sectors to $2800 with the
; card's MPR 6 bank $1F mapped; compares $4D5 bytes, then $4A zero ones, then JSR $2B26.
; WAITS: through cd_dinfo and cd_read, and in the program it runs.
; Entry: A, X, Y (pushed by the block moves). Exit: no data track: C 0, A the last track, N, Z from comparing it with the
; last, V bit 6 of the answer's last control byte. A byte differs: C 1, A that byte, N and Z from the difference. Else C 1
; after the program returns.
check_boot_disc:
    lda a:CardDiscFirstTrack
check_boot_disc_at0:
    pha
    sta z:CardAh
    lda #$02
    sta z:CardAl
    lda #$56
    sta z:CardBl
    lda #$22
    sta z:CardBh
    jsr cd_dinfo
    pla
    tst #$04,a:CardCdResult+3
    bne check_boot_disc_at2
    cmp a:CardDiscFirstTrack+1
    bcs check_boot_disc_at1
    sed
    clc
    adc #$01
    cld
    bra check_boot_disc_at0
check_boot_disc_at1:
    clc
    rts
check_boot_disc_at2:
    sta a:$2269
    jsr msf_to_lba
    tii $20F8,CardCdBases,$0003
    tii $20F8,CardCdBases+3,$0003
    stz z:CardCl
    stz z:CardCh
    stz z:CardDl
    lda #$00
    sta z:CardBl
    lda #$28
    sta z:CardBh
    lda #$01
    sta z:CardDh
    inc a
    sta z:CardAl
    stz z:CardAh
    jsr cd_read
    lda #$1F
    tam #$40
    lda #$00
    sta z:CardPointerLo
    lda #$28
    sta z:CardPointerHi
    lda #$E1
    sta z:CardCopyLo
    lda #$DA
    sta z:CardCopyHi
    lda #$D5
    sta z:CardCl
    lda #$04
    sta z:CardCh
check_boot_disc_at3:
    lda (CardPointerLo)
    cmp (CardCopyLo)
    bne check_boot_disc_at10
    inc z:CardPointerLo
    bne check_boot_disc_at4
    inc z:CardPointerHi
check_boot_disc_at4:
    inc z:CardCopyLo
    bne check_boot_disc_at5
    inc z:CardCopyHi
check_boot_disc_at5:
    lda z:CardCl
    bne check_boot_disc_at6
    dec z:CardCh
check_boot_disc_at6:
    dec z:CardCl
    lda z:CardCl
    ora z:CardCh
    bne check_boot_disc_at3
    lda #$4A
    sta z:CardCl
    lda #$00
    sta z:CardCh
check_boot_disc_at7:
    lda (CardPointerLo)
    bne check_boot_disc_at10
    inc z:CardPointerLo
    bne check_boot_disc_at8
    inc z:CardPointerHi
check_boot_disc_at8:
    inc z:CardCopyLo
    bne check_boot_disc_at9
    inc z:CardCopyHi
check_boot_disc_at9:
    lda z:CardCl
    bne check_boot_disc_at11
    dec z:CardCh
check_boot_disc_at11:
    dec z:CardCl
    lda z:CardCl
    ora z:CardCh
    bne check_boot_disc_at7
    jsr $2B26
check_boot_disc_at10:
    sec
    rts
