; @routine 0002:000 BootStart
; @run F8:3000
; WHAT IT DOES: The boot program: writes zeros to VRAM $1100 to $110F and the VDC's control register's high byte,
; fills VRAM from 0 with 4096 words of $0110, asks the card's cd_base twice ($81 and $82 with 2 and $22, 1 and 2),
; then the card's version. Below 3.0 it maps the CD RAM banks from $FFF5, reads 20 sectors from $23 to $4000 until
; cd_read answers zero, and jumps to the old-card program at $4000. From 3.0, ex_memopen; when it has no RAM to give
; the old-card path is taken all the same; otherwise its bank goes to zRamBaseBank, LoadBootBlocks reads the
; kernel by the first record of the load table, the bank above it goes to MPR2 and the next to MPR3, and it jumps to
; the kernel's entry at $4000.
; HOW: ST0, ST1 and ST2 to the VDC; the card's entries with the registers its code is handed, built by shims.
; WAITS: yes, in the card's entries (cd_read until it answers zero).
; Entry: from the boot block. Exit: never: a jump to $4000.
BootStart:
    st0 #$00
    st1 #$00
    st2 #$11
    st0 #$02
    lda #$10
BootStart_at0:
    st1 #$00
    st2 #$00
    dec a
    bne BootStart_at0
    st0 #$05
    st2 #$00
    st0 #$00
    st1 #$00
    st2 #$00
    st0 #$02
    clx
    ldy #$0F
BootStart_at1:
    st1 #$10
    st2 #$01
    inx
    bne BootStart_at1
    dey
    bpl BootStart_at1
    lda #$81
    sta z:zBiosBh
    lda #$02
    sta z:zBiosAl
    lda #$01
    sta z:zBiosCl
    jsr $E006
    lda #$82
    sta z:zBiosBh
    lda #$22
    sta z:zBiosAl
    lda #$02
    sta z:zBiosCl
    jsr $E006
    jsr BiosExGetver
    cpx #$03
    bcs BootStart_at2
BootStart_at5:
    lda a:$FFF5
    sta z:zRamBaseBank
    tam #$04
    inc a
    tam #$08
    inc a
    tam #$10
    inc a
    tam #$20
BootStart_at3:
    lda #$00
    sta z:zBiosCl
    lda #$00
    sta z:zBiosCh
    lda #$23
    sta z:zBiosDl
    lda #$01
    sta z:zBiosDh
    lda #$00
    sta z:zBiosBl
    lda #$40
    sta z:zBiosBh
    lda #$14
    sta z:zBiosAl
    jsr BiosCdRead
    tax
    bne BootStart_at3
    jmp BootStart_at4
BootStart_at2:
    jsr $E0DE
    bcs BootStart_at5
    sta z:zRamBaseBank
    cla
    jsr LoadBootBlocks
    lda z:zRamBaseBank
    clc
    adc #$01
    tam #$04
    inc a
    tam #$08
BootStart_at4:
    jmp $4000

; @routine 0002:09C LoadBootBlocks
; @run F8:309C
; WHAT IT DOES: Reads the blocks of the load table's record A (five bytes a record at $312F: the first sector, three
; bytes from the disc's start, then the address of the record's ops). The ops, until one with bit 7 set: 0 sets
; MPR4 to MPR6 from the next three bytes above zRamBaseBank (four bytes); any other reads, by the card's cd_read
; until it answers zero, the count of sectors in its third byte from the record's sector plus its second byte, to
; $8000 under those banks (three bytes).
; HOW: ma_mul8u for the record's offset (A times 5), zTemp00-01 the record, zTemp08-09 the op; the sector less
; $F32 into zBiosCl, Ch and Dl, zBiosDh 1, zBiosB $8000, zBiosAl the count.
; WAITS: yes, in the card's cd_read.
; Entry: A the record. Exit: A the op that ended the list, N and Z from it; Y 4 for an empty list, 3 after a bank op,
; as cd_read left it after a read op (nothing loads Y again before the RTS); C and V from
; the last pointer add; X as the card's last call left it (0 after a read).
LoadBootBlocks:
    sta z:zBiosAl
    lda #$05
    sta z:zBiosBl
    jsr BiosMaMul8u
    lda #$2F
    clc
    adc z:zBiosCl
    sta z:zTemp00
    lda #$31
    adc z:zBiosCh
    sta z:zTemp01
    ldy #$03
    lda (zTemp00),y
    sta z:zTemp08
    iny
    lda (zTemp00),y
    sta z:zTemp09
LoadBootBlocks_at2:
    lda (zTemp08)
    bmi LoadBootBlocks_at0
    bne LoadBootBlocks_at1
    ldy #$01
    lda (zTemp08),y
    clc
    adc z:zRamBaseBank
    tam #$10
    iny
    lda (zTemp08),y
    clc
    adc z:zRamBaseBank
    tam #$20
    iny
    lda (zTemp08),y
    clc
    adc z:zRamBaseBank
    tam #$40
    lda #$04
LoadBootBlocks_at3:
    clc
    adc z:zTemp08
    sta z:zTemp08
    cla
    adc z:zTemp09
    sta z:zTemp09
    bra LoadBootBlocks_at2
LoadBootBlocks_at0:
    rts
LoadBootBlocks_at1:
    cly
    lda (zTemp00),y
    sec
    sbc #$32
    sta z:zBiosDl
    iny
    lda (zTemp00),y
    sbc #$0F
    sta z:zBiosCh
    iny
    lda (zTemp00),y
    sbc #$00
    sta z:zBiosCl
    lda #$01
    sta z:zBiosDh
    lda #$00
    sta z:zBiosBl
    lda #$80
    sta z:zBiosBh
    ldy #$01
    lda (zTemp08),y
    clc
    adc z:zBiosDl
    sta z:zBiosDl
    cla
    adc z:zBiosCh
    sta z:zBiosCh
    cla
    adc z:zBiosCl
    sta z:zBiosCl
    iny
    lda (zTemp08),y
    sta z:zBiosAl
    jsr BiosCdRead
    tax
    bne LoadBootBlocks_at1
    lda #$03
    bra LoadBootBlocks_at3
