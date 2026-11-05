; @routine 0045:506 ClearListChannels
; @run 69:5506
; WHAT IT DOES: Stops every list channel and everything kept after them.
; HOW: Block-fills $3C80 up with zero.
; Entry: nothing. Exit: all registers and flags kept.
ClearListChannels:
    tai KernelZeros,ListChannelKind,$007F
    rts

; @routine 0045:50E StartListChannel0Kind1
; @run 69:550E
; WHAT IT DOES: Starts list channel 0 as kind 1.
; HOW: Sets X to 0 and runs into StartListChannelKind1.
; Entry: A list. Exit: A 1, N 0, Z 0, X 0. Y kept.
StartListChannel0Kind1:
    clx

; @routine 0045:50F StartListChannelKind1
; @run 69:550F
; WHAT IT DOES: Starts a list channel as kind 1.
; HOW: Stores the list, then the kind, and clears its place and hold.
; Entry: A list, X channel. Exit: A 1, N 0, Z 0. X and Y kept.
StartListChannelKind1:
    sta a:ListChannelList,x
    lda #$01
StartListChannelKind1_at0:
    sta a:ListChannelKind,x
    stz a:ListChannelPlace,x
    stz a:ListChannelHold,x
    rts

; @routine 0045:51E StartListChannel0Kind2
; @run 69:551E
; WHAT IT DOES: Starts list channel 0 as kind 2.
; HOW: Sets X to 0 and runs into StartListChannelKind2.
; Entry: A list. Exit: A 2, N 0, Z 0, X 0. Y kept.
StartListChannel0Kind2:
    clx

; @routine 0045:51F StartListChannelKind2
; @run 69:551F
; WHAT IT DOES: Starts a list channel as kind 2.
; HOW: Stores the list, then the kind, and clears its place and hold.
; Entry: A list, X channel. Exit: A 2, N 0, Z 0. X and Y kept.
StartListChannelKind2:
    sta a:ListChannelList,x
    lda #$02
    bra StartListChannelKind1_at0

; @routine 0045:526 StartListChannelKind3
; @run 69:5526
; WHAT IT DOES: Starts a list channel as kind 3.
; HOW: Stores the list, then the kind, and clears its place and hold.
; Entry: A list, X channel. Exit: A 3, N 0, Z 0. X and Y kept.
StartListChannelKind3:
    sta a:ListChannelList,x
    lda #$03
    bra StartListChannelKind1_at0

; @routine 0045:52D StartScriptSlot0Kind1
; @run 69:552D
; WHAT IT DOES: Starts script slot 0 as kind 1.
; HOW: Sets X to 0 and runs into StartScriptSlotKind1.
; Entry: A script. Exit: A and Y the script, X 0, N and Z from it.
StartScriptSlot0Kind1:
    clx

; @routine 0045:52E StartScriptSlotKind1
; @run 69:552E
; WHAT IT DOES: Starts a script slot as kind 1.
; HOW: Empties it, stores the kind and the script.
; Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
StartScriptSlotKind1:
    tay
    bsr ClearScriptSlot
    lda #$01
StartScriptSlotKind1_at0:
    sta a:ScriptSlotKind,x
    tya
    sta a:ScriptSlotScript,x
    rts

; @routine 0045:53B StartScriptSlot0Kind2
; @run 69:553B
; WHAT IT DOES: Starts script slot 0 as kind 2.
; HOW: Sets X to 0 and runs into StartScriptSlotKind2.
; Entry: A script. Exit: A and Y the script, X 0, N and Z from it.
StartScriptSlot0Kind2:
    clx

; @routine 0045:53C StartScriptSlotKind2
; @run 69:553C
; WHAT IT DOES: Starts a script slot as kind 2.
; HOW: Empties it, stores the kind and the script.
; Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
StartScriptSlotKind2:
    tay
    bsr ClearScriptSlot
    lda #$02
    bra StartScriptSlotKind1_at0

; @routine 0045:543 StartScriptSlot0Kind3
; @run 69:5543
; WHAT IT DOES: Starts script slot 0 as kind 3.
; HOW: Sets X to 0 and runs into StartScriptSlotKind3.
; Entry: A script. Exit: A and Y the script, X 0, N and Z from it.
StartScriptSlot0Kind3:
    clx

; @routine 0045:544 StartScriptSlotKind3
; @run 69:5544
; WHAT IT DOES: Starts a script slot as kind 3.
; HOW: Empties it, stores the kind and the script.
; Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
StartScriptSlotKind3:
    tay
    bsr ClearScriptSlot
    lda #$03
    bra StartScriptSlotKind1_at0

; @routine 0045:54B StartScriptSlotKind4
; @run 69:554B
; WHAT IT DOES: Starts a script slot as kind 4.
; HOW: Empties it, stores the kind and the script.
; Entry: A script, X slot. Exit: A and Y the script, N and Z from it. X kept.
StartScriptSlotKind4:
    tay
    bsr ClearScriptSlot
    lda #$04
    bra StartScriptSlotKind1_at0

; @routine 0045:552 StartSoleScriptSlot
; @run 69:5552
; WHAT IT DOES: Starts a script slot as kind 5, unless slot 3 is in use: says which.
; HOW: Slot 3's kind nonzero refuses. Otherwise empties the slot and starts it.
; Entry: A script, X slot. Exit, started: A the script, N and Z from it, C 1.
; Refused: A slot 3's kind, N and Z from it, C 0. Y the script, X kept, either way.
StartSoleScriptSlot:
    tay
    lda a:ScriptSlotKind+3
    bne StartSoleScriptSlot_at0
    bsr ClearScriptSlot
    lda #$05
    bsr StartScriptSlotKind1_at0
    sec
    rts
StartSoleScriptSlot_at0:
    clc
    rts

; @routine 0045:562 StartScriptSlotKind6
; @run 69:5562
; WHAT IT DOES: Starts a script slot as kind 6, keeping Y.
; HOW: Empties it, then the kind and, from what it finds in Y, the script.
; Entry: X slot, Y script. Exit: A and Y the script, N and Z from it. X kept.
StartScriptSlotKind6:
    bsr ClearScriptSlot
    lda #$06
    bra StartScriptSlotKind1_at0

; @routine 0045:568 StartScriptSlotKind7
; @run 69:5568
; WHAT IT DOES: Starts a script slot as kind 7, keeping Y.
; HOW: Empties it, then the kind and, from what it finds in Y, the script.
; Entry: X slot, Y script. Exit: A and Y the script, N and Z from it. X kept.
StartScriptSlotKind7:
    bsr ClearScriptSlot
    lda #$07
    bra StartScriptSlotKind1_at0

; @routine 0045:56E ClearScriptSlot
; @run 69:556E
; WHAT IT DOES: Empties a script slot.
; HOW: Zeroes its first eight fields, 12 bytes apart.
; Entry: X slot. Exit: all registers and flags kept.
ClearScriptSlot:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotScript,x
    stz a:ScriptSlotField18,x
    stz a:ScriptSlotField24,x
    stz a:ScriptSlotTimer,x
    stz a:ScriptSlotField3C,x
    stz a:ScriptSlotField48,x
    stz a:ScriptSlotField54,x
    rts
