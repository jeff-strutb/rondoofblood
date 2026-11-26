; @card 01:CB0F card_boot_init
; WHAT IT DOES: Clears the card's RAM and starts it: the zero page and $2200 to $3FFF zeroed, the vectors set, the drive
; reset, interrupts allowed, the hardware put in its first state, the display set up.
; HOW: Two block moves that copy a zero upward, then init_card_vectors, reset_cd_unit, CLI, init_card_hardware, ex_irqon
; and boot_display.
; WAITS: through boot_display's frame.
; Entry: A, X, Y (pushed by the block moves). Exit: as boot_display.
card_boot_init:
    stz z:$00
    tii $2000,$2001,$00FF
    stz a:$2200
    tii $2200,$2201,$1DFF
    bsr init_card_vectors
    bsr reset_cd_unit
    cli
    bsr init_card_hardware
    jsr ex_irqon
    jsr boot_display
    rts

; @card 01:CB64 init_card_hardware
; WHAT IT DOES: Puts the machine in its first state: the PSG, timer and pad port written to zero, the interrupts off, the
; display set to 32 columns by 30 rows, the sprite table's address chosen, the pads read twice and the sprite table cleared.
; HOW: Zeros to the ports, a few of the card's bytes set, then the card's own entries called in turn: ex_hmode,
; ex_irqoff, ex_scrmod, ex_scrsiz, ex_dmamod, ex_joysns twice with every pad asked for, ex_satclr and ex_sprdma.
; WAITS: no.
; Entry: nothing. Exit: as ex_sprdma.
init_card_hardware:
    stz z:CardVdcReg
    stz a:$0800
    stz a:$0801
    stz a:$0C00
    stz a:$0C01
    stz a:$1403
    stz a:$1000
    stz a:$1402
    stz z:CardIrqFlags
    stz a:$221F
    lda #$10
    sta a:CardVdcMwr
    stz z:$F3
    stz z:$F4
    jsr ex_hmode
    jsr ex_irqoff
    cla
    ldx #$20
    ldy #$1E
    jsr ex_scrmod
    lda #$00
    jsr ex_scrsiz
    lda #$00
    jsr ex_dmamod
    lda #$00
    sta a:CardSatAddress
    lda #$10
    sta a:$2215
    stz a:$2226
    stz a:CardPadAsked
    jsr ex_joysns
    jsr ex_joysns
    lda #$1F
    sta a:CardPadAsked
    jsr ex_satclr
    jsr ex_sprdma
    rts
