; @routine 05BE:46C RunStageGauge
; @run 6B:8C6C
; WHAT IT DOES: The gauge object's frame: spawns row 2 until the object's frame (slot 85) is set, clears the
; gauge value then, lets the object's X low byte down by one unless it is $D8, and then the timer check.
; HOW: SpawnStageObjectLong for row 2 in a loop; the rest as above.
; Ten stage overlays hold this code; the stage's frame code reaches it by a jump.
; WAITS: no.
; Entry: nothing. Exit: as the step the timer check took.
RunStageGauge:
    ldx #$02
    jsr SpawnStageObjectLong
    stz a:Field2742
RunStageGauge_at0:
    lda a:$2ABD
    beq RunStageGauge
    lda a:$2C1D
    cmp #$D8
    beq RunStageGauge_at1
    dec a:$2C1D
RunStageGauge_at1:
    lda a:$350D
    beq GaugeStep
    jmp $8D25

; @routine 05BE:48B GaugeStep
; @run 6B:8C8B
; WHAT IT DOES: Moves the gauge one step towards its target (slot 16 of the f32a8 column): up when the target
; is above the value, down when below, nothing when equal.
; HOW: Compares the target with Field2742; GaugeStepUp for up; for down SetGaugeAddress, AndVramDataLong with
; $0F and $FF and the value down by one. A negative target is held, the timer check run and the
; target put back (untraced).
; Ten stage overlays hold this code.
; WAITS: no.
; Entry: nothing. Exit: as the step taken.
GaugeStep:
    lda a:$32B8
    bmi GaugeStep_at0
    cmp a:Field2742
    beq GaugeStepUp_at0
    bcs GaugeStepUp
    bsr SetGaugeAddress
    ldx #$0F
    ldy #$FF
    jsr AndVramDataLong
    dec a:Field2742
    rts
GaugeStep_at0:
    pha
    stz a:$32B8
    bsr RunStageGauge_at1
    pla
    sta a:$32B8
    ldy a:Field2742
    bne GaugeStepUp_at0
    sta a:$350D
    rts

; @routine 05BE:4B7 GaugeStepUp
; @run 6B:8CB7
; WHAT IT DOES: Grows the gauge one: its value up by one, its cell found, and its bits set.
; HOW: SetGaugeAddress, then OrVramDataLong with $F0 and 0.
; Ten stage overlays hold this code.
; WAITS: no.
; Entry: nothing. Exit: A the high byte written, N and Z from it, X $F0, Y 0.
GaugeStepUp:
    inc a:Field2742
    bsr SetGaugeAddress
    ldx #$F0
    ldy #$00
    jsr OrVramDataLong
GaugeStepUp_at0:
    rts

; @routine 05BE:4C4 SetGaugeAddress
; @run 6B:8CC4
; WHAT IT DOES: Points the VDC at the gauge's cell for its value (Field2742): none when the value is zero.
; HOW: $5E minus the value gives a cell number; its low nibble plus $40 and its high nibble times eight make
; an offset from $7D00, plus $C0 for the second half; SetVramAddressesLong does the rest.
; Ten stage overlays hold this code, the ones whose VRAM routine is at $968F.
; WAITS: no.
; Entry: nothing. Exit: A 2, X and Y the address, N 0, Z 0, C and V from the high add; with a zero
; value A 0 and the rest kept.
SetGaugeAddress:
    lda a:Field2742
    beq GaugeStepUp_at0
    stz z:zTemp08
    stz z:zTemp09
    cmp #$3F
    bcc SetGaugeAddress_at0
    lda #$C0
    sta z:zTemp08
    inc z:zTemp09
SetGaugeAddress_at0:
    lda #$5E
    sec
    sbc a:Field2742
    pha
    and #$0F
    clc
    adc #$40
    sta z:zTemp00
    pla
    and #$F0
    stz z:zTemp01
    asl a
    rol z:zTemp01
    asl a
    rol z:zTemp01
    asl a
    rol z:zTemp01
    clc
    adc z:zTemp00
    sta z:zTemp00
    lda z:zTemp01
    adc #$7D
    sta z:zTemp01
    lda z:zTemp08
    clc
    adc z:zTemp00
    tax
    lda z:zTemp09
    adc z:zTemp01
    tay
    jsr SetVramAddressesLong
    rts

; @routine 024E:483 GaugeTimerCheckB
; @run 6B:8C83
; @stop 8C8B
; WHAT IT DOES: While the life timer of slot 85 is zero, the gauge steps (GaugeStepB); else the flash at $8D25.
; HOW: The shared body with the VRAM routines at $937D, $9392 and $93A1.
; WAITS: no.
; Entry: nothing. Exit: as the step or the flash taken.
GaugeTimerCheckB:
    lda a:BossGaugeEnding
    beq GaugeStepB
    jmp BossGaugeFlash

; @routine 024E:48B GaugeStepB
; @run 6B:8C8B
; @stop 8CB7
; WHAT IT DOES: Moves the gauge one step towards its target, as GaugeStep with the VRAM routines at $937D to $93A1.
; HOW: The shared body with the VRAM routines at $937D, $9392 and $93A1.
; WAITS: no.
; Entry: nothing. Exit: as GaugeStep.
GaugeStepB:
    lda a:OBJ_HEALTH+16
    bmi GaugeStepB_at0
    cmp a:BossGaugeShown
    beq GaugeStepUpB_at0
    bcs GaugeStepUpB
    bsr SetGaugeAddressB
    ldx #$0F
    ldy #$FF
    jsr AndVramData
    dec a:BossGaugeShown
    rts
GaugeStepB_at0:
    pha
    stz a:OBJ_HEALTH+16
    bsr GaugeTimerCheckB
    pla
    sta a:OBJ_HEALTH+16
    ldy a:BossGaugeShown
    bne GaugeStepUpB_at0
    sta a:BossGaugeEnding
    rts

; @routine 024E:4B7 GaugeStepUpB
; @run 6B:8CB7
; @stop 8CC4
; WHAT IT DOES: Grows the gauge one, as GaugeStepUp with the VRAM routines at $937D to $93A1.
; HOW: The shared body with the VRAM routines at $937D, $9392 and $93A1.
; WAITS: no.
; Entry: nothing. Exit: as GaugeStepUp.
GaugeStepUpB:
    inc a:BossGaugeShown
    bsr SetGaugeAddressB
    ldx #$F0
    ldy #$00
    jsr OrVramData
GaugeStepUpB_at0:
    rts

; @routine 024E:4C4 SetGaugeAddressB
; @run 6B:8CC4
; WHAT IT DOES: Points the VDC at the gauge's cell for its value, as SetGaugeAddress with the VRAM routine at $937D.
; HOW: The shared body with the VRAM routines at $937D, $9392 and $93A1.
; WAITS: no.
; Entry: nothing. Exit: as SetGaugeAddress.
SetGaugeAddressB:
    lda a:BossGaugeShown
    beq GaugeStepUpB_at0
    stz z:zTemp08
    stz z:zTemp09
    cmp #$3F
    bcc SetGaugeAddressB_at0
    lda #$C0
    sta z:zTemp08
    inc z:zTemp09
SetGaugeAddressB_at0:
    lda #$5E
    sec
    sbc a:BossGaugeShown
    pha
    and #$0F
    clc
    adc #$40
    sta z:zTemp00
    pla
    and #$F0
    stz z:zTemp01
    asl a
    rol z:zTemp01
    asl a
    rol z:zTemp01
    asl a
    rol z:zTemp01
    clc
    adc z:zTemp00
    sta z:zTemp00
    lda z:zTemp01
    adc #$7D
    sta z:zTemp01
    lda z:zTemp08
    clc
    adc z:zTemp00
    tax
    lda z:zTemp09
    adc z:zTemp01
    tay
    jsr SetVramAddresses
SetGaugeAddressB_ret:
    rts

; @routine 024E:46C RunStageGaugeB
; @run 6B:8C6C
; @stop 8C74
; WHAT IT DOES: The gauge object's frame, as RunStageGauge in the build whose spawn is SpawnStageObject.
; HOW: SpawnStageObject for row 2, the gauge value cleared, running into RunStageGaugeFromFrameB.
; WAITS: no.
; Entry: nothing. Exit: as the step the timer check took.
RunStageGaugeB:
    ldx #$02
    jsr SpawnStageObject
    stz a:BossGaugeShown

; @routine 024E:474 RunStageGaugeFromFrameB
; @run 6B:8C74
; @stop 8C83
; WHAT IT DOES: The gauge's frame where StepStageFrame enters it: without the object's frame, RunStageGaugeB; else
; its X low byte one down unless it is $D8, and the timer check (GaugeTimerCheckB).
; HOW: A test of the frame, a compare and a DEC, running into GaugeTimerCheckB.
; WAITS: no.
; Entry: nothing. Exit: as the step the timer check took.
RunStageGaugeFromFrameB:
    lda a:OBJ_FRAME+85
    beq RunStageGaugeB
    lda a:OBJ_X_LO+85
    cmp #$D8
    beq GaugeTimerCheckB
    dec a:OBJ_X_LO+85
