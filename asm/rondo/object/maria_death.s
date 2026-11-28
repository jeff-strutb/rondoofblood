; @routine 01DD:00C MariaVecDeathLeap
; @run 6D:A00C
; WHAT IT DOES: Her overlay's vector to MariaDeathLeap.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaDeathLeap.
MariaVecDeathLeap:
    jmp MariaDeathLeap

; @routine 01DD:015 MariaVecClearObjectsBut
; @run 6D:A015
; WHAT IT DOES: Her overlay's vector to MariaClearObjectsBut.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaClearObjectsBut.
MariaVecClearObjectsBut:
    jmp MariaClearObjectsBut

; @routine 01E1:3A9 MariaDeathFrame
; @run 6E:C3A9
; WHAT IT DOES: A frame of her death. Step 0: the leap's timer, and at its end script 8 unless held; her animation and
; the knock's speed; falling, a floor her column stands on lifts her onto it, stops her, animation $25, step 1, and
; unless held script 2; else a bit-3 tile under column 0 starts the burst; gravity $0020 held at $07xx; off the bottom
; of the map (Y high 1), it is over (zField89 1). Step 1: the columns, the floor, a landing, and when her animation
; has stopped, the dissolve set up (OBJ_DISSOLVE_PHASE_K 0, OBJ_DISSOLVE_CLOCK_J $40) and step 2. Later steps,
; MariaStagePointFrame.
; HOW: OBJ_SUBSTATE; DEC of OBJ_ANIM_TIMER, TST of PlayerHeld, MariaStartHitScript; MariaAnimationTick, MariaKnockSpeedX;
; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn, MariaLiftByColumn, MariaStopMoving, MariaSetAnimation;
; MariaBit3Burst; the 16-bit add; MariaLandOnFloor, MariaAnimationRunning; the JMP to MariaStagePointFrame and a branch
; into its end.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in, or the last test.
MariaDeathFrame:
    lda a:OBJ_SUBSTATE
    beq MariaDeathFrame_to1
    dec a
    beq MariaDeathFrame_at0
    jmp MariaStagePointFrame
MariaDeathFrame_to1:
    lda a:OBJ_ANIM_TIMER
    beq MariaDeathFrame_at1
    dec a:OBJ_ANIM_TIMER
    bne MariaDeathFrame_at1
    tst #$FF,a:PlayerHeld
    bne MariaDeathFrame_at1
    lda #$08
    jsr MariaStartHitScript
MariaDeathFrame_at1:
    jsr MariaAnimationTick
    jsr MariaKnockSpeedX
    lda a:OBJ_VY_HI
    bmi MariaDeathFrame_at2
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaDeathFrame_at3
    jsr MariaSolidUnderColumn
    bne MariaDeathFrame_at3
    jsr MariaLiftByColumn
    jsr MariaStopMoving
    lda #$25
    jsr MariaSetAnimation
    inc a:OBJ_SUBSTATE
    tst #$FF,a:PlayerHeld
    bne MariaDeathFrame_at4
    lda #$02
    jmp MariaStartHitScript
MariaDeathFrame_at4:
    rts
MariaDeathFrame_at3:
    tst #$08,a:RichterColumnHit
    beq MariaDeathFrame_at2
    jsr MariaBit3Burst
MariaDeathFrame_at2:
    lda a:OBJ_VY_LO
    clc
    adc #$20
    sta a:OBJ_VY_LO
    lda a:OBJ_VY_HI
    adc #$00
    sta a:OBJ_VY_HI
    bmi MariaDeathFrame_at5
    cmp #$07
    bcc MariaDeathFrame_at5
    stz a:OBJ_VY_LO
MariaDeathFrame_at5:
    lda a:OBJ_Y_HI
    cmp #$01
    beq MariaDeathFrame_at6
    rts
MariaDeathFrame_at6:
    bra MariaStagePointFrame_at0
MariaDeathFrame_at0:
    jsr MariaReadColumns
    jsr MariaFloorCheck
    jsr MariaLandOnFloor
    jsr MariaAnimationRunning
    beq MariaDeathFrame_at7
    rts
MariaDeathFrame_at7:
    stz a:OBJ_WORK_K
    lda #$40
    sta a:OBJ_WORK_J
    inc a:OBJ_SUBSTATE
    rts

; @routine 01E1:484 MariaDeathLeap
; @run 6E:C484
; WHAT IT DOES: The leap her death starts with: the side she is knocked to, lifted three rows, Y speed $FCC0,
; animation $1E (plus OBJ_KNOCK_FORWARD), the death's sound, state 3 step 0, timer 6, script $0B, and held, script 1.
; HOW: MariaHurtSide, MariaLiftThree, MariaSetAnimation, MariaDeathSound, MariaStartHitScript; the tail JMP to it when
; held.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaStartHitScript with $0B, then TST of PlayerHeld (Z set, N and V its
; bits 7 and 6), or as MariaStartHitScript with 1.
MariaDeathLeap:
    jsr MariaHurtSide
    jsr MariaLiftThree
    lda #$C0
    sta a:OBJ_VY_LO
    lda #$FC
    sta a:OBJ_VY_HI
    lda #$1E
    clc
    adc a:OBJ_KNOCK_FORWARD
    jsr MariaSetAnimation
    jsr MariaDeathSound
    stz a:OBJ_SUBSTATE
    lda #$03
    sta a:OBJ_STATE
    lda #$06
    sta a:OBJ_ANIM_TIMER
    lda #$0B
    jsr MariaStartHitScript
    tst #$FF,a:PlayerHeld
    beq MariaDeathLeap_at0
    lda #$01
    jmp MariaStartHitScript
MariaDeathLeap_at0:
    rts
