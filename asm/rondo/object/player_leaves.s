; @routine 01BB:0A0 RichterTurnOnPad
; @run 6D:B0A0
; WHAT IT DOES: Turns the player round when the pad says so: facing right and the pad's bit 7 held turns him left,
; facing left and bit 5 held turns him right; the state then takes the next one from a table.
; HOW: OBJ_FACING_LEFT is the facing; a TST of zPadHeld; the state less 5 indexes the table at $B03B.
; WAITS: no.
; Entry: nothing. Exit: no turn: A the facing, N, V, Z from the TST. A turn: A the new state, N and Z from it, Y the
; old state less 5, C and V from that subtract.
RichterTurnOnPad:
    lda a:OBJ_FACING_LEFT
    bne RichterTurnOnPad_at0
    tst #$80,z:zPadHeld
    beq RichterTurnOnPad_at1
    inc a:OBJ_FACING_LEFT
RichterTurnOnPad_at2:
    lda a:OBJ_STATE
    sec
    sbc #$05
    tay
    lda a:$B03B,y
    sta a:OBJ_STATE
    rts
RichterTurnOnPad_at0:
    tst #$20,z:zPadHeld
    beq RichterTurnOnPad_at1
    stz a:OBJ_FACING_LEFT
    bra RichterTurnOnPad_at2
RichterTurnOnPad_at1:
    rts

; @routine 01BB:1AA CountDownThenStand
; @run 6D:B1AA
; WHAT IT DOES: Counts the player's timer in OBJ_HURT_TIMER down; when it reaches zero he stands.
; HOW: DEC of the byte; zero goes on to RichterStand, any other value returns.
; WAITS: no.
; Entry: nothing. Exit: counting on, N and Z from the new value; at zero as RichterStand.
; Checked with --callees wrappers: RichterStand's earlier C differs from its draft in the call frames of its own calls.
CountDownThenStand:
    dec a:OBJ_HURT_TIMER
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
    lda a:OBJ_STATE,x
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
    lda a:OBJ_SCROLL_BAND
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

; @routine 01BE:567 MoveObjectUpBySigned
; @run 6E:CD67
; WHAT IT DOES: Moves the object up the screen by the signed amount in zTemp02: its Y position less the amount,
; sign extended into the high byte.
; HOW: A 16-bit subtract of zTemp02, read again for the low byte; the high byte takes the borrow, less 1 more when the
; amount is negative ($FF subtracted). The negative arm is untraced (137 of 400 cases reach it, and the draft stops
; there) and is written from the disc's bytes.
; WAITS: no.
; Entry: X object, zTemp02 the amount. Exit: A the Y high byte stored, N, Z, C, V from its subtract. X, Y kept.
MoveObjectUpBySigned:
    lda z:zTemp02
    bmi MoveObjectUpBySigned_at0
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_Y_HI,x
    rts
MoveObjectUpBySigned_at0:
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$FF
    sta a:OBJ_Y_HI,x
    rts

; @routine 01C0:063 NegateVelocityXIfFacingLeft
; @run 6E:D863
; WHAT IT DOES: Turns the object's horizontal velocity round when its facing byte is set.
; HOW: Reads the facing byte; when it is not zero, goes on to NegateVelocityX.
; WAITS: no.
; Entry: X the object. Exit: facing zero: A zero, Z. Otherwise as NegateVelocityX.
NegateVelocityXIfFacingLeft:
    lda a:OBJ_FACING_LEFT,x
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

; @routine 05C0:0A5 HitKindFromBit7
; @run 6B:98A5
; WHAT IT DOES: Gives the hit slot's kind (OBJ_WORK_P) 1 or 2 by bit 7 of the object's OBJ_PACE; kind 2 also sets the
; slot's OBJ_WORK_N to 5; then reports the hit.
; HOW: AND #$80 rotated twice, AND #1, INC, one store, CMP #2; the report as ReportHitToStage. The kind 2 arm is
; untraced (135 of 400 cases reach it and the draft stops there), written from the disc's bytes.
; WAITS: no.
; Entry: X object, Y the slot. Exit: as StoreHitFromTable.
HitKindFromBit7:
    lda a:OBJ_PACE,x
    and #$80
    rol a
    rol a
    and #$01
    inc a
    sta a:OBJ_WORK_P,y
    cmp #$02
    bne ReportHitToStage
    lda #$05
    sta a:OBJ_WORK_N,y
    bra ReportHitToStage

; @routine 0720:076 ReportHitUnlessTwo
; @run 6B:9876
; WHAT IT DOES: Reports the hit to the stage unless the kind just stored in OBJ_WORK_P is 2.
; HOW: CMP #2 on A; equal returns, otherwise goes on to ReportHitToStage.
; WAITS: no.
; Entry: A the kind, X object, Y the slot. Exit: equal: A and N, Z, C from the compare; otherwise as StoreHitFromTable.
ReportHitUnlessTwo:
    cmp #$02
    beq ReportHitUnlessTwo_at0
    lda a:WorkByteA
    sta a:StageHitReport
    lda a:WorkByteB
    sta a:StageHitReport+1
    lda a:HitboxXLo+3
    sta a:StageHitReport+2
    lda a:HitboxYLo+3
    sta a:StageHitReport+3
    lda a:OBJ_WORK_N,y
    sta a:OBJ_HIT_TAKEN,x
ReportHitUnlessTwo_at0:
    rts

; @routine 0720:099 HitKindFromBit1
; @run 6B:9899
; WHAT IT DOES: Gives the hit slot's kind (OBJ_WORK_P) 1 or 2 by bit 1 of the object's OBJ_PACE, then reports the hit.
; HOW: AND #2, LSR, INC, one store, then ReportHitUnlessTwo.
; WAITS: no.
; Entry: X object, Y the slot. Exit: as ReportHitUnlessTwo.
HitKindFromBit1:
    lda a:OBJ_PACE,x
    and #$02
    lsr a
    inc a
    sta a:OBJ_WORK_P,y
    bra ReportHitUnlessTwo

; @routine 0720:4ED LoadRecordThenFrameByKind
; @run 6B:9CED
; WHAT IT DOES: Loads the object's record, then gives it frame 0 or 1 of a two byte table by bit 0 of its next kind.
; HOW: LoadObjectRecord, OBJ_START_ARG and 1, the table at $9CFD; falls into the RTS at $9CFC's neighbour.
; WAITS: no.
; Entry: X object. Exit: A the frame, N and Z from it, Y the index, C from the load of the record. X kept.
LoadRecordThenFrameByKind:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:$9CFD,y
    sta a:OBJ_FRAME,x

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

; @routine 0E00:07A ReportHitToStage
; @run 6B:987A
; WHAT IT DOES: Copies the stream count's two bytes and the hitbox X and Y of slot 3 to the stage's four byte report,
; then gives the object's hit timer its byte.
; HOW: Four read and write pairs, then goes on to StoreHitFromTable.
; WAITS: no.
; Entry: X object, Y the slot. Exit: as StoreHitFromTable.
ReportHitToStage:
    lda a:WorkByteA
    sta a:StageHitReport
    lda a:WorkByteB
    sta a:StageHitReport+1
    lda a:HitboxXLo+3
    sta a:StageHitReport+2
    lda a:HitboxYLo+3
    sta a:StageHitReport+3

; @routine 0E00:092 StoreHitFromTable
; @run 6B:9892
; WHAT IT DOES: Gives the object's hit timer OBJ_HIT_TAKEN the byte of OBJ_WORK_N at the slot in Y.
; HOW: One read, one write.
; WAITS: no.
; Entry: X object, Y the slot. Exit: A the byte, N and Z from it.
StoreHitFromTable:
    lda a:OBJ_WORK_N,y
    sta a:OBJ_HIT_TAKEN,x
    rts

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

; @routine 01E1:4E4 MariaStateDoubled
; @run 6E:C4E4
; WHAT IT DOES: RichterStateDoubled in Maria's player overlay (01DD), the same instructions at her address.
; HOW: RichterStateDoubled's body.
; WAITS: no.
; Entry, exit: as RichterStateDoubled.
MariaStateDoubled:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    asl a
    tax
    rts

; @routine 01E1:28B MariaSetVelocityYFromTable
; @run 6E:C28B
; WHAT IT DOES: SetVelocityYFromTable in Maria's player overlay (01DD).
; HOW: The same instructions with her table of words at $C29A.
; WAITS: no.
; Entry, exit: as SetVelocityYFromTable.
MariaSetVelocityYFromTable:
    asl a
    tay
    lda a:$C29A,y
    sta a:OBJ_VY_LO,x
    lda a:$C29B,y
    sta a:OBJ_VY_HI,x
    rts

; @routine 01E1:2BC MariaSetVelocityXFromTable
; @run 6E:C2BC
; WHAT IT DOES: SetVelocityXFromTable in Maria's player overlay (01DD).
; HOW: The same instructions with her table of words at $C2ED.
; WAITS: no.
; Entry, exit: as SetVelocityXFromTable.
MariaSetVelocityXFromTable:
    asl a
    tay
    lda a:$C2ED,y
    sta a:OBJ_VX_LO,x
    lda a:$C2EE,y
    sta a:OBJ_VX_HI,x
    rts
