; @routine 0250:29D StageGateAdvance
; @run 6B:9A9D
; WHAT IT DOES: Unless zField8C is negative, advances the object's state with $04 in its OBJ_WORK_D, twice for the
; start arguments whose low nibble is 2 to 5, $0A or $0B.
; HOW: CMP chain on OBJ_START_ARG's low nibble.
; WAITS: no.
; Entry: X object. Exit: negative: A zField8C; else A the nibble, N, Z and C from the last compare or INC.
StageGateAdvance:
    lda z:zField8C
    bmi StageGateAdvance_at0
    inc a:OBJ_STATE,x
    lda #$04
    sta a:OBJ_WORK_D,x
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
; WHAT IT DOES: Turns the object into the score marker (kind $9F) after blending the score added: its OBJ_WORK_D from
; the value in Y, its state cleared.
; HOW: PHY, BlendStageScoreAddRows by its $9181 C, PLA.
; WAITS: no.
; Entry: X object, Y the marker's value. Exit: A $9F, N and Z from it; X, Y, C and V as the blend left them.
StageObjectBecomeScore:
    phy
    jsr BlendStageScoreAddRows
    pla
    sta a:OBJ_WORK_D,x
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
    sta a:OBJ_WORK_D,x
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
; WHAT IT DOES: The flash's countdown: bit 1 of HitAttackFlags cleared; when the object's OBJ_WORK_D runs out the
; flash is over and the object cleared.
; HOW: AND and a store, DEC, the tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: A HitAttackFlags' new value; counting N and Z from the DEC, else as ClearObject.
StageFlashEnd:
    lda a:HitAttackFlags
    and #$FD
    sta a:HitAttackFlags
    dec a:OBJ_WORK_D,x
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

