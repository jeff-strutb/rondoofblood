; @routine 0EBE:107 SpeedByFacingAndStep0EBE
; @run 71:C907
; WHAT IT DOES: Gives the object the speeds of its facing and OBJ_WORK_D from the tables at $C934, and counts its state on
; (Stage 09's overlay).
; HOW: Loads, adds and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the Y speed's low byte, N and Z from the new state, Y the row.
SpeedByFacingAndStep0EBERows = $C934 ; the first row by facing
SpeedByFacingAndStep0EBESpeeds = $C936 ; X then Y speed words, four bytes a row
SpeedByFacingAndStep0EBE:
    ldy a:OBJ_FACING_LEFT,x
    lda a:SpeedByFacingAndStep0EBERows,y
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:SpeedByFacingAndStep0EBESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:SpeedByFacingAndStep0EBESpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:SpeedByFacingAndStep0EBESpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:SpeedByFacingAndStep0EBESpeeds+2,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_STATE,x
    rts

; @routine 0ECB:558 ScreenTileAddress0ECB
; @run 74:D558
; WHAT IT DOES: The tile map address (zTemp07 high, zTemp05 low) of the screen point zTemp00-01 along X, zTemp02-03 along Y, with
; the scroll added (Stage 09's overlay).
; HOW: Loads, adds and stores in turn.
; WAITS: no.
; Entry: Exit: A zTemp05's byte, N, Z, C and V from its add; X and Y kept.
ScreenTileAddress0ECB:
    lda z:zTemp00
    clc
    adc a:ScrollXLo
    sta z:zTemp04
    lda z:zTemp01
    adc a:ScrollXHi
    and #$01
    lsr a
    lda z:zTemp04
    ror a
    lsr a
    lsr a
    and #$3F
    sta z:zTemp05
    lda z:zTemp02
    clc
    adc a:ScrollYLo
    sta z:zTemp06
    lda z:zTemp03
    adc a:ScrollYHi
    and #$01
    sta z:zTemp07
    lda z:zTemp06
    asl a
    rol z:zTemp07
    asl a
    rol z:zTemp07
    asl a
    rol z:zTemp07
    and #$C0
    clc
    adc z:zTemp05
    sta z:zTemp05
    rts

; @routine 117B:14A PlaceAtLevelWhenCount117B
; @run 70:B14A
; WHAT IT DOES: With OBJ_WORK_F 0 and OBJ_WORK_G-H at the count the table at $B112 gives for its start argument, puts the object at
; the level X kept in OBJ_WORK_J-K (Stage 0B's overlay).
; HOW: Loads, adds and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last compare or the subtract; Y twice the start argument.
PlaceAtLevelWhenCount117BCounts = $B112 ; OBJ_WORK_H then G by start argument
PlaceAtLevelWhenCount117B:
    lda a:OBJ_WORK_F,x
    bne PlaceAtLevelWhenCount117B_at0
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:OBJ_WORK_H,x
    cmp a:PlaceAtLevelWhenCount117BCounts,y
    bne PlaceAtLevelWhenCount117B_at0
    lda a:OBJ_WORK_G,x
    cmp a:PlaceAtLevelWhenCount117BCounts+1,y
    bne PlaceAtLevelWhenCount117B_at0
    lda a:OBJ_WORK_K,x
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda a:OBJ_WORK_J,x
    sbc a:ScrollXHiScreen
    sta a:OBJ_X_HI,x
PlaceAtLevelWhenCount117B_at0:
    rts

; @routine 117B:26F SpeedXTowardTarget117B
; @run 70:B26F
; WHAT IT DOES: Adds the step the table at $B1D0 gives for OBJ_WORK_F to the X speed until it is the target at $B1CC (Stage 0B's
; overlay).
; HOW: Loads, adds and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last compare or the add; Y twice OBJ_WORK_F.
SpeedXTowardTarget117BTargets = $B1CC ; the X speed to reach, a word each
SpeedXTowardTarget117BSteps = $B1D0 ; the step toward it, a word each
SpeedXTowardTarget117B:
    lda a:OBJ_WORK_F,x
    asl a
    tay
    lda a:OBJ_VX_LO,x
    cmp a:SpeedXTowardTarget117BTargets,y
    bne SpeedXTowardTarget117B_at0
    lda a:OBJ_VX_HI,x
    cmp a:SpeedXTowardTarget117BTargets+1,y
    bne SpeedXTowardTarget117B_at0
    rts
SpeedXTowardTarget117B_at0:
    lda a:OBJ_VX_LO,x
    clc
    adc a:SpeedXTowardTarget117BSteps,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:SpeedXTowardTarget117BSteps+1,y
    sta a:OBJ_VX_HI,x
    rts

; @routine 117F:2D4 HitBoxByFrameRow117F
; @run 71:D2D4
; WHAT IT DOES: Gives the object the hit box of its frame's row, 8 wider (Stage 0B's overlay).
; HOW: Loads, adds and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the half height, N and Z from it, Y the row's offset, C and V from the add.
HitBoxByFrameRow117FRows = $D2FD ; a box number by frame less 2
HitBoxByFrameRow117FBoxes = $D8A6 ; X and Y offset, half width and half height
HitBoxByFrameRow117F:
    lda a:OBJ_FRAME,x
    sec
    sbc #$02
    tay
    lda a:HitBoxByFrameRow117FRows,y
    asl a
    asl a
    tay
    lda a:HitBoxByFrameRow117FBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:HitBoxByFrameRow117FBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:HitBoxByFrameRow117FBoxes+2,y
    clc
    adc #$08
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:HitBoxByFrameRow117FBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 12DC:35C HitStopToState6_12DC
; @run 70:BB5C
; WHAT IT DOES: Under state 6, once hit: stopped with no hit box, the animation restarted, state 6 (Stage 0C's overlay).
; HOW: Loads, adds and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load; C from the state's compare.
HitStopToState6_12DC:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs HitStopToState6_12DC_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitStopToState6_12DC_at0
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitStopToState6_12DC_at0:
    rts

; @routine 13FD:3F2 SwingStep13FD
; @run 71:C3F2
; WHAT IT DOES: Counts OBJ_WORK_F on by 4, gives zero page $9E the byte at $C41E its bit 7 picks, and adds half a step to
; OBJ_WORK_G-H up to $18 (secret stage 0C's overlay).
; HOW: Loads, adds and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the compare or the add; Y 0 or 1.
SwingStep13FDHalves = $C41E ; zero page $9E by the half of the swing
SwingStep13FD:
    lda a:OBJ_WORK_F,x
    clc
    adc #$04
    sta a:OBJ_WORK_F,x
    rol a
    rol a
    and #$01
    tay
    lda a:SwingStep13FDHalves,y
    sta z:zRasterStage
    lda a:OBJ_WORK_G,x
    cmp #$18
    bcs SwingStep13FD_at0
    lda a:OBJ_WORK_H,x
    clc
    adc #$80
    sta a:OBJ_WORK_H,x
    lda a:OBJ_WORK_G,x
    adc #$00
    sta a:OBJ_WORK_G,x
SwingStep13FD_at0:
    rts

; @routine 18DB:483 LaunchByFacing18DB
; @run 70:B483
; WHAT IT DOES: The hit box off, the animation restarted, frame $11, OBJ_WORK_E 32 and the X speed of its facing; the substate is
; counted on (Stage 10's overlay).
; HOW: Loads, adds and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the speed's high byte, N and Z from the new substate, Y twice the facing.
LaunchByFacing18DBSpeeds = $B4AB ; the X speed by facing, a word each
LaunchByFacing18DB:
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$11
    sta a:OBJ_FRAME,x
    lda #$20
    sta a:OBJ_WORK_E,x
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:LaunchByFacing18DBSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:LaunchByFacing18DBSpeeds+1,y
    sta a:OBJ_VX_HI,x
    inc a:OBJ_SUBSTATE,x
    rts
