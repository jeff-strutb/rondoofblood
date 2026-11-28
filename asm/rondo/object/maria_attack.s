; @routine 01DD:366 MariaCrashInAir
; @run 6D:A366
; WHAT IT DOES: Starts her item crash in the air: step 5 of the state she is in, animation $2B, then as
; MariaCrashOnGround.
; HOW: The store, then a branch into MariaCrashOnGround's start.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaCrashOnGround.
MariaCrashInAir:
    lda #$05
    sta a:OBJ_SUBSTATE
    lda #$2B
    bra MariaCrashOnGround_start

; @routine 01DD:36F MariaCrashOnGround
; @run 6D:A36F
; WHAT IT DOES: Starts her item crash from the ground: state 0 step 2, animation $2A, then the crash's object, kind,
; sound and slowed time.
; HOW: The stores, then the crash's start (MariaSetAnimation, MariaStopMoving, ClearObject, SoundCommand when the
; crash has a sound, the tail JMP to MariaPadForcesOff).
; WAITS: no.
; Entry: X and Y the caller's. Exit: A $FF, N and Z from it; X zCrashKind and C and V from its subtract, or X, Y, C
; and V as SoundCommand left them; Y as MariaSetAnimation left it with no sound.
MariaCrashOnGround:
    lda #$00
    sta a:OBJ_STATE
    lda #$02
    sta a:OBJ_SUBSTATE
    lda #$2A
MariaCrashOnGround_start:
    jsr MariaSetAnimation
    jsr MariaStopMoving
    ldx #$05
    jsr VecClearObject
    ldx z:zField93
    lda a:MariaCrashKinds,x
    sta a:OBJ_KIND+5
    sec
    sbc #$0D
    sta z:zCrashKind
    tax
    lda a:MariaCrashSounds,x
    beq MariaCrashOnGround_at0
    jsr VecSoundCommand
MariaCrashOnGround_at0:
    stz z:zCrashPhase
    stz z:zFieldD2
    lda #$02
    sta a:TimeSlowMode
    lda #$FF
    sta a:OBJ_ANIM_TIMER
    jmp MariaPadForcesOff

; @routine 01DD:3C0 MariaSequenceMoveStart
; @run 6D:A3C0
; WHAT IT DOES: Starts the move of her pad sequence: sound $6A, state $11 step 0, object slot 15 of kind $0A, animation
; $29, the pad's forced presses off, all speed gone.
; HOW: SoundCommand, ClearObject, MariaSetAnimation, MariaPadForcesOff, then the tail JMP to MariaStopMoving.
; WAITS: no.
; Entry: X and Y the caller's (SoundCommand is given them). Exit: as MariaSetAnimation, after A $29 and X 15.
MariaSequenceMoveStart:
    lda #$6A
    jsr VecSoundCommand
    lda #$11
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    ldx #$0F
    jsr VecClearObject
    lda #$0A
    sta a:OBJ_KIND,x
    lda #$29
    jsr MariaSetAnimation
    jsr MariaPadForcesOff
    jmp MariaStopMoving

; @routine 01DD:3E4 MariaAttack
; @run 6D:A3E4
; WHAT IT DOES: Does what the attack button said (PlayerAttackKind): her pad sequence's move ($FF), the item thrown
; (1), or her attack's object (0).
; HOW: The sign, then zero, of PlayerAttackKind; on into MariaAttackSlot.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaSequenceMoveStart, MariaThrowItem or MariaAttackSlot.
MariaAttack:
    lda a:PlayerAttackKind
    bmi MariaSequenceMoveStart
    bne MariaThrowItem

; @routine 01DD:3EB MariaAttackSlot
; @run 6D:A3EB
; WHAT IT DOES: Starts her attack's object: the first of slots 3 and 4 that is empty becomes kind 1, with sound 4, and
; the pad's second forced press is cleared. With both out, nothing.
; HOW: OBJ_KIND not zero is out; ClearObject, then SoundCommand.
; WAITS: no.
; Entry: Y the caller's (SoundCommand is given it). Exit: both out, A slot 4's kind, N and Z from it, X 4. Started,
; A 0, Z set, N clear; X, Y, C and V as SoundCommand left them (X the slot when the command was dropped).
MariaAttackSlot:
    ldx #$03
    lda a:OBJ_KIND,x
    bne MariaAttackSlot_at0
MariaAttackSlot_at2:
    jsr VecClearObject
    lda #$01
    sta a:OBJ_KIND,x
    lda #$04
    jsr VecSoundCommand
MariaAttackSlot_end:
    lda #$00
    sta a:PadForcedPressB
MariaAttackSlot_at1:
    rts
MariaAttackSlot_at0:
    inx
    lda a:OBJ_KIND,x
    bne MariaAttackSlot_at1
    bra MariaAttackSlot_at2

; @routine 01DD:40D MariaAttackFromUse
; @run 6D:A40D
; WHAT IT DOES: The attack button's press as MariaUseItem left it: her pad sequence's move; the standing attack with
; the item thrown; or the attack's object, and if it started, the standing attack.
; HOW: PlayerAttackKind read for its sign and again for zero; a BSR to MariaAttack, whose Z says the object started;
; on into MariaStandAttack.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaSequenceMoveStart, MariaAttackAndThrow, MariaAttack when nothing started,
; or MariaStandAttack.
MariaAttackFromUse:
    lda a:PlayerAttackKind
    bmi MariaSequenceMoveStart
    lda a:PlayerAttackKind
    bne MariaAttackAndThrow
    bsr MariaAttack
    bne MariaStandAttack_done

; @routine 01DD:41B MariaStandAttack
; @run 6D:A41B
; WHAT IT DOES: Puts her in the standing attack: state 0, step 1, animation $20.
; HOW: MariaSetAnimation between the stores.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z from it; Y and C as MariaSetAnimation left them; X and V kept.
MariaStandAttack:
    lda #$00
    sta a:OBJ_STATE
    lda #$20
    jsr MariaSetAnimation
    lda #$01
    sta a:OBJ_SUBSTATE
MariaStandAttack_done:
    rts

; @routine 01DD:42B MariaAttackAndThrow
; @run 6D:A42B
; WHAT IT DOES: The standing attack, then the item thrown.
; HOW: A BSR to MariaStandAttack, then on into MariaThrowItem.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaThrowItem.
MariaAttackAndThrow:
    bsr MariaStandAttack

; @routine 01DD:42D MariaThrowItem
; @run 6D:A42D
; WHAT IT DOES: Throws the item in hand (zField93), with sound $69. Item 7 starts her attack and, with it, an object
; of kind 7 in the first empty slot of 5 to 14 that keeps the attack's slot in OBJ_WORK_D. Item 4 is a state of its
; own: state $10 with animation $26 standing or $27 walking (nothing in a state MariaItem4ByState gives 2), no speed,
; an object of kind 4 in slot 5 and, with $50 hearts or more, one of kind $0C in slot 9. Any other item is an object
; of the item's number in slot 5, or 8 when 5 is out, and the pad's second forced press is cleared.
; HOW: SoundCommand, then by item; MariaAttackSlot, ClearObject, MariaSetAnimation and MariaStopMoving.
; WAITS: no.
; Entry: X and Y the caller's (SoundCommand is given them). Exit: item 7, as MariaAttackSlot when it started nothing;
; with no slot from 5 to 14, A slot 14's kind, X 15, Y as MariaAttackSlot left X, Z and C set; else A and Y that X,
; N and Z from it, X the slot, C clear past slot 5. Item 4, in a state giving 2: A 2, X the state, Z and C set; else
; X 5 and A zField94, N and Z from it less $50, C clear, or with $50 hearts A $0C, X 9, C set. Other items: A 0, Z
; set, X the slot, C item 4 or more, V bit 6 of slot 5's kind, Y as SoundCommand left it.
MariaThrowItem:
    lda #$69
    jsr VecSoundCommand
    lda z:zField93
    cmp #$07
    beq MariaThrowItem_at0
    cmp #$04
    beq MariaThrowItem_at1
    tst #$FF,a:OBJ_KIND+5
    bne MariaThrowItem_at2
    ldx #$05
MariaThrowItem_at3:
    jsr VecClearObject
    sta a:OBJ_KIND,x
    bra MariaAttackSlot_end
MariaThrowItem_at2:
    ldx #$08
    bra MariaThrowItem_at3
MariaThrowItem_at0:
    jsr MariaAttackSlot
    beq MariaThrowItem_at4
    rts
MariaThrowItem_at4:
    txa
    tay
    ldx #$05
MariaThrowItem_at6:
    lda a:OBJ_KIND,x
    beq MariaThrowItem_at5
    inx
    cpx #$0F
    bne MariaThrowItem_at6
    rts
MariaThrowItem_at5:
    jsr VecClearObject
    lda #$07
    sta a:OBJ_KIND,x
    tya
    sta a:OBJ_WORK_D,x
    rts
MariaThrowItem_at1:
    ldx a:OBJ_STATE
    lda a:MariaItem4ByState,x
    beq InitObjectFromRecord
    cmp #$02
    beq MariaThrowItem_at7
    lda #$27
    bra MariaThrowItem_at8
    lda #$26
MariaThrowItem_at8:
    jsr MariaSetAnimation
    stz a:OBJ_WORK_D
    lda #$10
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    ldx #$05
    jsr VecClearObject
    lda #$04
    sta a:OBJ_KIND,x
    jsr MariaStopMoving
    lda z:zField94
    cmp #$50
    bcc MariaThrowItem_at7
    ldx #$09
    jsr VecClearObject
    lda #$0C
    sta a:OBJ_KIND,x
MariaThrowItem_at7:
    rts

; @routine 01DD:4E7 MariaCrashFrame
; @run 6D:A4E7
; WHAT IT DOES: A frame of her item crash (state 0 step 2): the columns under her, a landing when she has footing,
; then the crash's frame handler by its kind (MariaCrashHandlers).
; HOW: MariaReadColumns, MariaHasNoFooting's carry, MariaLandOnFloor; zCrashKind doubled, the JMP (table,X).
; WAITS: no.
; Entry: nothing. Exit: as the kind's handler, after A and X the doubled kind and C its bit 7.
MariaCrashFrame:
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcs MariaCrashFrame_at0
    jsr MariaLandOnFloor
MariaCrashFrame_at0:
    lda z:zCrashKind
    asl a
    tax
    jmp (MariaCrashHandlers,x)

; @routine 01DD:507 MariaCrashEnd
; @run 6D:A507
; WHAT IT DOES: The crash frame of kinds 4 and 5: time back to normal; when her animation has stopped, the crash's end
; (MariaCrashRecover, then standing in state 0, or her ledge's hold).
; HOW: MariaAnimationRunning's Z; the tail JMPs to MariaStand and MariaClimbHold.
; WAITS: no.
; Entry: nothing. Exit: running, as MariaAnimationRunning; else as MariaStand, or MariaClimbHold.
MariaCrashEnd:
    stz a:TimeSlowMode
    jsr MariaAnimationRunning
    bne MariaCrashRecover_done
MariaCrashEnd_over:
    bsr MariaCrashRecover
    lda a:OBJ_STATE
    bne MariaCrashEnd_at0
    stz a:PlayerMayLeaveTop
    jmp MariaStand
MariaCrashEnd_at0:
    stz a:OBJ_SUBSTATE
    jmp MariaClimbHold

; @routine 01DD:522 MariaCrashRecover
; @run 6D:A522
; WHAT IT DOES: The end of a crash for her: the item's first use to come again, and no hit timer.
; HOW: PlayerItemFirstUse 1; OBJ_ANIM_TIMER $20, then 0.
; WAITS: no.
; Entry: nothing. Exit: A $20, N and Z from it. X, Y, C and V kept.
MariaCrashRecover:
    lda #$01
    sta a:PlayerItemFirstUse
    lda #$20
    sta a:OBJ_ANIM_TIMER
    stz a:OBJ_ANIM_TIMER
MariaCrashRecover_done:
    rts

; @routine 01DD:530 MariaCrashTurn
; @run 6D:A530
; WHAT IT DOES: The crash frame of kind 6: in phase 0 her animation runs until frame $52, then turns to animation $2F
; and the phase goes on; after that, when the animation has stopped, the crash's end.
; HOW: MariaAnimationTick and the frame, MariaSetAnimation; MariaAnimationRunning's Z, then MariaCrashEnd's tail.
; WAITS: no.
; Entry: nothing. Exit: phase 0, A the frame, N, Z and C from comparing it with $52, or after the turn as
; MariaSetAnimation with N and Z from the new phase; later, as MariaAnimationRunning, or the crash's end.
MariaCrashTurn:
    lda z:zCrashPhase
    bne MariaCrashTurn_at0
    jsr MariaAnimationTick
    lda a:OBJ_FRAME
    cmp #$52
    bne MariaCrashRecover_done
    lda #$2F
    jsr MariaSetAnimation
    bra MariaCrashWait_next
MariaCrashTurn_at0:
    jsr MariaAnimationRunning
    bne MariaCrashRecover_done
    bra MariaCrashEnd_over

; @routine 01DD:54C MariaCrashPhase
; @run 6D:A54C
; WHAT IT DOES: The crash frame of kind 1: its phase's handler, from MariaCrashPhases (wait, signal, rise).
; HOW: zCrashPhase doubled; the JMP (table,X).
; WAITS: no.
; Entry: nothing. Exit: as the phase's handler.
MariaCrashPhase:
    lda z:zCrashPhase
    asl a
    tax
    jmp (MariaCrashPhases,x)

; @routine 01DD:559 MariaCrashWait
; @run 6D:A559
; WHAT IT DOES: A crash's first phase: her animation runs until frame $52, then script $1B starts and the phase goes
; on.
; HOW: MariaAnimationTick and the frame; MariaStartHitScript; INC zCrashPhase.
; WAITS: no.
; Entry: nothing. Exit: A the frame, N, Z and C from comparing it with $52; at frame $52, as MariaStartHitScript with
; $1B, N and Z from the new phase, C set.
MariaCrashWait:
    jsr MariaAnimationTick
    lda a:OBJ_FRAME
    cmp #$52
    bne MariaCrashWait_at0
    lda #$1B
    jsr MariaStartHitScript
MariaCrashWait_next:
    inc z:zCrashPhase
MariaCrashWait_at0:
    rts

; @routine 01DD:56B MariaCrashSignal
; @run 6D:A56B
; WHAT IT DOES: A crash's phase that waits on zFieldD2: once set, animation $2C, zFieldD2 2, her animation stopped,
; time back to normal, she may leave the top of the map with no push, and the phase goes on.
; HOW: The test, MariaSetAnimation, the stores, INC zCrashPhase.
; WAITS: no.
; Entry: nothing. Exit: waiting, A 0, Z set. Else A 1, N and Z from the new phase; Y and C as MariaSetAnimation left
; them.
MariaCrashSignal:
    lda z:zFieldD2
    bne MariaCrashSignal_at0
MariaCrashSignal_at1:
    rts
MariaCrashSignal_at0:
    lda #$2C
    jsr MariaSetAnimation
    lda #$02
    sta z:zFieldD2
    lda #$FF
    sta a:OBJ_ANIM_TIMER
    stz a:TimeSlowMode
    lda #$01
    sta a:PlayerMayLeaveTop
    stz z:zPlayerPush
    jmp MariaCrashWait_next

; @routine 01DD:58B MariaCrashRise
; @run 6D:A58B
; WHAT IT DOES: A crash's last phase: her animation runs and she may leave the top of the map; once zFieldD2 is set,
; the crash's end for her and a jump, from no higher than row 0.
; HOW: MariaAnimationTick; MariaCrashRecover; her Y below $10 is set to 0; the tail JMP to MariaJump.
; WAITS: no.
; Entry: nothing. Exit: waiting, A 0, Z set, and C and Y as MariaAnimationTick left them. Else as MariaJump.
MariaCrashRise:
    jsr MariaAnimationTick
    lda #$01
    sta a:PlayerMayLeaveTop
    stz z:zPlayerPush
    lda z:zFieldD2
    beq MariaCrashSignal_at1
    jsr MariaCrashRecover
    stz a:PlayerMayLeaveTop
    lda a:OBJ_Y_LO
    clc
    adc #$F0
    bcs MariaCrashRise_at0
    stz a:OBJ_Y_LO
MariaCrashRise_at0:
    jmp MariaJump

; @routine 01DD:5AD MariaCrashFall
; @run 6D:A5AD
; WHAT IT DOES: The crash frame of kinds 2 and 3: phase 0 as MariaCrashWait; after it, when her animation has stopped,
; time back to normal and the crash's end.
; HOW: zCrashPhase; the tail JMP to MariaCrashWait; MariaAnimationRunning's Z, then MariaCrashEnd's tail.
; WAITS: no.
; Entry: nothing. Exit: as MariaCrashWait, MariaAnimationRunning, or the crash's end.
MariaCrashFall:
    lda z:zCrashPhase
    bne CollectScoreItem
    jmp MariaCrashWait
    jsr MariaAnimationRunning
    beq MariaCrashFall_at0
    rts
MariaCrashFall_at0:
    stz a:TimeSlowMode
    jmp MariaCrashEnd_over

; @routine 01DD:5C0 MariaCrashPhaseB
; @run 6D:A5C0
; WHAT IT DOES: The crash frame of kind 0: its phase's handler, from MariaCrashPhasesB (wait, land, rise).
; HOW: zCrashPhase doubled; the JMP (table,X).
; WAITS: no.
; Entry: nothing. Exit: as the phase's handler.
MariaCrashPhaseB:
    lda z:zCrashPhase
    asl a
    tax
    jmp (MariaCrashPhasesB,x)

; @routine 01DD:5CD MariaCrashLand
; @run 6D:A5CD
; WHAT IT DOES: Phase 1 of crash kind 0: MariaCrashSignal, and once that has moved the phase on, animation $2D.
; HOW: MariaCrashSignal, then zCrashPhase compared with 1; the tail JMP to MariaSetAnimation.
; WAITS: no.
; Entry: nothing. Exit: still phase 1, A 1, Z and C set. Else as MariaSetAnimation.
MariaCrashLand:
    jsr MariaCrashSignal
    lda z:zCrashPhase
    cmp #$01
    bne MariaCrashLand_at0
    rts
MariaCrashLand_at0:
    lda #$2D
    jmp MariaSetAnimation

; @routine 01DD:6AD MariaSetAnimationZero
; @run 6D:A6AD
; WHAT IT DOES: Sets her animation 0.
; HOW: The tail JMP to MariaSetAnimation.
; WAITS: no.
; Entry: nothing. Exit: as MariaSetAnimation.
MariaSetAnimationZero:
    lda #$00
    jmp MariaSetAnimation

; @routine 01DD:6B2 MariaSpeedWalk
; @run 6D:A6B2
; WHAT IT DOES: Gives her the walking speed the way she faces ($0160 right, $FEA0 left).
; HOW: MariaWalkSpeeds by OBJ_FACING_LEFT doubled.
; WAITS: no.
; Entry: nothing. Exit: A the speed's high byte, N and Z from it; Y the facing doubled; C bit 7 of the facing. X and V
; kept.
MariaSpeedWalk:
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:MariaWalkSpeeds,y
    sta a:OBJ_VX_LO
    lda a:MariaWalkSpeeds+1,y
    sta a:OBJ_VX_HI
MariaSpeedWalk_done:
    rts

; @routine 01DD:6C8 MariaJump
; @run 6D:A6C8
; WHAT IT DOES: Starts her jump (state 3, step 0): may not leave the top, the pad's second forced press and zFieldC6
; cleared, the rise begun. With right or left held, she faces that way for animation $0B and takes the walking speed;
; jumping the way she did not face, she keeps her facing and the animation is $0A, as for a jump straight up.
; HOW: A BSR to MariaJumpSpeed; BBS5 and BBS7 of zPadHeld (two reads) give Y 0 or 1 for the facing; MariaSetAnimation,
; MariaSpeedWalk; her old facing from the stack compared with the new.
; WAITS: no.
; Entry: nothing. Exit: straight up or across backward, as MariaSetAnimation with $0A. Across forward, A the facing, Z
; and C set; Y as MariaSpeedWalk left it.
MariaJump:
    stz a:PlayerMayLeaveTop
    stz a:PadForcedPressB
    stz z:zFieldC6
    lda #$03
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    bsr MariaJumpSpeed
    cly
    bbs5 zPadHeld,MariaJump_at0
    iny
    bbs7 zPadHeld,MariaJump_at0
    lda #$0A
    jmp MariaSetAnimation
MariaJump_at0:
    lda a:OBJ_FACING_LEFT
    pha
    sty a:OBJ_FACING_LEFT
    lda #$0B
    jsr MariaSetAnimation
    jsr MariaSpeedWalk
    pla
    cmp a:OBJ_FACING_LEFT
    bne MariaJump_at1
    sta a:OBJ_FACING_LEFT
    rts
MariaJump_at1:
    sta a:OBJ_FACING_LEFT
    lda #$0A
    jmp MariaSetAnimation

; @routine 01DD:70A MariaJumpSpeed
; @run 6D:A70A
; WHAT IT DOES: Starts her jump's rise: keeps her Y on the map (PlayerJumpStartY), stops her, gives her Y speed $FC80
; (up), and lifts her three rows.
; HOW: Her Y plus ScrollYLo, the scroll's screen and the carry; MariaStopMoving; on into MariaLiftThree.
; WAITS: no.
; Entry: nothing. Exit: as MariaLiftThree.
MariaJumpSpeed:
    lda a:OBJ_Y_LO
    clc
    adc a:ScrollYLo
    sta a:PlayerJumpStartYLo
    lda a:ScrollYHiScreen
    adc #$00
    sta a:PlayerJumpStartYHi
    jsr MariaStopMoving
    lda #$80
    sta a:OBJ_VY_LO
    lda #$FC
    sta a:OBJ_VY_HI

; @routine 01DD:729 MariaLiftThree
; @run 6D:A729
; WHAT IT DOES: Moves her up three rows, no higher than the top of the map (Y $0000).
; HOW: A 16-bit subtract; a high byte of $FF is put back to zero.
; WAITS: no.
; Entry: nothing. Exit: A her Y's high byte, N, Z and C from comparing it with $FF, V from the subtract; at the top A
; $FF, Z and C set. X and Y kept.
MariaLiftThree:
    lda a:OBJ_Y_LO
    sec
    sbc #$03
    sta a:OBJ_Y_LO
    lda a:OBJ_Y_HI
    sbc #$00
    sta a:OBJ_Y_HI
    cmp #$FF
    beq MariaLiftThree_at0
    rts
MariaLiftThree_at0:
    stz a:OBJ_Y_LO
    stz a:OBJ_Y_HI
    rts

; @routine 01DE:707 MariaClimbHold
; @run 6D:AF07
; WHAT IT DOES: Puts her back in her ledge's hold (states 5 to 8): the attack button's outcome cleared, her frame and
; OBJ_WORK_A by low or high ledge, her animation stopped (OBJ_HURT_TIMER $FF), step 0.
; HOW: (state - 5) / 2 picks MariaClimbFrames and MariaClimbWork.
; WAITS: no.
; Entry: nothing. Exit: A $FF, N and Z from it; Y the ledge's index; C bit 0 of state less 5, V from that subtract.
; X kept.
MariaClimbHold:
    stz a:PlayerAttackKind
    lda a:OBJ_STATE
    sec
    sbc #$05
    lsr a
    tay
    lda a:MariaClimbFrames,y
    sta a:OBJ_FRAME
    lda a:MariaClimbWork,y
    sta a:OBJ_WORK_A
    lda #$FF
    sta a:OBJ_HURT_TIMER
    stz a:OBJ_SUBSTATE
    rts
