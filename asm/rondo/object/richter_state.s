; @routine 01B9:000 RichterVecFrame
; @run 6D:A000
; WHAT IT DOES: Goes to RichterFrame.
; HOW: One jump.
; Entry, exit: as RichterFrame.
RichterVecFrame:
    jmp RichterFrame

; @routine 01B9:003 RichterVecStart
; @run 6D:A003
; WHAT IT DOES: Goes to RichterStart.
; HOW: One jump.
; Entry, exit: as RichterStart.
RichterVecStart:
    jmp RichterStart

; @routine 01B9:006 RichterVecApplyMove
; @run 6D:A006
; WHAT IT DOES: Goes to $BB23.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecApplyMove:
    jmp $BB23

; @routine 01B9:009 RichterVecC3A7
; @run 6D:A009
; WHAT IT DOES: Goes to $C3A7.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecC3A7:
    jmp $C3A7

; @routine 01B9:00C RichterVecC023
; @run 6D:A00C
; WHAT IT DOES: Goes to $C023.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecC023:
    jmp $C023

; @routine 01B9:00F RichterVecC076
; @run 6D:A00F
; WHAT IT DOES: Goes to $C076.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecC076:
    jmp $C076

; @routine 01B9:012 RichterVecAnimationSound
; @run 6D:A012
; WHAT IT DOES: Goes to RichterAnimationSound.
; HOW: One jump.
; Entry, exit: as RichterAnimationSound.
RichterVecAnimationSound:
    jmp RichterAnimationSound

; @routine 01B9:015 RichterVecCE1D
; @run 6D:A015
; WHAT IT DOES: Goes to $CE1D.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecCE1D:
    jmp $CE1D

; @routine 01B9:018 RichterVecSetAnimation
; @run 6D:A018
; WHAT IT DOES: Goes to $B67B.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecSetAnimation:
    jmp $B67B

; @routine 01B9:01B RichterVecSetIdle
; @run 6D:A01B
; WHAT IT DOES: Goes to $B669.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecSetIdle:
    jmp $B669

; @routine 01B9:01E RichterVecCE73
; @run 6D:A01E
; WHAT IT DOES: Goes to $CE73.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecCE73:
    jmp $CE73

; @routine 01B9:021 RichterVecWall
; @run 6D:A021
; WHAT IT DOES: Goes to $BB6E, the wall probe.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecWall:
    jmp $BB6E

; @routine 01B9:024 RichterVecFloor
; @run 6D:A024
; WHAT IT DOES: Goes to $BBC1, the floor probe.
; HOW: One jump.
; Entry, exit: as that code.
RichterVecFloor:
    jmp $BBC1

; @routine 01B9:027 RichterApplyMoveAndPushGoTo
; @run 6D:A027
; WHAT IT DOES: Goes to RichterApplyMoveAndPush.
; HOW: One jump.
; Entry, exit: as RichterApplyMoveAndPush.
RichterApplyMoveAndPushGoTo:
    jmp RichterApplyMoveAndPush

; @routine 01B9:02A RichterApplyMoveAndPush
; @run 6D:A02A
; WHAT IT DOES: Applies the move and, if nothing in three probes touches, takes the push
; from $CA into $3BC8 when the push flag says so. Called from outside what has been read
; of the overlay so far.
; HOW: Bits $81 of the three bytes at $3BC3 stop it.
; Entry: nothing. Exit: as the path taken leaves them.
RichterApplyMoveAndPush:
    jsr $BB23
    clx
RichterApplyMoveAndPush_at1:
    lda a:$3BC3,x
    and #$81
    bne RichterApplyMoveAndPush_at0
    inx
    cpx #$03
    bne RichterApplyMoveAndPush_at1
    tst #$01,z:zPlayerPush
    beq RichterApplyMoveAndPush_at0
    lda z:$CA
    sta a:$3BC8
    clx
    jsr $BA80
    stz z:$CA
RichterApplyMoveAndPush_at0:
    rts

; @routine 01B9:04B RichterFrame
; @run 6D:A04B
; WHAT IT DOES: One frame of Richter: the pad, forced while OBJ_3510 counts down; the
; pad's overrides; the state's handler; the animation's sound on a change; its hitbox.
; HOW: The animation before and after are compared; the hitbox index comes from the table
; by animation and is four bytes of AnimationHitbox.
; Entry: nothing. Exit: A the last hitbox byte, N and Z from it, X the hitbox offset; Y, C
; and V as the last code left them.
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
    jsr $B611
RichterFrame_at1:
    lda a:PadForcedPress
    ora z:zPadPressed
    ora a:PadForcedPressB
    sta z:zPadPressed
    lda a:OBJ_2A68
    pha
    jsr $B64C
    jsr RichterHitWhilePushed
    jsr RichterStateFrame
    jsr $B704
    jsr $B7E9
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
; Entry: nothing. Exit: as the last step left them.
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
; HOW: Each point's X from $B6F9 pulled one pixel in, Y from the table by state; the
; terrain from $B6D2.
; Entry: nothing. Exit: on finding one, A bit 1, N and Z from it, X the point. Else A
; zero, X 3, N, Z and C from comparing X with 3.
RichterSpikeBelow:
    clx
RichterSpikeBelow_at3:
    jsr $B6F9
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
    jsr $B6D2
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
; HOW: The table of four entries, each of which goes on into the next.
; Entry: nothing. Exit: as RichterAnimationSound.
RichterStart:
    lda z:zField8A
    asl a
    tax
    jmp (PlayerStartHandlers,x)

; @routine 01B9:11E RichterClearWork
; @run 6D:A11E
; WHAT IT DOES: Clears the player's work bytes and zero page.
; HOW: Two block moves from the kernel's zeros.
; Entry: nothing. Exit: all registers and flags kept.
RichterClearWork:
    tai KernelZeros,$3BC0,$0040
    tai KernelZeros,zPadHeld+$2000,$0019
    rts

; @routine 01B9:12D RichterClearKeepPlace
; @run 6D:A12D
; WHAT IT DOES: Clears object 0, the player, keeping where it is and which way it faces.
; HOW: The three bytes on the stack around ClearObject.
; Entry: nothing. Exit: A the X position, N and Z from it, X 0. Y, C, V as ClearObject.
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
; Entry: nothing. Exit: as RichterAnimationSound.
RichterStartNew:
    stz z:zField93
    lda #$10
    sta z:zField94

; @routine 01B9:150 RichterStartHealth
; @run 6D:A150
; WHAT IT DOES: Fills the health, then as RichterStartClear.
; HOW: Runs on into it.
; Entry: nothing. Exit: as RichterAnimationSound.
RichterStartHealth:
    lda #$5C
    sta z:zPlayerHealth

; @routine 01B9:154 RichterStartClear
; @run 6D:A154
; WHAT IT DOES: Clears the player's work and object, marks it live, gives a frame of the
; hit timer, then as RichterStartStand.
; HOW: Runs on into it.
; Entry: nothing. Exit: as RichterAnimationSound.
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
; Entry: nothing. Exit: as RichterAnimationSound.
RichterStartStand:
    lda #$01
    sta a:PlayerByte3BD6
    jsr RichterStand

; @routine 01B9:16F RichterAnimationSound
; @run 6D:A16F
; WHAT IT DOES: Starts the list that goes with the player's animation on its channel.
; HOW: The table by animation, then StartListChannelKind2 with X 7.
; Entry: nothing. Exit: as StartListChannelKind2, Y the animation.
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
; Entry: nothing. Exit: as the last step left them.
RichterDyingStep:
    jsr RichterStopX
    stz a:OBJ_2A68
    dec a:PlayerDyingTimer
    bne RichterDyingStep_at0
    ldx #$01
RichterDyingStep_at1:
    lda a:OBJ_2750,x
    bne RichterDyingStep_at0
    jsr VecClearObject
    inx
    cpx #$10
    bne RichterDyingStep_at1
    lda #$05
    sta z:zSubStep
    stz a:PlayerByte3BD7
    stz a:PlayerDying
    lda #$09
    jsr RichterStartHitScript
RichterDyingStep_at0:
    rts

; @routine 01B9:1A4 RichterDie
; @run 6D:A1A4
; WHAT IT DOES: Starts the player's death: no health, the timer, the state, the music out
; and the death's sound.
; HOW: FadeAndStopSound, then SoundCommand $60.
; Entry: nothing. Exit: as SoundCommand.
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
; hit timer; a hit in OBJ_33B0 taken off the health; then the state's handler.
; HOW: Y high 1 or $FF low is below the map. A hit of kind 5 is ignored unless
; PlayerMayLeaveTop is set. Health gone goes to RichterOutOfHealth.
; Entry: nothing. Exit: as the path taken left them.
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
    jsr $B73D
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
; HOW: SoundCommand $64 on the way out.
; Entry: nothing. Exit: as the path taken left them.
RichterOutOfHealth:
    stz z:zPlayerHealth
    lda a:PlayerByte3BF7
    cmp #$01
    bne RichterOutOfHealth_at0
    inc z:zPlayerHealth
    jmp RichterStateFrame_at5
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
; Entry: nothing. Exit: as RichterStartHitScript.
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
; Entry: A script. Exit: as StartScriptSlotKind1.
RichterStartHitScript:
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 01B9:2E4 RichterTurnOnPress
; @run 6D:A2E4
; WHAT IT DOES: Turns the player to face the way just pressed.
; HOW: OBJ_2AC0 is the facing, 0 right, 1 left.
; Entry: nothing. Exit: A the facing or 1, flags as the last test left them. X, Y, C kept.
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
; WHAT IT DOES: The standing state's attacking step: moves, falls if nothing is below;
; turns while the attack is young, and when it ends stands or crouches.
; HOW: OBJ_FLAGS_2F38 counts the attack's progress; object 3, the whip, ends it.
; Entry: nothing. Exit: as the path taken left them.
RichterStandStep1:
    jsr $BB23
    jsr $B885
    bcc RichterStandStep1_at0
    jmp $A9DD
RichterStandStep1_at0:
    jsr $BA7A
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
    jsr $B868
    bcs RichterStandStep1_at4
RichterStandStep1_at2:
    jsr $B659
    stz a:PlayerByte3BD5
    jmp RichterStand
RichterStandStep1_at4:
    lda #$06
    sta a:OBJ_FIELDS_2EE0
    jsr $B669
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
; Entry: nothing. Exit: as ClearObject, X 3.
RichterClearWhip:
    ldx #$03
    jmp VecClearObject

; @routine 01B9:351 RichterOrphanClearWhipAir
; @run 6D:A351
; WHAT IT DOES: Clears the whip and takes to the air. Nothing reached calls it.
; HOW: RichterClearWhip, RichterToAir.
; Entry: nothing. Exit: as RichterToAir.
RichterOrphanClearWhipAir:
    bsr RichterClearWhip
    jmp RichterToAir

; @routine 01B9:358 RichterStandStep2
; @run 6D:A358
; WHAT IT DOES: The standing state's item-crash step: turns with the pad, moves while its
; timer runs and spawns object $0C in the first free slot from 5; when it ends, stands or
; falls.
; HOW: OBJ_30F0 is the timer. Coming off a floor or jumping sets PlayerByte3BD6.
; Entry: nothing. Exit: as the path taken left them.
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
    jsr $BB23
    jsr $B885
    bcs RichterStandStep2_at4
RichterStandStep2_at3:
    jsr $B87E
    bcs RichterStandStep2_at5
    jsr $B669
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
    jmp $B180
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
; WHAT IT DOES: Gives the state's step as a table offset.
; HOW: OBJ_2800 doubled, in A and X.
; Entry: nothing. Exit: A and X the offset, N and Z from it, C the step's bit 7. Y, V kept.
RichterStepIndex:
    lda a:OBJ_2800
    asl a
    tax
    rts

; @routine 01B9:3D6 RichterStateStand
; @run 6D:A3D6
; WHAT IT DOES: State 0: runs the step's handler.
; HOW: A table of three by RichterStepIndex.
; Entry: nothing. Exit: as the handler leaves them.
RichterStateStand:
    bsr RichterStepIndex
    jmp (PlayerStandHandlers,x)

; @routine 01B9:3E1 RichterStandStep0
; @run 6D:A3E1
; WHAT IT DOES: Standing: moves, then the first thing that applies: nothing below is a
; fall; a jump; a hit in the air; an attack; a climb; a crouch; down looks down, and left
; or right start walking.
; HOW: Each check is a routine leaving its answer in C.
; Entry: nothing. Exit: as the path taken left them.
RichterStandStep0:
    jsr $BB23
    jsr $B885
    bcc RichterStandStep0_at0
    jmp RichterToFall
RichterStandStep0_at0:
    jsr $B87E
    bcc RichterStandStep0_at1
    jmp RichterToAir
RichterStandStep0_at1:
    jsr $BA7A
    jsr $B7FC
    bcc RichterStandStep0_at2
    jmp RichterToHurtAir
RichterStandStep0_at2:
    jsr $B929
    bcc RichterStandStep0_at3
    jmp RichterAttack
RichterStandStep0_at3:
    jsr $BA41
    bcc RichterStandStep0_at4
    jmp $A7C4
RichterStandStep0_at4:
    jsr $B868
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
    jsr $B669
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
; Entry: Y the facing. Exit: as the animation's code, or on an attack held as the test.
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
    jmp $B67B
RichterStartWalk_at2:
    lda #$32
    jmp $B67B

; @routine 01B9:46C RichterSpeedWalk
; @run 6D:A46C
; WHAT IT DOES: Sets the X speed to walking, by facing.
; HOW: WalkSpeeds.
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
; Entry: nothing. Exit: as the animation's code left them.
RichterCrouch:
    jsr RichterPadOverridesOff
    lda #$02
    sta a:OBJ_27A8
    lda #$03
    sta a:OBJ_2800
    lda #$0C
    jsr $B67B

; @routine 01B9:4C0 RichterStopMoving
; @run 6D:A4C0
; WHAT IT DOES: Stops the player along Y and X.
; HOW: Runs on into RichterStopX.
; Entry: nothing. Exit: all registers and flags kept.
RichterStopMoving:
    stz a:OBJ_VY_LO
    stz a:OBJ_VY_HI

; @routine 01B9:4C6 RichterStopX
; @run 6D:A4C6
; WHAT IT DOES: Stops the player along X.
; HOW: Zeroes the speed.
; Entry: nothing. Exit: all registers and flags kept.
RichterStopX:
    stz a:OBJ_VX_LO
    stz a:OBJ_VX_HI
    rts

; @routine 01B9:4CD RichterAttack
; @run 6D:A4CD
; WHAT IT DOES: Starts an attack standing: state 0, step 1, the animation by the attack's
; kind; the whip, with its sound, or the kind's object in a free slot among 5, 8 and $0B.
; HOW: PlayerByte3BD5 is the kind. zField93 is what goes in the slot.
; Entry: nothing. Exit: as the path taken left them.
RichterAttack:
    jsr RichterPadOverridesOff
    stz a:PlayerByte3BE1
    jsr $B7CA
    lda #$00
    sta a:OBJ_27A8
    jsr RichterStopMoving
    ldx a:PlayerByte3BD5
    lda a:LandAnimations,x
    jsr $B67B
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
RichterAttack_at3:
    jsr VecClearObject
    sta a:OBJ_2750,x
    rts
RichterAttack_at1:
    tst #$FF,a:OBJ_2750+8
    bne RichterAttack_at2
    ldx #$08
    bra RichterAttack_at3
RichterAttack_at2:
    ldx #$0B
    bra RichterAttack_at3

; @routine 01B9:521 RichterToFall
; @run 6D:A521
; WHAT IT DOES: Starts falling: state 4, animation $10, Y speed 3, no X speed.
; HOW: Runs on into RichterPadOverridesOff.
; Entry: nothing. Exit: A as the animation's code left it, flags too.
RichterToFall:
    lda #$04
    sta a:OBJ_27A8
    lda #$00
    sta a:OBJ_2800
    lda #$10
    jsr $B67B
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
; Entry: nothing. Exit: as the path taken left them.
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
; Entry: nothing. Exit: as the path taken left them.
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
    jsr $B67B
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
; WHAT IT DOES: Jumps up off a ledge the first way; with button $40 held and $B893's
; answer, drops off it instead.
; HOW: Reads the pad; clears PadForcedPressB and zFieldC6.
; Entry: nothing. Exit: as the path taken left them.
RichterUpWhipLeft:
    jsr $B64C
    stz a:PadForcedPressB
    stz z:zFieldC6
    tst #$40,z:zPadHeld
    beq RichterUpWhipLeft_at0
    jsr $B893
    bcs RichterUpWhipLeft_at1
    rts
RichterUpWhipLeft_at1:
    ldx a:OBJ_2AC0
    lda a:LedgeNudge,x
    bra RichterUpWhipRight_at0
RichterUpWhipLeft_at0:
    bsr RichterNudgeA
RichterUpWhipLeft_at2:
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
; Entry: nothing. Exit: as RichterNudgeA.
RichterNudgeB:
    ldx a:OBJ_2AC0
    lda a:LedgeNudgeB,x
    bra RichterNudgeA_at0

; @routine 01B9:5C4 RichterNudgeBJump
; @run 6D:A5C4
; WHAT IT DOES: The second nudge, and the jump up off the ledge.
; HOW: RichterNudgeB, then as RichterUpWhipLeft's jump.
; Entry: nothing. Exit: as the jump left them.
RichterNudgeBJump:
    bsr RichterNudgeB
    bra RichterUpWhipLeft_at2

; @routine 01B9:5C8 RichterUpWhipRight
; @run 6D:A5C8
; WHAT IT DOES: As RichterUpWhipLeft by the second table.
; HOW: The same tests.
; Entry: nothing. Exit: as the path taken left them.
RichterUpWhipRight:
    jsr $B64C
    stz a:PadForcedPressB
    stz z:zFieldC6
    tst #$40,z:zPadHeld
    beq RichterNudgeBJump
    jsr $B893
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
; HOW: Table by zField93.
; Entry: nothing. Exit: as the reaction left them.
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
    jsr $B67B
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
; Entry: nothing. Exit: A $0B, Y $10, N, Z, C from comparing it.
RichterReactFillB:
    lda #$0B
    bra RichterReactFillD_at1

; @routine 01B9:644 RichterItemCrashFive
; @run 6D:A644
; WHAT IT DOES: Item $05's crash, from a hurt jump: no speed, animation $2A, its sound,
; state 0, step 2, its timer.
; HOW: The fields in turn.
; Entry: nothing. Exit: A $60, N and Z from it; C and V as SoundCommand left them.
RichterItemCrashFive:
    jsr RichterStopMoving
    lda #$2A
    jsr $B67B
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
; Entry: nothing. Exit: A $10, N and Z from it. X, Y, C, V kept.
RichterReact10:
    lda #$10
    bra RichterReactE_at0

; @routine 01B9:67B RichterReact12
; @run 6D:A67B
; WHAT IT DOES: Puts object $12 in slot 5.
; HOW: Goes back into RichterReactE's store.
; Entry: nothing. Exit: A $12, N and Z from it. X, Y, C, V kept.
RichterReact12:
    lda #$12
    bra RichterReactE_at0

; @routine 01B9:67F RichterReact13
; @run 6D:A67F
; WHAT IT DOES: Puts object $13 in slot 5.
; HOW: Goes back into RichterReactE's store.
; Entry: nothing. Exit: A $13, N and Z from it. X, Y, C, V kept.
RichterReact13:
    lda #$13
    bra RichterReactE_at0

; @routine 01B9:683 RichterKnockBack
; @run 6D:A683
; WHAT IT DOES: Knocked into the air: state 3, animation $31, the knock's speeds, marks
; PlayerByte3BF1 bit 0.
; HOW: The jump's speed replaced by the knock's.
; Entry: nothing. Exit: A PlayerByte3BF1, N and Z from it; C and V as the last left them.
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
    jsr $B67B
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
; HOW: Y is the facing the pad asks for, tried and put back.
; Entry: nothing. Exit: as the animation's code, or A $80 and its flags.
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
    jmp $B67B
RichterToAir_at0:
    lda a:OBJ_2AC0
    pha
    sty a:OBJ_2AC0
    lda #$02
    jsr $B67B
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
    jmp $B67B

; @routine 01B9:70C RichterJumpSpeed
; @run 6D:A70C
; WHAT IT DOES: Sets a jump's Y speed, stopped along X, and lifts the player off the floor.
; HOW: Speed $FB50; runs on into RichterLiftThree.
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
; WHAT IT DOES: State 1, walking: moves, a pixel down onto a slope, falls if nothing is
; below; then the checks of standing; then keeps walking while the pad says, walking
; forward or, with the jump held, backward; else stands.
; HOW: $BAC2's C says there is a floor. The direction held must be the facing's.
; Entry: nothing. Exit: as the path taken left them.
RichterStateWalk:
    jsr $BB23
    jsr $BAC2
    bcs RichterStateWalk_at0
    inc a:OBJ_Y_LO
    jsr $BB23
    jsr $BAC2
    bcs RichterStateWalk_at0
    jmp RichterToFall
RichterStateWalk_at0:
    jsr $B87E
    bcc RichterStateWalk_at1
    jmp RichterToAir
RichterStateWalk_at1:
    jsr $BA7A
    jsr $B7FC
    bcc RichterStateWalk_at2
    jmp RichterToHurtAir
RichterStateWalk_at2:
    jsr $B929
    bcc RichterStateWalk_at3
    jmp RichterAttack
RichterStateWalk_at3:
    jsr $BA41
    bcc RichterStateWalk_at4
    jmp $A7C4
RichterStateWalk_at4:
    jsr $B868
    bcc RichterStateWalk_at5
    jmp RichterCrouch
RichterStateWalk_at5:
    jsr $B669
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
; Entry: nothing. Exit: A and flags as the animation's code left them.
RichterStand:
    stz a:PlayerMayLeaveTop
    stz a:PlayerByte3BD5
    lda #$00
    sta a:OBJ_27A8
    lda #$00
    sta a:OBJ_2800
    lda #$01
    jsr $B67B
    jmp RichterStopMoving
