; @routine 0B4C:007 CountDownOrPickNextTimer
; @run 70:B807
; WHAT IT DOES: Counts OBJ_34B8 down and says no (C 0). At zero it clears the animation and OBJ_3778, goes to state 7, advances OBJ_35C0, takes OBJ_34B8 from the table at $B830 by its low two bits and says yes (C 1).
; HOW: A test, a memory decrement, stores, an INC, a table read.
; WAITS: no.
; Entry: X object. Exit: C 1 when the timer ran out, 0 when not; N and Z from the last load or decrement.
CountDownOrPickNextTimer:
    lda a:OBJ_34B8,x
    beq CountdownNextStateC
    dec a:OBJ_34B8,x
    clc
    rts
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$07
    sta a:OBJ_STATE,x
    stz a:OBJ_3778,x
    inc a:OBJ_35C0,x
    lda a:OBJ_35C0,x
    and #$03
    tay
    lda a:$B830,y
    sta a:OBJ_34B8,x
    sec
    rts

; @routine 0B4C:051 StepListThenPickStateByLuck
; @run 70:B851
; WHAT IT DOES: In state 0, when the frame is 4, advances OBJ_SUBSTATE; otherwise (and in later states when the animation list at $B8D2 ends) it takes frame 2, the facing of the player's side, clears the animation timer and picks the next state and OBJ_SUBSTATE from the tables at $B8A2 and $B89D by OBJ_HEALTH and a random number.
; HOW: StepObjectFrames, SideOfPlayer, NextRandom, table reads, stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. The state 0 arm ran in no script; the draft stops there.
StepListThenPickStateByLuck:
    lda a:OBJ_STATE,x
    bne StepListThenPickStateByLuck_at0
    lda a:OBJ_FRAME,x
    cmp #$04
    bne StepListThenPickStateByLuck_at1
    inc a:OBJ_SUBSTATE,x
    rts
StepListThenPickStateByLuck_at0:
    lda #$B8
    sta z:zTemp11
    lda #$D2
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne ChainSwings
StepListThenPickStateByLuck_at1:
    lda #$02
    sta a:OBJ_FRAME,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_HEALTH,x
    lsr a
    and #$F0
    sta z:zTemp00
    jsr VecNextRandom
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$B8A2,y
    tay
    lda a:$B89D,y
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 0B4C:0EF StepListThenLeapByDistance
; @run 70:B8EF
; WHAT IT DOES: Steps the animation list at $B936; when it ends plays sound $B5, clears the animation, and from the distance to the player in X (eight steps) sets the speed from the table at $B93F, goes to the next OBJ_SUBSTATE and, when facing left, reverses the X speed.
; HOW: StepObjectFrames, SoundCommand, DistanceXToPlayer, table reads, stores, NegateVelocityX.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. Reversing the speed ran in no script; the draft stops there.
StepListThenLeapByDistance:
    lda #$B9
    sta z:zTemp11
    lda #$36
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne StepListThenLeapByDistance_at0
    lda #$B5
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    asl a
    tay
    lda a:$B940,y
    sta a:OBJ_VX_HI,x
    lda a:$B93F,y
    sta a:OBJ_VX_LO,x
    lda a:$B942,y
    sta a:OBJ_VY_HI,x
    lda a:$B941,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_FACING_LEFT,x
    beq StepListThenLeapByDistance_at0
    jsr VecNegateVelocityX
StepListThenLeapByDistance_at0:
    rts

; @routine 0B4C:18A CountDownThenState5
; @run 70:B98A
; WHAT IT DOES: Counts OBJ_3510 down; at zero clears the animation and goes to state 5.
; HOW: A memory decrement, stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
CountDownThenState5:
    dec a:OBJ_3510,x
    bne CountDownThenState5_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
CountDownThenState5_at0:
    rts

; @routine 0B4C:336 SoundAndTileWriteAtFrame24
; @run 70:BB36
; WHAT IT DOES: At frame $24 plays sound $19 and writes two tile words into the background map at the column the object's X names (its position plus the scroll less 2, divided by 8, low six bits).
; HOW: SoundCommand, SelectVramWriteAddress, a 16-bit add and three shifts, VDC writes, ClearVdcControlHigh, SelectVramData.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: A $51, C from the last rotate, V from the subtract.
SoundAndTileWriteAtFrame24:
    lda a:OBJ_FRAME,x
    cmp #$24
    bne SoundAndTileWriteAtFrame24_at0
    lda #$19
    jsr VecSoundCommand
    jsr VecSelectVramWriteAddress
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:ScrollXHiScreen
    sec
    sbc #$02
    sta z:zTemp01
    lsr z:zTemp01
    ror z:zTemp00
    lsr z:zTemp01
    ror z:zTemp00
    lsr z:zTemp01
    ror z:zTemp00
    lda z:zTemp00
    and #$3F
    sta a:VDC_DATA_LO
    lda #$06
    sta a:VDC_DATA_HI
    jsr VecClearVdcControlHigh
    jsr VecSelectVramData
    lda #$8F
    sta a:VDC_DATA_LO
    lda #$51
    sta a:VDC_DATA_HI
    lda #$8F
    sta a:VDC_DATA_LO
    lda #$51
    sta a:VDC_DATA_HI
SoundAndTileWriteAtFrame24_at0:
    rts

; @routine 0B4C:38C SoundAndSpawnFiveAtFrame27
; @run 70:BB8C
; WHAT IT DOES: At frame $27 plays sound $B5 and starts up to five objects of type $27 in free slots of $20 to $3F at its place, each facing as this one and numbered 0 to 4 in OBJ_34B8.
; HOW: SoundCommand, a loop of FindFreeSlotFrom20 and StartObjectAtPlace with a counter in zTemp00.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
SoundAndSpawnFiveAtFrame27:
    lda a:OBJ_FRAME,x
    cmp #$27
    bne SoundAndSpawnFiveAtFrame27_at0
    lda #$B5
    jsr VecSoundCommand
    stz z:zTemp00
    jsr VecFindFreeSlotFrom20
    bcc SoundAndSpawnFiveAtFrame27_at0
    lda #$27
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda z:zTemp00
    sta a:OBJ_34B8,y
    cmp #$04
    beq SoundAndSpawnFiveAtFrame27_at0
    inc z:zTemp00
    bra AnimateThenNumberGroupAndLeave
SoundAndSpawnFiveAtFrame27_at0:
    rts

; @routine 0B4C:409 StepListThenState5
; @run 70:BC09
; WHAT IT DOES: Steps the animation list at $BC22; when it ends clears the animation and goes to state 5.
; HOW: StepObjectFrames, stores.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
StepListThenState5:
    lda #$BC
    sta z:zTemp11
    lda #$22
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne StepListThenState5_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
StepListThenState5_at0:
    rts

; @routine 0B4C:447 SoundThenLeapByFacing
; @run 70:BC47
; WHAT IT DOES: Plays sound $B8, gives the object the X speed from the table at $BC6B by its facing and the Y speed -$0340 (high $FC, low $C0) and goes to the next OBJ_SUBSTATE.
; HOW: SoundCommand, table reads, stores, an INC.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
SoundThenLeapByFacing:
    lda #$B8
    jsr VecSoundCommand
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:$BC6C,y
    sta a:OBJ_VX_HI,x
    lda a:$BC6B,y
    sta a:OBJ_VX_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0B4C:5BB MaybeSound19AndSpawnPiece
; @run 70:BDBB
; WHAT IT DOES: When the low two bits of A are 0 plays sound $19 and starts an object of type $9A in a free slot of $30 to $3F at the object's place at Y low $BE with OBJ_34B8 1 and OBJ_3510 a random number to 7 plus the facing.
; HOW: An AND, SoundCommand, FindFreeSlotFrom30, StartObjectAtPlace, NextRandom.
; WAITS: no.
; Entry: A the number, X object, Y kept for the sound. Exit: as the last instruction that ran.
MaybeSound19AndSpawnPiece:
    and #$03
    bne MaybeSound19AndSpawnPiece_at0
    lda #$19
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom30
    bne MaybeSound19AndSpawnPiece_at0
    lda #$9A
    jsr VecStartObjectAtPlace
    lda #$BE
    sta a:OBJ_Y_LO,y
    lda #$01
    sta a:OBJ_34B8,y
    jsr VecNextRandom
    and #$07
    clc
    adc a:OBJ_FACING_LEFT,x
    sta a:OBJ_3510,y
MaybeSound19AndSpawnPiece_at0:
    rts
