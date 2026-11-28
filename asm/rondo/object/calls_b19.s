; @routine 0B4A:679 BlinkThenWakeOther0B4A
; @run 70:AE79
; WHAT IT DOES: State 2 blinks for 4 counts; state 3 sets the slot in the start argument to state 2 and goes.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BlinkThenWakeOther0B4AFrames = $AEB4 ; the two blink frames
BlinkThenWakeOther0B4A:
    lda a:OBJ_STATE,x
    bne BlinkThenWakeOther0B4A_at0
    jsr VecLoadObjectRecord
    stz a:OBJ_FRAME,x
    lda #$04
    sta a:OBJ_WORK_D,x
    rts
BlinkThenWakeOther0B4A_at0:
    dec a
    bne BlinkThenWakeOther0B4A_at1
BlinkThenWakeOther0B4A_at3:
    rts
BlinkThenWakeOther0B4A_at1:
    dec a
    bne BlinkThenWakeOther0B4A_at2
    lda a:OBJ_WORK_D,x
    and #$01
    tay
    lda a:BlinkThenWakeOther0B4AFrames,y
    sta a:OBJ_FRAME,x
    dec a:OBJ_WORK_D,x
    bne BlinkThenWakeOther0B4A_at3
    inc a:OBJ_STATE,x
    rts
BlinkThenWakeOther0B4A_at2:
    lda a:OBJ_START_ARG,x
    beq BlinkThenWakeOther0B4A_at3
    tay
    lda #$02
    sta a:OBJ_STATE,y
    jmp VecClearObject

; @routine 0B4C:19B ListSpit0B4C
; @run 70:B99B
; WHAT IT DOES: Runs the list at $B9D5, spitting kind $25 at frame $15; at its end clears it and goes to state 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSpit0B4C:
    lda #$B9
    sta z:zTemp11
    lda #$D5
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq ListSpit0B4C_at0
    bcc ListSpit0B4C_at1
    lda a:OBJ_FRAME,x
    cmp #$15
    bne ListSpit0B4C_at1
    lda #$20
    jsr VecSoundCommand
    ldy #$51
    lda #$25
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    rts
ListSpit0B4C_at0:
    ldy #$51
    jsr VecClearObjectInY
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
ListSpit0B4C_at1:
    rts

; @routine 0CAC:0CE CircleCentre0CAC
; @run 70:B8CE
; WHAT IT DOES: Circles the screen's centre at radius $50 by OBJ_WORK_E; at angle $C0 the substate counts on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CircleCentre0CAC:
    lda #$AB
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    stz a:OBJ_X_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$70
    sta a:OBJ_Y_LO,x
    stz z:zTemp00
    lda #$50
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    inc a:OBJ_WORK_E,x
    jsr StartAtCountWithSound0CAC
    lda a:OBJ_WORK_E,x
    cmp #$C0
    bne CircleCentre0CAC_at0
    inc a:OBJ_SUBSTATE,x
CircleCentre0CAC_at0:
    rts

; @routine 0EBC:2BE SprinkleKind9A0EBC
; @run 70:BABE
; WHAT IT DOES: Sound and load; then each time a kind $9A object scattered 0-15 to the right.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SprinkleKind9A0EBC:
    lda a:OBJ_STATE,x
    bne SprinkleKind9A0EBC_at0
    lda #$59
    jsr VecSoundCommand
    jmp VecLoadObjectRecord
SprinkleKind9A0EBC_at0:
    jsr VecFindFreeSlotFrom20
    bcc SprinkleKind9A0EBC_at1
    lda #$9A
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_D,x
    and #$07
    sta a:OBJ_WORK_E,y
    lda #$10
    sta a:OBJ_SPRITE_ATTR,y
    inc a:OBJ_WORK_D,x
    jsr VecNextRandom
    and #$0F
    clc
    adc a:OBJ_X_LO,y
    sta a:OBJ_X_LO,y
    cla
    adc a:OBJ_X_HI,y
    sta a:OBJ_X_HI,y
SprinkleKind9A0EBC_at1:
    rts

; @routine 0EBE:460 ThrowThenWait0EBE
; @run 71:CC60
; WHAT IT DOES: Animates, throwing kind $39 at frame $2C; at the list's end (or off the first screen) state 3 with the next wait.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ThrowThenWait0EBEWaits = $CC9A ; four waits
ThrowThenWait0EBE:
    lda a:OBJ_Y_HI,x
    bne ThrowThenWait0EBE_at0
    ldy #$02
    jsr FaceAndAnimate0EBE
    beq ThrowThenWait0EBE_at0
    bcc ThrowThenWait0EBE_at1
    lda a:OBJ_FRAME,x
    cmp #$2C
    bne ThrowThenWait0EBE_at1
    jsr VecFindFreeSlotFrom30
    bcc ThrowThenWait0EBE_at1
    lda #$39
    jmp VecStartObjectAtPlace
ThrowThenWait0EBE_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_F,x
    and #$03
    tay
    lda a:ThrowThenWait0EBEWaits,y
    sta a:OBJ_WORK_D,x
ThrowThenWait0EBE_at1:
    rts

; @routine 0EBF:15E DiveWhenNear0EBF
; @run 71:D15E
; WHAT IT DOES: On screen and within $60 across: dives (state 4, substate 2) with a random time.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DiveWhenNear0EBF:
    cly
    jsr FaceAndAnimate0EBF
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne DiveWhenNear0EBF_at0
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs DiveWhenNear0EBF_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_SCROLL_BAND,x
    and #$80
    sta a:OBJ_SCROLL_BAND,x
    lda #$03
    sta a:OBJ_VY_HI,x
    jsr VecNextRandom
    and #$7F
    clc
    adc #$44
    sta a:OBJ_WORK_J,x
    lda #$04
    sta a:OBJ_STATE,x
    lda #$02
    sta a:OBJ_SUBSTATE,x
DiveWhenNear0EBF_at0:
    rts

; @routine 0EBF:3B5 FlyTillLevel0EBF
; @run 71:D3B5
; WHAT IT DOES: Flies until its Y speed levels off (0 or $FFFF) or it is above Y $10; then state 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FlyTillLevel0EBFStepD2C9 = $D2C9 ; a routine of this overlay
FlyTillLevel0EBFStepD3A8 = $D3A8 ; a routine of this overlay
FlyTillLevel0EBF:
    ldy #$04
    jsr FaceAndAnimate0EBF
    jsr FlyTillLevel0EBFStepD3A8
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne FlyTillLevel0EBF_at0
    lda a:OBJ_Y_LO,x
    cmp #$10
    bcc FlyTillLevel0EBF_at1
    lda a:OBJ_WORK_H,x
    and #$01
    bne FlyTillLevel0EBF_at0
    jsr FlyTillLevel0EBFStepD2C9
    lda a:OBJ_VY_HI,x
    ora a:OBJ_VY_LO,x
    beq FlyTillLevel0EBF_at1
    lda a:OBJ_VY_HI,x
    and a:OBJ_VY_LO,x
    cmp #$FF
    bne FlyTillLevel0EBF_at0
FlyTillLevel0EBF_at1:
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
FlyTillLevel0EBF_at0:
    rts

; @routine 0EC0:2B0 LoadAtRandomSide0EC0
; @run 71:DAB0
; WHAT IT DOES: Loads at one side of the screen, at one of four heights, with that side's speed.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAtRandomSide0EC0Sides = $DAEB ; vx and x by side, four bytes
LoadAtRandomSide0EC0Heights = $DAF3 ; four heights
LoadAtRandomSide0EC0:
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    sta a:OBJ_WORK_D,x
    and #$01
    asl a
    asl a
    tay
    lda a:LoadAtRandomSide0EC0Sides+1,y
    sta a:OBJ_VX_HI,x
    lda a:LoadAtRandomSide0EC0Sides,y
    sta a:OBJ_VX_LO,x
    lda a:LoadAtRandomSide0EC0Sides+3,y
    sta a:OBJ_X_HI,x
    lda a:LoadAtRandomSide0EC0Sides+2,y
    sta a:OBJ_X_LO,x
    jsr VecNextRandom
    and #$03
    tay
    lda a:LoadAtRandomSide0EC0Heights,y
    sta a:OBJ_Y_LO,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    rts

; @routine 10CC:02D BobWhileList10CC
; @run 70:B82D
; WHAT IT DOES: Bobs while its list runs; then starts upward, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BobWhileList10CC:
    lda #$30
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_WORK_G,x
    jsr VecScaleByCosine
    lda z:zProduct1
    sta a:OBJ_VY_LO,x
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_G,x
    clc
    adc #$02
    sta a:OBJ_WORK_G,x
    lda #$68
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne BobWhileList10CC_at0
    lda #$FC
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
BobWhileList10CC_at0:
    rts

; @routine 117C:00E FlyTillLevel117C
; @run 70:B80E
; WHAT IT DOES: Flies until its Y speed levels off (0 or $FFFF) or it is above Y $10; then state 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FlyTillLevel117CStepB722 = $B722 ; a routine of this overlay
FlyTillLevel117C:
    ldy #$04
    jsr StepFramesFacingPlayerD
    jsr SoundEveryFourthFrameC
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne FlyTillLevel117C_at0
    lda a:OBJ_Y_LO,x
    cmp #$10
    bcc FlyTillLevel117C_at1
    lda a:OBJ_WORK_H,x
    and #$01
    bne FlyTillLevel117C_at0
    jsr FlyTillLevel117CStepB722
    lda a:OBJ_VY_HI,x
    ora a:OBJ_VY_LO,x
    beq FlyTillLevel117C_at1
    lda a:OBJ_VY_HI,x
    and a:OBJ_VY_LO,x
    cmp #$FF
    bne FlyTillLevel117C_at0
FlyTillLevel117C_at1:
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
FlyTillLevel117C_at0:
    rts

; @routine 117C:38C LandAnimateByRise117C
; @run 70:BB8C
; WHAT IT DOES: Fades its attribute by OBJ_WORK_H, stops on landing, and shows frame $1B falling or the list rising.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandAnimateByRise117C:
    lda a:OBJ_WORK_H,x
    beq LandAnimateByRise117C_at0
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    clc
    adc #$8C
    sta a:OBJ_SPRITE_ATTR,x
    dec a:OBJ_WORK_H,x
LandAnimateByRise117C_at0:
    jsr LandOnRowThenDebris117C
    lda #$08
    sta z:zProduct1
    jsr VecStopOnLanding
    bcc FrameOneBWhenFallingElseAnimate
    stz a:OBJ_SUBSTATE,x
    inc a:OBJ_STATE,x

; @routine 12DB:25E CountThenFire12DB
; @run 70:B25E
; WHAT IT DOES: Runs the list and $B297; after OBJ_WORK_D counts: script $86, a kind $27 object in slot $40, sound $72, rising, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenFire12DB:
    lda #$68
    sta z:zTemp10
    lda #$B1
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr SpawnOnOddFrames12DB
    dec a:OBJ_WORK_D,x
    bne CountThenFire12DB_at0
    phx
    lda #$86
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    ldy #$40
    jsr VecClearObjectInY
    lda #$27
    jsr VecStartObjectAtPlace
    lda #$72
    jsr VecSoundCommand
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    inc a:OBJ_STATE,x
CountThenFire12DB_at0:
    rts

; @routine 12DB:70B RiseToTop12DB
; @run 70:B70B
; WHAT IT DOES: Rises faster each count; at Y $40 stops there and the substate counts on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RiseToTop12DB:
    lda #$82
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    jsr RasterWhenLow12DB
    jsr DripWhenLow12DB
    lda a:OBJ_VY_LO,x
    sec
    sbc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_LO,x
    cmp #$40
    bcs RiseToTop12DB_at0
    lda #$40
    sta a:OBJ_Y_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
RiseToTop12DB_at0:
    rts

; @routine 12DC:0E1 LoadSpreadBySlot12DC
; @run 70:B8E1
; WHAT IT DOES: Loads by the slot from $20: a count, a first displacement, then a speed up or down.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadSpreadBySlot12DCOffsets = $B91B ; first step by slot
LoadSpreadBySlot12DCSpeeds = $B935 ; vy by slot, four bytes
LoadSpreadBySlot12DCWaits = $B963 ; OBJ_WORK_E by slot
LoadSpreadBySlot12DC:
    jsr VecLoadObjectRecord
    txa
    sec
    sbc #$20
    tay
    lda a:LoadSpreadBySlot12DCWaits,y
    sta a:OBJ_WORK_E,x
    tya
    asl a
    tay
    lda a:LoadSpreadBySlot12DCOffsets,y
    sta a:OBJ_VX_HI,x
    lda a:LoadSpreadBySlot12DCOffsets+1,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    txa
    sec
    sbc #$20
    asl a
    asl a
    tay
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda a:LoadSpreadBySlot12DCSpeeds+1,y
    sta a:OBJ_VY_HI,x
    lda a:LoadSpreadBySlot12DCSpeeds,y
    sta a:OBJ_VY_LO,x
    rts

; @routine 12DE:1E5 PopThenRise12DE
; @run 71:C9E5
; WHAT IT DOES: Loads at a random height below; then rises animated until its list ends.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PopThenRise12DE:
    lda a:OBJ_STATE,x
    bne PopThenRise12DE_at0
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$1C
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    rts
PopThenRise12DE_at0:
    lda #$8C
    sta a:OBJ_SPRITE_ATTR,x
    jsr VecApplyVelocityY
    lda #$1F
    sta z:zTemp10
    lda #$CA
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne PopThenRise12DE_at1
    jmp VecClearObject
PopThenRise12DE_at1:
    rts

; @routine 12DE:348 MoveBounceByTable12DE
; @run 71:CB48
; WHAT IT DOES: Moves across at the speed OBJ_WORK_E picks, turning round at a wall.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
MoveBounceByTable12DESpeeds = $CB81 ; vx by OBJ_WORK_E
MoveBounceByTable12DE:
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:MoveBounceByTable12DESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:MoveBounceByTable12DESpeeds,y
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    lda #$0A
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
    bne MoveBounceByTable12DE_at0
    rts
MoveBounceByTable12DE_at0:
    jsr VecNegateVelocityX
    jmp VecApplyVelocityX

; @routine 12DE:4D6 ListFallThenCrash12DE
; @run 71:CCD6
; WHAT IT DOES: Runs the list and $CAF4; carry from it: two sounds, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListFallThenCrash12DE:
    lda #$11
    sta z:zTemp10
    lda #$CD
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr FallToFloor93_12DE
    bcs ListFallThenCrash12DE_at0
    lda a:OBJ_VY_LO,x
    sec
    sbc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    rts
ListFallThenCrash12DE_at0:
    lda #$4A
    jsr VecSoundCommand
    lda #$7D
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$01
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 13FB:74E StrikeSoundsByStep13FB
; @run 70:B74E
; WHAT IT DOES: Runs the list at $B787, the sound for OBJ_WORK_I at step 10; at its end state 2.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StrikeSoundsByStep13FBSounds = $B7AC ; sound by OBJ_WORK_I, from 1
StrikeSoundsByStep13FB:
    lda #$87
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StrikeSoundsByStep13FB_at0
    bcc StrikeSoundsByStep13FB_at1
    lda a:OBJ_ANIM_STEP,x
    cmp #$0A
    bne StrikeSoundsByStep13FB_at2
    ldy a:OBJ_WORK_I,x
    dey
    lda a:StrikeSoundsByStep13FBSounds,y
    jsr VecSoundCommand
StrikeSoundsByStep13FB_at2:
    lda a:OBJ_ANIM_STEP,x
    cmp #$0B
    bne StrikeSoundsByStep13FB_at1
    stz a:OBJ_WORK_I,x
    lda #$01
    sta a:OBJ_WORK_L,x
    rts
StrikeSoundsByStep13FB_at0:
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StrikeSoundsByStep13FB_at1:
    rts

; @routine 161C:0F6 ThrowThenWait161C
; @run 70:B8F6
; WHAT IT DOES: Animates, throwing kind $36 at frame $21; at the list's end (or off the first screen) state 3 with the next wait.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ThrowThenWait161CWaits = $B930 ; four waits
ThrowThenWait161C:
    lda a:OBJ_Y_HI,x
    bne ThrowThenWait161C_at0
    ldy #$02
    jsr FaceAndAnimate161C
    beq ThrowThenWait161C_at0
    bcc ThrowThenWait161C_at1
    lda a:OBJ_FRAME,x
    cmp #$21
    bne ThrowThenWait161C_at1
    jsr VecFindFreeSlotFrom30
    bcc ThrowThenWait161C_at1
    lda #$36
    jmp VecStartObjectAtPlace
ThrowThenWait161C_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_F,x
    and #$03
    tay
    lda a:ThrowThenWait161CWaits,y
    sta a:OBJ_WORK_D,x
ThrowThenWait161C_at1:
    rts

; @routine 161C:283 TossThenBecome9B161C
; @run 70:BA83
; WHAT IT DOES: Loads moving away from or toward the player; falls animated for OBJ_WORK_E counts and then becomes kind $9B.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TossThenBecome9B161CLists = $BABC ; frame lists by OBJ_WORK_D
TossThenBecome9B161C:
    lda a:OBJ_STATE,x
    bne TossThenBecome9B161C_at0
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    beq TossThenBecome9B161C_at1
    jsr VecNegateVelocityX
TossThenBecome9B161C_at1:
    rts
TossThenBecome9B161C_at0:
    dec a:OBJ_WORK_E,x
    bne TossThenBecome9B161C_at2
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
TossThenBecome9B161C_at2:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:TossThenBecome9B161CLists,y
    sta z:zTemp10
    lda a:TossThenBecome9B161CLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 161D:3F4 FallDroppingKind9C161D
; @run 71:C3F4
; WHAT IT DOES: Falls, cleared below the first screen; every eighth count of OBJ_WORK_E leaves a kind $9C object.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallDroppingKind9C161D:
    lda #$2D
    sta z:zTemp10
    lda #$C4
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_VY_LO,x
    clc
    adc #$14
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    beq FallDroppingKind9C161D_at0
    jmp VecClearObject
FallDroppingKind9C161D_at0:
    lda a:OBJ_WORK_E,x
    and #$07
    bne FallDroppingKind9C161D_at1
    jsr VecFindFreeSlotFrom30
    bcc FallDroppingKind9C161D_at1
    lda #$9C
    jsr VecStartObjectAtPlace
FallDroppingKind9C161D_at1:
    rts

; @routine 16CB:7C3 FallThenGone16CB
; @run 70:B7C3
; WHAT IT DOES: Falls animated (sound $8E at step 3); below the first screen the raster effect off and cleared.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallThenGone16CB:
    lda #$FD
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc FallThenGone16CB_at0
    lda a:OBJ_ANIM_STEP,x
    cmp #$03
    bne FallThenGone16CB_at0
    lda #$8E
    jsr VecSoundCommand
FallThenGone16CB_at0:
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    clc
    adc #$09
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_HI,x
    beq FallThenGone16CB_at1
    stz z:zRasterArg
    stz z:zRasterEffect
    jmp VecClearObject
FallThenGone16CB_at1:
    rts

; @routine 18DB:53C SwingThenScript18DB
; @run 70:B53C
; WHAT IT DOES: Bobs by the cosine of OBJ_WORK_G; after OBJ_WORK_E counts script $83, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwingThenScript18DB:
    lda #$E0
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_WORK_G,x
    jsr VecScaleByCosine
    lda z:zProduct1
    sta a:OBJ_VY_LO,x
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_G,x
    clc
    adc #$04
    sta a:OBJ_WORK_G,x
    dec a:OBJ_WORK_E,x
    bne SwingThenScript18DB_at0
    phx
    lda #$83
    ldx #$04
    jsr VecStartScriptSlotKind1
    plx
    lda #$80
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
SwingThenScript18DB_at0:
    rts

; @routine 18DB:575 SwingThenChannel18DB
; @run 70:B575
; WHAT IT DOES: Bobs by the cosine of OBJ_WORK_G; after OBJ_WORK_E counts list channel $82, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwingThenChannel18DB:
    lda #$E0
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_WORK_G,x
    jsr VecScaleByCosine
    lda z:zProduct1
    sta a:OBJ_VY_LO,x
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_G,x
    clc
    adc #$04
    sta a:OBJ_WORK_G,x
    dec a:OBJ_WORK_E,x
    bne SwingThenChannel18DB_at0
    phx
    lda #$82
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    stz a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SwingThenChannel18DB_at0:
    rts

; @routine 18DC:100 WalkInsideScreen18DC
; @run 70:B900
; WHAT IT DOES: Between X $40 and $C0 on the first screen walks; outside it the substate counts on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkInsideScreen18DCSpeeds = $B6FA ; vx by facing
WalkInsideScreen18DC:
    lda a:OBJ_X_HI,x
    bne WalkInsideScreen18DC_at0
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcs WalkInsideScreen18DC_at0
    cmp #$40
    bcc WalkInsideScreen18DC_at0
    lda #$E9
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:WalkInsideScreen18DCSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:WalkInsideScreen18DCSpeeds+1,y
    sta a:OBJ_VX_HI,x
    jmp VecApplyVelocityX
WalkInsideScreen18DC_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 18DD:173 MarkInPool18DD
; @run 71:C173
; WHAT IT DOES: Marks (bit 7) one unmarked entry of the 112 at $3F90; with none left script $85, a wait and the state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
MarkInPool18DDPool = $3F90 ; 112 entries
MarkInPool18DD:
    lda #$10
    sta z:zTemp00
MarkInPool18DD_at0:
    jsr VecNextRandom
    and #$7F
    cmp #$70
    bcs MarkInPool18DD_at0
    tay
    lda a:MarkInPool18DDPool,y
    bpl MarkInPool18DD_at1
    dec z:zTemp00
    bne MarkInPool18DD_at0
    cly
MarkInPool18DD_at2:
    lda a:MarkInPool18DDPool,y
    bpl MarkInPool18DD_at1
    iny
    cpy #$70
    bne MarkInPool18DD_at2
    phx
    ldx #$0B
    lda #$85
    jsr VecStartScriptSlotKind1
    plx
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
    rts
MarkInPool18DD_at1:
    ora #$80
    sta a:MarkInPool18DDPool,y
    rts
