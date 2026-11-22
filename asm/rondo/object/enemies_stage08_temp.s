; TEMPORARY: the stage 08 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
;
;
;
;
;
;
;
; TEMPORARY: the stage 08 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
;
;
;
;
;
;
;
; @routine 0D5B:200 SetPhase2
; @run 70:B200
; WHAT IT DOES: Sets the object's f34b8 to 2, the phase SetSpeedFromPhaseTable reads.
; HOW: One store.
; WAITS: no.
; Entry: X object. Exit: A 2, N and Z clear; X, Y, C and V kept.
SetPhase2:
    lda #$02
    sta a:OBJ_34B8,x
    rts

; @routine 0D5B:206 SetPhase3
; @run 70:B206
; WHAT IT DOES: Sets the object's f34b8 to 3, the phase SetSpeedFromPhaseTable reads.
; HOW: One store.
; WAITS: no.
; Entry: X object. Exit: A 3, N and Z clear; X, Y, C and V kept.
SetPhase3:
    lda #$03
    sta a:OBJ_34B8,x
    rts

; @routine 0D5B:20C SetPhase4
; @run 70:B20C
; WHAT IT DOES: Sets the object's f34b8 to 4, the phase SetSpeedFromPhaseTable reads.
; HOW: One store.
; WAITS: no.
; Entry: X object. Exit: A 4, N and Z clear; X, Y, C and V kept.
SetPhase4:
    lda #$04
    sta a:OBJ_34B8,x
SetPhase4_at0:
    rts

; @routine 0D5B:212 SetSpeedFromPhaseTable
; @run 70:B212
; WHAT IT DOES: Sets the object's X and Y speeds from a table of four byte entries, the entry
; picked by the object's phase (f34b8) from a block chosen by the way it faces, and moves the
; object on to its next state.
; HOW: A table at overlay address $B23F gives each facing's block start; the entry's bytes are
; at $B241 to $B244 from there (X low, X high, Y low, Y high; the loads run high byte first).
; The block start passes through zTemp00.
; WAITS: no.
; Entry: X object. Exit: A the last byte loaded; N, Z from the new state; C and V from the add
; of the block start; Y the entry's offset; X kept. Clobbers zTemp00.
SetSpeedFromPhaseTable:
    ldy a:OBJ_2AC0,x
    lda a:$B23F,y
    sta z:zTemp00
    lda a:OBJ_34B8,x
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:$B242,y
    sta a:OBJ_VX_HI,x
    lda a:$B241,y
    sta a:OBJ_VX_LO,x
    lda a:$B244,y
    sta a:OBJ_VY_HI,x
    lda a:$B243,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_27A8,x
    rts

; @routine 0D5E:14F EnterState6IfFlaggedB
; @run 71:C94F
; WHAT IT DOES: Sends the object to state 6 when bit 7 of its status flags is set (the same as
; EnterState6IfFlagged, in this stage's overlay).
; HOW: Reads the flags once and masks bit 7; writes the state only when it is set.
; WAITS: no.
; Entry: X object. Exit: A the masked bit, or 6 when entered; N, Z from it; X, Y and C kept.
EnterState6IfFlaggedB:
    lda a:OBJ_31A0,x
    and #$80
    beq EnterState6IfFlaggedB_at0
    lda #$06
    sta a:OBJ_27A8,x
EnterState6IfFlaggedB_at0:
    rts

; @routine 0D5E:171 AdvanceState
; @run 71:C971
; WHAT IT DOES: Moves the object on to its next state.
; HOW: One increment of the state byte.
; WAITS: no.
; Entry: X object. Exit: N, Z from the new state; A, X, Y, C and V kept.
AdvanceState:
    inc a:OBJ_27A8,x
    rts

; @routine 0D5E:6A6 EnterState3WithTallHitbox
; @run 71:CEA6
; WHAT IT DOES: Puts the object in state 3 with a narrow, tall hit rectangle (half width 6, half
; height $18, no offsets), f3358 set to 2 and its step byte cleared.
; HOW: Constant stores.
; WAITS: no.
; Entry: X object. Exit: A 3, N and Z clear; X, Y, C and V kept.
EnterState3WithTallHitbox:
    lda #$02
    sta a:OBJ_3358,x
    lda #$06
    sta a:OBJ_3988,x
    lda #$18
    sta a:OBJ_3A38,x
    stz a:OBJ_3930,x
    stz a:OBJ_39E0,x
    lda #$03
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    rts

; @routine 0D5E:6EE SetHitboxFromFrameRowsB
; @run 71:CEEE
; WHAT IT DOES: Sets the object's whole hit rectangle from a four byte row of a table, the row
; chosen by the frame minus $5A; frames under $5A leave it alone.
; HOW: The table is at overlay address $CF12.
; WAITS: no.
; Entry: X object. Exit: A the last byte loaded (or the frame minus $5A when under); N, Z from
; it; C clear when under, else bit 6 of the row number; V from the subtract; Y the row's offset
; when read; X kept.
SetHitboxFromFrameRowsB:
    lda a:OBJ_2A68,x
    sec
    sbc #$5A
    bcc SetHitboxFromFrameRowsB_at0
    asl a
    asl a
    tay
    lda a:$CF12,y
    sta a:OBJ_3930,x
    lda a:$CF13,y
    sta a:OBJ_39E0,x
    lda a:$CF14,y
    sta a:OBJ_3988,x
    lda a:$CF15,y
    sta a:OBJ_3A38,x
SetHitboxFromFrameRowsB_at0:
    rts
