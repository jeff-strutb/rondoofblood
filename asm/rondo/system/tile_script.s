; @routine 0047:405 TileOpAttributeHigh
; @run 6A:6405
; WHAT IT DOES: Script ops $C0-$CF: sets the high nibble of the tile attribute, the
; palette.
; HOW: The op's low nibble shifted up, ORed with the attribute's low nibble.
; Entry: A op. Exit: back to the loop, with Y the shifted nibble.
TileOpAttributeHigh:
    asl a
    asl a
    asl a
    asl a
    tay
    lda z:zTemp04
    and #$0F
TileOpAttributeHigh_at0:
    sta z:zTemp04
    tya
    ora z:zTemp04
    sta z:zTemp04
    bra RunTileScript

; @routine 0047:417 TileOpAttributeLow
; @run 6A:6417
; WHAT IT DOES: Script ops $D0-$FD not otherwise taken: sets the low nibble of the
; tile attribute, the high bits of the tile number.
; HOW: The op's low nibble ORed with the attribute's high nibble.
; Entry: A op. Exit: back to the loop, with Y the nibble.
TileOpAttributeLow:
    and #$0F
    tay
    lda z:zTemp04
    and #$F0
    bra TileOpAttributeHigh_at0

; @routine 0047:420 TileScriptReturn
; @run 6A:6420
; WHAT IT DOES: Ends a tile script: op $FF.
; HOW: Returns.
; Entry: nothing. Exit: everything kept.
TileScriptReturn:
    rts

; @routine 0047:421 TileOpAlternateUp
; @run 6A:6421
; WHAT IT DOES: Script ops $40-$5F: a run of tiles alternating up and back, for a
; two-tile pattern.
; HOW: Next byte the first tile; WriteTileAscending, WriteTileDescending in turn.
; Entry: A op. Exit: back to the loop, with Y 0.
TileOpAlternateUp:
    and #$1F
    tay
    lda (zTemp02)
    jsr AdvanceTileScript
TileOpAlternateUp_at0:
    jsr WriteTileAscending
    dey
    beq RunTileScript
    jsr WriteTileDescending
    dey
    bne TileOpAlternateUp_at0
    bra RunTileScript

; @routine 0047:437 TileOpAlternateDown
; @run 6A:6437
; WHAT IT DOES: Script ops $60-$7F: a run of tiles alternating down and back.
; HOW: Next byte the first tile; WriteTileDescending, WriteTileAscending in turn.
; Entry: A op. Exit: back to the loop, with Y 0.
TileOpAlternateDown:
    and #$1F
    tay
    lda (zTemp02)
    jsr AdvanceTileScript
TileOpAlternateDown_at0:
    jsr WriteTileDescending
    dey
    beq RunTileScript
    jsr WriteTileAscending
    dey
    bne TileOpAlternateDown_at0
    bra RunTileScript

; @routine 0047:44D DrawTileScript
; @run 6A:644D
; WHAT IT DOES: Draws one of the stage's tile scripts: a group and an item within it.
; HOW: The overlay's pointer at OverlayTileScripts leads to a table of groups, each
; a table of scripts. Clears the VDC's high control byte, takes the address from
; the script and runs it.
; Entry: A item, Y group, stage overlay mapped, zTemp04 the attribute. Exit: as
; RunTileScript. Clobbers zTemp00-03, zTemp08-09.
DrawTileScript:
    pha
    lda a:OverlayTileScripts
    sta z:zTemp08
    lda a:OverlayTileScripts+1
    sta z:zTemp09
    tya
    asl a
    tay
    lda (zTemp08),y
    sta z:zTemp00
    iny
    lda (zTemp08),y
    sta z:zTemp01
    pla
    asl a
    tay
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    jsr VecClearVdcControlHigh
    jsr SetVramFromScript

; @routine 0047:476 RunTileScript
; @run 6A:6476
; WHAT IT DOES: Runs a tile script until op $FF. Ops: $00 a literal tile word; $01-$1F
; a tile repeated; $20-$3F tiles ascending; $40-$7F alternating; $80-$BF literal
; tiles; $C0-$CF the palette; $E0 jump; $F0 next line; $FE a new VRAM address; the
; rest of $D0-$FD the attribute's low nibble.
; HOW: Reads an op, compares it down through the ranges in the order above from
; the top, and does it.
; Entry: zTemp02-03 the script, VDC on its data port, zTemp04 the attribute. Exit:
; A $FF, N 0, Z 1, C 1, Y as the last op left it, V from the last line step. X
; kept. Clobbers zTemp00-01, zTemp04.
RunTileScript:
    lda (zTemp02)
    jsr AdvanceTileScript
    cmp #$FF
    beq TileScriptReturn
    cmp #$FE
    beq RunTileScript_at0
    cmp #$F0
    beq RunTileScript_at1
    cmp #$E0
    beq RunTileScript_at2
    cmp #$D0
    bcs TileOpAttributeLow
    cmp #$C0
    bcc RunTileScript_at3
    jmp TileOpAttributeHigh
RunTileScript_at3:
    cmp #$00
    beq RunTileScript_at4
    cmp #$20
    bcc RunTileScript_at5
    cmp #$40
    bcc RunTileScript_at6
    cmp #$60
    bcs RunTileScript_at7
    jmp TileOpAlternateUp
RunTileScript_at7:
    cmp #$80
    bcc TileOpAlternateDown
    and #$3F
    tay
RunTileScript_at8:
    lda (zTemp02)
    jsr AdvanceTileScript
    jsr WriteTile
    dey
    bne RunTileScript_at8
    bra RunTileScript
RunTileScript_at2:
    lda (zTemp02)
    tay
    jsr AdvanceTileScript
    lda (zTemp02)
    sty z:zTemp02
    sta z:zTemp03
    bra RunTileScript
RunTileScript_at4:
    lda (zTemp02)
    sta a:VDC_DATA_LO
    jsr AdvanceTileScript
    lda (zTemp02)
    sta a:VDC_DATA_HI
    jsr AdvanceTileScript
    bra RunTileScript
RunTileScript_at0:
    jsr SetVramFromScript
    bra RunTileScript
RunTileScript_at1:
    jsr NextVramLine
    bra RunTileScript
RunTileScript_at5:
    and #$1F
    tay
    lda (zTemp02)
    jsr AdvanceTileScript
RunTileScript_at9:
    jsr WriteTile
    dey
    bne RunTileScript_at9
    jmp RunTileScript
RunTileScript_at6:
    and #$1F
    tay
    lda (zTemp02)
    jsr AdvanceTileScript
RunTileScript_at10:
    jsr WriteTileAscending
    dey
    bne RunTileScript_at10
    jmp RunTileScript

; @routine 0047:509 WriteTileAscending
; @run 6A:6509
; WHAT IT DOES: Writes a tile and gives the next one up.
; HOW: WriteTile, then adds one.
; Entry: A tile. Exit: A plus one, N and Z from it. X, Y, C, V kept.
WriteTileAscending:
    jsr WriteTile
    inc a
    rts

; @routine 0047:50E WriteTileDescending
; @run 6A:650E
; WHAT IT DOES: Writes a tile and gives the next one down.
; HOW: WriteTile, then subtracts one.
; Entry: A tile. Exit: A minus one, N and Z from it. X, Y, C, V kept.
WriteTileDescending:
    jsr WriteTile
    dec a
    rts

; @routine 0047:513 WriteTile
; @run 6A:6513
; WHAT IT DOES: Writes one tile to VRAM.
; HOW: The tile as the low byte, zTemp04 as the attribute byte.
; Entry: A tile, VDC on its data port. Exit: A kept, N and Z from it. X, Y, C, V
; kept.
WriteTile:
    pha
    sta a:VDC_DATA_LO
    lda z:zTemp04
    sta a:VDC_DATA_HI
    pla
    rts

; @routine 0047:51E SetVramFromScript
; @run 6A:651E
; WHAT IT DOES: Takes a VRAM address from the script and readies writes there.
; HOW: Writes it to the write address register, keeps it in zTemp00-01 as the
; line's start, and selects the data port.
; Entry: zTemp02-03 pointer. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
SetVramFromScript:
    jsr VecSelectVramWriteAddress
    lda (zTemp02)
    sta a:VDC_DATA_LO
    sta z:zTemp00
    jsr AdvanceTileScript
    lda (zTemp02)
    sta a:VDC_DATA_HI
    sta z:zTemp01
    jsr AdvanceTileScript
    jmp VecSelectVramData

; @routine 0047:538 NextVramLine
; @run 6A:6538
; WHAT IT DOES: Moves drawing down to the start of the next background line, $40
; tiles on from the line's start.
; HOW: Adds $40 to the start kept in zTemp00-01, writes it to the write address
; register and selects the data port.
; Entry: zTemp00-01 line start. Exit: A 2, N 0, Z 0, C and V from the sum. X and Y
; kept.
NextVramLine:
    jsr VecSelectVramWriteAddress
    lda z:zTemp00
    clc
    adc #$40
    sta z:zTemp00
    sta a:VDC_DATA_LO
    cla
    adc z:zTemp01
    sta z:zTemp01
    sta a:VDC_DATA_HI
    jmp VecSelectVramData

; @routine 0047:550 AdvanceTileScript
; @run 6A:6550
; WHAT IT DOES: Moves the script pointer on one byte.
; HOW: A 16-bit increment in zero page.
; Entry: zTemp02-03 pointer. Exit: N and Z from the last byte stepped. A, X, Y, C,
; V kept.
AdvanceTileScript:
    inc z:zTemp02
    bne AdvanceTileScript_at0
    inc z:zTemp03
AdvanceTileScript_at0:
    rts
