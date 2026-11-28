; @routine 0D2D:284 ActByState0D2D_284
; @run 81:C284
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D2D_284Jumps = $C296 ; the routine for each state, a word each
ActByState0D2D_284:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D2D_284Jumps,y
    sta z:zTemp00
    lda a:ActByState0D2D_284Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D2D:2F4 CountDownOnHitResult0D2D
; @run 81:C2F4
; WHAT IT DOES: Runs HitRoutineByArg0D2D; when that returns C set, counts OBJ_WORK_E down and steps the state on at
; zero, and otherwise reloads OBJ_WORK_E from OBJ_WORK_D (Stage 08's overlay).
; HOW: A call, then DEC or INC, or a copy.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountDownOnHitResult0D2D:
    jsr HitRoutineByArg0D2D
    bcc CountDownOnHitResult0D2D_at0
    lda a:OBJ_WORK_E,x
    beq CountDownOnHitResult0D2D_at1
    dec a:OBJ_WORK_E,x
    rts
CountDownOnHitResult0D2D_at1:
    inc a:OBJ_STATE,x
    rts
CountDownOnHitResult0D2D_at0:
    lda a:OBJ_WORK_D,x
    sta a:OBJ_WORK_E,x
    rts

; @routine 0D2D:30D ActByArg0D2D
; @run 81:C30D
; WHAT IT DOES: Runs the routine for the object's OBJ_START_ARG (Stage 08's overlay).
; HOW: A jump table by OBJ_START_ARG.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByArg0D2DJumps = $C234 ; the routine for each OBJ_START_ARG, a word each
ActByArg0D2D:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:ActByArg0D2DJumps,y
    sta z:zTemp00
    lda a:ActByArg0D2DJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D2D:31F ActBySubstate0D2D
; @run 81:C31F
; WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActBySubstate0D2DJumps = $C331 ; the routine for each substate, a word each
ActBySubstate0D2D:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:ActBySubstate0D2DJumps,y
    sta z:zTemp00
    lda a:ActBySubstate0D2DJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D2D:37D HitRoutineByArg0D2D
; @run 81:C37D
; WHAT IT DOES: Clears the object's invulnerability timer; when it has taken a hit of 1 to 4 (not a damaging one)
; this frame, runs the routine a table gives by OBJ_START_ARG, else returns C clear (Stage 08's overlay).
; HOW: A store, a compare of OBJ_HIT_TAKEN, then a jump table by OBJ_START_ARG or CLC.
; WAITS: no.
; Entry: X the object. Exit: C clear when no such hit was taken; else as the routine for OBJ_START_ARG leaves it.
HitRoutineByArg0D2DJumps = $C25C ; the routine for each OBJ_START_ARG, a word each
HitRoutineByArg0D2D:
    stz a:OBJ_INVULN_TIMER,x
    lda a:OBJ_HIT_TAKEN,x
    beq HitRoutineByArg0D2D_at0
    cmp #$05
    bcs HitRoutineByArg0D2D_at0
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:HitRoutineByArg0D2DJumps,y
    sta z:zTemp00
    lda a:HitRoutineByArg0D2DJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)
HitRoutineByArg0D2D_at0:
    clc
    rts

; @routine 0D2D:3BE ReturnCarryClear0D2D
; @run 81:C3BE
; WHAT IT DOES: Returns with C clear; the piece CheckPlayerStillNoPad0D2D ends in (Stage 08's overlay).
; HOW: CLC.
; WAITS: no.
; Entry: nothing in the registers. Exit: C clear.
ReturnCarryClear0D2D:
    clc
    rts

; @routine 0D2D:3C0 CheckPlayerStillNoPad0D2D
; @run 81:C3C0
; WHAT IT DOES: Says whether the player (slot 0) is in state 0 with no button or direction held (Stage 08's
; overlay).
; HOW: A compare of slot 0's state and a test of zPadHeld.
; WAITS: no.
; Entry: nothing in the registers. Exit: C set when the player is in state 0 and zPadHeld is 0; else C clear.
CheckPlayerStillNoPad0D2D:
    lda a:OBJ_STATE
    cmp #$00
    bne CheckPlayerStillNoPad0D2D_at0
    lda z:zPadHeld
    bne ReturnCarryClear0D2D
    sec
    rts
CheckPlayerStillNoPad0D2D_at0:
    clc
    rts

; @routine 0D2D:3CF ActByState0D2D_3CF
; @run 81:C3CF
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D2D_3CFJumps = $C3E1 ; the routine for each state, a word each
ActByState0D2D_3CF:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D2D_3CFJumps,y
    sta z:zTemp00
    lda a:ActByState0D2D_3CFJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D2D:493 MoveAndSwingByCosine0D2D
; @run 81:C493
; WHAT IT DOES: Moves the object by its speed and swings the 16-bit angle OBJ_WORK_D-E by a cosine of OBJ_WORK_F,
; which steps by 2; when that wraps to 0 the angle goes back to $40; the tail of SwingLinkByCosine0D2D (Stage 08's
; overlay).
; HOW: ApplyVelocity, ScaleByCosine of a word from a table by OBJ_START_ARG, a 16-bit add, an add of 2.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveAndSwingByCosine0D2DSwings = $C4D0 ; the swing's size, a word per OBJ_START_ARG
MoveAndSwingByCosine0D2D:
    jsr VecApplyVelocity
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:MoveAndSwingByCosine0D2DSwings,y
    sta z:zTemp00
    lda a:MoveAndSwingByCosine0D2DSwings+1,y
    sta z:zTemp01
    lda a:OBJ_WORK_F,x
    jsr VecScaleByCosine
    lda a:OBJ_WORK_E,x
    clc
    adc z:SpriteFlipMask
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    adc z:zPlaneRowsLeft
    sta a:OBJ_WORK_D,x
    lda a:OBJ_WORK_F,x
    clc
    adc #$02
    sta a:OBJ_WORK_F,x
    bne MoveAndSwingByCosine0D2D_at0
    lda #$40
    sta a:OBJ_WORK_D,x
    stz a:OBJ_WORK_E,x
MoveAndSwingByCosine0D2D_at0:
    rts

; @routine 0D2D:4D6 ActByState0D2D_4D6
; @run 81:C4D6
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D2D_4D6Jumps = $C4E8 ; the routine for each state, a word each
ActByState0D2D_4D6:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D2D_4D6Jumps,y
    sta z:zTemp00
    lda a:ActByState0D2D_4D6Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D2D:51B FollowLinkOrLeaveScreen0D2D
; @run 81:C51B
; WHAT IT DOES: With OBJ_WORK_G 0, clears the object once its X is $160 or more or below -$60; else clears it
; unless the slot below holds kind $C4, and otherwise puts it at that slot's place, sets a speed of $0800 at angle
; OBJ_WORK_D, and moves and swings it (MoveAndSwingByCosine0D2D) (Stage 08's overlay).
; HOW: Compares of the X bytes and ClearObject; a kind compare, CopyObjectPlaceBack, SetVelocityFromAngle, then the
; piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FollowLinkOrLeaveScreen0D2D:
    lda a:OBJ_WORK_G,x
    bne FollowLinkOrLeaveScreen0D2D_at0
    lda a:OBJ_X_HI,x
    beq FollowLinkOrLeaveScreen0D2D_at1
    bmi FollowLinkOrLeaveScreen0D2D_at2
    lda a:OBJ_X_LO,x
    cmp #$60
    bcc FollowLinkOrLeaveScreen0D2D_at3
    jmp VecClearObject
FollowLinkOrLeaveScreen0D2D_at3:
    rts
FollowLinkOrLeaveScreen0D2D_at2:
    lda a:OBJ_X_LO,x
    cmp #$A0
    bcs FollowLinkOrLeaveScreen0D2D_at4
    jmp VecClearObject
FollowLinkOrLeaveScreen0D2D_at4:
    rts
FollowLinkOrLeaveScreen0D2D_at0:
    lda a:OBJ_KIND-1,x
    cmp #$C4
    beq FollowLinkOrLeaveScreen0D2D_at5
    jmp VecClearObject
FollowLinkOrLeaveScreen0D2D_at5:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    stz z:zTemp00
    lda #$08
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jmp MoveAndSwingByCosine0D2D
FollowLinkOrLeaveScreen0D2D_at1:
    rts

; @routine 0D5A:44D RunKindsInBankC0D5A
; @run 70:AC4D
; WHAT IT DOES: Maps bank $0C past zRamBaseBank at $C000, runs ActByKind59To5C0D6C there, then maps bank 9 back
; (Stage 08's overlay).
; HOW: TAM of MPR6, a call, then the piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunKindsInBankC0D5A:
    lda #$0C
    clc
    adc z:zRamBaseBank
    tam #$40
    jsr ActByKind59To5C0D6C

; @routine 0D5A:457 MapBank9AtC000_0D5A
; @run 70:AC57
; WHAT IT DOES: Maps bank 9 past zRamBaseBank at $C000 (Stage 08's overlay).
; HOW: An add and TAM of MPR6.
; WAITS: no.
; Entry: D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left them.
MapBank9AtC000_0D5A:
    lda #$09
    clc
    adc z:zRamBaseBank
    tam #$40
    rts

; @routine 0D5A:460 RunStateTableInBank19_0D5A
; @run 70:AC60
; WHAT IT DOES: Maps bank $19 past zRamBaseBank at $C000, runs ActByState0D2D_3CF there, then maps bank 9 back
; (Stage 08's overlay).
; HOW: TAM of MPR6, a call, then MapBank9AtC000_0D5A.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunStateTableInBank19_0D5A:
    lda #$19
    clc
    adc z:zRamBaseBank
    tam #$40
    jsr ActByState0D2D_3CF
    bra MapBank9AtC000_0D5A

; @routine 0D5A:46C RunOtherStateTableInBank19_0D5A
; @run 70:AC6C
; WHAT IT DOES: Maps bank $19 past zRamBaseBank at $C000, runs ActByState0D2D_4D6 there, then maps bank 9 back
; (Stage 08's overlay).
; HOW: TAM of MPR6, a call, then MapBank9AtC000_0D5A.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunOtherStateTableInBank19_0D5A:
    lda #$19
    clc
    adc z:zRamBaseBank
    tam #$40
    jsr ActByState0D2D_4D6
    bra MapBank9AtC000_0D5A

; @routine 0D5B:0F6 BecomeKind9CLowOrByState0D5B
; @run 70:B0F6
; WHAT IT DOES: With OBJ_START_ARG not 0 and its Y $BC or more, turns the object into kind $9C in state 0; else
; runs the routine for its state (Stage 08's overlay).
; HOW: Two compares and two stores, or a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BecomeKind9CLowOrByState0D5BJumps = $B11D ; the routine for each state, a word each
BecomeKind9CLowOrByState0D5B:
    lda a:OBJ_START_ARG,x
    beq BecomeKind9CLowOrByState0D5B_at0
    lda a:OBJ_Y_LO,x
    cmp #$BC
    bcc BecomeKind9CLowOrByState0D5B_at0
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
BecomeKind9CLowOrByState0D5B_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:BecomeKind9CLowOrByState0D5BJumps,y
    sta z:zTemp00
    lda a:BecomeKind9CLowOrByState0D5BJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5B:180 FacePlayerThenActByDistance0D5B
; @run 70:B180
; WHAT IT DOES: Turns the object to face the player, clears OBJ_WORK_D, runs ActByDistanceToPlayer0D5B and steps
; the state on (Stage 08's overlay).
; HOW: SideOfPlayer's bit 0 into OBJ_FACING_LEFT, a store, a call, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FacePlayerThenActByDistance0D5B:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_WORK_D,x
    jsr ActByDistanceToPlayer0D5B
    inc a:OBJ_STATE,x
    rts

; @routine 0D5B:2F4 ActAndKeepAngleHistory0D5B
; @run 70:B2F4
; WHAT IT DOES: Runs State4WhenHit0D5B, FlashChainWhileInvuln0D5B and ActByState0D5B_356, then shifts a 128-byte
; history along by one and puts OBJ_WORK_D plus $70 at its start (Stage 08's overlay).
; HOW: Three calls, a copy loop down from $7F, an add and a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActAndKeepAngleHistory0D5BHistory = $B3A2 ; the history at $B3A3, newest first: OBJ_WORK_D plus $70 for each frame; read one byte below
ActAndKeepAngleHistory0D5B:
    jsr State4WhenHit0D5B
    jsr FlashChainWhileInvuln0D5B
    jsr ActByState0D5B_356
    ldy #$7F
ActAndKeepAngleHistory0D5B_at0:
    lda a:ActAndKeepAngleHistory0D5BHistory,y
    sta a:ActAndKeepAngleHistory0D5BHistory+1,y
    dey
    bne ActAndKeepAngleHistory0D5B_at0
    lda a:OBJ_WORK_D,x
    clc
    adc #$70
    sta a:ActAndKeepAngleHistory0D5BHistory+1
    rts

; @routine 0D5B:329 FlashChainWhileInvuln0D5B
; @run 70:B329
; WHAT IT DOES: In state 1, sets OBJ_WORK_J of the object and the nine slots after it to 2 while its
; invulnerability timer runs and to 0 otherwise, and halves OBJ_WORK_O when it took a damaging hit (Stage 08's
; overlay).
; HOW: A compare, a loop of ten stores, a compare of OBJ_HIT_TAKEN and LSR.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlashChainWhileInvuln0D5B:
    lda a:OBJ_STATE,x
    cmp #$01
    bne FlashChainWhileInvuln0D5B_at0
    stz z:zTemp00
    lda a:OBJ_INVULN_TIMER,x
    beq FlashChainWhileInvuln0D5B_at1
    lda #$02
    sta z:zTemp00
FlashChainWhileInvuln0D5B_at1:
    txa
    tay
    lda #$0A
    sta z:zTemp01
FlashChainWhileInvuln0D5B_at2:
    lda z:zTemp00
    sta a:OBJ_WORK_J,y
    iny
    dec z:zTemp01
    bne FlashChainWhileInvuln0D5B_at2
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc FlashChainWhileInvuln0D5B_at0
    lsr a:OBJ_WORK_O,x
FlashChainWhileInvuln0D5B_at0:
    rts

; @routine 0D5B:356 ActByState0D5B_356
; @run 70:B356
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5B_356Jumps = $B368 ; the routine for each state, a word each
ActByState0D5B_356:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5B_356Jumps,y
    sta z:zTemp00
    lda a:ActByState0D5B_356Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5B:423 SwingChainThenPickRandom0D5B
; @run 70:B423
; WHAT IT DOES: Runs NextStateByPattern0D5B, MoveChainOfNine0D5B, EaseWorkFToTarget0D5B and ScaleByRecordWords0D5B,
; adds the scaled word to the angle OBJ_WORK_D-E and the record's first word to OBJ_WORK_K-L; when that overflows,
; puts a random 0 to 3 in OBJ_WORK_M and clears OBJ_WORK_D, E, K and L (Stage 08's overlay).
; HOW: Four calls, two 16-bit adds, NextRandom and stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SwingChainThenPickRandom0D5B:
    jsr NextStateByPattern0D5B
    jsr MoveChainOfNine0D5B
    jsr EaseWorkFToTarget0D5B
    jsr ScaleByRecordWords0D5B
    lda a:OBJ_WORK_E,x
    clc
    adc z:SpriteFlipMask
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    adc z:zPlaneRowsLeft
    sta a:OBJ_WORK_D,x
    lda a:OBJ_WORK_L,x
    clc
    adc z:zSetUpColumnsLeft
    sta a:OBJ_WORK_L,x
    lda a:OBJ_WORK_K,x
    adc z:zTransposeRowsLeft
    sta a:OBJ_WORK_K,x
    bcc SwingChainThenPickRandom0D5B_at0
    jsr VecNextRandom
    and #$03
    sta a:OBJ_WORK_M,x
    stz a:OBJ_WORK_D,x
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_K,x
    stz a:OBJ_WORK_L,x
SwingChainThenPickRandom0D5B_at0:
    rts

; @routine 0D5B:478 AngleToPlayerFromNinthSlot0D5B
; @run 70:B478
; WHAT IT DOES: Gives the angle to the player from the slot nine after the object (Stage 08's overlay).
; HOW: X plus 9, AngleToPlayer, X back from zRichterObject.
; WAITS: no.
; Entry: X the object. Exit: A the angle; X the object.
AngleToPlayerFromNinthSlot0D5B:
    stx z:zClimbStepBefore
    txa
    clc
    adc #$09
    tax
    jsr VecAngleToPlayer
    ldx z:zClimbStepBefore
    rts

; @routine 0D5B:59A MoveChainThenBySubstate0D5B
; @run 70:B59A
; WHAT IT DOES: Runs MoveChainOfNine0D5B and EaseWorkFToTarget0D5B, then the routine for the object's substate
; (Stage 08's overlay).
; HOW: Two calls, then a jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveChainThenBySubstate0D5BJumps = $B5B2 ; the routine for each substate, a word each
MoveChainThenBySubstate0D5B:
    jsr MoveChainOfNine0D5B
    jsr EaseWorkFToTarget0D5B
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:MoveChainThenBySubstate0D5BJumps,y
    sta z:zTemp00
    lda a:MoveChainThenBySubstate0D5BJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5B:5BC AimChainIfPlayerBelow0D5B
; @run 70:B5BC
; WHAT IT DOES: When the angle from the ninth slot to the player is $50 to $AF, puts the nine slots after the
; object in state 2, sets OBJ_WORK_I $60 and steps the substate on; else puts the object in state 1 (Stage 08's
; overlay).
; HOW: AngleToPlayerFromNinthSlot0D5B, two compares, the piece, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AimChainIfPlayerBelow0D5B:
    jsr AngleToPlayerFromNinthSlot0D5B
    cmp #$50
    bcc AimChainIfPlayerBelow0D5B_at0
    cmp #$B0
    bcs AimChainIfPlayerBelow0D5B_at0
    jsr ChainToState2_0D5B
    lda #$60
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
    rts
AimChainIfPlayerBelow0D5B_at0:
    lda #$01
    sta a:OBJ_STATE,x
    rts

; @routine 0D5B:5D9 ChainToState2_0D5B
; @run 70:B5D9
; WHAT IT DOES: Puts the nine slots after the object in state 2, substate 0 (Stage 08's overlay).
; HOW: A loop of two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ChainToState2_0D5B:
    txa
    tay
    iny
    lda #$09
    sta z:zTemp01
ChainToState2_0D5B_at0:
    lda #$02
    sta a:OBJ_STATE,y
    cla
    sta a:OBJ_SUBSTATE,y
    iny
    dec z:zTemp01
    bne ChainToState2_0D5B_at0
    rts

; @routine 0D5B:5EF TurnAngleToZeroBy2_0D5B
; @run 70:B5EF
; WHAT IT DOES: Counts OBJ_WORK_I down and turns the angle OBJ_WORK_D by 2 toward 0, keeping it even; on reaching 0
; the substate steps on (Stage 08's overlay).
; HOW: DEC, TurnAngleToward, AND #$FE, a compare, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnAngleToZeroBy2_0D5B:
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
    bne TurnAngleToZeroBy2_0D5B_at0
    inc a:OBJ_SUBSTATE,x
TurnAngleToZeroBy2_0D5B_at0:
    rts

; @routine 0D5B:60D AimChainAfterWait0D5B
; @run 70:B60D
; WHAT IT DOES: Counts OBJ_WORK_I down; at zero aims the chain at the player (AimChainAtPlayer0D5B), plays sound
; $C3, sets OBJ_WORK_I $30 and steps the substate on (Stage 08's overlay).
; HOW: DEC, a call, SoundCommand, stores, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AimChainAfterWait0D5B:
    dec a:OBJ_WORK_I,x
    bne AimChainAfterWait0D5B_at0
    jsr AimChainAtPlayer0D5B
    lda #$C3
    jsr VecSoundCommand
    lda #$30
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
AimChainAfterWait0D5B_at0:
    rts

; @routine 0D5B:623 AimChainAtPlayer0D5B
; @run 70:B623
; WHAT IT DOES: Puts the angle from the ninth slot to the player, held to $50 to $B0 and rounded down to a multiple
; of 8, in OBJ_WORK_H of the object and of the nine slots after it (Stage 08's overlay).
; HOW: AngleToPlayerFromNinthSlot0D5B, the clamp piece, AND #$F8, a loop of stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
AimChainAtPlayer0D5B:
    jsr AngleToPlayerFromNinthSlot0D5B
    jsr ClampAngle50ToB0_0D5B
    and #$F8
    sta a:OBJ_WORK_H,x
    sta z:zTemp00
    txa
    tay
    iny
    lda #$09
    sta z:zTemp01
AimChainAtPlayer0D5B_at0:
    lda z:zTemp00
    sta a:OBJ_WORK_H,y
    iny
    dec z:zTemp01
    bne AimChainAtPlayer0D5B_at0
    rts

; @routine 0D5B:642 ClampAngle50ToB0_0D5B
; @run 70:B642
; WHAT IT DOES: Holds the angle in A to $50 to $B0 (Stage 08's overlay).
; HOW: Two compares.
; WAITS: no.
; Entry: A the angle. Exit: A the angle held to $50 to $B0.
ClampAngle50ToB0_0D5B:
    cmp #$50
    bcc ClampAngle50ToB0_0D5B_at0
    cmp #$B0
    bcs ClampAngle50ToB0_0D5B_at1
    rts
ClampAngle50ToB0_0D5B_at0:
    lda #$50
    rts
ClampAngle50ToB0_0D5B_at1:
    lda #$B0
    rts

; @routine 0D5B:671 TurnToZeroThenState1_0D5B
; @run 70:B671
; WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward 0; on reaching it puts the object in state 1, substate 0,
; with OBJ_WORK_E, K and L cleared (Stage 08's overlay).
; HOW: TurnAngleToward, a compare, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnToZeroThenState1_0D5B:
    stz z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    cmp z:zTemp02
    bne TurnToZeroThenState1_0D5B_at0
    lda #$01
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_K,x
    stz a:OBJ_WORK_L,x
TurnToZeroThenState1_0D5B_at0:
    rts

; @routine 0D5B:698 DropChainThenFall0D5B
; @run 70:B698
; WHAT IT DOES: In substate 0 puts the nine slots after the object in state 3 with its OBJ_START_ARG in their
; OBJ_WORK_L, gives it frame 6, OBJ_WORK_O $60, no hit box, no X speed and a Y speed of -4, and steps the substate
; on; later counts OBJ_WORK_O down, clearing it at zero, and meanwhile runs SpawnKind9COnBeat0D5B and falls under
; gravity (Stage 08's overlay).
; HOW: The piece and stores; or DEC, ClearObject, a call, ApplyGravityUnlimited and ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
DropChainThenFall0D5B:
    lda a:OBJ_SUBSTATE,x
    bne DropChainThenFall0D5B_at0
    jsr ChainToState3_0D5B
    lda #$06
    sta a:OBJ_FRAME,x
    lda #$60
    sta a:OBJ_WORK_O,x
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
    rts
DropChainThenFall0D5B_at0:
    dec a:OBJ_WORK_O,x
    bne DropChainThenFall0D5B_at1
    jmp VecClearObject
DropChainThenFall0D5B_at1:
    jsr SpawnKind9COnBeat0D5B
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0D5B:6D0 ChainToState3_0D5B
; @run 70:B6D0
; WHAT IT DOES: Puts the nine slots after the object in state 3, substate 0, each with the object's OBJ_START_ARG
; in its OBJ_WORK_L (Stage 08's overlay).
; HOW: A loop of three stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ChainToState3_0D5B:
    txa
    tay
    iny
    lda #$09
    sta z:zTemp01
ChainToState3_0D5B_at0:
    lda #$03
    sta a:OBJ_STATE,y
    lda a:OBJ_START_ARG,x
    sta a:OBJ_WORK_L,y
    cla
    sta a:OBJ_SUBSTATE,y
    iny
    dec z:zTemp01
    bne ChainToState3_0D5B_at0
    rts

; @routine 0D5B:715 ActByState0D5B_715
; @run 70:B715
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5B_715Jumps = $B727 ; the routine for each state, a word each
ActByState0D5B_715:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5B_715Jumps,y
    sta z:zTemp00
    lda a:ActByState0D5B_715Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5B:7F1 TurnToAimThenState1_0D5B
; @run 70:B7F1
; WHAT IT DOES: Turns the angle OBJ_WORK_D by 8 toward OBJ_WORK_H and counts OBJ_WORK_I down; at zero clears
; OBJ_WORK_J and OBJ_WORK_G and puts the object in state 1, substate 0 (Stage 08's overlay).
; HOW: TurnAngleToward, DEC, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnToAimThenState1_0D5B:
    lda a:OBJ_WORK_H,x
    sta z:zTemp02
    lda #$08
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    sta a:OBJ_WORK_D,x
    dec a:OBJ_WORK_I,x
    bne TurnToAimThenState1_0D5B_at0
    stz a:OBJ_WORK_J,x
    stz a:OBJ_WORK_G,x
    lda #$01
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
TurnToAimThenState1_0D5B_at0:
    rts

; @routine 0D5C:019 ActBySubstate0D5C_019
; @run 70:B819
; WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActBySubstate0D5C_019Jumps = $B82B ; the routine for each substate, a word each
ActBySubstate0D5C_019:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:ActBySubstate0D5C_019Jumps,y
    sta z:zTemp00
    lda a:ActBySubstate0D5C_019Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5C:082 FallToFloorOrClear0D5C
; @run 70:B882
; WHAT IT DOES: Clears the object once its Y high byte is not 0; else falls and lands with probe height 0, and on
; landing sets OBJ_WORK_M $20 and steps the substate on (Stage 08's overlay).
; HOW: A test, ClearObject or StopOnLanding, a store and INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FallToFloorOrClear0D5C:
    lda a:OBJ_Y_HI,x
    beq FallToFloorOrClear0D5C_at0
    jmp VecClearObject
FallToFloorOrClear0D5C_at0:
    lda #$00
    jsr VecStopOnLanding
    bcc FallToFloorOrClear0D5C_at1
    lda #$20
    sta a:OBJ_WORK_M,x
    inc a:OBJ_SUBSTATE,x
FallToFloorOrClear0D5C_at1:
    rts

; @routine 0D5C:105 StartWithCounts8And9_0D5C
; @run 70:B905
; WHAT IT DOES: Starts the object from its record with OBJ_WORK_F 8 and OBJ_WORK_E 9 (Stage 08's overlay).
; HOW: LoadObjectRecord and two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWithCounts8And9_0D5C:
    jsr VecLoadObjectRecord
    lda #$08
    sta a:OBJ_WORK_F,x
    lda #$09
    sta a:OBJ_WORK_E,x
    rts

; @routine 0D5C:275 ActBySubstate0D5C_275
; @run 70:BA75
; WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActBySubstate0D5C_275Jumps = $BA87 ; the routine for each substate, a word each
ActBySubstate0D5C_275:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:ActBySubstate0D5C_275Jumps,y
    sta z:zTemp00
    lda a:ActBySubstate0D5C_275Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5C:3ED ActByState0D5C_3ED
; @run 70:BBED
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5C_3EDJumps = $BBFF ; the routine for each state, a word each
ActByState0D5C_3ED:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5C_3EDJumps,y
    sta z:zTemp00
    lda a:ActByState0D5C_3EDJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5C:405 StartFromRecord0D5C
; @run 70:BC05
; WHAT IT DOES: Starts the object from its record (Stage 08's overlay).
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartFromRecord0D5C:
    jsr VecLoadObjectRecord
    rts

; @routine 0D5C:51C StartFromRecordInState0_0D5C
; @run 70:BD1C
; WHAT IT DOES: In state 0, starts the object from its record (Stage 08's overlay).
; HOW: A test and LoadObjectRecord.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartFromRecordInState0_0D5C:
    lda a:OBJ_STATE,x
    bne StartFromRecordInState0_0D5C_at0
    jsr VecLoadObjectRecord
StartFromRecordInState0_0D5C_at0:
    rts

; @routine 0D5C:540 ClearTemp00Bit3ThenByState0D5C
; @run 70:BD40
; WHAT IT DOES: Clears bit 3 of zTemp00 and falls into ActByState0D5C_542, which overwrites it (Stage 08's
; overlay).
; HOW: RMB3, then the next routine.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ClearTemp00Bit3ThenByState0D5C:
    rmb3 z:zTemp00

; @routine 0D5C:542 ActByState0D5C_542
; @run 70:BD42
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5C_542Jumps = $BD54 ; the routine for each state, a word each
ActByState0D5C_542:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5C_542Jumps,y
    sta z:zTemp00
    lda a:ActByState0D5C_542Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5D:3A9 ActAndThrowPartBelow0D5D
; @run 71:C3A9
; WHAT IT DOES: Steps OBJ_WORK_G, runs FacePlayerInStates0D5D, StartDeathFall0D5D, HitBoxWidthByFrame0D5D and
; ActByState0D5D_45C; then, when the slot below holds kind $38 in a state below 2, puts it at the object's place
; with the object's facing and the X and Y speeds a table gives by the object's frame less $1E (X turned round when
; facing left), and moves it once (Stage 08's overlay).
; HOW: INC, four calls, compares of the slot below, CopyObjectPlace, a pair read by twice the frame less $1E, a
; negate, ApplyVelocity on that slot.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActAndThrowPartBelow0D5DSpeeds = $C8E4 ; X speed high, Y speed high for each frame from $1E
ActAndThrowPartBelow0D5D:
    inc a:OBJ_WORK_G,x
    jsr FacePlayerInStates0D5D
    jsr StartDeathFall0D5D
    jsr HitBoxWidthByFrame0D5D
    jsr ActByState0D5D_45C
    lda a:OBJ_KIND-1,x
    cmp #$38
    bne ActAndThrowPartBelow0D5D_at0
    lda a:OBJ_STATE-1,x
    cmp #$02
    bcs ActAndThrowPartBelow0D5D_at0
    txa
    tay
    dey
    jsr VecCopyObjectPlace
    stx z:zClimbStepBefore
    lda a:OBJ_FRAME,x
    sec
    sbc #$1E
    asl a
    tax
    lda a:ActAndThrowPartBelow0D5DSpeeds,x
    sta a:OBJ_VX_HI,y
    lda a:ActAndThrowPartBelow0D5DSpeeds+1,x
    sta a:OBJ_VY_HI,y
    ldx z:zClimbStepBefore
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT-1,x
    beq ActAndThrowPartBelow0D5D_at1
    lda a:OBJ_VX_HI,y
    eor #$FF
    inc a
    sta a:OBJ_VX_HI,y
ActAndThrowPartBelow0D5D_at1:
    sxy
    jsr VecApplyVelocity
    sxy
ActAndThrowPartBelow0D5D_at0:
    rts

; @routine 0D5D:412 StartDeathFall0D5D
; @run 71:C412
; WHAT IT DOES: Once the object is dead (OBJ_HIT_STATUS bit 7) outside state 5, turns its hit box off, clears the
; slot below it, restarts its animation, and puts it in state 5 with no X speed, a Y speed of -$1C0 and OBJ_WORK_D
; $40 (Stage 08's overlay).
; HOW: A compare, a test, ClearObjectInY on X less 1, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartDeathFall0D5D:
    lda a:OBJ_STATE,x
    cmp #$05
    beq StartDeathFall0D5D_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq StartDeathFall0D5D_at0
    stz a:OBJ_HITBOX_ON,x
    txa
    tay
    dey
    jsr VecClearObjectInY
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    lda #$40
    sta a:OBJ_WORK_D,x
    lda #$05
    sta a:OBJ_STATE,x
StartDeathFall0D5D_at0:
    rts

; @routine 0D5D:45C ActByState0D5D_45C
; @run 71:C45C
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5D_45CJumps = $C46E ; the routine for each state, a word each
ActByState0D5D_45C:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5D_45CJumps,y
    sta z:zTemp00
    lda a:ActByState0D5D_45CJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5D:4A2 LandThenNextState0D5D
; @run 71:C4A2
; WHAT IT DOES: Falls and lands with probe height 6; on landing steps the state on (Stage 08's overlay).
; HOW: StopOnLanding and INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LandThenNextState0D5D:
    lda #$06
    jsr VecStopOnLanding
    bcc LandThenNextState0D5D_at0
    inc a:OBJ_STATE,x
LandThenNextState0D5D_at0:
    rts

; @routine 0D5D:4AD WalkForwardUntilNear0D5D
; @run 71:C4AD
; WHAT IT DOES: Walks the way it faces (WalkBySpeedIndex0D5D with OBJ_WORK_F its facing); while waiting on its
; pattern (WaitThenStepPattern0D5D C clear), goes to state 3 once the player is less than $4E away (Stage 08's
; overlay).
; HOW: A call, a store, a call, DistanceXToPlayer and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkForwardUntilNear0D5D:
    jsr WalkBySpeedIndex0D5D
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_F,x
    jsr WaitThenStepPattern0D5D
    bcs WalkForwardUntilNear0D5D_at0
    jsr VecDistanceXToPlayer
    cmp #$4E
    bcs WalkForwardUntilNear0D5D_at0
    lda #$03
    sta a:OBJ_STATE,x
WalkForwardUntilNear0D5D_at0:
    rts

; @routine 0D5D:546 WalkBackUntilFar0D5D
; @run 71:C546
; WHAT IT DOES: Walks against the way it faces (WalkBySpeedIndex0D5D with OBJ_WORK_F its facing flipped); while
; waiting on its pattern (WaitThenStepPattern0D5D C clear), goes to state 2 once the player is $64 or more away
; (Stage 08's overlay).
; HOW: A call, an EOR and a store, a call, DistanceXToPlayer and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkBackUntilFar0D5D:
    jsr WalkBySpeedIndex0D5D
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_F,x
    jsr WaitThenStepPattern0D5D
    bcs WalkBackUntilFar0D5D_at0
    jsr VecDistanceXToPlayer
    cmp #$64
    bcc WalkBackUntilFar0D5D_at0
    lda #$02
    sta a:OBJ_STATE,x
WalkBackUntilFar0D5D_at0:
    rts

; @routine 0D5D:563 ActBySubstate0D5D
; @run 71:C563
; WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActBySubstate0D5DJumps = $C575 ; the routine for each substate, a word each
ActBySubstate0D5D:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:ActBySubstate0D5DJumps,y
    sta z:zTemp00
    lda a:ActBySubstate0D5DJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5D:5ED PlayListC60FAndShoot0D5D
; @run 71:C5ED
; WHAT IT DOES: Plays the frame list at $C60F; when it ends restarts in state 3 (RestartInState3_0D5D), and when
; the frame changes runs ShootKind37AtStep5_0D5D (Stage 08's overlay).
; HOW: StepObjectFrames, then a jump or the piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayListC60FAndShoot0D5D:
    lda #$0F
    sta z:zTemp10
    lda #$C6
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq RestartInState3_0D5D
    bcc PlayListC60FAndShoot0D5D_at0
    jmp ShootKind37AtStep5_0D5D
PlayListC60FAndShoot0D5D_at0:
    rts

; @routine 0D5D:61C ShootKind37AtStep5_0D5D
; @run 71:C61C
; WHAT IT DOES: At step 5 of the object's frame list, plays sound 6 and starts an object of kind $37 at its place
; in a free slot from $30, with its facing and the substate less 1 in OBJ_WORK_E (Stage 08's overlay).
; HOW: A compare, SoundCommand, FindFreeSlotFrom30, StartObjectAtPlace, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ShootKind37AtStep5_0D5D:
    lda a:OBJ_ANIM_STEP,x
    cmp #$05
    bne ShootKind37AtStep5_0D5D_at0
    lda #$06
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom30
    bcc ShootKind37AtStep5_0D5D_at0
    lda #$37
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda a:OBJ_SUBSTATE,x
    dec a
    sta a:OBJ_WORK_E,y
ShootKind37AtStep5_0D5D_at0:
    rts

; @routine 0D5D:640 PlayListC653AndShoot0D5D
; @run 71:C640
; WHAT IT DOES: Plays the frame list at $C653; when it ends restarts in state 3 (RestartInState3_0D5D), and when
; the frame changes runs ShootKind37AtStep5_0D5D (Stage 08's overlay).
; HOW: StepObjectFrames, then a jump or the piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayListC653AndShoot0D5D:
    lda #$53
    sta z:zTemp10
    lda #$C6
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq RestartInState3_0D5D
    bcc PlayListC653AndShoot0D5D_at0
    jmp ShootKind37AtStep5_0D5D
PlayListC653AndShoot0D5D_at0:
    rts

; @routine 0D5D:65E PlayListC671AndShoot0D5D
; @run 71:C65E
; WHAT IT DOES: Plays the frame list at $C671; when it ends restarts in state 3 (RestartInState3_0D5D), and when
; the frame changes runs ShootKind37AtStep5_0D5D (Stage 08's overlay).
; HOW: StepObjectFrames, then a jump or the piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayListC671AndShoot0D5D:
    lda #$71
    sta z:zTemp10
    lda #$C6
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq RestartInState3_0D5D
    bcc PlayListC671AndShoot0D5D_at0
    jmp ShootKind37AtStep5_0D5D
PlayListC671AndShoot0D5D_at0:
    rts

; @routine 0D5D:6CD BounceAndShootThenClear0D5D
; @run 71:C6CD
; WHAT IT DOES: Plays the frame list at $C71B and counts OBJ_WORK_D down, clearing the object at zero; else runs
; ShootKind9COnCount0D5D and falls with probe height 6, and on landing with frame $38 bounces at the Y speed a
; table gives by OBJ_WORK_H, which steps on up to 6 (Stage 08's overlay).
; HOW: StepObjectFrames, DEC, ClearObject, a call, StopOnLanding, a compare, a word table read, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BounceAndShootThenClear0D5DSpeeds = $C70D ; the bounce's Y speed for each OBJ_WORK_H, a word each
BounceAndShootThenClear0D5D:
    lda #$C7
    sta z:zTemp11
    lda #$1B
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_D,x
    bne BounceAndShootThenClear0D5D_at0
    jmp VecClearObject
BounceAndShootThenClear0D5D_at0:
    jsr ShootKind9COnCount0D5D
    lda #$06
    jsr VecStopOnLanding
    bcc BounceAndShootThenClear0D5D_at1
    lda a:OBJ_FRAME,x
    cmp #$38
    bne BounceAndShootThenClear0D5D_at1
    lda a:OBJ_WORK_H,x
    asl a
    tay
    lda a:BounceAndShootThenClear0D5DSpeeds+1,y
    sta a:OBJ_VY_HI,x
    lda a:BounceAndShootThenClear0D5DSpeeds,y
    sta a:OBJ_VY_LO,x
    lda a:OBJ_WORK_H,x
    cmp #$06
    beq BounceAndShootThenClear0D5D_at1
    inc a:OBJ_WORK_H,x
BounceAndShootThenClear0D5D_at1:
    rts

; @routine 0D5D:776 ActByState0D5D_776
; @run 71:C776
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5D_776Jumps = $C788 ; the routine for each state, a word each
ActByState0D5D_776:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5D_776Jumps,y
    sta z:zTemp00
    lda a:ActByState0D5D_776Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5D:790 StartWithSpeedByWorkE0D5D
; @run 71:C790
; WHAT IT DOES: Starts the object from its record, runs MoveByDirectionIndex0D5D and sets OBJ_WORK_F $48; with
; OBJ_WORK_E not 2 gives it the X speed a table gives by OBJ_WORK_E and no Y speed, and with 2 the X speed a table
; gives by the player's distance over 32 and a Y speed of -5; facing left, the X speed is turned round and
; OBJ_WORK_D set 1 (Stage 08's overlay).
; HOW: LoadObjectRecord, a call, word table reads, DistanceXToPlayer and LSRs, NegateVelocityX.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWithSpeedByWorkE0D5DSpeeds = $C7E1 ; the X speed for each OBJ_WORK_E, a word each
StartWithSpeedByWorkE0D5DLeapSpeeds = $C7E5 ; the X speed for each 32 pixels of distance to the player, a word each
StartWithSpeedByWorkE0D5D:
    jsr VecLoadObjectRecord
    jsr MoveByDirectionIndex0D5D
    lda #$48
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    cmp #$02
    beq StartWithSpeedByWorkE0D5D_at0
    asl a
    tay
    lda a:StartWithSpeedByWorkE0D5DSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:StartWithSpeedByWorkE0D5DSpeeds,y
    sta a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
StartWithSpeedByWorkE0D5D_at2:
    lda a:OBJ_FACING_LEFT,x
    beq StartWithSpeedByWorkE0D5D_at1
    jsr VecNegateVelocityX
    lda #$01
    sta a:OBJ_WORK_D,x
StartWithSpeedByWorkE0D5D_at1:
    rts
StartWithSpeedByWorkE0D5D_at0:
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:StartWithSpeedByWorkE0D5DLeapSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:StartWithSpeedByWorkE0D5DLeapSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    bra StartWithSpeedByWorkE0D5D_at2

; @routine 0D5E:0A6 ActByState0D5E_0A6
; @run 71:C8A6
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5E_0A6Jumps = $C8B8 ; the routine for each state, a word each
ActByState0D5E_0A6:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5E_0A6Jumps,y
    sta z:zTemp00
    lda a:ActByState0D5E_0A6Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5E:126 FallUnderGravity0D5E
; @run 71:C926
; WHAT IT DOES: Falls under gravity and moves by its speed (Stage 08's overlay).
; HOW: ApplyGravityUnlimited and ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FallUnderGravity0D5E:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0D5E:175 WalkTowardUntilNear0D5E
; @run 71:C975
; WHAT IT DOES: Walks (WalkWhileFrameChangesC) with OBJ_WORK_E the side of the player, goes to state 3 once the
; player is less than $4C away, then checks the ground ahead (TurnAtLedgeOrWall0D5E) (Stage 08's overlay).
; HOW: A call, SideOfPlayer, DistanceXToPlayer and a compare, then the piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkTowardUntilNear0D5E:
    jsr WalkWhileFrameChangesC
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$4C
    bcs WalkTowardUntilNear0D5E_at0
    lda #$03
    sta a:OBJ_STATE,x
WalkTowardUntilNear0D5E_at0:
    jmp TurnAtLedgeOrWall0D5E

; @routine 0D5E:21F TurnAtLedgeOrWall0D5E
; @run 71:CA1F
; WHAT IT DOES: Probes the floor; unless all three columns have floor, or when a wall probe finds a wall (then also
; flipping OBJ_WORK_E), restarts the animation in state 5, substate 0; otherwise counts OBJ_WORK_D down and at zero
; restarts the animation in state 4 (Stage 08's overlay).
; HOW: ProbeTerrain request 9, an AND of the three TileHit bytes, ProbeTerrain request $22, stores or DEC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnAtLedgeOrWall0D5E:
    lda #$0A
    sta z:SpriteFlipMask
    lda #$09
    jsr VecProbeTerrain
    lda a:TileHit
    and a:TileDepthX-3
    and a:TileDepthX-2
    and #$01
    beq TurnAtLedgeOrWall0D5E_at0
    lda #$0A
    sta z:SpriteFlipMask
    lda #$22
    jsr VecProbeTerrain
    and #$02
    beq TurnAtLedgeOrWall0D5E_at1
    lda a:OBJ_WORK_E,x
    eor #$01
    sta a:OBJ_WORK_E,x
TurnAtLedgeOrWall0D5E_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    lda #$05
    sta a:OBJ_STATE,x
    rts
TurnAtLedgeOrWall0D5E_at1:
    dec a:OBJ_WORK_D,x
    bne TurnAtLedgeOrWall0D5E_at2
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
TurnAtLedgeOrWall0D5E_at2:
    rts

; @routine 0D5E:26A WalkAwayUntilFar0D5E
; @run 71:CA6A
; WHAT IT DOES: Walks (WalkWhileFrameChangesC) with OBJ_WORK_E away from the player, goes to state 2 once the
; player is $6C or more away, then checks the ground ahead (TurnAtLedgeOrWall0D5E) (Stage 08's overlay).
; HOW: A call, SideOfPlayer flipped, DistanceXToPlayer and a compare, then the piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkAwayUntilFar0D5E:
    jsr WalkWhileFrameChangesC
    jsr VecSideOfPlayer
    and #$01
    eor #$01
    sta a:OBJ_WORK_E,x
    jsr VecDistanceXToPlayer
    cmp #$6C
    bcc WalkAwayUntilFar0D5E_at0
    lda #$02
    sta a:OBJ_STATE,x
WalkAwayUntilFar0D5E_at0:
    jmp TurnAtLedgeOrWall0D5E

; @routine 0D5E:2CB ActBySubstate0D5E
; @run 71:CACB
; WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActBySubstate0D5EJumps = $CADD ; the routine for each substate, a word each
ActBySubstate0D5E:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:ActBySubstate0D5EJumps,y
    sta z:zTemp00
    lda a:ActBySubstate0D5EJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5E:45D ActByStateAfterStruckCheck0D5E
; @run 71:CC5D
; WHAT IT DOES: Steps OBJ_WORK_D, runs State5WhenStruck0D5E, then the routine for the object's state (Stage 08's
; overlay).
; HOW: INC, a call, then a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByStateAfterStruckCheck0D5EJumps = $CC75 ; the routine for each state, a word each
ActByStateAfterStruckCheck0D5E:
    inc a:OBJ_WORK_D,x
    jsr State5WhenStruck0D5E
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByStateAfterStruckCheck0D5EJumps,y
    sta z:zTemp00
    lda a:ActByStateAfterStruckCheck0D5EJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5E:53B WalkForwardUntilClose0D5E
; @run 71:CD3B
; WHAT IT DOES: Walks (WalkOnFloorOrFall0D5E) the way it faces; every $40 counts of OBJ_WORK_D turns to face the
; player and goes to state 4 when it is less than 8 away (Stage 08's overlay).
; HOW: A call, a store, a test of OBJ_WORK_D's low six bits, SideOfPlayer, DistanceXToPlayer and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkForwardUntilClose0D5E:
    jsr WalkOnFloorOrFall0D5E
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_D,x
    and #$3F
    bne WalkForwardUntilClose0D5E_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jsr VecDistanceXToPlayer
    cmp #$08
    bcs WalkForwardUntilClose0D5E_at0
    lda #$04
    sta a:OBJ_STATE,x
WalkForwardUntilClose0D5E_at0:
    rts

; @routine 0D5E:5DA WalkBackUntilAway0D5E
; @run 71:CDDA
; WHAT IT DOES: Walks (WalkOnFloorOrFall0D5E) against the way it faces; every $40 counts of OBJ_WORK_D turns to
; face the player and goes to state 3 when it is $30 or more away (Stage 08's overlay).
; HOW: A call, an EOR and a store, a test of OBJ_WORK_D's low six bits, SideOfPlayer, DistanceXToPlayer and a
; compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WalkBackUntilAway0D5E:
    jsr WalkOnFloorOrFall0D5E
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_D,x
    and #$3F
    bne WalkBackUntilAway0D5E_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jsr VecDistanceXToPlayer
    cmp #$30
    bcc WalkBackUntilAway0D5E_at0
    lda #$03
    sta a:OBJ_STATE,x
WalkBackUntilAway0D5E_at0:
    rts

; @routine 0D5E:72E RoamAndKeepSpeedHistory0D5E
; @run 71:CF2E
; WHAT IT DOES: Runs StartOrRoamNearPlayer0D5E, then shifts $3F bytes of four speed histories along by one from the
; place a table gives by OBJ_START_ARG, and puts its X and Y speeds at their start (Stage 08's overlay).
; HOW: A call, a copy loop of four buffers, a table read less $3F and four stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RoamAndKeepSpeedHistory0D5EVxHi = $B2F3 ; the X speed high byte history at $B2F4; read one byte below
RoamAndKeepSpeedHistory0D5EVxLo = $B373 ; the X speed low byte history at $B374; read one byte below
RoamAndKeepSpeedHistory0D5EVyHi = $B3F3 ; the Y speed high byte history at $B3F4; read one byte below
RoamAndKeepSpeedHistory0D5EVyLo = $B473 ; the Y speed low byte history at $B474; read one byte below
RoamAndKeepSpeedHistory0D5EBases = $CF7C ; the buffers' index, plus $3F, for each OBJ_START_ARG
RoamAndKeepSpeedHistory0D5E:
    jsr StartOrRoamNearPlayer0D5E
    lda #$3F
    sta z:zTemp00
    ldy a:OBJ_START_ARG,x
    lda a:RoamAndKeepSpeedHistory0D5EBases,y
    tay
RoamAndKeepSpeedHistory0D5E_at0:
    lda a:RoamAndKeepSpeedHistory0D5EVxHi,y
    sta a:RoamAndKeepSpeedHistory0D5EVxHi+1,y
    lda a:RoamAndKeepSpeedHistory0D5EVxLo,y
    sta a:RoamAndKeepSpeedHistory0D5EVxLo+1,y
    lda a:RoamAndKeepSpeedHistory0D5EVyHi,y
    sta a:RoamAndKeepSpeedHistory0D5EVyHi+1,y
    lda a:RoamAndKeepSpeedHistory0D5EVyLo,y
    sta a:RoamAndKeepSpeedHistory0D5EVyLo+1,y
    dey
    dec z:zTemp00
    bne RoamAndKeepSpeedHistory0D5E_at0
    ldy a:OBJ_START_ARG,x
    lda a:RoamAndKeepSpeedHistory0D5EBases,y
    sec
    sbc #$3F
    tay
    lda a:OBJ_VX_HI,x
    sta a:RoamAndKeepSpeedHistory0D5EVxHi+1,y
    lda a:OBJ_VX_LO,x
    sta a:RoamAndKeepSpeedHistory0D5EVxLo+1,y
    lda a:OBJ_VY_HI,x
    sta a:RoamAndKeepSpeedHistory0D5EVyHi+1,y
    lda a:OBJ_VY_LO,x
    sta a:RoamAndKeepSpeedHistory0D5EVyLo+1,y
    rts

; @routine 0D5F:083 ActByState0D5F_083
; @run 71:D083
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5F_083Jumps = $D095 ; the routine for each state, a word each
ActByState0D5F_083:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5F_083Jumps,y
    sta z:zTemp00
    lda a:ActByState0D5F_083Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5F:17A BlockActUnlessSlowed0D5F
; @run 71:D17A
; WHAT IT DOES: Bars or carries the player as a block $14 wide and $20 high, goes to state 4 once dead, and unless
; the frame is skipped by the time-slowing, runs FallWithLowerProbe0D5F, HitBoxXByFrame0D5F and the routine for its
; state (Stage 08's overlay).
; HOW: Three pieces, two calls, then a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BlockActUnlessSlowed0D5FJumps = $D19E ; the routine for each state, a word each
BlockActUnlessSlowed0D5F:
    jsr BlockContact14By20_0D5F
    jsr State4WhenDead0D5F
    jsr SkipFrameWhenSlowed0D5F
    bcc BlockActUnlessSlowed0D5F_at0
    rts
BlockActUnlessSlowed0D5F_at0:
    jsr FallWithLowerProbe0D5F
    jsr HitBoxXByFrame0D5F
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:BlockActUnlessSlowed0D5FJumps,y
    sta z:zTemp00
    lda a:BlockActUnlessSlowed0D5FJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5F:1A8 BlockContact14By20_0D5F
; @run 71:D1A8
; WHAT IT DOES: Tests the object as a block $14 wide and $20 high against the player (ContactWithPlayer) (Stage
; 08's overlay).
; HOW: Two stores, then a jump.
; WAITS: no.
; Entry: X the object. Exit: as ContactWithPlayer: C set when the player stands on it.
BlockContact14By20_0D5F:
    lda #$14
    sta z:zTemp08
    lda #$20
    sta z:zTemp09
    jmp ContactWithPlayer

; @routine 0D5F:1B3 State4WhenDead0D5F
; @run 71:D1B3
; WHAT IT DOES: Once the object is dead (OBJ_HIT_STATUS bit 7) outside state 4, restarts its animation in state 4
; (Stage 08's overlay).
; HOW: A compare, a test, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
State4WhenDead0D5F:
    lda a:OBJ_STATE,x
    cmp #$04
    beq State4WhenDead0D5F_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State4WhenDead0D5F_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
State4WhenDead0D5F_at0:
    rts

; @routine 0D5F:1CD SkipFrameWhenSlowed0D5F
; @run 71:D1CD
; WHAT IT DOES: In states other than 0 and 4 with TimeSlowMode on, decides whether this frame is skipped from the
; pattern byte for zWaveCounter's low three bits, row $18 of one table in mode 2 and of the other otherwise (Stage
; 08's overlay).
; HOW: Compares of the state and the mode; a table read by $18 plus the count.
; WAITS: no.
; Entry: X the object. Exit: C set when the frame is skipped; else C clear.
SkipFrameWhenSlowed0D5FMode1 = $A146 ; the time-slowing pattern bytes read in the other modes; nonzero skips
SkipFrameWhenSlowed0D5FMode2 = $A166 ; the time-slowing pattern bytes read in mode 2; nonzero skips
SkipFrameWhenSlowed0D5F:
    lda a:OBJ_STATE,x
    beq SkipFrameWhenSlowed0D5F_at0
    cmp #$04
    beq SkipFrameWhenSlowed0D5F_at0
    lda a:TimeSlowMode
    beq SkipFrameWhenSlowed0D5F_at0
    cmp #$02
    bne SkipFrameWhenSlowed0D5F_at1
    lda z:zWaveCounter
    and #$07
    clc
    adc #$18
    tay
    lda a:SkipFrameWhenSlowed0D5FMode2,y
    beq SkipFrameWhenSlowed0D5F_at0
SkipFrameWhenSlowed0D5F_at2:
    sec
    rts
SkipFrameWhenSlowed0D5F_at1:
    lda z:zWaveCounter
    and #$07
    clc
    adc #$18
    tay
    lda a:SkipFrameWhenSlowed0D5FMode1,y
    bne SkipFrameWhenSlowed0D5F_at2
SkipFrameWhenSlowed0D5F_at0:
    clc
    rts

; @routine 0D5F:279 StartWithKind52Followers0D5F
; @run 71:D279
; WHAT IT DOES: Starts the object from its record; with OBJ_START_ARG not negative, starts that many objects of
; kind $52 at its place in the slots after it, with OBJ_START_ARG $81 up, the object's slot in OBJ_WORK_M and its
; slot plus the count in OBJ_WORK_N; then faces it by OBJ_START_ARG bit 0 and runs ShiftOnceByArg0D5F,
; RandomWaitDE0D5F and RandomWordFG0D5F (Stage 08's overlay).
; HOW: LoadObjectRecord, a loop of ClearObjectInY and StartObjectAtPlace counting OBJ_START_ARG down, an AND and
; EOR, three calls.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWithKind52Followers0D5F:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    bmi StartWithKind52Followers0D5F_at0
    txa
    sta a:OBJ_WORK_M,x
    clc
    adc a:OBJ_START_ARG,x
    sta a:OBJ_WORK_N,x
    txa
    tay
    iny
    lda #$81
    sta z:zTemp00
StartWithKind52Followers0D5F_at1:
    jsr VecClearObjectInY
    lda #$52
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    txa
    sta a:OBJ_WORK_M,y
    lda a:OBJ_WORK_N,x
    sta a:OBJ_WORK_N,y
    inc z:zTemp00
    iny
    dec a:OBJ_START_ARG,x
    bne StartWithKind52Followers0D5F_at1
StartWithKind52Followers0D5F_at0:
    lda a:OBJ_START_ARG,x
    and #$7F
    sta a:OBJ_START_ARG,x
    and #$01
    eor #$01
    sta a:OBJ_FACING_LEFT,x
    jsr ShiftOnceByArg0D5F
    jsr RandomWaitDE0D5F
    jsr RandomWordFG0D5F
    rts

; @routine 0D5F:328 CountBothWaits0D5F
; @run 71:D328
; WHAT IT DOES: Runs CountWaitThenState2_0D5F and CountWaitThenState3_0D5F (Stage 08's overlay).
; HOW: Two calls.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountBothWaits0D5F:
    jsr CountWaitThenState2_0D5F
    jsr CountWaitThenState3_0D5F
    rts

; @routine 0D5F:32F CountWaitThenState2_0D5F
; @run 71:D32F
; WHAT IT DOES: Counts the 16-bit OBJ_WORK_D-E down; at zero takes a new random wait (RandomWaitDE0D5F) and
; restarts the animation in state 2 (Stage 08's overlay).
; HOW: An ORA test, a call and stores, or a 16-bit subtract.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountWaitThenState2_0D5F:
    lda a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    bne CountWaitThenState2_0D5F_at0
    jsr RandomWaitDE0D5F
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$02
    sta a:OBJ_STATE,x
    rts
CountWaitThenState2_0D5F_at0:
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    rts

; @routine 0D5F:358 CountWaitThenState3_0D5F
; @run 71:D358
; WHAT IT DOES: While the object's X high byte is 0, counts the 16-bit OBJ_WORK_F-G down; at zero takes a new
; random word (RandomWordFG0D5F) and restarts the animation in state 3, substate 0 (Stage 08's overlay).
; HOW: A test, an ORA test, a call and stores, or a 16-bit subtract.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountWaitThenState3_0D5F:
    lda a:OBJ_X_HI,x
    bne CountWaitThenState3_0D5F_at0
    lda a:OBJ_WORK_F,x
    ora a:OBJ_WORK_G,x
    bne CountWaitThenState3_0D5F_at1
    jsr RandomWordFG0D5F
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    lda #$03
    sta a:OBJ_STATE,x
    rts
CountWaitThenState3_0D5F_at1:
    lda a:OBJ_WORK_G,x
    sec
    sbc #$01
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    sbc #$00
    sta a:OBJ_WORK_F,x
CountWaitThenState3_0D5F_at0:
    rts

; @routine 0D5F:3BE ActBySubstate0D5F
; @run 71:D3BE
; WHAT IT DOES: Runs the routine for the object's substate (Stage 08's overlay).
; HOW: A jump table by the substate.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActBySubstate0D5FJumps = $D3D0 ; the routine for each substate, a word each
ActBySubstate0D5F:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:ActBySubstate0D5FJumps,y
    sta z:zTemp00
    lda a:ActBySubstate0D5FJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5F:54A ActByState0D5F_54A
; @run 71:D54A
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5F_54AJumps = $D55C ; the routine for each state, a word each
ActByState0D5F_54A:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5F_54AJumps,y
    sta z:zTemp00
    lda a:ActByState0D5F_54AJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5F:62A ActThenKeepYOfSlot0D5F
; @run 71:D62A
; WHAT IT DOES: Runs ActByState0D5F_630, then KeepYOfWorkFSlot0D5F (Stage 08's overlay).
; HOW: A call and a jump.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActThenKeepYOfSlot0D5F:
    jsr ActByState0D5F_630
    jmp KeepYOfWorkFSlot0D5F

; @routine 0D5F:630 ActByState0D5F_630
; @run 71:D630
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D5F_630Jumps = $D642 ; the routine for each state, a word each
ActByState0D5F_630:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D5F_630Jumps,y
    sta z:zTemp00
    lda a:ActByState0D5F_630Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D5F:67F WaitThenSoundAndHitBox0D5F
; @run 71:D67F
; WHAT IT DOES: Counts OBJ_WORK_E down; at zero plays sound $26, sets OBJ_WORK_E $40 and OBJ_HITBOX_ON 2 and steps
; the state on (Stage 08's overlay).
; HOW: DEC, SoundCommand, stores, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WaitThenSoundAndHitBox0D5F:
    dec a:OBJ_WORK_E,x
    bne WaitThenSoundAndHitBox0D5F_at0
    lda #$26
    jsr VecSoundCommand
    lda #$40
    sta a:OBJ_WORK_E,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
WaitThenSoundAndHitBox0D5F_at0:
    rts

; @routine 0D5F:75D StartWithFrameFromArg0D5F
; @run 71:D75D
; WHAT IT DOES: In state 0, starts the object from its record with its frame from OBJ_START_ARG and sprite set 6
; (Stage 08's overlay).
; HOW: A test, LoadObjectRecord, two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWithFrameFromArg0D5F:
    lda a:OBJ_STATE,x
    bne StartWithFrameFromArg0D5F_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
StartWithFrameFromArg0D5F_at0:
    rts

; @routine 0D60:1F5 ActByState0D60
; @run 71:D9F5
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByState0D60Jumps = $DA07 ; the routine for each state, a word each
ActByState0D60:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByState0D60Jumps,y
    sta z:zTemp00
    lda a:ActByState0D60Jumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D60:213 ProbeFloorThreePoints0D60
; @run 71:DA13
; WHAT IT DOES: Reads the map 12 pixels left and 12 right of the object, 9 below it, then (FloorUnderCentre0D60) 16
; below its middle, stopping at the first that has tile bits $21 (Stage 08's overlay).
; HOW: 16-bit adds into zTemp00-03 and TerrainTileRead twice, an AND #$21, then the next routine.
; WAITS: no.
; Entry: X the object. Exit: A the tile's bits $21, Z clear when a point has them; zTemp00 the column and zTemp02
; the row in the tile.
ProbeFloorThreePoints0D60:
    lda a:OBJ_X_LO,x
    clc
    adc #$F4
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc #$FF
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    clc
    adc #$09
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    adc #$00
    sta z:zTemp03
    jsr StageVec8006_050D
    and #$21
    beq ProbeFloorThreePoints0D60_at0
    rts
ProbeFloorThreePoints0D60_at0:
    lda a:OBJ_X_LO,x
    clc
    adc #$0C
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc #$00
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    clc
    adc #$09
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    adc #$00
    sta z:zTemp03
    jsr StageVec8006_050D
    and #$21
    beq FloorUnderCentre0D60
    rts

; @routine 0D60:25F FloorUnderCentre0D60
; @run 71:DA5F
; WHAT IT DOES: Reads the map 16 pixels below the object's place (Stage 08's overlay).
; HOW: A 16-bit add into zTemp00-03, TerrainTileRead, AND #$21.
; WAITS: no.
; Entry: X the object. Exit: A the tile's bits $21, Z clear when it has them; zTemp00 the column and zTemp02 the
; row in the tile.
FloorUnderCentre0D60:
    lda a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    clc
    adc #$10
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    adc #$00
    sta z:zTemp03
    jsr StageVec8006_050D
    and #$21
    rts

; @routine 0D60:29E MoveXAndBounceOffWalls0D60
; @run 71:DA9E
; WHAT IT DOES: Moves the object along X; with OBJ_START_ARG not negative, reads the map ahead of it on the side it
; moves to, $0E below, and at a wall (bit 1) moves it back out of the tile and turns its X speed round (Stage 08's
; overlay).
; HOW: ApplyVelocityX, a word table read by the speed's sign, TerrainTileRead, a subtract of the column or an add
; from a table by it, NegateVelocityX.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveXAndBounceOffWalls0D60Pushes = $DB02 ; the push back right for each column in the tile
MoveXAndBounceOffWalls0D60Offsets = $DB0A ; the probe's X offset, plus 1, moving right then left, a word each
MoveXAndBounceOffWalls0D60:
    jsr VecApplyVelocityX
    lda a:OBJ_START_ARG,x
    bmi MoveXAndBounceOffWalls0D60_at0
    lda a:OBJ_VX_HI,x
    asl a
    rol a
    and #$01
    asl a
    tay
    lda a:MoveXAndBounceOffWalls0D60Offsets,y
    sta z:zTemp08
    lda a:MoveXAndBounceOffWalls0D60Offsets+1,y
    sta z:zTemp09
    dec z:zTemp08
    lda a:OBJ_X_LO,x
    clc
    adc z:zTemp08
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc z:zTemp09
    sta z:zTemp01
    lda a:OBJ_Y_LO,x
    clc
    adc #$0E
    sta z:zTemp02
    lda a:OBJ_Y_HI,x
    adc #$00
    sta z:zTemp03
    jsr StageVec8006_050D
    and #$02
    beq MoveXAndBounceOffWalls0D60_at0
    lda a:OBJ_VX_HI,x
    bmi MoveXAndBounceOffWalls0D60_at1
    lda a:OBJ_X_LO,x
    sec
    sbc z:zTemp00
    sta a:OBJ_X_LO,x
    jsr VecNegateVelocityX
MoveXAndBounceOffWalls0D60_at0:
    rts
MoveXAndBounceOffWalls0D60_at1:
    ldy z:zTemp00
    lda a:OBJ_X_LO,x
    clc
    adc a:MoveXAndBounceOffWalls0D60Pushes,y
    sta a:OBJ_X_LO,x
    jsr VecNegateVelocityX
    rts

; @routine 0D60:3B3 RideSlotAndCarryPlayer0D60
; @run 71:DBB3
; WHAT IT DOES: Puts the object 8 pixels below the slot in OBJ_WORK_H, sets its speed to the move that made
; (keeping the old place in OBJ_WORK_I to L), and carries the player by it (CarryPlayerByHeightTable0D60) (Stage
; 08's overlay).
; HOW: Copies, 16-bit subtracts into the speeds, then a call.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RideSlotAndCarryPlayer0D60:
    lda a:OBJ_X_FRAC,x
    sta a:OBJ_WORK_I,x
    lda a:OBJ_X_LO,x
    sta a:OBJ_WORK_J,x
    lda a:OBJ_Y_FRAC,x
    sta a:OBJ_WORK_K,x
    lda a:OBJ_Y_LO,x
    sta a:OBJ_WORK_L,x
    ldy a:OBJ_WORK_H,x
    lda a:OBJ_X_LO,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,y
    sta a:OBJ_X_HI,x
    lda a:OBJ_Y_FRAC,y
    sta a:OBJ_Y_FRAC,x
    lda a:OBJ_Y_LO,y
    clc
    adc #$08
    sta a:OBJ_Y_LO,x
    cla
    adc a:OBJ_Y_HI,y
    sta a:OBJ_Y_HI,x
    lda a:OBJ_X_FRAC,x
    sec
    sbc a:OBJ_WORK_I,x
    sta a:OBJ_VX_LO,x
    lda a:OBJ_X_LO,x
    sbc a:OBJ_WORK_J,x
    sta a:OBJ_VX_HI,x
    lda a:OBJ_Y_FRAC,x
    sec
    sbc a:OBJ_WORK_K,x
    sta a:OBJ_VY_LO,x
    lda a:OBJ_Y_LO,x
    sbc a:OBJ_WORK_L,x
    sta a:OBJ_VY_HI,x
    jsr CarryPlayerByHeightTable0D60
    rts

; @routine 0D60:41A RollAndLandOnFloor0D60
; @run 71:DC1A
; WHAT IT DOES: Carries the player, moves along X bouncing off walls and moves along Y; finding no floor (or with
; OBJ_START_ARG negative) falls faster; on floor, when falling at 2 or more and on the first screen goes to state 4
; with OBJ_WORK_G $10 and sound $2C, turns its X speed round on tile bit 5, sits on the floor with no Y speed and
; steps the state on; then rolls its frames (Stage 08's overlay).
; HOW: Calls, ApplyVelocityY, ProbeFloorThreePoints0D60, FallFasterTo4_0D60, compares, SoundCommand,
; NegateVelocityX, RiseByTemp02_0D60, stores, then the frame piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RollAndLandOnFloor0D60:
    jsr CarryPlayerByHeightTable0D60
    jsr MoveXAndBounceOffWalls0D60
    jsr VecApplyVelocityY
    lda a:OBJ_START_ARG,x
    bmi RollAndLandOnFloor0D60_at0
    jsr ProbeFloorThreePoints0D60
    bne RollAndLandOnFloor0D60_at1
RollAndLandOnFloor0D60_at0:
    jsr FallFasterTo4_0D60
    jmp RollFrames0D60
RollAndLandOnFloor0D60_at1:
    pha
    lda a:OBJ_VY_HI,x
    cmp #$02
    bcc RollAndLandOnFloor0D60_at2
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne RollAndLandOnFloor0D60_at2
    lda #$04
    sta a:OBJ_STATE,x
    lda #$10
    sta a:OBJ_WORK_G,x
    lda #$2C
    jsr VecSoundCommand
RollAndLandOnFloor0D60_at2:
    pla
    and #$20
    beq RollAndLandOnFloor0D60_at3
    lda a:OBJ_VY_HI,x
    cmp #$02
    bcc RollAndLandOnFloor0D60_at3
    jsr VecNegateVelocityX
RollAndLandOnFloor0D60_at3:
    jsr RiseByTemp02_0D60
    stz a:OBJ_VY_LO,x
    stz a:OBJ_VY_HI,x
    inc a:OBJ_STATE,x
    jmp RollFrames0D60

; @routine 0D60:470 RollUntilFloorEnds0D60
; @run 71:DC70
; WHAT IT DOES: Carries the player and moves along X bouncing off walls; with OBJ_START_ARG not negative, sits on
; the floor under its middle until there is none, and with it negative goes on until its X is $A2 (clearing it at X
; low byte $F0 with the high byte not 0); then clears OBJ_WORK_D and E, keeps its X speed in OBJ_WORK_F and steps
; the state on; it rolls its frames throughout (Stage 08's overlay).
; HOW: Calls, FloorUnderCentre0D60, RiseByTemp02_0D60, compares of the X bytes, ClearObject, stores, the frame
; piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RollUntilFloorEnds0D60:
    jsr CarryPlayerByHeightTable0D60
    jsr MoveXAndBounceOffWalls0D60
    lda a:OBJ_START_ARG,x
    bmi RollUntilFloorEnds0D60_at0
    jsr FloorUnderCentre0D60
    beq RollUntilFloorEnds0D60_at1
    jsr RiseByTemp02_0D60
RollUntilFloorEnds0D60_at2:
    jmp RollFrames0D60
RollUntilFloorEnds0D60_at3:
    lda a:OBJ_X_LO,x
    cmp #$F0
    bne RollUntilFloorEnds0D60_at2
    jmp VecClearObject
RollUntilFloorEnds0D60_at0:
    lda a:OBJ_X_HI,x
RollUntilFloorEnds0D60_at4:
    bne RollUntilFloorEnds0D60_at3
    lda a:OBJ_X_LO,x
    cmp #$A2
    bne RollUntilFloorEnds0D60_at2
RollUntilFloorEnds0D60_at1:
    lda #$00
    sta a:OBJ_WORK_D,x
    lda #$00
    sta a:OBJ_WORK_E,x
    lda a:OBJ_VX_HI,x
    sta a:OBJ_WORK_F,x
    inc a:OBJ_STATE,x
    jmp RollFrames0D60

; @routine 0D60:4B2 CarryThenState2_0D60
; @run 71:DCB2
; WHAT IT DOES: Carries the player, puts the object in state 2 and rolls its frames (Stage 08's overlay).
; HOW: A call, a store, the frame piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CarryThenState2_0D60:
    jsr CarryPlayerByHeightTable0D60
    lda #$02
    sta a:OBJ_STATE,x
    jmp RollFrames0D60

; @routine 0D60:4BD CarryAndCountToState3_0D60
; @run 71:DCBD
; WHAT IT DOES: Carries the player and counts OBJ_WORK_G down; at zero puts the object in state 3 (Stage 08's
; overlay).
; HOW: A call, DEC, a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CarryAndCountToState3_0D60:
    jsr CarryPlayerByHeightTable0D60
    dec a:OBJ_WORK_G,x
    bne CarryAndCountToState3_0D60_at0
    lda #$03
    sta a:OBJ_STATE,x
CarryAndCountToState3_0D60_at0:
    rts

; @routine 0D60:4CB RollFrames0D60
; @run 71:DCCB
; WHAT IT DOES: Every $10 frames steps the object's frame through $3B to $42, up while it moves left and down
; otherwise, wrapping round (Stage 08's overlay).
; HOW: INC and a compare of OBJ_ANIM_TIMER, INC or DEC of the frame, compares, then ReturnOnly0D60.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RollFrames0D60:
    lda #$10
    inc a:OBJ_ANIM_TIMER,x
    cmp a:OBJ_ANIM_TIMER,x
    bne ReturnOnly0D60
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_VX_HI,x
    bpl RollFrames0D60_at0
    inc a:OBJ_FRAME,x
    lda a:OBJ_FRAME,x
    cmp #$43
    bne ReturnOnly0D60
    lda #$3B
    bra RollFrames0D60_at1
RollFrames0D60_at0:
    dec a:OBJ_FRAME,x
    lda a:OBJ_FRAME,x
    cmp #$3A
    bne ReturnOnly0D60
    lda #$42
RollFrames0D60_at1:
    sta a:OBJ_FRAME,x

; @routine 0D6C:200 ActByKind59To5C0D6C
; @run 74:DA00
; WHAT IT DOES: Runs the object by its kind: $59 by the routine for its state, $5A nothing, $5B by
; ActByStateKind5B0D6C, $5C by FlyAndFallKind5C0D6C; other kinds nothing (Stage 08's overlay).
; HOW: Kind compares, then a jump table by the state or a piece.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByKind59To5C0D6CJumps = $DA2F ; the routine for each state of kind $59, a word each
ActByKind59To5C0D6C:
    lda a:OBJ_KIND,x
    cmp #$59
    beq ActByKind59To5C0D6C_at0
    cmp #$5A
    bne ActByKind59To5C0D6C_at1
    jmp ReturnForKind5A0D6C
ActByKind59To5C0D6C_at1:
    cmp #$5B
    bne ActByKind59To5C0D6C_at2
    jmp ActByStateKind5B0D6C
ActByKind59To5C0D6C_at2:
    cmp #$5C
    bne ActByKind59To5C0D6C_at3
    jmp FlyAndFallKind5C0D6C
ActByKind59To5C0D6C_at3:
    rts
ActByKind59To5C0D6C_at0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByKind59To5C0D6CJumps,y
    sta z:zTemp00
    lda a:ActByKind59To5C0D6CJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D6C:237 DrawList6AndSetScroll0D6C
; @run 74:DA37
; WHAT IT DOES: Draws block list 6 (DrawBlockListToVram0D6C), puts the object at Y $C0, sets StageVramBuffer+$13 to
; $31 and the scroll's X limit to $0280, and steps the state on (Stage 08's overlay).
; HOW: A call with X kept on the stack, stores, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
DrawList6AndSetScroll0D6C:
    phx
    lda #$06
    jsr DrawBlockListToVram0D6C
    plx
    lda #$C0
    sta a:OBJ_Y_LO,x
    lda #$31
    sta a:StageVramBuffer+19
    lda #$02
    sta a:ScrollXMaxHi
    lda #$80
    sta a:ScrollXMaxLo
    inc a:OBJ_STATE,x
    rts

; @routine 0D6C:269 CountDownAndDrawLists0D6C
; @run 74:DA69
; WHAT IT DOES: Sets StageVramBuffer+$13 to $23, the player's OBJ_WORK_E $80 and OBJ_WORK_F $10 and turns the
; player's hit box off; runs LevelWordByStep0D6C, spawns kind $9E (SpawnKind9EScattered0D6C) once OBJ_WORK_F is
; below $A0, and counts OBJ_WORK_F down; below $60 also counts OBJ_WORK_E down and each time it ends draws block
; list OBJ_WORK_D, steps that on, and after list 5 sets OBJ_WORK_F $80 and steps the state on, else OBJ_WORK_E $0C
; (Stage 08's overlay).
; HOW: Stores, two calls, compares, DECs, DrawBlockListToVram0D6C, INC and compares.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountDownAndDrawLists0D6C:
    lda #$23
    sta a:StageVramBuffer+19
    lda #$80
    sta a:OBJ_WORK_E
    lda #$10
    sta a:OBJ_WORK_F
    stz a:OBJ_HITBOX_ON
    jsr LevelWordByStep0D6C
    lda a:OBJ_WORK_F,x
    cmp #$A0
    bcs CountDownAndDrawLists0D6C_at0
    jsr SpawnKind9EScattered0D6C
    lda a:OBJ_WORK_F,x
    cmp #$60
    bcc CountDownAndDrawLists0D6C_at1
CountDownAndDrawLists0D6C_at0:
    dec a:OBJ_WORK_F,x
    rts
CountDownAndDrawLists0D6C_at1:
    dec a:OBJ_WORK_F,x
    dec a:OBJ_WORK_E,x
    bne CountDownAndDrawLists0D6C_at2
    lda a:OBJ_WORK_D,x
    phx
    jsr DrawBlockListToVram0D6C
    plx
    lda a:OBJ_WORK_D,x
    inc a
    sta a:OBJ_WORK_D,x
    cmp #$06
    beq CountDownAndDrawLists0D6C_at3
    lda #$0C
    sta a:OBJ_WORK_E,x
CountDownAndDrawLists0D6C_at2:
    rts
CountDownAndDrawLists0D6C_at3:
    lda #$80
    sta a:OBJ_WORK_F,x
    inc a:OBJ_STATE,x
    rts

; @routine 0D6C:2BD CountUpThenRestoreScroll0D6C
; @run 74:DABD
; WHAT IT DOES: Runs SpawnKind9EEvery8_0D6C and LevelWordByStep0D6C and steps OBJ_WORK_F on; when it wraps to 0,
; clears the level word at $3EB8, records the object's event, sets StageVramBuffer+$13 to $23, the scroll's X limit
; to $0700 and the player's OBJ_HITBOX_ON 1, and clears the object (Stage 08's overlay).
; HOW: Two calls, INC, stores, RecordObjectEvent, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountUpThenRestoreScroll0D6C:
    jsr SpawnKind9EEvery8_0D6C
    jsr LevelWordByStep0D6C
    inc a:OBJ_WORK_F,x
    beq CountUpThenRestoreScroll0D6C_at0
    rts
CountUpThenRestoreScroll0D6C_at0:
    stz a:RasterWaterWanted
    stz a:LevelYHi
    jsr VecRecordObjectEvent
    lda #$23
    sta a:StageVramBuffer+19
    lda #$07
    sta a:ScrollXMaxHi
    lda #$00
    sta a:ScrollXMaxLo
    lda #$01
    sta a:OBJ_HITBOX_ON
    jmp VecClearObject

; @routine 0D6C:3C6 DrawBlockListToVram0D6C
; @run 74:DBC6
; WHAT IT DOES: Draws the block list whose index is in A: from each VRAM address in it, copies each block it names
; to VRAM (CopyBlockToVram0D6C), 4 on per block, until $FE gives a new address or $FF ends it (Stage 08's overlay).
; HOW: A word table read by twice A, a loop of reads through zTemp00 with calls, and adds of 4.
; WAITS: no.
; Entry: A the list's index; X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
DrawBlockListToVram0D6CLists = $DC5D ; the address of each block list, a word each
DrawBlockListToVram0D6C:
    asl a
    tay
    lda a:DrawBlockListToVram0D6CLists,y
    sta z:zTemp00
    lda a:DrawBlockListToVram0D6CLists+1,y
    sta z:zTemp01
    cly
DrawBlockListToVram0D6C_at1:
    lda (zTemp00),y
    sta z:zTemp10
    sta z:zTemp08
    iny
    lda (zTemp00),y
    sta z:zTemp11
    sta z:zTemp09
    iny
DrawBlockListToVram0D6C_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    beq DrawBlockListToVram0D6C_at0
    cmp #$FE
    beq DrawBlockListToVram0D6C_at1
    phy
    jsr CopyBlockToVram0D6C
    ply
    lda z:zTemp08
    clc
    adc #$04
    sta z:zTemp08
    sta z:zTemp10
    lda z:zTemp09
    sta z:zTemp11
    bra DrawBlockListToVram0D6C_at2
DrawBlockListToVram0D6C_at0:
    rts

; @routine 0D6C:519 ReturnForKind5A0D6C
; @run 74:DD19
; WHAT IT DOES: Returns; what ActByKind59To5C0D6C runs for kind $5A (Stage 08's overlay).
; HOW: RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
ReturnForKind5A0D6C:
    rts

; @routine 0D6C:52A ActByStateKind5B0D6C
; @run 74:DD2A
; WHAT IT DOES: Runs the routine for the object's state (Stage 08's overlay).
; HOW: A jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ActByStateKind5B0D6CJumps = $DD3C ; the routine for each state, a word each
ActByStateKind5B0D6C:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ActByStateKind5B0D6CJumps,y
    sta z:zTemp00
    lda a:ActByStateKind5B0D6CJumps+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0D6C:5B2 StartKind5CRowsThenMask0D6C
; @run 74:DDB2
; WHAT IT DOES: While slot $1E does not hold kind $9B, counts OBJ_WORK_M down; at zero starts three objects of kind
; $5C (StartKind5CByWorkL0D6C), and with all three started masks tiles by the record (MaskTilesByRecord0D6C), sets
; OBJ_WORK_M 4 and steps OBJ_WORK_L on, and at $12 sets OBJ_WORK_M $81 and steps the state on (Stage 08's overlay).
; HOW: A kind compare, DEC, three calls each needing C set, a call with X kept on the stack, stores, INC and a
; compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartKind5CRowsThenMask0D6C:
    lda a:OBJ_KIND+30
    cmp #$9B
    beq StartKind5CRowsThenMask0D6C_at0
    lda a:OBJ_WORK_M,x
    beq StartKind5CRowsThenMask0D6C_at1
    dec a:OBJ_WORK_M,x
    rts
StartKind5CRowsThenMask0D6C_at1:
    jsr StartKind5CByWorkL0D6C
    bcc StartKind5CRowsThenMask0D6C_at0
    jsr StartKind5CByWorkL0D6C
    bcc StartKind5CRowsThenMask0D6C_at0
    jsr StartKind5CByWorkL0D6C
    bcc StartKind5CRowsThenMask0D6C_at0
    phx
    jsr MaskTilesByRecord0D6C
    plx
    lda #$04
    sta a:OBJ_WORK_M,x
    inc a:OBJ_WORK_L,x
    lda a:OBJ_WORK_L,x
    cmp #$12
    bcc StartKind5CRowsThenMask0D6C_at0
    lda #$81
    sta a:OBJ_WORK_M,x
    inc a:OBJ_STATE,x
StartKind5CRowsThenMask0D6C_at0:
    rts

; @routine 0D6C:5EE SpawnKind9EEvery8Counts0D6C
; @run 74:DDEE
; WHAT IT DOES: Counts OBJ_WORK_M down, clearing the object at zero; every 8 counts starts an object of kind $9E in
; a free slot from $20 (StartKind9EInY0D6C), plays sound $C1 and sets OBJ_WORK_H of slot $19 to 1 (Stage 08's
; overlay).
; HOW: DEC, ClearObject, TST #$07, FindFreeSlotFrom20, a call, SoundCommand, a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SpawnKind9EEvery8Counts0D6C:
    dec a:OBJ_WORK_M,x
    bne SpawnKind9EEvery8Counts0D6C_at0
    jmp VecClearObject
SpawnKind9EEvery8Counts0D6C_at0:
    tst #$07,a:OBJ_WORK_M,x
    bne SpawnKind9EEvery8Counts0D6C_at1
    jsr VecFindFreeSlotFrom20
    bcc SpawnKind9EEvery8Counts0D6C_at1
    jsr StartKind9EInY0D6C
    lda #$C1
    jsr VecSoundCommand
    lda #$01
    sta a:OBJ_WORK_H+25
SpawnKind9EEvery8Counts0D6C_at1:
    rts

; @routine 0D6C:67D FlyAndFallKind5C0D6C
; @run 74:DE7D
; WHAT IT DOES: In state 0 gives the object sprite set 3, frame $6A, attribute $10, OBJ_WORK_D 6 and the X and Y
; speeds two tables give by its slot's low two bits, and steps the state on; later counts OBJ_WORK_D down, clearing
; it at zero, moves by its speed and adds $10 to its Y speed (Stage 08's overlay).
; HOW: Stores and word table reads by twice the slot's low bits; or DEC, ClearObject, ApplyVelocityX and
; ApplyVelocityY, a 16-bit add.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlyAndFallKind5C0D6CSpeedsX = $DED6 ; the X speed for each of the slot's low two bits, a word each
FlyAndFallKind5C0D6CSpeedsY = $DEDE ; the Y speed for each of the slot's low two bits, a word each
FlyAndFallKind5C0D6C:
    lda a:OBJ_STATE,x
    bne FlyAndFallKind5C0D6C_at0
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$6A
    sta a:OBJ_FRAME,x
    txa
    and #$03
    asl a
    tay
    lda a:FlyAndFallKind5C0D6CSpeedsX,y
    sta a:OBJ_VX_LO,x
    lda a:FlyAndFallKind5C0D6CSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:FlyAndFallKind5C0D6CSpeedsY,y
    sta a:OBJ_VY_LO,x
    lda a:FlyAndFallKind5C0D6CSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda #$06
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
    rts
FlyAndFallKind5C0D6C_at0:
    dec a:OBJ_WORK_D,x
    bne FlyAndFallKind5C0D6C_at1
    jmp VecClearObject
FlyAndFallKind5C0D6C_at1:
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    lda a:OBJ_VY_LO,x
    clc
    adc #$10
    sta a:OBJ_VY_LO,x
    cla
    adc a:OBJ_VY_HI,x
    sta a:OBJ_VY_HI,x
    rts
