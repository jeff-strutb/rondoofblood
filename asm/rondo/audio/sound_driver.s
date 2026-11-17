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

; @routine 004D:7CC SoundPanSlideStep
; @run 75:87CC
; WHAT IT DOES: Moves a channel's pan on by its step, until it reaches the target.
; HOW: SoundPanStepLo and Hi onto SoundPanLo and SoundPan; SoundPanTarget.
; Entry: X channel. Exit: as SoundSlideStep.
SoundPanSlideStep:
    lda a:SoundPanStepLo,x
    ora a:SoundPanStepHi,x
    beq SoundPanSlideStep_at0
    lda a:SoundPan,x
    cmp a:SoundPanTarget,x
    bne SoundPanSlideStep_at1
    stz a:SoundPanStepLo,x
    stz a:SoundPanStepHi,x
    bra SoundPanSlideStep_at0
SoundPanSlideStep_at1:
    clc
    lda a:SoundPanStepLo,x
    adc a:SoundPanLo,x
    sta a:SoundPanLo,x
    lda a:SoundPan,x
    adc a:SoundPanStepHi,x
    sta a:SoundPan,x
SoundPanSlideStep_at0:
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
; WHAT IT DOES: Moves a channel's ramp on by its step, once the note's gate is over and only while
; the volume less the slide is still at least the ramp.
; HOW: SoundGateTimer, SoundRampBy, SoundVolumeBase less SoundSlideHi against SoundRampHi;
; SoundRampBy onto SoundRampLo, carrying into SoundRampHi.
; Entry: X channel. Exit: X and Y kept. Waiting, A the wait, N and Z from it. No step, A 0, Z
; set, N clear. Too close, A the distance, N and Z from it less the ramp, C clear, V from the
; distance's subtract. Moved, A the ramp, N, Z, C and V from its add.
SoundRampStep:
    lda a:SoundGateTimer,x
    bne SoundRampStep_at0
    lda a:SoundRampBy,x
    beq SoundRampStep_at0
    lda a:SoundVolumeBase,x
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

; @routine 004E:245 SoundTempoSlideStep
; @run 75:8A45
; WHAT IT DOES: Moves a channel's tempo toward its target by its step, as 8.8, and ends the slide
; when it gets there.
; HOW: SoundTempoSlideBy added to or taken from SoundTempoLo, carrying into SoundTempo;
; SoundTempoTarget.
; Entry: X channel. Exit: X and Y kept. No step, A 0, Z set, N clear. There, A the target, Z and
; C set, N clear. Moved, A the fraction, C and V from its add or subtract, N and Z from it, or
; from the value when that carried into it.
SoundTempoSlideStep:
    lda a:SoundTempoSlideBy,x
    beq SoundTempoSlideStep_at0
    lda a:SoundTempoTarget,x
    cmp a:SoundTempo,x
    bne SoundTempoSlideStep_at1
    stz a:SoundTempoSlideBy,x
    bra SoundTempoSlideStep_at0
SoundTempoSlideStep_at1:
    bcc SoundTempoSlideStep_at2
    lda a:SoundTempoLo,x
    clc
    adc a:SoundTempoSlideBy,x
    sta a:SoundTempoLo,x
    bcc SoundTempoSlideStep_at0
    inc a:SoundTempo,x
    bra SoundTempoSlideStep_at0
SoundTempoSlideStep_at2:
    lda a:SoundTempoLo,x
    sec
    sbc a:SoundTempoSlideBy,x
    sta a:SoundTempoLo,x
    bcs SoundTempoSlideStep_at0
    dec a:SoundTempo,x
SoundTempoSlideStep_at0:
    rts

; @routine 004E:310 SoundSetPan
; @run 75:8B10
; WHAT IT DOES: Sets a channel's pan outright, as set and as it plays, with no fraction.
; HOW: SoundPanSet and SoundPan, SoundPanLo cleared.
; Entry: A pan, X channel. Exit: all registers and flags kept.
SoundSetPan:
    sta a:SoundPanSet,x
    sta a:SoundPan,x
    stz a:SoundPanLo,x
SoundSetPan_at0:
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

; @routine 004E:62D SoundStartEnvelope
; @run 75:8E2D
; WHAT IT DOES: Starts a channel's envelope from its first phase.
; HOW: SoundEnvelopePhase 1.
; Entry: X channel. Exit: A, X, Y, C and V kept; N and Z from A.
SoundStartEnvelope:
    pha
    lda #$01
    sta a:SoundEnvelopePhase,x
    pla
    rts

; @routine 004E:635 SoundSilenceChannel
; @run 75:8E35
; WHAT IT DOES: Silences a channel: its envelope's level to $1F and its phase off, and the PSG
; channel's control and balance cleared, unless it is 4 or 5 and an effect plays on it.
; HOW: The PSG channel is the one already selected.
; Entry: X channel. Exit: A, X and Y kept; N and Z from A; C set if the channel is 6 or more; V
; bit 6 of the effect's command when channel 4 or 5, else kept.
SoundSilenceChannel:
    pha
    lda #$1F
    sta a:SoundEnvelopeLevel,x
    stz a:SoundEnvelopePhase,x
    cpx #$04
    bcc SoundSilenceChannel_at0
    cpx #$06
    bcs SoundSilenceChannel_at0
    tst #$FF,a:SoundChannelCommand+2,x
    bne SoundSilenceChannel_at1
SoundSilenceChannel_at0:
    stz a:PsgControl
    stz a:PsgBalance
SoundSilenceChannel_at1:
    pla
    rts

; @routine 004E:746 SoundMultiply
; @run 75:8F46
; WHAT IT DOES: Multiplies two bytes into a word in zSoundWordLo and zSoundWordHi.
; HOW: Eight steps of shift and add, the multiplier from zSoundScratch.
; Entry: A and Y the bytes. Exit: X and Y kept, N and Z from X; C clear; A the product's high
; byte as of the last add, V from that add, or A and V kept when A was 0.
SoundMultiply:
    phx
    sta z:zSoundScratch
    ldx #$08
    stz z:zSoundWordLo
    stz z:zSoundWordHi
SoundMultiply_at1:
    asl z:zSoundWordLo
    rol z:zSoundWordHi
    asl z:zSoundScratch
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
    sta z:zSoundScratch
    ldx #$04
    stz z:zSoundWordLo
    stz z:zSoundWordHi
SoundMultiplyNibble_at1:
    asl z:zSoundWordLo
    rol z:zSoundWordHi
    asl z:zSoundScratch
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

; @routine 004E:0FF SoundScaledOffset
; @run 75:88FF
; WHAT IT DOES: A signed offset into zSoundWordLo and Hi: an entry of SoundOffsetTable times the
; channel's SoundVibratoDepth, negated when bit 6 of the byte is set.
; HOW: The byte's low six bits pick the entry; SoundMultiply.
; Entry: A the byte, X channel. Exit: Y the scale; X kept. Positive, as SoundMultiply. Negative, A
; the word's high byte, N, Z, C and V from subtracting it from 0 with the low byte's borrow.
SoundScaledOffset:
    and #$7F
    cmp #$40
    bcs SoundScaledOffset_at0
    tay
    lda a:SoundOffsetTable,y
    ldy a:SoundVibratoDepth,x
    jsr SoundMultiply
    bra SoundScaledOffset_at1
SoundScaledOffset_at0:
    and #$3F
    tay
    lda a:SoundOffsetTable,y
    ldy a:SoundVibratoDepth,x
    jsr SoundMultiply
    cla
    sec
    sbc z:zSoundWordLo
    sta z:zSoundWordLo
    cla
    sbc z:zSoundWordHi
    sta z:zSoundWordHi
SoundScaledOffset_at1:
    rts

; @routine 004E:590 SoundEnvelopeStep
; @run 75:8D90
; WHAT IT DOES: One frame of a channel's volume envelope. Start: the attack's level and timer, or
; straight to the decay with no attack. Attack: a step louder each time its timer runs out, to
; the decay at full volume. Decay: a step quieter, to the release at the sustain level. Release:
; a step quieter each time, without end. Off, and past the release, nothing.
; HOW: SoundEnvelopePhase, SoundEnvelopeTimer and SoundEnvelopeLevel by channel, with the rates
; and levels set for it. The release compares A, which is 0 there, with $1F, so it never stops
; itself.
; Entry: X channel. Exit: X, Y and V kept. Off, A $FF, N set, Z clear. Past the release, A the
; phase less 4, N and Z from it. Timer still running, A 0, N and Z from the timer. Timer set, A
; the frames it was set to, N and Z from them; C from comparing the level with the sustain in the
; decay, clear in the release, else kept.
SoundEnvelopeStep:
    lda a:SoundEnvelopePhase,x
    dec a
    bmi SoundEnvelopeStep_at0
    bne SoundEnvelopeStep_at1
    inc a:SoundEnvelopePhase,x
    lda a:SoundAttackLevel,x
    sta a:SoundEnvelopeLevel,x
    lda a:SoundAttackRate,x
    beq SoundEnvelopeStep_at2
    sta a:SoundEnvelopeTimer,x
    bra SoundEnvelopeStep_at0
SoundEnvelopeStep_at1:
    dec a
    bne SoundEnvelopeStep_at3
    dec a:SoundEnvelopeTimer,x
    bne SoundEnvelopeStep_at0
    dec a:SoundEnvelopeLevel,x
    beq SoundEnvelopeStep_at2
    lda a:SoundAttackRate,x
    sta a:SoundEnvelopeTimer,x
    bra SoundEnvelopeStep_at0
SoundEnvelopeStep_at2:
    inc a:SoundEnvelopePhase,x
    lda a:SoundDecayRate,x
    sta a:SoundEnvelopeTimer,x
    bra SoundEnvelopeStep_at0
SoundEnvelopeStep_at3:
    dec a
    bne SoundEnvelopeStep_at4
    dec a:SoundEnvelopeTimer,x
    bne SoundEnvelopeStep_at0
    inc a:SoundEnvelopeLevel,x
    lda a:SoundEnvelopeLevel,x
    cmp a:SoundSustainLevel,x
    beq SoundEnvelopeStep_at5
    lda a:SoundDecayRate,x
    sta a:SoundEnvelopeTimer,x
    bra SoundEnvelopeStep_at0
SoundEnvelopeStep_at5:
    inc a:SoundEnvelopePhase,x
    lda a:SoundReleaseRate,x
    sta a:SoundEnvelopeTimer,x
    bra SoundEnvelopeStep_at0
SoundEnvelopeStep_at4:
    dec a
    bne SoundEnvelopeStep_at0
    dec a:SoundEnvelopeTimer,x
    bne SoundEnvelopeStep_at0
    inc a:SoundEnvelopeLevel,x
    cmp #$1F
    bne SoundEnvelopeStep_at6
    stz a:SoundEnvelopePhase,x
    bra SoundEnvelopeStep_at0
SoundEnvelopeStep_at6:
    lda a:SoundReleaseRate,x
    sta a:SoundEnvelopeTimer,x
SoundEnvelopeStep_at0:
    rts

; @routine 004E:27A SoundApplyVolume
; @run 75:8A7A
; WHAT IT DOES: A frame of the channel's envelope, then its PSG volume: the base, less the
; envelope, the slide, the ramp and what is given, less the fade on the six PSG channels, plus
; the two additions; below 0 it is 0, above $1F it is $1F.
; HOW: zSoundWordLo holds what is given; SoundEnvelopeStep; SoundSetControl.
; Entry: A what to take off, X channel. Exit: as SoundSetControl, with A the volume when left
; alone.
SoundApplyVolume:
    sta z:zSoundWordLo
    jsr SoundEnvelopeStep
    lda a:SoundVolumeBase,x
    sec
    sbc a:SoundEnvelopeLevel,x
    sec
    sbc a:SoundSlideHi,x
    sec
    sbc a:SoundRampHi,x
    sec
    sbc z:zSoundWordLo
    cpx #$06
    bcs SoundApplyVolume_at0
    sec
    sbc a:SoundFading
SoundApplyVolume_at0:
    clc
    adc a:SoundVolumeAddA,x
    clc
    adc a:SoundVolumeAddB,x
    bpl SoundApplyVolume_at1
    cla
    bra SoundApplyVolume_at2
SoundApplyVolume_at1:
    cmp #$20
    bcc SoundApplyVolume_at2
    lda #$1F
SoundApplyVolume_at2:
    jsr SoundSetControl
    rts

; @routine 004E:2B0 SoundRest
; @run 75:8AB0
; WHAT IT DOES: A rest: the channel silenced for the length in the command's argument, with no
; gate and no volume.
; HOW: SoundChannel2550 1, SoundSilenceChannel; the length into SoundChannelDefaultArg and
; SoundNoteLength.
; Entry: X channel. Exit: A the length, N and Z from it; C and V as SoundSilenceChannel. X and Y
; kept.
SoundRest:
    lda #$01
    sta a:SoundChannel2550,x
    jsr SoundSilenceChannel
    lda a:SoundCommandArgs
    sta a:SoundChannelDefaultArg,x
    sta a:SoundNoteLength,x
    stz a:SoundGate,x
    stz a:SoundGateTimer,x
    stz a:SoundVolumeBase,x
    rts

; @routine 004E:2CB SoundSetLength
; @run 75:8ACB
; WHAT IT DOES: Sets a note's length and its gate from the command's two arguments: the gate is
; that many sixteenths of the length, at least one frame; a gate of 0 sounds the whole length.
; HOW: SoundNoteLength and SoundChannelDefaultArg, SoundGate and SoundGateSet;
; SoundMultiplyNibble, its product shifted down four places, into SoundGateTimer.
; Entry: X channel. Exit: A the gate timer, N and Z from it. No gate, C, V and Y kept. Otherwise
; Y the length, C bit 3 of the product, V as SoundMultiplyNibble. X kept.
SoundSetLength:
    lda a:SoundCommandArgs
    sta a:SoundChannelDefaultArg,x
    sta a:SoundNoteLength,x
    lda a:SoundCommandArgs+1
    sta a:SoundGate,x
    sta a:SoundGateSet,x
    bne SoundSetLength_at0
    lda a:SoundCommandArgs
    bra SoundSetLength_at1
SoundSetLength_at0:
    ldy a:SoundCommandArgs
    jsr SoundMultiplyNibble
    lda z:zSoundWordLo
    lsr z:zSoundWordHi
    ror a
    lsr z:zSoundWordHi
    ror a
    lsr z:zSoundWordHi
    ror a
    lsr z:zSoundWordHi
    ror a
    bne SoundSetLength_at1
    inc a
SoundSetLength_at1:
    sta a:SoundGateTimer,x
    rts

; @routine 004E:6E5 SoundSetControl
; @run 75:8EE5
; WHAT IT DOES: Sets the selected PSG channel's volume, on when not 0, and its balance by the
; channel's pan; channels 4 and 5 are left alone while an effect plays on them.
; HOW: The volume with bit 7 to the control; SoundBalances by SoundPan, the second set when
; zSoundAltBalance is set.
; Entry: A volume, X channel. Exit: X kept; C set if the channel is 6 or more. Left alone, A and
; Y kept, N and V the effect's bits 7 and 6, Z clear. Otherwise A the balance, N and Z from it, Y
; the pan; V clear for channels 4 and 5, else kept.
SoundSetControl:
    cpx #$04
    bcc SoundSetControl_at0
    cpx #$06
    bcs SoundSetControl_at0
    tst #$FF,a:SoundChannelCommand+2,x
    bne SoundSetControl_at1
SoundSetControl_at0:
    and #$FF
    beq SoundSetControl_at2
    ora #$80
SoundSetControl_at2:
    sta a:PsgControl
    ldy a:SoundPan,x
    tst #$FF,z:zSoundAltBalance
    bne SoundSetControl_at3
    lda a:SoundBalances,y
    bra SoundSetControl_at4
SoundSetControl_at3:
    lda a:SoundBalances+16,y
SoundSetControl_at4:
    sta a:PsgBalance
SoundSetControl_at1:
    rts

; @routine 004E:654 SoundLoadWave
; @run 75:8E54
; WHAT IT DOES: Loads a waveform into the selected PSG channel: the channel off, its write place
; reset, the 32 samples written. $FF loads none, and channels 4 and 5 are left alone while an
; effect plays on them.
; HOW: SoundWaveAddress, then the RAM block-move stub at $22E7 built as a TIN of 32 bytes to
; PsgWaveData and called.
; Entry: A waveform, X channel. Exit: X and Y kept. $FF, A $FF, Z and C set, N clear, V kept.
; Left alone, A kept, N and V the effect's bits 7 and 6, Z and C clear. Loaded, A $60, N and Z
; clear, C and V as SoundWaveAddress.
SoundLoadWave:
    cmp #$FF
    beq SoundLoadWave_at0
    cpx #$04
    bcc SoundLoadWave_at1
    cpx #$06
    bcs SoundLoadWave_at1
    tst #$FF,a:SoundChannelCommand+2,x
    bne SoundLoadWave_at0
SoundLoadWave_at1:
    pha
    stz a:PsgBalance
    stz a:PsgControl
    lda #$40
    sta a:PsgControl
    stz a:PsgControl
    pla
    jsr SoundWaveAddress
    lda #$06
    sta a:BlockMoveDestLo
    lda #$08
    sta a:BlockMoveDestHi
    lda #$D3
    sta a:BlockMoveStub
    lda #$20
    sta a:BlockMoveLengthLo
    lda #$00
    sta a:BlockMoveLengthHi
    lda #$60
    sta a:BlockMoveReturn
    jsr BlockMoveStub
    stz a:PsgControl
SoundLoadWave_at0:
    rts

; @routine 004E:69E SoundSetFrequency
; @run 75:8E9E
; WHAT IT DOES: Sets the selected PSG channel's pitch for a note: the note's period less a fine
; share of the step to the next note; on a channel whose waveform is noise, the noise frequency
; instead. Channels 4 and 5 are left alone while an effect plays on them.
; HOW: SoundPeriods by note; the fine part times the step, SoundMultiply, its high byte off the
; period; noise off for a tone. Noise takes the note less one, its low five bits.
; Entry: A the fine part, X channel, Y note. Exit: X kept. Left alone, A and Y kept, N and V the
; effect's bits 7 and 6, Z and C clear. Noise, A the noise byte, N and Z from it, C set, Y the
; note less one. Tone, A the period's high byte, N, Z, C and V from its subtract, Y twice the
; note. V for channels 4 and 5 clear otherwise.
SoundSetFrequency:
    cpx #$04
    bcc SoundSetFrequency_at0
    cpx #$06
    bcs SoundSetFrequency_at0
    tst #$FF,a:SoundChannelCommand+2,x
    bne SoundSetFrequency_at1
SoundSetFrequency_at0:
    sta z:zSoundFine
    lda a:SoundChannelWave,x
    cmp #$FF
    beq SoundSetFrequency_at2
    tya
    asl a
    tay
    lda a:SoundPeriods,y
    sec
    sbc a:SoundPeriods+2,y
    phy
    ldy z:zSoundFine
    jsr SoundMultiply
    stz a:PsgNoise
    ply
    lda a:SoundPeriods,y
    sec
    sbc z:zSoundWordHi
    sta a:PsgFreqLo
    lda a:SoundPeriods+1,y
    sbc #$00
    sta a:PsgFreqHi
SoundSetFrequency_at1:
    rts
SoundSetFrequency_at2:
    dey
    tya
    and #$1F
    ora #$80
    sta a:PsgNoise
    rts

; @routine 0053:233 SoundInit
; @run 76:B233
; WHAT IT DOES: Starts the driver: every channel cleared, the PSG silent, no fade, nothing
; playing, the LFO off.
; HOW: SoundChannel25B0, SoundChannelCommand and SoundChannelPriority cleared by channel;
; SoundSilenceAll.
; Entry: nothing. Exit: A, X, Y and V kept; N and Z from X; C set by the loop's last compare.
SoundInit:
    phx
    pha
    clx
    stz a:SoundInCommand
SoundInit_at0:
    stz a:SoundChannel25B0,x
    stz a:SoundChannelCommand,x
    stz a:SoundChannelPriority,x
    stz z:zSoundAltBalance
    inx
    cpx #$08
    bne SoundInit_at0
    jsr SoundSilenceAll
    stz a:SoundFading
    stz z:zSoundChannelsOn
    stz a:PsgLfoFreq
    stz a:PsgLfoControl
    pla
    plx
    rts

; @routine 0053:25A SoundDriverStopAll
; @run 76:B25A
; WHAT IT DOES: Stops everything: no command or priority on any channel, no fade, nothing
; playing, the PSG silent, noise and the LFO off.
; HOW: The channels from 7 down; SoundSilenceAll.
; Entry: nothing. Exit: X $FF, N set, Z clear. A, Y, C and V kept.
SoundDriverStopAll:
    ldx #$07
SoundDriverStopAll_at0:
    stz a:SoundChannelCommand,x
    stz a:SoundChannelPriority,x
    dex
    bpl SoundDriverStopAll_at0
    stz a:SoundFading
    stz z:zSoundChannelsOn
    jsr SoundSilenceAll
    stz a:PsgNoise
    stz a:PsgLfoControl
    rts

; @routine 004D:76F SoundVolumeTick
; @run 75:876F
; WHAT IT DOES: A frame of a channel's volume: its ramp, then, with a volume at all, its PSG
; volume, less its tremolo once the tremolo's wait is over. Says whether the tremolo is still
; waiting.
; HOW: SoundRampStep; the tremolo's phase moved on, made positive and shifted up by its depth,
; the high byte taken off by SoundApplyVolume.
; Entry: X channel. Exit: no volume, A 0, Z set, N clear, C clear, Y and V as SoundRampStep.
; Otherwise as SoundApplyVolume, with C set while the tremolo waits, else clear; Y 0 after a
; tremolo where it keeps it.
SoundVolumeTick:
    jsr SoundRampStep
    lda a:SoundVolumeBase,x
    beq SoundVolumeTick_at0
    lda a:SoundTremoloDepth,x
    beq SoundVolumeTick_at1
    lda a:SoundTremoloWait,x
    cmp a:SoundTremoloDelay,x
    beq SoundVolumeTick_at2
    cla
    jsr SoundApplyVolume
    sec
    rts
SoundVolumeTick_at2:
    lda a:SoundTremoloPhase,x
    clc
    adc a:SoundTremoloSpeed,x
    sta a:SoundTremoloPhase,x
    bpl SoundVolumeTick_at3
    eor #$FF
    inc a
SoundVolumeTick_at3:
    sta z:zSoundWordLo
    cla
    ldy a:SoundTremoloDepth,x
SoundVolumeTick_at4:
    asl z:zSoundWordLo
    rol a
    dey
    beq SoundVolumeTick_at1
    bra SoundVolumeTick_at4
SoundVolumeTick_at1:
    jsr SoundApplyVolume
SoundVolumeTick_at0:
    clc
    rts

; @routine 004E:02A SoundPitchTick
; @run 75:882A
; WHAT IT DOES: A frame of a channel's pitch: its glide, after its delay, a step toward the
; target note and at the end onto it; its vibrato, after its delay, growing to its depth; its
; modulation; then the PSG frequency for the pitch with those offsets.
; HOW: SoundGlide*, SoundVibrato* and SoundModulate by channel; zSoundScratch counts what
; changed; SoundSetFrequency with the fine part and the note.
; Entry: X channel. Exit: as SoundSetFrequency, with A the fine part, Y the note and C and V from
; adding the note where it keeps them.
SoundPitchTick:
    stz z:zSoundWordLo
    stz z:zSoundWordHi
    stz z:zSoundScratch
    lda a:SoundGlideFrames,x
    beq SoundPitchTick_at0
    lda a:SoundGlideDelay,x
    beq SoundPitchTick_at1
    dec a:SoundGlideDelay,x
    bra SoundPitchTick_at0
SoundPitchTick_at1:
    dec a:SoundGlideFrames,x
    bne SoundPitchTick_at2
    lda a:SoundGlideTargetFine,x
    sta a:SoundPitchFine,x
    lda a:SoundGlideTargetNote,x
    sta a:SoundPitchNote,x
    bra SoundPitchTick_at3
SoundPitchTick_at2:
    lda a:SoundPitchFine,x
    clc
    adc a:SoundGlideStepLo,x
    sta a:SoundPitchFine,x
    lda a:SoundPitchNote,x
    adc a:SoundGlideStepHi,x
    sta a:SoundPitchNote,x
SoundPitchTick_at3:
    inc z:zSoundScratch
SoundPitchTick_at0:
    lda a:SoundVibratoDepthMax,x
    beq SoundPitchTick_at4
    lda a:SoundVibratoWait,x
    cmp a:SoundVibratoDelay,x
    beq SoundPitchTick_at5
    inc a:SoundVibratoWait,x
    bra SoundPitchTick_at4
SoundPitchTick_at5:
    lda a:SoundVibratoDepth,x
    cmp a:SoundVibratoDepthMax,x
    bcs SoundPitchTick_at6
    lda a:SoundVibratoDepthLo,x
    clc
    adc a:SoundVibratoGrowLo,x
    sta a:SoundVibratoDepthLo,x
    lda a:SoundVibratoDepth,x
    adc a:SoundVibratoGrowHi,x
    sta a:SoundVibratoDepth,x
SoundPitchTick_at6:
    lda a:SoundVibratoPhase,x
    clc
    adc a:SoundVibratoSpeed,x
    sta a:SoundVibratoPhase,x
    jsr SoundScaledOffset
    inc z:zSoundScratch
SoundPitchTick_at4:
    lda a:SoundModulationSpeed,x
    beq SoundPitchTick_at7
    jsr SoundModulate
    inc z:zSoundScratch
SoundPitchTick_at7:
    lda a:SoundPitchFine,x
    clc
    adc z:zSoundWordLo
    pha
    lda a:SoundPitchNote,x
    adc z:zSoundWordHi
    tay
    pla
    jsr SoundSetFrequency
    rts

; @routine 004E:159 SoundPitchTickPlain
; @run 75:8959
; WHAT IT DOES: A frame of a channel's pitch, the plain way: the vibrato at its depth with no
; growing and no counting of its wait; the pitch; the glide's step scaled by the tempo's phase and
; added, or taken off when it goes down; the modulation; then the PSG frequency.
; HOW: The step is shifted down one place for each bit of SoundTempoPhase above its highest set bit,
; and one more: a power of two near the share of the tick gone.
; Entry: X channel. Exit: as SoundSetFrequency, with A the fine part, Y the note, N and Z from
; the note, and C and V from the last add or subtract where it keeps them.
SoundPitchTickPlain:
    stz z:zSoundWordLo
    stz z:zSoundWordHi
    stz z:zSoundScratch
    lda a:SoundVibratoDepthLo,x
    ora a:SoundVibratoDepth,x
    beq SoundPitchTickPlain_at0
    lda a:SoundVibratoWait,x
    cmp a:SoundVibratoDelay,x
    bne SoundPitchTickPlain_at0
    lda a:SoundVibratoPhase,x
    clc
    adc a:SoundVibratoSpeed,x
    sta a:SoundVibratoPhase,x
    jsr SoundScaledOffset
    inc z:zSoundScratch
SoundPitchTickPlain_at0:
    lda z:zSoundWordLo
    clc
    adc a:SoundPitchFine,x
    sta z:zSoundWordLo
    lda z:zSoundWordHi
    adc a:SoundPitchNote,x
    sta z:zSoundWordHi
    lda a:SoundGlideFrames,x
    beq SoundPitchTickPlain_at1
    lda a:SoundGlideDelay,x
    bne SoundPitchTickPlain_at1
    inc z:zSoundScratch
    lda a:SoundGlideStepLo,x
    sta z:zSoundFine
    lda a:SoundGlideStepHi,x
    sta z:zSoundStepHi
    bmi SoundPitchTickPlain_at2
    lda a:SoundTempoPhase,x
SoundPitchTickPlain_at3:
    lsr z:zSoundStepHi
    ror z:zSoundFine
    asl a
    bcc SoundPitchTickPlain_at3
    lda z:zSoundFine
    clc
    adc z:zSoundWordLo
    sta z:zSoundWordLo
    lda z:zSoundStepHi
    adc z:zSoundWordHi
    sta z:zSoundWordHi
    bra SoundPitchTickPlain_at1
SoundPitchTickPlain_at2:
    cla
    sec
    sbc z:zSoundFine
    sta z:zSoundFine
    cla
    sbc z:zSoundStepHi
    sta z:zSoundStepHi
    lda a:SoundTempoPhase,x
SoundPitchTickPlain_at4:
    lsr z:zSoundStepHi
    ror z:zSoundFine
    asl a
    bcc SoundPitchTickPlain_at4
    lda z:zSoundWordLo
    sec
    sbc z:zSoundFine
    sta z:zSoundWordLo
    lda z:zSoundWordHi
    sbc z:zSoundStepHi
    sta z:zSoundWordHi
SoundPitchTickPlain_at1:
    lda a:SoundModulationSpeed,x
    beq SoundPitchTickPlain_at5
    jsr SoundModulate
    inc z:zSoundScratch
SoundPitchTickPlain_at5:
    lda z:zSoundWordLo
    ldy z:zSoundWordHi
    jsr SoundSetFrequency
    rts

; @routine 004D:610 SoundStartNote
; @run 75:8610
; WHAT IT DOES: Starts a note from the command's three arguments: its length, its gate in
; sixteenths of the length, and its volume, $10 up; its pitch; then its gate timer, at least a
; frame, or the whole length with no gate.
; HOW: SoundChannelDefaultArg, SoundGate and SoundVolumeBase; SoundNotePitch; SoundNoteLength;
; SoundMultiplyNibble, its product shifted down four places, into SoundGateTimer.
; Entry: X channel. Exit: A the gate timer, N and Z from it, Y the length. No gate, C and V as
; SoundNotePitch left them; otherwise C bit 3 of the product, V as SoundMultiplyNibble. X kept.
SoundStartNote:
    lda a:SoundCommandArgs
    sta a:SoundChannelDefaultArg,x
    lda a:SoundCommandArgs+1
    and #$0F
    sta a:SoundGate,x
    lda a:SoundCommandArgs+2
    and #$0F
    clc
    adc #$10
    sta a:SoundVolumeBase,x
    jsr SoundNotePitch
    lda a:SoundChannelDefaultArg,x
    sta a:SoundNoteLength,x
    tay
    lda a:SoundGate,x
    bne SoundStartNote_at0
    lda a:SoundChannelDefaultArg,x
    bra SoundStartNote_at1
SoundStartNote_at0:
    jsr SoundMultiplyNibble
    lda z:zSoundWordLo
    lsr z:zSoundWordHi
    ror a
    lsr z:zSoundWordHi
    ror a
    lsr z:zSoundWordHi
    ror a
    lsr z:zSoundWordHi
    ror a
    bne SoundStartNote_at1
    inc a
SoundStartNote_at1:
    sta a:SoundGateTimer,x
    rts

; @routine 004D:655 SoundNotePitch
; @run 75:8655
; WHAT IT DOES: Starts a channel's note at the pitch of the command: the note with the
; transpose, the signed fine tune and both detunes; the vibrato from the start; a glide to it,
; from an offset below it or from the last note; the frequency, the envelope from its start, the
; tremolo and the ramp from the start; then a frame of the pitch.
; HOW: SoundCommandByte is the note. The last note's pitch is kept in zSoundScratch and
; zSoundFine for a glide from it; SoundGlideSetup; SoundSetFrequency; SoundStartEnvelope;
; SoundPitchTick.
; Entry: X channel. Exit: as SoundPitchTick.
SoundNotePitch:
    lda a:SoundCommandByte
    lda a:SoundPitchFine,x
    sta z:zSoundScratch
    lda a:SoundPitchNote,x
    sta z:zSoundFine
    lda a:SoundCommandByte
    clc
    adc a:SoundTranspose,x
    sta z:zSoundWordLo
    lda a:SoundFineTune,x
    bpl SoundNotePitch_at0
    dec z:zSoundWordLo
SoundNotePitch_at0:
    clc
    adc a:SoundDetuneAFine,x
    pha
    lda z:zSoundWordLo
    adc a:SoundDetuneANote,x
    sta z:zSoundWordLo
    pla
    clc
    adc a:SoundDetuneBFine,x
    sta a:SoundPitchFine,x
    lda z:zSoundWordLo
    adc a:SoundDetuneBNote,x
    sta a:SoundPitchNote,x
    stz a:SoundVibratoPhase,x
    stz a:SoundVibratoWait,x
    stz a:SoundVibratoDepthLo,x
    stz a:SoundVibratoDepth,x
    lda a:SoundGlideFramesSet,x
    sta a:SoundGlideFrames,x
    beq SoundNotePitch_at1
    lda a:SoundGlideDelaySet,x
    sta a:SoundGlideDelay,x
    lda a:SoundGlideFromLast,x
    bne SoundNotePitch_at2
    lda a:SoundPitchFine,x
    pha
    sec
    sbc a:SoundGlideOffsetFine,x
    sta a:SoundPitchFine,x
    lda a:SoundPitchNote,x
    tay
    sbc a:SoundGlideOffsetNote,x
    sta a:SoundPitchNote,x
    pla
    bra SoundNotePitch_at3
SoundNotePitch_at2:
    ldy a:SoundPitchNote,x
    lda a:SoundPitchFine,x
    pha
    lda z:zSoundScratch
    sta a:SoundPitchFine,x
    lda z:zSoundFine
    sta a:SoundPitchNote,x
    pla
SoundNotePitch_at3:
    jsr SoundGlideSetup
SoundNotePitch_at1:
    lda a:SoundPitchFine,x
    ldy a:SoundPitchNote,x
    jsr SoundSetFrequency
    jsr SoundStartEnvelope
    stz a:SoundTremoloPhase,x
    stz a:SoundTremoloWait,x
    stz a:SoundRampLo,x
    stz a:SoundRampHi,x
    jsr SoundPitchTick
    rts

; @routine 004D:6F6 SoundGlideSetup
; @run 75:86F6
; WHAT IT DOES: Aims a channel's glide at a pitch: the target kept, and the step a frame that
; gets there from the pitch it has now in the glide's frames. Nothing without a glide.
; HOW: The distance in zSoundWordLo and Hi, made positive for SoundDivide and the quotient made
; negative again; SoundGlideStepLo and Hi.
; Entry: A the target's fine part, Y its note, X channel. Exit: no glide, A and Y kept, Z set, N
; and V clear. Otherwise A the step's low byte, N and Z from it, Y 0, C and V as SoundDivide, or
; from negating the quotient's high byte when the glide goes down. X kept.
SoundGlideSetup:
    tst #$FF,a:SoundGlideFrames,x
    beq SoundGlideSetup_at0
    sta a:SoundGlideTargetFine,x
    tya
    sta a:SoundGlideTargetNote,x
    lda a:SoundGlideTargetFine,x
    sec
    sbc a:SoundPitchFine,x
    sta z:zSoundWordLo
    tya
    sbc a:SoundPitchNote,x
    sta z:zSoundWordHi
    bpl SoundGlideSetup_at1
    cla
    sec
    sbc z:zSoundWordLo
    sta z:zSoundWordLo
    cla
    sbc z:zSoundWordHi
    sta z:zSoundWordHi
    lda a:SoundGlideFrames,x
    jsr SoundDivide
    cla
    sec
    sbc z:zSoundQuotientLo
    sta z:zSoundQuotientLo
    cla
    sbc z:zSoundQuotientHi
    sta z:zSoundQuotientHi
    bra SoundGlideSetup_at2
SoundGlideSetup_at1:
    lda a:SoundGlideFrames,x
    jsr SoundDivide
SoundGlideSetup_at2:
    lda z:zSoundQuotientHi
    sta a:SoundGlideStepHi,x
    lda z:zSoundQuotientLo
    sta a:SoundGlideStepLo,x
SoundGlideSetup_at0:
    rts

; @routine 004E:710 SoundDivide
; @run 75:8F10
; WHAT IT DOES: Divides the word in zSoundWordLo and Hi by A: the quotient in zSoundQuotientLo
; and Hi, what is left in zSoundScratch and zSoundFine.
; HOW: Sixteen steps of shift and subtract. Where the remainder's low byte is under the divisor
; but its high byte is not 0, the high byte is taken down once before the subtract and again by
; its borrow; the C keeps that.
; Entry: A divisor. Exit: Y 0, Z set, N clear; X kept. A, C and V from the last step: after a
; subtract, A the remainder's high byte with C and V from it; else A its low byte, C and V clear.
SoundDivide:
    stz z:zSoundScratch
    stz z:zSoundFine
    sta z:zSoundStepHi
    stz z:zSoundQuotientLo
    stz z:zSoundQuotientHi
    ldy #$10
SoundDivide_at2:
    asl z:zSoundWordLo
    rol z:zSoundWordHi
    rol z:zSoundScratch
    rol z:zSoundFine
    asl z:zSoundQuotientLo
    rol z:zSoundQuotientHi
    lda z:zSoundScratch
    cmp z:zSoundStepHi
    bcs SoundDivide_at0
    tst #$FF,z:zSoundFine
    beq SoundDivide_at1
    dec z:zSoundFine
SoundDivide_at0:
    sec
    sbc z:zSoundStepHi
    sta z:zSoundScratch
    lda z:zSoundFine
    sbc #$00
    sta z:zSoundFine
    inc z:zSoundQuotientLo
SoundDivide_at1:
    dey
    bne SoundDivide_at2
    rts

; @routine 004E:2FF SoundCommandWave
; @run 75:8AFF
; WHAT IT DOES: Command $E2: the channel's waveform, loaded into its PSG channel.
; HOW: SoundChannelWave; SoundLoadWave.
; Entry: X channel. Exit: as SoundLoadWave.
SoundCommandWave:
    lda a:SoundCommandArgs
    sta a:SoundChannelWave,x
    jsr SoundLoadWave
    rts

; @routine 004E:309 SoundCommandPan
; @run 75:8B09
; WHAT IT DOES: Command $E3: the channel's pan, unless it is 0.
; HOW: SoundSetPan.
; Entry: X channel. Exit: A the pan, N and Z from it. X, Y, C and V kept.
SoundCommandPan:
    lda a:SoundCommandArgs
    bne SoundSetPan
    bra SoundSetPan_at0

; @routine 004E:31A SoundCommandPanSlide
; @run 75:8B1A
; WHAT IT DOES: Command $F8: a pan slide by a step toward a target pan, down when the target is
; below the pan as set.
; HOW: SoundPanStepLo and Hi, SoundPanTarget against SoundPanSet.
; Entry: X channel. Exit: X and Y kept. Up, A the target, N and Z from comparing it, C set, V
; kept. Down, A the step made negative, N set, Z clear, C and V from negating it.
SoundCommandPanSlide:
    lda a:SoundCommandArgs
    sta a:SoundPanStepLo,x
    stz a:SoundPanStepHi,x
    lda a:SoundCommandArgs+1
    sta a:SoundPanTarget,x
    cmp a:SoundPanSet,x
    bcs SoundCommandPanSlide_at0
    lda #$00
    sec
    sbc a:SoundCommandArgs
    sta a:SoundPanStepLo,x
    dec a:SoundPanStepHi,x
SoundCommandPanSlide_at0:
    rts

; @routine 004E:33B SoundCommandVibrato
; @run 75:8B3B
; WHAT IT DOES: Command $E4: the vibrato's delay, speed and the depth it grows to, by a whole
; step a frame, from no fraction.
; HOW: SoundVibratoDelay, SoundVibratoSpeed, SoundVibratoDepthMax and SoundVibratoGrowHi.
; Entry: X channel. Exit: A the depth, N and Z from it. X, Y, C and V kept.
SoundCommandVibrato:
    lda a:SoundCommandArgs
    sta a:SoundVibratoDelay,x
    lda a:SoundCommandArgs+1
    sta a:SoundVibratoSpeed,x
    lda a:SoundCommandArgs+2
    sta a:SoundVibratoDepthMax,x
    sta a:SoundVibratoGrowHi,x
    stz a:SoundVibratoDepthLo,x
    rts

; @routine 004E:354 SoundCommandVibratoGrow
; @run 75:8B54
; WHAT IT DOES: Command $F9: how fast the vibrato's depth grows, in sixteenths a frame.
; HOW: The argument shifted up four places into SoundVibratoGrowLo and Hi.
; Entry: X channel. Exit: A the low byte, N and Z from the high, C clear. X, Y and V kept.
SoundCommandVibratoGrow:
    stz a:SoundVibratoGrowHi,x
    lda a:SoundCommandArgs
    asl a
    rol a:SoundVibratoGrowHi,x
    asl a
    rol a:SoundVibratoGrowHi,x
    asl a
    rol a:SoundVibratoGrowHi,x
    asl a
    rol a:SoundVibratoGrowHi,x
    sta a:SoundVibratoGrowLo,x
    rts

; @routine 004E:36E SoundCommandModulation
; @run 75:8B6E
; WHAT IT DOES: Command $E5: the modulation's speed and its two masks, from the start.
; HOW: SoundModulationSpeed, SoundModulationMaskHi and Lo; the phase cleared.
; Entry: X channel. Exit: A the low mask, N and Z from it. X, Y, C and V kept.
SoundCommandModulation:
    lda a:SoundCommandArgs
    sta a:SoundModulationSpeed,x
    lda a:SoundCommandArgs+1
    sta a:SoundModulationMaskHi,x
    lda a:SoundCommandArgs+2
    sta a:SoundModulationMaskLo,x
    stz a:SoundModulationPhaseLo,x
    stz a:SoundModulationPhase,x
    rts

; @routine 004E:387 SoundCommandLoopA
; @run 75:8B87
; WHAT IT DOES: Command $E6: the start of loop A, with no passes made and nothing added yet.
; HOW: The place after the command into SoundLoopAStartLo and Hi; SoundLoopACount,
; SoundVolumeAddA and detune A cleared.
; Entry: X channel. Exit: A the place's high byte, N and Z from it. X, Y, C and V kept.
SoundCommandLoopA:
    lda z:zSoundSequenceLo
    sta a:SoundLoopAStartLo,x
    lda z:zSoundSequenceHi
    sta a:SoundLoopAStartHi,x
    stz a:SoundLoopACount,x
    stz a:SoundVolumeAddA,x
    stz a:SoundDetuneAFine,x
    stz a:SoundDetuneANote,x
    rts

; @routine 004E:39E SoundCommandLoopAEnd
; @run 75:8B9E
; WHAT IT DOES: Command $E7: the end of loop A: after its passes, on with what it added cleared;
; else the volume and detune A moved by the arguments and back to its start. 0 passes loops for
; ever.
; HOW: SoundLoopACount; SoundVolumeAddA; the detune in sixteenths, signed, onto detune A.
; Entry: X channel. Exit: done, A the passes, Z and C set, N clear, V kept. Back, A the start's
; high byte, N and Z from it, C and V from adding the detune's notes. X and Y kept.
SoundCommandLoopAEnd:
    lda a:SoundCommandArgs
    beq SoundCommandLoopAEnd_at0
    inc a:SoundLoopACount,x
    cmp a:SoundLoopACount,x
    beq SoundCommandLoopAEnd_at1
SoundCommandLoopAEnd_at0:
    lda a:SoundCommandArgs+1
    clc
    adc a:SoundVolumeAddA,x
    sta a:SoundVolumeAddA,x
    stz z:zSoundWordLo
    lda a:SoundCommandArgs+2
    bmi SoundCommandLoopAEnd_at2
    bra SoundCommandLoopAEnd_at3
SoundCommandLoopAEnd_at2:
    dec z:zSoundWordLo
SoundCommandLoopAEnd_at3:
    asl a
    rol z:zSoundWordLo
    asl a
    rol z:zSoundWordLo
    asl a
    rol z:zSoundWordLo
    asl a
    rol z:zSoundWordLo
    clc
    adc a:SoundDetuneAFine,x
    sta a:SoundDetuneAFine,x
    lda z:zSoundWordLo
    adc a:SoundDetuneANote,x
    sta a:SoundDetuneANote,x
    lda a:SoundLoopAStartLo,x
    sta z:zSoundSequenceLo
    lda a:SoundLoopAStartHi,x
    sta z:zSoundSequenceHi
    bra SoundCommandLoopAEnd_at4
SoundCommandLoopAEnd_at1:
    stz a:SoundVolumeAddA,x
    stz a:SoundDetuneAFine,x
    stz a:SoundDetuneANote,x
SoundCommandLoopAEnd_at4:
    rts

; @routine 004E:3F1 SoundCommandLoopB
; @run 75:8BF1
; WHAT IT DOES: Command $E8: the start of loop B, as loop A's.
; HOW: SoundLoopBStartLo and Hi; SoundLoopBCount, SoundVolumeAddB and detune B cleared.
; Entry: X channel. Exit: as SoundCommandLoopA.
SoundCommandLoopB:
    lda z:zSoundSequenceLo
    sta a:SoundLoopBStartLo,x
    lda z:zSoundSequenceHi
    sta a:SoundLoopBStartHi,x
    stz a:SoundLoopBCount,x
    stz a:SoundVolumeAddB,x
    stz a:SoundDetuneBFine,x
    stz a:SoundDetuneBNote,x
    rts

; @routine 004E:408 SoundCommandLoopBEnd
; @run 75:8C08
; WHAT IT DOES: Command $E9: the end of loop B, as loop A's.
; HOW: SoundLoopBCount; SoundVolumeAddB; detune B.
; Entry: X channel. Exit: as SoundCommandLoopAEnd.
SoundCommandLoopBEnd:
    lda a:SoundCommandArgs
    beq SoundCommandLoopBEnd_at0
    inc a:SoundLoopBCount,x
    cmp a:SoundLoopBCount,x
    beq SoundCommandLoopBEnd_at1
SoundCommandLoopBEnd_at0:
    lda a:SoundCommandArgs+1
    clc
    adc a:SoundVolumeAddB,x
    sta a:SoundVolumeAddB,x
    stz z:zSoundWordLo
    lda a:SoundCommandArgs+2
    bmi SoundCommandLoopBEnd_at2
    bra SoundCommandLoopBEnd_at3
SoundCommandLoopBEnd_at2:
    dec z:zSoundWordLo
SoundCommandLoopBEnd_at3:
    asl a
    rol z:zSoundWordLo
    asl a
    rol z:zSoundWordLo
    asl a
    rol z:zSoundWordLo
    asl a
    rol z:zSoundWordLo
    clc
    adc a:SoundDetuneBFine,x
    sta a:SoundDetuneBFine,x
    lda z:zSoundWordLo
    adc a:SoundDetuneBNote,x
    sta a:SoundDetuneBNote,x
    lda a:SoundLoopBStartLo,x
    sta z:zSoundSequenceLo
    lda a:SoundLoopBStartHi,x
    sta z:zSoundSequenceHi
    bra SoundCommandLoopBEnd_at4
SoundCommandLoopBEnd_at1:
    stz a:SoundVolumeAddB,x
    stz a:SoundDetuneBFine,x
    stz a:SoundDetuneBNote,x
SoundCommandLoopBEnd_at4:
    rts

; @routine 004E:45B SoundCommandTempo
; @run 75:8C5B
; WHAT IT DOES: Command $EA: the channel's tempo.
; HOW: SoundTempo.
; Entry: X channel. Exit: A the value, N and Z from it. X, Y, C and V kept.
SoundCommandTempo:
    lda a:SoundCommandArgs
    sta a:SoundTempo,x
    rts

; @routine 004E:462 SoundCommandTempoSlide
; @run 75:8C62
; WHAT IT DOES: Command $EB: a tempo slide by a step to a target tempo, from no fraction.
; HOW: SoundTempoSlideBy, SoundTempoLo cleared, SoundTempoTarget.
; Entry: X channel. Exit: A the target, N and Z from it. X, Y, C and V kept.
SoundCommandTempoSlide:
    lda a:SoundCommandArgs
    sta a:SoundTempoSlideBy,x
    stz a:SoundTempoLo,x
    lda a:SoundCommandArgs+1
    sta a:SoundTempoTarget,x
    rts

; @routine 004E:472 SoundCommandTranspose
; @run 75:8C72
; WHAT IT DOES: Command $EC: the channel's transpose.
; HOW: SoundTranspose.
; Entry: X channel. Exit: A the transpose, N and Z from it. X, Y, C and V kept.
SoundCommandTranspose:
    lda a:SoundCommandArgs
    sta a:SoundTranspose,x
    rts

; @routine 004E:479 SoundCommandTremolo
; @run 75:8C79
; WHAT IT DOES: Command $ED: the tremolo's delay, speed and depth.
; HOW: SoundTremoloDelay, SoundTremoloSpeed, SoundTremoloDepth.
; Entry: X channel. Exit: A the depth, N and Z from it. X, Y, C and V kept.
SoundCommandTremolo:
    lda a:SoundCommandArgs
    sta a:SoundTremoloDelay,x
    lda a:SoundCommandArgs+1
    sta a:SoundTremoloSpeed,x
    lda a:SoundCommandArgs+2
    sta a:SoundTremoloDepth,x
    rts

; @routine 004E:48C SoundCommandVolume
; @run 75:8C8C
; WHAT IT DOES: Command $EE: the channel's volume, 0 to $0F, as the slide's attenuation $0F less
; it, with no fraction.
; HOW: SoundSlideHi, SoundSlideLo cleared.
; Entry: X channel. Exit: A the attenuation, N, Z, C and V from its subtract. X and Y kept.
SoundCommandVolume:
    lda #$0F
    sec
    sbc a:SoundCommandArgs
    sta a:SoundSlideHi,x
    stz a:SoundSlideLo,x
    rts

; @routine 004E:499 SoundCommandVolumeSlide
; @run 75:8C99
; WHAT IT DOES: Command $EF: a volume slide by a step to a target volume, as the attenuation $0F
; less it; down in attenuation when the target is below the slide's.
; HOW: SoundSlideStepLo and Hi, SoundSlideTarget against SoundSlideHi.
; Entry: X channel. Exit: as SoundCommandPanSlide, with V from the target's subtract when up.
SoundCommandVolumeSlide:
    lda a:SoundCommandArgs
    sta a:SoundSlideStepLo,x
    stz a:SoundSlideStepHi,x
    lda #$0F
    sec
    sbc a:SoundCommandArgs+1
    sta a:SoundSlideTarget,x
    cmp a:SoundSlideHi,x
    bcs SoundCommandVolumeSlide_at0
    lda #$00
    sec
    sbc a:SoundCommandArgs
    sta a:SoundSlideStepLo,x
    dec a:SoundSlideStepHi,x
SoundCommandVolumeSlide_at0:
    rts

; @routine 004E:4BD SoundCommandGlideFromLast
; @run 75:8CBD
; WHAT IT DOES: Command $F0: glides from the last note to each new one, over the frames given,
; with no delay.
; HOW: SoundGlideFromLast 1, SoundGlideDelaySet 0, SoundGlideFramesSet.
; Entry: X channel. Exit: A the frames, N and Z from them. X, Y, C and V kept.
SoundCommandGlideFromLast:
    lda #$01
    sta a:SoundGlideFromLast,x
    stz a:SoundGlideDelaySet,x
    lda a:SoundCommandArgs
    sta a:SoundGlideFramesSet,x
    rts

; @routine 004E:4CC SoundCommandGlideFromBelow
; @run 75:8CCC
; WHAT IT DOES: Command $F1: glides into each new note from notes below it, after a delay, over
; the frames given.
; HOW: SoundGlideFromLast 0, SoundGlideDelaySet, SoundGlideFramesSet, SoundGlideOffsetNote, no
; fine offset.
; Entry: X channel. Exit: A the notes below, N and Z from them. X, Y, C and V kept.
SoundCommandGlideFromBelow:
    stz a:SoundGlideFromLast,x
    lda a:SoundCommandArgs
    sta a:SoundGlideDelaySet,x
    lda a:SoundCommandArgs+1
    sta a:SoundGlideFramesSet,x
    lda a:SoundCommandArgs+2
    sta a:SoundGlideOffsetNote,x
    stz a:SoundGlideOffsetFine,x
    rts

; @routine 004E:4E5 SoundCommandFineTune
; @run 75:8CE5
; WHAT IT DOES: Command $F2: the channel's fine tune.
; HOW: SoundFineTune.
; Entry: X channel. Exit: A the fine tune, N and Z from it. X, Y, C and V kept.
SoundCommandFineTune:
    lda a:SoundCommandArgs
    sta a:SoundFineTune,x
    rts

; @routine 004E:4EC SoundCommandRepeatMark
; @run 75:8CEC
; WHAT IT DOES: Command $F6: marks the start of a section to be played twice.
; HOW: The place after the command into SoundRepeatMarkLo and Hi; SoundRepeatState 0.
; Entry: X channel. Exit: A the place's high byte, N and Z from it. X, Y, C and V kept.
SoundCommandRepeatMark:
    lda z:zSoundSequenceLo
    sta a:SoundRepeatMarkLo,x
    lda z:zSoundSequenceHi
    sta a:SoundRepeatMarkHi,x
    stz a:SoundRepeatState,x
    rts

; @routine 004E:4FA SoundCommandRepeat
; @run 75:8CFA
; WHAT IT DOES: Command $F7, met three times: the first goes on; the second keeps its place and
; goes back to the mark, playing the section again; the third goes back to where the second was,
; and the next pass goes on again.
; HOW: SoundRepeatState 0, 1 and 2; SoundRepeatReturnLo and Hi.
; Entry: X channel. Exit: X, Y, C and V kept. First, A 0, N and Z from the state 1. Second and
; third, A the high byte of where it goes, N and Z from it. Past 2, A the state less 2, N and Z
; from it.
SoundCommandRepeat:
    lda a:SoundRepeatState,x
    bne SoundCommandRepeat_at0
    inc a:SoundRepeatState,x
    bra SoundCommandRepeat_at1
SoundCommandRepeat_at0:
    dec a
    bne SoundCommandRepeat_at2
    inc a:SoundRepeatState,x
    lda z:zSoundSequenceLo
    sta a:SoundRepeatReturnLo,x
    lda z:zSoundSequenceHi
    sta a:SoundRepeatReturnHi,x
    lda a:SoundRepeatMarkLo,x
    sta z:zSoundSequenceLo
    lda a:SoundRepeatMarkHi,x
    sta z:zSoundSequenceHi
    bra SoundCommandRepeat_at1
SoundCommandRepeat_at2:
    dec a
    bne SoundCommandRepeat_at1
    dec a:SoundRepeatState,x
    lda a:SoundRepeatReturnLo,x
    sta z:zSoundSequenceLo
    lda a:SoundRepeatReturnHi,x
    sta z:zSoundSequenceHi
SoundCommandRepeat_at1:
    rts

; @routine 004E:55B SoundCommandEnvelope
; @run 75:8D5B
; WHAT IT DOES: Command $FA: the channel's envelope: its attack from a preset, its decay and
; release rates from the tens and units of the second argument, its sustain level.
; HOW: SoundAttackPresets by the first; SoundRatePresets by the second's tens and units.
; Entry: X channel. Exit: A the sustain level, N and Z from it, Y the units, C clear, V from the
; last subtract of ten if there was one. X kept.
SoundCommandEnvelope:
    lda a:SoundCommandArgs
    asl a
    tay
    lda a:SoundAttackPresets,y
    sta a:SoundAttackRate,x
    iny
    lda a:SoundAttackPresets,y
    sta a:SoundAttackLevel,x
    lda a:SoundCommandArgs+1
    cly
SoundCommandEnvelope_at1:
    cmp #$0A
    bcc SoundCommandEnvelope_at0
    sec
    sbc #$0A
    iny
    bra SoundCommandEnvelope_at1
SoundCommandEnvelope_at0:
    pha
    lda a:SoundRatePresets,y
    sta a:SoundDecayRate,x
    ply
    lda a:SoundRatePresets,y
    sta a:SoundReleaseRate,x
    lda a:SoundCommandArgs+2
    sta a:SoundSustainLevel,x
    rts

; @routine 004E:60C SoundCommandRamp
; @run 75:8E0C
; WHAT IT DOES: Command $FB: the ramp's step.
; HOW: SoundRampBy.
; Entry: X channel. Exit: A the step, N and Z from it. X, Y, C and V kept.
SoundCommandRamp:
    lda a:SoundCommandArgs
    sta a:SoundRampBy,x
    rts

; @routine 004E:613 SoundCommandNothing
; @run 75:8E13
; WHAT IT DOES: Commands $F3, $F4, $F5 and $FC: nothing.
; HOW: Returns.
; Entry: anything. Exit: all registers and flags kept.
SoundCommandNothing:
    rts

; @routine 0053:745 SoundCommandJump
; @run 76:B745
; WHAT IT DOES: Command $FD: the sequence goes on at the address given.
; HOW: The two arguments into zSoundSequenceLo and Hi.
; Entry: X channel. Exit: A the address's high byte, N and Z from it. X, Y, C and V kept.
SoundCommandJump:
    lda a:SoundCommandArgs
    sta z:zSoundSequenceLo
    lda a:SoundCommandArgs+1
    sta z:zSoundSequenceHi
    rts

; @routine 0053:750 SoundCommandCall
; @run 76:B750
; WHAT IT DOES: Command $FE: plays the sequence at the address given, then comes back after the
; command; one level only.
; HOW: The place after the command into SoundReturnLo and Hi, then the address.
; Entry: X channel. Exit: A the address's high byte, N and Z from it. X, Y, C and V kept.
SoundCommandCall:
    lda z:zSoundSequenceLo
    sta a:SoundReturnLo,x
    lda z:zSoundSequenceHi
    sta a:SoundReturnHi,x
    lda a:SoundCommandArgs
    sta z:zSoundSequenceLo
    lda a:SoundCommandArgs+1
    sta z:zSoundSequenceHi
    rts

; @routine 0053:765 SoundCommandReturn
; @run 76:B765
; WHAT IT DOES: Command $FF: back from a called sequence; with none to go back to, the end of the
; channel's sequence: it is silenced, the main volume off while the sound is paused, the channel's
; command cleared from 4 up, and from 6 up its priority too and the waveform given back to the
; PSG channel it shared.
; HOW: SoundReturnLo and Hi. The end drops the return into the command loop, so the loop's
; caller gets C set; SoundSilenceChannel; SoundLoadWave with the shared channel's waveform.
; Entry: X channel. Exit: back, A the high byte, N and Z from it, C clear. End, C set, and as the
; last call left them; the return address dropped from the stack.
SoundCommandReturn:
    lda a:SoundReturnLo,x
    ora a:SoundReturnHi,x
    bne SoundCommandReturn_at0
    pla
    pla
    jsr SoundSilenceChannel
    lda a:SoundPaused
    beq SoundCommandReturn_at1
    stz a:PsgMainVolume
SoundCommandReturn_at1:
    cpx #$04
    bcc SoundCommandReturn_at2
    stz a:SoundChannelCommand,x
SoundCommandReturn_at2:
    cpx #$06
    bcc SoundCommandReturn_at3
    stz a:SoundChannelPriority,x
    lda a:SoundChannelWave-2,x
    jsr SoundLoadWave
SoundCommandReturn_at3:
    sec
    rts
SoundCommandReturn_at0:
    lda a:SoundReturnLo,x
    sta z:zSoundSequenceLo
    lda a:SoundReturnHi,x
    sta z:zSoundSequenceHi
    stz a:SoundReturnLo,x
    stz a:SoundReturnHi,x
    clc
    rts

; @routine 004D:592 SoundChannelTick
; @run 75:8592
; WHAT IT DOES: A frame of one channel: its tempo is added to its phase, and a carry is a tick.
; Between ticks, a playing note gets its volume and its pitch between ticks. On a tick, the note
; counts down, and at its end the sequence is read on; the channel is switched off when its
; sequence ends. A rest only runs its tempo, slide and pan; a note runs its gate, the tempo,
; the glide's lookahead, the slide, the pan and the volume, and counts the tremolo's wait.
; HOW: SoundTempo, or $FF when SoundFullTempo's bit 0 is set, onto SoundTempoPhase; SoundChannel2550
; says a rest; SoundNoteLength; SoundReadSequence; the channel's bit in zSoundChannelBit taken
; out of zSoundChannelsOn.
; Entry: X channel. Exit: as the last call left them; a rest's tick or between ticks while
; resting, A SoundChannel2550 with the tempo's add in C and V; switched off, A the channel's bit,
; Z if no other bit is on, N and V the playing bits 7 and 6 before.
SoundChannelTick:
    lda a:SoundFullTempo
    and #$01
    bne SoundChannelTick_at0
    lda a:SoundTempo,x
    bra SoundChannelTick_at1
SoundChannelTick_at0:
    lda #$FF
SoundChannelTick_at1:
    clc
    adc a:SoundTempoPhase,x
    sta a:SoundTempoPhase,x
    bcc SoundChannelTick_at2
    jsr SoundLoadSequence
    dec a:SoundNoteLength,x
    bne SoundChannelTick_at3
    jsr SoundReadSequence
    bcc SoundChannelTick_at4
    lda z:zSoundChannelBit
    trb z:zSoundChannelsOn
    bra SoundChannelTick_at5
SoundChannelTick_at3:
    lda a:SoundChannel2550,x
    bne SoundChannelTick_at6
    jsr SoundGateStep
SoundChannelTick_at4:
    jsr SoundTempoSlideStep
    jsr SoundGlideLookahead
    jsr SoundSlideStep
    jsr SoundPanSlideStep
    jsr SoundVolumeTick
    bcc SoundChannelTick_at5
    inc a:SoundTremoloWait,x
    bra SoundChannelTick_at5
SoundChannelTick_at6:
    jsr SoundTempoSlideStep
    jsr SoundSlideStep
    jsr SoundPanSlideStep
    bra SoundChannelTick_at5
SoundChannelTick_at2:
    lda a:SoundChannel2550,x
    bne SoundChannelTick_at5
    jsr SoundVolumeTick
    jsr SoundPitchTickPlain
SoundChannelTick_at5:
    rts

; @routine 004D:7F8 SoundGateStep
; @run 75:87F8
; WHAT IT DOES: A frame of the note's gate: while it sounds, its timer counts down; when it is
; over, the ramp moves on while the volume less the slide is still at least the ramp, and the
; channel is silenced once it is not, or at once with no ramp. Then a frame of the pitch.
; HOW: SoundGateTimer; SoundRampBy onto SoundRampLo and Hi against SoundVolumeBase less
; SoundSlideHi; SoundSilenceChannel; runs on into SoundPitchTick.
; Entry: X channel. Exit: as SoundPitchTick.
SoundGateStep:
    lda a:SoundGateTimer,x
    beq SoundGateStep_at0
    dec a:SoundGateTimer,x
    bne SoundPitchTick
    lda a:SoundRampBy,x
    beq SoundGateStep_at1
SoundGateStep_at0:
    lda a:SoundVolumeBase,x
    sec
    sbc a:SoundSlideHi,x
    cmp a:SoundRampHi,x
    bcc SoundGateStep_at1
    lda a:SoundRampLo,x
    clc
    adc a:SoundRampBy,x
    sta a:SoundRampLo,x
    lda a:SoundRampHi,x
    adc #$00
    sta a:SoundRampHi,x
    bra SoundPitchTick
SoundGateStep_at1:
    jsr SoundSilenceChannel

; @routine 004E:129 SoundGlideLookahead
; @run 75:8929
; WHAT IT DOES: With no glide going, looks at the channel's next command: a $F3 is a glide to its
; note, after its delay, over its frames, from the pitch now; it is taken and the glide aimed.
; HOW: SoundPeekCommand; SoundSequenceFromWord takes it; SoundGlideDelay and SoundGlideFrames; the
; note with the transpose, less one with a negative fine tune; SoundGlideSetup with the fine tune.
; Entry: X channel. Exit: X kept. Gliding, A the frames, N and Z from them. Not $F3, as
; SoundPeekCommand, with A the command, N, Z and C from comparing it with $F3. Taken, as
; SoundGlideSetup, with A the fine tune, Y the note and C from adding the transpose where it
; keeps them.
SoundGlideLookahead:
    lda a:SoundGlideFrames,x
    bne SoundGlideLookahead_at0
    jsr SoundPeekCommand
    lda a:SoundCommandByte
    cmp #$F3
    bne SoundGlideLookahead_at0
    jsr SoundSequenceFromWord
    lda a:SoundCommandArgs
    sta a:SoundGlideDelay,x
    lda a:SoundCommandArgs+1
    sta a:SoundGlideFrames,x
    lda a:SoundCommandArgs+2
    clc
    adc a:SoundTranspose,x
    tay
    lda a:SoundFineTune,x
    bpl SoundGlideLookahead_at1
    dey
SoundGlideLookahead_at1:
    jsr SoundGlideSetup
SoundGlideLookahead_at0:
    rts

; @routine 0053:645 SoundReadSequence
; @run 76:B645
; WHAT IT DOES: Reads a channel's sequence on to its next note, rest or length: commands $E2 up
; run their handlers, $60 to $6F set the gate notes take and $70 to $7F their volume, and the
; channel's place is kept. Says whether the sequence ended.
; HOW: SoundReadCommand; the handlers through SoundCommandHandlers, each pushed and returned to,
; coming back to read again with X from zSoundChannel; $E0 SoundRest, $E1 SoundSetLength, a note
; SoundStartNote with SoundGateSet and SoundVolumeDefault; SoundSaveSequence.
; Entry: X channel. Exit: ended, C set, and as SoundCommandReturn left them on its way out.
; Otherwise A the place's high byte, N and Z from it, C clear, X the channel, Y and V as the last
; command left them.
SoundReadSequence:
    jsr SoundReadCommand
    lda a:SoundCommandByte
    cmp #$E2
    bcc SoundReadSequence_at0
    sbc #$E2
    asl a
    tax
    lda #$B6
    pha
    lda #$44
    pha
    lda a:SoundCommandHandlers+1,x
    pha
    lda a:SoundCommandHandlers,x
    pha
    ldx z:zSoundChannel
    rts
SoundReadSequence_at0:
    cmp #$E0
    bne SoundReadSequence_at1
    jsr SoundRest
    bra SoundReadSequence_at2
SoundReadSequence_at1:
    cmp #$E1
    bne SoundReadSequence_at4
    jsr SoundSetLength
    bra SoundReadSequence_at2
SoundReadSequence_at4:
    cmp #$70
    bcc SoundReadSequence_at5
    and #$0F
    sta a:SoundVolumeDefault,x
    bra SoundReadSequence
SoundReadSequence_at5:
    cmp #$60
    bcc SoundReadSequence_at6
    and #$0F
    sta a:SoundGateSet,x
    bra SoundReadSequence
SoundReadSequence_at6:
    stz a:SoundChannel2550,x
    lda a:SoundGateSet,x
    sta a:SoundCommandArgs+1
    lda a:SoundVolumeDefault,x
    sta a:SoundCommandArgs+2
    jsr SoundStartNote
SoundReadSequence_at2:
    jsr SoundSaveSequence
    clc
    rts

; @routine 004D:4EE SoundStartChannel
; @run 75:84EE
; WHAT IT DOES: Starts a channel from nothing: every effect off, its note due on the next tick, a
; length of 1, the slide, envelope, loops and returns cleared, the volume base $1F, the pan 8,
; the tempo 1 with its phase full so the first frame ticks.
; HOW: Each field by channel cleared or set; SoundSetPan.
; Entry: X channel. Exit: A 1, N and Z from it. X, Y, C and V kept.
SoundStartChannel:
    stz a:SoundModulationMaskLo,x
    stz a:SoundGateTimer,x
    stz a:SoundChannelDefaultArg,x
    stz a:SoundGate,x
    stz a:SoundLoopACount,x
    stz a:SoundLoopBCount,x
    stz a:SoundVolumeAddA,x
    stz a:SoundVolumeAddB,x
    stz a:SoundDetuneAFine,x
    stz a:SoundDetuneANote,x
    stz a:SoundDetuneBFine,x
    stz a:SoundDetuneBNote,x
    stz a:SoundSlideStepLo,x
    stz a:SoundPanLo,x
    stz a:SoundPanStepLo,x
    stz a:SoundGlideFramesSet,x
    stz a:SoundVibratoDepthLo,x
    stz a:SoundVibratoDepthMax,x
    stz a:SoundVibratoGrowHi,x
    stz a:SoundVibratoGrowLo,x
    stz a:SoundTremoloDepth,x
    stz a:SoundChannelWave,x
    stz a:SoundTranspose,x
    stz a:SoundFineTune,x
    stz a:SoundRampBy,x
    stz a:SoundChannel2550,x
    stz a:SoundModulationSpeed,x
    lda #$01
    sta a:SoundChannel2430,x
    sta a:SoundNoteLength,x
    stz a:SoundSlideHi,x
    stz a:SoundSlideLo,x
    stz a:SoundSlideStepLo,x
    stz a:SoundSlideStepHi,x
    stz a:SoundEnvelopeLevel,x
    stz a:SoundAttackRate,x
    stz a:SoundAttackLevel,x
    stz a:SoundSustainLevel,x
    stz a:SoundEnvelopePhase,x
    lda #$1F
    sta a:SoundVolumeBase,x
    lda #$08
    jsr SoundSetPan
    lda #$FF
    sta a:SoundTempoPhase,x
    stz a:SoundDecayRate,x
    stz a:SoundReleaseRate,x
    lda #$01
    sta a:SoundTempo,x
    stz a:SoundTempoSlideBy,x
    stz a:SoundChannel25B0,x
    stz a:SoundGateSet,x
    stz a:SoundChannel25E0,x
    stz a:SoundVolumeDefault,x
    stz a:SoundReturnLo,x
    stz a:SoundReturnHi,x
    rts

; @routine 0053:3B6 SoundStartMusic
; @run 76:B3B6
; @ends B4AF
; WHAT IT DOES: Starts a piece of music, commands $55 to $61, on the six PSG channels: each takes
; the command and its sequence from the piece's list and starts from nothing; no fade, the main
; volume full.
; HOW: zSoundChannelsOn gets the six channels' bits; SoundMusicTable by the command less $55 gives
; the list, through zSoundTableLo and Hi; SoundReadPair and SoundStartChannel for each.
; Entry: from SoundDriverCommand, SoundCommandIn the command. Exit, at the command's shared
; end: A $FF, N set, Z clear, X 6, Y 1, C set, V from the table address's add.
SoundStartMusic:
    lda #$3F
    tsb z:zSoundChannelsOn
    lda a:SoundCommandIn
    sec
    sbc #$55
    asl a
    clc
    adc #$FA
    sta z:zSoundTableLo
    cla
    adc #$B8
    sta z:zSoundTableHi
    lda (zSoundTableLo)
    sta z:zSoundWordLo
    ldy #$01
    lda (zSoundTableLo),y
    sta z:zSoundWordHi
    clx
SoundStartMusic_at0:
    lda a:SoundCommandIn
    sta a:SoundChannelCommand,x
    jsr SoundReadPair
    jsr SoundStartChannel
    inx
    cpx #$06
    bcc SoundStartMusic_at0
    stz a:SoundFading
    jsr SoundVolumeFull
    jmp SoundCommandEnd

; @routine 0053:4B5 SoundVoiceAddressBack
; @run 76:B4B5
; WHAT IT DOES: Moves a voice's ADPCM start back by 10, modulo the buffer's $4000: every voice of
; the second set, and the first set's above $0E.
; HOW: Bit 7 of SoundVoiceFlags is the set, SoundVoiceIndex the voice; $3FF6 added to zBiosBl and
; zBiosBh.
; Entry: nothing. Exit: not moved, A $0E, N, Z and C from comparing it with the voice, V clear.
; Moved, A the high byte, N, Z, C and V from its add. X and Y kept.
SoundVoiceAddressBack:
    lda a:SoundVoiceFlags
    and #$80
    bne SoundVoiceAddressBack_at0
    lda #$70
    sec
    sbc #$62
    cmp a:SoundVoiceIndex
    bcs SoundVoiceAddressBack_at1
SoundVoiceAddressBack_at0:
    lda #$F6
    clc
    adc z:zBiosBl
    sta z:zBiosBl
    lda #$3F
    adc z:zBiosBh
    sta z:zBiosBh
SoundVoiceAddressBack_at1:
    rts

; @routine 0053:3F0 SoundStartVoice
; @run 76:B3F0
; @ends B4AF
; WHAT IT DOES: Starts an ADPCM voice, commands $62 up: the second set from $72 on stages $0C
; up, else the first. One of lower priority than the voice playing is dropped unless one is
; waiting; otherwise the playing voice is stopped and this one played, or left waiting while the
; unit is still busy. The second character's voices start ten bytes back.
; HOW: ad_stat; SoundVoicePriority, cleared when nothing plays or waits; SoundVoicesFirst or
; SoundVoicesSecond by the command, six bytes a voice, through zSoundTableLo and Hi; ad_stop,
; ad_stat; the voice into zBiosBl to zBiosDh; SoundVoiceAddressBack for character 1;
; KernelAdpcmPlay. Bit 0 of SoundVoiceFlags marks a voice waiting.
; Entry: from SoundDriverCommand, SoundCommandIn the command. Exit, at the command's shared end,
; as the last of those calls left them, then: dropped, A 0, Z set, N and C clear; waiting, A the
; flags with bit 0, N and Z from them, C set; played, Z and C set, N clear.
SoundStartVoice:
    jsr BiosAdStat
    cmp #$00
    bne SoundStartVoice_at1
    lda a:SoundVoiceFlags
    and #$01
    bne SoundStartVoice_at1
    stz a:SoundVoicePriority
SoundStartVoice_at1:
    lda a:SoundCommandIn
    cmp #$72
    bmi SoundStartVoice_at2
    lda z:zStage
    and #$7F
    cmp #$0C
    bpl SoundStartVoice_at3
SoundStartVoice_at2:
    lda a:SoundVoiceFlags
    and #$7F
    sta a:SoundVoiceFlags
    lda a:SoundCommandIn
    sec
    sbc #$62
    sta a:SoundVoiceIndex
    ldy #$06
    jsr SoundMultiply
    lda z:zSoundWordLo
    clc
    adc #$14
    sta z:zSoundTableLo
    lda z:zSoundWordHi
    adc #$B9
    sta z:zSoundTableHi
    bra SoundStartVoice_at4
SoundStartVoice_at3:
    lda a:SoundVoiceFlags
    ora #$80
    sta a:SoundVoiceFlags
    lda a:SoundCommandIn
    sec
    sbc #$72
    sta a:SoundVoiceIndex
    ldy #$06
    jsr SoundMultiply
    lda z:zSoundWordLo
    clc
    adc #$44
    sta z:zSoundTableLo
    lda z:zSoundWordHi
    adc #$BC
    sta z:zSoundTableHi
SoundStartVoice_at4:
    lda (zSoundTableLo)
    cmp a:SoundVoicePriority
    bcs SoundStartVoice_at5
    lda a:SoundVoiceFlags
    beq SoundCommandEnd
    lda a:SoundVoicePriority
SoundStartVoice_at5:
    sta a:SoundVoicePriority
    jsr BiosAdStop
    jsr BiosAdStat
    cmp #$00
    beq SoundStartVoice_at6
SoundStartVoice_at9:
    lda a:SoundVoiceFlags
    ora #$01
    sta a:SoundVoiceFlags
    bra SoundCommandEnd
SoundStartVoice_at6:
    ldy #$01
    lda (zSoundTableLo),y
    sta z:zBiosBl
    iny
    lda (zSoundTableLo),y
    sta z:zBiosBh
    iny
    lda (zSoundTableLo),y
    sta z:zBiosAl
    iny
    lda (zSoundTableLo),y
    sta z:zBiosAh
    iny
    lda (zSoundTableLo),y
    sta z:zBiosDh
    stz z:zBiosDl
    lda z:zCharacter
    and #$01
    beq SoundStartVoice_at7
    jsr SoundVoiceAddressBack
SoundStartVoice_at7:
    jsr KernelAdpcmPlay
    cmp #$00
    beq SoundStartVoice_at8
    bra SoundStartVoice_at9
SoundStartVoice_at8:
    stz a:SoundVoiceFlags
SoundCommandEnd:
    ply
    plx
    stz a:SoundInCommand
    rts

; @routine 0053:284 SoundDriverCommand
; @run 76:B284
; WHAT IT DOES: Takes a command: $FF stops everything, $FE fades the music, $FD and $FC choose
; the balance tables, $55 to $61 start music and $62 up a voice; below $55 an effect starts, on
; channel 6 or 7 or, from $3D, on both, if its priority is at least what plays there. Nothing
; starts while music $60 plays.
; HOW: SoundInCommand set while it works; SoundCommandIn keeps the command; SoundEffectTable, three
; bytes an effect, through zSoundTableLo and Hi; SoundChannelPriority, SoundChannelCommand and
; zSoundChannelsOn by channel; SoundReadPair, SoundStartChannel, SoundVolumeFull.
; Entry: A command. Exit: X and Y kept, N and Z from X; A, C and V as the command's path left them.
SoundDriverCommand:
    phx
    phy
    ldx #$01
    stx a:SoundInCommand
    cmp #$FF
    bne SoundDriverCommand_at0
    jsr SoundDriverStopAll
    jmp SoundCommandEnd
SoundDriverCommand_at0:
    cmp #$FE
    bne SoundDriverCommand_at1
    jsr SoundStartFade
    jmp SoundCommandEnd
SoundDriverCommand_at1:
    cmp #$FD
    bne SoundDriverCommand_at2
    lda #$01
    sta z:zSoundAltBalance
    jmp SoundCommandEnd
SoundDriverCommand_at2:
    cmp #$FC
    bne SoundDriverCommand_at3
    stz z:zSoundAltBalance
    jmp SoundCommandEnd
SoundDriverCommand_at3:
    cmp #$61
    bne SoundDriverCommand_at4
    sta a:SoundCommandIn
    jmp SoundStartMusic
SoundDriverCommand_at4:
    pha
    lda a:SoundCommandIn
    cmp #$60
    bne SoundDriverCommand_at5
    lda z:zSoundChannelsOn
    and #$01
    beq SoundDriverCommand_at5
    pla
    jmp SoundCommandEnd
SoundDriverCommand_at5:
    pla
    sta a:SoundCommandIn
    cmp #$62
    bcs SoundDriverCommand_at6
    bra SoundDriverCommand_at7
SoundDriverCommand_at6:
    jmp SoundStartVoice
SoundDriverCommand_at7:
    cmp #$55
    bcs SoundDriverCommand_at8
    bra SoundDriverCommand_at9
SoundDriverCommand_at8:
    jmp SoundStartMusic
SoundDriverCommand_at9:
    cmp #$3D
    bcs SoundDriverCommand_at10
    bne SoundDriverCommand_at11
    jmp SoundCommandEnd
SoundDriverCommand_at11:
    lda a:SoundCommandIn
    tst #$FF,a:SoundChannelCommand+7
    beq SoundDriverCommand_at12
    cmp a:SoundChannelCommand+7
    beq SoundDriverCommand_at12
    cmp a:SoundChannelCommand+6
    beq SoundDriverCommand_at13
    ldx a:SoundChannel25F6
    cpx a:SoundChannel25F7
    bcs SoundDriverCommand_at12
SoundDriverCommand_at13:
    ldx #$06
    ldy #$40
    bra SoundDriverCommand_at14
SoundDriverCommand_at12:
    ldx #$07
    ldy #$80
SoundDriverCommand_at14:
    phy
    lda a:SoundCommandIn
    ldy #$03
    jsr SoundMultiply
    ply
    lda z:zSoundWordLo
    clc
    adc #$FB
    sta z:zSoundTableLo
    lda z:zSoundWordHi
    adc #$B7
    sta z:zSoundTableHi
    lda (zSoundTableLo)
    cmp a:SoundChannelPriority,x
    bcs SoundDriverCommand_at15
    jmp SoundCommandEnd
SoundDriverCommand_at15:
    sta a:SoundChannelPriority,x
    tya
    tsb z:zSoundChannelsOn
    lda a:SoundCommandIn
    sta a:SoundChannelCommand,x
    ldy #$01
    lda (zSoundTableLo),y
    sta z:zSoundWordLo
    iny
    lda (zSoundTableLo),y
    sta z:zSoundWordHi
    jsr SoundReadPair
    jsr SoundStartChannel
    jsr SoundVolumeFull
    jmp SoundCommandEnd
SoundDriverCommand_at10:
    ldx #$07
    lda a:SoundChannel25F6
    cmp a:SoundChannel25F7
    bcc SoundDriverCommand_at16
    dex
SoundDriverCommand_at16:
    phy
    lda a:SoundCommandIn
    ldy #$03
    jsr SoundMultiply
    ply
    lda z:zSoundWordLo
    clc
    adc #$FB
    sta z:zSoundTableLo
    lda z:zSoundWordHi
    adc #$B7
    sta z:zSoundTableHi
    lda (zSoundTableLo)
    cmp a:SoundChannelPriority,x
    bcs SoundDriverCommand_at17
    jmp SoundCommandEnd
SoundDriverCommand_at17:
    ldx #$06
    sta a:SoundChannelPriority,x
    sta a:SoundChannelPriority+1,x
    lda #$C0
    tsb z:zSoundChannelsOn
    lda a:SoundCommandIn
    sta a:SoundChannelCommand,x
    sta a:SoundChannelCommand+1,x
    ldy #$01
    lda (zSoundTableLo),y
    sta z:zSoundWordLo
    iny
    lda (zSoundTableLo),y
    sta z:zSoundWordHi
    jsr SoundReadPair
    jsr SoundStartChannel
    inx
    jsr SoundReadPair
    jsr SoundStartChannel
    jsr SoundVolumeFull
    jmp SoundCommandEnd

; @routine 0053:4D4 SoundCheckPause
; @run 76:B4D4
; WHAT IT DOES: Follows the game's pause: on pausing, the PSG silenced, ADPCM stopped and effect
; $53 played; on going on, the two effect channels let go and PSG channels 4 and 5 given their
; music's waveforms back, the main volume full.
; HOW: Bit 0 of zPaused against SoundPaused; SoundSilenceAll, ad_stop, SoundDriverCommand; or
; SoundLoadWave on 4 and 5 and SoundVolumeFull.
; Entry: nothing. Exit: unchanged, A the bit, Z and C set, N clear. Paused, as SoundDriverCommand.
; Going on, as SoundVolumeFull after SoundLoadWave on channel 5, X 5. Y kept.
SoundCheckPause:
    lda z:zPaused
    and #$01
    cmp a:SoundPaused
    beq SoundCheckPause_at0
    sta a:SoundPaused
    cmp #$00
    beq SoundCheckPause_at1
    jsr SoundSilenceAll
    jsr BiosAdStop
    lda #$53
    jsr SoundDriverCommand
SoundCheckPause_at0:
    rts
SoundCheckPause_at1:
    ldx #$04
    stx a:PsgSelect
    stx z:zSoundChannel
    stz a:SoundChannelCommand+6
    lda #$40
    trb z:zSoundChannelsOn
    stz a:SoundChannel25F6
    lda a:SoundChannelWave,x
    jsr SoundLoadWave
    ldx #$05
    stx a:PsgSelect
    stx z:zSoundChannel
    stz a:SoundChannelCommand+7
    lda #$80
    trb z:zSoundChannelsOn
    stz a:SoundChannel25F7
    lda a:SoundChannelWave,x
    jsr SoundLoadWave
    jmp SoundVolumeFull

; @routine 0053:521 SoundDriverTick
; @run 76:B521
; WHAT IT DOES: One frame of the driver, unless a command is going in: the pause followed; while
; paused, ADPCM stopped and only the two effect channels run; otherwise a voice waiting played
; once the unit is free, the music's fade a step on, and every playing channel's frame. With
; nothing playing, the main volume off.
; HOW: SoundInCommand set while it works; SoundCheckPause; ad_stat and ad_stop; the voice waiting
; from its set's table, SoundVoiceAddressBack and KernelAdpcmPlay; SoundFading and SoundFadeStep,
; $14 its end; zSoundChannelBit walks zSoundChannelsOn; channels 6 and 7 select PSG channels 4 and
; 5; SoundChannelTick.
; Entry: frame interrupt. Exit: A, X and Y kept, N and Z from A; C set, and C and V kept if a
; command was going in; otherwise V as the last call left it.
SoundDriverTick:
    pha
    phx
    phy
    lda a:SoundInCommand
    beq SoundDriverTick_at0
    jmp SoundDriverTick_at20
SoundDriverTick_at0:
    inc a:SoundInCommand
    jsr SoundCheckPause
    lda a:SoundPaused
    beq SoundDriverTick_at1
    jsr BiosAdStat
    cmp #$00
    beq SoundDriverTick_at2
    jsr BiosAdStop
SoundDriverTick_at2:
    ldx #$06
    lda #$40
    sta z:zSoundChannelBit
    jmp SoundDriverTick_at3
SoundDriverTick_at1:
    jsr BiosAdStat
    cmp #$00
    bne SoundDriverTick_at4
    lda a:SoundVoiceFlags
    and #$01
    bne SoundDriverTick_at5
SoundDriverTick_at4:
    jmp SoundDriverTick_at6
SoundDriverTick_at5:
    jsr BiosAdStop
    jsr BiosAdStat
    cmp #$00
    beq SoundDriverTick_at7
SoundDriverTick_at12:
    jmp SoundDriverTick_at6
SoundDriverTick_at7:
    lda a:SoundVoiceFlags
    and #$80
    bne SoundDriverTick_at8
    lda a:SoundVoiceIndex
    ldy #$06
    jsr SoundMultiply
    lda z:zSoundWordLo
    clc
    adc #$14
    sta z:zSoundTableLo
    lda z:zSoundWordHi
    adc #$B9
    sta z:zSoundTableHi
    bra SoundDriverTick_at9
SoundDriverTick_at8:
    lda a:SoundVoiceIndex
    ldy #$06
    jsr SoundMultiply
    lda z:zSoundWordLo
    clc
    adc #$44
    sta z:zSoundTableLo
    lda z:zSoundWordHi
    adc #$BC
    sta z:zSoundTableHi
SoundDriverTick_at9:
    ldy #$01
    lda (zSoundTableLo),y
    sta z:zBiosBl
    iny
    lda (zSoundTableLo),y
    sta z:zBiosBh
    iny
    lda (zSoundTableLo),y
    sta z:zBiosAl
    iny
    lda (zSoundTableLo),y
    sta z:zBiosAh
    iny
    lda (zSoundTableLo),y
    sta z:zBiosDh
    stz z:zBiosDl
    lda z:zCharacter
    and #$01
    beq SoundDriverTick_at10
    jsr SoundVoiceAddressBack
SoundDriverTick_at10:
    jsr KernelAdpcmPlay
    cmp #$00
    beq SoundDriverTick_at11
    jmp SoundDriverTick_at12
SoundDriverTick_at11:
    stz a:SoundVoiceFlags
SoundDriverTick_at6:
    lda a:SoundFading
    beq SoundDriverTick_at13
    dec a:SoundFadeStep
    bne SoundDriverTick_at13
    lda #$10
    sta a:SoundFadeStep
    inc a:SoundFading
    lda #$14
    cmp a:SoundFading
    bne SoundDriverTick_at13
    ldx #$05
    stz a:SoundFading
SoundDriverTick_at14:
    stx z:zSoundChannel
    stx a:PsgSelect
    jsr SoundSilenceChannel
    dex
    bpl SoundDriverTick_at14
    lda #$3F
    trb z:zSoundChannelsOn
SoundDriverTick_at13:
    clx
    stx z:zSoundChannelBit
    sec
SoundDriverTick_at15:
    rol z:zSoundChannelBit
SoundDriverTick_at3:
    stx z:zSoundChannel
    lda z:zSoundChannelBit
    and z:zSoundChannelsOn
    beq SoundDriverTick_at18
    cpx #$06
    bcc SoundDriverTick_at16
    dex
    dex
    stx a:PsgSelect
    inx
    inx
    bra SoundDriverTick_at17
SoundDriverTick_at16:
    stx a:PsgSelect
SoundDriverTick_at17:
    jsr SoundChannelTick
SoundDriverTick_at18:
    inx
    cpx #$08
    bcc SoundDriverTick_at15
    lda z:zSoundChannelsOn
    bne SoundDriverTick_at19
    stz a:PsgMainVolume
SoundDriverTick_at19:
    stz a:SoundInCommand
SoundDriverTick_at20:
    ply
    plx
    pla
    rts
