; @routine 0CAD:066 SparkPop0CAD
; @run 71:C066
; WHAT IT DOES: A spark thrown at random, rising until its list ends.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SparkPop0CAD:
    lda a:OBJ_STATE,x
    bne SparkPop0CAD_at0
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda #$FE
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    rts
SparkPop0CAD_at0:
    lda #$A3
    sta z:zTemp10
    lda #$C0
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne SparkPop0CAD_at1
    jmp VecClearObject
SparkPop0CAD_at1:
    jmp VecApplyVelocityY

; @routine 0E0B:29F WalkByStateTurning0E0B
; @run 70:B29F
; WHAT IT DOES: Walks the state's list at OBJ_WORK_E's speed; on turning at a wall takes the next state from a table.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WalkByStateTurning0E0BLists = $B2DA ; frame lists by state from 4
WalkByStateTurning0E0BTurns = $B2FC ; state after a turn
WalkByStateTurning0E0BSpeeds = $B2FE ; vx by OBJ_WORK_E
WalkByStateTurning0E0B:
    lda a:OBJ_STATE,x
    sec
    sbc #$04
    asl a
    tay
    lda a:WalkByStateTurning0E0BLists,y
    sta z:zTemp10
    lda a:WalkByStateTurning0E0BLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc WalkByStateTurning0E0B_at0
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:WalkByStateTurning0E0BSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkByStateTurning0E0BSpeeds,y
    sta a:OBJ_VX_LO,x
    jsr WalkTurnOrStop0E0B
    bcc WalkByStateTurning0E0B_at0
    lda a:OBJ_STATE,x
    sec
    sbc #$04
    tay
    lda a:WalkByStateTurning0E0BTurns,y
    sta a:OBJ_STATE,x
WalkByStateTurning0E0B_at0:
    rts

; @routine 0E0B:42E LungeFromList0E0B
; @run 70:B42E
; WHAT IT DOES: At frame $17 lunges (sounds $CD, $1F); at the list's end state 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LungeFromList0E0BSpeeds = $B470 ; vx by facing
LungeFromList0E0B:
    lda #$63
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq LungeFromList0E0B_at0
    bcc LungeFromList0E0B_at1
    lda a:OBJ_FRAME,x
    cmp #$17
    bne LungeFromList0E0B_at1
    lda #$CD
    jsr VecSoundCommand
    lda #$1F
    jsr VecSoundCommand
    ldy a:OBJ_FACING_LEFT,x
    lda a:LungeFromList0E0BSpeeds,y
    sta a:OBJ_VX_HI,x
    jmp WalkTurnOrStop0E0B
LungeFromList0E0B_at0:
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
LungeFromList0E0B_at1:
    rts

; @routine 10CC:499 BurstRow10CC
; @run 70:BC99
; WHAT IT DOES: At step 2 shakes (raster stage 3) and starts six kind $2E objects; at the list's end substate 1.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BurstRow10CC:
    lda #$D6
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq BurstRow10CC_at0
    bcc BurstRow10CC_at1
    lda a:OBJ_ANIM_STEP,x
    cmp #$02
    bne BurstRow10CC_at1
    lda #$03
    sta z:zRasterStage
    ldy #$13
BurstRow10CC_at2:
    jsr VecClearObjectInY
    lda #$2E
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda #$E5
    jsr VecSoundCommand
    iny
    cpy #$19
    bcc BurstRow10CC_at2
    rts
BurstRow10CC_at0:
    stz z:zRasterStage
    lda #$01
    sta a:OBJ_SUBSTATE,x
BurstRow10CC_at1:
    rts

; @routine 117E:45E ListByWorkIStomp117E
; @run 71:CC5E
; WHAT IT DOES: Runs the list OBJ_WORK_I picks, stomping (sound $1F) at two frames; at its end state 7 with a random wait.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListByWorkIStomp117EWaits = $CC9B ; four waits
ListByWorkIStomp117ELists = $CC9F ; frame lists by OBJ_WORK_I
ListByWorkIStomp117E:
    lda a:OBJ_WORK_I,x
    asl a
    tay
    lda a:ListByWorkIStomp117ELists,y
    sta z:zTemp10
    lda a:ListByWorkIStomp117ELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ListByWorkIStomp117E_at0
    bcc ListByWorkIStomp117E_at1
    lda a:OBJ_FRAME,x
    cmp #$26
    beq ListByWorkIStomp117E_at2
    cmp #$2D
    bne ListByWorkIStomp117E_at1
ListByWorkIStomp117E_at2:
    lda #$1F
    jmp VecSoundCommand
ListByWorkIStomp117E_at0:
    lda a:OBJ_START_ARG,x
    bne ListByWorkIStomp117E_at1
    lda #$07
    sta a:OBJ_STATE,x
    jsr VecPlayerDistanceSum
    and #$03
    tay
    lda a:ListByWorkIStomp117EWaits,y
    sta a:OBJ_WORK_F,x
ListByWorkIStomp117E_at1:
    rts

; @routine 12DB:45F FlickerIn12DB
; @run 70:B45F
; WHAT IT DOES: Flickers in, showing more often as OBJ_WORK_E runs down; then hit-tested.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FlickerIn12DBMasks = $B49B ; flicker mask by sixteenth
FlickerIn12DB:
    lda #$A3
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    beq FlickerIn12DB_at0
    sta a:OBJ_WORK_G,x
FlickerIn12DB_at0:
    stz a:OBJ_FRAME,x
    lda a:OBJ_WORK_E,x
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    lda a:OBJ_WORK_F,x
    and #$07
    and a:FlickerIn12DBMasks,y
    beq FlickerIn12DB_at1
    lda a:OBJ_WORK_G,x
    sta a:OBJ_FRAME,x
FlickerIn12DB_at1:
    dec a:OBJ_WORK_E,x
    bne FlickerIn12DB_at2
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
FlickerIn12DB_at2:
    rts

; @routine 12DC:569 SpitWhileList12DC
; @run 70:BD69
; WHAT IT DOES: Steers and runs the list, spitting kind $2F at frame $25; at its end state 2.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpitWhileList12DC:
    jsr SteerAndBob12DC
    jsr SpawnWhileEarly12DC
    lda #$A4
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq SpitWhileList12DC_at0
    bcc SpitWhileList12DC_at1
    lda a:OBJ_FRAME,x
    cmp #$25
    bne SpitWhileList12DC_at1
    lda #$2E
    jsr VecSoundCommand
    ldy #$50
    jsr VecClearObjectInY
    lda #$2F
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    rts
SpitWhileList12DC_at0:
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SpitWhileList12DC_at1:
    rts
