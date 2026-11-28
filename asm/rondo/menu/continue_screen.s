; @routine 004D:000 ContinueFrame
; @run 75:8000
; @stop 800B
; WHAT IT DOES: The continue screen's frame, which ModeDead calls: runs the step of zGameSubMode, Richter's or Maria's.
; HOW: The sub-mode doubled indexes ContinueStepsRichter ($800B) or, when zCharacter is not zero, ContinueStepsMaria
; ($801A, through ContinueFrameMaria): Richter's set-up and message, or Maria's; then the fade, the clearing, the
; menu's showing and the menu, the same for both.
; WAITS: no; no step waits.
; Entry: Y the caller's (ContinueFade takes it as a script). Exit: as the step leaves them.
ContinueFrame:
    lda z:zGameSubMode
    asl a
    tax
    lda z:zCharacter
    bne ContinueFrameMaria
    jmp (ContinueStepsRichter,x)

; @routine 004D:017 ContinueFrameMaria
; @run 75:8017
; WHAT IT DOES: The continue screen's step for Maria: the JMP through ContinueStepsMaria by the doubled sub-mode.
; HOW: JMP (ContinueStepsMaria,X): the word's two bytes read, low then high, and the C of the routine there run.
; WAITS: as the step does (no step waits).
; Entry: X the doubled sub-mode. Exit: as the step leaves them.
ContinueFrameMaria:
    jmp (ContinueStepsMaria,x)

; @routine 004D:026 ContinueBegin
; @run 75:8026
; WHAT IT DOES: The continue screen's first step for either character: its music, the stage value cleared, the
; letters' VRAM ($6E00, eight rows) cleared, and zTemp10 set to the last letter.
; HOW: SoundCommand $61, STZ zRasterStage, ClearVramRows, zTemp10 7.
; WAITS: no.
; Entry: X and Y the caller's (SoundCommand is called with them). Exit: A 7, X 0, Y $6E, N and Z from 7; C and V as
; SoundCommand left them.
ContinueBegin:
    lda #$61
    jsr VecSoundCommand
    stz z:zRasterStage
    ldx #$00
    ldy #$6E
    lda #$08
    jsr ClearVramRows
    lda #$07
    sta z:zTemp10
    rts

; @routine 004D:03B ContinueStartRichter
; @run 75:803B
; WHAT IT DOES: Richter's set-up: ContinueBegin, then his eight letters as objects (frame $0D up, sprite set 5, at
; X $48, Y $59), each letter's step count twice its number, the message script started, the screen's timer set ($C0
; frames, one round) and the next sub-mode.
; HOW: zTemp10 counts the letters down from 7 and is read again for each; ClearObject; StartScriptSlot0Kind1.
; WAITS: no.
; Entry: X and Y the dispatch's (for ContinueBegin). Exit: A $C0, X 0, Y $0D, N and Z from the sub-mode's INC, C 0, V 0.
ContinueStartRichter:
    jsr ContinueBegin
ContinueStartRichter_at0:
    ldx z:zTemp10
    jsr VecClearObject
    txa
    asl a
    sta a:OBJ_34B8,x
    txa
    clc
    adc #$0D
    sta a:OBJ_FRAME,x
    lda #$05
    sta a:OBJ_SPRITE_SET,x
    lda #$48
    sta a:OBJ_X_LO,x
    lda #$59
    sta a:OBJ_Y_LO,x
    dec z:zTemp10
    bpl ContinueStartRichter_at0
    lda #$0D
    jsr VecStartScriptSlot0Kind1
    lda #$01
    sta a:OBJ_3568
    lda #$C0
    sta a:OBJ_3510
    inc z:zGameSubMode
    rts

; @routine 004D:074 ContinueWaitRichter
; @run 75:8074
; WHAT IT DOES: Richter's message, a frame: RUN once the last letter is done goes on at once; otherwise every letter
; steps, the last one's count goes from $10 to $11 (done), and once it is done the screen's timer runs (at $8087,
; which Maria's step shares); its end goes on.
; HOW: BiosJoyTrigger's RUN bit and the last letter's life_timer; RichterLetterStep for letters 7 down to 0.
; WAITS: no.
; Entry: nothing. Exit: on: A $11, C 1, N and Z from the sub-mode's INC, X, Y and V kept; else X $FF and A, Y, C and V
; as the last letter's step and what followed leave them (the count's compare, its INC or the timer's DEC).
ContinueWaitRichter:
    lda a:BiosJoyTrigger
    and #$08
    beq ContinueWaitRichter_at0
    lda a:OBJ_34B8+7
    cmp #$11
    bne ContinueWaitRichter_at0
ContinueWaitRichter_at1:
    stz z:zField80
    inc z:zGameSubMode
    rts
ContinueWaitRichter_at3:
    dec a:OBJ_3510
    bne RichterLetterStep_at0
    dec a:OBJ_3568
    bne RichterLetterStep_at0
    bra ContinueWaitRichter_at1
ContinueWaitRichter_at0:
    ldx #$07
ContinueWaitRichter_at2:
    bsr RichterLetterStep
    dex
    bpl ContinueWaitRichter_at2
    lda a:OBJ_34B8+7
    cmp #$11
    beq ContinueWaitRichter_at3
    cmp #$10
    bne RichterLetterStep_at0
    inc a:OBJ_34B8+7
    rts

; @routine 004D:0A9 RichterLetterStep
; @run 75:80A9
; WHAT IT DOES: One frame of one of Richter's letters. A letter not started waits out its step count, moving 8
; pixels right a step, and starts at zero; a started one, until 16 steps, writes four words of its pattern (columns
; $0F, $1F, $2F, $3F less the steps), moves a pixel down and counts a step.
; HOW: OBJ state, life_timer, x_lo and y_lo of the letter's slot; LetterPlace; the pattern from $B754 in the letters'
; bank; WriteLetterColumn four times.
; WAITS: no.
; Entry: X the letter. Exit: not started, count zero: A 0, N and Z from the state's INC; counting: A the new X, C and
; V from its add, N and Z from the count's DEC; started, 16 steps or more: A the count, N, Z and C from CMP #$10;
; writing: as the last WriteLetterColumn, N and Z from the count's INC. X kept; Y kept but where it writes.
RichterLetterStep:
    lda a:OBJ_STATE,x
    bne RichterLetterStep_at1
    lda a:OBJ_34B8,x
    bne RichterLetterStep_at2
    inc a:OBJ_STATE,x
    rts
RichterLetterStep_at2:
    lda a:OBJ_X_LO,x
    clc
    adc #$08
    sta a:OBJ_X_LO,x
    dec a:OBJ_34B8,x
RichterLetterStep_at0:
    rts
RichterLetterStep_at1:
    lda a:OBJ_34B8,x
    cmp #$10
    bcs RichterLetterStep_at0
    jsr LetterPlace
    lda #$54
    clc
    adc z:zTemp08
    sta z:zTemp08
    lda #$B7
    adc z:zTemp09
    sta z:zTemp09
    lda #$0F
    bsr WriteLetterColumn
    lda #$1F
    bsr WriteLetterColumn
    lda #$2F
    bsr WriteLetterColumn
    lda #$3F
    bsr WriteLetterColumn
    inc a:OBJ_Y_LO,x
    inc a:OBJ_34B8,x
    rts

; @routine 004D:0F2 WriteLetterColumn
; @run 75:80F2
; WHAT IT DOES: Writes one word of a stepping letter: at the letter's VRAM address plus the column less its step
; count, the word of its pattern at that index.
; HOW: The column in zTemp10; SelectVramWriteAddress, the address (the low byte added, the high byte as it is);
; SelectVramData; the word read through zTemp08, low byte then high.
; WAITS: no.
; Entry: A the column, X the letter. Exit: A the word's high byte, N and Z from it; Y the index of that byte; C the
; index's carry out of the doubling, V from the second subtraction. X kept.
WriteLetterColumn:
    sta z:zTemp10
    jsr VecSelectVramWriteAddress
    lda z:zTemp00
    clc
    adc z:zTemp10
    sec
    sbc a:OBJ_34B8,x
    sta a:VDC_DATA_LO
    lda z:zTemp01
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda z:zTemp10
    sec
    sbc a:OBJ_34B8,x
    asl a
    tay
    lda (zTemp08),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp08),y
    sta a:VDC_DATA_HI
    rts

; @routine 004D:11F LetterPlace
; @run 75:811F
; WHAT IT DOES: Finds a letter's places: its VRAM address in zTemp00 and zTemp01 ($6E00 plus $40 words a letter) and
; its pattern's offset in zTemp08 and zTemp09 ($80 bytes a letter), and maps the letters' bank pair ($10 above the
; RAM base bank) into MPR5 and MPR6.
; HOW: The letter's bits shifted into the low bytes with LSR and ROR; the high bytes added.
; WAITS: no.
; Entry: X the letter. Exit: A the letter halved, N and Z from zTemp08, C 0, V from the bank add. X and Y kept.
LetterPlace:
    stz z:zTemp00
    txa
    lsr a
    ror z:zTemp00
    lsr a
    ror z:zTemp00
    sta z:zTemp01
    lda #$00
    clc
    adc z:zTemp00
    sta z:zTemp00
    lda #$6E
    adc z:zTemp01
    sta z:zTemp01
    lda #$10
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    stz z:zTemp08
    txa
    lsr a
    ror z:zTemp08
    sta z:zTemp09
    rts

; @routine 004D:14A ContinueStartMaria
; @run 75:814A
; WHAT IT DOES: Maria's set-up: ContinueBegin, then her eight letters as objects (frame $0D up, sprite set 5, 16
; pixels apart from X $48, at Y $68), the message script started, the screen's timer set ($10 frames, one round),
; the first letter started and the next sub-mode.
; HOW: zTemp10 counts the letters down from 7 and is read again for each; ClearObject; StartScriptSlot0Kind1.
; WAITS: no.
; Entry: X and Y the dispatch's (for ContinueBegin). Exit: A $10, X 0, Y $0D, N and Z from the sub-mode's INC, C 0, V 0.
ContinueStartMaria:
    jsr ContinueBegin
ContinueStartMaria_at0:
    ldx z:zTemp10
    jsr VecClearObject
    txa
    clc
    adc #$0D
    sta a:OBJ_FRAME,x
    txa
    asl a
    asl a
    asl a
    asl a
    clc
    adc #$48
    sta a:OBJ_X_LO,x
    lda #$05
    sta a:OBJ_SPRITE_SET,x
    lda #$68
    sta a:OBJ_Y_LO,x
    dec z:zTemp10
    bpl ContinueStartMaria_at0
    lda #$0D
    jsr VecStartScriptSlot0Kind1
    lda #$01
    sta a:OBJ_3568
    lda #$10
    sta a:OBJ_3510
    inc a:OBJ_STATE
    inc z:zGameSubMode
    rts

; @routine 004D:187 ContinueWaitMaria
; @run 75:8187
; WHAT IT DOES: Maria's message, a frame. RUN: once the last letter is done goes on; before, writes every letter
; whole at once and goes on. Otherwise, once the last letter is done the screen's timer runs (Richter's, at $8087);
; until then every letter steps.
; HOW: BiosJoyTrigger's RUN bit, the last letter's state; LetterPlace, SetVramAddressXY and the pattern from $BB54
; through zTemp08 for the whole writes; MariaLetterStep for letters 7 down to 0.
; WAITS: no.
; Entry: nothing. Exit: RUN: A 2 (done) or the last pattern byte (written), C 1, N and Z from the
; sub-mode's INC, X kept or $FF, Y kept or $80, V kept or 0; timer: as continue_count_down from A 2, C 1; stepping: X
; $FF, N 1, Z 0, the rest as the last MariaLetterStep left them.
ContinueWaitMaria:
    lda a:BiosJoyTrigger
    and #$08
    beq ContinueWaitMaria_at0
    lda a:OBJ_STATE+7
    cmp #$02
    bne ContinueWaitMaria_at1
    stz z:zField80
    inc z:zGameSubMode
    rts
ContinueWaitMaria_at1:
    ldx #$07
ContinueWaitMaria_at3:
    jsr LetterPlace
    lda #$54
    clc
    adc z:zTemp08
    sta z:zTemp08
    lda #$BB
    adc z:zTemp09
    sta z:zTemp09
    phx
    ldx z:zTemp00
    ldy z:zTemp01
    jsr SetVramAddressXY
    plx
    cly
ContinueWaitMaria_at2:
    lda (zTemp08),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp08),y
    sta a:VDC_DATA_HI
    iny
    cpy #$80
    bne ContinueWaitMaria_at2
    dex
    bpl ContinueWaitMaria_at3
    stz z:zField80
    inc z:zGameSubMode
    rts
ContinueWaitMaria_at0:
    lda a:OBJ_STATE+7
    cmp #$02
    bne ContinueWaitMaria_at4
    jmp ContinueWaitRichter_at3
ContinueWaitMaria_at4:
    ldx #$07
ContinueWaitMaria_at5:
    bsr MariaLetterStep
    dex
    bpl ContinueWaitMaria_at5
ContinueWaitMaria_at6:
    rts

; @routine 004D:1E0 MariaLetterStep
; @run 75:81E0
; WHAT IT DOES: One frame of one of Maria's letters: a letter in state 1 turns a step (MariaLetterReveal of its
; pattern from $BB54), and at its 32nd step it is done (state 2) and the next letter starts.
; HOW: OBJ state and life_timer; LetterPlace; the pattern's address added; the next slot's state counted up.
; WAITS: no.
; Entry: X the letter. Exit: not turning: A the state, or the state less one, N and Z from it, Y, C and V kept;
; turning: A the count, N, Z and C from CMP #$20, Y and V as MariaLetterReveal left them; done: A $20, C 1, N and Z
; from the next letter's INC. X kept.
MariaLetterStep:
    lda a:OBJ_STATE,x
    beq ContinueWaitMaria_at6
    dec a
    bne ContinueWaitMaria_at6
    jsr LetterPlace
    lda #$54
    clc
    adc z:zTemp08
    sta z:zTemp08
    lda #$BB
    adc z:zTemp09
    sta z:zTemp09
    bsr MariaLetterReveal
    lda a:OBJ_34B8,x
    cmp #$20
    bne ContinueWaitMaria_at6
    inc a:OBJ_STATE,x
    inc a:OBJ_STATE+1,x
    rts

; @routine 004D:208 MariaLetterReveal
; @run 75:8208
; WHAT IT DOES: One step of one of Maria's turning letters: its pattern copied into StageMaskBuffer's two planes,
; its step counted, the planes stretched and turned by eight times the count (not when that is a multiple of 256),
; and the planes written to its VRAM.
; HOW: The pattern read through zTemp08 (its words' low bytes to the PLANE_LO rows, high bytes to PLANE_HI); the
; count in life_timer; StretchLetterPlanes, TurnLetterPlanes; SetVramAddressXY from zTemp00 and zTemp01; X kept on
; the stack.
; WAITS: no.
; Entry: X the letter, zTemp00-01 its VRAM address, zTemp08-09 its pattern. Exit: A $80, X kept, N and Z from it,
; Y zTemp01, C 1, V 1.
MariaLetterReveal:
    phx
    cly
    clx
MariaLetterReveal_at1:
    lda (zTemp08),y
    sta a:StageMaskBuffer+16,x
    iny
    lda (zTemp08),y
    sta a:StageMaskBuffer,x
    iny
    inx
    tya
    and #$1F
    bne MariaLetterReveal_at0
    txa
    clc
    adc #$10
    tax
MariaLetterReveal_at0:
    cpy #$80
    bne MariaLetterReveal_at1
    plx
    phx
    inc a:OBJ_34B8,x
    lda a:OBJ_34B8,x
    asl a
    asl a
    asl a
    beq MariaLetterReveal_at2
    pha
    jsr StretchLetterPlanes
    pla
    jsr TurnLetterPlanes
MariaLetterReveal_at2:
    ldx z:zTemp00
    ldy z:zTemp01
    jsr SetVramAddressXY
    clx
MariaLetterReveal_at4:
    lda a:StageMaskBuffer+16,x
    sta a:VDC_DATA_LO
    lda a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    txa
    and #$0F
    bne MariaLetterReveal_at3
    txa
    clc
    adc #$10
    tax
MariaLetterReveal_at3:
    cpx #$80
    bne MariaLetterReveal_at4
    plx
    rts

; @routine 004D:260 StretchLetterPlanes
; @run 75:8260
; WHAT IT DOES: Stretches the letter in StageMaskBuffer out from its middle: for columns 7 down to 0, a running sum
; of the step that carries copies the column to the next one out (and its mirror, 15 less each), and the columns
; left over at the middle are cleared.
; HOW: The step in zPlaneStep; CopyLetterPlaneColumn twice a carry, the registers kept on the stack across;
; ClearLetterPlaneByte for both halves of each column left, zPlaneRowsLeft counting the rows.
; WAITS: no.
; Entry: A the step, X the caller's (kept). Exit: X kept, N and Z from it; Y $FF; A $FF when no column was left,
; else 8; C and V from the last add.
StretchLetterPlanes:
    phx
    sta z:zPlaneStep
    ldy #$07
    ldx #$07
StretchLetterPlanes_at1:
    clc
    adc z:zPlaneStep
    bcc StretchLetterPlanes_at0
    pha
    jsr CopyLetterPlaneColumn
    phx
    phy
    txa
    eor #$0F
    tax
    tya
    eor #$0F
    tay
    jsr CopyLetterPlaneColumn
    ply
    plx
    dey
    pla
StretchLetterPlanes_at0:
    dex
    bpl StretchLetterPlanes_at1
    tya
    bmi StretchLetterPlanes_at2
    eor #$0F
    tax
StretchLetterPlanes_at4:
    phx
    phy
    lda #$08
    sta z:zPlaneRowsLeft
StretchLetterPlanes_at3:
    jsr ClearLetterPlaneByte
    sxy
    jsr ClearLetterPlaneByte
    sxy
    dec z:zPlaneRowsLeft
    bne StretchLetterPlanes_at3
    ply
    plx
    inx
    dey
    bpl StretchLetterPlanes_at4
StretchLetterPlanes_at2:
    plx
    rts

; @routine 004D:2A4 ClearLetterPlaneByte
; @run 75:82A4
; WHAT IT DOES: Clears one byte of StageMaskBuffer and moves X a row (16) on.
; HOW: STZ abs,X; SAX, CLC, ADC #$10, SAX.
; WAITS: no.
; Entry: X the byte. Exit: X 16 on, N, Z, C and V from that add. A and Y kept.
ClearLetterPlaneByte:
    stz a:StageMaskBuffer,x
    sax
    clc
    adc #$10
    sax
    rts

; @routine 004D:2AD CopyLetterPlaneColumn
; @run 75:82AD
; WHAT IT DOES: Copies a column of StageMaskBuffer's eight 16-byte rows to another column.
; HOW: X and Y kept on the stack; zPlaneRowsLeft counts the rows; both move a row with CLC, ADC #$10.
; WAITS: no.
; Entry: X the source column, Y the destination's. Exit: A Y plus $80, C and V from the last add, N and Z from X. X
; and Y kept.
CopyLetterPlaneColumn:
    phx
    phy
    lda #$08
    sta z:zPlaneRowsLeft
CopyLetterPlaneColumn_at0:
    lda a:StageMaskBuffer,x
    sta a:StageMaskBuffer,y
    txa
    clc
    adc #$10
    tax
    tya
    clc
    adc #$10
    tay
    dec z:zPlaneRowsLeft
    bne CopyLetterPlaneColumn_at0
    ply
    plx
    rts

; @routine 004D:2CA TurnLetterPlanes
; @run 75:82CA
; WHAT IT DOES: Stretches the letter's tiles along the other axis: transposes them, stretches, transposes back.
; HOW: X and the step kept on the stack; TransposeLetterTiles, StretchLetterPlanes, TransposeLetterTiles.
; WAITS: no.
; Entry: A the step, X the caller's (kept). Exit: X kept, N and Z from it; A zTemp0F, Y $80, C 1, V from the stretch.
TurnLetterPlanes:
    phx
    pha
    jsr TransposeLetterTiles
    pla
    jsr StretchLetterPlanes
    jsr TransposeLetterTiles
    plx
    rts

; @routine 004D:2D8 TransposeLetterTiles
; @run 75:82D8
; WHAT IT DOES: Transposes all sixteen tiles of StageMaskBuffer, four groups of four.
; HOW: Y 0; TransposeTileGroup three times by JSR and the fourth by running into it.
; WAITS: no.
; Entry: nothing. Exit: A zTemp0F, X 8, Y $80, N 0, Z 1, C 1. V kept.
TransposeLetterTiles:
    cly
    jsr TransposeTileGroup
    jsr TransposeTileGroup
    jsr TransposeTileGroup

; @routine 004D:2E2 TransposeTileGroup
; @run 75:82E2
; WHAT IT DOES: Transposes four tiles from Y: the first and last straight, the middle two mirrored.
; HOW: TransposeTile, TransposeTileMirrored twice, then the tail JMP to TransposeTile. Each leaves Y eight on.
; WAITS: no.
; Entry: Y the first row. Exit: as StoreTileRows, Y 32 on.
TransposeTileGroup:
    jsr TransposeTile
    jsr TransposeTileMirrored
    jsr TransposeTileMirrored
    jmp TransposeTile

; @routine 004D:2EE TransposeTile
; @run 75:82EE
; WHAT IT DOES: Transposes the 8 by 8 bit tile of StageMaskBuffer at Y: row r's bit 7 - c goes to row c's bit r.
; HOW: Each row shifted out with ASL into zTemp08 to zTemp0F by ROL <$08,X, X up; StoreTileRows puts them back.
; zTransposeRowsLeft counts the rows; Y is kept on the stack.
; WAITS: no.
; Entry: Y the first row. Exit: as StoreTileRows.
TransposeTile:
    phy
    lda #$08
    sta z:zTransposeRowsLeft
TransposeTile_at1:
    clx
    lda a:StageMaskBuffer,y
TransposeTile_at0:
    asl a
    rol z:zTemp08,x
    inx
    cpx #$08
    bne TransposeTile_at0
    iny
    dec z:zTransposeRowsLeft
    bne TransposeTile_at1
    ply
    jsr StoreTileRows
    rts

; @routine 004D:309 TransposeTileMirrored
; @run 75:8309
; WHAT IT DOES: As TransposeTile, the bits gathered the other way: row r's bit 7 - c goes to row 7 - c's bit 7 - r.
; HOW: Each row shifted out with ASL into zTemp0F down to zTemp08 by ROR <$08,X, X down; StoreTileRows.
; WAITS: no.
; Entry: Y the first row. Exit: as StoreTileRows.
TransposeTileMirrored:
    phy
    lda #$08
    sta z:zTransposeRowsLeft
TransposeTileMirrored_at1:
    ldx #$07
    lda a:StageMaskBuffer,y
TransposeTileMirrored_at0:
    asl a
    ror z:zTemp08,x
    dex
    bpl TransposeTileMirrored_at0
    iny
    dec z:zTransposeRowsLeft
    bne TransposeTileMirrored_at1
    ply
    jsr StoreTileRows
    rts

; @routine 004D:323 StoreTileRows
; @run 75:8323
; WHAT IT DOES: Puts the eight bytes zTemp08 to zTemp0F back into StageMaskBuffer from Y on.
; HOW: LDA <$08,X and STA abs,Y for X 0 to 7.
; WAITS: no.
; Entry: Y the first row. Exit: A zTemp0F, X 8, Y eight on, N 0, Z 1, C 1. V kept.
StoreTileRows:
    clx
StoreTileRows_at0:
    lda z:zTemp08,x
    sta a:StageMaskBuffer,y
    iny
    inx
    cpx #$08
    bne StoreTileRows_at0
    rts

; @routine 004D:330 ContinueFade
; @run 75:8330
; WHAT IT DOES: Fades the screen out. Its first frame clears the list channels and script slots and starts script
; slot 0 as kind 6 with the script in Y; then every fourth frame, nine times, every palette word in the bank's
; $B000-$B3FF (but $B2A0-$B2BF) loses one step of each colour, with script slot 0 restarted as kind 7, script $20;
; the ninth goes on.
; HOW: zField80 says the fade has begun; TAI from KernelZeros; the steps in slot 0's life_timer, the frames in its
; f3510; the palette's bank ($12, or $30 with zIdleFlag bit 0, above the RAM base bank) mapped into MPR5.
; WAITS: no.
; Entry: X and Y the dispatch's (Y the script for kind 6). Exit: first frame: A 1, X 0, Y the script, N and Z from
; zField80's INC; waiting: A zField80, N and Z from the DEC, X, Y, C and V kept; a step: A 4, X 0, Y $20, C 1, V 0,
; N and Z from the step's DEC or the sub-mode's INC.
ContinueFade:
    lda z:zField80
    bne ContinueFade_at0
    tai KernelZeros,ListChannelKind,$007F
    tai KernelZeros,ScriptSlotKind,$0060
    clx
    jsr StartScriptSlotKind6
    lda #$09
    sta a:OBJ_34B8
    lda #$01
    sta a:OBJ_3510
    inc z:zField80
ContinueFade_at1:
    rts
ContinueFade_at0:
    dec a:OBJ_3510
    bne ContinueFade_at1
    lda #$12
    bbr0 zIdleFlag,ContinueFade_at2
    lda #$30
ContinueFade_at2:
    clc
    adc z:zRamBaseBank
    tam #$20
    stz z:zTemp00
    lda #$B0
    sta z:zTemp01
ContinueFade_at9:
    cly
ContinueFade_at6:
    lda (zTemp00),y
    and #$07
    beq ContinueFade_at3
    dec a
ContinueFade_at3:
    sta z:zTemp02
    lda (zTemp00),y
    and #$38
    beq ContinueFade_at4
    sec
    sbc #$08
ContinueFade_at4:
    ora z:zTemp02
    sta z:zTemp02
    lda (zTemp00),y
    sta z:zTemp03
    iny
    lda (zTemp00),y
    lsr a
    ror z:zTemp03
    lda z:zTemp03
    and #$E0
    beq ContinueFade_at5
    sec
    sbc #$20
ContinueFade_at5:
    stz z:zTemp03
    asl a
    rol z:zTemp03
    ora z:zTemp02
    dey
    sta (zTemp00),y
    iny
    lda z:zTemp03
    sta (zTemp00),y
    iny
    cpy #$20
    bcc ContinueFade_at6
ContinueFade_at10:
    lda #$20
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ContinueFade_at7
    inc z:zTemp01
ContinueFade_at7:
    lda z:zTemp01
    cmp #$B4
    beq ContinueFade_at8
    lda z:zTemp00
    cmp #$A0
    bne ContinueFade_at9
    lda z:zTemp01
    cmp #$B2
    bne ContinueFade_at9
    bra ContinueFade_at10
ContinueFade_at8:
    clx
    jsr StartScriptSlotKind7
    lda #$04
    sta a:OBJ_3510
    dec a:OBJ_34B8
    bne ContinueFade_at11
    inc z:zGameSubMode
ContinueFade_at11:
    rts

; @routine 004D:3D8 ContinueClear
; @run 75:83D8
; WHAT IT DOES: Clears the stage away for the menu: the enemy slots, zField90 and zField91, the raster effect, the
; scroll, object 8 and objects $54-$57; then the menu's list channel and its cursor (object 8: frame 4, sprite set
; 3, X $4F) on the first choice.
; HOW: ClearEnemySlots; ClearObject; a CPX #$10 and BPL after slot 8, which ends the loop at once (9 less 16 is
; negative); StartListChannel0Kind2 $6C; ContinueChoice 0, and the cursor's row from ContinueCursorRows.
; WAITS: no.
; Entry: X the dispatch's (ClearEnemySlots keeps it on the stack), Y the dispatch's. Exit: A the cursor's row, N and Z
; from it, X the choice, C 1. Y and V kept.
ContinueClear:
    jsr VecClearEnemySlots
    stz z:zField90
    stz z:zField91
    stz z:zRasterEffect
    stz a:ScrollXLo
    stz a:ScrollXHi
    stz a:ScrollYLo
    stz a:ScrollYHi
    ldx #$08
ContinueClear_at0:
    jsr VecClearObject
    inx
    cpx #$10
    bpl ContinueClear_at0
    ldx #$54
ContinueClear_at1:
    jsr VecClearObject
    inx
    cpx #$58
    bne ContinueClear_at1
    inc z:zGameSubMode
    lda #$6C
    jsr VecStartListChannel0Kind2
    lda #$04
    sta a:OBJ_FRAME+8
    lda #$03
    sta a:OBJ_SPRITE_SET+8
    lda #$4F
    sta a:OBJ_X_LO+8
    stz a:ContinueChoice
    bra ContinueMenu_at0

; @routine 004D:41C ContinueShowMenu
; @run 75:841C
; WHAT IT DOES: Shows the menu: its script, and, when there is no third choice, the third choice's line blanked
; (VRAM $0540, $20 words of tile $0110).
; HOW: StartScriptSlot0Kind1 $0E; zStage and ProgressCharacter's bit 0; SetVramAddressXY, ST1 and ST2.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the sub-mode's INC, X 0; third choice: A ProgressCharacter halved, Y $0E, C 1;
; else A 2, Y 5, C 0 (or kept when zStage is zero). V kept.
ContinueShowMenu:
    lda #$0E
    jsr VecStartScriptSlot0Kind1
    lda z:zStage
    beq ContinueShowMenu_at0
    lda a:ProgressCharacter
    lsr a
    bcs ContinueShowMenu_at1
ContinueShowMenu_at0:
    ldx #$40
    ldy #$05
    jsr SetVramAddressXY
    ldx #$20
ContinueShowMenu_at2:
    st1 #$10
    st2 #$01
    dex
    bne ContinueShowMenu_at2
ContinueShowMenu_at1:
    inc z:zGameSubMode
    rts

; @routine 004D:43E ContinueMenu
; @run 75:843E
; WHAT IT DOES: The menu, a frame: the cursor blinks; I or RUN chooses (the character's voice, the music stopped, and
; zModeDone counted up for ModeDead); UP, DOWN or SELECT moves the choice (through two choices, or three when there
; is a third), restarts the menu's script for it, sounds the move and puts the cursor on its row.
; HOW: ContinueCursorBlink; BiosJoyTrigger; ContinueMovesTwo or ContinueMovesThree by the choice (two on for any key
; but UP); StartScriptSlot0Kind1 $0E plus the choice; SoundCommand 1; ContinueVoices by zCharacter; SoundCommand $FF.
; SoundCommand keeps X and Y, so the second call has the first one's.
; WAITS: no.
; Entry: Y the dispatch's. Exit: idle: A 0, N 0, Z 1, X, Y, C and V as ContinueCursorBlink left them; moved: A the
; row, N and Z from it, X the choice, Y the choice plus $0E, C and V as SoundCommand left them; chosen: as
; SoundCommand $FF left them, X zCharacter, N and Z from zModeDone's INC.
ContinueMenu:
    jsr ContinueCursorBlink
    lda a:BiosJoyTrigger
    and #$09
    bne ContinueMenu_at1
    lda a:BiosJoyTrigger
    and #$54
    beq ContinueMenu_at2
    ldx a:ContinueChoice
    and #$10
    bne ContinueMenu_at3
    inx
    inx
ContinueMenu_at3:
    lda z:zStage
    beq ContinueMenu_at4
    lda a:ProgressCharacter
    lsr a
    bcc ContinueMenu_at4
    lda a:ContinueMovesThree,x
    bra ContinueMenu_at5
ContinueMenu_at4:
    lda a:ContinueMovesTwo,x
ContinueMenu_at5:
    sta a:ContinueChoice
    clc
    adc #$0E
    jsr VecStartScriptSlot0Kind1
    lda #$01
    jsr VecSoundCommand
ContinueMenu_at0:
    ldx a:ContinueChoice
    lda a:ContinueCursorRows,x
    sta a:OBJ_Y_LO+8
    rts
ContinueMenu_at1:
    ldx z:zCharacter
    lda a:ContinueVoices,x
    jsr VecSoundCommand
    lda #$FF
    jsr VecSoundCommand
    inc z:zModeDone
ContinueMenu_at2:
    rts

; @routine 004D:4A0 ContinueCursorBlink
; @run 75:84A0
; WHAT IT DOES: Every fourth frame, starts the cursor's next blink script (0 to 5 and back, ten steps) in script slot 0.
; HOW: zFrameTick's low two bits; the step in the cursor's life_timer (slot 8) indexes ContinueBlinkScripts;
; StartScriptSlotKind4; the step counted and wrapped at 10.
; WAITS: no.
; Entry: X and Y the caller's. Exit: off frames: A zFrameTick's low bits, N and Z from them, X, Y, C and V kept; else A
; the step, N, Z and C from CMP #$0A, X 0, Y the script, V kept.
ContinueCursorBlink:
    lda z:zFrameTick
    and #$03
    bne ContinueCursorBlink_at0
    ldy a:OBJ_34B8+8
    lda a:ContinueBlinkScripts,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:OBJ_34B8+8
    lda a:OBJ_34B8+8
    cmp #$0A
    bcc ContinueCursorBlink_at0
    stz a:OBJ_34B8+8
ContinueCursorBlink_at0:
    rts

; @routine 004D:4C8 ClearVramRows
; @run 75:84C8
; WHAT IT DOES: Clears VRAM from an address, $40 zero words a row.
; HOW: The row count in zTemp00; SetVramAddressXY; ST1 and ST2 of zero 64 times a row; DEC zTemp00 until it is zero
; (a count of 0 clears 256 rows).
; WAITS: no.
; Entry: A rows, X the address's low byte, Y its high byte. Exit: A 2, X 0, N 0, Z 1. Y, C and V kept.
ClearVramRows:
    sta z:zTemp00
    jsr SetVramAddressXY
ClearVramRows_at1:
    ldx #$40
ClearVramRows_at0:
    st1 #$00
    st2 #$00
    dex
    bne ClearVramRows_at0
    dec z:zTemp00
    bne ClearVramRows_at1
    rts

; @routine 004D:4DB SetVramAddressXY
; @run 75:84DB
; WHAT IT DOES: Points the VDC's VRAM writes at an address and selects the data port.
; HOW: SelectVramWriteAddress, the address's two bytes to the data ports, then the tail JMP to SelectVramData.
; WAITS: no.
; Entry: X the address's low byte, Y its high byte. Exit: A 2, N 0, Z 0. X, Y, C and V kept.
SetVramAddressXY:
    jsr VecSelectVramWriteAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jmp VecSelectVramData

