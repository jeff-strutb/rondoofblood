; @routine 01BD:00C RichterObjectAddXLow
; @run 6E:C00C
; WHAT IT DOES: Adds to an object's X position's low byte.
; HOW: A plain add, the carry left for RichterObjectAddXHigh.
; WAITS: no.
; Entry: A the amount, X object. Exit: A the low byte, N, Z, C and V from the add. X and Y kept.
RichterObjectAddXLow:
    clc
    adc a:OBJ_X_LO,x
    sta a:OBJ_X_LO,x
    rts

; @routine 01BD:014 RichterObjectAddXHigh
; @run 6E:C014
; WHAT IT DOES: Adds to an object's X position's high byte, with the carry from the low byte.
; HOW: An add with the carry in.
; WAITS: no.
; Entry: A the amount, C the carry, X object. Exit: A the high byte, N, Z, C and V from the add.
; X and Y kept.
RichterObjectAddXHigh:
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x
    rts

; @routine 01BD:1CF RichterObjectFall
; @run 6E:C1CF
; WHAT IT DOES: Pulls an object down: its Y speed grows by 6.
; HOW: Runs on into RichterObjectSetFall.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectSetFall.
RichterObjectFall:
    lda a:OBJ_VY_LO,x
    clc
    adc #$06

; @routine 01BD:1D5 RichterObjectSetFall
; @run 6E:C1D5
; WHAT IT DOES: Sets an object's Y speed's low byte, carrying into its high byte.
; HOW: The carry added to OBJ_VY_HI.
; WAITS: no.
; Entry: A the low byte, C the carry, X object. Exit: A the high byte, N, Z, C and V from adding
; the carry. X and Y kept.
RichterObjectSetFall:
    sta a:OBJ_VY_LO,x
    cla
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
    rts

; @routine 01BD:1E0 RichterObjectFallSlow
; @run 6E:C1E0
; WHAT IT DOES: Pulls an object down more gently: its Y speed grows by 3.
; HOW: RichterObjectSetFall.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectSetFall.
RichterObjectFallSlow:
    lda a:OBJ_VY_LO,x
    clc
    adc #$03
    bra RichterObjectSetFall

; @routine 01BD:238 RichterObjectMarkOn
; @run 6E:C238
; WHAT IT DOES: Sets bit 4 of an object's OBJ_SPRITE_ATTR.
; HOW: ORed in.
; WAITS: no.
; Entry: X object. Exit: A the new OBJ_SPRITE_ATTR, N and Z from it. X, Y, C and V kept.
RichterObjectMarkOn:
    lda #$10
    ora a:OBJ_SPRITE_ATTR,x
    sta a:OBJ_SPRITE_ATTR,x
    rts

; @routine 01BD:3EC RichterObjectMarkOff
; @run 6E:C3EC
; WHAT IT DOES: Clears bit 4 of an object's OBJ_SPRITE_ATTR.
; HOW: ANDed out.
; WAITS: no.
; Entry: X object. Exit: A the new OBJ_SPRITE_ATTR, N and Z from it. X, Y, C and V kept.
RichterObjectMarkOff:
    lda #$EF
    and a:OBJ_SPRITE_ATTR,x
    sta a:OBJ_SPRITE_ATTR,x
    rts

; @routine 01BD:3F8 RichterObjectFromPlayer
; @run 6E:C3F8
; WHAT IT DOES: Gives an object Richter's facing and his OBJ_SPRITE_FLIP, doubled.
; HOW: RichterObjectTake2FE8, then the facing copied.
; WAITS: no.
; Entry: X object. Exit: A the facing, N and Z from it, C as RichterObjectTake2FE8. X, Y and V
; kept.
RichterObjectFromPlayer:
    bsr RichterObjectTake2FE8
    lda a:OBJ_FACING_LEFT
    sta a:OBJ_FACING_LEFT,x
    rts

; @routine 01BD:401 RichterObjectTake2FE8
; @run 6E:C401
; WHAT IT DOES: Gives an object Richter's OBJ_SPRITE_FLIP, doubled.
; HOW: Shifted up a place.
; WAITS: no.
; Entry: X object. Exit: A the value, N and Z from it, C Richter's bit 7. X, Y and V kept.
RichterObjectTake2FE8:
    lda a:OBJ_SPRITE_FLIP
    asl a
    sta a:OBJ_SPRITE_FLIP,x
    rts

; @routine 01BD:409 RichterObjectSlot
; @run 6E:C409
; WHAT IT DOES: Gives the slot of the object being run.
; HOW: zRichterObject.
; WAITS: no.
; Entry: nothing. Exit: X the slot, N and Z from it. A, Y, C and V kept.
RichterObjectSlot:
    ldx z:zRichterObject
    rts

; @routine 01BD:47C RichterObjectQuicken
; @run 6E:C47C
; WHAT IT DOES: Once, at its animation's step 6 and while PlayerWhipStruck is set, makes an object's
; frame last 4 longer and Richter's one frame shorter than that.
; HOW: OBJ_WORK_G marks it done; OBJ_2F38 the step; OBJ_HURT_TIMER the frame's time.
; WAITS: no.
; Entry: X object. Exit: X and Y kept. Stopped, A what was read last, N and Z from it, or from
; comparing the step with 6 and C with it. Done, A Richter's new time, N and Z from the mark, C
; and V from the add.
RichterObjectQuicken:
    lda a:OBJ_WORK_G,x
    bne RichterObjectQuicken_at0
    lda a:OBJ_ANIMATION_AT,x
    cmp #$06
    bne RichterObjectQuicken_at0
    lda a:PlayerWhipStruck
    beq RichterObjectQuicken_at0
    lda #$04
    clc
    adc a:OBJ_HURT_TIMER,x
    sta a:OBJ_HURT_TIMER,x
    dec a
    sta a:OBJ_HURT_TIMER
    inc a:OBJ_WORK_G,x
    rts
RichterObjectQuicken_at0:
    rts

; @routine 01BD:49F RichterObjectFlash
; @run 6E:C49F
; WHAT IT DOES: While an object is charged, shows the flashing form of its frame, if it is one
; of the four that have one, with its OBJ_SPRITE_ATTR for the flash.
; HOW: OBJ_WORK_F is the charge; frames $07, $0E, $15 and $1C become $8E to $91; RichterFlashFlags
; by OBJ_WORK_D less $0E, with bit 7.
; WAITS: no.
; Entry: X object. Exit: X kept. Not charged, A 0, Z set, Y $8E. Not one of the four, A the
; frame, N, Z and C from comparing it with $1C, Y $91. Flashed, A the flags, N and Z from them, Y
; OBJ_WORK_D less $0E, C and V from that subtract. V kept otherwise.
RichterObjectFlash:
    ldy #$8E
    lda a:OBJ_WORK_F,x
    beq RichterObjectFlash_at0
    lda a:OBJ_FRAME,x
    cmp #$07
    beq RichterObjectFlash_at1
    iny
    cmp #$0E
    beq RichterObjectFlash_at1
    iny
    cmp #$15
    beq RichterObjectFlash_at1
    iny
    cmp #$1C
    beq RichterObjectFlash_at1
RichterObjectFlash_at0:
    rts
RichterObjectFlash_at1:
    tya
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_D,x
    sec
    sbc #$0E
    tay
    lda a:RichterFlashFlags,y
    ora #$80
    sta a:OBJ_SPRITE_ATTR,x
    rts

; @routine 01BD:4D1 RichterObjectCharge
; @run 6E:C4D1
; WHAT IT DOES: Charges an object from OBJ_WORK_D $0C on, the first time the way it faces is newly
; pressed.
; HOW: OBJ_WORK_F counted up; RichterChargePads by OBJ_FACING_LEFT against zPadPressed.
; WAITS: no.
; Entry: X object. Exit: X kept. Before $0C, A OBJ_WORK_D, N, Z and C from comparing it with $0C.
; Charged already, A the charge, N and Z from it, C set. Not pressed, A 0, Z set, C set, Y the
; facing. Charged now, A the press, N and Z from the charge, C set, Y the facing. V kept.
RichterObjectCharge:
    lda a:OBJ_WORK_D,x
    cmp #$0C
    bcc RichterObjectCharge_at0
    lda a:OBJ_WORK_F,x
    bne RichterObjectCharge_at0
    ldy a:OBJ_FACING_LEFT,x
    lda z:zPadPressed
    and a:RichterChargePads,y
    beq RichterObjectCharge_at0
    inc a:OBJ_WORK_F,x
RichterObjectCharge_at0:
    rts

; @routine 01BD:5C5 RichterObjectNextState
; @run 6E:C5C5
; WHAT IT DOES: Moves an object on to its next state.
; HOW: OBJ_STATE counted up.
; WAITS: no.
; Entry: X object. Exit: N and Z from the new state. A, X, Y, C and V kept.
RichterObjectNextState:
    inc a:OBJ_STATE,x
    rts

; @routine 01BE:67B RichterObjectAtPlayer
; @run 6E:CE7B
; WHAT IT DOES: Puts an object at Richter's place plus an offset, signed, the X offset turned
; round when the object faces left.
; HOW: zTemp04 and zTemp06 the offsets, made 16-bit in zTemp05 and zTemp07.
; WAITS: no.
; Entry: X object, zTemp04 and zTemp06. Exit: A the Y position's high byte, N, Z, C and V from its
; add. X and Y kept.
RichterObjectAtPlayer:
    stz z:zTemp05
    stz z:zTemp07
    lda z:zTemp04
    bpl RichterObjectAtPlayer_at0
    dec z:zTemp05
RichterObjectAtPlayer_at0:
    lda z:zTemp06
    bpl RichterObjectAtPlayer_at1
    dec z:zTemp07
RichterObjectAtPlayer_at1:
    lda a:OBJ_FACING_LEFT,x
    beq RichterObjectAtPlayer_at2
    cla
    sec
    sbc z:zTemp04
    sta z:zTemp04
    cla
    sbc z:zTemp05
    sta z:zTemp05
RichterObjectAtPlayer_at2:
    lda a:OBJ_X_LO
    clc
    adc z:zTemp04
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI
    adc z:zTemp05
    sta a:OBJ_X_HI,x
    lda a:OBJ_Y_LO
    clc
    adc z:zTemp06
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    adc z:zTemp07
    sta a:OBJ_Y_HI,x
    rts

; @routine 01BE:6BE RichterObjectAtPlayerPlace
; @run 6E:CEBE
; WHAT IT DOES: Puts an object exactly where Richter is.
; HOW: His position into zTemp00 to 03; runs on into RichterObjectToPoint.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectToPoint.
RichterObjectAtPlayerPlace:
    lda a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_X_HI
    sta z:zTemp01
    lda a:OBJ_Y_LO
    sta z:zTemp02
    lda a:OBJ_Y_HI
    sta z:zTemp03

; @routine 01BE:6D2 RichterObjectToPoint
; @run 6E:CED2
; WHAT IT DOES: Puts an object at the point in zTemp00 to 03.
; HOW: X low and high, Y low and high.
; WAITS: no.
; Entry: X object. Exit: A the Y position's high byte, N and Z from it. X, Y, C and V kept.
RichterObjectToPoint:
    lda z:zTemp00
    sta a:OBJ_X_LO,x
    lda z:zTemp01
    sta a:OBJ_X_HI,x
    lda z:zTemp02
    sta a:OBJ_Y_LO,x
    lda z:zTemp03
    sta a:OBJ_Y_HI,x
    rts

; @routine 01C0:240 RichterObjectAnimationWait
; @run 6E:DA40
; WHAT IT DOES: Steps an object's animation and, unless it has ended or shows frame 0, returns to
; its caller's caller, so the caller's own work waits for the end.
; HOW: RichterObjectAnimationTick; Z from it; the assembly drops the return address with two pulls.
; WAITS: no.
; Entry: X object. Exit, Z set: as the tick. Otherwise the caller's return address is gone, A its
; high byte, N and Z from it; its caller then ends at once.
RichterObjectAnimationWait:
    jsr RichterObjectAnimationTick
    beq RichterObjectAnimationWait_at0
    pla
    pla
RichterObjectAnimationWait_at0:
    rts

; @routine 01C0:248 RichterObjectAnimationRunning
; @run 6E:DA48
; WHAT IT DOES: Steps an object's animation; its caller looks at whether it has ended.
; HOW: RichterObjectAnimationTick, then the timer against $FF.
; WAITS: no.
; Entry: X object. Exit: A the timer, N, Z and C from comparing it with $FF. Y as the tick. X and
; V kept. Clobbers zTemp00-01.
RichterObjectAnimationRunning:
    jsr RichterObjectAnimationTick
    lda a:OBJ_HURT_TIMER,x
    cmp #$FF
    rts

; @routine 01C0:251 RichterObjectAnimationTick
; @run 6E:DA51
; WHAT IT DOES: Counts down an object's frame on show; at zero moves to the next step of its list.
; A timer of $FF means its animation has stopped.
; HOW: OBJ_HURT_TIMER the timer, OBJ_ANIMATION_AT the step, OBJ_FRAME the frame shown.
; WAITS: no.
; Entry: X object. Exit: as RichterAnimationTick.
RichterObjectAnimationTick:
    lda a:OBJ_HURT_TIMER,x
    cmp #$FF
    bne RichterObjectAnimationTick_at0
    rts
RichterObjectAnimationTick_at0:
    dec a:OBJ_HURT_TIMER,x
    bne RichterObjectSetAnimation_at0
    inc a:OBJ_ANIMATION_AT,x
    bra RichterObjectSetAnimation_at1

; @routine 01C0:263 RichterObjectSetAnimation
; @run 6E:DA63
; @stop DA84
; WHAT IT DOES: Starts an object's animation from its first step.
; HOW: The animation into OBJ_ANIMATION, step zero, and its first entry read.
; WAITS: no.
; Entry: A animation, X object. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
RichterObjectSetAnimation:
    sta a:OBJ_ANIMATION,x
RichterObjectSetAnimation_at3:
    stz a:OBJ_ANIMATION_AT,x
RichterObjectSetAnimation_at1:
    bsr RichterObjectAnimationList
    lda (zTemp00),y
    sta a:OBJ_HURT_TIMER,x
    cmp #$FF
    bne RichterObjectSetAnimation_at2
    rts
RichterObjectSetAnimation_at2:
    cmp #$FE
    beq RichterObjectSetAnimation_at3
RichterObjectSetAnimation_at4:
    iny
    lda (zTemp00),y
    sta a:OBJ_FRAME,x
    rts
RichterObjectSetAnimation_at0:
    bsr RichterObjectAnimationList
    bra RichterObjectSetAnimation_at4

; @routine 01C0:284 RichterObjectAnimationList
; @run 6E:DA84
; WHAT IT DOES: Points at an object's animation list and its step's entry.
; HOW: RichterObjectAnimations by OBJ_ANIMATION into zTemp00 and 01; the step doubled.
; WAITS: no.
; Entry: X object. Exit: A and Y twice the step, N and Z from it, C the step's bit 7. X and V
; kept.
RichterObjectAnimationList:
    lda a:OBJ_ANIMATION,x
    asl a
    tay
    lda a:RichterObjectAnimations,y
    sta z:zTemp00
    lda a:RichterObjectAnimations+1,y
    sta z:zTemp01
    lda a:OBJ_ANIMATION_AT,x
    asl a
    tay
    rts

; @routine 01C0:51B RichterObjectHitboxOff
; @run 6E:DD1B
; WHAT IT DOES: Takes an object's hitbox away.
; HOW: OBJ_HITBOX_ON cleared.
; WAITS: no.
; Entry: X object. Exit: all registers and flags kept.
RichterObjectHitboxOff:
    stz a:OBJ_HITBOX_ON,x
    rts

; @routine 01C0:51F RichterObjectHitboxOn
; @run 6E:DD1F
; WHAT IT DOES: Marks an object as having a hitbox and gives it its frame's.
; HOW: OBJ_HITBOX_ON 1; runs on into RichterObjectHitboxByFrame.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectHitboxByFrame.
RichterObjectHitboxOn:
    lda #$01
    sta a:OBJ_HITBOX_ON,x

; @routine 01C0:524 RichterObjectHitboxByFrame
; @run 6E:DD24
; WHAT IT DOES: Gives an object the hitbox of the frame it shows, or none.
; HOW: RichterFrameHitboxes by OBJ_FRAME; runs on into RichterObjectHitboxFrom.
; WAITS: no.
; Entry: X object. Exit: none, A 0, Z set, N clear, Y the frame. Otherwise as
; RichterObjectHitboxFrom.
RichterObjectHitboxByFrame:
    ldy a:OBJ_FRAME,x
    lda a:RichterFrameHitboxes,y

; @routine 01C0:52A RichterObjectHitboxFrom
; @run 6E:DD2A
; WHAT IT DOES: Gives an object the hitbox picked, or none.
; HOW: Z from the caller says none; otherwise the hitbox's four bytes from RichterObjectHitboxes,
; four bytes a hitbox.
; WAITS: no.
; Entry: A the hitbox, Z set for none, X object. Exit: none, as RichterObjectHitboxOff. Otherwise
; A the last byte, N and Z from it, Y four times the hitbox, C its bit 6. X and V kept.
RichterObjectHitboxFrom:
    beq RichterObjectHitboxOff
    asl a
    asl a
    tay
    lda a:RichterObjectHitboxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:RichterObjectHitboxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:RichterObjectHitboxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:RichterObjectHitboxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 01DE:48B MariaClaimSlotsB
; @run 6D:AC8B
; WHAT IT DOES: Takes every free one of object slots $0B to $0E for an object of kind $18.
; HOW: OBJ_KIND zero is free: ClearObject (through its kernel vector), then the kind.
; Untraced: a slot already in use ($AC90 taken); no script had one taken when she jumped.
; WAITS: no.
; Entry: nothing. Exit: X $0F; A the last kind read or $18; N clear, Z and C set from the CPX. Y and V kept.
MariaClaimSlotsB:
    ldx #$0B
MariaClaimSlotsB_at1:
    lda a:OBJ_KIND,x
    bne MariaClaimSlotsB_at0
    jsr VecClearObject
    lda #$18
    sta a:OBJ_KIND,x
MariaClaimSlotsB_at0:
    inx
    cpx #$0F
    bne MariaClaimSlotsB_at1
    rts

; @routine 01DE:7BA MariaReplaceSlotsOneTwo
; @run 6D:AFBA
; WHAT IT DOES: Clears object slots 1 and 2 as kind 4, each unless it holds kind 1 or 2.
; HOW: OBJ_KIND compared with 1 and 2; MariaClearSlotAsKind for slot 1, then a run on into it for slot 2.
; Untraced: a slot of kind 1 or 2 ($AFC3, $AFC7, $AFD1 and $AFD5 taken); no script had one there.
; WAITS: no.
; Entry: nothing. Exit: Y 4, X 2; slot 2 kept, A its kind, N, Z and C from comparing it with 1 or 2; else A 4, N
; and Z from it, C from comparing the kind with 2. V kept.
MariaReplaceSlotsOneTwo:
    ldy #$04
    ldx #$01
    lda a:OBJ_KIND,x
    cmp #$01
    beq MariaReplaceSlotsOneTwo_at0
    cmp #$02
    beq MariaReplaceSlotsOneTwo_at0
    bsr MariaClearSlotAsKind
MariaReplaceSlotsOneTwo_at0:
    inx
    lda a:OBJ_KIND,x
    cmp #$01
    beq StepScrollThenMoveOrFadeOut
    cmp #$02
    beq StepScrollThenMoveOrFadeOut

; @routine 01DE:7D7 MariaClearSlotAsKind
; @run 6D:AFD7
; WHAT IT DOES: Clears an object slot and gives it a kind.
; HOW: ClearObject (through its kernel vector), then Y into OBJ_KIND.
; WAITS: no.
; Entry: X object, Y the kind. Exit: A the kind, N and Z from it. X, Y, C and V kept.
MariaClearSlotAsKind:
    jsr VecClearObject
    tya
    sta a:OBJ_KIND,x
    rts

; @routine 01E1:27C MariaObjectAddXLow
; @run 6E:C27C
; WHAT IT DOES: RichterObjectAddXLow in Maria's player overlay (01DD), the same instructions at her address.
; HOW: RichterObjectAddXLow's body.
; WAITS: no.
; Entry, exit: as RichterObjectAddXLow.
MariaObjectAddXLow:
    clc
    adc a:OBJ_X_LO,x
    sta a:OBJ_X_LO,x
    rts

; @routine 01E1:284 MariaObjectAddXHigh
; @run 6E:C284
; WHAT IT DOES: RichterObjectAddXHigh in Maria's player overlay (01DD), the same instructions at her address.
; HOW: RichterObjectAddXHigh's body.
; WAITS: no.
; Entry, exit: as RichterObjectAddXHigh.
MariaObjectAddXHigh:
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x
    rts

; @routine 01E1:2CB MariaObjectFallCapped
; @run 6E:C2CB
; WHAT IT DOES: Adds an object's own pull (OBJ_WORK_H and OBJ_WORK_I) to its Y speed, held at $0700.
; HOW: A word add into OBJ_VY_LO and OBJ_VY_HI; at 7 and up the speed is set to $0700.
; Untraced: the hold at $0700 ($C2E4); no script let one of her objects fall that fast.
; WAITS: no.
; Entry: X object. Exit: A the speed's high byte, V from its add; rising, N, Z and C from the add; falling slower
; than 7, N, Z and C from comparing it with 7; held, A 7, N and Z from it, C set. X and Y kept.
MariaObjectFallCapped:
    lda a:OBJ_WORK_H,x
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,x
    lda a:OBJ_WORK_I,x
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
    bmi MariaObjectFallCapped_at0
    cmp #$07
    bcc MariaObjectFallCapped_at0
    lda #$07
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
MariaObjectFallCapped_at0:
    rts

; @routine 01E1:328 MariaObjectAgainstScroll
; @run 6E:C328
; WHAT IT DOES: Moves an object by the negative of the frame's scroll on both axes, so it stays put on the map
; while the screen moves.
; HOW: ScrollSpeedX and ScrollSpeedY negated, signed into zTemp10; X through MariaObjectAddXLow and
; MariaObjectAddXHigh, Y by a word add of its own.
; Untraced: the sign byte of a negative scroll, $C332 and $C346 (the scroll's negative was never below zero).
; WAITS: no.
; Entry: X object. Exit: A the Y position's high byte, N, Z, C and V from its add. X and Y kept. Clobbers zTemp10.
MariaObjectAgainstScroll:
    stz z:zTemp10
    lda a:ScrollSpeedX
    eor #$FF
    inc a
    bpl StartFirstObjectWhenCamera2
    dec z:zTemp10
    jsr MariaObjectAddXLow
    lda z:zTemp10
    jsr MariaObjectAddXHigh
    stz z:zTemp10
    lda a:ScrollSpeedY
    eor #$FF
    inc a
    bpl MariaObjectAgainstScroll_at0
    dec z:zTemp10
MariaObjectAgainstScroll_at0:
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    lda z:zTemp10
    adc a:OBJ_Y_HI,x
    sta a:OBJ_Y_HI,x
    rts

; @routine 01E1:358 MariaObjectMove
; @run 6E:C358
; WHAT IT DOES: Moves an object by its velocity, and against the scroll unless the camera holds still (mode $17).
; HOW: CameraMode, MariaObjectAgainstScroll, then on into MariaObjectApplyVelocity.
; Untraced: the camera's mode $17, which skips the scroll ($C35D taken).
; WAITS: no.
; Entry: X object. Exit: as MariaObjectApplyVelocity. Clobbers zTemp10.
MariaObjectMove:
    lda a:CameraMode
    cmp #$17
    beq MariaObjectApplyVelocity
    bsr MariaObjectAgainstScroll

; @routine 01E1:361 MariaObjectApplyVelocity
; @run 6E:C361
; WHAT IT DOES: Moves an object by its velocity, Y then X.
; HOW: ApplyVelocityY, then a tail jump into ApplyVelocityX (through their kernel vectors).
; WAITS: no.
; Entry: X object. Exit: as ApplyVelocityX. Clobbers zTemp10.
MariaObjectApplyVelocity:
    jsr VecApplyVelocityY
    jmp VecApplyVelocityX

; @routine 01E1:369 MariaObjectFacingVelocity
; @run 6E:C369
; WHAT IT DOES: Turns an object's X velocity round when it faces left.
; HOW: OBJ_FACING_LEFT, then NegateVelocityX (through its kernel vector).
; WAITS: no.
; Entry: X object. Exit: facing right, A 0, Z set; else as NegateVelocityX. X and Y kept.
MariaObjectFacingVelocity:
    lda a:OBJ_FACING_LEFT,x
    beq MariaObjectFacingVelocity_at0
    jsr VecNegateVelocityX
MariaObjectFacingVelocity_at0:
    rts

; @routine 01E1:37C MariaObjectMarkOn
; @run 6E:C37C
; WHAT IT DOES: RichterObjectMarkOn in Maria's player overlay (01DD), the same instructions at her address.
; HOW: RichterObjectMarkOn's body.
; WAITS: no.
; Entry, exit: as RichterObjectMarkOn.
MariaObjectMarkOn:
    lda #$10
    ora a:OBJ_SPRITE_ATTR,x
    sta a:OBJ_SPRITE_ATTR,x
    rts

; @routine 01E1:387 MariaObjectFromPlayer
; @run 6E:C387
; WHAT IT DOES: Gives an object Maria's OBJ_SPRITE_FLIP, doubled, and her facing; the doubled byte is written twice.
; HOW: The doubling once inline, then on into the copy of RichterObjectFromPlayer at $C38E, which calls
; MariaObjectTake2FE8 and copies the facing.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectFromPlayer.
MariaObjectFromPlayer:
    lda a:OBJ_SPRITE_FLIP
    asl a
    sta a:OBJ_SPRITE_FLIP,x
    bsr MariaObjectTake2FE8
    lda a:OBJ_FACING_LEFT
    sta a:OBJ_FACING_LEFT,x
    rts

; @routine 01E1:397 MariaObjectTake2FE8
; @run 6E:C397
; WHAT IT DOES: RichterObjectTake2FE8 in Maria's player overlay (01DD), the same bytes.
; HOW: RichterObjectTake2FE8's body.
; WAITS: no.
; Entry, exit: as RichterObjectTake2FE8.
MariaObjectTake2FE8:
    lda a:OBJ_SPRITE_FLIP
    asl a
    sta a:OBJ_SPRITE_FLIP,x
    rts

; @routine 01E1:4EC MariaObjectAtPlayer
; @run 6E:C4EC
; WHAT IT DOES: RichterObjectAtPlayer in Maria's player overlay (01DD), the same bytes.
; HOW: RichterObjectAtPlayer's body.
; WAITS: no.
; Entry, exit: as RichterObjectAtPlayer.
MariaObjectAtPlayer:
    stz z:zTemp05
    stz z:zTemp07
    lda z:zTemp04
    bpl MariaObjectAtPlayer_at0
    dec z:zTemp05
MariaObjectAtPlayer_at0:
    lda z:zTemp06
    bpl MariaObjectAtPlayer_at1
    dec z:zTemp07
MariaObjectAtPlayer_at1:
    lda a:OBJ_FACING_LEFT,x
    beq MariaObjectAtPlayer_at2
    cla
    sec
    sbc z:zTemp04
    sta z:zTemp04
    cla
    sbc z:zTemp05
    sta z:zTemp05
MariaObjectAtPlayer_at2:
    lda a:OBJ_X_LO
    clc
    adc z:zTemp04
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI
    adc z:zTemp05
    sta a:OBJ_X_HI,x
    lda a:OBJ_Y_LO
    clc
    adc z:zTemp06
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    adc z:zTemp07
    sta a:OBJ_Y_HI,x
    rts

; @routine 01E1:589 MariaObjectNextState
; @run 6E:C589
; WHAT IT DOES: RichterObjectNextState in Maria's player overlay (01DD), the same instructions at her address.
; HOW: RichterObjectNextState's body.
; WAITS: no.
; Entry, exit: as RichterObjectNextState.
MariaObjectNextState:
    inc a:OBJ_STATE,x
    rts

; @routine 01E1:677 MariaObjectClear
; @run 6E:C677
; WHAT IT DOES: Clears an object: the end her object handlers branch to.
; HOW: A tail jump into ClearObject (through its kernel vector).
; WAITS: no.
; Entry: X object. Exit: every register and flag kept.
MariaObjectClear:
    jmp VecClearObject

; @routine 01E1:703 MariaObjectSlot
; @run 6E:C703
; WHAT IT DOES: RichterObjectSlot in Maria's player overlay (01DD), the same instructions at her address.
; HOW: RichterObjectSlot's body.
; WAITS: no.
; Entry, exit: as RichterObjectSlot.
MariaObjectSlot:
    ldx z:zRichterObject
    rts

; @routine 01E2:080 MariaObjectAccelerate
; @run 6E:C880
; WHAT IT DOES: Adds to an object's X speed the step for the way it faces, and says whether the speed's sign
; changed.
; HOW: The word from her table at $C8A2 by OBJ_FACING_LEFT, added into OBJ_VX_LO and OBJ_VX_HI; the old high byte kept in
; zTemp00 and its bit 7 compared.
; WAITS: no.
; Entry: X object. Exit: A $80 if the sign changed, else 0, N and Z from it; C and V from the high byte's add; Y
; twice the facing. X kept.
MariaObjectAccelerate:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:MariaObjectAccelerate+$22,y
    clc
    adc a:OBJ_VX_LO,x
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    sta z:zTemp00
    lda a:MariaObjectAccelerate+$23,y
    adc a:OBJ_VX_HI,x
    sta a:OBJ_VX_HI,x
    eor z:zTemp00
    and #$80
    rts

; @routine 01E2:0A6 MariaObjectNearPlayer
; @run 6E:C8A6
; WHAT IT DOES: Says whether an object is at Maria: within 4 across and $12 down of her point (her Y plus her
; OBJ_HITBOX_DY), on the first screen; on the second screen down it counts as near at once.
; HOW: OBJ_Y_HI against 1, OBJ_X_HI, the X difference plus 4 against 8, the Y difference plus $12 against $24.
; Untraced: the second screen ($C8AB taken) and an object below the first screen ($C8CC taken).
; WAITS: no.
; Entry: X object. Exit: C set if near; A the last value tested, N and Z from its test; V from the last add. X and
; Y kept. Clobbers zTemp00.
MariaObjectNearPlayer:
    lda a:OBJ_Y_HI,x
    cmp #$01
    beq MariaObjectNearPlayer_at0
    lda a:OBJ_Y_LO
    clc
    adc a:OBJ_HITBOX_DY
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    bne MariaObjectNearPlayer_at1
    lda a:OBJ_X_LO,x
    sec
    sbc a:OBJ_X_LO
    clc
    adc #$04
    cmp #$08
    bcs MariaObjectNearPlayer_at1
    lda a:OBJ_Y_HI,x
    bne MariaObjectNearPlayer_at1
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp00
    clc
    adc #$12
    cmp #$24
    bcs MariaObjectNearPlayer_at1
MariaObjectNearPlayer_at0:
    sec
    rts
MariaObjectNearPlayer_at1:
    clc
    rts

; @routine 01E4:58C MariaObjectAnimationRunning
; @run 6E:DD8C
; WHAT IT DOES: RichterObjectAnimationRunning in Maria's player overlay (01DD).
; HOW: The same instructions with her tick, MariaObjectAnimationTick at $DD95.
; WAITS: no.
; Entry, exit: as RichterObjectAnimationRunning.
MariaObjectAnimationRunning:
    jsr MariaObjectAnimationTick
    lda a:OBJ_HURT_TIMER,x
    cmp #$FF
    rts

; @routine 01E4:595 MariaObjectAnimationTick
; @run 6E:DD95
; WHAT IT DOES: RichterObjectAnimationTick in Maria's player overlay (01DD), the same bytes.
; HOW: RichterObjectAnimationTick's body with her layout: its calls go to MariaObjectAnimationList
; at $DDC8, where his C would read his list.
; WAITS: no.
; Entry, exit: as RichterObjectAnimationTick.
MariaObjectAnimationTick:
    lda a:OBJ_HURT_TIMER,x
    cmp #$FF
    bne MariaObjectAnimationTick_at0
    rts
MariaObjectAnimationTick_at0:
    dec a:OBJ_HURT_TIMER,x
    bne MariaObjectSetAnimation_at0
    inc a:OBJ_SCROLL_BAND,x
    bra MariaObjectSetAnimation_at1

; @routine 01E4:5A7 MariaObjectSetAnimation
; @run 6E:DDA7
; @stop DDC8
; WHAT IT DOES: RichterObjectSetAnimation in Maria's player overlay (01DD), the same bytes.
; HOW: RichterObjectSetAnimation's body with her layout: its calls go to MariaObjectAnimationList.
; WAITS: no.
; Entry, exit: as RichterObjectSetAnimation.
MariaObjectSetAnimation:
    sta a:OBJ_WORK_A,x
MariaObjectSetAnimation_at3:
    stz a:OBJ_SCROLL_BAND,x
MariaObjectSetAnimation_at1:
    bsr MariaObjectAnimationList
    lda (zTemp00),y
    sta a:OBJ_HURT_TIMER,x
    cmp #$FF
    bne MariaObjectSetAnimation_at2
    rts
MariaObjectSetAnimation_at2:
    cmp #$FE
    beq MariaObjectSetAnimation_at3
MariaObjectSetAnimation_at4:
    iny
    lda (zTemp00),y
    sta a:OBJ_FRAME,x
    rts
MariaObjectSetAnimation_at0:
    bsr MariaObjectAnimationList
    bra MariaObjectSetAnimation_at4

; @routine 01E4:5C8 MariaObjectAnimationList
; @run 6E:DDC8
; WHAT IT DOES: RichterObjectAnimationList in Maria's player overlay (01DD).
; HOW: The same instructions with her table of object animations at $DDDD, $344 above his.
; WAITS: no.
; Entry, exit: as RichterObjectAnimationList.
MariaObjectAnimationList:
    lda a:OBJ_WORK_A,x
    asl a
    tay
    lda a:RichterObjectAnimations+$344,y
    sta z:zTemp00
    lda a:RichterObjectAnimations+$344+1,y
    sta z:zTemp01
    lda a:OBJ_SCROLL_BAND,x
    asl a
    tay
    rts

; @routine 01BC:7FC RichterObjectStopX
; @run 6D:BFFC
; WHAT IT DOES: Stops an object along X.
; HOW: Zeroes OBJ_VX_LO and OBJ_VX_HI.
; WAITS: no.
; Entry: X object. Exit: all registers and flags kept.
RichterObjectStopX:
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VX_HI,x
    rts

; @routine 01BD:003 RichterObjectStop
; @run 6E:C003
; WHAT IT DOES: Stops an object along X and Y.
; HOW: RichterObjectStopX, then runs on into RichterObjectStopY.
; WAITS: no.
; Entry: X object. Exit: all registers and flags kept.
RichterObjectStop:
    bsr RichterObjectStopX

; @routine 01BD:005 RichterObjectStopY
; @run 6E:C005
; WHAT IT DOES: Stops an object along Y.
; HOW: Zeroes OBJ_VY_LO and OBJ_VY_HI.
; WAITS: no.
; Entry: X object. Exit: all registers and flags kept.
RichterObjectStopY:
    stz a:OBJ_VY_LO,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 01BD:076 RichterDeathObjects
; @run 6E:C076
; WHAT IT DOES: The frame of his death's objects: the player's object, the scroll's steps stopped, then
; each of slots 1 to 15 as a piece.
; HOW: RichterDeathFrame for slot 0; ScrollStep cleared; RichterDeathPieceFrame for X 1 to $0F.
; WAITS: no.
; Entry: nothing. Exit: X $10, N clear, Z and C set; A and Y as the last piece left them.
RichterDeathObjects:
    clx
    jsr RichterDeathFrame
    stz a:ScrollStepXHi
    stz a:ScrollStepXLo
    stz a:ScrollStepYLo
    stz a:ScrollStepYHi
    inx
RichterDeathObjects_at0:
    jsr RichterDeathPieceFrame
    inx
    cpx #$10
    bne RichterDeathObjects_at0
    rts

; @routine 01BD:090 RichterDeathFrame
; @run 6E:C090
; WHAT IT DOES: The player's object in his death (state 0 on the floor, then state 1): moved with the
; scroll, held between $10 and $F0 across, falling; when the animation ends, slots 1 to 15 become pieces
; (kind 1, a timer of four times the slot), the death's row of points starts ($60, OBJ_WORK_K 0), the
; death's sound, state 1. In state 1, moved and falling on even frames; the row of points from $6080 once
; OBJ_WORK_J is down to $40.
; HOW: RichterObjectScrollAndMove, RichterObjectFall, RichterAnimationRunning, RichterDeathSound;
; RichterDeathPointRow.
; WAITS: no.
; Entry: X 0. Exit: as its last step leaves the registers.
RichterDeathFrame:
    lda a:OBJ_KIND,x
    bne RichterDeathFrame_at0
    rts
RichterDeathFrame_at0:
    lda a:OBJ_STATE
    bne RichterDeathFrame_at1
    jsr RichterObjectScrollAndMove
    lda a:OBJ_X_LO
    cmp #$10
    bcs RichterDeathFrame_at2
    lda #$10
    sta a:OBJ_X_LO
    bra RichterDeathFrame_at3
RichterDeathFrame_at2:
    cmp #$F0
    bcc RichterDeathFrame_at3
    lda #$F0
    sta a:OBJ_X_LO
RichterDeathFrame_at3:
    jsr RichterObjectFall
    jsr RichterAnimationRunning
    beq RichterDeathFrame_at4
    rts
RichterDeathFrame_at4:
    ldy #$01
RichterDeathFrame_at5:
    lda #$01
    sta a:OBJ_KIND,y
    tya
    asl a
    asl a
    sta a:OBJ_ANIM_TIMER,y
    iny
    cpy #$10
    bne RichterDeathFrame_at5
    lda #$60
    sta a:OBJ_WORK_J
    stz a:OBJ_WORK_K
    jsr RichterDeathSound
    inc a:OBJ_STATE
    rts
RichterDeathFrame_at1:
    lda z:zFrameTick
    and #$01
    bne RichterDeathFrame_at6
    jsr RichterObjectScrollAndMove
    jsr RichterObjectFall
RichterDeathFrame_at6:
    lda a:OBJ_WORK_J
    cmp #$41
    bcc RichterDeathFrame_at7
    dec a:OBJ_WORK_J
    rts
RichterDeathFrame_at7:
    lda #$60
    sta z:zTemp09
    lda #$80
    sta z:zTemp08
    ldx #$06

; @routine 01BD:100 RichterDeathPointRow
; @run 6E:C100
; WHAT IT DOES: Gives the stage overlay's entry at $8009 X points $40 apart from zTemp08-09, each with his
; OBJ_WORK_K in A as zTemp0F; what the stage's code leaves in zTemp0F is his OBJ_WORK_K after; his OBJ_WORK_J
; counts down, and at 0 his frame and kind go.
; HOW: RichterDeathPointStep X times.
; WAITS: no.
; Entry: X the count, zTemp08-09 the first point. Exit: counting, N and Z from OBJ_WORK_J, A zTemp0F; ended,
; A zTemp0F, Z set. X 0; Y as the stage's code left it; C and V from the last point's add.
RichterDeathPointRow:
    lda a:OBJ_WORK_K
    sta z:zTemp0F
    jsr RichterDeathPointStep
    bne RichterDeathPointRow
    lda z:zTemp0F
    sta a:OBJ_WORK_K
    dec a:OBJ_WORK_J
    beq RichterDeathPointRow_at0
    rts
RichterDeathPointRow_at0:
    stz a:OBJ_FRAME
    stz a:OBJ_KIND
    rts

; @routine 01BD:11C RichterDeathPointStep
; @run 6E:C11C
; WHAT IT DOES: Gives the stage overlay's entry at $8009 the point in zTemp08-09, then moves the point $40
; on and counts X down.
; HOW: The JSR to $8009 (the C of the routine resident there), a 16-bit add of $0040, DEX.
; WAITS: no.
; Entry: X the count, the rest as the stage's code takes them. Exit: A the point's high byte, C and V from
; its add, X one less, N and Z from it; Y as the stage's code left it.
RichterDeathPointStep:
    jsr StageVecClearBitRows
    lda z:zTemp08
    clc
    adc #$40
    sta z:zTemp08
    cla
    adc z:zTemp09
    sta z:zTemp09
    dex
    rts

; @routine 01BD:12D RichterDeathPieceFrame
; @run 6E:C12D
; WHAT IT DOES: One of the death's pieces, slots 1 to 15: its state's handler, if the slot holds an object.
; HOW: zRichterObject keeps the slot; the four words at $C13D by OBJ_STATE.
; WAITS: no.
; Entry: X the slot. Exit: empty, A 0, Z set; else as the handler.
RichterDeathPieceFrame:
    lda a:OBJ_KIND,x
    bne RichterDeathPieceFrame_at0
    rts
RichterDeathPieceFrame_at0:
    stx z:zRichterObject
RichterDeathPieceFrame_at1:
    lda a:OBJ_STATE,x
    asl a
    tax
    jmp (RichterDeathPieceStates,x)

; @routine 01BD:145 RichterDeathPieceStart
; @run 6E:C145
; WHAT IT DOES: A death piece's state 0: when its timer runs out, put at the player by the slot's offsets,
; flung up and across by the slot's speeds, marked, animation $14.
; HOW: The slot's bytes at $C218 and $C228 as RichterObjectAtPlayer's offsets; Y speed $FF and the byte at
; $C1E8; X speed the word at $C1F8; RichterObjectMarkOn; a jump to RichterObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: counting, N and Z from the timer; else as
; RichterObjectStartAnimation.
RichterDeathPieceStart:
    ldx z:zRichterObject
    dec a:OBJ_ANIM_TIMER,x
    beq RichterDeathPieceStart_at0
    rts
RichterDeathPieceStart_at0:
    lda a:RichterDeathPieceOffsetsX,x
    sta z:zTemp04
    lda a:RichterDeathPieceOffsetsY,x
    sta z:zTemp06
    jsr RichterObjectAtPlayer
    lda a:RichterDeathPieceSpeedsY,x
    sta a:OBJ_VY_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    txa
    asl a
    tay
    lda a:RichterDeathPieceSpeedsX,y
    sta a:OBJ_VX_LO,x
    lda a:RichterDeathPieceSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    jsr RichterObjectMarkOn
    lda #$14
    jmp RichterObjectStartAnimation

; @routine 01BD:17C RichterDeathPieceMove
; @run 6E:C17C
; WHAT IT DOES: A death piece moves with the scroll and falls slowly; then X against $0F.
; HOW: RichterObjectScrollAndMove, RichterObjectFallSlow, CPX.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: X the slot, N, Z and C from comparing it with $0F; A and V as the
; fall left them.
RichterDeathPieceMove:
    ldx z:zRichterObject
    jsr RichterObjectScrollAndMove
    jsr RichterObjectFallSlow
    cpx #$0F
RichterDeathPieceMove_at0:
    rts

; @routine 01BD:187 RichterDeathPieceFly
; @run 6E:C187
; WHAT IT DOES: A death piece's state 1: moves until its animation ends, then the next state.
; HOW: RichterDeathPieceMove, RichterObjectAnimationWait, a jump to RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: waiting, as RichterObjectAnimationWait; else as
; RichterObjectNextState.
RichterDeathPieceFly:
    jsr RichterDeathPieceMove
    jsr RichterObjectAnimationWait
    jmp RichterObjectNextState

; @routine 01BD:190 RichterDeathPieceFall
; @run 6E:C190
; WHAT IT DOES: A death piece's state 2: moves; the last piece (slot $0F), once the player's OBJ_WORK_J is
; 0, starts the end's row ($40, OBJ_WORK_K 0) and goes on.
; HOW: RichterDeathPieceMove; its RTS where it is not the last ($C186); RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterDeathPieceMove, or N and Z from OBJ_WORK_J, or as
; RichterObjectNextState.
RichterDeathPieceFall:
    jsr RichterDeathPieceMove
    bne RichterDeathPieceMove_at0
    lda a:OBJ_WORK_J
    bne RichterDeathPieceMove_at0
    lda #$40
    sta a:OBJ_WORK_J
    stz a:OBJ_WORK_K
    jmp RichterObjectNextState

; @routine 01BD:1A5 RichterDeathPieceEnd
; @run 6E:C1A5
; WHAT IT DOES: A death piece's state 3: moves; the last piece gives the end's row of six points from $6C00,
; and when the player's OBJ_WORK_J runs out clears slots 1 to 15 and sets zField89 to 1.
; HOW: RichterDeathPieceMove; RichterDeathPointRow; ClearObject (through its kernel vector).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step; ended, A 1, X the slot, N and Z from it.
RichterDeathPieceEnd:
    jsr RichterDeathPieceMove
    bne RichterDeathPieceMove_at0
    lda #$6C
    sta z:zTemp09
    lda #$00
    sta z:zTemp08
    ldx #$06
    jsr RichterDeathPointRow
    ldx z:zRichterObject
    lda a:OBJ_WORK_J
    bne RichterDeathPieceMove_at0
    ldx #$01
RichterDeathPieceEnd_at0:
    jsr VecClearObject
    inx
    cpx #$10
    bne RichterDeathPieceEnd_at0
    lda #$01
    sta z:zField89
    ldx z:zRichterObject
    rts

; @routine 01BD:241 RichterCompanionKind1
; @run 6E:C241
; WHAT IT DOES: Slot 1 or 2 holding kind 1 or 2. State 0: when its timer runs out (or was 0), animation
; $11, sprite set 3, state 1, its place from the player's by the offsets at $C2EF and $C2F3 (by the slot
; less 1 and bit 1 of zFrameTick, kept in OBJ_WORK_E), and the timer $10 when the stage's terrain $10 below
; him has bit 3. State 1: when the timer runs out, script 8 and the slot emptied; otherwise it rises with
; OBJ_WORK_D, and once the animation ends goes back to state 0 unless the player's OBJ_ANIM_TIMER is below
; 8, his invincibility runs, or the game is in sub-mode 3 (then emptied).
; HOW: RichterObjectSetAnimation; StageTileRead ($8006); RichterStartHitScript with X pushed;
; RichterObjectAnimationWait; ClearObject (through its kernel vector).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
RichterCompanionKind1:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne RichterCompanionKind1_at0
    lda a:OBJ_ANIM_TIMER,x
    beq RichterCompanionKind1_at1
    dec a:OBJ_ANIM_TIMER,x
    bne RichterCompanionKind1_at2
RichterCompanionKind1_at1:
    lda #$11
    jsr RichterObjectSetAnimation
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
    adc a:RichterCompanionOffsetsY,y
    sta a:OBJ_Y_LO,x
    cla
    adc a:OBJ_Y_HI
    sta a:OBJ_Y_HI,x
    lda a:OBJ_X_LO
    clc
    adc a:RichterCompanionOffsetsX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_LO
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_Y_LO
    clc
    adc #$10
    sta z:zTemp02
    bcs RichterCompanionKind1_at2
    stz z:zTemp03
    jsr StageVecTerrainTileRead
    and #$08
    beq RichterCompanionKind1_at2
    lda #$10
    sta a:OBJ_ANIM_TIMER,x
RichterCompanionKind1_at2:
    rts
RichterCompanionKind1_at0:
    lda a:OBJ_ANIM_TIMER,x
    beq RichterCompanionKind1_at3
    dec a:OBJ_ANIM_TIMER,x
    bne RichterCompanionKind1_at3
    phx
    lda #$08
    jsr RichterStartHitScript
    plx
RichterCompanionKind1_at4:
    jmp VecClearObject
RichterCompanionKind1_at3:
    ldy a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    eor #$FF
    inc a
    clc
    adc a:RichterCompanionOffsetsY,y
    clc
    adc a:OBJ_Y_LO
    sta a:OBJ_Y_LO,x
    inc a:OBJ_WORK_D,x
    jsr RichterObjectAnimationWait
    lda a:OBJ_ANIM_TIMER
    cmp #$08
    bcc RichterCompanionKind1_at4
    lda a:PlayerInvincibleTimer
    bne RichterCompanionKind1_at4
    lda z:zGameSubMode
    cmp #$03
    beq RichterCompanionKind1_at4
    stz a:OBJ_WORK_D,x
    stz a:OBJ_STATE,x
    rts

; @routine 01BD:2F7 RichterCompanionKind3
; @run 6E:C2F7
; WHAT IT DOES: Slot 1 or 2 holding kind 3: emptied while the player is in state 9; state 0 takes his
; sprite flip, goes to his place, sprite set 3, frame $14, timer $80; state 1 stays at his place, marked,
; and is emptied when the timer runs out.
; HOW: RichterObjectTake2FE8, RichterObjectAtPlayerPlace, RichterObjectNextState, RichterObjectMarkOn;
; ClearObject (through its kernel vector).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
RichterCompanionKind3:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    beq RichterCompanionKind3_at0
    lda a:OBJ_STATE,x
    bne RichterCompanionKind3_at1
    jsr RichterObjectTake2FE8
    jsr RichterObjectAtPlayerPlace
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$14
    sta a:OBJ_FRAME,x
    lda #$80
    sta a:OBJ_ANIM_TIMER,x
    jmp RichterObjectNextState
RichterCompanionKind3_at1:
    jsr RichterObjectAtPlayerPlace
    jsr RichterObjectMarkOn
    dec a:OBJ_ANIM_TIMER,x
    beq RichterCompanionKind3_at0
    rts
RichterCompanionKind3_at0:
    jmp VecClearObject

; @routine 01BD:32C RichterCompanions
; @run 6E:C32C
; WHAT IT DOES: Runs slots 1 and 2.
; HOW: zRichterObject each slot; RichterCompanionByKind.
; WAITS: no.
; Entry: nothing. Exit: X 3, N clear, Z and C set; A and Y as the last handler left them.
RichterCompanions:
    ldx #$01
RichterCompanions_at0:
    stx z:zRichterObject
    bsr RichterCompanionByKind
    inx
    cpx #$03
    bne RichterCompanions_at0
    rts

; @routine 01BD:338 RichterCompanionByKind
; @run 6E:C338
; WHAT IT DOES: Runs slot 1's or 2's handler by its kind: none for 0, RichterCompanionKind1 for 1 and 2,
; RichterCompanionKind3, RichterCompanionKind4.
; HOW: The five words at $C340 by OBJ_KIND.
; WAITS: no.
; Entry: X the slot. Exit: as the handler leaves them.
RichterCompanionByKind:
    lda a:OBJ_KIND,x
    asl a
    tax
    jmp (RichterCompanionKinds,x)

; @routine 01BD:34A RichterCompanionKind4
; @run 6E:C34A
; WHAT IT DOES: Slot 1 or 2 holding kind 4: state 0 takes the player's facing, marked, put at him,
; sprite set 3, animation $1B, state 1; state 1 moves with the scroll and stays at him while the
; animation runs, emptied at its end.
; HOW: RichterObjectFromPlayer, RichterObjectMarkOn, RichterCompanionAtPlayer, RichterObjectSetAnimation,
; RichterObjectNextState; RichterObjectScrollAndMove, RichterObjectAnimationRunning; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last call left them.
RichterCompanionKind4:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne RichterCompanionKind4_at0
    jsr RichterObjectFromPlayer
    jsr RichterObjectMarkOn
    bsr RichterCompanionAtPlayer
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$1B
    jsr RichterObjectSetAnimation
    jmp RichterObjectNextState
RichterCompanionKind4_at0:
    jsr RichterObjectScrollAndMove
    jsr RichterObjectAnimationRunning
    beq RichterCompanionKind3_at0

; @routine 01BD:36E RichterCompanionAtPlayer
; @run 6E:C36E
; WHAT IT DOES: Puts a kind 4 companion at the player by the slot and the player's state: the offsets at
; $C386, two bytes at twice the slot less 1 plus the byte at $C396 by his state.
; HOW: A jump to RichterObjectAtPlayer with zTemp04 and zTemp06.
; WAITS: no.
; Entry: X the slot. Exit: as RichterObjectAtPlayer, Y the offset.
RichterCompanionAtPlayer:
    txa
    dec a
    asl a
    ldy a:OBJ_STATE
    clc
    adc a:RichterCompanionPlaceByState,y
    tay
    lda a:RichterCompanionPlaces,y
    sta z:zTemp04
    lda a:RichterCompanionPlaces+1,y
    sta z:zTemp06
    jmp RichterObjectAtPlayer

; @routine 01BD:3A7 RichterObjects
; @run 6E:C3A7
; WHAT IT DOES: Richter's objects' frame: slots 1 and 2, then each of slots 3 to 15 by its kind.
; HOW: RichterCompanions; RichterObjectByKind for X 3 to $0F.
; WAITS: no.
; Entry: nothing. Exit: X $10, N clear, Z and C set; A and Y as the last handler left them.
RichterObjects:
    jsr RichterCompanions
    ldx #$03
RichterObjects_at0:
    bsr RichterObjectByKind
    inx
    cpx #$10
    bne RichterObjects_at0
    rts

; @routine 01BD:3B4 RichterObjectByKind
; @run 6E:C3B4
; WHAT IT DOES: Runs an object of slots 3 to 15 by its kind: the 23 handlers at $C3BE.
; HOW: zRichterObject the slot; the jump by twice OBJ_KIND.
; WAITS: no.
; Entry: X the slot. Exit: as the handler leaves them.
RichterObjectByKind:
    stx z:zRichterObject
    lda a:OBJ_KIND,x
    asl a
    tax
    jmp (RichterObjectKinds,x)

; @routine 01BD:3F5 RichterObjectMarkThenFromPlayer
; @run 6E:C3F5
; WHAT IT DOES: Marks the object on, then as RichterObjectFromPlayer.
; HOW: RichterObjectMarkOn; runs on into RichterObjectFromPlayer.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectFromPlayer.
RichterObjectMarkThenFromPlayer:
    jsr RichterObjectMarkOn

; @routine 01BD:40C RichterWhipFrame
; @run 6E:C40C
; WHAT IT DOES: The whip, kind 1: in state 0, hit timer 5, his facing and place, and the animation for his
; state (the bytes at $C505) started. After: unmarked, his facing, its charge counted and checked, his place,
; the animation by his state (none for $FF, emptied then); emptied unless he is at step 1 attacking and the
; animation runs; quickened, its sprite flipped from the charge's 14th frame on, flashed, struck, hitbox
; on; marked while it is above row $28.
; HOW: RichterObjectFromPlayer, RichterObjectAtPlayerPlace, RichterObjectStartAnimation; RichterObjectMarkOff,
; RichterObjectCharge, RichterObjectQuicken, RichterObjectAnimationRunning, RichterChargePads,
; RichterObjectFlash, RichterWhipStrike, RichterObjectHitboxOn, RichterObjectMarkOn; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
RichterWhipFrame:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne RichterWhipFrame_at0
    lda #$05
    sta a:OBJ_WORK_N,x
    jsr RichterObjectFromPlayer
    jsr RichterObjectAtPlayerPlace
    ldy a:OBJ_STATE
    lda a:RichterWhipAnimations,y
    jmp RichterObjectStartAnimation
RichterWhipFrame_at0:
    jsr RichterObjectMarkOff
    jsr RichterObjectFromPlayer
    inc a:OBJ_WORK_D,x
    jsr RichterObjectCharge
    jsr RichterObjectAtPlayerPlace
    ldy a:OBJ_STATE
    lda a:RichterWhipAnimations,y
    cmp #$FF
    sta a:OBJ_ANIMATION,x
    beq RichterWhipFrame_at1
    lda a:OBJ_SUBSTATE
    cmp #$01
    bne RichterWhipFrame_at1
    jsr RichterObjectQuicken
    jsr RichterObjectAnimationRunning
    beq RichterWhipFrame_at1
    lda a:OBJ_WORK_D,x
    cmp #$0E
    bcc RichterWhipFrame_at2
    sec
    sbc #$0E
    tay
    lda a:RichterChargePads+2,y
    ora #$80
    sta a:OBJ_SPRITE_ATTR,x
RichterWhipFrame_at2:
    jsr RichterObjectFlash
    jsr RichterWhipStrike
    jsr RichterObjectHitboxOn
    lda a:OBJ_Y_LO,x
    cmp #$28
    bcc RichterWhipFrame_at3
    rts
RichterWhipFrame_at3:
    jmp RichterObjectMarkOn
RichterWhipFrame_at1:
    jmp VecClearObject

; @routine 01BD:512 RichterWhipStrike
; @run 6E:C512
; WHAT IT DOES: When the object that tested the whip is an enemy (slot $10 up) with OBJ_HITBOX_ON 2 or more:
; the spark and PlayerWhipStruck; then OBJ_HIT_BY_SLOT is cleared.
; HOW: RichterWhipSpark.
; WAITS: no.
; Entry: X the whip's slot. Exit: Y the hitter's slot; as the compares or RichterWhipSpark left the rest, A 1
; after a strike.
RichterWhipStrike:
    ldy a:OBJ_HIT_BY_SLOT,x
    cpy #$10
    bcc RichterWhipStrike_at0
    lda a:OBJ_HITBOX_ON,y
    cmp #$02
    bcc RichterWhipStrike_at0
    bsr RichterWhipSpark
    lda #$01
    sta a:PlayerWhipStruck
RichterWhipStrike_at0:
    stz a:OBJ_HIT_BY_SLOT,x
    rts

; @routine 01BD:52B RichterWhipSpark
; @run 6E:C52B
; WHAT IT DOES: A spark where the whip struck, unless slot 4 is in use: the middle of the stage's report of
; the hit (StageHitReport's boxes), kind $16 in the next slot, frame $7D, sprite set 1, animation $1A.
; HOW: Averages by ADC and ROR; ClearObject (through its kernel vector), RichterObjectSetAnimation; X back.
; WAITS: no.
; Entry: X the whip's slot. Exit: slot 4 in use, A its kind, N and Z from it; else X the whip's slot, N and
; Z from it, A, Y, C and V as RichterObjectSetAnimation left them.
RichterWhipSpark:
    lda a:StageHitReport+2
    clc
    adc a:StageHitReport
    ror a
    clc
    adc a:StageHitReport
    ror a
    sta z:zTemp00
    lda a:StageHitReport+3
    clc
    adc a:StageHitReport+1
    ror a
    sta z:zTemp01
    lda a:OBJ_KIND+4
    beq RichterWhipSpark_at0
    rts
RichterWhipSpark_at0:
    inx
    jsr VecClearObject
    lda #$16
    sta a:OBJ_KIND,x
    lda z:zTemp00
    sta a:OBJ_X_LO,x
    lda z:zTemp01
    sta a:OBJ_Y_LO,x
    lda #$7D
    sta a:OBJ_FRAME,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$1A
    jsr RichterObjectSetAnimation
    dex
    rts

; @routine 01BD:56E RichterKind22Frame
; @run 6E:C56E
; WHAT IT DOES: Kind $16, the whip's spark: moves with the scroll until its animation ends, then is emptied.
; HOW: RichterObjectScrollAndMove, RichterObjectAnimationRunning; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectAnimationRunning.
RichterKind22Frame:
    ldx z:zRichterObject
    jsr RichterObjectScrollAndMove
    jsr RichterObjectAnimationRunning
    beq RichterKind22Frame_at0
    rts
RichterKind22Frame_at0:
    jmp VecClearObject

; @routine 01BD:57C RichterKind2Frame
; @run 6E:C57C
; WHAT IT DOES: Kind 2, an item thrown in an arc: in flight, as RichterKind2Fly; at state 0, emptied while
; the player is hurt; else, once RichterFlagsAgainstFour lets it, his facing, his place offset ($14, $E8),
; the throw's speeds (X entry 0 by facing, Y entry 1), hit timer 8, hitbox on, animation 7 started.
; HOW: RichterFlagsAgainstFour; RichterObjectFromPlayer, RichterObjectAtPlayer, SetVelocityXFromTable,
; NegateVelocityXIfFacingLeft, SetVelocityYFromTable; runs on into RichterObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind2Frame:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    beq RichterKind2Frame_at0
    jmp RichterKind2Fly
RichterKind2Frame_at0:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    bne RichterKind2Frame_at1
    jmp VecClearObject
RichterKind2Frame_at1:
    jsr RichterFlagsAgainstFour
    beq RichterKind2Frame_at2
    rts
RichterKind2Frame_at2:
    jsr RichterObjectFromPlayer
    lda #$14
    sta z:zTemp04
    lda #$E8
    sta z:zTemp06
    jsr RichterObjectAtPlayer
    lda #$00
    jsr SetVelocityXFromTable
    jsr NegateVelocityXIfFacingLeft
    lda #$01
    jsr SetVelocityYFromTable
    lda #$08
    sta a:OBJ_WORK_N,x
    inc a:OBJ_HITBOX_ON,x
    lda #$07

; @routine 01BD:5BD RichterObjectStartAnimation
; @run 6E:C5BD
; WHAT IT DOES: Starts an object's animation, sprite set 1, and its next state.
; HOW: RichterObjectSetAnimation; runs on into RichterObjectNextState.
; WAITS: no.
; Entry: A the animation, X object. Exit: as RichterObjectNextState.
RichterObjectStartAnimation:
    jsr RichterObjectSetAnimation
    lda #$01
    sta a:OBJ_SPRITE_SET,x

; @routine 01BD:5C9 RichterKind2Fly
; @run 6E:C5C9
; WHAT IT DOES: Kind 2 in flight: with its hitbox on, sound 6 every eighth wave count; deflected (OBJ_WORK_P
; 2), hitbox and mark off, the bounce's speeds (X entry 7 by facing, Y entry 2), animation $1C and sound $11;
; animation 8 once its own ends; hitbox by frame; emptied below the map or off the screen's sides; else moved
; with the scroll and falling at $44 a frame to $0700.
; HOW: zWaveCounter; SoundCommand; SetVelocityXFromTable, NegateVelocityXIfFacingLeft, SetVelocityYFromTable;
; RichterObjectSetAnimation, RichterObjectAnimationRunning, RichterObjectHitboxByFrame; ClearObject;
; RichterObjectScrollAndMove; runs on into RichterObjectFallCapped.
; WAITS: no.
; Entry: X the slot. Exit: as its last instruction or call left them.
RichterKind2Fly:
    lda a:OBJ_HITBOX_ON,x
    beq RichterKind2Fly_at0
    lda z:zWaveCounter
    and #$07
    bne RichterKind2Fly_at0
    lda #$06
    jsr VecSoundCommand
RichterKind2Fly_at0:
    lda a:OBJ_WORK_P,x
    cmp #$02
    bne RichterKind2Fly_at1
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_WORK_P,x
    lda #$07
    jsr SetVelocityXFromTable
    jsr NegateVelocityXIfFacingLeft
    lda #$02
    jsr SetVelocityYFromTable
    lda #$1C
    jsr RichterObjectSetAnimation
    lda #$11
    jsr VecSoundCommand
RichterKind2Fly_at1:
    jsr RichterObjectAnimationRunning
    bne RichterKind2Fly_at2
    lda #$08
    jsr RichterObjectSetAnimation
RichterKind2Fly_at2:
    jsr RichterObjectHitboxByFrame
    lda a:OBJ_Y_HI,x
    cmp #$01
    beq RichterKind2Fly_at3
    lda a:OBJ_Y_HI,x
    cmp #$FF
    beq RichterKind2Fly_at4
    lda a:OBJ_Y_LO,x
    cmp #$F0
    bcs RichterKind2Fly_at3
RichterKind2Fly_at4:
    lda a:OBJ_X_HI,x
    beq RichterKind2Fly_at5
    lda a:OBJ_X_LO,x
    cmp #$10
    bcc RichterKind2Fly_at5
    cmp #$F0
    bcs RichterKind2Fly_at5
RichterKind2Fly_at3:
    jmp VecClearObject
RichterKind2Fly_at5:
    jsr RichterObjectScrollAndMove
    lda #$44

; @routine 01BD:637 RichterObjectFallCapped
; @run 6E:C637
; WHAT IT DOES: Adds A to an object's Y speed as a fall, held at $0700.
; HOW: RichterObjectSetFall; at 7 and up the speed is $0700.
; WAITS: no.
; Entry: A the pull, X object. Exit: as RichterObjectSetFall, or A 7, N and Z from it, C set.
RichterObjectFallCapped:
    clc
    adc a:OBJ_VY_LO,x
    jsr RichterObjectSetFall
    bmi RichterObjectFallCapped_at0
    cmp #$07
    bcc RichterObjectFallCapped_at0
    lda #$07
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
RichterObjectFallCapped_at0:
    rts

; @routine 01BD:64D RichterKind3Frame
; @run 6E:C64D
; WHAT IT DOES: Kind 3: runs its state's handler.
; HOW: RichterStateDoubled; the five words at $C653.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind3Frame:
    jsr RichterStateDoubled
    jmp (RichterKind3States,x)

; @routine 01BD:65D RichterKind3Start
; @run 6E:C65D
; WHAT IT DOES: Kind 3's state 0: emptied while the player is hurt; once RichterFlagsAgainstFour lets it, his
; facing, his place offset ($0A, $EF), X speed entry 1 by facing, hit timer 9, animation 9 started.
; HOW: RichterFlagsAgainstFour, RichterObjectFromPlayer, RichterObjectAtPlayer, SetVelocityXFromTable,
; NegateVelocityXIfFacingLeft; a jump to RichterObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind3Start:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    bne RichterKind3Start_at0
    jmp VecClearObject
RichterKind3Start_at0:
    jsr RichterFlagsAgainstFour
    beq RichterKind3Start_at1
    rts
RichterKind3Start_at1:
    jsr RichterObjectFromPlayer
    lda #$0A
    sta z:zTemp04
    lda #$EF
    sta z:zTemp06
    jsr RichterObjectAtPlayer
    lda #$01
    jsr SetVelocityXFromTable
    jsr NegateVelocityXIfFacingLeft
    lda #$09
    sta a:OBJ_WORK_N,x
    lda #$09
    jmp RichterObjectStartAnimation

; @routine 01BD:68F RichterKind3Rise
; @run 6E:C68F
; WHAT IT DOES: Kind 3's second state: its count up; emptied while the player is hurt; animated, moved with
; the scroll, held $11 above his Y; the next state when the count reaches 2.
; HOW: RichterObjectAnimationTick, RichterObjectScrollAndMove, RichterObjectNextState; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
RichterKind3Rise:
    ldx z:zRichterObject
    inc a:OBJ_WORK_E,x
    lda a:OBJ_STATE
    cmp #$09
    bne RichterKind3Rise_at0
    jmp VecClearObject
RichterKind3Rise_at0:
    jsr RichterObjectAnimationTick
    jsr RichterObjectScrollAndMove
    lda a:OBJ_Y_LO
    clc
    adc #$EF
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    adc #$FF
    sta a:OBJ_Y_HI,x
    lda a:OBJ_WORK_E,x
    cmp #$02
    beq RichterKind3Rise_at1
    rts
RichterKind3Rise_at1:
    jmp RichterObjectNextState

; @routine 01BD:6C0 RichterKind3Out
; @run 6E:C6C0
; WHAT IT DOES: Kind 3's first leg: deflected (OBJ_WORK_P 2), its count to $30, timer $40, X speed the byte at
; $C7C7 by facing (both bytes), sound $11; then the count, and as RichterKind3Move.
; HOW: SoundCommand; RichterKind3Count; runs on into RichterKind3Move.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterKind3Move.
RichterKind3Out:
    ldx z:zRichterObject
    lda a:OBJ_WORK_P,x
    cmp #$02
    bne RichterKind3Out_at0
    lda #$30
    sta a:OBJ_WORK_E,x
    lda #$40
    sta a:OBJ_ANIM_TIMER,x
    ldy a:OBJ_FACING_LEFT,x
    lda a:RichterKind3DeflectSpeeds,y
    sta a:OBJ_VX_LO,x
    sta a:OBJ_VX_HI,x
    stz a:OBJ_WORK_P,x
    lda #$11
    jsr VecSoundCommand
RichterKind3Out_at0:
    jsr RichterKind3Count

; @routine 01BD:6EA RichterKind3Move
; @run 6E:C6EA
; WHAT IT DOES: Kind 3 on its way: the trail, the animation, the hitbox, moved with the scroll; its X speed
; eased by the word at $C80F by facing until it is the byte at $C813, and the next state when the speed's
; sign turns.
; HOW: RichterKind3Trail, RichterObjectAnimationTick, RichterObjectHitboxOn, RichterObjectScrollAndMove.
; WAITS: no.
; Entry: X the slot. Exit: as its last instruction left them.
RichterKind3Move:
    jsr RichterKind3Trail
    jsr RichterObjectAnimationTick
    jsr RichterObjectHitboxOn
    jsr RichterObjectScrollAndMove
    lda a:OBJ_FACING_LEFT,x
    tay
    lda a:OBJ_VX_HI,x
    cmp a:RichterKind3TopSpeeds,y
    beq RichterKind3Move_at0
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:RichterKind3Eases,y
    clc
    adc a:OBJ_VX_LO,x
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    sta z:zTemp00
    lda a:RichterKind3Eases+1,y
    adc a:OBJ_VX_HI,x
    sta a:OBJ_VX_HI,x
    eor z:zTemp00
    and #$80
    beq RichterKind3Move_at0
    inc a:OBJ_STATE,x
RichterKind3Move_at0:
    rts

; @routine 01BD:729 RichterKind3Back
; @run 6E:C729
; WHAT IT DOES: Kind 3 coming back: deflected when its timer is out, the bounce (X speed by the facing turned,
; Y entry 2), timer $60, animation $0A, hitbox off, sound $11, the next state; otherwise the count, and
; emptied when it meets the player or reaches the screen's edge it was thrown from; else as
; RichterKind3Move.
; HOW: The words at $C98D; SetVelocityYFromTable; SoundCommand; RichterObjectNextState; RichterKind3Count;
; ObjectTouchesPlayerBox; the X high bytes at $C7FD; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind3Back:
    ldx z:zRichterObject
    lda a:OBJ_ANIM_TIMER,x
    beq RichterKind3Back_at0
    dec a:OBJ_ANIM_TIMER,x
    bne RichterKind3Back_at1
RichterKind3Back_at0:
    lda a:OBJ_WORK_P,x
    cmp #$02
    bne RichterKind3Back_at1
    lda a:OBJ_FACING_LEFT,x
    inc a
    and #$01
    asl a
    tay
    lda a:RichterBounceSpeedsX,y
    sta a:OBJ_VX_LO,x
    lda a:RichterBounceSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda #$02
    jsr SetVelocityYFromTable
    lda #$60
    sta a:OBJ_ANIM_TIMER,x
    lda #$0A
    sta a:OBJ_ANIMATION,x
    stz a:OBJ_HITBOX_ON,x
    lda #$11
    jsr VecSoundCommand
    jmp RichterObjectNextState
RichterKind3Back_at1:
    stz a:OBJ_WORK_P,x
    jsr RichterKind3Count
    jsr ObjectTouchesPlayerBox
    beq RichterKind3Back_at2
    ldy a:OBJ_FACING_LEFT,x
    lda a:OBJ_X_HI,x
    cmp a:RichterKind3EdgeScreens,y
    beq RichterKind3Back_at2
    jmp RichterKind3Move
RichterKind3Back_at2:
    jmp VecClearObject

; @routine 01BD:786 RichterKind3Drop
; @run 6E:C786
; WHAT IT DOES: Kind 3 after a deflection: falls as RichterObjectFall38, emptied when its timer runs out.
; HOW: RichterObjectFall38; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: counting, N and Z from the timer.
RichterKind3Drop:
    jsr RichterObjectFall38
    lda a:OBJ_ANIM_TIMER,x
    beq RichterKind3Back_at2
    dec a:OBJ_ANIM_TIMER,x
    beq RichterKind3Back_at2
    rts

; @routine 01BD:794 RichterKind3Count
; @run 6E:C794
; WHAT IT DOES: Counts kind 3's OBJ_WORK_E up (emptied at $FF), its animation the byte at $C7FF by the count's
; high nibble; sound 7 every sixteenth count, or with animation $0B sound 8 every fourth.
; HOW: Tail jumps to SoundCommand; ClearObject at $FF.
; WAITS: no.
; Entry: X the slot. Exit: as its last instruction or call left them.
RichterKind3Count:
    lda a:OBJ_WORK_E,x
    cmp #$FF
    beq RichterKind3Back_at2
    inc a:OBJ_WORK_E,x
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    lda a:RichterKind3Animations,y
    sta a:OBJ_ANIMATION,x
    cmp #$0B
    beq RichterKind3Count_at0
    lda a:OBJ_WORK_E,x
    and #$0F
    beq RichterKind3Count_at1
    rts
RichterKind3Count_at1:
    lda #$07
    jmp VecSoundCommand
RichterKind3Count_at0:
    lda a:OBJ_WORK_E,x
    and #$03
    beq RichterKind3Count_at2
    rts
RichterKind3Count_at2:
    lda #$08
    jmp VecSoundCommand

; @routine 01BD:815 RichterKind3Trail
; @run 6E:C815
; WHAT IT DOES: Every eighth frame by OBJ_WORK_F, a kind 9 object at this one's place in the first free of
; the next two slots.
; HOW: ClearObjectInYAndCopyPlace.
; WAITS: no.
; Entry: X the slot. Exit: as its last instruction or call left them; Y the slot looked at.
RichterKind3Trail:
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_F,x
    cmp #$08
    beq RichterKind3Trail_at0
    rts
RichterKind3Trail_at0:
    stz a:OBJ_WORK_F,x
    txa
    tay
    iny
    lda a:OBJ_KIND,y
    beq RichterKind3Trail_at1
    iny
    lda a:OBJ_KIND,y
    beq RichterKind3Trail_at1
    rts
RichterKind3Trail_at1:
    jsr ClearObjectInYAndCopyPlace
    lda #$09
    sta a:OBJ_KIND,y
    rts

; @routine 01BE:03B RichterKind9Frame
; @run 6E:C83B
; WHAT IT DOES: Kind 9, kind 3's trail: state 0 takes the player's facing, Y speed entry 3, animation $0C
; started; then moved with the scroll until its animation ends, and emptied.
; HOW: RichterObjectFromPlayer, SetVelocityYFromTable, RichterObjectStartAnimation;
; RichterObjectScrollAndMove, RichterObjectAnimationWait; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind9Frame:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne RichterKind9Frame_at0
    jsr RichterObjectFromPlayer
    lda #$03
    jsr SetVelocityYFromTable
    lda #$0C
    jmp RichterObjectStartAnimation
RichterKind9Frame_at0:
    jsr RichterObjectScrollAndMove
    jsr RichterObjectAnimationWait
    jmp VecClearObject

; @routine 01BE:058 RichterKind5Frame
; @run 6E:C858
; WHAT IT DOES: Kind 5: runs its state's handler.
; HOW: RichterStateDoubled; the four words at $C85E.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind5Frame:
    jsr RichterStateDoubled
    jmp (RichterKind5States,x)

; @routine 01BE:078 RichterKind5Spread
; @run 6E:C878
; WHAT IT DOES: The first of a kind 5 throw (slot 5, 8 or $0B) makes the next two slots kind 5 too, then aims
; and throws as RichterKind5Start.
; HOW: OBJ_KIND; RichterKind5Start's aim from $C8A6.
; WAITS: no.
; Entry: X the slot. Exit: as RichterKind5Throw.
RichterKind5Spread:
    inx
    lda #$05
    sta a:OBJ_KIND,x
    inx
    sta a:OBJ_KIND,x
    dex
    dex
    bra RichterKind5Start_at0

; @routine 01BE:086 RichterKind5Start
; @run 6E:C886
; WHAT IT DOES: Kind 5's state 0: emptied while the player is hurt; once RichterFlagsAgainstFour lets it, the
; first of three (slots 5, 8, $0B) spreads to the next two; each is aimed by its place and thrown.
; HOW: RichterFlagsAgainstFour; RichterKind5Spread; the words at $C866; runs on into RichterKind5Throw.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind5Start:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    bne RichterKind5Start_at1
    jmp VecClearObject
RichterKind5Start_at1:
    jsr RichterFlagsAgainstFour
    beq RichterKind5Start_at2
    rts
RichterKind5Start_at2:
    cpx #$05
    beq RichterKind5Spread
    cpx #$08
    beq RichterKind5Spread
    cpx #$0B
    beq RichterKind5Spread
RichterKind5Start_at0:
    txa
    sec
    sbc #$05
    asl a
    tay
    lda a:RichterKind5SpeedsY,y
    sta a:OBJ_VY_LO,x
    lda a:RichterKind5SpeedsY+1,y
    sta a:OBJ_VY_HI,x

; @routine 01BE:0B6 RichterKind5Throw
; @run 6E:C8B6
; WHAT IT DOES: Throws a kind 5: sound 7, the player's facing, his place offset ($0A, $F5), X speed entry 2
; by facing, hit timer 6, animation 6 started, frame $49.
; HOW: SoundCommand, RichterObjectFromPlayer, RichterObjectAtPlayer, SetVelocityXFromTable,
; NegateVelocityXIfFacingLeft, RichterObjectStartAnimation.
; WAITS: no.
; Entry: X the slot. Exit: A $49, N and Z from the next state; the rest as RichterObjectStartAnimation.
RichterKind5Throw:
    lda #$07
    jsr VecSoundCommand
    jsr RichterObjectFromPlayer
    lda #$0A
    sta z:zTemp04
    lda #$F5
    sta z:zTemp06
    jsr RichterObjectAtPlayer
    lda #$02
    jsr SetVelocityXFromTable
    jsr NegateVelocityXIfFacingLeft
    lda #$06
    sta a:OBJ_WORK_N,x
    jsr RichterObjectStartAnimation
    lda #$49
    sta a:OBJ_FRAME,x
    rts

; @routine 01BE:0DF RichterKind5Fly
; @run 6E:C8DF
; WHAT IT DOES: Kind 5 (and kind 12) in flight: emptied off range; while the object that tested it is not
; striking (OBJ_HITBOX_ON 2) or nothing hit it, moved with the scroll, hitbox on, OBJ_WORK_P cleared. Hit
; (OBJ_WORK_P 1): stopped, frame $3D for 6 frames, the next state, hitbox off, or for 4 frames with it on
; when the hitter's is. Deflected (2): hitbox off, the bounce's X speed by facing, animation $0D, Y entry 2,
; sound $11, state 3.
; HOW: ClearObjectIfOffRange; OBJ_HIT_BY_SLOT; RichterObjectScrollAndMove, RichterObjectHitboxOn,
; RichterObjectStopX; the words at $C98D; RichterObjectSetAnimation, SetVelocityYFromTable, SoundCommand.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
RichterKind5Fly:
    ldx z:zRichterObject
    jsr ClearObjectIfOffRange
    ldy a:OBJ_HIT_BY_SLOT,x
    lda a:OBJ_HITBOX_ON,y
    cmp #$02
    bne RichterKind5Fly_at0
    lda a:OBJ_WORK_P,x
    cmp #$01
    beq RichterKind5Fly_at1
    cmp #$02
    beq RichterKind5Fly_at2
RichterKind5Fly_at0:
    jsr RichterObjectScrollAndMove
    jsr RichterObjectHitboxOn
    stz a:OBJ_WORK_P,x
    rts
RichterKind5Fly_at1:
    jsr RichterObjectStopX
    lda #$3D
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_HURT_TIMER,x
    inc a:OBJ_STATE,x
    stz a:OBJ_HITBOX_ON,x
    ldy a:OBJ_HIT_BY_SLOT,x
    lda a:OBJ_HITBOX_ON,y
    beq RichterKind5Fly_at3
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$04
    sta a:OBJ_HURT_TIMER,x
RichterKind5Fly_at3:
    rts
RichterKind5Fly_at2:
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:RichterBounceSpeedsX,y
    sta a:OBJ_VX_LO,x
    lda a:RichterBounceSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda #$0D
    jsr RichterObjectSetAnimation
    lda #$02
    jsr SetVelocityYFromTable
    lda #$11
    jsr VecSoundCommand
    lda #$03
    sta a:OBJ_STATE,x
    rts

; @routine 01BE:152 RichterKind5Stuck
; @run 6E:C952
; WHAT IT DOES: Kind 5 after a hit: moved with the scroll, emptied when its timer runs out.
; HOW: RichterObjectScrollAndMove; ClearObjectIfOffRange's jump to ClearObject ($C98A).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: counting, N and Z from the timer.
RichterKind5Stuck:
    ldx z:zRichterObject
    jsr RichterObjectScrollAndMove
    dec a:OBJ_HURT_TIMER,x
    beq RichterKind5Stuck_at0
    rts
RichterKind5Stuck_at0:
    jmp ClearObjectIfOffRange_at0

; @routine 01BE:160 RichterObjectFall38
; @run 6E:C960
; WHAT IT DOES: An object falls: emptied off range, moved with the scroll, $38 more Y speed, animated.
; HOW: ClearObjectIfOffRange, RichterObjectScrollAndMove, RichterObjectSetFall, a tail jump to
; RichterObjectAnimationTick.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectAnimationTick.
RichterObjectFall38:
    ldx z:zRichterObject
    jsr ClearObjectIfOffRange
    jsr RichterObjectScrollAndMove
    lda a:OBJ_VY_LO,x
    clc
    adc #$38
    jsr RichterObjectSetFall
    jmp RichterObjectAnimationTick

; @routine 01BE:191 RichterKind6Frame
; @run 6E:C991
; WHAT IT DOES: Kind 6: runs its state's handler.
; HOW: RichterStateDoubled; the seven words at $C997.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind6Frame:
    jsr RichterStateDoubled
    jmp (RichterKind6States,x)

; @routine 01BE:1A5 RichterKind6Start
; @run 6E:C9A5
; WHAT IT DOES: Kind 6's state 0: the first (slot 4) sounds $5F, sets TimeSlowMode, zFadeCount and
; zFadeActive, and makes slots 4 to 15 kind 6; each takes its place from the four bytes at $CA87 by four
; times its slot less 4 (the first from its own), animation $16, sprite set 1, its circling start, the next
; state.
; HOW: SoundCommand; RichterObjectSetAnimation; RichterKind6CircleStart; a jump to RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectNextState.
RichterKind6Start:
    ldx z:zRichterObject
    txa
    sec
    sbc #$04
    asl a
    asl a
    cmp #$04
    bne RichterKind6Start_at0
    lda #$5F
    jsr VecSoundCommand
    lda #$01
    sta a:TimeSlowMode
    sta z:zFadeCount
    sta z:zFadeActive
    ldy #$04
    lda #$06
RichterKind6Start_at1:
    sta a:OBJ_KIND,y
    iny
    cpy #$10
    bne RichterKind6Start_at1
    lda #$04
RichterKind6Start_at0:
    tay
    lda a:RichterKind6Places,y
    sta a:OBJ_X_LO,x
    lda a:RichterKind6Places+1,y
    sta a:OBJ_X_HI,x
    lda a:RichterKind6Places+2,y
    sta a:OBJ_Y_LO,x
    lda a:RichterKind6Places+3,y
    sta a:OBJ_Y_HI,x
    lda #$16
    jsr RichterObjectSetAnimation
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    jsr RichterKind6CircleStart
    jmp RichterObjectNextState

; @routine 01BE:1F6 RichterKind6Circle
; @run 6E:C9F6
; WHAT IT DOES: Kind 6 circling: a step of the circle, the animation; when the step count reaches $73,
; animation $0E and the next state.
; HOW: RichterKind6CircleStep, RichterObjectAnimationTick; RichterKind6Count's RTS ($CA36) before then.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
RichterKind6Circle:
    ldx z:zRichterObject
    jsr RichterKind6CircleStep
    jsr RichterObjectAnimationTick
    lda a:OBJ_ANIM_TIMER,x
    cmp #$73
    bne RichterKind6Count_at0
    lda #$0E
    sta a:OBJ_ANIMATION,x
    jmp RichterObjectNextState

; @routine 01BE:20D RichterKind6Hold
; @run 6E:CA0D
; WHAT IT DOES: Kind 6, once its animation ends: the two timers by its slot (the words at $CAB7), the next
; state.
; HOW: RichterObjectAnimationWait; a jump to RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: waiting, as RichterObjectAnimationWait; else as
; RichterObjectNextState.
RichterKind6Hold:
    ldx z:zRichterObject
    jsr RichterObjectAnimationWait
    txa
    sec
    sbc #$04
    asl a
    tay
    lda a:RichterKind6Timers,y
    sta a:OBJ_HURT_TIMER,x
    lda a:RichterKind6Timers+1,y
    sta a:OBJ_ANIM_TIMER,x
    jmp RichterObjectNextState

; @routine 01BE:227 RichterKind6Count
; @run 6E:CA27
; WHAT IT DOES: Kind 6 waiting out its two timers, the second counted down each time the first runs out;
; then animation $0F and the next state.
; HOW: OBJ_HURT_TIMER and OBJ_ANIM_TIMER; RichterObjectSetAnimation, RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: counting, N and Z from the count; else as RichterObjectNextState.
RichterKind6Count:
    ldx z:zRichterObject
    dec a:OBJ_HURT_TIMER,x
    bne RichterKind6Count_at0
    lda a:OBJ_ANIM_TIMER,x
    beq RichterKind6Count_at1
    dec a:OBJ_ANIM_TIMER,x
RichterKind6Count_at0:
    rts
RichterKind6Count_at1:
    lda #$0F
    jsr RichterObjectSetAnimation
    jmp RichterObjectNextState

; @routine 01BE:23F RichterKind6Wait
; @run 6E:CA3F
; WHAT IT DOES: Kind 6: the next state once its animation ends.
; HOW: RichterObjectAnimationWait, a jump to RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectAnimationWait or RichterObjectNextState.
RichterKind6Wait:
    ldx z:zRichterObject
    jsr RichterObjectAnimationWait
    jmp RichterObjectNextState

; @routine 01BE:247 RichterKind6Gather
; @run 6E:CA47
; WHAT IT DOES: Kind 6's state 5: the last (slot $0F), once slots 4 to 15 have all reached state 5, puts them
; all in state 6 with animation $10 and sprite set 3.
; HOW: OBJ_STATE of each; RichterObjectSetAnimation with X pushed.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
RichterKind6Gather:
    ldx z:zRichterObject
    cpx #$0F
    bne RichterKind6Gather_at0
    ldy #$03
RichterKind6Gather_at2:
    iny
    cpy #$10
    beq RichterKind6Gather_at1
    lda a:OBJ_STATE,y
    cmp #$05
    beq RichterKind6Gather_at2
RichterKind6Gather_at0:
    rts
RichterKind6Gather_at1:
    phx
    ldx #$04
RichterKind6Gather_at3:
    lda #$06
    sta a:OBJ_STATE,x
    lda #$10
    jsr RichterObjectSetAnimation
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    inx
    cpx #$10
    bne RichterKind6Gather_at3
    plx
    rts

; @routine 01BE:275 RichterKind6Fade
; @run 6E:CA75
; WHAT IT DOES: Kind 6's last state: rises a pixel a frame until its animation ends; then zFadeCount 2,
; TimeSlowMode off, emptied.
; HOW: RichterObjectAnimationWait; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectAnimationWait, or A 2 and the rest as ClearObject
; keeps them.
RichterKind6Fade:
    ldx z:zRichterObject
    dec a:OBJ_Y_LO,x
    jsr RichterObjectAnimationWait
    lda #$02
    sta z:zFadeCount
    stz a:TimeSlowMode
    jmp VecClearObject

; @routine 01BE:2CF RichterKind7End
; @run 6E:CACF
; WHAT IT DOES: Kind 7 ends: if a stage object caught it (PlayerKind7Caught), that is cleared with the item in
; hand; else, unless it struck (OBJ_WORK_D), a kind 3 companion is made sure of; if it struck, the flag is
; cleared and a kind 3 companion in slot 1 or 2 is emptied. Then it is emptied.
; HOW: RichterClaimKind3Slot; ClearObjectInY (through its kernel vector); ClearObject.
; WAITS: no.
; Entry: X the slot. Exit: as the last call left them.
RichterKind7End:
    lda a:PlayerKind7Caught
    bne RichterKind7End_at0
    lda a:OBJ_WORK_D,x
    bne RichterKind7End_at1
    jsr RichterClaimKind3Slot
    bra RichterKind7End_at2
RichterKind7End_at1:
    stz a:PlayerKind7Caught
    ldy #$01
    lda a:OBJ_KIND,y
    cmp #$03
    beq RichterKind7End_at3
    iny
    lda a:OBJ_KIND,y
    cmp #$03
    beq RichterKind7End_at3
    bra RichterKind7End_at2
RichterKind7End_at3:
    jsr VecClearObjectInY
    bra RichterKind7End_at2
RichterKind7End_at0:
    stz a:PlayerKind7Caught
    stz z:zField93
RichterKind7End_at2:
    jmp VecClearObject

; @routine 01BE:301 RichterKind7Frame
; @run 6E:CB01
; WHAT IT DOES: Kind 7, held by the player while he attacks: emptied while he is hurt; ended when his step
; is not 1; at state 0, once his animation reaches step 5, the frame for his state (the bytes at $CB5F),
; sprite set 1, his facing, hitbox on, hit timer $0D, his place, the next state; then the frame and his place
; each frame, and OBJ_WORK_D set the first time an enemy (slot $10 up) with OBJ_HITBOX_ON 2 or more tested it.
; HOW: RichterKind7End; RichterObjectFromPlayer, RichterObjectHitboxOn, RichterObjectAtPlayerPlace,
; RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind7Frame:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    beq RichterKind7End_at2
    lda a:OBJ_SUBSTATE
    cmp #$01
    bne RichterKind7End
    lda a:OBJ_STATE,x
    bne RichterKind7Frame_at0
    lda a:OBJ_ANIMATION_AT
    cmp #$05
    beq RichterKind7Frame_at1
    rts
RichterKind7Frame_at1:
    ldy a:OBJ_STATE
    lda a:RichterKind7Frames,y
    sta a:OBJ_FRAME,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    jsr RichterObjectFromPlayer
    jsr RichterObjectHitboxOn
    lda #$0D
    sta a:OBJ_WORK_N,x
    jsr RichterObjectAtPlayerPlace
    jmp RichterObjectNextState
RichterKind7Frame_at0:
    ldy a:OBJ_STATE
    lda a:RichterKind7Frames,y
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_D,x
    bne RichterKind7Frame_at2
    ldy a:OBJ_HIT_BY_SLOT,x
    cpy #$10
    bcc RichterKind7Frame_at2
    lda a:OBJ_HITBOX_ON,y
    cmp #$02
    bcc RichterKind7Frame_at2
    inc a:OBJ_WORK_D,x
RichterKind7Frame_at2:
    jmp RichterObjectAtPlayerPlace

; @routine 01BE:36C RichterKind4Frame
; @run 6E:CB6C
; WHAT IT DOES: Kind 4: runs its state's handler.
; HOW: RichterStateDoubled; the six words at $CB72.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind4Frame:
    jsr RichterStateDoubled
    jmp (RichterKind4States,x)

; @routine 01BE:37E RichterKind4Start
; @run 6E:CB7E
; WHAT IT DOES: Kind 4's state 0: emptied while the player is hurt; once RichterFlagsAgainstFour lets it, his
; facing, his place offset ($0C, $EE), the throw's speeds (X entry 3 by facing, Y entry 4), hit timer 7,
; sprite set 1, frame $55, the next state.
; HOW: RichterFlagsAgainstFour, RichterObjectFromPlayer, RichterObjectAtPlayer, SetVelocityXFromTable,
; NegateVelocityXIfFacingLeft, SetVelocityYFromTable.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind4Start:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    bne RichterKind4Start_at0
    jmp VecClearObject
RichterKind4Start_at0:
    jsr RichterFlagsAgainstFour
    beq RichterKind4Start_at1
    rts
RichterKind4Start_at1:
    jsr RichterObjectFromPlayer
    lda #$0C
    sta z:zTemp04
    lda #$EE
    sta z:zTemp06
    jsr RichterObjectAtPlayer
    lda #$03
    jsr SetVelocityXFromTable
    jsr NegateVelocityXIfFacingLeft
    lda #$04
    jsr SetVelocityYFromTable
    lda #$07
    sta a:OBJ_WORK_N,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$55
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    rts

; @routine 01BE:3BE RichterObjectProbeBelow
; @run 6E:CBBE
; WHAT IT DOES: RichterObjectProbe straight below an object by A; its answer's bits 0, 4 and 5.
; HOW: zTemp06 A, zTemp04, 05 and 07 zero; AND #$31.
; WAITS: no.
; Entry: A how far below, X the object. Exit: A the bits, N and Z from them; the rest as the read left them.
RichterObjectProbeBelow:
    sta z:zTemp06
    stz z:zTemp07
    stz z:zTemp04
    stz z:zTemp05
    jsr RichterObjectProbe
    and #$31
    rts

; @routine 01BE:3CC RichterObjectProbe
; @run 6E:CBCC
; WHAT IT DOES: Reads the stage's terrain at a point offset from an object (zTemp04-05 across, zTemp06-07
; down); a point past the bottom of the first screen down reads as nothing.
; HOW: The point to zTemp00-03 (Y first; a carry out of its low byte is past the bottom); a tail jump to the
; stage overlay's read at $8006 (StageTileRead).
; WAITS: no.
; Entry: X the object, zTemp04-07 the offset. Exit: past the bottom, A 0, N and Z from the add, C set, V from
; it. Else as the stage's read.
RichterObjectProbe:
    lda a:OBJ_Y_LO,x
    clc
    adc z:zTemp06
    sta z:zTemp02
    bcs RichterObjectProbe_at0
    lda a:OBJ_Y_HI,x
    adc z:zTemp07
    sta z:zTemp03
    lda a:OBJ_X_LO,x
    clc
    adc z:zTemp04
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc z:zTemp05
    sta z:zTemp01
    jmp StageVecTerrainTileRead
RichterObjectProbe_at0:
    cla
    rts

; @routine 01BE:3F1 RichterKind4Fall
; @run 6E:CBF1
; WHAT IT DOES: Kind 4 in the air: emptied below the map; falling on the first screen, a floor 8 below lands
; it (marked when the read has bit 5); else a floor at its feet or $10 below lands it; rising or above, a
; marked floor at its feet lands it; otherwise hitbox on and moved.
; HOW: RichterObjectProbeBelow; RichterKind4LandMarked and the landing; RichterObjectHitAndMove.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind4Fall:
    ldx z:zRichterObject
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne RichterKind4Fall_at0
    jmp VecClearObject
RichterKind4Fall_at0:
    cmp #$FF
    beq RichterKind4Fall_at1
    lda a:OBJ_VY_HI,x
    bmi RichterKind4Fall_at1
    lda #$08
    jsr RichterObjectProbeBelow
    beq RichterKind4Fall_at1
    and #$20
    bne RichterKind4LandMarked
    lda #$00
    jsr RichterObjectProbeBelow
    beq RichterKind4LandMarked_at0
    lda #$10
    jsr RichterObjectProbeBelow
    bne RichterKind4LandMarked
RichterKind4Fall_at1:
    lda #$00
    jsr RichterObjectProbeBelow
    and #$20
    bne RichterKind4LandMarked

; @routine 01BE:428 RichterObjectHitAndMove
; @run 6E:CC28
; WHAT IT DOES: An object's hitbox on, then moved with the scroll.
; HOW: RichterObjectHitboxOn, RichterObjectScrollAndMove.
; WAITS: no.
; Entry: X the object. Exit: as RichterObjectScrollAndMove.
RichterObjectHitAndMove:
    jsr RichterObjectHitboxOn
    jsr RichterObjectScrollAndMove
    rts

; @routine 01BE:42F RichterKind4LandMarked
; @run 6E:CC2F
; WHAT IT DOES: Kind 4 lands on a marked floor: OBJ_WORK_D 1, then the landing.
; HOW: The landing's stores from $CC33.
; WAITS: no.
; Entry: X the slot, zTemp02 the read's row. Exit: as RichterObjectNextState.
RichterKind4LandMarked:
    lda #$01
    sta a:OBJ_WORK_D,x
RichterKind4LandMarked_at0:
    lda #$05
    jsr VecSoundCommand
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO,x
    jsr RichterObjectStop
    stz a:OBJ_Y_FRAC,x
    lda #$56
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_HURT_TIMER,x
    jmp RichterObjectNextState

; @routine 01BE:455 RichterKind4Landed
; @run 6E:CC55
; WHAT IT DOES: Kind 4 on the floor: hitbox and move; when its timer runs out, landed plain: animation $12, X
; speed entry 6 by facing, its place kept in OBJ_WORK_F to H, state 4; landed marked: animation $13, the
; next state.
; HOW: RichterObjectHitAndMove; RichterObjectSetAnimation, SetVelocityXFromTable,
; NegateVelocityXIfFacingLeft, RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind4Landed:
    ldx z:zRichterObject
    jsr RichterObjectHitAndMove
    dec a:OBJ_HURT_TIMER,x
    beq RichterKind4Landed_at0
    rts
RichterKind4Landed_at0:
    lda a:OBJ_WORK_D,x
    bne RichterKind4Landed_at1
    lda #$12
    jsr RichterObjectSetAnimation
    lda #$06
    jsr SetVelocityXFromTable
    jsr NegateVelocityXIfFacingLeft
    lda a:OBJ_X_LO,x
    sta a:OBJ_WORK_F,x
    lda a:OBJ_X_HI,x
    sta a:OBJ_WORK_G,x
    lda a:OBJ_X_FRAC,x
    sta a:OBJ_WORK_H,x
    lda #$04
    sta a:OBJ_STATE,x
    rts
RichterKind4Landed_at1:
    lda #$13
    jsr RichterObjectSetAnimation
    jmp RichterObjectNextState

; @routine 01BE:492 RichterKind4Splash
; @run 6E:CC92
; WHAT IT DOES: Kind 4 after a marked landing: hitbox and move until the animation ends, then emptied.
; HOW: RichterObjectHitAndMove, RichterObjectAnimationWait, ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectAnimationWait.
RichterKind4Splash:
    ldx z:zRichterObject
    jsr RichterObjectHitAndMove
    jsr RichterObjectAnimationWait
    jmp VecClearObject

; @routine 01BE:49D RichterKind4Burn
; @run 6E:CC9D
; WHAT IT DOES: Kind 4 (and the flames, kind $0A) burning along the floor: emptied when its animation ends;
; the flames spread; hitbox and move; its angle nudged; a floor 8 below with bit 4 sets Y speed entry 8,
; otherwise it is raised; a wall at the side by facing (the words at $CE0F, bits 1 and 5) stops it; no floor
; 8 below, the next state.
; HOW: RichterObjectAnimationRunning; RichterKind4Spawn; RichterObjectHitAndMove;
; NudgeAngleByVelocityAndScroll; RichterObjectProbeBelow, SetVelocityYFromTable, MoveObjectUpBySigned;
; RichterObjectProbe; tail jumps to RichterObjectStopX and RichterObjectNextState; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind4Burn:
    ldx z:zRichterObject
    jsr RichterObjectAnimationRunning
    bne RichterKind4Burn_at0
    jmp VecClearObject
RichterKind4Burn_at0:
    jsr RichterKind4Spawn
    jsr RichterObjectHitAndMove
    jsr NudgeAngleByVelocityAndScroll
    lda #$08
    jsr RichterObjectProbeBelow
    beq RichterKind4Burn_at1
    and #$10
    beq RichterKind4Burn_at2
    lda #$08
    jsr SetVelocityYFromTable
    bra RichterKind4Burn_at3
RichterKind4Burn_at2:
    jsr MoveObjectUpBySigned
RichterKind4Burn_at3:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:RichterKind4WallReach,y
    sta z:zTemp04
    lda a:RichterKind4WallReach+1,y
    sta z:zTemp05
    stz z:zTemp06
    stz z:zTemp07
    jsr RichterObjectProbe
    and #$22
    bne RichterKind4Burn_at4
    rts
RichterKind4Burn_at4:
    jmp RichterObjectStopX
RichterKind4Burn_at1:
    jmp RichterObjectNextState

; @routine 01BE:531 RichterKind4Drop
; @run 6E:CD31
; WHAT IT DOES: A burning kind 4 or flame off the floor's edge: emptied when its animation ends; spread,
; hitbox and move, angle nudged; a floor 8 below raises it, stops it falling and goes back a state; else it
; falls at $28 a frame to $0700.
; HOW: RichterObjectAnimationRunning, RichterKind4Spawn, RichterObjectHitAndMove,
; NudgeAngleByVelocityAndScroll, RichterObjectProbe, MoveObjectUpBySigned, RichterObjectStopY; a jump to
; RichterObjectFallCapped; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind4Drop:
    ldx z:zRichterObject
    jsr RichterObjectAnimationRunning
    bne RichterKind4Drop_at0
    jmp VecClearObject
RichterKind4Drop_at0:
    jsr RichterKind4Spawn
    jsr RichterObjectHitAndMove
    jsr NudgeAngleByVelocityAndScroll
    lda #$08
    sta z:zTemp06
    stz z:zTemp07
    stz z:zTemp04
    stz z:zTemp05
    jsr RichterObjectProbe
    and #$31
    beq RichterKind4Drop_at1
    jsr MoveObjectUpBySigned
    stz a:OBJ_Y_FRAC,x
    jsr RichterObjectStopY
    dec a:OBJ_STATE,x
    rts
RichterKind4Drop_at1:
    lda #$28
    jmp RichterObjectFallCapped

; @routine 01BE:58F RichterKind4Spawn
; @run 6E:CD8F
; WHAT IT DOES: At its animation's step $0B, the next slot copies this one as a flame (kind $0A, state 0,
; animation $12, its count one more), up to two after the first.
; HOW: The copy of the fields; RichterObjectSetAnimation on the new slot with X pushed.
; WAITS: no.
; Entry: X the slot. Exit: as its last instruction or call left them; Y the next slot.
RichterKind4Spawn:
    txa
    tay
    iny
    lda a:OBJ_ANIMATION_AT,x
    cmp #$0B
    beq RichterKind4Spawn_at0
    rts
RichterKind4Spawn_at0:
    lda a:OBJ_WORK_E,x
    cmp #$02
    bne RichterKind4Spawn_at1
    rts
RichterKind4Spawn_at1:
    inc a
    sta a:OBJ_WORK_E,y
    lda a:OBJ_SPRITE_FLIP,x
    sta a:OBJ_SPRITE_FLIP,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda a:OBJ_WORK_F,x
    sta a:OBJ_X_LO,y
    sta a:OBJ_WORK_F,y
    lda a:OBJ_WORK_G,x
    sta a:OBJ_X_HI,y
    sta a:OBJ_WORK_G,y
    lda a:OBJ_X_FRAC,x
    sta a:OBJ_X_FRAC,y
    sta a:OBJ_WORK_H,y
    lda a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_HI,x
    sta a:OBJ_Y_HI,y
    lda a:OBJ_VX_HI,x
    sta a:OBJ_VX_HI,y
    lda a:OBJ_VX_LO,x
    sta a:OBJ_VX_LO,y
    lda a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,y
    lda a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,y
    lda a:OBJ_SPRITE_SET,x
    sta a:OBJ_SPRITE_SET,y
    lda a:OBJ_WORK_N,x
    sta a:OBJ_WORK_N,y
    phx
    tya
    tax
    lda #$12
    jsr RichterObjectSetAnimation
    lda #$0A
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    plx
    rts

; @routine 01BE:613 RichterKind10Frame
; @run 6E:CE13
; WHAT IT DOES: Kind $0A, a flame of kind 4: runs its state's handler, kind 4's burning and dropping.
; HOW: RichterStateDoubled; the two words at $CE19.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind10Frame:
    jsr RichterStateDoubled
    jmp (RichterKind10States,x)

; @routine 01BE:63D RichterObjectScrollAndMove
; @run 6E:CE3D
; WHAT IT DOES: Moves an object against the scroll (unless the camera is in mode $17), then by its velocity.
; HOW: ScrollSpeedX and ScrollSpeedY negated and added with their signs (zTemp10 the high byte);
; RichterObjectAddXLow and RichterObjectAddXHigh; runs on into RichterObjectMove.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectMove.
RichterObjectScrollAndMove:
    lda a:CameraMode
    cmp #$17
    beq RichterObjectMove
    stz z:zTemp10
    lda a:ScrollSpeedX
    eor #$FF
    inc a
    bpl RichterObjectScrollAndMove_at0
    dec z:zTemp10
RichterObjectScrollAndMove_at0:
    jsr RichterObjectAddXLow
    lda z:zTemp10
    jsr RichterObjectAddXHigh
    stz z:zTemp10
    lda a:ScrollSpeedY
    eor #$FF
    inc a
    bpl RichterObjectScrollAndMove_at1
    dec z:zTemp10
RichterObjectScrollAndMove_at1:
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    lda z:zTemp10
    adc a:OBJ_Y_HI,x
    sta a:OBJ_Y_HI,x

; @routine 01BE:673 RichterObjectMove
; @run 6E:CE73
; WHAT IT DOES: Moves an object by its velocity, Y then X.
; HOW: ApplyVelocityY, then a tail jump to ApplyVelocityX (through their kernel vectors).
; WAITS: no.
; Entry: X object. Exit: as ApplyVelocityX.
RichterObjectMove:
    jsr VecApplyVelocityY
    jmp VecApplyVelocityX

; @routine 01BE:6E7 RichterAngleToObject
; @run 6E:CEE7
; WHAT IT DOES: The angle from object X to object Y: the deltas of their positions into zTemp00-03.
; HOW: Two 16-bit subtracts; a tail jump to AngleFromDelta (through its kernel vector).
; WAITS: no.
; Entry: X and Y the objects. Exit: as AngleFromDelta.
RichterAngleToObject:
    lda a:OBJ_X_LO,y
    sec
    sbc a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:OBJ_X_HI,y
    sbc a:OBJ_X_HI,x
    sta z:zTemp01
    lda a:OBJ_Y_LO,y
    sec
    sbc a:OBJ_Y_LO,x
    sta z:zTemp02
    lda a:OBJ_Y_HI,y
    sbc a:OBJ_Y_HI,x
    sta z:zTemp03
    jmp VecAngleFromDelta

; @routine 01BE:70C RichterKind11Frame
; @run 6E:CF0C
; WHAT IT DOES: Kind $0B: runs its state's handler.
; HOW: RichterStateDoubled; the two words at $CF12.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind11Frame:
    jsr RichterStateDoubled
    jmp (RichterKind11States,x)

; @routine 01BE:716 RichterKind11Start
; @run 6E:CF16
; WHAT IT DOES: Kind $0B's state 0: once the player's animation reaches step 2, marked, his facing, his place
; offset (0, $F8), its angle count up, its angle the byte at $CFFF by slot, hit timer 8, the wobble $40,
; animation 8 started; sound $3D unless it is kind $0D, then sound $3E.
; HOW: RichterObjectMarkThenFromPlayer, RichterObjectAtPlayer, RichterObjectStartAnimation, SoundCommand.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as SoundCommand with $3E.
RichterKind11Start:
    ldx z:zRichterObject
    lda a:OBJ_ANIMATION_AT
    cmp #$02
    beq RichterKind11Start_at0
    rts
RichterKind11Start_at0:
    jsr RichterObjectMarkThenFromPlayer
    lda #$00
    sta z:zTemp04
    lda #$F8
    sta z:zTemp06
    jsr RichterObjectAtPlayer
    inc a:OBJ_WORK_G,x
    lda a:RichterKind11Angles,x
    sta a:OBJ_WORK_D,x
    lda #$08
    sta a:OBJ_WORK_N,x
    lda #$40
    sta a:OBJ_WORK_I,x
    lda #$08
    jsr RichterObjectStartAnimation
    lda a:OBJ_KIND,x
    cmp #$0D
    beq RichterKind11Start_at1
    lda #$3D
    jsr VecSoundCommand
RichterKind11Start_at1:
    lda #$3E
    jmp VecSoundCommand

; @routine 01BE:76F RichterKind11Fly
; @run 6E:CF6F
; WHAT IT DOES: Kind $0B in flight: its angle wobbles by the byte at $CF57 (facing plus OBJ_WORK_J); while the
; wobble count runs it is unmarked, and every sixteenth frame of it script 1 (3 for kind $0C) on slot 3; at
; its end the speed grows by $70 (held at $0F00), OBJ_WORK_J the byte at $CF5F by it; emptied below the
; map's second screen or off the screen's sides; else as RichterKind11Move.
; HOW: RichterObjectMarkOff; StartSoleScriptSlot with X pushed; ClearObject; runs on into RichterKind11Move.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind11Fly:
    ldx z:zRichterObject
    lda a:OBJ_FACING_LEFT,x
    clc
    adc a:OBJ_WORK_J,x
    tay
    lda a:OBJ_WORK_D,x
    clc
    adc a:RichterKind11Wobble,y
    sta a:OBJ_WORK_D,x
    lda a:OBJ_WORK_I,x
    beq RichterKind11Fly_at0
    dec a:OBJ_WORK_I,x
    bne RichterKind11Fly_at1
    jsr RichterObjectMarkOff
    lda a:OBJ_WORK_I,x
    and #$0F
    bne RichterKind11Fly_at0
    phx
    ldy #$01
    lda a:OBJ_KIND,x
    cmp #$0C
    bne RichterKind11Fly_at2
    ldy #$03
RichterKind11Fly_at2:
    tya
    ldx #$03
    jsr VecStartSoleScriptSlot
    plx
RichterKind11Fly_at0:
    lda #$70
    clc
    adc a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,x
    cla
    adc a:OBJ_WORK_G,x
    sta a:OBJ_WORK_G,x
    cmp #$0F
    bne RichterKind11Fly_at3
    stz a:OBJ_WORK_F,x
RichterKind11Fly_at3:
    tay
    lda a:RichterKind11WobbleSteps,y
    sta a:OBJ_WORK_J,x
RichterKind11Fly_at1:
    lda a:OBJ_Y_HI,x
    beq RichterKind11Fly_at4
    bmi RichterKind11Fly_at4
    cmp #$02
    beq RichterKind11Fly_at5
RichterKind11Fly_at4:
    lda a:OBJ_X_HI,x
    beq RichterKind11Move
    lda a:OBJ_X_LO,x
    cmp #$20
    bcc RichterKind11Move
    cmp #$E0
    bcs RichterKind11Move
RichterKind11Fly_at5:
    jmp VecClearObject

; @routine 01BE:7E6 RichterKind11Move
; @run 6E:CFE6
; WHAT IT DOES: Kind $0B on its way: animated, hitbox on, moved with the scroll, then its velocity from its
; angle (OBJ_WORK_D) at the speed in OBJ_WORK_F and G.
; HOW: RichterObjectAnimationTick, RichterObjectHitboxOn, RichterObjectScrollAndMove; zTemp00-01; a tail jump
; to SetVelocityFromAngle (through its kernel vector).
; WAITS: no.
; Entry: X the slot. Exit: as SetVelocityFromAngle.
RichterKind11Move:
    jsr RichterObjectAnimationTick
    jsr RichterObjectHitboxOn
    jsr RichterObjectScrollAndMove
    lda a:OBJ_WORK_F,x
    sta z:zTemp00
    lda a:OBJ_WORK_G,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jmp VecSetVelocityFromAngle

; @routine 01BF:00F RichterKind6CircleStart
; @run 6E:D00F
; WHAT IT DOES: Starts kind 6's circle: speed $04E0, its angle the byte at $D025 by slot, the count $46.
; HOW: Stores.
; WAITS: no.
; Entry: X the slot. Exit: A $46, N and Z from it.
RichterKind6CircleStart:
    lda #$E0
    sta a:OBJ_WORK_F,x
    lda #$04
    sta a:OBJ_WORK_G,x
    lda a:RichterKind6Angles,x
    sta a:OBJ_WORK_D,x
    lda #$46
    sta a:OBJ_ANIM_TIMER,x
    rts

; @routine 01BF:035 RichterKind6CircleStep
; @run 6E:D035
; WHAT IT DOES: A step of kind 6's circle until the count reaches $73: moved, its angle 2 on, its velocity from
; the angle at its speed, the count up.
; HOW: RichterObjectMove; zWaveCounter read; SetVelocityFromAngle (through its kernel vector).
; WAITS: no.
; Entry: X the slot. Exit: at $73, A $73, Z and C set; else N and Z from the count, the rest as
; SetVelocityFromAngle.
RichterKind6CircleStep:
    lda a:OBJ_ANIM_TIMER,x
    cmp #$73
    bne RichterKind6CircleStep_at0
    rts
RichterKind6CircleStep_at0:
    jsr RichterObjectMove
    lda z:zWaveCounter
    lda a:OBJ_WORK_D,x
    clc
    adc #$02
    sta a:OBJ_WORK_D,x
    lda a:OBJ_WORK_F,x
    sta z:zTemp00
    lda a:OBJ_WORK_G,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    inc a:OBJ_ANIM_TIMER,x
    rts

; @routine 01BF:05F RichterKind12Frame
; @run 6E:D05F
; WHAT IT DOES: Kind $0C: runs its state's handler: its start, then kind 5's flight, stuck and fall.
; HOW: RichterStateDoubled; the four words at $D065.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind12Frame:
    jsr RichterStateDoubled
    jmp (RichterKind12States,x)

; @routine 01BF:06D RichterKind12Start
; @run 6E:D06D
; WHAT IT DOES: Kind $0C's state 0: thrown as kind 5, then moved down by the word at $D0A4 by its place among
; three, his facing, and across by the word at $D0A0 by his facing.
; HOW: RichterKind5Throw; RichterObjectFromPlayer, RichterObjectAddXLow and RichterObjectAddXHigh.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectAddXHigh, Y the offset.
RichterKind12Start:
    ldx z:zRichterObject
    jsr RichterKind5Throw
    txa
    sec
    sbc #$05
    asl a
    tay
    lda a:RichterKind12Drops,y
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    lda a:RichterKind12Drops+1,y
    adc a:OBJ_Y_HI,x
    sta a:OBJ_Y_HI,x
    jsr RichterObjectFromPlayer
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:RichterKind12Across,y
    jsr RichterObjectAddXLow
    lda a:RichterKind12Across+1,y
    jsr RichterObjectAddXHigh
    rts

; @routine 01BF:0BA RichterKind8Frame
; @run 6E:D0BA
; WHAT IT DOES: Kind 8: in flight, as RichterKind8Fly. At state 0: emptied while the player is hurt; once
; RichterFlagsAgainstFour lets it, the lead (slot 5) makes slots 6 to 15 kind 8; each takes his facing, his
; place offset (0, $E8), speed $0100 at angle $A4, hit timer $0A, its delay and count the byte at $D133 by
; slot, marked, animation 6 started, hitbox on; the lead then unmarked, sound 9, sprite set 3, animation $19.
; HOW: RichterFlagsAgainstFour; RichterObjectFromPlayer, RichterObjectAtPlayer, RichterObjectMarkOn,
; RichterObjectStartAnimation, RichterObjectHitboxOn; RichterObjectMarkOff, SoundCommand, a tail jump to
; RichterObjectSetAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind8Frame:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    beq RichterKind8Frame_at0
    jmp RichterKind8Fly
RichterKind8Frame_at0:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    bne RichterKind8Frame_at1
    jmp VecClearObject
RichterKind8Frame_at1:
    jsr RichterFlagsAgainstFour
    beq RichterKind8Frame_at2
    rts
RichterKind8Frame_at2:
    cpx #$05
    bne RichterKind8Frame_at3
    ldy #$06
    lda #$08
RichterKind8Frame_at4:
    sta a:OBJ_KIND,y
    iny
    cpy #$10
    bne RichterKind8Frame_at4
RichterKind8Frame_at3:
    jsr RichterObjectFromPlayer
    lda #$00
    sta z:zTemp04
    lda #$E8
    sta z:zTemp06
    jsr RichterObjectAtPlayer
    lda #$00
    sta a:OBJ_WORK_F,x
    lda #$01
    sta a:OBJ_WORK_G,x
    lda #$A4
    sta a:OBJ_WORK_D,x
    lda #$0A
    sta a:OBJ_WORK_N,x
    lda a:RichterKind8Delays,x
    sta a:OBJ_WORK_I,x
    sta a:OBJ_WORK_J,x
    jsr RichterObjectMarkOn
    lda #$06
    jsr RichterObjectStartAnimation
    jsr RichterObjectHitboxOn
    cpx #$05
    beq RichterKind8Frame_at5
    rts
RichterKind8Frame_at5:
    jsr RichterObjectMarkOff
    lda #$09
    jsr VecSoundCommand
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$19
    jmp RichterObjectSetAnimation

; @routine 01BF:143 RichterKind8Fly
; @run 6E:D143
; WHAT IT DOES: Kind 8, a chain led by slot 5: each waits out its delay moving with the scroll; the lead
; wobbles its angle (the byte at $CF57 by facing), grows its speed by $18, moves as kind $0B, and records its
; velocity in the path at $D1DE by its count; emptied below the map's second screen or off $60-$9F of a
; side screen. A tail link, while the one before it is out, takes the lead's velocity from the path its own
; delay back and moves with the scroll; else it is emptied.
; HOW: OBJ_WORK_I the delay, OBJ_WORK_J the count; RichterKind11Move; RichterObjectScrollAndMove; ClearObject.
; WAITS: no.
; Entry: X the slot. Exit: as its last step left them.
RichterKind8Fly:
    lda a:OBJ_WORK_I,x
    beq RichterKind8Fly_at0
    dec a:OBJ_WORK_I,x
    jmp RichterObjectScrollAndMove
RichterKind8Fly_at0:
    cpx #$05
    bne RichterKind8Fly_at1
    ldy a:OBJ_FACING_LEFT,x
    lda a:OBJ_WORK_D,x
    clc
    adc a:RichterKind11Wobble,y
    sta a:OBJ_WORK_D,x
    lda #$18
    clc
    adc a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,x
    cla
    adc a:OBJ_WORK_G,x
    sta a:OBJ_WORK_G,x
    jsr RichterKind11Move
    lda a:OBJ_WORK_J,x
    asl a
    asl a
    tay
    lda a:OBJ_VX_LO,x
    sta a:RichterKind8Path,y
    lda a:OBJ_VX_HI,x
    sta a:RichterKind8Path+1,y
    lda a:OBJ_VY_LO,x
    sta a:RichterKind8Path+2,y
    lda a:OBJ_VY_HI,x
    sta a:RichterKind8Path+3,y
    inc a:OBJ_WORK_J,x
    lda a:OBJ_Y_HI,x
    cmp #$02
    beq RichterKind8Fly_at2
    lda a:OBJ_X_HI,x
    beq RichterKind8Fly_at3
    lda a:OBJ_X_LO,x
    cmp #$60
    bcc RichterKind8Fly_at3
    cmp #$A0
    bcs RichterKind8Fly_at3
    beq RichterKind8Fly_at2
RichterKind8Fly_at2:
    jmp VecClearObject
RichterKind8Fly_at3:
    rts
RichterKind8Fly_at1:
    lda a:OBJ_KIND-1,x
    bne RichterKind8Fly_at4
    jmp VecClearObject
RichterKind8Fly_at4:
    lda a:OBJ_WORK_J+5
    sec
    sbc a:OBJ_WORK_J,x
    asl a
    asl a
    tay
    lda a:RichterKind8Path,y
    sta a:OBJ_VX_LO,x
    lda a:RichterKind8Path+1,y
    sta a:OBJ_VX_HI,x
    lda a:RichterKind8Path+2,y
    sta a:OBJ_VY_LO,x
    lda a:RichterKind8Path+3,y
    sta a:OBJ_VY_HI,x
    jsr RichterObjectScrollAndMove
    rts

; @routine 01BF:2DE RichterKind13Frame
; @run 6E:D2DE
; WHAT IT DOES: Kind $0D: runs its state's handler.
; HOW: RichterStateDoubled; the four words at $D2E4.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind13Frame:
    jsr RichterStateDoubled
    jmp (RichterKind13States,x)

; @routine 01BF:2EC RichterKind13Start
; @run 6E:D2EC
; WHAT IT DOES: Kind $0D's state 0 as kind $0B's; once started, speed $0160, the wobble $3C, animation $17.
; HOW: RichterKind11Start; a tail jump to RichterObjectSetAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step left them.
RichterKind13Start:
    jsr RichterKind11Start
    lda a:OBJ_STATE,x
    bne RichterKind13Start_at0
    rts
RichterKind13Start_at0:
    lda #$60
    sta a:OBJ_WORK_F,x
    lda #$01
    sta a:OBJ_WORK_G,x
    lda #$3C
    sta a:OBJ_WORK_I,x
    lda #$17
    jmp RichterObjectSetAnimation

; @routine 01BF:309 RichterKind13Fly
; @run 6E:D309
; WHAT IT DOES: Kind $0D in flight as kind $0B; when its wobble runs out, the next state, and all but the lead
; (slot 5) are emptied.
; HOW: RichterKind11Fly; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step left them.
RichterKind13Fly:
    jsr RichterKind11Fly
    lda a:OBJ_WORK_I,x
    bne RichterKind13Fly_at0
    inc a:OBJ_STATE,x
    cpx #$05
    beq RichterKind13Fly_at0
    jmp VecClearObject
RichterKind13Fly_at0:
    rts

; @routine 01BF:31C RichterKind13Hold
; @run 6E:D31C
; WHAT IT DOES: Kind $0D held by the player while he attacks (step 1): hit timer $0D, sprite set 1, his facing,
; animation $15, the next state, at his place, its pieces placed.
; HOW: RichterObjectFromPlayer, RichterObjectSetAnimation, RichterKind13AtPlayer; a jump to RichterKind13Place.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step left them.
RichterKind13Hold:
    ldx z:zRichterObject
    lda a:OBJ_SUBSTATE
    cmp #$01
    beq RichterKind13Hold_at0
    rts
RichterKind13Hold_at0:
    lda #$0D
    sta a:OBJ_WORK_N,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    jsr RichterObjectFromPlayer
    ldy a:OBJ_STATE
    lda #$15
    jsr RichterObjectSetAnimation
    inc a:OBJ_STATE,x
    jsr RichterKind13AtPlayer
    jmp RichterKind13Place

; @routine 01BF:344 RichterKind13AtPlayer
; @run 6E:D344
; WHAT IT DOES: Kind $0D at the player's place, moved further across by facing in his states $0B and $0C
; (the words at $D392 and $D396).
; HOW: RichterObjectAtPlayerPlace; RichterObjectAddXLow and RichterObjectAddXHigh.
; WAITS: no.
; Entry: X the slot. Exit: as the last step left them.
RichterKind13AtPlayer:
    jsr RichterObjectAtPlayerPlace
    lda a:OBJ_STATE
    cmp #$0B
    beq RichterKind13AtPlayer_at0
    cmp #$0C
    beq RichterKind13AtPlayer_at1
    rts
RichterKind13AtPlayer_at0:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:RichterKind13ReachA,y
    jsr RichterObjectAddXLow
    lda a:RichterKind13ReachA+1,y
    jsr RichterObjectAddXHigh
    rts
RichterKind13AtPlayer_at1:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:RichterKind13ReachB,y
    jsr RichterObjectAddXLow
    lda a:RichterKind13ReachB+1,y
    jsr RichterObjectAddXHigh
    rts

; @routine 01BF:377 RichterKind13Swing
; @run 6E:D377
; WHAT IT DOES: Kind $0D swinging: his facing, at his place; its pieces placed while the animation runs; at its
; end slots from this one to $0E are emptied.
; HOW: RichterObjectFromPlayer, RichterKind13AtPlayer, RichterObjectAnimationRunning, RichterKind13Place;
; ClearObject with X pushed.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step left them.
RichterKind13Swing:
    ldx z:zRichterObject
    jsr RichterObjectFromPlayer
    jsr RichterKind13AtPlayer
    jsr RichterObjectAnimationRunning
    beq RichterKind13Swing_at0
    jmp RichterKind13Place
RichterKind13Swing_at0:
    phx
RichterKind13Swing_at1:
    jsr VecClearObject
    inx
    cpx #$0F
    bne RichterKind13Swing_at1
    plx
    rts

; @routine 01BF:39A RichterKind13Place
; @run 6E:D39A
; WHAT IT DOES: Places kind $0D's pieces by its frame: the list at $DD48 by twice the frame less $6E.
; HOW: zTemp00-01; RichterKind13Pieces with X pushed.
; WAITS: no.
; Entry: X the slot. Exit: X the slot, N and Z from it; the rest as RichterKind13Pieces left them.
RichterKind13Place:
    lda a:OBJ_FRAME,x
    sec
    sbc #$6E
    asl a
    tay
    lda a:RichterKind13Lists,y
    sta z:zTemp00
    lda a:RichterKind13Lists+1,y
    sta z:zTemp01
    phx
    jsr RichterKind13Pieces
    plx
    rts

; @routine 01BF:3B2 RichterKind13Pieces
; @run 6E:D3B2
; WHAT IT DOES: Places kind $0D's six pieces (slots 6 to 11) round slot 5 by the list at zTemp00-01: a count,
; then for each its Y offset, its X offset (taken from the lead's X when he faces left), and its hitbox;
; each with hit timer $0D and its hitbox on.
; HOW: Reads through the zero page pointer; RichterObjectHitboxFrom with Y pushed.
; WAITS: no.
; Entry: zTemp00-01 the list. Exit: X one past the last piece, N and Z from the count's DEC; the rest as the
; last hitbox left them.
RichterKind13Pieces:
    ldx #$06
    cly
    lda (zTemp00),y
    sta z:zTemp03
    iny
RichterKind13Pieces_at2:
    lda (zTemp00),y
    iny
    clc
    adc a:OBJ_Y_LO+5
    sta a:OBJ_Y_LO,x
    lda (zTemp00),y
    adc a:OBJ_Y_HI+5
    sta a:OBJ_Y_HI,x
    iny
    lda a:OBJ_FACING_LEFT+5
    bne RichterKind13Pieces_at0
    lda (zTemp00),y
    iny
    clc
    adc a:OBJ_X_LO+5
    sta a:OBJ_X_LO,x
    lda (zTemp00),y
    adc a:OBJ_X_HI+5
    sta a:OBJ_X_HI,x
    iny
    bra RichterKind13Pieces_at1
RichterKind13Pieces_at0:
    lda (zTemp00),y
    sta z:zTemp04
    iny
    lda (zTemp00),y
    sta z:zTemp05
    iny
    lda a:OBJ_X_LO+5
    sec
    sbc z:zTemp04
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI+5
    sbc z:zTemp05
    sta a:OBJ_X_HI,x
RichterKind13Pieces_at1:
    lda (zTemp00),y
    iny
    phy
    jsr RichterObjectHitboxFrom
    ply
    lda #$0D
    sta a:OBJ_WORK_N,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inx
    dec z:zTemp03
    bne RichterKind13Pieces_at2
    rts

; @routine 01BF:41A RichterKind14Frame
; @run 6E:D41A
; WHAT IT DOES: Kind $0E: at state 0, script 2 on slot 3 until the player's animation reaches step 2; then
; $A0 frames, sound $33, the raster stage kept (PlayerKeptRasterStage) and set to 2, the next state. Then,
; when its frames run out, the raster stage given back and it is emptied; on even wave counts, a drop (kind
; $0F) in the slot 6 plus its count when free: its place from the four bytes at $D56D by its other count
; (X wobbling by the wave counter's low bits), sprite set 1, frame $61, the speeds (X entry 4 by facing, Y
; entry 5), hit timer $0B, hitbox on; the counts move on (the second in BCD, both kept below $10).
; HOW: StartSoleScriptSlot with X pushed; SoundCommand; zRasterStage; SetVelocityXFromTable,
; NegateVelocityXIfFacingLeft, SetVelocityYFromTable, RichterObjectHitboxOn with X pushed and swapped;
; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
RichterKind14Frame:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne RichterKind14Frame_at0
    lda a:OBJ_ANIMATION_AT
    cmp #$02
    beq RichterKind14Frame_at1
    phx
    ldx #$03
    lda #$02
    jsr VecStartSoleScriptSlot
    plx
    rts
RichterKind14Frame_at1:
    lda #$A0
    sta a:OBJ_HURT_TIMER,x
    lda #$33
    jsr VecSoundCommand
    lda z:zRasterStage
    sta a:PlayerKeptRasterStage
    lda #$02
    sta z:zRasterStage
    jmp RichterObjectNextState
RichterKind14Frame_at0:
    dec a:OBJ_HURT_TIMER,x
    bne RichterKind14Frame_at2
    lda a:PlayerKeptRasterStage
    sta z:zRasterStage
    jmp VecClearObject
RichterKind14Frame_at2:
    lda z:zWaveCounter
    and #$01
    beq RichterKind14Frame_at3
    rts
RichterKind14Frame_at3:
    lda a:OBJ_WORK_D,x
    clc
    adc #$06
    tay
    lda a:OBJ_KIND,y
    beq RichterKind14Frame_at4
    rts
RichterKind14Frame_at4:
    lda #$0F
    sta a:OBJ_KIND,y
    lda a:OBJ_WORK_E,x
    phx
    sxy
    asl a
    asl a
    tay
    lda a:RichterKind14Drops,y
    sta z:zTemp00
    lda a:RichterKind14Drops+1,y
    sta z:zTemp01
    lda a:RichterKind14Drops+2,y
    sta z:zTemp02
    lda a:RichterKind14Drops+3,y
    sta z:zTemp03
    lda z:zTemp00
    sta a:OBJ_Y_LO,x
    lda z:zTemp01
    sta a:OBJ_Y_HI,x
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp02
    sta a:OBJ_X_LO,x
    lda z:zTemp03
    sta a:OBJ_X_HI,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$61
    sta a:OBJ_FRAME,x
    lda #$04
    jsr SetVelocityXFromTable
    jsr NegateVelocityXIfFacingLeft
    lda #$05
    jsr SetVelocityYFromTable
    lda #$0B
    sta a:OBJ_WORK_N,x
    jsr RichterObjectHitboxOn
    plx
    lda a:OBJ_WORK_E,x
    inc a
    and #$0F
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sed
    clc
    adc #$01
    cld
    and #$0F
    sta a:OBJ_WORK_D,x
    rts

; @routine 01BF:4DA RichterKind15Frame
; @run 6E:D4DA
; WHAT IT DOES: Kind $0F, a drop of kind $0E: its frame by its slot's bit 0 and the wave counter (the four
; bytes at $D569), marked, moved with the scroll; struck, a shot; emptied below the map; its hitbox on only
; on every eighth wave count.
; HOW: RichterObjectMarkOn, RichterObjectScrollAndMove, RichterKind15Shot; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction or call left them.
RichterKind15Frame:
    ldx z:zRichterObject
    txa
    and #$01
    eor z:zWaveCounter
    and #$03
    tay
    lda a:RichterKind15Frames,y
    sta a:OBJ_FRAME,x
    jsr RichterObjectMarkOn
    jsr RichterObjectScrollAndMove
    lda a:OBJ_HIT_BY_SLOT,x
    beq RichterKind15Frame_at0
    jsr RichterKind15Shot
RichterKind15Frame_at0:
    lda a:OBJ_Y_HI,x
    cmp #$01
    beq RichterKind15Frame_at1
    stz a:OBJ_HIT_BY_SLOT,x
    stz a:OBJ_HITBOX_ON,x
    tst #$07,z:zWaveCounter
    bne RichterKind15Frame_at2
    inc a:OBJ_HITBOX_ON,x
RichterKind15Frame_at2:
    rts
RichterKind15Frame_at1:
    jmp VecClearObject

; @routine 01BF:511 RichterKind15Shot
; @run 6E:D511
; WHAT IT DOES: When a drop is struck by an object whose OBJ_HITBOX_ON is 2 or more, a kind $9C object in the
; first free slot from $30 to $3F at that object's place, $10 up and wobbling across by the wave counter.
; HOW: The point in zTemp00-03; ClearObject, RichterObjectToPoint; X pushed.
; WAITS: no.
; Entry: A the striking object's slot, X the drop. Exit: X the drop, N and Z from it; the rest as the last step
; left them.
RichterKind15Shot:
    phx
    tax
    lda a:OBJ_HITBOX_ON,x
    cmp #$02
    bcc RichterKind15Shot_at0
    lda a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    sta z:zTemp01
    stz z:zTemp04
    lda z:zWaveCounter
    lsr a
    lsr a
    lsr a
    and #$1F
    sec
    sbc #$10
    bpl RichterKind15Shot_at1
    dec z:zTemp04
RichterKind15Shot_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    lda z:zTemp01
    adc z:zTemp04
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    clc
    adc #$F0
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    adc #$FF
    sta z:zTemp03
    ldx #$30
RichterKind15Shot_at3:
    lda a:OBJ_KIND,x
    beq RichterKind15Shot_at2
    inx
    cpx #$40
    bne RichterKind15Shot_at3
RichterKind15Shot_at0:
    plx
    rts
RichterKind15Shot_at2:
    jsr VecClearObject
    lda #$9C
    sta a:OBJ_KIND,x
    jsr RichterObjectToPoint
    plx
    rts

; @routine 01BF:5AD RichterKind16Frame
; @run 6E:D5AD
; WHAT IT DOES: Kind $10: runs its state's handler, some of them kind 6's.
; HOW: RichterStateDoubled; the seven words at $D5B3.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind16Frame:
    jsr RichterStateDoubled
    jmp (RichterKind16States,x)

; @routine 01BF:5C1 RichterKind16Start
; @run 6E:D5C1
; WHAT IT DOES: Kind $10's state 0: the first (slot 4) sounds $5F, sets TimeSlowMode 2 and zFadeCount 1, and
; makes slots 4 to 15 kind $10; each takes its place as kind 6's does, sprite attribute $85, its circling
; start, animation $16 started.
; HOW: SoundCommand; RichterKind6Places; RichterKind6CircleStart; a jump to RichterObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectStartAnimation.
RichterKind16Start:
    ldx z:zRichterObject
    txa
    sec
    sbc #$04
    asl a
    asl a
    cmp #$04
    bne RichterKind16Start_at0
    lda #$5F
    jsr VecSoundCommand
    lda #$02
    sta a:TimeSlowMode
    lda #$01
    sta z:zFadeCount
    ldy #$04
    lda #$10
RichterKind16Start_at1:
    sta a:OBJ_KIND,y
    iny
    cpy #$10
    bne RichterKind16Start_at1
    lda #$04
RichterKind16Start_at0:
    tay
    lda a:RichterKind6Places,y
    sta a:OBJ_X_LO,x
    lda a:RichterKind6Places+1,y
    sta a:OBJ_X_HI,x
    lda a:RichterKind6Places+2,y
    sta a:OBJ_Y_LO,x
    lda a:RichterKind6Places+3,y
    sta a:OBJ_Y_HI,x
    lda #$85
    sta a:OBJ_SPRITE_ATTR,x
    jsr RichterKind6CircleStart
    lda #$16
    jmp RichterObjectStartAnimation

; @routine 01BF:60F RichterKind16Aim
; @run 6E:D60F
; WHAT IT DOES: Kind $10 waiting out its two timers as kind 6; then animation $16 and, with a target, flies at
; it (speed $0380 at the angle to it), $70 frames, hit timer $0C, hitbox on; with none, animation $0F and the
; timer cleared; the next state.
; HOW: RichterObjectSetAnimation, RichterFindTarget, RichterAngleToObject, SetVelocityFromAngle with A pushed,
; RichterObjectHitboxOn, RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind16Aim:
    ldx z:zRichterObject
    dec a:OBJ_HURT_TIMER,x
    beq RichterKind16Aim_at0
    rts
RichterKind16Aim_at0:
    lda a:OBJ_ANIM_TIMER,x
    beq RichterKind16Aim_at1
    dec a:OBJ_ANIM_TIMER,x
    rts
RichterKind16Aim_at1:
    lda #$16
    jsr RichterObjectSetAnimation
    jsr RichterFindTarget
    bcc RichterKind16Aim_at2
    jsr RichterAngleToObject
    pha
    lda #$80
    sta z:zTemp00
    lda #$03
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    lda #$70
    sta a:OBJ_ANIM_TIMER,x
    lda #$0C
    sta a:OBJ_WORK_N,x
    jsr RichterObjectHitboxOn
    jmp RichterObjectNextState
RichterKind16Aim_at2:
    lda #$0F
    jsr RichterObjectSetAnimation
    stz a:OBJ_ANIM_TIMER,x
    jmp RichterObjectNextState

; @routine 01BF:655 RichterKind16Fly
; @run 6E:D655
; WHAT IT DOES: Kind $10 flying at its target: when its animation ends, hitbox and attribute off, the next
; state; while its timer runs, moved, turned back both ways at $38; at its end stopped, hitbox off, animation
; $0F.
; HOW: RichterObjectAnimationRunning; NegateVelocityX ($D869) and RichterObjectNegateVelocityY; a jump to
; RichterObjectMove; RichterObjectStop.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind16Fly:
    ldx z:zRichterObject
    jsr RichterObjectAnimationRunning
    beq RichterKind16Fly_at0
    lda a:OBJ_ANIM_TIMER,x
    beq RichterKind16Fly_at1
    cmp #$38
    bne RichterKind16Fly_at2
    jsr NegateVelocityXIfFacingLeft_at0
    jsr RichterObjectNegateVelocityY
RichterKind16Fly_at2:
    dec a:OBJ_ANIM_TIMER,x
    jmp RichterObjectMove
RichterKind16Fly_at1:
    jsr RichterObjectStop
    stz a:OBJ_HITBOX_ON,x
    lda #$0F
    sta a:OBJ_ANIMATION,x
    rts
RichterKind16Fly_at0:
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_SPRITE_ATTR,x
    jmp RichterObjectNextState

; @routine 01BF:686 RichterFindTarget
; @run 6E:D686
; WHAT IT DOES: Finds the first enemy (slots $10 to $57) to aim at: OBJ_HITBOX_ON 2, its health's bit 7
; clear, on the first screen across and down.
; HOW: A scan by Y.
; WAITS: no.
; Entry: nothing. Exit: found, C set, Y the slot; else C clear, Y $58; A and N, Z as the last test left them.
RichterFindTarget:
    ldy #$10
RichterFindTarget_at1:
    lda a:OBJ_HITBOX_ON,y
    cmp #$02
    bne RichterFindTarget_at0
    lda a:OBJ_HEALTH,y
    bmi RichterFindTarget_at0
    lda a:OBJ_X_HI,y
    ora a:OBJ_Y_HI,y
    bne RichterFindTarget_at0
    sec
    rts
RichterFindTarget_at0:
    iny
    cpy #$58
    bne RichterFindTarget_at1
    clc
    rts

; @routine 01BF:6A5 RichterKind17Frame
; @run 6E:D6A5
; WHAT IT DOES: Kind $11, held by the player at his steps 2 and 3: otherwise a kind 3 companion is made sure
; of and it is emptied. At state 0, once his animation reaches step 2, frame $7A, sprite set 1, his facing and
; place, the next state; then at his place, its sprite attribute by bit 0 of the wave counter (the two bytes
; at $D6EA).
; HOW: RichterClaimKind3Slot, ClearObject; RichterObjectFromPlayer, RichterObjectAtPlayerPlace,
; RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind17Frame:
    ldx z:zRichterObject
    lda a:OBJ_SUBSTATE
    cmp #$02
    beq RichterKind17Frame_at0
    lda a:OBJ_SUBSTATE
    cmp #$03
    beq RichterKind17Frame_at0
    jsr RichterClaimKind3Slot
    jmp VecClearObject
RichterKind17Frame_at0:
    lda a:OBJ_STATE,x
    bne RichterKind17Frame_at1
    lda a:OBJ_ANIMATION_AT
    cmp #$02
    beq RichterKind17Frame_at2
    rts
RichterKind17Frame_at2:
    lda #$7A
    sta a:OBJ_FRAME,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    jsr RichterObjectFromPlayer
    jsr RichterObjectAtPlayerPlace
    jmp RichterObjectNextState
RichterKind17Frame_at1:
    jsr RichterObjectAtPlayerPlace
    lda z:zWaveCounter
    and #$01
    tay
    lda a:RichterKind17Attributes,y
    sta a:OBJ_SPRITE_ATTR,x
    rts

; @routine 01BF:6EC RichterKind18Frame
; @run 6E:D6EC
; WHAT IT DOES: Kind $12: runs its state's handler.
; HOW: RichterStateDoubled; the seven words at $D6F2.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind18Frame:
    jsr RichterStateDoubled
    jmp (RichterKind18States,x)

; @routine 01BF:700 RichterKind18Start
; @run 6E:D700
; WHAT IT DOES: Kind $12's state 0: once the player's animation reaches step 2, his facing and place, then
; flies at the screen's point ($80, $50) at speed $0400, animation $18 started.
; HOW: RichterObjectFromPlayer, RichterObjectAtPlayerPlace; the delta to zTemp00-03; AngleFromDelta,
; SetVelocityFromAngle with A pushed; a jump to RichterObjectStartAnimation.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectStartAnimation.
RichterKind18Start:
    ldx z:zRichterObject
    lda a:OBJ_ANIMATION_AT
    cmp #$02
    beq RichterKind18Start_at0
    rts
RichterKind18Start_at0:
    jsr RichterObjectFromPlayer
    jsr RichterObjectAtPlayerPlace
    lda #$80
    sec
    sbc a:OBJ_X_LO,x
    sta z:zTemp00
    cla
    sbc a:OBJ_X_HI,x
    sta z:zTemp01
    lda #$50
    sec
    sbc a:OBJ_Y_LO,x
    sta z:zTemp02
    cla
    sbc a:OBJ_Y_HI,x
    sta z:zTemp03
    jsr VecAngleFromDelta
    pha
    lda #$00
    sta z:zTemp00
    lda #$04
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    lda #$18
    jmp RichterObjectStartAnimation

; @routine 01BF:741 RichterKind18Fly
; @run 6E:D741
; WHAT IT DOES: Kind $12 flying to the screen's middle: moved; there ($7C-$83 across, $48-$57 down), stopped;
; once its animation ends, script 2 on slot 3, both halves get a hitbox $38 by $18 (crossed), on, hit timer
; $0E, $10 frames, the next state; the second half at its place.
; HOW: RichterObjectMove; RichterObjectStop, RichterObjectAnimationWait; StartSoleScriptSlot with X pushed;
; runs on into RichterKind18Pair.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterKind18Pair, or the last compare.
RichterKind18Fly:
    ldx z:zRichterObject
    jsr RichterObjectMove
    lda a:OBJ_X_LO,x
    cmp #$7C
    bcc RichterKind18Fly_at0
    cmp #$84
    bcs RichterKind18Fly_at0
    lda a:OBJ_Y_LO,x
    cmp #$48
    bcc RichterKind18Fly_at0
    cmp #$58
    bcs RichterKind18Fly_at0
    jsr RichterObjectStop
RichterKind18Fly_at0:
    jsr RichterObjectAnimationWait
    phx
    ldx #$03
    lda #$02
    jsr VecStartSoleScriptSlot
    plx
    lda #$38
    sta a:OBJ_HITBOX_HALF_W,x
    sta a:OBJ_HITBOX_HALF_H+1,x
    lda #$18
    sta a:OBJ_HITBOX_HALF_H,x
    sta a:OBJ_HITBOX_HALF_W+1,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    sta a:OBJ_HITBOX_ON+1,x
    lda #$0E
    sta a:OBJ_WORK_N,x
    sta a:OBJ_WORK_N+1,x
    lda #$10
    sta a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x

; @routine 01BF:793 RichterKind18Pair
; @run 6E:D793
; WHAT IT DOES: The second half of kind $12 (the next slot) at its place.
; HOW: OBJ_X_LO and OBJ_Y_LO copied.
; WAITS: no.
; Entry: X the slot. Exit: A its Y low byte, N and Z from it.
RichterKind18Pair:
    lda a:OBJ_X_LO,x
    sta a:OBJ_X_LO+1,x
    lda a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO+1,x
    rts

; @routine 01BF:7A0 RichterKind18Hold
; @run 6E:D7A0
; WHAT IT DOES: Kind $12 holding in the middle, marked; when its frames run out, Y speed entry 6, $FF frames,
; the next state.
; HOW: RichterKind18Mark, SetVelocityYFromTable, RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind18Hold:
    ldx z:zRichterObject
    jsr RichterKind18Mark
    dec a:OBJ_ANIM_TIMER,x
    beq RichterKind18Hold_at0
    rts
RichterKind18Hold_at0:
    lda #$06
    jsr SetVelocityYFromTable
    lda #$FF
    sta a:OBJ_ANIM_TIMER,x
    jmp RichterObjectNextState

; @routine 01BF:7B8 RichterKind18Rise
; @run 6E:D7B8
; WHAT IT DOES: Kind $12 rising: when its frames run out, as RichterKind18End; moved, its half paired; past $80
; frames left, its Y speed $10 less a frame; off the first screen, back on it with a step of $40 across and
; sound $35 or $36 by bit 7 of the step's sum, paired again; marked.
; HOW: RichterObjectMove, RichterKind18Pair, RichterObjectAddXLow, SoundCommand; runs on into
; RichterKind18Mark.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterKind18Mark or RichterKind18End.
RichterKind18Rise:
    ldx z:zRichterObject
    dec a:OBJ_ANIM_TIMER,x
    beq RichterKind18End
    jsr RichterObjectMove
    jsr RichterKind18Pair
    lda a:OBJ_ANIM_TIMER,x
    cmp #$80
    bcc RichterKind18Rise_at0
    lda #$F0
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,x
    lda #$FF
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
RichterKind18Rise_at0:
    lda a:OBJ_Y_HI,x
    beq RichterKind18Mark
    stz a:OBJ_Y_HI,x
    lda #$40
    jsr RichterObjectAddXLow
    asl a
    rol a
    and #$01
    tay
    lda a:RichterKind18Sounds,y
    jsr VecSoundCommand
    jsr RichterKind18Pair

; @routine 01BF:7F8 RichterKind18Mark
; @run 6E:D7F8
; WHAT IT DOES: Marks kind $12 on.
; HOW: RichterObjectMarkOn.
; WAITS: no.
; Entry: X the slot. Exit: as RichterObjectMarkOn.
RichterKind18Mark:
    jsr RichterObjectMarkOn
    rts

; @routine 01BF:7FC RichterKind18End
; @run 6E:D7FC
; WHAT IT DOES: Kind $12's rise ends: $24 frames, the next state.
; HOW: A jump to RichterObjectNextState.
; WAITS: no.
; Entry: X the slot. Exit: as RichterObjectNextState.
RichterKind18End:
    lda #$24
    sta a:OBJ_ANIM_TIMER,x
    jmp RichterObjectNextState

; @routine 01BF:804 RichterKind18Fade
; @run 6E:D804
; WHAT IT DOES: Kind $12's end begins: $40 rows of points, the count cleared, unmarked, the next state.
; HOW: RichterObjectMarkOff; a jump to RichterObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterObjectNextState.
RichterKind18Fade:
    ldx z:zRichterObject
    lda #$40
    sta a:OBJ_WORK_D,x
    stz a:OBJ_WORK_E,x
    jsr RichterObjectMarkOff
    jmp RichterObjectNextState

; @routine 01BF:814 RichterKind18Points
; @run 6E:D814
; WHAT IT DOES: Kind $12's end: eleven points from $6C00 to the stage overlay's entry at $8009 each frame, its
; OBJ_WORK_E in zTemp0F for each and what the stage's code leaves there kept; when its rows run out, both
; halves emptied.
; HOW: RichterDeathPointStep; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind18Points:
    ldx z:zRichterObject
    lda a:OBJ_WORK_E,x
    sta z:zTemp10
    lda #$6C
    sta z:zTemp09
    lda #$00
    sta z:zTemp08
    ldx #$0B
RichterKind18Points_at0:
    lda z:zTemp10
    sta z:zTemp0F
    jsr RichterDeathPointStep
    bne RichterKind18Points_at0
    lda z:zTemp0F
    ldx z:zRichterObject
    sta a:OBJ_WORK_E,x
    dec a:OBJ_WORK_D,x
    beq RichterKind18Points_at1
    rts
RichterKind18Points_at1:
    inx
    jsr VecClearObject
    dex
    jmp VecClearObject

; @routine 01C0:045 RichterClaimKind3Slot
; @run 6E:D845
; WHAT IT DOES: MariaClaimKind3Slot in Richter's overlay, the same bytes at his address: a companion of kind
; 3 in slot 1 when it is empty; nothing when slot 1 holds kind 3; else slot 2 when it is empty.
; HOW: MariaClaimKind3Slot's body with his JSR's return.
; WAITS: no.
; Entry, exit: as MariaClaimKind3Slot.
RichterClaimKind3Slot:
    phx
    ldx #$01
    lda a:OBJ_KIND,x
    bne RichterClaimKind3Slot_at0
    bra RichterClaimKind3Slot_at1
RichterClaimKind3Slot_at0:
    cmp #$03
    beq RichterClaimKind3Slot_at2
    inx
    lda a:OBJ_KIND,x
    bne RichterClaimKind3Slot_at2
RichterClaimKind3Slot_at1:
    jsr VecClearObject
    lda #$03
    sta a:OBJ_KIND,x
RichterClaimKind3Slot_at2:
    plx
    rts

; @routine 01C0:06C RichterObjectNegateVelocityY
; @run 6E:D86C
; WHAT IT DOES: Turns an object's vertical velocity round.
; HOW: A tail jump to NegateVelocityY (through its kernel vector).
; WAITS: no.
; Entry: X object. Exit: as NegateVelocityY.
RichterObjectNegateVelocityY:
    jmp VecNegateVelocityY

; @routine 01C0:06F RichterKind19Frame
; @run 6E:D86F
; WHAT IT DOES: Kind $13: runs its state's handler.
; HOW: RichterStateDoubled; the three words at $D875.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the handler leaves them.
RichterKind19Frame:
    jsr RichterStateDoubled
    jmp (RichterKind19States,x)

; @routine 01C0:07B RichterKind19Start
; @run 6E:D87B
; WHAT IT DOES: Kind $13's state 0: once the player's animation reaches step 2, frame $12, sprite set 3, his
; facing, the next state, at him.
; HOW: RichterObjectFromPlayer, RichterObjectNextState; runs on into RichterKind19AtPlayer.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as RichterKind19AtPlayer, or the compare.
RichterKind19Start:
    ldx z:zRichterObject
    lda a:OBJ_ANIMATION_AT
    cmp #$02
    bne RichterKind19AtPlayer_at0
    lda #$12
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    jsr RichterObjectFromPlayer
    jsr RichterObjectNextState

; @routine 01C0:094 RichterKind19AtPlayer
; @run 6E:D894
; WHAT IT DOES: Kind $13 at the player, offset ($F2, $FB).
; HOW: RichterObjectAtPlayer.
; WAITS: no.
; Entry: X the slot. Exit: as RichterObjectAtPlayer.
RichterKind19AtPlayer:
    lda #$F2
    sta z:zTemp04
    lda #$FB
    sta z:zTemp06
    jsr RichterObjectAtPlayer
RichterKind19AtPlayer_at0:
    rts

; @routine 01C0:0A0 RichterKind19Hold
; @run 6E:D8A0
; WHAT IT DOES: Kind $13 at the player; once his step is 0, X speed entry 5 by facing, frame 0, the next
; state, and slots 6 to 13 copy its place as links (kind $14) with its facing and sprite flip.
; HOW: RichterKind19AtPlayer, SetVelocityXFromTable, NegateVelocityXIfFacingLeft, RichterObjectNextState,
; ClearObjectInYAndCopyPlace.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind19Hold:
    ldx z:zRichterObject
    jsr RichterKind19AtPlayer
    lda a:OBJ_SUBSTATE
    bne RichterKind19Hold_at0
    lda #$05
    jsr SetVelocityXFromTable
    jsr NegateVelocityXIfFacingLeft
    stz a:OBJ_FRAME,x
    jsr RichterObjectNextState
    ldy #$06
RichterKind19Hold_at1:
    jsr ClearObjectInYAndCopyPlace
    lda #$14
    sta a:OBJ_KIND,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda a:OBJ_SPRITE_FLIP,x
    sta a:OBJ_SPRITE_FLIP,y
    iny
    cpy #$0E
    bne RichterKind19Hold_at1
RichterKind19Hold_at0:
    rts

; @routine 01C0:0D8 RichterKind19Fly
; @run 6E:D8D8
; WHAT IT DOES: Kind $13 flying: up to 8 times, a stop (OBJ_WORK_L) takes its X speed away; moved with the
; scroll; its X speed grows by the word at $D8D4 by facing; emptied once it is $20 into a screen beside.
; HOW: OBJ_WORK_M the stops; RichterObjectScrollAndMove; ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last instruction left them.
RichterKind19Fly:
    ldx z:zRichterObject
    lda a:OBJ_WORK_M,x
    cmp #$08
    bcs RichterKind19Fly_at0
    lda a:OBJ_WORK_L,x
    beq RichterKind19Fly_at0
    stz a:OBJ_WORK_L,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VX_HI,x
    inc a:OBJ_WORK_M,x
RichterKind19Fly_at0:
    jsr RichterObjectScrollAndMove
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:RichterKind19Pulls,y
    clc
    adc a:OBJ_VX_LO,x
    sta a:OBJ_VX_LO,x
    lda a:RichterKind19Pulls+1,y
    adc a:OBJ_VX_HI,x
    sta a:OBJ_VX_HI,x
    lda a:OBJ_X_HI,x
    beq RichterKind19Fly_at1
    bmi RichterKind19Fly_at2
    lda a:OBJ_X_LO,x
    cmp #$20
    bcc RichterKind19Fly_at1
    jmp VecClearObject
RichterKind19Fly_at2:
    lda a:OBJ_X_LO,x
    cmp #$E0
    bcs RichterKind19Fly_at1
    jmp VecClearObject
RichterKind19Fly_at1:
    rts

; @routine 01C0:145 RichterKind20Frame
; @run 6E:D945
; WHAT IT DOES: Kind $14, a link swinging round slot 5: emptied once slot 5 is empty; at state 0 its angle the
; byte at $DA2F by slot, frame $2C, sprite set 1, the next state, the head (slot 6) frame $12, sprite set 3
; and sound 9; hit timer $0A, hitbox on. Then as RichterKind20Swing.
; HOW: RichterObjectNextState, SoundCommand, a tail jump to RichterObjectHitboxOn; RichterKind20Swing.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as its last step left them.
RichterKind20Frame:
    ldx z:zRichterObject
    lda a:OBJ_KIND+5
    bne RichterKind20Frame_at0
    jmp VecClearObject
RichterKind20Frame_at0:
    lda a:OBJ_STATE,x
    bne RichterKind20Swing
    lda a:RichterKind20Angles,x
    sta a:OBJ_WORK_E,x
    lda #$2C
    sta a:OBJ_FRAME,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    jsr RichterObjectNextState
    cpx #$06
    bne RichterKind20Frame_at1
    lda #$12
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$09
    jsr VecSoundCommand
RichterKind20Frame_at1:
    lda #$0A
    sta a:OBJ_WORK_N,x
    jmp RichterObjectHitboxOn

; @routine 01C0:182 RichterKind20Struck
; @run 6E:D982
; WHAT IT DOES: Notes on slot 5's OBJ_WORK_L whether the object that tested this link is striking
; (OBJ_HITBOX_ON 2 or more), clearing OBJ_HIT_BY_SLOT then.
; HOW: Stores.
; WAITS: no.
; Entry: X the link. Exit: Y the tester's slot; A and the flags as the compare or the stores left them.
RichterKind20Struck:
    stz a:OBJ_WORK_L+5
    ldy a:OBJ_HIT_BY_SLOT,x
    lda a:OBJ_HITBOX_ON,y
    cmp #$02
    bcc RichterKind20Struck_at0
    lda #$01
    sta a:OBJ_WORK_L+5
    stz a:OBJ_HIT_BY_SLOT,x
RichterKind20Struck_at0:
    rts

; @routine 01C0:198 RichterKind20Swing
; @run 6E:D998
; WHAT IT DOES: A link of kind $14 swinging round slot 5: the strike noted; its reach (OBJ_WORK_F-G) grows by
; $0100 to $6000, its width (OBJ_WORK_H-I) by $40 to $1800; its angle (OBJ_WORK_E) 4 on; placed at slot 5
; plus the reach by the angle's cosine across and the width by its sine down.
; HOW: RichterKind20Struck; ScaleByCosine and ScaleBySine (through their kernel vectors), the product's high
; byte with its sign.
; WAITS: no.
; Entry: X the link. Exit: A the Y high byte, N and Z from it, C and V from the add; Y as ScaleBySine left it.
RichterKind20Swing:
    bsr RichterKind20Struck
    lda a:OBJ_WORK_G,x
    cmp #$60
    bcs RichterKind20Swing_at0
    lda #$00
    clc
    adc a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,x
    lda #$01
    adc a:OBJ_WORK_G,x
    sta a:OBJ_WORK_G,x
RichterKind20Swing_at0:
    lda a:OBJ_WORK_I,x
    cmp #$18
    bcs RichterKind20Swing_at1
    lda #$40
    clc
    adc a:OBJ_WORK_H,x
    sta a:OBJ_WORK_H,x
    lda #$00
    adc a:OBJ_WORK_I,x
    sta a:OBJ_WORK_I,x
RichterKind20Swing_at1:
    lda #$00
    clc
    adc a:OBJ_WORK_D,x
    sta a:OBJ_WORK_D,x
    lda #$04
    clc
    adc a:OBJ_WORK_E,x
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_H,x
    sta z:zTemp00
    lda a:OBJ_WORK_I,x
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecScaleByCosine
    stz z:zTemp00
    lda z:zProduct2
    bpl RichterKind20Swing_at2
    dec z:zTemp00
RichterKind20Swing_at2:
    lda a:OBJ_X_LO+5
    clc
    adc z:zProduct2
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI+5
    adc z:zTemp00
    sta a:OBJ_X_HI,x
    lda a:OBJ_WORK_F,x
    sta z:zTemp00
    lda a:OBJ_WORK_G,x
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecScaleBySine
    stz z:zTemp00
    lda z:zProduct2
    bpl RichterKind20Swing_at3
    dec z:zTemp00
RichterKind20Swing_at3:
    lda a:OBJ_Y_LO+5
    clc
    adc z:zProduct2
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI+5
    adc z:zTemp00
    sta a:OBJ_Y_HI,x
    rts

; @routine 01C0:23D RichterObjectSlotLast
; @run 6E:DA3D
; WHAT IT DOES: RichterObjectSlot again, kind $15's handler: the same bytes at this address.
; HOW: RichterObjectSlot's body.
; WAITS: no.
; Entry, exit: as RichterObjectSlot.
RichterObjectSlotLast:
    ldx z:zRichterObject
    rts
