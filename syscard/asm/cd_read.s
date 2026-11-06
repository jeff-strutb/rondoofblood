; @card 00:EC05 cd_read
; WHAT IT DOES: Reads sectors from the disc to memory or VRAM.
; HOW: Command 8, with the address plus the chosen base. _DH says where they go: 0 _AX bytes
; to _BX, 1 _AL sectors to _BX, 2-6 _AL sectors through the MPR it names, $FE _AX bytes to VRAM
; with a handshake each, $FF _AL sectors to VRAM. After a sense code of $10 or more it uses the
; other base.
; Entry: _CL, _CH, _DL address; the rest by _DH. Exit: A 0 if well, $22 for a bad request; else
; as cd_retry_after_error. C 1 if well. X, Y and V as the last step left them.
cd_read:
    stz a:CardCdBaseIndex
    jsr save_card_args
cd_read_at0:
    jsr clear_cd_command
    lda #$08
    sta a:CardCdCommand
    jsr add_cd_base
    ldx #$04
    ldy #$01
    jsr copy_args_to_command
    lda z:CardDh
    cmp #$02
    bcc cd_read_at1
    cmp #$FF
    beq cd_read_at8
    cmp #$FE
    bne cd_read_at14
    jmp cd_read_at12
cd_read_at14:
    jmp cd_read_at20
cd_read_at1:
    lda z:CardDh
    beq cd_read_at2
    lda z:CardAl
    sta a:CardCdSectors
    asl a
    asl a
    asl a
    sta z:CardAh
    stz z:CardAl
    bra cd_read_at3
cd_read_at2:
    jsr cd_bytes_to_sectors
cd_read_at3:
    lda a:CardCdSectors
    beq cd_read_at7
    sta a:CardCdCommand+4
    jsr cd_send_command
    cmp #$C8
    bne cd_read_at4
    jsr cd_read_to_memory
    cmp #$C8
    bne cd_read_at4
    jsr cd_drop_extra_data
cd_read_at4:
    jsr cd_finish_command
    cmp #$00
    beq cd_read_at5
    jsr cd_retry_after_error
    bcc cd_read_at6
    rts
cd_read_at5:
    lda z:CardAl
    ora z:CardAh
    bne cd_read_at6
    rts
cd_read_at6:
    jmp cd_read_at19
cd_read_at7:
    lda #$22
    rts
cd_read_at8:
    lda z:CardAl
    beq cd_read_at7
    sta a:CardCdCommand+4
    jsr cd_send_command
    cmp #$C8
    bne cd_read_at9
    lda z:CardAl
    asl a
    asl a
    asl a
    sta z:CardAh
    stz z:CardAl
    lda #$00
    sta z:CardVdcReg
    sta a:VdcAddress
    jsr write_vram_word_pending
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
    jsr cd_read_to_vram_fast
    stz a:CardVramPending
cd_read_at9:
    jsr cd_finish_command
    cmp #$00
    beq cd_read_at10
    jsr cd_retry_after_error
    bcc cd_read_at19
    rts
cd_read_at10:
    lda z:CardAl
    ora z:CardAh
    bne cd_read_at19
    rts
cd_read_at12:
    jsr cd_bytes_to_sectors
    lda a:CardCdSectors
    beq cd_read_at7
    sta a:CardCdCommand+4
    jsr cd_send_command
    cmp #$C8
    bne cd_read_at13
    lda #$00
    sta z:CardVdcReg
    sta a:VdcAddress
    jsr write_vram_word_pending
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
    jsr cd_read_to_vram_slow
    stz a:CardVramPending
cd_read_at13:
    jsr cd_finish_command
    cmp #$00
    beq cd_read_at10
    jsr cd_retry_after_error
    bcc cd_read_at19
    rts
cd_read_at19:
    jsr restore_card_args
    lda a:CardCdSense
    cmp #$10
    bcc cd_read_at15
    lda #$01
    sta a:CardCdBaseIndex
cd_read_at15:
    jmp cd_read_at0
cd_read_at20:
    ldx z:CardAl
    beq cd_read_at16
    cmp #$07
    bmi cd_read_at17
cd_read_at16:
    lda #$22
    rts
cd_read_at17:
    tax
    lda #$01
cd_read_at18:
    asl a
    dex
    bne cd_read_at18
    sta a:CardMprStubMask
    lda #$43
    sta a:CardMprStub
    lda #$60
    sta a:CardMprStubReturn
    jsr CardMprStub
    sta a:CardSavedMprBank
    lda z:CardBl
    sta z:CardDl
    lda z:CardAl
    sta a:CardCdCommand+4
    lsr a
    lsr a
    sta a:CardCdSectors
    lda z:CardAl
    and #$03
    sta a:CardCdWhole
    jsr cd_send_command
    ora #$40
    cmp #$C8
    bne cd_read_at23
cd_read_at21:
    lda a:CardCdSectors
    beq cd_read_at22
    lda #$20
    sta z:CardAh
    bra cd_read_at24
cd_read_at22:
    lda a:CardCdWhole
    beq cd_read_at23
    asl a
    asl a
    asl a
    sta z:CardAh
cd_read_at24:
    stz z:CardAl
    lda z:CardDh
    asl a
    asl a
    asl a
    asl a
    asl a
    sta z:CardBh
    stz z:CardBl
    lda #$53
    sta a:CardMprStub
    lda z:CardDl
    jsr CardMprStub
    jsr cd_read_to_memory
    ora #$40
    cmp #$C8
    bne cd_read_at23
    inc z:CardDl
    dec a:CardCdSectors
    bpl cd_read_at21
cd_read_at23:
    jsr cd_finish_command
    cmp #$00
    beq cd_read_at25
    jsr cd_retry_after_error
    bcs cd_read_at25
    jmp cd_read_at19
cd_read_at25:
    pha
    lda #$53
    sta a:CardMprStub
    lda a:CardSavedMprBank
    jsr CardMprStub
    pla
    rts
