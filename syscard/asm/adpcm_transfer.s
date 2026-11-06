; @card 00:F393 ad_trans
; WHAT IT DOES: Reads sectors from the disc into ADPCM memory.
; HOW: Command 8 with the unit's own transfer running until the drive is done. Refuses while
; a transfer is already running, and retries like cd_read.
; Entry: _AL sectors, _BX ADPCM address, _CL, _CH, _DL disc address, _DH 0 to set the address.
; Exit, refused: A 4, N and Z from it, V bit 6 of the transfer port. Exit, well: A 0, N 0, Z 1,
; C 1. Exit, gave up: as cd_retry_after_error. Y 1 unless a step changed it.
ad_trans:
    tst #$03,a:AdpcmDma
    beq ad_trans_at0
    lda #$04
    bra ad_trans_at4
ad_trans_at0:
    stz a:CardCdBaseIndex
    jsr save_card_args
ad_trans_at1:
    lda z:CardDh
    bne ad_trans_at2
    ldx z:CardBl
    ldy z:CardBh
    jsr ad_set_address
    jsr ad_latch_write_address
ad_trans_at2:
    jsr clear_cd_command
    lda #$08
    sta a:CardCdCommand
    jsr add_cd_base
    ldx #$04
    ldy #$01
    jsr copy_args_to_command
    lda z:CardAl
    sta a:CardCdCommand+4
    jsr cd_send_command
    lda #$02
    sta a:AdpcmDma
ad_trans_at3:
    tst #$04,a:AdpcmStatus
    bne ad_trans_at3
    lda a:CdIrqStatus
    and #$20
    beq ad_trans_at3
    stz a:AdpcmDma
    jsr cd_finish_command
    cmp #$00
    beq ad_trans_at4
    jsr cd_retry_after_error
    bcs ad_trans_at4
    jsr restore_card_args
    lda a:CardCdSense
    cmp #$10
    bcc ad_trans_at1
    lda #$01
    sta a:CardCdBaseIndex
    bra ad_trans_at1
ad_trans_at4:
    rts
