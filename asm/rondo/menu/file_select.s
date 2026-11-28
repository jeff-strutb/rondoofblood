; @routine 00C1:228 IncrementTempWord
; @run 6B:8228
; WHAT IT DOES: Adds one to the 16-bit pointer in zTemp00 and zTemp01, the place the text
; printer reads its next byte from.
; HOW: Adds one to the low byte, and to the high byte when the low byte wrapped to zero.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the byte last incremented, C and the registers kept.
MenuBlocked = $7DDB ; menu blocked
PadRepeat = $7DF2 ; pad repeat
CursorFirstItems = $9CBE ; cursor first items
CursorLastItems = $9CC4 ; cursor last items
CursorFrames = $9D55 ; cursor frames
GlyphPasses = $A0F2 ; glyph passes
GlyphNumbers = $A18F ; glyph numbers
GlyphPlaces = $A500 ; glyph places
ObjectLists = $A55A ; object lists
PercentAddresses = $A845 ; percent addresses
TextColorAddresses = $A901 ; text color addresses
TextPalettes = $A91B ; text palettes
FaceTiles = $A9BE ; face tiles
CursorPlaces = $AD1F ; cursor places
ClearedTableOffsets = $AFD6 ; cleared table offsets
ClearedTableMasks = $AFE3 ; cleared table masks
IncrementTempWord:
    inc z:zTemp00
    bne IncrementTempWord_at0
    inc z:zTemp01
IncrementTempWord_at0:
    rts

; @routine 00C1:5CA WriteGlyphRowsTwice
; @run 6B:85CA
; WHAT IT DOES: Writes the same eight glyph rows twice, so a glyph fills two tiles' worth.
; HOW: Keeps X on the stack across one call of WriteGlyphRows, then runs on into it with X as it
; was.
; WAITS: no.
; Entry, exit: as WriteGlyphRows.
WriteGlyphRowsTwice:
    phx
    jsr WriteGlyphRows
    plx

; @routine 00C1:5CF WriteGlyphRows
; @run 6B:85CF
; WHAT IT DOES: Writes eight rows of a glyph's tile to the VDC, from the glyph buffer at $3B40:
; every other byte from the buffer is the low byte of a row's word, $FF the high byte.
; HOW: Eight times, the buffer byte at X goes to the VDC data port and $FF to its high byte; X
; goes up by two each row.
; WAITS: no.
; Entry: X the buffer offset; the VDC's write address set. Exit: A the last byte written, X plus
; 16, Y 8, Z and C set, N clear.
WriteGlyphRows:
    cly
WriteGlyphRows_at0:
    lda a:StageMaskBuffer,x
    sta a:VDC_DATA_LO
    st2 #$FF
    inx
    inx
    iny
    cpy #$08
    bne WriteGlyphRows_at0
    rts

; @routine 00C3:2E7 PadRepeatReset
; @run 6B:92E7
; WHAT IT DOES: Starts the menu's key repeat afresh: no button is down, and each of the eight
; buttons waits the full first delay.
; HOW: Zeroes the repeat bits ($7DFA), then loads each of the eight repeat counters (from $7DF2)
; with 24, from the last to the first.
; WAITS: no.
; Entry: nothing. Exit: A $18, X $FF, N set, Z clear; Y and C kept.
PadRepeatReset:
    stz a:MenuPadRepeatBits
    ldx #$07
    lda #$18
PadRepeatReset_at0:
    sta a:PadRepeat,x
    dex
    bpl PadRepeatReset_at0
PadRepeatReset_at1:
    rts

; @routine 00C3:31B MenuBackOnButtonII
; @run 6B:931B
; WHAT IT DOES: Takes the menu back to its page 8 when button II has just been pressed.
; HOW: ANDs the card's trigger byte with 2 and returns when it is zero; otherwise runs on into
; MenuBackToPageEight.
; WAITS: no.
; Entry: nothing. Exit: not pressed: A 0, Z set, N clear, C kept. Pressed: as
; MenuBackToPageEight with N and Z clear.
MenuBackOnButtonII:
    lda a:BiosJoyTrigger
    and #$02
    beq PadRepeatReset_at1

; @routine 00C3:322 MenuBackToPageEight
; @run 6B:9322
; WHAT IT DOES: Puts the menu on its page 8, at its first step.
; HOW: Loads 8 and goes to SetMenuPage.
; WAITS: no.
; Entry: nothing. Exit: A 0, N and Z clear (the 8 loaded), C kept.
MenuBackToPageEight:
    lda #$08
    jmp SetMenuPage

; @routine 00C4:025 PadRepeatUpdate
; @run 6B:9825
; WHAT IT DOES: Gives the menu a pad with key repeat: for each of the eight pad bits, a bit in
; $7DFA that is set on the frame the button goes down and then every 12 frames after 24 held.
; HOW: The new presses (the card's trigger byte) and the held bits (the card's pad byte) are
; shifted out a bit at a time. A new press loads the bit's counter (the eight bytes from $7DF2)
; with 24 and sets the bit. A held button counts the counter down; at zero it loads 12 and sets
; the bit. A released button clears the counter. Each bit is rotated into $7DFA, so that button 0
; ends on bit 0.
; WAITS: no. The arm that reloads a counter that has run down to zero (LDA #$0C) is in no script's
; trace: the draft stops there, so the comparison does not reach it.
; Entry: nothing. Exit: A the last value the counter loop loaded (24, 12, 0, or the held byte if
; no counter was loaded), X 8, Z and C set, N clear. Clobbers zTemp00 and zTemp01.
PadRepeatUpdate:
    lda a:BiosJoyTrigger
    sta z:zTemp00
    lda a:BiosPadState
    sta z:zTemp01
    clx
PadRepeatUpdate_at4:
    lsr z:zTemp00
    bcc PadRepeatUpdate_at0
    lda #$18
    lsr z:zTemp01
    sec
    bra PadRepeatUpdate_at1
PadRepeatUpdate_at0:
    lsr z:zTemp01
    bcc PadRepeatUpdate_at2
    dec a:PadRepeat,x
    clc
    bne PadRepeatUpdate_at3
    lda #$0C
    sec
    bra PadRepeatUpdate_at1
PadRepeatUpdate_at2:
    cla
    clc
PadRepeatUpdate_at1:
    sta a:PadRepeat,x
PadRepeatUpdate_at3:
    ror a:MenuPadRepeatBits
    inx
    cpx #$08
    bne PadRepeatUpdate_at4
    rts

; @routine 00C4:0CC DrawSelectedFileDigit
; @run 6B:98CC
; WHAT IT DOES: Draws the number of the selected file as a digit tile at the screen place $00CE.
; HOW: Sets the VDC's write address to $00CE and writes the digit tile for the value in $7DF1
; (its value plus $10, palette 1).
; WAITS: no.
; Entry: nothing. Exit: A the value plus $10, N, Z, C and V from that add; X and Y kept.
DrawSelectedFileDigit:
    jsr VecSelectVramWriteAddress
    st1 #$CE
    st2 #$00
    jsr VecSelectVramData
    lda a:MenuSelectedSlot
DrawSelectedFileDigit_at0:
    clc
    adc #$10
    sta a:VDC_DATA_LO
    st2 #$11
    rts

; @routine 00C4:0E2 DrawCursorRowDigit
; @run 6B:98E2
; WHAT IT DOES: Draws the number of the cursor's row as a digit tile at the screen place $012B.
; HOW: Sets the VDC's write address to $012B and runs into DrawSelectedFileDigit's last
; part with the value in $7DFF.
; WAITS: no.
; Entry: nothing. Exit: as DrawSelectedFileDigit.
DrawCursorRowDigit:
    jsr VecSelectVramWriteAddress
    st1 #$2B
    st2 #$01
    jsr VecSelectVramData
    lda a:MenuChosenItem
    bra DrawSelectedFileDigit_at0

; @routine 00C4:3F6 ShiftObjectsByScrollChange
; @run 6B:9BF6
; WHAT IT DOES: Moves the menu's objects 16 to 63 sideways by what the menu's scroll has moved
; since the last time, so they stay where they are on the screen.
; HOW: The X scroll word kept last time (the menu keeps it in the first X and Y speed bytes,
; which it has no other use for) less the scroll's X word now is kept in their place as the shift, and added, low byte then high byte, to the X
; place of each of the objects.
; WAITS: no.
; Entry: nothing. Exit: A the last high byte sum, V from that sum, X $40, Z and C set, N clear.
ShiftObjectsByScrollChange:
    lda a:ScrollSpeedX
    sec
    sbc a:ScrollXLo
    sta a:ScrollSpeedX
    lda a:ScrollSpeedY
    sbc a:ScrollXHi
    sta a:ScrollSpeedY
    ldx #$10
ShiftObjectsByScrollChange_at0:
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollSpeedX
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    adc a:ScrollSpeedY
    sta a:OBJ_X_HI,x
    inx
    cpx #$40
    bne ShiftObjectsByScrollChange_at0
    rts

; @routine 00C4:46A SetCursorValue
; @run 6B:9C6A
; WHAT IT DOES: Puts both cursors on the cursor value A and sends them to its place.
; HOW: Stores A in both cursor objects' value and goes to SetCursorTargets.
; WAITS: no.
; Entry: A the cursor value. Exit: as SetCursorTargets.
SetCursorValue:
    sta a:OBJ_WORK_D
    sta a:OBJ_WORK_D+1
    jmp SetCursorTargets

; @routine 00C4:473 CursorMoveByPad
; @run 6B:9C73
; WHAT IT DOES: Moves the menu cursor up or down by the pad's new presses, wrapping within the
; menu's first and last item, and over the items that cannot be picked: item $14 when the file
; has no data (bit 0 of its first byte clear), item $15 when $7DDB is not zero.
; HOW: Up or down (the trigger byte's bits $10 and $40) takes one from or adds one to the cursor
; value (OBJ->f34b8 of object 0); at the first item's edge it wraps to the last (tables by
; menu, the first items at $9CBE, the last at $9CC4); SetCursorValue sends the cursors there.
; The arms that wrap at an edge and that skip an item are in no script's trace (the draft stops there).
; An item that cannot be picked repeats the move. The Y the callee leaves is the table
; index of the cursor, and it is that Y the original's next pass indexes the tables with.
; WAITS: no.
; Entry: Y the menu (the table index). Exit: as the last test left them: A 0 (no press, or the
; last test was $7DDB), or the value tested last; X 1 (kept when no press moved the cursor), Y
; the table index SetCursorValue left (kept when none).
CursorMoveByPad:
    lda a:BiosJoyTrigger
    and #$50
    beq CursorMoveByPad_at0
    and #$40
    bne CursorMoveByPad_at1
    lda a:OBJ_WORK_D
    dec a
    cmp a:CursorFirstItems,y
    bne CursorMoveByPad_at2
    lda a:CursorLastItems,y
    dec a
    bra CursorMoveByPad_at2
CursorMoveByPad_at1:
    lda a:OBJ_WORK_D
    inc a
    cmp a:CursorLastItems,y
    bne CursorMoveByPad_at2
    lda a:CursorFirstItems,y
    inc a
CursorMoveByPad_at2:
    jsr SetCursorValue
    lda a:OBJ_WORK_D
    cmp #$14
    bne CursorMoveByPad_at3
    phy
    ldy z:zSaveSlot
    ldx a:SlotOffsets,y
    lda a:SaveBuffer,x
    ply
    lsr a
    bcc CursorMoveByPad
CursorMoveByPad_at3:
    lda a:OBJ_WORK_D
    cmp #$15
    bne CursorMoveByPad_at0
    lda a:MenuBlocked
    bne CursorMoveByPad
CursorMoveByPad_at0:
    rts

; @routine 00C4:4CA PlaceCursorObjects
; @run 6B:9CCA
; WHAT IT DOES: PlaceCursorObject for both cursor objects, 0 then 1, with the same value.
; HOW: Keeps A on the stack; a call for object 0; the PLA and INX after it run on into
; PlaceCursorObject for object 1.
; WAITS: no.
; Entry: A the cursor value. Exit: as PlaceCursorObject for object 1, X 1.
PlaceCursorObjects:
    clx
    pha
    bsr PlaceCursorObject
    pla
    inx

; @routine 00C4:4D0 PlaceCursorObject
; @run 6B:9CD0
; WHAT IT DOES: Makes a cursor object afresh at the place its cursor value names.
; HOW: Keeps A on the stack across ClearObject, gives the object the value, frame 4 and the
; place from the same table SetCursorTarget reads (index as there).
; WAITS: no.
; Entry: A the cursor value, X the object (0 or 1). Exit: A the Y place, N and Z from it, C
; bit 6 of the cursor value, Y the table index. X kept.
PlaceCursorObject:
    pha
    jsr VecClearObject
    lda #$04
    sta a:OBJ_SPRITE_SET,x
    txa
    lsr a
    pla
    sta a:OBJ_WORK_D,x
    rol a
    asl a
    tay
    lda a:CursorPlaces,y
    sta a:OBJ_X_LO,x
    lda a:CursorPlaces+1,y
    sta a:OBJ_Y_LO,x
    rts

; @routine 00C4:4EF GlideCursorObjects
; @run 6B:9CEF
; WHAT IT DOES: GlideCursorObject for the two name cursor objects, 2 and then 3.
; HOW: A call for object 2; the INX after it runs on into GlideCursorObject for 3.
; WAITS: no.
; Entry: nothing. Exit: as GlideCursorObject for object 3, X 3.
GlideCursorObjects:
    ldx #$02
    bsr GlideCursorObject
    inx

; @routine 00C4:4F4 GlideCursorObject
; @run 6B:9CF4
; WHAT IT DOES: Moves a cursor object a quarter of the way to its target each frame, and
; stops it there; shows its picture frame while it is moving, or the frame that turns with the
; tick once it has arrived.
; HOW: If OBJ->work_e says it is moving, the speed on each axis is set to half the distance to the
; target (a 9-bit difference rotated down, its sign flipped), halved again by
; HalveObjectVelocity, and ApplyVelocity moves it. The picture frame comes from a table by
; whether it is below $B0 and whether the object is 3 or 4; when both places match the target
; the moving flag clears. A frame of 1 to 8 is turned by ObjectFrameFromTick.
; WAITS: no.
; Entry: X the object. Exit: A, N, Z and C as the frame turn or its compare leaves them (V from
; the move when it moved); Y the table index when it moved; X kept.
GlideCursorObject:
    lda a:OBJ_WORK_E,x
    beq GlideCursorObject_at0
    lda a:OBJ_WORK_F,x
    sec
    sbc a:OBJ_X_LO,x
    ror a
    eor #$80
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    ror a:OBJ_VX_LO,x
    lda a:OBJ_WORK_G,x
    sec
    sbc a:OBJ_Y_LO,x
    ror a
    eor #$80
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    ror a:OBJ_VY_LO,x
    jsr HalveObjectVelocity
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$B0
    cla
    rol a
    cpx #$03
    rol a
    tay
    lda a:CursorFrames,y
    sta a:OBJ_FRAME,x
    lda a:OBJ_X_LO,x
    cmp a:OBJ_WORK_F,x
    bne GlideCursorObject_at0
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_WORK_G,x
    bne GlideCursorObject_at0
    stz a:OBJ_WORK_E,x
GlideCursorObject_at0:
    lda a:OBJ_FRAME,x
    dec a
    cmp #$08
    bcs GlideCursorObject_at1
    jsr ObjectFrameFromTick
GlideCursorObject_at1:
    rts

; @routine 00C4:559 JitterCursorObjects
; @run 6B:9D59
; WHAT IT DOES: JitterCursorObject for both cursor objects, 0 then 1.
; HOW: A call for object 0; the INX after it runs on into JitterCursorObject for 1.
; WAITS: no.
; Entry: nothing. Exit: as JitterCursorObject for object 1, X 1.
JitterCursorObjects:
    clx
    bsr JitterCursorObject
    inx

; @routine 00C4:55D JitterCursorObject
; @run 6B:9D5D
; WHAT IT DOES: GlideCursorObject's way of moving, aimed at a point a few pixels off the
; target, chosen at random on each axis each frame, so the cursor looks to shake as it
; settles; its picture frame always turns with the tick.
; HOW: As GlideCursorObject, with the target on each axis moved by a random number from -8 to 7
; (NextRandom, masked with $0F, less 8); no table frame; at the end it goes to
; ObjectFrameFromTick.
; WAITS: no.
; Entry: X the object. Exit: as ObjectFrameFromTick; V from the move when it moved; X kept.
JitterCursorObject:
    lda a:OBJ_WORK_E,x
    beq JitterCursorObject_at0
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$08
    clc
    adc a:OBJ_WORK_F,x
    sec
    sbc a:OBJ_X_LO,x
    ror a
    eor #$80
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    ror a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$08
    clc
    adc a:OBJ_WORK_G,x
    sec
    sbc a:OBJ_Y_LO,x
    ror a
    eor #$80
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    ror a:OBJ_VY_LO,x
    jsr HalveObjectVelocity
    jsr VecApplyVelocity
    lda a:OBJ_X_LO,x
    cmp a:OBJ_WORK_F,x
    bne JitterCursorObject_at0
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_WORK_G,x
    bne JitterCursorObject_at0
    stz a:OBJ_WORK_E,x
JitterCursorObject_at0:
    jmp ObjectFrameFromTick

; @routine 00C4:5B6 SetCursorTargets
; @run 6B:9DB6
; WHAT IT DOES: SetCursorTarget for both cursor objects, 0 then 1.
; HOW: A call for object 0; the one byte after it, INX, runs on into SetCursorTarget for 1.
; WAITS: no.
; Entry: nothing. Exit: as SetCursorTarget for object 1, X 1.
SetCursorTargets:
    clx
    bsr SetCursorTarget
    inx

; @routine 00C4:5BA SetCursorTarget
; @run 6B:9DBA
; WHAT IT DOES: Sends a cursor object to the place its cursor value names: the target is
; read from a table of places, one pair for each value and each of the two cursors.
; HOW: The object's cursor value (OBJ->f34b8) doubled with the object's low bit under it, doubled
; again, is the table index; the pair at that index goes to the object's target (X and Y) and
; the object is marked as on its way (OBJ->work_e is 1).
; WAITS: no.
; Entry: X the cursor object (0 or 1). Exit: A 1, N and Z clear, C bit 6 of the cursor value, Y
; the table index. X kept.
SetCursorTarget:
    txa
    lsr a
    lda a:OBJ_WORK_D,x
    rol a
    asl a
    tay
    lda a:CursorPlaces,y
    sta a:OBJ_WORK_F,x
    lda a:CursorPlaces+1,y
    sta a:OBJ_WORK_G,x
    lda #$01
    sta a:OBJ_WORK_E,x
    rts

; @routine 00C4:5D4 HalveObjectVelocity
; @run 6B:9DD4
; WHAT IT DOES: Halves an object's speed along both axes, keeping the sign.
; HOW: The X speed (high byte, then low byte) and then the Y speed, each as a 16-bit
; arithmetic shift right: the sign is copied into the carry first so the high byte keeps it.
; WAITS: no.
; Entry: X the object. Exit: A the Y speed's high byte shifted left once, N and Z from the Y
; speed's new low byte, C the bit it lost. X and Y kept.
HalveObjectVelocity:
    lda a:OBJ_VX_HI,x
    asl a
    ror a:OBJ_VX_HI,x
    ror a:OBJ_VX_LO,x
    lda a:OBJ_VY_HI,x
    asl a
    ror a:OBJ_VY_HI,x
    ror a:OBJ_VY_LO,x
    rts

; @routine 00C4:5E9 ObjectFrameFromTick
; @run 6B:9DE9
; WHAT IT DOES: Gives an object the picture frame (1 to 8) that the frame counter says: a
; frame lasts four ticks, and the eight frames go round.
; HOW: Reads zFrameTick, shifts it right twice, keeps three bits, adds one, stores it in the
; object's frame byte.
; WAITS: no.
; Entry: X the object. Exit: A the frame, N and Z clear, C bit 1 of the tick. X and Y kept.
ObjectFrameFromTick:
    lda z:zFrameTick
    lsr a
    lsr a
    and #$07
    inc a
    sta a:OBJ_FRAME,x
    rts

; @routine 00C4:602 MenuPageAdvance
; @run 6B:9E02
; WHAT IT DOES: Moves the menu on to the first step of the next page.
; HOW: Reads zGameSubMode, adds one and runs on into SetMenuPage.
; WAITS: no.
; Entry: nothing. Exit: A 0, N and Z from the new page, C kept.
MenuPageAdvance:
    lda z:zGameSubMode
    inc a

; @routine 00C4:605 SetMenuPage
; @run 6B:9E05
; WHAT IT DOES: Puts the menu on page A, at its first step.
; HOW: Stores A in zGameSubMode, then runs on into SetMenuStep with the step 0.
; WAITS: no.
; Entry: A the page. Exit: A 0, the flags kept.
SetMenuPage:
    sta z:zGameSubMode
    cla
    bra SetMenuStep

; @routine 00C4:60A MenuStepAdvance
; @run 6B:9E0A
; WHAT IT DOES: Moves the menu on to the next step of its page.
; HOW: Reads zField80, adds one and runs on into SetMenuStep.
; WAITS: no.
; Entry: nothing. Exit: A the new step, N and Z from it, C kept.
MenuStepAdvance:
    lda z:zField80
    inc a

; @routine 00C4:60D SetMenuStep
; @run 6B:9E0D
; WHAT IT DOES: Puts the menu in step A of its page.
; HOW: One store into zField80.
; WAITS: no.
; Entry: A the step. Exit: A and the flags kept.
SetMenuStep:
    sta z:zField80
    rts

; @routine 00DD:01C ClearVramBlock
; @run 6F:A01C
; WHAT IT DOES: Blanks 1024 words of VRAM at the place the object's kind names, and moves the
; object on to its next state.
; HOW: The kind doubled picks the place from a table, set as the write address; $0000 is
; written 1024 times (four rounds of 256) and the object's state (OBJ->f27a8) goes up by one.
; WAITS: no.
; Entry: X the object. Exit: A 4, X kept, N and Z from the new state, C bit 7 of the kind, Y the
; kind doubled. Clobbers zTemp00.
ClearVramBlock:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    jsr VecSelectVramWriteAddress
    lda a:GlyphPlaces,y
    sta a:VDC_DATA_LO
    lda a:GlyphPlaces+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    phx
    lda #$04
    sta z:zTemp00
    clx
ClearVramBlock_at0:
    st1 #$00
    st2 #$00
    dex
    bne ClearVramBlock_at0
    dec z:zTemp00
    bne ClearVramBlock_at0
    plx
    inc a:OBJ_STATE,x
    rts

; @routine 00DD:0DC GlyphPassByStep
; @run 6F:A0DC
; WHAT IT DOES: Chooses how the object's glyph is drawn this step: the object's step (OBJ->f35c0)
; low two bits pick OverlayGlyphPass (steps 0 and 3) or OverlayGlyphPassBoth (steps 1 and 2).
; HOW: The two bits doubled index a table of handler addresses; the address goes to zTemp00 and
; zTemp01 and is jumped through; zTemp08 is cleared first.
; WAITS: no.
; Entry: X the object. Exit: as the handler chosen.
GlyphPassByStep:
    lda a:OBJ_WORK_G,x
    and #$03
    asl a
    tay
    lda a:GlyphPasses,y
    sta z:zTemp00
    lda a:GlyphPasses+1,y
    sta z:zTemp01
    stz z:zTemp08
    jmp (ZeroPage)

; @routine 00DD:0FA OverlayGlyphPass
; @run 6F:A0FA
; WHAT IT DOES: One pass of the object's glyph draw: OverlayGlyphRows with the column offset 0,
; then the glyph buffer shifted right a bit and OverlayGlyphRows again with the offset 1.
; HOW: Sets zBiosAl to 0, calls OverlayGlyphRows, adds one to zBiosAl, calls
; ShiftGlyphBufferRight, and runs on into OverlayGlyphRows.
; WAITS: no.
; Entry: X the object. Exit: as OverlayGlyphRows.
OverlayGlyphPass:
    stz z:zBiosAl
    jsr OverlayGlyphRows
    inc z:zBiosAl
    jsr ShiftGlyphBufferRight

; @routine 00DD:104 OverlayGlyphRows
; @run 6F:A104
; WHAT IT DOES: Draws a glyph into the tiles of an object's row by overlaying it on what is in
; VRAM: two places one glyph number apart are set, the first has the glyph buffer's low bytes
; ORed in, the second its high bytes.
; HOW: The object's step (OBJ->f35c0) picks a glyph number from a table. SetGlyphVramPlace for
; it, OrBufferIntoVramLow, then SetGlyphVramPlace for the number plus one and a loop of 16 words
; that reads each word, writes it back and ORs the buffer's odd bytes (from $3B41) into the high
; byte.
; WAITS: no.
; Entry: X the object. Exit: A the last high byte written, X kept (N and Z from it), C set, Y the
; object's kind doubled, V from SetGlyphVramPlace.
OverlayGlyphRows:
    ldy a:OBJ_WORK_G,x
    lda a:GlyphNumbers,y
    jsr SetGlyphVramPlace
    jsr OrBufferIntoVramLow
    ldy a:OBJ_WORK_G,x
    lda a:GlyphNumbers,y
    inc a
    jsr SetGlyphVramPlace
    phx
    clx
OverlayGlyphRows_at0:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$20
    bne OverlayGlyphRows_at0
    plx
    rts

; @routine 00DD:133 OrBufferIntoVramLow
; @run 6F:A133
; WHAT IT DOES: Merges the glyph buffer into the VRAM words from the read address on: the low
; byte of each of 16 words has the buffer's bytes ORed in (every other byte, from $3B40).
; HOW: Sixteen times the low byte is read, ORed with the buffer byte and written, then the high
; byte is read and written back.
; WAITS: no.
; Entry: X kept on the stack across it (a typed caller passes its own); the VDC's read and write
; addresses set. Exit: A the last high byte, X kept with N and Z from it, C set.
OrBufferIntoVramLow:
    phx
    clx
OrBufferIntoVramLow_at0:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$20
    bne OrBufferIntoVramLow_at0
    plx
    rts

; @routine 00DD:14C OverlayGlyphPassBoth
; @run 6F:A14C
; WHAT IT DOES: One pass of the object's glyph draw with both bytes overlaid: as OverlayGlyphPass
; with OverlayGlyphRowsBoth in place of OverlayGlyphRows.
; HOW: Sets zBiosAl to 0, calls OverlayGlyphRowsBoth, adds one to zBiosAl, calls
; ShiftGlyphBufferRight, and runs on into OverlayGlyphRowsBoth.
; WAITS: no.
; Entry: X the object. Exit: as OverlayGlyphRowsBoth.
OverlayGlyphPassBoth:
    stz z:zBiosAl
    jsr OverlayGlyphRowsBoth
    inc z:zBiosAl
    jsr ShiftGlyphBufferRight

; @routine 00DD:156 OverlayGlyphRowsBoth
; @run 6F:A156
; WHAT IT DOES: Draws a glyph into the tiles of an object's row by overlaying it on what is in
; VRAM, both bytes of each word: the low bytes from the buffer's even bytes and the high bytes
; from the odd ones.
; HOW: The object's step (OBJ->f35c0) picks a glyph number from a table, SetGlyphVramPlace sets the
; place, then 16 words are read and written back with the buffer ORed in.
; WAITS: no.
; Entry: X the object. Exit: A the last high byte written, X kept (N and Z from it), C set, Y the
; object's kind doubled, V from SetGlyphVramPlace.
OverlayGlyphRowsBoth:
    ldy a:OBJ_WORK_G,x
    lda a:GlyphNumbers,y
    jsr SetGlyphVramPlace
    phx
    clx
OverlayGlyphRowsBoth_at0:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$20
    bne OverlayGlyphRowsBoth_at0
    plx
    rts

; @routine 00DD:17B ShiftGlyphBufferRight
; @run 6F:A17B
; WHAT IT DOES: Shifts the glyph buffer's twelve words right by one bit, to move a glyph a pixel
; along, and sets the column offset zTemp08 to $10.
; HOW: Twelve times the high byte (at $3B44) is shifted right and the carry rotated into the low
; byte after it; then $10 is stored in zTemp08.
; WAITS: no.
; Entry: X kept on the stack across it (a typed caller passes its own). Exit: A $10, N and Z
; clear, C set; X kept.
ShiftGlyphBufferRight:
    phx
    clx
ShiftGlyphBufferRight_at0:
    lsr a:StageMaskBuffer+4,x
    ror a:StageMaskBuffer+5,x
    inx
    inx
    cpx #$18
    bne ShiftGlyphBufferRight_at0
    plx
    lda #$10
    sta z:zTemp08
    rts

; @routine 00DD:1A3 SetGlyphVramPlace
; @run 6F:A1A3
; WHAT IT DOES: Points the VDC's read and write addresses at a glyph's place in VRAM: the
; object's base place (from a table by the object's kind), plus the column offset zBiosAl, plus
; the glyph number times 64 (with the half-column in zTemp08 ORed in).
; HOW: The glyph number is shifted right twice into zTemp01 with the two bits shifted out kept
; as the top of zTemp00, which zTemp08 is ORed into. The base place is added to zBiosAl and
; zTemp00 (the carry of the first add is dropped, as in the original) and zTemp01, written to the
; write address, and then the same word to the read address; the data port is selected.
; WAITS: no.
; Entry: A the glyph number, X the object, zBiosAl, zTemp08. Exit: A 2, N and Z clear, C and V from
; the high byte add, Y the kind doubled; X kept. Clobbers zTemp00 to zTemp03.
SetGlyphVramPlace:
    stz z:zTemp00
    lsr a
    ror z:zTemp00
    lsr a
    ror z:zTemp00
    sta z:zTemp01
    lda z:zTemp00
    ora z:zTemp08
    sta z:zTemp00
    lda a:OBJ_START_ARG,x
    asl a
    tay
    jsr VecSelectVramWriteAddress
    lda a:GlyphPlaces,y
    clc
    adc z:zBiosAl
    clc
    adc z:zTemp00
    sta z:zTemp02
    sta a:VDC_DATA_LO
    lda a:GlyphPlaces+1,y
    adc z:zTemp01
    sta z:zTemp03
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zTemp02
    sta a:VDC_DATA_LO
    lda z:zTemp03
    sta a:VDC_DATA_HI
    jmp VecSelectVramData

; @routine 00DD:1E3 GlideObjectUp
; @run 6F:A1E3
; WHAT IT DOES: One frame of a short slide: on every other frame the object moves up one pixel
; (its Y goes down by one), the step counter counts down, and at zero the object goes on to its
; next state.
; HOW: The counter (OBJ->f35c0) read and shifted right gives the odd or even frame; on an even
; one OBJ->y_lo is decremented; the counter is decremented and at zero OBJ->f27a8 is incremented.
; WAITS: no.
; Entry: X the object. Exit: A the counter shifted right, X kept, C the counter's bit 0, N and Z
; from the counter after the decrement (from the new state when it reached zero).
GlideObjectUp:
    lda a:OBJ_WORK_G,x
    lsr a
    bcs GlideObjectUp_at0
    dec a:OBJ_Y_LO,x
GlideObjectUp_at0:
    dec a:OBJ_WORK_G,x
    bne GlideObjectUp_at1
    inc a:OBJ_STATE,x
GlideObjectUp_at1:
    rts

; @routine 00DD:1F5 WaitThenStartGlide
; @run 6F:A1F5
; WHAT IT DOES: Waits out the object's two-byte count, then starts the object's glide (file_select.c).
; HOW: As the C's header.
; WAITS: no.
; Entry: X the object.
WaitThenStartGlide:
    dec a:OBJ_WORK_E,x
    bne WaitThenStartGlide_done
    dec a:OBJ_WORK_F,x
    bpl WaitThenStartGlide_done
    cpx #$2E
    bne WaitThenStartGlide_set
    phx
    lda #$06
    ldx #$07
    jsr VecStartScriptSlotKind2
    plx
WaitThenStartGlide_set:
    lda #$10
    sta a:OBJ_WORK_G,x
    inc a:OBJ_STATE,x
WaitThenStartGlide_done:
    rts

; @routine 00DD:215 GlideObjectDown
; @run 6F:A215
; WHAT IT DOES: One frame of a short slide down: on every other frame the object moves down one
; pixel, the step counter counts down, and at zero the object is cleared away.
; HOW: The counter (OBJ->f35c0) read and shifted right gives the odd or even frame; on an even
; one OBJ->y_lo is incremented; the counter is decremented and at zero ClearObject empties the
; slot.
; WAITS: no.
; Entry: X the object. Exit: A the counter shifted right, X kept, C the counter's bit 0, N and Z
; from the counter after the decrement, Y kept.
GlideObjectDown:
    lda a:OBJ_WORK_G,x
    lsr a
    bcs GlideObjectDown_at0
    inc a:OBJ_Y_LO,x
GlideObjectDown_at0:
    dec a:OBJ_WORK_G,x
    bne WaitThenStartGlide_done
    jmp VecClearObject

; @routine 00DD:50A PlaceObjectList
; @run 6F:A50A
; WHAT IT DOES: Puts a list of scene objects on the screen: each is cleared and given a place and
; a picture frame, all the same X high byte, in consecutive slots from the one the list names.
; HOW: A (doubled) picks the list's address from a table. The list's first byte is the first slot
; (kept in X), its second the common X high byte (in zTemp08); then entries of three bytes (X low,
; Y low, frame) until an X low of zero. The pointer is zTemp00 and zTemp01; ClearObject empties
; each slot, whose kind of picture (OBJ->f2a10) is set to 4.
; WAITS: no.
; Entry: A the list. Exit: A 0, Z set, N clear; X the slot after the last placed; Y 2 (1 for an
; empty list); C and V from the last pointer add.
PlaceObjectList:
    asl a
    tay
    lda a:ObjectLists,y
    sta z:zTemp00
    lda a:ObjectLists+1,y
    sta z:zTemp01
    lda (zTemp00)
    tax
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp08
    lda z:zTemp00
    clc
    adc #$02
    sta z:zTemp00
    bcc PlaceObjectList_at0
    inc z:zTemp01
PlaceObjectList_at0:
    jsr VecClearObject
    lda (zTemp00)
    beq PlaceObjectList_at1
    sta a:OBJ_X_LO,x
    lda z:zTemp08
    sta a:OBJ_X_HI,x
    ldy #$01
    lda (zTemp00),y
    sta a:OBJ_Y_LO,x
    iny
    lda (zTemp00),y
    sta a:OBJ_FRAME,x
    lda #$04
    sta a:OBJ_SPRITE_SET,x
    lda z:zTemp00
    clc
    adc #$03
    sta z:zTemp00
    bcc JumpByKindTable
    inc z:zTemp01
    inx
    bra PlaceObjectList_at0
PlaceObjectList_at1:
    rts

; @routine 00DD:699 SlotToWorkArea
; @run 6F:A699
; WHAT IT DOES: Copies the chosen save slot's bytes that the file menu shows into a work area
; ($3F40 up): the first byte, the two at $1D and $1E, and four groups of five bytes (at $01,
; $06, $0B and $10) as four runs of five.
; HOW: The slot's offset into the backup file comes from a table; the three single bytes are
; copied, then a loop of five takes one byte from each of the four groups at a time.
; WAITS: no.
; Entry: nothing. Exit: A the last byte copied, X the slot offset plus 5, Y 5, Z and C set, N
; clear.
SlotToWorkArea:
    ldy z:zSaveSlot
    ldx a:SlotOffsets,y
    lda a:SaveBuffer,x
    sta a:ProgressCharacter
    lda a:SaveContinues,x
    sta a:ContinuesLo
    lda a:SaveContinues+1,x
    sta a:ContinuesHi
    cly
SlotToWorkArea_at0:
    lda a:SaveBuffer+1,x
    sta a:StagesReached,y
    lda a:SaveBuffer+6,x
    sta a:StagesReached+5,y
    lda a:SaveBuffer+11,x
    sta a:StagesVisited,y
    lda a:SaveBuffer+16,x
    sta a:StagesVisited+5,y
    inx
    iny
    cpy #$05
    bne SlotToWorkArea_at0
    rts

; @routine 00DD:7D2 DrawSlotName
; @run 6F:A7D2
; WHAT IT DOES: Writes the eight characters of a save slot's name to the screen at the place the
; menu has for the row.
; HOW: The row (doubled) picks the place from a table; the row is reduced by repeated subtraction
; of 3 to the slot; the VDC's write address is set; the eight characters from the slot's name
; (at $15 into the slot) go out as tiles with palette 1.
; WAITS: no.
; Entry: A the row, X kept on the stack across it (a typed caller passes its own). Exit: A the
; last character, Y the slot's offset plus 8, X kept (N and Z from it), C set, V as the last
; subtraction left it (kept when the row is under 3).
DrawSlotName:
    phx
    pha
    asl a
    tay
    pla
DrawSlotName_at1:
    cmp #$03
    bcc DrawSlotName_at0
    sbc #$03
    bra DrawSlotName_at1
DrawSlotName_at0:
    tax
    jsr VecSelectVramWriteAddress
    lda a:TextRowAddresses,y
    sta a:VDC_DATA_LO
    lda a:TextRowAddresses+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    ldy a:SlotOffsets,x
    clx
DrawSlotName_at2:
    lda a:SaveBuffer+21,y
    sta a:VDC_DATA_LO
    st2 #$11
    iny
    inx
    cpx #$08
    bne DrawSlotName_at2
    plx
    rts

; @routine 00DE:006 DrawSlotPercent
; @run 6F:A806
; WHAT IT DOES: Writes the completion of a save slot as a number of up to three digits at the
; place the menu has for the row: "100" or a blank and two digits.
; HOW: The row is kept doubled (the place table's index) and reduced by repeated subtraction of 3
; to the slot (0 to 2); the VRAM write address is set from the table, SlotCompletionPercent
; works the number out in zTemp08 and zTemp09, and a 1 digit tile, or a blank tile, and the low
; byte as two hex digits are written.
; WAITS: no.
; Entry: A the row, X kept on the stack across it (a typed caller passes its own). Exit: A the
; last digit tile, N and Z from X, C bit 3 of the low byte written, V as the last subtraction left it
; (kept when the row is under 3), X kept, Y the slot.
DrawSlotPercent:
    phx
    pha
    asl a
    tay
    pla
DrawSlotPercent_at1:
    cmp #$03
    bcc DrawSlotPercent_at0
    sbc #$03
    bra DrawSlotPercent_at1
DrawSlotPercent_at0:
    tax
    jsr VecSelectVramWriteAddress
    lda a:PercentAddresses,y
    sta a:VDC_DATA_LO
    lda a:PercentAddresses+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    sxy
    phx
    phy
    jsr SlotCompletionPercent
    ply
    plx
    lda z:zTemp09
    beq DrawSlotPercent_at2
    lda #$01
    jsr WriteDigitTile
    cla
    bra DrawSlotPercent_at3
DrawSlotPercent_at2:
    st1 #$20
    st2 #$11
    lda z:zTemp08
DrawSlotPercent_at3:
    jsr WriteHexByteTiles
    plx
    rts

; @routine 00DE:05D BlankTextRow
; @run 6F:A85D
; WHAT IT DOES: Blanks a row of eight tiles of the menu's text: each tile's number becomes $3D
; and its palette byte stays.
; HOW: The row number doubled picks the row's VRAM address from a table, which is set as both
; the write and the read address; eight times the tile's low byte is read, $3D is written in its
; place, the high byte is read and written back.
; WAITS: no.
; Entry: A the row, X kept on the stack across it (a typed caller passes its own). Exit: A the
; last high byte, Y the row doubled, X kept with N and Z from it (the PLX), C set.
BlankTextRow:
    phx
    asl a
    tay
    jsr VecSelectVramWriteAddress
    lda a:TextRowAddresses,y
    sta a:VDC_DATA_LO
    lda a:TextRowAddresses+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda a:TextRowAddresses,y
    sta a:VDC_DATA_LO
    lda a:TextRowAddresses+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    clx
    lda a:VDC_DATA_LO
    st1 #$3D
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    inx
    cpx #$08
    bne DrawTilePiece072A
    plx
    rts

; @routine 00DE:0B2 RecolorTextRun
; @run 6F:A8B2
; WHAT IT DOES: Gives a run of the menu's text a palette: from the row's VRAM address on, each tile
; that has a number is rewritten with the palette byte, until a tile whose high nibble is 3 ends
; the run; tiles whose high nibble is 0 are rewritten as they are.
; HOW: The palette byte is looked up by Y and kept in $F8. The row (doubled) picks the VRAM
; address, set as read and write address. Each pass reads a word, keeps its high byte in $F9, and
; writes the word back with its own high byte, or with $F8 as the high byte, or stops.
; WAITS: no.
; Entry: A the row, Y the palette index. Exit: A $30, Y the last tile's low byte, Z and C set, N
; clear; X kept. Clobbers zBiosAl and zBiosAh.
RecolorTextRun:
    pha
    lda a:TextPalettes,y
    sta z:zBiosAl
    pla
    asl a
    tay
    jsr VecSelectVramReadAddress
    lda a:TextColorAddresses,y
    sta a:VDC_DATA_LO
    lda a:TextColorAddresses+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    lda a:TextColorAddresses,y
    sta a:VDC_DATA_LO
    lda a:TextColorAddresses+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    bra RecolorTextRun_at0
RecolorTextRun_at1:
    lda z:zBiosAh
    sty a:VDC_DATA_LO
    sta a:VDC_DATA_HI
RecolorTextRun_at0:
    ldy a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta z:zBiosAh
    and #$F0
    beq RecolorTextRun_at1
    cmp #$30
    beq RecolorTextRun_at2
    lda z:zBiosAl
    sty a:VDC_DATA_LO
    sta a:VDC_DATA_HI
    bra RecolorTextRun_at0
RecolorTextRun_at2:
    rts

; @routine 00DE:153 DrawSlotDetails
; @run 6F:A953
; WHAT IT DOES: Writes the chosen save slot's continues and face (DrawSlotContinuesAndFace) and
; its score total, six digits, at the screen place $022E.
; HOW: DrawSlotContinuesAndFace; the write address set to $022E; the three score bytes (at $1F to
; $21 into the slot, highest first) as hex digit pairs, the last by running on into
; WriteHexByteTiles.
; WAITS: no.
; Entry: nothing. Exit: A the last digit tile, N and Z from it, C bit 3 of the last score byte, X the
; slot's offset, Y the slot.
DrawSlotDetails:
    bsr DrawSlotContinuesAndFace
    jsr VecSelectVramWriteAddress
    st1 #$2E
    st2 #$02
    jsr VecSelectVramData
    ldy z:zSaveSlot
    ldx a:SlotOffsets,y
    lda a:SaveScoreTotal+2,x
    bsr WriteHexByteTiles
    lda a:SaveScoreTotal+1,x
    bsr WriteHexByteTiles
    lda a:SaveScoreTotal,x

; @routine 00DE:171 WriteHexByteTiles
; @run 6F:A971
; WHAT IT DOES: Writes a byte to the screen as two hex digit tiles, high digit first.
; HOW: Keeps A on the stack, writes the high nibble as a digit tile, pulls A and runs on into
; WriteDigitTile for the low nibble.
; WAITS: no.
; Entry: A the byte; the VDC's write address set. Exit: A the low digit's tile number, the flags as the last OR left them (N and Z clear), C bit 3 of the byte (the last
; shift's); X and Y kept.
WriteHexByteTiles:
    pha
    lsr a
    lsr a
    lsr a
    lsr a
    bsr WriteDigitTile
    pla

; @routine 00DE:179 WriteLowDigitTile
; @run 6F:A979
; WHAT IT DOES: Writes the low nibble of A to the screen as a digit tile.
; HOW: Masks A with $0F and runs on into WriteDigitTile.
; WAITS: no.
; Entry: A the value; the VDC's write address set. Exit: as WriteDigitTile.
WriteLowDigitTile:
    and #$0F

; @routine 00DE:17B WriteDigitTile
; @run 6F:A97B
; WHAT IT DOES: Writes one digit tile to the screen: the digit's tile number is $10 over the
; digit, with palette 1 in the high byte.
; HOW: ORs $10 into A, writes it to the VDC's data port and then $11 to its high byte.
; WAITS: no.
; Entry: A the digit (0 to $F, or any value to be ORed with $10); the VDC's write address set.
; Exit: A the value ORed with $10, N and Z from it, C kept, X and Y kept.
WriteDigitTile:
    ora #$10
    sta a:VDC_DATA_LO
    st2 #$11
    rts

; @routine 00DE:183 DrawSlotContinuesAndFace
; @run 6F:A983
; WHAT IT DOES: Writes the chosen save slot's continues left (two digits, from its $1D and $1E
; bytes) at the screen place $06AD, and the character's face tile pair, at $0229.
; HOW: The write address is set to $06AD; the byte at $1E goes out as one digit and the byte at $1D
; as two; the write address is set to $0229 and the character (at $7DE3 up, by the slot) picks a
; pair of tile numbers from a table, written with palette 1.
; WAITS: no.
; Entry: nothing. Exit: A the second tile number, X the slot's offset, Y the table index, N and Z
; from A, C bit 7 of the character.
DrawSlotContinuesAndFace:
    jsr VecSelectVramWriteAddress
    st1 #$AD
    st2 #$06
    jsr VecSelectVramData
    ldy z:zSaveSlot
    ldx a:SlotOffsets,y
    lda a:SaveContinues+1,x
    bsr WriteLowDigitTile
    lda a:SaveContinues,x
    bsr WriteHexByteTiles
    jsr VecSelectVramWriteAddress
    st1 #$29
    st2 #$02
    jsr VecSelectVramData
    ldy z:zSaveSlot
    lda a:MenuSlotProgress,y
    asl a
    tay
    lda a:FaceTiles,y
    sta a:VDC_DATA_LO
    st2 #$11
    lda a:FaceTiles+1,y
    sta a:VDC_DATA_LO
    st2 #$11
    rts

; @routine 00DE:1D8 EventFlagsClear
; @run 6F:A9D8
; WHAT IT DOES: Forgets every event: the 32 bytes of event flags go to zero.
; HOW: A loop from byte 31 down to 0.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear; A, Y and C kept.
EventFlagsClear:
    ldx #$1F
EventFlagsClear_at0:
    stz a:EventFlags,x
EventFlagsClear_at1:
    dex
    bpl EventFlagsClear_at0
    rts

; @routine 00DE:1E1 EventBitClear
; @run 6F:A9E1
; WHAT IT DOES: Takes one event out of the flags the save remembers.
; HOW: EventBitMask gives the byte and bit; the byte is read, ANDed with the bit's complement and
; written back.
; WAITS: no.
; Entry: A the event number. Exit: A the byte written, N and Z from it, C clear, Y the flag byte
; index; X kept.
EventBitClear:
    bsr EventBitMask
    eor #$FF
    sxy
    and a:EventFlags,x
    sta a:EventFlags,x
    sxy
    rts

; @routine 00DE:1EE EventBitSet
; @run 6F:A9EE
; WHAT IT DOES: Puts one event into the flags the save remembers.
; HOW: EventBitMask gives the byte and bit; the byte is read, ORed with the bit and written back.
; WAITS: no.
; Entry: A the event number. Exit: as EventBitClear.
EventBitSet:
    bsr EventBitMask
    sxy
    ora a:EventFlags,x
    sta a:EventFlags,x
    sxy
    rts

; @routine 00DE:1F9 EventBitTest
; @run 6F:A9F9
; WHAT IT DOES: Tells whether one event is in the flags the save remembers.
; HOW: EventBitMask gives the byte and bit; the byte is read and ANDed with the bit.
; WAITS: no.
; Entry: A the event number. Exit: A the bit if the event is set, else 0, N and Z from it, C
; clear, Y the flag byte index; X kept.
EventBitTest:
    bsr EventBitMask
    sxy
    and a:EventFlags,x
    sxy
    rts

; @routine 00DE:201 EventBitMask
; @run 6F:AA01
; WHAT IT DOES: Turns an event number into the byte of the event flags that holds it and the bit
; in that byte.
; HOW: The number is kept on the stack while its byte (over 8) is worked out, then pulled; the bit
; is 1 shifted left by the number's low three bits. The original counts the shifts down in $FE (one more than the bits, then built one place too
; high and rotated back so that 7 ends on $80); the counter is left at zero.
; WAITS: no.
; Entry: A the event number. Exit: A the bit, N and Z from it, C clear, Y the byte; X kept;
; zBiosDl zero.
EventBitMask:
    pha
    lsr a
    lsr a
    lsr a
    tay
    pla
    and #$07
    sta z:zBiosDl
    inc z:zBiosDl
    lda #$01
EventBitMask_at0:
    asl a
    dec z:zBiosDl
    bne EventBitMask_at0
    ror a
    rts

; @routine 00DE:216 SlotCompletionPercent
; @run 6F:AA16
; WHAT IT DOES: Works out how complete a save slot is, as a decimal number for the file menu: two
; for each bit set in the slot's first byte, and four for each bit set in the OR of its five
; pairs of bytes at offsets $0B and $10.
; HOW: Decimal mode on. The slot's offset into the backup file is read from a table; each byte's
; set bits are counted by CountSetBits; the sum is built in zTemp08 (low) and zTemp09 (high) with
; the count added once for the first byte twice, and four times for each of the five pairs (the
; pair's count kept in zTemp0B between the four adds, five pairs counted in zTemp0A). Decimal
; mode off.
; WAITS: no. CountSetBits's arm that counts a bit is in no trace and the draft stops there, so the
; comparison now runs with every counted byte zero (curated); the sums with counts were compared
; when the harness still ran the lifted CountSetBits on both sides: 400 of 400, decimal and not.
; Entry: Y the slot (0 to 2). Exit: A zTemp09, C from the last decimal add, N clear and Z set (the
; pair counter reached zero), V as the caller left it, D clear; X the slot's offset plus 5; Y the
; last count.
SlotCompletionPercent:
    sed
    ldx a:SlotOffsets,y
    lda a:SaveBuffer,x
    jsr CountSetBits
    sta z:zTemp08
    clc
    adc z:zTemp08
    sta z:zTemp08
    stz z:zTemp09
    lda #$05
    sta z:zTemp0A
SlotCompletionPercent_at0:
    lda a:SaveBuffer+11,x
    ora a:SaveBuffer+16,x
    jsr CountSetBits
    sta z:zTemp0B
    clc
    adc z:zTemp08
    sta z:zTemp08
    cla
    adc z:zTemp09
    sta z:zTemp09
    lda z:zTemp0B
    clc
    adc z:zTemp08
    sta z:zTemp08
    cla
    adc z:zTemp09
    sta z:zTemp09
    lda z:zTemp0B
    clc
    adc z:zTemp08
    sta z:zTemp08
    cla
    adc z:zTemp09
    sta z:zTemp09
    lda z:zTemp0B
    clc
    adc z:zTemp08
    sta z:zTemp08
    cla
    adc z:zTemp09
    sta z:zTemp09
    inx
    dec z:zTemp0A
    bne SlotCompletionPercent_at0
    cld
    rts

; @routine 00DE:26D CountSetBits
; @run 6F:AA6D
; WHAT IT DOES: Counts the bits set in a byte.
; HOW: Shifts the byte right eight times, counting each carry out.
; WAITS: no. The arm that counts a set bit (an INY) is in no script's trace: the draft stops
; there, so the comparison does not reach it.
; Entry: A the byte, X kept on the stack across it (a typed caller passes its own X). Exit: A and
; Y the count, N and Z from X (the PLX that gives it back is the last to set them), C the byte's
; bit 7; X kept.
CountSetBits:
    cly
    phx
    ldx #$08
CountSetBits_at1:
    lsr a
    bcc CountSetBits_at0
    iny
CountSetBits_at0:
    dex
    bne CountSetBits_at1
    tya
    plx
    rts

; @routine 00DE:7A3 SlotLastClearedFlag
; @run 6F:AFA3
; WHAT IT DOES: Finds the highest of the 13 things SlotClearedFlags marks that the chosen slot
; has, or 0 if it has none.
; HOW: SlotClearedFlags, then a scan of its bytes from the 13th down for the first that is not
; zero.
; WAITS: no.
; Entry: nothing. Exit: X the number found (0 to 12; 0 too when none), A the byte read last (zero when
; none), N and Z from the read (from the last decrement of X when none), C set, V from
; SlotClearedFlags; Y as it left it.
SlotLastClearedFlag:
    bsr SlotClearedFlags
    ldx #$0C
SlotLastClearedFlag_at1:
    lda a:StageVramBuffer,x
    bne SlotLastClearedFlag_at0
    dex
    bpl SlotLastClearedFlag_at1
    clx
SlotLastClearedFlag_at0:
    rts

; @routine 00DE:7B1 SlotClearedFlags
; @run 6F:AFB1
; WHAT IT DOES: For the chosen save slot, marks which of 13 things it has: a byte for each, in a
; list that ends in a zero byte, set when the slot has any of the bits a table names for it.
; HOW: For each of the 13, clears its byte, reads the two slot bytes the table's offset names (at
; $01 and $06 into the slot), ORs them, ANDs the table's mask and adds one to the byte if
; anything is left; then clears the byte after the last.
; WAITS: no. The arm that adds one to a byte (an INC) is in no script's trace: the draft stops
; there, so the comparison does not reach it (it is compared with every mask zero).
; Entry: nothing. Exit: A the last AND, X 13, Y the last slot byte index, Z and C set, N clear, V
; from the last offset add (the slot's offset plus the table's).
SlotClearedFlags:
    clx
SlotClearedFlags_at1:
    stz a:StageVramBuffer,x
    ldy z:zSaveSlot
    lda a:SlotOffsets,y
    clc
    adc a:ClearedTableOffsets,x
    tay
    lda a:SaveBuffer+1,y
    ora a:SaveBuffer+6,y
    and a:ClearedTableMasks,x
    beq SlotClearedFlags_at0
    inc a:StageVramBuffer,x
SlotClearedFlags_at0:
    inx
    cpx #$0D
    bne SlotClearedFlags_at1
    stz a:StageVramBuffer,x
    rts

; @routine 00E0:3DF StartScriptSlotKind6B
; @run 6F:BBDF
; WHAT IT DOES: Starts a script on a script slot as kind 6.
; HOW: ClearScriptSlot, then the slot's kind and script bytes. The body is that of
; StartScriptSlotKind6 in the kernel (near copy). liftcmp compares nothing past its first call: the
; draft stubs the call to the copy of ClearScriptSlot at $BBF1.
; WAITS: no.
; Entry: X the slot, Y the script. Exit: A the script, N and Z from it; X and Y kept.
StartScriptSlotKind6B:
    bsr ClearScriptSlotB
    lda #$06
    bra NearObject19

; @routine 00E0:3E5 StartScriptSlotKind7B
; @run 6F:BBE5
; WHAT IT DOES: Starts a script on a script slot as kind 7.
; HOW: ClearScriptSlot, then the slot's kind and script bytes. Near copy of the kernel's
; StartScriptSlotKind7; the draft stubs the call to the copy of ClearScriptSlot at $BBF1.
; WAITS: no.
; Entry: X the slot, Y the script. Exit: A the script, N and Z from it; X and Y kept.
StartScriptSlotKind7B:
    bsr ClearScriptSlotB
    lda #$07
    sta a:ScriptSlotKind,x
    tya
    sta a:ScriptSlotScript,x
    rts

; @routine 00E0:3F1 ClearScriptSlotB
; @run 6F:BBF1
; WHAT IT DOES: Empties a script slot (the overlay's copy of ClearScriptSlot).
; HOW: Zeroes its first eight fields, 12 bytes apart.
; WAITS: no.
; Entry: X slot. Exit: all registers and flags kept.
ClearScriptSlotB:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts
