; @routine 0A8E:0FB ReloadStageEntryAndScroll0A8E
; @run 6B:88FB
; WHAT IT DOES: Reloads the stage's entry with the scroll's place kept: the screen and X low byte of the scroll
; into zBiosAl and zBiosAh, the scroll's tables zeroed, the player's sprite flip cleared, the entry loaded
; (zField8E kept across it), the bank mapped and the scroll set up; where the raster effect's table says so, the
; tables are zeroed and the entry loaded again with the effect's scroll; at the end the tables are zeroed and the
; entry loaded a last time (Stage 06's $8000 overlay; stages 07 and 08 hold the same bytes here).
; HOW: TAI from the kernel's zero bytes; the calls to the routines loaded at $8952, $80B7, $A94C and $A9B1, each to
; the one the overlay holds; a table read by zRasterEffect.
; WAITS: no.
; Entry: X as the caller set it. Exit: A, X, Y and the flags as its last instruction or call left them.
ReloadStageEntryAndScroll0A8ERasterReload = $88CE ; nonzero for the raster effects that load twice
ReloadStageEntryAndScroll0A8E:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    stz a:OBJ_SPRITE_FLIP
    lda z:zField8E
    pha
    bsr LoadStageEntryI
    pla
    sta z:zField8E
    jsr MapOverlayBank7
    jsr StageSetUpScroll
    ldy z:zRasterEffect
    lda a:ReloadStageEntryAndScroll0A8ERasterReload,y
    beq ReloadStageEntryAndScroll0A8E_at0
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    lda z:zField8E
    pha
    bsr LoadStageEntryI
    pla
    sta z:zField8E
    jsr StageSetUpScrollByEffect
ReloadStageEntryAndScroll0A8E_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088

; @routine 0A9A:0BC DebrisFlightStep0A9A
; @run 70:A8BC
; WHAT IT DOES: One frame of a debris piece's flight: sound $19 and a burst, then OBJ_WORK_D counts the flight's
; steps; until the count of its record (OBJ_WORK_E) it moves by the step's speeds; then it sets its OBJ_START_ARG's
; bit in zField8F, turns into the record's next kind and stops (Stage 06's overlay).
; HOW: SoundCommand, StartDebrisBurst, table reads by the record and the step, ApplyVelocity; BitMaskFromIndex.
; WAITS: no.
; Entry: X the object; Y as the caller left it (the sound command keeps it). Exit: A, X, Y and the flags as its
; last instruction or call left them.
DebrisFlightStep0A9ASteps = $B677 ; the number of steps of each record
DebrisFlightStep0A9AKinds = $B67C ; the kind each record turns into
DebrisFlightStep0A9ASpeeds = $B67D ; X and Y speed high bytes per step
DebrisFlightStep0A9A:
    lda #$19
    jsr VecSoundCommand
    lda #$01
    jsr StartDebrisBurst
    ldy a:OBJ_WORK_E,x
    inc a:OBJ_WORK_D,x
    lda a:OBJ_WORK_D,x
    cmp a:DebrisFlightStep0A9ASteps,y
    bcs DebrisFlightStep0A9A_at0
    lda a:OBJ_WORK_D,x
    dec a
    asl a
    clc
    adc a:OBJ_WORK_E,x
    tay
    lda a:DebrisFlightStep0A9ASpeeds,y
    sta a:OBJ_VX_HI,x
    lda a:DebrisFlightStep0A9ASpeeds+1,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    rts
DebrisFlightStep0A9A_at0:
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    ora z:zField8F
    sta z:zField8F
    ldy a:OBJ_WORK_E,x
    lda a:DebrisFlightStep0A9AKinds,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 0A9E:125 LoadWaitAndClearWork0A9E
; @run 71:C925
; WHAT IT DOES: Loads the object's record, sets a wait of $50 in OBJ_WORK_D and clears OBJ_WORK_E and OBJ_WORK_F
; (Stage 06's overlay).
; HOW: LoadObjectRecord, three stores.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
LoadWaitAndClearWork0A9E:
    jsr VecLoadObjectRecord
    lda #$50
    sta a:OBJ_WORK_D,x
    stz a:OBJ_WORK_E,x
    stz a:OBJ_WORK_F,x
    rts

; @routine 0BFA:0BC DebrisFlightStep0BFA
; @run 70:A8BC
; WHAT IT DOES: One frame of a debris piece's flight: sound $19 and a burst, then OBJ_WORK_D counts the flight's
; steps; until the count of its record (OBJ_WORK_E) it moves by the step's speeds; then it sets its OBJ_START_ARG's
; bit in zField8F, turns into the record's next kind and stops (Stage 07's overlay).
; HOW: SoundCommand, StartDebrisBurst, table reads by the record and the step, ApplyVelocity; BitMaskFromIndex.
; WAITS: no.
; Entry: X the object; Y as the caller left it (the sound command keeps it). Exit: A, X, Y and the flags as its
; last instruction or call left them.
DebrisFlightStep0BFASteps = $B036 ; the number of steps of each record
DebrisFlightStep0BFAKinds = $B03B ; the kind each record turns into
DebrisFlightStep0BFASpeeds = $B03C ; X and Y speed high bytes per step
DebrisFlightStep0BFA:
    lda #$19
    jsr VecSoundCommand
    lda #$01
    jsr StartDebrisBurst
    ldy a:OBJ_WORK_E,x
    inc a:OBJ_WORK_D,x
    lda a:OBJ_WORK_D,x
    cmp a:DebrisFlightStep0BFASteps,y
    bcs DebrisFlightStep0BFA_at0
    lda a:OBJ_WORK_D,x
    dec a
    asl a
    clc
    adc a:OBJ_WORK_E,x
    tay
    lda a:DebrisFlightStep0BFASpeeds,y
    sta a:OBJ_VX_HI,x
    lda a:DebrisFlightStep0BFASpeeds+1,y
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    rts
DebrisFlightStep0BFA_at0:
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    ora z:zField8F
    sta z:zField8F
    ldy a:OBJ_WORK_E,x
    lda a:DebrisFlightStep0BFAKinds,y
    sta a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 0D5C:113 FollowYWithOffset0D5C
; @run 70:B913
; WHAT IT DOES: Takes the slot OBJ_WORK_D plus OBJ_WORK_E names into Y, then follows it as FollowYAndCount0D5C does
; (Stage 08's overlay).
; HOW: An add into Y; it runs into FollowYAndCount0D5C.
; WAITS: no.
; Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
FollowYWithOffset0D5C:
    lda a:OBJ_WORK_D,x
    clc
    adc a:OBJ_WORK_E,x
    tay
