; @routine 09EC:768 RainKind9D_09EC
; @run 70:BF68
; WHAT IT DOES: Moves to a random X; every other count starts a kind $9D object (a quarter of them in the other slots).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RainKind9D_09EC:
    inc a:OBJ_WORK_D,x
    lda a:OBJ_STATE,x
    bne RainKind9D_09EC_at0
    jsr VecLoadObjectRecord
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$C0
    sta a:OBJ_Y_LO,x
    rts
RainKind9D_09EC_at0:
    jsr VecNextRandom
    sta a:OBJ_X_LO,x
    lda a:OBJ_WORK_D,x
    and #$01
    bne RainKind9D_09EC_at1
    jsr VecNextRandom
    and #$03
    beq RainKind9D_09EC_at2
    jsr VecFindFreeSlotFrom30
    bcc RainKind9D_09EC_at1
    lda #$9D
    jmp VecStartObjectAtPlace
RainKind9D_09EC_at2:
    jsr VecFindFreeSlotFrom50
    bcc RainKind9D_09EC_at1
    lda #$9D
    jmp VecStartObjectAtPlace
RainKind9D_09EC_at1:
    rts

; @routine 0B4C:1F6 ChargeThenBrake0B4C
; @run 70:B9F6
; WHAT IT DOES: Runs the list at $BA3C moving two steps a frame; at its end sound $B6, a slower speed, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChargeThenBrake0B4CRuns = $BA38 ; charge speed by facing
ChargeThenBrake0B4CBrakes = $BA3A ; speed after by facing
ChargeThenBrake0B4C:
    lda #$BA
    sta z:zTemp11
    lda #$3C
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq ChargeThenBrake0B4C_at0
    bcc ChargeThenBrake0B4C_at1
    ldy a:OBJ_FACING_LEFT,x
    lda a:ChargeThenBrake0B4CRuns,y
    sta a:OBJ_VX_HI,x
    jsr MoveXAndProbeWallBounce
    jmp MoveXAndProbeWallBounce
ChargeThenBrake0B4C_at0:
    lda #$B6
    jsr VecSoundCommand
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_F,x
    tay
    lda a:ChargeThenBrake0B4CBrakes,y
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$19
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
ChargeThenBrake0B4C_at1:
    rts

; @routine 0B4C:70C FlyFromSlot10_0B4C
; @run 70:BF0C
; WHAT IT DOES: Held a step ahead of slot $10 by the facing, its box sized by its frame.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FlyFromSlot10_0B4CSpeeds = $BF4A ; vx by facing
FlyFromSlot10_0B4CBoxes = $BF51 ; box size by frame
FlyFromSlot10_0B4C:
    lda a:OBJ_STATE,x
    bne FlyFromSlot10_0B4C_at0
    jmp VecLoadObjectRecord
FlyFromSlot10_0B4C_at0:
    lda #$BF
    sta z:zTemp11
    lda #$4C
    sta z:zTemp10
    jsr VecStepObjectFrames
    ldy #$10
    jsr VecCopyObjectPlaceBack
    ldy a:OBJ_FACING_LEFT,x
    lda a:FlyFromSlot10_0B4CSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$08
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_FRAME,x
    sec
    sbc #$01
    asl a
    tay
    lda a:FlyFromSlot10_0B4CBoxes,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:FlyFromSlot10_0B4CBoxes+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0EBA:7D5 ExitByDoor0EBA
; @run 70:AFD5
; WHAT IT DOES: Waits for the player within $60, fades the CD music; 128 counts on asks the mode for what comes next (zFieldA3 9).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ExitByDoor0EBAFlags = $26EF ; bits cleared for the next screen
ExitByDoor0EBA:
    lda a:OBJ_STATE,x
    bne ExitByDoor0EBA_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$03
    sta a:OBJ_FRAME,x
ExitByDoor0EBA_at2:
    rts
ExitByDoor0EBA_at0:
    dec a
    bne ExitByDoor0EBA_at1
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs ExitByDoor0EBA_at2
    phx
    lda #$0C
    jsr BiosCdFade
    plx
    inc a:OBJ_STATE,x
    rts
ExitByDoor0EBA_at1:
    dec a:OBJ_WORK_D,x
    dec a:OBJ_WORK_D,x
    bne ExitByDoor0EBA_at2
    lda a:ExitByDoor0EBAFlags
    and #$FC
    sta a:ExitByDoor0EBAFlags
    smb1 z:zModeEvents
    lda #$09
    sta z:zFieldA3
    rts

; @routine 101B:1A8 BoxBesidePrevious101B
; @run 70:B1A8
; WHAT IT DOES: In the previous slot's place; a box of $0C while that slot shows frames $06-$15.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BoxBesidePrevious101BOffsets = $B1E8 ; box offset by the previous frame
BoxBesidePrevious101B:
    lda a:OBJ_STATE,x
    bne BoxBesidePrevious101B_at0
    jmp VecLoadObjectRecord
BoxBesidePrevious101B_at0:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FACING_LEFT-1,x
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_FRAME-1,x
    cmp #$06
    bcc BoxBesidePrevious101B_at1
    cmp #$16
    bcs BoxBesidePrevious101B_at1
    sec
    sbc #$06
    asl a
    tay
    lda a:BoxBesidePrevious101BOffsets,y
    sta a:OBJ_HITBOX_DX,x
    lda a:BoxBesidePrevious101BOffsets+1,y
    sta a:OBJ_HITBOX_DY,x
    lda #$0C
    sta a:OBJ_HITBOX_HALF_W,x
    sta a:OBJ_HITBOX_HALF_H,x
    rts
BoxBesidePrevious101B_at1:
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 101D:025 LoadSegmentByArg101D
; @run 71:C025
; WHAT IT DOES: With the player on one side only: loads with its box and delay by the start argument.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadSegmentByArg101DBoxes = $C065 ; hit box by start argument, four bytes
LoadSegmentByArg101D:
    jsr VecSideOfPlayer
    and #$01
    beq LoadSegmentByArg101D_at0
    jmp VecClearObject
LoadSegmentByArg101D_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_K,x
    lda #$A0
    sta a:OBJ_WORK_I,x
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    tay
    lda a:LoadSegmentByArg101DBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:LoadSegmentByArg101DBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:LoadSegmentByArg101DBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:LoadSegmentByArg101DBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$03
    sta a:OBJ_WORK_A+$11
    rts

; @routine 101E:0B2 StartLeaderAndTrail101E
; @run 71:C8B2
; WHAT IT DOES: Loads, fills the 128-byte trail with $40 and starts fifteen numbered kind $C2 pieces in the next slots.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartLeaderAndTrail101ETrail = $C7FC ; the leader's angles
StartLeaderAndTrail101E:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    beq StartLeaderAndTrail101E_at0
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
StartLeaderAndTrail101E_at0:
    lda #$48
    sta a:OBJ_WORK_K,x
    lda #$40
    sta a:OBJ_WORK_D,x
    cly
StartLeaderAndTrail101E_at1:
    sta a:StartLeaderAndTrail101ETrail,y
    iny
    cpy #$80
    bcc StartLeaderAndTrail101E_at1
    txa
    tay
    iny
    lda #$0F
    sta z:zTemp00
    stz z:zTemp01
StartLeaderAndTrail101E_at2:
    jsr VecClearObjectInY
    lda #$C2
    jsr VecStartObjectAtPlace
    lda z:zTemp01
    sta a:OBJ_WORK_D,y
    iny
    inc z:zTemp01
    dec z:zTemp00
    bne StartLeaderAndTrail101E_at2
    rts

; @routine 10CC:0C9 WrapToTop10CC
; @run 70:B8C9
; WHAT IT DOES: Once above the screen before, comes back at the top at one of two places, state 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WrapToTop10CCPlaces = $B90B ; two X places
WrapToTop10CC:
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    cmp #$FE
    bne WrapToTop10CC_at0
    lda #$80
    sta a:OBJ_SCROLL_BAND,x
    stz z:zRasterStage
    stz a:OBJ_X_HI,x
    lda a:ScrollXHiScreen
    lsr a
    lsr a
    and #$01
    tay
    lda a:WrapToTop10CCPlaces,y
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$C0
    sta a:OBJ_Y_LO,x
    lda #$98
    sta a:OBJ_FRAME,x
    lda #$05
    sta a:OBJ_STATE,x
    stz z:zRasterStage
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_SUBSTATE,x
WrapToTop10CC_at0:
    rts

; @routine 10CC:1C0 LandStartShield10CC
; @run 70:B9C0
; WHAT IT DOES: On landing: two kind $2D objects in slots $11-$12, hit-tested and armed, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandStartShield10CC:
    jsr LandBelow90OrFall10CC
    bcc LandStartShield10CC_at0
    ldy #$11
    jsr VecClearObjectInY
    lda #$2D
    jsr VecStartObjectAtPlace
    iny
    jsr VecClearObjectInY
    lda #$2D
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_START_ARG,y
    lda #$03
    sta a:OBJ_WORK_A,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$02
    sta a:OBJ_WORK_C,x
    lda #$16
    sta a:OBJ_WORK_B,x
    lda #$87
    sta a:OBJ_PACE,x
    stz a:OBJ_WORK_G,x
    lda #$80
    sta a:OBJ_WORK_F,x
    inc a:OBJ_SUBSTATE,x
LandStartShield10CC_at0:
    rts

; @routine 117F:3D8 HoldThenDrop117F
; @run 71:D3D8
; WHAT IT DOES: Holds the player (forced pad) for OBJ_WORK_D counts, then a list channel and a kind $34 object off the left.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
HoldThenDrop117F:
    lda #$10
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    stz a:HitAttackFlags
    dec a:OBJ_WORK_D,x
    bne HoldThenDrop117F_at0
    phx
    lda #$83
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    txa
    clc
    adc #$03
    tay
    jsr VecClearObjectInY
    lda #$34
    jsr VecStartObjectAtPlace
    lda #$FF
    sta a:OBJ_X_HI,y
    lda #$E0
    sta a:OBJ_X_LO,y
    lda #$01
    sta a:OBJ_START_ARG,y
    inc a:OBJ_SUBSTATE,x
HoldThenDrop117F_at0:
    rts

; @routine 117F:600 ListThrow117F
; @run 71:D600
; WHAT IT DOES: Throws kind $36 at frame $25, OBJ_WORK_K more times, then goes to $D65D.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThrow117F:
    lda #$42
    sta z:zTemp10
    lda #$D6
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListThrow117F_at0
    bcc ListThrow117F_at1
    lda a:OBJ_FRAME,x
    cmp #$25
    bne ListThrow117F_at1
    lda #$1B
    jsr VecSoundCommand
    lda #$E1
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom30
    bcc ListThrow117F_at1
    lda #$36
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
ListThrow117F_at1:
    rts
ListThrow117F_at0:
    lda a:OBJ_WORK_K,x
    bne ListThrow117F_at2
    jmp NextWaitToState3_117F
ListThrow117F_at2:
    dec a:OBJ_WORK_K,x
    lda #$06
    sta a:OBJ_ANIM_STEP,x
    rts

; @routine 12DB:643 SinkToB0_12DB
; @run 70:B643
; WHAT IT DOES: Sinks faster each count; at Y $B0 stops, a wait of $20, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SinkToB0_12DB:
    lda #$82
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    jsr RasterWhenLow12DB
    jsr DripWhenLow12DB
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_HI,x
    bne SinkToB0_12DB_at0
    lda a:OBJ_Y_LO,x
    cmp #$B0
    bcc SinkToB0_12DB_at0
    lda #$B0
    sta a:OBJ_Y_LO,x
    lda #$20
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
SinkToB0_12DB_at0:
    rts

; @routine 12DE:743 FallingBit12DE
; @run 71:CF43
; WHAT IT DOES: Starts above the screen with one of two frames and falls until below the first screen.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallingBit12DE:
    lda a:OBJ_STATE,x
    bne FallingBit12DE_at0
    jsr VecLoadObjectRecord
    stz a:OBJ_X_HI,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$F0
    sta a:OBJ_Y_LO,x
    jsr VecNextRandom
    and #$01
    clc
    adc #$8D
    sta a:OBJ_FRAME,x
    rts
FallingBit12DE_at0:
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne FallingBit12DE_at1
    jmp VecClearObject
FallingBit12DE_at1:
    rts

; @routine 161B:255 BobUntilRandom161B
; @run 70:B255
; WHAT IT DOES: Bobs by the cosine of OBJ_WORK_D until a 1-in-256 chance: then a wait of 8-39, state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BobUntilRandom161B:
    lda #$98
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:zProduct1
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    clc
    adc #$02
    sta a:OBJ_WORK_D,x
    jsr VecNextRandom
    bne BobUntilRandom161B_at0
    jsr VecNextRandom
    and #$1F
    clc
    adc #$08
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
BobUntilRandom161B_at0:
    rts

; @routine 161C:34B LoadWithPartner161C
; @run 70:BB4B
; WHAT IT DOES: Loads with a kind $48 partner, health by the start argument; an odd one starts in state 4.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadWithPartner161CHealth = $BB8B ; health by argument
LoadWithPartner161C:
    jsr VecLoadObjectRecord
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$48
    jsr VecStartObjectAtPlace
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$50
    sta a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    lda a:LoadWithPartner161CHealth,y
    sta a:OBJ_HEALTH,x
    lda a:OBJ_START_ARG,x
    and #$01
    beq LoadWithPartner161C_at0
    lda #$01
    sta a:OBJ_WORK_K,x
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$04
    sta a:OBJ_STATE,x
LoadWithPartner161C_at0:
    rts

; @routine 161C:732 BoxByPreviousFrame161C
; @run 70:BF32
; WHAT IT DOES: Hit-tested while the previous slot shows frames $10-$16, in its place and facing, with that frame's box offset.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BoxByPreviousFrame161COffsets = $BF6F ; box offset by the previous frame
BoxByPreviousFrame161C:
    lda a:OBJ_STATE,x
    bne BoxByPreviousFrame161C_at0
    jmp VecLoadObjectRecord
BoxByPreviousFrame161C_at0:
    lda a:OBJ_FRAME-1,x
    cmp #$10
    bcc BoxByPreviousFrame161C_at1
    cmp #$17
    bcs BoxByPreviousFrame161C_at1
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FACING_LEFT-1,x
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_FRAME-1,x
    sec
    sbc #$10
    asl a
    tay
    lda a:BoxByPreviousFrame161COffsets,y
    sta a:OBJ_HITBOX_DX,x
    lda a:BoxByPreviousFrame161COffsets+1,y
    sta a:OBJ_HITBOX_DY,x
    rts
BoxByPreviousFrame161C_at1:
    stz a:OBJ_HITBOX_ON,x
    rts

; @routine 161F:675 ExitByDoor161F
; @run 71:D675
; WHAT IT DOES: Waits for the player within $78, fades the CD music; 128 counts on asks the mode for what comes next (zFieldA3 8).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ExitByDoor161FFlags = $26EF ; bits cleared for the next screen
ExitByDoor161F:
    lda a:OBJ_STATE,x
    bne ExitByDoor161F_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$37
    sta a:OBJ_FRAME,x
ExitByDoor161F_at2:
    rts
ExitByDoor161F_at0:
    dec a
    bne ExitByDoor161F_at1
    jsr VecDistanceXToPlayer
    cmp #$78
    bcs ExitByDoor161F_at2
    phx
    lda #$0C
    jsr BiosCdFade
    plx
    inc a:OBJ_STATE,x
    rts
ExitByDoor161F_at1:
    dec a:OBJ_WORK_D,x
    dec a:OBJ_WORK_D,x
    bne ExitByDoor161F_at2
    lda a:ExitByDoor161FFlags
    and #$F8
    sta a:ExitByDoor161FFlags
    smb1 z:zModeEvents
    lda #$08
    sta z:zFieldA3
    rts

; @routine 18DC:039 ChooseByEdge18DC
; @run 70:B839
; WHAT IT DOES: Faces the player; short of the edge picks the next substate, else state 6 with OBJ_WORK_F one less.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChooseByEdge18DCCycle = $B87A ; eight substates
ChooseByEdge18DC:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_X_HI,x
    bne ChooseByEdge18DC_at0
    lda a:OBJ_FACING_LEFT,x
    bne ChooseByEdge18DC_at1
    lda a:OBJ_X_LO,x
    cmp #$30
    bcs ChooseByEdge18DC_at0
    bra ChooseByEdge18DC_at2
ChooseByEdge18DC_at1:
    lda a:OBJ_X_LO,x
    cmp #$D0
    bcs ChooseByEdge18DC_at0
ChooseByEdge18DC_at2:
    stz a:OBJ_WORK_I,x
    lda a:OBJ_WORK_H,x
    and #$07
    tay
    lda a:ChooseByEdge18DCCycle,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_H,x
    rts
ChooseByEdge18DC_at0:
    lda #$06
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    dec a:OBJ_WORK_F,x
    rts

; @routine 18DC:373 DriftSpitting18DC
; @run 70:BB73
; WHAT IT DOES: Drifts, speeding by the facing, leaving a kind $2A object every fourth count of OBJ_WORK_D.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DriftSpitting18DCSteps = $BB6F ; vx step by facing
DriftSpitting18DC:
    lda #$B1
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:DriftSpitting18DCSteps,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:DriftSpitting18DCSteps+1,y
    sta a:OBJ_VX_HI,x
    lda a:OBJ_WORK_D,x
    and #$03
    bne DriftSpitting18DC_at0
    jsr VecFindFreeSlotFrom30
    bcc DriftSpitting18DC_at0
    lda #$2A
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
DriftSpitting18DC_at0:
    rts
