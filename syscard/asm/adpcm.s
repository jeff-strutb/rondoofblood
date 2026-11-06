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
