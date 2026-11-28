; @routine 01DF:103 MariaStateSlideHeld
; @run 6D:B103
; WHAT IT DOES: State $0D, sliding with the direction held: with footing at her Y she slides on; with none there,
; footing one row lower takes her off the floor (MariaToSlideAir), and so does none one row higher either; footing
; one row higher keeps her sliding there. Then a landing, a ledge to take hold of; from the slide's fifth
; animation step on, while MariaUpHeld holds it, the slide slows and may slide again; else when her animation has
; run, she crouches.
; HOW: MariaReadColumns and MariaHasNoFooting at three heights (INC and DEC of her Y); MariaLandOnFloor,
; MariaLedgeCheck and the JMP to MariaClimbStart; OBJ_ANIMATION_AT compared with 5; MariaUpHeld, MariaSlideSlow,
; MariaSlideCheck and the JMP to MariaSlideStart; MariaAnimationWait, which ends the step while it runs; on into
; MariaSlideEnd.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in.
MariaStateSlideHeld:
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcc MariaStateSlideHeld_at0
    inc a:OBJ_Y_LO
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcc MariaToSlideAir
    dec a:OBJ_Y_LO
    dec a:OBJ_Y_LO
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcs MariaToSlideAir
MariaStateSlideHeld_at0:
    jsr MariaLandOnFloor
    jsr MariaLedgeCheck
    bcc MariaStateSlideHeld_to1
    jmp MariaClimbStart
MariaStateSlideHeld_to1:
    lda a:OBJ_SCROLL_BAND
    cmp #$05
    bcc MariaStateSlideHeld_to2
    jsr MariaUpHeld
    bcc MariaSlideEnd
    lda a:OBJ_SCROLL_BAND
    cmp #$05
    bcc MariaStateSlideHeld_to2
    jsr MariaSlideSlow
    jsr MariaSlideCheck
    bcc MariaStateSlideHeld_to2
    jmp MariaSlideStart
MariaStateSlideHeld_to2:
    jsr MariaAnimationWait

; @routine 01DF:150 MariaSlideEnd
; @run 6D:B150
; WHAT IT DOES: Ends her slide in the crouch: no speed, frame $1B, state 2, step 0.
; HOW: MariaStopMoving, the stores.
; WAITS: no.
; Entry: nothing. Exit: A 0, Z set, N clear. X, Y, C and V kept.
MariaSlideEnd:
    jsr MariaStopMoving
    lda #$1B
    sta a:OBJ_FRAME
    lda #$02
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    rts

; @routine 01DF:163 MariaToSlideAir
; @run 6D:B163
; WHAT IT DOES: Her slide goes off the floor: the X speed the way she faces (MariaSlideAirSpeeds), state $0E, step 0.
; HOW: The table by the facing doubled; the stores.
; WAITS: no.
; Entry: nothing. Exit: A 0, Z set, N clear; Y the facing doubled; C bit 7 of the facing. X and V kept.
MariaToSlideAir:
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:MariaSlideAirSpeeds,y
    sta a:OBJ_VX_LO
    lda a:MariaSlideAirSpeeds+1,y
    sta a:OBJ_VX_HI
    lda #$0E
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    rts

; @routine 01DF:17F MariaSlideSlow
; @run 6D:B17F
; WHAT IT DOES: Slows her slide: while she has X speed, the friction the way she faces is added (MariaSlideFriction);
; when the speed's sign turns over, she stops along X.
; HOW: A 16-bit add; the high byte's sign before (in zTemp00) and after; MariaStopX.
; WAITS: no.
; Entry: nothing. Exit: no speed, A 0, Z set; else A the sign bits that changed, N and Z from them, Y the facing
; doubled, C and V from the add. X kept.
MariaSlideSlow:
    lda a:OBJ_VX_HI
    ora a:OBJ_VX_LO
    beq MariaSlideSlow_to1
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:OBJ_VX_LO
    clc
    adc a:MariaSlideFriction,y
    sta a:OBJ_VX_LO
    lda a:OBJ_VX_HI
    sta z:zTemp00
    adc a:MariaSlideFriction+1,y
    sta a:OBJ_VX_HI
    eor z:zTemp00
    and #$80
    beq MariaSlideSlow_to1
    jsr MariaStopX
MariaSlideSlow_to1:
    rts

; @routine 01DF:1B5 MariaStateSlide
; @run 6D:B1B5
; WHAT IT DOES: State $0F. Step 0, the slide let go: the mark under her feet noted, then as the held slide; if that
; crouched her with a mark under her feet, she crawls (animation $18, state $0F, step 1). Step 1, the crawl: footing
; at her Y or a row lower, else off the floor; the mark noted, her animation; off the mark the crawl ends in the
; crouch, on it she stops, and with right or left held crawls that way ($0080 or $FF80).
; HOW: MariaFeetMarked, MariaStateSlideHeld, PlayerFeetOnMark; MariaReadColumns and MariaHasNoFooting, MariaAnimationTick,
; MariaStopX, TST of zPadHeld, MariaCrawlSpeeds; the JMPs to MariaToSlideAir and MariaSlideEnd.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in, or the last test or store.
MariaStateSlide:
    lda a:OBJ_SUBSTATE
    bne MariaStateSlide_at0
    jsr MariaFeetMarked
    jsr MariaStateSlideHeld
    lda a:OBJ_STATE
    cmp #$02
    bne MariaStateSlide_at1
    lda a:PlayerFeetOnMark
    beq MariaStateSlide_to1
    lda #$18
    jsr MariaSetAnimation
    lda #$0F
    sta a:OBJ_STATE
    lda #$01
    sta a:OBJ_SUBSTATE
MariaStateSlide_at1:
    rts
MariaStateSlide_to1:
    rts
MariaStateSlide_at0:
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcc MariaStateSlide_at2
    inc a:OBJ_Y_LO
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcc MariaStateSlide_at2
    jmp MariaToSlideAir
MariaStateSlide_at2:
    jsr MariaFeetMarked
    jsr MariaAnimationTick
    lda a:PlayerFeetOnMark
    beq MariaStateSlide_at3
    jsr MariaStopX
    tst #$A0,z:zPadHeld
    beq MariaStateSlide_to2
    cly
    tst #$20,z:zPadHeld
    bne MariaStateSlide_to3
    iny
    iny
MariaStateSlide_to3:
    lda a:MariaCrawlSpeeds,y
    sta a:OBJ_VX_LO
    lda a:MariaCrawlSpeeds+1,y
    sta a:OBJ_VX_HI
MariaStateSlide_to2:
    rts
MariaStateSlide_at3:
    jmp MariaSlideEnd

; @routine 01DF:222 MariaStateSlideAir
; @run 6D:B222
; WHAT IT DOES: State $0E, a slide gone off the floor: a floor her column stands on lands her, and slow enough, with
; button I and down held, she slides on from it (her Y up by the column's row); the marked-floor landings, the bit-3
; burst; gravity $0038, held at $07xx; the jump pressed is her second jump; her animation.
; HOW: MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn's Z; the JMPs to MariaSlideStart and MariaJumpLand;
; MariaOnMarkedFloor, MariaFeetOnMarkedFloor, MariaCheckBit3Under; the 16-bit add; MariaJumpPressed and the JMP to
; MariaSecondJump; the tail JMP to MariaAnimationTick.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in.
MariaStateSlideAir:
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaStateSlideAir_at0
    jsr MariaSolidUnderColumn
    beq MariaStateSlideAir_at1
MariaStateSlideAir_at0:
    jsr MariaOnMarkedFloor
    bcc MariaStateSlideAir_to1
    jmp MariaLandOnMarked
MariaStateSlideAir_to1:
    jsr MariaFeetOnMarkedFloor
    bcc MariaStateSlideAir_at2
    jmp MariaLandOnMarkedFeet
MariaStateSlideAir_at2:
    jsr MariaCheckBit3Under
    lda a:OBJ_VY_LO
    clc
    adc #$38
    sta a:OBJ_VY_LO
    lda a:OBJ_VY_HI
    adc #$00
    sta a:OBJ_VY_HI
    bmi MariaStateSlideAir_at3
    cmp #$07
    bcc MariaStateSlideAir_at3
    stz a:OBJ_VY_LO
MariaStateSlideAir_at3:
    jsr MariaJumpPressed
    bcc MariaStateSlideAir_to2
    jmp MariaSecondJump
MariaStateSlideAir_to2:
    jmp MariaAnimationTick
MariaStateSlideAir_at1:
    lda a:OBJ_VY_HI
    cmp #$04
    bcs MariaStateSlideAir_to3
    tst #$01,z:zPadHeld
    beq MariaStateSlideAir_to3
    tst #$40,z:zPadHeld
    beq MariaStateSlideAir_to3
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    jmp MariaSlideStart
MariaStateSlideAir_to3:
    jmp MariaJumpLand
