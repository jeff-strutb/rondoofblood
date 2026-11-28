; @routine 1018:4D7 PlaySound58A
; @run 6F:BCD7
; WHAT IT DOES: Plays sound $58.
; HOW: A load and a tail jump into SoundCommand.
; WAITS: no.
; Entry: Y kept for the sound. Exit: as SoundCommand.
PlaySound58A:
    lda #$58
    jmp VecSoundCommand

; @routine 1019:2C5 MaybeDropItemByPlayerDistance1019
; @run 70:A2C5
; WHAT IT DOES: MaybeDropItemByPlayerDistance with this overlay's tables at $AC4C and $AC54.
; HOW: drop_item_with_tables.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The drop arm ran in no script; the draft stops there.
MaybeDropItemByPlayerDistance1019:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    and #$07
    beq RichterStandStep1
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:$AC4C,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$AC54,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 101B:296 CountDownThenStartPieceInSlots18To1B
; @run 70:B296
; WHAT IT DOES: In state 0 loads the record. Later, once the player's screen position (OBJ_X_LO of slot 0 plus the scroll) is past page 4, it counts the 16-bit timer OBJ_WORK_D:OBJ_WORK_E down; at zero it clears the first empty slot of $18 to $1B and starts an object of type $28 there at its place with OBJ_WORK_D 1 and OBJ_WORK_E $20.
; HOW: LoadObjectRecord, a 16-bit add and subtract, a search, ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The exit when no slot is empty ran in no script.
CountDownThenStartPieceInSlots18To1B:
    lda a:OBJ_STATE,x
    bne CountDownThenStartPieceInSlots18To1B_at0
    jmp VecLoadObjectRecord
CountDownThenStartPieceInSlots18To1B_at0:
    lda a:OBJ_X_LO
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI
    adc a:ScrollXHiScreen
    cmp #$04
    bcc CountDownThenStartPieceInSlots18To1B_at1
    lda a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    beq ClearSlot4EUnlessCamera18
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    rts
    ldy #$18
CountDownThenStartPieceInSlots18To1B_at3:
    lda a:OBJ_KIND,y
    beq CountDownThenStartPieceInSlots18To1B_at2
    iny
    cpy #$1C
    bcc CountDownThenStartPieceInSlots18To1B_at3
CountDownThenStartPieceInSlots18To1B_at1:
    rts
CountDownThenStartPieceInSlots18To1B_at2:
    jsr VecClearObjectInY
    lda #$28
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,x
    lda #$20
    sta a:OBJ_WORK_E,x
    rts

; @routine 101B:2EB ChaseWanderingPointWithSpeedRamp
; @run 70:B2EB
; WHAT IT DOES: In state 0 loads the record, sets bit 4 of the flash flags and places the object at X $0020, Y $0110. Later it steps the list at $B3E3, runs SettleWhenLowOnFirstRow, faces the player, and every $30 to $6F frames (OBJ_WORK_H) picks one of eight offsets (OBJ_WORK_G, table at $B406) around the player as its target. It moves by the speed made from the angle OBJ_WORK_D and the length OBJ_WORK_E:OBJ_WORK_F, turns the angle two steps toward the target and speeds up by 8 when the angle is within 3 of it (slowing by $10 when it is not and it is going faster than $50).
; HOW: LoadObjectRecord, StepObjectFrames, SettleWhenLowOnFirstRow, SideOfPlayer, NextRandom, SetVelocityFromAngle, ApplyVelocity, AngleToPoint, TurnAngleToward, 16-bit adds in memory.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
ChaseWanderingPointWithSpeedRamp:
    lda a:OBJ_STATE,x
    bne ChaseWanderingPointWithSpeedRamp_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_SPRITE_ATTR,x
    ora #$10
    sta a:OBJ_SPRITE_ATTR,x
    stz a:OBJ_X_HI,x
    lda #$20
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_Y_HI,x
    lda #$10
    sta a:OBJ_Y_LO,x
    rts
ChaseWanderingPointWithSpeedRamp_at0:
    lda #$E3
    sta z:zTemp10
    lda #$B3
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr SettleWhenLowOnFirstRow
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_WORK_H,x
    bne ChaseWanderingPointWithSpeedRamp_at1
    jsr VecNextRandom
    and #$3F
    clc
    adc #$30
    sta a:OBJ_WORK_H,x
    jsr VecNextRandom
    and #$07
    sta a:OBJ_WORK_G,x
ChaseWanderingPointWithSpeedRamp_at1:
    dec a:OBJ_WORK_H,x
    lda a:OBJ_WORK_F,x
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_G,x
    asl a
    asl a
    tay
    lda a:OBJ_X_LO
    clc
    adc a:$B406,y
    sta z:zTemp08
    lda a:OBJ_X_HI
    adc a:$B407,y
    sta z:zTemp09
    lda a:OBJ_Y_LO
    clc
    adc a:$B408,y
    sta z:zTemp0A
    lda a:OBJ_Y_HI
    adc a:$B409,y
    sta z:zTemp0B
    jsr VecAngleToPoint
    sta z:zTemp02
    lda #$02
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    and #$FE
    sta a:OBJ_WORK_D,x
    sec
    sbc z:zTemp02
    clc
    adc #$04
    cmp #$08
    bcs ChaseWanderingPointWithSpeedRamp_at2
    lda a:OBJ_WORK_E,x
    cmp #$01
    bcc ChaseWanderingPointWithSpeedRamp_at3
    lda a:OBJ_WORK_F,x
    cmp #$10
    bcc ChaseWanderingPointWithSpeedRamp_at3
    lda #$01
    sta a:OBJ_WORK_E,x
    lda #$10
    sta a:OBJ_WORK_F,x
    rts
ChaseWanderingPointWithSpeedRamp_at3:
    lda a:OBJ_WORK_F,x
    clc
    adc #$08
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    adc #$00
    sta a:OBJ_WORK_E,x
ChaseWanderingPointWithSpeedRamp_at5:
    rts
ChaseWanderingPointWithSpeedRamp_at2:
    lda a:OBJ_WORK_E,x
    bne ChaseWanderingPointWithSpeedRamp_at4
    lda a:OBJ_WORK_F,x
    cmp #$50
    bcc ChaseWanderingPointWithSpeedRamp_at5
ChaseWanderingPointWithSpeedRamp_at4:
    lda a:OBJ_WORK_F,x
    sec
    sbc #$10
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    sbc #$00
    sta a:OBJ_WORK_E,x
    rts

; @routine 101B:426 SettleWhenLowOnFirstRow
; @run 70:B426
; WHAT IT DOES: When OBJ_WORK_I and the Y high byte are both 0 and the Y low byte is below $D0, sets OBJ_WORK_I 1 and clears the flash flags.
; HOW: An ORA, a compare, an INC, a store.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
SettleWhenLowOnFirstRow:
    lda a:OBJ_WORK_I,x
    ora a:OBJ_Y_HI,x
    bne SettleWhenLowOnFirstRow_at0
    lda a:OBJ_Y_LO,x
    cmp #$D0
    bcs SettleWhenLowOnFirstRow_at0
    inc a:OBJ_WORK_I,x
    stz a:OBJ_SPRITE_ATTR,x
SettleWhenLowOnFirstRow_at0:
    rts

; @routine 101F:199 LoadRecordFrameFromKind
; @run 71:D199
; WHAT IT DOES: In state 0 loads the record and takes OBJ_START_ARG as its frame.
; HOW: LoadObjectRecord, a copy.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordFrameFromKind:
    lda a:OBJ_STATE,x
    bne LoadRecordFrameFromKind_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
LoadRecordFrameFromKind_at0:
    rts

; @routine 1178:67A PlaySound58B
; @run 6F:BE7A
; WHAT IT DOES: Plays sound $58 (a copy of PlaySound58A for this overlay).
; HOW: A load and a tail jump into SoundCommand.
; WAITS: no.
; Entry: Y kept for the sound. Exit: as SoundCommand.
PlaySound58B:
    lda #$58
    jmp VecSoundCommand

; @routine 161B:112 CapPlayerXOnScreen1
; @run 70:B112
; WHAT IT DOES: On the scroll's screen 1 keeps slot 0's OBJ_X_LO at $36 or more.
; HOW: Two compares and a store.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The arm past the first compare ran in no script; the draft stops there.
CapPlayerXOnScreen1:
    lda a:ScrollXHiScreen
    cmp #$01
    bne CapPlayerXOnScreen1_at0
    lda a:OBJ_X_LO
    cmp #$36
    bcs CapPlayerXOnScreen1_at0
    lda #$36
    sta a:OBJ_X_LO
CapPlayerXOnScreen1_at0:
    rts

; @routine 161B:126 SpawnerWithScrollGates
; @run 70:B126
; WHAT IT DOES: In state 0 loads the record. State 1 starts list $8D on channel 3 when OBJ_START_ARG is not 0 and goes on. Later it runs CapPlayerXOnScreen1 when OBJ_START_ARG is 0, waits OBJ_WORK_D down, then with OBJ_START_ARG 0 and the scroll in the right window starts an object of type $2C in a free slot of $1A to $1F numbered by OBJ_WORK_E (which counts up) and takes a random wait of $80 to $BF; with OBJ_START_ARG not 0 and the scroll's Y in range clears and starts an object of type $55 in the first free slot of $11 to $1F and waits $D0.
; HOW: LoadObjectRecord, StartListChannelKind2, CapPlayerXOnScreen1, searches of the slot kinds, StartObjectAtPlace, NextRandom, ClearObjectInY.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
SpawnerWithScrollGates:
    lda a:OBJ_STATE,x
    bne JumpByStateTable0CAB12F
    jsr VecLoadObjectRecord
    rts
    dec a
    bne SpawnerWithScrollGates_at0
    lda a:OBJ_START_ARG,x
    beq SpawnerWithScrollGates_at1
    phx
    lda #$8D
    ldx #$03
    jsr VecStartListChannelKind2
    plx
SpawnerWithScrollGates_at1:
    inc a:OBJ_STATE,x
    rts
SpawnerWithScrollGates_at0:
    lda a:OBJ_START_ARG,x
    bne MoveAndWrapDriftA
    jsr CapPlayerXOnScreen1
    lda a:OBJ_WORK_D,x
    beq SpawnerWithScrollGates_at2
    dec a:OBJ_WORK_D,x
    rts
SpawnerWithScrollGates_at2:
    ldy #$1A
SpawnerWithScrollGates_at4:
    lda a:OBJ_KIND,y
    beq SpawnerWithScrollGates_at3
    iny
    cpy #$20
    bcc SpawnerWithScrollGates_at4
    rts
SpawnerWithScrollGates_at3:
    lda a:OBJ_START_ARG,x
    bne SpawnerWithScrollGates_at5
    lda a:ScrollXHiScreen
    cmp #$09
    bcc SpawnerWithScrollGates_at6
    bne SpawnerWithScrollGates_at7
    lda a:ScrollXLo
    cmp #$20
    bcc SpawnerWithScrollGates_at8
    rts
SpawnerWithScrollGates_at6:
    cmp #$01
    bne SpawnerWithScrollGates_at8
    lda a:ScrollXLo
    cmp #$A0
    bcc SpawnerWithScrollGates_at7
SpawnerWithScrollGates_at8:
    lda #$2C
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_E,x
    sta a:OBJ_START_ARG,y
    inc a:OBJ_WORK_E,x
    jsr VecNextRandom
    and #$3F
    clc
    adc #$80
    sta a:OBJ_WORK_D,x
SpawnerWithScrollGates_at7:
    rts
SpawnerWithScrollGates_at5:
    lda a:ScrollYHiScreen
    cmp #$02
    bcs SpawnerWithScrollGates_at9
    lda a:ScrollYLo
    cmp #$D0
    bcs SpawnerWithScrollGates_at9
    ldy #$11
SpawnerWithScrollGates_at10:
    lda a:OBJ_KIND,y
    beq EnterState5WhenFlagged
    iny
    cpy #$20
    bcc SpawnerWithScrollGates_at10
    rts
    jsr VecClearObjectInY
    lda #$55
    jsr VecStartObjectAtPlace
    lda #$D0
    sta a:OBJ_WORK_D,x
SpawnerWithScrollGates_at9:
    rts

; @routine 161E:617 LoadRecordThenDriftAndCount
; @run 71:CE17
; WHAT IT DOES: In state 0 loads the record, sets sprite set 6, frame $45, OBJ_WORK_E $10 and the speeds' low bytes 1. In state 1 it moves, counts OBJ_WORK_D down through $FF and then OBJ_WORK_E; when that goes below 0 it goes to the next state.
; HOW: LoadObjectRecord, ApplyVelocity, memory decrements, a compare, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordThenDriftAndCount:
    lda a:OBJ_STATE,x
    bne LoadRecordThenDriftAndCount_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$45
    sta a:OBJ_FRAME,x
    lda #$10
    sta a:OBJ_WORK_E,x
    lda #$01
    sta a:OBJ_VX_LO,x
    sta a:OBJ_VY_LO,x
LoadRecordThenDriftAndCount_at1:
    rts
LoadRecordThenDriftAndCount_at0:
    dec a
    bne LoadRecordThenDriftAndCount_at1
    jsr VecApplyVelocity
    dec a:OBJ_WORK_D,x
    lda a:OBJ_WORK_D,x
    cmp #$FF
    bne LoadRecordThenDriftAndCount_at1
    dec a:OBJ_WORK_E,x
    bpl LoadRecordThenDriftAndCount_at1
    inc a:OBJ_STATE,x
    rts

; @routine 18DD:260 LoadRecordAtX80
; @run 71:C260
; WHAT IT DOES: In state 0 loads the record and places the object at X $0080.
; HOW: LoadObjectRecord and two stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordAtX80:
    lda a:OBJ_STATE,x
    bne LoadRecordAtX80_at0
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_X_LO,x
    stz a:OBJ_X_HI,x
LoadRecordAtX80_at0:
    rts
