; @routine 01DD:000 MariaVecFrame
; @run 6D:A000
; WHAT IT DOES: Her overlay's first vector: goes to MariaFrame.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaFrame.
MariaVecFrame:
    jmp MariaFrame

; @routine 01DD:050 MariaFrame
; @run 6D:A050
; WHAT IT DOES: One frame of Maria: the pad, forced from OBJ_WORK_F while OBJ_WORK_E counts down, else read (with
; PlayerStartPose cleared); the forced presses ORed in; the pad's overrides cleared; the attack delay counted down; her
; pad sequence; the spike check while pushed; the state frame; the turn window; the item's hearts; the animation's
; sound when her animation changed; her hitbox from her tables by animation.
; HOW: OBJ_FRAME kept on the stack across the frame's calls; MariaAnimationHitboxIndex by animation, four bytes of
; MariaAnimationHitbox.
; WAITS: no.
; Entry: X and Y the caller's. Exit: A the hitbox's last byte, N and Z from it, X the hitbox's offset, C bit 6 of the
; index; Y and V as the last call left them.
MariaFrame:
    lda a:OBJ_FLYER_WAIT
    beq MariaFrame_at0
    dec a:OBJ_FLYER_WAIT
    lda z:zPadHeld
    sta z:zPadPrevious
    lda a:OBJ_CHILD_ORDER
    sta z:zPadHeld
    sta z:zPadPressed
    bra MariaFrame_at1
MariaFrame_at0:
    stz a:PlayerStartPose
    jsr MariaReadPad
MariaFrame_at1:
    lda a:PadForcedPress
    ora z:zPadPressed
    ora a:PadForcedPressB
    sta z:zPadPressed
    lda a:OBJ_FRAME
    pha
    jsr MariaClearPadOverrides
    lda a:OBJ_EVENT_ID
    beq MariaFrame_at2
    dec a:OBJ_EVENT_ID
MariaFrame_at2:
    jsr MariaPadSequence
    jsr MariaHitWhilePushed
    jsr MariaStateFrame
    jsr MariaCheckTurnWindow
    jsr MariaCheckItemHearts
    pla
    eor a:OBJ_FRAME
    beq MariaFrame_at3
    jsr MariaAnimationSound
MariaFrame_at3:
    ldx a:OBJ_FRAME
    lda a:MariaAnimationHitboxIndex,x
    asl a
    asl a
    tax
    lda a:MariaAnimationHitbox,x
    sta a:OBJ_HITBOX_DX
    lda a:MariaAnimationHitbox+1,x
    sta a:OBJ_HITBOX_DY
    lda a:MariaAnimationHitbox+2,x
    sta a:OBJ_HITBOX_HALF_W
    lda a:MariaAnimationHitbox+3,x
    sta a:OBJ_HITBOX_HALF_H
    rts

; @routine 01DD:1E9 MariaStateFrame
; @run 6D:A1E9
; WHAT IT DOES: A frame of her state. Dying, the death's wait. Off the bottom of the map (Y high 1, or Y low $FF on
; the screen above), she dies. Then the invincibility; while the hit timer runs no hit is taken, and when it ends with
; no hold or slowing, script 9; a hit: of kind 5 with her not above the map, or of kind 3 or more in the item crash
; or item 4's state, it is dropped; else its damage is half again more, paid from her health: none left, out of
; health; else MariaHitTaken. Then the hold and the state's handler.
; HOW: PlayerDying, her Y; MariaInvincibleTick; OBJ_ANIM_TIMER, PlayerHeld, PlayerSlowTimer; OBJ_HIT_TAKEN's kind,
; OBJ_ANIMATION and OBJ_STATE; OBJ_HEALTH (the hit's damage) with LSR and ADC; zPlayerHealth's SBC; branches and JMPs to
; MariaDyingStep, MariaDie, MariaOutOfHealth, MariaHitTaken, MariaHeldFrame and MariaStateDispatch.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in.
MariaStateFrame:
    lda a:PlayerDying
    bne MariaDyingStep
    lda a:OBJ_Y_HI
    bmi MariaStateFrame_at0
    dec a
    beq MariaDie
    lda a:OBJ_Y_LO
    cmp #$FF
    bcs MariaDie
MariaStateFrame_at0:
    jsr MariaInvincibleTick
    lda a:OBJ_ANIM_TIMER
    beq MariaStateFrame_at1
    stz a:OBJ_HIT_TAKEN
    dec a:OBJ_ANIM_TIMER
    beq MariaStateFrame_to1
    jmp MariaHeldFrame
MariaStateFrame_to1:
    lda a:PlayerHeld
    ora a:PlayerSlowTimerLo
    ora a:PlayerSlowTimerHi
    bne MariaStateFrame_at1
    lda #$09
    jsr MariaStartHitScript
MariaStateFrame_at1:
    lda a:OBJ_HIT_TAKEN
    beq MariaHeldFrame
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$05
    bne MariaStateFrame_at2
    lda a:PlayerMayLeaveTop
    bne MariaStateFrame_at2
    stz a:OBJ_HIT_TAKEN
    jmp MariaStateDispatch
MariaStateFrame_at2:
    lda a:OBJ_WORK_A
    cmp #$2A
    beq MariaStateFrame_at3
    cmp #$2B
    beq MariaStateFrame_at3
    lda a:OBJ_STATE
    cmp #$10
    bne MariaStateFrame_at4
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$03
    bcs MariaHitTaken_react
MariaStateFrame_at5:
    stz a:OBJ_HIT_TAKEN
    jmp MariaStateDispatch
MariaStateFrame_at3:
    lda a:OBJ_HIT_TAKEN
    and #$07
    cmp #$03
    bcs MariaStateFrame_at4
    bra MariaStateFrame_at5
MariaStateFrame_at4:
    lda a:OBJ_HEALTH
    lsr a
    clc
    adc a:OBJ_HEALTH
    sta a:OBJ_HEALTH
    lda z:zPlayerHealth
    sec
    sbc a:OBJ_HEALTH
    sta z:zPlayerHealth
    bcs MariaStateFrame_at6
    jmp MariaOutOfHealth
MariaStateFrame_at6:
    bne MariaHitTaken
    jmp MariaOutOfHealth

; @routine 01DD:282 MariaHitTaken
; @run 6D:A282
; WHAT IT DOES: A hit paid for in health: the hit timer $30; held, the held hit's script 1 and the hit cleared; else
; her hit reaction. Then as MariaHeldFrame.
; HOW: PlayerHeld; MariaStartHitScript or MariaHitReact; a branch, or a run on, into MariaHeldFrame.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHeldFrame.
MariaHitTaken:
    lda #$30
    sta a:OBJ_ANIM_TIMER
    lda a:PlayerHeld
    beq MariaHitTaken_at0
    lda #$01
    jsr MariaStartHitScript
    stz a:OBJ_HIT_TAKEN
    bra MariaHeldFrame
MariaHitTaken_at0:
MariaHitTaken_react:
    jsr MariaHitReact

; @routine 01DD:299 MariaHeldFrame
; @run 6D:A299
; WHAT IT DOES: The hold on her (PlayerHeld), outside the hit's state: its first frame starts script 0 and counts
; PlayerHeldCount to 1; each frame a button is newly pressed counts it on, and at 8 the hold ends (script 9) and the
; state's handler runs; else the frame ends here. Not held, the state's handler.
; HOW: PlayerHeld, OBJ_STATE, PlayerHeldCount, zPadPressed; MariaStartHitScript; on into MariaStateDispatch.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaStateDispatch; held, A the count or the press, N and Z from it.
MariaHeldFrame:
    lda a:PlayerHeld
    beq MariaStateDispatch
    lda a:OBJ_STATE
    cmp #$09
    beq MariaStateDispatch
    lda a:PlayerHeldCount
    bne MariaHeldFrame_at0
    cla
    jsr MariaStartHitScript
    inc a:PlayerHeldCount
MariaHeldFrame_at0:
    lda z:zPadPressed
    beq MariaHeldFrame_at1
    inc a:PlayerHeldCount
    lda a:PlayerHeldCount
    cmp #$08
    beq MariaHeldFrame_at2
MariaHeldFrame_at1:
    rts
MariaHeldFrame_at2:
    stz a:PlayerHeld
    lda #$09
    jsr MariaStartHitScript

; @routine 01DD:2C8 MariaStateDispatch
; @run 6D:A2C8
; WHAT IT DOES: Runs her state's handler (MariaStateHandlers); below the map (Y high $FF) the pad's buttons are kept
; and its directions dropped first.
; HOW: The pad bytes ANDed with $F0; OBJ_STATE doubled, the JMP (table,X).
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the state's handler, after A and X the state doubled and C its bit 7.
MariaStateDispatch:
    lda a:OBJ_Y_HI
    cmp #$FF
    bne MariaStateDispatch_at0
    lda z:zPadHeld
    and #$F0
    sta z:zPadHeld
    lda z:zPadPressed
    and #$F0
    sta z:zPadPressed
MariaStateDispatch_at0:
    lda a:OBJ_STATE
    asl a
    tax
    jmp (MariaStateHandlers,x)

; @routine 01DD:307 MariaOutOfHealth
; @run 6D:A307
; WHAT IT DOES: No health left: with PlayerKeepsLastPoint 1 she keeps one point and the hit is paid as
; MariaHitTaken; else, held by a hit of bit 5 PlayerHeld is set, and the life is lost: zSubStep 4, the door's gate and
; the top cleared, her hitbox off, sound $68.
; HOW: The stores; the JMP to MariaHitTaken; the tail JMP to SoundCommand.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaHitTaken, or SoundCommand with $68.
MariaOutOfHealth:
    stz z:zPlayerHealth
    lda a:PlayerKeepsLastPoint
    cmp #$01
    bne MariaOutOfHealth_at0
    inc z:zPlayerHealth
    jmp MariaHitTaken
MariaOutOfHealth_at0:
    tst #$20,a:OBJ_HIT_TAKEN
    beq MariaOutOfHealth_at1
    lda #$01
    sta a:PlayerHeld
MariaOutOfHealth_at1:
    stz z:zPlayerHealth
    lda #$04
    sta z:zSubStep
    stz a:PlayerDoorReady
    stz a:PlayerMayLeaveTop
    stz a:OBJ_HITBOX_ON
    lda #$68
    jmp VecSoundCommand
