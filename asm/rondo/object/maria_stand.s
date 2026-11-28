; @routine 01DD:4C4 MariaStandAttackFrame
; @run 6D:A4C4
; WHAT IT DOES: Step 1 of standing, the attack: while her animation runs, as standing still (MariaStandFrame), and
; with OBJ_WORK_A $20 (the attack's animation) still set that is all; when it has stopped, or OBJ_WORK_A has changed,
; the columns under her, a landing with footing, and back to standing in state 0.
; HOW: MariaAnimationRunning's Z; MariaStandFrame; OBJ_WORK_A compared with $20; MariaReadColumns, MariaHasNoFooting,
; MariaLandOnFloor; the tail JMP to MariaStand.
; WAITS: no.
; Entry: X and Y the caller's. Exit: held, A $20, Z and C set; not in state 0, A the state, N and Z from it; else as
; MariaStand.
MariaStandAttackFrame:
    jsr MariaAnimationRunning
    beq MariaStandAttackFrame_at0
    jsr MariaStandFrame
    lda a:OBJ_WORK_A
    cmp #$20
    bne MariaStandAttackFrame_at0
MariaStandAttackFrame_at2:
    rts
MariaStandAttackFrame_at0:
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcs MariaStandAttackFrame_at1
    jsr MariaLandOnFloor
MariaStandAttackFrame_at1:
    lda a:OBJ_STATE
    bne MariaStandAttackFrame_at2
    jmp MariaStand

; @routine 01DD:5E2 MariaStateStand
; @run 6D:A5E2
; WHAT IT DOES: State 0: the step's handler, from MariaStandSteps (still, the attack, the item crash).
; HOW: A BSR to MariaStepIndex, then the JMP (table,X).
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
MariaStateStand:
    bsr MariaStepIndex
    jmp (MariaStandSteps,x)

; @routine 01DD:5ED MariaGroundChecks
; @run 6D:A5ED
; WHAT IT DOES: The checks a step on the ground makes first: the columns under her; no footing, she falls; the jump
; pressed, she jumps; a landing; then the item crash and the attack button. When one of them fires, the step ends
; there: its return address is pulled and the fall, the jump, the item crash or the attack runs in its place.
; HOW: MariaReadColumns, MariaHasNoFooting, MariaJumpPressed, MariaLandOnFloor, MariaTryItemCrash and MariaUseItem,
; each carry set firing; PLA, PLA, then the JMP to MariaToFall, MariaJump, MariaCrashOnGround or MariaAttackFromUse.
; WAITS: no.
; Entry: X and Y the caller's. Exit: nothing fired, as MariaUseItem with C clear. Fired, the caller's return address
; gone and S two higher; as the routine jumped to, after A the return address's high byte.
MariaGroundChecks:
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcs MariaGroundChecks_at0
    jsr MariaJumpPressed
    bcs MariaGroundChecks_at1
    jsr MariaLandOnFloor
    jsr MariaTryItemCrash
    bcs MariaGroundChecks_at2
    jsr MariaUseItem
    bcs MariaGroundChecks_at3
    rts
MariaGroundChecks_at0:
    pla
    pla
    jmp MariaToFall
MariaGroundChecks_at1:
    pla
    pla
    jmp MariaJump
MariaGroundChecks_at2:
    pla
    pla
    jmp MariaCrashOnGround
MariaGroundChecks_at3:
    pla
    pla
    jmp MariaAttackFromUse

; @routine 01DD:61C MariaStandFrame
; @run 6D:A61C
; WHAT IT DOES: Step 0 of standing (and of the standing attack, which calls it): the ground checks; a ledge, she takes
; hold of it; MariaUpHeld, she crouches. In step 0: up pressed, she looks up (animation 7, its step 9, timer $20, one
; tick); outside state 0, while PlayerStartPose is set, frame 1; else when her animation has stopped, with no push on
; the floor and all three columns under her solid, the next idle animation (OBJ_WORK_A plus one), otherwise animation
; 7. Then, right or left held, she faces that way and, unless button I is held, walks (state 1, animation 0).
; HOW: MariaGroundChecks, MariaLedgeCheck and MariaClimbStart, MariaUpHeld and MariaCrouch; TST of zPadPressed and
; zPadHeld; MariaSetAnimation, MariaAnimationTick, MariaAnimationRunning; the pad's forced presses off before facing.
; The walk runs on into MariaSetAnimationZero.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as the check or move that ended it; standing on with no direction, Y 1, Z set, N
; clear, V bit 6 of zPadHeld; facing with button I held, Y the facing, Z clear, V bit 6 of zPadHeld.
MariaStandFrame:
    jsr MariaGroundChecks
    jsr MariaLedgeCheck
    bcc MariaStandFrame_at0
    jmp MariaClimbStart
MariaStandFrame_at0:
    jsr MariaUpHeld
    bcc CameraMode0A
    jmp MariaCrouch
    lda a:OBJ_SUBSTATE
    cmp #$01
    beq MariaStandFrame_at1
    tst #$10,z:zPadPressed
    beq MariaStandFrame_at2
    lda #$07
    sta a:OBJ_WORK_A
    jsr MariaSetAnimation
    lda #$09
    sta a:OBJ_SCROLL_BAND
    lda #$20
    sta a:OBJ_HURT_TIMER
    jsr MariaAnimationTick
    bra MariaStandFrame_at1
MariaStandFrame_at2:
    lda a:OBJ_STATE
    beq MariaStandFrame_at3
    lda a:PlayerStartPose
    beq MariaStandFrame_at3
    lda #$01
    sta a:OBJ_FRAME
    bra MariaStandFrame_at1
MariaStandFrame_at3:
    jsr MariaAnimationRunning
    bne MariaStandFrame_at1
    lda z:zPlayerPush
    and #$01
    bne MariaStandFrame_at4
    lda a:RichterColumnHit
    and a:RichterColumnHit+1
    and a:RichterColumnHit+2
    and #$01
    beq MariaStandFrame_at4
    inc a:OBJ_WORK_A
    lda a:OBJ_WORK_A
    jsr MariaSetAnimation
MariaStandFrame_at1:
    cly
    tst #$20,z:zPadHeld
    bne MariaStandFrame_at5
    iny
    tst #$80,z:zPadHeld
    bne MariaStandFrame_at5
    rts
MariaStandFrame_at4:
    lda #$07
    jsr MariaSetAnimation
    bra MariaStandFrame_at1
MariaStandFrame_at5:
    jsr MariaPadForcesOff
    sty a:OBJ_FACING_LEFT
    tst #$01,z:zPadHeld
    bne MariaSpeedWalk_done
    lda #$01
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE

; @routine 01DD:746 MariaClimbStart
; @run 6D:A746
; WHAT IT DOES: Takes hold of the ledge MariaLedgeCheck found, to climb it: the low ledge (zTemp01 0) or the high one,
; each in one of two holds by whether the point past the ledge's edge reads bit 2: states 5 and 6 low, 7 and 8 high.
; PlayerClimbReach is how far she has to go across to the edge; on the high ledge she faces the way the reach points.
; She stops, and the state's step is 2.
; HOW: The column's offset (MariaColumnOffset) and 8 more past it, read $0A below her for the low ledge, $1E for the
; high (MariaReadBelow); the reach from the offset less zTemp00 for the low, less the column's RichterColumnX for the
; high. Animation 0 unless she was walking. The pad's forced presses go off first.
; WAITS: no.
; Entry: X the column, zTemp00 the low ledge's X, zTemp01 low or high. Exit: A 2, N and Z clear, X the state; Y the
; column's offset index (low) or the reach (high), or as MariaSetAnimation; C set unless she was standing, or as
; MariaSetAnimation; V from the reach's last add or subtract.
MariaClimbStart:
    jsr MariaPadForcesOff
    stz z:zFieldC4
    lda #$01
    sta a:PlayerMayLeaveTop
    lda z:zTemp01
    bne MariaClimbStart_at0
    lda z:zTemp00
    pha
    jsr MariaColumnOffset
    clc
    adc a:OBJ_X_LO
    clc
    adc #$08
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$0A
    jsr MariaReadBelow
    and #$04
    beq MariaClimbStart_at1
    pla
    sta z:zTemp00
    jsr MariaColumnOffset
    sec
    sbc z:zTemp00
    sec
    sbc #$08
    ldx #$05
MariaClimbStart_at3:
    sta a:PlayerClimbReach
    lda a:OBJ_STATE
    cmp #$01
    beq MariaClimbStart_at2
    lda #$00
    jsr MariaSetAnimation
MariaClimbStart_at2:
    lda #$02
    sta a:OBJ_SUBSTATE
    stx a:OBJ_STATE
    jmp MariaStopMoving
MariaClimbStart_at1:
    pla
    sta z:zTemp00
    jsr MariaColumnOffset
    sec
    sbc z:zTemp00
    clc
    adc #$07
    clc
    adc #$08
    ldx #$06
    bra MariaClimbStart_at3
MariaClimbStart_at0:
    jsr MariaColumnOffset
    clc
    adc a:OBJ_X_LO
    clc
    adc #$08
    sta z:zTemp00
    lda a:OBJ_Y_LO
    clc
    adc #$1E
    jsr MariaReadBelow
    and #$04
    beq MariaClimbStart_at4
    jsr MariaColumnOffset
    sec
    sbc a:RichterColumnX,x
    clc
    adc #$07
    ldx #$07
MariaClimbStart_at6:
    tay
    bpl MariaClimbStart_at5
    stz a:OBJ_FACING_LEFT
    inc a:OBJ_FACING_LEFT
    bra MariaClimbStart_at3
MariaClimbStart_at5:
    stz a:OBJ_FACING_LEFT
    bra MariaClimbStart_at3
MariaClimbStart_at4:
    jsr MariaColumnOffset
    sec
    sbc a:RichterColumnX,x
    ldx #$08
    bra MariaClimbStart_at6
