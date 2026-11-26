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
