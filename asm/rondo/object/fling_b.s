; @routine 05CB:02A FlungObject05CB
; @run 70:B02A
; WHAT IT DOES: An object flung from another (Secret stage 02): cleared unless the slot it was flung from holds
; kind $22; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $B049; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject05CBStates = $B049 ; the handlers by state
FlungObject05CB:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$22
    beq FlungObject05CB_at0
    jmp VecClearObject
FlungObject05CB_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject05CBStates,y
    sta z:zTemp00
    lda a:FlungObject05CBStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 05CB:051 FlungObjectLaunch05CB
; @run 70:B051
; WHAT IT DOES: Launches a flung object (Secret stage 02): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B0A9 and $B0B1.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch05CBSpeedsX = $B0A9 ; four X speeds, low byte then high
FlungObjectLaunch05CBSpeedsY = $B0B1 ; four Y speeds, low byte then high
FlungObjectLaunch05CB:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch05CBSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch05CBSpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch05CBSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch05CBSpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 05CB:0B9 FlungObjectWait
; @run 70:B0B9
; WHAT IT DOES: A flung object waits: its wait (OBJ_FLING_WAIT) is counted down, and at zero it goes to its next state.
; HOW: DEC, and INC of the state when it reaches zero. The same nine bytes follow every copy of the launch.
; WAITS: no.
; Entry: X object. Exit: N and Z from the wait left, or from the new state; A, X, Y, C and V kept.
FlungObjectWait:
    dec a:OBJ_FLING_WAIT,x
    bne FlungObjectWait_at0
    inc a:OBJ_STATE,x
FlungObjectWait_at0:
    rts

; @routine 05CB:0C2 FlungObjectFly05CB
; @run 70:B0C2
; WHAT IT DOES: A flung object flies (Secret stage 02): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $B0D8 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly05CB:
    jsr VecApplyVelocity
    lda #$B0
    sta z:zTemp11
    lda #$D8
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly05CB_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly05CB_at0:
    rts

; @routine 072B:05A FlungObject072B
; @run 70:B05A
; WHAT IT DOES: An object flung from another (Secret stage 03): cleared unless the slot it was flung from holds
; kind $22; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $B079; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject072BStates = $B079 ; the handlers by state
FlungObject072B:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$22
    beq FlungObject072B_at0
    jmp VecClearObject
FlungObject072B_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject072BStates,y
    sta z:zTemp00
    lda a:FlungObject072BStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 072B:0FA FlungObjectFly072B
; @run 70:B0FA
; WHAT IT DOES: A flung object flies (Secret stage 03): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $B110 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly072B:
    jsr VecApplyVelocity
    lda #$B1
    sta z:zTemp11
    lda #$10
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly072B_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly072B_at0:
    rts

; @routine 088D:166 FlungObject088D
; @run 71:C166
; WHAT IT DOES: An object flung from another (Secret stage 04): cleared unless the slot it was flung from holds
; kind $2D; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $C185; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject088DStates = $C185 ; the handlers by state
FlungObject088D:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$2D
    beq FlungObject088D_at0
    jmp VecClearObject
FlungObject088D_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject088DStates,y
    sta z:zTemp00
    lda a:FlungObject088DStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088D:200 FlungObjectFly088D
; @run 71:C200
; WHAT IT DOES: A flung object flies (Secret stage 04): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $C216 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly088D:
    jsr VecApplyVelocity
    lda #$C2
    sta z:zTemp11
    lda #$16
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly088D_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly088D_at0:
    rts

; @routine 09EC:6AF FlungObject09EC
; @run 70:BEAF
; WHAT IT DOES: An object flung from another (Secret stage 05): cleared unless the slot it was flung from holds
; kind $31; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $BECE; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject09ECStates = $BECE ; the handlers by state
FlungObject09EC:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$31
    beq FlungObject09EC_at0
    jmp VecClearObject
FlungObject09EC_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject09ECStates,y
    sta z:zTemp00
    lda a:FlungObject09ECStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 09EC:6D6 FlungObjectLaunch09EC
; @run 70:BED6
; WHAT IT DOES: Launches a flung object (Secret stage 05): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $BF2E and $BF36.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch09ECSpeedsX = $BF2E ; four X speeds, low byte then high
FlungObjectLaunch09ECSpeedsY = $BF36 ; four Y speeds, low byte then high
FlungObjectLaunch09EC:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch09ECSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch09ECSpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch09ECSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch09ECSpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 09EC:747 FlungObjectFly09EC
; @run 70:BF47
; WHAT IT DOES: A flung object flies (Secret stage 05): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $BF5D through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly09EC:
    jsr VecApplyVelocity
    lda #$BF
    sta z:zTemp11
    lda #$5D
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly09EC_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly09EC_at0:
    rts

; @routine 0B4B:2B5 FlungObject0B4B
; @run 70:B2B5
; WHAT IT DOES: An object flung from another (Secret stage 06): cleared unless the slot it was flung from holds
; kind $21; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $B2D4; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject0B4BStates = $B2D4 ; the handlers by state
FlungObject0B4B:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$21
    beq FlungObject0B4B_at0
    jmp VecClearObject
FlungObject0B4B_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject0B4BStates,y
    sta z:zTemp00
    lda a:FlungObject0B4BStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4B:2DC FlungObjectLaunch0B4B
; @run 70:B2DC
; WHAT IT DOES: Launches a flung object (Secret stage 06): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B334 and $B33C.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch0B4BSpeedsX = $B334 ; four X speeds, low byte then high
FlungObjectLaunch0B4BSpeedsY = $B33C ; four Y speeds, low byte then high
FlungObjectLaunch0B4B:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch0B4BSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch0B4BSpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch0B4BSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch0B4BSpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 0B4B:34D FlungObjectFly0B4B
; @run 70:B34D
; WHAT IT DOES: A flung object flies (Secret stage 06): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $B363 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly0B4B:
    jsr VecApplyVelocity
    lda #$B3
    sta z:zTemp11
    lda #$63
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly0B4B_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly0B4B_at0:
    rts

; @routine 0CAB:076 FlungObject0CAB
; @run 70:B076
; WHAT IT DOES: An object flung from another (Secret stage 07): cleared unless the slot it was flung from holds
; kind $2E; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $B095; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject0CABStates = $B095 ; the handlers by state
FlungObject0CAB:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$2E
    beq FlungObject0CAB_at0
    jmp VecClearObject
FlungObject0CAB_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject0CABStates,y
    sta z:zTemp00
    lda a:FlungObject0CABStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAB:09D FlungObjectLaunch0CAB
; @run 70:B09D
; WHAT IT DOES: Launches a flung object (Secret stage 07): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B0F5 and $B0FD.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch0CABSpeedsX = $B0F5 ; four X speeds, low byte then high
FlungObjectLaunch0CABSpeedsY = $B0FD ; four Y speeds, low byte then high
FlungObjectLaunch0CAB:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch0CABSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch0CABSpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch0CABSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch0CABSpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 0CAB:10E FlungObjectFly0CAB
; @run 70:B10E
; WHAT IT DOES: A flung object flies (Secret stage 07): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $B124 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly0CAB:
    jsr VecApplyVelocity
    lda #$B1
    sta z:zTemp11
    lda #$24
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly0CAB_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly0CAB_at0:
    rts

; @routine 0E0A:63A FlungObject0E0A
; @run 70:AE3A
; WHAT IT DOES: An object flung from another (Secret stage 08): cleared unless the slot it was flung from holds
; kind $21; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $AE59; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject0E0AStates = $AE59 ; the handlers by state
FlungObject0E0A:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$21
    beq FlungObject0E0A_at0
    jmp VecClearObject
FlungObject0E0A_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject0E0AStates,y
    sta z:zTemp00
    lda a:FlungObject0E0AStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0E0A:661 FlungObjectLaunch0E0A
; @run 70:AE61
; WHAT IT DOES: Launches a flung object (Secret stage 08): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $AEB9 and $AEC1.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch0E0ASpeedsX = $AEB9 ; four X speeds, low byte then high
FlungObjectLaunch0E0ASpeedsY = $AEC1 ; four Y speeds, low byte then high
FlungObjectLaunch0E0A:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch0E0ASpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch0E0ASpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch0E0ASpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch0E0ASpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 0E0A:6D2 FlungObjectFly0E0A
; @run 70:AED2
; WHAT IT DOES: A flung object flies (Secret stage 08): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $AEE8 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly0E0A:
    jsr VecApplyVelocity
    lda #$AE
    sta z:zTemp11
    lda #$E8
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly0E0A_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly0E0A_at0:
    rts

; @routine 0F6A:766 FlungObject0F6A
; @run 70:AF66
; WHAT IT DOES: An object flung from another (Secret stage 09): cleared unless the slot it was flung from holds
; kind $22; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $AF85; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject0F6AStates = $AF85 ; the handlers by state
FlungObject0F6A:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$22
    beq FlungObject0F6A_at0
    jmp VecClearObject
FlungObject0F6A_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject0F6AStates,y
    sta z:zTemp00
    lda a:FlungObject0F6AStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0F6A:78D FlungObjectLaunch0F6A
; @run 70:AF8D
; WHAT IT DOES: Launches a flung object (Secret stage 09): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $AFE5 and $AFED.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch0F6ASpeedsX = $AFE5 ; four X speeds, low byte then high
FlungObjectLaunch0F6ASpeedsY = $AFED ; four Y speeds, low byte then high
FlungObjectLaunch0F6A:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch0F6ASpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch0F6ASpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch0F6ASpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch0F6ASpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 0F6A:7FE FlungObjectFly0F6A
; @run 70:AFFE
; WHAT IT DOES: A flung object flies (Secret stage 09): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $B014 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly0F6A:
    jsr VecApplyVelocity
    lda #$B0
    sta z:zTemp11
    lda #$14
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly0F6A_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly0F6A_at0:
    rts

; @routine 10CB:0C2 FlungObject10CB
; @run 70:B0C2
; WHAT IT DOES: An object flung from another (Secret stage 0A): cleared unless the slot it was flung from holds
; kind $29; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $B0E1; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject10CBStates = $B0E1 ; the handlers by state
FlungObject10CB:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$29
    beq FlungObject10CB_at0
    jmp VecClearObject
FlungObject10CB_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject10CBStates,y
    sta z:zTemp00
    lda a:FlungObject10CBStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 10CB:0E9 FlungObjectLaunch10CB
; @run 70:B0E9
; WHAT IT DOES: Launches a flung object (Secret stage 0A): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B141 and $B149.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch10CBSpeedsX = $B141 ; four X speeds, low byte then high
FlungObjectLaunch10CBSpeedsY = $B149 ; four Y speeds, low byte then high
FlungObjectLaunch10CB:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch10CBSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch10CBSpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch10CBSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch10CBSpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 10CB:15A FlungObjectFly10CB
; @run 70:B15A
; WHAT IT DOES: A flung object flies (Secret stage 0A): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $B170 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly10CB:
    jsr VecApplyVelocity
    lda #$B1
    sta z:zTemp11
    lda #$70
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly10CB_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly10CB_at0:
    rts

; @routine 13FB:084 FlungObject13FB
; @run 70:B084
; WHAT IT DOES: An object flung from another (Secret stage 0C): cleared unless the slot it was flung from holds
; kind $43; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $B0A3; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject13FBStates = $B0A3 ; the handlers by state
FlungObject13FB:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$43
    beq FlungObject13FB_at0
    jmp VecClearObject
FlungObject13FB_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject13FBStates,y
    sta z:zTemp00
    lda a:FlungObject13FBStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FB:0AB FlungObjectLaunch13FB
; @run 70:B0AB
; WHAT IT DOES: Launches a flung object (Secret stage 0C): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B103 and $B10B.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch13FBSpeedsX = $B103 ; four X speeds, low byte then high
FlungObjectLaunch13FBSpeedsY = $B10B ; four Y speeds, low byte then high
FlungObjectLaunch13FB:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch13FBSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch13FBSpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch13FBSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch13FBSpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 13FB:11C FlungObjectFly13FB
; @run 70:B11C
; WHAT IT DOES: A flung object flies (Secret stage 0C): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $B132 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly13FB:
    jsr VecApplyVelocity
    lda #$B1
    sta z:zTemp11
    lda #$32
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly13FB_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly13FB_at0:
    rts

; @routine 16CB:092 FlungObject16CB
; @run 70:B092
; WHAT IT DOES: An object flung from another (Secret stage 0E): cleared unless the slot it was flung from holds
; kind $29; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $B0B1; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject16CBStates = $B0B1 ; the handlers by state
FlungObject16CB:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$29
    beq FlungObject16CB_at0
    jmp VecClearObject
FlungObject16CB_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject16CBStates,y
    sta z:zTemp00
    lda a:FlungObject16CBStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CB:0B9 FlungObjectLaunch16CB
; @run 70:B0B9
; WHAT IT DOES: Launches a flung object (Secret stage 0E): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B111 and $B119.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch16CBSpeedsX = $B111 ; four X speeds, low byte then high
FlungObjectLaunch16CBSpeedsY = $B119 ; four Y speeds, low byte then high
FlungObjectLaunch16CB:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch16CBSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch16CBSpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch16CBSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch16CBSpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 16CB:12A FlungObjectFly16CB
; @run 70:B12A
; WHAT IT DOES: A flung object flies (Secret stage 0E): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $B140 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly16CB:
    jsr VecApplyVelocity
    lda #$B1
    sta z:zTemp11
    lda #$40
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly16CB_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly16CB_at0:
    rts

; @routine 18DB:09A FlungObject18DB
; @run 70:B09A
; WHAT IT DOES: An object flung from another (Stage 10): cleared unless the slot it was flung from holds
; kind $30; else its state's handler runs (load its record, launch, wait, fly).
; HOW: One compare, then JumpByStateTableBody with the table at $B0B9; ClearObject through its vector by a tail JMP.
; WAITS: no.
; Entry: X object. Exit: cleared: A the source's kind, N, Z and C from the compare, Y the source; else as the jump:
; A the handler's high byte, Y the doubled state, C the state's bit 7.
FlungObject18DBStates = $B0B9 ; the handlers by state
FlungObject18DB:
    ldy a:OBJ_FLING_SOURCE,x
    lda a:OBJ_KIND,y
    cmp #$30
    beq FlungObject18DB_at0
    jmp VecClearObject
FlungObject18DB_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlungObject18DBStates,y
    sta z:zTemp00
    lda a:FlungObject18DBStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DB:0C1 FlungObjectLaunch18DB
; @run 70:B0C1
; WHAT IT DOES: Launches a flung object (Stage 10): placed on its source and moved a random step from it,
; given a wait of 1 to 4 frames and one of four speeds on each axis, its animation restarted, to its next state.
; HOW: CopyObjectPlaceBack, five NextRandoms, one ApplyVelocity; the speeds' words at $B119 and $B121.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, Y its index, C 0, V from the last NextRandom, N and Z from the
; new state.
FlungObjectLaunch18DBSpeedsX = $B119 ; four X speeds, low byte then high
FlungObjectLaunch18DBSpeedsY = $B121 ; four Y speeds, low byte then high
FlungObjectLaunch18DB:
    ldy a:OBJ_FLING_SOURCE,x
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$18
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    inc a
    sta a:OBJ_FLING_WAIT,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch18DBSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlungObjectLaunch18DBSpeedsX,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:FlungObjectLaunch18DBSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda a:FlungObjectLaunch18DBSpeedsY,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts

; @routine 18DB:132 FlungObjectFly18DB
; @run 70:B132
; WHAT IT DOES: A flung object flies (Stage 10): it moves and plays its frames; when the list ends it is
; launched again (state 1).
; HOW: ApplyVelocity, then StepObjectFrames on the list at $B148 through zTemp10-11.
; WAITS: no.
; Entry: X object. Exit: list ended: A 1, N and Z from it, C 0; else A $FF, C set when the frame changed; Y as
; StepObjectFrames leaves it, V from ApplyVelocity.
FlungObjectFly18DB:
    jsr VecApplyVelocity
    lda #$B1
    sta z:zTemp11
    lda #$48
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlungObjectFly18DB_at0
    lda #$01
    sta a:OBJ_STATE,x
FlungObjectFly18DB_at0:
    rts
