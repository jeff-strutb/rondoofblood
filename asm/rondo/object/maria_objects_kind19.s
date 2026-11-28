; @routine 01E4:1B3 MariaKind11
; @run 6E:D9B3
; WHAT IT DOES: The handler of kind 11, a falling piece: in state 0 a hitbox $1C square, live, OBJ_WORK_N $0D,
; animation $15, sprite set 3, and its next state; after, it moves with its animation and is cleared when the
; animation has stopped.
; HOW: OBJ_STATE; MariaObjectSetAnimation, MariaObjectNextState; MariaObjectMove, MariaObjectAnimationRunning's Z,
; ClearObject by the tail JMP.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step made.
MariaKind11:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne MariaKind11_at0
    lda #$0E
    sta a:OBJ_HITBOX_HALF_H,x
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$0D
    sta a:OBJ_WORK_N,x
    lda #$15
    jsr MariaObjectSetAnimation
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    jmp MariaObjectNextState
MariaKind11_at0:
    jsr MariaObjectMove
    jsr MariaObjectAnimationRunning
    bne MariaKind11_at1
    jmp VecClearObject
MariaKind11_at1:
    rts

; @routine 01E4:225 MariaKind18
; @run 6E:DA25
; WHAT IT DOES: The handler of kind 18. State 0: cleared when she is hit; at her frame $50 its next state, sprite set
; 1, frame $2A, and her place plus (9, -$0F) with her flip and facing. After: at her frame $50 the same place; at $51
; and $52 her place plus (7, -$14); at any other frame a kind 3 slot is claimed and it is cleared.
; HOW: Her OBJ_STATE and OBJ_FRAME; MariaObjectNextState, MariaObjectFromPlayer, MariaObjectAtPlayer with zTemp04 and
; zTemp06; MariaClaimKind3Slot, ClearObject by the tail JMP.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step made.
MariaKind18:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne MariaKind18_at0
    lda a:OBJ_STATE
    cmp #$09
    bne MariaKind18_at1
    jmp VecClearObject
MariaKind18_at1:
    lda a:OBJ_FRAME
    cmp #$50
    beq MariaKind18_at2
    rts
MariaKind18_at2:
    jsr MariaObjectNextState
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$2A
    sta a:OBJ_FRAME,x
MariaKind18_at3:
    jsr MariaObjectFromPlayer
    lda #$09
    sta z:zTemp04
    lda #$F1
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    rts
MariaKind18_at0:
    lda a:OBJ_FRAME
    cmp #$50
    beq MariaKind18_at3
    cmp #$51
    beq MariaKind18_at4
    cmp #$52
    beq MariaKind18_at4
    jsr MariaClaimKind3Slot
    jmp VecClearObject
MariaKind18_at4:
    lda #$07
    sta z:zTemp04
    lda #$EC
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    jmp MariaObjectFromPlayer

; @routine 01E4:27D MariaKind19
; @run 6E:DA7D
; WHAT IT DOES: The handler of kind 19: its state's routine from MariaKind19States (start, aim, wait, fall, wait for
; slot 14, end).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
MariaKind19:
    jsr MariaStateDoubled
    jmp (MariaKind19States,x)

; @routine 01E4:28F MariaKind19Start
; @run 6E:DA8F
; WHAT IT DOES: Kind 19's state 0: from slot 5, slots 6 to 14 become kind $13 too; each takes a hitbox $10 high and
; $0A wide, live, OBJ_WORK_N $0B, her flip and facing, her place plus (2, -2), faces right, and starts its animation
; by slot (MariaKind19Animations).
; HOW: The stores; MariaObjectFromPlayer, MariaObjectAtPlayer; the tail JMP to MariaObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectStartAnimation.
MariaKind19Start:
    ldx z:zRichterObject
    cpx #$05
    bne MariaKind19Start_at0
    lda #$13
    cly
MariaKind19Start_at1:
    sta a:OBJ_KIND+6,y
    iny
    cpy #$09
    bne MariaKind19Start_at1
MariaKind19Start_at0:
    lda #$08
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$05
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$0B
    sta a:OBJ_WORK_N,x
    jsr MariaObjectFromPlayer
    lda #$02
    sta z:zTemp04
    lda #$FE
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    txa
    sec
    sbc #$05
    tay
    stz a:OBJ_FACING_LEFT,x
    lda a:MariaKind19Animations,y
    jmp MariaObjectStartAnimation

; @routine 01E4:2D0 MariaKind19Aim
; @run 6E:DAD0
; WHAT IT DOES: Kind 19's state 1: its animation, its aim, its move; once above its place's Y, slot 14 sounds $5F,
; slows time and starts a fade; each takes its wait (MariaKind19Waits), stops, and its next state.
; HOW: MariaObjectAnimationTick, MariaObjectAimAtPlace, MariaObjectApplyVelocity; its Y against MariaKind19TargetsY;
; SoundCommand, TimeSlowMode, zFadeCount, zFadeActive; MariaObjectStop, the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: short of its place, A its Y, N, Z and C from the compare; else as
; MariaObjectNextState.
MariaKind19Aim:
    ldx z:zRichterObject
    jsr MariaObjectAnimationTick
    jsr MariaObjectAimAtPlace
    jsr MariaObjectApplyVelocity
    txa
    sec
    sbc #$05
    tay
    lda a:OBJ_Y_LO,x
    cmp a:MariaKind19TargetsY,y
    bcc MariaKind19Aim_at0
    rts
MariaKind19Aim_at0:
    cpx #$0E
    bne MariaKind19Aim_at1
    lda #$5F
    jsr VecSoundCommand
    lda #$01
    sta a:TimeSlowMode
    sta z:zFadeCount
    sta z:zFadeActive
MariaKind19Aim_at1:
    txa
    sec
    sbc #$05
    tay
    lda a:MariaKind19Waits,y
    sta a:OBJ_ANIM_TIMER,x
    jsr MariaObjectStop
    jmp MariaObjectNextState

; @routine 01E4:30C MariaKind19Wait
; @run 6E:DB0C
; WHAT IT DOES: Kind 19's state 2: its wait counts down every other frame with its animation; when it runs out, the
; Y speed of entry $0F, a fall of $FFC8 a frame (OBJ_FALL_LO and _HI), and its next state.
; HOW: TST of zWaveCounter, DEC of OBJ_ANIM_TIMER; MariaObjectAnimationTick by the tail JMP; MariaSetVelocityYFromTable,
; MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectAnimationTick or MariaObjectNextState.
MariaKind19Wait:
    ldx z:zRichterObject
    tst #$01,z:zWaveCounter
    bne MariaKind19Wait_at0
    dec a:OBJ_ANIM_TIMER,x
    beq MariaKind19Wait_at1
MariaKind19Wait_at0:
    jmp MariaObjectAnimationTick
MariaKind19Wait_at1:
    lda #$0F
    jsr MariaSetVelocityYFromTable
    lda #$C8
    sta a:OBJ_FALL_LO,x
    lda #$FF
    sta a:OBJ_RAIN_COUNT,x
    jmp MariaObjectNextState

; @routine 01E4:32D MariaKind19Fall
; @run 6E:DB2D
; WHAT IT DOES: Kind 19's state 3: moves and falls; after $20 frames its animation goes back three and is at step 2,
; and its next state.
; HOW: MariaObjectApplyVelocity, MariaObjectFallCapped; INC of OBJ_ANIM_TIMER; DECs of OBJ_ANIMATION, the tail JMP to
; MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: A the count, N, Z and C from comparing it with $20; or as MariaObjectNextState.
MariaKind19Fall:
    ldx z:zRichterObject
    jsr MariaObjectApplyVelocity
    jsr MariaObjectFallCapped
    inc a:OBJ_ANIM_TIMER,x
    lda a:OBJ_ANIM_TIMER,x
    cmp #$20
    bne MariaKind19Fall_at0
    dec a:OBJ_WORK_A,x
    dec a:OBJ_WORK_A,x
    dec a:OBJ_WORK_A,x
    lda #$02
    sta a:OBJ_SCROLL_BAND,x
    jmp MariaObjectNextState
MariaKind19Fall_at0:
    rts

; @routine 01E4:351 MariaKind19WaitLast
; @run 6E:DB51
; WHAT IT DOES: Kind 19's state 4: waits until slot 14 is in state 4 too, then its next state.
; HOW: OBJ_STATE of slot 14; the tail JMP to MariaObjectNextState, or MariaKind19Fall's RTS.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: waiting, A slot 14's state, N, Z and C from comparing it with 4; else as
; MariaObjectNextState.
MariaKind19WaitLast:
    ldx z:zRichterObject
    lda a:OBJ_STATE+14
    cmp #$04
    bne MariaKind19Fall_at0
    jmp MariaObjectNextState

; @routine 01E4:35D MariaKind19End
; @run 6E:DB5D
; WHAT IT DOES: Kind 19's state 5: when its animation has stopped, time back to normal, the fade's count 2, and it is
; cleared.
; HOW: MariaObjectAnimationRunning's Z; the stores; ClearObject by the tail JMP.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectAnimationRunning, or ClearObject.
MariaKind19End:
    ldx z:zRichterObject
    jsr MariaObjectAnimationRunning
    beq MariaKind19End_at0
    rts
MariaKind19End_at0:
    stz a:TimeSlowMode
    lda #$02
    sta z:zFadeCount
    jmp VecClearObject

; @routine 01E4:36F MariaObjectAimAtPlace
; @run 6E:DB6F
; WHAT IT DOES: Turns a kind 19 object toward its place (MariaKind19TargetsX and Y by slot less 5) by up to 8 a frame
; (its angle in OBJ_FLY_ANGLE, kept a multiple of 8), and gives it speed $0100 along it.
; HOW: The place less its own X and Y into zTemp00-03; AngleFromDelta, TurnAngleToward with step 8, SetVelocityFromAngle
; by the tail JMP (the kernel's vectors at $404E, $406C and $4048).
; WAITS: no.
; Entry: X the object. Exit: as SetVelocityFromAngle.
MariaObjectAimAtPlace:
    txa
    sec
    sbc #$05
    tay
    lda a:MariaKind19TargetsX,y
    sec
    sbc a:OBJ_X_LO,x
    sta z:zTemp00
    cla
    sbc a:OBJ_X_HI,x
    sta z:zTemp01
    lda a:MariaKind19TargetsY,y
    sec
    sbc a:OBJ_Y_LO,x
    sta z:zTemp02
    cla
    sbc a:OBJ_Y_HI,x
    sta z:zTemp03
    jsr VecAngleFromDelta
    sta z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_FLY_ANGLE,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    and #$F8
    sta a:OBJ_FLY_ANGLE,x
    pha
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    pla
    jmp VecSetVelocityFromAngle
