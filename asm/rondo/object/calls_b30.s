; @routine 0E0B:11E OpenArenaWithRaster0E0B
; @run 70:B11E
; WHAT IT DOES: OpenArena0E0B with raster effect $0B in A: the entry two bytes before it.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
OpenArenaWithRaster0E0B:
    lda #$0B

; @routine 0EBA:75C WeightPlatform0EBA
; @run 70:AF5C
; WHAT IT DOES: Falls to the line its argument gives; stepped on there, dips.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WeightPlatform0EBA:
    lda a:OBJ_STATE,x
    bne WeightPlatform0EBA_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$06
    sta a:OBJ_FRAME,x
    lda a:OBJ_Y_LO,x
    sta a:OBJ_WORK_E,x
    rts
WeightPlatform0EBA_at0:
    lda a:OBJ_START_ARG,x
    jsr FallToLineInA0EBA
    lda #$18
    sta z:zTemp08
    lda #$1D
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc WeightPlatform0EBA_at1
    lda a:OBJ_WORK_D,x
    bne WeightPlatform0EBA_at2
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_START_ARG,x
    bne WeightPlatform0EBA_at2
    lda #$01
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
WeightPlatform0EBA_at2:
    lda #$01
    sta a:OBJ_WORK_D,x
    rts
WeightPlatform0EBA_at1:
    stz a:OBJ_WORK_D,x
    rts

; @routine 0EBB:014 BellStruck0EBB
; @run 70:B014
; WHAT IT DOES: A bell: a hard hit rings it (sound $3A) and records it; $C0 counts on, the state counts on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BellStruck0EBB:
    lda a:OBJ_STATE,x
    bne BellStruck0EBB_at0
    jsr VecLoadObjectRecord
    lda #$20
    sta a:OBJ_VY_LO,x
    rts
BellStruck0EBB_at0:
    dec a
    bne BellStruck0EBB_at1
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc BellStruck0EBB_at2
    lda #$3A
    jsr VecSoundCommand
    jsr VecRecordObjectEvent
    lda #$C0
    sta a:OBJ_WORK_D,x
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_KIND,y
    cmp #$A1
    bne BellStruck0EBB_at3
    lda a:OBJ_STATE,y
    cmp #$02
    bcc BellStruck0EBB_at3
    lda #$14
    sta a:StageVramBuffer+2
    lda #$CC
    jsr VecSoundCommand
BellStruck0EBB_at3:
    inc a:OBJ_STATE,x
BellStruck0EBB_at2:
    rts
BellStruck0EBB_at1:
    dec a
    bne BellStruck0EBB_at2
    dec a:OBJ_WORK_D,x
    beq BellStruck0EBB_at3
    jmp VecApplyVelocityY

; @routine 101D:647 HoldAndSwallow101D
; @run 71:C647
; WHAT IT DOES: Holds the player hidden at its place; at step 19 starts nine kind $BE objects and drops the player below.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
HoldAndSwallow101D:
    lda a:OBJ_SUBSTATE,x
    bne HoldAndSwallow101D_at0
    stz a:OBJ_FRAME
    lda #$02
    sta a:PlayerStartPose
    lda #$03
    sta a:OBJ_WORK_E
    stz a:OBJ_WORK_F
    ldy #$00
    jsr VecCopyObjectPlace
    lda #$95
    sta z:zTemp10
    lda #$C6
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq HoldAndSwallow101D_at1
    bcc HoldAndSwallow101D_at0
    lda a:OBJ_ANIM_STEP,x
    cmp #$13
    bne HoldAndSwallow101D_at0
    ldy #$10
HoldAndSwallow101D_at2:
    jsr VecClearObjectInY
    lda #$BE
    jsr VecStartObjectAtPlace
    iny
    cpy #$19
    bcc HoldAndSwallow101D_at2
    lda #$01
    sta a:OBJ_Y_HI
    lda #$80
    sta a:OBJ_Y_LO
    rts
HoldAndSwallow101D_at1:
    inc a:OBJ_SUBSTATE,x
HoldAndSwallow101D_at0:
    rts

; @routine 10CC:21C ArmedPace10CC
; @run 70:BA1C
; WHAT IT DOES: Paces toward the player, readying an attack when it can (sound $46) or every $60 counts.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ArmedPace10CC:
    lda a:OBJ_WORK_F,x
    bne ArmedPace10CC_at0
    lda #$60
    sta a:OBJ_WORK_F,x
    lda #$08
    sta a:OBJ_SUBSTATE,x
    rts
ArmedPace10CC_at0:
    dec a:OBJ_WORK_F,x
    jsr Slot18HitIs6Or9_10CC
    bcc ArmedPace10CC_at1
    lda #$46
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$07
    sta a:OBJ_SUBSTATE,x
    rts
ArmedPace10CC_at1:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc ArmedPace10CC_at2
    inc a:OBJ_SUBSTATE,x
    rts
ArmedPace10CC_at2:
    jsr SpawnEvery48_10CC
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$6A
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr SlideWithCosine10CC
    rts

; @routine 18DB:648 SinkThenTurn18DB
; @run 70:B648
; WHAT IT DOES: Sinks to Y $78 while its list runs; then allows the player to be, faces the player and goes on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SinkThenTurn18DB:
    lda #$95
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq SinkThenTurn18DB_at0
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    clc
    adc #$0C
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_HI,x
    bne SinkThenTurn18DB_at1
    lda a:OBJ_Y_LO,x
    cmp #$78
    bcc SinkThenTurn18DB_at1
    lda #$78
    sta a:OBJ_Y_LO,x
SinkThenTurn18DB_at1:
    rts
SinkThenTurn18DB_at0:
    lda z:zField8E
    ora #$04
    sta z:zField8E
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$20
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 18DD:0D1 BubbleAtRandom18DD
; @run 71:C0D1
; WHAT IT DOES: A bubble at a random spot with one of four lists, gone when it ends.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BubbleAtRandom18DDLists = $C11D ; frame lists by argument
BubbleAtRandom18DD:
    lda a:OBJ_STATE,x
    bne BubbleAtRandom18DD_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    jsr VecNextRandom
    and #$03
    sta a:OBJ_START_ARG,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$78
    sta a:OBJ_Y_LO,x
    jsr VecNextRandom
    and #$1F
    sta z:zTemp00
    jsr VecNextRandom
    sec
    sbc z:zTemp00
    clc
    adc #$10
    sta a:OBJ_X_LO,x
    rts
BubbleAtRandom18DD_at0:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:BubbleAtRandom18DDLists,y
    sta z:zTemp10
    lda a:BubbleAtRandom18DDLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne BubbleAtRandom18DD_at1
    jmp VecClearObject
BubbleAtRandom18DD_at1:
    rts
