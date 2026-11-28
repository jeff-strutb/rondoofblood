; @routine 051B:526 CountdownNextStepA
; @run 70:B526
; WHAT IT DOES: Waits out the object's timer (OBJ_WORK_D) and then moves it on: once the timer has
; run down, the object's step byte (OBJ_SUBSTATE) goes up by one.
; HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_SUBSTATE.
; WAITS: no.
; Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
CountdownNextStepA:
    dec a:OBJ_WORK_D,x
    bne CountdownNextStepA_at0
    inc a:OBJ_SUBSTATE,x
CountdownNextStepA_at0:
    rts

; @routine 05CB:209 CountdownNextStateA
; @run 70:B209
; WHAT IT DOES: Waits out the object's timer (OBJ_WORK_E) and then moves it on: once the timer has
; run down, the object's state byte (the index into its handler table) goes up by one.
; HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
; WAITS: no.
; Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
CountdownNextStateA:
    dec a:OBJ_WORK_E,x
    bne CountdownNextStateA_at0
    inc a:OBJ_STATE,x
CountdownNextStateA_at0:
    rts

; @routine 07DB:732 CountdownNextStateB
; @run 70:B732
; WHAT IT DOES: Waits out the object's timer (OBJ_WORK_D) and then moves it on: once the timer has
; run down, the object's state byte (the index into its handler table) goes up by one.
; HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
; WAITS: no.
; Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
CountdownNextStateB:
    dec a:OBJ_WORK_D,x
    bne CountdownNextStateB_at0
    inc a:OBJ_STATE,x
CountdownNextStateB_at0:
    rts

; @routine 093D:45E CountdownNextStepB
; @run 71:C45E
; WHAT IT DOES: Waits out the object's timer (OBJ_WORK_G) and then moves it on: once the timer has
; run down, the object's step byte (OBJ_SUBSTATE) goes up by one.
; HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_SUBSTATE.
; WAITS: no.
; Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
CountdownNextStepB:
    dec a:OBJ_WORK_G,x
    bne CountdownNextStepB_at0
    inc a:OBJ_SUBSTATE,x
CountdownNextStepB_at0:
    rts

; @routine 09EC:011 CountdownNextStateC
; @run 70:B811
; WHAT IT DOES: Waits out the object's timer (OBJ_WORK_D) and then moves it on: once the timer has
; run down, the object's state byte (the index into its handler table) goes up by one.
; HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
; WAITS: no.
; Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
CountdownNextStateC:
    dec a:OBJ_WORK_D,x
    bne CountdownNextStateC_at0
    inc a:OBJ_STATE,x
CountdownNextStateC_at0:
    rts

; @routine 0CAC:6FA CountdownNextStateD
; @run 70:BEFA
; WHAT IT DOES: Waits out the object's timer (OBJ_WORK_G) and then moves it on: once the timer has
; run down, the object's state byte (the index into its handler table) goes up by one.
; HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
; WAITS: no.
; Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
CountdownNextStateD:
    dec a:OBJ_WORK_G,x
    bne CountdownNextStateD_at0
    inc a:OBJ_STATE,x
CountdownNextStateD_at0:
    rts

; @routine 117C:101 CountdownNextStateE
; @run 70:B901
; WHAT IT DOES: Waits out the object's timer (OBJ_WORK_D) and then moves it on: once the timer has
; run down, the object's state byte (the index into its handler table) goes up by one.
; HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_STATE.
; WAITS: no.
; Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
CountdownNextStateE:
    dec a:OBJ_WORK_D,x
    bne RichterLatchButtons
    inc a:OBJ_STATE,x
    rts

; @routine 117D:020 CountdownNextStepC
; @run 71:C020
; WHAT IT DOES: Waits out the object's timer (OBJ_WORK_G) and then moves it on: once the timer has
; run down, the object's step byte (OBJ_SUBSTATE) goes up by one.
; HOW: Counts the timer down; if it is not zero it stops, otherwise it adds one to OBJ_SUBSTATE.
; WAITS: no.
; Entry: X object. Exit: N and Z from the last byte written; A, X, Y, C and V kept.
CountdownNextStepC:
    dec a:OBJ_WORK_G,x
    bne CountdownNextStepC_at1
CountdownNextStepC_at0:
    inc a:OBJ_SUBSTATE,x
CountdownNextStepC_at1:
    rts
