; @routine 01BA:6F3 RichterReplaceSlotsOneTwo
; @run 6D:AEF3
; WHAT IT DOES: Gives object slots 1 and 2 kind 4, each emptied first, unless it holds kind 1 or 2.
; HOW: OBJ_KIND compared with 1 and 2; RichterClearSlotKindFour for slot 1, then a run on into it for
; slot 2. A kind 1 or 2 in slot 2 ends at RichterClearSlotKindFour's RTS.
; WAITS: no.
; Entry: nothing. Exit: X 2. Slot 2 kept: A its kind, N, Z and C from comparing it with 1 or 2; else
; A 4, N and Z from it, C from comparing the kind with 2. Y and V kept.
RichterReplaceSlotsOneTwo:
    ldx #$01
    lda a:OBJ_KIND,x
    cmp #$01
    beq RichterReplaceSlotsOneTwo_at0
    cmp #$02
    beq RichterReplaceSlotsOneTwo_at0
    bsr RichterClearSlotKindFour
RichterReplaceSlotsOneTwo_at0:
    inx
    lda a:OBJ_KIND,x
    cmp #$01
    beq RichterClearSlotKindFour_at0
    cmp #$02
    beq RichterClearSlotKindFour_at0

; @routine 01BA:70E RichterClearSlotKindFour
; @run 6D:AF0E
; WHAT IT DOES: Empties an object slot and gives it kind 4.
; HOW: ClearObject (through its kernel vector), then 4 into OBJ_KIND.
; WAITS: no.
; Entry: X object. Exit: A 4, N and Z from it. X, Y, C and V kept.
RichterClearSlotKindFour:
    jsr VecClearObject
    lda #$04
    sta a:OBJ_KIND,x
RichterClearSlotKindFour_at0:
    rts

; @routine 01BB:245 RichterHurtStart
; @run 6D:B245
; WHAT IT DOES: The hit's step 0: stands him on a floor a pixel below if there is one, clears the
; player bytes the hit ends, works out which way the hit throws him, and goes on by the hit's kind
; (3 and 4 throw him) or else by the state he was hit in. A spike (kind 6) throws him back from his
; facing and is taken as kind 2.
; HOW: The floor checks twice; RichterLandOnFloor; OBJ_KNOCK_RIGHT 1 when the object that hit him
; (OBJ_HIT_BY_SLOT) is left of him; RichterStopMoving; the thirteen words at $B2C1 by
; OBJ_STATE_BEFORE_HIT.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as the routine it goes to.
RichterHurtStart:
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcs RichterHurtStart_at0
    inc a:OBJ_Y_LO
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcc RichterHurtStart_at1
RichterHurtStart_at0:
    jsr RichterLandOnFloor
RichterHurtStart_at1:
    jsr RichterPadOverridesOff
    stz a:PlayerAttackKind
    stz a:PlayerHeld
    stz a:PlayerHeldCount
    stz a:PlayerSlowTimerLo
    stz a:PlayerSlowTimerHi
    stz a:PlayerJumpKind
    lda a:OBJ_FACING_LEFT
    sta a:OBJ_KNOCK_RIGHT
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$06
    bne RichterHurtStart_at2
    lda #$02
    sta a:OBJ_HIT_TAKEN
    bra RichterHurtStart_at3
RichterHurtStart_at2:
    stz a:OBJ_KNOCK_RIGHT
    ldx a:OBJ_HIT_BY_SLOT
    lda a:OBJ_X_HI,x
    beq RichterHurtStart_at4
    bpl RichterHurtStart_at3
    inc a:OBJ_KNOCK_RIGHT
    bra RichterHurtStart_at3
RichterHurtStart_at4:
    lda a:OBJ_X_LO,x
    cmp a:OBJ_X_LO
    bcs RichterHurtStart_at3
    inc a:OBJ_KNOCK_RIGHT
RichterHurtStart_at3:
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$03
    bne RichterHurtStart_at5
    jmp RichterHurtKindThree
RichterHurtStart_at5:
    cmp #$04
    bne RichterHurtStart_at6
    jmp RichterHurtKindFour
RichterHurtStart_at6:
    jsr RichterStopMoving
    lda a:OBJ_STATE_BEFORE_HIT
    asl a
    tax
    jmp (RichterHurtByState,x)

; @routine 01BB:2DB RichterHurtFromStand
; @run 6D:B2DB
; WHAT IT DOES: A hit while standing or walking: knocked up when he was at step 1 or the hit's low
; bits are not 1; put down otherwise.
; HOW: OBJ_STEP_BEFORE_HIT against 1, bits 0 and 1 of OBJ_HIT_TAKEN against 1; branches to
; RichterHurtKnockUp and RichterHurtDown, and runs on into RichterHurtKnockUp.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as the routine it goes to, from the last compare.
RichterHurtFromStand:
    lda a:OBJ_STEP_BEFORE_HIT
    cmp #$01
    beq RichterHurtKnockUp
    lda a:OBJ_HIT_TAKEN
    and #$03
    cmp #$01
    beq RichterHurtDown

; @routine 01BB:2EB RichterHurtKnockUp
; @run 6D:B2EB
; WHAT IT DOES: Knocks him up: Y speed $FC00, step 2, frame $46, then the hit's sound and effects.
; HOW: A tail jump to RichterHitSoundAndEffects.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as RichterHitSoundAndEffects, A $46 where it keeps A.
RichterHurtKnockUp:
    lda #$00
    sta a:OBJ_VY_LO
    lda #$FC
    sta a:OBJ_VY_HI
    lda #$02
    sta a:OBJ_SUBSTATE
    lda #$46
    sta a:OBJ_FRAME
    jmp RichterHitSoundAndEffects

; @routine 01BB:302 RichterHurtDown
; @run 6D:B302
; WHAT IT DOES: Puts him down: step 6 with animation $1E, then the hit's sound and effects.
; HOW: RichterSetAnimation; a tail jump to RichterHitSoundAndEffects.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as RichterHitSoundAndEffects, from what
; RichterSetAnimation left.
RichterHurtDown:
    lda #$06
    sta a:OBJ_SUBSTATE
    lda #$1E
    jsr RichterSetAnimation
    jmp RichterHitSoundAndEffects

; @routine 01BB:30F RichterHurtFromCrouch
; @run 6D:B30F
; WHAT IT DOES: A hit while crouched: knocked up unless at step 0; otherwise a slide, step $0B when
; the hit's low bits are 1, else step 1, with frame $72 for 8 frames; then the hit's sound and
; effects.
; HOW: A branch to RichterHurtKnockUp; OBJ_HIT_FRAMES; a tail jump to RichterHitSoundAndEffects.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as RichterHurtKnockUp, or as RichterHitSoundAndEffects
; with A 8 where it keeps A.
RichterHurtFromCrouch:
    lda a:OBJ_STEP_BEFORE_HIT
    bne RichterHurtKnockUp
    lda a:OBJ_HIT_TAKEN
    and #$03
    cmp #$01
    bne RichterHurtFromCrouch_at0
    lda #$0B
    bra RichterHurtFromCrouch_at1
RichterHurtFromCrouch_at0:
    lda #$01
RichterHurtFromCrouch_at1:
    sta a:OBJ_SUBSTATE
    lda #$72
    sta a:OBJ_FRAME
    lda #$08
    sta a:OBJ_HIT_FRAMES
    jmp RichterHitSoundAndEffects

; @routine 01BB:333 RichterHurtInAir
; @run 6D:B333
; WHAT IT DOES: A hit in the air or in the hit's own state: knocked up, at Y speed $FD00.
; HOW: RichterHurtKnockUp, then the speed's high byte rewritten.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: A $FD, N set, Z clear; X, Y, C and V as
; RichterHurtKnockUp left them.
RichterHurtInAir:
    jsr RichterHurtKnockUp
    lda #$00
    sta a:OBJ_VY_LO
    lda #$FD
    sta a:OBJ_VY_HI
    rts

; @routine 01BB:341 RichterHurtClimbKindFive
; @run 6D:B341
; WHAT IT DOES: A hit of kind 5 while climbing: nudged off by the second table for states 5 and 7,
; the first for 6 and 8; knocked up at Y speed $FB00.
; HOW: TST of bit 0 of OBJ_STATE_BEFORE_HIT; RichterNudgeB or RichterNudgeA; RichterHurtKnockUp.
; WAITS: no.
; Entry: Y for the sound driver. Exit: A $FB, N set, Z clear; X, Y, C and V as RichterHurtKnockUp
; left them.
RichterHurtClimbKindFive:
    tst #$01,a:OBJ_STATE_BEFORE_HIT
    bne RichterHurtClimbKindFive_at0
    jsr RichterNudgeB
    bra RichterHurtClimbKindFive_at1
RichterHurtClimbKindFive_at0:
    jsr RichterNudgeA
RichterHurtClimbKindFive_at1:
    jsr RichterHurtKnockUp
    lda #$00
    sta a:OBJ_VY_LO
    lda #$FB
    sta a:OBJ_VY_HI
    rts

; @routine 01BB:35D RichterHurtWhileClimbing
; @run 6D:B35D
; @stop B37D
; WHAT IT DOES: A hit in states 5 to 8: kind 5 as RichterHurtClimbKindFive; at steps 2 to 4 of the
; state as a hit while standing; otherwise held for $20 frames at step 3, then the hit's sound and
; effects.
; HOW: The six bytes at $B37D by OBJ_STEP_BEFORE_HIT; branches to RichterHurtClimbKindFive and
; RichterHitSoundAndEffects, a jump to RichterHurtFromStand.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as the routine it goes to; held, A 3 where
; RichterHitSoundAndEffects keeps A, Y the step.
RichterHurtWhileClimbing:
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$05
    beq RichterHurtClimbKindFive
    ldy a:OBJ_STEP_BEFORE_HIT
    lda a:RichterClimbStepsHurtAsStanding,y
    beq RichterHurtWhileClimbing_at0
    jmp RichterHurtFromStand
RichterHurtWhileClimbing_at0:
    lda #$20
    sta a:OBJ_HIT_FRAMES
    lda #$03
    sta a:OBJ_SUBSTATE
    bra RichterHitSoundAndEffects

; @routine 01BB:383 RichterHitSoundAndEffects
; @run 6D:B383
; WHAT IT DOES: Sound $62 for a hit with neither bit 5 nor bit 6 of OBJ_HIT_TAKEN, then
; RichterHitEffects.
; HOW: A TST of OBJ_HIT_TAKEN with $60; SoundCommand through its kernel vector; runs on into
; RichterHitEffects.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as RichterHitEffects, from what SoundCommand left when
; it ran.
RichterHitSoundAndEffects:
    tst #$60,a:OBJ_HIT_TAKEN
    bne RichterHitEffects
    lda #$62
    jsr VecSoundCommand

; @routine 01BB:38E RichterHitEffects
; @run 6D:B38E
; WHAT IT DOES: What a hit does besides hurting, by bits 4 to 6 of OBJ_HIT_TAKEN, the first set of
; them in that order; then OBJ_HIT_TAKEN is cleared. Bit 4: slots 1 and 2 emptied as kind 1, slot
; 1's OBJ_ANIM_TIMER $0A, script $0A. Bit 5: sound $12, PlayerHeld counted up, script 1. Bit 6:
; sound $24, PlayerSlowTimerLo $FF and PlayerSlowTimerHi 1, script $17.
; HOW: Three TSTs of OBJ_HIT_TAKEN; ClearObject and SoundCommand through their kernel vectors; a tail
; jump to RichterStartHitScript.
; WAITS: no.
; Entry: X and Y for the sound driver as SoundCommand takes them. Exit: no bit: OBJ_HIT_TAKEN 0, Z
; set, N bit 7 of OBJ_HIT_TAKEN as the last TST read it, V clear; A, X, Y and C kept. Otherwise as
; RichterStartHitScript with the script: C kept; V bit 6 of OBJ_HIT_TAKEN for bit 4, as SoundCommand
; left it for bits 5 and 6.
RichterHitEffects:
    tst #$10,a:OBJ_HIT_TAKEN
    bne RichterHitEffects_at0
    tst #$20,a:OBJ_HIT_TAKEN
    bne RichterHitEffects_at1
    tst #$40,a:OBJ_HIT_TAKEN
    bne RichterHitEffects_at2
    stz a:OBJ_HIT_TAKEN
    rts
RichterHitEffects_at0:
    ldx #$01
    jsr VecClearObject
    inx
    jsr VecClearObject
    lda #$01
    sta a:OBJ_KIND+1
    sta a:OBJ_KIND+2
    lda #$0A
    sta a:OBJ_ANIM_TIMER+1
    lda #$0A
RichterHitEffects_at3:
    stz a:OBJ_HIT_TAKEN
    jmp RichterStartHitScript
RichterHitEffects_at1:
    lda #$12
    jsr VecSoundCommand
    inc a:PlayerHeld
    lda #$01
    bra RichterHitEffects_at3
RichterHitEffects_at2:
    lda #$24
    jsr VecSoundCommand
    lda #$FF
    sta a:PlayerSlowTimerLo
    lda #$01
    sta a:PlayerSlowTimerHi
    lda #$17
    bra RichterHitEffects_at3

; @routine 01BB:3E1 RichterHurtKindThree
; @run 6D:B3E1
; WHAT IT DOES: A hit of kind 3 throws him: up at $FE50 if he was in the air, else lifted three
; pixels and up at $FC50; across at $FD80 or $0280 by OBJ_KNOCK_RIGHT; step 7.
; HOW: The thirteen bytes at $B428 by OBJ_STATE_BEFORE_HIT (1 for states 3, 4, 11 and 12); RichterLiftThree;
; the two words at $B435; RichterSetAnimation $1F, SoundCommand $63, a tail jump to RichterHitEffects.
; WAITS: no.
; Entry: Y for the sound driver. Exit: as RichterHitEffects.
RichterHurtKindThree:
    ldx a:OBJ_STATE_BEFORE_HIT
    lda a:RichterHurtFromAirStates,x
    beq RichterHurtKindThree_at0
    lda #$50
    sta a:OBJ_VY_LO
    lda #$FE
    sta a:OBJ_VY_HI
    bra RichterHurtKindThree_at1
RichterHurtKindThree_at0:
    jsr RichterLiftThree
    lda #$50
    sta a:OBJ_VY_LO
    lda #$FC
    sta a:OBJ_VY_HI
RichterHurtKindThree_at1:
    lda a:OBJ_KNOCK_RIGHT
    asl a
    tay
    lda a:RichterThrowSpeedsX,y
    sta a:OBJ_VX_LO
    lda a:RichterThrowSpeedsX+1,y
    sta a:OBJ_VX_HI
    lda #$07
RichterHurtKindThree_at2:
    sta a:OBJ_SUBSTATE
    lda #$1F
    jsr RichterSetAnimation
    stz a:PlayerMayLeaveTop
    lda #$63
    jsr VecSoundCommand
    jmp RichterHitEffects

; @routine 01BB:439 RichterHurtKindFour
; @run 6D:B439
; WHAT IT DOES: A hit of kind 4 throws him: up at $F980, lifted three pixels, right at $0380; step 9.
; HOW: RichterLiftThree, then RichterHurtKindThree's end from $B413.
; WAITS: no.
; Entry: Y for the sound driver. Exit: as RichterHitEffects.
RichterHurtKindFour:
    ldx a:OBJ_STATE_BEFORE_HIT
    lda #$80
    sta a:OBJ_VY_LO
    lda #$F9
    sta a:OBJ_VY_HI
    jsr RichterLiftThree
    lda #$80
    sta a:OBJ_VX_LO
    lda #$03
    sta a:OBJ_VX_HI
    lda #$09
    bra RichterHurtKindThree_at2

; @routine 01BB:457 RichterHurtThrown
; @run 6D:B457
; WHAT IT DOES: The throw's steps 7 and 9: falls with gravity and the animation until he lands on a
; floor on the first screen row while falling, its column answering 0; then he stands on it, slots 1 and 2 go to kind
; 4, and the next step starts with animation $20 (and sound 2) if the animation had stopped, else
; $21; its X speed by OBJ_KNOCK_RIGHT, or right from step $0A.
; HOW: RichterReadColumns, RichterFloorCheck, RichterSolidUnderColumn, RichterColumnRow;
; RichterDebrisOnMarkedColumns while in the air; RichterFallGravity, a tail jump to
; RichterAnimationTick; the two words at $B4C1.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: in the air, as RichterAnimationTick. Landed, A the
; speed's high byte, N and Z from it, Y twice the index, C bit 7 of the index; X 2; V as
; RichterSetAnimation left it.
RichterHurtThrown:
    jsr RichterReadColumns
    lda a:OBJ_Y_HI
    bne RichterHurtThrown_at0
    lda a:OBJ_VY_HI
    bmi RichterHurtThrown_at0
    jsr RichterFloorCheck
    bcc RichterHurtThrown_at1
    jsr RichterSolidUnderColumn
    bne RichterHurtThrown_at1
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    stz a:OBJ_VY_LO
    stz a:OBJ_VY_HI
    jsr RichterReplaceSlotsOneTwo
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    bne RichterHurtThrown_at2
    lda #$02
    jsr VecSoundCommand
    lda #$20
    bra RichterHurtThrown_at3
RichterHurtThrown_at1:
    jsr RichterDebrisOnMarkedColumns
RichterHurtThrown_at0:
    jsr RichterFallGravity
    jmp RichterAnimationTick
RichterHurtThrown_at2:
    lda #$21
RichterHurtThrown_at3:
    jsr RichterSetAnimation
    inc a:OBJ_SUBSTATE
    lda a:OBJ_SUBSTATE
    cmp #$0A
    beq RichterHurtThrown_at4
    bra RichterHurtThrown_at5
RichterHurtThrown_at4:
    lda #$01
    bra RichterHurtThrown_at6
RichterHurtThrown_at5:
    lda a:OBJ_KNOCK_RIGHT
RichterHurtThrown_at6:
    asl a
    tay
    lda a:RichterThrownLandSpeedsX,y
    sta a:OBJ_VX_LO
    lda a:RichterThrownLandSpeedsX+1,y
    sta a:OBJ_VX_HI
    rts

; @routine 01BB:4C5 RichterHurtThrownLanded
; @run 6D:B4C5
; WHAT IT DOES: The throw's steps 8 and $0A: slots 1 and 2 to kind 4 every eighth frame; while a
; floor is under him or a pixel below, stands on it and, once the animation ends, crouches; with no
; floor goes back a step.
; HOW: TST of zWaveCounter's low three bits; RichterReplaceSlotsOneTwo; the floor checks twice;
; RichterLandOnFloor, RichterAnimationWait, a tail jump to RichterHurtToCrouch.
; WAITS: no.
; Entry: nothing. Exit: back a step, N and Z from the step. Landed, as RichterAnimationWait, or as
; RichterHurtToCrouch.
RichterHurtThrownLanded:
    tst #$07,z:zWaveCounter
    bne RichterHurtThrownLanded_at0
    jsr RichterReplaceSlotsOneTwo
RichterHurtThrownLanded_at0:
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcs RichterHurtThrownLanded_at1
    inc a:OBJ_Y_LO
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcc RichterHurtThrownLanded_at2
RichterHurtThrownLanded_at1:
    jsr RichterLandOnFloor
    jsr RichterAnimationWait
    jmp RichterHurtToCrouch
RichterHurtThrownLanded_at2:
    dec a:OBJ_SUBSTATE
    rts

; @routine 01BB:52C RichterHurtLandSound
; @run 6D:B52C
; WHAT IT DOES: Slots 1 and 2 to kind 4, then sound $4A.
; HOW: RichterReplaceSlotsOneTwo, then a tail jump to SoundCommand (through its kernel vector).
; WAITS: no.
; Entry: X and Y for the sound driver as SoundCommand takes them (X is 2 by then). Exit: as SoundCommand
; with $4A, from what RichterReplaceSlotsOneTwo left.
RichterHurtLandSound:
    jsr RichterReplaceSlotsOneTwo
    lda #$4A
    jmp VecSoundCommand

; @routine 01BB:562 RichterKnockSpeedXFromHit
; @run 6D:B562
; WHAT IT DOES: RichterKnockSpeedX for the side the hit throws him to, OBJ_KNOCK_RIGHT.
; HOW: Runs on into RichterKnockSpeedX.
; WAITS: no.
; Entry: nothing. Exit: as RichterKnockSpeedX.
RichterKnockSpeedXFromHit:
    lda a:OBJ_KNOCK_RIGHT

; @routine 01BB:565 RichterKnockSpeedX
; @run 6D:B565
; WHAT IT DOES: Gives him one pixel a frame of X speed: left for 0, right for 1.
; HOW: Two words at $B5AA, $FF00 and $0100, by twice A.
; WAITS: no.
; Entry: A 0 or 1. Exit: A the speed's high byte, N and Z from it; Y twice the entry; C bit 7 of
; the entry. X and V kept.
RichterKnockSpeedX:
    asl a
    tay
    lda a:RichterKnockSpeedsX,y
    sta a:OBJ_VX_LO
    lda a:RichterKnockSpeedsX+1,y
    sta a:OBJ_VX_HI
    rts

; @routine 01BB:574 RichterHurtSlideStart
; @run 6D:B574
; WHAT IT DOES: The slide's first step: the knock speed, then as RichterHurtSlide.
; HOW: RichterKnockSpeedXFromHit, then runs on into RichterHurtSlide.
; WAITS: no.
; Entry: nothing. Exit: as RichterHurtSlide.
RichterHurtSlideStart:
    bsr RichterKnockSpeedXFromHit

; @routine 01BB:576 RichterHurtSlide
; @run 6D:B576
; WHAT IT DOES: The slide's step: falls if no floor is under him or a pixel below; else stands on it
; and, when OBJ_HIT_FRAMES runs out, crouches.
; HOW: RichterFloorCheck, again after RichterReadColumns a pixel lower; a tail jump to RichterToFall;
; RichterLandOnFloor; a branch to RichterHurtToCrouch.
; WAITS: no.
; Entry: nothing. Exit: as RichterToFall; counting, N and Z from the count; as RichterHurtToCrouch.
RichterHurtSlide:
    jsr RichterFloorCheck
    bcs RichterHurtSlide_at0
    inc a:OBJ_Y_LO
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcs RichterHurtSlide_at0
    jmp RichterToFall
RichterHurtSlide_at0:
    jsr RichterLandOnFloor
    dec a:OBJ_HIT_FRAMES
    beq RichterHurtToCrouch
    rts

; @routine 01BB:592 RichterHurtToCrouch
; @run 6D:B592
; WHAT IT DOES: Ends a reaction in a crouch: the crouched frame, the animation stopped, no speed,
; state 2 at step 0.
; HOW: OBJ_FRAME $0E, OBJ_HURT_TIMER $FF, RichterStopMoving, OBJ_STATE and OBJ_SUBSTATE.
; WAITS: no.
; Entry: nothing. Exit: A 0, N clear, Z set. X, Y, C and V kept.
RichterHurtToCrouch:
    lda #$0E
    sta a:OBJ_FRAME
    lda #$FF
    sta a:OBJ_HURT_TIMER
    jsr RichterStopMoving
    lda #$02
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    rts

; @routine 01BB:5AE RichterHurtResume
; @run 6D:B5AE
; WHAT IT DOES: The reaction's step 3: shows the hurt frame for the state he was hit in, and when
; OBJ_HIT_FRAMES runs out gives back that state and its step; with step 1 given back it ends as
; RichterEndItemCrash.
; HOW: The frame from the two bytes at $B5D5 by (the state less 5) / 2; a tail jump to
; RichterEndItemCrash.
; WAITS: no.
; Entry: nothing. Exit: X the frame's index; V from the subtract. Still counting: A the frame, N and Z
; from the count, C bit 0 of the state less 5. Given back: A the step, N, Z and C from comparing it
; with 1; step 1, then as RichterEndItemCrash. Y kept but there.
RichterHurtResume:
    lda a:OBJ_STATE_BEFORE_HIT
    sec
    sbc #$05
    lsr a
    tax
    lda a:RichterHurtResumeFrames,x
    sta a:OBJ_FRAME
    dec a:OBJ_HIT_FRAMES
    bne RichterHurtResume_at0
    lda a:OBJ_STATE_BEFORE_HIT
    sta a:OBJ_STATE
    lda a:OBJ_STEP_BEFORE_HIT
    sta a:OBJ_SUBSTATE
    cmp #$01
    bne RichterHurtResume_at0
    jmp RichterEndItemCrash
RichterHurtResume_at0:
    rts

; @routine 01BB:5D7 RichterHurtHoldFrame
; @run 6D:B5D7
; WHAT IT DOES: Runs the animation's time but keeps the frame he shows; stands him when it ends.
; HOW: OBJ_FRAME pushed around RichterAnimationTick; OBJ_HURT_TIMER against $FF; a tail jump to
; RichterStand.
; WAITS: no.
; Entry: nothing. Exit: running, A the timer, N, Z and C from comparing it with $FF; ended, as
; RichterStand.
RichterHurtHoldFrame:
    lda a:OBJ_FRAME
    pha
    jsr RichterAnimationTick
    pla
    sta a:OBJ_FRAME
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    beq RichterHurtHoldFrame_at0
    rts
RichterHurtHoldFrame_at0:
    jmp RichterStand

; @routine 01BB:5ED RichterHurtDownStep
; @run 6D:B5ED
; WHAT IT DOES: The step he is down in (6): falls if no floor is under him or a pixel below; else
; stands on it; with PlayerHeld set holds the frame as RichterHurtHoldFrame, else runs the
; animation and stands him when it ends.
; HOW: RichterFloorCheck twice, RichterToFall, RichterLandOnFloor; RichterAnimationRunning and a tail
; jump to RichterStand.
; WAITS: no.
; Entry: nothing. Exit: as the routine it goes to; running, as RichterAnimationRunning.
RichterHurtDownStep:
    jsr RichterFloorCheck
    bcs RichterHurtDownStep_at0
    inc a:OBJ_Y_LO
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcs RichterHurtDownStep_at0
    jmp RichterToFall
RichterHurtDownStep_at0:
    jsr RichterLandOnFloor
    lda a:PlayerHeld
    bne RichterHurtHoldFrame
    jsr RichterAnimationRunning
    beq RichterHurtDownStep_at1
    rts
RichterHurtDownStep_at1:
    jmp RichterStand
