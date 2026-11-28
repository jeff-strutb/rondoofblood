; @routine 01E1:385 MariaObjectMarkFromPlayer
; @run 6E:C385
; WHAT IT DOES: Marks an object on (sprite attribute bit 4), then gives it her flip and facing. Nothing is known to
; reach it: the code map follows no path into $C385.
; HOW: A BSR to MariaObjectMarkOn, then on into MariaObjectFromPlayer.
; WAITS: no.
; Entry: X the object. Exit: as MariaObjectFromPlayer.
MariaObjectMarkFromPlayer:
    bsr MariaObjectMarkOn

; @routine 01E1:558 MariaObjectAddPlayerHitbox
; @run 6E:C558
; WHAT IT DOES: Moves an object by her hitbox's offsets: her OBJ_HITBOX_DX added to its X and her OBJ_HITBOX_DY to its
; Y, each sign-extended to 16 bits. Nothing is known to reach it: the code map follows no path into $C558.
; HOW: zTemp00 the high byte ($FF for a negative offset), the 16-bit adds.
; WAITS: no.
; Entry: X the object. Exit: A its Y's high byte, N, Z, C and V from the add. X and Y kept.
MariaObjectAddPlayerHitbox:
    stz z:zTemp00
    lda a:OBJ_HITBOX_DX
    bpl MariaObjectAddPlayerHitbox_at0
    dec z:zTemp00
MariaObjectAddPlayerHitbox_at0:
    clc
    adc a:OBJ_X_LO,x
    sta a:OBJ_X_LO,x
    lda z:zTemp00
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:OBJ_HITBOX_DY
    bpl MariaObjectAddPlayerHitbox_to1
    dec z:zTemp00
MariaObjectAddPlayerHitbox_to1:
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    lda z:zTemp00
    adc a:OBJ_Y_HI,x
    sta a:OBJ_Y_HI,x
    rts

; @routine 01E1:724 MariaRunObjects
; @run 6E:C724
; WHAT IT DOES: A frame of her objects: her companions in slots 1 and 2, then the objects of slots 3 to 15.
; HOW: MariaRunCompanions; a BSR to MariaRunObjectSlot for each slot, X as the handler left it, one up, until $10.
; WAITS: no.
; Entry: nothing. Exit: X $10, Z and C set, N clear; A, Y and V as the last handler left them.
MariaRunObjects:
    jsr MariaRunCompanions
    ldx #$03
MariaRunObjects_to1:
    bsr MariaRunObjectSlot
    inx
    cpx #$10
    bne MariaRunObjects_to1
    rts

; @routine 01E1:731 MariaRunObjectSlot
; @run 6E:C731
; WHAT IT DOES: Runs one object: zRichterObject its slot, then its kind's handler from MariaObjectHandlers (kind 0
; ends at once).
; HOW: OBJ_KIND doubled, the JMP (table,X).
; WAITS: no.
; Entry: X the slot. Exit: the processor as the handler left it.
MariaRunObjectSlot:
    stx z:zRichterObject
    lda a:OBJ_KIND,x
    asl a
    tax
    jmp (MariaObjectHandlers,x)
