; @routine 024F:3F8 HitTestAllObjects
; @run 6B:93F8
; WHAT IT DOES: One frame of hit testing: places the first sixteen hitboxes and tests the
; objects from $57 down to $10, clearing the hit-taken byte of each whose timer is running and
; counting that timer down.
; HOW: ComputeHitboxPositions, then HitTestObject for each object.
; Eleven stage overlays hold this code, the ones whose hit test is at $9466.
; WAITS: no.
; Entry: X kept on the stack. Exit: as the last object left it; X kept, C 0.
HitTestAllObjects:
    phx
    jsr ComputeHitboxPositions
    ldx #$57
HitTestAllObjects_at1:
    jsr HitTestObject
    lda a:OBJ_INVULN_TIMER,x
    beq HitTestAllObjects_at0
    stz a:OBJ_HIT_TAKEN,x
    dec a:OBJ_INVULN_TIMER,x
HitTestAllObjects_at0:
    dex
    cpx #$10
    bcs HitTestAllObjects_at1
    plx
    rts

; @routine 024F:466 HitTestObject
; @run 6B:9466
; @stop 9525
; WHAT IT DOES: Tests the object X's hitbox against the hitboxes of the other side's sixteen
; slots (an object with its high bit set in hitbox_on tests the first, the rest the last)
; and, on the first overlap, runs the handler its kind names.
; HOW: The hitbox is placed by its offsets, mirrored when the object faces left; each live slot
; not already hit is compared by the sum of the half sizes on both axes.
; Eleven stage overlays hold this code, the ones whose handlers are at $9543 and $9568.
; WAITS: no.
; Entry: X the object. Exit: with no hit A the last value, Y $FF, N set; with one, as the handler.
HitTestObject:
    stz a:OBJ_HIT_TAKEN,x
    lda a:OBJ_HITBOX_ON,x
    bne HitTestObject_at0
    jmp HitTestObject_at1
HitTestObject_at0:
    lda a:OBJ_HITBOX_HALF_W,x
    ora a:OBJ_HITBOX_HALF_H,x
    bne HitTestObject_at2
    jmp HitTestObject_at1
HitTestObject_at2:
    lda a:OBJ_HITBOX_DX,x
    sta z:zTemp01
    lda a:OBJ_FACING_LEFT,x
    beq HitTestObject_at3
    lda z:zTemp01
    eor #$FF
    inc a
    sta z:zTemp01
HitTestObject_at3:
    stz z:zTemp00
    lda z:zTemp01
    bpl HitTestObject_at4
    dec z:zTemp00
HitTestObject_at4:
    clc
    adc a:OBJ_X_LO,x
    sta a:WorkByteA
    lda a:OBJ_X_HI,x
    adc z:zTemp00
    bne HitTestObject_at1
    stz z:zTemp00
    lda a:OBJ_HITBOX_DY,x
    bpl HitTestObject_at5
    dec z:zTemp00
HitTestObject_at5:
    clc
    adc a:OBJ_Y_LO,x
    sta a:WorkByteB
    lda a:OBJ_Y_HI,x
    adc z:zTemp00
    bne HitTestObject_at1
    lda a:OBJ_HITBOX_ON,x
    bpl HitTestObject_at6
    cly
    bra HitTestObject_at7
HitTestObject_at6:
    ldy #$0F
HitTestObject_at7:
    lda a:OBJ_HITBOX_ON,y
    beq HitTestObject_at8
    lda a:HitboxHi,y
    bne HitTestObject_at8
    lda a:OBJ_HITBOX_HALF_W,x
    clc
    adc a:OBJ_HITBOX_HALF_W,y
    sta a:StreamStepLo
    lda a:OBJ_HITBOX_HALF_H,x
    clc
    adc a:OBJ_HITBOX_HALF_H,y
    sta a:StreamStepHi
    lda a:WorkByteA
    sec
    sbc a:HitboxXLo,y
    bcs HitTestObject_at9
    eor #$FF
    inc a
HitTestObject_at9:
    cmp a:StreamStepLo
    bcs HitTestObject_at8
    lda a:WorkByteB
    sec
    sbc a:HitboxYLo,y
    bcs HitTestObject_at10
    eor #$FF
    inc a
HitTestObject_at10:
    cmp a:StreamStepHi
    bcc HitTestObject_at11
HitTestObject_at8:
    dey
    bpl HitTestObject_at7
HitTestObject_at1:
    rts
HitTestObject_at11:
    txa
    sta a:OBJ_WORK_O,y
    sty z:zHitSlot
    lda a:OBJ_WORK_N,y
    beq HitStoreHandler_at0
    dec a
    asl a
    tay
    lda a:$9525,y
    sta z:zTemp00
    lda a:$9526,y
    sta z:zTemp01
    ldy z:zHitSlot
    jmp (ZeroPage)

; @routine 024F:543 HitStoreHandler
; @run 6B:9543
; WHAT IT DOES: What a hit does for the kinds that take the hit's value: the hit slot's value
; goes into the object's hit-taken byte, and when slot 0's hit-taken byte was empty it is given
; the object's own two stored bytes.
; HOW: The entry of the hit test's jump table for the first four kinds.
; Eleven stage overlays hold this code, the ones whose hit test is at $9466.
; WAITS: no.
; Entry: X the object, Y the hit slot. Exit: A the last byte loaded, N and Z from it. C and V kept.
HitStoreHandler:
    lda a:OBJ_WORK_N,y
    sta a:OBJ_HIT_TAKEN,x
    lda a:OBJ_HIT_TAKEN
    bne HitStoreHandler_at0
    lda a:OBJ_WORK_C,x
    sta a:OBJ_HIT_TAKEN
    lda a:OBJ_WORK_B,x
    sta a:OBJ_HEALTH
HitStoreHandler_at0:
    rts

; @routine 024F:568 HitReportHandler
; @run 6B:9568
; WHAT IT DOES: What a hit does for the kinds that report it: the hit's position (the two
; work bytes and the other side's hitbox position) go to the stage hit report, and the hit
; slot's value to the object's hit-taken byte.
; HOW: Four copies into StageHitReport, then the same store as HitStoreHandler's first.
; Every stage overlay holds this code.
; WAITS: no.
; Entry: X the object, Y the hit slot. Exit: A the value stored, N and Z from it. C and V kept.
HitReportHandler:
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
HitReportHandler_ret:
    rts

; @routine 05BF:70A HitTestAllObjectsLong
; @run 6B:970A
; WHAT IT DOES: HitTestAllObjects for the other stage group, whose hit test is at $9778.
; HOW: The same code at other run addresses, calling ComputeHitboxPositionsLong and HitTestObjectLong.
; WAITS: no.
; Entry, exit: as HitTestAllObjects.
HitTestAllObjectsLong:
    phx
    jsr ComputeHitboxPositionsLong
    ldx #$57
HitTestAllObjectsLong_at1:
    jsr HitTestObjectLong
    lda a:OBJ_INVULN_TIMER,x
    beq HitTestAllObjectsLong_at0
    stz a:OBJ_HIT_TAKEN,x
    dec a:OBJ_INVULN_TIMER,x
HitTestAllObjectsLong_at0:
    dex
    cpx #$10
    bcs HitTestAllObjectsLong_at1
    plx
    rts

; @routine 05BF:778 HitTestObjectLong
; @run 6B:9778
; @stop 9837
; WHAT IT DOES: HitTestObject for the other stage group, which keeps its hit test at $9778 and its
; handlers at $9855 and $987A.
; HOW: The same code with its own table.
; WAITS: no.
; Entry, exit: as HitTestObject.
HitTestObjectLong:
    stz a:OBJ_HIT_TAKEN,x
    lda a:OBJ_HITBOX_ON,x
    bne HitTestObjectLong_at0
    jmp HitTestObjectLong_at1
HitTestObjectLong_at0:
    lda a:OBJ_HITBOX_HALF_W,x
    ora a:OBJ_HITBOX_HALF_H,x
    bne HitTestObjectLong_at2
    jmp HitTestObjectLong_at1
HitTestObjectLong_at2:
    lda a:OBJ_HITBOX_DX,x
    sta z:zTemp01
    lda a:OBJ_FACING_LEFT,x
    beq HitTestObjectLong_at3
    lda z:zTemp01
    eor #$FF
    inc a
    sta z:zTemp01
HitTestObjectLong_at3:
    stz z:zTemp00
    lda z:zTemp01
    bpl HitTestObjectLong_at4
    dec z:zTemp00
HitTestObjectLong_at4:
    clc
    adc a:OBJ_X_LO,x
    sta a:WorkByteA
    lda a:OBJ_X_HI,x
    adc z:zTemp00
    bne HitTestObjectLong_at1
    stz z:zTemp00
    lda a:OBJ_HITBOX_DY,x
    bpl HitTestObjectLong_at5
    dec z:zTemp00
HitTestObjectLong_at5:
    clc
    adc a:OBJ_Y_LO,x
    sta a:WorkByteB
    lda a:OBJ_Y_HI,x
    adc z:zTemp00
    bne HitTestObjectLong_at1
    lda a:OBJ_HITBOX_ON,x
    bpl HitTestObjectLong_at6
    cly
    bra HitTestObjectLong_at7
HitTestObjectLong_at6:
    ldy #$0F
HitTestObjectLong_at7:
    lda a:OBJ_HITBOX_ON,y
    beq HitTestObjectLong_at8
    lda a:HitboxHi,y
    bne HitTestObjectLong_at8
    lda a:OBJ_HITBOX_HALF_W,x
    clc
    adc a:OBJ_HITBOX_HALF_W,y
    sta a:StreamStepLo
    lda a:OBJ_HITBOX_HALF_H,x
    clc
    adc a:OBJ_HITBOX_HALF_H,y
    sta a:StreamStepHi
    lda a:WorkByteA
    sec
    sbc a:HitboxXLo,y
    bcs HitTestObjectLong_at9
    eor #$FF
    inc a
HitTestObjectLong_at9:
    cmp a:StreamStepLo
    bcs HitTestObjectLong_at8
    lda a:WorkByteB
    sec
    sbc a:HitboxYLo,y
    bcs HitTestObjectLong_at10
    eor #$FF
    inc a
HitTestObjectLong_at10:
    cmp a:StreamStepHi
    bcc HitTestObjectLong_at11
HitTestObjectLong_at8:
    dey
    bpl HitTestObjectLong_at7
HitTestObjectLong_at1:
    rts
HitTestObjectLong_at11:
    txa
    sta a:OBJ_WORK_O,y
    sty z:zHitSlot
    lda a:OBJ_WORK_N,y
    beq $986C
    dec a
    asl a
    tay
    lda a:TileRowsB+2,y
    sta z:zTemp00
    lda a:TileRowsB+3,y
    sta z:zTemp01
    ldy z:zHitSlot
    jmp (ZeroPage)

; @routine 18CF:778 HitTestObjectLongB
; @run 6B:9778
; @stop 9837
; WHAT IT DOES: HitTestObjectLong for the overlay at 18CF, whose bytes the intake keeps apart from the
; other overlays' because its handler table and what follows it differ.
; HOW: The same code with the same table address.
; WAITS: no.
; Entry, exit: as HitTestObject.
HitTestObjectLongB:
    stz a:OBJ_HIT_TAKEN,x
    lda a:OBJ_HITBOX_ON,x
    bne HitTestObjectLongB_at0
    jmp HitTestObjectLongB_at1
HitTestObjectLongB_at0:
    lda a:OBJ_HITBOX_HALF_W,x
    ora a:OBJ_HITBOX_HALF_H,x
    bne HitTestObjectLongB_at2
    jmp HitTestObjectLongB_at1
HitTestObjectLongB_at2:
    lda a:OBJ_HITBOX_DX,x
    sta z:zTemp01
    lda a:OBJ_FACING_LEFT,x
    beq HitTestObjectLongB_at3
    lda z:zTemp01
    eor #$FF
    inc a
    sta z:zTemp01
HitTestObjectLongB_at3:
    stz z:zTemp00
    lda z:zTemp01
    bpl HitTestObjectLongB_at4
    dec z:zTemp00
HitTestObjectLongB_at4:
    clc
    adc a:OBJ_X_LO,x
    sta a:WorkByteA
    lda a:OBJ_X_HI,x
    adc z:zTemp00
    bne HitTestObjectLongB_at1
    stz z:zTemp00
    lda a:OBJ_HITBOX_DY,x
    bpl HitTestObjectLongB_at5
    dec z:zTemp00
HitTestObjectLongB_at5:
    clc
    adc a:OBJ_Y_LO,x
    sta a:WorkByteB
    lda a:OBJ_Y_HI,x
    adc z:zTemp00
    bne HitTestObjectLongB_at1
    lda a:OBJ_HITBOX_ON,x
    bpl HitTestObjectLongB_at6
    cly
    bra HitTestObjectLongB_at7
HitTestObjectLongB_at6:
    ldy #$0F
HitTestObjectLongB_at7:
    lda a:OBJ_HITBOX_ON,y
    beq HitTestObjectLongB_at8
    lda a:HitboxHi,y
    bne HitTestObjectLongB_at8
    lda a:OBJ_HITBOX_HALF_W,x
    clc
    adc a:OBJ_HITBOX_HALF_W,y
    sta a:StreamStepLo
    lda a:OBJ_HITBOX_HALF_H,x
    clc
    adc a:OBJ_HITBOX_HALF_H,y
    sta a:StreamStepHi
    lda a:WorkByteA
    sec
    sbc a:HitboxXLo,y
    bcs HitTestObjectLongB_at9
    eor #$FF
    inc a
HitTestObjectLongB_at9:
    cmp a:StreamStepLo
    bcs HitTestObjectLongB_at8
    lda a:WorkByteB
    sec
    sbc a:HitboxYLo,y
    bcs HitTestObjectLongB_at10
    eor #$FF
    inc a
HitTestObjectLongB_at10:
    cmp a:StreamStepHi
    bcc HitTestObjectLongB_at11
HitTestObjectLongB_at8:
    dey
    bpl HitTestObjectLongB_at7
HitTestObjectLongB_at1:
    rts
HitTestObjectLongB_at11:
    txa
    sta a:OBJ_WORK_O,y
    sty z:zHitSlot
    lda a:OBJ_WORK_N,y
    beq $986C
    dec a
    asl a
    tay
    lda a:TileRowsB+2,y
    sta z:zTemp00
    lda a:TileRowsB+3,y
    sta z:zTemp01
    ldy z:zHitSlot
    jmp (ZeroPage)

; @routine 05C0:055 HitStoreHandlerLong
; @run 6B:9855
; WHAT IT DOES: HitStoreHandler in 05BF's group of stage overlays: the hit slot's value into the object's hit-taken
; byte, and slot 0's given the object's two stored bytes when its own was empty.
; HOW: The same instructions as HitStoreHandler, the entry of HitTestObjectLong's jump table for the first four kinds.
; The thirteen overlays of 05BF's group hold these bytes.
; WAITS: no.
; Entry: X the object, Y the hit slot. Exit: as HitStoreHandler.
HitStoreHandlerLong:
    lda a:OBJ_WORK_N,y
    sta a:OBJ_HIT_TAKEN,x
    lda a:OBJ_HIT_TAKEN
    bne HitStoreHandlerLong_at0
    lda a:OBJ_WORK_C,x
    sta a:OBJ_HIT_TAKEN
    lda a:OBJ_WORK_B,x
    sta a:OBJ_HEALTH
HitStoreHandlerLong_at0:
    rts

; @routine 05C0:07A HitReportHandlerLong
; @run 6B:987A
; WHAT IT DOES: HitReportHandler in 05BF's group of stage overlays: the hit's position to the stage hit report and the
; hit slot's value to the object's hit-taken byte.
; HOW: The same instructions as HitReportHandler. The thirteen overlays of 05BF's group hold these bytes.
; WAITS: no.
; Entry: X the object, Y the hit slot. Exit: as HitReportHandler.
HitReportHandlerLong:
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
    rts
