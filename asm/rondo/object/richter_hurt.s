; @routine 01BA:6F3 RichterReplaceSlotsOneTwo
; @run 6D:AEF3
; WHAT IT DOES: Gives object slots 1 and 2 kind 4, each emptied first, unless it holds kind 1 or 2.
; HOW: OBJ_KIND compared with 1 and 2; RichterClearSlotKindFour for slot 1, then a run on into it for
; slot 2. A kind 1 or 2 in slot 2 ends at RichterClearSlotKindFour's RTS.
; WAITS: no.
; Entry: nothing. Exit: X 2. Slot 2 kept: A its kind, N, Z and C from comparing it with 1 or 2; else
; A 4, N and Z from it, C from comparing the kind with 2. Y and V kept.
RichterReplaceSlotsOneTwo:
    ldx #$01
    lda a:OBJ_KIND,x
    cmp #$01
    beq RichterReplaceSlotsOneTwo_at0
    cmp #$02
    beq RichterReplaceSlotsOneTwo_at0
    bsr RichterClearSlotKindFour
RichterReplaceSlotsOneTwo_at0:
    inx
    lda a:OBJ_KIND,x
    cmp #$01
    beq RichterClearSlotKindFour_at0
    cmp #$02
    beq RichterClearSlotKindFour_at0

; @routine 01BA:70E RichterClearSlotKindFour
; @run 6D:AF0E
; WHAT IT DOES: Empties an object slot and gives it kind 4.
; HOW: ClearObject (through its kernel vector), then 4 into OBJ_KIND.
; WAITS: no.
; Entry: X object. Exit: A 4, N and Z from it. X, Y, C and V kept.
RichterClearSlotKindFour:
    jsr VecClearObject
    lda #$04
    sta a:OBJ_KIND,x
RichterClearSlotKindFour_at0:
    rts

; @routine 01BB:383 RichterHitSoundAndEffects
; @run 6D:B383
; WHAT IT DOES: Sound $62 for a hit with neither bit 5 nor bit 6 of OBJ_HIT_TAKEN, then
; RichterHitEffects.
; HOW: A TST of OBJ_HIT_TAKEN with $60; SoundCommand through its kernel vector; runs on into
; RichterHitEffects.
; WAITS: no.
; Entry: X and Y for the sound driver. Exit: as RichterHitEffects, from what SoundCommand left when
; it ran.
RichterHitSoundAndEffects:
    tst #$60,a:OBJ_HIT_TAKEN
    bne RichterHitEffects
    lda #$62
    jsr VecSoundCommand

; @routine 01BB:38E RichterHitEffects
; @run 6D:B38E
; WHAT IT DOES: What a hit does besides hurting, by bits 4 to 6 of OBJ_HIT_TAKEN, the first set of
; them in that order; then OBJ_HIT_TAKEN is cleared. Bit 4: slots 1 and 2 emptied as kind 1, slot
; 1's OBJ_ANIM_TIMER $0A, script $0A. Bit 5: sound $12, PlayerByte3BE2 counted up, script 1. Bit 6:
; sound $24, PlayerByte3BE8 $FF and PlayerByte3BE9 1, script $17.
; HOW: Three TSTs of OBJ_HIT_TAKEN; ClearObject and SoundCommand through their kernel vectors; a tail
; jump to RichterStartHitScript.
; WAITS: no.
; Entry: X and Y for the sound driver as SoundCommand takes them. Exit: no bit: OBJ_HIT_TAKEN 0, Z
; set, N bit 7 of OBJ_HIT_TAKEN as the last TST read it, V clear; A, X, Y and C kept. Otherwise as
; RichterStartHitScript with the script: C kept; V bit 6 of OBJ_HIT_TAKEN for bit 4, as SoundCommand
; left it for bits 5 and 6.
RichterHitEffects:
    tst #$10,a:OBJ_HIT_TAKEN
    bne RichterHitEffects_at0
    tst #$20,a:OBJ_HIT_TAKEN
    bne RichterHitEffects_at1
    tst #$40,a:OBJ_HIT_TAKEN
    bne RichterHitEffects_at2
    stz a:OBJ_HIT_TAKEN
    rts
RichterHitEffects_at0:
    ldx #$01
    jsr VecClearObject
    inx
    jsr VecClearObject
    lda #$01
    sta a:OBJ_KIND+1
    sta a:OBJ_KIND+2
    lda #$0A
    sta a:OBJ_ANIM_TIMER+1
    lda #$0A
RichterHitEffects_at3:
    stz a:OBJ_HIT_TAKEN
    jmp RichterStartHitScript
RichterHitEffects_at1:
    lda #$12
    jsr VecSoundCommand
    inc a:PlayerByte3BE2
    lda #$01
    bra RichterHitEffects_at3
RichterHitEffects_at2:
    lda #$24
    jsr VecSoundCommand
    lda #$FF
    sta a:PlayerByte3BE8
    lda #$01
    sta a:PlayerByte3BE9
    lda #$17
    bra RichterHitEffects_at3

; @routine 01BB:52C RichterHurtLandSound
; @run 6D:B52C
; WHAT IT DOES: Slots 1 and 2 to kind 4, then sound $4A.
; HOW: RichterReplaceSlotsOneTwo, then a tail jump to SoundCommand (through its kernel vector).
; WAITS: no.
; Entry: X and Y for the sound driver as SoundCommand takes them (X is 2 by then). Exit: as SoundCommand
; with $4A, from what RichterReplaceSlotsOneTwo left.
RichterHurtLandSound:
    jsr RichterReplaceSlotsOneTwo
    lda #$4A
    jmp VecSoundCommand

; @routine 01BB:562 RichterKnockSpeedXFromHit
; @run 6D:B562
; WHAT IT DOES: RichterKnockSpeedX for the side the hit throws him to, OBJ_KNOCK_RIGHT.
; HOW: Runs on into RichterKnockSpeedX.
; WAITS: no.
; Entry: nothing. Exit: as RichterKnockSpeedX.
RichterKnockSpeedXFromHit:
    lda a:OBJ_KNOCK_RIGHT

; @routine 01BB:565 RichterKnockSpeedX
; @run 6D:B565
; WHAT IT DOES: Gives him one pixel a frame of X speed: left for 0, right for 1.
; HOW: Two words at $B5AA, $FF00 and $0100, by twice A.
; WAITS: no.
; Entry: A 0 or 1. Exit: A the speed's high byte, N and Z from it; Y twice the entry; C bit 7 of
; the entry. X and V kept.
RichterKnockSpeedX:
    asl a
    tay
    lda a:RichterKnockSpeedsX,y
    sta a:OBJ_VX_LO
    lda a:RichterKnockSpeedsX+1,y
    sta a:OBJ_VX_HI
    rts

; @routine 01BB:592 RichterHurtToCrouch
; @run 6D:B592
; WHAT IT DOES: Ends a reaction in a crouch: the crouched frame, the animation stopped, no speed,
; state 2 at step 0.
; HOW: OBJ_FRAME $0E, OBJ_HURT_TIMER $FF, RichterStopMoving, OBJ_STATE and OBJ_SUBSTATE.
; WAITS: no.
; Entry: nothing. Exit: A 0, N clear, Z set. X, Y, C and V kept.
RichterHurtToCrouch:
    lda #$0E
    sta a:OBJ_FRAME
    lda #$FF
    sta a:OBJ_HURT_TIMER
    jsr RichterStopMoving
    lda #$02
    sta a:OBJ_STATE
    lda #$00
    sta a:OBJ_SUBSTATE
    rts

; @routine 01BB:5AE RichterHurtResume
; @run 6D:B5AE
; WHAT IT DOES: The reaction's step 3: shows the hurt frame for the state he was hit in, and when
; OBJ_HIT_FRAMES runs out gives back that state and its step; with step 1 given back it ends as
; RichterEndItemCrash.
; HOW: The frame from the two bytes at $B5D5 by (the state less 5) / 2; a tail jump to
; RichterEndItemCrash.
; WAITS: no.
; Entry: nothing. Exit: X the frame's index; V from the subtract. Still counting: A the frame, N and Z
; from the count, C bit 0 of the state less 5. Given back: A the step, N, Z and C from comparing it
; with 1; step 1, then as RichterEndItemCrash. Y kept but there.
RichterHurtResume:
    lda a:OBJ_STATE_BEFORE_HIT
    sec
    sbc #$05
    lsr a
    tax
    lda a:RichterHurtResumeFrames,x
    sta a:OBJ_FRAME
    dec a:OBJ_HIT_FRAMES
    bne RichterHurtResume_at0
    lda a:OBJ_STATE_BEFORE_HIT
    sta a:OBJ_STATE
    lda a:OBJ_STEP_BEFORE_HIT
    sta a:OBJ_SUBSTATE
    cmp #$01
    bne RichterHurtResume_at0
    jmp RichterEndItemCrash
RichterHurtResume_at0:
    rts
