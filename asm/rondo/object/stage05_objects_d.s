; @routine 093F:083 SetUpRidersAround093F
; @run 71:D083
; WHAT IT DOES: Loads the record, starts list channel 2 with list $84, a wait of $50, and a kind $39 object below and a kind $41 above.
; HOW: ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetUpRidersAround093F:
    jsr VecLoadObjectRecord
    phx
    lda #$84
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    lda #$50
    sta a:OBJ_PIECE_PLACE,x
    txa
    tay
    dey
    jsr VecClearObjectInY
    lda #$39
    jsr VecStartObjectAtPlace
    iny
    iny
    jsr VecClearObjectInY
    lda #$41
    jsr VecStartObjectAtPlace
    rts

; @routine 093F:0AA NextOnFirstScreen093F
; @run 71:D0AA
; WHAT IT DOES: Goes on once on the first screen.
; HOW: X high.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NextOnFirstScreen093F:
    lda a:OBJ_X_HI,x
    bne NextOnFirstScreen093F_at0
    inc a:OBJ_STATE,x
NextOnFirstScreen093F_at0:
    rts

; @routine 093F:0B3 WalkLeftUntilMap720_093F
; @run 71:D0B3
; WHAT IT DOES: Walks ($D0DD) leftward; at map X below $0720 goes to state 3. Counts the pause down ($D144).
; HOW: The map X from the camera.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkLeftUntilMap720_093F:
    jsr StepAndPickSpeed093F
    stz a:OBJ_SWOOP_SPEED_LO,x
    lda a:ScrollXLo
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:ScrollXHiScreen
    adc a:OBJ_X_HI,x
    cmp #$07
    bcc WalkLeftUntilMap720_093F_at0
    bne WalkLeftUntilMap720_093F_at1
    lda z:zTemp00
    cmp #$20
    bcs WalkLeftUntilMap720_093F_at1
WalkLeftUntilMap720_093F_at0:
    lda #$03
    sta a:OBJ_STATE,x
WalkLeftUntilMap720_093F_at1:
    jsr CountPauseThenState4_093F
    rts

; @routine 093F:0DD StepAndPickSpeed093F
; @run 71:D0DD
; WHAT IT DOES: Steps the list at $D137; on a new frame takes the speed for OBJ_SWOOP_SPEED_LO and moves turning at walls ($D0F3).
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepAndPickSpeed093FSpeeds = $D135 ; VX high by direction
StepAndPickSpeed093FList = $D137 ; a frame list
StepAndPickSpeed093F:
    lda #$37
    sta z:zTemp10
    lda #$D1
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc MoveTurningAtWalls093F_at0
    ldy a:OBJ_SWOOP_SPEED_LO,x
    lda a:StepAndPickSpeed093FSpeeds,y
    sta a:OBJ_VX_HI,x

; @routine 093F:0F3 MoveTurningAtWalls093F
; @run 71:D0F3
; WHAT IT DOES: Moves sideways; when the terrain ahead is solid at probes 9 and 2, turns back and moves.
; HOW: ApplyVelocityX and two ProbeTerrain with SpriteFlipMask 8.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
MoveTurningAtWalls093F:
    jsr VecApplyVelocityX
    lda #$08
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    tay
    lda a:TileHit+1,y
    and #$01
    beq MoveTurningAtWalls093F_at1
    lda #$08
    sta z:SpriteFlipMask
    lda #$02
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    asl a
    tay
    lda a:TileHit,y
    ora a:TileHit+1,y
    and #$02
    bne MoveTurningAtWalls093F_at1
MoveTurningAtWalls093F_at0:
    rts
MoveTurningAtWalls093F_at1:
    jsr VecNegateVelocityX
    jmp VecApplyVelocityX

; @routine 093F:144 CountPauseThenState4_093F
; @run 71:D144
; WHAT IT DOES: Counts OBJ_PIECE_PLACE out and goes to state 4.
; HOW: A countdown.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountPauseThenState4_093F:
    lda a:OBJ_PIECE_PLACE,x
    beq CountPauseThenState4_093F_at0
    dec a:OBJ_PIECE_PLACE,x
    rts
CountPauseThenState4_093F_at0:
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 093F:156 WalkRightUntilMap8A0_093F
; @run 71:D156
; WHAT IT DOES: Walks rightward; from map X $07A0 goes to state 2. Counts the pause down.
; HOW: The map X from the camera.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkRightUntilMap8A0_093F:
    jsr StepAndPickSpeed093F
    lda #$01
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:ScrollXLo
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:ScrollXHiScreen
    adc a:OBJ_X_HI,x
    cmp #$08
    bcs WalkRightUntilMap8A0_093F_at0
    cmp #$07
    bne WalkRightUntilMap8A0_093F_at1
    lda z:zTemp00
    cmp #$A0
    bcc WalkRightUntilMap8A0_093F_at1
WalkRightUntilMap8A0_093F_at0:
    lda #$02
    sta a:OBJ_STATE,x
WalkRightUntilMap8A0_093F_at1:
    jsr CountPauseThenState4_093F
    rts

; @routine 093F:184 BySubstate093FD184
; @run 71:D184
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $D196.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate093FD184Handlers = $D196 ; a handler per substate
BySubstate093FD184:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate093FD184Handlers,y
    sta z:zTemp00
    lda a:BySubstate093FD184Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093F:1A2 PickNextAttack093F
; @run 71:D1A2
; WHAT IT DOES: Restarts the animation, a random repeat count and the next substate from the table by count.
; HOW: NextRandom and a table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PickNextAttack093FOrder = $D1C0 ; substates in turn
PickNextAttack093F:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    jsr VecNextRandom
    and #$03
    sta a:OBJ_WORK_K,x
    lda a:OBJ_FALL_LO,x
    and #$07
    tay
    lda a:PickNextAttack093FOrder,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_FALL_LO,x
    rts

; @routine 093F:1C8 SlashForward093F
; @run 71:D1C8
; WHAT IT DOES: Steps the list at $D200, at step 8 sounding $95 and $1A, moving forward turning at walls; at its end a pause of $40 and the next substate.
; HOW: MoveTurningAtWalls093F.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SlashForward093FList = $D200 ; a frame list
SlashForward093FSpeeds = $D219 ; VX high by facing
SlashForward093F:
    lda #$00
    sta z:zTemp10
    lda #$D2
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne SlashForward093F_at0
    jmp SlashForward093F_at2
SlashForward093F_at0:
    bcc SlashForward093F_at3
    lda a:OBJ_ANIM_STEP,x
    cmp #$08
    bne SlashForward093F_at1
    lda #$95
    jsr VecSoundCommand
    lda #$1A
    jsr VecSoundCommand
SlashForward093F_at1:
    ldy a:OBJ_FACING_LEFT,x
    lda a:SlashForward093FSpeeds,y
    sta a:OBJ_VX_HI,x
    jmp MoveTurningAtWalls093F
SlashForward093F_at2:
    lda #$40
    sta a:OBJ_PIECE_NEXT,x
    inc a:OBJ_SUBSTATE,x
SlashForward093F_at3:
    rts

; @routine 093F:21B BackStepWaiting093F
; @run 71:D21B
; WHAT IT DOES: Steps the list at $D23D, backing on new frames; the pause over, ends the attack ($D325).
; HOW: MoveTurningAtWalls093F.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BackStepWaiting093FList = $D23D ; a frame list
BackStepWaiting093FSpeeds = $D24A ; VX high by facing
BackStepWaiting093F:
    lda #$3D
    sta z:zTemp10
    lda #$D2
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc BackStepWaiting093F_at0
    ldy a:OBJ_FACING_LEFT,x
    lda a:BackStepWaiting093FSpeeds,y
    sta a:OBJ_VX_HI,x
    jsr MoveTurningAtWalls093F
BackStepWaiting093F_at0:
    dec a:OBJ_PIECE_NEXT,x
    bne BackStepWaiting093F_at1
    jmp EndAttack093F
BackStepWaiting093F_at1:
    rts

; @routine 093F:24C ThrustForward093F
; @run 71:D24C
; WHAT IT DOES: As SlashForward093F with the list at $D285, sounding $95 and 7 at step 9; at its end substate 2.
; HOW: MoveTurningAtWalls093F.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrustForward093FSpeeds = $D283 ; VX high by facing
ThrustForward093FList = $D285 ; a frame list
ThrustForward093F:
    lda #$85
    sta z:zTemp10
    lda #$D2
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ThrustForward093F_at1
    bcc ThrustForward093F_at2
    lda a:OBJ_ANIM_STEP,x
    cmp #$09
    bne ThrustForward093F_at0
    lda #$95
    jsr VecSoundCommand
    lda #$07
    jsr VecSoundCommand
ThrustForward093F_at0:
    ldy a:OBJ_FACING_LEFT,x
    lda a:ThrustForward093FSpeeds,y
    sta a:OBJ_VX_HI,x
    jmp MoveTurningAtWalls093F
ThrustForward093F_at1:
    lda #$40
    sta a:OBJ_PIECE_NEXT,x
    lda #$02
    sta a:OBJ_SUBSTATE,x
ThrustForward093F_at2:
    rts

; @routine 093F:2C8 ThrowRepeatedly093F
; @run 71:D2C8
; WHAT IT DOES: Steps the list at $D30A, throwing a kind $3B object with sounds $1B and $96 at frame $24; at its end repeats from step 6 while OBJ_WORK_K lasts, then ends the attack.
; HOW: FindFreeSlotFrom30.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrowRepeatedly093FList = $D30A ; a frame list
ThrowRepeatedly093F:
    lda #$0A
    sta z:zTemp10
    lda #$D3
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq ThrowRepeatedly093F_at1
    bcc ThrowRepeatedly093F_at0
    lda a:OBJ_FRAME,x
    cmp #$24
    bne ThrowRepeatedly093F_at0
    lda #$1B
    jsr VecSoundCommand
    lda #$96
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom30
    bcc ThrowRepeatedly093F_at0
    lda #$3B
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
ThrowRepeatedly093F_at0:
    rts
ThrowRepeatedly093F_at1:
    lda a:OBJ_WORK_K,x
    bne ThrowRepeatedly093F_at2
    jmp EndAttack093F
ThrowRepeatedly093F_at2:
    dec a:OBJ_WORK_K,x
    lda #$06
    sta a:OBJ_ANIM_STEP,x
    rts

; @routine 093F:325 EndAttack093F
; @run 71:D325
; WHAT IT DOES: State 3 from its start with the next pause from the table.
; HOW: A count of attacks in OBJ_SWOOP_SPEED_HI.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EndAttack093FPauses = $D340 ; four pauses
EndAttack093F:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    inc a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_SWOOP_SPEED_HI,x
    and #$03
    tay
    lda a:EndAttack093FPauses,y
    sta a:OBJ_PIECE_PLACE,x
    rts

; @routine 093F:344 BySubstate093FD344
; @run 71:D344
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $D356.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate093FD344Handlers = $D356 ; a handler per substate
BySubstate093FD344:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate093FD344Handlers,y
    sta z:zTemp00
    lda a:BySubstate093FD344Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093F:35C RoarThenWait093F
; @run 71:D35C
; WHAT IT DOES: Sound $97, a count of $40 and the next substate.
; HOW: SoundCommand.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RoarThenWait093F:
    lda #$97
    jsr VecSoundCommand
    lda #$40
    sta a:OBJ_PIECE_NEXT,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093F:36A RainDown093F
; @run 71:D36A
; WHAT IT DOES: Steps the list at $D386, counting and raining ($D391); at its end the next substate with a count of $80.
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RainDown093FList = $D386 ; a frame list
RainDown093F:
    lda #$86
    sta z:zTemp10
    lda #$D3
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq RainDown093F_at0
    dec a:OBJ_PIECE_NEXT,x
    jmp RainAtCounts093F
RainDown093F_at0:
    inc a:OBJ_SUBSTATE,x
    lda #$80
    sta a:OBJ_PIECE_NEXT,x
    rts

; @routine 093F:391 RainAtCounts093F
; @run 71:D391
; WHAT IT DOES: At one of eight counts starts a kind $9E drop with sound $18 and the row's speeds, and moves it.
; HOW: X kept in zClimbStepBefore, the row in zBurstCount.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RainAtCounts093FTimes = $D3CB ; eight counts
RainAtCounts093FSpeeds = $D3D3 ; VX high and VY high by row
RainAtCounts093F:
    cly
RainAtCounts093F_at0:
    lda a:OBJ_PIECE_NEXT,x
    cmp a:RainAtCounts093FTimes,y
    beq RainAtCounts093F_at1
    iny
    cpy #$08
    bcc RainAtCounts093F_at0
    rts
RainAtCounts093F_at1:
    stx z:zClimbStepBefore
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc RainAtCounts093F_at2
    lda #$9E
    jsr VecStartObjectAtPlace
    lda #$18
    jsr VecSoundCommand
    lda z:zBurstCount
    asl a
    tax
    lda a:RainAtCounts093FSpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:RainAtCounts093FSpeeds+1,x
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zClimbStepBefore
RainAtCounts093F_at2:
    rts

; @routine 093F:3E3 DissolveThenBurst093F
; @run 71:D3E3
; WHAT IT DOES: Every other count of OBJ_WORK_D dissolves a strip of tiles by OBJ_WORK_L; at the end becomes kind $93 rising.
; HOW: DissolveStrips093F.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DissolveThenBurst093F:
    dec a:OBJ_PIECE_NEXT,x
    beq DissolveThenBurst093F_at1
    lda a:OBJ_PIECE_NEXT,x
    and #$01
    bne DissolveThenBurst093F_at0
    lda a:OBJ_WORK_L,x
    clc
    adc #$13
    and #$3F
    sta a:OBJ_WORK_L,x
    jsr DissolveStrips093F
DissolveThenBurst093F_at0:
    rts
DissolveThenBurst093F_at1:
    lda #$93
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$16
    sta a:OBJ_INVULN_TIMER,x
    rts

; @routine 093F:41A DissolveStrips093F
; @run 71:D41A
; WHAT IT DOES: Makes the mask ($D429) and applies it to ten tile strips ($D454).
; HOW: X the strip.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DissolveStrips093F:
    phx
    jsr MakeDissolveMask093F
    clx
DissolveStrips093F_at0:
    jsr DissolveStripX093F
    inx
    cpx #$0A
    bcc DissolveStrips093F_at0
    plx
    rts

; @routine 093F:429 MakeDissolveMask093F
; @run 71:D429
; WHAT IT DOES: From A: zTemp00 the line (high nibble), zTemp01-02 all bits but the low nibble's.
; HOW: A shift loop and inversion.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
MakeDissolveMask093F:
    pha
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp00
    pla
    and #$0F
    sta z:zTemp03
    lda #$01
    sta z:zTemp01
    stz z:zTemp02
MakeDissolveMask093F_at0:
    lda z:zTemp03
    beq MakeDissolveMask093F_at1
    asl z:zTemp01
    rol z:zTemp02
    dec z:zTemp03
    bra MakeDissolveMask093F_at0
MakeDissolveMask093F_at1:
    lda z:zTemp01
    eor #$FF
    sta z:zTemp01
    lda z:zTemp02
    eor #$FF
    sta z:zTemp02
    rts

; @routine 093F:454 DissolveStripX093F
; @run 71:D454
; WHAT IT DOES: Dissolves the strip at the VRAM address the table gives for X.
; HOW: DissolveWords093F.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DissolveStripX093FPlaces = $D465 ; VRAM address per strip
DissolveStripX093F:
    txa
    asl a
    tay
    lda a:DissolveStripX093FPlaces,y
    sta z:zBiosCl
    lda a:DissolveStripX093FPlaces+1,y
    sta z:zBiosCh
    jsr DissolveWords093F
    rts

; @routine 093F:479 DissolveWords093F
; @run 71:D479
; WHAT IT DOES: Over $40 words from the address in zBiosCl-Ch, masks the words whose index low bits are zTemp00.
; HOW: Reads and writes each word through the VDC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DissolveWords093F:
    phy
    cly
DissolveWords093F_at0:
    tya
    and #$03
    cmp z:zTemp00
    bne DissolveWords093F_at1
    jsr VecSelectVramWriteAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    and z:zTemp01
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and z:zTemp02
    sta a:VDC_DATA_HI
DissolveWords093F_at1:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc DissolveWords093F_at0
    ply
    rts

; @routine 093F:4C3 ThrownDagger093F
; @run 71:D4C3
; @stop D4DD
; WHAT IT DOES: State 0 loads it with the facing's speed and $1B upward, and moves.
; HOW: ApplyVelocity; later states at $D4DF.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ThrownDagger093FSpeeds = $D4DD ; VX high by facing
ThrownDagger093F:
    lda a:OBJ_STATE,x
    bne FlyDagger093F
    jsr VecLoadObjectRecord
    ldy a:OBJ_FACING_LEFT,x
    lda a:ThrownDagger093FSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$1B
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    rts

; @routine 093F:526 ByState093FD526
; @run 71:D526
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $D538.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState093FD526Handlers = $D538 ; a handler per state
ByState093FD526:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState093FD526Handlers,y
    sta z:zTemp00
    lda a:ByState093FD526Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093F:53E HopFromFacing093F
; @run 71:D53E
; @stop D565
; WHAT IT DOES: Substate 0 hops by the facing without loop flag bit 1; substate 1 falls ($D569).
; HOW: A speed table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HopFromFacing093FSpeeds = $D565 ; VX words by facing
HopFromFacing093F:
    lda a:OBJ_SUBSTATE,x
    bne FallStep093F
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:HopFromFacing093FSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:HopFromFacing093FSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda a:OBJ_LOOP_FLAGS,x
    and #$FD
    sta a:OBJ_LOOP_FLAGS,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 093F:569 FallStep093F
; @run 71:D569
; WHAT IT DOES: Falls without the speed limit and moves.
; HOW: ApplyGravityUnlimited and a tail jump.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallStep093F:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 093F:5B9 ShieldOnHolder093F
; @run 71:D5B9
; WHAT IT DOES: State 0 loads it. Then, while the slot below holds kind $38, takes its place, facing, hit box setting and the frame's hit box; else cleared.
; HOW: Reads of slot X-1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ShieldOnHolder093FBoxes = $D691 ; four bytes by frame from 1
ShieldOnHolder093F:
    lda a:OBJ_STATE,x
    bne ShieldOnHolder093F_at0
    jmp VecLoadObjectRecord
ShieldOnHolder093F_at0:
    lda a:OBJ_KIND-1,x
    cmp #$38
    beq ShieldOnHolder093F_at1
    jmp VecClearObject
ShieldOnHolder093F_at1:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FRAME-1,x
    sec
    sbc #$01
    cmp #$22
    bcc ShieldOnHolder093F_at2
    lda #$22
ShieldOnHolder093F_at2:
    asl a
    asl a
    tay
    lda a:ShieldOnHolder093FBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:ShieldOnHolder093FBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:ShieldOnHolder093FBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:ShieldOnHolder093FBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:OBJ_FACING_LEFT-1,x
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_HITBOX_ON-1,x
    sta a:OBJ_HITBOX_ON,x
    rts

; @routine 093F:745 ByState093FD745
; @run 71:D745
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $D757.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState093FD745Handlers = $D757 ; a handler per state
ByState093FD745:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState093FD745Handlers,y
    sta z:zTemp00
    lda a:ByState093FD745Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093F:75F LoadThenWaitByArgument093F
; @run 71:D75F
; WHAT IT DOES: Loads the record and sets the wait ($D766).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadThenWaitByArgument093F:
    jsr VecLoadObjectRecord
    jsr WaitByArgument093F
    rts

; @routine 093F:766 WaitByArgument093F
; @run 71:D766
; WHAT IT DOES: The wait OBJ_START_ARG picks.
; HOW: A table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WaitByArgument093FWaits = $D770 ; wait by argument
WaitByArgument093F:
    ldy a:OBJ_START_ARG,x
    lda a:WaitByArgument093FWaits,y
    sta a:OBJ_PIECE_NEXT,x
    rts

; @routine 093F:778 WalkLeftOnLanding093F
; @run 71:D778
; WHAT IT DOES: On landing (height $0A) goes on walking left at -2.
; HOW: StopOnLanding.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkLeftOnLanding093F:
    lda #$0A
    jsr VecStopOnLanding
    bcc WalkLeftOnLanding093F_at0
    inc a:OBJ_STATE,x
    lda #$FE
    sta a:OBJ_VX_HI,x
WalkLeftOnLanding093F_at0:
    rts

; @routine 093F:788 WalkAndPause093F
; @run 71:D788
; @stop D7D3
; WHAT IT DOES: Steps the list OBJ_START_ARG picks and walks with the ground, turning at walls ($D810), off the ground to state 1; the wait over, state 3.
; HOW: Frames $86-$87 do not move.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WalkAndPause093FLists = $D7D3 ; list address by argument
WalkAndPause093F:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:WalkAndPause093FLists,y
    sta z:zTemp10
    lda a:WalkAndPause093FLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc WalkAndPause093F_at0
    lda a:OBJ_FRAME,x
    cmp #$86
    beq WalkAndPause093F_at0
    cmp #$87
    beq WalkAndPause093F_at0
    lda #$0A
    jsr VecMoveAndLand
    bcs TurnBack093F
    and #$01
    bne WalkAndPause093F_at0
    stz a:OBJ_VX_HI,x
    lda #$01
    sta a:OBJ_STATE,x
    rts
WalkAndPause093F_at0:
    lda a:OBJ_PIECE_NEXT,x
    cmp #$FF
    beq WalkAndPause093F_at1
    dec a:OBJ_PIECE_NEXT,x
    bne WalkAndPause093F_at1
    jsr WaitByArgument093F
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
WalkAndPause093F_at1:
    rts

; @routine 093F:810 TurnBack093F
; @run 71:D810
; WHAT IT DOES: Turns the speed and facing about.
; HOW: NegateVelocityX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
TurnBack093F:
    jsr VecNegateVelocityX
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_FACING_LEFT,x
    rts

; @routine 093F:4DF FlyDagger093F
; @run 71:D4DF
; WHAT IT DOES: Steps the list at $D511 and takes the hit box the frame names; cleared at the list's end.
; HOW: A box table by frame from $2B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyDagger093FList = $D511 ; a frame list
FlyDagger093FBoxes = $D71D ; four bytes by frame
FlyDagger093F:
    lda #$11
    sta z:zTemp10
    lda #$D5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne FlyDagger093F_at0
    jmp VecClearObject
FlyDagger093F_at0:
    lda a:OBJ_FRAME,x
    sec
    sbc #$2B
    asl a
    asl a
    tay
    lda a:FlyDagger093FBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:FlyDagger093FBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:FlyDagger093FBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:FlyDagger093FBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts
