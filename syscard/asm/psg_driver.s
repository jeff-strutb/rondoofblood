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
