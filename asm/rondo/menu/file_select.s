; @routine 00C1:228 IncrementTempWord
; @run 6B:8228
; WHAT IT DOES: Adds one to the 16-bit pointer in zTemp00 and zTemp01, the place the text
; printer reads its next byte from.
; HOW: Adds one to the low byte, and to the high byte when the low byte wrapped to zero.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the byte last incremented, C and the registers kept.
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
WriteGlyphRows_at1:
    lda a:StageMaskBuffer,x
    sta a:VDC_DATA_LO
    st2 #$FF
    inx
    inx
WriteGlyphRows_at0:
    iny
    cpy #$08
    bne WriteGlyphRows_at1
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
    stz a:$7DFA
    ldx #$07
    lda #$18
PadRepeatReset_at0:
    sta a:$7DF2,x
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
    dec a:$7DF2,x
    clc
    bne PadRepeatUpdate_at3
    lda #$0C
    sec
    bra PadRepeatUpdate_at1
PadRepeatUpdate_at2:
    cla
    clc
PadRepeatUpdate_at1:
    sta a:$7DF2,x
PadRepeatUpdate_at3:
    ror a:$7DFA
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
    lda a:$7DF1
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
    lda a:$7DFF
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
    sta a:OBJ_34B8
    sta a:OBJ_34B8+1
    jmp SetCursorTargets

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
    sta a:OBJ_2A10,x
    txa
    lsr a
    pla
    sta a:OBJ_34B8,x
    rol a
    asl a
    tay
    lda a:$AD1F,y
    sta a:OBJ_X_LO,x
    lda a:$AD20,y
    sta a:OBJ_Y_LO,x
    rts

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
; the object is marked as on its way (OBJ->f3510 is 1).
; WAITS: no.
; Entry: X the cursor object (0 or 1). Exit: A 1, N and Z clear, C bit 6 of the cursor value, Y
; the table index. X kept.
SetCursorTarget:
    txa
    lsr a
    lda a:OBJ_34B8,x
    rol a
    asl a
    tay
    lda a:$AD1F,y
    sta a:OBJ_3568,x
    lda a:$AD20,y
    sta a:OBJ_35C0,x
    lda #$01
    sta a:OBJ_3510,x
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
    sta a:OBJ_2A68,x
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
    ldx a:$9252,y
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
    lda a:$7D0B,x
    sta a:StagesVisited,y
    lda a:$7D10,x
    sta a:StagesVisited+5,y
    inx
    iny
    cpy #$05
    bne SlotToWorkArea_at0
    rts

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
    ldx a:$9252,y
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
    lda a:$7D0B,x
    ora a:$7D10,x
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
SlotCompletionPercent_at1:
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
CountSetBits_at0:
    lsr a
    bcc CountSetBits_at1
    iny
CountSetBits_at1:
    dex
    bne CountSetBits_at0
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
    lda a:ListChannelClearEnd+1,x
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
    stz a:ListChannelClearEnd+1,x
    ldy z:zSaveSlot
    lda a:$9252,y
    clc
    adc a:$AFD6,x
    tay
    lda a:SaveBuffer+1,y
    ora a:SaveBuffer+6,y
    and a:$AFE3,x
    beq SlotClearedFlags_at0
    inc a:ListChannelClearEnd+1,x
SlotClearedFlags_at0:
    inx
    cpx #$0D
    bne SlotClearedFlags_at1
    stz a:ListChannelClearEnd+1,x
    rts
