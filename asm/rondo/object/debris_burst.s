; @routine 025A:10B StartDebrisBurst
; @run 70:A90B
; WHAT IT DOES: Starts up to six kind $9A objects at this object's place, each in the first free slot from $30, each given the
; timer in A and its own number 0 to 5 in OBJ_WORK_E; stops at the first search that finds no slot.
; HOW: Y is pushed and pulled back; zHitSlot keeps the count across the calls; FindFreeSlotFrom30, StartObjectAtPlace.
; WAITS: no.
; Entry: A the timer, X object, Y pushed and pulled back. Exit: Y as it came, N and Z from it; C 1 when all six were
; started, 0 when a search found no slot; A the last byte loaded.
StartDebrisBurst:
    sta z:zTemp00
    phy
    cly
StartDebrisBurst_at1:
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom30
    bcc StartDebrisBurst_at0
    lda z:zTemp00
    sta a:OBJ_WORK_D,y
    lda #$9A
    jsr VecStartObjectAtPlace
    lda z:zHitSlot
    sta a:OBJ_WORK_E,y
    ldy z:zHitSlot
    iny
    cpy #$06
    bcc StartDebrisBurst_at1
StartDebrisBurst_at0:
    ply
    rts

; @routine 01BA:6D0 RichterStartDebrisBurst
; @run 6D:AED0
; WHAT IT DOES: StartDebrisBurst in Richter's overlay, the same bytes at his address.
; HOW: StartDebrisBurst's body, with this copy's returns.
; WAITS: no.
; Entry, exit: as StartDebrisBurst.
RichterStartDebrisBurst:
    sta z:zTemp00
    phy
    cly
RichterStartDebrisBurst_at1:
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom30
    bcc RichterStartDebrisBurst_at0
    lda z:zTemp00
    sta a:OBJ_WORK_D,y
    lda #$9A
    jsr VecStartObjectAtPlace
    lda z:zHitSlot
    sta a:OBJ_WORK_E,y
    ldy z:zHitSlot
    iny
    cpy #$06
    bcc RichterStartDebrisBurst_at1
RichterStartDebrisBurst_at0:
    ply
    rts
