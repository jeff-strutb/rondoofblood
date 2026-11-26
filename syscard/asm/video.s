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

; @card 00:E63C ex_sprput
; WHAT IT DOES: Puts the sprite in CardSprite into the next of the sprite table's 64 entries in
; VRAM, and counts it.
; HOW: CardSatFlag counts the sprites put since ex_satclr; at 64 nothing is done. The write
; address is CardSatAddress plus four words for each sprite put, then the sprite's four words
; go out.
; Entry: CardSprite, CardSatFlag, CardSatAddress. Exit: C 1; X, Y kept. Put: A the sprite's
; last byte, N and Z from Y, V from adding the low byte's carry to the table's high byte. Full:
; A the count, N and Z from it less $40, V kept.
ex_sprput:
    lda a:CardSatFlag
    cmp #$40
    bcs ex_sprput_at1
    phy
    lda #$00
    sta z:CardVdcReg
    sta a:VdcAddress
    lda a:CardSatFlag
    asl a
    asl a
    clc
    adc a:CardSatAddress
    sta a:VdcDataLo
    lda a:CardSatAddress+1
    adc #$00
    sta a:VdcDataHi
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
    cly
ex_sprput_at0:
    lda a:CardSprite,y
    sta a:VdcDataLo
    iny
    lda a:CardSprite,y
    sta a:VdcDataHi
    iny
    cpy #$08
    bne ex_sprput_at0
    inc a:CardSatFlag
    ply
ex_sprput_at1:
    rts


; @card 00:E382 ex_imode
; WHAT IT DOES: Sets how far the VDC's VRAM address steps after each access: A 0 to 3 for 1,
; 32, 64 or 128 words.
; HOW: The two bits go to bits 3 and 4 of the control register's high byte, kept in
; CardVdcCrHi, which is then written.
; Entry: A. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
ex_imode:
    rmb3 z:CardVdcCrHi
    rmb4 z:CardVdcCrHi
    asl a
    asl a
    asl a
    and #$18
    ora z:CardVdcCrHi
    sta z:CardVdcCrHi
    bra ex_hmode_at0

; @card 00:E391 ex_vmode
; WHAT IT DOES: Steps the VRAM address by one column of the background map: 32 words if the map
; is 32 tiles wide, else 64.
; HOW: Reads the map's width from CardVdcMwr. (Its test of bit 0 after keeping bits 4 and 5 can
; never be true.)
; Entry: nothing. Exit: as ex_imode.
ex_vmode:
    smb3 z:CardVdcCrHi
    lda a:CardVdcMwr
    and #$30
    beq ex_hmode_at1
    smb4 z:CardVdcCrHi
    and #$01
    bne ex_hmode_at0
    rmb3 z:CardVdcCrHi
    bra ex_hmode_at0

; @card 00:E3A4 ex_hmode
; WHAT IT DOES: Steps the VRAM address by one word after each access.
; HOW: Clears both step bits, then writes the high byte.
; Entry: nothing. Exit: as ex_imode.
ex_hmode:
    rmb3 z:CardVdcCrHi
ex_hmode_at1:
    rmb4 z:CardVdcCrHi
ex_hmode_at0:
    lda #$05
    sta z:CardVdcReg
    sta a:VdcAddress
    lda z:CardVdcCrHi
    sta a:VdcDataHi
    rts

; @card 00:E3B5 ex_vsync
; WHAT IT DOES: Waits for the next vertical blank, unless bit 1 of CardIrqFlags says not to.
; HOW: Enables interrupts, turns on the VDC's vertical blank interrupt if it was off, then
; waits for CardFrameTick to change.
; Entry: nothing. Exit, no wait: everything kept. Waited: I clear, A the count it waited on, Z 0,
; N and C from comparing it with the new one. X, Y, V kept.
ex_vsync:
    bbs1 CardIrqFlags,ex_vsync_at0
    cli
    bbs3 CardVdcCrLo,ex_vsync_at1
    bsr ex_irqon
ex_vsync_at1:
    lda a:CardFrameTick
ex_vsync_at2:
    cmp a:CardFrameTick
    beq ex_vsync_at2
ex_vsync_at0:
    rts

; @card 00:E3C7 ex_rcron
; WHAT IT DOES: Turns on the VDC's raster compare interrupt.
; HOW: Sets bit 2 of CardVdcCrLo and writes the control register's low byte.
; Entry: nothing. Exit: A the low byte, N and Z from it. X, Y, C, V kept.
ex_rcron:
    smb2 z:CardVdcCrLo
    bra ex_irqoff_at0

; @card 00:E3CB ex_rcroff
; WHAT IT DOES: Turns off the VDC's raster compare interrupt.
; HOW: As ex_rcron, clearing the bit.
; Entry: nothing. Exit: as ex_rcron.
ex_rcroff:
    rmb2 z:CardVdcCrLo
    bra ex_irqoff_at0

; @card 00:E3CF ex_irqon
; WHAT IT DOES: Turns on the VDC's vertical blank interrupt.
; HOW: As ex_rcron, with bit 3.
; Entry: nothing. Exit: as ex_rcron.
ex_irqon:
    smb3 z:CardVdcCrLo
    bra ex_irqoff_at0

; @card 00:E3D3 ex_irqoff
; WHAT IT DOES: Turns off the VDC's vertical blank interrupt.
; HOW: As ex_rcroff, with bit 3.
; Entry: nothing. Exit: as ex_rcron.
ex_irqoff:
    rmb3 z:CardVdcCrLo
ex_irqoff_at0:
    lda #$05
    sta z:CardVdcReg
    sta a:VdcAddress
    lda z:CardVdcCrLo
    sta a:VdcDataLo
    rts

; @card 00:E3E2 ex_bgon
; WHAT IT DOES: Asks for the background to be shown, from the next time the control register's
; low byte is written.
; HOW: Sets bit 7 of CardVdcCrLo.
; Entry: nothing. Exit: everything kept.
ex_bgon:
    smb7 z:CardVdcCrLo
    rts

; @card 00:E3E5 ex_bgoff
; WHAT IT DOES: Asks for the background to be hidden, as ex_bgon.
; HOW: Clears bit 7 of CardVdcCrLo.
; Entry: nothing. Exit: everything kept.
ex_bgoff:
    rmb7 z:CardVdcCrLo
    rts

; @card 00:E3E8 ex_spron
; WHAT IT DOES: Asks for sprites to be shown, as ex_bgon.
; HOW: Sets bit 6 of CardVdcCrLo.
; Entry: nothing. Exit: everything kept.
ex_spron:
    smb6 z:CardVdcCrLo
    rts

; @card 00:E3EB ex_sproff
; WHAT IT DOES: Asks for sprites to be hidden, as ex_bgon.
; HOW: Clears bit 6 of CardVdcCrLo.
; Entry: nothing. Exit: everything kept.
ex_sproff:
    rmb6 z:CardVdcCrLo
    rts

; @card 00:E3EE ex_dspon
; WHAT IT DOES: Asks for the background and sprites to be shown, as ex_bgon.
; HOW: Sets bits 6 and 7 of CardVdcCrLo.
; Entry: nothing. Exit: everything kept.
ex_dspon:
    smb6 z:CardVdcCrLo
    smb7 z:CardVdcCrLo
    rts

; @card 00:E3F3 ex_dspoff
; WHAT IT DOES: Asks for the background and sprites to be hidden, as ex_bgon.
; HOW: Clears bits 6 and 7 of CardVdcCrLo.
; Entry: nothing. Exit: everything kept.
ex_dspoff:
    rmb6 z:CardVdcCrLo
    rmb7 z:CardVdcCrLo
    rts

; @card 00:E3F8 ex_dmamod
; WHAT IT DOES: Sets the VDC's DMA control register.
; HOW: Its five bits of A to CardVdcDcr, which goes to the register's low byte.
; Entry: A. Exit: A the bits, N and Z from them. X, Y, C, V kept.
ex_dmamod:
    and #$1F
    sta a:CardVdcDcr
    lda #$0F
    sta z:CardVdcReg
    sta a:VdcAddress
    lda a:CardVdcDcr
    sta a:VdcDataLo
    rts

; @card 00:E40B ex_sprdma
; WHAT IT DOES: Has the VDC copy the sprite table from VRAM at the next vertical blank.
; HOW: CardSatAddress to the VDC's sprite table address register.
; Entry: CardSatAddress. Exit: A its high byte, N and Z from it. X, Y, C, V kept.
ex_sprdma:
    lda #$13
    sta z:CardVdcReg
    sta a:VdcAddress
    lda a:CardSatAddress
    sta a:VdcDataLo
    lda a:CardSatAddress+1
    sta a:VdcDataHi
    rts

; @card 00:E41F ex_setrcr
; WHAT IT DOES: Sets the raster line the VDC interrupts at: A low, X high.
; HOW: The two bytes to the raster compare register.
; Entry: A, X. Exit: N and Z from A. A, X, Y, C, V kept.
ex_setrcr:
    pha
    lda #$06
    sta z:CardVdcReg
    sta a:VdcAddress
    pla
    sta a:VdcDataLo
    stx a:VdcDataHi
    rts

; @card 00:E42F ex_setred
; WHAT IT DOES: Sets the VRAM address to read from, A low, X high, and selects the data port.
; HOW: The address to the read address register, then the data register selected.
; Entry: A, X. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
ex_setred:
    pha
    lda #$01
    sta z:CardVdcReg
    sta a:VdcAddress
    pla
    sta a:VdcDataLo
    stx a:VdcDataHi
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
    rts

; @card 00:E446 ex_setwrt
; WHAT IT DOES: Sets the VRAM address to write to, A low, X high, and selects the data port.
; HOW: As ex_setred, with the write address register.
; Entry: A, X. Exit: as ex_setred.
ex_setwrt:
    pha
    lda #$00
    sta z:CardVdcReg
    sta a:VdcAddress
    pla
    sta a:VdcDataLo
    stx a:VdcDataHi
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
    rts

; @card 00:E45D ex_setdma
; WHAT IT DOES: Sets up a VRAM to VRAM copy: _BX the source, _DX the destination, _CX the length.
; HOW: The three to the VDC's DMA registers, then the register that was selected is selected
; again.
; Entry: _BX, _CX, _DX. Exit: A the register selected, N and Z from it. X, Y, C, V kept.
ex_setdma:
    lda z:CardVdcReg
    pha
    lda #$10
    sta z:CardVdcReg
    sta a:VdcAddress
    lda z:CardBl
    sta a:VdcDataLo
    lda z:CardBh
    sta a:VdcDataHi
    lda #$11
    sta z:CardVdcReg
    sta a:VdcAddress
    lda z:CardDl
    sta a:VdcDataLo
    lda z:CardDh
    sta a:VdcDataHi
    lda #$12
    sta z:CardVdcReg
    sta a:VdcAddress
    lda z:CardCl
    sta a:VdcDataLo
    lda z:CardCh
    sta a:VdcDataHi
    pla
    sta z:CardVdcReg
    sta a:VdcAddress
    rts


; @card 01:CBF0 clear_vram
; WHAT IT DOES: Fills the first 4K words of VRAM, where the background map lies, with tile $200.
; HOW: The write address to 0, then $0200 written 4,096 times.
; Entry: nothing. Exit: A 2, X 0, Y 0, N 0, Z 1. C, V kept.
clear_vram:
    lda #$00
    sta z:CardVdcReg
    sta a:VdcAddress
    st1 #$00
    st2 #$00
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
    ldy #$40
clear_vram_at1:
    ldx #$40
clear_vram_at0:
    lda #$00
    sta a:VdcDataLo
    lda #$02
    sta a:VdcDataHi
    dex
    bne clear_vram_at0
    dey
    bne clear_vram_at1
    rts

; @card 01:CC9E load_font
; WHAT IT DOES: Loads the card's font and its title picture's tiles into VRAM from $2000.
; HOW: 256 one-plane glyphs of eight rows from $C000 in bank 1, each row written as both bytes
; of a word, twice over, for 16 words a tile; then banks $16 and $17 mapped at $8000 for a block
; move of $2000 bytes to the data port, and the MPRs put back.
; Entry: bank 1 at $C000. Exit: A what MPR4 held, N and Z from it, X 0, Y 8, C 0, V 0.
; CardPointerLo-Hi $C800, CardCopyLo 0.
load_font:
    lda #$00
    sta z:CardVdcReg
    sta a:VdcAddress
    st1 #$00
    st2 #$20
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
    lda #$00
    sta z:CardPointerLo
    lda #$C0
    sta z:CardPointerHi
    stz z:CardCopyLo
load_font_at2:
    ldx #$02
load_font_at1:
    cly
load_font_at0:
    lda (CardPointerLo),y
    sta a:VdcDataLo
    sta a:VdcDataHi
    iny
    cpy #$08
    bcc load_font_at0
    dex
    bne load_font_at1
    clc
    lda z:CardPointerLo
    adc #$08
    sta z:CardPointerLo
    lda z:CardPointerHi
    adc #$00
    sta z:CardPointerHi
    dec z:CardCopyLo
    bne load_font_at2
    tma #$10
    pha
    tma #$20
    pha
    lda #$16
    tam #$10
    inc a
    tam #$20
    tia Mpr4Window,VdcDataLo,8192
    pla
    tam #$20
    pla
    tam #$10
    rts


; @card 00:E509 ex_colorcmd
; WHAT IT DOES: Does what CardColorCommand asks of the palettes, if anything: writes the
; background and sprite colours to the VCE, or with bit 0 reads them back into memory.
; HOW: Writes a block move in RAM at CardBlockStub (TIA to the colour data port, or TAI from it)
; and runs it once for each set whose count is not zero: the background's at colour 0, the
; sprites' at colour $100, 32 bytes a 16-colour palette. The C makes the moves itself. Then it
; clears the command.
; Entry: CardColorCommand, CardColorBackground, CardColorSprites. Exit, no command: A 0, N 0, Z 1.
; Sprites moved: A 1, the VCE address's high byte it last loaded, N 0, Z 0, C 0. Sprites skipped:
; A 0, N 0, Z 1, C 0 if the background moved, else as it was. X, Y, V kept.
; The block move it writes in RAM is done in C and compared as part of this routine (boot script).
ex_colorcmd:
    lda a:CardColorCommand
    bne ex_colorcmd_at0
    rts
ex_colorcmd_at0:
    and #$01
    beq ex_colorcmd_at1
    bne ex_colorcmd_at2
ex_colorcmd_at1:
    lda #$E3
    sta a:CardBlockStub
    lda #$04
    sta a:CardBlockStub+3
    sta a:CardBlockStub+4
    lda #$60
    sta a:CardBlockStub+7
    lda a:CardColorBackground+2
    beq ex_colorcmd_at3
    stz a:CardBlockStub+6
    asl a
    asl a
    asl a
    asl a
    rol a:CardBlockStub+6
    asl a
    rol a:CardBlockStub+6
    sta a:CardBlockStub+5
    tii CardColorBackground,CardBlockStub+1,2
    stz a:VceAddressLo
    stz a:VceAddressHi
    jsr CardBlockStub
ex_colorcmd_at3:
    lda a:CardColorSprites+2
    beq ex_colorcmd_at4
    stz a:CardBlockStub+6
    asl a
    asl a
    asl a
    asl a
    rol a:CardBlockStub+6
    asl a
    rol a:CardBlockStub+6
    sta a:CardBlockStub+5
    tii CardColorSprites,CardBlockStub+1,2
    stz a:VceAddressLo
    lda #$01
    sta a:VceAddressHi
    jsr CardBlockStub
ex_colorcmd_at4:
    stz a:CardColorCommand
    rts
ex_colorcmd_at2:
    tii ColorReadStub,CardBlockStub,3
    lda #$60
    sta a:CardBlockStub+7
    lda a:CardColorBackground+2
    beq ex_colorcmd_at5
    stz a:CardBlockStub+6
    asl a
    asl a
    asl a
    asl a
    rol a:CardBlockStub+6
    asl a
    rol a:CardBlockStub+6
    sta a:CardBlockStub+5
    tii CardColorBackground,CardBlockStub+3,2
    stz a:VceAddressLo
    stz a:VceAddressHi
    jsr CardBlockStub
ex_colorcmd_at5:
    lda a:CardColorSprites+2
    beq ex_colorcmd_at6
    stz a:CardBlockStub+6
    asl a
    asl a
    asl a
    asl a
    rol a:CardBlockStub+6
    asl a
    rol a:CardBlockStub+6
    sta a:CardBlockStub+5
    tii CardColorSprites,CardBlockStub+3,2
    stz a:VceAddressLo
    lda #$01
    sta a:VceAddressHi
    jsr CardBlockStub
ex_colorcmd_at6:
    stz a:CardColorCommand
    rts

