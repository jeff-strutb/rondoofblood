; @routine 1018:4D7 PlaySound58A
; @run 6F:BCD7
; WHAT IT DOES: Plays sound $58.
; HOW: A load and a tail jump into SoundCommand.
; WAITS: no.
; Entry: Y kept for the sound. Exit: as SoundCommand.
PlaySound58A:
    lda #$58
    jmp VecSoundCommand

; @routine 1019:2C5 MaybeDropItemByPlayerDistance1019
; @run 70:A2C5
; WHAT IT DOES: MaybeDropItemByPlayerDistance with this overlay's tables at $AC4C and $AC54.
; HOW: drop_item_with_tables.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The drop arm ran in no script; the draft stops there.
MaybeDropItemByPlayerDistance1019:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq RichterStandStep1
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:$AC4C,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$AC54,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 101B:426 SettleWhenLowOnFirstRow
; @run 70:B426
; WHAT IT DOES: When OBJ_3670 and the Y high byte are both 0 and the Y low byte is below $D0, sets OBJ_3670 1 and clears the flash flags.
; HOW: An ORA, a compare, an INC, a store.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
SettleWhenLowOnFirstRow:
    lda a:OBJ_3670,x
    ora a:OBJ_Y_HI,x
    bne SettleWhenLowOnFirstRow_at0
    lda a:OBJ_Y_LO,x
    cmp #$D0
    bcs SettleWhenLowOnFirstRow_at0
    inc a:OBJ_3670,x
    stz a:OBJ_2B18,x
SettleWhenLowOnFirstRow_at0:
    rts

; @routine 101F:199 LoadRecordFrameFromKind
; @run 71:D199
; WHAT IT DOES: In state 0 loads the record and takes OBJ_3040 as its frame.
; HOW: LoadObjectRecord, a copy.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordFrameFromKind:
    lda a:OBJ_27A8,x
    bne LoadRecordFrameFromKind_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_3040,x
    sta a:OBJ_2A68,x
LoadRecordFrameFromKind_at0:
    rts

; @routine 1178:67A PlaySound58B
; @run 6F:BE7A
; WHAT IT DOES: Plays sound $58 (a copy of PlaySound58A for this overlay).
; HOW: A load and a tail jump into SoundCommand.
; WAITS: no.
; Entry: Y kept for the sound. Exit: as SoundCommand.
PlaySound58B:
    lda #$58
    jmp VecSoundCommand

; @routine 161B:112 CapPlayerXOnScreen1
; @run 70:B112
; WHAT IT DOES: On the scroll's screen 1 keeps slot 0's OBJ_X_LO at $36 or more.
; HOW: Two compares and a store.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The arm past the first compare ran in no script; the draft stops there.
CapPlayerXOnScreen1:
    lda a:ScrollXHiScreen
    cmp #$01
    bne CapPlayerXOnScreen1_at0
    lda a:OBJ_X_LO
    cmp #$36
    bcs CapPlayerXOnScreen1_at0
    lda #$36
    sta a:OBJ_X_LO
CapPlayerXOnScreen1_at0:
    rts

; @routine 161E:617 LoadRecordThenDriftAndCount
; @run 71:CE17
; WHAT IT DOES: In state 0 loads the record, sets sprite set 6, frame $45, OBJ_3510 $10 and the speeds' low bytes 1. In state 1 it moves, counts OBJ_34B8 down through $FF and then OBJ_3510; when that goes below 0 it goes to the next state.
; HOW: LoadObjectRecord, ApplyVelocity, memory decrements, a compare, an INC.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordThenDriftAndCount:
    lda a:OBJ_27A8,x
    bne LoadRecordThenDriftAndCount_at0
    jsr VecLoadObjectRecord
    lda #$06
    sta a:OBJ_2A10,x
    lda #$45
    sta a:OBJ_2A68,x
    lda #$10
    sta a:OBJ_3510,x
    lda #$01
    sta a:OBJ_VX_LO,x
    sta a:OBJ_VY_LO,x
LoadRecordThenDriftAndCount_at1:
    rts
LoadRecordThenDriftAndCount_at0:
    dec a
    bne LoadRecordThenDriftAndCount_at1
    jsr VecApplyVelocity
    dec a:OBJ_34B8,x
    lda a:OBJ_34B8,x
    cmp #$FF
    bne LoadRecordThenDriftAndCount_at1
    dec a:OBJ_3510,x
    bpl LoadRecordThenDriftAndCount_at1
    inc a:OBJ_27A8,x
    rts

; @routine 18DD:260 LoadRecordAtX80
; @run 71:C260
; WHAT IT DOES: In state 0 loads the record and places the object at X $0080.
; HOW: LoadObjectRecord and two stores.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
LoadRecordAtX80:
    lda a:OBJ_27A8,x
    bne LoadRecordAtX80_at0
    jsr VecLoadObjectRecord
    lda #$80
    sta a:OBJ_X_LO,x
    stz a:OBJ_X_HI,x
LoadRecordAtX80_at0:
    rts
