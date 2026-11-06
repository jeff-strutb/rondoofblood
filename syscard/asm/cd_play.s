; @card 00:EE10 cd_play
; WHAT IT DOES: Plays CD audio: seeks to the start unless told not to, then starts the play;
; bit 0 of _BH stops after the seek.
; HOW: Commands $D8 and $D9. The play's mode goes in _DH; mode 2 of the drive's previous play
; hands the end to an interrupt.
; Entry: _AX, _BL start, _BH its mode, _CX, _DL end, _DH its mode. Exit: A 0, C 1 and the rest
; as the path left them; or as cd_retry_after_error if it gave up.
cd_play:
    jsr save_card_args
cd_play_at0:
    jsr clear_cd_command
    lda z:CardBh
    and #$C0
    cmp #$C0
    beq cd_play_at4
    cmp #$80
    bne cd_play_at1
    stz z:CardAh
    stz z:CardBl
cd_play_at1:
    sta a:CardCdCommand+9
    lda #$D8
    sta a:CardCdCommand
    lda z:CardBh
    bne cd_play_at2
    clx
    ldy #$03
    jsr copy_args_to_command
    bra cd_play_at3
cd_play_at2:
    clx
    ldy #$02
    jsr copy_args_to_command
cd_play_at3:
    jsr cd_send_command
    jsr cd_finish_command
    cmp #$00
    beq cd_play_at4
    jsr cd_retry_after_error
    bcc cd_play_at8
    rts
cd_play_at8:
    jsr restore_card_args
    bra cd_play_at0
cd_play_at4:
    lda z:CardBh
    and #$01
    beq cd_play_at5
cd_play_at9:
    cla
    rts
cd_play_at5:
    jsr clear_cd_command
    lda #$D9
    sta a:CardCdCommand
    lda z:CardDh
    and #$07
    sta a:CardCdCommand+1
    lda z:CardDh
    and #$C0
    sta a:CardCdCommand+9
    bne cd_play_at6
    ldx #$04
    ldy #$03
    jsr copy_args_to_command
    bra cd_play_at11
cd_play_at6:
    cmp #$C0
    beq cd_play_at10
    cmp #$80
    bne cd_play_at7
    stz z:CardCh
    stz z:CardDl
cd_play_at7:
    ldx #$04
    ldy #$02
    jsr copy_args_to_command
    bra cd_play_at11
cd_play_at10:
    lda a:CardDiscLength
    sta a:CardDiscEnd
    lda a:CardDiscLength+1
    sta a:CardDiscEnd+1
    lda a:CardDiscLength+2
    sta a:CardDiscEnd+2
    bsr disc_end_minus_one_frame
    lda a:CardDiscEnd
    sta a:CardCdCommand+2
    lda a:CardDiscEnd+1
    sta a:CardCdCommand+3
    lda a:CardDiscEnd+2
    sta a:CardCdCommand+4
    lda #$40
    sta a:CardCdCommand+9
cd_play_at11:
    jsr cd_send_command
    cmp #$D8
    beq cd_play_at13
    lda a:CardCdCommand+1
    cmp #$04
    bne cd_play_at12
    lda a:CardCdPlayMode
cd_play_at12:
    sta a:CardCdPlayMode
    cmp #$01
    beq cd_play_at9
    cmp #$02
    bne cd_play_at13
    stz a:CardCdBusy
    lda #$20
    tsb a:CdIrqControl
    jmp cd_play_at9
cd_play_at13:
    lda a:CardCdCommand+1
    cmp #$04
    bne cd_play_at14
    sta a:CardCdPlayMode
cd_play_at14:
    jsr cd_finish_command
    cmp #$00
    beq cd_play_at15
    jsr cd_retry_after_error
    bcs cd_play_at15
    jsr restore_card_args
    jmp cd_play_at4
cd_play_at15:
    rts

; @card 00:EF02 disc_end_minus_one_frame
; WHAT IT DOES: Takes one frame off the time kept in CardDiscEnd.
; HOW: Minutes, seconds and frames in packed decimal: below 0 frames go to 74 and borrow a
; second, below 0 seconds go to 59 and borrow a minute.
; Entry: CardDiscEnd. Exit: A kept, N and Z from it; C from the last digit's step. X, Y, V kept.
disc_end_minus_one_frame:
    pha
    sed
    lda a:CardDiscEnd+2
    sec
    sbc #$01
    sta a:CardDiscEnd+2
    cmp #$75
    bcc disc_end_minus_one_frame_at0
    lda #$74
    sta a:CardDiscEnd+2
    lda a:CardDiscEnd+1
    sec
    sbc #$01
    sta a:CardDiscEnd+1
    cmp #$60
    bcc disc_end_minus_one_frame_at0
    lda #$59
    sta a:CardDiscEnd+1
    lda a:CardDiscEnd
    sec
    sbc #$01
    sta a:CardDiscEnd
disc_end_minus_one_frame_at0:
    cld
    pla
    rts
