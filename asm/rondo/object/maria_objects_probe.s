; @routine 01E1:443 MariaStagePointFrame
; @run 6E:C443
; WHAT IT DOES: A frame of her stage's point row (step 2 of the state that runs it): the columns under her, the floor
; and the landing, then MariaStagePointRow; when its count in OBJ_WORK_J runs out, zField89 is set to 1.
; HOW: MariaReadColumns, MariaFloorCheck, MariaLandOnFloor; a BSR to MariaStagePointRow, whose Z says the count ran
; out.
; WAITS: no.
; Entry: nothing. Exit: as MariaStagePointRow; at the end A 1, N and Z from it.
MariaStagePointFrame:
    jsr MariaReadColumns
    jsr MariaFloorCheck
    jsr MariaLandOnFloor
    bsr MariaStagePointRow
    beq MariaStagePointFrame_at0
    rts
MariaStagePointFrame_at0:
    lda #$01
    sta z:zField89
    rts

; @routine 01E1:456 MariaStagePointRow
; @run 6E:C456
; WHAT IT DOES: Gives the stage overlay's entry at $8009 six points $40 apart from $6080, each with her OBJ_WORK_K in
; A; whatever the stage's code leaves in zTemp0F is her OBJ_WORK_K after; her OBJ_WORK_J counts one down.
; HOW: MariaStagePointStep six times (X 6 down to 0).
; WAITS: no.
; Entry: as the stage's code takes them. Exit: A zTemp0F, N and Z from the new OBJ_WORK_J, X 0; C and V from the
; last point's add; Y as the stage's code left it.
MariaStagePointRow:
    lda #$60
    sta z:zTemp09
    lda #$80
    sta z:zTemp08
    ldx #$06
MariaStagePointRow_at0:
    lda a:OBJ_WORK_K
    sta z:zTemp0F
    jsr MariaStagePointStep
    bne MariaStagePointRow_at0
    lda z:zTemp0F
    sta a:OBJ_WORK_K
    dec a:OBJ_WORK_J
    rts

; @routine 01E1:473 MariaStagePointStep
; @run 6E:C473
; WHAT IT DOES: Gives the stage overlay's entry at $8009 the point in zTemp08-09, then moves the point $40 on and
; counts X down.
; HOW: The JSR to $8009 (the C of the routine resident there), a 16-bit add of $0040, DEX.
; WAITS: no.
; Entry: X the count, the rest as the stage's code takes them. Exit: A the point's high byte, C and V from its add, X
; one less, N and Z from it; Y as the stage's code left it.
MariaStagePointStep:
    jsr FrameOverlay8009
    lda z:zTemp08
    clc
    adc #$40
    sta z:zTemp08
    cla
    adc z:zTemp09
    sta z:zTemp09
    dex
    rts

; @routine 01E2:431 MariaObjectProbeBelow
; @run 6E:CC31
; WHAT IT DOES: Reads the terrain A rows below an object: bits 0 and 4 of the answer, or nothing for an answer with
; bit 7 set.
; HOW: The offset to zTemp06 (zTemp04, zTemp05 and zTemp07 zero), MariaObjectProbe, then AND #$11, or CLA on N set.
; WAITS: no.
; Entry: A rows down, X the object. Exit: A the bits, N and Z from them; or A 0 with N set; the rest as
; MariaObjectProbe left them.
MariaObjectProbeBelow:
    sta z:zTemp06
    stz z:zTemp07
    stz z:zTemp04
    stz z:zTemp05
    jsr MariaObjectProbe
    bmi MariaObjectProbeBelow_at0
    and #$11
    rts
MariaObjectProbeBelow_at0:
    cla
    rts

; @routine 01E2:443 MariaObjectProbeBelowAny
; @run 6E:CC43
; WHAT IT DOES: Reads the terrain A rows below an object: bits 0, 4 and 5 of the answer.
; HOW: The offset to zTemp06 (zTemp04, zTemp05 and zTemp07 zero), MariaObjectProbe, then AND #$31.
; WAITS: no.
; Entry: A rows down, X the object. Exit: A the bits, N and Z from them; the rest as MariaObjectProbe left them.
MariaObjectProbeBelowAny:
    sta z:zTemp06
    stz z:zTemp07
    stz z:zTemp04
    stz z:zTemp05
    jsr MariaObjectProbe
    and #$31
    rts

; @routine 01E2:451 MariaObjectProbe
; @run 6E:CC51
; WHAT IT DOES: Reads the terrain at a point offset from an object: zTemp04-05 across, zTemp06-07 down. A point below
; the bottom of the first screen down reads as nothing.
; HOW: The point to zTemp00-03 (its Y first; a carry out of the low byte is the bottom), then the tail JMP to the
; stage overlay's read at $8006 (StageTileRead).
; WAITS: no.
; Entry: X the object, zTemp04-07 the offset. Exit: past the bottom, A 0, N and Z from the Y add, C set, V from it.
; Else as the stage's read left the processor.
MariaObjectProbe:
    lda a:OBJ_Y_LO,x
    clc
    adc z:zTemp06
    sta z:zTemp02
    bcs MariaObjectProbe_at0
    lda a:OBJ_Y_HI,x
    adc z:zTemp07
    sta z:zTemp03
    lda a:OBJ_X_LO,x
    clc
    adc z:zTemp04
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc z:zTemp05
    sta z:zTemp01
    jmp StageTerrainRead
MariaObjectProbe_at0:
    cla
    rts

; @routine 01E2:476 MariaObjectProbeAtBit1
; @run 6E:CC76
; WHAT IT DOES: MariaObjectProbeAt's answer, bit 1 only.
; HOW: A BSR to MariaObjectProbeAt, then AND #$02.
; WAITS: no.
; Entry: as MariaObjectProbeAt. Exit: A the bit, N and Z from it; the rest as MariaObjectProbe left them.
MariaObjectProbeAtBit1:
    bsr MariaObjectProbeAt
    and #$02
    rts

; @routine 01E2:47B MariaObjectProbeAtSolid
; @run 6E:CC7B
; WHAT IT DOES: MariaObjectProbeAt's answer, bits 0 and 4 only.
; HOW: A BSR to MariaObjectProbeAt, then AND #$11.
; WAITS: no.
; Entry: as MariaObjectProbeAt. Exit: A the bits, N and Z from them; the rest as MariaObjectProbe left them.
MariaObjectProbeAtSolid:
    bsr MariaObjectProbeAt
    and #$11
    rts

; @routine 01E2:480 MariaObjectProbeAt
; @run 6E:CC80
; WHAT IT DOES: Reads the terrain at a point offset from an object by A across and Y down, each a signed byte.
; HOW: The offsets to zTemp04 and zTemp06 (read back for their signs, which make zTemp05 and zTemp07), then the tail
; JMP to MariaObjectProbe.
; WAITS: no.
; Entry: A across, Y down, X the object. Exit: as MariaObjectProbe.
MariaObjectProbeAt:
    stz z:zTemp07
    stz z:zTemp05
    sta z:zTemp04
    lda z:zTemp04
    bpl MariaObjectProbeAt_at0
    dec z:zTemp05
MariaObjectProbeAt_at0:
    sty z:zTemp06
    ldy z:zTemp06
    bpl MariaObjectProbeAt_at1
    dec z:zTemp07
MariaObjectProbeAt_at1:
    jmp MariaObjectProbe
