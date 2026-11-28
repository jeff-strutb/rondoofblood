; @routine 05CB:382 CheckThenJump05CB382
; @run 70:B382
; WHAT IT DOES: An object's dispatcher (Secret stage 02): the overlay's check at $B58D, then the handler of its
; phase from the table at $B397.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump05CB382States = $B397 ; the handlers by phase
CheckThenJump05CB382:
    jsr SoundEvery32Frames
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:CheckThenJump05CB382States,y
    sta z:zTemp00
    lda a:CheckThenJump05CB382States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 09EC:03C CheckThenJump09EC03C
; @run 70:B83C
; WHAT IT DOES: An object's dispatcher (Secret stage 05): the overlay's check at $B859, then the handler of its
; state from the table at $B851.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump09EC03CStates = $B851 ; the handlers by state
CheckThenJump09EC03C:
    jsr RaiseStateAfterSlot10
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump09EC03CStates,y
    sta z:zTemp00
    lda a:CheckThenJump09EC03CStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0E0C:0CC CheckThenJump0E0C0CC
; @run 70:B8CC
; WHAT IT DOES: An object's dispatcher (Secret stage 08): the overlay's check at $B8F1, then the handler of its
; state from the table at $B8E1.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump0E0C0CCStates = $B8E1 ; the handlers by state
CheckThenJump0E0C0CC:
    jsr StopWhenSlot16Marked0E0C
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump0E0C0CCStates,y
    sta z:zTemp00
    lda a:CheckThenJump0E0C0CCStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBE:17E CheckThenJump0EBE17E
; @run 71:C97E
; WHAT IT DOES: An object's dispatcher (Stage 09): the overlay's check at $C9A1, then the handler of its
; state from the table at $C993.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump0EBE17EStates = $C993 ; the handlers by state
CheckThenJump0EBE17E:
    jsr ToState6WhenHitBit7
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump0EBE17EStates,y
    sta z:zTemp00
    lda a:CheckThenJump0EBE17EStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBE:1A1 ToState6WhenHitBit7
; @run 71:C9A1
; WHAT IT DOES: Takes the object to state 6 once bit 7 of its OBJ_HIT_STATUS is set.
; HOW: One test and store. Stage 0E's overlay holds the same bytes at $B7C6.
; WAITS: no.
; Entry: X object. Exit: A 0 (Z set) or 6; X, Y, C and V kept.
ToState6WhenHitBit7:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq ToState6WhenHitBit7_at0
    lda #$06
    sta a:OBJ_STATE,x
ToState6WhenHitBit7_at0:
    rts

; @routine 0EBE:652 CheckThenJump0EBE652
; @run 71:CE52
; WHAT IT DOES: An object's dispatcher (Stage 09): the overlay's check at $CE6D, then the handler of its
; state from the table at $CE67.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump0EBE652States = $CE67 ; the handlers by state
CheckThenJump0EBE652:
    jsr HitBoxByFrameTableCE8F
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump0EBE652States,y
    sta z:zTemp00
    lda a:CheckThenJump0EBE652States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBF:472 CheckThenJump0EBF472
; @run 71:D472
; WHAT IT DOES: An object's dispatcher (Stage 09): the overlay's check at $D491, then the handler of its
; state from the table at $D487.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump0EBF472States = $D487 ; the handlers by state
CheckThenJump0EBF472Check = $D491 ; the overlay's check
CheckThenJump0EBF472:
    jsr CheckThenJump0EBF472Check
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump0EBF472States,y
    sta z:zTemp00
    lda a:CheckThenJump0EBF472States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EC0:264 CheckThenJump0EC0264
; @run 71:DA64
; WHAT IT DOES: An object's dispatcher (Stage 09): the overlay's check at $DA81, then the handler of its
; state from the table at $DA79.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump0EC0264States = $DA79 ; the handlers by state
CheckThenJump0EC0264:
    jsr RiseWhenHitBit7_0EC0
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump0EC0264States,y
    sta z:zTemp00
    lda a:CheckThenJump0EC0264States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EC0:281 RiseWhenHitBit7_0EC0
; @run 71:DA81
; WHAT IT DOES: In states under 3, once bit 7 of its OBJ_HIT_STATUS is set: the hit box off, sprite set 3, frame 0,
; an upward speed and state 3 (Stage 09's overlay).
; HOW: Two tests and the stores.
; WAITS: no.
; Entry: X object. Exit: as the last test, or A 3; C from the state's compare; X, Y and V kept.
RiseWhenHitBit7_0EC0:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq RiseWhenHitBit7_0EC0_at0
    lda a:OBJ_STATE,x
    cmp #$03
    bcs RiseWhenHitBit7_0EC0_at0
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    stz a:OBJ_FRAME,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$FE
    sta a:OBJ_VY_LO,x
    lda #$03
    sta a:OBJ_STATE,x
RiseWhenHitBit7_0EC0_at0:
    rts

; @routine 101D:285 CheckThenJump101D285
; @run 71:C285
; WHAT IT DOES: An object's dispatcher (Stage 0A): the overlay's check at $C2A2, then the handler of its
; state from the table at $C29A.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump101D285States = $C29A ; the handlers by state
CheckThenJump101D285:
    jsr HoldWhenHitBit7_101D
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump101D285States,y
    sta z:zTemp00
    lda a:CheckThenJump101D285States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101D:2A2 HoldWhenHitBit7_101D
; @run 71:C2A2
; WHAT IT DOES: In states under 3, once bit 7 of its OBJ_HIT_STATUS is set: OBJ_WORK_D $40, frame $AA, state 3 and
; the hit box off (Stage 0A's overlay).
; HOW: Two tests and the stores.
; WAITS: no.
; Entry: X object. Exit: as the last test, or A 3; C from the state's compare; X, Y and V kept.
HoldWhenHitBit7_101D:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HoldWhenHitBit7_101D_at0
    lda a:OBJ_STATE,x
    cmp #$03
    bcs HoldWhenHitBit7_101D_at0
    lda #$40
    sta a:OBJ_FLASH_FRAMES,x
    lda #$AA
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_HITBOX_ON,x
HoldWhenHitBit7_101D_at0:
    rts

; @routine 10CC:10D CheckThenJump10CC10D
; @run 70:B90D
; WHAT IT DOES: An object's dispatcher (Secret stage 0A): the overlay's check at $B936, then the handler of its
; phase from the table at $B922.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump10CC10DStates = $B922 ; the handlers by phase
CheckThenJump10CC10D:
    jsr HitBoxByFrame10CC
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:CheckThenJump10CC10DStates,y
    sta z:zTemp00
    lda a:CheckThenJump10CC10DStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 10CC:136 HitBoxByFrame10CC
; @run 70:B936
; WHAT IT DOES: Gives the object the hit box of its frame, from $94 on (secret stage 0A's overlay).
; HOW: Four bytes of the table at $B958 by four times the frame less $94.
; WAITS: no.
; Entry: X object. Exit: A the half height, N and Z from it, Y the row's offset, C from the second shift, V from the
; subtract.
HitBoxByFrame10CCBoxes = $B958 ; X and Y offset, half width and half height by frame
HitBoxByFrame10CC:
    lda a:OBJ_FRAME,x
    sec
    sbc #$94
    asl a
    asl a
    tay
    lda a:HitBoxByFrame10CCBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:HitBoxByFrame10CCBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:HitBoxByFrame10CCBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:HitBoxByFrame10CCBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 117C:0CB CheckThenJump117C0CB
; @run 70:B8CB
; WHAT IT DOES: An object's dispatcher (Stage 0B): the overlay's check at $B8EA, then the handler of its
; state from the table at $B8E0.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump117C0CBStates = $B8E0 ; the handlers by state
CheckThenJump117C0CB:
    jsr CountdownThenState4B
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump117C0CBStates,y
    sta z:zTemp00
    lda a:CheckThenJump117C0CBStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117D:39D CheckThenJump117D39D
; @run 71:C39D
; WHAT IT DOES: An object's dispatcher (Stage 0B): the overlay's check at $C3B6, then the handler of its
; state from the table at $C3B2.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump117D39DStates = $C3B2 ; the handlers by state
CheckThenJump117D39D:
    jsr AnimateAlongListC3C2
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump117D39DStates,y
    sta z:zTemp00
    lda a:CheckThenJump117D39DStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117E:7F7 CheckThenJump117E7F7
; @run 71:CFF7
; WHAT IT DOES: An object's dispatcher (Stage 0B): the overlay's check at $D016, then the handler of its
; state from the table at $D00C.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump117E7F7States = $D00C ; the handlers by state
CheckThenJump117E7F7:
    jsr StartPiecesWhenHit117F
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump117E7F7States,y
    sta z:zTemp00
    lda a:CheckThenJump117E7F7States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117F:016 StartPiecesWhenHit117F
; @run 71:D016
; WHAT IT DOES: Once bit 7 of its OBJ_HIT_STATUS is set (Stage 0B's overlay): the next slot becomes kind $9C, the six
; after it get OBJ_WORK_N from the table at $D054 with their hit boxes off and pace bits kept to $87, and the object
; goes to state 4 with no frame or hit box.
; HOW: A loop over six slots, X kept on the stack.
; WAITS: no.
; Entry: X object, not yet in state 4 for anything to happen. Exit: A 4 or as the test; X kept, Y 6, C set.
StartPiecesWhenHit117FPieces = $D054 ; OBJ_WORK_N of the six slots
StartPiecesWhenHit117F:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq StartPiecesWhenHit117F_at0
    lda a:OBJ_STATE,x
    cmp #$04
    beq StartPiecesWhenHit117F_at0
    phx
    inx
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    inx
    cly
StartPiecesWhenHit117F_at1:
    lda a:StartPiecesWhenHit117FPieces,y
    sta a:OBJ_WORK_N,x
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_PACE,x
    and #$87
    sta a:OBJ_PACE,x
    inx
    iny
    cpy #$06
    bcc StartPiecesWhenHit117F_at1
    plx
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_FRAME,x
    lda #$04
    sta a:OBJ_STATE,x
StartPiecesWhenHit117F_at0:
    rts

; @routine 13FB:7AF CheckThenJump13FB7AF
; @run 70:B7AF
; WHAT IT DOES: An object's dispatcher (Secret stage 0C): the overlay's check at $B7CE, then the handler of its
; phase from the table at $B7C4.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump13FB7AFStates = $B7C4 ; the handlers by phase
CheckThenJump13FB7AF:
    jsr HoldNearLevelX13FB
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:CheckThenJump13FB7AFStates,y
    sta z:zTemp00
    lda a:CheckThenJump13FB7AFStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FB:7CE HoldNearLevelX13FB
; @run 70:B7CE
; WHAT IT DOES: Keeps the object's level X near $03xx (secret stage 0C's overlay): below $033D it moves half a dot
; right, on another screen from $44 of the low byte half a dot left; its X velocity is then cleared.
; HOW: The level X from the scroll and the object's screen X; one ApplyVelocityX.
; WAITS: no.
; Entry: X object. Exit: as the last compare or the move; X kept.
HoldNearLevelX13FB:
    lda a:ScrollXLo
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:ScrollXHiScreen
    adc a:OBJ_X_HI,x
    cmp #$03
    beq HoldNearLevelX13FB_at0
    lda z:zTemp00
    cmp #$44
    bcc HoldNearLevelX13FB_at1
    lda #$80
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    bra HoldNearLevelX13FB_at1
HoldNearLevelX13FB_at0:
    lda z:zTemp00
    cmp #$3D
    bcs HoldNearLevelX13FB_at1
    lda #$80
    sta a:OBJ_VX_LO,x
    stz a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
HoldNearLevelX13FB_at1:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    rts

; @routine 161B:7A3 CheckThenJump161B7A3
; @run 70:B7A3
; WHAT IT DOES: An object's dispatcher (Stage 0E): the overlay's check at $B7C6, then the handler of its
; state from the table at $B7B8.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump161B7A3States = $B7B8 ; the handlers by state
CheckThenJump161B7A3Check = $B7C6 ; the overlay's check
CheckThenJump161B7A3:
    jsr CheckThenJump161B7A3Check
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckThenJump161B7A3States,y
    sta z:zTemp00
    lda a:CheckThenJump161B7A3States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CC:112 PlaceChain16CC
; @run 70:B912
; WHAT IT DOES: Places the chain of slots $15 down to $11 (secret stage 0A's overlay): the first from slot $16 with a
; Y offset by slot $16's frame, each next from the slot above 8 dots down, each then moved by its angle's velocity.
; HOW: CopyObjectPlaceBack, ApplyVelocityY, SetVelocityFromAngle with the angle OBJ_WORK_D (speed zTemp00-01), the
; X velocity halved with its sign, ApplyVelocity.
; WAITS: no.
; Entry: nothing but X to keep. Exit: X restored, N and Z from it; C set; A, Y and V as the last calls left them.
PlaceChain16CCDrops = $B95B ; the first piece's Y offset by slot $16's frame less $0C
PlaceChain16CC:
    phx
    ldx #$15
PlaceChain16CC_at2:
    cpx #$15
    bne PlaceChain16CC_at0
    ldy #$16
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FRAME+22
    sec
    sbc #$0C
    tay
    lda a:PlaceChain16CCDrops,y
    sta a:OBJ_VY_HI,x
    bra PlaceChain16CC_at1
PlaceChain16CC_at0:
    txa
    tay
    iny
    jsr VecCopyObjectPlaceBack
    lda #$08
    sta a:OBJ_VY_HI,x
PlaceChain16CC_at1:
    jsr VecApplyVelocityY
    stz z:zTemp00
    lda #$06
    sta z:zTemp01
    lda a:OBJ_FLASH_FRAMES,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_VX_HI,x
    asl a
    ror a:OBJ_VX_HI,x
    ror a:OBJ_VX_LO,x
    jsr VecApplyVelocity
    dex
    cpx #$10
    bne PlaceChain16CC_at2
    plx
    rts

; @routine 16CC:165 CheckThenJump16CC165
; @run 70:B965
; WHAT IT DOES: An object's dispatcher (Secret stage 0E): the overlay's check at $B912, then the handler of its
; phase from the table at $B97A.
; HOW: JSR to the routine loaded at the check's address, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckThenJump16CC165States = $B97A ; the handlers by phase
CheckThenJump16CC165:
    jsr PlaceChain16CC
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:CheckThenJump16CC165States,y
    sta z:zTemp00
    lda a:CheckThenJump16CC165States+1,y
    sta z:zTemp01
    jmp (ZeroPage)
