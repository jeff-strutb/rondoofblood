; @card 00:EF34 cd_search
; WHAT IT DOES: Seeks to a place on the disc; with bit 0 of _BH, returns at once and lets an
; interrupt report the end.
; HOW: Command $D8. Bits 6-7 of _BH say what the address is.
; Entry: _AX, _BL the place, _BH the mode. Exit, on an interrupt: A 0, Z clear, N and V from
; the control port, C as the send left it. Exit: as cd_subq, with Y the command offset unless
; a step changed it.
cd_search:
    jsr save_card_args
cd_search_at0:
    jsr clear_cd_command
    lda #$D8
    sta a:CardCdCommand
    lda z:CardBh
    and #$C0
    sta a:CardCdCommand+9
    bne cd_search_at1
    clx
    ldy #$03
    jsr copy_args_to_command
    bra cd_search_at3
cd_search_at1:
    cmp #$80
    bne cd_search_at2
    stz z:CardAh
    stz z:CardBl
cd_search_at2:
    clx
    ldy #$02
    jsr copy_args_to_command
cd_search_at3:
    lda z:CardBh
    tax
    and #$01
    sta a:CardCdNoWait
    txa
    and #$02
    lsr a
    sta a:CardCdCommand+1
    jsr cd_send_command
    lda a:CardCdNoWait
    beq cd_search_at4
    stz a:CardCdBusy
    lda #$20
    tsb a:CdIrqControl
    stz a:CardCdNoWait
    cla
    rts
cd_search_at4:
    jsr cd_finish_command
    cmp #$00
    beq cd_search_at5
    jsr cd_retry_after_error
    bcs cd_search_at5
    jsr restore_card_args
    bra cd_search_at0
cd_search_at5:
    rts

; @card 00:EF94 cd_pause
; WHAT IT DOES: Pauses the drive if it is playing.
; HOW: cd_subq; a first byte not zero says nothing plays. Otherwise command $DA, and on
; failure the retry decision, whose try starts again from cd_subq.
; Entry: nothing. Exit, not playing: A 0, N and Z from the byte, C 1. Exit, paused: A 0, N 0,
; Z 1, C 1. Exit, gave up: as cd_retry_after_error. Clobbers _BX.
cd_pause:
    lda #$56
    sta z:CardBl
    lda #$22
    sta z:CardBh
    bsr cd_subq
    lda a:CardCdResult
    cmp #$00
    beq cd_pause_at0
    cla
    rts
cd_pause_at0:
    jsr clear_cd_command
    lda #$DA
    sta a:CardCdCommand
    jsr cd_send_command
    jsr cd_finish_command
    cmp #$00
    beq cd_pause_at1
    jsr cd_retry_after_error
    bcc cd_pause
cd_pause_at1:
    rts

; @card 00:EFBF cd_subq
; WHAT IT DOES: Reads the drive's ten bytes of position to where _BX points.
; HOW: Command $DD, tried until it works or the BIOS gives up.
; Entry: _BX pointer. Exit, well: A 0, N 0, Z 1, C 1. Exit, gave up: as
; cd_retry_after_error. X, Y and V as the last exchange left them.
cd_subq:
    jsr save_card_args
cd_subq_at0:
    jsr clear_cd_command
    lda #$DD
    sta a:CardCdCommand
    lda #$0A
    sta a:CardCdCommand+1
    jsr cd_send_command
    cmp #$C8
    bne cd_subq_at1
    lda #$0A
    sta z:CardAl
    stz z:CardAh
    jsr cd_read_data_in
cd_subq_at1:
    jsr cd_finish_command
    cmp #$00
    beq cd_subq_at2
    jsr cd_retry_after_error
    bcs cd_subq_at2
    jsr restore_card_args
    bra cd_subq_at0
cd_subq_at2:
    rts

; @card 00:EFF1 cd_dinfo
; WHAT IT DOES: Reads a piece of the disc's table of contents to where _BX points.
; HOW: Command $DE with _AX as what is asked for; the answer is four bytes.
; Entry: _AX what, _BX pointer. Exit: as cd_subq. _AX and _BX as they were.
cd_dinfo:
    jsr save_card_args
cd_dinfo_at0:
    jsr clear_cd_command
    lda #$DE
    sta a:CardCdCommand
    lda z:CardAl
    sta a:CardCdCommand+1
    lda z:CardAh
    sta a:CardCdCommand+2
    jsr cd_send_command
    cmp #$C8
    bne cd_dinfo_at1
    lda #$04
    sta z:CardAl
    stz z:CardAh
    jsr cd_read_data_in
cd_dinfo_at1:
    jsr cd_finish_command
    pha
    jsr restore_card_args
    pla
    cmp #$00
    beq cd_dinfo_at2
    jsr cd_retry_after_error
    bcs cd_dinfo_at2
    jsr restore_card_args
    bra cd_dinfo_at0
cd_dinfo_at2:
    rts

; @card 00:F0A9 cd_contnts
; WHAT IT DOES: Reads the disc's first and last tracks and its length, and keeps them.
; HOW: cd_dinfo twice into CardCdResult, with the answers copied to CardDiscFirstTrack and
; CardDiscLength.
; Entry: _AH the high byte cd_dinfo asks with. Exit, well: A 0, N 0, Z 1, C 1. Exit, failed: A
; as cd_dinfo left it, N and Z from it, C 1. X, Y, V as cd_dinfo left them.
cd_contnts:
    stz z:CardAl
    lda #$56
    sta z:CardBl
    lda #$22
    sta z:CardBh
    jsr cd_dinfo
    cmp #$00
    bne cd_contnts_at0
    lda a:CardCdResult
    sta a:CardDiscFirstTrack
    lda a:CardCdResult+1
    sta a:CardDiscFirstTrack+1
    lda #$01
    sta z:CardAl
    lda #$56
    sta z:CardBl
    lda #$22
    sta z:CardBh
    jsr cd_dinfo
    cmp #$00
    bne cd_contnts_at0
    pha
    lda a:CardCdResult
    sta a:CardDiscLength
    lda a:CardCdResult+1
    sta a:CardDiscLength+1
    lda a:CardCdResult+2
    sta a:CardDiscLength+2
    pla
cd_contnts_at0:
    rts
