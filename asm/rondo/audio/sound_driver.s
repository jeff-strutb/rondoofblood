; @routine 004E:614 SoundSilenceAll
; @run 75:8E14
; WHAT IT DOES: Turns every PSG channel off, and the main volume down.
; HOW: Selects channels 5 down to 0 and clears each one's control; then the main volume.
; Entry: nothing. Exit: A, X, Y, C and V kept; N and Z from X. zSoundChannel 0.
SoundSilenceAll:
    phx
    ldx #$05
SoundSilenceAll_at0:
    stx z:zSoundChannel
    stx a:PsgSelect
    stz a:PsgControl
    dex
    bpl SoundSilenceAll_at0
    stz a:PsgMainVolume
    plx
    rts

; @routine 004E:627 SoundVolumeFull
; @run 75:8E27
; WHAT IT DOES: Turns the main volume full up, left and right.
; HOW: $FF to the main volume.
; Entry: nothing. Exit: A $FF, N set, Z clear. X, Y, C and V kept.
SoundVolumeFull:
    lda #$FF
    sta a:PsgMainVolume
    rts

; @routine 004E:62D SoundMarkChannel
; @run 75:8E2D
; WHAT IT DOES: Marks a channel in SoundChannelMarked.
; HOW: Stores 1.
; Entry: X channel. Exit: A, X, Y, C and V kept; N and Z from A.
SoundMarkChannel:
    pha
    lda #$01
    sta a:SoundChannelMarked,x
    pla
    rts

; @routine 004E:635 SoundReleaseChannel
; @run 75:8E35
; WHAT IT DOES: Releases a channel: SoundChannel25A8 to $1F, the mark cleared, and the PSG
; channel's control and balance cleared, unless it is 4 or 5 and an effect plays on it.
; HOW: The PSG channel is the one already selected.
; Entry: X channel. Exit: A, X and Y kept; N and Z from A; C set if the channel is 6 or more; V
; bit 6 of the effect's command when channel 4 or 5, else kept.
SoundReleaseChannel:
    pha
    lda #$1F
    sta a:SoundChannel25A8,x
    stz a:SoundChannelMarked,x
    cpx #$04
    bcc SoundReleaseChannel_at0
    cpx #$06
    bcs SoundReleaseChannel_at0
    tst #$FF,a:SoundChannelCommand+2,x
    bne SoundReleaseChannel_at1
SoundReleaseChannel_at0:
    stz a:PsgControl
    stz a:PsgBalance
SoundReleaseChannel_at1:
    pla
    rts

; @routine 004E:746 SoundMultiply
; @run 75:8F46
; WHAT IT DOES: Multiplies two bytes into a word in zSoundWordLo and zSoundWordHi.
; HOW: Eight steps of shift and add, the multiplier from zSoundMultiplier.
; Entry: A and Y the bytes. Exit: X and Y kept, N and Z from X; C clear; A the product's high
; byte as of the last add, V from that add, or A and V kept when A was 0.
SoundMultiply:
    phx
    sta z:zSoundMultiplier
    ldx #$08
    stz z:zSoundWordLo
    stz z:zSoundWordHi
SoundMultiply_at1:
    asl z:zSoundWordLo
    rol z:zSoundWordHi
    asl z:zSoundMultiplier
    bcc SoundMultiply_at0
    tya
    clc
    adc z:zSoundWordLo
    sta z:zSoundWordLo
    lda z:zSoundWordHi
    adc #$00
    sta z:zSoundWordHi
SoundMultiply_at0:
    dex
    bne SoundMultiply_at1
    plx
    rts

; @routine 004E:768 SoundMultiplyNibble
; @run 75:8F68
; WHAT IT DOES: Multiplies the low nibble of A by Y into zSoundWordLo and zSoundWordHi.
; HOW: A shifted up four places, then four steps of shift and add.
; Entry: A and Y. Exit: as SoundMultiply, with A 0 when the nibble was 0.
SoundMultiplyNibble:
    phx
    asl a
    asl a
    asl a
    asl a
    sta z:zSoundMultiplier
    ldx #$04
    stz z:zSoundWordLo
    stz z:zSoundWordHi
SoundMultiplyNibble_at1:
    asl z:zSoundWordLo
    rol z:zSoundWordHi
    asl z:zSoundMultiplier
    bcc SoundMultiplyNibble_at0
    tya
    clc
    adc z:zSoundWordLo
    sta z:zSoundWordLo
    lda z:zSoundWordHi
    adc #$00
    sta z:zSoundWordHi
SoundMultiplyNibble_at0:
    dex
    bne SoundMultiplyNibble_at1
    plx
    rts

; @routine 0053:274 SoundStartFade
; @run 76:B274
; WHAT IT DOES: Starts the music fading out, unless it already is.
; HOW: SoundFading set, SoundFadeStep $10.
; Entry: nothing. Exit: A SoundFading when it was set, else $10; N and Z from A. X, Y, C and V
; kept.
SoundStartFade:
    lda a:SoundFading
    bne SoundStartFade_at0
    lda #$01
    sta a:SoundFading
    lda #$10
    sta a:SoundFadeStep
SoundStartFade_at0:
    rts

; @routine 0053:62E SoundReadPair
; @run 76:B62E
; WHAT IT DOES: Reads the next two bytes of a sequence into a channel's pair.
; HOW: Through the pointer in zSoundWordLo and zSoundWordHi, moved on past them.
; Entry: X channel. Exit: A the second byte; N and Z from the pointer's low byte, or its high
; when the low wrapped to 0. X, Y, C and V kept.
SoundReadPair:
    lda (zSoundWordLo)
    sta a:SoundChannelPairA,x
    inc z:zSoundWordLo
    bne SoundReadPair_at0
    inc z:zSoundWordHi
SoundReadPair_at0:
    lda (zSoundWordLo)
    sta a:SoundChannelPairB,x
    inc z:zSoundWordLo
    bne SoundReadPair_at1
    inc z:zSoundWordHi
SoundReadPair_at1:
    rts
