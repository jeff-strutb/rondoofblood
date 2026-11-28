; @routine 072C:43F RiseTrailingDebris072C
; @run 70:BC3F
; WHAT IT DOES: Rises with debris every fourth count until falling; then not hit-tested and substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RiseTrailingDebris072C:
    lda a:OBJ_WORK_D,x
    and #$03
    bne RiseTrailingDebris072C_at0
    lda #$23
    jsr VecSoundCommand
    cla
    jsr StartDebrisBurst
RiseTrailingDebris072C_at0:
    lda #$8A
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_VY_LO,x
    clc
    adc #$2C
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_VY_HI,x
    bmi RiseTrailingDebris072C_at1
    stz a:OBJ_HITBOX_ON,x
    lda #$07
    sta a:OBJ_WORK_I,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$16
    sta a:OBJ_WORK_J,x
    inc a:OBJ_SUBSTATE,x
RiseTrailingDebris072C_at1:
    rts

; @routine 0B4D:03B WaterByLeader0B4D
; @run 71:C03B
; WHAT IT DOES: Rides $80 above slot $10 for OBJ_WORK_D counts, flickering, waving the water line; then the line off and cleared.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WaterByLeader0B4DFrames = $C085 ; two frames
WaterByLeader0B4DLines = $C087 ; four water lines
WaterByLeader0B4D:
    ldy #$10
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_Y_LO,x
    sec
    sbc #$80
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_Y_HI,x
    lda a:OBJ_FACING_LEFT+$10
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_WORK_D,x
    and #$01
    tay
    lda a:WaterByLeader0B4DFrames,y
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_D,x
    and #$03
    asl a
    tay
    lda a:WaterByLeader0B4DLines+1,y
    sta a:LevelYHi
    lda a:WaterByLeader0B4DLines,y
    sta a:RasterWaterWanted
    dec a:OBJ_WORK_D,x
    bne WaterByLeader0B4D_at0
    stz a:LevelYHi
    stz a:RasterWaterWanted
    jmp VecClearObject
WaterByLeader0B4D_at0:
    rts

; @routine 0E0B:120 OpenArena0E0B
; @run 70:B120
; WHAT IT DOES: Entered with the raster effect in A: holds the scroll at X $280; there, $80 counts, the CD track again, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
OpenArena0E0B:
    sta z:zRasterEffect
    lda #$BF
    sta z:zRasterArg
    lda #$02
    sta a:ScrollXMaxHi
    lda #$80
    sta a:ScrollXMaxLo
    lda a:OBJ_SUBSTATE,x
    bne OpenArena0E0B_at0
    lda a:ScrollXHiScreen
    cmp #$02
    bne OpenArena0E0B_at1
    lda #$80
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
OpenArena0E0B_at1:
    rts
OpenArena0E0B_at0:
    dec a:OBJ_WORK_D,x
    bne OpenArena0E0B_at2
    rmb3 z:zField8E
    rmb4 z:zField8E
    jsr VecCdPlayAgain
    lda #$48
    sta a:OBJ_WORK_D,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
OpenArena0E0B_at2:
    rts

; @routine 0E0B:302 WalkTurnOrStop0E0B
; @run 70:B302
; WHAT IT DOES: Walks; off the ground stops in state 3; at a wall turns round (carry set).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkTurnOrStop0E0B:
    jsr VecApplyVelocityX
    lda #$04
    sta z:zProduct1
    lda #$01
    jsr VecProbeTerrain
    and #$01
    beq WalkTurnOrStop0E0B_at0
    lda #$04
    sta z:zProduct1
    lda #$22
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    asl a
    tay
    lda a:TileHit,y
    ora a:TileHit+1,y
    and #$02
    bne WalkTurnOrStop0E0B_at1
    clc
    rts
WalkTurnOrStop0E0B_at1:
    jsr VecNegateVelocityX
    jsr VecApplyVelocityX
    sec
    rts
WalkTurnOrStop0E0B_at0:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$03
    sta a:OBJ_STATE,x
    clc
    rts

; @routine 0E0B:3AF ChooseAttack0E0B
; @run 70:B3AF
; WHAT IT DOES: Picks the next attack: by the health and a count when the player is near in height, at random when not; 0 means state 4.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChooseAttack0E0BOrders = $B3F8 ; the order of attacks
ChooseAttack0E0BSubstates = $B428 ; substate by attack
ChooseAttack0E0B:
    jsr VecDistanceYToPlayer
    cmp #$30
    bcc ChooseAttack0E0B_at0
    jsr VecNextRandom
    and #$01
    clc
    adc #$04
    bra ChooseAttack0E0B_at1
ChooseAttack0E0B_at0:
    lda a:OBJ_HEALTH,x
    lsr a
    and #$F0
    sta z:zTemp00
    lda a:OBJ_WORK_G,x
    and #$0F
    clc
    adc z:zTemp00
    tay
    inc a:OBJ_WORK_G,x
    lda a:ChooseAttack0E0BOrders,y
    bne ChooseAttack0E0B_at1
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts
ChooseAttack0E0B_at1:
    tay
    lda a:ChooseAttack0E0BSubstates,y
    sta a:OBJ_SUBSTATE,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    rts

; @routine 0E0B:472 ListThenLeapByDistance0E0B
; @run 70:B472
; WHAT IT DOES: At the list's end sets the raster effect and leaps at speeds by the distance to the player.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenLeapByDistance0E0BLeaps = $B4C9 ; vx and vy by distance, four bytes
ListThenLeapByDistance0E0B:
    lda #$BC
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenLeapByDistance0E0B_at0
    lda #$0B
    sta z:zRasterEffect
    lda #$BF
    sta z:zRasterArg
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    asl a
    tay
    lda a:ListThenLeapByDistance0E0BLeaps+1,y
    sta a:OBJ_VX_HI,x
    lda a:ListThenLeapByDistance0E0BLeaps,y
    sta a:OBJ_VX_LO,x
    lda a:ListThenLeapByDistance0E0BLeaps+3,y
    sta a:OBJ_VY_HI,x
    lda a:ListThenLeapByDistance0E0BLeaps+2,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_FACING_LEFT,x
    beq ListThenLeapByDistance0E0B_at0
    jsr VecNegateVelocityX
ListThenLeapByDistance0E0B_at0:
    rts

; @routine 0EBD:1D8 AmbushFromBehind0EBD
; @run 71:C1D8
; WHAT IT DOES: Far from the player and on one side, becomes kind $38 $34 to the right; on the other side goes back a state.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AmbushFromBehind0EBDThen = $CC24 ; a routine of this overlay
AmbushFromBehind0EBD:
    lda z:zGameSubMode
    cmp #$03
    bne AmbushFromBehind0EBD_at0
    rts
AmbushFromBehind0EBD_at0:
    jsr VecSideOfPlayer
    and #$01
    beq AmbushFromBehind0EBD_at1
    jsr VecDistanceXToPlayer
    cmp #$78
    bcc AmbushFromBehind0EBD_at2
    lda #$10
    sta a:OBJ_WORK_D-1,x
    lda #$38
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    lda a:OBJ_X_LO,x
    clc
    adc #$34
    sta a:OBJ_X_LO,x
    lda #$00
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$60
    sta a:OBJ_LOOP_FLAGS,x
    jmp AmbushFromBehind0EBDThen
AmbushFromBehind0EBD_at1:
    jsr VecDistanceXToPlayer
    cmp #$78
    bcc AmbushFromBehind0EBD_at2
    dec a:OBJ_STATE,x
AmbushFromBehind0EBD_at2:
    rts

; @routine 0EC0:44A PlatformCarry0EC0
; @run 71:DC4A
; WHAT IT DOES: A moving platform: carries the player at its speed; the first step on it at Y $BC starts it down.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PlatformCarry0EC0:
    lda #$30
    sta z:zTemp08
    lda #$16
    sta z:zTemp09
    jsr ContactWithPlayer
    bcs PlatformCarry0EC0_at0
    lda a:OBJ_WORK_D,x
    beq PlatformCarry0EC0_at1
    lda #$08
    sta a:OBJ_WORK_E,x
PlatformCarry0EC0_at1:
    stz a:OBJ_WORK_D,x
    rts
PlatformCarry0EC0_at0:
    lda z:zPlayerPush
    ora #$10
    and #$F9
    sta z:zPlayerPush
    lda a:OBJ_VX_LO,x
    sta z:zPushXLo
    lda a:OBJ_VX_HI,x
    sta z:zPushXHi
    lda a:OBJ_WORK_D,x
    bne PlatformCarry0EC0_at2
    lda a:OBJ_Y_LO,x
    cmp #$BC
    bne PlatformCarry0EC0_at2
    lda #$01
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
PlatformCarry0EC0_at2:
    lda #$01
    sta a:OBJ_WORK_D,x
    rts

; @routine 101E:1A2 SteerAndBob101E
; @run 71:C9A2
; WHAT IT DOES: Turns OBJ_WORK_D toward angle $40 and adds a cosine bob into OBJ_WORK_E-F; each cycle restarts it with OBJ_WORK_J $80.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SteerAndBob101E:
    lda #$40
    sta z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    lda #$00
    sta z:zTemp00
    lda #$02
    sta z:zTemp01
    lda a:OBJ_WORK_G,x
    jsr VecScaleByCosine
    lda a:OBJ_WORK_F,x
    clc
    adc z:zProduct1
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    adc z:zProduct2
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_G,x
    clc
    adc #$04
    sta a:OBJ_WORK_G,x
    bcc SteerAndBob101E_at0
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_F,x
    lda #$80
    sta a:OBJ_WORK_J,x
SteerAndBob101E_at0:
    rts

; @routine 117E:38B ChooseState117E
; @run 71:CB8B
; WHAT IT DOES: Rushes (state 9) when slot 0 is in substate 1 and the player is far; else after OBJ_WORK_F counts picks the next state.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChooseState117ECycle = $CBD7 ; eight picks
ChooseState117EStates = $CBDF ; state by pick
ChooseState117EWaits = $CBE2 ; four waits
ChooseState117E:
    lda a:OBJ_SUBSTATE
    cmp #$01
    bne ChooseState117E_at0
    jsr VecDistanceXToPlayer
    cmp #$50
    bcc ChooseState117E_at0
    jsr VecPlayerDistanceSum
    and #$01
    bne ChooseState117E_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$09
    sta a:OBJ_STATE,x
    rts
ChooseState117E_at0:
    dec a:OBJ_WORK_F,x
    bne ChooseState117E_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    jsr VecPlayerDistanceSum
    and #$03
    tay
    lda a:ChooseState117EWaits,y
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_M,x
    and #$07
    tay
    lda a:ChooseState117ECycle,y
    tay
    lda a:ChooseState117EStates,y
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_M,x
ChooseState117E_at1:
    rts

; @routine 12DB:2F4 SpiralDown12DB
; @run 70:B2F4
; WHAT IT DOES: Circles at half the Y speed while OBJ_WORK_D opens to $8E; off the left, placed at Y $50 for the next state.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpiralDown12DB:
    lda a:OBJ_WORK_D,x
    cmp #$8E
    bcs SpiralDown12DB_at0
    lda a:OBJ_WORK_D,x
    clc
    adc #$03
    sta a:OBJ_WORK_D,x
SpiralDown12DB_at0:
    lda #$80
    sta z:zTemp00
    lda #$03
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_VY_HI,x
    asl a
    ror a:OBJ_VY_HI,x
    ror a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_X_HI,x
    bne SpiralDown12DB_at1
    lda a:OBJ_X_LO,x
    cmp #$10
    bcs SpiralDown12DB_at1
    stz a:OBJ_FRAME,x
    lda #$50
    sta a:OBJ_Y_LO,x
    lda #$80
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
SpiralDown12DB_at1:
    rts

; @routine 12DC:491 SteerAndBob12DC
; @run 70:BC91
; WHAT IT DOES: Turns OBJ_WORK_D toward the facing's angle and adds a cosine bob into OBJ_WORK_E-F, restarted each cycle.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SteerAndBob12DCAngles = $BCD9 ; angle by facing
SteerAndBob12DC:
    ldy a:OBJ_FACING_LEFT,x
    lda a:SteerAndBob12DCAngles,y
    sta z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x

; @routine 12DD:6F9 FlyOverWater12DD
; @run 71:C6F9
; WHAT IT DOES: Crosses down the screen with a raster effect, cleared below.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FlyOverWater12DD:
    lda a:OBJ_STATE,x
    bne FlyOverWater12DD_at0
    jsr VecLoadObjectRecord
    lda #$04
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$60
    sta a:OBJ_Y_LO,x
    lda #$03
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    lda #$0B
    sta z:zRasterEffect
    lda #$BF
    sta z:zRasterArg
    rts
FlyOverWater12DD_at0:
    lda #$43
    sta z:zTemp10
    lda #$C7
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    cmp #$02
    bne FlyOverWater12DD_at1
    stz z:zRasterEffect
    stz z:zRasterArg
    jsr VecClearObject
FlyOverWater12DD_at1:
    rts

; @routine 13FB:5EF LeapUntilLand13FB
; @run 70:B5EF
; WHAT IT DOES: Falls until it lands at Y $A8; then hit-tested, turned round, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LeapUntilLand13FB:
    lda #$3A
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr SpawnEveryFew13FB_63F
    jsr VecApplyVelocity
    lda #$04
    sta z:zProduct1
    lda #$22
    jsr VecProbeTerrain
    lda a:OBJ_VY_LO,x
    clc
    adc #$80
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    bmi LeapUntilLand13FB_at0
    lda a:OBJ_Y_LO,x
    cmp #$A8
    bcc LeapUntilLand13FB_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
LeapUntilLand13FB_at0:
    rts

; @routine 161C:19E BreakIntoSix161C
; @run 70:B99E
; WHAT IT DOES: Replaced by six kind $37 pieces, each with its waits and speeds.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BreakIntoSix161CPieces = $B9E9 ; eight bytes per piece
BreakIntoSix161C:
    cly
BreakIntoSix161C_at1:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc BreakIntoSix161C_at0
    lda #$37
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    stx z:zClimbStepBefore
    lda z:zBurstCount
    asl a
    asl a
    asl a
    tax
    lda a:BreakIntoSix161CPieces,x
    sta a:OBJ_WORK_D,y
    lda a:BreakIntoSix161CPieces+1,x
    sta a:OBJ_WORK_E,y
    lda a:BreakIntoSix161CPieces+2,x
    sta a:OBJ_VX_HI,y
    lda a:BreakIntoSix161CPieces+3,x
    sta a:OBJ_VX_LO,y
    lda a:BreakIntoSix161CPieces+4,x
    sta a:OBJ_VY_HI,y
    lda a:BreakIntoSix161CPieces+5,x
    sta a:OBJ_VY_LO,y
    ldx z:zClimbStepBefore
    ldy z:zBurstCount
    iny
    cpy #$06
    bcc BreakIntoSix161C_at1
BreakIntoSix161C_at0:
    jmp VecClearObject

; @routine 161F:21E CrushingPlatform161F
; @run 71:D21E
; WHAT IT DOES: Stood on, sinks with the player, one every other frame; left, springs back to its height.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CrushingPlatform161F:
    lda a:OBJ_STATE,x
    bne CrushingPlatform161F_at0
    jsr VecLoadObjectRecord
    lda #$26
    sta a:OBJ_WORK_D,x
    rts
CrushingPlatform161F_at0:
    lda #$11
    sta z:zTemp08
    lda a:OBJ_WORK_D,x
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc CrushingPlatform161F_at1
    lda #$01
    sta a:OBJ_WORK_E,x
    lda z:zPlayerPush
    ora #$20
    sta z:zPlayerPush
    lda z:zWaveCounter
    and #$01
    bne CrushingPlatform161F_at2
    lda #$01
    sta z:zPushYHi
    dec a:OBJ_WORK_D,x
CrushingPlatform161F_at2:
    rts
CrushingPlatform161F_at1:
    jsr VecSideOfPlayer
    and #$02
    beq CrushingPlatform161F_at3
    lda z:zPlayerPush
    and #$F9
    sta z:zPlayerPush
CrushingPlatform161F_at3:
    lda #$26
    sta a:OBJ_WORK_D,x
    stz a:OBJ_WORK_E,x
    rts

; @routine 161F:5DE BreakableDoor161F
; @run 71:D5DE
; WHAT IT DOES: A door that first needs a hit with kind 7 in slot 5 (PlayerKind7Caught set); then a weak hit takes the route in the start argument.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BreakableDoor161F:
    lda a:OBJ_WORK_F,x
BreakableDoor161F_at0:
    bne BreakableDoor161F_at1
    lda a:OBJ_HIT_TAKEN,x
    beq BreakableDoor161F_at2
    lda a:OBJ_KIND+5
    cmp #$07
    bne BreakableDoor161F_at2
    lda #$01
    sta a:PlayerKind7Caught
    inc a:OBJ_WORK_F,x
BreakableDoor161F_at1:
    lda a:StageDoorHeld
    eor #$01
    and a:PlayerDoorReady
    beq BreakableDoor161F_at2
    lda a:OBJ_HIT_TAKEN,x
    beq BreakableDoor161F_at2
    cmp #$03
    bcs BreakableDoor161F_at2
    lda a:OBJ_START_ARG,x
    and #$0F
    inc a
    sta z:zField8C
    lda #$03
    sta z:zSubStep
    lda #$85
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
    stz a:OBJ_WORK_E,x
    lda #$04
    sta a:OBJ_WORK_D,x
BreakableDoor161F_at2:
    rts
