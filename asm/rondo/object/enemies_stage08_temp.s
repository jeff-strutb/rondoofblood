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

; @routine 0D5B:12B StartFacingPlayerThenCheck
; @run 70:B12B
; WHAT IT DOES: Starts the object from its record, turns it to face the player, sets f3510 to
; $18, and, when its next_kind is 1, either removes the object (the player under $20 pixels
; away along X) or sends it to state 6.
; HOW: LoadObjectRecord, SideOfPlayer, DistanceXToPlayer, ClearObject as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C from the last compare or load; V from the
; distance's subtract on the near path; Y 11 (the record); X kept. Clobbers zTemp00, zTemp10-11.
StartFacingPlayerThenCheck:
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    lda #$18
    sta a:OBJ_3510,x
    lda a:OBJ_3040,x
    cmp #$01
    bne StartFacingPlayerThenCheck_at0
    jsr VecDistanceXToPlayer
    cmp #$20
    bcs StartFacingPlayerThenCheck_at1
    jmp VecClearObject
StartFacingPlayerThenCheck_at1:
    lda #$06
    sta a:OBJ_27A8,x
StartFacingPlayerThenCheck_at0:
    rts

; @routine 0D5B:152 AnimateThenCountToState3
; @run 70:B152
; WHAT IT DOES: Unless the object in slot $54 is showing frame 4, animates the object along the
; list at overlay address $B17B and, when its next_kind is not $FF and its X high byte is zero,
; counts f3510 down; at zero it enters state 3.
; HOW: StepObjectFrames, then the tests in that order, each stopping at the first that fails.
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C from the last compare, load or decrement; Y as the
; animation step left it; X and V kept. Clobbers zTemp10-11.
AnimateThenCountToState3:
    lda a:$2ABC
    cmp #$04
    beq AnimateThenCountToState3_at0
    lda #$B1
    sta z:zTemp11
    lda #$7B
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_3040,x
    cmp #$FF
    beq AnimateThenCountToState3_at0
    lda a:OBJ_X_HI,x
    bne AnimateThenCountToState3_at0
    dec a:OBJ_3510,x
    bne AnimateThenCountToState3_at0
    lda #$03
    sta a:OBJ_27A8,x
AnimateThenCountToState3_at0:
    rts

; @routine 0D5B:1CE PickPhaseAtRandomA
; @run 70:B1CE
; WHAT IT DOES: Picks the object's phase (the f34b8 byte SetSpeedFromPhaseTable reads) from a
; random number's low three bits: 0 and 1 give phase 3, 2 gives phase 2, 3 gives phase 1, any
; other gives phase 0.
; HOW: NextRandom, compares, and a tail into SetPhase3 or SetPhase2.
; WAITS: no.
; Entry: X object, D clear. Exit: A the phase (or the masked number for phase 0), N, Z from it; C
; from the last compare (V from the random number's add).
PickPhaseAtRandomA:
    jsr VecNextRandom
    and #$07
    cmp #$02
    bcc SetPhase3
    cmp #$02
    beq SetPhase2
    cmp #$03
    beq PickPhaseAtRandomB_at0
PickPhaseAtRandomA_at0:
    stz a:OBJ_34B8,x
    rts

; @routine 0D5B:1E3 PickPhaseAtRandomB
; @run 70:B1E3
; WHAT IT DOES: Picks the object's phase from a random number's low three bits: 0 gives phase 3,
; 1 gives phase 2, 2 gives phase 0, 3 to 7 give phase 1 when the object faces the way the player
; does and phase 0 when not.
; HOW: NextRandom, compares, and a tail into SetPhase3 or SetPhase2.
; WAITS: no.
; Entry: X object, D clear. Exit: A the phase (or the facing difference), N, Z from it; C from the
; last compare (V from the random number's add).
PickPhaseAtRandomB:
    jsr VecNextRandom
    and #$07
    beq SetPhase3
    cmp #$01
    beq SetPhase2
    cmp #$02
    beq PickPhaseAtRandomA_at0
    lda a:OBJ_2AC0,x
    eor a:OBJ_2AC0
    bne PickPhaseAtRandomA_at0
PickPhaseAtRandomB_at0:
    lda #$01
    sta a:OBJ_34B8,x
    rts

; @routine 0D5E:15C StartWithTimerByNextKind
; @run 71:C95C
; WHAT IT DOES: Starts the object from its record, sets its life_timer from a table at overlay
; address $C96F by its next_kind, and clears f3510 and f3568.
; HOW: LoadObjectRecord, one table read.
; WAITS: no.
; Entry: X object, D clear. Exit: A the timer, N, Z from it; C from the record; Y the next_kind.
StartWithTimerByNextKind:
    jsr VecLoadObjectRecord
    ldy a:OBJ_3040,x
    lda a:$C96F,y
    sta a:OBJ_34B8,x
    stz a:OBJ_3510,x
    stz a:OBJ_3568,x
    rts

; @routine 0D5E:18F WalkWhileFrameChangesC
; @run 71:C98F
; WHAT IT DOES: Animates the object facing the player and, on the frames where the picture
; changed, gives it an X speed from a table at overlay address $C9A2 by its f3510 and moves it.
; HOW: StepFramesFacingPlayerE with Y 0, then ApplyVelocityX.
; WAITS: no.
; Entry, exit: as WalkWhileFrameChanges.
WalkWhileFrameChangesC:
    cly
    jsr StepFramesFacingPlayerE
    bcc WalkWhileFrameChangesC_at0
    ldy a:OBJ_3510,x
    lda a:$C9A2,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
WalkWhileFrameChangesC_at0:
    rts

; @routine 0D5E:1A4 StepFramesFacingPlayerE
; @run 71:C9A4
; WHAT IT DOES: Turns the object to face the player and steps its animation along the list whose
; pointer is the pair of bytes at overlay address $C9B9 plus Y.
; HOW: SideOfPlayer, the pointer into zTemp10-11, then StepObjectFrames as a tail jump.
; WAITS: no.
; Entry: X object, Y the list's offset, D clear. Exit: as StepObjectFrames. Clobbers zTemp00,
; zTemp10-11.
StepFramesFacingPlayerE:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    lda a:$C9B9,y
    sta z:zTemp10
    lda a:$C9BA,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0D5E:4B0 StartWithFrameByNextKind
; @run 71:CCB0
; WHAT IT DOES: Starts the object from its record, clears f3568, and gives it a frame from a two
; byte table at overlay address $CCC7 by bit 4 of its next_kind.
; HOW: LoadObjectRecord, one table read.
; WAITS: no.
; Entry: X object, D clear. Exit: A the frame, N, Z from it; C bit 3 of next_kind; Y 0 or 1.
StartWithFrameByNextKind:
    jsr VecLoadObjectRecord
    stz a:OBJ_3568,x
    lda a:OBJ_3040,x
    lsr a
    lsr a
    lsr a
    lsr a
    and #$01
    tay
    lda a:$CCC7,y
    sta a:OBJ_2A68,x
    rts

; @routine 0D5E:4EC AnimateByKindThenNextState
; @run 71:CCEC
; WHAT IT DOES: Animates the object along one of two lists (pointers at overlay address $CD0F)
; picked by bit 4 of its next_kind; when the list has ended it sets f3250 to 2 and moves the
; object on to its next state.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object, D clear. Exit: as StepObjectFrames, or at the end A 2 and N, Z from the new
; state. Clobbers zTemp10-11.
AnimateByKindThenNextState:
    lda a:OBJ_3040,x
    lsr a
    lsr a
    lsr a
    lsr a
    and #$01
    asl a
    tay
    lda a:$CD0F,y
    sta z:zTemp10
    lda a:$CD10,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne AnimateByKindThenNextState_at0
    lda #$02
    sta a:OBJ_3250,x
    inc a:OBJ_27A8,x
AnimateByKindThenNextState_at0:
    rts
