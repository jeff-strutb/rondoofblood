; @routine 0250:1BB StageGateByState
; @run 6B:99BB
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $99CD.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
StageGateByStateHandlers = $99CD ; a handler per state
StageGateByState:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:StageGateByStateHandlers,y
    sta z:zTemp00
    lda a:StageGateByStateHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 025A:6FC ByState025AAEFC
; @run 70:AEFC
; WHAT IT DOES: Goes to the state's handler.
; HOW: A table of handlers at $AF0E.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ByState025AAEFCHandlers = $AF0E ; a handler per state
ByState025AAEFC:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ByState025AAEFCHandlers,y
    sta z:zTemp00
    lda a:ByState025AAEFCHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 025B:36F FlyInAndAnnounce025B
; @run 70:B36F
; WHAT IT DOES: Pieces flying in to their places with the fade held: after the flight, number 4 starts script $82, then $81..$83 in turn by state, and the last wait sets sub-step 3 and zField8C $83.
; HOW: Tables by the argument; the holds drop out of this routine ($B441, $B446) until their counts end.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FlyInX = $B44E ; X low by argument
FlyInY = $B453 ; Y low by argument
FlyInXHi = $B458 ; X high by argument
FlyInYHi = $B45D ; Y high by argument
FlyInVx = $B462 ; X speed by argument
FlyInVy = $B467 ; Y speed by argument
FlyInFrames = $B46C ; frames of flight by argument
FlyInAndAnnounce025B:
    jsr SetBossFixedFields
    lda #$01
    sta z:zFadeActive
    lda a:OBJ_STATE,x
    bne FlyInAndAnnounce025B_at0
    jsr VecLoadObjectRecord
    ldy a:OBJ_START_ARG,x
    tya
    clc
    adc #$1E
    sta a:OBJ_FRAME,x
    lda a:FlyInX,y
    sta a:OBJ_X_LO,x
    lda a:FlyInXHi,y
    sta a:OBJ_X_HI,x
    lda a:FlyInY,y
    sta a:OBJ_Y_LO,x
    lda a:FlyInYHi,y
    sta a:OBJ_Y_HI,x
    lda a:FlyInVx,y
    sta a:OBJ_VX_HI,x
    lda a:FlyInVy,y
    sta a:OBJ_VY_HI,x
    lda a:FlyInFrames,y
    sta a:OBJ_PIECE_NEXT,x
    rts
FlyInAndAnnounce025B_at0:
    dec a
    bne FlyInAndAnnounce025B_at3
    jsr VecApplyVelocity
    dec a:OBJ_PIECE_NEXT,x
    bne FlyInAndAnnounce025B_at2
    lda a:OBJ_START_ARG,x
    cmp #$04
    bne FlyInAndAnnounce025B_at1
    phx
    ldx #$05
    lda #$82
    jsr VecStartScriptSlotKind1
    plx
FlyInAndAnnounce025B_at1:
    inc a:OBJ_STATE,x
    lda #$05
    sta a:OBJ_PIECE_PLACE,x
FlyInAndAnnounce025B_at2:
    rts
FlyInAndAnnounce025B_at3:
    dec a
    bne FlyInAndAnnounce025B_at5
    lda a:OBJ_START_ARG,x
    cmp #$04
    bne FlyInAndAnnounce025B_at2
    lda a:BiosJoyTrigger
    and #$08
    bne FlyInAndAnnounce025B_at4
    jsr HoldByWorkD025B
    jsr HoldByWorkE025B
FlyInAndAnnounce025B_at4:
    lda #$0C
    jsr BiosCdFade
    phx
    lda #$81
    ldx #$05
    jsr VecStartScriptSlotKind2
    plx
    lda #$50
    sta a:OBJ_PIECE_NEXT,x
    inc a:OBJ_STATE,x
    rts
FlyInAndAnnounce025B_at5:
    dec a
    bne FlyInAndAnnounce025B_at6
    jsr HoldByWorkD025B
    phx
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind2
    plx
    inc a:OBJ_STATE,x
    lda #$80
    sta a:OBJ_PIECE_NEXT,x
    rts
FlyInAndAnnounce025B_at6:
    dec a
    bne FlyInAndAnnounce025B_at7
    jsr HoldByWorkD025B
    phx
    lda #$83
    ldx #$05
    jsr VecStartScriptSlotKind2
    plx
    inc a:OBJ_STATE,x
    lda #$80
    sta a:OBJ_PIECE_NEXT,x
    rts
FlyInAndAnnounce025B_at7:
    jsr HoldByWorkD025B
    lda #$03
    sta z:zSubStep
    lda #$83
    sta z:zField8C
    rts

; @routine 025B:441 HoldByWorkE025B
; @run 70:B441
; WHAT IT DOES: As HoldByWorkD025B on OBJ_PIECE_PLACE.
; HOW: A DEC, then a branch into HoldByWorkD025B's test.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HoldByWorkE025B:
    dec a:OBJ_PIECE_PLACE,x
    bra HoldByWorkD025B_at0

; @routine 025B:446 HoldByWorkD025B
; @run 70:B446
; WHAT IT DOES: Counts the object's hold down; while it lasts the caller is not returned to: the return goes to the caller's caller.
; HOW: DEC OBJ_PIECE_NEXT; not zero, two PLA before the RTS.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
HoldByWorkD025B:
    dec a:OBJ_PIECE_NEXT,x
HoldByWorkD025B_at0:
    beq HoldByWorkD025B_at1
    pla
    pla
HoldByWorkD025B_at1:
    rts

; @routine 025C:2C9 SoundWhenHurtThenByStateBADE
; @run 70:BAC9
; WHAT IT DOES: Plays the hurt sound in states 1 to 3, then goes to the state's handler.
; HOW: SoundWhenHurtInStatesOneToThree, then a table at $BADE.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
SoundWhenHurtThenByStateBADEHandlers = $BADE ; a handler per state
SoundWhenHurtThenByStateBADE:
    jsr SoundWhenHurtInStatesOneToThree
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SoundWhenHurtThenByStateBADEHandlers,y
    sta z:zTemp00
    lda a:SoundWhenHurtThenByStateBADEHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 025C:6B9 ScriptByWaveThenByStateBECE
; @run 70:BEB9
; WHAT IT DOES: Starts the wave bit's script, then goes to the state's handler.
; HOW: StartScriptByWaveBit, then a table at $BECE.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
ScriptByWaveThenByStateBECEHandlers = $BECE ; a handler per state
ScriptByWaveThenByStateBECE:
    jsr StartScriptByWaveBit
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:ScriptByWaveThenByStateBECEHandlers,y
    sta z:zTemp00
    lda a:ScriptByWaveThenByStateBECEHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)
