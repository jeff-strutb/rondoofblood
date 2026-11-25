; @routine 0259:000 RunObjects
; @run 70:A000
; @stop A13C
; WHAT IT DOES: Runs every object slot from $10 to $57 once. For each slot with a kind and a state: unless its flags bit 0 says it stays put, it moves by the scroll speeds (the table
; entry by its flags); an object whose flags bit 1 is clear and which is far off the first screen is cleared; the hit handler runs (ObjectHit); an object whose flags bit 2 is
; set is held while it is outside the window; one with a hurt timer counts it down; the player's mode (PlayerByte3BD8) may skip the object on some frames by a wave-counter table;
; the rest dispatch by kind (DispatchByKind0259). A slot with no state dispatches at once. At the end, every fourth wave tick it starts the next script of a cue table at $A13C.
; HOW: A loop of X from $10 over the pieces of the original (000, 044, 05E, 090, 0C0, 0D1, 112), which are one function here; each place the original code is entered, the registers the
; typed steps so far left are put into the machine first.
; Untraced: parts of the position tests and of the player modes' tables.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
RunObjects:
    phx
    ldx #$10
    lda a:OBJ_2750,x
    bne RunObjects_at0
    jmp RunObjects_at1
RunObjects_at0:
    lda a:OBJ_27A8,x
    bne RunObjects_at2
    jmp RunObjects_at3
RunObjects_at2:
    lda a:OBJ_30F0,x
    and #$01
    bne RunObjects_at4
    lda a:OBJ_FLAGS_2F38,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects_at5
    dec z:zTemp00
RunObjects_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects_at6
    dec z:zTemp00
RunObjects_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects_at4:
    lda a:OBJ_27A8,x
    bne RunObjects_at7
    jmp RunObjects_at3
RunObjects_at7:
    lda a:OBJ_30F0,x
    and #$02
    bne RunObjects_at8
    lda a:OBJ_X_HI,x
    beq RunObjects_at9
    bpl RunObjects_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects_at9
    jsr VecClearObject
    jmp RunObjects_at1
RunObjects_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects_at8
    bpl RunObjects_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects_at8
    bra ScrollSpeedsFromBands
RunObjects_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects_at8:
    jsr ObjectHit
    lda a:OBJ_30F0,x
    and #$04
    beq RunObjects_at12
    lda a:OBJ_X_HI,x
    beq RunObjects_at12
    bpl RunObjects_at13
    cmp #$FF
    bne RunObjects_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects_at1
    bra RunObjects_at12
RunObjects_at13:
    cmp #$01
    bne RunObjects_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects_at1
RunObjects_at12:
    lda a:OBJ_2F90,x
    beq RunObjects_at14
    dec a:OBJ_2F90,x
    lda a:OBJ_3148,x
    and #$04
    bne RunObjects_at14
    bra RunObjects_at1
RunObjects_at14:
    lda a:PlayerByte3BD8
    beq RunObjects_at3
    cmp #$03
    bne RunObjects_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects_at1
    bra RunObjects_at3
RunObjects_at15:
    cmp #$02
    bne RunObjects_at16
    lda a:OBJ_3148,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:$A166,y
    bne RunObjects_at1
    bra RunObjects_at3
RunObjects_at16:
    lda a:OBJ_3148,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:$A146,y
    bne RunObjects_at1
RunObjects_at3:
    jsr DispatchByKind0259
RunObjects_at1:
    inx
    cpx #$58
    bcs RunObjects_at17
    jmp RichterVecStart
RunObjects_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects_at18
    ldy a:ObjectCueIndex
    lda a:$A13C,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects_at18
    stz a:ObjectCueIndex
RunObjects_at18:
    plx
    rts
