; @routine 088A:703 FrameLandCall088A
; @run 70:AF03
; WHAT IT DOES: Frame $0A; on landing runs $B784, counts the state on and runs $AF1E.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FrameLandCall088A:
    lda #$0A
    sta a:OBJ_FRAME,x
    lda #$10
    sta a:OBJ_WORK_F,x
    lda #$04
    jsr VecStopOnLanding
    bcc FrameLandCall088A_at0
    jsr ClearStageTimers
    inc a:OBJ_STATE,x
    jsr PlaySoundTwo
FrameLandCall088A_at0:
    rts

; @routine 088B:791 TurnOrClearSlots12to15_088B
; @run 70:B791
; WHAT IT DOES: Of slots $12-$15, those with OBJ_WORK_D 2 become kind $9C, the rest are cleared.
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: as the last call, compare or load left them.
TurnOrClearSlots12to15_088B:
    phx
    ldx #$12
TurnOrClearSlots12to15_088B_at2:
    lda a:OBJ_WORK_D,x
    cmp #$02
    bne TurnOrClearSlots12to15_088B_at0
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    bra TurnOrClearSlots12to15_088B_at1
TurnOrClearSlots12to15_088B_at0:
    jsr VecClearObject
TurnOrClearSlots12to15_088B_at1:
    inx
    cpx #$16
    bcc TurnOrClearSlots12to15_088B_at2
    plx
    rts

; @routine 0E0B:6CD KeepLastPointLandSubstate0E0B
; @run 70:B6CD
; WHAT IT DOES: Sets PlayerKeepsLastPoint, stops on landing, then the substate's handler.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
KeepLastPointLandSubstate0E0BSubstates = $B6E9 ; the handlers by substate
KeepLastPointLandSubstate0E0B:
    lda #$01
    sta a:PlayerKeepsLastPoint
    lda #$04
    jsr VecStopOnLanding
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:KeepLastPointLandSubstate0E0BSubstates,y
    sta z:zTemp00
    lda a:KeepLastPointLandSubstate0E0BSubstates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0E0B:795 StartFifteenKind2B_0E0B
; @run 70:B795
; WHAT IT DOES: Starts numbered kind $2B objects facing the same way in slots $20-$2E.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartFifteenKind2B_0E0B:
    ldy #$20
    stz z:zTemp00
StartFifteenKind2B_0E0B_at0:
    jsr VecClearObjectInY
    lda #$2B
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    inc z:zTemp00
    iny
    cpy #$2F
    bcc StartFifteenKind2B_0E0B_at0
    rts

; @routine 0EBD:268 CountThenRecordClear0EBD
; @run 71:C268
; WHAT IT DOES: Runs $B22B; when OBJ_WORK_D runs out, records the event and clears.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenRecordClear0EBDStep = $B22B ; a routine of this overlay
CountThenRecordClear0EBD:
    jsr CountThenRecordClear0EBDStep
    lda a:OBJ_WORK_D,x
    beq CountThenRecordClear0EBD_at0
    dec a:OBJ_WORK_D,x
    bne CountThenRecordClear0EBD_at0
    jsr VecRecordObjectEvent
    jmp VecClearObject
CountThenRecordClear0EBD_at0:
    rts

; @routine 0EBD:547 ListThenResetOther0EBD
; @run 71:C547
; WHAT IT DOES: At the end of the list at $C566 clears the start argument of the slot in OBJ_WORK_F; OBJ_WORK_G $80, state 1.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenResetOther0EBD:
    lda #$66
    sta z:zTemp10
    lda #$C5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenResetOther0EBD_at0
    ldy a:OBJ_WORK_F,x
    cla
    sta a:OBJ_START_ARG,y
    lda #$80
    sta a:OBJ_WORK_G,x
    lda #$01
    sta a:OBJ_STATE,x
ListThenResetOther0EBD_at0:
    rts

; @routine 0EBE:371 AnimateThenDrift0EBE
; @run 71:CB71
; WHAT IT DOES: Faces the player and animates; with carry set from that, moves across at the speed OBJ_WORK_E picks.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AnimateThenDrift0EBESpeeds = $CB84 ; vx high byte by OBJ_WORK_E
AnimateThenDrift0EBE:
    cly
    jsr FaceAndAnimate0EBE
    bcc AnimateThenDrift0EBE_at0
    ldy a:OBJ_WORK_E,x
    lda a:AnimateThenDrift0EBESpeeds,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
AnimateThenDrift0EBE_at0:
    rts

; @routine 0EBE:4F2 AnimateThenStateThree0EBE
; @run 71:CCF2
; WHAT IT DOES: Runs the second frame list; at its end state 3 from the start.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AnimateThenStateThree0EBE:
    ldy #$06
    jsr FaceAndAnimate0EBE
    bne AnimateThenStateThree0EBE_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
AnimateThenStateThree0EBE_at0:
    rts

; @routine 0EBF:444 WaitFrameListThenState0EBF
; @run 71:D444
; WHAT IT DOES: Animates for $1C counts, then state 2.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WaitFrameListThenState0EBF:
    lda a:OBJ_SUBSTATE,x
    bne WaitFrameListThenState0EBF_at0
    lda #$1C
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
    rts
WaitFrameListThenState0EBF_at0:
    ldy #$02
    jsr FaceAndAnimate0EBF
    dec a:OBJ_WORK_D,x
    bne WaitFrameListThenState0EBF_at1
    lda #$02
    sta a:OBJ_STATE,x
WaitFrameListThenState0EBF_at1:
    rts

; @routine 101C:5DF FollowSlotZeroFor101C
; @run 70:BDDF
; WHAT IT DOES: Stays by slot 0 for OBJ_WORK_D counts; cleared then, or in game sub-mode 4.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FollowSlotZeroFor101C:
    lda z:zGameSubMode
    cmp #$04
    bne FollowSlotZeroFor101C_at0
    jmp VecClearObject
FollowSlotZeroFor101C_at0:
    ldy #$00
    jsr VecCopyObjectPlaceBack
    jsr VecApplyVelocity
    dec a:OBJ_WORK_D,x
    bne FollowSlotZeroFor101C_at1
    jmp VecClearObject
FollowSlotZeroFor101C_at1:
    rts

; @routine 101D:4B0 ListThenHittable101D
; @run 71:C4B0
; WHAT IT DOES: Runs the list at $C4C9; at its end the object is hit-tested and the state counts on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenHittable101D:
    lda #$C9
    sta z:zTemp10
    lda #$C4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenHittable101D-1
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 101F:14A KeepArgsThenClear101F
; @run 71:D14A
; WHAT IT DOES: Fills 64 bytes at $7E00 with the start argument (a non-zero one also flips slot 0), records the event and clears.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
KeepArgsThenClear101FBuffer = $7E00 ; 64 bytes filled
KeepArgsThenClear101F:
    lda a:OBJ_START_ARG,x
    beq KeepArgsThenClear101F_at0
    lda #$C0
    sta a:OBJ_SPRITE_FLIP
KeepArgsThenClear101F_at0:
    cly
    lda a:OBJ_START_ARG,x
KeepArgsThenClear101F_at1:
    sta a:KeepArgsThenClear101FBuffer,y
    iny
    cpy #$40
    bne KeepArgsThenClear101F_at1
    jsr VecRecordObjectEvent
    jmp VecClearObject

; @routine 101F:652 ChannelScriptThenClear101F
; @run 71:D652
; WHAT IT DOES: Once zField92 is set, or in game sub-mode 0: list channel $88, script $82, and the object goes.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChannelScriptThenClear101F:
    lda z:zField92
    bne ChannelScriptThenClear101F_at0
    lda z:zGameSubMode
    beq ChannelScriptThenClear101F_at0
    rts
ChannelScriptThenClear101F_at0:
    lda #$88
    phx
    ldx #$06
    jsr VecStartListChannelKind2
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    jmp VecClearObject

; @routine 10CB:304 ListThenSound46_10CB
; @run 70:B304
; WHAT IT DOES: Runs $B383 and the list at $B320; at its end sound $46, OBJ_WORK_F cleared, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenSound46_10CB:
    jsr ScriptByFrameMatch10CB
    lda #$20
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenSound46_10CB_at0
    lda #$46
    jsr VecSoundCommand
    stz a:OBJ_WORK_F,x
    inc a:OBJ_SUBSTATE,x
ListThenSound46_10CB_at0:
    rts

; @routine 10CD:439 AnimateByWorkF10CD
; @run 71:C439
; WHAT IT DOES: Runs the frame list OBJ_WORK_F picks, in sprite set 6.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AnimateByWorkF10CDLists = $C4C1 ; frame lists by OBJ_WORK_F
AnimateByWorkF10CD:
    lda a:OBJ_WORK_F,x
    asl a
    tay
    lda a:AnimateByWorkF10CDLists,y
    sta z:zTemp10
    lda a:AnimateByWorkF10CDLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    rts

; @routine 117B:330 PointCallThenCount117B
; @run 70:B330
; WHAT IT DOES: Calls $AA35 with the point $1C,$1A, takes bits 1-2 off the player's push, and after OBJ_WORK_D counts the state goes on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PointCallThenCount117B:
    lda #$1C
    sta z:zTemp08
    lda #$1A
    sta z:zTemp09
    jsr ContactWithPlayer
    lda z:zPlayerPush
    and #$F9
    sta z:zPlayerPush
    dec a:OBJ_WORK_D,x
    bne PointCallThenCount117B_at0
    inc a:OBJ_STATE,x
PointCallThenCount117B_at0:
    rts

; @routine 117F:43E TurnToAttack117F
; @run 71:D43E
; WHAT IT DOES: Looks ahead (in OBJ_WORK_F the way to go); with carry clear and the player nearer than $50 across, state 3.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnToAttack117F:
    jsr ListThenProbe117F
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_F,x
    jsr CountOnScreenToState4_117F
    bcs TurnToAttack117F_at0
    jsr VecDistanceXToPlayer
    cmp #$50
    bcs TurnToAttack117F_at0
    lda #$03
    sta a:OBJ_STATE,x
TurnToAttack117F_at0:
    rts

; @routine 117F:49F TurnToBackOff117F
; @run 71:D49F
; WHAT IT DOES: Looks ahead (in OBJ_WORK_F the way to go); with carry clear and the player at least $78 across, state 2.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnToBackOff117F:
    jsr ListThenProbe117F
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_F,x
    jsr CountOnScreenToState4_117F
    bcs TurnToBackOff117F_at0
    jsr VecDistanceXToPlayer
    cmp #$78
    bcc TurnToBackOff117F_at0
    lda #$02
    sta a:OBJ_STATE,x
TurnToBackOff117F_at0:
    rts

; @routine 12DC:3F4 FallToFloorA4_12DC
; @run 70:BBF4
; WHAT IT DOES: Falls with gravity; at Y $A4 it stops there, carry set.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallToFloorA4_12DC:
    lda #$04
    sta z:zProduct1
    lda #$22
    jsr VecProbeTerrain
    jsr VecApplyVelocity
    jsr VecApplyGravity
    lda a:OBJ_Y_LO,x
    cmp #$A4
    bcc FallToFloorA4_12DC_at0
    lda #$A4
    sta a:OBJ_Y_LO,x
    sec
    rts
FallToFloorA4_12DC_at0:
    clc
    rts

; @routine 12DD:037 StateWhileOwner12DD
; @run 71:C037
; WHAT IT DOES: While slot $10 holds kind $2C the state's handler runs; else the object goes.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StateWhileOwner12DDStates = $C053 ; the handlers by state
StateWhileOwner12DD:
    lda a:OBJ_KIND+$10
    cmp #$2C
    beq StateWhileOwner12DD_at0
    jmp VecClearObject
StateWhileOwner12DD_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:StateWhileOwner12DDStates,y
    sta z:zTemp00
    lda a:StateWhileOwner12DDStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DE:15A DriftThenPickBySide12DE
; @run 71:C95A
; WHAT IT DOES: Moves across for OBJ_WORK_E counts, then picks OBJ_WORK_H by the player's side and the state counts on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DriftThenPickBySide12DEValues = $C8F0 ; OBJ_WORK_H by side
DriftThenPickBySide12DE:
    jsr VecApplyVelocityX
    dec a:OBJ_WORK_E,x
    bne DriftThenPickBySide12DE_at0
    jsr VecSideOfPlayer
    lsr a
    and #$01
    tay
    lda a:DriftThenPickBySide12DEValues,y
    sta a:OBJ_WORK_H,x
    inc a:OBJ_STATE,x
DriftThenPickBySide12DE_at0:
    rts

; @routine 12DE:2F4 FallToFloor93_12DE
; @run 71:CAF4
; WHAT IT DOES: Falls with gravity; at Y $93 it stops there, carry set.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallToFloor93_12DE:
    lda #$0A
    sta z:zProduct1
    lda #$22
    jsr VecProbeTerrain
    jsr VecApplyVelocity
    jsr VecApplyGravity
    lda a:OBJ_Y_LO,x
    cmp #$93
    bcc FallToFloor93_12DE_at0
    lda #$93
    sta a:OBJ_Y_LO,x
    sec
    rts
FallToFloor93_12DE_at0:
    clc
    rts

; @routine 13FB:6AB ListCallThenSubstate13FB
; @run 70:B6AB
; WHAT IT DOES: Runs the list at $B6C5 and $B6CC; carry from it counts the substate on with the animation from the start.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListCallThenSubstate13FB:
    lda #$C5
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr FallProbeToA8_13FB
    bcc ListCallThenSubstate13FB_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
ListCallThenSubstate13FB_at0:
    rts

; @routine 161C:007 AnimateThenDrift161C
; @run 70:B807
; WHAT IT DOES: Faces the player and animates; with carry set from that, moves across at the speed OBJ_WORK_E picks.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AnimateThenDrift161CSpeeds = $B81A ; vx high byte by OBJ_WORK_E
AnimateThenDrift161C:
    cly
    jsr FaceAndAnimate161C
    bcc AnimateThenDrift161C_at0
    ldy a:OBJ_WORK_E,x
    lda a:AnimateThenDrift161CSpeeds,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
AnimateThenDrift161C_at0:
    rts

; @routine 161C:188 AnimateThenStateThree161C
; @run 70:B988
; WHAT IT DOES: Runs the second frame list; at its end state 3 from the start.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AnimateThenStateThree161C:
    ldy #$06
    jsr FaceAndAnimate161C
    bne AnimateThenStateThree161C_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
AnimateThenStateThree161C_at0:
    rts

; @routine 161D:654 TurnToAttack161D
; @run 71:C654
; WHAT IT DOES: Looks ahead (in OBJ_WORK_E the way to go); with carry clear and the player nearer than $4C across, state 3.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnToAttack161D:
    jsr ListSpeedProbe161D
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_E,x
    jsr CountThenState4_161D
    bcs TurnToAttack161D_at0
    jsr VecDistanceXToPlayer
    cmp #$4C
    bcs TurnToAttack161D_at0
    lda #$03
    sta a:OBJ_STATE,x
TurnToAttack161D_at0:
    rts

; @routine 161D:6CA TurnToBackOff161D
; @run 71:C6CA
; WHAT IT DOES: Looks ahead (in OBJ_WORK_E the way to go); with carry clear and the player at least $74 across, state 2.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
TurnToBackOff161D:
    jsr ListSpeedProbe161D
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_E,x
    jsr CountThenState4_161D
    bcs TurnToBackOff161D_at0
    jsr VecDistanceXToPlayer
    cmp #$74
    bcc TurnToBackOff161D_at0
    lda #$02
    sta a:OBJ_STATE,x
TurnToBackOff161D_at0:
    rts

; @routine 16CB:14B CountHitsThenState16CB
; @run 70:B14B
; WHAT IT DOES: Counts OBJ_WORK_J, runs $B175, $B195 and $B1F4, then the state's handler.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountHitsThenState16CBStates = $B169 ; the handlers by state
CountHitsThenState16CB:
    inc a:OBJ_WORK_J,x
    jsr CountThenSpawnInState2
    jsr ShudderWhenHit16CB
    jsr StateFiveOnStatusBit7
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CountHitsThenState16CBStates,y
    sta z:zTemp00
    lda a:CountHitsThenState16CBStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CC:23C StateWhileOwner16CC
; @run 70:BA3C
; WHAT IT DOES: While slot $16 holds kind $23 the state's handler runs; else the object goes.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StateWhileOwner16CCStates = $BA58 ; the handlers by state
StateWhileOwner16CC:
    lda a:OBJ_KIND+$16
    cmp #$23
    beq StateWhileOwner16CC_at0
    jmp VecClearObject
StateWhileOwner16CC_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:StateWhileOwner16CCStates,y
    sta z:zTemp00
    lda a:StateWhileOwner16CCStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)
