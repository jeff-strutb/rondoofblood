; @routine 072C:39F FallUntilScreen4_072C
; @run 70:BB9F
; WHAT IT DOES: Falls four screens, running $BBC7; then new counts and the substate on (secret stage 03's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallUntilScreen4_072C:
    lda #$05
    sta a:OBJ_WORK_I,x
    jsr VecApplyGravity
    jsr VecApplyVelocityY
    jsr SpawnEveryFew072C_3C7
    lda a:OBJ_Y_HI,x
    cmp #$04
    bne FallUntilScreen4_072C_at0
    lda #$00
    sta a:OBJ_WORK_N,x
    lda #$06
    sta a:OBJ_WORK_I,x
    lda #$30
    sta a:OBJ_WORK_J,x
    inc a:OBJ_SUBSTATE,x
FallUntilScreen4_072C_at0:
    rts

; @routine 088B:665 MoveOrAimShot088B
; @run 70:B665
; WHAT IT DOES: Moving: moves; else loads, aims at the player at speed 6 with the frame of that angle, sound $2E (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
MoveOrAimShot088B:
    lda a:OBJ_STATE,x
    beq MoveOrAimShot088B_at0
    jmp VecApplyVelocity
MoveOrAimShot088B_at0:
    jsr VecLoadObjectRecord
    jsr VecAngleToPlayer
    pha
    lsr a
    lsr a
    lsr a
    clc
    adc #$01
    sta a:OBJ_FRAME,x
    lda #$06
    sta z:zTemp01
    lda #$00
    sta z:zTemp00
    pla
    jsr VecSetVelocityFromAngle
    lda #$2E
    jsr VecSoundCommand
    rts

; @routine 09EB:3F2 RunListThenState13_09EB
; @run 70:B3F2
; WHAT IT DOES: The routine at $AA35 and the list at $B41C; when it ends state $D, at least 3 frames not hit-tested (secret stage 05's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RunListThenState13_09EB:
    lda #$20
    sta z:zTemp08
    lda #$2C
    sta z:zTemp09
    jsr ContactWithPlayer
    lda #$B4
    sta z:zTemp11
    lda #$1C
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne RunListThenState13_09EB_at0
    lda #$0D
    sta a:OBJ_STATE,x
    lda a:OBJ_INVULN_TIMER,x
    cmp #$03
    bcs RunListThenState13_09EB_at0
    lda #$03
    sta a:OBJ_INVULN_TIMER,x
RunListThenState13_09EB_at0:
    rts

; @routine 0E0B:5A8 ListShootAtFrame3B_0E0B
; @run 70:B5A8
; WHAT IT DOES: Plays the list at $B5D2; at frame $3B sound $CF and a kind $25 object; when it ends, substate on (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListShootAtFrame3B_0E0B:
    lda #$D2
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListShootAtFrame3B_0E0B_at0
    bcc ListShootAtFrame3B_0E0B_at1
    lda a:OBJ_FRAME,x
    cmp #$3B
    bne ListShootAtFrame3B_0E0B_at1
    lda #$CF
    jsr VecSoundCommand
    ldy #$50
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    rts
ListShootAtFrame3B_0E0B_at0:
    inc a:OBJ_SUBSTATE,x
ListShootAtFrame3B_0E0B_at1:
    rts

; @routine 0E0B:661 DripAtFrame26_0E0B
; @run 70:B661
; WHAT IT DOES: At frame $26, every fourth count, a kind $28 object at the next of 16 X places (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DripAtFrame26_0E0BPlaces = $B68B ; 16 X lows
DripAtFrame26_0E0B:
    lda a:OBJ_FRAME,x
    cmp #$26
    bne DripAtFrame26_0E0B_at0
    lda a:OBJ_WORK_F,x
    and #$03
    bne DripAtFrame26_0E0B_at0
    jsr VecFindFreeSlotFrom30
    bcc DripAtFrame26_0E0B_at0
    lda #$28
    jsr VecStartObjectAtPlace
    phx
    lda a:OBJ_WORK_J,x
    and #$0F
    tax
    lda a:DripAtFrame26_0E0BPlaces,x
    sta a:OBJ_X_LO,y
    plx
    inc a:OBJ_WORK_J,x
DripAtFrame26_0E0B_at0:
    rts

; @routine 0EBD:1AE ListThenWhenNear0EBD
; @run 71:C1AE
; WHAT IT DOES: Outside sub-mode 3: hit-tested, playing the list at $C223; within $70 dots the state on (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenWhenNear0EBD:
    lda z:zGameSubMode
    cmp #$03
    bne ListThenWhenNear0EBD_at0
    rts
ListThenWhenNear0EBD_at0:
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    stz a:OBJ_LOOP_FLAGS,x
    lda #$23
    sta z:zTemp10
    lda #$C2
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenWhenNear0EBD_at1
    jsr VecDistanceXToPlayer
    cmp #$70
    bcs ListThenWhenNear0EBD_at1
    inc a:OBJ_STATE,x
    stz a:OBJ_HITBOX_ON,x
ListThenWhenNear0EBD_at1:
    rts

; @routine 0EBE:04B RideSourceOrState5_0EBE
; @run 71:C84B
; WHAT IT DOES: Rides 20 dots below the slot the start argument names while it is kind $2C; else stops, state 5 (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RideSourceOrState5_0EBE:
    ldy a:OBJ_START_ARG,x
    beq RideSourceOrState5_0EBE_at0
    lda a:OBJ_KIND,y
    cmp #$2C
    bne RideSourceOrState5_0EBE_at0
    jsr VecCopyObjectPlaceBack
    lda #$14
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    rts
RideSourceOrState5_0EBE_at0:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$05
    sta a:OBJ_STATE,x
    rts

; @routine 10CC:273 SpawnEvery48_10CC
; @run 70:BA73
; WHAT IT DOES: Every 49th call, a kind $26 object in the first empty of slots $18 to $1A (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnEvery48_10CC:
    lda a:OBJ_WORK_M,x
    beq SpawnEvery48_10CC_at0
    dec a:OBJ_WORK_M,x
    rts
SpawnEvery48_10CC_at0:
    lda #$30
    sta a:OBJ_WORK_M,x
    ldy #$18
SpawnEvery48_10CC_at2:
    lda a:OBJ_KIND,y
    beq SpawnEvery48_10CC_at1
    iny
    cpy #$1B
    bcc SpawnEvery48_10CC_at2
    rts
SpawnEvery48_10CC_at1:
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_START_ARG,y
    rts

; @routine 117B:68D TurnAround117B
; @run 70:B68D
; WHAT IT DOES: Turns the object around with a delay by the player's distance, substate on (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnAround117BSecond = $B722 ; a routine of this overlay
TurnAround117B:
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_K,x
    jsr VecPlayerDistanceSum
    and #$0F
    sta a:OBJ_WORK_I,x
    jsr PickSpeedAndTurnFromTable
    jsr VecNegateVelocityX
    jsr VecNegateVelocityY
    jsr TurnAround117BSecond
    lda #$0C
    sta a:OBJ_WORK_D,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 117C:5E1 ThrowThenLandRow117C
; @run 70:BDE1
; WHAT IT DOES: Throws the object up; then it falls, running $BBD8 on the $Dx row and $BBB1 after (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ThrowThenLandRow117C:
    lda a:OBJ_SUBSTATE,x
    bne ThrowThenLandRow117C_at0
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
    rts
ThrowThenLandRow117C_at0:
    lda a:OBJ_Y_LO,x
    and #$F0
    cmp #$D0
    bne ThrowThenLandRow117C_at1
    jsr LandOnRowThenDebris117C
ThrowThenLandRow117C_at1:
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocity
    jmp FrameOneBWhenFallingElseAnimate

; @routine 117D:15A ListLandThen117D
; @run 71:C15A
; WHAT IT DOES: Plays the list at $C182 falling; on landing sound 2, OBJ_WORK_G 24, frame $3B, substate on (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListLandThen117D:
    lda #$C1
    sta z:zTemp11
    lda #$82
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr LandOnRowThenDebris117C
    lda #$0C
    jsr VecStopOnLandingFrontColumn
    bcc ListLandThen117D_at0
    lda #$02
    jsr VecSoundCommand
    lda #$18
    sta a:OBJ_WORK_G,x
    lda #$3B
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
ListLandThen117D_at0:
    rts

; @routine 117D:515 LoadFacingWithPrevious117D
; @run 71:C515
; WHAT IT DOES: Loads facing the player with a kind $33 object in the slot before; start argument 2 sets attribute $87 (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFacingWithPrevious117D:
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$40
    sta a:OBJ_WORK_D,x
    txa
    tay
    dey
    jsr VecClearObjectInY
    lda #$33
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    cmp #$02
    bne LoadFacingWithPrevious117D_at0
    lda #$87
    sta a:OBJ_SPRITE_ATTR,x
LoadFacingWithPrevious117D_at0:
    rts

; @routine 117E:221 LoadAndStart30_117E
; @run 71:CA21
; WHAT IT DOES: Loads with a kind $30 object in the next slot; a start argument takes the player's side and state 8 (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAndStart30_117E:
    jsr VecLoadObjectRecord
    lda #$B0
    sta a:OBJ_WORK_E,x
    lda #$40
    sta a:OBJ_WORK_F,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$30
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    beq LoadAndStart30_117E_at0
    jsr VecSideOfPlayer
    lsr a
    sta a:OBJ_WORK_I,x
    lda #$08
    sta a:OBJ_STATE,x
LoadAndStart30_117E_at0:
    rts

; @routine 117E:4D9 ListKeepingPace117E
; @run 71:CCD9
; WHAT IT DOES: Plays the list at $CD02 with OBJ_PACE bits 0-1 set; then clears them, state 5 (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListKeepingPace117E:
    lda a:OBJ_PACE,x
    ora #$03
    sta a:OBJ_PACE,x
    lda #$CD
    sta z:zTemp11
    lda #$02
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne ListKeepingPace117E_at0
    lda a:OBJ_PACE,x
    and #$FC
    sta a:OBJ_PACE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
ListKeepingPace117E_at0:
    rts

; @routine 117F:416 ListDashThenState117F
; @run 71:D416
; WHAT IT DOES: Dashes left on the list's new frames until X under $C0 on the first screen; then the state on (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListDashThenState117F:
    lda #$79
    sta z:zTemp10
    lda #$D4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc ListDashThenState117F_at0
    lda #$FB
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda a:OBJ_X_HI,x
    bne ListDashThenState117F_at0
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcs ListDashThenState117F_at0
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListDashThenState117F_at0:
    rts

; @routine 12DC:45C MoveBounceOffWall12DC
; @run 70:BC5C
; WHAT IT DOES: Moves along X; a wall met on the way it goes turns it back (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
MoveBounceOffWall12DC:
    jsr VecApplyVelocityX
    lda #$04
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
    bne MoveBounceOffWall12DC_at0
    rts
MoveBounceOffWall12DC_at0:
    jsr VecNegateVelocityX
    jmp VecApplyVelocityX

; @routine 12DC:7B6 ScriptsAtCounts12DC
; @run 70:BFB6
; WHAT IT DOES: While OBJ_WORK_L counts, scripts $8B-$8E at the table's counts; at zero 128 and substate on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ScriptsAtCounts12DCCounts = $BFDF ; four counts that start a script
ScriptsAtCounts12DC:
    dec a:OBJ_WORK_L,x
    beq ScriptsAtCounts12DC_at0
    cly
ScriptsAtCounts12DC_at2:
    lda a:OBJ_WORK_L,x
    cmp a:ScriptsAtCounts12DCCounts,y
    beq ScriptsAtCounts12DC_at1
    iny
    cpy #$04
    bcc ScriptsAtCounts12DC_at2
    rts
ScriptsAtCounts12DC_at1:
    tya
    clc
    adc #$8B
    phx
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    rts
ScriptsAtCounts12DC_at0:
    lda #$80
    sta a:OBJ_WORK_L,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 13FB:678 ListThenHop13FB
; @run 70:B678
; WHAT IT DOES: Plays the list at $B6A2; then hops by facing, substate on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenHop13FBSpeeds = $B6A7 ; X speed words by facing
ListThenHop13FB:
    lda #$A2
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenHop13FB_at0
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:ListThenHop13FBSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:ListThenHop13FBSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
ListThenHop13FB_at0:
    rts

; @routine 13FC:070 ProbeSpawnListWhenAlone13FC
; @run 70:B870
; WHAT IT DOES: Falls ($B6CC), spawns ($B493), plays the list at $B89A until slots $11 and $12 are empty (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ProbeSpawnListWhenAlone13FC:
    jsr FallProbeToA8_13FB
    jsr SpawnOddFramesAnswer13FB
    bcc ProbeSpawnListWhenAlone13FC_at0
    lda #$01
    sta a:OBJ_START_ARG,y
ProbeSpawnListWhenAlone13FC_at0:
    lda #$9A
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_KIND+17
    ora a:OBJ_KIND+18
    bne ProbeSpawnListWhenAlone13FC_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
ProbeSpawnListWhenAlone13FC_at1:
    rts

; @routine 161C:437 ListByStateThenProbe161C
; @run 70:BC37
; WHAT IT DOES: The list for the state from 4; on new frames the X speed by OBJ_WORK_E and a probe (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListByStateThenProbe161CSpeeds = $BC5F ; X speed high byte by OBJ_WORK_E
ListByStateThenProbe161CLists = $BC61 ; a frame list per state from 4
ListByStateThenProbe161C:
    lda a:OBJ_STATE,x
    sec
    sbc #$04
    asl a
    tay
    lda a:ListByStateThenProbe161CLists,y
    sta z:zTemp10
    lda a:ListByStateThenProbe161CLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc ListByStateThenProbe161C_at0
    ldy a:OBJ_WORK_E,x
    lda a:ListByStateThenProbe161CSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$0A
    sta z:zProduct1
    jsr BounceAtWalls
ListByStateThenProbe161C_at0:
    rts

; @routine 161D:211 ListThenAim161D
; @run 71:C211
; WHAT IT DOES: Moves while the list at $C238 plays; then aims at the player at speed 4, OBJ_WORK_D 48, substate on (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenAim161D:
    jsr VecApplyVelocityY
    lda #$38
    sta z:zTemp10
    lda #$C2
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenAim161D_at0
    jsr VecAngleToPlayer
    pha
    stz z:zTemp00
    lda #$04
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    lda #$30
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
ListThenAim161D_at0:
    rts

; @routine 161E:5E5 LoadThenListClear161E
; @run 71:CDE5
; WHAT IT DOES: In state 0 loads rising; later moves playing the list at $CE0E and is cleared when it ends (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadThenListClear161E:
    lda a:OBJ_STATE,x
    bne LoadThenListClear161E_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$0D
    sta a:OBJ_FRAME,x
    dec a:OBJ_VY_HI,x
LoadThenListClear161E_at1:
    rts
LoadThenListClear161E_at0:
    jsr VecApplyVelocity
    lda #$0E
    sta z:zTemp10
    lda #$CE
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LoadThenListClear161E_at1
    jmp VecClearObject

; @routine 18DB:2DE CountThenDrop26_18DB
; @run 70:B2DE
; WHAT IT DOES: Counts OBJ_WORK_E; then moves to a random X, starts a kind $26 object in slot $18, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenDrop26_18DBPlaces = $B305 ; eight X lows
CountThenDrop26_18DB:
    dec a:OBJ_WORK_E,x
    bne CountThenDrop26_18DB_at0
    jsr VecNextRandom
    and #$07
    tay
    lda a:CountThenDrop26_18DBPlaces,y
    sta a:OBJ_X_LO,x
    lda #$76
    sta a:OBJ_Y_LO,x
    ldy #$18
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    inc a:OBJ_WORK_F,x
    inc a:OBJ_SUBSTATE,x
CountThenDrop26_18DB_at0:
    rts

; @routine 18DB:3F9 ListWithChecksThenSpawn18DB
; @run 70:B3F9
; WHAT IT DOES: Plays the list at $B422, on new frames setting the hit box and starting; when it ends a kind $26 object, substate back (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListWithChecksThenSpawn18DB:
    lda #$22
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListWithChecksThenSpawn18DB_at0
    bcc ListWithChecksThenSpawn18DB_at1
    jsr HitBoxByAnimationStep18DB
    jsr StartByAnimationStep18DB
ListWithChecksThenSpawn18DB_at1:
    rts
ListWithChecksThenSpawn18DB_at0:
    ldy #$18
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_START_ARG,y
    dec a:OBJ_SUBSTATE,x
    rts

; @routine 18DC:082 ListThenSpawn27_18DC
; @run 70:B882
; WHAT IT DOES: Plays the list at $B8AB; then a kind $27 object in slot $18 with the facing, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenSpawn27_18DC:
    lda #$AB
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenSpawn27_18DC_at0
    ldy #$18
    jsr VecClearObjectInY
    lda #$27
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_START_ARG,y
    sta a:OBJ_WORK_J,x
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    inc a:OBJ_SUBSTATE,x
ListThenSpawn27_18DC_at0:
    rts

; @routine 18DC:0B6 SpawnWhenSlot24Gone18DC
; @run 70:B8B6
; WHAT IT DOES: Once slot $18 is empty: sound $26, a kind $2C object in slot $20, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnWhenSlot24Gone18DC:
    lda a:OBJ_KIND+24
    bne SpawnWhenSlot24Gone18DC_at0
    lda #$26
    jsr VecSoundCommand
    ldy #$20
    jsr VecClearObjectInY
    lda #$2C
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda a:OBJ_WORK_I,x
    sta a:OBJ_START_ARG,y
    lda #$38
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
SpawnWhenSlot24Gone18DC_at0:
    rts

; @routine 18DC:249 ListSoundThenSpawn30_18DC
; @run 70:BA49
; WHAT IT DOES: Plays the list at $BA73, sound $90 at step 2; when it ends a kind $30 object in slot $30, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSoundThenSpawn30_18DC:
    lda #$73
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListSoundThenSpawn30_18DC_at0
    bcc ListSoundThenSpawn30_18DC_at1
    lda a:OBJ_ANIM_STEP,x
    cmp #$02
    bne ListSoundThenSpawn30_18DC_at1
    lda #$90
    jsr VecSoundCommand
    rts
ListSoundThenSpawn30_18DC_at0:
    ldy #$30
    jsr VecClearObjectInY
    lda #$30
    jsr VecStartObjectAtPlace
    inc a:OBJ_SUBSTATE,x
ListSoundThenSpawn30_18DC_at1:
    rts
