; @routine 093E:0EB LoadPaleEnemy093E
; @run 71:C8EB
; WHAT IT DOES: Loads the record with attribute $8D.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadPaleEnemy093E:
    jsr VecLoadObjectRecord
    lda #$8D
    sta a:OBJ_SPRITE_ATTR,x
    rts

; @routine 093E:0F4 WakeWhenNear093E
; @run 71:C8F4
; WHAT IT DOES: Argument 0: within $74 of the player, drops the attribute, sets loop flags $60 and a hit box, and goes on.
; HOW: DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WakeWhenNear093E:
    lda a:OBJ_START_ARG,x
    bne WakeWhenNear093E_at0
    jsr VecDistanceXToPlayer
    cmp #$74
    bcs WakeWhenNear093E_at0
    stz a:OBJ_SPRITE_ATTR,x
    lda a:OBJ_LOOP_FLAGS,x
    ora #$60
    sta a:OBJ_LOOP_FLAGS,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
WakeWhenNear093E_at0:
    rts

; @routine 093E:114 StepThenNext093E
; @run 71:C914
; WHAT IT DOES: Steps the list at $C92A; at its end keeps $20 in OBJ_WORK_J and goes on.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepThenNext093EList = $C92A ; a frame list
StepThenNext093E:
    lda #$C9
    sta z:zTemp11
    lda #$2A
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne StepThenNext093E_at0
    lda #$20
    sta a:OBJ_WORK_J,x
    inc a:OBJ_STATE,x
StepThenNext093E_at0:
    rts

; @routine 093E:139 ChooseByDistance093E
; @run 71:C939
; WHAT IT DOES: Restarts the animation and substate. Far from the player, state 6; near, a random state from two tables.
; HOW: DistanceXToPlayer, DistanceYToPlayer, NextRandom.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChooseByDistance093ERows = $C967 ; sixteen rows
ChooseByDistance093EStates = $C977 ; states by row
ChooseByDistance093E:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs ChooseByDistance093E_at0
    jsr VecDistanceYToPlayer
    cmp #$60
    bcc ChooseByDistance093E_at1
ChooseByDistance093E_at0:
    lda #$06
    sta a:OBJ_STATE,x
    rts
ChooseByDistance093E_at1:
    jsr VecNextRandom
    and #$0F
    tay
    lda a:ChooseByDistance093ERows,y
    tay
    lda a:ChooseByDistance093EStates,y
    sta a:OBJ_STATE,x
    rts

; @routine 093E:179 PickWaitThenMove093E
; @run 71:C979
; @stop C994
; WHAT IT DOES: Substate 0 takes a random wait and restarts the animation; substate 1 moves ($C998).
; HOW: NextRandom and a table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PickWaitThenMove093EWaits = $C994 ; four waits
PickWaitThenMove093E:
    lda a:OBJ_SUBSTATE,x
    bne StepWaiting093E
    jsr VecNextRandom
    and #$03
    tay
    lda a:PickWaitThenMove093EWaits,y
    sta a:OBJ_WORK_K,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093E:198 StepWaiting093E
; @run 71:C998
; WHAT IT DOES: Steps the list at $C9CE, then counts ($C9A3).
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepWaiting093EList = $C9CE ; a frame list
StepWaiting093E:
    lda #$C9
    sta z:zTemp11
    lda #$CE
    sta z:zTemp10
    jsr VecStepObjectFrames

; @routine 093E:1A3 CountToTurn093E
; @run 71:C9A3
; WHAT IT DOES: Counts OBJ_WORK_K to state 3, else OBJ_WORK_J to state 7 with a new random count.
; HOW: Two countdowns.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountToTurn093ECounts = $C9D5 ; four counts
CountToTurn093E:
    dec a:OBJ_WORK_K,x
    bne CountToTurn093E_at0
    lda #$03
    sta a:OBJ_STATE,x
    rts
CountToTurn093E_at0:
    dec a:OBJ_WORK_J,x
    bne CountToTurn093E_at1
    jsr VecNextRandom
    and #$03
    tay
    lda a:CountToTurn093ECounts,y
    sta a:OBJ_WORK_J,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$07
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
CountToTurn093E_at1:
    rts

; @routine 093E:1D9 AimNearPlayer093E
; @run 71:C9D9
; @stop CA09
; WHAT IT DOES: Substate 0 takes the angle to the player plus the row's offset, a wait, a turn of $0200 and counts the row on; substate 1 curves ($CA19).
; HOW: AngleToPlayer and tables.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
AimNearPlayer093EOffsets = $CA09 ; angle offsets
AimNearPlayer093EWaits = $CA11 ; waits
AimNearPlayer093E:
    lda a:OBJ_SUBSTATE,x
    bne CurveSlowing093E
    jsr VecAngleToPlayer
    sta a:OBJ_PIECE_PLACE,x
    lda a:OBJ_FALL_LO,x
    and #$07
    tay
    lda a:OBJ_PIECE_PLACE,x
    clc
    adc a:AimNearPlayer093EOffsets,y
    sta a:OBJ_PIECE_PLACE,x
    lda a:AimNearPlayer093EWaits,y
    sta a:OBJ_WORK_K,x
    lda #$02
    sta a:OBJ_SWOOP_SPEED_LO,x
    stz a:OBJ_SWOOP_SPEED_HI,x
    inc a:OBJ_FALL_LO,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093E:219 CurveSlowing093E
; @run 71:CA19
; WHAT IT DOES: Steps the list at $CA53 and moves at the angle, slowing the speed by 8; then counts ($C9A3).
; HOW: SetVelocityFromAngle with the speed word.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CurveSlowing093EList = $CA53 ; a frame list
CurveSlowing093E:
    lda #$CA
    sta z:zTemp11
    lda #$53
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_SWOOP_SPEED_HI,x
    sta z:zTemp00
    lda a:OBJ_SWOOP_SPEED_LO,x
    sta z:zTemp01
    lda a:OBJ_PIECE_PLACE,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_SWOOP_SPEED_LO,x
    ora a:OBJ_SWOOP_SPEED_HI,x
    beq CurveSlowing093E_at0
    lda a:OBJ_SWOOP_SPEED_HI,x
    sec
    sbc #$08
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    sbc #$00
    sta a:OBJ_SWOOP_SPEED_LO,x
CurveSlowing093E_at0:
    jmp CountToTurn093E

; @routine 093E:25A DriftToPlayer093E
; @run 71:CA5A
; @stop CAA7
; WHAT IT DOES: Substate 0 stops, then drifts toward the player on each axis $60 or more away, with a wait of $20; substate 1 moves ($CAAB).
; HOW: Speed tables by side.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DriftToPlayer093ESpeeds = $CAA7 ; speed words by side
DriftToPlayer093E:
    lda a:OBJ_SUBSTATE,x
    bne DriftUntilWait093E
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    jsr VecDistanceXToPlayer
    cmp #$60
    bcc DriftToPlayer093E_at0
    jsr VecSideOfPlayer
    and #$01
    asl a
    tay
    lda a:DriftToPlayer093ESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:DriftToPlayer093ESpeeds,y
    sta a:OBJ_VX_LO,x
DriftToPlayer093E_at0:
    jsr VecDistanceYToPlayer
    cmp #$60
    bcc DriftToPlayer093E_at1
    jsr VecSideOfPlayer
    and #$02
    tay
    lda a:DriftToPlayer093ESpeeds+1,y
    sta a:OBJ_VY_HI,x
    lda a:DriftToPlayer093ESpeeds,y
    sta a:OBJ_VY_LO,x
DriftToPlayer093E_at1:
    lda #$20
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093E:2AB DriftUntilWait093E
; @run 71:CAAB
; WHAT IT DOES: Steps the list at $CA53 and moves; the wait over, state 3.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DriftUntilWait093EList = $CA53 ; a frame list
DriftUntilWait093E:
    lda #$CA
    sta z:zTemp11
    lda #$53
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    dec a:OBJ_WORK_K,x
    bne DriftUntilWait093E_at0
    lda #$03
    sta a:OBJ_STATE,x
DriftUntilWait093E_at0:
    rts

; @routine 093E:2C4 DriftByHeight093E
; @run 71:CAC4
; WHAT IT DOES: Takes the Y speed for being above or below the player, a wait of $20 and the next substate.
; HOW: SideOfPlayer bit 1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DriftByHeight093ESpeeds = $CAA7 ; speed words by side
DriftByHeight093E:
    jsr VecSideOfPlayer
    and #$02
    tay
    lda a:DriftByHeight093ESpeeds+1,y
    sta a:OBJ_VY_HI,x
    lda a:DriftByHeight093ESpeeds,y
    sta a:OBJ_VY_LO,x
    lda #$20
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093E:2DF BySubstate093ECADF
; @run 71:CADF
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $CAF1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate093ECADFHandlers = $CAF1 ; a handler per substate
BySubstate093ECADF:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate093ECADFHandlers,y
    sta z:zTemp00
    lda a:BySubstate093ECADFHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093E:305 ChooseAttack093E
; @run 71:CB05
; WHAT IT DOES: One time in four substate 7; else level with the player substate 1, below it substate 4.
; HOW: NextRandom, DistanceYToPlayer, SideOfPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChooseAttack093E:
    jsr VecNextRandom
    and #$03
    bne ChooseAttack093E_at1
ChooseAttack093E_at0:
    lda #$07
    sta a:OBJ_SUBSTATE,x
    rts
ChooseAttack093E_at1:
    jsr VecDistanceYToPlayer
    cmp #$18
    bcs ChooseAttack093E_at2
    lda #$01
    sta a:OBJ_SUBSTATE,x
    rts
ChooseAttack093E_at2:
    jsr VecSideOfPlayer
    and #$02
    beq ChooseAttack093E_at0
    lda #$04
    sta a:OBJ_SUBSTATE,x
    rts

; @routine 093E:33C DashByFacing093E
; @run 71:CB3C
; WHAT IT DOES: The X speed for the facing and the next substate.
; HOW: A table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DashByFacing093ESpeeds = $CB51 ; VX words by facing
DashByFacing093E:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:DashByFacing093ESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:DashByFacing093ESpeeds,y
    sta a:OBJ_VX_LO,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093E:355 DashThenAim093E
; @run 71:CB55
; WHAT IT DOES: Moves while the list at $CB8C steps; at its end aims at the player at speed $0300, faces them and goes on with a wait of $38.
; HOW: AngleToPlayer and SetVelocityFromAngle.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DashThenAim093EList = $CB8C ; a frame list
DashThenAim093E:
    jsr VecApplyVelocityX
    lda #$CB
    sta z:zTemp11
    lda #$8C
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne DashThenAim093E_at0
    jsr VecAngleToPlayer
    pha
    lda #$03
    sta z:zTemp01
    lda #$00
    sta z:zTemp00
    pla
    jsr VecSetVelocityFromAngle
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$38
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
DashThenAim093E_at0:
    rts

; @routine 093E:3A5 FlyThenState4_093E
; @run 71:CBA5
; WHAT IT DOES: Steps the list at $CBC7 and moves; the wait over, state 4.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyThenState4_093EList = $CBC7 ; a frame list
FlyThenState4_093E:
    lda #$CB
    sta z:zTemp11
    lda #$C7
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    dec a:OBJ_WORK_K,x
    bne FlyThenState4_093E_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
FlyThenState4_093E_at0:
    rts

; @routine 093E:3D6 AimBesidePlayer093E
; @run 71:CBD6
; WHAT IT DOES: Aims at a point beside the player at speed $0140, with a wait of $30.
; HOW: AngleToPlayerOffset.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
AimBesidePlayer093EOffsets = $CC02 ; X offset words by facing
AimBesidePlayer093E:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:AimBesidePlayer093EOffsets,y
    sta z:zTemp08
    lda a:AimBesidePlayer093EOffsets+1,y
    sta z:zTemp09
    stz z:zTemp0A
    stz z:zTemp0B
    jsr VecAngleToPlayerOffset
    pha
    lda #$01
    sta z:zTemp01
    lda #$40
    sta z:zTemp00
    pla
    jsr VecSetVelocityFromAngle
    lda #$30
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093E:406 FlyAtPoint093E
; @run 71:CC06
; @stop CC3B
; WHAT IT DOES: Steps the list at $CC3D, moving and facing the player. The wait over, state 3 ($CC6B); below the player at $20-$2F down, next substate.
; HOW: DistanceYToPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyAtPoint093EList = $CC3D ; a frame list
FlyAtPoint093E:
    lda #$CC
    sta z:zTemp11
    lda #$3D
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    dec a:OBJ_WORK_K,x
    beq RestartState3_093E
    jsr VecSideOfPlayer
    and #$02
    beq FlyAtPoint093E_at0
    jsr VecDistanceYToPlayer
    and #$F0
    cmp #$20
    bne FlyAtPoint093E_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
FlyAtPoint093E_at0:
    rts

; @routine 093E:444 ThrowAtFrame6C_093E
; @run 71:CC44
; WHAT IT DOES: Steps the list at $CC7A; at its end, or with no free slot, state 3 ($CC6B). At frame $6C starts a kind $2F object.
; HOW: FindFreeSlotFrom50.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowAtFrame6C_093EList = $CC7A ; a frame list
ThrowAtFrame6C_093E:
    lda #$CC
    sta z:zTemp11
    lda #$7A
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq RestartState3_093E
    bcc RestartState3_093E_at0
    lda a:OBJ_FRAME,x
    cmp #$6C
    bne RestartState3_093E_at0
    jsr VecFindFreeSlotFrom50
    bcc RestartState3_093E
    lda #$2F
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    rts

; @routine 093E:46B RestartState3_093E
; @run 71:CC6B
; WHAT IT DOES: State 3 from its start.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RestartState3_093E:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
RestartState3_093E_at0:
    rts

; @routine 093E:499 AimAtPoint50_093E
; @run 71:CC99
; WHAT IT DOES: Aims at the point the facing picks (Y $50) at speed $0140 and goes on.
; HOW: AngleToPoint.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
AimAtPoint50_093EXs = $CCBD ; X by facing
AimAtPoint50_093E:
    ldy a:OBJ_FACING_LEFT,x
    lda a:AimAtPoint50_093EXs,y
    sta z:zTemp08
    stz z:zTemp09
    lda #$50
    sta z:zTemp0A
    stz z:zTemp0B
    jsr VecAngleToPoint
    pha
    lda #$01
    sta z:zTemp01
    lda #$40
    sta z:zTemp00
    pla
    jsr VecSetVelocityFromAngle
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093E:4BF FlyUntilEdge093E
; @run 71:CCBF
; WHAT IT DOES: Moves and steps the list at $CCE4; at its end or within $10 of a screen side, next substate.
; HOW: Compares of X low.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyUntilEdge093EList = $CCE4 ; a frame list
FlyUntilEdge093E:
    jsr VecApplyVelocity
    lda #$E4
    sta z:zTemp10
    lda #$CC
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq FlyUntilEdge093E_at0
    lda a:OBJ_X_LO,x
    cmp #$F0
    bcs FlyUntilEdge093E_at0
    cmp #$10
    bcs FlyUntilEdge093E_at1
FlyUntilEdge093E_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
FlyUntilEdge093E_at1:
    rts

; @routine 093E:4FD SpitAtFrame6C_093E
; @run 71:CCFD
; WHAT IT DOES: Steps the list at $CD25; at frame $6C starts the six flames ($CD44), at its end state 3.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SpitAtFrame6C_093EList = $CD25 ; a frame list
SpitAtFrame6C_093E:
    lda #$CD
    sta z:zTemp11
    lda #$25
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq SpitAtFrame6C_093E_at0
    bcc SpitAtFrame6C_093E_at1
    lda a:OBJ_FRAME,x
    cmp #$6C
    bne SpitAtFrame6C_093E_at1
    jmp StartSevenFlames093E
SpitAtFrame6C_093E_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SpitAtFrame6C_093E_at1:
    rts

; @routine 093E:544 StartSevenFlames093E
; @run 71:CD44
; WHAT IT DOES: Sound $25 and up to seven kind $36 flames in free slots, facing this way and numbered.
; HOW: FindFreeSlotFrom30 and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartSevenFlames093E:
    lda #$25
    jsr VecSoundCommand
    lda #$07
    sta z:zTemp00
    stz z:zTemp01
StartSevenFlames093E_at0:
    jsr VecFindFreeSlotFrom30
    bcc StartSevenFlames093E_at1
    lda #$36
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda z:zTemp01
    sta a:OBJ_PIECE_NEXT,y
    inc z:zTemp01
    dec z:zTemp00
    bne StartSevenFlames093E_at0
StartSevenFlames093E_at1:
    rts

; @routine 093E:56B BySubstate093ECD6B
; @run 71:CD6B
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $CD7D.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate093ECD6BHandlers = $CD7D ; a handler per substate
BySubstate093ECD6B:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate093ECD6BHandlers,y
    sta z:zTemp00
    lda a:BySubstate093ECD6BHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093E:583 LungeFromHolder093E
; @run 71:CD83
; WHAT IT DOES: Aims from the holder's hit box (OBJ_WORK_L) at speed $0400, a random half-angle off, with a wait of $14.
; HOW: AngleToPoint and SetVelocityFromAngle.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LungeFromHolder093EOffsets = $CDD5 ; two angle offsets
LungeFromHolder093E:
    ldy a:OBJ_WORK_L,x
    lda a:OBJ_HITBOX_DX,y
    sta z:zTemp00
    lda a:OBJ_FACING_LEFT,y
    beq LungeFromHolder093E_at0
    lda z:zTemp00
    eor #$FF
    inc a
    sta z:zTemp00
LungeFromHolder093E_at0:
    lda a:OBJ_X_LO,y
    clc
    adc z:zTemp00
    sta z:zTemp08
    lda a:OBJ_X_HI,y
    adc #$00
    sta z:zTemp09
    lda a:OBJ_Y_HI,y
    sta z:zTemp0B
    lda a:OBJ_Y_LO,y
    sta z:zTemp0A
    jsr VecAngleToPoint
    sta z:zTemp02
    lda #$04
    sta z:zTemp01
    lda #$00
    sta z:zTemp00
    jsr VecNextRandom
    and #$01
    tay
    lda z:zTemp02
    clc
    adc a:LungeFromHolder093EOffsets,y
    jsr VecSetVelocityFromAngle
    lda #$14
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093E:5D7 LungeAway093E
; @run 71:CDD7
; WHAT IT DOES: Steps the list at $CDF3 and moves; the wait over, reloads $14 and goes on.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LungeAway093EList = $CDF3 ; a frame list
LungeAway093E:
    lda #$CD
    sta z:zTemp11
    lda #$F3
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    dec a:OBJ_WORK_K,x
    bne LungeAway093E_at0
    lda #$14
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
LungeAway093E_at0:
    rts

; @routine 093E:602 CountThenState3_093E
; @run 71:CE02
; WHAT IT DOES: The wait over, state 3 from its start.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountThenState3_093E:
    dec a:OBJ_WORK_K,x
    bne CountThenState3_093E_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
CountThenState3_093E_at0:
    rts

; @routine 093E:613 DieWithSparks093E
; @run 71:CE13
; WHAT IT DOES: Substate 0: frame $43 and a count of $40. Then falls, cleared at the count's end, sparking at eight counts.
; HOW: SparkOfRow093E.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DieWithSparks093ETimes = $CE68 ; eight counts
DieWithSparks093E:
    lda a:OBJ_SUBSTATE,x
    bne DieWithSparks093E_at0
    lda #$43
    sta a:OBJ_FRAME,x
    lda #$40
    sta a:OBJ_RAIN_COUNT,x
    inc a:OBJ_SUBSTATE,x
    rts
DieWithSparks093E_at0:
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocityY
    dec a:OBJ_RAIN_COUNT,x
    bne DieWithSparks093E_at1
    jmp VecClearObject
DieWithSparks093E_at1:
    cly
DieWithSparks093E_at2:
    lda a:OBJ_RAIN_COUNT,x
    cmp a:DieWithSparks093ETimes,y
    beq SparkOfRow093E
    iny
    cpy #$08
    bcc DieWithSparks093E_at2
    rts

; @routine 093E:690 FlameBurst093E
; @run 71:CE90
; WHAT IT DOES: State 0 loads it with sound $26; later steps the list at $CED6 and takes the hit box the frame names, cleared at the end.
; HOW: A box table by frame from $70.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlameBurst093EList = $CED6 ; a frame list
FlameBurst093EBoxes = $CF81 ; four bytes by frame
FlameBurst093E:
    lda a:OBJ_STATE,x
    bne FlameBurst093E_at0
    jsr VecLoadObjectRecord
    lda #$26
    jsr VecSoundCommand
    rts
FlameBurst093E_at0:
    lda #$D6
    sta z:zTemp10
    lda #$CE
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne FlameBurst093E_at1
    jmp VecClearObject
FlameBurst093E_at1:
    lda a:OBJ_FRAME,x
    sec
    sbc #$70
    cmp #$05
    bcc FlameBurst093E_at2
    lda #$05
FlameBurst093E_at2:
    asl a
    asl a
    tay
    lda a:FlameBurst093EBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:FlameBurst093EBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:FlameBurst093EBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:FlameBurst093EBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 093E:6ED LaunchFlame093E
; @run 71:CEED
; @stop CF1D
; WHAT IT DOES: State 0: loads it, steps once at -$18 upward with the facing's X, then takes an angle from the facing and its number at $0140; later flies ($CF28).
; HOW: SetVelocityFromAngle.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LaunchFlame093ESpeeds = $CF1D ; VX high by facing
LaunchFlame093EAngles = $CF1F ; base angle by facing
LaunchFlame093ESpread = $CF21 ; angle by number
LaunchFlame093E:
    lda a:OBJ_STATE,x
    bne FlyFlame093E
    jsr VecLoadObjectRecord
    ldy a:OBJ_FACING_LEFT,x
    lda a:LaunchFlame093ESpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$E8
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:LaunchFlame093EAngles,y
    ldy a:OBJ_PIECE_NEXT,x
    clc
    adc a:LaunchFlame093ESpread,y
    pha
    lda #$40
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    pla
    jmp VecSetVelocityFromAngle

; @routine 093E:728 FlyFlame093E
; @run 71:CF28
; WHAT IT DOES: Steps the list at $CF41; on the first screen moves, else cleared.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyFlame093EList = $CF41 ; a frame list
FlyFlame093E:
    lda #$41
    sta z:zTemp10
    lda #$CF
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    beq FlyFlame093E_at0
    jmp VecClearObject
FlyFlame093E_at0:
    jmp VecApplyVelocity

; @routine 093E:799 HeadChecksThenBody093E
; @run 71:CF99
; WHAT IT DOES: Counts, faces the player in some states, checks for a hit, sets the hit box, goes to the state's handler and drives the body ($D032).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HeadChecksThenBody093E:
    inc a:OBJ_RAIN_COUNT,x
    jsr FacePlayerInSomeStates093E
    jsr State5WhenHit093E
    jsr SetHitBoxByFrame093F
    jsr ByState093ECFAB
    jmp DriveBodyBelow093F

; @routine 093E:7AB ByState093ECFAB
; @run 71:CFAB
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $CFBD.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState093ECFABHandlers = $CFBD ; a handler per state
ByState093ECFAB:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState093ECFABHandlers,y
    sta z:zTemp00
    lda a:ByState093ECFABHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093E:7C9 FacePlayerInSomeStates093E
; @run 71:CFC9
; WHAT IT DOES: Faces the player in the states the table at $CFDA marks.
; HOW: SideOfPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FacePlayerInSomeStates093EFaced = $CFDA ; whether each state faces the player
FacePlayerInSomeStates093E:
    ldy a:OBJ_STATE,x
    lda a:FacePlayerInSomeStates093EFaced,y
    beq FacePlayerInSomeStates093E_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FacePlayerInSomeStates093E_at0:
    rts

; @routine 093E:7EA State5WhenHit093E
; @run 71:CFEA
; WHAT IT DOES: Under state 5, a hit goes to state 5 from its start.
; HOW: Two tests.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
State5WhenHit093E:
    lda a:OBJ_STATE,x
    cmp #$05
    bcs State5WhenHit093E_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State5WhenHit093E_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State5WhenHit093E_at0:
    rts

; @routine 093F:007 SetHitBoxByFrame093F
; @run 71:D007
; WHAT IT DOES: The hit box the frame names (up to $23), its half width widened by 8.
; HOW: A four-byte table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHitBoxByFrame093FBoxes = $D605 ; four bytes by frame from 1
SetHitBoxByFrame093F:
    lda a:OBJ_FRAME,x
    sec
    sbc #$01
    cmp #$22
    bcc SetHitBoxByFrame093F_at0
    lda #$22
SetHitBoxByFrame093F_at0:
    asl a
    asl a
    tay
    lda a:SetHitBoxByFrame093FBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:SetHitBoxByFrame093FBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:SetHitBoxByFrame093FBoxes+2,y
    clc
    adc #$08
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:SetHitBoxByFrame093FBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 093F:032 DriveBodyBelow093F
; @run 71:D032
; WHAT IT DOES: With health under $17, once: puts the slot below in its next state and clears its pace bits. Else the slot below takes this place, facing and attribute, thrown at the frame's speed turned by facing, and moves one step.
; HOW: CopyObjectPlace and ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DriveBodyBelow093FSpeeds = $D56F ; VX high and VY high by frame from 1
DriveBodyBelow093F:
    lda a:OBJ_WORK_J,x
    bne DriveBodyBelow093F_at2
    lda a:OBJ_HEALTH,x
    cmp #$17
    bcs DriveBodyBelow093F_at0
    inc a:OBJ_STATE-1,x
    inc a:OBJ_WORK_J,x
    lda a:OBJ_PACE,x
    and #$FC
    sta a:OBJ_PACE,x
    rts
DriveBodyBelow093F_at0:
    txa
    tay
    dey
    jsr VecCopyObjectPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda a:OBJ_SPRITE_ATTR,x
    sta a:OBJ_SPRITE_ATTR,y
    phx
    lda a:OBJ_FRAME,x
    sec
    sbc #$01
    asl a
    tax
    lda a:DriveBodyBelow093FSpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:DriveBodyBelow093FSpeeds+1,x
    sta a:OBJ_VY_HI,y
    plx
    sxy
    lda a:OBJ_FACING_LEFT,x
    beq DriveBodyBelow093F_at1
    jsr VecNegateVelocityX
DriveBodyBelow093F_at1:
    jsr VecApplyVelocity
    sxy
DriveBodyBelow093F_at2:
    rts
