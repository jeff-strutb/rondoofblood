; @routine 05CC:103 LoadRecordLaunchByFacing
; @run 70:B903
; WHAT IT DOES: Loads the object's record and launches it: the X speed from the tables at $B92E and $B930 by its facing, the Y speed $08 then $06, moved once in between.
; HOW: LoadObjectRecord, table reads, ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: A $06, Y the facing doubled, C and V from the move.
LoadRecordLaunchByFacing:
    jsr VecLoadObjectRecord
    ldy a:OBJ_2AC0,x
    lda a:$B92E,y
    sta a:OBJ_VX_HI,x
    lda #$08
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_2AC0,x
    asl a
    tay
    lda a:$B931,y
    sta a:OBJ_VX_HI,x
    lda a:$B930,y
    sta a:OBJ_VX_LO,x
    lda #$06
    sta a:OBJ_VY_HI,x
    rts

; @routine 05CC:134 StepListBounceOffFloorAtRandom
; @run 70:B934
; WHAT IT DOES: Steps the animation list at $B97E and moves; when its Y low byte reaches $C4 it is put back to $C2 and gets a new random speed (X from the table at $B989 by a number to 15, Y from the table at $B9A9 by a number to 7), the animation is cleared and the state advances.
; HOW: StepObjectFrames, ApplyVelocity, two NextRandom calls, table reads, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListBounceOffFloorAtRandom:
    lda #$7E
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$C4
    bcc StepListBounceOffFloorAtRandom_at0
    lda #$C2
    sta a:OBJ_Y_LO,x
    jsr VecNextRandom
    and #$0F
    asl a
    tay
    lda a:$B98A,y
    sta a:OBJ_VX_HI,x
    lda a:$B989,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$07
    asl a
    tay
    lda a:$B9AA,y
    sta a:OBJ_VY_HI,x
    lda a:$B9A9,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_27A8,x
StepListBounceOffFloorAtRandom_at0:
    rts

; @routine 05CC:1B9 StepListFallThenLandOrClear
; @run 70:B9B9
; WHAT IT DOES: Steps the animation list at $B9F9 and moves; once the X high byte is not 0 the object is cleared. Otherwise the Y speed gains $0012 a frame, and at Y low $C4 it stops there, clears OBJ_3250 and the animation and goes to the next state.
; HOW: StepObjectFrames, ApplyVelocity, a 16-bit add in memory, a compare, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListFallThenLandOrClear:
    lda #$F9
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_X_HI,x
    beq LiftObjectsBySlowFraction
    jmp VecClearObject
    lda a:OBJ_VY_LO,x
    clc
    adc #$12
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_Y_LO,x
    cmp #$C4
    bcc StepListFallThenLandOrClear_at0
    lda #$C4
    sta a:OBJ_Y_LO,x
    stz a:OBJ_3250,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_27A8,x
StepListFallThenLandOrClear_at0:
    rts

; @routine 05CC:204 StepListThenClearWhenEnded
; @run 70:BA04
; WHAT IT DOES: Steps the animation list at $BA15; when it ends the object is cleared.
; HOW: StepObjectFrames, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step (all kept by ClearObject).
StepListThenClearWhenEnded:
    lda #$15
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne RichterItemSecondCosts
    jmp VecClearObject
    rts

; @routine 05CC:376 CountFramesThenRunByState
; @run 70:BB76
; WHAT IT DOES: Counts OBJ_34B8 up and runs the code a table at $BB8B names for the object's state (391, 3BA or 3FA).
; HOW: An INC, a table of two bytes to zTemp00-01, a jump through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
CountFramesThenRunByState:
    inc a:OBJ_34B8,x
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$BB8B,y
    sta z:zTemp00
    lda a:$BB8C,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 05CC:391 LoadRecordLaunchSideways
; @run 70:BB91
; WHAT IT DOES: Loads the object's record, gives it the X speed from the table at $BBB6 by its facing and the Y speed $08, moves it once, then the X speed from the table at $BBB8, the X low speed 0 and the Y speed $04.
; HOW: LoadObjectRecord, table reads by the facing, ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: A $04, Y the facing, C and V from the move.
LoadRecordLaunchSideways:
    jsr VecLoadObjectRecord
    ldy a:OBJ_2AC0,x
    lda a:$BBB6,y
    sta a:OBJ_VX_HI,x
    lda #$08
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:$BBB8,y
    sta a:OBJ_VX_HI,x
    lda #$00
    sta a:OBJ_VX_LO,x
    lda #$04
    sta a:OBJ_VY_HI,x
    rts

; @routine 05CC:3BA StepListFallThenTurnToPlayer
; @run 70:BBBA
; WHAT IT DOES: Steps the animation list at $BBF1 and moves; at Y low $C8 it takes the facing of the player's side, the X speed from the table at $BBF7 by it, stops at Y low $C7, clears the frame and goes to the next state.
; HOW: StepObjectFrames, ApplyVelocity, SideOfPlayer, table reads, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListFallThenTurnToPlayer:
    lda #$F1
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$C8
    bcc StepListFallThenTurnToPlayer_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    asl a
    tay
    lda a:$BBF7,y
    sta a:OBJ_VX_HI,x
    lda a:$BBF7,y
    sta a:OBJ_VX_LO,x
    lda #$C7
    sta a:OBJ_Y_LO,x
    stz a:OBJ_2A68,x
    inc a:OBJ_27A8,x
StepListFallThenTurnToPlayer_at0:
    rts

; @routine 05CC:3FA StepListSpawnAndMoveXOrClear
; @run 70:BBFA
; WHAT IT DOES: Steps the animation list at $BC14, runs SoundAndSpawnEveryFourthFrame and moves along X; once the X high byte is not 0 the object is cleared.
; HOW: StepObjectFrames, a call, ApplyVelocityX, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListSpawnAndMoveXOrClear:
    lda #$14
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr SoundAndSpawnEveryFourthFrame
    jsr VecApplyVelocityX
    lda a:OBJ_X_HI,x
    beq StepListSpawnAndMoveXOrClear_at0
    jmp VecClearObject
StepListSpawnAndMoveXOrClear_at0:
    rts

; @routine 05CC:419 SoundAndSpawnEveryFourthFrame
; @run 70:BC19
; WHAT IT DOES: When the low two bits of OBJ_34B8 are 0 plays sound $19 and starts an object of type $29 in a free slot of $30 to $3F at the object's place.
; HOW: An AND, SoundCommand, FindFreeSlotFrom30, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
SoundAndSpawnEveryFourthFrame:
    lda a:OBJ_34B8,x
    and #$03
    bne SoundAndSpawnEveryFourthFrame_at0
    lda #$19
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom30
    bne SoundAndSpawnEveryFourthFrame_at0
    lda #$29
    jsr VecStartObjectAtPlace
SoundAndSpawnEveryFourthFrame_at0:
    rts

; @routine 05CC:430 LoadRecordRandomFlingOrFall
; @run 70:BC30
; @stop BC69
; WHAT IT DOES: In state 0 loads the record, picks frame $23 or $24 at random and the speed (X and Y) from the table at $BC69 by another random number, and writes $BE to OBJ_Y_LO of the slot the record's Y names. Later it runs ClearIfAboveOrFall.
; HOW: LoadObjectRecord, two NextRandom calls, table reads, stores, a tail jump.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. OBJ_Y_LO of slot 11 is written because LoadObjectRecord leaves Y 11.
LoadRecordRandomFlingOrFall:
    lda a:OBJ_27A8,x
    bne ClearIfAboveOrFall
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    and #$01
    clc
    adc #$23
    sta a:OBJ_2A68,x
    lda #$BE
    sta a:OBJ_Y_LO,y
    jsr VecNextRandom
    and #$07
    asl a
    asl a
    tay
    lda a:$BC6A,y
    sta a:OBJ_VX_HI,x
    lda a:$BC69,y
    sta a:OBJ_VX_LO,x
    lda a:$BC6C,y
    sta a:OBJ_VY_HI,x
    lda a:$BC6B,y
    sta a:OBJ_VY_LO,x
    rts

; @routine 05CC:489 ClearIfAboveOrFall
; @run 70:BC89
; WHAT IT DOES: When the object's Y high byte is not 0 the object is cleared; otherwise it falls and moves.
; HOW: A test, a tail jump into ClearObject, or ApplyGravityUnlimited and a tail jump into ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: the load, or as ApplyVelocity.
ClearIfAboveOrFall:
    lda a:OBJ_Y_HI,x
    beq ClearIfAboveOrFall_at0
    jmp VecClearObject
ClearIfAboveOrFall_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 05CC:498 LoadRecordThenClearEnemiesAtScreen2
; @run 70:BC98
; WHAT IT DOES: In state 0 loads the record. Later, when the scroll's screen byte is 2, it sets the scroll's start high byte to 2, clears every object of kind $A0 whose flags' low nibble is 2 in slots $10 to $53, sets bit 4 of zField8E, plays sound $FE, draws tile script 0 and clears itself.
; HOW: LoadObjectRecord, a loop over the slots with ClearObject, SoundCommand, DrawTileScript, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
LoadRecordThenClearEnemiesAtScreen2:
    lda a:OBJ_27A8,x
    bne LoadRecordThenClearEnemiesAtScreen2_at0
    jmp VecLoadObjectRecord
LoadRecordThenClearEnemiesAtScreen2_at0:
    lda a:ScrollXHiScreen
    cmp #$02
    beq LoadRecordThenClearEnemiesAtScreen2_at1
    rts
LoadRecordThenClearEnemiesAtScreen2_at1:
    lda #$02
    sta a:ScrollXMinHi
    phx
    ldx #$10
LoadRecordThenClearEnemiesAtScreen2_at3:
    lda a:OBJ_2750,x
    cmp #$A0
    bne LoadRecordThenClearEnemiesAtScreen2_at2
    lda a:OBJ_FLAGS_2F38,x
    and #$0F
    cmp #$02
    bne LoadRecordThenClearEnemiesAtScreen2_at2
    jsr VecClearObject
LoadRecordThenClearEnemiesAtScreen2_at2:
    inx
    cpx #$54
    bcc LoadRecordThenClearEnemiesAtScreen2_at3
    plx
    smb4 z:zField8E
    lda #$FE
    jsr VecSoundCommand
    cly
    cla
    jsr VecDrawTileScript
    jmp VecClearObject
