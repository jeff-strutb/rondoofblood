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
