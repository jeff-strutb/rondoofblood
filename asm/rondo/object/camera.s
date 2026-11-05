; @routine 0047:557 PlayerDistanceSum
; @run 6A:6557
; WHAT IT DOES: Takes the distance to the player and adds and subtracts fifteen bytes
; of the object fields to it. What the sum means is not known yet.
; HOW: A carry chain through all of them, set once before the last.
; Entry: X object. Exit: A the sum, N, Z, C and V from the last subtract. X and Y kept.
PlayerDistanceSum:
    jsr VecDistanceXToPlayer
    adc a:$3418
    adc a:$3480
    adc a:$2CE1
    sbc a:$2C32
    sbc a:$2CE3
    adc a:$2BDC
    adc a:$2765
    sbc a:$2766
    sbc a:$2C37
    adc a:$2D00
    adc a:$2781
    sbc a:$2BFA
    sbc a:$2D03
    adc a:OBJ_FIELDS_2EE0
    sec
    sbc a:OBJ_2F90
    rts

; @routine 0047:589 SoundIfOnFirstScreen
; @run 6A:6589
; WHAT IT DOES: Plays a sound if the object is on the first screen across and down.
; HOW: The high bytes of its position ORed must be zero, then SoundCommand.
; Entry: A command, X object. Exit, not played: A kept, N and Z from it. Exit, played:
; as SoundCommand.
SoundIfOnFirstScreen:
    pha
    lda a:OBJ_X_HI,x
    ora a:OBJ_Y_HI,x
    bne SoundIfOnFirstScreen_at0
    pla
    jmp VecSoundCommand
SoundIfOnFirstScreen_at0:
    pla
    rts

; @routine 0047:598 StopPlayerX
; @run 6A:6598
; WHAT IT DOES: Stops the player moving along X.
; HOW: Zeroes the X speed.
; Entry: nothing. Exit: all registers and flags kept.
StopPlayerX:
    stz a:OBJ_VX_LO
    stz a:OBJ_VX_HI
    rts

; @routine 0047:59F MovePlayerX
; @run 6A:659F
; WHAT IT DOES: Moves the player by its X speed.
; HOW: Adds the 8.8 speed to position low and fraction, with no carry into the high
; byte: the player stays on the screen.
; Entry: nothing. Exit: A position low, N, Z, C and V from its add. X and Y kept.
MovePlayerX:
    lda a:OBJ_VX_LO
    clc
    adc a:OBJ_X_FRAC
    sta a:OBJ_X_FRAC
    lda a:OBJ_VX_HI
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    rts

; @routine 0047:5B3 MovePlayerY
; @run 6A:65B3
; WHAT IT DOES: Moves the player by its Y speed, and stops it at the top of the map if
; this section does not let it leave.
; HOW: A 16.8 add with the speed's sign in the high byte. A negative result with
; PlayerMayLeaveTop clear becomes zero.
; Entry: nothing. Exit: A the position's high byte, or PlayerMayLeaveTop if it was read;
; N and Z from A, C and V from the add. X and Y kept.
MovePlayerY:
    lda a:OBJ_VY_LO
    clc
    adc a:OBJ_Y_FRAC
    sta a:OBJ_Y_FRAC
    lda a:OBJ_VY_HI
    adc a:OBJ_Y_LO
    sta a:OBJ_Y_LO
    lda a:OBJ_VY_HI
    bmi MovePlayerY_at0
    cla
    bra MovePlayerY_at1
MovePlayerY_at0:
    lda #$FF
MovePlayerY_at1:
    adc a:OBJ_Y_HI
    sta a:OBJ_Y_HI
    bpl MovePlayerY_at2
    lda a:PlayerMayLeaveTop
    bne MovePlayerY_at2
    stz a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    stz a:OBJ_Y_FRAC
MovePlayerY_at2:
    rts

; @routine 0047:5E7 ScrollStepXFromPlayer
; @run 6A:65E7
; WHAT IT DOES: Takes the player's X speed as the frame's scroll step.
; HOW: Copies the word.
; Entry: nothing. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
ScrollStepXFromPlayer:
    lda a:OBJ_VX_LO
    sta a:ScrollStepXLo
    lda a:OBJ_VX_HI
    sta a:ScrollStepXHi
    rts

; @routine 0047:5F4 MovePlayerYIfMoving
; @run 6A:65F4
; WHAT IT DOES: Moves the player along Y if it has a Y speed, and when that speed is
; downward asks the player overlay about the floor.
; HOW: MovePlayerY, then the overlay's floor entry on the way down.
; Entry: nothing. Exit, not moving: A zero, Z set. Exit, moving up: A the speed's high
; byte, C and V from MovePlayerY. Exit, moving down: as the overlay leaves them.
MovePlayerYIfMoving:
    lda a:OBJ_VY_LO
    ora a:OBJ_VY_HI
    beq MovePlayerYIfMoving_at0
    jsr MovePlayerY
    lda a:OBJ_VY_HI
    bmi MovePlayerYIfMoving_at0
    jmp PlayerOvlFloor
MovePlayerYIfMoving_at0:
    rts

; @routine 0047:608 PlayerAndCameraFrame
; @run 6A:6608
; WHAT IT DOES: One frame of the player's movement and the camera. Adds what is pushing
; the player (a moving platform, say) to its speed for the frame, runs the section's
; camera mode, puts the speed back, runs the overlay's step after moving and clears the
; push.
; HOW: The speed is kept on the stack. Bit 4 and bit 5 of zPlayerPush add the push to X
; and Y speed; bit 5 also takes the Y fraction from PlayerAnchorObject. Bit 3 moves the
; player down the screen against the push's Y and scrolls by it, in place of the camera
; mode.
; Entry: nothing. Exit: A and Y as the overlay leaves them, V too, X 7, N 0, Z 1, C 1.
PlayerAndCameraFrame:
    lda a:OBJ_VX_LO
    pha
    lda a:OBJ_VX_HI
    pha
    lda a:OBJ_VY_LO
    pha
    lda a:OBJ_VY_HI
    pha
    bbr4 zPlayerPush,PlayerAndCameraFrame_at0
    lda a:OBJ_VX_LO
    clc
    adc z:zPushXLo
    sta a:OBJ_VX_LO
    lda a:OBJ_VX_HI
    adc z:zPushXHi
    sta a:OBJ_VX_HI
PlayerAndCameraFrame_at0:
    bbr5 zPlayerPush,PlayerAndCameraFrame_at1
    lda a:OBJ_VY_LO
    clc
    adc z:zPushYLo
    sta a:OBJ_VY_LO
    lda a:OBJ_VY_HI
    adc z:zPushYHi
    sta a:OBJ_VY_HI
    ldx a:PlayerAnchorObject
    lda a:OBJ_Y_FRAC,x
    sta a:ScrollYFrac
    sta a:OBJ_Y_FRAC
PlayerAndCameraFrame_at1:
    bbr3 zPlayerPush,PlayerAndCameraFrame_at2
    stz a:ScrollStepYLo
    lda z:zPushYHi
    sta a:ScrollStepYHi
    eor #$FF
    inc a
    clc
    adc a:OBJ_Y_LO
    sta a:OBJ_Y_LO
    bra PlayerAndCameraFrame_at3
PlayerAndCameraFrame_at2:
    bsr PlayerAndCameraFrame_at4
PlayerAndCameraFrame_at3:
    pla
    sta a:OBJ_VY_HI
    pla
    sta a:OBJ_VY_LO
    pla
    sta a:OBJ_VX_HI
    pla
    sta a:OBJ_VX_LO
    jsr PlayerOvlMoved
    clx
PlayerAndCameraFrame_at5:
    stz z:zPlayerPush,x
    inx
    cpx #$07
    bne PlayerAndCameraFrame_at5
    rts
PlayerAndCameraFrame_at4:
    lda a:CameraMode
    asl a
    tax
    jmp (CameraModeHandlers,x)

; @routine 0047:6C5 CameraModeCarried
; @run 6A:66C5
; WHAT IT DOES: The camera mode where the scroll follows what carries the player, not
; the player: moves the player on the screen, and takes the push as the scroll step
; unless bit 4 of zPlayerPush says it was already added to the speed.
; HOW: Zeroes both steps, MovePlayerYIfMoving, PlayerXOnScreen, copies the push.
; Entry: cpu flags current. Exit: A and N, Z the last read; C and V as the moves left
; them; X and Y as the overlay left them.
CameraModeCarried:
    stz a:ScrollStepXLo
    stz a:ScrollStepXHi
    stz a:ScrollStepYLo
    stz a:ScrollStepYHi
    jsr MovePlayerYIfMoving
    jsr PlayerXOnScreen
    bbs4 zPlayerPush,CameraModeCarried_at0
    lda z:zPushXLo
    sta a:ScrollStepXLo
    lda z:zPushXHi
    sta a:ScrollStepXHi
    lda z:zPushYLo
    sta a:ScrollStepYLo
    lda z:zPushYHi
    sta a:ScrollStepYHi
CameraModeCarried_at0:
    rts

; @routine 0047:6EF PlayerNextX
; @run 6A:66EF
; WHAT IT DOES: Works out where the player's speed would put it along X, without moving
; it.
; HOW: The 8.8 sum, left in zTemp04-05.
; Entry: nothing. Exit: A zTemp05, N, Z, C, V from its add. X and Y kept.
PlayerNextX:
    lda a:OBJ_VX_LO
    clc
    adc a:OBJ_X_FRAC
    sta z:zTemp04
    lda a:OBJ_VX_HI
    adc a:OBJ_X_LO
    sta z:zTemp05
    rts

; @routine 0047:701 CheckPlayerAtLevelEnd
; @run 6A:6701
; WHAT IT DOES: When the scroll is at the level's end and the player has walked into
; the last stretch of the screen, $C0 to $EB, sets the player's two exit fields.
; HOW: Compares the scroll's high byte with its limit, and the position with the range.
; Entry: nothing. Exit: A and flags as the last compare or load left them. X, Y, V kept.
CheckPlayerAtLevelEnd:
    lda a:ScrollXHiScreen
    cmp a:ScrollXMaxHi
    bne CheckPlayerAtLevelEnd_at0
    lda a:OBJ_X_LO
    cmp #$C0
    bcc CheckPlayerAtLevelEnd_at0
    lda a:OBJ_X_LO
    cmp #$EC
    bcs CheckPlayerAtLevelEnd_at0
    lda #$FF
    sta a:OBJ_3510
    lda #$20
    sta a:OBJ_3568
CheckPlayerAtLevelEnd_at0:
    rts

; @routine 0047:722 CameraModeLeftWall
; @run 6A:6722
; WHAT IT DOES: The camera mode for a section the scroll does not leave to the left:
; right as in CameraModeFollow, left moves the player on the screen alone. A wall to the
; left pushes the player right by a table on how deep it went.
; HOW: Follows the speed's sign, or bits 2 and 1 of zPlayerPush at no speed, then the
; overlay's wall entry with Y 1.
; Entry: cpu flags current. Exit: A and flags as the path taken left them; X and Y as
; the overlay left them or X the depth.
CameraModeLeftWall:
    jsr CheckPlayerAtLevelEnd
    jsr MovePlayerYIfMoving
    stz a:ScrollStepXLo
    stz a:ScrollStepXHi
    lda a:OBJ_VX_HI
    ora a:OBJ_VX_LO
    bne CameraModeLeftWall_at0
    tst #$04,z:zPlayerPush
    bne CameraModeLeftWall_at1
    tst #$02,z:zPlayerPush
    beq CameraModeLeftWall_at2
    jmp MovePlayerRightOrWall
CameraModeLeftWall_at2:
    rts
CameraModeLeftWall_at0:
    lda a:OBJ_VX_HI
    bmi CameraModeLeftWall_at1
    jmp MovePlayerRightOrWall
CameraModeLeftWall_at1:
    ldy #$01
    jsr PlayerOvlWall
    beq CameraModeLeftWall_at3
    lda a:OBJ_27A8
    cmp #$03
    bne CameraModeLeftWall_at4
    lda a:OBJ_VY_HI
    bpl CameraModeLeftWall_at5
    rts
CameraModeLeftWall_at5:
    jmp StopPlayerX
CameraModeLeftWall_at4:
    ldx z:zTemp00
    lda a:WallLeftPush,x
    clc
    adc a:OBJ_X_LO
    cmp #$F0
    bcs CameraModeLeftWall_at6
    sta a:OBJ_X_LO
    stz a:OBJ_X_FRAC
    jmp StopPlayerX
CameraModeLeftWall_at6:
    jmp PlayerAtRightEdge
CameraModeLeftWall_at3:
    jsr PlayerNextX
    cmp #$10
    bcs CameraModeLeftWall_at7
    jmp PlayerAtLeftEdge
CameraModeLeftWall_at7:
    bne CameraModeLeftWall_at8
    jmp PlayerAtLeftEdge
CameraModeLeftWall_at8:
    jmp PlayerXFromNextAlt

; @routine 0047:78E CameraModeFollow
; @run 6A:678E
; WHAT IT DOES: The usual camera mode: moves the player down or up, then left or right
; with the scroll following.
; HOW: Follows the speed's sign, or bits 2 and 1 of zPlayerPush at no speed.
; Entry: cpu flags current. Exit: A and flags as the path taken left them.
CameraModeFollow:
    jsr MovePlayerYIfMoving
    stz a:ScrollStepXLo
    stz a:ScrollStepXHi
    lda a:OBJ_VX_HI
    ora a:OBJ_VX_LO
    bne CameraModeFollow_at0
    tst #$04,z:zPlayerPush
    beq CameraModeFollow_at1
    jmp MovePlayerLeftOrWall
CameraModeFollow_at1:
    tst #$02,z:zPlayerPush
    beq CameraModeFollow_at2
    jmp MovePlayerRightOrWall
CameraModeFollow_at2:
    rts
CameraModeFollow_at0:
    lda a:OBJ_VX_HI
    bpl CameraModeFollow_at3
    jmp MovePlayerLeftOrWall
CameraModeFollow_at3:
    jmp MovePlayerRightOrWall

; @routine 0047:7BB PlayerWallRight
; @run 6A:67BB
; WHAT IT DOES: Asks the overlay about a wall to the right, and pushes the player back
; out of one: left by how deep it went, but not into another wall the overlay finds
; there, and not past the left edge.
; HOW: Y 0 on the wall entry; on a wall, zTemp00 is the depth. In player state 3 the
; player only stops, when not rising. The second question is Y 1 with the position moved
; and zPlayerPush clear; if there is a wall there too, the position goes back.
; Entry: cpu flags current. Exit, no wall: A and flags as the overlay left them, Z set.
; Exit, wall: A 1, N and Z from it.
PlayerWallRight:
    cly
    jsr PlayerOvlWall
    beq PlayerWallRight_at0
    lda a:OBJ_27A8
    cmp #$03
    bne PlayerWallRight_at1
    lda a:OBJ_VY_HI
    bmi PlayerWallRight_at2
    jsr StopPlayerX
    bra PlayerWallRight_at2
PlayerWallRight_at1:
    lda z:zTemp00
    eor #$FF
    inc a
    clc
    adc a:OBJ_X_LO
    cmp #$10
    bcc PlayerWallRight_at3
    ldx a:OBJ_X_LO
    phx
    sta a:OBJ_X_LO
    stz a:OBJ_X_FRAC
    jsr StopPlayerX
    stz z:zPlayerPush
    ldy #$01
    jsr PlayerOvlWall
    plx
    and #$FF
    beq PlayerWallRight_at2
    stx a:OBJ_X_LO
    bra PlayerWallRight_at2
PlayerWallRight_at3:
    jsr PlayerAtLeftEdge
PlayerWallRight_at2:
    lda #$01
PlayerWallRight_at0:
    rts

; @routine 0048:003 MovePlayerRightOrWall
; @run 6A:6803
; WHAT IT DOES: Moves the player right, unless there is a wall.
; HOW: PlayerWallRight, then MovePlayerRight.
; Entry: cpu flags current. Exit: as the one that ran last.
MovePlayerRightOrWall:
    jsr PlayerWallRight
    beq MovePlayerRightOrWall_at0
    rts
MovePlayerRightOrWall_at0:
    jmp MovePlayerRight

; @routine 0048:00C PlayerXFromNext
; @run 6A:680C
; WHAT IT DOES: Accepts the position PlayerNextX worked out.
; HOW: Copies zTemp04-05 to the position.
; Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
PlayerXFromNext:
    lda z:zTemp04
    sta a:OBJ_X_FRAC
    lda z:zTemp05
    sta a:OBJ_X_LO
    rts

; @routine 0048:017 MovePlayerRight
; @run 6A:6817
; WHAT IT DOES: Moves the player right, or the scroll under it. Before the middle, or
; with the scroll at the level's end, the player moves; past the middle, the scroll
; moves by what is left, and the player stays at the middle. At the level's end the
; scroll takes only what reaches the limit, and the player the rest.
; HOW: Follows the next position through NextPastMiddle and the limit compares.
; Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
; zTemp00-05.
MovePlayerRight:
    stz a:ScrollStepXLo
    stz a:ScrollStepXHi
    jsr PlayerNextX
    cmp #$F0
    bcc MovePlayerRight_at0
    jmp PlayerAtRightEdge
MovePlayerRight_at0:
    jsr NextPastMiddle
    bcc PlayerXFromNext
    lda a:ScrollXLo
    sta z:zTemp03
    lda a:ScrollXHiScreen
    jsr CompareScrollWithRightLimit
    beq PlayerXFromNext
    jsr NextPastMiddleAmount
    jsr PlayerXPastMiddle
    bcc MovePlayerRight_at1
    lda a:OBJ_VX_LO
    sta z:zTemp00
    clc
    adc a:ScrollXFrac
    sta z:zTemp02
    lda a:OBJ_VX_HI
    sta z:zTemp01
    adc a:ScrollXLo
    sta z:zTemp03
    cla
    adc a:ScrollXHiScreen
    jsr CompareScrollWithRightLimit
    bcc MovePlayerRight_at2
    bra MovePlayerRight_at3
MovePlayerRight_at1:
    lda z:zTemp00
    clc
    adc a:ScrollXFrac
    sta z:zTemp02
    lda z:zTemp01
    adc a:ScrollXLo
    sta z:zTemp03
    cla
    adc a:ScrollXHiScreen
    jsr CompareScrollWithRightLimit
    bcc MovePlayerRight_at4
MovePlayerRight_at3:
    cla
    sec
    sbc a:ScrollXFrac
    sta a:ScrollStepXLo
    lda a:ScrollXMaxLo
    sbc a:ScrollXLo
    sta a:ScrollStepXHi
    lda z:zTemp04
    sec
    sbc a:ScrollStepXLo
    sta a:OBJ_X_FRAC
    lda z:zTemp05
    sbc a:ScrollStepXHi
    sta a:OBJ_X_LO
    rts
MovePlayerRight_at4:
    lda #$80
    sta a:OBJ_X_LO
    stz a:OBJ_X_FRAC
MovePlayerRight_at2:
    lda z:zTemp00
    sta a:ScrollStepXLo
    lda z:zTemp01
    sta a:ScrollStepXHi
    rts

; @routine 0048:0AF PlayerAtRightEdge
; @run 6A:68AF
; WHAT IT DOES: Stops the player at the right edge of the screen.
; HOW: Position $F0, fraction zero.
; Entry: nothing. Exit: A $F0, N 1, Z 0. X, Y, C, V kept.
PlayerAtRightEdge:
    lda #$F0
    sta a:OBJ_X_LO
    stz a:OBJ_X_FRAC
    rts

; @routine 0048:0B8 NextPastMiddle
; @run 6A:68B8
; WHAT IT DOES: Says whether the next position, in zTemp05, is at the screen's middle or
; past it.
; HOW: Compares with $80. At $80 it goes on to look at the player's X fraction, also for
; a Y position, and the answer is C set either way.
; Entry: zTemp05. Exit: C the answer; A and N, Z from the last read or compare. X, Y, V
; kept.
NextPastMiddle:
    lda z:zTemp05
    cmp #$80
    bcs NextPastMiddle_at0
    rts
NextPastMiddle_at0:
    beq PlayerXPastMiddle_at0
    rts

; @routine 0048:0C2 NextFractionSet
; @run 6A:68C2
; WHAT IT DOES: Sets C if the next position's fraction, zTemp04, is not zero. Nothing in
; the kernel reaches it: left over from an earlier NextPastMiddle.
; HOW: A read and a branch.
; Entry: zTemp04. Exit: A zTemp04, N and Z from it; C set if not zero, else kept.
NextFractionSet:
    lda z:zTemp04
    bne NextFractionSet_at0
    rts
NextFractionSet_at0:
    sec
    rts

; @routine 0048:0C9 PlayerXPastMiddle
; @run 6A:68C9
; WHAT IT DOES: Says whether the player is at the screen's middle or past it, along X.
; HOW: As NextPastMiddle, on the position.
; Entry: nothing. Exit: as NextPastMiddle.
PlayerXPastMiddle:
    lda a:OBJ_X_LO
    cmp #$80
    bcs PlayerXPastMiddle_at1
    rts
PlayerXPastMiddle_at1:
    beq PlayerXPastMiddle_at0
    rts
PlayerXPastMiddle_at0:
    lda a:OBJ_X_FRAC
    bne PlayerXPastMiddle_at2
    rts
PlayerXPastMiddle_at2:
    sec
    rts

; @routine 0048:0DC NextPastMiddleAmount
; @run 6A:68DC
; WHAT IT DOES: Gives how far the next position lies past the middle.
; HOW: Subtracts $8000 from zTemp04-05, into zTemp00-01.
; Entry: zTemp04-05. Exit: A zTemp01, N, Z, C, V from its subtract. X and Y kept.
NextPastMiddleAmount:
    lda z:zTemp04
    sec
    sbc #$00
    sta z:zTemp00
    lda z:zTemp05
    sbc #$80
    sta z:zTemp01
    rts

; @routine 0048:0EA CompareScrollWithRightLimit
; @run 6A:68EA
; WHAT IT DOES: Compares a scroll position with the level's right limit.
; HOW: High bytes, then on equal low bytes.
; Entry: A high, zTemp03 low. Exit: A and N, Z, C from the last compare. X, Y, V kept.
CompareScrollWithRightLimit:
    cmp a:ScrollXMaxHi
    bne CompareScrollWithRightLimit_at0
    lda z:zTemp03
    cmp a:ScrollXMaxLo
CompareScrollWithRightLimit_at0:
    rts

; @routine 0048:0F5 PlayerWallLeft
; @run 6A:68F5
; WHAT IT DOES: Asks the overlay about a wall to the left, and pushes the player back
; out of one: right by a table on how deep it went, but not into another wall the
; overlay finds there, and not past the right edge.
; HOW: As PlayerWallRight, with Y 1 then Y 0.
; Entry: cpu flags current. Exit: as PlayerWallRight.
PlayerWallLeft:
    ldy #$01
    jsr PlayerOvlWall
    beq PlayerWallLeft_at0
    lda a:OBJ_27A8
    cmp #$03
    bne PlayerWallLeft_at1
    lda a:OBJ_VY_HI
    bmi PlayerWallLeft_at2
    jsr StopPlayerX
    bra PlayerWallLeft_at2
PlayerWallLeft_at1:
    ldx z:zTemp00
    lda a:WallLeftPush,x
    clc
    adc a:OBJ_X_LO
    cmp #$F0
    bcs PlayerWallLeft_at3
    ldx a:OBJ_X_LO
    phx
    sta a:OBJ_X_LO
    stz a:OBJ_X_FRAC
    jsr StopPlayerX
    stz z:zPlayerPush
    ldy #$00
    jsr PlayerOvlWall
    plx
    and #$FF
    beq PlayerWallLeft_at2
    stx a:OBJ_X_LO
    bra PlayerWallLeft_at2
PlayerWallLeft_at3:
    jsr PlayerAtRightEdge
PlayerWallLeft_at2:
    lda #$01
PlayerWallLeft_at0:
    rts

; @routine 0048:146 MovePlayerLeftOrWall
; @run 6A:6946
; WHAT IT DOES: Moves the player left, unless there is a wall.
; HOW: PlayerWallLeft, then MovePlayerLeft.
; Entry: cpu flags current. Exit: as the one that ran last.
MovePlayerLeftOrWall:
    jsr PlayerWallLeft
    beq MovePlayerLeftOrWall_at0
    rts
MovePlayerLeftOrWall_at0:
    jmp MovePlayerLeft

; @routine 0048:14F PlayerXFromNextAlt
; @run 6A:694F
; WHAT IT DOES: The same as PlayerXFromNext, written again for the leftward moves.
; HOW: Copies zTemp04-05 to the position.
; Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
PlayerXFromNextAlt:
    lda z:zTemp04
    sta a:OBJ_X_FRAC
    lda z:zTemp05
    sta a:OBJ_X_LO
    rts

; @routine 0048:15A MovePlayerLeft
; @run 6A:695A
; WHAT IT DOES: Moves the player left, or the scroll under it, as MovePlayerRight does
; to the right, and stops the player at the left edge of the screen.
; HOW: The next position, the edge, then the scroll's start in place of its limit.
; Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
; zTemp00-05.
MovePlayerLeft:
    jsr PlayerNextX
    cmp #$10
    bcs MovePlayerLeft_at0
    jmp PlayerAtLeftEdge
MovePlayerLeft_at0:
    bne MovePlayerLeft_at1
    jmp PlayerAtLeftEdge
MovePlayerLeft_at1:
    jsr NextPastMiddle
    bcs PlayerXFromNextAlt
    jsr NextPastMiddleAmount
    lda a:ScrollXHiScreen
    cmp a:ScrollXMinHi
    bne MovePlayerLeft_at2
    lda a:ScrollXFrac
    ora a:ScrollXLo
    bne MovePlayerLeft_at3
    bra PlayerXFromNextAlt
MovePlayerLeft_at2:
    jsr PlayerXPastMiddle
    bcc MovePlayerLeft_at4
    bra MovePlayerLeft_at5
MovePlayerLeft_at3:
    jsr PlayerXPastMiddle
    bcs MovePlayerLeft_at6
    lda a:OBJ_VX_LO
    clc
    adc a:ScrollXFrac
    sta z:zTemp02
    lda a:OBJ_VX_HI
    adc a:ScrollXLo
    sta z:zTemp03
    lda #$FF
    adc a:ScrollXHiScreen
    cmp a:ScrollXMinHi
    beq MovePlayerLeft_at4
    bra MovePlayerLeft_at7
MovePlayerLeft_at6:
    lda z:zTemp00
    clc
    adc a:ScrollXFrac
    sta z:zTemp02
    lda z:zTemp01
    adc a:ScrollXLo
    sta z:zTemp03
    lda #$FF
    adc a:ScrollXHiScreen
    cmp a:ScrollXMinHi
    beq MovePlayerLeft_at5
MovePlayerLeft_at7:
    cla
    sec
    sbc a:ScrollXFrac
    sta a:ScrollStepXLo
    cla
    sbc a:ScrollXLo
    sta a:ScrollStepXHi
    lda a:OBJ_X_FRAC
    clc
    adc z:zTemp02
    sta a:OBJ_X_FRAC
    lda a:OBJ_X_LO
    adc z:zTemp03
    sta a:OBJ_X_LO
    rts
MovePlayerLeft_at4:
    lda a:OBJ_VX_LO
    sta a:ScrollStepXLo
    lda a:OBJ_VX_HI
    sta a:ScrollStepXHi
    rts
MovePlayerLeft_at5:
    lda #$80
    sta a:OBJ_X_LO
    stz a:OBJ_X_FRAC
    lda z:zTemp00
    sta a:ScrollStepXLo
    lda z:zTemp01
    sta a:ScrollStepXHi
    rts

; @routine 0048:206 PlayerAtLeftEdge
; @run 6A:6A06
; WHAT IT DOES: Stops the player at the left edge of the screen.
; HOW: Position $10, fraction zero.
; Entry: nothing. Exit: A $10, N 0, Z 0. X, Y, C, V kept.
PlayerAtLeftEdge:
    lda #$10
    sta a:OBJ_X_LO
    stz a:OBJ_X_FRAC
    rts

; @routine 0048:20F CameraModeVertical
; @run 6A:6A0F
; WHAT IT DOES: The camera mode for a section that scrolls up and down: the player moves
; on the screen along X, and the scroll follows it along Y. Going down it asks the
; overlay about the floor before and after moving.
; HOW: PlayerXOnScreen; a Y speed of zero clears the Y scroll's fraction; then the Y
; speed's sign. The first floor answer not zero means no move.
; Entry: cpu flags current. Exit: A and flags as the path taken left them.
CameraModeVertical:
    jsr PlayerXOnScreen
    stz a:ScrollStepYHi
    stz a:ScrollStepYLo
    lda a:OBJ_VY_HI
    ora a:OBJ_VY_LO
    bne CameraModeVertical_at0
    stz a:ScrollYFrac
CameraModeVertical_at0:
    lda a:OBJ_VY_HI
    bmi CameraModeVertical_at1
    jsr PlayerOvlFloor
    bne CameraModeVertical_at2
    jsr MovePlayerDown
    stz z:zPlayerPush
    jmp PlayerOvlFloor
CameraModeVertical_at2:
    rts
CameraModeVertical_at1:
    jmp MovePlayerUp

; @routine 0048:239 MovePlayerYIfMovingGoTo
; @run 6A:6A39
; WHAT IT DOES: Goes to MovePlayerYIfMoving. Nothing in the kernel reaches it.
; HOW: One jump.
; Entry, exit: as MovePlayerYIfMoving.
MovePlayerYIfMovingGoTo:
    jmp MovePlayerYIfMoving

; @routine 0048:23C PlayerStopAtRightEdge
; @run 6A:6A3C
; WHAT IT DOES: Stops the player at the right edge, for PlayerXOnScreen.
; HOW: Writes the position and zeroes the fraction.
; Entry: nothing. Exit: A $F0, N 1, Z 0. X, Y, C, V kept.
PlayerStopAtRightEdge:
    lda #$F0
PlayerStopAtRightEdge_at0:
    sta a:OBJ_X_LO
    stz a:OBJ_X_FRAC
    rts

; @routine 0048:245 PlayerStopAtLeftEdge
; @run 6A:6A45
; WHAT IT DOES: Stops the player at the left edge, for PlayerXOnScreen.
; HOW: The same write as PlayerStopAtRightEdge, with $10.
; Entry: nothing. Exit: A $10, N 0, Z 0. X, Y, C, V kept.
PlayerStopAtLeftEdge:
    lda #$10
    bra PlayerStopAtRightEdge_at0

; @routine 0048:249 PlayerXOnScreen
; @run 6A:6A49
; WHAT IT DOES: Moves the player along X by its speed, on the screen alone, unless there
; is a wall, and keeps it between $10 and $EF.
; HOW: Follows the speed's sign, or bits 2 and 1 of zPlayerPush at no speed, through the
; wall checks.
; Entry: cpu flags current. Exit: A and flags as the path taken left them.
PlayerXOnScreen:
    lda a:OBJ_VX_LO
    ora a:OBJ_VX_HI
    beq PlayerXOnScreen_at0
    lda a:OBJ_VX_HI
    bmi PlayerXOnScreen_at1
PlayerXOnScreen_at4:
    jsr PlayerWallRight
    bne PlayerXOnScreen_at2
PlayerXOnScreen_at3:
    lda a:OBJ_VX_LO
    clc
    adc a:OBJ_X_FRAC
    sta a:OBJ_X_FRAC
    lda a:OBJ_VX_HI
    adc a:OBJ_X_LO
    sta a:OBJ_X_LO
    cmp #$F0
    bcs PlayerStopAtRightEdge
    cmp #$10
    bcc PlayerStopAtLeftEdge
PlayerXOnScreen_at2:
    rts
PlayerXOnScreen_at1:
    jsr PlayerWallLeft
    beq PlayerXOnScreen_at3
    rts
PlayerXOnScreen_at0:
    tst #$04,z:zPlayerPush
    bne PlayerXOnScreen_at1
    tst #$02,z:zPlayerPush
    bne PlayerXOnScreen_at4
    rts

; @routine 0048:288 PlayerYFromNextWord
; @run 6A:6A88
; WHAT IT DOES: Accepts the Y position worked out in zTemp04-06, with its high byte.
; HOW: Copies it to the position.
; Entry: zTemp04-06. Exit: A the high byte, N and Z from it. X, Y, C, V kept.
PlayerYFromNextWord:
    lda z:zTemp04
    sta a:OBJ_Y_FRAC
    lda z:zTemp05
    sta a:OBJ_Y_LO
    lda z:zTemp06
    sta a:OBJ_Y_HI
    rts

; @routine 0048:298 MovePlayerDown
; @run 6A:6A98
; WHAT IT DOES: Moves the player down, or the scroll under it, as MovePlayerRight does
; to the right.
; HOW: The 16.8 next position in zTemp04-06, then as MovePlayerRight on the Y fields.
; Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
; zTemp00-06.
MovePlayerDown:
    stz a:ScrollStepYLo
    stz a:ScrollStepYHi
    lda a:OBJ_VY_LO
    clc
    adc a:OBJ_Y_FRAC
    sta z:zTemp04
    lda a:OBJ_VY_HI
    adc a:OBJ_Y_LO
    sta z:zTemp05
    cla
    adc a:OBJ_Y_HI
    sta z:zTemp06
    jsr NextPastMiddle
    bcc PlayerYFromNextWord
    lda a:ScrollYLo
    sta z:zTemp03
    lda a:ScrollYHiScreen
    jsr CompareScrollWithBottomLimit
    beq PlayerYFromNextWord
    jsr NextPastMiddleAmount
    jsr PlayerYPastMiddle
    bcc MovePlayerDown_at0
    lda a:OBJ_VY_LO
    sta z:zTemp00
    clc
    adc a:ScrollYFrac
    sta z:zTemp02
    lda a:OBJ_VY_HI
    sta z:zTemp01
    adc a:ScrollYLo
    sta z:zTemp03
    cla
    adc a:ScrollYHiScreen
    jsr CompareScrollWithBottomLimit
    bcc MovePlayerDown_at1
    bra MovePlayerDown_at2
MovePlayerDown_at0:
    lda z:zTemp00
    clc
    adc a:ScrollYFrac
    sta z:zTemp02
    lda z:zTemp01
    adc a:ScrollYLo
    sta z:zTemp03
    cla
    adc a:ScrollYHiScreen
    jsr CompareScrollWithBottomLimit
    bcc MovePlayerDown_at3
MovePlayerDown_at2:
    cla
    sec
    sbc a:ScrollYFrac
    sta a:ScrollStepYLo
    lda a:ScrollYMaxLo
    sbc a:ScrollYLo
    sta a:ScrollStepYHi
    lda z:zTemp04
    sec
    sbc a:ScrollStepYLo
    sta a:OBJ_Y_FRAC
    lda z:zTemp05
    sbc a:ScrollStepYHi
    sta a:OBJ_Y_LO
    rts
MovePlayerDown_at3:
    lda #$80
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_FRAC
MovePlayerDown_at1:
    lda z:zTemp00
    sta a:ScrollStepYLo
    lda z:zTemp01
    sta a:ScrollStepYHi
    rts

; @routine 0048:33D PlayerYPastMiddle
; @run 6A:6B3D
; WHAT IT DOES: Says whether the player is at the screen's middle or past it, along Y.
; HOW: As PlayerXPastMiddle.
; Entry: nothing. Exit: as NextPastMiddle.
PlayerYPastMiddle:
    lda a:OBJ_Y_LO
    cmp #$80
    bcs PlayerYPastMiddle_at0
    rts
PlayerYPastMiddle_at0:
    beq PlayerYPastMiddle_at1
    rts
PlayerYPastMiddle_at1:
    lda a:OBJ_Y_FRAC
    bne PlayerYPastMiddle_at2
    rts
PlayerYPastMiddle_at2:
    sec
    rts

; @routine 0048:350 CompareScrollWithBottomLimit
; @run 6A:6B50
; WHAT IT DOES: Compares a vertical scroll position with the section's bottom limit.
; HOW: High bytes, then on equal low bytes.
; Entry: A high, zTemp03 low. Exit: A and N, Z, C from the last compare. X, Y, V kept.
CompareScrollWithBottomLimit:
    cmp a:ScrollYMaxHi
    bne CompareScrollWithBottomLimit_at0
    lda z:zTemp03
    cmp a:ScrollYMaxLo
CompareScrollWithBottomLimit_at0:
    rts

; @routine 0048:35B PlayerAtTop
; @run 6A:6B5B
; WHAT IT DOES: Puts the player at the top of the map.
; HOW: Zeroes its Y position.
; Entry: nothing. Exit: all registers and flags kept.
PlayerAtTop:
    stz a:OBJ_Y_FRAC
    stz a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    rts

; @routine 0048:365 PlayerYFromNext
; @run 6A:6B65
; WHAT IT DOES: Accepts the Y position worked out in zTemp04-05.
; HOW: Copies it to the position.
; Entry: zTemp04-05. Exit: A zTemp05, N and Z from it. X, Y, C, V kept.
PlayerYFromNext:
    lda z:zTemp04
    sta a:OBJ_Y_FRAC
    lda z:zTemp05
    sta a:OBJ_Y_LO
    rts

; @routine 0048:370 MovePlayerUp
; @run 6A:6B70
; WHAT IT DOES: Moves the player up, or the scroll under it, as MovePlayerLeft does to
; the left, and stops the player at the top of the map if it would pass it.
; HOW: The next position, zero or a borrow meaning the top, then as MovePlayerLeft on the
; Y fields.
; Entry: nothing. Exit: A, flags as the last step left them. X and Y kept. Clobbers
; zTemp00-05.
MovePlayerUp:
    lda a:OBJ_VY_LO
    clc
    adc a:OBJ_Y_FRAC
    sta z:zTemp04
    lda a:OBJ_VY_HI
    adc a:OBJ_Y_LO
    sta z:zTemp05
    beq PlayerAtTop
    bcc PlayerAtTop
    jsr NextPastMiddle
    bcs PlayerYFromNext
    jsr NextPastMiddleAmount
    lda a:ScrollYHiScreen
    cmp a:ScrollYMinHi
    bne MovePlayerUp_at0
    lda a:ScrollYFrac
    ora a:ScrollYLo
    bne MovePlayerUp_at1
    bra PlayerYFromNext
MovePlayerUp_at0:
    jsr PlayerYPastMiddle
    bcc MovePlayerUp_at2
    bra MovePlayerUp_at3
MovePlayerUp_at1:
    jsr PlayerYPastMiddle
    bcs MovePlayerUp_at4
    lda a:OBJ_VY_LO
    clc
    adc a:ScrollYFrac
    sta z:zTemp02
    lda a:OBJ_VY_HI
    adc a:ScrollYLo
    sta z:zTemp03
    lda #$FF
    adc a:ScrollYHiScreen
    cmp a:ScrollYMinHi
    beq MovePlayerUp_at2
    bra MovePlayerUp_at5
MovePlayerUp_at4:
    lda z:zTemp00
    clc
    adc a:ScrollYFrac
    sta z:zTemp02
    lda z:zTemp01
    adc a:ScrollYLo
    sta z:zTemp03
    lda #$FF
    adc a:ScrollYHiScreen
    cmp a:ScrollYMinHi
    beq MovePlayerUp_at3
MovePlayerUp_at5:
    cla
    sec
    sbc a:ScrollYFrac
    sta a:ScrollStepYLo
    cla
    sbc a:ScrollYLo
    sta a:ScrollStepYHi
    lda a:OBJ_Y_FRAC
    clc
    adc a:ScrollStepYLo
    sta a:OBJ_Y_FRAC
    lda a:OBJ_Y_LO
    adc a:ScrollStepYHi
    sta a:OBJ_Y_LO
    rts
MovePlayerUp_at2:
    lda a:OBJ_VY_LO
    sta a:ScrollStepYLo
    lda a:OBJ_VY_HI
    sta a:ScrollStepYHi
    rts
MovePlayerUp_at3:
    lda #$80
    sta a:OBJ_Y_LO
    stz a:OBJ_Y_FRAC
    lda z:zTemp00
    sta a:ScrollStepYLo
    lda z:zTemp01
    sta a:ScrollStepYHi
    rts
