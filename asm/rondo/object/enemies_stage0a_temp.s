; @routine 0A9D:01F BreakWhenHit
; @run 71:C01F
; WHAT IT DOES: An object in a state below $0B whose status bit 7 is set clears the object after it, stops its hitbox and animation,
; sets OBJ_3778 to $40 and enters state $0B.
; HOW: Reads the state and the status flags; ClearObject on slot plus one.
; WAITS: no.
; Entry: X object. Exit: A $0B, N and Z from it, C 0.
BreakWhenHit:
    lda a:OBJ_27A8,x
    cmp #$0B
    bcs BreakWhenHit_at0
    lda a:OBJ_31A0,x
    and #$80
    beq BreakWhenHit_at0
    inx
    jsr VecClearObject
    dex
    stz a:OBJ_3250,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$40
    sta a:OBJ_3778,x
    lda #$0B
    sta a:OBJ_27A8,x
BreakWhenHit_at0:
    rts

; @routine 0A9D:070 StartWithPartner
; @run 71:C070
; WHAT IT DOES: Loads the object's record, sets OBJ_3510 to $B0 and its angle byte to $40, and starts a kind $25 object at its place in the slot
; after it.
; HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: A the last byte copied, Y the slot plus one, C from the record.
StartWithPartner:
    jsr VecLoadObjectRecord
    lda #$B0
    sta a:OBJ_3510,x
    lda #$40
    sta a:OBJ_3568,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$25
    jmp VecStartObjectAtPlace

; @routine 0A9D:088 StopWhenLanded
; @run 71:C088
; WHAT IT DOES: Unless slot $54's frame is 4, asks StopOnLanding with a probe height of 4; on landing the object's animation is reset and it enters
; its next state.
; HOW: StopOnLanding, two stores, an INC.
; Untraced: one byte.
; WAITS: no.
; Entry: X object, Y the probe's column, C passed on. Exit: the mover's registers, then N and Z of the new state when landed.
StopWhenLanded:
    lda a:$2ABC
    cmp #$04
    beq StopWhenLanded_at0
    lda #$04
    jsr VecStopOnLanding
    bcc StopWhenLanded_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_27A8,x
StopWhenLanded_at0:
    rts

; @routine 0A9D:113 SlowToPlayer
; @run 71:C113
; WHAT IT DOES: When the player is within 5 along X, the object's X speed becomes that distance and its direction byte OBJ_3618 the
; side the player is on (inverted).
; HOW: DistanceXToPlayer, SideOfPlayer.
; WAITS: no.
; Entry: X object. Exit: the compare with 5, or A the flipped side bit.
SlowToPlayer:
    jsr VecDistanceXToPlayer
    cmp #$05
    bcs SlowToPlayer_at0
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    jsr VecSideOfPlayer
    and #$01
    eor #$01
    sta a:OBJ_3618,x
SlowToPlayer_at0:
    rts

; @routine 0A9D:133 ChooseAttackByDistance
; @run 71:C133
; WHAT IT DOES: When the player is within $20 vertically the object resets its animation, sets OBJ_3720 1 and enters state 4; else when within
; $18 along X it resets, sets OBJ_3720 2 and enters state 7.
; HOW: DistanceYToPlayer, DistanceXToPlayer.
; WAITS: no.
; Entry: X object. Exit: the last compare or load.
ChooseAttackByDistance:
    jsr VecDistanceYToPlayer
    cmp #$20
    bcs ChooseAttackByDistance_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$01
    sta a:OBJ_3720,x
    lda #$04
    sta a:OBJ_27A8,x
    rts
ChooseAttackByDistance_at0:
    jsr VecDistanceXToPlayer
    cmp #$18
    bcs ChooseAttackByDistance_at1
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$02
    sta a:OBJ_3720,x
    lda #$07
    sta a:OBJ_27A8,x
ChooseAttackByDistance_at1:
    rts

; @routine 0A9D:1CF ChooseNextMove
; @run 71:C1CF
; WHAT IT DOES: While the object in slot 0 is in OBJ_2800 state 1 and the player is $50 or more away along X and the distance sum is even, the
; object resets its animation and enters state 9; otherwise its wait OBJ_3568 counts down and at zero it resets, picks a new wait from the table
; at $C226 by the distance sum and its next state from the table at $C223 by a counter OBJ_37D0, which goes up one.
; HOW: DistanceXToPlayer, PlayerDistanceSum (twice, with the carry the compares left), table reads (bank 71).
; WAITS: no.
; Entry: X object, C entry (it reaches PlayerDistanceSum when a compare did not set it). Exit: the last compare, AND, DEC or INC.
ChooseNextMove:
    lda a:OBJ_2800
    cmp #$01
    bne ChooseNextMove_at0
    jsr VecDistanceXToPlayer
    cmp #$50
    bcc ChooseNextMove_at0
    jsr VecPlayerDistanceSum
    and #$01
    bne ChooseNextMove_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$09
    sta a:OBJ_27A8,x
    rts
ChooseNextMove_at0:
    dec a:OBJ_3568,x
    bne ChooseNextMove_at1
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    jsr VecPlayerDistanceSum
    and #$03
    tay
    lda a:$C226,y
    sta a:OBJ_3568,x
    lda a:OBJ_37D0,x
    and #$07
    tay
    lda a:$C21B,y
    tay
    lda a:$C223,y
    sta a:OBJ_27A8,x
    inc a:OBJ_37D0,x
ChooseNextMove_at1:
    rts

; @routine 0A9D:28A TurnWhenWaitEnds
; @run 71:C28A
; WHAT IT DOES: Counts the angle byte down; at zero resets the animation, takes OBJ_3670 from the player's side (bit 1, vertical) and enters state 8.
; HOW: DEC, SideOfPlayer and a shift.
; WAITS: no.
; Entry: X object. Exit: wait running: N and Z from the count; at zero A $08, C bit 0 of the sides.
TurnWhenWaitEnds:
    dec a:OBJ_3568,x
    bne TurnWhenWaitEnds_at0
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    jsr VecSideOfPlayer
    lsr a
    sta a:OBJ_3670,x
    lda #$08
    sta a:OBJ_27A8,x
TurnWhenWaitEnds_at0:
    rts

; @routine 0A9D:2A2 StepAndShout
; @run 71:C2A2
; WHAT IT DOES: Steps the animation list a table at $C2E4 names for OBJ_3670; when the list ends it resets, enters state 7 and picks a wait
; from the table at $C2E0 by the distance sum; on a new frame $26 or $2D it plays sound $1F if the object is on the first screen.
; HOW: StepObjectFrames, PlayerDistanceSum, a tail jump into SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
StepAndShout:
    lda a:OBJ_3670,x
    asl a
    tay
    lda a:$C2E4,y
    sta z:zTemp10
    lda a:$C2E5,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepAndShout_at0
    bcc StepAndShout_at1
    lda a:OBJ_2A68,x
    cmp #$26
    beq StepAndShout_at2
    cmp #$2D
    bne StepAndShout_at1
StepAndShout_at2:
    lda #$1F
    jmp VecSoundIfOnFirstScreen
StepAndShout_at0:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$07
    sta a:OBJ_27A8,x
    jsr VecPlayerDistanceSum
    and #$03
    tay
    lda a:$C2E0,y
    sta a:OBJ_3568,x
StepAndShout_at1:
    rts

; @routine 0A9D:31E StepWithArmor
; @run 71:C31E
; WHAT IT DOES: Sets bits 0 and 1 of the object's OBJ_3148 while it steps the animation list at $C347; when the list ends it clears them, resets
; the animation and enters state 5.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the step; at the end A 5.
StepWithArmor:
    lda a:OBJ_3148,x
    ora #$03
    sta a:OBJ_3148,x
    lda #$C3
    sta z:zTemp11
    lda #$47
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne StepWithArmor_at0
    lda a:OBJ_3148,x
    and #$FC
    sta a:OBJ_3148,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$05
    sta a:OBJ_27A8,x
StepWithArmor_at0:
    rts

; @routine 0A9D:3DA FlashAndCount
; @run 71:C3DA
; @stop C401
; WHAT IT DOES: Flashes the object ($8A), steps the animation list at $C401, counts OBJ_3778 down and clears the object at zero; while it runs, when
; the count equals one of eight cue values (table at $C43B) it shoots an arrow with that cue's index.
; HOW: StepObjectFrames, DEC, a scan of the table, ShootArrow (reached by a branch).
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
FlashAndCount:
    lda #$8A
    sta a:OBJ_2B18,x
    lda #$01
    sta z:zTemp10
    lda #$C4
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_3778,x
    bne FlashAndCount_at0
    jmp VecClearObject
FlashAndCount_at0:
    cly
FlashAndCount_at1:
    lda a:OBJ_3778,x
    cmp a:$C43B,y
    beq ShootArrow
    iny
    cpy #$08
    bcc FlashAndCount_at1
    rts

; @routine 0A9D:410 ShootArrow
; @run 71:C410
; WHAT IT DOES: Starts a kind $9C object at this object's place in the first free slot from $30, plays sound $15 if it is on the first screen,
; gives the new object an X and Y speed from a table at $C443 (two bytes by the cue's index doubled) and moves it once.
; HOW: The slot and the index are kept in zRichterObject and zHitSlot; FindFreeSlotFrom30, StartObjectAtPlace, SoundIfOnFirstScreen,
; ApplyVelocity.
; Untraced: one byte.
; WAITS: no.
; Entry: X object, Y the cue's index. Exit: see the wrapper.
ShootArrow:
    stx z:zRichterObject
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom30
    bcc LaunchHopByF3510
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$15
    jsr VecSoundIfOnFirstScreen
    lda z:zHitSlot
    asl a
    tax
    lda a:$C443,x
    sta a:OBJ_VX_HI,y
    lda a:$C444,x
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zRichterObject
    rts
