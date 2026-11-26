; @card 00:E6CF run_bank2_tasks
; WHAT IT DOES: Runs the card's CD player tasks in its bank 2: the first (at $CD12) when CardPlayerMode is not zero, the second (at $C2EF)
; unless it is 1; each that answers 0 marks itself done in CardPlayerDone ($22DE: bit 6 for the first, bit 7 for the second). Then
; the PSG's timer is loaded from $22D4 once, unless the reset mark's bit 0 is set, and the PSG's channel select put back.
; HOW: MPR4, MPR5 and MPR6 kept on the stack; MPR4 and MPR5 set from $22EB and $22EC, MPR6 to bank 2; the tasks called; the MPRs
; pulled back.
; WAITS: no.
; Entry: X, Y (the interrupted code's). Exit: A $22ED's byte, N and Z from it, C and V as the last compare left them. X, Y as the tasks left them.
; The tasks themselves, in bank 2, are original code: no script runs them.
run_bank2_tasks:
    tma #$10
    pha
    tma #$20
    pha
    tma #$40
    pha
    lda a:$22EB
    tam #$10
    lda a:$22EC
    tam #$20
    lda #$02
    tam #$40
    lda a:$22DF
    beq run_bank2_tasks_at0
    jsr $CD12
    cmp #$00
    bne run_bank2_tasks_at0
    lda a:$22DE
    ora #$40
    sta a:$22DE
run_bank2_tasks_at0:
    lda a:$22DF
    cmp #$01
    beq run_bank2_tasks_at1
    jsr $C2EF
    cmp #$00
    bne run_bank2_tasks_at1
    lda a:$22DE
    ora #$80
    sta a:$22DE
run_bank2_tasks_at1:
    lda z:CardResetMark
    and #$01
    bne run_bank2_tasks_at2
    lda a:$22D4
    bmi run_bank2_tasks_at2
    sta a:$0C00
    ora #$80
    sta a:$22D4
run_bank2_tasks_at2:
    pla
    tam #$40
    pla
    tam #$20
    pla
    tam #$10
    lda a:$22ED
    sta a:$0800
    rts
