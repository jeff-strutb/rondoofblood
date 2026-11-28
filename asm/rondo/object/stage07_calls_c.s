; @routine 0BFA:3DE DrawTilesThenClear0BFA
; @run 70:ABDE
; WHAT IT DOES: Draws item $0B of tile script group 0 and removes the object (Stage 07's overlay).
; HOW: DrawTileScript, then ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
DrawTilesThenClear0BFA:
    lda #$0B
    cly
    jsr VecDrawTileScript
    jsr VecClearObject
    rts

; @routine 0BFB:10D StartSinkingTop0BFB
; @run 70:B10D
; @stop B11F
; WHAT IT DOES: In state 0 loads the object's record and sets OBJ_WORK_D, the height of the top the player stands
; on, from a table by OBJ_START_ARG; in later states goes on to the contact test at $B127 (Stage 07's overlay).
; HOW: A test of the state; LoadObjectRecord and a table read.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartSinkingTop0BFBHeights = $B123 ; the full OBJ_WORK_D height for each OBJ_START_ARG
StartSinkingTop0BFB:
    lda a:OBJ_STATE,x
    bne SinkUnderPlayer0BFB
    jsr VecLoadObjectRecord
    ldy a:OBJ_START_ARG,x
    lda a:StartSinkingTop0BFBHeights,y
    sta a:OBJ_WORK_D,x
    rts

; @routine 0BFB:127 SinkUnderPlayer0BFB
; @run 70:B127
; @stop B198
; WHAT IT DOES: Unless the player's Y high byte is 1 or his Y low $FA or more, tests his contact with the object
; (width by OBJ_START_ARG, height OBJ_WORK_D); standing on it sets OBJ_WORK_E and zPlayerPush bit 4, pushes him
; back by his own X speed, and lowers OBJ_WORK_D by a step for each of its first 8 frames (OBJ_WORK_F), then by 1
; on even zWaveCounter with the push halved; off it, goes on at $B1A0 (Stage 07's overlay).
; HOW: ContactWithPlayer with zTemp08/09; zPlayerPush bits 1 and 2 cleared; tables by OBJ_START_ARG and OBJ_WORK_F.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SinkUnderPlayer0BFBWidths = $B11F ; the contact width for each OBJ_START_ARG
SinkUnderPlayer0BFBDrops = $B198 ; the drop of OBJ_WORK_D for each of the first 8 frames stood on
SinkUnderPlayer0BFB:
    lda a:OBJ_Y_HI
    cmp #$01
    beq SinkUnderPlayer0BFB_at0
    lda a:OBJ_Y_LO
    cmp #$FA
    bcs SinkUnderPlayer0BFB_at0
    ldy a:OBJ_START_ARG,x
    lda a:SinkUnderPlayer0BFBWidths,y
    sta z:zTemp08
    lda a:OBJ_WORK_D,x
    sta z:zTemp09
    beq SinkUnderPlayer0BFB_at0
    jsr ContactWithPlayer
    php
    lda z:zPlayerPush
    and #$F9
    sta z:zPlayerPush
    plp
    bcc RiseBackAndKick0BFB
    lda #$01
    sta a:OBJ_WORK_E,x
    lda z:zPlayerPush
    ora #$10
    sta z:zPlayerPush
    lda a:OBJ_VX_LO
    eor #$FF
    clc
    adc #$01
    sta z:zPushXLo
    lda a:OBJ_VX_HI
    eor #$FF
    adc #$00
    sta z:zPushXHi
    lda a:OBJ_WORK_F,x
    cmp #$08
    bcc SinkUnderPlayer0BFB_at1
    lda z:zWaveCounter
    and #$01
    bne SinkUnderPlayer0BFB_at0
    lda z:zPushXHi
    asl a
    ror z:zPushXHi
    ror z:zPushXLo
    dec a:OBJ_WORK_D,x
SinkUnderPlayer0BFB_at0:
    rts
SinkUnderPlayer0BFB_at1:
    ldy a:OBJ_WORK_F,x
    lda a:OBJ_WORK_D,x
    sec
    sbc a:SinkUnderPlayer0BFBDrops,y
    sta a:OBJ_WORK_D,x
    inc a:OBJ_WORK_F,x
    rts

; @routine 0BFB:1A0 RiseBackAndKick0BFB
; @run 70:B1A0
; WHAT IT DOES: Raises OBJ_WORK_D by 1 while it is below its full height; if the player had stood on it
; (OBJ_WORK_E) and now moves up or down, unless zGameSubMode is 4, halves his X speed and gives him Y speed -$340;
; then clears OBJ_WORK_E and OBJ_WORK_F (Stage 07's overlay).
; HOW: A compare with the table by OBJ_START_ARG; stores to slot 0's speeds.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RiseBackAndKick0BFBHeights = $B123 ; the full OBJ_WORK_D height for each OBJ_START_ARG
RiseBackAndKick0BFB:
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_WORK_D,x
    cmp a:RiseBackAndKick0BFBHeights,y
    bcs RiseBackAndKick0BFB_at0
    inc a:OBJ_WORK_D,x
RiseBackAndKick0BFB_at0:
    lda a:OBJ_WORK_E,x
    beq RiseBackAndKick0BFB_at1
    lda a:OBJ_VY_HI
    beq RiseBackAndKick0BFB_at1
    lda z:zGameSubMode
    cmp #$04
    beq RiseBackAndKick0BFB_at1
    lda a:OBJ_VX_HI
    asl a
    ror a:OBJ_VX_HI
    ror a:OBJ_VX_LO
    lda #$FC
    sta a:OBJ_VY_HI
    lda #$C0
    sta a:OBJ_VY_LO
RiseBackAndKick0BFB_at1:
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_F,x
    rts

; @routine 0BFB:1F1 ShoveToEdgeWhenHit0BFB
; @run 70:B1F1
; WHAT IT DOES: Tests the player's contact (width $0E, height $26); on a hit below 5 moves the object by an X speed
; chosen by the side of the player, and when its screen X reaches $1FC sounds $3A, records its event, starts kind
; $A7 in slot $41 at the player's place and steps the state on (Stage 07's overlay).
; HOW: ContactWithPlayer; a word table by SideOfPlayer's bit 0; a 16-bit screen X from the scroll.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ShoveToEdgeWhenHit0BFBSpeeds = $B24B ; the X speed for each side of the player, low and high byte
ShoveToEdgeWhenHit0BFB:
    lda #$0E
    sta z:zTemp08
    lda #$26
    sta z:zTemp09
    jsr ContactWithPlayer
    lda a:OBJ_HIT_TAKEN,x
    beq ShoveToEdgeWhenHit0BFB_at0
    cmp #$05
    bcs ShoveToEdgeWhenHit0BFB_at0
    jsr VecSideOfPlayer
    and #$01
    asl a
    tay
    lda a:ShoveToEdgeWhenHit0BFBSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:ShoveToEdgeWhenHit0BFBSpeeds,y
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    lda a:ScrollXLo
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:ScrollXHiScreen
    adc a:OBJ_X_HI,x
    cmp #$01
    bne ShoveToEdgeWhenHit0BFB_at0
    lda z:zTemp00
    cmp #$FC
    bne ShoveToEdgeWhenHit0BFB_at0
    lda #$3A
    jsr VecSoundCommand
    jsr VecRecordObjectEvent
    ldy #$41
    phx
    ldx #$00
    lda #$A7
    jsr VecStartObjectAtPlace
    plx
    inc a:OBJ_STATE,x
ShoveToEdgeWhenHit0BFB_at0:
    rts

; @routine 0BFB:24F CarryPlayerSprite0BFB
; @run 70:B24F
; WHAT IT DOES: In state 0 loads the record, takes the player's hit box off, sets sprite flip $80, OBJ_WORK_D $B8
; and Y speed low $40; then counts OBJ_WORK_D down, moving down and wearing the player's sprite set, frame and
; facing (his own frame cleared, OBJ_WORK_E $10, PlayerStartPose 2); at 0 sets zField83 $0B, zField87 1 and
; zSubStep 2 (Stage 07's overlay).
; HOW: Two arms by the state; DEC, ApplyVelocityY and copies from slot 0.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CarryPlayerSprite0BFB:
    lda a:OBJ_STATE,x
    bne CarryPlayerSprite0BFB_at0
    jsr VecLoadObjectRecord
    stz a:OBJ_HITBOX_ON
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
    lda #$B8
    sta a:OBJ_WORK_D,x
    lda #$40
    sta a:OBJ_VY_LO,x
    rts
CarryPlayerSprite0BFB_at0:
    dec a:OBJ_WORK_D,x
    bne CarryPlayerSprite0BFB_at1
    lda #$0B
    sta z:zField83
    lda #$01
    sta z:zField87
    lda #$02
    sta z:zSubStep
    stz a:OBJ_FRAME
    rts
CarryPlayerSprite0BFB_at1:
    jsr VecApplyVelocityY
    lda #$10
    sta a:OBJ_WORK_E
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_WORK_F
    lda a:OBJ_SPRITE_SET
    sta a:OBJ_SPRITE_SET,x
    lda a:OBJ_FRAME
    sta a:OBJ_FRAME,x
    lda a:OBJ_FACING_LEFT
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_FRAME
    rts

; @routine 0BFB:2BD LoadRecordBoxDown08_0BFB
; @run 70:B2BD
; WHAT IT DOES: Loads the object's record and sets its hit box Y offset to 8 (Stage 07's overlay).
; HOW: LoadObjectRecord and a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordBoxDown08_0BFB:
    jsr VecLoadObjectRecord
    lda #$08
    sta a:OBJ_HITBOX_DY,x
    rts

; @routine 0BFB:2C6 BreakWithNextSlot0BFB
; @run 70:B2C6
; WHAT IT DOES: On a hit of 5 or more sounds $12, records the event of the slot after the object, stops that slot's
; fall and steps its state and the object's on; with OBJ_START_ARG not 0 also records event $11 (Stage 07's
; overlay).
; HOW: A compare of OBJ_HIT_TAKEN; INX and DEX around the stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BreakWithNextSlot0BFB:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc BreakWithNextSlot0BFB_at0
    lda #$12
    jsr VecSoundCommand
    inx
    jsr VecRecordObjectEvent
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_STATE,x
    dex
    inc a:OBJ_STATE,x
    lda a:OBJ_START_ARG,x
    beq BreakWithNextSlot0BFB_at0
    lda #$11
    jsr VecRecordEvent
BreakWithNextSlot0BFB_at0:
    rts

; @routine 0BFB:306 LoadRecordBoxDown4B_0BFB
; @run 70:B306
; WHAT IT DOES: Loads the object's record and sets its hit box Y offset to $4B (Stage 07's overlay).
; HOW: LoadObjectRecord and a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordBoxDown4B_0BFB:
    jsr VecLoadObjectRecord
    lda #$4B
    sta a:OBJ_HITBOX_DY,x
    rts

; @routine 0BFB:30F SoundAndGuardOnHit0BFB
; @run 70:B30F
; WHAT IT DOES: On a hit of 5 or more sounds $12 and makes the object untouchable for 5 frames (OBJ_INVULN_TIMER)
; (Stage 07's overlay).
; HOW: A compare, SoundCommand, a store.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SoundAndGuardOnHit0BFB:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc SoundAndGuardOnHit0BFB_at0
    lda #$12
    jsr VecSoundCommand
    lda #$05
    sta a:OBJ_INVULN_TIMER,x
SoundAndGuardOnHit0BFB_at0:
    rts

; @routine 0BFB:321 FallThenClearLow0BFB
; @run 70:B321
; WHAT IT DOES: Falls and moves; once its Y reaches $180 or more (high byte 1, low $80 up) sounds $4E when the slot
; before it has an OBJ_START_ARG, and removes itself (Stage 07's overlay).
; HOW: ApplyGravityUnlimited, ApplyVelocity, two compares of Y.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FallThenClearLow0BFB:
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne FallThenClearLow0BFB_at0
    lda a:OBJ_Y_LO,x
    bpl FallThenClearLow0BFB_at0
    lda a:OBJ_START_ARG-1,x
    beq FallThenClearLow0BFB_at1
    lda #$4E
    jsr VecSoundCommand
FallThenClearLow0BFB_at1:
    jsr VecClearObject
FallThenClearLow0BFB_at0:
    rts

; @routine 0BFB:341 SwingSlotsBySine0BFB
; @run 70:B341
; WHAT IT DOES: In state 0 loads the record; then steps OBJ_WORK_F (frame $0A or $0B by its bit 4), and moves the
; slot two before it, and the slot just before it while that is below state 2, up or down by the sine of OBJ_WORK_D
; scaled by the phase OBJ_WORK_E's size; OBJ_WORK_D advances by the phase's step, and wrapping past $FF sets the
; phase back to 0 (Stage 07's overlay).
; HOW: Tables by OBJ_WORK_E; ScaleBySine, NegateVelocityY and ApplyVelocityY on slots X-2 and X-1.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SwingSlotsBySine0BFBFrameSteps = $B3B6 ; the step of OBJ_WORK_F for each phase
SwingSlotsBySine0BFBPhases = $B3BA ; per phase: the step of OBJ_WORK_D, then the sine's scale high byte
SwingSlotsBySine0BFB:
    lda a:OBJ_STATE,x
    bne SwingSlotsBySine0BFB_at0
    jmp VecLoadObjectRecord
SwingSlotsBySine0BFB_at0:
    ldy a:OBJ_WORK_E,x
    lda a:OBJ_WORK_F,x
    clc
    adc a:SwingSlotsBySine0BFBFrameSteps,y
    sta a:OBJ_WORK_F,x
    lsr a
    lsr a
    lsr a
    lsr a
    and #$01
    clc
    adc #$0A
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_E,x
    asl a
    tay
    stz z:zTemp00
    lda a:SwingSlotsBySine0BFBPhases+1,y
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecScaleBySine
    phx
    dex
    dex
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecNegateVelocityY
    jsr VecApplyVelocityY
    inx
    lda a:OBJ_STATE,x
    cmp #$02
    bcs SwingSlotsBySine0BFB_at1
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecNegateVelocityY
    jsr VecApplyVelocityY
SwingSlotsBySine0BFB_at1:
    plx
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:OBJ_WORK_D,x
    clc
    adc a:SwingSlotsBySine0BFBPhases,y
    sta a:OBJ_WORK_D,x
    bcc SwingSlotsBySine0BFB_at2
    cla
    sta a:OBJ_WORK_E,x
SwingSlotsBySine0BFB_at2:
    rts

; @routine 0BFB:3BD FlaggedThenByState0BFB
; @run 70:B3BD
; WHAT IT DOES: Puts a flagged object in state 4 (EnterState4IfFlagged), then runs the handler of its state (Stage
; 07's overlay).
; HOW: A call, then a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlaggedThenByState0BFBHandlers = $B3D2 ; the handler address for each state, low and high byte
FlaggedThenByState0BFB:
    jsr EnterState4IfFlagged
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlaggedThenByState0BFBHandlers,y
    sta z:zTemp00
    lda a:FlaggedThenByState0BFBHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFB:3FF WobbleThenLeap0BFB
; @run 70:B3FF
; WHAT IT DOES: Moves by an X speed from an eight-step table by OBJ_WORK_D while counting it down; at 0 leaps at
; the player: X speed by his distance in steps of 32 pixels, Y speed high $FA, state on, turned to face him (Stage
; 07's overlay).
; HOW: Tables by OBJ_WORK_D's low three bits and by DistanceXToPlayer over 32; NegateVelocityX when facing left.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WobbleThenLeap0BFBSteps = $B43F ; the X speed high byte for each of the eight steps
WobbleThenLeap0BFBSpeeds = $B447 ; the leap's X speed by distance, low and high byte
WobbleThenLeap0BFB:
    lda a:OBJ_WORK_D,x
    and #$07
    tay
    lda a:WobbleThenLeap0BFBSteps,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    dec a:OBJ_WORK_D,x
    bne WobbleThenLeap0BFB_at0
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:WobbleThenLeap0BFBSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WobbleThenLeap0BFBSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FA
    sta a:OBJ_VY_HI,x
    inc a:OBJ_STATE,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    beq WobbleThenLeap0BFB_at0
    jsr VecNegateVelocityX
WobbleThenLeap0BFB_at0:
    rts

; @routine 0BFB:457 AnimateUntilLanded0BFB
; @run 70:B457
; WHAT IT DOES: Plays the frame list at $B47F and falls (height 8); on landing sounds $4A, restarts the animation
; and goes to state 1 with frame $27 (Stage 07's overlay).
; HOW: StepObjectFrames, StopOnLanding, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
AnimateUntilLanded0BFB:
    lda #$7F
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$08
    jsr VecStopOnLanding
    bcc AnimateUntilLanded0BFB_at0
    lda #$4A
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$27
    sta a:OBJ_FRAME,x
    lda #$01
    sta a:OBJ_STATE,x
AnimateUntilLanded0BFB_at0:
    rts

; @routine 0BFB:48A DebrisAboveAndBelow0BFB
; @run 70:B48A
; WHAT IT DOES: Starts a debris burst (timer 1) 16 pixels above the object and another 16 below, moves back up and
; removes the object (Stage 07's overlay).
; HOW: ApplyVelocityY by $F0, $20 and $F0 around two StartDebrisBurst calls; ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
DebrisAboveAndBelow0BFB:
    lda #$F0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$01
    jsr StartDebrisBurst
    lda #$20
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$01
    jsr StartDebrisBurst
    lda #$F0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    jmp VecClearObject

; @routine 0BFB:58D RiseToListEnd0BFB
; @run 70:B58D
; WHAT IT DOES: Moves by its Y speed and plays the frame list at $B5BE; when the list ends sets Y speed -$100,
; sprite set 4, frame 0 and Y low $4C, restarts the animation and steps the state on (Stage 07's overlay).
; HOW: ApplyVelocityY, StepObjectFrames, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RiseToListEnd0BFB:
    jsr VecApplyVelocityY
    lda #$BE
    sta z:zTemp10
    lda #$B5
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne RiseToListEnd0BFB_at0
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    lda #$04
    sta a:OBJ_SPRITE_SET,x
    stz a:OBJ_FRAME,x
    lda #$4C
    sta a:OBJ_Y_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
RiseToListEnd0BFB_at0:
    rts

; @routine 0BFB:601 StartObjectsFromList0BFB
; @run 70:B601
; WHAT IT DOES: Starts the objects of a list at the object's place: for each record, clears the slot it names,
; starts the kind it names there and gives it its OBJ_START_ARG, until the $FF end (Stage 07's overlay).
; HOW: A loop over four-byte records; ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartObjectsFromList0BFBRecords = $B62D ; slot, kind, OBJ_START_ARG, a spare byte per record; $FF ends
StartObjectsFromList0BFB:
    cly
StartObjectsFromList0BFB_at1:
    lda a:StartObjectsFromList0BFBRecords,y
    cmp #$FF
    bne StartObjectsFromList0BFB_at0
    rts
StartObjectsFromList0BFB_at0:
    sta z:zTemp00
    lda a:StartObjectsFromList0BFBRecords+1,y
    sta z:zTemp01
    lda a:StartObjectsFromList0BFBRecords+2,y
    sta z:zTemp02
    tya
    clc
    adc #$04
    pha
    ldy z:zTemp00
    jsr VecClearObjectInY
    lda z:zTemp01
    jsr VecStartObjectAtPlace
    lda z:zTemp02
    sta a:OBJ_START_ARG,y
    ply
    bra StartObjectsFromList0BFB_at1

; @routine 0BFB:646 SteerTowardPlayer0BFB
; @run 70:B646
; WHAT IT DOES: Plays the frame list at $B6A6, turns its angle OBJ_WORK_E by 1 toward the point $20 above the
; player, raises its speed OBJ_WORK_F:OBJ_WORK_G by 2 while the low byte is below $80, and moves at that angle and
; speed (Stage 07's overlay).
; HOW: AngleToPlayerOffset, TurnAngleToward, SetVelocityFromAngle, ApplyVelocity.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SteerTowardPlayer0BFB:
    lda #$A6
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    stz z:zTemp08
    stz z:zTemp09
    lda #$E0
    sta z:zTemp0A
    lda #$FF
    sta z:zTemp0B
    jsr VecAngleToPlayerOffset
    pha
    lda #$01
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    sta z:zTemp01
    pla
    sta z:zTemp02
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_F,x
    cmp #$00
    bcc SteerTowardPlayer0BFB_at0
    lda a:OBJ_WORK_G,x
    cmp #$80
    bcs SteerTowardPlayer0BFB_at1
SteerTowardPlayer0BFB_at0:
    lda a:OBJ_WORK_G,x
    clc
    adc #$02
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    adc #$00
    sta a:OBJ_WORK_F,x
SteerTowardPlayer0BFB_at1:
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 0BFB:6B7 BackAwayFromPlayer0BFB
; @run 70:B6B7
; WHAT IT DOES: In substate 0 aims directly away from the player at speed $0080 for $14 frames (OBJ_WORK_H); then
; moves at that angle and, when the count runs out, goes to state 4 with its speed cleared (Stage 07's overlay).
; HOW: AngleToPlayer plus $80; SetVelocityFromAngle and ApplyVelocity; DEC.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
BackAwayFromPlayer0BFB:
    lda a:OBJ_SUBSTATE,x
    bne BackAwayFromPlayer0BFB_at0
    jsr VecAngleToPlayer
    clc
    adc #$80
    sta a:OBJ_WORK_E,x
    lda #$00
    sta a:OBJ_WORK_F,x
    lda #$80
    sta a:OBJ_WORK_G,x
    lda #$14
    sta a:OBJ_WORK_H,x
    inc a:OBJ_SUBSTATE,x
    rts
BackAwayFromPlayer0BFB_at0:
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    dec a:OBJ_WORK_H,x
    bne BackAwayFromPlayer0BFB_at1
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_WORK_F,x
    stz a:OBJ_WORK_G,x
BackAwayFromPlayer0BFB_at1:
    rts

; @routine 0BFB:714 DefeatSoundsAndRecord0BFB
; @run 70:B714
; WHAT IT DOES: Sends sounds $FF and $B2, records the object's event, clears OBJ_PACE bits 3 to 6, sets OBJ_WORK_H
; $40 and steps the substate on (Stage 07's overlay).
; HOW: The calls in turn, then stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
DefeatSoundsAndRecord0BFB:
    lda #$FF
    jsr VecSoundCommand
    lda #$B2
    jsr VecSoundCommand
    jsr VecRecordObjectEvent
    lda a:OBJ_PACE,x
    and #$87
    sta a:OBJ_PACE,x
    lda #$40
    sta a:OBJ_WORK_H,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0BFB:76A PieceOnCount0BFB
; @run 70:B76A
; WHAT IT DOES: When OBJ_WORK_H's low six bits match one of eight counts, starts kind $9E in a free slot from $30,
; sounds $18 and gives it the X and Y speed of that count, moving it once (Stage 07's overlay).
; HOW: A search of the counts; FindFreeSlotFrom30, StartObjectAtPlace; a two-byte record by the match.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PieceOnCount0BFBCounts = $B7A6 ; the eight OBJ_WORK_H counts that start a piece
PieceOnCount0BFBSpeeds = $B7AE ; the piece's X speed high, Y speed high for each count
PieceOnCount0BFB:
    cly
PieceOnCount0BFB_at1:
    lda a:OBJ_WORK_H,x
    and #$3F
    cmp a:PieceOnCount0BFBCounts,y
    beq PieceOnCount0BFB_at0
    iny
    cpy #$08
    bcc PieceOnCount0BFB_at1
    rts
PieceOnCount0BFB_at0:
    stx z:zClimbStepBefore
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc PieceOnCount0BFB_at2
    lda #$9E
    jsr VecStartObjectAtPlace
    lda #$18
    jsr VecSoundCommand
    lda z:zBurstCount
    asl a
    tax
    lda a:PieceOnCount0BFBSpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:PieceOnCount0BFBSpeeds+1,x
    sta a:OBJ_VY_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zClimbStepBefore
PieceOnCount0BFB_at2:
    rts

; @routine 0BFB:7D6 FrameByAngleUntilHit0BFB
; @run 70:B7D6
; WHAT IT DOES: Shows frame $0E plus a sixteenth of its angle to the player; once hit (OBJ_HIT_STATUS bit 7) sounds
; $B1, stops its fall, sets OBJ_WORK_D $40 and steps the state on (Stage 07's overlay).
; HOW: AngleToPlayer shifted down 4; a test of bit 7; stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FrameByAngleUntilHit0BFB:
    jsr VecAngleToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc #$0E
    sta a:OBJ_FRAME,x
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq FrameByAngleUntilHit0BFB_at0
    lda #$B1
    jsr VecSoundCommand
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
FrameByAngleUntilHit0BFB_at0:
    rts

; @routine 0BFC:004 FallDroppingPieces0BFC
; @run 70:B804
; WHAT IT DOES: Falls and counts OBJ_WORK_D down, removing itself at 0; at eight of the counts starts kind $9C in a
; free slot from $30 with sound $18 (Stage 07's overlay).
; HOW: ApplyGravityUnlimited, ApplyVelocityY, DEC; a search of the counts.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FallDroppingPieces0BFCCounts = $B831 ; the eight OBJ_WORK_D counts that start a piece
FallDroppingPieces0BFC:
    jsr VecApplyGravityUnlimited
    jsr VecApplyVelocityY
    dec a:OBJ_WORK_D,x
    bne FallDroppingPieces0BFC_at0
    jmp VecClearObject
FallDroppingPieces0BFC_at0:
    cly
FallDroppingPieces0BFC_at2:
    lda a:OBJ_WORK_D,x
    cmp a:FallDroppingPieces0BFCCounts,y
    beq FallDroppingPieces0BFC_at1
    iny
    cpy #$08
    bcc FallDroppingPieces0BFC_at2
    rts
FallDroppingPieces0BFC_at1:
    jsr VecFindFreeSlotFrom30
    bcc FallDroppingPieces0BFC_at3
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$18
    jsr VecSoundCommand
FallDroppingPieces0BFC_at3:
    rts

; @routine 0BFC:051 StartAngleByArg0BFC
; @run 70:B851
; WHAT IT DOES: Loads the object's record and sets OBJ_WORK_D, its angle, from a table by OBJ_START_ARG (Stage 07's
; overlay).
; HOW: LoadObjectRecord and a table read.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartAngleByArg0BFCAngles = $B85E ; the starting OBJ_WORK_D for each OBJ_START_ARG
StartAngleByArg0BFC:
    jsr VecLoadObjectRecord
    ldy a:OBJ_START_ARG,x
    lda a:StartAngleByArg0BFCAngles,y
    sta a:OBJ_WORK_D,x
    rts

; @routine 0BFC:061 OrbitSlot4C0BFC
; @run 70:B861
; WHAT IT DOES: Plays the frame list at $B8AA and circles slot $4C: takes its place, then moves at angle OBJ_WORK_D
; (stepped by 2) at a speed of $24 plus a sine of OBJ_WORK_E scaled by $20; the hit box is on (2) only while that
; speed is $30 or more (Stage 07's overlay).
; HOW: CopyObjectPlaceBack, ScaleBySine, SetVelocityFromAngle, ApplyVelocity.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
OrbitSlot4C0BFC:
    lda #$AA
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    ldy #$4C
    jsr VecCopyObjectPlaceBack
    stz z:zTemp00
    lda #$20
    sta z:zTemp01
    lda a:OBJ_WORK_E,x
    jsr VecScaleBySine
    inc a:OBJ_WORK_E,x
    lda z:SpriteFlipMask
    sta z:zTemp00
    lda z:zPlaneRowsLeft
    clc
    adc #$24
    sta z:zTemp01
    stz a:OBJ_HITBOX_ON,x
    cmp #$30
    bcc OrbitSlot4C0BFC_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
OrbitSlot4C0BFC_at0:
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    clc
    adc #$02
    sta a:OBJ_WORK_D,x
    rts

; @routine 0BFC:0B5 FallFromRest0BFC
; @run 70:B8B5
; WHAT IT DOES: In substate 0 stops its fall, takes the hit box off and clears OBJ_LOOP_FLAGS bit 1; then falls
; (Stage 07's overlay).
; HOW: Two arms by the substate; ApplyGravityUnlimited and ApplyVelocityY.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FallFromRest0BFC:
    lda a:OBJ_SUBSTATE,x
    bne FallFromRest0BFC_at0
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_LOOP_FLAGS,x
    and #$FD
    sta a:OBJ_LOOP_FLAGS,x
    inc a:OBJ_SUBSTATE,x
    rts
FallFromRest0BFC_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocityY

; @routine 0BFC:0D5 StartKind34OnTimer0BFC
; @run 70:B8D5
; WHAT IT DOES: In state 0 loads the record; then, unless slot $54 shows frame 4 or the scroll stands at exactly
; $300, counts the 16-bit OBJ_WORK_D:OBJ_WORK_E down and at 0 starts kind $34 in a free slot from $30 and reloads
; the count from a table by OBJ_START_ARG (Stage 07's overlay).
; HOW: A 16-bit subtract; FindFreeSlotFrom30, StartObjectAtPlace; a word table.
; WAITS: no.
; Entry: X the object; Y as the caller set it; D as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
StartKind34OnTimer0BFCWaits = $B927 ; the count for each OBJ_START_ARG: OBJ_WORK_E, then OBJ_WORK_D
StartKind34OnTimer0BFC:
    lda a:OBJ_STATE,x
    bne StartKind34OnTimer0BFC_at0
    jmp VecLoadObjectRecord
StartKind34OnTimer0BFC_at0:
    lda a:OBJ_FACING_LEFT-4
    cmp #$04
    beq StartKind34OnTimer0BFC_at1
    lda a:ScrollXHiScreen
    cmp #$03
    bne StartKind34OnTimer0BFC_at2
    lda a:ScrollXLo
    bne StartKind34OnTimer0BFC_at2
    rts
StartKind34OnTimer0BFC_at2:
    lda a:OBJ_WORK_E,x
    ora a:OBJ_WORK_D,x
    beq StartKind34OnTimer0BFC_at3
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    rts
StartKind34OnTimer0BFC_at3:
    jsr VecFindFreeSlotFrom30
    bcc StartKind34OnTimer0BFC_at1
    lda #$34
    jsr VecStartObjectAtPlace
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:StartKind34OnTimer0BFCWaits+1,y
    sta a:OBJ_WORK_D,x
    lda a:StartKind34OnTimer0BFCWaits,y
    sta a:OBJ_WORK_E,x
StartKind34OnTimer0BFC_at1:
    rts

; @routine 0BFC:25A PiecesBoxThenByState0BFC
; @run 70:BA5A
; WHAT IT DOES: Spawns the object's pieces when it is hit (SpawnPiecesWhenFlaggedHit), sizes its hit box by frame,
; then runs the handler of its state (Stage 07's overlay).
; HOW: Two calls, then a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PiecesBoxThenByState0BFCHandlers = $BA72 ; the handler address for each state, low and high byte
PiecesBoxThenByState0BFC:
    jsr SpawnPiecesWhenFlaggedHit
    jsr SetHitboxSizeFromFrame
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:PiecesBoxThenByState0BFCHandlers,y
    sta z:zTemp00
    lda a:PiecesBoxThenByState0BFCHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFC:358 WalkUntilProbeBit1_0BFC
; @run 70:BB58
; WHAT IT DOES: Plays the frame list at $BB88 and moves by its X speed; when the terrain probe ($22, 4) reports bit
; 1, clears OBJ_WORK_B, OBJ_WORK_C, the animation and the substate and goes to state 3 (Stage 07's overlay).
; HOW: StepObjectFrames, ApplyVelocityX, ProbeTerrain, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
WalkUntilProbeBit1_0BFC:
    lda #$88
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityX
    lda #$04
    sta z:SpriteFlipMask
    lda #$22
    jsr VecProbeTerrain
    and #$02
    beq WalkUntilProbeBit1_0BFC_at0
    stz a:OBJ_WORK_C,x
    stz a:OBJ_WORK_B,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    lda #$03
    sta a:OBJ_STATE,x
WalkUntilProbeBit1_0BFC_at0:
    rts

; @routine 0BFC:444 HitThenByState0BFC
; @run 70:BC44
; WHAT IT DOES: Sends a hit object to state 6 ($BC67), then runs the handler of its state (Stage 07's overlay).
; HOW: A call, then a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
HitThenByState0BFCHandlers = $BC59 ; the handler address for each state, low and high byte
HitThenByState0BFC:
    jsr State6WhenHit0BFC
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:HitThenByState0BFCHandlers,y
    sta z:zTemp00
    lda a:HitThenByState0BFCHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFC:467 State6WhenHit0BFC
; @run 70:BC67
; WHAT IT DOES: Puts the object in state 6 once it is hit (OBJ_HIT_STATUS bit 7) (Stage 07's overlay).
; HOW: A test of bit 7, a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
State6WhenHit0BFC:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State6WhenHit0BFC_at0
    lda #$06
    sta a:OBJ_STATE,x
State6WhenHit0BFC_at0:
    rts

; @routine 0BFC:4CE FacePlayerPlayList0BFC
; @run 70:BCCE
; WHAT IT DOES: Turns the object to face the player and plays the frame list Y picks (Stage 07's overlay).
; HOW: SideOfPlayer; a word table by Y; StepObjectFrames.
; WAITS: no.
; Entry: X the object; Y the frame list's word offset. Exit: A, X, Y and the flags as its last instruction or call
; left them.
FacePlayerPlayList0BFCLists = $BCE3 ; frame list addresses, low and high byte
FacePlayerPlayList0BFC:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:FacePlayerPlayList0BFCLists,y
    sta z:zTemp10
    lda a:FacePlayerPlayList0BFCLists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0BFC:619 LandThenSubstate0BFC
; @run 70:BE19
; WHAT IT DOES: Falls and lands with height 6, stepping the substate on when it lands (Stage 07's overlay).
; HOW: StopOnLanding, then INC on carry.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandThenSubstate0BFC:
    lda #$06
    jsr VecStopOnLanding
    bcc LandThenSubstate0BFC_at0
    inc a:OBJ_SUBSTATE,x
LandThenSubstate0BFC_at0:
    rts

; @routine 0BFC:63A BurstSixPieces0BFC
; @run 70:BE3A
; WHAT IT DOES: Starts six kind $3A pieces at the object's place in free slots from $30, each with its facing, and
; its own OBJ_WORK_D, OBJ_WORK_E and 16-bit X and Y speeds; stops at the first search with no slot, then removes
; the object (Stage 07's overlay).
; HOW: A loop over eight-byte records; FindFreeSlotFrom30, StartObjectAtPlace, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BurstSixPieces0BFCPieces = $BE85 ; OBJ_WORK_D, OBJ_WORK_E, X speed high, low, Y speed high, low, two spare
BurstSixPieces0BFC:
    cly
BurstSixPieces0BFC_at1:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc BurstSixPieces0BFC_at0
    lda #$3A
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    stx z:zClimbStepBefore
    lda z:zBurstCount
    asl a
    asl a
    asl a
    tax
    lda a:BurstSixPieces0BFCPieces,x
    sta a:OBJ_WORK_D,y
    lda a:BurstSixPieces0BFCPieces+1,x
    sta a:OBJ_WORK_E,y
    lda a:BurstSixPieces0BFCPieces+2,x
    sta a:OBJ_VX_HI,y
    lda a:BurstSixPieces0BFCPieces+3,x
    sta a:OBJ_VX_LO,y
    lda a:BurstSixPieces0BFCPieces+4,x
    sta a:OBJ_VY_HI,y
    lda a:BurstSixPieces0BFCPieces+5,x
    sta a:OBJ_VY_LO,y
    ldx z:zClimbStepBefore
    ldy z:zBurstCount
    iny
    cpy #$06
    bcc BurstSixPieces0BFC_at1
BurstSixPieces0BFC_at0:
    jmp VecClearObject

; @routine 0BFC:6B5 LeapAtPlayer0BFC
; @run 70:BEB5
; @stop BEF0
; WHAT IT DOES: In state 0 loads the record, moves up 16 pixels and leaps at the player: X speed by his distance in
; steps of 32 pixels, Y speed -$480, turned toward him; in later states flies on ($BF00) (Stage 07's overlay).
; HOW: A word table by DistanceXToPlayer over 32; NegateVelocityX by SideOfPlayer.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LeapAtPlayer0BFCSpeeds = $BEF0 ; the X speed by distance, low and high byte
LeapAtPlayer0BFC:
    lda a:OBJ_STATE,x
    bne ArcWithGravity24_0BFC
    jsr VecLoadObjectRecord
    lda #$F0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:LeapAtPlayer0BFCSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LeapAtPlayer0BFCSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    jsr VecSideOfPlayer
    and #$01
    beq LeapAtPlayer0BFC_at0
    jsr VecNegateVelocityX
LeapAtPlayer0BFC_at0:
    rts

; @routine 0BFC:700 ArcWithGravity24_0BFC
; @run 70:BF00
; WHAT IT DOES: Plays the frame list at $BCFF, adds $24 to its Y speed and moves (Stage 07's overlay).
; HOW: StepObjectFrames, a 16-bit add, ApplyVelocity.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
ArcWithGravity24_0BFC:
    lda #$BC
    sta z:zTemp11
    lda #$FF
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_VY_LO,x
    clc
    adc #$24
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jmp VecApplyVelocity

; @routine 0BFC:71F HopThenBecome9B_0BFC
; @run 70:BF1F
; WHAT IT DOES: In state 0 loads the record and turns its X speed toward the player; then counts OBJ_WORK_E down,
; playing the frame list OBJ_WORK_D picks and falling, and at 0 becomes kind $9B in state 0 (Stage 07's overlay).
; HOW: A word table by OBJ_WORK_D; StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
HopThenBecome9B_0BFCLists = $BF58 ; frame list addresses by OBJ_WORK_D, low and high byte
HopThenBecome9B_0BFC:
    lda a:OBJ_STATE,x
    bne HopThenBecome9B_0BFC_at0
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    beq HopThenBecome9B_0BFC_at1
    jsr VecNegateVelocityX
HopThenBecome9B_0BFC_at1:
    rts
HopThenBecome9B_0BFC_at0:
    dec a:OBJ_WORK_E,x
    bne HopThenBecome9B_0BFC_at2
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
HopThenBecome9B_0BFC_at2:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:HopThenBecome9B_0BFCLists,y
    sta z:zTemp10
    lda a:HopThenBecome9B_0BFCLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0BFC:760 FrameChecksThenByState0BFC
; @run 70:BF60
; WHAT IT DOES: Tests the player's contact, the hit, and the time-slowing skip, returning when the frame is
; skipped; then falls with its probe, sets the hit box X by frame and runs the handler of its state (Stage 07's
; overlay).
; HOW: The calls in turn, then a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FrameChecksThenByState0BFCHandlers = $BF84 ; the handler address for each state, low and high byte
FrameChecksThenByState0BFC:
    jsr StandOnBox14By20_0BFC
    jsr State4WhenHit0BFC
    jsr SkipFrameWhenSlowed0BFC
    bcc FrameChecksThenByState0BFC_at0
    rts
FrameChecksThenByState0BFC_at0:
    jsr FallProbedBelow0BFC
    jsr HitBoxXByFrame0BFD
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FrameChecksThenByState0BFCHandlers,y
    sta z:zTemp00
    lda a:FrameChecksThenByState0BFCHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFC:78E StandOnBox14By20_0BFC
; @run 70:BF8E
; WHAT IT DOES: Tests the player's contact with the object with width $14 and height $20 (Stage 07's overlay).
; HOW: zTemp08 and zTemp09, then ContactWithPlayer.
; WAITS: no.
; Entry: X the object. Exit: C set when the player stands on it.
StandOnBox14By20_0BFC:
    lda #$14
    sta z:zTemp08
    lda #$20
    sta z:zTemp09
    jmp ContactWithPlayer

; @routine 0BFC:799 State4WhenHit0BFC
; @run 70:BF99
; WHAT IT DOES: Unless the object is in state 4, once it is hit (OBJ_HIT_STATUS bit 7) restarts its animation in
; state 4 (Stage 07's overlay).
; HOW: A compare, a test of bit 7, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
State4WhenHit0BFC:
    lda a:OBJ_STATE,x
    cmp #$04
    beq State4WhenHit0BFC_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State4WhenHit0BFC_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
State4WhenHit0BFC_at0:
    rts

; @routine 0BFC:7B3 SkipFrameWhenSlowed0BFC
; @run 70:BFB3
; WHAT IT DOES: In states other than 0 and 4 with TimeSlowMode on, decides whether this frame is skipped from the
; pattern byte for zWaveCounter's low three bits, row $18 of one table in mode 2 and of the other otherwise (Stage
; 07's overlay).
; HOW: Compares of the state and the mode; a table read by $18 plus the count.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: C set when the frame is skipped, clear when the object runs.
SkipFrameWhenSlowed0BFCMode1 = $A146 ; the time-slowing pattern bytes read in the other modes; nonzero skips
SkipFrameWhenSlowed0BFCMode2 = $A166 ; the time-slowing pattern bytes read in mode 2; nonzero skips
SkipFrameWhenSlowed0BFC:
    lda a:OBJ_STATE,x
    beq SkipFrameWhenSlowed0BFC_at0
    cmp #$04
    beq SkipFrameWhenSlowed0BFC_at0
    lda a:TimeSlowMode
    beq SkipFrameWhenSlowed0BFC_at0
    cmp #$02
    bne SkipFrameWhenSlowed0BFC_at1
    lda z:zWaveCounter
    and #$07
    clc
    adc #$18
    tay
    lda a:SkipFrameWhenSlowed0BFCMode2,y
    beq SkipFrameWhenSlowed0BFC_at0
SkipFrameWhenSlowed0BFC_at2:
    sec
    rts
SkipFrameWhenSlowed0BFC_at1:
    lda z:zWaveCounter
    and #$07
    clc
    adc #$18
    tay
    lda a:SkipFrameWhenSlowed0BFCMode1,y
    bne SkipFrameWhenSlowed0BFC_at2
SkipFrameWhenSlowed0BFC_at0:
    clc
    rts

; @routine 0BFC:7E3 FallProbedBelow0BFC
; @run 70:BFE3
; WHAT IT DOES: In a state other than 0 with its Y high byte 0, moves down, falls and probes the floor ($09, $0A)
; at a point OBJ_START_ARG times 16 pixels below its place, stopping its fall on floor (Stage 07's overlay).
; HOW: Y raised by 16 times OBJ_START_ARG and lowered back around ApplyVelocityY, ApplyGravity and ProbeTerrain.
; WAITS: no.
; Entry: X the object; Y as the caller set it; D as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
FallProbedBelow0BFC:
    lda a:OBJ_STATE,x
    beq FallProbedBelow0BFC_at0
    lda a:OBJ_Y_HI,x
    bne FallProbedBelow0BFC_at0
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
    lda #$0A
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    and #$01
    beq FallProbedBelow0BFC_at1
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
FallProbedBelow0BFC_at1:
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
FallProbedBelow0BFC_at0:
    rts

; @routine 0BFD:0B1 SideStepByArg0BFD
; @run 71:C0B1
; WHAT IT DOES: Sets the object's X speed by bit 0 of OBJ_START_ARG and goes on to its one-frame offset ($C0BD)
; (Stage 07's overlay).
; HOW: A table read, then the fall into $C0BD.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SideStepByArg0BFDSpeeds = $C0D4 ; the X speed high byte for each value of OBJ_START_ARG's bit 0
SideStepByArg0BFD:
    lda a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:SideStepByArg0BFDSpeeds,y
    sta a:OBJ_VX_HI,x

; @routine 0BFD:0BD OffsetUpByArg0BFD
; @run 71:C0BD
; WHAT IT DOES: Moves the object once by its X speed and up by 16 times OBJ_START_ARG, then clears both speeds'
; high bytes (Stage 07's overlay).
; HOW: A negated shift into the Y speed; ApplyVelocity; two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
OffsetUpByArg0BFD:
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

; @routine 0BFD:0D6 RandomWaitDE0BFD
; @run 71:C0D6
; WHAT IT DOES: Sets OBJ_WORK_D:OBJ_WORK_E to one of four values chosen at random (Stage 07's overlay).
; HOW: NextRandom's low two bits index a word table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RandomWaitDE0BFDValues = $C0EA ; four values: OBJ_WORK_E, then OBJ_WORK_D
RandomWaitDE0BFD:
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:RandomWaitDE0BFDValues,y
    sta a:OBJ_WORK_E,x
    lda a:RandomWaitDE0BFDValues+1,y
    sta a:OBJ_WORK_D,x
    rts

; @routine 0BFD:0F2 RandomSpeedFG0BFD
; @run 71:C0F2
; WHAT IT DOES: Sets OBJ_WORK_F:OBJ_WORK_G to one of four values chosen at random (Stage 07's overlay).
; HOW: NextRandom's low two bits index a word table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RandomSpeedFG0BFDValues = $C106 ; four values: OBJ_WORK_G, then OBJ_WORK_F
RandomSpeedFG0BFD:
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:RandomSpeedFG0BFDValues,y
    sta a:OBJ_WORK_G,x
    lda a:RandomSpeedFG0BFDValues+1,y
    sta a:OBJ_WORK_F,x
    rts

; @routine 0BFD:16F ListByWorkHThenState1_0BFD
; @run 71:C16F
; WHAT IT DOES: Plays the frame list chosen by OBJ_WORK_H's bit 0; when it ends steps OBJ_WORK_H on and goes to
; state 1 (Stage 07's overlay).
; HOW: A word table; StepObjectFrames; stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ListByWorkHThenState1_0BFDLists = $C18E ; the two frame list addresses, low and high byte
ListByWorkHThenState1_0BFD:
    lda a:OBJ_WORK_H,x
    and #$01
    asl a
    tay
    lda a:ListByWorkHThenState1_0BFDLists,y
    sta z:zTemp10
    lda a:ListByWorkHThenState1_0BFDLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListByWorkHThenState1_0BFD_at0
    inc a:OBJ_WORK_H,x
    lda #$01
    sta a:OBJ_STATE,x
ListByWorkHThenState1_0BFD_at0:
    rts

; @routine 0BFD:1BE PickThrow0BFD
; @run 71:C1BE
; WHAT IT DOES: Goes to state 1 when the player is not on the side the object faces (OBJ_WORK_H's bit 0 flips it);
; otherwise picks OBJ_WORK_I at random from 16, OBJ_WORK_K at random from four for that pick, OBJ_WORK_J by
; OBJ_WORK_H's bit 0, and steps the substate on (Stage 07's overlay).
; HOW: SideOfPlayer compared with the facing; NextRandom twice; three tables.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
PickThrow0BFDPicks = $C206 ; the 16 OBJ_WORK_I values chosen from
PickThrow0BFDCounts = $C216 ; four OBJ_WORK_K values for each OBJ_WORK_I
PickThrow0BFDWorkJ = $C222 ; OBJ_WORK_J for each value of OBJ_WORK_H's bit 0
PickThrow0BFD:
    lda a:OBJ_FACING_LEFT,x
    eor a:OBJ_WORK_H,x
    and #$01
    sta z:zTemp08
    jsr VecSideOfPlayer
    and #$01
    cmp z:zTemp08
    beq PickThrow0BFD_at0
    lda #$01
    sta a:OBJ_STATE,x
    rts
PickThrow0BFD_at0:
    jsr VecNextRandom
    and #$0F
    tay
    lda a:PickThrow0BFDPicks,y
    sta a:OBJ_WORK_I,x
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    and #$03
    clc
    adc z:zTemp00
    tay
    lda a:PickThrow0BFDCounts,y
    sta a:OBJ_WORK_K,x
    lda a:OBJ_WORK_H,x
    and #$01
    tay
    lda a:PickThrow0BFDWorkJ,y
    sta a:OBJ_WORK_J,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0BFD:224 ThrowKindByPick0BFD
; @run 71:C224
; WHAT IT DOES: Starts the kind OBJ_WORK_I picks in a free slot from $30 at the object's place, with OBJ_WORK_D the
; pick, the object's facing, the object's slot in OBJ_WORK_F and X speed high $F8, or $EE with its facing turned
; when OBJ_WORK_J is set, and steps the substate on; with no free slot goes back to state 1 ($C2B6) (Stage 07's
; overlay).
; HOW: FindFreeSlotFrom30; a table by OBJ_WORK_I; StartObjectAtPlace; stores to slot Y.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
ThrowKindByPick0BFDKinds = $C266 ; the kind started for each OBJ_WORK_I
ThrowKindByPick0BFD:
    jsr VecFindFreeSlotFrom30
    bcs ThrowKindByPick0BFD_at0
    jmp Stand5COr5F_0BFD
ThrowKindByPick0BFD_at0:
    sty z:zBurstCount
    ldy a:OBJ_WORK_I,x
    lda a:ThrowKindByPick0BFDKinds,y
    ldy z:zBurstCount
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_I,x
    sta a:OBJ_WORK_D,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    txa
    sta a:OBJ_WORK_F,y
    lda a:OBJ_WORK_J,x
    bne ThrowKindByPick0BFD_at1
    lda #$F8
    sta a:OBJ_VX_HI,y
    bra ThrowKindByPick0BFD_at2
ThrowKindByPick0BFD_at1:
    lda #$EE
    sta a:OBJ_VX_HI,y
    lda a:OBJ_FACING_LEFT,y
    eor #$01
    sta a:OBJ_FACING_LEFT,y
ThrowKindByPick0BFD_at2:
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0BFD:269 ListByPickAndJ0BFD
; @run 71:C269
; WHAT IT DOES: Plays the frame list picked by twice OBJ_WORK_I plus OBJ_WORK_J, stepping the substate on when it
; ends (Stage 07's overlay).
; HOW: A table read by the sum; StepObjectFrames.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
ListByPickAndJ0BFDLists = $C285 ; frame list addresses, low and high byte
ListByPickAndJ0BFD:
    lda a:OBJ_WORK_I,x
    asl a
    clc
    adc a:OBJ_WORK_J,x
    tay
    lda a:ListByPickAndJ0BFDLists,y
    sta z:zTemp10
    lda a:ListByPickAndJ0BFDLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListByPickAndJ0BFD_at0
    inc a:OBJ_SUBSTATE,x
ListByPickAndJ0BFD_at0:
    rts

; @routine 0BFD:2B6 Stand5COr5F_0BFD
; @run 71:C2B6
; WHAT IT DOES: Puts the object in state 1 with frame $5C, or $5F when OBJ_WORK_J is set (Stage 07's overlay).
; HOW: Stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
Stand5COr5F_0BFD:
    lda #$01
    sta a:OBJ_STATE,x
    lda #$5C
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_J,x
    beq Stand5COr5F_0BFD_at0
    lda #$5F
    sta a:OBJ_FRAME,x
Stand5COr5F_0BFD_at0:
    rts

; @routine 0BFD:2CB NumberKind3CThenClear0BFD
; @run 71:C2CB
; WHAT IT DOES: Plays the frame list chosen by OBJ_WORK_H's bit 0; when it ends numbers each kind $3C below state 4
; in slots OBJ_WORK_M to OBJ_WORK_N (OBJ_START_ARG 0 up, OBJ_WORK_M the first of them) and removes itself (Stage
; 07's overlay).
; HOW: A word table; StepObjectFrames; a loop over the slots; ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
NumberKind3CThenClear0BFDLists = $C316 ; the two frame list addresses, low and high byte
NumberKind3CThenClear0BFD:
    lda a:OBJ_WORK_H,x
    and #$01
    asl a
    tay
    lda a:NumberKind3CThenClear0BFDLists,y
    sta z:zTemp10
    lda a:NumberKind3CThenClear0BFDLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne NumberKind3CThenClear0BFD_at0
    ldy a:OBJ_WORK_M,x
    stz z:zTemp00
    lda a:OBJ_WORK_N,x
    sta z:zTemp01
    inc z:zTemp01
NumberKind3CThenClear0BFD_at3:
    lda a:OBJ_KIND,y
    cmp #$3C
    bne NumberKind3CThenClear0BFD_at1
    lda a:OBJ_STATE,y
    cmp #$04
    bcs NumberKind3CThenClear0BFD_at1
    lda z:zTemp00
    bne NumberKind3CThenClear0BFD_at2
    sty z:zTemp03
NumberKind3CThenClear0BFD_at2:
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    lda z:zTemp03
    sta a:OBJ_WORK_M,y
    inc z:zTemp00
NumberKind3CThenClear0BFD_at1:
    iny
    cpy z:zTemp01
    bcc NumberKind3CThenClear0BFD_at3
    jsr VecClearObject
NumberKind3CThenClear0BFD_at0:
    rts

; @routine 0BFD:348 LaunchWithKind3F0BFD
; @run 71:C348
; WHAT IT DOES: Loads the record, sets OBJ_WORK_E $28, moves once by its X speed (turned by facing), takes X and Y
; speeds from a record by OBJ_WORK_D and facing, and starts kind $3F in a free slot from $50 with its OBJ_WORK_F
; (Stage 07's overlay).
; HOW: A four-byte record at eight times OBJ_WORK_D plus four times the facing; FindFreeSlotFrom50,
; StartObjectAtPlace.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
LaunchWithKind3F0BFDSpeeds = $C395 ; X speed low, high, Y speed low, high per record
LaunchWithKind3F0BFD:
    jsr VecLoadObjectRecord
    lda #$28
    sta a:OBJ_WORK_E,x
    lda a:OBJ_FACING_LEFT,x
    beq LaunchWithKind3F0BFD_at0
    jsr VecNegateVelocityX
LaunchWithKind3F0BFD_at0:
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
    lda a:LaunchWithKind3F0BFDSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LaunchWithKind3F0BFDSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:LaunchWithKind3F0BFDSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:LaunchWithKind3F0BFDSpeeds+2,y
    sta a:OBJ_VY_LO,x
    jsr VecFindFreeSlotFrom50
    bcc LaunchWithKind3F0BFD_at1
    lda #$3F
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,y
LaunchWithKind3F0BFD_at1:
    rts

; @routine 0BFD:3A5 WaitThenBoxOn0BFD
; @run 71:C3A5
; WHAT IT DOES: Counts OBJ_WORK_E down; at 0 sounds $25, turns the hit box on (2), restarts the animation and steps
; the state on; then follows its owner ($C3C0) (Stage 07's overlay).
; HOW: DEC, stores, then the call.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
WaitThenBoxOn0BFD:
    dec a:OBJ_WORK_E,x
    bne WaitThenBoxOn0BFD_at0
    lda #$25
    jsr VecSoundCommand
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
WaitThenBoxOn0BFD_at0:
    jmp FollowOwnerY0BFD

; @routine 0BFD:3C0 FollowOwnerY0BFD
; @run 71:C3C0
; WHAT IT DOES: Takes the Y low byte of the slot in OBJ_WORK_F, and removes itself once that slot is in state 4
; (Stage 07's overlay).
; HOW: A copy, a compare, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FollowOwnerY0BFD:
    ldy a:OBJ_WORK_F,x
    lda a:OBJ_Y_LO,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_STATE,y
    cmp #$04
    bne FollowOwnerY0BFD_at0
    jmp VecClearObject
FollowOwnerY0BFD_at0:
    rts

; @routine 0BFD:3D4 FlyByListWorkD0BFD
; @run 71:C3D4
; WHAT IT DOES: Plays the frame list OBJ_WORK_D picks and moves; with OBJ_WORK_D 1 its Y speed grows by $18 a frame
; (Stage 07's overlay).
; HOW: A word table; StepObjectFrames, ApplyVelocity, a 16-bit add.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FlyByListWorkD0BFDLists = $C402 ; frame list addresses by OBJ_WORK_D, low and high byte
FlyByListWorkD0BFD:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:FlyByListWorkD0BFDLists,y
    sta z:zTemp10
    lda a:FlyByListWorkD0BFDLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    cmp #$01
    bne FlyByListWorkD0BFD_at0
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
FlyByListWorkD0BFD_at0:
    rts

; @routine 0BFD:42E StartWithKind3FAtOne0BFD
; @run 71:C42E
; WHAT IT DOES: Loads the record, sets OBJ_WORK_E $40, moves once by its X speed (turned by facing), starts kind
; $3F in a free slot from $50 with OBJ_WORK_D 1 and its OBJ_WORK_F, then moves again by an X speed by its facing
; (Stage 07's overlay).
; HOW: NegateVelocityX, ApplyVelocityX, FindFreeSlotFrom50, StartObjectAtPlace; a table by the facing.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StartWithKind3FAtOne0BFDSpeeds = $C463 ; the X speed high byte for each facing
StartWithKind3FAtOne0BFD:
    jsr VecLoadObjectRecord
    lda #$40
    sta a:OBJ_WORK_E,x
    lda a:OBJ_FACING_LEFT,x
    beq StartWithKind3FAtOne0BFD_at0
    jsr VecNegateVelocityX
StartWithKind3FAtOne0BFD_at0:
    jsr VecApplyVelocityX
    jsr VecFindFreeSlotFrom50
    bcc StartWithKind3FAtOne0BFD_at1
    lda #$3F
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,y
    lda a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,y
StartWithKind3FAtOne0BFD_at1:
    ldy a:OBJ_FACING_LEFT,x
    lda a:StartWithKind3FAtOne0BFDSpeeds,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    rts

; @routine 0BFD:47D AnimateBoxedTimed0BFD
; @run 71:C47D
; WHAT IT DOES: Plays the frame list at $C494 with the hit box of each frame, and removes itself when OBJ_WORK_E
; counts down to 0 (Stage 07's overlay).
; HOW: StepObjectFrames, HitBoxByFrame0BFD, DEC, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AnimateBoxedTimed0BFD:
    lda #$C4
    sta z:zTemp11
    lda #$94
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr HitBoxByFrame0BFD
    dec a:OBJ_WORK_E,x
    bne AnimateBoxedTimed0BFD_at0
    jmp VecClearObject
AnimateBoxedTimed0BFD_at0:
    rts

; @routine 0BFD:4F5 TimedListFollowOwner0BFD
; @run 71:C4F5
; WHAT IT DOES: In state 0 loads the record, sets sprite set OBJ_WORK_D plus 3 and OBJ_WORK_E $28; then plays the
; frame list OBJ_WORK_D picks, removes itself when OBJ_WORK_E runs out, and otherwise follows its owner ($C3C0)
; (Stage 07's overlay).
; HOW: Two arms by the state; a word table; StepObjectFrames; DEC.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
TimedListFollowOwner0BFDLists = $C529 ; frame list addresses by OBJ_WORK_D, low and high byte
TimedListFollowOwner0BFD:
    lda a:OBJ_STATE,x
    bne TimedListFollowOwner0BFD_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_D,x
    clc
    adc #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$28
    sta a:OBJ_WORK_E,x
    rts
TimedListFollowOwner0BFD_at0:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:TimedListFollowOwner0BFDLists,y
    sta z:zTemp10
    lda a:TimedListFollowOwner0BFDLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_E,x
    bne TimedListFollowOwner0BFD_at1
    jmp VecClearObject
TimedListFollowOwner0BFD_at1:
    jmp FollowOwnerY0BFD

; @routine 0BFD:561 State4SoundWhenHit0BFD
; @run 71:C561
; WHAT IT DOES: Once the object is hit (OBJ_HIT_STATUS bit 7) in a state below 4, sounds $B0 and puts it in state
; 4, substate 0 (Stage 07's overlay).
; HOW: A test of bit 7, a compare, SoundCommand, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
State4SoundWhenHit0BFD:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State4SoundWhenHit0BFD_at0
    lda a:OBJ_STATE,x
    cmp #$04
    bcs State4SoundWhenHit0BFD_at0
    lda #$B0
    jsr VecSoundCommand
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State4SoundWhenHit0BFD_at0:
    rts

; @routine 0BFD:5C6 StartNineLinks0BFD
; @run 71:C5C6
; WHAT IT DOES: Loads the record, sets OBJ_WORK_O $80, fills the overlay's 128-byte buffer with $80, and starts
; kind $41 in the nine slots after the object, numbered 0 to 8 in OBJ_WORK_K (Stage 07's overlay).
; HOW: A fill loop; a loop of ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartNineLinks0BFDBuffer = $C5F7 ; 128 bytes the routine fills with $80; the links' angles are read from it
StartNineLinks0BFD:
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_WORK_O,x
    cly
StartNineLinks0BFD_at0:
    lda #$80
    sta a:StartNineLinks0BFDBuffer,y
    iny
    cpy #$80
    bcc StartNineLinks0BFD_at0
    txa
    tay
    iny
    lda #$09
    sta z:zTemp00
    stz z:zTemp01
StartNineLinks0BFD_at1:
    jsr VecClearObjectInY
    lda #$41
    jsr VecStartObjectAtPlace
    lda z:zTemp01
    sta a:OBJ_WORK_K,y
    iny
    inc z:zTemp01
    dec z:zTemp00
    bne StartNineLinks0BFD_at1
    rts

; @routine 0BFD:6CC AngleFromNinthToPlayer0BFD
; @run 71:C6CC
; WHAT IT DOES: Gives the angle to the player from the slot nine after the object (Stage 07's overlay).
; HOW: X plus 9, AngleToPlayer, X back.
; WAITS: no.
; Entry: X the object. Exit: A the angle; X the object.
AngleFromNinthToPlayer0BFD:
    stx z:zClimbStepBefore
    txa
    clc
    adc #$09
    tax
    jsr VecAngleToPlayer
    ldx z:zClimbStepBefore
    rts

; @routine 0BFD:6D9 ScaleByLinkRow0BFD
; @run 71:C6D9
; WHAT IT DOES: Scales the cosine of OBJ_WORK_K by the size in the row OBJ_WORK_M picks, and leaves that row's
; other word in zMultiplier (Stage 07's overlay).
; HOW: A four-byte row at four times OBJ_WORK_M; ScaleByCosine.
; WAITS: no.
; Entry: X the object. Exit: zProduct the scaled cosine; zMultiplierLo/Hi the row's first word; Y and zHitSlot the
; row's offset.
ScaleByLinkRow0BFDRows = $C6BC ; per row: the multiplier low, high, then the cosine's scale low, high
ScaleByLinkRow0BFD:
    lda a:OBJ_WORK_M,x
    asl a
    asl a
    tay
    sty z:zBurstCount
    lda a:ScaleByLinkRow0BFDRows+2,y
    sta z:zTemp00
    lda a:ScaleByLinkRow0BFDRows+3,y
    sta z:zTemp01
    lda a:OBJ_WORK_K,x
    jsr VecScaleByCosine
    ldy z:zBurstCount
    lda a:ScaleByLinkRow0BFDRows,y
    sta z:zSetUpColumnsLeft
    lda a:ScaleByLinkRow0BFDRows+1,y
    sta z:zTransposeRowsLeft
    rts

; @routine 0BFD:6FE NextStateOnTimer0BFD
; @run 71:C6FE
; WHAT IT DOES: Counts OBJ_WORK_O down; when it runs out reloads it with one of four waits at random, and takes the
; next state of a sixteen-step pattern by OBJ_WORK_N, substate 0, stepping OBJ_WORK_N on (Stage 07's overlay).
; HOW: NextRandom; a table by OBJ_WORK_N's low four bits indexes the states.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
NextStateOnTimer0BFDWaits = $C72A ; the four waits chosen from
NextStateOnTimer0BFDSteps = $C72E ; the state index for each of the 16 steps
NextStateOnTimer0BFDStates = $C73E ; the state for each index
NextStateOnTimer0BFD:
    lda a:OBJ_WORK_O,x
    beq NextStateOnTimer0BFD_at0
    dec a:OBJ_WORK_O,x
    rts
NextStateOnTimer0BFD_at0:
    jsr VecNextRandom
    and #$03
    tay
    lda a:NextStateOnTimer0BFDWaits,y
    sta a:OBJ_WORK_O,x
    lda a:OBJ_WORK_N,x
    and #$0F
    tay
    lda a:NextStateOnTimer0BFDSteps,y
    tay
    lda a:NextStateOnTimer0BFDStates,y
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_N,x
    rts

; @routine 0BFD:741 DragChainAndMove0BFD
; @run 71:C741
; WHAT IT DOES: Moves the chain of the eight slots after the object: each takes the place of the slot after it and
; moves by its speed; then the object takes the next slot's place, moves left by X speed high $F8, and moves at
; angle OBJ_WORK_D plus $80 at speed OBJ_WORK_F:OBJ_WORK_G (Stage 07's overlay).
; HOW: A loop down the slots of CopyObjectPlaceBack and ApplyVelocity; SetVelocityFromAngle.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
DragChainAndMove0BFD:
    stx z:zClimbStepBefore
    txa
    clc
    adc #$08
    tax
DragChainAndMove0BFD_at0:
    txa
    tay
    iny
    jsr VecCopyObjectPlaceBack
    jsr VecApplyVelocity
    dex
    cpx z:zClimbStepBefore
    bne DragChainAndMove0BFD_at0
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
    adc #$80
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 0BFD:7DA StartKindC2ThenState1_0BFD
; @run 71:C7DA
; WHAT IT DOES: Starts kind $C2 in a free slot from $30 at the object's place with the object's slot in its
; OBJ_WORK_D, and puts the object in state 1 (Stage 07's overlay).
; HOW: FindFreeSlotFrom30, StartObjectAtPlace, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StartKindC2ThenState1_0BFD:
    jsr VecFindFreeSlotFrom30
    bcc StartKindC2ThenState1_0BFD_at0
    lda #$C2
    jsr VecStartObjectAtPlace
    txa
    sta a:OBJ_WORK_D,y
StartKindC2ThenState1_0BFD_at0:
    lda #$01
    sta a:OBJ_STATE,x
    rts

; @routine 0BFE:043 TurnToZeroBy2_0BFE
; @run 71:C843
; WHAT IT DOES: Counts OBJ_WORK_I down and turns the angle OBJ_WORK_D toward 0 by 2, kept even; steps the substate
; on when it is there (Stage 07's overlay).
; HOW: DEC; TurnAngleToward; AND #$FE and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnToZeroBy2_0BFE:
    dec a:OBJ_WORK_I,x
    stz z:zTemp02
    lda #$02
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    and #$FE
    sta a:OBJ_WORK_D,x
    cmp z:zTemp02
    bne TurnToZeroBy2_0BFE_at0
    inc a:OBJ_SUBSTATE,x
TurnToZeroBy2_0BFE_at0:
    rts

; @routine 0BFE:0A5 TurnTowardWorkH0BFE
; @run 71:C8A5
; WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward OBJ_WORK_H less $80, and steps the substate on when
; OBJ_WORK_I counts down to 0 (Stage 07's overlay).
; HOW: TurnAngleToward; DEC.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
TurnTowardWorkH0BFE:
    lda a:OBJ_WORK_H,x
    sec
    sbc #$80
    sta z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    dec a:OBJ_WORK_I,x
    bne TurnTowardWorkH0BFE_at0
    inc a:OBJ_SUBSTATE,x
TurnTowardWorkH0BFE_at0:
    rts

; @routine 0BFE:0C5 TurnToZeroThenState1_0BFE
; @run 71:C8C5
; WHAT IT DOES: Turns the angle OBJ_WORK_D toward 0 by 8; once there goes to state 1, substate 0, with OBJ_WORK_E,
; OBJ_WORK_K and OBJ_WORK_L cleared (Stage 07's overlay).
; HOW: TurnAngleToward, a compare, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnToZeroThenState1_0BFE:
    stz z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    cmp z:zTemp02
    bne TurnToZeroThenState1_0BFE_at0
    lda #$01
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_K,x
    stz a:OBJ_WORK_L,x
TurnToZeroThenState1_0BFE_at0:
    rts

; @routine 0BFE:140 PieceOnCount0BFE
; @run 71:C940
; WHAT IT DOES: When OBJ_WORK_O's low five bits match one of eight counts, starts kind $9C in a free slot from $30
; with sound $15 (Stage 07's overlay).
; HOW: A search of the counts; FindFreeSlotFrom30, StartObjectAtPlace, SoundCommand.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PieceOnCount0BFECounts = $C961 ; the eight OBJ_WORK_O counts that start a piece
PieceOnCount0BFE:
    cly
PieceOnCount0BFE_at1:
    lda a:OBJ_WORK_O,x
    and #$1F
    cmp a:PieceOnCount0BFECounts,y
    beq PieceOnCount0BFE_at0
    iny
    cpy #$08
    bcc PieceOnCount0BFE_at1
    rts
PieceOnCount0BFE_at0:
    jsr VecFindFreeSlotFrom30
    bcc PieceOnCount0BFE_at2
    lda #$9C
    jsr VecStartObjectAtPlace
    lda #$15
    jsr VecSoundCommand
PieceOnCount0BFE_at2:
    rts

; @routine 0BFE:183 AimLinkFromBuffer0BFE
; @run 71:C983
; WHAT IT DOES: Unless OBJ_WORK_K is 8, takes its angle OBJ_WORK_D from the overlay's buffer at the entry for its
; number OBJ_WORK_K, eases its speed toward the target (EaseWorkFToTarget0BFD) and sets its velocity from that
; angle and speed (Stage 07's overlay).
; HOW: Two table reads; the call; SetVelocityFromAngle.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AimLinkFromBuffer0BFEBuffer = $C5F7 ; the 128-byte buffer of angles StartNineLinks0BFD fills
AimLinkFromBuffer0BFEOffsets = $C9A9 ; the buffer offset for each OBJ_WORK_K
AimLinkFromBuffer0BFE:
    lda a:OBJ_WORK_K,x
    cmp #$08
    beq AimLinkFromBuffer0BFE_at0
    tay
    lda a:AimLinkFromBuffer0BFEOffsets,y
    tay
    lda a:AimLinkFromBuffer0BFEBuffer,y
    sta a:OBJ_WORK_D,x
    jsr EaseWorkFToTarget0BFD
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
AimLinkFromBuffer0BFE_at0:
    rts

; @routine 0BFE:1B9 EaseThenBySubstate0BFE
; @run 71:C9B9
; WHAT IT DOES: Eases the speed OBJ_WORK_F:OBJ_WORK_G toward its target, then goes on to aim and run by the
; substate ($C9C6) (Stage 07's overlay).
; HOW: EaseWorkFToTarget0BFD, two loads, then the fall into $C9C6.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
EaseThenBySubstate0BFE:
    jsr EaseWorkFToTarget0BFD
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    sta z:zTemp01

; @routine 0BFE:1C6 AimThenBySubstate0BFE
; @run 71:C9C6
; WHAT IT DOES: Sets the velocity from the angle OBJ_WORK_D at the speed in zTemp00/01, then runs the handler of
; its substate (Stage 07's overlay).
; HOW: SetVelocityFromAngle, then a jump table by the substate.
; WAITS: no.
; Entry: X the object; zTemp00/01 the speed. Exit: A, X, Y and the flags as its last instruction or call left them.
AimThenBySubstate0BFEHandlers = $C9DE ; the handler address for each substate, low and high byte
AimThenBySubstate0BFE:
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:AimThenBySubstate0BFEHandlers,y
    sta z:zTemp00
    lda a:AimThenBySubstate0BFEHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFE:1FC TurnToTargetByK0BFE
; @run 71:C9FC
; WHAT IT DOES: Counts OBJ_WORK_I down and turns the angle OBJ_WORK_D by 2, kept even, toward the target OBJ_WORK_K
; picks; steps the substate on when it is there (Stage 07's overlay).
; HOW: A table read; TurnAngleToward; AND #$FE and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnToTargetByK0BFETargets = $CA20 ; the target angle for each OBJ_WORK_K
TurnToTargetByK0BFE:
    dec a:OBJ_WORK_I,x
    ldy a:OBJ_WORK_K,x
    lda a:TurnToTargetByK0BFETargets,y
    sta z:zTemp02
    lda #$02
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    and #$FE
    sta a:OBJ_WORK_D,x
    cmp z:zTemp02
    bne TurnToTargetByK0BFE_at0
    inc a:OBJ_SUBSTATE,x
TurnToTargetByK0BFE_at0:
    rts

; @routine 0BFE:23C TurnTowardHThenState1_0BFE
; @run 71:CA3C
; WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward OBJ_WORK_H; when OBJ_WORK_I counts down to 0 clears
; OBJ_WORK_J and OBJ_WORK_G and goes to state 1, substate 0 (Stage 07's overlay).
; HOW: TurnAngleToward; DEC; stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnTowardHThenState1_0BFE:
    lda a:OBJ_WORK_H,x
    sta z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    dec a:OBJ_WORK_I,x
    bne TurnTowardHThenState1_0BFE_at0
    stz a:OBJ_WORK_J,x
    stz a:OBJ_WORK_G,x
    lda #$01
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
TurnTowardHThenState1_0BFE_at0:
    rts

; @routine 0BFE:2CD LandOrClearLow0BFE
; @run 71:CACD
; WHAT IT DOES: Removes the object when its Y high byte is not 0; otherwise falls and lands with height 0, and on
; landing sets OBJ_WORK_M $20 and steps the substate on (Stage 07's overlay).
; HOW: A test of Y high; ClearObject or StopOnLanding.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandOrClearLow0BFE:
    lda a:OBJ_Y_HI,x
    beq LandOrClearLow0BFE_at0
    jmp VecClearObject
LandOrClearLow0BFE_at0:
    lda #$00
    jsr VecStopOnLanding
    bcc LandOrClearLow0BFE_at1
    lda #$20
    sta a:OBJ_WORK_M,x
    inc a:OBJ_SUBSTATE,x
LandOrClearLow0BFE_at1:
    rts

; @routine 0BFE:2E5 FlickerThenBecomeL0BFE
; @run 71:CAE5
; WHAT IT DOES: Counts OBJ_WORK_M down, showing one of two frames by its bit 0; at 0 moves up 4 pixels, sets Y
; speed -1 and becomes the kind in OBJ_WORK_L, state 0 (Stage 07's overlay).
; HOW: DEC; a table read; ApplyVelocityY; stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlickerThenBecomeL0BFEFrames = $CB11 ; the frame for each value of OBJ_WORK_M's bit 0
FlickerThenBecomeL0BFE:
    dec a:OBJ_WORK_M,x
    beq FlickerThenBecomeL0BFE_at0
    lda a:OBJ_WORK_M,x
    and #$01
    tay
    lda a:FlickerThenBecomeL0BFEFrames,y
    sta a:OBJ_FRAME,x
    rts
FlickerThenBecomeL0BFE_at0:
    lda #$FC
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$FF
    sta a:OBJ_VY_HI,x
    sta a:OBJ_VY_LO,x
    lda a:OBJ_WORK_L,x
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts

; @routine 0BFE:32D LoadRecordWorkF8E9_0BFE
; @run 71:CB2D
; WHAT IT DOES: Loads the object's record and sets OBJ_WORK_F 8 and OBJ_WORK_E 9 (Stage 07's overlay).
; HOW: LoadObjectRecord and two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordWorkF8E9_0BFE:
    jsr VecLoadObjectRecord
    lda #$08
    sta a:OBJ_WORK_F,x
    lda #$09
    sta a:OBJ_WORK_E,x
    rts

; @routine 0BFE:33B StepDownOwnerChain0BFE
; @run 71:CB3B
; WHAT IT DOES: Removes itself once the slot in OBJ_WORK_D is in state 4; otherwise takes the place of the slot
; OBJ_WORK_E after that one, stepping OBJ_WORK_E down every 8 frames (OBJ_WORK_F) and the state on when it reaches
; 0 (Stage 07's overlay).
; HOW: A compare; CopyObjectPlaceBack; two counters.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StepDownOwnerChain0BFE:
    ldy a:OBJ_WORK_D,x
    lda a:OBJ_STATE,y
    cmp #$04
    bne StepDownOwnerChain0BFE_at0
    jmp VecClearObject
StepDownOwnerChain0BFE_at0:
    lda a:OBJ_WORK_D,x
    clc
    adc a:OBJ_WORK_E,x
    tay
    jsr VecCopyObjectPlaceBack
    dec a:OBJ_WORK_F,x
    bne StepDownOwnerChain0BFE_at1
    lda #$08
    sta a:OBJ_WORK_F,x
    dec a:OBJ_WORK_E,x
    bne StepDownOwnerChain0BFE_at1
    inc a:OBJ_STATE,x
StepDownOwnerChain0BFE_at1:
    rts

; @routine 0BFE:366 ShotFromOwner0BFE
; @run 71:CB66
; WHAT IT DOES: Plays the frame list at $CBBC and sits at its owner's place (OBJ_WORK_D), removing itself if the
; owner is in state 4; it plays the list at $CBC3 on the owner, and when that ends fires at the player at speed
; $3C0 with its hit box on (2) and its state on, or removes itself unless the angle is $40 to $BF (Stage 07's
; overlay).
; HOW: StepObjectFrames on both slots; CopyObjectPlaceBack; AngleToPlayer, two compares, SetVelocityFromAngle.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ShotFromOwner0BFE:
    lda #$BC
    sta z:zTemp10
    lda #$CB
    sta z:zTemp11
    jsr VecStepObjectFrames
    ldy a:OBJ_WORK_D,x
    lda a:OBJ_STATE,y
    cmp #$04
    bne ShotFromOwner0BFE_at0
    jmp VecClearObject
ShotFromOwner0BFE_at0:
    jsr VecCopyObjectPlaceBack
    stx z:zClimbStepBefore
    sxy
    lda #$C3
    sta z:zTemp10
    lda #$CB
    sta z:zTemp11
    jsr VecStepObjectFrames
    php
    ldx z:zClimbStepBefore
    plp
    bne ShotFromOwner0BFE_at1
    jsr VecAngleToPlayer
    cmp #$40
    bcs ShotFromOwner0BFE_at2
    jmp VecClearObject
ShotFromOwner0BFE_at2:
    cmp #$C0
    bcc ShotFromOwner0BFE_at3
    jmp VecClearObject
ShotFromOwner0BFE_at3:
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
ShotFromOwner0BFE_at1:
    rts

; @routine 0BFE:3F2 FlyUntilOutside0BFE
; @run 71:CBF2
; WHAT IT DOES: Removes the object when its X or Y high byte is 2 or $FE; otherwise plays the frame list at $CBBC
; and moves (Stage 07's overlay).
; HOW: Four compares, StepObjectFrames, ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlyUntilOutside0BFE:
    lda a:OBJ_X_HI,x
    cmp #$02
    bne FlyUntilOutside0BFE_at0
    jmp VecClearObject
FlyUntilOutside0BFE_at0:
    cmp #$FE
    bne FlyUntilOutside0BFE_at1
    jmp VecClearObject
FlyUntilOutside0BFE_at1:
    lda a:OBJ_Y_HI,x
    cmp #$02
    bne FlyUntilOutside0BFE_at2
    jmp VecClearObject
FlyUntilOutside0BFE_at2:
    cmp #$FE
    bne FlyUntilOutside0BFE_at3
    jmp VecClearObject
FlyUntilOutside0BFE_at3:
    lda #$BC
    sta z:zTemp10
    lda #$CB
    sta z:zTemp11
    jsr VecStepObjectFrames
    jmp VecApplyVelocity

; @routine 0BFE:44B FacePlayerInStates0BFE
; @run 71:CC4B
; WHAT IT DOES: In the states a table marks, turns the object to face the player (Stage 07's overlay).
; HOW: A table read by the state; SideOfPlayer.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FacePlayerInStates0BFEStates = $CC5C ; nonzero for the states in which it faces the player
FacePlayerInStates0BFE:
    ldy a:OBJ_STATE,x
    lda a:FacePlayerInStates0BFEStates,y
    beq FacePlayerInStates0BFE_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FacePlayerInStates0BFE_at0:
    rts

; @routine 0BFE:464 State6SoundWhenHit0BFE
; @run 71:CC64
; WHAT IT DOES: Once the object is hit (OBJ_HIT_STATUS bit 7) in a state below 6, sounds $13 and restarts its
; animation in state 6, substate 0 (Stage 07's overlay).
; HOW: A test of bit 7, a compare, SoundCommand, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
State6SoundWhenHit0BFE:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State6SoundWhenHit0BFE_at0
    lda a:OBJ_STATE,x
    cmp #$06
    bcs State6SoundWhenHit0BFE_at0
    lda #$13
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State6SoundWhenHit0BFE_at0:
    rts

; @routine 0BFE:486 StartKind47Next0BFE
; @run 71:CC86
; WHAT IT DOES: Loads the record, starts kind $47 in the slot after the object (cleared first) and sets OBJ_WORK_H
; $20 (Stage 07's overlay).
; HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace, a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartKind47Next0BFE:
    jsr VecLoadObjectRecord
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$47
    jsr VecStartObjectAtPlace
    lda #$20
    sta a:OBJ_WORK_H,x
    rts

; @routine 0BFE:49A LandThenNearPlayer0BFE
; @run 71:CC9A
; WHAT IT DOES: Falls and lands with height $0C, and steps the state on once the player is less than $60 away in X
; (Stage 07's overlay).
; HOW: StopOnLanding, DistanceXToPlayer, a compare.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandThenNearPlayer0BFE:
    lda #$0C
    jsr VecStopOnLanding
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs LandThenNearPlayer0BFE_at0
    inc a:OBJ_STATE,x
LandThenNearPlayer0BFE_at0:
    rts

; @routine 0BFE:4AA ListThenBoxOn0BFE
; @run 71:CCAA
; WHAT IT DOES: Plays the frame list at $CCC0; when it ends turns the hit box on (2) and steps the state on (Stage
; 07's overlay).
; HOW: StepObjectFrames, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ListThenBoxOn0BFE:
    lda #$C0
    sta z:zTemp10
    lda #$CC
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenBoxOn0BFE_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
ListThenBoxOn0BFE_at0:
    rts

; @routine 0BFE:4E2 StepWhileLanded0BFE
; @run 71:CCE2
; WHAT IT DOES: Plays the frame list at $CD06, taking an X speed by OBJ_WORK_E when the list's step reports carry,
; falls and lands with height $0C, then clears the X speed (Stage 07's overlay).
; HOW: StepObjectFrames, a table read, StopOnLanding, two stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
StepWhileLanded0BFESpeeds = $CD04 ; the X speed high byte for each OBJ_WORK_E
StepWhileLanded0BFE:
    lda #$06
    sta z:zTemp10
    lda #$CD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc StepWhileLanded0BFE_at0
    ldy a:OBJ_WORK_E,x
    lda a:StepWhileLanded0BFESpeeds,y
    sta a:OBJ_VX_HI,x
StepWhileLanded0BFE_at0:
    lda #$0C
    jsr VecStopOnLanding
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    rts

; @routine 0BFE:5FB SlideByFacing0BFE
; @run 71:CDFB
; WHAT IT DOES: Sets the X speed by the object's facing and falls and lands with height $0C (Stage 07's overlay).
; HOW: A word table by the facing; StopOnLanding.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SlideByFacing0BFESpeeds = $CDF7 ; the X speed for each facing, low and high byte
SlideByFacing0BFE:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:SlideByFacing0BFESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:SlideByFacing0BFESpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$0C
    jsr VecStopOnLanding
    rts

; @routine 0BFE:634 SlideAndSlow0BFE
; @run 71:CE34
; WHAT IT DOES: Moves at the X speed OBJ_WORK_I:OBJ_WORK_J, falling and landing with height $0C, and while that
; speed is not 0 adds a step by facing to it (Stage 07's overlay).
; HOW: StopOnLanding; a 16-bit add from a word table by the facing.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it; D as the caller left it. Exit: A, X, Y and
; the flags as its last instruction or call left them.
SlideAndSlow0BFESteps = $CE66 ; the speed step for each facing, low and high byte
SlideAndSlow0BFE:
    lda a:OBJ_WORK_I,x
    sta a:OBJ_VX_HI,x
    lda a:OBJ_WORK_J,x
    sta a:OBJ_VX_LO,x
    lda #$0C
    jsr VecStopOnLanding
    lda a:OBJ_WORK_I,x
    ora a:OBJ_WORK_J,x
    beq SlideAndSlow0BFE_at0
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:OBJ_WORK_J,x
    clc
    adc a:SlideAndSlow0BFESteps,y
    sta a:OBJ_WORK_J,x
    lda a:OBJ_WORK_I,x
    adc a:SlideAndSlow0BFESteps+1,y
    sta a:OBJ_WORK_I,x
SlideAndSlow0BFE_at0:
    rts

; @routine 0BFE:690 JumpAtPlayer0BFE
; @run 71:CE90
; WHAT IT DOES: Sets X and Y speeds by the player's distance in steps of 32 pixels, frame $89, and steps the
; substate on, turning the X speed when facing left (Stage 07's overlay).
; HOW: A four-byte record by DistanceXToPlayer over 32; NegateVelocityX.
; WAITS: no.
; Entry: X the object; A as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
JumpAtPlayer0BFESpeeds = $CEC4 ; X speed low, high, Y speed low, high by distance
JumpAtPlayer0BFE:
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    asl a
    tay
    lda a:JumpAtPlayer0BFESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:JumpAtPlayer0BFESpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:JumpAtPlayer0BFESpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:JumpAtPlayer0BFESpeeds+2,y
    sta a:OBJ_VY_LO,x
    lda #$89
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_FACING_LEFT,x
    beq JumpAtPlayer0BFE_at0
    jsr VecNegateVelocityX
JumpAtPlayer0BFE_at0:
    rts

; @routine 0BFE:6E4 FrameBySignLand0BFE
; @run 71:CEE4
; WHAT IT DOES: Shows a frame by whether it is rising or falling, falls and lands with height $0C on its front
; column, and steps the substate on when it lands (Stage 07's overlay).
; HOW: The Y speed's sign bit indexes the frames; StopOnLandingFrontColumn.
; WAITS: no.
; Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FrameBySignLand0BFEFrames = $CEFD ; the frame falling, then rising
FrameBySignLand0BFE:
    lda a:OBJ_VY_HI,x
    rol a
    rol a
    and #$01
    tay
    lda a:FrameBySignLand0BFEFrames,y
    sta a:OBJ_FRAME,x
    lda #$0C
    jsr VecStopOnLandingFrontColumn
    bcc FrameBySignLand0BFE_at0
    inc a:OBJ_SUBSTATE,x
FrameBySignLand0BFE_at0:
    rts

; @routine 0BFE:6FF ListThenState3_0BFE
; @run 71:CEFF
; WHAT IT DOES: Plays the frame list at $CF1B; when it ends restarts the animation in state 3, substate 0 (Stage
; 07's overlay).
; HOW: StepObjectFrames, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ListThenState3_0BFE:
    lda #$1B
    sta z:zTemp10
    lda #$CF
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenState3_0BFE_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListThenState3_0BFE_at0:
    rts

; @routine 0BFE:724 HopAtPlayer0BFE
; @run 71:CF24
; WHAT IT DOES: Sets an X speed by the player's distance in steps of 32 pixels, Y speed high $FB, frame $89, and
; steps the substate on, turning the X speed when facing left (Stage 07's overlay).
; HOW: A word table by DistanceXToPlayer over 32; NegateVelocityX.
; WAITS: no.
; Entry: X the object; A as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HopAtPlayer0BFESpeeds = $CF50 ; the X speed by distance, low and high byte
HopAtPlayer0BFE:
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:HopAtPlayer0BFESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:HopAtPlayer0BFESpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    lda #$89
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_FACING_LEFT,x
    beq HopAtPlayer0BFE_at0
    jsr VecNegateVelocityX
HopAtPlayer0BFE_at0:
    rts

; @routine 0BFE:760 LandFrontColumn0BFE
; @run 71:CF60
; WHAT IT DOES: Falls and lands with height $0C on its front column, stepping the substate on when it lands (Stage
; 07's overlay).
; HOW: StopOnLandingFrontColumn, then INC on carry.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandFrontColumn0BFE:
    lda #$0C
    jsr VecStopOnLandingFrontColumn
    bcc LandFrontColumn0BFE_at0
    inc a:OBJ_SUBSTATE,x
LandFrontColumn0BFE_at0:
    rts

; @routine 0BFE:791 LandListThenWakeNext0BFE
; @run 71:CF91
; WHAT IT DOES: Falls and lands with height $0C and plays the frame list at $CFBC; when it ends turns the slot
; after into kind $9C, state 0, sets sprite attribute bit 4, OBJ_WORK_D $70 and steps the substate on (Stage 07's
; overlay).
; HOW: StopOnLanding, StepObjectFrames, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandListThenWakeNext0BFE:
    lda #$0C
    jsr VecStopOnLanding
    lda #$BC
    sta z:zTemp10
    lda #$CF
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LandListThenWakeNext0BFE_at0
    lda #$9C
    sta a:OBJ_KIND+1,x
    stz a:OBJ_STATE+1,x
    lda a:OBJ_SPRITE_ATTR,x
    ora #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda #$70
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
LandListThenWakeNext0BFE_at0:
    rts

; @routine 0BFE:7CD TurnDownTimed0BFE
; @run 71:CFCD
; WHAT IT DOES: Counts OBJ_WORK_D down, removing itself at 0; moves at angle OBJ_WORK_K at speed $120, turning it
; by -2 a frame, and at $E0 sets it to $40 and steps the substate on (Stage 07's overlay).
; HOW: DEC; SetVelocityFromAngle, ApplyVelocity; a subtract and a compare.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
TurnDownTimed0BFE:
    dec a:OBJ_WORK_D,x
    bne TurnDownTimed0BFE_at0
    jmp VecClearObject
TurnDownTimed0BFE_at0:
    lda #$20
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_K,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_K,x
    sec
    sbc #$02
    sta a:OBJ_WORK_K,x
    cmp #$E0
    bne TurnDownTimed0BFE_at1
    lda #$40
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
TurnDownTimed0BFE_at1:
    rts

; @routine 0BFE:7FC TurnUpTimed0BFE
; @run 71:CFFC
; WHAT IT DOES: Counts OBJ_WORK_D down, removing itself at 0; moves at angle OBJ_WORK_K at speed $120, turning it
; by 2 a frame, and at $A0 sets it to $40 and steps the substate back (Stage 07's overlay).
; HOW: DEC; SetVelocityFromAngle, ApplyVelocity; an add and a compare.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
TurnUpTimed0BFE:
    dec a:OBJ_WORK_D,x
    bne TurnUpTimed0BFE_at0
    jmp VecClearObject
TurnUpTimed0BFE_at0:
    lda #$20
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_WORK_K,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_K,x
    clc
    adc #$02
    sta a:OBJ_WORK_K,x
    cmp #$A0
    bne TurnUpTimed0BFE_at1
    lda #$40
    sta a:OBJ_WORK_K,x
    dec a:OBJ_SUBSTATE,x
TurnUpTimed0BFE_at1:
    rts

; @routine 0BFF:02B FollowPrevBoxByFrame0BFF
; @run 71:D02B
; WHAT IT DOES: In state 0 loads the record; then takes the place and facing of the slot before it and gives itself
; the hit box of that slot's frame $86, $8A or $8B, or no box for any other frame (Stage 07's overlay).
; HOW: CopyObjectPlaceBack; three compares pick a four-byte row.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FollowPrevBoxByFrame0BFFBoxes = $D077 ; X offset, Y offset, half width, half height per row
FollowPrevBoxByFrame0BFF:
    lda a:OBJ_STATE,x
    bne FollowPrevBoxByFrame0BFF_at0
    jmp VecLoadObjectRecord
FollowPrevBoxByFrame0BFF_at0:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FACING_LEFT-1,x
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_FRAME-1,x
    cmp #$86
    beq FollowPrevBoxByFrame0BFF_at1
    cmp #$8A
    beq FollowPrevBoxByFrame0BFF_at2
    cmp #$8B
    beq FollowPrevBoxByFrame0BFF_at3
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_HALF_H,x
    rts
FollowPrevBoxByFrame0BFF_at1:
    cly
    bra FollowPrevBoxByFrame0BFF_at4
FollowPrevBoxByFrame0BFF_at2:
    ldy #$04
    bra FollowPrevBoxByFrame0BFF_at4
FollowPrevBoxByFrame0BFF_at3:
    ldy #$08
FollowPrevBoxByFrame0BFF_at4:
    lda a:FollowPrevBoxByFrame0BFFBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:FollowPrevBoxByFrame0BFFBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:FollowPrevBoxByFrame0BFFBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:FollowPrevBoxByFrame0BFFBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0BFF:0AE LoadRecordDrawTiles09_0BFF
; @run 71:D0AE
; WHAT IT DOES: Loads the object's record and draws item 9 of tile script group 0 (Stage 07's overlay).
; HOW: LoadObjectRecord, then DrawTileScript.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordDrawTiles09_0BFF:
    jsr VecLoadObjectRecord
    lda #$09
    cly
    jmp VecDrawTileScript

; @routine 0BFF:0B7 BreakWallOnHit0BFF
; @run 71:D0B7
; WHAT IT DOES: With its X high byte not 0 draws item 9 of tile script group 0; otherwise on a hit of exactly 5
; draws item $0A, sounds $19, starts three debris bursts (16 pixels up, then down 16 and right 4, then down 16 and
; left 8), records its event and removes itself (Stage 07's overlay).
; HOW: DrawTileScript; ApplyVelocityY and ApplyVelocity around StartDebrisBurst; RecordObjectEvent, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BreakWallOnHit0BFF:
    lda a:OBJ_X_HI,x
    beq BreakWallOnHit0BFF_at0
    lda #$09
    cly
    jmp VecDrawTileScript
BreakWallOnHit0BFF_at0:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bne BreakWallOnHit0BFF_at1
    lda #$0A
    cly
    jsr VecDrawTileScript
    lda #$19
    jsr VecSoundCommand
    lda #$F0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$01
    jsr StartDebrisBurst
    lda #$04
    sta a:OBJ_VX_HI,x
    lda #$10
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda #$01
    jsr StartDebrisBurst
    lda #$F8
    sta a:OBJ_VX_HI,x
    lda #$10
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda #$01
    jsr StartDebrisBurst
    jsr VecRecordObjectEvent
    jmp VecClearObject
BreakWallOnHit0BFF_at1:
    rts

; @routine 0BFF:115 TileSwapOnHit0BFF
; @run 71:D115
; WHAT IT DOES: In state 0 loads the record, sets the player's hit box Y offset to 4 and sets bit 7 of
; OBJ_HITBOX_ON; then, on a hit below 3, sets OBJ_WORK_D 2 and a frame by OBJ_START_ARG, draws tile item $26 plus
; OBJ_START_ARG the next frame, and once the hits stop draws item $1A plus OBJ_START_ARG with frame 0 (Stage 07's
; overlay).
; HOW: A three-value OBJ_WORK_D; compares of OBJ_HIT_TAKEN; DrawTileScript.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
TileSwapOnHit0BFFFrames = $D175 ; the frame shown while hit for each OBJ_START_ARG
TileSwapOnHit0BFF:
    lda a:OBJ_STATE,x
    bne TileSwapOnHit0BFF_at0
    jsr VecLoadObjectRecord
    lda #$04
    sta a:OBJ_HITBOX_DY
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_HITBOX_ON,x
    ora #$80
    sta a:OBJ_HITBOX_ON,x
    rts
TileSwapOnHit0BFF_at0:
    lda a:OBJ_WORK_D,x
    cmp #$02
    bne TileSwapOnHit0BFF_at1
    dec a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    clc
    adc #$26
    cly
    jsr VecDrawTileScript
TileSwapOnHit0BFF_at1:
    lda a:OBJ_HIT_TAKEN,x
    beq TileSwapOnHit0BFF_at2
    cmp #$03
    bcs TileSwapOnHit0BFF_at2
    lda a:OBJ_WORK_D,x
    bne TileSwapOnHit0BFF_at3
    lda #$02
    sta a:OBJ_WORK_D,x
    ldy a:OBJ_START_ARG,x
    lda a:TileSwapOnHit0BFFFrames,y
    sta a:OBJ_FRAME,x
    rts
TileSwapOnHit0BFF_at2:
    lda a:OBJ_WORK_D,x
    beq TileSwapOnHit0BFF_at3
    stz a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    clc
    adc #$1A
    cly
    jsr VecDrawTileScript
    stz a:OBJ_FRAME,x
TileSwapOnHit0BFF_at3:
    rts

; @routine 0BFF:181 FlipPlayerByScroll0BFF
; @run 71:D181
; WHAT IT DOES: In state 0 loads the record; then sets the player's sprite flip to $80, or to 0 when OBJ_START_ARG
; is set and the scroll is at screen 3 or more or its X low byte is $C8 or more (Stage 07's overlay).
; HOW: Compares of the scroll; a store to slot 0.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlipPlayerByScroll0BFF:
    lda a:OBJ_STATE,x
    bne FlipPlayerByScroll0BFF_at0
    jmp VecLoadObjectRecord
FlipPlayerByScroll0BFF_at0:
    ldy #$80
    lda a:OBJ_START_ARG,x
    beq FlipPlayerByScroll0BFF_at1
    lda a:ScrollXHiScreen
    cmp #$03
    bcs FlipPlayerByScroll0BFF_at2
    lda a:ScrollXLo
    cmp #$C8
    bcc FlipPlayerByScroll0BFF_at1
FlipPlayerByScroll0BFF_at2:
    cly
FlipPlayerByScroll0BFF_at1:
    sty a:OBJ_SPRITE_FLIP
    rts

; @routine 0BFF:1D4 GrabAndSinkPlayer0BFF
; @run 71:D1D4
; WHAT IT DOES: In state 0 loads the record and faces by OBJ_START_ARG's bit 0; then plays the frame list at $D25D:
; in state 1 a hit below 3 sounds $AE and steps the state on; in state 2 clears the slot in OBJ_START_ARG, takes
; the player's hit box off, gives both Y speed $180 and starts kind $A0 there at the player's place ($D220); later
; states sink with the player ($D22F) (Stage 07's overlay).
; HOW: Arms by the state; StepObjectFrames; ClearObjectInY; the falls into $D220.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
GrabAndSinkPlayer0BFF:
    lda a:OBJ_STATE,x
    bne GrabAndSinkPlayer0BFF_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    and #$01
    sta a:OBJ_FACING_LEFT,x
    rts
GrabAndSinkPlayer0BFF_at0:
    pha
    lda #$D2
    sta z:zTemp11
    lda #$5D
    sta z:zTemp10
    jsr VecStepObjectFrames
    pla
    dec a
    bne GrabAndSinkPlayer0BFF_at1
    lda a:OBJ_HIT_TAKEN,x
    beq JustReturn0BFF_22E
    cmp #$03
    bcs JustReturn0BFF_22E
    lda #$AE
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
    rts
GrabAndSinkPlayer0BFF_at1:
    dec a
    bne SinkWithPlayer0BFF
    ldy a:OBJ_START_ARG,x
    jsr VecClearObjectInY
    stz a:OBJ_HITBOX_ON
    lda #$01
    sta a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,y
    lda #$80
    sta a:OBJ_VY_LO,x

; @routine 0BFF:220 StartKindA0AtPlayer0BFF
; @run 71:D220
; WHAT IT DOES: Sets slot Y's Y speed low byte to A, starts kind $A0 in slot Y at the player's place and steps the
; object's state on (Stage 07's overlay).
; HOW: A store, StartObjectAtPlace with X 0, INC.
; WAITS: no.
; Entry: X the object; Y the slot; A the Y speed low byte. Exit: A, X, Y and the flags as its last instruction or
; call left them.
StartKindA0AtPlayer0BFF:
    sta a:OBJ_VY_LO,y
    phx
    clx
    lda #$A0
    jsr VecStartObjectAtPlace
    plx
    inc a:OBJ_STATE,x

; @routine 0BFF:22E JustReturn0BFF_22E
; @run 71:D22E
; WHAT IT DOES: Returns at once (Stage 07's overlay).
; HOW: RTS.
; WAITS: no.
; Entry: nothing.. Exit: A, X, Y and the flags as its last instruction or call left them.
JustReturn0BFF_22E:
    rts

; @routine 0BFF:22F SinkWithPlayer0BFF
; @run 71:D22F
; WHAT IT DOES: Moves the object and the slot in its OBJ_START_ARG by their Y speeds, sets the player's OBJ_WORK_E
; $10 and OBJ_WORK_F 0, hands his frame, sprite set and facing to that slot and clears his frame, then puts him at
; the object's place (Stage 07's overlay).
; HOW: ApplyVelocityY on both slots; copies from slot 0; CopyObjectPlace to slot 0.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SinkWithPlayer0BFF:
    jsr VecApplyVelocityY
    ldy a:OBJ_START_ARG,x
    sxy
    jsr VecApplyVelocityY
    sxy
    lda #$10
    sta a:OBJ_WORK_E
    stz a:OBJ_WORK_F
    lda a:OBJ_FRAME
    beq SinkWithPlayer0BFF_at0
    sta a:OBJ_FRAME,y
    lda a:OBJ_SPRITE_SET
    sta a:OBJ_SPRITE_SET,y
SinkWithPlayer0BFF_at0:
    lda a:OBJ_FACING_LEFT
    sta a:OBJ_FACING_LEFT,y
    stz a:OBJ_FRAME
    cly
    jmp VecCopyObjectPlace

; @routine 0BFF:266 LoadOrPlayList0BFF
; @run 71:D266
; WHAT IT DOES: In state 0 loads the object's record; in later states plays the frame list at $D279 (Stage 07's
; overlay).
; HOW: A test of the state; LoadObjectRecord or StepObjectFrames.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadOrPlayList0BFF:
    lda a:OBJ_STATE,x
    bne LoadOrPlayList0BFF_at0
    jmp VecLoadObjectRecord
LoadOrPlayList0BFF_at0:
    lda #$D2
    sta z:zTemp11
    lda #$79
    sta z:zTemp10
    jmp VecStepObjectFrames

; @routine 0BFF:307 ClearCarry0BFF
; @run 71:D307
; WHAT IT DOES: Returns with the carry clear (Stage 07's overlay).
; HOW: CLC.
; WAITS: no.
; Entry: nothing.. Exit: C clear; A, X, Y and the other flags kept.
ClearCarry0BFF:
    clc
    rts

; @routine 0BFF:309 StandOnUnlessSide2_0BFF
; @run 71:D309
; WHAT IT DOES: Tests the player's contact with the object (width $11, height $1E), unless SideOfPlayer reports bit
; 1 (Stage 07's overlay).
; HOW: SideOfPlayer, then ContactWithPlayer or CLC.
; WAITS: no.
; Entry: X the object. Exit: C set when the player stands on it.
StandOnUnlessSide2_0BFF:
    jsr VecSideOfPlayer
    and #$02
    bne ClearCarry0BFF
    lda #$11
    sta z:zTemp08
    lda #$1E
    sta z:zTemp09
    jmp ContactWithPlayer

; @routine 0BFF:31B OpenWhenPartnerReady0BFF
; @run 71:D31B
; WHAT IT DOES: In state 0 loads the record and, when zField8F bit 7 is set, goes straight to its open place (frame
; $1E, $D369); in state 1 waits for the slot in OBJ_START_ARG to reach state 2, then sets zField8F bit 7 and steps
; the state on; later states run on at $D343 (Stage 07's overlay).
; HOW: Arms by the state; an OR into zField8F.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
OpenWhenPartnerReady0BFF:
    lda a:OBJ_STATE,x
    bne OpenWhenPartnerReady0BFF_at0
    jsr VecLoadObjectRecord
    bbr7 zField8F,JustReturn0BFF_342
    lda #$1E
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    bra SetRow7EAndDraw0BFF
OpenWhenPartnerReady0BFF_at0:
    dec a
    bne DropToRow7E0BFF
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_STATE,y
    cmp #$02
    bne JustReturn0BFF_342
    smb7 z:zField8F
    inc a:OBJ_STATE,x

; @routine 0BFF:342 JustReturn0BFF_342
; @run 71:D342
; WHAT IT DOES: Returns at once (Stage 07's overlay).
; HOW: RTS.
; WAITS: no.
; Entry: nothing.. Exit: A, X, Y and the flags as its last instruction or call left them.
JustReturn0BFF_342:
    rts

; @routine 0BFF:343 DropToRow7E0BFF
; @run 71:D343
; WHAT IT DOES: In state 2 plays the frame list at $D391 and falls, its Y speed growing by $28 a frame, until its Y
; low byte reaches $7E ($D369); in later states checks its tiles ($D377) (Stage 07's overlay).
; HOW: DEC of the state in A; StepObjectFrames, a 16-bit add, ApplyVelocity, a compare.
; WAITS: no.
; Entry: X the object; A the state less 1. Exit: A, X, Y and the flags as its last instruction or call left them.
DropToRow7E0BFF:
    dec a
    bne RedrawIfTileChanged0BFF
    lda #$D3
    sta z:zTemp11
    lda #$91
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_VY_LO,x
    clc
    adc #$28
    sta a:OBJ_VY_LO,x
    bcc DropToRow7E0BFF_at0
    inc a:OBJ_VY_HI,x
DropToRow7E0BFF_at0:
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$7E
    bcc JustReturn0BFF_342

; @routine 0BFF:369 SetRow7EAndDraw0BFF
; @run 71:D369
; WHAT IT DOES: Sets Y low $7E, steps the state on and draws item $0D of tile script group 0 (Stage 07's overlay).
; HOW: Stores, then the fall into $D371.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetRow7EAndDraw0BFF:
    lda #$7E
    sta a:OBJ_Y_LO,x
    inc a:OBJ_STATE,x

; @routine 0BFF:371 DrawTiles0D0BFF
; @run 71:D371
; WHAT IT DOES: Draws item $0D of tile script group 0 (Stage 07's overlay).
; HOW: DrawTileScript.
; WAITS: no.
; Entry: nothing.. Exit: A, X, Y and the flags as its last instruction or call left them.
DrawTiles0D0BFF:
    lda #$0D
    cly
    jmp VecDrawTileScript

; @routine 0BFF:377 RedrawIfTileChanged0BFF
; @run 71:D377
; WHAT IT DOES: With the scroll at screen 0, reads the VRAM word at $0394 and, unless its low byte is $D0, draws
; item $0D of tile script group 0 (Stage 07's overlay).
; HOW: SelectVramReadAddress, SelectVramData, a compare.
; WAITS: no.
; Entry: nothing.. Exit: A, X, Y and the flags as its last instruction or call left them.
RedrawIfTileChanged0BFF:
    lda a:ScrollXHiScreen
    bne JustReturn0BFF_342
    jsr VecSelectVramReadAddress
    st1 #$94
    st2 #$03
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    ldy a:VDC_DATA_HI
    cmp #$D0
    bne DrawTiles0D0BFF
    rts

; @routine 0BFF:39A AwaitHitThenList0BFF
; @run 71:D39A
; WHAT IT DOES: In state 0 loads the record and, when zField8F bit 7 is set, takes frame $21 and goes to state 2;
; in state 1 a hit of 5 or more steps the state on; in state 2 plays the frame list at $D3D3 and steps the state on
; when it ends (Stage 07's overlay).
; HOW: Arms by the state; StepObjectFrames.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AwaitHitThenList0BFF:
    lda a:OBJ_STATE,x
    bne AwaitHitThenList0BFF_at0
    jsr VecLoadObjectRecord
    bbr7 zField8F,AwaitHitThenList0BFF_at1
    lda #$21
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    inc a:OBJ_STATE,x
    rts
AwaitHitThenList0BFF_at0:
    dec a
    bne AwaitHitThenList0BFF_at2
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc JustReturn0BFF_342
    inc a:OBJ_STATE,x
AwaitHitThenList0BFF_at1:
    rts
AwaitHitThenList0BFF_at2:
    dec a
    bne AwaitHitThenList0BFF_at1
    lda #$D3
    sta z:zTemp11
    lda #$D3
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne AwaitHitThenList0BFF_at1
    inc a:OBJ_STATE,x
    rts

; @routine 0BFF:42A CheckTileAndPartner0BFF
; @run 71:D42A
; WHAT IT DOES: Reads the VRAM word at $0418: unless its low byte is $19, reports whether the slot in OBJ_START_ARG
; is empty (Stage 07's overlay).
; HOW: SelectVramReadAddress, SelectVramData, a compare; a test of the slot's kind.
; WAITS: no.
; Entry: X the object. Exit: C set when the tile is not $19 and the slot's kind is 0; otherwise clear.
CheckTileAndPartner0BFF:
    jsr VecSelectVramReadAddress
    st1 #$18
    st2 #$04
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    ldy a:VDC_DATA_HI
    cmp #$19
    beq CheckTileAndPartner0BFF_at0
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_KIND,y
    bne CheckTileAndPartner0BFF_at0
    sec
    rts
CheckTileAndPartner0BFF_at0:
    clc
    rts

; @routine 0BFF:471 LoadRecordScreenY0BFF
; @run 71:D471
; WHAT IT DOES: Loads the object's record and takes the scroll's Y low byte off its Y low byte (Stage 07's
; overlay).
; HOW: LoadObjectRecord, a negate and an add.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
LoadRecordScreenY0BFF:
    jsr VecLoadObjectRecord
    lda a:ScrollYLo
    eor #$FF
    inc a
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    rts

; @routine 0BFF:482 SecretWallOnHit0BFF
; @run 71:D482
; WHAT IT DOES: With PlayerDoorReady set and a hit below 3, when the VRAM word at $05BE has low byte $17, steps
; zField8C on, sets zSubStep 3, starts list channel $85 on channel 6 and steps the state on (Stage 07's overlay).
; HOW: Compares; SelectVramReadAddress, SelectVramData; StartListChannelKind2.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SecretWallOnHit0BFF:
    lda a:PlayerDoorReady
    beq SecretWallOnHit0BFF_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SecretWallOnHit0BFF_at0
    cmp #$03
    bcs SecretWallOnHit0BFF_at0
    jsr VecSelectVramReadAddress
    st1 #$BE
    st2 #$05
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    ldy a:VDC_DATA_HI
    cmp #$17
    bne SecretWallOnHit0BFF_at0
    inc z:zField8C
    lda #$03
    sta z:zSubStep
    phx
    lda #$85
    ldx #$06
    jsr VecStartListChannelKind2
    plx
    inc a:OBJ_STATE,x
SecretWallOnHit0BFF_at0:
    rts

; @routine 0BFF:4F8 CopyEightVramRows0BFF
; @run 71:D4F8
; WHAT IT DOES: Copies eight runs of 8 VRAM words: the source addresses from the list A's low three bits pick, the
; destinations from a table turned within each group of four by A's upper bits (Stage 07's overlay).
; HOW: A loop of ClearVdcControlHigh, read and write addresses and eight word copies through the VDC.
; WAITS: no.
; Entry: A the list (bits 0 to 2) and the turn (bits 3 up). Exit: A, X, Y and the flags as its last instruction or
; call left them.
CopyEightVramRows0BFFDestinations = $D569 ; the eight VRAM destination addresses, low and high byte
CopyEightVramRows0BFFLists = $D579 ; the source list address for each value of A's low bits, low and high
CopyEightVramRows0BFF:
    pha
    and #$07
    asl a
    tay
    lda a:CopyEightVramRows0BFFLists,y
    sta z:zTemp00
    lda a:CopyEightVramRows0BFFLists+1,y
    sta z:zTemp01
    stz z:zTemp02
    pla
    lsr a
    lsr a
    lsr a
    sta z:zTemp10
CopyEightVramRows0BFF_at2:
    jsr VecClearVdcControlHigh
    jsr VecSelectVramReadAddress
    lda (zTemp00)
    sta a:VDC_DATA_LO
    ldy #$01
    lda (zTemp00),y
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    lda z:zTemp02
    and #$FC
    sta z:zTemp03
    lda z:zTemp02
    clc
    adc z:zTemp10
    and #$03
    ora z:zTemp03
    asl a
    tay
    lda a:CopyEightVramRows0BFFDestinations,y
    sta a:VDC_DATA_LO
    lda a:CopyEightVramRows0BFFDestinations+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    ldy #$08
CopyEightVramRows0BFF_at0:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    dey
    bne CopyEightVramRows0BFF_at0
    lda #$02
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc CopyEightVramRows0BFF_at1
    inc z:zTemp01
CopyEightVramRows0BFF_at1:
    inc z:zTemp02
    lda z:zTemp02
    cmp #$08
    bne CopyEightVramRows0BFF_at2
    rts

; @routine 0BFF:609 LoadRecordFlagPartner0BFF
; @run 71:D609
; WHAT IT DOES: In state 0 loads the object's record and, when ProgressCharacter bit 1 is set, sets OBJ_WORK_E $0F
; in the slot in its OBJ_START_ARG (Stage 07's overlay).
; HOW: LoadObjectRecord; a test of bit 1; a store to slot Y.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordFlagPartner0BFF:
    lda a:OBJ_STATE,x
    bne LoadRecordFlagPartner0BFF_at0
    jsr VecLoadObjectRecord
    lda a:ProgressCharacter
    and #$02
    beq LoadRecordFlagPartner0BFF_at0
    ldy a:OBJ_START_ARG,x
    lda #$0F
    sta a:OBJ_WORK_E,y
LoadRecordFlagPartner0BFF_at0:
    rts
