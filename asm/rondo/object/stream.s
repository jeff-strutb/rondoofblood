; @routine 0046:53E StreamFullWalk
; @run 69:5D3E
; WHAT IT DOES: Walks the stream across the screen and back when a level section is
; entered, so everything that belongs on screen comes into being, with the full-walk
; flag up for handlers that act only then.
; HOW: A forward walk of $141 lines and a backward walk of $41 from the scroll on the
; axis the level runs along.
; Entry: LevelAxis. Exit: as StreamWalkLines; the flag down. Clobbers zTemp04-06.
StreamFullWalk:
    lda a:LevelAxis
    cmp #$01
    beq StreamFullWalk_at0
    lda #$01
    sta a:LevelFullWalk
    lda #$01
    sta a:StreamStepLo
    stz a:StreamStepHi
    lda a:ScrollXHiScreen
    sta z:zTemp05
    lda a:ScrollXLo
    sta z:zTemp04
    lda #$01
    sta a:WorkByteA
    lda #$41
    sta a:WorkByteB
    lda #$01
    sta z:zTemp06
    jsr StreamWalkLines
    lda #$FF
    sta a:StreamStepLo
    lda #$FF
    sta a:StreamStepHi
    lda a:ScrollXHiScreen
    sta z:zTemp05
    lda a:ScrollXLo
    sta z:zTemp04
    lda #$00
    sta a:WorkByteA
    lda #$41
    sta a:WorkByteB
    lda #$01
    sta z:zTemp06
    jsr StreamWalkLines
    stz a:LevelFullWalk
    rts
StreamFullWalk_at0:
    lda #$01
    sta a:LevelFullWalk
    lda #$01
    sta a:StreamStepLo
    stz a:StreamStepHi
    lda a:ScrollYHiScreen
    sta z:zTemp05
    lda a:ScrollYLo
    sta z:zTemp04
    lda #$01
    sta a:WorkByteA
    lda #$41
    sta a:WorkByteB
    lda #$01
    sta z:zTemp06
    jsr StreamWalkLines
    lda #$FF
    sta a:StreamStepLo
    lda #$FF
    sta a:StreamStepHi
    lda a:ScrollYHiScreen
    sta z:zTemp05
    lda a:ScrollYLo
    sta z:zTemp04
    lda #$00
    sta a:WorkByteA
    lda #$41
    sta a:WorkByteB
    lda #$01
    sta z:zTemp06
    jsr StreamWalkLines
    stz a:LevelFullWalk
    rts

; @routine 0046:5E7 StreamWalkLines
; @run 69:5DE7
; WHAT IT DOES: Walks the stream line by line across a span, a walk at every step.
; HOW: From the stream position, StreamWalkForward or Back by the sign of the step;
; then adds the step to the position and counts the word in WorkByteA and B down.
; Entry: zTemp04-05 position, zTemp06 range, StreamStep the step, WorkByteA-B the
; count. Exit: A 0, N 0, Z 1, C from the count's borrow, V from the last subtract, Y
; as the last walk left it. X kept.
StreamWalkLines:
    lda a:LevelStreamLo
    sta z:zTemp10
    lda a:LevelStreamHi
    sta z:zTemp11
    cly
    lda (zTemp10),y
    sta z:zTemp12
    iny
    lda (zTemp10),y
    sta z:zProduct0
    lda a:StreamStepLo
    bmi StreamWalkLines_at0
    jsr StreamWalkForward
    bra StreamWalkLines_at1
StreamWalkLines_at0:
    jsr StreamWalkBack
StreamWalkLines_at1:
    lda z:zTemp04
    clc
    adc a:StreamStepLo
    sta z:zTemp04
    lda z:zTemp05
    adc a:StreamStepHi
    sta z:zTemp05
    lda a:WorkByteB
    sec
    sbc #$01
    sta a:WorkByteB
    lda a:WorkByteA
    sbc #$00
    sta a:WorkByteA
    ora a:WorkByteB
    bne StreamWalkLines
    rts

; @routine 0046:62E StreamScrollStep
; @run 69:5E2E
; WHAT IT DOES: Each frame, walks the stream at whichever screen edge the scroll moved
; toward.
; HOW: Picks the axis the level runs along and goes by the sign of its scroll speed;
; no speed returns.
; Entry: nothing. Exit: as the walk; no speed, A 0, N 0, Z 1, Y 1. C and V from the
; compare and what came before.
StreamScrollStep:
    lda a:LevelStreamLo
    sta z:zTemp10
    lda a:LevelStreamHi
    sta z:zTemp11
    cly
    lda (zTemp10),y
    sta z:zTemp12
    iny
    lda (zTemp10),y
    sta z:zProduct0
    lda a:LevelAxis
    cmp #$01
    beq StreamScrollStep_at0
    lda a:ScrollSpeedX
    beq StreamScrollStep_at1
    bpl StreamScrollStep_at2
    jmp StreamEdgeLeft
StreamScrollStep_at2:
    jmp StreamEdgeRight
StreamScrollStep_at0:
    lda a:ScrollSpeedY
    beq StreamScrollStep_at1
    bmi StreamScrollStep_at3
    jmp StreamEdgeDown
StreamScrollStep_at3:
    jmp StreamEdgeUp
StreamScrollStep_at1:
    rts

; @routine 0046:664 StreamEdgeRight
; @run 69:5E64
; WHAT IT DOES: Scrolling right: walks the stream at the screen's right edge.
; HOW: Scroll plus $140 as the position, the scroll speed as the range.
; Entry: zTemp10-11 and the record position set. Exit: as StreamWalkForward.
StreamEdgeRight:
    lda a:ScrollXLo
    clc
    adc #$40
    sta z:zTemp04
    lda a:ScrollXHiScreen
    adc #$01
    sta z:zTemp05
    lda a:ScrollSpeedX
    sta z:zTemp06
    jmp StreamWalkForward

; @routine 0046:67B StreamEdgeLeft
; @run 69:5E7B
; WHAT IT DOES: Scrolling left: walks the stream back at the screen's left edge,
; unless that lies before the level's start.
; HOW: Scroll less $40 as the position, the scroll speed negated as the range.
; Entry: as StreamEdgeRight. Exit: as StreamWalkBack; before the start, A the
; position's high byte, N 1, Z 0, C and V from its add.
StreamEdgeLeft:
    lda a:ScrollXLo
    clc
    adc #$C0
    sta z:zTemp04
    lda a:ScrollXHiScreen
    adc #$FF
    bmi StreamEdgeLeft_at0
    sta z:zTemp05
    lda a:ScrollSpeedX
    eor #$FF
    inc a
    sta z:zTemp06
    jmp StreamWalkBack
StreamEdgeLeft_at0:
    rts

; @routine 0046:698 StreamEdgeDown
; @run 69:5E98
; WHAT IT DOES: Scrolling down: walks the stream at the screen's bottom edge.
; HOW: As StreamEdgeRight on Y.
; Entry and exit: as StreamEdgeRight.
StreamEdgeDown:
    lda a:ScrollYLo
    clc
    adc #$40
    sta z:zTemp04
    lda a:ScrollYHiScreen
    adc #$01
    sta z:zTemp05
    lda a:ScrollSpeedY
    sta z:zTemp06
    jmp StreamWalkForward

; @routine 0046:6AF StreamEdgeUp
; @run 69:5EAF
; WHAT IT DOES: Scrolling up: walks the stream back at the screen's top edge, unless
; that lies before the level's start.
; HOW: As StreamEdgeLeft on Y.
; Entry and exit: as StreamEdgeLeft.
StreamEdgeUp:
    lda a:ScrollYLo
    clc
    adc #$C0
    sta z:zTemp04
    lda a:ScrollYHiScreen
    adc #$FF
    bmi StreamEdgeUp_at0
    sta z:zTemp05
    lda a:ScrollSpeedY
    eor #$FF
    inc a
    sta z:zTemp06
    jmp StreamWalkBack
StreamEdgeUp_at0:
    rts

; @routine 0046:6CC StreamWalkForward
; @run 69:5ECC
; WHAT IT DOES: Walks the stream forward from the record it stands on: acts on each
; record in range, skips those behind, stops at the first too far ahead or at the
; end mark, where the stream position stays on the last real record.
; HOW: StreamDistance against the range in zTemp06, then the position compared
; with the walk's position.
; Entry: zTemp10-11 the record, zTemp12, zProduct0 its position, zTemp04-05 the
; position, zTemp06 the range. Exit: A, Y and flags as the last step left them: Y 1
; once a record has been stepped to. X kept. Clobbers what the handlers do.
StreamWalkForward:
    jsr StreamDistance
    lda z:zProduct2
    bne StreamWalkForward_at0
    lda z:zProduct1
    cmp z:zTemp06
    bcs StreamWalkForward_at0
    jsr RunStreamRecord
StreamWalkForward_at2:
    lda z:zTemp10
    clc
    adc #$0A
    sta z:zTemp10
    sta a:LevelStreamLo
    lda z:zTemp11
    adc #$00
    sta z:zTemp11
    sta a:LevelStreamHi
    cly
    lda (zTemp10),y
    sta z:zTemp12
    iny
    lda (zTemp10),y
    sta z:zProduct0
    bra StreamWalkForward
StreamWalkForward_at0:
    lda z:zProduct0
    and z:zTemp12
    cmp #$FF
    beq StreamWalkForward_at1
    lda z:zProduct0
    cmp z:zTemp05
    bcc StreamWalkForward_at2
    bne StreamWalkForward_at3
    lda z:zTemp12
    cmp z:zTemp04
    bcs StreamWalkForward_at3
    bra StreamWalkForward_at2
StreamWalkForward_at1:
    lda z:zTemp10
    sec
    sbc #$0A
    sta a:LevelStreamLo
    lda z:zTemp11
    sbc #$00
    sta a:LevelStreamHi
StreamWalkForward_at3:
    rts

; @routine 0046:723 StreamDistance
; @run 69:5F23
; WHAT IT DOES: Gives how far the record is from the walk's position, as a word.
; HOW: Subtracts the walk's position, and negates a negative result.
; Entry: zTemp12, zProduct0 record position, zTemp04-05 walk position. Exit:
; zProduct1-2 the distance; A its high byte, N, Z, C and V from the last add or
; subtract. X and Y kept.
StreamDistance:
    lda z:zTemp12
    sec
    sbc z:zTemp04
    sta z:zProduct1
    lda z:zProduct0
    sbc z:zTemp05
    sta z:zProduct2
    bcs StreamDistance_at0
    lda z:zProduct1
    eor #$FF
    clc
    adc #$01
    sta z:zProduct1
    lda z:zProduct2
    eor #$FF
    adc #$00
    sta z:zProduct2
StreamDistance_at0:
    rts

; @routine 0046:744 StreamWalkBack
; @run 69:5F44
; WHAT IT DOES: Walks the stream backward from the record it stands on, as
; StreamWalkForward does forward: stops at the first record behind the position, at
; the end mark, or before the stream's start, where a record's position has its top
; bit set.
; HOW: The same tests the other way. The stream position moves only onto a record
; that passes the top-bit test.
; Entry and exit: as StreamWalkForward.
StreamWalkBack:
    jsr StreamDistance
    lda z:zProduct2
    bne StreamWalkBack_at0
    lda z:zProduct1
    cmp z:zTemp06
    bcs StreamWalkBack_at0
    jsr RunStreamRecord
StreamWalkBack_at3:
    lda z:zTemp10
    sec
    sbc #$0A
    sta z:zTemp10
    lda z:zTemp11
    sbc #$00
    sta z:zTemp11
    ldy #$01
    lda (zTemp10),y
    bmi StreamWalkBack_at1
    lda z:zTemp10
    sta a:LevelStreamLo
    lda z:zTemp11
    sta a:LevelStreamHi
    cly
    lda (zTemp10),y
    sta z:zTemp12
    iny
    lda (zTemp10),y
    sta z:zProduct0
    bra StreamWalkBack
StreamWalkBack_at0:
    lda z:zProduct0
    and z:zTemp12
    cmp #$FF
    beq StreamWalkBack_at2
    lda z:zProduct0
    cmp z:zTemp05
    bcc StreamWalkBack_at1
    bne StreamWalkBack_at3
    lda z:zTemp12
    cmp z:zTemp04
    bcc StreamWalkBack_at1
    bra StreamWalkBack_at3
StreamWalkBack_at2:
    lda z:zTemp10
    clc
    adc #$0A
    sta a:LevelStreamLo
    lda z:zTemp11
    adc #$00
    sta a:LevelStreamHi
StreamWalkBack_at1:
    rts

; @routine 0046:7A5 RunStreamRecord
; @run 69:5FA5
; WHAT IT DOES: Acts on the record the walk stands on, unless its event has already
; happened.
; HOW: Copies its eight bytes to zero page, the slot with $10 added past the player's
; own slots. Tests the event's bit in EventFlags, and goes through the handler table,
; whose entry it keeps in zTemp00-01.
; Entry: zTemp10-11 the record. Exit: event done, A the bit found, N and Z from it, Y
; its byte, C as BitOfIndex. Else as the handler, with Y its table index if the
; handler leaves it. X kept. Clobbers zTemp00-01, zTemp08-0F.
RunStreamRecord:
    ldy #$02
    lda (zTemp10),y
    sta z:zTemp08
    iny
    lda (zTemp10),y
    sta z:zTemp09
    iny
    lda (zTemp10),y
    sta z:zTemp0A
    iny
    lda (zTemp10),y
    clc
    adc #$10
    sta z:zTemp0B
    iny
    lda (zTemp10),y
    sta z:zTemp0C
    iny
    lda (zTemp10),y
    sta z:zTemp0D
    iny
    lda (zTemp10),y
    sta z:zTemp0E
    iny
    lda (zTemp10),y
    sta z:zTemp0F
    jsr VecBitOfIndex
    and a:EventFlags,y
    beq RunStreamRecord_at0
    rts
RunStreamRecord_at0:
    lda z:zTemp08
    asl a
    tay
    lda a:StreamHandlers,y
    sta z:zTemp00
    lda a:StreamHandlers+1,y
    sta z:zTemp01
    jmp (zTemp00+$2000)

; @routine 0046:7F9 StreamSpawnIfFree
; @run 69:5FF9
; WHAT IT DOES: Record handler 0: starts an object in the record's slot, if that slot
; is empty.
; HOW: The slot's type must be zero. The object is placed at the record's position
; less the scroll, on the axis the level runs along.
; Entry: the record in zTemp09-0F, zTemp12, zProduct0. Exit: A zTemp0F (or the
; slot's type, if taken), X and Y kept, N and Z from X, C and V from the last step.
StreamSpawnIfFree:
    phx
    ldx z:zTemp0B
    lda a:OBJ_2750,x
    beq StreamPlaceObject
    jmp StreamPlaceObject_at0

; @routine 0047:004 StreamPlaceObject
; @run 6A:6004
; WHAT IT DOES: The body of the spawning record handlers, entered with the slot in X.
; HOW: As StreamSpawnIfFree, past its check.
; Entry: X slot, on the stack the caller's X. Exit: as StreamSpawnIfFree.
StreamPlaceObject:
    jsr VecClearObject
    lda z:zTemp0C
    sta a:OBJ_2750,x
    lda a:LevelAxis
    cmp #$01
    beq StreamPlaceObject_at1
    lda z:zTemp12
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda z:zProduct0
    sbc a:ScrollXHiScreen
    sta a:OBJ_X_HI,x
    lda z:zTemp09
    sec
    sbc a:ScrollYOffset
    sta a:OBJ_Y_LO,x
    lda z:zTemp0A
    sbc a:ScrollYHiScreen
    sta a:OBJ_Y_HI,x
    lda a:LevelAxis
    cmp #$02
    beq StreamPlaceObject_at2
    stz z:zTemp09
    lda a:ScrollYLo
    bpl StreamPlaceObject_at3
    dec z:zTemp09
StreamPlaceObject_at3:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollYLo
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp09
    sta a:OBJ_Y_HI,x
    bra StreamPlaceObject_at2
StreamPlaceObject_at1:
    lda z:zTemp12
    sec
    sbc a:ScrollYLo
    sta a:OBJ_Y_LO,x
    lda z:zProduct0
    sbc a:ScrollYHiScreen
    sta a:OBJ_Y_HI,x
    lda z:zTemp09
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda z:zTemp0A
    sbc a:ScrollXHiScreen
    sta a:OBJ_X_HI,x
StreamPlaceObject_at2:
    lda z:zTemp0D
    sta a:OBJ_3040,x
    lda z:zTemp0E
    sta a:OBJ_FLAGS_2F38,x
    lda z:zTemp0F
    sta a:OBJ_EVENT_ID,x
StreamPlaceObject_at0:
    plx
    rts

; @routine 0047:08C StreamStartScript
; @run 6A:608C
; WHAT IT DOES: Record handler 1: starts a script slot, kind 1.
; HOW: Slot and script from the record; then RecordIfMarked.
; Entry: zTemp09 script, zTemp0A slot. Exit: as RecordIfMarked, with Y the script
; if nothing else set it.
StreamStartScript:
    phx
    lda z:zTemp09
    ldx z:zTemp0A
    jsr VecStartScriptSlotKind1
    plx

; @routine 0047:095 RecordIfMarked
; @run 6A:6095
; WHAT IT DOES: The end of the record handlers: if this record's flags ask for it,
; marks its event.
; HOW: Bit 7 of zTemp0E, event in zTemp0F.
; Entry: zTemp0E, zTemp0F. Exit: unmarked, A zTemp0E, N and Z from it, C, Y kept.
; Else as RecordEvent. X and V kept.
RecordIfMarked:
    lda z:zTemp0E
    bpl RecordIfMarked_at0
    lda z:zTemp0F
    jsr VecRecordEvent
RecordIfMarked_at0:
    rts

; @routine 0047:09F StreamStartListKind2
; @run 6A:609F
; WHAT IT DOES: Record handler 2: starts a list channel as kind 2, only while the
; full-walk flag is up: that is, when a level section is first entered.
; HOW: Tests LevelFullWalk; then list and channel from the record, and RecordIfMarked.
; Entry: zTemp09 list, zTemp0A channel. Exit: flag down, A 0, N 0, Z 1. Else as
; RecordIfMarked.
StreamStartListKind2:
    lda a:LevelFullWalk
    beq StreamStartListKind2_at0
    phx
    lda z:zTemp09
    ldx z:zTemp0A
    jsr VecStartListChannelKind2
    plx
    bra RecordIfMarked
StreamStartListKind2_at0:
    rts

; @routine 0047:0B0 StreamStartListKind3
; @run 6A:60B0
; WHAT IT DOES: Record handler 3: starts a list channel as kind 3.
; HOW: List and channel from the record; then RecordIfMarked.
; Entry: zTemp09 list, zTemp0A channel. Exit: as RecordIfMarked.
StreamStartListKind3:
    phx
    lda z:zTemp09
    ldx z:zTemp0A
    jsr VecStartListChannelKind3
    plx
    bra RecordIfMarked

; @routine 0047:0BB StreamClearObject
; @run 6A:60BB
; WHAT IT DOES: Record handler 4: empties an object slot.
; HOW: ClearObject on the record's slot; then RecordIfMarked.
; Entry: zTemp09 slot. Exit: as RecordIfMarked.
StreamClearObject:
    phx
    ldx z:zTemp09
    jsr VecClearObject
    plx
    bra RecordIfMarked

; @routine 0047:0C4 StreamSpawnIfOther
; @run 6A:60C4
; WHAT IT DOES: Record handler 5: starts an object in the record's slot unless that
; slot already holds the object for this same event, so one spawned earlier stays.
; HOW: Empty, or a different event id, places the object.
; Entry: as StreamSpawnIfFree. Exit: placed, as StreamSpawnIfFree. Same event: A the
; event id, N and Z from X, C 1. X and Y kept. An inner entry at $6004 places directly.
StreamSpawnIfOther:
    phx
    ldx z:zTemp0B
    lda a:OBJ_2750,x
    bne StreamSpawnIfOther_at0
    jmp StreamPlaceObject
StreamSpawnIfOther_at0:
    lda a:OBJ_EVENT_ID,x
    cmp z:zTemp0F
    beq StreamSpawnIfOther_at1
    jmp StreamPlaceObject
StreamSpawnIfOther_at1:
    plx
    rts

; @routine 0047:0DB StreamCallOverlay
; @run 6A:60DB
; WHAT IT DOES: Record handler 6: calls the code the record points to.
; HOW: JMP through the pointer in zTemp09-0A; then RecordIfMarked.
; Entry: zTemp09-0A code. Exit: as RecordIfMarked.
StreamCallOverlay:
    phx
    jsr JumpThroughRecord
    plx
    jmp RecordIfMarked

; @routine 0047:0E3 JumpThroughRecord
; @run 6A:60E3
; WHAT IT DOES: The jump StreamCallOverlay reaches a record's code by.
; HOW: JMP [zTemp09].
; Entry: zTemp09-0A code. Exit: as that code leaves it.
JumpThroughRecord:
    jmp (zTemp09+$2000)
