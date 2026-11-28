; @routine 088B:4F6 AimAtPlayer088B
; @run 70:B4F6
; WHAT IT DOES: Aims at the player at speed $0480, OBJ_WORK_B 36, substate on (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
AimAtPlayer088B:
    jsr VecAngleToPlayer
    pha
    lda #$80
    sta z:zTemp00
    lda #$04
    sta z:zTemp01
    pla
    jsr VecSetVelocityFromAngle
    lda #$24
    sta a:OBJ_WORK_B,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 088B:543 StartRow088B_543
; @run 70:B543
; WHAT IT DOES: Starts 3 kind $2B objects at this place, each told its number in OBJ_WORK_D (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartRow088B_543:
    cly
StartRow088B_543_at0:
    sty z:zSpreadCount
    jsr VecFindFreeSlotFrom30
    lda #$2B
    jsr VecStartObjectAtPlace
    lda z:zSpreadCount
    sta a:OBJ_WORK_D,y
    ldy z:zSpreadCount
    iny
    cpy #$03
    bcc StartRow088B_543_at0
    rts

; @routine 088B:55B StartRow088B_55B
; @run 70:B55B
; WHAT IT DOES: Starts 16 kind $2C objects at this place, each told its number in OBJ_WORK_D (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartRow088B_55B:
    cly
StartRow088B_55B_at0:
    sty z:zSpreadCount
    jsr VecFindFreeSlotFrom20
    lda #$2C
    jsr VecStartObjectAtPlace
    lda z:zSpreadCount
    sta a:OBJ_WORK_D,y
    ldy z:zSpreadCount
    iny
    cpy #$10
    bcc StartRow088B_55B_at0
    rts

; @routine 088C:017 LoadSpeedByWorkD088C
; @run 70:B817
; WHAT IT DOES: Loads the record, takes the X speed and OBJ_WORK_F the table gives for OBJ_WORK_D, and moves (secret stage 04's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadSpeedByWorkD088CSpeeds = $B82F ; X speed high byte, then OBJ_WORK_F, by OBJ_WORK_D
LoadSpeedByWorkD088C:
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_D,x
    asl a
    tay
    lda a:LoadSpeedByWorkD088CSpeeds,y
    sta a:OBJ_VX_HI,x
    lda a:LoadSpeedByWorkD088CSpeeds+1,y
    sta a:OBJ_WORK_F,x
    jsr VecApplyVelocityX
    rts

; @routine 0B4D:0CD CountThenDraw0B4D
; @run 71:C0CD
; WHAT IT DOES: Counts OBJ_WORK_D; at zero frame 5, tile script 6 plus the start argument, state on (secret stage 06's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenDraw0B4D:
    dec a:OBJ_WORK_D,x
    bne CountThenDraw0B4D_at0
    lda #$05
    sta a:OBJ_FRAME,x
    lda a:OBJ_START_ARG,x
    clc
    adc #$06
    cly
    jsr VecDrawTileScript
    inc a:OBJ_STATE,x
CountThenDraw0B4D_at0:
    rts

; @routine 0CAC:145 ListAndSink0CAC
; @run 70:B945
; WHAT IT DOES: Plays the frame list at $B8AB while sinking a dot a frame; from Y $70 the substate on (secret stage 07's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListAndSink0CAC:
    lda #$AB
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    inc a:OBJ_Y_LO,x
    lda a:OBJ_Y_LO,x
    cmp #$70
    bcc ListAndSink0CAC_at0
    inc a:OBJ_SUBSTATE,x
ListAndSink0CAC_at0:
    rts

; @routine 0CAC:15E ListThenWhenSlot24Gone0CAC
; @run 70:B95E
; WHAT IT DOES: Plays the frame list at $B8AB; once slot $18 is empty, the animation restarted and substate on (secret stage 07's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenWhenSlot24Gone0CAC:
    lda #$AB
    sta z:zTemp10
    lda #$B8
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_KIND+24
    bne ListThenWhenSlot24Gone0CAC_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
ListThenWhenSlot24Gone0CAC_at0:
    rts

; @routine 0E0B:769 WaitForSlotSpawn0E0B
; @run 70:B769
; WHAT IT DOES: Frame $43; once slot $50's state byte is 0, slot $11 cleared, OBJ_WORK_D 96, substate on (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
WaitForSlotSpawn0E0B:
    lda #$43
    sta a:OBJ_FRAME,x
    lda a:OBJ_KIND+80
    bne WaitForSlotSpawn0E0B_at0
    ldy #$11
    jsr VecClearObjectInY
    lda #$60
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
WaitForSlotSpawn0E0B_at0:
    rts

; @routine 0E0C:32D ListMoveCount0E0C_32D
; @run 70:BB2D
; WHAT IT DOES: Plays the frame list at $B972, moves with gravity and counts OBJ_WORK_E; at zero cleared (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListMoveCount0E0C_32D:
    lda #$72
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyGravity
    jsr VecApplyVelocityY
    dec a:OBJ_WORK_E,x
    bne ListMoveCount0E0C_32D_at0
    jmp VecClearObject
ListMoveCount0E0C_32D_at0:
    rts

; @routine 0E0C:478 LoadOrListThenClear0E0C
; @run 70:BC78
; WHAT IT DOES: In state 0 loads the record; later plays the frame list at $BC91 and is cleared when it ends (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadOrListThenClear0E0C:
    lda a:OBJ_STATE,x
    bne LoadOrListThenClear0E0C_at0
    jmp VecLoadObjectRecord
LoadOrListThenClear0E0C_at0:
    lda #$91
    sta z:zTemp10
    lda #$BC
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne LoadOrListThenClear0E0C_at1
    jmp VecClearObject
LoadOrListThenClear0E0C_at1:
    rts

; @routine 0E0C:4E0 ListMoveCount0E0C_4E0
; @run 70:BCE0
; WHAT IT DOES: Plays the frame list at $B972, moves with gravity and counts OBJ_WORK_E; at zero the state counted on (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListMoveCount0E0C_4E0:
    lda #$72
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityY
    jsr VecApplyGravity
    dec a:OBJ_WORK_E,x
    bne ListMoveCount0E0C_4E0_at0
    inc a:OBJ_STATE,x
ListMoveCount0E0C_4E0_at0:
    rts

; @routine 0E0C:6C1 SpawnWithSoundAtRandom0E0C
; @run 70:BEC1
; WHAT IT DOES: One frame in 16: sound $13 and a kind $9B object in a free slot (secret stage 08's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnWithSoundAtRandom0E0C:
    jsr VecNextRandom
    and #$0F
    bne SpawnWithSoundAtRandom0E0C_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnWithSoundAtRandom0E0C_at0
    lda #$13
    jsr VecSoundCommand
    lda #$9B
    jsr VecStartObjectAtPlace
SpawnWithSoundAtRandom0E0C_at0:
    rts

; @routine 0EC0:1DC FallListThenClear0EC0
; @run 71:D9DC
; WHAT IT DOES: Attribute $8B, moves along Y, plays the frame list at $D9F5 and is cleared when it ends (Stage 09's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FallListThenClear0EC0:
    lda #$8B
    sta a:OBJ_SPRITE_ATTR,x
    jsr VecApplyVelocityY
    lda #$F5
    sta z:zTemp10
    lda #$D9
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne FallListThenClear0EC0_at0
    jmp VecClearObject
FallListThenClear0EC0_at0:
    rts

; @routine 101B:73B SpeedByPlayerSide101B
; @run 70:B73B
; WHAT IT DOES: Faces the player and takes the X speed of that side and OBJ_WORK_E (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpeedByPlayerSide101BSpeeds = $B755 ; X speed words
SpeedByPlayerSide101B:
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
    eor a:OBJ_WORK_E,x
    asl a
    tay
    lda a:SpeedByPlayerSide101BSpeeds+1,y
    sta a:OBJ_VX_HI,x
    lda a:SpeedByPlayerSide101BSpeeds,y
    sta a:OBJ_VX_LO,x
    rts

; @routine 101E:36C StartRowAndClear101E
; @run 71:CB6C
; WHAT IT DOES: Starts kind $C3 objects at this place in slots $11 to $18 with its flip bits, then clears itself (Stage 0A's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
StartRowAndClear101E:
    ldy #$11
StartRowAndClear101E_at0:
    jsr VecClearObjectInY
    lda #$C3
    jsr VecStartObjectAtPlace
    lda a:OBJ_SPRITE_FLIP,x
    sta a:OBJ_SPRITE_FLIP,y
    iny
    cpy #$19
    bcc StartRowAndClear101E_at0
    jmp VecClearObject

; @routine 117D:029 LandInFront117D
; @run 71:C029
; WHAT IT DOES: Runs $BBD8 and falls with a front probe; on landing OBJ_WORK_G 12, frame $3B, substate on (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LandInFront117D:
    jsr LandOnRowThenDebris117C
    lda #$0C
    jsr VecStopOnLandingFrontColumn
    bcc LandInFront117D_at0
    lda #$0C
    sta a:OBJ_WORK_G,x
    lda #$3B
    sta a:OBJ_FRAME,x
    inc a:OBJ_SUBSTATE,x
LandInFront117D_at0:
    rts

; @routine 117D:13E ListThenSubstate117D
; @run 71:C13E
; WHAT IT DOES: Plays the frame list at $C155; when it ends, the animation restarted and substate on (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenSubstate117D:
    lda #$C1
    sta z:zTemp11
    lda #$55
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne ListThenSubstate117D_at0
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    inc a:OBJ_SUBSTATE,x
ListThenSubstate117D_at0:
    rts

; @routine 117F:05A LoadHitBoxAndSpawn117F
; @run 71:D05A
; WHAT IT DOES: Loads the record, hit-tested, OBJ_WORK_E 96, a kind $3D object in the next slot (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadHitBoxAndSpawn117F:
    jsr VecLoadObjectRecord
    lda #$02
    sta a:OBJ_HITBOX_ON,x
    lda #$60
    sta a:OBJ_WORK_E,x
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$3D
    jsr VecStartObjectAtPlace
    rts

; @routine 117F:694 SoundE2AndClearNext117F
; @run 71:D694
; WHAT IT DOES: The hit box off, sound $E2, the next slot cleared, OBJ_WORK_D 64, substate on (Stage 0B's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SoundE2AndClearNext117F:
    stz a:OBJ_HITBOX_ON,x
    lda #$E2
    jsr VecSoundCommand
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$40
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 12DC:50F FaceByBitThenNear12DC
; @run 70:BD0F
; WHAT IT DOES: Faces by A's bit 0; unless $BCDB leaves C set, an object $70 dots or more from the player goes to state 2 (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FaceByBitThenNear12DC:
    and #$01
    sta a:OBJ_FACING_LEFT,x
    jsr CountToState4_12DC
    bcs FaceByBitThenNear12DC_at0
    jsr VecDistanceXToPlayer
    cmp #$70
    bcc FaceByBitThenNear12DC_at0
    lda #$02
    sta a:OBJ_STATE,x
FaceByBitThenNear12DC_at0:
    rts

; @routine 12DC:5AF SpawnWhileEarly12DC
; @run 70:BDAF
; WHAT IT DOES: In the first five animation steps, every eighth count of OBJ_WORK_H starts a kind $2E object (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnWhileEarly12DC:
    lda a:OBJ_ANIM_STEP,x
    cmp #$05
    bcs SpawnWhileEarly12DC_at0
    lda a:OBJ_WORK_H,x
    and #$07
    bne SpawnWhileEarly12DC_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnWhileEarly12DC_at0
    lda #$2E
    jsr VecStartObjectAtPlace
SpawnWhileEarly12DC_at0:
    rts

; @routine 12DE:1A3 MoveYThenClearOffScreen12DE
; @run 71:C9A3
; WHAT IT DOES: Moves along Y; off the screen above, or below and still falling, the object is cleared (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
MoveYThenClearOffScreen12DE:
    jsr VecApplyVelocityY
    lda a:OBJ_Y_HI,x
    beq MoveYThenClearOffScreen12DE_at0
    bpl MoveYThenClearOffScreen12DE_at1
    jmp VecClearObject
MoveYThenClearOffScreen12DE_at1:
    lda a:OBJ_VY_HI,x
    bmi MoveYThenClearOffScreen12DE_at0
    jmp VecClearObject
MoveYThenClearOffScreen12DE_at0:
    rts

; @routine 12DE:648 RunThenCountClear12DE
; @run 71:CE48
; WHAT IT DOES: Runs $CE5E over patterns 4 to $11 while OBJ_WORK_G counts; at zero the object is cleared (Stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
RunThenCountClear12DE:
    lda #$04
    sta a:WorkByteA
    lda #$12
    sta a:WorkByteB
    jsr DissolveStep12DE65E
    dec a:OBJ_WORK_G,x
    bne RunThenCountClear12DE_at0
    jsr VecClearObject
RunThenCountClear12DE_at0:
    rts

; @routine 13FB:651 ListThenState213FB_651
; @run 70:B651
; WHAT IT DOES: Plays the frame list at $B667; when it ends, state 2 with substate 0 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenState213FB_651:
    lda #$67
    sta z:zTemp10
    lda #$B6
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenState213FB_651_at0
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListThenState213FB_651_at0:
    rts

; @routine 13FB:6F0 ListThenState213FB_6F0
; @run 70:B6F0
; WHAT IT DOES: Plays the frame list at $B706; when it ends, state 2 with substate 0 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenState213FB_6F0:
    lda #$06
    sta z:zTemp10
    lda #$B7
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenState213FB_6F0_at0
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListThenState213FB_6F0_at0:
    rts

; @routine 13FC:405 NextSubstateByTable13FC
; @run 70:BC05
; WHAT IT DOES: The animation restarted, the substate the table gives for OBJ_WORK_L; substate 1 sounds $26 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
NextSubstateByTable13FCNext = $BC1E ; the substate by OBJ_WORK_L
NextSubstateByTable13FC:
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    ldy a:OBJ_WORK_L,x
    lda a:NextSubstateByTable13FCNext,y
    sta a:OBJ_SUBSTATE,x
    cmp #$01
    bne NextSubstateByTable13FC_at0
    lda #$26
    jsr VecSoundCommand
NextSubstateByTable13FC_at0:
    rts

; @routine 13FC:515 ListThenState213FC_515
; @run 70:BD15
; WHAT IT DOES: Plays the frame list at $BD2B; when it ends, state 2 with substate 0 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenState213FC_515:
    lda #$2B
    sta z:zTemp10
    lda #$BD
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenState213FC_515_at0
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ListThenState213FC_515_at0:
    rts

; @routine 13FD:2D4 LoadThenStateOrDrop13FD
; @run 71:C2D4
; WHAT IT DOES: Loads the record; a start argument means state 3, else the object is put at Y $70 (secret stage 0C's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadThenStateOrDrop13FD:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    beq LoadThenStateOrDrop13FD_at0
    lda #$03
    sta a:OBJ_STATE,x
    rts
LoadThenStateOrDrop13FD_at0:
    stz a:OBJ_Y_HI,x
    lda #$70
    sta a:OBJ_Y_LO,x
    rts

; @routine 161B:303 FollowSource161B
; @run 70:B303
; WHAT IT DOES: Keeps the place of the slot OBJ_WORK_D names: cleared once it is empty, state on once it is not kind $2C (Stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
FollowSource161B:
    ldy a:OBJ_WORK_D,x
    jsr VecCopyObjectPlaceBack
    lda a:OBJ_KIND,y
    bne FollowSource161B_at0
    jmp VecClearObject
FollowSource161B_at0:
    cmp #$2C
    beq FollowSource161B_at1
    inc a:OBJ_STATE,x
FollowSource161B_at1:
    rts

; @routine 16CB:58F SpawnFromCount32_16CB
; @run 70:B58F
; WHAT IT DOES: From OBJ_WORK_D 32, on even counts of OBJ_WORK_J starts a kind $22 object (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
SpawnFromCount32_16CB:
    lda a:OBJ_WORK_D,x
    cmp #$20
    bcc SpawnFromCount32_16CB_at0
    lda a:OBJ_WORK_J,x
    and #$01
    bne SpawnFromCount32_16CB_at0
    jsr VecFindFreeSlotFrom30
    bcc SpawnFromCount32_16CB_at0
    lda #$22
    jsr VecStartObjectAtPlace
SpawnFromCount32_16CB_at0:
    rts

; @routine 16CB:66F CountThenScript97_16CB
; @run 70:B66F
; WHAT IT DOES: Counts OBJ_WORK_D; at zero script $97 starts, the count 192, substate on (secret stage 0E's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenScript97_16CB:
    dec a:OBJ_WORK_D,x
    bne CountThenScript97_16CB_at0
    phx
    lda #$97
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    lda #$C0
    sta a:OBJ_WORK_D,x
    inc a:OBJ_SUBSTATE,x
CountThenScript97_16CB_at0:
    rts

; @routine 18DB:1D2 LoadSpawnScript18DB
; @run 70:B1D2
; WHAT IT DOES: Loads the record, starts a kind $25 object in slot $11 and script $82 (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
LoadSpawnScript18DB:
    jsr VecLoadObjectRecord
    ldy #$11
    jsr VecClearObjectInY
    lda #$25
    jsr VecStartObjectAtPlace
    phx
    lda #$82
    ldx #$08
    jsr VecStartScriptSlotKind1
    plx
    rts

; @routine 18DB:327 CountThenSound91_18DB
; @run 70:B327
; WHAT IT DOES: Counts OBJ_WORK_E; at zero sound $91 and the substate the table gives for OBJ_WORK_F (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
CountThenSound91_18DBNext = $B33E ; the substate by OBJ_WORK_F's low four bits
CountThenSound91_18DB:
    dec a:OBJ_WORK_E,x
    bne CountThenSound91_18DB_at0
    lda #$91
    jsr VecSoundCommand
    lda a:OBJ_WORK_F,x
    and #$0F
    tay
    lda a:CountThenSound91_18DBNext,y
    sta a:OBJ_SUBSTATE,x
CountThenSound91_18DB_at0:
    rts

; @routine 18DB:4CA ListThenWait18DB
; @run 70:B4CA
; WHAT IT DOES: Plays the frame list at $B4E0; when it ends, OBJ_WORK_E 48 and substate on (Stage 10's overlay).
; HOW: The calls in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load left them.
ListThenWait18DB:
    lda #$E0
    sta z:zTemp10
    lda #$B4
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne ListThenWait18DB_at0
    lda #$30
    sta a:OBJ_WORK_E,x
    inc a:OBJ_SUBSTATE,x
ListThenWait18DB_at0:
    rts
