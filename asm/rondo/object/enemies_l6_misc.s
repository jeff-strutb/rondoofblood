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
; @routine 0E0B:099 HalveDirectionWhenHurt
; @run 70:B099
; WHAT IT DOES: Unless the object is in state 6, halves its OBJ_3618 once its hits taken reach 5.
; HOW: Two compares and a memory shift right.
; WAITS: no.
; Entry: X object. Exit: the last compare, or the shift's N, Z and C.
HalveDirectionWhenHurt:
    lda a:OBJ_27A8,x
    cmp #$06
    beq HalveDirectionWhenHurt_at0
    lda a:OBJ_33B0,x
    cmp #$05
    bcc HalveDirectionWhenHurt_at0
    lsr a:OBJ_3618,x
HalveDirectionWhenHurt_at0:
    rts

; @routine 0E0B:10B LoadThenSpawn24
; @run 70:B10B
; WHAT IT DOES: Loads the object's record, sets OBJ_3618 to $80, clears the object in slot $11 and starts a kind $24 object there at this object's place.
; HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A the last byte copied, Y $11.
LoadThenSpawn24:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_3618,x
    ldy #$11
    jsr VecClearObjectInY
    lda #$24
    jmp VecStartObjectAtPlace

; @routine 0E0B:184 FallThenStopForRaster
; @run 70:B184
; WHAT IT DOES: Moves the object along Y by its speed and counts OBJ_34B8 down; at zero it clears the raster effect and its argument, clears the object's Y speed and goes
; on to the next OBJ_2800.
; HOW: ApplyVelocityY, DEC, stores, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new OBJ_2800's N and Z.
FallThenStopForRaster:
    jsr VecApplyVelocityY
    dec a:OBJ_34B8,x
    bne FallThenStopForRaster_at0
    stz z:zRasterArg
    stz z:zRasterEffect
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_2800,x
FallThenStopForRaster_at0:
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

; @routine 0E0B:5E8 StepListB5FEToState4
; @run 70:B5E8
; WHAT IT DOES: Steps the animation list at $B5FE; when it ends, the object goes to state 4 and OBJ_2800 0.
; HOW: StepObjectFrames, then stores and an increment.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or A 4 when the list ended.
StepListB5FEToState4:
    lda #$FE
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ClampSpeedXToTwo
    lda #$04
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
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
