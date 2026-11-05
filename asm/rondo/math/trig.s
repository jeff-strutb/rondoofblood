; @routine 0045:640 SetVelocityFromAngle
; @run 69:5640
; WHAT IT DOES: Points an object along an angle at a given speed: its X
; velocity becomes speed times cosine, its Y velocity speed times sine.
; HOW: ScaleByCosine and ScaleBySine in turn, taking the top two bytes of each
; product as an 8.8 velocity.
; Entry: A angle, X object, zTemp00-01 speed. Exit: A the Y velocity's low
; byte, N and Z from it, C and V from the sine, Y 0. X kept. Clobbers zTemp10-12,
; zProduct0-2, zMultiplierLo-Hi.
SetVelocityFromAngle:
    pha
    jsr ScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:zProduct1
    sta a:OBJ_VX_LO,x
    pla
    jsr ScaleBySine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:zProduct1
    sta a:OBJ_VY_LO,x
    rts

; @routine 0045:65D ScaleByCosine
; @run 69:565D
; WHAT IT DOES: Scales the value in zTemp00 by the cosine of an angle.
; HOW: A quarter turn added to the angle, then ScaleBySine, which it runs into.
; Entry: A angle, zTemp00-01 value. Exit: as ScaleBySine; V from the angle's
; add when no later add set it.
ScaleByCosine:
    clc
    adc #$40

; @routine 0045:660 ScaleBySine
; @run 69:5660
; WHAT IT DOES: Scales the value in zTemp00 by the sine of an angle: 256 steps
; to the turn, angle 64 the peak, angles from 128 up negative.
; HOW: Doubles the angle for a word index: bit 7 goes out as the sign, the
; rest indexes the half wave in SineTable. Multiplies, and for the negative
; half takes the two's complement of the 24-bit product.
; Entry: A angle, zTemp00-01 value. Exit: zProduct0-2 result; A, N, Z, C and V
; from the multiply, or from the complement's last add when negative; Y 0.
; X kept. Clobbers zTemp10-12, zMultiplierLo-Hi.
ScaleBySine:
    asl a
    bcs ScaleBySine_at0
    tay
    lda a:SineTable,y
    sta z:zMultiplierLo
    lda a:SineTable+1,y
    sta z:zMultiplierHi
    jsr MultiplyBySineWord
    rts
ScaleBySine_at0:
    tay
    lda a:SineTable,y
    sta z:zMultiplierLo
    lda a:SineTable+1,y
    sta z:zMultiplierHi
    jsr MultiplyBySineWord
    lda z:zProduct0
    eor #$FF
    clc
    adc #$01
    sta z:zProduct0
    lda z:zProduct1
    eor #$FF
    adc #$00
    sta z:zProduct1
    lda z:zProduct2
    eor #$FF
    adc #$00
    sta z:zProduct2
    rts

; @routine 0045:69A MultiplyBySineWord
; @run 69:569A
; WHAT IT DOES: Multiplies the 16-bit value in zTemp00 by the 9-bit word in
; zMultiplierLo, for a 24-bit product in zProduct0-2; the word is a sine, so
; the product is the value scaled by that sine as 8.8.
; HOW: Shift and add, nine rounds: the multiplier shifts right through RAM, a
; 24-bit copy of the value in zTemp10-12 shifts left, and each bit shifted out
; set adds the copy to the product.
; Entry: zTemp00-01 value, zMultiplierLo-Hi word. Exit: A the last byte
; loaded (the top of the product if any bit was set, else zTemp01), Y 0, N 0,
; Z 1, C the top bit shifted out of zTemp12, V from the last add if any.
; X kept. Clobbers zTemp10-12, zMultiplierLo-Hi.
MultiplyBySineWord:
    stz z:zProduct0
    stz z:zProduct1
    stz z:zProduct2
    lda z:zTemp00
    sta z:zTemp10
    lda z:zTemp01
    sta z:zTemp11
    stz z:zTemp12
    ldy #$09
MultiplyBySineWord_at1:
    lsr z:zMultiplierHi
    ror z:zMultiplierLo
    bcc MultiplyBySineWord_at0
    lda z:zProduct0
    clc
    adc z:zTemp10
    sta z:zProduct0
    lda z:zProduct1
    adc z:zTemp11
    sta z:zProduct1
    lda z:zProduct2
    adc z:zTemp12
    sta z:zProduct2
MultiplyBySineWord_at0:
    asl z:zTemp10
    rol z:zTemp11
    rol z:zTemp12
    dey
    bne MultiplyBySineWord_at1
    rts

; @routine 0045:7CF AngleToPoint
; @run 69:57CF
; WHAT IT DOES: Gives the direction from an object to a point.
; HOW: Subtracts the object's position from the point, then AngleFromDelta.
; Entry: X object, zTemp08-09 point X, zTemp0A-0B point Y. Exit: as
; AngleFromDelta. Clobbers zTemp00-03.
AngleToPoint:
    lda z:zTemp08
    sec
    sbc a:OBJ_X_LO,x
    sta z:zTemp00
    lda z:zTemp09
    sbc a:OBJ_X_HI,x
    sta z:zTemp01
    lda z:zTemp0A
    sec
    sbc a:OBJ_Y_LO,x
    sta z:zTemp02
    lda z:zTemp0B
    sbc a:OBJ_Y_HI,x
    sta z:zTemp03
    bra AngleFromDelta

; @routine 0045:7EF AngleToPlayerOffset
; @run 69:57EF
; WHAT IT DOES: Gives the direction from an object to a point offset from the
; player, object 0.
; HOW: Subtracts the object's position from the player's, adds the offset, then
; AngleFromDelta.
; Entry: X object, zTemp08-09 offset X, zTemp0A-0B offset Y. Exit: as
; AngleFromDelta. Clobbers zTemp00-03.
AngleToPlayerOffset:
    lda a:OBJ_X_LO
    sec
    sbc a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:OBJ_X_HI
    sbc a:OBJ_X_HI,x
    sta z:zTemp01
    lda a:OBJ_Y_LO
    sec
    sbc a:OBJ_Y_LO,x
    sta z:zTemp02
    lda a:OBJ_Y_HI
    sbc a:OBJ_Y_HI,x
    sta z:zTemp03
    lda z:zTemp00
    clc
    adc z:zTemp08
    sta z:zTemp00
    lda z:zTemp01
    adc z:zTemp09
    sta z:zTemp01
    lda z:zTemp02
    clc
    adc z:zTemp0A
    sta z:zTemp02
    lda z:zTemp03
    adc z:zTemp0B
    sta z:zTemp03
    bra AngleFromDelta

; @routine 0046:02D AngleToPlayer
; @run 69:582D
; WHAT IT DOES: Gives the direction from an object to the player, object 0.
; HOW: Subtracts the object's position from the player's, then AngleFromDelta.
; Entry: X object. Exit: as AngleFromDelta. Clobbers zTemp00-03.
AngleToPlayer:
    lda a:OBJ_X_LO
    sec
    sbc a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:OBJ_X_HI
    sbc a:OBJ_X_HI,x
    sta z:zTemp01
    lda a:OBJ_Y_LO
    sec
    sbc a:OBJ_Y_LO,x
    sta z:zTemp02
    lda a:OBJ_Y_HI
    sbc a:OBJ_Y_HI,x
    sta z:zTemp03

; @routine 0046:04F AngleFromDelta
; @run 69:584F
; WHAT IT DOES: Gives the direction of a 16-bit vector as an angle, 256 steps
; to the turn.
; HOW: By the signs of Y then X, makes both axes positive in RAM, reads the
; first-quadrant angle and folds it into its real quadrant: X negative and Y
; positive mirrors about a quarter turn, both negative adds a half turn, Y
; negative alone negates.
; Entry: zTemp00-01 delta X, zTemp02-03 delta Y. Exit: A the angle, Y the
; table index, N and Z from the last step, C and V as it left them. X kept.
; Clobbers zTemp00-03.
AngleFromDelta:
    lda z:zTemp03
    bmi AngleFromDelta_at0
    lda z:zTemp01
    bmi AngleFromDelta_at1
    bra AngleFromDelta_at2
AngleFromDelta_at1:
    lda z:zTemp00
    eor #$FF
    clc
    adc #$01
    sta z:zTemp00
    lda z:zTemp01
    eor #$FF
    adc #$00
    sta z:zTemp01
    bsr AngleFromDelta_at2
    sec
    sbc #$80
    eor #$FF
    inc a
    inc a
    rts
AngleFromDelta_at0:
    lda z:zTemp01
    bpl AngleFromDelta_at3
    lda z:zTemp00
    eor #$FF
    clc
    adc #$01
    sta z:zTemp00
    lda z:zTemp01
    eor #$FF
    adc #$00
    sta z:zTemp01
    lda z:zTemp02
    eor #$FF
    clc
    adc #$01
    sta z:zTemp02
    lda z:zTemp03
    eor #$FF
    adc #$00
    sta z:zTemp03
    bsr AngleFromDelta_at2
    clc
    adc #$80
    rts
AngleFromDelta_at3:
    lda z:zTemp02
    eor #$FF
    clc
    adc #$01
    sta z:zTemp02
    lda z:zTemp03
    eor #$FF
    adc #$00
    sta z:zTemp03
    bsr AngleFromDelta_at2
    eor #$FF
    inc a
    inc a
    rts
AngleFromDelta_at2:
    lda z:zTemp01
    ora z:zTemp03
    beq AngleFromDelta_at4
AngleFromDelta_at5:
    lsr z:zTemp01
    ror z:zTemp00
    lsr z:zTemp03
    ror z:zTemp02
    lsr a
    bne AngleFromDelta_at5
AngleFromDelta_at4:
    lda z:zTemp00
    ora z:zTemp02
    bit #$F0
    beq AngleFromDelta_at6
AngleFromDelta_at7:
    lsr z:zTemp00
    lsr z:zTemp02
    lsr a
    bit #$F0
    bne AngleFromDelta_at7
AngleFromDelta_at6:
    lda z:zTemp02
    asl a
    asl a
    asl a
    asl a
    ora z:zTemp00
    tay
    lda a:AtanTable,y
    rts

; @routine 0046:29A NextRandom
; @run 69:5A9A
; WHAT IT DOES: Steps the game's random numbers and returns the next byte. The
; frame interrupt also steps it once a frame, so it depends on time as well as
; on calls.
; HOW: A 32-bit counter generator: the state becomes state * $01010101 + 1,
; each byte added into those above it, top byte first, then the 32-bit
; increment. The top byte is the result.
; Entry: nothing. Exit: A zRandom3, N and Z from it, C and V from the last add.
; X and Y kept.
NextRandom:
    lda z:zRandom3
    clc
    adc z:zRandom2
    sta z:zRandom3
    lda z:zRandom2
    clc
    adc z:zRandom1
    sta z:zRandom2
    lda z:zRandom3
    adc z:zRandom1
    sta z:zRandom3
    lda z:zRandom1
    clc
    adc z:zRandom0
    sta z:zRandom1
    lda z:zRandom2
    adc z:zRandom0
    sta z:zRandom2
    lda z:zRandom3
    adc z:zRandom0
    sta z:zRandom3
    inc z:zRandom0
    bne NextRandom_at0
    inc z:zRandom1
    bne NextRandom_at0
    inc z:zRandom2
    bne NextRandom_at0
    inc z:zRandom3
NextRandom_at0:
    lda z:zRandom3
    rts

; @routine 0046:2D2 TurnAngleToward
; @run 69:5AD2
; WHAT IT DOES: Turns an angle one step toward a target, the short way round.
; HOW: Equal angles stay. Otherwise compares them in the half of the circle
; the current angle lies in, a half turn deciding which way is shorter, and
; adds or subtracts the step. A step can overshoot the target: nothing here
; clamps it.
; Entry: zTemp00 step, zTemp01 current, zTemp02 target. Exit: A the new angle,
; N and Z from it, C and V from the step; on equal angles A current, N 0, Z 1,
; C 1, V kept. X and Y kept.
TurnAngleToward:
    lda z:zTemp01
    cmp z:zTemp02
    beq TurnAngleToward_at0
    lda z:zTemp01
    bmi TurnAngleToward_at1
    cmp z:zTemp02
    bcs TurnAngleToward_at2
    clc
    adc #$80
    cmp z:zTemp02
    bcc TurnAngleToward_at2
    bra TurnAngleToward_at3
TurnAngleToward_at1:
    cmp z:zTemp02
    bcc TurnAngleToward_at3
    sec
    sbc #$80
    cmp z:zTemp02
    bcs TurnAngleToward_at3
    bra TurnAngleToward_at2
TurnAngleToward_at0:
    rts
TurnAngleToward_at2:
    lda z:zTemp01
    sec
    sbc z:zTemp00
    rts
TurnAngleToward_at3:
    lda z:zTemp01
    clc
    adc z:zTemp00
    rts
