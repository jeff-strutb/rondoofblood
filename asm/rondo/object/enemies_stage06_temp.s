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
