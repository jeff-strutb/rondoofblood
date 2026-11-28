; @routine 0B4C:273 SpitThenLeapByFacing0B4C
; @run 70:BA73
; WHAT IT DOES: Spits ($BABD) playing the facing's list while OBJ_WORK_E counts, then leaps (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpitThenLeapByFacing0B4CLists = $BAA5 ; a frame list per facing
SpitThenLeapByFacing0B4CSpeeds = $BABB ; X speed high byte by facing
SpitThenLeapByFacing0B4C:
    jsr SpawnWithRandom0B4C
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:SpitThenLeapByFacing0B4CLists,y
    sta z:zTemp10
    lda a:SpitThenLeapByFacing0B4CLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_E,x
    bne SpitThenLeapByFacing0B4C_at0
    ldy a:OBJ_FACING_LEFT,x
    lda a:SpitThenLeapByFacing0B4CSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$F9
    sta a:OBJ_VY_HI,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
SpitThenLeapByFacing0B4C_at0:
    rts

; @routine 0B4C:522 ListSoundThenTurnLeap0B4C
; @run 70:BD22
; WHAT IT DOES: Plays the list at $BD57 with sound $19 at frame $3E; then faces the player and starts walking (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListSoundThenTurnLeap0B4CSpeeds = $BDB7 ; X speed high byte by facing
ListSoundThenTurnLeap0B4C:
    jsr FallToGroundAt98
    lda #$BD
    sta z:zTemp11
    lda #$57
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq ListSoundThenTurnLeap0B4C_at0
    bcc ListSoundThenTurnLeap0B4C_at1
    lda a:OBJ_FRAME,x
    cmp #$3E
    bne ListSoundThenTurnLeap0B4C_at1
    lda #$19
    jsr VecSoundCommand
    rts
ListSoundThenTurnLeap0B4C_at0:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    tay
    lda a:ListSoundThenTurnLeap0B4CSpeeds,y
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    inc a:OBJ_SUBSTATE,x
ListSoundThenTurnLeap0B4C_at1:
    rts

; @routine 0B4C:5E5 FallListThenRest0B4C
; @run 70:BDE5
; WHAT IT DOES: Every fourth count falls playing the list at $BE1A; once falling again, stops with no hit box (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallListThenRest0B4C:
    lda a:OBJ_WORK_H,x
    and #$03
    bne FallListThenRest0B4C_at0
    lda #$BE
    sta z:zTemp11
    lda #$1A
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocityY
    lda a:OBJ_VY_HI,x
    bmi FallListThenRest0B4C_at0
    lda #$40
    sta a:OBJ_WORK_D,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_HITBOX_ON,x
    inc a:OBJ_SUBSTATE,x
FallListThenRest0B4C_at0:
    rts

; @routine 0EC0:226 LoadHopThenWalk0EC0
; @run 71:DA26
; WHAT IT DOES: Appears hopping with sound $25 and its facing's speed; later walks playing the list at $DA5F (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadHopThenWalk0EC0Speeds = $DA5B ; X speed words by facing
LoadHopThenWalk0EC0:
    lda a:OBJ_STATE,x
    bne LoadHopThenWalk0EC0_at0
    lda #$25
    jsr VecSoundCommand
    jsr VecLoadObjectRecord
    lda #$F5
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:LoadHopThenWalk0EC0Speeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LoadHopThenWalk0EC0Speeds,y
    sta a:OBJ_VX_LO,x
    rts
LoadHopThenWalk0EC0_at0:
    lda #$5F
    sta z:zTemp10
    lda #$DA
    sta z:zTemp11
    jsr VecStepObjectFrames
    jmp VecApplyVelocityX

; @routine 101C:23E LoadSoundThenListClear101C
; @run 70:BA3E
; WHAT IT DOES: Appears with sound $13 rising; plays the list at $BA70 and is cleared when it ends (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadSoundThenListClear101C:
    lda a:OBJ_STATE,x
    bne LoadSoundThenListClear101C_at0
    jsr VecLoadObjectRecord
    lda #$13
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    rts
LoadSoundThenListClear101C_at0:
    jsr VecApplyVelocityY
    lda #$70
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LoadSoundThenListClear101C_at1
    jmp VecClearObject
LoadSoundThenListClear101C_at1:
    rts

; @routine 117B:199 PaceBackAndForth117B
; @run 70:B199
; WHAT IT DOES: Moves along X counting the word OBJ_WORK_H-G; at zero turns (OBJ_WORK_F) and reloads the count by start argument (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PaceBackAndForth117BCounts = $B122 ; a count word per start argument
PaceBackAndForth117B:
    jsr VecApplyVelocityX
    lda a:OBJ_WORK_H,x
    sec
    sbc #$01
    sta a:OBJ_WORK_H,x
    lda a:OBJ_WORK_G,x
    sbc #$00
    sta a:OBJ_WORK_G,x
    ora a:OBJ_WORK_H,x
    bne PaceBackAndForth117B_at0
    lda a:OBJ_WORK_F,x
    eor #$01
    sta a:OBJ_WORK_F,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:PaceBackAndForth117BCounts,y
    sta a:OBJ_WORK_H,x
    lda a:PaceBackAndForth117BCounts+1,y
    sta a:OBJ_WORK_G,x
PaceBackAndForth117B_at0:
    rts

; @routine 117E:3E6 ListStrikeThenState4_117E
; @run 71:CBE6
; WHAT IT DOES: Plays the list at $CC1C, lunging at frame 7 with sound $1F; when it ends, state 4 (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListStrikeThenState4_117ELunges = $CC1A ; X speed high byte by facing
ListStrikeThenState4_117E:
    lda #$CC
    sta z:zTemp11
    lda #$1C
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq ListStrikeThenState4_117E_at0
    bcc ListStrikeThenState4_117E_at1
    lda a:OBJ_FRAME,x
    cmp #$07
    bne ListStrikeThenState4_117E_at1
    lda #$1F
    jsr VecSoundCommand
    ldy a:OBJ_FACING_LEFT,x
    lda a:ListStrikeThenState4_117ELunges,y
    sta a:OBJ_VX_HI,x
    jsr ProbeHeightCall117E_2E7
ListStrikeThenState4_117E_at1:
    rts
ListStrikeThenState4_117E_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    rts

; @routine 12DA:740 ArenaStartOnScreen3_12DA
; @run 70:AF40
; WHAT IT DOES: From screen 3, holds the player until slot $50 reaches state 3; then tile script 6, the event recorded, state on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ArenaStartOnScreen3_12DA:
    lda a:ScrollXHiScreen
    cmp #$03
    bcc ArenaStartOnScreen3_12DA_at0
    lda #$03
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    lda #$10
    sta z:zTemp08
    lda #$26
    sta z:zTemp09
    jsr ContactWithPlayer
    lda a:OBJ_STATE+80
    cmp #$03
    bcc ArenaStartOnScreen3_12DA_at0
    lda #$06
    cly
    jsr VecDrawTileScript
    jsr VecRecordObjectEvent
    inc a:OBJ_STATE,x
ArenaStartOnScreen3_12DA_at0:
    rts

; @routine 12DC:4A8 WaveAccumulate12DC
; @run 70:BCA8
; WHAT IT DOES: Adds the cosine of OBJ_WORK_G scaled by $C0 into OBJ_WORK_E-F, the angle stepped; a full turn clears the sum (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WaveAccumulate12DC:
    lda #$C0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_G,x
    jsr VecScaleByCosine
    lda a:OBJ_WORK_F,x
    clc
    adc z:zProduct1
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    adc z:zProduct2
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_G,x
    clc
    adc #$04
    sta a:OBJ_WORK_G,x
    bcc WaveAccumulate12DC_at0
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_F,x
WaveAccumulate12DC_at0:
    rts

; @routine 12DD:3E4 LoadWithArmsAndScript12DD
; @run 71:C3E4
; WHAT IT DOES: Loads with kind $32 and $33 objects in slots $11 and $12, list channel $83 and script $8F (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadWithArmsAndScript12DD:
    jsr VecLoadObjectRecord
    lda #$A4
    sta a:OBJ_Y_LO,x
    ldy #$11
    jsr VecClearObjectInY
    lda #$32
    jsr VecStartObjectAtPlace
    iny
    jsr VecClearObjectInY
    lda #$33
    jsr VecStartObjectAtPlace
    phx
    lda #$83
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$8F
    ldx #$04
    jsr VecStartScriptSlotKind1
    plx
    lda #$30
    sta a:OBJ_WORK_F,x
    rts

; @routine 12DE:0AD LoadByArg12DE
; @run 71:C8AD
; WHAT IT DOES: Loads 16 dots up with angle and speed from tables by the start argument (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadByArg12DEAngles = $C8E0 ; by start argument
LoadByArg12DETurns = $C8F0 ; added by OBJ_WORK_I
LoadByArg12DESpeeds = $C8F2 ; speed words by start argument
LoadByArg12DECounts = $C912 ; OBJ_WORK_E by start argument
LoadByArg12DE:
    jsr VecLoadObjectRecord
    lda #$F0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    ldy a:OBJ_START_ARG,x
    lda a:LoadByArg12DECounts,y
    sta a:OBJ_WORK_E,x
    lda a:LoadByArg12DEAngles,y
    ldy a:OBJ_WORK_I,x
    clc
    adc a:LoadByArg12DETurns,y
    sta a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:LoadByArg12DESpeeds+1,y
    sta a:OBJ_WORK_F,x
    lda a:LoadByArg12DESpeeds,y
    sta a:OBJ_WORK_G,x
    rts

; @routine 12DE:127 TurnQuarter12DE
; @run 71:C927
; WHAT IT DOES: Moves at angle OBJ_WORK_D turning toward the half-turn OBJ_WORK_I names; there, the state on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnQuarter12DE:
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_I,x
    eor #$01
    ror a
    ror a
    and #$80
    pha
    sta z:zTemp02
    lda #$01
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    pla
    cmp a:OBJ_WORK_D,x
    bne TurnQuarter12DE_at0
    inc a:OBJ_STATE,x
TurnQuarter12DE_at0:
    rts

; @routine 13FB:4A7 FireTwiceAtStep13_13FB
; @run 70:B4A7
; WHAT IT DOES: At animation step $13: at timer $30 a kind $40 object in slot $12, at $10 a kind $3F in slot $11, each with sound $46 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FireTwiceAtStep13_13FB:
    lda a:OBJ_ANIM_STEP,x
    cmp #$13
    bne FireTwiceAtStep13_13FB_at0
    lda a:OBJ_ANIM_TIMER,x
    cmp #$10
    beq FireTwiceAtStep13_13FB_at1
    cmp #$30
    bne FireTwiceAtStep13_13FB_at0
    lda #$46
    jsr VecSoundCommand
    ldy #$12
    jsr VecClearObjectInY
    lda #$40
    jsr VecStartObjectAtPlace
FireTwiceAtStep13_13FB_at0:
    rts
FireTwiceAtStep13_13FB_at1:
    lda #$46
    jsr VecSoundCommand
    ldy #$11
    jsr VecClearObjectInY
    lda #$3F
    jsr VecStartObjectAtPlace
    rts

; @routine 161C:5B9 StartRowByWorkK161C
; @run 70:BDB9
; WHAT IT DOES: Starts kind $51 objects numbered over the range OBJ_WORK_K names, then clears itself (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartRowByWorkK161CRanges = $BDEE ; first and past-last number by OBJ_WORK_K
StartRowByWorkK161C:
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda a:OBJ_WORK_K,x
    asl a
    tay
    lda a:StartRowByWorkK161CRanges,y
    sta z:zTemp00
    lda a:StartRowByWorkK161CRanges+1,y
    sta z:zTemp01
StartRowByWorkK161C_at1:
    jsr VecFindFreeSlotFrom30
    bcc StartRowByWorkK161C_at0
    lda #$51
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    inc z:zTemp00
    lda z:zTemp00
    cmp z:zTemp01
    bcc StartRowByWorkK161C_at1
StartRowByWorkK161C_at0:
    jmp VecClearObject

; @routine 161D:2A3 ListThenThreeShots161D
; @run 71:C2A3
; WHAT IT DOES: Plays the list at $C238; then kind $4C shots in slots $12 to $14 with sound $2E each, state 2 (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenThreeShots161D:
    lda #$38
    sta z:zTemp10
    lda #$C2
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenThreeShots161D_at0
    ldy #$12
ListThenThreeShots161D_at1:
    jsr VecClearObjectInY
    lda #$4C
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    phy
    lda #$2E
    jsr VecSoundCommand
    ply
    iny
    cpy #$15
    bcc ListThenThreeShots161D_at1
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListThenThreeShots161D_at0:
    rts

; @routine 161E:035 ListThenBurstSix161E
; @run 71:C835
; WHAT IT DOES: Plays the list at $C86A; then sound $88, six numbered kind $4F objects, and cleared (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenBurstSix161E:
    lda #$6A
    sta z:zTemp10
    lda #$C8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenBurstSix161E_at0
    lda #$88
    jsr VecSoundCommand
    stz z:zTemp00
ListThenBurstSix161E_at2:
    jsr VecFindFreeSlotFrom30
    bcc ListThenBurstSix161E_at1
    lda #$4F
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    inc z:zTemp00
    lda z:zTemp00
    cmp #$06
    bcc ListThenBurstSix161E_at2
ListThenBurstSix161E_at1:
    jmp VecClearObject
ListThenBurstSix161E_at0:
    rts

; @routine 16CB:1C2 SpitDebrisEvery8_16CB
; @run 70:B1C2
; WHAT IT DOES: Every eighth count of OBJ_WORK_J throws a kind $9C object at random speeds with sound $13 (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpitDebrisEvery8_16CB:
    lda a:OBJ_WORK_J,x
    and #$07
    bne SpitDebrisEvery8_16CB_at0
    jsr VecFindFreeSlotFrom30
    bcc SpitDebrisEvery8_16CB_at0
    lda #$9C
    jsr VecStartObjectAtPlace
    sxy
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$20
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    sxy
    lda #$13
    jsr VecSoundCommand
SpitDebrisEvery8_16CB_at0:
    rts
