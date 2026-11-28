; @routine 051C:5E5 BurstIntoTwoPieces051B
; @run 70:BDE5
; WHAT IT DOES: Starts two kind $4F objects in free slots from $30 with this one's facing and each piece's argument, count and
; speeds from the tables at $BE2A, then clears this one.
; HOW: A loop of two; the piece number in zBurstCount.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BurstIntoTwoPieces051BArgs = $BE2A ; two pieces: start argument
BurstIntoTwoPieces051BCounts = $BE2C ; two pieces: OBJ_PIECE_NEXT
BurstIntoTwoPieces051BXHigh = $BE2E ; two pieces: X speed high
BurstIntoTwoPieces051BXLow = $BE30 ; two pieces: X speed low
BurstIntoTwoPieces051BYHigh = $BE32 ; two pieces: Y speed high
BurstIntoTwoPieces051BYLow = $BE34 ; two pieces: Y speed low
BurstIntoTwoPieces051B:
    cly
BurstIntoTwoPieces051B_at0:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc BurstIntoTwoPieces051B_at1
    lda #$4F
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    phx
    ldx z:zBurstCount
    lda a:BurstIntoTwoPieces051BArgs,x
    sta a:OBJ_START_ARG,y
    lda a:BurstIntoTwoPieces051BCounts,x
    sta a:OBJ_PIECE_NEXT,y
    lda a:BurstIntoTwoPieces051BXHigh,x
    sta a:OBJ_VX_HI,y
    lda a:BurstIntoTwoPieces051BXLow,x
    sta a:OBJ_VX_LO,y
    lda a:BurstIntoTwoPieces051BYHigh,x
    sta a:OBJ_VY_HI,y
    lda a:BurstIntoTwoPieces051BYLow,x
    sta a:OBJ_VY_LO,y
    plx
    ldy z:zBurstCount
    iny
    cpy #$02
    bcc BurstIntoTwoPieces051B_at0
BurstIntoTwoPieces051B_at1:
    jmp VecClearObject

; @routine 051C:636 LoadThenByStartArgSign051B
; @run 70:BE36
; WHAT IT DOES: Loads the record and goes to substate 1; with bit 7 of OBJ_START_ARG, also drops the hit box and loop flag bit 6
; and goes to state 4.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadThenByStartArgSign051B:
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_SUBSTATE,x
    lda a:OBJ_START_ARG,x
    bmi LoadThenByStartArgSign051B_at0
    rts
LoadThenByStartArgSign051B_at0:
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_LOOP_FLAGS,x
    and #$BF
    sta a:OBJ_LOOP_FLAGS,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 051C:658 SlowAndLandWhileList14_051B
; @run 70:BE58
; WHAT IT DOES: Slows the X speed, steps list $14 ($C200) until it ends, then goes to state 2, else moves and lands.
; HOW: SlowByFacing051B, StepListByNumber051B, MoveAndLand with height 4.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SlowAndLandWhileList14_051B:
    jsr SlowByFacing051B
    ldy #$14
    jsr StepListByNumber051B
    bne SlowAndLandWhileList14_051B_at0
    jmp StartState2_051B
SlowAndLandWhileList14_051B_at0:
    lda #$04
    jsr VecMoveAndLand
    rts

; @routine 051C:66B FaceOffThePlayer051B
; @run 70:BE6B
; WHAT IT DOES: Facing as the player faces, goes to $BF92. Else substate 0 steps list $12 and, within $40, goes to substate 1;
; substate 1 slows, steps list 4 until it ends (state 2) unless the player faces away in state 3, which goes to
; state $0A facing the other way. Moves and lands while the list runs.
; HOW: Calls and compares.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FaceOffThePlayer051B:
    lda a:OBJ_FACING_LEFT
    eor a:OBJ_FACING_LEFT,x
    bne FaceOffThePlayer051B_at0
    jmp StartState6_051B
FaceOffThePlayer051B_at0:
    lda a:OBJ_SUBSTATE,x
    bne FaceOffThePlayer051B_at1
    ldy #$12
    jsr StepListByNumber051B
    jsr VecDistanceXToPlayer
    cmp #$40
    bcs FaceOffThePlayer051B_at3
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    bra FaceOffThePlayer051B_at3
FaceOffThePlayer051B_at1:
    jsr SlowByFacing051B
    jsr VecSideOfPlayer
    and #$01
    cmp a:OBJ_FACING_LEFT,x
    beq FaceOffThePlayer051B_at2
    lda a:OBJ_STATE
    cmp #$03
    beq FaceOffThePlayer051B_at4
FaceOffThePlayer051B_at2:
    ldy #$04
    jsr StepListByNumber051B
    bne FaceOffThePlayer051B_at3
    jmp StartState2_051B
FaceOffThePlayer051B_at3:
    lda #$04
    jsr VecMoveAndLand
    rts
FaceOffThePlayer051B_at4:
    lda #$0B
    sta a:OBJ_PIECE_PLACE,x
    jsr SpeedByPlayerSide051B
    lda #$0A
    sta a:OBJ_STATE,x
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    rts

; @routine 051C:6D5 LandSlowList8_051B
; @run 70:BED5
; WHAT IT DOES: Moves and lands, slows, steps list 8 and goes to state 2 when it ends.
; HOW: MoveAndLand, SlowByFacing051B, StepListByNumber051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LandSlowList8_051B:
    lda #$04
    jsr VecMoveAndLand
    jsr SlowByFacing051B
    ldy #$08
    jsr StepListByNumber051B
    bne LandSlowList8_051B_at0
    jsr StartState2_051B
LandSlowList8_051B_at0:
    rts

; @routine 051C:6E8 SlowByFacing051B
; @run 70:BEE8
; WHAT IT DOES: Takes the value for the facing in the table at $BF03 from the X speed.
; HOW: A 16-bit subtraction.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SlowByFacing051BSteps = $BF03 ; a word per facing
SlowByFacing051B:
    lda a:OBJ_FACING_LEFT,x
    and #$01
    asl a
    tay
    lda a:OBJ_VX_LO,x
    sec
    sbc a:SlowByFacing051BSteps,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    sbc a:SlowByFacing051BSteps+1,y
    sta a:OBJ_VX_HI,x
    rts

; @routine 051C:707 List10ThenLand051B
; @run 70:BF07
; WHAT IT DOES: Substate 0 steps list $10 and lands ($BF1D); substate 1 is $BF2E.
; HOW: StepListByNumber051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
List10ThenLand051B:
    lda a:OBJ_SUBSTATE,x
    bne List0EThenState2_051B
    ldy #$10
    jsr StepListByNumber051B
    bra NextSubstateOnLanding051B

; @routine 051C:713 List0AThenLand051B
; @run 70:BF13
; WHAT IT DOES: Substate 0 steps list $0A and lands ($BF1D); substate 1 is $BF2E.
; HOW: StepListByNumber051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
List0AThenLand051B:
    lda a:OBJ_SUBSTATE,x
    bne List0EThenState2_051B
    ldy #$0A
    jsr StepListByNumber051B

; @routine 051C:71D NextSubstateOnLanding051B
; @run 70:BF1D
; WHAT IT DOES: Stops on landing; landed, the next substate with the animation from its start.
; HOW: StopOnLanding with height 4.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NextSubstateOnLanding051B:
    lda #$04
    jsr VecStopOnLanding
    bcc NextSubstateOnLanding051B_at0
    inc a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
NextSubstateOnLanding051B_at0:
    rts

; @routine 051C:72E List0EThenState2_051B
; @run 70:BF2E
; WHAT IT DOES: Steps list $0E and goes to state 2 when it ends.
; HOW: StepListByNumber051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
List0EThenState2_051B:
    ldy #$0E
    jsr StepListByNumber051B
    bne NextSubstateOnLanding051B_at0
    jmp StartState2_051B

; @routine 051C:738 StartState8_051B
; @run 70:BF38
; WHAT IT DOES: Velocity by $C0A3 with OBJ_PIECE_PLACE 6, then state 8 from its start.
; HOW: SpeedByPlayerSide051B and stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartState8_051B:
    lda #$06
    sta a:OBJ_PIECE_PLACE,x
    jsr SpeedByPlayerSide051B
    lda #$08
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    rts

; @routine 051C:74F DecideByDistance051B
; @run 70:BF4F
; WHAT IT DOES: Within $40 backs off ($BF92). Further, unless slot 3 holds a kind under $58 (then $BF81), the pad's bit 6 clear
; goes to state 8 ($BF38), set to state 7 ($BF65).
; HOW: DistanceXToPlayer, the kind in slot 3 and BiosPadState.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DecideByDistance051B:
    jsr VecDistanceXToPlayer
    cmp #$40
    bcc StartState6_051B
    ldy a:OBJ_KIND+3
    beq WaitWhileCharged051B
    cmp #$58
    bcs WaitWhileCharged051B
    lda a:BiosPadState
    asl a
    bpl StartState8_051B

; @routine 051C:765 StartState7_051B
; @run 70:BF65
; WHAT IT DOES: Velocity by $C0A3 with OBJ_PIECE_PLACE 4, then state 7 rising at -4.
; HOW: SpeedByPlayerSide051B and stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartState7_051B:
    lda #$04
    sta a:OBJ_PIECE_PLACE,x
    jsr SpeedByPlayerSide051B
    lda #$07
    sta a:OBJ_STATE,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    rts

; @routine 051C:781 WaitWhileCharged051B
; @run 70:BF81
; WHAT IT DOES: Substate 0: while the player moves toward it, counts OBJ_SWOOP_SPEED_LO out and backs off ($BF92) at zero;
; otherwise reloads the count ($BFAE). Substate 1 counts down ($BFB7).
; HOW: PlayerHeadingToObject051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WaitWhileCharged051B:
    lda a:OBJ_SUBSTATE,x
    bne CountThenState2_051B
    jsr PlayerHeadingToObject051B
    bcc ReloadWaitThenNext051B
    bne ReloadWaitThenNext051B
    dec a:OBJ_SWOOP_SPEED_LO,x
    bne StartState6_051B_at0

; @routine 051C:792 StartState6_051B
; @run 70:BF92
; WHAT IT DOES: Velocity by $C0A3 with OBJ_PIECE_PLACE 2, then state 6 rising at -6.
; HOW: SpeedByPlayerSide051B and stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartState6_051B:
    lda #$02
    sta a:OBJ_PIECE_PLACE,x
    jsr SpeedByPlayerSide051B
    lda #$06
    sta a:OBJ_STATE,x
    lda #$FA
    sta a:OBJ_VY_HI,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
StartState6_051B_at0:
    rts

; @routine 051C:7AE ReloadWaitThenNext051B
; @run 70:BFAE
; WHAT IT DOES: A wait of $50 in OBJ_SWOOP_SPEED_LO and the next substate.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ReloadWaitThenNext051B:
    lda #$50
    sta a:OBJ_SWOOP_SPEED_LO,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 051C:7B7 CountThenState2_051B
; @run 70:BFB7
; WHAT IT DOES: Counts OBJ_SWOOP_SPEED_LO down and goes to state 2 at zero.
; HOW: A decrement.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountThenState2_051B:
    dec a:OBJ_SWOOP_SPEED_LO,x
    bne StartState6_051B_at0
    jmp StartState2_051B

; @routine 051C:7BF List6ThenState2_051B
; @run 70:BFBF
; WHAT IT DOES: Steps list 6 and goes to state 2 when it ends.
; HOW: StepListByNumber051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
List6ThenState2_051B:
    ldy #$06
    jsr StepListByNumber051B
    bne List6ThenState2_051B_at0
    jsr StartState2_051B
List6ThenState2_051B_at0:
    rts

; @routine 051C:7CA BySubstate051BBFCA
; @run 70:BFCA
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $BFDC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate051BBFCASubstates = $BFDC ; a handler per substate
BySubstate051BBFCA:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate051BBFCASubstates,y
    sta z:zTemp00
    lda a:BySubstate051BBFCASubstates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051C:7E2 List0TwiceThenTurn051B
; @run 70:BFE2
; WHAT IT DOES: Steps list 0; the second time it ends, counts OBJ_PIECE_PLACE up and sets the speed anew. Moves on ($C012), or
; while the list runs looks at the player ($C01E) when the step's carry is clear.
; HOW: StepListByNumber051B and the substate as the count.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
List0TwiceThenTurn051B:
    cly
    jsr StepListByNumber051B
    beq List0TwiceThenTurn051B_at0
    bcc ChooseAttackByPlayer051B
    bra MoveLandThenLookOrJump051B
List0TwiceThenTurn051B_at0:
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_SUBSTATE,x
    cmp #$02
    bne MoveLandThenLookOrJump051B
    inc a:OBJ_PIECE_PLACE,x
    jsr SpeedByPlayerSide051B
    bra MoveLandThenLookOrJump051B

; @routine 051C:7FE List2ThenTurnBack051B
; @run 70:BFFE
; WHAT IT DOES: Steps list 2; when it ends, substate 0 with OBJ_PIECE_PLACE 0 and the speed anew. Moves on ($C012), or while the
; list runs looks at the player ($C01E) when the step's carry is clear.
; HOW: StepListByNumber051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
List2ThenTurnBack051B:
    ldy #$02
    jsr StepListByNumber051B
    beq List2ThenTurnBack051B_at0
    bcc ChooseAttackByPlayer051B
    bra MoveLandThenLookOrJump051B
List2ThenTurnBack051B_at0:
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_PIECE_PLACE,x
    jsr SpeedByPlayerSide051B

; @routine 051D:012 MoveLandThenLookOrJump051B
; @run 71:C012
; WHAT IT DOES: Moves and lands; off the ground (bit 0 clear) goes to state 7 ($BF65), else looks at the player ($C01E).
; HOW: MoveAndLand with height 4.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
MoveLandThenLookOrJump051B:
    lda #$04
    jsr VecMoveAndLand
    and #$01
    bne ChooseAttackByPlayer051B
    jmp StartState7_051B

; @routine 051D:01E ChooseAttackByPlayer051B
; @run 71:C01E
; WHAT IT DOES: Within $28 goes to state 3. Under state 3 with the player facing the other way, to state 9. Otherwise, while the
; player moves away with the list stopped, to state 5.
; HOW: DistanceXToPlayer, facing, PlayerHeadingToObject051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ChooseAttackByPlayer051B:
    jsr VecDistanceXToPlayer
    cmp #$28
    bcc ChooseAttackByPlayer051B_at2
    lda a:OBJ_STATE,x
    cmp #$03
    bcs ChooseAttackByPlayer051B_at0
    lda a:OBJ_FACING_LEFT
    eor a:OBJ_FACING_LEFT,x
    bne ChooseAttackByPlayer051B_at4
ChooseAttackByPlayer051B_at0:
    jsr PlayerHeadingToObject051B
    bcc ChooseAttackByPlayer051B_at1
    beq ChooseAttackByPlayer051B_at3
ChooseAttackByPlayer051B_at1:
    rts
ChooseAttackByPlayer051B_at2:
    stz a:OBJ_PIECE_PLACE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    rts
ChooseAttackByPlayer051B_at3:
    stz a:OBJ_PIECE_PLACE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$40
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda #$32
    sta a:OBJ_FRAME,x
    jsr SetHitBoxByFrameTable051B
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    rts
ChooseAttackByPlayer051B_at4:
    lda #$08
    sta a:OBJ_PIECE_PLACE,x
    jsr SpeedByPlayerSide051B
    lda #$09
    sta a:OBJ_STATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 051D:08D PlayerHeadingToObject051B
; @run 71:C08D
; WHAT IT DOES: Carry clear when the player is not moving; else carry set and A bit 0 of the sign of its X speed against the
; object's facing.
; HOW: The player's X speed bytes; two ROLs bring the sign to bit 0.
; WAITS: no.
; Entry: X object, C as passed. Exit: C the answer; A, N and Z by the arm.
PlayerHeadingToObject051B:
    lda a:OBJ_VX_LO
    ora a:OBJ_VX_HI
    beq PlayerHeadingToObject051B_at0
    lda a:OBJ_VX_HI
    rol a
    rol a
    eor a:OBJ_FACING_LEFT,x
    and #$01
    sec
    rts
PlayerHeadingToObject051B_at0:
    clc
    rts

; @routine 051D:0A3 SpeedByPlayerSide051B
; @run 71:C0A3
; WHAT IT DOES: Faces the player and takes the X speed the table at $C0BD holds for the facing and OBJ_PIECE_PLACE.
; HOW: SideOfPlayer and a word table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SpeedByPlayerSide051BSpeeds = $C0BD ; X speed words
SpeedByPlayerSide051B:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    eor a:OBJ_PIECE_PLACE,x
    asl a
    tay
    lda a:SpeedByPlayerSide051BSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:SpeedByPlayerSide051BSpeeds,y
    sta a:OBJ_VX_LO,x
    rts

; @routine 051D:0D5 BySubstate051BC0D5
; @run 71:C0D5
; WHAT IT DOES: Goes to the substate's handler.
; HOW: A table of handlers at $C0E7.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BySubstate051BC0D5Substates = $C0E7 ; a handler per substate
BySubstate051BC0D5:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:BySubstate051BC0D5Substates,y
    sta z:zTemp00
    lda a:BySubstate051BC0D5Substates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051D:0ED BreakAndLeapWhenCameraReaches051B
; @run 71:C0ED
; WHAT IT DOES: Once its X reaches the column in OBJ_START_ARG's low digit: draws the tile script the high digit names, clears the
; next slot, starts four pieces ($B6ED), records the event, sound $81, and leaps by the player's side.
; HOW: As BreakWhenCameraReaches051B, then a speed from the table at $C14C and -4 upward.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BreakAndLeapWhenCameraReaches051BSpeeds = $C14C ; X speed words by side
BreakAndLeapWhenCameraReaches051B:
    lda a:OBJ_X_HI,x
    bne BreakAndLeapWhenCameraReaches051B_at0
    lda a:OBJ_START_ARG,x
    and #$0F
    sta z:zTemp00
    lda a:OBJ_X_LO,x
    lsr a
    lsr a
    lsr a
    lsr a
    cmp z:zTemp00
    bcs BreakAndLeapWhenCameraReaches051B_at0
    lda a:OBJ_START_ARG,x
    and #$70
    lsr a
    lsr a
    lsr a
    lsr a
    inc a
    cly
    jsr VecDrawTileScript
    txa
    tay
    iny
    jsr VecClearObjectInY
    jsr StartFourPiecesWithCount
    jsr VecRecordObjectEvent
    lda #$81
    jsr VecSoundCommand
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_LOOP_FLAGS,x
    ora #$40
    sta a:OBJ_LOOP_FLAGS,x
    jsr VecSideOfPlayer
    and #$01
    asl a
    tay
    lda a:BreakAndLeapWhenCameraReaches051BSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:BreakAndLeapWhenCameraReaches051BSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    inc a:OBJ_SUBSTATE,x
BreakAndLeapWhenCameraReaches051B_at0:
    rts

; @routine 051D:150 ListCStateTwoOnLanding051B
; @run 71:C150
; WHAT IT DOES: Steps list $0C and, on landing, goes to state 2 ($C15C).
; HOW: StepListByNumber051B, StopOnLanding with height 4.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ListCStateTwoOnLanding051B:
    ldy #$0C
    jsr StepListByNumber051B
    lda #$04
    jsr VecStopOnLanding
    bcc StartState2_051B_at0

; @routine 051D:15C StartState2_051B
; @run 71:C15C
; WHAT IT DOES: Velocity by $C0A3 with OBJ_PIECE_PLACE 0, then state 2 from its start.
; HOW: SpeedByPlayerSide051B and stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartState2_051B:
    stz a:OBJ_PIECE_PLACE,x
    jsr SpeedByPlayerSide051B
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
StartState2_051B_at0:
    rts

; @routine 051D:171 SetHitBoxByFrameTable051B
; @run 71:C171
; WHAT IT DOES: The hit box and OBJ_WORK_C and B from the tables, by the frame from $29.
; HOW: A row index table, then six columns.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SetHitBoxByFrameTable051BDx = $C1A1 ; X offsets by row
SetHitBoxByFrameTable051BDy = $C1A7 ; Y offsets by row
SetHitBoxByFrameTable051BHalfW = $C1AD ; half widths by row
SetHitBoxByFrameTable051BHalfH = $C1B3 ; half heights by row
SetHitBoxByFrameTable051BWorkC = $C1B9 ; OBJ_WORK_C by row
SetHitBoxByFrameTable051BWorkB = $C1BF ; OBJ_WORK_B by row
SetHitBoxByFrameTable051BRows = $C1C5 ; a row per frame from $29
SetHitBoxByFrameTable051B:
    lda a:OBJ_FRAME,x
    sec
    sbc #$29
    tay
    lda a:SetHitBoxByFrameTable051BRows,y
    tay
    lda a:SetHitBoxByFrameTable051BDx,y
    sta a:OBJ_HITBOX_DX,x
    lda a:SetHitBoxByFrameTable051BDy,y
    sta a:OBJ_HITBOX_DY,x
    lda a:SetHitBoxByFrameTable051BHalfW,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:SetHitBoxByFrameTable051BHalfH,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:SetHitBoxByFrameTable051BWorkC,y
    sta a:OBJ_WORK_C,x
    lda a:SetHitBoxByFrameTable051BWorkB,y
    sta a:OBJ_WORK_B,x
    rts

; @routine 051D:200 StepListByNumber051B
; @run 71:C200
; WHAT IT DOES: Steps the list the table at $C213 names for Y, then sets the hit box by the frame ($C171), keeping the step's flags.
; HOW: PHP and PLP around the call.
; WAITS: no.
; Entry: X object, Y the list number doubled. Exit: the step's flags; A, X, Y as the calls left them.
StepListByNumber051BLists = $C213 ; list addresses
StepListByNumber051B:
    lda a:StepListByNumber051BLists,y
    sta z:zTemp10
    lda a:StepListByNumber051BLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    php
    jsr SetHitBoxByFrameTable051B
    plp
    rts
