; @routine 05CB:6F2 SoundThenSubstate05CB_6F2
; @run 70:B6F2
; WHAT IT DOES: Sound $87, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
SoundThenSubstate05CB_6F2:
    lda #$87
    jsr VecSoundCommand
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 072C:6B5 FallAndGo072C
; @run 70:BEB5
; WHAT IT DOES: Gravity, the Y move, then the routine at $BE56 (secret stage 03's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
FallAndGo072C:
    jsr VecApplyGravity
    jsr VecApplyVelocityY
    jsr BurstAtHeightE0
    rts

; @routine 088B:22D State9AndTurnBack088B
; @run 70:B22D
; WHAT IT DOES: State 9 with both speeds reversed (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
State9AndTurnBack088B:
    lda #$09
    sta a:OBJ_STATE,x
    jsr VecNegateVelocityX
    jmp VecNegateVelocityY

; @routine 0EBF:7B5 Sound23ThenDebris0EBF
; @run 71:D7B5
; WHAT IT DOES: Sound $23, then on to the routine at $A90B with A 0 (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
Sound23ThenDebris0EBF:
    lda #$23
    jsr VecSoundCommand
    cla
    jmp StartDebrisBurst

; @routine 0EC0:3C3 LoadThenSet0EC0_3C3
; @run 71:DBC3
; WHAT IT DOES: Loads the object's record, then facing left.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadThenSet0EC0_3C3:
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    rts

; @routine 101C:307 LoadThenSet101C_307
; @run 70:BB07
; WHAT IT DOES: Loads the object's record, then OBJ_WORK_F 64.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadThenSet101C_307:
    jsr VecLoadObjectRecord
    lda #$40
    sta a:OBJ_WORK_F,x
    rts

; @routine 101D:2C3 LoadThenFaceByArg101D
; @run 71:C2C3
; WHAT IT DOES: Loads the record; the start argument is the facing (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadThenFaceByArg101D:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FACING_LEFT,x
    rts

; @routine 101F:66E ClearAndFlag101F
; @run 71:D66E
; WHAT IT DOES: Sets zField92 and clears the object (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
ClearAndFlag101F:
    lda #$01
    sta z:zField92
    jmp VecClearObject

; @routine 10CD:331 LoadRecordTail10CD_331
; @run 71:C331
; WHAT IT DOES: Loads the object's record (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadRecordTail10CD_331:
    jmp VecLoadObjectRecord

; @routine 117C:76E ProbeHeight12Tail117C
; @run 70:BF6E
; WHAT IT DOES: Probe height 12, then on to the routine at $AB3A (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: as the last call or load left them.
ProbeHeight12Tail117C:
    lda #$0C
    sta z:SpriteFlipMask
    jmp BounceAtWalls

; @routine 117E:2E7 ProbeHeightCall117E_2E7
; @run 71:CAE7
; WHAT IT DOES: Probe height 16, then the routine at $AB3A (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: as the last call or load left them.
ProbeHeightCall117E_2E7:
    lda #$10
    sta z:SpriteFlipMask
    jsr BounceAtWalls
    rts

; @routine 117F:073 StateOnWhenNear117F
; @run 71:D073
; WHAT IT DOES: Counts the state on once the player is within 32 dots along X (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
StateOnWhenNear117F:
    jsr VecDistanceXToPlayer
    cmp #$20
    bcs StateOnWhenNear117F_at0
    inc a:OBJ_STATE,x
StateOnWhenNear117F_at0:
    rts

; @routine 117F:46F ProbeHeightCall117F_46F
; @run 71:D46F
; WHAT IT DOES: Probe height 18, then the routine at $AB3A (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: as the last call or load left them.
ProbeHeightCall117F_46F:
    lda #$12
    sta z:SpriteFlipMask
    jsr BounceAtWalls
    rts

; @routine 12DA:736 LoadThenDraw5_12DA
; @run 70:AF36
; WHAT IT DOES: Loads the record and draws tile script 5 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadThenDraw5_12DA:
    jsr VecLoadObjectRecord
    lda #$05
    cly
    jsr VecDrawTileScript
    rts

; @routine 12DA:7B1 LoadClearFlagWait12DA
; @run 70:AFB1
; WHAT IT DOES: Loads the record, clears zField92, OBJ_WORK_D 160 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadClearFlagWait12DA:
    jsr VecLoadObjectRecord
    stz z:zField92
    lda #$A0
    sta a:OBJ_WORK_D,x
    rts

; @routine 12DB:2A8 MoveY12DB_2A8
; @run 70:B2A8
; WHAT IT DOES: Moves the object along Y (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
MoveY12DB_2A8:
    jsr VecApplyVelocityY
    rts

; @routine 12DE:074 MoveX12DE_074
; @run 71:C874
; WHAT IT DOES: Moves the object along X (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
MoveX12DE_074:
    jsr VecApplyVelocityX
    rts

; @routine 161B:2FA LoadThenSet161B_2FA
; @run 70:B2FA
; WHAT IT DOES: Loads the object's record, then the hit box's Y offset $23.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadThenSet161B_2FA:
    jsr VecLoadObjectRecord
    lda #$23
    sta a:OBJ_HITBOX_DY,x
    rts

; @routine 161B:319 Fall161B
; @run 70:B319
; WHAT IT DOES: Gravity, then the Y move (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
Fall161B:
    jsr VecApplyGravity
    jmp VecApplyVelocityY

; @routine 161B:37F LoadThenSet161B_37F
; @run 70:B37F
; WHAT IT DOES: Loads the object's record, then OBJ_WORK_K 128.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadThenSet161B_37F:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_WORK_K,x
    rts

; @routine 161B:63D LoadThenSet161B_63D
; @run 70:B63D
; WHAT IT DOES: Loads the object's record, then OBJ_WORK_H 64.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadThenSet161B_63D:
    jsr VecLoadObjectRecord
    lda #$40
    sta a:OBJ_WORK_H,x
    rts

; @routine 161D:080 LoadThenSet161D_080
; @run 71:C080
; WHAT IT DOES: Loads the object's record, then OBJ_WORK_K 240.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
LoadThenSet161D_080:
    jsr VecLoadObjectRecord
    lda #$F0
    sta a:OBJ_WORK_K,x
    rts

; @routine 161F:03A StepList161F_03A
; @run 71:D03A
; WHAT IT DOES: Plays the frame list at $D045 (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
StepList161F_03A:
    lda #$45
    sta z:zTemp10
    lda #$D0
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 161F:052 StepList161F_052
; @run 71:D052
; WHAT IT DOES: Plays the frame list at $D05D (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
StepList161F_052:
    lda #$5D
    sta z:zTemp10
    lda #$D0
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 16CC:1FB SoundThenSubstate16CC_1FB
; @run 70:B9FB
; WHAT IT DOES: Sound $2A, substate on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or load left them.
SoundThenSubstate16CC_1FB:
    lda #$2A
    jsr VecSoundCommand
    inc a:OBJ_SUBSTATE,x
    rts
