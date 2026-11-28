; @routine 0EBD:44D MarkWhenPlayerNear0EBD
; @run 71:C44D
; WHAT IT DOES: While OBJ_WORK_G's bit 7 is clear, sets it to 1 once the player is within $30 dots along X (Stage 09's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call, compare or load; X kept.
MarkWhenPlayerNear0EBD:
    lda a:OBJ_WORK_G,x
    bmi MarkWhenPlayerNear0EBD_at0
    jsr VecDistanceXToPlayer
    cmp #$30
    bcs MarkWhenPlayerNear0EBD_at0
    lda #$01
    sta a:OBJ_WORK_G,x
MarkWhenPlayerNear0EBD_at0:
    rts

; @routine 0EBD:45F HitToState4Frame0EBD
; @run 71:C45F
; WHAT IT DOES: Under state 4, once hit: frame $05, no vertical speed, state 4, substate 0 and the hit box off.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
HitToState4Frame0EBD:
    lda a:OBJ_STATE,x
    cmp #$04
    bcs HitToState4Frame0EBD_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitToState4Frame0EBD_at0
    lda #$05
    sta a:OBJ_FRAME,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_HITBOX_ON,x
HitToState4Frame0EBD_at0:
    rts

; @routine 0EBF:0EB HitSoundAndSix0EBF
; @run 71:D0EB
; WHAT IT DOES: Once hit, not yet in state 6 (Stage 09's overlay): sound $C7, six pieces, the hit box off, the animation restarted,
; state 6.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A 6 or as the test; C from the state's compare; X kept.
HitSoundAndSix0EBF:
    lda a:OBJ_STATE,x
    cmp #$06
    beq HitSoundAndSix0EBF_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitSoundAndSix0EBF_at0
    lda #$C7
    jsr VecSoundCommand
    jsr StartSixAt0EBF
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
HitSoundAndSix0EBF_at0:
    rts

; @routine 0EBF:110 StartSixAt0EBF
; @run 71:D110
; WHAT IT DOES: Starts six kind $3E objects at this place in free slots from $30, each with its OBJ_WORK_D, OBJ_WORK_E and speed
; high bytes from the table at $D146 (Stage 09's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: X kept; as the last compare, or C clear with no free slot.
StartSixAt0EBFPieces = $D146 ; four bytes per piece: OBJ_WORK_D, OBJ_WORK_E, X and Y speed high bytes
StartSixAt0EBF:
    cly
StartSixAt0EBF_at1:
    sty z:zSpreadCount
    jsr VecFindFreeSlotFrom30
    bcc StartSixAt0EBF_at0
    lda #$3E
    jsr VecStartObjectAtPlace
    stx z:zSpreadSource
    lda z:zSpreadCount
    asl a
    asl a
    tax
    lda a:StartSixAt0EBFPieces,x
    sta a:OBJ_WORK_D,y
    lda a:StartSixAt0EBFPieces+1,x
    sta a:OBJ_WORK_E,y
    lda a:StartSixAt0EBFPieces+2,x
    sta a:OBJ_VX_HI,y
    lda a:StartSixAt0EBFPieces+3,x
    sta a:OBJ_VY_HI,y
    ldx z:zSpreadSource
    ldy z:zSpreadCount
    iny
    cpy #$06
    bcc StartSixAt0EBF_at1
StartSixAt0EBF_at0:
    rts

; @routine 101B:088 HitToState4_101B
; @run 70:B088
; WHAT IT DOES: Once hit, an object under state 4 goes to state 4 (Stage 0A's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
HitToState4_101B:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitToState4_101B_at0
    lda a:OBJ_STATE,x
    cmp #$04
    bcs HitToState4_101B_at0
    lda #$04
    sta a:OBJ_STATE,x
HitToState4_101B_at0:
    rts

; @routine 101B:46C HitToState11_101B
; @run 70:B46C
; WHAT IT DOES: Under state 11, once hit: state 11 and substate 0.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
HitToState11_101B:
    lda a:OBJ_STATE,x
    cmp #$0B
    bcs HitToState11_101B_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitToState11_101B_at0
    lda #$0B
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitToState11_101B_at0:
    rts

; @routine 101C:2AE HitSoundToState4_101C
; @run 70:BAAE
; WHAT IT DOES: Once hit (Stage 0A's overlay): sound $DD and state 4.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A 4 or 0; X kept.
HitSoundToState4_101C:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitSoundToState4_101C_at0
    lda #$DD
    jsr VecSoundCommand
    lda #$04
    sta a:OBJ_STATE,x
HitSoundToState4_101C_at0:
    rts

; @routine 101C:2C0 HitBoxSizeByFrame101C
; @run 70:BAC0
; WHAT IT DOES: Gives the object the hit box size of its frame, from $74 on (Stage 0A's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A the half height, N and Z from it, Y the size's offset, C from the shift, V from the subtract.
HitBoxSizeByFrame101CSizes = $BAD9 ; a size number by frame less $74
HitBoxSizeByFrame101CHalves = $BB01 ; half width and half height by size number
HitBoxSizeByFrame101C:
    lda a:OBJ_FRAME,x
    sec
    sbc #$74
    tay
    lda a:HitBoxSizeByFrame101CSizes,y
    asl a
    tay
    lda a:HitBoxSizeByFrame101CHalves,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:HitBoxSizeByFrame101CHalves+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    rts

; @routine 101C:790 CycleWhileUnderState2_101C
; @run 70:BF90
; WHAT IT DOES: The first of its kind (OBJ_START_ARG 0), from state 2 (Stage 0A's overlay): every 224 frames OBJ_WORK_M names the
; next of 14 slots from the table at $BFD2 (at once again when slot $11 plus it is not kind $36), else $FF.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load; X kept.
CycleWhileUnderState2_101COrder = $BFD2 ; the slot offsets in turn, 14
CycleWhileUnderState2_101C:
    lda a:OBJ_START_ARG,x
    bne CycleWhileUnderState2_101C_at0
    lda a:OBJ_STATE,x
    cmp #$02
    bcc CycleWhileUnderState2_101C_at0
    lda #$FF
    sta a:OBJ_WORK_M,x
    dec a:OBJ_WORK_I,x
    bne CycleWhileUnderState2_101C_at0
    lda #$E0
    sta a:OBJ_WORK_I,x
    ldy a:OBJ_WORK_L,x
    lda a:CycleWhileUnderState2_101COrder,y
    sta a:OBJ_WORK_M,x
    clc
    adc #$11
    tay
    lda a:OBJ_KIND,y
    cmp #$36
    beq CycleWhileUnderState2_101C_at1
    lda #$01
    sta a:OBJ_WORK_I,x
CycleWhileUnderState2_101C_at1:
    inc a:OBJ_WORK_L,x
    lda a:OBJ_WORK_L,x
    cmp #$0E
    bcc CycleWhileUnderState2_101C_at0
    stz a:OBJ_WORK_L,x
CycleWhileUnderState2_101C_at0:
    rts

; @routine 101C:7E0 HitBringDownSlots101C
; @run 70:BFE0
; WHAT IT DOES: Under state 4, once hit (Stage 0A's overlay): slot $11 plays sounds $DE and $FE and marks slots $12 to $1D hit
; (OBJ_HIT_TAKEN 5, OBJ_WORK_N 1); the object stops, its hit box off, state 4.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load; X kept.
HitBringDownSlots101C:
    lda a:OBJ_STATE,x
    cmp #$04
    bcs HitBringDownSlots101C_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitBringDownSlots101C_at0
    cpx #$11
    bne HitBringDownSlots101C_at1
    lda #$DE
    jsr VecSoundCommand
    lda #$FE
    jsr VecSoundCommand
    ldy #$12
HitBringDownSlots101C_at2:
    lda #$05
    sta a:OBJ_HIT_TAKEN,y
    lda #$01
    sta a:OBJ_WORK_N,y
    iny
    cpy #$1E
    bne HitBringDownSlots101C_at2
HitBringDownSlots101C_at1:
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitBringDownSlots101C_at0:
    rts

; @routine 101D:40F ToState4WhenPlayerClose101D
; @run 71:C40F
; WHAT IT DOES: Under state 4, with the player within 8 dots along X and $20 along Y (Stage 0A's overlay): sound $D7, the animation
; restarted, state 4.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
ToState4WhenPlayerClose101D:
    lda a:OBJ_STATE,x
    cmp #$04
    bcs ToState4WhenPlayerClose101D_at0
    jsr VecDistanceXToPlayer
    cmp #$08
    bcs ToState4WhenPlayerClose101D_at0
    jsr VecDistanceYToPlayer
    cmp #$20
    bcs ToState4WhenPlayerClose101D_at0
    lda #$D7
    jsr VecSoundCommand
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
ToState4WhenPlayerClose101D_at0:
    rts

; @routine 101D:438 HitToState5_101D
; @run 71:C438
; WHAT IT DOES: Under state 5, once hit (Stage 0A's overlay): the hit box off, OBJ_WORK_G $60, frame 1, state 5, substate 0.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
HitToState5_101D:
    lda a:OBJ_STATE,x
    cmp #$05
    bcs HitToState5_101D_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitToState5_101D_at0
    stz a:OBJ_HITBOX_ON,x
    lda #$60
    sta a:OBJ_WORK_G,x
    lda #$01
    sta a:OBJ_FRAME,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitToState5_101D_at0:
    rts

; @routine 101E:68F HitToState2Frame101E
; @run 71:CE8F
; WHAT IT DOES: Under state 2, once hit: frame $21, no vertical speed, state 2, substate 0 and the hit box off.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
HitToState2Frame101E:
    lda a:OBJ_STATE,x
    cmp #$02
    bcs HitToState2Frame101E_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitToState2Frame101E_at0
    lda #$21
    sta a:OBJ_FRAME,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$02
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_HITBOX_ON,x
HitToState2Frame101E_at0:
    rts

; @routine 12DB:43A HitToState4_12DB
; @run 70:B43A
; WHAT IT DOES: Under state 4, once hit: state 4 and substate 0.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
HitToState4_12DB:
    lda a:OBJ_STATE,x
    cmp #$04
    bcs HitToState4_12DB_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitToState4_12DB_at0
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitToState4_12DB_at0:
    rts

; @routine 12DD:37F DropAndShake12DD
; @run 71:C37F
; WHAT IT DOES: While OBJ_WORK_I counts, every fourth frame of OBJ_WORK_D a kind $36 object starts at its place; else a hit of 5
; or more under state 5 starts the count and halves OBJ_WORK_F (Stage 0C's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test, call or shift; X kept.
DropAndShake12DD:
    lda a:OBJ_WORK_I,x
    beq DropAndShake12DD_at0
    dec a:OBJ_WORK_I,x
    lda a:OBJ_WORK_D,x
    and #$03
    bne DropAndShake12DD_at1
    jsr VecFindFreeSlotFrom30
    bcc DropAndShake12DD_at1
    lda #$36
    jmp VecStartObjectAtPlace
DropAndShake12DD_at0:
    lda a:OBJ_STATE,x
    cmp #$05
    bcs DropAndShake12DD_at1
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc DropAndShake12DD_at1
    lda #$08
    sta a:OBJ_WORK_I,x
    lsr a:OBJ_WORK_F,x
DropAndShake12DD_at1:
    rts

; @routine 12DD:3AF HitStopAll12DD
; @run 71:C3AF
; WHAT IT DOES: Under state 5, once hit (Stage 0C's overlay): slot $11 cleared, the raster effect off, the object stopped
; with no hit box, its animation restarted, state 5.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A 5 or as the test; Y $11 when taken; X kept.
HitStopAll12DD:
    lda a:OBJ_STATE,x
    cmp #$05
    bcs HitStopAll12DD_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitStopAll12DD_at0
    ldy #$11
    jsr VecClearObjectInY
    stz z:zRasterEffect
    stz z:zRasterArg
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitStopAll12DD_at0:
    rts

; @routine 12DE:273 HalveWhenHit12DE
; @run 71:CA73
; WHAT IT DOES: Under state 5, a hit of 5 or more halves OBJ_WORK_F (Stage 0C's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last compare or the shift; X and Y kept.
HalveWhenHit12DE:
    lda a:OBJ_STATE,x
    cmp #$05
    bcs HalveWhenHit12DE_at0
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc HalveWhenHit12DE_at0
    lsr a:OBJ_WORK_F,x
HalveWhenHit12DE_at0:
    rts

; @routine 12DE:285 HitStopAll12DE
; @run 71:CA85
; WHAT IT DOES: Under state 5, once hit (Stage 0C's overlay): slot $11 cleared, the water line off, the object stopped
; with no hit box, its animation restarted, state 5.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A 5 or as the test; Y $11 when taken; X kept.
HitStopAll12DE:
    lda a:OBJ_STATE,x
    cmp #$05
    bcs HitStopAll12DE_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitStopAll12DE_at0
    ldy #$11
    jsr VecClearObjectInY
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:LevelYHi
    stz a:RasterWaterWanted
    lda #$05
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitStopAll12DE_at0:
    rts

; @routine 161B:341 HitSpawn92_161B
; @run 70:B341
; WHAT IT DOES: Under state 4, once hit (Stage 0E's overlay): the next slot cleared and started as kind $92 at this place, rising,
; not hit-tested for 22 frames; this object takes frame 9, state 4 and no hit box.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A 4 or as the test; Y the next slot; X kept.
HitSpawn92_161B:
    lda a:OBJ_STATE,x
    cmp #$04
    bcs HitSpawn92_161B_at0
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitSpawn92_161B_at0
    txa
    tay
    iny
    jsr VecClearObjectInY
    lda #$92
    jsr VecStartObjectAtPlace
    lda #$FC
    sta a:OBJ_VY_HI,y
    cla
    sta a:OBJ_VY_LO,y
    lda #$16
    sta a:OBJ_INVULN_TIMER,y
    lda #$09
    sta a:OBJ_FRAME,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    lda #$04
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_HITBOX_ON,x
HitSpawn92_161B_at0:
    rts

; @routine 161B:625 HitHold32_161B
; @run 70:B625
; WHAT IT DOES: Once hit (Stage 0E's overlay): the hit box off, OBJ_WORK_H 32, frame 0, state 3.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A 3 or 0; X, Y, C and V kept.
HitHold32_161B:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitHold32_161B_at0
    stz a:OBJ_HITBOX_ON,x
    lda #$20
    sta a:OBJ_WORK_H,x
    stz a:OBJ_FRAME,x
    lda #$03
    sta a:OBJ_STATE,x
HitHold32_161B_at0:
    rts

; @routine 161C:2EF HitToState7_161C
; @run 70:BAEF
; WHAT IT DOES: Once hit, under state 7 (Stage 0E's overlay): the sound OBJ_WORK_K names (table at $BB18) when on the first screen,
; the hit box off, the animation restarted, state 7.
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or load; X kept.
HitToState7_161CSounds = $BB18 ; the sound by OBJ_WORK_K
HitToState7_161C:
    lda a:OBJ_HIT_STATUS,x
    and #$80
    beq HitToState7_161C_at0
    lda a:OBJ_STATE,x
    cmp #$07
    bcs HitToState7_161C_at0
    ldy a:OBJ_WORK_K,x
    lda a:HitToState7_161CSounds,y
    jsr VecSoundIfOnFirstScreen
    stz a:OBJ_HITBOX_ON,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    lda #$07
    sta a:OBJ_STATE,x
    stz a:OBJ_SUBSTATE,x
HitToState7_161C_at0:
    rts

; @routine 18DB:6FE SpawnAtRandom18DB
; @run 70:B6FE
; WHAT IT DOES: One time in 32, starts a kind $32 object at this place in the first empty slot from $40 to $4F (Stage 10's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: A 0 and as the last call, or as the random's mask or the last compare; X kept.
SpawnAtRandom18DB:
    jsr VecNextRandom
    and #$1F
    bne SpawnAtRandom18DB_at0
    ldy #$40
SpawnAtRandom18DB_at2:
    lda a:OBJ_KIND,y
    beq SpawnAtRandom18DB_at1
    iny
    cpy #$50
    bcc SpawnAtRandom18DB_at2
SpawnAtRandom18DB_at0:
    rts
SpawnAtRandom18DB_at1:
    jsr VecClearObjectInY
    lda #$32
    jsr VecStartObjectAtPlace
    rts

; @routine 18DB:71B HealthByFirstHit18DB
; @run 70:B71B
; WHAT IT DOES: The first of slots $11 to $13 with a hit of 5 or more gives this object's OBJ_HURT_TIMER half the table's byte for
; it (Stage 10's overlay).
; HOW: The tests and stores in turn.
; WAITS: no.
; Entry: X object. Exit: as the last test or the shift; X kept.
HealthByFirstHit18DBHitTimes = $A27D ; a time by the hit
HealthByFirstHit18DB:
    ldy #$11
HealthByFirstHit18DB_at1:
    lda a:OBJ_HIT_TAKEN,y
    bne HealthByFirstHit18DB_at0
    iny
    cpy #$14
    bcc HealthByFirstHit18DB_at1
    rts
HealthByFirstHit18DB_at0:
    cmp #$05
    bcc HealthByFirstHit18DB_at2
    tay
    lda a:HealthByFirstHit18DBHitTimes,y
    lsr a
    sta a:OBJ_HURT_TIMER,x
HealthByFirstHit18DB_at2:
    rts
