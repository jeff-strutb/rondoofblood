; @routine 0A8F:002 StepSlots54And55ThenMask0A8F
; @run 6B:9002
; @stop 904A
; WHAT IT DOES: Drives slots $54 and $55: moves $55 by its Y speed; counts $54's OBJ_WORK_E down and on zero steps
; its OBJ_WORK_D on, taking a new X speed and count from two tables; moves $54 by its X speed; while $55's
; OBJ_WORK_D is 6, copies $55's OBJ_WORK_E into $54's OBJ_WORK_F (clearing $54's frame at $1F); once $54's
; OBJ_WORK_D is 6, goes on into the VRAM mask with $54's OBJ_WORK_F (Stage 06's $8000 overlay).
; HOW: Fixed slots; a DEC and two table reads by the step; the mask by a fall into $9066.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
StepSlots54And55ThenMask0A8FSpeeds = $9049 ; the X speed per step: high byte in bits 1-7, bit 0 the low byte's $80
StepSlots54And55ThenMask0A8FCounts = $904F ; the frames each step lasts
StepSlots54And55ThenMask0A8F:
    lda a:BossGaugeEnding
    cmp #$06
    bne StepSlots54And55ThenMask0A8F_at0
    lda a:OBJ_WORK_F-3
    cmp #$1F
    bne StepSlots54And55ThenMask0A8F_at1
    stz a:OBJ_FACING_LEFT-4
StepSlots54And55ThenMask0A8F_at1:
    sta a:OBJ_WORK_G-4
StepSlots54And55ThenMask0A8F_at0:
    ldx #$55
    jsr VecApplyVelocityY
    dec a:OBJ_WORK_F-4
    bne StepSlots54And55ThenMask0A8F_at2
    inc a:OBJ_WORK_E-4
    ldy a:OBJ_WORK_E-4
    lda a:StepSlots54And55ThenMask0A8FSpeeds,y
    lsr a
    sta a:OBJ_VX_LO-4
    cla
    ror a
    sta a:OBJ_VY_HI-4
    lda a:StepSlots54And55ThenMask0A8FCounts,y
    sta a:OBJ_WORK_F-4
StepSlots54And55ThenMask0A8F_at2:
    ldx #$54
    jsr VecApplyVelocityX
    lda a:OBJ_WORK_E-4
    cmp #$06
    beq StepSlots54And55ThenMask0A8F_at3
    rts
StepSlots54And55ThenMask0A8F_at3:
    lda a:OBJ_WORK_G-4
    bra MaskVramRowsByLevel0A8F

; @routine 0A8F:066 MaskVramRowsByLevel0A8F
; @run 6B:9066
; @stop 90B1
; WHAT IT DOES: From the level in A (folded down when $20 or more) sets the mask's depth (multiplier high, $10 less
; the low four bits) and picks one of four runs of VRAM rows; for each row takes a random number and masks the row
; (Stage 06's $8000 overlay).
; HOW: Bits 4-5 of the folded level index four-byte records; a loop that adds the step to the 16-bit address.
; WAITS: no.
; Entry: A the level. Exit: A, X, Y and the flags as its last instruction or call left them.
MaskVramRowsByLevel0A8FRuns = $9056 ; VRAM address low, high, row count, address step per run
MaskVramRowsByLevel0A8F:
    sta z:zTransposeRowsLeft
    cmp #$20
    bcc MaskVramRowsByLevel0A8F_at0
    and #$1F
    lsr a
    bcc MaskVramRowsByLevel0A8F_at1
    clc
    adc #$10
MaskVramRowsByLevel0A8F_at1:
    adc #$20
MaskVramRowsByLevel0A8F_at0:
    pha
    and #$0F
    sta z:zTemp10
    lda #$10
    sec
    sbc z:zTemp10
    sta z:zTransposeRowsLeft
    pla
    lsr a
    lsr a
    and #$0C
    tax
    lda a:MaskVramRowsByLevel0A8FRuns,x
    sta z:zTemp10
    lda a:MaskVramRowsByLevel0A8FRuns+1,x
    sta z:zTemp11
    lda a:MaskVramRowsByLevel0A8FRuns+2,x
    sta z:zTemp12
    lda a:MaskVramRowsByLevel0A8FRuns+3,x
    sta z:zProduct0
MaskVramRowsByLevel0A8F_at3:
    jsr VecNextRandom
    bsr MaskVramRowsRandom
    lda z:zTemp10
    clc
    adc z:zProduct0
    sta z:zTemp10
    bcc MaskVramRowsByLevel0A8F_at2
    inc z:zTemp11
MaskVramRowsByLevel0A8F_at2:
    dec z:zTemp12
    bne MaskVramRowsByLevel0A8F_at3
    rts

; @routine 0A9A:6C4 SpawnKind2AWhenRoom0A9A
; @run 70:AEC4
; WHAT IT DOES: State 0 loads the record; then counts OBJ_WORK_D down and on zero looks for seven empty slots in a
; row at $10, $17 or $1E: with them, starts a kind $2A in the first and waits $A0 frames; without, waits $40 (Stage
; 06's overlay).
; HOW: A DEC; an OR of seven kinds per group; StartObjectAtPlace.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SpawnKind2AWhenRoom0A9A:
    lda a:OBJ_STATE,x
    bne SpawnKind2AWhenRoom0A9A_at0
    jmp VecLoadObjectRecord
SpawnKind2AWhenRoom0A9A_at0:
    lda a:OBJ_WORK_D,x
    beq SpawnKind2AWhenRoom0A9A_at1
    dec a:OBJ_WORK_D,x
    rts
SpawnKind2AWhenRoom0A9A_at1:
    ldy #$10
SpawnKind2AWhenRoom0A9A_at3:
    lda a:OBJ_KIND,y
    ora a:OBJ_KIND+1,y
    ora a:OBJ_KIND+2,y
    ora a:OBJ_KIND+3,y
    ora a:OBJ_KIND+4,y
    ora a:OBJ_KIND+5,y
    ora a:OBJ_KIND+6,y
    beq SpawnKind2AWhenRoom0A9A_at2
    tya
    clc
    adc #$07
    tay
    cpy #$25
    bne SpawnKind2AWhenRoom0A9A_at3
    lda #$40
    sta a:OBJ_WORK_D,x
    rts
SpawnKind2AWhenRoom0A9A_at2:
    jsr VecClearObjectInY
    lda #$2A
    jsr VecStartObjectAtPlace
    lda #$A0
    sta a:OBJ_WORK_D,x
    rts

; @routine 0A9A:70B PlaceAndStartSixKind2B0A9A
; @run 70:AF0B
; WHAT IT DOES: State 0 puts the object at one of eight places chosen by PlayerDistanceSum, drifts it with the
; scroll, starts six kind $2B objects in the next six slots (OBJ_WORK_F from the place's record, OBJ_WORK_D 0, 4 ..
; $14, OBJ_WORK_I 0 to 5) and loads its record; later it clears itself once the next slot no longer holds kind $2B
; (Stage 06's overlay).
; HOW: Four times the random's low three bits indexes four-byte records; a loop of six starts.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlaceAndStartSixKind2B0A9APlaces = $AF74 ; X low, Y high, Y low, the parts' OBJ_WORK_F per place
PlaceAndStartSixKind2B0A9A:
    lda a:OBJ_STATE,x
    bne PlaceAndStartSixKind2B0A9A_at0
    stz a:OBJ_X_HI,x
    jsr VecPlayerDistanceSum
    and #$07
    asl a
    asl a
    tay
    lda a:PlaceAndStartSixKind2B0A9APlaces,y
    sta a:OBJ_X_LO,x
    lda a:PlaceAndStartSixKind2B0A9APlaces+1,y
    sta a:OBJ_Y_HI,x
    lda a:PlaceAndStartSixKind2B0A9APlaces+2,y
    sta a:OBJ_Y_LO,x
    lda a:PlaceAndStartSixKind2B0A9APlaces+3,y
    sta z:zTemp01
    jsr DriftWithScrollSpeed0A9A
    txa
    tay
    iny
    lda #$06
    sta z:zTemp00
    stz z:zTemp02
    stz z:zTemp03
PlaceAndStartSixKind2B0A9A_at1:
    jsr VecClearObjectInY
    lda #$2B
    jsr VecStartObjectAtPlace
    lda z:zTemp01
    sta a:OBJ_WORK_F,y
    lda z:zTemp02
    sta a:OBJ_WORK_D,y
    lda z:zTemp03
    sta a:OBJ_WORK_I,y
    iny
    lda z:zTemp02
    clc
    adc #$04
    sta z:zTemp02
    inc z:zTemp03
    dec z:zTemp00
    bne PlaceAndStartSixKind2B0A9A_at1
    jsr VecLoadObjectRecord
    rts
PlaceAndStartSixKind2B0A9A_at0:
    lda a:OBJ_KIND+1,x
    cmp #$2B
    beq PlaceAndStartSixKind2B0A9A_at2
    jmp VecClearObject
PlaceAndStartSixKind2B0A9A_at2:
    rts

; @routine 0A9A:7D1 LoadAndArmLastPart0A9A
; @run 70:AFD1
; WHAT IT DOES: Loads the record; the part with OBJ_WORK_I 5 also takes OBJ_WORK_J from a table by
; PlayerDistanceSum, its hit box on, hit reaction $44 and its hit status cleared (Stage 06's overlay).
; HOW: A compare, a table read by the random's low two bits, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadAndArmLastPart0A9AWorkJ = $AFF5 ; four values of OBJ_WORK_J
LoadAndArmLastPart0A9A:
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_I,x
    cmp #$05
    bne LoadAndArmLastPart0A9A_at0
    jsr VecPlayerDistanceSum
    and #$03
    tay
    lda a:LoadAndArmLastPart0A9AWorkJ,y
    sta a:OBJ_WORK_J,x
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$44
    sta a:OBJ_HIT_REACTION,x
    stz a:OBJ_HIT_STATUS,x
LoadAndArmLastPart0A9A_at0:
    rts

; @routine 0A9B:04A FollowPrevThenMove0A9B
; @run 70:B04A
; WHAT IT DOES: Puts the object at the place of the slot before it, then moves it as its direction gives (Stage
; 06's overlay).
; HOW: CopyObjectPlaceBack from X - 1, then a fall into $B050.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FollowPrevThenMove0A9B:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack

; @routine 0A9B:050 MoveByDirectionFrame0A9B
; @run 70:B050
; @stop B0B5
; WHAT IT DOES: Part 5 (OBJ_WORK_I 5) turns and drops at the player instead; the others take X and Y speeds for the
; direction in OBJ_WORK_F (bit 7 reverses both), move, and show frame $70 plus the direction's top four bits (Stage
; 06's overlay).
; HOW: Twice OBJ_WORK_F indexes the high and low speed bytes; NegateVelocityX and Y; bit 6 picks the rounding
; added.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
MoveByDirectionFrame0A9BRound = $B0B5 ; the rounding added to OBJ_WORK_F, by its bit 6
MoveByDirectionFrame0A9BSpeedsHi = $B0D8 ; X speed high, Y speed high per direction
MoveByDirectionFrame0A9BSpeedsLo = $B1D8 ; X speed low, Y speed low per direction
MoveByDirectionFrame0A9B:
    lda a:OBJ_WORK_I,x
    cmp #$05
    beq TurnAndDropAtPlayer0A9B
    lda a:OBJ_WORK_F,x
    asl a
    bcs MoveByDirectionFrame0A9B_at0
    tay
    lda a:MoveByDirectionFrame0A9BSpeedsHi,y
    sta a:OBJ_VX_HI,x
    lda a:MoveByDirectionFrame0A9BSpeedsLo,y
    sta a:OBJ_VX_LO,x
    lda a:MoveByDirectionFrame0A9BSpeedsHi+1,y
    sta a:OBJ_VY_HI,x
    lda a:MoveByDirectionFrame0A9BSpeedsLo+1,y
    sta a:OBJ_VY_LO,x
    bra MoveByDirectionFrame0A9B_at1
MoveByDirectionFrame0A9B_at0:
    tay
    lda a:MoveByDirectionFrame0A9BSpeedsHi,y
    sta a:OBJ_VX_HI,x
    lda a:MoveByDirectionFrame0A9BSpeedsLo,y
    sta a:OBJ_VX_LO,x
    lda a:MoveByDirectionFrame0A9BSpeedsHi+1,y
    sta a:OBJ_VY_HI,x
    lda a:MoveByDirectionFrame0A9BSpeedsLo+1,y
    sta a:OBJ_VY_LO,x
    jsr VecNegateVelocityX
    jsr VecNegateVelocityY
MoveByDirectionFrame0A9B_at1:
    jsr VecApplyVelocity
    lda a:OBJ_WORK_F,x
    rol a
    rol a
    rol a
    and #$01
    tay
    lda a:OBJ_WORK_F,x
    clc
    adc a:MoveByDirectionFrame0A9BRound,y
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc #$70
    sta a:OBJ_FRAME,x
    rts

; @routine 0A9B:0B7 TurnAndDropAtPlayer0A9B
; @run 70:B0B7
; WHAT IT DOES: Shows frame $6B, faces the player, takes the X speed of that facing and a Y speed of $17 down, and
; moves (Stage 06's overlay).
; HOW: SideOfPlayer, a table read by the facing, ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
TurnAndDropAtPlayer0A9BSpeeds = $B0D6 ; the X speed high byte by facing
TurnAndDropAtPlayer0A9B:
    lda #$6B
    sta a:OBJ_FRAME,x
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    ldy a:OBJ_FACING_LEFT,x
    lda a:TurnAndDropAtPlayer0A9BSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$17
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    rts

; @routine 0A9B:327 BecomeKind2CInY0A9B
; @run 70:B327
; WHAT IT DOES: Starts a kind $2C in slot Y at the object's place (clearing it again if its X high byte is not 0)
; and removes the object (Stage 06's overlay).
; HOW: StartObjectAtPlace, a test, ClearObjectInY, ClearObject.
; WAITS: no.
; Entry: X the object; Y the slot for the new object. Exit: A, X, Y and the flags as its last instruction or call
; left them.
BecomeKind2CInY0A9B:
    lda #$2C
    jsr VecStartObjectAtPlace
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda a:OBJ_X_HI,y
    beq BecomeKind2CInY0A9B_at0
    jsr VecClearObjectInY
BecomeKind2CInY0A9B_at0:
    jmp VecClearObject

; @routine 0A9B:397 SoundWhenSlot29Low0A9B
; @run 70:B397
; WHAT IT DOES: Once slot $29's Y position is $00B0 or more, plays sound $4E and steps the state on (Stage 06's
; overlay).
; HOW: Two compares, SoundCommand, INC.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SoundWhenSlot29Low0A9B:
    lda a:OBJ_Y_HI+41
    bne SoundWhenSlot29Low0A9B_at0
    lda a:OBJ_Y_LO+41
    cmp #$B0
    bcc SoundWhenSlot29Low0A9B_at0
    lda #$4E
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SoundWhenSlot29Low0A9B_at0:
    rts

; @routine 0A9B:3AC SetLevelYSteps0A9B
; @run 70:B3AC
; WHAT IT DOES: Sets the level's Y word (LEVELW) to the value of step OBJ_WORK_D and steps it on; at step 4 removes
; the object (Stage 06's overlay).
; HOW: A compare, twice the step indexes a word table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetLevelYSteps0A9BValues = $B3C8 ; the level's Y word for steps 0 to 3
SetLevelYSteps0A9B:
    lda a:OBJ_WORK_D,x
    cmp #$04
    bne SetLevelYSteps0A9B_at0
    jmp VecClearObject
SetLevelYSteps0A9B_at0:
    asl a
    tay
    lda a:SetLevelYSteps0A9BValues,y
    sta a:RasterWaterWanted
    lda a:SetLevelYSteps0A9BValues+1,y
    sta a:LevelYHi
    inc a:OBJ_WORK_D,x
    rts

; @routine 0A9B:3E8 LoadFrameByArg0A9B
; @run 70:B3E8
; WHAT IT DOES: Loads the record and shows frame 1 plus OBJ_START_ARG (Stage 06's overlay).
; HOW: LoadObjectRecord and an add.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
LoadFrameByArg0A9B:
    jsr VecLoadObjectRecord
    lda #$01
    clc
    adc a:OBJ_START_ARG,x
    sta a:OBJ_FRAME,x
    rts

; @routine 0A9B:3F5 RecordThenSpeedByArg0A9B
; @run 70:B3F5
; WHAT IT DOES: With its X high byte 0 and slot $10 not holding kind $36, records the object's event, takes an X
; speed by OBJ_START_ARG and steps the state on (Stage 06's overlay).
; HOW: Two tests, RecordObjectEvent, a table read.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RecordThenSpeedByArg0A9BSpeeds = $B411 ; the X speed high byte by OBJ_START_ARG
RecordThenSpeedByArg0A9B:
    lda a:OBJ_X_HI,x
    bne RecordThenSpeedByArg0A9B_at0
    lda a:OBJ_KIND+16
    cmp #$36
    beq RecordThenSpeedByArg0A9B_at0
    jsr VecRecordObjectEvent
    ldy a:OBJ_START_ARG,x
    lda a:RecordThenSpeedByArg0A9BSpeeds,y
    sta a:OBJ_VX_HI,x
    inc a:OBJ_STATE,x
RecordThenSpeedByArg0A9B_at0:
    rts

; @routine 0A9B:433 ContactWhilePlayerMovesY0A9B
; @run 70:B433
; WHAT IT DOES: Sets sprite attribute $87; when the object touches the player while the player's Y speed is not 0,
; keeps the side the player is on in OBJ_WORK_E, sets OBJ_WORK_D 5 and steps the state on (Stage 06's overlay).
; HOW: ContactWithPlayer with a $1C by $1A box, a test of slot 0, SideOfPlayer.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ContactWhilePlayerMovesY0A9B:
    lda #$87
    sta a:OBJ_SPRITE_ATTR,x
    lda #$1C
    sta z:zTemp08
    lda #$1A
    sta z:zTemp09
    jsr ContactWithPlayer
    bcc ContactWhilePlayerMovesY0A9B_at0
    lda a:OBJ_VY_HI
    beq ContactWhilePlayerMovesY0A9B_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_WORK_E,x
    lda #$05
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
ContactWhilePlayerMovesY0A9B_at0:
    rts

; @routine 0A9B:45B ContactThenCountDown0A9B
; @run 70:B45B
; WHAT IT DOES: Lets the player touch the object, counts OBJ_WORK_D down and on zero steps the state on (Stage 06's
; overlay).
; HOW: ContactWithPlayer with a $1C by $1A box, DEC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ContactThenCountDown0A9B:
    lda #$1C
    sta z:zTemp08
    lda #$1A
    sta z:zTemp09
    jsr ContactWithPlayer
    dec a:OBJ_WORK_D,x
    bne ContactThenCountDown0A9B_at0
    inc a:OBJ_STATE,x
ContactThenCountDown0A9B_at0:
    rts

; @routine 0A9B:46F PlayListByWorkE0A9B
; @run 70:B46F
; WHAT IT DOES: Plays the frame list OBJ_WORK_E chooses; when it ends, goes to state 1 (Stage 06's overlay).
; HOW: Twice OBJ_WORK_E indexes a word table of lists; StepObjectFrames.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayListByWorkE0A9BLists = $B489 ; the frame list address per OBJ_WORK_E
PlayListByWorkE0A9B:
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:PlayListByWorkE0A9BLists,y
    sta z:zTemp10
    lda a:PlayListByWorkE0A9BLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne PlayListByWorkE0A9B_at0
    lda #$01
    sta a:OBJ_STATE,x
PlayListByWorkE0A9B_at0:
    rts

; @routine 0A9B:52B KnockBackAndBurst0A9B
; @run 70:B52B
; WHAT IT DOES: Runs the wait-to-be-hit step; then, unless the object is kind $CE, gives it a speed of $0180 across
; and $FC80 up, plays sound $19 and starts a debris burst (Stage 06's overlay).
; HOW: StepReleaseOnHit, a compare of the kind, stores, SoundCommand, StartDebrisBurst with A 1.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
KnockBackAndBurst0A9B:
    jsr StepReleaseOnHit
    lda a:OBJ_KIND,x
    cmp #$CE
    beq KnockBackAndBurst0A9B_at0
    lda #$80
    sta a:OBJ_VX_LO,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda #$19
    jsr VecSoundCommand
    lda #$01
    jsr StartDebrisBurst
KnockBackAndBurst0A9B_at0:
    rts

; @routine 0A9B:578 LoadBoxAndWaitByArg0A9B
; @run 70:B578
; WHAT IT DOES: Loads the record, shows frame 5, and takes its hit box and a wait (into OBJ_WORK_D and OBJ_WORK_E)
; by OBJ_START_ARG (Stage 06's overlay).
; HOW: Twice OBJ_START_ARG indexes the box bytes; OBJ_START_ARG indexes the waits.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadBoxAndWaitByArg0A9BWaits = $B556 ; the wait per OBJ_START_ARG
LoadBoxAndWaitByArg0A9BBoxes = $B558 ; half width, half height, X offset, Y offset, two bytes apart
LoadBoxAndWaitByArg0A9B:
    jsr VecLoadObjectRecord
    lda #$05
    sta a:OBJ_FRAME,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:LoadBoxAndWaitByArg0A9BBoxes,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:LoadBoxAndWaitByArg0A9BBoxes+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:LoadBoxAndWaitByArg0A9BBoxes+2,y
    sta a:OBJ_HITBOX_DX,x
    lda a:LoadBoxAndWaitByArg0A9BBoxes+3,y
    sta a:OBJ_HITBOX_DY,x
    ldy a:OBJ_START_ARG,x
    lda a:LoadBoxAndWaitByArg0A9BWaits,y
    sta a:OBJ_WORK_D,x
    sta a:OBJ_WORK_E,x
    rts

; @routine 0A9B:5EB FrameAndWaitByArg0A9B
; @run 70:B5EB
; WHAT IT DOES: Takes a frame and OBJ_WORK_E by OBJ_START_ARG, plays sound $3A and steps the substate on (Stage
; 06's overlay).
; HOW: Two table reads, SoundCommand, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FrameAndWaitByArg0A9BValues = $B554 ; the frame, then OBJ_WORK_E, per OBJ_START_ARG
FrameAndWaitByArg0A9B:
    ldy a:OBJ_START_ARG,x
    lda a:FrameAndWaitByArg0A9BValues,y
    sta a:OBJ_FRAME,x
    lda a:FrameAndWaitByArg0A9BValues+1,y
    sta a:OBJ_WORK_E,x
    lda #$3A
    jsr VecSoundCommand
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0A9B:603 RiseThenBecomeKind0A9B
; @run 70:B603
; WHAT IT DOES: Moves the object up a pixel a frame while OBJ_WORK_E counts down; on zero records its event and
; turns it into the kind OBJ_START_ARG chooses, in state 0 with its hit box offsets and Y speed cleared (Stage 06's
; overlay).
; HOW: Two DECs, RecordObjectEvent, a table read, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RiseThenBecomeKind0A9BKinds = $B557 ; the new kind per OBJ_START_ARG
RiseThenBecomeKind0A9B:
    dec a:OBJ_Y_LO,x
    dec a:OBJ_WORK_E,x
    bne RiseThenBecomeKind0A9B_at0
    jsr VecRecordObjectEvent
    ldy a:OBJ_START_ARG,x
    lda a:RiseThenBecomeKind0A9BKinds,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_HITBOX_DX,x
    stz a:OBJ_HITBOX_DY,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
RiseThenBecomeKind0A9B_at0:
    rts

; @routine 0A9B:6E7 BreakAfterFiveHits0A9B
; @run 70:B6E7
; WHAT IT DOES: State 0 loads the record with half height $10, sprite set 6 and frame $14; later, once hit 5 times
; or more, records its event, becomes kind $8B in state 0, plays sound $19 and starts a debris burst (Stage 06's
; overlay).
; HOW: A test of the state; a compare of OBJ_HIT_TAKEN; stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BreakAfterFiveHits0A9B:
    lda a:OBJ_STATE,x
    bne BreakAfterFiveHits0A9B_at0
    jsr VecLoadObjectRecord
    lda #$10
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$14
    sta a:OBJ_FRAME,x
    rts
BreakAfterFiveHits0A9B_at0:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc BreakAfterFiveHits0A9B_at1
    jsr VecRecordObjectEvent
    lda #$8B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    lda #$19
    jsr VecSoundCommand
    lda #$01
    jsr StartDebrisBurst
BreakAfterFiveHits0A9B_at1:
    rts

; @routine 0A9B:738 PlayFixedList0A9B
; @run 70:B738
; WHAT IT DOES: Plays the frame list at $B7B8 (Stage 06's overlay).
; HOW: StepObjectFrames.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayFixedList0A9B:
    lda #$B8
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    rts

; @routine 0A9B:7A5 FreeSlotFrom38_0A9B
; @run 70:B7A5
; WHAT IT DOES: Finds the first empty slot from $38 to $3F and clears it (Stage 06's overlay).
; HOW: A search loop, ClearObjectInY.
; WAITS: no.
; Entry: nothing in the registers. Exit: C set with Y the slot found; C clear when none is empty.
FreeSlotFrom38_0A9B:
    ldy #$38
FreeSlotFrom38_0A9B_at1:
    lda a:OBJ_KIND,y
    beq FreeSlotFrom38_0A9B_at0
    iny
    cpy #$40
    bcc FreeSlotFrom38_0A9B_at1
    clc
    rts
FreeSlotFrom38_0A9B_at0:
    jsr VecClearObjectInY
    sec
    rts

; @routine 0A9B:7BD ArcAndShedKind9B0A9B
; @run 70:B7BD
; WHAT IT DOES: State 0 loads the record with sprite set 6 and the frame and X and Y speeds OBJ_WORK_D chooses;
; later moves it with gravity and every fourth frame starts a kind $9B (slot from $20) with sprite attribute $10
; (Stage 06's overlay).
; HOW: Table reads by OBJ_WORK_D and twice it; the moves; a test of zWaveCounter; FindFreeSlotFrom20.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
ArcAndShedKind9B0A9BFrames = $B80F ; the frame per OBJ_WORK_D
ArcAndShedKind9B0A9BSpeedsX = $B812 ; the X speed per OBJ_WORK_D, a word each
ArcAndShedKind9B0A9BSpeedsY = $B818 ; the Y speed per OBJ_WORK_D, a word each
ArcAndShedKind9B0A9B:
    lda a:OBJ_STATE,x
    bne ArcAndShedKind9B0A9B_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    ldy a:OBJ_WORK_D,x
    lda a:ArcAndShedKind9B0A9BFrames,y
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:ArcAndShedKind9B0A9BSpeedsX,y
    sta a:OBJ_VX_LO,x
    lda a:ArcAndShedKind9B0A9BSpeedsX+1,y
    sta a:OBJ_VX_HI,x
    lda a:ArcAndShedKind9B0A9BSpeedsY,y
    sta a:OBJ_VY_LO,x
    lda a:ArcAndShedKind9B0A9BSpeedsY+1,y
    sta a:OBJ_VY_HI,x
    rts
ArcAndShedKind9B0A9B_at0:
    jsr VecApplyVelocityX
    jsr VecApplyVelocityY
    jsr VecApplyGravity
    tst #$03,z:zWaveCounter
    bne ArcAndShedKind9B0A9B_at1
    jsr VecFindFreeSlotFrom20
    bcc ArcAndShedKind9B0A9B_at1
    lda #$9B
    jsr VecStartObjectAtPlace
    lda #$10
    sta a:OBJ_SPRITE_ATTR,y
ArcAndShedKind9B0A9B_at1:
    rts

; @routine 0A9C:01E SteerAroundSlotPoint0A9C
; @run 70:B81E
; WHAT IT DOES: State 0 sets sprite set 6 and OBJ_PACE bits 3-6. Later: waits for or finds its target, plays its
; list, faces the player, every $10 to $1F frames picks one of eight points around the slot in OBJ_START_ARG
; (another eight round the player when it is 0), moves at angle OBJ_WORK_D and speed OBJ_WORK_E:F, turns the angle
; $10 toward the point, and speeds up by $20 to $0410 while within $10 of it or slows by $20 (not below $50) when
; off (Stage 06's overlay).
; HOW: WaitForSlotThenFind0A9C, NextRandom, SetVelocityFromAngle, AngleToPoint from a table of offsets,
; TurnAngleToward, 16-bit adds.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SteerAroundSlotPoint0A9COffsets = $B950 ; X offset word, Y offset word per point
SteerAroundSlotPoint0A9C:
    lda a:OBJ_STATE,x
    bne SteerAroundSlotPoint0A9C_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$78
    ora a:OBJ_PACE,x
    sta a:OBJ_PACE,x
    rts
SteerAroundSlotPoint0A9C_at0:
    jsr WaitForSlotThenFind0A9C
    lda #$4B
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:OBJ_WORK_H,x
    bne SteerAroundSlotPoint0A9C_at1
    jsr VecNextRandom
    and #$0F
    clc
    adc #$10
    sta a:OBJ_WORK_H,x
    jsr VecNextRandom
    and #$07
    sta a:OBJ_WORK_G,x
SteerAroundSlotPoint0A9C_at1:
    dec a:OBJ_WORK_H,x
    lda a:OBJ_WORK_F,x
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_WORK_G,x
    tst #$FF,a:OBJ_START_ARG,x
    bne SteerAroundSlotPoint0A9C_at2
    clc
    adc #$08
SteerAroundSlotPoint0A9C_at2:
    asl a
    asl a
    tay
    lda a:OBJ_START_ARG,x
    phx
    tax
    lda a:OBJ_X_LO,x
    clc
    adc a:SteerAroundSlotPoint0A9COffsets,y
    sta z:zTemp08
    lda a:OBJ_X_HI,x
    adc a:SteerAroundSlotPoint0A9COffsets+1,y
    sta z:zTemp09
    lda a:OBJ_Y_LO,x
    clc
    adc a:SteerAroundSlotPoint0A9COffsets+2,y
    sta z:zTemp0A
    lda a:OBJ_Y_HI,x
    adc a:SteerAroundSlotPoint0A9COffsets+3,y
    sta z:zTemp0B
    plx
    jsr VecAngleToPoint
    sta z:zTemp02
    lda #$10
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    sta z:zTemp01
    jsr VecTurnAngleToward
    and #$F0
    sta a:OBJ_WORK_D,x
    sec
    sbc z:zTemp02
    clc
    adc #$10
    cmp #$20
    bcs SteerAroundSlotPoint0A9C_at3
    lda a:OBJ_WORK_E,x
    cmp #$04
    bcc SteerAroundSlotPoint0A9C_at4
    lda a:OBJ_WORK_F,x
    cmp #$10
    bcc SteerAroundSlotPoint0A9C_at4
    lda #$04
    sta a:OBJ_WORK_E,x
    lda #$10
    sta a:OBJ_WORK_F,x
    rts
SteerAroundSlotPoint0A9C_at4:
    lda a:OBJ_WORK_F,x
    clc
    adc #$20
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    adc #$00
    sta a:OBJ_WORK_E,x
SteerAroundSlotPoint0A9C_at6:
    rts
SteerAroundSlotPoint0A9C_at3:
    lda a:OBJ_WORK_E,x
    bne SteerAroundSlotPoint0A9C_at5
    lda a:OBJ_WORK_F,x
    cmp #$50
    bcc SteerAroundSlotPoint0A9C_at6
SteerAroundSlotPoint0A9C_at5:
    lda a:OBJ_WORK_F,x
    sec
    sbc #$20
    sta a:OBJ_WORK_F,x
    lda a:OBJ_WORK_E,x
    sbc #$00
    sta a:OBJ_WORK_E,x
    rts

; @routine 0A9C:190 FlipPlayerBySide0A9C
; @run 70:B990
; WHAT IT DOES: In any state but 0, sets the player's sprite flip by the side the player is on, as OBJ_START_ARG
; turns it (Stage 06's overlay).
; HOW: SideOfPlayer EOR OBJ_START_ARG, bit 0 indexes a table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlipPlayerBySide0A9CFlips = $B9A8 ; the player's sprite flip byte by side
FlipPlayerBySide0A9C:
    lda a:OBJ_STATE,x
    bne FlipPlayerBySide0A9C_at0
    jmp VecLoadObjectRecord
FlipPlayerBySide0A9C_at0:
    jsr VecSideOfPlayer
    eor a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:FlipPlayerBySide0A9CFlips,y
    sta a:OBJ_SPRITE_FLIP
    rts

; @routine 0A9C:1BE LoadAndLockScroll0A9C
; @run 70:B9BE
; WHAT IT DOES: State 0 loads the record and sets the scroll's X limit high byte to one screen past its low limit;
; other states go on at $B9CE (Stage 06's overlay).
; HOW: A test of the state; LoadObjectRecord; INC and a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadAndLockScroll0A9C:
    lda a:OBJ_STATE,x
    bne WaitForFifthHit0A9C
    jsr VecLoadObjectRecord
    lda a:ScrollXMinHi
    inc a
    sta a:ScrollXMaxHi

; @routine 0A9C:1CD JustReturn0A9C
; @run 70:B9CD
; WHAT IT DOES: Returns at once (Stage 06's overlay).
; HOW: RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
JustReturn0A9C:
    rts

; @routine 0A9C:1CE WaitForFifthHit0A9C
; @run 70:B9CE
; @stop B9EA
; WHAT IT DOES: In state 1, once the object has been hit 5 times or more and a slot from $30 is free, keeps the hit
; count in OBJ_WORK_E and steps the state on; other states go on at $BA1D (Stage 06's overlay).
; HOW: DEC of the state, a compare of OBJ_HIT_TAKEN, FindFreeSlotFrom30.
; WAITS: no.
; Entry: X the object; A its state. Exit: A, X, Y and the flags as its last instruction or call left them.
WaitForFifthHit0A9C:
    dec a
    bne PlayListSoundThenSpawn0A9C
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc JustReturn0A9C
    sta a:OBJ_WORK_E,x
    jsr VecFindFreeSlotFrom30
    bcc JustReturn0A9C
    lda a:OBJ_WORK_D,x
    beq StepStateOn0A9C
    lda a:OBJ_HIT_TAKEN,x
    bra StepStateOn0A9C

; @routine 0A9C:1EC StartKindInYGuarded0A9C
; @run 70:B9EC
; WHAT IT DOES: Starts an object of kind A in slot Y at the object's place, with OBJ_INVULN_TIMER $10 and
; OBJ_START_ARG 2, and steps the object's state on (Stage 06's overlay).
; HOW: StartObjectAtPlace, two stores, a fall into $B9F9.
; WAITS: no.
; Entry: X the object; Y the free slot; A the kind. Exit: A, X, Y and the flags as its last instruction or call
; left them.
StartKindInYGuarded0A9C:
    jsr VecStartObjectAtPlace
    lda #$10
    sta a:OBJ_INVULN_TIMER,y
    lda #$02
    sta a:OBJ_START_ARG,y

; @routine 0A9C:1F9 StepStateOn0A9C
; @run 70:B9F9
; WHAT IT DOES: Steps the object's state on (Stage 06's overlay).
; HOW: INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StepStateOn0A9C:
    inc a:OBJ_STATE,x
    rts

; @routine 0A9C:1FD SpawnNextKind0A9C
; @run 70:B9FD
; @stop BA10
; WHAT IT DOES: With a free slot from $30, puts the object back to state 0 and starts kind $8B there when
; OBJ_WORK_E is $FF, or a kind chosen at random otherwise (Stage 06's overlay).
; HOW: FindFreeSlotFrom30, a compare, then $B9EC or $BA3F.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SpawnNextKind0A9C:
    jsr VecFindFreeSlotFrom30
    bcc JustReturn0A9C
    stz a:OBJ_STATE,x
    lda a:OBJ_WORK_E,x
    cmp #$FF
    bne StartRandomKind0A9C
    lda #$8B
    bra StartKindInYGuarded0A9C

; @routine 0A9C:21D PlayListSoundThenSpawn0A9C
; @run 70:BA1D
; WHAT IT DOES: Plays the frame list at $BA5C, playing sound $A7 as steps 2 and 6 begin; when the list ends, spawns
; the next object (Stage 06's overlay).
; HOW: StepObjectFrames, compares of the animation timer and step, SoundCommand; $B9FD at the end.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
PlayListSoundThenSpawn0A9C:
    lda #$BA
    sta z:zTemp11
    lda #$5C
    sta z:zTemp10
    jsr VecStepObjectFrames
    beq SpawnNextKind0A9C
    lda a:OBJ_ANIM_TIMER,x
    bne JustReturn0A9C
    lda a:OBJ_ANIM_STEP,x
    cmp #$02
    beq PlayListSoundThenSpawn0A9C_at0
    cmp #$06
    bne JustReturn0A9C
PlayListSoundThenSpawn0A9C_at0:
    lda #$A7
    jmp VecSoundCommand

; @routine 0A9C:23F StartRandomKind0A9C
; @run 70:BA3F
; WHAT IT DOES: Starts one of eight kinds, chosen at random, in slot Y (Stage 06's overlay).
; HOW: NextRandom's low three bits index a table; then $B9EC.
; WAITS: no.
; Entry: X the object; Y the free slot. Exit: A, X, Y and the flags as its last instruction or call left them.
StartRandomKind0A9CKinds = $BA54 ; eight kinds to choose from
StartRandomKind0A9C:
    jsr VecNextRandom
    and #$07
    phy
    tay
    lda a:StartRandomKind0A9CKinds,y
    ply
    bra StartKindInYGuarded0A9C

; @routine 0A9C:26F BreakIntoArgKind0A9C
; @run 70:BA6F
; WHAT IT DOES: State 0 loads the record with hit box Y offset $D0; state 1 plays the list at $BAB4 and, once hit 5
; times or more with a slot from $30 free, starts the kind in OBJ_START_ARG there (OBJ_INVULN_TIMER $10), shows
; frame $0D, steps the state on, starts a debris burst and plays sound $19 (Stage 06's overlay).
; HOW: A test of the state; StepObjectFrames; a compare; FindFreeSlotFrom30, StartObjectAtPlace, StartDebrisBurst.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BreakIntoArgKind0A9C:
    lda a:OBJ_STATE,x
    bne BreakIntoArgKind0A9C_at0
    jsr VecLoadObjectRecord
    lda #$D0
    sta a:OBJ_HITBOX_DY,x
BreakIntoArgKind0A9C_at1:
    rts
BreakIntoArgKind0A9C_at0:
    dec a
    bne BreakIntoArgKind0A9C_at1
    lda #$BA
    sta z:zTemp11
    lda #$B4
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc BreakIntoArgKind0A9C_at1
    jsr VecFindFreeSlotFrom30
    bcc BreakIntoArgKind0A9C_at1
    lda a:OBJ_START_ARG,x
    jsr VecStartObjectAtPlace
    lda #$10
    sta a:OBJ_INVULN_TIMER,y
    lda #$0D
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    lda #$01
    jsr StartDebrisBurst
    lda #$19
    jmp VecSoundCommand

; @routine 0A9C:2BF TurnOnFifthHit0A9C
; @run 70:BABF
; WHAT IT DOES: State 0 loads the record. State 1: with OBJ_START_ARG set and it and both neighbours facing left,
; starts a kind $88 (slot from $30, OBJ_INVULN_TIMER $10) and puts all three in state 3; else on its fifth hit
; turns round, waits $20 frames in state 2 with sound $3A, then goes back to state 1 (Stage 06's overlay).
; HOW: Two DECs of the state; an AND of three facings; compares; FindFreeSlotFrom30.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
TurnOnFifthHit0A9C:
    lda a:OBJ_STATE,x
    bne TurnOnFifthHit0A9C_at0
    jmp VecLoadObjectRecord
TurnOnFifthHit0A9C_at0:
    dec a
    bne TurnOnFifthHit0A9C_at1
    lda a:OBJ_START_ARG,x
    beq TurnOnFifthHit0A9C_at2
    lda a:OBJ_FACING_LEFT,x
    and a:OBJ_FACING_LEFT+1,x
    and a:OBJ_FACING_LEFT-1,x
    bne TurnOnFifthHit0A9C_at3
TurnOnFifthHit0A9C_at2:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bne TurnOnFifthHit0A9C_at4
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$20
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
    lda #$3A
    jmp VecSoundCommand
TurnOnFifthHit0A9C_at1:
    dec a
    bne TurnOnFifthHit0A9C_at4
    dec a:OBJ_WORK_D,x
    bne TurnOnFifthHit0A9C_at4
    dec a:OBJ_STATE,x
TurnOnFifthHit0A9C_at4:
    rts
TurnOnFifthHit0A9C_at3:
    jsr VecFindFreeSlotFrom30
    bcc TurnOnFifthHit0A9C_at2
    lda #$88
    jsr VecStartObjectAtPlace
    lda #$10
    sta a:OBJ_INVULN_TIMER,y
    lda #$03
    sta a:OBJ_STATE,x
    sta a:OBJ_STATE+1,x
    sta a:OBJ_STATE-1,x
    rts

; @routine 0A9C:45A RandomMinusOneToOne0A9C
; @run 70:BC5A
; WHAT IT DOES: Gives a random step of -1, 0 or 1 (Stage 06's overlay).
; HOW: NextRandom's bit 0, plus 1 on odd frames of zWaveCounter, less 1.
; WAITS: no.
; Entry: nothing in the registers. Exit: A $FF, 0 or 1; N, Z and C from the SBC.
RandomMinusOneToOne0A9C:
    jsr VecNextRandom
    and #$01
    bbr0 zWaveCounter,RandomMinusOneToOne0A9C_at0
    inc a
RandomMinusOneToOne0A9C_at0:
    sec
    sbc #$01
    rts

; @routine 0A9C:467 SoundOnce0A9C
; @run 70:BC67
; WHAT IT DOES: The first time only (OBJ_WORK_F 0), sets OBJ_WORK_F and plays sound $AB (Stage 06's overlay).
; HOW: A test, INC, SoundCommand.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SoundOnce0A9C:
    lda a:OBJ_WORK_F,x
    bne SoundOnce0A9C_at0
    inc a:OBJ_WORK_F,x
    lda #$AB
    jsr VecSoundCommand
SoundOnce0A9C_at0:
    rts

; @routine 0A9C:522 RandomMinusTwoToTwo0A9C
; @run 70:BD22
; WHAT IT DOES: Gives a random step of -2 to 2 (Stage 06's overlay).
; HOW: NextRandom's low two bits, plus 1 on odd frames of zWaveCounter, less 2.
; WAITS: no.
; Entry: nothing in the registers. Exit: A $FE to 2; N, Z and C from the SBC.
RandomMinusTwoToTwo0A9C:
    jsr VecNextRandom
    and #$03
    bbr0 zWaveCounter,RandomMinusTwoToTwo0A9C_at0
    inc a
RandomMinusTwoToTwo0A9C_at0:
    sec
    sbc #$02
    rts

; @routine 0A9D:62E EnterFromScreenEdge0A9D
; @run 71:C62E
; WHAT IT DOES: State 0 loads the record and puts the object $0C below the player at a screen edge: at X -$10
; moving right at $0120 (facing left set) when the player is at X $C0 or more, or from $40 up facing left; else at
; X $0110 moving left at $0120, one time in eight with OBJ_WORK_C $21, OBJ_WORK_F 1 and sprite attribute $8D. Later
; states go on at $C69F (Stage 06's overlay).
; HOW: Compares of the player's X and facing; stores; PlayerDistanceSum's low three bits.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
EnterFromScreenEdge0A9D:
    lda a:OBJ_STATE,x
    beq EnterFromScreenEdge0A9D_at0
    jmp BobAlongCosine0A9D
EnterFromScreenEdge0A9D_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_Y_LO
    clc
    adc #$0C
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI
    adc #$00
    sta a:OBJ_Y_HI,x
    lda a:OBJ_X_LO
    cmp #$40
    bcc EnterFromScreenEdge0A9D_at1
    cmp #$C0
    bcs EnterFromScreenEdge0A9D_at2
    lda a:OBJ_FACING_LEFT
    beq EnterFromScreenEdge0A9D_at1
EnterFromScreenEdge0A9D_at2:
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$FF
    sta a:OBJ_X_HI,x
    lda #$F0
    sta a:OBJ_X_LO,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$20
    sta a:OBJ_VX_LO,x
    rts
EnterFromScreenEdge0A9D_at1:
    lda #$01
    sta a:OBJ_X_HI,x
    lda #$10
    sta a:OBJ_X_LO,x
    lda #$FE
    sta a:OBJ_VX_HI,x
    lda #$E0
    sta a:OBJ_VX_LO,x
    jsr VecPlayerDistanceSum
    and #$07
    bne EnterFromScreenEdge0A9D_at3
    lda #$21
    sta a:OBJ_WORK_C,x
    lda #$01
    sta a:OBJ_WORK_F,x
    lda #$8D
    sta a:OBJ_SPRITE_ATTR,x
EnterFromScreenEdge0A9D_at3:
    rts

; @routine 0A9D:69F BobAlongCosine0A9D
; @run 71:C69F
; WHAT IT DOES: Plays the list at $C6EF (two frames on while rising), moves across by its X speed, and takes its Y
; speed from the cosine of OBJ_WORK_D at an amplitude of $0240, stepping OBJ_WORK_D:E on by $0280 a frame (Stage
; 06's overlay).
; HOW: StepObjectFrames, ApplyVelocityX, ScaleByCosine, ApplyVelocityY, a 16-bit add.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
BobAlongCosine0A9D:
    lda #$C6
    sta z:zTemp11
    lda #$EF
    sta z:zTemp10
    jsr VecStepObjectFrames
    bcc BobAlongCosine0A9D_at0
    lda a:OBJ_VY_HI,x
    bpl BobAlongCosine0A9D_at0
    lda a:OBJ_FRAME,x
    clc
    adc #$02
    sta a:OBJ_FRAME,x
BobAlongCosine0A9D_at0:
    jsr VecApplyVelocityX
    lda #$40
    sta z:zTemp00
    lda #$02
    sta z:zTemp01
    lda a:OBJ_WORK_D,x
    jsr VecScaleByCosine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda a:OBJ_WORK_F,x
    asl a
    tay
    lda a:OBJ_WORK_E,x
    clc
    adc #$80
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    adc #$02
    sta a:OBJ_WORK_D,x
    rts

; @routine 0A9D:716 FlipThenRunState0A9D
; @run 71:C716
; WHAT IT DOES: Sets the sprite flip by place, then runs the routine of the object's state (Stage 06's overlay).
; HOW: FlipBelowRow50AtRight0A9D; a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlipThenRunState0A9DStates = $C72B ; the routine address per state
FlipThenRunState0A9D:
    jsr FlipBelowRow50AtRight0A9D
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:FlipThenRunState0A9DStates,y
    sta z:zTemp00
    lda a:FlipThenRunState0A9DStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9D:76D LoadThenStateByArg0A9D
; @run 71:C76D
; WHAT IT DOES: Loads the record; with OBJ_START_ARG 1 faces left and goes to state 2 with speeds $0140 across and
; $00A0 down; with any other nonzero OBJ_START_ARG goes to state 5 (Stage 06's overlay).
; HOW: Two compares, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadThenStateByArg0A9D:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    beq LoadThenStateByArg0A9D_at0
    cmp #$01
    beq LoadThenStateByArg0A9D_at1
    lda #$05
    sta a:OBJ_STATE,x
LoadThenStateByArg0A9D_at0:
    rts
LoadThenStateByArg0A9D_at1:
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    lda #$01
    sta a:OBJ_VX_HI,x
    lda #$40
    sta a:OBJ_VX_LO,x
    lda #$00
    sta a:OBJ_VY_HI,x
    lda #$A0
    sta a:OBJ_VY_LO,x
    lda #$02
    sta a:OBJ_STATE,x
    rts

; @routine 0A9D:79E PlayFaceUntilNear0A9D
; @run 71:C79E
; WHAT IT DOES: Plays the list at $C7BE, faces the player, and goes to state 3 when the player is less than $60
; away across (Stage 06's overlay).
; HOW: StepObjectFrames, SideOfPlayer, DistanceXToPlayer and a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayFaceUntilNear0A9D:
    lda #$C7
    sta z:zTemp11
    lda #$BE
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs PlayFaceUntilNear0A9D_at0
    lda #$03
    sta a:OBJ_STATE,x
PlayFaceUntilNear0A9D_at0:
    rts

; @routine 0A9D:7C3 MoveUntilFarAndPast0A9D
; @run 71:C7C3
; WHAT IT DOES: Moves; when the player is $4C or more away across and the object's stage X has high byte $0A or
; more and low byte $7C or more, steps the state on (Stage 06's overlay).
; HOW: ApplyVelocity, DistanceXToPlayer, the scroll added to X, two compares.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveUntilFarAndPast0A9D:
    jsr VecApplyVelocity
    jsr VecDistanceXToPlayer
    cmp #$4C
    bcc MoveUntilFarAndPast0A9D_at0
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:ScrollXHiScreen
    cmp #$0A
    bcc MoveUntilFarAndPast0A9D_at1
    lda z:zTemp00
    cmp #$7C
    bcc MoveUntilFarAndPast0A9D_at1
MoveUntilFarAndPast0A9D_at0:
    inc a:OBJ_STATE,x
MoveUntilFarAndPast0A9D_at1:
    rts

; @routine 0A9D:7FE ChooseByPlayerDistance0A9D
; @run 71:C7FE
; WHAT IT DOES: Hit 1 to 4 times, sets OBJ_WORK_D 2; otherwise runs the choice made for the player's distance
; across, in bands of 16 pixels (Stage 06's overlay).
; HOW: The distance's top four bits index a choice; twice the choice indexes a jump table.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ChooseByPlayerDistance0A9DChoices = $C824 ; the choice per band of 16 pixels
ChooseByPlayerDistance0A9DRoutines = $C834 ; the routine address per choice
ChooseByPlayerDistance0A9D:
    lda a:OBJ_HIT_TAKEN,x
    beq ChooseByPlayerDistance0A9D_at0
    cmp #$05
    bcs ChooseByPlayerDistance0A9D_at0
    jmp SetWorkD2_0A9E
ChooseByPlayerDistance0A9D_at0:
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    tay
    lda a:ChooseByPlayerDistance0A9DChoices,y
    asl a
    tay
    lda a:ChooseByPlayerDistance0A9DRoutines,y
    sta z:zTemp00
    lda a:ChooseByPlayerDistance0A9DRoutines+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9E:03A PickWorkDByChance0A9E
; @run 71:C83A
; WHAT IT DOES: Sets OBJ_WORK_D at random: 3 for two chances in eight, 2 for one, 1 for one, 0 for the other four
; (Stage 06's overlay).
; HOW: PlayerDistanceSum's low three bits and compares.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PickWorkDByChance0A9E:
    jsr VecPlayerDistanceSum
    and #$07
    cmp #$02
    bcc SetWorkD3_0A9E
    cmp #$02
    beq SetWorkD2_0A9E
    cmp #$03
    beq SetWorkD1_0A9E

; @routine 0A9E:04B ClearWorkD0A9E
; @run 71:C84B
; WHAT IT DOES: Sets OBJ_WORK_D to 0 (Stage 06's overlay).
; HOW: One store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ClearWorkD0A9E:
    stz a:OBJ_WORK_D,x
    rts

; @routine 0A9E:04F PickWorkDByChanceAndFacing0A9E
; @run 71:C84F
; WHAT IT DOES: Sets OBJ_WORK_D at random: 3, 2 or 0 for one chance in eight each; otherwise 1 when the object
; faces the way the player faces, 0 when not (Stage 06's overlay).
; HOW: PlayerDistanceSum's low three bits, compares, an EOR of the facings.
; WAITS: no.
; Entry: X the object; A as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
PickWorkDByChanceAndFacing0A9E:
    jsr VecPlayerDistanceSum
    and #$07
    beq SetWorkD3_0A9E
    cmp #$01
    beq SetWorkD2_0A9E
    cmp #$02
    beq ClearWorkD0A9E
    lda a:OBJ_FACING_LEFT,x
    eor a:OBJ_FACING_LEFT
    bne ClearWorkD0A9E

; @routine 0A9E:066 SetWorkD1_0A9E
; @run 71:C866
; WHAT IT DOES: Sets OBJ_WORK_D to 1 (Stage 06's overlay).
; HOW: One store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetWorkD1_0A9E:
    lda #$01
    sta a:OBJ_WORK_D,x
    rts

; @routine 0A9E:06C SetWorkD2_0A9E
; @run 71:C86C
; WHAT IT DOES: Sets OBJ_WORK_D to 2 (Stage 06's overlay).
; HOW: One store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetWorkD2_0A9E:
    lda #$02
    sta a:OBJ_WORK_D,x
    rts

; @routine 0A9E:072 SetWorkD3_0A9E
; @run 71:C872
; WHAT IT DOES: Sets OBJ_WORK_D to 3 (Stage 06's overlay).
; HOW: One store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetWorkD3_0A9E:
    lda #$03
    sta a:OBJ_WORK_D,x
    rts

; @routine 0A9E:0D5 FallFrameThenLand0A9E
; @run 71:C8D5
; WHAT IT DOES: Shows the rising or falling frame by the sign of its Y speed, falls with height 6, and on landing
; goes to state 3, substate 0 (Stage 06's overlay).
; HOW: Bit 7 of the Y speed indexes a table; StopOnLanding.
; WAITS: no.
; Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FallFrameThenLand0A9EFrames = $C8F3 ; the frame while rising, then while falling
FallFrameThenLand0A9E:
    lda a:OBJ_VY_HI,x
    rol a
    rol a
    and #$01
    tay
    lda a:FallFrameThenLand0A9EFrames,y
    sta a:OBJ_FRAME,x
    lda #$06
    jsr VecStopOnLanding
    bcc FallFrameThenLand0A9E_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
FallFrameThenLand0A9E_at0:
    rts

; @routine 0A9E:0F5 HitCheckThenRunState0A9E
; @run 71:C8F5
; WHAT IT DOES: Goes to state 6 when hit, then runs the routine of the object's state (Stage 06's overlay).
; HOW: The call at $C918; a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
HitCheckThenRunState0A9EStates = $C90A ; the routine address per state
HitCheckThenRunState0A9E:
    jsr State6WhenHitBit7_0A9E
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:HitCheckThenRunState0A9EStates,y
    sta z:zTemp00
    lda a:HitCheckThenRunState0A9EStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9E:118 State6WhenHitBit7_0A9E
; @run 71:C918
; WHAT IT DOES: Puts the object in state 6 when bit 7 of OBJ_HIT_STATUS is set (Stage 06's overlay).
; HOW: An AND and a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
State6WhenHitBit7_0A9E:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State6WhenHitBit7_0A9E_at0
    lda #$06
    sta a:OBJ_STATE,x
State6WhenHitBit7_0A9E_at0:
    rts

; @routine 0A9E:134 LandThenStateOn0A9E
; @run 71:C934
; WHAT IT DOES: Falls with height 8 and on landing steps the state on (Stage 06's overlay).
; HOW: StopOnLanding, INC.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandThenStateOn0A9E:
    lda #$08
    jsr VecStopOnLanding
    bcc LandThenStateOn0A9E_at0
    inc a:OBJ_STATE,x
LandThenStateOn0A9E_at0:
    rts

; @routine 0A9E:16E FacePlayerPlayList0A9E
; @run 71:C96E
; WHAT IT DOES: Faces the player and plays the frame list that Y chooses (Stage 06's overlay).
; HOW: SideOfPlayer; Y indexes a word table of lists; StepObjectFrames.
; WAITS: no.
; Entry: X the object; Y twice the list's number. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FacePlayerPlayList0A9ELists = $C983 ; the frame list addresses
FacePlayerPlayList0A9E:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    lda a:FacePlayerPlayList0A9ELists,y
    sta z:zTemp10
    lda a:FacePlayerPlayList0A9ELists+1,y
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0A9E:2F9 LandThenSubstateOn0A9E
; @run 71:CAF9
; WHAT IT DOES: Falls with height 8 and on landing steps the substate on (Stage 06's overlay).
; HOW: StopOnLanding, INC.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandThenSubstateOn0A9E:
    lda #$08
    jsr VecStopOnLanding
    bcc LandThenSubstateOn0A9E_at0
    inc a:OBJ_SUBSTATE,x
LandThenSubstateOn0A9E_at0:
    rts

; @routine 0A9E:31A BurstIntoSixKind2E0A9E
; @run 71:CB1A
; WHAT IT DOES: Starts up to six kind $2E objects (slots from $30) at the object's place with its facing, each with
; OBJ_WORK_D, OBJ_WORK_E and X and Y speeds from its record, then removes the object (Stage 06's overlay).
; HOW: A loop of six with eight times the count indexing the records; ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BurstIntoSixKind2E0A9EParts = $CB65 ; OBJ_WORK_D, OBJ_WORK_E, X speed high, low, Y speed high, low; eight bytes apart
BurstIntoSixKind2E0A9E:
    cly
BurstIntoSixKind2E0A9E_at1:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc BurstIntoSixKind2E0A9E_at0
    lda #$2E
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    stx z:zClimbStepBefore
    lda z:zBurstCount
    asl a
    asl a
    asl a
    tax
    lda a:BurstIntoSixKind2E0A9EParts,x
    sta a:OBJ_WORK_D,y
    lda a:BurstIntoSixKind2E0A9EParts+1,x
    sta a:OBJ_WORK_E,y
    lda a:BurstIntoSixKind2E0A9EParts+2,x
    sta a:OBJ_VX_HI,y
    lda a:BurstIntoSixKind2E0A9EParts+3,x
    sta a:OBJ_VX_LO,y
    lda a:BurstIntoSixKind2E0A9EParts+4,x
    sta a:OBJ_VY_HI,y
    lda a:BurstIntoSixKind2E0A9EParts+5,x
    sta a:OBJ_VY_LO,y
    ldx z:zClimbStepBefore
    ldy z:zBurstCount
    iny
    cpy #$06
    bcc BurstIntoSixKind2E0A9E_at1
BurstIntoSixKind2E0A9E_at0:
    jmp VecClearObject

; @routine 0A9E:395 LeapTowardPlayer0A9E
; @run 71:CB95
; @stop CBD0
; WHAT IT DOES: State 0 loads the record, lifts the object 16 pixels, and gives it a speed of $FB80 up and an X
; speed toward the player that grows with the distance; later states go on at $CBE0 (Stage 06's overlay).
; HOW: ApplyVelocityY; the distance over 32 indexes a word table; NegateVelocityX by SideOfPlayer.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LeapTowardPlayer0A9ESpeeds = $CBD0 ; the X speed per 32 pixels of distance, a word each
LeapTowardPlayer0A9E:
    lda a:OBJ_STATE,x
    bne FlyListWithGravity0A9E
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
    lda a:LeapTowardPlayer0A9ESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LeapTowardPlayer0A9ESpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    jsr VecSideOfPlayer
    and #$01
    beq LeapTowardPlayer0A9E_at0
    jsr VecNegateVelocityX
LeapTowardPlayer0A9E_at0:
    rts

; @routine 0A9E:3E0 FlyListWithGravity0A9E
; @run 71:CBE0
; WHAT IT DOES: Plays the list at $C9B9, adds $24 to the Y speed and moves (Stage 06's overlay).
; HOW: StepObjectFrames, a 16-bit add, ApplyVelocity.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FlyListWithGravity0A9E:
    lda #$C9
    sta z:zTemp11
    lda #$B9
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

; @routine 0A9E:3FF FlyThenBecomeKind9B0A9E
; @run 71:CBFF
; WHAT IT DOES: State 0 loads the record and turns its X speed toward the player; later, while OBJ_WORK_E counts
; down, plays the list OBJ_WORK_D chooses and moves with gravity; on zero becomes kind $9B in state 0 (Stage 06's
; overlay).
; HOW: DEC; twice OBJ_WORK_D indexes a word table; ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlyThenBecomeKind9B0A9ELists = $CC38 ; the frame list address per OBJ_WORK_D
FlyThenBecomeKind9B0A9E:
    lda a:OBJ_STATE,x
    bne FlyThenBecomeKind9B0A9E_at0
    jsr VecLoadObjectRecord
    jsr VecSideOfPlayer
    and #$01
    beq FlyThenBecomeKind9B0A9E_at1
    jsr VecNegateVelocityX
FlyThenBecomeKind9B0A9E_at1:
    rts
FlyThenBecomeKind9B0A9E_at0:
    dec a:OBJ_WORK_E,x
    bne FlyThenBecomeKind9B0A9E_at2
    lda #$9B
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts
FlyThenBecomeKind9B0A9E_at2:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:FlyThenBecomeKind9B0A9ELists,y
    sta z:zTemp10
    lda a:FlyThenBecomeKind9B0A9ELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0A9E:440 RunStateUnlessSlowed0A9E
; @run 71:CC40
; WHAT IT DOES: Lets the player touch the object and starts its hit state; unless time is slowed this frame, hops,
; sets its hit box and runs the routine of its state (Stage 06's overlay).
; HOW: The calls at $CC6E, $CC79 and $CC93; HopAndSettle, SetHitboxByFrame; a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RunStateUnlessSlowed0A9EStates = $CC64 ; the routine address per state
RunStateUnlessSlowed0A9E:
    jsr ContactBox14By20_0A9E
    jsr State4WhenHitBit7_0A9E
RunStateUnlessSlowed0A9E_at1:
    jsr SkipFrameWhenSlowed0A9E
    bcc RunStateUnlessSlowed0A9E_at0
    rts
RunStateUnlessSlowed0A9E_at0:
    jsr HopAndSettle
    jsr SetHitboxByFrame
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:RunStateUnlessSlowed0A9EStates,y
    sta z:zTemp00
    lda a:RunStateUnlessSlowed0A9EStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0A9E:46E ContactBox14By20_0A9E
; @run 71:CC6E
; WHAT IT DOES: Tests the object's touch with the player with a $14 by $20 box (Stage 06's overlay).
; HOW: Two stores, then ContactWithPlayer.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
ContactBox14By20_0A9E:
    lda #$14
    sta z:zTemp08
    lda #$20
    sta z:zTemp09
    jmp ContactWithPlayer

; @routine 0A9E:479 State4WhenHitBit7_0A9E
; @run 71:CC79
; WHAT IT DOES: Outside state 4, when bit 7 of OBJ_HIT_STATUS is set, restarts the animation in state 4 (Stage 06's
; overlay).
; HOW: A compare, an AND, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
State4WhenHitBit7_0A9E:
    lda a:OBJ_STATE,x
    cmp #$04
    beq State4WhenHitBit7_0A9E_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State4WhenHitBit7_0A9E_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
State4WhenHitBit7_0A9E_at0:
    rts

; @routine 0A9E:493 SkipFrameWhenSlowed0A9E
; @run 71:CC93
; WHAT IT DOES: In states other than 0 and 4, while time is slowed (TimeSlowMode not 0), says whether to skip this
; frame by the pattern of the mode (mode 2 has its own) (Stage 06's overlay).
; HOW: zWaveCounter's low three bits plus $18 index a pattern table.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: C set to skip the frame; C clear to run it.
SkipFrameWhenSlowed0A9EPattern = $A146 ; nonzero for the frames skipped, at +$18 by zWaveCounter
SkipFrameWhenSlowed0A9EPattern2 = $A166 ; for mode 2: zero for the frames run, at +$18 by zWaveCounter
SkipFrameWhenSlowed0A9E:
    lda a:OBJ_STATE,x
    beq SkipFrameWhenSlowed0A9E_at0
    cmp #$04
    beq SkipFrameWhenSlowed0A9E_at0
    lda a:TimeSlowMode
    beq SkipFrameWhenSlowed0A9E_at0
    cmp #$02
    bne SkipFrameWhenSlowed0A9E_at1
    lda z:zWaveCounter
    and #$07
    clc
    adc #$18
    tay
    lda a:SkipFrameWhenSlowed0A9EPattern2,y
    beq SkipFrameWhenSlowed0A9E_at0
SkipFrameWhenSlowed0A9E_at2:
    sec
    rts
SkipFrameWhenSlowed0A9E_at1:
    lda z:zWaveCounter
    and #$07
    clc
    adc #$18
    tay
    lda a:SkipFrameWhenSlowed0A9EPattern,y
    bne SkipFrameWhenSlowed0A9E_at2
SkipFrameWhenSlowed0A9E_at0:
    clc
    rts

; @routine 0A9E:64F PlayAlternateListThenState1_0A9E
; @run 71:CE4F
; WHAT IT DOES: Plays one of two lists by bit 0 of OBJ_WORK_H; when it ends, steps OBJ_WORK_H on and goes to state
; 1 (Stage 06's overlay).
; HOW: Bit 0 of OBJ_WORK_H indexes a word table; StepObjectFrames.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayAlternateListThenState1_0A9ELists = $CE6E ; two frame list addresses
PlayAlternateListThenState1_0A9E:
    lda a:OBJ_WORK_H,x
    and #$01
    asl a
    tay
    lda a:PlayAlternateListThenState1_0A9ELists,y
    sta z:zTemp10
    lda a:PlayAlternateListThenState1_0A9ELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne PlayAlternateListThenState1_0A9E_at0
    inc a:OBJ_WORK_H,x
    lda #$01
    sta a:OBJ_STATE,x
PlayAlternateListThenState1_0A9E_at0:
    rts

; @routine 0A9E:69E PickMoveAtRandom0A9E
; @run 71:CE9E
; WHAT IT DOES: Goes back to state 1 when the player is on the side away from its facing (as OBJ_WORK_H's bit 0
; turns it); else picks OBJ_WORK_I and then OBJ_WORK_K at random, takes OBJ_WORK_J by bit 0 of OBJ_WORK_H and steps
; the substate on (Stage 06's overlay).
; HOW: SideOfPlayer and a compare; NextRandom's low four and two bits index tables.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
PickMoveAtRandom0A9EMoves = $CEE6 ; sixteen values of OBJ_WORK_I
PickMoveAtRandom0A9EWorkK = $CEF6 ; four values of OBJ_WORK_K per OBJ_WORK_I
PickMoveAtRandom0A9EWorkJ = $CF02 ; OBJ_WORK_J by bit 0 of OBJ_WORK_H
PickMoveAtRandom0A9E:
    lda a:OBJ_FACING_LEFT,x
    eor a:OBJ_WORK_H,x
    and #$01
    sta z:zTemp08
    jsr VecSideOfPlayer
    and #$01
    cmp z:zTemp08
    beq PickMoveAtRandom0A9E_at0
    lda #$01
    sta a:OBJ_STATE,x
    rts
PickMoveAtRandom0A9E_at0:
    jsr VecNextRandom
    and #$0F
    tay
    lda a:PickMoveAtRandom0A9EMoves,y
    sta a:OBJ_WORK_I,x
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    and #$03
    clc
    adc z:zTemp00
    tay
    lda a:PickMoveAtRandom0A9EWorkK,y
    sta a:OBJ_WORK_K,x
    lda a:OBJ_WORK_H,x
    and #$01
    tay
    lda a:PickMoveAtRandom0A9EWorkJ,y
    sta a:OBJ_WORK_J,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0A9E:704 ThrowKindByWorkI0A9E
; @run 71:CF04
; WHAT IT DOES: With a slot from $30, starts the kind OBJ_WORK_I chooses there with OBJ_WORK_D its OBJ_WORK_I, the
; object's facing and slot (OBJ_WORK_F), and an X speed of $F8, or $EE and the facing reversed when OBJ_WORK_J is
; set; steps the substate on. With none, stands in state 1 (Stage 06's overlay).
; HOW: FindFreeSlotFrom30, a table read, StartObjectAtPlace, stores; else $CF96.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
ThrowKindByWorkI0A9EKinds = $CF46 ; the kind per OBJ_WORK_I
ThrowKindByWorkI0A9E:
    jsr VecFindFreeSlotFrom30
    bcs ThrowKindByWorkI0A9E_at0
    jmp StandInState1_0A9E
ThrowKindByWorkI0A9E_at0:
    sty z:zBurstCount
    ldy a:OBJ_WORK_I,x
    lda a:ThrowKindByWorkI0A9EKinds,y
    ldy z:zBurstCount
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_I,x
    sta a:OBJ_WORK_D,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    txa
    sta a:OBJ_WORK_F,y
    lda a:OBJ_WORK_J,x
    bne ThrowKindByWorkI0A9E_at1
    lda #$F8
    sta a:OBJ_VX_HI,y
    bra ThrowKindByWorkI0A9E_at2
ThrowKindByWorkI0A9E_at1:
    lda #$EE
    sta a:OBJ_VX_HI,y
    lda a:OBJ_FACING_LEFT,y
    eor #$01
    sta a:OBJ_FACING_LEFT,y
ThrowKindByWorkI0A9E_at2:
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0A9E:749 PlayListByWorkIJ0A9E
; @run 71:CF49
; WHAT IT DOES: Plays the list OBJ_WORK_I and OBJ_WORK_J choose; when it ends, steps the substate on (Stage 06's
; overlay).
; HOW: Twice OBJ_WORK_I plus OBJ_WORK_J indexes a word table; StepObjectFrames.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
PlayListByWorkIJ0A9ELists = $CF65 ; the frame list addresses
PlayListByWorkIJ0A9E:
    lda a:OBJ_WORK_I,x
    asl a
    clc
    adc a:OBJ_WORK_J,x
    tay
    lda a:PlayListByWorkIJ0A9ELists,y
    sta z:zTemp10
    lda a:PlayListByWorkIJ0A9ELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne PlayListByWorkIJ0A9E_at0
    inc a:OBJ_SUBSTATE,x
PlayListByWorkIJ0A9E_at0:
    rts

; @routine 0A9E:796 StandInState1_0A9E
; @run 71:CF96
; WHAT IT DOES: Puts the object in state 1 with frame $80, or $83 when OBJ_WORK_J is set (Stage 06's overlay).
; HOW: Stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StandInState1_0A9E:
    lda #$01
    sta a:OBJ_STATE,x
    lda #$80
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_J,x
    beq StandInState1_0A9E_at0
    lda #$83
    sta a:OBJ_FRAME,x
StandInState1_0A9E_at0:
    rts

; @routine 0A9E:7AB RenumberKind30ThenClear0A9E
; @run 71:CFAB
; WHAT IT DOES: Plays one of two lists by bit 0 of OBJ_WORK_H; when it ends, numbers the kind $30 objects below
; state 4 in slots OBJ_WORK_M to OBJ_WORK_N from 0 (OBJ_START_ARG) and gives each the first one's slot
; (OBJ_WORK_M), then removes the object (Stage 06's overlay).
; HOW: A word table by bit 0; a loop over the slots; ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RenumberKind30ThenClear0A9ELists = $CFF6 ; two frame list addresses
RenumberKind30ThenClear0A9E:
    lda a:OBJ_WORK_H,x
    and #$01
    asl a
    tay
    lda a:RenumberKind30ThenClear0A9ELists,y
    sta z:zTemp10
    lda a:RenumberKind30ThenClear0A9ELists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne RenumberKind30ThenClear0A9E_at0
    ldy a:OBJ_WORK_M,x
    stz z:zTemp00
    lda a:OBJ_WORK_N,x
    sta z:zTemp01
    inc z:zTemp01
RenumberKind30ThenClear0A9E_at3:
    lda a:OBJ_KIND,y
    cmp #$30
    bne RenumberKind30ThenClear0A9E_at1
    lda a:OBJ_STATE,y
    cmp #$04
    bcs RenumberKind30ThenClear0A9E_at1
    lda z:zTemp00
    bne RenumberKind30ThenClear0A9E_at2
    sty z:zTemp03
RenumberKind30ThenClear0A9E_at2:
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    lda z:zTemp03
    sta a:OBJ_WORK_M,y
    inc z:zTemp00
RenumberKind30ThenClear0A9E_at1:
    iny
    cpy z:zTemp01
    bcc RenumberKind30ThenClear0A9E_at3
    jsr VecClearObject
RenumberKind30ThenClear0A9E_at0:
    rts

; @routine 0A9F:028 SetFlightAndSpawnKind33_0A9F
; @run 71:D028
; WHAT IT DOES: Loads the record with OBJ_WORK_E $28, moves across (X speed reversed facing left), takes X and Y
; speeds by OBJ_WORK_D and the facing, and starts a kind $33 (slot from $50) with its OBJ_WORK_F (Stage 06's
; overlay).
; HOW: Eight times OBJ_WORK_D plus four times the facing indexes four-byte records; FindFreeSlotFrom50.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SetFlightAndSpawnKind33_0A9FSpeeds = $D075 ; X speed low, high, Y speed low, high
SetFlightAndSpawnKind33_0A9F:
    jsr VecLoadObjectRecord
    lda #$28
    sta a:OBJ_WORK_E,x
    lda a:OBJ_FACING_LEFT,x
    beq SetFlightAndSpawnKind33_0A9F_at0
    jsr VecNegateVelocityX
SetFlightAndSpawnKind33_0A9F_at0:
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
    lda a:SetFlightAndSpawnKind33_0A9FSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:SetFlightAndSpawnKind33_0A9FSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:SetFlightAndSpawnKind33_0A9FSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:SetFlightAndSpawnKind33_0A9FSpeeds+2,y
    sta a:OBJ_VY_LO,x
    jsr VecFindFreeSlotFrom50
    bcc SetFlightAndSpawnKind33_0A9F_at1
    lda #$33
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,y
SetFlightAndSpawnKind33_0A9F_at1:
    rts

; @routine 0A9F:085 WaitThenArmFollowY0A9F
; @run 71:D085
; WHAT IT DOES: Counts OBJ_WORK_E down; on zero plays sound $25, turns the hit box on, restarts the animation and
; steps the state on; then follows its parent's Y (Stage 06's overlay).
; HOW: DEC, SoundCommand, stores, then $D0A0.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
WaitThenArmFollowY0A9F:
    dec a:OBJ_WORK_E,x
    bne WaitThenArmFollowY0A9F_at0
    lda #$25
    jsr VecSoundCommand
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
WaitThenArmFollowY0A9F_at0:
    jmp FollowParentYOrClear0A9F

; @routine 0A9F:0A0 FollowParentYOrClear0A9F
; @run 71:D0A0
; WHAT IT DOES: Takes the Y position of the slot in OBJ_WORK_F, and removes the object once that slot is in state 4
; (Stage 06's overlay).
; HOW: A copy and a compare; ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FollowParentYOrClear0A9F:
    ldy a:OBJ_WORK_F,x
    lda a:OBJ_Y_LO,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_STATE,y
    cmp #$04
    bne FollowParentYOrClear0A9F_at0
    jmp VecClearObject
FollowParentYOrClear0A9F_at0:
    rts

; @routine 0A9F:0B4 PlayListMoveFall0A9F
; @run 71:D0B4
; WHAT IT DOES: Plays the list OBJ_WORK_D chooses and moves; with OBJ_WORK_D 1 also adds $18 to the Y speed (Stage
; 06's overlay).
; HOW: Twice OBJ_WORK_D indexes a word table; StepObjectFrames, ApplyVelocity, a 16-bit add.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
PlayListMoveFall0A9FLists = $D0E2 ; the frame list address per OBJ_WORK_D
PlayListMoveFall0A9F:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:PlayListMoveFall0A9FLists,y
    sta z:zTemp10
    lda a:PlayListMoveFall0A9FLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_WORK_D,x
    cmp #$01
    bne PlayListMoveFall0A9F_at0
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
PlayListMoveFall0A9F_at0:
    rts

; @routine 0A9F:10E MoveAndSpawnKind33_0A9F
; @run 71:D10E
; WHAT IT DOES: Loads the record with OBJ_WORK_E $40, moves across (X speed reversed facing left), starts a kind
; $33 (slot from $50) with OBJ_WORK_D 1 and its OBJ_WORK_F, then takes the X speed of its facing and moves across
; again (Stage 06's overlay).
; HOW: NegateVelocityX, ApplyVelocityX, FindFreeSlotFrom50, a table read by the facing.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
MoveAndSpawnKind33_0A9FSpeeds = $D143 ; the X speed high byte by facing
MoveAndSpawnKind33_0A9F:
    jsr VecLoadObjectRecord
    lda #$40
    sta a:OBJ_WORK_E,x
    lda a:OBJ_FACING_LEFT,x
    beq MoveAndSpawnKind33_0A9F_at0
    jsr VecNegateVelocityX
MoveAndSpawnKind33_0A9F_at0:
    jsr VecApplyVelocityX
    jsr VecFindFreeSlotFrom50
    bcc MoveAndSpawnKind33_0A9F_at1
    lda #$33
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_WORK_D,y
    lda a:OBJ_WORK_F,x
    sta a:OBJ_WORK_F,y
MoveAndSpawnKind33_0A9F_at1:
    ldy a:OBJ_FACING_LEFT,x
    lda a:MoveAndSpawnKind33_0A9FSpeeds,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    rts

; @routine 0A9F:15D PlayBoxedListThenClear0A9F
; @run 71:D15D
; WHAT IT DOES: Plays the list at $D174 with the hit box of each frame, and removes the object when OBJ_WORK_E runs
; out (Stage 06's overlay).
; HOW: StepObjectFrames, HitBoxByFrame0A9F_197, DEC, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayBoxedListThenClear0A9F:
    lda #$D1
    sta z:zTemp11
    lda #$74
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr HitBoxByFrame0A9F_197
    dec a:OBJ_WORK_E,x
    bne PlayBoxedListThenClear0A9F_at0
    jmp VecClearObject
PlayBoxedListThenClear0A9F_at0:
    rts

; @routine 0A9F:1D5 TimedListFollowParent0A9F
; @run 71:D1D5
; WHAT IT DOES: State 0 loads the record with sprite set OBJ_WORK_D + 3 and OBJ_WORK_E $28; later plays the list
; OBJ_WORK_D chooses, follows its parent's Y, and is removed when OBJ_WORK_E runs out (Stage 06's overlay).
; HOW: Twice OBJ_WORK_D indexes a word table; StepObjectFrames, DEC, ClearObject or $D0A0.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
TimedListFollowParent0A9FLists = $D209 ; the frame list address per OBJ_WORK_D
TimedListFollowParent0A9F:
    lda a:OBJ_STATE,x
    bne TimedListFollowParent0A9F_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_D,x
    clc
    adc #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$28
    sta a:OBJ_WORK_E,x
    rts
TimedListFollowParent0A9F_at0:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:TimedListFollowParent0A9FLists,y
    sta z:zTemp10
    lda a:TimedListFollowParent0A9FLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_E,x
    bne TimedListFollowParent0A9F_at1
    jmp VecClearObject
TimedListFollowParent0A9F_at1:
    jmp FollowParentYOrClear0A9F

; @routine 0A9F:24D CarrySlotInWorkE0A9F
; @run 71:D24D
; WHAT IT DOES: Carries the slot in OBJ_WORK_E: puts it at the object's place, moves it by the offset of the
; object's frame (X reversed facing left), holds its speeds and hits at 0, and puts it back to state 2 when it is
; in state 2 or more (Stage 06's overlay).
; HOW: CopyObjectPlace; twice the frame less $9C indexes two-byte records; ApplyVelocity on that slot; stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CarrySlotInWorkE0A9FOffsets = $D2A0 ; X speed high, Y speed high per frame from $9C
CarrySlotInWorkE0A9F:
    ldy a:OBJ_WORK_E,x
    beq CarrySlotInWorkE0A9F_at0
    jsr VecCopyObjectPlace
    stx z:zClimbStepBefore
    lda a:OBJ_FRAME,x
    sec
    sbc #$9C
    asl a
    tax
    lda a:CarrySlotInWorkE0A9FOffsets,x
    sta a:OBJ_VX_HI,y
    lda a:CarrySlotInWorkE0A9FOffsets+1,x
    sta a:OBJ_VY_HI,y
    ldx z:zClimbStepBefore
    lda a:OBJ_FACING_LEFT,x
    beq CarrySlotInWorkE0A9F_at1
    lda a:OBJ_VX_HI,y
    eor #$FF
    inc a
    sta a:OBJ_VX_HI,y
CarrySlotInWorkE0A9F_at1:
    sxy
    jsr VecApplyVelocity
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_HIT_TAKEN,x
    sxy
    lda a:OBJ_STATE,y
    cmp #$02
    bcc CarrySlotInWorkE0A9F_at0
    cla
    sta a:OBJ_HIT_TAKEN,y
    lda #$02
    sta a:OBJ_STATE,y
CarrySlotInWorkE0A9F_at0:
    rts

; @routine 0A9F:2C0 RandomWorkDOnZeroG0A9F
; @run 71:D2C0
; WHAT IT DOES: When OBJ_WORK_G's low seven bits are 0, sets OBJ_WORK_D to a random 0 to 3 (Stage 06's overlay).
; HOW: An AND, PlayerDistanceSum's low two bits.
; WAITS: no.
; Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
RandomWorkDOnZeroG0A9F:
    lda a:OBJ_WORK_G,x
    and #$7F
    bne RandomWorkDOnZeroG0A9F_at0
    jsr VecPlayerDistanceSum
    and #$03
    sta a:OBJ_WORK_D,x
RandomWorkDOnZeroG0A9F_at0:
    rts

; @routine 0A9F:2FF SpawnDropByField93_0A9F
; @run 71:D2FF
; WHAT IT DOES: Starts kind $84, or with zField93 set the kind it chooses (setting PlayerItemFirstUse and clearing
; zField93; kind 0 starts nothing), in a slot from $30, keeps that slot in OBJ_WORK_E and plays sound $A8 (Stage
; 06's overlay).
; HOW: A table read by zField93, FindFreeSlotFrom30, StartObjectAtPlace, SoundCommand.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SpawnDropByField93_0A9FKinds = $D328 ; the kind per value of zField93
SpawnDropByField93_0A9F:
    lda z:zField93
    bne SpawnDropByField93_0A9F_at0
    lda #$84
    bra SpawnDropByField93_0A9F_at1
SpawnDropByField93_0A9F_at0:
    tay
    lda #$01
    sta a:PlayerItemFirstUse
    stz z:zField93
    lda a:SpawnDropByField93_0A9FKinds,y
    beq SpawnDropByField93_0A9F_at2
SpawnDropByField93_0A9F_at1:
    pha
    jsr VecFindFreeSlotFrom30
    bcc SpawnDropByField93_0A9F_at2
    pla
    jsr VecStartObjectAtPlace
    tya
    sta a:OBJ_WORK_E,x
    lda #$A8
    jsr VecSoundCommand
SpawnDropByField93_0A9F_at2:
    rts

; @routine 0A9F:337 LandThenStateOn0A9F
; @run 71:D337
; WHAT IT DOES: Falls with height $0C and on landing steps the state on (Stage 06's overlay).
; HOW: StopOnLanding, INC.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandThenStateOn0A9F:
    lda #$0C
    jsr VecStopOnLanding
    bcc LandThenStateOn0A9F_at0
    inc a:OBJ_STATE,x
LandThenStateOn0A9F_at0:
    rts

; @routine 0A9F:342 PlayFaceThenWalk0A9F
; @run 71:D342
; WHAT IT DOES: Plays the list at $D3BC, faces the player, and walks (Stage 06's overlay).
; HOW: StepObjectFrames, SideOfPlayer, then $D358.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayFaceThenWalk0A9F:
    lda #$D3
    sta z:zTemp11
    lda #$BC
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jmp WalkUntilEdge0A9F

; @routine 0A9F:358 WalkUntilEdge0A9F
; @run 71:D358
; WHAT IT DOES: Walks at the speed of OBJ_WORK_D and its facing; goes to state 3, substate 0 when the move stops
; it, when the terrain on the side it moves to has bit 0 clear, one time in 32, or while the player is in substate
; 1 (Stage 06's overlay).
; HOW: Four times OBJ_WORK_D plus twice the facing indexes a word table; MoveAndLand, ProbeTerrain,
; PlayerDistanceSum.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
WalkUntilEdge0A9FSpeeds = $D3AC ; the X speed per OBJ_WORK_D and facing, a word each
WalkUntilEdge0A9F:
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_FACING_LEFT,x
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:WalkUntilEdge0A9FSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkUntilEdge0A9FSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$0C
    jsr VecMoveAndLand
    bcs WalkUntilEdge0A9F_at0
    lda #$0C
    sta z:SpriteFlipMask
    lda #$01
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    inc a
    tay
    lda a:TileHit,y
    and #$01
    beq WalkUntilEdge0A9F_at0
    jsr VecPlayerDistanceSum
    and #$1F
    beq WalkUntilEdge0A9F_at0
    lda a:OBJ_SUBSTATE
    cmp #$01
    bne WalkUntilEdge0A9F_at1
WalkUntilEdge0A9F_at0:
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
WalkUntilEdge0A9F_at1:
    rts

; @routine 0A9F:3C3 JumpAtRandomThenLand0A9F
; @run 71:D3C3
; @stop D3FA
; WHAT IT DOES: Substate 0 takes one of four X and Y speeds at random (X reversed facing left), restarts the
; animation and steps the substate on; later ones land (Stage 06's overlay).
; HOW: Four times PlayerDistanceSum's low two bits indexes four-byte records; else $D40A.
; WAITS: no.
; Entry: X the object; C as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
JumpAtRandomThenLand0A9FSpeeds = $D3FA ; X speed low, high, Y speed low, high
JumpAtRandomThenLand0A9F:
    lda a:OBJ_SUBSTATE,x
    bne PlayListUntilLanded0A9F
    jsr VecPlayerDistanceSum
    and #$03
    asl a
    asl a
    tay
    lda a:JumpAtRandomThenLand0A9FSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:JumpAtRandomThenLand0A9FSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:JumpAtRandomThenLand0A9FSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:JumpAtRandomThenLand0A9FSpeeds+2,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
    lda a:OBJ_FACING_LEFT,x
    beq JumpAtRandomThenLand0A9F_at0
    jsr VecNegateVelocityX
JumpAtRandomThenLand0A9F_at0:
    rts

; @routine 0A9F:40A PlayListUntilLanded0A9F
; @run 71:D40A
; WHAT IT DOES: Plays the list at $D42F and falls with height $0C; on landing restarts the animation in state 2,
; substate 0, with OBJ_WORK_H 0 (Stage 06's overlay).
; HOW: StepObjectFrames, StopOnLandingFrontColumn, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
PlayListUntilLanded0A9F:
    lda #$D4
    sta z:zTemp11
    lda #$2F
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$0C
    jsr VecStopOnLandingFrontColumn
    bcc PlayListUntilLanded0A9F_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_WORK_H,x
    rts
PlayListUntilLanded0A9F_at0:
    rts

; @routine 0A9F:436 WalkRightCounted0A9F
; @run 71:D436
; WHAT IT DOES: Plays the list at $D493 facing right and walks at the speed of OBJ_WORK_D; steps the state on
; (substate 0, animation restarted) when the move stops it, when the terrain ahead has bit 0 clear, or when
; OBJ_WORK_F counts out (Stage 06's overlay).
; HOW: Four times OBJ_WORK_D indexes a word table; MoveAndLand, ProbeTerrain, DEC.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
WalkRightCounted0A9FSpeeds = $D3AC ; the X speed per OBJ_WORK_D and facing, a word each
WalkRightCounted0A9F:
    lda #$D4
    sta z:zTemp11
    lda #$93
    sta z:zTemp10
    jsr VecStepObjectFrames
    stz a:OBJ_FACING_LEFT,x
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_FACING_LEFT,x
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:WalkRightCounted0A9FSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:WalkRightCounted0A9FSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$0C
    jsr VecMoveAndLand
    bcs WalkRightCounted0A9F_at0
    lda #$0C
    sta z:SpriteFlipMask
    lda #$01
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    inc a
    tay
    lda a:TileHit,y
    and #$01
    beq WalkRightCounted0A9F_at0
    dec a:OBJ_WORK_F,x
    bne WalkRightCounted0A9F_at1
WalkRightCounted0A9F_at0:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
WalkRightCounted0A9F_at1:
    rts

; @routine 0A9F:4B2 PlayListThenRise0A9F_4B2
; @run 71:D4B2
; WHAT IT DOES: Plays the list at $D51F; when it ends, stops across, takes a Y speed of $FA00, restarts the
; animation and steps the substate on (Stage 06's overlay).
; HOW: StepObjectFrames, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayListThenRise0A9F_4B2:
    lda #$D5
    sta z:zTemp11
    lda #$1F
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne PlayListThenRise0A9F_4B2_at0
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FA
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
PlayListThenRise0A9F_4B2_at0:
    rts

; @routine 0A9F:4D7 FlyListThenSpawnKind35_0A9F
; @run 71:D4D7
; WHAT IT DOES: Moves with gravity and plays the list at $D528; when it ends, starts a kind $35 (slot from $30),
; takes a Y speed of $FE00, restarts the animation and steps the substate on (Stage 06's overlay).
; HOW: ApplyGravity, ApplyVelocity, StepObjectFrames, FindFreeSlotFrom30, StartObjectAtPlace.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FlyListThenSpawnKind35_0A9F:
    jsr VecApplyGravity
    jsr VecApplyVelocity
    lda #$D5
    sta z:zTemp11
    lda #$28
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne FlyListThenSpawnKind35_0A9F_at0
    jsr VecFindFreeSlotFrom30
    bcc FlyListThenSpawnKind35_0A9F_at0
    lda #$35
    jsr VecStartObjectAtPlace
    lda #$FE
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
FlyListThenSpawnKind35_0A9F_at0:
    rts

; @routine 0A9F:506 PlayListMoveYClear0A9F
; @run 71:D506
; WHAT IT DOES: Plays the list at $D531, then moves up or down and is removed at Y high byte $FE (Stage 06's
; overlay).
; HOW: StepObjectFrames, a fall into $D511.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayListMoveYClear0A9F:
    lda #$D5
    sta z:zTemp11
    lda #$31
    sta z:zTemp10
    jsr VecStepObjectFrames

; @routine 0A9F:511 MoveYClearAtFE0A9F
; @run 71:D511
; WHAT IT DOES: Moves by its Y speed and removes the object once its Y high byte is $FE (Stage 06's overlay).
; HOW: ApplyVelocityY, a compare, ClearObject.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MoveYClearAtFE0A9F:
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    cmp #$FE
    bne MoveYClearAtFE0A9F_at0
    jmp VecClearObject
MoveYClearAtFE0A9F_at0:
    rts

; @routine 0A9F:536 StartHighThenRise0A9F
; @run 71:D536
; WHAT IT DOES: State 0 loads the record, moves the object up by $60 and leaves it a Y speed of $FE00; later states
; move it and remove it at Y high byte $FE (Stage 06's overlay).
; HOW: LoadObjectRecord, ApplyVelocityY with $A0, a store; else $D511.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartHighThenRise0A9F:
    lda a:OBJ_STATE,x
    bne MoveYClearAtFE0A9F
    jsr VecLoadObjectRecord
    lda #$A0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$FE
    sta a:OBJ_VY_HI,x
    rts

; @routine 0A9F:57F FacePlayerEvery16_0A9F
; @run 71:D57F
; WHAT IT DOES: In the states a table marks, faces the player when OBJ_WORK_I's low four bits are 0 (Stage 06's
; overlay).
; HOW: A table read by the state, an AND, SideOfPlayer.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FacePlayerEvery16_0A9FStates = $D597 ; nonzero for the states that turn
FacePlayerEvery16_0A9F:
    ldy a:OBJ_STATE,x
    lda a:FacePlayerEvery16_0A9FStates,y
    beq FacePlayerEvery16_0A9F_at0
    lda a:OBJ_WORK_I,x
    and #$0F
    bne FacePlayerEvery16_0A9F_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FacePlayerEvery16_0A9F_at0:
    rts

; @routine 0A9F:5D2 State8WhenHit0A9F
; @run 71:D5D2
; WHAT IT DOES: Below state 8, when bit 7 of OBJ_HIT_STATUS is set, plays sound $A6 (on the first screen), restarts
; the animation and goes to state 8, substate 0 (Stage 06's overlay).
; HOW: A compare, an AND, SoundIfOnFirstScreen, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
State8WhenHit0A9F:
    lda a:OBJ_STATE,x
    cmp #$08
    bcs State8WhenHit0A9F_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State8WhenHit0A9F_at0
    lda #$A6
    jsr VecSoundIfOnFirstScreen
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$08
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State8WhenHit0A9F_at0:
    rts

; @routine 0A9F:621 StartPartsHoldDoors0A9F
; @run 71:D621
; WHAT IT DOES: Loads the record with OBJ_WORK_E $50 and sprite flip $80, holds the stage's doors shut
; (StageDoorHeld 1), and starts kinds $37 and $39 in the next two slots (Stage 06's overlay).
; HOW: Stores; ClearObjectInY and StartObjectAtPlace twice.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartPartsHoldDoors0A9F:
    jsr VecLoadObjectRecord
    lda #$50
    sta a:OBJ_WORK_E,x
    lda #$01
    sta a:StageDoorHeld
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$37
    jsr VecStartObjectAtPlace
    iny
    jsr VecClearObjectInY
    lda #$39
    jmp VecStartObjectAtPlace

; @routine 0A9F:647 StateOnWhenNear60_0A9F
; @run 71:D647
; WHAT IT DOES: Steps the state on when the player is less than $60 away across (Stage 06's overlay).
; HOW: DistanceXToPlayer, a compare, INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StateOnWhenNear60_0A9F:
    jsr VecDistanceXToPlayer
    cmp #$60
    bcs StateOnWhenNear60_0A9F_at0
    inc a:OBJ_STATE,x
StateOnWhenNear60_0A9F_at0:
    rts

; @routine 0A9F:652 PlayListThenRise0A9F_652
; @run 71:D652
; WHAT IT DOES: Plays the list at $D66B; when it ends, clears the sprite flip, takes a Y speed high byte of $FD and
; steps the state on (Stage 06's overlay).
; HOW: StepObjectFrames, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayListThenRise0A9F_652:
    lda #$6B
    sta z:zTemp10
    lda #$D6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne PlayListThenRise0A9F_652_at0
    stz a:OBJ_SPRITE_FLIP,x
    lda #$FD
    sta a:OBJ_VY_HI,x
    inc a:OBJ_STATE,x
PlayListThenRise0A9F_652_at0:
    rts

; @routine 0A9F:67E LandSoundArm0A9F
; @run 71:D67E
; WHAT IT DOES: Falls with height $0F; on landing plays sound $4A (on the first screen), turns the hit box on and
; steps the state on (Stage 06's overlay).
; HOW: StopOnLanding, SoundIfOnFirstScreen, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandSoundArm0A9F:
    lda #$0F
    jsr VecStopOnLanding
    bcc LandSoundArm0A9F_at0
    lda #$4A
    jsr VecSoundIfOnFirstScreen
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
LandSoundArm0A9F_at0:
    rts

; @routine 0A9F:6AC BounceByListStep0A9F
; @run 71:D6AC
; WHAT IT DOES: Plays the list at $D6CC; as each step begins, takes the X speed OBJ_WORK_F chooses and moves
; across, bouncing at walls (Stage 06's overlay).
; HOW: StepObjectFrames' carry, a table read, then $D6C2.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BounceByListStep0A9FSpeeds = $D6CA ; the X speed high byte per OBJ_WORK_F
BounceByListStep0A9F:
    lda #$CC
    sta z:zTemp10
    lda #$D6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc JustReturn0A9F
    ldy a:OBJ_WORK_F,x
    lda a:BounceByListStep0A9FSpeeds,y
    sta a:OBJ_VX_HI,x

; @routine 0A9F:6C2 BounceAtWallsHeightF0A9F
; @run 71:D6C2
; WHAT IT DOES: Moves across and bounces at walls with a height of $0F (Stage 06's overlay).
; HOW: A store to zProduct mid, BounceAtWalls.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BounceAtWallsHeightF0A9F:
    lda #$0F
    sta z:SpriteFlipMask
    jsr BounceAtWalls

; @routine 0A9F:6C9 JustReturn0A9F
; @run 71:D6C9
; WHAT IT DOES: Returns at once (Stage 06's overlay).
; HOW: RTS.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
JustReturn0A9F:
    rts

; @routine 0A9F:750 ThrowKind38AtFrameC3_0A9F
; @run 71:D750
; @stop D77C
; WHAT IT DOES: Plays the list at $D77C; as frame $C3 begins, plays sound $1A and starts a kind $38 (slot from $30)
; with the object's facing; when the list ends, goes on to the next round (Stage 06's overlay).
; HOW: StepObjectFrames' Z and carry, a compare, FindFreeSlotFrom30; $D78D at the end.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
ThrowKind38AtFrameC3_0A9F:
    lda #$7C
    sta z:zTemp10
    lda #$D7
    sta z:zTemp11
    jsr VecStepObjectFrames
    beq NextRoundToState4_0A9F
    bcc ThrowKind38AtFrameC3_0A9F_at0
    lda a:OBJ_FRAME,x
    cmp #$C3
    bne ThrowKind38AtFrameC3_0A9F_at0
    lda #$1A
    jsr VecSoundCommand
    jsr VecFindFreeSlotFrom30
    bcc ThrowKind38AtFrameC3_0A9F_at0
    lda #$38
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
ThrowKind38AtFrameC3_0A9F_at0:
    rts

; @routine 0A9F:78D NextRoundToState4_0A9F
; @run 71:D78D
; WHAT IT DOES: Restarts the animation in state 4, steps OBJ_WORK_G on, takes OBJ_WORK_E for it and sets OBJ_WORK_C
; 2 (Stage 06's overlay).
; HOW: OBJ_WORK_G's low two bits index a table; stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
NextRoundToState4_0A9FWaits = $D7AD ; OBJ_WORK_E per round
NextRoundToState4_0A9F:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_G,x
    lda a:OBJ_WORK_G,x
    and #$03
    tay
    lda a:NextRoundToState4_0A9FWaits,y
    sta a:OBJ_WORK_E,x
    lda #$02
    sta a:OBJ_WORK_C,x
    rts

; @routine 0A9F:7B1 SoundA5ThenSubstateOn0A9F
; @run 71:D7B1
; WHAT IT DOES: Plays sound $A5 and steps the substate on (Stage 06's overlay).
; HOW: SoundCommand, INC.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SoundA5ThenSubstateOn0A9F:
    lda #$A5
    jsr VecSoundCommand
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0A9F:7FE LeapByFacingWithSound0A9F
; @run 71:D7FE
; WHAT IT DOES: Plays sound $A5, takes the X speed of its facing and a Y speed of $FA00, sets OBJ_WORK_D $60 and
; steps the substate on (Stage 06's overlay).
; HOW: SoundCommand, a table read by the facing, stores.
; WAITS: no.
; Entry: X the object; Y as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
LeapByFacingWithSound0A9FSpeeds = $D820 ; the X speed high byte by facing
LeapByFacingWithSound0A9F:
    lda #$A5
    jsr VecSoundCommand
    ldy a:OBJ_FACING_LEFT,x
    lda a:LeapByFacingWithSound0A9FSpeeds,y
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$FA
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$60
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0AA0:022 WaitFallLand0AA0
; @run 71:D822
; WHAT IT DOES: Counts OBJ_WORK_D down; then plays the list at $D870, stops across once its stage X has high byte 3
; or more and low byte $D0 or more, falls with height $0F, and on landing plays sound $4A, sets OBJ_WORK_C 2,
; restarts the animation and steps the substate on (Stage 06's overlay).
; HOW: DEC; StepObjectFrames; the scroll added to X and two compares; StopOnLanding, SoundCommand.
; WAITS: no.
; Entry: X the object; Y as the caller set it; D as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
WaitFallLand0AA0:
    lda a:OBJ_WORK_D,x
    beq WaitFallLand0AA0_at0
    dec a:OBJ_WORK_D,x
    rts
WaitFallLand0AA0_at0:
    lda #$70
    sta z:zTemp10
    lda #$D8
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:ScrollXHiScreen
    cmp #$03
    bcc WaitFallLand0AA0_at1
    lda z:zTemp00
    cmp #$D0
    bcc WaitFallLand0AA0_at1
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
WaitFallLand0AA0_at1:
    lda #$0F
    jsr VecStopOnLanding
    bcc WaitFallLand0AA0_at2
    lda #$4A
    jsr VecSoundCommand
    lda #$02
    sta a:OBJ_WORK_C,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
WaitFallLand0AA0_at2:
    rts

; @routine 0AA0:085 PlayListThenNextRound0AA0
; @run 71:D885
; WHAT IT DOES: Plays the list at $D896; when it ends, goes on to the next round (Stage 06's overlay).
; HOW: StepObjectFrames, then $D78D.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayListThenNextRound0AA0:
    lda #$96
    sta z:zTemp10
    lda #$D8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne PlayListThenNextRound0AA0_at0
    jmp NextRoundToState4_0A9F
PlayListThenNextRound0AA0_at0:
    rts

; @routine 0AA0:0BB QuarterWorkEThenState5_0AA0
; @run 71:D8BB
; WHAT IT DOES: Plays the list at $D8E6; when it ends, quarters OBJ_WORK_E, restarts the animation in state 5 and
; gives it OBJ_INVULN_TIMER of at least 3 (Stage 06's overlay).
; HOW: StepObjectFrames, two LSRs, stores, a compare.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
QuarterWorkEThenState5_0AA0:
    lda #$E6
    sta z:zTemp10
    lda #$D8
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne QuarterWorkEThenState5_0AA0_at0
    lsr a:OBJ_WORK_E,x
    lsr a:OBJ_WORK_E,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
    lda a:OBJ_INVULN_TIMER,x
    cmp #$03
    bcs QuarterWorkEThenState5_0AA0_at0
    lda #$03
    sta a:OBJ_INVULN_TIMER,x
QuarterWorkEThenState5_0AA0_at0:
    rts

; @routine 0AA0:0ED LandThenWipe0AA0
; @run 71:D8ED
; @stop D90D
; WHAT IT DOES: Falls with height $0F; in substate 0 plays the list at $D90D and when it ends sets OBJ_WORK_D $50
; and steps the substate on; later substates wipe the VRAM blocks a step a frame (Stage 06's overlay).
; HOW: StopOnLanding, StepObjectFrames; $D926 after.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
LandThenWipe0AA0:
    lda #$0F
    jsr VecStopOnLanding
    lda a:OBJ_SUBSTATE,x
    bne WipeStepThenOpenDoors0AA0
    lda #$0D
    sta z:zTemp10
    lda #$D9
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LandThenWipe0AA0_at0
    lda #$50
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
LandThenWipe0AA0_at0:
    rts

; @routine 0AA0:126 WipeStepThenOpenDoors0AA0
; @run 71:D926
; WHAT IT DOES: Wipes the VRAM blocks by step OBJ_WORK_J (low six bits) and steps it on; when OBJ_WORK_D runs out,
; removes the object, sets slot $20's OBJ_HIT_TAKEN $FF and lets the doors open (StageDoorHeld 0) (Stage 06's
; overlay).
; HOW: The call at $DB94, INC, DEC, ClearObject, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WipeStepThenOpenDoors0AA0:
    lda a:OBJ_WORK_J,x
    and #$3F
    jsr WipeVramBlocks0AA0
    inc a:OBJ_WORK_J,x
    dec a:OBJ_WORK_D,x
    bne WipeStepThenOpenDoors0AA0_at0
    jsr VecClearObject
    lda #$FF
    sta a:OBJ_HIT_TAKEN+32
    stz a:StageDoorHeld
WipeStepThenOpenDoors0AA0_at0:
    rts

; @routine 0AA0:142 HopTowardPlayer0AA0
; @run 71:D942
; @stop D97F
; WHAT IT DOES: State 0 loads the record, moves by the X speed of its facing and $D0 up, then takes an X speed
; toward the player that grows with the distance and a Y speed high byte of $FC; later states fly (Stage 06's
; overlay).
; HOW: A table read by the facing, ApplyVelocity; the distance over 32 indexes a word table; NegateVelocityX facing
; left.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
HopTowardPlayer0AA0FirstSpeeds = $D97F ; the first X speed high byte by facing
HopTowardPlayer0AA0Speeds = $D981 ; the X speed per 32 pixels of distance, a word each
HopTowardPlayer0AA0:
    lda a:OBJ_STATE,x
    bne FlyListWithGravity0AA0
    jsr VecLoadObjectRecord
    ldy a:OBJ_FACING_LEFT,x
    lda a:HopTowardPlayer0AA0FirstSpeeds,y
    sta a:OBJ_VX_HI,x
    lda #$D0
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:HopTowardPlayer0AA0Speeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:HopTowardPlayer0AA0Speeds,y
    sta a:OBJ_VX_LO,x
    lda #$FC
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT,x
    beq HopTowardPlayer0AA0_at0
    jsr VecNegateVelocityX
HopTowardPlayer0AA0_at0:
    rts

; @routine 0AA0:191 FlyListWithGravity0AA0
; @run 71:D991
; WHAT IT DOES: Plays the list at $D9A2 and moves with gravity (Stage 06's overlay).
; HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FlyListWithGravity0AA0:
    lda #$A2
    sta z:zTemp10
    lda #$D9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0AA0:1BB FollowKind36HitBox0AA0
; @run 71:D9BB
; WHAT IT DOES: State 0 loads the record. Later, while the slot before holds kind $36, copies its OBJ_WORK_C,
; OBJ_WORK_B, hit box switch, facing and place, and takes the hit box of its frame $AD to $CC (none from $CD);
; otherwise removes itself (Stage 06's overlay).
; HOW: Copies from X - 1, CopyObjectPlaceBack; four times the frame less $AD indexes four-byte records.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FollowKind36HitBox0AA0Boxes = $DA18 ; X offset, Y offset, half width, half height per frame
FollowKind36HitBox0AA0:
    lda a:OBJ_STATE,x
    bne FollowKind36HitBox0AA0_at0
    jmp VecLoadObjectRecord
FollowKind36HitBox0AA0_at0:
    lda a:OBJ_KIND-1,x
    cmp #$36
    beq FollowKind36HitBox0AA0_at1
    jmp VecClearObject
FollowKind36HitBox0AA0_at1:
    lda a:OBJ_WORK_C-1,x
    sta a:OBJ_WORK_C,x
    lda a:OBJ_WORK_B-1,x
    sta a:OBJ_WORK_B,x
    lda a:OBJ_HITBOX_ON-1,x
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_FACING_LEFT-1,x
    sta a:OBJ_FACING_LEFT,x
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_FRAME-1,x
    cmp #$CD
    bcc FollowKind36HitBox0AA0_at2
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_HALF_H,x
    rts
FollowKind36HitBox0AA0_at2:
    sec
    sbc #$AD
    asl a
    asl a
    tay
    lda a:FollowKind36HitBox0AA0Boxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:FollowKind36HitBox0AA0Boxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:FollowKind36HitBox0AA0Boxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:FollowKind36HitBox0AA0Boxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0AA0:2B4 FollowTwoBackOrHandOff0AA0
; @run 71:DAB4
; WHAT IT DOES: When the slot two before is in state 8, starts a kind $39 (slot from $50) in state 2 with
; OBJ_WORK_E $A0 and the object's facing, and removes the object; otherwise, for that slot's frames $AD to $CC,
; takes its place and facing, moves by the frame's offset (X reversed facing left) and lets the player touch it
; (Stage 06's overlay).
; HOW: A compare; FindFreeSlotFrom50; twice the frame less $AD indexes two-byte records; CopyObjectPlaceBack,
; ApplyVelocity, ContactWithPlayer with a $11 by $1D box.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FollowTwoBackOrHandOff0AA0Offsets = $DB17 ; X speed high, Y speed high per frame from $AD
FollowTwoBackOrHandOff0AA0:
    lda a:OBJ_STATE-2,x
    cmp #$08
    bne FollowTwoBackOrHandOff0AA0_at0
    jsr VecFindFreeSlotFrom50
    bcc FollowTwoBackOrHandOff0AA0_at1
    lda #$39
    jsr VecStartObjectAtPlace
    lda #$A0
    sta a:OBJ_WORK_E,y
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda #$02
    sta a:OBJ_STATE,y
    jmp VecClearObject
FollowTwoBackOrHandOff0AA0_at0:
    lda a:OBJ_FRAME-2,x
    cmp #$CD
    bcs FollowTwoBackOrHandOff0AA0_at1
    sec
    sbc #$AD
    asl a
    tay
    lda a:FollowTwoBackOrHandOff0AA0Offsets,y
    sta a:OBJ_VX_HI,x
    lda a:FollowTwoBackOrHandOff0AA0Offsets+1,y
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT-2,x
    sta a:OBJ_FACING_LEFT,x
    beq FollowTwoBackOrHandOff0AA0_at2
    lda a:OBJ_VX_HI,x
    eor #$FF
    inc a
    sta a:OBJ_VX_HI,x
FollowTwoBackOrHandOff0AA0_at2:
    txa
    tay
    dey
    dey
    jsr VecCopyObjectPlaceBack
    jsr VecApplyVelocity
    lda #$11
    sta z:zTemp08
    lda #$1D
    sta z:zTemp09
    jsr ContactWithPlayer
FollowTwoBackOrHandOff0AA0_at1:
    rts

; @routine 0AA0:357 BounceToRest0AA0
; @run 71:DB57
; WHAT IT DOES: Shows frame $D6 and counts OBJ_WORK_E down (on zero steps the state on); falls with height 0 and on
; each landing bounces with the Y speed of OBJ_WORK_D, stepping it on up to 6 (Stage 06's overlay).
; HOW: DEC; StopOnLanding; twice OBJ_WORK_D indexes a word table.
; WAITS: no.
; Entry: X the object; Y as the caller set it; C as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
BounceToRest0AA0Speeds = $DB85 ; the Y speed per bounce, a word each
BounceToRest0AA0:
    lda #$D6
    sta a:OBJ_FRAME,x
    dec a:OBJ_WORK_E,x
    bne BounceToRest0AA0_at0
    inc a:OBJ_STATE,x
    rts
BounceToRest0AA0_at0:
    lda #$00
    jsr VecStopOnLanding
    bcc BounceToRest0AA0_at1
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:BounceToRest0AA0Speeds+1,y
    sta a:OBJ_VY_HI,x
    lda a:BounceToRest0AA0Speeds,y
    sta a:OBJ_VY_LO,x
    cpy #$0C
    beq BounceToRest0AA0_at1
    inc a:OBJ_WORK_D,x
BounceToRest0AA0_at1:
    rts

; @routine 0AA0:394 WipeVramBlocks0AA0
; @run 71:DB94
; WHAT IT DOES: Clears one bit, in one of every four words, of each of ten blocks of $40 VRAM words, as the step in
; A chooses (Stage 06's overlay).
; HOW: The mask from $DBA3, then $DBCE for blocks 0 to 9.
; WAITS: no.
; Entry: A the step: bits 4-5 the word of four, bits 0-3 the bit. Exit: A, X, Y and the flags as its last
; instruction or call left them.
WipeVramBlocks0AA0:
    phx
    jsr WipeMaskForStep0AA0
    clx
WipeVramBlocks0AA0_at0:
    jsr WipeVramBlock0AA0
    inx
    cpx #$0A
    bcc WipeVramBlocks0AA0_at0
    plx
    rts

; @routine 0AA0:3A3 WipeMaskForStep0AA0
; @run 71:DBA3
; WHAT IT DOES: Splits the step in A into the word of four (zTemp00, from its top four bits) and a mask with every
; bit set but the one its low four bits name (zTemp01:zTemp02) (Stage 06's overlay).
; HOW: LSRs, then a loop of 16-bit shifts and an EOR.
; WAITS: no.
; Entry: A the step. Exit: zTemp00 the word of four; zTemp01:zTemp02 the mask.
WipeMaskForStep0AA0:
    pha
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp00
    pla
    and #$0F
    sta z:zTemp03
    lda #$01
    sta z:zTemp01
    stz z:zTemp02
WipeMaskForStep0AA0_at1:
    lda z:zTemp03
    beq WipeMaskForStep0AA0_at0
    asl z:zTemp01
    rol z:zTemp02
    dec z:zTemp03
    bra WipeMaskForStep0AA0_at1
WipeMaskForStep0AA0_at0:
    lda z:zTemp01
    eor #$FF
    sta z:zTemp01
    lda z:zTemp02
    eor #$FF
    sta z:zTemp02
    rts

; @routine 0AA0:3CE WipeVramBlock0AA0
; @run 71:DBCE
; WHAT IT DOES: Points at VRAM block X and wipes its $40 words with the mask (Stage 06's overlay).
; HOW: Twice X indexes a word table into zBiosC; then $DBF3.
; WAITS: no.
; Entry: X the block; zTemp00 and zTemp01:zTemp02 as $DBA3 left them. Exit: A, X, Y and the flags as its last
; instruction or call left them.
WipeVramBlock0AA0Addresses = $DBDF ; the VRAM address per block
WipeVramBlock0AA0:
    txa
    asl a
    tay
    lda a:WipeVramBlock0AA0Addresses,y
    sta z:zBiosCl
    lda a:WipeVramBlock0AA0Addresses+1,y
    sta z:zBiosCh
    jsr MaskVramWords0AA0
    rts

; @routine 0AA0:3F3 MaskVramWords0AA0
; @run 71:DBF3
; WHAT IT DOES: ANDs the mask zTemp01:zTemp02 into every fourth of the $40 VRAM words from zBiosC, the ones whose
; number's low two bits equal zTemp00 (Stage 06's overlay).
; HOW: A loop of $40 that sets the VDC's write and read addresses, reads, ANDs and writes the word.
; WAITS: no.
; Entry: zBiosC the VRAM address; zTemp00 the word of four; zTemp01:zTemp02 the mask. Exit: zBiosC $40 on; Y kept.
MaskVramWords0AA0:
    phy
    cly
MaskVramWords0AA0_at1:
    tya
    and #$03
    cmp z:zTemp00
    bne MaskVramWords0AA0_at0
    jsr VecSelectVramWriteAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    and z:zTemp01
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and z:zTemp02
    sta a:VDC_DATA_HI
MaskVramWords0AA0_at0:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc MaskVramWords0AA0_at1
    ply
    rts

; @routine 0AA0:4BD HitCheckThenRunState0AA0
; @run 71:DCBD
; WHAT IT DOES: Goes to state 4 when hit twice or more, then runs the routine of the object's state (Stage 06's
; overlay).
; HOW: State4WhenHitTwice0AA0; a jump table by the state.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
HitCheckThenRunState0AA0States = $DCD2 ; the routine address per state
HitCheckThenRunState0AA0:
    jsr State4WhenHitTwice0AA0
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:HitCheckThenRunState0AA0States,y
    sta z:zTemp00
    lda a:HitCheckThenRunState0AA0States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0AA0:506 PlayListThenArm0AA0
; @run 71:DD06
; WHAT IT DOES: Plays the list at $DD22; when it ends, turns the hit box on, restarts the animation and steps the
; state on (Stage 06's overlay).
; HOW: StepObjectFrames, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PlayListThenArm0AA0:
    lda #$22
    sta z:zTemp10
    lda #$DD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne PlayListThenArm0AA0_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
PlayListThenArm0AA0_at0:
    rts

; @routine 0AA0:52D TurnTowardPlayerSpeedUp0AA0
; @run 71:DD2D
; WHAT IT DOES: Plays the list at $DD89, faces the player, turns its angle (OBJ_WORK_E) one step toward the player,
; speeds OBJ_WORK_F:G up by 4 a frame to $0180 and moves (Stage 06's overlay).
; HOW: AngleToPlayer, TurnAngleToward, a 16-bit add, SetVelocityFromAngle, ApplyVelocity.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
TurnTowardPlayerSpeedUp0AA0:
    lda #$89
    sta z:zTemp10
    lda #$DD
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jsr VecAngleToPlayer
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
    cmp #$01
    bcc TurnTowardPlayerSpeedUp0AA0_at0
    lda a:OBJ_WORK_G,x
    cmp #$80
    bcs TurnTowardPlayerSpeedUp0AA0_at1
TurnTowardPlayerSpeedUp0AA0_at0:
    lda a:OBJ_WORK_G,x
    clc
    adc #$04
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    adc #$00
    sta a:OBJ_WORK_F,x
TurnTowardPlayerSpeedUp0AA0_at1:
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    rts

; @routine 0AA0:592 BackAwayFromPlayer0AA0
; @run 71:DD92
; WHAT IT DOES: Substate 0 aims away from the player at a speed of $0180 for $14 frames and steps the substate on;
; later moves that way and, when the count runs out, goes to state 3 with the speed 0 (Stage 06's overlay).
; HOW: AngleToPlayer plus $80, stores; SetVelocityFromAngle, ApplyVelocity, DEC.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
BackAwayFromPlayer0AA0:
    lda a:OBJ_SUBSTATE,x
    bne BackAwayFromPlayer0AA0_at0
    jsr VecAngleToPlayer
    clc
    adc #$80
    sta a:OBJ_WORK_E,x
    lda #$01
    sta a:OBJ_WORK_F,x
    lda #$80
    sta a:OBJ_WORK_G,x
    lda #$14
    sta a:OBJ_WORK_H,x
    inc a:OBJ_SUBSTATE,x
    rts
BackAwayFromPlayer0AA0_at0:
    lda a:OBJ_WORK_F,x
    sta z:zTemp01
    lda a:OBJ_WORK_G,x
    sta z:zTemp00
    lda a:OBJ_WORK_E,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    dec a:OBJ_WORK_H,x
    bne BackAwayFromPlayer0AA0_at1
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_WORK_F,x
    stz a:OBJ_WORK_G,x
BackAwayFromPlayer0AA0_at1:
    rts
