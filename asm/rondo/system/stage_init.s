; @routine 024E:0EC InitStage
; @run 6B:88EC
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry
; record, maps the object bank and runs the overlay's set-up at $A94C; when the section's
; effect byte has a second set-up, clears and loads again and runs the one at $A9B1; then
; clears the scroll state once more and runs on into the entry loader.
; Twenty-one stage overlays hold this code.
; HOW: ClearScreenState with the scroll kept in zBiosAl, block moves of zeros over SCROLL and
; SCROLLE, LoadStageEntry through the overlay's copy, MapOverlayBank7, and the overlay's own code.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStage:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStage_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStage_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    stz a:OBJ_SPRITE_FLIP
    lda z:zField8E
    pha
    bsr LoadStageEntry
    pla
    sta z:zField8E
    jsr MapOverlayBank7
    jsr StageSetUpScroll
    ldy z:zRasterEffect
    lda a:EffectSecondSetUp,y
    beq InitStage_at1
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntry
    pla
    sta z:zField8E
    jsr StageSetUpScrollByEffect
InitStage_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088

; @routine 10BE:0EC InitStageB
; @run 6B:88EC
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry
; record, maps the object bank and runs the overlay's set-up at $ACDC; when the section's
; effect byte has a second set-up, clears and loads again and runs the one at $AD41; then
; clears the scroll state once more and runs on into the entry loader.
; The same code as InitStage; this overlay's set-ups are at different addresses.
; HOW: ClearScreenState with the scroll kept in zBiosAl, block moves of zeros over SCROLL and
; SCROLLE, LoadStageEntry through the overlay's copy, MapOverlayBank7, and the overlay's own code.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStageB:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStageB_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStageB_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    stz a:OBJ_SPRITE_FLIP
    lda z:zField8E
    pha
    bsr LoadStageEntryR
    pla
    sta z:zField8E
    jsr MapOverlayBank7
    jsr $ACDC
    ldy z:zRasterEffect
    lda a:EffectSecondSetUp,y
    beq ScriptPlaceTitleObjectB
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntryR
    pla
    sta z:zField8E
    jsr $AD41
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088

; @routine 12CE:0EC InitStageC
; @run 6B:88EC
; WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry
; record, maps the object bank and runs the overlay's set-up at $ACD5; when the section's
; effect byte has a second set-up, clears and loads again and runs the one at $AD3A; then
; clears the scroll state once more and runs on into the entry loader.
; The same code as InitStage; this overlay's set-ups are at different addresses.
; HOW: ClearScreenState with the scroll kept in zBiosAl, block moves of zeros over SCROLL and
; SCROLLE, LoadStageEntry through the overlay's copy, MapOverlayBank7, and the overlay's own code.
; WAITS: no.
; Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
InitStageC:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    bra InitStageC_at0
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
InitStageC_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    stz a:OBJ_SPRITE_FLIP
    lda z:zField8E
    pha
    bsr LoadStageEntryT
    pla
    sta z:zField8E
    jsr MapOverlayBank7
    jsr $ACD5
    ldy z:zRasterEffect
    lda a:EffectSecondSetUp,y
    beq ScriptPlaceTitleObjectB
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntryT
    pla
    sta z:zField8E
    jsr $AD3A
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
