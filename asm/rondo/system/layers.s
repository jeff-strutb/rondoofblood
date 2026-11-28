; @routine 0515:12F HalveScrollStepFirstToSecond
; @run 6F:A12F
; WHAT IT DOES: Sets entry 1 of this frame's X scroll step to half of entry 0's.
; HOW: Entry 0 to entry 1 through HalveScrollStep. The three bytes load Y and clear X, then
; run on into it.
; WAITS: no.
; Entry: nothing. Exit: as HalveScrollStep, X 0 and Y 1.
HalveScrollStepFirstToSecond:
    ldy #$01
    clx

; @routine 0515:132 HalveScrollStep
; @run 6F:A132
; WHAT IT DOES: Sets one entry of this frame's X scroll step to half another's, so a layer
; behind the first moves at half its speed.
; HOW: A signed 16-bit shift right: the high byte keeps its sign (the first read of it
; only gives the sign, the second is shifted), and the bit it loses goes into the low byte's
; top. The high byte is written before the low byte is read.
; WAITS: no.
; Entry: X the entry to halve, Y the entry that gets the result. Exit: A the low byte written, N
; and Z from it, C the low bit shifted out. X and Y kept.
HalveScrollStep:
    lda a:ScrollStepXHi,x
    asl a
    lda a:ScrollStepXHi,x
    ror a
    sta a:ScrollStepXHi,y
    lda a:ScrollStepXLo,x
    ror a
    sta a:ScrollStepXLo,y
    rts

; @routine 07D5:131 HalveScrollStepFromFirst
; @run 6F:A131
; WHAT IT DOES: Sets entry Y of this frame's X scroll step to half of entry 0's.
; HOW: Entry 0 into HalveScrollStep. The one byte is a CLX that runs on into it.
; WAITS: no.
; Entry: Y the entry that gets the result. Exit: as HalveScrollStep, X 0.
HalveScrollStepFromFirst:
    clx

; @routine 0255:067 ClearScrollSpeeds
; @run 6F:A067
; WHAT IT DOES: Stops every layer's scroll: the eight X speeds and the eight Y speeds go to zero.
; HOW: A loop from entry 7 down to 0, the X column then the Y column each time.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear. A and C kept.
ClearScrollSpeeds:
    ldx #$07
ClearScrollSpeeds_at0:
    stz a:ScrollSpeedX,x
    stz a:ScrollSpeedY,x
    dex
    bpl ClearScrollSpeeds_at0
    rts

; @routine 10C5:3F7 ClearScrollSpeedsB
; @run 6F:A3F7
; WHAT IT DOES: ClearScrollSpeeds, as the stage at 10C1 has its own copy of it.
; HOW: The same body, shared through clear_scroll_speeds.
; WAITS: no.
; Entry, exit: as ClearScrollSpeeds.
ClearScrollSpeedsB:
    ldx #$07
ClearScrollSpeedsB_at0:
    stz a:ScrollSpeedX,x
    stz a:ScrollSpeedY,x
    dex
    bpl ClearScrollSpeedsB_at0
    rts

; @routine 12D5:3F0 ClearScrollSpeedsC
; @run 6F:A3F0
; WHAT IT DOES: ClearScrollSpeeds, as the stage at 1291 has its own copy of it.
; HOW: The same body, shared through clear_scroll_speeds.
; WAITS: no.
; Entry, exit: as ClearScrollSpeeds.
ClearScrollSpeedsC:
    ldx #$07
ClearScrollSpeedsC_at0:
    stz a:ScrollSpeedX,x
    stz a:ScrollSpeedY,x
    dex
    bpl ClearScrollSpeedsC_at0
    rts

; @routine 0255:122 ScrollYFromLevelWord
; @run 6F:A122
; WHAT IT DOES: Takes the level's own scroll Y word as the scroll Y the game wants.
; HOW: Two bytes copied, low then high, each read then written.
; WAITS: no.
; Entry: nothing. Exit: A the high byte, N and Z from it. X, Y, C and V kept.
; Names: the source word has no name past its first byte (RasterWaterWanted holds it in the
; symbols); an enum of this file stands for both bytes until the owner adds them.
ScrollYFromLevelWord:
    lda a:RasterWaterWanted
    sta a:ScrollYLo
    lda a:LevelYHi
    sta a:ScrollYHi
    rts

; @routine 00C5:133 AddToListWord
; @run 6C:8133
; WHAT IT DOES: Adds a byte to entry X of a list's 16-bit counter, carrying into the high byte.
; HOW: The low byte is read and written, then the high byte gets the carry.
; WAITS: no.
; Entry: A the amount, X the entry. Exit: A the high byte, N Z C V from its add. X, Y kept.
; Names: the two columns ($3CA0, $3CA8) have no name yet, so the enum of this file gives them.
AddToListWord:
    clc
    adc a:ListWordLo,x
    sta a:ListWordLo,x
    cla
    adc a:ListWordHi,x
    sta a:ListWordHi,x
    rts

; @routine 0515:6DA ScreenMapOfEntry
; @run 6F:A6DA
; WHAT IT DOES: Reads the level map's byte for the screen entry X of the scroll stands on.
; HOW: Its column goes to zTemp00, its row is loaded, and the eight bytes run on into ScreenMapAt.
; WAITS: no.
; Entry: X the entry. Exit: as ScreenMapAt.
ScreenMapOfEntry:
    lda a:ScrollXHiScreen,x
    sta z:zTemp00
    lda a:ScrollYHiScreen,x

; @routine 10C6:26A ScreenMapOfEntryB
; @run 6F:AA6A
; WHAT IT DOES: ScreenMapOfEntry, as the stage at 1081 has its own copy of it.
; HOW: The same body, shared through screen_map_of_entry.
; WAITS: no.
; Entry, exit: as ScreenMapOfEntry.
ScreenMapOfEntryB:
    lda a:ScrollXHiScreen,x
    sta z:zTemp00
    lda a:ScrollYHiScreen,x

; @routine 0255:6E2 ScreenMapAt
; @run 6F:A6E2
; WHAT IT DOES: Reads the level map's byte for the screen at row A, column zTemp00.
; HOW: A shifted up four bits and or'd with zTemp00 is the place; it goes back to zTemp00 with
; $3D in zTemp01 as a pointer, and the byte is read through it.
; WAITS: no.
; Entry: A the row, zTemp00 the column. Exit: A the byte, N and Z from it, C and V clear.
; X and Y kept. Clobbers zTemp00 and zTemp01.
ScreenMapAt:
    asl a
    asl a
    asl a
    asl a
    ora z:zTemp00
    clc
    adc #$00
    sta z:zTemp00
    lda #$3D
    sta z:zTemp01
    lda (zTemp00)
    rts

; @routine 0256:180 ScrollXAddWrap
; @run 6F:A980
; WHAT IT DOES: Adds A to entry X of the scroll X the game wants, and moves its screen back by
; two plus the carry, wrapping within sixteen screens.
; HOW: The low byte is read and written; the level position's screen byte gets $FE and the carry,
; is masked to four bits, and goes to both ScrollXHiScreen and ScrollXHi.
; WAITS: no.
; Entry: A the amount, X the entry. Exit: A the screen written, N and Z from it, C and V from the
; add of $FE. X, Y kept.
ScrollXAddWrap:
    clc
    adc a:ScrollXLo,x
    sta a:ScrollXLo,x
    lda a:ScrollXHiScreen,x
    adc #$FE
    and #$0F
    sta a:ScrollXHiScreen,x
    sta a:ScrollXHi,x
    rts

; @routine 0255:073 ScrollSpeedsFromBands
; @run 6F:A073
; WHAT IT DOES: Sets each layer's scroll speed this frame to how far the game's wanted scroll is
; from the band's: X and Y, for all eight entries.
; HOW: From entry 7 down to 0, the speed along X is ScrollXLo less ScrollBgxLo, then the speed
; along Y is ScrollYLo less ScrollBgyLo, each a subtract of the low bytes with carry set.
; WAITS: no.
; Entry: nothing. Exit: A the last Y speed, X $FF, N set, Z clear, C and V from its subtract. Y kept.
ScrollSpeedsFromBands:
    ldx #$07
ScrollSpeedsFromBands_at0:
    lda a:ScrollXLo,x
    sec
    sbc a:ScrollBgxLo,x
    sta a:ScrollSpeedX,x
    lda a:ScrollYLo,x
    sec
    sbc a:ScrollBgyLo,x
    sta a:ScrollSpeedY,x
    dex
    bpl ScrollSpeedsFromBands_at0
    rts

; @routine 0256:078 BitmapBitShifted
; @run 6F:A878
; WHAT IT DOES: Looks up bit (Y and 7) of the bitmap's byte Y/8 and leaves it in the carry. The
; bitmap is read through the pointer in zTemp0C.
; HOW: The byte goes to zTemp0E and is shifted right (Y and 7) plus one times; the last bit out is
; the carry. X and Y are saved on the stack and put back.
; WAITS: no.
; Entry: Y the bit number, zTemp0C the bitmap. Exit: A Y and 7; C the bit; N and Z from X; X and Y
; kept. Clobbers zTemp0E.
BitmapBitShifted:
    phy
    tya
    lsr a
    lsr a
    lsr a
    and #$07
    tay
    lda (zTemp0C),y
    sta z:zTemp0E
    ply
    tya
    and #$07
    phx
    tax
    inx
BitmapBitShifted_at0:
    lsr z:zTemp0E
    dex
    bne BitmapBitShifted_at0
    plx
    rts

; @routine 00C5:27F CopyWordsToVdc
; @run 6C:827F
; WHAT IT DOES: Writes X blocks of sixteen words from the pointer in zTemp12 to the VDC's data
; port, moving the pointer on a block at a time.
; HOW: Each word is its low byte to the data port's low half, its high byte to the high half,
; read through the pointer by Y. The pointer then goes up by $20 (the carry into its high byte).
; WAITS: no.
; Entry: X the number of blocks (0 is 256), zTemp12 the source, the VDC set to write. Exit: A the
; pointer's high byte, X 0, Y $20, Z set, C and V from that last add.
; Names: the pointer's high byte ($2013) is by address until the records name it.
CopyWordsToVdc:
    cly
CopyWordsToVdc_at0:
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_HI
    iny
    cpy #$20
    bne CopyWordsToVdc_at0
    tya
    clc
    adc z:zTemp12
    sta z:zTemp12
    cla
    adc z:zProduct0
    sta z:zProduct0
    dex
    bne CopyWordsToVdc
    rts

; @routine 0255:75C ScrollXPlusOffset
; @run 6F:A75C
; WHAT IT DOES: The scroll X of entry X plus an offset from a table of the overlay (word Y), as a
; point on the level: the low byte to zTemp02, the screen (kept to 4 bits) to zTemp03.
; HOW: The offset's low byte is added to ScrollXLo, and its high byte with the carry to
; ScrollXHiScreen. When the switch at $3EBA is set the low byte comes from the word before.
; WAITS: no.
; Entry: X the entry, Y the table index (a word). Exit: A the screen, N and Z from it, C and V from
; its add. X and Y kept. The switch's arm is untraced (00C5 bytes $5..$D of this routine).
; Names: the switch is by address until the records name it.
ScrollOffsetSwitch = $3EBA ; not zero: the entry's offsets come from the word before (ScrollXPlusOffset)
ScrollXPlusOffset:
    lda a:ScrollOffsetSwitch
    beq ScrollXPlusOffset_at0
    lda a:ScrollXLo,x
    clc
    adc a:LayerPlaceOffsetsLoAlt,y
    bra ScrollXPlusOffset_at1
ScrollXPlusOffset_at0:
    lda a:ScrollXLo,x
    clc
    adc a:LayerPlaceOffsetsLo,y
ScrollXPlusOffset_at1:
    sta z:zTemp02
    lda a:ScrollXHiScreen,x
    adc a:LayerPlaceOffsetsHi,y
    and #$0F
    sta z:zTemp03
    rts

; @routine 10C6:2EC ScrollXPlusOffsetB
; @run 6F:AAEC
; WHAT IT DOES: ScrollXPlusOffset with the offsets of the stage at 1081.
; HOW: The body is scroll_plus_offset's; the table is at $AB10.
; WAITS: no.
; Entry, exit: as ScrollXPlusOffset.
ScrollXPlusOffsetB:
    lda a:$3EBA
    beq ScrollXPlusOffsetB_at0
    lda a:ScrollXLo,x
    clc
    adc a:$AB0E,y
    bra ScrollXPlusOffsetB_at1
ScrollXPlusOffsetB_at0:
    lda a:ScrollXLo,x
    clc
    adc a:$AB10,y
ScrollXPlusOffsetB_at1:
    sta z:zTemp02
    lda a:ScrollXHiScreen,x
    adc a:$AB12,y
    and #$0F
    sta z:zTemp03
    rts

; @routine 12D6:2E5 ScrollXPlusOffsetC
; @run 6F:AAE5
; WHAT IT DOES: ScrollXPlusOffset with the offsets of the stage at 1291.
; HOW: The body is scroll_plus_offset's; the table is at $AB09.
; WAITS: no.
; Entry, exit: as ScrollXPlusOffset.
ScrollXPlusOffsetC:
    lda a:$3EBA
    beq ScrollXPlusOffsetC_at0
    lda a:ScrollXLo,x
    clc
    adc a:$AB07,y
    bra ScrollXPlusOffsetC_at1
ScrollXPlusOffsetC_at0:
    lda a:ScrollXLo,x
    clc
    adc a:$AB09,y
ScrollXPlusOffsetC_at1:
    sta z:zTemp02
    lda a:ScrollXHiScreen,x
    adc a:$AB0B,y
    and #$0F
    sta z:zTemp03
    rts

; @routine 0726:092 ScrollYPlusOffset
; @run 6F:A892
; WHAT IT DOES: ScrollXPlusOffset along Y: ScrollYLo and ScrollYHiScreen plus the table's word,
; the result in zTemp06 and zTemp07.
; HOW: The body is scroll_plus_offset's; the table is at $A780.
; WAITS: no.
; Entry, exit: as ScrollXPlusOffset.
ScrollYPlusOffset:
    lda a:$3EBA
    beq ScrollYPlusOffset_at0
    lda a:ScrollYLo,x
    clc
    adc a:LayerPlaceOffsetsLoAlt,y
    bra ScrollYPlusOffset_at1
ScrollYPlusOffset_at0:
    lda a:ScrollYLo,x
    clc
    adc a:LayerPlaceOffsetsLo,y
ScrollYPlusOffset_at1:
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    adc a:LayerPlaceOffsetsHi,y
    and #$0F
    sta z:zTemp07
    rts

; @routine 10C6:422 ScrollYPlusOffsetB
; @run 6F:AC22
; WHAT IT DOES: ScrollYPlusOffset with the offsets of the stage at 1081.
; HOW: The body is scroll_plus_offset's; the table is at $AB10.
; WAITS: no.
; Entry, exit: as ScrollXPlusOffset.
ScrollYPlusOffsetB:
    lda a:$3EBA
    beq ScrollYPlusOffsetB_at0
    lda a:ScrollYLo,x
    clc
    adc a:$AB0E,y
    bra ScrollYPlusOffsetB_at1
ScrollYPlusOffsetB_at0:
    lda a:ScrollYLo,x
    clc
    adc a:$AB10,y
ScrollYPlusOffsetB_at1:
    sta z:zTemp06
    lda a:ScrollYHiScreen,x
    adc a:$AB12,y
    and #$0F
    sta z:zTemp07
    rts

; @routine 0255:6F4 AdvanceScrollX
; @run 6F:A6F4
; WHAT IT DOES: Moves entry X's scroll along X by its step.
; HOW: See advance_scroll. The table at $A727 gives the sign extension.
; WAITS: no.
; Entry: X the entry. Exit: A the screen, N and Z from it, C and V from its add. X kept; Y the step's sign
; (AdvanceResult).
AdvanceScrollXScrollXHiAdds = $A727 ; table AdvanceScrollX reads
AdvanceScrollX:
    lda a:ScrollStepXHi,x
    rol a
    rol a
    and #$01
    tay
    lda a:ScrollStepXLo,x
    clc
    adc a:ScrollXFrac,x
    sta a:ScrollXFrac,x
    lda a:ScrollStepXHi,x
    adc a:ScrollXLo,x
    sta a:ScrollXLo,x
    lda #$00
    adc a:AdvanceScrollXScrollXHiAdds,y
    pha
    clc
    adc a:ScrollXHi,x
    sta a:ScrollXHi,x
    pla
    clc
    adc a:ScrollXHiScreen,x
    and #$0F
    sta a:ScrollXHiScreen,x
    rts

; @routine 10C6:284 AdvanceScrollXB
; @run 6F:AA84
; WHAT IT DOES: AdvanceScrollX with the sign table of the stage at 1081 ($AAB7).
; HOW: See advance_scroll.
; WAITS: no.
; Entry, exit: as AdvanceScrollX.
AdvanceScrollXB:
    lda a:ScrollStepXHi,x
    rol a
    rol a
    and #$01
    tay
    lda a:ScrollStepXLo,x
    clc
    adc a:ScrollXFrac,x
    sta a:ScrollXFrac,x
    lda a:ScrollStepXHi,x
    adc a:ScrollXLo,x
    sta a:ScrollXLo,x
    lda #$00
    adc a:$AAB7,y
    pha
    clc
    adc a:ScrollXHi,x
    sta a:ScrollXHi,x
    pla
    clc
    adc a:ScrollXHiScreen,x
    and #$0F
    sta a:ScrollXHiScreen,x
    rts

; @routine 12D6:27D AdvanceScrollXC
; @run 6F:AA7D
; WHAT IT DOES: AdvanceScrollX with the sign table of the stage at 1291 ($AAB0).
; HOW: See advance_scroll.
; WAITS: no.
; Entry, exit: as AdvanceScrollX.
AdvanceScrollXC:
    lda a:ScrollStepXHi,x
    rol a
    rol a
    and #$01
    tay
    lda a:ScrollStepXLo,x
    clc
    adc a:ScrollXFrac,x
    sta a:ScrollXFrac,x
    lda a:ScrollStepXHi,x
    adc a:ScrollXLo,x
    sta a:ScrollXLo,x
    lda #$00
    adc a:$AAB0,y
    pha
    clc
    adc a:ScrollXHi,x
    sta a:ScrollXHi,x
    pla
    clc
    adc a:ScrollXHiScreen,x
    and #$0F
    sta a:ScrollXHiScreen,x
    rts

; @routine 0725:729 AdvanceScrollY
; @run 6F:A729
; WHAT IT DOES: Moves entry X's scroll along Y by its step.
; HOW: See advance_scroll. The table at $A727 gives the sign extension.
; WAITS: no.
; Entry, exit: as AdvanceScrollX.
AdvanceScrollY:
    lda a:ScrollStepYHi,x
    rol a
    rol a
    and #$01
    tay
    lda a:ScrollStepYLo,x
    clc
    adc a:ScrollYFrac,x
    sta a:ScrollYFrac,x
    lda a:ScrollStepYHi,x
    adc a:ScrollYLo,x
    sta a:ScrollYLo,x
    lda #$00
    adc a:$A727,y
    pha
    clc
    adc a:ScrollYHi,x
    sta a:ScrollYHi,x
    pla
    clc
    adc a:ScrollYHiScreen,x
    and #$0F
    sta a:ScrollYHiScreen,x
    rts

; @routine 10C6:2B9 AdvanceScrollYB
; @run 6F:AAB9
; WHAT IT DOES: AdvanceScrollY with the sign table of the stage at 1081 ($AAB7).
; HOW: See advance_scroll.
; WAITS: no.
; Entry, exit: as AdvanceScrollX.
AdvanceScrollYB:
    lda a:ScrollStepYHi,x
    rol a
    rol a
    and #$01
    tay
    lda a:ScrollStepYLo,x
    clc
    adc a:ScrollYFrac,x
    sta a:ScrollYFrac,x
    lda a:ScrollStepYHi,x
    adc a:ScrollYLo,x
    sta a:ScrollYLo,x
    lda #$00
    adc a:$AAB7,y
    pha
    clc
    adc a:ScrollYHi,x
    sta a:ScrollYHi,x
    pla
    clc
    adc a:ScrollYHiScreen,x
    and #$0F
    sta a:ScrollYHiScreen,x
    rts

; @routine 10C6:272 ScreenMapAtB
; @run 6F:AA72
; WHAT IT DOES: ScreenMapAt, as the stage at 1081 has it, run at $AA72.
; HOW: The same bytes as ScreenMapAt at another run address; its C is screen_map_at.
; WAITS: no.
; Entry, exit: as ScreenMapAt.
ScreenMapAtB:
    asl a
    asl a
    asl a
    asl a
    ora z:zTemp00
    clc
    adc #$00
    sta z:zTemp00
    lda #$3D
    sta z:zTemp01
    lda (zTemp00)
    rts

