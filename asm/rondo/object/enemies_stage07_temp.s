; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; TEMPORARY: the stage 07 overlay's object routines whose enemy kind is not yet identified, kept
; together here until each kind has a file of its own.
; The disc tables the routines read are named here by their overlay address until the typed data
; layer reads them.
;
;
;
;
;
;
;
; @routine 0BFB:3DC EnterState4IfFlagged
; @run 70:B3DC
; WHAT IT DOES: Sends the object to state 4 when bit 7 of its status flags is set.
; HOW: Reads the flags once and masks bit 7; writes the state only when it is set.
; WAITS: no.
; Entry: X object. Exit: A the masked bit, or 4 when entered; N, Z from it; X, Y and C kept.
EnterState4IfFlagged:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq EnterState4IfFlagged_at0
    lda #$04
    sta a:OBJ_STATE,x
EnterState4IfFlagged_at0:
    rts

; @routine 0BFC:141 ScrollNotAtRightEnd
; @run 70:B941
; WHAT IT DOES: Says, in the carry, that the scroll has not come to the right end of the level:
; carry set when zField82 is set, when two or more screens remain, or when one screen remains
; and the scroll's low byte is under $E0; clear otherwise.
; HOW: Subtracts the first scroll entry's high screen byte from the scroll's limit
; (ScrollXMaxHi) and tests the difference.
; WAITS: no.
; Entry: nothing. Exit: C the answer; A and N, Z from the last load or compare; V from the
; subtract. Y and X kept. The arms past the screen difference of 0 or 1 ran in no script: the
; draft stops there and liftcmp judges only the first tests; the rest waits for coverage
; (PROCEDURE section 6).
ScrollNotAtRightEnd:
    lda z:zField82
    bne ScrollNotAtRightEnd_set
    lda a:ScrollXMaxHi
    sec
    sbc a:ScrollXHiScreen
    cmp #$02
    bcs ScrollNotAtRightEnd_out
    cmp #$00
    beq ScrollNotAtRightEnd_clear
    lda a:ScrollXLo
    cmp #$E0
    bcs ScrollNotAtRightEnd_clear
ScrollNotAtRightEnd_set:
    sec
ScrollNotAtRightEnd_out:
    rts
ScrollNotAtRightEnd_clear:
    clc
    rts

; @routine 0BFC:2AB SetHitboxSizeFromFrame
; @run 70:BAAB
; WHAT IT DOES: Sets the object's hit rectangle's half width and half height from a two byte
; row of a table by its frame minus $2A; frame 0 clears both.
; HOW: The table is at overlay address $BAC9.
; WAITS: no.
; Entry: X object. Exit: A the last byte loaded; N, Z from it; C bit 7 of the frame minus $2A
; (the shift); V from the subtract; Y the row's offset; X kept.
SetHitboxSizeFromFrame:
    lda a:OBJ_FRAME,x
    beq SetHitboxSizeFromFrame_clear
    sec
    sbc #$2A
    asl a
    tay
    lda a:$BAC9,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:$BACA,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts
SetHitboxSizeFromFrame_clear:
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0BFC:3B3 ChangeToType9C
; @run 70:BBB3
; WHAT IT DOES: Turns the object into one of type $9C, in state 0.
; HOW: Stores $9C in its kind byte and zero in its state.
; WAITS: no.
; Entry: X object. Exit: A $9C, N set, Z clear; X, Y and C kept.
ChangeToType9C:
    lda #$9C
    sta a:OBJ_KIND,x
ChangeToType9C_at0:
    stz a:OBJ_STATE,x
    rts

; @routine 0BFB:3E9 EnterNextStateWhenPlayerNear
; @run 70:B3E9
; WHAT IT DOES: When the player is nearer along X than the distance in the object's next_kind byte,
; sets f3250 to 2 and life_timer to $20 and moves the object on to its next state.
; HOW: DistanceXToPlayer compared with next_kind (read once).
; WAITS: no.
; Entry: X object, D clear. Exit: as the compare left them (V from the distance's subtract), or
; A $20 and N, Z from the new state; X, Y kept.
EnterNextStateWhenPlayerNear:
    jsr VecDistanceXToPlayer
    cmp a:OBJ_START_ARG,x
    bcs EnterNextStateWhenPlayerNear_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$20
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
EnterNextStateWhenPlayerNear_at0:
    rts

; @routine 0BFC:1C6 SwingOnCosine
; @run 70:B9C6
; WHAT IT DOES: Animates the object along the list at overlay address $B9F6, gives it a Y speed
; of its life_timer's angle's cosine (speed 1.0), moves it, and turns life_timer on by 2.
; HOW: StepObjectFrames, ScaleByCosine with zTemp00-01 set to 256, ApplyVelocity.
; WAITS: no.
; Entry: X object, D clear. Exit: A the new life_timer, N, Z, C and V from the add of 2; Y 0.
; Clobbers zTemp00-01, zTemp10-12, zProduct0-2, zMultiplierLo-Hi.
SwingOnCosine:
    lda #$F6
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    clc
    adc #$02
    sta a:OBJ_WORK_D,x
    rts

; @routine 0BFC:330 AnimateThenFrameRecord
; @run 70:BB30
; WHAT IT DOES: Animates the object along the list at overlay address $BB4B; when the list has
; ended, sets f3358 to 2 and f3300 to $0E and moves the object on to its next state.
; HOW: StepObjectFrames; its result says the list ended.
; WAITS: no.
; Entry: X object, D clear. Exit: as StepObjectFrames, or at the end A $0E and N, Z from the new
; state; X kept. Clobbers zTemp10-11.
AnimateThenFrameRecord:
    lda #$4B
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne AnimateThenFrameRecord_at0
    lda #$02
    sta a:OBJ_WORK_C,x
    lda #$0E
    sta a:OBJ_WORK_B,x
    inc a:OBJ_STATE,x
AnimateThenFrameRecord_at0:
    rts

; @routine 0BFF:1A3 StartThenAnimate
; @run 71:D1A3
; WHAT IT DOES: The first time the object runs (state 0), starts it from its record and sets its
; f2a10 to 6. Every time, it animates it along the list at overlay address $D1BC.
; HOW: LoadObjectRecord when the state is 0, then StepObjectFrames as the last call.
; WAITS: no.
; Entry: X object, D clear. Exit: as StepObjectFrames (Y 11 from the record start when the
; animation does not change it); X and V kept. (A JSR then RTS, not a tail jump.) Clobbers zTemp10-11.
StartThenAnimate:
    lda a:OBJ_STATE,x
    bne StartThenAnimate_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
StartThenAnimate_at0:
    lda #$D1
    sta z:zTemp11
    lda #$BC
    sta z:zTemp10
    jsr VecStepObjectFrames
    rts

; @routine 0BFC:247 RemoveFirstFreeSlotFrom1C
; @run 70:BA47
; WHAT IT DOES: Finds the first empty object slot from $1C to $1F and empties it fully (it is
; already empty; this is the slot search's tail without its start), carry set when it found one.
; HOW: A loop on the type byte; ClearObjectInY through its vector.
; WAITS: no.
; Entry: nothing, D clear. Exit: found: C 1, Y the slot, A 0, N 0, Z 1; none: C 0, Y $20, A the
; last type, N 0, Z 1. X kept.
RemoveFirstFreeSlotFrom1C:
    ldy #$1C
RemoveFirstFreeSlotFrom1C_at1:
    lda a:OBJ_KIND,y
    beq RemoveFirstFreeSlotFrom1C_at0
    iny
    cpy #$20
    bcc RemoveFirstFreeSlotFrom1C_at1
    clc
    rts
RemoveFirstFreeSlotFrom1C_at0:
    jsr VecClearObjectInY
    sec
    rts

; @routine 0BFC:27C SpawnPiecesWhenFlaggedHit
; @run 70:BA7C
; WHAT IT DOES: When the object is past state 0 and bit 7 of its status flags is set, spawns its
; eight pieces (SpawnEightPiecesType37) and enters state 4.
; HOW: Two tests, then the spawn.
; WAITS: no.
; Entry: X object, D clear. Exit: A 4 and N, Z clear, C, V and Y as the spawn left them; or A, N,
; Z from the failing test.
SpawnPiecesWhenFlaggedHit:
    lda a:OBJ_STATE,x
    beq SpawnPiecesWhenFlaggedHit_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq SpawnPiecesWhenFlaggedHit_at0
    jsr SpawnEightPiecesType37
    lda #$04
    sta a:OBJ_STATE,x
SpawnPiecesWhenFlaggedHit_at0:
    rts

; @routine 0BFC:291 SpawnEightPiecesType37
; @run 70:BA91
; WHAT IT DOES: Starts up to eight objects of type $37 at the object's place, each in a free slot
; from $30, each with its piece number in life_timer.
; HOW: A loop over pieces 0 to 7 (kept in zHitSlot): FindFreeSlotFrom30, StartObjectAtPlace.
; WAITS: no.
; Entry: X object, D clear. Exit: after eight pieces A the last piece number, N, Z, C from the
; compare of Y with 8, Y 8; when no slot was free as FindFreeSlotFrom30 left it. X kept.
SpawnEightPiecesType37:
    cly
SpawnEightPiecesType37_at1:
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom30
    bcc SpawnEightPiecesType37_at0
    lda #$37
    jsr VecStartObjectAtPlace
    lda z:zHitSlot
    sta a:OBJ_WORK_D,y
    ldy z:zHitSlot
    iny
    cpy #$08
    bcc SpawnEightPiecesType37_at1
SpawnEightPiecesType37_at0:
    rts

; @routine 0BFC:395 AnimateThenRemove
; @run 70:BB95
; WHAT IT DOES: Animates the object along the list at overlay address $BBA6; when the list has
; ended the object removes itself.
; HOW: StepObjectFrames; ClearObject as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: as StepObjectFrames. Clobbers zTemp10-11.
AnimateThenRemove:
    lda #$A6
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne AnimateThenRemove_at0
    jmp VecClearObject
AnimateThenRemove_at0:
    rts

; @routine 0BFC:3BC StartWithRecordTableRow
; @run 70:BBBC
; @stop BBF5
; WHAT IT DOES: The first time (state 0) starts the object from its record and gives it a row of
; a table at overlay address $BBF5 (eight bytes, picked by its life_timer): a first Y speed (the
; object is moved once by it), its frame, and its X and Y speeds. Past state 0 it runs
; FallAndLeaveWhenLow, which it jumps to.
; HOW: LoadObjectRecord; the row offset is kept on the stack across ApplyVelocityY.
; WAITS: no.
; Entry: X object, D clear. Exit: A the last row byte, N, Z from it, C and V from ApplyVelocityY,
; Y the row offset; past state 0 as FallAndLeaveWhenLow.
StartWithRecordTableRow:
    lda a:OBJ_STATE,x
    bne FallAndLeaveWhenLow
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    asl a
    tay
    phy
    lda a:$BBF5,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    ply
    lda a:$BBF6,y
    sta a:OBJ_FRAME,x
    lda a:$BBF8,y
    sta a:OBJ_VX_HI,x
    lda a:$BBF7,y
    sta a:OBJ_VX_LO,x
    lda a:$BBFA,y
    sta a:OBJ_VY_HI,x
    lda a:$BBF9,y
    sta a:OBJ_VY_LO,x
    rts

; @routine 0BFC:435 FallAndLeaveWhenLow
; @run 70:BC35
; WHAT IT DOES: Pulls the object down with gravity, moves it, and removes it when its Y high
; byte is not zero (it has fallen off the screen).
; HOW: ApplyGravityUnlimited, ApplyVelocity, a test; ClearObject as a tail jump.
; WAITS: no.
; Entry: X object, D clear. Exit: A the Y high byte, N, Z from it; C and V from ApplyVelocity.
FallAndLeaveWhenLow:
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    beq FallAndLeaveWhenLow_at0
    jmp VecClearObject
FallAndLeaveWhenLow_at0:
    rts

; @routine 0BFC:1FF CountTimerPairDownThenSpawn36
; @run 70:B9FF
; WHAT IT DOES: In state 0 it loads its record. Later it counts the sixteen bit value life_timer (high) and
; f3510 (low) down; when it is zero it frees the first empty slot of $1C to $1F, starts an object of type $36
; there at its place and sets the pair again from a table of word pairs at overlay address $BA3F by two bits of
; PlayerDistanceSum.
; HOW: RemoveFirstFreeSlotFrom1C, StartObjectAtPlace, PlayerDistanceSum. The arm with no empty slot ran in no
; script: the draft stops there, so liftcmp does not judge it.
; WAITS: no.
; Entry: X object, D clear. Exit: state 0: as LoadObjectRecord; counting: A the new high byte and N, Z, C, V from
; the borrow subtract; when none of the four slots is empty, as RemoveFirstFreeSlotFrom1C; otherwise A the low
; byte set, N, Z from it, C and V from the sum, Y the table index.
CountTimerPairDownThenSpawn36:
    lda a:OBJ_STATE,x
    bne CountTimerPairDownThenSpawn36_at0
    jmp VecLoadObjectRecord
CountTimerPairDownThenSpawn36_at0:
    lda a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    beq CountTimerPairDownThenSpawn36_at1
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    rts
CountTimerPairDownThenSpawn36_at1:
    jsr RemoveFirstFreeSlotFrom1C
    bcc AnimateThenState1
    lda #$36
    jsr VecStartObjectAtPlace
    jsr VecPlayerDistanceSum
    and #$03
    asl a
    tay
    lda a:RichterItemAttacks+8,y
    sta a:OBJ_WORK_D,x
    lda a:RichterItemAttacks+7,y
    sta a:OBJ_WORK_E,x
    rts

; @routine 0BFC:2E3 StartAtRandomSpeedTowardPlayer
; @run 70:BAE3
; WHAT IT DOES: Loads the record, gives the object an X speed (high byte) from a table at overlay address $BB20
; by three bits of PlayerDistanceSum and moves it along X. When the player is nearer than $18 along X it removes
; itself; otherwise it takes a speed from a table of word pairs at $BB28 by two bits of PlayerDistanceSum, faces
; the player's side (bit 0 of SideOfPlayer into facing_left) and reverses its speed when that bit is set.
; HOW: LoadObjectRecord, PlayerDistanceSum twice, ApplyVelocityX, DistanceXToPlayer, SideOfPlayer, NegateVelocityX.
; WAITS: no.
; Entry: X object, D clear. Exit: removed: A the distance, N, Z, C from its compare with $18; otherwise
; A, N, Z, C, V from NegateVelocityX when it ran, else A the side bit and N, Z from it.
StartAtRandomSpeedTowardPlayer:
    jsr VecLoadObjectRecord
    jsr VecPlayerDistanceSum
    and #$07
    tay
    lda a:$BB20,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    jsr VecDistanceXToPlayer
    cmp #$18
    bcs RichterLowColumnsBlocked
    jmp VecClearObject
    jsr VecPlayerDistanceSum
    and #$03
    asl a
    tay
    lda a:$BB29,y
    sta a:OBJ_VX_HI,x
    lda a:$BB28,y
    sta a:OBJ_VX_LO,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    beq StartAtRandomSpeedTowardPlayer_at0
    jsr VecNegateVelocityX
StartAtRandomSpeedTowardPlayer_at0:
    rts

; @routine 0BFC:15F StartFromScreenEdgeByScroll
; @run 70:B95F
; WHAT IT DOES: Loads the record, then puts the object at one edge of the screen heading across it: on the left
; edge (X $FFE0, speed $0140, facing left set) when the player's X low byte is $C0 or more, or between $40 and
; $BF when the scroll is not at its right end or the player faces left; otherwise on the right (X $0120, speed
; -$0140). Its Y is the player's, thrown up at -$1000 and moved once, kept at $30 or more.
; HOW: LoadObjectRecord, ScrollNotAtRightEnd, ApplyVelocityY.
; WAITS: no.
; Entry: X object, D clear. Exit: A $30 and N, Z clear when the player's Y low byte is below $30; otherwise A the
; player's Y low byte, N, Z, C from its compare with $30; V from ApplyVelocityY; Y 11.
StartFromScreenEdgeByScroll:
    jsr VecLoadObjectRecord
    lda a:OBJ_X_LO
    cmp #$40
    bcc StartFromScreenEdgeByScroll_at0
    cmp #$C0
    bcs StartFromScreenEdgeByScroll_at1
    bsr ScrollNotAtRightEnd
    bcc StartFromScreenEdgeByScroll_at1
    lda a:OBJ_FACING_LEFT
    beq StartFromScreenEdgeByScroll_at0
StartFromScreenEdgeByScroll_at1:
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$FF
    sta a:OBJ_X_HI,x
    lda #$E0
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
    bra PickTimersAtRandom
StartFromScreenEdgeByScroll_at0:
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$20
    sta a:OBJ_X_LO,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$C0
    sta a:OBJ_VX_LO,x
    lda a:OBJ_Y_HI
    sta a:OBJ_Y_HI,x
    lda a:OBJ_Y_LO
    sta a:OBJ_Y_LO,x
    lda #$F0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO
    cmp #$30
    bcs StartFromScreenEdgeByScroll_at2
    lda #$30
    sta a:OBJ_Y_LO,x
StartFromScreenEdgeByScroll_at2:
    rts
