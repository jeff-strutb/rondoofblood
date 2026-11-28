; @routine 01BB:611 RichterReadPad
; @run 6D:B611
; WHAT IT DOES: Reads the pad for the frame: what is held, and what is newly pressed. Bit 0
; counts as pressed only when held for three frames after being up, which steadies it.
; HOW: The last four frames of the pad are kept in PadHistory.
; WAITS: no.
; Entry: nothing. Exit: A the new presses, N and Z from them. X, Y, C and V kept. Clobbers
; zTemp00.
RichterReadPad:
    lda z:zPadHeld
    sta z:zPadPrevious
    lda a:PadHistory+2
    sta a:PadHistory+3
    lda a:PadHistory+1
    sta a:PadHistory+2
    lda a:PadHistory
    sta a:PadHistory+1
    lda a:BiosPadState
    sta a:PadHistory
    sta z:zPadHeld
    eor a:PadHistory+3
    and a:PadHistory+2
    and a:PadHistory+1
    and a:PadHistory
    and #$01
    sta z:zTemp00
    lda z:zPadHeld
    eor z:zPadPrevious
    and z:zPadHeld
    and #$FE
    ora z:zTemp00
    sta z:zPadPressed
    rts

; @routine 01BB:64C RichterClearPadOverrides
; @run 6D:B64C
; WHAT IT DOES: Clears the four bytes that force the pad.
; HOW: Stores zero.
; WAITS: no.
; Entry: nothing. Exit: all registers and flags kept.
RichterClearPadOverrides:
    stz a:PadOverrides
    stz a:PadOverrides+1
    stz a:PadOverrides+2
    stz a:PadOverrides+3
    rts

; @routine 01BB:659 RichterAnimationWait
; @run 6D:B659
; WHAT IT DOES: Steps the animation and, unless it has ended, returns to its caller's caller, so
; the caller's own work waits for the end.
; HOW: RichterAnimationRunning; the assembly drops the return address with two pulls.
; WAITS: no.
; Entry: nothing. Exit, ended: as RichterAnimationRunning. Otherwise the caller's return address
; is gone, A its high byte, N and Z from it; its caller then ends at once.
RichterAnimationWait:
    bsr RichterAnimationRunning
    beq RichterAnimationWait_at0
    pla
    pla
RichterAnimationWait_at0:
    rts

; @routine 01BB:660 RichterAnimationRunning
; @run 6D:B660
; WHAT IT DOES: Steps the animation; a caller looks at whether it has ended.
; HOW: RichterAnimationTick, then the timer against $FF.
; WAITS: no.
; Entry: nothing. Exit: A the timer, N, Z and C from comparing it with $FF. Y as the tick. X and V
; kept. Clobbers zTemp00-01.
RichterAnimationRunning:
    jsr RichterAnimationTick
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    rts

; @routine 01BB:669 RichterAnimationTick
; @run 6D:B669
; WHAT IT DOES: Counts down the frame on show; at zero moves to the next step of the list.
; A timer of $FF means the animation has stopped.
; HOW: The timer in OBJ_HURT_TIMER, the step in OBJ_SCROLL_BAND, the frame shown in OBJ_FRAME.
; WAITS: no.
; Entry: nothing. Exit: A the frame or $FF; Y twice the step, plus one with a frame; C and N, Z
; from what was read. X and V kept. Clobbers zTemp00-01.
RichterAnimationTick:
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    bne RichterAnimationTick_at0
    rts
RichterAnimationTick_at0:
    dec a:OBJ_HURT_TIMER
    bne $B693
    inc a:OBJ_SCROLL_BAND
    bra $B681

; @routine 01BB:67B RichterSetAnimation
; @run 6D:B67B
; WHAT IT DOES: Starts an animation from its first step.
; HOW: The animation into OBJ_WORK_A, step zero, and its first entry read.
; WAITS: no.
; Entry: A animation. Exit: as RichterAnimationTick. Clobbers zTemp00-01.
RichterSetAnimation:
    sta a:OBJ_WORK_A
    stz a:OBJ_SCROLL_BAND
    bsr RichterAnimationPoint
    lda (zTemp00),y
    sta a:OBJ_HURT_TIMER
    cmp #$FF
    bne RichterSetAnimation_at0
    rts
RichterSetAnimation_at0:
    cmp #$FE
    beq LoadRecordStartPieceAtPlace
    bra RichterSetAnimation_at1
    bsr RichterAnimationPoint
RichterSetAnimation_at1:
    iny
    lda (zTemp00),y
    sta a:OBJ_FRAME
    rts

; @routine 01BB:69C RichterAnimationPoint
; @run 6D:B69C
; WHAT IT DOES: Points zTemp00-01 at the list of the animation in play and gives Y twice the step in play.
; HOW: OBJ_WORK_A doubled indexes AnimationLists; the step in OBJ_SCROLL_BAND doubled.
; WAITS: no.
; Entry: nothing. Exit: A and Y twice the step, N and Z from it, C the step's bit 7. X and V kept.
RichterAnimationPoint:
    lda a:OBJ_WORK_A
    asl a
    tay
    lda a:AnimationLists,y
    sta z:zTemp00
    lda a:AnimationLists+1,y
    sta z:zTemp01
    lda a:OBJ_SCROLL_BAND
    asl a
    tay
    rts

; @routine 01BB:704 RichterCheckTurnWindow
; @run 6D:B704
; WHAT IT DOES: Says whether the state in play lets him turn at the screen's middle: a table
; by state, in the first step only, and only on the side of the middle he faces away from.
; HOW: PlayerDoorReady is the answer.
; WAITS: no.
; Entry: nothing. Exit: A the last read, N and Z from it; X the state if it went that far. Y,
; C and V kept.
RichterCheckTurnWindow:
    stz a:PlayerDoorReady
    lda a:OBJ_SUBSTATE
    beq RichterCheckTurnWindow_at0
    rts
RichterCheckTurnWindow_at0:
    ldx a:OBJ_STATE
    lda a:RichterTurnStates,x
    sta a:PlayerDoorReady
    lda a:OBJ_FACING_LEFT
    bne RichterCheckTurnWindow_at1
    lda a:OBJ_X_LO
    bmi RichterCheckTurnWindow_at2
    stz a:PlayerDoorReady
    rts
RichterCheckTurnWindow_at1:
    lda a:OBJ_X_LO
    bpl RichterCheckTurnWindow_at2
    stz a:PlayerDoorReady
RichterCheckTurnWindow_at2:
    rts

; @routine 01BB:7CA RichterFaceByPad
; @run 6D:B7CA
; WHAT IT DOES: Turns him to face the way the pad holds, left or right; with neither, as he is.
; HOW: Bits 7 and 5 of zPadHeld, and facing in OBJ_FACING_LEFT: 0 for right, 1 for left.
; WAITS: no.
; Entry: nothing. Exit: A the bits looked at last; N and Z from them, or from the facing when it
; turns left. X, Y, C and V kept.
RichterFaceByPad:
    lda z:zPadHeld
    and #$A0
    bne RichterFaceByPad_at0
    rts
RichterFaceByPad_at0:
    stz a:OBJ_FACING_LEFT
    and #$20
    beq RichterFaceByPad_at1
    rts
RichterFaceByPad_at1:
    inc a:OBJ_FACING_LEFT
    rts

; @routine 01BB:7DD RichterNoteItemPress
; @run 6D:B7DD
; WHAT IT DOES: Notes that the item button was pressed this frame.
; HOW: Bit 2 of zPadPressed sets RichterItemRequest to 4.
; WAITS: no.
; Entry: nothing. Exit: Z from the test, N and V bits 7 and 6 of the presses; A 4 if pressed.
; X, Y and C kept.
RichterNoteItemPress:
    tst #$04,z:zPadPressed
    bne RichterNoteItemPress_at0
    rts
RichterNoteItemPress_at0:
    lda #$04
    sta a:PadForcedPress
    rts

; @routine 01BB:7E9 RichterCheckItemHearts
; @run 6D:B7E9
; WHAT IT DOES: Says whether he has the hearts the item in hand costs.
; HOW: zHearts against a table by item, in BCD; RichterItemReady is $0F if so, else 0.
; WAITS: no.
; Entry: nothing. Exit: X the item; A the hearts, N, Z and C from the compare, or A $0F. Y and
; V kept.
RichterCheckItemHearts:
    stz a:RichterItemReady
    ldx z:zField93
    lda z:zField94
    cmp a:RichterItemCosts,x
    bcs RichterCheckItemHearts_at0
    rts
RichterCheckItemHearts_at0:
    lda #$0F
    sta a:RichterItemReady
    rts

; @routine 01BB:7FC RichterTryItemCrash
; @run 6D:B7FC
; WHAT IT DOES: The item crash: on a press of the item button with no item of his out, clears
; the item slots and, if he has the hearts the crash costs, starts its sound and its script and
; pays the hearts. Says whether it started; when it does not, the forced press is dropped.
; HOW: Bit 2 of zPadPressed; OBJ_KIND of slots 3 to 15 all empty; RichterItemCosts in BCD, then
; RichterItemCrashLists and RichterItemCrashScripts by item, 0 for none, on channel and slot 6.
; WAITS: no.
; Entry: nothing. Exit: V bit 6 of the presses. Started: C set, A the hearts left, N and Z from
; them, X the item, Y the script if one started. Not started, C clear and: no press, N bit 7 of
; the presses, Z set, A, X, Y kept; an item out, A its OBJ_KIND, N and Z from it, X its slot;
; too few hearts, A the hearts, N and Z from comparing them with the cost, X the item. Y kept.
RichterTryItemCrash:
    tst #$04,z:zPadPressed
    beq RichterTryItemCrash_at0
    ldx #$03
RichterTryItemCrash_at1:
    lda a:OBJ_KIND,x
    bne RichterTryItemCrash_at0
    inx
    cpx #$10
    bne RichterTryItemCrash_at1
    dex
RichterTryItemCrash_at2:
    jsr VecClearObject
    dex
    cpx #$02
    bne RichterTryItemCrash_at2
    ldx z:zField93
    lda z:zField94
    cmp a:RichterItemCosts,x
    bcs DroppingPieceStep
RichterTryItemCrash_at0:
    stz a:PadForcedPress
    clc
    rts
    ldx z:zField93
    lda a:RichterItemCrashLists,x
    beq RichterTryItemCrash_at3
    ldx #$06
    jsr VecStartListChannelKind2
RichterTryItemCrash_at3:
    stz a:PlayerItemFirstUse
    ldx z:zField93
    lda a:RichterItemCrashScripts,x
    beq RichterTryItemCrash_at4
    ldx #$06
    jsr VecStartScriptSlotKind1
RichterTryItemCrash_at4:
    ldx z:zField93
    lda z:zField94
    sed
    sec
    sbc a:RichterItemCosts,x
    cld
    sta z:zField94
    sec
    rts

; @routine 01BB:909 RichterLatchButtons
; @run 6D:B909
; WHAT IT DOES: Keeps the jump and attack presses until a state acts on them.
; HOW: Bits 1 and 0 of zPadPressed ORed into RichterButtonsLatched.
; WAITS: no.
; Entry: nothing. Exit: A what is latched, N and Z from it. X, Y, C and V kept.
RichterLatchButtons:
    lda z:zPadPressed
    and #$02
    bsr RichterLatchBits
    lda z:zPadPressed
    and #$01

; @routine 01BC:113 RichterLatchBits
; @run 6D:B913
; WHAT IT DOES: Adds the buttons in A to the ones latched in PadForcedPressB.
; HOW: ORA of the byte, STA back.
; WAITS: no.
; Entry: A the buttons. Exit: A the latched byte, N and Z from it. X, Y, C and V kept.
RichterLatchBits:
    ora a:PadForcedPressB
    sta a:PadForcedPressB
    rts

; @routine 01BC:129 RichterUseItem
; @run 6D:B929
; WHAT IT DOES: Using the item in hand: on the use press with the use direction held and the
; hearts the item costs, goes to the item's own handler, which throws it or not. Says whether
; to attack, and with what in PlayerAttackKind: 0 the whip, or the item's attack.
; HOW: Bit 1 of zPadPressed, bit 4 of zPadHeld, RichterItemUseCosts by item, then the jump
; through RichterItemUseHandlers: items 0 and 1 attack with the whip. zField93 is 0 to 8; the
; C takes anything past 8 as the whip.
; WAITS: no.
; Entry: nothing. Exit: no press, C clear and all else kept. Otherwise C set, V and Y kept
; except as a handler leaves Y, and: not held, A 0, N and Z from it; too few hearts, A the
; hearts, X the item, N and Z from comparing them; items 0 and 1, A and X twice the item, N and
; Z from it; as the handler otherwise.
RichterUseItem:
    bbs1 zPadPressed,StartGroupThenThrow
    clc
    rts
    lda z:zPadHeld
    and #$10
    beq RichterUseItem_at0
    ldx z:zField93
    lda z:zField94
    cmp a:RichterItemUseCosts,x
    bcs RichterUseItem_at1
RichterUseItem_at0:
    stz a:PlayerAttackKind
    sec
    rts
RichterUseItem_at1:
    lda z:zField93
    asl a
    tax
    jmp (RichterItemUseHandlers,x)

; @routine 01BC:15B RichterUseThrownItem
; @run 6D:B95B
; WHAT IT DOES: Items 2 to 5, which can be thrown up to three at a time: the first throw of a
; use starts the item's sound and script, unless one is out already; a second or third throw
; needs more hearts, and a fourth is refused. Then the item's attack, and its hearts paid.
; HOW: PlayerItemFirstUse marks the first throw. The items fly in slots 5, 8 and 11; with one out
; the hearts must reach RichterItemSecondCosts, with two RichterItemThirdCosts.
; WAITS: no.
; Entry: X twice the item. Exit: as RichterUseItem.
RichterUseThrownItem:
    lda a:PlayerItemFirstUse
    beq RichterUseThrownItem_at0
    jsr RichterFindItemOut
    bne $B93D
    ldx z:zField93
    lda a:RichterItemUseLists,x
    ldx #$06
    jsr VecStartListChannelKind2
    stz a:PlayerItemFirstUse
    ldx z:zField93
    lda a:RichterItemUseScripts,x
    ldx #$06
    jsr VecStartScriptSlotKind1
RichterUseThrownItem_at0:
    ldx z:zField93
    lda a:OBJ_KIND+5
    beq RichterUseThrownItem_at1
    ldx z:zField93
    lda z:zField94
    cmp a:RichterItemSecondCosts,x
    bcc $B93D
    lda a:OBJ_KIND+8
    beq RichterUseThrownItem_at1
    lda z:zField94
    cmp a:RichterItemThirdCosts,x
    bcc $B93D
    lda a:$275B
    bne $B93D
RichterUseThrownItem_at1:
    ldx z:zField93
    lda a:RichterItemAttacks,x
    sta a:PlayerAttackKind
    lda z:zField94
    sed
    sec
    sbc a:RichterItemUseCosts,x
    cld
    sta z:zField94
    sec
    rts

; @routine 01BC:1B1 RichterUseItemSix
; @run 6D:B9B1
; WHAT IT DOES: Item 6: only with no item out; the first use starts sound list $54 and script 7.
; Then the item's attack, and its hearts paid.
; HOW: RichterFindItemOut; PlayerItemFirstUse marks the first use.
; WAITS: no.
; Entry: X twice the item. Exit: as RichterUseItem.
RichterUseItemSix:
    bsr RichterFindItemOut
    bne $B93D
    lda a:PlayerItemFirstUse
    beq RichterUseItemSix_at0
    lda #$54
    ldx #$06
    jsr VecStartListChannelKind2
    lda #$07
    ldx #$06
    jsr VecStartScriptSlotKind1
    stz a:PlayerItemFirstUse
RichterUseItemSix_at0:
    bra $B99D

; @routine 01BC:1CD RichterUseItemSeven
; @run 6D:B9CD
; WHAT IT DOES: Item 7: used while one of slots 5, 8 and 11 is free; then the item's attack, and
; its hearts paid. With all three taken, the whip.
; HOW: OBJ_KIND of each slot in turn.
; WAITS: no.
; Entry: X twice the item. Exit: as RichterUseItem.
RichterUseItemSeven:
    lda a:OBJ_KIND+5
    beq $B99D
    lda a:OBJ_KIND+8
    beq $B99D
    lda a:$275B
    beq $B99D
    jmp $B93D

; @routine 01BC:1DF RichterFindItemOut
; @run 6D:B9DF
; WHAT IT DOES: Finds the first of slots 5 to 15 with an object in it.
; HOW: OBJ_KIND of each.
; WAITS: no.
; Entry: nothing. Exit: Y the slot, or $10; A its OBJ_KIND, or 0. Found: N and Z from A, C kept
; if it was slot 5, else clear. None: Z and C set, N clear. X and V kept.
RichterFindItemOut:
    ldy #$05
    lda a:OBJ_KIND,y
    bne RichterFindItemOut_at0
    iny
    cpy #$10
    bne StepListThenStartType2E
RichterFindItemOut_at0:
    rts

; @routine 01BC:1EC RichterUseItemEight
; @run 6D:B9EC
; WHAT IT DOES: Item 8: only with no item out; the first use starts sound list $53 and script
; $15. Then the item's attack, and its hearts paid.
; HOW: RichterFindItemOut; PlayerItemFirstUse marks the first use.
; WAITS: no.
; Entry: X twice the item. Exit: as RichterUseItem.
RichterUseItemEight:
    bsr RichterFindItemOut
    beq RichterUseItemEight_at0
    jmp $B93D
RichterUseItemEight_at0:
    lda a:PlayerItemFirstUse
    beq $B9CB
    lda #$53
    ldx #$06
    jsr VecStartListChannelKind2
    lda #$15
    ldx #$06
    jsr VecStartScriptSlotKind1
    stz a:PlayerItemFirstUse
    bra $B99D

; @routine 01E0:124 MariaReadPad
; @run 6D:B924
; WHAT IT DOES: Reads the pad for the frame: what is held, and what is newly pressed.
; HOW: The last frame's zPadHeld into zPadPrevious, BiosPadState into zPadHeld, the bits newly set into zPadPressed.
; WAITS: no.
; Entry: nothing. Exit: A the presses, N and Z from them. X, Y, C and V kept.
MariaReadPad:
    lda z:zPadHeld
    sta z:zPadPrevious
    lda a:BiosPadState
    sta z:zPadHeld
    eor z:zPadPrevious
    and z:zPadHeld
    sta z:zPadPressed
    rts

; @routine 01E0:134 MariaPadSequence
; @run 6D:B934
; WHAT IT DOES: Follows a sequence of directions on her pad: each must come within ten frames of the last, in the
; order her table at $B96B gives for the way she faces; the third sets the done flag.
; HOW: A timer, the step and the done flag, three bytes after PlayerByte3BE3; the table by twice the step plus her
; facing; a timer run out clears the step and the flag.
; Untraced: the third step ($B95A); no script finished a sequence.
; WAITS: no.
; Entry: nothing. Exit: timer out or a step made, A $0A, N and Z from it; no direction, A 0, Z set; a direction not
; the next, A the directions, N, Z and C from comparing them. X the table index unless the timer was out; C from
; the index's add, or from comparing the step with 3; V from the index's add. Y kept.
MariaPadSequence:
    lda a:PlayerByte3BE3+3
    beq MariaPadSequence_at2
    dec a:PlayerByte3BE3+3
    lda a:PlayerByte3BE3+2
    asl a
    clc
    adc a:OBJ_FACING_LEFT
    tax
    lda z:zPadHeld
    and #$F0
    beq MariaPadSequence_at0
    cmp a:MariaPadSequence+$37,x
    bne MariaPadSequence_at0
    inc a:PlayerByte3BE3+2
    lda a:PlayerByte3BE3+2
    cmp #$03
    bne MariaPadSequence_at1
    inc a:PlayerByte3BE3+4
    bra MariaPadSequence_at1
MariaPadSequence_at2:
    stz a:PlayerByte3BE3+2
    stz a:PlayerByte3BE3+4
MariaPadSequence_at1:
    lda #$0A
    sta a:PlayerByte3BE3+3
MariaPadSequence_at0:
    rts

; @routine 01E0:173 MariaClearPadOverrides
; @run 6D:B973
; WHAT IT DOES: RichterClearPadOverrides in Maria's player overlay (01DD), the same instructions at her address.
; HOW: RichterClearPadOverrides's body.
; WAITS: no.
; Entry, exit: as RichterClearPadOverrides.
MariaClearPadOverrides:
    stz a:PadOverrides
    stz a:PadOverrides+1
    stz a:PadOverrides+2
    stz a:PadOverrides+3
    rts

; @routine 01E0:180 MariaAnimationWait
; @run 6D:B980
; WHAT IT DOES: RichterAnimationWait in Maria's player overlay (01DD), the same instructions.
; HOW: RichterAnimationWait's body with her layout (MariaAnimationRunning at $B987).
; WAITS: no.
; Entry, exit: as RichterAnimationWait.
MariaAnimationWait:
    bsr MariaAnimationRunning
    beq MariaAnimationWait_at0
    pla
    pla
MariaAnimationWait_at0:
    rts

; @routine 01E0:187 MariaAnimationRunning
; @run 6D:B987
; WHAT IT DOES: RichterAnimationRunning in Maria's player overlay (01DD).
; HOW: The same instructions with her tick, MariaAnimationTick at $B990.
; WAITS: no.
; Entry, exit: as RichterAnimationRunning.
MariaAnimationRunning:
    jsr MariaAnimationTick
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    rts

; @routine 01E0:190 MariaAnimationTick
; @run 6D:B990
; WHAT IT DOES: RichterAnimationTick in Maria's player overlay (01DD), the same bytes.
; HOW: RichterAnimationTick's body with her layout: its calls go to MariaAnimationPoint at $B9C3.
; WAITS: no.
; Entry, exit: as RichterAnimationTick.
MariaAnimationTick:
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    bne MariaAnimationTick_at0
    rts
MariaAnimationTick_at0:
    dec a:OBJ_HURT_TIMER
    bne MariaSetAnimation_at3
    inc a:OBJ_SCROLL_BAND
    bra MariaSetAnimation_at2

; @routine 01E0:1A2 MariaSetAnimation
; @run 6D:B9A2
; WHAT IT DOES: RichterSetAnimation in Maria's player overlay (01DD), the same bytes.
; HOW: RichterSetAnimation's body with her layout: its calls go to MariaAnimationPoint at $B9C3.
; WAITS: no.
; Entry, exit: as RichterSetAnimation.
MariaSetAnimation:
    sta a:OBJ_WORK_A
MariaSetAnimation_at4:
    stz a:OBJ_SCROLL_BAND
MariaSetAnimation_at2:
    bsr MariaAnimationPoint
    lda (zTemp00),y
    sta a:OBJ_HURT_TIMER
    cmp #$FF
    bne MariaSetAnimation_at0
    rts
MariaSetAnimation_at0:
    cmp #$FE
    beq MariaSetAnimation_at4
    bra MariaSetAnimation_at1
MariaSetAnimation_at3:
    bsr MariaAnimationPoint
MariaSetAnimation_at1:
    iny
    lda (zTemp00),y
    sta a:OBJ_FRAME
    rts

; @routine 01E0:306 MariaNoteItemPress
; @run 6D:BB06
; WHAT IT DOES: RichterNoteItemPress in Maria's player overlay (01DD), the same instructions at her address.
; HOW: RichterNoteItemPress's body.
; WAITS: no.
; Entry, exit: as RichterNoteItemPress.
MariaNoteItemPress:
    tst #$04,z:zPadPressed
    bne MariaNoteItemPress_at0
    rts
MariaNoteItemPress_at0:
    lda #$04
    sta a:PadForcedPress
    rts

; @routine 01E0:1C3 MariaAnimationPoint
; @run 6D:B9C3
; WHAT IT DOES: RichterAnimationPoint in Maria's player overlay (01DD).
; HOW: The same instructions with her table of animation lists at $BF3D, $322 above his.
; WAITS: no.
; Entry, exit: as RichterAnimationPoint.
MariaAnimationPoint:
    lda a:OBJ_WORK_A
    asl a
    tay
    lda a:AnimationLists+$322,y
    sta z:zTemp00
    lda a:AnimationLists+$322+1,y
    sta z:zTemp01
    lda a:OBJ_SCROLL_BAND
    asl a
    tay
    rts

; @routine 01E0:22B MariaCheckTurnWindow
; @run 6D:BA2B
; WHAT IT DOES: RichterCheckTurnWindow in Maria's player overlay (01DD).
; HOW: The same instructions with her table of turn states at $BA54, $327 above his.
; WAITS: no.
; Entry, exit: as RichterCheckTurnWindow.
MariaCheckTurnWindow:
    stz a:PlayerByte3BD7
    lda a:OBJ_SUBSTATE
    beq MariaCheckTurnWindow_at0
    rts
MariaCheckTurnWindow_at0:
    ldx a:OBJ_STATE
    lda a:RichterTurnStates+$327,x
    sta a:PlayerByte3BD7
    lda a:OBJ_FACING_LEFT
    bne MariaCheckTurnWindow_at1
    lda a:OBJ_X_LO
    bmi MariaCheckTurnWindow_at2
    stz a:PlayerByte3BD7
    rts
MariaCheckTurnWindow_at1:
    lda a:OBJ_X_LO
    bpl MariaCheckTurnWindow_at2
    stz a:PlayerByte3BD7
MariaCheckTurnWindow_at2:
    rts

; @routine 01E0:312 MariaCheckItemHearts
; @run 6D:BB12
; WHAT IT DOES: RichterCheckItemHearts in Maria's player overlay (01DD).
; HOW: The same instructions with her table of item costs at $BD4A, $4FD above his.
; WAITS: no.
; Entry, exit: as RichterCheckItemHearts.
MariaCheckItemHearts:
    stz a:RichterItemReady
    ldx z:zField93
    lda z:zField94
    cmp a:RichterItemCosts+$4FD,x
    bcs MariaCheckItemHearts_at0
    rts
MariaCheckItemHearts_at0:
    lda #$0F
    sta a:RichterItemReady
    rts

; @routine 01E0:43A MariaItemUseAll
; @run 6D:BC3A
; WHAT IT DOES: The end of MariaUseItem when her sequence is done: PlayerByte3BD5 $FF, then the item delay.
; HOW: A jump into MariaUseItem's end.
; Untraced: all of it; no script finished a sequence.
; WAITS: no.
; Entry: nothing. Exit: A $10, N and Z from it, C set. X, Y and V kept.
MariaItemUseAll:
    lda #$FF
    sta a:PlayerByte3BD5
    jmp MariaUseItem_at8

; @routine 01E0:442 MariaUseItem
; @run 6D:BC42
; WHAT IT DOES: Her attack button: unless the delay in OBJ_EVENT_ID runs, on a press of button II says she attacks
; (C set) and starts the delay. With down held and the hearts her table at $BCDE asks, an item of 2 and up is
; thrown: its list and script start once (PlayerByte3BD6) unless an object is alive in slots 5 to 14, it waits for
; slot 5, and slot 8 past the second cost at $BCD5, and the hearts are paid; PlayerByte3BD5 then says the item is
; in use. In states below 5 with her pad sequence done, PlayerByte3BD5 is $FF.
; HOW: Bit 1 of zPadPressed, bit 4 of zPadHeld, zField93 the item, zField94 the hearts in BCD; MariaFindLiveObject;
; the list on channel 6 and the script in slot 6.
; Untraced: the sequence's arm ($BC56 taken, into MariaItemUseAll) and the throw ($BC77 to $BCC0); no script threw
; one of her items or finished a sequence.
; WAITS: no.
; Entry: nothing. Exit: the delay runs, A the delay, N and Z from it, C clear; no press, A 0, Z set, C clear;
; otherwise A $10, N and Z from it, C set, X the item once it was read, Y as the throw's last call left it. V kept.
MariaUseItem:
    lda a:OBJ_EVENT_ID
    bne MariaUseItem_at0
    bbs1 zPadPressed,MariaUseItem_at1
MariaUseItem_at0:
    clc
    rts
MariaUseItem_at1:
    lda a:OBJ_STATE
    cmp #$05
    bcs MariaUseItem_at2
    lda a:PlayerByte3BE3+4
    bne MariaItemUseAll
MariaUseItem_at2:
    lda z:zPadHeld
    and #$10
    beq MariaUseItem_at3
    ldx z:zField93
    lda z:zField94
    cmp a:MariaUseItem+$9C,x
    bcs MariaUseItem_at4
MariaUseItem_at3:
    stz a:PlayerByte3BD5
MariaUseItem_at8:
    lda #$10
    sta a:OBJ_EVENT_ID
    sec
    rts
MariaUseItem_at4:
    lda z:zField93
    cmp #$02
    bcc MariaUseItem_at3
    lda a:PlayerByte3BD6
    beq MariaUseItem_at5
    jsr MariaFindLiveObject
    bne MariaUseItem_at3
    ldx z:zField93
    lda a:MariaUseItem+$A5,x
    beq MariaUseItem_at6
    ldx #$06
    jsr VecStartListChannelKind2
MariaUseItem_at6:
    stz a:PlayerByte3BD6
    ldx z:zField93
    lda a:MariaUseItem+$AE,x
    beq MariaUseItem_at5
    ldx #$06
    jsr VecStartScriptSlotKind1
MariaUseItem_at5:
    ldx z:zField93
    lda a:OBJ_KIND+5
    beq MariaUseItem_at7
    ldx z:zField93
    lda z:zField94
    cmp a:MariaUseItem+$93,x
    bcc MariaUseItem_at3
    lda a:OBJ_KIND+8
    bne MariaUseItem_at3
MariaUseItem_at7:
    lda #$01
    sta a:PlayerByte3BD5
    lda z:zField94
    sed
    sec
    sbc a:MariaUseItem+$9C,x
    cld
    sta z:zField94
    jmp MariaUseItem_at8

; @routine 01E0:4C3 MariaFindLiveObject
; @run 6D:BCC3
; WHAT IT DOES: Looks for an object alive in slots 5 to 14, other than one of kind $18.
; HOW: OBJ_KIND by Y.
; Untraced: all of it; only MariaUseItem calls it, past its own untraced test.
; WAITS: no.
; Entry: nothing. Exit: found, Y the slot, A its kind, N, Z and C from comparing it with $18; else Y $0F, A the
; last kind read, Z and C set, N clear. X and V kept.
MariaFindLiveObject:
    ldy #$05
MariaFindLiveObject_at1:
    lda a:OBJ_KIND,y
    bne MariaFindLiveObject_at0
MariaFindLiveObject_at2:
    iny
    cpy #$0F
    bne MariaFindLiveObject_at1
    rts
MariaFindLiveObject_at0:
    cmp #$18
    beq MariaFindLiveObject_at2
    rts

; @routine 01E0:4F9 MariaTryItemCrash
; @run 6D:BCF9
; WHAT IT DOES: RichterTryItemCrash in Maria's player overlay (01DD): her item slots are 3 to 14, not 3 to 15.
; HOW: RichterTryItemCrash's body with her end of the scan and her tables of costs, lists and scripts at $BD4A,
; $BD53 and $BD5C, $4FD above his.
; Untraced: everything after a press of the item button ($BCFE to $BD1B, $BD21 to $BD49); no script pressed it.
; WAITS: no.
; Entry, exit: as RichterTryItemCrash.
MariaTryItemCrash:
    tst #$04,z:zPadPressed
    beq MariaTryItemCrash_at5
    ldx #$03
MariaTryItemCrash_at0:
    lda a:OBJ_KIND,x
    bne MariaTryItemCrash_at5
    inx
    cpx #$0F
    bne MariaTryItemCrash_at0
    dex
MariaTryItemCrash_at1:
    jsr VecClearObject
    dex
    cpx #$02
    bne MariaTryItemCrash_at1
    ldx z:zField93
    lda z:zField94
    cmp a:RichterItemCosts+$4FD,x
    bcs MariaTryItemCrash_at2
MariaTryItemCrash_at5:
    stz a:PadForcedPress
    clc
    rts
MariaTryItemCrash_at2:
    ldx z:zField93
    lda a:RichterItemCrashLists+$4FD,x
    beq MariaTryItemCrash_at3
    ldx #$06
    jsr VecStartListChannelKind2
MariaTryItemCrash_at3:
    stz a:PlayerByte3BD6
    ldx z:zField93
    lda a:RichterItemCrashScripts+$4FD,x
    beq MariaTryItemCrash_at4
    ldx #$06
    jsr VecStartScriptSlotKind1
MariaTryItemCrash_at4:
    ldx z:zField93
    lda z:zField94
    sed
    sec
    sbc a:RichterItemCosts+$4FD,x
    cld
    sta z:zField94
    sec
    rts
