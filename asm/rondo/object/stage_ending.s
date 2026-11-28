; @routine 024E:7C4 StepStageEnding
; @run 6B:8FC4
; @stop 904A
; WHAT IT DOES: One step of a stage's ending: starts the CD track again on the first step unless
; the stage's mode is one of two, drops two objects along tables of speeds and times (a step
; each when their timers run out; the last step of the first sets the done flag in zField8E), and
; on the sixth step of the second draws a fade of random columns.
; HOW: Two timers count down in the f3510 column's slots 84 and 85; the tables at $9049 and $904F
; give each step's speed and timer; the fade is MaskVramRowsRandom called with the table row
; the picked step names (StepStageEndingFade, past the tables at $9049-$9065).
; Every stage overlay holds the same code here (the fade picks the overlay's mask routine).
; WAITS: no.
; Entry: X and Y as the CD track routine keeps them. Exit: as the last step left it.
StageEndingSpeeds = $9049 ; a step's speed by step, from 1
StageEndingTimers = $904F ; a step's timer by step, from 1
StepStageEnding:
    lda a:OBJ_STATE_BEFORE_HIT+84
    bne StepStageEnding_at0
    lda z:zStage
    beq StepStageEnding_at1
    cmp #$0C
    beq StepStageEnding_at1
    cmp #$10
    beq StepStageEnding_at1
    jsr VecCdPlayAgain
StepStageEnding_at1:
    inc a:OBJ_STATE_BEFORE_HIT+84
StepStageEnding_at0:
    dec a:OBJ_FLING_WAIT+85
    bne StepStageEnding_at2
    inc a:OBJ_DISSOLVE_CLOCK_D+85
    ldy a:OBJ_DISSOLVE_CLOCK_D+85
    cpy #$07
    bne StepStageEnding_at3
    stz a:OBJ_FRAME+85
    smb0 z:zField8E
    rts
StepStageEnding_at3:
    lda a:StageEndingSpeeds,y
    lsr a
    sta a:OBJ_VY_HI+85
    cla
    ror a
    sta a:OBJ_VY_LO+85
    lda a:StageEndingTimers,y
    sta a:OBJ_FLING_WAIT+85
StepStageEnding_at2:
    lda a:OBJ_DISSOLVE_CLOCK_D+85
    cmp #$06
    bne StepStageEnding_at4
    lda a:OBJ_FLING_WAIT+85
    cmp #$1F
    bne StepStageEnding_at5
    stz a:OBJ_FRAME+84
StepStageEnding_at5:
    sta a:OBJ_CHILD_ORDER+84
StepStageEnding_at4:
    ldx #$55
    jsr VecApplyVelocityY
    dec a:OBJ_FLING_WAIT+84
    bne StepStageEnding_at6
    inc a:OBJ_DISSOLVE_CLOCK_D+84
    ldy a:OBJ_DISSOLVE_CLOCK_D+84
    lda a:StageEndingSpeeds,y
    lsr a
    sta a:OBJ_VX_HI+84
    cla
    ror a
    sta a:OBJ_VX_LO+84
    lda a:StageEndingTimers,y
    sta a:OBJ_FLING_WAIT+84
StepStageEnding_at6:
    ldx #$54
    jsr VecApplyVelocityX
    lda a:OBJ_DISSOLVE_CLOCK_D+84
    cmp #$06
    beq StepStageEnding_at7
    rts
StepStageEnding_at7:
    lda a:OBJ_CHILD_ORDER+84
    bra StepStageEndingFade

; @routine 024E:866 StepStageEndingFade
; @run 6B:9066
; WHAT IT DOES: StepStageEnding's fade: masks columns of VRAM rows with random bytes, from the row of the fade
; table the pick names, the address moving on a column each.
; HOW: The pick folded into a row, four bytes of StageEndingFadeRows (address, columns, step); NextRandom and
; MaskVramRowsRandom per column.
; WAITS: no.
; Entry: A the pick (OBJ_CHILD_ORDER of slot 84). Exit: as the last column left it.
StageEndingFadeRows = $9056 ; four bytes a row: the VRAM address, the columns, the address step
StepStageEndingFade:
    sta z:SpritesLeft
    cmp #$20
    bcc StepStageEndingFade_at11
    and #$1F
    lsr a
    bcc StepStageEndingFade_at12
    clc
    adc #$10
StepStageEndingFade_at12:
    adc #$20
StepStageEndingFade_at11:
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
    lda a:StageEndingFadeRows,x
    sta z:zTemp10
    lda a:StageEndingFadeRows+1,x
    sta z:zTemp11
    lda a:StageEndingFadeRows+2,x
    sta z:zTemp12
    lda a:StageEndingFadeRows+3,x
    sta z:zProduct0
StepStageEndingFade_at14:
    jsr VecNextRandom
    bsr MaskVramRowsRandom
    lda z:zTemp10
    clc
    adc z:zProduct0
    sta z:zTemp10
    bcc StepStageEndingFade_at13
    inc z:zTemp11
StepStageEndingFade_at13:
    dec z:zTemp12
    bne StepStageEndingFade_at14
    rts
