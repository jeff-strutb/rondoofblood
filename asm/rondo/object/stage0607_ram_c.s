; @routine 0A9C:31D FillSevenWithThreeThenClear0A9C
; @run 70:BB1D
; WHAT IT DOES: Stores 3 into the seven bytes from $7E1A and clears the object (Stage 06's overlay). Its RAM
; operand ($7E1A + Y, or $7E70 + Y) has no name: no reached code reads those bytes, so it stays a raw operand and
; the routine counts for M1 only until code that reads them settles a name.
; HOW: A loop of seven stores by Y; a tail JMP to ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FillSevenWithThreeThenClear0A9C:
    lda #$03
    cly
FillSevenWithThreeThenClear0A9C_at0:
    sta a:$7E1A,y
    iny
    cpy #$07
    bne FillSevenWithThreeThenClear0A9C_at0
    jmp VecClearObject

; @routine 0BFF:1C3 FillEightWithArgThenClear0BFF
; @run 71:D1C3
; WHAT IT DOES: Stores the object's OBJ_START_ARG into the eight bytes from $7E70, records its event and clears it
; (Stage 07's overlay). Its RAM operand ($7E1A + Y, or $7E70 + Y) has no name: no reached code reads those bytes,
; so it stays a raw operand and the routine counts for M1 only until code that reads them settles a name.
; HOW: A loop of eight stores by Y down to 0; RecordObjectEvent; a tail JMP to ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FillEightWithArgThenClear0BFF:
    lda a:OBJ_START_ARG,x
    ldy #$07
FillEightWithArgThenClear0BFF_at0:
    sta a:$7E70,y
    dey
    bpl FillEightWithArgThenClear0BFF_at0
    jsr VecRecordObjectEvent
    jmp VecClearObject

; @routine 0BFF:3DA TileGateBySaveByte0BFF
; @run 71:D3DA
; WHAT IT DOES: Waits on CheckTileAndPartner0BFF's carry by state: state 0 loads the record (sprite set 6, frame 1)
; and, once it reports C, sets frame and state 3; state 1, once it reports C, steps the state on; state 3 and on,
; once it reports C, does nothing more; each of these then clears SaveBuffer+$4C and draws tile script $0C. State 2
; plays the frame list at $D44A and steps the state on when it ends (Stage 07's overlay).
; HOW: A chain of DEC A tests of the state; CheckTileAndPartner0BFF's carry; DrawTileScript as a tail;
; StepObjectFrames.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TileGateBySaveByte0BFF:
    lda a:OBJ_STATE,x
    bne TileGateBySaveByte0BFF_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$01
    sta a:OBJ_FRAME,x
    jsr CheckTileAndPartner0BFF
    bcc TileGateBySaveByte0BFF_at1
    lda #$03
    sta a:OBJ_FRAME,x
    sta a:OBJ_STATE,x
    bra TileGateBySaveByte0BFF_at2
TileGateBySaveByte0BFF_at0:
    dec a
    bne TileGateBySaveByte0BFF_at3
    jsr CheckTileAndPartner0BFF
    bcc TileGateBySaveByte0BFF_at1
    inc a:OBJ_STATE,x
TileGateBySaveByte0BFF_at2:
    stz a:SaveBuffer+76
    cly
    lda #$0C
    jmp VecDrawTileScript
TileGateBySaveByte0BFF_at3:
    dec a
    bne TileGateBySaveByte0BFF_at4
    lda #$D4
    sta z:zTemp11
    lda #$4A
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne TileGateBySaveByte0BFF_at1
    inc a:OBJ_STATE,x
TileGateBySaveByte0BFF_at1:
    rts
TileGateBySaveByte0BFF_at4:
    jsr CheckTileAndPartner0BFF
    bcc TileGateBySaveByte0BFF_at1
    bra TileGateBySaveByte0BFF_at2
