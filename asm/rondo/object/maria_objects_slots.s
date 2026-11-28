; @routine 01E1:52F MariaObjectAtPlayerPlace
; @run 6E:C52F
; WHAT IT DOES: RichterObjectAtPlayerPlace at her address, the same bytes: puts an object where she is.
; HOW: RichterObjectAtPlayerPlace's body.
; WAITS: no.
; Entry: X the object. Exit: A her Y's high byte, N and Z from it. X, Y, C and V kept.
MariaObjectAtPlayerPlace:
    lda a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_X_HI
    sta z:zTemp01
    lda a:OBJ_Y_LO
    sta z:zTemp02
    lda a:OBJ_Y_HI
    sta z:zTemp03
    lda z:zTemp00
    sta a:OBJ_X_LO,x
    lda z:zTemp01
    sta a:OBJ_X_HI,x
    lda z:zTemp02
    sta a:OBJ_Y_LO,x
    lda z:zTemp03
    sta a:OBJ_Y_HI,x
    rts

; @routine 01E1:58D MariaObjectKind1
; @run 6E:C58D
; WHAT IT DOES: The handler of her companions of kinds 1 and 2. State 0: after OBJ_ANIM_TIMER runs out, animation 3,
; sprite set 3, state 1, a place beside her by slot and the frame tick (OBJ_WORK_E, MariaCompanionYOffsets and
; MariaCompanionXOffsets), and a terrain read $10 rows below her: on ground (bit 3) the timer is set to $10. State 1:
; while its timer runs, rises from its place one row a frame more (OBJ_WORK_D counts the rows) as its animation runs;
; it ends when its animation ends with her hit timer below 8, her invincibility on or the game in sub-mode 3; at the
; end of its own timer it starts script 8 and is cleared.
; HOW: The stage overlay's read at $8006; MariaObjectAnimationWait, which ends this handler while the animation runs.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaObjectKind1:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne MariaObjectKind1_at0
    lda a:OBJ_ANIM_TIMER,x
    beq MariaObjectKind1_at1
    dec a:OBJ_ANIM_TIMER,x
    bne MariaObjectKind1_at2
MariaObjectKind1_at1:
    lda #$03
    jsr MariaObjectSetAnimation
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    inc a:OBJ_STATE,x
    txa
    dec a
    sta z:zTemp00
    lda z:zFrameTick
    and #$02
    ora z:zTemp00
    sta a:OBJ_WORK_E,x
    tay
    lda a:OBJ_Y_LO
    clc
    adc a:MariaCompanionYOffsets,y
    sta a:OBJ_Y_LO,x
    cla
    adc a:OBJ_Y_HI
    sta a:OBJ_Y_HI,x
    lda a:OBJ_X_LO
    clc
    adc a:MariaCompanionXOffsets,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_LO
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_Y_LO
    clc
    adc #$10
    sta z:zTemp02
    adc #$00
    sta z:zTemp03
    bcs MariaObjectKind1_at2
    jsr StageTerrainRead
    and #$08
    beq MariaObjectKind1_at2
    lda #$10
    sta a:OBJ_ANIM_TIMER,x
MariaObjectKind1_at2:
    rts
MariaObjectKind1_at0:
    lda a:OBJ_ANIM_TIMER,x
    beq MariaObjectKind1_at3
    dec a:OBJ_ANIM_TIMER,x
    bne MariaObjectKind1_at3
    phx
    lda #$08
    jsr MariaStartHitScript
    plx
MariaObjectKind1_at4:
    jmp VecClearObject
MariaObjectKind1_at3:
    ldy a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    eor #$FF
    inc a
    clc
    adc a:MariaCompanionYOffsets,y
    clc
    adc a:OBJ_Y_LO
    sta a:OBJ_Y_LO,x
    inc a:OBJ_WORK_D,x
    jsr MariaObjectAnimationWait
    lda a:OBJ_ANIM_TIMER
    cmp #$08
    bcc MariaObjectKind1_at4
    lda a:PlayerInvincibleTimer
    bne MariaObjectKind1_at4
    lda z:zGameSubMode
    cmp #$03
    beq MariaObjectKind1_at4
    stz a:OBJ_WORK_D,x
    stz a:OBJ_STATE,x
    rts

; @routine 01E1:645 MariaObjectKind3
; @run 6E:C645
; WHAT IT DOES: The handler of her companion of kind 3: cleared when she is hit (her state 9). State 0: where she
; is, OBJ_SPRITE_FLIP from hers, sprite set 1, frame $2B, timer $80, state 1. State 1: stays where she is, marked,
; until its timer runs out, then is cleared.
; HOW: MariaObjectTake2FE8, MariaObjectAtPlayerPlace, MariaObjectMarkOn; the tail JMPs to MariaObjectNextState and
; MariaObjectClear.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaObjectKind3:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    beq MariaObjectClear
    lda a:OBJ_STATE,x
    bne MariaObjectKind3_at0
    jsr MariaObjectTake2FE8
    jsr MariaObjectAtPlayerPlace
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$2B
    sta a:OBJ_FRAME,x
    lda #$80
    sta a:OBJ_ANIM_TIMER,x
    jmp MariaObjectNextState
MariaObjectKind3_at0:
    jsr MariaObjectAtPlayerPlace
    jsr MariaObjectMarkOn
    dec a:OBJ_ANIM_TIMER,x
    beq MariaObjectClear
    rts

; @routine 01E1:67A MariaObjectKind4
; @run 6E:C67A
; WHAT IT DOES: The handler of her companions of kinds 4 and 5: cleared when she is hit (her state 9). State 0:
; from her (MariaObjectFromPlayer), marked, at its place beside her, sprite set 3, animation $13, state 1. State 1:
; moves, and stays at its place while its animation runs; cleared when it ends.
; HOW: MariaObjectPlaceByState (a BSR, then run on into at the end); MariaObjectMove, MariaObjectAnimationRunning.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaObjectKind4:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    beq MariaObjectClear
    lda a:OBJ_STATE,x
    bne MariaObjectKind4_at0
    jsr MariaObjectFromPlayer
    jsr MariaObjectMarkOn
    bsr MariaObjectPlaceByState
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$13
    jsr MariaObjectSetAnimation
    jmp MariaObjectNextState
MariaObjectKind4_at0:
    jsr MariaObjectMove
    jsr MariaObjectAnimationRunning
    beq MariaObjectClear

; @routine 01E1:6E3 MariaRunCompanions
; @run 6E:C6E3
; WHAT IT DOES: Runs her companions in slots 1 and 2.
; HOW: zRichterObject the slot, a BSR to MariaRunCompanionKind; X as the handler left it, one up, until 3.
; WAITS: no.
; Entry: nothing. Exit: X 3, Z and C set, N clear; A, Y and V as the last handler left them.
MariaRunCompanions:
    ldx #$01
MariaRunCompanions_at0:
    stx z:zRichterObject
    bsr MariaRunCompanionKind
    inx
    cpx #$03
    bne MariaRunCompanions_at0
    rts

; @routine 01E1:6EF MariaRunCompanionKind
; @run 6E:C6EF
; WHAT IT DOES: Runs a companion's handler by its kind (MariaCompanionHandlers: none, kinds 1 and 2, kind 3, kinds 4
; and 5).
; HOW: OBJ_KIND doubled; the JMP (table,X). A word past the four handlers goes to the routine at it.
; WAITS: no.
; Entry: X the slot. Exit: as the handler.
MariaRunCompanionKind:
    lda a:OBJ_KIND,x
    asl a
    tax
    jmp (MariaCompanionHandlers,x)

; @routine 01E4:584 MariaObjectAnimationWait
; @run 6E:DD84
; WHAT IT DOES: Steps an object's animation and, unless the tick left Z set (the animation stopped or a frame of 0),
; returns to its caller's caller: the caller's own work waits for the animation.
; HOW: MariaObjectAnimationTick; on Z clear the caller's return address is pulled (PLA PLA).
; WAITS: no.
; Entry: X the object. Exit, Z set: as the tick. Otherwise A the return address's high byte, N and Z from it; the
; rest as the tick left them. Returns true when it dropped the caller's return address: the caller then ends at once.
MariaObjectAnimationWait:
    jsr MariaObjectAnimationTick
    beq MariaObjectAnimationWait_at0
    pla
    pla
MariaObjectAnimationWait_at0:
    rts
