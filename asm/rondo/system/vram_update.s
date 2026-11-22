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

