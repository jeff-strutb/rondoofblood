; @routine 01DD:7EB MariaStateWalk
; @run 6D:A7EB
; WHAT IT DOES: State 1, walking: the columns under her and a floor (one row lower if not), else she falls; the jump,
; a landing, the item crash, the attack, a ledge, MariaUpHeld's crouch; then her animation ticks and, while the way she
; faces is held, she keeps the walking speed; let go, she stands.
; HOW: MariaReadColumns and MariaFloorCheck (twice, with INC of her Y between), MariaJumpPressed, MariaLandOnFloor,
; MariaTryItemCrash, MariaUseItem, MariaLedgeCheck, MariaUpHeld, each carry set ending in its JMP; MariaAnimationTick;
; BBR5 or BBR7 of zPadHeld by OBJ_FACING_LEFT; the tail JMPs to MariaSpeedWalk and MariaStand.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in.
MariaStateWalk:
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcs MariaStateWalk_at0
    inc a:OBJ_Y_LO
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcs MariaStateWalk_at0
    jmp MariaToFall
MariaStateWalk_at0:
    jsr MariaJumpPressed
    bcc MariaStateWalk_at1
    jmp MariaJump
MariaStateWalk_at1:
    jsr MariaLandOnFloor
    jsr MariaTryItemCrash
    bcc MariaStateWalk_at2
    jmp MariaCrashOnGround
MariaStateWalk_at2:
    jsr MariaUseItem
    bcc MariaStateWalk_at3
    jmp MariaAttack
MariaStateWalk_at3:
    jsr MariaLedgeCheck
    bcc MariaStateWalk_at4
    jmp MariaClimbStart
MariaStateWalk_at4:
    jsr MariaUpHeld
    bcc MariaStateWalk_at5
    jmp MariaCrouch
MariaStateWalk_at5:
    jsr MariaAnimationTick
    lda a:OBJ_FACING_LEFT
    bne MariaStateWalk_at6
    bbr5 zPadHeld,MariaStand
    jmp MariaSpeedWalk
MariaStateWalk_at6:
    bbr7 zPadHeld,MariaStand
    jmp MariaSpeedWalk

; @routine 01DE:09C MariaStateCrouch
; @run 6D:A89C
; WHAT IT DOES: State 2, crouching: the step's handler from MariaCrouchSteps (crouched, the attack, nothing, going
; down, standing up).
; HOW: MariaStepIndex, then the JMP (table,X).
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
MariaStateCrouch:
    jsr MariaStepIndex
    jmp (MariaCrouchSteps,x)

; @routine 01DE:0AC MariaCrouchIdle
; @run 6D:A8AC
; WHAT IT DOES: Step 2 of the crouch: nothing.
; HOW: RTS.
; WAITS: no.
; Entry, exit: nothing; all registers and flags kept.
MariaCrouchIdle:
    rts

; @routine 01DE:0AD MariaCrouchFrame
; @run 6D:A8AD
; WHAT IT DOES: Step 0 of the crouch: the crouch's checks; with neither up nor a mark above her (MariaUpOrMarkedAbove
; clear), she starts to stand; a slide, she slides; else she faces the way the pad was pressed.
; HOW: MariaCrouchChecks, MariaUpOrMarkedAbove's carry, MariaSlideCheck's carry and the JMP to MariaSlideStart; zPadPressed
; on into MariaFaceByPad.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in, or MariaFaceByPad with zPadPressed.
MariaCrouchFrame:
    jsr MariaCrouchChecks
    jsr MariaUpOrMarkedAbove
    bcc MariaCrouchRise
    jsr MariaSlideCheck
    bcc MariaCrouchFrame_at0
    jmp MariaSlideStart
MariaCrouchFrame_at0:
    lda z:zPadPressed

; @routine 01DE:0BF MariaFaceByPad
; @run 6D:A8BF
; WHAT IT DOES: Faces her the way a pad byte holds right or left (bit 5 right, bit 7 left); neither or both, no change.
; HOW: AND #$A0 and two compares; STZ then INC of OBJ_FACING_LEFT.
; WAITS: no.
; Entry: A the pad byte. Exit: A its bits 5 and 7; N and Z from the last test of them, or from the facing's INC when
; left; C from the last compare (kept when neither is held). X, Y and V kept.
MariaFaceByPad:
    and #$A0
    beq MariaFaceByPad_at0
    cmp #$A0
    beq MariaFaceByPad_at0
    stz a:OBJ_FACING_LEFT
    cmp #$20
    beq MariaFaceByPad_at0
    inc a:OBJ_FACING_LEFT
MariaFaceByPad_at0:
    rts

; @routine 01DE:0D2 MariaCrouchRise
; @run 6D:A8D2
; WHAT IT DOES: Starts her standing up from the crouch: animation $0F, step 4.
; HOW: MariaSetAnimation, then the store.
; WAITS: no.
; Entry: nothing. Exit: A 4, N and Z from it; Y and C as MariaSetAnimation left them. X and V kept.
MariaCrouchRise:
    lda #$0F
    jsr MariaSetAnimation
    lda #$04
    sta a:OBJ_SUBSTATE
    rts

; @routine 01DE:0DD MariaCrouchChecks
; @run 6D:A8DD
; WHAT IT DOES: The checks a crouch's step makes first: the columns under her; no footing, she falls; a landing; then
; the item crash and the attack button. When one fires, the step ends there: its return address is pulled and the
; fall, the item crash or the crouching attack runs in its place.
; HOW: MariaReadColumns, MariaHasNoFooting, MariaLandOnFloor, MariaTryItemCrash, MariaUseItem; PLA, PLA and the JMP to
; MariaToFall, MariaCrashOnGround or MariaCrouchAttack.
; WAITS: no.
; Entry: X and Y the caller's. Exit: nothing fired, as MariaUseItem with C clear. Fired, the caller's return address
; gone and S two higher; as the routine jumped to, after A the return address's high byte.
MariaCrouchChecks:
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcs MariaCrouchChecks_at0
    jsr MariaLandOnFloor
    jsr MariaTryItemCrash
    bcs MariaCrouchChecks_at1
MariaCrouchChecks_at3:
    jsr MariaUseItem
    bcs MariaCrouchChecks_at2
    rts
MariaCrouchChecks_at0:
    pla
    pla
    jmp MariaToFall
MariaCrouchChecks_at1:
    pla
    pla
    jmp MariaCrashOnGround
MariaCrouchChecks_at2:
    pla
    pla
    jmp MariaCrouchAttack

; @routine 01DE:102 MariaCrouchAttack
; @run 6D:A902
; WHAT IT DOES: Her attack from the crouch: what the attack button said (MariaAttack); if it started her attack's
; object, step 1 and animation $21.
; HOW: MariaAttack's Z; the tail JMP to MariaSetAnimation.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaAttack when nothing started; else as MariaSetAnimation.
MariaCrouchAttack:
    jsr MariaAttack
    beq MariaCrouchAttack_at0
    rts
MariaCrouchAttack_at0:
    lda #$01
    sta a:OBJ_SUBSTATE
    lda #$21
    jmp MariaSetAnimation

; @routine 01DE:112 MariaCrouchAttackFrame
; @run 6D:A912
; WHAT IT DOES: Step 1 of the crouch, the attack: while her animation runs, as step 0; when it has stopped, the attack
; button's outcome cleared and back to step 0.
; HOW: MariaAnimationRunning's Z; the JMP to MariaCrouchFrame.
; WAITS: no.
; Entry: X and Y the caller's. Exit: running, as MariaCrouchFrame; else A 0, Z set, the rest as MariaAnimationRunning
; left it.
MariaCrouchAttackFrame:
    jsr MariaAnimationRunning
    beq MariaCrouchAttackFrame_at0
    jmp MariaCrouchFrame
MariaCrouchAttackFrame_at0:
    stz a:PlayerAttackKind
    lda #$00
    sta a:OBJ_SUBSTATE
    rts

; @routine 01DE:123 MariaCrouchStart
; @run 6D:A923
; WHAT IT DOES: Step 3 of the crouch, going down: the pad's forced presses off, the crouch's checks, a slide; while her
; animation runs, nothing more; when it has stopped, step 0.
; HOW: MariaPadForcesOff, MariaCrouchChecks, MariaSlideCheck and the JMP to MariaSlideStart; MariaAnimationWait, which
; ends the step while the animation runs.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in; at the animation's end, A 0, Z set.
MariaCrouchStart:
    jsr MariaPadForcesOff
    jsr MariaCrouchChecks
    jsr MariaSlideCheck
    bcc MariaCrouchStart_at0
    jmp MariaSlideStart
MariaCrouchStart_at0:
    jsr MariaAnimationWait
    lda #$00
    sta a:OBJ_SUBSTATE
    rts

; @routine 01DE:13A MariaStandUp
; @run 6D:A93A
; WHAT IT DOES: Step 4 of the crouch, standing up: the stand's ground checks; while her animation runs, nothing more;
; when it has stopped, she stands.
; HOW: MariaGroundChecks, MariaAnimationWait, then the tail JMP to MariaStand.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the routine it ended in, or MariaStand.
MariaStandUp:
    jsr MariaGroundChecks
    jsr MariaAnimationWait
    jmp MariaStand

; @routine 01DE:7DF MariaReplaceSlotsOneTwoKind5
; @run 6D:AFDF
; WHAT IT DOES: As MariaReplaceSlotsOneTwo, with kind 5: slots 1 and 2 cleared as kind 5, each unless it holds kind 1
; or 2.
; HOW: Y 5, then a branch into MariaReplaceSlotsOneTwo past its LDY: OBJ_KIND compared with 1 and 2; a BSR to
; MariaClearSlotAsKind for slot 1, a run on into it for slot 2.
; WAITS: no.
; Entry: nothing. Exit: Y 5, X 2; slot 2 kept, A its kind, N, Z and C from comparing it with 1 or 2; else A 5, N and
; Z from it, C from comparing the kind with 2. V kept.
MariaReplaceSlotsOneTwoKind5:
    ldy #$05
    bra MariaReplaceSlotsOneTwo+2

; @routine 01DE:7E3 MariaCheckBit3Under
; @run 6D:AFE3
; WHAT IT DOES: Starts MariaBit3Burst when any of the three columns under her read bit 3.
; HOW: RichterColumnHit's first three ORed; a branch into MariaBit3Burst.
; WAITS: no.
; Entry: X and Y the caller's. Exit: none, A 0, Z set; else as MariaBit3Burst.
MariaCheckBit3Under:
    lda a:RichterColumnHit
    ora a:RichterColumnHit+1
    ora a:RichterColumnHit+2
    and #$08
    bne MariaBit3Burst
    rts

; @routine 01DE:7F1 MariaBit3Burst
; @run 6D:AFF1
; WHAT IT DOES: Sound $23; each of slots 2 and 1 that holds kind 1 is emptied and starts the hit's script 8; then six
; objects of type $9A start where she is, 16 rows below her (MariaStart9AObjects with 0), her Y put back after.
; HOW: SoundCommand; OBJ_KIND less one; ClearObject, MariaStartHitScript; her Y bytes on the stack around the 16-bit add.
; The slots are counted down in X, which MariaStartHitScript leaves 7: after a hit the count goes on from slot 6.
; WAITS: no.
; Entry: X and Y the caller's (SoundCommand is given them). Exit: A her Y's low byte, N and Z from it, X 0; Y and C as
; MariaStart9AObjects left them; V from the add to her Y's high byte.
MariaBit3Burst:
    lda #$23
    jsr VecSoundCommand
    ldx #$02
MariaBit3Burst_at1:
    lda a:OBJ_KIND,x
    dec a
    bne MariaBit3Burst_at0
    jsr VecClearObject
    lda #$08
    jsr MariaStartHitScript
MariaBit3Burst_at0:
    dex
    bne MariaBit3Burst_at1
    lda a:OBJ_Y_LO
    pha
    clc
    adc #$10
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    pha
    adc #$00
    sta a:OBJ_Y_HI
    cla
    bsr MariaStart9AObjects
    pla
    sta a:OBJ_Y_HI
    pla
    sta a:OBJ_Y_LO
    rts

; @routine 01DF:028 MariaStart9AObjects
; @run 6D:B028
; WHAT IT DOES: Starts six objects of type $9A where object X is, in free slots from $30: each with OBJ_WORK_D the
; value given and OBJ_WORK_E its number, 0 to 5. With no free slot, stops there.
; HOW: zTemp00 the value, zBurstCount the count; FindFreeSlotFrom30, StartObjectAtPlace (both through the kernel's
; vectors). Y is kept on the stack.
; WAITS: no.
; Entry: A the value, X the object they start at, Y kept. Exit: Y as given, N and Z from it. Six started, A 5, C set;
; else A the last type FindFreeSlotFrom30 read, C clear. X and V kept.
MariaStart9AObjects:
    sta z:zTemp00
    phy
    cly
MariaStart9AObjects_at1:
    sty z:zBurstCount
    jsr VecFindFreeSlotFrom30
    bcc MariaStart9AObjects_at0
    lda z:zTemp00
    sta a:OBJ_WORK_D,y
    lda #$9A
    jsr VecStartObjectAtPlace
    lda z:zBurstCount
    sta a:OBJ_WORK_E,y
    ldy z:zBurstCount
    iny
    cpy #$06
    bcc MariaStart9AObjects_at1
MariaStart9AObjects_at0:
    ply
    rts

; @routine 01DF:04B MariaSlideCheck
; @run 6D:B04B
; WHAT IT DOES: Whether she slides from the crouch: she faces the way the pad holds (MariaFaceByPad); with the jump
; pressed, short of the screen's edge that way ($10 left, $F0 right) and with no bit 1 ahead of her (7 across), C
; set. Else C clear and her facing put back.
; HOW: The facing on the stack; MariaFaceByPad with zPadHeld, MariaJumpPressed, the edge, MariaSlideAhead by the
; facing and a BSR to MariaReadAhead.
; WAITS: no.
; Entry: X and Y the caller's. Exit: A the facing she had, N and Z from it; C set to slide; Y as the last call left
; it (the facing when the read was made after it), V likewise. X kept.
MariaSlideCheck:
    lda a:OBJ_FACING_LEFT
    pha
    lda z:zPadHeld
    jsr MariaFaceByPad
    jsr MariaJumpPressed
    bcc LoadRecordSplitIntoFourAndClear
    lda a:OBJ_FACING_LEFT
    beq MariaSlideCheck_at0
    lda a:OBJ_X_LO
    cmp #$10
    bcc LoadRecordSplitIntoFourAndClear
    beq LoadRecordSplitIntoFourAndClear
    bra MariaSlideCheck_at1
MariaSlideCheck_at0:
    lda a:OBJ_X_LO
    cmp #$F0
    bcs LoadRecordSplitIntoFourAndClear
MariaSlideCheck_at1:
    ldy a:OBJ_FACING_LEFT
    lda a:MariaSlideAhead,y
    bsr MariaReadAhead
    bne LoadRecordSplitIntoFourAndClear
    pla
    sec
    rts
    pla
    sta a:OBJ_FACING_LEFT
    clc
    rts

; @routine 01DF:083 MariaReadAhead
; @run 6D:B083
; WHAT IT DOES: Reads the terrain a given distance across from her and 16 rows below, for bit 1.
; HOW: The distance plus her X to zTemp00, her Y plus $10 and its carry, MariaReadBelow; AND #2.
; WAITS: no.
; Entry: A the distance, X and Y the caller's (the read's handler is entered with them). Exit: A bit 1 of the read,
; N and Z from it; Y, C and V as MariaReadBelow left them. X kept.
MariaReadAhead:
    clc
    adc a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$10
    jsr MariaReadBelow
    and #$02
    rts

; @routine 01DF:097 MariaSlideStart
; @run 6D:B097
; WHAT IT DOES: Starts her slide: a tile of bit 3 under her (16 rows below, or else 8) starts MariaBit3Burst,
; otherwise slots 1 and 2 are replaced as kind 4; she stops, takes the slide's speed the way she faces, and is in
; state $0D (animation $16) with left or right still held, else state $0F (animation $17), step 0.
; HOW: MariaReadBelow twice at her X; MariaBit3Burst or MariaReplaceSlotsOneTwo; MariaStopMoving; MariaSlideSpeeds by
; the facing doubled; TST of zPadHeld; the tail JMP to MariaSetAnimation.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaSetAnimation, after A the animation, Y the facing doubled, C its bit 7.
MariaSlideStart:
    lda a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$10
    jsr MariaReadBelow
    and #$08
    beq EnterStateWhenSlotClear
    jsr MariaBit3Burst
    bra MariaSlideStart_at0
    lda a:OBJ_X_LO
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$08
    jsr MariaReadBelow
    and #$08
    beq MariaSlideStart_at1
    jsr MariaBit3Burst
    bra MariaSlideStart_at0
MariaSlideStart_at1:
    jsr MariaReplaceSlotsOneTwo
MariaSlideStart_at0:
    jsr MariaStopMoving
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:MariaSlideSpeeds,y
    sta a:OBJ_VX_LO
    lda a:MariaSlideSpeeds+1,y
    sta a:OBJ_VX_HI
    tst #$A0,z:zPadHeld
    beq StepVerticalScrollTable
    lda #$0D
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    lda #$16
    jmp MariaSetAnimation
    lda #$0F
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    lda #$17
    jmp MariaSetAnimation
