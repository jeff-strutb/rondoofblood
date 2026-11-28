; @routine 0A9A:7B7 RunByState0A9A
; @run 70:AFB7
; WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunByState0A9AHandlers = $AFC9 ; the handler address for each state, low and high byte
RunByState0A9A:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RunByState0A9AHandlers,y
    sta z:zTemp00
    lda a:RunByState0A9AHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9B:006 SwingDirectionAbout40_0A9B
; @run 70:B006
; WHAT IT DOES: Follows the slot before it and moves, checks its partner or the hit, lets the last part split off,
; then swings its direction (OBJ_WORK_F) about $40: the 16-bit step OBJ_WORK_G:OBJ_WORK_H is added to
; OBJ_WORK_E:OBJ_WORK_F and pulled back by $0A each frame toward $40 (Stage 06's overlay).
; HOW: Three calls, a 16-bit add, a compare with $40 and a 16-bit add or subtract of $0A.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SwingDirectionAbout40_0A9B:
    jsr FollowPrevThenMove0A9B
    jsr CheckPartnerOrHit0A9B
    jsr LastPartBecomeKind2C0A9B
    lda a:OBJ_WORK_E,x
    clc
    adc a:OBJ_WORK_G,x
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_F,x
    adc a:OBJ_WORK_H,x
    sta a:OBJ_WORK_F,x
    cmp #$40
    bcc SwingDirectionAbout40_0A9B_at0
    lda a:OBJ_WORK_G,x
    sec
    sbc #$0A
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_H,x
    sbc #$00
    sta a:OBJ_WORK_H,x
    rts
SwingDirectionAbout40_0A9B_at0:
    lda a:OBJ_WORK_G,x
    clc
    adc #$0A
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_H,x
    adc #$00
    sta a:OBJ_WORK_H,x
    rts

; @routine 0A9B:2D8 LastPartBecomeKind2C0A9B
; @run 70:B2D8
; WHAT IT DOES: For the last part (OBJ_WORK_I 5): counts OBJ_WORK_J down; at 0, when the player is $28 or more
; away, fewer than two kind $2C are out and a slot from $30 is free, it becomes a kind $2C there and puts the six
; parts (the five slots before it and itself) in state 3 (Stage 06's overlay).
; HOW: DEC; DistanceXToPlayer, CountKind2CFrom30_0A9B, FindFreeSlotFrom30, BecomeKind2CInY0A9B; a loop of six
; stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LastPartBecomeKind2C0A9B:
    lda a:OBJ_WORK_I,x
    cmp #$05
    bne LastPartBecomeKind2C0A9B_at0
    lda a:OBJ_WORK_J,x
    beq LastPartBecomeKind2C0A9B_at1
    dec a:OBJ_WORK_J,x
    rts
LastPartBecomeKind2C0A9B_at1:
    jsr VecDistanceXToPlayer
    cmp #$28
    bcc LastPartBecomeKind2C0A9B_at0
    jsr CountKind2CFrom30_0A9B
    cmp #$02
    bcs LastPartBecomeKind2C0A9B_at0
    jsr VecFindFreeSlotFrom30
    bcc LastPartBecomeKind2C0A9B_at0
    jsr BecomeKind2CInY0A9B
    txa
    sec
    sbc #$05
    tay
    lda #$06
    sta z:zTemp00
LastPartBecomeKind2C0A9B_at2:
    lda #$03
    sta a:OBJ_STATE,y
    iny
    dec z:zTemp00
    bne LastPartBecomeKind2C0A9B_at2
LastPartBecomeKind2C0A9B_at0:
    rts

; @routine 0A9B:379 RunByState0A9B_379
; @run 70:B379
; WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunByState0A9B_379Handlers = $B38B ; the handler address for each state, low and high byte
RunByState0A9B_379:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RunByState0A9B_379Handlers,y
    sta z:zTemp00
    lda a:RunByState0A9B_379Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9B:391 LoadRecordAndEvent0A9B
; @run 70:B391
; WHAT IT DOES: Loads the object's record and records its event (Stage 06's overlay).
; HOW: LoadObjectRecord, then a tail into RecordObjectEvent.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordAndEvent0A9B:
    jsr VecLoadObjectRecord
    jmp VecRecordObjectEvent

; @routine 0A9B:3D0 RunByState0A9B_3D0
; @run 70:B3D0
; WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunByState0A9B_3D0Handlers = $B3E2 ; the handler address for each state, low and high byte
RunByState0A9B_3D0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RunByState0A9B_3D0Handlers,y
    sta z:zTemp00
    lda a:RunByState0A9B_3D0Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9B:413 FallAndMove0A9B
; @run 70:B413
; WHAT IT DOES: Lets the object fall with gravity and moves it by its speeds (Stage 06's overlay).
; HOW: ApplyGravityUnlimited, then a tail into ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FallAndMove0A9B:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0A9B:419 RunByState0A9B_419
; @run 70:B419
; WHAT IT DOES: Runs the handler of the object's state; state 0's is LoadObjectRecord itself (Stage 06's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunByState0A9B_419Handlers = $B42B ; the handler address for each state, low and high byte
RunByState0A9B_419:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RunByState0A9B_419Handlers,y
    sta z:zTemp00
    lda a:RunByState0A9B_419Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9B:4CF BuildListsUntilScrollY6_0A9B
; @run 70:B4CF
; WHAT IT DOES: State 0 loads the record, builds the stage lists of section 3 (zField82 3 for ExpandStageListsI,
; then 2) and sets zField86 7; with OBJ_START_ARG set it also sets zField82 3, becomes kind $C2 in state 0 at X
; $0080, Y $00FE with OBJ_START_ARG $0D and starts list $87 on list channel 4. Later states wait for the scroll's Y
; high byte to be 6, then clear CameraMode, set zField82 2 and remove the object (Stage 06's overlay).
; HOW: LoadObjectRecord, ExpandStageListsI, stores, StartListChannelKind2; a compare, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BuildListsUntilScrollY6_0A9B:
    lda a:OBJ_STATE,x
    bne BuildListsUntilScrollY6_0A9B_at0
    jsr VecLoadObjectRecord
    lda #$03
    sta z:zField82
    phx
    jsr ExpandStageListsI
    plx
    lda #$02
    sta z:zField82
    lda #$07
    sta z:zField86
    lda a:OBJ_START_ARG,x
    beq BuildListsUntilScrollY6_0A9B_at1
    lda #$03
    sta z:zField82
    lda #$C2
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    lda #$FE
    sta a:OBJ_Y_LO,x
    lda #$0D
    sta a:OBJ_START_ARG,x
    phx
    lda #$87
    ldx #$04
    jsr VecStartListChannelKind2
    plx
BuildListsUntilScrollY6_0A9B_at1:
    rts
BuildListsUntilScrollY6_0A9B_at0:
    lda a:ScrollYHiScreen
    cmp #$06
    bne BuildListsUntilScrollY6_0A9B_at2
    stz a:CameraMode
    lda #$02
    sta z:zField82
    jmp VecClearObject
BuildListsUntilScrollY6_0A9B_at2:
    rts

; @routine 0A9B:560 RunByState0A9B_560
; @run 70:B560
; WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunByState0A9B_560Handlers = $B572 ; the handler address for each state, low and high byte
RunByState0A9B_560:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RunByState0A9B_560Handlers,y
    sta z:zTemp00
    lda a:RunByState0A9B_560Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9B:5AA CountLightHitsThenState0A9B
; @run 70:B5AA
; WHAT IT DOES: While the light-hit check passes, counts OBJ_WORK_E down and at 0 steps the state on; when it
; fails, sets OBJ_WORK_E back to OBJ_WORK_D (Stage 06's overlay).
; HOW: LightHitCheckByArg0A9B's carry; DEC or INC; a copy.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountLightHitsThenState0A9B:
    jsr LightHitCheckByArg0A9B
    bcc CountLightHitsThenState0A9B_at0
    lda a:OBJ_WORK_E,x
    beq CountLightHitsThenState0A9B_at1
    dec a:OBJ_WORK_E,x
    rts
CountLightHitsThenState0A9B_at1:
    inc a:OBJ_STATE,x
    rts
CountLightHitsThenState0A9B_at0:
    lda a:OBJ_WORK_D,x
    sta a:OBJ_WORK_E,x
    rts

; @routine 0A9B:5C3 RunByStartArg0A9B
; @run 70:B5C3
; WHAT IT DOES: Runs the handler that OBJ_START_ARG picks (Stage 06's overlay).
; HOW: A jump table by OBJ_START_ARG.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunByStartArg0A9BHandlers = $B55C ; the handler address for each OBJ_START_ARG, low and high byte
RunByStartArg0A9B:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:RunByStartArg0A9BHandlers,y
    sta z:zTemp00
    lda a:RunByStartArg0A9BHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9B:5D5 RunBySubstate0A9B
; @run 70:B5D5
; WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunBySubstate0A9BHandlers = $B5E7 ; the handler address for each substate, low and high byte
RunBySubstate0A9B:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:RunBySubstate0A9BHandlers,y
    sta z:zTemp00
    lda a:RunBySubstate0A9BHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9B:633 LightHitCheckByArg0A9B
; @run 70:B633
; WHAT IT DOES: Clears the object's OBJ_INVULN_TIMER; hit lightly (OBJ_HIT_TAKEN 1 to 4) it runs the check
; OBJ_START_ARG picks (for 0, $B653: whether the player is in state 2), otherwise it says no (Stage 06's overlay).
; HOW: A store, a compare, a jump table by OBJ_START_ARG; CLC.
; WAITS: no.
; Entry: X the object. Exit: C set when the check passes; C clear when it fails or the object was not hit lightly.
LightHitCheckByArg0A9BChecks = $B55E ; the check address for each OBJ_START_ARG, low and high byte (RunByStartArg0A9B's table one word on)
LightHitCheckByArg0A9B:
    stz a:OBJ_INVULN_TIMER,x
    lda a:OBJ_HIT_TAKEN,x
    beq LightHitCheckByArg0A9B_at0
    cmp #$05
    bcs LightHitCheckByArg0A9B_at0
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:LightHitCheckByArg0A9BChecks,y
    sta z:zTemp00
    lda a:LightHitCheckByArg0A9BChecks+1,y
    sta z:zTemp01
    jmp (ZeroPage)
LightHitCheckByArg0A9B_at0:
    clc
    rts

; @routine 0A9B:71C StartWithTwoKindCB0A9B
; @run 70:B71C
; WHAT IT DOES: State 0 loads the record, sets zRasterStage 4, starts two kind $CB objects in slots from $38 that
; keep its slot, sets its hit box half height $0C and sprite set 6, and plays its frame list; later states go on at
; $B744 (Stage 06's overlay).
; HOW: LoadObjectRecord, two calls of StartKindCBFrom38_0A9B, stores, a fall into PlayFixedList0A9B.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWithTwoKindCB0A9B:
    lda a:OBJ_STATE,x
    bne PlayUntilHitThenBurst0A9B
    jsr VecLoadObjectRecord
    lda #$04
    sta z:zRasterStage
    jsr StartKindCBFrom38_0A9B
    jsr StartKindCBFrom38_0A9B
    lda #$0C
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x

; @routine 0A9B:744 PlayUntilHitThenBurst0A9B
; @run 70:B744
; WHAT IT DOES: Plays the frame list; on a damaging hit (OBJ_HIT_TAKEN 5 or more) records its event, becomes the
; kind in OBJ_START_ARG in state 0 with OBJ_INVULN_TIMER $10, plays sound $19, and starts three kind $CA
; (OBJ_WORK_D 1 and 2 for the second and third) and a kind $9C in free slots from $30, as long as slots are found
; (Stage 06's overlay).
; HOW: PlayFixedList0A9B, a compare, RecordObjectEvent, stores, SoundCommand, four FindFreeSlotFrom30 and
; StartObjectAtPlace.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayUntilHitThenBurst0A9B:
    bsr PlayFixedList0A9B
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc PlayUntilHitThenBurst0A9B_at0
    jsr VecRecordObjectEvent
    lda a:OBJ_START_ARG,x
    sta a:OBJ_KIND,x
    lda #$10
    sta a:OBJ_INVULN_TIMER,x
    stz a:OBJ_STATE,x
    lda #$19
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom30
    bcc PlayUntilHitThenBurst0A9B_at0
    lda #$CA
    jsr VecStartObjectAtPlace
    jsr VecFindFreeSlotFrom30
    bcc PlayUntilHitThenBurst0A9B_at0
    lda #$CA
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,y
    jsr VecFindFreeSlotFrom30
    bcc PlayUntilHitThenBurst0A9B_at0
    lda #$CA
    jsr VecStartObjectAtPlace
    lda #$02
    sta a:OBJ_WORK_D,y
    jsr VecFindFreeSlotFrom30
    bcc PlayUntilHitThenBurst0A9B_at0
    lda #$9C
    jsr VecStartObjectAtPlace
PlayUntilHitThenBurst0A9B_at0:
    rts

; @routine 0A9B:796 StartKindCBFrom38_0A9B
; @run 70:B796
; WHAT IT DOES: Starts a kind $CB at the object's place in the first empty slot from $38, with the object's slot in
; its OBJ_START_ARG (Stage 06's overlay).
; HOW: FreeSlotFrom38_0A9B, StartObjectAtPlace, a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartKindCBFrom38_0A9B:
    jsr FreeSlotFrom38_0A9B
    bcc StartKindCBFrom38_0A9B_at0
    lda #$CB
    jsr VecStartObjectAtPlace
    txa
    sta a:OBJ_START_ARG,y
StartKindCBFrom38_0A9B_at0:
    rts

; @routine 0A9C:1AA ShowFrameByArg0A9C
; @run 70:B9AA
; WHAT IT DOES: In state 0 loads the record and shows the frame in OBJ_START_ARG with sprite set 6 (Stage 06's
; overlay).
; HOW: LoadObjectRecord and two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ShowFrameByArg0A9C:
    lda a:OBJ_STATE,x
    bne ShowFrameByArg0A9C_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
ShowFrameByArg0A9C_at0:
    rts

; @routine 0A9C:32B SinkPairWhileStoodOn0A9C
; @run 70:BB2B
; WHAT IT DOES: A pair of slots (the object and the one after it) that the player stands on: in state 1, standing
; on it while not rising, it sinks a pixel a frame down to Y $A9 while the pair in OBJ_START_ARG rises a pixel and
; slot $2E shakes about slot $2F's place, with sound $AB once (stopped by ad_stop when he steps off); in state 2
; the first pair (OBJ_START_ARG 0) rises at speed 3 to Y $90, the other, once OBJ_WORK_D is set, sinks while its
; partner rises, and with the player above Y $10 sets zField83 5, zField87 0 and zSubStep 2; from state 3 it
; springs back to Y $90 with its second slot $A0 above, and dips once at Y speed $1C0 when the player lands on it
; at X $48 or more (Stage 06's overlay).
; HOW: ContactWithPlayer with a $1A by 6 box, 16-bit INC and DEC of the Y positions, RandomMinusOneToOne0A9C,
; ApplyVelocity on both slots, RiseToYInA0A9C.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SinkPairWhileStoodOn0A9C:
    lda a:OBJ_STATE,x
    bne SinkPairWhileStoodOn0A9C_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    bne SinkPairWhileStoodOn0A9C_at1
    jsr VecRecordObjectEvent
    inx
    jsr VecRecordObjectEvent
    dex
    inc a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
    jmp MarkAndSoundAB0A9C
SinkPairWhileStoodOn0A9C_at4:
    lda a:OBJ_WORK_F,x
    beq SinkPairWhileStoodOn0A9C_at2
    jsr BiosAdStop
    stz a:OBJ_WORK_F,x
SinkPairWhileStoodOn0A9C_at2:
    rts
SinkPairWhileStoodOn0A9C_at0:
    dec a
    bne SinkPairWhileStoodOn0A9C_at3
    lda #$1A
    sta z:zTemp08
    lda #$06
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc SinkPairWhileStoodOn0A9C_at4
    lda a:OBJ_VY_HI
    bmi SinkPairWhileStoodOn0A9C_at4
    lda a:OBJ_Y_LO,x
    cmp #$A9
    bcs SinkPairWhileStoodOn0A9C_at4
    jsr SoundOnce0A9C
    inc a:OBJ_Y_LO,x
    inc a:OBJ_Y_LO+1,x
    bne SinkPairWhileStoodOn0A9C_at5
    inc a:OBJ_Y_HI+1,x
SinkPairWhileStoodOn0A9C_at5:
    lda a:OBJ_START_ARG,x
    phx
    tax
    dec a:OBJ_Y_LO,x
    dec a:OBJ_Y_LO+1,x
    lda a:OBJ_Y_LO+1,x
    inc a
    bne SinkPairWhileStoodOn0A9C_at6
    dec a:OBJ_Y_HI+1,x
SinkPairWhileStoodOn0A9C_at6:
    ldx #$2E
    jsr RandomMinusOneToOne0A9C
    clc
    adc a:OBJ_X_LO+1,x
    sta a:OBJ_X_LO,x
    jsr RandomMinusOneToOne0A9C
    clc
    adc a:OBJ_Y_LO+1,x
    sta a:OBJ_Y_LO,x
    plx
SinkPairWhileStoodOn0A9C_at1:
    rts
SinkPairWhileStoodOn0A9C_at10:
    lda a:OBJ_Y_LO,x
    cmp #$90
    bcs SinkPairWhileStoodOn0A9C_at7
    lda #$90
    sta a:OBJ_Y_LO,x
    lda #$F0
    sta a:OBJ_Y_LO+1,x
    inc a:OBJ_STATE,x
    rts
SinkPairWhileStoodOn0A9C_at7:
    txa
    bra SinkPairWhileStoodOn0A9C_at8
SinkPairWhileStoodOn0A9C_at3:
    dec a
    bne SinkPairWhileStoodOn0A9C_at9
    lda #$1A
    sta z:zTemp08
    lda #$06
    sta z:zTemp09
    jsr ContactWithPlayer
    lda a:OBJ_START_ARG,x
    beq SinkPairWhileStoodOn0A9C_at10
    lda a:OBJ_WORK_D,x
    beq SinkPairWhileStoodOn0A9C_at1
    jsr SoundOnce0A9C
    lda #$03
    sta a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI+1,x
    jsr VecApplyVelocity
    inx
    jsr VecApplyVelocity
    dex
    lda a:OBJ_START_ARG,x
SinkPairWhileStoodOn0A9C_at8:
    phx
    tax
    lda #$FD
    sta a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI+1,x
    jsr VecApplyVelocity
    inx
    jsr VecApplyVelocity
    dex
    plx
    lda a:OBJ_Y_LO
    cmp #$10
    bcs SinkPairWhileStoodOn0A9C_at1
    lda #$05
    sta z:zField83
    stz z:zField87
    lda #$02
    sta z:zSubStep
    rts
SinkPairWhileStoodOn0A9C_at9:
    lda #$90
    jsr RiseToYInA0A9C
    lda a:OBJ_Y_LO,x
    sec
    sbc #$A0
    sta a:OBJ_Y_LO+1,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_Y_HI+1,x
    lda #$1A
    sta z:zTemp08
    lda #$06
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc SinkPairWhileStoodOn0A9C_at11
    lda a:OBJ_WORK_E,x
    bne SinkPairWhileStoodOn0A9C_at12
    lda a:OBJ_X_LO
    cmp #$48
    bcc SinkPairWhileStoodOn0A9C_at12
    jsr SoundOnce0A9C
    lda #$01
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    inc a:OBJ_WORK_E,x
SinkPairWhileStoodOn0A9C_at12:
    rts
SinkPairWhileStoodOn0A9C_at11:
    stz a:OBJ_WORK_E,x
    rts

; @routine 0A9C:46C MarkAndSoundAB0A9C
; @run 70:BC6C
; WHAT IT DOES: Sets OBJ_WORK_F on and plays sound $AB: the second half of SoundOnce0A9C, which state 0 of
; SinkPairWhileStoodOn0A9C enters directly (Stage 06's overlay).
; HOW: INC, then SoundCommand.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MarkAndSoundAB0A9C:
    inc a:OBJ_WORK_F,x
    lda #$AB
    jsr VecSoundCommand

; @routine 0A9C:475 ShakeThenDropOnPair0A9C
; @run 70:BC75
; WHAT IT DOES: State 0 loads the record; state 1 waits for a damaging hit, then shakes about the next slot's place
; for $18 frames; after that it falls, and on coming within 8 pixels above the slot in OBJ_START_ARG it records its
; event, steps that slot's state and OBJ_WORK_D on and the state of the slot in that one's OBJ_START_ARG (recording
; the events of both pairs), then falls on at Y speed 3 and is removed once its Y high byte is set and its Y low
; byte is $80 or more (Stage 06's overlay).
; HOW: A compare of OBJ_HIT_TAKEN, DEC, RandomMinusTwoToTwo0A9C, a gravity add of $28, ApplyVelocity,
; RecordObjectEvent, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ShakeThenDropOnPair0A9C:
    lda a:OBJ_STATE,x
    bne ShakeThenDropOnPair0A9C_at0
    jmp VecLoadObjectRecord
ShakeThenDropOnPair0A9C_at0:
    dec a
    bne ShakeThenDropOnPair0A9C_at1
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc ShakeThenDropOnPair0A9C_at2
    lda #$18
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
    rts
ShakeThenDropOnPair0A9C_at1:
    dec a
    bne ShakeThenDropOnPair0A9C_at3
    dec a:OBJ_WORK_D,x
    beq ShakeThenDropOnPair0A9C_at4
    jsr RandomMinusTwoToTwo0A9C
    clc
    adc a:OBJ_X_LO+1,x
    sta a:OBJ_X_LO,x
    jsr RandomMinusTwoToTwo0A9C
    clc
    adc a:OBJ_Y_LO+1,x
    sta a:OBJ_Y_LO,x
    rts
ShakeThenDropOnPair0A9C_at4:
    inc a:OBJ_STATE,x
    lda a:OBJ_X_LO+1,x
    sta a:OBJ_X_LO,x
    lda a:OBJ_Y_LO+1,x
    sta a:OBJ_Y_LO,x
ShakeThenDropOnPair0A9C_at2:
    rts
ShakeThenDropOnPair0A9C_at3:
    lda a:OBJ_WORK_E,x
    bne ShakeThenDropOnPair0A9C_at5
    lda a:OBJ_VY_LO,x
    clc
    adc #$28
    sta a:OBJ_VY_LO,x
    bcc ShakeThenDropOnPair0A9C_at6
    inc a:OBJ_VY_HI,x
ShakeThenDropOnPair0A9C_at6:
    jsr VecApplyVelocity
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_Y_LO,y
    sec
    sbc a:OBJ_Y_LO,x
    bcc ShakeThenDropOnPair0A9C_at7
    cmp #$08
    bcs ShakeThenDropOnPair0A9C_at2
ShakeThenDropOnPair0A9C_at7:
    inc a:OBJ_WORK_E,x
    jsr VecRecordObjectEvent
    lda a:OBJ_START_ARG,x
    phx
    tax
    inc a:OBJ_STATE,x
    inc a:OBJ_WORK_D,x
    jsr VecRecordObjectEvent
    inx
    jsr VecRecordObjectEvent
    dex
    lda a:OBJ_START_ARG,x
    tax
    inc a:OBJ_STATE,x
    jsr VecRecordObjectEvent
    inx
    jsr VecRecordObjectEvent
    dex
    plx
    lda #$03
    sta a:OBJ_VY_HI,x
    rts
ShakeThenDropOnPair0A9C_at5:
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    beq ShakeThenDropOnPair0A9C_at2
    lda a:OBJ_Y_LO,x
    bpl ShakeThenDropOnPair0A9C_at2
    jmp VecClearObject

; @routine 0A9C:52F FallWhenSlotBeforeGone0A9C
; @run 70:BD2F
; WHAT IT DOES: State 0 loads the record with frame $11 and sprite set 6; then, with a Y speed and the slot before
; it empty, stops the ADPCM sound the first time (OBJ_WORK_D) and moves by its speeds (Stage 06's overlay).
; HOW: LoadObjectRecord, stores; tests of the Y speed and the kind of X - 1, ad_stop, ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FallWhenSlotBeforeGone0A9C:
    lda a:OBJ_STATE,x
    bne FallWhenSlotBeforeGone0A9C_at0
    jsr VecLoadObjectRecord
    lda #$11
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    rts
FallWhenSlotBeforeGone0A9C_at0:
    lda a:OBJ_VY_HI,x
    beq FallWhenSlotBeforeGone0A9C_at1
    lda a:OBJ_KIND-1,x
    bne FallWhenSlotBeforeGone0A9C_at1
    lda a:OBJ_WORK_D,x
    bne FallWhenSlotBeforeGone0A9C_at2
    inc a:OBJ_WORK_D,x
    jsr BiosAdStop
FallWhenSlotBeforeGone0A9C_at2:
    jsr VecApplyVelocity
FallWhenSlotBeforeGone0A9C_at1:
    rts

; @routine 0A9C:55B RiseToYInA0A9C
; @run 70:BD5B
; WHAT IT DOES: Moves the object by its Y speed, speeding it up by $40 a frame while it is below the Y in A; at or
; above that Y puts it there and stops it, stopping the ADPCM sound if OBJ_WORK_F is set (Stage 06's overlay).
; HOW: ApplyVelocityY, a compare, a 16-bit subtract, stores, ad_stop.
; WAITS: no.
; Entry: X the object; A the Y to rise to. Exit: A, X, Y and the flags as its last instruction or call left them.
RiseToYInA0A9C:
    sta z:zTemp00
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp z:zTemp00
    beq RiseToYInA0A9C_at0
    bcc RiseToYInA0A9C_at0
    lda a:OBJ_VY_LO,x
    sec
    sbc #$40
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    sbc #$00
    sta a:OBJ_VY_HI,x
    rts
RiseToYInA0A9C_at0:
    lda z:zTemp00
    sta a:OBJ_Y_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda a:OBJ_WORK_F,x
    beq RiseToYInA0A9C_at1
    jsr BiosAdStop
    stz a:OBJ_WORK_F,x
RiseToYInA0A9C_at1:
    rts

; @routine 0A9D:5AA StartKind27InXRange0A9D
; @run 71:C5AA
; WHAT IT DOES: State 0 loads the record; later counts the 16-bit OBJ_WORK_D:OBJ_WORK_E down and at 0, with the
; player inside the object's X range and an empty slot in $30 to $32, starts a kind $27 there (Stage 06's overlay).
; HOW: A 16-bit decrement, PlayerXInRange0A9D, a search loop, then $C5DE.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartKind27InXRange0A9D:
    lda a:OBJ_STATE,x
    bne StartKind27InXRange0A9D_at0
    jmp VecLoadObjectRecord
StartKind27InXRange0A9D_at0:
    lda a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    beq StartKind27InXRange0A9D_at1
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    rts
StartKind27InXRange0A9D_at1:
    jsr PlayerXInRange0A9D
    bcc PlainReturn0A9D
    ldy #$30
StartKind27InXRange0A9D_at2:
    lda a:OBJ_KIND,y
    beq StartKind27InY0A9D
    iny
    cpy #$33
    bcc StartKind27InXRange0A9D_at2

; @routine 0A9D:5DD PlainReturn0A9D
; @run 71:C5DD
; WHAT IT DOES: Returns with the flags as they are; the X range test and the timed start jump here to leave (Stage
; 06's overlay).
; HOW: RTS.
; WAITS: no.
; Entry: nothing. Exit: A, X, Y and the flags as its last instruction or call left them.
PlainReturn0A9D:
    rts

; @routine 0A9D:5DE StartKind27InY0A9D
; @run 71:C5DE
; WHAT IT DOES: Clears slot Y, starts a kind $27 in it at the object's place and sets the object's timer
; OBJ_WORK_D:OBJ_WORK_E to $0080 (Stage 06's overlay).
; HOW: ClearObjectInY, StartObjectAtPlace, two stores.
; WAITS: no.
; Entry: X the object; Y the slot. Exit: A, X, Y and the flags as its last instruction or call left them.
StartKind27InY0A9D:
    jsr VecClearObjectInY
    lda #$27
    jsr VecStartObjectAtPlace
    stz a:OBJ_WORK_D,x
    lda #$80
    sta a:OBJ_WORK_E,x
    rts

; @routine 0A9D:5EF PlayerXInRange0A9D
; @run 71:C5EF
; WHAT IT DOES: Says whether the player's place across the stage (the scroll's X plus the player's X low byte, with
; the object's X high byte in the high byte) lies in the range OBJ_START_ARG picks, from its low end up to below
; its high end (Stage 06's overlay).
; HOW: Four times OBJ_START_ARG indexes four-byte records; 16-bit compares high byte first.
; WAITS: no.
; Entry: X the object. Exit: C set when inside the range; C clear when outside.
PlayerXInRange0A9DRanges = $C626 ; low end low, high, high end low, high per OBJ_START_ARG
PlayerXInRange0A9D:
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    tay
    lda a:ScrollXLo
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:ScrollXHiScreen
    adc a:OBJ_X_HI,x
    cmp a:PlayerXInRange0A9DRanges+3,y
    bcc PlayerXInRange0A9D_at0
    beq PlayerXInRange0A9D_at1
PlayerXInRange0A9D_at2:
    clc
    rts
PlayerXInRange0A9D_at1:
    lda z:zTemp00
    cmp a:PlayerXInRange0A9DRanges+2,y
    bcs PlayerXInRange0A9D_at2
PlayerXInRange0A9D_at3:
    sec
    rts
PlayerXInRange0A9D_at0:
    cmp a:PlayerXInRange0A9DRanges+1,y
    bcc PlainReturn0A9D
    bne PlayerXInRange0A9D_at3
    lda z:zTemp00
    cmp a:PlayerXInRange0A9DRanges,y
    bcc PlainReturn0A9D
    sec
    rts

; @routine 0A9D:7EC FacePlayerThenChoose0A9D
; @run 71:C7EC
; WHAT IT DOES: Turns the object to face the player, clears OBJ_WORK_D, makes the choice for the player's distance
; and steps the state on (Stage 06's overlay).
; HOW: SideOfPlayer's bit 0, a store, ChooseByPlayerDistance0A9D, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FacePlayerThenChoose0A9D:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_WORK_D,x
    jsr ChooseByPlayerDistance0A9D
    inc a:OBJ_STATE,x
    rts

; @routine 0A9E:078 SetWorkD4_0A9E
; @run 71:C878
; WHAT IT DOES: Sets OBJ_WORK_D to 4 (Stage 06's overlay).
; HOW: One store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetWorkD4_0A9E:
    lda #$04
    sta a:OBJ_WORK_D,x
    rts

; @routine 0A9E:13F ApproachUntilNear0A9E
; @run 71:C93F
; WHAT IT DOES: Walks toward the player (OBJ_WORK_E from his side); closer than $4C pixels it goes to state 3; then
; checks the ground ahead (Stage 06's overlay).
; HOW: WalkByWorkE0A9E, SideOfPlayer, DistanceXToPlayer, a store, then $C9E9.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ApproachUntilNear0A9E:
    jsr WalkByWorkE0A9E
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$4C
    bcs ApproachUntilNear0A9E_at0
    lda #$03
    sta a:OBJ_STATE,x
ApproachUntilNear0A9E_at0:
    jmp StopAtLedgeOrWall0A9E

; @routine 0A9E:159 WalkByWorkE0A9E
; @run 71:C959
; WHAT IT DOES: Faces the player and plays frame list 0; as each step begins, takes the X speed OBJ_WORK_E picks
; ($FC left, $04 right) and moves across (Stage 06's overlay).
; HOW: FacePlayerPlayList0A9E with Y 0, its carry, a table read, ApplyVelocityX.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkByWorkE0A9ESpeeds = $C96C ; the X speed high byte for each OBJ_WORK_E
WalkByWorkE0A9E:
    cly
    jsr FacePlayerPlayList0A9E
    bcc WalkByWorkE0A9E_at0
    ldy a:OBJ_WORK_E,x
    lda a:WalkByWorkE0A9ESpeeds,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
WalkByWorkE0A9E_at0:
    rts

; @routine 0A9E:1E9 StopAtLedgeOrWall0A9E
; @run 71:C9E9
; WHAT IT DOES: Probes the floor (height 8): with floor under all three points and no wall, counts OBJ_WORK_D down
; and at 0 restarts the animation in state 4; off a ledge or at a wall restarts it in state 5, substate 0 (Stage
; 06's overlay).
; HOW: ProbeTerrain with 9, then with $22; an AND of the three TileHit bytes; DEC, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StopAtLedgeOrWall0A9E:
    lda #$08
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    lda a:TileHit
    and a:TileDepthX-3
    and a:TileDepthX-2
    and #$01
    beq StopAtLedgeOrWall0A9E_at0
    lda #$08
    sta z:SpriteFlipMask
    lda #$22
    jsr VecProbeTerrain
    and #$02
    beq StopAtLedgeOrWall0A9E_at1
StopAtLedgeOrWall0A9E_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    lda #$05
    sta a:OBJ_STATE,x
    rts
StopAtLedgeOrWall0A9E_at1:
    dec a:OBJ_WORK_D,x
    bne StopAtLedgeOrWall0A9E_at2
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
StopAtLedgeOrWall0A9E_at2:
    rts

; @routine 0A9E:22C BackAwayUntilFar0A9E
; @run 71:CA2C
; WHAT IT DOES: Walks away from the player (OBJ_WORK_E the other side); $6C pixels or more away it goes to state 2;
; then checks the ground ahead (Stage 06's overlay).
; HOW: WalkByWorkE0A9E, SideOfPlayer, EOR, DistanceXToPlayer, a store, then $C9E9.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BackAwayUntilFar0A9E:
    jsr WalkByWorkE0A9E
    jsr VecSideOfPlayer
    and #$01
    eor #$01
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$6C
    bcc BackAwayUntilFar0A9E_at0
    lda #$02
    sta a:OBJ_STATE,x
BackAwayUntilFar0A9E_at0:
    jmp StopAtLedgeOrWall0A9E

; @routine 0A9E:248 AttackListSpawn2D0A9E
; @run 71:CA48
; WHAT IT DOES: Faces the player and plays frame list 2; at a new step on frame $59 (with X and Y high bytes 0)
; starts a kind $2D in a free slot from $30; when the list ends, or a step begins with a high byte set, restarts
; the animation in state 3 and takes the next of four waits for OBJ_WORK_D by OBJ_START_ARG (Stage 06's overlay).
; HOW: FacePlayerPlayList0A9E with Y 2, FindFreeSlotFrom30, StartObjectAtPlace; INC OBJ_WORK_F, whose low two bits
; plus four times OBJ_START_ARG index the table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AttackListSpawn2D0A9EWaits = $CA8F ; four values of OBJ_WORK_D per OBJ_START_ARG
AttackListSpawn2D0A9E:
    ldy #$02
    jsr FacePlayerPlayList0A9E
    beq AttackListSpawn2D0A9E_at0
    bcc AttackListSpawn2D0A9E_at1
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne AttackListSpawn2D0A9E_at0
    lda a:OBJ_FRAME,x
    cmp #$59
    bne AttackListSpawn2D0A9E_at1
    jsr VecFindFreeSlotFrom30
    bcc AttackListSpawn2D0A9E_at1
    lda #$2D
    jmp VecStartObjectAtPlace
AttackListSpawn2D0A9E_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_F,x
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_WORK_F,x
    and #$03
    clc
    adc z:zTemp00
    tay
    lda a:AttackListSpawn2D0A9EWaits,y
    sta a:OBJ_WORK_D,x
AttackListSpawn2D0A9E_at1:
    rts

; @routine 0A9E:297 RunBySubstate0A9E_297
; @run 71:CA97
; WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunBySubstate0A9E_297Handlers = $CAA9 ; the handler address for each substate, low and high byte
RunBySubstate0A9E_297:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:RunBySubstate0A9E_297Handlers,y
    sta z:zTemp00
    lda a:RunBySubstate0A9E_297Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9E:2AF JumpAfterList0A9E
; @run 71:CAAF
; WHAT IT DOES: Faces the player and plays frame list 4; when it ends, jumps: with OBJ_START_ARG set toward the
; screen's middle (by bit 7 of its X), else the way of OBJ_WORK_E reversed one time in four; X speed $FE or $02, Y
; speed $FD00, then the next substate (Stage 06's overlay).
; HOW: FacePlayerPlayList0A9E with Y 4; a ROL of X low or PlayerDistanceSum's low two bits; zHitSlot the side; a
; table read, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
JumpAfterList0A9ESpeeds = $CAF7 ; the X speed high byte for each side
JumpAfterList0A9E:
    ldy #$04
    jsr FacePlayerPlayList0A9E
    bne JumpAfterList0A9E_at0
    lda a:OBJ_START_ARG,x
    beq JumpAfterList0A9E_at1
    lda a:OBJ_X_LO,x
    rol a
    rol a
    and #$01
    eor #$01
    sta z:zBurstCount
    bra JumpAfterList0A9E_at2
JumpAfterList0A9E_at1:
    ldy a:OBJ_WORK_E,x
    sty z:zBurstCount
    jsr VecPlayerDistanceSum
    and #$03
    bne JumpAfterList0A9E_at2
    lda z:zBurstCount
    eor #$01
    sta z:zBurstCount
JumpAfterList0A9E_at2:
    ldy z:zBurstCount
    lda a:JumpAfterList0A9ESpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
JumpAfterList0A9E_at0:
    rts

; @routine 0A9E:304 ListThenState3_0A9E
; @run 71:CB04
; WHAT IT DOES: Faces the player and plays frame list 6; when it ends restarts the animation in state 3, substate 0
; (Stage 06's overlay).
; HOW: FacePlayerPlayList0A9E with Y 6, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ListThenState3_0A9E:
    ldy #$06
    jsr FacePlayerPlayList0A9E
    bne ListThenState3_0A9E_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
ListThenState3_0A9E_at0:
    rts

; @routine 0A9E:684 RunBySubstate0A9E_684
; @run 71:CE84
; WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunBySubstate0A9E_684Handlers = $CE96 ; the handler address for each substate, low and high byte
RunBySubstate0A9E_684:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:RunBySubstate0A9E_684Handlers,y
    sta z:zTemp00
    lda a:RunBySubstate0A9E_684Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9F:010 RunByState0A9F_010
; @run 71:D010
; WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunByState0A9F_010Handlers = $D022 ; the handler address for each state, low and high byte
RunByState0A9F_010:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RunByState0A9F_010Handlers,y
    sta z:zTemp00
    lda a:RunByState0A9F_010Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9F:0F0 ByStateThenFollow0A9F
; @run 71:D0F0
; WHAT IT DOES: Runs the handler of the object's state, then takes the Y of the slot in OBJ_WORK_F, removed once
; that slot is in state 4 (Stage 06's overlay).
; HOW: RunByState0A9F_0F6, then a tail into FollowParentYOrClear0A9F.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ByStateThenFollow0A9F:
    jsr RunByState0A9F_0F6
    jmp FollowParentYOrClear0A9F

; @routine 0A9F:0F6 RunByState0A9F_0F6
; @run 71:D0F6
; WHAT IT DOES: Runs the handler of the object's state (Stage 06's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunByState0A9F_0F6Handlers = $D108 ; the handler address for each state, low and high byte
RunByState0A9F_0F6:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RunByState0A9F_0F6Handlers,y
    sta z:zTemp00
    lda a:RunByState0A9F_0F6Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9F:145 CountThenArm0A9F
; @run 71:D145
; WHAT IT DOES: Counts OBJ_WORK_E down; at 0 plays sound $26, sets OBJ_WORK_E $40 and OBJ_HITBOX_ON 2 and steps the
; state on (Stage 06's overlay).
; HOW: DEC, SoundCommand, stores, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountThenArm0A9F:
    dec a:OBJ_WORK_E,x
    bne CountThenArm0A9F_at0
    lda #$26
    jsr VecSoundCommand
    lda #$40
    sta a:OBJ_WORK_E,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
CountThenArm0A9F_at0:
    rts

; @routine 0A9F:223 CarryChecksThenByState0A9F
; @run 71:D223
; WHAT IT DOES: Steps OBJ_WORK_G on, carries the slot in OBJ_WORK_E, picks a new OBJ_WORK_D now and then, drops on
; a light hit, then runs the handler of its state (Stage 06's overlay).
; HOW: INC, CarrySlotInWorkE0A9F, RandomWorkDOnZeroG0A9F, LightHitDropThenState4_0A9F, a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CarryChecksThenByState0A9FHandlers = $D241 ; the handler address for each state, low and high byte
CarryChecksThenByState0A9F:
    inc a:OBJ_WORK_G,x
    jsr CarrySlotInWorkE0A9F
    jsr RandomWorkDOnZeroG0A9F
    jsr LightHitDropThenState4_0A9F
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CarryChecksThenByState0A9FHandlers,y
    sta z:zTemp00
    lda a:CarryChecksThenByState0A9FHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9F:2D0 LightHitDropThenState4_0A9F
; @run 71:D2D0
; WHAT IT DOES: Hit lightly (OBJ_HIT_TAKEN 1 to 4) in a state below 4: starts its drop, restarts the animation and
; goes to state 4 with OBJ_WORK_F one of four values picked at random (Stage 06's overlay).
; HOW: Compares, SpawnDropByField93_0A9F, PlayerDistanceSum's low two bits index a table, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LightHitDropThenState4_0A9FValues = $D2FB ; four values of OBJ_WORK_F
LightHitDropThenState4_0A9F:
    lda a:OBJ_HIT_TAKEN,x
    beq LightHitDropThenState4_0A9F_at0
    cmp #$05
    bcs LightHitDropThenState4_0A9F_at0
    lda a:OBJ_STATE,x
    cmp #$04
    bcs LightHitDropThenState4_0A9F_at0
    jsr SpawnDropByField93_0A9F
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    jsr VecPlayerDistanceSum
    and #$03
    tay
    lda a:LightHitDropThenState4_0A9FValues,y
    sta a:OBJ_WORK_F,x
    lda #$04
    sta a:OBJ_STATE,x
LightHitDropThenState4_0A9F_at0:
    rts

; @routine 0A9F:331 LoadRecordAndEvent0A9F
; @run 71:D331
; WHAT IT DOES: Loads the object's record and records its event (Stage 06's overlay).
; HOW: LoadObjectRecord, then a tail into RecordObjectEvent.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadRecordAndEvent0A9F:
    jsr VecLoadObjectRecord
    jmp VecRecordObjectEvent

; @routine 0A9F:49A RunBySubstate0A9F_49A
; @run 71:D49A
; WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunBySubstate0A9F_49AHandlers = $D4AC ; the handler address for each substate, low and high byte
RunBySubstate0A9F_49A:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:RunBySubstate0A9F_49AHandlers,y
    sta z:zTemp00
    lda a:RunBySubstate0A9F_49AHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9F:54C ChecksThenByState0A9F
; @run 71:D54C
; WHAT IT DOES: Steps OBJ_WORK_I on, faces the player every 16 frames, staggers or goes to state 8 when hit, sets
; the hit box by frame, then runs the handler of its state (Stage 06's overlay).
; HOW: INC, FacePlayerEvery16_0A9F, StaggerWhenHit0A9F, State8WhenHit0A9F, HitBoxByFrame0A9F_5F4, a jump table by
; the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ChecksThenByState0A9FHandlers = $D56D ; the handler address for each state, low and high byte
ChecksThenByState0A9F:
    inc a:OBJ_WORK_I,x
    jsr FacePlayerEvery16_0A9F
    jsr StaggerWhenHit0A9F
    jsr State8WhenHit0A9F
    jsr HitBoxByFrame0A9F_5F4
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ChecksThenByState0A9FHandlers,y
    sta z:zTemp00
    lda a:ChecksThenByState0A9FHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9F:693 AdvanceUntilClose0A9F
; @run 71:D693
; WHAT IT DOES: Moves the way it faces, bouncing at walls; closer to the player than $30 pixels it goes to state 5,
; otherwise it counts toward state 6 (Stage 06's overlay).
; HOW: BounceByListStep0A9F, OBJ_WORK_F from the facing, DistanceXToPlayer, a store or $D6D9.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AdvanceUntilClose0A9F:
    jsr BounceByListStep0A9F
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_F,x
    jsr VecDistanceXToPlayer
    cmp #$30
    bcc AdvanceUntilClose0A9F_at0
    jmp CountToState6_0A9F
AdvanceUntilClose0A9F_at0:
    lda #$05
    sta a:OBJ_STATE,x
    rts

; @routine 0A9F:6D9 CountToState6_0A9F
; @run 71:D6D9
; WHAT IT DOES: Counts OBJ_WORK_E down; at 0 goes to state 6, substate 0 (Stage 06's overlay).
; HOW: DEC or two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountToState6_0A9F:
    lda a:OBJ_WORK_E,x
    beq CountToState6_0A9F_at0
    dec a:OBJ_WORK_E,x
    rts
CountToState6_0A9F_at0:
    lda #$06
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 0A9F:6EB BackOffUntilFar0A9F
; @run 71:D6EB
; WHAT IT DOES: Moves against the way it faces, bouncing at walls; $50 pixels or more from the player it goes to
; state 4, otherwise it counts toward state 6 (Stage 06's overlay).
; HOW: BounceByListStep0A9F, OBJ_WORK_F the facing reversed, DistanceXToPlayer, a store or $D6D9.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BackOffUntilFar0A9F:
    jsr BounceByListStep0A9F
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_F,x
    jsr VecDistanceXToPlayer
    cmp #$50
    bcs BackOffUntilFar0A9F_at0
    jmp CountToState6_0A9F
BackOffUntilFar0A9F_at0:
    lda #$04
    sta a:OBJ_STATE,x
    rts

; @routine 0A9F:706 RunBySubstate0A9F_706
; @run 71:D706
; WHAT IT DOES: Runs the handler of the object's substate (Stage 06's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunBySubstate0A9F_706Handlers = $D718 ; the handler address for each substate, low and high byte
RunBySubstate0A9F_706:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:RunBySubstate0A9F_706Handlers,y
    sta z:zTemp00
    lda a:RunBySubstate0A9F_706Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9F:7BA LungeOnFramesB7_0A9F
; @run 71:D7BA
; WHAT IT DOES: Plays the frame list at $D7E1; on frames $B7 to $BA it takes the X speed of its facing ($0C or $F4)
; and moves, bouncing at walls; when the list ends it starts the next round in state 4 (Stage 06's overlay).
; HOW: StepObjectFrames, compares of the frame, a table read by the facing, BounceAtWallsHeightF0A9F,
; NextRoundToState4_0A9F.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LungeOnFramesB7_0A9FSpeeds = $D7FC ; the X speed high byte for each facing
LungeOnFramesB7_0A9F:
    lda #$E1
    sta z:zTemp10
    lda #$D7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq NextRoundToState4_0A9F
    bcc LungeOnFramesB7_0A9F_at0
    lda a:OBJ_FRAME,x
    cmp #$B7
    bcc LungeOnFramesB7_0A9F_at0
    cmp #$BB
    bcs LungeOnFramesB7_0A9F_at0
    ldy a:OBJ_FACING_LEFT,x
    lda a:LungeOnFramesB7_0A9FSpeeds,y
    sta a:OBJ_VX_HI,x
    jsr BounceAtWallsHeightF0A9F
LungeOnFramesB7_0A9F_at0:
    rts

; @routine 0AA0:298 RunByState0AA0
; @run 71:DA98
; WHAT IT DOES: Runs the handler of the object's state; states 0 and 2 are LoadObjectRecord itself (Stage 06's
; overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunByState0AA0Handlers = $DAAA ; the handler address for each state, low and high byte
RunByState0AA0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RunByState0AA0Handlers,y
    sta z:zTemp00
    lda a:RunByState0AA0Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)
