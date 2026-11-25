; @routine 024E:7C4 StepStageEnding
; @run 6B:8FC4
; WHAT IT DOES: One step of a stage's ending: starts the CD track again on the first step unless
; the stage's mode is one of two, drops two objects along tables of speeds and times (a step
; each when their timers run out; the last step of the first sets the done flag in zField8E), and
; on the sixth step of the second draws a fade of random columns.
; HOW: Two timers count down in the f3510 column's slots 84 and 85; the tables at $9049 and $904F
; give each step's speed and timer; the fade is MaskVramRowsRandom called with the table row
; the picked step names.
; Every stage overlay holds the same code here (the fade picks the overlay's mask routine).
; WAITS: no.
; Entry: X and Y as the CD track routine keeps them. Exit: as the last step left it.
StepStageEnding:
    lda a:$3614
    bne StepStageEnding_at0
    lda z:zStage
    beq StepStageEnding_at1
    cmp #$0C
    beq StepStageEnding_at1
    cmp #$10
    beq StepStageEnding_at1
    jsr VecCdPlayAgain
StepStageEnding_at1:
    inc a:$3614
StepStageEnding_at0:
    dec a:$3565
    bne StepStageEnding_at2
    inc a:$350D
    ldy a:$350D
    cpy #$07
    bne StepStageEnding_at3
    stz a:$2ABD
    smb0 z:zField8E
    rts
StepStageEnding_at3:
    lda a:$9049,y
    lsr a
    sta a:$2E85
    cla
    ror a
    sta a:$2EDD
    lda a:$904F,y
    sta a:$3565
StepStageEnding_at2:
    lda a:$350D
    cmp #$06
    bne StepStageEnding_at4
    lda a:$3565
    cmp #$1F
    bne StepStageEnding_at5
    stz a:$2ABC
StepStageEnding_at5:
    sta a:$35BC
StepStageEnding_at4:
    ldx #$55
    jsr VecApplyVelocityY
    dec a:$3564
    bne StepStageEnding_at6
    inc a:$350C
    ldy a:$350C
    lda a:$9049,y
    lsr a
    sta a:$2DD4
    cla
    ror a
    sta a:$2E2C
    lda a:$904F,y
    sta a:$3564
StepStageEnding_at6:
    ldx #$54
    jsr VecApplyVelocityX
    lda a:$350C
    cmp #$06
    beq StepStageEnding_at7
    rts
StepStageEnding_at7:
    lda a:$35BC
    bra StepStageEnding_at8
    tsb z:zTemp03
    sxy
    ora (zTemp00,x)
    brk
    bpl StepStageEnding_at9
    bpl StepStageEnding_at10
    jsr $003F
    jmp (KernelZeros+2)
    rti
    adc a:$4001
    bra $90CD
    ora z:zField80
StepStageEnding_at9:
    cpy #$6D
StepStageEnding_at10:
    ora z:zField80
StepStageEnding_at8:
    sta z:SpritesLeft
    cmp #$20
    bcc StepStageEnding_at11
    and #$1F
    lsr a
    bcc StepStageEnding_at12
    clc
    adc #$10
StepStageEnding_at12:
    adc #$20
StepStageEnding_at11:
    pha
    and #$0F
    sta z:zTemp10
    lda #$10
    sec
    sbc z:zTemp10
    sta z:SpritesLeft
    pla
    lsr a
    lsr a
    and #$0C
    tax
    lda a:$9056,x
    sta z:zTemp10
    lda a:$9057,x
    sta z:zTemp11
    lda a:$9058,x
    sta z:zTemp12
    lda a:$9059,x
    sta z:zProduct0
StepStageEnding_at14:
    jsr VecNextRandom
    bsr MaskVramRowsRandom
    lda z:zTemp10
    clc
    adc z:zProduct0
    sta z:zTemp10
    bcc StepStageEnding_at13
    inc z:zTemp11
StepStageEnding_at13:
    dec z:zTemp12
    bne StepStageEnding_at14
    rts
