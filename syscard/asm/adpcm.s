; @card 00:F6C1 ad_stop
; WHAT IT DOES: Stops ADPCM playback and forgets what was playing.
; HOW: Clears its interrupt enables and its control bits, then the card's play state.
; Entry: nothing. Exit: A $60; N and V bits 7 and 6 of the control port as it read, Z if
; nothing else was set in it. X, Y, C kept.
ad_stop:
    lda #$0C
    trb a:CdIrqControl
    lda #$60
    trb a:AdpcmControl
    stz a:CardAdpcmActive
    stz a:CardAdpcmStep
    stz a:CardAdpcmSectorsLeft
    stz a:CardAdpcmSectorsLeft+1
    stz a:CardAdpcmSectorsLeft+2
    rts

; @card 00:F6DB ad_stat
; WHAT IT DOES: Says whether ADPCM is busy and whether it is playing.
; HOW: Each answer is one port's bit, or if clear another's.
; Entry: nothing. Exit: X busy, A playing, N and Z from A. Y, C, V kept.
ad_stat:
    lda a:AdpcmStatus
    and #$01
    bne ad_stat_at0
    lda a:CdIrqStatus
    and #$04
ad_stat_at0:
    tax
    lda a:AdpcmControl
    and #$20
    bne ad_stat_at1
    lda a:AdpcmStatus
    and #$08
ad_stat_at1:
    rts

; @card 00:F37F ad_reset
; WHAT IT DOES: Resets the ADPCM unit.
; HOW: Pulses its reset bit, stops any transfer from the CD, turns off the CD unit's interrupt
; enables but the bus acknowledge, and sets the playback rate to zero.
; Entry: nothing. Exit: A $6F, N and V bits 7 and 6 of the interrupt enables as they were, Z if
; nothing outside $6F was set in them. X, Y, C kept.
ad_reset:
    lda #$80
    sta a:AdpcmControl
    stz a:AdpcmControl
    stz a:AdpcmDma
    lda #$6F
    trb a:CdIrqControl
    stz a:AdpcmRate
    rts

; @card 00:F5C6 ad_play
; WHAT IT DOES: Plays ADPCM: _AX bytes from address _BX at rate _DH; bit 7 of _DL repeats it,
; and any other bit of _DL plays what is already latched.
; HOW: Does nothing while ADPCM plays. Keeps _BX, _AX and the rate, latches the address and the
; length, sets the rate, then turns on the half-way interrupt and starts the unit.
; Entry: _AX, _BX, _DL, _DH. Exit, busy: as ad_stat. Bad rate: A _DH, C set, N and Z from
; comparing it with $10, X as ad_stat. Played: A 0, N 0, Z 0, V bit 6 of the interrupt enables
; as they were; latched: X and Y _AX, C clear; played again: X as ad_stat. Y, C, V kept
; otherwise.
ad_play:
    jsr ad_stat
    bne ad_play_at0
    lda z:CardDl
    and #$80
    sta a:CardAdpcmActive
    lda z:CardDl
    and #$7F
    bne ad_play_at1
    lda z:CardBl
    sta a:CardAdpcmAddress
    lda z:CardBh
    sta a:CardAdpcmAddress+1
    lda z:CardAl
    sta a:CardAdpcmLength
    lda z:CardAh
    sta a:CardAdpcmLength+1
    lda z:CardDh
    cmp #$10
    bcs ad_play_at0
    sta a:CardAdpcmRate
    ldx a:CardAdpcmAddress
    ldy a:CardAdpcmAddress+1
    jsr ad_set_address
    jsr ad_latch_read_address
    ldx a:CardAdpcmLength
    ldy a:CardAdpcmLength+1
    jsr ad_set_address
    jsr ad_latch_length
    lda a:CardAdpcmRate
    sta a:AdpcmRate
ad_play_at1:
    lda #$08
    tsb a:CdIrqControl
    lda #$60
    sta a:AdpcmControl
    cla
ad_play_at0:
    rts


; @card 00:F407 ad_read
; WHAT IT DOES: Reads _AX bytes of ADPCM memory from address _CX: to memory at _BX if _DH is 0,
; to VRAM at _BX if _DH is $FF, or else to banks from _BL on, mapped through the MPR numbered _DH
; (1 to 6). Any other _DH gives $22.
; HOW: Waits until ADPCM is not playing, latches the address, drops the byte the unit had
; fetched, then reads byte after byte. VRAM goes through the data port, its two halves in turn;
; banks through a TMA or TAM stub it writes in RAM, 8 KB a bank, the MPR put back at the end.
; Entry: _AX, _BX, _CX, _DH, and _BL for banks. Exit: A 0, or $22 refused. See abi_ad_read.
ad_read:
    jsr ad_stat
    bne ad_read
    ldx z:CardCl
    ldy z:CardCh
    jsr ad_set_address
    jsr ad_latch_read_address
    bsr ad_read_byte
    lda z:CardDh
    beq ad_read_at0
    cmp #$FF
    beq ad_read_at1
    jmp ad_read_to_banks
ad_read_at0:
    bsr ad_read_byte
    sta (CardBl)
    inc z:CardBl
    bne ad_read_at2
    inc z:CardBh
ad_read_at2:
    lda z:CardAl
    bne ad_read_at3
    dec z:CardAh
ad_read_at3:
    dec z:CardAl
    lda z:CardAl
    ora z:CardAh
    bne ad_read_at0
    cla
    rts
ad_read_at1:
    lda #$00
    sta z:CardVdcReg
    sta a:VdcAddress
    jsr aim_bx_at_vram_port
    lda #$02
    sta z:CardVdcReg
    sta a:VdcAddress
ad_read_at5:
    bsr ad_read_byte
    sta (CardBl)
    lda z:CardBl
    eor #$01
    sta z:CardBl
    lda z:CardAl
    bne ad_read_at4
    dec z:CardAh
ad_read_at4:
    dec z:CardAl
    lda z:CardAl
    ora z:CardAh
    bne ad_read_at5
    stz a:CardVramPending
    cla
    rts

; @card 00:F46B ad_read_to_banks
; WHAT IT DOES: ad_read's reading to banks: _AX bytes of ADPCM memory to bank _BL and the banks
; after it, through the MPR numbered _DH (A on entry). A number of 7 or more is refused with $22.
; HOW: A TMA stub written in RAM keeps the bank the MPR held; each bank is mapped with a TAM stub
; and filled, 8 KB, at the MPR's window; the kept bank goes back at the end.
; Entry: A _DH, the ADPCM address latched. Exit, refused: A $22, N 0, Z 0, C from comparing with
; 7. Read: A 0, N and Z from the bank put back, C bit 3 of _DH, X the pages and Y the bytes left
; of the last bank. V kept.
ad_read_to_banks:
    cmp #$07
    bmi ad_read_to_banks_at0
    lda #$22
    rts
ad_read_to_banks_at0:
    tax
    lda #$01
ad_read_to_banks_at1:
    asl a
    dex
    bne ad_read_to_banks_at1
    sta a:CardMprStubMask
    lda #$43
    sta a:CardMprStub
    lda #$60
    sta a:CardMprStubReturn
    jsr CardMprStub
    sta a:CardSavedMprBank
    lda z:CardBl
    sta z:CardDl
ad_read_to_banks_at6:
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
    ldx #$20
    cly
ad_read_to_banks_at5:
    jsr ad_read_byte
    sta (CardBl)
    inc z:CardBl
    bne ad_read_to_banks_at2
    inc z:CardBh
ad_read_to_banks_at2:
    lda z:CardAl
    bne ad_read_to_banks_at3
    dec z:CardAh
ad_read_to_banks_at3:
    dec z:CardAl
    lda z:CardAl
    ora z:CardAh
    beq ad_read_to_banks_at4
    dey
    bne ad_read_to_banks_at5
    dex
    bne ad_read_to_banks_at5
    inc z:CardDl
    bra ad_read_to_banks_at6
ad_read_to_banks_at4:
    lda #$53
    sta a:CardMprStub
    lda a:CardSavedMprBank
    jsr CardMprStub
    cla
    rts

; @card 00:F6F5 aim_bx_at_vram_port
; WHAT IT DOES: Makes _BX the VDC's write address, then points _BX at the VDC's data port, for a
; copy that writes through [_BX].
; HOW: write_vram_word_pending, with the address register selected; then _BX = $0002.
; Entry: _BX, the VDC's address register selected. Exit: A 2, N 0, Z 0. X, Y, C, V kept.
aim_bx_at_vram_port:
    jsr write_vram_word_pending
    lda #$02
    sta z:CardBl
    stz z:CardBh
    rts


; @card 00:F61F ad_cplay
; WHAT IT DOES: Plays ADPCM straight from the disc: _AL, _AH, _BL sectors (low first) from the
; sector _CL, _CH, _DL (high first, counted from the first disc base) at rate _DH.
; HOW: Does nothing while ADPCM plays. Keeps the count and the first sector, resets the unit,
; sets the rate, loads the first $20 sectors into ADPCM memory from 0 with ad_trans, moves the
; counts on by them, makes the length all of memory, and starts the unit with its half-way and
; end interrupts, which carry on the play.
; Entry: _AX, _BL, _CL, _CH, _DL, _DH. Exit: busy, as ad_stat; bad rate, A _DH and C set; a
; failed load, as ad_trans with C set; started, A 0, N 0, Z 0, X and Y as ad_trans left them.
; _AL, _BX and _DH changed.
ad_cplay:
    jsr ad_stat
    bne ad_cplay_at0
    lda z:CardAl
    sta a:CardAdpcmSectorsLeft
    lda z:CardAh
    sta a:CardAdpcmSectorsLeft+1
    lda z:CardBl
    sta a:CardAdpcmSectorsLeft+2
    lda z:CardDl
    clc
    adc a:CardCdBases+2
    sta a:CardAdpcmNextSector+2
    lda z:CardCh
    adc a:CardCdBases+1
    sta a:CardAdpcmNextSector+1
    lda z:CardCl
    adc a:CardCdBases
    sta a:CardAdpcmNextSector
    jsr ad_reset
    lda z:CardDh
    cmp #$10
    bcs ad_cplay_at0
    sta a:AdpcmRate
    lda #$20
    sta z:CardAl
    lda #$00
    sta z:CardDh
    stz z:CardBl
    stz z:CardBh
    jsr ad_trans
    cmp #$00
    bne ad_cplay_at0
    lda #$20
    sta a:CardAdpcmStep
    bsr cplay_advance
    lda #$FF
    sta a:AdpcmAddressLo
    sta a:AdpcmAddressHi
    jsr ad_latch_length
    stz a:CardAdpcmActive
    lda #$0C
    tsb a:CdIrqControl
    lda #$60
    sta a:AdpcmControl
    cla
ad_cplay_at0:
    rts

; @card 00:F68C cplay_advance
; WHAT IT DOES: Moves continuous ADPCM play on by CardAdpcmStep sectors: the next sector to read
; on, the sectors left down.
; HOW: A three-byte add to CardAdpcmNextSector (high first) and a three-byte subtract from
; CardAdpcmSectorsLeft (low first).
; Entry: CardAdpcmStep. Exit: A the high byte of what is left, N and Z from it, C clear if the
; subtract went below zero, V from its high byte. X, Y kept.
cplay_advance:
    clc
    lda a:CardAdpcmNextSector+2
    adc a:CardAdpcmStep
    sta a:CardAdpcmNextSector+2
    lda a:CardAdpcmNextSector+1
    adc #$00
    sta a:CardAdpcmNextSector+1
    lda a:CardAdpcmNextSector
    adc #$00
    sta a:CardAdpcmNextSector
    sec
    lda a:CardAdpcmSectorsLeft
    sbc a:CardAdpcmStep
    sta a:CardAdpcmSectorsLeft
    lda a:CardAdpcmSectorsLeft+1
    sbc #$00
    sta a:CardAdpcmSectorsLeft+1
    lda a:CardAdpcmSectorsLeft+2
    sbc #$00
    sta a:CardAdpcmSectorsLeft+2
    rts

; @card 00:F730 ad_trans_begin
; WHAT IT DOES: Sets up a read of _AL sectors from the disc into ADPCM memory at _BX, from the
; sector _CL, _CH, _DL (high first) plus the base CardCdBaseIndex chooses, for ad_trans_step to
; carry out a step a call. Returns at once; refused while a transfer from the CD runs.
; HOW: Builds command 8 in CardCdCommand without clearing it first (bytes 6 to 9 stay as they
; were), keeps _BX, and sets ad_trans_step's first step.
; Entry: _AL, _BX, _CL, _CH, _DL. Exit, refused: C 1, N and V bits 7 and 6 of the transfer port,
; Z 0; A, X, Y kept. Set up: A 1, N 0, Z 0, C 0, V from the base's add, Y the base's offset; X
; kept. _CL, _CH, _DL left as the sector plus the base.
ad_trans_begin:
    tst #$03,a:AdpcmDma
    beq ad_trans_begin_at0
    sec
    bra ad_trans_begin_at1
ad_trans_begin_at0:
    lda #$08
    sta a:CardCdCommand
    jsr add_cd_base
    lda z:CardCl
    sta a:CardCdCommand+1
    lda z:CardCh
    sta a:CardCdCommand+2
    lda z:CardDl
    sta a:CardCdCommand+3
    lda z:CardAl
    sta a:CardCdCommand+4
    stz a:CardCdCommand+5
    lda z:CardBl
    sta a:CardAdTransAddress
    lda z:CardBh
    sta a:CardAdTransAddress+1
    stz a:CardAdTransSent
    lda #$01
    sta a:CardAdTransStep
    clc
ad_trans_begin_at1:
    rts
