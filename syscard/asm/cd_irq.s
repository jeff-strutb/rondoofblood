; @card 00:E733 jump_user_irq2
; WHAT IT DOES: Goes to the program's IRQ2 handler.
; HOW: JMP through $2200.
; WAITS: no.
; Entry, exit: the program's.
jump_user_irq2:
    jmp (CardIrqVectors)

; @card 00:E736 card_irq2
; WHAT IT DOES: The IRQ2 interrupt: the CD unit's. A program that has set its own handler (bit 0 of CardIrqFlags) has it;
; otherwise cd_interrupt answers.
; HOW: A, X and Y pushed, cd_interrupt called, Y, X and A pulled, RTI.
; WAITS: through cd_interrupt.
; Entry: interrupt; A, X, Y the interrupted code's. Exit: RTI, every register as it was.
card_irq2:
    bbs0 z:CardIrqFlags,jump_user_irq2
    pha
    phx
    phy
    bsr cd_interrupt
    ply
    plx
    pla
    rti

; @card 00:E742 cd_interrupt
; WHAT IT DOES: Answers what the CD unit's interrupt says: a sector of an ADPCM play read (the next asked for), the transfer to
; ADPCM started, data ready (its handshake taken), a subcode byte ready, the ADPCM play half way or ended.
; HOW: The enabled and raised bits of the unit are ORed into the owed bits at $F2, and each bit owed is answered in turn:
; bit 2 the next block of an ADPCM play from the disc, bit 6 the transfer started, bit 5 the data phase and its handshake, bit 4
; the subcode, bit 3 the ADPCM end; each clears its enable and its owed bit and allows interrupts again before it works.
; WAITS: on the bus's request line in the handshake, and for the ADPCM unit to stop, with interrupts on.
; Entry: X, Y (the interrupted code's). Exit: RTS to the handler.
cd_interrupt:
    lda a:CdIrqControl
    and a:CdIrqStatus
    ora z:$F2
    sta z:$F2
    bbr2 z:$F2,cd_interrupt_at3
    lda #$04
    trb a:CdIrqControl
    lda #$04
    trb z:$F2
    cli
    lda a:CardAdpcmSectorsLeft
    ora a:CardAdpcmSectorsLeft+1
    ora a:CardAdpcmSectorsLeft+2
    beq cd_interrupt_at3
    lda a:CdBusStatus
    and #$80
    beq cd_interrupt_at0
    stz a:CardCdBusy
    jsr ad_stop_clear
    bra cd_interrupt_at3
cd_interrupt_at0:
    jsr clear_cd_command
    lda #$08
    sta a:CardCdCommand
    lda a:CardAdpcmNextSector
    sta a:CardCdCommand+1
    lda a:CardAdpcmNextSector+1
    sta a:CardCdCommand+2
    lda a:CardAdpcmNextSector+2
    sta a:CardCdCommand+3
    ldx #$10
    lda a:CardAdpcmSectorsLeft+1
    ora a:CardAdpcmSectorsLeft+2
    bne cd_interrupt_at1
    lda a:CardAdpcmSectorsLeft
    cmp #$10
    bcs cd_interrupt_at1
    tax
cd_interrupt_at1:
    txa
    sta a:CardAdpcmStep
    sta a:CardCdCommand+4
    lda #$01
    sta a:CardCdBusy
    jsr $E905
    lda #$60
    tsb a:CdIrqControl
cd_interrupt_at3:
    bbr6 z:$F2,cd_interrupt_at4
    lda #$40
    trb a:CdIrqControl
    lda #$40
    trb z:$F2
    cli
    lda #$02
    sta a:AdpcmDma
cd_interrupt_at4:
    bbr5 z:$F2,cd_interrupt_at9
    lda #$20
    trb a:CdIrqControl
    lda #$20
    trb z:$F2
    cli
    lda a:CardCdBusy
    beq cd_interrupt_at5
    stz a:AdpcmDma
cd_interrupt_at5:
    lda a:CdData
    sta a:CardCdStatus
    beq cd_interrupt_at6
    stz a:CardCdBusy
    jsr ad_stop_clear
cd_interrupt_at6:
    lda #$80
    tsb a:CdIrqControl
cd_interrupt_at7:
    tst #$40,a:CdBusStatus
    bne cd_interrupt_at7
    lda #$80
    trb a:CdIrqControl
cd_interrupt_at8:
    lda a:CdBusStatus
    and #$F8
    cmp #$F8
    bne cd_interrupt_at8
    lda a:CdData
    lda #$80
    tsb a:CdIrqControl
cd_interrupt_at10:
    tst #$40,a:CdBusStatus
    bne cd_interrupt_at10
    lda #$80
    trb a:CdIrqControl
cd_interrupt_at11:
    tst #$80,a:CdBusStatus
    bne cd_interrupt_at11
    lda a:CardCdBusy
    beq cd_interrupt_at9
    stz a:CardCdBusy
    jsr cplay_advance
    stz a:CardAdpcmStep
    lda #$04
    tsb a:CdIrqControl
cd_interrupt_at9:
    bbr4 z:$F2,cd_interrupt_at12
    lda #$10
    trb a:CdIrqControl
    lda #$10
    trb z:$F2
    cli
    jsr cd_subrd_read
    lda #$10
    tsb a:CdIrqControl
cd_interrupt_at12:
    bbr3 z:$F2,cd_interrupt_at15
    lda #$0C
    trb a:CdIrqControl
    lda #$08
    trb z:$F2
    lda #$60
    trb a:AdpcmControl
    jsr ad_stop_clear
    cli
    lda a:CardAdpcmActive
    beq cd_interrupt_at15
cd_interrupt_at13:
    lda a:AdpcmStatus
    and #$08
    bne cd_interrupt_at13
    jsr ad_play_latch
cd_interrupt_at15:
    rts

; @card 00:E867 jump_user_hsync
; WHAT IT DOES: Goes to the program's horizontal sync handler.
; HOW: JMP through $220A.
; WAITS: no.
; Entry, exit: the program's.
jump_user_hsync:
    jmp (CardIrqVectors+10)

; @card 00:E86A jump_user_vsync
; WHAT IT DOES: Goes to the program's vertical sync handler.
; HOW: JMP through $2208.
; WAITS: no.
; Entry, exit: the program's.
jump_user_vsync:
    jmp (CardIrqVectors+8)

; @card 00:E86D jump_user_irq1
; WHAT IT DOES: Goes to the program's IRQ1 handler.
; HOW: JMP through $2202.
; WAITS: no.
; Entry, exit: the program's.
jump_user_irq1:
    jmp (CardIrqVectors+2)

; @card 00:E870 card_irq1
; WHAT IT DOES: The IRQ1 interrupt: the VDC's. A program that has set its own handler (bit 1 of CardIrqFlags) has it. Otherwise: on a
; raster interrupt the VDC's control register copy is written, the frame counted, the colours and the pads taken unless the program
; asked not to (bit 5), the program's vertical sync handler called (bit 4); on a vertical blank the raster lists written unless the
; program asked not to (bit 7), the horizontal sync handler called (bit 6).
; HOW: A, X and Y pushed, the VDC's status kept at $F6 and tested, TII block moves of the raster lists, ex_colorcmd, ex_joysns,
; the register the VDC had selected put back, the registers pulled, RTI.
; WAITS: no.
; Entry: interrupt; A, X, Y the interrupted code's. Exit: RTI, every register as it was.
card_irq1:
    bbs1 z:CardIrqFlags,jump_user_irq1
    pha
    phx
    phy
    lda a:VdcAddress
    sta z:$F6
    bbr5 z:$F6,card_irq1_at2
    st0 #$05
    lda z:CardVdcCrLo
    sta a:VdcDataLo
    lda z:CardVdcCrHi
    sta a:VdcDataHi
    inc a:CardFrameTick
    bbs5 z:CardIrqFlags,card_irq1_at1
    st0 #$07
    tii $220C,$0002,$0002
    st0 #$08
    tii $2210,$0002,$0002
    jsr ex_colorcmd
    inc a:CardRndSeed
    lda z:CardResetMark
    cmp #$01
    bne card_irq1_at0
    jsr $E6CF
card_irq1_at0:
    jsr ex_joysns
card_irq1_at1:
    bbr4 z:CardIrqFlags,card_irq1_at2
    bsr jump_user_vsync
card_irq1_at2:
    bbr2 z:$F6,card_irq1_at5
    bbs7 z:CardIrqFlags,card_irq1_at4
    bsr cd_interrupt_at15
    bsr cd_interrupt_at15
    st0 #$07
    tii $220E,$0002,$0002
    st0 #$08
    tii $2212,$0002,$0002
card_irq1_at4:
    bbr6 z:CardIrqFlags,card_irq1_at5
    bsr jump_user_hsync
card_irq1_at5:
    lda z:CardVdcReg
    sta a:VdcAddress
    ply
    plx
    pla
    rti
