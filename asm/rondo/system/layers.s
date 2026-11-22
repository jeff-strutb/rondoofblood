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
    lda a:$3EB9
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
    adc a:$3CA0,x
    sta a:$3CA0,x
    cla
    adc a:$3CA8,x
    sta a:$3CA8,x
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

; @routine 12D6:263 ScreenMapOfEntryC
; @run 6F:A6DA
; WHAT IT DOES: ScreenMapOfEntry, as the stage at 1291 has its own copy of it.
; HOW: The same body, shared through screen_map_of_entry.
; WAITS: no.
; Entry, exit: as ScreenMapOfEntry.
ScreenMapOfEntryC:
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
