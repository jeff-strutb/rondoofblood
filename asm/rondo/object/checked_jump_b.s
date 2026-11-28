; @routine 072C:58B CheckedJump072C58B
; @run 70:BD8B
; WHAT IT DOES: An object's dispatcher (Secret stage 03): the loaded overlay's routines at $B3A4, $B3DB are run,
; then the handler of its state from the table at $BDA3.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump072C58BHandlers = $BDA3 ; the handlers by state
CheckedJump072C58B:
    jsr FlashByHeight
    jsr FaceByVelocityAndCatch
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump072C58BHandlers,y
    sta z:zTemp00
    lda a:CheckedJump072C58BHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0E0B:040 CheckedJump0E0B040
; @run 70:B040
; WHAT IT DOES: An object's dispatcher (Secret stage 08): WORK_F is counted on, the loaded overlay's routines at $B071, $B099, $B0AB, $B0C2 are run,
; then the handler of its state from the table at $B061.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump0E0B040Handlers = $B061 ; the handlers by state
CheckedJump0E0B040:
    inc a:OBJ_CHILD_ORDER,x
    jsr FaceThePlayerByStateTableB
    jsr HalveDirectionWhenHurt
    jsr StateSevenOnStatusBit7
    jsr SetHitBoxFromTable
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump0E0B040Handlers,y
    sta z:zTemp00
    lda a:CheckedJump0E0B040Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0E0C:4A8 CheckedJump0E0C4A8
; @run 70:BCA8
; WHAT IT DOES: An object's dispatcher (Secret stage 08): WORK_D is counted on, no check is run,
; then the handler of its state from the table at $BCBD.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump0E0C4A8Handlers = $BCBD ; the handlers by state
CheckedJump0E0C4A8:
    inc a:OBJ_FLASH_FRAMES,x
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump0E0C4A8Handlers,y
    sta z:zTemp00
    lda a:CheckedJump0E0C4A8Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBD:428 CheckedJump0EBD428
; @run 71:C428
; WHAT IT DOES: An object's dispatcher (Stage 09): WORK_E is counted on, the loaded overlay's routines at $C44D, $C45F are run,
; then the handler of its state from the table at $C443.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump0EBD428Handlers = $C443 ; the handlers by state
CheckedJump0EBD428:
    inc a:OBJ_FLYER_WAIT,x
    jsr MarkWhenPlayerNear0EBD
    jsr HitToState4Frame0EBD
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump0EBD428Handlers,y
    sta z:zTemp00
    lda a:CheckedJump0EBD428Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBD:599 CheckedJump0EBD599
; @run 71:C599
; WHAT IT DOES: An object's dispatcher (Stage 09): WORK_E is counted on, the loaded overlay's routines at $C45F are run,
; then the handler of its state from the table at $C5B1.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump0EBD599Handlers = $C5B1 ; the handlers by state
CheckedJump0EBD599:
    inc a:OBJ_FLYER_WAIT,x
    jsr HitToState4Frame0EBD
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump0EBD599Handlers,y
    sta z:zTemp00
    lda a:CheckedJump0EBD599Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBF:0C5 CheckedJump0EBF0C5
; @run 71:D0C5
; WHAT IT DOES: An object's dispatcher (Stage 09): WORK_H is counted on, the loaded overlay's routines at $D0EB are run,
; then the handler of its state from the table at $D0DD.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump0EBF0C5Handlers = $D0DD ; the handlers by state
CheckedJump0EBF0C5:
    inc a:OBJ_FALL_LO,x
    jsr HitSoundAndSix0EBF
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump0EBF0C5Handlers,y
    sta z:zTemp00
    lda a:CheckedJump0EBF0C5Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBF:592 CheckedJump0EBF592
; @run 71:D592
; WHAT IT DOES: An object's dispatcher (Stage 09): the loaded overlay's routines at $D5BE, $D5E1 are run,
; then the handler of its state from the table at $D5AA.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump0EBF592Handlers = $D5AA ; the handlers by state
CheckedJump0EBF592Check0 = $D5BE ; a check of this overlay
CheckedJump0EBF592Check1 = $D5E1 ; a check of this overlay
CheckedJump0EBF592:
    jsr CheckedJump0EBF592Check0
    jsr CheckedJump0EBF592Check1
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump0EBF592Handlers,y
    sta z:zTemp00
    lda a:CheckedJump0EBF592Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101B:066 CheckedJump101B066
; @run 70:B066
; WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_G is counted on, the loaded overlay's routines at $B088 are run,
; then the handler of its state from the table at $B07E.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump101B066Handlers = $B07E ; the handlers by state
CheckedJump101B066:
    inc a:OBJ_WORK_G,x
    jsr HitToState4_101B
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump101B066Handlers,y
    sta z:zTemp00
    lda a:CheckedJump101B066Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101B:43C CheckedJump101B43C
; @run 70:B43C
; WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_D is counted on, the loaded overlay's routines at $B46C are run,
; then the handler of its state from the table at $B454.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump101B43CHandlers = $B454 ; the handlers by state
CheckedJump101B43C:
    inc a:OBJ_FLASH_FRAMES,x
    jsr HitToState11_101B
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump101B43CHandlers,y
    sta z:zTemp00
    lda a:CheckedJump101B43CHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101C:289 CheckedJump101C289
; @run 70:BA89
; WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_D is counted on, the loaded overlay's routines at $BAAE, $BAC0 are run,
; then the handler of its state from the table at $BAA4.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump101C289Handlers = $BAA4 ; the handlers by state
CheckedJump101C289:
    inc a:OBJ_FLASH_FRAMES,x
    jsr HitSoundToState4_101C
    jsr HitBoxSizeByFrame101C
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump101C289Handlers,y
    sta z:zTemp00
    lda a:CheckedJump101C289Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101C:64B CheckedJump101C64B
; @run 70:BE4B
; WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_D is counted on, the loaded overlay's routines at $BAAE, $BAC0 are run,
; then the handler of its state from the table at $BE66.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump101C64BHandlers = $BE66 ; the handlers by state
CheckedJump101C64B:
    inc a:OBJ_FLASH_FRAMES,x
    jsr HitSoundToState4_101C
    jsr HitBoxSizeByFrame101C
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump101C64BHandlers,y
    sta z:zTemp00
    lda a:CheckedJump101C64BHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101C:76E CheckedJump101C76E
; @run 70:BF6E
; WHAT IT DOES: An object's dispatcher (Stage 0A): the loaded overlay's routines at $BF90, $BFE0 are run,
; then the handler of its state from the table at $BF86.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump101C76EHandlers = $BF86 ; the handlers by state
CheckedJump101C76E:
    jsr CycleWhileUnderState2_101C
    jsr HitBringDownSlots101C
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump101C76EHandlers,y
    sta z:zTemp00
    lda a:CheckedJump101C76EHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101D:3EB CheckedJump101D3EB
; @run 71:C3EB
; WHAT IT DOES: An object's dispatcher (Stage 0A): the loaded overlay's routines at $C40F, $C438 are run,
; then the handler of its state from the table at $C403.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump101D3EBHandlers = $C403 ; the handlers by state
CheckedJump101D3EB:
    jsr ToState4WhenPlayerClose101D
    jsr HitToState5_101D
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump101D3EBHandlers,y
    sta z:zTemp00
    lda a:CheckedJump101D3EBHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101E:671 CheckedJump101E671
; @run 71:CE71
; WHAT IT DOES: An object's dispatcher (Stage 0A): WORK_E is counted on, the loaded overlay's routines at $CE8F are run,
; then the handler of its state from the table at $CE89.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump101E671Handlers = $CE89 ; the handlers by state
CheckedJump101E671:
    inc a:OBJ_FLYER_WAIT,x
    jsr HitToState2Frame101E
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump101E671Handlers,y
    sta z:zTemp00
    lda a:CheckedJump101E671Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117B:515 CheckedJump117B515
; @run 70:B515
; WHAT IT DOES: An object's dispatcher (Stage 0B): WORK_H is counted on, the loaded overlay's routines at $B53B are run,
; then the handler of its state from the table at $B52D.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump117B515Handlers = $B52D ; the handlers by state
CheckedJump117B515:
    inc a:OBJ_FALL_LO,x
    jsr BurstWhenFlaggedHitB
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump117B515Handlers,y
    sta z:zTemp00
    lda a:CheckedJump117B515Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117C:19B CheckedJump117C19B
; @run 70:B99B
; WHAT IT DOES: An object's dispatcher (Stage 0B): the loaded overlay's routines at $B9C7, $B9EA are run,
; then the handler of its state from the table at $B9B3.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump117C19BHandlers = $B9B3 ; the handlers by state
CheckedJump117C19B:
    jsr SixteenBitTimerToState8
    jsr KnockedUpWhenHit
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump117C19BHandlers,y
    sta z:zTemp00
    lda a:CheckedJump117C19BHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117C:653 CheckedJump117C653
; @run 70:BE53
; WHAT IT DOES: An object's dispatcher (Stage 0B): WORK_H is counted on, the loaded overlay's routines at $BE84, $BE9D, $BEB9, $BECB are run,
; then the handler of its state from the table at $BE74.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump117C653Handlers = $BE74 ; the handlers by state
CheckedJump117C653:
    inc a:OBJ_FALL_LO,x
    jsr FaceThePlayerInSomeStatesB
    jsr EnterState6WhenHitHard
    jsr EnterState7IfYHighOrFlagged
    jsr SetHitboxFromFrameRowsC
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump117C653Handlers,y
    sta z:zTemp00
    lda a:CheckedJump117C653Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DB:418 CheckedJump12DB418
; @run 70:B418
; WHAT IT DOES: An object's dispatcher (Stage 0C): WORK_F is counted on, the loaded overlay's routines at $B43A are run,
; then the handler of its state from the table at $B430.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump12DB418Handlers = $B430 ; the handlers by state
CheckedJump12DB418:
    inc a:OBJ_CHILD_ORDER,x
    jsr HitToState4_12DB
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump12DB418Handlers,y
    sta z:zTemp00
    lda a:CheckedJump12DB418Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DC:0C8 CheckedJump12DC0C8
; @run 70:B8C8
; WHAT IT DOES: An object's dispatcher (Stage 0C): WORK_D is counted on, no check is run,
; then the handler of its state from the table at $B8DD.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump12DC0C8Handlers = $B8DD ; the handlers by state
CheckedJump12DC0C8:
    inc a:OBJ_FLASH_FRAMES,x
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump12DC0C8Handlers,y
    sta z:zTemp00
    lda a:CheckedJump12DC0C8Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DD:358 CheckedJump12DD358
; @run 71:C358
; WHAT IT DOES: An object's dispatcher (Stage 0C): WORK_D is counted on, the loaded overlay's routines at $C37F, $C3AF are run,
; then the handler of its state from the table at $C373.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump12DD358Handlers = $C373 ; the handlers by state
CheckedJump12DD358:
    inc a:OBJ_FLASH_FRAMES,x
    jsr DropAndShake12DD
    jsr HitStopAll12DD
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump12DD358Handlers,y
    sta z:zTemp00
    lda a:CheckedJump12DD358Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DE:24C CheckedJump12DE24C
; @run 71:CA4C
; WHAT IT DOES: An object's dispatcher (Stage 0C): WORK_D is counted on, the loaded overlay's routines at $CA73, $CA85 are run,
; then the handler of its state from the table at $CA67.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump12DE24CHandlers = $CA67 ; the handlers by state
CheckedJump12DE24C:
    inc a:OBJ_FLASH_FRAMES,x
    jsr HalveWhenHit12DE
    jsr HitStopAll12DE
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump12DE24CHandlers,y
    sta z:zTemp00
    lda a:CheckedJump12DE24CHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161B:2E2 CheckedJump161B2E2
; @run 70:B2E2
; WHAT IT DOES: An object's dispatcher (Stage 0E): no check is run,
; then the handler of its state from the table at $B2F4.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump161B2E2Handlers = $B2F4 ; the handlers by state
CheckedJump161B2E2:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump161B2E2Handlers,y
    sta z:zTemp00
    lda a:CheckedJump161B2E2Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161B:31F CheckedJump161B31F
; @run 70:B31F
; WHAT IT DOES: An object's dispatcher (Stage 0E): WORK_E is counted on, the loaded overlay's routines at $B341 are run,
; then the handler of its state from the table at $B337.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump161B31FHandlers = $B337 ; the handlers by state
CheckedJump161B31F:
    inc a:OBJ_FLYER_WAIT,x
    jsr HitSpawn92_161B
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump161B31FHandlers,y
    sta z:zTemp00
    lda a:CheckedJump161B31FHandlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161B:605 CheckedJump161B605
; @run 70:B605
; WHAT IT DOES: An object's dispatcher (Stage 0E): WORK_I is counted on, the loaded overlay's routines at $B625 are run,
; then the handler of its state from the table at $B61D.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump161B605Handlers = $B61D ; the handlers by state
CheckedJump161B605:
    inc a:OBJ_FALL_HI,x
    jsr HitHold32_161B
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump161B605Handlers,y
    sta z:zTemp00
    lda a:CheckedJump161B605Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161C:2C4 CheckedJump161C2C4
; @run 70:BAC4
; WHAT IT DOES: An object's dispatcher (Stage 0E): WORK_H is counted on, the loaded overlay's routines at $BAEF, $BB1A are run,
; then the handler of its state from the table at $BADF.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump161C2C4Handlers = $BADF ; the handlers by state
CheckedJump161C2C4:
    inc a:OBJ_FALL_LO,x
    jsr HitToState7_161C
    jsr HitBoxByFrameRange161C
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump161C2C4Handlers,y
    sta z:zTemp00
    lda a:CheckedJump161C2C4Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DC:011 CheckedJump18DC011
; @run 70:B811
; WHAT IT DOES: An object's dispatcher (Stage 10): the loaded overlay's routines at $B6FE, $B71B are run,
; then the handler of its substate from the table at $B829.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump18DC011Handlers = $B829 ; the handlers by substate
CheckedJump18DC011:
    jsr SpawnAtRandom18DB
    jsr HealthByFirstHit18DB
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:CheckedJump18DC011Handlers,y
    sta z:zTemp00
    lda a:CheckedJump18DC011Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DC:2D0 CheckedJump18DC2D0
; @run 70:BAD0
; WHAT IT DOES: An object's dispatcher (Stage 10): WORK_D is counted on, no check is run,
; then the handler of its state from the table at $BAE5.
; HOW: JSRs to the routines loaded at those addresses, then the table jump.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled value.
CheckedJump18DC2D0Handlers = $BAE5 ; the handlers by state
CheckedJump18DC2D0:
    inc a:OBJ_FLASH_FRAMES,x
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:CheckedJump18DC2D0Handlers,y
    sta z:zTemp00
    lda a:CheckedJump18DC2D0Handlers+1,y
    sta z:zTemp01
    jmp (ZeroPage)
