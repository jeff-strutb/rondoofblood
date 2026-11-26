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

; @card 02:CD45 psg_start_song
; WHAT IT DOES: Starts the song that was asked for, if one was: sets up each channel the song's header names.
; HOW: $22E1 the song asked for (bit 7 set: none, or already started); the song's header found through the pointer at $22D0 and the list
; of headers, its first byte a mask of channels (bit 7 set: it is a song); for each channel in the mask the state, counts and effects
; arrays cleared, the default shape, balance and scale set, the channel's start pointer taken from the header, the flags set from
; $22E2 if the header's bit 6 says so; then the fade cleared, the song marked started and the player's done flag cleared.
; WAITS: no.
; Entry: nothing. Exit: A $40, X 6, Y the bytes of the header read, C 1, N and V from the done flags' old byte, Z from A and it.
psg_start_song:
    lda a:$22E1
    bpl psg_start_song_at0
psg_start_song_at12:
    rts
psg_start_song_at0:
    asl a
    tay
    lda a:$22D0
    sta z:$EA
    lda a:$22D1
    sta z:$EB
    lda (<$EA),y
    sta z:$E8
    iny
    lda (<$EA),y
    sta z:$E9
    lda (<$E8)
    bpl psg_start_song_at12
    sta z:$EA
    sta z:$EB
    cly
    clx
psg_start_song_at3:
    lsr z:$EA
    bcc psg_start_song_at10
    lda #$01
    sta a:$2607,x
    sta a:$24A9,x
    stz a:$260D,x
    stz a:$24B5,x
    stz a:$24AF,x
    stz a:$2527,x
    stz a:$254B,x
    stz a:$2533,x
    stz a:$2557,x
    stz a:$2581,x
    stz a:$2593,x
    stz a:$25E1,x
    stz a:$24CD,x
    stz a:$255D,x
    stz a:$2569,x
    stz a:$2575,x
    stz a:$24D3,x
    lda a:$D8A1
    sta a:$24D9,x
    lda a:$D8A2
    sta a:$24DF,x
    lda #$84
    sta a:$2509,x
    lda #$04
    sta a:$2497,x
    iny
    lda (<$E8),y
    sta a:$248B,x
    sta a:$24C1,x
    iny
    lda (<$E8),y
    sta a:$2491,x
    sta a:$24C7,x
    lda #$80
    cpx #$04
    bcc psg_start_song_at8
    sta a:$2601,x
psg_start_song_at8:
    sta a:$25FF,x
    bbr6 $EB,psg_start_song_at10
    lda a:$22E2
    sta a:$25FF,x
psg_start_song_at10:
    inx
    cpx #$06
    bcc psg_start_song_at3
    stz a:$2613
    stz a:$2615
    lda #$80
    tsb a:$22E1
    lda #$40
    trb a:$22DE
    rts

; @card 02:CC26 psg_tempo_tick_alt
; WHAT IT DOES: The second player's psg_tempo_tick (mode 0): counts the sound's fade-out time: while the fade is on, adds the step to a phase and counts its overflows,
; and when the count reaches $1E asks every channel that is playing to stop.
; HOW: $2613 the fade's step (zero: off), added to $2614 in seven bits; a carry into bit 7 is one tick of $2615; at $1E ticks every
; channel whose state ($2607 plus the channel) is not zero is set to 2.
; WAITS: no.
; Entry: nothing. Exit: A, X and the flags as the last step left them: off, A 0, Z; below the limit, A the ticks, C 0; else X $FF,
; N, C 1.
psg_tempo_tick_alt:
    lda a:$2488
    beq psg_tempo_tick_alt_at3
    clc
    adc a:$2489
    bpl psg_tempo_tick_alt_at0
    inc a:$248A
    and #$7F
psg_tempo_tick_alt_at0:
    sta a:$2489
    lda a:$248A
    cmp #$1E
    bcc psg_tempo_tick_alt_at3
    ldx #$05
psg_tempo_tick_alt_at1:
    lda a:$247C,x
    beq psg_tempo_tick_alt_at2
    lda #$02
    sta a:$247C,x
psg_tempo_tick_alt_at2:
    dex
    bpl psg_tempo_tick_alt_at1
psg_tempo_tick_alt_at3:
    rts

; @card 02:CD04 psg_wave_offset_alt
; WHAT IT DOES: The second player's psg_wave_offset: Makes a waveform's number into its offset in the table: the number times 32, in _EX and _EY zero page.
; HOW: Three shifts through zero page $E8.
; WAITS: no.
; Entry: A the number. Exit: $E8 the low byte and $E9 the high; A the high byte, C the last bit shifted out. N, Z from the last ROR; X, Y kept.
psg_wave_offset_alt:
    stz z:$E8
    lsr a
    ror z:$E8
    lsr a
    ror z:$E8
    lsr a
    ror z:$E8
    sta z:$E9
    rts

; @card 02:CC50 psg_write_channels_alt
; WHAT IT DOES: The second player's psg_write_channels (mode 0): Writes the six channels' state to the PSG, from channel 5 down to 0: a channel asked to stop is silenced and forgotten; a
; new one gets its waveform, noise or frequency, balance and volume.
; HOW: Each channel's state at $247C: below 1 skipped; 2 and up: key off, state cleared, and the channel's busy bit cleared in $2474
; unless the player's mode is 1 or the channel is not used; 1: a new note: the waveform ($2474 bit 7 not yet set) copied as 32 bytes
; from the built-in table at $D9A6 or the user's at $22D2, then noise (channels 4 and 5, if bit 7 of $2476 is clear) or frequency,
; the balance, and the volume less the fade ($2615), written with the key on, or the key off if none is left.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N from it; A and the other flags as the last channel left them.
; The PSG's registers at $0800 to $0807 and the arrays are by address; psg_wave_offset's frames are at $D68E and $D6A3.
psg_write_channels_alt:
    ldx #$05
psg_write_channels_alt_at0:
    lda a:$247C,x
    cmp #$01
    bpl psg_write_channels_alt_at1
    jmp psg_write_channels_alt_at18
psg_write_channels_alt_at1:
    stx a:$0800
    beq psg_write_channels_alt_at3
    stz a:$0804
    stz a:$247C,x
    jmp psg_write_channels_alt_at18
psg_write_channels_alt_at3:
    lda a:$22DF
    beq psg_write_channels_alt_at4
    lda a:$2607,x
    cmp #$01
    bne psg_write_channels_alt_at4
    jmp psg_write_channels_alt_at18
psg_write_channels_alt_at4:
    lda a:$2474,x
    bmi psg_write_channels_alt_at9
    ora #$80
    sta a:$2474,x
    and #$7F
    cmp #$2D
    bcs psg_write_channels_alt_at5
    jsr psg_wave_offset_alt
    lda #$A6
    clc
    adc z:$E8
    sta z:$E8
    lda #$D9
    adc z:$E9
    sta z:$E9
    bra psg_write_channels_alt_at6
psg_write_channels_alt_at5:
    sec
    sbc #$2D
    jsr psg_wave_offset_alt
    lda a:$22D2
    clc
    adc z:$E8
    sta z:$E8
    lda a:$22D3
    adc z:$E9
    sta z:$E9
psg_write_channels_alt_at6:
    lda #$40
    sta a:$0804
    stz a:$0804
    cly
psg_write_channels_alt_at7:
    lda (<$E8),y
    sta a:$0806
    iny
    cpy #$20
    bcc psg_write_channels_alt_at7
psg_write_channels_alt_at9:
    cpx #$04
    bcc psg_write_channels_alt_at10
    lda a:$2476,x
    bmi psg_write_channels_alt_at10
    ora #$80
    sta a:$0807
    bra psg_write_channels_alt_at11
psg_write_channels_alt_at10:
    stz a:$0807
    lda a:$245C,x
    sta a:$0802
    lda a:$2462,x
    sta a:$0803
psg_write_channels_alt_at11:
    lda a:$246E,x
    sta a:$0805
    lda a:$2468,x
    sec
    sbc a:$248A
    beq psg_write_channels_alt_at12
    bpl psg_write_channels_alt_at13
psg_write_channels_alt_at12:
    stz a:$0804
    bra psg_write_channels_alt_at18
psg_write_channels_alt_at13:
    ora #$80
    sta a:$0804
psg_write_channels_alt_at18:
    dex
    bmi psg_write_channels_alt_at19
    jmp psg_write_channels_alt_at0
psg_write_channels_alt_at19:
    rts

; @card 02:C85A psg_slide_channel_alt
; WHAT IT DOES: The second player's psg_slide_channel (mode 0): Moves the current channel's pitch, left volume and right volume one step towards where its slides are taking them.
; HOW: Each slide has a signed step ($255D, $2569, $2575 plus the channel) and a fraction ($2563, $256F, $257B): the step added to
; the fraction in seven bits, the carry out of bit 6 into bit 7 added to (or taken from) the whole part ($24AF, the nibbles of $25F9),
; clipped at its limit, where the slide ends by clearing its step.
; WAITS: no.
; Entry: CardPlayChannel ($22E0) the channel. Exit: A, X and the flags as the last step left them.
psg_slide_channel_alt:
    ldx a:$22E0
    lda a:$23C0,x
    beq psg_slide_channel_alt_at5
    bmi psg_slide_channel_alt_at2
    clc
    adc a:$23C6,x
    sta z:$EA
    rol a
    rol a
    and #$01
    clc
    adc a:$2312,x
    beq psg_slide_channel_alt_at4
    bmi psg_slide_channel_alt_at4
    cla
    stz a:$23C0,x
    bra psg_slide_channel_alt_at4
psg_slide_channel_alt_at2:
    eor #$FF
    inc a
    clc
    adc a:$23C6,x
    sta z:$EA
    rol a
    rol a
    and #$01
    sta z:$EB
    lda a:$2312,x
    sec
    sbc z:$EB
    beq psg_slide_channel_alt_at4
    cmp #$E1
    bcs psg_slide_channel_alt_at4
    lda #$E1
    stz a:$23C0,x
psg_slide_channel_alt_at4:
    sta a:$2312,x
    lda z:$EA
    and #$7F
    sta a:$23C6,x
psg_slide_channel_alt_at5:
    lda a:$23CC,x
    beq psg_slide_channel_alt_at12
    bmi psg_slide_channel_alt_at7
    clc
    adc a:$23D2,x
    sta z:$EA
    rol a
    rol a
    and #$01
    sta z:$EB
    lda a:$246E,x
    and #$0F
    clc
    adc z:$EB
    cmp #$10
    bcc psg_slide_channel_alt_at9
    lda #$0F
    stz a:$23CC,x
    bra psg_slide_channel_alt_at9
psg_slide_channel_alt_at7:
    eor #$FF
    inc a
    clc
    adc a:$23D2,x
    sta z:$EA
    rol a
    rol a
    and #$01
    sta z:$EB
    lda a:$246E,x
    and #$0F
    sec
    sbc z:$EB
    bne psg_slide_channel_alt_at9
    cla
    stz a:$23CC,x
psg_slide_channel_alt_at9:
    sta z:$EB
    lda a:$246E,x
    and #$F0
    ora z:$EB
    sta a:$246E,x
    lda z:$EA
    and #$7F
    sta a:$23D2,x
psg_slide_channel_alt_at12:
    lda a:$23D8,x
    beq psg_slide_channel_alt_at19
    bmi psg_slide_channel_alt_at14
    clc
    adc a:$23DE,x
    sta z:$EA
    and #$80
    lsr a
    lsr a
    lsr a
    sta z:$EB
    lda a:$246E,x
    and #$F0
    clc
    adc z:$EB
    bcc psg_slide_channel_alt_at17
    lda #$F0
    stz a:$23D8,x
    bra psg_slide_channel_alt_at17
psg_slide_channel_alt_at14:
    eor #$FF
    inc a
    clc
    adc a:$23DE,x
    sta z:$EA
    and #$80
    lsr a
    lsr a
    lsr a
    sta z:$EB
    lda a:$246E,x
    and #$F0
    sec
    sbc z:$EB
    bne psg_slide_channel_alt_at17
    cla
    stz a:$23D8,x
psg_slide_channel_alt_at17:
    sta z:$EB
    lda a:$246E,x
    and #$0F
    ora z:$EB
    sta a:$246E,x
    lda z:$EA
    and #$7F
    sta a:$23DE,x
psg_slide_channel_alt_at19:
    rts

; @card 02:C953 psg_read_notes_alt
; WHAT IT DOES: The second player's psg_read_notes (mode 0): Reads the current channel's note stream when the channel is in the state that wants it: a note's pitch, the sound's
; shape (its waveform pair), the volume or the channel flags, until a note or the stream's end.
; HOW: Only for a channel whose kind ($25E1, high nibble) is 1 and whose state ($260D) is not zero. Each byte of the stream at the
; pointer kept in $259F and $25A5: below $B0 a noise value (channels 4 and 5); $B0 to $BF the high pitch and the next byte the low, and
; the note is read; $C0 a shape number, built-in or the user's, $D0 the balance, $E0 the flags; anything else ends the stream. A
; shape, balance or flags byte goes on to the next byte.
; WAITS: no.
; Entry: CardPlayChannel ($22E0) the channel. Exit: A, X, Y and the flags as the last step left them.
psg_read_notes_alt:
    ldy a:$22E0
    lda a:$2456,y
    and #$F0
    cmp #$10
    beq psg_read_notes_alt_at0
psg_read_notes_alt_at22:
    rts
psg_read_notes_alt_at0:
    lda a:$2482,y
    beq psg_read_notes_alt_at22
    lda a:$2402,y
    sta z:$E8
    lda a:$2408,y
    sta z:$E9
psg_read_notes_alt_at1:
    lda (<$E8)
    inc z:$E8
    bne psg_read_notes_alt_at2
    inc z:$E9
psg_read_notes_alt_at2:
    tax
    cmp #$B0
    bcc psg_read_notes_alt_at7
    and #$F0
    cmp #$B0
    beq psg_read_notes_alt_at9
    cmp #$C0
    beq psg_read_notes_alt_at12
    cmp #$D0
    beq psg_read_notes_alt_at17
    cmp #$E0
    beq psg_read_notes_alt_at18
    jmp psg_read_notes_alt_at19
psg_read_notes_alt_at7:
    cpy #$04
    bcc psg_read_notes_alt_at11
    and #$1F
    sta a:$2476,y
    bra psg_read_notes_alt_at11
psg_read_notes_alt_at9:
    txa
    sec
    sbc #$B0
    sta a:$2462,y
    lda (<$E8)
    inc z:$E8
    bne psg_read_notes_alt_at10
    inc z:$E9
psg_read_notes_alt_at10:
    sta a:$245C,y
    cpy #$04
    bcc psg_read_notes_alt_at11
    lda #$80
    sta a:$2476,y
psg_read_notes_alt_at11:
    lda z:$E8
    sta a:$2402,y
    lda z:$E9
    sta a:$2408,y
    rts
psg_read_notes_alt_at12:
    lda (<$E8)
    cmp #$10
    bcs psg_read_notes_alt_at13
    asl a
    tax
    lda a:$D8A1,x
    sta a:$233C,y
    lda a:$D8A2,x
    sta a:$2342,y
    bra psg_read_notes_alt_at20
psg_read_notes_alt_at13:
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
    sta a:$233C,y
    inc z:$EA
    bne psg_read_notes_alt_at14
    inc z:$EB
psg_read_notes_alt_at14:
    lda (<$EA)
    sta a:$2342,y
    bra psg_read_notes_alt_at20
psg_read_notes_alt_at17:
    lda (<$E8)
    sta a:$246E,y
    bra psg_read_notes_alt_at20
psg_read_notes_alt_at18:
    lda (<$E8)
    sta a:$2474,y
    bra psg_read_notes_alt_at20
psg_read_notes_alt_at19:
    cla
    sta a:$2482,y
    lda z:$E8
    sec
    sbc #$01
    sta a:$2402,y
    lda z:$E9
    sbc #$00
    sta a:$2408,y
    rts
psg_read_notes_alt_at20:
    inc z:$E8
    bne psg_read_notes_alt_at21
    inc z:$E9
psg_read_notes_alt_at21:
    jmp psg_read_notes_alt_at1

; @card 02:CB13 psg_pitch_effects_alt
; WHAT IT DOES: The second player's psg_pitch_effects (mode 0): Works the current channel's pitch effects for the frame: the base pitch put back, then its vibrato, its detune, its pitch
; slide through a list and its portamento through another.
; HOW: Only for a channel of kind 0 ($25E1, high nibble). The frequency set from its base ($249D, $24A3), then: vibrato ($2581 step, $2593 on):
; counted down in $2599, then the accumulated amount ($2587, $258D) added, else the step scaled, accumulated and added; detune ($2557):
; scaled and added; slide ($254B on): each time $2551 reaches 0 the next byte of the list at ($2539, $253F) from $2545, $80 ends it
; (stepping back one byte); portamento ($2527 on): the same with the list at ($2515, $251B) from $2521, $80 sending it back to its start.
; WAITS: no.
; Entry: CardPlayChannel ($22E0) the channel. Exit: A, Y and the flags as the last step left them; X the channel.
psg_pitch_effects_alt:
    ldx a:$22E0
    lda a:$2456,x
    and #$F0
    beq psg_pitch_effects_alt_at0
    rts
psg_pitch_effects_alt_at0:
    lda a:$2300,x
    sta a:$245C,x
    lda a:$2306,x
    sta a:$2462,x
    lda a:$23E4,x
    tay
    beq psg_pitch_effects_alt_at4
    lda a:$23F6,x
    beq psg_pitch_effects_alt_at2
    dec a:$23FC,x
    bne psg_pitch_effects_alt_at2
    inc a:$23FC,x
    lda a:$23EA,x
    sta z:$EA
    lda a:$23F0,x
    sta z:$EB
    jsr psg_add_pitch_alt
    bra psg_pitch_effects_alt_at4
psg_pitch_effects_alt_at2:
    sty z:$EA
    jsr psg_scale_amount_alt
    lda a:$23EA,x
    clc
    adc z:$EA
    sta a:$23EA,x
    sta z:$EA
    lda a:$23F0,x
    adc z:$EB
    sta a:$23F0,x
    sta z:$EB
    jsr psg_add_pitch_alt
psg_pitch_effects_alt_at4:
    lda a:$23BA,x
    beq psg_pitch_effects_alt_at5
    sta z:$EA
    jsr psg_scale_amount_alt
    jsr psg_add_pitch_alt
psg_pitch_effects_alt_at5:
    lda a:$23AE,x
    beq psg_pitch_effects_alt_at11
    dec a:$23B4,x
    bne psg_pitch_effects_alt_at11
    inc a:$23B4,x
    lda a:$239C,x
    sta z:$E8
    lda a:$23A2,x
    sta z:$E9
    ldy a:$23A8,x
    lda (<$E8),y
    cmp #$80
    bne psg_pitch_effects_alt_at7
    cpy #$00
    beq psg_pitch_effects_alt_at11
    dey
    lda (<$E8),y
psg_pitch_effects_alt_at7:
    sta z:$EA
    phy
    jsr psg_scale_amount_alt
    jsr psg_add_pitch_alt
    ply
    iny
    bne psg_pitch_effects_alt_at8
    dey
psg_pitch_effects_alt_at8:
    tya
    sta a:$23A8,x
psg_pitch_effects_alt_at11:
    lda a:$238A,x
    beq psg_pitch_effects_alt_at15
    dec a:$2390,x
    bne psg_pitch_effects_alt_at15
    inc a:$2390,x
    lda a:$2378,x
    sta z:$E8
    lda a:$237E,x
    sta z:$E9
    ldy a:$2384,x
psg_pitch_effects_alt_at12:
    lda (<$E8),y
    cmp #$80
    bne psg_pitch_effects_alt_at14
    cpy #$00
    beq psg_pitch_effects_alt_at15
    stz a:$2384,x
    cly
    bra psg_pitch_effects_alt_at12
psg_pitch_effects_alt_at14:
    sta z:$EA
    jsr psg_scale_amount_alt
    jsr psg_add_pitch_alt
    inc a:$2384,x
psg_pitch_effects_alt_at15:
    rts

; @card 02:CBE5 psg_scale_amount_alt
; WHAT IT DOES: The second player's psg_scale_amount (mode 0): Makes the byte in $EA a signed 16-bit amount in $EA and $EB, scaled by the channel's scale less its base.
; HOW: $EB the sign of $EA; the channel's scale ($2533) less its base ($2497) as a shift count: left for plus, right for minus; a
; right shift that leaves something in the high byte makes it $FF.
; WAITS: no.
; Entry: X the channel, $EA the byte. Exit: $EA and $EB the amount; A, Y and the flags as the last step left them. X kept.
psg_scale_amount_alt:
    stz z:$EB
    lda z:$EA
    bpl psg_scale_amount_alt_at0
    dec z:$EB
psg_scale_amount_alt_at0:
    lda a:$2396,x
    beq psg_scale_amount_alt_at6
    sec
    sbc a:$22FA,x
    tay
    bmi psg_scale_amount_alt_at3
psg_scale_amount_alt_at1:
    beq psg_scale_amount_alt_at6
    asl z:$EA
    rol z:$EB
    dey
    bra psg_scale_amount_alt_at1
psg_scale_amount_alt_at3:
    beq psg_scale_amount_alt_at4
    lsr z:$EB
    ror z:$EA
    iny
    bra psg_scale_amount_alt_at3
psg_scale_amount_alt_at4:
    lda z:$EB
    beq psg_scale_amount_alt_at6
    lda #$FF
    sta z:$EB
psg_scale_amount_alt_at6:
    rts

; @card 02:CC14 psg_add_pitch_alt
; WHAT IT DOES: The second player's psg_add_pitch (mode 0): Adds the amount in $EA and $EB to the current channel's frequency.
; HOW: A 16-bit add to $25E7 and $25ED, plus the channel.
; WAITS: no.
; Entry: X the channel, $EA and $EB the amount. Exit: A the high byte, C and V from its add, N and Z from it. X, Y kept.
psg_add_pitch_alt:
    lda a:$245C,x
    clc
    adc z:$EA
    sta a:$245C,x
    lda a:$2462,x
    adc z:$EB
    sta a:$2462,x
    rts

; @card 02:CA25 psg_volume_envelope_alt
; WHAT IT DOES: The second player's psg_volume_envelope (mode 0): Works the current channel's volume envelope for the frame: the held part moves the level by its rates until its length is
; used up, the release moves it down by the release rates, and the next segment is read from the shape's list when the held part ends.
; HOW: State $260D: 0 off (volume 0); bit 7 the release; 2 ending (state counted down, position cleared, next segment); else the held part:
; $24BB counted down and the release bit set at 0, $250F the length left (0 reads the next segment), the level ($24EB, $24F1) moved by
; the rates ($24F7, $24FD). The list at ($24D9, $24DF) from $24E5: $FB skips three bytes, $FC sets the level then reads a segment (length, two
; rates), a length below $FB is a segment ($FF if 0), anything else holds. The volume is the level over 4 plus the channel's base ($24AF).
; WAITS: no.
; Entry: CardPlayChannel ($22E0) the channel. Exit: A, Y and the flags as the last step left them; X the channel.
psg_volume_envelope_alt:
    ldx a:$22E0
    lda a:$2482,x
    bne psg_volume_envelope_alt_at0
    jmp psg_volume_envelope_alt_at13
psg_volume_envelope_alt_at0:
    bmi psg_volume_envelope_alt_at8
    cmp #$02
    beq psg_volume_envelope_alt_at12
    dec a:$231E,x
    bne psg_volume_envelope_alt_at1
    ora #$80
    sta a:$2482,x
psg_volume_envelope_alt_at1:
    lda a:$2372,x
    bne psg_volume_envelope_alt_at2
    bra psg_volume_envelope_alt_at18
psg_volume_envelope_alt_at2:
    lda a:$234E,x
    clc
    adc a:$235A,x
    sta a:$234E,x
    lda a:$2354,x
    adc a:$2360,x
    bpl psg_volume_envelope_alt_at4
    cla
    sta a:$234E,x
    ldy a:$2360,x
    bmi psg_volume_envelope_alt_at4
    lda #$7C
psg_volume_envelope_alt_at4:
    sta a:$2354,x
    lda a:$2372,x
    cmp #$FF
    beq psg_volume_envelope_alt_at16
    dec a:$2372,x
    bra psg_volume_envelope_alt_at16
psg_volume_envelope_alt_at8:
    lda a:$234E,x
    clc
    adc a:$2366,x
    sta a:$234E,x
    lda a:$2354,x
    adc a:$236C,x
    bpl psg_volume_envelope_alt_at10
    cla
    sta a:$234E,x
    ldy a:$236C,x
    bmi psg_volume_envelope_alt_at10
    lda #$7C
psg_volume_envelope_alt_at10:
    sta a:$2354,x
    bra psg_volume_envelope_alt_at16
psg_volume_envelope_alt_at12:
    dec a:$2482,x
    stz a:$2348,x
    bra psg_volume_envelope_alt_at18
psg_volume_envelope_alt_at13:
    stz a:$2468,x
    bra psg_volume_envelope_alt_at17
psg_volume_envelope_alt_at15:
    tya
    sta a:$2348,x
psg_volume_envelope_alt_at16:
    lda a:$2354,x
    lsr a
    lsr a
    clc
    adc a:$2312,x
    bpl psg_volume_envelope_alt_at17a
    cla
psg_volume_envelope_alt_at17a:
    sta a:$2468,x
psg_volume_envelope_alt_at17:
    rts
psg_volume_envelope_alt_at18:
    ldy a:$2348,x
    lda a:$233C,x
    sta z:$E8
    lda a:$2342,x
    sta z:$E9
psg_volume_envelope_alt_at19:
    lda (<$E8),y
    cmp #$FB
    bcc psg_volume_envelope_alt_at21
    beq psg_volume_envelope_alt_at24
    cmp #$FC
    beq psg_volume_envelope_alt_at25
    bra psg_volume_envelope_alt_at28
psg_volume_envelope_alt_at21:
    cmp #$00
    bne psg_volume_envelope_alt_at22
    dec a
psg_volume_envelope_alt_at22:
    sta a:$2372,x
    iny
    lda (<$E8),y
    sta a:$235A,x
    iny
    lda (<$E8),y
    sta a:$2360,x
    iny
    bra psg_volume_envelope_alt_at15
psg_volume_envelope_alt_at24:
    iny
    iny
    iny
    bra psg_volume_envelope_alt_at19
psg_volume_envelope_alt_at25:
    iny
    lda (<$E8),y
    sta a:$234E,x
    iny
    lda (<$E8),y
    sta a:$2354,x
    iny
    lda (<$E8),y
    cmp #$FB
    bcc psg_volume_envelope_alt_at21
    stz a:$2372,x
    bra psg_volume_envelope_alt_at15
psg_volume_envelope_alt_at28:
    lda #$FF
    sta a:$2372,x
    stz a:$235A,x
    stz a:$2360,x
    jmp psg_volume_envelope_alt_at16

; @card 02:C330 psg_start_song_alt
; WHAT IT DOES: The second player's psg_start_song (mode 0): Starts the song that was asked for, if one was: sets up each channel the song's header names.
; HOW: $22E1 the song asked for (bit 7 set: none, or already started); the song's header found through the pointer at $22D0 and the list
; of headers, its first byte a mask of channels (bit 7 set: it is a song); for each channel in the mask the state, counts and effects
; arrays cleared, the default shape, balance and scale set, the channel's start pointer taken from the header, the flags set from
; $22E2 if the header's bit 6 says so; then the fade cleared, the song marked started and the player's done flag cleared.
; WAITS: no.
; Entry: nothing. Exit: A $40, X 6, Y the bytes of the header read, C 1, N and V from the done flags' old byte, Z from A and it.
psg_start_song_alt:
    lda a:$22E1
    bpl psg_start_song_alt_at0
psg_start_song_alt_at12:
    rts
psg_start_song_alt_at0:
    asl a
    tay
    lda a:$22D0
    sta z:$EA
    lda a:$22D1
    sta z:$EB
    lda (<$EA),y
    sta z:$E8
    iny
    lda (<$EA),y
    sta z:$E9
    lda (<$E8)
    bmi psg_start_song_alt_at12
    sta z:$EA
    sta z:$EB
    cly
    clx
psg_start_song_alt_at3:
    lsr z:$EA
    bcc psg_start_song_alt_at10
    lda #$01
    sta a:$247C,x
    sta a:$230C,x
    stz a:$2482,x
    stz a:$2318,x
    stz a:$2312,x
    stz a:$238A,x
    stz a:$23AE,x
    stz a:$2396,x
    stz a:$23BA,x
    stz a:$23E4,x
    stz a:$23F6,x
    stz a:$2456,x
    stz a:$2330,x
    stz a:$23C0,x
    stz a:$23CC,x
    stz a:$23D8,x
    stz a:$2336,x
    lda a:$D8A1
    sta a:$233C,x
    lda a:$D8A2
    sta a:$2342,x
    lda #$84
    sta a:$236C,x
    lda #$04
    sta a:$22FA,x
    iny
    lda (<$E8),y
    sta a:$22EE,x
    sta a:$2324,x
    iny
    lda (<$E8),y
    sta a:$22F4,x
    sta a:$232A,x
    lda #$80
    cpx #$04
    bcc psg_start_song_alt_at8
    sta a:$2476,x
psg_start_song_alt_at8:
    sta a:$2474,x
    bbr6 $EB,psg_start_song_alt_at10
    lda a:$22E2
    sta a:$2474,x
psg_start_song_alt_at10:
    inx
    cpx #$06
    bcc psg_start_song_alt_at3
    stz a:$2488
    stz a:$248A
    lda #$80
    tsb a:$22E1
    lda a:$22DE
    and #$70
    ora a:$22DD
    sta a:$22DE
    rts

; @card 02:CDF6 psg_sequencer
; WHAT IT DOES: Steps one channel's sequence when its note has run out: reads the stream's commands, each of which changes the
; channel's state, until a note or the end of the song's stream.
; HOW: The channel's note count ($24A9 plus the channel) counted down; while it is not zero nothing else. At zero the stream's pointer
; ($248B, $2491) is taken and each byte read: $D0 and above is a command through the jump table at $CE1D (duration scale, scale base
; and its step, rest, volume slides, shape, balance, flags, call and return, loops, tempo, the fade), anything below a note, which sets
; the count, the rest, the pitch (from the song's stream, the octave table or the noise value) and the channel's state, and keeps the
; pointer.
; WAITS: no.
; Entry: X the channel. Exit: counting: N and Z from the count. A note: A the pointer's high byte, N and Z from it, X, Y the channel.
; The stop command: A 2, N 0, Z 0, X, Y the channel. C, V as the last command's arithmetic left them.
psg_sequencer:
    dec a:$24A9,x
    beq psg_sequencer_at0
    rts
psg_sequencer_at0:
    sxy
    lda a:$248B,y
    sta z:$E8
    lda a:$2491,y
    sta z:$E9
psg_sequencer_at1:
    lda (<$E8)
    inc z:$E8
    bne psg_sequencer_at2
    inc z:$E9
psg_sequencer_at2:
    pha
    cmp #$D0
    bcc psg_sequencer_at3
    sec
    sbc #$D0
    asl a
    tax
    pla
    jmp (psg_sequencer_table,x)
psg_sequencer_table:
    .addr psg_sequencer_at19
    .addr psg_sequencer_at20
    .addr psg_sequencer_at20
    .addr psg_sequencer_at20
    .addr psg_sequencer_at20
    .addr psg_sequencer_at20
    .addr psg_sequencer_at20
    .addr psg_sequencer_at20
    .addr psg_sequencer_at21
    .addr psg_sequencer_at22
    .addr psg_sequencer_at23
    .addr psg_sequencer_at1
    .addr psg_sequencer_at24
    .addr psg_sequencer_at25
    .addr psg_sequencer_at26
    .addr psg_sequencer_at27
    .addr psg_sequencer_at1
    .addr psg_sequencer_at30
    .addr psg_sequencer_at31
    .addr psg_sequencer_at32
    .addr psg_sequencer_at35
    .addr psg_sequencer_at37
    .addr psg_sequencer_at38
    .addr psg_sequencer_at43
    .addr psg_sequencer_at44
    .addr psg_sequencer_at45
    .addr psg_sequencer_at46
    .addr psg_sequencer_at47
    .addr psg_sequencer_at48
    .addr psg_sequencer_at49
    .addr psg_sequencer_at50
    .addr psg_sequencer_at51
    .addr psg_sequencer_at53
    .addr psg_sequencer_at55
    .addr psg_sequencer_at57
    .addr psg_sequencer_at58
    .addr psg_sequencer_at59
    .addr psg_sequencer_at62
    .addr psg_sequencer_at63
    .addr psg_sequencer_at64
    .addr psg_sequencer_at65
    .addr psg_sequencer_at1
    .addr psg_sequencer_at1
    .addr psg_sequencer_at1
    .addr psg_sequencer_at1
    .addr psg_sequencer_at1
    .addr psg_sequencer_at66
    .addr psg_sequencer_at68
psg_sequencer_at3:
    ldx a:$24D3,y
    beq psg_sequencer_at6
    and #$0F
    inc a
    cmp a:$24D3,y
    bcs psg_sequencer_at4
    sax
psg_sequencer_at4:
    sta z:$EA
    clc
psg_sequencer_at5:
    dex
    beq psg_sequencer_at7
    adc z:$EA
    bra psg_sequencer_at5
psg_sequencer_at6:
    lda (<$E8)
    inc z:$E8
    bne psg_sequencer_at7
    inc z:$E9
psg_sequencer_at7:
    sta a:$24A9,y
    tax
    lda (<$E8)
    sax
    cpx #$DA
    beq psg_sequencer_at10
    lsr a
    lsr a
    lsr a
    sta z:$EA
    sec
    lda a:$24A9,y
    ldx a:$24B5,y
psg_sequencer_at8:
    beq psg_sequencer_at10
psg_sequencer_at9:
    sbc z:$EA
    dex
    bra psg_sequencer_at8
psg_sequencer_at10:
    sta a:$24BB,y
    pla
    and #$F0
    bne psg_sequencer_at11
    cla
    jmp psg_sequencer_at18
psg_sequencer_at11:
    sec
    sbc #$10
    lsr a
    lsr a
    lsr a
    tax
    lda a:$25E1,y
    and #$F0
    sax
    beq psg_sequencer_at14
    cpx #$10
    bne psg_sequencer_at13
    clc
    adc a:$22DB
    sta z:$EA
    cla
    adc a:$22DC
    sta z:$EB
    lda (<$EA)
    sta a:$259F,y
    inc z:$EA
    bne psg_sequencer_at12
    inc z:$EB
psg_sequencer_at12:
    lda (<$EA)
    sta a:$25A5,y
    bra psg_sequencer_at15
psg_sequencer_at13:
    cpy #$04
    bcc psg_sequencer_at14
    clc
    adc a:$24CD,y
    lsr a
    sta a:$2601,y
    bra psg_sequencer_at15
    brk
    clc
    bmi psg_sequencer_at17
    rts
    sei
    bcc psg_sequencer_at9
psg_sequencer_at14:
    ldx a:$2497,y
    clc
    adc a:$CF05,x
    adc a:$24CD,y
    tax
    lda a:$D71A,x
    sta a:$249D,y
    lda a:$D71B,x
    sta a:$24A3,y
    cpy #$04
    bcc psg_sequencer_at15
    lda #$80
    sta a:$2601,y
psg_sequencer_at15:
    lda a:$260D,y
    cmp #$03
    bne psg_sequencer_at16
    lda #$01
    bra psg_sequencer_at18
psg_sequencer_at16:
    cla
    sta a:$2521,y
    sta a:$2545,y
    sta a:$2587,y
    sta a:$258D,y
    lda a:$2527,y
    sta a:$252D,y
    lda a:$254B,y
    sta a:$2551,y
psg_sequencer_at17:
    lda a:$2593,y
    sta a:$2599,y
    lda #$02
psg_sequencer_at18:
    sta a:$260D,y
    lda z:$E8
    sta a:$248B,y
    lda z:$E9
    sta a:$2491,y
    rts
psg_sequencer_at19:
    lda (<$E8)
    and #$0F
    sta a:$24D3,y
    jmp psg_sequencer_at69
psg_sequencer_at20:
    and #$07
    sta a:$2497,y
    jmp psg_sequencer_at1
psg_sequencer_at21:
    ldx a:$22E0
    inc a:$2497,x
    jmp psg_sequencer_at1
psg_sequencer_at22:
    ldx a:$22E0
    dec a:$2497,x
    jmp psg_sequencer_at1
psg_sequencer_at23:
    lda #$03
    sta a:$260D,y
    jmp psg_sequencer_at1
psg_sequencer_at24:
    lda (<$E8)
    and #$1F
    sec
    sbc #$1F
    sta a:$24AF,y
    cla
    sta a:$255D,y
    jmp psg_sequencer_at69
psg_sequencer_at25:
    lda (<$E8)
    sta a:$25F9,y
    cla
    sta a:$2569,y
    sta a:$2575,y
    jmp psg_sequencer_at69
psg_sequencer_at26:
    lda (<$E8)
    dec a
    and #$07
    sec
    sbc #$08
    eor #$FF
    sta a:$24B5,y
    jmp psg_sequencer_at69
psg_sequencer_at27:
    lda a:$24AF,y
    clc
    adc (<$E8)
    bmi psg_sequencer_at28
    cla
    bra psg_sequencer_at29
psg_sequencer_at28:
    cmp #$E1
    bcs psg_sequencer_at29
    lda #$E1
psg_sequencer_at29:
    sta a:$24AF,y
    jmp psg_sequencer_at69
psg_sequencer_at30:
    lda a:$24C1,y
    sta z:$E8
    lda a:$24C7,y
    sta z:$E9
    jmp psg_sequencer_at1
psg_sequencer_at31:
    lda z:$E8
    sta a:$24C1,y
    lda z:$E9
    sta a:$24C7,y
    jmp psg_sequencer_at1
psg_sequencer_at32:
    sxy
    lda (<$E8)
    bne psg_sequencer_at33
    lda #$02
psg_sequencer_at33:
    pha
    inc z:$E8
    bne psg_sequencer_at34
    inc z:$E9
psg_sequencer_at34:
    jsr psg_call_stack
    lda z:$E8
    sta (<$EA),y
    iny
    lda z:$E9
    sta (<$EA),y
    iny
    pla
    sta (<$EA),y
    iny
    sty z:$EA
    lda a:$25E1,x
    and #$F0
    ora z:$EA
    sta a:$25E1,x
    sxy
    jmp psg_sequencer_at1
psg_sequencer_at35:
    sxy
    jsr psg_call_stack
    dey
    lda (<$EA),y
    dec a
    beq psg_sequencer_at36
    sta (<$EA),y
    dey
    lda (<$EA),y
    sta z:$E9
    dey
    lda (<$EA),y
    sta z:$E8
    sxy
    jmp psg_sequencer_at1
psg_sequencer_at36:
    dey
    dey
    sty z:$EA
    lda a:$25E1,x
    and #$F0
    ora z:$EA
    sta a:$25E1,x
    sxy
    jmp psg_sequencer_at1
psg_sequencer_at37:
    lda (<$E8)
    sta a:$25FF,y
    jmp psg_sequencer_at69
psg_sequencer_at38:
    sxy
    lda (<$E8)
    cmp #$10
    bcs psg_sequencer_at39
    asl a
    tay
    lda a:$D8A1,y
    sta z:$EA
    lda a:$D8A2,y
    sta z:$EB
    bra psg_sequencer_at41
psg_sequencer_at39:
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
    tay
    inc z:$EA
    bne psg_sequencer_at40
    inc z:$EB
psg_sequencer_at40:
    lda (<$EA)
    sta z:$EB
    sty z:$EA
psg_sequencer_at41:
    stz a:$2503,x
    stz a:$2509,x
    lda (<$EA)
    cmp #$FB
    bne psg_sequencer_at42
    ldy #$01
    lda (<$EA),y
    sta a:$2503,x
    iny
    lda (<$EA),y
    sta a:$2509,x
    lda #$03
    clc
    adc z:$EA
    sta z:$EA
    cla
    adc z:$EB
    sta z:$EB
psg_sequencer_at42:
    lda z:$EA
    sta a:$24D9,x
    lda z:$EB
    sta a:$24DF,x
    stz a:$250F,x
    stz a:$24F1,x
    stz a:$24F7,x
    stz a:$24FD,x
    sxy
    jmp psg_sequencer_at69
psg_sequencer_at43:
    sxy
    lda (<$E8)
    asl a
    tay
    lda a:$22D7
    sta z:$EA
    lda a:$22D8
    sta z:$EB
    lda (<$EA),y
    sta a:$2515,x
    iny
    lda (<$EA),y
    sta a:$251B,x
    stz a:$2521,x
    sxy
    jmp psg_sequencer_at69
psg_sequencer_at44:
    lda (<$E8)
    sta a:$2527,y
    jmp psg_sequencer_at69
psg_sequencer_at45:
    lda (<$E8)
    and #$07
    sta a:$2533,y
    jmp psg_sequencer_at69
psg_sequencer_at46:
    sxy
    lda (<$E8)
    asl a
    tay
    lda a:$22D9
    sta z:$EA
    lda a:$22DA
    sta z:$EB
    lda (<$EA),y
    sta a:$2539,x
    iny
    lda (<$EA),y
    sta a:$253F,x
    stz a:$2545,x
    sxy
    jmp psg_sequencer_at69
psg_sequencer_at47:
    lda (<$E8)
    sta a:$254B,y
    jmp psg_sequencer_at69
psg_sequencer_at48:
    lda (<$E8)
    sta a:$2557,y
    jmp psg_sequencer_at69
psg_sequencer_at49:
    lda (<$E8)
    sta a:$2581,y
    jmp psg_sequencer_at69
psg_sequencer_at50:
    lda (<$E8)
    sta a:$2593,y
    jmp psg_sequencer_at69
psg_sequencer_at51:
    lda (<$E8)
    tax
    inc z:$E8
    bne psg_sequencer_at52
    inc z:$E9
psg_sequencer_at52:
    lda (<$E8)
    sta z:$E9
    stx z:$E8
    jmp psg_sequencer_at1
psg_sequencer_at53:
    sxy
    jsr psg_call_stack
    lda z:$E8
    sta (<$EA),y
    iny
    lda z:$E9
    sta (<$EA),y
    iny
    sty z:$EA
    lda a:$25E1,x
    and #$F0
    ora z:$EA
    sta a:$25E1,x
    lda (<$E8)
    tay
    inc z:$E8
    bne psg_sequencer_at54
    inc z:$E9
psg_sequencer_at54:
    lda (<$E8)
    sta z:$E9
    sty z:$E8
    sxy
    jmp psg_sequencer_at1
psg_sequencer_at55:
    sxy
    jsr psg_call_stack
    dey
    lda (<$EA),y
    sta z:$E9
    dey
    lda (<$EA),y
    sta z:$E8
    sty z:$EA
    lda a:$25E1,x
    and #$F0
    ora z:$EA
    sta a:$25E1,x
    inc z:$E8
    bne psg_sequencer_at56
    inc z:$E9
psg_sequencer_at56:
    sxy
    jmp psg_sequencer_at69
psg_sequencer_at57:
    lda (<$E8)
    asl a
    sta a:$24CD,y
    jmp psg_sequencer_at69
psg_sequencer_at58:
    lda (<$E8)
    asl a
    sta z:$EA
    lda a:$24CD,y
    clc
    adc z:$EA
    sta a:$24CD,y
    bra psg_sequencer_at69
psg_sequencer_at59:
    phy
    lda (<$E8)
    asl a
    tay
    ldx #$05
psg_sequencer_at60:
    lda a:$2607,x
    beq psg_sequencer_at61
    tya
    sta a:$24CD,x
psg_sequencer_at61:
    dex
    bpl psg_sequencer_at60
    ply
    bra psg_sequencer_at69
psg_sequencer_at62:
    lda (<$E8)
    sta a:$255D,y
    cla
    sta a:$2563,y
    bra psg_sequencer_at69
psg_sequencer_at63:
    lda (<$E8)
    sta a:$2569,y
    cla
    sta a:$256F,y
    bra psg_sequencer_at69
psg_sequencer_at64:
    lda (<$E8)
    sta a:$2575,y
    cla
    sta a:$257B,y
    bra psg_sequencer_at69
psg_sequencer_at65:
    lda (<$E8)
    asl a
    asl a
    asl a
    asl a
    sta z:$EA
    lda a:$25E1,y
    and #$0F
    ora z:$EA
    sta a:$25E1,y
    lda (<$E8)
    cmp #$01
    bne psg_sequencer_at69
    cla
    sta a:$2503,y
    sta a:$2509,y
    bra psg_sequencer_at69
psg_sequencer_at66:
    lda (<$E8)
    bit #$7F
    beq psg_sequencer_at69
    cmp #$00
    bpl psg_sequencer_at67
    eor #$FF
    inc a
psg_sequencer_at67:
    sta a:$2613
    stz a:$2614
    stz a:$2615
    bra psg_sequencer_at69
psg_sequencer_at68:
    lda #$02
    sta a:$2607,y
    rts
psg_sequencer_at69:
    inc z:$E8
    bne psg_sequencer_at70
    inc z:$E9
psg_sequencer_at70:
    jmp psg_sequencer_at1

; @card 02:D23A psg_call_stack
; WHAT IT DOES: Finds the stack of calls the channel keeps for its sequence: a pointer to it in $EA and $EB, and how deep it is in Y.
; HOW: The channel's stack lies at the address the two tables at $D24B and $D251 give ($25AB, $25B4, $25BD, $25C6, $25CF, $25D8: nine bytes
; each); its depth is the low nibble of the channel's kind byte ($25E1 plus the channel).
; WAITS: no.
; Entry: X the channel. Exit: $EA and $EB the stack's address; A and Y the depth, N and Z from it. X, C and V kept.
psg_call_stack:
    lda a:$D24B,x
    sta z:$EA
    lda a:$D251,x
    sta z:$EB
    lda a:$25E1,x
    and #$0F
    tay
    rts
    .byte $AB,$B4,$BD,$C6,$CF,$D8
    .byte $25,$25,$25,$25,$25,$25
