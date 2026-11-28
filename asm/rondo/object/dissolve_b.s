; @routine 072C:35D JumpByPhaseKeepingPoint072C35D
; @run 70:BB5D
; WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 03).
; HOW: One store, then JumpByPhaseTableBody with the table at $BB74.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
JumpByPhaseKeepingPoint072C35DPhases = $BB74 ; the handlers by phase
JumpByPhaseKeepingPoint072C35D:
    lda #$01
    sta a:PlayerKeepsLastPoint
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseKeepingPoint072C35DPhases,y
    sta z:zTemp00
    lda a:JumpByPhaseKeepingPoint072C35DPhases+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088B:482 JumpByPhaseKeepingPoint088B482
; @run 70:B482
; WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 04).
; HOW: One store, then JumpByPhaseTableBody with the table at $B499.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
JumpByPhaseKeepingPoint088B482Phases = $B499 ; the handlers by phase
JumpByPhaseKeepingPoint088B482:
    lda #$01
    sta a:PlayerKeepsLastPoint
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseKeepingPoint088B482Phases,y
    sta z:zTemp00
    lda a:JumpByPhaseKeepingPoint088B482Phases+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4C:4DB JumpByPhaseKeepingPoint0B4C4DB
; @run 70:BCDB
; WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 06).
; HOW: One store, then JumpByPhaseTableBody with the table at $BCF2.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
JumpByPhaseKeepingPoint0B4C4DBPhases = $BCF2 ; the handlers by phase
JumpByPhaseKeepingPoint0B4C4DB:
    lda #$01
    sta a:PlayerKeepsLastPoint
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseKeepingPoint0B4C4DBPhases,y
    sta z:zTemp00
    lda a:JumpByPhaseKeepingPoint0B4C4DBPhases+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAC:032 JumpByPhaseKeepingPoint0CAC032
; @run 70:B832
; WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 07).
; HOW: One store, then JumpByPhaseTableBody with the table at $B849.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
JumpByPhaseKeepingPoint0CAC032Phases = $B849 ; the handlers by phase
JumpByPhaseKeepingPoint0CAC032:
    lda #$01
    sta a:PlayerKeepsLastPoint
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseKeepingPoint0CAC032Phases,y
    sta z:zTemp00
    lda a:JumpByPhaseKeepingPoint0CAC032Phases+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0E0A:235 DissolveMask
; @run 70:AA35
; WHAT IT DOES: Makes the mask of a dissolve step from A (0 to 63): the row in zTemp00 (A's bits 4 and 5) and a word in
; zTemp01-02 with every bit set but the one A's low four bits name.
; HOW: A bit shifted up from 1 and the word inverted. The same bytes follow every copy of the dissolve.
; WAITS: no.
; Entry: A the phase. Exit: A zTemp02's byte, N and Z from it; C from the last shift; X, Y and V kept.
DissolveMask:
    pha
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp00
    pla
    and #$0F
    sta z:zTemp03
    lda #$01
    sta z:zTemp01
    stz z:zTemp02
DissolveMask_at1:
    lda z:zTemp03
    beq DissolveMask_at0
    asl z:zTemp01
    rol z:zTemp02
    dec z:zTemp03
    bra DissolveMask_at1
DissolveMask_at0:
    lda z:zTemp01
    eor #$FF
    sta z:zTemp01
    lda z:zTemp02
    eor #$FF
    sta z:zTemp02
    rts

; @routine 0E0A:260 DissolveWords0E0A260
; @run 70:AA60
; WHAT IT DOES: Applies a dissolve step's mask (Secret stage 08): of the 64 VRAM words from the address in zBiosCl-Ch, every
; fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
; HOW: The address stepped through zBiosCl-Ch.
; WAITS: no.
; Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
DissolveWords0E0A260:
    phy
    cly
DissolveWords0E0A260_at1:
    tya
    and #$03
    cmp z:zTemp00
    bne DissolveWords0E0A260_at0
    jsr VecSelectVramWriteAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    and z:zTemp01
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and z:zTemp02
    sta a:VDC_DATA_HI
DissolveWords0E0A260_at0:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc DissolveWords0E0A260_at1
    ply
    rts

; @routine 0E0B:7ED DissolveStep0E0B7ED
; @run 70:B7ED
; WHAT IT DOES: One step of a dissolve (Secret stage 08): on the frames its clock allows, the next mask of the order is
; applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
; HOW: DissolveMask, then DissolveWords0E0A260 for each pattern address from $B821.
; WAITS: no.
; Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
; bits; else A the new phase, X kept, Y twice the last pattern.
DissolveStep0E0B7EDPatterns = $B821 ; the VRAM addresses of the patterns, a word each
DissolveStep0E0B7ED:
    lda a:OBJ_DISSOLVE_CLOCK_F,x
    and #$01
    bne DissolveStep0E0B7ED_at0
    phx
    lda a:OBJ_DISSOLVE_PHASE_K,x
    jsr DissolveMask
    ldx a:WorkByteA
DissolveStep0E0B7ED_at1:
    txa
    asl a
    tay
    lda a:DissolveStep0E0B7EDPatterns,y
    sta z:zBiosCl
    lda a:DissolveStep0E0B7EDPatterns+1,y
    sta z:zBiosCh
    jsr DissolveWords0E0A260
    inx
    cpx a:WorkByteB
    bcc DissolveStep0E0B7ED_at1
    plx
    lda a:OBJ_DISSOLVE_PHASE_K,x
    clc
    adc #$13
    and #$3F
    sta a:OBJ_DISSOLVE_PHASE_K,x
DissolveStep0E0B7ED_at0:
    rts

; @routine 10CA:272 DissolveMask10CA272
; @run 70:AA72
; WHAT IT DOES: DissolveMask where Secret stage 0A's overlay holds it: the same bytes at another run address, with no
; absolute operand inside itself, so DissolveMask's C serves it.
; HOW: DissolveMask.
; WAITS: no.
; Entry and exit: as DissolveMask.
DissolveMask10CA272:
    pha
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp00
    pla
    and #$0F
    sta z:zTemp03
    lda #$01
    sta z:zTemp01
    stz z:zTemp02
DissolveMask10CA272_at1:
    lda z:zTemp03
    beq DissolveMask10CA272_at0
    asl z:zTemp01
    rol z:zTemp02
    dec z:zTemp03
    bra DissolveMask10CA272_at1
DissolveMask10CA272_at0:
    lda z:zTemp01
    eor #$FF
    sta z:zTemp01
    lda z:zTemp02
    eor #$FF
    sta z:zTemp02
    rts

; @routine 10CA:29D DissolveWords10CA29D
; @run 70:AA9D
; WHAT IT DOES: Applies a dissolve step's mask (Secret stage 0A): of the 64 VRAM words from the address in zBiosCl-Ch, every
; fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
; HOW: The address stepped through zBiosCl-Ch.
; WAITS: no.
; Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
DissolveWords10CA29D:
    phy
    cly
DissolveWords10CA29D_at1:
    tya
    and #$03
    cmp z:zTemp00
    bne DissolveWords10CA29D_at0
    jsr VecSelectVramWriteAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    and z:zTemp01
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and z:zTemp02
    sta a:VDC_DATA_HI
DissolveWords10CA29D_at0:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc DissolveWords10CA29D_at1
    ply
    rts

; @routine 10CC:5E1 DissolveStep10CC5E1
; @run 70:BDE1
; WHAT IT DOES: One step of a dissolve (Secret stage 0A): on the frames its clock allows, the next mask of the order is
; applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
; HOW: DissolveMask, then DissolveWords10CA29D for each pattern address from $BE15.
; WAITS: no.
; Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
; bits; else A the new phase, X kept, Y twice the last pattern.
DissolveStep10CC5E1Patterns = $BE15 ; the VRAM addresses of the patterns, a word each
DissolveStep10CC5E1:
    lda a:OBJ_DISSOLVE_CLOCK_D,x
    and #$01
    bne DissolveStep10CC5E1_at0
    phx
    lda a:OBJ_DISSOLVE_PHASE_P,x
    jsr DissolveMask10CA272
    ldx a:WorkByteA
DissolveStep10CC5E1_at1:
    txa
    asl a
    tay
    lda a:DissolveStep10CC5E1Patterns,y
    sta z:zBiosCl
    lda a:DissolveStep10CC5E1Patterns+1,y
    sta z:zBiosCh
    jsr DissolveWords10CA29D
    inx
    cpx a:WorkByteB
    bcc DissolveStep10CC5E1_at1
    plx
    lda a:OBJ_DISSOLVE_PHASE_P,x
    clc
    adc #$13
    and #$3F
    sta a:OBJ_DISSOLVE_PHASE_P,x
DissolveStep10CC5E1_at0:
    rts

; @routine 12DA:33A DissolveMask12DA33A
; @run 70:AB3A
; WHAT IT DOES: DissolveMask where Stage 0C's overlay holds it: the same bytes at another run address, with no
; absolute operand inside itself, so DissolveMask's C serves it.
; HOW: DissolveMask.
; WAITS: no.
; Entry and exit: as DissolveMask.
DissolveMask12DA33A:
    pha
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp00
    pla
    and #$0F
    sta z:zTemp03
    lda #$01
    sta z:zTemp01
    stz z:zTemp02
DissolveMask12DA33A_at1:
    lda z:zTemp03
    beq DissolveMask12DA33A_at0
    asl z:zTemp01
    rol z:zTemp02
    dec z:zTemp03
    bra DissolveMask12DA33A_at1
DissolveMask12DA33A_at0:
    lda z:zTemp01
    eor #$FF
    sta z:zTemp01
    lda z:zTemp02
    eor #$FF
    sta z:zTemp02
    rts

; @routine 12DA:365 DissolveWords12DA365
; @run 70:AB65
; WHAT IT DOES: Applies a dissolve step's mask (Stage 0C): of the 64 VRAM words from the address in zBiosCl-Ch, every
; fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
; HOW: The address stepped through zBiosCl-Ch.
; WAITS: no.
; Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
DissolveWords12DA365:
    phy
    cly
DissolveWords12DA365_at1:
    tya
    and #$03
    cmp z:zTemp00
    bne DissolveWords12DA365_at0
    jsr VecSelectVramWriteAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    and z:zTemp01
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and z:zTemp02
    sta a:VDC_DATA_HI
DissolveWords12DA365_at0:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc DissolveWords12DA365_at1
    ply
    rts

; @routine 12DE:65E DissolveStep12DE65E
; @run 71:CE5E
; WHAT IT DOES: One step of a dissolve (Stage 0C): on the frames its clock allows, the next mask of the order is
; applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
; HOW: DissolveMask, then DissolveWords12DA365 for each pattern address from $CE92.
; WAITS: no.
; Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
; bits; else A the new phase, X kept, Y twice the last pattern.
DissolveStep12DE65EPatterns = $CE92 ; the VRAM addresses of the patterns, a word each
DissolveStep12DE65E:
    lda a:OBJ_DISSOLVE_CLOCK_D,x
    and #$01
    bne DissolveStep12DE65E_at0
    phx
    lda a:OBJ_DISSOLVE_PHASE_L,x
    jsr DissolveMask12DA33A
    ldx a:WorkByteA
DissolveStep12DE65E_at1:
    txa
    asl a
    tay
    lda a:DissolveStep12DE65EPatterns,y
    sta z:zBiosCl
    lda a:DissolveStep12DE65EPatterns+1,y
    sta z:zBiosCh
    jsr DissolveWords12DA365
    inx
    cpx a:WorkByteB
    bcc DissolveStep12DE65E_at1
    plx
    lda a:OBJ_DISSOLVE_PHASE_L,x
    clc
    adc #$11
    and #$3F
    sta a:OBJ_DISSOLVE_PHASE_L,x
DissolveStep12DE65E_at0:
    rts

; @routine 13FA:33A DissolveMask13FA33A
; @run 70:AB3A
; WHAT IT DOES: DissolveMask where Secret stage 0C's overlay holds it: the same bytes at another run address, with no
; absolute operand inside itself, so DissolveMask's C serves it.
; HOW: DissolveMask.
; WAITS: no.
; Entry and exit: as DissolveMask.
DissolveMask13FA33A:
    pha
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zTemp00
    pla
    and #$0F
    sta z:zTemp03
    lda #$01
    sta z:zTemp01
    stz z:zTemp02
DissolveMask13FA33A_at1:
    lda z:zTemp03
    beq DissolveMask13FA33A_at0
    asl z:zTemp01
    rol z:zTemp02
    dec z:zTemp03
    bra DissolveMask13FA33A_at1
DissolveMask13FA33A_at0:
    lda z:zTemp01
    eor #$FF
    sta z:zTemp01
    lda z:zTemp02
    eor #$FF
    sta z:zTemp02
    rts

; @routine 13FA:365 DissolveWords13FA365
; @run 70:AB65
; WHAT IT DOES: Applies a dissolve step's mask (Secret stage 0C): of the 64 VRAM words from the address in zBiosCl-Ch, every
; fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
; HOW: The address stepped through zBiosCl-Ch.
; WAITS: no.
; Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
DissolveWords13FA365:
    phy
    cly
DissolveWords13FA365_at1:
    tya
    and #$03
    cmp z:zTemp00
    bne DissolveWords13FA365_at0
    jsr VecSelectVramWriteAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    and z:zTemp01
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and z:zTemp02
    sta a:VDC_DATA_HI
DissolveWords13FA365_at0:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc DissolveWords13FA365_at1
    ply
    rts

; @routine 13FC:0EA DissolveStep13FC0EA
; @run 70:B8EA
; WHAT IT DOES: One step of a dissolve (Secret stage 0C): on the frames its clock allows, the next mask of the order is
; applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
; HOW: DissolveMask, then DissolveWords13FA365 for each pattern address from $B91E.
; WAITS: no.
; Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
; bits; else A the new phase, X kept, Y twice the last pattern.
DissolveStep13FC0EAPatterns = $B91E ; the VRAM addresses of the patterns, a word each
DissolveStep13FC0EA:
    lda a:OBJ_DISSOLVE_CLOCK_D,x
    and #$03
    bne DissolveStep13FC0EA_at0
    phx
    lda a:OBJ_DISSOLVE_PHASE_N,x
    jsr DissolveMask13FA33A
    ldx a:WorkByteA
DissolveStep13FC0EA_at1:
    txa
    asl a
    tay
    lda a:DissolveStep13FC0EAPatterns,y
    sta z:zBiosCl
    lda a:DissolveStep13FC0EAPatterns+1,y
    sta z:zBiosCh
    jsr DissolveWords13FA365
    inx
    cpx a:WorkByteB
    bcc DissolveStep13FC0EA_at1
    plx
    lda a:OBJ_DISSOLVE_PHASE_N,x
    clc
    adc #$13
    and #$3F
    sta a:OBJ_DISSOLVE_PHASE_N,x
DissolveStep13FC0EA_at0:
    rts

; @routine 16CA:260 DissolveWords16CA260
; @run 70:AA60
; WHAT IT DOES: Applies a dissolve step's mask (Secret stage 0E): of the 64 VRAM words from the address in zBiosCl-Ch, every
; fourth one, from the row zTemp00 names, is ANDed with zTemp01-02.
; HOW: The address stepped through zBiosCl-Ch.
; WAITS: no.
; Entry: zTemp00-02 the mask, zBiosCl-Ch the address. Exit: C 1, Y restored, N and Z from it, V from the last add; X kept.
DissolveWords16CA260:
    phy
    cly
DissolveWords16CA260_at1:
    tya
    and #$03
    cmp z:zTemp00
    bne DissolveWords16CA260_at0
    jsr VecSelectVramWriteAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramReadAddress
    lda z:zBiosCl
    sta a:VDC_DATA_LO
    lda z:zBiosCh
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    and z:zTemp01
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and z:zTemp02
    sta a:VDC_DATA_HI
DissolveWords16CA260_at0:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc DissolveWords16CA260_at1
    ply
    rts

; @routine 16CB:61D JumpByPhaseKeepingPoint16CB61D
; @run 70:B61D
; WHAT IT DOES: Sets PlayerKeepsLastPoint, then goes to the object's phase's handler (Secret stage 0E).
; HOW: One store, then JumpByPhaseTableBody with the table at $B634.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled phase, C its bit 7.
JumpByPhaseKeepingPoint16CB61DPhases = $B634 ; the handlers by phase
JumpByPhaseKeepingPoint16CB61D:
    lda #$01
    sta a:PlayerKeepsLastPoint
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseKeepingPoint16CB61DPhases,y
    sta z:zTemp00
    lda a:JumpByPhaseKeepingPoint16CB61DPhases+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CB:6A4 DissolveStep16CB6A4
; @run 70:B6A4
; WHAT IT DOES: One step of a dissolve (Secret stage 0E): on the frames its clock allows, the next mask of the order is
; applied to the patterns from zero-page $26FF to $2700 of the table, and the phase steps.
; HOW: DissolveMask, then DissolveWords16CA260 for each pattern address from $B6D8.
; WAITS: no.
; Entry: X object, WorkByteA and WorkByteB the first pattern and the one past the last. Exit: skipped: A the clock's
; bits; else A the new phase, X kept, Y twice the last pattern.
DissolveStep16CB6A4Patterns = $B6D8 ; the VRAM addresses of the patterns, a word each
DissolveStep16CB6A4:
    lda a:OBJ_DISSOLVE_CLOCK_J,x
    and #$01
    bne DissolveStep16CB6A4_at0
    phx
    lda a:OBJ_DISSOLVE_PHASE_N,x
    jsr DissolveMask
    ldx a:WorkByteA
DissolveStep16CB6A4_at1:
    txa
    asl a
    tay
    lda a:DissolveStep16CB6A4Patterns,y
    sta z:zBiosCl
    lda a:DissolveStep16CB6A4Patterns+1,y
    sta z:zBiosCh
    jsr DissolveWords16CA260
    inx
    cpx a:WorkByteB
    bcc DissolveStep16CB6A4_at1
    plx
    lda a:OBJ_DISSOLVE_PHASE_N,x
    clc
    adc #$13
    and #$3F
    sta a:OBJ_DISSOLVE_PHASE_N,x
DissolveStep16CB6A4_at0:
    rts
