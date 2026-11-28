; @routine 024E:353 SpawnStageObjectsRows01
; @run 6B:8B53
; WHAT IT DOES: Puts the stage's first two spawn rows in their slots, clears two stage state
; bytes (one to $C0), clears bit 5 of zField8E and starts list channel 2 with list $65.
; HOW: SpawnStageObject for row 0 and row 1, then the stores, then StartListChannelKind2.
; Eleven stage overlays hold this code, the ones whose spawn code is at $93B0.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 2, Y the second row's slot, N 0, Z 0. C and V as the spawn left them.
SpawnStageObjectsRows01:
    clx
    jsr SpawnStageObject
    inx
    jsr SpawnStageObject
    stz a:StageShownStock
    lda #$C0
    sta a:StageShownStock+1
    rmb5 z:zField8E
    lda #$65
    ldx #$02
    jmp VecStartListChannelKind2

; @routine 024E:798 SpawnStageObjectsRows34
; @run 6B:8F98
; WHAT IT DOES: Unless zField8E bit 0 is set, puts the stage's spawn rows 3 and 4 in their
; slots and sets the two objects up: their high position bytes back one, speed 4, hold $20,
; the player byte 2, and the first one's angle cleared.
; HOW: SpawnStageObject for row 3 and row 4, then the stores.
; Eleven stage overlays hold this code, the ones whose spawn code is at $93B0.
; WAITS: no.
; Entry: nothing. Exit: A 2, X 4, Y the second row's slot, N 0, Z 0, C and V as the spawn left
; them; with bit 0 set everything is kept.
SpawnStageObjectsRows34:
    bbr0 zField8E,SpawnStageObjectsRows34_at0
    rts
SpawnStageObjectsRows34_at0:
    ldx #$03
    jsr SpawnStageObject
    inx
    jsr SpawnStageObject
    dec a:OBJ_X_HI+84
    dec a:OBJ_Y_HI+85
    lda #$04
    sta a:OBJ_VX_HI+84
    sta a:OBJ_VY_HI+85
    lda #$20
    sta a:OBJ_FLING_WAIT+84
    sta a:OBJ_FLING_WAIT+85
    lda #$02
    sta a:TimeSlowMode
    stz a:OBJ_STATE_BEFORE_HIT+84
    rts

; @routine 05BE:353 SpawnStageObjectsRows01Long
; @run 6B:8B53
; WHAT IT DOES: SpawnStageObjectsRows01 for the other stage group, whose spawn code is at $96C2.
; HOW: The same code, calling SpawnStageObjectLong.
; WAITS: no.
; Entry, exit: as SpawnStageObjectsRows01.
SpawnStageObjectsRows01Long:
    clx
    jsr SpawnStageObjectLong
    inx
    jsr SpawnStageObjectLong
    stz a:StageVar3616
    lda #$C0
    sta a:StageVar3616+1
    rmb5 z:zField8E
    lda #$65
    ldx #$02
    jmp VecStartListChannelKind2

; @routine 05BE:798 SpawnStageObjectsRows34Long
; @run 6B:8F98
; WHAT IT DOES: SpawnStageObjectsRows34 for the other stage group, whose spawn code is at $96C2.
; HOW: The same code, calling SpawnStageObjectLong.
; WAITS: no.
; Entry, exit: as SpawnStageObjectsRows34.
SpawnStageObjectsRows34Long:
    bbr0 zField8E,SpawnStageObjectsRows34Long_at0
    rts
SpawnStageObjectsRows34Long_at0:
    ldx #$03
    jsr SpawnStageObjectLong
    inx
    jsr SpawnStageObjectLong
    dec a:$2BC4
    dec a:$2CCD
    lda #$04
    sta a:$2DD4
    sta a:$2E85
    lda #$20
    sta a:$3564
    sta a:$3565
    lda #$02
    sta a:TimeSlowMode
    stz a:$3614
    rts
