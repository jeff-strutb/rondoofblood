; @routine 07DC:65E ByState07DABE5E
; @run 70:BE5E
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $BE70.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DABE5EStates = $BE70 ; a handler per state
ByState07DABE5E:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DABE5EStates,y
    sta z:zTemp00
    lda a:ByState07DABE5EStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DC:678 LaunchByKindOfShot07DB
; @run 70:BE78
; WHAT IT DOES: Loads the record, starts it by its number ($BEDD) with a life of $48; numbers other than 2 fly level at the
; speed the table gives, number 2 hops at a speed by the player's distance. Facing left turns it.
; HOW: Speed tables.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LaunchByKindOfShot07DASpeeds = $BEC9 ; VX words by number
LaunchByKindOfShot07DAHops = $BECD ; VX words by distance / 32
LaunchByKindOfShot07DB:
    jsr VecLoadObjectRecord
    jsr StartShotByNumber07DB
    lda #$48
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:OBJ_PIECE_PLACE,x
    cmp #$02
    beq LaunchByKindOfShot07DB_at2
    asl a
    tay
    lda a:LaunchByKindOfShot07DASpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LaunchByKindOfShot07DASpeeds,y
    sta a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
LaunchByKindOfShot07DB_at0:
    lda a:OBJ_FACING_LEFT,x
    beq LaunchByKindOfShot07DB_at1
    jsr VecNegateVelocityX
    lda #$01
    sta a:OBJ_PIECE_NEXT,x
LaunchByKindOfShot07DB_at1:
    rts
LaunchByKindOfShot07DB_at2:
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    asl a
    tay
    lda a:LaunchByKindOfShot07DAHops+1,y
    sta a:OBJ_VX_HI,x
    lda a:LaunchByKindOfShot07DAHops,y
    sta a:OBJ_VX_LO,x
    lda #$FB
    sta a:OBJ_VY_HI,x
    bra LaunchByKindOfShot07DB_at0

; @routine 07DC:6DD StartShotByNumber07DB
; @run 70:BEDD
; WHAT IT DOES: Takes the speed high bytes the table gives for OBJ_PIECE_PLACE, turned when facing left, and moves one step.
; HOW: A tail jump to ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartShotByNumber07DASpeeds = $BEF9 ; VX high and VY high by number
StartShotByNumber07DB:
    lda a:OBJ_PIECE_PLACE,x
    asl a
    tay
    lda a:StartShotByNumber07DASpeeds,y
    sta a:OBJ_VX_HI,x
    lda a:StartShotByNumber07DASpeeds+1,y
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT,x
    beq StartShotByNumber07DB_at0
    jsr VecNegateVelocityX
StartShotByNumber07DB_at0:
    jmp VecApplyVelocity

; @routine 07DC:6FF FlyThenNextOrFall07DB
; @run 70:BEFF
; WHAT IT DOES: Steps the list at $BF2F and moves; number 2 falls by $1C, others count their life out to the next state.
; HOW: StepObjectFrames and ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyThenNextOrFall07DAList = $BF2F ; a frame list
FlyThenNextOrFall07DB:
    lda #$BF
    sta z:zTemp11
    lda #$2F
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_PIECE_PLACE,x
    cmp #$02
    bne FlyThenNextOrFall07DB_at0
    lda a:OBJ_VY_LO,x
    clc
    adc #$1C
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    rts
FlyThenNextOrFall07DB_at0:
    dec a:OBJ_SWOOP_SPEED_LO,x
    bne FlyThenNextOrFall07DB_at1
    inc a:OBJ_STATE,x
FlyThenNextOrFall07DB_at1:
    rts

; @routine 07DC:740 SteerBackUntilSpeed07DB
; @run 70:BF40
; WHAT IT DOES: Steps the list at $BF2F and moves, accelerating by the table until the speed reaches its limit, then goes on.
; HOW: A 16-bit add and a compare of both bytes.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SteerBackUntilSpeed07DAList = $BF2F ; a frame list
SteerBackUntilSpeed07DASteer = $BF78 ; step word and limit word by direction
SteerBackUntilSpeed07DB:
    lda #$BF
    sta z:zTemp11
    lda #$2F
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr VecApplyVelocity
    lda a:OBJ_PIECE_NEXT,x
    asl a
    asl a
    tay
    lda a:OBJ_VX_LO,x
    clc
    adc a:SteerBackUntilSpeed07DASteer,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    adc a:SteerBackUntilSpeed07DASteer+1,y
    sta a:OBJ_VX_HI,x
    cmp a:SteerBackUntilSpeed07DASteer+3,y
    bne SteerBackUntilSpeed07DB_at0
    lda a:OBJ_VX_LO,x
    cmp a:SteerBackUntilSpeed07DASteer+2,y
    bne SteerBackUntilSpeed07DB_at0
    inc a:OBJ_STATE,x
SteerBackUntilSpeed07DB_at0:
    rts

; @routine 07DC:780 StepAndMove07DBF80
; @run 70:BF80
; WHAT IT DOES: Steps the list at $BF2F and moves.
; HOW: A tail jump to ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StepAndMove07DBF80List = $BF2F ; a frame list
StepAndMove07DBF80:
    lda #$BF
    sta z:zTemp11
    lda #$2F
    sta z:zTemp10
    jsr VecStepObjectFrames
    jmp VecApplyVelocity

; @routine 07DC:78E ByState07DABF8E
; @run 70:BF8E
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $BFA0.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState07DABF8EStates = $BFA0 ; a handler per state
ByState07DABF8E:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState07DABF8EStates,y
    sta z:zTemp00
    lda a:ByState07DABF8EStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DC:7A8 FollowBodyUntilHurt07DB
; @run 70:BFA8
; WHAT IT DOES: Takes the next slot's attribute; once its health is under 8, clears its pace bits, this loop flag bit 1 and
; attribute, and goes on.
; HOW: Reads of slot X+1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FollowBodyUntilHurt07DB:
    lda a:OBJ_SPRITE_ATTR+1,x
    sta a:OBJ_SPRITE_ATTR,x
    lda a:OBJ_HEALTH+1,x
    cmp #$08
    bcs FollowBodyUntilHurt07DB_at0
    lda a:OBJ_PACE+1,x
    and #$FC
    sta a:OBJ_PACE+1,x
    lda a:OBJ_LOOP_FLAGS,x
    and #$FD
    sta a:OBJ_LOOP_FLAGS,x
    stz a:OBJ_SPRITE_ATTR,x
    inc a:OBJ_STATE,x
FollowBodyUntilHurt07DB_at0:
    rts

; @routine 07DC:7FA LeapByBodyFacing07DB
; @run 70:BFFA
; WHAT IT DOES: Rises at -2 with the X speed the next slot's facing picks, and goes on.
; HOW: A table.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LeapByBodyFacing07DASpeeds = $C00C ; VX high by facing
LeapByBodyFacing07DB:
    lda #$FE
    sta a:OBJ_VY_HI,x
    ldy a:OBJ_FACING_LEFT+1,x
    lda a:LeapByBodyFacing07DASpeeds,y
    sta a:OBJ_VX_HI,x
    inc a:OBJ_STATE,x
    rts

; @routine 07DD:014 HitsThenByState07DC014
; @run 71:C014
; WHAT IT DOES: Takes the hits ($C031) and goes to the state's handler.
; HOW: A table of handlers at $C029.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HitsThenByState07DC014States = $C029 ; a handler per state
HitsThenByState07DC014:
    jsr BossPartsWhenHit07DD
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:HitsThenByState07DC014States,y
    sta z:zTemp00
    lda a:HitsThenByState07DC014States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DD:031 BossPartsWhenHit07DD
; @run 71:C031
; WHAT IT DOES: Under state 2: on section 2 at map X below $D0, or on status bit 7, records the event with sound $8D, drops the
; three hit boxes, stops falling and goes to state 2 with a wait of $50. Else status bit 5 clears the water level
; and puts this and the next two slots in states 3, 4 and 4.
; HOW: A 16-bit map position and status tests.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BossPartsWhenHit07DD:
    lda a:OBJ_STATE,x
    cmp #$02
    bcs NothingState07DD
    lda z:zField83
    cmp #$02
    bne BossPartsWhenHit07DD_at0
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:ScrollXHiScreen
    bne BossPartsWhenHit07DD_at0
    lda z:zTemp00
    cmp #$D0
    bcs BossPartsWhenHit07DD_at2
BossPartsWhenHit07DD_at0:
    lda a:OBJ_HIT_STATUS,x
    and #$20
    beq BossPartsWhenHit07DD_at1
    stz a:LevelYHi
    stz a:RasterWaterWanted
    lda #$03
    sta a:OBJ_STATE,x
    lda #$04
    sta a:OBJ_STATE+1,x
    lda #$04
    sta a:OBJ_STATE+2,x
    rts
BossPartsWhenHit07DD_at1:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq NothingState07DD
BossPartsWhenHit07DD_at2:
    jsr VecRecordObjectEvent
    lda #$8D
    jsr VecSoundCommand
    lda #$01
    sta z:zField92
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_HITBOX_ON+1,x
    stz a:OBJ_HITBOX_ON+2,x
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda a:OBJ_LOOP_FLAGS,x
    and #$FD
    sta a:OBJ_LOOP_FLAGS,x
    lda #$50
    sta a:OBJ_PIECE_PLACE,x

; @routine 07DD:0A6 NothingState07DD
; @run 71:C0A6
; WHAT IT DOES: Does nothing: the handler of the state the table at $C029 gives no work.
; HOW: An RTS.
; WAITS: no.
; Entry: none. Exit: unchanged.
NothingState07DD:
    rts

; @routine 07DD:0A7 BossSetUp07DD
; @run 71:C0A7
; WHAT IT DOES: Once the boss is beaten clears the object. Else loads the record at the X the section gives and starts a kind $AB
; and a kind $AC object in the next two slots.
; HOW: ClearObjectInY and StartObjectAtPlace.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BossSetUp07DAPlaces = $C0D7 ; X word by section from 1
BossSetUp07DD:
    lda z:zField92
    beq BossSetUp07DD_at0
    jmp VecClearObject
BossSetUp07DD_at0:
    jsr VecLoadObjectRecord
    lda z:zField83
    dec a
    asl a
    tay
    lda a:BossSetUp07DAPlaces+1,y
    sta a:OBJ_X_HI,x
    lda a:BossSetUp07DAPlaces,y
    sta a:OBJ_X_LO,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$AB
    jsr VecStartObjectAtPlace
    iny
    jsr VecClearObjectInY
    lda #$AC
    jsr VecStartObjectAtPlace
    rts

; @routine 07DD:0DB CircleBodyTwoBelow07DD
; @run 71:C0DB
; WHAT IT DOES: Steps the list at $C11A and places itself on a circle around the object two slots on, its height halved.
; HOW: CopyObjectPlaceBack, ApplyVelocityX $34, SetVelocityFromAngle magnitude $0600, VX halved.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CircleBodyTwoBelow07DAList = $C11A ; a frame list
CircleBodyTwoBelow07DD:
    lda #$C1
    sta z:zTemp11
    lda #$1A
    sta z:zTemp10
    jsr VecStepObjectFrames
    txa
    inc a
    inc a
    tay
    jsr VecCopyObjectPlaceBack
    lda #$34
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda #$06
    sta z:zTemp01
    lda #$00
    sta z:zTemp00
    lda a:OBJ_PIECE_NEXT,x
    jsr VecSetVelocityFromAngle
    lda a:OBJ_VX_HI,x
    asl a
    ror a:OBJ_VX_HI,x
    ror a:OBJ_VX_LO,x
    jsr VecApplyVelocity
    lda a:OBJ_PIECE_NEXT,x
    clc
    adc #$02
    sta a:OBJ_PIECE_NEXT,x
    rts

; @routine 07DD:123 CountSparksThenFall07DD
; @run 71:C123
; WHAT IT DOES: Counts out sparks ($C12C), then falls.
; HOW: ApplyGravityUnlimited and a tail jump to ApplyVelocityY.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CountSparksThenFall07DD:
    jsr SparksAtCounts07DD
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocityY

; @routine 07DD:12C SparksAtCounts07DD
; @run 71:C12C
; WHAT IT DOES: Counts OBJ_PIECE_PLACE down; at one of sixteen counts starts a kind $9D spark with sound $4E and that count's X
; speed, and moves it.
; HOW: A search of the table; X kept in zClimbStepBefore, the row in zBurstCount.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SparksAtCounts07DATimes = $C166 ; sixteen counts
SparksAtCounts07DASpeeds = $C176 ; VX high by count
SparksAtCounts07DD:
    lda a:OBJ_PIECE_PLACE,x
    beq SparksAtCounts07DD_at2
    dec a:OBJ_PIECE_PLACE,x
    cly
SparksAtCounts07DD_at0:
    lda a:SparksAtCounts07DATimes,y
    cmp a:OBJ_PIECE_PLACE,x
    beq SparksAtCounts07DD_at1
    iny
    cpy #$10
    bcc SparksAtCounts07DD_at0
    rts
SparksAtCounts07DD_at1:
    stx z:zClimbStepBefore
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc SparksAtCounts07DD_at2
    lda #$9D
    jsr VecStartObjectAtPlace
    lda #$4E
    jsr VecSoundCommand
    ldx z:zBurstCount
    lda a:SparksAtCounts07DASpeeds,x
    sta a:OBJ_VX_HI,y
    sxy
    jsr VecApplyVelocity
    sxy
    ldx z:zClimbStepBefore
SparksAtCounts07DD_at2:
    rts

; @routine 07DD:186 CheckHeadThenByState07DD
; @run 71:C186
; WHAT IT DOES: Falls away when the part below is gone ($C1A5) and goes to the state's handler.
; HOW: A table of handlers at $C19B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
CheckHeadThenByState07DAStates = $C19B ; a handler per state
CheckHeadThenByState07DD:
    jsr FallWithoutHead07DD
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckHeadThenByState07DAStates,y
    sta z:zTemp00
    lda a:CheckHeadThenByState07DAStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 07DD:1A5 FallWithoutHead07DD
; @run 71:C1A5
; WHAT IT DOES: Once the slot below no longer holds kind $AA, goes to state 3 at rest (unless there) with pace bits 3-6 and loop
; flag bit 1 cleared and a wait of $50.
; HOW: The kind of slot X-1.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallWithoutHead07DD:
    lda a:OBJ_KIND-1,x
    cmp #$AA
    beq FallWithoutHead07DD_at0
    lda a:OBJ_STATE,x
    cmp #$03
    beq FallWithoutHead07DD_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda a:OBJ_PACE,x
    and #$87
    sta a:OBJ_PACE,x
    lda a:OBJ_LOOP_FLAGS,x
    and #$FD
    sta a:OBJ_LOOP_FLAGS,x
    lda #$50
    sta a:OBJ_PIECE_PLACE,x
FallWithoutHead07DD_at0:
    rts

; @routine 07DD:1D4 LoadWithWait20_07DD
; @run 71:C1D4
; WHAT IT DOES: Loads the record with a wait of $20.
; HOW: LoadObjectRecord.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadWithWait20_07DD:
    jsr VecLoadObjectRecord
    lda #$20
    sta a:OBJ_PIECE_PLACE,x
    rts

; @routine 07DD:1DD FlickerWaterThenNext07DD
; @run 71:C1DD
; WHAT IT DOES: Steps the list at $C21A, setting the water level by the player's distance and the count; at the count's end goes
; on with a wait of $20 and $80.
; HOW: A word table as in WaterLevelByDistance051B.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlickerWaterThenNext07DAList = $C21A ; a frame list
FlickerWaterThenNext07DALevels = $C21F ; water level words
FlickerWaterThenNext07DD:
    lda #$C2
    sta z:zTemp11
    lda #$1A
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_PIECE_PLACE,x
    and #$07
    asl a
    sta z:zTemp00
    jsr VecDistanceXToPlayer
    lsr a
    lsr a
    and #$F0
    clc
    adc z:zTemp00
    tay
    lda a:FlickerWaterThenNext07DALevels,y
    sta a:RasterWaterWanted
    lda a:FlickerWaterThenNext07DALevels+1,y
    sta a:LevelYHi
    dec a:OBJ_PIECE_PLACE,x
    bne FlickerWaterThenNext07DD_at0
    inc a:OBJ_STATE,x
    lda #$20
    sta a:OBJ_PIECE_PLACE,x
    lda #$80
    sta a:OBJ_PIECE_NEXT,x
FlickerWaterThenNext07DD_at0:
    rts
