; @routine 051B:23D LoadRecordA04
; @run 70:B23D
; WHAT IT DOES: Loads the object's record.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: as LoadObjectRecord.
LoadRecordA04:
    jsr VecLoadObjectRecord
    rts

; @routine 051B:76A LoadRecordB04
; @run 70:B76A
; WHAT IT DOES: Loads the object's record.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: as LoadObjectRecord.
LoadRecordB04:
    jsr VecLoadObjectRecord
    rts
