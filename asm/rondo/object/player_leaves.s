; @routine 01BC:7B4 RichterStateDoubled
; @run 6D:BFB4
; WHAT IT DOES: Twice the state of the object Richter's code is running, for a table of words.
; HOW: Reads zRichterObject, the object's state, shifts it left.
; WAITS: no.
; Entry: nothing. Exit: A and X the doubled state, N and Z from it, C the state's bit 7. Y, V kept.
RichterStateDoubled:
    ldx z:zRichterObject
    lda a:OBJ_27A8,x
    asl a
    tax
    rts

; @routine 01BC:7BC SetVelocityYFromTable
; @run 6D:BFBC
; WHAT IT DOES: Gives the object a vertical velocity from a table of words, entry A.
; HOW: Y = A doubled; the low byte from $BFCB+Y into OBJ_VY_LO, the high from $BFCC+Y into OBJ_VY_HI.
; WAITS: no.
; Entry: A the entry, X the object. Exit: A the high byte, N and Z from it, C the entry's bit 7, Y the doubled entry.
SetVelocityYFromTable:
    asl a
    tay
    lda a:$BFCB,y
    sta a:OBJ_VY_LO,x
    lda a:$BFCC,y
    sta a:OBJ_VY_HI,x
    rts

; @routine 01BC:7DD SetVelocityXFromTable
; @run 6D:BFDD
; WHAT IT DOES: Gives the object a horizontal velocity from a table of words, entry A.
; HOW: Y = A doubled; the low byte from $BFEC+Y into OBJ_VX_LO, the high from $BFED+Y into OBJ_VX_HI.
; WAITS: no.
; Entry and exit: as SetVelocityYFromTable.
SetVelocityXFromTable:
    asl a
    tay
    lda a:$BFEC,y
    sta a:OBJ_VX_LO,x
    lda a:$BFED,y
    sta a:OBJ_VX_HI,x
    rts

; @routine 01BD:01B RichterFlagsAgainstFour
; @run 6E:C01B
; WHAT IT DOES: Compares the player object's flag byte with 4, for the caller's branch.
; HOW: Loads zRichterObject into X (unused here), the flags of object 0, CMP #4.
; WAITS: no.
; Entry: nothing. Exit: X zRichterObject, A the flags, N, Z, C from A less 4. Y, V kept.
RichterFlagsAgainstFour:
    ldx z:zRichterObject
    lda a:OBJ_FLAGS_2F38
    cmp #$04
    rts

; @routine 01C0:063 NegateVelocityXIfFacingLeft
; @run 6E:D863
; WHAT IT DOES: Turns the object's horizontal velocity round when its facing byte is set.
; HOW: Reads the facing byte; when it is not zero, goes on to NegateVelocityX.
; WAITS: no.
; Entry: X the object. Exit: facing zero: A zero, Z. Otherwise as NegateVelocityX.
NegateVelocityXIfFacingLeft:
    lda a:OBJ_2AC0,x
    bne NegateVelocityXIfFacingLeft_at0
    rts
NegateVelocityXIfFacingLeft_at0:
    jmp VecNegateVelocityX

; @routine 024D:0A0 MapOverlayBanks5And6
; @run 6B:80A0
; WHAT IT DOES: Maps the banks 5 and 6 above zRamBaseBank into MPR5 and MPR6.
; HOW: ADC of the base bank to 5, TAM #$20, INC A, TAM #$40.
; WAITS: no.
; Entry: nothing. Exit: A the second bank, N and Z from it, C and V from the add; MPR5 and MPR6 set.
MapOverlayBanks5And6:
    lda #$05
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    rts

; @routine 024D:0B7 MapOverlayBank7
; @run 6B:80B7
; WHAT IT DOES: Maps the bank 7 above zRamBaseBank into MPR5.
; HOW: ADC of the base bank to 7, TAM #$20.
; WAITS: no.
; Entry: nothing. Exit: A the bank, N, Z, C, V from the add; MPR5 set. X, Y kept.
MapOverlayBank7:
    lda #$07
    clc
    adc z:zRamBaseBank
    tam #$20
    rts

; @routine 024D:23B EmptyHandler823B
; @run 6B:823B
; WHAT IT DOES: Nothing: the handler an overlay's table names where an object has nothing to do.
; HOW: Returns at once.
; WAITS: no.
; Entry: any. Exit: all registers and flags kept.
EmptyHandler823B:
    rts

; @routine 050D:66A StepStageTable9C20
; @run 6B:866A
; WHAT IT DOES: Steps zField82 and sets zField83 from the byte of the table at $9C20 it indexes.
; HOW: INC zField82, read it again as the index, one table read, one store.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. C, V kept.
StepStageTable9C20:
    inc z:zField82
    lda z:zField82
    tay
    lda a:$9C20,y
    sta z:zField83
    rts

; @routine 0720:4FC EmptyHandler9CFC
; @run 6B:9CFC
; WHAT IT DOES: Nothing: the handler this overlay's table names where an object has nothing to do.
; HOW: Returns at once.
; WAITS: no.
; Entry: any. Exit: all registers and flags kept.
EmptyHandler9CFC:
    rts

; @routine 10C0:482 TileNudgeAt9C82
; @run 6B:9C82
; WHAT IT DOES: A tile's handler: the point's row one lower.
; HOW: Increments zTemp02; the handled answer $81 in A.
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81, N set, Z clear; C, V and Y kept.
TileNudgeAt9C82:
    inc z:zTemp02
    lda #$81
    rts

; @routine 10C0:487 TileLift7At9C87
; @run 6B:9C87
; WHAT IT DOES: A tile's handler: the point's row seven higher.
; HOW: A subtract from zTemp02; the handled answer $81 in A.
; WAITS: no.
; Entry: zTemp02 row. Exit: A $81; C and V from the subtract; Y kept.
TileLift7At9C87:
    lda z:zTemp02
    sec
    sbc #$07
    sta z:zTemp02
    lda #$81
    rts

; @routine 10C0:491 TileLift6At9C91
; @run 6B:9C91
; WHAT IT DOES: A tile's handler: the point's row six higher.
; HOW: A subtract from zTemp02; the handled answer $81 in A.
; WAITS: no.
; Entry and exit: as TileLift7At9C87.
TileLift6At9C91:
    lda z:zTemp02
    sec
    sbc #$06
    sta z:zTemp02
    lda #$81
    rts

; @routine 10C0:49B TileLift5At9C9B
; @run 6B:9C9B
; WHAT IT DOES: A tile's handler: the point's row five higher.
; HOW: A subtract from zTemp02; the handled answer $81 in A.
; WAITS: no.
; Entry and exit: as TileLift7At9C87.
TileLift5At9C9B:
    lda z:zTemp02
    sec
    sbc #$05
    sta z:zTemp02
    lda #$81
    rts

; @routine 10C0:4A5 TileLift4At9CA5
; @run 6B:9CA5
; WHAT IT DOES: A tile's handler: the point's row four higher.
; HOW: A subtract from zTemp02; the handled answer $81 in A.
; WAITS: no.
; Entry and exit: as TileLift7At9C87.
TileLift4At9CA5:
    lda z:zTemp02
    sec
    sbc #$04
    sta z:zTemp02
    lda #$81
    rts

; @routine 10C0:4AF TileLift3At9CAF
; @run 6B:9CAF
; WHAT IT DOES: A tile's handler: the point's row three higher.
; HOW: A subtract from zTemp02; the handled answer $81 in A.
; WAITS: no.
; Entry and exit: as TileLift7At9C87.
TileLift3At9CAF:
    lda z:zTemp02
    sec
    sbc #$03
    sta z:zTemp02
    lda #$81
    rts

; @routine 10C0:4B9 TileLift2At9CB9
; @run 6B:9CB9
; WHAT IT DOES: A tile's handler: the point's row two higher.
; HOW: A subtract from zTemp02; the handled answer $81 in A.
; WAITS: no.
; Entry and exit: as TileLift7At9C87.
TileLift2At9CB9:
    lda z:zTemp02
    sec
    sbc #$02
    sta z:zTemp02
    lda #$81
    rts

; @routine 10C0:4C3 TileLift1At9CC3
; @run 6B:9CC3
; WHAT IT DOES: A tile's handler: the point's row one higher.
; HOW: A subtract from zTemp02; the handled answer $81 in A.
; WAITS: no.
; Entry and exit: as TileLift7At9C87.
TileLift1At9CC3:
    lda z:zTemp02
    sec
    sbc #$01
    sta z:zTemp02
    lda #$81
    rts
