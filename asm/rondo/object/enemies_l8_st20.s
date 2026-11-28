; @routine 0B4D:0A9 RecordEventWhenSlot10InState0F
; @run 71:C0A9
; WHAT IT DOES: When the state of slot $10 is $0F records the object's event, takes OBJ_34B8 from the table at $C0C0 by OBJ_3040 and goes to the next state.
; HOW: A compare, RecordObjectEvent, a table read, a store, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The arm past the compare ran in no script; the draft stops there.
RecordEventWhenSlot10InState0F:
    lda a:$27B8
    cmp #$0F
    bne RecordEventWhenSlot10InState0F_at0
    jsr VecRecordObjectEvent
    ldy a:OBJ_3040,x
    lda a:$C0C0,y
    sta a:OBJ_34B8,x
    inc a:OBJ_STATE,x
RecordEventWhenSlot10InState0F_at0:
    rts

; @routine 0E0B:071 FaceThePlayerByStateTableB
; @run 70:B071
; WHAT IT DOES: Every eighth frame of OBJ_3568, in the states the table at $B089 marks, the object takes the facing of the player's side.
; HOW: An AND, a table read by the state, SideOfPlayer, a store.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
FaceThePlayerByStateTableB:
    lda a:OBJ_3568,x
    and #$07
    bne FaceThePlayerByStateTableB_at0
    ldy a:OBJ_STATE,x
    lda a:$B089,y
    beq FaceThePlayerByStateTableB_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FaceThePlayerByStateTableB_at0:
    rts

; @routine 0EB9:2C5 MaybeDropItemByPlayerDistance
; @run 70:A2C5
; WHAT IT DOES: An object with 5 or more hits whose OBJ_HIT_REACTION bits 2 to 4 are not 0 draws a random number against the table at $AC70 by those bits; when the number is below it, it picks an item from the table at $AC78 by the player's distance and starts it in a free slot of $30 to $3F at its place.
; HOW: NextRandom, PlayerDistanceSum, a table read, FindFreeSlotFrom30, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The drop arm ran in no script; the draft stops there.
MaybeDropItemByPlayerDistance:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    and #$07
    beq RichterStandStep1
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:$AC70,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$AC78,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 0EBD:2A7 SpawnerByKindAndScreen
; @run 71:C2A7
; WHAT IT DOES: In state 0 loads the record. Unless frame $54's byte is 4 it waits OBJ_34B8 down; then by OBJ_3040: 0 starts an object of type $2C in a free slot (every second one from $1A) when the scroll's screen byte is 2 or 4, and waits $50 to $8F; 2 starts one of type $CB in a free slot of $30 to $3F and waits $80; otherwise, with OBJ_3568 below $80, it finds a group of four empty slots from $20 and starts four objects of type $49 there (each cleared first) numbered 3, 2, 1, 0 in OBJ_35C0 with one random number to 3 in OBJ_3670, and waits $F0.
; HOW: LoadObjectRecord, searches of the slot kinds, StartObjectAtPlace, ClearObjectInY, NextRandom, FindFreeSlotFrom30.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
SpawnerByKindAndScreen:
    lda a:OBJ_STATE,x
    bne SpawnerByKindAndScreen_at0
    jmp VecLoadObjectRecord
SpawnerByKindAndScreen_at0:
    lda a:$2ABC
    cmp #$04
    beq SpawnerByKindAndScreen_at1
    lda a:OBJ_34B8,x
    beq SpawnerByKindAndScreen_at2
    dec a:OBJ_34B8,x
SpawnerByKindAndScreen_at1:
    rts
SpawnerByKindAndScreen_at2:
    lda a:OBJ_3040,x
    bne SpawnerByKindAndScreen_at3
    lda a:ScrollXHiScreen
    cmp #$02
    bcc SpawnerByKindAndScreen_at1
    cmp #$03
    beq SpawnerByKindAndScreen_at1
    cmp #$05
    bcs SpawnerByKindAndScreen_at1
    ldy #$1A
SpawnerByKindAndScreen_at5:
    lda a:OBJ_KIND,y
    beq SpawnerByKindAndScreen_at4
    iny
    iny
    cpy #$20
    bcc SpawnerByKindAndScreen_at5
    rts
SpawnerByKindAndScreen_at4:
    lda #$2C
    jsr VecStartObjectAtPlace
    jsr VecNextRandom
    and #$3F
    clc
    adc #$50
    sta a:OBJ_34B8,x
    rts
SpawnerByKindAndScreen_at3:
    cmp #$02
    beq SpawnerByKindAndScreen_at6
    lda a:OBJ_3568,x
    cmp #$80
    bcs SpawnerByKindAndScreen_at7
    ldy #$20
SpawnerByKindAndScreen_at9:
    lda a:OBJ_KIND,y
    ora a:OBJ_KIND+1,y
    ora a:OBJ_KIND+2,y
    ora a:OBJ_KIND+3,y
    beq SpawnerByKindAndScreen_at8
    iny
    iny
    iny
    iny
    cpy #$40
    bcc SpawnerByKindAndScreen_at9
SpawnerByKindAndScreen_at7:
    rts
SpawnerByKindAndScreen_at8:
    phy
    jsr VecNextRandom
    and #$03
    sta z:zTemp01
    ply
    lda #$04
    sta z:zTemp00
SpawnerByKindAndScreen_at10:
    jsr VecClearObjectInY
    lda #$49
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    dec a
    sta a:OBJ_35C0,y
    lda z:zTemp01
    sta a:OBJ_3670,y
    iny
    dec z:zTemp00
    bne SpawnerByKindAndScreen_at10
    lda #$F0
    sta a:OBJ_34B8,x
    rts
SpawnerByKindAndScreen_at6:
    jsr VecFindFreeSlotFrom30
    bcc SpawnerByKindAndScreen_at11
    lda #$CB
    jsr VecStartObjectAtPlace
    lda #$80
    sta a:OBJ_34B8,x
SpawnerByKindAndScreen_at11:
    rts

; @routine 0EBE:66D HitBoxByFrameTableCE8F
; @run 71:CE6D
; WHAT IT DOES: Takes the hit box (OBJ_HITBOX_DX, OBJ_HITBOX_DY, OBJ_HITBOX_HALF_W, OBJ_HITBOX_HALF_H) from a four byte row of the table at $CE8F picked by the frame less $45.
; HOW: A subtract, two shifts, four table reads, stores.
; WAITS: no.
; Entry: X object. Exit: A the last byte, Y the row, C from the shift.
HitBoxByFrameTableCE8F:
    lda a:OBJ_FRAME,x
    sec
    sbc #$45
HitBoxByFrameTableCE8F_at0:
    asl a
    asl a
    tay
    lda a:$CE8F,y
    sta a:OBJ_HITBOX_DX,x
    lda a:$CE90,y
    sta a:OBJ_HITBOX_DY,x
    lda a:$CE91,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:$CE92,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0EBE:6AB StepListFacePlayerUntilNear
; @run 71:CEAB
; WHAT IT DOES: Steps the animation list at $CECF while it faces the player; once the player is within $60 in X it clears the animation and goes to the next state.
; HOW: StepObjectFrames, SideOfPlayer, DistanceXToPlayer, a compare, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListFacePlayerUntilNear:
    lda #$CE
    sta z:zTemp11
    lda #$CF
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs StepListFacePlayerUntilNear_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
StepListFacePlayerUntilNear_at0:
    rts

; @routine 0EBE:6F6 FacePlayerThenPickPhase
; @run 71:CEF6
; WHAT IT DOES: Takes the facing of the player's side and sets OBJ_SUBSTATE from the table at $CF0E by the low four bits of OBJ_34B8, which then counts up.
; HOW: SideOfPlayer, a table read, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: A the table byte, Y the index, N and Z from the new OBJ_34B8.
FacePlayerThenPickPhase:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_34B8,x
    and #$0F
    tay
    lda a:$CF0E,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_34B8,x
    rts

; @routine 0EBE:71E LeapByPlayerDistance
; @run 71:CF1E
; WHAT IT DOES: When the player is $40 or more away in X clears OBJ_3568 and gives the object the speed X -$01C0, Y -$00C0; when nearer plays sound $C6, sets OBJ_3568 1 and the speed X -$00E0, Y -$0400. Goes to the next OBJ_SUBSTATE and, facing left, reverses the X speed.
; HOW: DistanceXToPlayer, SoundCommand, stores, an INC, NegateVelocityX.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The near arm and the reverse ran in no script; the draft stops there.
LeapByPlayerDistance:
    jsr VecDistanceXToPlayer
    cmp #$40
    bcc LeapByPlayerDistance_at0
    stz a:OBJ_3568,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    bra LeapByPlayerDistance_at1
LeapByPlayerDistance_at0:
    lda #$C6
    jsr VecSoundCommand
    lda #$01
    sta a:OBJ_3568,x
    lda #$FF
    sta a:OBJ_VX_HI,x
    lda #$20
    sta a:OBJ_VX_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
LeapByPlayerDistance_at1:
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_FACING_LEFT,x
    beq LeapByPlayerDistance_at2
    jsr VecNegateVelocityX
LeapByPlayerDistance_at2:
    rts

; @routine 0EBE:7B9 CountDownThenClearPhase
; @run 71:CFB9
; WHAT IT DOES: Counts OBJ_3510 down; at zero clears OBJ_SUBSTATE.
; HOW: A memory decrement and a store.
; WAITS: no.
; Entry: X object. Exit: the decrement's N and Z. The store ran in no script; the draft stops there.
CountDownThenClearPhase:
    dec a:OBJ_3510,x
    bne CountDownThenClearPhase_at0
    stz a:OBJ_SUBSTATE,x
CountDownThenClearPhase_at0:
    rts

; @routine 0EBF:006 HopBetweenSidesWithBox
; @run 71:D006
; WHAT IT DOES: Sets the hit box by the frame (HitBoxByFrameTableCE8F). In state 0 loads the record, picks a side at random, places the object from the table at $D0BD (X) and a random Y near $00DC, and launches it up and aside. In state 1 it steps the list at $D0B8 and moves; its X speed grows by the table at $D0C1 for its side until it changes sign, then it stops, takes the Y speed $FFC0, OBJ_34B8 $10 and goes to state 2. In later states it counts OBJ_34B8 down while it moves in Y and at zero goes back to state 1 and launches again.
; HOW: HitBoxByFrameTableCE8F, LoadObjectRecord, NextRandom twice, StepObjectFrames, ApplyVelocityY and X, 16-bit adds, NegateVelocityX.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
HopBetweenSidesWithBox:
    jsr HitBoxByFrameTableCE8F
    lda a:OBJ_STATE,x
    bne HopBetweenSidesWithBox_at0
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    and #$01
    sta a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:$D0BD,y
    sta a:OBJ_X_LO,x
    lda a:$D0BE,y
    sta a:OBJ_X_HI,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    clc
    adc #$DC
    sta a:OBJ_Y_LO,x
HopBetweenSidesWithBox_at5:
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$90
    sta a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$E0
    sta a:OBJ_VY_LO,x
    lda a:OBJ_FACING_LEFT,x
    bne HopBetweenSidesWithBox_at1
    rts
HopBetweenSidesWithBox_at1:
    jmp VecNegateVelocityX
HopBetweenSidesWithBox_at0:
    dec a
    bne HopBetweenSidesWithBox_at2
    lda #$B8
    sta z:zTemp10
    lda #$D0
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    jsr VecApplyVelocityX
    lda a:OBJ_VX_HI,x
    sta z:zTemp00
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:$D0C1,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:$D0C2,y
    sta a:OBJ_VX_HI,x
    eor z:zTemp00
    and #$80
    beq HopBetweenSidesWithBox_at3
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$00
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    lda #$10
    sta a:OBJ_34B8,x
    inc a:OBJ_STATE,x
HopBetweenSidesWithBox_at3:
    rts
HopBetweenSidesWithBox_at2:
    dec a:OBJ_34B8,x
    beq HopBetweenSidesWithBox_at4
    jmp VecApplyVelocityY
HopBetweenSidesWithBox_at4:
    lda #$01
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_ANIM_STEP,x
    jmp HopBetweenSidesWithBox_at5
