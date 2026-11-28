; @routine 0000:326 IplStart
; @run F8:2B26
; WHAT IT DOES: The disc's loader. Returns to the card unless the $4A bytes at $3020 match the card's at $DFB6. The
; working flags (zTemp00) are the information block's, or $60 when it has none. With bit 0 set it reads the block's
; graphics data by three cd_reads (bit 0 is dropped when one fails or the count is out of range); with bit 1 set it
; reads the block's ADPCM and starts it; with bit 5 clear and bit 0 set it calls the card's $E08A. Then it maps I/O,
; work RAM and the block's five banks, and reads the program the block names: with no sector count it jumps to the
; run address plus $3000; else it reads to the load address and jumps to the run address, from a copy of its last
; $15 bytes at $DFE0 when the load address is below $3000. A failed program read returns to the card.
; HOW: The compare loop; the flags; ipl_load_data and ipl_load_adpcm are the two optional parts of the original's
; straight line; TAM for each bank; TII for the eight-byte block; the stack emptied and a jump through $2007.
; WAITS: yes, in the card's calls (cd_read on the drive); the stack there is the card's return and this call's two bytes
; (more inside TII).
; Entry: from the card, the information block at $3000. Exit: to the program, with the stack emptied; or RTS to the
; card (signature: A the byte, X and Y the loop's, flags from the compare; failed read: A the card's answer).
IplProgram = $3000 ; the information block (sector 1): the program's sector, count, load and run addresses
IplBanks = $3008 ; the banks for MPR2 to MPR6, from the card's CD RAM base
IplFlags = $300D ; the loader's working flags
IplDataSector = $300E ; the data's sector: high, middle, low
IplDataMode = $3011 ; the third data read's sector count; 0 or $22 and up: no data
IplDataAddress = $3012 ; the third data read's address
IplAdpcmSector = $3014 ; the ADPCM's sector: high, middle, low
IplAdpcmCount = $3017 ; the ADPCM's sector count, 1 to $20
IplAdpcmRate = $3018 ; the ADPCM's rate byte
CardArg221F = $221F ; the card's RAM: arguments the loader writes for the card
IplStackCopy = $DFE0 ; where the last read's instructions are copied to and run
IplLastRead = $2CC0 ; the instructions so copied: IplStart's last read
CardRamBase = $FFF5 ; the card's byte naming its first CD RAM bank
IplStart:
    cly
    ldx #$4A
    lda #$20
    sta z:zBiosBl
    lda #$30
    sta z:zBiosBh
    lda #$B6
    sta z:zBiosCl
    lda #$DF
    sta z:zBiosCh
IplStart_at1:
    lda (zBiosBl),y
    cmp (zBiosCl),y
    beq IplStart_at0
    rts
IplStart_at0:
    iny
    dex
    bne IplStart_at1
    lda a:IplFlags
    bne IplStart_at2
    lda #$60
IplStart_at2:
    sta z:zTemp00
    bbs0 zTemp00,IplStart_at3
    bra IplStart_at4
IplStart_at3:
    jsr BiosExDspoff
    lda a:IplDataMode
    beq IplStart_at5
    cmp #$22
    bcc IplStart_at6
IplStart_at5:
    bra IplStart_at7
IplStart_at6:
    tii IplDataSector,zBiosCl+ZeroPage,$0003
    stz z:zBiosDh
    stz z:zBiosBl
    lda #$38
    sta z:zBiosBh
    stz z:zBiosAl
    lda #$02
    sta z:zBiosAh
    jsr BiosCdRead
    cmp #$00
    beq IplStart_at8
IplStart_at7:
    bra IplStart_at9
IplStart_at8:
    lda #$10
    sta a:CardArg221F+3
    sta a:CardArg221F+6
    lda #$00
    sta a:CardArg221F+1
    lda #$38
    sta a:CardArg221F+2
    lda #$00
    sta a:CardArg221F+4
    lda #$39
    sta a:CardArg221F+5
    lda #$02
    sta a:CardArg221F
    lda a:IplDataSector+2
    clc
    adc #$01
    sta z:zBiosDl
    lda a:IplDataSector+1
    adc #$00
    sta z:zBiosCh
    lda a:IplDataSector
    adc #$00
    sta z:zBiosCl
    lda #$FF
    sta z:zBiosDh
    lda #$01
    sta z:zBiosAl
    stz z:zBiosBl
    stz z:zBiosBh
    jsr BiosCdRead
    cmp #$00
    beq IplStart_at10
IplStart_at4:
    bra IplStart_at11
IplStart_at10:
    lda a:IplDataSector+2
    clc
    adc #$02
    sta z:zBiosDl
    lda a:IplDataSector+1
    adc #$00
    sta z:zBiosCh
    lda a:IplDataSector
    adc #$00
    sta z:zBiosCl
    lda #$FF
    sta z:zBiosDh
    lda a:IplDataMode
    sta z:zBiosAl
    lda a:IplDataAddress
    sta z:zBiosBl
    lda a:IplDataAddress+1
    sta z:zBiosBh
    jsr BiosCdRead
    cmp #$00
    beq IplStart_at11
IplStart_at9:
    rmb0 z:zTemp00
IplStart_at11:
    bbr1 zTemp00,IplStart_at12
    tii IplAdpcmSector,zBiosCl+ZeroPage,$0003
    lda a:IplAdpcmCount
    beq IplStart_at12
    cmp #$21
    bcs IplStart_at12
    sta z:zBiosAl
    stz z:zBiosDh
    stz z:zBiosBl
    stz z:zBiosBh
    jsr BiosAdTrans
    cmp #$00
    bne IplStart_at12
    bbs6 zTemp00,IplStart_at12
    stz z:zBiosBl
    stz z:zBiosBh
    stz z:zBiosAl
    lda a:IplAdpcmCount
    asl a
    asl a
    asl a
    bne IplStart_at13
    lda #$FF
    sta z:zBiosAl
IplStart_at13:
    sta z:zBiosAh
    lda a:IplAdpcmRate
    sta z:zBiosDh
    lda z:zTemp00
    and #$80
    sta z:zBiosDl
    jsr BiosAdPlay
IplStart_at12:
    bbs5 zTemp00,IplStart_at14
    bbr0 zTemp00,IplStart_at14
    jsr BiosExBgon
IplStart_at14:
    lda #$FF
    tam #$01
    lda #$F8
    tam #$02
    lda a:IplBanks
    clc
    adc a:CardRamBase
    tam #$04
    lda a:IplBanks+1
    clc
    adc a:CardRamBase
    tam #$08
    lda a:IplBanks+2
    clc
    adc a:CardRamBase
    tam #$10
    lda a:IplBanks+3
    clc
    adc a:CardRamBase
    tam #$20
    lda a:IplBanks+4
    clc
    adc a:CardRamBase
    tam #$40
    tii IplProgram,zTemp01+ZeroPage,$0008
    lda z:zTemp04
    bne IplStart_at15
    clc
    lda z:zTemp07
    adc #$00
    sta z:zTemp07
    lda z:zTemp08
    adc #$30
    sta z:zTemp08
    bra IplStart_at16
IplStart_at15:
    tii zTemp01+ZeroPage,zBiosCl+ZeroPage,$0003
    lda #$01
    sta z:zBiosDh
    lda z:zTemp04
    sta z:zBiosAl
    stz z:zBiosAh
    lda #$FF
    cmp z:zTemp05
    lda #$2F
    sbc z:zTemp06
    bcc IplStart_at17
    tii IplLastRead,IplStackCopy,$0015
    jmp IplStackCopy
IplStart_at17:
    tii zTemp05+ZeroPage,zBiosBl+ZeroPage,$0002
    jsr BiosCdRead
    cmp #$00
    bne IplStart_at18
IplStart_at16:
    ldx #$FF
    txs
    jmp (zTemp07+ZeroPage)
IplStart_at18:
    rts
