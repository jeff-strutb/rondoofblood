; @routine 088B:573 CountDown088B_573
; @run 70:B573
; WHAT IT DOES: Counts OBJ_WORK_F; at zero the hit box off and state $12 (secret stage 04's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
CountDown088B_573:
    dec a:OBJ_WORK_F,x
    bne CountDown088B_573_at0
    stz a:OBJ_HITBOX_ON,x
    lda #$12
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
CountDown088B_573_at0:
    rts

; @routine 088B:76A SetWorkDOfSlots18To21_088B
; @run 70:B76A
; WHAT IT DOES: OBJ_WORK_D 2 in slots $12 to $15 (secret stage 04's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A 2, Y $16, N 0, Z 1, C 1; X kept.
SetWorkDOfSlots18To21_088B:
    ldy #$12
SetWorkDOfSlots18To21_088B_at0:
    lda #$02
    sta a:OBJ_WORK_D,y
    iny
    cpy #$16
    bcc SetWorkDOfSlots18To21_088B_at0
    rts

; @routine 088C:035 CountDown088C_035
; @run 70:B835
; WHAT IT DOES: Counts OBJ_WORK_F; at zero sets it to 32 and counts the state (secret stage 04's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
CountDown088C_035:
    dec a:OBJ_WORK_F,x
    bne CountDown088C_035_at0
    lda #$20
    sta a:OBJ_WORK_F,x
    inc a:OBJ_STATE,x
CountDown088C_035_at0:
    rts

; @routine 0B4D:02B StartOnSlot16Frame0B4D
; @run 71:C02B
; WHAT IT DOES: When slot $10 shows frame $39: OBJ_WORK_D $2E, state on (secret stage 06's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
StartOnSlot16Frame0B4D:
    lda a:OBJ_FRAME+16
    cmp #$39
    bne StartOnSlot16Frame0B4D_at0
    lda #$2E
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
StartOnSlot16Frame0B4D_at0:
    rts

; @routine 0E0C:0BC StoreYHigh0E0C
; @run 70:B8BC
; WHAT IT DOES: Stores A as the object's Y high (secret stage 08's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: all registers and flags kept.
StoreYHigh0E0C:
    sta a:OBJ_Y_HI,x
    rts

; @routine 0EBC:060 RasterByScreenSide0EBC
; @run 70:B860
; WHAT IT DOES: Raster effect $17 unless the object is left of the screen (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
RasterByScreenSide0EBC:
    lda a:OBJ_X_HI,x
    bpl RasterByScreenSide0EBC_at0
    stz z:zRasterEffect
    rts
RasterByScreenSide0EBC_at0:
    lda #$17
    sta z:zRasterEffect
    rts

; @routine 0EBC:0D5 CountWord_FG_0EBC
; @run 70:B8D5
; WHAT IT DOES: Counts the word OBJ_WORK_F-G on (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
CountWord_FG_0EBC:
    inc a:OBJ_WORK_F,x
    bne CountWord_FG_0EBC_at0
    inc a:OBJ_WORK_G,x
CountWord_FG_0EBC_at0:
    rts

; @routine 0EBC:183 LoopFlagBit00EBC_183
; @run 70:B983
; WHAT IT DOES: Sets bit 0 of OBJ_LOOP_FLAGS (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
LoopFlagBit00EBC_183:
    lda a:OBJ_LOOP_FLAGS,x
    ora #$01
    sta a:OBJ_LOOP_FLAGS,x
    rts

; @routine 0EBC:18C LoopFlagBit00EBC_18C
; @run 70:B98C
; WHAT IT DOES: Clears bit 0 of OBJ_LOOP_FLAGS (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
LoopFlagBit00EBC_18C:
    lda a:OBJ_LOOP_FLAGS,x
    and #$FE
    sta a:OBJ_LOOP_FLAGS,x
    rts

; @routine 0EBD:714 LowerSlotInY20_0EBD
; @run 71:C714
; WHAT IT DOES: Moves the slot in Y 20 dots down, Y low only (Stage 09's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A the new Y low, flags from the add; X, Y kept.
LowerSlotInY20_0EBD:
    lda a:OBJ_Y_LO,y
    clc
    adc #$14
    sta a:OBJ_Y_LO,y
    rts

; @routine 101E:07C HalveWorkKOnHit101E
; @run 71:C87C
; WHAT IT DOES: A hit of 5 or more halves OBJ_WORK_K (Stage 0A's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
HalveWorkKOnHit101E:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc HalveWorkKOnHit101E_at0
    lsr a:OBJ_WORK_K,x
HalveWorkKOnHit101E_at0:
    rts

; @routine 101E:087 HitToState5_101E
; @run 71:C887
; WHAT IT DOES: Once hit, state 5 (Stage 0A's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
HitToState5_101E:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitToState5_101E_at0
    lda #$05
    sta a:OBJ_STATE,x
HitToState5_101E_at0:
    rts

; @routine 101F:68F StateOnWhenSlot30Free101F
; @run 71:D68F
; WHAT IT DOES: Counts the state on once slot $1E is empty (Stage 0A's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
StateOnWhenSlot30Free101F:
    lda a:OBJ_KIND+30
    bne StateOnWhenSlot30Free101F_at0
    inc a:OBJ_STATE,x
StateOnWhenSlot30Free101F_at0:
    rts

; @routine 10CC:350 Slot18HitIs6Or9_10CC
; @run 70:BB50
; WHAT IT DOES: C set while slot $12's hit is 6 or 9 (secret stage 0A's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: C the answer; A the hit; X, Y kept.
Slot18HitIs6Or9_10CC:
    lda a:OBJ_HIT_TAKEN+18
    cmp #$06
    beq Slot18HitIs6Or9_10CC_at0
    cmp #$09
    beq Slot18HitIs6Or9_10CC_at0
    clc
    rts
Slot18HitIs6Or9_10CC_at0:
    sec
    rts

; @routine 12D0:264 TileColumnAdd12D0_264
; @run 6B:9A64
; WHAT IT DOES: A tile handler of Stage 0C's overlay: adds the column's byte of $9AFF to zTemp02.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A $81; Y the column; C and V from the add; X kept.
TileColumnAdd12D0_264Steps = $9AFF ; a step per column
TileColumnAdd12D0_264:
    ldy z:zTemp00
    lda a:TileColumnAdd12D0_264Steps,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 12D0:271 TileRowStep12D0_271
; @run 6B:9A71
; WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9B07 by row zTemp02 and column zTemp00.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
TileRowStep12D0_271Rows = $9B07 ; new rows, 8 to a row
TileRowStep12D0_271:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowStep12D0_271Rows,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 12D0:282 TileRowStep12D0_282
; @run 6B:9A82
; WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9B47 by row zTemp02 and column zTemp00.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
TileRowStep12D0_282Rows = $9B47 ; new rows, 8 to a row
TileRowStep12D0_282:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowStep12D0_282Rows,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 12D0:2C0 TileColumnAdd12D0_2C0
; @run 6B:9AC0
; WHAT IT DOES: A tile handler of Stage 0C's overlay: adds the column's byte of $9B87 to zTemp02.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A $81; Y the column; C and V from the add; X kept.
TileColumnAdd12D0_2C0Steps = $9B87 ; a step per column
TileColumnAdd12D0_2C0:
    ldy z:zTemp00
    lda a:TileColumnAdd12D0_2C0Steps,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 12D0:2CD TileRowStep12D0_2CD
; @run 6B:9ACD
; WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9B8F by row zTemp02 and column zTemp00.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
TileRowStep12D0_2CDRows = $9B8F ; new rows, 8 to a row
TileRowStep12D0_2CD:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowStep12D0_2CDRows,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 12D0:2DE TileRowStep12D0_2DE
; @run 6B:9ADE
; WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9BCF by row zTemp02 and column zTemp00.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
TileRowStep12D0_2DERows = $9BCF ; new rows, 8 to a row
TileRowStep12D0_2DE:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowStep12D0_2DERows,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 12D0:431 TileRowStep12D0_431
; @run 6B:9C31
; WHAT IT DOES: A tile handler of Stage 0C's overlay: zTemp02 from the table at $9C42 by row zTemp02 and column zTemp00.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: A $81, N 1, Z 0; Y the index; C and V from the index add; X kept.
TileRowStep12D0_431Rows = $9C42 ; new rows, 8 to a row
TileRowStep12D0_431:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowStep12D0_431Rows,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 12DB:578 WaitThenSubstate12DB_578
; @run 70:B578
; WHAT IT DOES: OBJ_WORK_E 64, substate on.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
WaitThenSubstate12DB_578:
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 12DD:0FF AddWhileNotAt12DD
; @run 71:C0FF
; WHAT IT DOES: Steps OBJ_WORK_I by zTemp01 until it equals zTemp00 (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
AddWhileNotAt12DD:
    lda a:OBJ_WORK_I,x
    cmp z:zTemp00
    beq AddWhileNotAt12DD_at0
    lda a:OBJ_WORK_I,x
    clc
    adc z:zTemp01
    sta a:OBJ_WORK_I,x
AddWhileNotAt12DD_at0:
    rts

; @routine 12DD:578 SubstateOn12DD
; @run 71:C578
; WHAT IT DOES: The substate counted on (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
SubstateOn12DD:
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 12DD:5C2 RasterFromLineBF_12DD
; @run 71:C5C2
; WHAT IT DOES: Raster effect $0B from line $BF, substate on (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
RasterFromLineBF_12DD:
    lda #$0B
    sta z:zRasterEffect
    lda #$BF
    sta z:zRasterArg
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 12DF:016 Slot16FrameIn74To76_12DF
; @run 71:D016
; WHAT IT DOES: C set while slot $10's frame is $74 to $76 (Stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: Exit: C the answer; A the frame; X, Y kept.
Slot16FrameIn74To76_12DF:
    lda a:OBJ_FRAME+16
    cmp #$74
    bcc Slot16FrameIn74To76_12DF_at0
    cmp #$77
    bcs Slot16FrameIn74To76_12DF_at0
    sec
    rts
Slot16FrameIn74To76_12DF_at0:
    clc
    rts

; @routine 13FB:3CA StartOnScreen3_13FB
; @run 70:B3CA
; WHAT IT DOES: When the scroll reaches screen 3: OBJ_WORK_E 64, substate on (secret stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
StartOnScreen3_13FB:
    lda a:ScrollXHiScreen
    cmp #$03
    bne StartOnScreen3_13FB_at0
    lda #$40
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
StartOnScreen3_13FB_at0:
    rts

; @routine 13FB:5A0 SubstateByWorkK13FB
; @run 70:B5A0
; WHAT IT DOES: Substate 4 while OBJ_WORK_K is set, else the substate on (secret stage 0C's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
SubstateByWorkK13FB:
    lda a:OBJ_WORK_K,x
    beq SubstateByWorkK13FB_at0
    lda #$04
    sta a:OBJ_SUBSTATE,x
    rts
SubstateByWorkK13FB_at0:
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 13FC:53A WaitThenSubstate13FC_53A
; @run 70:BD3A
; WHAT IT DOES: OBJ_WORK_I 64, substate on.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
WaitThenSubstate13FC_53A:
    lda #$40
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 13FC:663 WaitThenSubstate13FC_663
; @run 70:BE63
; WHAT IT DOES: OBJ_WORK_I 240, substate on.
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
WaitThenSubstate13FC_663:
    lda #$F0
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 161D:3E3 CountDown161D_3E3
; @run 71:C3E3
; WHAT IT DOES: Counts OBJ_WORK_D; at zero restarts the animation and goes to state 2 (Stage 0E's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
CountDown161D_3E3:
    dec a:OBJ_WORK_D,x
    bne CountDown161D_3E3_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$02
    sta a:OBJ_STATE,x
CountDown161D_3E3_at0:
    rts

; @routine 16CB:63C Frame5Wait128_16CB
; @run 70:B63C
; WHAT IT DOES: OBJ_WORK_D 128, frame 5, substate on (secret stage 0E's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
Frame5Wait128_16CB:
    lda #$80
    sta a:OBJ_WORK_D,x
    lda #$05
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 18DB:21D CountDown18DB_21D
; @run 70:B21D
; WHAT IT DOES: Counts OBJ_WORK_E; at zero zRasterStage 4 and the substate on (Stage 10's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
CountDown18DB_21D:
    dec a:OBJ_WORK_E,x
    bne CountDown18DB_21D_at0
    lda #$04
    sta z:zRasterStage
    inc a:OBJ_SUBSTATE,x
CountDown18DB_21D_at0:
    rts

; @routine 18DB:63F CountDown18DB_63F
; @run 70:B63F
; WHAT IT DOES: Counts OBJ_WORK_E; at zero the substate on (Stage 10's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
CountDown18DB_63F:
    dec a:OBJ_WORK_E,x
    bne CountDown18DB_63F_at0
    inc a:OBJ_SUBSTATE,x
CountDown18DB_63F_at0:
    rts

; @routine 18DD:0C5 LeftOfSlot16_18DD
; @run 71:C0C5
; WHAT IT DOES: A 1 when the object's X low is under slot $10's, else 0 (Stage 10's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the answer, N 0, Z from it; C from the ROL; X kept.
LeftOfSlot16_18DD:
    lda a:OBJ_X_LO,x
    cmp a:OBJ_X_LO+16
    rol a
    and #$01
    eor #$01
    rts

; @routine 18DD:165 Raster1A_18DD
; @run 71:C165
; WHAT IT DOES: Raster effect $1A, state on (Stage 10's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
Raster1A_18DD:
    lda #$1A
    sta z:zRasterEffect
    inc a:OBJ_STATE,x
    rts

; @routine 18DD:16D RasterBack_18DD
; @run 71:C16D
; WHAT IT DOES: The raster effect counted down, state on (Stage 10's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
RasterBack_18DD:
    dec z:zRasterEffect
    inc a:OBJ_STATE,x
    rts

; @routine 18DD:1DF CountDown18DD_1DF
; @run 71:C1DF
; WHAT IT DOES: Counts OBJ_WORK_E; at zero the state and the raster effect on (Stage 10's overlay).
; HOW: Loads, compares and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last load, compare or count left them; X kept.
CountDown18DD_1DF:
    dec a:OBJ_WORK_E,x
    bne CountDown18DD_1DF_at0
    inc a:OBJ_STATE,x
    inc z:zRasterEffect
CountDown18DD_1DF_at0:
    rts
