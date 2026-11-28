; @routine 267B:7AB Scene58TextObject
; @run 6B:87AB
; WHAT IT DOES: An object handler (kind 1): runs the step of the object's state from the table after it. (Scene
; $58's overlay.)
; HOW: OBJ_STATE doubled indexes five words; a jump through zTemp00.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextObjectStates = $87BD ; the five steps' addresses
Scene58TextObject:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:Scene58TextObjectStates,y
    sta z:zTemp00
    lda a:Scene58TextObjectStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 267B:7C7 Scene58TextStart
; @run 6B:87C7
; WHAT IT DOES: The text object's state 0: points the VDC at the line's VRAM place by OBJ_START_ARG, clears $400
; words there, sets a rising Y speed of -$60, takes the line OBJ_WORK_D names, its sprite attribute from the line's
; second byte, a wait of $12 in OBJ_WORK_H, and steps the state on. (Scene $58's overlay.)
; HOW: SelectVramWriteAddress, a table word into the address port, SelectVramData, a loop of $400 ST1/ST2 zeros;
; table reads.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextStartLines = $89E2 ; the address of each line, a word each
Scene58TextStartVramRows = $8E72 ; the VRAM address of each line place, a word each
Scene58TextStart:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    jsr VecSelectVramWriteAddress
    lda a:Scene58TextStartVramRows,y
    sta a:VDC_DATA_LO
    lda a:Scene58TextStartVramRows+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda #$A0
    sta a:OBJ_VY_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    phx
    lda #$04
    sta z:zTemp00
    clx
Scene58TextStart_at0:
    st1 #$00
    st2 #$00
    dex
    bne Scene58TextStart_at0
    dec z:zTemp00
    bne Scene58TextStart_at0
    plx
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:Scene58TextStartLines,y
    sta z:zTemp10
    lda a:Scene58TextStartLines+1,y
    sta z:zTemp11
    ldy #$01
    lda (zTemp10),y
    sta a:OBJ_SPRITE_ATTR,x
    lda #$12
    sta a:OBJ_WORK_H,x
    inc a:OBJ_STATE,x
    rts

; @routine 267C:019 Scene58TextSkipSpace
; @run 6B:8819
; WHAT IT DOES: Steps the pattern line's code on (OBJ_WORK_G), passing a space. (Scene $58's overlay.)
; HOW: INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextSkipSpace:
    inc a:OBJ_WORK_G,x
    rts

; @routine 267C:01D Scene58TextPatterns
; @run 6B:881D
; WHAT IT DOES: A pattern line: for each code of the line copies $40 words of the overlay's pattern bank (from
; $B000 by the code) to the glyph's VRAM place, skipping code $20; at the line's end the object is placed and its
; state stepped on. (Scene $58's overlay.)
; HOW: The VRAM address set; the code halved into the pattern's address; a loop of $80 bytes to the data port.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextPatterns:
    stz z:zTemp08
    stz z:zBiosAl
    jsr Scene58TextGlyphVramAddress
    ldy a:OBJ_WORK_G,x
    iny
    iny
    lda (zTemp10),y
    cmp #$20
    beq Scene58TextSkipSpace
    stz z:zTemp00
    lsr a
    ror z:zTemp00
    ora #$B0
    sta z:zTemp01
    cly
Scene58TextPatterns_at0:
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    iny
    cpy #$80
    bne Scene58TextPatterns_at0
    inc a:OBJ_WORK_G,x
    ldy a:OBJ_WORK_G,x
    iny
    iny
    lda (zTemp10),y
    bpl Scene58TextPatterns_at1
    jmp Scene58TextLineDone
Scene58TextPatterns_at1:
    rts

; @routine 267C:059 Scene58TextWrite
; @run 6B:8859
; WHAT IT DOES: The text object's state 1: counts OBJ_WORK_H down and takes the line again; a line marked by bit 7
; of the sprite attribute is drawn from the overlay's patterns instead; otherwise the next code of the line
; (OBJ_WORK_G) is looked up in the card's font into the buffer at $3B40, or a code $E0.. is the blank glyph, and
; drawn. (Scene $58's overlay.)
; HOW: The code's two bytes into zBiosAh and zBiosAl, zBiosB $3B40 and zBiosDh 0, ex_getfnt between PHX and PLX,
; then the glyph drawn.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextWriteLines = $89E2 ; the address of each line, a word each
Scene58TextWrite:
    dec a:OBJ_WORK_H,x
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:Scene58TextWriteLines,y
    sta z:zTemp10
    lda a:Scene58TextWriteLines+1,y
    sta z:zTemp11
    lda a:OBJ_SPRITE_ATTR,x
    bmi Scene58TextPatterns
    lda a:OBJ_WORK_G,x
    inc a
    asl a
    tay
    lda (zTemp10),y
    sta z:zBiosAh
    cmp #$E0
    beq Scene58TextBlankGlyph
    iny
    lda (zTemp10),y
    sta z:zBiosAl
    lda #$40
    sta z:zBiosBl
    lda #$3B
    sta z:zBiosBh
    lda #$00
    sta z:zBiosDh
    phx
    jsr BiosExGetfnt
    plx

; @routine 267C:094 Scene58TextNextGlyph
; @run 6B:8894
; WHAT IT DOES: Draws the glyph in the buffer at $3B40, steps OBJ_WORK_G on, and ends the line when the next code
; is zero. (Scene $58's overlay.)
; HOW: The draw, INC, a read through zTemp10.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextNextGlyph:
    jsr Scene58TextDrawGlyph
    inc a:OBJ_WORK_G,x
    lda a:OBJ_WORK_G,x
    inc a
    asl a
    tay
    lda (zTemp10),y
    bne Scene58TextReturn

; @routine 267C:0A4 Scene58TextLineDone
; @run 6B:88A4
; WHAT IT DOES: At the end of a line: the line's first byte becomes the object's X, its Y high byte and OBJ_WORK_G
; are cleared and the state steps on. (Scene $58's overlay.)
; HOW: Stores through zTemp10; a compare of X whose branch goes on either way.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextLineDone:
    lda (zTemp10)
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    stz a:OBJ_WORK_G,x
    cpx #$2E
    bne Scene58TextLineDone_at0
Scene58TextLineDone_at0:
    inc a:OBJ_STATE,x

; @routine 267C:0B6 Scene58TextReturn
; @run 6B:88B6
; WHAT IT DOES: Returns: the end the text steps branch to. (Scene $58's overlay.)
; HOW: RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextReturn:
    rts

; @routine 267C:0B7 Scene58TextBlankGlyph
; @run 6B:88B7
; WHAT IT DOES: For a code $E0..: copies the blank glyph's 32 bytes into the buffer at $3B40 and draws it. (Scene
; $58's overlay.)
; HOW: A copy loop, then the draw.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextBlankGlyphBlank = $88C6 ; the blank glyph's 32 bytes
Scene58TextBlankGlyph:
    cly
Scene58TextBlankGlyph_at0:
    lda a:Scene58TextBlankGlyphBlank,y
    sta a:StageMaskBuffer,y
    iny
    cpy #$20
    bne Scene58TextBlankGlyph_at0
    jmp Scene58TextNextGlyph

; @routine 267C:0E6 Scene58TextDrawGlyph
; @run 6B:88E6
; WHAT IT DOES: Draws the 16 by 16 glyph in the buffer at $3B40 into VRAM: its four 8-line quarters ORed into the
; tiles at the glyph's place, the right half shifted a bit to the right. (Scene $58's overlay.)
; HOW: Four times the VRAM address set and $10 or $0F word pairs read, ORed and written back; the buffer shifted
; once.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextDrawGlyph:
    stz z:zTemp08
    stz z:zBiosAl
    jsr Scene58TextGlyphVramAddress
    phx
    clx
Scene58TextDrawGlyph_at0:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$20
    bne Scene58TextDrawGlyph_at0
    plx
    inc z:zBiosAl
    lda #$10
    sta z:zTemp08
    jsr Scene58TextGlyphVramAddress
    phx
    clx
Scene58TextDrawGlyph_at1:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$1E
    bne Scene58TextDrawGlyph_at1
    plx
    stz z:zBiosAl
    jsr Scene58TextShiftGlyph
    jsr Scene58TextGlyphVramAddress
    phx
    clx
Scene58TextDrawGlyph_at2:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$20
    bne Scene58TextDrawGlyph_at2
    plx
    inc z:zBiosAl
    jsr Scene58TextGlyphVramAddress
    phx
    clx
Scene58TextDrawGlyph_at3:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$1E
    bne Scene58TextDrawGlyph_at3
    plx
    rts

; @routine 267C:170 Scene58TextShiftGlyph
; @run 6B:8970
; WHAT IT DOES: Shifts the 32-byte glyph buffer at $3B40 right by one bit, a word at a time. (Scene $58's overlay.)
; HOW: A loop of LSR and ROR, X kept.
; WAITS: no.
; Entry: X as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextShiftGlyph:
    phx
    clx
Scene58TextShiftGlyph_at0:
    lsr a:StageMaskBuffer,x
    ror a:StageMaskBuffer+1,x
    inx
    inx
    cpx #$20
    bne Scene58TextShiftGlyph_at0
    plx
    rts

; @routine 267C:180 Scene58TextGlyphVramAddress
; @run 6B:8980
; WHAT IT DOES: Points the VDC at the glyph's VRAM place for both reading and writing: the line's place (by
; OBJ_START_ARG) plus the glyph's column (OBJ_WORK_G times $40), plus zBiosAl and zTemp08. (Scene $58's overlay.)
; HOW: Shifts and adds into zTemp02-03; SelectVramWriteAddress and SelectVramReadAddress with the address, then
; SelectVramData as the tail.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextGlyphVramAddressVramRows = $8E72 ; the VRAM address of each line place, a word each
Scene58TextGlyphVramAddress:
    lda a:OBJ_WORK_G,x
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
    lda a:Scene58TextGlyphVramAddressVramRows,y
    clc
    adc z:zBiosAl
    clc
    adc z:zTemp00
    sta z:zTemp02
    sta a:VDC_DATA_LO
    lda a:Scene58TextGlyphVramAddressVramRows+1,y
    adc z:zTemp01
    sta z:zTemp03
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zTemp02
    sta a:VDC_DATA_LO
    lda z:zTemp03
    sta a:VDC_DATA_HI
    jmp VecSelectVramData

; @routine 267C:1C3 Scene58TextRise
; @run 6B:89C3
; WHAT IT DOES: The text object's state 3: moves by its Y speed. (Scene $58's overlay.)
; HOW: ApplyVelocityY.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextRise:
    jsr VecApplyVelocityY
    rts

; @routine 267C:1C7 Scene58TextShow
; @run 6B:89C7
; WHAT IT DOES: The text object's state 2: counts OBJ_WORK_H down; at zero takes its frame and sprite set by
; OBJ_START_ARG and steps the state on. (Scene $58's overlay.)
; HOW: DEC, two table reads.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextShowFrames = $8E7E ; the sprite frame of each line place
Scene58TextShowSpriteSets = $8E84 ; the sprite set of each line place
Scene58TextShow:
    dec a:OBJ_WORK_H,x
    bne Scene58TextShow_at0
    ldy a:OBJ_START_ARG,x
    lda a:Scene58TextShowFrames,y
    sta a:OBJ_FRAME,x
    lda a:Scene58TextShowSpriteSets,y
    sta a:OBJ_SPRITE_SET,x
    inc a:OBJ_STATE,x
Scene58TextShow_at0:
    rts

; @routine 267C:1DF Scene58TextEnd
; @run 6B:89DF
; WHAT IT DOES: The text object's state 4: clears the object. (Scene $58's overlay.)
; HOW: A tail JMP to ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene58TextEnd:
    jmp VecClearObject

; @routine 267F:003 SceneEffectsVecStep
; @run 6C:8003
; WHAT IT DOES: The second entry of the jump table at the start of the overlay at sector $267F: goes to the effect
; step at $829F.
; HOW: A JMP to the routine loaded there.
; WAITS: no.
; Entry: X as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
SceneEffectsVecStep:
    jmp EffectStep1A31

; @routine 2703:248 Scene5AQuakeScroll
; @run 6B:8248
; WHAT IT DOES: An object handler (kind 15) of the $5A overlay: in state 0 sets a count of $A1 in OBJ_WORK_D and
; steps the state on; in state 1 counts it down, shaking the scroll's Y ($3E20) by a random step plus half of $A0
; less the count, and steps on at zero; after that the shake rides on $50.
; HOW: A test of the state; NextRandom's low four bits index the shake steps.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5AQuakeScrollSteps = $8294 ; the shake step for each of 16 random values
Scene5AQuakeScroll:
    lda a:OBJ_STATE,x
    bne Scene5AQuakeScroll_at0
    lda #$A1
    sta a:OBJ_WORK_D,x
Scene5AQuakeScroll_at2:
    inc a:OBJ_STATE,x
    rts
Scene5AQuakeScroll_at0:
    dec a
    bne Scene5AQuakeScroll_at1
    dec a:OBJ_WORK_D,x
    beq Scene5AQuakeScroll_at2
    lda #$A0
    sec
    sbc a:OBJ_WORK_D,x
    lsr a
    sta z:zTemp00
    jsr VecNextRandom
    and #$0F
    tay
    lda a:Scene5AQuakeScrollSteps,y
    clc
    adc z:zTemp00
    sta a:ScrollYLo
    rts
Scene5AQuakeScroll_at1:
    jsr VecNextRandom
    and #$0F
    tay
    lda a:Scene5AQuakeScrollSteps,y
    clc
    adc #$50
    sta a:ScrollYLo
    rts

; @routine 2703:287 Scene5AJitterScroll
; @run 6B:8287
; WHAT IT DOES: An object handler (kind 14) of the $5A overlay: sets the scroll's Y ($3E20) to a random shake step.
; HOW: NextRandom's low four bits index the shake steps.
; WAITS: no.
; Entry: A as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5AJitterScrollSteps = $8294 ; the shake step for each of 16 random values
Scene5AJitterScroll:
    jsr VecNextRandom
    and #$0F
    tay
    lda a:Scene5AJitterScrollSteps,y
    sta a:ScrollYLo
    rts

; @routine 2703:6BD Scene5ATextObject
; @run 6B:86BD
; WHAT IT DOES: An object handler (kind 1): runs the step of the object's state from the table after it. (Scene
; $5A's overlay.)
; HOW: OBJ_STATE doubled indexes five words; a jump through zTemp00.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextObjectStates = $86CF ; the five steps' addresses
Scene5ATextObject:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:Scene5ATextObjectStates,y
    sta z:zTemp00
    lda a:Scene5ATextObjectStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 2703:6D9 Scene5ATextStart
; @run 6B:86D9
; WHAT IT DOES: The text object's state 0: points the VDC at the line's VRAM place by OBJ_START_ARG, clears $400
; words there, sets a rising Y speed of -$60, takes the line OBJ_WORK_D names, its sprite attribute from the line's
; second byte, a wait of $12 in OBJ_WORK_H, and steps the state on. (Scene $5A's overlay.)
; HOW: SelectVramWriteAddress, a table word into the address port, SelectVramData, a loop of $400 ST1/ST2 zeros;
; table reads.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextStartLines = $88F4 ; the address of each line, a word each
Scene5ATextStartVramRows = $8D84 ; the VRAM address of each line place, a word each
Scene5ATextStart:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    jsr VecSelectVramWriteAddress
    lda a:Scene5ATextStartVramRows,y
    sta a:VDC_DATA_LO
    lda a:Scene5ATextStartVramRows+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda #$A0
    sta a:OBJ_VY_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    phx
    lda #$04
    sta z:zTemp00
    clx
Scene5ATextStart_at0:
    st1 #$00
    st2 #$00
    dex
    bne Scene5ATextStart_at0
    dec z:zTemp00
    bne Scene5ATextStart_at0
    plx
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:Scene5ATextStartLines,y
    sta z:zTemp10
    lda a:Scene5ATextStartLines+1,y
    sta z:zTemp11
    ldy #$01
    lda (zTemp10),y
    sta a:OBJ_SPRITE_ATTR,x
    lda #$12
    sta a:OBJ_WORK_H,x
    inc a:OBJ_STATE,x
    rts

; @routine 2703:72B Scene5ATextSkipSpace
; @run 6B:872B
; WHAT IT DOES: Steps the pattern line's code on (OBJ_WORK_G), passing a space. (Scene $5A's overlay.)
; HOW: INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextSkipSpace:
    inc a:OBJ_WORK_G,x
    rts

; @routine 2703:72F Scene5ATextPatterns
; @run 6B:872F
; WHAT IT DOES: A pattern line: for each code of the line copies $40 words of the overlay's pattern bank (from
; $B000 by the code) to the glyph's VRAM place, skipping code $20; at the line's end the object is placed and its
; state stepped on. (Scene $5A's overlay.)
; HOW: The VRAM address set; the code halved into the pattern's address; a loop of $80 bytes to the data port.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextPatterns:
    stz z:zTemp08
    stz z:zBiosAl
    jsr Scene5ATextGlyphVramAddress
    ldy a:OBJ_WORK_G,x
    iny
    iny
    lda (zTemp10),y
    cmp #$20
    beq Scene5ATextSkipSpace
    stz z:zTemp00
    lsr a
    ror z:zTemp00
    ora #$B0
    sta z:zTemp01
    cly
Scene5ATextPatterns_at0:
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    iny
    cpy #$80
    bne Scene5ATextPatterns_at0
    inc a:OBJ_WORK_G,x
    ldy a:OBJ_WORK_G,x
    iny
    iny
    lda (zTemp10),y
    bpl Scene5ATextPatterns_at1
    jmp Scene5ATextLineDone
Scene5ATextPatterns_at1:
    rts

; @routine 2703:76B Scene5ATextWrite
; @run 6B:876B
; WHAT IT DOES: The text object's state 1: counts OBJ_WORK_H down and takes the line again; a line marked by bit 7
; of the sprite attribute is drawn from the overlay's patterns instead; otherwise the next code of the line
; (OBJ_WORK_G) is looked up in the card's font into the buffer at $3B40, or a code $E0.. is the blank glyph, and
; drawn. (Scene $5A's overlay.)
; HOW: The code's two bytes into zBiosAh and zBiosAl, zBiosB $3B40 and zBiosDh 0, ex_getfnt between PHX and PLX,
; then the glyph drawn.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextWriteLines = $88F4 ; the address of each line, a word each
Scene5ATextWrite:
    dec a:OBJ_WORK_H,x
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:Scene5ATextWriteLines,y
    sta z:zTemp10
    lda a:Scene5ATextWriteLines+1,y
    sta z:zTemp11
    lda a:OBJ_SPRITE_ATTR,x
    bmi Scene5ATextPatterns
    lda a:OBJ_WORK_G,x
    inc a
    asl a
    tay
    lda (zTemp10),y
    sta z:zBiosAh
    cmp #$E0
    beq Scene5ATextBlankGlyph
    iny
    lda (zTemp10),y
    sta z:zBiosAl
    lda #$40
    sta z:zBiosBl
    lda #$3B
    sta z:zBiosBh
    lda #$00
    sta z:zBiosDh
    phx
    jsr BiosExGetfnt
    plx

; @routine 2703:7A6 Scene5ATextNextGlyph
; @run 6B:87A6
; WHAT IT DOES: Draws the glyph in the buffer at $3B40, steps OBJ_WORK_G on, and ends the line when the next code
; is zero. (Scene $5A's overlay.)
; HOW: The draw, INC, a read through zTemp10.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextNextGlyph:
    jsr Scene5ATextDrawGlyph
    inc a:OBJ_WORK_G,x
    lda a:OBJ_WORK_G,x
    inc a
    asl a
    tay
    lda (zTemp10),y
    bne Scene5ATextReturn

; @routine 2703:7B6 Scene5ATextLineDone
; @run 6B:87B6
; WHAT IT DOES: At the end of a line: the line's first byte becomes the object's X, its Y high byte and OBJ_WORK_G
; are cleared and the state steps on. (Scene $5A's overlay.)
; HOW: Stores through zTemp10; a compare of X whose branch goes on either way.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextLineDone:
    lda (zTemp10)
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    stz a:OBJ_WORK_G,x
    cpx #$2E
    bne Scene5ATextLineDone_at0
Scene5ATextLineDone_at0:
    inc a:OBJ_STATE,x

; @routine 2703:7C8 Scene5ATextReturn
; @run 6B:87C8
; WHAT IT DOES: Returns: the end the text steps branch to. (Scene $5A's overlay.)
; HOW: RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextReturn:
    rts

; @routine 2703:7C9 Scene5ATextBlankGlyph
; @run 6B:87C9
; WHAT IT DOES: For a code $E0..: copies the blank glyph's 32 bytes into the buffer at $3B40 and draws it. (Scene
; $5A's overlay.)
; HOW: A copy loop, then the draw.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextBlankGlyphBlank = $87D8 ; the blank glyph's 32 bytes
Scene5ATextBlankGlyph:
    cly
Scene5ATextBlankGlyph_at0:
    lda a:Scene5ATextBlankGlyphBlank,y
    sta a:StageMaskBuffer,y
    iny
    cpy #$20
    bne Scene5ATextBlankGlyph_at0
    jmp Scene5ATextNextGlyph

; @routine 2703:7F8 Scene5ATextDrawGlyph
; @run 6B:87F8
; WHAT IT DOES: Draws the 16 by 16 glyph in the buffer at $3B40 into VRAM: its four 8-line quarters ORed into the
; tiles at the glyph's place, the right half shifted a bit to the right. (Scene $5A's overlay.)
; HOW: Four times the VRAM address set and $10 or $0F word pairs read, ORed and written back; the buffer shifted
; once.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextDrawGlyph:
    stz z:zTemp08
    stz z:zBiosAl
    jsr Scene5ATextGlyphVramAddress
    phx
    clx
Scene5ATextDrawGlyph_at0:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$20
    bne Scene5ATextDrawGlyph_at0
    plx
    inc z:zBiosAl
    lda #$10
    sta z:zTemp08
    jsr Scene5ATextGlyphVramAddress
    phx
    clx
Scene5ATextDrawGlyph_at1:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$1E
    bne Scene5ATextDrawGlyph_at1
    plx
    stz z:zBiosAl
    jsr Scene5ATextShiftGlyph
    jsr Scene5ATextGlyphVramAddress
    phx
    clx
Scene5ATextDrawGlyph_at2:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$20
    bne Scene5ATextDrawGlyph_at2
    plx
    inc z:zBiosAl
    jsr Scene5ATextGlyphVramAddress
    phx
    clx
Scene5ATextDrawGlyph_at3:
    lda a:VDC_DATA_LO
    ora a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    ora a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$1E
    bne Scene5ATextDrawGlyph_at3
    plx
    rts

; @routine 2704:082 Scene5ATextShiftGlyph
; @run 6B:8882
; WHAT IT DOES: Shifts the 32-byte glyph buffer at $3B40 right by one bit, a word at a time. (Scene $5A's overlay.)
; HOW: A loop of LSR and ROR, X kept.
; WAITS: no.
; Entry: X as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextShiftGlyph:
    phx
    clx
Scene5ATextShiftGlyph_at0:
    lsr a:StageMaskBuffer,x
    ror a:StageMaskBuffer+1,x
    inx
    inx
    cpx #$20
    bne Scene5ATextShiftGlyph_at0
    plx
    rts

; @routine 2704:092 Scene5ATextGlyphVramAddress
; @run 6B:8892
; WHAT IT DOES: Points the VDC at the glyph's VRAM place for both reading and writing: the line's place (by
; OBJ_START_ARG) plus the glyph's column (OBJ_WORK_G times $40), plus zBiosAl and zTemp08. (Scene $5A's overlay.)
; HOW: Shifts and adds into zTemp02-03; SelectVramWriteAddress and SelectVramReadAddress with the address, then
; SelectVramData as the tail.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextGlyphVramAddressVramRows = $8D84 ; the VRAM address of each line place, a word each
Scene5ATextGlyphVramAddress:
    lda a:OBJ_WORK_G,x
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
    lda a:Scene5ATextGlyphVramAddressVramRows,y
    clc
    adc z:zBiosAl
    clc
    adc z:zTemp00
    sta z:zTemp02
    sta a:VDC_DATA_LO
    lda a:Scene5ATextGlyphVramAddressVramRows+1,y
    adc z:zTemp01
    sta z:zTemp03
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zTemp02
    sta a:VDC_DATA_LO
    lda z:zTemp03
    sta a:VDC_DATA_HI
    jmp VecSelectVramData

; @routine 2704:0D5 Scene5ATextRise
; @run 6B:88D5
; WHAT IT DOES: The text object's state 3: moves by its Y speed. (Scene $5A's overlay.)
; HOW: ApplyVelocityY.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextRise:
    jsr VecApplyVelocityY
    rts

; @routine 2704:0D9 Scene5ATextShow
; @run 6B:88D9
; WHAT IT DOES: The text object's state 2: counts OBJ_WORK_H down; at zero takes its frame and sprite set by
; OBJ_START_ARG and steps the state on. (Scene $5A's overlay.)
; HOW: DEC, two table reads.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextShowFrames = $8D90 ; the sprite frame of each line place
Scene5ATextShowSpriteSets = $8D96 ; the sprite set of each line place
Scene5ATextShow:
    dec a:OBJ_WORK_H,x
    bne Scene5ATextShow_at0
    ldy a:OBJ_START_ARG,x
    lda a:Scene5ATextShowFrames,y
    sta a:OBJ_FRAME,x
    lda a:Scene5ATextShowSpriteSets,y
    sta a:OBJ_SPRITE_SET,x
    inc a:OBJ_STATE,x
Scene5ATextShow_at0:
    rts

; @routine 2704:0F1 Scene5ATextEnd
; @run 6B:88F1
; WHAT IT DOES: The text object's state 4: clears the object. (Scene $5A's overlay.)
; HOW: A tail JMP to ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Scene5ATextEnd:
    jmp VecClearObject
