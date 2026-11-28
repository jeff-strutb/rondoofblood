; @routine 05CB:6A0 ClearPairAndStop05CB
; @run 70:B6A0
; WHAT IT DOES: Clears slots $11 and $12, stops with no hit box, OBJ_WORK_E 64, substate on (secret stage 02's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ClearPairAndStop05CB:
    ldy #$11
    jsr VecClearObjectInY
    iny
    jsr VecClearObjectInY
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 072C:4B9 FallThenBecome22_072C
; @run 70:BCB9
; WHAT IT DOES: Adds A to the Y speed and moves; two screens down the slot becomes kind $22 (secret stage 03's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallThenBecome22_072C:
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    cmp #$02
    bcc FallThenBecome22_072C_at0
    jsr VecClearObject
FallThenBecome22_072C_at1:
    lda #$22
    sta a:OBJ_KIND,x
FallThenBecome22_072C_at0:
    rts

; @routine 088A:5D8 ToState11WhenHit088A
; @run 70:ADD8
; WHAT IT DOES: In states the table allows, a hit of 5 or more: $B784, sound $54, state $11 (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ToState11WhenHit088AAllowed = $ADF8 ; by state: 0 ignores hits
ToState11WhenHit088A:
    ldy a:OBJ_STATE,x
    lda a:ToState11WhenHit088AAllowed,y
    beq ToState11WhenHit088A_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc ToState11WhenHit088A_at0
    jsr ClearStageTimers
    lda #$54
    jsr VecSoundCommand
    lda #$11
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ToState11WhenHit088A_at0:
    rts

; @routine 0B4C:2BD SpawnWithRandom0B4C
; @run 70:BABD
; WHAT IT DOES: Every fourth count of OBJ_WORK_H: sound $19 and a kind $9A object with a random OBJ_WORK_E (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnWithRandom0B4C:
    lda a:OBJ_WORK_H,x
    and #$03
    bne SpawnWithRandom0B4C_at0
    lda #$19
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom20
    bcc SpawnWithRandom0B4C_at0
    lda #$9A
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,y
    jsr VecNextRandom
    and #$07
    sta a:OBJ_WORK_E,y
SpawnWithRandom0B4C_at0:
    rts

; @routine 0E0B:6F7 Frame44AndChannel0E0B
; @run 70:B6F7
; WHAT IT DOES: Frame $44, the water line off, no hit box, list channel $83, substate on (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
Frame44AndChannel0E0B:
    lda #$44
    sta a:OBJ_FRAME,x
    stz a:LevelYHi
    stz a:RasterWaterWanted
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_HITBOX_ON,x
    phx
    lda #$83
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0EBB:728 SpawnScattered0EBB
; @run 70:B728
; WHAT IT DOES: A kind $A9 object near this place, its work and X low random (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnScattered0EBB:
    jsr VecFindFreeSlotFrom20
    bcc SpawnScattered0EBB_at0
    lda #$A9
    jsr VecStartObjectAtPlace
    jsr VecNextRandom
    and #$07
    sta a:OBJ_WORK_E,y
    lda #$10
    sta a:OBJ_SPRITE_ATTR,y
    jsr VecNextRandom
    and #$0F
    clc
    adc a:OBJ_X_LO,y
    sta a:OBJ_X_LO,y
SpawnScattered0EBB_at0:
    rts

; @routine 0EBD:577 FallAndDropEmbers0EBD
; @run 71:C577
; WHAT IT DOES: Falls, cleared a screen down; every fourth count of OBJ_WORK_E drops a kind $9D object (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallAndDropEmbers0EBD:
    jsr VecApplyGravity
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne FallAndDropEmbers0EBD_at0
    jmp VecClearObject
FallAndDropEmbers0EBD_at0:
    lda a:OBJ_WORK_E,x
    and #$03
    bne FallAndDropEmbers0EBD_at1
    jsr VecFindFreeSlotFrom30
    bcc FallAndDropEmbers0EBD_at1
    lda #$9D
    jsr VecStartObjectAtPlace
FallAndDropEmbers0EBD_at1:
    rts

; @routine 0EBE:026 ListFaceThenNear0EBE
; @run 71:C826
; WHAT IT DOES: Plays the list at $C846 facing the player; within $60 dots state 3 (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListFaceThenNear0EBE:
    lda #$C8
    sta z:zTemp11
    lda #$46
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs ListFaceThenNear0EBE_at0
    lda #$03
    sta a:OBJ_STATE,x
ListFaceThenNear0EBE_at0:
    rts

; @routine 0EBF:648 PlaceAtRandom0EBF
; @run 71:D648
; WHAT IT DOES: Puts the object at a random place on the first screen from two tables (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PlaceAtRandom0EBFPlacesX = $D66A ; eight X lows
PlaceAtRandom0EBFPlacesY = $D672 ; four Y lows
PlaceAtRandom0EBF:
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    jsr VecNextRandom
    and #$07
    tay
    lda a:PlaceAtRandom0EBFPlacesX,y
    sta a:OBJ_X_LO,x
    jsr DriftWithScroll0EBF
    jsr VecNextRandom
    and #$03
    tay
    lda a:PlaceAtRandom0EBFPlacesY,y
    sta a:OBJ_Y_LO,x
    rts

; @routine 101B:09C LoadAndStart24_101B
; @run 70:B09C
; WHAT IT DOES: Loads the record facing left, OBJ_WORK_L half the start argument, a kind $24 object in the next slot (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAndStart24_101B:
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_START_ARG,x
    lsr a
    sta a:OBJ_WORK_L,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$24
    jsr VecStartObjectAtPlace
    lda #$30
    sta a:OBJ_WORK_H,x
    rts

; @routine 101C:44D SpawnAtStep101C
; @run 70:BC4D
; WHAT IT DOES: At animation step zTemp00: sound 6 and a kind $30 object with the facing and start argument zTemp01 (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnAtStep101C:
    lda a:OBJ_ANIM_STEP,x
    cmp z:zTemp00
    bne SpawnAtStep101C_at0
    lda #$06
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom30
    bcc SpawnAtStep101C_at0
    lda #$30
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda z:zTemp01
    sta a:OBJ_START_ARG,y
SpawnAtStep101C_at0:
    rts

; @routine 101E:5F4 SpawnInTurn101E
; @run 71:CDF4
; WHAT IT DOES: On this slot's frame of eight (slot less $11): sound $18 and a kind $9D object with the flip bits (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnInTurn101E:
    lda z:zWaveCounter
    and #$07
    sta z:zTemp00
    txa
    sec
    sbc #$11
    cmp z:zTemp00
    bne SpawnInTurn101E_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnInTurn101E_at0
    lda #$18
    jsr VecSoundCommand
    lda #$9D
    jsr VecStartObjectAtPlace
    lda a:OBJ_SPRITE_FLIP,x
    sta a:OBJ_SPRITE_FLIP,y
SpawnInTurn101E_at0:
    rts

; @routine 10CC:66C LoadOrFallList10CC
; @run 70:BE6C
; WHAT IT DOES: In state 0 loads the record rising; later plays the list at $BE8C while falling (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadOrFallList10CC:
    lda a:OBJ_STATE,x
    bne LoadOrFallList10CC_at0
    jsr VecLoadObjectRecord
    lda #$F6
    sta a:OBJ_VY_HI,x
    rts
LoadOrFallList10CC_at0:
    lda #$8C
    sta z:zTemp10
    lda #$BE
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravity
    jsr VecApplyVelocityY
    rts

; @routine 117D:070 NextRandomMove117D
; @run 71:C070
; WHAT IT DOES: The next count from a cycle, the player's side in OBJ_WORK_E and substate 1 or 2 at random (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
NextRandomMove117DCounts = $C093 ; OBJ_WORK_D in turn
NextRandomMove117D:
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_F,x
    and #$03
    tay
    lda a:NextRandomMove117DCounts,y
    sta a:OBJ_WORK_D,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_WORK_E,x
    jsr VecNextRandom
    and #$01
    clc
    adc #$01
    sta a:OBJ_SUBSTATE,x
    rts

; @routine 117F:553 ListSpeedThenCount117F
; @run 71:D553
; WHAT IT DOES: Plays the list at $D575, moving on new frames, until OBJ_WORK_D counts out (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSpeedThenCount117FSpeeds = $D582 ; X speed high byte by facing
ListSpeedThenCount117F:
    lda #$75
    sta z:zTemp10
    lda #$D5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc ListSpeedThenCount117F_at0
    ldy a:OBJ_FACING_LEFT,x
    lda a:ListSpeedThenCount117FSpeeds,y
    sta a:OBJ_VX_HI,x
    jsr ProbeHeightCall117F_46F
ListSpeedThenCount117F_at0:
    dec a:OBJ_WORK_D,x
    bne ListSpeedThenCount117F_at1
    jmp NextWaitToState3_117F
ListSpeedThenCount117F_at1:
    rts

; @routine 12DA:773 RunThenFall12DA
; @run 70:AF73
; WHAT IT DOES: The routine at $AA35 for point $10, $26, then a fall of $10 along Y (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RunThenFall12DA:
    lda #$10
    sta z:zTemp08
    lda #$26
    sta z:zTemp09
    jsr ContactWithPlayer
    lda a:OBJ_VY_LO,x
    clc
    adc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    rts

; @routine 12DB:06D CountScriptAndChannel12DB
; @run 70:B06D
; WHAT IT DOES: Counts OBJ_WORK_D; at zero script $86 and list channel $A9, the count 96, state 1 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountScriptAndChannel12DB:
    dec a:OBJ_WORK_D,x
    bne CountScriptAndChannel12DB_at0
    phx
    lda #$86
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$A9
    ldx #$01
    jsr VecStartListChannelKind2
    plx
    lda #$60
    sta a:OBJ_WORK_D,x
    lda #$01
    sta a:OBJ_STATE,x
CountScriptAndChannel12DB_at0:
    rts

; @routine 12DB:2C6 LoadRiseAndScript12DB
; @run 70:B2C6
; WHAT IT DOES: Loads the record 32 dots up, stops, script $8A, OBJ_WORK_E 32 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadRiseAndScript12DB:
    jsr VecLoadObjectRecord
    lda #$E0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    phx
    lda #$8A
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    lda #$20
    sta a:OBJ_WORK_E,x
    rts

; @routine 12DB:5C4 ListThenWideBox12DB
; @run 70:B5C4
; WHAT IT DOES: Plays the list at $B5E5; then a tall hit box, no Y speed, substate on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenWideBox12DB:
    lda #$E5
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenWideBox12DB_at0
    lda #$08
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$2C
    sta a:OBJ_HITBOX_HALF_H,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
ListThenWideBox12DB_at0:
    rts

; @routine 12DD:66A ListThenShoot12DD
; @run 71:C66A
; WHAT IT DOES: Plays the list at $C68A; then a rising kind $8B object in slot $30 and this one cleared (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenShoot12DD:
    lda #$8A
    sta z:zTemp10
    lda #$C6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenShoot12DD_at0
    ldy #$30
    jsr VecClearObjectInY
    lda #$8B
    jsr VecStartObjectAtPlace
    lda #$FC
    sta a:OBJ_VY_HI,y
    jsr VecClearObject
ListThenShoot12DD_at0:
    rts

; @routine 12DE:029 MoveSoundThenWait12DE
; @run 71:C829
; WHAT IT DOES: Moves along Y while OBJ_WORK_E counts, sounding $46 at 12; then stops, state on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
MoveSoundThenWait12DE:
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_E,x
    cmp #$0C
    bne MoveSoundThenWait12DE_at0
    lda #$46
    jsr VecSoundCommand
MoveSoundThenWait12DE_at0:
    dec a:OBJ_WORK_E,x
    bne MoveSoundThenWait12DE_at1
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$20
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
MoveSoundThenWait12DE_at1:
    rts

; @routine 12DE:04C CountThenSpeed12DE
; @run 71:C84C
; WHAT IT DOES: Counts OBJ_WORK_E; then the state on and the X speed of the start argument, reversed when OBJ_WORK_D (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenSpeed12DESpeeds = $C86E ; X speed words by start argument
CountThenSpeed12DE:
    dec a:OBJ_WORK_E,x
    bne CountThenSpeed12DE_at0
    inc a:OBJ_STATE,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:CountThenSpeed12DESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:CountThenSpeed12DESpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_WORK_D,x
    beq CountThenSpeed12DE_at0
    jsr VecNegateVelocityX
CountThenSpeed12DE_at0:
    rts

; @routine 12DE:2BC LoadWithCompanions12DE
; @run 71:CABC
; WHAT IT DOES: Loads the record, a kind $3B object in slot $11, list channel $84, script $8F, OBJ_WORK_F $B0 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadWithCompanions12DE:
    jsr VecLoadObjectRecord
    ldy #$11
    jsr VecClearObjectInY
    lda #$3B
    jsr VecStartObjectAtPlace
    phx
    lda #$84
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$8F
    ldx #$04
    jsr VecStartScriptSlotKind1
    plx
    lda #$B0
    sta a:OBJ_WORK_F,x
    rts

; @routine 12DE:3F2 SoundAndNextOfCycle12DE
; @run 71:CBF2
; WHAT IT DOES: Sound $7C, the animation restarted, OBJ_WORK_G 40, the next substate of a cycle (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundAndNextOfCycle12DECycle = $CC12 ; the substates in turn
SoundAndNextOfCycle12DE:
    lda #$7C
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$28
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_H,x
    and #$07
    tay
    lda a:SoundAndNextOfCycle12DECycle,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_H,x
    rts

; @routine 12DE:4AC ListThenLeap12DE
; @run 71:CCAC
; WHAT IT DOES: Plays the list at $CCCD; then leaps straight up, substate on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenLeap12DE:
    lda #$CD
    sta z:zTemp10
    lda #$CC
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenLeap12DE_at0
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    lda #$20
    sta a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
ListThenLeap12DE_at0:
    rts

; @routine 13FB:3FF LandAtA8_13FB
; @run 70:B3FF
; WHAT IT DOES: Falls to Y $A8: sound $72, held there, raster effect off, substate on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandAtA8_13FB:
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    bne LandAtA8_13FB_at0
    lda a:OBJ_Y_LO,x
    cmp #$A8
    bcc LandAtA8_13FB_at0
    lda #$72
    jsr VecSoundCommand
    lda #$A8
    sta a:OBJ_Y_LO,x
    stz z:zRasterEffect
    stz z:zRasterArg
    inc a:OBJ_SUBSTATE,x
LandAtA8_13FB_at0:
    rts

; @routine 13FB:725 CycleAndSpawn13FB
; @run 70:B725
; WHAT IT DOES: The next of a cycle in OBJ_WORK_I; a 3 starts a kind $42 object in slot $13; substate on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CycleAndSpawn13FBCycle = $B746 ; OBJ_WORK_I in turn
CycleAndSpawn13FB:
    lda a:OBJ_WORK_H,x
    and #$07
    tay
    lda a:CycleAndSpawn13FBCycle,y
    sta a:OBJ_WORK_I,x
    cmp #$03
    bne CycleAndSpawn13FB_at0
    ldy #$13
    jsr VecClearObjectInY
    lda #$42
    jsr VecStartObjectAtPlace
CycleAndSpawn13FB_at0:
    inc a:OBJ_WORK_H,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 13FC:09F ListThenRaster13FC
; @run 70:B89F
; WHAT IT DOES: Plays the list at $B8C2; then raster effect $0B from line $BF, a slow sink, substate on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenRaster13FC:
    lda #$C2
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenRaster13FC_at0
    lda #$0B
    sta z:zRasterEffect
    lda #$BF
    sta z:zRasterArg
    lda #$C0
    sta a:OBJ_WORK_E,x
    lda #$10
    sta a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
ListThenRaster13FC_at0:
    rts

; @routine 13FC:0C7 DissolveSinkThenBecome43_13FC
; @run 70:B8C7
; WHAT IT DOES: Dissolves patterns 0 to 5 while sinking and OBJ_WORK_E counts; then the slot becomes kind $43 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DissolveSinkThenBecome43_13FC:
    lda #$01
    sta a:OBJ_WORK_M,x
    lda #$00
    sta a:WorkByteA
    lda #$06
    sta a:WorkByteB
    jsr DissolveStep13FC0EA
    jsr VecApplyVelocityY
    dec a:OBJ_WORK_E,x
    bne DissolveSinkThenBecome43_13FC_at0
    lda #$43
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
DissolveSinkThenBecome43_13FC_at0:
    rts

; @routine 13FC:12A FollowSlot16UntilLow13FC
; @run 70:B92A
; WHAT IT DOES: In state 0 loads the record with frame $7B; later keeps slot $10's place and is cleared below $A8 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FollowSlot16UntilLow13FC:
    lda a:OBJ_STATE,x
    bne FollowSlot16UntilLow13FC_at0
    jsr VecLoadObjectRecord
    lda #$7B
    sta a:OBJ_FRAME,x
    rts
FollowSlot16UntilLow13FC_at0:
    ldy #$10
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_Y_HI,x
    bne FollowSlot16UntilLow13FC_at1
    lda a:OBJ_Y_LO,x
    cmp #$A8
    bcc FollowSlot16UntilLow13FC_at1
    jsr VecClearObject
FollowSlot16UntilLow13FC_at1:
    rts

; @routine 13FC:256 LoadTurnAndRise13FC
; @run 70:BA56
; WHAT IT DOES: Loads the record, OBJ_WORK_K opposite the next slot's, moves 12 dots left and rises (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadTurnAndRise13FC:
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_K+1,x
    clc
    adc #$80
    sta a:OBJ_WORK_K,x
    lda #$F4
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    rts

; @routine 13FC:275 ListRiseUntil13FC_275
; @run 70:BA75
; WHAT IT DOES: Plays the list at $BA94 while moving along Y; above $20 the animation restarted and the state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListRiseUntil13FC_275:
    lda #$94
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp #$20
    bcs ListRiseUntil13FC_275_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
ListRiseUntil13FC_275_at0:
    rts

; @routine 13FD:15F ListRiseUntil13FD_15F
; @run 71:C15F
; WHAT IT DOES: Plays the list at $C17E while moving along Y; above $40 the animation restarted and the state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListRiseUntil13FD_15F:
    lda #$7E
    sta z:zTemp10
    lda #$C1
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp #$40
    bcs ListRiseUntil13FD_15F_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
ListRiseUntil13FD_15F_at0:
    rts

; @routine 13FD:2EB AimWhenBothReady13FD
; @run 71:C2EB
; WHAT IT DOES: Once slots $11 and $12 are in substate 9: aims at the player, state on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AimWhenBothReady13FD:
    lda a:OBJ_SUBSTATE+17
    cmp #$09
    bne AimWhenBothReady13FD_at0
    lda a:OBJ_SUBSTATE+18
    cmp #$09
    bne AimWhenBothReady13FD_at0
    jsr VecAngleToPlayer
    sta a:OBJ_WORK_J,x
    lda #$01
    sta a:OBJ_WORK_D,x
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
AimWhenBothReady13FD_at0:
    rts

; @routine 18DB:4F1 AccelerateUp18DB
; @run 70:B4F1
; WHAT IT DOES: Moves along Y while the speed grows upward; above Y $4C the substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AccelerateUp18DB:
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    sec
    sbc #$0C
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_LO,x
    cmp #$4C
    bcs AccelerateUp18DB_at0
    inc a:OBJ_SUBSTATE,x
AccelerateUp18DB_at0:
    rts

; @routine 18DB:7EE SpawnInRow64_18DB
; @run 70:B7EE
; WHAT IT DOES: On even counts of OBJ_WORK_D, a kind $2E object in the first empty slot from $40, with the facing (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnInRow64_18DB:
    lda a:OBJ_WORK_D,x
    and #$01
    bne SpawnInRow64_18DB_at0
    ldy #$40
SpawnInRow64_18DB_at2:
    lda a:OBJ_KIND,y
    beq SpawnInRow64_18DB_at1
    iny
    cpy #$50
    bcc SpawnInRow64_18DB_at2
SpawnInRow64_18DB_at0:
    rts
SpawnInRow64_18DB_at1:
    jsr VecClearObjectInY
    lda #$2E
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    rts

; @routine 18DC:1BF SoundAndClearThree18DC
; @run 70:B9BF
; WHAT IT DOES: Sound $92, the hit box off, slots $11 to $13 cleared, the animation restarted, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundAndClearThree18DC:
    stz a:OBJ_WORK_J,x
    lda #$92
    jsr VecSoundCommand
    stz a:OBJ_HITBOX_ON,x
    ldy #$11
SoundAndClearThree18DC_at0:
    jsr VecClearObjectInY
    iny
    cpy #$14
    bcc SoundAndClearThree18DC_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 18DC:2EB LoadSpeedByFacingAndSlot18DC
; @run 70:BAEB
; WHAT IT DOES: Loads the record, X speed by facing and Y speed by slot from $30, moves, sound $4D (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadSpeedByFacingAndSlot18DCSpeedsX = $BB0B ; by facing
LoadSpeedByFacingAndSlot18DCSpeedsY = $BB0D ; by slot less $30
LoadSpeedByFacingAndSlot18DC:
    jsr VecLoadObjectRecord
    ldy a:OBJ_FACING_LEFT,x
    lda a:LoadSpeedByFacingAndSlot18DCSpeedsX,y
    sta a:OBJ_VX_HI,x
    txa
    sec
    sbc #$30
    tay
    lda a:LoadSpeedByFacingAndSlot18DCSpeedsY,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda #$4D
    jsr VecSoundCommand
    rts
