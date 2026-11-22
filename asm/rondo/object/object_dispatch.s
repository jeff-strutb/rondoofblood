; @routine 0519:46A EnterHurtStateOnHit
; @run 69:A46A
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
; @run 69:A74E
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
; @run 69:A482
; WHAT IT DOES: Starts a newly placed object: loads its record, copies slot 0's byte of the
; OBJ_2FE8 column into the object's, sets two of its counters and its animation delay to 4, and
; its timer to $60.
; HOW: Calls LoadObjectRecord through its vector (which keeps X), then stores each byte.
; WAITS: no.
; Entry: X object. Exit: A $60 and flags from that load; X kept; Y as LoadObjectRecord left it.
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
