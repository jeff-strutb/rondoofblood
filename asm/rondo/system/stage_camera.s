; @routine 0255:12F ScrollStepXHalfOf0To1
; @run 6F:A12F
; WHAT IT DOES: Sets entry 1's step along X to half entry 0's.
; HOW: LDY #1, then ScrollStepXHalfOf0 by running into it.
; WAITS: no.
; Entry: nothing. Exit: as ScrollStepXHalf, X 0, Y 1.
ScrollStepXHalfOf0To1:
    ldy #$01

; @routine 0255:131 ScrollStepXHalfOf0
; @run 6F:A131
; WHAT IT DOES: Sets entry Y's step along X to half entry 0's.
; HOW: CLX, then ScrollStepXHalf by running into it.
; WAITS: no.
; Entry: Y the entry set. Exit: as ScrollStepXHalf, X 0.
ScrollStepXHalfOf0:
    clx

; @routine 0255:132 ScrollStepXHalf
; @run 6F:A132
; WHAT IT DOES: Sets entry Y's step along X to half entry X's, its sign kept.
; HOW: ASL of the high byte for its sign, then ROR of the high byte and of the low byte into entry Y.
; WAITS: no.
; Entry: X the entry halved, Y the one set. Exit: A the low byte written, N and Z from it, C its bit shifted out. X, Y
; and V kept.
ScrollStepXHalf:
    lda a:ScrollStepXHi,x
    asl a
    lda a:ScrollStepXHi,x
    ror a
    sta a:ScrollStepXHi,y
    lda a:ScrollStepXLo,x
    ror a
    sta a:ScrollStepXLo,y
    rts

; @routine 0255:145 CameraMode1B
; @run 6F:A145
; WHAT IT DOES: Camera mode $1B: camera mode 0 with columns of eight blocks at scroll Y 0; entry 1 at scroll Y $BF
; with an X step of -$40, moved.
; HOW: CameraMode00Set; STZ ScrollYLo; the stores; the tail JMP to AdvanceScrollX with X 1.
; WAITS: no.
; Entry: nothing. Exit: as AdvanceScrollX of entry 1, X 1.
CameraMode1B:
    lda #$08
    cly
    jsr CameraMode00Set
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    lda #$BF
    sta a:ScrollYLo+1
    lda #$C0
    sta a:ScrollStepXLo+1
    lda #$FF
    sta a:ScrollStepXHi+1
    ldx #$01
    jmp AdvanceScrollX

; @routine 0255:26F CameraMode14
; @run 6F:A26F
; WHAT IT DOES: Camera mode $14: camera mode 0 with columns of eight blocks at scroll Y 0, entry 4's scroll Y high 1,
; then the parallax steps.
; HOW: CameraMode00Set; STZ ScrollYLo; the stores; the tail JMP to CameraParallaxSteps.
; WAITS: no.
; Entry: nothing. Exit: as CameraParallaxSteps.
CameraMode14:
    lda #$08
    cly
    jsr CameraMode00Set
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    lda #$01
    sta a:ScrollYHi+4
    jmp CameraParallaxSteps

; @routine 0255:38D CameraMode11
; @run 6F:A38D
; WHAT IT DOES: Camera mode $11: entry 0 moves along X, then the parallax steps.
; HOW: AdvanceScrollX of entry 0, the tail JMP to CameraParallaxSteps.
; WAITS: no.
; Entry: nothing. Exit: as CameraParallaxSteps.
CameraMode11:
    clx
    jsr AdvanceScrollX
    jmp CameraParallaxSteps

; @routine 0255:3C5 CameraMode0F
; @run 6F:A3C5
; WHAT IT DOES: Camera mode $0F: as mode $0D, then entry 2's X step low $40 and the high steps of entries 3 to 6 1, 4,
; 8 and $0A, and the parallax entries moved.
; HOW: AdvanceScrollX, ScrollYFromLevelWord, STZ ScrollYOffset, five stores, the tail JMP to CameraParallaxAdvance.
; WAITS: no.
; Entry: nothing. Exit: as CameraParallaxAdvance.
CameraMode0F:
    clx
    jsr AdvanceScrollX
    jsr ScrollYFromLevelWord
    stz a:ScrollYOffset
    lda #$40
    sta a:ScrollStepXLo+2
    lda #$01
    sta a:ScrollStepXHi+3
    lda #$04
    sta a:ScrollStepXHi+4
    lda #$08
    sta a:ScrollStepXHi+5
    lda #$0A
    sta a:ScrollStepXHi+6
    jmp CameraParallaxAdvance

; @routine 0255:3EB CameraMode0D
; @run 6F:A3EB
; WHAT IT DOES: Camera mode $0D: entry 0 moves along X, its scroll Y comes from the level word, and the fixed Y offset
; is cleared.
; HOW: AdvanceScrollX of entry 0, ScrollYFromLevelWord, STZ ScrollYOffset.
; WAITS: no.
; Entry: nothing. Exit: A the level word's high byte, N and Z from it, X 0, Y the step's sign, C and V from the X add.
CameraMode0D:
    clx
    jsr AdvanceScrollX
    jsr ScrollYFromLevelWord
    stz a:ScrollYOffset
    rts

; @routine 0255:623 CameraMode0C
; @run 6F:A623
; WHAT IT DOES: Camera mode $0C: entry 0 in columns of eight blocks at scroll Y 0, then the parallax.
; HOW: STA ScrollColumnBlocks of 8, CLA (no flags), BRA into CameraBandsY.
; WAITS: no.
; Entry: nothing. Exit: as CameraBandsY.
CameraMode0C:
    lda #$08
    sta a:ScrollColumnBlocks
    cla
    bra CameraBandsY

; @routine 0255:62B CameraMode0B
; @run 6F:A62B
; WHAT IT DOES: Camera mode $0B: as mode $0A with columns of four blocks.
; HOW: LDA #4, BRA into CameraBandsBlocks.
; WAITS: no.
; Entry: nothing. Exit: as CameraBandsY.
CameraMode0B:
    lda #$04
    bra CameraBandsBlocks

; @routine 0255:62F CameraMode0A
; @run 6F:A62F
; WHAT IT DOES: Camera mode $0A: entry 0 in columns of three blocks at scroll Y $60, then the parallax.
; HOW: LDA #3, running into CameraBandsBlocks.
; WAITS: no.
; Entry: nothing. Exit: as CameraBandsY.
CameraMode0A:
    lda #$03

; @routine 0255:631 CameraBandsBlocks
; @run 6F:A631
; WHAT IT DOES: Entry 0's column blocks set, then CameraBandsY with scroll Y $60.
; HOW: STA ScrollColumnBlocks, LDA #$60, running into CameraBandsY.
; WAITS: no.
; Entry: A the blocks. Exit: as CameraBandsY.
CameraBandsBlocks:
    sta a:ScrollColumnBlocks
    lda #$60

; @routine 0255:636 CameraBandsY
; @run 6F:A636
; WHAT IT DOES: Camera mode 0 run with entry 0's scroll Y set for it, then cleared, entry 0's column blocks 8, and the
; parallax steps.
; HOW: STA ScrollYLo; CameraMode00; STZ ScrollYLo; then CameraParallaxSteps by running into it.
; WAITS: no.
; Entry: A the scroll Y's low byte. Exit: as CameraParallaxSteps.
CameraBandsY:
    sta a:ScrollYLo
    jsr CameraMode00
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks

; @routine 0255:644 CameraParallaxSteps
; @run 6F:A644
; WHAT IT DOES: Sets the parallax steps from entry 0's: entries 1, 2 and 3 each half the one before, entries 4, 5 and 6
; entries 1, 2 and 3 plus $40, $20 and $10 in the low byte; then moves them all.
; HOW: ScrollStepXHalfOf0To1, the adds with CLA, ADC of the high bytes, ScrollStepXHalf twice; CameraParallaxAdvance by
; running into it.
; WAITS: no.
; Entry: nothing. Exit: as CameraParallaxAdvance.
CameraParallaxSteps:
    jsr ScrollStepXHalfOf0To1
    lda a:ScrollStepXLo+1
    clc
    adc #$40
    sta a:ScrollStepXLo+4
    cla
    adc a:ScrollStepXHi+1
    sta a:ScrollStepXHi+4
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf
    lda a:ScrollStepXLo+2
    clc
    adc #$20
    sta a:ScrollStepXLo+5
    cla
    adc a:ScrollStepXHi+2
    sta a:ScrollStepXHi+5
    ldx #$02
    ldy #$03
    jsr ScrollStepXHalf
    lda a:ScrollStepXLo+3
    clc
    adc #$10
    sta a:ScrollStepXLo+6
    cla
    adc a:ScrollStepXHi+3
    sta a:ScrollStepXHi+6

; @routine 0255:685 CameraParallaxAdvance
; @run 6F:A685
; WHAT IT DOES: Moves the parallax entries 1 to 6 along X by their steps.
; HOW: AdvanceScrollX for X 1 to 6, the last by a tail JMP.
; WAITS: no.
; Entry: nothing. Exit: as AdvanceScrollX of entry 6, X 6.
CameraParallaxAdvance:
    ldx #$01
    jsr AdvanceScrollX
    inx
    jsr AdvanceScrollX
    inx
    jsr AdvanceScrollX
    inx
    jsr AdvanceScrollX
    inx
    jsr AdvanceScrollX
    inx
    jmp AdvanceScrollX

; @routine 0255:69E CameraMode00Set
; @run 6F:A69E
; WHAT IT DOES: Sets entry 0's column blocks and scroll Y, then camera mode 0.
; HOW: Two stores, then CameraMode00 by running into it.
; WAITS: no.
; Entry: A the blocks a column, Y the scroll Y's low byte. Exit: as CameraMode00.
CameraMode00Set:
    sta a:ScrollColumnBlocks
    sty a:ScrollYLo

; @routine 0255:6A4 CameraMode00
; @run 6F:A6A4
; @stop A6AF
; WHAT IT DOES: Camera mode 0: entry 0 moves along X by its step, the map byte of its screen is kept, and the column
; it crosses is drawn.
; HOW: AdvanceScrollX of entry 0; ScreenMapAtEntry into CameraScreenByte; the tail JMP to ColumnDue with the step's
; sign in Y.
; WAITS: no.
; Entry: nothing. Exit: as ColumnDue, X 0.
CameraMode00:
    clx
    bsr AdvanceScrollX
    bsr ScreenMapAtEntry
    sta a:CameraScreenByte
    jmp ColumnDue

; @routine 0255:6DA ScreenMapAtEntry
; @run 6F:A6DA
; WHAT IT DOES: Reads the level map's byte for the screen layer entry X has reached.
; HOW: ScrollXHiScreen into zTemp00 as the column, ScrollYHiScreen as the row, then ScreenMapAt by running into it.
; WAITS: no.
; Entry: X the entry. Exit: as ScreenMapAt: A the byte, N and Z from it, C 0, V 0. X and Y kept.
ScreenMapAtEntry:
    lda a:ScrollXHiScreen,x
    sta z:zTemp00
    lda a:ScrollYHiScreen,x

; @routine 0255:784 ColumnDueLeft
; @run 6F:A784
; WHAT IT DOES: For an entry moving left: when one less than its scroll X and one less than its band's have bit 5
; different, it crossed a column, which is drawn.
; HOW: The two values less one in zTemp09 and A, EOR, AND #$20; the BNE and BRA to DrawBlockColumn.
; WAITS: no.
; Entry: X the entry, Y its index for DrawBlockColumn. Exit: not crossed: A 0, N 0, Z 1, X, Y, C and V kept; else as
; DrawBlockColumn.
ColumnDueLeft:
    lda a:ScrollBgxLo,x
    dec a
    sta z:zTemp09
    lda a:ScrollXLo,x
    dec a
    eor z:zTemp09
    and #$20
    bne ColumnDueLeft_at0
    rts
ColumnDueLeft_at0:
    bra DrawBlockColumn

; @routine 0255:797 ColumnDue
; @run 6F:A797
; WHAT IT DOES: Draws the column an entry has crossed, if it has: moving left (Y zero) as ColumnDueLeft; else when
; its scroll X and its band's differ in bit 5.
; HOW: TYA; EOR of the two, AND #$20; the BNE to DrawBlockColumn.
; WAITS: no.
; Entry: X the entry, Y 0 moving left, 1 moving right. Exit: not crossed: A 0, N 0, Z 1, X, Y, C and V kept; else as
; DrawBlockColumn.
ColumnDue:
    tya
    beq ColumnDueLeft
    lda a:ScrollXLo,x
    eor a:ScrollBgxLo,x
    and #$20
    bne DrawBlockColumn
    rts

