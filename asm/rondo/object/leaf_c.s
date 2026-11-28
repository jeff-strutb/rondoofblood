; @routine 0A8D:66A StepField82Table0A8D
; @run 6B:866A
; WHAT IT DOES: Steps zField82 on and copies the byte of the overlay's table at that step into zField83 (Stage 06's
; $8000 overlay).
; HOW: INC, then a table read by the new step.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
StepField82Table0A8DBytes = $9CAC ; the byte for each step of zField82
StepField82Table0A8D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StepField82Table0A8DBytes,y
    sta z:zField83
    rts

; @routine 0A99:6BD BecomeKindInA0A99
; @run 70:A6BD
; WHAT IT DOES: Turns the object into the kind in A, with OBJ_WORK_D 3 and its state back to 0 (Stage 06's
; overlay).
; HOW: Three stores.
; WAITS: no.
; Entry: X the object; A the new kind. Exit: A, X, Y and the flags as its last instruction or call left them.
BecomeKindInA0A99:
    sta a:OBJ_KIND,x
    lda #$03
    sta a:OBJ_WORK_D,x
    stz a:OBJ_STATE,x
    rts

; @routine 0A9A:794 DriftWithScrollSpeed0A9A
; @run 70:AF94
; WHAT IT DOES: While the scroll moves (ScrollSpeedX not 0), moves the object's X by a step chosen by the speed's
; sign (Stage 06's overlay).
; HOW: Bit 7 of the speed rolled down to bit 1 gives the word's index; a 16-bit add to the X position.
; WAITS: no.
; Entry: X the object; C as the caller left it; D as the caller left it. Exit: A, X, Y and the flags as its last
; instruction or call left them.
DriftWithScrollSpeed0A9ASteps = $AFB3 ; two X steps, a word each: for a speed of 0 to $7F, then for $80 up
DriftWithScrollSpeed0A9A:
    lda a:ScrollSpeedX
    beq DriftWithScrollSpeed0A9A_at0
    rol a
    rol a
    and #$01
    asl a
    tay
    lda a:OBJ_X_LO,x
    clc
    adc a:DriftWithScrollSpeed0A9ASteps,y
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    adc a:DriftWithScrollSpeed0A9ASteps+1,y
    sta a:OBJ_X_HI,x
DriftWithScrollSpeed0A9A_at0:
    rts

; @routine 0A9A:7F9 CountDownThenState0A9A
; @run 70:AFF9
; WHAT IT DOES: Counts OBJ_WORK_D down; once it is zero, steps the object's state on (Stage 06's overlay).
; HOW: A load, then DEC or INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountDownThenState0A9A:
    lda a:OBJ_WORK_D,x
    beq CountDownThenState0A9A_at0
    dec a:OBJ_WORK_D,x
    rts
CountDownThenState0A9A_at0:
    inc a:OBJ_STATE,x
    rts

; @routine 0A9B:312 CountKind2CFrom30_0A9B
; @run 70:B312
; WHAT IT DOES: Counts the objects of kind $2C in slots $30 to $3F into zTemp00 and leaves the count in A (Stage
; 06's overlay).
; HOW: A loop over the slots with a compare of each kind.
; WAITS: no.
; Entry: nothing in the registers. Exit: A and zTemp00 the count, N and Z from it; Y $40; C set from the last
; compare.
CountKind2CFrom30_0A9B:
    stz z:zTemp00
    ldy #$30
CountKind2CFrom30_0A9B_at1:
    lda a:OBJ_KIND,y
    cmp #$2C
    bne CountKind2CFrom30_0A9B_at0
    inc z:zTemp00
CountKind2CFrom30_0A9B_at0:
    iny
    cpy #$40
    bcc CountKind2CFrom30_0A9B_at1
    lda z:zTemp00
    rts

; @routine 0A9B:33F CheckPartnerOrHit0A9B
; @run 70:B33F
; WHAT IT DOES: With OBJ_WORK_I not 5, puts the object in state 3 unless the slot 5 less OBJ_WORK_I after it holds
; kind $2B; with 5, once it has been hit (OBJ_HIT_STATUS bits 5 to 7) it becomes kind $2C in state 0 (Stage 06's
; overlay).
; HOW: A compare of OBJ_WORK_I; the slot found by an add of X; stores.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
CheckPartnerOrHit0A9B:
    lda a:OBJ_WORK_I,x
    cmp #$05
    beq CheckPartnerOrHit0A9B_at0
    lda #$05
    sec
    sbc a:OBJ_WORK_I,x
    sta z:zTemp00
    txa
    clc
    adc z:zTemp00
    tay
    lda a:OBJ_KIND,y
    cmp #$2B
    beq CheckPartnerOrHit0A9B_at1
    lda #$03
    sta a:OBJ_STATE,x
CheckPartnerOrHit0A9B_at1:
    rts
CheckPartnerOrHit0A9B_at0:
    lda a:OBJ_HIT_STATUS,x
    and #$E0
    beq CheckPartnerOrHit0A9B_at1
    lda #$2C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts

; @routine 0A9C:118 WaitForSlotThenFind0A9C
; @run 70:B918
; WHAT IT DOES: In substate 0 waits for the slot in OBJ_START_ARG to be empty, then counts OBJ_WORK_P down from 0;
; when the count runs out looks in slots $10 to $4F for kind $C9 and keeps its slot in OBJ_START_ARG (Stage 06's
; overlay).
; HOW: Two arms by the substate; a search loop.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WaitForSlotThenFind0A9C:
    lda a:OBJ_SUBSTATE,x
    bne WaitForSlotThenFind0A9C_at0
    ldy a:OBJ_START_ARG,x
    lda a:OBJ_KIND,y
    bne WaitForSlotThenFind0A9C_at1
    stz a:OBJ_START_ARG,x
    stz a:OBJ_WORK_P,x
    inc a:OBJ_SUBSTATE,x
WaitForSlotThenFind0A9C_at1:
    rts
WaitForSlotThenFind0A9C_at0:
    dec a:OBJ_WORK_P,x
    bne WaitForSlotThenFind0A9C_at2
    ldy #$10
WaitForSlotThenFind0A9C_at4:
    lda a:OBJ_KIND,y
    cmp #$C9
    beq WaitForSlotThenFind0A9C_at3
    iny
    cpy #$50
    bne WaitForSlotThenFind0A9C_at4
WaitForSlotThenFind0A9C_at2:
    rts
WaitForSlotThenFind0A9C_at3:
    tya
    sta a:OBJ_START_ARG,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 0A9D:737 FlipBelowRow50AtRight0A9D
; @run 71:C737
; WHAT IT DOES: Clears OBJ_SPRITE_FLIP unless the object is at Y $50 or more and its screen X is $830 or more; then
; sets it from a table by the state (Stage 06's overlay).
; HOW: A compare of Y, a 16-bit screen X from the scroll and two compares, a table read.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FlipBelowRow50AtRight0A9DFlips = $C767 ; the sprite flip byte by state
FlipBelowRow50AtRight0A9D:
    lda a:OBJ_Y_LO,x
    cmp #$50
    bcs FlipBelowRow50AtRight0A9D_at0
FlipBelowRow50AtRight0A9D_at1:
    stz a:OBJ_SPRITE_FLIP,x
    rts
FlipBelowRow50AtRight0A9D_at0:
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollXLo
    sta z:zTemp00
    lda a:OBJ_X_HI,x
    adc a:ScrollXHiScreen
    cmp #$08
    bcc FlipBelowRow50AtRight0A9D_at1
    bne FlipBelowRow50AtRight0A9D_at2
    lda z:zTemp00
    cmp #$30
    bcc FlipBelowRow50AtRight0A9D_at1
FlipBelowRow50AtRight0A9D_at2:
    ldy a:OBJ_STATE,x
    lda a:FlipBelowRow50AtRight0A9DFlips,y
    sta a:OBJ_SPRITE_FLIP,x
    rts

; @routine 0A9E:07E SetSpeedByFacingAndStep0A9E
; @run 71:C87E
; WHAT IT DOES: Sets the object's X and Y speeds from a table of four-byte records chosen by its facing and
; OBJ_WORK_D, then steps the state on (Stage 06's overlay).
; HOW: The facing's base plus four times OBJ_WORK_D indexes the records.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
SetSpeedByFacingAndStep0A9EBases = $C8AB ; the record index for each facing
SetSpeedByFacingAndStep0A9ESpeeds = $C8AD ; X speed low, high, Y speed low, high per record
SetSpeedByFacingAndStep0A9E:
    ldy a:OBJ_FACING_LEFT,x
    lda a:SetSpeedByFacingAndStep0A9EBases,y
    sta z:zTemp00
    lda a:OBJ_WORK_D,x
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:SetSpeedByFacingAndStep0A9ESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:SetSpeedByFacingAndStep0A9ESpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:SetSpeedByFacingAndStep0A9ESpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:SetSpeedByFacingAndStep0A9ESpeeds+2,y
    sta a:OBJ_VY_LO,x
    inc a:OBJ_STATE,x
    rts

; @routine 0A9E:785 RepeatOrStand0A9E
; @run 71:CF85
; WHAT IT DOES: Counts OBJ_WORK_K down: while it runs, restarts the animation in substate 1; when it ends, state 1
; with frame $80, or $83 when OBJ_WORK_J is set (Stage 06's overlay).
; HOW: DEC and two arms of stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RepeatOrStand0A9E:
    dec a:OBJ_WORK_K,x
    beq RepeatOrStand0A9E_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$01
    sta a:OBJ_SUBSTATE,x
    rts
RepeatOrStand0A9E_at0:
    lda #$01
    sta a:OBJ_STATE,x
    lda #$80
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_J,x
    beq RepeatOrStand0A9E_at1
    lda #$83
    sta a:OBJ_FRAME,x
RepeatOrStand0A9E_at1:
    rts

; @routine 0A9F:197 HitBoxByFrame0A9F_197
; @run 71:D197
; WHAT IT DOES: Gives the object the hit box of its frame from $92 on (Stage 06's overlay).
; HOW: Four times the frame less $92 indexes four-byte records: X and Y offset, half width, half height.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HitBoxByFrame0A9F_197Boxes = $D1B9 ; X offset, Y offset, half width, half height per frame
HitBoxByFrame0A9F_197:
    lda a:OBJ_FRAME,x
    sec
    sbc #$92
    asl a
    asl a
    tay
    lda a:HitBoxByFrame0A9F_197Boxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:HitBoxByFrame0A9F_197Boxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:HitBoxByFrame0A9F_197Boxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:HitBoxByFrame0A9F_197Boxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0A9F:5A7 StaggerWhenHit0A9F
; @run 71:D5A7
; WHAT IT DOES: In the states a table marks, an object hit 5 times or more restarts its animation in state 7 (Stage
; 06's overlay).
; HOW: A table read by the state, a compare of OBJ_HIT_TAKEN, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StaggerWhenHit0A9FStates = $D5C2 ; nonzero for the states in which a hit counts
StaggerWhenHit0A9F:
    ldy a:OBJ_STATE,x
    lda a:StaggerWhenHit0A9FStates,y
    beq StaggerWhenHit0A9F_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc StaggerWhenHit0A9F_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$07
StaggerWhenHit0A9F_at1:
    sta a:OBJ_STATE,x
StaggerWhenHit0A9F_at0:
    rts

; @routine 0A9F:5F4 HitBoxByFrame0A9F_5F4
; @run 71:D5F4
; WHAT IT DOES: Gives the object the hit box of its frame $AD to $CC; from frame $CD on it has none (Stage 06's
; overlay).
; HOW: A compare, then four times the frame less $AD indexes four-byte records.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HitBoxByFrame0A9F_5F4Boxes = $DC3D ; X offset, Y offset, half width, half height per frame
HitBoxByFrame0A9F_5F4:
    lda a:OBJ_FRAME,x
    cmp #$CD
    bcc HitBoxByFrame0A9F_5F4_at0
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_HALF_H,x
    rts
HitBoxByFrame0A9F_5F4_at0:
    sec
    sbc #$AD
    asl a
    asl a
    tay
    lda a:HitBoxByFrame0A9F_5F4Boxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:HitBoxByFrame0A9F_5F4Boxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:HitBoxByFrame0A9F_5F4Boxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:HitBoxByFrame0A9F_5F4Boxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0A9F:726 NextPatternStep0A9F
; @run 71:D726
; WHAT IT DOES: Restarts the animation and takes the next step of an eight-step pattern: its substate and
; OBJ_WORK_C by OBJ_WORK_K (Stage 06's overlay).
; HOW: OBJ_WORK_K's low three bits index the substates; the substate indexes OBJ_WORK_C's values.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
NextPatternStep0A9FSubstates = $D743 ; the substate for each of the eight steps
NextPatternStep0A9FWorkC = $D74B ; OBJ_WORK_C for each substate
NextPatternStep0A9F:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_WORK_K,x
    and #$07
    tay
    lda a:NextPatternStep0A9FSubstates,y
    sta a:OBJ_SUBSTATE,x
    tay
    lda a:NextPatternStep0A9FWorkC,y
    sta a:OBJ_WORK_C,x
    inc a:OBJ_WORK_K,x
    rts

; @routine 0AA0:4DC State4WhenHitTwice0AA0
; @run 71:DCDC
; WHAT IT DOES: Puts an object hit twice or more in state 4, substate 0, unless it is in state 4 already (Stage
; 06's overlay).
; HOW: Two compares, two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
State4WhenHitTwice0AA0:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$02
    bcc State4WhenHitTwice0AA0_at0
    lda a:OBJ_STATE,x
    cmp #$04
    beq State4WhenHitTwice0AA0_at0
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State4WhenHitTwice0AA0_at0:
    rts

; @routine 0AA0:4F3 StateOnAtColumn0AA0
; @run 71:DCF3
; WHAT IT DOES: Steps the state on when the object stands in the first screen column its OBJ_START_ARG names (X
; high byte 0, X low byte's top four bits equal to it) (Stage 06's overlay).
; HOW: Two compares and INC.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StateOnAtColumn0AA0:
    lda a:OBJ_X_HI,x
    bne StateOnAtColumn0AA0_at0
    lda a:OBJ_X_LO,x
    and #$F0
    cmp a:OBJ_START_ARG,x
    bne StateOnAtColumn0AA0_at0
    inc a:OBJ_STATE,x
StateOnAtColumn0AA0_at0:
    rts

; @routine 0BED:66A StepField82Table0BED
; @run 6B:866A
; WHAT IT DOES: Steps zField82 on and copies the byte of the overlay's table at that step into zField83 (Stage 07's
; $8000 overlay).
; HOW: INC, then a table read by the new step.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
StepField82Table0BEDBytes = $9CC4 ; the byte for each step of zField82
StepField82Table0BED:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StepField82Table0BEDBytes,y
    sta z:zField83
    rts

; @routine 0BFB:006 ReturnCarrySet0BFB
; @run 70:B006
; WHAT IT DOES: Returns with the carry set (Stage 07's overlay).
; HOW: SEC.
; WAITS: no.
; Entry: nothing in the registers. Exit: C set; A, X, Y and the other flags kept.
ReturnCarrySet0BFB:
    sec
    rts

; @routine 0BFB:51F OpenWhenGuardsDown0BFB
; @run 70:B51F
; WHAT IT DOES: In states 4 and 5, once no object of kind $2D in slots $49 to $4B is in a state other than 2, takes
; the hit box off, steps the next three slots' states on and goes to state 6, substate 0 (Stage 07's overlay).
; HOW: Two compares of the state, a search loop, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
OpenWhenGuardsDown0BFB:
    lda a:OBJ_STATE,x
    cmp #$04
    bcc OpenWhenGuardsDown0BFB_at0
    cmp #$06
    bcs OpenWhenGuardsDown0BFB_at0
    ldy #$49
OpenWhenGuardsDown0BFB_at2:
    lda a:OBJ_KIND,y
    cmp #$2D
    bne OpenWhenGuardsDown0BFB_at1
    lda a:OBJ_STATE,y
    cmp #$02
    beq OpenWhenGuardsDown0BFB_at1
    rts
OpenWhenGuardsDown0BFB_at1:
    iny
    cpy #$4C
    bcc OpenWhenGuardsDown0BFB_at2
    stz a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE+$01,x
    inc a:OBJ_STATE+$02,x
    inc a:OBJ_STATE+$03,x
    lda #$06
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
OpenWhenGuardsDown0BFB_at0:
    rts

; @routine 0BFB:555 State5WhenHitInState4_0BFB
; @run 70:B555
; WHAT IT DOES: An object hit twice or more while in state 4 goes to state 5, substate 0 (Stage 07's overlay).
; HOW: Two compares, two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
State5WhenHitInState4_0BFB:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$02
    bcc State5WhenHitInState4_0BFB_at0
    lda a:OBJ_STATE,x
    cmp #$04
    bne State5WhenHitInState4_0BFB_at0
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State5WhenHitInState4_0BFB_at0:
    rts

; @routine 0BFB:570 RiseAtScreen3_0BFB
; @run 70:B570
; WHAT IT DOES: When the scroll reaches screen 3, holds the scroll's X limits at screen 3, starts the object rising
; at -$60 a frame and steps its state on (Stage 07's overlay).
; HOW: A compare of ScrollXHiScreen, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RiseAtScreen3_0BFB:
    lda a:ScrollXHiScreen
    cmp #$03
    bne RiseAtScreen3_0BFB_at0
    lda #$03
    sta a:ScrollXMinHi
    sta a:ScrollXMaxHi
    lda #$FF
    sta a:OBJ_VY_HI,x
    lda #$A0
    sta a:OBJ_VY_LO,x
    inc a:OBJ_STATE,x
RiseAtScreen3_0BFB_at0:
    rts

; @routine 0BFB:732 CountThenFall0BFB
; @run 70:B732
; WHAT IT DOES: Counts OBJ_WORK_H down; when it runs out, starts the object falling at 1 pixel a frame and steps
; the substate on (Stage 07's overlay).
; HOW: DEC, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountThenFall0BFB:
    dec a:OBJ_WORK_H,x
    bne CountThenFall0BFB_at0
    lda #$01
    sta a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_SUBSTATE,x
CountThenFall0BFB_at0:
    rts

; @routine 0BFD:03B HitBoxXByFrame0BFD
; @run 71:C03B
; WHAT IT DOES: Turns the hit box on with an X offset by frame for frames up to $61; from $62 on, the box is off
; (Stage 07's overlay).
; HOW: A compare of the frame, a table read by the frame less $5C.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HitBoxXByFrame0BFDOffsets = $C059 ; the hit box's X offset for frames $5C to $61
HitBoxXByFrame0BFD:
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_FRAME,x
    cmp #$62
    bcs HitBoxXByFrame0BFD_at0
    sec
    sbc #$5C
    tay
    lda a:HitBoxXByFrame0BFDOffsets,y
    sta a:OBJ_HITBOX_DX,x
    rts
HitBoxXByFrame0BFD_at0:
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_HITBOX_DX,x
    rts

; @routine 0BFD:2A5 RepeatOrStand0BFD
; @run 71:C2A5
; WHAT IT DOES: Counts OBJ_WORK_K down: while it runs, restarts the animation in substate 1; when it ends, state 1
; with frame $5C, or $5F when OBJ_WORK_J is set (Stage 07's overlay).
; HOW: DEC and two arms of stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RepeatOrStand0BFD:
    dec a:OBJ_WORK_K,x
    beq RepeatOrStand0BFD_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$01
    sta a:OBJ_SUBSTATE,x
    rts
RepeatOrStand0BFD_at0:
    lda #$01
    sta a:OBJ_STATE,x
    lda #$5C
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_J,x
    beq RepeatOrStand0BFD_at1
    lda #$5F
    sta a:OBJ_FRAME,x
RepeatOrStand0BFD_at1:
    rts

; @routine 0BFD:4B7 HitBoxByFrame0BFD
; @run 71:C4B7
; WHAT IT DOES: Gives the object the hit box of its frame from $6E on (Stage 07's overlay).
; HOW: Four times the frame less $6E indexes four-byte records.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HitBoxByFrame0BFDBoxes = $C4D9 ; X offset, Y offset, half width, half height per frame
HitBoxByFrame0BFD:
    lda a:OBJ_FRAME,x
    sec
    sbc #$6E
    asl a
    asl a
    tay
    lda a:HitBoxByFrame0BFDBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:HitBoxByFrame0BFDBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:HitBoxByFrame0BFDBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:HitBoxByFrame0BFDBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0BFD:57D MarkPartsByBlink0BFD
; @run 71:C57D
; WHAT IT DOES: In state 1 writes 2 (while OBJ_INVULN_TIMER runs) or 0 into OBJ_WORK_J of the object's slot and the
; nine after it, and halves OBJ_WORK_O once it has been hit 5 times or more (Stage 07's overlay).
; HOW: A loop of ten stores from Y = X; a compare and LSR.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
MarkPartsByBlink0BFD:
    lda a:OBJ_STATE,x
    cmp #$01
    bne MarkPartsByBlink0BFD_at0
    stz z:zTemp00
    lda a:OBJ_INVULN_TIMER,x
    beq MarkPartsByBlink0BFD_at1
    lda #$02
    sta z:zTemp00
MarkPartsByBlink0BFD_at1:
    txa
    tay
    lda #$0A
    sta z:zTemp01
MarkPartsByBlink0BFD_at2:
    lda z:zTemp00
    sta a:OBJ_WORK_J,y
    iny
    dec z:zTemp01
    bne MarkPartsByBlink0BFD_at2
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc MarkPartsByBlink0BFD_at0
    lsr a:OBJ_WORK_O,x
MarkPartsByBlink0BFD_at0:
    rts

; @routine 0BFD:77B EaseWorkFToTarget0BFD
; @run 71:C77B
; WHAT IT DOES: Moves the 16-bit value OBJ_WORK_F:OBJ_WORK_G toward the target of its step (OBJ_WORK_J) by that
; step's speed, up or down, and stops on the target (Stage 07's overlay).
; HOW: Twice OBJ_WORK_J indexes the targets and the speeds; a compare picks the direction; 16-bit adds or
; subtracts.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
EaseWorkFToTarget0BFDTargets = $C7CF ; the target high byte per step, two bytes apart
EaseWorkFToTarget0BFDSpeeds = $C7D4 ; the speed per step, a word each
EaseWorkFToTarget0BFD:
    lda a:OBJ_WORK_J,x
    asl a
    tay
    lda a:OBJ_WORK_F,x
    cmp a:EaseWorkFToTarget0BFDTargets,y
    bcc EaseWorkFToTarget0BFD_at0
    bne EaseWorkFToTarget0BFD_at1
    lda a:OBJ_WORK_G,x
    bne EaseWorkFToTarget0BFD_at1
    rts
EaseWorkFToTarget0BFD_at0:
    lda a:OBJ_WORK_G,x
    clc
    adc a:EaseWorkFToTarget0BFDSpeeds,y
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    adc a:EaseWorkFToTarget0BFDSpeeds+1,y
    sta a:OBJ_WORK_F,x
    cmp a:EaseWorkFToTarget0BFDTargets,y
    bne EaseWorkFToTarget0BFD_at2
    stz a:OBJ_WORK_G,x
EaseWorkFToTarget0BFD_at2:
    rts
EaseWorkFToTarget0BFD_at1:
    lda a:OBJ_WORK_G,x
    sec
    sbc a:EaseWorkFToTarget0BFDSpeeds,y
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    sbc a:EaseWorkFToTarget0BFDSpeeds+1,y
    sta a:OBJ_WORK_F,x
    cmp a:EaseWorkFToTarget0BFDTargets,y
    bcs EaseWorkFToTarget0BFD_at2
    lda a:EaseWorkFToTarget0BFDTargets,y
    sta a:OBJ_WORK_F,x
    stz a:OBJ_WORK_G,x
    rts

; @routine 0BFE:02D SetNineAfterToState2_0BFE
; @run 71:C82D
; WHAT IT DOES: Puts the nine slots after the object's in state 2, substate 0 (Stage 07's overlay).
; HOW: A loop from Y = X + 1.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetNineAfterToState2_0BFE:
    txa
    tay
    iny
    lda #$09
    sta z:zTemp01
SetNineAfterToState2_0BFE_at0:
    lda #$02
    sta a:OBJ_STATE,y
    cla
    sta a:OBJ_SUBSTATE,y
    iny
    dec z:zTemp01
    bne SetNineAfterToState2_0BFE_at0
    rts

; @routine 0BFE:096 ClampA50ToB0_0BFE
; @run 71:C896
; WHAT IT DOES: Clamps A to $50 to $B0 (Stage 07's overlay).
; HOW: Two compares.
; WAITS: no.
; Entry: A the value. Exit: A the value clamped; N, Z and C from the last compare or load; X and Y kept.
ClampA50ToB0_0BFE:
    cmp #$50
    bcc ClampA50ToB0_0BFE_at0
    cmp #$B0
    bcs ClampA50ToB0_0BFE_at1
    rts
ClampA50ToB0_0BFE_at0:
    lda #$50
    rts
ClampA50ToB0_0BFE_at1:
    lda #$B0
    rts

; @routine 0BFE:124 SetNineAfterToState3_0BFE
; @run 71:C924
; WHAT IT DOES: Puts the nine slots after the object's in state 3 with the object's OBJ_START_ARG in their
; OBJ_WORK_L (Stage 07's overlay).
; HOW: Y = X + 1, then the loop that follows.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetNineAfterToState3_0BFE:
    txa
    tay
    iny

; @routine 0BFE:127 SetNineFromYToState3_0BFE
; @run 71:C927
; WHAT IT DOES: Puts nine slots from Y on in state 3, substate 0, with the object's OBJ_START_ARG in their
; OBJ_WORK_L (Stage 07's overlay).
; HOW: A loop of nine.
; WAITS: no.
; Entry: X the object; Y the first slot. Exit: A, X, Y and the flags as its last instruction or call left them.
SetNineFromYToState3_0BFE:
    lda #$09
    sta z:zTemp01
SetNineFromYToState3_0BFE_at0:
    lda #$03
    sta a:OBJ_STATE,y
    lda a:OBJ_START_ARG,x
    sta a:OBJ_WORK_L,y
    cla
    sta a:OBJ_SUBSTATE,y
    iny
    dec z:zTemp01
    bne SetNineFromYToState3_0BFE_at0
    rts

; @routine 0BFE:1E6 WorkIByStep0BFE
; @run 71:C9E6
; WHAT IT DOES: Sets OBJ_WORK_I from a table by OBJ_WORK_K and steps the substate on (Stage 07's overlay).
; HOW: A table read.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WorkIByStep0BFEValues = $C9F3 ; OBJ_WORK_I for each value of OBJ_WORK_K
WorkIByStep0BFE:
    ldy a:OBJ_WORK_K,x
    lda a:WorkIByStep0BFEValues,y
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0BFE:229 CountThenRewind0BFE
; @run 71:CA29
; WHAT IT DOES: Counts OBJ_WORK_I down; when it runs out, reloads it with $30, sets OBJ_WORK_J and steps the
; substate on (Stage 07's overlay).
; HOW: DEC, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
CountThenRewind0BFE:
    dec a:OBJ_WORK_I,x
    bne CountThenRewind0BFE_at0
    lda #$30
    sta a:OBJ_WORK_I,x
    lda #$01
    sta a:OBJ_WORK_J,x
    inc a:OBJ_SUBSTATE,x
CountThenRewind0BFE_at0:
    rts

; @routine 0BFE:27C LaunchByStep0BFE
; @run 71:CA7C
; WHAT IT DOES: Sets the object's X and Y speeds from a four-byte record by OBJ_WORK_K, takes its hit box off,
; clears OBJ_PACE bits 3 to 6 and steps the substate on (Stage 07's overlay).
; HOW: Four times OBJ_WORK_K indexes the records.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LaunchByStep0BFESpeeds = $CAA9 ; X speed low, high, Y speed low, high per step
LaunchByStep0BFE:
    lda a:OBJ_WORK_K,x
    asl a
    asl a
    tay
    lda a:LaunchByStep0BFESpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LaunchByStep0BFESpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:LaunchByStep0BFESpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:LaunchByStep0BFESpeeds+2,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_PACE,x
    and #$87
    sta a:OBJ_PACE,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0BFE:58A PickPhaseOfFour0BFE
; @run 71:CD8A
; WHAT IT DOES: Takes the substate and OBJ_WORK_H of the next of four phases by OBJ_WORK_F, and steps OBJ_WORK_F on
; (Stage 07's overlay).
; HOW: OBJ_WORK_F's low two bits index two tables.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PickPhaseOfFour0BFESubstates = $CDA0 ; the substate of each phase
PickPhaseOfFour0BFEWorkH = $CDA4 ; OBJ_WORK_H of each phase
PickPhaseOfFour0BFE:
    lda a:OBJ_WORK_F,x
    and #$03
    tay
    lda a:PickPhaseOfFour0BFESubstates,y
    sta a:OBJ_SUBSTATE,x
    lda a:PickPhaseOfFour0BFEWorkH,y
    sta a:OBJ_WORK_H,x
    inc a:OBJ_WORK_F,x
    rts

; @routine 0BFE:5A8 SetFrame89Wait28_0BFE
; @run 71:CDA8
; WHAT IT DOES: Sets frame $89 and a wait of $28 in OBJ_WORK_D, and steps the substate on (Stage 07's overlay).
; HOW: Stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetFrame89Wait28_0BFE:
    lda #$28
    sta a:OBJ_WORK_D,x
    lda #$89
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0BFE:785 HitBoxOffWait40_0BFE
; @run 71:CF85
; WHAT IT DOES: Takes the hit box off, sets OBJ_WORK_K to $40 and steps the substate on (Stage 07's overlay).
; HOW: Stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
HitBoxOffWait40_0BFE:
    stz a:OBJ_HITBOX_ON,x
    lda #$40
    sta a:OBJ_WORK_K,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0BFF:4C1 StateOnAndMarkField8C0BFF
; @run 71:D4C1
; WHAT IT DOES: Steps the state on and sets bit 7 of zField8C (Stage 07's overlay).
; HOW: INC, ORA.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StateOnAndMarkField8C0BFF:
    inc a:OBJ_STATE,x
    lda z:zField8C
    ora #$80
    sta z:zField8C
    rts

; @routine 0D2D:29C SetUpByStartArg0D2D
; @run 81:C29C
; WHAT IT DOES: Starts the object: sprite set 3, hit box on, health $80, state on; the record five on when
; zCharacter is not 0, or when it is 0 and zField93's byte (by a table) is the record's key; then its hit box,
; OBJ_WORK_D and OBJ_WORK_E from the record of OBJ_START_ARG (Stage 08's overlay).
; HOW: The record index in OBJ_START_ARG; a compare through two tables; table reads by twice the index and by the
; index.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetUpByStartArg0D2DWaits = $C1BC ; OBJ_WORK_D and OBJ_WORK_E per record
SetUpByStartArg0D2DKeys = $C1D0 ; the byte each record is kept for
SetUpByStartArg0D2DSizes = $C1E4 ; half width, half height per record
SetUpByStartArg0D2DOffsets = $C20C ; X offset, Y offset per record
SetUpByStartArg0D2DByField93 = $C374 ; the byte compared for each value of zField93
SetUpByStartArg0D2D:
    lda #$03
    sta a:OBJ_SPRITE_SET,x
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    lda #$80
    sta a:OBJ_HEALTH,x
    inc a:OBJ_STATE,x
    lda z:zCharacter
    bne SetUpByStartArg0D2D_at0
    ldy a:OBJ_START_ARG,x
    phx
    ldx z:zField93
    lda a:SetUpByStartArg0D2DByField93,x
    plx
    cmp a:SetUpByStartArg0D2DKeys,y
    bne SetUpByStartArg0D2D_at1
SetUpByStartArg0D2D_at0:
    lda a:OBJ_START_ARG,x
    clc
    adc #$05
    sta a:OBJ_START_ARG,x
SetUpByStartArg0D2D_at1:
    lda a:OBJ_START_ARG,x
    asl a
    tay
    lda a:SetUpByStartArg0D2DSizes,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:SetUpByStartArg0D2DSizes+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:SetUpByStartArg0D2DOffsets,y
    sta a:OBJ_HITBOX_DX,x
    lda a:SetUpByStartArg0D2DOffsets+1,y
    sta a:OBJ_HITBOX_DY,x
    ldy a:OBJ_START_ARG,x
    lda a:SetUpByStartArg0D2DWaits,y
    sta a:OBJ_WORK_D,x
    sta a:OBJ_WORK_E,x
    rts

; @routine 0D4D:66A StepField82Table0D4D
; @run 6B:866A
; WHAT IT DOES: Steps zField82 on and copies the byte of the overlay's table at that step into zField83 (Stage 08's
; $8000 overlay).
; HOW: INC, then a table read by the new step.
; WAITS: no.
; Entry: nothing in the registers. Exit: A, X, Y and the flags as its last instruction or call left them.
StepField82Table0D4DBytes = $9CE7 ; the byte for each step of zField82
StepField82Table0D4D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:StepField82Table0D4DBytes,y
    sta z:zField83
    rts

; @routine 0D5B:312 State4WhenHit0D5B
; @run 70:B312
; WHAT IT DOES: Once the object is hit (OBJ_HIT_STATUS bit 7) in a state below 4, puts it in state 4, substate 0
; (Stage 08's overlay).
; HOW: Two tests, two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
State4WhenHit0D5B:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq State4WhenHit0D5B_at0
    lda a:OBJ_STATE,x
    cmp #$04
    bcs State4WhenHit0D5B_at0
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
State4WhenHit0D5B_at0:
    rts

; @routine 0D5B:527 EaseWorkFToTarget0D5B
; @run 70:B527
; WHAT IT DOES: Moves the 16-bit value OBJ_WORK_F:OBJ_WORK_G toward the target of its step (OBJ_WORK_J) by that
; step's speed, up or down, and stops on the target (Stage 08's overlay).
; HOW: Twice OBJ_WORK_J indexes the targets and the speeds; a compare picks the direction; 16-bit adds or
; subtracts.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
EaseWorkFToTarget0D5BTargets = $B57B ; the target high byte per step, two bytes apart
EaseWorkFToTarget0D5BSpeeds = $B580 ; the speed per step, a word each
EaseWorkFToTarget0D5B:
    lda a:OBJ_WORK_J,x
    asl a
    tay
    lda a:OBJ_WORK_F,x
    cmp a:EaseWorkFToTarget0D5BTargets,y
    bcc EaseWorkFToTarget0D5B_at0
    bne EaseWorkFToTarget0D5B_at1
    lda a:OBJ_WORK_G,x
    bne EaseWorkFToTarget0D5B_at1
    rts
EaseWorkFToTarget0D5B_at0:
    lda a:OBJ_WORK_G,x
    clc
    adc a:EaseWorkFToTarget0D5BSpeeds,y
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    adc a:EaseWorkFToTarget0D5BSpeeds+1,y
    sta a:OBJ_WORK_F,x
    cmp a:EaseWorkFToTarget0D5BTargets,y
    bne EaseWorkFToTarget0D5B_at2
    stz a:OBJ_WORK_G,x
EaseWorkFToTarget0D5B_at2:
    rts
EaseWorkFToTarget0D5B_at1:
    lda a:OBJ_WORK_G,x
    sec
    sbc a:EaseWorkFToTarget0D5BSpeeds,y
    sta a:OBJ_WORK_G,x
    lda a:OBJ_WORK_F,x
    sbc a:EaseWorkFToTarget0D5BSpeeds+1,y
    sta a:OBJ_WORK_F,x
    cmp a:EaseWorkFToTarget0D5BTargets,y
    bcs EaseWorkFToTarget0D5B_at2
    lda a:EaseWorkFToTarget0D5BTargets,y
    sta a:OBJ_WORK_F,x
    stz a:OBJ_WORK_G,x
    rts

; @routine 0D5B:79B WorkIByStep0D5B
; @run 70:B79B
; WHAT IT DOES: Sets OBJ_WORK_I from a table by OBJ_WORK_K and steps the substate on (Stage 08's overlay).
; HOW: A table read.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WorkIByStep0D5BValues = $B7A8 ; OBJ_WORK_I for each value of OBJ_WORK_K
WorkIByStep0D5B:
    ldy a:OBJ_WORK_K,x
    lda a:WorkIByStep0D5BValues,y
    sta a:OBJ_WORK_I,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0D5C:031 LaunchByStep0D5C
; @run 70:B831
; WHAT IT DOES: Sets the object's X and Y speeds from a four-byte record by OBJ_WORK_K, takes its hit box off,
; clears OBJ_PACE bits 3 to 6 and steps the substate on (Stage 08's overlay).
; HOW: Four times OBJ_WORK_K indexes the records.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LaunchByStep0D5CSpeeds = $B85E ; X speed low, high, Y speed low, high per step
LaunchByStep0D5C:
    lda a:OBJ_WORK_K,x
    asl a
    asl a
    tay
    lda a:LaunchByStep0D5CSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:LaunchByStep0D5CSpeeds,y
    sta a:OBJ_VX_LO,x
    lda a:LaunchByStep0D5CSpeeds+3,y
    sta a:OBJ_VY_HI,x
    lda a:LaunchByStep0D5CSpeeds+2,y
    sta a:OBJ_VY_LO,x
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_PACE,x
    and #$87
    sta a:OBJ_PACE,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0D5C:0BC BecomeWorkLKind0D5C
; @run 70:B8BC
; WHAT IT DOES: Turns the object into the kind in its OBJ_WORK_L, state 0 (Stage 08's overlay).
; HOW: Two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BecomeWorkLKind0D5C:
    lda a:OBJ_WORK_L,x
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts

; @routine 0D5C:0E8 BreakUnlessLeaderAlive0D5C
; @run 70:B8E8
; WHAT IT DOES: Unless the slot in OBJ_WORK_D holds a kind $22 in a state other than 4, turns the object into kind
; $9C, state 0, and returns with C set; else with C clear (Stage 08's overlay).
; HOW: Two compares, stores.
; WAITS: no.
; Entry: X the object. Exit: C set when the object was turned; A, N and Z from the last compare or store; X kept.
BreakUnlessLeaderAlive0D5C:
    ldy a:OBJ_WORK_D,x
    lda a:OBJ_KIND,y
    cmp #$22
    bne BreakUnlessLeaderAlive0D5C_at0
    lda a:OBJ_STATE,y
    cmp #$04
    bne BreakUnlessLeaderAlive0D5C_at1
BreakUnlessLeaderAlive0D5C_at0:
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    sec
    rts
BreakUnlessLeaderAlive0D5C_at1:
    clc
    rts

; @routine 0D5C:20E BreakUpWhenHit0D5C
; @run 70:BA0E
; WHAT IT DOES: Once the object is hit (OBJ_HIT_STATUS bit 7) outside state 4: the next slot becomes kind $9C, the
; six after it get OBJ_WORK_N from a table, their hit boxes off and OBJ_PACE bits 3 to 6 cleared; the object loses
; its hit box and frame and goes to state 4 (Stage 08's overlay).
; HOW: Two tests, then a loop of six over X with Y the table's index, X kept on the stack.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
BreakUpWhenHit0D5CWorkN = $BA4C ; OBJ_WORK_N for each of the six pieces
BreakUpWhenHit0D5C:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq BreakUpWhenHit0D5C_at0
    lda a:OBJ_STATE,x
    cmp #$04
    beq BreakUpWhenHit0D5C_at0
    phx
    inx
    lda #$9C
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    inx
    cly
BreakUpWhenHit0D5C_at1:
    lda a:BreakUpWhenHit0D5CWorkN,y
    sta a:OBJ_WORK_N,x
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_PACE,x
    and #$87
    sta a:OBJ_PACE,x
    inx
    iny
    cpy #$06
    bcc BreakUpWhenHit0D5C_at1
    plx
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_FRAME,x
    lda #$04
    sta a:OBJ_STATE,x
BreakUpWhenHit0D5C_at0:
    rts

; @routine 0D5C:28B TurnBackPastColumn0D5C
; @run 70:BA8B
; WHAT IT DOES: On the first screen, once the object's X reaches the column of the record in slot Y's
; OBJ_START_ARG, sets its X speed to -$20 and steps the substate on (Stage 08's overlay).
; HOW: A table read by the record's low seven bits, a compare, stores.
; WAITS: no.
; Entry: X the object; Y the slot whose record counts. Exit: A, X, Y and the flags as its last instruction or call
; left them.
TurnBackPastColumn0D5CColumns = $BAAC ; the X where the object turns, by record
TurnBackPastColumn0D5C:
    lda a:OBJ_X_HI,x
    bne TurnBackPastColumn0D5C_at0
    lda a:OBJ_START_ARG,y
    and #$7F
    tay
    lda a:OBJ_X_LO,x
    cmp a:TurnBackPastColumn0D5CColumns,y
    bcc TurnBackPastColumn0D5C_at0
    lda #$FF
    sta a:OBJ_VX_HI,x
    lda #$E0
    sta a:OBJ_VX_LO,x
    inc a:OBJ_SUBSTATE,x
TurnBackPastColumn0D5C_at0:
    rts

; @routine 0D5C:577 SetFrame60_0D5C
; @run 70:BD77
; WHAT IT DOES: Sets frame $60 and clears OBJ_WORK_C (Stage 08's overlay).
; HOW: Two stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetFrame60_0D5C:
    lda #$60
    sta a:OBJ_FRAME,x
    stz a:OBJ_WORK_C,x
    rts

; @routine 0D5D:145 WakeSlotInStartArg0D5D
; @run 71:C145
; WHAT IT DOES: Steps the slot named by OBJ_START_ARG's low seven bits from state 1 to state 2 (Stage 08's
; overlay).
; HOW: A test, a compare, INC A and a store.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
WakeSlotInStartArg0D5D:
    lda a:OBJ_START_ARG,x
    and #$7F
    beq WakeSlotInStartArg0D5D_at0
    tay
    lda a:OBJ_STATE,y
    cmp #$01
    bne WakeSlotInStartArg0D5D_at0
    inc a
    sta a:OBJ_STATE,y
WakeSlotInStartArg0D5D_at0:
    rts

; @routine 0D5D:27F OthersInState2_0D5D
; @run 71:C27F
; WHAT IT DOES: For an object in slot $33, $34 or $35, tests whether the other two of those slots are in state 2;
; returns Z set when one is (Stage 08's overlay).
; HOW: Two compares of X choose the slots; compares of their states.
; WAITS: no.
; Entry: X the slot. Exit: Z set when another of slots $33 to $35 is in state 2; A that slot's state; C from the
; compare; X and Y kept.
OthersInState2_0D5D:
    cpx #$33
    beq OthersInState2_0D5D_at0
    cpx #$34
    beq OthersInState2_0D5D_at1
    lda a:OBJ_STATE+$33
OthersInState2_0D5D_at3:
    cmp #$02
    beq OthersInState2_0D5D_at2
    lda a:OBJ_STATE+$34
    cmp #$02
OthersInState2_0D5D_at2:
    rts
OthersInState2_0D5D_at0:
    lda a:OBJ_STATE+$35
    bra OthersInState2_0D5D_at3
OthersInState2_0D5D_at1:
    lda a:OBJ_STATE+$33
    cmp #$02
    beq OthersInState2_0D5D_at2
    lda a:OBJ_STATE+$35
    cmp #$02
    rts

; @routine 0D5D:2A6 PassRecordFromState2_0D5D
; @run 71:C2A6
; WHAT IT DOES: For slot $35 with OBJ_START_ARG set: finds the first of slots $33 to $35 in state 2 and gives its
; OBJ_START_ARG to the occupied slots from $10 on, up to $1A (Stage 08's overlay).
; HOW: Three compares, then a copy loop that stops at an empty slot.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
PassRecordFromState2_0D5D:
    cpx #$35
    bne PassRecordFromState2_0D5D_at0
    lda a:OBJ_START_ARG,x
    beq PassRecordFromState2_0D5D_at0
    ldy #$33
    lda a:OBJ_STATE,y
    cmp #$02
    beq PassRecordFromState2_0D5D_at1
    iny
    lda a:OBJ_STATE,y
    cmp #$02
    beq PassRecordFromState2_0D5D_at1
    iny
    lda a:OBJ_STATE,y
    cmp #$02
    beq PassRecordFromState2_0D5D_at1
PassRecordFromState2_0D5D_at0:
    rts
PassRecordFromState2_0D5D_at1:
    lda a:OBJ_START_ARG,y
    sta z:zTemp00
    ldy #$10
PassRecordFromState2_0D5D_at3:
    lda a:OBJ_KIND,y
    beq PassRecordFromState2_0D5D_at2
    lda a:OBJ_START_ARG,x
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    iny
    cpy #$1B
    bne PassRecordFromState2_0D5D_at3
PassRecordFromState2_0D5D_at2:
    rts

; @routine 0D5D:44A HitBoxWidthByFrame0D5D
; @run 71:C44A
; WHAT IT DOES: Sets the hit box's half width to 8, or $13 for frame $2F (Stage 08's overlay).
; HOW: A compare, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
HitBoxWidthByFrame0D5D:
    lda #$08
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:OBJ_FRAME,x
    cmp #$2F
    bne HitBoxWidthByFrame0D5D_at0
    lda #$13
    sta a:OBJ_HITBOX_HALF_W,x
HitBoxWidthByFrame0D5D_at0:
    rts

; @routine 0D5D:4FD WaitThenStepPattern0D5D
; @run 71:C4FD
; WHAT IT DOES: Counts OBJ_WORK_D down, returning with C clear; once it is zero, takes the next wait from the table
; by OBJ_START_ARG and OBJ_WORK_I, restarts the animation in state 4, substate 0 and returns with C set (Stage 08's
; overlay).
; HOW: DEC, or eight times OBJ_START_ARG plus OBJ_WORK_I's low three bits indexing the waits.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: C set when the pattern stepped, clear while waiting; A and N,
; Z as the last load or DEC left them; X kept.
WaitThenStepPattern0D5DWaits = $C52E ; eight waits for each record
WaitThenStepPattern0D5D:
    lda a:OBJ_WORK_D,x
    beq WaitThenStepPattern0D5D_at0
    dec a:OBJ_WORK_D,x
    clc
    rts
WaitThenStepPattern0D5D_at0:
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_WORK_I,x
    and #$07
    clc
    adc z:zTemp00
    tay
    lda a:WaitThenStepPattern0D5DWaits,y
    sta a:OBJ_WORK_D,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    sec
    rts

; @routine 0D5E:112 LeapByNextFacing0D5E
; @run 71:C912
; WHAT IT DOES: Starts the object upward at -2 with an X speed by the facing of the next slot, and steps the state
; on (Stage 08's overlay).
; HOW: A table read by the next slot's OBJ_FACING_LEFT.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LeapByNextFacing0D5EXSpeeds = $C924 ; the X speed high byte for each facing
LeapByNextFacing0D5E:
    lda #$FE
    sta a:OBJ_VY_HI,x
    ldy a:OBJ_FACING_LEFT+$01,x
    lda a:LeapByNextFacing0D5EXSpeeds,y
    sta a:OBJ_VX_HI,x
    inc a:OBJ_STATE,x
    rts

; @routine 0D5F:255 HitBoxXByFrame0D5F
; @run 71:D255
; WHAT IT DOES: Turns the hit box on with an X offset by frame for frames up to $8F; from $90 on, the box is off
; (Stage 08's overlay).
; HOW: A compare of the frame, a table read by the frame less $8A.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HitBoxXByFrame0D5FOffsets = $D273 ; the hit box's X offset for frames $8A to $8F
HitBoxXByFrame0D5F:
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda a:OBJ_FRAME,x
    cmp #$90
    bcs HitBoxXByFrame0D5F_at0
    sec
    sbc #$8A
    tay
    lda a:HitBoxXByFrame0D5FOffsets,y
    sta a:OBJ_HITBOX_DX,x
    rts
HitBoxXByFrame0D5F_at0:
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_HITBOX_DX,x
    rts

; @routine 0D5F:4BF RepeatOrStand0D5F
; @run 71:D4BF
; WHAT IT DOES: Counts OBJ_WORK_K down: while it runs, restarts the animation in substate 1; when it ends, state 1
; with frame $8A, or $8D when OBJ_WORK_J is set (Stage 08's overlay).
; HOW: DEC and two arms of stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
RepeatOrStand0D5F:
    dec a:OBJ_WORK_K,x
    beq RepeatOrStand0D5F_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$01
    sta a:OBJ_SUBSTATE,x
    rts
RepeatOrStand0D5F_at0:
    lda #$01
    sta a:OBJ_STATE,x
    lda #$8A
    sta a:OBJ_FRAME,x
    lda a:OBJ_WORK_J,x
    beq RepeatOrStand0D5F_at1
    lda #$8D
    sta a:OBJ_FRAME,x
RepeatOrStand0D5F_at1:
    rts

; @routine 0D5F:6D1 HitBoxByFrame0D5F
; @run 71:D6D1
; WHAT IT DOES: Gives the object the hit box of its frame from $9C on (Stage 08's overlay).
; HOW: Four times the frame less $9C indexes four-byte records.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
HitBoxByFrame0D5FBoxes = $D6F3 ; X offset, Y offset, half width, half height per frame
HitBoxByFrame0D5F:
    lda a:OBJ_FRAME,x
    sec
    sbc #$9C
    asl a
    asl a
    tay
    lda a:HitBoxByFrame0D5FBoxes,y
    sta a:OBJ_HITBOX_DX,x
    lda a:HitBoxByFrame0D5FBoxes+1,y
    sta a:OBJ_HITBOX_DY,x
    lda a:HitBoxByFrame0D5FBoxes+2,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:HitBoxByFrame0D5FBoxes+3,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 0D60:27E FallFasterTo4_0D60
; @run 71:DA7E
; WHAT IT DOES: Adds $10 to the object's Y speed, up to 4 pixels a frame (Stage 08's overlay).
; HOW: A 16-bit add and a clamp.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
FallFasterTo4_0D60:
    lda a:OBJ_VY_LO,x
    clc
    adc #$10
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    cmp #$04
    bcc FallFasterTo4_0D60_at0
    lda #$00
    sta a:OBJ_VY_LO,x
    lda #$04
    sta a:OBJ_VY_HI,x
FallFasterTo4_0D60_at0:
    rts

; @routine 0D60:30E RiseByTemp02_0D60
; @run 71:DB0E
; WHAT IT DOES: Moves the object up by zTemp02 pixels and clears its Y fraction (Stage 08's overlay).
; HOW: A 16-bit subtract.
; WAITS: no.
; Entry: X the object; D as the caller left it. Exit: A, X, Y and the flags as its last instruction or call left
; them.
RiseByTemp02_0D60:
    lda a:OBJ_Y_LO,x
    sec
    sbc z:zTemp02
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_Y_HI,x
    stz a:OBJ_Y_FRAC,x
    rts

; @routine 0D6C:256 StartWhenWorkH0D6C
; @run 74:DA56
; WHAT IT DOES: Once OBJ_WORK_H is set, sets OBJ_WORK_F to $FF, OBJ_WORK_E to $0C and steps the state on (Stage
; 08's overlay).
; HOW: A test, stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
StartWhenWorkH0D6C:
    lda a:OBJ_WORK_H,x
    beq StartWhenWorkH0D6C_at0
    lda #$FF
    sta a:OBJ_WORK_F,x
    lda #$0C
    sta a:OBJ_WORK_E,x
    inc a:OBJ_STATE,x
StartWhenWorkH0D6C_at0:
    rts

; @routine 0D6C:546 SetUpSmallPiece0D6C
; @run 74:DD46
; WHAT IT DOES: Sets sprite set 6, frame $62 and a 4 by 4 hit box offset by -$0C and 8 (Stage 08's overlay).
; HOW: Stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
SetUpSmallPiece0D6C:
    lda #$06
    sta a:OBJ_SPRITE_SET,x
    lda #$62
    sta a:OBJ_FRAME,x
    lda #$04
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$04
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$F4
    sta a:OBJ_HITBOX_DX,x
    lda #$08
    sta a:OBJ_HITBOX_DY,x
    inc a:OBJ_STATE,x
    rts
