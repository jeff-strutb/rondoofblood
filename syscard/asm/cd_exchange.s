; @card 00:EB5E cd_drop_extra_data
; WHAT IT DOES: Throws away what the drive still has to send, until it moves to its status.
; HOW: Reads whole blocks through the auto port and drops them.
; Entry: nothing. Exit: A $D8, N 1, Z 0, C 1; X 0 and Y 0 if a block was read. V kept. Gives
; whether a block was read.
cd_drop_extra_data:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$C8
    beq cd_drop_extra_data_at0
    cmp #$D8
    beq cd_drop_extra_data_at3
    bra cd_drop_extra_data
cd_drop_extra_data_at0:
    ldx #$05
cd_drop_extra_data_at1:
    dex
    bne cd_drop_extra_data_at1
    cly
    ldx #$08
cd_drop_extra_data_at2:
    lda a:CdDataAuto
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    iny
    bne cd_drop_extra_data_at2
    dex
    bne cd_drop_extra_data_at2
    bra cd_drop_extra_data
cd_drop_extra_data_at3:
    lda a:CardBusPhase
    rts

; @card 00:EA3B cd_read_data_in
; WHAT IT DOES: Takes bytes from the drive to where _BX points, up to the count in _AX.
; HOW: A byte and an acknowledge for each while the bus is in its data phase; if the drive
; had more than was asked for, cd_drop_extra_data.
; Entry: _AX count, _BX pointer. Exit: A the phase's answer bits, N, Z, C from comparing it
; with $88, or as cd_drop_extra_data. V from the last acknowledge, else kept.
cd_read_data_in:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$C8
    beq cd_read_data_in_at0
    cmp #$D8
    beq cd_read_data_in_at3
    bra cd_read_data_in
cd_read_data_in_at0:
    lda a:CdData
    sta (CardBl)
    bsr cd_ack_pulse
    inc z:CardBl
    bne cd_read_data_in_at1
    inc z:CardBh
cd_read_data_in_at1:
    lda z:CardAl
    bne cd_read_data_in_at2
    dec z:CardAh
cd_read_data_in_at2:
    dec a
    sta z:CardAl
    ora z:CardAh
    bne cd_read_data_in
cd_read_data_in_at3:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    and #$B8
    cmp #$88
    bne cd_read_data_in_at4
    jmp cd_drop_extra_data
cd_read_data_in_at4:
    rts

; @card 00:E20B cd_retry_after_error
; WHAT IT DOES: Decides whether a failed command is worth trying again.
; HOW: Only error 2 asks the drive why: keeps the command, requests sense, and gives up on the
; sense codes that retrying cannot mend. Sense code 4 marks the drive not ready.
; Entry: A error. Exit: C set to give up, A and N, Z as its last compare left them; for error
; 2, Y $FF unless the exchange changed it, and X and V as the exchange left them.
cd_retry_after_error:
    cmp #$06
    beq cd_retry_after_error_at2
    cmp #$02
    bne cd_retry_after_error_at2
    ldy #$09
cd_retry_after_error_at0:
    lda a:CardCdCommand,y
    sta a:CardCdFailedCommand,y
    dey
    bpl cd_retry_after_error_at0
    jsr cd_request_sense
    lda a:CardCdSense
    beq cd_retry_after_error_at2
    cmp #$04
    bne cd_retry_after_error_at1
    sta a:CardCdNotReady
    bra cd_retry_after_error_at2
cd_retry_after_error_at1:
    cmp #$0B
    beq cd_retry_after_error_at3
    cmp #$0D
    beq cd_retry_after_error_at3
    cmp #$11
    beq cd_retry_after_error_at2
    cmp #$15
    beq cd_retry_after_error_at2
    cmp #$16
    beq cd_retry_after_error_at2
    cmp #$1C
    beq cd_retry_after_error_at3
    cmp #$1D
    beq cd_retry_after_error_at3
    cmp #$20
    beq cd_retry_after_error_at3
    cmp #$21
    beq cd_retry_after_error_at3
    cmp #$22
    beq cd_retry_after_error_at3
    cmp #$25
    beq cd_retry_after_error_at3
    cmp #$2A
    beq cd_retry_after_error_at3
    cmp #$2C
    beq cd_retry_after_error_at3
cd_retry_after_error_at2:
    clc
    rts
cd_retry_after_error_at3:
    sec
    rts

; @card 00:F053 cd_request_sense
; WHAT IT DOES: Asks the drive why the last command failed, and keeps its answer.
; HOW: Command 3 for ten bytes into CardCdResult; on success bytes 2 and 9 go to the sense key
; and sense code.
; Entry: nothing. Exit: A the command's status, N and Z from it, C 1; V, X and Y as the
; exchange left them. Clobbers _AX, _BX if the drive sent data.
cd_request_sense:
    jsr clear_cd_result
    jsr clear_cd_command
    lda #$03
    sta a:CardCdCommand
    lda #$0A
    sta a:CardCdCommand+4
    jsr cd_send_command
    cmp #$C8
    bne cd_request_sense_at0
    lda #$56
    sta z:CardBl
    lda #$22
    sta z:CardBh
    lda #$0A
    sta z:CardAl
    stz z:CardAh
    jsr cd_read_data_in
cd_request_sense_at0:
    jsr cd_finish_command
    pha
    cmp #$00
    bne cd_request_sense_at1
    lda a:CardCdResult+2
    sta a:CardCdSenseKey
    lda a:CardCdResult+9
    sta a:CardCdSense
cd_request_sense_at1:
    pla
    rts

; @card 00:F091 cd_test_unit
; WHAT IT DOES: Asks whether the drive is ready.
; HOW: A test command; on failure the retry decision, whose answer does not repeat it.
; Entry: nothing. Exit, well: A 0, N 0, Z 1, C clear. Exit, failed: C set, A and N, Z as
; cd_retry_after_error leaves them.
cd_test_unit:
    bsr clear_cd_command
    lda #$00
    sta a:CardCdCommand
    jsr cd_send_command
    jsr cd_finish_command
    cmp #$00
    beq cd_test_unit_at0
    jsr cd_retry_after_error
    sec
    rts
cd_test_unit_at0:
    clc
    rts

; @card 00:F347 cd_stat
; WHAT IT DOES: Says whether the drive is busy; or, for any other request, tests the unit.
; HOW: Zero reads the bus's busy bit; anything else is cd_test_unit.
; Entry: A what. Exit, zero: A the bit, N and Z from it, C 1. Exit: as cd_test_unit.
cd_stat:
    cmp #$00
    bne cd_stat_at0
    lda a:CdBusStatus
    and #$80
    rts
cd_stat_at0:
    jmp cd_test_unit
