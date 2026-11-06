; @card 00:E8E3 cd_reset
; WHAT IT DOES: Resets the CD unit.
; HOW: Pulses its reset line, with pauses the C does not state.
; Entry: nothing. Exit: A what the port was left holding, X 0, Y 0, N 0, Z 1, V bit 6 of what
; the port first held. C kept.
cd_reset:
    lda #$02
    tsb a:CdResetPort
    ldy #$0A
cd_reset_at0:
    ldx #$3B
cd_reset_at1:
    dex
    bne cd_reset_at1
    dey
    bne cd_reset_at0
    lda a:CdResetPort
    and #$FD
    sta a:CdResetPort
    ldx #$77
cd_reset_at2:
    dex
    bne cd_reset_at2
    rts

; @card 00:E900 cd_send_command
; WHAT IT DOES: Sends the command in CardCdCommand to the drive, and unless told not to,
; waits for it to answer.
; HOW: Waits for a transfer in progress, selects the drive, hands over bytes while the bus
; is in its command phase. Then waits up to $FE frames for the request bit.
; Entry: the command built. Exit, see cd_send_exit_shim: A, N, Z, C and V as the path left
; them; X the bytes sent, or the first frame count if it waited. Y kept.
cd_send_command:
    lda a:CardCdBusy
    bne cd_send_command
    stz a:CardCdTimedOut
cd_send_command_at0:
    lda #$81
    sta a:CdData
    tst #$80,a:CdBusStatus
    beq cd_send_command_at4
    lda #$60
    trb a:CdIrqControl
    sta a:CdBusStatus
    ldx #$95
cd_send_command_at1:
    lda #$ED
cd_send_command_at2:
    dec a
    bne cd_send_command_at2
    nop
    nop
    nop
    dex
    bne cd_send_command_at1
    lda #$FF
    sta a:CdData
    tst #$40,a:CdBusStatus
    beq cd_send_command_at0
cd_send_command_at3:
    jsr cd_ack_pulse
cd_send_command_at8:
    tst #$40,a:CdBusStatus
    bne cd_send_command_at3
    tst #$80,a:CdBusStatus
    bne cd_send_command_at8
    bra cd_send_command_at0
cd_send_command_at4:
    sta a:CdBusStatus
    clx
cd_send_command_at5:
    lda a:CdBusStatus
    and #$40
    bne cd_send_command_at7
    lda #$5A
cd_send_command_at6:
    dec a
    bne cd_send_command_at6
    nop
    dex
    bne cd_send_command_at5
    bra cd_send_command_at0
cd_send_command_at7:
    stz a:CardBusPhase
    clx
cd_send_command_at9:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$D0
    beq cd_send_command_at10
    and #$B8
    cmp #$98
    beq cd_send_command_at11
    cmp #$88
    beq cd_send_command_at11
    cmp #$80
    beq cd_send_command_at11
    bra cd_send_command_at9
cd_send_command_at10:
    lda a:CardCdCommand,x
    inx
    sta a:CdData
    nop
    nop
    nop
    nop
    jsr cd_ack_pulse
    bra cd_send_command_at9
cd_send_command_at11:
    lda a:CardCdBusy
    ora a:CardCdNoWait
    bne cd_send_command_at14
    lda a:CardCdCommand
    cmp #$D9
    bne cd_send_command_at12
    lda a:CardCdCommand+1
    cmp #$01
    beq cd_send_command_at14
    cmp #$02
    beq cd_send_command_at14
cd_send_command_at12:
    ldx a:CardFrameTick
cd_send_command_at13:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    bit #$40
    bne cd_send_command_at14
    txa
    sec
    sbc a:CardFrameTick
    eor #$FF
    inc a
    cmp #$FE
    bcc cd_send_command_at13
    lda #$01
    sta a:CardCdTimedOut
cd_send_command_at14:
    rts

; @card 00:E9C5 cd_finish_command
; WHAT IT DOES: Finishes a command: if the drive timed out resets it, else takes its status.
; HOW: CardCdTimedOut. A zero status clears the sense code.
; Entry: nothing. Exit, timed out: A 6, N and Z from it, X 0, Y 0, V as cd_reset. Exit: as
; cd_read_status.
cd_finish_command:
    lda a:CardCdTimedOut
    beq cd_finish_command_at0
    stz a:CardCdTimedOut
    jsr cd_reset
    lda #$06
    rts
cd_finish_command_at0:
    jsr cd_read_status
    bne cd_finish_command_at1
    stz a:CardCdSense
cd_finish_command_at1:
    rts

; @card 00:E9DC cd_read_status
; WHAT IT DOES: Takes the drive's status byte and its message at the end of a command.
; HOW: Waits for the status phase, takes the byte; then the message phase, or status again,
; takes that; waits for the drive to leave the bus.
; Entry: nothing. Exit: A the status, N and Z from it, C 1, V bit 6 of the bus's last read. X
; and Y kept.
cd_read_status:
    stz a:CardCdStatus
cd_read_status_at0:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$D8
    bne cd_read_status_at0
    lda a:CdData
    sta a:CardCdStatus
    bsr cd_ack_on
cd_read_status_at1:
    tst #$40,a:CdBusStatus
    bne cd_read_status_at1
    bsr cd_ack_off
cd_read_status_at2:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$F8
    beq cd_read_status_at3
    cmp #$D8
    bne cd_read_status_at2
    bra cd_read_status_at0
cd_read_status_at3:
    lda a:CdData
    sta a:CardBusPhase
    bsr cd_ack_on
cd_read_status_at4:
    tst #$40,a:CdBusStatus
    bne cd_read_status_at4
    bsr cd_ack_off
cd_read_status_at5:
    tst #$80,a:CdBusStatus
    bne cd_read_status_at5
    lda a:CardCdStatus
    rts

; @card 00:EA27 cd_ack_pulse
; WHAT IT DOES: Acknowledges a byte on the bus.
; HOW: Raises the acknowledge, waits for the drive to drop its request, lowers it.
; Entry: nothing. Exit: as cd_ack_off.
cd_ack_pulse:
    bsr cd_ack_on
cd_ack_pulse_at0:
    tst #$40,a:CdBusStatus
    bne cd_ack_pulse_at0

; @card 00:EA2F cd_ack_off
; WHAT IT DOES: Lowers the bus's acknowledge.
; HOW: Clears its bit in the control port, and gives what the port held.
; Entry: nothing. Exit: A $80, Z if nothing else was set, N and V bits 7 and 6 of what the
; port held. X, Y, C kept.
cd_ack_off:
    lda #$80
    trb a:CdIrqControl
    rts

; @card 00:EA35 cd_ack_on
; WHAT IT DOES: Raises the bus's acknowledge.
; HOW: Sets its bit in the control port, and gives what the port held.
; Entry: nothing. Exit: A $80, Z clear, N and V bits 7 and 6 of what the port held. X, Y, C
; kept.
cd_ack_on:
    lda #$80
    tsb a:CdIrqControl
    rts
