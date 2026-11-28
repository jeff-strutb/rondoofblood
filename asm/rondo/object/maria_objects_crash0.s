; @routine 01E3:3D3 MariaCrash0Object
; @run 6E:D3D3
; WHAT IT DOES: The handler of kind 13, crash kind 0's object: its state's routine from MariaCrash0States (start,
; rise, hover, rain, leave).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
MariaCrash0Object:
    jsr MariaStateDoubled
    jmp (MariaCrash0States,x)

; @routine 01E3:3E3 MariaCrash0Clear
; @run 6E:D3E3
; WHAT IT DOES: Clears crash kind 0's object when she is hit or below the screen before it starts.
; HOW: The tail JMP to ClearObject through the kernel's vector.
; WAITS: no.
; Entry: X the object. Exit: all registers and flags kept.
MariaCrash0Clear:
    jmp VecClearObject

; @routine 01E3:3E6 MariaCrash0Start
; @run 6E:D3E6
; WHAT IT DOES: State 0 of crash kind 0's object: cleared when she is hit or below the screen; at her crash's frame $52
; it starts script 3 in slot 3, keeps the raster stage (PlayerKeptRasterStage) and sets stage 2, takes a hitbox $40
; square at $20 across and $0C down, live, OBJ_WORK_N $0D, her facing and X, sprite set 1, frame $23, Y speed entry 8,
; and its next state.
; HOW: Her OBJ_STATE, OBJ_Y_HI and OBJ_FRAME; MariaStartScriptSlot3, MariaObjectFromPlayer, MariaSetVelocityYFromTable;
; the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $52; else as the
; routine jumped to.
MariaCrash0Start:
    ldx z:zClimbStepBefore
    lda a:OBJ_STATE
    cmp #$09
    beq MariaCrash0Clear
    lda a:OBJ_Y_HI
    dec a
    beq MariaCrash0Clear
    lda a:OBJ_FRAME
    cmp #$52
    bne MariaCrash0Start_at0
    lda #$03
    jsr MariaStartScriptSlot3
    lda z:zRasterStage
    sta a:PlayerKeptRasterStage
    lda #$02
    sta z:zRasterStage
    lda #$20
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$20
    sta a:OBJ_HITBOX_HALF_W,x
    sta a:OBJ_HITBOX_DX,x
    lda #$0C
    sta a:OBJ_HITBOX_DY,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$0D
    sta a:OBJ_WORK_N,x
    jsr MariaObjectFromPlayer
    lda a:OBJ_X_LO
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$23
    sta a:OBJ_FRAME,x
    lda #$08
    jsr MariaSetVelocityYFromTable
    jmp MariaObjectNextState
MariaCrash0Start_at0:
    rts

; @routine 01E3:442 MariaCrash0Rise
; @run 6E:D442
; WHAT IT DOES: State 1 of crash kind 0's object: it moves; released at the meeting (zFieldD2 2), it hovers: frame $22,
; Y speed entry 9, $20 frames, its next state. Within 8 rows of her on the first screen it marks the meeting; when she
; is hit or below the screen, or it is not near her, it is cleared once it is high on the screen below (Y under $C0 on
; screen 1).
; HOW: MariaObjectApplyVelocity; her OBJ_STATE and OBJ_Y_HI; zFieldD2; the Y difference plus 8 against $10; the tail
; JMPs to MariaObjectNextState and ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last test, or the routine jumped to.
MariaCrash0Rise:
    ldx z:zClimbStepBefore
    jsr MariaObjectApplyVelocity
    lda a:OBJ_STATE
    cmp #$09
    beq MariaCrash0Rise_to1
    lda a:OBJ_Y_HI
    dec a
    beq MariaCrash0Rise_to1
    lda z:zFieldD2
    cmp #$02
    beq MariaCrash0Rise_at0
    lda a:OBJ_Y_HI,x
    bne MariaCrash0Rise_to1
    lda a:OBJ_Y_LO,x
    sec
    sbc a:OBJ_Y_LO
    clc
    adc #$08
    cmp #$10
    bcs MariaCrash0Rise_to1
    lda #$01
    sta z:zFieldD2
    rts
MariaCrash0Rise_to1:
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne MariaCrash0Rise_at1
    lda a:OBJ_Y_LO,x
    cmp #$C0
    bcs MariaCrash0Rise_at1
    jmp VecClearObject
MariaCrash0Rise_at1:
    rts
MariaCrash0Rise_at0:
    lda #$22
    sta a:OBJ_FRAME,x
    stz z:zFieldD2
    lda #$09
    jsr MariaSetVelocityYFromTable
    lda #$20
    sta a:OBJ_ANIM_TIMER,x
    jmp MariaObjectNextState

; @routine 01E3:498 MariaCrash0Hover
; @run 6E:D498
; WHAT IT DOES: State 2 of crash kind 0's object: it moves, no higher than the top of the map; when its timer runs
; out it stops, sets $80 frames of rain with sound $34, and goes to its next state.
; HOW: MariaObjectApplyVelocity; DEC of OBJ_ANIM_TIMER; MariaObjectStop, SoundCommand, the tail JMP to
; MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the timer's DEC, or MariaObjectNextState.
MariaCrash0Hover:
    ldx z:zClimbStepBefore
    jsr MariaObjectApplyVelocity
    lda a:OBJ_Y_HI,x
    cmp #$FF
    bne MariaCrash0Hover_at0
    stz a:OBJ_Y_LO,x
    stz a:OBJ_Y_HI,x
MariaCrash0Hover_at0:
    dec a:OBJ_ANIM_TIMER,x
    beq MariaCrash0Hover_at1
    rts
MariaCrash0Hover_at1:
    jsr MariaObjectStop
    lda #$80
    sta a:OBJ_ANIM_TIMER,x
    lda #$34
    jsr VecSoundCommand
    jmp MariaObjectNextState

; @routine 01E3:4C0 MariaCrash0Rain
; @run 6E:D4C0
; WHAT IT DOES: State 3 of crash kind 0's object: it moves; every $20 frames script 3 starts again; each frame a piece
; may drop (MariaCrash0Drop). When the rain's timer runs out it rises (Y speed entry $0A), marks the meeting again
; (zFieldD2 1), shows frame $23 and goes to its next state.
; HOW: MariaObjectApplyVelocity; DEC of OBJ_ANIM_TIMER and its low five bits; MariaStartScriptSlot3; the JMP to
; MariaCrash0Drop; the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as MariaCrash0Drop, or MariaObjectNextState.
MariaCrash0Rain:
    ldx z:zClimbStepBefore
    jsr MariaObjectApplyVelocity
    dec a:OBJ_ANIM_TIMER,x
    beq MariaCrash0Rain_at0
    lda a:OBJ_ANIM_TIMER,x
    and #$1F
    cmp #$1F
    bne MariaCrash0Rain_at1
    lda #$03
    jsr MariaStartScriptSlot3
MariaCrash0Rain_at1:
    jmp MariaCrash0Drop
MariaCrash0Rain_at0:
    lda #$0A
    jsr MariaSetVelocityYFromTable
    lda #$01
    sta z:zFieldD2
    lda #$23
    sta a:OBJ_FRAME,x
    jmp MariaObjectNextState

; @routine 01E3:4EC MariaCrash0Leave
; @run 6E:D4EC
; WHAT IT DOES: State 4 of crash kind 0's object: it moves; above the top of the map it puts the raster stage back
; (PlayerKeptRasterStage) and is cleared.
; HOW: MariaObjectMove; the tail JMP to ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the compare, or ClearObject.
MariaCrash0Leave:
    ldx z:zClimbStepBefore
    jsr MariaObjectMove
    lda a:OBJ_Y_HI,x
    cmp #$FF
    beq MariaCrash0Leave_at0
    rts
MariaCrash0Leave_at0:
    lda a:PlayerKeptRasterStage
    sta z:zRasterStage
    jmp VecClearObject

; @routine 01E3:501 MariaCrash0Drop
; @run 6E:D501
; WHAT IT DOES: On even frames, drops a piece of kind 21 into the next of slots 6 to 14 (OBJ_PIECE_NEXT) if it is
; empty: at the place OBJ_PIECE_PLACE picks in MariaCrash0PiecePlaces (Y, and X plus the frame count's low three
; bits), sprite set 1, frame $31, X speed entry 4 the way it faces, Y speed entry $0B, OBJ_WORK_N $0B. The place
; steps on through sixteen, the slot through nine.
; HOW: zWaveCounter; the piece's slot in X by SXY while the object's is in Y and on the stack; the velocity tables and
; MariaObjectFacingVelocity for the piece.
; WAITS: no.
; Entry: X the object. Exit: odd frame, A 1, Z clear; slot taken, A its kind, Z clear; else A the next slot, N, Z
; and C from comparing it with 9 (cleared at 9). Y as the last step left it.
MariaCrash0Drop:
    lda z:zWaveCounter
    and #$01
    beq MariaCrash0Drop_at0
    rts
MariaCrash0Drop_at0:
    lda a:OBJ_PIECE_NEXT,x
    clc
    adc #$06
    tay
    lda a:OBJ_KIND,y
    beq MariaCrash0Drop_at1
    rts
MariaCrash0Drop_at1:
    lda #$15
    sta a:OBJ_KIND,y
    lda a:OBJ_PIECE_PLACE,x
    phx
    sxy
    asl a
    asl a
    tay
    lda a:MariaCrash0PiecePlaces,y
    sta a:OBJ_Y_LO,x
    lda a:MariaCrash0PiecePlaces+1,y
    sta a:OBJ_Y_HI,x
    lda a:MariaCrash0PiecePlaces+2,y
    sta z:zTemp02
    lda a:MariaCrash0PiecePlaces+3,y
    sta z:zTemp03
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp02
    sta a:OBJ_X_LO,x
    lda z:zTemp03
    sta a:OBJ_X_HI,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$31
    sta a:OBJ_FRAME,x
    lda #$04
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$0B
    jsr MariaSetVelocityYFromTable
    lda #$0B
    sta a:OBJ_WORK_N,x
    plx
    lda a:OBJ_PIECE_PLACE,x
    inc a
    and #$0F
    sta a:OBJ_PIECE_PLACE,x
    lda a:OBJ_PIECE_NEXT,x
    inc a
    cmp #$09
    sta a:OBJ_PIECE_NEXT,x
    bcc MariaCrash0Drop_at2
    stz a:OBJ_PIECE_NEXT,x
MariaCrash0Drop_at2:
    rts

; @routine 01E3:5C0 MariaCrash0Piece
; @run 6E:D5C0
; WHAT IT DOES: The handler of kind 21, a piece crash kind 0's object drops: a hitbox $C0 by $E0 at $70 across and $0C
; down, its frame by the frame count and its slot's low bit (MariaCrash0PieceFrames); it moves; below the screen it is
; cleared; its hitbox is live one frame in eight.
; HOW: The stores, MariaObjectMove, zWaveCounter; INC of OBJ_HITBOX_ON; the tail JMP to ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: A the frame count's low three bits, N and Z from them; at the eighth, as the
; INC; or as ClearObject.
MariaCrash0Piece:
    ldx z:zClimbStepBefore
    lda #$60
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$70
    sta a:OBJ_HITBOX_HALF_W,x
    sta a:OBJ_HITBOX_DX,x
    lda #$0C
    sta a:OBJ_HITBOX_DY,x
    txa
    and #$01
    eor z:zWaveCounter
    and #$03
    tay
    lda a:MariaCrash0PieceFrames,y
    sta a:OBJ_FRAME,x
    jsr MariaObjectMove
    lda a:OBJ_Y_HI,x
    cmp #$01
    beq MariaCrash0Piece_at0
    stz a:OBJ_HITBOX_ON,x
    lda z:zWaveCounter
    and #$07
    beq MariaCrash0Piece_at1
    rts
MariaCrash0Piece_at1:
    inc a:OBJ_HITBOX_ON,x
    rts
MariaCrash0Piece_at0:
    jmp VecClearObject
