; @card 00:F379 cd_fade
; WHAT IT DOES: Starts or stops the CD and ADPCM volume fade.
; HOW: The low nibble of the type goes to the fade port.
; Entry: A type. Exit: A the nibble, N and Z from it. X, Y, C, V kept.
cd_fade:
    and #$0F
    sta a:CdFade
    rts
