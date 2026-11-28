; @routine 093A:0BC BreakPieceByCount093A
; @run 70:A8BC
; WHAT IT DOES: Sound $19 and debris; counts a hit. Under the piece record's hits takes its speeds by count and moves; at them
; sets the piece's zField8F bit and becomes the record's kind, at rest.
; HOW: As BreakPieceByHits051B with the records at $B3A8.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BreakPieceByCount093AHits = $B3A7
BreakPieceByCount093A:
    lda #$19
    jsr VecSoundCommand
    lda #$01
    jsr StartDebrisBurst
    ldy a:OBJ_PIECE_PLACE,x
    inc a:OBJ_PIECE_NEXT,x
    lda a:OBJ_PIECE_NEXT,x
    cmp a:PieceRecords093A-1,y
    bcs BreakPieceByCount093A_at0
    lda a:OBJ_PIECE_NEXT,x
    dec a
    asl a
    clc
    adc a:OBJ_PIECE_PLACE,x
    tay
    lda a:PieceRecords093A+5,y
    sta a:OBJ_VX_HI,x
    lda a:PieceRecords093A+6,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    rts
BreakPieceByCount093A_at0:
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex072A
    ora z:zField8F
    sta z:zField8F
    ldy a:OBJ_PIECE_PLACE,x
    lda a:PieceRecords093A+4,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 093A:689 PictureByArgument093A
; @run 70:AE89
; WHAT IT DOES: State 0 loads the record with the frame OBJ_START_ARG, sprite set 6.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PictureByArgument093A:
    lda a:OBJ_STATE,x
    bne PictureByArgument093A_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
PictureByArgument093A_at0:
    rts

; @routine 093A:69D StartScriptAndVanish093A
; @run 70:AE9D
; WHAT IT DOES: Starts script $88 in the slot OBJ_START_ARG names, records the event and clears the object.
; HOW: StartScriptSlotKind1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartScriptAndVanish093A:
    jsr VecLoadObjectRecord
    phx
    lda a:OBJ_START_ARG,x
    tax
    lda #$88
    jsr VecStartScriptSlotKind1
    plx
    jsr VecRecordObjectEvent
    jmp VecClearObject

; @routine 093A:6B1 ContactPicture093A
; @run 70:AEB1
; WHAT IT DOES: State 0 loads the record as sprite set 6 frame $25; then checks the player against a box $14 by $40.
; HOW: ContactWithPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ContactPicture093A:
    lda a:OBJ_STATE,x
    bne ContactPicture093A_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$25
    sta a:OBJ_FRAME,x
ContactPicture093A_at0:
    lda #$14
    sta z:zTemp08
    lda #$40
    sta z:zTemp09
    jmp ContactWithPlayer

; @routine 093A:6CE BreakableWall093A
; @run 70:AECE
; WHAT IT DOES: State 0 loads the record and draws it ($AED6); later states at $AEE0.
; HOW: A branch by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BreakableWall093A:
    lda a:OBJ_STATE,x
    bne BreakableWallBroken093A
    jsr VecLoadObjectRecord

; @routine 093A:6D6 DrawWallWhole093A
; @run 70:AED6
; WHAT IT DOES: The whole wall's tile script, $0F.
; HOW: Into DrawWallScript093A.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DrawWallWhole093A:
    lda #$0F

; @routine 093A:6D8 DrawWallScript093A
; @run 70:AED8
; WHAT IT DOES: Draws the tile script A plus OBJ_START_ARG.
; HOW: A tail jump to DrawTileScript.
; WAITS: no.
; Entry: A the script's base, X object. Exit: as DrawTileScript.
DrawWallScript093A:
    clc
    adc a:OBJ_START_ARG,x
    cly
    jmp VecDrawTileScript

; @routine 093A:6E0 BreakableWallBroken093A
; @run 70:AEE0
; WHAT IT DOES: State 1: while the tiles beneath are not the wall's, redraws it; a hit of 1 or 2 shows the broken frame and waits
; $20. State 2 counts and draws script $16 plus the argument. Later states fall ($B1E4).
; HOW: A read of VRAM at the wall's column.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BreakableWallBroken093AFrames = $AF3E ; broken frame by argument
BreakableWallBroken093A:
    dec a
    bne BreakableWallBroken093A_at1
    lda a:OBJ_START_ARG,x
    asl a
    sta z:zTemp00
    jsr VecSelectVramReadAddress
    lda #$31
    clc
    adc z:zTemp00
    sta a:VDC_DATA_LO
    lda #$03
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    ldy a:VDC_DATA_HI
    cmp #$4D
    beq DrawWallWhole093A
    lda a:VDC_DATA_LO
    ldy a:VDC_DATA_HI
    cmp #$4D
    beq DrawWallWhole093A
    lda a:OBJ_HIT_TAKEN,x
    beq BreakableWallBroken093A_at0
    cmp #$03
    bcs BreakableWallBroken093A_at0
    ldy a:OBJ_START_ARG,x
    lda a:BreakableWallBroken093AFrames,y
    sta a:OBJ_FRAME,x
    lda #$20
    sta a:OBJ_PIECE_NEXT,x
    inc a:OBJ_STATE,x
BreakableWallBroken093A_at0:
    rts
BreakableWallBroken093A_at1:
    dec a
    bne BreakableWallBroken093A_at2
    dec a:OBJ_PIECE_NEXT,x
    bne BreakableWallBroken093A_at0
    lda #$16
    jsr DrawWallScript093A
    inc a:OBJ_STATE,x
BreakableWallBroken093A_at2:
    jmp FallStone093A

; @routine 093A:745 BannerSpelledOut093A
; @run 70:AF45
; WHAT IT DOES: State 0 loads the record, starts script $86 in slot 5, clears the text area ($B001) and takes the text's address
; by argument. State 1 writes one character every eighth frame until the text's zero, sounding $3B except for
; Shift-JIS $8140. Then counts $40 and clears itself.
; HOW: Shift-JIS pairs read through zTemp00.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BannerSpelledOut093ATexts = $AFC3 ; text addresses by argument
BannerSpelledOut093A:
    lda a:OBJ_STATE,x
    bne BannerSpelledOut093A_at0
    jsr VecLoadObjectRecord
    phx
    lda #$86
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    jsr ClearBannerArea093A
    lda #$D0
    sta a:OBJ_X_LO,x
    lda #$16
    sta a:OBJ_Y_LO,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:BannerSpelledOut093ATexts,y
    sta a:OBJ_PIECE_PLACE,x
    lda a:BannerSpelledOut093ATexts+1,y
    sta a:OBJ_SWOOP_SPEED_LO,x
    rts
BannerSpelledOut093A_at0:
    dec a
    bne BannerSpelledOut093A_at6
    lda z:zWaveCounter
    and #$07
    bne BannerSpelledOut093A_at5
    lda a:OBJ_PIECE_PLACE,x
    sta z:zTemp00
    lda a:OBJ_SWOOP_SPEED_LO,x
    sta z:zTemp01
    lda (zTemp00)
    beq BannerSpelledOut093A_at4
    cmp #$81
    bne BannerSpelledOut093A_at1
    ldy #$01
    lda (zTemp00),y
    cmp #$40
    beq BannerSpelledOut093A_at2
BannerSpelledOut093A_at1:
    lda #$3B
    jsr VecSoundCommand
BannerSpelledOut093A_at2:
    jsr WriteBannerGlyph093A
    inc a:OBJ_PIECE_NEXT,x
    lda a:OBJ_PIECE_PLACE,x
    clc
    adc #$02
    sta a:OBJ_PIECE_PLACE,x
    bcc BannerSpelledOut093A_at3
    inc a:OBJ_SWOOP_SPEED_LO,x
BannerSpelledOut093A_at3:
    rts
BannerSpelledOut093A_at4:
    lda #$40
    sta a:OBJ_SWOOP_SPEED_HI,x
    inc a:OBJ_STATE,x
BannerSpelledOut093A_at5:
    rts
BannerSpelledOut093A_at6:
    dec a:OBJ_SWOOP_SPEED_HI,x
    bne BannerSpelledOut093A_at5
    jmp VecClearObject

; @routine 093B:001 ClearBannerArea093A
; @run 70:B001
; WHAT IT DOES: Clears $1000 VRAM words from $0C00.
; HOW: ST1 and ST2 of zero, 16 times 64.
; WAITS: no.
; Entry: X kept. Exit: A as SelectVramData, Y 0.
ClearBannerArea093A:
    jsr VecSelectVramWriteAddress
    lda #$00
    sta a:VDC_DATA_LO
    lda #$0C
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    phx
    ldy #$10
ClearBannerArea093A_at0:
    ldx #$40
ClearBannerArea093A_at1:
    st1 #$00
    st2 #$00
    dex
    bne ClearBannerArea093A_at1
    dey
    bne ClearBannerArea093A_at0
    plx
    rts

; @routine 093B:022 WriteBannerGlyph093A
; @run 70:B022
; WHAT IT DOES: Reads the card's glyph for the Shift-JIS pair at (zTemp00), shifts its lines three pixels, and writes it at the
; VRAM place for OBJ_PIECE_NEXT, its shadow plane half a line on.
; HOW: ex_getfnt into StageMaskBuffer's page, then the VDC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WriteBannerGlyph093APlaces = $B0AF ; VRAM address by character
WriteBannerGlyph093A:
    cly
    lda (zTemp00),y
    sta z:zBiosAh
    iny
    lda (zTemp00),y
    sta z:zBiosAl
    lda #$42
    sta z:zBiosBl
    lda #$3B
    sta z:zBiosBh
    lda #$01
    sta z:zBiosDh
    phx
    jsr BiosExGetfnt
    plx
    jsr VecSelectVramWriteAddress
    lda a:OBJ_PIECE_NEXT,x
    asl a
    tay
    lda a:WriteBannerGlyph093APlaces,y
    sta a:VDC_DATA_LO
    lda a:WriteBannerGlyph093APlaces+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    stz a:StageMaskBuffer
    stz a:StageMaskBuffer+1
    phx
    clx
WriteBannerGlyph093A_at0:
    lsr a:StageMaskBuffer,x
    ror a:StageMaskBuffer+1,x
    lsr a:StageMaskBuffer,x
    ror a:StageMaskBuffer+1,x
    lsr a:StageMaskBuffer,x
    ror a:StageMaskBuffer+1,x
    inx
    inx
    cpx #$20
    bne WriteBannerGlyph093A_at0
    clx
WriteBannerGlyph093A_at1:
    lda a:StageMaskBuffer+1,x
    sta a:VDC_DATA_LO
    lda a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$20
    bne WriteBannerGlyph093A_at1
    st1 #$00
    st2 #$00
    inx
    inx
WriteBannerGlyph093A_at2:
    lda a:StageMaskBuffer-34,x
    lsr a
    pha
    lda a:StageMaskBuffer-33,x
    ror a
    sta a:VDC_DATA_LO
    pla
    sta a:VDC_DATA_HI
    inx
    inx
    cpx #$40
    bne WriteBannerGlyph093A_at2
WriteBannerGlyph093A_at3:
    st1 #$00
    st2 #$00
    inx
    inx
    cpx #$80
    bne WriteBannerGlyph093A_at3
    plx
    rts

; @routine 093B:0CF ExitWhenDoorHit093A
; @run 70:B0CF
; WHAT IT DOES: State 0 loads the record. With the door ready and not held, a hit of 1 or 2 sets zField8C to $80 plus
; OBJ_START_ARG and sub-step 3.
; HOW: As FadeOutWhenDoorHit051B without the fade.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ExitWhenDoorHit093A:
    lda a:OBJ_STATE,x
    bne ExitWhenDoorHit093A_at0
    jmp VecLoadObjectRecord
ExitWhenDoorHit093A_at0:
    lda a:StageDoorHeld
    eor #$01
    and a:PlayerDoorReady
    beq ExitWhenDoorHit093A_at1
    lda a:OBJ_HIT_TAKEN,x
    beq ExitWhenDoorHit093A_at1
    cmp #$03
    bcs ExitWhenDoorHit093A_at1
    lda #$80
    ora a:OBJ_START_ARG,x
    sta z:zField8C
    lda #$03
    sta z:zSubStep
ExitWhenDoorHit093A_at1:
    rts

; @routine 093B:104 CrumblingColumn093A
; @run 70:B104
; WHAT IT DOES: State 0 loads the record and draws it whole ($B10C); later states at $B119.
; HOW: A branch by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CrumblingColumn093A:
    lda a:OBJ_STATE,x
    bne CrumbleColumnByTile093A
    jsr VecLoadObjectRecord

; @routine 093B:10C DrawColumnWhole093A
; @run 70:B10C
; WHAT IT DOES: Four pieces left: draws tile script 4 plus OBJ_START_ARG.
; HOW: A tail jump to DrawTileScript.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DrawColumnWhole093A:
    lda #$04
    sta a:OBJ_PIECE_NEXT,x
    clc
    adc a:OBJ_START_ARG,x
    cly
    jmp VecDrawTileScript

; @routine 093B:119 CrumbleColumnByTile093A
; @run 70:B119
; WHAT IT DOES: State 1: redraws it whole while the tile beneath is $15 or more; below X $E0 goes on with a wait of $10. State 2
; crumbles one piece per wait with sound $91, then drops five kind $37 stones and records the event. State 3 draws
; the stump and clears itself.
; HOW: A read of VRAM under the column.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CrumbleColumnByTile093APlaces = $B19F ; VRAM address by argument
CrumbleColumnByTile093A:
    dec a
    bne CrumbleColumnByTile093A_at0
    lda a:OBJ_START_ARG,x
    lsr a
    and #$06
    tay
    jsr VecSelectVramReadAddress
    lda a:CrumbleColumnByTile093APlaces,y
    sta a:VDC_DATA_LO
    lda a:RichterClimbReachFacings,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    ldy a:VDC_DATA_HI
    cmp #$15
    bcs DrawColumnWhole093A
    lda a:OBJ_X_HI,x
    bne CrumbleColumnByTile093A_at5
    lda a:OBJ_X_LO,x
    cmp #$E0
    bcs CrumbleColumnByTile093A_at5
    inc a:OBJ_STATE,x
    bra CrumbleColumnByTile093A_at4
CrumbleColumnByTile093A_at0:
    dec a
    bne CrumbleColumnByTile093A_at6
    dec a:OBJ_PIECE_PLACE,x
    bne CrumbleColumnByTile093A_at5
    dec a:OBJ_PIECE_NEXT,x
    lda a:OBJ_PIECE_NEXT,x
    beq CrumbleColumnByTile093A_at1
    clc
    adc a:OBJ_START_ARG,x
    cly
    jsr VecDrawTileScript
CrumbleColumnByTile093A_at1:
    lda #$91
    jsr VecSoundCommand
    lda a:OBJ_PIECE_NEXT,x
    bne CrumbleColumnByTile093A_at4
    lda #$05
    sta z:zTemp00
CrumbleColumnByTile093A_at2:
    jsr VecFindFreeSlotFrom30
    bcc CrumbleColumnByTile093A_at3
    lda #$37
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    dec z:zTemp00
    bpl CrumbleColumnByTile093A_at2
CrumbleColumnByTile093A_at3:
    inc a:OBJ_STATE,x
    jmp VecRecordObjectEvent
CrumbleColumnByTile093A_at4:
    lda #$10
    sta a:OBJ_PIECE_PLACE,x
CrumbleColumnByTile093A_at5:
    rts
CrumbleColumnByTile093A_at6:
    lda a:OBJ_START_ARG,x
    cly
    jsr VecDrawTileScript
    jmp VecClearObject

; @routine 093B:1A5 FallingStone093A
; @run 70:B1A5
; WHAT IT DOES: State 0 loads the record as sprite set 6, its frame and place offsets and Y speed by argument; later falls.
; HOW: Tables by argument.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallingStone093AXOffsets = $B208 ; X offset by stone
FallingStone093AYOffsets = $B20E ; Y offset by stone
FallingStone093ASpeeds = $B214 ; VY low by stone
FallingStone093A:
    lda a:OBJ_STATE,x
    bne FallStone093A
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda a:OBJ_START_ARG,x
    tay
    clc
    adc #$14
    sta a:OBJ_FRAME,x
    lda a:FallingStone093AXOffsets,y
    clc
    adc a:OBJ_X_LO,x
    sta a:OBJ_X_LO,x
    lda #$FF
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x
    lda a:FallingStone093AYOffsets,y
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    bcc FallingStone093A_at0
    inc a:OBJ_Y_HI,x
FallingStone093A_at0:
    lda a:FallingStone093ASpeeds,y
    sta a:OBJ_VY_LO,x
FallingStone093A_at1:
    rts

; @routine 093B:1E4 FallStone093A
; @run 70:B1E4
; WHAT IT DOES: Checks the player (box $0C by $1D), falls a little faster, and is cleared past the first screen.
; HOW: ContactWithPlayer, ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallStone093A:
    lda #$0C
    sta z:zTemp08
    lda #$1D
    sta z:zTemp09
    jsr ContactWithPlayer
    lda a:OBJ_VY_LO,x
    clc
    adc #$14
    sta a:OBJ_VY_LO,x
    bcc FallStone093A_at0
    inc a:OBJ_VY_HI,x
FallStone093A_at0:
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    beq FallingStone093A_at1
    jmp VecClearObject

; @routine 093B:21A WavingBackdrop093A
; @run 70:B21A
; WHAT IT DOES: State 0 with argument 0 starts list channel 3 with list 0; loads the record. Then draws the wave of rows by the
; second plane's scroll ($B24C).
; HOW: The scroll of plane 1 doubled, plus 8, as the phase.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WavingBackdrop093A:
    lda a:OBJ_STATE,x
    bne WavingBackdrop093A_at1
    lda a:OBJ_START_ARG,x
    bne WavingBackdrop093A_at0
    phx
    cla
    ldx #$05
    jsr VecStartListChannelKind3
    plx
WavingBackdrop093A_at0:
    jsr VecLoadObjectRecord
WavingBackdrop093A_at1:
    lda a:ScrollXFrac+1
    sec
    sbc a:ScrollStepXLo+1
    sta z:zTemp00
    lda a:ScrollXLo+1
    sbc a:ScrollStepXHi+1
    sta z:zTemp01
    asl z:zTemp00
    lda z:zTemp01
    adc #$08
    and #$1F
    jsr CopyWaveColumns093A
    rts

; @routine 093B:24C CopyWaveColumns093A
; @run 70:B24C
; WHAT IT DOES: Copies twelve eight-word runs between VRAM addresses: from the pairs at the table for the phase's low bits, to
; the table of destinations turned by the phase's high bits.
; HOW: The VDC's read and write addresses per run.
; WAITS: no.
; Entry: A the phase. Exit: as the last instruction left them.
CopyWaveColumns093ADestinations = $B2BD ; VRAM address words
CopyWaveColumns093ASources = $B2D5 ; source list addresses by phase
CopyWaveColumns093A:
    pha
    and #$07
    asl a
    tay
    lda a:CopyWaveColumns093ASources,y
    sta z:zTemp00
    lda a:CopyWaveColumns093ASources+1,y
    sta z:zTemp01
    stz z:zTemp02
    pla
    lsr a
    lsr a
    lsr a
    sta z:zTemp10
CopyWaveColumns093A_at0:
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    lda (zTemp00)
    sta a:VDC_DATA_LO
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    lda z:zTemp02
    and #$FC
    sta z:zTemp03
    lda z:zTemp02
    clc
    adc z:zTemp10
    and #$03
    ora z:zTemp03
    asl a
    tay
    lda a:CopyWaveColumns093ADestinations,y
    sta a:VDC_DATA_LO
    lda a:CopyWaveColumns093ADestinations+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    ldy #$08
CopyWaveColumns093A_at1:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne CopyWaveColumns093A_at1
    lda #$02
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc CopyWaveColumns093A_at2
    inc z:zTemp01
CopyWaveColumns093A_at2:
    inc z:zTemp02
    lda z:zTemp02
    cmp #$0C
    bne CopyWaveColumns093A_at0
    rts

; @routine 093B:3D7 ByState093AB3D7
; @run 70:B3D7
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $B3E9.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState093AB3D7States = $B3E9 ; a handler per state
ByState093AB3D7:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState093AB3D7States,y
    sta z:zTemp00
    lda a:ByState093AB3D7States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093B:3EF ThrowWhenStruck093A
; @run 70:B3EF
; WHAT IT DOES: Holds the object it carries ($B423); five hits throw a kind $8B object, record the event, sound $91 and wait $40.
; HOW: FindFreeSlotFrom30 and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowWhenStruck093A:
    jsr CarryChild093A
    lda a:OBJ_START_ARG,x
    bmi ThrowWhenStruck093A_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc ThrowWhenStruck093A_at0
    jsr VecFindFreeSlotFrom30
    bcc ThrowWhenStruck093A_at0
    lda #$8B
    jsr VecStartObjectAtPlace
    lda #$FB
    sta a:OBJ_VY_HI,y
    lda #$16
    sta a:OBJ_INVULN_TIMER,y
    jsr VecRecordObjectEvent
    lda #$91
    jsr VecSoundCommand
    lda #$40
    sta a:OBJ_PIECE_NEXT,x
    inc a:OBJ_STATE,x
ThrowWhenStruck093A_at0:
    rts

; @routine 093B:423 CarryChild093A
; @run 70:B423
; WHAT IT DOES: While the object OBJ_START_ARG names is kind $2E under state 3: places it here, clears it off the first screen,
; else lifts it at -$1A00.
; HOW: CopyObjectPlace into that slot.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CarryChild093A:
    lda a:OBJ_START_ARG,x
    and #$7F
    tay
    lda a:OBJ_KIND,y
    cmp #$2E
    bne CarryChild093A_at1
    lda a:OBJ_STATE,y
    cmp #$03
    bcs CarryChild093A_at1
    jsr VecCopyObjectPlace
    lda a:OBJ_Y_HI,y
    beq CarryChild093A_at0
    jmp VecClearObjectInY
CarryChild093A_at0:
    lda #$E6
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocityY
    sxy
CarryChild093A_at1:
    rts

; @routine 093B:44D FallCarryingThenBurst093A
; @run 70:B44D
; WHAT IT DOES: Holds the child, falls, and counts out; at eight counts on the way sparks ($CE43).
; HOW: A search of the counts.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallCarryingThenBurst093ATimes = $B470 ; eight counts
FallCarryingThenBurst093A:
    jsr CarryChild093A
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocityY
    dec a:OBJ_PIECE_NEXT,x
    bne FallCarryingThenBurst093A_at0
    jmp VecClearObject
FallCarryingThenBurst093A_at0:
    cly
FallCarryingThenBurst093A_at1:
    lda a:OBJ_PIECE_NEXT,x
    cmp a:FallCarryingThenBurst093ATimes,y
    bne FallCarryingThenBurst093A_at2
    jmp SparkOfRow093E
FallCarryingThenBurst093A_at2:
    iny
    cpy #$08
    bcc FallCarryingThenBurst093A_at1
    rts

; @routine 093B:478 PlatformOnWater093A
; @run 70:B478
; WHAT IT DOES: State 0 loads the record as sprite set 6 frame $1B and keeps its Y. Then floats toward Y $A0 ($DA9A) with the
; player's weight (box $18 by $1D), dipping once as they step on.
; HOW: FloatToLine093F, ContactWithPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PlatformOnWater093A:
    lda a:OBJ_STATE,x
    bne PlatformOnWater093A_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$1B
    sta a:OBJ_FRAME,x
    lda a:OBJ_Y_LO,x
    sta a:OBJ_PIECE_PLACE,x
    rts
PlatformOnWater093A_at0:
    lda #$A0
    jsr FloatToLine0940
    lda #$18
    sta z:zTemp08
    lda #$1D
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc PlatformOnWater093A_at2
    lda a:OBJ_PIECE_NEXT,x
    bne PlatformOnWater093A_at1
    lda a:OBJ_Y_LO,x
    cmp #$A0
    bne PlatformOnWater093A_at1
    lda #$01
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
PlatformOnWater093A_at1:
    lda #$01
    sta a:OBJ_PIECE_NEXT,x
    rts
PlatformOnWater093A_at2:
    stz a:OBJ_PIECE_NEXT,x
    rts

; @routine 093B:525 LoomingPicture093A
; @run 70:B525
; WHAT IT DOES: State as PictureByArgument093A, drawn with attribute $87 and loop flag bit 1; at X $01xx past $17F wraps to the
; left.
; HOW: A call and stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoomingPicture093A:
    jsr PictureByArgument093A
    lda #$87
    sta a:OBJ_SPRITE_ATTR,x
    lda a:OBJ_LOOP_FLAGS,x
    ora #$02
    sta a:OBJ_LOOP_FLAGS,x
    lda a:OBJ_X_HI,x
    cmp #$01
    bne LoomingPicture093A_at0
    lda a:OBJ_X_LO,x
    bpl LoomingPicture093A_at0
    lda #$FF
    sta a:OBJ_X_HI,x
LoomingPicture093A_at0:
    rts

; @routine 093E:643 SparkOfRow093E
; @run 71:CE43
; WHAT IT DOES: Starts a kind $9C spark in a free slot with sound $18 and the row's X speed, and moves it.
; HOW: As SparkByCount07DB for row Y.
; WAITS: no.
; Entry: X object, Y the row. Exit: as the last instruction left them.
SparkOfRow093ESpeeds = $CE70 ; VX high by row
SparkOfRow093E:
    stx z:zClimbStepBefore
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc SparkOfRow093E_at0
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$18
    jsr VecSoundCommand
    lda z:zBurstCount
    asl a
    tax
    lda a:SparkOfRow093ESpeeds,x
    sta a:OBJ_VX_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zClimbStepBefore
SparkOfRow093E_at0:
    rts

; @routine 0940:29A FloatToLine0940
; @run 71:DA9A
; WHAT IT DOES: Moves down the Y speed; at or past the line in A stops there, else eases the speed upward by $40.
; HOW: ApplyVelocityY and a compare.
; WAITS: no.
; Entry: A the line, X object. Exit: as the last instruction left them.
FloatToLine0940:
    sta z:zTemp00
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp z:zTemp00
    beq FloatToLine0940_at0
    bcc FloatToLine0940_at0
    lda a:OBJ_VY_LO,x
    sec
    sbc #$40
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    rts
FloatToLine0940_at0:
    lda z:zTemp00
    sta a:OBJ_Y_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    rts
