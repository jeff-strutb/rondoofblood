; @routine 0255:08D CameraStep
; @run 6F:A08D
; WHAT IT DOES: The camera's frame: the layers' speeds cleared, the section's camera mode run, the speeds taken from
; the bands, and in the camera modes 4, 5, 7, 9, $0D, $13, $15, $16 and $1C the player moved against the scroll
; along Y.
; HOW: ClearScrollSpeeds, StageCameraModeDispatch, ScrollSpeedsFromBands, a CMP chain on CameraMode read once; the
; speed's negative added to the player's Y, sign extended through zTemp00.
; WAITS: no.
; Entry: nothing. Exit: another mode: A CameraMode, N, Z and C from the last compare, X, Y and V as the calls left
; them; moving: A the player's Y high byte, N, Z, C and V from its add, Y the negated speed.
CameraStep:
    jsr ClearScrollSpeeds
    jsr StageCameraModeDispatch
    jsr ScrollSpeedsFromBands
    lda a:CameraMode
    cmp #$04
    beq CameraStep_at0
    cmp #$05
    beq CameraStep_at0
    cmp #$07
    beq CameraStep_at0
    cmp #$09
    beq CameraStep_at0
    cmp #$0D
    beq CameraStep_at0
    cmp #$13
    beq CameraStep_at0
    cmp #$15
    beq CameraStep_at0
    cmp #$16
    beq CameraStep_at0
    cmp #$1C
    beq CameraStep_at0
    rts
CameraStep_at0:
    stz z:zTemp00
    lda a:ScrollSpeedY
    eor #$FF
    inc a
    tay
    bpl CameraStep_at1
    dec z:zTemp00
CameraStep_at1:
    clc
    adc a:OBJ_Y_LO
    sta a:OBJ_Y_LO
    lda z:zTemp00
    adc a:OBJ_Y_HI
    sta a:OBJ_Y_HI
    rts

; @routine 0255:0DB StageCameraModeDispatch
; @run 6F:A0DB
; @stop A0E6
; WHAT IT DOES: Runs the section's camera mode; a mode of $80 or more does nothing.
; HOW: CameraMode doubled; the bit out of the double returns; else TAX and the jump through CameraModes.
; WAITS: no.
; Entry: nothing. Exit, mode $80 up: A the mode doubled, N and Z from it, C 1; else as the mode leaves them, entered
; with A and X the mode doubled, C 0.
StageCameraModeDispatch:
    lda a:CameraMode
    asl a
    bcc StageCameraModeDispatch_at0
    rts
StageCameraModeDispatch_at0:
    tax
    jmp (CameraModes,x)

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

; @routine 0255:167 CameraMode19
; @run 6F:A167
; WHAT IT DOES: Camera mode $19: camera mode 0 with three blocks a column at scroll Y $40; entries 4, 5 and 6's X steps are entries 1, 2 and 3's less $40, $20 and $10; entries 1 and 2 are moved with six blocks a column at their own scroll Y and their columns drawn; then entries 3 to 6 move.
; HOW: CameraMode00, ScrollStepXHalfOf0To1, the subtracts with the borrow into the high bytes, ScrollStepXHalf, AdvanceScrollX and ColumnDue, the last AdvanceScrollX by a tail JMP.
; WAITS: no.
; Entry: nothing. Exit: as AdvanceScrollX of entry 6, X 6.
CameraMode19:
    lda #$03
    sta a:ScrollColumnBlocks
    lda #$40
    sta a:ScrollYLo
    jsr CameraMode00
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To1
    lda a:ScrollStepXLo+1
    sec
    sbc #$40
    sta a:ScrollStepXLo+4
    lda a:ScrollStepXHi+1
    sbc #$00
    sta a:ScrollStepXHi+4
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf
    lda a:ScrollStepXLo+2
    sec
    sbc #$20
    sta a:ScrollStepXLo+5
    lda a:ScrollStepXHi+2
    sbc #$00
    sta a:ScrollStepXHi+5
    ldx #$02
    ldy #$03
    jsr ScrollStepXHalf
    lda a:ScrollStepXLo+3
    sec
    sbc #$10
    sta a:ScrollStepXLo+6
    lda a:ScrollStepXHi+3
    sbc #$00
    sta a:ScrollStepXHi+6
    lda #$06
    sta a:ScrollColumnBlocks+1
    lda #$C0
    sta a:ScrollYLo+1
    ldx #$01
    jsr AdvanceScrollX
    jsr ColumnDue
    stz a:ScrollYLo+1
    lda #$06
    sta a:ScrollColumnBlocks+2
    lda #$A0
    sta a:ScrollYLo+2
    ldx #$02
    jsr AdvanceScrollX
    jsr ColumnDue
    stz a:ScrollYLo+2
    ldx #$03
    jsr AdvanceScrollX
    inx
    jsr AdvanceScrollX
    inx
    jsr AdvanceScrollX
    inx
    jmp AdvanceScrollX

; @routine 0255:1FB CameraMode17
; @run 6F:A1FB
; WHAT IT DOES: Camera mode $17: entry 0 moves along both axes with the column and the row it crosses drawn, and ScrollYOffset takes its scroll Y.
; HOW: AdvanceScrollX, ColumnDue, AdvanceScrollY, RowDueAt with the registers AdvanceScrollY leaves.
; WAITS: no.
; Entry: nothing. Exit: A ScrollYLo, N and Z from it; X 0, the rest as RowDueAt left them.
CameraMode17:
    clx
    jsr AdvanceScrollX
    jsr ColumnDue
    clx
    jsr AdvanceScrollY
    jsr RowDueAt
    lda a:ScrollYLo
    sta a:ScrollYOffset
    rts

; @routine 0255:210 CameraMode16
; @run 6F:A210
; WHAT IT DOES: Camera mode $16: entry 2's screen from entry 0's, its low byte crept along every fourth wave count, then camera mode $15.
; HOW: TST of zWaveCounter (V from its bit 6), the INC and AND of entry 2's ScrollXLo; running into CameraMode15.
; WAITS: no.
; Entry: nothing. Exit: as CameraMode15.
CameraMode16:
    lda a:ScrollXHi
    sta a:ScrollXHi+2
    tst #$03,z:zWaveCounter
    bne CameraMode15
    lda a:ScrollXLo+2
    inc a
    and #$3F
    sta a:ScrollXLo+2

; @routine 0255:224 CameraMode15
; @run 6F:A224
; WHAT IT DOES: Camera mode $15: the water wave, camera mode 0 at scroll Y 0, the scroll Y from the level word, and entry 0's screen row rounded up to an even one.
; HOW: CameraWaterWave, STZs, CameraMode00, ScrollYFromLevelWord, INC and AND of ScrollYHiScreen.
; WAITS: no.
; Entry: nothing. Exit: A the screen row written, N and Z from it; C, V and Y as ScrollYFromLevelWord and the calls left them.
CameraMode15:
    jsr CameraWaterWave
    stz a:ScrollYLo
    stz a:ScrollYHi
    jsr CameraMode00
    jsr ScrollYFromLevelWord
    lda a:ScrollYHiScreen
    inc a
    and #$FE
    sta a:ScrollYHiScreen
    rts

; @routine 0255:23D CameraWaterWave
; @run 6F:A23D
; WHAT IT DOES: Moves the water line the game wants on a wave: $0C00 scaled by the sine of the wave phase less $40, its high byte plus $0C into the word at RasterWaterWanted, and the phase counted on.
; HOW: zTemp00-01 the value, ScaleBySine with the phase plus $C0, zProduct2 read twice for the sign and the sum, the phase's add.
; WAITS: no.
; Entry: nothing. Exit: A the new phase, N, Z, C and V from its add; Y as ScaleBySine left it (0). X kept.
CameraWaterWave:
    lda #$00
    sta z:zTemp00
    lda #$0C
    sta z:zTemp01
    lda a:CameraWavePhase
    clc
    adc #$C0
    jsr VecScaleBySine
    stz z:zTemp00
    lda z:zPlaneRowsLeft
    bpl CameraWaterWave_at0
    dec z:zTemp00
CameraWaterWave_at0:
    lda z:zPlaneRowsLeft
    clc
    adc #$0C
    sta a:RasterWaterWanted
    lda z:zTemp00
    adc #$00
    sta a:LevelYHi
    lda #$01
    clc
    adc a:CameraWavePhase
    sta a:CameraWavePhase
    rts

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

; @routine 0255:285 CameraMode13
; @run 6F:A285
; WHAT IT DOES: Camera mode $13: the parallax steps of entries 3 to 5 halved from entry 0's, entries 2 and 1 the sums of their pairs, entry 1's plus $10; camera mode 0 with three blocks a column at scroll Y $A0, entry 2 moved and drawn, the scroll Y from the level word, and entry 1 moved.
; HOW: ScrollStepXHalfOf0 and ScrollStepXHalf, two-byte adds, CameraMode00Set, AdvanceScrollX, ColumnDue, ScrollYFromLevelWord, the tail JMP.
; WAITS: no.
; Entry: nothing. Exit: as AdvanceScrollX of entry 1, X 1.
CameraMode13:
    ldy #$03
    jsr ScrollStepXHalfOf0
    ldx #$03
    ldy #$04
    jsr ScrollStepXHalf
    ldx #$04
    ldy #$05
    jsr ScrollStepXHalf
    lda a:ScrollStepXLo+4
    clc
    adc a:ScrollStepXLo+5
    sta a:ScrollStepXLo+2
    lda a:ScrollStepXHi+4
    adc a:ScrollStepXHi+5
    sta a:ScrollStepXHi+2
    lda a:ScrollStepXLo+3
    clc
    adc a:ScrollStepXLo+5
    sta a:ScrollStepXLo+1
    lda a:ScrollStepXHi+3
    adc a:ScrollStepXHi+5
    sta a:ScrollStepXHi+1
    lda a:ScrollStepXLo+1
    clc
    adc #$10
    sta a:ScrollStepXLo+1
    cla
    adc a:ScrollStepXHi+1
    sta a:ScrollStepXHi+1
    lda #$03
    ldy #$A0
    jsr CameraMode00Set
    ldx #$02
    lda #$05
    sta a:ScrollColumnBlocks+2
    jsr AdvanceScrollX
    jsr ColumnDue
    jsr ScrollYFromLevelWord
    lda #$01
    sta a:ScrollYHi
    lda a:ScrollYLo
    sta a:ScrollYLo+2
    ldx #$01
    jmp AdvanceScrollX

; @routine 0255:2F5 CameraMode12
; @run 6F:A2F5
; WHAT IT DOES: Camera mode $12: entries 3 and 4's steps from entries 0 and 6's, entry 4 with 6's added again, entry 6 stopped; entry 1 half of 0 and entries 5 and 2 halves again; camera mode 0 at its own rows, then entries 3, 1, 4 and 2 moved with their columns.
; HOW: Two-byte adds, ScrollStepXHalfOf0To1 and ScrollStepXHalf, CameraMode00Set, AdvanceScrollX and ColumnDue, the last by a tail JMP.
; WAITS: no.
; Entry: nothing. Exit: as ColumnDue of entry 2.
CameraMode12:
    lda a:ScrollStepXLo
    clc
    adc a:ScrollStepXLo+6
    sta a:ScrollStepXLo+3
    sta a:ScrollStepXLo+4
    lda a:ScrollStepXHi
    adc a:ScrollStepXHi+6
    sta a:ScrollStepXHi+3
    sta a:ScrollStepXHi+4
    lda a:ScrollStepXLo+4
    clc
    adc a:ScrollStepXLo+6
    sta a:ScrollStepXLo+4
    lda a:ScrollStepXHi+4
    adc a:ScrollStepXHi+6
    sta a:ScrollStepXHi+4
    stz a:ScrollStepXLo+6
    stz a:ScrollStepXHi+6
    jsr ScrollStepXHalfOf0To1
    lda #$3F
    sta a:ScrollYLo+1
    lda #$01
    sta a:ScrollYHi+1
    ldx #$01
    ldy #$05
    jsr ScrollStepXHalf
    ldx #$05
    ldy #$02
    jsr ScrollStepXHalf
    lda a:ScrollStepXLo+5
    clc
    adc a:ScrollStepXLo+2
    sta a:ScrollStepXLo+2
    lda a:ScrollStepXHi+5
    adc a:ScrollStepXHi+2
    sta a:ScrollStepXHi+2
    lda #$02
    ldy #$C0
    jsr CameraMode00Set
    ldx #$03
    lda #$04
    sta a:ScrollColumnBlocks+3
    lda #$40
    sta a:ScrollYLo+3
    jsr AdvanceScrollX
    jsr ColumnDue
    lda #$47
    sta a:ScrollYLo+3
    ldx #$01
    jsr AdvanceScrollX
    ldx #$04
    jsr AdvanceScrollX
    ldx #$02
    lda #$02
    sta a:ScrollColumnBlocks+2
    stz a:ScrollYLo
    jsr AdvanceScrollX
    jmp ColumnDue

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

; @routine 0255:394 CameraMode10
; @run 6F:A394
; WHAT IT DOES: Camera mode $10: camera mode 0 with eight blocks a column at scroll Y 0; entry 1 half of 0, entries 4, 2, 5, 3 and 6 set, then the parallax entries moved.
; HOW: CameraMode00Set, ScrollStepXHalfOf0To1 and ScrollStepXHalf with the low bytes of entries 4 to 6 stored between, the tail JMP to CameraParallaxAdvance.
; WAITS: no.
; Entry: nothing. Exit: as CameraParallaxAdvance.
CameraMode10:
    lda #$08
    cly
    jsr CameraMode00Set
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To1
    lda #$80
    sta a:ScrollStepXLo+4
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf
    lda #$40
    sta a:ScrollStepXLo+5
    ldx #$02
    ldy #$03
    jsr ScrollStepXHalf
    lda #$20
    sta a:ScrollStepXLo+6
    jmp CameraParallaxAdvance

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

; @routine 0255:3F6 CameraMode03
; @run 6F:A3F6
; WHAT IT DOES: Camera mode 3: camera mode 0 with one block a column at scroll Y $C0; entry 3 half of 0 and entry 4 of 3, entry 2 their sum, entry 2 moved and drawn; entry 1 at half entry 0's scroll X, row $100, and ScrollYOffset cleared.
; HOW: CameraMode00Set, the halves, a two-byte add, AdvanceScrollX, ColumnDue, then LSR of ScrollXHi and ROR of ScrollXLo into entry 1.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z from it, C ScrollXLo's bit 0; X, Y and V as the calls left them.
CameraMode03:
    lda #$01
    ldy #$C0
    jsr CameraMode00Set
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    ldy #$03
    jsr ScrollStepXHalfOf0
    ldx #$03
    ldy #$04
    jsr ScrollStepXHalf
    lda a:ScrollStepXLo+3
    clc
    adc a:ScrollStepXLo+4
    sta a:ScrollStepXLo+2
    lda a:ScrollStepXHi+3
    adc a:ScrollStepXHi+4
    sta a:ScrollStepXHi+2
    lda #$06
    sta a:ScrollColumnBlocks+2
    ldx #$02
    jsr AdvanceScrollX
    jsr ColumnDue
    lda a:ScrollXHi
    lsr a
    sta a:ScrollXHi+1
    lda a:ScrollXLo
    ror a
    sta a:ScrollXLo+1
    stz a:ScrollYLo+1
    lda #$01
    sta a:ScrollYHi+1
    stz a:ScrollYOffset
    rts

; @routine 0255:44B CameraMode1C
; @run 6F:A44B
; WHAT IT DOES: Camera mode $1C: camera mode 0 at scroll Y $100, the scroll Y from the level word when it is not zero, and ScrollYOffset 1.
; HOW: STZ and store of the scroll Y, CameraMode00, ORA of the word's two bytes, ScrollYFromLevelWord.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z from it; X, Y, C and V as the calls left them.
CameraMode1C:
    stz a:ScrollYLo
    lda #$01
    sta a:ScrollYHi
    jsr CameraMode00
    lda a:RasterWaterWanted
    ora a:LevelYHi
    beq CameraMode1C_at0
    jsr ScrollYFromLevelWord
CameraMode1C_at0:
    lda #$01
    sta a:ScrollYOffset
    rts

; @routine 0255:467 CameraMode04
; @run 6F:A467
; WHAT IT DOES: Camera modes 4 and 5: camera mode 0 at scroll Y 0, then the scroll Y from the level word and ScrollYOffset cleared.
; HOW: Two STZs, CameraMode00, ScrollYFromLevelWord, STZ.
; WAITS: no.
; Entry: nothing. Exit: A the level word's high byte, N and Z from it; X, Y, C and V as the calls left them.
CameraMode04:
    stz a:ScrollYLo
    stz a:ScrollYHi
    jsr CameraMode00
    jsr ScrollYFromLevelWord
    stz a:ScrollYOffset
    rts

; @routine 0255:477 CameraMode06
; @run 6F:A477
; WHAT IT DOES: Camera mode 6: band 1's scroll X from entry 1's, camera mode 0, then entry 1 at half entry 0's scroll X and ScrollYOffset cleared.
; HOW: A copy to ScrollBgxLo+1, CameraMode00, LSR and ROR of the screen and low byte.
; WAITS: no.
; Entry: nothing. Exit: A the low byte written, N and Z from it, C ScrollXLo's bit 0; X, Y and V as CameraMode00 left them.
CameraMode06:
    lda a:ScrollXLo+1
    sta a:ScrollBgxLo+1
    jsr CameraMode00
    lda a:ScrollXHi
    lsr a
    sta a:ScrollXHi+1
    lda a:ScrollXLo
    ror a
    sta a:ScrollXLo+1
    stz a:ScrollYOffset
    rts

; @routine 0255:492 CameraMode07
; @run 6F:A492
; @stop A4C9
; WHAT IT DOES: Camera mode 7: camera mode 0 with the scroll Y kept, then the scroll Y by the screen entry 0 is on: below 3 at the top, 3 held until past $F8, 4 plus 4, 5 plus $84, 6 at the bottom, else ScrollYOffset takes it.
; HOW: As CameraMode00KeepY inline; CPYs of ScrollXHiScreen in A in turn; the branches into the shared pieces.
; WAITS: no.
; Entry: nothing. Exit: as the piece it goes to; past screen 6 A the scroll Y, N and Z from it, C from the compare with 6.
CameraMode07:
    lda a:ScrollYLo
    pha
    lda a:ScrollYHi
    pha
    lda a:ScrollYHiScreen
    pha
    jsr CameraMode00
    pla
    sta a:ScrollYHiScreen
    pla
    sta a:ScrollYHi
    pla
    sta a:ScrollYLo
    lda a:ScrollXHiScreen
    cmp #$03
    beq ScrollYHeldByX
    bcc ScrollYAtTop
    cmp #$04
    beq ScrollYSlideBy4
    cmp #$05
    beq ScrollYSlideBy84
    cmp #$06
    beq ScrollYAtBottom
    lda a:ScrollYLo
    sta a:ScrollYOffset
    rts

; @routine 0255:4C9 ScrollYHeldByX
; @run 6F:A4C9
; WHAT IT DOES: Holds the scroll Y at the top until entry 0's scroll X passes $F8, past which it slides down (ScrollYSlideDown).
; HOW: CMP #$F8 of ScrollXLo; the BCS, else running into ScrollYAtTop.
; WAITS: no.
; Entry: nothing. Exit: as the routine it goes to; entered with C from the compare.
ScrollYHeldByX:
    lda a:ScrollXLo
    cmp #$F8
    bcs ScrollYSlideDown

; @routine 0255:4D0 ScrollYAtTop
; @run 6F:A4D0
; WHAT IT DOES: The scroll Y and its screen cleared, with ScrollYOffset.
; HOW: Four STZs.
; WAITS: no.
; Entry: nothing. Exit: every register and flag kept.
ScrollYAtTop:
    stz a:ScrollYLo
    stz a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 0255:4DD ScrollYSlideDown
; @run 6F:A4DD
; WHAT IT DOES: The scroll Y half of entry 0's scroll X past $F8, ScrollYOffset the same, on the top screen.
; HOW: SEC, SBC #$F8, LSR into ScrollYLo and ScrollYOffset.
; WAITS: no.
; Entry: A entry 0's scroll X. Exit: A the scroll Y, N and Z from it, C bit 0 of the difference, V from the subtract. X and Y kept. Entered with A the scroll X.
ScrollYSlideDown:
    sec
    sbc #$F8
    lsr a
    sta a:ScrollYLo
    sta a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 0255:4EE ScrollYAtBottom
; @run 6F:A4EE
; WHAT IT DOES: The scroll Y $100 (screen row 1) with ScrollYOffset cleared.
; HOW: Stores of 1, STZs.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z from it; X, Y, C and V kept.
ScrollYAtBottom:
    lda #$01
    sta a:ScrollYHi
    sta a:ScrollYHiScreen
    stz a:ScrollYLo
    stz a:ScrollYOffset
    rts

; @routine 0255:4FD ScrollYSlideBy84
; @run 6F:A4FD
; WHAT IT DOES: The scroll Y half of entry 0's scroll X plus $84, at the bottom (ScrollYAtBottom) past $F8.
; HOW: CMP #$F8; the BCS to ScrollYAtBottom; LSR, CLC, ADC #$84.
; WAITS: no.
; Entry: nothing. Exit: past $F8 as ScrollYAtBottom; else A the scroll Y, N, Z, C and V from the add. X and Y kept.
ScrollYSlideBy84:
    lda a:ScrollXLo
    cmp #$F8
    bcs ScrollYAtBottom
    lsr a
    clc
    adc #$84
    sta a:ScrollYLo
    sta a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 0255:515 ScrollYSlideBy4
; @run 6F:A515
; WHAT IT DOES: The scroll Y half of entry 0's scroll X plus 4, on the top screen.
; HOW: LSR, CLC, ADC #4.
; WAITS: no.
; Entry: nothing. Exit: A the scroll Y, N, Z, C and V from the add. X and Y kept.
ScrollYSlideBy4:
    lda a:ScrollXLo
    lsr a
    clc
    adc #$04
    sta a:ScrollYLo
    sta a:ScrollYOffset
    stz a:ScrollYHi
    stz a:ScrollYHiScreen
    rts

; @routine 0255:529 CameraMode1A
; @run 6F:A529
; WHAT IT DOES: Camera mode $1A: as mode 7 with the screens four further along.
; HOW: CameraMode00KeepY, then the compares with 7 to $0A.
; WAITS: no.
; Entry: nothing. Exit: as the piece it goes to; past screen $0A A the scroll Y, N and Z from it.
CameraMode1A:
    bsr CameraMode00KeepY
    lda a:ScrollXHiScreen
    cmp #$07
    bne CameraMode1A_at0
    jmp ScrollYHeldByX
CameraMode1A_at0:
    bcc ScrollYAtTop
    cmp #$08
    beq ScrollYSlideBy4
    cmp #$09
    beq ScrollYSlideBy84
    cmp #$0A
    beq ScrollYAtBottom
    lda a:ScrollYLo
    sta a:ScrollYOffset
    rts

; @routine 0255:54A CameraMode00KeepY
; @run 6F:A54A
; WHAT IT DOES: Camera mode 0 with entry 0's scroll Y and screen row kept as they were.
; HOW: ScrollYLo, ScrollYHi and ScrollYHiScreen of entry 0 pushed, CameraMode00, pulled back in turn.
; WAITS: no.
; Entry: nothing. Exit: A entry 0's ScrollYLo, N and Z from it; X, Y, C and V as CameraMode00 left them.
CameraMode00KeepY:
    lda a:ScrollYLo
    pha
    lda a:ScrollYHi
    pha
    lda a:ScrollYHiScreen
    pha
    jsr CameraMode00
    pla
    sta a:ScrollYHiScreen
    pla
    sta a:ScrollYHi
    pla
    sta a:ScrollYLo
    rts

; @routine 0255:566 CameraMode08
; @run 6F:A566
; WHAT IT DOES: Camera mode 8: camera mode 0 with eight blocks a column at scroll Y 0; entry 1 at row $200 and half of entry 0's scroll X, with its step the distance to there, moved and drawn.
; HOW: CameraMode00Set, the row stores, ScrollXLo into the step's high byte, ROR from the screen's bit 0, SBC of entry 1's scroll X.
; WAITS: no.
; Entry: nothing. Exit: as ColumnDue of entry 1; ScrollYOffset cleared.
CameraMode08:
    lda #$08
    cly
    jsr CameraMode00Set
    lda #$02
    sta a:ScrollYHiScreen+1
    stz a:ScrollYHi+1
    lda #$10
    sta a:ScrollColumnBlocks
    lda a:ScrollXLo
    sta a:ScrollStepXHi+1
    lda a:ScrollXHiScreen
    lsr a
    ror a:ScrollStepXHi+1
    lda a:ScrollStepXHi+1
    sec
    sbc a:ScrollXLo+1
    sta a:ScrollStepXHi+1
    stz a:ScrollStepXLo+1
    lda #$04
    sta a:ScrollColumnBlocks+1
    ldx #$01
    jsr AdvanceScrollX
    jsr ColumnDue
    stz a:ScrollYOffset
    rts

; @routine 0255:5A4 CameraMode1D
; @run 6F:A5A4
; WHAT IT DOES: Camera mode $1D: camera mode 0 with eight blocks a column at scroll Y 0, then entry 1 at half entry 0's scroll X, its screen one past the half.
; HOW: Stores, CameraMode00, INC, LSR, INC of ScrollXHiScreen and ROR of ScrollXLo.
; WAITS: no.
; Entry: nothing. Exit: A the low byte written, N and Z from it, C ScrollXLo's bit 0; X, Y and V as CameraMode00 left them.
CameraMode1D:
    lda #$08
    sta a:ScrollColumnBlocks
    cla
    sta a:ScrollYLo
    jsr CameraMode00
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    lda a:ScrollXHiScreen
    inc a
    lsr a
    inc a
    sta a:ScrollXHi+1
    lda a:ScrollXLo
    ror a
    sta a:ScrollXLo+1
    rts

; @routine 0255:5C9 CameraMode0E
; @run 6F:A5C9
; WHAT IT DOES: Camera mode $0E: camera mode 0 with eight blocks a column at scroll Y 0; entry 1 at row $100, half entry 0's speed, moved and drawn.
; HOW: CameraMode00Set, the rows, ScrollStepXHalfOf0To1, AdvanceScrollX, ColumnDue.
; WAITS: no.
; Entry: nothing. Exit: as ColumnDue of entry 1; ScrollYOffset cleared.
CameraMode0E:
    lda #$08
    cly
    jsr CameraMode00Set
    lda #$01
    sta a:ScrollYHiScreen+1
    sta a:ScrollYHi+1
    lda #$10
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To1
    lda #$05
    sta a:ScrollColumnBlocks+1
    ldx #$01
    jsr AdvanceScrollX
    jsr ColumnDue
    stz a:ScrollYOffset
    rts

; @routine 0255:5F0 CameraMode09
; @run 6F:A5F0
; WHAT IT DOES: Camera mode 9: camera mode 0 with eight blocks a column at scroll Y 0; entry 2 a quarter of entry 0's speed, entries 1 and 2 at row $100 and moved, the scroll Y from the level word.
; HOW: CameraMode00Set, ScrollStepXHalf twice, AdvanceScrollX of 1 and 2 (INX between), ScrollYFromLevelWord.
; WAITS: no.
; Entry: nothing. Exit: A the level word's high byte, N and Z from it; X 2, Y, C and V as the calls left them.
CameraMode09:
    lda #$08
    cly
    jsr CameraMode00Set
    stz a:ScrollYLo
    lda #$08
    sta a:ScrollColumnBlocks
    jsr ScrollStepXHalfOf0To1
    ldx #$01
    ldy #$02
    jsr ScrollStepXHalf
    stz a:ScrollYHi
    lda #$01
    sta a:ScrollYHi+1
    sta a:ScrollYHi+2
    ldx #$01
    jsr AdvanceScrollX
    inx
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

; @routine 0255:6AF CameraMode01
; @run 6F:A6AF
; WHAT IT DOES: Camera modes 1 and $18: entry 0 moves along Y by its step, the map byte of its screen is kept, and
; the row it crosses is drawn.
; HOW: AdvanceScrollY of entry 0; ScreenMapAtEntry into CameraScreenByte; the tail JMP to RowDueAt with the step's
; sign in Y.
; WAITS: no.
; Entry: nothing. Exit: as RowDueAt, X 0.
CameraMode01:
    clx
    bsr AdvanceScrollY
    bsr ScreenMapAtEntry
    sta a:CameraScreenByte
    jmp RowDueAt

; @routine 0255:6BA CameraMode02
; @run 6F:A6BA
; WHAT IT DOES: Camera mode 2: entry 0 moves along X and along Y, the map byte of its screen is kept, and the column
; and then the row it crosses are drawn, each by the sign of its step.
; HOW: AdvanceScrollX, AdvanceScrollY, ScreenMapAtEntry into CameraScreenByte; the X step's sign (two ROLs) for
; ColumnDue, the Y step's for the tail JMP to RowDueAt.
; WAITS: no.
; Entry: nothing. Exit: as RowDueAt, X 0.
CameraMode02:
    clx
    bsr AdvanceScrollX
    bsr AdvanceScrollY
    bsr ScreenMapAtEntry
    sta a:CameraScreenByte
    lda a:ScrollStepXHi,x
    rol a
    rol a
    and #$01
    tay
    jsr ColumnDue
    lda a:ScrollStepYHi,x
    rol a
    rol a
    and #$01
    tay
    jmp RowDueAt

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

