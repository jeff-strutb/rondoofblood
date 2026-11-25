; @routine 050E:2A4 StageTriggerObject
; @run 6B:8AA4
; WHAT IT DOES: A stage object that is started by a hit. Its first frame loads its record and
; keeps the record's byte split in three (a number, an angle and a count); every later frame, when
; the pad overrides allow, hit-taken is under five and the hit-taken byte is set, it hands its two
; numbers to the stage (zField83, zField87) and starts step 2.
; HOW: LoadObjectRecord, then rotates of the record's byte; the later frames read PadOverrides
; by the count.
; Five stage overlays hold this code.
; WAITS: no.
; Entry: X the object. Exit: as the last path left it.
StageTriggerObject:
    lda a:OBJ_27A8,x
    bne StageTriggerObject_at0
    jsr VecLoadObjectRecord
    lda #$80
    clc
    adc a:OBJ_3250,x
    sta a:OBJ_3250,x
    lda a:OBJ_3040,x
    tay
    and #$0F
    sta a:OBJ_3510,x
    tya
    rol a
    rol a
    rol a
    tay
    and #$03
    sta a:OBJ_3568,x
    tya
    rol a
    rol a
    and #$03
    sta a:OBJ_34B8,x
    rts
    stz a:OBJ_33B0,x
    rts
StageTriggerObject_at0:
    stz a:OBJ_3A90,x
    ldy a:OBJ_34B8,x
    lda a:PadOverrides,y
    beq ClearHitsTaken
    lda a:OBJ_33B0,x
    beq ClearHitsTaken
    cmp #$05
    bcs ClearHitsTaken
    lda a:OBJ_3510,x
    sta z:zField83
    lda a:OBJ_3568,x
    sta z:zField87
    lda #$01
    sta z:zField8C
    lda #$02
    sta z:zSubStep
    rts

; @routine 088B:784 ClearStageTimers
; @run 70:B784
; WHAT IT DOES: Clears four of the stage's object timers (slots 18 to 21 of the life timer column).
; HOW: Four stores of zero.
; WAITS: no.
; Entry: nothing. Exit: all registers and flags kept.
ClearStageTimers:
    stz a:$34CA
    stz a:$34CB
    stz a:$34CC
    stz a:$34CD
    rts

; @routine 092E:2FC StageTriggerObjectB
; @run 6B:8AFC
; WHAT IT DOES: StageTriggerObject's sibling for the stages that keep a five-bit number: the same first
; frame (a five-bit number and a count) and, later, it starts step 2 with a clear flag when the player
; may leave the top, and stops the object's speed.
; HOW: As StageTriggerObject, with PlayerMayLeaveTop in place of PadOverrides.
; Two stage overlays hold this code.
; WAITS: no.
; Entry: X the object. Exit: as the last path left it.
StageTriggerObjectB:
    lda a:OBJ_27A8,x
    bne StageTriggerObjectB_at0
    jsr VecLoadObjectRecord
    lda #$80
    clc
    adc a:OBJ_3250,x
    sta a:OBJ_3250,x
    lda a:OBJ_3040,x
    tay
    and #$1F
    sta a:OBJ_3510,x
    tya
    rol a
    rol a
    rol a
    and #$03
    sta a:OBJ_3568,x
StageTriggerObjectB_at2:
    rts
StageTriggerObjectB_at0:
    stz a:OBJ_3A90,x
    lda a:OBJ_3040,x
    bpl StageTriggerObjectB_at1
    lda a:PlayerByte3BD7
    beq ClearHitsTaken
StageTriggerObjectB_at1:
    lda a:PlayerMayLeaveTop
    dec a
    beq StageTriggerObjectB_at2
    lda a:OBJ_33B0,x
    beq ClearHitsTaken
    cmp #$05
    bcs ClearHitsTaken
    lda a:OBJ_3510,x
    sta z:zField83
    lda a:OBJ_3568,x
    sta z:zField87
    stz z:zField8C
    lda #$02
    sta z:zSubStep
    stz a:OBJ_VX_HI
    stz a:OBJ_VX_LO
    rts
