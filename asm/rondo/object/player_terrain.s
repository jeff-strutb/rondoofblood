; @routine 01BA:22F RichterSolidUnderColumn
; @run 6D:AA2F
; WHAT IT DOES: Says whether one column of terrain, $0E below Richter, is solid to stand on:
; a tile with bit 7 set does not count.
; HOW: Bit 0 of the answer, unless bit 7 is set.
; Entry: X column. Exit: A the answer, N and Z from it. X kept; Y, C and V from the read.
RichterSolidUnderColumn:
    jsr RichterColumnOffset
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0E
    jsr RichterReadBelow
    and #$81
    bpl RichterSolidUnderColumn_at0
    lda #$00
RichterSolidUnderColumn_at0:
    rts

; @routine 01BA:248 RichterReadUnderFeet
; @run 6D:AA48
; WHAT IT DOES: Reads the terrain straight under his feet, $1E below his position.
; HOW: His own X with no column's offset.
; Entry: nothing. Exit: as the read.
RichterReadUnderFeet:
    lda a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$1E
    jmp RichterReadBelow

; @routine 01BB:6D2 RichterReadAbove
; @run 6D:B6D2
; WHAT IT DOES: Reads the terrain at a row that may lie above the map: a row off the top
; reads as nothing.
; HOW: The row's add, with its carry, decides the point's Y high byte; only zero is read.
; Entry: A row, C its add's carry, zTemp00 X. Exit: as the read, or A 0 with N and Z from the add.
RichterReadAbove:
    sta z:zTemp02
    cla
    adc #$FF
    sta z:zTemp03
    bne RichterReadBelow_at0
    bra RichterReadBelow_at1

; @routine 01BB:6DD RichterReadBelow
; @run 6D:B6DD
; WHAT IT DOES: Reads the terrain at a row on the first screen down: a row past it reads as
; nothing.
; HOW: The carry from the row's add says it passed; otherwise X and Y high bytes are zero.
; Entry: A row, C its add's carry, zTemp00 X. Exit: as the read, or A 0 with N, Z, C and V kept.
RichterReadBelow:
    sta z:zTemp02
    bcs RichterReadBelow_at0
    stz z:zTemp03
RichterReadBelow_at1:
    stz z:zTemp01
    jmp FrameOverlay8006
RichterReadBelow_at0:
    cla
    rts

; @routine 01BB:6EA RichterReadOnScreen
; @run 6D:B6EA
; WHAT IT DOES: Reads the terrain at a row, the Y high byte taken from Richter's own.
; HOW: His Y high byte plus the carry from the row's add.
; Entry: A row, C its add's carry, zTemp00 X. Exit: as the read.
RichterReadOnScreen:
    sta z:zTemp02
    cla
    adc a:OBJ_Y_HI
    sta z:zTemp03
    bra RichterReadBelow_at1

; @routine 01BB:6F4 RichterReadIfOnPage
; @run 6D:B6F4
; WHAT IT DOES: Reads the terrain at the point in zTemp00-03, unless the Y high byte just
; added is not zero.
; HOW: Tests what the caller's add left.
; Entry: Z from that byte, zTemp00-03 the point. Exit: as the read, or A 0 with N and Z kept.
RichterReadIfOnPage:
    bne RichterReadBelow_at0
    jmp FrameOverlay8006

; @routine 01BB:6F9 RichterColumnOffset
; @run 6D:B6F9
; WHAT IT DOES: Gives how far across a probe's column lies, by column and facing.
; HOW: A table of pairs, one for each way he faces.
; Entry: X column. Exit: A the offset, N and Z from it, Y its index, C and V from the index's
; add. X kept.
RichterColumnOffset:
    txa
    asl a
    clc
    adc a:OBJ_2AC0
    tay
    lda a:RichterColumnOffsets,y
    rts

; @routine 01BC:068 RichterUpHeld
; @run 6D:B868
; WHAT IT DOES: Says whether up is held.
; HOW: Bit 6 of zPadHeld.
; Entry: nothing. Exit: C the answer. All else kept.
RichterUpHeld:
    bbs6 zPadHeld,RichterUpHeld_at0
    clc
    rts
RichterUpHeld_at0:
    sec
    rts

; @routine 01BC:06F RichterJumpOrClimb
; @run 6D:B86F
; WHAT IT DOES: Says whether jump was pressed and he may act on it: with up held, only if there
; is a ladder to take.
; HOW: RichterJumpPressed, then RichterLadderCheck when up is held; the latched buttons clear.
; Entry: nothing. Exit: C the answer, with what the last step left.
RichterJumpOrClimb:
    bsr RichterJumpPressed
    bcc RichterJumpPressed_at0
    stz a:PadForcedPressB
    tst #$40,z:zPadHeld
    beq RichterJumpPressed_at1
    jmp RichterLadderCheck

; @routine 01BC:07E RichterJumpPressed
; @run 6D:B87E
; WHAT IT DOES: Says whether the jump button was newly pressed.
; HOW: Bit 0 of zPadPressed.
; Entry: nothing. Exit: C the answer. All else kept.
RichterJumpPressed:
    bbs0 zPadPressed,RichterJumpPressed_at1
RichterJumpPressed_at0:
    clc
    rts
RichterJumpPressed_at1:
    sec
    rts

; @routine 01BC:085 RichterHasNoFooting
; @run 6D:B885
; WHAT IT DOES: Says whether Richter has nothing to stand on: no floor found, or the column
; found is not solid under him.
; HOW: RichterFloorCheck, then RichterSolidUnderColumn.
; Entry: nothing. Exit: C the answer; A, X, Y and V as the last step left them.
RichterHasNoFooting:
    jsr RichterFloorCheck
    bcc RichterHasNoFooting_at0
    jsr RichterSolidUnderColumn
    beq RichterHasNoFooting_at1
RichterHasNoFooting_at0:
    sec
    rts
RichterHasNoFooting_at1:
    clc
    rts

; @routine 01BC:093 RichterLadderCheck
; @run 6D:B893
; WHAT IT DOES: Says whether there is something to climb down from here: looks downward from
; $18 below him, a tile at a time, for the first solid tile with one of bits 0 or 2 above it.
; HOW: The section's camera mode settles it in two cases; low on the screen it cannot be.
; Entry: nothing. Exit: C the answer. A, Y and V as the last step left them. X kept. Clobbers
; zTemp00-03, zTemp10-11.
RichterLadderCheck:
    lda a:CameraMode
    cmp #$01
    beq RichterLadderCheck_at0
    lda a:CameraMode
    cmp #$18
    beq RichterLadderCheck_at1
    lda a:OBJ_Y_HI
    bne RichterLadderCheck_at0
    lda a:OBJ_Y_LO
    cmp #$B0
    bcs RichterLadderCheck_at0
    lda #$18
    sta z:zTemp10
    stz z:zTemp11
    bsr RichterLadderCheck_at2
    bne RichterLadderCheck_at0
    jsr FrameOverlay8006
    and #$01
    bne RichterLadderCheck_at0
    bra RichterLadderCheck_at3
RichterLadderCheck_at5:
    bsr RichterLadderCheck_at2
    bne RichterLadderCheck_at0
    jsr FrameOverlay8006
    and #$01
    bne RichterLadderCheck_at4
RichterLadderCheck_at3:
    lda #$08
    clc
    adc z:zTemp10
    sta z:zTemp10
    bcs RichterLadderCheck_at0
    bra RichterLadderCheck_at5
RichterLadderCheck_at4:
    lda #$F8
    clc
    adc z:zTemp10
    sta z:zTemp10
    lda #$FF
    adc z:zTemp11
    sta z:zTemp11
    bsr RichterLadderCheck_at2
    jsr FrameOverlay8006
    and #$05
    beq RichterLadderCheck_at1
    bra RichterLadderCheck_at0
RichterLadderCheck_at1:
    sec
    rts
RichterLadderCheck_at0:
    clc
    rts
RichterLadderCheck_at2:
    lda a:OBJ_X_LO
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_Y_LO
    clc
    adc z:zTemp10
    sta z:zTemp02
    lda a:OBJ_Y_HI
    adc z:zTemp11
    sta z:zTemp03
    rts

; @routine 01BC:241 RichterLedgeCheck
; @run 6D:BA41
; WHAT IT DOES: Says whether a low or a high column in front of him gives him a ledge to act
; on, unless a push holds him. zTemp01 says which: 0 low, 1 high.
; HOW: RichterLowColumnsBlocked with down held, RichterColumnsBlocked with up held; bit $20 of
; the tile counts whatever the pad.
; Entry: nothing. Exit: C the answer, with what the last step left.
RichterLedgeCheck:
    tst #$01,z:zPlayerPush
    bne RichterLedgeCheck_at0
    bsr RichterLedgeCheck_at1
    bcs RichterLedgeCheck_at2
    bsr RichterLedgeCheck_at3
    bcs RichterLedgeCheck_at2
RichterLedgeCheck_at0:
    clc
    rts
RichterLedgeCheck_at1:
    jsr RichterLowColumnsBlocked
    beq RichterLedgeCheck_at0
    and #$20
    beq RichterLedgeCheck_at4
    cpx #$02
    bne RichterLedgeCheck_at5
RichterLedgeCheck_at4:
    bbr4 zPadHeld,RichterLedgeCheck_at0
RichterLedgeCheck_at5:
    stz z:zTemp01
    sec
    rts
RichterLedgeCheck_at3:
    jsr RichterColumnsBlocked
    beq RichterLedgeCheck_at0
    and #$20
    beq RichterLedgeCheck_at6
    cpx #$02
    bne RichterLedgeCheck_at7
RichterLedgeCheck_at6:
    bbr6 zPadHeld,RichterLedgeCheck_at0
RichterLedgeCheck_at7:
    lda #$01
    sta z:zTemp01
RichterLedgeCheck_at2:
    sec
    rts

; @routine 01BC:27A RichterLandOnFloor
; @run 6D:BA7A
; WHAT IT DOES: If there is a floor under him, lifts him out of it by how deep the column read,
; and not above the top of the map.
; HOW: RichterFloorCheck, then the depth in RichterColumnRow, which is cleared.
; Entry: nothing. Exit: C from the lift, or clear with no floor. A and N, Z from the last load.
RichterLandOnFloor:
    jsr RichterFloorCheck
    bcs RichterLandOnFloor_at0
    rts
RichterLandOnFloor_at0:
    stz z:zTemp10
    lda a:RichterColumnRow,x
    bpl RichterLandOnFloor_at1
    dec z:zTemp10
RichterLandOnFloor_at1:
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    sbc z:zTemp10
    sta a:OBJ_Y_HI
    bmi RichterLandOnFloor_at2
    stz a:RichterColumnRow,x
    rts
RichterLandOnFloor_at2:
    lda #$02
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:RichterColumnRow,x
    rts

; @routine 01BC:2AD RichterColumnsBlocked
; @run 6D:BAAD
; WHAT IT DOES: Says whether either of the first two columns read holds a blocking tile.
; HOW: Bits $24 of RichterColumnHit.
; Entry: nothing. Exit: at one, A those bits, X the column; else A 0, X 2, C 1, Z 1. V and,
; at one, C kept. Y kept.
RichterColumnsBlocked:
    clx
RichterColumnsBlocked_at1:
    lda a:RichterColumnHit,x
    and #$24
    beq RichterColumnsBlocked_at0
    rts
RichterColumnsBlocked_at0:
    inx
    cpx #$02
    bne RichterColumnsBlocked_at1
    rts

; @routine 01BC:2C2 RichterFloorCheck
; @run 6D:BAC2
; WHAT IT DOES: Says whether the columns read have a floor under Richter, and at which: on
; odd screens across, always at the first, with its depth cleared. A push that was already
; carrying him counts as floor too.
; HOW: Bits $81 of the first three columns, but bit 7 alone counts only for the first.
; Entry: nothing. Exit: C the answer, X the column. A, N, Z and V as the path left them.
RichterFloorCheck:
    lda a:OBJ_X_HI
    and #$01
    bne $BABC
    clx
RichterFloorCheck_at1:
    lda a:RichterColumnHit,x
    and #$81
    bne RichterFloorCheck_at0
RichterFloorCheck_at4:
    inx
    cpx #$03
    bne RichterFloorCheck_at1
    tst #$01,z:zPlayerPush
    bne RichterFloorCheck_at2
    rmb0 z:zPlayerPush
    rmb5 z:zPlayerPush
    clc
    rts
RichterFloorCheck_at0:
    bpl RichterFloorCheck_at3
    cpx #$00
    bne RichterFloorCheck_at4
RichterFloorCheck_at3:
    rmb0 z:zPlayerPush
    lda z:zFieldCA
    bpl RichterFloorCheck_at5
RichterFloorCheck_at6:
    rmb5 z:zPlayerPush
    sec
    rts
RichterFloorCheck_at5:
    lda z:zPushYHi
    bpl RichterFloorCheck_at6
    sec
    rts
RichterFloorCheck_at2:
    clx
    lda z:zFieldCA
    sta a:RichterColumnRow,x
    sec
    rts

; @routine 01BC:2FF RichterLowColumnsBlocked
; @run 6D:BAFF
; WHAT IT DOES: Reads the two columns lowest under Richter and says whether either blocks.
; HOW: Each is read $12 below him, and bits $24 of the answer tell.
; Entry: nothing. Exit: at one, A those bits, X the column; else A 0, X 2, C 1, Z 1. Y and V
; from the last read.
RichterLowColumnsBlocked:
    clx
RichterLowColumnsBlocked_at1:
    jsr RichterColumnOffset
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$12
    jsr RichterReadBelow
    and #$24
    beq RichterLowColumnsBlocked_at0
    rts
RichterLowColumnsBlocked_at0:
    inx
    cpx #$02
    bne RichterLowColumnsBlocked_at1
    rts

; @routine 01BC:31D RichterReadColumnsHigh
; @run 6D:BB1D
; WHAT IT DOES: Reads the five columns of terrain at his waist.
; HOW: As RichterReadColumns, $0B below his position.
; Entry and exit: as RichterReadColumns.
RichterReadColumnsHigh:
    lda #$0B
    sta z:zTemp10
    bra RichterReadColumns_at0

; @routine 01BC:323 RichterReadColumns
; @run 6D:BB23
; WHAT IT DOES: Reads the five columns of terrain under Richter, for what the floor checks
; look at: the step the overlay makes after he has moved.
; HOW: Each column's answer, row and X go to RichterColumnHit, RichterColumnRow and
; RichterColumnX.
; Entry: nothing. Exit: A the last column's X, X 5, N 0, Z 1, C 1, Y and V from the last read,
; or its row's add if that passed the screen. Clobbers zTemp00-03, zTemp10.
RichterReadColumns:
    lda #$16
    sta z:zTemp10
RichterReadColumns_at0:
    clx
RichterReadColumns_at1:
    jsr RichterColumnOffset
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc z:zTemp10
    jsr RichterReadBelow
    sta a:RichterColumnHit,x
    lda z:zTemp02
    sta a:RichterColumnRow,x
    lda z:zTemp00
    sta a:RichterColumnX,x
    inx
    cpx #$05
    bne RichterReadColumns_at1
    rts

; @routine 01BC:36E RichterWall
; @run 6D:BB6E
; WHAT IT DOES: Asks whether Richter runs into a wall on one side, and how deep: what pushes
; him that way counts as a wall, and otherwise up to six points down his side are read,
; fewer when he crouches or is in the air.
; HOW: The reach by side, then each column's height added to his position.
; Entry: Y side. Exit: A 2 with N 0, Z 0 at a wall, zTemp00 how deep; else Z set. Other
; registers and flags as the last step left them.
RichterWall:
    lda a:PlayerMayLeaveTop
    cmp #$01
    beq RichterWall_at0
    lda z:zPlayerPush
    and a:RichterWallPushBits,y
    bne $BB67
    lda a:RichterWallReach,y
    sta z:zTemp10
    lda a:OBJ_27A8
    cmp #$03
    beq RichterWall_at1
    cmp #$04
    beq RichterWall_at1
    cmp #$02
    beq RichterWall_at2
    clx
RichterWall_at3:
    lda a:OBJ_X_LO
    clc
    adc z:zTemp10
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_Y_LO
    clc
    adc a:RichterWallLift,x
    sta z:zTemp02
    lda a:OBJ_Y_HI
    adc a:RichterWallLiftHigh,x
    sta z:zTemp03
    jsr RichterReadIfOnPage
    and #$02
    bne RichterWall_at0
    inx
    cpx #$06
    bne RichterWall_at3
RichterWall_at0:
    rts
RichterWall_at1:
    ldx #$05
    bra RichterWall_at3
RichterWall_at2:
    ldx #$03
    bra RichterWall_at3

; @routine 01BC:3C1 RichterFloor
; @run 6D:BBC1
; WHAT IT DOES: Asks whether Richter stands on a floor, and lifts him out of it: by how deep
; the column read, but not above the top of the map. A push from below does not count.
; HOW: RichterReadColumns and RichterFloorCheck, then the column once more $0E below him.
; Entry: X kept unless a column was found. Exit: A 1 and N 0, Z 0 on a floor, else A 0, Z set.
; C from the lift, V as the last step left it. Clobbers zTemp00-03, zTemp10.
RichterFloor:
    lda a:PlayerMayLeaveTop
    bne RichterFloor_at0
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcc RichterFloor_at0
    jsr RichterColumnOffset
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0E
    jsr RichterReadBelow
    and #$81
    beq RichterFloor_at1
    bmi RichterFloor_at1
    bra RichterFloor_at0
RichterFloor_at1:
    stz z:zTemp10
    lda a:RichterColumnRow,x
    bpl RichterFloor_at2
    dec z:zTemp10
RichterFloor_at2:
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    sbc z:zTemp10
    sta a:OBJ_Y_HI
    bmi RichterFloor_at3
    tst #$20,z:zPlayerPush
    bne RichterFloor_at0
    lda #$01
    rts
RichterFloor_at0:
    lda #$00
    rts
RichterFloor_at3:
    lda #$02
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    lda #$01
    rts
