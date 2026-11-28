; @routine 072C:3D9 RaiseColumn072C
; @run 70:BBD9
; WHAT IT DOES: After OBJ_WORK_J: placed under slot 0, the parts in slots $11-$26 spread down 16 apart, then launched upward.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RaiseColumn072C:
    dec a:OBJ_WORK_J,x
    bne RaiseColumn072C_at0
    lda #$8A
    jsr VecSoundCommand
    ldy #$00
    jsr VecCopyObjectPlaceBack
    lda #$01
    sta a:OBJ_Y_HI,x
    lda #$40
    sta a:OBJ_Y_LO,x
    jsr StartPieceChain
    lda #$10
    sta z:zTemp00
    stz z:zTemp01
    ldy #$11
RaiseColumn072C_at1:
    lda a:OBJ_Y_LO,y
    clc
    adc z:zTemp00
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_HI,y
    adc z:zTemp01
    sta a:OBJ_Y_HI,y
    lda z:zTemp00
    clc
    adc #$10
    sta z:zTemp00
    lda z:zTemp01
    adc #$00
    sta z:zTemp01
    iny
    cpy #$27
    bcc RaiseColumn072C_at1
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$F6
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
RaiseColumn072C_at0:
    rts

; @routine 0EBD:000 ShatterEleven0EBD
; @run 71:C000
; WHAT IT DOES: Clears the neighbouring parts and scatters 11 pieces with their waits and speeds, then becomes kind $9C.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ShatterEleven0EBDPieces = $C065 ; six bytes per piece
ShatterEleven0EBD:
    txa
    tay
    iny
    jsr VecClearObjectInY
    dey
    dey
    jsr VecClearObjectInY
    cly
ShatterEleven0EBD_at1:
    sty z:zBurstCount
    jsr ClearFirstEmptySlot0EBD
    bcc ShatterEleven0EBD_at0
    lda #$37
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    stx z:zClimbStepBefore
    lda z:zBurstCount
    asl a
    sta z:zTemp00
    asl a
    clc
    adc z:zTemp00
    tax
    lda a:ShatterEleven0EBDPieces,x
    sta a:OBJ_WORK_D,y
    lda a:ShatterEleven0EBDPieces+1,x
    sta a:OBJ_WORK_E,y
    lda a:ShatterEleven0EBDPieces+2,x
    sta a:OBJ_VX_HI,y
    lda a:ShatterEleven0EBDPieces+3,x
    sta a:OBJ_VX_LO,y
    lda a:ShatterEleven0EBDPieces+4,x
    sta a:OBJ_VY_HI,y
    lda a:ShatterEleven0EBDPieces+5,x
    sta a:OBJ_VY_LO,y
    ldx z:zClimbStepBefore
    ldy z:zBurstCount
    iny
    cpy #$0B
    bcc ShatterEleven0EBD_at1
ShatterEleven0EBD_at0:
    lda #$30
    sta a:OBJ_WORK_L+$11
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts

; @routine 0EBD:78F PlayerRidesBell0EBD
; @run 71:C78F
; WHAT IT DOES: Under it, pressing down: the player is held while its animation rings (sound $3B).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PlayerRidesBell0EBD:
    lda a:OBJ_STATE,x
    bne PlayerRidesBell0EBD_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    rts
PlayerRidesBell0EBD_at0:
    lda a:OBJ_SUBSTATE,x
    bne PlayerRidesBell0EBD_at1
    lda a:OBJ_STATE
    bne PlayerRidesBell0EBD_at2
    lda a:OBJ_X_LO
    sec
    sbc a:OBJ_X_LO,x
    clc
    adc #$0C
    cmp #$18
    bcs PlayerRidesBell0EBD_at2
    lda z:zPadPressed
    and #$10
    beq PlayerRidesBell0EBD_at2
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_X_LO
    sta a:OBJ_X_LO,x
    lda a:OBJ_Y_LO
    clc
    adc #$D8
    sta a:OBJ_Y_LO,x
PlayerRidesBell0EBD_at2:
    rts
PlayerRidesBell0EBD_at1:
    lda #$C0
    sta a:OBJ_WORK_E
    lda #$10
    sta a:OBJ_WORK_F
    lda #$80
    sta z:zTemp10
    lda #$C7
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne PlayerRidesBell0EBD_at3
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_FRAME,x
    stz a:OBJ_WORK_E
    rts
PlayerRidesBell0EBD_at3:
    bcc PlayerRidesBell0EBD_at4
    lda #$3B
    jsr VecSoundCommand
PlayerRidesBell0EBD_at4:
    rts

; @routine 0EBF:308 SwoopAndClimb0EBF
; @run 71:D308
; WHAT IT DOES: Swoops toward the player: first falling (turning frame while moving the other way), then climbing to Y OBJ_WORK_J.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwoopAndClimb0EBFSteps = $D36F ; vx step by side
SwoopAndClimb0EBFStepD373 = $D373 ; a routine of this overlay
SwoopAndClimb0EBFStepD398 = $D398 ; a routine of this overlay
SwoopAndClimb0EBFStepD3A8 = $D3A8 ; a routine of this overlay
SwoopAndClimb0EBF:
    jsr VecApplyVelocity
    jsr VecSideOfPlayer
    and #$01
    sta z:zTemp00
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:SwoopAndClimb0EBFSteps,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:SwoopAndClimb0EBFSteps+1,y
    sta a:OBJ_VX_HI,x
    jsr SwoopAndClimb0EBFStepD373
    lda a:OBJ_VY_LO,x
    sec
    sbc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    bmi SwoopAndClimb0EBF_at0
    ldy #$04
    jsr FaceAndAnimate0EBF
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor z:zTemp00
    beq SwoopAndClimb0EBF_at1
    lda #$4F
    sta a:OBJ_FRAME,x
    rts
SwoopAndClimb0EBF_at0:
    jsr SwoopAndClimb0EBFStepD398
    ldy #$04
    jsr FaceAndAnimate0EBF
    jsr SwoopAndClimb0EBFStepD3A8
    lda a:OBJ_Y_HI,x
    bne SwoopAndClimb0EBF_at1
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_WORK_J,x
    bcs SwoopAndClimb0EBF_at1
    inc a:OBJ_SUBSTATE,x
SwoopAndClimb0EBF_at1:
    rts

; @routine 0EC0:2F7 HoverThenSwoop0EC0
; @run 71:DAF7
; WHAT IT DOES: Bobs along; near the player, now and then swoops at one of eight speeds (state on).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
HoverThenSwoop0EC0Swoops = $DB66 ; vx and vy, eight of four bytes
HoverThenSwoop0EC0:
    lda #$DB
    sta z:zTemp11
    lda #$5D
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$40
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:zProduct1
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    inc a:OBJ_WORK_D,x
    jsr VecApplyVelocityX
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs HoverThenSwoop0EC0_at0
    jsr VecNextRandom
    cmp #$02
    bcs HoverThenSwoop0EC0_at0
    jsr VecNextRandom
    and #$07
    asl a
    asl a
    tay
    lda a:HoverThenSwoop0EC0Swoops,y
    sta a:OBJ_VX_LO,x
    lda a:HoverThenSwoop0EC0Swoops+1,y
    sta a:OBJ_VX_HI,x
    lda a:HoverThenSwoop0EC0Swoops+2,y
    sta a:OBJ_VY_LO,x
    lda a:HoverThenSwoop0EC0Swoops+3,y
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT,x
    beq HoverThenSwoop0EC0_at1
    jsr VecNegateVelocityX
HoverThenSwoop0EC0_at1:
    inc a:OBJ_STATE,x
HoverThenSwoop0EC0_at0:
    rts

; @routine 0EC0:4BF StepOffThenWake0EC0
; @run 71:DCBF
; WHAT IT DOES: Steps forward four times at frame 3 of its list, slowing; then, when the player's flags allow, wakes.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StepOffThenWake0EC0:
    lda a:OBJ_WORK_F,x
    cmp #$04
    bcs StepOffThenWake0EC0_at0
    lda #$DD
    sta z:zTemp11
    lda #$25
    sta z:zTemp10
    jsr VecStepObjectFrames
    bcc StepOffThenWake0EC0_at0
    lda a:OBJ_FRAME,x
    cmp #$03
    bne StepOffThenWake0EC0_at0
    inc a:OBJ_WORK_F,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$70
    sta a:OBJ_VX_LO,x
    bra StepOffThenWake0EC0_at1
StepOffThenWake0EC0_at0:
    lda a:OBJ_VX_LO,x
    clc
    adc #$FC
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc #$FF
    sta a:OBJ_VX_HI,x
    bpl StepOffThenWake0EC0_at1
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda a:OBJ_WORK_F,x
    cmp #$04
    bne StepOffThenWake0EC0_at1
    lda a:RichterColumnHit
    ora a:RichterColumnHit+1
    ora a:RichterColumnHit+2
    and #$01
    beq StepOffThenWake0EC0_at1
    lda a:OBJ_LOOP_FLAGS,x
    and #$FE
    sta a:OBJ_LOOP_FLAGS,x
    inc a:OBJ_STATE,x
StepOffThenWake0EC0_at1:
    jsr VecApplyVelocityX
    rts

; @routine 101D:4E8 OrbitSlot1F_101D
; @run 71:C4E8
; WHAT IT DOES: Orbits slot $1F at a widening radius until a hard hit; then state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
OrbitSlot1F_101D:
    lda a:OBJ_WORK_E,x
    cmp #$78
    bcs OrbitSlot1F_101D_at0
    lda a:OBJ_WORK_F,x
    clc
    adc #$40
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    adc #$00
    sta a:OBJ_WORK_E,x
OrbitSlot1F_101D_at0:
    lda #$4D
    sta z:zTemp10
    lda #$C5
    sta z:zTemp11
    jsr VecStepObjectFrames
    ldy #$1F
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_WORK_F,x
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_VY_HI,x
    asl a
    ror a:OBJ_VY_HI,x
    ror a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    sec
    sbc #$01
    sta a:OBJ_WORK_D,x
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc OrbitSlot1F_101D_at1
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
OrbitSlot1F_101D_at1:
    rts

; @routine 117B:761 SwoopAndClimb117B
; @run 70:B761
; WHAT IT DOES: Swoops toward the player: first falling (turning frame while moving the other way), then climbing to Y OBJ_WORK_J.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwoopAndClimb117BSteps = $B7C8 ; vx step by side
SwoopAndClimb117BStepB7CC = $B7CC ; a routine of this overlay
SwoopAndClimb117BStepB7F1 = $B7F1 ; a routine of this overlay
SwoopAndClimb117B:
    jsr VecApplyVelocity
    jsr VecSideOfPlayer
    and #$01
    sta z:zTemp00
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:SwoopAndClimb117BSteps,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:SwoopAndClimb117BSteps+1,y
    sta a:OBJ_VX_HI,x
    jsr SwoopAndClimb117BStepB7CC
    lda a:OBJ_VY_LO,x
    sec
    sbc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    bmi SwoopAndClimb117B_at0
    ldy #$04
    jsr StepFramesFacingPlayerD
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor z:zTemp00
    beq SwoopAndClimb117B_at1
    lda #$04
    sta a:OBJ_FRAME,x
    rts
SwoopAndClimb117B_at0:
    jsr SwoopAndClimb117BStepB7F1
    ldy #$04
    jsr StepFramesFacingPlayerD
    jsr SoundEveryFourthFrameC
    lda a:OBJ_Y_HI,x
    bne SwoopAndClimb117B_at1
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_WORK_J,x
    bcs SwoopAndClimb117B_at1
    inc a:OBJ_SUBSTATE,x
SwoopAndClimb117B_at1:
    rts

; @routine 117D:1E4 ShatterEight117D
; @run 71:C1E4
; WHAT IT DOES: Clears the neighbouring parts and scatters 8 pieces with their waits and speeds, then becomes kind $9C.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ShatterEight117DPieces = $C247 ; six bytes per piece
ShatterEight117D:
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda a:OBJ_Y_HI,x
    beq ShatterEight117D_at0
    jmp VecClearObject
ShatterEight117D_at0:
    cly
ShatterEight117D_at2:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc ShatterEight117D_at1
    lda #$2D
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    stx z:zClimbStepBefore
    lda z:zBurstCount
    asl a
    sta z:zTemp00
    asl a
    clc
    adc z:zTemp00
    tax
    lda a:ShatterEight117DPieces,x
    sta a:OBJ_WORK_D,y
    lda a:ShatterEight117DPieces+1,x
    sta a:OBJ_WORK_E,y
    lda a:ShatterEight117DPieces+2,x
    sta a:OBJ_VX_HI,y
    lda a:ShatterEight117DPieces+3,x
    sta a:OBJ_VX_LO,y
    lda a:ShatterEight117DPieces+4,x
    sta a:OBJ_VY_HI,y
    lda a:ShatterEight117DPieces+5,x
    sta a:OBJ_VY_LO,y
    ldx z:zClimbStepBefore
    ldy z:zBurstCount
    iny
    cpy #$08
    bcc ShatterEight117D_at2
ShatterEight117D_at1:
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts

; @routine 161D:7AA SlideWhileList161D
; @run 71:C7AA
; WHAT IT DOES: Slides during steps 9-13 (sound at 9), speeding by facing until a wall; at the list's end state 3.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SlideWhileList161DSteps = $C7A6 ; vx step by facing
SlideWhileList161D:
    lda #$0C
    sta z:zTemp10
    lda #$C8
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq SlideWhileList161D_at0
    lda a:OBJ_ANIM_STEP,x
    cmp #$09
    bcc SlideWhileList161D_at1
    cmp #$0E
    bcs SlideWhileList161D_at1
    cmp #$09
    bne SlideWhileList161D_at2
    lda a:OBJ_ANIM_TIMER,x
    cmp #$10
    bne SlideWhileList161D_at2
    lda #$1F
    jsr VecSoundIfOnFirstScreen
SlideWhileList161D_at2:
    lda a:OBJ_VX_HI,x
    ora a:OBJ_VX_LO,x
    beq SlideWhileList161D_at1
    lda #$0C
    sta z:zProduct1
    jsr BounceAtWalls
    bcc SlideWhileList161D_at3
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    rts
SlideWhileList161D_at3:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:SlideWhileList161DSteps,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:SlideWhileList161DSteps+1,y
    sta a:OBJ_VX_HI,x
    rts
SlideWhileList161D_at0:
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SlideWhileList161D_at1:
    rts

; @routine 161E:650 BobbingLift161E
; @run 71:CE50
; WHAT IT DOES: A lift that carries the player, swinging OBJ_WORK_E between 0 and $A0 and turning at each end.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BobbingLift161ESpeeds = $CEB4 ; vy by direction
BobbingLift161E:
    lda a:OBJ_STATE,x
    bne BobbingLift161E_at0
    jsr VecLoadObjectRecord
    lda #$50
    sta a:OBJ_WORK_E,x
BobbingLift161E_at5:
    ldy a:OBJ_WORK_D,x
    lda a:BobbingLift161ESpeeds,y
    sta a:OBJ_VY_LO,x
    lda a:BobbingLift161ESpeeds+1,y
    sta a:OBJ_VY_HI,x
BobbingLift161E_at4:
    rts
BobbingLift161E_at0:
    jsr VecApplyVelocityY
    jsr PushWithin161E
    lda a:OBJ_START_ARG,x
    beq BobbingLift161E_at1
    jsr StepList161F_03A
    bra BobbingLift161E_at2
BobbingLift161E_at1:
    jsr StepList161F_052
BobbingLift161E_at2:
    lda a:OBJ_WORK_D,x
    beq BobbingLift161E_at3
    lda a:OBJ_WORK_F,x
    eor #$80
    sta a:OBJ_WORK_F,x
    beq BobbingLift161E_at4
    inc a:OBJ_WORK_E,x
    lda a:OBJ_WORK_E,x
    cmp #$A0
    bne BobbingLift161E_at4
    stz a:OBJ_WORK_D,x
    bra BobbingLift161E_at5
BobbingLift161E_at3:
    lda a:OBJ_WORK_F,x
    eor #$80
    sta a:OBJ_WORK_F,x
    beq BobbingLift161E_at4
    dec a:OBJ_WORK_E,x
    bne BobbingLift161E_at4
    lda #$02
    sta a:OBJ_WORK_D,x
    bra BobbingLift161E_at5
