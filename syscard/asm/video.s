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

; @card 00:E292 ex_scrmod_fallback
; WHAT IT DOES: Where ex_scrmod refuses a size: sets 32 columns by 30 rows, narrow, and fails.
; HOW: Calls ex_scrmod with mode 0, $20 columns and $1E rows, then gives 1 with C set.
; Entry: nothing. Exit: A 1, N 0, Z 0, C set, X $1F, Y 0, V as ex_scrmod leaves it.
ex_scrmod_fallback:
    cla
    ldx #$20
    ldy #$1E
    bsr ex_scrmod
    lda #$01
    sec
    rts

; @card 00:E29D ex_scrmod
; WHAT IT DOES: Sets the screen's size and dot clock: A the mode (bit 0 wide, bit 2 the dot
; clock), X the columns of tiles, Y the rows.
; HOW: Keeps the three; refuses rows outside $0A-$1E, or columns outside $0A-$22 ($10-$2C
; wide), by setting the fallback mode instead; sets the vertical display, then the horizontal
; sync, start, width and end, and the memory width register's dot width.
; Entry: A, X, Y. Exit, set: C clear, A the memory width register, N and Z from it, X the
; columns less one, Y the dot bits, V from adding the end's base. Refused: as ex_scrmod_fallback.
ex_scrmod:
    sta a:CardScreenMode
    and #$05
    sta a:CardScreenMode
    stx a:CardScreenColumns
    sty a:CardScreenRows
    lda a:CardScreenRows
    cmp #$0A
    bcc ex_scrmod_fallback
    cmp #$1F
    bcs ex_scrmod_fallback
    tst #$01,a:CardScreenMode
    bne ex_scrmod_at0
    lda a:CardScreenColumns
    cmp #$0A
    bcc ex_scrmod_fallback
    cmp #$23
    bcs ex_scrmod_fallback
    bsr set_screen_rows
    lda #$0A
    sta z:CardVdcReg
    sta a:VdcAddress
    st1 #$02
    lda a:CardScreenColumns
    tax
    dex
    lsr a
    eor #$FF
    clc
    adc #$13
    sta a:VdcDataHi
    clc
    adc a:CardScreenColumns
    eor #$FF
    tay
    lda #$0B
    sta z:CardVdcReg
    sta a:VdcAddress
    stx a:VdcDataLo
    tya
    clc
    adc #$27
    sta a:VdcDataHi
    cla
    jsr ex_dotmod
    clc
    rts
ex_scrmod_at0:
    lda a:CardScreenColumns
    cmp #$10
    bcc ex_scrmod_fallback
    cmp #$2D
    bcs ex_scrmod_fallback
    bsr set_screen_rows
    lda #$0A
    sta z:CardVdcReg
    sta a:VdcAddress
    st1 #$03
    lda a:CardScreenColumns
    tax
    dex
    lsr a
    eor #$FF
    clc
    adc #$19
    sta a:VdcDataHi
    clc
    adc a:CardScreenColumns
    eor #$FF
    tay
    lda #$0B
    sta z:CardVdcReg
    sta a:VdcAddress
    stx a:VdcDataLo
    tya
    clc
    adc #$35
    sta a:VdcDataHi
    lda #$0A
    jsr ex_dotmod
    clc
    rts

; @card 00:E341 set_screen_rows
; WHAT IT DOES: Sets the VDC's vertical display for CardScreenRows rows of tiles, and the VCE's
; dot clock from CardScreenMode.
; HOW: Vertical sync two lines wide, the display starting $87 less half the rows' lines, the
; display rows times eight lines tall, its end at $7E plus half the lines (the shift's carry is
; the one, as the lines are always a multiple of eight); then the mode with
; bit 2 flipped goes to the VCE.
; Entry: CardScreenRows, CardScreenMode. Exit: A what went to the VCE, N and Z from it, X the
; lines less one, C and V from adding the end's base. Y kept.
set_screen_rows:
    lda #$0C
    sta z:CardVdcReg
    sta a:VdcAddress
    st1 #$02
    lda a:CardScreenRows
    asl a
    asl a
    asl a
    tax
    dex
    lsr a
    eor #$FF
    clc
    adc #$88
    sta a:VdcDataHi
    lda #$0D
    sta z:CardVdcReg
    sta a:VdcAddress
    stx a:VdcDataLo
    st2 #$00
    lda #$0E
    sta z:CardVdcReg
    sta a:VdcAddress
    txa
    inc a
    inc a
    lsr a
    adc #$7D
    sta a:VdcDataLo
    st2 #$00
    lda a:CardScreenMode
    eor #$04
    sta a:VceControl
    rts

; @card 00:E5DA ex_satclr
; WHAT IT DOES: Clears the sprite table in VRAM.
; HOW: Sets the VDC's write address to CardSatAddress and writes 256 words of zero, then
; clears CardSatFlag.
; Entry: CardSatAddress. Exit: A 2, X 0, N 0, Z 1. Y, C, V kept.
ex_satclr:
    lda #$00
    sta z:CardVdcReg
    sta a:VdcAddress
    lda a:CardSatAddress
    sta a:VdcDataLo
    lda a:CardSatAddress+1
    sta a:VdcDataHi
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
    clx
ex_satclr_at0:
    st1 #$00
    st2 #$00
    dex
    bne ex_satclr_at0
    stz a:CardSatFlag
    rts

