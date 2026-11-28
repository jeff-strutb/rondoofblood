; @routine 05CB:734 ListThenWait05CB
; @run 70:B734
; WHAT IT DOES: Runs $B706 and the list at $B74D; at its end a wait of $F0, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenWait05CB:
    jsr SinkToB8_05CB
    lda #$4D
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenWait05CB_at0
    lda #$F0
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
ListThenWait05CB_at0:
    rts

; @routine 05CC:4D8 LoadFrameByArg05CC
; @run 70:BCD8
; WHAT IT DOES: In state 0 loads, with the start argument as the frame, in sprite set 6.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFrameByArg05CC:
    lda a:OBJ_STATE,x
    bne LoadFrameByArg05CC_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
LoadFrameByArg05CC_at0:
    rts

; @routine 088B:4DD FallThenLandCall088B
; @run 70:B4DD
; WHAT IT DOES: Moves and falls; above Y $18, or no longer rising, runs $B784 and the substate counts on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallThenLandCall088B:
    jsr VecApplyVelocity
    jsr VecApplyGravity
    lda a:OBJ_Y_LO,x
    cmp #$18
    bcc FallThenLandCall088B_at0
    lda a:OBJ_VY_HI,x
    bmi FallThenLandCall088B_at1
FallThenLandCall088B_at0:
    jsr ClearStageTimers
    inc a:OBJ_SUBSTATE,x
FallThenLandCall088B_at1:
    rts

; @routine 088C:3E6 LoadScriptInStageFiveClear088C
; @run 70:BBE6
; WHAT IT DOES: Loads and clears the object, with script $92 first when the stage asked for is 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadScriptInStageFiveClear088C:
    jsr VecLoadObjectRecord
    lda z:zStageRequested
    cmp #$05
    bne LoadScriptInStageFiveClear088C_at0
    phx
    ldx #$05
    lda #$92
    jsr VecStartScriptSlotKind1
    plx
LoadScriptInStageFiveClear088C_at0:
    jmp VecClearObject

; @routine 09EC:23B LoadByPieceSlot09EC
; @run 70:BA3B
; WHAT IT DOES: Loads; OBJ_WORK_D by the slot counted from $20, frame $38 on from it.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadByPieceSlot09ECValues = $BA51 ; OBJ_WORK_D by slot
LoadByPieceSlot09EC:
    jsr VecLoadObjectRecord
    txa
    sec
    sbc #$20
    tay
    lda a:LoadByPieceSlot09ECValues,y
    sta a:OBJ_WORK_D,x
    tya
    clc
    adc #$38
    sta a:OBJ_FRAME,x
    rts

; @routine 09EC:31A LoadThenRideSlot10_09EC
; @run 70:BB1A
; WHAT IT DOES: Loads in state 0; after it stays at slot $10's place running the list at $BB33.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadThenRideSlot10_09EC:
    lda a:OBJ_STATE,x
    bne LoadThenRideSlot10_09EC_at0
    jsr VecLoadObjectRecord
    rts
LoadThenRideSlot10_09EC_at0:
    ldy #$10
    jsr VecCopyObjectPlaceBack
    lda #$33
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0B4C:6F3 ScriptByFrame0B4C
; @run 70:BEF3
; WHAT IT DOES: On frames $48-$4A starts that frame's script in slot 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ScriptByFrame0B4CScripts = $BF09 ; three script numbers
ScriptByFrame0B4C:
    lda a:OBJ_FRAME,x
    sec
    sbc #$48
    cmp #$03
    bcs ScriptByFrame0B4C_at0
    tay
    phx
    lda a:ScriptByFrame0B4CScripts,y
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
ScriptByFrame0B4C_at0:
    rts

; @routine 0E0C:56E StartSevenKind26_0E0C
; @run 70:BD6E
; WHAT IT DOES: Starts numbered kind $26 objects in slots $18-$1E.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartSevenKind26_0E0C:
    ldy #$18
    stz z:zTemp00
StartSevenKind26_0E0C_at0:
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    inc z:zTemp00
    iny
    cpy #$1F
    bcc StartSevenKind26_0E0C_at0
    rts

; @routine 0EBC:0BE CallWhenLowAndDue0EBC
; @run 70:B8BE
; WHAT IT DOES: With OBJ_WORK_G 0 and OBJ_WORK_F at least $B8 runs $B3E4.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CallWhenLowAndDue0EBC:
    lda a:OBJ_Y_LO,x
    cmp #$D8
    bcc CallWhenLowAndDue0EBC_at0
CallWhenLowAndDue0EBC_at0:
    lda a:OBJ_WORK_G,x
    bne CallWhenLowAndDue0EBC_at1
    lda a:OBJ_WORK_F,x
    cmp #$B8
    bcc CallWhenLowAndDue0EBC_at1
    jsr FallUpToSpeed7_0EBB
CallWhenLowAndDue0EBC_at1:
    rts

; @routine 0EBE:386 FaceAndAnimate0EBE
; @run 71:CB86
; WHAT IT DOES: Faces the player and runs that side's frame list.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FaceAndAnimate0EBELists = $CB9B ; the frame list by the side the player is on
FaceAndAnimate0EBE:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:FaceAndAnimate0EBELists,y
    sta z:zTemp10
    lda a:FaceAndAnimate0EBELists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0EBF:19C FaceAndAnimate0EBF
; @run 71:D19C
; WHAT IT DOES: Faces the player and runs that side's frame list.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FaceAndAnimate0EBFLists = $D1B1 ; the frame list by the side the player is on
FaceAndAnimate0EBF:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:FaceAndAnimate0EBFLists,y
    sta z:zTemp10
    lda a:FaceAndAnimate0EBFLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0EBF:57F ClearOneEmptyOf20to27_0EBF
; @run 71:D57F
; WHAT IT DOES: Finds an empty slot in $20-$27 and clears it; carry set when there is one.
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: as the last call, compare or load left them.
ClearOneEmptyOf20to27_0EBF:
    ldy #$20
ClearOneEmptyOf20to27_0EBF_at1:
    lda a:OBJ_KIND,y
    beq ClearOneEmptyOf20to27_0EBF_at0
    iny
    cpy #$28
    bcc ClearOneEmptyOf20to27_0EBF_at1
    clc
    rts
ClearOneEmptyOf20to27_0EBF_at0:
    jsr VecClearObjectInY
    sec
    rts

; @routine 0F6C:7FD FaceCallStateTwelve0F6C
; @run 70:BFFD
; WHAT IT DOES: Faces the player, OBJ_WORK_D 1, runs $C35F and goes to state 12.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FaceCallStateTwelve0F6C:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$01
    sta a:OBJ_WORK_D,x
    jsr ClearAnimationThenStepListC585
    lda #$0C
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 101B:124 ListThenStateTwo101B
; @run 70:B124
; WHAT IT DOES: Runs the list at $B13A; at its end state 2.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenStateTwo101B:
    lda #$3A
    sta z:zTemp10
    lda #$B1
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenStateTwo101B_at0
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListThenStateTwo101B_at0:
    rts

; @routine 101B:281 StartKind9BOnBeat101B
; @run 70:B281
; WHAT IT DOES: When bits 1-3 of the wave counter are the start argument, starts kind $9B here in a free slot.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartKind9BOnBeat101B:
    lda z:zWaveCounter
    lsr a
    and #$07
    cmp a:OBJ_START_ARG,x
    bne StartKind9BOnBeat101B_at0
    jsr VecFindFreeSlotFrom30
    bcc StartKind9BOnBeat101B_at0
    lda #$9B
    jsr VecStartObjectAtPlace
StartKind9BOnBeat101B_at0:
    rts

; @routine 101B:7D8 RestartIntoStateTwo101B
; @run 70:B7D8
; WHAT IT DOES: Clears OBJ_WORK_E, runs $B73B and starts state 2 with its animation from the start.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RestartIntoStateTwo101B:
    stz a:OBJ_WORK_E,x
    jsr SpeedByPlayerSide101B
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    rts

; @routine 101C:6ED LandThenFrame74_101C
; @run 70:BEED
; WHAT IT DOES: On landing shows frame $74 for 6 counts, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandThenFrame74_101C:
    lda #$06
    jsr VecStopOnLanding
    bcc LandThenFrame74_101C_at0
    lda #$74
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
    lda #$06
    sta a:OBJ_ANIM_TIMER,x
LandThenFrame74_101C_at0:
    rts

; @routine 101F:698 WakeWhenNear101F
; @run 71:D698
; WHAT IT DOES: With the player nearer than $50 across, the state counts on; OBJ_WORK_D $80, sprite attribute $10.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WakeWhenNear101F:
    jsr VecDistanceXToPlayer
    cmp #$50
    bcs WakeWhenNear101F_at0
    inc a:OBJ_STATE,x
    lda #$80
    sta a:OBJ_WORK_D,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
WakeWhenNear101F_at0:
    rts

; @routine 10CC:4FD SoundHitOffWait10CC
; @run 70:BCFD
; WHAT IT DOES: Sound $E6, hitbox off, raster stage 3, a wait of $40 in OBJ_WORK_E, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundHitOffWait10CC:
    lda #$E6
    jsr VecSoundCommand
    stz a:OBJ_HITBOX_ON,x
    lda #$03
    sta z:zRasterStage
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 12DB:08D ClearUnlessScriptFive12DB
; @run 70:B08D
; WHAT IT DOES: Once script slot 3's kind is not 5: sets slot 0's OBJ_WORK_E and F, asks the mode (zModeEvents bit 0) and clears.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ClearUnlessScriptFive12DB:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq ClearUnlessScriptFive12DB_at0
    lda #$04
    sta a:OBJ_WORK_E
    lda #$10
    sta a:OBJ_WORK_F
    smb0 z:zModeEvents
    jmp VecClearObject
ClearUnlessScriptFive12DB_at0:
    rts

; @routine 13FC:04B ListThenSubstate13FC
; @run 70:B84B
; WHAT IT DOES: Runs $B6CC and the list at $B85F; at its end the substate counts on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenSubstate13FC:
    jsr FallProbeToA8_13FB
    lda #$5F
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenSubstate13FC_at0
    inc a:OBJ_SUBSTATE,x
ListThenSubstate13FC_at0:
    rts

; @routine 161C:01C FaceAndAnimate161C
; @run 70:B81C
; WHAT IT DOES: Faces the player and runs that side's frame list.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FaceAndAnimate161CLists = $B831 ; the frame list by the side the player is on
FaceAndAnimate161C:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:FaceAndAnimate161CLists,y
    sta z:zTemp10
    lda a:FaceAndAnimate161CLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 16CB:267 WaitThenScriptSound16CB
; @run 70:B267
; WHAT IT DOES: When OBJ_WORK_D runs out: script $82 in slot 5, substate on, sound $25.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WaitThenScriptSound16CB:
    dec a:OBJ_WORK_D,x
    bne WaitThenScriptSound16CB-1
    phx
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_SUBSTATE,x
    lda #$25
    jsr VecSoundCommand
    rts

; @routine 16CC:25E LoadByPieceSlot16CC
; @run 70:BA5E
; WHAT IT DOES: Loads; OBJ_WORK_E and the frame by the slot, counted from $11.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadByPieceSlot16CCWaits = $BA73 ; OBJ_WORK_E by slot
LoadByPieceSlot16CCFrames = $BA78 ; the frame by slot
LoadByPieceSlot16CC:
    jsr VecLoadObjectRecord
    txa
    sec
    sbc #$11
    tay
    lda a:LoadByPieceSlot16CCWaits,y
    sta a:OBJ_WORK_E,x
    lda a:LoadByPieceSlot16CCFrames,y
    sta a:OBJ_FRAME,x
    rts
