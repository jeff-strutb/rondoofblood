; @routine 0043:3FC RasterStart
; @run 69:43FC
; WHAT IT DOES: At the frame's start, copies the bands this frame's effect uses.
; HOW: By effect number through a table of the copy's entries.
; Entry: nothing. Exit: as the copy; A the effect doubled and N, Z and C from that
; if the effect copies nothing. X the effect doubled. Y kept.
RasterStart:
    lda a:RasterEffectNow
    asl a
    tax
    jmp (RasterStartEntries,x)

; @routine 0043:444 CopyBands7Down
; @run 69:4444
; WHAT IT DOES: Copies the game's scroll to the frame's bands, for an effect using
; bands up to 7.
; HOW: Byte by byte, down to band 1; the later entries start lower.
; Entry: nothing. Exit: A the last byte, N and Z from it. X and Y kept.
CopyBands7Down:
    lda a:ScrollXLo+7
    sta a:ScrollBgxLo+7

; @routine 0043:44A CopyBands6Down
; @run 69:444A
; WHAT IT DOES: Copies the game's scroll to the frame's bands, up to band 6.
; HOW: As CopyBands7Down.
; Entry and exit: as CopyBands7Down.
CopyBands6Down:
    lda a:ScrollXLo+6
    sta a:ScrollBgxLo+6
    lda a:ScrollXHi+6
    sta a:ScrollBgxHi+6
    lda a:ScrollYLo+6
    sta a:ScrollBgyLo+6
    lda a:ScrollYHi+6
    sta a:ScrollBgyHi+6
    lda a:ScrollXLo+5
    sta a:ScrollBgxLo+5
    lda a:ScrollXHi+5
    sta a:ScrollBgxHi+5
    lda a:ScrollYLo+5
    sta a:ScrollBgyLo+5
    lda a:ScrollYHi+5
    sta a:ScrollBgyHi+5

; @routine 0043:47A CopyBands4Down
; @run 69:447A
; WHAT IT DOES: Copies the game's scroll to the frame's bands, up to band 4.
; HOW: As CopyBands7Down.
; Entry and exit: as CopyBands7Down.
CopyBands4Down:
    lda a:ScrollXLo+4
    sta a:ScrollBgxLo+4
    lda a:ScrollXHi+4
    sta a:ScrollBgxHi+4
    lda a:ScrollYLo+4
    sta a:ScrollBgyLo+4
    lda a:ScrollYHi+4
    sta a:ScrollBgyHi+4

; @routine 0043:492 CopyBands3Down
; @run 69:4492
; WHAT IT DOES: Copies the game's scroll to the frame's bands, up to band 3.
; HOW: As CopyBands7Down.
; Entry and exit: as CopyBands7Down.
CopyBands3Down:
    lda a:ScrollXLo+3
    sta a:ScrollBgxLo+3
    lda a:ScrollXHi+3
    sta a:ScrollBgxHi+3

; @routine 0043:49E CopyBandsYFrom3
; @run 69:449E
; WHAT IT DOES: Copies the game's scroll to the frame's bands: band 3's Y only, then
; from band 2 down.
; HOW: As CopyBands7Down.
; Entry and exit: as CopyBands7Down.
CopyBandsYFrom3:
    lda a:ScrollYLo+3
    sta a:ScrollBgyLo+3
    lda a:ScrollYHi+3
    sta a:ScrollBgyHi+3

; @routine 0043:4AA CopyBandsWaterDown
; @run 69:44AA
; WHAT IT DOES: Copies the game's scroll to the frame's bands from band 2 down, with the
; water line first.
; HOW: As CopyBands7Down.
; Entry and exit: as CopyBands7Down.
CopyBandsWaterDown:
    lda a:RasterWaterWanted
    sta a:RasterWaterLine
    lda a:ScrollXLo+2
    sta a:ScrollBgxLo+2
    lda a:ScrollXHi+2
    sta a:ScrollBgxHi+2
    lda a:ScrollYLo+2
    sta a:ScrollBgyLo+2
    lda a:ScrollYHi+2
    sta a:ScrollBgyHi+2

; @routine 0043:4C8 CopyBand1
; @run 69:44C8
; WHAT IT DOES: Copies the game's scroll to the frame's band 1 alone.
; HOW: As CopyBands7Down.
; Entry and exit: as CopyBands7Down.
CopyBand1:
    lda a:ScrollXLo+1
    sta a:ScrollBgxLo+1
    lda a:ScrollXHi+1
    sta a:ScrollBgxHi+1
    lda a:ScrollYLo+1
    sta a:ScrollBgyLo+1
    lda a:ScrollYHi+1
    sta a:ScrollBgyHi+1

; @routine 0043:4E0 CopyNoBands
; @run 69:44E0
; WHAT IT DOES: The effects with no band of their own copy nothing.
; HOW: Returns.
; Entry: nothing. Exit: everything kept.
CopyNoBands:
    rts

; @routine 0043:4E1 RasterInterrupt
; @run 69:44E1
; @ends 4287
; WHAT IT DOES: A raster interrupt: runs this frame's effect.
; HOW: Through the table by effect number. Effect 0 only ends the interrupt.
; Entry: from FrameInterrupt, the effect below 32, which is all the table holds. Exit:
; to its exit.
RasterInterrupt:
    lda a:RasterEffectNow
    asl a
    tax
    jmp (RasterEffects,x)

; @routine 0043:529 RasterEndBand1
; @run 69:4529
; @ends 4287
; WHAT IT DOES: A raster effect's last band: band 1's scroll, and no more lines.
; HOW: WriteBandScroll, then RasterDone. Effect 1 is this alone.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEndBand1:
    ldx #$01
RasterEndBand1_at0:
    jsr WriteBandScroll

; @routine 0043:52E RasterDone
; @run 69:452E
; @ends 4287
; WHAT IT DOES: Ends a raster interrupt, with interrupts back on for the next.
; HOW: CLI, then the frame interrupt's exit.
; Entry: from a raster effect. Exit: to that exit.
RasterDone:
    cli
    jmp FrameInterrupt_at2

; @routine 0043:532 RasterEndWorkBand
; @run 69:4532
; @ends 4287
; WHAT IT DOES: A raster effect's last band: the working band's scroll, and no more
; lines.
; HOW: WriteBandScroll, then RasterDone.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEndWorkBand:
    ldx #$08
    bra RasterEndBand1_at0

; @routine 0043:536 RasterEffect2
; @run 69:4536
; @ends 4287
; WHAT IT DOES: Raster effect 2: two bands, split at line $BF.
; HOW: Band 1, then band 0's X and Y high with Y low $7F.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect2:
    lda a:RasterStep
    beq RasterEffect2_at0
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyHi
    sta a:ScrollBgyHi+8
    lda #$7F
    sta a:ScrollBgyLo+8
    jmp RasterEndWorkBand
RasterEffect2_at0:
    ldx #$01
    lda #$BF
    jmp RasterBandThenLine

; @routine 0043:55C RasterEffect3
; @run 69:455C
; @ends 4287
; WHAT IT DOES: Raster effect 3: three bands, the first split moving with half of band
; 3's Y scroll.
; HOW: The split's line comes from that scroll; the second band takes band 2 with its
; Y offset; the last, band 0.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect3:
    lda a:RasterStep
    beq RasterEffect3_at0
    dec a
    beq RasterEffect3_at1
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$BF
    sta a:ScrollBgyLo+8
    jmp RasterEndWorkBand
RasterEffect3_at0:
    ldx #$01
    lda a:ScrollBgyLo+3
    lsr a
    sta a:ScrollBgyLo+8
    lda #$AF
    sec
    sbc a:ScrollBgyLo+8
    jmp RasterBandThenLine
RasterEffect3_at1:
    lda a:ScrollBgxLo+2
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+2
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo+2
    clc
    adc #$6F
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi+2
    sta a:ScrollBgyHi+8
    ldx #$08
    lda #$FF
    sec
    sbc a:ScrollBgyLo+3
    jmp RasterBandThenLine

; @routine 0043:5B1 RasterEffect4
; @run 69:45B1
; @ends 4287
; WHAT IT DOES: Raster effect 4: band 0's X with band 1's Y, in one step.
; HOW: Copies them to the working band.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect4:
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo+1
    sta a:ScrollBgyLo+8
    lda a:ScrollBgyHi+1
    sta a:ScrollBgyHi+8
    jmp RasterEndWorkBand

; @routine 0043:5CC RasterEffect5
; @run 69:45CC
; @ends 4287
; WHAT IT DOES: Raster effect 5: four bands, split at lines $5F, $7F and $FF.
; HOW: Bands 1, 2 and 1 with their Y offsets, then band 0.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect5:
    lda a:RasterStep
    beq RasterEffect5_at0
    dec a
    beq RasterEffect5_at1
    dec a
    beq RasterEffect5_at2
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo
    clc
    adc #$BF
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi
    sta a:ScrollBgyHi+8
    jmp RasterEndWorkBand
RasterEffect5_at0:
    ldx #$01
    lda #$5F
    jmp RasterBandThenLine
RasterEffect5_at1:
    lda a:ScrollBgxLo+2
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+2
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo+2
    clc
    adc #$1F
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi+2
    sta a:ScrollBgyHi+8
    ldx #$08
    lda #$7F
    jmp RasterBandThenLine
RasterEffect5_at2:
    lda a:ScrollBgxLo+1
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+1
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo+1
    clc
    adc #$3F
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi+1
    sta a:ScrollBgyHi+8
    ldx #$08
    lda #$FF
    jmp RasterBandThenLine

; @routine 0043:643 RasterEffect6
; @run 69:4643
; @ends 4287
; WHAT IT DOES: Raster effect 6: seven bands from tables.
; HOW: Band 4, five steps from the tables, then band 1 at Y $DB.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect6:
    lda a:RasterStep
    beq RasterEffect6_at0
    cmp #$06
    bne RasterEffect6_at1
    lda a:ScrollBgxLo+1
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+1
    sta a:ScrollBgxHi+8
    lda #$DB
    sta a:ScrollBgyLo+8
    jmp RasterEndWorkBand
RasterEffect6_at0:
    ldx #$04
    lda #$67
    jmp RasterBandThenLine
RasterEffect6_at1:
    tay
    ldx a:Effect6Bands,y
    lda a:ScrollBgxLo,x
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi,x
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo,x
    clc
    adc a:Effect6YAdds,y
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi,x
    sta a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$40
    clc
    adc a:Effect6Lines,y
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone

; @routine 0043:6B6 RasterEffect7
; @run 69:46B6
; @ends 4287
; WHAT IT DOES: Raster effect 7: seven bands from tables, with word Y offsets.
; HOW: Band 4, five steps from the tables, then band 1 at Y $11B.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect7:
    lda a:RasterStep
    beq RasterEffect7_at0
    cmp #$06
    bne RasterEffect7_at1
    lda a:ScrollBgxLo+1
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+1
    sta a:ScrollBgxHi+8
    lda #$1B
    sta a:ScrollBgyLo+8
    lda #$01
    sta a:ScrollBgyHi+8
    jmp RasterEndWorkBand
RasterEffect7_at0:
    ldx #$04
    lda #$67
    jmp RasterBandThenLine
RasterEffect7_at1:
    tay
    ldx a:Effect6Bands,y
    asl a
    tay
    lda a:ScrollBgxLo,x
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi,x
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo,x
    clc
    adc a:Effect7YAdds,y
    sta a:ScrollBgyLo+8
    lda a:ScrollBgyHi,x
    adc a:Effect7YAdds+1,y
    sta a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    tya
    lsr a
    tay
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$40
    clc
    adc a:Effect6Lines,y
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone

; @routine 0043:730 RasterEffect8Step4
; @run 69:4730
; @ends 4287
; WHAT IT DOES: Effect 8's fifth step: turns the background off below the band.
; HOW: Clears the background bit in the VDC's control, and sets line $E0.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect8Step4:
    jsr SelectVdcControl
    lda z:zVdcCrLo
    and #$BF
    sta a:VDC_DATA_LO
    lda #$E0
    jmp RasterSetNextLine

; @routine 0043:73F RasterEffect8Step0
; @run 69:473F
; @ends 4287
; WHAT IT DOES: Effect 8's first step: band 5 scrolled one line further, for the one
; band only.
; HOW: Raises its Y high byte around WriteBandScroll, and sets line $5F.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect8Step0:
    inc a:ScrollBgyHi+5
    ldx #$05
    jsr WriteBandScroll
    dec a:ScrollBgyHi+5
    lda #$5F
    jmp RasterSetNextLine

; @routine 0043:74F RasterEffect8Step3
; @run 69:474F
; @ends 4287
; WHAT IT DOES: Effect 8's fourth step: band 0's X at Y $6F, and line $DF.
; HOW: Builds the working band and writes it.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect8Step3:
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$6F
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    ldx #$08
    lda #$DF
    jmp RasterBandThenLine

; @routine 0043:76A RasterEffect8
; @run 69:476A
; @ends 4287
; WHAT IT DOES: Raster effect 8: the most elaborate of the splits. Two bands from
; tables, a third, the background off, then sprites' colours flattened below a line
; and restored at the screen's end.
; HOW: A step per band, as the steps above.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect8:
    lda a:RasterStep
    beq RasterEffect8Step0
    cmp #$03
    bcc RasterEffect8_at0
    beq RasterEffect8Step3
    cmp #$04
    beq RasterEffect8Step4
    cmp #$05
    beq RasterEffect8_at1
RasterEffect8_at5:
    lda #$01
    sta a:VCE_ADDR_LO
    lda #$01
    sta a:VCE_ADDR_HI
    ldx #$0F
RasterEffect8_at2:
    lda a:SavedColoursLo,x
    sta a:VCE_DATA_LO
    lda a:SavedColoursHi,x
    sta a:VCE_DATA_HI
    dex
    bne RasterEffect8_at2
    jmp RasterDone
RasterEffect8_at0:
    tay
    ldx a:Effect8Bands,y
    asl a
    tay
    lda a:ScrollBgxLo,x
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi,x
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo,x
    clc
    adc a:Effect8YAdds,y
    sta a:ScrollBgyLo+8
    lda a:ScrollBgyHi,x
    adc a:Effect8YAdds+1,y
    sta a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    tya
    lsr a
    tay
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$40
    clc
    adc a:Effect8Lines,y
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone
RasterEffect8_at1:
    lda #$01
    sta a:VCE_ADDR_LO
    lda #$01
    sta a:VCE_ADDR_HI
    ldx #$0F
RasterEffect8_at3:
    lda a:VCE_DATA_LO
    sta a:SavedColoursLo,x
    lda a:VCE_DATA_HI
    sta a:SavedColoursHi,x
    dex
    bne RasterEffect8_at3
    lda #$01
    sta a:VCE_ADDR_LO
    lda #$01
    sta a:VCE_ADDR_HI
    ldx #$0F
    lda #$07
RasterEffect8_at4:
    sta a:VCE_DATA_LO
    stz a:VCE_DATA_HI
    dex
    bne RasterEffect8_at4
    jsr SelectVdcControl
    lda z:zFrameCount
    and #$01
    tax
    lda z:zVdcCrLo
    and a:FlickerMasks,x
    sta a:VDC_DATA_LO
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    st1 #$21
    st2 #$01
    jmp RasterDone

; @routine 0044:03D RasterEffect9
; @run 69:483D
; @ends 4287
; WHAT IT DOES: Raster effect 9: three bands, split at lines $7F and $BF.
; HOW: Band 0, band 1 at Y $13F, band 0 at Y $7F.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect9:
    lda a:RasterStep
    beq RasterEffect9_at0
    dec a
    beq RasterEffect9_at1
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$7F
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    jmp RasterEndWorkBand
RasterEffect9_at0:
    clx
    lda #$7F
    jmp RasterBandThenLine
RasterEffect9_at1:
    lda a:ScrollBgxLo+1
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+1
    sta a:ScrollBgxHi+8
    lda #$3F
    sta a:ScrollBgyLo+8
    lda #$01
    sta a:ScrollBgyHi+8
    ldx #$08
    lda #$BF
    jmp RasterBandThenLine

; @routine 0044:07F RasterEffect10
; @run 69:487F
; @ends 4287
; WHAT IT DOES: Raster effect 10: three bands, split at lines $9F and $DF.
; HOW: Band 0, band 1 at Y $15F, band 0 at Y $9F.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect10:
    lda a:RasterStep
    beq RasterEffect10_at0
    dec a
    beq RasterEffect10_at1
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$9F
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    jmp RasterEndWorkBand
RasterEffect10_at0:
    clx
    lda #$9F
    jmp RasterBandThenLine
RasterEffect10_at1:
    lda a:ScrollBgxLo+1
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+1
    sta a:ScrollBgxHi+8
    lda #$5F
    sta a:ScrollBgyLo+8
    lda #$01
    sta a:ScrollBgyHi+8
    ldx #$08
    lda #$DF
    jmp RasterBandThenLine

; @routine 0044:0C1 RasterEffect11
; @run 69:48C1
; @ends 4287
; WHAT IT DOES: Raster effect 11: the background off below the line.
; HOW: Clears its bit in the VDC's control.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect11:
    jsr SelectVdcControl
    lda z:zVdcCrLo
    and #$BF
    sta a:VDC_DATA_LO
    jmp RasterDone

; @routine 0044:0CE RasterEffect12
; @run 69:48CE
; @ends 4287
; WHAT IT DOES: Raster effect 12: five bands from tables.
; HOW: Band 2, three steps from the tables, then band 5 at Y $B8.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect12:
    lda a:RasterStep
    beq RasterEffect12_at0
    cmp #$04
    bne RasterEffect12_at1
    lda a:ScrollBgxLo+5
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+5
    sta a:ScrollBgxHi+8
    lda #$B8
    sta a:ScrollBgyLo+8
    jmp RasterEndWorkBand
RasterEffect12_at0:
    ldx #$02
    lda #$7F
    jmp RasterBandThenLine
RasterEffect12_at1:
    tay
    ldx a:Effect12Bands,y
    lda a:ScrollBgxLo,x
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi,x
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo,x
    clc
    adc a:Effect12YAdds,y
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi,x
    sta a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$40
    clc
    adc a:Effect12Lines,y
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone

; @routine 0044:139 RasterEffect13
; @run 69:4939
; @ends 4287
; WHAT IT DOES: Raster effect 13: six bands from tables, the first scrolled one line
; further for the one band, the last with the background off at the game's say.
; HOW: Raises band 4's Y high around its write, then the tables, then band 0.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect13:
    lda a:RasterStep
    beq RasterEffect13_at0
    cmp #$05
    bne RasterEffect13_at1
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$CF
    sta a:ScrollBgyLo+8
    ldx #$08
    jsr WriteBandScroll
    lda a:ScrollXLo+7
    beq RasterEffect13_at2
    jsr SelectVdcControl
    lda z:zVdcCrLo
    and #$BF
    sta a:VDC_DATA_LO
RasterEffect13_at2:
    jmp RasterDone
RasterEffect13_at0:
    inc a:ScrollBgyHi+4
    ldx #$04
    jsr WriteBandScroll
    dec a:ScrollBgyHi+4
    lda #$5F
    jmp RasterSetNextLine
RasterEffect13_at1:
    tay
    ldx a:Effect13Bands,y
    asl a
    tay
    lda a:ScrollBgxLo,x
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi,x
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo,x
    clc
    adc a:Effect13YAdds,y
    sta a:ScrollBgyLo+8
    lda a:ScrollBgyHi,x
    adc a:Effect13YAdds+1,y
    sta a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    tya
    lsr a
    tay
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$40
    clc
    adc a:Effect13Lines,y
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone

; @routine 0044:1D1 RasterEffect14
; @run 69:49D1
; @ends 4287
; WHAT IT DOES: Raster effect 14: bands with the sprite colours flattened and restored
; over the last two.
; HOW: Band 4, band 0, then the colours' two steps.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect14:
    lda a:RasterStep
    beq RasterEffect14_at0
    cmp #$02
    beq RasterEffect14_at1
    cmp #$03
    beq RasterEffect14_at2
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyHi
    sta a:ScrollBgyHi+8
    lda #$2F
    sta a:ScrollBgyLo+8
    ldx #$08
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    st1 #$11
    st2 #$01
    jmp RasterDone
RasterEffect14_at0:
    ldx #$04
    lda #$6F
    jmp RasterBandThenLine
RasterEffect14_at1:
    jmp RasterEffect8_at1
RasterEffect14_at2:
    jmp RasterEffect8_at5

; @routine 0044:216 RasterEffect15
; @run 69:4A16
; @ends 4287
; WHAT IT DOES: Raster effect 15: a water wave: the band each line pair shifted
; sideways by a table that moves with the frame's count, for 20 steps.
; HOW: Band 2, then each step's X from band 1 plus the wave, Y moving on two lines
; with the step; then band 0.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect15:
    lda a:RasterStep
    beq RasterEffect15_at0
    cmp #$15
    bcc RasterEffect15_at1
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$01
    sta a:ScrollBgyHi+8
    lda #$B7
    sta a:ScrollBgyLo+8
    jmp RasterEndWorkBand
RasterEffect15_at0:
    ldx #$02
    lda #$CF
    sec
    sbc a:RasterWaterLine
    jmp RasterBandThenLine
RasterEffect15_at1:
    lda a:RasterStep
    dec a
    asl a
    adc a:ScrollBgyLo+1
    clc
    adc #$8F
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi+1
    sta a:ScrollBgyHi+8
    lda z:zFrameTick
    lsr a
    lsr a
    lsr a
    clc
    adc a:RasterStep
    and #$07
    asl a
    tay
    lda a:WaterWave,y
    clc
    adc a:ScrollBgxLo+1
    sta a:ScrollBgxLo+8
    lda a:WaterWave+1,y
    adc a:ScrollBgxHi+1
    sta a:ScrollBgxHi+8
    ldx #$08
    jsr WriteBandScroll
    lda #$06
    jsr SelectVdcRegister
    lda a:RasterStep
    asl a
    sta a:ScrollBgyLo+8
    lda #$CF
    sec
    sbc a:RasterWaterLine
    clc
    adc a:ScrollBgyLo+8
    sta a:VDC_DATA_LO
    cla
    rol a
    sta a:VDC_DATA_HI
    inc a:RasterStep
    jmp RasterDone

; @routine 0044:2B1 RasterEffect16
; @run 69:4AB1
; @ends 4287
; WHAT IT DOES: Raster effect 16: six bands from tables.
; HOW: Band 5, four steps from the tables, then band 1 at Y $BF.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect16:
    lda a:RasterStep
    beq RasterEffect16_at0
    cmp #$05
    bne RasterEffect16_at1
    lda a:ScrollBgxLo+1
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+1
    sta a:ScrollBgxHi+8
    lda #$BF
    sta a:ScrollBgyLo+8
    jmp RasterEndWorkBand
RasterEffect16_at0:
    ldx #$05
    lda #$57
    jmp RasterBandThenLine
RasterEffect16_at1:
    tay
    ldx a:Effect16Bands,y
    lda a:ScrollBgxLo,x
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi,x
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo,x
    clc
    adc a:Effect16YAdds,y
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi,x
    sta a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$40
    clc
    adc a:Effect16Lines,y
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone

; @routine 0044:321 RasterEffect17
; @run 69:4B21
; @ends 4287
; WHAT IT DOES: Raster effect 17: six bands from tables.
; HOW: Band 4, four steps from the tables, then band 2 at Y $D7.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect17:
    lda a:RasterStep
    beq RasterEffect17_at0
    cmp #$05
    bne RasterEffect17_at1
    lda a:ScrollBgxLo+2
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+2
    sta a:ScrollBgxHi+8
    lda #$D7
    sta a:ScrollBgyLo+8
    jmp RasterEndWorkBand
RasterEffect17_at0:
    ldx #$04
    lda #$67
    jmp RasterBandThenLine
RasterEffect17_at1:
    tay
    ldx a:Effect17Bands,y
    lda a:ScrollBgxLo,x
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi,x
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo,x
    clc
    adc a:Effect17YAdds,y
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi,x
    sta a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$40
    clc
    adc a:Effect17Lines,y
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone

; @routine 0044:38F RasterEffect18
; @run 69:4B8F
; @ends 4287
; WHAT IT DOES: Raster effect 18: a band whose height the game sets, then the rest of
; the screen following it.
; HOW: The fixed band to the line in band 3's Y, then band 1 built from band 0 moved
; down by that height, then the fixed band again.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect18:
    lda a:RasterStep
    beq RasterEffect18_at0
    dec a
    beq RasterEffect18_at1
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda #$01
    sta a:ScrollBgxHi+8
    lda #$68
    sta a:ScrollBgyLo+8
    lda #$01
    sta a:ScrollBgyHi+8
    jmp RasterEndWorkBand
RasterEffect18_at0:
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda #$01
    sta a:ScrollBgxHi+8
    lda #$68
    sta a:ScrollBgyLo+8
    lda #$01
    sta a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda a:ScrollBgyLo+2
    jmp RasterEffect20_at0
RasterEffect18_at1:
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+1
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+1
    lda a:ScrollBgyLo
    clc
    adc a:ScrollBgyLo+2
    sta a:ScrollBgyLo+1
    cla
    adc #$00
    sta a:ScrollBgyHi+1
    ldx #$01
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$E0
    sec
    sbc a:ScrollBgyLo+2
    clc
    adc #$40
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone

; @routine 0044:415 RasterEffect19
; @run 69:4C15
; @ends 4287
; WHAT IT DOES: Raster effect 19: a stretch of screen with the background off, its
; edges set by two bytes the game moves.
; HOW: Band 0 background off to the first; band 1 background on to the second; band 2,
; background off at the game's say.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect19:
    lda a:RasterStep
    beq RasterEffect19_at0
    dec a
    beq RasterEffect19_at1
    lda a:ScrollBgyHi+6
    bne RasterEffect19_at2
    jsr SelectVdcControl
    lda z:zVdcCrLo
    and #$BF
    sta a:VDC_DATA_LO
RasterEffect19_at2:
    ldx #$02
    jsr WriteBandScroll
    jmp RasterDone
RasterEffect19_at0:
    clx
    jsr WriteBandScroll
    jsr SelectVdcControl
    lda z:zVdcCrLo
    and #$BF
    sta a:VDC_DATA_LO
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda a:ScrollBgyLo+5
    jmp RasterEffect20_at0
RasterEffect19_at1:
    ldx #$01
    jsr WriteBandScroll
    jsr SelectVdcControl
    lda z:zVdcCrLo
    ora #$40
    sta a:VDC_DATA_LO
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda a:ScrollBgyLo+6
    clc
    adc #$40
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone

; @routine 0044:479 RasterEffect20
; @run 69:4C79
; @ends 4287
; WHAT IT DOES: Raster effect 20: as effect 19 with the edges allowed to meet: steps
; skip when a moving edge passes a fixed line at $60, $77, $A7 or $BF.
; HOW: Band 3, band 0 and band 6 up to those lines, band 1 background off, band 4
; background on.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect20:
    lda a:RasterStep
    beq RasterEffect20_at1
    dec a
    beq RasterEffect20_at2
    dec a
    beq RasterEffect20_at3
    dec a
    beq RasterEffect20_at4
    jsr SelectVdcControl
    lda z:zVdcCrLo
    ora #$40
    sta a:VDC_DATA_LO
    ldx #$04
    jsr WriteBandScroll
    jmp RasterDone
RasterEffect20_at1:
    ldx #$03
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda a:ScrollBgyLo+5
    cmp #$60
    bcs RasterEffect20_at5
    inc a:RasterStep
    bra RasterEffect20_at6
RasterEffect20_at5:
    lda #$5F
RasterEffect20_at6:
    jmp RasterEffect20_at0
RasterEffect20_at2:
    clx
    jsr WriteBandScroll
    lda a:ScrollBgyLo+5
    cmp #$77
    bcs RasterEffect20_at7
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda a:ScrollBgyLo+5
    jmp RasterEffect20_at0
RasterEffect20_at7:
    lda #$04
    sta a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$A7
    jmp RasterEffect20_at0
RasterEffect20_at4:
    ldx #$06
    jsr WriteBandScroll
    jsr SelectVdcControl
    lda z:zVdcCrLo
    ora #$40
    sta a:VDC_DATA_LO
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda a:ScrollBgyLo+6
    cmp #$A8
    bcs RasterEffect20_at8
    lda #$A7
RasterEffect20_at8:
    jmp RasterEffect20_at0
RasterEffect20_at3:
    ldx #$01
    jsr WriteBandScroll
    jsr SelectVdcControl
    lda z:zVdcCrLo
    and #$BF
    sta a:VDC_DATA_LO
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda a:ScrollBgyLo+6
    cmp #$BF
    beq RasterEffect20_at9
    cmp #$A7
    bcc RasterEffect20_at0
    inc a:RasterStep
RasterEffect20_at0:
    clc
    adc #$40
    sta a:VDC_DATA_LO
    st2 #$00
RasterEffect20_at9:
    jmp RasterDone

; @routine 0044:532 RasterEffect21
; @run 69:4D32
; @ends 4287
; WHAT IT DOES: Raster effect 21: five bands split at $7F, $87 and $FF, then a late
; compare, the background off in the last at a stage value.
; HOW: Bands 2, 1, 3, then band 0 twice.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect21:
    lda a:RasterStep
    beq RasterEffect21_at0
    dec a
    beq RasterEffect21_at1
    dec a
    beq RasterEffect21_at2
    dec a
    beq RasterEffect21_at3
    lda z:zRasterStage
    cmp #$03
    bne RasterEffect21_at4
    jsr SelectVdcControl
    lda z:zVdcCrLo
    and #$BF
    sta a:VDC_DATA_LO
RasterEffect21_at4:
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$C7
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    jmp RasterEndWorkBand
RasterEffect21_at0:
    ldx #$02
    lda #$7F
    jmp RasterBandThenLine
RasterEffect21_at1:
    ldx #$01
    lda #$87
    jmp RasterBandThenLine
RasterEffect21_at2:
    ldx #$03
    lda #$FF
    jmp RasterBandThenLine
RasterEffect21_at3:
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$BF
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    st1 #$07
    st2 #$01
    jmp RasterDone

; @routine 0044:5A4 RasterEffect22
; @run 69:4DA4
; @ends 4287
; WHAT IT DOES: Raster effect 22: two bands: band 2 at Y $110 to line $67, band 0 after.
; HOW: Builds the working band each step.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect22:
    lda a:RasterStep
    bne RasterEffect22_at0
    lda #$10
    sta a:ScrollBgyLo+8
    lda #$01
    sta a:ScrollBgyHi+8
    lda a:ScrollBgxLo+2
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+2
    sta a:ScrollBgxHi+8
    ldx #$08
    lda #$67
    jmp RasterBandThenLine
RasterEffect22_at0:
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$27
    clc
    adc a:ScrollBgyLo
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    jmp RasterEndWorkBand

; @routine 0044:5E1 RasterEffect23
; @run 69:4DE1
; @ends 4287
; WHAT IT DOES: Raster effect 23: the sprite colours flattened from line $141's compare
; and restored after.
; HOW: A first compare, then the colours' two steps.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect23:
    lda a:RasterStep
    beq RasterEffect23_at0
    dec a
    bne RasterEffect23_at1
    jmp RasterEffect8_at1
RasterEffect23_at1:
    jmp RasterEffect8_at5
RasterEffect23_at0:
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    st1 #$01
    st2 #$01
    jmp RasterDone

; @routine 0044:5FE RasterEffect25
; @run 69:4DFE
; @ends 4287
; WHAT IT DOES: Raster effect 25: the sprite colours flattened at once and restored
; after.
; HOW: The colours' two steps.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect25:
    lda a:RasterStep
    bne RasterEffect25_at0
    jmp RasterEffect8_at1
RasterEffect25_at0:
    jmp RasterEffect8_at5

; @routine 0044:609 RasterEffect29
; @run 69:4E09
; @ends 4287
; WHAT IT DOES: Raster effect 29: the sprite colours flattened from a later compare than
; effect 23's, and restored after.
; HOW: A first compare, then the colours' two steps.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect29:
    lda a:RasterStep
    beq RasterEffect29_at0
    dec a
    bne RasterEffect29_at1
    jmp RasterEffect8_at1
RasterEffect29_at1:
    jmp RasterEffect8_at5
RasterEffect29_at0:
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    st1 #$18
    st2 #$01
    jmp RasterDone

; @routine 0044:626 RasterEffect24
; @run 69:4E26
; @ends 4287
; WHAT IT DOES: Raster effect 24: water under a line with a wave that moves on its own
; counter, for 16 steps; each step the background moves further down, two or three
; lines by a slower wave.
; HOW: Band 0, then each step's X from band 1 plus the wave. The next compare line is
; kept in band 2's X bytes, itself moved on each step.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect24:
    lda a:RasterStep
    beq RasterEffect24_at0
    cmp #$11
    bcc RasterEffect24_at1
    jmp RasterEndBand1
RasterEffect24_at1:
    lda a:RasterStep
    dec a
    asl a
    adc a:ScrollBgyLo+1
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    lda z:zWaveCounter
    lsr a
    lsr a
    lsr a
    clc
    adc a:RasterStep
    and #$07
    asl a
    tay
    lda a:DeepWave,y
    clc
    adc a:ScrollBgxLo+1
    sta a:ScrollBgxLo+8
    lda a:DeepWave+1,y
    adc a:ScrollBgxHi+1
    sta a:ScrollBgxHi+8
    ldx #$08
    jsr WriteBandScroll
    lda #$06
    jsr SelectVdcRegister
    lda z:zWaveCounter
    lsr a
    lsr a
    lsr a
    clc
    adc a:RasterStep
    lsr a
    lsr a
    lsr a
    and #$01
    clc
    adc #$02
    adc a:ScrollBgxLo+2
    sta a:ScrollBgxLo+2
    sta a:VDC_DATA_LO
    cla
    adc a:ScrollBgxHi+2
    sta a:ScrollBgxHi+2
    sta a:VDC_DATA_HI
    inc a:RasterStep
    jmp RasterDone
RasterEffect24_at0:
    clx
    lda #$FF
    sta a:ScrollBgxLo+2
    stz a:ScrollBgxHi+2
    jmp RasterBandThenLine

; @routine 0044:6B0 RasterEffect26
; @run 69:4EB0
; @ends 4287
; WHAT IT DOES: Raster effect 26: a marked band every two lines, for $70 steps.
; HOW: See marked_band_step.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect26:
    lda a:RasterStep
    beq RasterEffect26_at0
    cmp #$71
    bcc RasterEffect26_at1
    jmp RasterEndBand1
RasterEffect26_at1:
    lda a:RasterStep
    tay
    dec a
    asl a
    inc a
    sta a:ScrollBgyLo+8
    lda a:RasterMarks,y
    bmi RasterEffect26_at2
    stz a:ScrollBgxLo+8
    stz a:ScrollBgyHi+8
    lda #$01
    sta a:ScrollBgxHi+8
    bra RasterEffect26_at3
RasterEffect26_at2:
    lda #$01
    sta a:ScrollBgyHi+8
    lda z:zWaveCounter
    lsr a
    lsr a
    clc
    adc a:RasterStep
    and #$07
    asl a
    tax
    lda a:DeepWave,x
    sta a:ScrollBgxLo+8
    lda a:DeepWave+1,x
    sta a:ScrollBgxHi+8
RasterEffect26_at3:
    lda a:RasterMarks,y
    and #$7F
    clc
    adc a:ScrollBgxLo+8
    sta a:ScrollBgxLo+8
    ldx #$08
    jsr WriteBandScroll
    lda #$06
    jsr SelectVdcRegister
    lda a:RasterStep
    asl a
    clc
    adc #$41
    sta a:VDC_DATA_LO
    cla
    rol a
    sta a:VDC_DATA_HI
    inc a:RasterStep
    jmp RasterDone
RasterEffect26_at0:
    clx
    lda #$41
    jmp RasterBandThenLine

; @routine 0044:726 RasterEffect27
; @run 69:4F26
; @ends 4287
; WHAT IT DOES: Raster effect 27: a marked band every eight lines, for $1C steps.
; HOW: See marked_band_step.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect27:
    lda a:RasterStep
    beq RasterEffect27_at0
    cmp #$1D
    bcc RasterEffect27_at1
    jmp RasterEndBand1
RasterEffect27_at1:
    lda a:RasterStep
    tay
    dec a
    asl a
    asl a
    asl a
    inc a
    sta a:ScrollBgyLo+8
    lda a:RasterMarks,y
    bmi RasterEffect27_at2
    stz a:ScrollBgxLo+8
    stz a:ScrollBgyHi+8
    lda #$01
    sta a:ScrollBgxHi+8
    bra RasterEffect27_at3
RasterEffect27_at2:
    lda #$01
    sta a:ScrollBgyHi+8
    lda z:zWaveCounter
    lsr a
    lsr a
    clc
    adc a:RasterStep
    and #$07
    asl a
    tax
    lda a:DeepWave,x
    sta a:ScrollBgxLo+8
    lda a:DeepWave+1,x
    sta a:ScrollBgxHi+8
RasterEffect27_at3:
    lda a:RasterMarks,y
    and #$7F
    clc
    adc a:ScrollBgxLo+8
    sta a:ScrollBgxLo+8
    ldx #$08
    jsr WriteBandScroll
    lda #$06
    jsr SelectVdcRegister
    lda a:RasterStep
    asl a
    asl a
    asl a
    clc
    adc #$41
    sta a:VDC_DATA_LO
    cla
    rol a
    sta a:VDC_DATA_HI
    inc a:RasterStep
    jmp RasterDone
RasterEffect27_at0:
    clx
    lda #$41
    jmp RasterBandThenLine

; @routine 0044:7A0 RasterEffect28
; @run 69:4FA0
; @ends 4287
; WHAT IT DOES: Raster effect 28: a band every two lines from line $A7, each shaking a
; pixel or two sideways, now and then, at random.
; HOW: Band 3, then steps with band 3's X; when the frame's count and step land on
; zero, the top of the random byte is added. Then band 0.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect28:
    lda a:RasterStep
    beq RasterEffect28_at0
    cmp #$0C
    bcc RasterEffect28_at1
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$7F
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    jmp RasterEndWorkBand
RasterEffect28_at0:
    ldx #$03
    lda #$A7
    jmp RasterBandThenLine
RasterEffect28_at1:
    lda a:RasterStep
    asl a
    clc
    adc #$67
    sta a:ScrollBgyLo+8
    lda a:RasterStep
    cmp #$01
    bne RasterEffect28_at2
    dec a:ScrollBgyLo+8
    dec a:ScrollBgyLo+8
RasterEffect28_at2:
    stz a:ScrollBgyHi+8
    lda a:ScrollBgxLo+3
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi+3
    sta a:ScrollBgxHi+8
    lda z:zFrameTick
    lsr a
    lsr a
    lsr a
    clc
    adc a:RasterStep
    lsr a
    and #$0F
    bne RasterEffect28_at3
    lda z:zRandom3
    and #$01
    inc a
    clc
    adc a:ScrollBgxLo+8
    sta a:ScrollBgxLo+8
    cla
    adc a:ScrollBgxHi+8
    sta a:ScrollBgxHi+8
RasterEffect28_at3:
    ldx #$08
    jsr WriteBandScroll
    lda #$06
    jsr SelectVdcRegister
    lda a:RasterStep
    asl a
    clc
    adc #$A9
    sta a:VDC_DATA_LO
    cla
    rol a
    sta a:VDC_DATA_HI
    inc a:RasterStep
    jmp RasterDone

; @routine 0045:02D RasterEffect30
; @run 69:502D
; @ends 4287
; WHAT IT DOES: Raster effect 30: five bands from tables.
; HOW: Band 1, three steps from the tables, then band 0 at Y $AF.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect30:
    lda a:RasterStep
    beq RasterEffect30_at0
    cmp #$04
    bne RasterEffect30_at1
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$AF
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    jmp RasterEndWorkBand
RasterEffect30_at0:
    ldx #$01
    lda #$8F
    jmp RasterBandThenLine
RasterEffect30_at1:
    tay
    ldx a:Effect30Bands,y
    lda a:ScrollBgxLo,x
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi,x
    sta a:ScrollBgxHi+8
    lda a:ScrollBgyLo,x
    clc
    adc a:Effect30YAdds,y
    sta a:ScrollBgyLo+8
    cla
    adc a:ScrollBgyHi,x
    sta a:ScrollBgyHi+8
    ldx #$08
    jsr WriteBandScroll
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$40
    clc
    adc a:Effect30Lines,y
    sta a:VDC_DATA_LO
    cla
    adc #$00
    sta a:VDC_DATA_HI
    jmp RasterDone

; @routine 0045:09B RasterEffect31
; @run 69:509B
; @ends 4287
; WHAT IT DOES: Raster effect 31: the background off from one line and back on at
; another, with band 1's Y set for the second.
; HOW: Background off and line $57; background on and line $E7; band 0.
; Entry: raster interrupt. Exit: to the frame interrupt's exit.
RasterEffect31:
    lda a:RasterStep
    beq RasterEffect31_at0
    cmp #$02
    bne RasterEffect31_at1
    lda a:ScrollBgxLo
    sta a:ScrollBgxLo+8
    lda a:ScrollBgxHi
    sta a:ScrollBgxHi+8
    lda #$A7
    sta a:ScrollBgyLo+8
    stz a:ScrollBgyHi+8
    jmp RasterEndWorkBand
RasterEffect31_at0:
    jsr SelectVdcControl
    lda z:zVdcCrLo
    and #$BF
    sta a:VDC_DATA_LO
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    lda #$57
    sta a:VDC_DATA_LO
    st2 #$00
    jmp RasterDone
RasterEffect31_at1:
    jsr SelectVdcControl
    lda z:zVdcCrLo
    ora #$40
    sta a:VDC_DATA_LO
    lda #$17
    sta a:ScrollBgyLo+1
    ldx #$01
    lda #$E7
    jmp RasterBandThenLine

; @routine 0045:0ED RasterSetNextLine
; @run 69:50ED
; @ends 4287
; WHAT IT DOES: Moves a raster effect on: the next band's compare line goes in.
; HOW: Counts the step, selects the compare register, writes the line.
; Entry: A line. Exit: to the frame interrupt's exit.
RasterSetNextLine:
    pha
    bra RasterBandThenLine_at0

; @routine 0045:0F0 RasterBandThenLine
; @run 69:50F0
; @ends 4287
; WHAT IT DOES: The usual raster step: writes a band's scroll, then the next band's
; compare line.
; HOW: WriteBandScroll, then runs into RasterSetNextLine's body.
; Entry: X band, A line. Exit: to the frame interrupt's exit.
RasterBandThenLine:
    pha
    jsr WriteBandScroll
RasterBandThenLine_at0:
    inc a:RasterStep
    lda #$06
    jsr SelectVdcRegister
    pla
    sta a:VDC_DATA_LO
    st2 #$00
    jmp RasterDone
