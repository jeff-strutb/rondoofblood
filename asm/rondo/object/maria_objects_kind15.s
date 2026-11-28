; @routine 01E3:5FD MariaKind16
; @run 6E:D5FD
; WHAT IT DOES: The handler of kind 16: its state's routine from MariaKind16States (start, fly, wait, drift, circle).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
MariaKind16:
    jsr MariaStateDoubled
    jmp (MariaKind16States,x)

; @routine 01E3:60D MariaKind16Clear
; @run 6E:D60D
; WHAT IT DOES: Clears kind 16's object when she is hit before it starts.
; HOW: The tail JMP to ClearObject through the kernel's vector.
; WAITS: no.
; Entry: X the object. Exit: all registers and flags kept.
MariaKind16Clear:
    jmp VecClearObject

; @routine 01E3:610 MariaKind16Start
; @run 6E:D610
; WHAT IT DOES: Kind 16's state 0: cleared when she is hit; at her frame $51 it takes a hitbox $80 square, live,
; OBJ_WORK_N $0E, the place $FFC4, $A3 (left of the screen), sprite set 1, frame $28, its mark, speed $0300 at angle
; $24, timer $44, and its next state.
; HOW: Her OBJ_STATE and OBJ_FRAME; the stores, MariaObjectMarkOn; the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $51; else as the
; routine jumped to.
MariaKind16Start:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    beq MariaKind16Clear
    lda a:OBJ_FRAME
    cmp #$51
    bne MariaKind16Start_at0
    lda #$40
    sta a:OBJ_HITBOX_HALF_H,x
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$0E
    sta a:OBJ_WORK_N,x
    lda #$A3
    sta a:OBJ_Y_LO,x
    lda #$C4
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_X_HI,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$28
    sta a:OBJ_FRAME,x
    jsr MariaObjectMarkOn
    lda #$00
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda #$03
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda #$24
    sta a:OBJ_FLY_ANGLE,x
    lda #$44
    sta a:OBJ_HURT_TIMER,x
    jmp MariaObjectNextState
MariaKind16Start_at0:
    rts

; @routine 01E3:666 MariaKind16Fly
; @run 6E:D666
; WHAT IT DOES: Kind 16's state 1: flies along its angle, turning by two a frame; when its timer runs out it stops,
; the timer $10, and its next state.
; HOW: MariaObjectFlyAlong, two DECs of OBJ_FLY_ANGLE, DEC of the timer; MariaObjectStop, MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: N and Z from the timer, or as MariaObjectNextState.
MariaKind16Fly:
    ldx z:zRichterObject
    jsr MariaObjectFlyAlong
    dec a:OBJ_FLY_ANGLE,x
    dec a:OBJ_FLY_ANGLE,x
    dec a:OBJ_HURT_TIMER,x
    bne MariaKind16Fly_at0
    lda #$10
    sta a:OBJ_HURT_TIMER,x
    jsr MariaObjectStop
    jsr MariaObjectNextState
MariaKind16Fly_at0:
MariaKind16Fly_done:
    rts

; @routine 01E3:682 MariaKind16Wait
; @run 6E:D682
; WHAT IT DOES: Kind 16's state 2: waits out its timer; then the Y and X speeds of entries $0D and $0E, turned the
; way it faces, the timer 8, and its next state.
; HOW: DEC of the timer; MariaSetVelocityYFromTable, MariaSetVelocityXFromTable, MariaObjectFacingVelocity; the tail
; JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: N and Z from the timer, or as MariaObjectNextState.
MariaKind16Wait:
    ldx z:zRichterObject
    dec a:OBJ_HURT_TIMER,x
    bne MariaKind16Fly_done
    lda #$0D
    jsr MariaSetVelocityYFromTable
    lda #$0E
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    lda #$08
    sta a:OBJ_HURT_TIMER,x
    jmp MariaObjectNextState

; @routine 01E3:69E MariaKind16Drift
; @run 6E:D69E
; WHAT IT DOES: Kind 16's state 3: moves by its velocity; when its timer runs out, slot 6's kind becomes $16, its
; timer $D0, and it flies again slowly (MariaObjectFlyReset).
; HOW: MariaObjectApplyVelocity, DEC of the timer, the stores; on into MariaObjectFlyReset.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: N and Z from the timer, or as MariaObjectFlyReset.
MariaKind16Drift:
    ldx z:zRichterObject
    jsr MariaObjectApplyVelocity
    dec a:OBJ_HURT_TIMER,x
    beq MariaKind16Drift_at0
    rts
MariaKind16Drift_at0:
    lda #$16
    sta a:OBJ_KIND+6
    lda #$D0
    sta a:OBJ_HURT_TIMER,x

; @routine 01E3:6B3 MariaObjectFlyReset
; @run 6E:D6B3
; WHAT IT DOES: Stops an object and sets it flying slowly at angle 0 (speed $0040), then its next state.
; HOW: MariaObjectStop, the stores, the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: X the object. Exit: as MariaObjectNextState.
MariaObjectFlyReset:
    jsr MariaObjectStop
    lda #$40
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda #$00
    sta a:OBJ_SWOOP_SPEED_HI,x
    stz a:OBJ_FLY_ANGLE,x
    jmp MariaObjectNextState

; @routine 01E3:6C6 MariaObjectFlyAlong
; @run 6E:D6C6
; WHAT IT DOES: Moves an object along its angle at its speed: OBJ_SWOOP_SPEED_LO and _HI to zTemp00-01, OBJ_FLY_ANGLE
; to SetVelocityFromAngle, then its velocity applied.
; HOW: The loads and stores; SetVelocityFromAngle through the kernel's vector at $4048; the tail JMP to
; MariaObjectApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: as MariaObjectApplyVelocity.
MariaObjectFlyAlong:
    lda a:OBJ_SWOOP_SPEED_LO,x
    sta z:zTemp00
    lda a:OBJ_SWOOP_SPEED_HI,x
    sta z:zTemp01
    lda a:OBJ_FLY_ANGLE,x
    jsr VecSetVelocityFromAngle
    jmp MariaObjectApplyVelocity

; @routine 01E3:6D9 MariaKind16Circle
; @run 6E:D6D9
; WHAT IT DOES: Kind 16's state 4: flies along its angle, turning the other way by two a frame; at timer $30 its speed
; becomes $0340 at angle $38; every $20 frames script 2; when the timer runs out the object and the next slot are
; cleared.
; HOW: MariaObjectFlyAlong, INCs of OBJ_FLY_ANGLE, DEC of the timer, MariaStartScriptSlot3; ClearObject (the second by
; the tail JMP) through the kernel's vector.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step made.
MariaKind16Circle:
    ldx z:zRichterObject
    jsr MariaObjectFlyAlong
    lda a:OBJ_HURT_TIMER,x
    cmp #$30
    bne MariaKind16Circle_at0
    lda #$40
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda #$03
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda #$38
    sta a:OBJ_FLY_ANGLE,x
MariaKind16Circle_at0:
    inc a:OBJ_FLY_ANGLE,x
    inc a:OBJ_FLY_ANGLE,x
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_HURT_TIMER,x
    and #$1F
    cmp #$01
    bne MariaKind16Circle_at1
    lda #$02
    jsr MariaStartScriptSlot3
MariaKind16Circle_at1:
    lda a:OBJ_HURT_TIMER,x
    beq MariaKind16Circle_at2
    jmp MariaKind16Fly_done
MariaKind16Circle_at2:
    inx
    jsr VecClearObject
    dex
    jmp VecClearObject

; @routine 01E3:71B MariaKind22
; @run 6E:D71B
; WHAT IT DOES: The handler of kind 22. State 0: a hitbox $C0 square, live, OBJ_WORK_N $0D, sprite set 1, frame $29,
; its mark, the Y and X speeds of entries $0D and $0E turned the way it faces, and its next state. After: it moves,
; and its Y speed grows by $0018 a frame and is its X speed too; once off the screen sideways (X high not zero) sound
; $37 and its X and Y high bytes cleared.
; HOW: OBJ_STATE; the stores, MariaObjectMarkOn, the velocity tables, MariaObjectFacingVelocity, MariaObjectNextState;
; MariaObjectApplyVelocity, the 16-bit add (CLA and ADC for the high byte), SoundCommand.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step made.
MariaKind22:
    ldx z:zRichterObject
    lda a:OBJ_STATE,x
    bne MariaKind22_at0
    ldx z:zRichterObject
    lda #$60
    sta a:OBJ_HITBOX_HALF_H,x
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$0D
    sta a:OBJ_WORK_N,x
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$29
    sta a:OBJ_FRAME,x
    jsr MariaObjectMarkOn
    lda #$0D
    jsr MariaSetVelocityYFromTable
    lda #$0E
    jsr MariaSetVelocityXFromTable
    jsr MariaObjectFacingVelocity
    jmp MariaObjectNextState
MariaKind22_at0:
    jsr MariaObjectApplyVelocity
    lda #$18
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,x
    sta a:OBJ_VX_LO,x
    cla
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
    sta a:OBJ_VX_HI,x
    lda a:OBJ_X_HI,x
    beq MariaKind22_at1
    lda #$37
    jsr VecSoundCommand
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
MariaKind22_at1:
    rts

; @routine 01E3:77D MariaKind15
; @run 6E:D77D
; WHAT IT DOES: The handler of kind 15: its state's routine from MariaKind15States (start, fly, swoop, ride, spawn,
; rain).
; HOW: MariaStateDoubled, then the JMP (table,X).
; WAITS: no.
; Entry: zRichterObject the slot. Exit: the processor as the state's routine left it.
MariaKind15:
    jsr MariaStateDoubled
    jmp (MariaKind15States,x)

; @routine 01E3:78F MariaKind15Clear
; @run 6E:D78F
; WHAT IT DOES: Clears kind 15's object when she is hit before it starts.
; HOW: The tail JMP to ClearObject through the kernel's vector.
; WAITS: no.
; Entry: X the object. Exit: all registers and flags kept.
MariaKind15Clear:
    jmp VecClearObject

; @routine 01E3:792 MariaKind15Start
; @run 6E:D792
; WHAT IT DOES: Kind 15's state 0: cleared when she is hit; at her frame $51, from slot 5 slots 6 to 8 become kind 15
; too; each takes her facing, a hitbox $40 square, live, OBJ_WORK_N $0E, its place by slot (MariaKind15StartPlaces,
; or ...Left when it faces left) and speeds (MariaKind15StartSpeedsX and Y) turned the way it faces, sprite set 1,
; frame $26, timer $7F, and its next state.
; HOW: Her OBJ_STATE and OBJ_FRAME; the stores; the tables by slot less 5 doubled; MariaObjectFacingVelocity; the
; tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: waiting, A her frame, N, Z and C from comparing it with $51; else as the
; routine jumped to.
MariaKind15Start:
    ldx z:zRichterObject
    lda a:OBJ_STATE
    cmp #$09
    beq MariaKind15Clear
    lda a:OBJ_FRAME
    cmp #$51
    beq MariaKind15Start_at0
    rts
MariaKind15Start_at0:
    cpx #$05
    bne MariaKind15Start_at1
    lda #$0F
    sta a:OBJ_KIND+6
    sta a:OBJ_KIND+7
    sta a:OBJ_KIND+8
MariaKind15Start_at1:
    jsr MariaObjectFromPlayer+9
    lda #$20
    sta a:OBJ_HITBOX_HALF_H,x
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$0E
    sta a:OBJ_WORK_N,x
    txa
    sec
    sbc #$05
    asl a
    tay
    lda a:MariaKind15StartPlaces,y
    sta a:OBJ_X_LO,x
    lda a:MariaKind15StartPlaces+1,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_FACING_LEFT,x
    beq MariaKind15Start_at2
    lda a:MariaKind15StartPlacesLeft,y
    sta a:OBJ_X_LO,x
    lda a:MariaKind15StartPlacesLeft+1,y
    sta a:OBJ_Y_LO,x
MariaKind15Start_at2:
    lda a:MariaKind15StartSpeedsX,y
    sta a:OBJ_VX_LO,x
    lda a:MariaKind15StartSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:MariaKind15StartSpeedsY,y
    sta a:OBJ_VY_LO,x
    lda a:MariaKind15StartSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    jsr MariaObjectFacingVelocity
    lda #$01
    sta a:OBJ_SPRITE_SET,x
    lda #$26
    sta a:OBJ_FRAME,x
    lda #$7F
    sta a:OBJ_HURT_TIMER,x
    jmp MariaObjectNextState

; @routine 01E4:017 MariaKind15Fly
; @run 6E:D817
; WHAT IT DOES: Kind 15's state 1: moves and keeps its mark; every $20 frames script 1 and sound $46; when its timer
; runs out, its animation timer $40, animation $12, and it flies again slowly; outside slot 5 it then shows nothing and
; its hitbox is off. Its hitbox is $20 high, $30 wide.
; HOW: MariaObjectApplyVelocity, MariaObjectMarkOn, MariaStartScriptSlot3, SoundCommand; DEC of the timer;
; MariaObjectSetAnimation, MariaObjectFlyReset.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: A $18, N and Z from it; the rest as the last call left it.
MariaKind15Fly:
    ldx z:zRichterObject
    jsr MariaObjectApplyVelocity
    jsr MariaObjectMarkOn
    lda a:OBJ_HURT_TIMER,x
    and #$1F
    cmp #$01
    bne MariaKind15Fly_at0
    lda #$01
    jsr MariaStartScriptSlot3
    lda #$46
    jsr VecSoundCommand
MariaKind15Fly_at0:
    dec a:OBJ_HURT_TIMER,x
    bne MariaKind15Fly_at1
    lda #$40
    sta a:OBJ_ANIM_TIMER,x
    lda #$12
    jsr MariaObjectSetAnimation
    jsr MariaObjectFlyReset
    cpx #$05
    beq MariaKind15Fly_at1
    stz a:OBJ_FRAME,x
    stz a:OBJ_HITBOX_ON,x
MariaKind15Fly_at1:
    lda #$10
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$18
    sta a:OBJ_HITBOX_HALF_W,x
    rts

; @routine 01E4:059 MariaKind15Swoop
; @run 6E:D859
; WHAT IT DOES: Kind 15's state 2: slot 5 drops its mark and runs its animation; it flies along its angle, turning by
; two a frame; when its animation timer runs out, that timer is set by slot (slot less 5), it stops, X speed $08xx the
; way it faces, script 1, frame $27, no hitter, sound $1E, and its next state.
; HOW: MariaObjectMarkOff, MariaObjectAnimationTick, MariaObjectFlyAlong, DECs; MariaObjectStop,
; MariaObjectFacingVelocity, MariaStartScriptSlot3, SoundCommand, the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: N and Z from the animation timer, or as MariaObjectNextState.
MariaKind15Swoop:
    ldx z:zRichterObject
    cpx #$05
    bne MariaKind15Swoop_at0
    jsr MariaObjectMarkOff
    jsr MariaObjectAnimationTick
MariaKind15Swoop_at0:
    jsr MariaObjectFlyAlong
    dec a:OBJ_FLY_ANGLE,x
    dec a:OBJ_FLY_ANGLE,x
    dec a:OBJ_ANIM_TIMER,x
    beq MariaKind15Swoop_at1
    rts
MariaKind15Swoop_at1:
    txa
    sec
    sbc #$05
    sta a:OBJ_ANIM_TIMER,x
    jsr MariaObjectStop
    lda #$08
    sta a:OBJ_VX_HI,x
    jsr MariaObjectFacingVelocity
    lda #$01
    jsr MariaStartScriptSlot3
    lda #$27
    sta a:OBJ_FRAME,x
    stz a:OBJ_HIT_BY_SLOT,x
    lda #$1E
    jsr VecSoundCommand
    jmp MariaObjectNextState

; @routine 01E4:09B MariaKind15Ride
; @run 6E:D89B
; WHAT IT DOES: Kind 15's state 3: slot 5, once the object its hit found has a live hitbox (2 or more) and health not
; negative, waits 4 frames, stops, and goes to its next state. Else while its animation timer runs nothing; then it
; rides: moves, X speed up by $80 and the way it faces (MariaKind15RideSpeedsHi), steered by the pad, and cleared once
; past $40 on the right or under $C0 on the left of the next screen.
; HOW: OBJ_HIT_BY_SLOT, that object's OBJ_HITBOX_ON and OBJ_HEALTH; MariaObjectStop, MariaObjectNextState; DEC of the
; timer; MariaObjectMove, the 16-bit add, MariaObjectSteerByPad, ClearObject by the tail JMP.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the last step made.
MariaKind15Ride:
    ldx z:zRichterObject
    cpx #$05
    bne MariaKind15Ride_at0
    ldy a:OBJ_HIT_BY_SLOT,x
    lda a:OBJ_HITBOX_ON,y
    cmp #$02
    bcc MariaKind15Ride_at0
    lda a:OBJ_HEALTH,y
    bmi MariaKind15Ride_at0
    lda #$04
    sta a:OBJ_ANIM_TIMER,x
    jsr MariaObjectStop
    jmp MariaObjectNextState
MariaKind15Ride_at0:
    lda a:OBJ_ANIM_TIMER,x
    beq MariaKind15Ride_at1
    dec a:OBJ_ANIM_TIMER,x
    rts
MariaKind15Ride_at1:
    jsr MariaObjectMove
    ldy a:OBJ_FACING_LEFT,x
    lda #$80
    clc
    adc a:OBJ_VX_LO,x
    sta a:OBJ_VX_LO,x
    lda a:MariaKind15RideSpeedsHi,y
    adc a:OBJ_VX_HI,x
    sta a:OBJ_VX_HI,x
    jsr MariaObjectSteerByPad
    lda a:OBJ_X_HI,x
    beq MariaKind15Ride_at2
    lda a:OBJ_VX_HI,x
    bmi MariaKind15Ride_at3
    lda a:OBJ_X_LO,x
    cmp #$40
    bcc MariaKind15Ride_at2
MariaKind15Ride_at4:
    jmp VecClearObject
MariaKind15Ride_at3:
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc MariaKind15Ride_at4
MariaKind15Ride_at2:
    rts

; @routine 01E4:0FB MariaObjectSteerByPad
; @run 6E:D8FB
; WHAT IT DOES: With up or down held, the object's Y speed's high byte goes one less (up) or one more (down).
; HOW: zPadHeld's bits 4 and 6; the ADC.
; WAITS: no.
; Entry: X the object. Exit: A the AND or the new high byte, N and Z from it; C and V from the add. X and Y kept.
MariaObjectSteerByPad:
    lda z:zPadHeld
    and #$50
    beq MariaObjectSteerByPad_at0
    and #$10
    bne MariaObjectSteerByPad_at1
    lda #$01
    bra MariaObjectSteerByPad_at2
MariaObjectSteerByPad_at1:
    lda #$FF
MariaObjectSteerByPad_at2:
    clc
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
MariaObjectSteerByPad_at0:
    rts

; @routine 01E4:113 MariaKind15Spawn
; @run 6E:D913
; WHAT IT DOES: Kind 15's state 4: rides until its animation timer runs out; then slots 6 to 8 are emptied, the timer
; $80, its mark, it stops, and its next state.
; HOW: DEC of the timer, the JMP into MariaKind15Ride's ride; ClearObjectInY three times through the kernel's vector;
; MariaObjectMarkOn, MariaObjectStop, the tail JMP to MariaObjectNextState.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: as the ride, or MariaObjectNextState.
MariaKind15Spawn:
    ldx z:zRichterObject
    dec a:OBJ_ANIM_TIMER,x
    beq MariaKind15Spawn_at0
    jmp MariaKind15Ride_at1
MariaKind15Spawn_at0:
    ldy #$06
    jsr VecClearObjectInY
    iny
    jsr VecClearObjectInY
    iny
    jsr VecClearObjectInY
    lda #$80
    sta a:OBJ_ANIM_TIMER,x
    jsr MariaObjectMarkOn
    jsr MariaObjectStop
    jmp MariaObjectNextState

; @routine 01E4:138 MariaKind15Rain
; @run 6E:D938
; WHAT IT DOES: Kind 15's state 5: moves; every eighth frame, in the first empty slot of 6 to 15, a piece of kind 11 at
; its place plus MariaKind15RainPlaces by OBJ_RAIN_COUNT, with sound $15, and the count on; when its timer runs out it
; is cleared.
; HOW: MariaObjectMove; OBJ_KIND by Y; ClearObjectInY through the kernel's vector; the 16-bit adds of the place with X
; on the stack; SoundCommand; DEC of the timer, ClearObject by the tail JMP.
; WAITS: no.
; Entry: zRichterObject the slot. Exit: N and Z from the timer, or as ClearObject.
MariaKind15Rain:
    ldx z:zRichterObject
    jsr MariaObjectMove
    lda a:OBJ_ANIM_TIMER,x
    and #$07
    bne MariaKind15Rain_at0
    ldy #$06
MariaKind15Rain_at2:
    lda a:OBJ_KIND,y
    beq MariaKind15Rain_at1
    iny
    cpy #$10
    bne MariaKind15Rain_at2
MariaKind15Rain_at0:
    dec a:OBJ_ANIM_TIMER,x
    bne MariaKind15Rain_at3
    jmp VecClearObject
MariaKind15Rain_at3:
    rts
MariaKind15Rain_at1:
    jsr VecClearObjectInY
    lda #$0B
    sta a:OBJ_KIND,y
    phx
    lda a:OBJ_RAIN_COUNT,x
    and #$07
    asl a
    asl a
    tax
    lda a:MariaKind15RainPlaces,x
    sta a:OBJ_X_LO,y
    lda a:MariaKind15RainPlaces+1,x
    sta a:OBJ_X_HI,y
    lda a:MariaKind15RainPlaces+2,x
    sta a:OBJ_Y_LO,y
    lda a:MariaKind15RainPlaces+3,x
    sta a:OBJ_Y_HI,y
    plx
    lda a:OBJ_X_LO,x
    clc
    adc a:OBJ_X_LO,y
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI,x
    adc a:OBJ_X_HI,y
    sta a:OBJ_X_HI,y
    lda a:OBJ_Y_LO,x
    clc
    adc a:OBJ_Y_LO,y
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_HI,x
    adc a:OBJ_Y_HI,y
    sta a:OBJ_Y_HI,y
    lda #$15
    jsr VecSoundCommand
    inc a:OBJ_RAIN_COUNT,x
    bra MariaKind15Rain_at0
