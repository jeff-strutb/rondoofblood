; @routine 072D:039 SparkFromSlot10_072D
; @run 71:C039
; WHAT IT DOES: A spark thrown from slot $10, falling until its list ends.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SparkFromSlot10_072D:
    lda a:OBJ_STATE,x
    bne SparkFromSlot10_072D_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_SPRITE_FLIP+$10
    sta a:OBJ_SPRITE_FLIP,x
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$08
    sta a:OBJ_VX_HI,x
    lda #$E0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$02
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    rts
SparkFromSlot10_072D_at0:
    lda #$8E
    sta z:zTemp10
    lda #$C0
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne SparkFromSlot10_072D_at1
    jmp VecClearObject
SparkFromSlot10_072D_at1:
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    rts

; @routine 0EC0:3CC SwitchPlatform0EC0
; @run 71:DBCC
; WHAT IT DOES: Stood on: starts a kind $4A object with a count, goes down, holds the player, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwitchPlatform0EC0:
    lda #$30
    sta z:zTemp08
    lda #$16
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc SwitchPlatform0EC0_at0
    ldy #$10
    jsr VecClearObjectInY
    lda #$4A
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_G,x
    sta a:OBJ_START_ARG,y
    cmp #$03
    bcs SwitchPlatform0EC0_at1
    inc a:OBJ_WORK_G,x
SwitchPlatform0EC0_at1:
    lda #$01
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    lda #$01
    sta a:OBJ_WORK_D,x
    lda #$80
    sta a:OBJ_WORK_E,x
    lda #$C0
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    lda a:OBJ_LOOP_FLAGS,x
    ora #$01
    sta a:OBJ_LOOP_FLAGS,x
    inc a:OBJ_STATE,x
SwitchPlatform0EC0_at0:
    rts

; @routine 0EC0:59F ReleaseOnStateChange0EC0
; @run 71:DD9F
; WHAT IT DOES: When $DC1D changes the state, throws the item for OBJ_WORK_G (by zField93 for the last).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ReleaseOnStateChange0EC0Items = $DDF0 ; item by count, $FF the last
ReleaseOnStateChange0EC0Lasts = $DDF4 ; the last item, two choices
ReleaseOnStateChange0EC0:
    lda a:OBJ_STATE,x
    pha
    jsr PointCallThenCount0EC0
    pla
    cmp a:OBJ_STATE,x
    beq ReleaseOnStateChange0EC0_at0
    ldy a:OBJ_WORK_G,x
    lda a:ReleaseOnStateChange0EC0Items,y
    cmp #$FF
    bne ReleaseOnStateChange0EC0_at1
    cly
    lda z:zField93
    cmp #$07
    bne ReleaseOnStateChange0EC0_at2
    iny
ReleaseOnStateChange0EC0_at2:
    lda a:ReleaseOnStateChange0EC0Lasts,y
ReleaseOnStateChange0EC0_at1:
    ldy #$11
    jsr VecClearObjectInY
    jsr VecStartObjectAtPlace
    lda #$E8
    clc
    adc a:OBJ_Y_LO,y
    sta a:OBJ_Y_LO,y
    lda #$F0
    clc
    adc a:OBJ_X_LO,y
    sta a:OBJ_X_LO,y
    lda #$FA
    sta a:OBJ_VY_HI,y
    lda #$01
    sta a:OBJ_VX_HI,y
    lda #$80
    sta a:OBJ_VX_LO,y
    lda #$50
    sta a:OBJ_INVULN_TIMER,y
ReleaseOnStateChange0EC0_at0:
    rts

; @routine 101E:4A5 LeadOrLaunch101E
; @run 71:CCA5
; WHAT IT DOES: While slot $10's OBJ_WORK_M is 1 follows the trail at its lag; else launches on its own angle, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LeadOrLaunch101ETrail = $C7FC ; the leader's angles
LeadOrLaunch101ELags = $CC1B ; the lag in the trail by OBJ_WORK_D
LeadOrLaunch101E:
    jsr FollowPreviousWithSlot16_101E
    lda a:OBJ_WORK_M+$10
    cmp #$01
    beq LeadOrLaunch101E_at0
    stz a:OBJ_WORK_J,x
    stz z:zTemp00
    lda a:OBJ_WORK_I,x
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_VY_HI,x
    asl a
    ror a:OBJ_VY_HI,x
    ror a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    inc a:OBJ_STATE,x
    rts
LeadOrLaunch101E_at0:
    ldy a:OBJ_WORK_D,x
    lda a:LeadOrLaunch101ELags,y
    tay
    stz z:zTemp00
    lda a:OBJ_WORK_I,x
    sta z:zTemp01
    lda a:LeadOrLaunch101ETrail,y
    sta a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_VY_HI,x
    asl a
    ror a:OBJ_VY_HI,x
    ror a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    rts

; @routine 101E:7F5 GateStates101E
; @run 71:CFF5
; WHAT IT DOES: In state 0 loads the gate and starts its marker (the code of StartMarkerLeft101F); other states go to $D027.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
GateStates101ELater = $D027 ; the other states' code of this overlay
GateStates101E:
    lda a:OBJ_STATE,x
    bne GateStates101ELater
    jsr VecLoadObjectRecord
    lda #$12
    sta a:OBJ_FRAME,x
    lda a:OBJ_KIND-2,x
    cmp #$BB

; @routine 1020:1EA DriftingBackground1020
; @run 71:D9EA
; WHAT IT DOES: A drifting piece by slot and speed, wrapped from the left edge to the right.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DriftingBackground1020Heights = $D9E2 ; Y by slot
DriftingBackground1020Speeds = $DA5A ; vx by slot
DriftingBackground1020:
    lda a:OBJ_STATE,x
    bne DriftingBackground1020_at0
    jsr VecLoadObjectRecord
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
    txa
    and #$03
    sta a:OBJ_WORK_D,x
    txa
    sec
    sbc #$48
    tay
    lda a:DriftingBackground1020Heights,y
    sta a:OBJ_Y_LO,x
    rts
DriftingBackground1020_at0:
    lda a:OBJ_START_ARG,x
    bne DriftingBackground1020_at1
    lda a:OBJ_WORK_D,x
    and #$03
    asl a
    tay
    lda a:DriftingBackground1020Speeds,y
    sta a:OBJ_VX_LO,x
    lda a:DriftingBackground1020Speeds+1,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
DriftingBackground1020_at1:
    jsr MoveWithScrollY1020
    lda a:OBJ_X_HI,x
    cmp #$FF
    bne DriftingBackground1020_at2
    lda a:OBJ_X_LO,x
    bmi DriftingBackground1020_at2
    lda #$01
    sta a:OBJ_X_HI,x
DriftingBackground1020_at2:
    rts

; @routine 10CC:2F9 SlideWithCosine10CC
; @run 70:BAF9
; WHAT IT DOES: Slides at the facing's speed, with a cosine sway while OBJ_WORK_N is 0, until it lands.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SlideWithCosine10CCSpeeds = $BB48 ; vx by facing
SlideWithCosine10CC:
    lda a:OBJ_WORK_N,x
    bne SlideWithCosine10CC_at0
    lda #$50
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_G,x
    jsr VecScaleByCosine
    lda a:OBJ_WORK_G,x
    clc
    adc #$02
    sta a:OBJ_WORK_G,x
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda z:zProduct1
    clc
    adc a:SlideWithCosine10CCSpeeds,y
    sta a:OBJ_VX_LO,x
    lda z:zProduct2
    adc a:SlideWithCosine10CCSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda #$04
    jsr VecStopOnLanding
    rts
SlideWithCosine10CC_at0:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:SlideWithCosine10CCSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:SlideWithCosine10CCSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda #$04
    jsr VecStopOnLanding
    rts

; @routine 10CD:035 LoadDelaysAtRandom10CD
; @run 71:C035
; WHAT IT DOES: Loads random waits and speeds, by the start argument or the player's side.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadDelaysAtRandom10CDBases = $C088 ; wait base by side
LoadDelaysAtRandom10CDSideSpeeds = $C08A ; four speeds
LoadDelaysAtRandom10CDArgSpeeds = $C08E ; four speeds
LoadDelaysAtRandom10CDPairs = $C092 ; four pairs
LoadDelaysAtRandom10CD:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    beq LoadDelaysAtRandom10CD_at0
    jsr VecNextRandom
    and #$3F
    clc
    adc #$A0
    sta a:OBJ_WORK_D,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:LoadDelaysAtRandom10CDArgSpeeds,y
    sta a:OBJ_WORK_G,x
    bra LoadDelaysAtRandom10CD_at1
LoadDelaysAtRandom10CD_at0:
    jsr VecSideOfPlayer
    and #$01
    tay
    jsr VecNextRandom
    and #$7F
    clc
    adc a:LoadDelaysAtRandom10CDBases,y
    sta a:OBJ_WORK_D,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:LoadDelaysAtRandom10CDSideSpeeds,y
    sta a:OBJ_WORK_G,x
LoadDelaysAtRandom10CD_at1:
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:LoadDelaysAtRandom10CDPairs,y
    sta a:OBJ_WORK_F,x
    lda a:LoadDelaysAtRandom10CDPairs+1,y
    sta a:OBJ_WORK_E,x
    rts

; @routine 117A:0BC BounceThenOpen117A
; @run 70:A8BC
; WHAT IT DOES: A hit piece bounces a few times (sound $19), then notes itself broken in zField8F and becomes what it holds.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BounceThenOpen117ACounts = $B072 ; bounces by kind of piece
BounceThenOpen117AHolds = $B077 ; what it becomes
BounceThenOpen117ASpeeds = $B078 ; vx and vy pairs
BounceThenOpen117A:
    lda #$19
    jsr VecSoundCommand
    lda #$01
    jsr StartDebrisBurst
    ldy a:OBJ_WORK_E,x
    inc a:OBJ_WORK_D,x
    lda a:OBJ_WORK_D,x
    cmp a:BounceThenOpen117ACounts,y
    bcs BounceThenOpen117A_at0
    lda a:OBJ_WORK_D,x
    dec a
    asl a
    clc
    adc a:OBJ_WORK_E,x
    tay
    lda a:BounceThenOpen117AHolds+1,y
    sta a:OBJ_VX_HI,x
    lda a:BounceThenOpen117ASpeeds+1,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    rts
BounceThenOpen117A_at0:
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    ora z:zField8F
    sta z:zField8F
    ldy a:OBJ_WORK_E,x
    lda a:BounceThenOpen117AHolds,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 117E:005 LoadLeapByKind117E
; @run 71:C805
; WHAT IT DOES: Loads and leaps: by OBJ_WORK_E, or for 2 by the distance to the player; mirrored by facing.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadLeapByKind117ESpeeds = $C856 ; vx by kind
LoadLeapByKind117EReaches = $C85C ; vx by distance
LoadLeapByKind117E:
    jsr VecLoadObjectRecord
    jsr SpeedByWorkEAndMove117E
    lda #$38
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    cmp #$02
    beq LoadLeapByKind117E_at0
    asl a
    tay
    lda a:LoadLeapByKind117ESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LoadLeapByKind117ESpeeds,y
    sta a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
LoadLeapByKind117E_at2:
    lda a:OBJ_FACING_LEFT,x
    beq LoadLeapByKind117E_at1
    jsr VecNegateVelocityX
    lda #$01
    sta a:OBJ_WORK_D,x
LoadLeapByKind117E_at1:
    rts
LoadLeapByKind117E_at0:
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:LoadLeapByKind117EReaches+1,y
    sta a:OBJ_VX_HI,x
    lda a:LoadLeapByKind117EReaches,y
    sta a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    bra LoadLeapByKind117E_at2

; @routine 12DD:482 WalkByStateList12DD
; @run 71:C482
; WHAT IT DOES: On each new frame of the state's list walks at OBJ_WORK_E's speed, turning at walls.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkByStateList12DDSpeeds = $C4D2 ; vx by OBJ_WORK_E
WalkByStateList12DDLists = $C4D6 ; frame lists by state from 2
WalkByStateList12DD:
    lda a:OBJ_STATE,x
    sec
    sbc #$02
    asl a
    tay
    lda a:WalkByStateList12DDLists,y
    sta z:zTemp10
    lda a:WalkByStateList12DDLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc WalkByStateList12DD_at0
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:WalkByStateList12DDSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkByStateList12DDSpeeds,y
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    lda #$06
    sta z:zProduct1
    lda #$02
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
    bne WalkByStateList12DD_at1
WalkByStateList12DD_at0:
    rts
WalkByStateList12DD_at1:
    jsr VecNegateVelocityX
    jmp VecApplyVelocityX

; @routine 161A:0BC BounceThenOpen161A
; @run 70:A8BC
; WHAT IT DOES: A hit piece bounces a few times (sound $19), then notes itself broken in zField8F and becomes what it holds.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BounceThenOpen161ACounts = $B0A2 ; bounces by kind of piece
BounceThenOpen161AHolds = $B0A7 ; what it becomes
BounceThenOpen161ASpeeds = $B0A8 ; vx and vy pairs
BounceThenOpen161A:
    lda #$19
    jsr VecSoundCommand
    lda #$01
    jsr StartDebrisBurst
    ldy a:OBJ_WORK_E,x
    inc a:OBJ_WORK_D,x
    lda a:OBJ_WORK_D,x
    cmp a:BounceThenOpen161ACounts,y
    bcs BounceThenOpen161A_at0
    lda a:OBJ_WORK_D,x
    dec a
    asl a
    clc
    adc a:OBJ_WORK_E,x
    tay
    lda a:BounceThenOpen161AHolds+1,y
    sta a:OBJ_VX_HI,x
    lda a:BounceThenOpen161ASpeeds+1,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    rts
BounceThenOpen161A_at0:
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    ora z:zField8F
    sta z:zField8F
    ldy a:OBJ_WORK_E,x
    lda a:BounceThenOpen161AHolds,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 161E:33B LoadColumnOfFour161E
; @run 71:CB3B
; WHAT IT DOES: The first of four starts three more, $20 apart to the left; the fourth shows frame 4.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadColumnOfFour161E:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_WORK_G,x
    and #$0F
    sta a:OBJ_START_ARG,x
    and #$03
    sta a:OBJ_WORK_D,x
    bne LoadColumnOfFour161E_at0
    lda #$03
    sta z:zTemp00
    txa
    inc a
    tay
LoadColumnOfFour161E_at1:
    jsr VecClearObjectInY
    lda #$A1
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_G-1,y
    inc a
    sta a:OBJ_START_ARG,y
    sta a:OBJ_WORK_G,y
    lda a:OBJ_X_LO-1,y
    sec
    sbc #$20
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI-1,y
    sbc #$00
    sta a:OBJ_X_HI,y
    iny
    dec z:zTemp00
    bne LoadColumnOfFour161E_at1
LoadColumnOfFour161E_at0:
    lda a:OBJ_WORK_D,x
    cmp #$03
    bne LoadColumnOfFour161E_at2
    lda #$04
    sta a:OBJ_FRAME,x
LoadColumnOfFour161E_at2:
    rts

; @routine 16CC:4A5 DropThenScatter16CC
; @run 70:BCA5
; WHAT IT DOES: Falls to Y $B8 and breaks into six numbered kind $2C pieces.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DropThenScatter16CC:
    lda a:OBJ_STATE,x
    bne DropThenScatter16CC_at0
    jsr VecLoadObjectRecord
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda #$1F
    sta a:OBJ_FRAME,x
    rts
DropThenScatter16CC_at0:
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp #$B8
    bcc DropThenScatter16CC_at1
    lda #$B8
    sta a:OBJ_Y_LO,x
    stz z:zTemp00
DropThenScatter16CC_at3:
    jsr VecFindFreeSlotFrom30
    bcc DropThenScatter16CC_at2
    lda #$2C
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    inc z:zTemp00
    lda z:zTemp00
    cmp #$06
    bcc DropThenScatter16CC_at3
DropThenScatter16CC_at2:
    jmp VecClearObject
DropThenScatter16CC_at1:
    rts
