; @routine 05CB:706 SinkToB8_05CB
; @run 70:B706
; WHAT IT DOES: Every fourth count of OBJ_WORK_D sinks with growing speed to Y $B8 (secret stage 02's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SinkToB8_05CB:
    lda a:OBJ_WORK_D,x
    and #$03
    bne SinkToB8_05CB_at0
    lda a:OBJ_VY_LO,x
    clc
    adc #$0E
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp #$B8
    bcc SinkToB8_05CB_at0
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$B8
    sta a:OBJ_Y_LO,x
SinkToB8_05CB_at0:
    rts

; @routine 05CB:7EF FallDissolvingThenBecome22_05CB
; @run 70:B7EF
; WHAT IT DOES: Falls eroding patterns every fourth count; below a screen the slot becomes kind $22 (secret stage 02's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallDissolvingThenBecome22_05CB:
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_D,x
    and #$03
    bne FallDissolvingThenBecome22_05CB_at0
    lda a:OBJ_WORK_I,x
    clc
    adc #$13
    and #$3F
    sta a:OBJ_WORK_I,x
    jsr EachOfPatterns05CC_01E
FallDissolvingThenBecome22_05CB_at0:
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne FallDissolvingThenBecome22_05CB_at1
    lda a:OBJ_Y_LO,x
    cmp #$10
    bcc FallDissolvingThenBecome22_05CB_at1
    jsr VecClearObject
    lda #$22
    sta a:OBJ_KIND,x
FallDissolvingThenBecome22_05CB_at1:
    rts

; @routine 088C:043 BobThenClear088C
; @run 70:B843
; WHAT IT DOES: A Y speed of the cosine of OBJ_WORK_E scaled by $2000, the angle stepped; cleared when OBJ_WORK_F counts out (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BobThenClear088C:
    lda #$20
    sta z:zTemp01
    lda #$00
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:zProduct1
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_E,x
    clc
    adc #$20
    sta a:OBJ_WORK_E,x
    dec a:OBJ_WORK_F,x
    bne BobThenClear088C_at0
    jmp VecClearObject
BobThenClear088C_at0:
    rts

; @routine 0B4C:584 ListThenJumpWhen0B4C
; @run 70:BD84
; WHAT IT DOES: Plays the list at $BDB2 while $BDBB and $B7CC run; when C is left set, jumps by facing (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenJumpWhen0B4CSpeeds = $BDB9 ; X speed high byte by facing
ListThenJumpWhen0B4C:
    lda #$BD
    sta z:zTemp11
    lda #$B2
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_H,x
    jsr MaybeSound19AndSpawnPiece
    jsr MoveXAndProbeWallBounce
    bcc ListThenJumpWhen0B4C_at0
    ldy a:OBJ_FACING_LEFT,x
    lda a:ListThenJumpWhen0B4CSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$FA
    sta a:OBJ_VY_HI,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
ListThenJumpWhen0B4C_at0:
    rts

; @routine 0CAC:7D6 LoadOnCircle0CAC
; @run 70:BFD6
; WHAT IT DOES: Loads on a circle of radius $50 around the screen's centre at the angle of OBJ_WORK_D (secret stage 07's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadOnCircle0CACAngles = $C005 ; the angle by OBJ_WORK_D
LoadOnCircle0CAC:
    jsr VecLoadObjectRecord
    stz a:OBJ_X_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$70
    sta a:OBJ_Y_LO,x
    ldy a:OBJ_WORK_D,x
    lda a:LoadOnCircle0CACAngles,y
    sta a:OBJ_WORK_E,x
    stz z:zTemp00
    lda #$50
    sta z:zTemp01
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 0E0C:143 ListFallCountThen0E0C
; @run 70:B943
; WHAT IT DOES: Falls playing the list at $B972 while OBJ_WORK_E counts; then the state on, or with a start argument sound $CF and state 5 (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListFallCountThen0E0C:
    lda #$72
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    jsr VecApplyGravity
    dec a:OBJ_WORK_E,x
    bne ListFallCountThen0E0C_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    lda a:OBJ_START_ARG,x
    beq ListFallCountThen0E0C_at0
    lda #$CF
    jsr VecSoundCommand
    lda #$05
    sta a:OBJ_STATE,x
ListFallCountThen0E0C_at0:
    rts

; @routine 0EBD:5BB LoadAboveRightWithPartner0EBD
; @run 71:C5BB
; WHAT IT DOES: Loads above and right of the screen with a kind $B0 partner in the next slot (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAboveRightWithPartner0EBD:
    jsr VecLoadObjectRecord
    lda #$18
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$E8
    sta a:OBJ_Y_LO,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$50
    sta a:OBJ_WORK_P,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$B0
    sta a:OBJ_KIND,y
    lda a:OBJ_START_ARG,x
    sta a:OBJ_START_ARG,y
    rts

; @routine 101C:39B PickNextSubstate101C
; @run 70:BB9B
; WHAT IT DOES: Off screen goes on at $BC87; else the next substate and count of a cycle once frame $74 shows (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PickNextSubstate101CCycle = $BBCA ; eight substates
PickNextSubstate101CCounts = $BBD2 ; OBJ_WORK_F with each
PickNextSubstate101COff = $BC87 ; a routine of this overlay
PickNextSubstate101C:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_X_HI,x
    beq PickNextSubstate101C_at0
    jmp PickNextSubstate101COff
PickNextSubstate101C_at0:
    lda a:OBJ_FRAME,x
    cmp #$74
    beq PickNextSubstate101C_at1
    inc a:OBJ_SUBSTATE,x
    rts
PickNextSubstate101C_at1:
    lda a:OBJ_WORK_H,x
    and #$07
    tay
    lda a:PickNextSubstate101CCycle,y
    sta a:OBJ_SUBSTATE,x
    lda a:PickNextSubstate101CCounts,y
    sta a:OBJ_WORK_F,x
    inc a:OBJ_WORK_H,x
    rts

; @routine 101C:545 LoadShotByArg101C
; @run 70:BD45
; WHAT IT DOES: Loads, steps once by facing and start argument, then takes its X speed (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadShotByArg101CStepX = $BD74 ; by facing
LoadShotByArg101CStepY = $BD76 ; by start argument
LoadShotByArg101CSpeeds = $BD78 ; X speed words by facing
LoadShotByArg101C:
    jsr VecLoadObjectRecord
    lda #$F4
    sta a:OBJ_HITBOX_DX,x
    ldy a:OBJ_FACING_LEFT,x
    lda a:LoadShotByArg101CStepX,y
    sta a:OBJ_VX_HI,x
    ldy a:OBJ_START_ARG,x
    lda a:LoadShotByArg101CStepY,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:LoadShotByArg101CSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LoadShotByArg101CSpeeds,y
    sta a:OBJ_VX_LO,x
    rts

; @routine 101D:7A3 SegmentBySlot101D
; @run 71:C7A3
; WHAT IT DOES: A piece of a row from slot $20: waits eight frames a slot after $10, its frame by slot, then falls (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SegmentBySlot101D:
    lda a:OBJ_STATE,x
    bne SegmentBySlot101D_at0
    jsr VecLoadObjectRecord
    txa
    sec
    sbc #$20
    pha
    asl a
    asl a
    asl a
    clc
    adc #$10
    sta a:OBJ_WORK_D,x
    pla
    clc
    adc #$29
    sta a:OBJ_FRAME,x
    rts
SegmentBySlot101D_at0:
    lda a:OBJ_WORK_D,x
    beq SegmentBySlot101D_at1
    dec a:OBJ_WORK_D,x
    rts
SegmentBySlot101D_at1:
    jsr VecApplyGravity
    jsr VecApplyVelocityY
    rts

; @routine 10CC:3FA FallCountThenShoot10CC
; @run 70:BBFA
; WHAT IT DOES: Falls while OBJ_WORK_E counts; then sound $45 and a drop with the X speed of OBJ_WORK_O (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallCountThenShoot10CCSpeeds = $BC27 ; X speed words by OBJ_WORK_O
FallCountThenShoot10CC:
    jsr VecApplyGravity
    jsr VecApplyVelocity
    dec a:OBJ_WORK_E,x
    bne FallCountThenShoot10CC_at0
    lda #$45
    jsr VecSoundCommand
    lda a:OBJ_WORK_O,x
    asl a
    tay
    lda a:FallCountThenShoot10CCSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:FallCountThenShoot10CCSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda #$02
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
FallCountThenShoot10CC_at0:
    rts

; @routine 117E:27C ListByAThenCheck117E
; @run 71:CA7C
; WHAT IT DOES: The list and X speed of A's row; on new frames $CACF and $CAE7, the speed reversed when OBJ_WORK_H (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListByAThenCheck117ERows = $CAA9 ; a frame list then an X speed word, four bytes a row
ListByAThenCheck117EFirst = $CACF ; a routine of this overlay
ListByAThenCheck117E:
    asl a
    asl a
    tay
    lda a:ListByAThenCheck117ERows,y
    sta z:zTemp10
    lda a:ListByAThenCheck117ERows+1,y
    sta z:zTemp11
    lda a:ListByAThenCheck117ERows+2,y
    sta a:OBJ_VX_LO,x
    lda a:ListByAThenCheck117ERows+3,y
    sta a:OBJ_VX_HI,x
    jsr VecStepObjectFrames
    bcc ListByAThenCheck117E_at0
    jsr ListByAThenCheck117EFirst
    lda a:OBJ_WORK_H,x
    beq ListByAThenCheck117E_at1
    jsr VecNegateVelocityX
ListByAThenCheck117E_at1:
    jsr ProbeHeightCall117E_2E7
ListByAThenCheck117E_at0:
    rts

; @routine 117F:379 LoadWithArms117F
; @run 71:D379
; WHAT IT DOES: Loads with a kind $35 object in the slot before and a $37 in the slot after (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadWithArms117F:
    jsr VecLoadObjectRecord
    txa
    tay
    dey
    jsr VecClearObjectInY
    lda #$35
    jsr VecStartObjectAtPlace
    iny
    iny
    jsr VecClearObjectInY
    lda #$37
    jsr VecStartObjectAtPlace
    lda #$50
    sta a:OBJ_WORK_E,x
    jsr VecNextRandom
    sta a:OBJ_WORK_H,x
    lda a:OBJ_START_ARG,x
    bne LoadWithArms117F_at0
    rts
LoadWithArms117F_at0:
    inc a:OBJ_STATE,x
    rts

; @routine 12DE:569 DripWhileEarly12DE
; @run 71:CD69
; WHAT IT DOES: In the first four animation steps, every fifth call a kind $39 object at the next of 16 X places (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DripWhileEarly12DEPlaces = $CD96 ; 16 X lows
DripWhileEarly12DE:
    lda a:OBJ_ANIM_STEP,x
    cmp #$04
    bcs DripWhileEarly12DE_at0
    dec a:OBJ_WORK_K,x
    bne DripWhileEarly12DE_at0
    lda #$05
    sta a:OBJ_WORK_K,x
    jsr VecFindFreeSlotFrom30
    bcc DripWhileEarly12DE_at0
    lda #$39
    jsr VecStartObjectAtPlace
    phx
    lda a:OBJ_WORK_J,x
    and #$0F
    tax
    lda a:DripWhileEarly12DEPlaces,x
    sta a:OBJ_X_LO,y
    plx
    inc a:OBJ_WORK_J,x
DripWhileEarly12DE_at0:
    rts

; @routine 13FB:5AF ListThenLeapByFacing13FB
; @run 70:B5AF
; WHAT IT DOES: Plays the list at $B5DC; then leaps by facing with no hit box, substate on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenLeapByFacing13FBSpeeds = $B5EB ; X speed words by facing
ListThenLeapByFacing13FB:
    lda #$DC
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenLeapByFacing13FB_at0
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:ListThenLeapByFacing13FBSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:ListThenLeapByFacing13FBSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$F7
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
ListThenLeapByFacing13FB_at0:
    rts

; @routine 13FD:289 AfterImage13FD
; @run 71:C289
; WHAT IT DOES: An after-image of slot $10: shown every fourth of 24 counts with its facing and frame, then cleared (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AfterImage13FD:
    lda a:OBJ_STATE,x
    bne AfterImage13FD_at0
    jsr VecLoadObjectRecord
    lda #$18
    sta a:OBJ_WORK_D,x
    rts
AfterImage13FD_at0:
    dec a:OBJ_WORK_D,x
    bne AfterImage13FD_at1
    jmp VecClearObject
AfterImage13FD_at1:
    stz a:OBJ_FRAME,x
    lda a:OBJ_WORK_D,x
    and #$03
    bne AfterImage13FD_at2
    lda a:OBJ_FACING_LEFT+16
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_FRAME+16
    sta a:OBJ_FRAME,x
AfterImage13FD_at2:
    rts

; @routine 161C:4DB SoundAtStep9ThenState4_161C
; @run 70:BCDB
; WHAT IT DOES: Plays the list at $BD09 sounding $1A at step 9; then OBJ_WORK_D 64 and state 4 (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundAtStep9ThenState4_161C:
    lda a:OBJ_ANIM_STEP,x
    cmp #$09
    bne SoundAtStep9ThenState4_161C_at0
    lda a:OBJ_ANIM_TIMER,x
    cmp #$03
    bne SoundAtStep9ThenState4_161C_at0
    lda #$1A
    jsr VecSoundIfOnFirstScreen
SoundAtStep9ThenState4_161C_at0:
    lda #$09
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne SoundAtStep9ThenState4_161C_at1
    lda #$40
    sta a:OBJ_WORK_D,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SoundAtStep9ThenState4_161C_at1:
    rts

; @routine 161F:1D4 LoadItemByArg161F
; @run 71:D1D4
; WHAT IT DOES: Loads, unpacks the start argument into OBJ_WORK_D to F and becomes kind $A5 (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadItemByArg161F:
    jsr VecLoadObjectRecord
    lda #$80
    clc
    adc a:OBJ_HITBOX_ON,x
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_START_ARG,x
    tay
    and #$0F
    ora #$10
    sta a:OBJ_WORK_E,x
    tya
    rol a
    rol a
    rol a
    tay
    and #$03
    sta a:OBJ_WORK_F,x
    tya
    rol a
    rol a
    and #$03
    sta a:OBJ_WORK_D,x
    lda #$A5
    sta a:OBJ_KIND,x
    rts

; @routine 16CC:1BD ListRiseFasterThenClear16CC
; @run 70:B9BD
; WHAT IT DOES: Rises ever faster playing the list at $B9EA, cleared once well off the screen (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListRiseFasterThenClear16CC:
    lda #$EA
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    sec
    sbc #$20
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_HI,x
    beq ListRiseFasterThenClear16CC_at0
    lda a:OBJ_Y_LO,x
    bmi ListRiseFasterThenClear16CC_at0
    jmp VecClearObject
ListRiseFasterThenClear16CC_at0:
    rts

; @routine 16CC:3A9 ListSinkToB8Then16CC
; @run 70:BBA9
; WHAT IT DOES: Falls playing the list at $BBD7 to Y $B8; then attribute $10, no hit box, state on (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSinkToB8Then16CC:
    lda #$D7
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$B8
    bcc ListSinkToB8Then16CC_at0
    lda #$B8
    sta a:OBJ_Y_LO,x
    jsr VecNextRandom
    lda #$20
    sta a:OBJ_WORK_D,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    stz a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
ListSinkToB8Then16CC_at0:
    rts

; @routine 18DC:21C CountThenOutburst18DC
; @run 70:BA1C
; WHAT IT DOES: Counts OBJ_WORK_E; then scripts, a list channel, slot $40's state on, frame $11 at Y $76 (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenOutburst18DC:
    dec a:OBJ_WORK_E,x
    bne CountThenOutburst18DC_at0
    phx
    lda #$86
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$88
    ldx #$04
    jsr VecStartScriptSlotKind1
    lda #$8F
    ldx #$04
    jsr VecStartListChannelKind2
    plx
    inc a:OBJ_STATE+64
    lda #$11
    sta a:OBJ_FRAME,x
    lda #$76
    sta a:OBJ_Y_LO,x
    inc a:OBJ_SUBSTATE,x
CountThenOutburst18DC_at0:
    rts

; @routine 18DC:30F ListThenWalkByFacing18DC
; @run 70:BB0F
; WHAT IT DOES: Plays the list at $BB3C; then walks by facing, hit-tested, state on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenWalkByFacing18DCSpeeds = $BB6B ; X speed words by facing
ListThenWalkByFacing18DC:
    lda #$3C
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenWalkByFacing18DC_at0
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:ListThenWalkByFacing18DCSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:ListThenWalkByFacing18DCSpeeds+1,y
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
ListThenWalkByFacing18DC_at0:
    rts
