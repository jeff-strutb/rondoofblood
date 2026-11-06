; @card 00:E267 ex_scrsiz
; WHAT IT DOES: Sets the size of the background map.
; HOW: The size's three bits go to bits 4-6 of the memory width register.
; Entry: A size. Exit: A the register, N and Z from it, Y the bits, C clear. X and V kept.
ex_scrsiz:
    and #$07
    asl a
    asl a
    asl a
    asl a
    tay
    lda #$8F
    bra ex_dotmod_at0

; @card 00:E272 ex_dotmod
; WHAT IT DOES: Sets the VRAM dot width and sprite width of the memory width register.
; HOW: The mode's bits outside the map size go to the register.
; Entry: A mode. Exit: A the register, N and Z from it, Y the bits. X, C, V kept.
ex_dotmod:
    and #$8F
    tay
    lda #$70
ex_dotmod_at0:
    and a:CardVdcMwr
    sta a:CardVdcMwr
    tya
    ora a:CardVdcMwr
    sta a:CardVdcMwr
    lda #$09
    sta z:CardVdcReg
    sta a:VdcAddress
    lda a:CardVdcMwr
    sta a:VdcDataLo
    rts
