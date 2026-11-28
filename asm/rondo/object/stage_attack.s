; @routine 024F:364 StageClearVramRowsOn
; @run 6B:9364
; WHAT IT DOES: Writes rows of $40 zero words to the VDC from where it points, zTemp00 rows.
; HOW: ST1 and ST2 of zero, DEC zTemp00 until it is zero (a count of 0 writes 256 rows).
; WAITS: no.
; Entry: zTemp00 rows, the VDC's write address set. Exit: X 0, N 0, Z 1. A, Y, C and V kept.
StageClearVramRowsOn:
    ldx #$40
StageClearVramRowsOn_at0:
    st1 #$00
    st2 #$00
    dex
    bne StageClearVramRowsOn_at0
    dec z:zTemp00
    bne StageClearVramRowsOn
    rts

; @routine 024F:55B StageHitReportLow
; @run 6B:955B
; WHAT IT DOES: Records an attack's hit: the side from OBJ_PACE bit 0 into the attack's report byte; unless it is 2,
; the hit report kept and the attack's power taken as the object's hit.
; HOW: AND, INC; the shared end at $9564.
; WAITS: no.
; Entry: X the object, Y the attack. Exit: side 2: A 2, Z 1, C 1; else A the power, N and Z from it. X, Y, V kept.
StageHitReportLow:
    lda a:OBJ_PACE,x
    and #$01
    inc a
    sta a:OBJ_ATTACK_SIDE,y
StageHitReportLow_at0:
    cmp #$02
    beq HitReportHandler_ret

; @routine 024F:587 StageHitReportHigh
; @run 6B:9587
; WHAT IT DOES: As StageHitReportLow with the side from OBJ_PACE bit 1.
; HOW: AND, LSR, INC; BRA to $9564.
; WAITS: no.
; Entry, exit: as StageHitReportLow.
StageHitReportHigh:
    lda a:OBJ_PACE,x
    and #$02
    lsr a
    inc a
    sta a:OBJ_ATTACK_SIDE,y
    bra StageHitReportLow_at0

; @routine 024F:593 StageHitReportTop
; @run 6B:9593
; WHAT IT DOES: As StageHitReportLow with the side from OBJ_PACE bit 7; side 2 gives the attack power 5 and is
; reported all the same.
; HOW: AND, two ROLs and AND (bit 7 to bit 0), INC; the report from $9568.
; WAITS: no.
; Entry: X the object, Y the attack. Exit: A the power, N and Z from it; C 0, or 1 when side 2. X, Y, V kept.
StageHitReportTop:
    lda a:OBJ_PACE,x
    and #$80
    rol a
    rol a
    and #$01
    inc a
    sta a:OBJ_ATTACK_SIDE,y
    cmp #$02
    bne HitReportHandler
    lda #$05
    sta a:OBJ_ATTACK_POWER,y
    bra HitReportHandler

; @routine 0250:1EF StageDoorTouched
; @run 6B:99EF
; WHAT IT DOES: A door the player can go through: while he is ready for it and the door is not held, a hit of 1 or 2
; takes him: the door's argument gives zField8C, the camera modes 1 and $18 reset, sub-step 3, the door's opening
; started; an argument with a high nibble also stops the raster effect and, but for $F0, starts a list on channel 6;
; one with a zero high nibble and 2 to 5 fades the CD.
; HOW: StageDoorHeld EOR 1 AND PlayerDoorReady; CPs of OBJ_HIT_TAKEN and OBJ_START_ARG; StartListChannelKind2 with X
; kept; the tail JMP to cd_fade.
; WAITS: no.
; Entry: X object. Exit: as the path it took leaves them, X kept.
StageDoorTouched:
    lda a:StageDoorHeld
    eor #$01
    and a:PlayerDoorReady
    beq StageDoorTouched_at0
    lda a:OBJ_HIT_TAKEN,x
    beq StageDoorTouched_at0
    cmp #$03
    bcs StageDoorTouched_at0
    lda a:OBJ_START_ARG,x
    and #$0F
    inc a
    sta z:zField8C
    lda a:CameraMode
    cmp #$01
    beq StageDoorTouched_at1
    cmp #$18
    bne StageDoorTouched_at2
StageDoorTouched_at1:
    stz a:CameraMode
StageDoorTouched_at2:
    lda #$03
    sta z:zSubStep
    inc a:OBJ_STATE,x
    stz a:OBJ_GATE_STEP,x
    lda #$04
    sta a:OBJ_GATE_WAIT,x
    lda a:OBJ_START_ARG,x
    and #$F0
    beq StageDoorTouched_at3
    stz z:zRasterEffect
    stz a:CameraMode
    cmp #$F0
    beq StageDoorTouched_at0
    lsr a
    lsr a
    lsr a
    lsr a
    ora #$80
    phx
    ldx #$06
    jsr VecStartListChannelKind2
    plx
StageDoorTouched_at0:
    rts
StageDoorTouched_at3:
    lda a:OBJ_START_ARG,x
    cmp #$02
    bcc StageDoorTouched_at0
    cmp #$06
    bcs StageDoorTouched_at0
    lda #$0C
    jmp BiosCdFade

