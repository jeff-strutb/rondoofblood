; @routine 0047:1BC ProbeTerrain
; @run 6A:61BC
; WHAT IT DOES: Asks what terrain an object touches, and on request acts on it.
; Bit 0 asks about the floor, bit 3 lifts the object out of it; bit 1 asks about
; walls, bit 5 pushes the object out. The answer has bit 0 for floor, bit 1 for
; wall, bit 4 for deadly ground, which also throws the object.
; HOW: The overlay's lookup fills TileHit and the depths; ProbeFloorRead reads the floor, the
; wall half is ProbeWalls.
; WAITS: no.
; Entry: A request, X object, Y column, zProduct1 probe height, C in (it reaches the overlay's
; lookup unchanged), D clear. Exit: A answer, N and Z from it; Y, C and V as the path left them.
; Clobbers zTemp00, zTemp12, zProduct0.
ProbeTerrain:
    stz z:zProduct0
    sta z:zTemp12
    and #$01
    beq ProbeWalls
    lda z:SpriteFlipMask
    jsr LookUpTerrainBelow

; @routine 0047:1C9 ProbeFloorRead
; @run 6A:61C9
; WHAT IT DOES: After a floor lookup: reads the three columns it filled; deadly ground in any of
; them throws the object up at a fixed speed and sets the answer's spikes bit; otherwise the first
; column with a solid top is the floor (ProbeFloorHit). Then the wall half.
; HOW: The three attributes ORed are tested for the deadly bit; a loop looks for $21 in them.
; WAITS: no.
; Entry: X object, C in (what the lookup left), D clear. Exit: as ProbeWalls.
ProbeFloorRead:
    lda a:TileHit
    ora a:TileHit+1
    ora a:TileHit+2
    and #$10
    beq ProbeFloorRead_at0
ProbeFloorRead_at2:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    lda #$40
    sta a:OBJ_VY_LO,x
    lda z:zProduct0
    ora #$10
    sta z:zProduct0
    bra ProbeWalls
ProbeFloorRead_at0:
    cly
ProbeFloorRead_at1:
    lda a:TileHit,y
    and #$21
    bne ProbeFloorHit
    iny
    cpy #$03
    bcc ProbeFloorRead_at1
    bra ProbeWalls

; @routine 0047:1FB ProbeFloorHit
; @run 6A:61FB
; WHAT IT DOES: Standing on a floor in the column in Y: sets the answer's floor bit and, when the
; request asks, lifts the object onto it by the column's depth (a signed byte). Then the wall
; half.
; HOW: The depth is read twice, as the assembly does: once for its sign byte (kept in zTemp00),
; once for the subtract; the object's Y goes down by it, the sign byte as the high part.
; WAITS: no.
; Entry: X object, Y floor column, C in, D clear. Exit: as ProbeWalls.
ProbeFloorHit:
    lda z:zProduct0
    ora #$01
    sta z:zProduct0
    lda z:zTemp12
    and #$08
    beq ProbeWalls
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

; @routine 0047:225 ProbeWalls
; @run 6A:6225
; WHAT IT DOES: The wall half of a probe, and its end: when the request asks about walls, looks
; the terrain up a row lower, reads the two columns on the side the object is moving to and, if
; one is solid, sets the answer's wall bit and, when asked, pushes the object out of the wall.
; HOW: The request is in zTemp12, the probe height in zProduct1; the answer in zProduct0 is
; the result.
; WAITS: no.
; Entry: X object, Y column, C in (kept to the lookup), D clear. Exit: A the answer, N and Z
; from it; Y, C and V as the path left them.
ProbeWalls:
    lda z:zTemp12
    and #$02
    beq ProbeWalls_at0
    lda z:SpriteFlipMask
    inc a
    jsr LookUpTerrainBelow
    cly
    lda a:OBJ_VX_HI,x
    bmi ProbeWalls_at1
    ldy #$02
ProbeWalls_at1:
    lda a:TileHit,y
    and #$02
    bne ProbeWalls_at2
    iny
    lda a:TileHit,y
    and #$02
    beq ProbeWalls_at0
ProbeWalls_at2:
    lda z:zProduct0
    ora #$02
    sta z:zProduct0
    lda z:zTemp12
    and #$20
    beq ProbeWalls_at0
    cpy #$02
    bcs ProbeWalls_at3
    lda a:TileDepthX,y
    tay
    lda a:OBJ_X_LO,x
    clc
    adc a:WallPushTable,y
    sta a:OBJ_X_LO,x
    bcc ProbeWalls_at0
    inc a:OBJ_X_HI,x
    bra ProbeWalls_at0
ProbeWalls_at3:
    lda a:OBJ_X_LO,x
    sec
    sbc a:TileDepthX,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    sbc #$00
    sta a:OBJ_X_HI,x
ProbeWalls_at0:
    lda z:zProduct0
    rts

; @routine 0047:282 ProbeTerrainAlt
; @run 6A:6282
; WHAT IT DOES: ProbeTerrain, using the overlay's second lookup for the floor.
; HOW: As ProbeTerrain; it jumps into ProbeFloorRead after the lookup.
; WAITS: no.
; Entry and exit: as ProbeTerrain.
ProbeTerrainAlt:
    stz z:zProduct0
    sta z:zTemp12
    and #$01
    bne ProbeTerrainAlt_at0
    jmp ProbeWalls
ProbeTerrainAlt_at0:
    lda z:SpriteFlipMask
    jsr LookUpTerrainBelowAlt
    jmp ProbeFloorRead

; @routine 0047:295 ProbeTerrainFrontColumn
; @run 6A:6295
; WHAT IT DOES: ProbeTerrainAlt looking at the first column only.
; HOW: The overlay's second lookup, then one column of TileHit: deadly throws;
; any floor bit is floor (ProbeFloorHit with column 0).
; WAITS: no.
; Entry and exit: as ProbeTerrain.
ProbeTerrainFrontColumn:
    stz z:zProduct0
    sta z:zTemp12
    and #$01
    bne ProbeTerrainFrontColumn_at0
    jmp ProbeWalls
ProbeTerrainFrontColumn_at0:
    lda z:SpriteFlipMask
    jsr LookUpTerrainBelowAlt
    lda a:TileHit
    and #$10
    beq ProbeTerrainFrontColumn_at1
    jmp ProbeFloorRead_at2
ProbeTerrainFrontColumn_at1:
    cly
    lda a:TileHit
    and #$01
    beq ProbeTerrainFrontColumn_at2
    jmp ProbeFloorHit
ProbeTerrainFrontColumn_at2:
    jmp ProbeWalls

; @routine 0047:2BD LookUpTerrainBelow
; @run 6A:62BD
; WHAT IT DOES: Reads the terrain around an object through the stage overlay's
; first terrain lookup, which leaves its answer in TileHit.
; HOW: Jumps to the overlay's vector.
; WAITS: no.
; Entry: A probe height, X object. Exit: as the overlay leaves it.
LookUpTerrainBelow:
    jmp OverlayTerrainLookup

; @routine 0047:2C0 LookUpTerrainBelowAlt
; @run 6A:62C0
; WHAT IT DOES: Reads the terrain around an object through the stage overlay's
; second terrain lookup, which leaves its answer in TileHit.
; HOW: Jumps to the overlay's vector.
; WAITS: no.
; Entry: A probe height, X object. Exit: as the overlay leaves it.
LookUpTerrainBelowAlt:
    jmp OverlayTerrainLookupAlt

; What an object can tell about the player and about the ground under it.
;
; A routine's body returns how it ended, as plain values. Its wrapper replays what those steps
; leave in the registers and flags, in the assembly's order, through the *_exit_shim functions.
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
; @routine 0047:2CB DistanceXToPlayer
; @run 6A:62CB
; WHAT IT DOES: Gives how far an object is from the player along X, in pixels,
; as a byte: $FF when either lies past the first 256 pixels.
; HOW: The high bytes ORed must be zero; then the absolute low difference.
; WAITS: no.
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
; WAITS: no.
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
; The player's high byte is not looked at. The bit for X goes to zTemp00, which is read
; back at the end.
; WAITS: no.
; Entry: X object, D clear. Exit: A the bits, N and Z from it, C from the last compare
; made (Y's when the Y high byte is zero, else X's when that is zero), else kept. zTemp00
; bit 0. X, Y and V kept.
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
; WAITS: no.
; Entry: A probe height, X object, Y column, C in (kept to the lookups), D clear. Exit: A bit 0
; landed, bit 1 walled; C 1 if walled; N and Z from A. Y and V as the probes left them. X kept.
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
; WAITS: no.
; Entry: A probe height, X object, Y column, C in, D clear. Exit: as StopOnLandingNoMove.
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
; WAITS: no.
; Entry: X object, Y column, zProduct1 probe height, WorkByteA clear, C in. Exit: A bit 0
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
; WAITS: no.
; Entry: A probe height, X object, Y column, C in, D clear. Exit: as StopOnLandingNoMove.
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
; WAITS: no.
; Entry: X object, D clear. Exit, at the limit: A the speed high, N and Z of A - 7, C 1.
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
; WAITS: no.
; Entry: X object, D clear. Exit: A the new low byte, C and V from its add, N and Z from it
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
