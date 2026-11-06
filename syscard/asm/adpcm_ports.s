; @card 00:F3FE ad_read_byte
; WHAT IT DOES: Reads the next byte of ADPCM memory.
; HOW: Waits for the unit to finish fetching, then its data port.
; Entry: nothing. Exit: A the byte, N and Z from it. X, Y, C, V kept.
ad_read_byte:
    lda a:AdpcmStatus
    bmi ad_read_byte
    lda a:AdpcmData
    rts

; @card 00:F6FF ad_latch_read_address
; WHAT IT DOES: Makes the address just given the one the ADPCM unit reads from.
; HOW: A strobe around a dummy read of the data port, with a pause the C does not state.
; Entry: nothing. Exit: A $08, Z if nothing else was set in the port, N and V bits 7 and 6 of
; what it held. X, Y, C kept.
ad_latch_read_address:
    lda #$08
    tsb a:AdpcmControl
    lda a:AdpcmData
    lda #$05
ad_latch_read_address_at0:
    dec a
    bne ad_latch_read_address_at0
    lda #$08
    bra ad_latch_length_at0

; @card 00:F710 ad_latch_write_address
; WHAT IT DOES: Makes the address just given the one the ADPCM unit writes to.
; HOW: Sets two strobes and clears them in turn.
; Entry: nothing. Exit: A $02, Z if nothing else was set in the port, N and V bits 7 and 6 of
; what it held. X, Y, C kept.
ad_latch_write_address:
    lda #$03
    tsb a:AdpcmControl
    lda #$01
    trb a:AdpcmControl
    lda #$02
    bra ad_latch_length_at0

; @card 00:F71E ad_latch_length
; WHAT IT DOES: Makes the address just given the length the ADPCM unit plays.
; HOW: A strobe.
; Entry: nothing. Exit: A $10, Z if nothing else was set in the port, N and V bits 7 and 6 of
; what it held. X, Y, C kept.
ad_latch_length:
    lda #$10
    tsb a:AdpcmControl
    lda #$10
ad_latch_length_at0:
    trb a:AdpcmControl
    rts

; @card 00:F729 ad_set_address
; WHAT IT DOES: Gives the ADPCM unit an address.
; HOW: Two bytes to its address ports.
; Entry: X low, Y high. Exit: all registers and flags kept.
ad_set_address:
    stx a:AdpcmAddressLo
    sty a:AdpcmAddressHi
    rts
