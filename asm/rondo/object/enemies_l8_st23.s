; @routine 0F6B:488 CountDownThenRestartMusicAndScript
; @run 70:B488
; WHAT IT DOES: Counts OBJ_3510 down; at zero clears OBJ_34B8, sets OBJ_38D8 1, OBJ_35C0 and OBJ_3568 $E0 and OBJ_3510 $40, goes to the next state, clears bits 3 and 4 of zField8E, plays the track again (CdPlayAgain), sets bit 2 of zField8E and bit 1 of OBJ_3250, plays sound $D1 and starts script 1 in the only free script slot 3.
; HOW: A memory decrement, stores, RMWs of zField8E, CdPlayAgain, SoundCommand, StartSoleScriptSlot between a PHX and a PLX.
; WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
; Entry: X object, Y kept for the track and the sound. Exit: as the last instruction that ran.
CountDownThenRestartMusicAndScript:
    dec a:OBJ_3510,x
    bne CountDownThenRestartMusicAndScript_at0
    stz a:OBJ_34B8,x
    lda #$01
    sta a:OBJ_38D8,x
    lda #$E0
    sta a:OBJ_35C0,x
    sta a:OBJ_3568,x
    lda #$40
    sta a:OBJ_3510,x
    inc a:OBJ_STATE,x
    rmb3 z:zField8E
    rmb4 z:zField8E
    jsr VecCdPlayAgain
    lda z:zField8E
    ora #$04
    sta z:zField8E
    lda #$02
    ora a:OBJ_3250,x
    sta a:OBJ_3250,x
    lda #$D1
    jsr VecSoundCommand
    phx
    lda #$01
    ldx #$03
    jsr VecStartSoleScriptSlot
    plx
CountDownThenRestartMusicAndScript_at0:
    rts

; @routine 0F6B:776 ThrownPieceThenScatterAndLeave
; @run 70:B776
; WHAT IT DOES: In state 0 loads the record, sets the speed X $0800 and Y -$0800 (moved once each) and OBJ_34B8 3. In state 1 it falls (ApplyVelocityY, ApplyGravity) and steps the list by OBJ_34B8 until it reaches Y $D0; there it stops, shrinks its hit box to 8 by 8, starts four objects of type $AC numbered 0 to 3 in free slots of $30 to $3F, plays sound $26, goes to the next state and restarts the animation. In later states, every fourth frame, it starts an object of type $9B in the first empty slot of $50 to $54 with a random speed, steps the list and clears itself when the list ends.
; HOW: LoadObjectRecord, ApplyVelocityX and Y, ApplyGravity, FindFreeSlotFrom30, StartObjectAtPlace, SoundCommand, NextRandom, StepListByNumberTable, ClearObject.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
ThrownPieceThenScatterAndLeave:
    lda a:OBJ_STATE,x
    bne ThrownPieceThenScatterAndLeave_at0
    jsr VecLoadObjectRecord
    lda #$08
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    lda #$F8
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    stz a:OBJ_VY_HI,x
    lda #$03
    sta a:OBJ_34B8,x
    rts
ThrownPieceThenScatterAndLeave_at0:
    dec a
    bne ThrownPieceThenScatterAndLeave_at1
    jsr VecApplyVelocityY
    jsr VecApplyGravity
    lda a:OBJ_Y_LO,x
    cmp #$D0
    bcs ThrownPieceThenScatterAndLeave_at2
    jmp StepListByNumberTable
ThrownPieceThenScatterAndLeave_at2:
    lda #$D0
    sta a:OBJ_Y_LO,x
    lda #$08
    sta a:OBJ_HITBOX_HALF_W,x
    sta a:OBJ_HITBOX_HALF_H,x
    lda #$00
    sta z:zTemp00
ThrownPieceThenScatterAndLeave_at3:
    jsr VecFindFreeSlotFrom30
    bcc JumpByStateTable09EB7D3
    lda #$AC
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_START_ARG,y
    inc a
    cmp #$04
    beq JumpByStateTable09EB7D3
    sta z:zTemp00
    bra ThrownPieceThenScatterAndLeave_at3
    lda #$26
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
    inc a:OBJ_34B8,x
    jsr ClearAnimationThenStepListByNumber
    rts
ThrownPieceThenScatterAndLeave_at1:
    tst #$03,z:zWaveCounter
    bne ThrownPieceThenScatterAndLeave_at4
    ldy #$50
    lda a:OBJ_KIND,y
    bne ThrownPieceThenScatterAndLeave_at5
    jsr VecClearObjectInY
    lda #$9B
    jsr VecStartObjectAtPlace
    sxy
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$07
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
    jsr VecNextRandom
    and #$0F
    sec
    sbc #$0F
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocityY
    sxy
    bra ThrownPieceThenScatterAndLeave_at4
ThrownPieceThenScatterAndLeave_at5:
    iny
    cpy #$55
    bne RichterCheckItemHearts
ThrownPieceThenScatterAndLeave_at4:
    jsr StepListByNumberTable
    bne ThrownPieceThenScatterAndLeave_at6
    jmp VecClearObject
ThrownPieceThenScatterAndLeave_at6:
    rts
