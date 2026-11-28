; @routine 0BFB:0C6 AddFiftyHeartsOnHit0BFB
; @run 70:B0C6
; WHAT IT DOES: In state 0 becomes kind $84 when zField92 bit 0 is set, else loads its record and sets its Y low to
; $32; later, on a hit of 1 to 4 sounds $0D, adds 50 to the hearts (zField94, BCD, at most 99), runs the score
; blend at $9181 and removes itself (Stage 07's overlay).
; HOW: A test of the state; a decimal add with a cap; BlendStageScoreAddRows, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AddFiftyHeartsOnHit0BFB:
    lda a:OBJ_STATE,x
    bne AddFiftyHeartsOnHit0BFB_at0
    lda z:zField92
    and #$01
    bne AddFiftyHeartsOnHit0BFB_at1
    jmp AddFiftyHeartsOnHit0BFB_at2
AddFiftyHeartsOnHit0BFB_at1:
    lda #$84
    sta a:OBJ_KIND,x
    lda z:zField92
    ora #$01
    sta z:zField92
    rts
AddFiftyHeartsOnHit0BFB_at2:
    jsr VecLoadObjectRecord
    lda #$32
    sta a:OBJ_Y_LO,x
    rts
AddFiftyHeartsOnHit0BFB_at0:
    lda a:OBJ_HIT_TAKEN,x
    beq AddFiftyHeartsOnHit0BFB_at3
    cmp #$05
    bcs AddFiftyHeartsOnHit0BFB_at3
    lda #$0D
    jsr VecSoundCommand
    sed
    lda z:zField94
    clc
    adc #$50
    sta z:zField94
    bcc AddFiftyHeartsOnHit0BFB_at4
    lda #$99
    sta z:zField94
AddFiftyHeartsOnHit0BFB_at4:
    cld
    jsr BlendStageScoreAddRows
    jmp VecClearObject
AddFiftyHeartsOnHit0BFB_at3:
    rts

; @routine 0BFB:1D9 ShoveByState0BFB
; @run 70:B1D9
; WHAT IT DOES: Runs the step of the object's state: 0 loads its record, 1 ShoveToEdgeWhenHit0BFB, 2 does nothing
; (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ShoveByState0BFBSteps = $B1EB ; the routine for each state, low and high byte
ShoveByState0BFB:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ShoveByState0BFBSteps,y
    sta z:zTemp00
    lda a:ShoveByState0BFBSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFB:2A5 BreakPairByState0BFB
; @run 70:B2A5
; WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordBoxDown08_0BFB, 1 BreakWithNextSlot0BFB, 2 does
; nothing (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BreakPairByState0BFBSteps = $B2B7 ; the routine for each state, low and high byte
BreakPairByState0BFB:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:BreakPairByState0BFBSteps,y
    sta z:zTemp00
    lda a:BreakPairByState0BFBSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFB:2EE GuardThenFallByState0BFB
; @run 70:B2EE
; WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordBoxDown4B_0BFB, 1 SoundAndGuardOnHit0BFB, 2
; FallThenClearLow0BFB (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
GuardThenFallByState0BFBSteps = $B300 ; the routine for each state, low and high byte
GuardThenFallByState0BFB:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:GuardThenFallByState0BFBSteps,y
    sta z:zTemp00
    lda a:GuardThenFallByState0BFBSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFB:4B0 HoldGuardsAroundIt0BFB
; @run 70:B4B0
; WHAT IT DOES: Opens once its guards are down (OpenWhenGuardsDown0BFB), takes state 5 when hit in state 4, and
; runs its state's step ($B4FF); then in states 4 and 5 puts each of the three slots before it that holds kind $2D
; in a state below 2 at its own place, moved by that slot's fixed offset (Stage 07's overlay).
; HOW: A loop over X-3 to X-1 of CopyObjectPlace and ApplyVelocity on the slot, the offsets stored as its X and Y
; speed high bytes; zRichterObject keeps X.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HoldGuardsAroundIt0BFBOffsets = $B7FE ; the X and Y speed high byte for each of the three guards: (0,-$26), (+$14,-6), (-$14,-6)
HoldGuardsAroundIt0BFB:
    jsr OpenWhenGuardsDown0BFB
    jsr State5WhenHitInState4_0BFB
    jsr RiseSteerByState0BFB
    lda a:OBJ_STATE,x
    cmp #$04
    bcc HoldGuardsAroundIt0BFB_at0
    cmp #$06
    bcs HoldGuardsAroundIt0BFB_at0
    stz z:zTemp00
    txa
    sec
    sbc #$03
    tay
HoldGuardsAroundIt0BFB_at2:
    lda a:OBJ_KIND,y
    cmp #$2D
    bne HoldGuardsAroundIt0BFB_at1
    lda a:OBJ_STATE,y
    cmp #$02
    bcs HoldGuardsAroundIt0BFB_at1
    jsr VecCopyObjectPlace
    stx z:zClimbStepBefore
    lda z:zTemp00
    asl a
    tax
    lda a:HoldGuardsAroundIt0BFBOffsets,x
    sta a:OBJ_VX_HI,y
    lda a:HoldGuardsAroundIt0BFBOffsets+1,x
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zClimbStepBefore
HoldGuardsAroundIt0BFB_at1:
    iny
    inc z:zTemp00
    lda z:zTemp00
    cmp #$03
    bcc HoldGuardsAroundIt0BFB_at2
HoldGuardsAroundIt0BFB_at0:
    rts

; @routine 0BFB:4FF RiseSteerByState0BFB
; @run 70:B4FF
; WHAT IT DOES: Runs the step of the object's state: 0 loads its record, 1 RiseAtScreen3_0BFB, 2 RiseToListEnd0BFB,
; 3 OpenAndStartList0BFB, 4 SteerTowardPlayer0BFB, 5 BackAwayFromPlayer0BFB, 6 DefeatBySubstate0BFB (Stage 07's
; overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RiseSteerByState0BFBSteps = $B511 ; the routine for each state, low and high byte
RiseSteerByState0BFB:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RiseSteerByState0BFBSteps,y
    sta z:zTemp00
    lda a:RiseSteerByState0BFBSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFB:56C JustLoadRecord0BFB
; @run 70:B56C
; WHAT IT DOES: Loads the object's record (Stage 07's overlay).
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
JustLoadRecord0BFB:
    jsr VecLoadObjectRecord
    rts

; @routine 0BFB:5C9 OpenAndStartList0BFB
; @run 70:B5C9
; WHAT IT DOES: Moves by its Y speed and plays the frame list at $B5F8; when the list ends sounds $57, starts the
; objects of its list (StartObjectsFromList0BFB), sets OBJ_HITBOX_ON 2 and OBJ_PACE bits 4 and 6, restarts its
; animation and steps the state on (Stage 07's overlay).
; HOW: ApplyVelocityY, StepObjectFrames, then stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
OpenAndStartList0BFB:
    jsr VecApplyVelocityY
    lda #$F8
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne OpenAndStartList0BFB_at0
    lda #$57
    jsr VecSoundCommand
    jsr StartObjectsFromList0BFB
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_PACE,x
    ora #$50
    sta a:OBJ_PACE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
OpenAndStartList0BFB_at0:
    rts

; @routine 0BFB:6FC DefeatBySubstate0BFB
; @run 70:B6FC
; WHAT IT DOES: Runs the step of the object's substate: 0 DefeatSoundsAndRecord0BFB, 1 CountThenFall0BFB, 2
; MoveThenOpenScroll0BFB (Stage 07's overlay).
; HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
DefeatBySubstate0BFBSteps = $B70E ; the routine for each substate, low and high byte
DefeatBySubstate0BFB:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:DefeatBySubstate0BFBSteps,y
    sta z:zTemp00
    lda a:DefeatBySubstate0BFBSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFB:743 MoveThenOpenScroll0BFB
; @run 70:B743
; WHAT IT DOES: Moves by its Y speed, counts OBJ_WORK_H down and drops pieces on its counts (PieceOnCount0BFB);
; while its Y is $100 to $1BF sets zField8E bit 7, sets the scroll's X range to high bytes 1 to 5 (ScrollXMinHi,
; ScrollXMaxHi) and removes itself (Stage 07's overlay).
; HOW: ApplyVelocityY, DEC, two compares of Y, SMB7, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveThenOpenScroll0BFB:
    jsr VecApplyVelocityY
    dec a:OBJ_WORK_H,x
    jsr PieceOnCount0BFB
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne MoveThenOpenScroll0BFB_at0
    lda a:OBJ_Y_LO,x
    cmp #$C0
    bcs MoveThenOpenScroll0BFB_at0
    smb7 z:zField8E
    lda #$01
    sta a:ScrollXMinHi
    lda #$05
    sta a:ScrollXMaxHi
    jmp VecClearObject
MoveThenOpenScroll0BFB_at0:
    rts

; @routine 0BFB:7BE AngleFrameByState0BFB
; @run 70:B7BE
; WHAT IT DOES: Runs the step of the object's state: 0 loads its record, 1 FrameByAngleUntilHit0BFB, 2
; FallDroppingPieces0BFC (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AngleFrameByState0BFBSteps = $B7D0 ; the routine for each state, low and high byte
AngleFrameByState0BFB:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:AngleFrameByState0BFBSteps,y
    sta z:zTemp00
    lda a:AngleFrameByState0BFBSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFC:039 OrbitByState0BFC
; @run 70:B839
; WHAT IT DOES: Runs the step of the object's state: 0 StartAngleByArg0BFC, 1 OrbitSlot4C0BFC, 2 FallFromRest0BFC
; (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
OrbitByState0BFCSteps = $B84B ; the routine for each state, low and high byte
OrbitByState0BFC:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:OrbitByState0BFCSteps,y
    sta z:zTemp00
    lda a:OrbitByState0BFCSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFC:474 LoadRecordWait50_0BFC
; @run 70:BC74
; WHAT IT DOES: Loads the object's record and sets OBJ_WORK_D to $50 (Stage 07's overlay).
; HOW: LoadObjectRecord and a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordWait50_0BFC:
    jsr VecLoadObjectRecord
    lda #$50
    sta a:OBJ_WORK_D,x
    rts

; @routine 0BFC:47D LandThenStateOn0BFC
; @run 70:BC7D
; WHAT IT DOES: Falls and lands with height 6, stopping dead; on landing steps the state on (Stage 07's overlay).
; HOW: StopOnLanding, then INC on carry.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandThenStateOn0BFC:
    lda #$06
    jsr VecStopOnLanding
    bcc LandThenStateOn0BFC_at0
    inc a:OBJ_STATE,x
LandThenStateOn0BFC_at0:
    rts

; @routine 0BFC:488 WalkTowardUntilNear0BFC
; @run 70:BC88
; WHAT IT DOES: Walks a step (StepWalkByWorkE0BFC) and keeps its facing in OBJ_WORK_E as the way to walk; within
; $4C of the player along X goes to state 3; then checks the ground ahead (TurnAtEdgeOrCount0BFC) (Stage 07's
; overlay).
; HOW: The walk, DistanceXToPlayer and a compare, then a jump to $BD2F.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkTowardUntilNear0BFC:
    jsr StepWalkByWorkE0BFC
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$4C
    bcs WalkTowardUntilNear0BFC_at0
    lda #$03
    sta a:OBJ_STATE,x
WalkTowardUntilNear0BFC_at0:
    jmp TurnAtEdgeOrCount0BFC

; @routine 0BFC:4A0 StepWalkByWorkE0BFC
; @run 70:BCA0
; WHAT IT DOES: Faces the player and plays frame list 0; on the list's steps that report carry moves by an X speed
; of -4 or +4 by OBJ_WORK_E and moves down one pixel (Stage 07's overlay).
; HOW: FacePlayerPlayList0BFC with Y 0; a table read; ApplyVelocityX; a 16-bit INC of Y.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StepWalkByWorkE0BFCSpeeds = $BCBB ; the X speed high byte by OBJ_WORK_E: -4, +4
StepWalkByWorkE0BFC:
    cly
    jsr FacePlayerPlayList0BFC
    bcc StepWalkByWorkE0BFC_at0
    ldy a:OBJ_WORK_E,x
    lda a:StepWalkByWorkE0BFCSpeeds,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    inc a:OBJ_Y_LO,x
    bne StepWalkByWorkE0BFC_at0
    inc a:OBJ_Y_HI,x
StepWalkByWorkE0BFC_at0:
    rts

; @routine 0BFC:52F TurnAtEdgeOrCount0BFC
; @run 70:BD2F
; WHAT IT DOES: Probes the terrain with height 6: with no bit 0 for request $09 or with bit 1 for request $22,
; restarts the animation in state 5, substate 0; otherwise counts OBJ_WORK_D down and at 0 restarts it in state 4
; (Stage 07's overlay).
; HOW: Two ProbeTerrain calls with zProduct1 6; TileHit and the result tested by bit.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnAtEdgeOrCount0BFC:
    lda #$06
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    lda a:TileHit
    and #$01
    beq TurnAtEdgeOrCount0BFC_at0
    lda #$06
    sta z:SpriteFlipMask
    lda #$22
    jsr VecProbeTerrain
    and #$02
    beq TurnAtEdgeOrCount0BFC_at1
TurnAtEdgeOrCount0BFC_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    lda #$05
    sta a:OBJ_STATE,x
    rts
TurnAtEdgeOrCount0BFC_at1:
    dec a:OBJ_WORK_D,x
    bne TurnAtEdgeOrCount0BFC_at2
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
TurnAtEdgeOrCount0BFC_at2:
    rts

; @routine 0BFC:56C WalkAwayUntilFar0BFC
; @run 70:BD6C
; WHAT IT DOES: Walks a step (StepWalkByWorkE0BFC) with OBJ_WORK_E the opposite of its facing; at $6C or more from
; the player along X goes to state 2; then checks the ground ahead (TurnAtEdgeOrCount0BFC) (Stage 07's overlay).
; HOW: The walk, EOR #1, DistanceXToPlayer and a compare, then a jump to $BD2F.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkAwayUntilFar0BFC:
    jsr StepWalkByWorkE0BFC
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$6C
    bcc WalkAwayUntilFar0BFC_at0
    lda #$02
    sta a:OBJ_STATE,x
WalkAwayUntilFar0BFC_at0:
    jmp TurnAtEdgeOrCount0BFC

; @routine 0BFC:586 ThrowOnFrame47_0BFC
; @run 70:BD86
; WHAT IT DOES: Faces the player and plays frame list 2; on a step with carry at frame $47 starts kind $39 at its
; place in a free slot from $30; at the list's end restarts its animation in state 3, steps OBJ_WORK_F on and sets
; OBJ_WORK_D to the wait its low two bits pick (Stage 07's overlay).
; HOW: FacePlayerPlayList0BFC with Y 2; FindFreeSlotFrom30, StartObjectAtPlace; a table read.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ThrowOnFrame47_0BFCWaits = $BCBD ; OBJ_WORK_D for each turn of OBJ_WORK_F's low two bits: $80, 8, 8, $40
ThrowOnFrame47_0BFC:
    ldy #$02
    jsr FacePlayerPlayList0BFC
    beq ThrowOnFrame47_0BFC_at0
    bcc ThrowOnFrame47_0BFC_at1
    lda a:OBJ_FRAME,x
    cmp #$47
    bne ThrowOnFrame47_0BFC_at1
    jsr VecFindFreeSlotFrom30
    bcc ThrowOnFrame47_0BFC_at1
    lda #$39
    jsr VecStartObjectAtPlace
    rts
ThrowOnFrame47_0BFC_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_F,x
    lda a:OBJ_WORK_F,x
    and #$03
    tay
    lda a:ThrowOnFrame47_0BFCWaits,y
    sta a:OBJ_WORK_D,x
ThrowOnFrame47_0BFC_at1:
    rts

; @routine 0BFC:5C9 LeapBySubstate0BFC
; @run 70:BDC9
; WHAT IT DOES: Runs the step of the object's substate: 0 LeapAfterList0BFC, 1 LandThenSubstate0BFC, 2
; LandListThenState3_0BFC (Stage 07's overlay).
; HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LeapBySubstate0BFCSteps = $BDDB ; the routine for each substate, low and high byte
LeapBySubstate0BFC:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:LeapBySubstate0BFCSteps,y
    sta z:zTemp00
    lda a:LeapBySubstate0BFCSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFC:5E1 LeapAfterList0BFC
; @run 70:BDE1
; WHAT IT DOES: Faces the player and plays frame list 4; at its end leaps: X speed -2 or +2 by OBJ_WORK_E (by its
; facing turned when OBJ_START_ARG is not 0), Y speed -3, animation restarted and substate stepped on (Stage 07's
; overlay).
; HOW: FacePlayerPlayList0BFC with Y 4; table reads; stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LeapAfterList0BFCSpeeds = $BE17 ; the X speed high byte by direction: -2, +2
LeapAfterList0BFC:
    ldy #$04
    jsr FacePlayerPlayList0BFC
    bne LeapAfterList0BFC_at0
    ldy a:OBJ_WORK_E,x
    lda a:LeapAfterList0BFCSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_START_ARG,x
    beq LeapAfterList0BFC_at0
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    tay
    lda a:LeapAfterList0BFCSpeeds,y
    sta a:OBJ_VX_HI,x
LeapAfterList0BFC_at0:
    rts

; @routine 0BFC:624 LandListThenState3_0BFC
; @run 70:BE24
; WHAT IT DOES: Faces the player and plays frame list 6; at its end restarts its animation in state 3, substate 0
; (Stage 07's overlay).
; HOW: FacePlayerPlayList0BFC with Y 6, then stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LandListThenState3_0BFC:
    ldy #$06
    jsr FacePlayerPlayList0BFC
    bne LandListThenState3_0BFC_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
LandListThenState3_0BFC_at0:
    rts

; @routine 0BFD:05F LoadAndStartFollowers0BFD
; @run 71:C05F
; WHAT IT DOES: Loads its record; with OBJ_START_ARG below $80 starts that many objects of kind $3C at its place in
; the slots after it (OBJ_START_ARG $81 up, OBJ_WORK_M its slot, OBJ_WORK_N its slot plus the count), counting
; OBJ_START_ARG down to 0; then clears bit 7 of OBJ_START_ARG, faces left when its bit 0 is clear, and sets its
; side speed, a random wait and a random OBJ_WORK_F:OBJ_WORK_G (Stage 07's overlay).
; HOW: A loop of ClearObjectInY and StartObjectAtPlace; SideStepByArg0BFD, RandomWaitDE0BFD, RandomSpeedFG0BFD.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
LoadAndStartFollowers0BFD:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    bmi LoadAndStartFollowers0BFD_at0
    txa
    sta a:OBJ_WORK_M,x
    clc
    adc a:OBJ_START_ARG,x
    sta a:OBJ_WORK_N,x
    txa
    tay
    iny
    lda #$81
    sta z:zTemp00
LoadAndStartFollowers0BFD_at1:
    jsr VecClearObjectInY
    lda #$3C
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    txa
    sta a:OBJ_WORK_M,y
    lda a:OBJ_WORK_N,x
    sta a:OBJ_WORK_N,y
    inc z:zTemp00
    iny
    dec a:OBJ_START_ARG,x
    bne LoadAndStartFollowers0BFD_at1
LoadAndStartFollowers0BFD_at0:
    lda a:OBJ_START_ARG,x
    and #$7F
    sta a:OBJ_START_ARG,x
    and #$01
    eor #$01
    sta a:OBJ_FACING_LEFT,x
    jsr SideStepByArg0BFD
    jsr RandomWaitDE0BFD
    jsr RandomSpeedFG0BFD
    rts

; @routine 0BFD:10E CountWaitAndSpeed0BFD
; @run 71:C10E
; WHAT IT DOES: Runs both counts of the object: WaitThenState2_0BFD, then CountFGThenState3_0BFD (Stage 07's
; overlay).
; HOW: Two calls.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
CountWaitAndSpeed0BFD:
    jsr WaitThenState2_0BFD
    jsr CountFGThenState3_0BFD
    rts

; @routine 0BFD:115 WaitThenState2_0BFD
; @run 71:C115
; WHAT IT DOES: With OBJ_WORK_D:OBJ_WORK_E at 0 takes a new random wait (RandomWaitDE0BFD) and restarts its
; animation in state 2; otherwise counts it down by 1 (Stage 07's overlay).
; HOW: An ORA test; a 16-bit subtract.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
WaitThenState2_0BFD:
    lda a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    bne WaitThenState2_0BFD_at0
    jsr RandomWaitDE0BFD
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$02
    sta a:OBJ_STATE,x
    rts
WaitThenState2_0BFD_at0:
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    rts

; @routine 0BFD:13E CountFGThenState3_0BFD
; @run 71:C13E
; WHAT IT DOES: While its X high byte is 0, counts OBJ_WORK_F:OBJ_WORK_G down by 1; at 0 takes a new random value
; (RandomSpeedFG0BFD) and restarts its animation in state 3, substate 0 (Stage 07's overlay).
; HOW: A test of X high; an ORA test; a 16-bit subtract.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
CountFGThenState3_0BFD:
    lda a:OBJ_X_HI,x
    bne CountFGThenState3_0BFD_at0
    lda a:OBJ_WORK_F,x
    ora a:OBJ_WORK_G,x
    bne CountFGThenState3_0BFD_at1
    jsr RandomSpeedFG0BFD
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    lda #$03
    sta a:OBJ_STATE,x
    rts
CountFGThenState3_0BFD_at1:
    lda a:OBJ_WORK_G,x
    sec
    sbc #$01
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    sbc #$00
    sta a:OBJ_WORK_F,x
CountFGThenState3_0BFD_at0:
    rts

; @routine 0BFD:1A4 ThrowBySubstate0BFD
; @run 71:C1A4
; WHAT IT DOES: Runs the step of the object's substate: 0 PickThrow0BFD, 1 ThrowKindByPick0BFD, 2
; ListByPickAndJ0BFD, 3 RepeatOrStand0BFD (Stage 07's overlay).
; HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ThrowBySubstate0BFDSteps = $C1B6 ; the routine for each substate, low and high byte
ThrowBySubstate0BFD:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:ThrowBySubstate0BFDSteps,y
    sta z:zTemp00
    lda a:ThrowBySubstate0BFDSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFD:330 LaunchFlyByState0BFD
; @run 71:C330
; WHAT IT DOES: Runs the step of the object's state: 0 LaunchWithKind3F0BFD, 1 WaitThenBoxOn0BFD, 2
; FlyByListWorkD0BFD (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LaunchFlyByState0BFDSteps = $C342 ; the routine for each state, low and high byte
LaunchFlyByState0BFD:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:LaunchFlyByState0BFDSteps,y
    sta z:zTemp00
    lda a:LaunchFlyByState0BFDSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFD:410 ByStateThenFollowY0BFD
; @run 71:C410
; WHAT IT DOES: Runs its state's step (BoxedByState0BFD), then takes its owner's Y (FollowOwnerY0BFD) (Stage 07's
; overlay).
; HOW: A call, then a jump to $C3C0.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ByStateThenFollowY0BFD:
    jsr BoxedByState0BFD
    jmp FollowOwnerY0BFD

; @routine 0BFD:416 BoxedByState0BFD
; @run 71:C416
; WHAT IT DOES: Runs the step of the object's state: 0 StartWithKind3FAtOne0BFD, 1 CountSoundBoxOn0BFD, 2
; AnimateBoxedTimed0BFD (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BoxedByState0BFDSteps = $C428 ; the routine for each state, low and high byte
BoxedByState0BFD:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:BoxedByState0BFDSteps,y
    sta z:zTemp00
    lda a:BoxedByState0BFDSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFD:465 CountSoundBoxOn0BFD
; @run 71:C465
; WHAT IT DOES: Counts OBJ_WORK_E down; at 0 sounds $26, sets OBJ_WORK_E to $40 and OBJ_HITBOX_ON to 2, and steps
; the state on (Stage 07's overlay).
; HOW: DEC, SoundCommand, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
CountSoundBoxOn0BFD:
    dec a:OBJ_WORK_E,x
    bne CountSoundBoxOn0BFD_at0
    lda #$26
    jsr VecSoundCommand
    lda #$40
    sta a:OBJ_WORK_E,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
CountSoundBoxOn0BFD_at0:
    rts

; @routine 0BFD:543 ShiftAngleTrail0BFD
; @run 71:C543
; WHAT IT DOES: Takes state 4 when hit (State4SoundWhenHit0BFD), marks its parts by its blink
; (MarkPartsByBlink0BFD) and runs its state's step (ChainByState0BFD); then shifts the overlay's 128-byte trail at
; $C5F7 back by one and puts OBJ_WORK_D plus $80 in its first byte (Stage 07's overlay).
; HOW: Three calls; a copy loop by Y from $7F down to 1; an add and a store.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
ShiftAngleTrail0BFDTrail = $C5F6 ; the trail of past angles at $C5F7 to $C676, newest first; read from $C5F6 by Y 1 to $7F
ShiftAngleTrail0BFD:
    jsr State4SoundWhenHit0BFD
    jsr MarkPartsByBlink0BFD
    jsr ChainByState0BFD
    ldy #$7F
ShiftAngleTrail0BFD_at0:
    lda a:ShiftAngleTrail0BFDTrail,y
    sta a:ShiftAngleTrail0BFDTrail+1,y
    dey
    bne ShiftAngleTrail0BFD_at0
    lda a:OBJ_WORK_D,x
    clc
    adc #$80
    sta a:ShiftAngleTrail0BFDTrail+1
    rts

; @routine 0BFD:5AA ChainByState0BFD
; @run 71:C5AA
; WHAT IT DOES: Runs the step of the object's state: 0 StartNineLinks0BFD, 1 SwayByRowStep0BFD, 2
; StartKindC2ThenState1_0BFD, 3 DragThenBySubstate0BFD, 4 CollapseAndClear0BFE (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ChainByState0BFDSteps = $C5BC ; the routine for each state, low and high byte
ChainByState0BFD:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ChainByState0BFDSteps,y
    sta z:zTemp00
    lda a:ChainByState0BFDSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFD:677 SwayByRowStep0BFD
; @run 71:C677
; WHAT IT DOES: Takes its next state on its timer, drags its chain, eases its speed and scales the cosine by its
; row; adds the scaled cosine to OBJ_WORK_D:OBJ_WORK_E and the row's other word to OBJ_WORK_K:OBJ_WORK_L; when that
; add carries out, picks a row 0 to 3 at random into OBJ_WORK_M and clears OBJ_WORK_D, E, K and L (Stage 07's
; overlay).
; HOW: NextStateOnTimer0BFD, DragChainAndMove0BFD, EaseWorkFToTarget0BFD, ScaleByLinkRow0BFD; 16-bit adds of
; zProduct and zMultiplier; NextRandom.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SwayByRowStep0BFD:
    jsr NextStateOnTimer0BFD
    jsr DragChainAndMove0BFD
    jsr EaseWorkFToTarget0BFD
    jsr ScaleByLinkRow0BFD
    lda a:OBJ_WORK_E,x
    clc
    adc z:SpriteFlipMask
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    adc z:zPlaneRowsLeft
    sta a:OBJ_WORK_D,x
    lda a:OBJ_WORK_L,x
    clc
    adc z:zSetUpColumnsLeft
    sta a:OBJ_WORK_L,x
    lda a:OBJ_WORK_K,x
    adc z:zTransposeRowsLeft
    sta a:OBJ_WORK_K,x
    bcc SwayByRowStep0BFD_at0
    jsr VecNextRandom
    and #$03
    sta a:OBJ_WORK_M,x
    stz a:OBJ_WORK_D,x
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_K,x
    stz a:OBJ_WORK_L,x
SwayByRowStep0BFD_at0:
    rts

; @routine 0BFD:7EE DragThenBySubstate0BFD
; @run 71:C7EE
; WHAT IT DOES: Drags its chain and eases its speed, then runs the step of its substate: 0 AimOrBackToState1_0BFE,
; 1 TurnToZeroBy2_0BFE, 2 TimedAimSound0BFE, 3 TurnTowardWorkH0BFE, 4 TurnToZeroThenState1_0BFE (Stage 07's
; overlay).
; HOW: DragChainAndMove0BFD, EaseWorkFToTarget0BFD; twice the substate indexes a word table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
DragThenBySubstate0BFDSteps = $C806 ; the routine for each substate, low and high byte
DragThenBySubstate0BFD:
    jsr DragChainAndMove0BFD
    jsr EaseWorkFToTarget0BFD
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:DragThenBySubstate0BFDSteps,y
    sta z:zTemp00
    lda a:DragThenBySubstate0BFDSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFE:010 AimOrBackToState1_0BFE
; @run 71:C810
; WHAT IT DOES: With the angle from its ninth link to the player $50 to $AF, puts the nine links in state 2, sets
; OBJ_WORK_I to $60 and steps the substate on; otherwise goes back to state 1 (Stage 07's overlay).
; HOW: AngleFromNinthToPlayer0BFD, two compares, SetNineAfterToState2_0BFE.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AimOrBackToState1_0BFE:
    jsr AngleFromNinthToPlayer0BFD
    cmp #$50
    bcc AimOrBackToState1_0BFE_at0
    cmp #$B0
    bcs AimOrBackToState1_0BFE_at0
    jsr SetNineAfterToState2_0BFE
    lda #$60
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
    rts
AimOrBackToState1_0BFE_at0:
    lda #$01
    sta a:OBJ_STATE,x
    rts

; @routine 0BFE:061 TimedAimSound0BFE
; @run 71:C861
; WHAT IT DOES: Counts OBJ_WORK_I down; at 0 aims its links at the player (AimNineLinksAtPlayer0BFE), sounds $B0,
; sets OBJ_WORK_I to $30 and steps the substate on (Stage 07's overlay).
; HOW: DEC, the call, SoundCommand, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TimedAimSound0BFE:
    dec a:OBJ_WORK_I,x
    bne TimedAimSound0BFE_at0
    jsr AimNineLinksAtPlayer0BFE
    lda #$B0
    jsr VecSoundCommand
    lda #$30
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
TimedAimSound0BFE_at0:
    rts

; @routine 0BFE:077 AimNineLinksAtPlayer0BFE
; @run 71:C877
; WHAT IT DOES: Takes the angle from its ninth link to the player, held to $50 to $B0 and rounded down to a
; multiple of 8, into OBJ_WORK_H of the object and of the nine slots after it (Stage 07's overlay).
; HOW: AngleFromNinthToPlayer0BFD, ClampA50ToB0_0BFE, AND #$F8; a store loop.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AimNineLinksAtPlayer0BFE:
    jsr AngleFromNinthToPlayer0BFD
    jsr ClampA50ToB0_0BFE
    and #$F8
    sta a:OBJ_WORK_H,x
    sta z:zTemp00
    txa
    tay
    iny
    lda #$09
    sta z:zTemp01
AimNineLinksAtPlayer0BFE_at0:
    lda z:zTemp00
    sta a:OBJ_WORK_H,y
    iny
    dec z:zTemp01
    bne AimNineLinksAtPlayer0BFE_at0
    rts

; @routine 0BFE:0EC CollapseAndClear0BFE
; @run 71:C8EC
; WHAT IT DOES: In substate 0 puts the nine links in state 3, sets frame $78 and OBJ_WORK_O $60, takes the hit box
; off and sets X speed 0 and Y speed -$400; then counts OBJ_WORK_O down, dropping pieces on its counts
; (PieceOnCount0BFE) and falling, and removes itself at 0 (Stage 07's overlay).
; HOW: SetNineAfterToState3_0BFE and stores; DEC, ApplyGravityUnlimited, ApplyVelocity, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CollapseAndClear0BFE:
    lda a:OBJ_SUBSTATE,x
    bne CollapseAndClear0BFE_at0
    jsr SetNineAfterToState3_0BFE
    lda #$78
    sta a:OBJ_FRAME,x
    lda #$60
    sta a:OBJ_WORK_O,x
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
    rts
CollapseAndClear0BFE_at0:
    dec a:OBJ_WORK_O,x
    bne CollapseAndClear0BFE_at1
    jmp VecClearObject
CollapseAndClear0BFE_at1:
    jsr PieceOnCount0BFE
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0BFE:169 LinkByState0BFE
; @run 71:C969
; WHAT IT DOES: Runs the step of the object's state: 0 loads its record, 1 AimLinkFromBuffer0BFE, 2
; EaseThenBySubstate0BFE, 3 LaunchLandBySubstate0BFE (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LinkByState0BFESteps = $C97B ; the routine for each state, low and high byte
LinkByState0BFE:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:LinkByState0BFESteps,y
    sta z:zTemp00
    lda a:LinkByState0BFESteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFE:264 LaunchLandBySubstate0BFE
; @run 71:CA64
; WHAT IT DOES: Runs the step of the object's substate: 0 LaunchByStep0BFE, 1 LandOrClearLow0BFE, 2
; FlickerThenBecomeL0BFE (Stage 07's overlay).
; HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LaunchLandBySubstate0BFESteps = $CA76 ; the routine for each substate, low and high byte
LaunchLandBySubstate0BFE:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:LaunchLandBySubstate0BFESteps,y
    sta z:zTemp00
    lda a:LaunchLandBySubstate0BFESteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFE:313 ShotByState0BFE
; @run 71:CB13
; WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordWorkF8E9_0BFE, 1 StepDownOwnerChain0BFE, 2
; ShotFromOwner0BFE, 3 FlyUntilOutside0BFE (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ShotByState0BFESteps = $CB25 ; the routine for each state, low and high byte
ShotByState0BFE:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ShotByState0BFESteps,y
    sta z:zTemp00
    lda a:ShotByState0BFESteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFE:422 FaceHitThenByState0BFE
; @run 71:CC22
; WHAT IT DOES: Steps OBJ_WORK_G on, faces the player in the states that do so, takes state 6 when hit, then runs
; the step of its state: 0 StartKind47Next0BFE, 1 LandThenNearPlayer0BFE, 2 ListThenBoxOn0BFE, 3
; AdvanceUntilNear0BFE, 4 BackOffUntilFar0BFE, 5 AttackBySubstate0BFE, 6 WakeTurnBySubstate0BFE (Stage 07's
; overlay).
; HOW: INC; FacePlayerInStates0BFE, State6SoundWhenHit0BFE; twice the state indexes a word table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FaceHitThenByState0BFESteps = $CC3D ; the routine for each state, low and high byte
FaceHitThenByState0BFE:
    inc a:OBJ_WORK_G,x
    jsr FacePlayerInStates0BFE
    jsr State6SoundWhenHit0BFE
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FaceHitThenByState0BFESteps,y
    sta z:zTemp00
    lda a:FaceHitThenByState0BFESteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFE:4C9 AdvanceUntilNear0BFE
; @run 71:CCC9
; WHAT IT DOES: Steps while landed with OBJ_WORK_E its facing; within $54 of the player along X goes to state 4,
; otherwise goes on to DodgeOrCountToState5_0BFE (Stage 07's overlay).
; HOW: StepWhileLanded0BFE, DistanceXToPlayer and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AdvanceUntilNear0BFE:
    jsr StepWhileLanded0BFE
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$54
    bcc AdvanceUntilNear0BFE_at0
    jmp DodgeOrCountToState5_0BFE
AdvanceUntilNear0BFE_at0:
    lda #$04
    sta a:OBJ_STATE,x
    rts

; @routine 0BFE:50F DodgeOrCountToState5_0BFE
; @run 71:CD0F
; WHAT IT DOES: When the player is in state 3 and the sign of his X speed equals the object's OBJ_FACING_LEFT,
; restarts its animation in state 5, substate 8; otherwise counts OBJ_WORK_H down and at 0 restarts it in state 5,
; substate 0 (Stage 07's overlay).
; HOW: Slot 0's state and X speed bit 7 rolled to bit 0; a compare; DEC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
DodgeOrCountToState5_0BFE:
    lda a:OBJ_STATE
    cmp #$03
    bne DodgeOrCountToState5_0BFE_at0
    lda a:OBJ_VX_HI
    rol a
    rol a
    and #$01
    cmp a:OBJ_FACING_LEFT,x
    bne DodgeOrCountToState5_0BFE_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
    lda #$08
    sta a:OBJ_SUBSTATE,x
    rts
DodgeOrCountToState5_0BFE_at0:
    dec a:OBJ_WORK_H,x
    bne DodgeOrCountToState5_0BFE_at1
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
DodgeOrCountToState5_0BFE_at1:
    rts

; @routine 0BFE:547 BackOffUntilFar0BFE
; @run 71:CD47
; WHAT IT DOES: Steps while landed with OBJ_WORK_E the opposite of its facing; at $6C or more from the player along
; X goes to state 3, otherwise goes on to DodgeOrCountToState5_0BFE (Stage 07's overlay).
; HOW: StepWhileLanded0BFE, EOR #1, DistanceXToPlayer and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BackOffUntilFar0BFE:
    jsr StepWhileLanded0BFE
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$6C
    bcs BackOffUntilFar0BFE_at0
    jmp DodgeOrCountToState5_0BFE
BackOffUntilFar0BFE_at0:
    lda #$03
    sta a:OBJ_STATE,x
    rts

; @routine 0BFE:562 AttackBySubstate0BFE
; @run 71:CD62
; WHAT IT DOES: Runs the step of the object's substate: 0 PickPhaseOfFour0BFE, 1 SetFrame89Wait28_0BFE, 2
; HoldFrame89ThenLeapBack0BFE, 3 BackSlideListA0BFE, 4 BackSlideListB0BFE, 5 JumpAtPlayer0BFE, 6
; FrameBySignLand0BFE, 7 and $0A ListThenState3_0BFE, 8 HopAtPlayer0BFE, 9 LandFrontColumn0BFE (Stage 07's
; overlay).
; HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AttackBySubstate0BFESteps = $CD74 ; the routine for each substate, low and high byte
AttackBySubstate0BFE:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:AttackBySubstate0BFESteps,y
    sta z:zTemp00
    lda a:AttackBySubstate0BFESteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFE:5B6 HoldFrame89ThenLeapBack0BFE
; @run 71:CDB6
; WHAT IT DOES: Shows frame $89, lands in place and counts OBJ_WORK_D down; within $30 of the player takes a speed
; away from him by its facing into OBJ_WORK_I:OBJ_WORK_J and substate 3 or 4 by bit 1 of its Y; at 0 goes to state
; 3, substate 0 (Stage 07's overlay).
; HOW: SlideByFacing0BFE, the X speed cleared, DEC, DistanceXToPlayer; a word table by twice the facing.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HoldFrame89ThenLeapBack0BFESpeeds = $CDF7 ; the X speed for each facing, low byte (OBJ_WORK_J) then high (OBJ_WORK_I): -$2C0, +$2C0
HoldFrame89ThenLeapBack0BFE:
    lda #$89
    sta a:OBJ_FRAME,x
    jsr SlideByFacing0BFE
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    dec a:OBJ_WORK_D,x
    beq HoldFrame89ThenLeapBack0BFE_at0
    jsr VecDistanceXToPlayer
    cmp #$30
    bcs HoldFrame89ThenLeapBack0BFE_at1
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:HoldFrame89ThenLeapBack0BFESpeeds+1,y
    sta a:OBJ_WORK_I,x
    lda a:HoldFrame89ThenLeapBack0BFESpeeds,y
    sta a:OBJ_WORK_J,x
    lda a:OBJ_Y_LO,x
    lsr a
    and #$01
    clc
    adc #$03
    sta a:OBJ_SUBSTATE,x
HoldFrame89ThenLeapBack0BFE_at1:
    rts
HoldFrame89ThenLeapBack0BFE_at0:
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 0BFE:612 BackSlideListA0BFE
; @run 71:CE12
; WHAT IT DOES: Slides and slows (SlideAndSlow0BFE) and plays the frame list at $CE2B; at its end goes to state 4,
; substate 0 (Stage 07's overlay).
; HOW: The call, StepObjectFrames, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BackSlideListA0BFE:
    jsr SlideAndSlow0BFE
    lda #$2B
    sta z:zTemp10
    lda #$CE
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne BackSlideListA0BFE_at0
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
BackSlideListA0BFE_at0:
    rts

; @routine 0BFE:66A BackSlideListB0BFE
; @run 71:CE6A
; WHAT IT DOES: Slides and slows (SlideAndSlow0BFE) and plays the frame list at $CE83; at its end goes to state 4,
; substate 0 (Stage 07's overlay).
; HOW: The call, StepObjectFrames, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BackSlideListB0BFE:
    jsr SlideAndSlow0BFE
    lda #$83
    sta z:zTemp10
    lda #$CE
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne BackSlideListB0BFE_at0
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
BackSlideListB0BFE_at0:
    rts

; @routine 0BFE:76B WakeTurnBySubstate0BFE
; @run 71:CF6B
; WHAT IT DOES: Runs the step of the object's substate: 0 HitBoxOffWait40_0BFE, 1 LandListThenWakeNext0BFE, 2
; TurnDownTimed0BFE, 3 TurnUpTimed0BFE (Stage 07's overlay).
; HOW: Twice the substate indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WakeTurnBySubstate0BFESteps = $CF7D ; the routine for each substate, low and high byte
WakeTurnBySubstate0BFE:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:WakeTurnBySubstate0BFESteps,y
    sta z:zTemp00
    lda a:WakeTurnBySubstate0BFESteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFF:084 LoadRecordFrameByArg0BFF
; @run 71:D084
; WHAT IT DOES: In state 0 loads the object's record and sets its frame to OBJ_START_ARG and its sprite set to 6
; (Stage 07's overlay).
; HOW: A test of the state, LoadObjectRecord, two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordFrameByArg0BFF:
    lda a:OBJ_STATE,x
    bne LoadRecordFrameByArg0BFF_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
LoadRecordFrameByArg0BFF_at0:
    rts

; @routine 0BFF:098 BreakWallByState0BFF
; @run 71:D098
; WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordDrawTiles09_0BFF, 1 BreakWallOnHit0BFF (Stage 07's
; overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BreakWallByState0BFFSteps = $D0AA ; the routine for each state, low and high byte
BreakWallByState0BFF:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:BreakWallByState0BFFSteps,y
    sta z:zTemp00
    lda a:BreakWallByState0BFFSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFF:10C LoadRecordInState0_0BFF
; @run 71:D10C
; WHAT IT DOES: In state 0 loads the object's record (Stage 07's overlay).
; HOW: A test of the state, LoadObjectRecord.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordInState0_0BFF:
    lda a:OBJ_STATE,x
    bne LoadRecordInState0_0BFF_at0
    jsr VecLoadObjectRecord
LoadRecordInState0_0BFF_at0:
    rts

; @routine 0BFF:28A StoodOnThenSink0BFF
; @run 71:D28A
; WHAT IT DOES: In state 0 loads its record with sprite set 6 and frame $11; in state 1, once the player stands on
; it (StandOnUnlessSide2_0BFF), sets OBJ_WORK_D $20, records its event and steps on; in state 2 counts OBJ_WORK_D
; down to the next state; in state 3 speeds down by $14 a frame until its Y low is $BA, then moves at Y speed $40;
; in state 4, from Y low $C2, removes itself when its Y high is not 0, else flips its frame between $11 and 0; from
; state 1 on it keeps the player's contact each frame (Stage 07's overlay).
; HOW: A chain of DEC tests of the state; RecordObjectEvent; a 16-bit add to Y speed; ApplyVelocity; ends in
; ClearCarry0BFF.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StoodOnThenSink0BFF:
    lda a:OBJ_STATE,x
    bne StoodOnThenSink0BFF_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$11
    sta a:OBJ_FRAME,x
StoodOnThenSink0BFF_at2:
    rts
StoodOnThenSink0BFF_at0:
    dec a
    bne StoodOnThenSink0BFF_at1
    jsr StandOnUnlessSide2_0BFF
    bcc StoodOnThenSink0BFF_at2
    lda #$20
    sta a:OBJ_WORK_D,x
    jsr VecRecordObjectEvent
    inc a:OBJ_STATE,x
    rts
StoodOnThenSink0BFF_at1:
    dec a
    bne StoodOnThenSink0BFF_at3
    jsr StandOnUnlessSide2_0BFF
    dec a:OBJ_WORK_D,x
    bne StoodOnThenSink0BFF_at2
    inc a:OBJ_STATE,x
    rts
StoodOnThenSink0BFF_at3:
    dec a
    bne StoodOnThenSink0BFF_at4
    jsr StandOnUnlessSide2_0BFF
    lda a:OBJ_VY_LO,x
StoodOnThenSink0BFF_at7:
    clc
    adc #$14
    sta a:OBJ_VY_LO,x
    bcc StoodOnThenSink0BFF_at5
    inc a:OBJ_VY_HI,x
StoodOnThenSink0BFF_at5:
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$BA
    bcc StoodOnThenSink0BFF_at2
    inc a:OBJ_STATE,x
    lda #$40
    sta a:OBJ_VY_LO,x
    stz a:OBJ_VY_HI,x
    rts
StoodOnThenSink0BFF_at4:
    jsr StandOnUnlessSide2_0BFF
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$C2
    bcc StoodOnThenSink0BFF_at2
    lda a:OBJ_Y_HI,x
    beq StoodOnThenSink0BFF_at6
    jmp VecClearObject
StoodOnThenSink0BFF_at6:
    lda a:OBJ_FRAME,x
    eor #$11
    sta a:OBJ_FRAME,x

; @routine 0BFF:451 SecretWallByState0BFF
; @run 71:D451
; WHAT IT DOES: Runs the step of the object's state: 0 LoadRecordScreenY0BFF, 1 SecretWallOnHit0BFF, 2
; StateOnWhenField85Is2_0BFF, 3 StateOnAndMarkField8C0BFF, 4 StateOnUnlessField8CSet0BFF, 5 steps the state on, 6
; does nothing (Stage 07's overlay).
; HOW: Twice the state indexes a word table; a jump through zTemp00/01.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SecretWallByState0BFFSteps = $D463 ; the routine for each state, low and high byte
SecretWallByState0BFF:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SecretWallByState0BFFSteps,y
    sta z:zTemp00
    lda a:SecretWallByState0BFFSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFF:4B7 StateOnWhenField85Is2_0BFF
; @run 71:D4B7
; WHAT IT DOES: Steps the object's state on when zField85 is 2 (Stage 07's overlay).
; HOW: A compare, then INC or a jump to the RTS at $D4CA.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StateOnWhenField85Is2_0BFF:
    lda z:zField85
    cmp #$02
    bne ReturnFromStep0BFF
    inc a:OBJ_STATE,x
    rts

; @routine 0BFF:4CA ReturnFromStep0BFF
; @run 71:D4CA
; WHAT IT DOES: Returns at once: the RTS that ends StateOnAndMarkField8C0BFF, which the steps beside it jump to
; when they do nothing (Stage 07's overlay).
; HOW: RTS.
; WAITS: no.
; Entry: nothing. Exit: A, X, Y and the flags as its last instruction or call left them.
ReturnFromStep0BFF:
    rts

; @routine 0BFF:4CB StateOnUnlessField8CSet0BFF
; @run 71:D4CB
; WHAT IT DOES: Steps the object's state on unless zField8C bit 7 is set (Stage 07's overlay).
; HOW: A test of N, then INC or a jump to the RTS at $D4CA.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StateOnUnlessField8CSet0BFF:
    lda z:zField8C
    bmi ReturnFromStep0BFF
    inc a:OBJ_STATE,x
    rts

; @routine 0BFF:4D3 AnimateTilesByScroll0BFF
; @run 71:D4D3
; WHAT IT DOES: In state 0 loads its record; each frame takes scroll layer 1's X less this frame's step, and copies
; the VRAM rows picked by its high byte, rounded by the low byte's bit 7, plus 8 and held to 0 to $1F
; (CopyEightVramRows0BFF), so the tiles follow the scroll (Stage 07's overlay).
; HOW: A 16-bit subtract of ScrollStepX from the scroll's X (index 1), ASL and ADC #8, AND #$1F, the call.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
AnimateTilesByScroll0BFF:
    lda a:OBJ_STATE,x
    bne AnimateTilesByScroll0BFF_at0
    jsr VecLoadObjectRecord
AnimateTilesByScroll0BFF_at0:
    lda a:ScrollXLo-7
    sec
    sbc a:ScrollStepXHi-7
    sta z:zTemp00
    lda a:ScrollXHi-7
    sbc a:ScrollYHiScreen-7
    sta z:zTemp01
    asl z:zTemp00
    lda z:zTemp01
    adc #$08
    and #$1F
    jsr CopyEightVramRows0BFF
    rts

; @routine 0BFF:621 WaitPlayerYThenFade0BFF
; @run 71:D621
; WHAT IT DOES: In state 0 loads its record with sprite set 6 and frame $29; in state 1, once the player's Y low is
; $AA, calls the CD fade ($E02D) with $0C and steps on; in state 2 counts OBJ_WORK_D down by 2 and at 0 sets
; zModeEvents bit 1 and zFieldA3 to $0E (Stage 07's overlay).
; HOW: DEC tests of the state; vec_cd_fade with X kept on the stack; SMB1.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WaitPlayerYThenFade0BFF:
    lda a:OBJ_STATE,x
    bne WaitPlayerYThenFade0BFF_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$29
    sta a:OBJ_FRAME,x
WaitPlayerYThenFade0BFF_at2:
    rts
WaitPlayerYThenFade0BFF_at0:
    dec a
    bne WaitPlayerYThenFade0BFF_at1
    lda a:OBJ_Y_LO
    cmp #$AA
    bne WaitPlayerYThenFade0BFF_at2
    phx
    lda #$0C
    jsr BiosCdFade
    plx
    inc a:OBJ_STATE,x
    rts
WaitPlayerYThenFade0BFF_at1:
    dec a:OBJ_WORK_D,x
    dec a:OBJ_WORK_D,x
    bne WaitPlayerYThenFade0BFF_at2
    smb1 z:zModeEvents
    lda #$0E
    sta z:zFieldA3
    rts
