; @routine 01E1:76D MariaAttackObject
; @run 6E:C76D
; WHAT IT DOES: The handler of kind 1, her attack's object: its state's step (MariaAttackStates: start, fly, fall,
; return).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the state's step.
MariaAttackObject:
    jsr MariaStateDoubled
    jmp (MariaAttackStates,x)

; @routine 01E1:77B MariaAttackStart
; @run 6E:C77B
; WHAT IT DOES: Starts her attack's object: from her (facing and sprite flip), $0A across and $0D up from her ($0A
; across and 3 down when she crouches), an 8 by 8 hitbox $04 to the left, live; its X speed from entry 0 the way it
; faces, Y speed entry 1, falling by $FFF0 a frame, OBJ_WORK_N 5; then animation 0 and its next state.
; HOW: MariaObjectFromPlayer, MariaObjectAtPlayer, the velocity tables, MariaObjectFacingVelocity; on into
; MariaObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectStartAnimation.
MariaAttackStart:
    ldx z:zRichterObject
    jsr MariaObjectFromPlayer
    lda #$0A
    sta z:zTemp04
    lda #$F3
    sta z:zTemp06
    jsr MariaObjectAtPlayer
    lda a:OBJ_STATE
    cmp #$02
    bne MariaAttackStart_at0
    lda #$0A
    sta z:zTemp04
    lda #$03
    sta z:zTemp06
    jsr MariaObjectAtPlayer
MariaAttackStart_at0:
    lda #$08
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$08
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$FC
    sta a:OBJ_HITBOX_DX,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$00
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$01
    jsr MariaSetVelocityYFromTable
    lda #$F0
    sta a:OBJ_FALL_LO,x
    lda #$FF
    sta a:OBJ_FALL_HI,x
    lda #$05
    sta a:OBJ_WORK_N,x
    lda #$00

; @routine 01E1:7DA MariaAttackFly
; @run 6E:C7DA
; WHAT IT DOES: Her attack's object flying out: its animation runs, it moves and falls, and it slows; when its X
; speed changes sign it turns: Y speed entry 2, falling by $0006 a frame, animation 1 and its next state.
; HOW: MariaObjectAnimationTick, MariaObjectMove, MariaObjectFallCapped, MariaObjectAccelerate (Z clear on the sign's
; change); the tail JMP to MariaObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectAccelerate, or MariaObjectStartAnimation.
MariaAttackFly:
    ldx z:zRichterObject
    jsr MariaObjectAnimationTick
    jsr MariaObjectMove
    jsr MariaObjectFallCapped
    jsr MariaObjectAccelerate
    beq MariaAttackFly_at0
    lda #$02
    jsr MariaSetVelocityYFromTable
    lda #$06
    sta a:OBJ_FALL_LO,x
    lda #$00
    sta a:OBJ_FALL_HI,x
    lda #$01
    jmp MariaObjectStartAnimation
MariaAttackFly_at0:
    rts

; @routine 01E1:7FF MariaAttackFall
; @run 6E:C7FF
; WHAT IT DOES: Her attack's object after its turn: cleared when it reaches her; otherwise it moves and falls, and
; when its animation ends it takes the angle back toward her by its facing (MariaAttackAngles, into OBJ_WORK_D),
; animation 2 and its next state.
; HOW: MariaObjectNearPlayer (C set when it is at her), MariaObjectApplyVelocity, MariaObjectFallCapped,
; MariaObjectAnimationRunning; the tail JMPs to ClearObject and MariaObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step left the processor.
MariaAttackFall:
    ldx z:zRichterObject
    jsr MariaObjectNearPlayer
    bcs MariaAttackFall_at0
    jsr MariaObjectApplyVelocity
    jsr MariaObjectFallCapped
    jsr MariaObjectAnimationRunning
    bne MariaAttackFall_at1
    ldy a:OBJ_FACING_LEFT,x
    lda a:MariaAttackAngles,y
    sta a:OBJ_WORK_D,x
    lda #$02
    jmp MariaObjectStartAnimation
MariaAttackFall_at1:
    rts
MariaAttackFall_at0:
    jmp VecClearObject

; @routine 01E1:825 MariaAttackReturn
; @run 6E:C825
; WHAT IT DOES: Her attack's object homing back: cleared when it reaches her; otherwise it moves, its animation
; runs, and it is pointed at speed $0300 at her hitbox's middle (her Y moved by her OBJ_HITBOX_DY, and $0D up unless
; she crouches, for the angle, then put back).
; HOW: MariaObjectNearPlayer, MariaObjectApplyVelocity, MariaObjectAnimationTick; her Y on the stack around
; AngleToPlayer and SetVelocityFromAngle (the kernel's vectors at $404B and $4048).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: A her Y's low byte, N and Z from it; C and V as SetVelocityFromAngle left
; them, Y 0.
MariaAttackReturn:
    ldx z:zRichterObject
    jsr MariaObjectNearPlayer
    bcs MariaAttackFall_at0
    jsr MariaObjectApplyVelocity
    jsr MariaObjectAnimationTick
    stz z:zTemp00
    lda a:OBJ_HITBOX_DY
    bpl MariaAttackReturn_at0
    dec z:zTemp00
MariaAttackReturn_at0:
    lda a:OBJ_Y_LO
    pha
    clc
    adc a:OBJ_HITBOX_DY
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    pha
    adc z:zTemp00
    sta a:OBJ_Y_HI
    lda a:OBJ_STATE
    cmp #$02
    beq MariaAttackReturn_at1
    lda a:OBJ_Y_LO
    clc
    adc #$F3
    sta a:OBJ_Y_LO
    lda #$FF
    adc a:OBJ_Y_HI
    sta a:OBJ_Y_HI
MariaAttackReturn_at1:
    jsr VecAngleToPlayer
    pha
    lda #$00
    sta z:zTemp00
    lda #$03
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    pla
    sta a:OBJ_Y_HI
    pla
    sta a:OBJ_Y_LO
    rts

; @routine 01E1:8DF MariaItem5
; @run 6E:C8DF
; WHAT IT DOES: The handler of kind 5, item 5: its state's step (MariaItem5States: start, fly, fall, return).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the state's step.
MariaItem5:
    jsr MariaStateDoubled
    jmp (MariaItem5States,x)

; @routine 01E1:8ED MariaItem5Pair
; @run 6E:C8ED
; WHAT IT DOES: The first of item 5's pair (slot 5 or 8): puts the second in the next slot, then starts this one
; facing the other way from her.
; HOW: Entered by MariaItem5Start's branch, with her facing on the stack: OBJ_KIND 5 one slot up, her facing
; flipped, then the start's tail.
; WAITS: no.
; Entry: X the slot, her facing pushed. Exit: as MariaItem5Start.
MariaItem5Pair:
    inx
    lda #$05
    sta a:OBJ_KIND,x
    dex
    bra MariaItem5Start_at0

; @routine 01E1:8F6 MariaItem5Start
; @run 6E:C8F6
; WHAT IT DOES: Starts item 5 as her attack's object starts, with X speed entry 4, Y speed entry 5, falling by $0044
; a frame and sprite attribute $85. From slot 5 or 8 it is the first of a pair (MariaItem5Pair).
; HOW: Her facing on the stack; MariaAttackStart; the velocity tables and MariaObjectFacingVelocity.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: A her facing, N and Z from it; the rest as the velocity table left them.
MariaItem5Start:
    lda a:OBJ_FACING_LEFT
    pha
    ldx z:zRichterObject
    cpx #$05
    beq MariaItem5Pair
    cpx #$08
    beq MariaItem5Pair
    bra MariaItem5Start_at1
MariaItem5Start_at0:
    lda a:OBJ_FACING_LEFT
    eor #$01
    and #$01
    sta a:OBJ_FACING_LEFT
MariaItem5Start_at1:
    jsr MariaAttackStart
    lda #$04
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$05
    jsr MariaSetVelocityYFromTable
    lda #$44
    sta a:OBJ_FALL_LO,x
    lda #$00
    sta a:OBJ_FALL_HI,x
    lda #$85
    sta a:OBJ_SPRITE_ATTR,x
    pla
    sta a:OBJ_FACING_LEFT
    rts

; @routine 01E2:134 MariaItem5Fly
; @run 6E:C934
; WHAT IT DOES: Item 5 flying out: as her attack's object.
; HOW: The tail JMP to MariaAttackFly.
; WAITS: no.
; Entry, exit: as MariaAttackFly.
MariaItem5Fly:
    jmp MariaAttackFly

; @routine 01E2:137 MariaObjectState2Work6
; @run 6E:C937
; WHAT IT DOES: In state 2, sets an object's OBJ_WORK_A to 6. Nothing is known to reach it: it follows
; MariaItem5Fly's jump and no call, jump or table found goes to it.
; HOW: A compare of OBJ_STATE with 2.
; WAITS: no.
; Entry: X the object. Exit: not state 2, A the state, N, Z and C from comparing it with 2. Else A 6, N and Z from
; it, C set. X, Y and V kept.
MariaObjectState2Work6:
    lda a:OBJ_STATE,x
    cmp #$02
    beq MariaObjectState2Work6_at0
    rts
MariaObjectState2Work6_at0:
    lda #$06
    sta a:OBJ_WORK_A,x
    rts

; @routine 01E2:145 MariaItem5Fall
; @run 6E:C945
; WHAT IT DOES: Item 5 after its turn: as her attack's object.
; HOW: The tail JMP to MariaAttackFall.
; WAITS: no.
; Entry, exit: as MariaAttackFall.
MariaItem5Fall:
    jmp MariaAttackFall

; @routine 01E2:148 MariaItem5Return
; @run 6E:C948
; WHAT IT DOES: Item 5 homing back: as her attack's object.
; HOW: The tail JMP to MariaAttackReturn.
; WAITS: no.
; Entry, exit: as MariaAttackReturn.
MariaItem5Return:
    jmp MariaAttackReturn
