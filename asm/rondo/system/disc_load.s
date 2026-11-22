; @routine 0048:424 BackupRead
; @run 6A:6C24
; WHAT IT DOES: Reads the game's backup file to RAM at $7D00.
; HOW: The card's bm_read, with the file's name at $6C9A and $90 bytes.
; WAITS: no. The card's bm_read is a tail jump.
; Entry: nothing. Exit: as bm_read, entered with A 0 and Z set.
BackupRead:
    lda #$9A
    sta z:zBiosAl
    lda #$6C
    sta z:zBiosAh
    lda #$00
    sta z:zBiosBl
    lda #$7D
    sta z:zBiosBh
    lda #$90
    sta z:zBiosCl
    lda #$00
    sta z:zBiosCh
    stz z:zBiosDl
    stz z:zBiosDh
    jmp BiosBmRead

; @routine 0048:443 BackupWrite
; @run 6A:6C43
; WHAT IT DOES: Writes the game's backup file from $7D00, if there is room once the old
; one is gone.
; HOW: The card's bm_delete, its bm_free count compared with $0092, its bm_write.
; WAITS: no.
; Entry: nothing. Exit, no room: A 1, N and Z from it, C and V from the compare's subtract, X
; and Y as bm_free left them. Exit: as bm_write, entered with A 0 and Z set.
BackupWrite:
    lda #$9A
    sta z:zBiosAl
    lda #$6C
    sta z:zBiosAh
    jsr BiosBmDelete
    jsr BiosBmFree
    lda z:zBiosCl
    cmp #$92
    lda z:zBiosCh
    sbc #$00
    bmi BackupFormat_at0
    lda #$9A
    sta z:zBiosAl
    lda #$6C
    sta z:zBiosAh
    lda #$00
    sta z:zBiosBl
    lda #$7D
    sta z:zBiosBh
    lda #$90
    sta z:zBiosCl
    lda #$00
    sta z:zBiosCh
    stz z:zBiosDl
    stz z:zBiosDh
    jmp BiosBmWrite

; @routine 0048:47A BackupFormat
; @run 6A:6C7A
; WHAT IT DOES: Formats the backup memory; on failure the game stops at BackupFailed.
; HOW: The card's bm_format with the blank name at $6C8F; its answer goes to X.
; WAITS: yes, only on failure: it then spins in BackupFailed with interrupts on; the stack there is the caller's return and this call's two bytes.
; Entry: nothing. Exit: as bm_format leaves them, with X its answer, N and Z from it.
BackupFormat:
    lda #$8F
    sta z:zBiosAl
    lda #$6C
    sta z:zBiosAh
    jsr BiosBmFormat
    tax
    bne BackupFailed
    rts
BackupFormat_at0:
    lda #$01
    rts

; @routine 0048:48C BackupFailed
; @run 6A:6C8C
; WHAT IT DOES: Where the game stops when the backup memory cannot be formatted.
; HOW: Jumps to itself; interrupts go on.
; WAITS: yes, for ever: a jump to itself with interrupts on; the stack is the caller's return and its call frame.
; Entry: anything. Exit: none.
BackupFailed:
    jmp BackupFailed

; @routine 0048:643 AdpcmLoad
; @run 6A:6E43
; WHAT IT DOES: Loads a sound into ADPCM memory from the disc, unless it is already there.
; HOW: Stops the ADPCM and waits on its status, waits for the drive, then the BIOS's ADPCM
; transfer from the sound's record in AdpcmRecords, until it answers zero.
; WAITS: yes, on the drive: the ADPCM status and the card's drive and transfer calls are asked until they answer; the stack there is the caller's return, this call's two bytes and the pushed sound (1).
; Entry: A sound. Exit, already there: A kept, N 0, Z 1, C 1 from the compare. Exit: A zero,
; N 0, Z 1, C 1; X, Y and V as the BIOS's last transfer leaves them. The C replays the last 32
; of the BIOS's calls.
AdpcmLoad:
    cmp z:zMusicWanted
    beq SaveVramToRam_at3
    sta z:zMusicWanted
    pha
    jsr BiosAdStop
AdpcmLoad_at0:
    jsr BiosAdStat
    tay
    bne AdpcmLoad_at0
    pla
    asl a
    asl a
    clc
    adc #<AdpcmRecords
    sta z:zTemp00
    cla
    adc #>AdpcmRecords
    sta z:zTemp01
AdpcmLoad_at3:
    cla
    jsr BiosCdStat
    cmp #$00
    beq AdpcmLoad_at1
AdpcmLoad_at2:
    jsr BiosCdPause
    cmp #$00
    bne AdpcmLoad_at2
    bra AdpcmLoad_at3
AdpcmLoad_at1:
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
    iny
    lda (zTemp00),y
    sta z:zBiosAl
    stz z:zBiosDh
    stz z:zBiosBh
    stz z:zBiosBl
    jsr BiosAdTrans
    cmp #$00
    bne AdpcmLoad_at3
    stz a:BiosAdpcmBusy
    rts

; @routine 0048:69D AdpcmLoadFixed
; @run 6A:6E9D
; WHAT IT DOES: Loads $20 sectors of ADPCM from $20 sectors past disc address $0010AB, with the
; track's start taken off as a record's is.
; HOW: Waits for the drive; the address worked out a byte at a time from its constants; the
; BIOS's ADPCM transfer, until it answers zero.
; WAITS: yes, on the drive: the card's drive and transfer calls are asked until they answer; the stack there is the caller's return and this call's two bytes.
; Entry: nothing. Exit: A zero, N 0, Z 1, C 1; X, Y and V as the BIOS's last transfer leaves
; them.
AdpcmLoadFixed:
    cla
    jsr BiosCdStat
    cmp #$00
    beq AdpcmLoadFixed_at0
AdpcmLoadFixed_at1:
    jsr BiosCdPause
    cmp #$00
    bne AdpcmLoadFixed_at1
    bra AdpcmLoadFixed
AdpcmLoadFixed_at0:
    cly
    lda #$AB
    sec
    sbc #$32
    sta z:zBiosDl
    iny
    lda #$10
    sbc #$0F
    sta z:zBiosCh
    iny
    lda #$00
    sbc #$00
    sta z:zBiosCl
    lda z:zBiosDl
    clc
    adc #$20
    sta z:zBiosDl
    cla
    adc z:zBiosCh
    sta z:zBiosCh
    cla
    adc z:zBiosCl
    sta z:zBiosCl
    iny
    lda #$20
    sta z:zBiosAl
    stz z:zBiosDh
    stz z:zBiosBh
    stz z:zBiosBl
    jsr BiosAdTrans
    cmp #$00
    bne AdpcmLoadFixed
    stz a:BiosAdpcmBusy
    rts

; @routine 0048:6EB AdpcmPlayFromDisc
; @run 6A:6EEB
; WHAT IT DOES: Plays a sound straight from the disc as ADPCM.
; HOW: The sound's record, five bytes each in AdpcmStreamRecords; waits for the drive; the
; BIOS's continuous ADPCM play in mode $0E, until it answers zero.
; WAITS: yes, on the drive: the card's drive and ADPCM play calls are asked until they answer; the stack there is the caller's return and this call's two bytes.
; Entry: A sound. Exit: A zero, N 0, Z 1, C 1; Y 4 unless the BIOS changed it; X and V as the
; BIOS leaves them.
AdpcmPlayFromDisc:
    sta z:zBiosAl
    lda #$05
    sta z:zBiosBl
    jsr BiosMaMul8u
    lda #<AdpcmStreamRecords
    clc
    adc z:zBiosCl
    sta z:zTemp00
    lda #>AdpcmStreamRecords
    adc z:zBiosCh
    sta z:zTemp01
AdpcmPlayFromDisc_at2:
    cla
    jsr BiosCdStat
    cmp #$00
    beq AdpcmPlayFromDisc_at0
AdpcmPlayFromDisc_at1:
    jsr BiosCdPause
    cmp #$00
    bne AdpcmPlayFromDisc_at1
    bra AdpcmPlayFromDisc_at2
AdpcmPlayFromDisc_at0:
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
    iny
    lda (zTemp00),y
    sta z:zBiosAl
    iny
    lda (zTemp00),y
    sta z:zBiosAh
    stz z:zBiosBl
    lda #$0E
    sta z:zBiosDh
    jsr BiosAdCplay
    cmp #$00
    bne AdpcmPlayFromDisc_at2
    stz a:BiosAdpcmBusy
    rts

; @routine 0048:743 CdPlayOnce
; @run 6A:6F43
; WHAT IT DOES: Plays a CD track once.
; HOW: Mode $42 for the BIOS's play.
; WAITS: yes, on the drive: the card's play is asked until it answers zero; the stack there is the caller's return, this call's two bytes and X and Y pushed (2).
; Entry: A track. Exit: A 0 from the BIOS's fade, C and V as its play left them, X and Y kept, N
; and Z from X. The BIOS's play is tried until it succeeds; the C replays the last 32 tries' exits.
CdPlayOnce:
    phx
    phy
    ldx #$42
    stx z:zCdPlayMode
    bra CdPlayRepeat_at0

; @routine 0048:74B CdPlayAgain
; @run 6A:6F4B
; WHAT IT DOES: Plays the last track again, repeating.
; HOW: zCdTrack into CdPlayRepeat.
; WAITS: yes, as CdPlayOnce.
; Entry: nothing. Exit: as CdPlayRepeat.
CdPlayAgain:
    lda z:zCdTrack

; @routine 0048:74D CdPlayRepeat
; @run 6A:6F4D
; WHAT IT DOES: Plays a CD track, repeating.
; HOW: Mode $41 for the BIOS's play.
; WAITS: yes, as CdPlayOnce.
; Entry: A track. Exit: as CdPlayOnce.
CdPlayRepeat:
    phx
    phy
    ldx #$41
    stx z:zCdPlayMode
CdPlayRepeat_at0:
    sta z:zCdTrack
    sta z:zBiosAl
    lda #$06
    sta z:zBiosBl
    jsr BiosMaMul8u
    lda #<CdTrackTable
    clc
    adc z:zBiosCl
    sta z:zTemp00
    lda #>CdTrackTable
    adc z:zBiosCh
    sta z:zTemp01
CdPlayRepeat_at1:
    lda #$40
    sta z:zBiosBh
    lda (zTemp00)
    sta z:zBiosAl
    ldy #$01
    lda (zTemp00),y
    sta z:zBiosAh
    iny
    lda (zTemp00),y
    sta z:zBiosBl
    lda z:zCdPlayMode
    sta z:zBiosDh
    iny
    lda (zTemp00),y
    sta z:zBiosCl
    iny
    lda (zTemp00),y
    sta z:zBiosCh
    iny
    lda (zTemp00),y
    sta z:zBiosDl
    jsr BiosCdPlay
    tax
    bne CdPlayRepeat_at1
    cla
    jsr BiosCdFade
    ply
    plx
CdPlayRepeat_at2:
    rts

; @routine 0048:79E CdSearchTrack
; @run 6A:6F9E
; WHAT IT DOES: Seeks to a CD track, so that its play will start at once. $FF does
; nothing.
; HOW: The track's entry in CdTrackTable; the BIOS's search from its first three bytes, until it
; answers zero.
; WAITS: yes, on the drive: the card's search is asked until it answers zero; the stack there is the caller's return, this call's two bytes and X and Y pushed (2).
; Entry: A track. Exit, $FF: A kept, N 0, Z 1, C 1. Exit: A zero, X and Y kept, N and Z
; from X; C and V as the BIOS leaves them.
CdSearchTrack:
    cmp #$FF
    beq CdPlayRepeat_at2
    phx
    phy
    sta z:zCdTrack
    sta z:zBiosAl
    lda #$06
    sta z:zBiosBl
    jsr BiosMaMul8u
    lda #<CdTrackTable
    clc
    adc z:zBiosCl
    sta z:zTemp00
    lda #>CdTrackTable
    adc z:zBiosCh
    sta z:zTemp01
CdSearchTrack_at0:
    lda #$40
    sta z:zBiosBh
    lda (zTemp00)
    sta z:zBiosAl
    ldy #$01
    lda (zTemp00),y
    sta z:zBiosAh
    iny
    lda (zTemp00),y
    sta z:zBiosBl
    jsr BiosCdSearch
    tax
    bne CdSearchTrack_at0
    ply
    plx
    rts

; @routine 0048:7D8 LoadSceneScript
; @run 6A:6FD8
; WHAT IT DOES: Loads a scene: runs its list of ops, which map banks, read sectors from
; the disc, and call the scene overlay, until an op it does not know ends it.
; HOW: The scene's record, five bytes each in SceneRecords, holds the disc address and at 3
; and 4 the list.
; WAITS: yes, on the drive: through SceneReadSectors, and on the overlay calls; the stack there is the caller's return and this call's two bytes (and an overlay call's).
; Entry: A scene. Exit: A the last op, N and Z from comparing it with $80, C 1; X, Y and V as
; the last op left them. Clobbers zTemp00-02, zTemp08-09.
LoadSceneScript:
    sta z:zTemp02
    sta z:zBiosAl
    lda #$05
    sta z:zBiosBl
    jsr BiosMaMul8u
    lda #<SceneRecords
    clc
    adc z:zBiosCl
    sta z:zTemp00
    lda #>SceneRecords
    adc z:zBiosCh
    sta z:zTemp01
    ldy #$03
    lda (zTemp00),y
    sta z:zTemp08
    iny
    lda (zTemp00),y
    sta z:zTemp09
LoadSceneScript_at2:
    lda (zTemp08)
    bmi LoadSceneScript_at0
    beq LoadSceneScript_at1
    jmp SceneReadSectors
LoadSceneScript_at1:
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
LoadSceneScript_at8:
    clc
    adc z:zTemp08
    sta z:zTemp08
    cla
    adc z:zTemp09
    sta z:zTemp09
    bra LoadSceneScript_at2
LoadSceneScript_at0:
    cmp #$C0
    beq LoadSceneScript_at3
    cmp #$83
    beq LoadSceneScript_at4
    cmp #$82
    beq LoadSceneScript_at5
    cmp #$81
    beq LoadSceneScript_at6
    cmp #$80
    beq LoadSceneScript_at7
    rts
LoadSceneScript_at3:
    ldy #$01
    lda (zTemp08),y
    tax
    iny
    lda (zTemp08),y
    stx z:zTemp08
    sta z:zTemp09
    bra LoadSceneScript_at2
LoadSceneScript_at6:
    lda #$04
    clc
    adc z:zRamBaseBank
    tam #$20
    tii $C000,$A000,$1400
    lda #$01
    bra LoadSceneScript_at8
LoadSceneScript_at7:
    clx
    jsr SceneCallOverlay
    ldx #$04
    jsr SceneCallOverlay
    lda #$01
    bra LoadSceneScript_at8
LoadSceneScript_at5:
    lda z:zTemp02
    cmp #$02
    beq LoadSceneScript_at9
    cmp #$58
    beq LoadSceneScript_at9
    cmp #$5A
    beq LoadSceneScript_at9
    cmp #$5C
    beq LoadSceneScript_at9
    cmp #$0E
    beq LoadSceneScript_at10
    cmp #$0C
    beq LoadSceneScript_at11
    cmp #$0A
    beq LoadSceneScript_at12
    cmp #$08
    beq LoadSceneScript_at13
    ldx #$10
    bra LoadSceneScript_at14
LoadSceneScript_at9:
    ldx #$0C
    bra LoadSceneScript_at14
LoadSceneScript_at10:
    ldx #$06
    bra LoadSceneScript_at14
LoadSceneScript_at11:
    ldx #$0A
    bra LoadSceneScript_at14
LoadSceneScript_at12:
    ldx #$08
    bra LoadSceneScript_at14
LoadSceneScript_at4:
    ldx #$02
    bra LoadSceneScript_at14
LoadSceneScript_at13:
    ldx #$0E
LoadSceneScript_at14:
    jsr SceneCallOverlay
    lda #$01
    jmp LoadSceneScript_at8

; @routine 0049:0B1 SceneReadSectors
; @run 6A:70B1
; WHAT IT DOES: A scene op from 1 to $7F: reads sectors from the disc to the place in RAM
; the op gives, at an offset from the scene's own disc address; then the ops go on.
; HOW: The op indexes SceneReadDestinations; the op's second byte is the sector offset and its
; third the count; the BIOS's read, asked again until it answers zero, and each time the table
; is indexed by what A then holds.
; WAITS: yes, on the drive: the card's read is asked until it answers zero; the stack there is the caller's return and this call's two bytes.
; Entry: A the op. Exit: as LoadSceneScript.
SceneReadSectors:
    asl a
    tay
    lda a:SceneReadDestinations,y
    sta z:zBiosBl
    lda a:SceneReadDestinations+1,y
    sta z:zBiosBh
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
    bne SceneReadSectors
    lda #$03
    jmp LoadSceneScript_at8

; @routine 0049:100 SceneCallOverlay
; @run 6A:7100
; WHAT IT DOES: Maps the scene overlay and runs its entry at $8006 with a pointer from
; SceneCallTable.
; HOW: The pointer goes to zProduct1-2, where that entry takes it; Y 0.
; WAITS: depends on the overlay it runs (an original-code tail: a jump to $8006).
; Entry: X the table's offset. Exit: as the overlay leaves them.
SceneCallOverlay:
    lda #$04
    clc
    adc z:zRamBaseBank
    tam #$10
    lda a:SceneCallTable,x
    sta z:zProduct1
    lda a:SceneCallTable+1,x
    sta z:zProduct2
    cly
    jmp FrameOverlay8006
