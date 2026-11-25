; @routine 05BF:181 BlendStageRowsOfPlayer
; @run 6B:9181
; WHAT IT DOES: Maps the stage's banks and, unless the byte at $366F (StageVars blend_on[1]) is set, clears the blend step $366E and blends the two row pairs the bytes zScoreAddLo and zScoreAddHi name
; (steps 3 and 5); then puts the banks back.
; HOW: SaveAndMapStageBanksLong, BlendStageRowPair twice with X pushed around them, a tail jump into RestoreStageBanksLong.
; WAITS: no.
; Entry: X kept, Y the second row of the first blend. Exit: A the bank restored, N and Z from it; C and V as the last blend or the bank add left them.
BlendStageRowsOfPlayer:
    jsr SaveAndMapStageBanksLong
    lda a:StageBlendOn+1
    bne $917E
    phx
    stz a:StageBlendOn
    lda z:zScoreAddLo
    ldx #$03
    jsr BlendStageRowPair
    lda z:zScoreAddHi
    ldx #$05
    jsr BlendStageRowPair
    plx
    jmp RestoreStageBanksLong
