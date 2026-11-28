; @routine 088C:4DB AndVramWithMask088C
; @run 70:BCDB
; WHAT IT DOES: ANDs the VRAM word at zBiosCl-Ch with StageMaskBuffer's byte X on both halves (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: A the new high byte, N and Z from it; X and Y kept.
AndVramWithMask088C:
    jsr VecSelectVramWriteAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    and a:StageMaskBuffer,x
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and a:StageMaskBuffer,x
    sta a:VDC_DATA_HI
    rts

; @routine 0B4C:243 MoveStopAtWall0B4C
; @run 70:BA43
; WHAT IT DOES: Moves along X; at a wall OBJ_WORK_E 96, the animation restarted, substate on (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
MoveStopAtWall0B4C:
    jsr VecApplyVelocityX
    lda #$06
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
    beq MoveStopAtWall0B4C_at0
    lda #$60
    sta a:OBJ_WORK_E,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
MoveStopAtWall0B4C_at0:
    rts

; @routine 0CAC:109 StartAtCountWithSound0CAC
; @run 70:B909
; WHAT IT DOES: At the table's counts of OBJ_WORK_E: a sound and a kind $2C object in slot $18 plus its place (secret stage 07's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartAtCountWithSound0CACCounts = $B939 ; eight counts
StartAtCountWithSound0CACSounds = $B941 ; four sounds in turn
StartAtCountWithSound0CAC:
    cly
StartAtCountWithSound0CAC_at1:
    lda a:OBJ_WORK_E,x
    cmp a:StartAtCountWithSound0CACCounts,y
    beq StartAtCountWithSound0CAC_at0
    iny
    cpy #$08
    bcc StartAtCountWithSound0CAC_at1
    rts
StartAtCountWithSound0CAC_at0:
    phy
    tya
    and #$03
    tay
    lda a:StartAtCountWithSound0CACSounds,y
    jsr VecSoundCommand
    ply
    sty z:zSpreadCount
    tya
    clc
    adc #$18
    tay
    jsr VecClearObjectInY
    lda #$2C
    jsr VecStartObjectAtPlace
    lda z:zSpreadCount
    sta a:OBJ_WORK_D,y
    rts

; @routine 0EBF:610 LoadRiser0EBF
; @run 71:D610
; WHAT IT DOES: Loads at a random place rising slowly, facing the player, with random work bytes (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadRiser0EBFWaits = $D640 ; OBJ_WORK_G, four at random
LoadRiser0EBF:
    jsr VecLoadObjectRecord
    lda #$8F
    sta a:OBJ_SPRITE_ATTR,x
    jsr PlaceAtRandom0EBF
    jsr VecNextRandom
    sta a:OBJ_WORK_J,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:LoadRiser0EBFWaits,y
    sta a:OBJ_WORK_G,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    rts

; @routine 0F6C:1A8 ThrowOnTurn0F6C
; @run 70:B9A8
; WHAT IT DOES: On this object's turn of four, a kind $9B object eight dots down and right (secret stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ThrowOnTurn0F6C:
    lda z:zWaveCounter
    and #$03
    sta z:zTemp00
    lda a:OBJ_START_ARG,x
    and #$03
    cmp z:zTemp00
    bne ThrowOnTurn0F6C_at0
    jsr VecFindFreeSlotFrom20
    bcc ThrowOnTurn0F6C_at0
    lda #$9B
    jsr VecStartObjectAtPlace
    sxy
    lda #$08
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda #$08
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    sxy
ThrowOnTurn0F6C_at0:
    rts

; @routine 101B:0CC WalkAndTurnEvery101B
; @run 70:B0CC
; WHAT IT DOES: Walks a step on each new frame, turning after as many steps as the start argument (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkAndTurnEvery101BSteps = $B0FC ; X speed high byte by direction
WalkAndTurnEvery101B:
    lda #$FE
    sta z:zTemp10
    lda #$B0
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc WalkAndTurnEvery101B_at0
    ldy a:OBJ_WORK_E,x
    lda a:WalkAndTurnEvery101BSteps,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    dec a:OBJ_WORK_L,x
    bne WalkAndTurnEvery101B_at0
    lda a:OBJ_START_ARG,x
    sta a:OBJ_WORK_L,x
    lda a:OBJ_WORK_E,x
    eor #$01
    sta a:OBJ_WORK_E,x
WalkAndTurnEvery101B_at0:
    jsr WaitThenState3_101B
    rts

; @routine 101D:3B4 FireWhenFacingPlayer101D
; @run 71:C3B4
; WHAT IT DOES: Cleared unless facing the player; else loads, jumps by facing and lasts two counts (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FireWhenFacingPlayer101DSpeeds = $C3E0 ; X speed high byte by facing
FireWhenFacingPlayer101D:
    lda a:OBJ_STATE,x
    bne FireWhenFacingPlayer101D_at0
    jsr VecSideOfPlayer
    and #$01
    cmp a:OBJ_FACING_LEFT,x
    beq FireWhenFacingPlayer101D_at1
    jmp VecClearObject
FireWhenFacingPlayer101D_at1:
    jsr VecLoadObjectRecord
    ldy a:OBJ_FACING_LEFT,x
    lda a:FireWhenFacingPlayer101DSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$F6
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda #$02
    sta a:OBJ_WORK_D,x
    rts
    sta (zScriptPtrHi,x)
FireWhenFacingPlayer101D_at0:
    dec a:OBJ_WORK_D,x
    bne FireWhenFacingPlayer101D_at2
    jmp VecClearObject
FireWhenFacingPlayer101D_at2:
    rts

; @routine 101E:42B FollowPreviousWithSlot16_101E
; @run 71:CC2B
; WHAT IT DOES: Keeps the previous slot's place; piece 0 offsets it by slot $10's frame and facing (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FollowPreviousWithSlot16_101EOffsets = $CC5C ; X and Y offset by slot $10's frame less one
FollowPreviousWithSlot16_101E:
    lda a:OBJ_HITBOX_ON+16
    sta a:OBJ_HITBOX_ON,x
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_WORK_D,x
    bne FollowPreviousWithSlot16_101E_at0
    lda a:OBJ_FRAME+16
    sec
    sbc #$01
    asl a
    tay
    lda a:FollowPreviousWithSlot16_101EOffsets,y
    sta a:OBJ_VX_HI,x
    lda a:FollowPreviousWithSlot16_101EOffsets+1,y
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT+16
    beq FollowPreviousWithSlot16_101E_at1
    jsr VecNegateVelocityX
FollowPreviousWithSlot16_101E_at1:
    jsr VecApplyVelocity
FollowPreviousWithSlot16_101E_at0:
    rts

; @routine 10CB:17B PieceChannelByFrame10CB
; @run 70:B17B
; WHAT IT DOES: Counts OBJ_WORK_D, runs $B256 and $B26D; a kind $A5 piece shows the list channel its frame names (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PieceChannelByFrame10CBChannels = $B1A9 ; the list channel less $81 by frame less one, 0 none
PieceChannelByFrame10CBFirst = $B256 ; a routine of this overlay
PieceChannelByFrame10CB:
    inc a:OBJ_WORK_D,x
    jsr PieceChannelByFrame10CBFirst
    jsr JumpByStateTable10CB26D
    phx
    lda a:OBJ_KIND,x
    cmp #$A5
    bne PieceChannelByFrame10CB_at0
    lda a:OBJ_WORK_L,x
    bne PieceChannelByFrame10CB_at0
    lda a:OBJ_FRAME,x
    beq PieceChannelByFrame10CB_at0
    sec
    sbc #$01
    tay
    lda a:PieceChannelByFrame10CBChannels,y
    beq PieceChannelByFrame10CB_at0
    clc
    adc #$81
    ldx #$05
    jsr VecStartListChannelKind2
PieceChannelByFrame10CB_at0:
    plx
    rts

; @routine 10CC:3CA SoundScriptAndPose10CC
; @run 70:BBCA
; WHAT IT DOES: Sound $E8, script $B0, then frame $98 shot upward, substate on (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundScriptAndPose10CC:
    lda #$E8
    jsr VecSoundCommand
    phx
    lda #$B0
    ldx #$08
    jsr VecStartScriptSlotKind1
    plx
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$F8
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$20
    sta a:OBJ_WORK_E,x
    lda #$98
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 117E:08E ListFlyThenFall117E
; @run 71:C88E
; WHAT IT DOES: Flies playing the list at $C8BE; type 2 falls, others count OBJ_WORK_F to the next state (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListFlyThenFall117E:
    lda #$C8
    sta z:zTemp11
    lda #$BE
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_WORK_E,x
    cmp #$02
    bne ListFlyThenFall117E_at0
    lda a:OBJ_VY_LO,x
    clc
    adc #$20
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    rts
ListFlyThenFall117E_at0:
    dec a:OBJ_WORK_F,x
    bne ListFlyThenFall117E_at1
    inc a:OBJ_STATE,x
ListFlyThenFall117E_at1:
    rts

; @routine 117F:72F RaiseSlot67_117F
; @run 71:D72F
; WHAT IT DOES: Once the slot three on is empty, raises slot $43 two dots a call; at Y $9F cleared (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RaiseSlot67_117F:
    lda a:OBJ_KIND+3,x
    bne RaiseSlot67_117F_at0
    lda #$80
    sta a:OBJ_SPRITE_FLIP+67
    stz a:OBJ_HIT_TAKEN+67
    lda a:OBJ_Y_LO+67
    sec
    sbc #$02
    sta a:OBJ_Y_LO+67
    lda a:OBJ_Y_HI+67
    sbc #$00
    sta a:OBJ_Y_HI+67
    bne RaiseSlot67_117F_at0
    lda a:OBJ_Y_LO+67
    cmp #$9F
    bcs RaiseSlot67_117F_at0
    lda #$9F
    sta a:OBJ_Y_LO+67
    jmp VecClearObject
RaiseSlot67_117F_at0:
    rts

; @routine 12DB:6A1 DripWhenLow12DB
; @run 70:B6A1
; WHAT IT DOES: Low on the screen, every fourth count: sound $19 and a kind $2B drop at Y $BE (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DripWhenLow12DB:
    lda a:OBJ_Y_HI,x
    bne DripWhenLow12DB_at0
    lda a:OBJ_Y_LO,x
    cmp #$A4
    bcc DripWhenLow12DB_at0
    lda a:OBJ_WORK_F,x
    and #$03
    bne DripWhenLow12DB_at0
    jsr VecFindFreeSlotFrom30
    bcc DripWhenLow12DB_at0
    lda #$19
    jsr VecSoundCommand
    lda #$2B
    jsr VecStartObjectAtPlace
    lda #$BE
    sta a:OBJ_Y_LO,y
    jsr VecNextRandom
    and #$0F
    sta a:OBJ_WORK_D,y
DripWhenLow12DB_at0:
    rts

; @routine 12DD:415 WaitForSlot18ThenList12DD
; @run 71:C415
; WHAT IT DOES: Waits for slot $12 to drop past Y $50 (sound $7B), then the list at $C445, hit-tested, state on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WaitForSlot18ThenList12DD:
    lda a:OBJ_SUBSTATE,x
    bne WaitForSlot18ThenList12DD_at0
    lda a:OBJ_Y_HI+18
    bne WaitForSlot18ThenList12DD_at1
    lda a:OBJ_Y_LO+18
    cmp #$50
    bcc WaitForSlot18ThenList12DD_at1
    lda #$7B
    jsr VecSoundCommand
    inc a:OBJ_SUBSTATE,x
WaitForSlot18ThenList12DD_at1:
    rts
WaitForSlot18ThenList12DD_at0:
    lda #$45
    sta z:zTemp10
    lda #$C4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne WaitForSlot18ThenList12DD_at2
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
WaitForSlot18ThenList12DD_at2:
    rts

; @routine 12DE:173 HomeOnAngle12DE
; @run 71:C973
; WHAT IT DOES: Moves at angle OBJ_WORK_D turning toward OBJ_WORK_H; there, the state on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
HomeOnAngle12DE:
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_H,x
    sta z:zTemp02
    lda #$02
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    cmp a:OBJ_WORK_H,x
    bne HomeOnAngle12DE_at0
    inc a:OBJ_STATE,x
HomeOnAngle12DE_at0:
    rts

; @routine 161D:43A LoadAimThenList161D
; @run 71:C43A
; WHAT IT DOES: In state 0 appears 32 dots lower aimed at the player; later flies playing the list at $C46A (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAimThenList161D:
    lda a:OBJ_STATE,x
    bne LoadAimThenList161D_at0
    jsr VecLoadObjectRecord
    lda #$20
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    jsr VecAngleToPlayer
    pha
    lda #$80
    sta z:zTemp00
    lda #$03
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    rts
LoadAimThenList161D_at0:
    lda #$6A
    sta z:zTemp10
    lda #$C4
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    rts

; @routine 16CC:46B ListSpiralOut16CC
; @run 70:BC6B
; WHAT IT DOES: Moves at angle OBJ_WORK_F with a growing speed (OBJ_WORK_D-E), playing the list at $BC9A (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSpiralOut16CC:
    lda #$9A
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_E,x
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    lda a:OBJ_WORK_F,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_WORK_E,x
    clc
    adc #$08
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    adc #$00
    sta a:OBJ_WORK_D,x
    jmp VecApplyVelocity

; @routine 18DB:183 HitInStates5To7_18DB
; @run 70:B183
; WHAT IT DOES: In states 5 to 7, once hit: state 8 and $500 points; a carry from the score runs $A6EC (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
HitInStates5To7_18DB:
    lda a:OBJ_STATE,x
    cmp #$05
    bcc HitInStates5To7_18DB_at0
    cmp #$08
    bcs HitInStates5To7_18DB_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitInStates5To7_18DB_at0
    lda #$08
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    lda #$00
    sta z:zTemp00
    lda #$05
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc HitInStates5To7_18DB_at0
    jsr GiveExtraLife
HitInStates5To7_18DB_at0:
    rts

; @routine 18DB:6AA ChecksListThenNextState18DB
; @run 70:B6AA
; WHAT IT DOES: Runs $B6FE and $B71B and the list at $B6E9; when OBJ_WORK_E counts out, the next state of a cycle (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChecksListThenNextState18DBCycle = $B6D9 ; 16 states in turn
ChecksListThenNextState18DB:
    jsr SpawnAtRandom18DB
    jsr HealthByFirstHit18DB
    lda #$E9
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_E,x
    bne ChecksListThenNextState18DB_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_WORK_F,x
    and #$0F
    tay
    lda a:ChecksListThenNextState18DBCycle,y
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_F,x
    stz a:OBJ_SUBSTATE,x
ChecksListThenNextState18DB_at0:
    rts

; @routine 18DC:139 ListThenLaunch27_18DC
; @run 70:B939
; WHAT IT DOES: Plays the list at $B969; then sound $4C and a kind $27 object in slot $33, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenLaunch27_18DC:
    lda #$69
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenLaunch27_18DC_at0
    lda #$4C
    jsr VecSoundCommand
    ldy #$33
    jsr VecClearObjectInY
    lda #$27
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_J,x
    lda #$02
    sta a:OBJ_START_ARG,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    inc a:OBJ_SUBSTATE,x
ListThenLaunch27_18DC_at0:
    rts
