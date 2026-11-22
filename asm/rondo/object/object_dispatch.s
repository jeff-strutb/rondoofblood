; @routine 0519:46A EnterHurtStateOnHit
; @run 70:A46A
; WHAT IT DOES: Sends an object that is in one of its first four states and has taken a hit
; of 1 to 4 into state 5, the state its handler table uses for being hurt.
; HOW: Reads the state and the hit byte, each once, and writes the state only when both are
; from 1 to 4.
; WAITS: no.
; Entry: X object. Exit: A and flags as the last compare or load left them; X and Y kept.
EnterHurtStateOnHit:
    lda a:OBJ_27A8,x
    beq EnterHurtStateOnHit_out
    cmp #$05
    bcs EnterHurtStateOnHit_out
    lda a:OBJ_33B0,x
    beq EnterHurtStateOnHit_out
    cmp #$05
    bcs EnterHurtStateOnHit_out
    lda #$05
    sta a:OBJ_27A8,x
EnterHurtStateOnHit_out:
    rts

; @routine 0519:74E SetBlinkFlash
; @run 70:A74E
; WHAT IT DOES: Sets an object's flash flag (the palette flash its sprite shows) to $86 on
; alternate beats of the wave counter once its timer has reached $18, and to 0 otherwise.
; HOW: Clears the flag first, then reads the timer and, if it is $18 or more and bit 1 of the
; wave counter is clear, writes $86.
; WAITS: no.
; Entry: X object. Exit: A and flags as the last compare, AND or load left them; X and Y kept.
SetBlinkFlash:
    stz a:OBJ_2B18,x
    lda a:OBJ_3A90,x
    cmp #$18
    bcc SetBlinkFlash_out
    lda z:zWaveCounter
    and #$02
    bne SetBlinkFlash_out
    lda #$86
    sta a:OBJ_2B18,x
SetBlinkFlash_out:
    rts

; @routine 0519:482 InitObjectFromRecord
; @run 70:A482
; WHAT IT DOES: Starts a newly placed object: loads its record, copies slot 0's byte of the
; OBJ_2FE8 column into the object's, sets two of its counters and its animation delay to 4, and
; its timer to $60.
; HOW: Calls LoadObjectRecord through its vector (which keeps X), then stores each byte.
; WAITS: no.
; Entry: X object. Exit: A $60, N and Z from it, C as LoadObjectRecord left it; Y 11; X kept. D clear.
InitObjectFromRecord:
    jsr VecLoadObjectRecord
    lda a:OBJ_2FE8
    sta a:OBJ_2FE8,x
    lda #$04
    sta a:OBJ_3988,x
    sta a:OBJ_3A38,x
    lda #$04
    sta a:OBJ_3510,x
    lda #$60
    sta a:OBJ_34B8,x
    rts

; @routine 0519:4BD FallAndBounce
; @run 70:A4BD
; WHAT IT DOES: Moves an object by its velocity and asks the ground under it; on floor the
; object enters its next state. Unless the ground is deadly, the object's X speed is set
; from its angle (the full cosine) and its Y speed to $80, and its angle is turned on by
; $0280.
; HOW: Calls ApplyVelocity and ProbeTerrain (floor, height 0), then ScaleByCosine with a
; multiplier of 256; the angle is the two bytes OBJ_35C0 (low) and OBJ_3568 (high).
; WAITS: no.
; Entry: X object. Exit: A and flags from the last AND or add; X kept; Y as ScaleByCosine
; left it.
FallAndBounce:
    jsr VecApplyVelocity
    lda #$00
    sta z:zProduct1
    lda #$01
    jsr VecProbeTerrain
    and #$01
    beq FallAndBounce_at0
    inc a:OBJ_27A8,x
FallAndBounce_at0:
    lda z:zProduct0
    and #$10
    bne FallAndBounce_out
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_3568,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:zProduct1
    sta a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda a:OBJ_35C0,x
    clc
    adc #$80
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    adc #$02
    sta a:OBJ_3568,x
FallAndBounce_out:
    rts

; @routine 051A:055 BitMaskFromIndex
; @run 70:A855
; WHAT IT DOES: Gives the byte with only bit Y set (1 shifted left Y times; 0 if Y is 8 or more).
; HOW: Shifts a 1 left, counting Y down to 0.
; WAITS: no.
; Entry: Y the bit number. Exit: A the mask; Y 0; Z and C set, N clear (the last compare).
BitMaskFromIndex:
    lda #$01
BitMaskFromIndex_at0:
    cpy #$00
    beq BitMaskFromIndex_out
    asl a
    dey
    bra BitMaskFromIndex_at0
BitMaskFromIndex_out:
    rts
