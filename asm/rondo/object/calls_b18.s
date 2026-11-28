; @routine 072C:6C4 LaunchByPattern072C
; @run 70:BEC4
; WHAT IT DOES: Animates, hit-tested, moving at the speeds the pattern for OBJ_WORK_D picks, then $BE56.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LaunchByPattern072CVxHi = $B46E ; vx high
LaunchByPattern072CVxLo = $B52E ; vx low
LaunchByPattern072CVyHi = $B5EE ; vy high
LaunchByPattern072CVyLo = $B6AE ; vy low
LaunchByPattern072CPatterns = $BE24 ; speed index by OBJ_WORK_D
LaunchByPattern072C:
    lda #$FD
    sta z:zTemp10
    lda #$BE
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    ldy a:OBJ_WORK_D,x
    lda a:LaunchByPattern072CPatterns,y
    clc
    adc #$04
    tay
    lda a:LaunchByPattern072CVxHi,y
    sta a:OBJ_VX_HI,x
    lda a:LaunchByPattern072CVxLo,y
    sta a:OBJ_VX_LO,x
    lda a:LaunchByPattern072CVyHi,y
    sta a:OBJ_VY_HI,x
    lda a:LaunchByPattern072CVyLo,y
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    jsr BurstAtHeightE0
    rts

; @routine 0EBD:635 ArcAndSpin0EBD
; @run 71:C635
; WHAT IT DOES: Moves out at angle OBJ_WORK_P, which turns by $60/256 each count; at $F0 cleared.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ArcAndSpin0EBD:
    lda #$2D
    sta z:zTemp10
    lda #$C5
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$80
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_P,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_WORK_O,x
    clc
    adc #$60
    sta a:OBJ_WORK_O,x
    lda a:OBJ_WORK_P,x
    adc #$00
    sta a:OBJ_WORK_P,x
    cmp #$F0
    bcc ArcAndSpin0EBD_at0
    jmp VecClearObject
ArcAndSpin0EBD_at0:
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    rts

; @routine 0EBE:1AE LoadAtTopOrFace0EBE
; @run 71:C9AE
; WHAT IT DOES: Loads; with no start argument placed at one of four spots along the top, cleared if $C9EF says so, else facing the player.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAtTopOrFace0EBEPlaces = $C9E7 ; four X places
LoadAtTopOrFace0EBE:
    jsr VecLoadObjectRecord
    lda #$50
    sta a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    bne LoadAtTopOrFace0EBE_at0
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:LoadAtTopOrFace0EBEPlaces+1,y
    sta a:OBJ_X_HI,x
    lda a:LoadAtTopOrFace0EBEPlaces,y
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$02
    sta a:OBJ_Y_LO,x
    jsr LevelXBefore150_0EBE
    bcc LoadAtTopOrFace0EBE_at1
    jmp VecClearObject
LoadAtTopOrFace0EBE_at1:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
LoadAtTopOrFace0EBE_at0:
    rts

; @routine 0EBE:611 TossThenBecome9B0EBE
; @run 71:CE11
; WHAT IT DOES: Loads moving away from or toward the player; falls animated for OBJ_WORK_E counts and then becomes kind $9B.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TossThenBecome9B0EBELists = $CE4A ; frame lists by OBJ_WORK_D
TossThenBecome9B0EBE:
    lda a:OBJ_STATE,x
    bne TossThenBecome9B0EBE_at0
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    beq TossThenBecome9B0EBE_at1
    jsr VecNegateVelocityX
TossThenBecome9B0EBE_at1:
    rts
TossThenBecome9B0EBE_at0:
    dec a:OBJ_WORK_E,x
    bne TossThenBecome9B0EBE_at2
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
TossThenBecome9B0EBE_at2:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:TossThenBecome9B0EBELists,y
    sta z:zTemp10
    lda a:TossThenBecome9B0EBELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 117B:4DC DoorByHit117B
; @run 70:B4DC
; WHAT IT DOES: A flagged door: a weak hit while it is open takes the route; start arguments other than 0 and 2 set zField82 to 3.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DoorByHit117B:
    lda a:OBJ_STATE,x
    bne DoorByHit117B_at0
    jsr VecLoadObjectRecord
    lda #$0C
    sta a:OBJ_HITBOX_HALF_W,x
    rts
DoorByHit117B_at0:
    lda a:StageDoorHeld
    eor #$01
    and a:PlayerDoorReady
    beq DoorByHit117B_at1
    lda a:OBJ_HIT_TAKEN,x
    beq DoorByHit117B_at1
    cmp #$03
    bcs DoorByHit117B_at1
    lda a:OBJ_START_ARG,x
    beq DoorByHit117B_at2
    cmp #$02
    beq DoorByHit117B_at2
    lda #$03
    sta z:zField82
    bra DoorByHit117B_at2
DoorByHit117B_at2:
    lda #$80
    sta z:zField8C
    lda #$03
    sta z:zSubStep
DoorByHit117B_at1:
    rts

; @routine 117D:4AD KnockedLoose117D
; @run 71:C4AD
; WHAT IT DOES: Hit outside state 5: not hit-tested, the previous slot cleared, knocked upward, state 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
KnockedLoose117D:
    lda a:OBJ_STATE,x
    cmp #$05
    beq KnockedLoose117D_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq KnockedLoose117D_at0
    stz a:OBJ_HITBOX_ON,x
    txa
    tay
    dey
    jsr VecClearObjectInY
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    lda #$40
    sta a:OBJ_WORK_D,x
    lda #$05
    sta a:OBJ_STATE,x
KnockedLoose117D_at0:
    rts

; @routine 117D:5FF NextSubstateByHeight117D
; @run 71:C5FF
; WHAT IT DOES: Far above or below: state 3 (every fourth time substate 3); near: the next substate from the list for the start argument.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
NextSubstateByHeight117DCycles = $C638 ; 16 substates per start argument
NextSubstateByHeight117D:
    jsr VecDistanceYToPlayer
    cmp #$18
    bcc NextSubstateByHeight117D_at0
    inc a:OBJ_WORK_I,x
    lda a:OBJ_WORK_I,x
    and #$03
    beq NextSubstateByHeight117D_at1
    lda #$03
    sta a:OBJ_STATE,x
    rts
NextSubstateByHeight117D_at1:
    lda #$03
    sta a:OBJ_SUBSTATE,x
    rts
NextSubstateByHeight117D_at0:
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_WORK_I,x
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:NextSubstateByHeight117DCycles,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_I,x
    rts

; @routine 117E:0CF AccelerateToSpeed117E
; @run 71:C8CF
; WHAT IT DOES: Animates and moves, adding to the X speed until it reaches the target for OBJ_WORK_D; then the state goes on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AccelerateToSpeed117ESteps = $C907 ; step and target per OBJ_WORK_D, four bytes
AccelerateToSpeed117E:
    lda #$C8
    sta z:zTemp11
    lda #$BE
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:AccelerateToSpeed117ESteps,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:AccelerateToSpeed117ESteps+1,y
    sta a:OBJ_VX_HI,x
    cmp a:AccelerateToSpeed117ESteps+3,y
    bne AccelerateToSpeed117E_at0
    lda a:OBJ_VX_LO,x
    cmp a:AccelerateToSpeed117ESteps+2,y
    bne AccelerateToSpeed117E_at0
    inc a:OBJ_STATE,x
AccelerateToSpeed117E_at0:
    rts

; @routine 117F:500 ChargeUntilListEnds117F
; @run 71:D500
; WHAT IT DOES: Runs the list at $D538, moving at the facing's speed on each new frame (sounds at step 8); at its end substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChargeUntilListEnds117FSpeeds = $D551 ; vx high byte by facing
ChargeUntilListEnds117F:
    lda #$38
    sta z:zTemp10
    lda #$D5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ChargeUntilListEnds117F_at0
    jmp ChargeUntilListEnds117F_at1
ChargeUntilListEnds117F_at0:
    bcc ChargeUntilListEnds117F_at2
    lda a:OBJ_ANIM_STEP,x
    cmp #$08
    bne ChargeUntilListEnds117F_at3
    lda #$E0
    jsr VecSoundCommand
    lda #$1A
    jsr VecSoundCommand
ChargeUntilListEnds117F_at3:
    ldy a:OBJ_FACING_LEFT,x
    lda a:ChargeUntilListEnds117FSpeeds,y
    sta a:OBJ_VX_HI,x
    jmp ProbeHeightCall117F_46F
ChargeUntilListEnds117F_at1:
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
ChargeUntilListEnds117F_at2:
    rts

; @routine 12DE:122 TurnQuarterAlt12DE
; @run 71:C922
; WHAT IT DOES: TurnQuarter12DE with OBJ_WORK_G as the speed's low byte (zTemp00): the entry five bytes before it.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnQuarterAlt12DE:
    lda a:OBJ_WORK_G,x
    sta z:zTemp00

; @routine 161B:2A1 BobAndCount161B
; @run 70:B2A1
; WHAT IT DOES: Bobs by the cosine of OBJ_WORK_D; after OBJ_WORK_E counts the state goes back.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BobAndCount161B:
    lda #$D9
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
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_D,x
    clc
    adc #$02
    sta a:OBJ_WORK_D,x
    dec a:OBJ_WORK_E,x
    bne BobAndCount161B_at0
    dec a:OBJ_STATE,x
BobAndCount161B_at0:
    rts

; @routine 161E:277 CountDownSpawning161E
; @run 71:CA77
; WHAT IT DOES: Counts OBJ_WORK_E down; from $20 on, every eighth count a kind $A2 object at a height by the count; past 0, frame 5 and state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountDownSpawning161EHeights = $CBD4 ; Y by the count over 8
CountDownSpawning161E:
    dec a:OBJ_WORK_E,x
    lda a:OBJ_WORK_E,x
    cmp #$FF
    beq CountDownSpawning161E_at0
    cmp #$21
    bcs CountDownSpawning161E_at1
    and #$07
    bne CountDownSpawning161E_at1
    jsr VecFindFreeSlotFrom30
    bcc CountDownSpawning161E_at1
    lda #$A2
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_E,x
    lsr a
    lsr a
    lsr a
    phx
    tax
    lda a:CountDownSpawning161EHeights,x
    sta a:OBJ_Y_LO,y
    plx
    rts
CountDownSpawning161E_at0:
    lda #$05
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    stz a:OBJ_WORK_E,x
CountDownSpawning161E_at1:
    rts

; @routine 18DB:260 StopMusicOnStep18DB
; @run 70:B260
; WHAT IT DOES: At slot $18's animation step 7: list channel $81, the sound stopped, the CD track again, state on; before it, attribute $10.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StopMusicOnStep18DB:
    lda a:OBJ_ANIM_STEP+$18
    beq StopMusicOnStep18DB_at0
    cmp #$07
    beq StopMusicOnStep18DB_at1
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
StopMusicOnStep18DB_at0:
    rts
StopMusicOnStep18DB_at1:
    stz a:OBJ_FRAME,x
    phx
    lda #$81
    ldx #$05
    jsr VecStartListChannelKind2
    smb4 z:zField8E
    lda #$FE
    jsr VecSoundCommand
    rmb3 z:zField8E
    rmb4 z:zField8E
    jsr VecCdPlayAgain
    plx
    lda z:zField8E
    ora #$04
    sta z:zField8E
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts
