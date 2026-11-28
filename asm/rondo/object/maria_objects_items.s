; @routine 01E1:31D MariaObjectSetFall
; @run 6E:C31D
; WHAT IT DOES: RichterObjectSetFall at her address, the same bytes: an object's Y speed low byte A, its high byte
; plus the carry.
; HOW: RichterObjectSetFall's body.
; WAITS: no.
; Entry: A the low byte, C the carry, X the object. Exit: A the high byte, N and Z from it, C and V from its add.
; X and Y kept.
MariaObjectSetFall:
    sta a:OBJ_VY_LO,x
    cla
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
    rts

; @routine 01E2:497 MariaItem7Part
; @run 6E:CC97
; WHAT IT DOES: The handler of kind 7, item 7's part: while the attack's object in its OBJ_WORK_D is out, sits 7 rows
; below it, as it faces and flips, sprite set 3 frame $13, an 8 by 8 hitbox 6 down, live, OBJ_WORK_N $0C. When the
; attack's object is gone it makes sure her kind-3 companion is out (MariaClaimKind3Slot) and is cleared. With
; PlayerKind7Caught set it is cleared at once, and zField93 (the item in hand) with it; PlayerKind7Caught is cleared.
; HOW: The attack's slot in Y; the copies; MariaClaimKind3Slot; the tail JMPs to ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaItem7Part:
    ldx z:zRichterObject
    ldy a:OBJ_WORK_D,x
    lda a:OBJ_KIND,y
    beq MariaItem7Part_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$13
    sta a:OBJ_FRAME,x
    lda a:OBJ_X_LO,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,y
    sta a:OBJ_X_HI,x
    lda a:OBJ_Y_LO,y
    clc
    adc #$07
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,y
    adc #$00
    sta a:OBJ_Y_HI,x
    lda a:OBJ_SPRITE_FLIP,y
    sta a:OBJ_SPRITE_FLIP,x
    lda a:OBJ_FACING_LEFT,y
    sta a:OBJ_FACING_LEFT,x
    lda #$06
    sta a:OBJ_HITBOX_DY,x
    lda #$08
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$08
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$0C
    sta a:OBJ_WORK_N,x
    lda a:PlayerKind7Caught
    bne MariaItem7Part_at1
    rts
MariaItem7Part_at0:
    lda a:PlayerKind7Caught
    bne MariaItem7Part_at1
    jsr MariaClaimKind3Slot
    stz a:PlayerKind7Caught
    jmp VecClearObject
MariaItem7Part_at1:
    stz a:PlayerKind7Caught
    stz z:zField93
    jmp VecClearObject

; @routine 01E2:509 MariaItem4Object
; @run 6E:CD09
; WHAT IT DOES: The handler of kind 4, item 4's object: where she is, with a hitbox $10 high, 4 wide and $0E down,
; OBJ_WORK_N $0D; cleared once her Y's high byte is 1 (she is below the first screen).
; HOW: The tail JMPs to MariaObjectAtPlayerPlace and ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaObjectAtPlayerPlace, or with the processor as the clear found it.
MariaItem4Object:
    ldx z:zRichterObject
    lda a:OBJ_Y_HI
    dec a
    beq MariaItem4Object_at0
    lda #$10
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$04
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$0E
    sta a:OBJ_HITBOX_DY,x
    lda #$0D
    sta a:OBJ_WORK_N,x
    jmp MariaObjectAtPlayerPlace
MariaItem4Object_at0:
    jmp VecClearObject

; @routine 01E2:52B MariaObjectKind9
; @run 6E:CD2B
; WHAT IT DOES: The handler of kind 9, four pieces in slots 5 to 8. State 0: the piece in slot 5 makes slots 6 to 8
; pieces too; each starts where she is, from her, facing right, with its X speed by slot (MariaPieceSpeedsX), the
; even slots also Y speed entry 0, its animation timer $20, frame $0D on by slot, sprite set 1, state 1. State 1:
; moves, falls by $38 a frame, and is cleared when its timer runs out.
; HOW: ClearObject for the new slots; MariaObjectFromPlayer, MariaObjectAtPlayerPlace, MariaSetVelocityYFromTable;
; MariaObjectMove, MariaObjectSetFall; the tail JMPs to MariaObjectNextState and ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the last step left it.
MariaObjectKind9:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne MariaObjectKind9_at0
    cpx #$05
    bne MariaObjectKind9_at1
    phx
MariaObjectKind9_at2:
    inx
    jsr VecClearObject
    lda #$09
    sta a:OBJ_KIND,x
    cpx #$08
    bne MariaObjectKind9_at2
    plx
MariaObjectKind9_at1:
    jsr MariaObjectFromPlayer
    stz a:OBJ_FACING_LEFT,x
    jsr MariaObjectAtPlayerPlace
    txa
    sec
    sbc #$05
    asl a
    tay
    lda a:MariaPieceSpeedsX,y
    sta a:OBJ_VX_LO,x
    lda a:MariaPieceSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    txa
    and #$01
    bne MariaObjectKind9_at3
    lda #$00
    jsr MariaSetVelocityYFromTable
MariaObjectKind9_at3:
    lda #$20
    sta a:OBJ_HURT_TIMER,x
    txa
    sec
    sbc #$05
    clc
    adc #$0D
    sta a:OBJ_FRAME,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    jmp MariaObjectNextState
MariaObjectKind9_at0:
    jsr MariaObjectMove
    lda a:OBJ_VY_LO,x
    clc
    adc #$38
    jsr MariaObjectSetFall
    dec a:OBJ_HURT_TIMER,x
    beq MariaObjectKind9_at4
    rts
MariaObjectKind9_at4:
    jmp VecClearObject

; @routine 01E2:59E MariaObjectClearAfterSpeeds
; @run 6E:CD9E
; WHAT IT DOES: Empties the object's slot. Nothing is known to reach it: it sits after MariaPieceSpeedsX and no call,
; jump or table found goes to it.
; HOW: The tail JMP to ClearObject (the kernel's vector at $4054).
; WAITS: no.
; Entry: X the object. Exit: as ClearObject: everything kept.
MariaObjectClearAfterSpeeds:
    jmp VecClearObject

; @routine 01E2:5A1 MariaObjectFallStart7
; @run 6E:CDA1
; WHAT IT DOES: An object falls by $0038 a frame, with animation 7 and its next state.
; HOW: The stores, then the tail JMP to MariaObjectStartAnimation.
; WAITS: no.
; Entry: X the object. Exit: as MariaObjectStartAnimation.
MariaObjectFallStart7:
    lda #$38
    sta a:OBJ_FALL_LO,x
    lda #$00
    sta a:OBJ_FALL_HI,x
    lda #$07
    jmp MariaObjectStartAnimation
