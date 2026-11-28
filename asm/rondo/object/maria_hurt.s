; @routine 01DF:288 MariaStateHurt
; @run 6D:B288
; WHAT IT DOES: State 9, the hit's reaction: the step's handler from MariaHurtSteps.
; HOW: MariaStepIndex, then the JMP (table,X).
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
MariaStateHurt:
    jsr MariaStepIndex
    jmp (MariaHurtSteps,x)

; @routine 01DF:2A6 MariaHurtSide
; @run 6D:B2A6
; WHAT IT DOES: Says which way the hit knocks her (OBJ_KNOCK_RIGHT): a spike (kind 6, made kind 2) the way she faces;
; kind 4 to the right; else away from the object that hit her (OBJ_HIT_BY_SLOT) by its X; and OBJ_KNOCK_FORWARD 1 when
; that is the way she faces.
; HOW: The kind in bits 0 to 2 of OBJ_HIT_TAKEN; the hitter's X high byte's sign, or its X low against hers.
; WAITS: no.
; Entry: nothing. Exit: A OBJ_KNOCK_FORWARD, N and Z from it; X the hitter's slot when it was read; C from the last
; compare made. Y and V kept.
MariaHurtSide:
    lda a:OBJ_FACING_LEFT
    sta a:OBJ_WORK_L
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$06
    bne MariaHurtSide_at0
    lda #$02
    sta a:OBJ_HIT_TAKEN
    bra MariaHurtSide_at1
MariaHurtSide_at0:
    stz a:OBJ_WORK_L
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$04
    bne MariaHurtSide_at2
    inc a:OBJ_WORK_L
    bra MariaHurtSide_at1
MariaHurtSide_at2:
    ldx a:OBJ_WORK_O
    lda a:OBJ_X_HI,x
    beq MariaHurtSide_to1
    bpl MariaHurtSide_at1
    inc a:OBJ_WORK_L
    bra MariaHurtSide_at1
MariaHurtSide_to1:
    lda a:OBJ_X_LO,x
    cmp a:OBJ_X_LO
    bcs MariaHurtSide_at1
    inc a:OBJ_WORK_L
MariaHurtSide_at1:
    lda a:OBJ_WORK_L
    eor a:OBJ_FACING_LEFT
    and #$01
    sta a:OBJ_KNOCK_FORWARD
    rts

; @routine 01DF:2F3 MariaHurtStart
; @run 6D:B2F3
; WHAT IT DOES: The hit's step 0: stands her on a floor at her Y or a pixel below if there is one; clears the pad's
; forced presses, the attack's outcome, the hold and the slowing; says which way she is knocked; kinds 3 and 4 throw
; her; else she stops and the hit's start by the state she was in runs (MariaHurtByState).
; HOW: MariaReadColumns and MariaFloorCheck (INC of her Y between), MariaLandOnFloor; MariaPadForcesOff; MariaHurtSide;
; the JMPs to MariaHurtKindThree and MariaHurtKindFour; MariaStopMoving; OBJ_STATE_BEFORE_HIT doubled, the JMP
; (table,X).
; WAITS: no.
; Entry: Y the caller's. Exit: as the handler it ended in.
MariaHurtStart:
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcs MariaHurtStart_at0
    inc a:OBJ_Y_LO
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaHurtStart_at1
MariaHurtStart_at0:
    jsr MariaLandOnFloor
MariaHurtStart_at1:
    jsr MariaPadForcesOff
    stz a:PlayerAttackKind
    stz a:PlayerHeld
    stz a:PlayerHeldCount
    stz a:PlayerSlowTimerLo
    stz a:PlayerSlowTimerHi
    jsr MariaHurtSide
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$03
    bne MariaHurtStart_at2
    jmp MariaHurtKindThree
MariaHurtStart_at2:
    cmp #$04
    bne MariaHurtStart_at3
    jmp MariaHurtKindFour
MariaHurtStart_at3:
    jsr MariaStopMoving
    lda a:OBJ_WORK_G
    asl a
    tax
    jmp (MariaHurtByState,x)

; @routine 01DF:360 MariaHurtFromItem4
; @run 6D:B360
; WHAT IT DOES: A hit in item 4's state ($10): item 4 ended (MariaClearSlot5AsKind9), then knocked up.
; HOW: MariaClearSlot5AsKind9, then a branch to MariaHurtKnockUp.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHurtKnockUp.
MariaHurtFromItem4:
    jsr MariaClearSlot5AsKind9
    bra MariaHurtKnockUp

; @routine 01DF:365 MariaHurtFromSequence
; @run 6D:B365
; WHAT IT DOES: A hit in her pad sequence's move (state $11): its object in slot 15 emptied, then knocked up.
; HOW: ClearObject, then a branch to MariaHurtKnockUp.
; WAITS: no.
; Entry: Y the caller's. Exit: as MariaHurtKnockUp.
MariaHurtFromSequence:
    ldx #$0F
    jsr VecClearObject
    bra MariaHurtKnockUp

; @routine 01DF:36C MariaHurtFromStand
; @run 6D:B36C
; WHAT IT DOES: A hit while standing or walking: knocked up at step 1 or unless the hit's low bits are 1; then down.
; HOW: OBJ_STEP_BEFORE_HIT and bits 0 and 1 of OBJ_HIT_TAKEN; on into MariaHurtKnockUp or the JMP to MariaHurtDown.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHurtKnockUp or MariaHurtDown.
MariaHurtFromStand:
    lda a:OBJ_FALL_LO
    cmp #$01
    beq MariaHurtKnockUp
    lda a:OBJ_HIT_TAKEN
    and #$03
    cmp #$01
    beq MariaHurtDown

; @routine 01DF:37C MariaHurtKnockUp
; @run 6D:B37C
; WHAT IT DOES: Knocks her up: step 2, animation $22 (plus OBJ_KNOCK_FORWARD), Y speed $FC00, then the hit's sound and
; effects.
; HOW: MariaSetAnimation, the stores, the JMP to MariaHitSoundAndEffects.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHitSoundAndEffects.
MariaHurtKnockUp:
    lda #$02
    sta a:OBJ_SUBSTATE
    lda #$22
    clc
    adc a:OBJ_KNOCK_FORWARD
    jsr MariaSetAnimation
    lda #$00
    sta a:OBJ_VY_LO
    lda #$FC
    sta a:OBJ_VY_HI
    jmp MariaHitSoundAndEffects

; @routine 01DF:397 MariaHurtDown
; @run 6D:B397
; WHAT IT DOES: Puts her down: step 6 with animation $1A, then the hit's sound and effects.
; HOW: MariaSetAnimation, the JMP to MariaHitSoundAndEffects.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHitSoundAndEffects.
MariaHurtDown:
    lda #$06
    sta a:OBJ_SUBSTATE
    lda #$1A
    jsr MariaSetAnimation
    jmp MariaHitSoundAndEffects

; @routine 01DF:3A4 MariaHurtFromCrouch
; @run 6D:B3A4
; WHAT IT DOES: A hit while crouched: knocked up unless at step 0; otherwise a slide, step $0B when the hit's low bits
; are 1 else step 1, frame $16 for 8 frames; then the hit's sound and effects.
; HOW: OBJ_STEP_BEFORE_HIT and bits 0 and 1 of OBJ_HIT_TAKEN; the JMPs to MariaHurtKnockUp and MariaHitSoundAndEffects.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHurtKnockUp or MariaHitSoundAndEffects.
MariaHurtFromCrouch:
    lda a:OBJ_FALL_LO
    bne MariaHurtKnockUp
    lda a:OBJ_HIT_TAKEN
    and #$03
    cmp #$01
    bne MariaHurtFromCrouch_to1
    lda #$0B
    bra MariaHurtFromCrouch_at0
MariaHurtFromCrouch_to1:
    lda #$01
MariaHurtFromCrouch_at0:
    sta a:OBJ_SUBSTATE
    lda #$16
    sta a:OBJ_FRAME
    lda #$08
    sta a:OBJ_FALL_HI
    jmp MariaHitSoundAndEffects

; @routine 01DF:3C8 MariaHurtInAir
; @run 6D:B3C8
; WHAT IT DOES: A hit in the air, on the ledge ahead or in the hit's own state: knocked up, at Y speed $FE00.
; HOW: MariaHurtKnockUp, then the speed.
; WAITS: no.
; Entry: X and Y the caller's. Exit: A $FE, N and Z from it; the rest as MariaHurtKnockUp left it.
MariaHurtInAir:
    jsr MariaHurtKnockUp
    lda #$00
    sta a:OBJ_VY_LO
    lda #$FE
    sta a:OBJ_VY_HI
    rts

; @routine 01DF:3D6 MariaHurtClimbKindFive
; @run 6D:B3D6
; WHAT IT DOES: A hit of kind 5 on the ledge behind her: knocked up, at Y speed $FB00.
; HOW: MariaHurtKnockUp, then the speed.
; WAITS: no.
; Entry: X and Y the caller's. Exit: A $FB, N and Z from it; the rest as MariaHurtKnockUp left it.
MariaHurtClimbKindFive:
    jsr MariaHurtKnockUp
    lda #$00
    sta a:OBJ_VY_LO
    lda #$FB
    sta a:OBJ_VY_HI
    rts

; @routine 01DF:3E4 MariaHurtWhileClimbing
; @run 6D:B3E4
; WHAT IT DOES: A hit on the ledge behind her (states 7 and 8): kind 5 as MariaHurtClimbKindFive; else frame $2C for
; $20 frames, step 3, then the hit's sound and effects.
; HOW: The kind; the stores; a branch into MariaHitSoundAndEffects.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHurtClimbKindFive or MariaHitSoundAndEffects.
MariaHurtWhileClimbing:
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$05
    beq MariaHurtClimbKindFive
    lda #$20
    sta a:OBJ_FALL_HI
    lda #$2C
    sta a:OBJ_FRAME
    lda #$03
    sta a:OBJ_SUBSTATE
    bra MariaHitSoundAndEffects

; @routine 01DF:3FE MariaHitSoundAndEffects
; @run 6D:B3FE
; WHAT IT DOES: Sound $66 for a hit with neither bit 5 nor bit 6 of OBJ_HIT_TAKEN, then the hit's effects.
; HOW: TST, SoundCommand, and on into MariaHitEffects.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHitEffects.
MariaHitSoundAndEffects:
    tst #$60,a:OBJ_HIT_TAKEN
    bne MariaHitEffects
    lda #$66
    jsr VecSoundCommand

; @routine 01DF:409 MariaHitEffects
; @run 6D:B409
; WHAT IT DOES: What a hit does besides hurting, by bits 4 to 6 of OBJ_HIT_TAKEN, the first set of them: bit 4 empties
; slots 1 and 2 and makes them kind 1 (slot 1's timer $0A), script $0A; bit 5 the hold (sound $12, PlayerHeld counted
; up), script 1; bit 6 the slowing (sound $24, PlayerSlowTimer $01FF), script $1C. The hit is cleared.
; HOW: TST of OBJ_HIT_TAKEN; ClearObject, SoundCommand; the tail JMP to MariaStartHitScript.
; WAITS: no.
; Entry: X and Y the caller's. Exit: none of them, A kept, Z set from the last TST, N and V its bits 7 and 6; else as
; MariaStartHitScript.
MariaHitEffects:
    tst #$10,a:OBJ_HIT_TAKEN
    bne MariaHitEffects_at0
    tst #$20,a:OBJ_HIT_TAKEN
    bne MariaHitEffects_at1
    tst #$40,a:OBJ_HIT_TAKEN
    bne MariaHitEffects_at2
    stz a:OBJ_HIT_TAKEN
    rts
MariaHitEffects_at0:
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
MariaHitEffects_at3:
    stz a:OBJ_HIT_TAKEN
    jmp MariaStartHitScript
MariaHitEffects_at1:
    lda #$12
    jsr VecSoundCommand
    inc a:PlayerHeld
    lda #$01
    bra MariaHitEffects_at3
MariaHitEffects_at2:
    lda #$24
    jsr VecSoundCommand
    lda #$FF
    sta a:PlayerSlowTimerLo
    lda #$01
    sta a:PlayerSlowTimerHi
    lda #$1C
    bra MariaHitEffects_at3

; @routine 01DF:45C MariaHurtKindThree
; @run 6D:B45C
; WHAT IT DOES: A hit of kind 3 throws her: stopped; in the air up at $FE50, else (item 4's state ended first)
; lifted three pixels and up at $FC50; across by MariaThrowSpeedsX; animation $1B (plus OBJ_KNOCK_FORWARD), step 7,
; she may not leave the top; sound $67, the hit's effects.
; HOW: MariaStopMoving, MariaHurtThrowKinds by the state she was hit in, MariaClearSlot5AsKind9, MariaLiftThree;
; MariaSetAnimation, SoundCommand, the JMP to MariaHitEffects.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHitEffects.
MariaHurtKindThree:
    jsr MariaStopMoving
    ldx a:OBJ_WORK_G
    lda a:MariaHurtThrowKinds,x
    beq MariaHurtKindThree_at0
    cmp #$02
    bne MariaHurtKindThree_at1
    jsr MariaClearSlot5AsKind9
    bra MariaHurtKindThree_at0
MariaHurtKindThree_at1:
    lda #$50
    sta a:OBJ_VY_LO
    lda #$FE
    sta a:OBJ_VY_HI
    bra MariaHurtKindThree_at2
MariaHurtKindThree_at0:
    jsr MariaLiftThree
    lda #$50
    sta a:OBJ_VY_LO
    lda #$FC
    sta a:OBJ_VY_HI
MariaHurtKindThree_at2:
    lda a:OBJ_WORK_L
    asl a
    tay
    lda a:MariaThrowSpeedsX,y
    sta a:OBJ_VX_LO
    lda a:MariaThrowSpeedsX+1,y
    sta a:OBJ_VX_HI
    lda #$1B
    clc
    adc a:OBJ_KNOCK_FORWARD
    jsr MariaSetAnimation
    lda #$07
    sta a:OBJ_SUBSTATE
    stz a:PlayerMayLeaveTop
    lda #$67
    jsr VecSoundCommand
    jmp MariaHitEffects

; @routine 01DF:4C8 MariaHurtKindFour
; @run 6D:B4C8
; WHAT IT DOES: A hit of kind 4 throws her to the right: stopped (item 4's state ended first), up at $F900, lifted
; three pixels, right at $0480; animation $1B (plus OBJ_KNOCK_FORWARD), step 9, she may not leave the top; sound $67,
; the hit's effects.
; HOW: As MariaHurtKindThree.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHitEffects.
MariaHurtKindFour:
    lda #$01
    sta a:OBJ_WORK_L
    jsr MariaStopMoving
    ldx a:OBJ_WORK_G
    lda a:MariaHurtThrowKinds,x
    cmp #$02
    bne MariaHurtKindFour_at0
    jsr MariaClearSlot5AsKind9
MariaHurtKindFour_at0:
    lda #$00
    sta a:OBJ_VY_LO
    lda #$F9
    sta a:OBJ_VY_HI
    jsr MariaLiftThree
    lda #$80
    sta a:OBJ_VX_LO
    lda #$04
    sta a:OBJ_VX_HI
    lda #$1B
    clc
    adc a:OBJ_KNOCK_FORWARD
    jsr MariaSetAnimation
    lda #$09
    sta a:OBJ_SUBSTATE
    stz a:PlayerMayLeaveTop
    lda #$67
    jsr VecSoundCommand
    jmp MariaHitEffects

; @routine 01DF:50D MariaHurtThrown
; @run 6D:B50D
; WHAT IT DOES: The throw's steps 7 and 9: on the map's first screen and falling, lands on a floor her column stands on
; (her Y up by its row, no Y speed, slots 1 and 2 as kind 5; animation $1D with sound 2 when her animation has
; stopped, else $24; the next step and the landing's X speed); else the bit-3 burst; light gravity and her animation.
; HOW: MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn; MariaReplaceSlotsOneTwoKind5, SoundCommand,
; MariaSetAnimation, MariaThrownLandSpeedsX; MariaCheckBit3Under, MariaGravityLight and the tail JMP to
; MariaAnimationTick.
; WAITS: no.
; Entry: X and Y the caller's. Exit: landed, A the speed's high byte, N and Z from it, Y the side doubled; else as
; MariaAnimationTick.
MariaHurtThrown:
    jsr MariaReadColumns
    lda a:OBJ_Y_HI
    bne MariaHurtThrown_at0
    lda a:OBJ_VY_HI
    bmi MariaHurtThrown_at0
    jsr MariaFloorCheck
    bcc MariaHurtThrown_at1
    jsr MariaSolidUnderColumn
    bne MariaHurtThrown_at1
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    stz a:OBJ_VY_LO
    stz a:OBJ_VY_HI
    jsr MariaReplaceSlotsOneTwoKind5
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    bne MariaHurtThrown_at2
    lda #$02
    jsr VecSoundCommand
    lda #$1D
    bra MariaHurtThrown_at3
MariaHurtThrown_at1:
    jsr MariaCheckBit3Under
MariaHurtThrown_at0:
    jsr MariaGravityLight
    jmp MariaAnimationTick
MariaHurtThrown_at2:
    lda #$24
MariaHurtThrown_at3:
    jsr MariaSetAnimation
    inc a:OBJ_SUBSTATE
    lda a:OBJ_WORK_L
    asl a
    tay
    lda a:MariaThrownLandSpeedsX,y
    sta a:OBJ_VX_LO
    lda a:MariaThrownLandSpeedsX+1,y
    sta a:OBJ_VX_HI
    rts

; @routine 01DF:56E MariaHurtThrownLanded
; @run 6D:B56E
; WHAT IT DOES: The throw's steps 8 and $0A: slots 1 and 2 as kind 5 every eighth frame; with a floor under her or a
; pixel below, while her animation runs nothing more, and when it has stopped the hit ends in a crouch; with none, back
; to the step before.
; HOW: TST of zWaveCounter, MariaReplaceSlotsOneTwoKind5; MariaReadColumns and MariaFloorCheck (INC of her Y between);
; MariaAnimationWait, which ends the step while it runs, then the JMP to the crouch's end ($B63B); DEC of the step.
; WAITS: no.
; Entry: nothing. Exit: as the routine it ended in; back, N and Z from the step.
MariaHurtThrownLanded:
    tst #$07,z:zWaveCounter
    bne MariaHurtThrownLanded_to1
    jsr MariaReplaceSlotsOneTwoKind5
MariaHurtThrownLanded_to1:
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcs MariaHurtThrownLanded_at0
    inc a:OBJ_Y_LO
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaHurtThrownLanded_at1
MariaHurtThrownLanded_at0:
    jsr MariaAnimationWait
    jmp MariaHurtSlide_crouch
MariaHurtThrownLanded_at1:
    dec a:OBJ_SUBSTATE
    rts

; @routine 01DF:593 MariaHurtKnocked
; @run 6D:B593
; WHAT IT DOES: The knock's steps 2, 4 and 5: her animation, the knock speed; rising, light gravity; falling, lands on
; a floor her column stands on (held: Y speed $0200, the landing, slots 1 and 2 as kind 5 and sound $4A; else full fall
; speed and MariaJumpLand), or on a marked floor; else the bit-3 burst and light gravity.
; HOW: MariaAnimationTick, a BSR to MariaKnockSpeedX; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn,
; PlayerHeld; MariaJumpLand, MariaReplaceSlotsOneTwoKind5, SoundCommand; MariaOnMarkedFloor, MariaFeetOnMarkedFloor and
; their hurt landings; MariaCheckBit3Under; the tail JMP to MariaGravityLight.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in.
MariaHurtKnocked:
    jsr MariaAnimationTick
    bsr MariaKnockSpeedX
    lda a:OBJ_VY_HI
    bmi MariaHurtKnocked_at0
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaHurtKnocked_at1
    jsr MariaSolidUnderColumn
    bne MariaHurtKnocked_at1
    lda a:PlayerHeld
    bne MariaHurtKnocked_to1
    lda #$07
    sta a:OBJ_VY_HI
    jmp MariaJumpLand
MariaHurtKnocked_at1:
    jsr MariaOnMarkedFloor
    bcc MariaHurtKnocked_at2
    jmp MariaHurtLandOnMarked
MariaHurtKnocked_at2:
    jsr MariaFeetOnMarkedFloor
    bcc MariaHurtKnocked_at3
    jmp MariaHurtLandOnMarkedFeet
MariaHurtKnocked_at3:
    jsr MariaCheckBit3Under
MariaHurtKnocked_at0:
    jmp MariaGravityLight
MariaHurtKnocked_to1:
    lda #$02
    sta a:OBJ_VY_HI
    jsr MariaJumpLand
MariaHurtKnocked_to2:
    jsr MariaReplaceSlotsOneTwoKind5
    lda #$4A
    jmp VecSoundCommand

; @routine 01DF:5DD MariaHurtLandOnMarked
; @run 6D:B5DD
; WHAT IT DOES: The knock lands on a marked floor: held, MariaHurtHeldLanding; else full fall speed and as
; MariaLandOnMarked.
; HOW: PlayerHeld; the JMP to MariaLandOnMarked.
; WAITS: held, never ends.
; Entry: X and Y the caller's. Exit: as MariaLandOnMarked.
MariaHurtLandOnMarked:
    lda a:PlayerHeld
    bne MariaHurtHeldLanding
    lda #$07
    sta a:OBJ_VY_HI
    jmp MariaLandOnMarked

; @routine 01DF:5EA MariaHurtLandOnMarkedFeet
; @run 6D:B5EA
; WHAT IT DOES: As MariaHurtLandOnMarked for marked ground under her feet: held, MariaHurtHeldLandingFeet; else full
; fall speed and as MariaLandOnMarkedFeet.
; HOW: PlayerHeld; the JMP to MariaLandOnMarkedFeet.
; WAITS: held, never ends.
; Entry: X and Y the caller's. Exit: as MariaLandOnMarkedFeet.
MariaHurtLandOnMarkedFeet:
    lda a:PlayerHeld
    bne MariaHurtHeldLandingFeet
    lda #$07
    sta a:OBJ_VY_HI
    jmp MariaLandOnMarkedFeet

; @routine 01DF:5F7 MariaHurtHeldLanding
; @run 6D:B5F7
; WHAT IT DOES: The held landing on a marked floor. Where Richter's matching routine (in RichterHurtLandOnMarkedFeet)
; calls his landing's sound, hers calls its own address: her Y speed's high byte 2, then a JSR to itself, without end.
; HOW: LDA #2, STA OBJ_VY_HI, JSR $B5F7: each round makes the same store and pushes another return address ($B5FF);
; the stack wraps in its page. The BRA after it is never reached.
; WAITS: never ends.
; Entry: anything. Exit: none.
MariaHurtHeldLanding:
    lda #$02
    sta a:OBJ_VY_HI
    jsr MariaHurtHeldLanding
    bra MariaHurtKnocked_to2

; @routine 01DF:601 MariaHurtHeldLandingFeet
; @run 6D:B601
; WHAT IT DOES: The same for a marked floor under her feet: her Y speed's high byte 2, then the JSR to
; MariaHurtHeldLanding, which never returns.
; HOW: LDA #2, STA OBJ_VY_HI, JSR $B5F7.
; WAITS: never ends.
; Entry: anything. Exit: none.
MariaHurtHeldLandingFeet:
    lda #$02
    sta a:OBJ_VY_HI
    jsr MariaHurtHeldLanding
    bra MariaHurtKnocked_to2

; @routine 01DF:60B MariaKnockSpeedX
; @run 6D:B60B
; WHAT IT DOES: Gives her the knock's X speed for the side the hit throws her to ($FF00 left, $0100 right).
; HOW: MariaKnockSpeeds by OBJ_KNOCK_RIGHT doubled.
; WAITS: no.
; Entry: nothing. Exit: A the high byte, N and Z from it; Y the side doubled; C bit 7 of the side. X and V kept.
MariaKnockSpeedX:
    lda a:OBJ_WORK_L
    asl a
    tay
    lda a:MariaKnockSpeeds,y
    sta a:OBJ_VX_LO
    lda a:MariaKnockSpeeds+1,y
    sta a:OBJ_VX_HI
    rts

; @routine 01DF:61D MariaHurtSlideStart
; @run 6D:B61D
; WHAT IT DOES: The slide's first step: the knock speed, then as MariaHurtSlide.
; HOW: A BSR to MariaKnockSpeedX, on into MariaHurtSlide.
; WAITS: no.
; Entry: nothing. Exit: as MariaHurtSlide.
MariaHurtSlideStart:
    bsr MariaKnockSpeedX

; @routine 01DF:61F MariaHurtSlide
; @run 6D:B61F
; WHAT IT DOES: The slide's step (1 and $0B): falls if no floor is under her or a pixel below; else stands on it, and
; when its frames run out the hit ends in a crouch.
; HOW: MariaFloorCheck (MariaReadColumns before the second), MariaToFall by its JMP, MariaLandOnFloor; DEC of
; OBJ_HIT_FRAMES.
; WAITS: no.
; Entry: nothing. Exit: as MariaToFall; frames left, N and Z from them; ended, A 0, Z set.
MariaHurtSlide:
    jsr MariaFloorCheck
    bcs MariaHurtSlide_to1
    inc a:OBJ_Y_LO
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcs MariaHurtSlide_to1
    jmp MariaToFall
MariaHurtSlide_to1:
    jsr MariaLandOnFloor
    dec a:OBJ_FALL_HI
    beq MariaHurtSlide_at0
    rts
MariaHurtSlide_at0:
MariaHurtSlide_crouch:
    lda #$1B
    sta a:OBJ_FRAME
    lda #$FF
    sta a:OBJ_HURT_TIMER
    jsr MariaStopMoving
    lda #$02
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    rts

; @routine 01DF:657 MariaHurtResume
; @run 6D:B657
; WHAT IT DOES: The hit's step 3: when its frames run out she is back in the state and step she was hit in, and at
; step 1 or 5 back in her ledge's hold.
; HOW: DEC of OBJ_HIT_FRAMES; OBJ_STATE_BEFORE_HIT and OBJ_STEP_BEFORE_HIT; the JMP to MariaClimbHold.
; WAITS: no.
; Entry: nothing. Exit: waiting, N and Z from the frames left; else A the step, N, Z and C from comparing it with 1
; or 5, or as MariaClimbHold.
MariaHurtResume:
    dec a:OBJ_FALL_HI
    bne MariaHurtResume_at0
    lda a:OBJ_WORK_G
    sta a:OBJ_STATE
    lda a:OBJ_FALL_LO
    sta a:OBJ_SUBSTATE
    cmp #$01
    beq MariaHurtResume_at1
    cmp #$05
    beq MariaHurtResume_at1
MariaHurtResume_at0:
    rts
MariaHurtResume_at1:
    jmp MariaClimbHold

; @routine 01DF:674 MariaHurtHoldFrame
; @run 6D:B674
; WHAT IT DOES: Runs the animation's time but keeps the frame she shows; stands her when it ends.
; HOW: Her frame on the stack around MariaAnimationTick; OBJ_HURT_TIMER $FF ends it; the tail JMP to MariaStand.
; WAITS: no.
; Entry: nothing. Exit: running, A the timer, N, Z and C from comparing it with $FF; else as MariaStand.
MariaHurtHoldFrame:
    lda a:OBJ_FRAME
    pha
    jsr MariaAnimationTick
    pla
    sta a:OBJ_FRAME
    lda a:OBJ_HURT_TIMER
    cmp #$FF
    beq MariaHurtHoldFrame_to1
    rts
MariaHurtHoldFrame_to1:
    jmp MariaStand

; @routine 01DF:68A MariaHurtDownStep
; @run 6D:B68A
; WHAT IT DOES: The step she is down in (6): falls if no floor is under her or a pixel below; else stands on it; held,
; as MariaHurtHoldFrame; else when her animation has stopped she stands.
; HOW: MariaFloorCheck (MariaReadColumns before the second), MariaToFall by its JMP, MariaLandOnFloor, PlayerHeld, a
; branch to MariaHurtHoldFrame, MariaAnimationRunning, the JMP to MariaStand.
; WAITS: no.
; Entry: nothing. Exit: as the routine it ended in.
MariaHurtDownStep:
    jsr MariaFloorCheck
    bcs MariaHurtDownStep_at0
    inc a:OBJ_Y_LO
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcs MariaHurtDownStep_at0
    jmp MariaToFall
MariaHurtDownStep_at0:
    jsr MariaLandOnFloor
    lda a:PlayerHeld
    bne MariaHurtHoldFrame
    jsr MariaAnimationRunning
    beq MariaHurtDownStep_to1
    rts
MariaHurtDownStep_to1:
    jmp MariaStand
