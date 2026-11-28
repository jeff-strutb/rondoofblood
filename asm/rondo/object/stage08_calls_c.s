; @routine 0D2D:335 FrameAndTimerByArgSound0D2D
; @run 81:C335
; WHAT IT DOES: Sets the object's frame and OBJ_WORK_E from two tables by OBJ_START_ARG, plays sound $3A and steps
; the substate on (Stage 08's overlay).
; HOW: Two table reads by OBJ_START_ARG, the sound call, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FrameAndTimerByArgSound0D2DFrames = $C194 ; the frame for each OBJ_START_ARG
FrameAndTimerByArgSound0D2DWorkE = $C1A8 ; OBJ_WORK_E for each OBJ_START_ARG
FrameAndTimerByArgSound0D2D:
    ldy a:OBJ_START_ARG,x
    lda a:FrameAndTimerByArgSound0D2DFrames,y
    sta a:OBJ_FRAME,x
    lda a:FrameAndTimerByArgSound0D2DWorkE,y
    sta a:OBJ_WORK_E,x
    lda #$3A
    jsr VecSoundCommand
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0D2D:34D RiseThenBecomeKind0D2D
; @run 81:C34D
; WHAT IT DOES: Moves the object up a pixel and counts OBJ_WORK_E down; at zero records its event and turns it into
; the kind a table gives by OBJ_START_ARG, in state 0 with no hit box offset and no Y speed (Stage 08's overlay).
; HOW: DEC of the Y low byte and of the count, then RecordObjectEvent, a table read and stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RiseThenBecomeKind0D2DKinds = $C1D0 ; the new kind for each OBJ_START_ARG
RiseThenBecomeKind0D2D:
    dec a:OBJ_Y_LO,x
    dec a:OBJ_WORK_E,x
    bne RiseThenBecomeKind0D2D_at0
    jsr VecRecordObjectEvent
    ldy a:OBJ_START_ARG,x
    lda a:RiseThenBecomeKind0D2DKinds,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_HITBOX_DX,x
    stz a:OBJ_HITBOX_DY,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
RiseThenBecomeKind0D2D_at0:
    rts

; @routine 0D2D:3E7 StartSevenLinks0D2D
; @run 81:C3E7
; WHAT IT DOES: Starts the object with hit box $82, OBJ_WORK_D $40 and OBJ_WORK_G 7, and seven objects of kind $C4
; at its place in the seven slots below it, each with its OBJ_START_ARG and its index 0 to 6 in OBJ_WORK_G;
; OBJ_WORK_H comes from a table (Stage 08's overlay).
; HOW: LoadObjectRecord, stores, a loop of ClearObjectInY and StartObjectAtPlace, then a table read by eight times
; OBJ_START_ARG plus OBJ_WORK_G.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StartSevenLinks0D2DCounts = $C430 ; OBJ_WORK_H, eight per OBJ_START_ARG, one per link index
StartSevenLinks0D2D:
    jsr VecLoadObjectRecord
    lda #$82
    sta a:OBJ_HITBOX_ON,x
    lda #$40
    sta a:OBJ_WORK_D,x
    lda #$07
    sta a:OBJ_WORK_G,x
    txa
    sec
    sbc #$07
    tay
    lda #$07
    sta z:zTemp00
    stz z:zTemp01
StartSevenLinks0D2D_at0:
    jsr VecClearObjectInY
    lda #$C4
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    sta a:OBJ_START_ARG,y
    lda z:zTemp01
    sta a:OBJ_WORK_G,y
    iny
    inc z:zTemp01
    dec z:zTemp00
    bne StartSevenLinks0D2D_at0
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    clc
    adc a:OBJ_WORK_G,x
    tay
    lda a:StartSevenLinks0D2DCounts,y
    sta a:OBJ_WORK_H,x
    rts

; @routine 0D2D:448 FollowLinkAtAngle0D2D
; @run 81:C448
; WHAT IT DOES: With OBJ_WORK_G not 0, puts the object at the place of the slot below it and moves it at angle
; OBJ_WORK_D by a speed a table gives by OBJ_WORK_G; counts OBJ_WORK_H down and steps the state on at zero (Stage
; 08's overlay).
; HOW: CopyObjectPlaceBack from X less 1, a table read, SetVelocityFromAngle and ApplyVelocity, then DEC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FollowLinkAtAngle0D2DSpeeds = $C46F ; the speed's high byte for each link index
FollowLinkAtAngle0D2D:
    lda a:OBJ_WORK_G,x
    beq FollowLinkAtAngle0D2D_at0
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    stz z:zTemp00
    ldy a:OBJ_WORK_G,x
    lda a:FollowLinkAtAngle0D2DSpeeds,y
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
FollowLinkAtAngle0D2D_at0:
    dec a:OBJ_WORK_H,x
    bne FollowLinkAtAngle0D2D_at1
    inc a:OBJ_STATE,x
FollowLinkAtAngle0D2D_at1:
    rts

; @routine 0D2D:477 SwingLinkByCosine0D2D
; @run 81:C477
; WHAT IT DOES: Clears the object unless the slot below holds kind $C4; else puts it at that slot's place, moves it
; at angle OBJ_WORK_D with speed $1000, and swings the 16-bit angle OBJ_WORK_D-E by a cosine of OBJ_WORK_F, which
; steps by 2; when that wraps to 0 the angle goes back to $40: that part is MoveAndSwingByCosine0D2D, which it runs
; into (Stage 08's overlay).
; HOW: A kind compare, CopyObjectPlaceBack, SetVelocityFromAngle, then MoveAndSwingByCosine0D2D's ApplyVelocity,
; ScaleByCosine of a word from a table by OBJ_START_ARG and a 16-bit add.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SwingLinkByCosine0D2D:
    lda a:OBJ_KIND-1,x
    cmp #$C4
    beq SwingLinkByCosine0D2D_at0
    jmp VecClearObject
SwingLinkByCosine0D2D_at0:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    stz z:zTemp00
    lda #$10
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle

; @routine 0D2D:4EE StartLinkByIndex0D2D
; @run 81:C4EE
; WHAT IT DOES: Starts the object with OBJ_WORK_D $40, moves it once by a Y speed a table gives by OBJ_WORK_G, and
; sets OBJ_WORK_H from the table of StartSevenLinks0D2D (Stage 08's overlay).
; HOW: LoadObjectRecord, a table read, ApplyVelocityY, a table read by eight times OBJ_START_ARG plus OBJ_WORK_G.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StartLinkByIndex0D2DCounts = $C430 ; OBJ_WORK_H, eight per OBJ_START_ARG, one per link index
StartLinkByIndex0D2DSpeeds = $C514 ; the Y speed's high byte for each link index
StartLinkByIndex0D2D:
    jsr VecLoadObjectRecord
    lda #$40
    sta a:OBJ_WORK_D,x
    ldy a:OBJ_WORK_G,x
    lda a:StartLinkByIndex0D2DSpeeds,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    clc
    adc a:OBJ_WORK_G,x
    tay
    lda a:StartLinkByIndex0D2DCounts,y
    sta a:OBJ_WORK_H,x
    rts

; @routine 0D5B:09B StartFromArgBits0D5B
; @run 70:B09B
; WHAT IT DOES: Starts the object, adds $80 to its hit box byte, and from OBJ_START_ARG sets OBJ_WORK_E (its low
; four bits plus $10), OBJ_WORK_D (bits 4 and 5) and OBJ_WORK_F (bits 6 and 7); then makes it kind $BB (Stage 08's
; overlay).
; HOW: LoadObjectRecord, an add, then the argument's bits taken by AND and ROL.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartFromArgBits0D5B:
    jsr VecLoadObjectRecord
    lda #$80
    clc
    adc a:OBJ_HITBOX_ON,x
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_START_ARG,x
    tay
    and #$0F
    ora #$10
    sta a:OBJ_WORK_E,x
    tya
    rol a
    rol a
    rol a
    tay
    and #$03
    sta a:OBJ_WORK_F,x
    tya
    rol a
    rol a
    and #$03
    sta a:OBJ_WORK_D,x
    lda #$BB
    sta a:OBJ_KIND,x
    rts

; @routine 0D5B:0CB SetSectionBySide0D5B
; @run 70:B0CB
; @stop B0E4
; WHAT IT DOES: In state 0 starts the object; then, with OBJ_START_ARG 0, sets zField82 (the stage section) from a
; table by whether the object is left of the player; with OBJ_START_ARG not 0 runs BuildSection2AndClear0D5B (Stage
; 08's overlay).
; HOW: LoadObjectRecord, or SideOfPlayer's bit 0 indexing a two-byte table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetSectionBySide0D5BSections = $B0E4 ; zField82 when right of the player, then when left
SetSectionBySide0D5B:
    lda a:OBJ_STATE,x
    bne SetSectionBySide0D5B_at0
    jmp VecLoadObjectRecord
SetSectionBySide0D5B_at0:
    lda a:OBJ_START_ARG,x
    bne BuildSection2AndClear0D5B
    jsr VecSideOfPlayer
    and #$01
    tay
    lda a:SetSectionBySide0D5BSections,y
    sta z:zField82
    rts

; @routine 0D5B:0E6 BuildSection2AndClear0D5B
; @run 70:B0E6
; WHAT IT DOES: Sets zField82 to 2, builds that section's stage lists, sets zField86 to 2 and clears the object
; (Stage 08's overlay).
; HOW: A store, ExpandStageListsM with X kept, a store and ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BuildSection2AndClear0D5B:
    lda #$02
    sta z:zField82
    phx
    jsr ExpandStageListsM
    plx
    lda #$02
    sta z:zField86
    jmp VecClearObject

; @routine 0D5B:192 ActByDistanceToPlayer0D5B
; @run 70:B192
; WHAT IT DOES: An object hit 1 to 4 times goes to SetPhase2; otherwise runs the routine a table picks by its X
; distance to the player in steps of 16 pixels (Stage 08's overlay).
; HOW: A compare of OBJ_HIT_TAKEN; DistanceXToPlayer over 16 indexes a byte table that picks from a jump table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByDistanceToPlayer0D5BChoices = $B1B8 ; the jump table's entry for each 16 pixels of distance
ActByDistanceToPlayer0D5BJumps = $B1C8 ; the routines' addresses, a word each
ActByDistanceToPlayer0D5B:
    lda a:OBJ_HIT_TAKEN,x
    beq ActByDistanceToPlayer0D5B_at0
    cmp #$05
    bcs ActByDistanceToPlayer0D5B_at0
    jmp SetPhase2
ActByDistanceToPlayer0D5B_at0:
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    lda a:ActByDistanceToPlayer0D5BChoices,y
    asl a
    tay
    lda a:ActByDistanceToPlayer0D5BJumps,y
    sta z:zTemp00
    lda a:ActByDistanceToPlayer0D5BJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5B:269 FallFrameThenLand0D5B
; @run 70:B269
; WHAT IT DOES: Gives the object a frame by whether its Y speed is upward, falls and lands with probe height 4, and
; on landing goes to state 3 (Stage 08's overlay).
; HOW: The speed's sign bit indexes a two-byte table; StopOnLanding's carry picks the landing.
; WAITS: no.
; Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FallFrameThenLand0D5BFrames = $B284 ; the frame moving down, then moving up
FallFrameThenLand0D5B:
    lda a:OBJ_VY_HI,x
    rol a
    rol a
    and #$01
    tay
    lda a:FallFrameThenLand0D5BFrames,y
    sta a:OBJ_FRAME,x
    lda #$04
    jsr VecStopOnLanding
    bcc ReturnOnly0D5B

; @routine 0D5B:27E EnterState3_0D5B
; @run 70:B27E
; WHAT IT DOES: Puts the object in state 3 (Stage 08's overlay).
; HOW: One store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
EnterState3_0D5B:
    lda #$03
    sta a:OBJ_STATE,x

; @routine 0D5B:283 ReturnOnly0D5B
; @run 70:B283
; WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
; HOW: An RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
ReturnOnly0D5B:
    rts

; @routine 0D5B:286 Frame5FallOrLand0D5B
; @run 70:B286
; WHAT IT DOES: Gives the object frame 5 and falls with probe height 4: on landing it goes to state 3; while in the
; air $14 comes off its Y speed each frame (Stage 08's overlay).
; HOW: StopOnLanding, then a 16-bit subtract.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Frame5FallOrLand0D5B:
    lda #$05
    sta a:OBJ_FRAME,x
    lda #$04
    jsr VecStopOnLanding
    bcs EnterState3_0D5B
    lda a:OBJ_VY_LO,x
    sec
    sbc #$14
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    rts

; @routine 0D5B:2A4 SpawnKind21WhileFlag0D5B
; @run 70:B2A4
; WHAT IT DOES: In state 0 starts the object. While event flag bit 2 is set it starts an object of kind $21
; (OBJ_START_ARG 1) at its place in the first free slot from $38 to $3F every $29 frames, shifted by a random X
; step; once flag bit 4 is set too the object is cleared (Stage 08's overlay).
; HOW: Flag tests, a countdown in OBJ_WORK_D, a search loop, StartObjectAtPlace, NextRandom as the X speed for one
; ApplyVelocityX.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SpawnKind21WhileFlag0D5B:
    lda a:OBJ_STATE,x
    bne SpawnKind21WhileFlag0D5B_at0
    jmp VecLoadObjectRecord
SpawnKind21WhileFlag0D5B_at0:
    lda a:EventFlags
    and #$04
    beq SpawnKind21WhileFlag0D5B_at1
    lda a:EventFlags
    and #$10
    beq SpawnKind21WhileFlag0D5B_at2
    jmp VecClearObject
SpawnKind21WhileFlag0D5B_at2:
    lda a:OBJ_WORK_D,x
    beq SpawnKind21WhileFlag0D5B_at3
    dec a:OBJ_WORK_D,x
    rts
SpawnKind21WhileFlag0D5B_at3:
    lda #$28
    sta a:OBJ_WORK_D,x
    ldy #$38
SpawnKind21WhileFlag0D5B_at5:
    lda a:OBJ_KIND,y
    beq SpawnKind21WhileFlag0D5B_at4
    iny
    cpy #$40
    bcc SpawnKind21WhileFlag0D5B_at5
    rts
SpawnKind21WhileFlag0D5B_at4:
    jsr VecClearObjectInY
    lda #$21
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_START_ARG,y
    sxy
    jsr VecNextRandom
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    stz a:OBJ_VX_HI,x
    sxy
SpawnKind21WhileFlag0D5B_at1:
    rts

; @routine 0D5B:372 StartNineParts0D5B
; @run 70:B372
; WHAT IT DOES: Starts the object with OBJ_WORK_O $80, fills a 128-byte buffer with $70, and starts nine objects of
; kind $23 at its place in the nine slots after it, each with its index 0 to 8 in OBJ_WORK_K (Stage 08's overlay).
; HOW: LoadObjectRecord, a fill loop, a loop of ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartNineParts0D5BBuffer = $B3A3 ; 128 bytes of angles, each $70 here
StartNineParts0D5B:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_WORK_O,x
    lda #$70
    cly
StartNineParts0D5B_at0:
    sta a:StartNineParts0D5BBuffer,y
    iny
    cpy #$80
    bcc StartNineParts0D5B_at0
    txa
    tay
    iny
    lda #$09
    sta z:zTemp00
    stz z:zTemp01
StartNineParts0D5B_at1:
    jsr VecClearObjectInY
    lda #$23
    jsr VecStartObjectAtPlace
    lda z:zTemp01
    sta a:OBJ_WORK_K,y
    iny
    inc z:zTemp01
    dec z:zTemp00
    bne StartNineParts0D5B_at1
    rts

; @routine 0D5B:485 ScaleByRecordWords0D5B
; @run 70:B485
; WHAT IT DOES: Takes the four-byte record OBJ_WORK_M picks: scales its second word by the cosine of OBJ_WORK_K and
; puts its first word in zMultiplier (Stage 08's overlay).
; HOW: Four times OBJ_WORK_M indexes the records, kept in zHitSlot across ScaleByCosine.
; WAITS: no.
; Entry: X the object. Exit: zProduct the scaled word; zMultiplier the record's first word; zHitSlot the record's
; index.
ScaleByRecordWords0D5BRecords = $B468 ; two words per record: the multiplier, then the size to scale
ScaleByRecordWords0D5B:
    lda a:OBJ_WORK_M,x
    asl a
    asl a
    tay
    sty z:zBurstCount
    lda a:ScaleByRecordWords0D5BRecords+2,y
    sta z:zTemp00
    lda a:ScaleByRecordWords0D5BRecords+3,y
    sta z:zTemp01
    lda a:OBJ_WORK_K,x
    jsr VecScaleByCosine
    ldy z:zBurstCount
    lda a:ScaleByRecordWords0D5BRecords,y
    sta z:zSetUpColumnsLeft
    lda a:ScaleByRecordWords0D5BRecords+1,y
    sta z:zTransposeRowsLeft
    rts

; @routine 0D5B:4AA NextStateByPattern0D5B
; @run 70:B4AA
; WHAT IT DOES: Counts OBJ_WORK_O down; at zero sets a new random wait and puts the object in the next state of a
; 16-step pattern counted by OBJ_WORK_N, substate 0 (Stage 08's overlay).
; HOW: NextRandom's low two bits index the waits; OBJ_WORK_N's low four bits index a table that indexes the states.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
NextStateByPattern0D5BWaits = $B4D6 ; four waits for OBJ_WORK_O
NextStateByPattern0D5BSteps = $B4DA ; the state table's index for each of 16 steps
NextStateByPattern0D5BStates = $B4EA ; the states
NextStateByPattern0D5B:
    lda a:OBJ_WORK_O,x
    beq NextStateByPattern0D5B_at0
    dec a:OBJ_WORK_O,x
    rts
NextStateByPattern0D5B_at0:
    jsr VecNextRandom
    and #$03
    tay
    lda a:NextStateByPattern0D5BWaits,y
    sta a:OBJ_WORK_O,x
    lda a:OBJ_WORK_N,x
    and #$0F
    tay
    lda a:NextStateByPattern0D5BSteps,y
    tay
    lda a:NextStateByPattern0D5BStates,y
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_N,x
    rts

; @routine 0D5B:4ED MoveChainOfNine0D5B
; @run 70:B4ED
; WHAT IT DOES: Moves a chain of nine objects: from the slot eight after the object down to the one after it, each
; takes the place of the slot above it and moves by its speed; the object itself takes the next slot's place, moves
; 8 pixels left, then moves at angle OBJ_WORK_D plus $70 with the speed in OBJ_WORK_F and OBJ_WORK_G (Stage 08's
; overlay).
; HOW: A loop down the slots of CopyObjectPlaceBack and ApplyVelocity, with the object's slot kept in
; zRichterObject; then ApplyVelocityX, SetVelocityFromAngle and ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveChainOfNine0D5B:
    stx z:zClimbStepBefore
    txa
    clc
    adc #$08
    tax
MoveChainOfNine0D5B_at0:
    txa
    tay
    iny
    jsr VecCopyObjectPlaceBack
    jsr VecApplyVelocity
    dex
    cpx z:zClimbStepBefore
    bne MoveChainOfNine0D5B_at0
    txa
    tay
    iny
    jsr VecCopyObjectPlaceBack
    lda #$F8
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    clc
    adc #$70
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 0D5B:586 StartKindA4Child0D5B
; @run 70:B586
; WHAT IT DOES: Starts an object of kind $A4 at the object's place in a free slot from $30, with the object's slot
; in its OBJ_WORK_D, and puts the object in state 1 (Stage 08's overlay).
; HOW: FindFreeSlotFrom30, StartObjectAtPlace, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StartKindA4Child0D5B:
    jsr VecFindFreeSlotFrom30
    bcc StartKindA4Child0D5B_at0
    lda #$A4
    jsr VecStartObjectAtPlace
    txa
    sta a:OBJ_WORK_D,y
StartKindA4Child0D5B_at0:
    lda #$01
    sta a:OBJ_STATE,x
    rts

; @routine 0D5B:651 TurnTowardWorkH0D5B
; @run 70:B651
; WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward OBJ_WORK_H less $70 and counts OBJ_WORK_I down, stepping the
; substate on at zero (Stage 08's overlay).
; HOW: TurnAngleToward, then DEC.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
TurnTowardWorkH0D5B:
    lda a:OBJ_WORK_H,x
    sec
    sbc #$70
    sta z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    dec a:OBJ_WORK_I,x
    bne TurnTowardWorkH0D5B_at0
    inc a:OBJ_SUBSTATE,x
TurnTowardWorkH0D5B_at0:
    rts

; @routine 0D5B:6EC SpawnKind9COnBeat0D5B
; @run 70:B6EC
; WHAT IT DOES: When OBJ_WORK_O's low five bits match one of eight values, starts an object of kind $9C at the
; object's place in a free slot from $30 and plays sound $15 (Stage 08's overlay).
; HOW: A search of the table, FindFreeSlotFrom30, StartObjectAtPlace, SoundCommand.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SpawnKind9COnBeat0D5BBeats = $B70D ; the eight counts at which an object starts
SpawnKind9COnBeat0D5B:
    cly
SpawnKind9COnBeat0D5B_at1:
    lda a:OBJ_WORK_O,x
    and #$1F
    cmp a:SpawnKind9COnBeat0D5BBeats,y
    beq SpawnKind9COnBeat0D5B_at0
    iny
    cpy #$08
    bcc SpawnKind9COnBeat0D5B_at1
    rts
SpawnKind9COnBeat0D5B_at0:
    jsr VecFindFreeSlotFrom30
    bcc SpawnKind9COnBeat0D5B_at2
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$15
    jsr VecSoundCommand
SpawnKind9COnBeat0D5B_at2:
    rts

; @routine 0D5B:72F StartWithHitBox82_0D5B
; @run 70:B72F
; WHAT IT DOES: Starts the object and sets its hit box byte to $82 (Stage 08's overlay).
; HOW: LoadObjectRecord, one store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWithHitBox82_0D5B:
    jsr VecLoadObjectRecord
    lda #$82
    sta a:OBJ_HITBOX_ON,x
    rts

; @routine 0D5B:738 AimPartFromBuffer0D5B
; @run 70:B738
; WHAT IT DOES: Unless OBJ_WORK_K is 8, takes the angle OBJ_WORK_D from the buffer StartNineParts0D5B filled at the
; place a table gives by OBJ_WORK_K, eases the speed (EaseWorkFToTarget0D5B) and sets the speed at that angle
; (Stage 08's overlay).
; HOW: Two table reads, a call, SetVelocityFromAngle with OBJ_WORK_G and OBJ_WORK_F as the speed.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AimPartFromBuffer0D5BBuffer = $B3A3 ; the 128-byte angle buffer
AimPartFromBuffer0D5BPlaces = $B75E ; the buffer index for each part 0 to 7
AimPartFromBuffer0D5B:
    lda a:OBJ_WORK_K,x
    cmp #$08
    beq AimPartFromBuffer0D5B_at0
    tay
    lda a:AimPartFromBuffer0D5BPlaces,y
    tay
    lda a:AimPartFromBuffer0D5BBuffer,y
    sta a:OBJ_WORK_D,x
    jsr EaseWorkFToTarget0D5B
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
AimPartFromBuffer0D5B_at0:
    rts

; @routine 0D5B:76E MoveThenBySubstate0D5B
; @run 70:B76E
; WHAT IT DOES: Eases the speed (EaseWorkFToTarget0D5B), sets the speed at angle OBJ_WORK_D, then runs the routine
; for its substate (Stage 08's overlay).
; HOW: A call, SetVelocityFromAngle, then a jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveThenBySubstate0D5BJumps = $B793 ; the routine for each substate, a word each
MoveThenBySubstate0D5B:
    jsr EaseWorkFToTarget0D5B
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:MoveThenBySubstate0D5BJumps,y
    sta z:zTemp00
    lda a:MoveThenBySubstate0D5BJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5B:7B1 TurnPartToTarget0D5B
; @run 70:B7B1
; WHAT IT DOES: Counts OBJ_WORK_I down and turns the angle OBJ_WORK_D by 2 toward the target a table gives by
; OBJ_WORK_K, keeping it even; on reaching it the substate steps on (Stage 08's overlay).
; HOW: A table read, TurnAngleToward, AND #$FE and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnPartToTarget0D5BTargets = $B7D5 ; the target angle for each part
TurnPartToTarget0D5B:
    dec a:OBJ_WORK_I,x
    ldy a:OBJ_WORK_K,x
    lda a:TurnPartToTarget0D5BTargets,y
    sta z:zTemp02
    lda #$02
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    and #$FE
    sta a:OBJ_WORK_D,x
    cmp z:zTemp02
    bne TurnPartToTarget0D5B_at0
    inc a:OBJ_SUBSTATE,x
TurnPartToTarget0D5B_at0:
    rts

; @routine 0D5C:09A BlinkThenRise0D5C
; @run 70:B89A
; WHAT IT DOES: Counts OBJ_WORK_M down, switching between two frames as it goes; at zero moves the object up 4
; pixels, sets its Y speed to -1 and runs BecomeWorkLKind0D5C (Stage 08's overlay).
; HOW: DEC; the count's bit 0 indexes the frames; ApplyVelocityY, stores, then falls into the next routine.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BlinkThenRise0D5CFrames = $B8C6 ; the two frames switched between
BlinkThenRise0D5C:
    dec a:OBJ_WORK_M,x
    beq BlinkThenRise0D5C_at0
    lda a:OBJ_WORK_M,x
    and #$01
    tay
    lda a:BlinkThenRise0D5CFrames,y
    sta a:OBJ_FRAME,x
    rts
BlinkThenRise0D5C_at0:
    lda #$FC
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$FF
    sta a:OBJ_VY_HI,x
    sta a:OBJ_VY_LO,x

; @routine 0D5C:0C8 ActByStateUnlessBroken0D5C
; @run 70:B8C8
; WHAT IT DOES: Runs BreakUnlessLeaderAlive0D5C; unless that set the carry, runs the routine for the object's state
; (Stage 08's overlay).
; HOW: A call, then a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByStateUnlessBroken0D5CJumps = $B8E0 ; the routine for each state, a word each
ActByStateUnlessBroken0D5C:
    jsr BreakUnlessLeaderAlive0D5C
    bcc ActByStateUnlessBroken0D5C_at0
    rts
ActByStateUnlessBroken0D5C_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByStateUnlessBroken0D5CJumps,y
    sta z:zTemp00
    lda a:ActByStateUnlessBroken0D5CJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5C:11B FollowYAndCount0D5C
; @run 70:B91B
; WHAT IT DOES: Puts the object at the place of slot Y and counts OBJ_WORK_F down by eights of OBJ_WORK_E: every 8
; frames OBJ_WORK_E steps down, and at zero the state steps on (Stage 08's overlay).
; HOW: CopyObjectPlaceBack, then two nested countdowns.
; WAITS: no.
; Entry: X the object; Y the slot whose place it takes. Exit: A, X, Y and the flags as its last instruction or call
; left them.
FollowYAndCount0D5C:
    jsr VecCopyObjectPlaceBack
    dec a:OBJ_WORK_F,x
    bne FollowYAndCount0D5C_at0
    lda #$08
    sta a:OBJ_WORK_F,x
    dec a:OBJ_WORK_E,x
    bne FollowYAndCount0D5C_at0
    inc a:OBJ_STATE,x
FollowYAndCount0D5C_at0:
    rts

; @routine 0D5C:131 LaunchWhenOwnerAnimEnds0D5C
; @run 70:B931
; WHAT IT DOES: Plays the object's frame list; clears it once the slot in OBJ_WORK_D is in state 4, else puts it at
; that slot's place and plays that slot's own list; when that list ends, launches the object toward the player at
; speed $3C0 with hit box 2 and steps its state on, or clears it when the angle is under $40 or $C0 and up (Stage
; 08's overlay).
; HOW: StepObjectFrames twice with zRichterObject keeping the slot, CopyObjectPlaceBack, AngleToPlayer and two
; compares, SetVelocityFromAngle.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LaunchWhenOwnerAnimEnds0D5C:
    lda #$87
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    ldy a:OBJ_WORK_D,x
    lda a:OBJ_STATE,y
    cmp #$04
    bne LaunchWhenOwnerAnimEnds0D5C_at0
    jmp VecClearObject
LaunchWhenOwnerAnimEnds0D5C_at0:
    jsr VecCopyObjectPlaceBack
    stx z:zClimbStepBefore
    sxy
    lda #$8E
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    php
    ldx z:zClimbStepBefore
    plp
    bne LaunchWhenOwnerAnimEnds0D5C_at1
    jsr VecAngleToPlayer
    cmp #$40
    bcs LaunchWhenOwnerAnimEnds0D5C_at2
    jmp VecClearObject
LaunchWhenOwnerAnimEnds0D5C_at2:
    cmp #$C0
    bcc LaunchWhenOwnerAnimEnds0D5C_at3
    jmp VecClearObject
LaunchWhenOwnerAnimEnds0D5C_at3:
    pha
    lda #$C0
    sta z:zTemp00
    lda #$03
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
LaunchWhenOwnerAnimEnds0D5C_at1:
    rts

; @routine 0D5C:1BD FlyUntilFarOff0D5C
; @run 70:B9BD
; WHAT IT DOES: Clears the object once its X or Y high byte is 2 or $FE; otherwise plays its frame list and moves
; it by its speed (Stage 08's overlay).
; HOW: Four compares, then StepObjectFrames and ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlyUntilFarOff0D5C:
    lda a:OBJ_X_HI,x
    cmp #$02
    bne FlyUntilFarOff0D5C_at0
    jmp VecClearObject
FlyUntilFarOff0D5C_at0:
    cmp #$FE
    bne FlyUntilFarOff0D5C_at1
    jmp VecClearObject
FlyUntilFarOff0D5C_at1:
    lda a:OBJ_Y_HI,x
    cmp #$02
    bne FlyUntilFarOff0D5C_at2
    jmp VecClearObject
FlyUntilFarOff0D5C_at2:
    cmp #$FE
    bne FlyUntilFarOff0D5C_at3
    jmp VecClearObject
FlyUntilFarOff0D5C_at3:
    lda #$87
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jmp VecApplyVelocity

; @routine 0D5C:1ED ActByStateAfterHitCheck0D5C
; @run 70:B9ED
; WHAT IT DOES: Runs BreakUpWhenHit0D5C, then the routine for the object's state (Stage 08's overlay).
; HOW: A call, then a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByStateAfterHitCheck0D5CJumps = $BA02 ; the routine for each state, a word each
ActByStateAfterHitCheck0D5C:
    jsr BreakUpWhenHit0D5C
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByStateAfterHitCheck0D5CJumps,y
    sta z:zTemp00
    lda a:ActByStateAfterHitCheck0D5CJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5C:252 StartWithKind26Partner0D5C
; @run 70:BA52
; WHAT IT DOES: Starts the object with OBJ_WORK_E $60 and OBJ_WORK_D $F0 and an object of kind $26 at its place in
; the next slot; with OBJ_START_ARG negative it begins in state 5 (Stage 08's overlay).
; HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace, stores, a sign test.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWithKind26Partner0D5C:
    jsr VecLoadObjectRecord
    lda #$60
    sta a:OBJ_WORK_E,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$26
    jsr VecStartObjectAtPlace
    lda #$F0
    sta a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    bpl StartWithKind26Partner0D5C_at0
    lda #$05
    sta a:OBJ_STATE,x
StartWithKind26Partner0D5C_at0:
    rts

; @routine 0D5C:2B0 BounceThreeTimes0D5C
; @run 70:BAB0
; WHAT IT DOES: Moves the object under gravity; on reaching Y $B8 it stops there and, for its first three landings,
; bounces with the Y speed a table gives by OBJ_WORK_J; after that OBJ_WORK_D becomes $90 and the state 2 (Stage
; 08's overlay).
; HOW: ApplyGravity and ApplyVelocity, a compare of Y, a table read by twice OBJ_WORK_J.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BounceThreeTimes0D5CSpeeds = $BAEB ; the Y speed of each bounce, a word each
BounceThreeTimes0D5C:
    jsr VecApplyGravity
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    bne BounceThreeTimes0D5C_at0
    lda a:OBJ_Y_LO,x
    cmp #$B8
    bcc BounceThreeTimes0D5C_at0
    lda #$B8
    sta a:OBJ_Y_LO,x
    lda a:OBJ_WORK_J,x
    cmp #$03
    bcs BounceThreeTimes0D5C_at1
    asl a
    tay
    lda a:BounceThreeTimes0D5CSpeeds,y
    sta a:OBJ_VY_LO,x
    lda a:BounceThreeTimes0D5CSpeeds+1,y
    sta a:OBJ_VY_HI,x
    inc a:OBJ_WORK_J,x
    rts
BounceThreeTimes0D5C_at1:
    lda #$90
    sta a:OBJ_WORK_D,x
    lda #$02
    sta a:OBJ_STATE,x
BounceThreeTimes0D5C_at0:
    rts

; @routine 0D5C:2F3 StateOnPlayerNear0D5C
; @run 70:BAF3
; WHAT IT DOES: Steps the object's state on once the player is less than $78 pixels away along X (Stage 08's
; overlay).
; HOW: DistanceXToPlayer and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StateOnPlayerNear0D5C:
    jsr VecDistanceXToPlayer
    cmp #$78
    bcs StateOnPlayerNear0D5C_at0
    inc a:OBJ_STATE,x
StateOnPlayerNear0D5C_at0:
    rts

; @routine 0D5C:2FE SpawnSixKind27AfterAnim0D5C
; @run 70:BAFE
; WHAT IT DOES: Plays the object's frame list; when it ends, steps the state on with hit box 2 and health 3 and
; starts six objects of kind $27 at its place in the slots two to seven after it, each with the object's slot in
; OBJ_WORK_L, its index 0 to 5 in OBJ_WORK_M and OBJ_WORK_D 3, $0B, $13 and so on (Stage 08's overlay).
; HOW: StepObjectFrames, stores, a loop of ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SpawnSixKind27AfterAnim0D5C:
    lda #$4F
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne SpawnSixKind27AfterAnim0D5C_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$03
    sta a:OBJ_HEALTH,x
    txa
    tay
    iny
    iny
    lda #$06
    sta z:zTemp00
    lda #$03
    sta z:zTemp01
    stz z:zTemp02
SpawnSixKind27AfterAnim0D5C_at1:
    jsr VecClearObjectInY
    lda #$27
    jsr VecStartObjectAtPlace
    txa
    sta a:OBJ_WORK_L,y
    lda z:zTemp01
    sta a:OBJ_WORK_D,y
    clc
    adc #$08
    sta z:zTemp01
    lda z:zTemp02
    sta a:OBJ_WORK_M,y
    inc z:zTemp02
    iny
    dec z:zTemp00
    bne SpawnSixKind27AfterAnim0D5C_at1
SpawnSixKind27AfterAnim0D5C_at0:
    rts

; @routine 0D5C:35C HomeOnPlayerWavering0D5C
; @run 70:BB5C
; WHAT IT DOES: Plays the object's frame list and turns its angle OBJ_WORK_D by 1 toward the point $20 above the
; player, or toward $C0 when OBJ_START_ARG is 1 and slot $10 holds kind $22; then moves at that angle with speed
; $140 plus a cosine swing by OBJ_WORK_H, which steps by 2. In that second case, once its Y high byte is not 0 it
; clears its own and the next seven slots (Stage 08's overlay).
; HOW: StepObjectFrames, AngleToPlayerOffset, TurnAngleToward, ScaleByCosine, SetVelocityFromAngle, ApplyVelocity;
; a loop of ClearObjectInY.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HomeOnPlayerWavering0D5C:
    lda #$DB
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_START_ARG,x
    cmp #$01
    bne HomeOnPlayerWavering0D5C_at0
    lda a:OBJ_KIND+16
    cmp #$22
    bne HomeOnPlayerWavering0D5C_at0
    lda a:OBJ_Y_HI,x
    beq HomeOnPlayerWavering0D5C_at1
    txa
    tay
    lda #$08
    sta z:zTemp00
HomeOnPlayerWavering0D5C_at2:
    jsr VecClearObjectInY
    iny
    dec z:zTemp00
    bne HomeOnPlayerWavering0D5C_at2
    rts
HomeOnPlayerWavering0D5C_at1:
    lda #$C0
    bra HomeOnPlayerWavering0D5C_at3
HomeOnPlayerWavering0D5C_at0:
    stz z:zTemp08
    stz z:zTemp09
    lda #$E0
    sta z:zTemp0A
    lda #$FF
    sta z:zTemp0B
    jsr VecAngleToPlayerOffset
HomeOnPlayerWavering0D5C_at3:
    sta z:zTemp02
    lda #$01
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    lda #$C0
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_WORK_H,x
    jsr VecScaleByCosine
    lda a:OBJ_WORK_H,x
    clc
    adc #$02
    sta a:OBJ_WORK_H,x
    lda z:SpriteFlipMask
    clc
    adc #$40
    sta z:zTemp00
    lda z:zPlaneRowsLeft
    adc #$01
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 0D5C:409 RideLeaderThenRelease0D5C
; @run 70:BC09
; WHAT IT DOES: Puts the object at the place of the slot in OBJ_WORK_L; once that slot's state is 3 or more, counts
; OBJ_WORK_D down and at zero gives the object the frame a table picks by OBJ_WORK_M, OBJ_WORK_E $40 and the next
; state (Stage 08's overlay).
; HOW: CopyObjectPlaceBack, a compare, DEC, a table read.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RideLeaderThenRelease0D5CFrames = $BC2D ; the frame for each OBJ_WORK_M
RideLeaderThenRelease0D5C:
    ldy a:OBJ_WORK_L,x
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_STATE,y
    cmp #$03
    bcc RideLeaderThenRelease0D5C_at0
    dec a:OBJ_WORK_D,x
    bne RideLeaderThenRelease0D5C_at0
    ldy a:OBJ_WORK_M,x
    lda a:RideLeaderThenRelease0D5CFrames,y
    sta a:OBJ_FRAME,x
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
RideLeaderThenRelease0D5C_at0:
    rts

; @routine 0D5C:433 SwingAndSpeedUp0D5C
; @run 70:BC33
; WHAT IT DOES: Takes the sprite attributes of the slot in OBJ_WORK_L and moves (FollowAndMoveAtAngle0D5C); counts
; OBJ_WORK_N down to turn into kind $9B in state 0 with sound $10. Otherwise raises the speed OBJ_WORK_J-K by 8 a
; frame up to $600, swings the angle OBJ_WORK_E-F by a cosine of OBJ_WORK_G, and steps OBJ_WORK_G-H by a word of
; four phases picked by OBJ_WORK_I; when that carries the angle goes back to $40 and the phase steps on (Stage 08's
; overlay).
; HOW: A call, DEC, SoundCommand, 16-bit adds, ScaleByCosine and a table read by twice OBJ_WORK_I's low two bits.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SwingAndSpeedUp0D5CSteps = $BCB6 ; the step of OBJ_WORK_G-H for each phase, a word each
SwingAndSpeedUp0D5C:
    ldy a:OBJ_WORK_L,x
    lda a:OBJ_SPRITE_ATTR,y
    sta a:OBJ_SPRITE_ATTR,x
    jsr FollowAndMoveAtAngle0D5C
    lda a:OBJ_WORK_N,x
    beq SwingAndSpeedUp0D5C_at0
    dec a:OBJ_WORK_N,x
    bne SwingAndSpeedUp0D5C_at0
    lda #$10
    jsr VecSoundCommand
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
SwingAndSpeedUp0D5C_at0:
    lda a:OBJ_WORK_J,x
    cmp #$06
    bcs SwingAndSpeedUp0D5C_at1
    lda a:OBJ_WORK_K,x
    clc
    adc #$08
    sta a:OBJ_WORK_K,x
    lda a:OBJ_WORK_J,x
    adc #$00
    sta a:OBJ_WORK_J,x
SwingAndSpeedUp0D5C_at1:
    lda #$04
    sta z:zTemp01
    lda #$00
    sta z:zTemp00
    lda a:OBJ_WORK_G,x
    jsr VecScaleByCosine
    lda a:OBJ_WORK_F,x
    clc
    adc z:SpriteFlipMask
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    adc z:zPlaneRowsLeft
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_I,x
    and #$03
    asl a
    tay
    lda a:OBJ_WORK_H,x
    clc
    adc a:SwingAndSpeedUp0D5CSteps,y
    sta a:OBJ_WORK_H,x
    lda a:OBJ_WORK_G,x
    adc a:SwingAndSpeedUp0D5CSteps+1,y
    sta a:OBJ_WORK_G,x
    bcc SwingAndSpeedUp0D5C_at2
    lda #$40
    sta a:OBJ_WORK_E,x
    stz a:OBJ_WORK_F,x
    inc a:OBJ_WORK_I,x
SwingAndSpeedUp0D5C_at2:
    rts

; @routine 0D5C:4BE FollowAndMoveAtAngle0D5C
; @run 70:BCBE
; WHAT IT DOES: Puts the object at the place of the slot below it, or with OBJ_WORK_M 0 of the slot two below and 4
; pixels lower; then moves it at angle OBJ_WORK_E with the speed in OBJ_WORK_J-K (Stage 08's overlay).
; HOW: CopyObjectPlaceBack, a 16-bit add to Y, SetVelocityFromAngle and ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FollowAndMoveAtAngle0D5C:
    txa
    tay
    dey
    lda a:OBJ_WORK_M,x
    beq FollowAndMoveAtAngle0D5C_at0
    jsr VecCopyObjectPlaceBack
    bra FollowAndMoveAtAngle0D5C_at1
FollowAndMoveAtAngle0D5C_at0:
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_Y_LO,x
    clc
    adc #$04
    sta a:OBJ_Y_LO,x
    bcc FollowAndMoveAtAngle0D5C_at1
    inc a:OBJ_Y_HI,x
FollowAndMoveAtAngle0D5C_at1:
    lda a:OBJ_WORK_K,x
    sta z:zTemp00
    lda a:OBJ_WORK_J,x
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 0D5C:4F1 FaceTowardPlayer0D5C
; @run 70:BCF1
; WHAT IT DOES: In state 0 starts the object; then it takes the sprite attributes and place of the slot below it
; and the frame $13 to $1A that points toward the player, one per eighth of a turn (Stage 08's overlay).
; HOW: LoadObjectRecord, or CopyObjectPlace, AngleToPlayer plus $10 shifted down five bits plus $13.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FaceTowardPlayer0D5C:
    lda a:OBJ_STATE,x
    bne FaceTowardPlayer0D5C_at0
    jmp VecLoadObjectRecord
FaceTowardPlayer0D5C_at0:
    lda a:OBJ_SPRITE_ATTR-1,x
    sta a:OBJ_SPRITE_ATTR,x
    txa
    tay
    dey
    sxy
    jsr VecCopyObjectPlace
    sxy
    jsr VecAngleToPlayer
    clc
    adc #$10
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc #$13
    sta a:OBJ_FRAME,x
    rts

; @routine 0D5C:525 StartAndAnimate0D5C
; @run 70:BD25
; WHAT IT DOES: In state 0 starts the object with sprite set 6; plays its frame list every frame (Stage 08's
; overlay).
; HOW: LoadObjectRecord, a store, StepObjectFrames.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartAndAnimate0D5C:
    lda a:OBJ_STATE,x
    bne StartAndAnimate0D5C_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
StartAndAnimate0D5C_at0:
    lda #$3D
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0D5C:562 StartThenStateByArg0D5C
; @run 70:BD62
; WHAT IT DOES: Starts the object and runs StateByStartArg0D5C (Stage 08's overlay).
; HOW: LoadObjectRecord, then falls into the next piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartThenStateByArg0D5C:
    jsr VecLoadObjectRecord

; @routine 0D5C:565 StateByStartArg0D5C
; @run 70:BD65
; WHAT IT DOES: Sets the object's state by OBJ_START_ARG: positive runs State3UnlessEnd0D5C, negative gives state
; 5; with 0, unless OBJ_WORK_D is 3, it runs SetFrame60_0D5C (Stage 08's overlay).
; HOW: Sign and compare tests, then jumps or falls into the next routine.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StateByStartArg0D5C:
    lda a:OBJ_START_ARG,x
    beq StateByStartArg0D5C_at0
    bpl State3UnlessEnd0D5C
    lda #$05
    bra SetStateFromA0D5C
StateByStartArg0D5C_at0:
    lda a:OBJ_WORK_D,x
    cmp #$03
    beq ReturnOnly0D5C

; @routine 0D5C:57F ReturnOnly0D5C
; @run 70:BD7F
; WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
; HOW: An RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
ReturnOnly0D5C:
    rts

; @routine 0D5C:580 State3UnlessEnd0D5C
; @run 70:BD80
; WHAT IT DOES: Runs SetFrame60_0D5C unless OBJ_WORK_D is 3 or the object is in slot $23, then puts the object in
; state 3 (Stage 08's overlay).
; HOW: Two compares, a call, then falls into SetStateFromA0D5C with A 3.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
State3UnlessEnd0D5C:
    lda a:OBJ_WORK_D,x
    cmp #$03
    beq State3UnlessEnd0D5C_at0
    cpx #$23
    beq State3UnlessEnd0D5C_at0
    bsr SetFrame60_0D5C
State3UnlessEnd0D5C_at0:
    lda #$03

; @routine 0D5C:58F SetStateFromA0D5C
; @run 70:BD8F
; WHAT IT DOES: Puts the object in the state in A (Stage 08's overlay).
; HOW: One store.
; WAITS: no.
; Entry: X the object; A the state. Exit: A, X, Y and the flags as its last instruction or call left them.
SetStateFromA0D5C:
    sta a:OBJ_STATE,x
    rts

; @routine 0D5C:593 BecomeKindACFrame5C0D5C
; @run 70:BD93
; WHAT IT DOES: Turns the object into kind $AC, starts it with frame $5C and sets its state by OBJ_START_ARG
; (StateByStartArg0D5C) (Stage 08's overlay).
; HOW: A store, LoadObjectRecord, a store, then the piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BecomeKindACFrame5C0D5C:
    lda #$AC
    sta a:OBJ_KIND,x
    jsr VecLoadObjectRecord
    lda #$5C
    sta a:OBJ_FRAME,x
    bra StateByStartArg0D5C

; @routine 0D5C:5A2 StartFrame5F0D5C
; @run 70:BDA2
; WHAT IT DOES: Starts the object with OBJ_WORK_C 0, sprite set 6 and frame $5F (Stage 08's overlay).
; HOW: LoadObjectRecord, three stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartFrame5F0D5C:
    jsr VecLoadObjectRecord
    stz a:OBJ_WORK_C,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$5F
    sta a:OBJ_FRAME,x
    rts

; @routine 0D5C:5B3 BuildChainOfKindBA0D5C
; @run 70:BDB3
; WHAT IT DOES: By OBJ_WORK_D 3, 2 or 1 runs BecomeKindACFrame5C0D5C, StartFrame5F0D5C or StartThenStateByArg0D5C.
; With 0 starts the object with frame $5D and builds a chain of kind $BA objects in the slots after it, each with
; the slot before it in OBJ_WORK_F: with OBJ_START_ARG 0 five (OBJ_WORK_E $0E, the last OBJ_WORK_D 2), then six, or
; seven for a negative argument, that take its OBJ_START_ARG and OBJ_WORK_E $E0, or $72 for 0, the last OBJ_WORK_D
; 3 and the first the object's slot; then clears the slots up to $2B (Stage 08's overlay).
; HOW: Compares of OBJ_WORK_D, LoadObjectRecord, loops of ClearObjectInY and StartObjectAtPlace, a clearing loop.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
BuildChainOfKindBA0D5C:
    lda a:OBJ_WORK_D,x
    cmp #$03
    beq BecomeKindACFrame5C0D5C
    cmp #$02
    beq StartFrame5F0D5C
    cmp #$01
    beq StartThenStateByArg0D5C
    jsr VecLoadObjectRecord
    lda #$5D
    sta a:OBJ_FRAME,x
    lda #$05
    sta z:zTemp00
    txa
    tay
    iny
    lda a:OBJ_START_ARG,x
    beq BuildChainOfKindBA0D5C_at0
    bpl BuildChainOfKindBA0D5C_at1
    lda #$07
    bra BuildChainOfKindBA0D5C_at2
BuildChainOfKindBA0D5C_at0:
    jsr VecClearObjectInY
    lda #$BA
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,y
    lda #$0E
    sta a:OBJ_WORK_E,y
    tya
    dec a
    sta a:OBJ_WORK_F,y
    iny
    dec z:zTemp00
    bne BuildChainOfKindBA0D5C_at0
    lda #$02
    sta a:OBJ_WORK_D-1,y
BuildChainOfKindBA0D5C_at1:
    lda #$06
BuildChainOfKindBA0D5C_at2:
    sta z:zTemp00
    sta z:zTemp01
BuildChainOfKindBA0D5C_at5:
    jsr VecClearObjectInY
    lda #$BA
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,y
    lda a:OBJ_START_ARG,x
    sta a:OBJ_START_ARG,y
    beq BuildChainOfKindBA0D5C_at3
    lda #$E0
    bra BuildChainOfKindBA0D5C_at4
BuildChainOfKindBA0D5C_at3:
    lda #$72
BuildChainOfKindBA0D5C_at4:
    sta a:OBJ_WORK_E,y
    tya
    dec a
    sta a:OBJ_WORK_F,y
    iny
    dec z:zTemp00
    bne BuildChainOfKindBA0D5C_at5
    lda #$03
    sta a:OBJ_WORK_D-1,y
    phy
    tya
    sec
    sbc z:zTemp01
    tay
    txa
    sta a:OBJ_WORK_F,y
    ply
BuildChainOfKindBA0D5C_at7:
    cpy #$2C
    beq BuildChainOfKindBA0D5C_at6
    jsr VecClearObjectInY
    iny
    bra BuildChainOfKindBA0D5C_at7
BuildChainOfKindBA0D5C_at6:
    rts

; @routine 0D5C:78C SwitchBlockByEvent0D5C
; @run 70:BF8C
; WHAT IT DOES: In state 0 starts the object with hit box Y offset $10; in state 1, once the event flag
; OBJ_START_ARG names is set, steps OBJ_WORK_D and the state on; in every state after 0 it is a block the player
; can stand on, $18 wide and $16 high (Stage 08's overlay).
; HOW: LoadObjectRecord, or BitOfIndex and an AND of the event flags, then ContactWithPlayer.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SwitchBlockByEvent0D5C:
    lda a:OBJ_STATE,x
    bne SwitchBlockByEvent0D5C_at0
    jsr VecLoadObjectRecord
    lda #$10
    sta a:OBJ_HITBOX_DY,x
    rts
SwitchBlockByEvent0D5C_at0:
    dec a
    bne SwitchBlockByEvent0D5C_at1
    lda a:OBJ_START_ARG,x
    jsr VecBitOfIndex
    and a:EventFlags,y
    beq SwitchBlockByEvent0D5C_at1
    inc a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
SwitchBlockByEvent0D5C_at1:
    lda #$18
    sta z:zTemp08
    lda #$16
    sta z:zTemp09
    jsr ContactWithPlayer
    rts

; @routine 0D5C:7BB SagUnderSlotArg0D5C
; @run 70:BFBB
; WHAT IT DOES: In state 0 starts the object with sprite set 6 and keeps in OBJ_WORK_D the Y of the slot
; OBJ_START_ARG names. In state 1, while that slot's OBJ_WORK_D is 0, counts OBJ_WORK_E up and picks a sag 0 to
; $1F: 0 below $40, then growing by a speed that gains $10 a frame, then from $80 shrinking back to 0; it then sets
; the frame and that slot's Y (SetFrameAndSlotY0D5D); once that slot's OBJ_WORK_D is not 0 the state steps on
; (Stage 08's overlay).
; HOW: LoadObjectRecord; a count, 16-bit adds of a speed to OBJ_WORK_F-G, a clamp, then the shared piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SagUnderSlotArg0D5C:
    lda a:OBJ_STATE,x
    bne SagUnderSlotArg0D5C_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_Y_LO,y
    sta a:OBJ_WORK_D,x
    rts
SagUnderSlotArg0D5C_at0:
    dec a
    bne SagUnderSlotArg0D5C_at1
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_WORK_D,y
    bne SagUnderSlotArg0D5C_at2
    inc a:OBJ_WORK_E,x
    beq SagUnderSlotArg0D5C_at3
    lda a:OBJ_WORK_E,x
    bmi SagUnderSlotArg0D5C_at4
    cmp #$40
    bcc SagUnderSlotArg0D5C_at5
    lda #$10
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,x
    bcc SagUnderSlotArg0D5C_at6
    inc a:OBJ_VY_HI,x
SagUnderSlotArg0D5C_at6:
    lda a:OBJ_WORK_F,x
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_G,x
    adc a:OBJ_VY_HI,x
    sta a:OBJ_WORK_G,x
    cmp #$1F
    bcc SetFrameAndSlotY0D5D
    lda #$1F
    bra SetFrameAndSlotY0D5D
SagUnderSlotArg0D5C_at3:
    stz a:OBJ_VY_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_WORK_F,x
    stz a:OBJ_WORK_G,x
SagUnderSlotArg0D5C_at5:
    cla
    bra SetFrameAndSlotY0D5D
SagUnderSlotArg0D5C_at4:
    eor #$FF
    lsr a
    lsr a
    bra SetFrameAndSlotY0D5D
SagUnderSlotArg0D5C_at2:
    inc a:OBJ_STATE,x
SagUnderSlotArg0D5C_at1:
    rts

; @routine 0D5D:02D SwaySlotArgBySine0D5D
; @run 71:C02D
; WHAT IT DOES: In state 0 starts the object with sprite set 6 and keeps in OBJ_WORK_D the Y of the slot
; OBJ_START_ARG names. In state 1, while that slot's OBJ_WORK_D is 0, steps OBJ_WORK_E and takes a sag of $10 plus
; a sine swing, 0 to $1F, for the frame and that slot's Y (SetFrameAndSlotY0D5D); once it is not 0 the state steps
; on (Stage 08's overlay).
; HOW: LoadObjectRecord; ScaleBySine of twice OBJ_WORK_E by $0FFF, then the shared piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SwaySlotArgBySine0D5D:
    lda a:OBJ_STATE,x
    bne SwaySlotArgBySine0D5D_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_Y_LO,y
    sta a:OBJ_WORK_D,x
    rts
SwaySlotArgBySine0D5D_at0:
    dec a
    bne ReturnOnly0D5D
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_WORK_D,y
    bne StepStateThenReturn0D5D
    lda #$FF
    sta z:zTemp00
    lda #$0F
    sta z:zTemp01
    inc a:OBJ_WORK_E,x
    lda a:OBJ_WORK_E,x
    asl a
    jsr VecScaleBySine
    lda z:zPlaneRowsLeft
    clc
    adc #$10
    and #$1F

; @routine 0D5D:068 SetFrameAndSlotY0D5D
; @run 71:C068
; WHAT IT DOES: From a sag in A gives the object frame $33 less the sag, and puts the slot OBJ_START_ARG names at Y
; OBJ_WORK_D plus twice the sag (Stage 08's overlay).
; HOW: A subtract, a doubled add, two stores.
; WAITS: no.
; Entry: X the object; A the sag, 0 to $1F. Exit: A, X, Y and the flags as its last instruction or call left them.
SetFrameAndSlotY0D5D:
    sta z:zTemp10
    lda #$33
    sec
    sbc z:zTemp10
    sta a:OBJ_FRAME,x
    lda z:zTemp10
    asl a
    clc
    adc a:OBJ_WORK_D,x
    ldy a:OBJ_START_ARG,x
    sta a:OBJ_Y_LO,y
    rts

; @routine 0D5D:080 StepStateThenReturn0D5D
; @run 71:C080
; WHAT IT DOES: Steps the object's state on (Stage 08's overlay).
; HOW: INC, then falls into the RTS.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StepStateThenReturn0D5D:
    inc a:OBJ_STATE,x

; @routine 0D5D:083 ReturnOnly0D5D
; @run 71:C083
; WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
; HOW: An RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
ReturnOnly0D5D:
    rts

; @routine 0D5D:084 SlideInThenRideOrFall0D5D
; @run 71:C084
; WHAT IT DOES: In state 0 starts the object moving left at 4 pixels a frame (hit box X offset -8, OBJ_WORK_D $35);
; state 2 plays sound 6, records its event and moves on. In state 3 it moves along X counting OBJ_WORK_D down, at
; $25 stepping on the state of the slot OBJ_START_ARG's low bits name; hit once or twice it keeps its offset from
; the player and stays there 16 frames before it is cleared (state 5); hit 3 times or more, or when the count runs
; out with OBJ_START_ARG not negative, it wakes that slot (WakeSlotInStartArg0D5D) and falls away with X speed $80
; and Y speed -2, gaining $20 a frame (state 4) (Stage 08's overlay).
; HOW: A chain of DEC tests of the state; LoadObjectRecord, SoundCommand, RecordObjectEvent, ApplyVelocityX,
; ApplyVelocity, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SlideInThenRideOrFall0D5D:
    lda a:OBJ_STATE,x
    bne SlideInThenRideOrFall0D5D_at0
    jsr VecLoadObjectRecord
    lda #$F8
    sta a:OBJ_HITBOX_DX,x
    lda #$FC
    sta a:OBJ_VX_HI,x
    lda #$35
    sta a:OBJ_WORK_D,x
    rts
SlideInThenRideOrFall0D5D_at0:
    dec a
    bne SlideInThenRideOrFall0D5D_at1
SlideInThenRideOrFall0D5D_at7:
    rts
SlideInThenRideOrFall0D5D_at1:
    dec a
    bne SlideInThenRideOrFall0D5D_at2
    lda #$06
    jsr VecSoundCommand
    jsr VecRecordObjectEvent
SlideInThenRideOrFall0D5D_at8:
    inc a:OBJ_STATE,x
    rts
SlideInThenRideOrFall0D5D_at2:
    dec a
    bne SlideInThenRideOrFall0D5D_at3
    lda a:OBJ_HIT_TAKEN,x
    beq SlideInThenRideOrFall0D5D_at4
    cmp #$03
    bcs SlideInThenRideOrFall0D5D_at5
    lda a:OBJ_X_LO,x
    sec
    sbc a:OBJ_X_LO
    sta a:OBJ_VX_LO,x
    lda a:OBJ_Y_LO,x
    sec
    sbc a:OBJ_Y_LO
    sta a:OBJ_VY_LO,x
    lda #$10
    sta a:OBJ_WORK_D,x
    jsr WakeSlotInStartArg0D5D
    inc a:OBJ_STATE,x
    inc a:OBJ_STATE,x
    rts
SlideInThenRideOrFall0D5D_at4:
    jsr VecApplyVelocityX
    dec a:OBJ_WORK_D,x
    beq SlideInThenRideOrFall0D5D_at6
    lda a:OBJ_WORK_D,x
    cmp #$25
    bne SlideInThenRideOrFall0D5D_at7
    lda a:OBJ_START_ARG,x
    and #$7F
    beq SlideInThenRideOrFall0D5D_at7
    tay
    lda a:OBJ_STATE,y
    inc a
    sta a:OBJ_STATE,y
    rts
SlideInThenRideOrFall0D5D_at6:
    lda a:OBJ_START_ARG,x
    bmi SlideInThenRideOrFall0D5D_at7
SlideInThenRideOrFall0D5D_at5:
    jsr WakeSlotInStartArg0D5D
    lda #$80
    sta a:OBJ_VX_LO,x
    stz a:OBJ_VX_HI,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    bra SlideInThenRideOrFall0D5D_at8
SlideInThenRideOrFall0D5D_at3:
    dec a
    bne SlideInThenRideOrFall0D5D_at9
    lda #$20
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_VY_LO,x
    bcc SlideInThenRideOrFall0D5D_at10
    inc a:OBJ_VY_HI,x
SlideInThenRideOrFall0D5D_at10:
    jmp VecApplyVelocity
SlideInThenRideOrFall0D5D_at9:
    dec a:OBJ_WORK_D,x
    beq SlideInThenRideOrFall0D5D_at11
    lda a:OBJ_X_LO
    clc
    adc a:OBJ_VX_LO,x
    sta a:OBJ_X_LO,x
    lda a:OBJ_Y_LO
    clc
    adc a:OBJ_VY_LO,x
    sta a:OBJ_Y_LO,x
    rts
SlideInThenRideOrFall0D5D_at11:
    jmp VecClearObject

; @routine 0D5D:159 MoveOnEventThenStand0D5D
; @run 71:C159
; @stop C181
; WHAT IT DOES: In state 0 starts the object with hit box X offset $14, X speed 2 and OBJ_WORK_D $28; in state 1,
; once the event flag OBJ_START_ARG names is set, steps on; in state 2 moves $28 frames then steps on. In states 1
; and up it is a block the player can stand on (StandOnBlock0D5D) (Stage 08's overlay).
; HOW: LoadObjectRecord, BitOfIndex and an AND of the event flags, or the pieces after it.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveOnEventThenStand0D5D:
    lda a:OBJ_STATE,x
    bne MoveOnEventThenStand0D5D_at0
    jsr VecLoadObjectRecord
    lda #$14
    sta a:OBJ_HITBOX_DX,x
    lda #$02
    sta a:OBJ_VX_HI,x
    lda #$28
    sta a:OBJ_WORK_D,x
    rts
MoveOnEventThenStand0D5D_at0:
    dec a
    bne MoveForCountThenStep0D5D
    lda a:OBJ_START_ARG,x
    jsr VecBitOfIndex
    and a:EventFlags,y
    bne StepState0D5D
    bra StandOnBlock0D5D

; @routine 0D5D:182 MoveForCountThenStep0D5D
; @run 71:C182
; WHAT IT DOES: In state 2 moves the object by its speed and counts OBJ_WORK_D down, stepping the state on at zero;
; in any state it then is a block the player can stand on (StandOnBlock0D5D) (Stage 08's overlay).
; HOW: DEC of the state copy in A, ApplyVelocity, DEC, then the pieces after it.
; WAITS: no.
; Entry: X the object; A its state less 1. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveForCountThenStep0D5D:
    dec a
    bne StandOnBlock0D5D
    jsr VecApplyVelocity
    dec a:OBJ_WORK_D,x
    bne StandOnBlock0D5D

; @routine 0D5D:18D StepState0D5D
; @run 71:C18D
; WHAT IT DOES: Steps the object's state on (Stage 08's overlay).
; HOW: INC and RTS.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StepState0D5D:
    inc a:OBJ_STATE,x
    rts

; @routine 0D5D:191 StandOnBlock0D5D
; @run 71:C191
; WHAT IT DOES: Lets the player stand on the object as a block $18 wide and $26 high, tested with its hit box X
; offset at 0 and then put back to $14 (Stage 08's overlay).
; HOW: Two stores around ContactWithPlayer.
; WAITS: no.
; Entry: X the object. Exit: C set when the player stands on it, from ContactWithPlayer.
StandOnBlock0D5D:
    stz a:OBJ_HITBOX_DX,x
    lda #$18
    sta z:zTemp08
    lda #$26
    sta z:zTemp09
    jsr ContactWithPlayer
    lda #$14
    sta a:OBJ_HITBOX_DX,x
    rts

; @routine 0D5D:1A5 AnimateWithHeightByFrame0D5D
; @run 71:C1A5
; WHAT IT DOES: In state 0 starts the object; plays its frame list and gives it the hit box half height of its
; frame from 3 on (Stage 08's overlay).
; HOW: LoadObjectRecord, StepObjectFrames, a table read by the frame less 3.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
AnimateWithHeightByFrame0D5DHeights = $C1C6 ; the hit box half height for each frame from 3
AnimateWithHeightByFrame0D5D:
    lda a:OBJ_STATE,x
    bne AnimateWithHeightByFrame0D5D_at0
    jsr VecLoadObjectRecord
AnimateWithHeightByFrame0D5D_at0:
    lda #$CE
    sta z:zTemp10
    lda #$C1
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    sec
    sbc #$03
    tay
    lda a:AnimateWithHeightByFrame0D5DHeights,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0D5D:1DB StepWhenStoodOn0D5D
; @run 71:C1DB
; WHAT IT DOES: Passes its record on (PassRecordFromState2_0D5D) and is a block $12 wide and $15 high; when the
; player stands on it and the first three of RichterColumnHit are 0, steps the substate on (Stage 08's overlay).
; HOW: Two calls, then an OR of three bytes.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StepWhenStoodOn0D5D:
    jsr PassRecordFromState2_0D5D
    lda #$12
    sta z:zTemp08
    lda #$15
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc StepWhenStoodOn0D5D_at0
    lda a:RichterColumnHit
    ora a:RichterColumnRow-4
    ora a:RichterColumnRow-3
    bne StepWhenStoodOn0D5D_at0
    inc a:OBJ_SUBSTATE,x
StepWhenStoodOn0D5D_at0:
    rts

; @routine 0D5D:1FA StartOrSinkBlock0D5D
; @run 71:C1FA
; WHAT IT DOES: In state 0 keeps its Y plus 8 in OBJ_WORK_D and starts with frame $0C (StartFrameC0D5D); in the
; other states runs SinkWhenStoodOn0D5D (Stage 08's overlay).
; HOW: A test of the state, an add, then falls into the next routine.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StartOrSinkBlock0D5D:
    lda a:OBJ_STATE,x
    bne SinkWhenStoodOn0D5D
    lda a:OBJ_Y_LO,x
    clc
    adc #$08
    sta a:OBJ_WORK_D,x

; @routine 0D5D:208 StartFrameC0D5D
; @run 71:C208
; WHAT IT DOES: Starts the object with sprite set 6 and frame $0C (Stage 08's overlay).
; HOW: LoadObjectRecord, two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartFrameC0D5D:
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$0C
    sta a:OBJ_FRAME,x
    rts

; @routine 0D5D:216 SinkWhenStoodOn0D5D
; @run 71:C216
; WHAT IT DOES: A block $12 wide and $15 high that passes its record on (PassRecordFromState2_0D5D). State 1: in
; substate 0 waits to be stood on (StepWhenStoodOn0D5D), then sinks a pixel a frame to the Y in OBJ_WORK_D, plays
; sound $C5 and steps on, or with OBJ_START_ARG 0 records its event and is cleared. State 2 steps on once
; OthersInState2_0D5D sets Z. Later states rise a pixel a frame back to 8 above OBJ_WORK_D and return to state 1,
; substate 0 (Stage 08's overlay).
; HOW: DEC tests of the state in A; the calls, INC or DEC of Y and a compare.
; WAITS: no.
; Entry: X the object; A its state. Exit: A, X, Y and the flags as its last instruction or call left them.
SinkWhenStoodOn0D5D:
    dec a
    bne SinkWhenStoodOn0D5D_at0
    lda a:OBJ_SUBSTATE,x
    beq StepWhenStoodOn0D5D
    jsr PassRecordFromState2_0D5D
    lda #$12
    sta z:zTemp08
    lda #$15
    sta z:zTemp09
    jsr ContactWithPlayer
    inc a:OBJ_Y_LO,x
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_WORK_D,x
    beq SinkWhenStoodOn0D5D_at1
    rts
SinkWhenStoodOn0D5D_at1:
    lda #$C5
    jsr VecSoundCommand
    lda a:OBJ_START_ARG,x
    bne SinkWhenStoodOn0D5D_at2
    jsr VecRecordObjectEvent
    jmp VecClearObject
SinkWhenStoodOn0D5D_at2:
    inc a:OBJ_STATE,x
    rts
SinkWhenStoodOn0D5D_at0:
    dec a
    bne SinkWhenStoodOn0D5D_at3
    bsr OthersInState2_0D5D
    bne SinkWhenStoodOn0D5D_at4
    inc a:OBJ_STATE,x
SinkWhenStoodOn0D5D_at4:
    jsr PassRecordFromState2_0D5D
    rts
SinkWhenStoodOn0D5D_at3:
    jsr PassRecordFromState2_0D5D
    lda #$12
    sta z:zTemp08
    lda #$15
    sta z:zTemp09
    jsr ContactWithPlayer
    dec a:OBJ_Y_LO,x
    lda a:OBJ_Y_LO,x
    clc
    adc #$08
    cmp a:OBJ_WORK_D,x
    bne SinkWhenStoodOn0D5D_at5
    lda #$01
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
SinkWhenStoodOn0D5D_at5:
    rts

; @routine 0D5D:2E3 StartFrameCHitUp0D5D
; @run 71:C2E3
; WHAT IT DOES: Starts the object with sprite set 6 and frame $0C (StartFrameC0D5D) and hit box Y offset $F0 (Stage
; 08's overlay).
; HOW: A call and a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartFrameCHitUp0D5D:
    jsr StartFrameC0D5D
    lda #$F0
    sta a:OBJ_HITBOX_DY,x
    rts

; @routine 0D5D:2EC RiseToY58AfterHits0D5D
; @run 71:C2EC
; WHAT IT DOES: In state 0 starts the object (StartFrameCHitUp0D5D); in state 1, once hit 5 times, plays sound $3A
; and steps on; then rises a pixel every fourth frame of zWaveCounter until Y $58, where it becomes kind $AF in
; state 0 (Stage 08's overlay).
; HOW: DEC tests of the state, a compare of OBJ_HIT_TAKEN, a TST of the counter, DEC of Y, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RiseToY58AfterHits0D5D:
    lda a:OBJ_STATE,x
    beq StartFrameCHitUp0D5D
    dec a
    bne RiseToY58AfterHits0D5D_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RiseToY58AfterHits0D5D_at1
    lda #$3A
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
RiseToY58AfterHits0D5D_at1:
    rts
RiseToY58AfterHits0D5D_at0:
    tst #$03,z:zWaveCounter
    bne RiseToY58AfterHits0D5D_at1
    lda a:OBJ_Y_LO,x
    cmp #$58
    beq RiseToY58AfterHits0D5D_at2
    dec a:OBJ_Y_LO,x
    rts
RiseToY58AfterHits0D5D_at2:
    lda #$AF
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 0D5D:320 SwitchPressedByStanding0D5D
; @run 71:C320
; WHAT IT DOES: A block $18 wide and $15 high. In state 0 starts with sprite set 6, frame $34, OBJ_WORK_E 8 and its
; Y plus 8 in OBJ_WORK_D, already at that Y in state 2 when its event (OBJ_EVENT_ID plus 1) is recorded; in state 1
; steps on once the player stands on it and the first three of RichterColumnHit are 0; in state 2 sinks a pixel a
; frame for nine frames, then steps on the state of the slot OBJ_START_ARG names, its own state, plays sound $C5
; and records the event (Stage 08's overlay).
; HOW: DEC tests of the state; LoadObjectRecord, BitOfIndex, ContactWithPlayer, SoundCommand, RecordEvent.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SwitchPressedByStanding0D5D:
    lda a:OBJ_STATE,x
    bne SwitchPressedByStanding0D5D_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_Y_LO,x
    clc
    adc #$08
    sta a:OBJ_WORK_D,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$34
    sta a:OBJ_FRAME,x
    lda #$08
    sta a:OBJ_WORK_E,x
    lda a:OBJ_EVENT_ID,x
    inc a
    jsr VecBitOfIndex
    and a:EventFlags,y
    beq SwitchPressedByStanding0D5D_at0
    lda a:OBJ_WORK_D,x
    sta a:OBJ_Y_LO,x
    inc a:OBJ_STATE,x
    inc a:OBJ_STATE,x
    rts
SwitchPressedByStanding0D5D_at0:
    dec a
    bne SwitchPressedByStanding0D5D_at1
    lda #$18
    sta z:zTemp08
    lda #$15
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc SwitchPressedByStanding0D5D_at2
    lda a:RichterColumnHit
    ora a:RichterColumnRow-4
    ora a:RichterColumnRow-3
    bne SwitchPressedByStanding0D5D_at2
    inc a:OBJ_STATE,x
SwitchPressedByStanding0D5D_at2:
    rts
SwitchPressedByStanding0D5D_at1:
    dec a
    bne SwitchPressedByStanding0D5D_at2
    dec a:OBJ_WORK_E,x
    bmi SwitchPressedByStanding0D5D_at3
    inc a:OBJ_Y_LO,x
    lda #$18
    sta z:zTemp08
    lda #$15
    sta z:zTemp09
    jmp ContactWithPlayer
SwitchPressedByStanding0D5D_at3:
    ldy a:OBJ_START_ARG,x
    beq SwitchPressedByStanding0D5D_at4
    lda a:OBJ_STATE,y
    inc a
    sta a:OBJ_STATE,y
SwitchPressedByStanding0D5D_at4:
    inc a:OBJ_STATE,x
    lda #$C5
    jsr VecSoundCommand
    lda a:OBJ_EVENT_ID,x
    inc a
    jmp VecRecordEvent

; @routine 0D5D:3FB FacePlayerInStates0D5D
; @run 71:C3FB
; WHAT IT DOES: In the states a table marks, turns the object to face the player (Stage 08's overlay).
; HOW: A table read by the state, SideOfPlayer's bit 0 into OBJ_FACING_LEFT.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FacePlayerInStates0D5DStates = $C40C ; nonzero for the states in which it turns
FacePlayerInStates0D5D:
    ldy a:OBJ_STATE,x
    lda a:FacePlayerInStates0D5DStates,y
    beq FacePlayerInStates0D5D_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FacePlayerInStates0D5D_at0:
    rts

; @routine 0D5D:47A StartWithKind38Below0D5D
; @run 71:C47A
; WHAT IT DOES: Starts the object facing the player with OBJ_WORK_D $40, and an object of kind $38 at its place in
; the slot below it; with OBJ_START_ARG 2 its sprite attributes are $87 (Stage 08's overlay).
; HOW: LoadObjectRecord, SideOfPlayer, ClearObjectInY, StartObjectAtPlace, a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWithKind38Below0D5D:
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$40
    sta a:OBJ_WORK_D,x
    txa
    tay
    dey
    jsr VecClearObjectInY
    lda #$38
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    cmp #$02
    bne StartWithKind38Below0D5D_at0
    lda #$87
    sta a:OBJ_SPRITE_ATTR,x
StartWithKind38Below0D5D_at0:
    rts

; @routine 0D5D:4C8 WalkBySpeedIndex0D5D
; @run 71:C4C8
; WHAT IT DOES: Plays the object's frame list and walks it at the X speed a table gives by OBJ_WORK_F, turning
; round at walls (BounceAtWalls with probe height 6) (Stage 08's overlay).
; HOW: StepObjectFrames, a word table read by twice OBJ_WORK_F, BounceAtWalls.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkBySpeedIndex0D5DSpeeds = $C4EC ; the X speed for each OBJ_WORK_F, a word each
WalkBySpeedIndex0D5D:
    lda #$C4
    sta z:zTemp11
    lda #$F0
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_WORK_F,x
    asl a
    tay
    lda a:WalkBySpeedIndex0D5DSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkBySpeedIndex0D5DSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$06
    sta z:SpriteFlipMask
    jsr BounceAtWalls
    rts

; @routine 0D5D:57F PickMoveByPattern0D5D
; @run 71:C57F
; WHAT IT DOES: With the object in the first 256 pixels across and the player less than $18 pixels away in Y, takes
; its substate from a 16-step pattern by OBJ_START_ARG and OBJ_WORK_I; otherwise state 3, or substate 3 every
; fourth count of OBJ_WORK_I when only the Y distance is too large (Stage 08's overlay).
; HOW: A test of the X high byte, DistanceYToPlayer, a table read by 16 times OBJ_START_ARG plus OBJ_WORK_I's low
; four bits.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
PickMoveByPattern0D5DSubstates = $C5BD ; 16 substates for each OBJ_START_ARG
PickMoveByPattern0D5D:
    lda a:OBJ_X_HI,x
    bne PickMoveByPattern0D5D_at0
    jsr VecDistanceYToPlayer
    cmp #$18
    bcc PickMoveByPattern0D5D_at1
    inc a:OBJ_WORK_I,x
    lda a:OBJ_WORK_I,x
    and #$03
    beq PickMoveByPattern0D5D_at2
PickMoveByPattern0D5D_at0:
    lda #$03
    sta a:OBJ_STATE,x
    rts
PickMoveByPattern0D5D_at2:
    lda #$03
    sta a:OBJ_SUBSTATE,x
    rts
PickMoveByPattern0D5D_at1:
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_WORK_I,x
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:PickMoveByPattern0D5DSubstates,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_I,x
    rts

; @routine 0D5D:600 RestartInState3_0D5D
; @run 71:C600
; WHAT IT DOES: Restarts the object's animation and puts it in state 3, substate 0 (Stage 08's overlay).
; HOW: Four stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RestartInState3_0D5D:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 0D5D:680 WalkByFacing0D5D
; @run 71:C680
; WHAT IT DOES: Plays the object's frame list, and when it ends restarts in state 3 (RestartInState3_0D5D);
; otherwise sets the X speed by its facing and, from the list's step 2 on, walks, turning round at walls
; (BounceAtWalls with probe height 6) (Stage 08's overlay).
; HOW: StepObjectFrames, a word table read by twice OBJ_FACING_LEFT, a compare of OBJ_ANIM_STEP, BounceAtWalls.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkByFacing0D5DSpeeds = $C6C9 ; the X speed facing right, then left, a word each
WalkByFacing0D5D:
    lda #$B0
    sta z:zTemp10
    lda #$C6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne WalkByFacing0D5D_at0
    jmp RestartInState3_0D5D
WalkByFacing0D5D_at0:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:WalkByFacing0D5DSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkByFacing0D5DSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_ANIM_STEP,x
    cmp #$02
    bcc WalkByFacing0D5D_at1
    lda #$06
    sta z:SpriteFlipMask
    jsr BounceAtWalls
WalkByFacing0D5D_at1:
    rts

; @routine 0D5D:724 ShootKind9COnCount0D5D
; @run 71:C724
; WHAT IT DOES: When OBJ_WORK_D matches one of eight counts, starts an object of kind $9C at the object's place in
; a free slot from $30, plays sound $15 if on the first screen, and moves it once by the X and Y speeds for that
; count (Stage 08's overlay).
; HOW: A search of the table, FindFreeSlotFrom30, StartObjectAtPlace, SoundIfOnFirstScreen, a pair read by twice
; the match, ApplyVelocity on the new slot.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ShootKind9COnCount0D5DCounts = $C75E ; the eight counts at which it shoots
ShootKind9COnCount0D5DSpeeds = $C766 ; X speed high, Y speed high for each count
ShootKind9COnCount0D5D:
    cly
ShootKind9COnCount0D5D_at1:
    lda a:OBJ_WORK_D,x
    cmp a:ShootKind9COnCount0D5DCounts,y
    beq ShootKind9COnCount0D5D_at0
    iny
    cpy #$08
    bcc ShootKind9COnCount0D5D_at1
    rts
ShootKind9COnCount0D5D_at0:
    stx z:zClimbStepBefore
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc ShootKind9COnCount0D5D_at2
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$15
    jsr VecSoundIfOnFirstScreen
    lda z:zBurstCount
    asl a
    tax
    lda a:ShootKind9COnCount0D5DSpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:ShootKind9COnCount0D5DSpeeds+1,x
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zClimbStepBefore
ShootKind9COnCount0D5D_at2:
    rts

; @routine 0D5D:7F5 MoveByDirectionIndex0D5D
; @run 71:C7F5
; WHAT IT DOES: Sets the object's X and Y speeds from a pair of bytes by OBJ_WORK_E, turning the X speed round when
; it faces left, and moves it (Stage 08's overlay).
; HOW: A table read by twice OBJ_WORK_E, NegateVelocityX, ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveByDirectionIndex0D5DSpeeds = $C811 ; X speed high, Y speed high for each OBJ_WORK_E
MoveByDirectionIndex0D5D:
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:MoveByDirectionIndex0D5DSpeeds,y
    sta a:OBJ_VX_HI,x
    lda a:MoveByDirectionIndex0D5DSpeeds+1,y
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT,x
    beq MoveByDirectionIndex0D5D_at0
    jsr VecNegateVelocityX
MoveByDirectionIndex0D5D_at0:
    jmp VecApplyVelocity

; @routine 0D5E:017 FlyListOrFall0D5E
; @run 71:C817
; WHAT IT DOES: Plays the object's frame list and moves it by its speed; with OBJ_WORK_E 2 it gains $1C of Y speed
; a frame, otherwise it counts OBJ_WORK_F down and steps its state on at zero (Stage 08's overlay).
; HOW: StepObjectFrames, ApplyVelocity, a compare, a 16-bit add or DEC.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FlyListOrFall0D5E:
    lda #$C8
    sta z:zTemp11
    lda #$47
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_WORK_E,x
    cmp #$02
    bne FlyListOrFall0D5E_at0
    lda a:OBJ_VY_LO,x
    clc
    adc #$1C
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    rts
FlyListOrFall0D5E_at0:
    dec a:OBJ_WORK_F,x
    bne FlyListOrFall0D5E_at1
    inc a:OBJ_STATE,x
FlyListOrFall0D5E_at1:
    rts

; @routine 0D5E:058 SpeedXToTarget0D5E
; @run 71:C858
; WHAT IT DOES: Plays the object's frame list, moves it, and adds a step to its X speed until the speed reaches a
; target, when the state steps on; both come from a record by OBJ_WORK_D (Stage 08's overlay).
; HOW: StepObjectFrames, ApplyVelocity, a 16-bit add and compare of four-byte records by four times OBJ_WORK_D.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SpeedXToTarget0D5ERecords = $C890 ; the X speed's step, then its target, a word each per record
SpeedXToTarget0D5E:
    lda #$C8
    sta z:zTemp11
    lda #$47
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:SpeedXToTarget0D5ERecords,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:SpeedXToTarget0D5ERecords+1,y
    sta a:OBJ_VX_HI,x
    cmp a:SpeedXToTarget0D5ERecords+3,y
    bne SpeedXToTarget0D5E_at0
    lda a:OBJ_VX_LO,x
    cmp a:SpeedXToTarget0D5ERecords+2,y
    bne SpeedXToTarget0D5E_at0
    inc a:OBJ_STATE,x
SpeedXToTarget0D5E_at0:
    rts

; @routine 0D5E:098 FlyWithList0D5E
; @run 71:C898
; WHAT IT DOES: Plays the object's frame list and moves it by its speed (Stage 08's overlay).
; HOW: StepObjectFrames, then ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlyWithList0D5E:
    lda #$C8
    sta z:zTemp11
    lda #$47
    sta z:zTemp10
    jsr VecStepObjectFrames
    jmp VecApplyVelocity

; @routine 0D5E:12C ActByStateUnlessFlagged0D5E
; @run 71:C92C
; WHAT IT DOES: Sends the object to state 6 when flagged (EnterState6IfFlaggedB), then runs the routine for its
; state (Stage 08's overlay).
; HOW: A call, then a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByStateUnlessFlagged0D5EJumps = $C941 ; the routine for each state, a word each
ActByStateUnlessFlagged0D5E:
    jsr EnterState6IfFlaggedB
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByStateUnlessFlagged0D5EJumps,y
    sta z:zTemp00
    lda a:ActByStateUnlessFlagged0D5EJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5E:2E3 JumpAfterList0D5E
; @run 71:CAE3
; WHAT IT DOES: Faces the player and plays the list at offset 4 (StepFramesFacingPlayerE); when it ends, gives the
; object the X speed a table picks by OBJ_WORK_E and Y speed -3, restarts its animation, sets OBJ_WORK_D 1 and
; steps the substate on (Stage 08's overlay).
; HOW: A call, then a word table read by twice OBJ_WORK_E and stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
JumpAfterList0D5ESpeeds = $CB12 ; the X speed for each OBJ_WORK_E, a word each
JumpAfterList0D5E:
    ldy #$04
    jsr StepFramesFacingPlayerE
    bne JumpAfterList0D5E_at0
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:JumpAfterList0D5ESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:JumpAfterList0D5ESpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$01
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
JumpAfterList0D5E_at0:
    rts

; @routine 0D5E:316 LandThenSubstate0D5E
; @run 71:CB16
; WHAT IT DOES: Falls and lands with probe height $0A; on landing steps the object's substate on (Stage 08's
; overlay).
; HOW: StopOnLanding, then INC on the carry.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandThenSubstate0D5E:
    lda #$0A
    jsr VecStopOnLanding
    bcc LandThenSubstate0D5E_at0
    inc a:OBJ_SUBSTATE,x
LandThenSubstate0D5E_at0:
    rts

; @routine 0D5E:321 ListThenState3_0D5E
; @run 71:CB21
; WHAT IT DOES: Faces the player and plays the list at offset 6 (StepFramesFacingPlayerE); when it ends, restarts
; the animation in state 3, substate 0 (Stage 08's overlay).
; HOW: A call, then four stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ListThenState3_0D5E:
    ldy #$06
    jsr StepFramesFacingPlayerE
    bne ListThenState3_0D5E_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
ListThenState3_0D5E_at0:
    rts

; @routine 0D5E:337 BurstIntoSixKind42_0D5E
; @run 71:CB37
; WHAT IT DOES: Starts up to six objects of kind $42 at the object's place in free slots from $30, each facing as
; it does with OBJ_WORK_D, OBJ_WORK_E and X and Y speeds from its own record; then clears the object (Stage 08's
; overlay).
; HOW: A loop of FindFreeSlotFrom30 and StartObjectAtPlace with the count in zHitSlot and X in zRichterObject;
; eight-byte records by eight times the count; ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BurstIntoSixKind42_0D5ERecords = $CB82 ; OBJ_WORK_D, OBJ_WORK_E, X speed high, low, Y speed high, low, two unused, per piece
BurstIntoSixKind42_0D5E:
    cly
BurstIntoSixKind42_0D5E_at1:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc BurstIntoSixKind42_0D5E_at0
    lda #$42
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    stx z:zClimbStepBefore
    lda z:zBurstCount
    asl a
    asl a
    asl a
    tax
    lda a:BurstIntoSixKind42_0D5ERecords,x
    sta a:OBJ_WORK_D,y
    lda a:BurstIntoSixKind42_0D5ERecords+1,x
    sta a:OBJ_WORK_E,y
    lda a:BurstIntoSixKind42_0D5ERecords+2,x
    sta a:OBJ_VX_HI,y
    lda a:BurstIntoSixKind42_0D5ERecords+3,x
    sta a:OBJ_VX_LO,y
    lda a:BurstIntoSixKind42_0D5ERecords+4,x
    sta a:OBJ_VY_HI,y
    lda a:BurstIntoSixKind42_0D5ERecords+5,x
    sta a:OBJ_VY_LO,y
    ldx z:zClimbStepBefore
    ldy z:zBurstCount
    iny
    cpy #$06
    bcc BurstIntoSixKind42_0D5E_at1
BurstIntoSixKind42_0D5E_at0:
    jmp VecClearObject

; @routine 0D5E:41C FlyArcThenKind9B0D5E
; @run 71:CC1C
; WHAT IT DOES: In state 0 starts the object, turning its X speed round when it is left of the player; then counts
; OBJ_WORK_E down to become kind $9B in state 0, and meanwhile plays the frame list OBJ_WORK_D picks and moves
; under gravity (Stage 08's overlay).
; HOW: LoadObjectRecord, SideOfPlayer, NegateVelocityX; DEC, a word table read, StepObjectFrames,
; ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlyArcThenKind9B0D5ELists = $CC55 ; the frame list's address for each OBJ_WORK_D, a word each
FlyArcThenKind9B0D5E:
    lda a:OBJ_STATE,x
    bne FlyArcThenKind9B0D5E_at0
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    beq FlyArcThenKind9B0D5E_at1
    jsr VecNegateVelocityX
FlyArcThenKind9B0D5E_at1:
    rts
FlyArcThenKind9B0D5E_at0:
    dec a:OBJ_WORK_E,x
    bne FlyArcThenKind9B0D5E_at2
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
FlyArcThenKind9B0D5E_at2:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:FlyArcThenKind9B0D5ELists,y
    sta z:zTemp10
    lda a:FlyArcThenKind9B0D5ELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0D5E:481 State5WhenStruck0D5E
; @run 71:CC81
; WHAT IT DOES: When a player attack hits this frame (HitAttackFlags bits 0 and 1) or the object has been hit 5
; times, and it is in a state below 5, plays sound $16 if on the first screen, turns its hit box off and restarts
; it in state 5, substate 0 with OBJ_WORK_C 0 (Stage 08's overlay).
; HOW: Two tests, a compare of the state, SoundIfOnFirstScreen, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
State5WhenStruck0D5E:
    lda a:HitAttackFlags
    and #$03
    bne State5WhenStruck0D5E_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc State5WhenStruck0D5E_at1
State5WhenStruck0D5E_at0:
    lda a:OBJ_STATE,x
    cmp #$05
    bcs State5WhenStruck0D5E_at1
    lda #$16
    jsr VecSoundIfOnFirstScreen
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_WORK_C,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State5WhenStruck0D5E_at1:
    rts

; @routine 0D5E:4C9 WakeWhenPlayerNear0D5E
; @run 71:CCC9
; WHAT IT DOES: Falls and lands with probe height 8; once the player is within $60 pixels along X and $20 along Y,
; steps the state on and, unless OBJ_START_ARG bit 4 is set, plays sound $17 if on the first screen (Stage 08's
; overlay).
; HOW: StopOnLanding, DistanceXToPlayer and DistanceYToPlayer with compares, SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
WakeWhenPlayerNear0D5E:
    lda #$08
    jsr VecStopOnLanding
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs WakeWhenPlayerNear0D5E_at0
    jsr VecDistanceYToPlayer
    cmp #$20
    bcs WakeWhenPlayerNear0D5E_at0
    inc a:OBJ_STATE,x
    lda a:OBJ_START_ARG,x
    and #$10
    bne WakeWhenPlayerNear0D5E_at0
    lda #$17
    jsr VecSoundIfOnFirstScreen
WakeWhenPlayerNear0D5E_at0:
    rts

; @routine 0D5E:560 WalkOnFloorOrFall0D5E
; @run 71:CD60
; WHAT IT DOES: With floor under the object plays its frame list and, when the frame changes, sets the X speed from
; a table by OBJ_START_ARG's low four bits and OBJ_WORK_F and walks, turning round at walls; with no floor stops
; along X and falls with probe height 8 (Stage 08's overlay).
; HOW: ProbeTerrain request 9, StepObjectFrames, a word table read by four times the argument plus twice
; OBJ_WORK_F, BounceAtWalls; or StopOnLanding.
; WAITS: no.
; Entry: X the object; Y the probe's column. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkOnFloorOrFall0D5ESpeeds = $CDAB ; two X speeds, a word each, for each OBJ_START_ARG
WalkOnFloorOrFall0D5E:
    lda #$08
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    and #$01
    beq WalkOnFloorOrFall0D5E_at0
    lda #$B3
    sta z:zTemp10
    lda #$CD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc WalkOnFloorOrFall0D5E_at1
    lda a:OBJ_START_ARG,x
    and #$0F
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:WalkOnFloorOrFall0D5ESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkOnFloorOrFall0D5ESpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$08
    sta z:SpriteFlipMask
    jsr BounceAtWalls
WalkOnFloorOrFall0D5E_at1:
    rts
WalkOnFloorOrFall0D5E_at0:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$08
    jsr VecStopOnLanding
    rts

; @routine 0D5E:601 ListThenArm0D5E
; @run 71:CE01
; WHAT IT DOES: Sets the object's hit box from its frame (SetHitboxFromFrameRowsB). In substate 0 plays the frame
; list OBJ_START_ARG's low four bits pick; when it ends, unless a player attack with HitAttackFlags bit 0 hits this
; frame, plays sound $17 if on the first screen, turns the hit box on (2) and steps the substate on. Later
; substates run ReverseListWhenStruck0D5E (Stage 08's overlay).
; HOW: A call, a word table read, StepObjectFrames and tests.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ListThenArm0D5ELists = $CE34 ; the frame list's address for each OBJ_START_ARG, a word each
ListThenArm0D5E:
    jsr SetHitboxFromFrameRowsB
    lda a:OBJ_SUBSTATE,x
    bne ReverseListWhenStruck0D5E
    lda a:OBJ_START_ARG,x
    and #$0F
    asl a
    tay
    lda a:ListThenArm0D5ELists,y
    sta z:zTemp10
    lda a:ListThenArm0D5ELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ReturnOnly0D5E
    lda a:HitAttackFlags
    and #$01
    bne ReturnOnly0D5E
    lda #$17
    jsr VecSoundIfOnFirstScreen
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_SUBSTATE,x

; @routine 0D5E:633 ReturnOnly0D5E
; @run 71:CE33
; WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
; HOW: An RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
ReturnOnly0D5E:
    rts

; @routine 0D5E:66A ReverseListWhenStruck0D5E
; @run 71:CE6A
; WHAT IT DOES: Plays the frame list OBJ_START_ARG's low four bits pick; when it ends runs
; EnterState3WithTallHitbox. When a player attack hits this frame (HitAttackFlags bits 0 and 1) or it has been hit
; 5 times, plays sound $16 if on the first screen, sets its list step to $0A less the step, turns the hit box off
; and goes back to substate 0 (Stage 08's overlay).
; HOW: A word table read, StepObjectFrames, tests, SoundIfOnFirstScreen, a subtract and stores.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
ReverseListWhenStruck0D5ELists = $CE38 ; the frame list's address for each OBJ_START_ARG, a word each
ReverseListWhenStruck0D5E:
    lda a:OBJ_START_ARG,x
    and #$0F
    asl a
    tay
    lda a:ReverseListWhenStruck0D5ELists,y
    sta z:zTemp10
    lda a:ReverseListWhenStruck0D5ELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq EnterState3WithTallHitbox
    lda a:HitAttackFlags
    and #$03
    bne ReverseListWhenStruck0D5E_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc ReturnOnly0D5E
ReverseListWhenStruck0D5E_at0:
    lda #$16
    jsr VecSoundIfOnFirstScreen
    lda #$0A
    sec
    sbc a:OBJ_ANIM_STEP,x
    sta a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 0D5E:77E StartOrRoamNearPlayer0D5E
; @run 71:CF7E
; WHAT IT DOES: In state 0 starts the object, at X $0080, Y -$40 unless OBJ_START_ARG is negative, with OBJ_WORK_D
; $40; clears $40 bytes of four speed buffers from a place by OBJ_START_ARG, and starts seven objects of kind $50
; at its place in the slots after it, each with its index 0 to 6 in OBJ_WORK_D and the object's slot in OBJ_WORK_E.
; Later it takes the frame $80 to $87 of its angle OBJ_WORK_D, moves at speed $100 at that angle, and turns toward
; a point near the player (TurnTowardPlayerOffset0D5F) chosen at random from eight for a random $40 to $5F frames
; (Stage 08's overlay).
; HOW: LoadObjectRecord, a clearing loop, a loop of ClearObjectInY and StartObjectAtPlace; NextRandom twice,
; SetVelocityFromAngle, ApplyVelocity, then falls into the next routine.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StartOrRoamNearPlayer0D5ESpeedXHi = $B2F4 ; a buffer of X speed high bytes
StartOrRoamNearPlayer0D5ESpeedXLo = $B374 ; a buffer of X speed low bytes
StartOrRoamNearPlayer0D5ESpeedYHi = $B3F4 ; a buffer of Y speed high bytes
StartOrRoamNearPlayer0D5ESpeedYLo = $B474 ; a buffer of Y speed low bytes
StartOrRoamNearPlayer0D5EBases = $CF7C ; the buffers' index, plus $3F, for each OBJ_START_ARG
StartOrRoamNearPlayer0D5E:
    lda a:OBJ_STATE,x
    bne StartOrRoamNearPlayer0D5E_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    bmi StartOrRoamNearPlayer0D5E_at1
    stz a:OBJ_X_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$C0
    sta a:OBJ_Y_LO,x
StartOrRoamNearPlayer0D5E_at1:
    lda a:OBJ_START_ARG,x
    and #$7F
    sta a:OBJ_START_ARG,x
    lda #$40
    sta a:OBJ_WORK_D,x
    lda #$40
    sta z:zTemp00
    ldy a:OBJ_START_ARG,x
    lda a:StartOrRoamNearPlayer0D5EBases,y
    sec
    sbc #$3F
    tay
    cla
StartOrRoamNearPlayer0D5E_at2:
    sta a:StartOrRoamNearPlayer0D5ESpeedXHi,y
    sta a:StartOrRoamNearPlayer0D5ESpeedXLo,y
    sta a:StartOrRoamNearPlayer0D5ESpeedYHi,y
    sta a:StartOrRoamNearPlayer0D5ESpeedYLo,y
    iny
    dec z:zTemp00
    bne StartOrRoamNearPlayer0D5E_at2
    txa
    tay
    iny
    lda #$07
    sta z:zTemp00
    stz z:zTemp01
StartOrRoamNearPlayer0D5E_at3:
    jsr VecClearObjectInY
    lda #$50
    jsr VecStartObjectAtPlace
    lda z:zTemp01
    sta a:OBJ_WORK_D,y
    txa
    sta a:OBJ_WORK_E,y
    iny
    inc z:zTemp01
    dec z:zTemp00
    bne StartOrRoamNearPlayer0D5E_at3
    rts
StartOrRoamNearPlayer0D5E_at0:
    lda a:OBJ_WORK_D,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc #$80
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_H,x
    bne StartOrRoamNearPlayer0D5E_at4
    jsr VecNextRandom
    and #$1F
    clc
    adc #$40
    sta a:OBJ_WORK_H,x
    jsr VecNextRandom
    and #$07
    sta a:OBJ_WORK_G,x
StartOrRoamNearPlayer0D5E_at4:
    dec a:OBJ_WORK_H,x
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_G,x
    asl a
    asl a

; @routine 0D5F:02B TurnTowardPlayerOffset0D5F
; @run 71:D02B
; WHAT IT DOES: Turns the object's angle OBJ_WORK_D by 1 toward the player's place plus an offset from a table
; (Stage 08's overlay).
; HOW: Two 16-bit adds into zTemp08-0B, AngleToPoint, TurnAngleToward.
; WAITS: no.
; Entry: X the object; A four times the offset's index. Exit: A, X, Y and the flags as its last instruction or call
; left them.
TurnTowardPlayerOffset0D5FOffsets = $D063 ; an X offset and a Y offset, a word each, per entry
TurnTowardPlayerOffset0D5F:
    tay
    lda a:OBJ_X_LO
    clc
    adc a:TurnTowardPlayerOffset0D5FOffsets,y
    sta z:zTemp08
    lda a:OBJ_X_HI
    adc a:TurnTowardPlayerOffset0D5FOffsets+1,y
    sta z:zTemp09
    lda a:OBJ_Y_LO
    clc
    adc a:TurnTowardPlayerOffset0D5FOffsets+2,y
    sta z:zTemp0A
    lda a:OBJ_Y_HI
    adc a:TurnTowardPlayerOffset0D5FOffsets+3,y
    sta z:zTemp0B
    jsr VecAngleToPoint
    sta z:zTemp02
    lda #$01
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    rts

; @routine 0D5F:09B MoveByOwnerBuffers0D5F
; @run 71:D09B
; WHAT IT DOES: While the slot in OBJ_WORK_E holds kind $4F, moves the object by the X and Y speeds kept in four
; buffers at a place by that slot's OBJ_START_ARG and its own OBJ_WORK_D; otherwise sets OBJ_WORK_F to four times
; OBJ_WORK_D plus $10, turns its hit box off and steps its state on (Stage 08's overlay).
; HOW: A kind compare, table reads for the index, four buffer reads, ApplyVelocity.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
MoveByOwnerBuffers0D5FSpeedXHi = $B2F4 ; a buffer of X speed high bytes
MoveByOwnerBuffers0D5FSpeedXLo = $B374 ; a buffer of X speed low bytes
MoveByOwnerBuffers0D5FSpeedYHi = $B3F4 ; a buffer of Y speed high bytes
MoveByOwnerBuffers0D5FSpeedYLo = $B474 ; a buffer of Y speed low bytes
MoveByOwnerBuffers0D5FBases = $CF7C ; the buffers' index, plus $3F, for each OBJ_START_ARG
MoveByOwnerBuffers0D5FDelays = $D0E8 ; the buffer offset for each OBJ_WORK_D
MoveByOwnerBuffers0D5F:
    ldy a:OBJ_WORK_E,x
    lda a:OBJ_KIND,y
    cmp #$4F
    beq MoveByOwnerBuffers0D5F_at0
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    clc
    adc #$10
    sta a:OBJ_WORK_F,x
    stz a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
    rts
MoveByOwnerBuffers0D5F_at0:
    lda a:OBJ_START_ARG,y
    tay
    lda a:MoveByOwnerBuffers0D5FBases,y
    sec
    sbc #$3F
    sta z:zTemp00
    ldy a:OBJ_WORK_D,x
    lda a:MoveByOwnerBuffers0D5FDelays,y
    clc
    adc z:zTemp00
    tay
    lda a:MoveByOwnerBuffers0D5FSpeedXHi,y
    sta a:OBJ_VX_HI,x
    lda a:MoveByOwnerBuffers0D5FSpeedXLo,y
    sta a:OBJ_VX_LO,x
    lda a:MoveByOwnerBuffers0D5FSpeedYHi,y
    sta a:OBJ_VY_HI,x
    lda a:MoveByOwnerBuffers0D5FSpeedYLo,y
    sta a:OBJ_VY_LO,x
    jmp VecApplyVelocity

; @routine 0D5F:0EF CountThenKind9B0D5F
; @run 71:D0EF
; WHAT IT DOES: Counts OBJ_WORK_F down; at zero plays sound $11 and turns the object into kind $9B in state 0
; (Stage 08's overlay).
; HOW: DEC, SoundCommand, two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountThenKind9B0D5F:
    dec a:OBJ_WORK_F,x
    bne CountThenKind9B0D5F_at0
    lda #$11
    jsr VecSoundCommand
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
CountThenKind9B0D5F_at0:
    rts

; @routine 0D5F:102 SpawnKind4FOnTimer0D5F
; @run 71:D102
; WHAT IT DOES: In state 0 starts the object; then counts the 16-bit OBJ_WORK_D-E down. At zero, starts an object
; of kind $4F at its place in slot $20, OBJ_START_ARG 0, when slots $20 and $27 are empty, or with OBJ_START_ARG 0
; in slot $28, argument 1, when $28 and $2F are; then waits again by a table. With no group free it waits a shorter
; time by another table (Stage 08's overlay).
; HOW: A 16-bit DEC, ORs of two kinds, ClearObjectInY and StartObjectAtPlace, word tables by twice OBJ_START_ARG.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SpawnKind4FOnTimer0D5FWaits = $D172 ; the wait after a start, OBJ_WORK_E then OBJ_WORK_D, per OBJ_START_ARG
SpawnKind4FOnTimer0D5FRetries = $D176 ; the wait when no group of slots is free, the same layout
SpawnKind4FOnTimer0D5F:
    lda a:OBJ_STATE,x
    bne SpawnKind4FOnTimer0D5F_at0
    jmp VecLoadObjectRecord
SpawnKind4FOnTimer0D5F_at0:
    lda a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    beq SpawnKind4FOnTimer0D5F_at1
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    rts
SpawnKind4FOnTimer0D5F_at1:
    stz z:zTemp00
    ldy #$20
    lda a:OBJ_KIND,y
    ora a:OBJ_KIND+7,y
    beq SpawnKind4FOnTimer0D5F_at2
    lda a:OBJ_START_ARG,x
    bne SpawnKind4FOnTimer0D5F_at3
    inc z:zTemp00
    ldy #$28
    lda a:OBJ_KIND,y
    ora a:OBJ_KIND+7,y
    beq SpawnKind4FOnTimer0D5F_at2
SpawnKind4FOnTimer0D5F_at3:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:SpawnKind4FOnTimer0D5FRetries,y
    sta a:OBJ_WORK_E,x
    lda a:SpawnKind4FOnTimer0D5FRetries+1,y
    sta a:OBJ_WORK_D,x
    rts
SpawnKind4FOnTimer0D5F_at2:
    jsr VecClearObjectInY
    lda #$4F
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:SpawnKind4FOnTimer0D5FWaits,y
    sta a:OBJ_WORK_E,x
    lda a:SpawnKind4FOnTimer0D5FWaits+1,y
    sta a:OBJ_WORK_D,x
    rts

; @routine 0D5F:1FD FallWithLowerProbe0D5F
; @run 71:D1FD
; WHAT IT DOES: Outside state 0, and while its Y high byte is 0, the object falls under gravity and stops on the
; floor (probe request 9, height $0C) with its Y taken $10 times OBJ_START_ARG lower for the probe (Stage 08's
; overlay).
; HOW: A 16-bit add to Y, ApplyVelocityY, ApplyGravity, ProbeTerrain, the same subtract back.
; WAITS: no.
; Entry: X the object; Y the probe's column. Exit: A, X, Y and the flags as its last instruction or call left them.
FallWithLowerProbe0D5F:
    lda a:OBJ_STATE,x
    beq FallWithLowerProbe0D5F_at0
    lda a:OBJ_Y_HI,x
    bne FallWithLowerProbe0D5F_at0
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_Y_LO,x
    clc
    adc z:zTemp00
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    adc #$00
    sta a:OBJ_Y_HI,x
    jsr VecApplyVelocityY
    jsr VecApplyGravity
    lda #$0C
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    and #$01
    beq FallWithLowerProbe0D5F_at1
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
FallWithLowerProbe0D5F_at1:
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp00
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_Y_HI,x
FallWithLowerProbe0D5F_at0:
    rts

; @routine 0D5F:2CB ShiftOnceByArg0D5F
; @run 71:D2CB
; WHAT IT DOES: Moves the object once by an X step a table gives by OBJ_START_ARG bit 0 and up by 16 times
; OBJ_START_ARG, then zeroes those speeds (Stage 08's overlay).
; HOW: A table read, a negated shift, ApplyVelocity, two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ShiftOnceByArg0D5FSteps = $D2EE ; the X speed's high byte for OBJ_START_ARG bit 0 clear, then set
ShiftOnceByArg0D5F:
    lda a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:ShiftOnceByArg0D5FSteps,y
    sta a:OBJ_VX_HI,x
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    eor #$FF
    inc a
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 0D5F:2F0 RandomWaitDE0D5F
; @run 71:D2F0
; WHAT IT DOES: Sets OBJ_WORK_E and OBJ_WORK_D from one of four words chosen at random (Stage 08's overlay).
; HOW: NextRandom's low two bits index a word table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RandomWaitDE0D5FWords = $D304 ; four words: OBJ_WORK_E, then OBJ_WORK_D
RandomWaitDE0D5F:
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:RandomWaitDE0D5FWords,y
    sta a:OBJ_WORK_E,x
    lda a:RandomWaitDE0D5FWords+1,y
    sta a:OBJ_WORK_D,x
    rts

; @routine 0D5F:30C RandomWordFG0D5F
; @run 71:D30C
; WHAT IT DOES: Sets OBJ_WORK_G and OBJ_WORK_F from one of four words chosen at random (Stage 08's overlay).
; HOW: NextRandom's low two bits index a word table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RandomWordFG0D5FWords = $D320 ; four words: OBJ_WORK_G, then OBJ_WORK_F
RandomWordFG0D5F:
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:RandomWordFG0D5FWords,y
    sta a:OBJ_WORK_G,x
    lda a:RandomWordFG0D5FWords+1,y
    sta a:OBJ_WORK_F,x
    rts

; @routine 0D5F:389 AlternateListThenState1_0D5F
; @run 71:D389
; WHAT IT DOES: Plays one of two frame lists by OBJ_WORK_H bit 0; when it ends, steps OBJ_WORK_H on and puts the
; object in state 1 (Stage 08's overlay).
; HOW: A word table read, StepObjectFrames, INC and a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AlternateListThenState1_0D5FLists = $D3A8 ; the two frame lists' addresses, a word each
AlternateListThenState1_0D5F:
    lda a:OBJ_WORK_H,x
    and #$01
    asl a
    tay
    lda a:AlternateListThenState1_0D5FLists,y
    sta z:zTemp10
    lda a:AlternateListThenState1_0D5FLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne AlternateListThenState1_0D5F_at0
    inc a:OBJ_WORK_H,x
    lda #$01
    sta a:OBJ_STATE,x
AlternateListThenState1_0D5F_at0:
    rts

; @routine 0D5F:3D8 PickMoveWhenFacing0D5F
; @run 71:D3D8
; WHAT IT DOES: When the object's facing, flipped by OBJ_WORK_H bit 0, differs from its side of the player, goes to
; state 1; otherwise picks OBJ_WORK_I from 16 at random, OBJ_WORK_K from four for that choice at random, OBJ_WORK_J
; by OBJ_WORK_H bit 0, and steps the substate on (Stage 08's overlay).
; HOW: An EOR and SideOfPlayer, NextRandom twice and three table reads.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
PickMoveWhenFacing0D5FChoices = $D420 ; 16 values of OBJ_WORK_I
PickMoveWhenFacing0D5FWorkK = $D430 ; four values of OBJ_WORK_K for each OBJ_WORK_I
PickMoveWhenFacing0D5FWorkJ = $D43C ; OBJ_WORK_J for OBJ_WORK_H bit 0 clear, then set
PickMoveWhenFacing0D5F:
    lda a:OBJ_FACING_LEFT,x
    eor a:OBJ_WORK_H,x
    and #$01
    sta z:zTemp08
    jsr VecSideOfPlayer
    and #$01
    cmp z:zTemp08
    beq PickMoveWhenFacing0D5F_at0
    lda #$01
    sta a:OBJ_STATE,x
    rts
PickMoveWhenFacing0D5F_at0:
    jsr VecNextRandom
    and #$0F
    tay
    lda a:PickMoveWhenFacing0D5FChoices,y
    sta a:OBJ_WORK_I,x
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    and #$03
    clc
    adc z:zTemp00
    tay
    lda a:PickMoveWhenFacing0D5FWorkK,y
    sta a:OBJ_WORK_K,x
    lda a:OBJ_WORK_H,x
    and #$01
    tay
    lda a:PickMoveWhenFacing0D5FWorkJ,y
    sta a:OBJ_WORK_J,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0D5F:43E StartChildByWorkI0D5F
; @run 71:D43E
; WHAT IT DOES: Starts an object of the kind a table gives by OBJ_WORK_I at the object's place in a free slot from
; $30, with OBJ_WORK_I in its OBJ_WORK_D, the object's facing and its slot in OBJ_WORK_F, and X speed $F8, or $EE
; facing the other way when OBJ_WORK_J is set; steps the substate on. With no free slot runs BackToState1Frame0D5F
; (Stage 08's overlay).
; HOW: FindFreeSlotFrom30, a table read, StartObjectAtPlace, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartChildByWorkI0D5FKinds = $D480 ; the kind for each OBJ_WORK_I
StartChildByWorkI0D5F:
    jsr VecFindFreeSlotFrom30
    bcs StartChildByWorkI0D5F_at0
    jmp BackToState1Frame0D5F
StartChildByWorkI0D5F_at0:
    sty z:zBurstCount
    ldy a:OBJ_WORK_I,x
    lda a:StartChildByWorkI0D5FKinds,y
    ldy z:zBurstCount
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_I,x
    sta a:OBJ_WORK_D,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    txa
    sta a:OBJ_WORK_F,y
    lda a:OBJ_WORK_J,x
    bne StartChildByWorkI0D5F_at1
    lda #$F8
    sta a:OBJ_VX_HI,y
    bra StartChildByWorkI0D5F_at2
StartChildByWorkI0D5F_at1:
    lda #$EE
    sta a:OBJ_VX_HI,y
    lda a:OBJ_FACING_LEFT,y
    eor #$01
    sta a:OBJ_FACING_LEFT,y
StartChildByWorkI0D5F_at2:
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0D5F:483 PlayChosenList0D5F
; @run 71:D483
; WHAT IT DOES: Plays the frame list at twice OBJ_WORK_I plus OBJ_WORK_J in a table; when it ends steps the
; substate on (Stage 08's overlay).
; HOW: A word table read, StepObjectFrames, INC.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
PlayChosenList0D5FLists = $D49F ; frame lists' addresses, a word each
PlayChosenList0D5F:
    lda a:OBJ_WORK_I,x
    asl a
    clc
    adc a:OBJ_WORK_J,x
    tay
    lda a:PlayChosenList0D5FLists,y
    sta z:zTemp10
    lda a:PlayChosenList0D5FLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne PlayChosenList0D5F_at0
    inc a:OBJ_SUBSTATE,x
PlayChosenList0D5F_at0:
    rts

; @routine 0D5F:4D0 BackToState1Frame0D5F
; @run 71:D4D0
; WHAT IT DOES: Puts the object in state 1 with frame $8A, or $8D when OBJ_WORK_J is set (Stage 08's overlay).
; HOW: Stores and a test.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BackToState1Frame0D5F:
    lda #$01
    sta a:OBJ_STATE,x
    lda #$8A
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_J,x
    beq BackToState1Frame0D5F_at0
    lda #$8D
    sta a:OBJ_FRAME,x
BackToState1Frame0D5F_at0:
    rts

; @routine 0D5F:4E5 RenumberKind52ThenClear0D5F
; @run 71:D4E5
; WHAT IT DOES: Plays one of two frame lists by OBJ_WORK_H bit 0; when it ends, numbers the objects of kind $52 in
; a state below 4 from slot OBJ_WORK_M to OBJ_WORK_N in their OBJ_START_ARG, gives each the first one's slot in
; OBJ_WORK_M, and clears the object (Stage 08's overlay).
; HOW: A word table read, StepObjectFrames, a loop over the slots with the count in zTemp00, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RenumberKind52ThenClear0D5FLists = $D530 ; the two frame lists' addresses, a word each
RenumberKind52ThenClear0D5F:
    lda a:OBJ_WORK_H,x
    and #$01
    asl a
    tay
    lda a:RenumberKind52ThenClear0D5FLists,y
    sta z:zTemp10
    lda a:RenumberKind52ThenClear0D5FLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne RenumberKind52ThenClear0D5F_at0
    ldy a:OBJ_WORK_M,x
    stz z:zTemp00
    lda a:OBJ_WORK_N,x
    sta z:zTemp01
    inc z:zTemp01
RenumberKind52ThenClear0D5F_at3:
    lda a:OBJ_KIND,y
    cmp #$52
    bne RenumberKind52ThenClear0D5F_at1
    lda a:OBJ_STATE,y
    cmp #$04
    bcs RenumberKind52ThenClear0D5F_at1
    lda z:zTemp00
    bne RenumberKind52ThenClear0D5F_at2
    sty z:zTemp03
RenumberKind52ThenClear0D5F_at2:
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    lda z:zTemp03
    sta a:OBJ_WORK_M,y
    inc z:zTemp00
RenumberKind52ThenClear0D5F_at1:
    iny
    cpy z:zTemp01
    bcc RenumberKind52ThenClear0D5F_at3
    jsr VecClearObject
RenumberKind52ThenClear0D5F_at0:
    rts

; @routine 0D5F:562 StartWithKind55AndSpeed0D5F
; @run 71:D562
; WHAT IT DOES: Starts the object with OBJ_WORK_E $28, moves it once along X (its speed turned round when facing
; left), sets its X and Y speeds from a record by OBJ_WORK_D and its facing, and starts an object of kind $55 at
; its place in a free slot from $50 with its OBJ_WORK_F (Stage 08's overlay).
; HOW: LoadObjectRecord, NegateVelocityX, ApplyVelocityX, a read of four-byte records by eight times OBJ_WORK_D
; plus four times the facing, FindFreeSlotFrom50, StartObjectAtPlace.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StartWithKind55AndSpeed0D5FSpeeds = $D5AF ; X speed, then Y speed, a word each, by OBJ_WORK_D and facing
StartWithKind55AndSpeed0D5F:
    jsr VecLoadObjectRecord
    lda #$28
    sta a:OBJ_WORK_E,x
    lda a:OBJ_FACING_LEFT,x
    beq StartWithKind55AndSpeed0D5F_at0
    jsr VecNegateVelocityX
StartWithKind55AndSpeed0D5F_at0:
    jsr VecApplyVelocityX
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_FACING_LEFT,x
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:StartWithKind55AndSpeed0D5FSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:StartWithKind55AndSpeed0D5FSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:StartWithKind55AndSpeed0D5FSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:StartWithKind55AndSpeed0D5FSpeeds+2,y
    sta a:OBJ_VY_LO,x
    jsr VecFindFreeSlotFrom50
    bcc StartWithKind55AndSpeed0D5F_at1
    lda #$55
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,y
StartWithKind55AndSpeed0D5F_at1:
    rts

; @routine 0D5F:5BF CountThenArm0D5F
; @run 71:D5BF
; WHAT IT DOES: Counts OBJ_WORK_E down; at zero plays sound $25, turns the hit box on (2), restarts the animation
; and steps the state on; then keeps the Y of the slot in OBJ_WORK_F (KeepYOfWorkFSlot0D5F) (Stage 08's overlay).
; HOW: DEC, SoundCommand, stores, then the piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountThenArm0D5F:
    dec a:OBJ_WORK_E,x
    bne CountThenArm0D5F_at0
    lda #$25
    jsr VecSoundCommand
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
CountThenArm0D5F_at0:
    jmp KeepYOfWorkFSlot0D5F

; @routine 0D5F:5DA KeepYOfWorkFSlot0D5F
; @run 71:D5DA
; WHAT IT DOES: Puts the object at the Y of the slot in OBJ_WORK_F, and clears it once that slot is in state 4
; (Stage 08's overlay).
; HOW: A copy of the Y low byte, a compare, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
KeepYOfWorkFSlot0D5F:
    ldy a:OBJ_WORK_F,x
    lda a:OBJ_Y_LO,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_STATE,y
    cmp #$04
    bne KeepYOfWorkFSlot0D5F_at0
    jmp VecClearObject
KeepYOfWorkFSlot0D5F_at0:
    rts

; @routine 0D5F:5EE FlyListByWorkD0D5F
; @run 71:D5EE
; WHAT IT DOES: Plays the frame list OBJ_WORK_D picks and moves the object by its speed; with OBJ_WORK_D 1 it gains
; $18 of Y speed a frame (Stage 08's overlay).
; HOW: A word table read, StepObjectFrames, ApplyVelocity, a 16-bit add.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FlyListByWorkD0D5FLists = $D61C ; the frame list's address for each OBJ_WORK_D, a word each
FlyListByWorkD0D5F:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:FlyListByWorkD0D5FLists,y
    sta z:zTemp10
    lda a:FlyListByWorkD0D5FLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    cmp #$01
    bne FlyListByWorkD0D5F_at0
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
FlyListByWorkD0D5F_at0:
    rts

; @routine 0D5F:648 StartWithKind55Step0D5F
; @run 71:D648
; WHAT IT DOES: Starts the object with OBJ_WORK_E $40, moves it once along X (its speed turned round when facing
; left), starts an object of kind $55 at its place in a free slot from $50 with OBJ_WORK_D 1 and its OBJ_WORK_F,
; then moves it once more by an X step for its facing (Stage 08's overlay).
; HOW: LoadObjectRecord, NegateVelocityX, ApplyVelocityX, FindFreeSlotFrom50, StartObjectAtPlace, a table read by
; the facing, ApplyVelocityX.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWithKind55Step0D5FSteps = $D67D ; the X speed's high byte facing right, then left
StartWithKind55Step0D5F:
    jsr VecLoadObjectRecord
    lda #$40
    sta a:OBJ_WORK_E,x
    lda a:OBJ_FACING_LEFT,x
    beq StartWithKind55Step0D5F_at0
    jsr VecNegateVelocityX
StartWithKind55Step0D5F_at0:
    jsr VecApplyVelocityX
    jsr VecFindFreeSlotFrom50
    bcc StartWithKind55Step0D5F_at1
    lda #$55
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,y
    lda a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,y
StartWithKind55Step0D5F_at1:
    ldy a:OBJ_FACING_LEFT,x
    lda a:StartWithKind55Step0D5FSteps,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    rts

; @routine 0D5F:697 AnimateHitBoxForCount0D5F
; @run 71:D697
; WHAT IT DOES: Plays the object's frame list, gives it its frame's hit box (HitBoxByFrame0D5F), and counts
; OBJ_WORK_E down, clearing the object at zero (Stage 08's overlay).
; HOW: StepObjectFrames, a call, DEC, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AnimateHitBoxForCount0D5F:
    lda #$D6
    sta z:zTemp11
    lda #$AE
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr HitBoxByFrame0D5F
    dec a:OBJ_WORK_E,x
    bne AnimateHitBoxForCount0D5F_at0
    jmp VecClearObject
AnimateHitBoxForCount0D5F_at0:
    rts

; @routine 0D5F:70F AnimateFollowingYForCount0D5F
; @run 71:D70F
; WHAT IT DOES: In state 0 starts the object with sprite set OBJ_WORK_D plus 3 and OBJ_WORK_E $28; then plays the
; frame list OBJ_WORK_D picks, counts OBJ_WORK_E down to clear it, and keeps the Y of the slot in OBJ_WORK_F
; (KeepYOfWorkFSlot0D5F) (Stage 08's overlay).
; HOW: LoadObjectRecord, an add; a word table read, StepObjectFrames, DEC, then the piece.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
AnimateFollowingYForCount0D5FLists = $D743 ; the frame list's address for each OBJ_WORK_D, a word each
AnimateFollowingYForCount0D5F:
    lda a:OBJ_STATE,x
    bne AnimateFollowingYForCount0D5F_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_D,x
    clc
    adc #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$28
    sta a:OBJ_WORK_E,x
    rts
AnimateFollowingYForCount0D5F_at0:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:AnimateFollowingYForCount0D5FLists,y
    sta z:zTemp10
    lda a:AnimateFollowingYForCount0D5FLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_E,x
    bne AnimateFollowingYForCount0D5F_at1
    jmp VecClearObject
AnimateFollowingYForCount0D5F_at1:
    jmp KeepYOfWorkFSlot0D5F

; @routine 0D60:138 RideAboveOwner0D60
; @run 71:D938
; WHAT IT DOES: In state 0 starts the object with frame 0, not cleared far off screen when OBJ_START_ARG is
; negative, and item 2 in its hit reaction. Later it plays its frame list and follows the slot in OBJ_WORK_D: on
; kind $C5 it sits $17 above that slot's place, facing left when that slot's X speed is negative unless the slot is
; in state 2; on kind $BF it sits $17 above that slot's place plus its hit box offset; on any other kind it is
; cleared (Stage 08's overlay).
; HOW: LoadObjectRecord and ORs; StepObjectFrames, kind compares, 16-bit copies and subtracts.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RideAboveOwner0D60:
    lda a:OBJ_STATE,x
    bne RideAboveOwner0D60_at0
    jsr VecLoadObjectRecord
    stz a:OBJ_FRAME,x
    lda a:OBJ_START_ARG,x
    bpl RideAboveOwner0D60_at1
    lda a:OBJ_LOOP_FLAGS,x
    ora #$02
    sta a:OBJ_LOOP_FLAGS,x
RideAboveOwner0D60_at1:
    lda a:OBJ_HIT_REACTION,x
    and #$E3
    ora #$08
    sta a:OBJ_HIT_REACTION,x
    bra RideAboveOwner0D60_at2
RideAboveOwner0D60_at0:
    lda #$D9
    sta z:zTemp11
    lda #$F0
    sta z:zTemp10
    jsr VecStepObjectFrames
    ldy a:OBJ_WORK_D,x
    lda a:OBJ_KIND,y
    cmp #$C5
    beq RideAboveOwner0D60_at3
    cmp #$BF
    beq RideAboveOwner0D60_at4
    jmp VecClearObject
RideAboveOwner0D60_at3:
    lda a:OBJ_X_LO,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,y
    sta a:OBJ_X_HI,x
    lda a:OBJ_Y_LO,y
    sec
    sbc #$17
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,y
    sbc #$00
    sta a:OBJ_Y_HI,x
    lda a:OBJ_STATE,y
    cmp #$02
    beq RideAboveOwner0D60_at5
RideAboveOwner0D60_at2:
    stz a:OBJ_FACING_LEFT,x
    lda a:OBJ_VX_HI,y
    bpl RideAboveOwner0D60_at5
    inc a:OBJ_FACING_LEFT,x
RideAboveOwner0D60_at5:
    rts
RideAboveOwner0D60_at4:
    stz z:zTemp00
    lda a:OBJ_HITBOX_DX,y
    bpl RideAboveOwner0D60_at6
    dec z:zTemp00
RideAboveOwner0D60_at6:
    stz z:zTemp01
    lda a:OBJ_HITBOX_DY,y
    bpl RideAboveOwner0D60_at7
    dec z:zTemp01
RideAboveOwner0D60_at7:
    lda a:OBJ_X_LO,y
    clc
    adc a:OBJ_HITBOX_DX,y
    sta a:OBJ_X_LO,x
    lda z:zTemp00
    adc a:OBJ_X_HI,y
    sta a:OBJ_X_HI,x
    lda a:OBJ_Y_LO,y
    clc
    adc a:OBJ_HITBOX_DY,y
    sta a:OBJ_Y_LO,x
    lda z:zTemp01
    adc a:OBJ_Y_HI,y
    sta a:OBJ_Y_HI,x
    lda a:OBJ_Y_LO,x
    sec
    sbc #$17
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_Y_HI,x
    rts

; @routine 0D60:323 CarryPlayerByHeightTable0D60
; @run 71:DB23
; WHAT IT DOES: Is a block $17 wide whose height above $16 a table gives by the player's X distance less 6, from 0
; to $0F; when the player stands on it, sets bits 0, 4 and 5 of zPlayerPush and gives the player its Y speed and
; one and a half times its X speed as the push (Stage 08's overlay).
; HOW: DistanceXToPlayer clamped, a table read, ContactWithPlayer, then the speeds copied and halved with LSR and
; ROR.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CarryPlayerByHeightTable0D60Heights = $DB74 ; the height above $16 for each of 16 distances
CarryPlayerByHeightTable0D60:
    jsr VecDistanceXToPlayer
    sec
    sbc #$06
    bcc CarryPlayerByHeightTable0D60_at0
    cmp #$10
    bcc CarryPlayerByHeightTable0D60_at1
    lda #$0F
    bra CarryPlayerByHeightTable0D60_at1
CarryPlayerByHeightTable0D60_at0:
    cla
CarryPlayerByHeightTable0D60_at1:
    tay
    lda a:CarryPlayerByHeightTable0D60Heights,y
    clc
    adc #$16
    sta z:zTemp09
    lda #$17
    sta z:zTemp08
    jsr ContactWithPlayer
    bcc CarryPlayerByHeightTable0D60_at2
    lda z:zPlayerPush
    ora #$31
    sta z:zPlayerPush
    lda a:OBJ_VY_HI,x
    sta z:zPushYHi
    lda a:OBJ_VY_LO,x
    sta z:zPushYLo
    lda a:OBJ_VX_HI,x
    lsr a
    lda a:OBJ_VX_LO,x
    ror a
    clc
    adc a:OBJ_VX_LO,x
    sta z:zPushXLo
    lda a:OBJ_VX_HI,x
    asl a
    lda a:OBJ_VX_HI,x
    php
    ror a
    plp
    adc a:OBJ_VX_HI,x
    sta z:zPushXHi
CarryPlayerByHeightTable0D60_at2:
    rts

; @routine 0D60:384 StartDrifting0D60
; @run 71:DB84
; WHAT IT DOES: Starts the object in state 1 with X speed $40, turned round when OBJ_START_ARG bit 0 is set; with
; OBJ_START_ARG negative it is not cleared far off screen and starts in state 2, or 1 when OBJ_WORK_H is set (Stage
; 08's overlay).
; HOW: LoadObjectRecord, INC, an OR of OBJ_LOOP_FLAGS, NegateVelocityX.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartDrifting0D60:
    jsr VecLoadObjectRecord
    inc a:OBJ_STATE,x
    lda a:OBJ_START_ARG,x
    bpl StartDrifting0D60_at0
    lda a:OBJ_LOOP_FLAGS,x
    ora #$02
    sta a:OBJ_LOOP_FLAGS,x
    inc a:OBJ_STATE,x
    lda a:OBJ_WORK_H,x
    beq StartDrifting0D60_at0
    lda #$01
    sta a:OBJ_STATE,x
StartDrifting0D60_at0:
    lda #$40
    sta a:OBJ_VX_LO,x
    lda a:OBJ_START_ARG,x
    lsr a
    bcc StartDrifting0D60_at1
    jsr VecNegateVelocityX
StartDrifting0D60_at1:
    rts

; @routine 0D60:4FA ReturnOnly0D60
; @run 71:DCFA
; WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
; HOW: An RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
ReturnOnly0D60:
    rts

; @routine 0D60:4FB SpawnKindC5WhenClear0D60
; @run 71:DCFB
; WHAT IT DOES: In state 0 starts the object and moves it 256 pixels up. Later, with zField8C 0 and no object of
; kind $C5 in slots $10 to $57 still above the screen or short of the X and Y a table gives by OBJ_START_ARG,
; starts one at its place in a free slot from $30: with OBJ_START_ARG set at X $90, Y 8 with argument 1; else at
; the X, Y and argument tables give by the scroll's Y over 8, the Y $40 above the scroll when the table's byte is
; positive. Then, except one time in four at random for the second case, starts an object of kind $2E in a free
; slot from $30 with the new slot in OBJ_WORK_D (Stage 08's overlay).
; HOW: LoadObjectRecord and DEC; a scan of the slots with compares; FindFreeSlotFrom30, StartObjectAtPlace and
; table reads by the scroll's Y shifted three bits; NextRandom.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SpawnKindC5WhenClear0D60MinX = $DDC0 ; the X a kind $C5 must have reached, per OBJ_START_ARG
SpawnKindC5WhenClear0D60MinY = $DDC2 ; the Y a kind $C5 must have reached, per OBJ_START_ARG
SpawnKindC5WhenClear0D60X = $DDC4 ; the new object's X low byte by the scroll's Y over 8
SpawnKindC5WhenClear0D60YLo = $DE05 ; its Y low byte, or a positive byte for $40 above the scroll
SpawnKindC5WhenClear0D60YHi = $DE46 ; its Y high byte
SpawnKindC5WhenClear0D60Args = $DE87 ; its OBJ_START_ARG
SpawnKindC5WhenClear0D60:
    lda a:OBJ_STATE,x
    bne SpawnKindC5WhenClear0D60_at0
    jsr VecLoadObjectRecord
    dec a:OBJ_Y_HI,x
    rts
SpawnKindC5WhenClear0D60_at0:
    lda z:zField8C
    bne ReturnOnly0D60
    ldy a:OBJ_START_ARG,x
    lda a:SpawnKindC5WhenClear0D60MinX,y
    sta z:zTemp00
    lda a:SpawnKindC5WhenClear0D60MinY,y
    sta z:zTemp01
    ldy #$10
SpawnKindC5WhenClear0D60_at2:
    lda a:OBJ_KIND,y
    cmp #$C5
    bne SpawnKindC5WhenClear0D60_at1
    lda a:OBJ_X_HI,y
    bne SpawnKindC5WhenClear0D60_at1
    lda a:OBJ_Y_HI,y
    bmi ReturnOnly0D60
    lda a:OBJ_Y_HI,y
    bne SpawnKindC5WhenClear0D60_at1
    lda a:OBJ_X_LO,y
    cmp z:zTemp00
    bcc ReturnOnly0D60
    lda a:OBJ_Y_LO,y
    cmp z:zTemp01
    bcc ReturnOnly0D60
SpawnKindC5WhenClear0D60_at1:
    iny
    cpy #$58
    bne SpawnKindC5WhenClear0D60_at2
    jsr VecFindFreeSlotFrom30
    bcc SpawnKindC5WhenClear0D60_at3
    lda #$C5
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    beq SpawnKindC5WhenClear0D60_at4
    lda #$90
    sta a:OBJ_X_LO,y
    lda #$08
    sta a:OBJ_Y_LO,y
    cla
    sta a:OBJ_Y_HI,y
    lda #$01
    sta a:OBJ_START_ARG,y
    bra SpawnKindC5WhenClear0D60_at5
SpawnKindC5WhenClear0D60_at4:
    phx
    lda a:ScrollYHiScreen
    sta z:zTemp00
    lda a:ScrollYLo
    lsr z:zTemp00
    ror a
    lsr z:zTemp00
    ror a
    lsr z:zTemp00
    ror a
    tax
    lda a:SpawnKindC5WhenClear0D60X,x
    sta a:OBJ_X_LO,y
    lda a:SpawnKindC5WhenClear0D60YLo,x
    bmi SpawnKindC5WhenClear0D60_at6
    lda #$40
    sec
    sbc a:ScrollYLo
    sta a:OBJ_Y_LO,y
    cla
    sbc a:ScrollYHiScreen
    sta a:OBJ_Y_HI,y
    bra SpawnKindC5WhenClear0D60_at7
SpawnKindC5WhenClear0D60_at6:
    sta a:OBJ_Y_LO,y
    lda a:SpawnKindC5WhenClear0D60YHi,x
    sta a:OBJ_Y_HI,y
SpawnKindC5WhenClear0D60_at7:
    lda a:SpawnKindC5WhenClear0D60Args,x
    sta a:OBJ_START_ARG,y
    plx
    jsr VecNextRandom
    and #$03
    beq SpawnKindC5WhenClear0D60_at3
SpawnKindC5WhenClear0D60_at5:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc SpawnKindC5WhenClear0D60_at3
    lda #$2E
    jsr VecStartObjectAtPlace
    lda z:zBurstCount
    sta a:OBJ_WORK_D,y
SpawnKindC5WhenClear0D60_at3:
    rts

; @routine 0D6C:2E9 LevelWordByStep0D6C
; @run 74:DAE9
; WHAT IT DOES: Sets the level word at $3EB8 (RasterWaterWanted, LevelYHi) from a table by OBJ_WORK_F's low three
; bits; when those are 0 plays sound $4E if the first byte of a second table is not 0, the index being always 0
; since the AND comes before the shifts (Stage 08's overlay).
; HOW: A word table read by twice the low bits, a test, a byte table read, SoundCommand.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LevelWordByStep0D6CWords = $DB12 ; the level word for each of eight steps
LevelWordByStep0D6CSounds = $DB52 ; nonzero when the sound plays
LevelWordByStep0D6C:
    lda a:OBJ_WORK_F,x
    and #$07
    asl a
    tay
    lda a:LevelWordByStep0D6CWords,y
    sta a:RasterWaterWanted
    lda a:LevelWordByStep0D6CWords+1,y
    sta a:LevelYHi
    lda a:OBJ_WORK_F,x
    and #$07
    bne LevelWordByStep0D6C_at0
    lsr a
    lsr a
    lsr a
    tay
    lda a:LevelWordByStep0D6CSounds,y
    beq LevelWordByStep0D6C_at0
    lda #$4E
    jsr VecSoundCommand
LevelWordByStep0D6C_at0:
    rts

; @routine 0D6C:372 SpawnKind9EEvery8_0D6C
; @run 74:DB72
; WHAT IT DOES: Every eighth frame of zWaveCounter starts an object of kind $9E (SpawnKind9EScattered0D6C) (Stage
; 08's overlay).
; HOW: A TST of the counter, then falls into the next routine.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SpawnKind9EEvery8_0D6C:
    tst #$07,z:zWaveCounter
    bne ReturnOnly0D6C

; @routine 0D6C:377 SpawnKind9EScattered0D6C
; @run 74:DB77
; WHAT IT DOES: Starts an object of kind $9E at the object's place in a free slot from $20, with the object's
; OBJ_WORK_G as its OBJ_START_ARG, sprite attributes $10 and Y $B0, moved by a random Y step of -$10 to $0F, an X
; step a table gives by its argument's low four bits and a random X step of -3 to 4; then steps OBJ_WORK_G on
; (Stage 08's overlay).
; HOW: FindFreeSlotFrom20, StartObjectAtPlace, NextRandom twice with ApplyVelocityY and ApplyVelocityX three times.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SpawnKind9EScattered0D6CSteps = $DD1A ; the X step for each argument's low four bits
SpawnKind9EScattered0D6C:
    jsr VecFindFreeSlotFrom20
    bcc ReturnOnly0D6C
    lda #$9E
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_G,x
    sta a:OBJ_START_ARG,y
    phx
    sxy
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    stz a:OBJ_Y_HI,x
    lda #$B0
    sta a:OBJ_Y_LO,x
    jsr VecNextRandom
    and #$1F
    sec
    sbc #$10
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda a:OBJ_START_ARG,x
    and #$0F
    tay
    lda a:SpawnKind9EScattered0D6CSteps,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    jsr VecNextRandom
    and #$07
    sec
    sbc #$03
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    plx
    inc a:OBJ_WORK_G,x

; @routine 0D6C:3C5 ReturnOnly0D6C
; @run 74:DBC5
; WHAT IT DOES: Returns at once: the end the routines around it share (Stage 08's overlay).
; HOW: An RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
ReturnOnly0D6C:
    rts

; @routine 0D6C:401 CopyBlockToVram0D6C
; @run 74:DC01
; WHAT IT DOES: Copies the 32 bytes at $C000 plus 32 times A to VRAM as four rows of four words, from the address
; in zTemp10-11 with the rows $40 words apart, the address kept below $1000 (Stage 08's overlay).
; HOW: Shifts build the source in zTemp12 and zProductLo; ClearVdcControlHigh, then per row SelectVramWriteAddress,
; SelectVramData and four word writes counted in zMultiplierHi.
; WAITS: no.
; Entry: A the block's index; zTemp10-11 the VRAM address. Exit: A, X, Y and the flags as its last instruction or
; call left them.
CopyBlockToVram0D6C:
    stz z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    asl a
    rol z:zProduct0
    clc
    adc #$00
    sta z:zTemp12
    lda z:zProduct0
    adc #$C0
    sta z:zProduct0
    jsr VecClearVdcControlHigh
    stz z:zTransposeRowsLeft
    cly
CopyBlockToVram0D6C_at1:
    jsr VecSelectVramWriteAddress
    lda z:zTemp10
    sta a:VDC_DATA_LO
    lda z:zTemp11
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
CopyBlockToVram0D6C_at0:
    lda (zTemp12),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp12),y
    sta a:VDC_DATA_HI
    iny
    inc z:zTransposeRowsLeft
    lda z:zTransposeRowsLeft
    and #$03
    bne CopyBlockToVram0D6C_at0
    lda #$40
    clc
    adc z:zTemp10
    sta z:zTemp10
    lda #$00
    adc z:zTemp11
    and #$0F
    sta z:zTemp11
    lda z:zTransposeRowsLeft
    cmp #$10
    bne CopyBlockToVram0D6C_at1
    rts

; @routine 0D6C:568 FallToB0ThenArm0D6C
; @run 74:DD68
; WHAT IT DOES: Moves the object down under gravity; once its Y low byte reaches $B0 it stops there with hit box 1
; and steps its state on (Stage 08's overlay).
; HOW: ApplyGravity, ApplyVelocityY, a compare and stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FallToB0ThenArm0D6C:
    jsr VecApplyGravity
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp #$B0
    bcc FallToB0ThenArm0D6C_at0
    lda #$B0
    sta a:OBJ_Y_LO,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
FallToB0ThenArm0D6C_at0:
    rts

; @routine 0D6C:583 Kind9BAtFifthHit0D6C
; @run 74:DD83
; WHAT IT DOES: When the object has been hit exactly 5 times, starts an object of kind $9B in slot $1E at its
; place, 12 pixels left and 8 down, and steps its state on with OBJ_WORK_M 4 and OBJ_WORK_L 0 (Stage 08's overlay).
; HOW: A compare, ClearObjectInY, StartObjectAtPlace, ApplyVelocityX and ApplyVelocityY on the new slot, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Kind9BAtFifthHit0D6C:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bne Kind9BAtFifthHit0D6C_at0
    ldy #$1E
    jsr VecClearObjectInY
    lda #$9B
    jsr VecStartObjectAtPlace
    sxy
    lda #$F4
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda #$08
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    sxy
    lda #$04
    sta a:OBJ_WORK_M,x
    stz a:OBJ_WORK_L,x
    inc a:OBJ_STATE,x
Kind9BAtFifthHit0D6C_at0:
    rts

; @routine 0D6C:60F StartKind9EInY0D6C
; @run 74:DE0F
; WHAT IT DOES: Starts an object of kind $9E at the object's place in slot Y and moves it once by its own speeds;
; the two random numbers drawn in A beside those moves are not read (Stage 08's overlay).
; HOW: StartObjectAtPlace, NextRandom with ApplyVelocityX, NextRandom with ApplyVelocityY, on the new slot.
; WAITS: no.
; Entry: X the object; Y the new slot. Exit: A, X, Y and the flags as its last instruction or call left them.
StartKind9EInY0D6C:
    lda #$9E
    jsr VecStartObjectAtPlace
    sxy
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$08
    jsr VecApplyVelocityX
    jsr VecNextRandom
    and #$0F
    jsr VecApplyVelocityY
    sxy
    rts

; @routine 0D6C:62A StartKind5CByWorkL0D6C
; @run 74:DE2A
; WHAT IT DOES: Starts an object of kind $5C at the object's place in a free slot from $20 and moves it once by the
; X and Y steps a table gives by OBJ_WORK_L (Stage 08's overlay).
; HOW: FindFreeSlotFrom20, StartObjectAtPlace, a pair read by twice OBJ_WORK_L, ApplyVelocityX and ApplyVelocityY.
; WAITS: no.
; Entry: X the object. Exit: C set when an object was started; Y its slot.
StartKind5CByWorkL0D6CSteps = $DE59 ; X step, Y step for each OBJ_WORK_L
StartKind5CByWorkL0D6C:
    jsr VecFindFreeSlotFrom20
    bcc StartKind5CByWorkL0D6C_at0
    lda #$5C
    jsr VecStartObjectAtPlace
    phy
    lda a:OBJ_WORK_L,x
    asl a
    tay
    lda a:StartKind5CByWorkL0D6CSteps,y
    sta z:zTemp00
    lda a:StartKind5CByWorkL0D6CSteps+1,y
    sta z:zTemp01
    ply
    sxy
    lda z:zTemp00
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda z:zTemp01
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    sxy
    sec
StartKind5CByWorkL0D6C_at0:
    rts

; @routine 0D6C:6E6 MaskTilesByRecord0D6C
; @run 74:DEE6
; WHAT IT DOES: Takes the five-byte record OBJ_WORK_L picks and masks VRAM words at $3F00 plus its first byte
; (MaskVramWords0D6C) with its second byte, then its third, and its third again unless that is $FF (Stage 08's
; overlay).
; HOW: Five times OBJ_WORK_L indexes the records; three calls with zTemp08, zTemp09 $3F and zTemp0F set.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MaskTilesByRecord0D6CRecords = $DF83 ; the VRAM column, then the mask bytes, five bytes per record
MaskTilesByRecord0D6C:
    lda a:OBJ_WORK_L,x
    sta z:zTemp00
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda #$3F
    sta z:zTemp09
    lda a:MaskTilesByRecord0D6CRecords,y
    sta z:zTemp08
    lda a:MaskTilesByRecord0D6CRecords+1,y
    sta z:zTemp0F
    phy
    jsr MaskVramWords0D6C
    ply
    lda a:MaskTilesByRecord0D6CRecords+2,y
    sta z:zTemp0F
    phy
    jsr MaskVramWords0D6C
    ply
    lda a:MaskTilesByRecord0D6CRecords+2,y
    sta z:zTemp0F
    cmp #$FF
    bne MaskVramWords0D6C
    rts

; @routine 0D6C:718 MaskVramWords0D6C
; @run 74:DF18
; WHAT IT DOES: ANDs four VRAM words, $10 apart from zTemp09-08 plus zTemp0F's high four bits, with the low and
; high mask bytes zTemp0F's low four bits pick (Stage 08's overlay).
; HOW: A loop of SelectVramReadAddress, SelectVramWriteAddress and SelectVramData with a read, AND and write.
; WAITS: no.
; Entry: zTemp08 the address low byte; zTemp09 its high byte; zTemp0F the offset and mask nibbles. Exit: A, X, Y
; and the flags as its last instruction or call left them.
MaskVramWords0D6CMasksLo = $DF6B ; the low byte's mask for each of eight entries
MaskVramWords0D6CMasksHi = $DF73 ; the high byte's mask for each of eight entries
MaskVramWords0D6C:
    lda z:zTemp0F
    and #$0F
    tay
    lda z:zTemp0F
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc z:zTemp08
    sta z:zTemp0A
    lda z:zTemp09
    sta z:zTemp0B
    lda #$04
    sta z:zTemp0E
MaskVramWords0D6C_at0:
    phx
    phy
    ldx z:zTemp0A
    ldy z:zTemp0B
    jsr VecSelectVramReadAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jsr VecSelectVramData
    ply
    lda a:VDC_DATA_LO
    and a:MaskVramWords0D6CMasksLo,y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and a:MaskVramWords0D6CMasksHi,y
    sta a:VDC_DATA_HI
    plx
    lda z:zTemp0A
    clc
    adc #$10
    sta z:zTemp0A
    dec z:zTemp0E
    bne MaskVramWords0D6C_at0
    rts
