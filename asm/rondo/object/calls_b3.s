; @routine 088A:750 PushByAnswer088A
; @run 70:AF50
; WHAT IT DOES: When the routine at $AF65 answers C for Y 0, moves along X at the table's speed for OBJ_WORK_H (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
PushByAnswer088ASpeeds = $AF63 ; the X speed high byte by OBJ_WORK_H
PushByAnswer088A:
    cly
    jsr StepListByIndex088A
    bcc PushByAnswer088A_at0
    ldy a:OBJ_WORK_H,x
    lda a:PushByAnswer088ASpeeds,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
PushByAnswer088A_at0:
    rts

; @routine 088B:031 ToState5WhenSlot2Ready088B
; @run 70:B031
; WHAT IT DOES: When the routine at $AF65 answers Z for slot 2: the animation restarted, state 5 (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ToState5WhenSlot2Ready088B:
    ldy #$02
    jsr StepListByIndex088A
    bne ToState5WhenSlot2Ready088B_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
ToState5WhenSlot2Ready088B_at0:
    rts

; @routine 088B:5FC StartWhenSlot18Ready088B
; @run 70:B5FC
; WHAT IT DOES: When the routine at $AF65 answers Z for slot $12: OBJ_WORK_L 1, OBJ_WORK_K 0, substate on (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartWhenSlot18Ready088B:
    ldy #$12
    jsr StepListByIndex088A
    bne StartWhenSlot18Ready088B_at0
    lda #$01
    sta a:OBJ_WORK_L,x
    stz a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
StartWhenSlot18Ready088B_at0:
    rts

; @routine 088B:630 WhenSlot20Ready088B
; @run 70:B630
; WHAT IT DOES: When the routine at $AF65 answers Z for slot $14: OBJ_WORK_K 128, substate on (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WhenSlot20Ready088B:
    ldy #$14
    jsr StepListByIndex088A
    bne WhenSlot20Ready088B_at0
    lda #$80
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
WhenSlot20Ready088B_at0:
    rts

; @routine 0B4C:7FF LoadFrame4OrFall0B4C
; @run 70:BFFF
; WHAT IT DOES: In state 0 loads the record with frame 4; later falls (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadFrame4OrFall0B4C:
    lda a:OBJ_STATE,x
    bne LoadFrame4OrFall0B4C_at0
    jsr VecLoadObjectRecord
    lda #$04
    sta a:OBJ_FRAME,x
    rts
LoadFrame4OrFall0B4C_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocityY

; @routine 0B4D:0E5 ListFallMove0B4D
; @run 71:C0E5
; WHAT IT DOES: Plays the frame list at $C0F6, falls and moves (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListFallMove0B4D:
    lda #$C0
    sta z:zTemp11
    lda #$F6
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0EBD:0A7 ClearFirstEmptySlot0EBD
; @run 71:C0A7
; WHAT IT DOES: Clears the first empty slot from $15 to $1F, leaving it in Y with C set; C clear with none (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: Exit: as the last call, compare or load left them.
ClearFirstEmptySlot0EBD:
    ldy #$15
ClearFirstEmptySlot0EBD_at1:
    lda a:OBJ_KIND,y
    bne ClearFirstEmptySlot0EBD_at0
    jsr VecClearObjectInY
    sec
    rts
ClearFirstEmptySlot0EBD_at0:
    iny
    cpy #$20
    bne ClearFirstEmptySlot0EBD_at1
    clc
    rts

; @routine 101E:13E HitStateWhenNear101E
; @run 71:C93E
; WHAT IT DOES: Rotates A, runs the routine at $C9EB; unless it leaves C set, an object within $50 dots of the player goes to state 3 (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
HitStateWhenNear101E:
    rol a
    jsr CountToState4_101E
    bcs HitStateWhenNear101E_at0
    jsr VecDistanceXToPlayer
    cmp #$50
    bcs HitStateWhenNear101E_at0
    lda #$03
    sta a:OBJ_STATE,x
HitStateWhenNear101E_at0:
    rts

; @routine 1020:0AA StepListByWorkF1020
; @run 71:D8AA
; WHAT IT DOES: Plays the frame list the table at $DA62 gives for OBJ_WORK_F (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StepListByWorkF1020Lists = $DA62 ; a frame list per OBJ_WORK_F
StepListByWorkF1020:
    lda a:OBJ_WORK_F,x
    asl a
    tay
    lda a:StepListByWorkF1020Lists,y
    sta z:zTemp10
    lda a:StepListByWorkF1020Lists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    rts

; @routine 10CC:468 ListThenSubstate1_10CC
; @run 70:BC68
; WHAT IT DOES: Plays the frame list at $BC7C; when it ends, substate 1 (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenSubstate1_10CC:
    lda #$7C
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListThenSubstate1_10CC_at0
    rts
ListThenSubstate1_10CC_at0:
    lda #$01
    sta a:OBJ_SUBSTATE,x
    rts

; @routine 117C:3D8 LandOnRowThenDebris117C
; @run 70:BBD8
; WHAT IT DOES: When the object reaches the row at $Dx: sound $23, then on to the routine at $A90B with A 0 (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandOnRowThenDebris117C:
    lda a:OBJ_Y_LO,x
    and #$F0
    cmp #$D0
    bne LandOnRowThenDebris117C_at0
    lda #$23
    jsr VecSoundCommand
    cla
    jmp StartDebrisBurst
LandOnRowThenDebris117C_at0:
    rts

; @routine 12DC:02E StartKind2AIn32To43_12DC
; @run 70:B82E
; WHAT IT DOES: Starts kind $2A objects at this place in slots $20 to $2B (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartKind2AIn32To43_12DC:
    ldy #$20
StartKind2AIn32To43_12DC_at0:
    jsr VecClearObjectInY
    lda #$2A
    jsr VecStartObjectAtPlace
    iny
    cpy #$2C
    bcc StartKind2AIn32To43_12DC_at0
    rts

; @routine 13FB:493 SpawnOddFramesAnswer13FB
; @run 70:B493
; WHAT IT DOES: Every other frame, starts a kind $3E object at this place in a free slot; C says whether (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnOddFramesAnswer13FB:
    lda z:zWaveCounter
    and #$01
    bne SpawnOddFramesAnswer13FB_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnOddFramesAnswer13FB_at0
    lda #$3E
    jsr VecStartObjectAtPlace
    sec
    rts
SpawnOddFramesAnswer13FB_at0:
    clc
    rts

; @routine 161D:630 LoadAndStartNext161D
; @run 71:C630
; WHAT IT DOES: Loads the record, OBJ_WORK_H $C0, and starts a kind $4E object at this place in the next slot (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAndStartNext161D:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_WORK_H,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$4E
    jsr VecStartObjectAtPlace
    rts

; @routine 161E:3D9 RecordThenClear161E
; @run 71:CBD9
; WHAT IT DOES: A first call only counts OBJ_WORK_E; the next records the object's event and clears it (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RecordThenClear161E:
    lda a:OBJ_WORK_E,x
    bne RecordThenClear161E_at0
    inc a:OBJ_WORK_E,x
    rts
RecordThenClear161E_at0:
    jsr VecRecordObjectEvent
    jmp VecClearObject

; @routine 16CC:3E0 ListThenClearOnCount16CC
; @run 70:BBE0
; WHAT IT DOES: Plays the frame list at $BBD7 and counts OBJ_WORK_D; at zero the object is cleared (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenClearOnCount16CC:
    lda #$D7
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_D,x
    bne ListThenClearOnCount16CC_at0
    jmp VecClearObject
ListThenClearOnCount16CC_at0:
    rts
