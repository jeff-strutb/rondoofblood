; @routine 0CAB:175 FallingWithGravity
; @run 70:B175
; WHAT IT DOES: Steps the animation list at $B187, applies unlimited gravity and moves the object.
; HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity, Y as the step left it.
FallingWithGravity:
    lda #$87
    sta z:zTemp10
    lda #$B1
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocity
    rts

; @routine 0CAB:203 StartWhenHurt
; @run 70:B203
; WHAT IT DOES: An object in state 3 that has taken a hit of 5 or more resets its animation, enters state 6 and clears OBJ_2800.
; HOW: Two compares.
; WAITS: no.
; Entry: X object. Exit: the last compare or load.
StartWhenHurt:
    lda a:OBJ_27A8,x
    cmp #$03
    bne StartWhenHurt_at0
    lda a:OBJ_33B0,x
    cmp #$05
    bcc StartWhenHurt_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$06
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
StartWhenHurt_at0:
    rts

; @routine 0CAB:220 EnterStateSevenOnFlag
; @run 70:B220
; WHAT IT DOES: When bit 7 of the object's status flags is set and its state is below 7, it faces right, resets its animation, enters state 7 and clears OBJ_2800.
; HOW: Two compares.
; Untraced: every arm past the flag test (B227 to B23E).
; WAITS: no.
; Entry: X object. Exit: the last AND, compare or load.
EnterStateSevenOnFlag:
    lda a:OBJ_31A0,x
    and #$80
    beq EnterStateSevenOnFlag_at0
    lda a:OBJ_27A8,x
    cmp #$07
    bcs EnterStateSevenOnFlag_at0
    stz a:OBJ_2AC0,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$07
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
EnterStateSevenOnFlag_at0:
    rts

; @routine 0CAB:304 StartWithGuard
; @run 70:B304
; WHAT IT DOES: Loads the object's record and starts a kind $25 object at its place in slot $11; its timer ends as $80.
; HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A $80, Y $11, C from the record, N set.
StartWithGuard:
    jsr VecLoadObjectRecord
    ldy #$11
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    lda #$E0
    sta a:OBJ_34B8,x
    lda #$80
    sta a:OBJ_34B8,x
    rts

; @routine 0CAB:389 SlideAngleDown
; @run 70:B389
; WHAT IT DOES: Takes one off the 16-bit angle word (OBJ_3568 high, OBJ_35C0 low, in steps of $0100 less 0 on the low byte as the code subtracts $0100); when the high byte
; reaches 0 the object's OBJ_2800 goes up one.
; HOW: A 16-bit subtract.
; WAITS: no.
; Entry: X object. Exit: the last subtract or INC.
SlideAngleDown:
    lda a:OBJ_35C0,x
    sec
    sbc #$00
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    sbc #$01
    sta a:OBJ_3568,x
    bne SlideAngleDown_at0
    inc a:OBJ_2800,x
SlideAngleDown_at0:
    rts

; @routine 0CAB:3A0 SlideAngleUp
; @run 70:B3A0
; WHAT IT DOES: Adds $00C0 to the 16-bit angle word (OBJ_3568 high, OBJ_35C0 low); once the high byte reaches $10 the object's OBJ_2800 goes up one.
; HOW: A 16-bit add and a compare.
; WAITS: no.
; Entry: X object. Exit: the compare or the INC.
SlideAngleUp:
    lda a:OBJ_35C0,x
    clc
    adc #$C0
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    adc #$00
    sta a:OBJ_3568,x
    cmp #$10
    bcc SlideAngleUp_at0
    inc a:OBJ_2800,x
SlideAngleUp_at0:
    rts
; @routine 0CAB:422 AimAtPlayerThenArm
; @run 70:B422
; WHAT IT DOES: Takes OBJ_3510 as the angle to the player (a random one on three counts out of four, by OBJ_3670 and 3), sets the angle word from a table at $B44F by
; OBJ_3670 and 3, and moves OBJ_3670 and OBJ_2800 on.
; HOW: AngleToPlayer, NextRandom, table reads (bank 70).
; WAITS: no.
; Entry: X object. Exit: N and Z of the new OBJ_2800; A the table's low byte, Y the table index, C 0.
AimAtPlayerThenArm:
    jsr VecAngleToPlayer
    sta a:OBJ_3510,x
    lda a:OBJ_3670,x
    and #$03
    beq AimAtPlayerThenArm_at0
    jsr VecNextRandom
    sta a:OBJ_3510,x
AimAtPlayerThenArm_at0:
    lda a:OBJ_3670,x
    and #$03
    asl a
    tay
    lda a:$B450,y
    sta a:OBJ_3568,x
    lda a:$B44F,y
    sta a:OBJ_35C0,x
    inc a:OBJ_3670,x
    inc a:OBJ_2800,x
    rts

; @routine 0CAB:4AC InsideScreenBox
; @run 70:B4AC
; WHAT IT DOES: Says whether the object is inside the box X $20 to $DF, Y $20 to $BF on the first screen: C clear when inside, set when not.
; HOW: The position's high bytes ORed, then four compares.
; Untraced: the outside answer (B4CC).
; Curated cases: tools/rondo/curated/inside_screen_box.txt (150 inside cases; the draft reaches the untraced outside answer on the seeded ones).
; WAITS: no.
; Entry: X object. Exit: C clear when inside; A and the flags as the last compare or load left them.
InsideScreenBox:
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne InsideScreenBox_at0
    lda a:OBJ_X_LO,x
    cmp #$20
    bcc InsideScreenBox_at0
    cmp #$E0
    bcs InsideScreenBox_at0
    lda a:OBJ_Y_LO,x
    cmp #$20
    bcc InsideScreenBox_at0
    cmp #$C0
    bcs InsideScreenBox_at0
    clc
    rts
InsideScreenBox_at0:
    sec
    rts

; @routine 0CAB:4CE CountThenStop
; @run 70:B4CE
; WHAT IT DOES: Counts the object's timer down; at zero it clears OBJ_2800.
; HOW: DEC and a store.
; WAITS: no.
; Entry: X object. Exit: N and Z from the count.
CountThenStop:
    dec a:OBJ_34B8,x
    bne CountThenStop_at0
    stz a:OBJ_2800,x
CountThenStop_at0:
    rts

; @routine 0CAB:509 PickNextMoveBySide
; @run 70:B509
; WHAT IT DOES: Faces the player (bit 0 of SideOfPlayer), picks OBJ_2800 from a table at $B52C by the object's hit points halved (the high nibble) and a counter OBJ_3720 (the low
; nibble), and counts the counter up.
; HOW: SideOfPlayer, table read (bank 70).
; WAITS: no.
; Entry: X object. Exit: N and Z of the new counter.
PickNextMoveBySide:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
    lda a:OBJ_32A8,x
    lsr a
    and #$F0
    sta z:zTemp00
    lda a:OBJ_3720,x
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$B52C,y
    sta a:OBJ_2800,x
    inc a:OBJ_3720,x
    rts

; @routine 0CAB:55C StepThenSpawnShot
; @run 70:B55C
; WHAT IT DOES: Steps the animation list at $B582; when it ends, starts a kind $28 object at its place in slot $12 with a timer of 1 and its facing, and moves OBJ_2800 on.
; HOW: StepObjectFrames, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: list running: as the step; ended: N and Z of the new OBJ_2800.
StepThenSpawnShot:
    lda #$82
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenSpawnShot_at0
    ldy #$12
    jsr VecClearObjectInY
    lda #$28
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_34B8,y
    lda a:OBJ_2AC0,x
    sta a:OBJ_2AC0,y
    inc a:OBJ_2800,x
StepThenSpawnShot_at0:
    rts

; @routine 0CAB:5A0 ResetToStateThree
; @run 70:B5A0
; WHAT IT DOES: Sets the object's frame to 1, its state to 3 and clears OBJ_2800.
; HOW: Three stores.
; WAITS: no.
; Entry: X object. Exit: A 3, N and Z from it.
ResetToStateThree:
    lda #$01
    sta a:OBJ_2A68,x
    lda #$03
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
    rts

; @routine 0CAB:5AE StartThreeShots
; @run 70:B5AE
; WHAT IT DOES: Starts three kind $26 objects at this object's place in slots $13 to $15, each with this object's facing and its number 0 to 2 as its timer.
; HOW: A loop of ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A the last number, Y $16, N, Z and C from the compare of Y with $16.
StartThreeShots:
    stz z:zTemp00
    ldy #$13
StartThreeShots_at0:
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    lda a:OBJ_2AC0,x
    sta a:OBJ_2AC0,y
    lda z:zTemp00
    sta a:OBJ_34B8,y
    inc z:zTemp00
    iny
    cpy #$16
    bcc StartThreeShots_at0
    rts
