; @routine 0045:597 AddScoreByte
; @run 69:5597
; WHAT IT DOES: Adds up to 99 points to the score, which stops at 999999, and
; says whether that crossed the mark the middle digits watch for.
; HOW: A decimal sum through zScore0-2, old middle byte kept in zTemp00. The
; answer is whether bits 7 and 5 of that byte differ afterward.
; Entry: A points in BCD. Exit: C the answer, A those differing bits, N and Z
; from them, D 0. V, X and Y kept.
AddScoreByte:
    sed
    clc
    adc z:zScore0
    sta z:zScore0
    lda z:zScore1
    sta z:zTemp00
    cla
    bra AddScoreWord_at0

; @routine 0045:5A4 AddScoreWord
; @run 69:55A4
; WHAT IT DOES: Adds up to 9999 points to the score, as AddScoreByte does.
; HOW: The same sum with a middle byte to add.
; Entry: zTemp00-01 points in BCD. Exit: as AddScoreByte. Clobbers zTemp00.
AddScoreWord:
    sed
    lda z:zTemp00
    clc
    adc z:zScore0
    sta z:zScore0
    lda z:zScore1
    sta z:zTemp00
    lda z:zTemp01
AddScoreWord_at0:
    adc z:zScore1
    sta z:zScore1
    cla
    adc z:zScore2
    sta z:zScore2
    bcc AddScoreWord_at1
    lda #$99
    sta z:zScore0
    sta z:zScore1
    sta z:zScore2
AddScoreWord_at1:
    cld
    lda z:zTemp00
    eor z:zScore1
    and #$A0
    beq AddScoreWord_at2
    sec
    rts
AddScoreWord_at2:
    clc
    rts

; @routine 0046:303 BitOfIndex
; @run 69:5B03
; WHAT IT DOES: Turns an index into a bit set into its byte and mask.
; HOW: The byte is the index over 8; the mask a one shifted by the low bits.
; Entry: A index. Exit: A mask, Y byte, C 1 for bit 0 else 0, X kept with N and
; Z from it. V kept.
BitOfIndex:
    phx
    pha
    lsr a
    lsr a
    lsr a
    tay
    pla
    and #$07
    tax
    lda #$01
    cpx #$00
    beq BitOfIndex_at0
BitOfIndex_at1:
    asl a
    dex
    bne BitOfIndex_at1
BitOfIndex_at0:
    plx
    rts

; @routine 0046:319 RecordObjectEvent
; @run 69:5B19
; WHAT IT DOES: If an object is one the save remembers, marks its event.
; HOW: Bit 7 of the object's flags says it is; its event field says which.
; Entry: X object. Exit: unmarked, A 0, N 0, Z 1, the rest kept; else as
; RecordEvent.
RecordObjectEvent:
    lda a:OBJ_FLAGS_2F38,x
    and #$80
    beq RecordEvent_at0
    lda a:OBJ_EVENT_ID,x

; @routine 0046:323 RecordEvent
; @run 69:5B23
; WHAT IT DOES: Marks an event as having happened, in the bit set the save
; keeps: an object so marked does not come back.
; HOW: BitOfIndex, then ORs the mask into its byte of EventFlags.
; Entry: A event. Exit: A the byte stored, Y its index, N and Z from it, C as
; BitOfIndex. X and V kept.
RecordEvent:
    jsr BitOfIndex
    ora a:EventFlags,y
    sta a:EventFlags,y
RecordEvent_at0:
    rts
