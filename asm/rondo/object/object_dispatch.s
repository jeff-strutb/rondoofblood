; @routine 0259:313 DispatchByKind0259
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $AB3A holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0259's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind0259:
    lda a:OBJ_KIND,x
    beq DispatchByKind0259_at0
    dec a
    asl a
    tay
    lda a:$AB3A,y
    sta z:zTemp00
    lda a:$AB3B,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind0259_at0:
    rts

; @routine 0519:329 StepReleaseOnHit
; @run 70:A329
; WHAT IT DOES: An object that waits to be hit. It starts from its record; each frame after, it
; steps its animation list (the list at $A365); once it has taken a hit of 5 or more it records
; the event, takes its next kind and a timer of $10, goes back to state 0, and starts an object of
; kind $9B at its place in the first free slot from $50 to $53.
; Untraced: the arm where no slot from $50 to $53 is free (FindFreeSlotFrom50 ran in no script that far).
; HOW: LoadObjectRecord and a frame of 1 on state 0; else StepObjectFrames with the list's pointer
; put in zTemp10-11, then the hit byte compared with 5. The spawn is a tail jump into
; StartObjectAtPlace. The list is read from the disc's table at $A365 by the step (bank 70).
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper (started: A 1, C from
; the record; stepped: A the hit byte, C clear and N, Z from hit less 5; no slot: as
; FindFreeSlotFrom50; spawned: as StartObjectAtPlace with the slot found in Y).
StepReleaseOnHit:
    lda a:OBJ_STATE,x
    bne StepReleaseOnHit_at0
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_FRAME,x
    rts
StepReleaseOnHit_at0:
    lda #$A3
    sta z:zTemp11
    lda #$65
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc StepReleaseOnHit_at1
    jsr VecRecordObjectEvent
    lda a:OBJ_3040,x
    sta a:OBJ_KIND,x
    lda #$10
    sta a:OBJ_3A90,x
    stz a:OBJ_STATE,x
    jsr VecFindFreeSlotFrom50
    bcc StepReleaseOnHit_at1
    lda #$9B
    jmp VecStartObjectAtPlace
StepReleaseOnHit_at1:
    rts

; @routine 0519:36D StepFallingItem
; @run 70:A36D
; WHAT IT DOES: A small thing that falls and then goes away. It starts from its record with its
; speed set from a two-byte row of the table at $A3B1 picked by its kind less $9B; each frame after,
; it moves down by its Y speed, clears its flash flag and steps the animation list the second table
; ($A3B9, by the same index) names; when the list ends the object is cleared.
; HOW: ApplyVelocityY, then StepObjectFrames with zTemp10-11 loaded from the table (bank 70); the
; clear is a tail jump into ClearObject, which leaves every register as it was.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper (started: A the speed's low
; byte, Y the table index, C bit 7 of kind less $9B; else as StepObjectFrames).
StepFallingItem:
    lda a:OBJ_STATE,x
    bne StepFallingItem_at0
    jsr VecLoadObjectRecord
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_KIND,x
    sec
    sbc #$9B
    asl a
    tay
    lda a:$A3B2,y
    sta a:OBJ_VY_HI,x
    lda a:$A3B1,y
    sta a:OBJ_VY_LO,x
    rts
StepFallingItem_at0:
    jsr VecApplyVelocityY
    stz a:OBJ_2B18,x
    lda a:OBJ_KIND,x
    sec
    sbc #$9B
    asl a
    tay
    lda a:$A3B9,y
    sta z:zTemp10
    lda a:$A3BA,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepFallingItem_at1
    jmp VecClearObject
StepFallingItem_at1:
    rts

; @routine 0519:449 HurtCheckThenJumpByState
; @run 70:A449
; WHAT IT DOES: Sends a hit object into its hurt state (EnterHurtStateOnHit), then goes to the code
; the table at $A45E names for the object's state.
; HOW: The hurt check, then the table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the table's high byte, N and Z from it, Y the
; doubled state, C the state's bit 7, X kept, V as on entry.
HurtCheckThenJumpByState:
    jsr EnterHurtStateOnHit
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$A45E,y
    sta z:zTemp00
    lda a:$A45F,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0519:46A EnterHurtStateOnHit
; @run 70:A46A
; WHAT IT DOES: Sends an object that is in one of its first four states and has taken a hit
; of 1 to 4 into state 5, the state its handler table uses for being hurt.
; HOW: Reads the state and the hit byte, each once, and writes the state only when both are
; from 1 to 4.
; WAITS: no.
; Entry: X object. Exit: A and flags as the last compare or load left them; X and Y kept.
EnterHurtStateOnHit:
    lda a:OBJ_STATE,x
    beq EnterHurtStateOnHit_out
    cmp #$05
    bcs EnterHurtStateOnHit_out
    lda a:OBJ_HIT_TAKEN,x
    beq EnterHurtStateOnHit_out
    cmp #$05
    bcs EnterHurtStateOnHit_out
    lda #$05
    sta a:OBJ_STATE,x
EnterHurtStateOnHit_out:
    rts

; @routine 0519:482 InitObjectFromRecord
; @run 70:A482
; WHAT IT DOES: Starts a newly placed object: loads its record, copies slot 0's byte of the
; OBJ_SPRITE_FLIP column into the object's, sets two of its counters and its animation delay to 4, and
; its timer to $60.
; HOW: Calls LoadObjectRecord through its vector (which keeps X), then stores each byte.
; WAITS: no.
; Entry: X object. Exit: A $60, N and Z from it, C as LoadObjectRecord left it; Y 11; X kept. D clear.
InitObjectFromRecord:
    jsr VecLoadObjectRecord
    lda a:OBJ_SPRITE_FLIP
    sta a:OBJ_SPRITE_FLIP,x
    lda #$04
    sta a:OBJ_HITBOX_HALF_W,x
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$04
    sta a:OBJ_3510,x
    lda #$60
    sta a:OBJ_34B8,x
    rts

; @routine 0519:49E ClearOnLanding
; @run 70:A49E
; WHAT IT DOES: One frame of an object that falls away: while it is not rising (its Y speed's top bit
; clear) it asks the floor with a probe height of 2 and, if the probe says floor, the object is cleared.
; Otherwise it takes its frame from OBJ_3510 and moves on one state.
; HOW: ProbeTerrain with the carry and column the caller left; the clear is a tail jump into
; ClearObject.
; Untraced: the arm that clears (the jump at A4B0) ran in no script.
; WAITS: no.
; Entry: X object, Y the probe's column, C passed to the lookup. Exit: cleared: as the probe left it,
; A 1; else A the frame, N and Z from the new state, C and V as the probe or entry left them.
ClearOnLanding:
    lda a:OBJ_VY_HI,x
    bmi ClearOnLanding_at0
    lda #$02
    sta z:SpriteFlipMask
    lda #$01
    jsr VecProbeTerrain
    and #$01
    beq ClearOnLanding_at0
    jmp VecClearObject
ClearOnLanding_at0:
    lda a:OBJ_3510,x
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    rts

; @routine 0519:4BD FallAndBounce
; @run 70:A4BD
; WHAT IT DOES: Moves an object by its velocity and asks the ground under it; on floor the
; object enters its next state. Unless the ground is deadly, the object's X speed is set
; from its angle (the full cosine) and its Y speed to $80, and its angle is turned on by
; $0280.
; HOW: Calls ApplyVelocity and ProbeTerrain (floor, height 0), then ScaleByCosine with a
; multiplier of 256; the angle is the two bytes OBJ_35C0 (low) and OBJ_3568 (high).
; WAITS: no.
; Entry: X object. Exit: A and flags from the last AND or add; X kept; Y as ScaleByCosine
; left it.
FallAndBounce:
    jsr VecApplyVelocity
    lda #$00
    sta z:zProduct1
    lda #$01
    jsr VecProbeTerrain
    and #$01
    beq FallAndBounce_at0
    inc a:OBJ_STATE,x
FallAndBounce_at0:
    lda z:zProduct0
    and #$10
    bne FallAndBounce_out
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_3568,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:zProduct1
    sta a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda a:OBJ_35C0,x
    clc
    adc #$80
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    adc #$02
    sta a:OBJ_3568,x
FallAndBounce_out:
    rts

; @routine 0519:508 FallThenWait
; @run 70:A508
; WHAT IT DOES: A falling object's wait: it falls as FallUntilFloor does, and counts its life timer
; down; at zero the timer becomes $40 and the object moves on one state.
; HOW: FallUntilFloor, then DEC of the timer.
; WAITS: no.
; Entry: X object, Y the probe's column, C passed on. Exit: see the wrapper (the fall's registers,
; then the timer's N and Z; at zero A $40 and N, Z from the new state).
FallThenWait:
    jsr FallUntilFloor
    dec a:OBJ_34B8,x
    bne FallThenWait_at0
    lda #$40
    sta a:OBJ_34B8,x
    inc a:OBJ_STATE,x
FallThenWait_at0:
    rts

; @routine 0519:519 FallUntilFloor
; @run 70:A519
; WHAT IT DOES: One frame of an object that drops until it finds floor. Only while the object is on
; the first screen down (its Y high byte 0) it asks the floor with a probe height of 0; deadly ground
; (bit 4) ends the frame by moving it down with ApplyVelocityY; when the probe does not report floor
; (bit 0) the object is moved on both axes and bounces off by its angle (the bounce the second half of
; FallAndBounce makes).
; HOW: ProbeTerrain with the carry and column the caller left. The deadly arm is a tail jump into
; ApplyVelocityY; the not-floor arm calls ApplyVelocity and branches into FallAndBounce at $A4D6.
; Untraced: the deadly arm and the not-floor arm (A52B, A534) ran in no script.
; WAITS: no.
; Entry: X object, Y the probe's column, C passed to the lookup. Exit: see the wrapper.
FallUntilFloor:
    lda a:OBJ_Y_HI,x
    bne FallUntilFloor_at0
    lda #$00
    sta z:SpriteFlipMask
    lda #$01
    jsr VecProbeTerrain
    and #$10
    beq FallUntilFloor_at1
    jmp VecApplyVelocityY
FallUntilFloor_at1:
    lda z:zProduct0
    and #$01
    bne FallUntilFloor_at0
    jsr VecApplyVelocity
    bra $A4D6
FallUntilFloor_at0:
    rts

; @routine 0519:53A FallThenClear
; @run 70:A53A
; WHAT IT DOES: A falling object that goes away: it falls as FallUntilFloor does and counts its life
; timer down; at zero the object is cleared. Until then its frame is 0, and it shows the frame in
; OBJ_3510 on the beats where bit 1 of the timer is set (a flicker).
; HOW: FallUntilFloor, DEC of the timer; the clear is a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y the probe's column, C passed on. Exit: see the wrapper.
FallThenClear:
    jsr FallUntilFloor
    dec a:OBJ_34B8,x
    bne FallThenClear_at0
    jmp VecClearObject
FallThenClear_at0:
    stz a:OBJ_FRAME,x
    lda a:OBJ_34B8,x
    and #$02
    beq FallThenClear_at1
    lda a:OBJ_3510,x
    sta a:OBJ_FRAME,x
FallThenClear_at1:
    rts

; @routine 0519:556 JumpByKindTable
; @run 70:A556
; WHAT IT DOES: Goes to the code a table at $A56B names for the object's kind less $83 (the handler
; of that kind's state).
; HOW: Reads the table's two bytes into zTemp00-01 and jumps through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the table's high byte, Y the index, N and Z from A,
; C and V from the kind's subtract and shift.
JumpByKindTable:
    lda a:OBJ_KIND,x
    sec
    sbc #$83
    asl a
    tay
    lda a:$A56B,y
    sta z:zTemp00
    lda a:$A56C,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0519:58D CollectHearts
; @run 70:A58D
; WHAT IT DOES: A heart item taken: plays the sound a table at $A5B0 names for its kind less $83, adds the hearts a table at $A5B2 gives to the player's hearts (zField94, decimal, at
; most 99), blends the stage rows (the blend at $9181) and clears the item.
; HOW: SoundCommand, a decimal add, the blend at $9181 (BlendStageRowsOfPlayer or 024F's group's BlendStageScoreAddRows, whichever
; is loaded), a tail jump into ClearObject.
; Untraced: the carry out of the decimal add (the cap of 99, A5A5).
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as BlendStageRowsOfPlayer.
CollectHearts:
    lda a:OBJ_KIND,x
    sec
    sbc #$83
    tay
    lda a:$A5B0,y
    jsr VecSoundCommand
    sed
    lda z:zField94
    clc
    adc a:$A5B2,y
    sta z:zField94
    bcc CollectHearts_at0
    lda #$99
    sta z:zField94
CollectHearts_at0:
    cld
    jsr BlendStageRowsOfPlayer
    jmp VecClearObject

; @routine 0519:5B4 CollectScoreItem
; @run 70:A5B4
; WHAT IT DOES: A score item taken: plays sound $0B, adds the points a table at $A5EB gives (decimal, the two bytes zScoreAddLo and zScoreAddHi, at most 9999) for its kind less $85,
; blends the stage rows, and the item turns into a kind $9F that waits the number of beats its first byte said ($34B8 timer) in state 0.
; HOW: SoundCommand, a decimal add through the two bytes, the blend at $9181 (BlendStageRowsOfPlayer or 024F's group's
; BlendStageScoreAddRows, whichever is loaded) with Y kept on the stack.
; Untraced: the carry out of the add (the cap of 9999, A5D4).
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: A $9F, N set, Z clear; C and V as the blend left them.
CollectScoreItem:
    lda #$0B
    jsr VecSoundCommand
    lda a:OBJ_KIND,x
    sec
    sbc #$85
    tay
    sed
    lda z:zScoreAddLo
    sta z:zTemp10
    clc
    adc a:$A5EB,y
    sta z:zScoreAddLo
    lda z:zScoreAddHi
    adc #$00
    sta z:zScoreAddHi
    cld
    bcc CollectScoreItem_at0
    lda #$99
    sta z:zScoreAddLo
    sta z:zScoreAddHi
CollectScoreItem_at0:
    phy
    jsr BlendStageRowsOfPlayer
    pla
    sta a:OBJ_34B8,x
    lda #$9F
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts

; @routine 0519:5EF ChangeKindFromTable
; @run 70:A5EF
; WHAT IT DOES: An object changes into another kind. Sound $0B plays, the player's byte at $3BD6
; becomes 1, and zField93 takes a byte of the table at $A650 picked by the kind less $80 (its old
; value is the index into the table at $A668 that names the new kind). A zero there clears the object.
; Otherwise the object takes the new kind and state 0 with a timer of $60, is put at the player's place,
; moved 8 up with a borrow, and thrown up at a speed of $FEC0 and sideways by a table pair picked by the
; player's facing.
; HOW: The old zField93 is kept on the stack across the table read; the place copy is made with X
; and Y swapped (source slot 0, destination the object); the clear is a tail jump into ClearObject.
; Tables are read from the disc (bank 70) by their addresses until the data layer names them; $3BD6
; has no name yet (a row is requested).
; WAITS: no.
; Entry: X object. Exit: see the wrapper (cleared: A 0, Y the old zField93, C and V from the kind
; less $80; changed: A the sideways speed's low byte, Y the facing's table index, C the facing's top
; bit, V from the borrow into the Y position's high byte).
ChangeKindFromTable:
    lda #$0B
    jsr VecSoundCommand
    lda z:zField93
    pha
    lda #$01
    sta a:PlayerByte3BD6
    lda a:OBJ_KIND,x
    sec
    sbc #$80
    tay
    lda a:$A650,y
    sta z:zField93
    ply
    lda a:$A668,y
    bne ChangeKindFromTable_at0
    jmp VecClearObject
ChangeKindFromTable_at0:
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    lda #$60
    sta a:OBJ_3A90,x
    ldy #$00
    sxy
    jsr VecCopyObjectPlace
    sxy
    lda a:OBJ_Y_LO,x
    sec
    sbc #$08
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_Y_HI,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:$A672,y
    sta a:OBJ_VX_HI,x
    lda a:$A671,y
    sta a:OBJ_VX_LO,x
    rts

; @routine 0519:703 RunObjectState
; @run 70:A703
; WHAT IT DOES: One frame of a common object: animates kind $88, takes a pending hit, blinks, then
; runs the handler of the object's state from the table at $A71E.
; HOW: AnimateKind88, EnterHurtStateOnHit, SetBlinkFlash, then the state doubled indexes the table
; (read into zTemp00-01 and jumped through).
; WAITS: no.
; Entry: X object, Y kept unless AnimateKind88 sets it. Exit: as the handler leaves it; at the jump A
; is the table's high byte, Y the state doubled, N and Z from A, C and V as SetBlinkFlash and the
; earlier calls left them.
RunObjectState:
    jsr AnimateKind88
    jsr EnterHurtStateOnHit
    jsr SetBlinkFlash
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$A71E,y
    sta z:zTemp00
    lda a:$A71F,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0519:72C AnimateKind88
; @run 70:A72C
; WHAT IT DOES: For an object of kind $88 only: steps its animation list (at $A745) and keeps the
; frame it shows in OBJ_3510. Any other kind is left alone.
; HOW: Compares the kind with $88; then StepObjectFrames with the list's pointer in zTemp10-11 and a
; copy of the frame byte.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: other kind: A the kind, flags of its compare
; with $88; kind $88: A the frame, N and Z from it, C as the step left it (changed), Y as it left it.
AnimateKind88:
    lda a:OBJ_KIND,x
    cmp #$88
    bne AnimateKind88_at0
    lda #$A7
    sta z:zTemp11
    lda #$45
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    sta a:OBJ_3510,x
AnimateKind88_at0:
    rts

; @routine 0519:74E SetBlinkFlash
; @run 70:A74E
; WHAT IT DOES: Sets an object's flash flag (the palette flash its sprite shows) to $86 on
; alternate beats of the wave counter once its timer has reached $18, and to 0 otherwise.
; HOW: Clears the flag first, then reads the timer and, if it is $18 or more and bit 1 of the
; wave counter is clear, writes $86.
; WAITS: no.
; Entry: X object. Exit: A and flags as the last compare, AND or load left them; X and Y kept.
SetBlinkFlash:
    stz a:OBJ_2B18,x
    lda a:OBJ_3A90,x
    cmp #$18
    bcc SetBlinkFlash_out
    lda z:zWaveCounter
    and #$02
    bne SetBlinkFlash_out
    lda #$86
    sta a:OBJ_2B18,x
SetBlinkFlash_out:
    rts

; @routine 0519:764 StartKindFromCharacter
; @run 70:A764
; WHAT IT DOES: Starts an object whose kind is a pair: a kind $93 with zField90 set becomes kind $84,
; and an object whose kind is already the one the table at $A668 gives for zField93 becomes $84; then
; the record is loaded, slot 0's byte of the OBJ_SPRITE_FLIP column is copied into the object's, and OBJ_3510
; is set from the table at $A79E, row of two bytes by the kind less $84, the second by which character
; plays.
; HOW: Compares and, on equality, stores kind $84; LoadObjectRecord; the row by an add with the
; character.
; Untraced: the two arms that store $84 (the bytes at A76B and A77E) ran in no script.
; WAITS: no.
; Entry: X object. Exit: A the table byte, N and Z from it, C and V from the add of the character,
; Y the table index.
StartKindFromCharacter:
    lda a:OBJ_KIND,x
    cmp #$93
    bne StartKindFromCharacter_at0
    lda z:zField90
    beq StartKindFromCharacter_at0
    lda #$84
    sta a:OBJ_KIND,x
StartKindFromCharacter_at0:
    ldy z:zField93
    lda a:$A668,y
    cmp a:OBJ_KIND,x
    bne StartKindFromCharacter_at1
    lda #$84
    sta a:OBJ_KIND,x
StartKindFromCharacter_at1:
    jsr VecLoadObjectRecord
    lda a:OBJ_SPRITE_FLIP
    sta a:OBJ_SPRITE_FLIP,x
    lda a:OBJ_KIND,x
    sec
    sbc #$84
    asl a
    clc
    adc z:zCharacter
    tay
    lda a:$A79E,y
    sta a:OBJ_3510,x
    rts

; @routine 0519:7BE SettleOnLanding
; @run 70:A7BE
; WHAT IT DOES: One frame of a thrown object that stops where it lands: StopOnLanding with a probe height of 0; on landing its life timer is set
; to $F0 and it enters its next state.
; HOW: StopOnLanding, then a store and an INC.
; WAITS: no.
; Entry: X object, Y the probe's column. Exit: the mover's registers, then A $F0 and N, Z from the new state when landed.
SettleOnLanding:
    lda #$00
    jsr VecStopOnLanding
    bcc SettleOnLanding_at0
    lda #$F0
    sta a:OBJ_34B8,x
    inc a:OBJ_STATE,x
SettleOnLanding_at0:
    rts

; @routine 0519:7CE LandThenWait
; @run 70:A7CE
; WHAT IT DOES: A thrown object on the first screen row is asked to land (StopOnLanding, height 0); its life timer counts down and at zero
; becomes $50 while the object enters its next state.
; HOW: StopOnLanding, DEC, INC.
; WAITS: no.
; Entry: X object, Y the probe's column. Exit: the mover's registers when asked, then the timer's N and Z; at zero A $50 and N, Z from the state.
LandThenWait:
    lda a:OBJ_Y_HI,x
    bne LandThenWait_at0
    lda #$00
    jsr VecStopOnLanding
LandThenWait_at0:
    dec a:OBJ_34B8,x
    bne LandThenWait_at1
    lda #$50
    sta a:OBJ_34B8,x
    inc a:OBJ_STATE,x
LandThenWait_at1:
    rts

; @routine 0519:7E6 LandThenClear
; @run 70:A7E6
; WHAT IT DOES: As LandThenWait, but at zero the object is cleared; until then its frame is 0, and it shows OBJ_3510 on the beats where bit 1
; of the timer is set.
; HOW: StopOnLanding, DEC; the clear is a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y the probe's column. Exit: the mover's registers when asked, then the last compare, AND or load.
LandThenClear:
    lda a:OBJ_Y_HI,x
    bne LandThenClear_at0
    lda #$00
    jsr VecStopOnLanding
LandThenClear_at0:
    dec a:OBJ_34B8,x
    bne LandThenClear_at1
    jmp VecClearObject
LandThenClear_at1:
    stz a:OBJ_FRAME,x
    lda a:OBJ_34B8,x
    and #$02
    beq LandThenClear_at2
    lda a:OBJ_3510,x
    sta a:OBJ_FRAME,x
LandThenClear_at2:
    rts

; @routine 051A:055 BitMaskFromIndex
; @run 70:A855
; WHAT IT DOES: Gives the byte with only bit Y set (1 shifted left Y times; 0 if Y is 8 or more).
; HOW: Shifts a 1 left, counting Y down to 0.
; WAITS: no.
; Entry: Y the bit number. Exit: A the mask; Y 0; Z and C set, N clear (the last compare).
BitMaskFromIndex:
    lda #$01
BitMaskFromIndex_at0:
    cpy #$00
    beq BitMaskFromIndex_out
    asl a
    dey
    bra BitMaskFromIndex_at0
BitMaskFromIndex_out:
    rts
; @routine 051A:12E StartThrownItem
; @run 70:A92E
; @stop A95E
; WHAT IT DOES: A thrown item: from state 0 it loads its record, takes its frame from a table by its
; life timer and its speeds from a four-byte row of the table at $A95E picked by OBJ_3510; in any other
; state it goes on as DropOffScreen.
; HOW: Tables read from the disc (bank 70) by address; the other states are a tail branch into
; DropOffScreen.
; WAITS: no.
; Entry: X object. Exit: started: A the last speed byte, Y the row's start, C bit 6 of OBJ_3510, N and
; Z from A; else as DropOffScreen.
StartThrownItem:
    lda a:OBJ_STATE,x
    bne DropOffScreen
    jsr VecLoadObjectRecord
    ldy a:OBJ_34B8,x
    lda a:$A97E,y
    sta a:OBJ_FRAME,x
    lda a:OBJ_3510,x
    asl a
    asl a
    tay
    lda a:$A95F,y
    sta a:OBJ_VX_HI,x
    lda a:$A95E,y
    sta a:OBJ_VX_LO,x
    lda a:$A961,y
    sta a:OBJ_VY_HI,x
    lda a:$A960,y
    sta a:OBJ_VY_LO,x
    rts

; @routine 051A:180 DropOffScreen
; @run 70:A980
; WHAT IT DOES: A thrown object's frame: when it is below the first screen (Y high byte above 0 and
; not negative) it is cleared; otherwise gravity takes its speed up and it moves by its velocity.
; HOW: The clear and the move are tail jumps (ClearObject, ApplyVelocity); gravity is
; ApplyGravityUnlimited.
; WAITS: no.
; Entry: X object. Exit: cleared: A the Y high byte, N and Z from it; else as ApplyVelocity.
DropOffScreen:
    lda a:OBJ_Y_HI,x
    beq DropOffScreen_at0
    bmi DropOffScreen_at0
    jmp VecClearObject
DropOffScreen_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 051A:1A8 StartScorePickup
; @run 70:A9A8
; WHAT IT DOES: Loads the object's record, clears its facing and adds the score a table at $A9E1 gives for its timer (two bytes of BCD); when that carries over
; the mark the score tests for it the player gets an extra life (GiveExtraLife). The frame comes from a table at $A9DC by the timer, the angle OBJ_3510 is $18 and the Y speed $FE00.
; HOW: LoadObjectRecord, AddScoreWord, GiveExtraLife, table reads (bank 70).
; Untraced: the extra life (A9C2).
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: see the wrapper.
StartScorePickup:
    jsr VecLoadObjectRecord
    stz a:OBJ_FACING_LEFT,x
    lda a:OBJ_34B8,x
    asl a
    tay
    lda a:$A9E1,y
    sta z:zTemp00
    lda a:$A9E2,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc StartScorePickup_at0
    jsr GiveExtraLife
StartScorePickup_at0:
    ldy a:OBJ_34B8,x
    lda a:$A9DC,y
    sta a:OBJ_FRAME,x
    lda #$18
    sta a:OBJ_3510,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    rts

; @routine 051A:1EB FallThenState
; @run 70:A9EB
; WHAT IT DOES: Moves an object down by its Y speed; OBJ_3510 counts down and at zero becomes $10
; while the object moves on one state.
; HOW: ApplyVelocityY, then DEC of OBJ_3510.
; WAITS: no.
; Entry: X object. Exit: the move's registers, then N and Z of the count; at zero A $10 and N, Z of
; the new state.
FallThenState:
    jsr VecApplyVelocityY
    dec a:OBJ_3510,x
    bne FallThenState_at0
    lda #$10
    sta a:OBJ_3510,x
    inc a:OBJ_STATE,x
FallThenState_at0:
    rts

; @routine 051A:1FC ClearWhenCountEnds
; @run 70:A9FC
; WHAT IT DOES: OBJ_3510 counts down; at zero the object is cleared.
; HOW: DEC; the clear is a tail jump into ClearObject.
; WAITS: no.
; Entry: X object. Exit: N and Z of the count; A, Y and the rest kept.
ClearWhenCountEnds:
    dec a:OBJ_3510,x
    bne ClearWhenCountEnds_at0
    jmp VecClearObject
ClearWhenCountEnds_at0:
    rts
; @routine 07D9:313 DispatchByKind07D9
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $AA72 holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 07D9's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind07D9:
    lda a:OBJ_KIND,x
    beq DispatchByKind07D9_at0
    dec a
    asl a
    tay
    lda a:$AA72,y
    sta z:zTemp00
    lda a:$AA73,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind07D9_at0:
    rts

; @routine 0889:313 DispatchByKind0889
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $AA35 holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0889's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind0889:
    lda a:OBJ_KIND,x
    beq DispatchByKind0889_at0
    dec a
    asl a
    tay
    lda a:$AA35,y
    sta z:zTemp00
    lda a:$AA36,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind0889_at0:
    rts

; @routine 0A99:313 DispatchByKind0A99
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $AB77 holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0A99's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind0A99:
    lda a:OBJ_KIND,x
    beq DispatchByKind0A99_at0
    dec a
    asl a
    tay
    lda a:$AB77,y
    sta z:zTemp00
    lda a:$AB78,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind0A99_at0:
    rts

; @routine 0CA9:313 DispatchByKind0CA9
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $AB9D holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0CA9's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind0CA9:
    lda a:OBJ_KIND,x
    beq DispatchByKind0CA9_at0
    dec a
    asl a
    tay
    lda a:$AB9D,y
    sta z:zTemp00
    lda a:$AB9E,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind0CA9_at0:
    rts

; @routine 0E09:313 DispatchByKind0E09
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $AAAA holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 0E09's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind0E09:
    lda a:OBJ_KIND,x
    beq DispatchByKind0E09_at0
    dec a
    asl a
    tay
    lda a:$AAAA,y
    sta z:zTemp00
    lda a:$AAAB,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind0E09_at0:
    rts

; @routine 10C9:313 DispatchByKind10C9
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $AB4A holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 10C9's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind10C9:
    lda a:OBJ_KIND,x
    beq DispatchByKind10C9_at0
    dec a
    asl a
    tay
    lda a:$AB4A,y
    sta z:zTemp00
    lda a:$AB4B,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind10C9_at0:
    rts

; @routine 12D9:313 DispatchByKind12D9
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $ABAF holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 12D9's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind12D9:
    lda a:OBJ_KIND,x
    beq DispatchByKind12D9_at0
    dec a
    asl a
    tay
    lda a:$ABAF,y
    sta z:zTemp00
    lda a:$ABB0,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind12D9_at0:
    rts

; @routine 16C9:313 DispatchByKind16C9
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $AB0D holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 16C9's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind16C9:
    lda a:OBJ_KIND,x
    beq DispatchByKind16C9_at0
    dec a
    asl a
    tay
    lda a:$AB0D,y
    sta z:zTemp00
    lda a:$AB0E,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind16C9_at0:
    rts

; @routine 18D9:313 DispatchByKind18D9
; @run 70:A313
; WHAT IT DOES: Sends an object to the code its kind names: the table at $AAD5 holds one address per
; kind, kind 1 first. Kind 0 does nothing (that arm, the byte at A328, ran in no script). This is the overlay at sector 18D9's copy.
; HOW: dispatch_by_kind with this overlay's table (read from the disc, bank 70); the jump goes through
; zTemp00-01.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; at the jump A the table's high byte, Y the table index,
; N and Z from A, C bit 7 of kind less 1. Kind 0: A 0, Z set.
DispatchByKind18D9:
    lda a:OBJ_KIND,x
    beq DispatchByKind18D9_at0
    dec a
    asl a
    tay
    lda a:$AAD5,y
    sta z:zTemp00
    lda a:$AAD6,y
    sta z:zTemp01
    jmp (ZeroPage)
DispatchByKind18D9_at0:
    rts
