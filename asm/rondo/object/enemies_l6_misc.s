; @routine 072C:5A7 LoadRecordA06
; @run 70:BDA7
; WHAT IT DOES: Loads the object's record.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: as LoadObjectRecord.
LoadRecordA06:
    jsr VecLoadObjectRecord
    rts

; @routine 092E:2D1 ClearHitsTaken
; @run 6B:8AD1
; WHAT IT DOES: Sets the object's hits taken (OBJ_33B0) to 0.
; HOW: A store.
; WAITS: no.
; Entry: X object. Exit: all kept.
ClearHitsTaken:
    stz a:OBJ_33B0,x
    rts
; @routine 0E0B:5DD NextPhaseUnlessKind25
; @run 70:B5DD
; WHAT IT DOES: Goes on to the next OBJ_2800 unless the object in slot $50 is of kind $25.
; HOW: A compare and an INC.
; WAITS: no.
; Entry: X object. Exit: the compare, or the new OBJ_2800's N and Z.
NextPhaseUnlessKind25:
    lda a:$27A0
    cmp #$25
    beq NextPhaseUnlessKind25_at0
    inc a:OBJ_2800,x
NextPhaseUnlessKind25_at0:
    rts

; @routine 12DF:038 RandomTimerC0
; @run 71:D038
; WHAT IT DOES: Sets the object's timer OBJ_34B8 to a random number with bits 6 and 7 set.
; HOW: NextRandom.
; WAITS: no.
; Entry: X object. Exit: A the timer, N set, Z clear; C and V from the random number.
RandomTimerC0:
    jsr VecNextRandom
    ora #$C0
    sta a:OBJ_34B8,x
    rts
