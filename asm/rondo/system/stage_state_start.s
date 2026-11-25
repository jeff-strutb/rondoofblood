; @routine 024E:02B EnterStageState
; @run 6B:882B
; WHAT IT DOES: The state a stage starts in: the section (zField86) becomes the one the stage loads
; (zField83), two stage bytes are cleared, bit 2 of zField8E is cleared and, when the stage number
; is negative, the System Card routine at $E018 is called until it answers zero and bit 3 of
; zField8E is set. Then, with the game running, the mode is counted done; otherwise the raster
; effect is cleared, the display delay set to 2 and the game sub mode cleared.
; HOW: A loop of the card call; the rest is stores.
; WAITS: it waits for the card as long as it answers.
; Entry: nothing. Exit: A the last value loaded, X the card's last answer when it was called.
; Seventeen stage overlays hold this code.
EnterStageState:
    lda z:zField86
    sta z:zField83
    stz z:zField8A
    stz z:zField8F
    rmb2 z:zField8E
    lda z:zStage
    bpl EnterStageState_at0
EnterStageState_at1:
    jsr BiosCdPause
    tax
    bne EnterStageState_at1
    smb3 z:zField8E
EnterStageState_at0:
    lda z:zGameRunning
    beq EnterStageState_at2
    inc z:zModeDone
    rts
EnterStageState_at2:
    stz z:zRasterEffect
    lda #$02
    sta z:zDisplayDelay
    stz z:zGameSubMode
    rts
