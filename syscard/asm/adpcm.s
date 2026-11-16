; @card 00:F6C1 ad_stop
; WHAT IT DOES: Stops ADPCM playback and forgets what was playing.
; HOW: Clears its interrupt enables and its control bits, then the card's play state.
; Entry: nothing. Exit: A $60; N and V bits 7 and 6 of the control port as it read, Z if
; nothing else was set in it. X, Y, C kept.
ad_stop:
    lda #$0C
    trb a:CdIrqControl
    lda #$60
    trb a:AdpcmControl
    stz a:CardAdpcmActive
    stz a:CardAdpcmPlayLo
    stz a:CardAdpcmPlayMid
    stz a:CardAdpcmPlayHi
    stz a:CardAdpcmPlayEnd
    rts

; @card 00:F6DB ad_stat
; WHAT IT DOES: Says whether ADPCM is busy and whether it is playing.
; HOW: Each answer is one port's bit, or if clear another's.
; Entry: nothing. Exit: X busy, A playing, N and Z from A. Y, C, V kept.
ad_stat:
    lda a:AdpcmStatus
    and #$01
    bne ad_stat_at0
    lda a:CdIrqStatus
    and #$04
ad_stat_at0:
    tax
    lda a:AdpcmControl
    and #$20
    bne ad_stat_at1
    lda a:AdpcmStatus
    and #$08
ad_stat_at1:
    rts

; @card 00:F37F ad_reset
; WHAT IT DOES: Resets the ADPCM unit.
; HOW: Pulses its reset bit, stops any transfer from the CD, turns off the CD unit's interrupt
; enables but the bus acknowledge, and sets the playback rate to zero.
; Entry: nothing. Exit: A $6F, N and V bits 7 and 6 of the interrupt enables as they were, Z if
; nothing outside $6F was set in them. X, Y, C kept.
ad_reset:
    lda #$80
    sta a:AdpcmControl
    stz a:AdpcmControl
    stz a:AdpcmDma
    lda #$6F
    trb a:CdIrqControl
    stz a:AdpcmRate
    rts

; @card 00:F5C6 ad_play
; WHAT IT DOES: Plays ADPCM: _AX bytes from address _BX at rate _DH; bit 7 of _DL repeats it,
; and any other bit of _DL plays what is already latched.
; HOW: Does nothing while ADPCM plays. Keeps _BX, _AX and the rate, latches the address and the
; length, sets the rate, then turns on the half-way interrupt and starts the unit.
; Entry: _AX, _BX, _DL, _DH. Exit, busy: as ad_stat. Bad rate: A _DH, C set, N and Z from
; comparing it with $10, X as ad_stat. Played: A 0, N 0, Z 0, V bit 6 of the interrupt enables
; as they were; latched: X and Y _AX, C clear; played again: X as ad_stat. Y, C, V kept
; otherwise.
ad_play:
    jsr ad_stat
    bne ad_play_at0
    lda z:CardDl
    and #$80
    sta a:CardAdpcmActive
    lda z:CardDl
    and #$7F
    bne ad_play_at1
    lda z:CardBl
    sta a:CardAdpcmAddress
    lda z:CardBh
    sta a:CardAdpcmAddress+1
    lda z:CardAl
    sta a:CardAdpcmLength
    lda z:CardAh
    sta a:CardAdpcmLength+1
    lda z:CardDh
    cmp #$10
    bcs ad_play_at0
    sta a:CardAdpcmRate
    ldx a:CardAdpcmAddress
    ldy a:CardAdpcmAddress+1
    jsr ad_set_address
    jsr ad_latch_read_address
    ldx a:CardAdpcmLength
    ldy a:CardAdpcmLength+1
    jsr ad_set_address
    jsr ad_latch_length
    lda a:CardAdpcmRate
    sta a:AdpcmRate
ad_play_at1:
    lda #$08
    tsb a:CdIrqControl
    lda #$60
    sta a:AdpcmControl
    cla
ad_play_at0:
    rts

