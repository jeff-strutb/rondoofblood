; @routine 0EBA:7A9 FallToLineInA0EBA
; @run 70:AFA9
; WHAT IT DOES: Falls toward the line given in A, gravity taken off as it goes; at the line it stops there (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallToLineInA0EBA:
    sta z:zTemp00
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp z:zTemp00
    beq FallToLineInA0EBA_at0
    bcc FallToLineInA0EBA_at0
    lda a:OBJ_VY_LO,x
    sec
    sbc #$40
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    rts
FallToLineInA0EBA_at0:
    lda z:zTemp00
    sta a:OBJ_Y_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    rts

; @routine 0EBC:382 ClearOffScreenElseState0EBC
; @run 70:BB82
; WHAT IT DOES: An object below the first screen, or off its right edge, is cleared; else its state's handler runs
; (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ClearOffScreenElseState0EBCStates = $BBB3 ; the handlers by state
ClearOffScreenElseState0EBC:
    lda a:OBJ_Y_HI,x
    cmp #$01
    beq ClearOffScreenElseState0EBC_at0
    lda a:OBJ_Y_LO,x
    cmp #$E0
    bcs ClearOffScreenElseState0EBC_at0
    lda a:OBJ_X_HI,x
    beq ClearOffScreenElseState0EBC_at1
    bmi ClearOffScreenElseState0EBC_at1
    lda a:OBJ_X_LO,x
    cmp #$18
    bcc ClearOffScreenElseState0EBC_at1
ClearOffScreenElseState0EBC_at0:
    jmp VecClearObject
ClearOffScreenElseState0EBC_at1:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ClearOffScreenElseState0EBCStates,y
    sta z:zTemp00
    lda a:ClearOffScreenElseState0EBCStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBF:699 SwayDownThenState0EBF
; @run 71:D699
; WHAT IT DOES: Drifts down swaying by the cosine of OBJ_WORK_J; above Y $E0 on the first screen the state counts on (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwayDownThenState0EBF:
    lda #$A0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_J,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:zProduct1
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    inc a:OBJ_WORK_J,x
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne SwayDownThenState0EBF_at0
    lda a:OBJ_Y_LO,x
    cmp #$E0
    bcs SwayDownThenState0EBF_at0
    inc a:OBJ_STATE,x
SwayDownThenState0EBF_at0:
    rts

; @routine 117F:20B OrbitPrevious117F
; @run 71:D20B
; WHAT IT DOES: Placed on the previous slot (or the one before, 4 down), then out at angle OBJ_WORK_E (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
OrbitPrevious117F:
    txa
    tay
    dey
    lda a:OBJ_WORK_M,x
    beq OrbitPrevious117F_at0
    jsr VecCopyObjectPlaceBack
    bra OrbitPrevious117F_at1
OrbitPrevious117F_at0:
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_Y_LO,x
    clc
    adc #$04
    sta a:OBJ_Y_LO,x
    bcc OrbitPrevious117F_at1
    inc a:OBJ_Y_HI,x
OrbitPrevious117F_at1:
    lda a:OBJ_WORK_K,x
    sta z:zTemp00
    lda a:OBJ_WORK_J,x
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 12DC:3A8 LoadWithEightStreamers12DC
; @run 70:BBA8
; WHAT IT DOES: Loads, fills the 240 bytes at $BA41 with $C0, and starts eight numbered kind $2D objects in the next slots (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadWithEightStreamers12DCTrail = $BA41 ; 240 bytes of the overlay's RAM, filled at the start
LoadWithEightStreamers12DC:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_WORK_K,x
    lda #$C0
    sta a:OBJ_WORK_D,x
    cly
LoadWithEightStreamers12DC_at0:
    sta a:LoadWithEightStreamers12DCTrail,y
    iny
    cpy #$F0
    bcc LoadWithEightStreamers12DC_at0
    txa
    tay
    iny
    lda #$08
    sta z:zTemp00
    stz z:zTemp01
LoadWithEightStreamers12DC_at1:
    jsr VecClearObjectInY
    lda #$2D
    jsr VecStartObjectAtPlace
    lda z:zTemp01
    sta a:OBJ_WORK_D,y
    iny
    inc z:zTemp01
    dec z:zTemp00
    bne LoadWithEightStreamers12DC_at1
    rts

; @routine 12DC:759 TurnThenSubstate12DC
; @run 70:BF59
; WHAT IT DOES: Turns OBJ_WORK_D toward the facing's angle in eighths, clears OBJ_WORK_E-F and goes to the substate's handler (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnThenSubstate12DCAngles = $BCD9 ; the angle aimed for by facing
TurnThenSubstate12DCHandlers = $BF8A ; the handlers by substate
TurnThenSubstate12DC:
    ldy a:OBJ_FACING_LEFT,x
    lda a:TurnThenSubstate12DCAngles,y
    sta z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    and #$F8
    sta a:OBJ_WORK_D,x
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_F,x
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:TurnThenSubstate12DCHandlers,y
    sta z:zTemp00
    lda a:TurnThenSubstate12DCHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DC:7EF DissolveStepWorkO12DC
; @run 70:BFEF
; WHAT IT DOES: Every other count of OBJ_WORK_H one step of a dissolve over 13 patterns, its phase OBJ_WORK_O (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DissolveStepWorkO12DCPatterns = $C01D ; 13 VRAM addresses
DissolveStepWorkO12DC:
    lda a:OBJ_WORK_H,x
    and #$01
    bne DissolveStepWorkO12DC_at0
    lda a:OBJ_WORK_O,x
    clc
    adc #$13
    and #$3F
    sta a:OBJ_WORK_O,x
    phx
    jsr DissolveMask12DA33A
    clx
DissolveStepWorkO12DC_at1:
    txa
    asl a
    tay
    lda a:DissolveStepWorkO12DCPatterns,y
    sta z:zBiosCl
    lda a:DissolveStepWorkO12DCPatterns+1,y
    sta z:zBiosCh
    jsr DissolveWords12DA365
    inx
    cpx #$0D
    bcc DissolveStepWorkO12DC_at1
    plx
DissolveStepWorkO12DC_at0:
    rts

; @routine 13FB:420 ListThenWakeWithMusic13FB
; @run 70:B420
; WHAT IT DOES: Runs $B48C and $B4A7 and the list at $B455; then the sound stopped, CD track 3 repeating, the object hit-tested, state on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenWakeWithMusic13FB:
    jsr SpawnAtStep18_13FB
    jsr FireTwiceAtStep13_13FB
    lda #$55
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenWakeWithMusic13FB_at0
    smb4 z:zField8E
    lda #$FE
    jsr VecSoundCommand
    rmb3 z:zField8E
    rmb4 z:zField8E
    lda #$03
    jsr VecCdPlayRepeat
    lda z:zField8E
    ora #$04
    sta z:zField8E
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListThenWakeWithMusic13FB_at0:
    rts

; @routine 16CB:195 ShudderWhenHit16CB
; @run 70:B195
; WHAT IT DOES: A hit of 5 or more shakes the object for 16 counts of OBJ_WORK_M (sound $8D); in state 2 it also goes to state 4 (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ShudderWhenHit16CB:
    lda a:OBJ_WORK_M,x
    beq ShudderWhenHit16CB_at0
    jsr SpitDebrisEvery8_16CB
    dec a:OBJ_WORK_M,x
    rts
ShudderWhenHit16CB_at0:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc ShudderWhenHit16CB_at1
    lda #$10
    sta a:OBJ_WORK_M,x
    lda #$8D
    jsr VecSoundCommand
    lda a:OBJ_STATE,x
    cmp #$02
    bne ShudderWhenHit16CB_at1
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ShudderWhenHit16CB_at1:
    rts
