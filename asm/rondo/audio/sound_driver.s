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

; @routine 004E:310 SoundSetPan
; @run 75:8B10
; WHAT IT DOES: Sets a channel's pan outright, as set and as it plays, with no fraction.
; HOW: SoundPanSet and SoundPan, SoundPanLo cleared.
; Entry: A pan, X channel. Exit: all registers and flags kept.
SoundSetPan:
    sta a:SoundPanSet,x
    sta a:SoundPan,x
    stz a:SoundPanLo,x
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
    lda a:$87BC,y
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
; Entry: nothing. Exit: A, X, Y, C and V kept; N and Z from X.
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
; growing and no counting of its wait; the pitch; the glide's step shifted down by the channel's
; shift and added, or taken off when it goes down; the modulation; then the PSG frequency.
; HOW: The shift is a bit in SoundGlideShift: one place for bit 7, two for bit 6, and so on.
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
    lda a:SoundGlideShift,x
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
    lda a:SoundGlideShift,x
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
