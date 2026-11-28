; @routine 088B:5D5 LandAtB5_088B
; @run 70:B5D5
; WHAT IT DOES: Falls until landing or Y $B5, then sound $9C and substate on (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandAtB5_088B:
    lda a:OBJ_Y_LO,x
    cmp #$B5
    bcs LandAtB5_088B_at0
    lda #$0A
    sta a:OBJ_FRAME,x
    lda #$04
    jsr VecStopOnLanding
    bcc LandAtB5_088B_at1
LandAtB5_088B_at0:
    lda #$9C
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$B5
    sta a:OBJ_Y_LO,x
    inc a:OBJ_SUBSTATE,x
LandAtB5_088B_at1:
    rts

; @routine 09EC:3D9 LoadFaceOrListClear09EC
; @run 70:BBD9
; WHAT IT DOES: In state 0 loads with a random facing; later follows slot $10 playing the list at $BC00, then cleared (secret stage 05's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFaceOrListClear09EC:
    lda a:OBJ_STATE,x
    bne LoadFaceOrListClear09EC_at0
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    and #$01
    sta a:OBJ_FACING_LEFT,x
    rts
LoadFaceOrListClear09EC_at0:
    ldy #$10
    jsr VecCopyObjectPlaceBack
    lda #$00
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LoadFaceOrListClear09EC_at1
    jmp VecClearObject
LoadFaceOrListClear09EC_at1:
    rts

; @routine 0B4C:4FC StartPairFacing0B4C
; @run 70:BCFC
; WHAT IT DOES: Faces the player and starts a kind $2D object in slot $51 and a $2E in slot $11, substate on (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartPairFacing0B4C:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    ldy #$51
    jsr VecClearObjectInY
    lda #$2D
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    ldy #$11
    jsr VecClearObjectInY
    lda #$2E
    jsr VecStartObjectAtPlace
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0B4D:13F CountThenStart21_0B4D
; @run 71:C13F
; WHAT IT DOES: Counts OBJ_WORK_D: every other count steps a phase and runs $C164; at zero a kind $21 object in slot $11 and cleared (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenStart21_0B4D:
    dec a:OBJ_WORK_D,x
    bne CountThenStart21_0B4D_at0
    lda #$21
    ldy #$11
    jsr VecStartObjectAtPlace
    jmp VecClearObject
CountThenStart21_0B4D_at0:
    lda a:OBJ_WORK_D,x
    and #$01
    bne CountThenStart21_0B4D_at1
    lda a:OBJ_WORK_E,x
    clc
    adc #$13
    and #$3F
    sta a:OBJ_WORK_E,x
    jsr EachOfPatterns0B4D_164
CountThenStart21_0B4D_at1:
    rts

; @routine 0CAB:768 ListByFacingThenState3_0CAB
; @run 70:B768
; WHAT IT DOES: Plays the facing's frame list; then state 3 (secret stage 07's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListByFacingThenState3_0CABLists = $B78F ; a frame list per facing
ListByFacingThenState3_0CAB:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:ListByFacingThenState3_0CABLists,y
    sta z:zTemp10
    lda a:ListByFacingThenState3_0CABLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListByFacingThenState3_0CAB_at0
    lda #$02
    sta a:OBJ_WORK_A,x
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListByFacingThenState3_0CAB_at0:
    rts

; @routine 0E0B:4E9 ListLandSounds0E0B
; @run 70:B4E9
; WHAT IT DOES: Plays the list at $B50F while falling; on landing sounds $CE and $4A, substate on (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListLandSounds0E0B:
    lda #$0F
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$04
    jsr VecStopOnLanding
    bcc ListLandSounds0E0B_at0
    lda #$CE
    jsr VecSoundCommand
    lda #$4A
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
ListLandSounds0E0B_at0:
    rts

; @routine 0E0B:735 ListAtFrame40Spawn0E0B
; @run 70:B735
; WHAT IT DOES: Plays the list at $B75A; at its frame $40 a kind $2A object in slot $50; when it ends, substate on (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListAtFrame40Spawn0E0B:
    lda #$5A
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListAtFrame40Spawn0E0B_at0
    bcc ListAtFrame40Spawn0E0B_at1
    lda a:OBJ_FRAME,x
    cmp #$40
    bne ListAtFrame40Spawn0E0B_at1
    ldy #$50
    jsr VecClearObjectInY
    lda #$2A
    jsr VecStartObjectAtPlace
    rts
ListAtFrame40Spawn0E0B_at0:
    inc a:OBJ_SUBSTATE,x
ListAtFrame40Spawn0E0B_at1:
    rts

; @routine 0E0B:7C6 DissolveThenBecome21_0E0B
; @run 70:B7C6
; WHAT IT DOES: Dissolves patterns 0 to $11 while OBJ_WORK_D counts; then the slot becomes kind $21 (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DissolveThenBecome21_0E0B:
    lda #$00
    sta a:WorkByteA
    lda #$12
    sta a:WorkByteB
    jsr DissolveStep0E0B7ED
    dec a:OBJ_WORK_D,x
    bne DissolveThenBecome21_0E0B_at0
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$21
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
DissolveThenBecome21_0E0B_at0:
    rts

; @routine 0EBF:1E4 StateByPlace0EBF
; @run 71:D1E4
; WHAT IT DOES: State 3 off the screen along X, 4 along Y, else by the player's distance from a table (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StateByPlace0EBFStates = $D20A ; the state by the distance's low four bits
StateByPlace0EBF:
    stz a:OBJ_SUBSTATE,x
    lda a:OBJ_X_HI,x
    beq StateByPlace0EBF_at0
    lda #$03
    sta a:OBJ_STATE,x
    rts
StateByPlace0EBF_at0:
    lda a:OBJ_Y_HI,x
    beq StateByPlace0EBF_at1
    lda #$04
    sta a:OBJ_STATE,x
    rts
StateByPlace0EBF_at1:
    jsr VecPlayerDistanceSum
    and #$0F
    tay
    lda a:StateByPlace0EBFStates,y
    sta a:OBJ_STATE,x
    rts

; @routine 101C:46F ListSpawnOrEnd101C
; @run 70:BC6F
; WHAT IT DOES: Plays the list at $BC95; on new frames the spawn at $BC4D for step 8; when it ends, state 1 (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSpawnOrEnd101C:
    lda #$95
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListSpawnOrEnd101C_at0
    bcc ListSpawnOrEnd101C_at1
    lda #$08
    sta z:zTemp00
    stz z:zTemp01
    jmp SpawnAtStep101C
ListSpawnOrEnd101C_at0:
    lda #$74
    sta a:OBJ_FRAME,x
    lda #$01
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListSpawnOrEnd101C_at1:
    rts

; @routine 101C:504 StartRowThenClear101C
; @run 70:BD04
; WHAT IT DOES: Starts eight kind $25 objects numbered in their start arguments, then clears itself (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartRowThenClear101C:
    stz z:zTemp00
StartRowThenClear101C_at1:
    jsr VecFindFreeSlotFrom30
    bcc StartRowThenClear101C_at0
    lda #$25
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    lda #$8C
    sta a:OBJ_WORK_E,y
    inc z:zTemp00
    lda z:zTemp00
    cmp #$08
    bcc StartRowThenClear101C_at1
StartRowThenClear101C_at0:
    jmp VecClearObject

; @routine 101C:5F9 FallAndScatter101C
; @run 70:BDF9
; WHAT IT DOES: Falls and every fourth frame throws a kind $9B object to either side (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallAndScatter101C:
    jsr VecApplyGravity
    jsr VecApplyVelocity
    lda z:zWaveCounter
    and #$03
    bne FallAndScatter101C_at0
    jsr VecFindFreeSlotFrom30
    bcc FallAndScatter101C_at0
    lda #$9B
    jsr VecStartObjectAtPlace
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$08
    sta a:OBJ_VX_HI,y
    sxy
    jsr VecApplyVelocityX
    sxy
FallAndScatter101C_at0:
    rts

; @routine 101E:3AF LoadFrameByWorkD101E
; @run 71:CBAF
; WHAT IT DOES: Loads the record with slot $10's flip; the frame by OBJ_WORK_D, hit-tested only for piece $E (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFrameByWorkD101EFrames = $CBD6 ; the frame by OBJ_WORK_D
LoadFrameByWorkD101E:
    jsr VecLoadObjectRecord
    lda a:OBJ_SPRITE_FLIP+16
    sta a:OBJ_SPRITE_FLIP,x
    lda a:OBJ_WORK_D,x
    cmp #$0E
    beq LoadFrameByWorkD101E_at0
    stz a:OBJ_HITBOX_ON,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
LoadFrameByWorkD101E_at0:
    ldy a:OBJ_WORK_D,x
    lda a:LoadFrameByWorkD101EFrames,y
    sta a:OBJ_FRAME,x
    lda #$05
    sta a:OBJ_WORK_I,x
    rts

; @routine 101E:553 StartSix9A_101E
; @run 71:CD53
; WHAT IT DOES: Starts up to six kind $9A objects numbered in OBJ_WORK_E, Y kept (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartSix9A_101E:
    phy
    cly
StartSix9A_101E_at1:
    sty z:zSpreadCount
    jsr VecFindFreeSlotFrom30
    bcc StartSix9A_101E_at0
    lda #$01
    sta a:OBJ_WORK_D,y
    lda #$9A
    jsr VecStartObjectAtPlace
    lda #$80
    sta a:OBJ_SPRITE_FLIP,y
    lda z:zSpreadCount
    sta a:OBJ_WORK_E,y
    ldy z:zSpreadCount
    iny
    cpy #$06
    bcc StartSix9A_101E_at1
StartSix9A_101E_at0:
    ply
    rts

; @routine 10CB:383 ScriptByFrameMatch10CB
; @run 70:B383
; WHAT IT DOES: Counts OBJ_WORK_F; when the count is in the table, script $83 plus its place (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ScriptByFrameMatch10CBCounts = $B3A8 ; 22 counts that start a script
ScriptByFrameMatch10CB:
    lda a:OBJ_FRAME,x
    beq ScriptByFrameMatch10CB_at0
    lda a:OBJ_WORK_F,x
    bmi ScriptByFrameMatch10CB_at0
    inc a:OBJ_WORK_F,x
    cly
ScriptByFrameMatch10CB_at2:
    cmp a:ScriptByFrameMatch10CBCounts,y
    beq ScriptByFrameMatch10CB_at1
    iny
    cpy #$16
    bcc ScriptByFrameMatch10CB_at2
ScriptByFrameMatch10CB_at0:
    rts
ScriptByFrameMatch10CB_at1:
    tya
    clc
    adc #$83
    phx
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    rts

; @routine 10CC:42B ListLandScript10CC
; @run 70:BC2B
; WHAT IT DOES: Plays the list at $BC50 falling; on landing script $AF and substate 1 (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListLandScript10CC:
    lda #$50
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr LandBelow90OrFall10CC
    bcc ListLandScript10CC_at0
    phx
    lda #$AF
    ldx #$08
    jsr VecStartScriptSlotKind1
    plx
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$01
    sta a:OBJ_SUBSTATE,x
ListLandScript10CC_at0:
    rts

; @routine 10CC:512 LandThenStartPair10CC
; @run 70:BD12
; WHAT IT DOES: On landing, frame $9C and kind $2F and $30 objects in slots $11 and $12, substate on (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandThenStartPair10CC:
    jsr LandBelow90OrFall10CC
    bcc LandThenStartPair10CC_at0
    lda #$9C
    sta a:OBJ_FRAME,x
    ldy #$11
    jsr VecClearObjectInY
    lda #$2F
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    iny
    jsr VecClearObjectInY
    lda #$30
    jsr VecStartObjectAtPlace
    inc a:OBJ_SUBSTATE,x
LandThenStartPair10CC_at0:
    rts

; @routine 10CC:5BA DissolveThenBecome29_10CC
; @run 70:BDBA
; WHAT IT DOES: Dissolves patterns 0 to $D while OBJ_WORK_E counts; then the slot becomes kind $29 (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DissolveThenBecome29_10CC:
    lda #$00
    sta a:WorkByteA
    lda #$0E
    sta a:WorkByteB
    jsr DissolveStep10CC5E1
    dec a:OBJ_WORK_E,x
    bne DissolveThenBecome29_10CC_at0
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$29
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
DissolveThenBecome29_10CC_at0:
    rts

; @routine 10CD:217 LoadOrChannelWhenFrame71_10CD
; @run 71:C217
; WHAT IT DOES: In state 0 loads with scroll band 1 and frame $71; cleared once slot $10 shows it, else list channel $9C (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadOrChannelWhenFrame71_10CD:
    lda a:OBJ_STATE,x
    bne LoadOrChannelWhenFrame71_10CD_at0
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_SCROLL_BAND,x
    lda #$71
    sta a:OBJ_FRAME,x
    rts
LoadOrChannelWhenFrame71_10CD_at0:
    lda a:OBJ_FRAME+16
    cmp #$71
    bne LoadOrChannelWhenFrame71_10CD_at1
    jmp VecClearObject
LoadOrChannelWhenFrame71_10CD_at1:
    phx
    lda #$9C
    ldx #$04
    jsr VecStartListChannelKind2
    plx
    rts

; @routine 117A:6FC RasterByLevelX117A
; @run 70:AEFC
; WHAT IT DOES: In state 0 loads the record; later raster effect $1D below level X $3A0, none above (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RasterByLevelX117A:
    lda a:OBJ_STATE,x
    bne RasterByLevelX117A_at0
    jmp VecLoadObjectRecord
RasterByLevelX117A_at0:
    lda a:ScrollXHiScreen
    cmp #$03
    bcc RasterByLevelX117A_at1
    cmp #$04
    bcs RasterByLevelX117A_at2
    lda a:ScrollXLo
    cmp #$A0
    bcc RasterByLevelX117A_at1
RasterByLevelX117A_at2:
    lda #$00
    sta z:zRasterEffect
    rts
RasterByLevelX117A_at1:
    lda #$1D
    sta z:zRasterEffect
    rts

; @routine 117D:563 ListSpeedProbe117D
; @run 71:C563
; WHAT IT DOES: Plays the list at $C58B, the X speed by OBJ_WORK_F, and probes with height 6 (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSpeedProbe117DSpeeds = $C587 ; X speed words by OBJ_WORK_F
ListSpeedProbe117D:
    lda #$C5
    sta z:zTemp11
    lda #$8B
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_F,x
    asl a
    tay
    lda a:ListSpeedProbe117DSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:ListSpeedProbe117DSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$06
    sta z:zProduct1
    jsr BounceAtWalls
    rts

; @routine 117F:156 WaitOnLeader117F
; @run 71:D156
; WHAT IT DOES: Keeps the place of the slot OBJ_WORK_L names until it reaches state 3 and OBJ_WORK_D counts out (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WaitOnLeader117FFrames = $D17A ; the frame by OBJ_WORK_M
WaitOnLeader117F:
    ldy a:OBJ_WORK_L,x
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_STATE,y
    cmp #$03
    bcc WaitOnLeader117F_at0
    dec a:OBJ_WORK_D,x
    bne WaitOnLeader117F_at0
    ldy a:OBJ_WORK_M,x
    lda a:WaitOnLeader117FFrames,y
    sta a:OBJ_FRAME,x
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
WaitOnLeader117F_at0:
    rts

; @routine 12DB:581 ListCountSoundSpawn12DB
; @run 70:B581
; WHAT IT DOES: Plays the list at $B5A6 while OBJ_WORK_E counts; then sound $25, a kind $29 object, state 2 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListCountSoundSpawn12DB:
    lda #$A6
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_E,x
    bne ListCountSoundSpawn12DB_at0
    lda #$25
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom50
    bcc ListCountSoundSpawn12DB_at0
    lda #$29
    jsr VecStartObjectAtPlace
    lda #$02
    sta a:OBJ_STATE,x
ListCountSoundSpawn12DB_at0:
    rts

; @routine 12DD:57C ListSoundAt48_12DD
; @run 71:C57C
; WHAT IT DOES: Plays the list at $C5A1 with sound $79 at frame $48; when it ends, state 3 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSoundAt48_12DD:
    lda #$A1
    sta z:zTemp10
    lda #$C5
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListSoundAt48_12DD_at0
    bcc ListSoundAt48_12DD_at1
    lda a:OBJ_FRAME,x
    cmp #$48
    bne ListSoundAt48_12DD_at1
    lda #$79
    jsr VecSoundCommand
    rts
ListSoundAt48_12DD_at0:
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListSoundAt48_12DD_at1:
    rts

; @routine 12DE:5A6 ListThenShoot13_12DE
; @run 71:CDA6
; WHAT IT DOES: Plays the list at $CDCB; then sound $21, a kind $3A object in slot $13, state 3 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenShoot13_12DE:
    lda #$CB
    sta z:zTemp10
    lda #$CD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenShoot13_12DE_at0
    lda #$21
    jsr VecSoundCommand
    ldy #$13
    jsr VecClearObjectInY
    lda #$3A
    jsr VecStartObjectAtPlace
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListThenShoot13_12DE_at0:
    rts

; @routine 12DE:71E StartEightAtBE_12DE
; @run 71:CF1E
; WHAT IT DOES: Sound $19 and eight kind $2B objects numbered at Y $BE (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: as the last call, compare or load left them.
StartEightAtBE_12DE:
    lda #$19
    jsr VecSoundCommand
    stz z:zTemp00
StartEightAtBE_12DE_at1:
    jsr VecFindFreeSlotFrom30
    bcc StartEightAtBE_12DE_at0
    lda #$2B
    jsr VecStartObjectAtPlace
    lda #$BE
    sta a:OBJ_Y_LO,y
    lda z:zTemp00
    sta a:OBJ_WORK_D,y
    inc z:zTemp00
    lda z:zTemp00
    cmp #$08
    bcc StartEightAtBE_12DE_at1
StartEightAtBE_12DE_at0:
    rts

; @routine 13FB:6CC FallProbeToA8_13FB
; @run 70:B6CC
; WHAT IT DOES: Moves and falls with a terrain probe; at Y $A8 it stops there with C set (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallProbeToA8_13FB:
    jsr VecApplyVelocity
    jsr VecApplyGravity
    lda #$04
    sta z:zProduct1
    lda #$22
    jsr VecProbeTerrain
    lda a:OBJ_Y_LO,x
    cmp #$A8
    bcc FallProbeToA8_13FB_at0
    lda #$A8
    sta a:OBJ_Y_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    sec
    rts
FallProbeToA8_13FB_at0:
    rts

; @routine 13FD:420 SwingFallToBC_13FD
; @run 71:C420
; WHAT IT DOES: Swings and falls; from Y $BC zRasterStage 0 and the object cleared (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwingFallToBC_13FD:
    jsr SwingStep13FD
    lda a:OBJ_VY_LO,x
    clc
    adc #$0C
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp #$BC
    bcc SwingFallToBC_13FD_at0
    stz z:zRasterStage
    jsr VecClearObject
SwingFallToBC_13FD_at0:
    rts

; @routine 161D:66F ListSpeedProbe161D
; @run 71:C66F
; WHAT IT DOES: Plays the list at $C695, the X speed by OBJ_WORK_E, and probes with height 12 (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSpeedProbe161DSpeeds = $C6A2 ; X speed words by OBJ_WORK_E
ListSpeedProbe161D:
    lda #$95
    sta z:zTemp10
    lda #$C6
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:ListSpeedProbe161DSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:ListSpeedProbe161DSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$0C
    sta z:zProduct1
    jsr BounceAtWalls
    rts

; @routine 161E:773 LoadDriftOrFall161E
; @run 71:CF73
; WHAT IT DOES: In state 0 loads and tosses; later falls, moves and runs on at $D03A (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadDriftOrFall161E:
    lda a:OBJ_STATE,x
    bne LoadDriftOrFall161E_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$36
    sta a:OBJ_FRAME,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    rts
LoadDriftOrFall161E_at0:
    jsr VecApplyGravity
    jsr VecApplyVelocity
    jmp StepList161F_03A

; @routine 16CB:64A CountScriptSpawn16CB
; @run 70:B64A
; WHAT IT DOES: Counts OBJ_WORK_D; then script $96 and a kind $2B object; with no free slot the count stays out (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountScriptSpawn16CB:
    dec a:OBJ_WORK_D,x
    bne CountScriptSpawn16CB_at0
    phx
    lda #$96
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    jsr VecFindFreeSlotFrom30
    bcc CountScriptSpawn16CB_at1
    lda #$2B
    jsr VecStartObjectAtPlace
    lda #$C0
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
CountScriptSpawn16CB_at0:
    rts
CountScriptSpawn16CB_at1:
    stz a:OBJ_WORK_D,x
    rts

; @routine 16CC:385 LoadTossFromSlot17_16CC
; @run 70:BB85
; WHAT IT DOES: Loads the record at slot $11's place with a random X speed and a falling Y speed (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadTossFromSlot17_16CC:
    jsr VecLoadObjectRecord
    ldy #$11
    jsr VecCopyObjectPlaceBack
    jsr VecNextRandom
    and #$03
    clc
    adc #$FD
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    sta a:OBJ_VX_LO,x
    lda #$02
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    rts

; @routine 18DB:22A ListThenSpawn26_18DB
; @run 70:B22A
; WHAT IT DOES: Plays the list at $B24F; then a kind $26 object in slot $18 at Y $5E with sound $91, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenSpawn26_18DB:
    lda #$4F
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenSpawn26_18DB_at0
    ldy #$18
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    lda #$91
    jsr VecSoundCommand
    lda #$5E
    sta a:OBJ_Y_LO,y
    inc a:OBJ_SUBSTATE,x
ListThenSpawn26_18DB_at0:
    rts

; @routine 18DB:43D StartByAnimationStep18DB
; @run 70:B43D
; WHAT IT DOES: At animation steps 7 and 8, a kind $29 object in slot $30 or $31 with the facing and sound $4D (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartByAnimationStep18DB:
    lda a:OBJ_ANIM_STEP,x
    cmp #$07
    beq StartByAnimationStep18DB_at0
    cmp #$08
    bne StartByAnimationStep18DB_at1
StartByAnimationStep18DB_at0:
    sec
    sbc #$07
    clc
    adc #$30
    tay
    jsr VecClearObjectInY
    lda #$29
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda #$4D
    jsr VecSoundCommand
StartByAnimationStep18DB_at1:
    rts

; @routine 18DC:1DE ListThenTwoScripts18DC
; @run 70:B9DE
; WHAT IT DOES: Plays the list at $BA04; then scripts $85 and $87, OBJ_WORK_E 64, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenTwoScripts18DC:
    lda #$04
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenTwoScripts18DC_at0
    phx
    lda #$85
    ldx #$04
    jsr VecStartScriptSlotKind1
    lda #$87
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
ListThenTwoScripts18DC_at0:
    rts
