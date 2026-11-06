; @card 00:F0EE clear_cd_command
; WHAT IT DOES: Clears the CD command being built, past its first byte.
; HOW: Zeroes bytes 1 to 9.
; Entry: nothing. Exit: N and Z from X. A, X, Y, C, V kept.
clear_cd_command:
    phx
    ldx #$09
clear_cd_command_at0:
    stz a:CardCdCommand,x
    dex
    bne clear_cd_command_at0
    plx
    rts

; @card 00:F0F9 clear_cd_result
; WHAT IT DOES: Clears what the drive last answered.
; HOW: Zeroes all ten bytes.
; Entry: nothing. Exit: N and Z from X. A, X, Y, C, V kept.
clear_cd_result:
    phx
    ldx #$09
clear_cd_result_at0:
    stz a:CardCdResult,x
    dex
    bpl clear_cd_result_at0
    plx
    rts

; @card 00:F104 add_cd_base
; WHAT IT DOES: Adds the base the caller chose to a disc address.
; HOW: The base table, three bytes an entry, by CardCdBaseIndex; a 24-bit add into _CL, _CH
; and _DL.
; Entry: _CL, _CH, _DL. Exit: A the sum's top byte, N, Z, C, V from its add, Y the table's
; offset. X kept.
add_cd_base:
    lda a:CardCdBaseIndex
    asl a
    clc
    adc a:CardCdBaseIndex
    tay
    lda z:CardDl
    clc
    adc a:CardCdBases+2,y
    sta z:CardDl
    lda z:CardCh
    adc a:CardCdBases+1,y
    sta z:CardCh
    lda z:CardCl
    adc a:CardCdBases,y
    sta z:CardCl
    rts

; @card 00:F309 save_card_args
; WHAT IT DOES: Keeps _AX to _DX across a command that uses them.
; HOW: Copies the eight bytes to CardSavedArgs.
; Entry: nothing. Exit: N and Z from X. A, X, Y, C, V kept.
save_card_args:
    phx
    pha
    ldx #$07
save_card_args_at0:
    lda z:CardAl,x
    sta a:CardSavedArgs,x
    dex
    bpl save_card_args_at0
    pla
    plx
    rts

; @card 00:F318 restore_card_args
; WHAT IT DOES: Puts back what save_card_args kept.
; HOW: Copies the eight bytes back.
; Entry: nothing. Exit: N and Z from X. A, X, Y, C, V kept.
restore_card_args:
    phx
    pha
    ldx #$07
restore_card_args_at0:
    lda a:CardSavedArgs,x
    sta z:CardAl,x
    dex
    bpl restore_card_args_at0
    pla
    plx
    rts

; @card 00:F327 copy_args_to_command
; WHAT IT DOES: Puts a three-byte argument into the CD command being built.
; HOW: Three bytes from _AL plus an offset to the command plus another.
; Entry: X the argument's offset, Y the command's. Exit: A the last byte, N and Z from it. X,
; Y, C, V kept.
copy_args_to_command:
    lda z:CardAl,x
    sta a:CardCdCommand,y
    lda z:CardAl+1,x
    sta a:CardCdCommand+1,y
    lda z:CardAl+2,x
    sta a:CardCdCommand+2,y
    rts

; @card 00:F337 write_vram_word_pending
; WHAT IT DOES: Writes _BX to the VDC's data port, and marks that it was written.
; HOW: CardVramPending 1, then the two bytes.
; Entry: _BX. Exit: A _BH, N and Z from it. X, Y, C, V kept.
write_vram_word_pending:
    lda #$01
    sta a:CardVramPending
    lda z:CardBl
    sta a:VdcDataLo
    lda z:CardBh
    sta a:VdcDataHi
    rts

; @card 00:F354 cd_subrd
; WHAT IT DOES: Reads a subcode byte, if one is waiting.
; HOW: Bit 4 of the status port; the byte comes from $1807 and is kept in CardSubcodeByte.
; Entry: nothing. Exit, one read: A the byte, N and Z from it, C clear, V bit 6 of the status.
; Exit, none: C set, Z set, N and V bits 7 and 6 of the status, A kept. X, Y kept.
cd_subrd:
    tst #$10,a:CdIrqStatus
    beq cd_subrd_at0
    lda a:CdSubcode
    sta a:CardSubcodeByte
    clc
    rts
cd_subrd_at0:
    sec
    rts

; @card 00:F364 cd_pcmrd
; WHAT IT DOES: Latches a CD audio sample and reads it.
; HOW: A write to the latch port, a pause for it that the C does not state, then its two
; bytes and the status port's bit 1.
; Entry: A what goes to the latch. Exit: X the low byte, Y the high, A the bit, N 0, Z from
; it, C clear. V kept.
cd_pcmrd:
    sta a:CdPcmLatch
    ldx #$1B
cd_pcmrd_at0:
    dex
    bne cd_pcmrd_at0
    ldx a:CdPcmLatch
    ldy a:CdPcmHigh
    lda a:CdIrqStatus
    and #$02
    lsr a
    rts
