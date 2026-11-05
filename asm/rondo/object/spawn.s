; @routine 0047:0E6 LoadObjectRecord
; @run 6A:60E6
; WHAT IT DOES: Starts an object from the stage's record for its type: copies the
; record's twelve bytes into the object's fields and moves it on to its next
; state.
; HOW: The overlay's pointer at OverlayObjectRecords points to a table of records
; by type minus one. The fourth byte packs two fields, a nibble each.
; Entry: X object, its type set, stage overlay mapped. Exit: A the record's last
; byte, Y 11, C bit 3 of the fourth byte, N and Z from the state field's new value.
; X and V kept. Clobbers zTemp10-11, zMultiplierLo-Hi.
LoadObjectRecord:
    lda a:OverlayObjectRecords
    sta z:zMultiplierLo
    lda a:OverlayObjectRecords+1
    sta z:zMultiplierHi
    lda a:OBJ_2750,x
    dec a
    asl a
    tay
    lda (zMultiplierLo),y
    sta z:zTemp10
    iny
    lda (zMultiplierLo),y
    sta z:zTemp11
    cly
    lda (zTemp10),y
    sta a:OBJ_3358,x
    iny
    lda (zTemp10),y
    sta a:OBJ_3300,x
    iny
    lda (zTemp10),y
    sta a:OBJ_32A8,x
    iny
    lda (zTemp10),y
    and #$0F
    sta a:OBJ_3250,x
    lda (zTemp10),y
    lsr a
    lsr a
    lsr a
    lsr a
    sta a:OBJ_FIELDS_2EE0,x
    iny
    lda (zTemp10),y
    sta a:OBJ_3988,x
    iny
    lda (zTemp10),y
    sta a:OBJ_3A38,x
    iny
    lda (zTemp10),y
    sta a:OBJ_2A10,x
    iny
    lda (zTemp10),y
    sta a:OBJ_2A68,x
    iny
    lda (zTemp10),y
    sta a:OBJ_30F0,x
    iny
    lda (zTemp10),y
    sta a:OBJ_3148,x
    iny
    lda (zTemp10),y
    sta a:OBJ_31A0,x
    iny
    lda (zTemp10),y
    sta a:OBJ_31F8,x
    stz a:OBJ_3930,x
    stz a:OBJ_39E0,x
    inc a:OBJ_27A8,x
    rts

; @routine 0047:15C StartObjectAtPlace
; @run 6A:615C
; WHAT IT DOES: Starts a new object of a type where another is.
; HOW: Stores the type, then runs into CopyObjectPlace.
; Entry: A type, X source, Y new object. Exit: as CopyObjectPlace.
StartObjectAtPlace:
    sta a:OBJ_2750,y

; @routine 0047:15F CopyObjectPlace
; @run 6A:615F
; WHAT IT DOES: Puts one object where another is.
; HOW: Copies the six position bytes field by field.
; Entry: X source, Y destination. Exit: A the last byte, N and Z from it. X, Y, C
; and V kept.
CopyObjectPlace:
    lda a:OBJ_X_HI,x
    sta a:OBJ_X_HI,y
    lda a:OBJ_X_LO,x
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_FRAC,x
    sta a:OBJ_X_FRAC,y
    lda a:OBJ_Y_HI,x
    sta a:OBJ_Y_HI,y
    lda a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_FRAC,x
    sta a:OBJ_Y_FRAC,y
    rts

; @routine 0047:184 CopyObjectPlaceBack
; @run 6A:6184
; WHAT IT DOES: CopyObjectPlace in the other direction, from Y to X.
; HOW: Swaps X and Y around the call.
; Entry: Y source, X destination. Exit: as CopyObjectPlace.
CopyObjectPlaceBack:
    sxy
    jsr CopyObjectPlace
    sxy
    rts

; @routine 0047:18A StepObjectFrames
; @run 6A:618A
; WHAT IT DOES: Steps an object's animation list one frame. A list is pairs of
; frames to hold (0 ends it) and a graphic; the list starts over from nothing after
; its end. Says whether the graphic changed, and whether the list ended.
; HOW: Holds while the timer runs; at zero, reads the next pair by the object's
; position in the list, setting the timer one short for the frame just used.
; Entry: X object, zTemp10-11 the list. Exit: changed: A $FF, C 1, N 1, Z 0, Y the
; pair's second byte's index. Held: A $FF, C 0, N 1, Z 0, Y kept. Ended: A 0, C 0,
; N 0, Z 1, Y the pair's index. X and V kept.
StepObjectFrames:
    lda a:OBJ_3460,x
    bne StepObjectFrames_at0
    lda a:OBJ_3408,x
    asl a
    tay
    lda (zTemp10),y
    bne StepObjectFrames_at1
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    clc
    lda #$00
    rts
StepObjectFrames_at1:
    sta a:OBJ_3460,x
    iny
    lda (zTemp10),y
    sta a:OBJ_2A68,x
    inc a:OBJ_3408,x
    dec a:OBJ_3460,x
    sec
    lda #$FF
    rts
StepObjectFrames_at0:
    dec a:OBJ_3460,x
    clc
    lda #$FF
    rts

; @routine 0047:3C8 FindFreeSlotFrom30
; @run 6A:63C8
; WHAT IT DOES: Finds an empty object slot from $30 to $3F and empties it fully.
; HOW: The first with a zero type gets ClearObject.
; Entry: nothing. Exit: found, C 1, Y the slot, A 0, N 0, Z 1. None, C 0, Y $40, A
; the last type, N 0, Z 1. X and V kept.
FindFreeSlotFrom30:
    ldy #$30
FindFreeSlotFrom30_at1:
    lda a:OBJ_2750,y
    beq FindFreeSlotFrom30_at0
    iny
    cpy #$40
    bcc FindFreeSlotFrom30_at1
    clc
    rts
FindFreeSlotFrom30_at0:
    jsr VecClearObjectInY
    sec
    rts

; @routine 0047:3DB FindFreeSlotFrom20
; @run 6A:63DB
; WHAT IT DOES: Finds an empty object slot from $20 to $3F and empties it fully.
; HOW: As FindFreeSlotFrom30, starting lower.
; Entry and exit: as FindFreeSlotFrom30.
FindFreeSlotFrom20:
    ldy #$20
    bra FindFreeSlotFrom30_at1

; @routine 0047:3DF FindFreeSlotFrom50
; @run 6A:63DF
; WHAT IT DOES: Finds an empty object slot from $50 to $53 and empties it fully.
; HOW: As FindFreeSlotFrom30, in its own loop.
; Entry: nothing. Exit: as FindFreeSlotFrom30, with Y $54 for none.
FindFreeSlotFrom50:
    ldy #$50
FindFreeSlotFrom50_at0:
    lda a:OBJ_2750,y
    beq FindFreeSlotFrom30_at0
    iny
    cpy #$54
    bcc FindFreeSlotFrom50_at0
    clc
    rts
