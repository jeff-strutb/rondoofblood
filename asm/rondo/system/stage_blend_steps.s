; @routine 024F:151 BlendStageStockRows
; @run 6B:9151
; WHAT IT DOES: Blends the stock number into the stage: with no blend running, its rows from step 0; with one running, marks the shown stock as done when the game is at mode 1.
; HOW: Saves and maps the stage banks, then by the blend flag either blends the stock's two rows with BlendStageRowPair or checks the stock shown against the stock; RestoreStageBanks ends it.
; The same instructions as 05BF's group's BlendStockRows, calling the bank routines at $933F and $9354: its body,
; blend_stock_rows_with (stock_display.c), with this group's bank routines.
; WAITS: no.
; Entry: X kept on the stack, Y the second row of the first blend. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
; else kept; X kept.
BlendStageStockRows:
    jsr SaveAndMapStageBanks
    lda a:StageBlendOn+1
    beq BlendStageStockRows_at0
    lda a:StageVar3616
    cmp z:zStock
    beq BlendStageStockRows_at1
    lda z:zGameSubMode
    cmp #$01
    bne BlendStageStockRows_at1
    smb1 z:zPaused
    lda #$C0
    sta a:StageVar3616+1
    jmp RestoreStageBanks
BlendStageStockRows_at0:
    phx
    stz a:StageBlendOn
    lda z:zStock
    sta a:StageVar3616
    clx
    jsr BlendStageRowPair
    plx
BlendStageStockRows_at1:
    jmp RestoreStageBanks

; @routine 024F:181 BlendStageScoreAddRows
; @run 6B:9181
; WHAT IT DOES: Blends the two bytes of the score being added into the stage (steps 3 and 5), when no blend is running.
; HOW: Saves and maps the stage banks, and when no blend is running calls BlendStageRowPair for each of the two bytes of the added score; RestoreStageBanks ends it.
; The same instructions as 05BF's group's BlendStageRowsOfPlayer, calling the bank routines at $933F and $9354: its body,
; blend_rows_of_player_with (stage_blend_long.c), with this group's bank routines.
; WAITS: no.
; Entry: X kept on the stack, Y the second row of the first blend. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
; else kept; X kept.
BlendStageScoreAddRows:
    jsr SaveAndMapStageBanks
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
    jmp RestoreStageBanks

; @routine 024F:1AC BlendStageScoreRows
; @run 6B:91AC
; WHAT IT DOES: Steps the score blend: with one running, blends the three score bytes' rows against the ones shown until six steps are made, when the shown bytes become the score's.
; HOW: Saves and maps the stage banks, steps the blend counter and calls BlendStageRowChanges for each of the three score bytes against the bytes shown; RestoreStageBanks ends it.
; WAITS: no.
; Entry: X kept on the stack. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
; else kept; X kept.
BlendStageScoreRows:
    jsr SaveAndMapStageBanks
    lda a:StageBlendOn+1
    beq BlendStageScoreRows_at0
    sta a:StageBlendOn
    lda a:StageBlendStep
    bne BlendStageScoreRows_at1
    lda z:zGameSubMode
    cmp #$06
    beq BlendStageScoreRows_at2
    bbr1 zPaused,BlendStageScoreRows_at3
BlendStageScoreRows_at2:
    lda #$05
    sta a:StageBlendStep
BlendStageScoreRows_at3:
    lda a:StageVar350E
    cmp z:zScore0
    bne BlendStageScoreRows_at1
    lda a:StageVar3566
    cmp z:zScore1
    bne BlendStageScoreRows_at1
    lda a:StageVar35BE
    cmp z:zScore2
    beq BlendStageScoreRows_at0
    inc a:StageBlendStep
    lda z:zScore0
    sta a:StageVar371E
    lda z:zScore1
    sta a:OBJ_DISSOLVE_PHASE_K+86
    lda z:zScore2
    sta a:OBJ_DISSOLVE_PHASE_L+86
BlendStageScoreRows_at1:
    phx
    lda a:StageVar350E
    sta z:zBiosAl
    lda z:zScore0
    ldx #$01
    jsr BlendStageRowChanges
    lda a:StageVar3566
    sta z:zBiosAl
    lda z:zScore1
    ldx #$03
    jsr BlendStageRowChanges
    lda a:StageVar35BE
    sta z:zBiosAl
    lda z:zScore2
    ldx #$05
    jsr BlendStageRowChanges
    plx
    inc a:StageBlendStep
    lda a:StageBlendStep
    cmp #$06
    bne BlendStageScoreRows_at0
    stz a:StageBlendStep
    lda z:zScore0
    sta a:StageVar350E
    lda z:zScore1
    sta a:StageVar3566
    lda z:zScore2
    sta a:StageVar35BE
BlendStageScoreRows_at0:
    jmp RestoreStageBanks

; @routine 05BF:1AC BlendStageScoreRowsLong
; @run 6B:91AC
; WHAT IT DOES: BlendStageScoreRows in 05BF's group of stage overlays, which keeps its bank routines at $9651 and $9666.
; HOW: The same instructions, calling SaveAndMapStageBanksLong and RestoreStageBanksLong.
; WAITS: no.
; Entry: X kept on the stack. Exit: A the bank restored, N and Z from it, C and V as the steps left them, Y 5 once a blend ran,
; else kept; X kept.
BlendStageScoreRowsLong:
    jsr SaveAndMapStageBanksLong
    lda a:StageBlendOn+1
    beq BlendStageScoreRowsLong_at0
    sta a:StageBlendOn
    lda a:StageBlendStep
    bne BlendStageScoreRowsLong_at1
    lda z:zGameSubMode
    cmp #$06
    beq BlendStageScoreRowsLong_at2
    bbr1 zPaused,BlendStageScoreRowsLong_at3
BlendStageScoreRowsLong_at2:
    lda #$05
    sta a:StageBlendStep
BlendStageScoreRowsLong_at3:
    lda a:StageVar350E
    cmp z:zScore0
    bne BlendStageScoreRowsLong_at1
    lda a:StageVar3566
    cmp z:zScore1
    bne BlendStageScoreRowsLong_at1
    lda a:StageVar35BE
    cmp z:zScore2
    beq BlendStageScoreRowsLong_at0
    inc a:StageBlendStep
    lda z:zScore0
    sta a:StageVar371E
    lda z:zScore1
    sta a:$3776
    lda z:zScore2
    sta a:$37CE
BlendStageScoreRowsLong_at1:
    phx
    lda a:StageVar350E
    sta z:zBiosAl
    lda z:zScore0
    ldx #$01
    jsr BlendStageRowChanges
    lda a:StageVar3566
    sta z:zBiosAl
    lda z:zScore1
    ldx #$03
    jsr BlendStageRowChanges
    lda a:StageVar35BE
    sta z:zBiosAl
    lda z:zScore2
    ldx #$05
    jsr BlendStageRowChanges
    plx
    inc a:StageBlendStep
    lda a:StageBlendStep
    cmp #$06
    bne BlendStageScoreRowsLong_at0
    stz a:StageBlendStep
    lda z:zScore0
    sta a:StageVar350E
    lda z:zScore1
    sta a:StageVar3566
    lda z:zScore2
    sta a:StageVar35BE
BlendStageScoreRowsLong_at0:
    jmp RestoreStageBanksLong
