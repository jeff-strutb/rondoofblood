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
; WHAT IT DOES: Once, at its animation's step 6 and while PlayerByte3BE1 is set, makes an object's
; frame last 4 longer and Richter's one frame shorter than that.
; HOW: OBJ_35C0 marks it done; OBJ_2F38 the step; OBJ_HURT_TIMER the frame's time.
; WAITS: no.
; Entry: X object. Exit: X and Y kept. Stopped, A what was read last, N and Z from it, or from
; comparing the step with 6 and C with it. Done, A Richter's new time, N and Z from the mark, C
; and V from the add.
RichterObjectQuicken:
    lda a:OBJ_35C0,x
    bne RichterObjectQuicken_at0
    lda a:OBJ_SCROLL_BAND,x
    cmp #$06
    bne RichterObjectQuicken_at0
    lda a:PlayerByte3BE1
    beq RichterObjectQuicken_at0
    lda #$04
    clc
    adc a:OBJ_HURT_TIMER,x
    sta a:OBJ_HURT_TIMER,x
    dec a
    sta a:OBJ_HURT_TIMER
    inc a:OBJ_35C0,x
    rts
RichterObjectQuicken_at0:
    rts

; @routine 01BD:49F RichterObjectFlash
; @run 6E:C49F
; WHAT IT DOES: While an object is charged, shows the flashing form of its frame, if it is one
; of the four that have one, with its OBJ_SPRITE_ATTR for the flash.
; HOW: OBJ_3568 is the charge; frames $07, $0E, $15 and $1C become $8E to $91; RichterFlashFlags
; by OBJ_34B8 less $0E, with bit 7.
; WAITS: no.
; Entry: X object. Exit: X kept. Not charged, A 0, Z set, Y $8E. Not one of the four, A the
; frame, N, Z and C from comparing it with $1C, Y $91. Flashed, A the flags, N and Z from them, Y
; OBJ_34B8 less $0E, C and V from that subtract. V kept otherwise.
RichterObjectFlash:
    ldy #$8E
    lda a:OBJ_3568,x
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
    lda a:OBJ_34B8,x
    sec
    sbc #$0E
    tay
    lda a:RichterFlashFlags,y
    ora #$80
    sta a:OBJ_SPRITE_ATTR,x
    rts

; @routine 01BD:4D1 RichterObjectCharge
; @run 6E:C4D1
; WHAT IT DOES: Charges an object from OBJ_34B8 $0C on, the first time the way it faces is newly
; pressed.
; HOW: OBJ_3568 counted up; RichterChargePads by OBJ_FACING_LEFT against zPadPressed.
; WAITS: no.
; Entry: X object. Exit: X kept. Before $0C, A OBJ_34B8, N, Z and C from comparing it with $0C.
; Charged already, A the charge, N and Z from it, C set. Not pressed, A 0, Z set, C set, Y the
; facing. Charged now, A the press, N and Z from the charge, C set, Y the facing. V kept.
RichterObjectCharge:
    lda a:OBJ_34B8,x
    cmp #$0C
    bcc RichterObjectCharge_at0
    lda a:OBJ_3568,x
    bne RichterObjectCharge_at0
    ldy a:OBJ_FACING_LEFT,x
    lda z:zPadPressed
    and a:RichterChargePads,y
    beq RichterObjectCharge_at0
    inc a:OBJ_3568,x
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
; HOW: OBJ_HURT_TIMER the timer, OBJ_SCROLL_BAND the step, OBJ_FRAME the frame shown.
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
    inc a:OBJ_SCROLL_BAND,x
    bra RichterObjectSetAnimation_at1

; @routine 01C0:263 RichterObjectSetAnimation
; @run 6E:DA63
; @stop DA84
; WHAT IT DOES: Starts an object's animation from its first step.
; HOW: The animation into OBJ_FIELDS_2EE0, step zero, and its first entry read.
; WAITS: no.
; Entry: A animation, X object. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
RichterObjectSetAnimation:
    sta a:OBJ_FIELDS_2EE0,x
RichterObjectSetAnimation_at3:
    stz a:OBJ_SCROLL_BAND,x
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
; HOW: RichterObjectAnimations by OBJ_FIELDS_2EE0 into zTemp00 and 01; the step doubled.
; WAITS: no.
; Entry: X object. Exit: A and Y twice the step, N and Z from it, C the step's bit 7. X and V
; kept.
RichterObjectAnimationList:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    tay
    lda a:RichterObjectAnimations,y
    sta z:zTemp00
    lda a:RichterObjectAnimations+1,y
    sta z:zTemp01
    lda a:OBJ_SCROLL_BAND,x
    asl a
    tay
    rts

; @routine 01C0:51B RichterObjectHitboxOff
; @run 6E:DD1B
; WHAT IT DOES: Takes an object's hitbox away.
; HOW: OBJ_3250 cleared.
; WAITS: no.
; Entry: X object. Exit: all registers and flags kept.
RichterObjectHitboxOff:
    stz a:OBJ_3250,x
    rts

; @routine 01C0:51F RichterObjectHitboxOn
; @run 6E:DD1F
; WHAT IT DOES: Marks an object as having a hitbox and gives it its frame's.
; HOW: OBJ_3250 1; runs on into RichterObjectHitboxByFrame.
; WAITS: no.
; Entry: X object. Exit: as RichterObjectHitboxByFrame.
RichterObjectHitboxOn:
    lda #$01
    sta a:OBJ_3250,x

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
; WHAT IT DOES: Adds an object's own pull (OBJ_3618 and OBJ_3670) to its Y speed, held at $0700.
; HOW: A word add into OBJ_VY_LO and OBJ_VY_HI; at 7 and up the speed is set to $0700.
; Untraced: the hold at $0700 ($C2E4); no script let one of her objects fall that fast.
; WAITS: no.
; Entry: X object. Exit: A the speed's high byte, V from its add; rising, N, Z and C from the add; falling slower
; than 7, N, Z and C from comparing it with 7; held, A 7, N and Z from it, C set. X and Y kept.
MariaObjectFallCapped:
    lda a:OBJ_3618,x
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,x
    lda a:OBJ_3670,x
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
    sta a:OBJ_FIELDS_2EE0,x
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
    lda a:OBJ_FIELDS_2EE0,x
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
