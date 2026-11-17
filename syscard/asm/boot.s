; @card 01:CBC3 boot_display
; WHAT IT DOES: Sets the display up at boot: VRAM cleared, the start screen's palettes asked for,
; the font and the title picture loaded, and the background shown.
; HOW: clear_vram; the background and sprite colours both from $C800 in bank 1, four 16-colour
; sets each, and CardColorCommand 2 to have the next frame write them; ex_vsync waits that
; frame; load_font; ex_bgon.
; Entry: bank 1 at $C000. Exit: as load_font: A what MPR4 held, N and Z from it, X 0, Y 8,
; C 0, V 0.
boot_display:
    bsr clear_vram
    lda #$00
    sta a:CardColorBackground
    lda #$C8
    sta a:CardColorBackground+1
    lda #$00
    sta a:CardColorSprites
    lda #$C8
    sta a:CardColorSprites+1
    lda #$04
    sta a:CardColorBackground+2
    sta a:CardColorSprites+2
    lda #$02
    sta a:CardColorCommand
    jsr ex_vsync
    jsr load_font
    jsr ex_bgon
    rts

; @card 01:C922 text_cursor
; WHAT IT DOES: Points VRAM writes at a place on the start screen, by column and row.
; HOW: The row times 32, with the column's low six bits set into it, in CardPointerLo-Hi, to
; the write address register; then the data register selected.
; Entry: X column, Y row. Exit: A 2, N 0, Z 0, C 0, V bit 1 of the row. X, Y kept.
; CardPointerLo-Hi the address.
text_cursor:
    stz z:CardPointerLo
    sty z:CardPointerHi
    lsr z:CardPointerHi
    ror z:CardPointerLo
    lsr z:CardPointerHi
    ror z:CardPointerLo
    lsr z:CardPointerHi
    ror z:CardPointerLo
    txa
    and #$3F
    tsb z:CardPointerLo
    lda #$00
    sta z:CardVdcReg
    sta a:VdcAddress
    lda z:CardPointerLo
    sta a:VdcDataLo
    lda z:CardPointerHi
    sta a:VdcDataHi
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
    rts

; @card 01:C950 print_text
; WHAT IT DOES: Writes a list of texts on the start screen.
; HOW: Each text is a header, the attribute byte less 3 then the column and the row, and its
; characters to an $FF; an $FF in place of a header ends the list. Each header's attribute plus
; 3 goes to CardDh, the high byte written with every character; text_cursor places it. Falls
; into print_text_run for the characters.
; Entry: _AX the list. Exit: A $FF, Y the end mark's index, N 0, Z 1, C 1; X the last text's
; column and V bit 1 of its row, or both kept if the list is empty. CardDh, CardPointerLo-Hi.
print_text:
    cly
print_text_record:
    lda (CardAl),y
    cmp #$FF
    bne print_text_at0
    rts
print_text_at0:
    clc
    adc #$03
    sta z:CardDh
    iny
    lda (CardAl),y
    tax
    iny
    lda (CardAl),y
    phy
    tay
    bsr text_cursor
    ply
    iny

; @card 01:C96A print_text_run
; WHAT IT DOES: Writes characters of a text from print_text's list, then the texts after it.
; HOW: Each byte to the VDC's data port, low byte, and CardDh to its high byte, until an $FF;
; then on at print_text_record with the next header.
; Entry: _AX the list, Y the index of the next character, CardDh, the VDC set to write. Exit: as
; print_text, X and V kept if no header follows.
print_text_run:
    lda (CardAl),y
    iny
    cmp #$FF
    beq print_text_record
    sta a:VdcDataLo
    lda z:CardDh
    sta a:VdcDataHi
    bra print_text_run

; @card 01:C860 start_screen
; WHAT IT DOES: The card's start screen: shows the title and whether the Super System Card's RAM is
; there, blinks PUSH RUN BUTTON! until run or select is pressed, and on run waits for a disc.
; HOW: print_text with the title, then ex_memopen picks one of two RAM messages ($AA, $55 to
; SuperRamKey first for the Super System Card's own). Each pass of the wait shows the prompt for 32
; frames of every 64 and blanks it for the rest, until pad 1's CardPadPressed is run alone or select
; alone. On run: the line cleared, JUST A MOMENT..., then cd_test_unit until it passes. Not ready
; (4) or an error it does not name waits 70 frames and tries again; no disc ($0B) says so, resets
; the CD unit and starts over; cover open ($0D) says so and starts over.
; Entry: bank 1 at $C000. Exit, select: A 1, X 8, Y $14, N 0, Z 0, C 1, V 1. Run: A 0, and the
; rest as cd_test_unit leaves them over X 8, Y $14, V 1 (X 0 if it waited); I clear if it ever
; waited. _AX, CardDh, CardPointerLo-Hi.
start_screen:
    lda #$7B
    sta z:CardAl
    lda #$C9
    sta z:CardAh
    jsr print_text
    jsr ex_memopen
    bcs start_screen_at1
    txa
    bit #$80
    beq start_screen_at0
    lda #$AA
    sta a:SuperRamKey
    lda #$55
    sta a:SuperRamKey
start_screen_at0:
    lda #$0C
    sta z:CardAl
    lda #$CA
    sta z:CardAh
    jsr print_text
    bra start_screen_at2
start_screen_at1:
    lda #$56
    sta z:CardAl
    lda #$CA
    sta z:CardAh
    jsr print_text
start_screen_at2:
    lda a:CardFrameTick
    and #$3F
    cmp #$20
    bcs start_screen_at3
    lda #$84
    sta z:CardAl
    lda #$CA
    sta z:CardAh
    jsr print_text
    bra start_screen_at4
start_screen_at3:
    ldx #$08
    ldy #$12
    jsr text_cursor
    lda #$03
    sta z:CardDh
    cly
    lda #$9C
    sta z:CardAl
    lda #$CA
    sta z:CardAh
    jsr print_text_run
start_screen_at4:
    lda a:CardPadPressed
    cmp #$08
    beq start_screen_at5
    cmp #$04
    bne start_screen_at2
    lda #$01
    rts
start_screen_at5:
    lda #$99
    sta z:CardAl
    lda #$CA
    sta z:CardAh
    bsr print_text
    lda #$B1
    sta z:CardAl
    lda #$CA
    sta z:CardAh
    bsr print_text
start_screen_at6:
    jsr cd_test_unit
    bcs start_screen_at7
    cla
    rts
start_screen_at7:
    cmp #$04
    beq start_screen_at9
    cmp #$0B
    bne start_screen_at8
    lda #$C6
    sta z:CardAl
    lda #$CA
    sta z:CardAh
    bsr print_text
    jsr reset_cd_unit
    jmp start_screen
start_screen_at8:
    cmp #$0D
    bne start_screen_at9
    lda #$DB
    sta z:CardAl
    lda #$CA
    sta z:CardAh
    bsr print_text
    jmp start_screen
start_screen_at9:
    ldx #$46
start_screen_at10:
    phx
    jsr ex_vsync
    plx
    dex
    bne start_screen_at10
    bra start_screen_at6

