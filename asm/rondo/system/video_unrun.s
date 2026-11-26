; @routine 0046:3CD WriteVramList
; @run 69:5BCD
; WHAT IT DOES: Writes a list of VRAM words: a write address, then words whose low byte is a
; list byte and whose high byte is held; with carry set at entry every word is the blank tile
; ($0110) instead. No JSR, JMP or table on the disc names it: the census found it after the
; return of the routine before it, and no input reaches it, so its script runs it by unit calls.
; HOW: Keeps the carry in bit 0 of zTemp02 (tested again for every word). Clears the control
; register's high byte, then from the list at the pointer in zTemp00: two bytes of VRAM write
; address, then bytes, each one word; $FF ends the list, $FE is followed by a new address, $FD
; by a new high byte.
; Entry: zTemp00-01 the list, X the first high byte, C erase. The list is indexed by Y from 0,
; so it is at most 256 bytes.
; Exit: A $FF, N 0, Z 1, C 1, V kept; X the high byte last held, Y the index past the end.
; WAITS: no.
WriteVramList:
    stz z:zTemp02
    rol z:zTemp02
    jsr ClearVdcControlHigh
    cly
WriteVramList_address:
    jsr SelectVramWriteAddress
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    iny
    jsr SelectVramData
WriteVramList_next:
    lda (zTemp00),y
    iny
    cmp #$FF
    beq WriteVramList_end
    cmp #$FE
    beq WriteVramList_address
    cmp #$FD
    beq WriteVramList_high
    bbr0 zTemp02,WriteVramList_put
    lda #$10
    ldx #$01
WriteVramList_put:
    sta a:VDC_DATA_LO
    stx a:VDC_DATA_HI
    bra WriteVramList_next
WriteVramList_high:
    lda (zTemp00),y
    iny
    tax
    bra WriteVramList_next
WriteVramList_end:
    rts
