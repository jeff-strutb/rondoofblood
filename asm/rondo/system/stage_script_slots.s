; @routine 0251:29F RunScriptSlotsByPause
; @run 6C:829F
; WHAT IT DOES: Runs the script slots: every slot from $0B down, or with zPaused's bit 0 set only slots 2 and $0B.
; HOW: BBR0 of zPaused to the full walk; else BSR for slot 2 and into the same for slot $0B.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
RunScriptSlotsByPause:
    bbr0 zPaused,RunAllScriptSlots
    ldx #$02
    bsr RunScriptSlotIfUsed
    ldx #$0B

; @routine 0251:2A8 RunScriptSlotIfUsed
; @run 6C:82A8
; WHAT IT DOES: Runs script slot X's step when the slot is in use.
; HOW: A test of ScriptSlotKind.
; WAITS: no.
; Entry: X the script slot. Exit: as the step, or A zero.
RunScriptSlotIfUsed:
    lda a:ScriptSlotKind,x
    bne RunScriptSlotByKind
    rts

; @routine 0251:2AE RunScriptSlot3Flash
; @run 6C:82AE
; WHAT IT DOES: Runs slot 3's flash step.
; HOW: LDX #3 and a jump to the flash.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
RunScriptSlot3Flash:
    ldx #$03
    jmp EffectFlash1A31

; @routine 0251:2B3 RunAllScriptSlots
; @run 6C:82B3
; WHAT IT DOES: Runs every used script slot from $0B down; with kind 5 in slot 3 only that slot's flash.
; HOW: The slot kept in zClimbStepBefore across each step.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction, call or jump left them.
RunAllScriptSlots:
    lda a:ScriptSlotKind+3
    cmp #$05
    beq RunScriptSlot3Flash
    ldx #$0B
RunAllScriptSlots_at0:
    lda a:ScriptSlotKind,x
    beq RunAllScriptSlots_at1
    stx z:zClimbStepBefore
    bsr RunScriptSlotByKind
    ldx z:zClimbStepBefore
RunAllScriptSlots_at1:
    dex
    bpl RunAllScriptSlots_at0
    rts

; @routine 0251:2CB RunScriptSlotByKind
; @run 6C:82CB
; WHAT IT DOES: Goes to the step of script kind A.
; HOW: A jump through the table at $82DB by kind less one.
; WAITS: no.
; Entry: A the slot's kind, X the slot. Exit: as the step.
ScriptKindSteps = $82DB ; a step per script kind
RunScriptSlotByKind:
    dec a
    asl a
    tay
    lda a:ScriptKindSteps,y
    sta z:zTemp00
    lda a:ScriptKindSteps+1,y
    sta z:zTemp01
    jmp (ZeroPage)
