; @routine 09EB:3D6 SoundAtCount09EB
; @run 70:B3D6
; WHAT IT DOES: Sound $A3 each time OBJ_WORK_I reaches OBJ_WORK_L, which then moves on by $40 (secret stage 05's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundAtCount09EB:
    lda a:OBJ_WORK_L,x
    beq SoundAtCount09EB_at0
    lda a:OBJ_WORK_I,x
    cmp a:OBJ_WORK_L,x
    bcc SoundAtCount09EB_at0
    lda #$A3
    jsr VecSoundCommand
    lda a:OBJ_WORK_L,x
    clc
    adc #$40
    sta a:OBJ_WORK_L,x
SoundAtCount09EB_at0:
    rts

; @routine 09EB:449 SoundA0AndSink09EB
; @run 70:B449
; WHAT IT DOES: Sound $A0, the hit box off, sinking half a dot, raster effect $0B from line $C0, substate on (secret stage 05's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundA0AndSink09EB:
    lda #$A0
    jsr VecSoundCommand
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda #$0B
    sta z:zRasterEffect
    lda #$C0
    sta z:zRasterArg
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 09EC:25F FallThenClearOffScreen09EC
; @run 70:BA5F
; WHAT IT DOES: Gravity of $18, a move, cleared once off the first screen along Y (secret stage 05's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallThenClearOffScreen09EC:
    lda a:OBJ_VY_LO,x
    clc
    adc #$18
    sta a:OBJ_VY_LO,x
    lda a:OBJ_VY_HI,x
    adc #$00
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI,x
    beq FallThenClearOffScreen09EC_at0
    jmp VecClearObject
FallThenClearOffScreen09EC_at0:
    rts

; @routine 0B4C:2E1 ListThenState5When0B4C
; @run 70:BAE1
; WHAT IT DOES: Plays the list at $BAFD; when $B77C leaves C set, the animation restarted and state 5 (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenState5When0B4C:
    lda #$BA
    sta z:zTemp11
    lda #$FD
    sta z:zTemp10
    jsr VecStepObjectFrames
    jsr FallToGroundAt98
    bcc ListThenState5When0B4C_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
ListThenState5When0B4C_at0:
    rts

; @routine 0CAD:00D ListByWorkDThenSlot16_0CAD
; @run 71:C00D
; WHAT IT DOES: Plays the list for OBJ_WORK_D; once slot $10 is in substate 4 the state counts on (secret stage 07's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListByWorkDThenSlot16_0CADLists = $C02A ; a frame list per OBJ_WORK_D
ListByWorkDThenSlot16_0CAD:
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:ListByWorkDThenSlot16_0CADLists,y
    sta z:zTemp10
    lda a:ListByWorkDThenSlot16_0CADLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_SUBSTATE+16
    cmp #$04
    bne ListByWorkDThenSlot16_0CAD_at0
    inc a:OBJ_STATE,x
ListByWorkDThenSlot16_0CAD_at0:
    rts

; @routine 0E0B:718 ListChannelThenSlot80_0E0B
; @run 70:B718
; WHAT IT DOES: List channel $84; substate 3 while slot $50 is taken, else sound $CF and substate on (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListChannelThenSlot80_0E0B:
    phx
    lda #$84
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    lda a:OBJ_KIND+80
    beq ListChannelThenSlot80_0E0B_at0
    lda #$03
    sta a:OBJ_SUBSTATE,x
    rts
ListChannelThenSlot80_0E0B_at0:
    lda #$CF
    jsr VecSoundCommand
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 0E0C:2E0 ListThenHover0E0C
; @run 70:BAE0
; WHAT IT DOES: Plays the list at $BAFC; when it ends, OBJ_WORK_E 23, no Y speed, state on (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenHover0E0C:
    lda #$FC
    sta z:zTemp10
    lda #$BA
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenHover0E0C_at0
    lda #$17
    sta a:OBJ_WORK_E,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    inc a:OBJ_STATE,x
ListThenHover0E0C_at0:
    rts

; @routine 0E0C:4C5 LoadAndPop0E0C
; @run 70:BCC5
; WHAT IT DOES: Loads the record, moves up 40 dots at once, then rises at -6 with OBJ_WORK_E 40 (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAndPop0E0C:
    jsr VecLoadObjectRecord
    lda #$D8
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    lda #$28
    sta a:OBJ_WORK_E,x
    lda #$FA
    sta a:OBJ_VY_HI,x
    lda #$00
    sta a:OBJ_VY_LO,x
    rts

; @routine 0EBD:191 LoadSetBoxAndList0EBD
; @run 71:C191
; WHAT IT DOES: Loads the record, sprite set 4, a narrow hit box, and plays the list at $C223 (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadSetBoxAndList0EBD:
    jsr VecLoadObjectRecord
    lda #$04
    sta a:OBJ_SPRITE_SET,x
    lda #$04
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$0F
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$23
    sta z:zTemp10
    lda #$C2
    sta z:zTemp11
    jmp VecStepObjectFrames

; @routine 0EBD:619 ListCountThenNext0EBD
; @run 71:C619
; WHAT IT DOES: Plays the list at $C56E while OBJ_WORK_D counts; at zero this state, and with a start argument the next slot's, count on (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListCountThenNext0EBD:
    lda #$6E
    sta z:zTemp10
    lda #$C5
    sta z:zTemp11
    jsr VecStepObjectFrames
    dec a:OBJ_WORK_D,x
    bne ListCountThenNext0EBD_at0
    inc a:OBJ_STATE,x
    lda a:OBJ_START_ARG,x
    beq ListCountThenNext0EBD_at0
    inc a:OBJ_STATE+1,x
ListCountThenNext0EBD_at0:
    rts

; @routine 0EBE:15E FrameByRiseThenLand0EBE
; @run 71:C95E
; WHAT IT DOES: A rising or falling frame; on landing state 3 (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FrameByRiseThenLand0EBEFrames = $C97C ; rising, falling
FrameByRiseThenLand0EBE:
    lda a:OBJ_VY_HI,x
    rol a
    rol a
    and #$01
    tay
    lda a:FrameByRiseThenLand0EBEFrames,y
    sta a:OBJ_FRAME,x
    lda #$06
    jsr VecStopOnLanding
    bcc FrameByRiseThenLand0EBE_at0
    lda #$03
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
FrameByRiseThenLand0EBE_at0:
    rts

; @routine 0EC0:386 ListThenBurstOnRow0EC0
; @run 71:DB86
; WHAT IT DOES: Plays the list at $DB5D, runs $D7B5 on the $Dx row, falls and moves (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenBurstOnRow0EC0:
    lda #$DB
    sta z:zTemp11
    lda #$5D
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_Y_LO,x
    and #$F0
    cmp #$D0
    bne ListThenBurstOnRow0EC0_at0
    jsr Sound23ThenDebris0EBF
ListThenBurstOnRow0EC0_at0:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity

; @routine 101B:4D4 LoadAndWait101B
; @run 70:B4D4
; WHAT IT DOES: Loads the record, facing left, no hit box, bit 6 of OBJ_LOOP_FLAGS off, state 4 (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAndWait101B:
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_HITBOX_ON,x
    lda a:OBJ_LOOP_FLAGS,x
    and #$BF
    sta a:OBJ_LOOP_FLAGS,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    rts

; @routine 101D:6D6 BurstThenClearPair101D
; @run 71:C6D6
; WHAT IT DOES: Counts OBJ_WORK_G; then kind $BF objects in slots $20 to $31 and this object and the next cleared (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BurstThenClearPair101D:
    dec a:OBJ_WORK_G,x
    bne BurstThenClearPair101D_at0
    ldy #$20
BurstThenClearPair101D_at1:
    jsr VecClearObjectInY
    lda #$BF
    jsr VecStartObjectAtPlace
    iny
    cpy #$32
    bcc BurstThenClearPair101D_at1
    jsr VecClearObject
    inx
    jsr VecClearObject
    dex
BurstThenClearPair101D_at0:
    rts

; @routine 101E:6B4 LoadAtRandomTop101E
; @run 71:CEB4
; WHAT IT DOES: Loads the record and puts the object above the screen at a random X (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAtRandomTop101E:
    jsr VecLoadObjectRecord
    stz a:OBJ_X_HI,x
    jsr VecNextRandom
    and #$7F
    clc
    adc #$C0
    sta a:OBJ_X_LO,x
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$F0
    sta a:OBJ_Y_LO,x
    rts

; @routine 101F:721 StartInNextThree101F
; @run 71:D721
; WHAT IT DOES: Starts a kind $B7 object in the first empty of the next three slots; zTemp00 counted (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartInNextThree101F:
    txa
    tay
    iny
    lda #$03
    sta z:zTemp01
StartInNextThree101F_at1:
    lda a:OBJ_KIND,y
    beq StartInNextThree101F_at0
    iny
    dec z:zTemp01
    bne StartInNextThree101F_at1
    rts
StartInNextThree101F_at0:
    jsr VecClearObjectInY
    lda #$B7
    jsr VecStartObjectAtPlace
    inc z:zTemp00
    rts

; @routine 1020:047 StartCopyOfWorks1020_047
; @run 71:D847
; WHAT IT DOES: Starts a kind $B3 object in a free slot with this object's work_f and work_e; zTemp00 counted.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartCopyOfWorks1020_047:
    jsr VecFindFreeSlotFrom30
    bcc StartCopyOfWorks1020_047_at0
    jsr VecClearObjectInY
    lda #$B3
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_F,x
    sta a:OBJ_WORK_G,y
    lda a:OBJ_WORK_E,x
    sta a:OBJ_WORK_H,y
    inc z:zTemp00
StartCopyOfWorks1020_047_at0:
    rts

; @routine 10CC:202 LandBelow90OrFall10CC
; @run 70:BA02
; WHAT IT DOES: Low on the first screen lands with height 4; else falls and moves, C clear (secret stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandBelow90OrFall10CC:
    lda a:OBJ_Y_HI,x
    bne LandBelow90OrFall10CC_at0
    lda a:OBJ_Y_LO,x
    cmp #$90
    bcc LandBelow90OrFall10CC_at0
    lda #$04
    jsr VecStopOnLanding
    rts
LandBelow90OrFall10CC_at0:
    jsr VecApplyGravity
    jsr VecApplyVelocity
    clc
    rts

; @routine 10CD:3D9 StartCopyOfWorks10CD_3D9
; @run 71:C3D9
; WHAT IT DOES: Starts a kind $A4 object in a free slot with this object's work_f and start_arg; zTemp00 counted.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartCopyOfWorks10CD_3D9:
    jsr VecFindFreeSlotFrom30
    bcc StartCopyOfWorks10CD_3D9_at0
    jsr VecClearObjectInY
    lda #$A4
    jsr VecStartObjectAtPlace
    lda a:OBJ_WORK_F,x
    sta a:OBJ_WORK_G,y
    lda a:OBJ_START_ARG,x
    sta a:OBJ_FACING_LEFT,y
    inc z:zTemp00
StartCopyOfWorks10CD_3D9_at0:
    rts

; @routine 117B:34A ListByWorkEThenState1_117B
; @run 70:B34A
; WHAT IT DOES: Plays the list the table gives for OBJ_WORK_E; when it ends, state 1 (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListByWorkEThenState1_117BLists = $B364 ; a frame list per OBJ_WORK_E
ListByWorkEThenState1_117B:
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:ListByWorkEThenState1_117BLists,y
    sta z:zTemp10
    lda a:ListByWorkEThenState1_117BLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListByWorkEThenState1_117B_at0
    lda #$01
    sta a:OBJ_STATE,x
ListByWorkEThenState1_117B_at0:
    rts

; @routine 117E:06C SpeedByWorkEAndMove117E
; @run 71:C86C
; WHAT IT DOES: The speeds the table gives for OBJ_WORK_E, X reversed when facing left, then a move (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpeedByWorkEAndMove117ESpeeds = $C888 ; X and Y speed high bytes by OBJ_WORK_E
SpeedByWorkEAndMove117E:
    lda a:OBJ_WORK_E,x
    asl a
    tay
    lda a:SpeedByWorkEAndMove117ESpeeds,y
    sta a:OBJ_VX_HI,x
    lda a:SpeedByWorkEAndMove117ESpeeds+1,y
    sta a:OBJ_VY_HI,x
    lda a:OBJ_FACING_LEFT,x
    beq SpeedByWorkEAndMove117E_at0
    jsr VecNegateVelocityX
SpeedByWorkEAndMove117E_at0:
    jmp VecApplyVelocity

; @routine 117F:459 ListThenProbe117F
; @run 71:D459
; WHAT IT DOES: Plays the list at $D479; on a new frame, the X speed by OBJ_WORK_F, running on into ProbeHeightCall117F_46F (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenProbe117FSpeeds = $D477 ; the X speed high byte by OBJ_WORK_F
ListThenProbe117F:
    lda #$79
    sta z:zTemp10
    lda #$D4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc ProbeHeightCall117F_46F+7
    ldy a:OBJ_WORK_F,x
    lda a:ListThenProbe117FSpeeds,y
    sta a:OBJ_VX_HI,x

; @routine 117F:4DA NextOfCycle117F_4DA
; @run 71:D4DA
; WHAT IT DOES: The next substate of a cycle of eight (OBJ_WORK_H), the animation restarted.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
NextOfCycle117F_4DACycle = $D4F8 ; the substates in turn
NextOfCycle117F_4DA:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    jsr VecNextRandom
    and #$03
    sta a:OBJ_WORK_K,x
    lda a:OBJ_WORK_H,x
    and #$07
    tay
    lda a:NextOfCycle117F_4DACycle,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_H,x
    rts

; @routine 12DA:7BC SoundScriptEvery180_12DA
; @run 70:AFBC
; WHAT IT DOES: Counts OBJ_WORK_D; at zero sound $47, script $87, the count 180 and the state on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundScriptEvery180_12DA:
    dec a:OBJ_WORK_D,x
    bne SoundScriptEvery180_12DA_at0
    lda #$47
    jsr VecSoundCommand
    phx
    lda #$87
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    lda #$B4
    sta a:OBJ_WORK_D,x
    inc a:OBJ_STATE,x
SoundScriptEvery180_12DA_at0:
    rts

; @routine 12DB:746 ListThenBox12DB
; @run 70:B746
; WHAT IT DOES: Plays the list at $B763; when it ends, a wide low hit box and state 2 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenBox12DB:
    lda #$63
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenBox12DB_at0
    lda #$1C
    sta a:OBJ_HITBOX_HALF_W,x
    lda #$10
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$02
    sta a:OBJ_STATE,x
ListThenBox12DB_at0:
    rts

; @routine 12DD:552 NextOfCycle12DD_552
; @run 71:C552
; WHAT IT DOES: The next substate of a cycle of eight (OBJ_WORK_H), the animation restarted.
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
NextOfCycle12DD_552Cycle = $C570 ; the substates in turn
NextOfCycle12DD_552:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda a:OBJ_WORK_H,x
    and #$07
    tay
    lda a:NextOfCycle12DD_552Cycle,y
    sta a:OBJ_SUBSTATE,x
    inc a:OBJ_WORK_H,x
    rts

; @routine 12DE:62A DissolveThenWait12DE
; @run 71:CE2A
; WHAT IT DOES: Dissolves patterns 0 to 3 while OBJ_WORK_G counts; then the count 128, substate on (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DissolveThenWait12DE:
    lda #$00
    sta a:WorkByteA
    lda #$04
    sta a:WorkByteB
    jsr DissolveStep12DE65E
    dec a:OBJ_WORK_G,x
    bne DissolveThenWait12DE_at0
    lda #$80
    sta a:OBJ_WORK_G,x
    stz a:OBJ_WORK_L,x
    inc a:OBJ_SUBSTATE,x
DissolveThenWait12DE_at0:
    rts

; @routine 13FB:384 LoadAboveAndScript13FB
; @run 70:B384
; WHAT IT DOES: Loads the record above the screen, OBJ_WORK_G $80, script $90 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAboveAndScript13FB:
    jsr VecLoadObjectRecord
    lda #$FF
    sta a:OBJ_Y_HI,x
    lda #$C0
    sta a:OBJ_Y_LO,x
    lda #$80
    sta a:OBJ_WORK_G,x
    phx
    lda #$90
    ldx #$04
    jsr VecStartScriptSlotKind1
    plx
    rts

; @routine 13FB:48C SpawnAtStep18_13FB
; @run 70:B48C
; WHAT IT DOES: At animation step $12 runs on into SpawnOddFramesAnswer13FB, else leaves C clear (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnAtStep18_13FB:
    lda a:OBJ_ANIM_STEP,x
    cmp #$12
    bne SpawnOddFramesAnswer13FB+18

; @routine 13FC:4FA KnockAwayWhenHit13FC
; @run 70:BCFA
; WHAT IT DOES: A hit of 5 or more: OBJ_WORK_D the angle away from the player, OBJ_WORK_E 2, OBJ_WORK_F $80 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
KnockAwayWhenHit13FC:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc KnockAwayWhenHit13FC_at0
    jsr VecAngleToPlayer
    clc
    adc #$80
    sta a:OBJ_WORK_D,x
    lda #$02
    sta a:OBJ_WORK_E,x
    lda #$80
    sta a:OBJ_WORK_F,x
KnockAwayWhenHit13FC_at0:
    rts

; @routine 13FD:144 LoadAndToss13FD
; @run 71:C144
; WHAT IT DOES: Loads the record, jumps 12 dots right and rises at -1.5 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadAndToss13FD:
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_START_ARG,x
    lda #$0C
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    rts

; @routine 13FD:3D5 SwingAndCountWord13FD
; @run 71:C3D5
; WHAT IT DOES: Swings (the routine at $C3F2) and counts the word OBJ_WORK_D-E down; at zero the state on (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SwingAndCountWord13FD:
    jsr SwingStep13FD
    lda a:OBJ_WORK_E,x
    sec
    sbc #$01
    sta a:OBJ_WORK_E,x
    lda a:OBJ_WORK_D,x
    sbc #$00
    sta a:OBJ_WORK_D,x
    ora a:OBJ_WORK_E,x
    bne SwingAndCountWord13FD_at0
    inc a:OBJ_STATE,x
SwingAndCountWord13FD_at0:
    rts

; @routine 161B:71E BreakIntoFive161B
; @run 70:B71E
; WHAT IT DOES: Starts up to five kind $34 objects numbered 5 down at this place, then clears itself (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
BreakIntoFive161B:
    lda #$05
    sta z:zTemp00
BreakIntoFive161B_at1:
    jsr VecFindFreeSlotFrom30
    bcc BreakIntoFive161B_at0
    lda #$34
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    dec z:zTemp00
    bne BreakIntoFive161B_at1
BreakIntoFive161B_at0:
    jmp VecClearObject

; @routine 16CB:686 DissolveThenBecome29_16CB
; @run 70:B686
; WHAT IT DOES: Dissolves patterns 0 to 9 while OBJ_WORK_D counts; then the slot becomes kind $29 (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
DissolveThenBecome29_16CB:
    lda #$00
    sta a:WorkByteA
    lda #$0A
    sta a:WorkByteB
    jsr DissolveStep16CB6A4
    dec a:OBJ_WORK_D,x
    bne DissolveThenBecome29_16CB_at0
    jsr VecClearObject
    lda #$29
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
DissolveThenBecome29_16CB_at0:
    rts

; @routine 18DB:4AF MoveThenStartFour18DB
; @run 70:B4AF
; WHAT IT DOES: Moves while OBJ_WORK_E counts; then starts kind $2F objects in slots $11 to $14 (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
MoveThenStartFour18DB:
    jsr VecApplyVelocityX
    dec a:OBJ_WORK_E,x
    bne MoveThenStartFour18DB_at0
    ldy #$11
MoveThenStartFour18DB_at1:
    jsr VecClearObjectInY
    lda #$2F
    jsr VecStartObjectAtPlace
    iny
    cpy #$15
    bcc MoveThenStartFour18DB_at1
    inc a:OBJ_SUBSTATE,x
MoveThenStartFour18DB_at0:
    rts

; @routine 18DC:172 StartKind2DAt48_18DC
; @run 70:B972
; WHAT IT DOES: Once slot $33 is empty, a kind $2D object in slot $30, OBJ_WORK_E 56, substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartKind2DAt48_18DC:
    lda a:OBJ_KIND+51
    bne StartKind2DAt48_18DC_at0
    ldy #$30
    jsr VecClearObjectInY
    lda #$2D
    jsr VecStartObjectAtPlace
    lda a:OBJ_FACING_LEFT,x
    sta a:OBJ_FACING_LEFT,y
    lda #$38
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
StartKind2DAt48_18DC_at0:
    rts
