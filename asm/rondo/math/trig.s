; @routine 0045:640 SetVelocityFromAngle
; @run 69:5640
; WHAT IT DOES: Points an object along an angle at a given speed: its X
; velocity becomes speed times cosine, its Y velocity speed times sine.
; HOW: Keeps the angle on the stack, ScaleByCosine and ScaleBySine in turn, taking the top
; two bytes of each product as an 8.8 velocity.
; WAITS: no.
; Entry: A angle, X object, zTemp00-01 speed, D clear. Exit: A the Y velocity's low
; byte, N and Z from it, C and V from the sine (V from the cosine if the sine set none), Y 0.
; X kept. Clobbers zTemp10-12, zProduct0-2, zMultiplierLo-Hi.
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
; WAITS: no.
; Entry: A angle, zTemp00-01 value, D clear. Exit: as ScaleBySine; V from the angle's
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
; WAITS: no.
; Entry: A angle, zTemp00-01 value, D clear. Exit: zProduct0-2 result; A, N, Z, C and V
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

; Angles, sines, the multiply under them, and the game's random numbers.
;
; The bodies work in whole words and leave the original's results in the original's RAM.
; What the assembly leaves in A and the flags is worked out from those values in the
; *_exit_shim functions, for the abi_ wrappers, and nowhere else.
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
; @routine 0045:69A MultiplyBySineWord
; @run 69:569A
; WHAT IT DOES: Multiplies the 16-bit value in zTemp00 by the 9-bit word in
; zMultiplierLo, for a 24-bit product in zProduct0-2; the word is a sine, so
; the product is the value scaled by that sine as 8.8.
; HOW: Shift and add, nine rounds: the multiplier shifts right through RAM, a
; 24-bit copy of the value in zTemp10-12 shifts left, and each bit shifted out
; set adds the copy to the product, byte by byte with the carry running on.
; WAITS: no.
; Entry: zTemp00-01 value, zMultiplierLo-Hi word, D clear. Exit: A the last byte
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
; WHAT IT DOES: The angle from an object to a point: the point's coordinates (zTemp08-0B) minus
; the object's, each axis a 16-bit subtract, then AngleFromDelta.
; HOW: As AngleToPlayer, with the point in place of the player; a branch into AngleFromDelta.
; WAITS: no.
; Entry: X object, zTemp08-0B the point, D clear. Exit: as AngleFromDelta. Clobbers zTemp00-03.
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
; WHAT IT DOES: The angle from an object to a point offset from the player: the player's position
; minus the object's, plus the offset in zTemp08-0B, each axis 16 bits, then AngleFromDelta.
; HOW: The four subtracts as AngleToPlayer, then the four adds of the offset; a branch into
; AngleFromDelta.
; WAITS: no.
; Entry: X object, zTemp08-0B the offset, D clear. Exit: as AngleFromDelta. Clobbers zTemp00-03.
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
; WHAT IT DOES: The angle from an object to the player: the player's position minus the object's,
; each axis a 16-bit subtract, then AngleFromDelta.
; HOW: Four bytes subtracted with the carries running on; it runs into AngleFromDelta.
; WAITS: no.
; Entry: X object, D clear. Exit: as AngleFromDelta. Clobbers zTemp00-03.
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
; WHAT IT DOES: The angle of a vector from its signed 16-bit axes (zTemp00-01 across, zTemp02-03
; up): both are made positive, AngleOfFirstQuadrant gives the angle in the first quarter turn, and
; the signs fold it into its quarter.
; HOW: Tests the signs of the two high bytes; the negative axes are negated in place; the
; folds are: across negative, up positive: $81 minus the angle; both negative: the angle plus
; $80; across positive, up negative: 2 minus the angle (in bytes, $FF minus it plus 1 plus 1).
; WAITS: no.
; Entry: zTemp00-03 the axes, D clear. Exit: A the angle, N and Z from it, C and V as the last
; add or subtract (or the first quarter's: C 0, V 1) left them, Y the table index.
AngleFromDelta:
    lda z:zTemp03
    bmi AngleFromDelta_at0
    lda z:zTemp01
    bmi AngleFromDelta_at1
    bra AngleOfFirstQuadrant
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
    bsr AngleOfFirstQuadrant
    sec
    sbc #$80
    eor #$FF
    inc a
    inc a
    rts
AngleFromDelta_at0:
    lda z:zTemp01
    bpl AngleFromDelta_at2
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
    bsr AngleOfFirstQuadrant
    clc
    adc #$80
    rts
AngleFromDelta_at2:
    lda z:zTemp02
    eor #$FF
    clc
    adc #$01
    sta z:zTemp02
    lda z:zTemp03
    eor #$FF
    adc #$00
    sta z:zTemp03
    bsr AngleOfFirstQuadrant
    eor #$FF
    inc a
    inc a
    rts

; @routine 0046:0B8 AngleOfFirstQuadrant
; @run 69:58B8
; WHAT IT DOES: The angle of a vector whose two axes (16-bit, zTemp00-01 across and zTemp02-03
; up) are both positive, within the first quarter turn.
; HOW: Both axes shift down together until the larger fits in a byte, then until both fit four
; bits (the counts kept in A), and the two nibbles index a table of angles at overlay address
; $58E7 (Y first, X in the low nibble).
; WAITS: no.
; Entry: zTemp00-03 the axes, D clear. Exit: A the angle, N and Z from it, C 0, V 1 (the
; BIT #$F0 leaves V set), Y the table index; zTemp00-03 shifted down.
AngleOfFirstQuadrant:
    lda z:zTemp01
    ora z:zTemp03
    beq AngleOfFirstQuadrant_at0
AngleOfFirstQuadrant_at1:
    lsr z:zTemp01
    ror z:zTemp00
    lsr z:zTemp03
    ror z:zTemp02
    lsr a
    bne AngleOfFirstQuadrant_at1
AngleOfFirstQuadrant_at0:
    lda z:zTemp00
    ora z:zTemp02
    bit #$F0
    beq AngleOfFirstQuadrant_at2
AngleOfFirstQuadrant_at3:
    lsr z:zTemp00
    lsr z:zTemp02
    lsr a
    bit #$F0
    bne AngleOfFirstQuadrant_at3
AngleOfFirstQuadrant_at2:
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
; HOW: A 32-bit counter generator: the state becomes state * $01010101 + 1. Each
; byte is added into those above it, the carry running on between the adds, top byte
; first, then the 32-bit increment, and the top byte is read again as the answer.
; WAITS: no.
; Entry: nothing, D clear. Exit: A zRandom3, N and Z from it, C and V from the last add (the
; top byte plus zRandom0 with the carry of the one before). X and Y kept. The increments
; past zRandom0 (when it wraps) ran in no script: the draft stops there, so liftcmp does not
; judge them; they wait for coverage (PROCEDURE section 6).
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
; HOW: Equal angles stay. Otherwise compares them in the half of the circle the current angle
; lies in, a half turn deciding which way is shorter, and adds or subtracts the step. A step
; can overshoot the target: nothing here clamps it.
; WAITS: no.
; Entry: zTemp00 step, zTemp01 current, zTemp02 target, D clear. Exit: A the new angle, N and Z
; from it, C and V from the step; on equal angles A current, N 0, Z 1, C 1, V kept. X and Y
; kept.
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
