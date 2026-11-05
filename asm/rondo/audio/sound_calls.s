; @routine 0043:33F SoundStopAll
; @run 69:433F
; WHAT IT DOES: Stops every sound channel.
; HOW: Maps the driver's banks, calls its stop entry, restores the banks from
; the stack.
; Entry: nothing. Exit: A the bank restored to MPR4, X and Y kept, N and Z from
; X, C and V as the driver left them.
SoundStopAll:
    phx
    phy
    tma #$10
    pha
    tma #$20
    pha
    tma #$40
    pha
    lda #$0D
    clc
    adc z:zRamBaseBank
    tam #$10
    inc a
    tam #$20
    inc a
    tam #$40
    jsr SoundDrvStopAll
    pla
    tam #$40
    pla
    tam #$20
    pla
    tam #$10
    ply
    plx
    rts

; @routine 0043:366 SoundFrameTick
; @run 69:4366
; WHAT IT DOES: Runs one frame of the sound driver, unless a command is going
; in from the main program at this moment.
; HOW: zSoundBusy nonzero returns. Otherwise sets it, maps the driver's banks,
; calls its tick, restores the banks and clears it.
; Entry: frame interrupt. Exit: busy, A zSoundBusy, N and Z from it. Else A the
; bank restored to MPR4, N and Z from it; X, Y, C and V as the driver left them.
SoundFrameTick:
    lda z:zSoundBusy
    bne SoundFrameTick_at0
    lda #$80
    sta z:zSoundBusy
    tma #$10
    pha
    tma #$20
    pha
    tma #$40
    pha
    lda #$0D
    clc
    adc z:zRamBaseBank
    tam #$10
    inc a
    tam #$20
    inc a
    tam #$40
    jsr SoundDrvTick
    pla
    tam #$40
    pla
    tam #$20
    pla
    tam #$10
    stz z:zSoundBusy
SoundFrameTick_at0:
    rts

; @routine 0043:393 SoundCommand
; @run 69:4393
; WHAT IT DOES: Gives the sound driver a command: a sound to play, or $FF stop,
; $FE fade, $FD and $FC to set and clear a flag of the driver's. Dropped if
; another command is going in.
; HOW: Bit 7 of zSoundBusy set returns. Otherwise sets it, which also holds off
; the frame tick; keeps the three MPRs in RAM, maps the driver and calls its
; command entry with the command in A.
; Entry: A command. Exit, dropped: A kept, N and V bits 7 and 6 of zSoundBusy,
; Z from A and it. Exit, taken: A the MPR4 bank restored, N and Z from it; X, Y,
; C and V as the driver left them.
SoundCommand:
    bit z:zSoundBusy
    bmi SoundCommand_at0
    pha
    lda #$80
    sta z:zSoundBusy
    tma #$10
    sta a:SavedMprA
    tma #$20
    sta a:SavedMprB
    tma #$40
    sta a:SavedMprC
    lda #$0D
    clc
    adc z:zRamBaseBank
    tam #$10
    inc a
    tam #$20
    inc a
    tam #$40
    pla
    jsr SoundDrvCommand
    lda a:SavedMprC
    tam #$40
    lda a:SavedMprB
    tam #$20
    lda a:SavedMprA
    tam #$10
    stz z:zSoundBusy
SoundCommand_at0:
    rts
