; @routine 05CC:01E EachOfPatterns05CC_01E
; @run 70:B81E
; WHAT IT DOES: Runs the routine at $B82D, then the one at $B858 for X 0 to 26, X restored.
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: X restored, N and Z from it; C set; else as the last call.
EachOfPatterns05CC_01EFirst = $B82D ; a routine of this overlay
EachOfPatterns05CC_01EEach = $B858 ; a routine of this overlay
EachOfPatterns05CC_01E:
    phx
    jsr EachOfPatterns05CC_01EFirst
    clx
EachOfPatterns05CC_01E_at0:
    jsr ErodePattern05CC058
    inx
    cpx #$1B
    bcc EachOfPatterns05CC_01E_at0
    plx
    rts

; @routine 072C:3C7 SpawnEveryFew072C_3C7
; @run 70:BBC7
; WHAT IT DOES: On some counts of OBJ_WORK_D, starts a kind $2E object at this place (secret stage 03's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnEveryFew072C_3C7:
    lda a:OBJ_WORK_D,x
    and #$07
    bne SpawnEveryFew072C_3C7_at0
    jsr VecFindFreeSlotFrom20
    bcc SpawnEveryFew072C_3C7_at0
    lda #$2E
    jsr VecStartObjectAtPlace
SpawnEveryFew072C_3C7_at0:
    rts

; @routine 072C:4E6 EachOfPatterns072C_4E6
; @run 70:BCE6
; WHAT IT DOES: Runs the routine at $BCF5, then the one at $BD20 for X 0 to 7, X restored.
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: X restored, N and Z from it; C set; else as the last call.
EachOfPatterns072C_4E6First = $BCF5 ; a routine of this overlay
EachOfPatterns072C_4E6Each = $BD20 ; a routine of this overlay
EachOfPatterns072C_4E6:
    phx
    jsr EachOfPatterns072C_4E6First
    clx
EachOfPatterns072C_4E6_at0:
    jsr ErodePattern072C520
    inx
    cpx #$08
    bcc EachOfPatterns072C_4E6_at0
    plx
    rts

; @routine 09EB:3C4 SpawnEveryFew09EB_3C4
; @run 70:B3C4
; WHAT IT DOES: On some counts of OBJ_WORK_G, starts a kind $30 object at this place (secret stage 05's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnEveryFew09EB_3C4:
    lda a:OBJ_WORK_G,x
    and #$3F
    bne SpawnEveryFew09EB_3C4_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnEveryFew09EB_3C4_at0
    lda #$30
    jsr VecStartObjectAtPlace
SpawnEveryFew09EB_3C4_at0:
    rts

; @routine 0B4D:115 LoadFrame4DWait0B4D_115
; @run 71:C115
; WHAT IT DOES: Loads the record, frame $4D, OBJ_WORK_D 192 (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFrame4DWait0B4D_115:
    jsr VecLoadObjectRecord
    lda #$4D
    sta a:OBJ_FRAME,x
    lda #$C0
    sta a:OBJ_WORK_D,x
    rts

; @routine 0B4D:164 EachOfPatterns0B4D_164
; @run 71:C164
; WHAT IT DOES: Runs the routine at $C173, then the one at $C19E for X 0 to 6, X restored.
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: X restored, N and Z from it; C set; else as the last call.
EachOfPatterns0B4D_164First = $C173 ; a routine of this overlay
EachOfPatterns0B4D_164Each = $C19E ; a routine of this overlay
EachOfPatterns0B4D_164:
    phx
    jsr EachOfPatterns0B4D_164First
    clx
EachOfPatterns0B4D_164_at0:
    jsr ErodePattern0B4D19E
    inx
    cpx #$07
    bcc EachOfPatterns0B4D_164_at0
    plx
    rts

; @routine 0CAB:759 SoundRestartSubstate0CAB_759
; @run 70:B759
; WHAT IT DOES: Sound $11, the animation restarted, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundRestartSubstate0CAB_759:
    lda #$11
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0CAC:059 ListThenStep0CAC_059
; @run 70:B859
; WHAT IT DOES: Plays the frame list at $B86A; when it ends, the substate stepped.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
ListThenStep0CAC_059:
    lda #$6A
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenStep0CAC_059_at0
    inc a:OBJ_SUBSTATE,x
ListThenStep0CAC_059_at0:
    rts

; @routine 0CAC:178 ListThenStep0CAC_178
; @run 70:B978
; WHAT IT DOES: Plays the frame list at $B989; when it ends, the substate stepped.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
ListThenStep0CAC_178:
    lda #$89
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenStep0CAC_178_at0
    inc a:OBJ_SUBSTATE,x
ListThenStep0CAC_178_at0:
    rts

; @routine 0CAC:21E SpawnWithSound0CAC
; @run 70:BA1E
; WHAT IT DOES: Sound $18 and a kind $2D object at this place, when a slot is free (secret stage 07's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnWithSound0CAC:
    jsr VecFindFreeSlotFrom20
    bcc SpawnWithSound0CAC_at0
    lda #$18
    jsr VecSoundCommand
    lda #$2D
    jsr VecStartObjectAtPlace
SpawnWithSound0CAC_at0:
    rts

; @routine 0EBB:083 LoadFrameByStep0EBB
; @run 70:B083
; WHAT IT DOES: Loads the record; the frame by the step through the stage, sprite set 6 (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFrameByStep0EBBFrames = $B094 ; the frame by zField82
LoadFrameByStep0EBB:
    jsr VecLoadObjectRecord
    ldy z:zField82
    lda a:LoadFrameByStep0EBBFrames,y
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    rts

; @routine 0EBD:536 ListThenStep0EBD_536
; @run 71:C536
; WHAT IT DOES: Plays the frame list at $C56E; when it ends, the state stepped.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
ListThenStep0EBD_536:
    lda #$6E
    sta z:zTemp10
    lda #$C5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenStep0EBD_536_at0
    dec a:OBJ_STATE,x
ListThenStep0EBD_536_at0:
    rts

; @routine 0EBE:015 LoadAttrByArg0EBE
; @run 71:C815
; WHAT IT DOES: Loads the record, attribute $8C; a start argument counts the state on (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAttrByArg0EBE:
    jsr VecLoadObjectRecord
    lda #$8C
    sta a:OBJ_SPRITE_ATTR,x
    lda a:OBJ_START_ARG,x
    beq LoadAttrByArg0EBE_at0
    inc a:OBJ_STATE,x
LoadAttrByArg0EBE_at0:
    rts

; @routine 0EBE:312 StopOnLandingThen0EBE_312
; @run 71:CB12
; WHAT IT DOES: Falls with height 4; on landing the state on with substate 0.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StopOnLandingThen0EBE_312:
    lda #$04
    jsr VecStopOnLanding
    bcc StopOnLandingThen0EBE_312_at0
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
StopOnLandingThen0EBE_312_at0:
    rts

; @routine 0EBE:7C2 ListThenStep0EBE_7C2
; @run 71:CFC2
; WHAT IT DOES: Plays the frame list at $CFD3; when it ends, the substate cleared.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
ListThenStep0EBE_7C2:
    lda #$D3
    sta z:zTemp10
    lda #$CF
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenStep0EBE_7C2_at0
    stz a:OBJ_SUBSTATE,x
ListThenStep0EBE_7C2_at0:
    rts

; @routine 0EBE:7DC ListThenStep0EBE_7DC
; @run 71:CFDC
; WHAT IT DOES: Plays the frame list at $CFED; when it ends, the substate cleared.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
ListThenStep0EBE_7DC:
    lda #$ED
    sta z:zTemp10
    lda #$CF
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenStep0EBE_7DC_at0
    stz a:OBJ_SUBSTATE,x
ListThenStep0EBE_7DC_at0:
    rts

; @routine 101B:0BC HitBoxWhenNear101B
; @run 70:B0BC
; WHAT IT DOES: Within $60 dots of the player along X: hit-tested, state on (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
HitBoxWhenNear101B:
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs HitBoxWhenNear101B_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
HitBoxWhenNear101B_at0:
    rts

; @routine 101D:2CD StateOnWhenClose101D
; @run 71:C2CD
; WHAT IT DOES: Counts the state on once the player is within $70 along X and $30 along Y (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StateOnWhenClose101D:
    jsr VecDistanceXToPlayer
    cmp #$70
    bcs StateOnWhenClose101D_at0
    jsr VecDistanceYToPlayer
    cmp #$30
    bcs StateOnWhenClose101D_at0
    inc a:OBJ_STATE,x
StateOnWhenClose101D_at0:
    rts

; @routine 101D:45C LoadAndStartC0_101D
; @run 71:C45C
; WHAT IT DOES: Loads the record and starts a kind $C0 object at its place in slot $1F (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAndStartC0_101D:
    jsr VecLoadObjectRecord
    ldy #$1F
    jsr VecClearObjectInY
    lda #$C0
    jsr VecStartObjectAtPlace
    rts

; @routine 10CC:001 ListThenSubstate3_10CC
; @run 70:B801
; WHAT IT DOES: Plays the frame list at $B814; when it ends, substate 3 (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenSubstate3_10CC:
    lda #$14
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenSubstate3_10CC_at0
    lda #$03
    sta a:OBJ_SUBSTATE,x
ListThenSubstate3_10CC_at0:
    rts

; @routine 10CC:455 SoundRestartSubstate10CC_455
; @run 70:BC55
; WHAT IT DOES: Sound $44, the animation restarted, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundRestartSubstate10CC_455:
    lda #$44
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 117D:496 FacePlayerInStates117D_496
; @run 71:C496
; WHAT IT DOES: In the states the table at $C4A7 marks, turns the object by the player's side (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FacePlayerInStates117D_496Turns = $C4A7 ; by state: 0 does not turn
FacePlayerInStates117D_496:
    ldy a:OBJ_STATE,x
    lda a:FacePlayerInStates117D_496Turns,y
    beq FacePlayerInStates117D_496_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FacePlayerInStates117D_496_at0:
    rts

; @routine 117E:10F ListThenMove117E_10F
; @run 71:C90F
; WHAT IT DOES: Plays the frame list at $C8BE, then moves the object.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenMove117E_10F:
    lda #$C8
    sta z:zTemp11
    lda #$BE
    sta z:zTemp10
    jsr VecStepObjectFrames
    jmp VecApplyVelocity

; @routine 117E:1AF FacePlayerInStates117E_1AF
; @run 71:C9AF
; WHAT IT DOES: In the states the table at $C9C0 marks, turns the object by the player's side (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FacePlayerInStates117E_1AFTurns = $C9C0 ; by state: 0 does not turn
FacePlayerInStates117E_1AF:
    ldy a:OBJ_STATE,x
    lda a:FacePlayerInStates117E_1AFTurns,y
    beq FacePlayerInStates117E_1AF_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FacePlayerInStates117E_1AF_at0:
    rts

; @routine 117E:24B StopOnLandingThen117E_24B
; @run 71:CA4B
; WHAT IT DOES: Falls with height 16; on landing the animation restarted and the state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StopOnLandingThen117E_24B:
    lda #$10
    jsr VecStopOnLanding
    bcc StopOnLandingThen117E_24B_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
StopOnLandingThen117E_24B_at0:
    rts

; @routine 117F:296 FacePlayerInStates117F_296
; @run 71:D296
; WHAT IT DOES: In the states the table at $D2A7 marks, turns the object by the player's side (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FacePlayerInStates117F_296Turns = $D2A7 ; by state: 0 does not turn
FacePlayerInStates117F_296:
    ldy a:OBJ_STATE,x
    lda a:FacePlayerInStates117F_296Turns,y
    beq FacePlayerInStates117F_296_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FacePlayerInStates117F_296_at0:
    rts

; @routine 12D0:4ED LoadFrameByArg12D0
; @run 6B:9CED
; WHAT IT DOES: Loads the record; the frame by the start argument's bit 0 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFrameByArg12D0Frames = $9CFD ; the frame by that bit
LoadFrameByArg12D0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:LoadFrameByArg12D0Frames,y
    sta a:OBJ_FRAME,x
    rts

; @routine 12DB:297 SpawnOnOddFrames12DB
; @run 70:B297
; WHAT IT DOES: Every other frame starts a kind $26 object at this place (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnOnOddFrames12DB:
    lda z:zWaveCounter
    and #$01
    bne SpawnOnOddFrames12DB_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnOnOddFrames12DB_at0
    lda #$26
    jsr VecStartObjectAtPlace
SpawnOnOddFrames12DB_at0:
    rts

; @routine 12DB:2E6 CountThenSound73_12DB
; @run 70:B2E6
; WHAT IT DOES: Counts OBJ_WORK_E; at zero sound $73 and the state on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenSound73_12DB:
    dec a:OBJ_WORK_E,x
    bne CountThenSound73_12DB_at0
    lda #$73
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
CountThenSound73_12DB_at0:
    rts

; @routine 12DB:451 LoadThenWaits12DB
; @run 70:B451
; WHAT IT DOES: Loads the record, OBJ_WORK_E $70 and OBJ_WORK_H $80 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadThenWaits12DB:
    jsr VecLoadObjectRecord
    lda #$70
    sta a:OBJ_WORK_E,x
    lda #$80
    sta a:OBJ_WORK_H,x
    rts

; @routine 12DB:5B5 SoundRestartSubstate12DB_5B5
; @run 70:B5B5
; WHAT IT DOES: Sound $75, the animation restarted, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundRestartSubstate12DB_5B5:
    lda #$75
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 12DD:059 LoadFrameByWorkD12DD
; @run 71:C059
; WHAT IT DOES: Loads the record; the frame by OBJ_WORK_D, OBJ_WORK_I 8 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFrameByWorkD12DDFrames = $C06B ; the frame by OBJ_WORK_D
LoadFrameByWorkD12DD:
    jsr VecLoadObjectRecord
    ldy a:OBJ_WORK_D,x
    lda a:LoadFrameByWorkD12DDFrames,y
    sta a:OBJ_FRAME,x
    lda #$08
    sta a:OBJ_WORK_I,x
    rts

; @routine 12DD:1FB ListThenMove12DD_1FB
; @run 71:C1FB
; WHAT IT DOES: Plays the frame list at $C20A, then moves the object.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenMove12DD_1FB:
    lda #$0A
    sta z:zTemp10
    lda #$C2
    sta z:zTemp11
    jsr VecStepObjectFrames
    jmp VecApplyVelocityX

; @routine 13FB:3BC StartKind3DAt17_13FB
; @run 70:B3BC
; WHAT IT DOES: Starts a kind $3D object at this place in slot $11, substate on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartKind3DAt17_13FB:
    ldy #$11
    jsr VecClearObjectInY
    lda #$3D
    jsr VecStartObjectAtPlace
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 13FB:63F SpawnEveryFew13FB_63F
; @run 70:B63F
; WHAT IT DOES: On some counts of OBJ_WORK_D, starts a kind $41 object at this place (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnEveryFew13FB_63F:
    lda a:OBJ_WORK_D,x
    and #$01
    bne SpawnEveryFew13FB_63F_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnEveryFew13FB_63F_at0
    lda #$41
    jsr VecStartObjectAtPlace
SpawnEveryFew13FB_63F_at0:
    rts

; @routine 161B:388 ListThenStep161B_388
; @run 70:B388
; WHAT IT DOES: Plays the frame list at $B399; when it ends, the state stepped.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
ListThenStep161B_388:
    lda #$99
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenStep161B_388_at0
    inc a:OBJ_STATE,x
ListThenStep161B_388_at0:
    rts

; @routine 161C:400 ListThenStep161C_400
; @run 70:BC00
; WHAT IT DOES: Plays the frame list at $BC11; when it ends, the state stepped.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, or N and Z from the step.
ListThenStep161C_400:
    lda #$11
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenStep161C_400_at0
    inc a:OBJ_STATE,x
ListThenStep161C_400_at0:
    rts

; @routine 161D:3D2 MoveAndCount161D
; @run 71:C3D2
; WHAT IT DOES: Moves; every fourth count of OBJ_WORK_D the substate on (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
MoveAndCount161D:
    jsr VecApplyVelocity
    dec a:OBJ_WORK_D,x
    bne MoveAndCount161D_at0
    lda #$04
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
MoveAndCount161D_at0:
    rts

; @routine 161D:644 StateOnWhenHitOrNear161D
; @run 71:C644
; WHAT IT DOES: Counts the state on once hit, or once the player is within $68 dots (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StateOnWhenHitOrNear161D:
    lda a:OBJ_HIT_TAKEN,x
    bne StateOnWhenHitOrNear161D_at0
    jsr VecDistanceXToPlayer
    cmp #$68
    bcs StateOnWhenHitOrNear161D_at1
StateOnWhenHitOrNear161D_at0:
    inc a:OBJ_STATE,x
StateOnWhenHitOrNear161D_at1:
    rts

; @routine 16CC:583 SpawnAtRandom16CC
; @run 70:BD83
; WHAT IT DOES: One frame in 16 starts a kind $9B object at this place (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnAtRandom16CC:
    jsr VecNextRandom
    and #$0F
    bne SpawnAtRandom16CC_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnAtRandom16CC_at0
    lda #$9B
    jsr VecStartObjectAtPlace
SpawnAtRandom16CC_at0:
    rts
