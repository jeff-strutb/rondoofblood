; @card 02:D706 psg_wave_offset
; WHAT IT DOES: Makes a waveform's number into its offset in the table: the number times 32, in _EX and _EY zero page.
; HOW: Three shifts through zero page $E8.
; WAITS: no.
; Entry: A the number. Exit: $E8 the low byte and $E9 the high; A the high byte, C the last bit shifted out. N, Z from the last ROR; X, Y kept.
psg_wave_offset:
    stz z:$E8
    lsr a
    ror z:$E8
    lsr a
    ror z:$E8
    lsr a
    ror z:$E8
    sta z:$E9
    rts

; @card 02:D623 psg_tempo_tick
; WHAT IT DOES: Counts the sound's fade-out time: while the fade is on, adds the step to a phase and counts its overflows,
; and when the count reaches $1E asks every channel that is playing to stop.
; HOW: $2613 the fade's step (zero: off), added to $2614 in seven bits; a carry into bit 7 is one tick of $2615; at $1E ticks every
; channel whose state ($2607 plus the channel) is not zero is set to 2.
; WAITS: no.
; Entry: nothing. Exit: A, X and the flags as the last step left them: off, A 0, Z; below the limit, A the ticks, C 0; else X $FF,
; N, C 1.
psg_tempo_tick:
    lda a:$2613
    beq psg_tempo_tick_at3
    clc
    adc a:$2614
    bpl psg_tempo_tick_at0
    inc a:$2615
    and #$7F
psg_tempo_tick_at0:
    sta a:$2614
    lda a:$2615
    cmp #$1E
    bcc psg_tempo_tick_at3
    ldx #$05
psg_tempo_tick_at1:
    lda a:$2607,x
    beq psg_tempo_tick_at2
    lda #$02
    sta a:$2607,x
psg_tempo_tick_at2:
    dex
    bpl psg_tempo_tick_at1
psg_tempo_tick_at3:
    rts

; @card 02:D64D psg_write_channels
; WHAT IT DOES: Writes the six channels' state to the PSG, from channel 5 down to 0: a channel asked to stop is silenced and forgotten; a
; new one gets its waveform, noise or frequency, balance and volume.
; HOW: Each channel's state at $2607: below 1 skipped; 2 and up: key off, state cleared, and the channel's busy bit cleared in $2474
; unless the player's mode is 1 or the channel is not used; 1: a new note: the waveform ($25FF bit 7 not yet set) copied as 32 bytes
; from the built-in table at $D9A6 or the user's at $22D2, then noise (channels 4 and 5, if bit 7 of $2601 is clear) or frequency,
; the balance, and the volume less the fade ($2615), written with the key on, or the key off if none is left.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N from it; A and the other flags as the last channel left them.
; The PSG's registers at $0800 to $0807 and the arrays are by address; psg_wave_offset's frames are at $D68E and $D6A3.
psg_write_channels:
    ldx #$05
psg_write_channels_at0:
    lda a:$2607,x
    cmp #$01
    bpl psg_write_channels_at1
    jmp psg_write_channels_at18
psg_write_channels_at1:
    stx a:$0800
    beq psg_write_channels_at4
    stz a:$0804
    stz a:$2607,x
    lda a:$22DF
    cmp #$01
    beq psg_write_channels_at2
    lda a:$247C,x
    beq psg_write_channels_at2
    lda a:$2474,x
    and #$7F
    sta a:$2474,x
psg_write_channels_at2:
    jmp psg_write_channels_at18
psg_write_channels_at4:
    lda a:$25FF,x
    bmi psg_write_channels_at9
    ora #$80
    sta a:$25FF,x
    and #$7F
    cmp #$2D
    bcs psg_write_channels_at5
    jsr psg_wave_offset
    lda #$A6
    clc
    adc z:$E8
    sta z:$E8
    lda #$D9
    adc z:$E9
    sta z:$E9
    bra psg_write_channels_at6
psg_write_channels_at5:
    sec
    sbc #$2D
    jsr psg_wave_offset
    lda a:$22D2
    clc
    adc z:$E8
    sta z:$E8
    lda a:$22D3
    adc z:$E9
    sta z:$E9
psg_write_channels_at6:
    lda #$40
    sta a:$0804
    stz a:$0804
    cly
psg_write_channels_at7:
    lda (<$E8),y
    sta a:$0806
    iny
    cpy #$20
    bcc psg_write_channels_at7
psg_write_channels_at9:
    cpx #$04
    bcc psg_write_channels_at10
    lda a:$2601,x
    bmi psg_write_channels_at10
    ora #$80
    sta a:$0807
    bra psg_write_channels_at11
psg_write_channels_at10:
    stz a:$0807
    lda a:$25E7,x
    sta a:$0802
    lda a:$25ED,x
    sta a:$0803
psg_write_channels_at11:
    lda a:$25F9,x
    sta a:$0805
    lda a:$25F3,x
    sec
    sbc a:$2615
    beq psg_write_channels_at12
    bpl psg_write_channels_at13
psg_write_channels_at12:
    stz a:$0804
    bra psg_write_channels_at18
psg_write_channels_at13:
    ora #$80
    sta a:$0804
psg_write_channels_at18:
    dex
    bmi psg_write_channels_at19
    jmp psg_write_channels_at0
psg_write_channels_at19:
    rts

; @card 02:D257 psg_slide_channel
; WHAT IT DOES: Moves the current channel's pitch, left volume and right volume one step towards where its slides are taking them.
; HOW: Each slide has a signed step ($255D, $2569, $2575 plus the channel) and a fraction ($2563, $256F, $257B): the step added to
; the fraction in seven bits, the carry out of bit 6 into bit 7 added to (or taken from) the whole part ($24AF, the nibbles of $25F9),
; clipped at its limit, where the slide ends by clearing its step.
; WAITS: no.
; Entry: CardPlayChannel ($22E0) the channel. Exit: A, X and the flags as the last step left them.
psg_slide_channel:
    ldx a:$22E0
    lda a:$255D,x
    beq psg_slide_channel_at5
    bmi psg_slide_channel_at2
    clc
    adc a:$2563,x
    sta z:$EA
    rol a
    rol a
    and #$01
    clc
    adc a:$24AF,x
    beq psg_slide_channel_at4
    bmi psg_slide_channel_at4
    cla
    stz a:$255D,x
    bra psg_slide_channel_at4
psg_slide_channel_at2:
    eor #$FF
    inc a
    clc
    adc a:$2563,x
    sta z:$EA
    rol a
    rol a
    and #$01
    sta z:$EB
    lda a:$24AF,x
    sec
    sbc z:$EB
    beq psg_slide_channel_at4
    cmp #$E1
    bcs psg_slide_channel_at4
    lda #$E1
    stz a:$255D,x
psg_slide_channel_at4:
    sta a:$24AF,x
    lda z:$EA
    and #$7F
    sta a:$2563,x
psg_slide_channel_at5:
    lda a:$2569,x
    beq psg_slide_channel_at12
    bmi psg_slide_channel_at7
    clc
    adc a:$256F,x
    sta z:$EA
    rol a
    rol a
    and #$01
    sta z:$EB
    lda a:$25F9,x
    and #$0F
    clc
    adc z:$EB
    cmp #$10
    bcc psg_slide_channel_at9
    lda #$0F
    stz a:$2569,x
    bra psg_slide_channel_at9
psg_slide_channel_at7:
    eor #$FF
    inc a
    clc
    adc a:$256F,x
    sta z:$EA
    rol a
    rol a
    and #$01
    sta z:$EB
    lda a:$25F9,x
    and #$0F
    sec
    sbc z:$EB
    bne psg_slide_channel_at9
    cla
    stz a:$2569,x
psg_slide_channel_at9:
    sta z:$EB
    lda a:$25F9,x
    and #$F0
    ora z:$EB
    sta a:$25F9,x
    lda z:$EA
    and #$7F
    sta a:$256F,x
psg_slide_channel_at12:
    lda a:$2575,x
    beq psg_slide_channel_at19
    bmi psg_slide_channel_at14
    clc
    adc a:$257B,x
    sta z:$EA
    and #$80
    lsr a
    lsr a
    lsr a
    sta z:$EB
    lda a:$25F9,x
    and #$F0
    clc
    adc z:$EB
    bcc psg_slide_channel_at17
    lda #$F0
    stz a:$2575,x
    bra psg_slide_channel_at17
psg_slide_channel_at14:
    eor #$FF
    inc a
    clc
    adc a:$257B,x
    sta z:$EA
    and #$80
    lsr a
    lsr a
    lsr a
    sta z:$EB
    lda a:$25F9,x
    and #$F0
    sec
    sbc z:$EB
    bne psg_slide_channel_at17
    cla
    stz a:$2575,x
psg_slide_channel_at17:
    sta z:$EB
    lda a:$25F9,x
    and #$0F
    ora z:$EB
    sta a:$25F9,x
    lda z:$EA
    and #$7F
    sta a:$257B,x
psg_slide_channel_at19:
    rts

; @card 02:D350 psg_read_notes
; WHAT IT DOES: Reads the current channel's note stream when the channel is in the state that wants it: a note's pitch, the sound's
; shape (its waveform pair), the volume or the channel flags, until a note or the stream's end.
; HOW: Only for a channel whose kind ($25E1, high nibble) is 1 and whose state ($260D) is not zero. Each byte of the stream at the
; pointer kept in $259F and $25A5: below $B0 a noise value (channels 4 and 5); $B0 to $BF the high pitch and the next byte the low, and
; the note is read; $C0 a shape number, built-in or the user's, $D0 the balance, $E0 the flags; anything else ends the stream. A
; shape, balance or flags byte goes on to the next byte.
; WAITS: no.
; Entry: CardPlayChannel ($22E0) the channel. Exit: A, X, Y and the flags as the last step left them.
psg_read_notes:
    ldy a:$22E0
    lda a:$25E1,y
    and #$F0
    cmp #$10
    beq psg_read_notes_at0
psg_read_notes_at22:
    rts
psg_read_notes_at0:
    lda a:$260D,y
    beq psg_read_notes_at22
    lda a:$259F,y
    sta z:$E8
    lda a:$25A5,y
    sta z:$E9
psg_read_notes_at1:
    lda (<$E8)
    inc z:$E8
    bne psg_read_notes_at2
    inc z:$E9
psg_read_notes_at2:
    tax
    cmp #$B0
    bcc psg_read_notes_at7
    and #$F0
    cmp #$B0
    beq psg_read_notes_at9
    cmp #$C0
    beq psg_read_notes_at12
    cmp #$D0
    beq psg_read_notes_at17
    cmp #$E0
    beq psg_read_notes_at18
    jmp psg_read_notes_at19
psg_read_notes_at7:
    cpy #$04
    bcc psg_read_notes_at11
    and #$1F
    sta a:$2601,y
    bra psg_read_notes_at11
psg_read_notes_at9:
    txa
    sec
    sbc #$B0
    sta a:$25ED,y
    lda (<$E8)
    inc z:$E8
    bne psg_read_notes_at10
    inc z:$E9
psg_read_notes_at10:
    sta a:$25E7,y
    cpy #$04
    bcc psg_read_notes_at11
    lda #$80
    sta a:$2601,y
psg_read_notes_at11:
    lda z:$E8
    sta a:$259F,y
    lda z:$E9
    sta a:$25A5,y
    rts
psg_read_notes_at12:
    lda (<$E8)
    cmp #$10
    bcs psg_read_notes_at13
    asl a
    tax
    lda a:$D8A1,x
    sta a:$24D9,y
    lda a:$D8A2,x
    sta a:$24DF,y
    bra psg_read_notes_at20
psg_read_notes_at13:
    sec
    sbc #$10
    asl a
    clc
    adc a:$22D5
    sta z:$EA
    cla
    adc a:$22D6
    sta z:$EB
    lda (<$EA)
    sta a:$24D9,y
    inc z:$EA
    bne psg_read_notes_at14
    inc z:$EB
psg_read_notes_at14:
    lda (<$EA)
    sta a:$24DF,y
    bra psg_read_notes_at20
psg_read_notes_at17:
    lda (<$E8)
    sta a:$25F9,y
    bra psg_read_notes_at20
psg_read_notes_at18:
    lda (<$E8)
    sta a:$25FF,y
    bra psg_read_notes_at20
psg_read_notes_at19:
    cla
    sta a:$260D,y
    lda z:$E8
    sec
    sbc #$01
    sta a:$259F,y
    lda z:$E9
    sbc #$00
    sta a:$25A5,y
    rts
psg_read_notes_at20:
    inc z:$E8
    bne psg_read_notes_at21
    inc z:$E9
psg_read_notes_at21:
    jmp psg_read_notes_at1

; @card 02:D510 psg_pitch_effects
; WHAT IT DOES: Works the current channel's pitch effects for the frame: the base pitch put back, then its vibrato, its detune, its pitch
; slide through a list and its portamento through another.
; HOW: Only for a channel of kind 0 ($25E1, high nibble). The frequency set from its base ($249D, $24A3), then: vibrato ($2581 step, $2593 on):
; counted down in $2599, then the accumulated amount ($2587, $258D) added, else the step scaled, accumulated and added; detune ($2557):
; scaled and added; slide ($254B on): each time $2551 reaches 0 the next byte of the list at ($2539, $253F) from $2545, $80 ends it
; (stepping back one byte); portamento ($2527 on): the same with the list at ($2515, $251B) from $2521, $80 sending it back to its start.
; WAITS: no.
; Entry: CardPlayChannel ($22E0) the channel. Exit: A, Y and the flags as the last step left them; X the channel.
psg_pitch_effects:
    ldx a:$22E0
    lda a:$25E1,x
    and #$F0
    beq psg_pitch_effects_at0
    rts
psg_pitch_effects_at0:
    lda a:$249D,x
    sta a:$25E7,x
    lda a:$24A3,x
    sta a:$25ED,x
    lda a:$2581,x
    tay
    beq psg_pitch_effects_at4
    lda a:$2593,x
    beq psg_pitch_effects_at2
    dec a:$2599,x
    bne psg_pitch_effects_at2
    inc a:$2599,x
    lda a:$2587,x
    sta z:$EA
    lda a:$258D,x
    sta z:$EB
    jsr psg_add_pitch
    bra psg_pitch_effects_at4
psg_pitch_effects_at2:
    sty z:$EA
    jsr psg_scale_amount
    lda a:$2587,x
    clc
    adc z:$EA
    sta a:$2587,x
    sta z:$EA
    lda a:$258D,x
    adc z:$EB
    sta a:$258D,x
    sta z:$EB
    jsr psg_add_pitch
psg_pitch_effects_at4:
    lda a:$2557,x
    beq psg_pitch_effects_at5
    sta z:$EA
    jsr psg_scale_amount
    jsr psg_add_pitch
psg_pitch_effects_at5:
    lda a:$254B,x
    beq psg_pitch_effects_at11
    dec a:$2551,x
    bne psg_pitch_effects_at11
    inc a:$2551,x
    lda a:$2539,x
    sta z:$E8
    lda a:$253F,x
    sta z:$E9
    ldy a:$2545,x
    lda (<$E8),y
    cmp #$80
    bne psg_pitch_effects_at7
    cpy #$00
    beq psg_pitch_effects_at11
    dey
    lda (<$E8),y
psg_pitch_effects_at7:
    sta z:$EA
    phy
    jsr psg_scale_amount
    jsr psg_add_pitch
    ply
    iny
    bne psg_pitch_effects_at8
    dey
psg_pitch_effects_at8:
    tya
    sta a:$2545,x
psg_pitch_effects_at11:
    lda a:$2527,x
    beq psg_pitch_effects_at15
    dec a:$252D,x
    bne psg_pitch_effects_at15
    inc a:$252D,x
    lda a:$2515,x
    sta z:$E8
    lda a:$251B,x
    sta z:$E9
    ldy a:$2521,x
psg_pitch_effects_at12:
    lda (<$E8),y
    cmp #$80
    bne psg_pitch_effects_at14
    cpy #$00
    beq psg_pitch_effects_at15
    stz a:$2521,x
    cly
    bra psg_pitch_effects_at12
psg_pitch_effects_at14:
    sta z:$EA
    jsr psg_scale_amount
    jsr psg_add_pitch
    inc a:$2521,x
psg_pitch_effects_at15:
    rts

; @card 02:D5E2 psg_scale_amount
; WHAT IT DOES: Makes the byte in $EA a signed 16-bit amount in $EA and $EB, scaled by the channel's scale less its base.
; HOW: $EB the sign of $EA; the channel's scale ($2533) less its base ($2497) as a shift count: left for plus, right for minus; a
; right shift that leaves something in the high byte makes it $FF.
; WAITS: no.
; Entry: X the channel, $EA the byte. Exit: $EA and $EB the amount; A, Y and the flags as the last step left them. X kept.
psg_scale_amount:
    stz z:$EB
    lda z:$EA
    bpl psg_scale_amount_at0
    dec z:$EB
psg_scale_amount_at0:
    lda a:$2533,x
    beq psg_scale_amount_at6
    sec
    sbc a:$2497,x
    tay
    bmi psg_scale_amount_at3
psg_scale_amount_at1:
    beq psg_scale_amount_at6
    asl z:$EA
    rol z:$EB
    dey
    bra psg_scale_amount_at1
psg_scale_amount_at3:
    beq psg_scale_amount_at4
    lsr z:$EB
    ror z:$EA
    iny
    bra psg_scale_amount_at3
psg_scale_amount_at4:
    lda z:$EB
    beq psg_scale_amount_at6
    lda #$FF
    sta z:$EB
psg_scale_amount_at6:
    rts

; @card 02:D611 psg_add_pitch
; WHAT IT DOES: Adds the amount in $EA and $EB to the current channel's frequency.
; HOW: A 16-bit add to $25E7 and $25ED, plus the channel.
; WAITS: no.
; Entry: X the channel, $EA and $EB the amount. Exit: A the high byte, C and V from its add, N and Z from it. X, Y kept.
psg_add_pitch:
    lda a:$25E7,x
    clc
    adc z:$EA
    sta a:$25E7,x
    lda a:$25ED,x
    adc z:$EB
    sta a:$25ED,x
    rts

; @card 02:D422 psg_volume_envelope
; WHAT IT DOES: Works the current channel's volume envelope for the frame: the held part moves the level by its rates until its length is
; used up, the release moves it down by the release rates, and the next segment is read from the shape's list when the held part ends.
; HOW: State $260D: 0 off (volume 0); bit 7 the release; 2 ending (state counted down, position cleared, next segment); else the held part:
; $24BB counted down and the release bit set at 0, $250F the length left (0 reads the next segment), the level ($24EB, $24F1) moved by
; the rates ($24F7, $24FD). The list at ($24D9, $24DF) from $24E5: $FB skips three bytes, $FC sets the level then reads a segment (length, two
; rates), a length below $FB is a segment ($FF if 0), anything else holds. The volume is the level over 4 plus the channel's base ($24AF).
; WAITS: no.
; Entry: CardPlayChannel ($22E0) the channel. Exit: A, Y and the flags as the last step left them; X the channel.
psg_volume_envelope:
    ldx a:$22E0
    lda a:$260D,x
    bne psg_volume_envelope_at0
    jmp psg_volume_envelope_at13
psg_volume_envelope_at0:
    bmi psg_volume_envelope_at8
    cmp #$02
    beq psg_volume_envelope_at12
    dec a:$24BB,x
    bne psg_volume_envelope_at1
    ora #$80
    sta a:$260D,x
psg_volume_envelope_at1:
    lda a:$250F,x
    bne psg_volume_envelope_at2
    bra psg_volume_envelope_at18
psg_volume_envelope_at2:
    lda a:$24EB,x
    clc
    adc a:$24F7,x
    sta a:$24EB,x
    lda a:$24F1,x
    adc a:$24FD,x
    bpl psg_volume_envelope_at4
    cla
    sta a:$24EB,x
    ldy a:$24FD,x
    bmi psg_volume_envelope_at4
    lda #$7C
psg_volume_envelope_at4:
    sta a:$24F1,x
    lda a:$250F,x
    cmp #$FF
    beq psg_volume_envelope_at16
    dec a:$250F,x
    bra psg_volume_envelope_at16
psg_volume_envelope_at8:
    lda a:$24EB,x
    clc
    adc a:$2503,x
    sta a:$24EB,x
    lda a:$24F1,x
    adc a:$2509,x
    bpl psg_volume_envelope_at10
    cla
    sta a:$24EB,x
    ldy a:$2509,x
    bmi psg_volume_envelope_at10
    lda #$7C
psg_volume_envelope_at10:
    sta a:$24F1,x
    bra psg_volume_envelope_at16
psg_volume_envelope_at12:
    dec a:$260D,x
    stz a:$24E5,x
    bra psg_volume_envelope_at18
psg_volume_envelope_at13:
    stz a:$25F3,x
    bra psg_volume_envelope_at17
psg_volume_envelope_at15:
    tya
    sta a:$24E5,x
psg_volume_envelope_at16:
    lda a:$24F1,x
    lsr a
    lsr a
    clc
    adc a:$24AF,x
    bpl psg_volume_envelope_at17a
    cla
psg_volume_envelope_at17a:
    sta a:$25F3,x
psg_volume_envelope_at17:
    rts
psg_volume_envelope_at18:
    ldy a:$24E5,x
    lda a:$24D9,x
    sta z:$E8
    lda a:$24DF,x
    sta z:$E9
psg_volume_envelope_at19:
    lda (<$E8),y
    cmp #$FB
    bcc psg_volume_envelope_at21
    beq psg_volume_envelope_at24
    cmp #$FC
    beq psg_volume_envelope_at25
    bra psg_volume_envelope_at28
psg_volume_envelope_at21:
    cmp #$00
    bne psg_volume_envelope_at22
    dec a
psg_volume_envelope_at22:
    sta a:$250F,x
    iny
    lda (<$E8),y
    sta a:$24F7,x
    iny
    lda (<$E8),y
    sta a:$24FD,x
    iny
    bra psg_volume_envelope_at15
psg_volume_envelope_at24:
    iny
    iny
    iny
    bra psg_volume_envelope_at19
psg_volume_envelope_at25:
    iny
    lda (<$E8),y
    sta a:$24EB,x
    iny
    lda (<$E8),y
    sta a:$24F1,x
    iny
    lda (<$E8),y
    cmp #$FB
    bcc psg_volume_envelope_at21
    stz a:$250F,x
    bra psg_volume_envelope_at15
psg_volume_envelope_at28:
    lda #$FF
    sta a:$250F,x
    stz a:$24F7,x
    stz a:$24FD,x
    jmp psg_volume_envelope_at16
