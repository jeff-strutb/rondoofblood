; @routine 051D:302 FallingRubble051B
; @run 71:C302
; WHAT IT DOES: State 0 loads the record and turns the speed by the player's side. State 1 waits out its count (then becomes kind
; $9B), steps its list on odd counts, falls and lands (state on, attribute $10, Y down by it) or rises away at frame
; $62 up. Later states count out, starting a kind $50 object every eighth count.
; HOW: Branches by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallingRubble051BLists = $C38C ; list addresses by start argument
FallingRubble051B:
    lda a:OBJ_STATE,x
    bne FallingRubble051B_at1
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    beq FallingRubble051B_at0
    jsr VecNegateVelocityX
FallingRubble051B_at0:
    rts
FallingRubble051B_at1:
    dec a
    bne FallingRubble051B_at6
    dec a:OBJ_PIECE_NEXT,x
    bne FallingRubble051B_at4
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
FallingRubble051B_at2:
    rts
FallingRubble051B_at3:
    lda #$FF
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    jmp VecApplyVelocityY
FallingRubble051B_at4:
    lda a:OBJ_PIECE_NEXT,x
    lsr a
    bcc FallingRubble051B_at2
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:FallingRubble051BLists,y
    sta z:zTemp10
    lda a:FallingRubble051BLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq FallingRubble051B_at5
    lda a:OBJ_FRAME,x
    cmp #$62
    bcs FallingRubble051B_at3
    jsr VecApplyGravityUnlimited
    lda #$04
    jsr VecStopOnLanding
    bcc FallingRubble051B_at2
    inc a:OBJ_STATE,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    rts
FallingRubble051B_at5:
    jmp VecClearObject
FallingRubble051B_at6:
    dec a:OBJ_PIECE_NEXT,x
    beq FallingRubble051B_at5
    lda a:OBJ_PIECE_NEXT,x
    and #$07
    bne FallingRubble051B_at2
    jsr VecFindFreeSlotFrom30
    bcc FallingRubble051B_at2
    lda #$50
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    rts

; @routine 051D:3C2 RisingPuff051B
; @run 71:C3C2
; WHAT IT DOES: State 0 loads the record with sound $13 and a rise of -3; after that moves up and steps the list at $C3F4,
; clearing the object at its end.
; HOW: Branches by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RisingPuff051B:
    lda a:OBJ_STATE,x
    bne RisingPuff051B_at0
    jsr VecLoadObjectRecord
    lda #$13
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    rts
RisingPuff051B_at0:
    jsr VecApplyVelocityY
    lda #$F4
    sta z:zTemp10
    lda #$C3
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne RisingPuff051B_at1
    jmp VecClearObject
RisingPuff051B_at1:
    rts

; @routine 051D:40D LoadPictureByStartArg051B
; @run 71:C40D
; WHAT IT DOES: State 0 loads the record, takes the frame from OBJ_START_ARG and sprite set 6.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadPictureByStartArg051B:
    lda a:OBJ_STATE,x
    bne LoadPictureByStartArg051B_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
LoadPictureByStartArg051B_at0:
    rts

; @routine 051D:421 FadeOutWhenDoorHit051B
; @run 71:C421
; WHAT IT DOES: State 0 loads the record. With the door ready and not held, a hit count of 1 or 2 sets zField8C to $80 plus
; OBJ_START_ARG and sub-step 3, and fades the CD out.
; HOW: cd_fade $0C by a tail jump.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FadeOutWhenDoorHit051B:
    lda a:OBJ_STATE,x
    bne FadeOutWhenDoorHit051B_at0
    jmp VecLoadObjectRecord
FadeOutWhenDoorHit051B_at0:
    lda a:StageDoorHeld
    eor #$01
    and a:PlayerDoorReady
    beq FadeOutWhenDoorHit051B_at1
    lda a:OBJ_HIT_TAKEN,x
    beq FadeOutWhenDoorHit051B_at1
    cmp #$03
    bcs FadeOutWhenDoorHit051B_at1
    lda #$80
    ora a:OBJ_START_ARG,x
    sta z:zField8C
    lda #$03
    sta z:zSubStep
    lda #$0C
    jmp BiosCdFade
FadeOutWhenDoorHit051B_at1:
    rts

; @routine 051D:44D EndAfterScripts051B
; @run 71:C44D
; WHAT IT DOES: State 0 loads the record. State 1 waits for script slot 5's script to be $80, then starts the fade. From then on
; keeps the wave counter at 0 and the player's piece count 4, pose 2 and swoop count 0, and once script slot 5 is
; empty sets zField8C $80 and sub-step 3.
; HOW: Branches by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EndAfterScripts051B:
    lda a:OBJ_STATE,x
    bne EndAfterScripts051B_at1
    jsr VecLoadObjectRecord
EndAfterScripts051B_at0:
    rts
EndAfterScripts051B_at1:
    dec a
    bne EndAfterScripts051B_at2
    lda a:ScriptSlotScript+5
    cmp #$80
    bne EndAfterScripts051B_at0
    lda #$01
    sta z:zFadeActive
    inc a:OBJ_STATE,x
EndAfterScripts051B_at2:
    stz z:zWaveCounter
    lda #$04
    sta a:OBJ_PIECE_PLACE
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_SWOOP_SPEED_LO
    lda a:ScriptSlotScript+5
    bne EndAfterScripts051B_at0
    lda #$80
    sta z:zField8C
    lda #$03
    sta z:zSubStep
    rts

; @routine 051D:484 StepListByStartArg051B
; @run 71:C484
; WHAT IT DOES: State 0 loads the record with sprite set 6; then steps the list the table at $C4A1 names for OBJ_START_ARG.
; HOW: A tail jump to StepObjectFrames.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepListByStartArg051BLists = $C4A1 ; list addresses
StepListByStartArg051B:
    lda a:OBJ_STATE,x
    bne StepListByStartArg051B_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
StepListByStartArg051B_at0:
    ldy a:OBJ_START_ARG,x
    lda a:StepListByStartArg051BLists,y
    sta z:zTemp10
    lda a:StepListByStartArg051BLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 051D:4C6 FlickerColoursThenDrawWater051B
; @run 71:C4C6
; WHAT IT DOES: Starts two scripts in slots 8 and 9 by the wave counter, then in state 0 loads the record and keeps the camera's
; place ($C4E8), else draws the water ($C4F5).
; HOW: StartScriptSlotKind4 twice with a number from the table at $C4B6.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlickerColoursThenDrawWater051BScripts = $C4B6 ; sixteen script numbers
FlickerColoursThenDrawWater051B:
    phx
    lda z:zWaveCounter
    lsr a
    and #$0F
    tax
    lda a:FlickerColoursThenDrawWater051BScripts,x
    ora #$80
    pha
    ldx #$08
    jsr VecStartScriptSlotKind4
    pla
    inc a
    ldx #$09
    jsr VecStartScriptSlotKind4
    plx
    lda a:OBJ_STATE,x
    bne DrawWaterTiles051B
    jsr VecLoadObjectRecord

; @routine 051D:4F5 DrawWaterTiles051B
; @run 71:C4F5
; WHAT IT DOES: Picks a wave place from the camera's movement and draws the water's tiles into VRAM from the bank $14 above the
; RAM base: eight and two of the moving rows ($C56B), then five merged rows ($C58F).
; HOW: MPR4 changed around the three loops; zTemp08-09 the source.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DrawWaterTiles051B:
    lda a:ScrollStepXLo+6
    eor a:ScrollStepXHi+6
    beq DrawWaterTiles051B_at0
    jsr VecNextRandom
    and #$03
    clc
    adc a:OBJ_PIECE_NEXT,x
    bra DrawWaterTiles051B_at1
DrawWaterTiles051B_at0:
    lda a:OBJ_PIECE_NEXT,x
DrawWaterTiles051B_at1:
    lsr a
    and #$0F
    eor #$0F
    sta z:zTemp00
    jsr CopyScrollToTimer
    tma #$10
    pha
    lda #$14
    clc
    adc z:zRamBaseBank
    tam #$10
    cly
DrawWaterTiles051B_at2:
    tya
    asl a
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp08
    lda z:zTemp00
    adc #$80
    sta z:zTemp09
    jsr CopyEightWordsToVram051B
    iny
    cpy #$08
    bne DrawWaterTiles051B_at2
DrawWaterTiles051B_at3:
    tya
    and #$07
    asl a
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp08
    lda z:zTemp00
    adc #$80
    sta z:zTemp09
    jsr CopyEightWordsToVram051B
    iny
    cpy #$0A
    bne DrawWaterTiles051B_at3
    cly
DrawWaterTiles051B_at4:
    lda z:zTemp00
    and #$03
    asl a
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp08
    cla
    adc #$90
    sta z:zTemp09
    jsr MergeEightWordsToVram051B
    iny
    cpy #$05
    bne DrawWaterTiles051B_at4
    pla
    tam #$10
    rts

; @routine 051D:56B CopyEightWordsToVram051B
; @run 71:C56B
; WHAT IT DOES: Copies the eight words at (zTemp08) to VRAM $1800 plus 16 times Y.
; HOW: Through the VDC data ports.
; WAITS: no.
; Entry: Y the row. Exit: Y kept.
CopyEightWordsToVram051B:
    jsr VecSelectVramWriteAddress
    tya
    asl a
    asl a
    asl a
    asl a
    sta a:VDC_DATA_LO
    st2 #$18
    jsr VecSelectVramData
    phy
    cly
CopyEightWordsToVram051B_at0:
    lda (zTemp08),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp08),y
    sta a:VDC_DATA_HI
    iny
    cpy #$10
    bne CopyEightWordsToVram051B_at0
    ply
    rts

; @routine 051D:58F MergeEightWordsToVram051B
; @run 71:C58F
; WHAT IT DOES: Writes the eight words at (zTemp08) to VRAM $18A0 plus 16 times Y, reading the VRAM word first each time.
; HOW: The read address set as the write address.
; WAITS: no.
; Entry: Y the row. Exit: Y kept.
MergeEightWordsToVram051B:
    jsr VecSelectVramWriteAddress
    tya
    asl a
    asl a
    asl a
    asl a
    adc #$A0
    sta a:VDC_DATA_LO
    st2 #$18
    jsr VecSelectVramReadAddress
    tya
    asl a
    asl a
    asl a
    asl a
    adc #$A0
    sta a:VDC_DATA_LO
    st2 #$18
    jsr VecSelectVramData
    phy
    cly
MergeEightWordsToVram051B_at0:
    lda a:VDC_DATA_LO
    lda (zTemp08),y
    sta a:VDC_DATA_LO
    iny
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    iny
    cpy #$10
    bne MergeEightWordsToVram051B_at0
    ply
    rts

; @routine 051D:5C8 LoadAndRecordFlipped051B
; @run 71:C5C8
; WHAT IT DOES: Loads the record, sets the player's sprite flip byte to $C0 and records the event.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadAndRecordFlipped051B:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_SPRITE_FLIP
    jmp VecRecordObjectEvent

; @routine 051D:5D3 CloseScrollOnce051B
; @run 71:C5D3
; WHAT IT DOES: Once zField8F has a bit of OBJ_START_ARG: limits the camera (X minimum 1 for bit 3, else maximum 3), sets
; zField8E bit 7 and records the event.
; HOW: Done once, kept in OBJ_PIECE_NEXT.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CloseScrollOnce051B:
    lda a:OBJ_STATE,x
    bne CloseScrollOnce051B_at0
    jsr VecLoadObjectRecord
CloseScrollOnce051B_at0:
    lda a:OBJ_PIECE_NEXT,x
    bne CloseScrollOnce051B_at3
    lda z:zField8F
    and a:OBJ_START_ARG,x
    beq CloseScrollOnce051B_at3
    cmp #$08
    bne CloseScrollOnce051B_at1
    lda #$01
    sta a:ScrollXMinHi
    bra CloseScrollOnce051B_at2
CloseScrollOnce051B_at1:
    lda #$03
    sta a:ScrollXMaxHi
CloseScrollOnce051B_at2:
    inc a:OBJ_PIECE_NEXT,x
    smb7 z:zField8E
    jsr VecRecordObjectEvent
CloseScrollOnce051B_at3:
    rts

; @routine 051D:600 MarkOtherWhenFlag051B
; @run 71:C600
; WHAT IT DOES: Once zField8F bit 3 is set, puts 8 into the low digit of the OBJ_START_ARG of the object OBJ_START_ARG names.
; HOW: Done once, kept in OBJ_PIECE_NEXT.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
MarkOtherWhenFlag051B:
    lda a:OBJ_STATE,x
    bne MarkOtherWhenFlag051B_at0
    jsr VecLoadObjectRecord
MarkOtherWhenFlag051B_at0:
    lda a:OBJ_PIECE_NEXT,x
    bne MarkOtherWhenFlag051B_at1
    lda z:zField8F
    and #$08
    beq MarkOtherWhenFlag051B_at1
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_START_ARG,y
    and #$F0
    ora #$08
    sta a:OBJ_START_ARG,y
    inc a:OBJ_PIECE_NEXT,x
MarkOtherWhenFlag051B_at1:
    rts

; @routine 051D:624 StartScriptWhenOtherAdvances051B
; @run 71:C624
; WHAT IT DOES: Once the object OBJ_START_ARG names is past state 1: starts in slot 8 the script the table at $9C1A holds for
; twice zField82 + 1 plus the character, and records the event.
; HOW: StartScriptSlotKind1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartScriptWhenOtherAdvances051BScripts = $9C1A ; a script per stage part and character
StartScriptWhenOtherAdvances051B:
    lda a:OBJ_STATE,x
    bne StartScriptWhenOtherAdvances051B_at0
    jmp VecLoadObjectRecord
StartScriptWhenOtherAdvances051B_at0:
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_STATE,y
    cmp #$02
    bcc StartScriptWhenOtherAdvances051B_at1
    phx
    lda z:zField82
    inc a
    asl a
    adc z:zCharacter
    tax
    lda a:StartScriptWhenOtherAdvances051BScripts,x
    ldx #$08
    jsr VecStartScriptSlotKind1
    plx
    jmp VecRecordObjectEvent
StartScriptWhenOtherAdvances051B_at1:
    rts

; @routine 051D:66E RaftOnWater051B
; @run 71:C66E
; WHAT IT DOES: A floating platform. State 0 loads the record as sprite set 6 frame 1 and keeps its map X. After: steps the list
; once started; at four in OBJ_KNOCK_FORWARD breaks up ($C7CF); else swings ($C6DE), moves sideways unless holding
; the player while the screen scrolls, checks for the player ($C736) and sinks on a new touch ($C781).
; HOW: The calls in turn; OBJ_WORK_K marks a touch that began this frame.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
RaftOnWater051B:
    lda a:OBJ_STATE,x
    bne RaftOnWater051B_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$01
    sta a:OBJ_FRAME,x
    lda a:ScrollXLo
    clc
    adc a:OBJ_X_LO,x
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:ScrollXHiScreen
    adc a:OBJ_X_HI,x
    sta a:OBJ_SWOOP_SPEED_LO,x
    rts
RaftOnWater051B_at0:
    lda a:OBJ_ANIM_STEP,x
    beq RaftOnWater051B_at1
    lda #$C6
    sta z:zTemp11
    lda #$4B
    sta z:zTemp10
    jsr VecStepObjectFrames
RaftOnWater051B_at1:
    lda a:OBJ_KNOCK_FORWARD,x
    cmp #$04
    bcc RaftOnWater051B_at2
    jmp BreakRaftIntoSix051B
RaftOnWater051B_at2:
    bsr SwingAndDrift051B
    lda a:OBJ_LOOP_FLAGS,x
    and #$01
    beq RaftOnWater051B_at3
    lda a:ScrollStepXLo
    ora a:ScrollStepXHi
    bne RaftOnWater051B_at4
RaftOnWater051B_at3:
    jsr VecApplyVelocityX
RaftOnWater051B_at4:
    stz a:OBJ_WORK_K,x
    lda a:OBJ_RAIN_COUNT,x
    sta a:OBJ_WORK_L,x
    jsr CarryTouchingPlayer051B
    lda a:OBJ_WORK_L,x
    eor a:OBJ_RAIN_COUNT,x
    and a:OBJ_RAIN_COUNT,x
    sta a:OBJ_WORK_K,x
    jsr SinkAndThrowOnTouch051B
    rts

; @routine 051D:6DE SwingAndDrift051B
; @run 71:C6DE
; WHAT IT DOES: X speed from the cosine of a 16-bit angle (OBJ_PIECE_PLACE:PIECE_NEXT) times $B0; every full turn corrects X by
; how far it has drifted from the kept map X.
; HOW: ScaleByCosine; the angle steps by $0180.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SwingAndDrift051B:
    lda #$B0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_PIECE_PLACE,x
    jsr VecScaleByCosine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VX_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VX_LO,x
    lda a:OBJ_PIECE_NEXT,x
    clc
    adc #$80
    sta a:OBJ_PIECE_NEXT,x
    lda a:OBJ_PIECE_PLACE,x
    adc #$01
    sta a:OBJ_PIECE_PLACE,x
    bcc SwingAndDrift051B_at0
    stz a:OBJ_PIECE_NEXT,x
    stz a:OBJ_PIECE_PLACE,x
    stz a:OBJ_X_FRAC,x
    lda a:ScrollXLo
    clc
    adc a:OBJ_X_LO,x
    sec
    sbc a:OBJ_SWOOP_SPEED_HI,x
    sta z:zTemp00
    lda a:ScrollXHiScreen
    adc a:OBJ_X_HI,x
    sbc a:OBJ_SWOOP_SPEED_LO,x
    sta z:zTemp01
    lda z:zTemp00
    eor #$FF
    inc a
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
SwingAndDrift051B_at0:
    rts

; @routine 051D:736 CarryTouchingPlayer051B
; @run 71:C736
; WHAT IT DOES: With the player touching (box $20 by $10): marks it in OBJ_RAIN_COUNT (and OBJ_WORK_L on a column hit) and, the
; player standing still, pushes them by the object's X speed (zPlayerPush bit 4) and sets loop flag bit 0; else
; clears that bit.
; HOW: ContactWithPlayer.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CarryTouchingPlayer051B:
    stz a:OBJ_RAIN_COUNT,x
    lda #$20
    sta z:zTemp08
    lda #$10
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc CarryTouchingPlayer051B_at1
    lda a:RichterColumnHit
    ora a:RichterColumnHit+1
    ora a:RichterColumnHit+2
    beq CarryTouchingPlayer051B_at0
    lda #$01
    sta a:OBJ_WORK_L,x
CarryTouchingPlayer051B_at0:
    lda #$01
    sta a:OBJ_RAIN_COUNT,x
    lda a:OBJ_VX_LO
    ora a:OBJ_VX_HI
    bne CarryTouchingPlayer051B_at1
    smb4 z:zPlayerPush
    lda a:OBJ_VX_LO,x
    sta z:zPushXLo
    lda a:OBJ_VX_HI,x
    sta z:zPushXHi
    lda a:OBJ_LOOP_FLAGS,x
    ora #$01
    sta a:OBJ_LOOP_FLAGS,x
    rts
CarryTouchingPlayer051B_at1:
    lda a:OBJ_LOOP_FLAGS,x
    and #$FE
    sta a:OBJ_LOOP_FLAGS,x
    rts

; @routine 051D:781 SinkAndThrowOnTouch051B
; @run 71:C781
; WHAT IT DOES: On a new touch, starts a bob (OBJ_WORK_J) and, a free slot found, starts a piece ($C7B7), counting up to four in
; OBJ_KNOCK_FORWARD; while bobbing, takes Y from the table at $C7E6.
; HOW: A 15-frame bob.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SinkAndThrowOnTouch051BBob = $C7E6 ; Y by bob step
SinkAndThrowOnTouch051B:
    lda a:OBJ_WORK_J,x
    bne SinkAndThrowOnTouch051B_at2
    lda a:OBJ_WORK_K,x
    beq SinkAndThrowOnTouch051B_at1
    inc a:OBJ_WORK_J,x
    lda #$01
    sta a:OBJ_ANIM_STEP,x
    inc a:OBJ_KNOCK_FORWARD,x
    jsr VecFindFreeSlotFrom30
    bcc SinkAndThrowOnTouch051B_at0
    jmp StartPiece4CBeside051B
SinkAndThrowOnTouch051B_at0:
    dec a:OBJ_KNOCK_FORWARD,x
SinkAndThrowOnTouch051B_at1:
    rts
SinkAndThrowOnTouch051B_at2:
    ldy a:OBJ_WORK_J,x
    cpy #$10
    beq SinkAndThrowOnTouch051B_at3
    lda a:SinkAndThrowOnTouch051BBob,y
    sta a:OBJ_Y_LO,x
    inc a:OBJ_WORK_J,x
    rts
SinkAndThrowOnTouch051B_at3:
    stz a:OBJ_WORK_J,x
    rts

; @routine 051D:7B7 StartPiece4CBeside051B
; @run 71:C7B7
; WHAT IT DOES: Kind $4C into $C7B9.
; HOW: One load, then on.
; WAITS: no.
; Entry: X object, Y a free slot. Exit: as $C7B9.
StartPiece4CBeside051B:
    lda #$4C

; @routine 051D:7B9 SetKindAndPlaceOfY051B
; @run 71:C7B9
; WHAT IT DOES: Gives slot Y the kind in A and this object's Y low and X.
; HOW: Four stores.
; WAITS: no.
; Entry: A kind, X object, Y the other slot. Exit: A the X high byte.
SetKindAndPlaceOfY051B:
    sta a:OBJ_KIND,y
    lda a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,y
    lda a:OBJ_X_LO,x
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI,x
    sta a:OBJ_X_HI,y
    rts

; @routine 051D:7CF BreakRaftIntoSix051B
; @run 71:C7CF
; WHAT IT DOES: Clears slots $10-$15 and starts a kind $4D piece in each at this place, sound $19, and clears this object.
; HOW: ClearObjectInY and SetKindAndPlaceOfY051B per slot.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BreakRaftIntoSix051B:
    ldy #$10
BreakRaftIntoSix051B_at0:
    jsr VecClearObjectInY
    lda #$4D
    bsr SetKindAndPlaceOfY051B
    iny
    cpy #$16
    bne BreakRaftIntoSix051B_at0
    lda #$19
    jsr VecSoundCommand
    jsr VecClearObject
    rts

; @routine 051D:7F6 DebrisFall051B
; @run 71:C7F6
; WHAT IT DOES: State 0 loads the record as sprite set 6 frame $37; after that falls by $28 a frame and is cleared on Y screen 1.
; HOW: A 16-bit add to the Y speed, then ApplyVelocityY and ApplyVelocityX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DebrisFall051B:
    lda a:OBJ_STATE,x
    bne DebrisFall051B_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$37
    sta a:OBJ_FRAME,x
    rts
DebrisFall051B_at0:
    lda a:OBJ_VY_LO,x
    clc
    adc #$28
    sta a:OBJ_VY_LO,x
    cla
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    jsr VecApplyVelocityX
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne DebrisFall051B_at1
    jmp VecClearObject
DebrisFall051B_at1:
    rts
