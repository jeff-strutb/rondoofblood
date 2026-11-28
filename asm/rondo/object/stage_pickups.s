; @routine 0250:29D StageGateAdvance
; @run 6B:9A9D
; WHAT IT DOES: Unless zField8C is negative, advances the object's state with $04 in its OBJ_GATE_WAIT, twice for the
; start arguments whose low nibble is 2 to 5, $0A or $0B.
; HOW: CMP chain on OBJ_START_ARG's low nibble.
; WAITS: no.
; Entry: X object. Exit: negative: A zField8C; else A the nibble, N, Z and C from the last compare or INC.
StageGateAdvance:
    lda z:zField8C
    bmi StageGateAdvance_at0
    inc a:OBJ_STATE,x
    lda #$04
    sta a:OBJ_GATE_WAIT,x
    lda a:OBJ_START_ARG,x
    and #$0F
    cmp #$02
    bcc StageGateAdvance_at1
    cmp #$06
    bcc StageGateAdvance_at2
    cmp #$0A
    beq StageGateAdvance_at2
    cmp #$0B
    beq StageGateAdvance_at2
StageGateAdvance_at1:
    bra StageGateAdvance_at0
StageGateAdvance_at2:
    inc a:OBJ_STATE,x
StageGateAdvance_at0:
    rts

; @routine 0259:36A StageObjectRemove
; @run 70:A36A
; WHAT IT DOES: Empties the object's slot.
; HOW: The tail JMP to the kernel's ClearObject vector.
; WAITS: no.
; Entry: X object. Exit: A, X, Y and flags kept.
StageObjectRemove:
    jmp VecClearObject

; @routine 0259:5DA StageObjectBecomeScore
; @run 70:A5DA
; WHAT IT DOES: Turns the object into the score marker (kind $9F) after blending the score added: its OBJ_MARKER_VALUE from
; the value in Y, its state cleared.
; HOW: PHY, BlendStageScoreAddRows by its $9181 C, PLA.
; WAITS: no.
; Entry: X object, Y the marker's value. Exit: A $9F, N and Z from it; X, Y, C and V as the blend left them.
StageObjectBecomeScore:
    phy
    jsr BlendStageScoreAddRows
    pla
    sta a:OBJ_MARKER_VALUE,x
    lda #$9F
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    rts

; @routine 0259:675 StageFlashStart
; @run 70:A675
; WHAT IT DOES: Starts the screen flash unless one is running, in which case the object is cleared: the sound $50, the
; sole script slot 3 with script 0, $12 frames, bit 1 of HitAttackFlags set, the object's frame cleared and its state
; counted on.
; HOW: StageFlashActive; the sound hook; StartSoleScriptSlot with X kept on the stack.
; WAITS: no.
; Entry: X object. Exit: running: as ClearObject (A the flag); else N and Z from the state's INC, A HitAttackFlags.
StageFlashStart:
    lda a:StageFlashActive
    beq StageFlashStart_at0
    jmp VecClearObject
StageFlashStart_at0:
    lda #$01
    sta a:StageFlashActive
    lda #$50
    jsr VecSoundCommand
    phx
    cla
    ldx #$03
    jsr VecStartSoleScriptSlot
    plx
    lda #$12
    sta a:OBJ_FLASH_FRAMES,x
    lda a:HitAttackFlags
    ora #$02
    sta a:HitAttackFlags
    stz a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
    rts

; @routine 0259:6A3 StagePickupPotion
; @run 70:A6A3
; WHAT IT DOES: The potion taken: sound $0B, the player's invincibility timer $80, the object cleared.
; HOW: The sound hook, a store, the tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: A $80, N and Z from it; X, Y, C and V as the sound left them.
StagePickupPotion:
    lda #$0B
    jsr VecSoundCommand
    lda #$80
    sta a:PlayerInvincibleTimer
    jmp VecClearObject

; @routine 025A:009 StageFlashEnd
; @run 70:A809
; WHAT IT DOES: The flash's countdown: bit 1 of HitAttackFlags cleared; when the object's OBJ_FLASH_FRAMES runs out the
; flash is over and the object cleared.
; HOW: AND and a store, DEC, the tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: A HitAttackFlags' new value; counting N and Z from the DEC, else as ClearObject.
StageFlashEnd:
    lda a:HitAttackFlags
    and #$FD
    sta a:HitAttackFlags
    dec a:OBJ_FLASH_FRAMES,x
    bne StageFlashEnd_at0
    stz a:StageFlashActive
    jmp VecClearObject
StageFlashEnd_at0:
    rts

; @routine 025C:126 StageObjectRiseToTop
; @run 70:B926
; WHAT IT DOES: Moves the object up a pixel while its Y low byte is $30 or more.
; HOW: CMP, DEC.
; WAITS: no.
; Entry: X object. Exit: A its Y low, C 1 when it moved, N and Z from the compare or the DEC.
StageObjectRiseToTop:
    lda a:OBJ_Y_LO,x
    cmp #$30
    bcc StageObjectRiseToTop_at0
    dec a:OBJ_Y_LO,x
StageObjectRiseToTop_at0:
    rts


; @routine 0250:1DB StageGateShow
; @run 6B:99DB
; WHAT IT DOES: Sets a gate up: its object record loaded and its frame by side.
; HOW: LoadObjectRecord; StageGateFrames by the start argument's bit 0.
; WAITS: no.
; Entry: X object. Exit: A the frame, N and Z from it, Y the side; C as the record load left it, V and X kept.
StageGateShow:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:StageGateFrames,y
    sta a:OBJ_FRAME,x
    rts

; @routine 0250:255 StageGateWaitRoom
; @run 6B:9A55
; WHAT IT DOES: Waits for the stage's step 2 (zField85), then advances the gate's state.
; HOW: CMP, INC.
; WAITS: no.
; Entry: X object. Exit: A zField85; C and Z from the compare, or N and Z from the INC.
StageGateWaitRoom:
    lda z:zField85
    cmp #$02
    bne StageGateAdvance_at0
    inc a:OBJ_STATE,x
    rts

; @routine 0250:25F StageGateRise
; @run 6B:9A5F
; WHAT IT DOES: Opens a gate a step every four frames; at the fourth step the gate's state advances, bit 7 of zField8C
; is set and the sound $71 goes out.
; HOW: OBJ_GATE_WAIT counts the frames, OBJ_GATE_STEP the steps; the frame and its list as gate_frame.
; WAITS: no.
; Entry: X object. Exit: waiting: N and Z from the DEC; the last step as SoundCommand; else as the list start, X
; kept.
StageGateRise:
    dec a:OBJ_GATE_WAIT,x
    bne StageGateRise_at0
    inc a:OBJ_GATE_STEP,x
    lda a:OBJ_GATE_STEP,x
    cmp #$04
    bne StageGateRise_at1
    inc a:OBJ_STATE,x
    lda z:zField8C
    ora #$80
    sta z:zField8C
    lda #$71
    jmp VecSoundCommand
StageGateRise_at1:
    lda #$04
    sta a:OBJ_GATE_WAIT,x
    lda a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:StageGateFrames,y
    clc
    adc a:OBJ_GATE_STEP,x
    sta a:OBJ_FRAME,x
    clc
    adc a:StageGateLists,y
    phx
    ldx #$05
    jsr VecStartListChannelKind2
    plx
StageGateRise_at0:
    rts

; @routine 0250:2C4 StageGateFall
; @run 6B:9AC4
; WHAT IT DOES: Closes a gate a step every four frames; past step 0 the gate's state advances with the sound $71.
; HOW: As StageGateRise, counting the step down.
; WAITS: no.
; Entry: X object. Exit: as StageGateRise.
StageGateFall:
    dec a:OBJ_GATE_WAIT,x
    bne StageGateFall_at0
    dec a:OBJ_GATE_STEP,x
    bpl StageGateFall_at1
    inc a:OBJ_STATE,x
    lda #$71
    jmp VecSoundCommand
StageGateFall_at1:
    lda #$04
    sta a:OBJ_GATE_WAIT,x
    lda a:OBJ_START_ARG,x
    and #$01
    tay
    lda a:StageGateFrames,y
    clc
    adc a:OBJ_GATE_STEP,x
    sta a:OBJ_FRAME,x
    clc
    adc a:StageGateLists,y
    phx
    ldx #$05
    jsr VecStartListChannelKind2
    plx
StageGateFall_at0:
    rts

; @routine 0259:6DB StagePickupExtraLife
; @run 70:A6DB
; WHAT IT DOES: The 1-up taken: the object becomes the score marker, zField90 1, the sound $52, one added to the stock
; in decimal (held at 99), and the stock rows blended.
; HOW: SED, CLC, ADC #1, CLD; BCC; the tail JMP to $9151 (BlendStageStockRows).
; WAITS: no.
; Entry: X object. Exit: as the blend.
StagePickupExtraLife:
    lda #$9F
    sta a:OBJ_KIND,x
    lda #$04
    sta a:OBJ_MARKER_VALUE,x
    stz a:OBJ_STATE,x
    lda #$01
    sta z:zField90
    lda #$52
    jsr VecSoundCommand
    sed
    lda z:zStock
    clc
    adc #$01
    sta z:zStock
    cld
    bcc StagePickupExtraLife_at0
    lda #$99
    sta z:zStock
StagePickupExtraLife_at0:
    jmp BlendStageStockRows

; @routine 025B:0ED StageScrollTableVector
; @run 70:B0ED
; @stop B0F0
; WHAT IT DOES: Goes to StepVerticalScrollTable.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as the routine at $B0F0.
StageScrollTableVector:
    jmp StepVerticalScrollTable

; @routine 025C:092 StageDropStart
; @run 70:B892
; WHAT IT DOES: Starts the thing flying: the pickup ordered, list $83 on channel 5, no hitbox, its animation reset,
; speed 3 along X, the next substate.
; HOW: StageDropPickup, StartListChannelKind2 with X kept, stores.
; WAITS: no.
; Entry: X object. Exit: A 3, N and Z from the substate's INC, X kept.
StageDropStart:
    jsr StageDropPickup
    phx
    lda #$83
    ldx #$05
    jsr VecStartListChannelKind2
    plx
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$03
    sta a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_FLY_PHASE,x
    inc a:OBJ_SUBSTATE,x
    rts

; @routine 025C:0B6 StageDropPickup
; @run 70:B8B6
; WHAT IT DOES: Orders the pickup in slot $50: if one of kind $29 is there, order 2; else, unless this object has
; OBJ_SPAWN_HELD, a kind $29 started at this object's place with order 1.
; HOW: ClearObjectInY and StartObjectAtPlace with Y $50.
; WAITS: no.
; Entry: X object. Exit: Y $50; ordering: A the order, N and Z from it, C 1 when slot $50 held kind $29; held: A the
; hold.
StageDropPickup:
    ldy #$50
    lda a:OBJ_KIND+80
    cmp #$29
    beq StageDropPickup_at0
    lda a:OBJ_SPAWN_HELD,x
    bne StageDropPickup_at1
    jsr VecClearObjectInY
    lda #$29
    jsr VecStartObjectAtPlace
    lda #$01
    sta a:OBJ_CHILD_ORDER,y
StageDropPickup_at1:
    rts
StageDropPickup_at0:
    lda #$02
    sta a:OBJ_CHILD_ORDER,y
    rts

; @routine 025C:0D8 StageDropFly
; @run 70:B8D8
; WHAT IT DOES: The flying thing: its frames stepped, moved along X and up to the top, slowed by $18 a frame; once
; it flies back past X $E0 on the first screen: the sound $73, its animation reset, the next substate and a wait.
; HOW: StepObjectFrames on the list at $B921, ApplyVelocityX, StageObjectRiseToTop, the two-byte subtract.
; WAITS: no.
; Entry: X object. Exit: as the last compare, subtract or INC.
StageDropFly:
    lda #$21
    sta z:zTemp10
    lda #$B9
    sta z:zTemp11
    jsr VecStepObjectFrames
    jsr VecApplyVelocityX
    jsr StageObjectRiseToTop
    lda a:OBJ_VX_LO,x
    sec
    sbc #$18
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    sbc #$00
    sta a:OBJ_VX_HI,x
    bpl StageDropFly_at0
    lda a:OBJ_X_HI,x
    bne StageDropFly_at0
    lda a:OBJ_X_LO,x
    cmp #$E0
    bcs StageDropFly_at0
    lda #$73
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_FLY_PHASE,x
    lda #$B0
    sta a:OBJ_FLYER_WAIT,x
    inc a:OBJ_SUBSTATE,x
StageDropFly_at0:
    rts

; @routine 025C:4FD StageFlyerSlow
; @run 70:BCFD
; WHAT IT DOES: A flyer's frames stepped, its X speed less $30, and moved.
; HOW: StepObjectFrames on the list at $BB69, the subtract, the tail JMP to ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as ApplyVelocity.
StageFlyerSlow:
    lda #$69
    sta z:zTemp10
    lda #$BB
    sta z:zTemp11
    jsr VecStepObjectFrames
    lda a:OBJ_VX_LO,x
    sec
    sbc #$30
    sta a:OBJ_VX_LO,x
    lda a:OBJ_VX_HI,x
    sbc #$00
    sta a:OBJ_VX_HI,x
    jmp VecApplyVelocity

