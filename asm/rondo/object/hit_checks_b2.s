; @routine 0EBC:5FF ToState6WhenHitOrAway0EBC
; @run 70:BDFF
; WHAT IT DOES: To state 6 once hit, or off the screen's height: Y high positive with Y low from $80, or negative with Y low
; under $80 (Stage 09's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; X kept.
ToState6WhenHitOrAway0EBC:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    bne ToState6WhenHitOrAway0EBC_at0
    lda a:OBJ_Y_HI,x
    beq ToState6WhenHitOrAway0EBC_at1
    bmi ToState6WhenHitOrAway0EBC_at2
    lda a:OBJ_Y_LO,x
    bpl ToState6WhenHitOrAway0EBC_at1
    bra ToState6WhenHitOrAway0EBC_at0
ToState6WhenHitOrAway0EBC_at2:
    lda a:OBJ_Y_LO,x
    bmi ToState6WhenHitOrAway0EBC_at1
ToState6WhenHitOrAway0EBC_at0:
    lda #$06
    sta a:OBJ_STATE,x
ToState6WhenHitOrAway0EBC_at1:
    rts

; @routine 0EBD:248 HitBecomesKind38_0EBD
; @run 71:C248
; WHAT IT DOES: Once hit (Stage 09's overlay): the slot before records its event and is cleared; this one becomes kind $38 in
; state 6 and goes on at the routine loaded at $C97E.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the routine at $C97E, or A 0 and Z when not hit; X kept.
HitBecomesKind38_0EBD:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitBecomesKind38_0EBD_at0
    dex
    jsr VecRecordObjectEvent
    jsr VecClearObject
    inx
    lda #$38
    sta a:OBJ_KIND,x
    lda #$06
    sta a:OBJ_STATE,x
    stz a:OBJ_FRAME,x
    jmp CheckThenJump0EBE17E
HitBecomesKind38_0EBD_at0:
    rts

; @routine 13FC:1EA KeepNearScreen13FC
; @run 70:B9EA
; WHAT IT DOES: Brings the object back to the screen next to the first one: an X or Y high byte of -2 becomes -1, of 2 becomes 1;
; one at a time (secret stage 0C's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the high byte or its new value, as the last compare; X kept.
KeepNearScreen13FC:
    lda a:OBJ_X_HI,x
    cmp #$FE
    bne KeepNearScreen13FC_at0
    lda #$FF
    sta a:OBJ_X_HI,x
    rts
KeepNearScreen13FC_at0:
    cmp #$02
    bne KeepNearScreen13FC_at1
    lda #$01
    sta a:OBJ_X_HI,x
    rts
KeepNearScreen13FC_at1:
    lda a:OBJ_Y_HI,x
    cmp #$FE
    bne KeepNearScreen13FC_at2
    lda #$FF
    sta a:OBJ_Y_HI,x
    rts
KeepNearScreen13FC_at2:
    cmp #$02
    bne KeepNearScreen13FC_at3
    lda #$01
    sta a:OBJ_Y_HI,x
KeepNearScreen13FC_at3:
    rts

; @routine 13FC:218 ToState4WhenSlot16Marked13FC
; @run 70:BA18
; WHAT IT DOES: Under state 4, once bit 7 of slot $10's OBJ_HIT_STATUS is set: state 4, substate 0 (secret stage 0C's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; X kept.
ToState4WhenSlot16Marked13FC:
    lda a:OBJ_STATE,x
    cmp #$04
    bcs ToState4WhenSlot16Marked13FC_at0
    lda a:OBJ_HIT_STATUS+16
    and #$80
    beq ToState4WhenSlot16Marked13FC_at0
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ToState4WhenSlot16Marked13FC_at0:
    rts

; @routine 13FC:22F HitBoxByFrameRange13FC
; @run 70:BA2F
; WHAT IT DOES: A long hit box below the object while its frame is $44 to $47, else a small one (secret stage 0C's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the last value stored, N and Z from it; C set only for frames from $48; X kept.
HitBoxByFrameRange13FC:
    lda a:OBJ_FRAME,x
    cmp #$44
    bcc HitBoxByFrameRange13FC_at0
    cmp #$48
    bcs HitBoxByFrameRange13FC_at0
    lda #$50
    sta a:OBJ_HITBOX_DY,x
    lda #$0C
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$40
    sta a:OBJ_HITBOX_HALF_H,x
    rts
HitBoxByFrameRange13FC_at0:
    stz a:OBJ_HITBOX_DY,x
    lda #$08
    sta a:OBJ_HITBOX_HALF_W,x
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 161D:02A StartByPlayerSide161D
; @run 71:C02A
; WHAT IT DOES: When the player is on the object's side (X low from $80 with OBJ_START_ARG 0, under $70 else) and the table at $C058
; allows its state: OBJ_WORK_M counted, the animation restarted, state 3 (Stage 0E's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; X kept.
StartByPlayerSide161DAllowed = $C058 ; by state: 0 leaves the object be
StartByPlayerSide161D:
    lda a:OBJ_START_ARG,x
    bne StartByPlayerSide161D_at0
    lda a:OBJ_X_LO
    cmp #$80
    bcc StartByPlayerSide161D_at1
StartByPlayerSide161D_at2:
    ldy a:OBJ_STATE,x
    lda a:StartByPlayerSide161DAllowed,y
    beq StartByPlayerSide161D_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_KNOCK_FORWARD,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StartByPlayerSide161D_at1:
    rts
StartByPlayerSide161D_at0:
    lda a:OBJ_X_LO
    cmp #$70
    bcc StartByPlayerSide161D_at2
    rts

; @routine 161D:060 HitStopToState5_161D
; @run 71:C060
; WHAT IT DOES: Under state 5, once hit (Stage 0E's overlay): no vertical speed, no hit box, state 5.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; X kept.
HitStopToState5_161D:
    lda a:OBJ_STATE,x
    cmp #$05
    bcs HitStopToState5_161D_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitStopToState5_161D_at0
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_HITBOX_ON,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitStopToState5_161D_at0:
    rts

; @routine 161D:5CE FaceThePlayerEvery8_161D
; @run 71:C5CE
; WHAT IT DOES: Every eighth count of OBJ_WORK_F, in the states the table at $C5E8 marks, turns the object to the player
; (Stage 0E's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; X kept.
FaceThePlayerEvery8_161DTurns = $C5E8 ; by state: 0 does not turn
FaceThePlayerEvery8_161D:
    lda a:OBJ_CHILD_ORDER,x
    and #$07
    bne FaceThePlayerEvery8_161D_at0
    ldy a:OBJ_STATE,x
    lda a:FaceThePlayerEvery8_161DTurns,y
    beq FaceThePlayerEvery8_161D_at0
    jsr VecSideOfPlayer
    and #$01
    eor #$01
    sta a:OBJ_FACING_LEFT,x
FaceThePlayerEvery8_161D_at0:
    rts

; @routine 161D:5F8 HalveWhenHit161D
; @run 71:C5F8
; WHAT IT DOES: Outside state 4, a hit of 5 or more halves OBJ_WORK_H (Stage 0E's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last compare or the shift; X kept.
HalveWhenHit161D:
    lda a:OBJ_STATE,x
    cmp #$04
    beq HalveWhenHit161D_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc HalveWhenHit161D_at0
    lsr a:OBJ_FALL_LO,x
HalveWhenHit161D_at0:
    rts

; @routine 161D:60A HitStopAndClearNext161D
; @run 71:C60A
; WHAT IT DOES: Once hit, under state 5 (Stage 0E's overlay): the next slot cleared, the hit box off, the animation restarted,
; state 5.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load; Y the next slot when taken; X kept.
HitStopAndClearNext161D:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitStopAndClearNext161D_at0
    lda a:OBJ_STATE,x
    cmp #$05
    bcs HitStopAndClearNext161D_at0
    txa
    tay
    iny
    jsr VecClearObjectInY
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitStopAndClearNext161D_at0:
    rts

; @routine 18DB:2C4 CountStateWhenHit18DB
; @run 70:B2C4
; WHAT IT DOES: Once hit, the object's state is counted on with substate 0 (Stage 10's overlay); C says whether.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: C set when taken; N and Z from the new state, else A 0 and Z; X kept.
CountStateWhenHit18DB:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq CountStateWhenHit18DB_at0
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    sec
    rts
CountStateWhenHit18DB_at0:
    clc
    rts
