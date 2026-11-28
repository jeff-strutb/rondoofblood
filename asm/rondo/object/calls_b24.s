; @routine 09EC:340 ScatterBit09EC
; @run 70:BB40
; WHAT IT DOES: A bit thrown at random from the foot of the screen, one of eight frames, cleared below.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ScatterBit09ECSpeeds = $BB99 ; vx and vy, sixteen of four bytes
ScatterBit09EC:
    lda a:OBJ_STATE,x
    bne ScatterBit09EC_at0
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    stz a:OBJ_Y_HI,x
    lda #$C0
    sta a:OBJ_Y_LO,x
    jsr VecNextRandom
    and #$0F
    asl a
    asl a
    tay
    lda a:ScatterBit09ECSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:ScatterBit09ECSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:ScatterBit09ECSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:ScatterBit09ECSpeeds+2,y
    sta a:OBJ_VY_LO,x
    jsr VecNextRandom
    and #$07
    clc
    adc #$46
    sta a:OBJ_FRAME,x
    rts
ScatterBit09EC_at0:
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    beq ScatterBit09EC_at1
    jmp VecClearObject
ScatterBit09EC_at1:
    rts

; @routine 0E0C:5D8 LoadShardByArg0E0C
; @run 70:BDD8
; WHAT IT DOES: Loads a shard: placed by the start argument (mirrored by facing), then flying at its speeds.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadShardByArg0E0CPlaces = $BE2F ; first step by argument
LoadShardByArg0E0CSpeeds = $BE4D ; vx and vy by argument, four bytes
LoadShardByArg0E0C:
    jsr VecLoadObjectRecord
    lda a:OBJ_SPRITE_ATTR,x
    ora #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda a:OBJ_START_ARG,x
    clc
    adc #$4E
    sta a:OBJ_FRAME,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:LoadShardByArg0E0CPlaces,y
    sta a:OBJ_VX_HI,x
    lda a:LoadShardByArg0E0CPlaces+1,y
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT,x
    beq LoadShardByArg0E0C_at0
    jsr VecNegateVelocityX
LoadShardByArg0E0C_at0:
    jsr VecApplyVelocity
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    tay
    lda a:LoadShardByArg0E0CSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LoadShardByArg0E0CSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:LoadShardByArg0E0CSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:LoadShardByArg0E0CSpeeds+2,y
    sta a:OBJ_VY_LO,x
    lda a:OBJ_FACING_LEFT,x
    beq LoadShardByArg0E0C_at1
    jsr VecNegateVelocityX
LoadShardByArg0E0C_at1:
    rts

; @routine 0EBD:0ED FollowHolder0EBD
; @run 71:C0ED
; WHAT IT DOES: Held by the previous slot at an offset by its frame; gone when that slot is empty.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FollowHolder0EBD:
    lda a:OBJ_KIND-1,x
    bne FollowHolder0EBD_at0
    jmp VecClearObject
FollowHolder0EBD_at0:
    lda a:OBJ_STATE,x
    bne FollowHolder0EBD_at1
    jsr VecLoadObjectRecord
FollowHolder0EBD_at1:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc FollowHolder0EBD_at2
    lda #$12
    jsr VecSoundCommand
FollowHolder0EBD_at2:
    lda a:OBJ_FRAME-1,x
    cmp #$13
    beq FollowHolder0EBD_at3
    lda #$16
    sta z:zTemp00
    lda #$04
    sta z:zTemp01
FollowHolder0EBD_at4:
    lda z:zTemp00
    clc
    adc a:OBJ_X_LO-1,x
    sta a:OBJ_X_LO,x
    cla
    adc a:OBJ_X_HI-1,x
    sta a:OBJ_X_HI,x
    lda z:zTemp01
    clc
    adc a:OBJ_Y_LO-1,x
    sta a:OBJ_Y_LO,x
    cla
    adc a:OBJ_Y_HI-1,x
    sta a:OBJ_Y_HI,x
    rts
FollowHolder0EBD_at3:
    lda #$24
    sta z:zTemp00
    lda #$14
    sta z:zTemp01
    bra FollowHolder0EBD_at4

; @routine 101E:618 SpawnerPastStart101E
; @run 71:CE18
; WHAT IT DOES: Past the first $C0 of the level, starts kind $45 objects in slots $20-$22 at random intervals.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnerPastStart101E:
    lda a:OBJ_STATE,x
    bne SpawnerPastStart101E_at0
    jmp VecLoadObjectRecord
SpawnerPastStart101E_at0:
    lda a:ScrollXHiScreen
    bne SpawnerPastStart101E_at1
    lda a:ScrollXLo
    cmp #$C0
    bcc SpawnerPastStart101E_at2
SpawnerPastStart101E_at1:
    lda a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    beq SpawnerPastStart101E_at3
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
SpawnerPastStart101E_at2:
    rts
SpawnerPastStart101E_at3:
    ldy #$20
SpawnerPastStart101E_at5:
    lda a:OBJ_KIND,y
    beq SpawnerPastStart101E_at4
    iny
    cpy #$23
    bcc SpawnerPastStart101E_at5
    jsr VecNextRandom
    and #$3F
    clc
    adc #$40
    sta a:OBJ_WORK_E,x
    rts
SpawnerPastStart101E_at4:
    lda #$45
    jsr VecStartObjectAtPlace
    jsr VecNextRandom
    and #$3F
    sta a:OBJ_WORK_E,x
    lda #$01
    sta a:OBJ_WORK_D,x
    rts

; @routine 117E:521 StrikeByFrame117E
; @run 71:CD21
; WHAT IT DOES: Lunges on frames $0C-$0D (sound at $0E) with the next slot's OBJ_WORK_C 3; at the list's end 2 and state 4.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StrikeByFrame117ESpeeds = $CD78 ; vx by frame and facing
StrikeByFrame117E:
    lda #$03
    sta a:OBJ_WORK_C+1,x
    lda #$CD
    sta z:zTemp11
    lda #$80
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq StrikeByFrame117E_at0
    bcc StrikeByFrame117E_at1
    lda a:OBJ_FRAME,x
    cmp #$0E
    bne StrikeByFrame117E_at2
    lda #$1A
    jsr VecSoundCommand
StrikeByFrame117E_at2:
    lda a:OBJ_FRAME,x
    sec
    sbc #$0C
    cmp #$02
    bcs StrikeByFrame117E_at1
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_FACING_LEFT,x
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:StrikeByFrame117ESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:StrikeByFrame117ESpeeds,y
    sta a:OBJ_VX_LO,x
    jsr ProbeHeightCall117E_2E7
StrikeByFrame117E_at1:
    rts
StrikeByFrame117E_at0:
    lda #$02
    sta a:OBJ_WORK_C+1,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    rts

; @routine 12DC:6F6 ChargeCountHits12DC
; @run 70:BEF6
; WHAT IT DOES: Charges by the facing; at the list's end counts it, state 2, or state 4 substate 6 on the third.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChargeCountHits12DCSpeeds = $BF55 ; vx by facing
ChargeCountHits12DC:
    lda #$4E
    sta z:zTemp10
    lda #$BF
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ChargeCountHits12DC_at0
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:ChargeCountHits12DCSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:ChargeCountHits12DCSpeeds,y
    sta a:OBJ_VX_LO,x
    jsr MoveBounceOffWall12DC
    lda a:OBJ_WORK_D,x
    clc
    adc #$04
    sta a:OBJ_WORK_D,x
    rts
ChargeCountHits12DC_at0:
    lda #$08
    sta a:OBJ_INVULN_TIMER,x
    lsr a:OBJ_WORK_K,x
    inc a:OBJ_WORK_P,x
    lda a:OBJ_WORK_P,x
    lda #$02
    sta a:OBJ_STATE,x
    lda a:OBJ_WORK_P,x
    cmp #$03
    bcc ChargeCountHits12DC_at1
    lda #$04
    sta a:OBJ_STATE,x
    lda #$06
    sta a:OBJ_SUBSTATE,x
    lda #$20
    sta a:OBJ_WORK_K,x
    stz a:OBJ_WORK_P,x
ChargeCountHits12DC_at1:
    rts

; @routine 13FB:30F DecideWhenNearOrHit13FB
; @run 70:B30F
; WHAT IT DOES: Hit, or the player near for $18 counts: halves OBJ_WORK_G and goes to state 3, OBJ_WORK_K 1 now and then at the right end.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DecideWhenNearOrHit13FB:
    lda a:OBJ_WORK_L,x
    beq DecideWhenNearOrHit13FB_at0
    lda a:OBJ_HIT_TAKEN,x
    bne DecideWhenNearOrHit13FB_at1
    jsr VecDistanceXToPlayer
    cmp #$50
    bcs DecideWhenNearOrHit13FB_at0
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_F,x
    cmp #$18
    bcs DecideWhenNearOrHit13FB_at1
    rts
DecideWhenNearOrHit13FB_at1:
    lsr a:OBJ_WORK_G,x
    stz a:OBJ_WORK_K,x
    jsr VecNextRandom
    and #$01
    beq DecideWhenNearOrHit13FB_at2
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:ScrollXHiScreen
    cmp #$03
    bne DecideWhenNearOrHit13FB_at2
    lda z:zTemp00
    bpl DecideWhenNearOrHit13FB_at2
    lda #$01
    sta a:OBJ_WORK_K,x
DecideWhenNearOrHit13FB_at2:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_WORK_F,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
DecideWhenNearOrHit13FB_at0:
    rts

; @routine 161C:399 PaceUntilHit161C
; @run 70:BB99
; WHAT IT DOES: Paces, turning at walls and at the screen's edges; a hard hit on an argument with bit 1 goes on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PaceUntilHit161CSpeeds = $BBFE ; vx by facing
PaceUntilHit161C:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc PaceUntilHit161C_at0
    lda a:OBJ_START_ARG,x
    and #$02
    beq PaceUntilHit161C_at0
    lda #$01
    sta a:OBJ_WORK_K,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    rts
PaceUntilHit161C_at0:
    lda #$EF
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    ldy a:OBJ_FACING_LEFT,x
    lda a:PaceUntilHit161CSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$08
    sta z:zProduct1
    jsr BounceAtWalls
    bcs PaceUntilHit161C_at1
    lda a:OBJ_WORK_J,x
    beq PaceUntilHit161C_at2
    dec a:OBJ_WORK_J,x
    rts
PaceUntilHit161C_at2:
    lda a:OBJ_X_HI,x
    beq PaceUntilHit161C_at3
PaceUntilHit161C_at1:
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$10
    sta a:OBJ_WORK_J,x
PaceUntilHit161C_at3:
    rts

; @routine 16CB:315 SwingBack16CB
; @run 70:B315
; WHAT IT DOES: Swings by angle OBJ_WORK_E, falling to $E0; reaching Y $B6 the sound is cut and substate 7.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwingBack16CB:
    lda #$6A
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_Y_LO,x
    cmp #$B6
    bcc SwingBack16CB_at0
    smb4 z:zField8E
    lda #$FE
    jsr VecSoundCommand
    lda #$72
    jsr VecSoundCommand
    lda #$07
    sta a:OBJ_SUBSTATE,x
    lda #$B6
    sta a:OBJ_Y_LO,x
    lda #$20
    sta a:OBJ_WORK_D,x
    rts
SwingBack16CB_at0:
    lda #$20
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_E,x
    sec
    sbc #$02
    sta a:OBJ_WORK_E,x
    cmp #$E0
    bne SwingBack16CB_at1
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
SwingBack16CB_at1:
    rts

; @routine 16CB:373 SwingForth16CB
; @run 70:B373
; WHAT IT DOES: Swings by angle OBJ_WORK_E, rising to $A0; reaching Y $B6 the sound is cut and substate 7.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwingForth16CB:
    lda #$6A
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_Y_LO,x
    cmp #$B6
    bcc SwingForth16CB_at0
    smb4 z:zField8E
    lda #$FE
    jsr VecSoundCommand
    lda #$72
    jsr VecSoundCommand
    lda #$07
    sta a:OBJ_SUBSTATE,x
    lda #$B6
    sta a:OBJ_Y_LO,x
    lda #$20
    sta a:OBJ_WORK_D,x
    rts
SwingForth16CB_at0:
    lda #$20
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_E,x
    clc
    adc #$02
    sta a:OBJ_WORK_E,x
    cmp #$A0
    bne SwingForth16CB_at1
    lda #$40
    sta a:OBJ_WORK_E,x
    dec a:OBJ_SUBSTATE,x
SwingForth16CB_at1:
    rts
