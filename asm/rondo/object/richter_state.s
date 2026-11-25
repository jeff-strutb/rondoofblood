; @routine 01B9:000 RichterVecFrame
; @run 6D:A000
; WHAT IT DOES: Goes to RichterFrame.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as RichterFrame.
RichterVecFrame:
    jmp RichterFrame

; @routine 01B9:003 RichterVecStart
; @run 6D:A003
; WHAT IT DOES: Goes to RichterStart.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as RichterStart.
RichterVecStart:
    jmp RichterStart

; @routine 01B9:006 RichterVecApplyMove
; @run 6D:A006
; WHAT IT DOES: Goes to $BB23.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as that code.
RichterVecApplyMove:
    jmp RichterReadColumns

; @routine 01B9:009 RichterVecC3A7
; @run 6D:A009
; WHAT IT DOES: Goes to $C3A7.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as that code.
RichterVecC3A7:
    jmp $C3A7

; @routine 01B9:00C RichterVecC023
; @run 6D:A00C
; WHAT IT DOES: Goes to RichterLeapClearingOthers.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as RichterLeapClearingOthers.
RichterVecC023:
    jmp RichterLeapClearingOthers

; @routine 01B9:00F RichterVecC076
; @run 6D:A00F
; WHAT IT DOES: Goes to $C076.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as that code.
RichterVecC076:
    jmp $C076

; @routine 01B9:012 RichterVecAnimationSound
; @run 6D:A012
; WHAT IT DOES: Goes to RichterAnimationSound.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as RichterAnimationSound.
RichterVecAnimationSound:
    jmp RichterAnimationSound

; @routine 01B9:015 RichterVecCE1D
; @run 6D:A015
; WHAT IT DOES: Goes to RichterClearOthersKeepWhip.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as RichterClearOthersKeepWhip.
RichterVecCE1D:
    jmp RichterClearOthersKeepWhip

; @routine 01B9:018 RichterVecSetAnimation
; @run 6D:A018
; WHAT IT DOES: Goes to $B67B.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as that code.
RichterVecSetAnimation:
    jmp RichterSetAnimation

; @routine 01B9:01B RichterVecSetIdle
; @run 6D:A01B
; WHAT IT DOES: Goes to $B669.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as that code.
RichterVecSetIdle:
    jmp RichterAnimationTick

; @routine 01B9:01E RichterVecCE73
; @run 6D:A01E
; WHAT IT DOES: Goes to $CE73.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as that code.
RichterVecCE73:
    jmp $CE73

; @routine 01B9:021 RichterVecWall
; @run 6D:A021
; WHAT IT DOES: Goes to $BB6E, the wall probe.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as that code.
RichterVecWall:
    jmp RichterWall

; @routine 01B9:024 RichterVecFloor
; @run 6D:A024
; WHAT IT DOES: Goes to $BBC1, the floor probe.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as that code.
RichterVecFloor:
    jmp RichterFloor

; @routine 01B9:027 RichterApplyMoveAndPushGoTo
; @run 6D:A027
; WHAT IT DOES: Goes to RichterApplyMoveAndPush.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as RichterApplyMoveAndPush.
RichterApplyMoveAndPushGoTo:
    jmp RichterApplyMoveAndPush

; @routine 01B9:02A RichterApplyMoveAndPush
; @run 6D:A02A
; WHAT IT DOES: Reads the five columns under him after his move and, if none of the first
; three is solid or a floor, lets a push on the floor lift him by zFieldCA.
; HOW: Bits $81 of RichterColumnHit; zFieldCA goes in as column 0's depth for
; RichterLiftByColumn, then is cleared.
; WAITS: no.
; Entry: nothing. Exit: Y as RichterReadColumns left it. A column hit, A its bits, N and Z from
; them, X the column, C and V from the read. Not pushed, A 0, X 3, Z and C set, N and V the
; push's bits 7 and 6. Lifted, as RichterLiftByColumn, X 0.
RichterApplyMoveAndPush:
    jsr RichterReadColumns
    clx
RichterApplyMoveAndPush_at1:
    lda a:RichterColumnHit,x
    and #$81
    bne RichterApplyMoveAndPush_at0
    inx
    cpx #$03
    bne RichterApplyMoveAndPush_at1
    tst #$01,z:zPlayerPush
    beq RichterApplyMoveAndPush_at0
    lda z:zFieldCA
    sta a:RichterColumnRow
    clx
    jsr RichterLiftByColumn
    stz z:zFieldCA
RichterApplyMoveAndPush_at0:
    rts

; @routine 01B9:04B RichterFrame
; @run 6D:A04B
; WHAT IT DOES: One frame of Richter: the pad, forced while OBJ_3510 counts down; the
; pad's overrides; a spike while pushed; the state's frame; the turn window and the item's
; hearts; the animation's sound on a change; its hitbox.
; HOW: The animation before and after are compared; the hitbox index comes from the table
; by animation and is four bytes of AnimationHitbox.
; WAITS: no.
; Entry: nothing. Exit: A the last hitbox byte, N and Z from it, X the hitbox offset, C bit 6
; of the hitbox index; Y and V as RichterAnimationSound left them on a change, else as the
; state's frame did.
RichterFrame:
    lda a:OBJ_3510
    beq RichterFrame_at0
    lda z:zPadHeld
    sta z:zPadPrevious
    lda a:OBJ_3568
    sta z:zPadHeld
    sta z:zPadPressed
    dec a:OBJ_3510
    bne RichterFrame_at1
RichterFrame_at0:
    stz a:PlayerByte3BF0
    jsr RichterReadPad
RichterFrame_at1:
    lda a:PadForcedPress
    ora z:zPadPressed
    ora a:PadForcedPressB
    sta z:zPadPressed
    lda a:OBJ_2A68
    pha
    jsr RichterClearPadOverrides
    jsr RichterHitWhilePushed
    jsr RichterStateFrame
    jsr RichterCheckTurnWindow
    jsr RichterCheckItemHearts
    pla
    eor a:OBJ_2A68
    beq RichterFrame_at2
    jsr RichterAnimationSound
RichterFrame_at2:
    ldx a:OBJ_2A68
    lda a:AnimationHitboxIndex,x
    asl a
    asl a
    tax
    lda a:AnimationHitbox,x
    sta a:OBJ_3930
    lda a:AnimationHitbox+1,x
    sta a:OBJ_39E0
    lda a:AnimationHitbox+2,x
    sta a:OBJ_3988
    lda a:AnimationHitbox+3,x
    sta a:OBJ_3A38
    rts

; @routine 01B9:0AE RichterHitWhilePushed
; @run 6D:A0AE
; WHAT IT DOES: While pushed (bit 0 of zPlayerPush), outside camera mode $18, with no hit
; timer, not above the map, a spike under the player is a hit of kind 6.
; HOW: RichterSpikeBelow, then the hit's fields; the state back to standing.
; WAITS: no.
; Entry: nothing. Exit: camera mode $18, A $18, Z and C set. Not pushed, A the camera mode, Z
; set, N and V the push's bits 7 and 6, C from comparing the mode with $18. Held, A OBJ_3460
; or PlayerMayLeaveTop, N and Z from it, C as before, V the push's bit 6. No spike, as
; RichterSpikeBelow. A spike, as RichterSpikeBelow found it, but A $10 with N and Z from it.
RichterHitWhilePushed:
    lda a:CameraMode
    cmp #$18
    beq RichterHitWhilePushed_at0
    tst #$01,z:zPlayerPush
    beq RichterHitWhilePushed_at0
    lda a:OBJ_3460
    ora a:PlayerMayLeaveTop
    bne RichterHitWhilePushed_at0
    jsr RichterSpikeBelow
    beq RichterHitWhilePushed_at0
    lda #$06
    sta a:OBJ_33B0
    lda #$10
    sta a:OBJ_32A8
    stz a:OBJ_27A8
RichterHitWhilePushed_at0:
    rts

; @routine 01B9:0D5 RichterSpikeBelow
; @run 6D:A0D5
; WHAT IT DOES: Probes three points at the player's feet for a tile with bit 1 set.
; HOW: Each point's X from RichterColumnOffset pulled one pixel in, Y from the table by
; state; the terrain from RichterReadAbove.
; WAITS: no.
; Entry: nothing. Exit: Y, C and V as the last read left them (Y the state when the row was
; off the map). Found, A 2, N and Z from it, X the point. Else A 0, X 3, Z and C set, N clear.
RichterSpikeBelow:
    clx
RichterSpikeBelow_at3:
    jsr RichterColumnOffset
    beq RichterSpikeBelow_at0
    bmi RichterSpikeBelow_at1
    dec a
    bra RichterSpikeBelow_at0
RichterSpikeBelow_at1:
    inc a
RichterSpikeBelow_at0:
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    ldy a:OBJ_27A8
    clc
    adc a:PlayerFootYByState,y
    jsr RichterReadAbove
    and #$02
    beq RichterSpikeBelow_at2
    rts
RichterSpikeBelow_at2:
    inx
    cpx #$03
    bne RichterSpikeBelow_at3
    rts

; @routine 01B9:10F RichterStart
; @run 6D:A10F
; WHAT IT DOES: Starts Richter as zField8A says.
; HOW: The table of four entries: a new run, the health, the stand, the clear; each goes on
; into the next. zField8A is 0 to 3; the C takes anything past 2 as the clear, where the table
; would send it elsewhere.
; WAITS: no.
; Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
RichterStart:
    lda z:zField8A
    asl a
    tax
    jmp (PlayerStartHandlers,x)

; @routine 01B9:11E RichterClearWork
; @run 6D:A11E
; WHAT IT DOES: Clears the player's work bytes and zero page.
; HOW: Two block moves from the kernel's zeros.
; WAITS: no.
; Entry: nothing. Exit: all registers and flags kept.
RichterClearWork:
    tai KernelZeros,$3BC0,$0040
    tai KernelZeros,zPadHeld+$2000,$0019
    rts

; @routine 01B9:12D RichterClearKeepPlace
; @run 6D:A12D
; WHAT IT DOES: Clears object 0, the player, keeping where it is and which way it faces.
; HOW: The three bytes on the stack around ClearObject.
; WAITS: no.
; Entry: nothing. Exit: A the X position, N and Z from it, X 0. Y, C and V kept.
RichterClearKeepPlace:
    lda a:OBJ_X_LO
    pha
    lda a:OBJ_Y_LO
    pha
    lda a:OBJ_2AC0
    pha
    clx
    jsr VecClearObject
    pla
    sta a:OBJ_2AC0
    pla
    sta a:OBJ_Y_LO
    pla
    sta a:OBJ_X_LO
    rts

; @routine 01B9:14A RichterStartNew
; @run 6D:A14A
; WHAT IT DOES: Starts a new run: clears zField93 and sets zField94, then as
; RichterStartHealth.
; HOW: Runs on into it.
; WAITS: no.
; Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
RichterStartNew:
    stz z:zField93
    lda #$10
    sta z:zField94

; @routine 01B9:150 RichterStartHealth
; @run 6D:A150
; WHAT IT DOES: Fills the health, then as RichterStartClear.
; HOW: Runs on into it.
; WAITS: no.
; Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
RichterStartHealth:
    lda #$5C
    sta z:zPlayerHealth

; @routine 01B9:154 RichterStartClear
; @run 6D:A154
; WHAT IT DOES: Clears the player's work and object, marks it live, gives a frame of the
; hit timer, then as RichterStartStand.
; HOW: Runs on into it.
; WAITS: no.
; Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
RichterStartClear:
    jsr RichterClearWork
    jsr RichterClearKeepPlace
    lda #$01
    sta a:OBJ_3250
    sta a:OBJ_3828
    lda #$01
    sta a:OBJ_3460

; @routine 01B9:167 RichterStartStand
; @run 6D:A167
; WHAT IT DOES: Stands the player, and runs on into RichterAnimationSound.
; HOW: PlayerByte3BD6 set, RichterStand.
; WAITS: no.
; Entry: nothing. Exit: as RichterAnimationSound, and C as RichterStand left it.
RichterStartStand:
    lda #$01
    sta a:PlayerByte3BD6
    jsr RichterStand

; @routine 01B9:16F RichterAnimationSound
; @run 6D:A16F
; WHAT IT DOES: Starts the list that goes with the player's animation on its channel.
; HOW: The table by animation, then StartListChannelKind2 with X 7.
; WAITS: no.
; Entry: nothing. Exit: A 2, N 0, Z 0, X 7, Y the animation. C and V kept.
RichterAnimationSound:
    ldy a:OBJ_2A68
    lda a:AnimationSounds,y
    ldx #$07
    jmp VecStartListChannelKind2

; @routine 01B9:17A RichterDyingStep
; @run 6D:A17A
; WHAT IT DOES: The death's wait: holds the player still, and when the timer ends clears
; the objects from 1 up, stopping at one still live, and ends the mode.
; HOW: OBJ_2750 not zero is live. zSubStep 5 ends it.
; WAITS: no.
; Entry: nothing. Exit: still dying, N and Z from the timer, A, X, Y, C and V kept. An object
; still out, A its OBJ_2750 and X its slot, N and Z from it, C clear past slot 1. Over, as
; RichterStartHitScript with script 9, and C set.
RichterDyingStep:
    jsr RichterStopX
    stz a:OBJ_2A68
    dec a:PlayerDyingTimer
    bne SetGlyphVramPlace
    ldx #$01
RichterDyingStep_at0:
    lda a:OBJ_2750,x
    bne SetGlyphVramPlace
    jsr VecClearObject
    inx
    cpx #$10
    bne RichterDyingStep_at0
    lda #$05
    sta z:zSubStep
    stz a:PlayerByte3BD7
    stz a:PlayerDying
    lda #$09
    jsr RichterStartHitScript
    rts

; @routine 01B9:1A4 RichterDie
; @run 6D:A1A4
; WHAT IT DOES: Starts the player's death: no health, the timer, the state, the music out
; and the death's sound.
; HOW: FadeAndStopSound, then SoundCommand $60.
; WAITS: no.
; Entry: nothing. Exit: as SoundCommand with $60.
RichterDie:
    stz z:zPlayerHealth
    inc a:PlayerDying
    lda #$60
    sta a:PlayerDyingTimer
    jsr RichterStopX
    lda #$09
    sta a:OBJ_27A8
    jsr VecFadeAndStopSound
    lda #$60
    jmp VecSoundCommand

; @routine 01B9:1BE RichterStateFrame
; @run 6D:A1BE
; WHAT IT DOES: The state's frame: the death; falling off the map's bottom is death; the
; potion's invincibility; the hit timer; a hit in OBJ_33B0 taken off the health; the blink a
; hit can leave, which presses shake off; then the state's handler.
; HOW: Y high 1 or $FF low is below the map. A hit of kind 5 is ignored unless
; PlayerMayLeaveTop is set. Health gone goes to RichterOutOfHealth. Above the map the pad's
; buttons are dropped. The handler from PlayerStateHandlers by OBJ_27A8.
; WAITS: no.
; Entry: nothing. Exit: dying, as RichterDyingStep. Falling off, as RichterDie. The life
; lost, as SoundCommand with $64. Blinking with no press, A 0, Z set; with a press short of
; eight, A the count, N, Z and C from comparing it with 8. Otherwise as the state's handler.
; In each, the registers the earlier steps left where the last step keeps them.
RichterStateFrame:
    lda a:PlayerDying
    bne RichterDyingStep
    lda a:OBJ_Y_HI
    bmi RichterStateFrame_at0
    dec a
    beq RichterDie
    lda a:OBJ_Y_LO
    cmp #$FF
    bcs RichterDie
RichterStateFrame_at0:
    jsr RichterInvincibleTick
    lda a:OBJ_3460
    beq RichterStateFrame_at1
    stz a:OBJ_33B0
    dec a:OBJ_3460
    bne RichterStateFrame_at2
    lda a:PlayerByte3BE2
    ora a:PlayerByte3BE8
    ora a:PlayerByte3BE9
    bne RichterStateFrame_at1
    lda #$09
    jsr RichterStartHitScript
RichterStateFrame_at1:
    lda a:OBJ_33B0
    beq RichterStateFrame_at2
    lda a:OBJ_33B0
    and #$07
    cmp #$05
    bne RichterStateFrame_at3
    lda a:PlayerMayLeaveTop
    bne RichterStateFrame_at3
    stz a:OBJ_33B0
    jmp RichterStateFrame_at2
RichterStateFrame_at3:
    lda z:zPlayerHealth
    sec
    sbc a:OBJ_32A8
    sta z:zPlayerHealth
    bcs RichterStateFrame_at4
    jmp RichterOutOfHealth
RichterStateFrame_at4:
    bne RichterStateFrame_at5
    jmp RichterOutOfHealth
RichterStateFrame_at5:
    lda #$30
    sta a:OBJ_3460
    lda a:PlayerByte3BE2
    beq RichterStateFrame_at6
    lda #$01
    jsr RichterStartHitScript
    stz a:OBJ_33B0
    bra RichterStateFrame_at2
RichterStateFrame_at6:
    jsr RichterHitReact
RichterStateFrame_at2:
    lda a:PlayerByte3BE2
    beq RichterStateFrame_at7
    lda a:OBJ_27A8
    cmp #$09
    beq RichterStateFrame_at7
    lda a:PlayerByte3BE3
    bne RichterStateFrame_at8
    cla
    jsr RichterStartHitScript
    inc a:PlayerByte3BE3
RichterStateFrame_at8:
    lda z:zPadPressed
    beq RichterStateFrame_at9
    inc a:PlayerByte3BE3
    lda a:PlayerByte3BE3
    cmp #$08
    beq RichterStateFrame_at10
RichterStateFrame_at9:
    rts
RichterStateFrame_at10:
    stz a:PlayerByte3BE2
    lda #$09
    jsr RichterStartHitScript
RichterStateFrame_at7:
    lda a:OBJ_Y_HI
    cmp #$FF
    bne RichterStateFrame_at11
    lda z:zPadHeld
    and #$F0
    sta z:zPadHeld
    lda z:zPadPressed
    and #$F0
    sta z:zPadPressed
RichterStateFrame_at11:
    lda a:OBJ_27A8
    asl a
    tax
    jmp (PlayerStateHandlers,x)

; @routine 01B9:298 RichterOutOfHealth
; @run 6D:A298
; WHAT IT DOES: The health has run out: with the one-more flag PlayerByte3BF7 at 1 the
; player keeps one point and takes the hit; otherwise the mode's step 4 ends the life.
; HOW: SoundCommand $64 on the way out; the hit goes on as RichterStateFrame's.
; WAITS: no.
; Entry: nothing. Exit: as RichterStateFrame from the health on.
RichterOutOfHealth:
    stz z:zPlayerHealth
    lda a:PlayerByte3BF7
    cmp #$01
    bne RichterOutOfHealth_at0
    inc z:zPlayerHealth
    jmp $A21D
RichterOutOfHealth_at0:
    lda #$04
    sta z:zSubStep
    stz a:PlayerByte3BD7
    lda #$64
    jmp VecSoundCommand

; @routine 01B9:2B2 RichterHitReact
; @run 6D:A2B2
; WHAT IT DOES: Starts the reaction to a hit: the invulnerable frames by the hit's kind,
; the state kept to come back to, the state set to 9, and the hit's script.
; HOW: Runs on into RichterStartHitScript with 8.
; WAITS: no.
; Entry: nothing. Exit: A and Y 8, X 7, N and Z from 8. C and V kept.
RichterHitReact:
    jsr RichterPadOverridesOff
    lda a:OBJ_33B0
    and #$07
    tax
    lda a:HitInvulnerableFrames,x
    sta a:OBJ_3460
    lda a:OBJ_27A8
    sta a:OBJ_35C0
    lda a:OBJ_2800
    sta a:OBJ_3618
    lda #$09
    sta a:OBJ_27A8
    stz a:OBJ_2800
    lda #$08

; @routine 01B9:2D7 RichterStartHitScript
; @run 6D:A2D7
; WHAT IT DOES: Starts a script on the player's slot.
; HOW: StartScriptSlotKind1 with X 7.
; WAITS: no.
; Entry: A script. Exit: A and Y the script, X 7, N and Z from the script. C and V kept.
RichterStartHitScript:
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 01B9:2E4 RichterTurnOnPress
; @run 6D:A2E4
; WHAT IT DOES: Turns the player to face the way just pressed.
; HOW: OBJ_2AC0 is the facing, 0 right, 1 left.
; WAITS: no.
; Entry: nothing. Exit: N and V bits 7 and 6 of the presses, Z set if it did not turn; A the
; facing, or 1 and N and Z from it when it turned left. X, Y and C kept.
RichterTurnOnPress:
    lda a:OBJ_2AC0
    bne RichterTurnOnPress_at0
    tst #$80,z:zPadPressed
    beq RichterTurnOnPress_at1
    lda #$01
    sta a:OBJ_2AC0
    rts
RichterTurnOnPress_at0:
    tst #$20,z:zPadPressed
    beq RichterTurnOnPress_at1
    stz a:OBJ_2AC0
RichterTurnOnPress_at1:
    rts

; @routine 01B9:2FD RichterStandStep1
; @run 6D:A2FD
; WHAT IT DOES: The standing state's attacking step: reads the ground, falls if nothing is
; below; turns with a press while the attack is young; with the whip out and up held early,
; crouches; otherwise waits for the animation's end, then stands.
; HOW: OBJ_FLAGS_2F38 counts the attack's progress; object 3 is the whip; PlayerByte3BD5
; set skips to the wait. RichterAnimationWait returns past this routine while it runs.
; WAITS: no.
; Entry: nothing. Exit: falling, as $A9DD. Crouched, A 2, N and Z clear, as the animation's
; tick left the rest. Waiting, as RichterAnimationRunning, A $A3 and N set, the return's high
; byte. Stood, as RichterStand. In each, what the checks before it left.
RichterStandStep1:
    jsr RichterReadColumns
    jsr RichterHasNoFooting
    bcc RichterStandStep1_at0
    jmp EventFlagsClear_at1
RichterStandStep1_at0:
    jsr RichterLandOnFloor
    lda a:OBJ_FLAGS_2F38
    cmp #$03
    bcs RichterStandStep1_at1
    bsr RichterTurnOnPress
RichterStandStep1_at1:
    lda a:PlayerByte3BD5
    bne RichterStandStep1_at2
    lda a:OBJ_2750+3
    cmp #$01
    bne RichterStandStep1_at2
    lda a:OBJ_FLAGS_2F38
    cmp #$06
    bcs RichterStandStep1_at3
    jsr RichterUpHeld
    bcs RichterStandStep1_at4
RichterStandStep1_at2:
    jsr RichterAnimationWait
    stz a:PlayerByte3BD5
    jmp RichterStand
RichterStandStep1_at4:
    lda #$06
    sta a:OBJ_FIELDS_2EE0
    jsr RichterAnimationTick
    lda #$02
    sta a:OBJ_27A8
    rts
RichterStandStep1_at3:
    lda a:OBJ_2F90
    cmp #$04
    bcs RichterStandStep1_at2
    bra RichterStandStep1_at2

; @routine 01B9:34C RichterClearWhip
; @run 6D:A34C
; WHAT IT DOES: Clears object 3, the whip.
; HOW: ClearObject with X 3.
; WAITS: no.
; Entry: nothing. Exit: X 3, N and Z from it. A, Y, C and V kept.
RichterClearWhip:
    ldx #$03
    jmp VecClearObject

; @routine 01B9:351 RichterOrphanClearWhipAir
; @run 6D:A351
; WHAT IT DOES: Clears the whip and takes to the air. Nothing reached calls it.
; HOW: RichterClearWhip, RichterToAir.
; WAITS: no.
; Entry: nothing. Exit: as RichterToAir.
RichterOrphanClearWhipAir:
    bsr RichterClearWhip
    jmp RichterToAir

; @routine 01B9:358 RichterStandStep2
; @run 6D:A358
; WHAT IT DOES: The standing state's item-crash step: turns with the pad, and while its timer
; runs reads the ground (in state 0) and falls if nothing is below, jumps on a press, or steps
; the animation and spawns object $0C in the first free slot from 5. When the timer has run
; out, stands, or ends the item crash outside state 0.
; HOW: OBJ_30F0 is the timer. Ending, falling and jumping count PlayerByte3BD6 up. A jump in
; the crash's states goes up the ledge by the first table below state 7, the second from it.
; WAITS: no.
; Entry: nothing. Exit: as the transition taken, after what the steps before it left. Spawned,
; A $0C, Y the slot, N clear, Z clear. No free slot, A the last slot's OBJ_2750, Y $10, Z and
; C set, N clear.
RichterStandStep2:
    lda a:OBJ_2AC0
    bne RichterStandStep2_at0
    tst #$80,z:zPadHeld
    beq RichterStandStep2_at1
    lda #$01
    sta a:OBJ_2AC0
    bra RichterStandStep2_at1
RichterStandStep2_at0:
    tst #$20,z:zPadHeld
    beq RichterStandStep2_at1
    stz a:OBJ_2AC0
RichterStandStep2_at1:
    lda a:OBJ_30F0
    beq RichterStandStep2_at2
    dec a:OBJ_30F0
    lda a:OBJ_27A8
    cmp #$00
    bne RichterStandStep2_at3
    jsr RichterReadColumns
    jsr RichterHasNoFooting
    bcs RichterStandStep2_at4
RichterStandStep2_at3:
    jsr RichterJumpPressed
    bcs RichterStandStep2_at5
    jsr RichterAnimationTick
    ldy #$05
RichterStandStep2_at7:
    lda a:OBJ_2750,y
    beq RichterStandStep2_at6
    iny
    cpy #$10
    bne RichterStandStep2_at7
    rts
RichterStandStep2_at6:
    lda #$0C
    sta a:OBJ_2750,y
    rts
RichterStandStep2_at2:
    inc a:PlayerByte3BD6
    lda a:OBJ_27A8
    cmp #$00
    bne RichterStandStep2_at8
    jmp RichterStand
RichterStandStep2_at8:
    jmp RichterEndItemCrash
RichterStandStep2_at4:
    inc a:PlayerByte3BD6
    jmp RichterToFall
RichterStandStep2_at5:
    inc a:PlayerByte3BD6
    lda a:OBJ_27A8
    cmp #$00
    bne RichterStandStep2_at9
    jmp RichterToAir
RichterStandStep2_at9:
    cmp #$07
    bcs RichterStandStep2_at10
    jmp RichterUpWhipLeft
RichterStandStep2_at10:
    jmp RichterUpWhipRight

; @routine 01B9:3D0 RichterStepIndex
; @run 6D:A3D0
; WHAT IT DOES: Gives the state's step, for its caller to index a table of words by.
; HOW: OBJ_2800, which the wrapper leaves doubled in A and X.
; WAITS: no.
; Entry: nothing. Exit: A and X the offset, N and Z from it, C the step's bit 7. Y, V kept.
RichterStepIndex:
    lda a:OBJ_2800
    asl a
    tax
    rts

; @routine 01B9:3D6 RichterStateStand
; @run 6D:A3D6
; WHAT IT DOES: State 0, standing: runs the step's handler: still, attacking, or the item
; crash.
; HOW: PlayerStandHandlers, three words by RichterStepIndex.
; WAITS: no.
; Entry: nothing. Exit: as the handler leaves them.
RichterStateStand:
    bsr RichterStepIndex
    jmp (PlayerStandHandlers,x)

; @routine 01B9:3E1 RichterStandStep0
; @run 6D:A3E1
; WHAT IT DOES: Standing: reads the ground he moved onto, then the first thing that applies:
; nothing below is a fall; a jump; the item crash; an item or the whip; a ledge to climb; up
; crouches; down, or PlayerByte3BF0, looks down; and left or right start walking.
; HOW: Each check leaves its answer in C. The look down is animation $47 set outright; otherwise
; the animation steps.
; WAITS: no.
; Entry: nothing. Exit: as the transition taken, with what the checks before it left. Still,
; Y 1, Z set, N and V the pad's bits 7 and 6, C clear, A as the look or the animation left it.
RichterStandStep0:
    jsr RichterReadColumns
    jsr RichterHasNoFooting
    bcc RichterStandStep0_at0
    jmp RichterToFall
RichterStandStep0_at0:
    jsr RichterJumpPressed
    bcc RichterStandStep0_at1
    jmp RichterToAir
RichterStandStep0_at1:
    jsr RichterLandOnFloor
    jsr RichterTryItemCrash
    bcc RichterStandStep0_at2
    jmp RichterToHurtAir
RichterStandStep0_at2:
    jsr RichterUseItem
    bcc RichterStandStep0_at3
    jmp RichterAttack
RichterStandStep0_at3:
    jsr RichterLedgeCheck
    bcc RichterStandStep0_at4
    jmp RichterClimbStart
RichterStandStep0_at4:
    jsr RichterUpHeld
    bcc RichterStandStep0_at5
    jmp RichterCrouch
RichterStandStep0_at5:
    tst #$10,z:zPadHeld
    beq RichterStandStep0_at6
    lda #$47
    sta a:OBJ_2A68
    bra RichterStandStep0_at7
RichterStandStep0_at6:
    lda a:PlayerByte3BF0
    beq RichterStandStep0_at8
    lda #$47
    sta a:OBJ_2A68
    bra RichterStandStep0_at7
RichterStandStep0_at8:
    jsr RichterAnimationTick
RichterStandStep0_at7:
    cly
    tst #$20,z:zPadHeld
    bne RichterStandStep0_at9
    iny
    tst #$80,z:zPadHeld
    bne RichterStandStep0_at9
    rts
RichterStandStep0_at9:
    jmp RichterStartWalk

; @routine 01B9:442 RichterStartWalk
; @run 6D:A442
; WHAT IT DOES: Starts walking the way the pad says, unless a button stops it; a held jump
; keeps the facing and walks backward.
; HOW: Y is the new facing, kept unless bit 1 is held. Animation 0 or $32.
; WAITS: no.
; Entry: Y the facing. Exit: as RichterSetAnimation; with an attack held, A, X, Y and C kept,
; N and V the pad's bits 7 and 6, Z clear.
RichterStartWalk:
    jsr RichterPadOverridesOff
    tst #$02,z:zPadHeld
    bne RichterStartWalk_at0
    sty a:OBJ_2AC0
RichterStartWalk_at0:
    tst #$01,z:zPadHeld
    beq RichterStartWalk_at1
    rts
RichterStartWalk_at1:
    lda #$01
    sta a:OBJ_27A8
    lda #$00
    sta a:OBJ_2800
    tst #$02,z:zPadHeld
    bne RichterStartWalk_at2
    lda #$00
    jmp RichterSetAnimation
RichterStartWalk_at2:
    lda #$32
    jmp RichterSetAnimation

; @routine 01B9:46C RichterSpeedWalk
; @run 6D:A46C
; WHAT IT DOES: Sets the X speed to walking, by facing.
; HOW: WalkSpeeds.
; WAITS: no.
; Entry: nothing. Exit: A the speed's high byte, N and Z from it, Y twice the facing, C
; the facing's bit 7. X and V kept.
RichterSpeedWalk:
    lda a:OBJ_2AC0
    asl a
    tay
    lda a:WalkSpeeds,y
    sta a:OBJ_VX_LO
    lda a:WalkSpeeds+1,y
    sta a:OBJ_VX_HI
    rts

; @routine 01B9:47E RichterSpeedJump
; @run 6D:A47E
; WHAT IT DOES: Sets the X speed for a jump, by facing.
; HOW: JumpSpeeds.
; WAITS: no.
; Entry: nothing. Exit: as RichterSpeedWalk.
RichterSpeedJump:
    lda a:OBJ_2AC0
    asl a
    tay
    lda a:JumpSpeeds,y
    sta a:OBJ_VX_LO
    lda a:JumpSpeeds+1,y
    sta a:OBJ_VX_HI
    rts

; @routine 01B9:494 RichterSpeedBackstep
; @run 6D:A494
; WHAT IT DOES: Sets the X speed for walking backward, by facing.
; HOW: BackstepSpeeds.
; WAITS: no.
; Entry: nothing. Exit: as RichterSpeedWalk.
RichterSpeedBackstep:
    lda a:OBJ_2AC0
    asl a
    tay
    lda a:BackstepSpeeds,y
    sta a:OBJ_VX_LO
    lda a:BackstepSpeeds+1,y
    sta a:OBJ_VX_HI
    rts

; @routine 01B9:4AE RichterCrouch
; @run 6D:A4AE
; WHAT IT DOES: Crouches: state 2, step 3, animation $0C, all speed gone.
; HOW: Runs on into RichterStopMoving.
; WAITS: no.
; Entry: nothing. Exit: as RichterSetAnimation. X and V kept.
RichterCrouch:
    jsr RichterPadOverridesOff
    lda #$02
    sta a:OBJ_27A8
    lda #$03
    sta a:OBJ_2800
    lda #$0C
    jsr RichterSetAnimation

; @routine 01B9:4C0 RichterStopMoving
; @run 6D:A4C0
; WHAT IT DOES: Stops the player along Y and X.
; HOW: Runs on into RichterStopX.
; WAITS: no.
; Entry: nothing. Exit: all registers and flags kept.
RichterStopMoving:
    stz a:OBJ_VY_LO
    stz a:OBJ_VY_HI

; @routine 01B9:4C6 RichterStopX
; @run 6D:A4C6
; WHAT IT DOES: Stops the player along X.
; HOW: Zeroes the speed.
; WAITS: no.
; Entry: nothing. Exit: all registers and flags kept.
RichterStopX:
    stz a:OBJ_VX_LO
    stz a:OBJ_VX_HI
    rts

; @routine 01B9:4CD RichterAttack
; @run 6D:A4CD
; WHAT IT DOES: Starts an attack standing: state 0, step 1, the animation by the attack's
; kind; the whip, with its sound, or the kind's object in a free slot among 5, 8 and $0B.
; HOW: PlayerByte3BD5 is the kind. zField93 is what goes in the slot. SoundCommand 3 for the whip.
; WAITS: no.
; Entry: nothing. Exit: Y and C as RichterSetAnimation. The whip, as SoundCommand with command
; 3 and X 3; an item, A the item, X its slot, N, Z and V from testing the last slot looked at.
RichterAttack:
    jsr RichterPadOverridesOff
    stz a:PlayerByte3BE1
    jsr RichterFaceByPad
    lda #$00
    sta a:OBJ_27A8
    jsr RichterStopMoving
    ldx a:PlayerByte3BD5
    lda a:LandAnimations,x
    jsr RichterSetAnimation
    lda #$01
    sta a:OBJ_2800
    lda a:PlayerByte3BD5
    bne RichterAttack_at0
    jsr RichterClearWhip
    lda #$01
    sta a:OBJ_2750+3
    lda #$03
    jmp VecSoundCommand
RichterAttack_at0:
    lda z:zField93
    tst #$FF,a:OBJ_2750+5
    bne RichterAttack_at1
    ldx #$05
    jsr VecClearObject
    sta a:OBJ_2750,x
    rts
RichterAttack_at1:
    tst #$FF,a:OBJ_2750+8
    bne FallUntilFloor
    ldx #$08
    bra FallThenWait
    ldx #$0B
    bra FallThenWait

; @routine 01B9:521 RichterToFall
; @run 6D:A521
; WHAT IT DOES: Starts falling: state 4, animation $10, Y speed 3, no X speed.
; HOW: Runs on into RichterPadOverridesOff.
; WAITS: no.
; Entry: nothing. Exit: A 4, N and Z from it; Y and C as RichterSetAnimation. X and V kept.
RichterToFall:
    lda #$04
    sta a:OBJ_27A8
    lda #$00
    sta a:OBJ_2800
    lda #$10
    jsr RichterSetAnimation
    lda #$00
    sta a:OBJ_VY_LO
    lda #$03
    sta a:OBJ_VY_HI
    lda #$04
    sta a:OBJ_34B8
    jsr RichterStopX

; @routine 01B9:542 RichterPadOverridesOff
; @run 6D:A542
; WHAT IT DOES: Clears the pad's overrides.
; HOW: Three bytes zeroed.
; WAITS: no.
; Entry: nothing. Exit: all registers and flags kept.
RichterPadOverridesOff:
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz z:zFieldD3
    rts

; @routine 01B9:54B RichterLedgeHurtA
; @run 6D:A54B
; WHAT IT DOES: A hurt jump off a ledge the first way, or item $05's crash.
; HOW: Nudges off the ledge and to RichterToHurtAir, or animation $2B.
; WAITS: no.
; Entry: nothing. Exit: item 5, A $60, N and Z from it, Y and C as RichterSetAnimation. Otherwise
; as RichterToHurtAir after RichterNudgeA.
RichterLedgeHurtA:
    jsr RichterPadOverridesOff
    lda z:zField93
    cmp #$05
    beq RichterLedgeHurtB_at0
    jsr RichterNudgeA
    jmp RichterToHurtAir

; @routine 01B9:55A RichterLedgeHurtB
; @run 6D:A55A
; WHAT IT DOES: A hurt jump off a ledge the second way, or item $05's crash.
; HOW: As RichterLedgeHurtA with the other nudge and animation $2C.
; WAITS: no.
; Entry: nothing. Exit: item 5, A $60, N and Z from it, Y and C as RichterSetAnimation. Otherwise
; as RichterToHurtAir after RichterNudgeB.
RichterLedgeHurtB:
    jsr RichterPadOverridesOff
    lda z:zField93
    cmp #$05
    beq RichterLedgeHurtB_at1
    jsr RichterNudgeB
    jmp RichterToHurtAir
RichterLedgeHurtB_at0:
    lda #$2B
RichterLedgeHurtB_at2:
    jsr RichterSetAnimation
    jsr RichterStopMoving
    lda #$05
    sta a:OBJ_2800
    lda #$60
    sta a:OBJ_30F0
    rts
RichterLedgeHurtB_at1:
    lda #$2C
    bra RichterLedgeHurtB_at2

; @routine 01B9:580 RichterUpWhipLeft
; @run 6D:A580
; WHAT IT DOES: Off a ledge by the first table of nudges: without up held, up into the air at
; $FAC0; with up held at a ladder, onto it, falling.
; HOW: The pad's overrides, PadForcedPressB and zFieldC6 cleared; bit 6 of zPadHeld;
; RichterLadderCheck; LedgeNudge by facing.
; WAITS: no.
; Entry: nothing. Exit: the jump, as RichterNudgeBJump with the first nudge. No ladder, as
; RichterLadderCheck. A ladder, as RichterToFall.
RichterUpWhipLeft:
    jsr RichterClearPadOverrides
    stz a:PadForcedPressB
    stz z:zFieldC6
    tst #$40,z:zPadHeld
    beq RichterUpWhipLeft_at0
    jsr RichterLadderCheck
    bcs RichterUpWhipLeft_at1
    rts
RichterUpWhipLeft_at1:
    ldx a:OBJ_2AC0
    lda a:LedgeNudge,x
    bra RichterUpWhipRight_at0
RichterUpWhipLeft_at0:
    bsr RichterNudgeA
    jsr RichterToAir
    lda #$C0
    sta a:OBJ_VY_LO
    lda #$FA
    sta a:OBJ_VY_HI
    rts

; @routine 01B9:5AB RichterNudgeA
; @run 6D:A5AB
; WHAT IT DOES: Moves the player off a ledge by the first table, by facing, and clears
; PlayerMayLeaveTop.
; HOW: One add to the X position.
; WAITS: no.
; Entry: nothing. Exit: A the position, N, Z, C, V from the add, X the facing. Y kept.
RichterNudgeA:
    ldx a:OBJ_2AC0
    lda a:LedgeNudge,x
RichterNudgeA_at0:
    clc
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    stz a:PlayerMayLeaveTop
    rts

; @routine 01B9:5BC RichterNudgeB
; @run 6D:A5BC
; WHAT IT DOES: As RichterNudgeA by the second table.
; HOW: Goes back into it.
; WAITS: no.
; Entry: nothing. Exit: as RichterNudgeA.
RichterNudgeB:
    ldx a:OBJ_2AC0
    lda a:LedgeNudgeB,x
    bra RichterNudgeA_at0

; @routine 01B9:5C4 RichterNudgeBJump
; @run 6D:A5C4
; WHAT IT DOES: The second nudge, and the jump up off the ledge.
; HOW: RichterNudgeB, then as RichterUpWhipLeft's jump.
; WAITS: no.
; Entry: nothing. Exit: A $FA, N set, Z clear; C, V and Y as RichterToAir after the nudge.
RichterNudgeBJump:
    bsr RichterNudgeB
    bra $A59D

; @routine 01B9:5C8 RichterUpWhipRight
; @run 6D:A5C8
; WHAT IT DOES: As RichterUpWhipLeft by the second table of nudges.
; HOW: The same tests; the jump is RichterNudgeBJump.
; WAITS: no.
; Entry: nothing. Exit: as RichterUpWhipLeft.
RichterUpWhipRight:
    jsr RichterClearPadOverrides
    stz a:PadForcedPressB
    stz z:zFieldC6
    tst #$40,z:zPadHeld
    beq RichterNudgeBJump
    jsr RichterLadderCheck
    bcs RichterUpWhipRight_at1
    rts
RichterUpWhipRight_at1:
    ldx a:OBJ_2AC0
    lda a:LedgeNudgeB,x
RichterUpWhipRight_at0:
    clc
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    stz a:PlayerMayLeaveTop
    jmp RichterToFall

; @routine 01B9:5F2 RichterToHurtAir
; @run 6D:A5F2
; WHAT IT DOES: Hurt jump: state 3, step 2, the jump's speeds, a long hit timer, animation
; $23, its sound, and a reaction by zField93. Item $05 goes to RichterItemCrashFive.
; HOW: Table by zField93, 0 to 8; the C takes anything past 8 as 8. SoundCommand $65.
; WAITS: no.
; Entry: nothing. Exit: item 5 first, as RichterItemCrashFive. Otherwise as the reaction, after
; A and X twice the item, N and Z from it, C its bit 7, Y as the animation and V as the sound left
; them.
; @stop A621
RichterToHurtAir:
    jsr RichterPadOverridesOff
    lda z:zField93
    cmp #$05
    beq RichterItemCrashFive
    lda #$03
    sta a:OBJ_27A8
    lda #$02
    sta a:OBJ_2800
    jsr RichterJumpSpeed
    lda #$FF
    sta a:OBJ_3460
    stz a:OBJ_3250
    lda #$23
    jsr RichterSetAnimation
    lda #$65
    jsr VecSoundCommand
    lda z:zField93
    asl a
    tax
    jmp (HitReactionHandlers,x)

; @routine 01B9:633 RichterReactFillD
; @run 6D:A633
; WHAT IT DOES: Fills the slots from 5 with object $0D.
; HOW: A loop to $10.
; WAITS: no.
; Entry: nothing. Exit: A $0D, Y $10, N, Z, C from comparing it.
RichterReactFillD:
    lda #$0D
RichterReactFillD_at1:
    ldy #$05
RichterReactFillD_at0:
    sta a:OBJ_2750,y
    iny
    cpy #$10
    bne RichterReactFillD_at0
    rts

; @routine 01B9:640 RichterReactFillB
; @run 6D:A640
; WHAT IT DOES: Fills the slots from 5 with object $0B.
; HOW: Goes back into the loop.
; WAITS: no.
; Entry: nothing. Exit: A $0B, Y $10, N, Z, C from comparing it.
RichterReactFillB:
    lda #$0B
    bra RichterReactFillD_at1

; @routine 01B9:644 RichterItemCrashFive
; @run 6D:A644
; WHAT IT DOES: Item $05's crash, from a hurt jump: no speed, animation $2A, its sound,
; state 0, step 2, its timer.
; HOW: The fields in turn.
; WAITS: no.
; Entry: nothing. Exit: A $60, N and Z from it; C and V as SoundCommand left them.
RichterItemCrashFive:
    jsr RichterStopMoving
    lda #$2A
    jsr RichterSetAnimation
    lda #$65
    jsr VecSoundCommand
    lda #$00
    sta a:OBJ_27A8
    lda #$02
    sta a:OBJ_2800
    lda #$60
    sta a:OBJ_30F0
    rts

; @routine 01B9:661 RichterReactE
; @run 6D:A661
; WHAT IT DOES: Puts object $0E in slot 5.
; HOW: One store.
; WAITS: no.
; Entry: nothing. Exit: A $0E, N and Z from it. X, Y, C, V kept.
RichterReactE:
    lda #$0E
RichterReactE_at0:
    sta a:OBJ_2750+5
    rts

; @routine 01B9:667 RichterReact11
; @run 6D:A667
; WHAT IT DOES: Puts object $11 in slot 5, and sets the hit timer $60 with the player live.
; HOW: Three stores.
; WAITS: no.
; Entry: nothing. Exit: A $60, N and Z from it. X, Y, C, V kept.
RichterReact11:
    lda #$11
    sta a:OBJ_2750+5
    lda #$01
    sta a:OBJ_3250
    lda #$60
    sta a:OBJ_3460
    rts

; @routine 01B9:677 RichterReact10
; @run 6D:A677
; WHAT IT DOES: Puts object $10 in slot 5.
; HOW: Goes back into RichterReactE's store.
; WAITS: no.
; Entry: nothing. Exit: A $10, N and Z from it. X, Y, C, V kept.
RichterReact10:
    lda #$10
    bra RichterReactE_at0

; @routine 01B9:67B RichterReact12
; @run 6D:A67B
; WHAT IT DOES: Puts object $12 in slot 5.
; HOW: Goes back into RichterReactE's store.
; WAITS: no.
; Entry: nothing. Exit: A $12, N and Z from it. X, Y, C, V kept.
RichterReact12:
    lda #$12
    bra RichterReactE_at0

; @routine 01B9:67F RichterReact13
; @run 6D:A67F
; WHAT IT DOES: Puts object $13 in slot 5.
; HOW: Goes back into RichterReactE's store.
; WAITS: no.
; Entry: nothing. Exit: A $13, N and Z from it. X, Y, C, V kept.
RichterReact13:
    lda #$13
    bra RichterReactE_at0

; @routine 01B9:683 RichterKnockBack
; @run 6D:A683
; WHAT IT DOES: Knocked into the air: state 3, animation $31, the knock's speeds, marks
; PlayerByte3BF1 bit 0.
; HOW: The jump's speed replaced by the knock's.
; WAITS: no.
; Entry: nothing. Exit: A PlayerByte3BF1, N and Z from it; Y twice the facing, C the facing's
; bit 7; V as RichterLiftThree. X kept.
RichterKnockBack:
    stz a:PlayerMayLeaveTop
    stz a:PadForcedPressB
    stz z:zFieldC6
    jsr RichterJumpSpeed
    lda #$03
    sta a:OBJ_27A8
    lda #$00
    sta a:OBJ_2800
    lda #$31
    jsr RichterSetAnimation
    lda a:OBJ_2AC0
    asl a
    tay
    lda a:KnockSpeeds,y
    sta a:OBJ_VX_LO
    lda a:KnockSpeeds+1,y
    sta a:OBJ_VX_HI
    lda #$C0
    sta a:OBJ_VY_LO
    lda #$FD
    sta a:OBJ_VY_HI
    lda #$01
    ora a:PlayerByte3BF1
    sta a:PlayerByte3BF1
    rts

; @routine 01B9:6C5 RichterToAir
; @run 6D:A6C5
; WHAT IT DOES: Takes to the air: state 3, the jump's speed; with left or right held, a
; moving jump, and when that faces the way the player already did, marks it in
; PlayerByte3BF1; otherwise animation $0A.
; HOW: The facing the pad asks for is set for the jump's speed, then the old one put back.
; WAITS: no.
; Entry: nothing. Exit: V as RichterLiftThree; X kept. Marked, A $80, N set, Z clear, C set, Y
; twice the facing asked for; otherwise as RichterSetAnimation.
RichterToAir:
    stz a:PlayerMayLeaveTop
    stz a:PadForcedPressB
    stz z:zFieldC6
    lda #$03
    sta a:OBJ_27A8
    lda #$00
    sta a:OBJ_2800
    bsr RichterJumpSpeed
    cly
    bbs5 zPadHeld,RichterToAir_at0
    iny
    bbs7 zPadHeld,RichterToAir_at0
    lda #$0A
    jmp RichterSetAnimation
RichterToAir_at0:
    lda a:OBJ_2AC0
    pha
    sty a:OBJ_2AC0
    lda #$02
    jsr RichterSetAnimation
    jsr RichterSpeedJump
    pla
    cmp a:OBJ_2AC0
    bne RichterToAir_at1
    sta a:OBJ_2AC0
    lda #$80
    sta a:PlayerByte3BF1
    rts
RichterToAir_at1:
    sta a:OBJ_2AC0
    lda #$0A
    jmp RichterSetAnimation

; @routine 01B9:70C RichterJumpSpeed
; @run 6D:A70C
; WHAT IT DOES: Sets a jump's Y speed, stopped along X, and lifts the player off the floor.
; HOW: Speed $FB50; runs on into RichterLiftThree.
; WAITS: no.
; Entry: nothing. Exit: as RichterLiftThree.
RichterJumpSpeed:
    stz a:PlayerByte3BF1
    jsr RichterStopMoving
    lda #$50
    sta a:OBJ_VY_LO
    lda #$FB
    sta a:OBJ_VY_HI

; @routine 01B9:71C RichterLiftThree
; @run 6D:A71C
; WHAT IT DOES: Lifts the player three pixels, stopping at the top of the map.
; HOW: A 16-bit subtract; high $FF means past the top, and the position becomes zero.
; WAITS: no.
; Entry: nothing. Exit: A the position's high byte; N, Z, C from comparing it with $FF; V
; from the subtract. X and Y kept.
RichterLiftThree:
    lda a:OBJ_Y_LO
    sec
    sbc #$03
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    sbc #$00
    sta a:OBJ_Y_HI
    cmp #$FF
    beq RichterLiftThree_at0
    rts
RichterLiftThree_at0:
    stz a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    rts

; @routine 01B9:739 RichterStateWalk
; @run 6D:A739
; WHAT IT DOES: State 1, walking: a floor under him, or one pixel lower, or he falls; then the
; first that applies of a jump, the item crash, an item or the whip, a ledge to climb, up to
; crouch. Otherwise the animation steps, and the pad held his way keeps him walking; held the
; other way with button II, he steps back; else he stands.
; HOW: RichterReadColumns and RichterFloorCheck, again after OBJ_Y_LO is stepped down; the
; checks as in the stand's first step. OBJ_FIELDS_2EE0 0 to walk, $32 to step back.
; WAITS: no.
; Entry: nothing. Exit: as the transition taken, with what the checks before it left.
RichterStateWalk:
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcs RichterStateWalk_at0
    inc a:OBJ_Y_LO
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcs RichterStateWalk_at0
    jmp RichterToFall
RichterStateWalk_at0:
    jsr RichterJumpPressed
    bcc RichterStateWalk_at1
    jmp RichterToAir
RichterStateWalk_at1:
    jsr RichterLandOnFloor
    jsr RichterTryItemCrash
    bcc RichterStateWalk_at2
    jmp RichterToHurtAir
RichterStateWalk_at2:
    jsr RichterUseItem
    bcc RichterStateWalk_at3
    jmp RichterAttack
RichterStateWalk_at3:
    jsr RichterLedgeCheck
    bcc RichterStateWalk_at4
    jmp RichterClimbStart
RichterStateWalk_at4:
    jsr RichterUpHeld
    bcc RichterStateWalk_at5
    jmp RichterCrouch
RichterStateWalk_at5:
    jsr RichterAnimationTick
    lda a:OBJ_2AC0
    bne RichterStateWalk_at6
    bbr5 zPadHeld,RichterStateWalk_at7
    bra RichterStateWalk_at8
RichterStateWalk_at6:
    bbr7 zPadHeld,RichterStateWalk_at7
RichterStateWalk_at8:
    lda #$00
    sta a:OBJ_FIELDS_2EE0
    jmp RichterSpeedWalk
RichterStateWalk_at7:
    tst #$02,z:zPadHeld
    beq RichterStand
    lda a:OBJ_2AC0
    beq RichterStateWalk_at9
    bbr5 zPadHeld,RichterStand
    bra RichterStateWalk_at10
RichterStateWalk_at9:
    bbr7 zPadHeld,RichterStand
RichterStateWalk_at10:
    lda #$32
    sta a:OBJ_FIELDS_2EE0
    jmp RichterSpeedBackstep

; @routine 01B9:7AC RichterStand
; @run 6D:A7AC
; WHAT IT DOES: Stands the player: state 0, step 0, animation 1, no speed.
; HOW: Clears PlayerMayLeaveTop and PlayerByte3BD5 first.
; WAITS: no.
; Entry: nothing. Exit: as RichterSetAnimation. X and V kept.
RichterStand:
    stz a:PlayerMayLeaveTop
    stz a:PlayerByte3BD5
    lda #$00
    sta a:OBJ_27A8
    lda #$00
    sta a:OBJ_2800
    lda #$01
    jsr RichterSetAnimation
    jmp RichterStopMoving

; @routine 01B9:7C4 RichterClimbStart
; @run 6D:A7C4
; WHAT IT DOES: Takes hold of the ledge RichterLedgeCheck found, to climb it: the low ledge
; (zTemp01 0) or the high one, each in one of two holds by whether the point past the ledge's
; edge reads bit 2: states 5 and 6 low, 7 and 8 high. PlayerClimbReach is how far he has to
; go across to the edge. He stops, and the state's step is 2.
; HOW: The column's offset (RichterColumnOffset) and 8 more past it, read $0A below him for the
; low ledge, $1E for the high; the reach from the offset less zTemp00 for the low, less the
; column's RichterColumnX for the high. The stand's animation unless he was walking.
; WAITS: no.
; Entry: X the column, zTemp00 the low ledge's X, zTemp01 low or high. Exit: A 2, N and Z clear,
; X the state; Y the column's index, or as RichterSetAnimation; C set unless he was standing, or as
; RichterSetAnimation; V from the reach's last add or subtract.
RichterClimbStart:
    jsr RichterPadOverridesOff
    stz z:zFieldC4
    lda #$01
    sta a:PlayerMayLeaveTop
    lda z:zTemp01
    bne RichterClimbStart_at0
    lda z:zTemp00
    pha
    jsr RichterColumnOffset
    clc
    adc a:OBJ_X_LO
    clc
    adc #$08
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0A
    jsr RichterReadBelow
    and #$04
    beq RichterClimbStart_at1
    pla
    sta z:zTemp00
    jsr RichterColumnOffset
    sec
    sbc z:zTemp00
    sec
    sbc #$08
    ldx #$05
RichterClimbStart_at3:
    sta a:PlayerClimbReach
    lda a:OBJ_27A8
    cmp #$01
    beq RichterClimbStart_at2
    lda #$00
    jsr RichterSetAnimation
RichterClimbStart_at2:
    lda #$02
    sta a:OBJ_2800
    stx a:OBJ_27A8
    jmp RichterStopMoving
RichterClimbStart_at1:
    pla
    sta z:zTemp00
    jsr RichterColumnOffset
    sec
    sbc z:zTemp00
    clc
    adc #$07
    clc
    adc #$08
    ldx #$06
    bra RichterClimbStart_at3
RichterClimbStart_at0:
    jsr RichterColumnOffset
    clc
    adc a:OBJ_X_LO
    clc
    adc #$08
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$1E
    jsr RichterReadBelow
    and #$04
    beq RichterClimbStart_at4
    jsr RichterColumnOffset
    sec
    sbc a:RichterColumnX,x
    sec
    sbc #$01
    ldx #$07
    bra RichterClimbStart_at3
RichterClimbStart_at4:
    jsr RichterColumnOffset
    sec
    sbc a:RichterColumnX,x
    clc
    adc #$08
    ldx #$08
    bra RichterClimbStart_at3

; @routine 01B9:85E RichterStateCrouch
; @run 6D:A85E
; WHAT IT DOES: State 2, crouching: runs the step's handler: down, attacking, nothing, the
; crouch's end, or rising.
; HOW: RichterCrouchHandlers, five words by RichterStepIndex; step 2's is the RTS of step 0.
; WAITS: no.
; Entry: nothing. Exit: as the handler leaves them; step 2, as RichterStepIndex.
RichterStateCrouch:
    jsr RichterStepIndex
    jmp (RichterCrouchHandlers,x)

; @routine 01B9:86E RichterCrouchStep0
; @run 6D:A86E
; WHAT IT DOES: Crouched: reads the ground, falls if nothing is below; then a jump, the item
; crash, an item or the whip from the crouch; with up let go he starts to rise (animation $0D,
; step 4); held, left or right alone turns him.
; HOW: The ground checks in turn, each answering in C. zPadPressed bits 7 and 5: right alone
; faces right, left alone left, both or neither leave him.
; WAITS: no.
; Entry: nothing. Exit: as the transition taken, with what the checks before it left. Rising,
; A 4, as RichterSetAnimation left the rest. Still down, A the presses' left and right bits,
; C set, Z clear only for left alone.
RichterCrouchStep0:
    jsr RichterReadColumns
    jsr RichterHasNoFooting
    bcc RichterCrouchStep0_at0
    jmp RichterToFall
RichterCrouchStep0_at0:
    jsr RichterLandOnFloor
    jsr RichterJumpPressed
    bcc RichterCrouchStep0_at1
    jmp RichterToAir
RichterCrouchStep0_at1:
    jsr RichterTryItemCrash
    bcc RichterCrouchStep0_at2
    jmp RichterToHurtAir
RichterCrouchStep0_at2:
    jsr RichterUseItem
    bcs RichterCrouchAttack
    jsr RichterUpHeld
    bcc RichterCrouchStep0_at3
    lda z:zPadPressed
    and #$A0
    beq RichterCrouchStep0_at4
    cmp #$A0
    beq RichterCrouchStep0_at4
    stz a:OBJ_2AC0
    cmp #$20
    beq RichterCrouchStep0_at4
    inc a:OBJ_2AC0
RichterCrouchStep0_at4:
    rts
RichterCrouchStep0_at3:
    lda #$0D
    jsr RichterSetAnimation
    lda #$04
    sta a:OBJ_2800
    rts

; @routine 01B9:8B6 RichterCrouchAttack
; @run 6D:A8B6
; WHAT IT DOES: Starts an attack from the crouch: turned by the pad, stopped, animation 6, and
; the whip or the item as a standing attack starts them.
; HOW: As RichterAttack from its animation on, keeping the state.
; WAITS: no.
; Entry: nothing. Exit: as RichterAttack.
RichterCrouchAttack:
    jsr RichterPadOverridesOff
    stz a:PlayerByte3BE1
    jsr RichterFaceByPad
    jsr RichterStopMoving
    lda #$06
    jmp $A4E4

; @routine 01B9:8C7 RichterCrouchStep1
; @run 6D:A8C7
; WHAT IT DOES: Attacking from the crouch: reads the ground (nothing below goes to $A9DD); turns
; with a press while the attack is young; with up let go before it is far along, stands; else
; waits for the animation's end, then is down again, animation $0E, step 0.
; HOW: OBJ_FLAGS_2F38 against 3 and 6. RichterAnimationWait returns past this routine while it
; runs.
; WAITS: no.
; Entry: nothing. Exit: falling, as $A9DD. Stood, as RichterAnimationTick, state 0 and
; OBJ_FIELDS_2EE0 3. Waiting, as RichterAnimationRunning, A $A8 and N set. Down, A $0E, N and Z
; clear, the rest as RichterAnimationRunning. With what the checks before left.
RichterCrouchStep1:
    jsr RichterReadColumns
    jsr RichterHasNoFooting
    bcc RichterCrouchStep1_at0
    jmp EventFlagsClear_at1
RichterCrouchStep1_at0:
    jsr RichterLandOnFloor
    lda a:OBJ_FLAGS_2F38
    cmp #$03
    bcs RichterCrouchStep1_at1
    jsr RichterTurnOnPress
RichterCrouchStep1_at1:
    lda a:OBJ_FLAGS_2F38
    cmp #$06
    bcs RichterCrouchStep1_at2
    jsr RichterUpHeld
    bcc RichterCrouchStep1_at3
RichterCrouchStep1_at2:
    jsr RichterAnimationWait
    stz a:PlayerByte3BD5
    lda #$0E
    sta a:OBJ_2A68
    stz a:OBJ_2800
    rts
RichterCrouchStep1_at3:
    lda #$00
    sta a:OBJ_27A8
    lda #$03
    sta a:OBJ_FIELDS_2EE0
    jmp RichterAnimationTick

; @routine 01B9:907 RichterCrouchStep3
; @run 6D:A907
; WHAT IT DOES: The crouch's end: the pad's overrides off; reads the ground, falls if nothing
; is below; a jump, the item crash, an item or the whip; else waits for the animation's end
; and goes back to step 0.
; HOW: The ground checks in turn. RichterAnimationWait returns past this routine while it runs.
; WAITS: no.
; Entry: nothing. Exit: as the transition taken; waiting, as RichterAnimationRunning, A $A9 and
; N set; ended, as RichterAnimationRunning. With what the checks before left.
RichterCrouchStep3:
    jsr RichterPadOverridesOff
    jsr RichterReadColumns
    jsr RichterHasNoFooting
    bcc RichterCrouchStep3_at0
    jmp RichterToFall
RichterCrouchStep3_at0:
    jsr RichterLandOnFloor
    jsr RichterJumpPressed
    bcc RichterCrouchStep3_at1
    jmp RichterToAir
RichterCrouchStep3_at1:
    jsr RichterTryItemCrash
    bcc RichterCrouchStep3_at2
    jmp RichterToHurtAir
RichterCrouchStep3_at2:
    jsr RichterUseItem
    bcc RichterCrouchStep3_at3
    jmp RichterCrouchAttack
RichterCrouchStep3_at3:
    jsr RichterAnimationWait
    stz a:OBJ_2800
    rts

; @routine 01B9:937 RichterCrouchStep4
; @run 6D:A937
; WHAT IT DOES: Rising from the crouch: reads the ground, falls if nothing is below; a jump,
; the item crash, a standing attack; else waits for the animation's end and stands.
; HOW: The ground checks in turn. RichterAnimationWait returns past this routine while it runs.
; WAITS: no.
; Entry: nothing. Exit: as the transition taken; waiting, as RichterAnimationRunning, A $A9 and
; N set; risen, as RichterStand. With what the checks before left.
RichterCrouchStep4:
    jsr RichterReadColumns
    jsr RichterHasNoFooting
    bcc RichterCrouchStep4_at0
    jmp RichterToFall
RichterCrouchStep4_at0:
    jsr RichterLandOnFloor
    jsr RichterJumpPressed
    bcc RichterCrouchStep4_at1
    jmp RichterToAir
RichterCrouchStep4_at1:
    jsr RichterTryItemCrash
    bcc RichterCrouchStep4_at2
    jmp RichterToHurtAir
RichterCrouchStep4_at2:
    jsr RichterUseItem
    bcc RichterCrouchStep4_at3
    jmp RichterAttack
RichterCrouchStep4_at3:
    jsr RichterAnimationWait
    jmp RichterStand

; @routine 01BA:389 RichterOnMarkedFloor
; @run 6D:AB89
; WHAT IT DOES: Says whether the floor under him is marked: once he is $10 down the screen, a
; column read with bit 5, or with bit 2 while down is held.
; HOW: OBJ_Y_LO; RichterColumnHit's bits 5 and 2; bit 4 of zPadHeld.
; WAITS: no.
; Entry: nothing. Exit: C set if marked. Too high, A his Y, N and Z from comparing it with $10.
; No bit, A 0, Z set. Bit 5, A $20, Z clear. Bit 2, A 0, Z clear if down is held, N and V the
; pad's bits 7 and 6. X, Y kept; V kept but for the pad's test.
RichterOnMarkedFloor:
    lda a:OBJ_Y_LO
    cmp #$10
    bcc RichterOnMarkedFloor_at0
    lda a:RichterColumnHit
    and #$24
    beq RichterOnMarkedFloor_at0
    and #$20
    bne RichterOnMarkedFloor_at1
    tst #$10,z:zPadHeld
    beq RichterOnMarkedFloor_at0
RichterOnMarkedFloor_at1:
    sec
    rts
RichterOnMarkedFloor_at0:
    clc
    rts

; @routine 01BA:3CB RichterFallGravity
; @run 6D:ABCB
; WHAT IT DOES: Pulls him down while he falls: $38 more Y speed a frame, held at $0700.
; HOW: OBJ_VY_LO and Hi; at 7 and up the low byte is cleared.
; WAITS: no.
; Entry: nothing. Exit: A the speed's high byte, V from its add; rising, N, Z and C from the add;
; falling, N, Z and C from comparing it with 7. X and Y kept.
RichterFallGravity:
    lda a:OBJ_VY_LO
    clc
    adc #$38
    sta a:OBJ_VY_LO
    lda a:OBJ_VY_HI
    adc #$00
    sta a:OBJ_VY_HI
    bmi RichterFallGravity_at0
    cmp #$07
    bcc RichterFallGravity_at0
    stz a:OBJ_VY_LO
RichterFallGravity_at0:
    rts

; @routine 01BA:62F RichterMoveStopped
; @run 6D:AE2F
; WHAT IT DOES: Moves him along X, stops his animation and sets the state's step back to 0.
; HOW: A added to OBJ_X_LO; OBJ_2F90 $FF; OBJ_2800 0.
; WAITS: no.
; Entry: A the move. Exit: A $FF, X the facing, N and Z from it, C and V from the add. Y kept.
RichterMoveStopped:
    clc
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    lda #$FF
    sta a:OBJ_2F90
    stz a:OBJ_2800
    ldx a:OBJ_2AC0
    rts

; @routine 01BA:73A RichterCrashFrame
; @run 6D:AF3A
; WHAT IT DOES: The frame an item crash shows: by which pair of crash states he is in, and by
; the attack's kind.
; HOW: RichterCrashFrames: states 5 and 6 at 0, 7 and 8 at 4, then PlayerByte3BD5.
; WAITS: no.
; Entry: nothing. Exit: A the frame, N and Z from it, X the entry, C and V from adding the kind.
; Y kept.
RichterCrashFrame:
    lda a:OBJ_27A8
    sec
    sbc #$05
    and #$FE
    asl a
    clc
    adc a:PlayerByte3BD5
    tax
    lda a:RichterCrashFrames,x
    rts

; @routine 01BB:180 RichterEndItemCrash
; @run 6D:B180
; WHAT IT DOES: Ends an item crash: no attack, the crash's last frame held, the animation stopped,
; the step back to 0.
; HOW: States 5 to 8 are the crashes; two of them share each entry of RichterCrashEndFrames and
; RichterCrashEndAnimations.
; WAITS: no.
; Entry: nothing. Exit: A $FF, N set, Z clear; Y the entry; C bit 0 of the state less 5, V from
; that subtract. X kept.
RichterEndItemCrash:
    stz a:PlayerByte3BD5
    lda a:OBJ_27A8
    sec
    sbc #$05
    lsr a
    tay
    lda a:RichterCrashEndFrames,y
    sta a:OBJ_2A68
    lda a:RichterCrashEndAnimations,y
    sta a:OBJ_FIELDS_2EE0
    lda #$FF
    sta a:OBJ_2F90
    stz a:OBJ_2800
    rts

; @routine 01BB:73D RichterInvincibleTick
; @run 6D:B73D
; WHAT IT DOES: Runs the invincibility a potion gives: while its timer runs, every fourth frame
; he is kept unhittable and the timer counts down, with a sound as it starts and another as it
; wears off, the flashing scripts at set marks, and his hit timer and flash back to 1 at the end.
; HOW: PlayerInvincibleTimer; zWaveCounter's low two bits pick the frame. The marks and their
; scripts are two tables of six. OBJ_3460 $80 holds hits off; OBJ_3828 2 while it runs.
; WAITS: no.
; Entry: nothing. Exit: off, A 0, Z set. Not this frame, A the frame's bits, N and Z from them.
; Otherwise X, Y, C and V as a sound command left them, C from comparing A with $10 when the
; second sound was dropped or not made. Ended, A 1, N and Z clear. A mark, as
; RichterStartHitScript with its script, C set. No mark, A the timer, Y 6, Z and C set, N clear.
RichterInvincibleTick:
    lda a:PlayerInvincibleTimer
    bne RichterInvincibleTick_at0
    rts
RichterInvincibleTick_at0:
    stz a:PlayerByte3BE8
    stz a:PlayerByte3BE9
    lda #$02
    sta a:OBJ_3828
    lda z:zWaveCounter
    and #$03
    beq RichterInvincibleTick_at1
    rts
RichterInvincibleTick_at1:
    lda #$80
    sta a:OBJ_3460
    lda a:PlayerInvincibleTimer
    cmp #$80
    bne RichterInvincibleTick_at2
    lda #$31
    jsr VecSoundCommand
RichterInvincibleTick_at2:
    cmp #$10
    bne RichterInvincibleTick_at3
    lda #$32
    jsr VecSoundCommand
RichterInvincibleTick_at3:
    dec a:PlayerInvincibleTimer
    beq RichterInvincibleTick_at4
    cly
RichterInvincibleTick_at6:
    lda a:PlayerInvincibleTimer
    cmp a:RichterInvincibleMarks,y
    beq RichterInvincibleTick_at5
    iny
    cpy #$06
    bne RichterInvincibleTick_at6
    rts
RichterInvincibleTick_at5:
    lda a:RichterInvincibleScripts,y
    jmp RichterStartHitScript
RichterInvincibleTick_at4:
    lda #$01
    sta a:OBJ_3460
    sta a:OBJ_3828
    rts

; @routine 01BD:023 RichterLeapClearingOthers
; @run 6D:C023
; WHAT IT DOES: Clears every object but him, starts sound list $60 and script 1, and script $0B
; unless PlayerByte3BE2 is set or bit 5 of his OBJ_33B0 is; then leaps up and back: Y speed
; $FE80, X speed by facing from RichterLeapSpeeds, animation $22, state 0, OBJ_2750 1.
; HOW: ClearObject on 1 to 15, StartListChannelKind2 on channel 6, RichterStartHitScript.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z from it; X 7; Y twice the facing, C the facing's bit 7; V
; bit 6 of OBJ_33B0 if it was tested, else kept.
RichterLeapClearingOthers:
    ldx #$01
RichterLeapClearingOthers_at0:
    jsr VecClearObject
    inx
    cpx #$10
    bne RichterLeapClearingOthers_at0
    lda #$60
    ldx #$06
    jsr VecStartListChannelKind2
    lda #$01
    jsr RichterStartHitScript
    lda a:PlayerByte3BE2
    bne RichterLeapClearingOthers_at1
    tst #$20,a:OBJ_33B0
    bne RichterLeapClearingOthers_at1
    lda #$0B
    jsr RichterStartHitScript
RichterLeapClearingOthers_at1:
    lda #$80
    sta a:OBJ_VY_LO
    lda #$FE
    sta a:OBJ_VY_HI
    lda #$22
    jsr RichterSetAnimation
    lda a:OBJ_2AC0
    asl a
    tay
    lda a:RichterLeapSpeeds,y
    sta a:OBJ_VX_LO
    lda a:RichterLeapSpeeds+1,y
    sta a:OBJ_VX_HI
    stz a:OBJ_27A8
    lda #$01
    sta a:OBJ_2750
    rts

; @routine 01BE:61D RichterClearOthersKeepWhip
; @run 6D:CE1D
; WHAT IT DOES: Clears every object but him, and the whip when its OBJ_2750 is 1; drops the
; forced presses and PlayerByte3BD8.
; HOW: ClearObject on 1 to 15, passing over 3 when it is kept.
; WAITS: no.
; Entry: nothing. Exit: A the whip's OBJ_2750; X $10; Z and C set, N clear. Y and V kept.
RichterClearOthersKeepWhip:
    ldx #$01
RichterClearOthersKeepWhip_at1:
    cpx #$03
    bne RichterClearOthersKeepWhip_at0
    lda a:OBJ_2750,x
    cmp #$01
    bne RichterClearOthersKeepWhip_at0
    inx
RichterClearOthersKeepWhip_at0:
    jsr VecClearObject
    inx
    cpx #$10
    bne RichterClearOthersKeepWhip_at1
    stz a:PlayerByte3BD8
    stz a:PadForcedPress
    stz a:PadForcedPressB
    rts
