; @routine 0259:000 RunObjects
; @run 70:A000
; @stop A13C
; WHAT IT DOES: Runs every object slot from $10 to $57 once. For each slot with a kind and a state: unless its flags bit 0 says it stays put, it moves by the scroll speeds (the table
; entry by its flags); an object whose flags bit 1 is clear and which is far off the first screen is cleared; the hit handler runs (ObjectHit); an object whose flags bit 2 is
; set is held while it is outside the window; one with a hurt timer counts it down; the player's mode (TimeSlowMode) may skip the object on some frames by a wave-counter table;
; the rest dispatch by kind (DispatchByKind0259). A slot with no state dispatches at once. At the end, every fourth wave tick it starts the next script of a cue table at $A13C.
; HOW: A loop of X from $10 over the pieces of the original (000, 044, 05E, 090, 0C0, 0D1, 112), which are one function here; each place the original code is entered, the registers the
; typed steps so far left are put into the machine first.
; Untraced: parts of the position tests and of the player modes' tables.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps2 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps1 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects_at0
    jmp RunObjects_at1
RunObjects_at0:
    lda a:OBJ_STATE,x
    bne RunObjects_at2
    jmp RunObjects_at3
RunObjects_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects_at4
    lda a:OBJ_SCROLL_BAND,x
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
    lda a:OBJ_STATE,x
    bne RunObjects_at7
    jmp RunObjects_at3
RunObjects_at7:
    lda a:OBJ_LOOP_FLAGS,x
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
    lda a:OBJ_LOOP_FLAGS,x
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
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects_at14
    bra RunObjects_at1
RunObjects_at14:
    lda a:TimeSlowMode
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
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps2,y
    bne RunObjects_at1
    bra RunObjects_at3
RunObjects_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps1,y
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
    lda a:ObjectCueScripts,y
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
; @routine 07D9:000 RunObjects07D9
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 07D9's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind07D9 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps207D9 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps107D9 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts07D9 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects07D9:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects07D9_at0
    jmp RunObjects07D9_at1
RunObjects07D9_at0:
    lda a:OBJ_STATE,x
    bne RunObjects07D9_at2
    jmp RunObjects07D9_at3
RunObjects07D9_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects07D9_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects07D9_at5
    dec z:zTemp00
RunObjects07D9_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects07D9_at6
    dec z:zTemp00
RunObjects07D9_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects07D9_at4:
    lda a:OBJ_STATE,x
    bne RunObjects07D9_at7
    jmp RunObjects07D9_at3
RunObjects07D9_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects07D9_at8
    lda a:OBJ_X_HI,x
    beq RunObjects07D9_at9
    bpl RunObjects07D9_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects07D9_at9
    jsr VecClearObject
    jmp RunObjects07D9_at1
RunObjects07D9_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects07D9_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects07D9_at8
    bpl RunObjects07D9_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects07D9_at8
    bra ScrollSpeedsFromBands
RunObjects07D9_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects07D9_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects07D9_at12
    lda a:OBJ_X_HI,x
    beq RunObjects07D9_at12
    bpl RunObjects07D9_at13
    cmp #$FF
    bne RunObjects07D9_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects07D9_at1
    bra RunObjects07D9_at12
RunObjects07D9_at13:
    cmp #$01
    bne RunObjects07D9_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects07D9_at1
RunObjects07D9_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects07D9_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects07D9_at14
    bra RunObjects07D9_at1
RunObjects07D9_at14:
    lda a:TimeSlowMode
    beq RunObjects07D9_at3
    cmp #$03
    bne RunObjects07D9_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects07D9_at1
    bra RunObjects07D9_at3
RunObjects07D9_at15:
    cmp #$02
    bne RunObjects07D9_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps207D9,y
    bne RunObjects07D9_at1
    bra RunObjects07D9_at3
RunObjects07D9_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps107D9,y
    bne RunObjects07D9_at1
RunObjects07D9_at3:
    jsr DispatchByKind07D9
RunObjects07D9_at1:
    inx
    cpx #$58
    bcs RunObjects07D9_at17
    jmp RichterVecStart
RunObjects07D9_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects07D9_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts07D9,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects07D9_at18
    stz a:ObjectCueIndex
RunObjects07D9_at18:
    plx
    rts

; @routine 0889:000 RunObjects0889
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 0889's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0889 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps20889 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps10889 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts0889 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects0889:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects0889_at0
    jmp RunObjects0889_at1
RunObjects0889_at0:
    lda a:OBJ_STATE,x
    bne RunObjects0889_at2
    jmp RunObjects0889_at3
RunObjects0889_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects0889_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects0889_at5
    dec z:zTemp00
RunObjects0889_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects0889_at6
    dec z:zTemp00
RunObjects0889_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects0889_at4:
    lda a:OBJ_STATE,x
    bne RunObjects0889_at7
    jmp RunObjects0889_at3
RunObjects0889_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects0889_at8
    lda a:OBJ_X_HI,x
    beq RunObjects0889_at9
    bpl RunObjects0889_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects0889_at9
    jsr VecClearObject
    jmp RunObjects0889_at1
RunObjects0889_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0889_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects0889_at8
    bpl RunObjects0889_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects0889_at8
    bra ScrollSpeedsFromBands
RunObjects0889_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0889_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects0889_at12
    lda a:OBJ_X_HI,x
    beq RunObjects0889_at12
    bpl RunObjects0889_at13
    cmp #$FF
    bne RunObjects0889_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects0889_at1
    bra RunObjects0889_at12
RunObjects0889_at13:
    cmp #$01
    bne RunObjects0889_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects0889_at1
RunObjects0889_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects0889_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects0889_at14
    bra RunObjects0889_at1
RunObjects0889_at14:
    lda a:TimeSlowMode
    beq RunObjects0889_at3
    cmp #$03
    bne RunObjects0889_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects0889_at1
    bra RunObjects0889_at3
RunObjects0889_at15:
    cmp #$02
    bne RunObjects0889_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps20889,y
    bne RunObjects0889_at1
    bra RunObjects0889_at3
RunObjects0889_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps10889,y
    bne RunObjects0889_at1
RunObjects0889_at3:
    jsr DispatchByKind0889
RunObjects0889_at1:
    inx
    cpx #$58
    bcs RunObjects0889_at17
    jmp RichterVecStart
RunObjects0889_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects0889_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts0889,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects0889_at18
    stz a:ObjectCueIndex
RunObjects0889_at18:
    plx
    rts

; @routine 0A99:000 RunObjects0A99
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 0A99's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0A99 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps20A99 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps10A99 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts0A99 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects0A99:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects0A99_at0
    jmp RunObjects0A99_at1
RunObjects0A99_at0:
    lda a:OBJ_STATE,x
    bne RunObjects0A99_at2
    jmp RunObjects0A99_at3
RunObjects0A99_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects0A99_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects0A99_at5
    dec z:zTemp00
RunObjects0A99_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects0A99_at6
    dec z:zTemp00
RunObjects0A99_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects0A99_at4:
    lda a:OBJ_STATE,x
    bne RunObjects0A99_at7
    jmp RunObjects0A99_at3
RunObjects0A99_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects0A99_at8
    lda a:OBJ_X_HI,x
    beq RunObjects0A99_at9
    bpl RunObjects0A99_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects0A99_at9
    jsr VecClearObject
    jmp RunObjects0A99_at1
RunObjects0A99_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0A99_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects0A99_at8
    bpl RunObjects0A99_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects0A99_at8
    bra ScrollSpeedsFromBands
RunObjects0A99_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0A99_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects0A99_at12
    lda a:OBJ_X_HI,x
    beq RunObjects0A99_at12
    bpl RunObjects0A99_at13
    cmp #$FF
    bne RunObjects0A99_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects0A99_at1
    bra RunObjects0A99_at12
RunObjects0A99_at13:
    cmp #$01
    bne RunObjects0A99_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects0A99_at1
RunObjects0A99_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects0A99_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects0A99_at14
    bra RunObjects0A99_at1
RunObjects0A99_at14:
    lda a:TimeSlowMode
    beq RunObjects0A99_at3
    cmp #$03
    bne RunObjects0A99_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects0A99_at1
    bra RunObjects0A99_at3
RunObjects0A99_at15:
    cmp #$02
    bne RunObjects0A99_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps20A99,y
    bne RunObjects0A99_at1
    bra RunObjects0A99_at3
RunObjects0A99_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps10A99,y
    bne RunObjects0A99_at1
RunObjects0A99_at3:
    jsr DispatchByKind0A99
RunObjects0A99_at1:
    inx
    cpx #$58
    bcs RunObjects0A99_at17
    jmp RichterVecStart
RunObjects0A99_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects0A99_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts0A99,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects0A99_at18
    stz a:ObjectCueIndex
RunObjects0A99_at18:
    plx
    rts

; @routine 0B49:000 RunObjects0B49
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 0B49's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0889 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps20B49 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps10B49 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts0B49 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects0B49:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects0B49_at0
    jmp RunObjects0B49_at1
RunObjects0B49_at0:
    lda a:OBJ_STATE,x
    bne RunObjects0B49_at2
    jmp RunObjects0B49_at3
RunObjects0B49_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects0B49_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects0B49_at5
    dec z:zTemp00
RunObjects0B49_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects0B49_at6
    dec z:zTemp00
RunObjects0B49_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects0B49_at4:
    lda a:OBJ_STATE,x
    bne RunObjects0B49_at7
    jmp RunObjects0B49_at3
RunObjects0B49_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects0B49_at8
    lda a:OBJ_X_HI,x
    beq RunObjects0B49_at9
    bpl RunObjects0B49_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects0B49_at9
    jsr VecClearObject
    jmp RunObjects0B49_at1
RunObjects0B49_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0B49_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects0B49_at8
    bpl RunObjects0B49_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects0B49_at8
    bra ScrollSpeedsFromBands
RunObjects0B49_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0B49_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects0B49_at12
    lda a:OBJ_X_HI,x
    beq RunObjects0B49_at12
    bpl RunObjects0B49_at13
    cmp #$FF
    bne RunObjects0B49_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects0B49_at1
    bra RunObjects0B49_at12
RunObjects0B49_at13:
    cmp #$01
    bne RunObjects0B49_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects0B49_at1
RunObjects0B49_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects0B49_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects0B49_at14
    bra RunObjects0B49_at1
RunObjects0B49_at14:
    lda a:TimeSlowMode
    beq RunObjects0B49_at3
    cmp #$03
    bne RunObjects0B49_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects0B49_at1
    bra RunObjects0B49_at3
RunObjects0B49_at15:
    cmp #$02
    bne RunObjects0B49_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps20B49,y
    bne RunObjects0B49_at1
    bra RunObjects0B49_at3
RunObjects0B49_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps10B49,y
    bne RunObjects0B49_at1
RunObjects0B49_at3:
    jsr DispatchByKind0A99
RunObjects0B49_at1:
    inx
    cpx #$58
    bcs RunObjects0B49_at17
    jmp RichterVecStart
RunObjects0B49_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects0B49_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts0B49,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects0B49_at18
    stz a:ObjectCueIndex
RunObjects0B49_at18:
    plx
    rts

; @routine 0CA9:000 RunObjects0CA9
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 0CA9's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0CA9 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps20CA9 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps10CA9 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts0CA9 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects0CA9:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects0CA9_at0
    jmp RunObjects0CA9_at1
RunObjects0CA9_at0:
    lda a:OBJ_STATE,x
    bne RunObjects0CA9_at2
    jmp RunObjects0CA9_at3
RunObjects0CA9_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects0CA9_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects0CA9_at5
    dec z:zTemp00
RunObjects0CA9_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects0CA9_at6
    dec z:zTemp00
RunObjects0CA9_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects0CA9_at4:
    lda a:OBJ_STATE,x
    bne RunObjects0CA9_at7
    jmp RunObjects0CA9_at3
RunObjects0CA9_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects0CA9_at8
    lda a:OBJ_X_HI,x
    beq RunObjects0CA9_at9
    bpl RunObjects0CA9_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects0CA9_at9
    jsr VecClearObject
    jmp RunObjects0CA9_at1
RunObjects0CA9_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0CA9_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects0CA9_at8
    bpl RunObjects0CA9_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects0CA9_at8
    bra ScrollSpeedsFromBands
RunObjects0CA9_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0CA9_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects0CA9_at12
    lda a:OBJ_X_HI,x
    beq RunObjects0CA9_at12
    bpl RunObjects0CA9_at13
    cmp #$FF
    bne RunObjects0CA9_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects0CA9_at1
    bra RunObjects0CA9_at12
RunObjects0CA9_at13:
    cmp #$01
    bne RunObjects0CA9_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects0CA9_at1
RunObjects0CA9_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects0CA9_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects0CA9_at14
    bra RunObjects0CA9_at1
RunObjects0CA9_at14:
    lda a:TimeSlowMode
    beq RunObjects0CA9_at3
    cmp #$03
    bne RunObjects0CA9_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects0CA9_at1
    bra RunObjects0CA9_at3
RunObjects0CA9_at15:
    cmp #$02
    bne RunObjects0CA9_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps20CA9,y
    bne RunObjects0CA9_at1
    bra RunObjects0CA9_at3
RunObjects0CA9_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps10CA9,y
    bne RunObjects0CA9_at1
RunObjects0CA9_at3:
    jsr DispatchByKind0CA9
RunObjects0CA9_at1:
    inx
    cpx #$58
    bcs RunObjects0CA9_at17
    jmp RichterVecStart
RunObjects0CA9_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects0CA9_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts0CA9,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects0CA9_at18
    stz a:ObjectCueIndex
RunObjects0CA9_at18:
    plx
    rts

; @routine 0D59:000 RunObjects0D59
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 0D59's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0A99 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps20D59 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps10D59 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts0D59 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects0D59:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects0D59_at0
    jmp RunObjects0D59_at1
RunObjects0D59_at0:
    lda a:OBJ_STATE,x
    bne RunObjects0D59_at2
    jmp RunObjects0D59_at3
RunObjects0D59_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects0D59_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects0D59_at5
    dec z:zTemp00
RunObjects0D59_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects0D59_at6
    dec z:zTemp00
RunObjects0D59_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects0D59_at4:
    lda a:OBJ_STATE,x
    bne RunObjects0D59_at7
    jmp RunObjects0D59_at3
RunObjects0D59_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects0D59_at8
    lda a:OBJ_X_HI,x
    beq RunObjects0D59_at9
    bpl RunObjects0D59_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects0D59_at9
    jsr VecClearObject
    jmp RunObjects0D59_at1
RunObjects0D59_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0D59_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects0D59_at8
    bpl RunObjects0D59_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects0D59_at8
    bra ScrollSpeedsFromBands
RunObjects0D59_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0D59_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects0D59_at12
    lda a:OBJ_X_HI,x
    beq RunObjects0D59_at12
    bpl RunObjects0D59_at13
    cmp #$FF
    bne RunObjects0D59_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects0D59_at1
    bra RunObjects0D59_at12
RunObjects0D59_at13:
    cmp #$01
    bne RunObjects0D59_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects0D59_at1
RunObjects0D59_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects0D59_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects0D59_at14
    bra RunObjects0D59_at1
RunObjects0D59_at14:
    lda a:TimeSlowMode
    beq RunObjects0D59_at3
    cmp #$03
    bne RunObjects0D59_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects0D59_at1
    bra RunObjects0D59_at3
RunObjects0D59_at15:
    cmp #$02
    bne RunObjects0D59_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps20D59,y
    bne RunObjects0D59_at1
    bra RunObjects0D59_at3
RunObjects0D59_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps10D59,y
    bne RunObjects0D59_at1
RunObjects0D59_at3:
    jsr DispatchByKind0CA9
RunObjects0D59_at1:
    inx
    cpx #$58
    bcs RunObjects0D59_at17
    jmp RichterVecStart
RunObjects0D59_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects0D59_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts0D59,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects0D59_at18
    stz a:ObjectCueIndex
RunObjects0D59_at18:
    plx
    rts

; @routine 0E09:000 RunObjects0E09
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 0E09's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0E09 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps20E09 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps10E09 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts0E09 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects0E09:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects0E09_at0
    jmp RunObjects0E09_at1
RunObjects0E09_at0:
    lda a:OBJ_STATE,x
    bne RunObjects0E09_at2
    jmp RunObjects0E09_at3
RunObjects0E09_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects0E09_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects0E09_at5
    dec z:zTemp00
RunObjects0E09_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects0E09_at6
    dec z:zTemp00
RunObjects0E09_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects0E09_at4:
    lda a:OBJ_STATE,x
    bne RunObjects0E09_at7
    jmp RunObjects0E09_at3
RunObjects0E09_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects0E09_at8
    lda a:OBJ_X_HI,x
    beq RunObjects0E09_at9
    bpl RunObjects0E09_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects0E09_at9
    jsr VecClearObject
    jmp RunObjects0E09_at1
RunObjects0E09_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0E09_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects0E09_at8
    bpl RunObjects0E09_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects0E09_at8
    bra ScrollSpeedsFromBands
RunObjects0E09_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0E09_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects0E09_at12
    lda a:OBJ_X_HI,x
    beq RunObjects0E09_at12
    bpl RunObjects0E09_at13
    cmp #$FF
    bne RunObjects0E09_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects0E09_at1
    bra RunObjects0E09_at12
RunObjects0E09_at13:
    cmp #$01
    bne RunObjects0E09_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects0E09_at1
RunObjects0E09_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects0E09_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects0E09_at14
    bra RunObjects0E09_at1
RunObjects0E09_at14:
    lda a:TimeSlowMode
    beq RunObjects0E09_at3
    cmp #$03
    bne RunObjects0E09_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects0E09_at1
    bra RunObjects0E09_at3
RunObjects0E09_at15:
    cmp #$02
    bne RunObjects0E09_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps20E09,y
    bne RunObjects0E09_at1
    bra RunObjects0E09_at3
RunObjects0E09_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps10E09,y
    bne RunObjects0E09_at1
RunObjects0E09_at3:
    jsr DispatchByKind0E09
RunObjects0E09_at1:
    inx
    cpx #$58
    bcs RunObjects0E09_at17
    jmp RichterVecStart
RunObjects0E09_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects0E09_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts0E09,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects0E09_at18
    stz a:ObjectCueIndex
RunObjects0E09_at18:
    plx
    rts

; @routine 0EB9:000 RunObjects0EB9
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 0EB9's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0A99 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps20EB9 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps10EB9 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts0EB9 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects0EB9:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects0EB9_at0
    jmp RunObjects0EB9_at1
RunObjects0EB9_at0:
    lda a:OBJ_STATE,x
    bne RunObjects0EB9_at2
    jmp RunObjects0EB9_at3
RunObjects0EB9_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects0EB9_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects0EB9_at5
    dec z:zTemp00
RunObjects0EB9_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects0EB9_at6
    dec z:zTemp00
RunObjects0EB9_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects0EB9_at4:
    lda a:OBJ_STATE,x
    bne RunObjects0EB9_at7
    jmp RunObjects0EB9_at3
RunObjects0EB9_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects0EB9_at8
    lda a:OBJ_X_HI,x
    beq RunObjects0EB9_at9
    bpl RunObjects0EB9_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects0EB9_at9
    jsr VecClearObject
    jmp RunObjects0EB9_at1
RunObjects0EB9_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0EB9_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects0EB9_at8
    bpl RunObjects0EB9_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects0EB9_at8
    bra ScrollSpeedsFromBands
RunObjects0EB9_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0EB9_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects0EB9_at12
    lda a:OBJ_X_HI,x
    beq RunObjects0EB9_at12
    bpl RunObjects0EB9_at13
    cmp #$FF
    bne RunObjects0EB9_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects0EB9_at1
    bra RunObjects0EB9_at12
RunObjects0EB9_at13:
    cmp #$01
    bne RunObjects0EB9_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects0EB9_at1
RunObjects0EB9_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects0EB9_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects0EB9_at14
    bra RunObjects0EB9_at1
RunObjects0EB9_at14:
    lda a:TimeSlowMode
    beq RunObjects0EB9_at3
    cmp #$03
    bne RunObjects0EB9_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects0EB9_at1
    bra RunObjects0EB9_at3
RunObjects0EB9_at15:
    cmp #$02
    bne RunObjects0EB9_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps20EB9,y
    bne RunObjects0EB9_at1
    bra RunObjects0EB9_at3
RunObjects0EB9_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps10EB9,y
    bne RunObjects0EB9_at1
RunObjects0EB9_at3:
    jsr DispatchByKind0E09
RunObjects0EB9_at1:
    inx
    cpx #$58
    bcs RunObjects0EB9_at17
    jmp RichterVecStart
RunObjects0EB9_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects0EB9_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts0EB9,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects0EB9_at18
    stz a:ObjectCueIndex
RunObjects0EB9_at18:
    plx
    rts

; @routine 0F69:000 RunObjects0F69
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 0F69's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0889 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps20F69 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps10F69 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts0F69 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects0F69:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects0F69_at0
    jmp RunObjects0F69_at1
RunObjects0F69_at0:
    lda a:OBJ_STATE,x
    bne RunObjects0F69_at2
    jmp RunObjects0F69_at3
RunObjects0F69_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects0F69_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects0F69_at5
    dec z:zTemp00
RunObjects0F69_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects0F69_at6
    dec z:zTemp00
RunObjects0F69_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects0F69_at4:
    lda a:OBJ_STATE,x
    bne RunObjects0F69_at7
    jmp RunObjects0F69_at3
RunObjects0F69_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects0F69_at8
    lda a:OBJ_X_HI,x
    beq RunObjects0F69_at9
    bpl RunObjects0F69_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects0F69_at9
    jsr VecClearObject
    jmp RunObjects0F69_at1
RunObjects0F69_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0F69_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects0F69_at8
    bpl RunObjects0F69_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects0F69_at8
    bra ScrollSpeedsFromBands
RunObjects0F69_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects0F69_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects0F69_at12
    lda a:OBJ_X_HI,x
    beq RunObjects0F69_at12
    bpl RunObjects0F69_at13
    cmp #$FF
    bne RunObjects0F69_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects0F69_at1
    bra RunObjects0F69_at12
RunObjects0F69_at13:
    cmp #$01
    bne RunObjects0F69_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects0F69_at1
RunObjects0F69_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects0F69_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects0F69_at14
    bra RunObjects0F69_at1
RunObjects0F69_at14:
    lda a:TimeSlowMode
    beq RunObjects0F69_at3
    cmp #$03
    bne RunObjects0F69_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects0F69_at1
    bra RunObjects0F69_at3
RunObjects0F69_at15:
    cmp #$02
    bne RunObjects0F69_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps20F69,y
    bne RunObjects0F69_at1
    bra RunObjects0F69_at3
RunObjects0F69_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps10F69,y
    bne RunObjects0F69_at1
RunObjects0F69_at3:
    jsr DispatchByKind0E09
RunObjects0F69_at1:
    inx
    cpx #$58
    bcs RunObjects0F69_at17
    jmp RichterVecStart
RunObjects0F69_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects0F69_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts0F69,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects0F69_at18
    stz a:ObjectCueIndex
RunObjects0F69_at18:
    plx
    rts

; @routine 1019:000 RunObjects1019
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 1019's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0A99 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps21019 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps11019 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts1019 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects1019:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects1019_at0
    jmp RunObjects1019_at1
RunObjects1019_at0:
    lda a:OBJ_STATE,x
    bne RunObjects1019_at2
    jmp RunObjects1019_at3
RunObjects1019_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects1019_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects1019_at5
    dec z:zTemp00
RunObjects1019_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects1019_at6
    dec z:zTemp00
RunObjects1019_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects1019_at4:
    lda a:OBJ_STATE,x
    bne RunObjects1019_at7
    jmp RunObjects1019_at3
RunObjects1019_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects1019_at8
    lda a:OBJ_X_HI,x
    beq RunObjects1019_at9
    bpl RunObjects1019_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects1019_at9
    jsr VecClearObject
    jmp RunObjects1019_at1
RunObjects1019_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects1019_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects1019_at8
    bpl RunObjects1019_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects1019_at8
    bra ScrollSpeedsFromBands
RunObjects1019_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects1019_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects1019_at12
    lda a:OBJ_X_HI,x
    beq RunObjects1019_at12
    bpl RunObjects1019_at13
    cmp #$FF
    bne RunObjects1019_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects1019_at1
    bra RunObjects1019_at12
RunObjects1019_at13:
    cmp #$01
    bne RunObjects1019_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects1019_at1
RunObjects1019_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects1019_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects1019_at14
    bra RunObjects1019_at1
RunObjects1019_at14:
    lda a:TimeSlowMode
    beq RunObjects1019_at3
    cmp #$03
    bne RunObjects1019_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects1019_at1
    bra RunObjects1019_at3
RunObjects1019_at15:
    cmp #$02
    bne RunObjects1019_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps21019,y
    bne RunObjects1019_at1
    bra RunObjects1019_at3
RunObjects1019_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps11019,y
    bne RunObjects1019_at1
RunObjects1019_at3:
    jsr DispatchByKind10C9
RunObjects1019_at1:
    inx
    cpx #$58
    bcs RunObjects1019_at17
    jmp RichterVecStart
RunObjects1019_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects1019_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts1019,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects1019_at18
    stz a:ObjectCueIndex
RunObjects1019_at18:
    plx
    rts

; @routine 10C9:000 RunObjects10C9
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 10C9's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind10C9 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps210C9 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps110C9 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts10C9 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects10C9:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects10C9_at0
    jmp RunObjects10C9_at1
RunObjects10C9_at0:
    lda a:OBJ_STATE,x
    bne RunObjects10C9_at2
    jmp RunObjects10C9_at3
RunObjects10C9_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects10C9_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects10C9_at5
    dec z:zTemp00
RunObjects10C9_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects10C9_at6
    dec z:zTemp00
RunObjects10C9_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects10C9_at4:
    lda a:OBJ_STATE,x
    bne RunObjects10C9_at7
    jmp RunObjects10C9_at3
RunObjects10C9_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects10C9_at8
    lda a:OBJ_X_HI,x
    beq RunObjects10C9_at9
    bpl RunObjects10C9_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects10C9_at9
    jsr VecClearObject
    jmp RunObjects10C9_at1
RunObjects10C9_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects10C9_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects10C9_at8
    bpl RunObjects10C9_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects10C9_at8
    bra ScrollSpeedsFromBands
RunObjects10C9_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects10C9_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects10C9_at12
    lda a:OBJ_X_HI,x
    beq RunObjects10C9_at12
    bpl RunObjects10C9_at13
    cmp #$FF
    bne RunObjects10C9_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects10C9_at1
    bra RunObjects10C9_at12
RunObjects10C9_at13:
    cmp #$01
    bne RunObjects10C9_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects10C9_at1
RunObjects10C9_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects10C9_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects10C9_at14
    bra RunObjects10C9_at1
RunObjects10C9_at14:
    lda a:TimeSlowMode
    beq RunObjects10C9_at3
    cmp #$03
    bne RunObjects10C9_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects10C9_at1
    bra RunObjects10C9_at3
RunObjects10C9_at15:
    cmp #$02
    bne RunObjects10C9_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps210C9,y
    bne RunObjects10C9_at1
    bra RunObjects10C9_at3
RunObjects10C9_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps110C9,y
    bne RunObjects10C9_at1
RunObjects10C9_at3:
    jsr DispatchByKind10C9
RunObjects10C9_at1:
    inx
    cpx #$58
    bcs RunObjects10C9_at17
    jmp RichterVecStart
RunObjects10C9_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects10C9_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts10C9,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects10C9_at18
    stz a:ObjectCueIndex
RunObjects10C9_at18:
    plx
    rts

; @routine 1179:000 RunObjects1179
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 1179's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0A99 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps21179 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps11179 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts1179 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects1179:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects1179_at0
    jmp RunObjects1179_at1
RunObjects1179_at0:
    lda a:OBJ_STATE,x
    bne RunObjects1179_at2
    jmp RunObjects1179_at3
RunObjects1179_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects1179_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects1179_at5
    dec z:zTemp00
RunObjects1179_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects1179_at6
    dec z:zTemp00
RunObjects1179_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects1179_at4:
    lda a:OBJ_STATE,x
    bne RunObjects1179_at7
    jmp RunObjects1179_at3
RunObjects1179_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects1179_at8
    lda a:OBJ_X_HI,x
    beq RunObjects1179_at9
    bpl RunObjects1179_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects1179_at9
    jsr VecClearObject
    jmp RunObjects1179_at1
RunObjects1179_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects1179_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects1179_at8
    bpl RunObjects1179_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects1179_at8
    bra ScrollSpeedsFromBands
RunObjects1179_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects1179_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects1179_at12
    lda a:OBJ_X_HI,x
    beq RunObjects1179_at12
    bpl RunObjects1179_at13
    cmp #$FF
    bne RunObjects1179_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects1179_at1
    bra RunObjects1179_at12
RunObjects1179_at13:
    cmp #$01
    bne RunObjects1179_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects1179_at1
RunObjects1179_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects1179_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects1179_at14
    bra RunObjects1179_at1
RunObjects1179_at14:
    lda a:TimeSlowMode
    beq RunObjects1179_at3
    cmp #$03
    bne RunObjects1179_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects1179_at1
    bra RunObjects1179_at3
RunObjects1179_at15:
    cmp #$02
    bne RunObjects1179_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps21179,y
    bne RunObjects1179_at1
    bra RunObjects1179_at3
RunObjects1179_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps11179,y
    bne RunObjects1179_at1
RunObjects1179_at3:
    jsr DispatchByKind10C9
RunObjects1179_at1:
    inx
    cpx #$58
    bcs RunObjects1179_at17
    jmp RichterVecStart
RunObjects1179_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects1179_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts1179,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects1179_at18
    stz a:ObjectCueIndex
RunObjects1179_at18:
    plx
    rts

; @routine 12D9:000 RunObjects12D9
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 12D9's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind12D9 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps212D9 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps112D9 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts12D9 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects12D9:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects12D9_at0
    jmp RunObjects12D9_at1
RunObjects12D9_at0:
    lda a:OBJ_STATE,x
    bne RunObjects12D9_at2
    jmp RunObjects12D9_at3
RunObjects12D9_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects12D9_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects12D9_at5
    dec z:zTemp00
RunObjects12D9_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects12D9_at6
    dec z:zTemp00
RunObjects12D9_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects12D9_at4:
    lda a:OBJ_STATE,x
    bne RunObjects12D9_at7
    jmp RunObjects12D9_at3
RunObjects12D9_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects12D9_at8
    lda a:OBJ_X_HI,x
    beq RunObjects12D9_at9
    bpl RunObjects12D9_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects12D9_at9
    jsr VecClearObject
    jmp RunObjects12D9_at1
RunObjects12D9_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects12D9_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects12D9_at8
    bpl RunObjects12D9_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects12D9_at8
    bra ScrollSpeedsFromBands
RunObjects12D9_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects12D9_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects12D9_at12
    lda a:OBJ_X_HI,x
    beq RunObjects12D9_at12
    bpl RunObjects12D9_at13
    cmp #$FF
    bne RunObjects12D9_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects12D9_at1
    bra RunObjects12D9_at12
RunObjects12D9_at13:
    cmp #$01
    bne RunObjects12D9_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects12D9_at1
RunObjects12D9_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects12D9_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects12D9_at14
    bra RunObjects12D9_at1
RunObjects12D9_at14:
    lda a:TimeSlowMode
    beq RunObjects12D9_at3
    cmp #$03
    bne RunObjects12D9_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects12D9_at1
    bra RunObjects12D9_at3
RunObjects12D9_at15:
    cmp #$02
    bne RunObjects12D9_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps212D9,y
    bne RunObjects12D9_at1
    bra RunObjects12D9_at3
RunObjects12D9_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps112D9,y
    bne RunObjects12D9_at1
RunObjects12D9_at3:
    jsr DispatchByKind12D9
RunObjects12D9_at1:
    inx
    cpx #$58
    bcs RunObjects12D9_at17
    jmp RichterVecStart
RunObjects12D9_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects12D9_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts12D9,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects12D9_at18
    stz a:ObjectCueIndex
RunObjects12D9_at18:
    plx
    rts

; @routine 13F9:000 RunObjects13F9
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 13F9's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind12D9 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps213F9 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps113F9 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts13F9 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects13F9:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects13F9_at0
    jmp RunObjects13F9_at1
RunObjects13F9_at0:
    lda a:OBJ_STATE,x
    bne RunObjects13F9_at2
    jmp RunObjects13F9_at3
RunObjects13F9_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects13F9_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects13F9_at5
    dec z:zTemp00
RunObjects13F9_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects13F9_at6
    dec z:zTemp00
RunObjects13F9_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects13F9_at4:
    lda a:OBJ_STATE,x
    bne RunObjects13F9_at7
    jmp RunObjects13F9_at3
RunObjects13F9_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects13F9_at8
    lda a:OBJ_X_HI,x
    beq RunObjects13F9_at9
    bpl RunObjects13F9_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects13F9_at9
    jsr VecClearObject
    jmp RunObjects13F9_at1
RunObjects13F9_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects13F9_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects13F9_at8
    bpl RunObjects13F9_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects13F9_at8
    bra ScrollSpeedsFromBands
RunObjects13F9_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects13F9_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects13F9_at12
    lda a:OBJ_X_HI,x
    beq RunObjects13F9_at12
    bpl RunObjects13F9_at13
    cmp #$FF
    bne RunObjects13F9_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects13F9_at1
    bra RunObjects13F9_at12
RunObjects13F9_at13:
    cmp #$01
    bne RunObjects13F9_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects13F9_at1
RunObjects13F9_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects13F9_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects13F9_at14
    bra RunObjects13F9_at1
RunObjects13F9_at14:
    lda a:TimeSlowMode
    beq RunObjects13F9_at3
    cmp #$03
    bne RunObjects13F9_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects13F9_at1
    bra RunObjects13F9_at3
RunObjects13F9_at15:
    cmp #$02
    bne RunObjects13F9_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps213F9,y
    bne RunObjects13F9_at1
    bra RunObjects13F9_at3
RunObjects13F9_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps113F9,y
    bne RunObjects13F9_at1
RunObjects13F9_at3:
    jsr DispatchByKind12D9
RunObjects13F9_at1:
    inx
    cpx #$58
    bcs RunObjects13F9_at17
    jmp RichterVecStart
RunObjects13F9_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects13F9_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts13F9,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects13F9_at18
    stz a:ObjectCueIndex
RunObjects13F9_at18:
    plx
    rts

; @routine 1619:000 RunObjects1619
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 1619's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind0A99 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps21619 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps11619 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts1619 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects1619:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects1619_at0
    jmp RunObjects1619_at1
RunObjects1619_at0:
    lda a:OBJ_STATE,x
    bne RunObjects1619_at2
    jmp RunObjects1619_at3
RunObjects1619_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects1619_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects1619_at5
    dec z:zTemp00
RunObjects1619_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects1619_at6
    dec z:zTemp00
RunObjects1619_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects1619_at4:
    lda a:OBJ_STATE,x
    bne RunObjects1619_at7
    jmp RunObjects1619_at3
RunObjects1619_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects1619_at8
    lda a:OBJ_X_HI,x
    beq RunObjects1619_at9
    bpl RunObjects1619_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects1619_at9
    jsr VecClearObject
    jmp RunObjects1619_at1
RunObjects1619_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects1619_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects1619_at8
    bpl RunObjects1619_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects1619_at8
    bra ScrollSpeedsFromBands
RunObjects1619_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects1619_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects1619_at12
    lda a:OBJ_X_HI,x
    beq RunObjects1619_at12
    bpl RunObjects1619_at13
    cmp #$FF
    bne RunObjects1619_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects1619_at1
    bra RunObjects1619_at12
RunObjects1619_at13:
    cmp #$01
    bne RunObjects1619_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects1619_at1
RunObjects1619_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects1619_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects1619_at14
    bra RunObjects1619_at1
RunObjects1619_at14:
    lda a:TimeSlowMode
    beq RunObjects1619_at3
    cmp #$03
    bne RunObjects1619_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects1619_at1
    bra RunObjects1619_at3
RunObjects1619_at15:
    cmp #$02
    bne RunObjects1619_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps21619,y
    bne RunObjects1619_at1
    bra RunObjects1619_at3
RunObjects1619_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps11619,y
    bne RunObjects1619_at1
RunObjects1619_at3:
    jsr DispatchByKind16C9
RunObjects1619_at1:
    inx
    cpx #$58
    bcs RunObjects1619_at17
    jmp RichterVecStart
RunObjects1619_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects1619_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts1619,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects1619_at18
    stz a:ObjectCueIndex
RunObjects1619_at18:
    plx
    rts

; @routine 16C9:000 RunObjects16C9
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 16C9's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind16C9 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps216C9 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps116C9 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts16C9 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects16C9:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects16C9_at0
    jmp RunObjects16C9_at1
RunObjects16C9_at0:
    lda a:OBJ_STATE,x
    bne RunObjects16C9_at2
    jmp RunObjects16C9_at3
RunObjects16C9_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects16C9_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects16C9_at5
    dec z:zTemp00
RunObjects16C9_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects16C9_at6
    dec z:zTemp00
RunObjects16C9_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects16C9_at4:
    lda a:OBJ_STATE,x
    bne RunObjects16C9_at7
    jmp RunObjects16C9_at3
RunObjects16C9_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects16C9_at8
    lda a:OBJ_X_HI,x
    beq RunObjects16C9_at9
    bpl RunObjects16C9_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects16C9_at9
    jsr VecClearObject
    jmp RunObjects16C9_at1
RunObjects16C9_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects16C9_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects16C9_at8
    bpl RunObjects16C9_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects16C9_at8
    bra ScrollSpeedsFromBands
RunObjects16C9_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects16C9_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects16C9_at12
    lda a:OBJ_X_HI,x
    beq RunObjects16C9_at12
    bpl RunObjects16C9_at13
    cmp #$FF
    bne RunObjects16C9_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects16C9_at1
    bra RunObjects16C9_at12
RunObjects16C9_at13:
    cmp #$01
    bne RunObjects16C9_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects16C9_at1
RunObjects16C9_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects16C9_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects16C9_at14
    bra RunObjects16C9_at1
RunObjects16C9_at14:
    lda a:TimeSlowMode
    beq RunObjects16C9_at3
    cmp #$03
    bne RunObjects16C9_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects16C9_at1
    bra RunObjects16C9_at3
RunObjects16C9_at15:
    cmp #$02
    bne RunObjects16C9_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps216C9,y
    bne RunObjects16C9_at1
    bra RunObjects16C9_at3
RunObjects16C9_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps116C9,y
    bne RunObjects16C9_at1
RunObjects16C9_at3:
    jsr DispatchByKind16C9
RunObjects16C9_at1:
    inx
    cpx #$58
    bcs RunObjects16C9_at17
    jmp RichterVecStart
RunObjects16C9_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects16C9_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts16C9,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects16C9_at18
    stz a:ObjectCueIndex
RunObjects16C9_at18:
    plx
    rts

; @routine 18D9:000 RunObjects18D9
; @run 70:A000
; @stop A13C
; WHAT IT DOES: The sector 18D9's copy of RunObjects: the same code, its dispatch by kind through the table at the address DispatchByKind18D9 reads (this overlay's own).
; HOW: run_objects, the body RunObjects shares.
; WAITS: no.
; Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
SlowModeSteps218D9 = $A166 ; mode 2: whether an object steps, by its pace bits 5-6 and the frame
SlowModeSteps118D9 = $A146 ; mode 1: whether an object steps, by its pace bits 3-4 and the frame
ObjectCueScripts18D9 = $A13C ; the script the object cue starts, by ObjectCueIndex
RunObjects18D9:
    phx
    ldx #$10
    lda a:OBJ_KIND,x
    bne RunObjects18D9_at0
    jmp RunObjects18D9_at1
RunObjects18D9_at0:
    lda a:OBJ_STATE,x
    bne RunObjects18D9_at2
    jmp RunObjects18D9_at3
RunObjects18D9_at2:
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    bne RunObjects18D9_at4
    lda a:OBJ_SCROLL_BAND,x
    and #$7F
    tay
    stz z:zTemp00
    lda a:ScrollSpeedX,y
    bpl RunObjects18D9_at5
    dec z:zTemp00
RunObjects18D9_at5:
    lda a:OBJ_X_LO,x
    sec
    sbc a:ScrollSpeedX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp00
    sta a:OBJ_X_HI,x
    stz z:zTemp00
    lda a:ScrollSpeedY,y
    bpl RunObjects18D9_at6
    dec z:zTemp00
RunObjects18D9_at6:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:ScrollSpeedY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
RunObjects18D9_at4:
    lda a:OBJ_STATE,x
    bne RunObjects18D9_at7
    jmp RunObjects18D9_at3
RunObjects18D9_at7:
    lda a:OBJ_LOOP_FLAGS,x
    and #$02
    bne RunObjects18D9_at8
    lda a:OBJ_X_HI,x
    beq RunObjects18D9_at9
    bpl RunObjects18D9_at10
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs RunObjects18D9_at9
    jsr VecClearObject
    jmp RunObjects18D9_at1
RunObjects18D9_at10:
    lda a:OBJ_X_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects18D9_at9:
    lda a:OBJ_Y_HI,x
    beq RunObjects18D9_at8
    bpl RunObjects18D9_at11
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs RunObjects18D9_at8
    bra ScrollSpeedsFromBands
RunObjects18D9_at11:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs ScrollSpeedsFromBands
RunObjects18D9_at8:
    jsr ObjectHit
    lda a:OBJ_LOOP_FLAGS,x
    and #$04
    beq RunObjects18D9_at12
    lda a:OBJ_X_HI,x
    beq RunObjects18D9_at12
    bpl RunObjects18D9_at13
    cmp #$FF
    bne RunObjects18D9_at1
    lda a:OBJ_X_LO,x
    cmp #$C0
    bcc RunObjects18D9_at1
    bra RunObjects18D9_at12
RunObjects18D9_at13:
    cmp #$01
    bne RunObjects18D9_at1
    lda a:OBJ_X_LO,x
    cmp #$40
    bcs RunObjects18D9_at1
RunObjects18D9_at12:
    lda a:OBJ_HURT_TIMER,x
    beq RunObjects18D9_at14
    dec a:OBJ_HURT_TIMER,x
    lda a:OBJ_PACE,x
    and #$04
    bne RunObjects18D9_at14
    bra RunObjects18D9_at1
RunObjects18D9_at14:
    lda a:TimeSlowMode
    beq RunObjects18D9_at3
    cmp #$03
    bne RunObjects18D9_at15
    lda z:zWaveCounter
    and #$03
    bne RunObjects18D9_at1
    bra RunObjects18D9_at3
RunObjects18D9_at15:
    cmp #$02
    bne RunObjects18D9_at16
    lda a:OBJ_PACE,x
    and #$60
    lsr a
    lsr a
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps218D9,y
    bne RunObjects18D9_at1
    bra RunObjects18D9_at3
RunObjects18D9_at16:
    lda a:OBJ_PACE,x
    and #$18
    sta z:zTemp00
    lda z:zWaveCounter
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:SlowModeSteps118D9,y
    bne RunObjects18D9_at1
RunObjects18D9_at3:
    jsr DispatchByKind18D9
RunObjects18D9_at1:
    inx
    cpx #$58
    bcs RunObjects18D9_at17
    jmp RichterVecStart
RunObjects18D9_at17:
    lda z:zWaveCounter
    and #$03
    bne RunObjects18D9_at18
    ldy a:ObjectCueIndex
    lda a:ObjectCueScripts18D9,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:ObjectCueIndex
    lda a:ObjectCueIndex
    cmp #$0A
    bcc RunObjects18D9_at18
    stz a:ObjectCueIndex
RunObjects18D9_at18:
    plx
    rts
