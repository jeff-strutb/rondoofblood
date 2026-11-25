; @routine 0B4A:39F StartRingOfPieces
; @run 70:AB9F
; WHAT IT DOES: For the numbers 0 to $17 starts an object of type $28 in a free slot of $20 to $3F at the object's place and gives it the seven bytes of the eight byte row of the table at $ABE8: OBJ_34B8, X speed high, Y speed high, OBJ_3568, OBJ_35C0, OBJ_3618 and OBJ_3670; it stops when no slot is free.
; HOW: A loop of FindFreeSlotFrom20, StartObjectAtPlace, a table read by the number times 8, stores. zD9 and zDA keep their address form until they have names.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
StartRingOfPieces:
    cly
StartRingOfPieces_at1:
    sty z:zHitSlot
    jsr VecFindFreeSlotFrom20
    bcc StartRingOfPieces_at0
    lda #$28
    jsr VecStartObjectAtPlace
    stx z:zRichterObject
    lda z:zHitSlot
    asl a
    asl a
    asl a
    tax
    lda a:$ABE8,x
    sta a:OBJ_34B8,y
    lda a:$ABE9,x
    sta a:OBJ_VX_HI,y
    lda a:$ABEA,x
    sta a:OBJ_VY_HI,y
    lda a:$ABEB,x
    sta a:OBJ_3568,y
    lda a:$ABEC,x
    sta a:OBJ_35C0,y
    lda a:$ABED,x
    sta a:OBJ_3618,y
    lda a:$ABEE,x
    sta a:OBJ_3670,y
    ldx z:zRichterObject
    ldy z:zHitSlot
    iny
    cpy #$18
    bcc StartRingOfPieces_at1
StartRingOfPieces_at0:
    rts

; @routine 0B4A:4A8 LoadRecordFlingThenFall
; @run 70:ACA8
; WHAT IT DOES: In state 0 loads the record, moves the object once and takes its speed from OBJ_3568, OBJ_35C0, OBJ_3618 and OBJ_3670, and its frame from OBJ_34B8 plus $12. Later it falls (ApplyGravityUnlimited) and moves.
; HOW: LoadObjectRecord, ApplyVelocity, four copies, an add, ApplyGravityUnlimited, a tail jump into ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordFlingThenFall:
    lda a:OBJ_27A8,x
    bne LoadRecordFlingThenFall_at0
    jsr VecLoadObjectRecord
    jsr VecApplyVelocity
    lda a:OBJ_3568,x
    sta a:OBJ_VX_HI,x
    lda a:OBJ_35C0,x
    sta a:OBJ_VX_LO,x
    lda a:OBJ_3618,x
    sta a:OBJ_VY_HI,x
    lda a:OBJ_3670,x
    sta a:OBJ_VY_LO,x
    lda a:OBJ_34B8,x
    clc
    adc #$12
    sta a:OBJ_2A68,x
    rts
LoadRecordFlingThenFall_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 0B4A:4F4 LoadRecordFacingLeftRising
; @run 70:ACF4
; WHAT IT DOES: Loads the object's record, sets its facing 1, frame $15 and Y speed high $F0.
; HOW: LoadObjectRecord and three stores.
; WAITS: no.
; Entry: X object. Exit: A $F0, Y 11, C from the record.
LoadRecordFacingLeftRising:
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_2AC0,x
    lda #$15
    sta a:OBJ_2A68,x
    lda #$F0
    sta a:OBJ_VY_HI,x
    rts

; @routine 0B4A:507 SoundBAAtScreen2
; @run 70:AD07
; WHAT IT DOES: When the scroll's screen byte is 2 plays sound $BA and goes to the next state.
; HOW: A compare, SoundCommand, an INC.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
SoundBAAtScreen2:
    lda a:ScrollXHiScreen
    cmp #$02
    bne SoundBAAtScreen2_at0
    lda #$BA
    jsr VecSoundCommand
    inc a:OBJ_27A8,x
SoundBAAtScreen2_at0:
    rts

; @routine 0B4A:517 PulseWaterLevelAndScatter
; @run 70:AD17
; WHAT IT DOES: Sets slot 0's OBJ_3510 3, OBJ_3568 0 and the byte at $3BF0 to 2; hands OBJ_3510 of this object to MaybeSound19AndSpawnPiece and counts it down. When it goes below 0 it sets it $10, plays sound $19, moves in Y and advances OBJ_34B8; at 4 it draws tile script 1, clears the water level, records the object's event and clears itself. Otherwise the water level takes a pair of bytes from the table at $AD6B by the wave counter.
; HOW: MaybeSound19AndSpawnPiece, SoundCommand, ApplyVelocityY, DrawTileScript, RecordObjectEvent, ClearObject.
; WAITS: no.
; Entry: X object, Y kept for the sounds. Exit: as the last instruction that ran. The byte at $3BF0 keeps its address form until it has a name.
PulseWaterLevelAndScatter:
    lda #$02
    sta a:PlayerByte3BF0
    lda #$03
    sta a:OBJ_3510
    stz a:OBJ_3568
    lda a:OBJ_3510,x
    jsr MaybeSound19AndSpawnPiece
    dec a:OBJ_3510,x
    bpl PulseWaterLevelAndScatter_at0
    lda #$10
    sta a:OBJ_3510,x
    lda #$19
    jsr VecSoundCommand
    jsr VecApplyVelocityY
    inc a:OBJ_34B8,x
    lda a:OBJ_34B8,x
    cmp #$04
    bcc PulseWaterLevelAndScatter_at0
    lda #$01
    cly
    jsr VecDrawTileScript
    stz a:LevelYHi
    stz a:RasterWaterWanted
    jsr VecRecordObjectEvent
    jmp VecClearObject
PulseWaterLevelAndScatter_at0:
    lda z:zWaveCounter
    and #$03
    asl a
    tay
    lda a:$AD6B,y
    sta a:RasterWaterWanted
    lda a:$AD6C,y
    sta a:LevelYHi
    rts

; @routine 0B4A:58B LoadRecordThenDrawTileScript2
; @run 70:AD8B
; WHAT IT DOES: When bit 0 of zField92 is set the object is cleared; otherwise it loads its record, sets its hit offset OBJ_39E0 $E8 and draws tile script 2.
; HOW: A test, a tail jump into ClearObject, LoadObjectRecord, DrawTileScript.
; WAITS: no.
; Entry: X object. Exit: as DrawTileScript, or the test when the object is cleared. The jump into ClearObject ran in no script.
LoadRecordThenDrawTileScript2:
    lda z:zField92
    and #$01
    beq LoadRecordThenDrawTileScript2_at0
    jmp VecClearObject
LoadRecordThenDrawTileScript2_at0:
    jsr VecLoadObjectRecord
    lda #$E8
    sta a:OBJ_39E0,x
    lda #$02
    cly
    jsr VecDrawTileScript
    rts

; @routine 0B4A:5A3 DrawTileScriptStepsThenScatter
; @run 70:ADA3
; WHAT IT DOES: When the object's X high byte is not 0 draws tile script $02 plus OBJ_34B8. Otherwise, at 5 hits taken, it sets OBJ_3A90 $18, draws the next tile script, plays sound $19, scatters pieces (ScatterPiecesDownward) and advances OBJ_34B8; below 3 steps it takes OBJ_39E0 and the frame from the table at $AE02; at 3 it sets zField92 1, clears OBJ_39E0, sets the hit box $10 by $10 and frame $17 and goes to the next state.
; HOW: DrawTileScript, SoundCommand, ScatterPiecesDownward, table reads, stores.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
DrawTileScriptStepsThenScatter:
    lda a:OBJ_X_HI,x
    beq DrawTileScriptStepsThenScatter_at0
    lda #$02
    clc
    adc a:OBJ_34B8,x
    cly
    jmp VecDrawTileScript
DrawTileScriptStepsThenScatter_at0:
    lda a:OBJ_33B0,x
    cmp #$05
    bne DrawTileScriptStepsThenScatter_at1
    lda #$18
    sta a:OBJ_3A90,x
    lda #$02
    clc
    adc a:OBJ_34B8,x
    inc a
    cly
    jsr VecDrawTileScript
    lda #$19
    jsr VecSoundCommand
    jsr ScatterPiecesDownward
    inc a:OBJ_34B8,x
    lda a:OBJ_34B8,x
    cmp #$03
    bcs DrawTileScriptStepsThenScatter_at2
    asl a
    tay
    lda a:$AE02,y
    sta a:OBJ_39E0,x
    lda a:$AE03,y
    sta a:OBJ_2A68,x
DrawTileScriptStepsThenScatter_at1:
    rts
DrawTileScriptStepsThenScatter_at2:
    lda #$01
    sta z:zField92
    stz a:OBJ_39E0,x
    lda #$10
    sta a:OBJ_3988,x
    sta a:OBJ_3A38,x
    lda #$17
    sta a:OBJ_2A68,x
    inc a:OBJ_27A8,x
    rts

; @routine 0B4A:608 ScatterPiecesDownward
; @run 70:AE08
; WHAT IT DOES: Moves the object down by its hit offset OBJ_39E0, starts up to 8 objects of type $9A in free slots of $30 to $3F at its place, each with an X speed from the table at $AE4E by the parity of the count, moved once, OBJ_34B8 1 and OBJ_3510 the count less 1, then moves the object back.
; HOW: An add and a subtract of OBJ_39E0 on OBJ_Y_LO, a loop of FindFreeSlotFrom30, StartObjectAtPlace and ApplyVelocityX with X and Y swapped, a counter in zTemp00. zD9 keeps its address form until it has a name.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
ScatterPiecesDownward:
    lda a:OBJ_Y_LO,x
    clc
    adc a:OBJ_39E0,x
    sta a:OBJ_Y_LO,x
    lda #$08
    sta z:zTemp00
ScatterPiecesDownward_at1:
    jsr VecFindFreeSlotFrom30
    bcc ScatterPiecesDownward_at0
    lda #$9A
    jsr VecStartObjectAtPlace
    stx z:zRichterObject
    lda z:zTemp00
    and #$01
    tax
    lda a:$AE4E,x
    sta a:OBJ_VX_HI,y
    sxy
    jsr VecApplyVelocityX
    sxy
    ldx z:zRichterObject
    lda #$01
    sta a:OBJ_34B8,y
    lda z:zTemp00
    dec a
    sta a:OBJ_3510,y
    dec z:zTemp00
    bne ScatterPiecesDownward_at1
ScatterPiecesDownward_at0:
    lda a:OBJ_Y_LO,x
    sec
    sbc a:OBJ_39E0,x
    sta a:OBJ_Y_LO,x
    rts

; @routine 0B4A:650 ClearWhenHurtThenWait
; @run 70:AE50
; WHAT IT DOES: When the object has taken 1 to 4 hits: if the player's health zPlayerHealth is $5C plays sound $0B, becomes kind $9F with OBJ_34B8 3 and state 0; otherwise sets the health $5C and clears itself.
; HOW: Two compares, a compare of the health, SoundCommand, stores, a tail jump into ClearObject.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The arm for health $5C ran in no script; the draft stops there.
ClearWhenHurtThenWait:
    lda a:OBJ_33B0,x
    beq ClearWhenHurtThenWait_at0
    cmp #$05
    bcs ClearWhenHurtThenWait_at0
    lda z:zPlayerHealth
    cmp #$5C
    bne ClearWhenHurtThenWait_at1
    lda #$0B
    jsr VecSoundCommand
    lda #$9F
    sta a:OBJ_2750,x
    lda #$03
    sta a:OBJ_34B8,x
    stz a:OBJ_27A8,x
ClearWhenHurtThenWait_at0:
    rts
ClearWhenHurtThenWait_at1:
    lda #$5C
    sta z:zPlayerHealth
    jmp VecClearObject

; @routine 0B4A:6B6 BossPhasesHurtFallAndTileWrites
; @run 70:AEB6
; WHAT IT DOES: A boss in phases. State 0 loads the record. State 1: at 1 to 4 hits records the event and goes to state 2; at 5 hits, with the player on the right, takes the X speed 4, OBJ_34B8 $10, records the event, counts OBJ_3568 up and goes to state 5. State 2 steps the list at $AF8A and, when it ends, goes to state 3 with OBJ_34B8 $10 and starts list 0 on channel 5. State 3 waits OBJ_34B8 down, then state 4 with OBJ_34B8 $40 and frame $0F. State 4 counts down, drops one row every eighth frame and writes four tile pairs; at the end it clears the object. States 5 on fall: the X speed loses $0034 a frame until OBJ_34B8 runs out, then state 2.
; HOW: LoadObjectRecord, RecordObjectEvent, SideOfPlayer, StepObjectFrames, StartListChannelKind3, VDC writes, ApplyVelocity, ClearObject.
; WAITS: no.
; Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
BossPhasesHurtFallAndTileWrites:
    lda a:OBJ_27A8,x
    bne BossPhasesHurtFallAndTileWrites_at0
    jmp VecLoadObjectRecord
BossPhasesHurtFallAndTileWrites_at0:
    dec a
    bne BossPhasesHurtFallAndTileWrites_at1
    lda a:OBJ_33B0,x
    beq BossPhasesHurtFallAndTileWrites_at2
    cmp #$05
    bcs BossPhasesHurtFallAndTileWrites_at3
    jsr VecRecordObjectEvent
BossPhasesHurtFallAndTileWrites_at4:
    inc a:OBJ_27A8,x
    rts
BossPhasesHurtFallAndTileWrites_at3:
    jsr VecSideOfPlayer
    lsr a
    bcs BossPhasesHurtFallAndTileWrites_at4
    lda #$04
    sta a:OBJ_VX_HI,x
    lda #$10
    sta a:OBJ_34B8,x
    jsr VecRecordObjectEvent
    inc a:OBJ_3568,x
    lda #$05
    sta a:OBJ_27A8,x
    rts
BossPhasesHurtFallAndTileWrites_at1:
    dec a
    bne BossPhasesHurtFallAndTileWrites_at5
    lda #$AF
    sta z:zTemp11
    lda #$8A
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne BossPhasesHurtFallAndTileWrites_at2
    inc a:OBJ_27A8,x
    lda #$10
    sta a:OBJ_34B8,x
    phx
    cla
    ldx #$05
    jsr VecStartListChannelKind3
    plx
BossPhasesHurtFallAndTileWrites_at2:
    rts
BossPhasesHurtFallAndTileWrites_at5:
    dec a
    bne SetFrameThenCountToNext
    dec a:OBJ_34B8,x
    bne BossPhasesHurtFallAndTileWrites_at2
    inc a:OBJ_27A8,x
    lda #$40
    sta a:OBJ_34B8,x
    lda #$0F
    sta a:OBJ_2A68,x
    rts
    dec a
    bne BossPhasesHurtFallAndTileWrites_at6
    dec a:OBJ_34B8,x
    beq BossPhasesHurtFallAndTileWrites_at7
    lda a:OBJ_34B8,x
    and #$07
    bne BossPhasesHurtFallAndTileWrites_at2
    inc a:OBJ_Y_LO,x
    lda a:OBJ_34B8,x
    lsr a
    lsr a
    lsr a
    clc
    adc #$C8
    sta a:OBJ_3510,x
    bsr BossPhasesHurtFallAndTileWrites_at8
    bsr BossPhasesHurtFallAndTileWrites_at8
    bsr BossPhasesHurtFallAndTileWrites_at8
    bra BossPhasesHurtFallAndTileWrites_at8
BossPhasesHurtFallAndTileWrites_at7:
    lda a:OBJ_3568,x
    beq BossPhasesHurtFallAndTileWrites_at9
BossPhasesHurtFallAndTileWrites_at9:
    jmp VecClearObject
BossPhasesHurtFallAndTileWrites_at8:
    jsr VecSelectVramWriteAddress
    lda a:OBJ_3510,x
    sta a:VDC_DATA_LO
    clc
    adc #$10
    sta a:OBJ_3510,x
    st2 #$0F
    jsr VecSelectVramData
    st1 #$00
    st2 #$00
    rts
BossPhasesHurtFallAndTileWrites_at6:
    jsr VecApplyVelocity
    lda a:OBJ_VX_LO,x
    sec
    sbc #$34
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    sbc #$00
    sta a:OBJ_VX_HI,x
    dec a:OBJ_34B8,x
    bne BossPhasesHurtFallAndTileWrites_at2
    lda #$02
    sta a:OBJ_27A8,x
    rts

; @routine 0B4A:79D LoadRecordThenWatchThenDropTwice
; @run 70:AF9D
; WHAT IT DOES: In state 0 loads the record. State 1 looks for an object of kind $24 in slots $10 to $2E; when it is in frame $11, at X low $B8 or more on the first screen, and the player's X low is $C0 or more, the object goes to state 2. State 2: its Y speed gains $0020 a frame and it moves; at Y low $BC it stops, takes Y speed high $FE, plays sound $2C and goes to state 3. State 3 does the same fall until its Y high byte is not 0, then goes to state 4.
; HOW: LoadObjectRecord, a search, compares, 16-bit adds in memory, ApplyVelocity, SoundCommand.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. Everything past state 0 ran in no script; the draft stops there and the arms are written from the decoded bytes.
LoadRecordThenWatchThenDropTwice:
    lda a:OBJ_27A8,x
    bne LoadRecordThenWatchThenDropTwice_at0
    jmp VecLoadObjectRecord
LoadRecordThenWatchThenDropTwice_at0:
    dec a
    bne LoadRecordThenWatchThenDropTwice_at1
    ldy #$10
LoadRecordThenWatchThenDropTwice_at3:
    lda a:OBJ_2750,y
    cmp #$24
    beq LoadRecordThenWatchThenDropTwice_at2
    iny
    cpy #$2F
    bne LoadRecordThenWatchThenDropTwice_at3
LoadRecordThenWatchThenDropTwice_at6:
    rts
LoadRecordThenWatchThenDropTwice_at1:
    dec a
    bne LoadRecordThenWatchThenDropTwice_at4
    lda a:OBJ_VY_LO,x
    clc
    adc #$20
    sta a:OBJ_VY_LO,x
    bcc LoadRecordThenWatchThenDropTwice_at5
    inc a:OBJ_VY_HI,x
LoadRecordThenWatchThenDropTwice_at5:
    jsr VecApplyVelocity
    lda a:OBJ_Y_LO,x
    cmp #$BC
    bcc LoadRecordThenWatchThenDropTwice_at6
    stz a:OBJ_VY_LO,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_27A8,x
    rts
LoadRecordThenWatchThenDropTwice_at4:
    dec a
    bne LoadRecordThenWatchThenDropTwice_at6
    lda a:OBJ_VY_LO,x
    clc
    adc #$20
    sta a:OBJ_VY_LO,x
    bcc LoadRecordThenWatchThenDropTwice_at7
    inc a:OBJ_VY_HI,x
LoadRecordThenWatchThenDropTwice_at7:
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    beq LoadRecordThenWatchThenDropTwice_at6
    inc a:OBJ_27A8,x
    rts
LoadRecordThenWatchThenDropTwice_at2:
    lda a:OBJ_2A68,y
    cmp #$11
    bne LoadRecordThenWatchThenDropTwice_at6
    lda a:OBJ_X_LO,y
    cmp #$B8
    bcc LoadRecordThenWatchThenDropTwice_at6
    lda a:OBJ_X_HI,y
    bne LoadRecordThenWatchThenDropTwice_at6
    lda a:OBJ_X_LO
    cmp #$C0
    bcc LoadRecordThenWatchThenDropTwice_at6
    inc a:OBJ_27A8,x
    rts
