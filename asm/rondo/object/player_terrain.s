; @routine 01BA:22F RichterSolidUnderColumn
; @run 6D:AA2F
; WHAT IT DOES: Says whether one column of terrain, $0E below Richter, is solid to stand on:
; a tile with bit 7 set does not count.
; HOW: Bit 0 of the answer, unless bit 7 is set.
; WAITS: no.
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
; WAITS: no.
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
; WAITS: no.
; Entry: A row, C its add's carry: set while the row stays on the map. zTemp00 X. Exit: as the
; read, or A 0 with N and Z from the add.
RichterReadAbove:
    sta z:zTemp02
    cla
    adc #$FF
    sta z:zTemp03
    bne $B6E8
    bra $B6E3

; @routine 01BB:6DD RichterReadBelow
; @run 6D:B6DD
; WHAT IT DOES: Reads the terrain at a row on the first screen down: a row past it reads as
; nothing.
; HOW: The carry from the row's add says it passed; otherwise X and Y high bytes are zero.
; WAITS: no.
; Entry: A row, C its add's carry, zTemp00 X. Exit: as the read, or A 0 with N, Z, C and V kept.
RichterReadBelow:
    sta z:zTemp02
    bcs RichterReadBelow_at0
    stz z:zTemp03
    stz z:zTemp01
    jmp AttractEntry
RichterReadBelow_at0:
    cla
    rts

; @routine 01BB:6EA RichterReadOnScreen
; @run 6D:B6EA
; WHAT IT DOES: Reads the terrain at a row, the Y high byte taken from Richter's own.
; HOW: His Y high byte plus the carry from the row's add.
; WAITS: no.
; Entry: A row, C its add's carry, zTemp00 X. Exit: as the read.
RichterReadOnScreen:
    sta z:zTemp02
    cla
    adc a:OBJ_Y_HI
    sta z:zTemp03
    bra $B6E3

; @routine 01BB:6F4 RichterReadIfOnPage
; @run 6D:B6F4
; WHAT IT DOES: Reads the terrain at the point in zTemp00-03, unless the Y high byte just
; added is not zero.
; HOW: Tests what the caller's add left.
; WAITS: no.
; Entry: Z from that byte, zTemp00-03 the point. Exit: as the read, or A 0 with N and Z kept.
RichterReadIfOnPage:
    bne $B6E8
    jmp AttractEntry

; @routine 01BB:6F9 RichterColumnOffset
; @run 6D:B6F9
; WHAT IT DOES: Gives how far across a probe's column lies, by column and facing.
; HOW: A table of pairs, one for each way he faces.
; WAITS: no.
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
; WAITS: no.
; Entry: nothing. Exit: C the answer. All else kept.
RichterUpHeld:
    bbs6 zPadHeld,StartChainLink
    clc
    rts
    sec
    rts

; @routine 01BC:06F RichterJumpOrClimb
; @run 6D:B86F
; WHAT IT DOES: Says whether jump was pressed and he may act on it: with up held, only if there
; is a ladder to take.
; HOW: RichterJumpPressed, then RichterLadderCheck when up is held; the latched buttons clear.
; WAITS: no.
; Entry: nothing. Exit: C the answer, with what the last step left.
RichterJumpOrClimb:
    bsr RichterJumpPressed
    bcc $B881
    stz a:PadForcedPressB
    tst #$40,z:zPadHeld
    beq $B883
    jmp RichterLadderCheck

; @routine 01BC:07E RichterJumpPressed
; @run 6D:B87E
; WHAT IT DOES: Says whether the jump button was newly pressed.
; HOW: Bit 0 of zPadPressed.
; WAITS: no.
; Entry: nothing. Exit: C the answer. All else kept.
RichterJumpPressed:
    bbs0 zPadPressed,RichterJumpPressed_at0
    clc
    rts
RichterJumpPressed_at0:
    sec
    rts

; @routine 01BC:085 RichterHasNoFooting
; @run 6D:B885
; WHAT IT DOES: Says whether Richter has nothing to stand on: no floor found, or the column
; found is not solid under him.
; HOW: RichterFloorCheck, then RichterSolidUnderColumn.
; WAITS: no.
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
; WAITS: no.
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
    bsr RichterLadderPoint
    bne RichterLadderCheck_at0
    jsr AttractEntry
    and #$01
    bne RichterLadderCheck_at0
    bra RichterLadderCheck_at2
RichterLadderCheck_at4:
    bsr RichterLadderPoint
    bne RichterLadderCheck_at0
    jsr AttractEntry
    and #$01
    bne RichterLadderCheck_at3
RichterLadderCheck_at2:
    lda #$08
    clc
    adc z:zTemp10
    sta z:zTemp10
    bcs RichterLadderCheck_at0
    bra RichterLadderCheck_at4
RichterLadderCheck_at3:
    lda #$F8
    clc
    adc z:zTemp10
    sta z:zTemp10
    lda #$FF
    adc z:zTemp11
    sta z:zTemp11
    bsr RichterLadderPoint
    jsr AttractEntry
    and #$05
    beq RichterLadderCheck_at1
    bra RichterLadderCheck_at0
RichterLadderCheck_at1:
    sec
    rts
RichterLadderCheck_at0:
    clc
    rts

; @routine 01BC:0F2 RichterLadderPoint
; @run 6D:B8F2
; WHAT IT DOES: Sets the point the ladder check reads: his X, and his Y plus the offset in zTemp10-11, into zTemp00-03.
; HOW: Two adds with the carry between them.
; WAITS: no.
; Entry: zTemp10-11 the offset. Exit: A the Y high byte of the point, N, Z, C and V from its add. X and Y kept.
RichterLadderPoint:
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
; WAITS: no.
; Entry: nothing. Exit: C the answer, with what the last step left.
RichterLedgeCheck:
    tst #$01,z:zPlayerPush
    bne RichterLedgeCheck_at0
    bsr RichterLedgeLow
    bcs $BA78
    bsr RichterLedgeHigh
    bcs $BA78
RichterLedgeCheck_at0:
    clc
    rts

; @routine 01BC:250 RichterLedgeLow
; @run 6D:BA50
; WHAT IT DOES: The ledge check's low half: the two low columns read; a column that blocks counts when its bit $20 is set or
; down is held, and then zTemp01 is cleared.
; HOW: RichterLowColumnsBlocked, the answer's bit $20, bit 4 of zPadHeld.
; WAITS: no.
; Entry: nothing. Exit: C set at a ledge, A the bit looked at, X the column; else C clear (A 0, X 2 when no column blocks).
RichterLedgeLow:
    jsr RichterLowColumnsBlocked
    beq $BA4E
    and #$20
    beq RichterLedgeLow_at0
    cpx #$02
    bne RichterLedgeLow_at1
RichterLedgeLow_at0:
    bbr4 zPadHeld,$BA4E
RichterLedgeLow_at1:
    stz z:zTemp01
    sec
    rts

; @routine 01BC:264 RichterLedgeHigh
; @run 6D:BA64
; WHAT IT DOES: The ledge check's high half: the first two columns read; a column that blocks counts when its bit $20 is set or
; up is held, and then zTemp01 is set to 1.
; HOW: RichterColumnsBlocked, the answer's bit $20, bit 6 of zPadHeld.
; WAITS: no.
; Entry: nothing. Exit: C set at a ledge with A 1, X the column; else C clear (A the bit looked at, X 2 when no column blocks).
RichterLedgeHigh:
    jsr RichterColumnsBlocked
    beq $BA4E
    and #$20
    beq RichterLedgeHigh_at0
    cpx #$02
    bne RichterLedgeHigh_at1
RichterLedgeHigh_at0:
    bbr6 zPadHeld,$BA4E
RichterLedgeHigh_at1:
    lda #$01
    sta z:zTemp01
    sec
    rts

; @routine 01BC:27A RichterLandOnFloor
; @run 6D:BA7A
; WHAT IT DOES: If there is a floor under him, lifts him out of it.
; HOW: RichterFloorCheck, then runs on into RichterLiftByColumn.
; WAITS: no.
; Entry: nothing. Exit: as RichterLiftByColumn, or as RichterFloorCheck with no floor.
RichterLandOnFloor:
    jsr RichterFloorCheck
    bcs RichterLiftByColumn
    rts

; @routine 01BC:280 RichterLiftByColumn
; @run 6D:BA80
; WHAT IT DOES: Lifts him out of the floor by how deep this column read, and not above the top
; of the map.
; HOW: The depth in RichterColumnRow, signed, which is cleared.
; WAITS: no.
; Entry: X column. Exit: A his Y high byte, or 2 at the top; N and Z from it; C and V from the
; subtract. X and Y kept. Clobbers zTemp10.
RichterLiftByColumn:
    stz z:zTemp10
    lda a:RichterColumnRow,x
    bpl RichterLiftByColumn_at0
    dec z:zTemp10
RichterLiftByColumn_at0:
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    sbc z:zTemp10
    sta a:OBJ_Y_HI
    bmi RichterLiftByColumn_at1
    stz a:RichterColumnRow,x
    rts
RichterLiftByColumn_at1:
    lda #$02
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:RichterColumnRow,x
    rts

; @routine 01BC:2AD RichterColumnsBlocked
; @run 6D:BAAD
; WHAT IT DOES: Says whether either of the first two columns read holds a blocking tile.
; HOW: Bits $24 of RichterColumnHit.
; WAITS: no.
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

; @routine 01BC:2BC RichterFloorOddScreen
; @run 6D:BABC
; WHAT IT DOES: The floor check's answer on an odd screen across: a floor at the first column,
; its depth cleared, as the floor check finds one.
; HOW: Sits before RichterFloorCheck, which branches back to it, and goes on into its found
; floor's end.
; WAITS: no.
; Entry: nothing. Exit: as RichterFloorCheck finding a floor at column 0.
RichterFloorOddScreen:
    clx
    stz a:RichterColumnRow,x
    bra $BAE1

; @routine 01BC:2C2 RichterFloorCheck
; @run 6D:BAC2
; WHAT IT DOES: Says whether the columns read have a floor under Richter, and at which: on
; odd screens across, always at the first, with its depth cleared. A push that was already
; carrying him counts as floor too.
; HOW: Bits $81 of the first three columns, but bit 7 alone counts only for the first.
; WAITS: no.
; Entry: nothing. Exit: C the answer, X the column. A, N, Z and V as the path left them.
RichterFloorCheck:
    lda a:OBJ_X_HI
    and #$01
    bne RichterFloorOddScreen
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
; WAITS: no.
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
; WAITS: no.
; Entry and exit: as RichterReadColumns.
RichterReadColumnsHigh:
    lda #$0B
    sta z:zTemp10
    bra $BB27

; @routine 01BC:323 RichterReadColumns
; @run 6D:BB23
; WHAT IT DOES: Reads the five columns of terrain under Richter, for what the floor checks
; look at: the step the overlay makes after he has moved.
; HOW: Each column's answer, row and X go to RichterColumnHit, RichterColumnRow and
; RichterColumnX.
; WAITS: no.
; Entry: nothing. Exit: A the last column's X, X 5, N 0, Z 1, C 1, Y and V from the last read,
; or its row's add if that passed the screen. Clobbers zTemp00-03, zTemp10.
RichterReadColumns:
    lda #$16
    sta z:zTemp10
    clx
RichterReadColumns_at0:
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
    bne RichterReadColumns_at0
    rts

; @routine 01BC:367 RichterWallPushed
; @run 6D:BB67
; WHAT IT DOES: The wall probe's answer when a push holds him against that side: a wall as
; deep as the push says.
; HOW: zPlayerPushDepth to zTemp00. Sits before RichterWall, which branches back to it.
; WAITS: no.
; Entry: nothing. Exit: A 2, N and Z clear. X, Y, C and V kept.
RichterWallPushed:
    lda z:zPlayerPushDepth
    sta z:zTemp00
    lda #$02
    rts

; @routine 01BC:36E RichterWall
; @run 6D:BB6E
; WHAT IT DOES: Asks whether Richter runs into a wall on one side, and how deep: what pushes
; him that way counts as a wall, and otherwise up to six points down his side are read,
; fewer when he crouches or is in the air.
; HOW: The reach by side, then each column's height added to his position.
; WAITS: no.
; Entry: Y side. Exit: A 2 with N 0, Z 0 at a wall, zTemp00 how deep; else Z set. Other
; registers and flags as the last step left them.
RichterWall:
    lda a:PlayerMayLeaveTop
    cmp #$01
    beq WaitThenStepListWithSoundAndBounce
    lda z:zPlayerPush
    and a:RichterWallPushBits,y
    bne RichterWallPushed
    lda a:RichterWallReach,y
    sta z:zTemp10
    lda a:OBJ_27A8
    cmp #$03
    beq RichterWall_at0
    cmp #$04
    beq RichterWall_at0
    cmp #$02
    beq RichterWall_at1
    clx
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
    bne WaitThenStepListWithSoundAndBounce
    inx
    cpx #$06
    bne LoadRecordLaunchSideways
    rts
RichterWall_at0:
    ldx #$05
    bra LoadRecordLaunchSideways
RichterWall_at1:
    ldx #$03
    bra LoadRecordLaunchSideways

; @routine 01BC:3C1 RichterFloor
; @run 6D:BBC1
; WHAT IT DOES: Asks whether Richter stands on a floor, and lifts him out of it: by how deep
; the column read, but not above the top of the map. A push from below does not count.
; HOW: RichterReadColumns and RichterFloorCheck, then the column once more $0E below him.
; WAITS: no.
; Entry: X kept unless a column was found. Exit: A 1 and N 0, Z 0 on a floor, else A 0, Z set.
; C from the lift, V as the last step left it. Clobbers zTemp00-03, zTemp10.
RichterFloor:
    lda a:PlayerMayLeaveTop
    bne JumpByPhaseTable025C40D
    jsr RichterReadColumns
    jsr RichterFloorCheck
    bcc JumpByPhaseTable025C40D
    jsr RichterColumnOffset
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0E
    jsr RichterReadBelow
    and #$81
    beq RichterFloor_at0
    bmi RichterFloor_at0
    bra JumpByPhaseTable025C40D
RichterFloor_at0:
    stz z:zTemp10
    lda a:RichterColumnRow,x
    bpl RichterFloor_at1
    dec z:zTemp10
RichterFloor_at1:
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    sbc z:zTemp10
    sta a:OBJ_Y_HI
    bmi RichterFloor_at2
    tst #$20,z:zPlayerPush
    bne JumpByPhaseTable025C40D
    lda #$01
    rts
    lda #$00
    rts
RichterFloor_at2:
    lda #$02
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    lda #$01
    rts

; @routine 01E0:20E MariaReadBelow
; @run 6D:BA0E
; WHAT IT DOES: RichterReadBelow in Maria's overlay (01DD): reads the terrain at a row on the first screen down; a row
; past it reads as nothing.
; HOW: The same instructions as RichterReadBelow.
; WAITS: no.
; Entry, exit: as RichterReadBelow.
MariaReadBelow:
    sta z:zTemp02
    bcs MariaReadBelow_at0
    stz z:zTemp03
    stz z:zTemp01
    jmp AttractEntry
MariaReadBelow_at0:
    cla
    rts

; @routine 01E0:220 MariaColumnOffset
; @run 6D:BA20
; WHAT IT DOES: RichterColumnOffset in Maria's overlay: how far across a probe's column lies, by column and facing.
; HOW: The same instructions, with her overlay's table at $BE69.
; WAITS: no.
; Entry, exit: as RichterColumnOffset.
MariaColumnOffset:
    txa
    asl a
    clc
    adc a:OBJ_2AC0
    tay
    lda a:RichterColumnOffsets+$31C,y
    rts

; @routine 01E0:5A4 MariaLiftByColumn
; @run 6D:BDA4
; WHAT IT DOES: Lifts Maria out of the floor by how deep this column read; above the top of the map she is put at 0.
; HOW: Her Y less the column's depth in RichterColumnRow, signed. Unlike RichterLiftByColumn the depth is not
; cleared, and a lift past the top leaves her Y at 0, not 2.
; WAITS: no.
; Entry: X column. Exit: A her Y high byte, N and Z from it; C and V from the subtract. X and Y kept. Clobbers zTemp10.
MariaLiftByColumn:
    stz z:zTemp10
    lda a:RichterColumnRow,x
    bpl MariaLiftByColumn_at0
    dec z:zTemp10
MariaLiftByColumn_at0:
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    sbc z:zTemp10
    sta a:OBJ_Y_HI
    bmi MariaLiftByColumn_at1
    rts
MariaLiftByColumn_at1:
    stz a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    rts

; @routine 01E0:639 MariaReadColumnsHigh
; @run 6D:BE39
; WHAT IT DOES: RichterReadColumnsHigh in Maria's overlay: the five columns of terrain at her waist.
; HOW: As MariaReadColumns, $0B below her position.
; WAITS: no.
; Entry and exit: as RichterReadColumns.
MariaReadColumnsHigh:
    lda #$0B
    sta z:zTemp10
    bra MariaReadColumns_at1

; @routine 01E0:63F MariaReadColumns
; @run 6D:BE3F
; WHAT IT DOES: RichterReadColumns in Maria's overlay: the five columns of terrain under her, into RichterColumnHit,
; RichterColumnRow and RichterColumnX; the step her overlay makes after she has moved.
; HOW: The same instructions as RichterReadColumns, calling MariaColumnOffset and MariaReadBelow.
; WAITS: no.
; Entry, exit: as RichterReadColumns.
MariaReadColumns:
    lda #$16
    sta z:zTemp10
MariaReadColumns_at1:
    clx
MariaReadColumns_at0:
    jsr MariaColumnOffset
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc z:zTemp10
    jsr MariaReadBelow
    sta a:RichterColumnHit,x
    lda z:zTemp02
    sta a:RichterColumnRow,x
    lda z:zTemp00
    sta a:RichterColumnX,x
    inx
    cpx #$05
    bne MariaReadColumns_at0
    rts

; @routine 01E0:21B MariaReadIfOnPage
; @run 6D:BA1B
; WHAT IT DOES: RichterReadIfOnPage in Maria's overlay: reads the terrain at the point in zTemp00-03, unless the Y high byte just
; added is not zero.
; HOW: The same instructions as RichterReadIfOnPage; its branch goes to the end of MariaReadBelow.
; WAITS: no.
; Entry, exit: as RichterReadIfOnPage.
MariaReadIfOnPage:
    bne MariaReadBelow_at0
    jmp AttractEntry

; @routine 01E0:5D8 MariaFloorOddScreen
; @run 6D:BDD8
; WHAT IT DOES: RichterFloorOddScreen in Maria's overlay: a floor at the first column on an odd screen across.
; HOW: The same instructions. Sits before MariaFloorCheck, which branches back to it.
; WAITS: no.
; Entry, exit: as RichterFloorOddScreen.
MariaFloorOddScreen:
    clx
    stz a:RichterColumnRow,x
    bra MariaFloorCheck_at0

; @routine 01E0:5DE MariaFloorCheck
; @run 6D:BDDE
; WHAT IT DOES: RichterFloorCheck in Maria's overlay: whether the columns read have a floor under her, and at which.
; HOW: The same instructions, branching back to MariaFloorOddScreen on an odd screen.
; WAITS: no.
; Entry, exit: as RichterFloorCheck.
MariaFloorCheck:
    lda a:OBJ_X_HI
    and #$01
    bne MariaFloorOddScreen
    clx
MariaFloorCheck_at1:
    lda a:RichterColumnHit,x
    and #$81
    bne MariaFloorCheck_at0
MariaFloorCheck_at4:
    inx
    cpx #$03
    bne MariaFloorCheck_at1
    tst #$01,z:zPlayerPush
    bne MariaFloorCheck_at2
    rmb0 z:zPlayerPush
    rmb5 z:zPlayerPush
    clc
    rts
MariaFloorCheck_at0:
    bpl MariaFloorCheck_at3
    cpx #$00
    bne MariaFloorCheck_at4
MariaFloorCheck_at3:
    rmb0 z:zPlayerPush
    lda z:zFieldCA
    bpl MariaFloorCheck_at5
MariaFloorCheck_at6:
    rmb5 z:zPlayerPush
    sec
    rts
MariaFloorCheck_at5:
    lda z:zPushYHi
    bpl MariaFloorCheck_at6
    sec
    rts
MariaFloorCheck_at2:
    clx
    lda z:zFieldCA
    sta a:RichterColumnRow,x
    sec
    rts

; @routine 01E0:683 MariaWallPushed
; @run 6D:BE83
; WHAT IT DOES: RichterWallPushed in Maria's overlay: the wall probe's answer when a push holds her against that side.
; HOW: The same instructions as RichterWallPushed. Sits before MariaWall, which branches back to it.
; WAITS: no.
; Entry, exit: as RichterWallPushed.
MariaWallPushed:
    lda z:zPlayerPushDepth
    sta z:zTemp00
    lda #$02
    rts

; @routine 01E0:68A MariaWall
; @run 6D:BE8A
; WHAT IT DOES: RichterWall in Maria's overlay: whether she runs into a wall on one side, and how deep. Her states
; choose the first column otherwise: 3, 4, $0E and $0F start at 5, $0D at 2, the rest at 0.
; HOW: RichterWall's instructions with her overlay's tables, MariaWallPushed and MariaReadIfOnPage, and more
; compares of the state.
; WAITS: no.
; Entry, exit: as RichterWall.
MariaWall:
    lda a:PlayerMayLeaveTop
    cmp #$01
    beq MariaWall_at2
    lda z:zPlayerPush
    and a:RichterWallPushBits+$31C,y
    bne MariaWallPushed
    lda a:RichterWallReach+$31C,y
    sta z:zTemp10
    lda a:OBJ_27A8
    cmp #$03
    beq MariaWall_at0
    cmp #$04
    beq MariaWall_at0
    cmp #$0D
    beq MariaWall_at1
    cmp #$0F
    beq MariaWall_at0
    cmp #$0E
    beq MariaWall_at0
    clx
MariaWall_at3:
    lda a:OBJ_X_LO
    clc
    adc z:zTemp10
    sta z:zTemp00
    stz z:zTemp01
    lda a:OBJ_Y_LO
    clc
    adc a:RichterWallLift+$31C,x
    sta z:zTemp02
    lda a:OBJ_Y_HI
    adc a:RichterWallLiftHigh+$31C,x
    sta z:zTemp03
    jsr MariaReadIfOnPage
    and #$02
    bne MariaWall_at2
    inx
    cpx #$06
    bne MariaWall_at3
MariaWall_at2:
    rts
MariaWall_at0:
    ldx #$05
    bra MariaWall_at3
MariaWall_at1:
    ldx #$02
    bra MariaWall_at3

; @routine 01E0:6E5 MariaFloor
; @run 6D:BEE5
; WHAT IT DOES: RichterFloor in Maria's overlay: whether she stands on a floor, and lifts her out of it; a lift past
; the top of the map leaves her Y at 0, not 2.
; HOW: RichterFloor's instructions calling MariaReadColumns, MariaFloorCheck, MariaColumnOffset and MariaReadBelow.
; WAITS: no.
; Entry, exit: as RichterFloor.
MariaFloor:
    lda a:PlayerMayLeaveTop
    bne MariaFloor_at3
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaFloor_at3
    jsr MariaColumnOffset
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0E
    jsr MariaReadBelow
    and #$81
    beq MariaFloor_at0
    bmi MariaFloor_at0
    bra MariaFloor_at3
MariaFloor_at0:
    stz z:zTemp10
    lda a:RichterColumnRow,x
    bpl MariaFloor_at1
    dec z:zTemp10
MariaFloor_at1:
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    sbc z:zTemp10
    sta a:OBJ_Y_HI
    bmi MariaFloor_at2
    tst #$20,z:zPlayerPush
    bne MariaFloor_at3
    lda #$01
    rts
MariaFloor_at3:
    lda #$00
    rts
MariaFloor_at2:
    stz a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    lda #$01
    rts

; @routine 01E0:389 MariaJumpPressed
; @run 6D:BB89
; WHAT IT DOES: RichterJumpPressed in Maria's player overlay (01DD), the same instructions at her address.
; HOW: RichterJumpPressed's body.
; WAITS: no.
; Entry, exit: as RichterJumpPressed.
MariaJumpPressed:
    bbs0 zPadPressed,MariaJumpPressed_at0
    clc
    rts
MariaJumpPressed_at0:
    sec
    rts

; @routine 01DE:1C9 MariaSolidUnderColumn
; @run 6D:A9C9
; WHAT IT DOES: RichterSolidUnderColumn in Maria's player overlay (01DD).
; HOW: The same instructions, calling MariaColumnOffset and MariaReadBelow.
; WAITS: no.
; Entry, exit: as RichterSolidUnderColumn.
MariaSolidUnderColumn:
    jsr MariaColumnOffset
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0E
    jsr MariaReadBelow
    and #$81
    bpl MariaSolidUnderColumn_at0
    lda #$00
MariaSolidUnderColumn_at0:
    rts

; @routine 01DE:1E2 MariaReadUnderFeet
; @run 6D:A9E2
; WHAT IT DOES: RichterReadUnderFeet in Maria's player overlay (01DD).
; HOW: The same instructions, its tail jump to MariaReadBelow.
; WAITS: no.
; Entry, exit: as RichterReadUnderFeet.
MariaReadUnderFeet:
    lda a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$1E
    jmp MariaReadBelow
