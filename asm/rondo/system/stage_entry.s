; @routine 024E:152 LoadStageEntry
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecords = $9BC2 ; the section's entry record by zField83
LoadStageEntry:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecords,y
    sta z:zTemp00
    lda a:StageEntryRecords+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntry_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntry_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntry_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntry_at2
LoadStageEntry_at1:
    iny
LoadStageEntry_at2:
    rts

; @routine 050E:152 LoadStageEntryB
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9C71.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsB = $9C71 ; the section's entry record by zField83
LoadStageEntryB:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsB,y
    sta z:zTemp00
    lda a:StageEntryRecordsB+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryB_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryB_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryB_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryB_at2
LoadStageEntryB_at1:
    iny
LoadStageEntryB_at2:
    rts

; @routine 05BE:152 LoadStageEntryC
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9EAD.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsC = $9EAD ; the section's entry record by zField83
LoadStageEntryC:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsC,y
    sta z:zTemp00
    lda a:StageEntryRecordsC+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryC_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryC_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryC_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryC_at2
LoadStageEntryC_at1:
    iny
LoadStageEntryC_at2:
    rts

; @routine 071E:152 LoadStageEntryD
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9F70.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsD = $9F70 ; the section's entry record by zField83
LoadStageEntryD:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsD,y
    sta z:zTemp00
    lda a:StageEntryRecordsD+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryD_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryD_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryD_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryD_at2
LoadStageEntryD_at1:
    iny
LoadStageEntryD_at2:
    rts

; @routine 07CE:152 LoadStageEntryE
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9D29.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsE = $9D29 ; the section's entry record by zField83
LoadStageEntryE:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsE,y
    sta z:zTemp00
    lda a:StageEntryRecordsE+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryE_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryE_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryE_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryE_at2
LoadStageEntryE_at1:
    iny
LoadStageEntryE_at2:
    rts

; @routine 087E:152 LoadStageEntryF
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9E70.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsF = $9E70 ; the section's entry record by zField83
LoadStageEntryF:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsF,y
    sta z:zTemp00
    lda a:StageEntryRecordsF+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryF_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryF_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryF_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryF_at2
LoadStageEntryF_at1:
    iny
LoadStageEntryF_at2:
    rts

; @routine 092E:152 LoadStageEntryG
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9D6C.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsG = $9D6C ; the section's entry record by zField83
LoadStageEntryG:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsG,y
    sta z:zTemp00
    lda a:StageEntryRecordsG+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryG_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryG_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryG_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryG_at2
LoadStageEntryG_at1:
    iny
LoadStageEntryG_at2:
    rts

; @routine 09DE:152 LoadStageEntryH
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9EB6.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsH = $9EB6 ; the section's entry record by zField83
LoadStageEntryH:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsH,y
    sta z:zTemp00
    lda a:StageEntryRecordsH+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryH_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryH_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryH_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryH_at2
LoadStageEntryH_at1:
    iny
LoadStageEntryH_at2:
    rts

; @routine 0A8E:152 LoadStageEntryI
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9D50.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsI = $9D50 ; the section's entry record by zField83
LoadStageEntryI:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsI,y
    sta z:zTemp00
    lda a:StageEntryRecordsI+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryI_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryI_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryI_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryI_at2
LoadStageEntryI_at1:
    iny
LoadStageEntryI_at2:
    rts

; @routine 0B3E:152 LoadStageEntryJ
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9E81.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsJ = $9E81 ; the section's entry record by zField83
LoadStageEntryJ:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsJ,y
    sta z:zTemp00
    lda a:StageEntryRecordsJ+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryJ_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryJ_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryJ_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryJ_at2
LoadStageEntryJ_at1:
    iny
LoadStageEntryJ_at2:
    rts

; @routine 0BEE:152 LoadStageEntryK
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9D05.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsK = $9D05 ; the section's entry record by zField83
LoadStageEntryK:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsK,y
    sta z:zTemp00
    lda a:StageEntryRecordsK+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryK_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryK_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryK_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryK_at2
LoadStageEntryK_at1:
    iny
LoadStageEntryK_at2:
    rts

; @routine 0C9E:152 LoadStageEntryL
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9E84.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsL = $9E84 ; the section's entry record by zField83
LoadStageEntryL:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsL,y
    sta z:zTemp00
    lda a:StageEntryRecordsL+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryL_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryL_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryL_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryL_at2
LoadStageEntryL_at1:
    iny
LoadStageEntryL_at2:
    rts

; @routine 0D4E:152 LoadStageEntryM
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9D41.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsM = $9D41 ; the section's entry record by zField83
LoadStageEntryM:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsM,y
    sta z:zTemp00
    lda a:StageEntryRecordsM+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryM_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryM_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryM_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryM_at2
LoadStageEntryM_at1:
    iny
LoadStageEntryM_at2:
    rts

; @routine 0DFE:152 LoadStageEntryN
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9EB3.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsN = $9EB3 ; the section's entry record by zField83
LoadStageEntryN:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsN,y
    sta z:zTemp00
    lda a:StageEntryRecordsN+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryN_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryN_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryN_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryN_at2
LoadStageEntryN_at1:
    iny
LoadStageEntryN_at2:
    rts

; @routine 0EAE:152 LoadStageEntryO
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9E76.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsO = $9E76 ; the section's entry record by zField83
LoadStageEntryO:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsO,y
    sta z:zTemp00
    lda a:StageEntryRecordsO+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryO_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryO_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryO_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryO_at2
LoadStageEntryO_at1:
    iny
LoadStageEntryO_at2:
    rts

; @routine 0F5E:152 LoadStageEntryP
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9F27.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsP = $9F27 ; the section's entry record by zField83
LoadStageEntryP:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsP,y
    sta z:zTemp00
    lda a:StageEntryRecordsP+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryP_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryP_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryP_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryP_at2
LoadStageEntryP_at1:
    iny
LoadStageEntryP_at2:
    rts

; @routine 100E:152 LoadStageEntryQ
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9CBA.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsQ = $9CBA ; the section's entry record by zField83
LoadStageEntryQ:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsQ,y
    sta z:zTemp00
    lda a:StageEntryRecordsQ+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryQ_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryQ_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryQ_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryQ_at2
LoadStageEntryQ_at1:
    iny
LoadStageEntryQ_at2:
    rts

; @routine 10BE:152 LoadStageEntryR
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9F1A.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsR = $9F1A ; the section's entry record by zField83
LoadStageEntryR:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsR,y
    sta z:zTemp00
    lda a:StageEntryRecordsR+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryR_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryR_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryR_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryR_at2
LoadStageEntryR_at1:
    iny
LoadStageEntryR_at2:
    rts

; @routine 116E:152 LoadStageEntryS
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9CD4.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsS = $9CD4 ; the section's entry record by zField83
LoadStageEntryS:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsS,y
    sta z:zTemp00
    lda a:StageEntryRecordsS+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryS_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryS_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryS_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryS_at2
LoadStageEntryS_at1:
    iny
LoadStageEntryS_at2:
    rts

; @routine 12CE:152 LoadStageEntryT
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9F5E.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsT = $9F5E ; the section's entry record by zField83
LoadStageEntryT:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsT,y
    sta z:zTemp00
    lda a:StageEntryRecordsT+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryT_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryT_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryT_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryT_at2
LoadStageEntryT_at1:
    iny
LoadStageEntryT_at2:
    rts

; @routine 160E:152 LoadStageEntryU
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9CE6.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsU = $9CE6 ; the section's entry record by zField83
LoadStageEntryU:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsU,y
    sta z:zTemp00
    lda a:StageEntryRecordsU+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryU_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryU_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryU_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryU_at2
LoadStageEntryU_at1:
    iny
LoadStageEntryU_at2:
    rts

; @routine 16BE:152 LoadStageEntryV
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9F4D.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsV = $9F4D ; the section's entry record by zField83
LoadStageEntryV:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsV,y
    sta z:zTemp00
    lda a:StageEntryRecordsV+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryV_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryV_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryV_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryV_at2
LoadStageEntryV_at1:
    iny
LoadStageEntryV_at2:
    rts

; @routine 18CE:152 LoadStageEntryW
; @run 6B:8952
; WHAT IT DOES: Loads the stage section's entry record: the scroll slots' screen and position,
; the camera mode and limits, the player's start position and the raster effect.
; HOW: The record is a pointer from the overlay's table by zField83; its bytes go to the scroll
; state, SCROLLLIM and the player's slot in the order the record keeps them.
; The same code as LoadStageEntry; this overlay's table of section records is at $9EC1.
; WAITS: no.
; Entry: zField83 the section. Exit: A the record's last byte, Y the record's length so far, N and Z from Y.
; Clobbers zTemp00-01.
StageEntryRecordsW = $9EC1 ; the section's entry record by zField83
LoadStageEntryW:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageEntryRecordsW,y
    sta z:zTemp00
    lda a:StageEntryRecordsW+1,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    sta a:ScrollXHiScreen+4
    sta a:ScrollXHiScreen+5
    sta a:ScrollXHiScreen+6
    sta a:ScrollXHiScreen+7
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    sta a:ScrollXHi+4
    sta a:ScrollXHi+5
    sta a:ScrollXHi+6
    sta a:ScrollXHi+7
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:Scroll3E66
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,LoadStageEntryW_at0
    rmb1 z:zField8E
    iny
    iny
    bra AnimateAndCueNine
LoadStageEntryW_at0:
    lda (zTemp00),y
    bne SoundModulate
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra EndingFrameList
    iny
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_X_HI
    lda z:zCharacter
    beq AnimateAndCueNine
    lda a:OBJ_STATE
    cmp #$05
    bcc AnimateAndCueNine
    sec
    sbc #$05
    cmp #$04
    bcs AnimateAndCueNine
    tax
    lda a:OBJ_X_LO
    clc
    adc a:$8A32,x
    sta a:OBJ_X_LO
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi LoadStageEntryW_at1
    iny
    sta a:OBJ_FACING_LEFT
    bra LoadStageEntryW_at2
LoadStageEntryW_at1:
    iny
LoadStageEntryW_at2:
    rts
