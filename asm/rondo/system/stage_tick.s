; @routine 024E:36D StepStageFrame
; @run 6B:8B6D
; @stop 8C19
; WHAT IT DOES: One tick of a stage: unless the stage's frame object is off, runs the opening
; sequence (StepStageSequence) or the three number blends, starts the wave's script, picks the
; frame the character shows, moves the health bar one step towards the health (with a sound each
; way), maps the stage banks and blends the two rows of the shown byte zField94 when it changed.
; HOW: The frame object is slot 87 of the frame column. The sequence runs when paused bit 1 is
; set or the count in StageVar3617 runs out; the health bar by SetHealthBarAddress with
; AndVramData or OrVramData; zField94 against zField95 by BlendStageTileRows at steps 8 and 9.
; With zField8E bit 2 set the code runs on into the next piece of the overlay, which has no C.
; Eleven stage overlays hold this code, the ones whose VRAM routines are at $9392 and $93A1.
; WAITS: no.
; Entry: X and Y as the blends take them. Exit: as the last step left it.
StepStageFrame:
    lda a:$2ABF
    beq $8B6C
    bbs1 zPaused,StepStageFrame_at0
    lda a:StageVar3616+1
    beq StepStageFrame_at1
    dec a:StageVar3616+1
    bne StepStageFrame_at1
    smb1 z:zPaused
StepStageFrame_at0:
    jsr StepStageSequence
    bra StepStageFrame_at2
StepStageFrame_at1:
    jsr BlendStageStockRows
    jsr BlendStageScoreAddRows
    jsr BlendStageScoreRows
StepStageFrame_at2:
    jsr StageScriptByWave
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8C19,x
    ldx z:zCharacter
    beq ScatterDebris
    cmp #$0D
    beq StepStageFrame_at3
    cmp #$11
    bne ScatterDebris
    ldx #$28
    stx a:$2D26
    lda #$71
    bra ScatterDebris
StepStageFrame_at3:
    lda #$72
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrame_at4
    bcs StepStageFrame_at5
    jsr SetHealthBarAddress
    ldx #$FF
    ldy #$F0
    jsr AndVramData
    dec z:zField99
    bra StepStageFrame_at6
StepStageFrame_at5:
    bbr5 zField8E,StepStageFrame_at7
    bbs6 zField8E,StepStageFrame_at7
    lda #$0F
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrame_at7:
    inc z:zField99
    bsr SetHealthBarAddress
    ldx #$00
    ldy #$0F
    jsr OrVramData
    bra StepStageFrame_at8
StepStageFrame_at4:
    smb5 z:zField8E
StepStageFrame_at6:
    bbr6 zField8E,StepStageFrame_at8
    rmb6 z:zField8E
    lda #$54
    jsr VecSoundCommand
StepStageFrame_at8:
    jsr MapStageBanks
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrame_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr BlendStageTileRows
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr BlendStageTileRows
StepStageFrame_at9:
    bbs2 zField8E,StepStageFrame_at10
    rts
StepStageFrame_at10:
    jmp $8C74

; @routine 024E:422 StageScriptByWave
; @run 6B:8C22
; WHAT IT DOES: Starts the stage's script in slot 1 that the wave counter names, when it is not
; the one already shown.
; HOW: The counter's low three bits, masked with RichterItemReady, index a table of eight script
; numbers at $8C3B; when the one found differs from StageVar3566's second byte, it goes there and
; StartScriptSlotKind4 starts it in slot 1.
; Every stage overlay holds the same code here.
; WAITS: no.
; Entry: nothing. Exit: unchanged script: A the script, X the table index, Z 1, C 1;
; else as StartScriptSlotKind4 with X 1, C as the compare left it.
StageScriptByWave:
    lda z:zWaveCounter
    and #$07
    and a:RichterItemReady
    tax
    lda a:$8C3B,x
    cmp a:StageVar3566+1
    beq StageScriptByWave_at0
    sta a:StageVar3566+1
    ldx #$01
    jmp VecStartScriptSlotKind4
StageScriptByWave_at0:
    rts

; @routine 024E:443 SetHealthBarAddress
; @run 6B:8C43
; WHAT IT DOES: Points the VDC's VRAM addresses at the health bar's cell for the health shown
; (zField99); nothing when none is shown.
; HOW: $5E minus the health shown gives a cell number; its high nibble times eight plus its low
; nibble is the offset, and the high byte is $65 plus the carry; SetVramAddresses does the rest.
; Eleven stage overlays hold this code, the ones whose VRAM routine is at $937D.
; WAITS: no.
; Entry: nothing. Exit: A 2, X and Y the address, N 0, Z 0, C and V from the high byte's add;
; with none shown A 0 and the rest kept.
SetHealthBarAddress:
    lda z:zField99
    beq SetHealthBarAddress_at0
    lda #$5E
    sec
    sbc z:zField99
    pha
    and #$0F
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
    tax
    lda z:zTemp01
    adc #$65
    tay
    jsr SetVramAddresses
SetHealthBarAddress_at0:
    rts

; @routine 05BE:36D StepStageFrameLong
; @run 6B:8B6D
; @stop 8C19
; WHAT IT DOES: StepStageFrame for the other stage group, whose VRAM routines are at $96A4 and
; $96B3 and whose bank mapping is at $965B.
; HOW: The same code, calling the Long routines.
; WAITS: no.
; Entry, exit: as StepStageFrame.
StepStageFrameLong:
    lda a:$2ABF
    beq $8B6C
    bbs1 zPaused,StepStageFrameLong_at0
    lda a:StageVar3616+1
    beq StepStageFrameLong_at1
    dec a:StageVar3616+1
    bne StepStageFrameLong_at1
    smb1 z:zPaused
StepStageFrameLong_at0:
    jsr StepStageSequence
    bra StepStageFrameLong_at2
StepStageFrameLong_at1:
    jsr BlendStockRows
    jsr BlendStageRowsOfPlayer
    jsr BlendStageScoreRowsLong
StepStageFrameLong_at2:
    jsr StageScriptByWave
    lda #$24
    sta a:$2D26
    ldx z:zField93
    lda a:$8C19,x
    ldx z:zCharacter
    beq ScatterDebris
    cmp #$0D
    beq StepStageFrameLong_at3
    cmp #$11
    bne ScatterDebris
    ldx #$28
    stx a:$2D26
    lda #$71
    bra ScatterDebris
StepStageFrameLong_at3:
    lda #$72
    sta a:$2ABE
    lda z:zPlayerHealth
    cmp z:zField99
    beq StepStageFrameLong_at4
    bcs StepStageFrameLong_at5
    jsr SetHealthBarAddress
    ldx #$FF
    ldy #$F0
    jsr AndVramDataLong
    dec z:zField99
    bra StepStageFrameLong_at6
StepStageFrameLong_at5:
    bbr5 zField8E,StepStageFrameLong_at7
    bbs6 zField8E,StepStageFrameLong_at7
    lda #$0F
    jsr VecSoundCommand
    smb6 z:zField8E
StepStageFrameLong_at7:
    inc z:zField99
    bsr SetHealthBarAddress
    ldx #$00
    ldy #$0F
    jsr OrVramDataLong
    bra StepStageFrameLong_at8
StepStageFrameLong_at4:
    smb5 z:zField8E
StepStageFrameLong_at6:
    bbr6 zField8E,StepStageFrameLong_at8
    rmb6 z:zField8E
    lda #$54
    jsr VecSoundCommand
StepStageFrameLong_at8:
    jsr MapStageBanksLong
    stz a:StageBlendOn
    lda z:zField94
    cmp z:zField95
    beq StepStageFrameLong_at9
    sta z:zField95
    and #$0F
    ldx #$08
    jsr BlendStageTileRows
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr BlendStageTileRows
StepStageFrameLong_at9:
    bbs2 zField8E,StepStageFrameLong_at10
    rts
StepStageFrameLong_at10:
    jmp $8C74

; @routine 09DE:443 SetHealthBarAddressLong
; @run 6B:8C43
; WHAT IT DOES: SetHealthBarAddress for the other stage group, whose VRAM routine is at $968F.
; HOW: The same code, calling SetVramAddressesLong.
; WAITS: no.
; Entry, exit: as SetHealthBarAddress.
SetHealthBarAddressLong:
    lda z:zField99
    beq SetHealthBarAddressLong_at0
    lda #$5E
    sec
    sbc z:zField99
    pha
    and #$0F
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
    tax
    lda z:zTemp01
    adc #$65
    tay
    jsr SetVramAddressesLong
SetHealthBarAddressLong_at0:
    rts
