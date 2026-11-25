; @routine 16CB:175 CountThenSpawnInState2
; @run 70:B175
; WHAT IT DOES: In state 2 counts OBJ_3880 down; at zero it sets it to $22 and starts a kind $28 object in a free slot from $30 at this object's place.
; HOW: DEC, FindFreeSlotFrom30, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: the last compare, the count, or the start's registers.
CountThenSpawnInState2:
    lda a:OBJ_27A8,x
    cmp #$02
    bne CountThenSpawnInState2_at0
    lda a:OBJ_3880,x
    beq CountThenSpawnInState2_at1
    dec a:OBJ_3880,x
    rts
CountThenSpawnInState2_at1:
    lda #$22
    sta a:OBJ_3880,x
    jsr VecFindFreeSlotFrom30
    bcc CountThenSpawnInState2_at0
    lda #$28
    jsr VecStartObjectAtPlace
CountThenSpawnInState2_at0:
    rts

; @routine 16CB:1F4 StateFiveOnStatusBit7
; @run 70:B1F4
; WHAT IT DOES: An object in a state under 5 whose OBJ_31A0 has bit 7 set goes to state 5, clears OBJ_2800 and OBJ_3250. (The part after the test ran in no script; it is written from the bytes.)
; HOW: A test, a compare, three stores.
; WAITS: no.
; Entry: X object. Exit: the test, the compare, or A 5.
StateFiveOnStatusBit7:
    lda a:OBJ_31A0,x
    and #$80
    beq StateFiveOnStatusBit7_at0
    lda a:OBJ_27A8,x
    cmp #$05
    bcs StateFiveOnStatusBit7_at0
    lda #$05
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    stz a:OBJ_3250,x
StateFiveOnStatusBit7_at0:
    rts

; @routine 16CB:20E LoadThenStartRise
; @run 70:B20E
; WHAT IT DOES: Loads the object's record, sets its flip byte (OBJ_2FE8) to $80, its Y speed to $FE80 and OBJ_3720 to $E0.
; HOW: LoadObjectRecord, stores.
; WAITS: no.
; Entry: X object. Exit: A $E0, N set, Z clear; C and Y from the record.
LoadThenStartRise:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_2FE8,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$E0
    sta a:OBJ_3720,x
    rts

; @routine 16CB:24C ClearSlotsWhenScrollZero
; @run 70:B24C
; WHAT IT DOES: When the level's X position high byte and the scroll X low are both zero, clears the objects in slots $11 to $4E, sets OBJ_34B8 to $60 and goes on to the next OBJ_2800.
; HOW: Two reads, a loop of ClearObjectInY, a store, an INC.
; WAITS: no.
; Entry: X object. Exit: the OR, or the new OBJ_2800's N and Z.
ClearSlotsWhenScrollZero:
    lda a:ScrollXHiScreen
    ora a:ScrollXLo
    bne ClearSlotsWhenScrollZero_at0
    ldy #$11
ClearSlotsWhenScrollZero_at1:
    jsr VecClearObjectInY
    iny
    cpy #$4F
    bcc ClearSlotsWhenScrollZero_at1
    lda #$60
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
ClearSlotsWhenScrollZero_at0:
    rts

; @routine 16CB:27E StepListB2A7ThenPlace
; @run 70:B27E
; WHAT IT DOES: Steps the animation list at $B2A7; while it runs the object moves along Y; when it ends the object is put at X $007C, Y $0026 relative to its screen, gets frame 0, OBJ_34B8 $30 and goes to the next OBJ_2800.
; HOW: StepObjectFrames, a tail jump into ApplyVelocityY, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocityY, or the new OBJ_2800's N and Z (A $30).
StepListB2A7ThenPlace:
    lda #$A7
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepListB2A7ThenPlace_at0
    jmp VecApplyVelocityY
StepListB2A7ThenPlace_at0:
    lda #$7C
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$26
    sta a:OBJ_Y_LO,x
    stz a:OBJ_2A68,x
    lda #$30
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
    rts

; @routine 16CB:2D4 CountThenRiseWithFlash
; @run 70:B2D4
; WHAT IT DOES: Counts OBJ_34B8 down; at zero it sets sprite set 4, flash flags $10, OBJ_34B8 $40 and a Y speed of $FFC0 and goes on to the next OBJ_2800.
; HOW: DEC, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new OBJ_2800's N and Z (A $C0).
CountThenRiseWithFlash:
    dec a:OBJ_34B8,x
    bne StateMachineWithSoundAndList
    lda #$04
    sta a:OBJ_2A10,x
    lda #$10
    sta a:OBJ_2B18,x
    lda #$40
    sta a:OBJ_34B8,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    inc a:OBJ_2800,x
    rts

; @routine 16CB:2F6 StepListB36AThenCount
; @run 70:B2F6
; WHAT IT DOES: Steps the animation list at $B36A and moves the object along Y; counts OBJ_34B8 down and at zero clears OBJ_2FE8, sets OBJ_3510 to $40 and goes on to the next OBJ_2800.
; HOW: StepObjectFrames, ApplyVelocityY, DEC, stores, INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the count, or the new OBJ_2800's N and Z (A $40).
StepListB36AThenCount:
    lda #$6A
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    dec a:OBJ_34B8,x
    bne StepListB36AThenCount_at0
    stz a:OBJ_2FE8,x
    lda #$40
    sta a:OBJ_3510,x
    inc a:OBJ_2800,x
StepListB36AThenCount_at0:
    rts

; @routine 16CB:3C8 CountThenJumpRight
; @run 70:B3C8
; WHAT IT DOES: Sets the object's frame to 1 and counts OBJ_34B8 down; at zero it gives the object an X speed high of $12 and a Y speed high of $E0, moves it once, clears its animation step and timer, sets frame 3 and goes on to the next OBJ_2800.
; HOW: DEC, stores, ApplyVelocity, INC.
; WAITS: no.
; Entry: X object. Exit: the count, or the new OBJ_2800's N and Z (A 3).
CountThenJumpRight:
    lda #$01
    sta a:OBJ_2A68,x
    dec a:OBJ_34B8,x
    bne StepFramesFacingPlayer
    lda #$12
    sta a:OBJ_VX_HI,x
    lda #$E0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$03
    sta a:OBJ_2A68,x
    inc a:OBJ_2800,x
    rts

; @routine 16CB:3EE StepListB404ThenTimer
; @run 70:B3EE
; WHAT IT DOES: Steps the animation list at $B404; when it ends, OBJ_34B8 becomes $48 and the object goes to the next OBJ_2800.
; HOW: StepObjectFrames, a store, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the step, or the new OBJ_2800's N and Z (A $48) when the list ended.
StepListB404ThenTimer:
    lda #$04
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepListB404ThenTimer_at0
    lda #$48
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
StepListB404ThenTimer_at0:
    rts
; @routine 16CB:54F StartKindFromTableAtSlot16
; @run 70:B54F
; WHAT IT DOES: Takes a kind from the table at $B56B by OBJ_3618, clears the object in slot $16, starts an object of that kind there at this object's place, counts OBJ_3618 up, sets OBJ_34B8 to $40 and goes on to the next OBJ_2800.
; HOW: A table read (bank 70), ClearObjectInY, StartObjectAtPlace, two INCs.
; WAITS: no.
; Entry: X object. Exit: A $40, Y $16, N and Z from the new OBJ_2800.
StartKindFromTableAtSlot16:
    ldy a:OBJ_3618,x
    lda a:$B56B,y
    pha
    ldy #$16
    jsr VecClearObjectInY
    pla
    jsr VecStartObjectAtPlace
    inc a:OBJ_3618,x
    lda #$40
    sta a:OBJ_34B8,x
    inc a:OBJ_2800,x
    rts
; @routine 16CB:5A8 JumpAwayThenLand
; @run 70:B5A8
; WHAT IT DOES: OBJ_2800 0: clears OBJ_3250, faces the player's side (bit 0 of SideOfPlayer), takes its X speed from the table at $B619 by that side and gets a Y speed of $FC00. Later it moves, adds $0020 to its Y speed, and once it falls (Y speed high not negative) to a Y low of $94 or more it stops at $94, sets OBJ_3250 to 2, turns round, flips bit 1 of OBJ_3618, halves OBJ_3720 twice and goes to state 2 with OBJ_2800 0.
; HOW: SideOfPlayer, table reads (bank 70), ApplyVelocity, a 16-bit add, stores.
; WAITS: no.
; Entry: X object. Exit: the last test or store.
JumpAwayThenLand:
    lda a:OBJ_2800,x
    bne JumpAwayThenLand_at0
    stz a:OBJ_3250,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    asl a
    tay
    tay
    lda a:$B61A,y
    sta a:OBJ_VX_HI,x
    lda a:$B619,y
    sta a:OBJ_VX_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_2800,x
    rts
JumpAwayThenLand_at0:
    jsr VecApplyVelocity
    lda a:OBJ_VY_LO,x
    clc
    adc #$20
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    bmi JumpAwayThenLand_at1
    lda a:OBJ_Y_LO,x
    cmp #$94
    bcc JumpAwayThenLand_at1
    lda #$94
    sta a:OBJ_Y_LO,x
    lda #$02
    sta a:OBJ_3250,x
    lda a:OBJ_2AC0,x
    eor #$01
    sta a:OBJ_2AC0,x
    lda a:OBJ_3618,x
    eor #$02
    sta a:OBJ_3618,x
    lsr a:OBJ_3720,x
    lsr a:OBJ_3720,x
    lda #$02
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
JumpAwayThenLand_at1:
    rts

; @routine 16CB:6EC LoadThenScatterAlongList
; @run 70:B6EC
; WHAT IT DOES: In state 0 loads the object's record, sets flash flags $10, gives it a random X speed high of -8 to 7 and Y speed high of -16 to -1, moves it once, then overwrites the speeds from the two tables at $B73F and $B747 by random rows and clears its animation step and timer. Later it steps the animation list at $B763, clears the object when it ends and otherwise moves it along Y.
; HOW: LoadObjectRecord, NextRandom four times, ApplyVelocity, table reads (bank 70), StepObjectFrames, ApplyVelocityY, ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: the last instruction's registers.
LoadThenScatterAlongList:
    lda a:OBJ_27A8,x
    bne $B74F
    jsr VecLoadObjectRecord
    lda #$10
    sta a:OBJ_2B18,x
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$08
    sta a:OBJ_VX_HI,x
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$10
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$B740,y
    sta a:OBJ_VX_HI,x
    lda a:$B73F,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$B748,y
    sta a:OBJ_VY_HI,x
    lda a:$B747,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    rts
    rti
    brk
    cpy #$00
    bra SoundCommandJump
    brk
    ora (zTemp00,x)
    inc a:$FD80,x
    bra $B74C
    rti
    inc a:$63A9,x
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LoadThenScatterAlongList_at0
    jmp VecClearObject
LoadThenScatterAlongList_at0:
    jsr VecApplyVelocityY
    rts

; @routine 16CB:782 LoadThenPlaceAtLeft
; @run 70:B782
; WHAT IT DOES: Loads the object's record, sets the raster effect to $0B with argument $BF, puts the object at X $0010, Y $00F6 and sets OBJ_34B8 to $40.
; HOW: LoadObjectRecord, stores.
; WAITS: no.
; Entry: X object. Exit: A $40.
LoadThenPlaceAtLeft:
    jsr VecLoadObjectRecord
    lda #$0B
    sta z:zRasterEffect
    lda #$BF
    sta z:zRasterArg
    stz a:OBJ_X_HI,x
    lda #$10
    sta a:OBJ_X_LO,x
    stz a:OBJ_Y_HI,x
    lda #$F6
    sta a:OBJ_Y_LO,x
    lda #$40
    sta a:OBJ_34B8,x
    rts
