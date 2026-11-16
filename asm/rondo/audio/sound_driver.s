; @routine 004D:5F1 SoundLoadSequence
; @run 75:85F1
; WHAT IT DOES: Takes up a channel's place in its sequence.
; HOW: SoundSequenceLo and Hi into zSoundSequenceLo and Hi.
; Entry: X channel. Exit: A the place's high byte, N and Z from it. X, Y, C and V kept.
SoundLoadSequence:
    lda a:SoundSequenceLo,x
    sta z:zSoundSequenceLo
    lda a:SoundSequenceHi,x
    sta z:zSoundSequenceHi
    rts

; @routine 004D:5FC SoundSaveSequence
; @run 75:85FC
; WHAT IT DOES: Keeps a channel's place in its sequence.
; HOW: zSoundSequenceLo and Hi into SoundSequenceLo and Hi.
; Entry: X channel. Exit: A the place's high byte, N and Z from it. X, Y, C and V kept.
SoundSaveSequence:
    lda z:zSoundSequenceLo
    sta a:SoundSequenceLo,x
    lda z:zSoundSequenceHi
    sta a:SoundSequenceHi,x
    rts

; @routine 004D:607 SoundSequenceFromWord
; @run 75:8607
; WHAT IT DOES: Moves the place in the sequence to the address in zSoundWordLo and Hi.
; HOW: Two bytes copied.
; Entry: nothing. Exit: A the address's high byte, N and Z from it. X, Y, C and V kept.
SoundSequenceFromWord:
    lda z:zSoundWordLo
    sta z:zSoundSequenceLo
    lda z:zSoundWordHi
    sta z:zSoundSequenceHi
    rts

; @routine 004D:743 SoundSlideStep
; @run 75:8743
; WHAT IT DOES: Moves a channel's slide on by its step, until its high byte reaches the target.
; HOW: SoundSlideStepLo and Hi onto SoundSlideLo and Hi; SoundSlideTarget.
; Entry: X channel. Exit: X and Y kept. No slide, A 0, Z set, N clear, C and V kept. At the
; target, A the high byte, Z and C set, N clear, V kept. Moved, A the high byte, N, Z, C and V
; from its add.
SoundSlideStep:
    lda a:SoundSlideStepLo,x
    ora a:SoundSlideStepHi,x
    beq SoundSlideStep_at0
    lda a:SoundSlideHi,x
    cmp a:SoundSlideTarget,x
    bne SoundSlideStep_at1
    stz a:SoundSlideStepLo,x
    stz a:SoundSlideStepHi,x
    bra SoundSlideStep_at0
SoundSlideStep_at1:
    lda a:SoundSlideStepLo,x
    clc
    adc a:SoundSlideLo,x
    sta a:SoundSlideLo,x
    lda a:SoundSlideStepHi,x
    adc a:SoundSlideHi,x
    sta a:SoundSlideHi,x
SoundSlideStep_at0:
    rts

; @routine 004D:7CC SoundVolumeSlideStep
; @run 75:87CC
; WHAT IT DOES: Moves a channel's volume on by its step, until it reaches the target.
; HOW: SoundVolumeStepLo and Hi onto SoundVolumeLo and SoundVolume; SoundVolumeTarget.
; Entry: X channel. Exit: as SoundSlideStep.
SoundVolumeSlideStep:
    lda a:SoundVolumeStepLo,x
    ora a:SoundVolumeStepHi,x
    beq SoundVolumeSlideStep_at0
    lda a:SoundVolume,x
    cmp a:SoundVolumeTarget,x
    bne SoundVolumeSlideStep_at1
    stz a:SoundVolumeStepLo,x
    stz a:SoundVolumeStepHi,x
    bra SoundVolumeSlideStep_at0
SoundVolumeSlideStep_at1:
    clc
    lda a:SoundVolumeStepLo,x
    adc a:SoundVolumeLo,x
    sta a:SoundVolumeLo,x
    lda a:SoundVolume,x
    adc a:SoundVolumeStepHi,x
    sta a:SoundVolume,x
SoundVolumeSlideStep_at0:
    rts

; @routine 004E:1F2 SoundModulate
; @run 75:89F2
; WHAT IT DOES: Adds a channel's modulation to the word in zSoundWordLo and Hi: a table word at
; its phase, masked by channel; the phase moves on by the channel's speed.
; HOW: SoundModulationSpeed onto SoundModulationPhaseLo, carrying into SoundModulationPhase,
; which indexes SoundModulationTable a byte a step; SoundModulationMaskLo and Hi.
; Entry: X channel. Exit: no speed, A 0, Z set, N clear, Y, C and V kept. Otherwise A the word's
; high byte, N, Z, C and V from adding to it, Y the phase before. X kept.
SoundModulate:
    lda a:SoundModulationSpeed,x
    beq SoundModulate_at0
    ldy a:SoundModulationPhase,x
    clc
    adc a:SoundModulationPhaseLo,x
    sta a:SoundModulationPhaseLo,x
    bcc SoundModulate_at1
    inc a:SoundModulationPhase,x
SoundModulate_at1:
    lda a:SoundModulationTable,y
    and a:SoundModulationMaskLo,x
    clc
    adc z:zSoundWordLo
    sta z:zSoundWordLo
    lda a:SoundModulationTable+1,y
    and a:SoundModulationMaskHi,x
    adc z:zSoundWordHi
    sta z:zSoundWordHi
SoundModulate_at0:
    rts

; @routine 004E:21C SoundRampStep
; @run 75:8A1C
; WHAT IT DOES: Moves a channel's ramp on by its step, once its wait is over and only while the
; slide is still at least the ramp away from where it started.
; HOW: SoundChannelWait, SoundRampBy, SoundSlideFrom less SoundSlideHi against SoundRampHi;
; SoundRampBy onto SoundRampLo, carrying into SoundRampHi.
; Entry: X channel. Exit: X and Y kept. Waiting, A the wait, N and Z from it. No step, A 0, Z
; set, N clear. Too close, A the distance, N and Z from it less the ramp, C clear, V from the
; distance's subtract. Moved, A the ramp, N, Z, C and V from its add.
SoundRampStep:
    lda a:SoundChannelWait,x
    bne SoundRampStep_at0
    lda a:SoundRampBy,x
    beq SoundRampStep_at0
    lda a:SoundSlideFrom,x
    sec
    sbc a:SoundSlideHi,x
    cmp a:SoundRampHi,x
    bcc SoundRampStep_at0
    lda a:SoundRampLo,x
    clc
    adc a:SoundRampBy,x
    sta a:SoundRampLo,x
    lda a:SoundRampHi,x
    adc #$00
    sta a:SoundRampHi,x
SoundRampStep_at0:
    rts

; @routine 004E:245 SoundApproachStep
; @run 75:8A45
; WHAT IT DOES: Moves a channel's approaching value toward its target by its step, as 8.8, and
; ends the approach when it gets there.
; HOW: SoundApproachBy added to or taken from SoundApproachLo, carrying into SoundApproach;
; SoundApproachTarget.
; Entry: X channel. Exit: X and Y kept. No step, A 0, Z set, N clear. There, A the target, Z and
; C set, N clear. Moved, A the fraction, C and V from its add or subtract, N and Z from it, or
; from the value when that carried into it.
SoundApproachStep:
    lda a:SoundApproachBy,x
    beq SoundApproachStep_at0
    lda a:SoundApproachTarget,x
    cmp a:SoundApproach,x
    bne SoundApproachStep_at1
    stz a:SoundApproachBy,x
    bra SoundApproachStep_at0
SoundApproachStep_at1:
    bcc SoundApproachStep_at2
    lda a:SoundApproachLo,x
    clc
    adc a:SoundApproachBy,x
    sta a:SoundApproachLo,x
    bcc SoundApproachStep_at0
    inc a:SoundApproach,x
    bra SoundApproachStep_at0
SoundApproachStep_at2:
    lda a:SoundApproachLo,x
    sec
    sbc a:SoundApproachBy,x
    sta a:SoundApproachLo,x
    bcs SoundApproachStep_at0
    dec a:SoundApproach,x
SoundApproachStep_at0:
    rts

; @routine 004E:310 SoundSetVolume
; @run 75:8B10
; WHAT IT DOES: Sets a channel's volume outright, as set and as it plays, with no fraction.
; HOW: SoundVolumeSet and SoundVolume, SoundVolumeLo cleared.
; Entry: A volume, X channel. Exit: all registers and flags kept.
SoundSetVolume:
    sta a:SoundVolumeSet,x
    sta a:SoundVolume,x
    stz a:SoundVolumeLo,x
    rts

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
; WHAT IT DOES: Reads a sequence's address for a channel: the next two bytes, its place low and
; high.
; HOW: Through the pointer in zSoundWordLo and zSoundWordHi, moved on past them.
; Entry: X channel. Exit: A the second byte; N and Z from the pointer's low byte, or its high
; when the low wrapped to 0. X, Y, C and V kept.
SoundReadPair:
    lda (zSoundWordLo)
    sta a:SoundSequenceLo,x
    inc z:zSoundWordLo
    bne SoundReadPair_at0
    inc z:zSoundWordHi
SoundReadPair_at0:
    lda (zSoundWordLo)
    sta a:SoundSequenceHi,x
    inc z:zSoundWordLo
    bne SoundReadPair_at1
    inc z:zSoundWordHi
SoundReadPair_at1:
    rts

; @routine 0053:6A3 SoundReadCommand
; @run 76:B6A3
; WHAT IT DOES: Reads the channel's next sequence command and its arguments, moving its place on.
; HOW: Through zSoundSequenceLo and Hi; SoundCommandLength says how many arguments follow.
; Entry: X channel. Exit: A $FF, or the count less one when it is above $80; N set, Z clear; Y
; the argument bytes read; C and V as SoundCommandLength left them. X kept.
SoundReadCommand:
    lda (zSoundSequenceLo)
    sta a:SoundCommandByte
    inc z:zSoundSequenceLo
    bne SoundReadCommand_at2
    inc z:zSoundSequenceHi
SoundReadCommand_at2:
    jsr SoundCommandLength
    cly
SoundReadCommand_at1:
    dec a
    bmi SoundReadCommand_at0
    pha
    lda (zSoundSequenceLo)
    sta a:SoundCommandArgs,y
    iny
    pla
    inc z:zSoundSequenceLo
    bne SoundReadCommand_at1
    inc z:zSoundSequenceHi
    bra SoundReadCommand_at1
SoundReadCommand_at0:
    rts

; @routine 0053:6C6 SoundPeekCommand
; @run 76:B6C6
; WHAT IT DOES: Reads the channel's next sequence command and its arguments without moving its
; place on.
; HOW: A copy of the place in zSoundWordLo and Hi is read through instead.
; Entry: X channel. Exit: as SoundReadCommand.
SoundPeekCommand:
    lda z:zSoundSequenceLo
    sta z:zSoundWordLo
    lda z:zSoundSequenceHi
    sta z:zSoundWordHi
    lda (zSoundWordLo)
    sta a:SoundCommandByte
    inc z:zSoundWordLo
    bne SoundPeekCommand_at0
    inc z:zSoundWordHi
SoundPeekCommand_at0:
    jsr SoundCommandLength
    cly
SoundPeekCommand_at1:
    dec a
    bmi SoundPeekCommand_at2
    pha
    lda (zSoundWordLo)
    sta a:SoundCommandArgs,y
    iny
    pla
    inc z:zSoundWordLo
    bne SoundPeekCommand_at1
    inc z:zSoundWordHi
    bra SoundPeekCommand_at1
SoundPeekCommand_at2:
    rts

; @routine 0053:6F1 SoundCommandLength
; @run 76:B6F1
; WHAT IT DOES: How many argument bytes the command in SoundCommandByte takes. Below $60 one,
; unless its bit 7 says to reuse the channel's default, which goes into SoundCommandArgs with none
; read; $60 up none, except the commands from $E0 up, whose lengths are a table.
; HOW: A with bit 7 off picks the class; SoundChannelDefaultArg; SoundCommandLengths.
; Entry: A the command byte, X channel. Exit: A the count. One, N and Z from it, C clear. A
; default, N and Z from the default, C set. None from $60 to $DF, N and Z from it less $E0, C
; clear. From the table, N and Z from the count, C set, V clear, Y the command less $E0.
; Otherwise X, Y and V kept.
SoundCommandLength:
    and #$7F
    cmp #$60
    bcs SoundCommandLength_at2
    lda a:SoundCommandByte
    cmp #$80
    bcs SoundCommandLength_at0
    lda #$01
    bra SoundCommandLength_at3
SoundCommandLength_at0:
    and #$7F
    sta a:SoundCommandByte
    lda a:SoundChannelDefaultArg,x
    sta a:SoundCommandArgs
    cla
    bra SoundCommandLength_at3
SoundCommandLength_at2:
    lda a:SoundCommandByte
    cmp #$E0
    bcs SoundCommandLength_at1
    cla
    bra SoundCommandLength_at3
SoundCommandLength_at1:
    sbc #$E0
    tay
    lda a:SoundCommandLengths,y
SoundCommandLength_at3:
    rts

; @routine 0053:721 SoundWaveAddress
; @run 76:B721
; WHAT IT DOES: Points the block-move stub's source at a waveform: 32 bytes each from $BD11.
; HOW: The waveform times 32, plus $BD11, into BlockMoveSourceLo and Hi.
; Entry: A waveform. Exit: A the address's high byte, N, Z, C and V from adding it. X and Y kept.
SoundWaveAddress:
    stz a:BlockMoveSourceLo
    lsr a
    ror a:BlockMoveSourceLo
    lsr a
    ror a:BlockMoveSourceLo
    lsr a
    ror a:BlockMoveSourceLo
    sta a:BlockMoveSourceHi
    lda a:BlockMoveSourceLo
    clc
    adc #$11
    sta a:BlockMoveSourceLo
    lda a:BlockMoveSourceHi
    adc #$BD
    sta a:BlockMoveSourceHi
    rts
