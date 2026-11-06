; @card 00:EA79 cd_read_to_memory
; WHAT IT DOES: Takes whole blocks from the drive to memory, through the auto port, up to the
; count in _AX.
; HOW: Bytes to where _BX points, in blocks of eight pages, until the count runs out or the
; drive moves to its status phase.
; Entry: _AX count, _BX pointer. Exit, status phase: A $D8, N 0, Z 1, C 1. Exit, count out: A
; the phase, N and Z from it, C 1. X and Y as the loop left them, if a byte moved. V kept.
; Clobbers _AX, _BH.
cd_read_to_memory:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$C8
    beq cd_read_to_memory_at0
    cmp #$D8
    beq cd_read_to_memory_at6
    bra cd_read_to_memory
cd_read_to_memory_at0:
    lda z:CardAl
    ora z:CardAh
    beq cd_read_to_memory_at5
    ldx #$03
cd_read_to_memory_at1:
    dex
    bne cd_read_to_memory_at1
    cly
    ldx #$08
cd_read_to_memory_at2:
    lda a:CdDataAuto
    sta (CardBl),y
    lda z:CardAl
    bne cd_read_to_memory_at3
    dec z:CardAh
cd_read_to_memory_at3:
    dec a
    sta z:CardAl
    ora z:CardAh
    beq cd_read_to_memory_at5
    iny
    bne cd_read_to_memory_at2
    inc z:CardBh
    dex
    bne cd_read_to_memory_at2
    bra cd_read_to_memory
cd_read_to_memory_at5:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
cd_read_to_memory_at6:
    rts

; @card 00:EABE cd_read_to_vram_fast
; WHAT IT DOES: Takes whole blocks from the drive to VRAM, through the auto port, up to the
; count in _AX.
; HOW: A word to the VDC's data port for each two bytes, in blocks of four pages of words.
; Entry: _AX count, VDC on its data port. Exit, status phase first: as the end of a read. Exit,
; count out: as cd_vram_read_end. X, Y as the loop left them. Clobbers _AX.
cd_read_to_vram_fast:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$C8
    beq cd_read_to_vram_fast_at0
    cmp #$D8
    beq cd_read_to_vram_fast_at3
    bra cd_read_to_vram_fast
cd_read_to_vram_fast_at0:
    ldx #$03
cd_read_to_vram_fast_at1:
    dex
    bne cd_read_to_vram_fast_at1
    cly
    ldx #$04
cd_read_to_vram_fast_at2:
    lda a:CdDataAuto
    sta a:VdcDataLo
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    nop
    lda a:CdDataAuto
    sta a:VdcDataHi
    lda z:CardAl
    sec
    sbc #$02
    sta z:CardAl
    lda z:CardAh
    sbc #$00
    sta z:CardAh
    ora z:CardAl
    beq cd_read_to_vram_fast_at3
    dey
    bne cd_read_to_vram_fast_at2
    dex
    bne cd_read_to_vram_fast_at2
    bra cd_read_to_vram_fast
cd_read_to_vram_fast_at3:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    and #$B8
    cmp #$88
    bne cd_read_to_vram_fast_at4
    bra cd_drop_extra_data
cd_read_to_vram_fast_at4:
    rts

; @card 00:EB16 cd_read_to_vram_slow
; WHAT IT DOES: Takes bytes from the drive to VRAM one handshake at a time, up to the count in
; _AX.
; HOW: Each byte to the VDC's data port, low then high, through a pointer in _BX.
; Entry: _AX count. Exit, status phase first: A $D8, N 0, Z 1, C 1. Exit, count out: as
; cd_vram_read_end. Clobbers _AX, _BX.
cd_read_to_vram_slow:
    lda #$02
    sta z:CardBl
    lda #$00
    sta z:CardBh
cd_read_to_vram_slow_at0:
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    cmp #$C8
    beq cd_read_to_vram_slow_at1
    cmp #$D8
    beq cd_read_to_vram_slow_at2
    bra cd_read_to_vram_slow_at0
cd_read_to_vram_slow_at1:
    lda a:CdData
    sta (CardBl)
    jsr cd_ack_pulse
    lda z:CardBl
    eor #$01
    sta z:CardBl
    lda z:CardAl
    sec
    sbc #$01
    sta z:CardAl
    lda z:CardAh
    sbc #$00
    sta z:CardAh
    ora z:CardAl
    bne cd_read_to_vram_slow_at0
    lda a:CdBusStatus
    and #$F8
    sta a:CardBusPhase
    and #$B8
    cmp #$88
    beq cd_drop_extra_data
cd_read_to_vram_slow_at2:
    rts

; @card 00:ED9F cd_bytes_to_sectors
; WHAT IT DOES: Works out how many sectors hold the count in _AX.
; HOW: The high byte over 8, and one more if anything is left over: CardCdWhole says none was.
; Entry: _AX bytes. Exit: A CardCdWhole; N and Z from it, or from the sectors counted if one
; was added; C bit 2 of _AH. X, Y, V kept.
cd_bytes_to_sectors:
    lda #$01
    sta a:CardCdWhole
    lda z:CardAh
    lsr a
    bcc cd_bytes_to_sectors_at0
    stz a:CardCdWhole
cd_bytes_to_sectors_at0:
    lsr a
    bcc cd_bytes_to_sectors_at1
    stz a:CardCdWhole
cd_bytes_to_sectors_at1:
    lsr a
    bcc cd_bytes_to_sectors_at2
    stz a:CardCdWhole
cd_bytes_to_sectors_at2:
    sta a:CardCdSectors
    lda z:CardAl
    beq cd_bytes_to_sectors_at3
    stz a:CardCdWhole
cd_bytes_to_sectors_at3:
    lda a:CardCdWhole
    bne cd_bytes_to_sectors_at4
    inc a:CardCdSectors
cd_bytes_to_sectors_at4:
    rts
