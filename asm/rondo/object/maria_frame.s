; @routine 01DD:003 MariaVecStart
; @run 6D:A003
; WHAT IT DOES: Her overlay's second vector: goes to MariaStart.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaStart.
MariaVecStart:
    jmp MariaStart

; @routine 01DD:012 MariaVecAnimationSound
; @run 6D:A012
; WHAT IT DOES: Goes to MariaAnimationSound.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaAnimationSound.
MariaVecAnimationSound:
    jmp MariaAnimationSound

; @routine 01DD:018 MariaVecSetAnimation
; @run 6D:A018
; WHAT IT DOES: Goes to MariaSetAnimation.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaSetAnimation.
MariaVecSetAnimation:
    jmp MariaSetAnimation

; @routine 01DD:01B MariaVecAnimationTick
; @run 6D:A01B
; WHAT IT DOES: Goes to MariaAnimationTick.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaAnimationTick.
MariaVecAnimationTick:
    jmp MariaAnimationTick

; @routine 01DD:01E MariaVecObjectApplyVelocity
; @run 6D:A01E
; WHAT IT DOES: Goes to MariaObjectApplyVelocity.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaObjectApplyVelocity.
MariaVecObjectApplyVelocity:
    jmp MariaObjectApplyVelocity

; @routine 01DD:027 MariaVecApplyMoveAndPush
; @run 6D:A027
; WHAT IT DOES: Goes to MariaApplyMoveAndPush.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaApplyMoveAndPush.
MariaVecApplyMoveAndPush:
    jmp MariaApplyMoveAndPush

; @routine 01DD:0BE MariaHitWhilePushed
; @run 6D:A0BE
; WHAT IT DOES: While she is pushed (bit 0 of zPlayerPush), outside camera mode $18, with no hit timer and not
; above the map, a spike under her is a hit of kind 6 with timer $10; the state goes back to standing, and a state
; $10 is ended first by MariaClearSlot5AsKind9.
; HOW: MariaSpikeBelow, then the hit's fields.
; WAITS: no.
; Entry: nothing. Exit: camera mode $18, A $18, Z and C set. Not pushed, A the camera mode, Z set, N and V the push's
; bits 7 and 6, C from comparing the mode with $18. Held, A OBJ_ANIM_TIMER or PlayerMayLeaveTop, N and Z from it, C as
; before, V the push's bit 6. No spike, as MariaSpikeBelow. A spike: A the state, N, Z and C from comparing it with
; $10, X, Y and V as MariaSpikeBelow left them; from state $10, as MariaClearSlot5AsKind9.
MariaHitWhilePushed:
    lda a:CameraMode
    cmp #$18
    beq MariaHitWhilePushed_at0
    tst #$01,z:zPlayerPush
    beq MariaHitWhilePushed_at0
    lda a:OBJ_ANIM_TIMER
    ora a:PlayerMayLeaveTop
    bne MariaHitWhilePushed_at0
    jsr MariaSpikeBelow
    beq MariaHitWhilePushed_at0
    lda #$06
    sta a:OBJ_HIT_TAKEN
    lda #$10
    sta a:OBJ_HEALTH
    lda a:OBJ_STATE
    cmp #$10
    bne MariaHitWhilePushed_at1
    jsr MariaClearSlot5AsKind9
MariaHitWhilePushed_at1:
    stz a:OBJ_STATE
MariaHitWhilePushed_at0:
    rts

; @routine 01DD:0EF MariaSpikeBelow
; @run 6D:A0EF
; WHAT IT DOES: Probes three points at her feet for a tile with bit 1 set (a spike).
; HOW: Each point's X from MariaColumnOffset pulled one pixel in toward her, its Y from MariaFootYByState by state;
; the terrain from MariaReadAbove.
; WAITS: no.
; Entry: nothing. Exit: Y, C and V as the last read left them (Y the state when the row was off the map). Found, A 2,
; N and Z from it, X the point. Else A 0, X 3, Z and C set, N clear.
MariaSpikeBelow:
    clx
MariaSpikeBelow_at2:
    jsr MariaColumnOffset
    beq MariaSpikeBelow_at0
    bmi MariaSpikeBelow_at3
    dec a
    bra MariaSpikeBelow_at0
MariaSpikeBelow_at3:
    inc a
MariaSpikeBelow_at0:
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    ldy a:OBJ_STATE
    clc
    adc a:MariaFootYByState,y
    jsr MariaReadAbove
    and #$02
    bne MariaSpikeBelow_at1
    inx
    cpx #$03
    bne MariaSpikeBelow_at2
MariaSpikeBelow_at1:
    rts

; @routine 01DD:12A MariaStart
; @run 6D:A12A
; WHAT IT DOES: Starts Maria as zField8A says: a new run, the health, the stand or the clear; each goes on into the
; next, so all end standing with her animation's list started.
; HOW: zField8A doubled is the index into MariaStartHandlers; the JMP (table,X) reads the word there.
; WAITS: no.
; Entry: Y the caller's (MariaClearWork's block move saves it). Exit: as MariaStartStand. A word in the table that is
; not one of the four goes to the routine at it with A and X the index, N and Z from it, C bit 7 of zField8A.
MariaStart:
    lda z:zField8A
    asl a
    tax
    jmp (MariaStartHandlers,x)

; @routine 01DD:139 MariaStartNew
; @run 6D:A139
; WHAT IT DOES: Starts a new run: clears zField93 and sets zField94 to $10, then as MariaStartHealth.
; HOW: Runs on into it.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaStartStand.
MariaStartNew:
    stz z:zField93
    lda #$10
    sta z:zField94

; @routine 01DD:13F MariaStartHealth
; @run 6D:A13F
; WHAT IT DOES: Fills her health ($5C), then as MariaStartClear.
; HOW: Runs on into it.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaStartStand.
MariaStartHealth:
    lda #$5C
    sta z:zPlayerHealth

; @routine 01DD:143 MariaStartClear
; @run 6D:A143
; WHAT IT DOES: Clears the player's work and her object (keeping her place and facing), makes her hitbox live, sets
; OBJ_WORK_N and gives her a frame of the hit timer; then as MariaStartStand.
; HOW: MariaClearWork, MariaClearKeepPlace, three stores of 1, and on into MariaStartStand.
; WAITS: no.
; Entry: A, X and Y as they stand (MariaClearWork's block move pushes and pulls them). Exit: as MariaStartStand.
MariaStartClear:
    jsr MariaClearWork
    jsr MariaClearKeepPlace
    lda #$01
    sta a:OBJ_HITBOX_ON
    lda #$01
    sta a:OBJ_WORK_N
    lda #$01
    sta a:OBJ_ANIM_TIMER

; @routine 01DD:158 MariaStartStand
; @run 6D:A158
; WHAT IT DOES: Marks the item's first use to come, stands her, and runs on into MariaAnimationSound.
; HOW: MariaStand, then the sound's instructions.
; WAITS: no.
; Entry: nothing. Exit: as MariaAnimationSound; C, and V where it sets it, as MariaSetAnimation left them.
MariaStartStand:
    lda #$01
    sta a:PlayerItemFirstUse
    jsr MariaStand

; @routine 01DD:160 MariaAnimationSound
; @run 6D:A160
; WHAT IT DOES: Starts the list that goes with her animation on the player's list channel.
; HOW: MariaAnimationSounds by OBJ_FRAME, then StartListChannelKind2 with X 7 (the tail JMP through the kernel's
; vector at $400F).
; WAITS: no.
; Entry: nothing. Exit: A 2, N 0, Z 0, X 7, Y the animation. C and V kept.
MariaAnimationSound:
    ldy a:OBJ_FRAME
    lda a:MariaAnimationSounds,y
    ldx #$07
    jmp VecStartListChannelKind2

; @routine 01DD:197 MariaDyingStep
; @run 6D:A197
; WHAT IT DOES: The death's wait, from her state frame: holds her still at the top of the map (Y $00FF) with no
; animation; when PlayerDyingTimer runs out, clears the objects from slot 1 up, and if one is still out sets the
; timer back to one frame; with all of them gone, starts script 9 and ends the stage's step (zSubStep 5).
; HOW: OBJ_KIND not zero is an object out. Entered by the state frame's branch.
; WAITS: no.
; Entry: nothing. Exit: waiting, A $FF, N and Z from the timer. An object out, A 1, N and Z from it, X its slot, C
; clear past slot 1 (kept at slot 1). Over, A 5, N and Z from it, X 7, Y 9, C set.
MariaDyingStep:
    jsr MariaStopX
    lda #$FF
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_FRAME
    dec a:PlayerDyingTimer
    bne MariaDyingStep_at0
    ldx #$01
MariaDyingStep_at2:
    lda a:OBJ_KIND,x
    bne MariaDyingStep_at1
    jsr VecClearObject
    inx
    cpx #$10
    bne MariaDyingStep_at2
    lda #$09
    jsr MariaStartHitScript
    lda #$05
    sta z:zSubStep
    stz a:PlayerByte3BD7
    stz a:PlayerDying
MariaDyingStep_at0:
    rts
MariaDyingStep_at1:
    lda #$01
    sta a:PlayerDyingTimer
    rts

; @routine 01DD:1CF MariaDie
; @run 6D:A1CF
; WHAT IT DOES: Starts her death: no health, PlayerDying counted up, its timer $40, the dying state (9), no X speed,
; then the death's sound.
; HOW: Entered by the state frame's branch when she has fallen off the bottom of the map; MariaStopX, then on into
; MariaDeathSound.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaDeathSound.
MariaDie:
    stz z:zPlayerHealth
    inc a:PlayerDying
    lda #$40
    sta a:PlayerDyingTimer
    lda #$09
    sta a:OBJ_STATE
    jsr MariaStopX

; @routine 01DD:1E1 MariaDeathSound
; @run 6D:A1E1
; WHAT IT DOES: Fades the music out and plays the death's sound.
; HOW: FadeAndStopSound, then the tail JMP to SoundCommand $60 through the kernel's vector at $401B.
; WAITS: no.
; Entry: X and Y the caller's (FadeAndStopSound leaves them; SoundCommand is given them). Exit: as SoundCommand
; with $60.
MariaDeathSound:
    jsr VecFadeAndStopSound
    lda #$60
    jmp VecSoundCommand

; @routine 01DD:334 MariaHitReact
; @run 6D:A334
; WHAT IT DOES: Her reaction to a hit: the pad's forced presses off, the hit timer by the hit's kind (MariaHitTimers),
; the state and step she was in kept in OBJ_WORK_G and OBJ_WORK_H, state 9 step 0, and the hit's script 8.
; HOW: The stores, then on into MariaStartHitScript.
; WAITS: no.
; Entry: nothing. Exit: as MariaStartHitScript with 8.
MariaHitReact:
    jsr MariaPadForcesOff
    lda a:OBJ_HIT_TAKEN
    and #$07
    tax
    lda a:MariaHitTimers,x
    sta a:OBJ_ANIM_TIMER
    lda a:OBJ_STATE
    sta a:OBJ_WORK_G
    lda a:OBJ_SUBSTATE
    sta a:OBJ_WORK_H
    lda #$09
    sta a:OBJ_STATE
    stz a:OBJ_SUBSTATE
    lda #$08

; @routine 01DE:040 MariaStand
; @run 6D:A840
; WHAT IT DOES: Stands her: may not leave the top, state 0, step 0, animation 1, all speed gone.
; HOW: MariaSetAnimation, then the tail JMP to MariaStopMoving.
; WAITS: no.
; Entry: nothing. Exit: as MariaSetAnimation; X and V kept.
MariaStand:
    stz a:PlayerMayLeaveTop
    lda #$00
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    lda #$01
    jsr MariaSetAnimation
    jmp MariaStopMoving

; @routine 01DE:055 MariaCrouch
; @run 6D:A855
; WHAT IT DOES: Crouches her: the pad's forced presses off, state 2, step 3, animation $0E, all speed gone.
; HOW: MariaPadForcesOff, MariaSetAnimation, and on into MariaStopMoving.
; WAITS: no.
; Entry: nothing. Exit: as MariaSetAnimation; X and V kept.
MariaCrouch:
    jsr MariaPadForcesOff
    lda #$02
    sta a:OBJ_STATE
    lda #$03
    sta a:OBJ_SUBSTATE
    lda #$0E
    jsr MariaSetAnimation

; @routine 01DE:067 MariaStopMoving
; @run 6D:A867
; WHAT IT DOES: Stops her along Y and X.
; HOW: Zeroes the Y speed and runs on into MariaStopX.
; WAITS: no.
; Entry: nothing. Exit: all registers and flags kept.
MariaStopMoving:
    stz a:OBJ_VY_LO
    stz a:OBJ_VY_HI

; @routine 01DE:06D MariaStopX
; @run 6D:A86D
; WHAT IT DOES: Stops her along X.
; HOW: Zeroes the X speed.
; WAITS: no.
; Entry: nothing. Exit: all registers and flags kept.
MariaStopX:
    stz a:OBJ_VX_LO
    stz a:OBJ_VX_HI
    rts

; @routine 01DE:074 MariaToFall
; @run 6D:A874
; WHAT IT DOES: Drops her: state 4, step 0, animation $0D, Y speed $0200 down, OBJ_WORK_D 4, no X speed, and the pad's
; forced presses off.
; HOW: MariaSetAnimation, the stores, MariaStopX, and on into MariaPadForcesOff.
; WAITS: no.
; Entry: nothing. Exit: A 4, N and Z from it; C, and Y and V where it sets them, as MariaSetAnimation left them; X kept.
MariaToFall:
    lda #$04
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    lda #$0D
    jsr MariaSetAnimation
    lda #$00
    sta a:OBJ_VY_LO
    lda #$02
    sta a:OBJ_VY_HI
    lda #$04
    sta a:OBJ_WORK_D
    jsr MariaStopX

; @routine 01E0:0B2 MariaClearSlot5AsKind9
; @run 6D:B8B2
; WHAT IT DOES: Empties object slot 5 and gives it kind 9, marks the item's first use to come, and plays sound $19.
; HOW: ClearObject on slot 5, the stores, then the tail JMP to SoundCommand through the kernel's vector at $401B.
; WAITS: no.
; Entry: Y the caller's (SoundCommand is given it). Exit: as SoundCommand with $19; X 5 given to it.
MariaClearSlot5AsKind9:
    ldx #$05
    jsr VecClearObject
    lda #$01
    sta a:PlayerItemFirstUse
    lda #$09
    sta a:OBJ_KIND,x
    lda #$19
    jmp VecSoundCommand
