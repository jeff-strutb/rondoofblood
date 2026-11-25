; @routine 0519:6EC GiveExtraLife
; @run 70:A6EC
; WHAT IT DOES: Plays sound $52, adds one to the player's stock (zStock, decimal, at most 99) and shows it (BlendStockRows, by a tail jump).
; HOW: SoundCommand, a decimal add.
; Untraced: the whole routine (no script ran it); the draft does not exist.
; WAITS: no.
; Entry: X kept, Y the sound driver's. Exit: as BlendStockRows.
GiveExtraLife:
    lda #$52
    jsr VecSoundCommand
    sed
    lda z:zStock
    clc
    adc #$01
    sta z:zStock
    cld
    bcc GiveExtraLife_at0
    lda #$99
    sta z:zStock
GiveExtraLife_at0:
    jmp BlendStockRows

; @routine 05BF:151 BlendStockRows
; @run 6B:9151
; WHAT IT DOES: Maps the stage's banks and puts the player's stock on the stage's tile rows: with the blend off ($366F clear) it clears the blend step $366E, notes the stock in StageVar3616 and
; blends the first row pair with the stock as the byte; with the blend on, it only asks for the pause bit (zPaused bit 1) and the value $C0 in StageVar3617 when the sub-mode is 1 and the noted stock differs;
; then puts the banks back.
; HOW: SaveAndMapStageBanksLong, BlendStageRowPair, a tail jump into RestoreStageBanksLong.
; Untraced: the arm that sets bit 1 of zPaused (9166).
; WAITS: no.
; Entry: X kept, Y the second row of the blend. Exit: A the bank restored, N and Z from it; C and V as the last blend or the bank add left them.
BlendStockRows:
    jsr SaveAndMapStageBanksLong
    lda a:StageBlendOn+1
    beq BlendStockRows_at0
    lda a:StageVar3616
    cmp z:zStock
    beq BlendStockRows_at1
    lda z:zGameSubMode
    cmp #$01
    bne BlendStockRows_at1
    smb1 z:zPaused
    lda #$C0
    sta a:StageVar3616+1
    jmp RestoreStageBanksLong
BlendStockRows_at0:
    phx
    stz a:StageBlendOn
    lda z:zStock
    sta a:StageVar3616
    clx
    jsr BlendStageRowPair
    plx
BlendStockRows_at1:
    jmp RestoreStageBanksLong
