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
; $8998); each palette loaded by the packed palette loader at $84BB, whose bytes are LoadPackedPaletteC's.
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
; table at $89BC; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
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
; or for a negative number the table at $89BC; the four bytes go to the VCE ports.
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
; @stop 05C0
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
    bne EffectFlash0441_at5
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
EffectFlash0441_at5:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash0441_at0
    jmp EffectFlashEnd0441
EffectFlash0441_at0:
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
EffectFlash0441_at2:
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
EffectFlash0441_at1:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash0441_at1
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash0441_at2

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
; HOW: Two TIA from $8698 (256 colours) and $8898 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
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
; $8998); each palette loaded by the packed palette loader at $84BB, whose bytes are LoadPackedPaletteC's.
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
; table at $89AC; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
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
; or for a negative number the table at $89AC; the four bytes go to the VCE ports.
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
; @stop 05C0
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
    bne EffectFlash0651_at5
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
EffectFlash0651_at5:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash0651_at0
    jmp EffectFlashEnd0651
EffectFlash0651_at0:
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
EffectFlash0651_at2:
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
EffectFlash0651_at1:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash0651_at1
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash0651_at2

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
; HOW: Two TIA from $8698 (256 colours) and $8898 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
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
; $8998); each palette loaded by the packed palette loader at $84BB, whose bytes are LoadPackedPaletteC's.
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
; table at $89B2; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
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
; or for a negative number the table at $89B2; the four bytes go to the VCE ports.
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
; @stop 05C0
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
    bne EffectFlash1231_at5
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
EffectFlash1231_at5:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash1231_at0
    jmp EffectFlashEnd1231
EffectFlash1231_at0:
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
EffectFlash1231_at2:
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
EffectFlash1231_at1:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash1231_at1
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash1231_at2

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
; HOW: Two TIA from $8698 (256 colours) and $8898 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
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

; @routine 1845:2AE EffectStep1845
; @run 6C:82AE
; @unreachable 6C:82AE-6C:82D9 no read the game makes loads disc sector 1845, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82AE runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
; otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
; HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
; 5 at slot 3 is entered with X 3.
; WAITS: no.
; Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
; left; C, Y and V as the last handler left them.
EffectStep1845:
    bbr0 zPaused,EffectStep1845_at0
    ldx #$02
    bsr EffectStep1845_at1
    ldx #$0B
EffectStep1845_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch1845
    rts
EffectStep1845_at2:
    ldx #$03
    jmp EffectFlash1845
EffectStep1845_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep1845_at2
    ldx #$0B
EffectStep1845_at4:
    lda a:ScriptSlotKind,x
    beq EffectStep1845_at3
    stx z:zRichterObject
    bsr EffectDispatch1845
    ldx z:zRichterObject
EffectStep1845_at3:
    dex
    bpl EffectStep1845_at4
    rts

; @routine 1845:2DA EffectDispatch1845
; @run 6C:82DA
; @stop 82EA
; @unreachable 6C:82DA-6C:82E9 no read the game makes loads disc sector 1845, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82DA runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82EA holds one address per
; kind, kind 1 first.
; HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
; with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
; WAITS: no.
; Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
EffectDispatch1845:
    dec a
    asl a
    tay
    lda a:$82EA,y
    sta z:zTemp00
    lda a:$82EB,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1845:2F4 EffectList1845
; @run 6C:82F4
; @unreachable 6C:82F4-6C:8353 no read the game makes loads disc sector 1845, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82F4 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $89A7); each palette loaded by the packed palette loader at $84CA, whose bytes are LoadPackedPaletteC's.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList1845:
    lda a:ScriptSlotTimer,x
    beq EffectList1845_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList1845_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList1845_at1
    asl a
    tay
    lda a:$89A7,y
    sta z:zTemp00
    lda a:$89A8,y
    sta z:zTemp01
    bra EffectList1845_at2
EffectList1845_at1:
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
EffectList1845_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList1845_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList1845_at3
    jsr $84CA
    inc a:ScriptSlotField18,x
    bra EffectList1845
EffectList1845_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList1845_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList1845_at4

; @routine 1845:354 ClearEffectSlot1845
; @run 6C:8354
; @unreachable 6C:8354-6C:836C no read the game makes loads disc sector 1845, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8354 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot1845:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 1845:36D EffectFade1845
; @run 6C:836D
; @unreachable 6C:836D-6C:8415 no read the game makes loads disc sector 1845, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $836D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at $9412; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade1845:
    lda a:ScriptSlotTimer,x
    beq EffectFade1845_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade1845_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade1845_at1
    jmp ClearEffectSlot1845
EffectFade1845_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade1845_at2
    lda a:$9412,y
    sta z:zTemp00
    lda a:$9413,y
    sta z:zTemp01
    bra EffectFade1845_at3
EffectFade1845_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade1845_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade1845_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade1845_at4
    rts
EffectFade1845_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade1845_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade1845_at5
    dec a
EffectFade1845_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade1845_at6
    sec
    sbc #$08
EffectFade1845_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade1845_at7
    sec
    sbc #$20
EffectFade1845_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade1845_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade1845_at9

; @routine 1845:416 EffectShade1845
; @run 6C:8416
; @stop 84CA
; @unreachable 6C:8416-6C:84C9 no read the game makes loads disc sector 1845, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8416 runs whatever overlay another scene
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
EffectShade1845:
    lda a:ScriptSlotTimer,x
    beq EffectShade1845_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade1845_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade1845_at1
    jmp ClearEffectSlot1845
EffectShade1845_at1:
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
EffectShade1845_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade1845_at2
    rts
EffectShade1845_at2:
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
EffectShade1845_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade1845_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade1845_at3
    cla
EffectShade1845_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade1845_at4
    cla
EffectShade1845_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade1845_at5
    cla
EffectShade1845_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade1845_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade1845_at7
    inc z:zTemp03
EffectShade1845_at7:
    dec z:zTemp05
    bpl EffectShade1845_at8
    ldy z:zHitSlot
    jmp EffectShade1845_at9

; @routine 1845:50A EffectColour1845
; @run 6C:850A
; @unreachable 6C:850A-6C:8543 no read the game makes loads disc sector 1845, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $850A runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at $89A7; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour1845:
    lda a:ScriptSlotScript,x
    bmi EffectColour1845_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour1845_at1
EffectColour1845_at0:
    asl a
    asl a
    adc #$A7
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour1845_at1:
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
    jmp ClearEffectSlot1845

; @routine 1845:544 EffectFlash1845
; @run 6C:8544
; @stop 05CF
; @unreachable 6C:8544-6C:85CE no read the game makes loads disc sector 1845, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8544 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8676 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86A7, $88A7 and $3C60;
; the colour table at $85F6 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash1845:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash1845_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$86A7,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$88A7,$0100
EffectFlash1845_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash1845_at1
    jmp EffectFlashEnd1845
EffectFlash1845_at1:
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
    lda a:$85F6,y
    sta z:zTemp02
    lda a:$85F7,y
    sta z:zTemp03
    cly
EffectFlash1845_at4:
    lda a:$8676,y
    cmp #$FF
    bne EffectFlash1845_at2
    rts
EffectFlash1845_at2:
    sta z:zTemp08
    iny
    lda a:$8676,y
    sta z:zTemp09
    iny
    clx
EffectFlash1845_at3:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash1845_at3
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash1845_at4

; @routine 1845:5CF EffectFlashEnd1845
; @run 6C:85CF
; @unreachable 6C:85CF-6C:85F5 no read the game makes loads disc sector 1845, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85CF runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $86A7 (256 colours) and $88A7 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd1845:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $86A7,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $88A7,VCE_DATA_LO,$0100
    jmp ClearEffectSlot1845

; @routine 1A31:29F EffectStep1A31
; @run 6C:829F
; @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 1A31, so these bytes are never in memory.
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
EffectStep1A31:
    bbr0 zPaused,EffectStep1A31_at0
    ldx #$02
    bsr EffectStep1A31_at1
    ldx #$0B
EffectStep1A31_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch1A31
    rts
EffectStep1A31_at4:
    ldx #$03
    jmp EffectFlash1A31
EffectStep1A31_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep1A31_at4
    ldx #$0B
EffectStep1A31_at3:
    lda a:ScriptSlotKind,x
    beq EffectStep1A31_at2
    stx z:zRichterObject
    bsr EffectDispatch1A31
    ldx z:zRichterObject
EffectStep1A31_at2:
    dex
    bpl EffectStep1A31_at3
    rts

; @routine 1A31:2CB EffectDispatch1A31
; @run 6C:82CB
; @stop 82DB
; @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 1A31, so these bytes are never in memory.
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
EffectDispatch1A31:
    dec a
    asl a
    tay
    lda a:$82DB,y
    sta z:zTemp00
    lda a:$82DC,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1A31:2E9 EffectList1A31
; @run 6C:82E9
; @unreachable 6C:82E9-6C:8348 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82E9 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $89C3); each palette loaded by the packed palette loader at $84BF, whose bytes are LoadPackedPaletteC's.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList1A31:
    lda a:ScriptSlotTimer,x
    beq EffectList1A31_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList1A31_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList1A31_at1
    asl a
    tay
    lda a:$89C3,y
    sta z:zTemp00
    lda a:$89C4,y
    sta z:zTemp01
    bra EffectList1A31_at2
EffectList1A31_at1:
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
EffectList1A31_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList1A31_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList1A31_at3
    jsr LoadPackedPalette
    inc a:ScriptSlotField18,x
    bra EffectList1A31
EffectList1A31_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList1A31_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList1A31_at4

; @routine 1A31:349 ClearEffectSlot1A31
; @run 6C:8349
; @unreachable 6C:8349-6C:8361 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8349 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot1A31:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 1A31:362 EffectFade1A31
; @run 6C:8362
; @unreachable 6C:8362-6C:840A no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8362 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at $89D9; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade1A31:
    lda a:ScriptSlotTimer,x
    beq EffectFade1A31_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade1A31_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade1A31_at1
    jmp ClearEffectSlot1A31
EffectFade1A31_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade1A31_at2
    lda a:$89D9,y
    sta z:zTemp00
    lda a:$89DA,y
    sta z:zTemp01
    bra EffectFade1A31_at3
EffectFade1A31_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade1A31_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade1A31_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade1A31_at4
    rts
EffectFade1A31_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade1A31_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade1A31_at5
    dec a
EffectFade1A31_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade1A31_at6
    sec
    sbc #$08
EffectFade1A31_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade1A31_at7
    sec
    sbc #$20
EffectFade1A31_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade1A31_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade1A31_at9

; @routine 1A31:40B EffectShade1A31
; @run 6C:840B
; @stop 84BF
; @unreachable 6C:840B-6C:84BE no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $840B runs whatever overlay another scene
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
EffectShade1A31:
    lda a:ScriptSlotTimer,x
    beq EffectShade1A31_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade1A31_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade1A31_at1
    jmp ClearEffectSlot1A31
EffectShade1A31_at1:
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
EffectShade1A31_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade1A31_at2
    rts
EffectShade1A31_at2:
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
EffectShade1A31_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade1A31_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade1A31_at3
    cla
EffectShade1A31_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade1A31_at4
    cla
EffectShade1A31_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade1A31_at5
    cla
EffectShade1A31_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade1A31_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade1A31_at7
    inc z:zTemp03
EffectShade1A31_at7:
    dec z:zTemp05
    bpl EffectShade1A31_at8
    ldy z:zHitSlot
    jmp EffectShade1A31_at9

; @routine 1A31:4FF EffectColour1A31
; @run 6C:84FF
; @unreachable 6C:84FF-6C:8538 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $84FF runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at $89D9; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour1A31:
    lda a:ScriptSlotScript,x
    bmi EffectColour1A31_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour1A31_at1
EffectColour1A31_at0:
    asl a
    asl a
    adc #$D9
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour1A31_at1:
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
    jmp ClearEffectSlot1A31

; @routine 1A31:539 EffectVceSave1A31
; @run 6C:8539
; @unreachable 6C:8539-6C:8542 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8539 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 6: all 512 colours of the VCE saved in the Super RAM bank mapped at $A000, at $B000.
; HOW: SelectSavedBank, then a TAI of $400 bytes from the VCE's data port.
; WAITS: no.
; Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
EffectVceSave1A31:
    bsr SelectSavedBank1A31
    tai VCE_DATA_LO,$B000,$0400
    rts

; @routine 1A31:543 EffectVceRestore1A31
; @run 6C:8543
; @unreachable 6C:8543-6C:854C no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8543 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 7: the 512 colours written back to the VCE from the Super RAM bank mapped at $A000, at $B000.
; HOW: SelectSavedBank, then a TIA of $400 bytes to the VCE's data port.
; WAITS: no.
; Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
EffectVceRestore1A31:
    bsr SelectSavedBank1A31
    tia $B000,VCE_DATA_LO,$0400
    rts

; @routine 1A31:54D SelectSavedBank1A31
; @run 6C:854D
; @unreachable 6C:854D-6C:855F no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $854D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the Super RAM bank of the saved palette at $A000 and sets the VCE's colour address to 0.
; HOW: The bank is the base bank plus $12, or plus $30 when bit 0 of zIdleFlag is set; TAM for MPR5.
; WAITS: no.
; Entry: nothing. Exit: A the bank, N and Z from it, C and V from the add; X and Y kept.
SelectSavedBank1A31:
    lda #$12
    bbr0 zIdleFlag,SelectSavedBank1A31_at0
    lda #$30
SelectSavedBank1A31_at0:
    clc
    adc z:zRamBaseBank
    tam #$20
    stz a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    rts

; @routine 1A31:560 EffectFlash1A31
; @run 6C:8560
; @stop 05EB
; @unreachable 6C:8560-6C:85EA no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8560 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8692 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86C3, $88C3 and $3C60;
; the colour table at $8612 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash1A31:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash1A31_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$86C3,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$88C3,$0100
EffectFlash1A31_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash1A31_at4
    jmp EffectFlashEnd1A31
EffectFlash1A31_at4:
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
    lda a:$8612,y
    sta z:zTemp02
    lda a:$8613,y
    sta z:zTemp03
    cly
EffectFlash1A31_at3:
    lda a:$8692,y
    cmp #$FF
    bne EffectFlash1A31_at1
    rts
EffectFlash1A31_at1:
    sta z:zTemp08
    iny
    lda a:$8692,y
    sta z:zTemp09
    iny
    clx
EffectFlash1A31_at2:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash1A31_at2
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash1A31_at3

; @routine 1A31:5EB EffectFlashEnd1A31
; @run 6C:85EB
; @unreachable 6C:85EB-6C:8611 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85EB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $86C3 (256 colours) and $88C3 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd1A31:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $86C3,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $88C3,VCE_DATA_LO,$0100
    jmp ClearEffectSlot1A31

; @routine 1AA0:2AE EffectStep1AA0
; @run 6C:82AE
; @unreachable 6C:82AE-6C:82D9 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82AE runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
; otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
; HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
; 5 at slot 3 is entered with X 3.
; WAITS: no.
; Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
; left; C, Y and V as the last handler left them.
EffectStep1AA0:
    bbr0 zPaused,EffectStep1AA0_at0
    ldx #$02
    bsr EffectStep1AA0_at1
    ldx #$0B
EffectStep1AA0_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch1AA0
    rts
EffectStep1AA0_at2:
    ldx #$03
    jmp EffectFlash1AA0
EffectStep1AA0_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep1AA0_at2
    ldx #$0B
EffectStep1AA0_at4:
    lda a:ScriptSlotKind,x
    beq EffectStep1AA0_at3
    stx z:zRichterObject
    bsr EffectDispatch1AA0
    ldx z:zRichterObject
EffectStep1AA0_at3:
    dex
    bpl EffectStep1AA0_at4
    rts

; @routine 1AA0:2DA EffectDispatch1AA0
; @run 6C:82DA
; @stop 82EA
; @unreachable 6C:82DA-6C:82E9 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82DA runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82EA holds one address per
; kind, kind 1 first.
; HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
; with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
; WAITS: no.
; Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
EffectDispatch1AA0:
    dec a
    asl a
    tay
    lda a:$82EA,y
    sta z:zTemp00
    lda a:$82EB,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1AA0:2F4 EffectList1AA0
; @run 6C:82F4
; @unreachable 6C:82F4-6C:8353 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82F4 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $89A7); each palette loaded by the packed palette loader at $84CA, whose bytes are LoadPackedPaletteC's.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList1AA0:
    lda a:ScriptSlotTimer,x
    beq EffectList1AA0_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList1AA0_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList1AA0_at1
    asl a
    tay
    lda a:$89A7,y
    sta z:zTemp00
    lda a:$89A8,y
    sta z:zTemp01
    bra EffectList1AA0_at2
EffectList1AA0_at1:
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
EffectList1AA0_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList1AA0_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList1AA0_at3
    jsr $84CA
    inc a:ScriptSlotField18,x
    bra EffectList1AA0
EffectList1AA0_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList1AA0_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList1AA0_at4

; @routine 1AA0:354 ClearEffectSlot1AA0
; @run 6C:8354
; @unreachable 6C:8354-6C:836C no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8354 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot1AA0:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 1AA0:36D EffectFade1AA0
; @run 6C:836D
; @unreachable 6C:836D-6C:8415 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $836D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at $940E; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade1AA0:
    lda a:ScriptSlotTimer,x
    beq EffectFade1AA0_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade1AA0_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade1AA0_at1
    jmp ClearEffectSlot1AA0
EffectFade1AA0_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade1AA0_at2
    lda a:$940E,y
    sta z:zTemp00
    lda a:$940F,y
    sta z:zTemp01
    bra EffectFade1AA0_at3
EffectFade1AA0_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade1AA0_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade1AA0_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade1AA0_at4
    rts
EffectFade1AA0_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade1AA0_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade1AA0_at5
    dec a
EffectFade1AA0_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade1AA0_at6
    sec
    sbc #$08
EffectFade1AA0_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade1AA0_at7
    sec
    sbc #$20
EffectFade1AA0_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade1AA0_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade1AA0_at9

; @routine 1AA0:416 EffectShade1AA0
; @run 6C:8416
; @stop 84CA
; @unreachable 6C:8416-6C:84C9 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8416 runs whatever overlay another scene
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
EffectShade1AA0:
    lda a:ScriptSlotTimer,x
    beq EffectShade1AA0_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade1AA0_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade1AA0_at1
    jmp ClearEffectSlot1AA0
EffectShade1AA0_at1:
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
EffectShade1AA0_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade1AA0_at2
    rts
EffectShade1AA0_at2:
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
EffectShade1AA0_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade1AA0_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade1AA0_at3
    cla
EffectShade1AA0_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade1AA0_at4
    cla
EffectShade1AA0_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade1AA0_at5
    cla
EffectShade1AA0_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade1AA0_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade1AA0_at7
    inc z:zTemp03
EffectShade1AA0_at7:
    dec z:zTemp05
    bpl EffectShade1AA0_at8
    ldy z:zHitSlot
    jmp EffectShade1AA0_at9

; @routine 1AA0:50A EffectColour1AA0
; @run 6C:850A
; @unreachable 6C:850A-6C:8543 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $850A runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at $89A7; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour1AA0:
    lda a:ScriptSlotScript,x
    bmi EffectColour1AA0_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour1AA0_at1
EffectColour1AA0_at0:
    asl a
    asl a
    adc #$A7
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour1AA0_at1:
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
    jmp ClearEffectSlot1AA0

; @routine 1AA0:544 EffectFlash1AA0
; @run 6C:8544
; @stop 05CF
; @unreachable 6C:8544-6C:85CE no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8544 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8676 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86A7, $88A7 and $3C60;
; the colour table at $85F6 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash1AA0:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash1AA0_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$86A7,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$88A7,$0100
EffectFlash1AA0_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash1AA0_at1
    jmp EffectFlashEnd1AA0
EffectFlash1AA0_at1:
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
    lda a:$85F6,y
    sta z:zTemp02
    lda a:$85F7,y
    sta z:zTemp03
    cly
EffectFlash1AA0_at4:
    lda a:$8676,y
    cmp #$FF
    bne EffectFlash1AA0_at2
    rts
EffectFlash1AA0_at2:
    sta z:zTemp08
    iny
    lda a:$8676,y
    sta z:zTemp09
    iny
    clx
EffectFlash1AA0_at3:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash1AA0_at3
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash1AA0_at4

; @routine 1AA0:5CF EffectFlashEnd1AA0
; @run 6C:85CF
; @unreachable 6C:85CF-6C:85F5 no read the game makes loads disc sector 1AA0, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85CF runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $86A7 (256 colours) and $88A7 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd1AA0:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $86A7,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $88A7,VCE_DATA_LO,$0100
    jmp ClearEffectSlot1AA0

; @routine 1B91:29F EffectStep1B91
; @run 6C:829F
; @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 1B91, so these bytes are never in memory.
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
EffectStep1B91:
    bbr0 zPaused,EffectStep1B91_at0
    ldx #$02
    bsr EffectStep1B91_at1
    ldx #$0B
EffectStep1B91_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch1B91
    rts
EffectStep1B91_at4:
    ldx #$03
    jmp EffectFlash1B91
EffectStep1B91_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep1B91_at4
    ldx #$0B
EffectStep1B91_at3:
    lda a:ScriptSlotKind,x
    beq EffectStep1B91_at2
    stx z:zRichterObject
    bsr EffectDispatch1B91
    ldx z:zRichterObject
EffectStep1B91_at2:
    dex
    bpl EffectStep1B91_at3
    rts

; @routine 1B91:2CB EffectDispatch1B91
; @run 6C:82CB
; @stop 82DB
; @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 1B91, so these bytes are never in memory.
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
EffectDispatch1B91:
    dec a
    asl a
    tay
    lda a:$82DB,y
    sta z:zTemp00
    lda a:$82DC,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1B91:2E9 EffectList1B91
; @run 6C:82E9
; @unreachable 6C:82E9-6C:8348 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82E9 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $89C3); each palette loaded by the packed palette loader at $84BF, whose bytes are LoadPackedPaletteC's.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList1B91:
    lda a:ScriptSlotTimer,x
    beq EffectList1B91_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList1B91_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList1B91_at1
    asl a
    tay
    lda a:$89C3,y
    sta z:zTemp00
    lda a:$89C4,y
    sta z:zTemp01
    bra EffectList1B91_at2
EffectList1B91_at1:
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
EffectList1B91_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList1B91_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList1B91_at3
    jsr LoadPackedPalette
    inc a:ScriptSlotField18,x
    bra EffectList1B91
EffectList1B91_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList1B91_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList1B91_at4

; @routine 1B91:349 ClearEffectSlot1B91
; @run 6C:8349
; @unreachable 6C:8349-6C:8361 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8349 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot1B91:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 1B91:362 EffectFade1B91
; @run 6C:8362
; @unreachable 6C:8362-6C:840A no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8362 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at $89CF; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade1B91:
    lda a:ScriptSlotTimer,x
    beq EffectFade1B91_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade1B91_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade1B91_at1
    jmp ClearEffectSlot1B91
EffectFade1B91_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade1B91_at2
    lda a:$89CF,y
    sta z:zTemp00
    lda a:$89D0,y
    sta z:zTemp01
    bra EffectFade1B91_at3
EffectFade1B91_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade1B91_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade1B91_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade1B91_at4
    rts
EffectFade1B91_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade1B91_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade1B91_at5
    dec a
EffectFade1B91_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade1B91_at6
    sec
    sbc #$08
EffectFade1B91_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade1B91_at7
    sec
    sbc #$20
EffectFade1B91_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade1B91_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade1B91_at9

; @routine 1B91:40B EffectShade1B91
; @run 6C:840B
; @stop 84BF
; @unreachable 6C:840B-6C:84BE no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $840B runs whatever overlay another scene
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
EffectShade1B91:
    lda a:ScriptSlotTimer,x
    beq EffectShade1B91_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade1B91_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade1B91_at1
    jmp ClearEffectSlot1B91
EffectShade1B91_at1:
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
EffectShade1B91_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade1B91_at2
    rts
EffectShade1B91_at2:
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
EffectShade1B91_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade1B91_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade1B91_at3
    cla
EffectShade1B91_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade1B91_at4
    cla
EffectShade1B91_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade1B91_at5
    cla
EffectShade1B91_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade1B91_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade1B91_at7
    inc z:zTemp03
EffectShade1B91_at7:
    dec z:zTemp05
    bpl EffectShade1B91_at8
    ldy z:zHitSlot
    jmp EffectShade1B91_at9

; @routine 1B91:4FF EffectColour1B91
; @run 6C:84FF
; @unreachable 6C:84FF-6C:8538 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $84FF runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at $89CF; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour1B91:
    lda a:ScriptSlotScript,x
    bmi EffectColour1B91_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour1B91_at1
EffectColour1B91_at0:
    asl a
    asl a
    adc #$CF
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour1B91_at1:
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
    jmp ClearEffectSlot1B91

; @routine 1B91:539 EffectVceSave1B91
; @run 6C:8539
; @unreachable 6C:8539-6C:8542 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8539 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 6: all 512 colours of the VCE saved in the Super RAM bank mapped at $A000, at $B000.
; HOW: SelectSavedBank, then a TAI of $400 bytes from the VCE's data port.
; WAITS: no.
; Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
EffectVceSave1B91:
    bsr SelectSavedBank1B91
    tai VCE_DATA_LO,$B000,$0400
    rts

; @routine 1B91:543 EffectVceRestore1B91
; @run 6C:8543
; @unreachable 6C:8543-6C:854C no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8543 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 7: the 512 colours written back to the VCE from the Super RAM bank mapped at $A000, at $B000.
; HOW: SelectSavedBank, then a TIA of $400 bytes to the VCE's data port.
; WAITS: no.
; Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
EffectVceRestore1B91:
    bsr SelectSavedBank1B91
    tia $B000,VCE_DATA_LO,$0400
    rts

; @routine 1B91:54D SelectSavedBank1B91
; @run 6C:854D
; @unreachable 6C:854D-6C:855F no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $854D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the Super RAM bank of the saved palette at $A000 and sets the VCE's colour address to 0.
; HOW: The bank is the base bank plus $12, or plus $30 when bit 0 of zIdleFlag is set; TAM for MPR5.
; WAITS: no.
; Entry: nothing. Exit: A the bank, N and Z from it, C and V from the add; X and Y kept.
SelectSavedBank1B91:
    lda #$12
    bbr0 zIdleFlag,SelectSavedBank1B91_at0
    lda #$30
SelectSavedBank1B91_at0:
    clc
    adc z:zRamBaseBank
    tam #$20
    stz a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    rts

; @routine 1B91:560 EffectFlash1B91
; @run 6C:8560
; @stop 05EB
; @unreachable 6C:8560-6C:85EA no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8560 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8692 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86C3, $88C3 and $3C60;
; the colour table at $8612 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash1B91:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash1B91_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$86C3,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$88C3,$0100
EffectFlash1B91_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash1B91_at4
    jmp EffectFlashEnd1B91
EffectFlash1B91_at4:
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
    lda a:$8612,y
    sta z:zTemp02
    lda a:$8613,y
    sta z:zTemp03
    cly
EffectFlash1B91_at3:
    lda a:$8692,y
    cmp #$FF
    bne EffectFlash1B91_at1
    rts
EffectFlash1B91_at1:
    sta z:zTemp08
    iny
    lda a:$8692,y
    sta z:zTemp09
    iny
    clx
EffectFlash1B91_at2:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash1B91_at2
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash1B91_at3

; @routine 1B91:5EB EffectFlashEnd1B91
; @run 6C:85EB
; @unreachable 6C:85EB-6C:8611 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85EB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $86C3 (256 colours) and $88C3 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd1B91:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $86C3,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $88C3,VCE_DATA_LO,$0100
    jmp ClearEffectSlot1B91

; @routine 1C2F:2AE EffectStep1C2F
; @run 6C:82AE
; @unreachable 6C:82AE-6C:82D9 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82AE runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
; otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
; HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
; 5 at slot 3 is entered with X 3.
; WAITS: no.
; Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
; left; C, Y and V as the last handler left them.
EffectStep1C2F:
    bbr0 zPaused,EffectStep1C2F_at0
    ldx #$02
    bsr EffectStep1C2F_at1
    ldx #$0B
EffectStep1C2F_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch1C2F
    rts
EffectStep1C2F_at2:
    ldx #$03
    jmp EffectFlash1C2F
EffectStep1C2F_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep1C2F_at2
    ldx #$0B
EffectStep1C2F_at4:
    lda a:ScriptSlotKind,x
    beq EffectStep1C2F_at3
    stx z:zRichterObject
    bsr EffectDispatch1C2F
    ldx z:zRichterObject
EffectStep1C2F_at3:
    dex
    bpl EffectStep1C2F_at4
    rts

; @routine 1C2F:2DA EffectDispatch1C2F
; @run 6C:82DA
; @stop 82EA
; @unreachable 6C:82DA-6C:82E9 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82DA runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82EA holds one address per
; kind, kind 1 first.
; HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
; with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
; WAITS: no.
; Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
EffectDispatch1C2F:
    dec a
    asl a
    tay
    lda a:$82EA,y
    sta z:zTemp00
    lda a:$82EB,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1C2F:2F4 EffectList1C2F
; @run 6C:82F4
; @unreachable 6C:82F4-6C:8353 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82F4 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $89A7); each palette loaded by the packed palette loader at $84CA, whose bytes are LoadPackedPaletteC's.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList1C2F:
    lda a:ScriptSlotTimer,x
    beq EffectList1C2F_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList1C2F_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList1C2F_at1
    asl a
    tay
    lda a:$89A7,y
    sta z:zTemp00
    lda a:$89A8,y
    sta z:zTemp01
    bra EffectList1C2F_at2
EffectList1C2F_at1:
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
EffectList1C2F_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList1C2F_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList1C2F_at3
    jsr $84CA
    inc a:ScriptSlotField18,x
    bra EffectList1C2F
EffectList1C2F_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList1C2F_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList1C2F_at4

; @routine 1C2F:354 ClearEffectSlot1C2F
; @run 6C:8354
; @unreachable 6C:8354-6C:836C no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8354 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot1C2F:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 1C2F:36D EffectFade1C2F
; @run 6C:836D
; @unreachable 6C:836D-6C:8415 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $836D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at $9410; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade1C2F:
    lda a:ScriptSlotTimer,x
    beq EffectFade1C2F_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade1C2F_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade1C2F_at1
    jmp ClearEffectSlot1C2F
EffectFade1C2F_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade1C2F_at2
    lda a:$9410,y
    sta z:zTemp00
    lda a:$9411,y
    sta z:zTemp01
    bra EffectFade1C2F_at3
EffectFade1C2F_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade1C2F_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade1C2F_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade1C2F_at4
    rts
EffectFade1C2F_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade1C2F_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade1C2F_at5
    dec a
EffectFade1C2F_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade1C2F_at6
    sec
    sbc #$08
EffectFade1C2F_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade1C2F_at7
    sec
    sbc #$20
EffectFade1C2F_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade1C2F_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade1C2F_at9

; @routine 1C2F:416 EffectShade1C2F
; @run 6C:8416
; @stop 84CA
; @unreachable 6C:8416-6C:84C9 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8416 runs whatever overlay another scene
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
EffectShade1C2F:
    lda a:ScriptSlotTimer,x
    beq EffectShade1C2F_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade1C2F_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade1C2F_at1
    jmp ClearEffectSlot1C2F
EffectShade1C2F_at1:
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
EffectShade1C2F_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade1C2F_at2
    rts
EffectShade1C2F_at2:
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
EffectShade1C2F_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade1C2F_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade1C2F_at3
    cla
EffectShade1C2F_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade1C2F_at4
    cla
EffectShade1C2F_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade1C2F_at5
    cla
EffectShade1C2F_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade1C2F_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade1C2F_at7
    inc z:zTemp03
EffectShade1C2F_at7:
    dec z:zTemp05
    bpl EffectShade1C2F_at8
    ldy z:zHitSlot
    jmp EffectShade1C2F_at9

; @routine 1C2F:50A EffectColour1C2F
; @run 6C:850A
; @unreachable 6C:850A-6C:8543 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $850A runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at $89A7; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour1C2F:
    lda a:ScriptSlotScript,x
    bmi EffectColour1C2F_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour1C2F_at1
EffectColour1C2F_at0:
    asl a
    asl a
    adc #$A7
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour1C2F_at1:
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
    jmp ClearEffectSlot1C2F

; @routine 1C2F:544 EffectFlash1C2F
; @run 6C:8544
; @stop 05CF
; @unreachable 6C:8544-6C:85CE no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8544 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8676 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86A7, $88A7 and $3C60;
; the colour table at $85F6 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash1C2F:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash1C2F_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$86A7,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$88A7,$0100
EffectFlash1C2F_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash1C2F_at1
    jmp EffectFlashEnd1C2F
EffectFlash1C2F_at1:
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
    lda a:$85F6,y
    sta z:zTemp02
    lda a:$85F7,y
    sta z:zTemp03
    cly
EffectFlash1C2F_at4:
    lda a:$8676,y
    cmp #$FF
    bne EffectFlash1C2F_at2
    rts
EffectFlash1C2F_at2:
    sta z:zTemp08
    iny
    lda a:$8676,y
    sta z:zTemp09
    iny
    clx
EffectFlash1C2F_at3:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash1C2F_at3
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash1C2F_at4

; @routine 1C2F:5CF EffectFlashEnd1C2F
; @run 6C:85CF
; @unreachable 6C:85CF-6C:85F5 no read the game makes loads disc sector 1C2F, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85CF runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $86A7 (256 colours) and $88A7 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd1C2F:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $86A7,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $88A7,VCE_DATA_LO,$0100
    jmp ClearEffectSlot1C2F

; @routine 1CF1:29F EffectStep1CF1
; @run 6C:829F
; @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
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
EffectStep1CF1:
    bbr0 zPaused,EffectStep1CF1_at0
    ldx #$02
    bsr EffectStep1CF1_at1
    ldx #$0B
EffectStep1CF1_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch1CF1
    rts
EffectStep1CF1_at4:
    ldx #$03
    jmp EffectFlash1CF1
EffectStep1CF1_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep1CF1_at4
    ldx #$0B
EffectStep1CF1_at3:
    lda a:ScriptSlotKind,x
    beq EffectStep1CF1_at2
    stx z:zRichterObject
    bsr EffectDispatch1CF1
    ldx z:zRichterObject
EffectStep1CF1_at2:
    dex
    bpl EffectStep1CF1_at3
    rts

; @routine 1CF1:2CB EffectDispatch1CF1
; @run 6C:82CB
; @stop 82DB
; @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
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
EffectDispatch1CF1:
    dec a
    asl a
    tay
    lda a:$82DB,y
    sta z:zTemp00
    lda a:$82DC,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1CF1:2E9 EffectList1CF1
; @run 6C:82E9
; @unreachable 6C:82E9-6C:8348 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82E9 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $89C3); each palette loaded by the packed palette loader at $84BF, whose bytes are LoadPackedPaletteC's.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList1CF1:
    lda a:ScriptSlotTimer,x
    beq EffectList1CF1_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList1CF1_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList1CF1_at1
    asl a
    tay
    lda a:$89C3,y
    sta z:zTemp00
    lda a:$89C4,y
    sta z:zTemp01
    bra EffectList1CF1_at2
EffectList1CF1_at1:
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
EffectList1CF1_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList1CF1_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList1CF1_at3
    jsr LoadPackedPalette
    inc a:ScriptSlotField18,x
    bra EffectList1CF1
EffectList1CF1_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList1CF1_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList1CF1_at4

; @routine 1CF1:349 ClearEffectSlot1CF1
; @run 6C:8349
; @unreachable 6C:8349-6C:8361 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8349 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot1CF1:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 1CF1:362 EffectFade1CF1
; @run 6C:8362
; @unreachable 6C:8362-6C:840A no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8362 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at $89D1; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade1CF1:
    lda a:ScriptSlotTimer,x
    beq EffectFade1CF1_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade1CF1_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade1CF1_at1
    jmp ClearEffectSlot1CF1
EffectFade1CF1_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade1CF1_at2
    lda a:$89D1,y
    sta z:zTemp00
    lda a:$89D2,y
    sta z:zTemp01
    bra EffectFade1CF1_at3
EffectFade1CF1_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade1CF1_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade1CF1_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade1CF1_at4
    rts
EffectFade1CF1_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade1CF1_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade1CF1_at5
    dec a
EffectFade1CF1_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade1CF1_at6
    sec
    sbc #$08
EffectFade1CF1_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade1CF1_at7
    sec
    sbc #$20
EffectFade1CF1_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade1CF1_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade1CF1_at9

; @routine 1CF1:40B EffectShade1CF1
; @run 6C:840B
; @stop 84BF
; @unreachable 6C:840B-6C:84BE no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $840B runs whatever overlay another scene
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
EffectShade1CF1:
    lda a:ScriptSlotTimer,x
    beq EffectShade1CF1_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade1CF1_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade1CF1_at1
    jmp ClearEffectSlot1CF1
EffectShade1CF1_at1:
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
EffectShade1CF1_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade1CF1_at2
    rts
EffectShade1CF1_at2:
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
EffectShade1CF1_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade1CF1_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade1CF1_at3
    cla
EffectShade1CF1_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade1CF1_at4
    cla
EffectShade1CF1_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade1CF1_at5
    cla
EffectShade1CF1_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade1CF1_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade1CF1_at7
    inc z:zTemp03
EffectShade1CF1_at7:
    dec z:zTemp05
    bpl EffectShade1CF1_at8
    ldy z:zHitSlot
    jmp EffectShade1CF1_at9

; @routine 1CF1:4FF EffectColour1CF1
; @run 6C:84FF
; @unreachable 6C:84FF-6C:8538 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $84FF runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at $89D1; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour1CF1:
    lda a:ScriptSlotScript,x
    bmi EffectColour1CF1_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour1CF1_at1
EffectColour1CF1_at0:
    asl a
    asl a
    adc #$D1
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour1CF1_at1:
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
    jmp ClearEffectSlot1CF1

; @routine 1CF1:539 EffectVceSave1CF1
; @run 6C:8539
; @unreachable 6C:8539-6C:8542 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8539 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 6: all 512 colours of the VCE saved in the Super RAM bank mapped at $A000, at $B000.
; HOW: SelectSavedBank, then a TAI of $400 bytes from the VCE's data port.
; WAITS: no.
; Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
EffectVceSave1CF1:
    bsr SelectSavedBank1CF1
    tai VCE_DATA_LO,$B000,$0400
    rts

; @routine 1CF1:543 EffectVceRestore1CF1
; @run 6C:8543
; @unreachable 6C:8543-6C:854C no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8543 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 7: the 512 colours written back to the VCE from the Super RAM bank mapped at $A000, at $B000.
; HOW: SelectSavedBank, then a TIA of $400 bytes to the VCE's data port.
; WAITS: no.
; Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
EffectVceRestore1CF1:
    bsr SelectSavedBank1CF1
    tia $B000,VCE_DATA_LO,$0400
    rts

; @routine 1CF1:54D SelectSavedBank1CF1
; @run 6C:854D
; @unreachable 6C:854D-6C:855F no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $854D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the Super RAM bank of the saved palette at $A000 and sets the VCE's colour address to 0.
; HOW: The bank is the base bank plus $12, or plus $30 when bit 0 of zIdleFlag is set; TAM for MPR5.
; WAITS: no.
; Entry: nothing. Exit: A the bank, N and Z from it, C and V from the add; X and Y kept.
SelectSavedBank1CF1:
    lda #$12
    bbr0 zIdleFlag,SelectSavedBank1CF1_at0
    lda #$30
SelectSavedBank1CF1_at0:
    clc
    adc z:zRamBaseBank
    tam #$20
    stz a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    rts

; @routine 1CF1:560 EffectFlash1CF1
; @run 6C:8560
; @stop 05EB
; @unreachable 6C:8560-6C:85EA no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8560 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8692 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86C3, $88C3 and $3C60;
; the colour table at $8612 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash1CF1:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash1CF1_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$86C3,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$88C3,$0100
EffectFlash1CF1_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash1CF1_at4
    jmp EffectFlashEnd1CF1
EffectFlash1CF1_at4:
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
    lda a:$8612,y
    sta z:zTemp02
    lda a:$8613,y
    sta z:zTemp03
    cly
EffectFlash1CF1_at3:
    lda a:$8692,y
    cmp #$FF
    bne EffectFlash1CF1_at1
    rts
EffectFlash1CF1_at1:
    sta z:zTemp08
    iny
    lda a:$8692,y
    sta z:zTemp09
    iny
    clx
EffectFlash1CF1_at2:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash1CF1_at2
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash1CF1_at3

; @routine 1CF1:5EB EffectFlashEnd1CF1
; @run 6C:85EB
; @unreachable 6C:85EB-6C:8611 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85EB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $86C3 (256 colours) and $88C3 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd1CF1:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $86C3,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $88C3,VCE_DATA_LO,$0100
    jmp ClearEffectSlot1CF1

; @routine 1DDA:2AE EffectStep1DDA
; @run 6C:82AE
; @unreachable 6C:82AE-6C:82D9 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82AE runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One frame of the ending's palette effects: while the game is paused only slots 2 and $0B are run;
; otherwise slot 3 alone when it holds kind 5, else every slot from $0B down to 0 that has a kind.
; HOW: zPaused, the kinds at $3C00; EffectDispatch for each slot, the slot kept in zRichterObject's byte across it; kind
; 5 at slot 3 is entered with X 3.
; WAITS: no.
; Entry: Y only for kind 5's block moves. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last handler
; left; C, Y and V as the last handler left them.
EffectStep1DDA:
    bbr0 zPaused,EffectStep1DDA_at0
    ldx #$02
    bsr EffectStep1DDA_at1
    ldx #$0B
EffectStep1DDA_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch1DDA
    rts
EffectStep1DDA_at2:
    ldx #$03
    jmp EffectFlash1DDA
EffectStep1DDA_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep1DDA_at2
    ldx #$0B
EffectStep1DDA_at4:
    lda a:ScriptSlotKind,x
    beq EffectStep1DDA_at3
    stx z:zRichterObject
    bsr EffectDispatch1DDA
    ldx z:zRichterObject
EffectStep1DDA_at3:
    dex
    bpl EffectStep1DDA_at4
    rts

; @routine 1DDA:2DA EffectDispatch1DDA
; @run 6C:82DA
; @stop 82EA
; @unreachable 6C:82DA-6C:82E9 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82DA runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sends a palette effect slot to the handler its kind names: the table at $82EA holds one address per
; kind, kind 1 first.
; HOW: The kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01 and is jumped through,
; with A the word's high byte, Y the index, C bit 7 of the kind less one, X the slot.
; WAITS: no.
; Entry: A the kind (not zero), X the slot. Exit: as the handler leaves them.
EffectDispatch1DDA:
    dec a
    asl a
    tay
    lda a:$82EA,y
    sta z:zTemp00
    lda a:$82EB,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1DDA:2F4 EffectList1DDA
; @run 6C:82F4
; @unreachable 6C:82F4-6C:8353 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82F4 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $89A7); each palette loaded by the packed palette loader at $84CA, whose bytes are LoadPackedPaletteC's.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList1DDA:
    lda a:ScriptSlotTimer,x
    beq EffectList1DDA_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList1DDA_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList1DDA_at1
    asl a
    tay
    lda a:$89A7,y
    sta z:zTemp00
    lda a:$89A8,y
    sta z:zTemp01
    bra EffectList1DDA_at2
EffectList1DDA_at1:
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
EffectList1DDA_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList1DDA_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList1DDA_at3
    jsr $84CA
    inc a:ScriptSlotField18,x
    bra EffectList1DDA
EffectList1DDA_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList1DDA_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList1DDA_at4

; @routine 1DDA:354 ClearEffectSlot1DDA
; @run 6C:8354
; @unreachable 6C:8354-6C:836C no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8354 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot1DDA:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 1DDA:36D EffectFade1DDA
; @run 6C:836D
; @unreachable 6C:836D-6C:8415 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $836D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at $9416; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade1DDA:
    lda a:ScriptSlotTimer,x
    beq EffectFade1DDA_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade1DDA_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade1DDA_at1
    jmp ClearEffectSlot1DDA
EffectFade1DDA_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade1DDA_at2
    lda a:$9416,y
    sta z:zTemp00
    lda a:$9417,y
    sta z:zTemp01
    bra EffectFade1DDA_at3
EffectFade1DDA_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade1DDA_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade1DDA_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade1DDA_at4
    rts
EffectFade1DDA_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade1DDA_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade1DDA_at5
    dec a
EffectFade1DDA_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade1DDA_at6
    sec
    sbc #$08
EffectFade1DDA_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade1DDA_at7
    sec
    sbc #$20
EffectFade1DDA_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade1DDA_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade1DDA_at9

; @routine 1DDA:416 EffectShade1DDA
; @run 6C:8416
; @stop 84CA
; @unreachable 6C:8416-6C:84C9 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8416 runs whatever overlay another scene
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
EffectShade1DDA:
    lda a:ScriptSlotTimer,x
    beq EffectShade1DDA_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade1DDA_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade1DDA_at1
    jmp ClearEffectSlot1DDA
EffectShade1DDA_at1:
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
EffectShade1DDA_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade1DDA_at2
    rts
EffectShade1DDA_at2:
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
EffectShade1DDA_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade1DDA_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade1DDA_at3
    cla
EffectShade1DDA_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade1DDA_at4
    cla
EffectShade1DDA_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade1DDA_at5
    cla
EffectShade1DDA_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade1DDA_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade1DDA_at7
    inc z:zTemp03
EffectShade1DDA_at7:
    dec z:zTemp05
    bpl EffectShade1DDA_at8
    ldy z:zHitSlot
    jmp EffectShade1DDA_at9

; @routine 1DDA:50A EffectColour1DDA
; @run 6C:850A
; @unreachable 6C:850A-6C:8543 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $850A runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at $89A7; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour1DDA:
    lda a:ScriptSlotScript,x
    bmi EffectColour1DDA_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour1DDA_at1
EffectColour1DDA_at0:
    asl a
    asl a
    adc #$A7
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour1DDA_at1:
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
    jmp ClearEffectSlot1DDA

; @routine 1DDA:544 EffectFlash1DDA
; @run 6C:8544
; @stop 05CF
; @unreachable 6C:8544-6C:85CE no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8544 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8676 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86A7, $88A7 and $3C60;
; the colour table at $85F6 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash1DDA:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash1DDA_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$86A7,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$88A7,$0100
EffectFlash1DDA_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash1DDA_at1
    jmp EffectFlashEnd1DDA
EffectFlash1DDA_at1:
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
    lda a:$85F6,y
    sta z:zTemp02
    lda a:$85F7,y
    sta z:zTemp03
    cly
EffectFlash1DDA_at4:
    lda a:$8676,y
    cmp #$FF
    bne EffectFlash1DDA_at2
    rts
EffectFlash1DDA_at2:
    sta z:zTemp08
    iny
    lda a:$8676,y
    sta z:zTemp09
    iny
    clx
EffectFlash1DDA_at3:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash1DDA_at3
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash1DDA_at4

; @routine 1DDA:5CF EffectFlashEnd1DDA
; @run 6C:85CF
; @unreachable 6C:85CF-6C:85F5 no read the game makes loads disc sector 1DDA, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85CF runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $86A7 (256 colours) and $88A7 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd1DDA:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $86A7,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $88A7,VCE_DATA_LO,$0100
    jmp ClearEffectSlot1DDA

; @routine 25F7:29F EffectStep25F7
; @run 6C:829F
; @unreachable 6C:829F-6C:82CA no read the game makes loads disc sector 25F7, so these bytes are never in memory.
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
EffectStep25F7:
    bbr0 zPaused,EffectStep25F7_at0
    ldx #$02
    bsr EffectStep25F7_at1
    ldx #$0B
EffectStep25F7_at1:
    lda a:ScriptSlotKind,x
    bne EffectDispatch25F7
    rts
EffectStep25F7_at4:
    ldx #$03
    jmp EffectFlash25F7
EffectStep25F7_at0:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq EffectStep25F7_at4
    ldx #$0B
EffectStep25F7_at3:
    lda a:ScriptSlotKind,x
    beq EffectStep25F7_at2
    stx z:zRichterObject
    bsr EffectDispatch25F7
    ldx z:zRichterObject
EffectStep25F7_at2:
    dex
    bpl EffectStep25F7_at3
    rts

; @routine 25F7:2CB EffectDispatch25F7
; @run 6C:82CB
; @stop 82DB
; @unreachable 6C:82CB-6C:82DA no read the game makes loads disc sector 25F7, so these bytes are never in memory.
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
EffectDispatch25F7:
    dec a
    asl a
    tay
    lda a:$82DB,y
    sta z:zTemp00
    lda a:$82DC,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 25F7:2E9 EffectList25F7
; @run 6C:82E9
; @unreachable 6C:82E9-6C:8348 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $82E9 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 1: a palette effect that plays a list of palette loads; once its timer has run down it takes the
; list's palettes one a step, the timer set from the list's first byte; at the list's end it counts a round and starts
; again, and when the rounds the list names have been played it gives the slot back.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9400 (for a negative list number, a table at
; $89C3); each palette loaded by the packed palette loader at $84BF, whose bytes are LoadPackedPaletteC's.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; when finished, A the rounds named, N,
; Z and C from comparing it with the rounds played, Y 1; X kept.
EffectList25F7:
    lda a:ScriptSlotTimer,x
    beq EffectList25F7_at0
    dec a:ScriptSlotTimer,x
    rts
EffectList25F7_at0:
    lda a:ScriptSlotScript,x
    bpl EffectList25F7_at1
    asl a
    tay
    lda a:$89C3,y
    sta z:zTemp00
    lda a:$89C4,y
    sta z:zTemp01
    bra EffectList25F7_at2
EffectList25F7_at1:
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
EffectList25F7_at2:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
EffectList25F7_at4:
    lda a:ScriptSlotField18,x
    asl a
    asl a
    inc a
    inc a
    tay
    lda (zTemp00),y
    cmp #$FF
    beq EffectList25F7_at3
    jsr LoadPackedPalette
    inc a:ScriptSlotField18,x
    bra EffectList25F7
EffectList25F7_at3:
    inc a:ScriptSlotField24,x
    stz a:ScriptSlotField18,x
    ldy #$01
    lda (zTemp00),y
    bmi EffectList25F7_at4
    and #$7F
    cmp a:ScriptSlotField24,x
    bcs EffectList25F7_at4

; @routine 25F7:349 ClearEffectSlot25F7
; @run 6C:8349
; @unreachable 6C:8349-6C:8361 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8349 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Gives a palette effect slot back: its eight fields cleared.
; HOW: Eight STZ of the slot's columns, the kind first.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
ClearEffectSlot25F7:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts

; @routine 25F7:362 EffectFade25F7
; @run 6C:8362
; @unreachable 6C:8362-6C:840A no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8362 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 2: a palette effect that fades palettes down: once its timer has run down it counts a round, gives
; the slot back after the eighth, and otherwise takes the list's palettes (a VCE address and colours each) and darkens
; every colour of them by one step in each of its three components, writing them back to the VCE.
; HOW: The slot's fields at $3C00 and up; the list through the pointer at $9402, or for a negative list number from the
; table at $9410; the VCE read into $3C60 by a TAI, changed word by word, and written back by a TIA.
; WAITS: no.
; Entry: X the slot. Exit: while waiting, A the timer, N and Z from its new value; at a list's end A $FF, Z and C set, X
; $20 when a palette was changed (else the slot), Y the list's place, V from the last subtract run (else kept).
EffectFade25F7:
    lda a:ScriptSlotTimer,x
    beq EffectFade25F7_at0
    dec a:ScriptSlotTimer,x
    rts
EffectFade25F7_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectFade25F7_at1
    jmp ClearEffectSlot25F7
EffectFade25F7_at1:
    lda a:ScriptSlotScript,x
    asl a
    tay
    bcc EffectFade25F7_at2
    lda a:$9410,y
    sta z:zTemp00
    lda a:$9411,y
    sta z:zTemp01
    bra EffectFade25F7_at3
EffectFade25F7_at2:
    lda a:$9402
    sta z:zBiosAl
    lda a:$9403
    sta z:zBiosAh
    lda (zBiosAl),y
    sta z:zTemp00
    iny
    lda (zBiosAl),y
    sta z:zTemp01
EffectFade25F7_at3:
    cly
    lda (zTemp00),y
    sta a:ScriptSlotTimer,x
    iny
EffectFade25F7_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectFade25F7_at4
    rts
EffectFade25F7_at4:
    sta z:zTemp08
    sta a:VCE_ADDR_LO
    iny
    lda (zTemp00),y
    sta z:zTemp09
    sta a:VCE_ADDR_HI
    iny
    tai VCE_DATA_LO,SavedColoursLo,$0020
    clx
EffectFade25F7_at8:
    lda a:SavedColoursLo,x
    and #$07
    beq EffectFade25F7_at5
    dec a
EffectFade25F7_at5:
    sta z:zTemp02
    lda a:SavedColoursLo,x
    and #$38
    beq EffectFade25F7_at6
    sec
    sbc #$08
EffectFade25F7_at6:
    ora z:zTemp02
    sta z:zTemp02
    lda a:SavedColoursLo,x
    lsr a:SavedColoursLo+1,x
    ror a
    and #$E0
    beq EffectFade25F7_at7
    sec
    sbc #$20
EffectFade25F7_at7:
    asl a
    rol a:SavedColoursLo+1,x
    ora z:zTemp02
    sta a:SavedColoursLo,x
    inx
    inx
    cpx #$20
    bcc EffectFade25F7_at8
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFade25F7_at9

; @routine 25F7:40B EffectShade25F7
; @run 6C:840B
; @stop 84BF
; @unreachable 6C:840B-6C:84BE no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $840B runs whatever overlay another scene
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
EffectShade25F7:
    lda a:ScriptSlotTimer,x
    beq EffectShade25F7_at0
    dec a:ScriptSlotTimer,x
    rts
EffectShade25F7_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$08
    bcc EffectShade25F7_at1
    jmp ClearEffectSlot25F7
EffectShade25F7_at1:
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
EffectShade25F7_at9:
    lda (zTemp00),y
    cmp #$FF
    bne EffectShade25F7_at2
    rts
EffectShade25F7_at2:
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
EffectShade25F7_at8:
    ldy #$01
    lda (zTemp02),y
    sta z:zTemp04
EffectShade25F7_at6:
    iny
    lda (zTemp02),y
    and #$07
    sec
    sbc z:zTemp08
    bcs EffectShade25F7_at3
    cla
EffectShade25F7_at3:
    sta z:zTemp0B
    lda (zTemp02),y
    and #$38
    sec
    sbc z:zTemp09
    bcs EffectShade25F7_at4
    cla
EffectShade25F7_at4:
    ora z:zTemp0B
    sta z:zTemp0B
    lda (zTemp02),y
    asl z:zTemp04
    ror a
    and #$E0
    sec
    sbc z:zTemp0A
    bcs EffectShade25F7_at5
    cla
EffectShade25F7_at5:
    asl a
    ora z:zTemp0B
    sta a:VCE_DATA_LO
    cla
    rol a
    sta a:VCE_DATA_HI
    cpy #$09
    bcc EffectShade25F7_at6
    lda z:zTemp02
    clc
    adc #$09
    sta z:zTemp02
    bcc EffectShade25F7_at7
    inc z:zTemp03
EffectShade25F7_at7:
    dec z:zTemp05
    bpl EffectShade25F7_at8
    ldy z:zHitSlot
    jmp EffectShade25F7_at9

; @routine 25F7:4FF EffectColour25F7
; @run 6C:84FF
; @unreachable 6C:84FF-6C:8538 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $84FF runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 4: a palette effect that sets one VCE colour and ends: four bytes of a table (a VCE address, the
; colour's two bytes) picked by the list number, the slot then given back.
; HOW: The list number times 4 (the carry the shifts leave added in, as the original does) indexes the table at $9406,
; or for a negative number the table at $89C3; the four bytes go to the VCE ports.
; WAITS: no.
; Entry: X the slot. Exit: A the last byte read, N and Z from it, Y 3, C and V from the table address's high byte add; X
; kept.
EffectColour25F7:
    lda a:ScriptSlotScript,x
    bmi EffectColour25F7_at0
    asl a
    asl a
    adc a:$9406
    sta z:zTemp00
    cla
    adc a:$9407
    sta z:zTemp01
    bra EffectColour25F7_at1
EffectColour25F7_at0:
    asl a
    asl a
    adc #$C3
    sta z:zTemp00
    cla
    adc #$89
    sta z:zTemp01
EffectColour25F7_at1:
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
    jmp ClearEffectSlot25F7

; @routine 25F7:539 EffectVceSave25F7
; @run 6C:8539
; @unreachable 6C:8539-6C:8542 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8539 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 6: all 512 colours of the VCE saved in the Super RAM bank mapped at $A000, at $B000.
; HOW: SelectSavedBank, then a TAI of $400 bytes from the VCE's data port.
; WAITS: no.
; Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
EffectVceSave25F7:
    bsr SelectSavedBank25F7
    tai VCE_DATA_LO,$B000,$0400
    rts

; @routine 25F7:543 EffectVceRestore25F7
; @run 6C:8543
; @unreachable 6C:8543-6C:854C no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8543 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 7: the 512 colours written back to the VCE from the Super RAM bank mapped at $A000, at $B000.
; HOW: SelectSavedBank, then a TIA of $400 bytes to the VCE's data port.
; WAITS: no.
; Entry: X the slot, Y kept for the block move. Exit: as SelectSavedBank.
EffectVceRestore25F7:
    bsr SelectSavedBank25F7
    tia $B000,VCE_DATA_LO,$0400
    rts

; @routine 25F7:54D SelectSavedBank25F7
; @run 6C:854D
; @unreachable 6C:854D-6C:855F no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $854D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Maps the Super RAM bank of the saved palette at $A000 and sets the VCE's colour address to 0.
; HOW: The bank is the base bank plus $12, or plus $30 when bit 0 of zIdleFlag is set; TAM for MPR5.
; WAITS: no.
; Entry: nothing. Exit: A the bank, N and Z from it, C and V from the add; X and Y kept.
SelectSavedBank25F7:
    lda #$12
    bbr0 zIdleFlag,SelectSavedBank25F7_at0
    lda #$30
SelectSavedBank25F7_at0:
    clc
    adc z:zRamBaseBank
    tam #$20
    stz a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    rts

; @routine 25F7:560 EffectFlash25F7
; @run 6C:8560
; @stop 05EB
; @unreachable 6C:8560-6C:85EA no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8560 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Kind 5: a palette effect that flashes: the first time it saves the VCE's background and sprite palettes
; in the overlay's RAM; each round it fills a 16 colour buffer with one colour of its table and writes it over every
; palette the list at $8692 names; after the sixteenth round it puts the saved palettes back and gives the slot up.
; HOW: Zero page $3C set to 1; the count in $3C24; TAI and TIA between the VCE's data port and $86C3, $88C3 and $3C60;
; the colour table at $8612 indexed by the list number times 32 and the count times 2.
; WAITS: no.
; Entry: X the slot, Y kept for the block moves. Exit: A $FF at the list's end with Z and C set, X $20 (or the slot when
; no palette was written), Y the list's place, V from the table index's add; after the sixteenth round as EffectFlashEnd
; with C set.
EffectFlash25F7:
    lda #$01
    sta z:$3C
    lda a:ScriptSlotField24,x
    bne EffectFlash25F7_at0
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$86C3,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tai VCE_DATA_LO,$88C3,$0100
EffectFlash25F7_at0:
    inc a:ScriptSlotField24,x
    lda a:ScriptSlotField24,x
    cmp #$10
    bcc EffectFlash25F7_at4
    jmp EffectFlashEnd25F7
EffectFlash25F7_at4:
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
    lda a:$8612,y
    sta z:zTemp02
    lda a:$8613,y
    sta z:zTemp03
    cly
EffectFlash25F7_at3:
    lda a:$8692,y
    cmp #$FF
    bne EffectFlash25F7_at1
    rts
EffectFlash25F7_at1:
    sta z:zTemp08
    iny
    lda a:$8692,y
    sta z:zTemp09
    iny
    clx
EffectFlash25F7_at2:
    lda z:zTemp02
    sta a:SavedColoursLo,x
    lda z:zTemp03
    sta a:SavedColoursLo+1,x
    inx
    inx
    cpx #$20
    bcc EffectFlash25F7_at2
    lda z:zTemp08
    sta a:VCE_ADDR_LO
    lda z:zTemp09
    sta a:VCE_ADDR_HI
    tia SavedColoursLo,VCE_DATA_LO,$0020
    jmp EffectFlash25F7_at3

; @routine 25F7:5EB EffectFlashEnd25F7
; @run 6C:85EB
; @unreachable 6C:85EB-6C:8611 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $85EB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The end of kind 5: the two saved palettes written back to the VCE, the flag at zero page $3C cleared,
; the slot given back.
; HOW: Two TIA from $86C3 (256 colours) and $88C3 (128) to the VCE; the slot cleared by ClearEffectSlot's bytes.
; WAITS: no.
; Entry: X the slot, Y kept. Exit: A $80, N set, Z clear; X, Y, C and V kept.
EffectFlashEnd25F7:
    stz z:$3C
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia $86C3,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia $88C3,VCE_DATA_LO,$0100
    jmp ClearEffectSlot25F7
