; @routine 072C:495 DissolveFall072C
; @run 70:BC95
; WHAT IT DOES: Every fourth count a dissolve step and falling; below the second screen becomes kind $22.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DissolveFall072C:
    lda #$D9
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_D,x
    and #$03
    bne FallThenBecome22_072C+$1F
    lda a:OBJ_WORK_O,x
    clc
    adc #$13
    and #$3F
    sta a:OBJ_WORK_O,x
    jsr EachOfPatterns072C_4E6
    lda a:OBJ_VY_LO,x
    clc

; @routine 088A:3B8 IntroList088A
; @run 70:ABB8
; WHAT IT DOES: The entrance: roars (sound $99) after a delay, its list (shorter once seen), then the music and a leap.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
IntroList088A:
    lda z:zWaveCounter
    and #$01
    ora a:OBJ_WORK_E,x
    bne IntroList088A_at0
    dec a:OBJ_WORK_D,x
    bne IntroList088A_at0
    lda #$01
    sta a:OBJ_WORK_E,x
    lda #$99
    jsr VecSoundCommand
IntroList088A_at0:
    lda #$31
    sta z:zTemp10
    lda #$AC
    sta z:zTemp11
    lda z:zField92
    beq IntroList088A_at1
    lda #$AE
    sta z:zTemp10
    lda #$AC
    sta z:zTemp11
IntroList088A_at1:
    jsr VecStepObjectFrames
    bne IntroList088A_at2
    rmb3 z:zField8E
    rmb4 z:zField8E
    jsr VecCdPlayAgain
    lda #$01
    sta z:zField92
    jsr StartThreeNumberedObjects
    lda #$FC
    sta a:OBJ_VY_HI,x
    inc a:OBJ_STATE,x
IntroList088A_at2:
    rts

; @routine 0EBF:760 LandAnimateByRise0EBF
; @run 71:D760
; WHAT IT DOES: Fades by OBJ_WORK_H, falls and lands (state on), showing frame $66 falling or the list rising.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandAnimateByRise0EBF:
    lda a:OBJ_WORK_H,x
    beq LandAnimateByRise0EBF_at0
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    clc
    adc #$8C
    sta a:OBJ_SPRITE_ATTR,x
    dec a:OBJ_WORK_H,x
LandAnimateByRise0EBF_at0:
    lda a:OBJ_Y_LO,x
    and #$F0
    cmp #$D0
    bne LandAnimateByRise0EBF_at1
    jsr Sound23ThenDebris0EBF
LandAnimateByRise0EBF_at1:
    lda #$0A
    sta z:zProduct1
    jsr VecStopOnLanding
    bcc LandAnimateByRise0EBF_at2
    stz a:OBJ_SUBSTATE,x
    inc a:OBJ_STATE,x
LandAnimateByRise0EBF_at2:
    lda a:OBJ_VY_HI,x
    bmi LandAnimateByRise0EBF_at3
    lda #$66
    sta a:OBJ_FRAME,x
    rts
LandAnimateByRise0EBF_at3:
    lda #$A4
    sta z:zTemp10
    lda #$D7
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0F6C:4FA ChaseOrLeap0F6C
; @run 70:BCFA
; WHAT IT DOES: Runs toward the player; turned away or at an edge, turns or goes to state 5.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ChaseOrLeap0F6C:
    jsr StartLeapWhenHurtAnswerYes
    bcc ChaseOrLeap0F6C_at0
    rts
ChaseOrLeap0F6C_at0:
    jsr HurtOnceThenAnswerYes
    bcc ChaseOrLeap0F6C_at1
    rts
ChaseOrLeap0F6C_at1:
    jsr StepListByNumberTableC585
    jsr VecApplyVelocityX
    jsr VecDistanceXToPlayer
    cmp #$60
    bcc ChaseOrLeap0F6C_at2
    jsr VecSideOfPlayer
    and #$01
    eor a:OBJ_FACING_LEFT,x
    bne ChaseOrLeap0F6C_at3
ChaseOrLeap0F6C_at2:
    jsr ProbeTileSideBit1
    bcc ChaseOrLeap0F6C_at4
    lda a:OBJ_Y_LO
    cmp #$9A
    beq ChaseOrLeap0F6C_at3
    jsr ToggleFacing
    jmp VecNegateVelocityX
ChaseOrLeap0F6C_at4:
    rts
ChaseOrLeap0F6C_at3:
    lda #$0C
    sta a:OBJ_WORK_D,x
    jsr ClearAnimationThenStepListC585
    lda #$05
    sta a:OBJ_STATE,x
    rts

; @routine 101D:09D GateOnScreen4_101D
; @run 71:C09D
; WHAT IT DOES: On screen 4 shuts the way behind (sound $57); $20 counts on, hit-tested and the state on.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
GateOnScreen4_101D:
    lda a:OBJ_SUBSTATE,x
    bne GateOnScreen4_101D_at0
    lda a:ScrollXHiScreen
    cmp #$04
    bne GateOnScreen4_101D_at1
    lda #$57
    jsr VecSoundCommand
    lda #$04
    sta a:ScrollXMinHi
    lda a:OBJ_START_ARG,x
    clc
    adc #$9C
    sta a:OBJ_FRAME,x
    lda a:OBJ_SPRITE_ATTR,x
    ora #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda #$20
    sta a:OBJ_WORK_J,x
    inc a:OBJ_SUBSTATE,x
GateOnScreen4_101D_at1:
    rts
GateOnScreen4_101D_at0:
    dec a:OBJ_WORK_J,x
    bne GateOnScreen4_101D_at1
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    stz a:OBJ_SPRITE_ATTR,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 1180:05A ArmByLeader1180
; @run 71:D85A
; WHAT IT DOES: A limb of the kind $34 object before it: its place, facing, hit test and the box for its frame.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ArmByLeader1180Boxes = $D8DA ; hit box by the leader's frame
ArmByLeader1180:
    lda a:OBJ_STATE,x
    bne ArmByLeader1180_at0
    jmp VecLoadObjectRecord
ArmByLeader1180_at0:
    lda a:OBJ_KIND-1,x
    cmp #$34
    beq ArmByLeader1180_at1
    jmp VecClearObject
ArmByLeader1180_at1:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FRAME-1,x
    sec
    sbc #$02
    cmp #$22
    bcc ArmByLeader1180_at2
    lda #$22
ArmByLeader1180_at2:
    asl a
    asl a
    tay
    lda a:ArmByLeader1180Boxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:ArmByLeader1180Boxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:ArmByLeader1180Boxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:ArmByLeader1180Boxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:OBJ_FACING_LEFT-1,x
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_HITBOX_ON-1,x
    sta a:OBJ_HITBOX_ON,x
    rts

; @routine 18DC:27D ShieldOfLeader18DC
; @run 70:BA7D
; WHAT IT DOES: Rides the kind $A4 object in slot $10 with the box its start argument picks; gone without it.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ShieldOfLeader18DCBoxes = $BAC0 ; hit box by argument, four bytes
ShieldOfLeader18DC:
    lda a:OBJ_STATE,x
    bne ShieldOfLeader18DC_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    tay
    lda a:ShieldOfLeader18DCBoxes,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:ShieldOfLeader18DCBoxes+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:ShieldOfLeader18DCBoxes+2,y
    sta a:OBJ_HITBOX_DX,x
    lda a:ShieldOfLeader18DCBoxes+3,y
    sta a:OBJ_HITBOX_DY,x
    rts
ShieldOfLeader18DC_at0:
    ldy #$10
    lda a:OBJ_KIND,y
    cmp #$A4
    beq ShieldOfLeader18DC_at1
    jmp VecClearObject
ShieldOfLeader18DC_at1:
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FACING_LEFT+$10
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_HITBOX_ON+$10
    sta a:OBJ_HITBOX_ON,x
    rts
