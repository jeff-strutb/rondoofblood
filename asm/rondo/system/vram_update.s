; @routine 0255:000 CopyTileBlockToVram
; @run 6F:A000
; WHAT IT DOES: Copies one 4 by 4 tile block's sixteen words to the VRAM, four to a row, from the
; block graphics, starting at the VRAM address in zTemp10 and going down a row (64 words) after each four.
; HOW: Maps the block bank into MPR6, makes the source pointer from the block number (32 bytes a
; block from $C000), and for each row selects the VRAM write address, writes the address, selects
; the data port and writes four words.
; WAITS: no.
; Entry: A the block number, carry the add into the bank (ADC #0 first), zTemp10 the VRAM address.
; Exit: A $10, Z set, C set, N clear, Y $20, V from the last high byte add; X kept.
; Names: zp $13, $17 (the pointer's high byte and the count) are by address until the records name them.
CopyTileBlockToVram:
    pha
    lda #$0B
    adc #$00
    clc
    adc z:zRamBaseBank
    tam #$40
    pla
    stz z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    clc
    adc #$00
    sta z:zTemp12
    lda z:zProduct0
    adc #$C0
    sta z:zProduct0
    jsr VecClearVdcControlHigh
    stz z:zMultiplierHi
    cly
CopyTileBlockToVram_at0:
    jsr VecSelectVramWriteAddress
    lda z:zTemp10
    sta a:VDC_DATA_LO
    lda z:zTemp11
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
CopyTileBlockToVram_at1:
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_HI
    iny
    inc z:zMultiplierHi
    lda z:zMultiplierHi
    and #$03
    bne CopyTileBlockToVram_at1
    lda #$40
    clc
    adc z:zTemp10
    sta z:zTemp10
    lda #$00
    adc z:zTemp11
    and #$0F
    sta z:zTemp11
    lda z:zMultiplierHi
    cmp #$10
    bne CopyTileBlockToVram_at0
    rts

; @routine 0256:00F SelectBlockTablesA
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $AAAC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesA:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$AC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$AA
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesA_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesA_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 0516:00F SelectBlockTablesB
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $AFEC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesB:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$EC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$AF
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesB_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesB_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 05C6:00F SelectBlockTablesC
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $AAEC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesC:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$EC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$AA
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesC_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesC_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 0726:00F SelectBlockTablesD
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $AB2C and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesD:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$2C
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$AB
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesD_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesD_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 07D6:00F SelectBlockTablesE
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B3AC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesE:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$AC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B3
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesE_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesE_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 0936:00F SelectBlockTablesF
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B4AC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesF:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$AC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B4
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesF_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesF_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 0A96:00F SelectBlockTablesG
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B5AC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesG:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$AC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B5
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesG_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesG_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 0BF6:00F SelectBlockTablesH
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B02C and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesH:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$2C
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B0
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesH_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesH_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 0D56:00F SelectBlockTablesI
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B6AC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesI:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$AC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B6
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesI_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesI_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 0EB6:00F SelectBlockTablesJ
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B4EC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesJ:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$EC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B4
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesJ_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesJ_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 1016:00F SelectBlockTablesK
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B2EC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesK:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$EC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B2
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesK_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesK_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 10C6:39F SelectBlockTablesL
; @run 6F:AB9F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B03C and $AD7C.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesL:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$3C
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B0
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesL_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesL_at0
    clc
    adc #$7C
    sta z:zTemp04
    lda z:zTemp05
    adc #$AD
    sta z:zTemp05
    rts

; @routine 1176:00F SelectBlockTablesM
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B5EC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesM:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$EC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B5
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesM_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesM_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 12D6:398 SelectBlockTablesN
; @run 6F:AB98
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $AEF5 and $AD75.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesN:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$F5
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$AE
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesN_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesN_at0
    clc
    adc #$75
    sta z:zTemp04
    lda z:zTemp05
    adc #$AD
    sta z:zTemp05
    rts

; @routine 1616:00F SelectBlockTablesO
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $B1EC and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesO:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$EC
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$B1
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesO_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesO_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 18D6:00F SelectBlockTablesP
; @run 6F:A80F
; WHAT IT DOES: Selects the tile block's two tables for the block index in A: zTemp0C the first
; table's entry, zTemp04 the second's, for the stage whose overlay this is.
; HOW: select_block_tables, with this stage's tables at $AC6C and $A9EC.
; WAITS: no.
; Entry: A the block index. Exit: A the second pointer's high byte, N Z C V from its add, Y 0, X kept.
; Clobbers zTemp0C, zTemp0D, zTemp04 and zTemp05.
SelectBlockTablesP:
    pha
    stz z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    asl a
    rol z:zTemp0D
    clc
    adc #$6C
    sta z:zTemp0C
    lda z:zTemp0D
    adc #$AC
    sta z:zTemp0D
    pla
    stz z:zTemp05
    ldy #$06
SelectBlockTablesP_at0:
    asl a
    rol z:zTemp05
    dey
    bne SelectBlockTablesP_at0
    clc
    adc #$EC
    sta z:zTemp04
    lda z:zTemp05
    adc #$A9
    sta z:zTemp05
    rts

; @routine 00C5:151 VramListFromIndex
; @run 6C:8151
; WHAT IT DOES: Runs the VRAM list whose number is in A: the overlay's table of lists gives its pointer.
; HOW: An entry into the list loop (vram_list): A doubled indexes the table at $9802, the pointer goes
; to zp $14, Y 0, and the loop runs on at VramListRun. A number of $80 or more branches to $8142
; (untraced), which reads a table at $8360 the same way.
; WAITS: no.
; Entry: A the list number. Exit: as VramListRun.
; Untraced: the branch to $8142 (the bytes of $8142 to $8150 are not in this routine).
VramListFromIndex:
    bmi *-15
    asl a
    tay
    lda a:$9802
    sta z:$F8
    lda a:$9803
    sta z:$F9
    lda ($F8),y
    sta z:$14
    iny
    lda ($F8),y
    sta z:$15
    cly

; @routine 00C5:169 VramListRun
; @run 6C:8169
; WHAT IT DOES: Runs the VRAM list at zp $14, Y its place: each entry maps its tile bank, sets the VRAM
; write address, and copies words or runs a stream of commands, until the list's $FF.
; HOW: The list loop, vram_list. The bank byte $28 to $2F is lowered by $18 unless the idle flag's bit 0
; is set, and a zero one becomes $31 when it is (untraced).
; WAITS: no.
; Entry: zp $14 the list, Y the place in it. Exit: A $FF, C and Z set, N clear, X and Y and V as the last
; entry leaves them.
; Untraced: the bank $31 replacement.
VramListRun:
    lda ($14),y
    cmp #$FF
    bne VramListRun_at0
    rts
VramListRun_at0:
    cmp #$00
    bne VramListRun_at1
    bbr0 zIdleFlag,VramListRun_at1
    lda #$31
VramListRun_at1:
    cmp #$28
    bcc VramListRun_at2
    cmp #$30
    bcs VramListRun_at2
    bbs0 zIdleFlag,VramListRun_at2
    sec
    sbc #$18
VramListRun_at2:
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    iny
    lda ($14),y
    sta z:zTemp11
    iny
    jsr VecClearVdcControlHigh
    jsr VecSelectVramWriteAddress
    lda ($14),y
    iny
    sta z:zMultiplierLo
    sta a:VDC_DATA_LO
    lda ($14),y
    iny
    sta z:zMultiplierHi
    sta a:VDC_DATA_HI
    lda ($14),y
    sta z:zTemp12
    iny
    lda ($14),y
    sta z:zProduct0
    iny
    jsr VecSelectVramData
    lda z:zTemp11
    bmi VramListRun_at3
    phy
    bsr VramCommandsFetch
    ply
    bra VramListRun
VramListRun_at3:
    lda ($14),y
    iny
    tax
    phy
    jsr CopyWordsToVdc
    ply
    bra VramListRun

; @routine 00C5:1CD VramCommandsFetch
; @run 6C:81CD
; WHAT IT DOES: Runs one stream of VRAM commands from the pointer in zp $12: counted words, a word repeated, pairs
; of a byte and a fixed byte, a new write address (absolute, or added to the last), until $FF.
; HOW: An entry into the command loop (vram_commands), at its first fetch. The commands that run on from the
; overlay's other entries (VramCommandsRepeat and its two inside) are cases of the same loop.
; WAITS: no.
; Entry: zp $12 the stream. Exit: A $FF, C and Z set, N clear, Y the index of the $FF, V from the last add.
; Untraced: the command $FD (set the write address).
VramCommandsFetch:
    cly
    lda (zTemp12),y
    bne VramCommandsFetch_at0
    jmp a:VramCommandsRepeat_pairs
VramCommandsFetch_at0:
    bmi VramCommandsFetch_at1
    jmp a:VramCommandsRepeat
VramCommandsFetch_at1:
    cmp #$FF
    bne VramCommandsFetch_at2
    rts
VramCommandsFetch_at2:
    cmp #$FE
    bne VramCommandsFetch_at3
    jmp a:VramCommandsRepeat_address_add
VramCommandsFetch_at3:
    cmp #$FD
    bne VramCommandsFetch_at4
    jmp a:VramCommandsRepeat_address
VramCommandsFetch_at4:
    and #$7F
    sta z:zTemp08
VramCommandsFetch_at5:
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_HI
    dec z:zTemp08
    bne VramCommandsFetch_at5

; @routine 00C5:201 VramCommandsNext
; @run 6C:8201
; WHAT IT DOES: Goes on to the next command of the stream: the pointer moves past this one's bytes.
; HOW: An entry into the command loop (vram_commands), at its advance.
; WAITS: no.
; Entry: Y the index of the command's last byte, zp $12 the stream. Exit: as VramCommandsFetch.
VramCommandsNext:
    iny
    tya
    clc
    adc z:zTemp12
    sta z:zTemp12
    bcc VramCommandsNext_at0
    inc z:zProduct0
VramCommandsNext_at0:
    bra VramCommandsFetch

; @routine 00C5:20E VramCommandsRepeat
; @run 6C:820E
; WHAT IT DOES: The commands from $820E on: a counted word written again and again, and the stream's other
; forms (pairs, the address commands).
; HOW: An entry into the command loop (vram_commands), at the repeat command with A its count; the pairs
; command at $822B, the address-add at $8247 and the address set at $8266 are its cases.
; WAITS: no.
; Entry: A the count, Y the index of the command, zp $12 the stream. Exit: as VramCommandsFetch.
; Untraced: the $8266 command.
VramCommandsRepeat:
    iny
    sta z:zTemp08
    lda (zTemp12),y
    sta z:zTemp09
    iny
    lda (zTemp12),y
    sta z:zTemp0A
VramCommandsRepeat_at0:
    lda z:zTemp09
    sta a:VDC_DATA_LO
    lda z:zTemp0A
    sta a:VDC_DATA_HI
    dec z:zTemp08
    bne VramCommandsRepeat_at0
    jmp a:VramCommandsNext
VramCommandsRepeat_pairs:
    iny
    lda (zTemp12),y
    sta z:zTemp08
    iny
    lda (zTemp12),y
    sta z:zTemp09
VramCommandsRepeat_at1:
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    lda z:zTemp09
    sta a:VDC_DATA_HI
    dec z:zTemp08
    bne VramCommandsRepeat_at1
    jmp a:VramCommandsNext
VramCommandsRepeat_address_add:
    iny
    jsr VecSelectVramWriteAddress
    lda (zTemp12),y
    clc
    adc z:zMultiplierLo
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    adc z:zMultiplierHi
    bbs0 zTemp11,VramCommandsRepeat_at2
    and #$0F
VramCommandsRepeat_at2:
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    jmp a:VramCommandsNext
VramCommandsRepeat_address:
    iny
    jsr VecSelectVramWriteAddress
    lda (zTemp12),y
    sta z:zMultiplierLo
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta z:zMultiplierHi
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    jmp a:VramCommandsNext

; @routine 0251:4BF LoadPackedPalette
; @run 6C:84BF
; WHAT IT DOES: Loads a packed palette into the VCE from a list: the colour address, a pointer to the blocks, then the
; colours, eight a block with their ninth bits in the block's first byte.
; HOW: load_packed_palette.
; WAITS: no.
; Entry: zp $00 the list, Y its place in it. Exit: A the last block pointer's low byte, Y 9, N and Z from the block
; count's last value, C and V from the pointer's add. X kept.
LoadPackedPalette:
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda (zTemp02)
    sta z:zTemp05
LoadPackedPalette_at0:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
LoadPackedPalette_at1:
    iny
    lda (zTemp02),y
    sta a:VCE_DATA_LO
    cla
    asl z:zTemp04
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc LoadPackedPalette_at1
    clc
    lda z:zTemp02
    adc #$09
    sta z:zTemp02
    bcc LoadPackedPalette_at2
    inc z:zTemp03
LoadPackedPalette_at2:
    dec z:zTemp05
    bpl LoadPackedPalette_at0
    rts

; @routine 00C5:580 LoadPackedPaletteB
; @run 6C:8580
; WHAT IT DOES: LoadPackedPalette, as the 00C5 overlay has it at $8580.
; HOW: The same bytes at another run address; load_packed_palette.
; WAITS: no.
; Entry, exit: as LoadPackedPalette.
LoadPackedPaletteB:
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda (zTemp02)
    sta z:zTemp05
LoadPackedPaletteB_at0:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
LoadPackedPaletteB_at1:
    iny
    lda (zTemp02),y
    sta a:VCE_DATA_LO
    cla
    asl z:zTemp04
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc LoadPackedPaletteB_at1
    clc
    lda z:zTemp02
    adc #$09
    sta z:zTemp02
    bcc LoadPackedPaletteB_at2
    inc z:zTemp03
LoadPackedPaletteB_at2:
    dec z:zTemp05
    bpl LoadPackedPaletteB_at0
    rts

; @routine 0111:4CE LoadPackedPaletteC
; @run 6C:84CE
; WHAT IT DOES: LoadPackedPalette, as the 0111 overlay has it at $84CE.
; HOW: The same bytes at another run address; load_packed_palette.
; WAITS: no.
; Entry, exit: as LoadPackedPalette.
LoadPackedPaletteC:
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda (zTemp02)
    sta z:zTemp05
LoadPackedPaletteC_at0:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
LoadPackedPaletteC_at1:
    iny
    lda (zTemp02),y
    sta a:VCE_DATA_LO
    cla
    asl z:zTemp04
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc LoadPackedPaletteC_at1
    clc
    lda z:zTemp02
    adc #$09
    sta z:zTemp02
    bcc LoadPackedPaletteC_at2
    inc z:zTemp03
LoadPackedPaletteC_at2:
    dec z:zTemp05
    bpl LoadPackedPaletteC_at0
    rts
