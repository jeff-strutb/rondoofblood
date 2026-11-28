; @routine 088C:070 LoadSpeedThenFall088C
; @run 70:B870
; WHAT IT DOES: First loads with the speeds OBJ_WORK_D picks; then animates, falling without limit.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadSpeedThenFall088CSpeeds = $B8A8 ; vx and vy by OBJ_WORK_D, four bytes
LoadSpeedThenFall088C:
    lda a:OBJ_STATE,x
    bne LoadSpeedThenFall088C_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    tay
    lda a:LoadSpeedThenFall088CSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LoadSpeedThenFall088CSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:LoadSpeedThenFall088CSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:LoadSpeedThenFall088CSpeeds+2,y
    sta a:OBJ_VY_LO,x
    rts
LoadSpeedThenFall088C_at0:
    lda #$B8
    sta z:zTemp11
    lda #$E8
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0E0C:4FA ListThenScatter0E0C
; @run 70:BCFA
; WHAT IT DOES: At the end of the list at $BD2F starts the pieces, sound $25, and goes on, still and not hit-tested.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenScatter0E0C:
    lda #$2F
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenScatter0E0C_at0
    jsr StartSevenKind26_0E0C
    lda #$25
    jsr VecSoundCommand
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_SPRITE_ATTR,x
    ora #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda #$60
    sta a:OBJ_WORK_E,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_STATE,x
ListThenScatter0E0C_at0:
    rts

; @routine 0E0C:689 BounceOnFloor0E0C
; @run 70:BE89
; WHAT IT DOES: Every other frame falls; on the floor at Y $B9 bounces three times, then the state goes on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BounceOnFloor0E0CBounces = $BEE6 ; vy for each of three bounces
BounceOnFloor0E0C:
    jsr SpawnWithSoundAtRandom0E0C
    lda z:zWaveCounter
    and #$01
    bne BounceOnFloor0E0C_at0
    jsr VecApplyGravity
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$B9
    bcc BounceOnFloor0E0C_at0
    lda #$B9
    sta a:OBJ_Y_LO,x
    lda a:OBJ_WORK_D,x
    cmp #$03
    bcs BounceOnFloor0E0C_at1
    asl a
    tay
    lda a:BounceOnFloor0E0CBounces+1,y
    sta a:OBJ_VY_HI,x
    lda a:BounceOnFloor0E0CBounces,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_WORK_D,x
BounceOnFloor0E0C_at0:
    rts
BounceOnFloor0E0C_at1:
    inc a:OBJ_STATE,x
    rts

; @routine 0EBB:74C TumbleThenGone0EBB
; @run 70:B74C
; WHAT IT DOES: First knocked up with its X speed halved; then $A92E each count until OBJ_WORK_L runs out, and cleared.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TumbleThenGone0EBB:
    lda a:OBJ_STATE,x
    bne TumbleThenGone0EBB_at0
    jsr StartThrownItem
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_VY_LO,x
    clc
    adc #$00
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$03
    sta a:OBJ_VY_HI,x
    lda a:OBJ_VX_HI,x
    asl a
    ror a:OBJ_VX_HI,x
    ror a:OBJ_VX_LO,x
    lda #$18
    sta a:OBJ_WORK_L,x
    rts
TumbleThenGone0EBB_at0:
    dec a:OBJ_WORK_L,x
    bne TumbleThenGone0EBB_at1
    jmp VecClearObject
TumbleThenGone0EBB_at1:
    jmp StartThrownItem

; @routine 0F6C:53E ChooseNextAct0F6C
; @run 70:BD3E
; WHAT IT DOES: Runs the overlay's checks and list; at the list's end goes to $BC84; on marked animation steps runs $C377.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChooseNextAct0F6CSteps = $BD60 ; which animation steps act
ChooseNextAct0F6CDone = $BC84 ; a routine of this overlay
ChooseNextAct0F6C:
    jsr StartLeapWhenHurtAnswerYes
    bcc ChooseNextAct0F6C_at0
    rts
ChooseNextAct0F6C_at0:
    jsr HurtOnceThenAnswerYes
    bcc ChooseNextAct0F6C_at1
    rts
ChooseNextAct0F6C_at1:
    jsr StepListByNumberTableC585
    beq ChooseNextAct0F6C_at2
    bcc ChooseNextAct0F6C_at3
    ldy a:OBJ_ANIM_STEP,x
    lda a:ChooseNextAct0F6CSteps,y
    beq ChooseNextAct0F6C_at3
    jsr ToggleFacing
ChooseNextAct0F6C_at3:
    rts
ChooseNextAct0F6C_at2:
    jmp ChooseNextAct0F6CDone

; @routine 101F:007 StartMarkerLeft101F
; @run 71:D007
; WHAT IT DOES: Entered with Z clear: starts kind $BB two slots below, $50 to the left.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartMarkerLeft101F:
    beq StartMarkerLeft101F_at0
    txa
    dec a
    dec a
    tay
    jsr VecClearObjectInY
    lda #$BB
    jsr VecStartObjectAtPlace
    lda a:OBJ_X_LO,y
    sec
    sbc #$50
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI,y
    sbc #$00
    sta a:OBJ_X_HI,y
StartMarkerLeft101F_at0:
    rts

; @routine 101F:166 WakeOnRoute101F
; @run 71:D166
; WHAT IT DOES: State 1 waits for the route to be set: then list channel $83, script $87; the wave's phase is cleared otherwise and after.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WakeOnRoute101F:
    lda a:OBJ_STATE,x
    bne WakeOnRoute101F_at0
    jmp VecLoadObjectRecord
WakeOnRoute101F_at0:
    dec a
    bne WakeOnRoute101F_at1
    lda z:zField8C
    beq WakeOnRoute101F_at2
    lda a:CameraWavePhase
    bne WakeOnRoute101F_at2
    lda a:OBJ_KIND,x
    ora #$80
    sta a:OBJ_KIND,x
    inc a:OBJ_STATE,x
    lda #$83
    phx
    ldx #$06
    jsr VecStartListChannelKind2
    lda #$87
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
WakeOnRoute101F_at1:
    stz a:CameraWavePhase
WakeOnRoute101F_at2:
    rts

; @routine 117F:584 ChargeList117F
; @run 71:D584
; WHAT IT DOES: Runs the list at $D5BD; on each new frame moves at the facing's speed (sounds at step 9); at its end substate 2.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChargeList117FSpeeds = $D5BB ; vx high byte by facing
ChargeList117F:
    lda #$BD
    sta z:zTemp10
    lda #$D5
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ChargeList117F_at0
    bcc ChargeList117F_at1
    lda a:OBJ_ANIM_STEP,x
    cmp #$09
    bne ChargeList117F_at2
    lda #$E0
    jsr VecSoundCommand
    lda #$07
    jsr VecSoundCommand
ChargeList117F_at2:
    ldy a:OBJ_FACING_LEFT,x
    lda a:ChargeList117FSpeeds,y
    sta a:OBJ_VX_HI,x
    jmp ProbeHeightCall117F_46F
ChargeList117F_at0:
    lda #$40
    sta a:OBJ_WORK_D,x
    lda #$02
    sta a:OBJ_SUBSTATE,x
ChargeList117F_at1:
    rts

; @routine 12DC:50A FaceIfThenAttack12DC
; @run 70:BD0A
; WHAT IT DOES: Entered with Z set faces the player first; runs $BCDB; with carry clear and the player at least $70 across, state 2; the rest is FaceByBitThenNear12DC's code.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FaceIfThenAttack12DC:
    bne FaceByBitThenNear12DC+5
    jsr VecSideOfPlayer

; @routine 12DD:073 OrbitByTrail12DD
; @run 71:C073
; WHAT IT DOES: Runs $C0B3, then out at the angle the trail holds at this piece's lag, runs $C0FF; slot $10's OBJ_WORK_M counts the state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
OrbitByTrail12DDTrail = $BA41 ; the leader's angles
OrbitByTrail12DDLags = $C0A9 ; the lag in the trail by OBJ_WORK_D
OrbitByTrail12DD:
    jsr FollowSlot16Piece12DD_0B3
    ldy a:OBJ_WORK_D,x
    lda a:OrbitByTrail12DDLags,y
    tay
    stz z:zTemp00
    lda a:OBJ_WORK_I,x
    sta z:zTemp01
    lda a:OrbitByTrail12DDTrail,y
    sta a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda #$08
    sta z:zTemp00
    lda #$FF
    sta z:zTemp01
    jsr AddWhileNotAt12DD
    lda a:OBJ_WORK_M+$10
    beq OrbitByTrail12DD_at0
    lda #$18
    sta a:OBJ_WORK_H,x
    inc a:OBJ_STATE,x
OrbitByTrail12DD_at0:
    rts

; @routine 12DE:458 StrikeList12DE
; @run 71:CC58
; WHAT IT DOES: Runs the list at $CC8F, sound $1A at step 4; at its end, or at frame $75 with slot $11 free to, state 3.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StrikeList12DE:
    lda #$8F
    sta z:zTemp10
    lda #$CC
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StrikeList12DE_at0
    bcc StrikeList12DE_at1
    lda a:OBJ_ANIM_STEP,x
    cmp #$04
    bne StrikeList12DE_at2
    lda #$1A
    jsr VecSoundCommand
    rts
StrikeList12DE_at2:
    lda a:OBJ_FRAME,x
    cmp #$75
    bne StrikeList12DE_at1
    lda a:OBJ_WORK_D+$11
    bne StrikeList12DE_at1
StrikeList12DE_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StrikeList12DE_at1:
    rts

; @routine 161E:07F BoxByPreviousFrame161E
; @run 71:C87F
; WHAT IT DOES: Takes the previous slot's place and facing, and its hit box by that slot's frame (from $57).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BoxByPreviousFrame161EBoxes = $C8B5 ; hit box by frame, four bytes
BoxByPreviousFrame161E:
    lda a:OBJ_STATE,x
    bne BoxByPreviousFrame161E_at0
    jmp VecLoadObjectRecord
BoxByPreviousFrame161E_at0:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FACING_LEFT-1,x
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_FRAME-1,x
    sec
    sbc #$57
    asl a
    asl a
    tay
    lda a:BoxByPreviousFrame161EBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:BoxByPreviousFrame161EBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:BoxByPreviousFrame161EBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:BoxByPreviousFrame161EBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 161E:6B8 PushWithin161E
; @run 71:CEB8
; WHAT IT DOES: Within $14 across and touching: pushes the player along, X by the start argument, Y by the object's speed.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PushWithin161EPushes = $CEEA ; push along X by start argument
PushWithin161EHeights = $CEEC ; the point's height by distance
PushWithin161E:
    jsr VecDistanceXToPlayer
    cmp #$14
    bcs PushWithin161E_at0
    tay
    lda a:PushWithin161EHeights,y
    clc
    adc #$12
    sta z:zTemp09
    lda #$17
    sta z:zTemp08
    jsr ContactWithPlayer
    bcc PushWithin161E_at0
    lda z:zPlayerPush
    ora #$30
    sta z:zPlayerPush
    ldy a:OBJ_START_ARG,x
    lda a:PushWithin161EPushes,y
    sta z:zPushXHi
    lda a:OBJ_VY_HI,x
    sta z:zPushYHi
    lda a:OBJ_VY_LO,x
    sta z:zPushYLo
PushWithin161E_at0:
    rts

; @routine 16CC:0CA RiseListSlowing16CC
; @run 70:B8CA
; WHAT IT DOES: Moves along Y taking $10 off the speed; when it turns negative: sound $8F, a wait of $40, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RiseListSlowing16CC:
    jsr PlaceChain16CC
    lda #$01
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    bne RiseListSlowing16CC_at0
    lda a:OBJ_VY_LO,x
    sec
    sbc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    bcs RiseListSlowing16CC_at0
    lda #$8F
    jsr VecSoundCommand
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
RiseListSlowing16CC_at0:
    rts

; @routine 18DB:3B1 SpitByStep18DB
; @run 70:B3B1
; WHAT IT DOES: At animation step 7 starts kind $27 in slot $33; at step 8 sound $26 and kind $28 in slots $30-$32.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpitByStep18DB:
    lda a:OBJ_ANIM_STEP,x
    cmp #$07
    beq SpitByStep18DB_at0
    cmp #$08
    bne SpitByStep18DB_at1
    lda #$26
    jsr VecSoundCommand
    ldy #$30
SpitByStep18DB_at2:
    jsr VecClearObjectInY
    lda #$28
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    iny
    cpy #$33
    bcc SpitByStep18DB_at2
SpitByStep18DB_at1:
    rts
SpitByStep18DB_at0:
    ldy #$33
    jsr VecClearObjectInY
    lda #$27
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    rts

; @routine 18DD:1AD TakeFromPool18DD
; @run 71:C1AD
; WHAT IT DOES: Takes one marked entry (bit 7) of the 112 at $3F90, 16 random tries and then a search; with none left a count of $40 in OBJ_WORK_E, the raster effect off, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TakeFromPool18DDPool = $3F90 ; 112 entries
TakeFromPool18DD:
    lda #$10
    sta z:zTemp00
TakeFromPool18DD_at0:
    jsr VecNextRandom
    and #$7F
    cmp #$70
    bcs TakeFromPool18DD_at0
    tay
    lda a:TakeFromPool18DDPool,y
    bmi TakeFromPool18DD_at1
    dec z:zTemp00
    bne TakeFromPool18DD_at0
    cly
TakeFromPool18DD_at2:
    lda a:TakeFromPool18DDPool,y
    bmi TakeFromPool18DD_at1
    iny
    cpy #$70
    bne TakeFromPool18DD_at2
    lda #$40
    sta a:OBJ_WORK_E,x
    stz z:zRasterEffect
    inc a:OBJ_STATE,x
    rts
TakeFromPool18DD_at1:
    cla
    sta a:TakeFromPool18DDPool,y
    rts
