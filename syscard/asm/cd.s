; @card 00:F379 cd_fade
; WHAT IT DOES: Starts or stops the CD and ADPCM volume fade.
; HOW: The low nibble of the type goes to the fade port.
; Entry: A type. Exit: A the nibble, N and Z from it. X, Y, C, V kept.
cd_fade:
    and #$0F
    sta a:CdFade
    rts

; @card 01:CB54 reset_cd_unit
; WHAT IT DOES: Puts the CD unit in a known state at boot: reset, its interrupts off, no fade,
; ADPCM reset, and any waiting subcode byte taken.
; HOW: cd_reset, then the interrupt enables and the fade port cleared, ad_reset, cd_subrd.
; Entry: nothing. Exit: X 0, Y 0; A, N, Z, C and V as cd_subrd leaves them, A $6F if no byte.
reset_cd_unit:
    jsr cd_reset
    stz a:CdIrqControl
    stz a:CdFade
    jsr ad_reset
    jsr cd_subrd
    rts

