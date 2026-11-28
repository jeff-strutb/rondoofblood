; @routine 050E:0FB SetUpStageScrollTwice
; @run 6B:88FB
; @stop 8952
; WHAT IT DOES: Sets up the stage's scroll from the section's record: keeps the camera's X for the entry, clears
; the scroll state, loads the entry and sets up the scroll; for a raster effect with a second set up, does it again
; for the second plane; then clears the state once more and goes on into the entry load.
; HOW: Block clears, then LoadStageEntry ($8952) around MapOverlayBank7, StageSetUpScroll and
; StageSetUpScrollByEffect in the overlay.
; WAITS: no.
; Entry: zField83 the section. Exit: as LoadStageEntry at $8952.
SetUpStageScrollTwice:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    stz a:OBJ_SPRITE_FLIP
    lda z:zField8E
    pha
    bsr LoadStageEntryB
    pla
    sta z:zField8E
    jsr MapOverlayBank7
    jsr StageSetUpScroll
    ldy z:zRasterEffect
    lda a:EffectSecondSetUp,y
    beq SetUpStageScrollTwice_at0
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntryB
    pla
    sta z:zField8E
    jsr StageSetUpScrollByEffect
SetUpStageScrollTwice_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
