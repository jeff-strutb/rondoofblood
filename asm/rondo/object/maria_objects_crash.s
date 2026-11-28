; @routine 01E3:273 MariaCrash1Object
; @run 6E:D273
; WHAT IT DOES: The handler of kind 14, crash kind 1's object: its state's routine from MariaCrash1States (start, come
; to her, cross twice, go).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
MariaCrash1Object:
    jsr MariaStateDoubled
    jmp (MariaCrash1States,x)

; @routine 01E3:283 MariaCrash1Clear
; @run 6E:D283
; WHAT IT DOES: Clears crash kind 1's object when she is hit or below the screen.
; HOW: The tail JMP to ClearObject through the kernel's vector.
; WAITS: no.
; Entry: X the object. Exit: all registers and flags kept.
MariaCrash1Clear:
    jmp VecClearObject

; @routine 01E3:286 MariaCrash1Start
; @run 6E:D286
; WHAT IT DOES: State 0 of crash kind 1's object: when she is hit or below the screen it is cleared; it waits for her
; crash's frame $52, then starts script 0 in slot 3, a hitbox $40 by $20, live, OBJ_WORK_N $0D, takes her facing and
; her Y (kept in OBJ_WORK_D) and her X (kept in OBJ_WORK_E), sprite set 1, frame $25, and enters from the side.
; HOW: Her OBJ_STATE, OBJ_Y_HI and OBJ_FRAME; MariaStartScriptSlot3, MariaObjectFromPlayer; on into MariaCrash1Enter.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $52; else as
; MariaCrash1Clear or MariaCrash1Enter.
MariaCrash1Start:
    ldx z:zClimbStepBefore
    lda a:OBJ_STATE
    cmp #$09
    beq MariaCrash1Clear
    lda a:OBJ_Y_HI
    dec a
    beq MariaCrash1Clear
    lda a:OBJ_FRAME
    cmp #$52
    bne MariaCrash1Wait
    cla
    jsr MariaStartScriptSlot3
    lda #$20
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$10
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$00
    sta a:OBJ_HITBOX_DX,x
    lda #$00
    sta a:OBJ_HITBOX_DY,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$0D
    sta a:OBJ_WORK_N,x
    jsr MariaObjectFromPlayer
    lda a:OBJ_Y_LO
    sta a:OBJ_Y_LO,x
    sta a:OBJ_WORK_D,x
    lda a:OBJ_X_LO
    sta a:OBJ_WORK_E,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$25
    sta a:OBJ_FRAME,x

; @routine 01E3:2DA MariaCrash1Enter
; @run 6E:D2DA
; WHAT IT DOES: Crash kind 1's object enters from the side it faces away from: its X at that edge (MariaCrash1Sides),
; its next state, X speed entry 8 the way it faces, and its sound by the way it moves (MariaCrash1Sounds: $39 right,
; $38 left).
; HOW: The facing doubled; MariaObjectNextState, the velocity table, MariaObjectFacingVelocity; bit 7 of the X speed
; rolled into bit 0; the tail JMP to SoundCommand.
; WAITS: no.
; Entry: X the object. Exit: as SoundCommand.
MariaCrash1Enter:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:MariaCrash1Sides,y
    sta a:OBJ_X_LO,x
    lda a:MariaCrash1Sides+1,y
    sta a:OBJ_X_HI,x
    jsr MariaObjectNextState
    lda #$08
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda a:OBJ_VX_HI,x
    asl a
    rol a
    and #$01
    tay
    lda a:MariaCrash1Sounds,y
    jmp VecSoundCommand

; @routine 01E3:304 MariaCrash1Wait
; @run 6E:D304
; WHAT IT DOES: Crash kind 1's object waiting for her crash's frame: nothing more this frame.
; HOW: RTS (the branch from MariaCrash1Start).
; WAITS: no.
; Entry, exit: all registers and flags kept.
MariaCrash1Wait:
    rts

; @routine 01E3:307 MariaCrash1Come
; @run 6E:D307
; WHAT IT DOES: State 1 of crash kind 1's object: it moves toward her; when she is hit or below the screen, or it is on
; another screen or not near her, it is cleared once far off the screen; at the meeting released (zFieldD2 2) it goes
; on (MariaObjectFrame24Next); within $10 of her, it marks the meeting (zFieldD2 1).
; HOW: MariaObjectMove; her OBJ_STATE and OBJ_Y_HI; zFieldD2; the X difference plus $10 against $20; a BSR to
; MariaObjectFarOffScreen and the tail JMP to ClearObject.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: met, A 1; released, as MariaObjectFrame24Next; else as
; MariaObjectFarOffScreen, or ClearObject.
MariaCrash1Come:
    ldx z:zClimbStepBefore
    jsr MariaObjectMove
    lda a:OBJ_STATE
    cmp #$09
    beq MariaCrash1Come_at0
    lda a:OBJ_Y_HI
    dec a
    beq MariaCrash1Come_at0
    lda z:zFieldD2
    cmp #$02
    beq MariaObjectFrame24Next
    lda a:OBJ_X_HI,x
    bne MariaCrash1Come_at0
    lda a:OBJ_X_LO,x
    sec
    sbc a:OBJ_X_LO
    clc
    adc #$10
    cmp #$20
    bcs MariaCrash1Come_at0
    lda #$01
    sta z:zFieldD2
    rts
MariaCrash1Come_at0:
    bsr MariaObjectFarOffScreen
    bne MariaCrash1Come_at1
    rts
MariaCrash1Come_at1:
    jmp VecClearObject

; @routine 01E3:367 MariaCrash1Cross
; @run 6E:D367
; WHAT IT DOES: States 2 and 3 of crash kind 1's object: its frame follows her frame (MariaObjectFrameByPlayerFrame);
; it moves; once far off the screen it turns: its Y mirrored about $68, its facing the other way, and it enters again
; (MariaCrash1Enter).
; HOW: BSRs to MariaObjectFrameByPlayerFrame and MariaObjectFarOffScreen; MariaObjectMove; the JMP to MariaCrash1Enter.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: on screen, as MariaObjectFarOffScreen; else as MariaCrash1Enter.
MariaCrash1Cross:
    ldx z:zClimbStepBefore
    bsr MariaObjectFrameByPlayerFrame
    jsr MariaObjectMove
    bsr MariaObjectFarOffScreen
    bne MariaCrash1Cross_at0
    rts
MariaCrash1Cross_at0:
    lda #$68
    sec
    sbc a:OBJ_Y_LO,x
    clc
    adc #$68
    sta a:OBJ_Y_LO,x
    lda a:OBJ_FACING_LEFT,x
    eor #$FF
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jmp MariaCrash1Enter

; @routine 01E3:39A MariaCrash1Go
; @run 6E:D39A
; WHAT IT DOES: State 4 of crash kind 1's object: it moves with frame $25; until the meeting it follows her frame and,
; within $10 of her on the first screen, marks the meeting (zFieldD2 1) and shows frame $25 again; otherwise it is
; cleared once far off the screen (MariaCrash1Leave).
; HOW: MariaObjectMove, BSRs to MariaObjectFrame25 and MariaObjectFrameByPlayerFrame; the X difference plus $10 against
; $20; on into MariaObjectFrame25, or the branch to MariaCrash1Leave.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: met, as MariaObjectFrame25; else as MariaCrash1Leave.
MariaCrash1Go:
    ldx z:zClimbStepBefore
    jsr MariaObjectMove
    bsr MariaObjectFrame25
    lda z:zFieldD2
    bne MariaCrash1Leave
    bsr MariaObjectFrameByPlayerFrame
    lda a:OBJ_X_HI,x
    bne MariaCrash1Leave
    lda a:OBJ_X_LO,x
    sec
    sbc a:OBJ_X_LO
    clc
    adc #$10
    cmp #$20
    bcs MariaCrash1Leave
    lda #$01
    sta z:zFieldD2

; @routine 01E3:3C4 MariaCrash1Leave
; @run 6E:D3C4
; WHAT IT DOES: Clears crash kind 1's object once it is far off the screen.
; HOW: MariaObjectFarOffScreen's Z; the tail JMP to ClearObject.
; WAITS: no.
; Entry: X the object. Exit: as MariaObjectFarOffScreen, or ClearObject.
MariaCrash1Leave:
    jsr MariaObjectFarOffScreen
    bne MariaCrash1Leave_to1
    rts
MariaCrash1Leave_to1:
    jmp VecClearObject
