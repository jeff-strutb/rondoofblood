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
    inc a:OBJ_27A8,x
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
    ldy a:OBJ_27A8,x
    lda a:$B089,y
    beq FaceThePlayerByStateTableB_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
FaceThePlayerByStateTableB_at0:
    rts

; @routine 0EB9:2C5 MaybeDropItemByPlayerDistance
; @run 70:A2C5
; WHAT IT DOES: An object with 5 or more hits whose OBJ_31F8 bits 2 to 4 are not 0 draws a random number against the table at $AC70 by those bits; when the number is below it, it picks an item from the table at $AC78 by the player's distance and starts it in a free slot of $30 to $3F at its place.
; HOW: NextRandom, PlayerDistanceSum, a table read, FindFreeSlotFrom30, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The drop arm ran in no script; the draft stops there.
MaybeDropItemByPlayerDistance:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_31F8,x
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

; @routine 0EBE:66D HitBoxByFrameTableCE8F
; @run 71:CE6D
; WHAT IT DOES: Takes the hit box (OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38) from a four byte row of the table at $CE8F picked by the frame less $45.
; HOW: A subtract, two shifts, four table reads, stores.
; WAITS: no.
; Entry: X object. Exit: A the last byte, Y the row, C from the shift.
HitBoxByFrameTableCE8F:
    lda a:OBJ_2A68,x
    sec
    sbc #$45
HitBoxByFrameTableCE8F_at0:
    asl a
    asl a
    tay
    lda a:$CE8F,y
    sta a:OBJ_3930,x
    lda a:$CE90,y
    sta a:OBJ_39E0,x
    lda a:$CE91,y
    sta a:OBJ_3988,x
    lda a:$CE92,y
    sta a:OBJ_3A38,x
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
    sta a:OBJ_2AC0,x
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs StepListFacePlayerUntilNear_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_27A8,x
StepListFacePlayerUntilNear_at0:
    rts

; @routine 0EBE:6F6 FacePlayerThenPickPhase
; @run 71:CEF6
; WHAT IT DOES: Takes the facing of the player's side and sets OBJ_2800 from the table at $CF0E by the low four bits of OBJ_34B8, which then counts up.
; HOW: SideOfPlayer, a table read, stores, an INC.
; WAITS: no.
; Entry: X object. Exit: A the table byte, Y the index, N and Z from the new OBJ_34B8.
FacePlayerThenPickPhase:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    lda a:OBJ_34B8,x
    and #$0F
    tay
    lda a:$CF0E,y
    sta a:OBJ_2800,x
    inc a:OBJ_34B8,x
    rts

; @routine 0EBE:71E LeapByPlayerDistance
; @run 71:CF1E
; WHAT IT DOES: When the player is $40 or more away in X clears OBJ_3568 and gives the object the speed X -$01C0, Y -$00C0; when nearer plays sound $C6, sets OBJ_3568 1 and the speed X -$00E0, Y -$0400. Goes to the next OBJ_2800 and, facing left, reverses the X speed.
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
    inc a:OBJ_2800,x
    lda a:OBJ_2AC0,x
    beq LeapByPlayerDistance_at2
    jsr VecNegateVelocityX
LeapByPlayerDistance_at2:
    rts

; @routine 0EBE:7B9 CountDownThenClearPhase
; @run 71:CFB9
; WHAT IT DOES: Counts OBJ_3510 down; at zero clears OBJ_2800.
; HOW: A memory decrement and a store.
; WAITS: no.
; Entry: X object. Exit: the decrement's N and Z. The store ran in no script; the draft stops there.
CountDownThenClearPhase:
    dec a:OBJ_3510,x
    bne CountDownThenClearPhase_at0
    stz a:OBJ_2800,x
CountDownThenClearPhase_at0:
    rts
