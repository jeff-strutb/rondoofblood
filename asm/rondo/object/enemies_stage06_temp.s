; @routine 072A:235 ContactWithPlayer
; @run 70:AA35
; WHAT IT DOES: Tests whether the object, a block that carries or bars the player, touches the player
; (slot 0): the width is in zTemp08 and the height in zTemp09. Says touching with C set (the player
; stands on its top), and sets the player's push: zPlayerPush bits for the side or top touched, the
; push depth, the way the player is pushed and PlayerAnchorObject.
; HOW: The player's falling speed (not rising) is required; the horizontal distance is made positive
; (the object's hitbox offset turned by its facing); the width must reach it; the vertical distance is
; made positive and compared against the height band above and below the top; the first match decides
; the side. The paths that ran in no script: the player a character other than 0 (width less 4, A$AA3E),
; the push depth when the width is 3 or more on the left side (AAF8) and on the right (AB0A).
; Curated cases: tools/rondo/curated/contact_with_player.txt (traced arms; 737 of 1240 cases checked, none differ).
; WAITS: no.
; Entry: X object, zTemp08 width, zTemp09 height. Exit: C set when standing on it; A and N, Z as the last
; load or compare left them; V as the last add or subtract.
ContactWithPlayer:
    jsr VecSideOfPlayer
    sta z:zTemp0E
    lda z:zCharacter
    beq ContactWithPlayer_at0
    lda z:zTemp08
    sec
    sbc #$04
    sta z:zTemp08
ContactWithPlayer_at0:
    lda a:OBJ_VY_HI
    bpl ContactWithPlayer_at1
    jmp ContactWithPlayer_at2
ContactWithPlayer_at1:
    lda a:OBJ_3930,x
    sta z:zTemp0D
    lda a:OBJ_2AC0,x
    beq ContactWithPlayer_at3
    lda z:zTemp0D
    eor #$FF
    inc a
    sta z:zTemp0D
ContactWithPlayer_at3:
    lda z:zTemp09
    clc
    adc #$05
    sta z:zTemp0A
    lda z:zTemp09
    sec
    sbc #$04
    sta z:zTemp0B
    lda a:OBJ_X_LO,x
    clc
    adc z:zTemp0D
    sec
    sbc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    sbc a:OBJ_X_HI
    sta z:zTemp01
    bpl ContactWithPlayer_at4
    lda z:zTemp00
    eor #$FF
    clc
    adc #$01
    sta z:zTemp00
    lda z:zTemp01
    eor #$FF
    adc #$00
    sta z:zTemp01
ContactWithPlayer_at4:
    lda z:zTemp01
    bne ContactWithPlayer_at2
    lda z:zTemp08
    cmp z:zTemp00
    bcc ContactWithPlayer_at2
    lda a:OBJ_Y_LO,x
    sec
    sbc a:OBJ_Y_LO
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    sbc a:OBJ_Y_HI
    sta z:zTemp01
    bpl ContactWithPlayer_at5
    lda z:zTemp02
    eor #$FF
    clc
    adc #$01
    sta z:zTemp02
    lda z:zTemp01
    eor #$FF
    adc #$00
    sta z:zTemp01
ContactWithPlayer_at5:
    lda z:zTemp01
    bne ContactWithPlayer_at2
    lda z:zTemp0E
    and #$02
    beq ContactWithPlayer_at6
    lda z:zTemp02
    cmp z:zTemp09
    bcc ContactWithPlayer_at7
ContactWithPlayer_at2:
    clc
    rts
ContactWithPlayer_at6:
    lda z:zTemp02
    cmp z:zTemp0B
    bcc ContactWithPlayer_at7
    cmp z:zTemp0A
    bcc ContactWithPlayer_at8
    clc
    rts
ContactWithPlayer_at7:
    lda z:zTemp08
    sec
    sbc z:zTemp00
    sta z:zTemp08
    lda z:zTemp0E
    and #$01
    bne ContactWithPlayer_at9
    lda z:zPlayerPush
    ora #$02
    sta z:zPlayerPush
    lda z:zTemp08
    cmp #$03
    bcc ContactWithPlayer_at10
    lda #$02
ContactWithPlayer_at10:
    sta z:zPlayerPushDepth
    clc
    rts
ContactWithPlayer_at9:
    lda z:zPlayerPush
    ora #$04
    sta z:zPlayerPush
    lda z:zTemp08
    cmp #$03
    bcc ContactWithPlayer_at11
    lda #$05
    sta z:zPlayerPushDepth
    clc
    rts
ContactWithPlayer_at11:
    lda #$07
    sec
    sbc z:zTemp08
    sta z:zPlayerPushDepth
    clc
    rts
ContactWithPlayer_at8:
    lda z:zTemp00
    cmp z:zTemp08
    beq ContactWithPlayer_at2
    stx a:PlayerAnchorObject
    lda a:OBJ_Y_LO,x
    sec
    sbc a:OBJ_Y_LO
    sta z:zTemp02
    lda z:zPlayerPush
    ora #$01
    sta z:zPlayerPush
    lda z:zTemp09
    sec
    sbc z:zTemp02
    sta z:zFieldCA
    sec
    rts

; @routine 072B:11B RisingDriftBlock
; @run 70:B11B
; @stop B144
; WHAT IT DOES: A block that drifts up and down: from state 0 it loads its record, shows frame $12,
; takes its start height from a table by its next kind (two bytes each, at $B144) and an upward speed
; of $FF80; in any later state it goes on as MoveAndWrapDriftA.
; HOW: drift_start with this block's frame, table and speed; tables read from the disc (bank 70).
; WAITS: no.
; Entry: X object. Exit: started: A $80, Y the table index, C bit 7 of the next kind, N set, Z clear;
; else as MoveAndWrapDriftA.
RisingDriftBlock:
    lda a:OBJ_27A8,x
    bne MoveAndWrapDriftA
    jsr VecLoadObjectRecord
    lda #$12
    sta a:OBJ_2A68,x
    lda a:OBJ_3040,x
    asl a
    tay
    lda a:$B145,y
    sta a:OBJ_Y_HI,x
    lda a:$B144,y
    sta a:OBJ_Y_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    rts

; @routine 072B:14C MoveAndWrapDriftA
; @run 70:B14C
; WHAT IT DOES: The later frames of RisingDriftBlock: moves by the Y speed and wraps the position.
; HOW: ApplyVelocityY, WrapVerticalDrift.
; WAITS: no.
; Entry: X object. Exit: the move's flags with the wrap's compare or load over them.
MoveAndWrapDriftA:
    jsr VecApplyVelocityY
    jsr WrapVerticalDrift
    rts

; @routine 072B:153 WrapVerticalDrift
; @run 70:B153
; WHAT IT DOES: Keeps a drifting object's Y position between two wrap points: at high byte $FF with
; the low byte under $80 it jumps to high byte 1; at high byte 1 with the low byte of $81 or more it
; jumps back to $FF.
; HOW: Compares the high byte with $FF and with 1, then the low byte, and stores the new high byte.
; WAITS: no.
; Entry: X object. Exit: A and flags of the last compare or load.
WrapVerticalDrift:
    lda a:OBJ_Y_HI,x
    cmp #$FF
    beq WrapVerticalDrift_at0
    cmp #$01
    beq WrapVerticalDrift_at1
    rts
WrapVerticalDrift_at0:
    lda a:OBJ_Y_LO,x
    cmp #$80
    bcs WrapVerticalDrift_at2
    lda #$01
    sta a:OBJ_Y_HI,x
    rts
WrapVerticalDrift_at1:
    lda a:OBJ_Y_LO,x
    cmp #$81
    bcc WrapVerticalDrift_at2
    lda #$FF
    sta a:OBJ_Y_HI,x
WrapVerticalDrift_at2:
    rts

; @routine 072B:179 LevelDriftBlock
; @run 70:B179
; @stop B1A2
; WHAT IT DOES: The same drifting block with frame $13, its start from the table at $B1A2 and a
; resting Y speed of $0080 (it drifts down).
; HOW: drift_start; later frames go on as MoveAndWrapDriftB.
; WAITS: no.
; Entry: X object. Exit: as RisingDriftBlock.
LevelDriftBlock:
    lda a:OBJ_27A8,x
    bne MoveAndWrapDriftB
    jsr VecLoadObjectRecord
    lda #$13
    sta a:OBJ_2A68,x
    lda a:OBJ_3040,x
    asl a
    tay
    lda a:$B1A3,y
    sta a:OBJ_Y_HI,x
    lda a:$B1A2,y
    sta a:OBJ_Y_LO,x
    lda #$00
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    rts

; @routine 072B:1AA MoveAndWrapDriftB
; @run 70:B1AA
; WHAT IT DOES: The later frames of LevelDriftBlock: as MoveAndWrapDriftA.
; HOW: ApplyVelocityY, WrapVerticalDrift.
; WAITS: no.
; Entry and exit: as MoveAndWrapDriftA.
MoveAndWrapDriftB:
    jsr VecApplyVelocityY
    jsr WrapVerticalDrift
    rts

; @routine 072B:1C9 StartRisingPiece
; @run 70:B1C9
; WHAT IT DOES: Starts a piece that rises: record loaded, Y speed $FF80, frame $14.
; HOW: LoadObjectRecord and three stores.
; WAITS: no.
; Entry: X object. Exit: A $14, Y 11, C from the record, N and Z from $14.
StartRisingPiece:
    jsr VecLoadObjectRecord
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$14
    sta a:OBJ_2A68,x
    rts

; @routine 072B:1DC StepPieceOnParent
; @run 70:B1DC
; WHAT IT DOES: A piece that rides the object 8 slots before it: steps its animation (list $B203), drops
; a piece every 32 frames, takes the parent's place and, when the player touches it (width $14, height
; $1D), moves on one state.
; HOW: StepObjectFrames, DropPieceEvery32, CopyObjectPlaceBack from slot less 8, ContactWithPlayer.
; WAITS: no.
; Entry: X object. Exit: see the wrapper; the contact's registers, then N and Z of the state when touched.
StepPieceOnParent:
    lda #$03
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr DropPieceEvery32
    txa
    sec
    sbc #$08
    tay
    jsr VecCopyObjectPlaceBack
    lda #$14
    sta z:zTemp08
    lda #$1D
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc StepPieceOnParent_at0
    inc a:OBJ_27A8,x
StepPieceOnParent_at0:
    rts

; @routine 072B:208 DropPieceEvery32
; @run 70:B208
; WHAT IT DOES: Every 32nd frame of the wave counter, starts an object of kind $AD at this object's
; place in the first free slot from $30.
; HOW: Wave counter AND $1F; FindFreeSlotFrom30; StartObjectAtPlace.
; Untraced: one byte (the later arm of the search).
; WAITS: no.
; Entry: X object. Exit: waiting: A the masked wave counter; no slot: as FindFreeSlotFrom30; started: as
; StartObjectAtPlace with C and Y from the search.
DropPieceEvery32:
    lda z:zWaveCounter
    and #$1F
    bne DropPieceEvery32_at0
    jsr VecFindFreeSlotFrom30
    bcc DropPieceEvery32_at0
    lda #$AD
    jsr VecStartObjectAtPlace
DropPieceEvery32_at0:
    rts

; @routine 072B:219 FallingPieceStep
; @run 70:B219
; WHAT IT DOES: FallingPieceMove with the animation (list $B203) stepped first and a piece dropped every 32
; frames.
; HOW: StepObjectFrames, DropPieceEvery32, then as FallingPieceMove.
; WAITS: no.
; Entry: X object. Exit: as FallingPieceMove.
FallingPieceStep:
    lda #$03
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr DropPieceEvery32
    jsr VecApplyVelocityY
    lda #$14
    sta z:zTemp08
    lda #$1D
    sta z:zTemp09
    jsr ContactWithPlayer
    bcs FallingPieceStep_at0
    dec a:OBJ_27A8,x
    rts
FallingPieceStep_at0:
    lda z:zPlayerPush
    ora #$21
    sta z:zPlayerPush
    lda a:OBJ_VY_HI,x
    sta z:zPushYHi
    lda a:OBJ_VY_LO,x
    sta z:zPushYLo
    rts

; @routine 072B:264 StartLevelPiece
; @run 70:B264
; WHAT IT DOES: Starts a piece that hangs: record loaded, Y speed $0080, frame $15.
; HOW: LoadObjectRecord and three stores.
; WAITS: no.
; Entry: X object. Exit: A $15, Y 11, C from the record, N and Z from $15.
StartLevelPiece:
    jsr VecLoadObjectRecord
    lda #$00
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$15
    sta a:OBJ_2A68,x
    rts

; @routine 072B:277 FollowParentUntilTouched
; @run 70:B277
; WHAT IT DOES: Takes the place of the object 8 slots before it and, when the player touches it, moves on
; one state.
; HOW: CopyObjectPlaceBack from slot less 8, ContactWithPlayer (width $14, height $1D).
; WAITS: no.
; Entry: X object. Exit: as StepPieceOnParent.
FollowParentUntilTouched:
    txa
    sec
    sbc #$08
    tay
    jsr VecCopyObjectPlaceBack
    lda #$14
    sta z:zTemp08
    lda #$1D
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc FollowParentUntilTouched_at0
    inc a:OBJ_27A8,x
FollowParentUntilTouched_at0:
    rts

; @routine 072B:290 FallingPieceMove
; @run 70:B290
; WHAT IT DOES: Moves a piece by its Y speed; if the player stands on it (width $14, height $1D) the
; player is pushed along by its speed, else the piece's state goes back one.
; HOW: ApplyVelocityY, ContactWithPlayer, DEC of the state or the push.
; WAITS: no.
; Entry: X object. Exit: the contact's registers, then the push's last load or the state's N and Z.
FallingPieceMove:
    jsr VecApplyVelocityY
    lda #$14
    sta z:zTemp08
    lda #$1D
    sta z:zTemp09
    jsr ContactWithPlayer
    bcs FallingPieceMove_at0
    dec a:OBJ_27A8,x
    rts
FallingPieceMove_at0:
    lda z:zPlayerPush
    ora #$21
    sta z:zPlayerPush
    lda a:OBJ_VY_HI,x
    sta z:zPushYHi
    lda a:OBJ_VY_LO,x
    sta z:zPushYLo
    rts
; @routine 072B:2B5 BreakIntoPieces
; @run 70:B2B5
; WHAT IT DOES: In state 0 loads the object's record. Once it has taken a hit of 5 or more it records its event, starts a kind $84 piece at its place in each of slots $3D to $3F with
; a speed row (four bytes each) from the table at $B30F, plays sound $19, starts a burst of debris (timer 1) and becomes a kind $9C object in state 0.
; HOW: LoadObjectRecord, RecordObjectEvent, ClearObjectInY and StartObjectAtPlace in a loop, SoundCommand, StartDebrisBurst.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: see the wrapper.
BreakIntoPieces:
    lda a:OBJ_27A8,x
    bne BreakIntoPieces_at0
    jsr VecLoadObjectRecord
    rts
BreakIntoPieces_at0:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc BreakIntoPieces_at1
    jsr VecRecordObjectEvent
    stz z:zTemp00
    ldy #$3D
BreakIntoPieces_at2:
    jsr VecClearObjectInY
    lda #$84
    jsr VecStartObjectAtPlace
    phx
    ldx z:zTemp00
    lda a:$B310,x
    sta a:OBJ_VX_HI,y
    lda a:$B30F,x
    sta a:OBJ_VX_LO,y
    lda a:$B312,x
    sta a:OBJ_VY_HI,y
    lda a:$B311,x
    sta a:OBJ_VY_LO,y
    plx
    lda z:zTemp00
    clc
    adc #$04
    sta z:zTemp00
    iny
    cpy #$40
    bcc BreakIntoPieces_at2
    lda #$19
    jsr VecSoundCommand
    lda #$01
    jsr StartDebrisBurst
    lda #$9C
    sta a:OBJ_2750,x
    stz a:OBJ_27A8,x
BreakIntoPieces_at1:
    rts

; @routine 072B:31B StartOnFirstState
; @run 70:B31B
; WHAT IT DOES: An object that only needs its record loaded: in state 0 it does LoadObjectRecord, otherwise nothing.
; HOW: A tail jump into LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: state 0: as LoadObjectRecord; else A the state, N and Z from it.
StartOnFirstState:
    lda a:OBJ_27A8,x
    bne StartOnFirstState_at0
    jmp VecLoadObjectRecord
StartOnFirstState_at0:
    rts

; @routine 072B:324 FallingDebris
; @run 70:B324
; WHAT IT DOES: A piece of debris: from state 0 it loads its record and takes frame $19 or $1A at random; later it
; falls under unlimited gravity and goes away once its Y high byte is not 0.
; HOW: NextRandom, ApplyGravityUnlimited, ApplyVelocityY, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object. Exit: state 0: A the frame, N and Z from it, C and V from the add; else the move's registers
; then the compare of the Y high byte.
FallingDebris:
    lda a:OBJ_27A8,x
    bne FallingDebris_at0
    jsr VecLoadObjectRecord
    jsr VecNextRandom
    and #$01
    clc
    adc #$19
    sta a:OBJ_2A68,x
    rts
FallingDebris_at0:
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    beq FallingDebris_at1
    jmp VecClearObject
FallingDebris_at1:
    rts

; @routine 072B:38D RaiseStateOnFlag
; @run 70:B38D
; WHAT IT DOES: When bit 7 of the object's status flags is set, an object in a state below 3 enters state 3 and
; clears OBJ_2800.
; HOW: Compares the state with 3.
; Untraced: every arm past the flag test (B394 to B39A) ran in no script.
; WAITS: no.
; Entry: X object. Exit: the last AND, compare or load.
RaiseStateOnFlag:
    lda a:OBJ_31A0,x
    and #$80
    beq RaiseStateOnFlag_at0
    lda a:OBJ_27A8,x
    cmp #$03
    bcs RaiseStateOnFlag_at0
    lda #$03
    sta a:OBJ_27A8,x
    stz a:OBJ_2800,x
RaiseStateOnFlag_at0:
    rts

; @routine 072B:3A4 FlashByHeight
; @run 70:B3A4
; WHAT IT DOES: Sets the object's flash flag: $86 while slot $10's flash flag is $86; else, when slot $10 is in state 2,
; a byte of a table at $B3CB picked by the object's height in sixteens; else 0.
; HOW: Reads slot $10's flash flag and state, then the object's Y low byte shifted four times.
; WAITS: no.
; Entry: X object. Exit: A the byte stored, N and Z from it; Y the table index when read.
FlashByHeight:
    lda a:$2B28
    cmp #$86
    beq FlashByHeight_at0
    lda a:$27B8
    cmp #$02
    bne FlashByHeight_at1
    lda a:OBJ_Y_LO,x
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    lda a:$B3CB,y
    sta a:OBJ_2B18,x
    rts
FlashByHeight_at1:
    stz a:OBJ_2B18,x
    rts
FlashByHeight_at0:
    lda #$86
    sta a:OBJ_2B18,x
    rts

; @routine 072B:427 StartWithEvent
; @run 70:B427
; WHAT IT DOES: Loads the object's record and records its event (when it is one the save remembers).
; HOW: LoadObjectRecord, RecordObjectEvent.
; WAITS: no.
; Entry: X object. Exit: as RecordObjectEvent.
StartWithEvent:
    jsr VecLoadObjectRecord
    jsr VecRecordObjectEvent
    rts

; @routine 072B:42E StartPieceChain
; @run 70:B42E
; WHAT IT DOES: Stops the object, clears four 192-byte work tables the overlay keeps at $B46E, $B52E,
; $B5EE and $B6AE, then starts 22 objects of kind $25 in the slots after this one, each at this object's
; place, each with its own number 0 to 21 in its life timer (the chain of pieces that follow it).
; HOW: Clears the four speed bytes and OBJ_2FE8; a loop of 192 stores to each table; a loop of 22 of
; ClearObjectInY and StartObjectAtPlace. The tables have no names yet.
; WAITS: no.
; Entry: X object. Exit: A 21, Y the slot plus 23, N 0, Z 1, C 1; V kept.
StartPieceChain:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_2FE8,x
    cla
    cly
StartPieceChain_at0:
    sta a:$B46E,y
    sta a:$B52E,y
    sta a:$B5EE,y
    sta a:$B6AE,y
    iny
    cpy #$C0
    bcc StartPieceChain_at0
    txa
    tay
    iny
    lda #$16
    sta z:zTemp00
    stz z:zTemp01
StartPieceChain_at1:
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    lda z:zTemp01
    sta a:OBJ_34B8,y
    iny
    inc z:zTemp01
    dec z:zTemp00
    bne StartPieceChain_at1
    rts

; @routine 072B:76E StepThenMusicAgain
; @run 70:B76E
; WHAT IT DOES: Steps the animation list at $B7AD; on the new frame with animation step 2 it sets bit 4 of zField8E and plays sound $FE; when the list ends it turns on the object's
; hitbox flag (2), clears bits 3 and 4 of zField8E, plays the music again (CdPlayAgain), clears OBJ_2F38, sets bit 2 of zField8E and bit 4 and 6 of OBJ_3148, and enters its next state.
; HOW: StepObjectFrames, SoundCommand, CdPlayAgain.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
StepThenMusicAgain:
    lda #$AD
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepThenMusicAgain_at0
    bcc StepThenMusicAgain_at1
    lda a:OBJ_3408,x
    cmp #$02
    bne StepThenMusicAgain_at1
    smb4 z:zField8E
    lda #$FE
    jsr VecSoundCommand
    rts
StepThenMusicAgain_at0:
    lda #$02
    sta a:OBJ_3250,x
    rmb3 z:zField8E
    rmb4 z:zField8E
    jsr VecCdPlayAgain
    stz a:OBJ_FLAGS_2F38,x
    lda z:zField8E
    ora #$04
    sta z:zField8E
    lda a:OBJ_3148,x
    ora #$50
    sta a:OBJ_3148,x
    inc a:OBJ_27A8,x
StepThenMusicAgain_at1:
    rts

; @routine 072C:053 StartPieceStreamA
; @run 70:B853
; WHAT IT DOES: Starts a stream of pieces from one of two sides: sound $88, OBJ_3670 1, OBJ_3828 2, frame $10;
; the side (OBJ_37D0) is a random bit and picks the start position from a table at $B894 (four bytes a side);
; then StartPieceChain, OBJ_3778 0 and the next OBJ_2800 state.
; HOW: SoundCommand, NextRandom, tables read from the disc (bank 70).
; WAITS: no.
; Entry: X object. Exit: N and Z from the new state, C 1 from the chain's loop, V as the random's add left it.
StartPieceStreamA:
    lda #$88
    jsr VecSoundCommand
    lda #$01
    sta a:OBJ_3670,x
    lda #$02
    sta a:OBJ_3828,x
    lda #$10
    sta a:OBJ_2A68,x
    jsr VecNextRandom
    and #$01
    sta a:OBJ_37D0,x
    asl a
    asl a
    tay
    lda a:$B895,y
    sta a:OBJ_X_HI,x
    lda a:$B894,y
    sta a:OBJ_X_LO,x
    lda a:$B897,y
    sta a:OBJ_Y_HI,x
    lda a:$B896,y
    sta a:OBJ_Y_LO,x
    jsr StartPieceChain
    stz a:OBJ_3778,x
    inc a:OBJ_2800,x
    rts

; @routine 072C:09C OrbitingObject
; @run 70:B89C
; WHAT IT DOES: An object that circles: it shifts the others (ShiftObjectsX), sets its speed from its angle
; (OBJ_3778) with a length of $0400, adds a drift word from the table at $B8E3 by its side (OBJ_37D0),
; moves, turns its angle on by a byte from the table at $B8E7, and when its X high byte reaches $FE or 2
; it clears its OBJ_2800 flag.
; HOW: SetVelocityFromAngle, ApplyVelocity, tables from the disc (bank 70).
; Untraced: one byte (the second compare's exit).
; WAITS: no.
; Entry: X object. Exit: A the X high byte, N, Z and C from the last compare, Y the side, V from the angle's add.
OrbitingObject:
    jsr ShiftObjectsX
    stz z:zTemp00
    lda #$04
    sta z:zTemp01
    lda a:OBJ_3778,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_37D0,x
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:$B8E3,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:$B8E4,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocity
    ldy a:OBJ_37D0,x
    lda a:OBJ_3778,x
    clc
    adc a:$B8E7,y
    sta a:OBJ_3778,x
    lda a:OBJ_X_HI,x
    cmp #$FE
    beq OrbitingObject_at0
    cmp #$02
    beq OrbitingObject_at0
    rts
OrbitingObject_at0:
    stz a:OBJ_2800,x
    rts

; @routine 072C:0E9 ShiftObjectsX
; @run 70:B8E9
; WHAT IT DOES: On every second frame of the object's life timer, moves every object slot from $10 to $26
; along X by a signed 16-bit step from the table at $B912, picked by the object's OBJ_37D0 (0 or 1).
; HOW: The table index is kept in X while the loop runs (X is pushed and pulled).
; WAITS: no.
; Entry: X object. Exit: odd timer: A 1, N 0, Z 0; else A the last high byte, Y $27, C 1, N and Z from X, V
; from the last add.
ShiftObjectsX:
    lda a:OBJ_34B8,x
    and #$01
    bne ShiftObjectsX_at0
    lda a:OBJ_37D0,x
    asl a
    phx
    tax
    ldy #$10
ShiftObjectsX_at1:
    lda a:OBJ_X_LO,y
    clc
    adc a:$B912,x
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI,y
    adc a:$B913,x
    sta a:OBJ_X_HI,y
    iny
    cpy #$27
    bcc ShiftObjectsX_at1
    plx
ShiftObjectsX_at0:
    rts

; @routine 072C:116 StartPieceStreamB
; @run 70:B916
; WHAT IT DOES: Starts a stream of pieces from the top: OBJ_3828 2, sound $89, animation reset, X 0 with a
; random low byte, Y $0110, StartPieceChain, then random X and Y speeds from tables at $B970 and $B978 (two
; bytes each, four of them) and the next OBJ_2800 state; when the object is on the player's left
; (SideOfPlayer bit 0) its X speed is reversed.
; HOW: SoundCommand, NextRandom three times, SideOfPlayer, NegateVelocityX.
; WAITS: no.
; Entry: X object. Exit: A the side bits (or the reversed speed's high byte), N and Z from it, C and V from the
; reversal when made, else C from the side and V from the last random.
StartPieceStreamB:
    lda #$02
    sta a:OBJ_3828,x
    lda #$89
    jsr VecSoundCommand
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    stz a:OBJ_X_HI,x
    jsr VecNextRandom
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_Y_HI,x
    lda #$10
    sta a:OBJ_Y_LO,x
    jsr StartPieceChain
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$B971,y
    sta a:OBJ_VX_HI,x
    lda a:$B970,y
    sta a:OBJ_VX_LO,x
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$B979,y
    sta a:OBJ_VY_HI,x
    lda a:$B978,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_2800,x
    jsr VecSideOfPlayer
    and #$01
    beq StartPieceStreamB_at0
    jsr VecNegateVelocityX
StartPieceStreamB_at0:
    rts

; @routine 072C:180 StepFallingPieceStream
; @run 70:B980
; WHAT IT DOES: One frame of a falling piece stream: steps the animation list at $B9B3, sets the hitbox
; flag by its direction, lifts the other objects (LiftObjectsBySlowFraction), adds $002C to its Y speed, moves,
; and when its Y high byte reaches 8 clears its flash flag and OBJ_2800.
; HOW: StepObjectFrames, HitboxByVerticalDirection, LiftObjectsBySlowFraction, ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: A the Y high byte, N, Z and C from its compare with 8,
; V from the move; Y as the lift left it ($27).
StepFallingPieceStream:
    lda #$B3
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr HitboxByVerticalDirection
    jsr LiftObjectsBySlowFraction
    lda a:OBJ_VY_LO,x
    clc
    adc #$2C
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    cmp #$08
    bne StepFallingPieceStream_at0
    stz a:OBJ_2B18,x
    stz a:OBJ_2800,x
StepFallingPieceStream_at0:
    rts

; @routine 072C:1BE HitboxByVerticalDirection
; @run 70:B9BE
; WHAT IT DOES: Sets the object's hitbox flag from a two-byte table at $B9CD by whether it is moving up
; (its Y speed's top bit): index 1 when rising, 0 otherwise.
; HOW: Two ROLs take the top bit to bit 0; the table is read from the disc (bank 70).
; WAITS: no.
; Entry: X object. Exit: A the table byte, Y the index, N and Z from A, C bit 6 of the Y speed.
HitboxByVerticalDirection:
    lda a:OBJ_VY_HI,x
    rol a
    rol a
    and #$01
    tay
    lda a:$B9CD,y
    sta a:OBJ_3250,x
    rts

; @routine 072C:1CF LiftObjectsBySlowFraction
; @run 70:B9CF
; WHAT IT DOES: For each object slot from $10 to $26, adds $E0 to this object's Y fraction and carries
; the sum into that slot's Y position (low then high byte).
; HOW: A loop of 23, the carry of each fraction add running through the two position adds.
; WAITS: no.
; Entry: X object. Exit: A the last high byte, Y $27, N 0, Z 1, C 1 from the loop's compare, V from the
; last add.
LiftObjectsBySlowFraction:
    ldy #$10
LiftObjectsBySlowFraction_at0:
    lda a:OBJ_Y_FRAC,x
    clc
    adc #$E0
    sta a:OBJ_Y_FRAC,x
    lda a:OBJ_Y_LO,y
    adc #$00
    sta a:OBJ_Y_LO,y
    lda a:OBJ_Y_HI,y
    adc #$00
    sta a:OBJ_Y_HI,y
    iny
    cpy #$27
    bcc LiftObjectsBySlowFraction_at0
    rts

; @routine 072C:1F0 StartPieceStreamC
; @run 70:B9F0
; WHAT IT DOES: Starts a stream of pieces from the bottom: OBJ_3828 3, OBJ_3510 0, a random side (OBJ_37D0)
; picks X from the table at $BA3E and the X speed from $BA40; Y is $0110; StartPieceChain; Y speed $F7;
; frame $80, animation reset, OBJ_3670 2, OBJ_36C8 $30 and the next OBJ_2800 state.
; HOW: NextRandom, tables from the disc (bank 70).
; WAITS: no.
; Entry: X object. Exit: N and Z from the new state, C 1 from the chain's loop, V as the random's add left it.
StartPieceStreamC:
    lda #$03
    sta a:OBJ_3828,x
    stz a:OBJ_3510,x
    jsr VecNextRandom
    and #$01
    sta a:OBJ_37D0,x
    tay
    lda a:RichterItemAttacks+6,y
    sta a:OBJ_X_LO,x
    stz a:OBJ_X_HI,x
    lda #$01
    sta a:OBJ_Y_HI,x
    lda #$10
    sta a:OBJ_Y_LO,x
    jsr StartPieceChain
    ldy a:OBJ_37D0,x
    lda a:RichterItemAttacks+8,y
    sta a:OBJ_VX_HI,x
    lda #$F7
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_2A68,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda #$02
    sta a:OBJ_3670,x
    lda #$30
    sta a:OBJ_36C8,x
    inc a:OBJ_2800,x
    rts

; @routine 072C:242 AccelerateAndSway
; @run 70:BA42
; WHAT IT DOES: A thing that falls and sways: its Y speed gains $002C, it moves, its X speed gains a drift
; word from the table at $BA7C by its side (OBJ_37D0); when its counter OBJ_36C8 runs out it takes
; OBJ_3670 3 and moves on one OBJ_2800 state.
; HOW: Adds, ApplyVelocity, DEC of the counter.
; WAITS: no.
; Entry: X object. Exit: counter not zero: N and Z from it, C and V from the X speed's add; zero: A 3, N and Z
; from the new state.
AccelerateAndSway:
    lda a:OBJ_VY_LO,x
    clc
    adc #$2C
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_37D0,x
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:$BA7C,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:$BA7D,y
    sta a:OBJ_VX_HI,x
    dec a:OBJ_36C8,x
    bne AccelerateAndSway_at0
    lda #$03
    sta a:OBJ_3670,x
    inc a:OBJ_2800,x
AccelerateAndSway_at0:
    rts

; @routine 072C:280 StepThenCount
; @run 70:BA80
; WHAT IT DOES: Steps the animation list at $BA96; when the list ends, sets the counter OBJ_36C8 to $80 and
; moves on one OBJ_2800 state.
; HOW: StepObjectFrames, then the stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: N and Z from the new state.
StepThenCount:
    lda #$96
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenCount_at0
    lda #$80
    sta a:OBJ_36C8,x
    inc a:OBJ_2800,x
StepThenCount_at0:
    rts

; @routine 072C:29F StepPieceEmitter
; @run 70:BA9F
; WHAT IT DOES: An object that sheds pieces while it plays its animation (list $BAE6): when the list ends it
; moves on one OBJ_2800 state; otherwise, every fourth frame of its life timer, once its animation step is
; $10 or more, on frame $13 and while OBJ_3510 is under 4, it starts an object of kind $26 at its place in the
; first free slot from $20, copies its facing and gives it the count OBJ_3510, which then goes up one.
; HOW: StepObjectFrames, FindFreeSlotFrom20, StartObjectAtPlace.
; Untraced: one byte.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: the last compare, load or INC.
StepPieceEmitter:
    lda #$E6
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq StepPieceEmitter_at0
    lda a:OBJ_34B8,x
    and #$03
    bne StepPieceEmitter_at1
    lda a:OBJ_3408,x
    cmp #$10
    bcc StepPieceEmitter_at1
    lda a:OBJ_2A68,x
    cmp #$13
    bne StepPieceEmitter_at1
    lda a:OBJ_3510,x
    cmp #$04
    bcs StepPieceEmitter_at1
    jsr VecFindFreeSlotFrom20
    bcc StepPieceEmitter_at1
    lda #$26
    jsr VecStartObjectAtPlace
    lda a:OBJ_2AC0,x
    sta a:OBJ_2AC0,y
    lda a:OBJ_3510,x
    sta a:OBJ_34B8,y
    inc a:OBJ_3510,x
    rts
StepPieceEmitter_at0:
    inc a:OBJ_2800,x
StepPieceEmitter_at1:
    rts

; @routine 072C:329 StepThenStop
; @run 70:BB29
; WHAT IT DOES: Steps the animation list at $BB45; when the list ends, sets OBJ_3670 to 4, stops the Y
; speed and moves on one OBJ_2800 state.
; HOW: StepObjectFrames, then the stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: A 4, N and Z from
; the new state.
StepThenStop:
    lda #$45
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepThenStop_at0
    lda #$04
    sta a:OBJ_3670,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_2800,x
StepThenStop_at0:
    rts

; @routine 072C:34C FallUntilLevel2
; @run 70:BB4C
; WHAT IT DOES: Applies gravity and moves the object down; when its Y high byte is 2 its OBJ_2800 is cleared.
; HOW: ApplyGravity, ApplyVelocityY, a compare.
; WAITS: no.
; Entry: X object. Exit: A the Y high byte, N, Z and C of the compare with 2, V from the move.
FallUntilLevel2:
    jsr VecApplyGravity
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    cmp #$02
    bne FallUntilLevel2_at0
    stz a:OBJ_2800,x
FallUntilLevel2_at0:
    rts

; @routine 072C:5AB JumpByStageStep
; @run 70:BDAB
; WHAT IT DOES: Goes to the code a table at $BDBD names for the stage's step counter OBJ_3670 slot $10.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: nothing. Exit: as the code jumped to; A the high byte, Y the doubled step.
JumpByStageStep:
    lda a:$3680
    asl a
    tay
    lda a:$BDBD,y
    sta z:zTemp00
    lda a:$BDBE,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 072C:5CD SetHitboxThenFollowPath
; @run 70:BDCD
; WHAT IT DOES: Sets the object's hitbox flag by its direction (HitboxByVerticalDirection) and goes on as FollowPathAndShow.
; HOW: HitboxByVerticalDirection, then the next routine's bytes.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as FollowPathAndShow.
SetHitboxThenFollowPath:
    jsr HitboxByVerticalDirection

; @routine 072C:5D0 FollowPathAndShow
; @run 70:BDD0
; WHAT IT DOES: Takes the object's speed from the four path tables (index from the table at $BE24 by its timer, plus 4), moves it and goes on as ShowByVelocity.
; HOW: Table reads (bank 70), ApplyVelocity, then the next routine's bytes.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ShowByVelocity, over the move's.
FollowPathAndShow:
    ldy a:OBJ_34B8,x
    lda a:$BE24,y
    clc
    adc #$04
    tay
    lda a:$B46E,y
    sta a:OBJ_VX_HI,x
    lda a:$B52E,y
    sta a:OBJ_VX_LO,x
    lda a:$B5EE,y
    sta a:OBJ_VY_HI,x
    lda a:$B6AE,y
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity

; @routine 072C:5F5 ShowByVelocity
; @run 70:BDF5
; WHAT IT DOES: Runs BurstAtHeightE0; the object's frame comes from a table at $BE39 by its timer, its hitbox half width and height from a table at $BE4F by that frame less one;
; when its X speed's high byte is not between 2 and $FD the frame goes up 7.
; HOW: BurstAtHeightE0, table reads (bank 70).
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: see the wrapper.
ShowByVelocity:
    jsr BurstAtHeightE0
    ldy a:OBJ_34B8,x
    lda a:$BE39,y
    sta a:OBJ_2A68,x
    sec
    sbc #$01
    tay
    lda a:$BE4F,y
    sta a:OBJ_3988,x
    sta a:OBJ_3A38,x
    lda a:OBJ_VX_HI,x
    cmp #$02
    bcc ShowByVelocity_at0
    cmp #$FE
    bcs ShowByVelocity_at0
    rts
ShowByVelocity_at0:
    lda a:OBJ_2A68,x
    clc
    adc #$07
    sta a:OBJ_2A68,x
    rts

; @routine 072C:656 BurstAtHeightE0
; @run 70:BE56
; WHAT IT DOES: When the object is on the first screen row and its Y low byte is in $E0 to $EF it plays sound $23 and starts a burst of debris (StartDebrisBurst, timer 0).
; HOW: Two compares, SoundCommand, a tail jump into StartDebrisBurst.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: no burst: the last compare or load; else as StartDebrisBurst.
BurstAtHeightE0:
    lda a:OBJ_Y_HI,x
    bne BurstAtHeightE0_at0
    lda a:OBJ_Y_LO,x
    and #$F0
    cmp #$E0
    bne BurstAtHeightE0_at0
    lda #$23
    jsr VecSoundCommand
    cla
    jmp StartDebrisBurst
BurstAtHeightE0_at0:
    rts

; @routine 072C:671 FollowPathB
; @run 70:BE71
; WHAT IT DOES: As FollowPathAndShow with the index table at $BE99, going on as ShowByVelocity by a jump.
; HOW: Table reads (bank 70), ApplyVelocity, a tail jump.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as FollowPathAndShow.
FollowPathB:
    ldy a:OBJ_34B8,x
    lda a:$BE99,y
    clc
    adc #$04
    tay
    lda a:$B46E,y
    sta a:OBJ_VX_HI,x
    lda a:$B52E,y
    sta a:OBJ_VX_LO,x
    lda a:$B5EE,y
    sta a:OBJ_VY_HI,x
    lda a:$B6AE,y
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    jmp ShowByVelocity

; @routine 072C:6AE StopVerticalSpeed
; @run 70:BEAE
; WHAT IT DOES: Sets the object's Y speed to 0.
; HOW: Two stores.
; WAITS: no.
; Entry: X object. Exit: all registers and flags kept.
StopVerticalSpeed:
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    rts
; @routine 072C:77B StartDriftingPiece
; @run 70:BF7B
; WHAT IT DOES: Loads the object's record, takes its X speed from a table at $BFA2 by its timer, a Y speed of 4, and reverses the X speed with sound $25 when slot $10 faces left.
; HOW: LoadObjectRecord, NegateVelocityX, SoundCommand.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: see the wrapper.
StartDriftingPiece:
    jsr VecLoadObjectRecord
    lda a:OBJ_34B8,x
    asl a
    tay
    lda a:$BFA3,y
    sta a:OBJ_VX_HI,x
    lda a:$BFA2,y
    sta a:OBJ_VX_LO,x
    lda #$04
    sta a:OBJ_VY_HI,x
    lda a:$2AD0
    beq StartDriftingPiece_at0
    jsr VecNegateVelocityX
    lda #$25
    jsr VecSoundCommand
StartDriftingPiece_at0:
    rts

; @routine 072C:7AA DropAndBurst
; @run 70:BFAA
; WHAT IT DOES: Steps the animation list at $BFEF and moves; once its Y low byte reaches $7F it sets it to $7F, plays sound $23, starts a burst of debris (timer 0), sets its hitbox
; to $10 by $0C, stops sideways, sets a Y speed of $FF00, resets the animation and enters its next state.
; HOW: StepObjectFrames, ApplyVelocity, SoundCommand, StartDebrisBurst.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
DropAndBurst:
    lda #$EF
    sta z:zTemp10
    lda #$BF
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$7F
    bcc DropAndBurst_at0
    lda #$7F
    sta a:OBJ_Y_LO,x
    lda #$23
    jsr VecSoundCommand
    cla
    jsr StartDebrisBurst
    lda #$10
    sta a:OBJ_3988,x
    lda #$0C
    sta a:OBJ_3A38,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FF
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    inc a:OBJ_27A8,x
DropAndBurst_at0:
    rts

; @routine 072C:7F4 StepFallAfterFour
; @run 70:BFF4
; WHAT IT DOES: Steps the animation list at $C010; once its animation step is 4 or more the hitbox goes off and the object falls (ApplyGravity, ApplyVelocity).
; HOW: StepObjectFrames, ApplyGravity, ApplyVelocity.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as ApplyVelocity when it falls; else the compare's flags.
StepFallAfterFour:
    lda #$10
    sta z:zTemp10
    lda #$C0
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_3408,x
    cmp #$04
    bcc StepFallAfterFour_at0
    stz a:OBJ_3250,x
    jsr VecApplyGravity
    jsr VecApplyVelocity
StepFallAfterFour_at0:
    rts
; @routine 072D:09F StartScriptsAndLists
; @run 71:C09F
; WHAT IT DOES: In state 0 starts script slot 4 with script $91 and loads the object's record; later it starts list channel 4 with the list $AE plus the low nibble of ScrollYLo.
; HOW: StartScriptSlotKind1, LoadObjectRecord (a tail jump), StartListChannelKind2; X is pushed around each start.
; WAITS: no.
; Entry: X object. Exit: state 0: as LoadObjectRecord; else A the channel's kind, N and Z from X, C and V from the add.
StartScriptsAndLists:
    lda a:OBJ_27A8,x
    bne StartScriptsAndLists_at0
    phx
    lda #$91
    ldx #$04
    jsr VecStartScriptSlotKind1
    plx
    jmp VecLoadObjectRecord
StartScriptsAndLists_at0:
    lda a:ScrollYLo
    and #$0F
    clc
    adc #$AE
    phx
    ldx #$04
    jsr VecStartListChannelKind2
    plx
    rts
