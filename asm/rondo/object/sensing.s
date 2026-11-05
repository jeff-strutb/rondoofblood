; @routine 0047:1BC ProbeTerrain
; @run 6A:61BC
; WHAT IT DOES: Asks what terrain an object touches, and on request acts on it.
; Bit 0 asks about the floor, bit 3 lifts the object out of it; bit 1 asks about
; walls, bit 5 pushes the object out. The answer has bit 0 for floor, bit 1 for
; wall, bit 4 for deadly ground, which also throws the object.
; HOW: The overlay's lookup fills TileHit and the depths; the floor half reads
; three columns, the wall half the two on the side the object is moving to.
; Entry: A request, X object, zProduct1 probe height. Exit: A answer, N and Z from
; it; Y, C and V as the path left them. Clobbers zTemp00, zTemp12, zProduct0.
ProbeTerrain:
    stz z:zProduct0
    sta z:zTemp12
    and #$01
    beq ProbeTerrain_at0
    lda z:zProduct1
    jsr LookUpTerrainBelow
ProbeTerrain_at8:
    lda a:TileHit
    ora a:TileHit+1
    ora a:TileHit+2
    and #$10
    beq ProbeTerrain_at1
ProbeTerrain_at9:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    lda z:zProduct0
    ora #$10
    sta z:zProduct0
    bra ProbeTerrain_at0
ProbeTerrain_at1:
    cly
ProbeTerrain_at3:
    lda a:TileHit,y
    and #$21
    bne ProbeTerrain_at2
    iny
    cpy #$03
    bcc ProbeTerrain_at3
    bra ProbeTerrain_at0
ProbeTerrain_at2:
    lda z:zProduct0
    ora #$01
    sta z:zProduct0
    lda z:zTemp12
    and #$08
    beq ProbeTerrain_at0
    lda a:TileDepthY,y
    rol a
    rol a
    and #$01
    eor #$FF
    inc a
    sta z:zTemp00
    lda a:OBJ_Y_LO,x
    sec
    sbc a:TileDepthY,y
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc z:zTemp00
    sta a:OBJ_Y_HI,x
ProbeTerrain_at0:
    lda z:zTemp12
    and #$02
    beq ProbeTerrain_at4
    lda z:zProduct1
    inc a
    jsr LookUpTerrainBelow
    cly
    lda a:OBJ_VX_HI,x
    bmi ProbeTerrain_at5
    ldy #$02
ProbeTerrain_at5:
    lda a:TileHit,y
    and #$02
    bne ProbeTerrain_at6
    iny
    lda a:TileHit,y
    and #$02
    beq ProbeTerrain_at4
ProbeTerrain_at6:
    lda z:zProduct0
    ora #$02
    sta z:zProduct0
    lda z:zTemp12
    and #$20
    beq ProbeTerrain_at4
    cpy #$02
    bcs ProbeTerrain_at7
    lda a:TileDepthX,y
    tay
    lda a:OBJ_X_LO,x
    clc
    adc a:WallPushTable,y
    sta a:OBJ_X_LO,x
    bcc ProbeTerrain_at4
    inc a:OBJ_X_HI,x
    bra ProbeTerrain_at4
ProbeTerrain_at7:
    lda a:OBJ_X_LO,x
    sec
    sbc a:TileDepthX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc #$00
    sta a:OBJ_X_HI,x
ProbeTerrain_at4:
    lda z:zProduct0
    rts

; @routine 0047:282 ProbeTerrainAlt
; @run 6A:6282
; WHAT IT DOES: ProbeTerrain, using the overlay's second lookup for the floor.
; HOW: As ProbeTerrain.
; Entry and exit: as ProbeTerrain.
ProbeTerrainAlt:
    stz z:zProduct0
    sta z:zTemp12
    and #$01
    bne ProbeTerrainAlt_at0
    jmp ProbeTerrain_at0
ProbeTerrainAlt_at0:
    lda z:zProduct1
    jsr LookUpTerrainBelowAlt
    jmp ProbeTerrain_at8

; @routine 0047:295 ProbeTerrainFrontColumn
; @run 6A:6295
; WHAT IT DOES: ProbeTerrainAlt looking at the first column only.
; HOW: The overlay's second lookup, then one column of TileHit: deadly throws;
; any floor bit is floor.
; Entry and exit: as ProbeTerrain.
ProbeTerrainFrontColumn:
    stz z:zProduct0
    sta z:zTemp12
    and #$01
    bne ProbeTerrainFrontColumn_at0
    jmp ProbeTerrain_at0
ProbeTerrainFrontColumn_at0:
    lda z:zProduct1
    jsr LookUpTerrainBelowAlt
    lda a:TileHit
    and #$10
    beq ProbeTerrainFrontColumn_at1
    jmp ProbeTerrain_at9
ProbeTerrainFrontColumn_at1:
    cly
    lda a:TileHit
    and #$01
    beq ProbeTerrainFrontColumn_at2
    jmp ProbeTerrain_at2
ProbeTerrainFrontColumn_at2:
    jmp ProbeTerrain_at0

; @routine 0047:2BD LookUpTerrainBelow
; @run 6A:62BD
; WHAT IT DOES: Reads the terrain around an object through the stage overlay's
; first terrain lookup, which leaves its answer in TileHit.
; HOW: Jumps to the overlay's vector.
; Entry: A probe height, X object. Exit: as the overlay leaves it.
LookUpTerrainBelow:
    jmp OverlayTerrainLookup

; @routine 0047:2C0 LookUpTerrainBelowAlt
; @run 6A:62C0
; WHAT IT DOES: Reads the terrain around an object through the stage overlay's
; second terrain lookup, which leaves its answer in TileHit.
; HOW: Jumps to the overlay's vector.
; Entry: A probe height, X object. Exit: as the overlay leaves it.
LookUpTerrainBelowAlt:
    jmp OverlayTerrainLookupAlt

; @routine 0047:2CB DistanceXToPlayer
; @run 6A:62CB
; WHAT IT DOES: Gives how far an object is from the player along X, in pixels,
; as a byte: $FF when either lies past the first 256 pixels.
; HOW: The high bytes ORed must be zero; then the absolute low difference.
; Entry: X object. Exit: A distance, N and Z from it, C 1 if the object is the
; further along, V from the subtract; when $FF, N 1, Z 0, C and V kept. X and Y
; kept.
DistanceXToPlayer:
    lda a:OBJ_X_HI,x
    ora a:OBJ_X_HI
    bne DistanceXToPlayer_at0
    lda a:OBJ_X_LO,x
    sec
    sbc a:OBJ_X_LO
    bcs DistanceXToPlayer_at1
    eor #$FF
    inc a
DistanceXToPlayer_at1:
    rts
DistanceXToPlayer_at0:
    lda #$FF
    rts

; @routine 0047:2E3 DistanceYToPlayer
; @run 6A:62E3
; WHAT IT DOES: Gives how far an object is from the player along Y, as
; DistanceXToPlayer does along X.
; HOW: The same, on the Y position.
; Entry: X object. Exit: as DistanceXToPlayer.
DistanceYToPlayer:
    lda a:OBJ_Y_HI,x
    ora a:OBJ_Y_HI
    bne DistanceYToPlayer_at0
    lda a:OBJ_Y_LO,x
    sec
    sbc a:OBJ_Y_LO
    bcs DistanceYToPlayer_at1
    eor #$FF
    inc a
DistanceYToPlayer_at1:
    rts
DistanceYToPlayer_at0:
    lda #$FF
    rts

; @routine 0047:2FB SideOfPlayer
; @run 6A:62FB
; WHAT IT DOES: Says which side of the player an object is on: bit 0 set when it
; is left of the player, bit 1 when above.
; HOW: On each axis, a high byte of the object nonzero decides by its sign
; (negative is before the player); zero compares the low byte with the player's.
; The player's high byte is not looked at.
; Entry: X object. Exit: A the bits, N and Z from it, C from the last compare
; made, else kept. zTemp00 bit 0. X, Y and V kept.
SideOfPlayer:
    lda a:OBJ_X_HI,x
    bne SideOfPlayer_at0
    lda a:OBJ_X_LO,x
    cmp a:OBJ_X_LO
    bcs SideOfPlayer_at1
    bra SideOfPlayer_at2
SideOfPlayer_at0:
    bpl SideOfPlayer_at1
SideOfPlayer_at2:
    lda #$01
    sta z:zTemp00
    bra SideOfPlayer_at3
SideOfPlayer_at1:
    stz z:zTemp00
SideOfPlayer_at3:
    lda a:OBJ_Y_HI,x
    bne SideOfPlayer_at4
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_Y_LO
    bcs SideOfPlayer_at5
    bra SideOfPlayer_at6
SideOfPlayer_at4:
    bpl SideOfPlayer_at5
SideOfPlayer_at6:
    lda z:zTemp00
    ora #$02
    rts
SideOfPlayer_at5:
    lda z:zTemp00
    rts

; @routine 0047:32D MoveAndLand
; @run 6A:632D
; WHAT IT DOES: One frame of a falling, walking object: moves it, applies
; gravity, lands it on floor (stopping its fall) and pushes it out of walls.
; HOW: ApplyVelocity, ApplyGravity; a floor probe only when not rising, then a
; wall probe.
; Entry: A probe height, X object. Exit: A bit 0 landed, bit 1 walled; C 1 if
; walled; N and Z from A. Y and V as the probes left them. X kept.
MoveAndLand:
    sta z:zProduct1
    jsr VecApplyVelocity
    stz a:WorkByteA
    stz a:WorkByteB
    jsr ApplyGravity
    lda a:OBJ_VY_HI,x
    bmi MoveAndLand_at0
    lda #$09
    jsr ProbeTerrain
    and #$01
    beq MoveAndLand_at0
    inc a:WorkByteB
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
MoveAndLand_at0:
    lda #$22
    jsr ProbeTerrain
    and #$02
    beq MoveAndLand_at1
    lda a:WorkByteB
    ora #$02
    sec
    rts
MoveAndLand_at1:
    lda a:WorkByteB
    clc
    rts

; @routine 0047:367 StopOnLanding
; @run 6A:6367
; WHAT IT DOES: One frame of a thrown object that stops dead where it lands: moves
; it, applies gravity, and on landing zeroes all its speed.
; HOW: The start of MoveAndLand, then runs into StopOnLandingNoMove.
; Entry: A probe height, X object. Exit: as StopOnLandingNoMove.
StopOnLanding:
    sta z:zProduct1
    jsr VecApplyVelocity
    stz a:WorkByteA
    stz a:WorkByteB
    jsr ApplyGravity

; @routine 0047:375 StopOnLandingNoMove
; @run 6A:6375
; WHAT IT DOES: The landing half of StopOnLanding, for a caller that has moved
; the object itself.
; HOW: Floor probe with the overlay's second lookup when not rising, then the
; wall probe and the stop.
; Entry: X object, zProduct1 probe height, WorkByteA clear. Exit: A bit 0
; landed, bit 1 walled; C 1 if landed; N and Z from A. Y and V as the probes left
; them.
StopOnLandingNoMove:
    lda a:OBJ_VY_HI,x
    bmi StopOnLandingNoMove_at0
    lda #$09
    jsr ProbeTerrainAlt
    sta a:WorkByteA
StopOnLandingNoMove_at0:
    lda #$22
    jsr ProbeTerrain
    and #$02
    sta a:WorkByteB
    lda a:WorkByteA
    and #$01
    beq StopOnLandingNoMove_at1
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda a:WorkByteB
    ora #$01
    sec
    rts
StopOnLandingNoMove_at1:
    lda a:WorkByteB
    clc
    rts

; @routine 0047:3AB StopOnLandingFrontColumn
; @run 6A:63AB
; WHAT IT DOES: StopOnLanding, reading only the first column of floor.
; HOW: The same start, ProbeTerrainFrontColumn for the floor, the same end.
; Entry: A probe height, X object. Exit: as StopOnLandingNoMove.
StopOnLandingFrontColumn:
    sta z:zProduct1
    jsr VecApplyVelocity
    stz a:WorkByteA
    stz a:WorkByteB
    jsr ApplyGravity
    lda a:OBJ_VY_HI,x
    bmi StopOnLandingNoMove_at0
    lda #$09
    jsr ProbeTerrainFrontColumn
    sta a:WorkByteA
    bra StopOnLandingNoMove_at0

; @routine 0047:3ED ApplyGravity
; @run 6A:63ED
; WHAT IT DOES: Pulls an object down: adds $38/256 pixel a frame to its Y speed,
; until the speed reaches 7 pixels a frame.
; HOW: Rising speeds always gain; falling speeds of 7 or more do not. Runs into
; ApplyGravityUnlimited.
; Entry: X object. Exit, at the limit: A the speed high, N and Z of A - 7, C 1.
; Exit, gaining: as ApplyGravityUnlimited. X and Y kept.
ApplyGravity:
    lda a:OBJ_VY_HI,x
    bmi ApplyGravityUnlimited
    cmp #$07
    bcs ApplyGravityUnlimited_at0

; @routine 0047:3F6 ApplyGravityUnlimited
; @run 6A:63F6
; WHAT IT DOES: Adds $38/256 pixel a frame to an object's Y speed, whatever it is.
; HOW: An 8.8 add carried into the high byte by an increment.
; Entry: X object. Exit: A the new low byte, C and V from its add, N and Z from it
; or, on a carry, from the new high byte. X and Y kept.
ApplyGravityUnlimited:
    lda a:OBJ_VY_LO,x
    clc
    adc #$38
    sta a:OBJ_VY_LO,x
    bcc ApplyGravityUnlimited_at0
    inc a:OBJ_VY_HI,x
ApplyGravityUnlimited_at0:
    rts
