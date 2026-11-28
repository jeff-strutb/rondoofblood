; @routine 072C:37E CalmSlots11To26_072C
; @run 70:BB7E
; WHAT IT DOES: Keeps only bits $87 of OBJ_PACE in slots $11 to $26, stops this object, sets OBJ_WORK_I 3 and counts its substate
; (secret stage 03's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A 3, Y $27, C set, N and Z from the new substate; X kept.
CalmSlots11To26_072C:
    ldy #$11
CalmSlots11To26_072C_at0:
    lda a:OBJ_PACE,y
    and #$87
    sta a:OBJ_PACE,y
    iny
    cpy #$27
    bcc CalmSlots11To26_072C_at0
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$03
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 088B:458 ClampAngle088B
; @run 70:B458
; WHAT IT DOES: Narrows an angle in A: under $40 to 0, $40 to $7F to $80, $A0 to $BF to $A0, $C0 to $DF to $E0 (secret stage 04's
; overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A the angle; C from the last compare; X and Y kept.
ClampAngle088B:
    cmp #$C0
    bcs ClampAngle088B_at0
    cmp #$40
    bcc ClampAngle088B_at1
    cmp #$80
    bcc ClampAngle088B_at2
    cmp #$A0
    bcc ClampAngle088B_at3
    lda #$A0
ClampAngle088B_at3:
    rts
ClampAngle088B_at2:
    lda #$80
    rts
ClampAngle088B_at0:
    cmp #$E0
    bcs ClampAngle088B_at3
    lda #$E0
    rts
ClampAngle088B_at1:
    cla
    rts

; @routine 0CAC:0B4 InWindow0CAC
; @run 70:B8B4
; WHAT IT DOES: C set when the object's place is within X $7F-$81 and Y $1F-$21 (secret stage 07's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: C the answer; A the byte last read.
InWindow0CAC:
    lda a:OBJ_X_LO,x
    cmp #$7F
    bcc InWindow0CAC_at0
    cmp #$82
    bcs InWindow0CAC_at0
    lda a:OBJ_Y_LO,x
    cmp #$1F
    bcc InWindow0CAC_at0
    cmp #$22
    bcs InWindow0CAC_at0
    sec
    rts
InWindow0CAC_at0:
    clc
    rts

; @routine 0EBB:12F StartByRoute0EBB
; @run 70:B12F
; WHAT IT DOES: While zField8C's bit 7 is clear: the state counted on and OBJ_WORK_D $40; start arguments 2 to 5 count it once more
; (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last compare; X kept.
StartByRoute0EBB:
    lda z:zField8C
    bmi StartByRoute0EBB_at0
    inc a:OBJ_STATE,x
    lda #$40
    sta a:OBJ_WORK_D,x
    lda a:OBJ_START_ARG,x
    and #$0F
    cmp #$02
    bcc StartByRoute0EBB_at1
    cmp #$06
    bcc StartByRoute0EBB_at2
StartByRoute0EBB_at1:
    bra StartByRoute0EBB_at0
StartByRoute0EBB_at2:
    inc a:OBJ_STATE,x
StartByRoute0EBB_at0:
    rts

; @routine 0EBB:3E4 FallUpToSpeed7_0EBB
; @run 70:B3E4
; WHAT IT DOES: Gravity of $48 a frame until the Y speed reaches 7 (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the compare or the add; X and Y kept.
FallUpToSpeed7_0EBB:
    lda a:OBJ_VY_HI,x
    bmi FallUpToSpeed7_0EBB_at0
    cmp #$07
    bcs FallUpToSpeed7_0EBB_at1
FallUpToSpeed7_0EBB_at0:
    lda a:OBJ_VY_LO,x
    clc
    adc #$48
    sta a:OBJ_VY_LO,x
    bcc FallUpToSpeed7_0EBB_at1
    inc a:OBJ_VY_HI,x
FallUpToSpeed7_0EBB_at1:
    rts

; @routine 0EBC:0FE BobByTable0EBC
; @run 70:B8FE
; WHAT IT DOES: While OBJ_WORK_E's bit 7 is set, moves the object down by the table's step and counts it on, 16 steps (Stage 09's
; overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the compare or the add; Y the step; X kept.
BobByTable0EBCSteps = $B91C ; the Y step, 16
BobByTable0EBC:
    lda a:OBJ_WORK_E,x
    bpl BobByTable0EBC_at0
    and #$7F
    tay
    cpy #$10
    beq BobByTable0EBC_at1
    lda a:BobByTable0EBCSteps,y
    clc
    adc a:OBJ_Y_LO,x
    sta a:OBJ_Y_LO,x
    inc a:OBJ_WORK_E,x
    rts
BobByTable0EBC_at1:
    stz a:OBJ_WORK_E,x
BobByTable0EBC_at0:
    rts

; @routine 0EBC:673 NotePlaceAbovePrevious0EBC
; @run 70:BE73
; WHAT IT DOES: Keeps the place 18 dots above the slot before in OBJ_WORK_D to G (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the X high byte, N and Z from it, C and V from the Y add; X kept.
NotePlaceAbovePrevious0EBC:
    lda #$EE
    clc
    adc a:OBJ_Y_LO-1,x
    sta a:OBJ_WORK_D,x
    lda #$FF
    adc a:OBJ_Y_HI-1,x
    sta a:OBJ_WORK_E,x
    lda a:OBJ_X_LO-1,x
    sta a:OBJ_WORK_F,x
    lda a:OBJ_X_HI-1,x
    sta a:OBJ_WORK_G,x
    rts

; @routine 0EBE:1EF LevelXBefore150_0EBE
; @run 71:C9EF
; WHAT IT DOES: C set while the object's level X is under $150 (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: C the answer; A the byte last read; zTemp00 the low byte.
LevelXBefore150_0EBE:
    lda a:ScrollXLo
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    lda a:ScrollXHiScreen
    adc a:OBJ_X_HI,x
    bmi LevelXBefore150_0EBE_at0
    cmp #$01
    bcc LevelXBefore150_0EBE_at0
    bne LevelXBefore150_0EBE_at1
    lda z:zTemp00
    cmp #$50
    bcs LevelXBefore150_0EBE_at1
LevelXBefore150_0EBE_at0:
    sec
    rts
LevelXBefore150_0EBE_at1:
    clc
    rts

; @routine 0EBF:676 DriftWithScroll0EBF
; @run 71:D676
; WHAT IT DOES: While the screen scrolls along X, moves the object by the table's word for the direction (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the add, or A 0 when still; Y the direction's offset; X kept.
DriftWithScroll0EBFDrifts = $D695 ; the move by scroll direction, a word each
DriftWithScroll0EBF:
    lda a:ScrollSpeedX
    beq DriftWithScroll0EBF_at0
    rol a
    rol a
    and #$01
    asl a
    tay
    lda a:OBJ_X_LO,x
    clc
    adc a:DriftWithScroll0EBFDrifts,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    adc a:DriftWithScroll0EBFDrifts+1,y
    sta a:OBJ_X_HI,x
DriftWithScroll0EBF_at0:
    rts

; @routine 101B:107 WaitThenState3_101B
; @run 70:B107
; WHAT IT DOES: Counts OBJ_WORK_H down; at zero restarts the animation, sets the count 104 and goes to state 3 (Stage 0A's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load or compare left them; X kept.
WaitThenState3_101B:
    lda a:OBJ_WORK_H,x
    bne WaitThenState3_101B_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$68
    sta a:OBJ_WORK_H,x
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts
WaitThenState3_101B_at0:
    dec a:OBJ_WORK_H,x
    rts

; @routine 101B:580 SlowByFacing101B
; @run 70:B580
; WHAT IT DOES: Takes the facing's step off the X speed (Stage 0A's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the high byte, flags from its subtract; Y the facing's offset.
SlowByFacing101BSteps = $B59B ; the step by facing, a word each
SlowByFacing101B:
    lda a:OBJ_FACING_LEFT,x
    and #$01
    asl a
    tay
    lda a:OBJ_VX_LO,x
    sec
    sbc a:SlowByFacing101BSteps,y
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    sbc a:SlowByFacing101BSteps+1,y
    sta a:OBJ_VX_HI,x
    rts

; @routine 101E:1EB CountToState4_101E
; @run 71:C9EB
; WHAT IT DOES: Counts OBJ_WORK_K down (C clear); at zero sets it to 192, clears OBJ_WORK_P and goes to state 4 (C set).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: C whether the state changed; X kept.
CountToState4_101E:
    lda a:OBJ_WORK_K,x
    bne CountToState4_101E_at0
    stz a:OBJ_WORK_P,x
    lda #$C0
    sta a:OBJ_WORK_K,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    sec
    rts
CountToState4_101E_at0:
    dec a:OBJ_WORK_K,x
    clc
    rts

; @routine 1020:23E MoveWithScrollY1020
; @run 71:DA3E
; WHAT IT DOES: Adds this frame's scroll speed along Y to the object's Y (Stage 0A's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the Y high byte, flags from its add; X and Y kept.
MoveWithScrollY1020:
    stz z:zTemp00
    lda a:ScrollSpeedY
    bpl MoveWithScrollY1020_at0
    dec z:zTemp00
MoveWithScrollY1020_at0:
    lda a:OBJ_Y_LO,x
    clc
    adc a:ScrollSpeedY
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    adc z:zTemp00
    sta a:OBJ_Y_HI,x
    rts

; @routine 10CC:539 RestartWhenAboveSlot12_10CC
; @run 70:BD39
; WHAT IT DOES: Once slot $12 is on the first screen below this object, restarts the animation and counts the substate (secret
; stage 0A's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load or compare left them; X kept.
RestartWhenAboveSlot12_10CC:
    lda a:OBJ_Y_HI+18
    bne RestartWhenAboveSlot12_10CC_at0
    lda a:OBJ_Y_LO,x
    cmp a:OBJ_Y_LO+18
    bcs RestartWhenAboveSlot12_10CC_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
RestartWhenAboveSlot12_10CC_at0:
    rts

; @routine 117B:178 FarOffScreenX117B
; @run 70:B178
; WHAT IT DOES: C set when the object is more than 32 dots off the screen along X (Stage 0B's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: C the answer; A the byte last read.
FarOffScreenX117B:
    lda a:OBJ_X_HI,x
    beq FarOffScreenX117B_at0
    bpl FarOffScreenX117B_at1
    cmp #$FF
    bne FarOffScreenX117B_at2
    lda a:OBJ_X_LO,x
    cmp #$E0
    bcs FarOffScreenX117B_at0
FarOffScreenX117B_at2:
    sec
    rts
FarOffScreenX117B_at1:
    cmp #$01
    bne FarOffScreenX117B_at2
    lda a:OBJ_X_LO,x
    cmp #$20
    bcs FarOffScreenX117B_at2
FarOffScreenX117B_at0:
    clc
    rts

; @routine 117F:486 CountOnScreenToState4_117F
; @run 71:D486
; WHAT IT DOES: On the first screen, counts OBJ_WORK_E down (C clear); at zero state 4 (C set) (Stage 0B's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: C whether the state changed.
CountOnScreenToState4_117F:
    lda a:OBJ_X_HI,x
    bne CountOnScreenToState4_117F_at0
    lda a:OBJ_WORK_E,x
    beq CountOnScreenToState4_117F_at1
    dec a:OBJ_WORK_E,x
CountOnScreenToState4_117F_at0:
    clc
    rts
CountOnScreenToState4_117F_at1:
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    sec
    rts

; @routine 117F:65D NextWaitToState3_117F
; @run 71:D65D
; WHAT IT DOES: State 3 with the animation restarted and the next of four counts in OBJ_WORK_E (Stage 0B's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the count; Y the index; X kept.
NextWaitToState3_117FCounts = $D678 ; OBJ_WORK_E by OBJ_WORK_G's low two bits
NextWaitToState3_117F:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_STATE,x
    inc a:OBJ_WORK_G,x
    lda a:OBJ_WORK_G,x
    and #$03
    tay
    lda a:NextWaitToState3_117FCounts,y
    sta a:OBJ_WORK_E,x
    rts

; @routine 12DB:559 NextSubstateOrState2_12DB
; @run 70:B559
; WHAT IT DOES: The next substate of a cycle of eight (OBJ_WORK_I); a 0 in the cycle means state 2 (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the substate or 2; Y the place in the cycle.
NextSubstateOrState2_12DBCycle = $B570 ; the substates in turn
NextSubstateOrState2_12DB:
    lda a:OBJ_WORK_I,x
    and #$07
    tay
    lda a:NextSubstateOrState2_12DBCycle,y
    sta a:OBJ_SUBSTATE,x
    bne NextSubstateOrState2_12DB_at0
    lda #$02
    sta a:OBJ_STATE,x
NextSubstateOrState2_12DB_at0:
    inc a:OBJ_WORK_I,x
    rts

; @routine 12DB:687 RasterWhenLow12DB
; @run 70:B687
; WHAT IT DOES: Raster effect $0B from line $C0 while the object is low on the first screen, else none (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load or compare left them; X kept.
RasterWhenLow12DB:
    lda a:OBJ_Y_HI,x
    bne RasterWhenLow12DB_at0
    lda a:OBJ_Y_LO,x
    cmp #$7E
    bcc RasterWhenLow12DB_at0
    lda #$0B
    sta z:zRasterEffect
    lda #$C0
    sta z:zRasterArg
    rts
RasterWhenLow12DB_at0:
    stz z:zRasterEffect
    stz z:zRasterArg
    rts

; @routine 12DC:4DB CountToState4_12DC
; @run 70:BCDB
; WHAT IT DOES: Counts OBJ_WORK_K down (C clear); at zero sets it to 128, clears OBJ_WORK_P and goes to state 4 (C set).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: C whether the state changed; X kept.
CountToState4_12DC:
    lda a:OBJ_WORK_K,x
    bne CountToState4_12DC_at0
    stz a:OBJ_WORK_P,x
    lda #$80
    sta a:OBJ_WORK_K,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    sec
    rts
CountToState4_12DC_at0:
    dec a:OBJ_WORK_K,x
    clc
    rts

; @routine 12DC:548 NextSubstateOfEight12DC
; @run 70:BD48
; WHAT IT DOES: The next substate of a cycle of eight (OBJ_WORK_N), the animation restarted (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the substate; Y the place in the cycle.
NextSubstateOfEight12DCCycle = $BD61 ; the substates in turn
NextSubstateOfEight12DC:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_WORK_P,x
    lda a:OBJ_WORK_N,x
    and #$07
    tay
    lda a:NextSubstateOfEight12DCCycle,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_N,x
    rts

; @routine 12DC:62F ThrowUp12DC
; @run 70:BE2F
; WHAT IT DOES: No X speed, an upward speed of -7, OBJ_WORK_L 48, substate on (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A $30; N and Z from the new substate; X kept.
ThrowUp12DC:
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    lda #$F9
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    lda #$30
    sta a:OBJ_WORK_L,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 12DC:648 SpinCount12DC
; @run 70:BE48
; WHAT IT DOES: Adds 6 to OBJ_WORK_D and counts OBJ_WORK_L; at zero sets it to 28 and counts the substate (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load or compare left them; X kept.
SpinCount12DC:
    lda a:OBJ_WORK_D,x
    clc
    adc #$06
    sta a:OBJ_WORK_D,x
    dec a:OBJ_WORK_L,x
    bne SpinCount12DC_at0
    lda #$1C
    sta a:OBJ_WORK_L,x
    inc a:OBJ_SUBSTATE,x
SpinCount12DC_at0:
    rts

; @routine 12DC:6C2 LaunchUpByFacing12DC
; @run 70:BEC2
; WHAT IT DOES: The X speed of its facing (both bytes the same table byte), an upward speed of -7.25, the substate counted on
; (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A $C0, N and Z from the new substate, Y twice the facing; X kept.
LaunchUpByFacing12DCSpeeds = $BEE2 ; the X speed's byte by facing, a word each
LaunchUpByFacing12DC:
    lda a:OBJ_FACING_LEFT,x
    asl a
    tay
    lda a:LaunchUpByFacing12DCSpeeds,y
    sta a:OBJ_VX_HI,x
    lda a:LaunchUpByFacing12DCSpeeds,y
    sta a:OBJ_VX_LO,x
    lda #$F8
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 13FB:2F5 HitToState5_13FB
; @run 70:B2F5
; WHAT IT DOES: Under state 5, once hit: OBJ_WORK_L cleared, state 5 (secret stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load or compare left them; X kept.
HitToState5_13FB:
    lda a:OBJ_STATE,x
    cmp #$05
    bcs HitToState5_13FB_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitToState5_13FB_at0
    stz a:OBJ_WORK_L,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitToState5_13FB_at0:
    rts

; @routine 161D:6A6 CountThenState4_161D
; @run 71:C6A6
; WHAT IT DOES: Counts OBJ_WORK_H down (C clear); at zero goes to state 4 and loads the next count by OBJ_WORK_G (C set)
; (Stage 0E's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: C whether the state changed; X kept.
CountThenState4_161DCounts = $C6C6 ; the next count by OBJ_WORK_G's low two bits
CountThenState4_161D:
    lda a:OBJ_WORK_H,x
    beq CountThenState4_161D_at0
    dec a:OBJ_WORK_H,x
    clc
    rts
CountThenState4_161D_at0:
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    lda a:OBJ_WORK_G,x
    and #$03
    tay
    lda a:CountThenState4_161DCounts,y
    sta a:OBJ_WORK_H,x
    sec
    rts

; @routine 18DB:203 StartAtLevelX100_18DB
; @run 70:B203
; WHAT IT DOES: When the scroll is exactly at level X $100, stops it there (ScrollXMaxHi 1), sets OBJ_WORK_E 96, substate on
; (Stage 10's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load or compare left them; X kept.
StartAtLevelX100_18DB:
    lda a:ScrollXHiScreen
    cmp #$01
    bne StartAtLevelX100_18DB_at0
    lda a:ScrollXLo
    bne StartAtLevelX100_18DB_at0
    lda #$01
    sta a:ScrollXMaxHi
    lda #$60
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
StartAtLevelX100_18DB_at0:
    rts

; @routine 18DB:30D StartWhenSlot18Step7_18DB
; @run 70:B30D
; WHAT IT DOES: When slot $18 reaches animation step 7: frame 1, attribute $10, OBJ_WORK_E 16, substate on (Stage 10's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load or compare left them; X kept.
StartWhenSlot18Step7_18DB:
    lda a:OBJ_ANIM_STEP+24
    cmp #$07
    bne StartWhenSlot18Step7_18DB_at0
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    lda #$10
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
StartWhenSlot18Step7_18DB_at0:
    rts

; @routine 18DB:394 HitBoxByAnimationStep18DB
; @run 70:B394
; WHAT IT DOES: The hit box on only in animation steps 6 to 11; else off with attribute $10 (Stage 10's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A 2 or $10; X and Y kept.
HitBoxByAnimationStep18DB:
    lda a:OBJ_ANIM_STEP,x
    cmp #$06
    bcc HitBoxByAnimationStep18DB_at0
    cmp #$0C
    bcs HitBoxByAnimationStep18DB_at0
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    stz a:OBJ_SPRITE_ATTR,x
    rts
HitBoxByAnimationStep18DB_at0:
    stz a:OBJ_HITBOX_ON,x
    lda #$10
    sta a:OBJ_SPRITE_ATTR,x
    rts
