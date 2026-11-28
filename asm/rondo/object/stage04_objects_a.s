; @routine 07DA:235 BounceOffWallsHigh07DA
; @run 70:AA35
; WHAT IT DOES: Moves sideways; when the terrain ahead is solid at height 9 and height 2, turns back and moves again. Carry set when
; it turned.
; HOW: ApplyVelocityX, two ProbeTerrain on the side of the movement, the answers in TileHit.
; WAITS: no.
; Entry: X object, Y and C for the probe. Exit: C whether it turned.
BounceOffWallsHigh07DA:
    jsr VecApplyVelocityX
    lda #$09
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    tay
    lda a:TileHit+1,y
    and #$01
    beq BounceOffWallsHigh07DA_at0
    lda #$02
    jsr VecProbeTerrain
    lda a:OBJ_VX_HI,x
    rol a
    rol a
    and #$01
    eor #$01
    asl a
    tay
    lda a:TileHit,y
    ora a:TileHit+1,y
    and #$02
    bne BounceOffWallsHigh07DA_at0
    clc
    rts
BounceOffWallsHigh07DA_at0:
    jsr VecNegateVelocityX
    jsr VecApplyVelocityX
    sec
    rts

; @routine 07DA:3DD ByState07DAABDD
; @run 70:ABDD
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $ABEF.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DAABDDStates = $ABEF ; a handler per state
ByState07DAABDD:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DAABDDStates,y
    sta z:zTemp00
    lda a:ByState07DAABDDStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DA:3F7 LoadAndLockPlayer07DA
; @run 70:ABF7
; WHAT IT DOES: Loads the record and records the event; zField92 cleared, a wait of $40, and the player's piece place $10, start
; pose 2, swoop count 0 and attack flags 2.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadAndLockPlayer07DA:
    jsr VecLoadObjectRecord
    jsr VecRecordObjectEvent
    stz z:zField92
    lda #$40
    sta a:OBJ_PIECE_NEXT,x
    lda #$10
    sta a:OBJ_PIECE_PLACE
    lda #$02
    sta a:PlayerStartPose
    stz a:OBJ_SWOOP_SPEED_LO
    lda #$02
    sta a:HitAttackFlags
    rts

; @routine 07DA:417 StopSoundWhenTimeRuns07DA
; @run 70:AC17
; WHAT IT DOES: Keeps the player's piece place at $10 while time is slowed; once not, at $C0 stops the sound, plays $56, clears
; the attack flags and goes on.
; HOW: StopSound and SoundCommand.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StopSoundWhenTimeRuns07DA:
    lda #$10
    sta a:OBJ_PIECE_PLACE
    lda a:TimeSlowMode
    bne StopSoundWhenTimeRuns07DA_at0
    lda #$C0
    sta a:OBJ_PIECE_PLACE
    jsr VecStopSound
    lda #$56
    jsr VecSoundCommand
    stz a:HitAttackFlags
    inc a:OBJ_STATE,x
StopSoundWhenTimeRuns07DA_at0:
    rts

; @routine 07DA:435 FlickerThenShatter07DA
; @run 70:AC35
; WHAT IT DOES: Counts the wait down, flickering the water level from the table at $AC60; at zero draws tile script 3, throws the
; shards ($ACAF) and waits $80.
; HOW: DrawTileScript and a call.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlickerThenShatter07DALevels = $AC60 ; water level words
FlickerThenShatter07DA:
    dec a:OBJ_PIECE_NEXT,x
    bne FlickerThenShatter07DA_at0
    lda #$03
    cly
    jsr VecDrawTileScript
    jsr ThrowShards07DA
    lda #$80
    sta a:OBJ_PIECE_NEXT,x
    inc a:OBJ_STATE,x
    rts
FlickerThenShatter07DA_at0:
    lda a:OBJ_PIECE_NEXT,x
    and #$07
    asl a
    tay
    lda a:FlickerThenShatter07DALevels,y
    sta a:RasterWaterWanted
    lda a:FlickerThenShatter07DALevels+1,y
    sta a:LevelYHi
    rts

; @routine 07DA:470 FlickerThenDrop07DA
; @run 70:AC70
; WHAT IT DOES: Counts the wait down, flickering the water level; at zero clears the water level and the time slowing, clears slot
; $4B, starts a kind $AA object here and clears this one.
; HOW: ClearObjectInY, StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlickerThenDrop07DALevels = $AC9F ; water level words
FlickerThenDrop07DA:
    dec a:OBJ_PIECE_NEXT,x
    beq FlickerThenDrop07DA_at0
    lda a:OBJ_PIECE_NEXT,x
    and #$07
    asl a
    tay
    lda a:FlickerThenDrop07DALevels,y
    sta a:RasterWaterWanted
    lda a:FlickerThenDrop07DALevels+1,y
    sta a:LevelYHi
    rts
FlickerThenDrop07DA_at0:
    stz a:RasterWaterWanted
    stz a:LevelYHi
    stz a:TimeSlowMode
    ldy #$4B
    jsr VecClearObjectInY
    lda #$AA
    jsr VecStartObjectAtPlace
    jmp VecClearObject

; @routine 07DA:4AF ThrowShards07DA
; @run 70:ACAF
; WHAT IT DOES: Starts a kind $3F shard in each of up to $18 free slots from $20, its place, speeds and counts from the table at
; $ACFE, eight bytes each.
; HOW: FindFreeSlotFrom20; the row in zBurstCount, X kept in zClimbStepBefore.
; WAITS: no.
; Entry: X object. Exit: as the last search, compare or load left them.
ThrowShards07DAShards = $ACFE ; $18 rows: count, X, Y, VX high and low, VY high and low, wait
ThrowShards07DA:
    cly
ThrowShards07DA_at0:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom20
    bcc ThrowShards07DA_at1
    lda #$3F
    sta a:OBJ_KIND,y
    stx z:zClimbStepBefore
    lda z:zBurstCount
    asl a
    asl a
    asl a
    tax
    lda a:ThrowShards07DAShards,x
    sta a:OBJ_PIECE_NEXT,y
    lda a:ThrowShards07DAShards+1,x
    sta a:OBJ_X_LO,y
    lda a:ThrowShards07DAShards+2,x
    sta a:OBJ_Y_LO,y
    lda a:ThrowShards07DAShards+3,x
    sta a:OBJ_VX_HI,y
    lda a:ThrowShards07DAShards+4,x
    sta a:OBJ_VX_LO,y
    lda a:ThrowShards07DAShards+5,x
    sta a:OBJ_VY_HI,y
    lda a:ThrowShards07DAShards+6,x
    sta a:OBJ_VY_LO,y
    lda a:ThrowShards07DAShards+7,x
    sta a:OBJ_PIECE_PLACE,y
    ldx z:zClimbStepBefore
    ldy z:zBurstCount
    iny
    cpy #$18
    bcc ThrowShards07DA_at0
ThrowShards07DA_at1:
    rts

; @routine 07DA:5BE FallAfterWaitList07DA
; @run 70:ADBE
; WHAT IT DOES: State 0 loads the record; then waits out OBJ_PIECE_PLACE, then steps the list OBJ_PIECE_NEXT picks and falls.
; HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallAfterWaitList07DALists = $ADE7 ; list addresses
FallAfterWaitList07DA:
    lda a:OBJ_STATE,x
    bne FallAfterWaitList07DA_at0
    jmp VecLoadObjectRecord
FallAfterWaitList07DA_at0:
    lda a:OBJ_PIECE_PLACE,x
    beq FallAfterWaitList07DA_at1
    dec a:OBJ_PIECE_PLACE,x
    rts
FallAfterWaitList07DA_at1:
    lda a:OBJ_PIECE_NEXT,x
    asl a
    tay
    lda a:FallAfterWaitList07DALists,y
    sta z:zTemp10
    lda a:FallAfterWaitList07DALists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 07DA:649 FloatingLanterns07DA
; @run 70:AE49
; WHAT IT DOES: State 0 sets it from the table by OBJ_START_ARG: speeds, entry place and, on later sections, the flip. Then it
; flickers every fourth frame by its number, slows by the table, moves and returns to its place when low.
; HOW: Eight-byte rows of speeds; a place word per number.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FloatingLanterns07DAPlaces = $AF05 ; X and Y by number
FloatingLanterns07DASpeeds = $AF0D ; eight bytes by number: VX word, VY word, VY limit, VY step, VX step
FloatingLanterns07DA:
    lda a:OBJ_STATE,x
    bne FloatingLanterns07DA_at1
    jsr VecLoadObjectRecord
    jsr VecRecordObjectEvent
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    tay
    sta a:OBJ_PIECE_NEXT,x
    lda a:FloatingLanterns07DASpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:FloatingLanterns07DASpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:FloatingLanterns07DASpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:FloatingLanterns07DASpeeds+2,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_X_HI,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:FloatingLanterns07DAPlaces,y
    sta a:OBJ_X_LO,x
    sta a:OBJ_PIECE_PLACE,x
    lda a:FloatingLanterns07DAPlaces+1,y
    sta a:OBJ_Y_LO,x
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda z:zField83
    beq FloatingLanterns07DA_at0
    lda #$80
    sta a:OBJ_SPRITE_FLIP,x
FloatingLanterns07DA_at0:
    rts
FloatingLanterns07DA_at1:
    lda #$29
    sta a:OBJ_FRAME,x
    lda z:zWaveCounter
    and #$03
    cmp a:OBJ_START_ARG,x
    bne FloatingLanterns07DA_at2
    stz a:OBJ_FRAME,x
FloatingLanterns07DA_at2:
    ldy a:OBJ_PIECE_NEXT,x
    lda a:OBJ_VY_HI,x
    cmp a:FloatingLanterns07DASpeeds+6,y
    bcs FloatingLanterns07DA_at3
    lda a:OBJ_VY_LO,x
    clc
    adc a:FloatingLanterns07DASpeeds+4,y
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    lda a:OBJ_VX_LO,x
    sec
    sbc a:FloatingLanterns07DASpeeds+5,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    sbc #$00
    sta a:OBJ_VX_HI,x
FloatingLanterns07DA_at3:
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    cmp #$01
    bne FloatingLanterns07DA_at4
    lda a:OBJ_Y_LO,x
    cmp #$20
    bcc FloatingLanterns07DA_at4
    stz a:OBJ_X_HI,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda a:OBJ_PIECE_PLACE,x
    sta a:OBJ_X_LO,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    sta a:OBJ_Y_LO,x
FloatingLanterns07DA_at4:
    rts

; @routine 07DA:72D PaletteSwapEveryThird07DA
; @run 70:AF2D
; WHAT IT DOES: State 0 sets a count of 3. Then each frame starts script $80 or $81 in slot 4 and swaps which every third frame.
; HOW: StartScriptSlotKind4.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PaletteSwapEveryThird07DA:
    lda a:OBJ_STATE,x
    bne PaletteSwapEveryThird07DA_at0
    jsr VecLoadObjectRecord
    lda #$03
    sta a:OBJ_PIECE_NEXT,x
    rts
PaletteSwapEveryThird07DA_at0:
    phx
    lda a:OBJ_PIECE_PLACE,x
    and #$01
    clc
    adc #$80
    ldx #$04
    jsr VecStartScriptSlotKind4
    plx
    dec a:OBJ_PIECE_NEXT,x
    bne PaletteSwapEveryThird07DA_at1
    lda #$03
    sta a:OBJ_PIECE_NEXT,x
    lda a:OBJ_PIECE_PLACE,x
    eor #$01
    sta a:OBJ_PIECE_PLACE,x
PaletteSwapEveryThird07DA_at1:
    rts

; @routine 07DA:75D ByState07DAAF5D
; @run 70:AF5D
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $AF6F.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DAAF5DStates = $AF6F ; a handler per state
ByState07DAAF5D:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DAAF5DStates,y
    sta z:zTemp00
    lda a:ByState07DAAF5DStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DA:775 LoadShard07DA
; @run 70:AF75
; WHAT IT DOES: Loads the record with the frame and speeds OBJ_START_ARG picks and a wait of $28.
; HOW: Tables by argument.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadShard07DAFrames = $AFD7 ; frame by argument
LoadShard07DASpeeds = $AFDB ; VX word, VY word by argument
LoadShard07DA:
    jsr VecLoadObjectRecord
    ldy a:OBJ_START_ARG,x
    lda a:LoadShard07DAFrames,y
    sta a:OBJ_FRAME,x
    lda #$28
    sta a:OBJ_PIECE_NEXT,x
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    tay
    lda a:LoadShard07DASpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LoadShard07DASpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:LoadShard07DASpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:LoadShard07DASpeeds+2,y
    sta a:OBJ_VY_LO,x
    rts

; @routine 07DA:7A5 CountOutWhenFlagged07DA
; @run 70:AFA5
; WHAT IT DOES: Once zField92 is clear, counts the wait out, records the event and goes on, with sound $4E for number 0.
; HOW: RecordObjectEvent.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountOutWhenFlagged07DA:
    lda z:zField92
    bne CountOutWhenFlagged07DA_at0
    dec a:OBJ_PIECE_NEXT,x
    bne CountOutWhenFlagged07DA_at0
    jsr VecRecordObjectEvent
    inc a:OBJ_STATE,x
    lda a:OBJ_START_ARG,x
    bne CountOutWhenFlagged07DA_at0
    lda #$4E
    jsr VecSoundCommand
CountOutWhenFlagged07DA_at0:
    rts

; @routine 07DA:7BF FallWithListByArgument07DA
; @run 70:AFBF
; WHAT IT DOES: Steps the list OBJ_START_ARG picks from the table at $AFEB and falls.
; HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallWithListByArgument07DALists = $AFEB ; list addresses
FallWithListByArgument07DA:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:FallWithListByArgument07DALists,y
    sta z:zTemp10
    lda a:FallWithListByArgument07DALists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 07DB:010 TrailedOrbit07DB
; @run 70:B010
; WHAT IT DOES: State 0: loads the record, fills the angle history with $40 and starts seven kind $C8 links in the slots below,
; numbered. Then follows the object in the slot below, orbiting it: a hit turns the swing, which eases back.
; HOW: The angle history at $B1D0, shifted each frame; a 16-bit swing.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
TrailedOrbit07DAKicks = $B17E ; swing kick by side
TrailedOrbit07DAHistory = $B1D0 ; 32 angles, newest first
TrailedOrbit07DB:
    lda a:OBJ_STATE,x
    bne TrailedOrbit07DB_at2
    jsr VecLoadObjectRecord
    lda #$40
    sta a:OBJ_PIECE_NEXT,x
    cly
TrailedOrbit07DB_at0:
    lda #$40
    sta a:TrailedOrbit07DAHistory,y
    iny
    cpy #$20
    bcc TrailedOrbit07DB_at0
    txa
    sec
    sbc #$07
    tay
    lda #$07
    sta z:zTemp00
    stz z:zTemp01
TrailedOrbit07DB_at1:
    jsr VecClearObjectInY
    lda #$C8
    jsr VecStartObjectAtPlace
    lda z:zTemp01
    sta a:OBJ_PIECE_PLACE,y
    iny
    inc z:zTemp01
    dec z:zTemp00
    bne TrailedOrbit07DB_at1
    rts
TrailedOrbit07DB_at2:
    txa
    tay
    dey
    jsr VecCopyObjectPlaceBack
    lda #$08
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    stz z:zTemp00
    lda #$08
    sta z:zTemp01
    lda a:OBJ_PIECE_NEXT,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
    lda a:OBJ_HIT_TAKEN,x
    beq TrailedOrbit07DB_at3
    lda #$12
    jsr VecSoundCommand
    lda #$20
    sta a:OBJ_INVULN_TIMER,x
    jsr VecSideOfPlayer
    and #$01
    tay
    lda a:OBJ_SWOOP_SPEED_LO,x
    clc
    adc a:TrailedOrbit07DAKicks,y
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda #$0A
    sta a:OBJ_RAIN_COUNT,x
TrailedOrbit07DB_at3:
    jsr ShiftAngleHistory07DB
    jsr DampSwingAtTurn07DB
    jsr DampSwingAtBottom07DB
    lda a:OBJ_SWOOP_SPEED_LO,x
    sta a:OBJ_FALL_LO,x
    lda a:OBJ_PIECE_PLACE,x
    clc
    adc a:OBJ_SWOOP_SPEED_HI,x
    sta a:OBJ_PIECE_PLACE,x
    lda a:OBJ_PIECE_NEXT,x
    adc a:OBJ_SWOOP_SPEED_LO,x
    sta a:OBJ_PIECE_NEXT,x
    jsr ClampSwingAngle07DB
    lda a:OBJ_PIECE_NEXT,x
    cmp #$40
    bcc TrailedOrbit07DB_at4
    lda a:OBJ_SWOOP_SPEED_HI,x
    sec
    sbc a:OBJ_RAIN_COUNT,x
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    sbc #$00
    sta a:OBJ_SWOOP_SPEED_LO,x
    rts
TrailedOrbit07DB_at4:
    lda a:OBJ_SWOOP_SPEED_HI,x
    clc
    adc a:OBJ_RAIN_COUNT,x
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    adc #$00
    sta a:OBJ_SWOOP_SPEED_LO,x
    rts

; @routine 07DB:0DB ShiftAngleHistory07DB
; @run 70:B0DB
; WHAT IT DOES: Shifts the 32-angle history along and puts this object's angle first.
; HOW: A copy from the end.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ShiftAngleHistory07DABefore = $B1CF ; the byte before the history
ShiftAngleHistory07DAHistory = $B1D0 ; 32 angles, newest first
ShiftAngleHistory07DB:
    ldy #$1F
ShiftAngleHistory07DB_at0:
    lda a:ShiftAngleHistory07DABefore,y
    sta a:ShiftAngleHistory07DAHistory,y
    dey
    bne ShiftAngleHistory07DB_at0
    lda a:OBJ_PIECE_NEXT,x
    sta a:ShiftAngleHistory07DAHistory
    rts

; @routine 07DB:0ED DampSwingAtTurn07DB
; @run 70:B0ED
; WHAT IT DOES: When the swing speed changes sign, takes one from the easing, down to 1.
; HOW: An EOR of the old and new high bytes.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DampSwingAtTurn07DB:
    lda a:OBJ_SWOOP_SPEED_LO,x
    eor a:OBJ_FALL_LO,x
    bpl DampSwingAtTurn07DB_at0
    lda a:OBJ_RAIN_COUNT,x
    cmp #$01
    beq DampSwingAtTurn07DB_at0
    dec a:OBJ_RAIN_COUNT,x
DampSwingAtTurn07DB_at0:
    rts

; @routine 07DB:100 DampSwingAtBottom07DB
; @run 70:B100
; WHAT IT DOES: Each time the angle passes the bottom ($40), takes a quarter off the swing speed, stopping the easing at zero.
; HOW: A signed 16-bit divide by four; OBJ_WORK_J the half passed.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
DampSwingAtBottom07DB:
    lda a:OBJ_WORK_J,x
    beq DampSwingAtBottom07DB_at1
    lda a:OBJ_PIECE_NEXT,x
    cmp #$40
    bcc DampSwingAtBottom07DB_at2
DampSwingAtBottom07DB_at0:
    lda a:OBJ_WORK_J,x
    eor #$01
    sta a:OBJ_WORK_J,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    sta z:zTemp01
    lda a:OBJ_SWOOP_SPEED_HI,x
    sta z:zTemp00
    lda z:zTemp01
    asl a
    ror z:zTemp01
    ror z:zTemp00
    lda z:zTemp01
    asl a
    ror z:zTemp01
    ror z:zTemp00
    lda a:OBJ_SWOOP_SPEED_HI,x
    sec
    sbc z:zTemp00
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    sbc z:zTemp01
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:OBJ_SWOOP_SPEED_LO,x
    ora a:OBJ_SWOOP_SPEED_HI,x
    bne DampSwingAtBottom07DB_at2
    sta a:OBJ_RAIN_COUNT,x
    rts
DampSwingAtBottom07DB_at1:
    lda a:OBJ_PIECE_NEXT,x
    cmp #$40
    bcc DampSwingAtBottom07DB_at0
DampSwingAtBottom07DB_at2:
    rts

; @routine 07DB:151 ClampSwingAngle07DB
; @run 70:B151
; WHAT IT DOES: Keeps the angle between $C0 and $80 round the top: past $80 it stops there, and below 0 it stops at 0.
; HOW: Compares; a stop clears the swing speed.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ClampSwingAngle07DB:
    lda a:OBJ_PIECE_NEXT,x
    cmp #$C0
    bcs ClampSwingAngle07DB_at1
    cmp #$40
    bcc ClampSwingAngle07DB_at1
    lda a:OBJ_PIECE_NEXT,x
    cmp #$80
    bcc ClampSwingAngle07DB_at0
    lda #$80
    sta a:OBJ_PIECE_NEXT,x
    stz a:OBJ_SWOOP_SPEED_LO,x
    stz a:OBJ_SWOOP_SPEED_HI,x
ClampSwingAngle07DB_at0:
    rts
ClampSwingAngle07DB_at1:
    lda a:OBJ_PIECE_NEXT,x
    bpl ClampSwingAngle07DB_at0
    stz a:OBJ_PIECE_NEXT,x
    stz a:OBJ_SWOOP_SPEED_LO,x
    stz a:OBJ_SWOOP_SPEED_HI,x
    rts

; @routine 07DB:184 LinkLoadThenFollow07DB
; @run 70:B184
; @stop B199
; WHAT IT DOES: State 0 loads the record and drops by the link's number; later follows the chain ($B1A0).
; HOW: ApplyVelocityY.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LinkLoadThenFollow07DADrops = $B199 ; VY high by link
LinkLoadThenFollow07DB:
    lda a:OBJ_STATE,x
    bne LinkFollowChain07DB
    jsr VecLoadObjectRecord
    ldy a:OBJ_PIECE_PLACE,x
    lda a:LinkLoadThenFollow07DADrops,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    rts

; @routine 07DB:1A0 LinkFollowChain07DB
; @run 70:B1A0
; WHAT IT DOES: Takes the angle from the history by the link's number and places itself at that angle from the link below.
; HOW: CopyObjectPlaceBack and SetVelocityFromAngle with magnitude $0800.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LinkFollowChain07DADelays = $B1C8 ; history position by link
LinkFollowChain07DAHistory = $B1D0 ; 32 angles, newest first
LinkFollowChain07DB:
    ldy a:OBJ_PIECE_PLACE,x
    lda a:LinkFollowChain07DADelays,y
    tay
    lda a:LinkFollowChain07DAHistory,y
    sta a:OBJ_PIECE_NEXT,x
    txa
    tay
    dey
    lda a:OBJ_PIECE_PLACE,x
    beq LinkFollowChain07DB_at0
    jsr VecCopyObjectPlaceBack
    stz z:zTemp00
    lda #$08
    sta z:zTemp01
    lda a:OBJ_PIECE_NEXT,x
    jsr VecSetVelocityFromAngle
    jsr VecApplyVelocity
LinkFollowChain07DB_at0:
    rts

; @routine 07DB:1F0 ByState07DAB1F0
; @run 70:B1F0
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $B202.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DAB1F0States = $B202 ; a handler per state
ByState07DAB1F0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DAB1F0States,y
    sta z:zTemp00
    lda a:ByState07DAB1F0States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DB:20A BobOnList07DAB234
; @run 70:B20A
; WHAT IT DOES: Steps the list at $B234, bobbing on the cosine of a stepping angle at half speed.
; HOW: ScaleByCosine, ApplyVelocityY.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BobOnList07DAList = $B234 ; a frame list
BobOnList07DAB234:
    lda #$34
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$80
    sta z:zTemp00
    lda #$00
    sta z:zTemp01
    lda a:OBJ_PIECE_NEXT,x
    jsr VecScaleByCosine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VY_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    inc a:OBJ_PIECE_NEXT,x
    rts

; @routine 07DB:239 SinkToLineBC07DB
; @run 70:B239
; WHAT IT DOES: Steps the list at $B234 and sinks at $0140 to Y $BC, then goes on.
; HOW: ApplyVelocityY.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SinkToLineBC07DAList = $B234 ; a frame list
SinkToLineBC07DB:
    lda #$34
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda #$01
    sta a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    jsr VecApplyVelocityY
    lda a:OBJ_Y_LO,x
    cmp #$BC
    bcc SinkToLineBC07DB_at0
    lda #$BC
    sta a:OBJ_Y_LO,x
    inc a:OBJ_STATE,x
SinkToLineBC07DB_at0:
    rts

; @routine 07DB:261 ByState07DAB261
; @run 70:B261
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $B273.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DAB261States = $B273 ; a handler per state
ByState07DAB261:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DAB261States,y
    sta z:zTemp00
    lda a:ByState07DAB261States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DB:27B WaitForPlayerState07DB
; @run 70:B27B
; WHAT IT DOES: Steps the list at $B297; once the player is in substate 1, restarts the animation and goes on.
; HOW: The player's substate.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
WaitForPlayerState07DAList = $B297 ; a frame list
WaitForPlayerState07DB:
    lda #$97
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_SUBSTATE
    cmp #$01
    bne WaitForPlayerState07DB_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_STATE,x
WaitForPlayerState07DB_at0:
    rts

; @routine 07DB:2A2 FadeCdAtListEnd07DB
; @run 70:B2A2
; WHAT IT DOES: Steps the list at $B2CC; at its end fades the CD out ($0C) and goes on.
; HOW: cd_fade.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FadeCdAtListEnd07DAList = $B2CC ; a frame list
FadeCdAtListEnd07DB:
    lda #$CC
    sta z:zTemp10
    lda #$B2
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne EndStageAfterCount07DB_at0
    phx
    lda #$0C
    jsr BiosCdFade
    plx
    inc a:OBJ_STATE,x
    rts

; @routine 07DB:2BA EndStageAfterCount07DB
; @run 70:B2BA
; WHAT IT DOES: Counts the wait down by two; at zero raises mode event bit 1, sets zFieldA3 to 9 and clears the object.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EndStageAfterCount07DB:
    dec a:OBJ_PIECE_NEXT,x
    dec a:OBJ_PIECE_NEXT,x
    bne EndStageAfterCount07DB_at0
    smb1 z:zModeEvents
    lda #$09
    sta z:zFieldA3
    jmp VecClearObject
EndStageAfterCount07DB_at0:
    rts

; @routine 07DB:2DB ExpandListsForPart3_07DB
; @run 70:B2DB
; WHAT IT DOES: Expands the stage lists of part 3 and goes back to part 2, then clears the object.
; HOW: ExpandStageListsE with zField82 set around it.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ExpandListsForPart3_07DB:
    lda #$03
    sta z:zField82
    phx
    jsr ExpandStageListsE
    plx
    lda #$02
    sta z:zField82
    jmp VecClearObject

; @routine 07DB:2EB CountCheckThenByState07DB
; @run 70:B2EB
; WHAT IT DOES: Counts OBJ_FALL_LO, bursts when hit and flagged ($B311) and goes to the state's handler.
; HOW: A table of handlers at $B303.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountCheckThenByState07DAStates = $B303 ; a handler per state
CountCheckThenByState07DB:
    inc a:OBJ_FALL_LO,x
    jsr BurstWhenFlaggedHit
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CountCheckThenByState07DAStates,y
    sta z:zTemp00
    lda a:CountCheckThenByState07DAStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)
