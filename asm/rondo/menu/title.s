; @routine 00C1:084 TitleScrollIn
; @run 6B:8084
; WHAT IT DOES: Scrolls the title in from the right, 8 pixels a frame, with the cycling tiles;
; at the end, three blinks to come.
; HOW: TitleTileRows; layer 1's X scroll, its high byte 2; zWaitTimer2 3 and the next sub-mode.
; Entry: nothing. Exit: A the scroll's low byte, N and Z from it, C and V from its subtract; at
; the end, A 3, N and Z from the new sub-mode. X and Y kept.
TitleScrollIn:
    jsr TitleTileRows
    lda #$02
    sta a:ScrollXHi+1
    lda a:ScrollXLo+1
    sec
    sbc #$08
    sta a:ScrollXLo+1
    bne TitleScrollIn_at0
    lda #$03
    sta z:zWaitTimer2
    inc z:zGameSubMode
TitleScrollIn_at0:
    rts

; @routine 00C1:09E TitleBlink
; @run 6B:809E
; WHAT IT DOES: Every fourth frame, rotates palette 3 from colour 2, counting the blinks down to
; the next sub-mode.
; HOW: Bits 0 and 1 of zFrameTick; TitlePaletteRotate; zWaitTimer2.
; Entry: nothing. Exit: between, N and V the frame count's bits 7 and 6, Z clear. Otherwise as
; TitlePaletteRotate, with N and Z from the count, or from the new sub-mode.
TitleBlink:
    tst #$03,z:zFrameTick
    bne TitleBlink_at0
    ldx #$32
    jsr TitlePaletteRotate
    dec z:zWaitTimer2
    bne TitleBlink_at0
    inc z:zGameSubMode
TitleBlink_at0:
    rts

; @routine 00C1:0AF TitleWipe
; @run 6B:80AF
; WHAT IT DOES: Moves the raster wipe's line down 2 a frame; past $E0 the effect is off, and on
; reaching 0 a frame's wait and the next sub-mode.
; HOW: zRasterArg; zRasterEffect cleared; zWaitTimer4.
; Entry: nothing. Exit: A the line, N, Z and C from comparing it with $E0; at 0, A 1, N and Z from
; the new sub-mode, C set. V from the add. X and Y kept.
TitleWipe:
    lda z:zRasterArg
    clc
    adc #$02
    sta z:zRasterArg
    beq TitleWipe_at0
    cmp #$E0
    bcc TitleWipe_at1
    stz z:zRasterEffect
    rts
TitleWipe_at0:
    lda #$01
    sta z:zWaitTimer4
    inc z:zGameSubMode
TitleWipe_at1:
    rts

; @routine 00C1:0C6 TitleLogoStart
; @run 6B:80C6
; WHAT IT DOES: Starts the logo: three waits of 10, script 1 on slot 0, and music $55.
; HOW: zWaitTimer, zWaitTimer2 and zWaitTimer3; StartScriptSlotKind1; SoundCommand.
; Entry: nothing. Exit: as SoundCommand after the script's start, with N and Z from the new
; sub-mode.
TitleLogoStart:
    lda #$0A
    sta z:zWaitTimer
    sta z:zWaitTimer2
    sta z:zWaitTimer3
    lda #$01
    jsr VecStartScriptSlot0Kind1
    lda #$55
    jsr VecSoundCommand
    inc z:zGameSubMode
    rts

; @routine 00C1:0DB TitleLogoFade
; @run 6B:80DB
; WHAT IT DOES: The logo's palettes: while zWaitTimer lasts, palette 1 rotates from colour 6;
; while zWaitTimer2 is not negative, palette 2 from colour 6 and three frames' hold; after a hold,
; script 2 on slot 0; then every other frame palette 0 from colour 5, until zWaitTimer3 goes
; negative, a frame's wait and the next sub-mode.
; HOW: TitlePaletteRotate by the waits; zWaitTimer4 the hold.
; Entry: nothing. Exit: as the last of those steps left the registers.
TitleLogoFade:
    lda z:zWaitTimer
    beq TitleLogoFade_at0
    dec z:zWaitTimer
    ldx #$16
    jsr TitlePaletteRotate
TitleLogoFade_at0:
    lda z:zWaitTimer2
    bmi TitleLogoFade_at1
    ldx #$26
    jsr TitlePaletteRotate
    dec z:zWaitTimer2
    lda #$03
    sta z:zWaitTimer4
    rts
TitleLogoFade_at1:
    lda z:zWaitTimer4
    beq TitleLogoFade_at2
    dec z:zWaitTimer4
    bne TitleLogoFade_at3
    lda #$02
    jmp VecStartScriptSlot0Kind1
TitleLogoFade_at2:
    bbs7 zWaitTimer3,TitleWipe_at0
    bbs0 zFrameTick,TitleLogoFade_at3
    dec z:zWaitTimer3
    ldx #$05
    jsr TitlePaletteRotate
TitleLogoFade_at3:
    rts

; @routine 00C1:111 TitleCycle
; @run 6B:8111
; WHAT IT DOES: Every fourth frame, moves colour 0 of palettes 0, 1 and 2 on by $49 together,
; until its low three bits are all set; then zWaitTimer4 $FF and the next sub-mode.
; HOW: zWaitTimer4 counts the frames; the colour read from the VCE, summed in zTemp00 and 01, and
; written to colours 0, $10 and $20.
; Entry: nothing. Exit: between, N and Z from the count's low bits, A them. Otherwise A the
; colour's low three bits, N, Z and C from comparing them with 7, V from the high byte's add; at
; the end, A $FF, N and Z from the new sub-mode, C set. X and Y kept.
TitleCycle:
    inc z:zWaitTimer4
    lda z:zWaitTimer4
    and #$03
    bne TitleCycle_at0
    stz a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    lda a:VCE_DATA_LO
    clc
    adc #$49
    sta z:zTemp00
    cla
    adc a:VCE_DATA_HI
    sta z:zTemp01
    stz a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    lda z:zTemp00
    sta a:VCE_DATA_LO
    lda z:zTemp01
    sta a:VCE_DATA_HI
    lda #$10
    sta a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    lda z:zTemp00
    sta a:VCE_DATA_LO
    lda z:zTemp01
    sta a:VCE_DATA_HI
    lda #$20
    sta a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    lda z:zTemp00
    sta a:VCE_DATA_LO
    lda z:zTemp01
    sta a:VCE_DATA_HI
    lda z:zTemp00
    and #$07
    cmp #$07
    bne TitleCycle_at0
    lda #$FF
    sta z:zWaitTimer4
    inc z:zGameSubMode
TitleCycle_at0:
    rts

; @routine 00C1:170 TitleLastScript
; @run 6B:8170
; WHAT IT DOES: Starts script 0 on slot 0 as kind 2, with $20 frames to wait.
; HOW: zWaitTimer4; the next sub-mode; StartScriptSlotKind2.
; Entry: nothing. Exit: as StartScriptSlot0Kind2 with script 0: A, X and Y 0, Z set.
TitleLastScript:
    lda #$20
    sta z:zWaitTimer4
    inc z:zGameSubMode
    cla
    jmp VecStartScriptSlot0Kind2

; @routine 00C1:17A TitleEnd
; @run 6B:817A
; WHAT IT DOES: The title's end: marks it seen, so Run skips it next time, and finishes the mode.
; HOW: TitleSeen 1, zModeDone counted up.
; Entry: nothing. Exit: A 1, N and Z from zModeDone. X, Y, C and V kept.
TitleEnd:
    lda #$01
    sta a:TitleSeen
    inc z:zModeDone
    rts

; @routine 00C1:182 TitleWait
; @run 6B:8182
; WHAT IT DOES: Waits out zWaitTimer4, then the next sub-mode.
; HOW: Counted down a frame.
; Entry: nothing. Exit: N and Z from the timer, or from the new sub-mode. A, X, Y, C, V kept.
TitleWait:
    dec z:zWaitTimer4
    bne TitleWait_at0
    inc z:zGameSubMode
TitleWait_at0:
    rts

; @routine 00C1:189 TitlePaletteRotate
; @run 6B:8189
; WHAT IT DOES: Rotates part of a 16-colour palette by one: from the colour given to the last,
; each takes the next one's place and the first goes to the end.
; HOW: The palette read from the VCE into PaletteWork, the first colour kept in PaletteWorkSpare,
; the rest moved down, then the 32 bytes written back.
; Entry: X the palette in its high nibble, the first colour in its low. Exit: A the last byte
; moved, X $22, Z and C set, N clear. Y and V kept.
TitlePaletteRotate:
    txa
    and #$F0
    sta a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    tai VCE_DATA_LO,PaletteWork,$0020
    sta a:VCE_ADDR_LO
    stz a:VCE_ADDR_HI
    txa
    and #$0F
    asl a
    tax
    lda a:PaletteWork,x
    sta a:PaletteWorkSpare
    lda a:PaletteWork+1,x
    sta a:PaletteWorkSpare+1
TitlePaletteRotate_at0:
    lda a:PaletteWork+2,x
    sta a:PaletteWork,x
    lda a:PaletteWork+3,x
    sta a:PaletteWork+1,x
    inx
    inx
    cpx #$22
    bcc TitlePaletteRotate_at0
    tia PaletteWork,VCE_DATA_LO,$0020
    rts

; @routine 00C1:1CA TitleTileRows
; @run 6B:81CA
; WHAT IT DOES: Writes the title's two rows of cycling tiles, $96 on by the frame count's bits 1
; and 2: two tiles at VRAM $0060, two at $00E0.
; HOW: SelectVramWriteAddress, SelectVramData, TitleTileWrite.
; Entry: nothing. Exit: as TitleTileWrite; C and V clear from the first tile's add. X and Y kept.
TitleTileRows:
    lda z:zFrameTick
    asl a
    and #$0C
    clc
    adc #$96
    sta z:zTemp00
    jsr VecSelectVramWriteAddress
    st1 #$60
    st2 #$00
    jsr VecSelectVramData
    bsr TitleTileWrite
    bsr TitleTileWrite
    jsr VecSelectVramWriteAddress
    st1 #$E0
    st2 #$00
    jsr VecSelectVramData
    bsr TitleTileWrite

; @routine 00C1:1EE TitleTileWrite
; @run 6B:81EE
; WHAT IT DOES: Writes the next tile of the title's cycling pattern to VRAM.
; HOW: zTemp00 is the tile, counted on; its high byte $31.
; Entry: nothing. Exit: A the tile written, N and Z from the next. X, Y, C and V kept.
TitleTileWrite:
    lda z:zTemp00
    sta a:VDC_DATA_LO
    inc z:zTemp00
    st2 #$31
    rts
