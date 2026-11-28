; @routine 01DF:6AE MariaStateItem4
; @run 6D:B6AE
; WHAT IT DOES: State $10, item 4: slot 5's hitbox off, then the step's handler from MariaItem4Steps (start, walk,
; air).
; HOW: STZ, MariaStepIndex, the JMP (table,X).
; WAITS: no.
; Entry: Y the caller's. Exit: as the step's handler, after A and X the step doubled and C its bit 7.
MariaStateItem4:
    stz a:OBJ_HITBOX_ON+5
    jsr MariaStepIndex
    jmp (MariaItem4Steps,x)

; @routine 01DF:6BD MariaItem4Start
; @run 6D:B6BD
; WHAT IT DOES: Item 4's step 0: the columns, a landing; while her animation runs, nothing more; then its tiles copied,
; four words a frame; when they are done, on a floor her column stands on or with a mark under her, she lands
; (MariaItem4Landed); else standing still, in the air.
; HOW: MariaReadColumns, MariaLandOnFloor, MariaAnimationWait (which ends the step while it runs); MariaItem4CopyTiles's
; Z; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn, the columns' bits 2 and 5; MariaItem4Still and the JMP to
; MariaItem4ToAir.
; WAITS: no.
; Entry: nothing. Exit: as the routine it ended in.
MariaItem4Start:
    jsr MariaReadColumns
    jsr MariaLandOnFloor
    jsr MariaAnimationWait
    jsr MariaItem4CopyTiles
    beq MariaItem4Start_at0
    rts
MariaItem4Start_at0:
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaItem4Start_at1
    jsr MariaSolidUnderColumn
    beq MariaItem4Landed
MariaItem4Start_at1:
    lda a:RichterColumnHit
    ora a:RichterColumnHit+1
    ora a:RichterColumnHit+2
    and #$24
    bne MariaItem4Landed
    jsr MariaItem4Still
    jmp MariaItem4ToAir

; @routine 01DF:6EC MariaItem4Landed
; @run 6D:B6EC
; WHAT IT DOES: Item 4 on the ground: animation $28, standing still, step 1.
; HOW: MariaSetAnimation, MariaItem4Still, the store.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z from it; X the other side; Y and C as MariaSetAnimation left them.
MariaItem4Landed:
    lda #$28
    jsr MariaSetAnimation
    jsr MariaItem4Still
    lda #$01
    sta a:OBJ_SUBSTATE
    rts

; @routine 01DF:6FA MariaItem4CopyTiles
; @run 6D:B6FA
; WHAT IT DOES: Copies four VRAM words of item 4's tiles, $10 apart, from $6Fxx to $6Cxx at the row OBJ_WORK_D gives
; (from $10 on, $40 more and $10 less), and counts OBJ_WORK_D on; Z set once it reaches $20.
; HOW: zTemp00, zTemp01 and zTemp03 set up; a BSR to MariaCopyVramWord four times; INC of OBJ_WORK_D, CMP #$20.
; WAITS: no.
; Entry: nothing. Exit: A OBJ_WORK_D, N, Z and C from comparing it with $20; Y 4. X kept, V from the subtract when it
; was made, else kept.
MariaItem4CopyTiles:
    lda a:OBJ_FLASH_FRAMES
    cmp #$10
    bcc MariaItem4CopyTiles_at0
    sec
    sbc #$10
    ora #$40
MariaItem4CopyTiles_at0:
    sta z:zTemp00
    lda #$6F
    sta z:zTemp01
    lda #$6C
    sta z:zTemp03
    cly
MariaItem4CopyTiles_to1:
    bsr MariaCopyVramWord
    lda z:zTemp00
    clc
    adc #$10
    sta z:zTemp00
    iny
    cpy #$04
    bne MariaItem4CopyTiles_to1
    inc a:OBJ_FLASH_FRAMES
    lda a:OBJ_FLASH_FRAMES
    cmp #$20
    rts

; @routine 01DF:728 MariaCopyVramWord
; @run 6D:B728
; WHAT IT DOES: Copies one VRAM word: read from zTemp01:zTemp00, written to zTemp03:zTemp00.
; HOW: The VDC's read address, then its write address, from the data ports; the data selected; the word read from the
; data ports and written back to them, low byte first.
; WAITS: no.
; Entry: zTemp00 the low byte of both addresses, zTemp01 the read's high byte, zTemp03 the write's. Exit: A the high
; byte copied, N and Z from it. X, Y, C and V as SelectVramData left them (kept).
MariaCopyVramWord:
    jsr VecSelectVramReadAddress
    lda z:zTemp00
    sta a:VDC_DATA_LO
    lda z:zTemp01
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    lda z:zTemp00
    sta a:VDC_DATA_LO
    lda z:zTemp03
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    rts

; @routine 01DF:752 MariaItem4Jump
; @run 6D:B752
; WHAT IT DOES: Item 4's jump: lifted three rows, Y speed $FE80, then in the air.
; HOW: MariaLiftThree, the stores, on into MariaItem4ToAir.
; WAITS: no.
; Entry: nothing. Exit: as MariaItem4ToAir; C and V as MariaLiftThree left them.
MariaItem4Jump:
    jsr MariaLiftThree
    lda #$80
    sta a:OBJ_VY_LO
    lda #$FE
    sta a:OBJ_VY_HI

; @routine 01DF:75F MariaItem4ToAir
; @run 6D:B75F
; WHAT IT DOES: Item 4 in the air: the pad's third and fourth overrides cleared, step 2.
; HOW: The stores.
; WAITS: no.
; Entry: nothing. Exit: A 2, N and Z from it. X, Y, C and V kept.
MariaItem4ToAir:
    stz a:PadOverrides+2
    stz a:PadOverrides+3
    lda #$02
    sta a:OBJ_SUBSTATE
    rts

; @routine 01DF:76B MariaItem4Walk
; @run 6D:B76B
; WHAT IT DOES: Item 4's walk (step 1): no X speed; called off, it ends; the columns and her footing (a row lower too,
; else in the air); on a mark the pad's third and fourth overrides follow whether she is low on the screen ($B0); the
; jump; a landing; then right or left held and not blocked a pixel that way, she walks ($0050 or $FFB0) with her
; animation; else she stands still.
; HOW: MariaStopX, MariaItem4Cancel and the JMP to MariaItem4End; MariaReadColumns, MariaHasNoFooting, the columns' bits;
; MariaJumpPressed and the JMP to MariaItem4Jump; MariaLandOnFloor; BBR5 and BBR7 of zPadHeld; MariaItem4Blocked under
; PHP and PLP; MariaAnimationTick, MariaItem4WalkSpeeds.
; WAITS: no.
; Entry: Y the caller's. Exit: as the routine it ended in, or the store of the other side's override.
MariaItem4Walk:
    jsr MariaStopX
    jsr MariaItem4Cancel
    bcc MariaItem4Walk_to1
    jmp MariaItem4End
MariaItem4Walk_to1:
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcc MariaItem4Walk_at0
    lda a:RichterColumnHit
    ora a:RichterColumnHit+1
    ora a:RichterColumnHit+2
    and #$24
    bne MariaItem4Walk_at1
    inc a:OBJ_Y_LO
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcc MariaItem4Walk_at0
    jmp MariaItem4ToAir
MariaItem4Walk_at1:
    stz a:PadOverrides+2
    stz a:PadOverrides+3
    lda a:OBJ_Y_LO
    cmp #$B0
    bcc MariaItem4Walk_at0
    lda #$01
    sta a:PadOverrides+2
    sta a:PadOverrides+3
MariaItem4Walk_at0:
    jsr MariaJumpPressed
    bcc MariaItem4Walk_to2
    jmp MariaItem4Jump
MariaItem4Walk_to2:
    jsr MariaLandOnFloor
    bbr5 zPadHeld,MariaItem4Walk_to3
    stz a:OBJ_FACING_LEFT
    inc a:OBJ_X_LO
    bsr MariaItem4Blocked
    php
    dec a:OBJ_X_LO
    plp
    bne MariaItem4Walk_at2
    bra MariaItem4Walk_at3
MariaItem4Walk_to3:
    bbr7 zPadHeld,MariaItem4Still
    lda #$01
    sta a:OBJ_FACING_LEFT
    dec a:OBJ_X_LO
    bsr MariaItem4Blocked
    php
    inc a:OBJ_X_LO
    plp
    bne MariaItem4Walk_at2
MariaItem4Walk_at3:
    jsr MariaAnimationTick
    lda a:OBJ_FACING_LEFT
    asl a
    tay
    lda a:MariaItem4WalkSpeeds,y
    sta a:OBJ_VX_LO
    lda a:MariaItem4WalkSpeeds+1,y
    sta a:OBJ_VX_HI
    bra MariaItem4Still_to1
MariaItem4Walk_at2:
    jsr MariaStopX

; @routine 01DF:7FA MariaItem4Still
; @run 6D:B7FA
; WHAT IT DOES: Item 4 standing still: frame $42; then the pad override of the side she faces away from cleared.
; HOW: The store; OBJ_FACING_LEFT XOR 1 indexes PadOverrides' third byte on.
; WAITS: no.
; Entry: nothing. Exit: A the other side, X it, N and Z from it. Y, C and V kept.
MariaItem4Still:
    lda #$42
    sta a:OBJ_FRAME
MariaItem4Still_to1:
    lda a:OBJ_FACING_LEFT
    eor #$01
    tax
    stz a:PadOverrides+2,x
    rts

; @routine 01E0:00D MariaItem4Blocked
; @run 6D:B80D
; WHAT IT DOES: Whether item 4's walk is blocked at the low columns: with a mark under her (bits 2 or 5 of the three
; columns), any of bits 2 and 5 of MariaLowColumnsBlocked's answer, else its bit 5.
; HOW: MariaLowColumnsBlocked, the AND; then CPX #2 and CPX #0, whose branches both go to the RTS.
; WAITS: no.
; Entry: nothing. Exit: Z clear when blocked, with N, Z and C from the compares of X; A the AND. X, Y and V as
; MariaLowColumnsBlocked left them.
MariaItem4Blocked:
    lda a:RichterColumnHit
    ora a:RichterColumnHit+1
    ora a:RichterColumnHit+2
    and #$24
    beq MariaItem4Blocked_at0
    jsr MariaLowColumnsBlocked
    and #$24
    bra MariaItem4Blocked_at1
MariaItem4Blocked_at0:
    jsr MariaLowColumnsBlocked
    and #$20
MariaItem4Blocked_at1:
    beq MariaItem4Blocked_at2
    cpx #$02
    beq MariaItem4Blocked_at2
    cpx #$00
    beq MariaItem4Blocked_at2
MariaItem4Blocked_at2:
    rts

; @routine 01E0:031 MariaItem4Air
; @run 6D:B831
; WHAT IT DOES: Item 4 in the air (step 2): called off, it ends; falling, on a floor her column stands on or a column
; with bit 2 or 5 she lands: her Y up by the column's row, slots 1 and 2 as kind 5, sound $30 hard (Y speed $06xx or
; more) or $4A, no speed, and on the ground; else the bit-3 burst; gravity $0048 held at $07xx; at $06xx or more down,
; slot 5's hitbox is live.
; HOW: MariaItem4Cancel and the JMP to MariaItem4End; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn, TST of
; RichterColumnHit by X; MariaReplaceSlotsOneTwoKind5, SoundCommand, MariaStopMoving and the JMP to MariaItem4Landed;
; MariaCheckBit3Under; the 16-bit add.
; WAITS: no.
; Entry: Y the caller's. Exit: as the routine it ended in, or the last compare.
MariaItem4Air:
    jsr MariaItem4Cancel
    bcc MariaItem4Air_at0
    jmp MariaItem4End
MariaItem4Air_at0:
    lda a:OBJ_VY_HI
    bmi MariaItem4Air_at1
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaItem4Air_at2
    jsr MariaSolidUnderColumn
    beq MariaItem4Air_at3
MariaItem4Air_at2:
    clx
    tst #$24,a:RichterColumnHit,x
    bne MariaItem4Air_at3
    inx
    tst #$24,a:RichterColumnHit,x
    bne MariaItem4Air_at3
    inx
    tst #$24,a:RichterColumnHit,x
    bne MariaItem4Air_at3
    jsr MariaCheckBit3Under
MariaItem4Air_at1:
    lda a:OBJ_VY_LO
    clc
    adc #$48
    sta a:OBJ_VY_LO
    lda a:OBJ_VY_HI
    adc #$00
    sta a:OBJ_VY_HI
    bmi MariaItem4Air_at4
    cmp #$07
    bcc MariaItem4Air_at4
    stz a:OBJ_VY_LO
MariaItem4Air_at4:
    lda a:OBJ_VY_HI
    bmi MariaItem4Air_at5
    cmp #$06
    bcc MariaItem4Air_at5
    lda #$01
    sta a:OBJ_HITBOX_ON+5
MariaItem4Air_at5:
    rts
MariaItem4Air_at3:
    lda a:OBJ_Y_LO
    sec
    sbc a:RichterColumnRow,x
    sta a:OBJ_Y_LO
    jsr MariaReplaceSlotsOneTwoKind5
    lda a:OBJ_VY_HI
    cmp #$06
    bcc MariaItem4Air_at6
    lda #$30
    jsr VecSoundCommand
    bra MariaItem4Air_at7
MariaItem4Air_at6:
    lda #$4A
    jsr VecSoundCommand
MariaItem4Air_at7:
    jsr MariaStopMoving
    jmp MariaItem4Landed

; @routine 01E0:0C6 MariaItem4End
; @run 6D:B8C6
; WHAT IT DOES: Ends item 4: slot 5 as kind 9 (MariaClearSlot5AsKind9), the hit timer $30; in the air (step 2) she
; falls, else she jumps.
; HOW: A BSR to MariaClearSlot5AsKind9; the JMPs to MariaToFall and MariaJump.
; WAITS: no.
; Entry: Y the caller's (the sound is given it). Exit: as MariaToFall or MariaJump.
MariaItem4End:
    bsr MariaClearSlot5AsKind9
    lda #$30
    sta a:OBJ_ANIM_TIMER
    lda a:OBJ_SUBSTATE
    cmp #$02
    bne MariaItem4End_to1
    jmp MariaToFall
MariaItem4End_to1:
    jmp MariaJump

; @routine 01E0:0DA MariaItem4Cancel
; @run 6D:B8DA
; WHAT IT DOES: Whether item 4 is called off: button II pressed with up held.
; HOW: Bit 1 of zPadPressed, bit 4 of zPadHeld.
; WAITS: no.
; Entry: nothing. Exit: C the answer; A the last AND, N and Z from it. X, Y and V kept.
MariaItem4Cancel:
    lda z:zPadPressed
    and #$02
    beq MariaItem4Cancel_at0
    lda z:zPadHeld
    and #$10
    beq MariaItem4Cancel_at0
    sec
    rts
MariaItem4Cancel_at0:
    clc
    rts

; @routine 01E0:0EA MariaStateSequence
; @run 6D:B8EA
; WHAT IT DOES: State $11, her pad sequence's move: her animation; at frame $46 with under $0C of its time left, she
; stands on a floor her column stands on, else falls (and the step goes on), the attack's outcome cleared either way;
; before that, the columns and a landing when she has footing.
; HOW: MariaAnimationTick; MariaReadColumns, MariaFloorCheck, MariaSolidUnderColumn and the JMP to MariaStand, or
; MariaToFall; MariaReadColumns, MariaHasNoFooting, MariaLandOnFloor.
; WAITS: no.
; Entry: nothing. Exit: as the routine it ended in, or the step's INC, or the last check.
MariaStateSequence:
    jsr MariaAnimationTick
    lda a:OBJ_FRAME
    cmp #$46
    bne MariaStateSequence_at0
    lda a:OBJ_HURT_TIMER
    cmp #$0C
    bcs MariaStateSequence_at0
    jsr MariaReadColumns
    jsr MariaFloorCheck
    bcc MariaStateSequence_at1
    jsr MariaSolidUnderColumn
    bne MariaStateSequence_at1
    stz a:PlayerAttackKind
    jmp MariaStand
MariaStateSequence_at1:
    stz a:PlayerAttackKind
    jsr MariaToFall
    inc a:OBJ_SUBSTATE
    rts
MariaStateSequence_at0:
    jsr MariaReadColumns
    jsr MariaHasNoFooting
    bcs MariaStateSequence_at2
    jsr MariaLandOnFloor
MariaStateSequence_at2:
    rts
