; @routine 0259:186 ObjectHit
; @run 70:A186
; WHAT IT DOES: What an object does with the hits it has taken this frame (the handler every enemy's state runs): its two attack marks are cleared; an object already marked dead (status bit 7) is left;
; the player's attacks (HitAttackFlags bits 0 and 1) mark it and decide whether it is hit; a hit with damage (HitDamage) shortens its health OBJ_32A8, flashes it ($86) and gives it a hurt timer;
; when the health runs out or an attack kills it outright it records its event, scores (ScoreForKind), may drop an item (DropItemWhenHitF) and plays its hit sound (PlayHitSoundA), then turns into
; the kind $9B plus its death number (or is cleared when that number is 7), or, with no number, is marked dead and its hit state cleared.
; HOW: Bit tests and stores in the order of the assembly; the steps are returned as an exit chain.
; Untraced: two arms of the hit and the death are shared with the chain; the draft reaches them as stubs (see liftcmp).
; WAITS: no.
; Entry: X object, Y the registers' Y for the sound calls. Exit: see abi_chain.
ObjectHit:
    lda a:OBJ_31A0,x
    and #$9F
    sta a:OBJ_31A0,x
    lda a:OBJ_HIT_REACTION,x
    and #$03
    beq ObjectHit_at0
    dec a:OBJ_HIT_REACTION,x
    lda a:OBJ_HIT_REACTION,x
    and #$03
    bne ObjectHit_at0
    lda a:OBJ_3AE8,x
    sta a:OBJ_2B18,x
ObjectHit_at0:
    lda a:OBJ_31A0,x
    and #$80
    beq ObjectHit_at1
    jmp ObjectHit_at2
ObjectHit_at1:
    lda a:HitAttackFlags
    and #$01
    beq ObjectHit_at3
    lda a:OBJ_31A0,x
    ora #$20
    sta a:OBJ_31A0,x
    lda a:OBJ_30F0,x
    and #$20
    bne ObjectHit_at4
ObjectHit_at3:
    lda a:HitAttackFlags
    and #$02
    beq ObjectHit_at5
    lda a:OBJ_31A0,x
    ora #$40
    sta a:OBJ_31A0,x
    lda a:OBJ_30F0,x
    and #$40
    bne ObjectHit_at4
ObjectHit_at5:
    lda a:OBJ_32A8,x
    bmi ObjectHit_at2
    lda a:OBJ_HIT_TAKEN,x
    beq ObjectHit_at2
    jsr HitDamage
    lda z:zTemp00
    bne ObjectHit_at6
    rts
ObjectHit_at6:
    ldy a:OBJ_HIT_TAKEN,x
    lda a:$A26D,y
    sta a:OBJ_3A90,x
    lda a:$A27D,y
    sta a:OBJ_2F90,x
    lda a:OBJ_2B18,x
    sta a:OBJ_3AE8,x
    lda #$86
    sta a:OBJ_2B18,x
    lda a:OBJ_HIT_REACTION,x
    ora #$03
    sta a:OBJ_HIT_REACTION,x
    lda a:OBJ_32A8,x
    sec
    sbc z:zTemp00
    sta a:OBJ_32A8,x
    bcc ObjectHit_at4
    lda a:OBJ_31A0,x
    lsr a
    lsr a
    lsr a
    and #$03
    tay
    lda a:$A269,y
    jmp VecSoundIfOnFirstScreen
ObjectHit_at4:
    jsr VecRecordObjectEvent
    jsr ScoreForKind
    jsr DropItemWhenHitF
    jsr PlayHitSoundA
    lda a:OBJ_31A0,x
    and #$07
    beq ObjectHit_at7
    cmp #$07
    bne ObjectHit_at8
    jmp VecClearObject
ObjectHit_at8:
    dec a
    clc
    adc #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
ObjectHit_at2:
    rts
ObjectHit_at7:
    lda a:OBJ_30F0,x
    and #$FB
    sta a:OBJ_30F0,x
    lda a:OBJ_3148,x
    and #$87
    ora #$04
    sta a:OBJ_3148,x
    lda a:OBJ_31A0,x
    ora #$80
    sta a:OBJ_31A0,x
    rts

; @routine 0259:28D HitDamage
; @run 70:A28D
; WHAT IT DOES: Looks up how much damage the object takes from the hit it has taken: a table at $ABA6 indexed by the object's OBJ_2EE0 times sixteen plus its hit; the damage goes to zTemp00.
; HOW: Four shifts, an add, a table read (bank 70).
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:$ABA6,y
    sta z:zTemp00
    rts

; @routine 0259:2A3 ScoreForKind
; @run 70:A2A3
; WHAT IT DOES: When the byte HitAttackFlags has bit 0 clear, adds the score a table at $ABF5 gives for the object's kind less $20 (two bytes of BCD, to zTemp00-01); when that carries over
; the mark the score tests for it, the player gets an extra life (GiveExtraLife).
; HOW: AddScoreWord, a table read (bank 70).
; Untraced: the extra life (A2C1).
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: see the wrapper.
ScoreForKind:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind_at0
    lda a:OBJ_KIND,x
    sec
    sbc #$20
    asl a
    tay
    lda a:$ABF5,y
    sta z:zTemp00
    lda a:$ABF6,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind_at0
    jsr GiveExtraLife
ScoreForKind_at0:
    rts
