; @card 00:F793 ad_trans_select
; WHAT IT DOES: Selects the drive for ad_trans_step's read, once.
; HOW: Puts the drive's ID on the bus; if the bus is free, raises select and looks 256 times,
; with a pause the C does not state, for the drive's request. Seeing it moves the state to 2.
; WAITS: no: 256 looks at most.
; Entry: nothing. Exit: A 2 selected, 1 not; N 0, Z 0; V bit 6 of the bus as the busy test
; read it; X the tries left when selected, 0 when not, kept when the bus was busy. Y, C kept.
ad_trans_select:
    lda #$81
    sta a:CdData
    tst #$80,a:CdBusStatus
    bne ad_trans_select_at2
    sta a:CdBusStatus
    clx
ad_trans_select_at0:
    lda a:CdBusStatus
    and #$40
    bne ad_trans_select_at3
    lda #$5A
ad_trans_select_at1:
    dec a
    bne ad_trans_select_at1
    nop
    dex
    bne ad_trans_select_at0
ad_trans_select_at2:
    lda #$01
    rts
ad_trans_select_at3:
    lda #$02
    sta a:CardAdTransStep
    rts

; @card 00:F7BB ad_trans_command
; WHAT IT DOES: Moves ad_trans_step's command on by one bus phase.
; HOW: In the command phase, sends the next byte of CardCdCommand and acknowledges it. A data
; phase moves the state to 3, a status phase to 4; any other phase does nothing.
; WAITS: through cd_ack_pulse, on the drive's request bit, in the command phase.
; Entry: nothing. Exit: a byte sent, A $80, X its index, C set, Z, N and V as cd_ack_off leaves
; them. A data or status phase: A the new state, N 0, Z 0, C set. Another: A the phase and $B8,
; N, Z and C from comparing it with $88. V and X kept on both. Y kept.
ad_trans_command:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$D0
    beq ad_trans_command_at0
    and #$B8
    cmp #$98
    beq ad_trans_command_at2
    cmp #$88
    beq ad_trans_command_at1
    rts
ad_trans_command_at0:
    ldx a:CardAdTransSent
    inc a:CardAdTransSent
    lda a:CardCdCommand,x
    sta a:CdData
    nop
    nop
    nop
    nop
    jmp cd_ack_pulse
ad_trans_command_at1:
    lda #$03
    bra ad_trans_command_at3
ad_trans_command_at2:
    lda #$04
ad_trans_command_at3:
    sta a:CardAdTransStep
    rts

; @card 00:F7EF ad_trans_start_dma
; WHAT IT DOES: Starts the drive's data flowing into ADPCM memory, at the address given.
; HOW: CardAdTransAddress to the ADPCM unit as its write address, the transfer from the CD
; on, and the state to 4.
; WAITS: no.
; Entry: nothing. Exit: A 4, N 0, Z 0, V bit 6 of the ADPCM control port as the strobe found
; it. X, Y, C kept.
ad_trans_start_dma:
    lda a:CardAdTransAddress
    sta a:AdpcmAddressLo
    lda a:CardAdTransAddress+1
    sta a:AdpcmAddressHi
    jsr ad_latch_write_address
    lda #$02
    sta a:AdpcmDma
    lda #$04
    sta a:CardAdTransStep
    rts

; @card 00:F809 ad_trans_finish
; WHAT IT DOES: Once the drive is in its status phase, ends ad_trans_step's read.
; HOW: Stops the transfer, takes the status byte into CardCdStatus and the message byte into
; CardBusPhase, each with its handshake, waits for the bus to go free and sets the state to 0.
; WAITS: on the bus, with the stack as the caller's and the two bytes of its own frame.
; Entry: nothing. Exit: done, A the status, N and Z from it, C set, V bit 6 of the bus's last
; read. Not yet, A the phase, N from it less $D8, Z 0, C clear, V kept. X, Y kept.
ad_trans_finish:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$D8
    bne ad_trans_finish_at4
    stz a:AdpcmDma
    lda a:CdData
    sta a:CardCdStatus
    jsr cd_ack_on
ad_trans_finish_at0:
    tst #$40,a:CdBusStatus
    bne ad_trans_finish_at0
    jsr cd_ack_off
ad_trans_finish_at1:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$F8
    bne ad_trans_finish_at1
    lda a:CdData
    sta a:CardBusPhase
    jsr cd_ack_on
ad_trans_finish_at2:
    tst #$40,a:CdBusStatus
    bne ad_trans_finish_at2
    jsr cd_ack_off
ad_trans_finish_at3:
    tst #$80,a:CdBusStatus
    bne ad_trans_finish_at3
    stz a:CardAdTransStep
    sec
    lda a:CardCdStatus
    rts
ad_trans_finish_at4:
    clc
    rts

; @card 00:F76C ad_trans_step
; WHAT IT DOES: Takes one step of the read from the disc into ADPCM memory that the entry at
; $F730 sets up (command $08, sectors and count from _CL-_DL and _AL, ADPCM address _BX),
; without waiting on the drive: the caller calls it until it returns with C set.
; HOW: Dispatches on CardAdTransStep: 1 ad_trans_select, 2 ad_trans_command, 3
; ad_trans_start_dma, 4 ad_trans_finish. 0 and anything above 4 do nothing.
; WAITS: through ad_trans_command and ad_trans_finish.
; Entry: nothing. Exit: state 4 as ad_trans_finish. Otherwise C clear, and A, X, N, Z and V as
; the step left them; state 0, A 0, N 0, Z 1; above 4, A the state, N and Z from it less 4.
; Y kept.
ad_trans_step:
    lda a:CardAdTransStep
    beq ad_trans_step_at3
    cmp #$01
    bne ad_trans_step_at0
    bsr ad_trans_select
    bra ad_trans_step_at3
ad_trans_step_at0:
    cmp #$02
    bne ad_trans_step_at1
    bsr ad_trans_command
    bra ad_trans_step_at3
ad_trans_step_at1:
    cmp #$03
    bne ad_trans_step_at2
    bsr ad_trans_start_dma
    bra ad_trans_step_at3
ad_trans_step_at2:
    cmp #$04
    bne ad_trans_step_at3
    jsr ad_trans_finish
    rts
ad_trans_step_at3:
    clc
    rts
