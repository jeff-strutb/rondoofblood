; @routine 09EB:2AD SinkOnWater09EB
; @run 70:B2AD
; WHAT IT DOES: Carries the player up while rising out of the water; at Y $A4: sound $A3, the water effects off, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SinkOnWater09EB:
    lda #$20
    sta z:zTemp08
    lda #$2C
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc SinkOnWater09EB_at0
    dec a:OBJ_Y_LO
    dec a:OBJ_Y_LO
SinkOnWater09EB_at0:
    lda z:zPlayerPush
    and #$F9
    sta z:zPlayerPush
    jsr PickWaterLevel
    lda #$1C
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr SpawnEveryFew09EB_3C4
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_G,x
    and #$01
    bne SinkOnWater09EB_at1
    jsr VecFindFreeSlotFrom30
    bcc SinkOnWater09EB_at1
    lda #$2F
    jsr VecStartObjectAtPlace
SinkOnWater09EB_at1:
    lda a:OBJ_Y_HI,x
    bne SinkOnWater09EB_at2
    lda a:OBJ_Y_LO,x
    cmp #$A4
    bcs SinkOnWater09EB_at2
    lda #$40
    sta a:OBJ_WORK_L,x
    lda #$A3
    jsr VecSoundCommand
    lda #$A4
    sta a:OBJ_Y_LO,x
    lda #$03
    sta a:OBJ_WORK_C,x
    stz a:LevelYHi
    stz a:RasterWaterWanted
    stz z:zRasterEffect
    stz z:zRasterArg
    stz a:OBJ_WORK_I,x
    inc a:OBJ_STATE,x
SinkOnWater09EB_at2:
    rts

; @routine 0EBD:484 LoadAtSideWithPartner0EBD
; @run 71:C484
; WHAT IT DOES: Loads at one side of the screen with a kind $2D partner in the next slot; with that slot taken, gone.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAtSideWithPartner0EBDSides = $C4F2 ; x and vx by side, four bytes
LoadAtSideWithPartner0EBDHeights = $C4FA ; eight heights
LoadAtSideWithPartner0EBD:
    jsr VecLoadObjectRecord
    lda a:OBJ_X_LO
    cmp #$C0
    bcs LoadAtSideWithPartner0EBD_at0
    cmp #$40
    bcc LoadAtSideWithPartner0EBD_at0
    jsr VecNextRandom
    bra LoadAtSideWithPartner0EBD_at1
LoadAtSideWithPartner0EBD_at0:
    lda a:OBJ_X_LO
    rol a
    rol a
LoadAtSideWithPartner0EBD_at1:
    and #$01
    asl a
    asl a
    tay
    lda a:LoadAtSideWithPartner0EBDSides+1,y
    sta a:OBJ_X_HI,x
    lda a:LoadAtSideWithPartner0EBDSides,y
    sta a:OBJ_X_LO,x
    lda a:LoadAtSideWithPartner0EBDSides+3,y
    sta a:OBJ_VX_HI,x
    lda a:LoadAtSideWithPartner0EBDSides+2,y
    sta a:OBJ_VX_LO,x
    stz a:OBJ_Y_HI,x
    jsr VecNextRandom
    and #$07
    tay
    lda a:LoadAtSideWithPartner0EBDHeights,y
    sta a:OBJ_Y_LO,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    txa
    inc a
    tay
    lda a:OBJ_KIND,y
    beq LoadAtSideWithPartner0EBD_at2
    jmp VecClearObject
LoadAtSideWithPartner0EBD_at2:
    jsr VecClearObjectInY
    lda #$2D
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    txa
    sta a:OBJ_START_ARG,y
    tya
    sta a:OBJ_WORK_F,x
    rts

; @routine 0EBE:508 BurstIntoSix0EBE
; @run 71:CD08
; WHAT IT DOES: From frame $41 pops off by the facing; then breaks into six kind $3A pieces and goes.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BurstIntoSix0EBEPops = $CB53 ; vx high by frame
BurstIntoSix0EBEPieces = $CD74 ; eight bytes per piece
BurstIntoSix0EBE:
    lda a:OBJ_FRAME,x
    cmp #$41
    bcc BurstIntoSix0EBE_at0
    sec
    sbc #$41
    tay
    lda a:BurstIntoSix0EBEPops,y
    sta a:OBJ_VX_HI,x
    lda #$20
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT,x
    beq BurstIntoSix0EBE_at1
    jsr VecNegateVelocityX
BurstIntoSix0EBE_at1:
    jsr VecApplyVelocity
BurstIntoSix0EBE_at0:
    cly
BurstIntoSix0EBE_at3:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc BurstIntoSix0EBE_at2
    lda #$3A
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    stx z:zClimbStepBefore
    lda z:zBurstCount
    asl a
    asl a
    asl a
    tax
    lda a:BurstIntoSix0EBEPieces,x
    sta a:OBJ_WORK_D,y
    lda a:BurstIntoSix0EBEPieces+1,x
    sta a:OBJ_WORK_E,y
    lda a:BurstIntoSix0EBEPieces+2,x
    sta a:OBJ_VX_HI,y
    lda a:BurstIntoSix0EBEPieces+3,x
    sta a:OBJ_VX_LO,y
    lda a:BurstIntoSix0EBEPieces+4,x
    sta a:OBJ_VY_HI,y
    lda a:BurstIntoSix0EBEPieces+5,x
    sta a:OBJ_VY_LO,y
    ldx z:zClimbStepBefore
    ldy z:zBurstCount
    iny
    cpy #$06
    bcc BurstIntoSix0EBE_at3
BurstIntoSix0EBE_at2:
    jmp VecClearObject

; @routine 0F6D:382 RiseSwayThenGo0F6D
; @run 71:C382
; WHAT IT DOES: Rises swaying to Y $C4, then waits $10 counts and goes.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RiseSwayThenGo0F6D:
    lda a:OBJ_STATE,x
    bne RiseSwayThenGo0F6D_at0
    jsr VecLoadObjectRecord
    lda #$08
    sta a:OBJ_SPRITE_SET,x
    lda #$04
    sta a:OBJ_FRAME,x
    rts
RiseSwayThenGo0F6D_at0:
    dec a
    bne RiseSwayThenGo0F6D_at1
    lda a:OBJ_Y_LO,x
    cmp #$C4
    bcs RiseSwayThenGo0F6D_at2
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_L,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:zProduct1
    sta a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda a:OBJ_WORK_M,x
    clc
    adc #$80
    sta a:OBJ_WORK_M,x
    lda a:OBJ_WORK_L,x
    adc #$02
    sta a:OBJ_WORK_L,x
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    rts
RiseSwayThenGo0F6D_at2:
    lda #$C4
    sta a:OBJ_Y_LO,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda #$10
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
    rts
RiseSwayThenGo0F6D_at1:
    dec a:OBJ_WORK_E,x
    bne RiseSwayThenGo0F6D_at3
    jmp VecClearObject
RiseSwayThenGo0F6D_at3:
    rts

; @routine 117B:200 CarrierPlatform117B
; @run 70:B200
; WHAT IT DOES: A platform: stood on it goes down and carries the player; the player's flags keep it still.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CarrierPlatform117B:
    lda a:RichterColumnHit
    ora a:RichterColumnHit+1
    ora a:RichterColumnHit+2
    and #$01
    beq CarrierPlatform117B_at0
    stz a:OBJ_WORK_I,x
    bra CarrierPlatform117B_at1
CarrierPlatform117B_at0:
    lda a:OBJ_WORK_D,x
    bne CarrierPlatform117B_at2
    lda a:OBJ_WORK_I,x
    beq CarrierPlatform117B_at2
    dec a:OBJ_WORK_I,x
    rts
CarrierPlatform117B_at2:
    lda #$18
    sta z:zTemp08
    lda #$1D
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc CarrierPlatform117B_at1
    lda a:OBJ_WORK_D,x
    bne CarrierPlatform117B_at3
    lda a:OBJ_LOOP_FLAGS,x
    ora #$01
    sta a:OBJ_LOOP_FLAGS,x
    lda #$01
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    inc a:OBJ_Y_LO,x
    inc a:OBJ_WORK_D,x
    lda #$10
    sta a:OBJ_WORK_I,x
CarrierPlatform117B_at3:
    jsr MoveXWithScrollBand117B
    lda z:zPlayerPush
    ora #$30
    sta z:zPlayerPush
    lda a:OBJ_VX_LO,x
    sta z:zPushXLo
    lda a:OBJ_VX_HI,x
    sta z:zPushXHi
    rts
CarrierPlatform117B_at1:
    lda a:OBJ_LOOP_FLAGS,x
    and #$FE
    sta a:OBJ_LOOP_FLAGS,x
    stz a:OBJ_WORK_D,x
    rts

; @routine 12DD:769 PlatformCarry12DD
; @run 71:C769
; WHAT IT DOES: Stood on carries the player (clearing eight bytes of $7D00); struck, pushes by its speed against the player's.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PlatformCarry12DD:
    lda #$17
    sta z:zTemp08
    lda #$24
    sta z:zTemp09
    jsr ContactWithPlayer
    bcs PlatformCarry12DD_at0
    lda a:OBJ_WORK_F,x
    beq PlatformCarry12DD_at1
    dec a:OBJ_WORK_F,x
    rts
PlatformCarry12DD_at1:
    lda a:OBJ_HIT_TAKEN,x
    beq PlatformCarry12DD_at2
    lda z:zPlayerPush
    ora #$10
    sta z:zPlayerPush
    lda a:OBJ_VX_LO,x
    sec
    sbc a:OBJ_VX_LO
    sta z:zPushXLo
    lda a:OBJ_VX_HI,x
    sbc a:OBJ_VX_HI
    sta z:zPushXHi
    lda a:OBJ_STATE,x
    cmp #$03
    bcc PlatformCarry12DD_at2
    lda #$08
    sta z:zProduct1
    lda #$02
    jsr VecProbeTerrain
    and #$02
    beq PlatformCarry12DD_at2
    lda #$02
    sta a:OBJ_WORK_C,x
    rts
PlatformCarry12DD_at0:
    lda z:zPlayerPush
    ora #$30
    sta z:zPlayerPush
    lda a:OBJ_VX_LO,x
    sta z:zPushXLo
    lda a:OBJ_VX_HI,x
    sta z:zPushXHi
    lda #$03
    sta a:OBJ_WORK_F,x
    ldy #$2C
    cla
PlatformCarry12DD_at3:
    sta a:SaveBuffer,y
    iny
    cpy #$34
    bcc PlatformCarry12DD_at3
PlatformCarry12DD_at2:
    rts

; @routine 13FD:30D SeekingOrb13FD
; @run 71:C30D
; WHAT IT DOES: Turns toward the player as it moves, for its 16-bit lifetime, pulsing OBJ_WORK_G-H (sound $46 each pulse).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SeekingOrb13FD:
    lda #$C0
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_J,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    jsr VecAngleToPlayer
    sta z:zTemp02
    lda #$01
    sta z:zTemp00
    lda a:OBJ_WORK_J,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_J,x
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    bne SeekingOrb13FD_at0
    jmp VecClearObject
SeekingOrb13FD_at0:
    lda a:OBJ_WORK_F,x
    clc
    adc #$04
    sta a:OBJ_WORK_F,x
    stz z:zTemp00
    lda #$0A
    sta z:zTemp01
    lda a:OBJ_WORK_K,x
    jsr VecScaleBySine
    lda z:zProduct1
    sta a:OBJ_WORK_H,x
    lda z:zProduct2
    clc
    adc #$0B
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_K,x
    clc
    adc #$02
    sta a:OBJ_WORK_K,x
    bcc SeekingOrb13FD_at1
    lda #$46
    jsr VecSoundCommand
SeekingOrb13FD_at1:
    rts

; @routine 161B:4BE SpitThenRest161B
; @run 70:B4BE
; WHAT IT DOES: Faces and waits for OBJ_WORK_D counts, then spits kind $2F objects (sound 7) every eighth count of OBJ_WORK_E.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpitThenRest161B:
    lda a:OBJ_SUBSTATE,x
    bne SpitThenRest161B_at0
    lda a:OBJ_WORK_E,x
    and #$1F
    bne SpitThenRest161B_at1
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
SpitThenRest161B_at1:
    lda #$AE
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_D,x
    bne SpitThenRest161B_at2
    lda #$89
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
SpitThenRest161B_at2:
    rts
SpitThenRest161B_at0:
    lda #$AE
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_D,x
    bne SpitThenRest161B_at3
    dec a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts
SpitThenRest161B_at3:
    lda a:OBJ_WORK_E,x
    and #$07
    bne SpitThenRest161B_at4
    jsr VecFindFreeSlotFrom30
    bcc SpitThenRest161B_at4
    lda #$07
    jsr VecSoundCommand
    lda #$2F
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
SpitThenRest161B_at4:
    rts
