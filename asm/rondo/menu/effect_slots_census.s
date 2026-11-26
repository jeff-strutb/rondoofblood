; @routine 0441:29F EffectStep0441
; @run 6C:829F
; @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
; otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
; HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
; 5 at slot 3 is entered with X 3.
; WAITS: no.
; Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
; left; C, Y and V as the last handler left them.
EffectStep0441:
    bbr0 zPaused,EffectStep0441_at0
    ldx #$02
    bsr EffectStep0441_at1
    ldx #$0B
EffectStep0441_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch0441
    rts
EffectStep0441_at4:
    ldx #$03
    jmp EffectFlash0441
EffectStep0441_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep0441_at4
    ldx #$0B
EffectStep0441_at3:
    lda a:ScriptSlotKind,x
    beq EffectStep0441_at2
    stx z:zRichterObject
    bsr EffectDispatch0441
    ldx z:zRichterObject
EffectStep0441_at2:
    dex
    bpl EffectStep0441_at3
    rts

; @routine 0441:2CB EffectDispatch0441
; @run 6C:82CB
; @stop 82DB
; @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
; kind, kind 1 first.
; HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
; with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
; WAITS: no.
; Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
EffectDispatch0441:
    dec a
    asl a
    tay
    lda a:$82DB,y
    sta z:zTemp00
    lda a:$82DC,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0441:2E5 EffectList0441
; @run 6C:82E5
; @unreachable 6C:82E5-6C:8344 no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82E5 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $8998); each palette loaded by LoadPackedPaletteC's bytes at $84BB.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList0441:
    lda a:ScriptSlotTimer,x
    beq EffectList0441_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList0441_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList0441_at1
    asl a
    tay
    lda a:$8998,y
    sta z:zTemp00
    lda a:$8999,y
    sta z:zTemp01
    bra EffectList0441_at2
EffectList0441_at1:
    asl a
    tay
    lda a:$9400
    sta z:zBiosAl
    lda a:$9401
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectList0441_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList0441_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList0441_at3
    jsr $84BB
    inc a:ScriptSlotField18,x
    bra EffectList0441
EffectList0441_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList0441_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList0441_at4

; @routine 0441:345 ClearEffectSlot0441
; @run 6C:8345
; @unreachable 6C:8345-6C:835D no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8345 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot0441:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 0441:35E EffectFade0441
; @run 6C:835E
; @unreachable 6C:835E-6C:8406 no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $835E runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at ${T:04X}; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade0441:
    lda a:ScriptSlotTimer,x
    beq EffectFade0441_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0441_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0441_at1
    jmp ClearEffectSlot0441
EffectFade0441_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0441_at2
    lda a:$89BC,y
    sta z:zTemp00
    lda a:$89BD,y
    sta z:zTemp01
    bra EffectFade0441_at3
EffectFade0441_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0441_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0441_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0441_at4
    rts
EffectFade0441_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0441_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0441_at5
    dec a
EffectFade0441_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0441_at6
    sec
    sbc #$08
EffectFade0441_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0441_at7
    sec
    sbc #$20
EffectFade0441_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0441_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0441_at9

; @routine 0441:407 EffectShade0441
; @run 6C:8407
; @stop 84BB
; @unreachable 6C:8407-6C:84BA no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8407 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
; run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
; colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
; the blocks (eight colours, their ninth bits first) read through zTemp02.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
; the list's place, V from the last block's pointer add (else kept).
EffectShade0441:
    lda a:ScriptSlotTimer,x
    beq EffectShade0441_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade0441_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade0441_at1
    jmp ClearEffectSlot0441
EffectShade0441_at1:
    eor #$07
    sta z:zTemp08
    asl a
    asl a
    asl a
    sta z:zTemp09
    asl a
    asl a
    sta z:zTemp0A
    lda a:ScriptSlotScript,x
    asl a
    tay
    lda a:$9404
    sta z:zBiosAl
    lda a:$9405
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectShade0441_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade0441_at2
    rts
EffectShade0441_at2:
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    iny
    sty z:zHitSlot
    lda (zTemp02)
    sta z:zTemp05
EffectShade0441_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade0441_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade0441_at3
    cla
EffectShade0441_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade0441_at4
    cla
EffectShade0441_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade0441_at5
    cla
EffectShade0441_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade0441_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade0441_at7
    inc z:zTemp03
EffectShade0441_at7:
    dec z:zTemp05
    bpl EffectShade0441_at8
    ldy z:zHitSlot
    jmp EffectShade0441_at9

; @routine 0441:4FB EffectColour0441
; @run 6C:84FB
; @unreachable 6C:84FB-6C:8534 no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $84FB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at ${T:04X}; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour0441:
    lda a:ScriptSlotScript,x
    bmi EffectColour0441_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour0441_at1
EffectColour0441_at0:
    asl a
    asl a
    adc #$BC
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour0441_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot0441

; @routine 0441:535 EffectFlash0441
; @run 6C:8535
; @stop 85C0
; @unreachable 6C:8535-6C:85BF no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8535 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8667 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $8698, $8898 and $3C60;
; the colour table at $85E7 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash0441:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash0441_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$8698,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$8898,$0100
EffectFlash0441_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash0441_at1
    jmp EffectFlashEnd0441
EffectFlash0441_at1:
    lda a:ScriptSlotScript,x
    asl a
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp04
    lda a:ScriptSlotField24,x
    asl a
    clc
    adc z:zTemp04
    tay
    lda a:$85E7,y
    sta z:zTemp02
    lda a:$85E8,y
    sta z:zTemp03
    cly
EffectFlash0441_at3:
    lda a:$8667,y
    cmp #$FF
    bne EffectFlash0441_at4
    rts
EffectFlash0441_at4:
    sta z:zTemp08
    iny
    lda a:$8667,y
    sta z:zTemp09
    iny
    clx
EffectFlash0441_at2:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash0441_at2
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash0441_at3

; @routine 0441:5C0 EffectFlashEnd0441
; @run 6C:85C0
; @unreachable 6C:85C0-6C:85E6 no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85C0 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $8698 (256 colours) and $8898 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes at
; $8345.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd0441:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $8698,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $8898,VCE_DATA_LO,$0100
    jmp ClearEffectSlot0441

; @routine 0651:29F EffectStep0651
; @run 6C:829F
; @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
; otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
; HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
; 5 at slot 3 is entered with X 3.
; WAITS: no.
; Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
; left; C, Y and V as the last handler left them.
EffectStep0651:
    bbr0 zPaused,EffectStep0651_at0
    ldx #$02
    bsr EffectStep0651_at1
    ldx #$0B
EffectStep0651_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch0651
    rts
EffectStep0651_at4:
    ldx #$03
    jmp EffectFlash0651
EffectStep0651_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep0651_at4
    ldx #$0B
EffectStep0651_at3:
    lda a:ScriptSlotKind,x
    beq EffectStep0651_at2
    stx z:zRichterObject
    bsr EffectDispatch0651
    ldx z:zRichterObject
EffectStep0651_at2:
    dex
    bpl EffectStep0651_at3
    rts

; @routine 0651:2CB EffectDispatch0651
; @run 6C:82CB
; @stop 82DB
; @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
; kind, kind 1 first.
; HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
; with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
; WAITS: no.
; Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
EffectDispatch0651:
    dec a
    asl a
    tay
    lda a:$82DB,y
    sta z:zTemp00
    lda a:$82DC,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0651:2E5 EffectList0651
; @run 6C:82E5
; @unreachable 6C:82E5-6C:8344 no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82E5 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $8998); each palette loaded by LoadPackedPaletteC's bytes at $84BB.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList0651:
    lda a:ScriptSlotTimer,x
    beq EffectList0651_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList0651_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList0651_at1
    asl a
    tay
    lda a:$8998,y
    sta z:zTemp00
    lda a:$8999,y
    sta z:zTemp01
    bra EffectList0651_at2
EffectList0651_at1:
    asl a
    tay
    lda a:$9400
    sta z:zBiosAl
    lda a:$9401
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectList0651_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList0651_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList0651_at3
    jsr $84BB
    inc a:ScriptSlotField18,x
    bra EffectList0651
EffectList0651_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList0651_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList0651_at4

; @routine 0651:345 ClearEffectSlot0651
; @run 6C:8345
; @unreachable 6C:8345-6C:835D no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8345 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot0651:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 0651:35E EffectFade0651
; @run 6C:835E
; @unreachable 6C:835E-6C:8406 no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $835E runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at ${T:04X}; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade0651:
    lda a:ScriptSlotTimer,x
    beq EffectFade0651_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade0651_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade0651_at1
    jmp ClearEffectSlot0651
EffectFade0651_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade0651_at2
    lda a:$89AC,y
    sta z:zTemp00
    lda a:$89AD,y
    sta z:zTemp01
    bra EffectFade0651_at3
EffectFade0651_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade0651_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade0651_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade0651_at4
    rts
EffectFade0651_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade0651_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade0651_at5
    dec a
EffectFade0651_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade0651_at6
    sec
    sbc #$08
EffectFade0651_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade0651_at7
    sec
    sbc #$20
EffectFade0651_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade0651_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade0651_at9

; @routine 0651:407 EffectShade0651
; @run 6C:8407
; @stop 84BB
; @unreachable 6C:8407-6C:84BA no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8407 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
; run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
; colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
; the blocks (eight colours, their ninth bits first) read through zTemp02.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
; the list's place, V from the last block's pointer add (else kept).
EffectShade0651:
    lda a:ScriptSlotTimer,x
    beq EffectShade0651_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade0651_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade0651_at1
    jmp ClearEffectSlot0651
EffectShade0651_at1:
    eor #$07
    sta z:zTemp08
    asl a
    asl a
    asl a
    sta z:zTemp09
    asl a
    asl a
    sta z:zTemp0A
    lda a:ScriptSlotScript,x
    asl a
    tay
    lda a:$9404
    sta z:zBiosAl
    lda a:$9405
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectShade0651_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade0651_at2
    rts
EffectShade0651_at2:
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    iny
    sty z:zHitSlot
    lda (zTemp02)
    sta z:zTemp05
EffectShade0651_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade0651_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade0651_at3
    cla
EffectShade0651_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade0651_at4
    cla
EffectShade0651_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade0651_at5
    cla
EffectShade0651_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade0651_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade0651_at7
    inc z:zTemp03
EffectShade0651_at7:
    dec z:zTemp05
    bpl EffectShade0651_at8
    ldy z:zHitSlot
    jmp EffectShade0651_at9

; @routine 0651:4FB EffectColour0651
; @run 6C:84FB
; @unreachable 6C:84FB-6C:8534 no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $84FB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at ${T:04X}; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour0651:
    lda a:ScriptSlotScript,x
    bmi EffectColour0651_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour0651_at1
EffectColour0651_at0:
    asl a
    asl a
    adc #$AC
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour0651_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot0651

; @routine 0651:535 EffectFlash0651
; @run 6C:8535
; @stop 85C0
; @unreachable 6C:8535-6C:85BF no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8535 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8667 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $8698, $8898 and $3C60;
; the colour table at $85E7 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash0651:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash0651_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$8698,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$8898,$0100
EffectFlash0651_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash0651_at1
    jmp EffectFlashEnd0651
EffectFlash0651_at1:
    lda a:ScriptSlotScript,x
    asl a
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp04
    lda a:ScriptSlotField24,x
    asl a
    clc
    adc z:zTemp04
    tay
    lda a:$85E7,y
    sta z:zTemp02
    lda a:$85E8,y
    sta z:zTemp03
    cly
EffectFlash0651_at3:
    lda a:$8667,y
    cmp #$FF
    bne EffectFlash0651_at4
    rts
EffectFlash0651_at4:
    sta z:zTemp08
    iny
    lda a:$8667,y
    sta z:zTemp09
    iny
    clx
EffectFlash0651_at2:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash0651_at2
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash0651_at3

; @routine 0651:5C0 EffectFlashEnd0651
; @run 6C:85C0
; @unreachable 6C:85C0-6C:85E6 no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85C0 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $8698 (256 colours) and $8898 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes at
; $8345.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd0651:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $8698,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $8898,VCE_DATA_LO,$0100
    jmp ClearEffectSlot0651

; @routine 1231:29F EffectStep1231
; @run 6C:829F
; @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $829F runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
; otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
; HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
; 5 at slot 3 is entered with X 3.
; WAITS: no.
; Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
; left; C, Y and V as the last handler left them.
EffectStep1231:
    bbr0 zPaused,EffectStep1231_at0
    ldx #$02
    bsr EffectStep1231_at1
    ldx #$0B
EffectStep1231_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch1231
    rts
EffectStep1231_at4:
    ldx #$03
    jmp EffectFlash1231
EffectStep1231_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep1231_at4
    ldx #$0B
EffectStep1231_at3:
    lda a:ScriptSlotKind,x
    beq EffectStep1231_at2
    stx z:zRichterObject
    bsr EffectDispatch1231
    ldx z:zRichterObject
EffectStep1231_at2:
    dex
    bpl EffectStep1231_at3
    rts

; @routine 1231:2CB EffectDispatch1231
; @run 6C:82CB
; @stop 82DB
; @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82CB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82DB holds one address per
; kind, kind 1 first.
; HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
; with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
; WAITS: no.
; Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
EffectDispatch1231:
    dec a
    asl a
    tay
    lda a:$82DB,y
    sta z:zTemp00
    lda a:$82DC,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1231:2E5 EffectList1231
; @run 6C:82E5
; @unreachable 6C:82E5-6C:8344 no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82E5 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $8998); each palette loaded by LoadPackedPaletteC's bytes at $84BB.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList1231:
    lda a:ScriptSlotTimer,x
    beq EffectList1231_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList1231_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList1231_at1
    asl a
    tay
    lda a:$8998,y
    sta z:zTemp00
    lda a:$8999,y
    sta z:zTemp01
    bra EffectList1231_at2
EffectList1231_at1:
    asl a
    tay
    lda a:$9400
    sta z:zBiosAl
    lda a:$9401
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectList1231_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList1231_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList1231_at3
    jsr $84BB
    inc a:ScriptSlotField18,x
    bra EffectList1231
EffectList1231_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList1231_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList1231_at4

; @routine 1231:345 ClearEffectSlot1231
; @run 6C:8345
; @unreachable 6C:8345-6C:835D no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8345 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot1231:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 1231:35E EffectFade1231
; @run 6C:835E
; @unreachable 6C:835E-6C:8406 no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $835E runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at ${T:04X}; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade1231:
    lda a:ScriptSlotTimer,x
    beq EffectFade1231_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade1231_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade1231_at1
    jmp ClearEffectSlot1231
EffectFade1231_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade1231_at2
    lda a:$89B2,y
    sta z:zTemp00
    lda a:$89B3,y
    sta z:zTemp01
    bra EffectFade1231_at3
EffectFade1231_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade1231_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade1231_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade1231_at4
    rts
EffectFade1231_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade1231_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade1231_at5
    dec a
EffectFade1231_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade1231_at6
    sec
    sbc #$08
EffectFade1231_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade1231_at7
    sec
    sbc #$20
EffectFade1231_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade1231_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade1231_at9

; @routine 1231:407 EffectShade1231
; @run 6C:8407
; @stop 84BB
; @unreachable 6C:8407-6C:84BA no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8407 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 3: a palette effect that darkens palettes by an amount that grows each round: once its timer has
; run down it counts a round (the slot is given back after the eighth) and writes the list's palettes to the VCE, each
; colour's blue, green and red parts taken down by 7 less the count, by that times 8 and times 32, not below zero.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9404; zTemp08 to zTemp0A the three amounts;
; the blocks (eight colours, their ninth bits first) read through zTemp02.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, Y
; the list's place, V from the last block's pointer add (else kept).
EffectShade1231:
    lda a:ScriptSlotTimer,x
    beq EffectShade1231_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade1231_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade1231_at1
    jmp ClearEffectSlot1231
EffectShade1231_at1:
    eor #$07
    sta z:zTemp08
    asl a
    asl a
    asl a
    sta z:zTemp09
    asl a
    asl a
    sta z:zTemp0A
    lda a:ScriptSlotScript,x
    asl a
    tay
    lda a:$9404
    sta z:zBiosAl
    lda a:$9405
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectShade1231_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade1231_at2
    rts
EffectShade1231_at2:
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    iny
    sty z:zHitSlot
    lda (zTemp02)
    sta z:zTemp05
EffectShade1231_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade1231_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade1231_at3
    cla
EffectShade1231_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade1231_at4
    cla
EffectShade1231_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade1231_at5
    cla
EffectShade1231_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade1231_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade1231_at7
    inc z:zTemp03
EffectShade1231_at7:
    dec z:zTemp05
    bpl EffectShade1231_at8
    ldy z:zHitSlot
    jmp EffectShade1231_at9

; @routine 1231:4FB EffectColour1231
; @run 6C:84FB
; @unreachable 6C:84FB-6C:8534 no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $84FB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at ${T:04X}; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour1231:
    lda a:ScriptSlotScript,x
    bmi EffectColour1231_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour1231_at1
EffectColour1231_at0:
    asl a
    asl a
    adc #$B2
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour1231_at1:
    cly
    lda (zTemp00),y
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta a:VCE_ADDR_HI
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_LO
    iny
    lda (zTemp00),y
    sta a:VCE_DATA_HI
    jmp ClearEffectSlot1231

; @routine 1231:535 EffectFlash1231
; @run 6C:8535
; @stop 85C0
; @unreachable 6C:8535-6C:85BF no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8535 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8667 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $8698, $8898 and $3C60;
; the colour table at $85E7 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash1231:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash1231_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$8698,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$8898,$0100
EffectFlash1231_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash1231_at1
    jmp EffectFlashEnd1231
EffectFlash1231_at1:
    lda a:ScriptSlotScript,x
    asl a
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp04
    lda a:ScriptSlotField24,x
    asl a
    clc
    adc z:zTemp04
    tay
    lda a:$85E7,y
    sta z:zTemp02
    lda a:$85E8,y
    sta z:zTemp03
    cly
EffectFlash1231_at3:
    lda a:$8667,y
    cmp #$FF
    bne EffectFlash1231_at4
    rts
EffectFlash1231_at4:
    sta z:zTemp08
    iny
    lda a:$8667,y
    sta z:zTemp09
    iny
    clx
EffectFlash1231_at2:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash1231_at2
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash1231_at3

; @routine 1231:5C0 EffectFlashEnd1231
; @run 6C:85C0
; @unreachable 6C:85C0-6C:85E6 no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85C0 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $8698 (256 colours) and $8898 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes at
; $8345.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd1231:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $8698,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $8898,VCE_DATA_LO,$0100
    jmp ClearEffectSlot1231
