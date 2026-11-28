; @routine 01BA:68B RichterDebrisOnMarkedColumns
; @run 6D:AE8B
; WHAT IT DOES: When a column read under him has bit 3: sound $23; each of slots 2 down to 1 holding
; kind 1 emptied, with script 8; a burst of debris 16 pixels below him.
; HOW: RichterColumnHit's three bytes ORed; SoundCommand and ClearObject through their kernel
; vectors, RichterStartHitScript. The script leaves X 7, so the count goes on down from 6 after it.
; His Y is pushed, moved down for RichterStartDebrisBurst with timer 0, and pulled back.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: no bit 3: A 0, N clear, Z set; X, Y, C and V kept.
; Otherwise A his Y low byte, N and Z from it; X 0; Y 8 if a slot was emptied, else kept; C as
; RichterStartDebrisBurst; V from adding the carry to his Y high byte.
RichterDebrisOnMarkedColumns:
    lda a:RichterColumnHit
    ora a:RichterColumnHit+1
    ora a:RichterColumnHit+2
    and #$08
    bne RichterDebrisOnMarkedColumns_at0
    rts
RichterDebrisOnMarkedColumns_at0:
    lda #$23
    jsr VecSoundCommand
    ldx #$02
RichterDebrisOnMarkedColumns_at2:
    lda a:OBJ_KIND,x
    dec a
    bne RichterDebrisOnMarkedColumns_at1
    jsr VecClearObject
    lda #$08
    jsr RichterStartHitScript
RichterDebrisOnMarkedColumns_at1:
    dex
    bne RichterDebrisOnMarkedColumns_at2
    lda a:OBJ_Y_LO
    pha
    clc
    adc #$10
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    pha
    adc #$00
    sta a:OBJ_Y_HI
    cla
    bsr RichterStartDebrisBurst
    pla
    sta a:OBJ_Y_HI
    pla
    sta a:OBJ_Y_LO
    rts
