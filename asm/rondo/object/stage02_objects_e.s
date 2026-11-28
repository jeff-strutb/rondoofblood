; @routine 051E:02A LoadPieceBySlot051B
; @run 71:C82A
; WHAT IT DOES: State 0 loads the record as sprite set 6, with the frame and speeds the tables give for this slot (from $10);
; later it falls ($C809).
; HOW: Tables by slot less $10.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadPieceBySlot051BFrames = $C85E ; frames by slot
LoadPieceBySlot051BXSpeeds = $C864 ; X speed words by slot
LoadPieceBySlot051BYSpeeds = $C870 ; Y speed words by slot
LoadPieceBySlot051B:
    lda a:OBJ_STATE,x
    bne FallAndClearBelow051B
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    txa
    sec
    sbc #$10
    tay
    lda a:LoadPieceBySlot051BFrames,y
    sta a:OBJ_FRAME,x
    tya
    asl a
    tay
    lda a:LoadPieceBySlot051BXSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:LoadPieceBySlot051BXSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LoadPieceBySlot051BYSpeeds,y
    sta a:OBJ_VY_LO,x
    lda a:LoadPieceBySlot051BYSpeeds+1,y
    sta a:OBJ_VY_HI,x
    rts

; @routine 051E:07C LoadPictureFrame0C_051B
; @run 71:C87C
; WHAT IT DOES: State 0 loads the record as sprite set 6 frame $0C; later does nothing.
; HOW: Stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
LoadPictureFrame0C_051B:
    lda a:OBJ_STATE,x
    bne LoadPictureFrame0C_051B_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$0C
    sta a:OBJ_FRAME,x
    rts
LoadPictureFrame0C_051B_at0:
    rts

; @routine 051E:101 EmitBubbles051B
; @run 71:C901
; WHAT IT DOES: State 0 loads the record and picks the first wait ($C909); later counts and emits ($C915).
; HOW: A branch by state.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
EmitBubbles051B:
    lda a:OBJ_STATE,x
    bne SpawnPointFromTables
    jsr VecLoadObjectRecord

; @routine 051E:17E BubbleRiseAndSway051B
; @run 71:C97E
; WHAT IT DOES: A bubble. State 0 loads it as sprite set 6 with a rise, drift and swing from tables and a life of $80-$FF. Then
; steps its list, rises, drifts left by its drift word and swings on the cosine; on the life's end it flickers
; (attribute bit 4) for $10 frames and goes. Off the first screen it goes; it also moves against the camera's
; horizontal speed times 64.
; HOW: Tables by start argument and count; two cosine speeds.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BubbleRiseAndSway051BRows = $C97C ; first row by start argument
BubbleRiseAndSway051BMotion = $CAE4 ; VY word and drift word by row
BubbleRiseAndSway051BSwing = $CB04 ; swing magnitude words by count
BubbleRiseAndSway051B:
    lda a:OBJ_STATE,x
    bne BubbleRiseAndSway051B_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_PIECE_PLACE,x
    asl a
    asl a
    clc
    adc a:BubbleRiseAndSway051BRows,y
    tay
    lda a:BubbleRiseAndSway051BMotion,y
    sta a:OBJ_VY_LO,x
    lda a:BubbleRiseAndSway051BMotion+1,y
    sta a:OBJ_VY_HI,x
    lda a:BubbleRiseAndSway051BMotion+2,y
    sta a:OBJ_SWOOP_SPEED_LO,x
    lda a:BubbleRiseAndSway051BMotion+3,y
    sta a:OBJ_SWOOP_SPEED_HI,x
    lda a:OBJ_PIECE_PLACE,x
    asl a
    tay
    lda a:BubbleRiseAndSway051BSwing,y
    sta a:OBJ_WORK_J,x
    lda a:BubbleRiseAndSway051BSwing+1,y
    sta a:OBJ_WORK_K,x
    jsr VecNextRandom
    and #$7F
    clc
    adc #$80
    sta a:OBJ_PIECE_NEXT,x
    rts
BubbleRiseAndSway051B_at0:
    dec a:OBJ_PIECE_NEXT,x
    bne BubbleRiseAndSway051B_at2
    tst #$10,a:OBJ_SPRITE_ATTR,x
    beq BubbleRiseAndSway051B_at1
    jmp VecClearObject
BubbleRiseAndSway051B_at1:
    lda #$10
    sta a:OBJ_PIECE_NEXT,x
    lda #$10
    ora a:OBJ_SPRITE_ATTR,x
    sta a:OBJ_SPRITE_ATTR,x
BubbleRiseAndSway051B_at2:
    lda #$D9
    sta z:zTemp10
    lda #$CA
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    lda a:OBJ_X_FRAC,x
    clc
    adc a:OBJ_SWOOP_SPEED_LO,x
    sta a:OBJ_X_FRAC,x
    lda a:OBJ_X_LO,x
    adc a:OBJ_SWOOP_SPEED_HI,x
    sta a:OBJ_X_LO,x
    lda #$FF
    adc a:OBJ_X_HI,x
    sta a:OBJ_X_HI,x
    jsr VecApplyVelocityX
    lda a:OBJ_WORK_J,x
    sta z:zTemp00
    lda a:OBJ_WORK_K,x
    sta z:zTemp01
    lda a:OBJ_RAIN_COUNT,x
    jsr VecScaleByCosine
    lda z:zPlaneRowsLeft
    sta a:OBJ_VX_HI,x
    lda z:SpriteFlipMask
    sta a:OBJ_VX_LO,x
    jsr VecApplyVelocityX
    lda a:OBJ_FALL_LO,x
    clc
    adc #$00
    sta a:OBJ_FALL_LO,x
    lda a:OBJ_RAIN_COUNT,x
    adc #$02
    sta a:OBJ_RAIN_COUNT,x
    lda a:OBJ_START_ARG,x
    bne BubbleRiseAndSway051B_at4
    lda a:OBJ_Y_HI,x
    and #$80
    ora a:OBJ_X_HI,x
    and #$80
    beq BubbleRiseAndSway051B_at5
BubbleRiseAndSway051B_at3:
    jmp VecClearObject
BubbleRiseAndSway051B_at4:
    lda a:OBJ_Y_HI,x
    and #$01
    ora a:OBJ_X_HI,x
    and #$80
    bne BubbleRiseAndSway051B_at3
BubbleRiseAndSway051B_at5:
    stz z:zTemp00
    stz z:zTemp02
    lda a:ScrollSpeedX
    sta z:zTemp01
    beq BubbleRiseAndSway051B_at7
    bpl BubbleRiseAndSway051B_at6
    dec z:zTemp02
BubbleRiseAndSway051B_at6:
    lda z:zTemp02
    lsr a
    ror z:zTemp01
    ror z:zTemp00
    lda z:zTemp02
    lsr a
    ror z:zTemp01
    ror z:zTemp00
    lda a:OBJ_X_FRAC,x
    sec
    sbc z:zTemp00
    sta a:OBJ_X_FRAC,x
    lda a:OBJ_X_LO,x
    sbc z:zTemp01
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc z:zTemp02
    sta a:OBJ_X_HI,x
BubbleRiseAndSway051B_at7:
    rts

; @routine 051E:30C PieceObjectInit051B
; @run 71:CB0C
; WHAT IT DOES: State 0: unless zField8F has the bit OBJ_START_ARG picks (then the object is cleared), loads the record and draws
; the tile script its 16-byte piece record names, with the record's hit box. Later: breaks by hits ($CB4F).
; HOW: As the stage's other piece objects.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
PieceObjectInit051B:
    lda a:OBJ_STATE,x
    bne BreakPieceByHits051B
    ldy a:OBJ_START_ARG,x
    jsr BitOfY051B
    and z:zField8F
    beq PieceObjectInit051B_at0
    jmp VecClearObject
PieceObjectInit051B_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_PIECE_PLACE,x
    tay
    lda a:PieceRecords051A,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:PieceRecords051A+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:PieceRecords051A+2,y
    clc
    adc a:OBJ_PIECE_NEXT,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 051E:344 BitOfY051B
; @run 71:CB44
; WHAT IT DOES: A with bit Y set.
; HOW: A shift loop.
; WAITS: no.
; Entry: Y the bit. Exit: A the mask, Y 0, C and N, Z from the last compare.
BitOfY051B:
    lda #$01
BitOfY051B_at0:
    cpy #$00
    beq BitOfY051B_at1
    asl a
    dey
    bra BitOfY051B_at0
BitOfY051B_at1:
    rts

; @routine 051E:34F BreakPieceByHits051B
; @run 71:CB4F
; WHAT IT DOES: Redraws the piece's current tile script; a hit that the piece's table takes draws the next one, with sound $7F
; and debris, and moves by the piece's speeds. After the record's number of hits it sets its zField8F bit and becomes
; the record's kind, at rest.
; HOW: Piece records of 16 bytes by OBJ_PIECE_PLACE, a hit-type table at $B0DE.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
BreakPieceByHits051BHits = $B07E
BreakPieceByHits051BTaken = $B0DE ; whether a piece takes a hit type
BreakPieceByHits051B:
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    beq BreakPieceByHits051B_at0
    ldy a:OBJ_PIECE_PLACE,x
    lda a:PieceRecords051A+2,y
    clc
    adc a:OBJ_PIECE_NEXT,x
    cly
    jsr VecDrawTileScript
BreakPieceByHits051B_at0:
    lda a:OBJ_HIT_TAKEN,x
    sta z:zTemp00
    ldy a:OBJ_PIECE_PLACE,x
    lda a:PieceRecords051A+3,y
    asl a
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:BreakPieceByHits051BTaken,y
    beq BreakPieceByHits051B_at1
    lda #$18
    sta a:OBJ_INVULN_TIMER,x
    ldy a:OBJ_PIECE_PLACE,x
    lda a:PieceRecords051A+2,y
    clc
    adc a:OBJ_PIECE_NEXT,x
    inc a
    cly
    jsr VecDrawTileScript
    lda #$7F
    jsr VecSoundCommand
    lda #$01
    jsr StartDebrisBurst
    ldy a:OBJ_PIECE_PLACE,x
    inc a:OBJ_PIECE_NEXT,x
    lda a:OBJ_PIECE_NEXT,x
    cmp a:PieceRecords051A-1,y
    bcs BreakPieceByHits051B_at2
    lda a:OBJ_PIECE_NEXT,x
    dec a
    asl a
    clc
    adc a:OBJ_PIECE_PLACE,x
    tay
    lda a:PieceRecords051A+5,y
    sta a:OBJ_VX_HI,x
    lda a:PieceRecords051A+6,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
BreakPieceByHits051B_at1:
    rts
BreakPieceByHits051B_at2:
    ldy a:OBJ_START_ARG,x
    jsr BitOfY051B
    ora z:zField8F
    sta z:zField8F
    ldy a:OBJ_PIECE_PLACE,x
    lda a:PieceRecords051A+4,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 051E:6C1 NegateSpeedIfFacingLeft051B
; @run 71:CEC1
; WHAT IT DOES: Facing left, negates the X speed.
; HOW: A 16-bit negation.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
NegateSpeedIfFacingLeft051B:
    lda a:OBJ_FACING_LEFT,x
    beq NegateSpeedIfFacingLeft051B_at0
    lda a:OBJ_VX_HI,x
    eor #$FF
    sta z:zTemp00
    lda a:OBJ_VX_LO,x
    eor #$FF
    clc
    adc #$01
    sta a:OBJ_VX_LO,x
    cla
    adc z:zTemp00
    sta a:OBJ_VX_HI,x
NegateSpeedIfFacingLeft051B_at0:
    rts

; @routine 051E:6DF CopyFacingToY051B
; @run 71:CEDF
; WHAT IT DOES: Gives slot Y this object's facing.
; HOW: One byte.
; WAITS: no.
; Entry: X object, Y the other. Exit: A the facing.
CopyFacingToY051B:
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    rts

; @routine 051E:6E6 PlaceAheadInY051B
; @run 71:CEE6
; WHAT IT DOES: Gives slot Y this object's X plus zTemp00-01, taken the other way when facing left.
; HOW: The offset negated in place by facing.
; WAITS: no.
; Entry: X object, Y the other, zTemp00-01 the offset. Exit: A the X high byte.
PlaceAheadInY051B:
    lda a:OBJ_FACING_LEFT,x
    beq PlaceAheadInY051B_at0
    lda z:zTemp01
    eor #$FF
    sta z:zTemp04
    lda z:zTemp00
    eor #$FF
    clc
    adc #$01
    sta z:zTemp00
    cla
    adc z:zTemp04
    sta z:zTemp01
PlaceAheadInY051B_at0:
    lda a:OBJ_X_LO,x
    clc
    adc z:zTemp00
    sta a:OBJ_X_LO,y
    lda a:OBJ_X_HI,x
    adc z:zTemp01
    sta a:OBJ_X_HI,y
    rts

; @routine 051E:711 StartEightState2WhenHit051B
; @run 71:CF11
; WHAT IT DOES: Under state 2, a hit records the event of slot $10, puts slots $10-$17 in state 2 and lets the camera scroll to
; screen 3.
; HOW: A loop over the eight.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StartEightState2WhenHit051B:
    lda a:OBJ_STATE,x
    cmp #$02
    bcs StartEightState2WhenHit051B_at1
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq StartEightState2WhenHit051B_at1
    phx
    ldx #$10
    jsr VecRecordObjectEvent
StartEightState2WhenHit051B_at0:
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    inx
    cpx #$18
    bne StartEightState2WhenHit051B_at0
    plx
    lda #$03
    sta a:ScrollXMaxHi
StartEightState2WhenHit051B_at1:
    rts

; @routine 051E:739 FacePlayerByPlace051B
; @run 71:CF39
; WHAT IT DOES: Faces left when the object is right of the player: off the first screen to the right, or on it at the player's X
; low or more.
; HOW: X high, then X low against the player's.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FacePlayerByPlace051B:
    lda a:OBJ_X_HI,x
    beq FacePlayerByPlace051B_at0
    bmi FacePlayerByPlace051B_at2
    bra FacePlayerByPlace051B_at1
FacePlayerByPlace051B_at0:
    lda a:OBJ_X_LO,x
    cmp a:OBJ_X_LO
    bcc FacePlayerByPlace051B_at2
FacePlayerByPlace051B_at1:
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    rts
FacePlayerByPlace051B_at2:
    stz a:OBJ_FACING_LEFT,x
    rts
