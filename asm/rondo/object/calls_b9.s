; @routine 09EB:280 CountThenErupt09EB
; @run 70:B280
; WHAT IT DOES: Counts OBJ_WORK_D; then a kind $2E object in slot $11, sound $A2, rising unseen with the raster effect (secret stage 05's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenErupt09EB:
    dec a:OBJ_WORK_D,x
    bne CountThenErupt09EB_at0
    ldy #$11
    jsr VecClearObjectInY
    lda #$2E
    jsr VecStartObjectAtPlace
    lda #$A2
    jsr VecSoundCommand
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    stz a:OBJ_FRAME,x
    lda #$0B
    sta z:zRasterEffect
    lda #$C0
    sta z:zRasterArg
    inc a:OBJ_STATE,x
CountThenErupt09EB_at0:
    rts

; @routine 0CAC:1E1 FallThenBecome2E_0CAC
; @run 70:B9E1
; WHAT IT DOES: Falls playing the list at $BA0D; five screens down becomes kind $2E and clears slot $11 (secret stage 07's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallThenBecome2E_0CAC:
    lda #$0D
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr SpawnWithSound0CAC
    jsr VecApplyGravity
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    cmp #$05
    bcc FallThenBecome2E_0CAC_at0
    jsr VecClearObject
    lda #$2E
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    ldy #$11
    jsr VecClearObjectInY
FallThenBecome2E_0CAC_at0:
    rts

; @routine 0CAD:03A CircleFromCentre0CAD
; @run 71:C03A
; WHAT IT DOES: Spirals in from the screen's centre: a place at angle OBJ_WORK_E, radius OBJ_WORK_G, until the radius is gone.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CircleFromCentre0CAD:
    stz a:OBJ_X_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$70
    sta a:OBJ_Y_LO,x
    stz z:zTemp00
    lda a:OBJ_WORK_G,x
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    inc a:OBJ_WORK_E,x
    dec a:OBJ_WORK_G,x
    bne CircleFromCentre0CAD_at0
    jmp VecClearObject
CircleFromCentre0CAD_at0:
    rts

; @routine 0EBB:490 DebrisBelow0EBB
; @run 70:B490
; WHAT IT DOES: Sound $23 and the debris at $A90B eight dots below, then stops at Y $9A (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DebrisBelow0EBB:
    lda #$9A
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_LO,x
    pha
    clc
    adc #$08
    sta a:OBJ_Y_LO,x
    lda #$23
    jsr VecSoundCommand
    cla
    jsr StartDebrisBurst
    pla
    sta a:OBJ_Y_LO,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_WORK_K,x
    rts

; @routine 0EBD:502 WalkListThenState0EBD
; @run 71:C502
; WHAT IT DOES: Walks playing the list at $C52D; OBJ_WORK_G 1 means state 3, bit 7 a random state step (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkListThenState0EBD:
    jsr VecApplyVelocityX
    lda #$2D
    sta z:zTemp10
    lda #$C5
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_G,x
    cmp #$01
    beq WalkListThenState0EBD_at0
    lda a:OBJ_WORK_G,x
    bpl WalkListThenState0EBD_at1
    jsr VecNextRandom
    and #$7F
    bne WalkListThenState0EBD_at1
    inc a:OBJ_STATE,x
    rts
WalkListThenState0EBD_at0:
    lda #$03
    sta a:OBJ_STATE,x
WalkListThenState0EBD_at1:
    rts

; @routine 0EBF:25D SpeedRowByWorkI0EBF
; @run 71:D25D
; WHAT IT DOES: The speeds of row OBJ_WORK_I, OBJ_WORK_E counted, X reversed toward the player (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpeedRowByWorkI0EBFSpeeds = $D289 ; X then Y speed words, four bytes a row
SpeedRowByWorkI0EBF:
    lda a:OBJ_WORK_I,x
    asl a
    asl a
    tay
    lda a:SpeedRowByWorkI0EBFSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:SpeedRowByWorkI0EBFSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:SpeedRowByWorkI0EBFSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:SpeedRowByWorkI0EBFSpeeds+2,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_WORK_E,x
    jsr VecSideOfPlayer
    and #$01
    beq SpeedRowByWorkI0EBF_at0
    jmp VecNegateVelocityX
SpeedRowByWorkI0EBF_at0:
    rts

; @routine 0F6C:797 SpeedByDistance0F6C
; @run 70:BF97
; WHAT IT DOES: An X speed by how far the player is (four bands), reversed when facing left (secret stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpeedByDistance0F6CSpeeds = $BFC2 ; X speed words, near to far
SpeedByDistance0F6C:
    jsr VecDistanceXToPlayer
    cly
    cmp #$48
    bcc SpeedByDistance0F6C_at0
    iny
    iny
    cmp #$58
    bcc SpeedByDistance0F6C_at0
    iny
    iny
    cmp #$68
    bcc SpeedByDistance0F6C_at0
    iny
    iny
SpeedByDistance0F6C_at0:
    lda a:SpeedByDistance0F6CSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:SpeedByDistance0F6CSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:OBJ_FACING_LEFT,x
    bne SpeedByDistance0F6C_at1
    rts
SpeedByDistance0F6C_at1:
    jmp VecNegateVelocityX

; @routine 101B:17B ScatterEightThenClearPair101B
; @run 70:B17B
; WHAT IT DOES: Starts eight numbered kind $25 objects, then clears this slot and the next (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ScatterEightThenClearPair101B:
    stz z:zTemp00
ScatterEightThenClearPair101B_at1:
    jsr VecFindFreeSlotFrom30
    bcc ScatterEightThenClearPair101B_at0
    lda #$25
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda #$17
    sta a:OBJ_WORK_E,y
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    inc z:zTemp00
    lda z:zTemp00
    cmp #$08
    bcc ScatterEightThenClearPair101B_at1
ScatterEightThenClearPair101B_at0:
    jsr VecClearObject
    inx
    jsr VecClearObject
    dex
    rts

; @routine 101C:620 ArenaLimits101C
; @run 70:BE20
; WHAT IT DOES: Holds the scroll at screen 6 until slots $18, $1A and $1C are empty, then frees it to screen 8 (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ArenaLimits101C:
    lda a:OBJ_STATE,x
    bne ArenaLimits101C_at0
    lda a:ScrollXHiScreen
    cmp #$06
    bne ArenaLimits101C_at1
    lda #$06
    sta a:ScrollXMinHi
    inc a:OBJ_STATE,x
ArenaLimits101C_at1:
    rts
ArenaLimits101C_at0:
    lda a:OBJ_KIND+24
    ora a:OBJ_KIND+26
    ora a:OBJ_KIND+28
    bne ArenaLimits101C_at1
    lda #$08
    sta a:ScrollXMaxHi
    jsr VecRecordObjectEvent
    jmp VecClearObject

; @routine 101C:6C0 FlyOffWhenSourceGone101C
; @run 70:BEC0
; WHAT IT DOES: Once the slot the start argument names is empty, flies off up and back with frame $81 (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FlyOffWhenSourceGone101C:
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_KIND,y
    bne FlyOffWhenSourceGone101C_at0
    lda #$00
    sta a:OBJ_VY_LO,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    lda a:OBJ_FACING_LEFT,x
    beq FlyOffWhenSourceGone101C_at1
    jsr VecNegateVelocityX
FlyOffWhenSourceGone101C_at1:
    lda #$81
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
FlyOffWhenSourceGone101C_at0:
    rts

; @routine 101D:2DF ShootAtFrameAB_101D
; @run 71:C2DF
; WHAT IT DOES: Plays the list at $C30A; at frame $AB sound $27 and a kind $BA object in the next slot (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ShootAtFrameAB_101D:
    lda #$0A
    sta z:zTemp10
    lda #$C3
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc ShootAtFrameAB_101D_at0
    lda a:OBJ_FRAME,x
    cmp #$AB
    bne ShootAtFrameAB_101D_at0
    lda #$27
    jsr VecSoundCommand
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$BA
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
ShootAtFrameAB_101D_at0:
    rts

; @routine 101E:0F0 LandThenWake101E
; @run 71:C8F0
; WHAT IT DOES: Falls playing the list at $C91B; once hit or the player within $70 dots, hit-tested and the state on (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandThenWake101E:
    lda #$1B
    sta z:zTemp10
    lda #$C9
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$08
    jsr VecStopOnLanding
    lda a:OBJ_HIT_TAKEN,x
    bne LandThenWake101E_at0
    jsr VecDistanceXToPlayer
    cmp #$70
    bcs LandThenWake101E_at1
LandThenWake101E_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
LandThenWake101E_at1:
    rts

; @routine 10CD:09A SpiralThenSound10CD
; @run 71:C09A
; WHAT IT DOES: Moves at angle OBJ_WORK_D while OBJ_WORK_G counts; then sound $1D on screen, else cleared (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpiralThenSound10CD:
    lda a:OBJ_WORK_F,x
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    dec a:OBJ_WORK_G,x
    bne SpiralThenSound10CD_at0
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    beq SpiralThenSound10CD_at1
    jmp VecClearObject
SpiralThenSound10CD_at1:
    lda #$1D
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SpiralThenSound10CD_at0:
    rts

; @routine 117B:1D4 RiseToLine117B
; @run 70:B1D4
; WHAT IT DOES: While below the line OBJ_WORK_E, rises with growing speed; at the line it stops there (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RiseToLine117B:
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_WORK_E,x
    bcc RiseToLine117B_at0
    beq RiseToLine117B_at0
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    sec
    sbc #$30
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    rts
RiseToLine117B_at0:
    lda a:OBJ_WORK_E,x
    sta a:OBJ_Y_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    rts

; @routine 117D:102 LeapByDistance117D
; @run 71:C102
; WHAT IT DOES: A leap whose X speed is by the player's distance, reversed when OBJ_WORK_E (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LeapByDistance117DSpeeds = $C12E ; X speed words by distance over 32
LeapByDistance117D:
    inc a:OBJ_SUBSTATE,x
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:LeapByDistance117DSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LeapByDistance117DSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda a:OBJ_WORK_E,x
    beq LeapByDistance117D_at0
    jsr VecNegateVelocityX
LeapByDistance117D_at0:
    rts

; @routine 117F:23E LoadOrAimFromPrevious117F
; @run 71:D23E
; WHAT IT DOES: In state 0 loads the record; later keeps the previous slot's place and attribute and shows the frame aiming at the player.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadOrAimFromPrevious117F:
    lda a:OBJ_STATE,x
    bne LoadOrAimFromPrevious117F_at0
    jmp VecLoadObjectRecord
LoadOrAimFromPrevious117F_at0:
    lda a:OBJ_SPRITE_ATTR-1,x
    sta a:OBJ_SPRITE_ATTR,x
    txa
    tay
    dey
    sxy
    jsr VecCopyObjectPlace
    sxy
    jsr VecAngleToPlayer
    clc
    adc #$10
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc #$75
    sta a:OBJ_FRAME,x
    rts

; @routine 12DB:13C WaitForScreen3_12DB
; @run 70:B13C
; WHAT IT DOES: Holds the player's pad bytes while playing the list at $B168 until screen 3; then records the event, state on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WaitForScreen3_12DB:
    lda #$03
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    lda #$68
    sta z:zTemp10
    lda #$B1
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:ScrollXHiScreen
    cmp #$03
    bcc WaitForScreen3_12DB_at0
    jsr VecRecordObjectEvent
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
WaitForScreen3_12DB_at0:
    rts

; @routine 12DC:331 ParryWhenHit12DC
; @run 70:BB31
; WHAT IT DOES: In states 2 and 3 a hit of 5 or more: sound $78, turn to the player, state 5 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ParryWhenHit12DC:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc ParryWhenHit12DC_at0
    lda a:OBJ_STATE,x
    cmp #$02
    beq ParryWhenHit12DC_at1
    cmp #$03
    bne ParryWhenHit12DC_at0
ParryWhenHit12DC_at1:
    lda #$78
    jsr VecSoundCommand
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
ParryWhenHit12DC_at0:
    rts

; @routine 12DC:603 SpawnThreeAtStep12DC
; @run 70:BE03
; WHAT IT DOES: At animation step 10: sound $25 and three numbered kind $30 objects with the facing (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnThreeAtStep12DC:
    lda a:OBJ_ANIM_STEP,x
    cmp #$0A
    bne SpawnThreeAtStep12DC_at0
    lda #$25
    jsr VecSoundCommand
    stz z:zTemp00
SpawnThreeAtStep12DC_at1:
    jsr VecFindFreeSlotFrom30
    bcc SpawnThreeAtStep12DC_at0
    lda #$30
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    inc z:zTemp00
    lda z:zTemp00
    cmp #$03
    bcc SpawnThreeAtStep12DC_at1
SpawnThreeAtStep12DC_at0:
    rts

; @routine 12DD:0B3 FollowSlot16Piece12DD_0B3
; @run 71:C0B3
; WHAT IT DOES: Follows the previous slot at an offset from a table by OBJ_WORK_D and slot $10's facing, with its attribute and hit-test.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FollowSlot16Piece12DD_0B3Offsets = $C0DF ; X offset, then Y offset two bytes on, by piece and facing
FollowSlot16Piece12DD_0B3:
    lda a:OBJ_SPRITE_ATTR+16
    sta a:OBJ_SPRITE_ATTR,x
    lda a:OBJ_HITBOX_ON+16
    sta a:OBJ_HITBOX_ON,x
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    clc
    adc a:OBJ_FACING_LEFT+16
    tay
    lda a:FollowSlot16Piece12DD_0B3Offsets,y
    sta a:OBJ_VX_HI,x
    lda a:FollowSlot16Piece12DD_0B3Offsets+2,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    rts

; @routine 12DD:14D FollowSlot16Piece12DD_14D
; @run 71:C14D
; WHAT IT DOES: Follows the previous slot at an offset from a table by OBJ_WORK_D and slot $10's facing, with its attribute and hit-test.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FollowSlot16Piece12DD_14DOffsets = $C179 ; X offset, then Y offset two bytes on, by piece and facing
FollowSlot16Piece12DD_14D:
    lda a:OBJ_SPRITE_ATTR+16
    sta a:OBJ_SPRITE_ATTR,x
    lda a:OBJ_HITBOX_ON+16
    sta a:OBJ_HITBOX_ON,x
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    clc
    adc a:OBJ_FACING_LEFT+16
    tay
    lda a:FollowSlot16Piece12DD_14DOffsets,y
    sta a:OBJ_VX_HI,x
    lda a:FollowSlot16Piece12DD_14DOffsets+2,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    rts

; @routine 13FC:422 ListThenHitTest13FC
; @run 70:BC22
; WHAT IT DOES: Plays the list at $BC4D; then hit-tested with fresh work bytes, substate on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenHitTest13FC:
    lda #$4D
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenHitTest13FC_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$12
    sta a:OBJ_WORK_C,x
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_F,x
    lda #$01
    sta a:OBJ_WORK_H,x
    lda #$00
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
ListThenHitTest13FC_at0:
    rts

; @routine 161F:269 RiseWithRoute161F
; @run 71:D269
; WHAT IT DOES: Loads; once zField8C is set rises two dots a frame until Y low passes $80 and is cleared (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RiseWithRoute161F:
    lda a:OBJ_STATE,x
    bne RiseWithRoute161F_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$44
    sta a:OBJ_FRAME,x
RiseWithRoute161F_at2:
    rts
RiseWithRoute161F_at0:
    dec a
    bne RiseWithRoute161F_at1
    lda z:zField8C
    beq RiseWithRoute161F_at2
    inc a:OBJ_STATE,x
    rts
RiseWithRoute161F_at1:
    dec a:OBJ_Y_LO,x
    dec a:OBJ_Y_LO,x
    bpl RiseWithRoute161F_at2
    jmp VecClearObject

; @routine 16CC:204 ListSpawnWhileCount16CC
; @run 70:BA04
; WHAT IT DOES: Plays the list at $B9EA starting a kind $26 object every eighth count of OBJ_WORK_D; then stops, substate on (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSpawnWhileCount16CC:
    lda #$EA
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_D,x
    and #$07
    bne ListSpawnWhileCount16CC_at0
    jsr VecFindFreeSlotFrom30
    bcc ListSpawnWhileCount16CC_at0
    lda #$26
    jsr VecStartObjectAtPlace
ListSpawnWhileCount16CC_at0:
    dec a:OBJ_WORK_D,x
    bne ListSpawnWhileCount16CC_at1
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
ListSpawnWhileCount16CC_at1:
    rts

; @routine 18DB:510 DiveThenSound18DB
; @run 70:B510
; WHAT IT DOES: Rises slowing under gravity $18; at the top sound $93, slot $40 to state 2, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DiveThenSound18DB:
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    bmi DiveThenSound18DB_at0
    lda #$93
    jsr VecSoundCommand
    lda #$02
    sta a:OBJ_STATE+64
    lda #$80
    sta a:OBJ_WORK_E,x
    stz a:OBJ_WORK_G,x
    inc a:OBJ_SUBSTATE,x
DiveThenSound18DB_at0:
    rts
