; @routine 0023:000 OldCardMain
; @run 80:4000
; WHAT IT DOES: The program from where the boot program lands: empties the objects and the scroll, sets the screen
; through the card, loads (the card's entry at $E0E4 with $57 and $59), maps the CD RAM banks from $FFF5's byte,
; asks the card's cd_read (sector 3, its arguments in zp $F8 to $FF as given) until it answers zero, starts the
; CD track, writes the title's tiles (8 KB from $7FDF by TIA to the VDC's data port), puts the soft reset's vector
; at $4225, and runs the title a frame at a time until RUN is pressed; then empties the objects again, loads the
; player screen ($53 and $55), sets up the player and object $20, and runs that screen a frame at a time for good.
; HOW: Calls of the card's entries with the registers its code is handed (most have no typed C yet), and of this
; program's routines by their frames. The frame loops wait on the card's entry at $E07B.
; WAITS: yes, in the card's entries it calls (not read yet); it never returns.
; Entry: from the boot program's jump, MPR2 to MPR6 $80 to $84. Exit: never.
OldCardMain:
    clx
OldCardMain_at0:
    jsr $4584
    inx
    cpx #$58
    bne OldCardMain_at0
    stz a:ScrollXFrac
    stz a:ScrollXLo
    stz a:ScrollXHi
    stz a:ScrollYFrac
    stz a:ScrollYLo
    stz a:ScrollYHi
    stz a:ScrollXFrac+1
    lda #$02
    jsr BiosExScrSiz
    cla
    ldx #$20
    ldy #$1E
    jsr BiosE06F
    stz z:BiosIrqDisable
    jsr $E099
    jsr $E07B
    cla
    jsr $E072
    lda #$DF
    sta a:$2220
    lda #$57
    sta a:$2221
    lda #$10
    sta a:$2222
    lda #$DF
    sta a:$2223
    lda #$59
    sta a:$2224
    lda #$10
    sta a:$2225
    lda #$02
    sta a:$221F
    jsr $E0E4
    lda a:$FFF5
    sta z:zRamBaseBank
    tam #$04
    inc a
    tam #$08
    inc a
    tam #$10
    inc a
    tam #$20
OldCardMain_at1:
    lda #$00
    sta z:zBiosCl
    lda #$00
    sta z:zBiosCh
    lda #$03
    sta z:zBiosDl
    lda #$FE
    sta z:zBiosDh
    lda #$00
    sta z:zBiosBl
    lda #$10
    sta z:zBiosBh
    lda #$00
    sta z:zBiosAl
    lda #$C0
    sta z:zBiosAh
    jsr BiosCdRead
    tax
    bne OldCardMain_at1
    jsr PlayCdTrack
    cla
    clx
    jsr $E0AE
    tia $7FDF,VDC_DATA_LO,$2000
    sei
    st0 #$07
    st1 #$00
    st2 #$00
    st0 #$08
    st1 #$00
    st2 #$00
    cli
    stz a:BiosScreenLo
    lda #$10
    sta a:BiosScreenHi
    jsr BiosE0A2
    stz a:BiosScreenLo
    lda #$10
    sta a:BiosScreenHi
    jsr $E09F
    jsr $E07B
    jsr SpawnTitleObjects
    jsr $E096
    jsr DrawSprites
    stz a:BiosScreenLo
    lda #$10
    sta a:BiosScreenHi
    jsr $E09F
    jsr $E07B
    lda #$06
    ldx #$25
    ldy #$42
    jsr BiosE05D
OldCardMain_at3:
    jsr BiosE063
    inc z:zFrameTick
    jsr $E07B
    lda a:ScrollXFrac
    sta a:ScrollBandFirst
    lda a:ScrollXLo
    sta a:ScrollBgxLo
    lda a:ScrollXHi
    sta a:ScrollBgxHi
    lda a:ScrollYFrac
    sta a:ScrollBandFourth
    lda a:ScrollYLo
    sta a:ScrollBgyLo
    lda a:ScrollYHi
    sta a:ScrollBgyHi
    jsr SetBackgroundScroll
    lda a:ScrollXHi
    inc a
    and #$03
    sta a:ScrollXHi
    jsr $4692
    jsr StepTitleTimer
    lda a:BiosJoyTrigger
    and #$08
    bne OldCardMain_at2
    jsr DrawSprites
    stz a:BiosScreenLo
    lda #$10
    sta a:BiosScreenHi
    jsr $E09F
    bra OldCardMain_at3
OldCardMain_at2:
    jsr $E099
    jsr $E07B
    stz a:ScrollXHi
    clx
OldCardMain_at4:
    jsr $4584
    inx
    cpx #$58
    bne OldCardMain_at4
    cla
    jsr $E072
    lda #$DF
    sta a:$2220
    lda #$53
    sta a:$2221
    lda #$10
    sta a:$2222
    lda #$DF
    sta a:$2223
    lda #$55
    sta a:$2224
    lda #$10
    sta a:$2225
    lda #$02
    sta a:$221F
    jsr $E0E4
    cla
    clx
    jsr $E0AE
    tia $5FDF,VDC_DATA_LO,$2000
    sei
    st0 #$07
    st1 #$00
    st2 #$00
    st0 #$08
    st1 #$00
    st2 #$00
    cli
    stz a:BiosScreenLo
    lda #$10
    sta a:BiosScreenHi
    jsr BiosE0A2
    stz a:BiosScreenLo
    lda #$10
    sta a:BiosScreenHi
    jsr $E09F
    jsr $E07B
    jsr InitPlayerObject
    jsr $4846
    jsr $4AFC
    jsr $49A2
    lda #$37
    sta a:$2A88
    lda #$80
    sta a:$2BE8
    lda #$58
    sta a:$2CF0
    lda #$02
    sta a:$3530
    jsr $E096
    jsr DrawSprites
    stz a:BiosScreenLo
    lda #$10
    sta a:BiosScreenHi
    jsr $E09F
    jsr $E07B
OldCardMain_at5:
    jsr BiosE063
    inc z:zFrameTick
    jsr $E07B
    lda a:ScrollXFrac
    sta a:ScrollBandFirst
    lda a:ScrollXLo
    sta a:ScrollBgxLo
    lda a:ScrollXHi
    sta a:ScrollBgxHi
    lda a:ScrollYFrac
    sta a:ScrollBandFourth
    lda a:ScrollYLo
    sta a:ScrollBgyLo
    lda a:ScrollYHi
    sta a:ScrollBgyHi
    jsr SetBackgroundScroll
    jsr $4692
    bsr StepDemoFrame
    jsr DrawSprites
    stz a:BiosScreenLo
    lda #$10
    sta a:BiosScreenHi
    jsr $E09F
    bra OldCardMain_at5

; @routine 0023:225 OldCardSoftReset
; @run 80:4225
; WHAT IT DOES: The soft reset's handler the main program installs (ex_setvec, vector 6): the card's reset.
; HOW: JMP $E000.
; WAITS: no; it does not return.
; Entry: from the card's soft reset. Exit: never.
OldCardSoftReset:
    jmp $E000

; @routine 0023:228 StepDemoFrame
; @run 80:4228
; WHAT IT DOES: One frame of the player screen: counts down object $20's timer (its $34B8 byte, then its $3510 byte
; when that reaches zero) and clears its frame when both have; runs the player, the program's routines at $4863,
; $4B11 and $49AD, ScrollWithPlayer, and the routines at $497B and $4BE8.
; HOW: Two decrements, then seven calls; the four with no C yet are run as original with the registers the call
; before left.
; WAITS: where the original code it calls does (not read yet).
; Entry: nothing. Exit: as the routine at $4BE8 leaves them.
StepDemoFrame:
    dec a:$34D8
    bne StepDemoFrame_at0
    dec a:$3530
    bne StepDemoFrame_at0
    stz a:$2A88
StepDemoFrame_at0:
    jsr RunPlayerObject
    jsr $4863
    jsr $4B11
    jsr $49AD
    jsr ScrollWithPlayer
    jsr $497B
    jsr $4BE8
    rts

; @routine 0023:24B SetBackgroundScroll
; @run 80:424B
; WHAT IT DOES: Writes the scroll the program wants, band 0's background X and Y, to the VDC's BXR and BYR.
; HOW: With interrupts off, selects BXR and writes ScrollBgxLo and Hi, selects BYR and writes ScrollBgyLo and Hi,
; then turns interrupts back on.
; WAITS: no.
; Entry: nothing. Exit: A ScrollBgyHi as read, N and Z from it, I clear. X, Y, C and V kept.
SetBackgroundScroll:
    sei
    lda #$07
    jsr SelectVdcRegisterOldCard
    lda a:ScrollBgxLo
    sta a:VDC_DATA_LO
    lda a:ScrollBgxHi
    sta a:VDC_DATA_HI
    lda #$08
    jsr SelectVdcRegisterOldCard
    lda a:ScrollBgyLo
    sta a:VDC_DATA_LO
    lda a:ScrollBgyHi
    sta a:VDC_DATA_HI
    cli
    rts

; @routine 0023:270 SelectVdcRegisterOldCard
; @run 80:4270
; WHAT IT DOES: Selects a VDC register, remembering which in zVdcReg: this program's copy of SelectVdcRegister.
; HOW: The same two stores (SelectVdcRegister's body).
; WAITS: no.
; Entry: A register. Exit: A, X, Y and flags kept.
SelectVdcRegisterOldCard:
    sta z:zVdcReg
    sta a:VDC_ADDR
    rts

; @routine 0023:276 StartObjectAnimation
; @run 80:4276
; WHAT IT DOES: Steps an object's animation on the list whose number is in A: the list pointer from this program's
; table at $52DF goes to zTemp10-11, then the step is the one StepObjectFrames makes.
; HOW: A doubled indexes the table (its bit 7 is dropped), the word goes to zTemp10 and zTemp11, and a jump to the
; program's copy of StepObjectFrames at $449E.
; WAITS: no.
; Entry: A the list, X the object. Exit: as StepObjectFrames, Y the doubled list number where it keeps Y.
StartObjectAnimation:
    asl a
    tay
    lda a:$52DF,y
    sta z:zTemp10
    lda a:$52E0,y
    sta z:zTemp11
    jmp $449E

; @routine 0023:285 ClampObjectLeft
; @run 80:4285
; WHAT IT DOES: Puts the player back at the left edge of the screen's walking room, X $28.
; HOW: One store; ScrollWithPlayer branches here.
; WAITS: no.
; Entry: nothing. Exit: A $28, N and Z from it. X, Y, C and V kept.
ClampObjectLeft:
    lda #$28
    sta a:OBJ_X_LO
    rts

; @routine 0023:28B ClampObjectRight
; @run 80:428B
; WHAT IT DOES: Puts the player back at the right edge of the screen's walking room, X $D8.
; HOW: One store; ScrollWithPlayer branches here, and to its RTS.
; WAITS: no.
; Entry: nothing. Exit: A $D8, N and Z from it. X, Y, C and V kept.
ClampObjectRight:
    lda #$D8
    sta a:OBJ_X_LO
ClampObjectRight_at0:
    rts

; @routine 0023:291 ScrollWithPlayer
; @run 80:4291
; WHAT IT DOES: Keeps the player between X $28 and $D8, and while the background is not at an end, scrolls it a
; pixel toward the player and puts the player back at the centre, X $80. The scroll runs from 0 to $301; reaching
; $301 sets $3E01 (the second byte of ScrollXFrac), which stops the scroll for good, and going below 0 holds it at 0.
; HOW: Compares of the player's X; ScrollXLo and ScrollXHi as a word stepped up or down, read back after each step.
; WAITS: no.
; Entry: nothing. Exit: by arm (FollowArm): the clamps as theirs with C from the compare that sent them (0 left,
; 1 right); otherwise A, N, Z, Y and C as the arm's last load, store or compare left them. X and V kept.
ScrollWithPlayer:
    lda a:OBJ_X_LO
    cmp #$28
    bcc ClampObjectLeft
    cmp #$D8
    bcs ClampObjectRight
    ldy a:ScrollXFrac+1
    bne ClampObjectRight_at0
    cmp #$80
    beq ClampObjectRight_at0
    bcc ScrollWithPlayer_at0
    inc a:ScrollXLo
    lda a:ScrollXLo
    cmp #$01
    beq ScrollWithPlayer_at1
    tay
    bne ScrollWithPlayer_at2
    inc a:ScrollXHi
    bra ScrollWithPlayer_at2
ScrollWithPlayer_at1:
    lda a:ScrollXHi
    cmp #$03
    bne ScrollWithPlayer_at2
    stz a:ScrollXLo
    lda #$03
    sta a:ScrollXHi
    inc a:ScrollXFrac+1
    rts
ScrollWithPlayer_at0:
    dec a:ScrollXLo
    lda a:ScrollXLo
    cmp #$FF
    bne ScrollWithPlayer_at2
    dec a:ScrollXHi
    bpl ScrollWithPlayer_at2
    stz a:ScrollXLo
    stz a:ScrollXHi
    rts
ScrollWithPlayer_at2:
    lda #$80
    sta a:OBJ_X_LO
    rts

; @routine 0023:2E8 DrawSprites
; @run 80:42E8
; WHAT IT DOES: Draws the frame's sprites: after the card's entry at $E0AE (A 0, X $10), every object of the 88
; with a frame is drawn by DrawObjectSprite, 64 sprites at most; the sprites left over are written as zeros.
; HOW: zp $17 the sprites left, zp $15 the objects left, zp $D9 the slot across the call; four zero words a sprite
; by ST1 and ST2.
; WAITS: where the card's entry does.
; Entry: nothing. Exit: by arm (SpritesArm).
DrawSprites:
    cla
    ldx #$10
    jsr $E0AE
    lda #$3F
    sta z:SpritesLeft
    lda #$58
    sta z:zProduct2
    clx
DrawSprites_at2:
    lda a:OBJ_2A68,x
    beq DrawSprites_at0
    stx z:zRichterObject
    bsr DrawObjectSprite
    bmi DrawSprites_at1
    ldx z:zRichterObject
DrawSprites_at0:
    inx
    dec z:zProduct2
    bne DrawSprites_at2
    ldx z:SpritesLeft
    beq DrawSprites_at1
DrawSprites_at3:
    st1 #$00
    st2 #$00
    st1 #$00
    st2 #$00
    st1 #$00
    st2 #$00
    st1 #$00
    st2 #$00
    dex
    bpl DrawSprites_at3
DrawSprites_at1:
    rts

; @routine 0023:321 DrawObjectSprite
; @run 80:4321
; WHAT IT DOES: Writes an object's sprites to the VDC's data port, a part at a time from the list its frame names
; in the table at $4C9D: each part is five bytes (attributes, Y and X offsets, the pattern's two bytes). A part off
; the screen (Y plus $40 outside 0 to $12F, X plus $20 outside the part's edge to $11F) is passed over; a part's X
; is mirrored when the object faces left, its palette replaced (flash bit 7) or offset (bit 6) by the low nibble
; of the object's flash byte, and its Y flip set from the object's flip byte. Each part written counts one of the
; sprites left (zp $17); when they run out it stops.
; HOW: The object's place plus the screen's offsets into zTemp06 to 0B, the part's four words built in zTemp00
; to 07 and written high word first, from zTemp07 down, through absolute reads of the zero page.
; WAITS: no.
; Entry: X the object. Exit: by arm (SpriteArm): A, X, Y, N, Z, C and V as the arm's last instructions left them.
DrawObjectSprite:
    lda a:OBJ_Y_LO,x
    clc
    adc #$40
    sta z:zTemp08
    lda a:OBJ_Y_HI,x
    adc #$00
    sta z:zTemp09
    lda a:OBJ_X_LO,x
    clc
    adc #$20
    sta z:zTemp0A
    lda a:OBJ_X_HI,x
    adc #$00
    sta z:zTemp0B
    lda a:OBJ_2AC0,x
    sta z:zTemp0D
    lda a:OBJ_2FE8,x
    sta z:SpriteFlipMask
    lda a:OBJ_2B18,x
    sta z:zTemp0E
    and #$0F
    sta z:zTemp0F
    lda #$9D
    sta z:zTemp00
    lda #$4C
    sta z:zTemp01
    lda a:OBJ_2A68,x
    asl a
    tay
    bcc DrawObjectSprite_at0
    inc z:zTemp01
DrawObjectSprite_at0:
    lda (zTemp00),y
    sta z:zTemp10
    iny
    lda (zTemp00),y
    sta z:zTemp11
    lda (zTemp10)
    sta z:SpritePartsLeft
    bne DrawObjectSprite_at1
    jmp DrawObjectSprite_at2
DrawObjectSprite_at1:
    inc z:zTemp10
    bne DrawObjectSprite_at3
    inc z:zTemp11
DrawObjectSprite_at3:
    clx
    cly
    lda (zTemp10),y
    sta z:zTemp00
    iny
    stz z:zTemp06
    lda (zTemp10),y
    bpl DrawObjectSprite_at4
    dec z:zTemp06
DrawObjectSprite_at4:
    clc
    adc z:zTemp08
    sta z:zTemp07
    lda z:zTemp06
    adc z:zTemp09
    sta z:zTemp06
    beq DrawObjectSprite_at5
    bpl DrawObjectSprite_at6
    jmp DrawObjectSprite_at7
DrawObjectSprite_at6:
    cmp #$01
    beq DrawObjectSprite_at8
    jmp DrawObjectSprite_at7
DrawObjectSprite_at8:
    lda z:zTemp07
    cmp #$30
    bcc DrawObjectSprite_at5
    jmp DrawObjectSprite_at7
DrawObjectSprite_at5:
    iny
    lda z:zTemp00
    and #$01
    tax
    stz z:zTemp04
    lda z:zTemp0D
    beq DrawObjectSprite_at9
    lda z:zTemp00
    eor #$08
    sta z:zTemp00
    lda a:$4453,x
    sec
    sbc (zTemp10),y
    bra DrawObjectSprite_at10
DrawObjectSprite_at9:
    lda (zTemp10),y
DrawObjectSprite_at10:
    bpl DrawObjectSprite_at11
    dec z:zTemp04
DrawObjectSprite_at11:
    clc
    adc z:zTemp0A
    sta z:zTemp05
    lda z:zTemp04
    adc z:zTemp0B
    sta z:zTemp04
    beq DrawObjectSprite_at12
    bpl DrawObjectSprite_at13
    jmp DrawObjectSprite_at7
DrawObjectSprite_at13:
    lda z:zTemp04
    cmp #$01
    bne DrawObjectSprite_at7
    lda z:zTemp05
    cmp #$20
    bcs DrawObjectSprite_at7
    bra DrawObjectSprite_at14
DrawObjectSprite_at12:
    lda z:zTemp05
    cmp a:$4455,x
    bcc DrawObjectSprite_at7
DrawObjectSprite_at14:
    iny
    lda (zTemp10),y
    sta z:zTemp03
    iny
    lda (zTemp10),y
    and #$70
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp02
    lda z:zTemp0E
    asl a
    bcs DrawObjectSprite_at15
    bpl DrawObjectSprite_at16
    lda (zTemp10),y
    clc
    adc z:zTemp0F
    and #$8F
    bra DrawObjectSprite_at17
DrawObjectSprite_at15:
    lda (zTemp10),y
    and #$80
    ora z:zTemp0F
    bra DrawObjectSprite_at17
DrawObjectSprite_at16:
    lda (zTemp10),y
    and #$8F
DrawObjectSprite_at17:
    sta z:zTemp01
    lda z:SpriteFlipMask
    and #$80
    eor z:zTemp01
    sta z:zTemp01
    iny
    ldy #$07
DrawObjectSprite_at18:
    lda a:ZeroPage,y
    sta a:VDC_DATA_LO
    dey
    lda a:ZeroPage,y
    sta a:VDC_DATA_HI
    dey
    bpl DrawObjectSprite_at18
    dec z:SpritesLeft
    bmi DrawObjectSprite_at2
DrawObjectSprite_at7:
    lda #$05
    clc
    adc z:zTemp10
    sta z:zTemp10
    bcc DrawObjectSprite_at19
    inc z:zTemp11
DrawObjectSprite_at19:
    dec z:SpritePartsLeft
    beq DrawObjectSprite_at2
    jmp DrawObjectSprite_at3
DrawObjectSprite_at2:
    rts

; @routine 0023:457 PlayCdTrack
; @run 80:4457
; WHAT IT DOES: Plays the program's CD track, repeating (mode $41), from the six bytes at $4498, asking the card's
; cd_play until it answers zero; then the card's cd_fade with A 0.
; HOW: The kernel's CdPlayRepeat without its track table: zTemp00-01 point at the bytes, which go to the card's
; argument bytes in its order, zBiosBh $40 first; X the mode, then each failed try's answer.
; WAITS: yes, on the drive, as CdPlayRepeat: the stack there is the caller's return, this call's two bytes and X and
; Y pushed.
; Entry: nothing. Exit: A 0 from the fade, C and V as the card leaves them, X and Y kept, N and Z from X. The C
; replays the last 32 tries' exits.
PlayCdTrack:
    phx
    phy
    ldx #$41
    stx z:zCdPlayMode
    lda #$98
    sta z:zTemp00
    lda #$44
    sta z:zTemp01
PlayCdTrack_at0:
    lda #$40
    sta z:zBiosBh
    lda (zTemp00)
    sta z:zBiosAl
    ldy #$01
    lda (zTemp00),y
    sta z:zBiosAh
    iny
    lda (zTemp00),y
    sta z:zBiosBl
    lda z:zCdPlayMode
    sta z:zBiosDh
    iny
    lda (zTemp00),y
    sta z:zBiosCl
    iny
    lda (zTemp00),y
    sta z:zBiosCh
    iny
    lda (zTemp00),y
    sta z:zBiosDl
    jsr BiosCdPlay
    tax
    bne PlayCdTrack_at0
    cla
    jsr BiosCdFade
    ply
    plx
    rts

; @routine 0023:4D0 SpawnTitleObjects
; @run 80:44D0
; WHAT IT DOES: Empties objects 0 to 8 and gives each its first frame and place from the program's three tables
; ($44F0 frames, $44F9 X, $4502 Y); then zeroes the title timer, a word at zp $7A-$7B (zWaitTimer3 and 4 in the
; game's own use).
; HOW: A loop over the nine slots through the program's copy of ClearObject at $4584, then three table reads.
; WAITS: no.
; Entry: nothing. Exit: A the last Y byte read (the table's ninth), X 9, N 0, Z 1, C 1 (the CPX). Y and V kept.
SpawnTitleObjects:
    clx
SpawnTitleObjects_at0:
    jsr $4584
    lda a:$44F0,x
    sta a:OBJ_2A68,x
    lda a:$44F9,x
    sta a:OBJ_X_LO,x
    lda a:$4502,x
    sta a:OBJ_Y_LO,x
    inx
    cpx #$09
    bne SpawnTitleObjects_at0
    stz z:zWaitTimer3
    stz z:zWaitTimer4
    rts

; @routine 0023:50B StepTitleTimer
; @run 80:450B
; WHAT IT DOES: One frame of the title: steps the animations of objects 4 (list $0A) and 5 (list $0B) and the title
; timer at zp $7A-$7B. At $1E0 it sets object 0's frame to $2F. When the timer's low seven bits reach $44 it clears
; the frames of objects 7 and 8, at $40 it gives object 8 (timer bit 7 set) or object 7 (clear) the frame $36; at
; either it hands the card at $E0E4 the arguments at $221F to $2225 ($DF and $57, $5B or $5D at $2220-$2221, $10 at
; $2222, 0 at $2225, 2 at $221F) and jumps to it.
; HOW: Two calls of StartObjectAnimation, the word's increment, compares of its bytes read again each time.
; WAITS: no; the card's routine is entered by a JMP and returns to this routine's caller.
; Entry: nothing. Exit: returned: A the timer's low seven bits, N and Z from their compare with $40, C from it,
; X 5, Y as the second step left it. Card arm: as the card's routine leaves them. V kept.
StepTitleTimer:
    ldx #$04
    lda #$0A
    jsr StartObjectAnimation
    inx
    lda #$0B
    jsr StartObjectAnimation
    inc z:zWaitTimer3
    bne StepTitleTimer_at0
    inc z:zWaitTimer4
StepTitleTimer_at0:
    lda z:zWaitTimer4
    cmp #$01
    bne StepTitleTimer_at1
    lda z:zWaitTimer3
    cmp #$E0
    bne StepTitleTimer_at1
    lda #$2F
    sta a:OBJ_2A68
StepTitleTimer_at1:
    lda z:zWaitTimer3
    and #$7F
    cmp #$44
    beq StepTitleTimer_at2
    cmp #$40
    beq StepTitleTimer_at3
    rts
StepTitleTimer_at3:
    lda z:zWaitTimer3
    bmi StepTitleTimer_at4
    bra StepTitleTimer_at5
StepTitleTimer_at2:
    stz a:OBJ_2A68+7
    stz a:OBJ_2A68+8
    lda #$DF
    sta a:$2220
    lda #$57
    sta a:$2221
StepTitleTimer_at6:
    lda #$10
    sta a:$2222
    stz a:$2225
    lda #$02
    sta a:$221F
    jmp $E0E4
StepTitleTimer_at4:
    lda #$36
    sta a:OBJ_2A68+8
    lda #$DF
    sta a:$2220
    lda #$5B
    sta a:$2221
    bra StepTitleTimer_at6
StepTitleTimer_at5:
    lda #$36
    sta a:OBJ_2A68+7
    lda #$DF
    sta a:$2220
    lda #$5D
    sta a:$2221
    bra StepTitleTimer_at6

; @routine 0023:6CA InitPlayerObject
; @run 80:46CA
; WHAT IT DOES: Empties object 0, the player, and puts it at X $40, Y $A0 with frame 3.
; HOW: The program's copy of ClearObject, then three stores.
; WAITS: no.
; Entry: nothing. Exit: A $40, X 0, N and Z from A. Y, C and V kept.
InitPlayerObject:
    ldx #$00
    jsr $4584
    lda #$03
    sta a:OBJ_2A68,x
    lda #$A0
    sta a:OBJ_Y_LO,x
    lda #$40
    sta a:OBJ_X_LO,x
    rts

; @routine 0023:6DF RunPlayerObject
; @run 80:46DF
; WHAT IT DOES: Runs the player object's handler for its state (the byte at $34B8): stand, walk, jump, and two the
; program has at $4812 and $4824.
; HOW: The state doubled indexes the table at $46F3; the word goes to zTemp00-01 and an indirect jump takes it.
; WAITS: no.
; Entry: nothing. Exit: X 0; the rest as the handler leaves them.
RunPlayerObject:
    ldx #$00
    lda a:OBJ_34B8,x
    asl a
    tay
    lda a:$46F3,y
    sta z:zTemp00
    lda a:$46F4,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0023:6FD PlayerStand
; @run 80:46FD
; WHAT IT DOES: The player standing: knocked back when hurt ($3250 set); I jumps, II attacks, left or right walks
; (a first step at once); otherwise it stays standing. Then the animation steps on the state's list.
; HOW: The pad bytes the card keeps (BiosJoyTrigger shifted bit by bit, BiosPadState), and the arms at $4720,
; $4729, $472F, $475C and $4769 that the walk and the jump share.
; WAITS: no.
; Entry: X the object. Exit: as player_exit_shim's arms. X kept.
PlayerStand:
    lda a:OBJ_3250,x
    bne PlayerStand_at0
    lda a:BiosJoyTrigger
    lsr a
    bcs PlayerStand_at1
    lsr a
    bcs PlayerStand_at2
    lda a:BiosPadState
    and #$A0
    beq PlayerStand_at3
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$01
    sta a:OBJ_34B8,x
    jmp $47A7
PlayerStand_at3:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    stz a:OBJ_34B8,x
PlayerStand_at6:
    lda a:OBJ_34B8,x
    jmp StartObjectAnimation
PlayerStand_at1:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VX_HI,x
    lda a:BiosPadState
    and #$A0
    beq PlayerStand_at4
    asl a
    bcs PlayerStand_at5
    inc a:OBJ_VX_HI,x
    bra PlayerStand_at4
PlayerStand_at5:
    dec a:OBJ_VX_HI,x
PlayerStand_at4:
    lda #$02
    sta a:OBJ_34B8,x
    bra PlayerStand_at6
PlayerStand_at2:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$03
    sta a:OBJ_34B8,x
    bra PlayerStand_at6
PlayerStand_at0:
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$80
    sta a:OBJ_VX_LO,x
    lda a:OBJ_3250,x
    lsr a
    tay
    lda a:$478D,y
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VY_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda #$04
    sta a:OBJ_34B8,x
    rts

; @routine 0023:78F PlayerRestFromWalk
; @run 80:478F
; WHAT IT DOES: The walk's way back to standing.
; HOW: A jump to $4720 in PlayerStand.
; WAITS: no.
; Entry: X the object. Exit: as player_exit_shim's animated arm. X kept.
PlayerRestFromWalk:
    jmp $4720

; @routine 0023:792 PlayerWalk
; @run 80:4792
; WHAT IT DOES: The player walking: hurt, I and II as standing; with neither left nor right held it stands again;
; otherwise a pixel toward the side held, turning to face it unless II is held.
; HOW: The same pad reads as PlayerStand; the step is at $47A7.
; WAITS: no.
; Entry: X the object. Exit: as player_exit_shim's arms. X kept.
PlayerWalk:
    lda a:OBJ_3250,x
    bne PlayerStand_at0
    lda a:BiosJoyTrigger
    lsr a
    bcs PlayerStand_at1
    lsr a
    bcs PlayerStand_at2
    lda a:BiosPadState
    and #$A0
    beq PlayerRestFromWalk
    lda a:BiosPadState
    asl a
    bcc PlayerWalk_at0
    lda a:BiosPadState
    and #$02
    bne PlayerWalk_at1
    lda #$01
    sta a:OBJ_2AC0,x
PlayerWalk_at1:
    dec a:OBJ_X_LO,x
    jmp PlayerStand_at6
PlayerWalk_at0:
    lda a:BiosPadState
    and #$02
    bne PlayerWalk_at2
    stz a:OBJ_2AC0,x
PlayerWalk_at2:
    inc a:OBJ_X_LO,x
    jmp PlayerStand_at6

; @routine 0023:7CF PlayerJump
; @run 80:47CF
; WHAT IT DOES: The player in the air: hurt as standing; otherwise gravity ($20 a frame on the Y velocity), the move
; by the velocity, a landing at Y $A0 that stops the X speed, and the animation; when the jump's list has ended the
; velocity is zeroed and the player stands.
; HOW: The velocity word's add, the program's copy of ApplyVelocity at $4624, StartObjectAnimation, whose A the TAY
; at $47FF tests.
; WAITS: no.
; Entry: X the object. Exit: as player_exit_shim's arms, V that of the move. X kept.
PlayerJump:
    lda a:OBJ_3250,x
    bne PlayerStand_at0
    lda #$20
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,x
    cla
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
    jsr $4624
    lda a:OBJ_Y_LO,x
    cmp #$A0
    bcc PlayerJump_at0
    lda #$A0
    sta a:OBJ_Y_LO,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
PlayerJump_at0:
    lda a:OBJ_34B8,x
    jsr StartObjectAnimation
    tay
    bne PlayerJump_at1
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    jmp $4720
PlayerJump_at1:
    rts
