; @routine 01BB:0A0 RichterTurnOnPad
; @run 6D:B0A0
; WHAT IT DOES: Turns the player round when the pad says so: facing right and the pad's bit 7 held turns him left,
; facing left and bit 5 held turns him right; the state then takes the next one from a table.
; HOW: OBJ_2AC0 is the facing; a TST of zPadHeld; the state less 5 indexes the table at $B03B.
; WAITS: no.
; Entry: nothing. Exit: no turn: A the facing, N, V, Z from the TST. A turn: A the new state, N and Z from it, Y the
; old state less 5, C and V from that subtract.
RichterTurnOnPad:
    lda a:OBJ_2AC0
    bne RichterTurnOnPad_at0
    tst #$80,z:zPadHeld
    beq RichterTurnOnPad_at1
    inc a:OBJ_2AC0
RichterTurnOnPad_at2:
    lda a:OBJ_27A8
    sec
    sbc #$05
    tay
    lda a:$B03B,y
    sta a:OBJ_27A8
    rts
RichterTurnOnPad_at0:
    tst #$20,z:zPadHeld
    beq RichterTurnOnPad_at1
    stz a:OBJ_2AC0
    bra RichterTurnOnPad_at2
RichterTurnOnPad_at1:
    rts

; @routine 01BB:1AA CountDownThenStand
; @run 6D:B1AA
; WHAT IT DOES: Counts the player's timer in OBJ_2F90 down; when it reaches zero he stands.
; HOW: DEC of the byte; zero goes on to RichterStand, any other value returns.
; WAITS: no.
; Entry: nothing. Exit: counting on, N and Z from the new value; at zero as RichterStand.
; Checked with --callees wrappers: RichterStand's earlier C differs from its draft in the call frames of its own calls.
CountDownThenStand:
    dec a:OBJ_2F90
    beq CountDownThenStand_at0
    rts
CountDownThenStand_at0:
    jmp RichterStand

; @routine 01BB:79E RichterReadAtOffset
; @run 6D:B79E
; WHAT IT DOES: Reads the terrain at a point offset from the player's own X and Y low bytes by a pair from a table.
; HOW: X low plus the table's byte at $B7B1+Y into zTemp00; Y low plus the one at $B7B2+Y becomes the row, its carry
; the Y high's; goes on to RichterReadOnScreen.
; WAITS: no.
; Entry: Y the table's offset. Exit: as RichterReadOnScreen.
; Checked with --callees wrappers; no case reaches the read's own code, so the read made by C is not reached.
RichterReadAtOffset:
    lda a:OBJ_X_LO
    clc
    adc a:$B7B1,y
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc a:$B7B2,y
    jmp RichterReadOnScreen

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

; @routine 01BE:174 ClearObjectIfOffRange
; @run 6E:C974
; WHAT IT DOES: Empties the object when it is off the map above or below, or far to the side.
; HOW: Y high must be zero; X high zero keeps it; otherwise X low from $40 up to $BF empties it.
; WAITS: no.
; Entry: X object. Exit: kept: A the last byte read, N, Z, C from the last compare; emptied: as the load or compare
; before the jump (ClearObject keeps every register).
ClearObjectIfOffRange:
    lda a:OBJ_Y_HI,x
    bne ClearObjectIfOffRange_at0
    lda a:OBJ_X_HI,x
    beq ClearObjectIfOffRange_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcc ClearObjectIfOffRange_at1
    cmp #$C0
    bcc ClearObjectIfOffRange_at0
ClearObjectIfOffRange_at1:
    rts
ClearObjectIfOffRange_at0:
    jmp VecClearObject

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

; @routine 01C0:129 ClearObjectInYAndCopyPlace
; @run 6E:D929
; WHAT IT DOES: Empties the object in Y, then gives it the position of the object in X.
; HOW: ClearObjectInY, then four byte copies: X low, X high, Y low, Y high.
; WAITS: no.
; Entry: X the source object, Y the one to empty. Exit: A the last byte copied, N and Z from it. X, Y, C, V kept.
ClearObjectInYAndCopyPlace:
    jsr VecClearObjectInY
    lda a:OBJ_X_LO,x
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI,x
    sta a:OBJ_X_HI,y
    lda a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_HI,x
    sta a:OBJ_Y_HI,y
    rts

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

; @routine 09EB:559 LoadRecordThenRecordEvent
; @run 70:B559
; WHAT IT DOES: Loads the object's record, then marks its event when it is one the save remembers.
; HOW: LoadObjectRecord, then goes on to RecordObjectEvent.
; WAITS: no.
; Entry: X object. Exit: as RecordObjectEvent.
LoadRecordThenRecordEvent:
    jsr VecLoadObjectRecord
    jmp VecRecordObjectEvent

; @routine 10C0:293 TileHalfStepAAt9A93
; @run 6B:9A93
; WHAT IT DOES: The tiles $83 and up of this overlay: a step of the row that differs between the tile's halves.
; HOW: A pair of bytes in the table at $9AEF by tile, the second for the right half.
; WAITS: no.
; Entry: A tile, zTemp00 column, zTemp02 row. Exit: A $81; Y the index; C and V from the add.
TileHalfStepAAt9A93:
    sec
    sbc #$83
    asl a
    tay
    bbr2 zTemp00,TileHalfStepAAt9A93_at0
    iny
TileHalfStepAAt9A93_at0:
    lda a:$9AEF,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 10C0:2A7 TileHalfStepBAt9AA7
; @run 6B:9AA7
; WHAT IT DOES: The tiles $8C and up of this overlay: as TileHalfStepAAt9A93, the other way round.
; HOW: A pair of bytes in the table at $9AF7 by tile, the second for the left half.
; WAITS: no.
; Entry and exit: as TileHalfStepAAt9A93.
TileHalfStepBAt9AA7:
    sec
    sbc #$8C
    asl a
    tay
    bbs2 zTemp00,TileHalfStepBAt9AA7_at0
    iny
TileHalfStepBAt9AA7_at0:
    lda a:$9AF7,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
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
