; @routine 05CA:40F ErodePatterns05CA40F
; @run 70:AC0F
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern05CA41B, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns05CA40F:
    phx
    clx
ErodePatterns05CA40F_at0:
    jsr ErodePattern05CA41B
    inx
    cpx #$08
    bcc ErodePatterns05CA40F_at0
    plx
    rts

; @routine 05CA:41B ErodePattern05CA41B
; @run 70:AC1B
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $AC2C into zBiosCl-Ch, then ErodeVramWords05CA43C.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern05CA41BPatterns = $AC2C ; the VRAM addresses of the eight patterns
ErodePattern05CA41B:
    txa
    asl a
    tay
    lda a:ErodePattern05CA41BPatterns,y
    sta z:zBiosCl
    lda a:ErodePattern05CA41BPatterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords05CA43C
    rts

; @routine 05CA:43C ErodeVramWords05CA43C
; @run 70:AC3C
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords05CA43C:
    phy
    cly
ErodeVramWords05CA43C_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords05CA43C_at0
    ply
    rts

; @routine 05CA:690 SubStepPickup05CA690
; @run 70:AE90
; WHAT IT DOES: The pickup of Secret stage 02: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck05CA6B3, then JumpByStateTableBody with the table at $AEA5.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup05CA690States = $AEA5 ; the handlers by state
SubStepPickup05CA690:
    jsr SubStepPickupHitCheck05CA6B3
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup05CA690States,y
    sta z:zTemp00
    lda a:SubStepPickup05CA690States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 05CA:6B3 SubStepPickupHitCheck05CA6B3
; @run 70:AEB3
; WHAT IT DOES: The pickup's hit (Secret stage 02): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck05CA6B3:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck05CA6B3_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck05CA6B3_at0
    cmp #$05
    bcs SubStepPickupHitCheck05CA6B3_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck05CA6B3_at0:
    rts

; @routine 05CA:6D3 SubStepPickupStart05CA6D3
; @run 70:AED3
; WHAT IT DOES: Starts the pickup (Secret stage 02): its record, a timer of $C0, its place, list channel $81 and script
; $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart05CA6D3:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$80
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$30
    sta a:OBJ_Y_LO,x
    phx
    lda #$81
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart05CA6D3_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart05CA6D3_at0:
    plx
    rts

; @routine 05CA:718 SubStepPickupWait05CA718
; @run 70:AF18
; WHAT IT DOES: The pickup waits (Secret stage 02): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $AF2C; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait05CA718:
    lda #$AF
    sta z:zTemp11
    lda #$2C
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait05CA718_at0
    inc a:OBJ_STATE,x
SubStepPickupWait05CA718_at0:
    rts

; @routine 05CA:735 SubStepPickupFirstScripts05CA735
; @run 70:AF35
; WHAT IT DOES: The pickup's first round (Secret stage 02): each time its frame list ends, script $83 plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts05CA735:
    lda #$AF
    sta z:zTemp11
    lda #$2C
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts05CA735_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$83
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts05CA735_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts05CA735_at0:
    rts

; @routine 05CA:765 SubStepPickupSecondScripts05CA765
; @run 70:AF65
; WHAT IT DOES: The pickup's second round (Secret stage 02): every fourth frame script $89 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn05CA78E.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts05CA765:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts05CA765_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$89
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts05CA765_at0
    jsr SubStepPickupSpawn05CA78E
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts05CA765_at0:
    rts

; @routine 05CA:78E SubStepPickupSpawn05CA78E
; @run 70:AF8E
; WHAT IT DOES: Starts up to eight objects of kind $23 in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn05CA78E:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn05CA78E_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn05CA78E_at0
    lda #$23
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn05CA78E_at1
SubStepPickupSpawn05CA78E_at0:
    plx
    rts

; @routine 05CA:7A8 SubStepPickupFall05CA7A8
; @run 70:AFA8
; WHAT IT DOES: The pickup falls (Secret stage 02): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall05CA7A8:
    lda #$AF
    sta z:zTemp11
    lda #$DB
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall05CA7A8_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall05CA7A8_at0:
    rts

; @routine 05CA:7C3 SubStepPickupRest05CA7C3
; @run 70:AFC3
; WHAT IT DOES: The landed pickup (Secret stage 02): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest05CA7C3:
    lda #$AF
    sta z:zTemp11
    lda #$DB
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest05CA7C3_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest05CA7C3_at0:
    rts

; @routine 05CB:01A SubStepPickupErase05CB01A
; @run 70:B01A
; WHAT IT DOES: The pickup's last state (Secret stage 02): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns05CA40F, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase05CB01A:
    jsr ErodePatterns05CA40F
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase05CB01A_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase05CB01A_at0:
    rts

; @routine 05CC:058 ErodePattern05CC058
; @run 70:B858
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $B869 into zBiosCl-Ch, then ErodeVramWords05CC09F.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern05CC058Patterns = $B869 ; the VRAM addresses of the eight patterns
ErodePattern05CC058:
    txa
    asl a
    tay
    lda a:ErodePattern05CC058Patterns,y
    sta z:zBiosCl
    lda a:ErodePattern05CC058Patterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords05CC09F
    rts

; @routine 05CC:09F ErodeVramWords05CC09F
; @run 70:B89F
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords05CC09F:
    phy
    cly
ErodeVramWords05CC09F_at1:
    tya
    and #$03
    cmp z:zTemp00
    bne ErodeVramWords05CC09F_at0
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
ErodeVramWords05CC09F_at0:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc ErodeVramWords05CC09F_at1
    ply
    rts

; @routine 072A:417 ErodePatterns072A417
; @run 70:AC17
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern072A423, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns072A417:
    phx
    clx
ErodePatterns072A417_at0:
    jsr ErodePattern072A423
    inx
    cpx #$08
    bcc ErodePatterns072A417_at0
    plx
    rts

; @routine 072A:423 ErodePattern072A423
; @run 70:AC23
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $AC34 into zBiosCl-Ch, then ErodeVramWords072A444.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern072A423Patterns = $AC34 ; the VRAM addresses of the eight patterns
ErodePattern072A423:
    txa
    asl a
    tay
    lda a:ErodePattern072A423Patterns,y
    sta z:zBiosCl
    lda a:ErodePattern072A423Patterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords072A444
    rts

; @routine 072A:444 ErodeVramWords072A444
; @run 70:AC44
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords072A444:
    phy
    cly
ErodeVramWords072A444_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords072A444_at0
    ply
    rts

; @routine 072A:6C4 SubStepPickup072A6C4
; @run 70:AEC4
; WHAT IT DOES: The pickup of Secret stage 03: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck072A6E7, then JumpByStateTableBody with the table at $AED9.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup072A6C4States = $AED9 ; the handlers by state
SubStepPickup072A6C4:
    jsr SubStepPickupHitCheck072A6E7
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup072A6C4States,y
    sta z:zTemp00
    lda a:SubStepPickup072A6C4States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 072A:6E7 SubStepPickupHitCheck072A6E7
; @run 70:AEE7
; WHAT IT DOES: The pickup's hit (Secret stage 03): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck072A6E7:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck072A6E7_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck072A6E7_at0
    cmp #$05
    bcs SubStepPickupHitCheck072A6E7_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck072A6E7_at0:
    rts

; @routine 072A:707 SubStepPickupStart072A707
; @run 70:AF07
; WHAT IT DOES: Starts the pickup (Secret stage 03): its record, a timer of $C0, its place, list channel $81 and script
; $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart072A707:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    lda #$30
    sta a:OBJ_Y_LO,x
    phx
    lda #$81
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart072A707_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart072A707_at0:
    plx
    rts

; @routine 072A:748 SubStepPickupWait072A748
; @run 70:AF48
; WHAT IT DOES: The pickup waits (Secret stage 03): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $AF5C; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait072A748:
    lda #$AF
    sta z:zTemp11
    lda #$5C
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait072A748_at0
    inc a:OBJ_STATE,x
SubStepPickupWait072A748_at0:
    rts

; @routine 072A:765 SubStepPickupFirstScripts072A765
; @run 70:AF65
; WHAT IT DOES: The pickup's first round (Secret stage 03): each time its frame list ends, script $83 plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts072A765:
    lda #$AF
    sta z:zTemp11
    lda #$5C
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts072A765_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$83
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts072A765_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts072A765_at0:
    rts

; @routine 072A:795 SubStepPickupSecondScripts072A795
; @run 70:AF95
; WHAT IT DOES: The pickup's second round (Secret stage 03): every fourth frame script $89 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn072A7BE.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts072A795:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts072A795_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$89
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts072A795_at0
    jsr SubStepPickupSpawn072A7BE
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts072A795_at0:
    rts

; @routine 072A:7BE SubStepPickupSpawn072A7BE
; @run 70:AFBE
; WHAT IT DOES: Starts up to eight objects of kind $23 in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn072A7BE:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn072A7BE_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn072A7BE_at0
    lda #$23
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn072A7BE_at1
SubStepPickupSpawn072A7BE_at0:
    plx
    rts

; @routine 072A:7D8 SubStepPickupFall072A7D8
; @run 70:AFD8
; WHAT IT DOES: The pickup falls (Secret stage 03): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall072A7D8:
    lda #$B0
    sta z:zTemp11
    lda #$0B
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall072A7D8_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall072A7D8_at0:
    rts

; @routine 072A:7F3 SubStepPickupRest072A7F3
; @run 70:AFF3
; WHAT IT DOES: The landed pickup (Secret stage 03): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest072A7F3:
    lda #$B0
    sta z:zTemp11
    lda #$0B
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest072A7F3_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest072A7F3_at0:
    rts

; @routine 072B:04A SubStepPickupErase072B04A
; @run 70:B04A
; WHAT IT DOES: The pickup's last state (Secret stage 03): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns072A417, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase072B04A:
    jsr ErodePatterns072A417
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase072B04A_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase072B04A_at0:
    rts

; @routine 072C:520 ErodePattern072C520
; @run 70:BD20
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $BD31 into zBiosCl-Ch, then ErodeVramWords072C541.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern072C520Patterns = $BD31 ; the VRAM addresses of the eight patterns
ErodePattern072C520:
    txa
    asl a
    tay
    lda a:ErodePattern072C520Patterns,y
    sta z:zBiosCl
    lda a:ErodePattern072C520Patterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords072C541
    rts

; @routine 072C:541 ErodeVramWords072C541
; @run 70:BD41
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords072C541:
    phy
    cly
ErodeVramWords072C541_at1:
    tya
    and #$03
    cmp z:zTemp00
    bne ErodeVramWords072C541_at0
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
ErodeVramWords072C541_at0:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc ErodeVramWords072C541_at1
    ply
    rts

; @routine 088C:52F ErodePatterns088C52F
; @run 70:BD2F
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern088C53B, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns088C52F:
    phx
    clx
ErodePatterns088C52F_at0:
    jsr ErodePattern088C53B
    inx
    cpx #$08
    bcc ErodePatterns088C52F_at0
    plx
    rts

; @routine 088C:53B ErodePattern088C53B
; @run 70:BD3B
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $BD4C into zBiosCl-Ch, then ErodeVramWords088C55C.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern088C53BPatterns = $BD4C ; the VRAM addresses of the eight patterns
ErodePattern088C53B:
    txa
    asl a
    tay
    lda a:ErodePattern088C53BPatterns,y
    sta z:zBiosCl
    lda a:ErodePattern088C53BPatterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords088C55C
    rts

; @routine 088C:55C ErodeVramWords088C55C
; @run 70:BD5C
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords088C55C:
    phy
    cly
ErodeVramWords088C55C_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords088C55C_at0
    ply
    rts

; @routine 088C:7CB SubStepPickup088C7CB
; @run 70:BFCB
; WHAT IT DOES: The pickup of Secret stage 04: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck088C7EE, then JumpByStateTableBody with the table at $BFE0.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup088C7CBStates = $BFE0 ; the handlers by state
SubStepPickup088C7CB:
    jsr SubStepPickupHitCheck088C7EE
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup088C7CBStates,y
    sta z:zTemp00
    lda a:SubStepPickup088C7CBStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088C:7EE SubStepPickupHitCheck088C7EE
; @run 70:BFEE
; WHAT IT DOES: The pickup's hit (Secret stage 04): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck088C7EE:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck088C7EE_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck088C7EE_at0
    cmp #$05
    bcs SubStepPickupHitCheck088C7EE_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck088C7EE_at0:
    rts

; @routine 088D:00E SubStepPickupStart088D00E
; @run 71:C00E
; WHAT IT DOES: Starts the pickup (Secret stage 04): its record, a timer of $C0, its place, list channel $84 and script
; $83, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart088D00E:
    jsr VecLoadObjectRecord
    lda #$07
    sta a:OBJ_SPRITE_SET,x
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    lda #$30
    sta a:OBJ_Y_LO,x
    phx
    lda #$84
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$83
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart088D00E_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart088D00E_at0:
    plx
    rts

; @routine 088D:054 SubStepPickupWait088D054
; @run 71:C054
; WHAT IT DOES: The pickup waits (Secret stage 04): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $C068; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait088D054:
    lda #$C0
    sta z:zTemp11
    lda #$68
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait088D054_at0
    inc a:OBJ_STATE,x
SubStepPickupWait088D054_at0:
    rts

; @routine 088D:071 SubStepPickupFirstScripts088D071
; @run 71:C071
; WHAT IT DOES: The pickup's first round (Secret stage 04): each time its frame list ends, script $84 plus its count is
; started; after 6, the count is cleared, the frame set to $4C and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts088D071:
    lda #$C0
    sta z:zTemp11
    lda #$68
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts088D071_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$84
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts088D071_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$4C
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts088D071_at0:
    rts

; @routine 088D:0A1 SubStepPickupSecondScripts088D0A1
; @run 71:C0A1
; WHAT IT DOES: The pickup's second round (Secret stage 04): every fourth frame script $8A plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn088D0CA.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts088D0A1:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts088D0A1_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$8A
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts088D0A1_at0
    jsr SubStepPickupSpawn088D0CA
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts088D0A1_at0:
    rts

; @routine 088D:0CA SubStepPickupSpawn088D0CA
; @run 71:C0CA
; WHAT IT DOES: Starts up to eight objects of kind $2E in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn088D0CA:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn088D0CA_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn088D0CA_at0
    lda #$2E
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn088D0CA_at1
SubStepPickupSpawn088D0CA_at0:
    plx
    rts

; @routine 088D:0E4 SubStepPickupFall088D0E4
; @run 71:C0E4
; WHAT IT DOES: The pickup falls (Secret stage 04): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall088D0E4:
    lda #$C1
    sta z:zTemp11
    lda #$17
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall088D0E4_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall088D0E4_at0:
    rts

; @routine 088D:0FF SubStepPickupRest088D0FF
; @run 71:C0FF
; WHAT IT DOES: The landed pickup (Secret stage 04): its frames play; on frame $58 sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest088D0FF:
    lda #$C1
    sta z:zTemp11
    lda #$17
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$58
    bne SubStepPickupRest088D0FF_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest088D0FF_at0:
    rts

; @routine 088D:156 SubStepPickupErase088D156
; @run 71:C156
; WHAT IT DOES: The pickup's last state (Secret stage 04): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns088C52F, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase088D156:
    jsr ErodePatterns088C52F
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase088D156_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase088D156_at0:
    rts

; @routine 09EC:511 SubStepPickup09EC511
; @run 70:BD11
; WHAT IT DOES: The pickup of Secret stage 05: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck09EC534, then JumpByStateTableBody with the table at $BD26.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup09EC511States = $BD26 ; the handlers by state
SubStepPickup09EC511:
    jsr SubStepPickupHitCheck09EC534
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup09EC511States,y
    sta z:zTemp00
    lda a:SubStepPickup09EC511States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 09EC:534 SubStepPickupHitCheck09EC534
; @run 70:BD34
; WHAT IT DOES: The pickup's hit (Secret stage 05): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck09EC534:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck09EC534_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck09EC534_at0
    cmp #$05
    bcs SubStepPickupHitCheck09EC534_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck09EC534_at0:
    rts

; @routine 09EC:554 SubStepPickupStart09EC554
; @run 70:BD54
; WHAT IT DOES: Starts the pickup (Secret stage 05): its record, a timer of $C0, its place, list channel $81 and script
; $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart09EC554:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$C0
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$30
    sta a:OBJ_Y_LO,x
    phx
    lda #$81
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart09EC554_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart09EC554_at0:
    plx
    rts

; @routine 09EC:599 SubStepPickupWait09EC599
; @run 70:BD99
; WHAT IT DOES: The pickup waits (Secret stage 05): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $BDAD; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait09EC599:
    lda #$BD
    sta z:zTemp11
    lda #$AD
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait09EC599_at0
    inc a:OBJ_STATE,x
SubStepPickupWait09EC599_at0:
    rts

; @routine 09EC:5B6 SubStepPickupFirstScripts09EC5B6
; @run 70:BDB6
; WHAT IT DOES: The pickup's first round (Secret stage 05): each time its frame list ends, script $83 plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts09EC5B6:
    lda #$BD
    sta z:zTemp11
    lda #$AD
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts09EC5B6_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$83
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts09EC5B6_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts09EC5B6_at0:
    rts

; @routine 09EC:5E6 SubStepPickupSecondScripts09EC5E6
; @run 70:BDE6
; WHAT IT DOES: The pickup's second round (Secret stage 05): every fourth frame script $89 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn09EC613.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts09EC5E6:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts09EC5E6_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$89
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts09EC5E6_at0
    jsr SubStepPickupSpawn09EC613
    stz z:zRasterEffect
    stz z:zRasterArg
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts09EC5E6_at0:
    rts

; @routine 09EC:613 SubStepPickupSpawn09EC613
; @run 70:BE13
; WHAT IT DOES: Starts up to eight objects of kind $32 in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn09EC613:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn09EC613_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn09EC613_at0
    lda #$32
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn09EC613_at1
SubStepPickupSpawn09EC613_at0:
    plx
    rts

; @routine 09EC:62D SubStepPickupFall09EC62D
; @run 70:BE2D
; WHAT IT DOES: The pickup falls (Secret stage 05): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall09EC62D:
    lda #$BE
    sta z:zTemp11
    lda #$60
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall09EC62D_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall09EC62D_at0:
    rts

; @routine 09EC:648 SubStepPickupRest09EC648
; @run 70:BE48
; WHAT IT DOES: The landed pickup (Secret stage 05): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest09EC648:
    lda #$BE
    sta z:zTemp11
    lda #$60
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest09EC648_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest09EC648_at0:
    rts

; @routine 09EC:69F SubStepPickupErase09EC69F
; @run 70:BE9F
; WHAT IT DOES: The pickup's last state (Secret stage 05): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns09ED123, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase09EC69F:
    jsr ErodePatterns09ED123
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase09EC69F_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase09EC69F_at0:
    rts

; @routine 09ED:123 ErodePatterns09ED123
; @run 71:C123
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern09ED12F, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns09ED123:
    phx
    clx
ErodePatterns09ED123_at0:
    jsr ErodePattern09ED12F
    inx
    cpx #$08
    bcc ErodePatterns09ED123_at0
    plx
    rts

; @routine 09ED:12F ErodePattern09ED12F
; @run 71:C12F
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $C140 into zBiosCl-Ch, then ErodeVramWords09ED150.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern09ED12FPatterns = $C140 ; the VRAM addresses of the eight patterns
ErodePattern09ED12F:
    txa
    asl a
    tay
    lda a:ErodePattern09ED12FPatterns,y
ErodePattern09ED12F_at0:
    sta z:zBiosCl
    lda a:ErodePattern09ED12FPatterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords09ED150
    rts

; @routine 09ED:150 ErodeVramWords09ED150
; @run 71:C150
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords09ED150:
    phy
    cly
ErodeVramWords09ED150_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords09ED150_at0
    ply
    rts

; @routine 0B4A:328 ErodePatterns0B4A328
; @run 70:AB28
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern0B4A334, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns0B4A328:
    phx
    clx
ErodePatterns0B4A328_at0:
    jsr ErodePattern0B4A334
    inx
    cpx #$08
    bcc ErodePatterns0B4A328_at0
    plx
    rts

; @routine 0B4A:334 ErodePattern0B4A334
; @run 70:AB34
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $AB45 into zBiosCl-Ch, then ErodeVramWords0B4A355.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern0B4A334Patterns = $AB45 ; the VRAM addresses of the eight patterns
ErodePattern0B4A334:
    txa
    asl a
    tay
    lda a:ErodePattern0B4A334Patterns,y
    sta z:zBiosCl
    lda a:ErodePattern0B4A334Patterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords0B4A355
    rts

; @routine 0B4A:355 ErodeVramWords0B4A355
; @run 70:AB55
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords0B4A355:
    phy
    cly
ErodeVramWords0B4A355_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords0B4A355_at0
    ply
    rts

; @routine 0B4B:11B SubStepPickup0B4B11B
; @run 70:B11B
; WHAT IT DOES: The pickup of Secret stage 06: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck0B4B13E, then JumpByStateTableBody with the table at $B130.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup0B4B11BStates = $B130 ; the handlers by state
SubStepPickup0B4B11B:
    jsr SubStepPickupHitCheck0B4B13E
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup0B4B11BStates,y
    sta z:zTemp00
    lda a:SubStepPickup0B4B11BStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4B:13E SubStepPickupHitCheck0B4B13E
; @run 70:B13E
; WHAT IT DOES: The pickup's hit (Secret stage 06): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck0B4B13E:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck0B4B13E_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck0B4B13E_at0
    cmp #$05
    bcs SubStepPickupHitCheck0B4B13E_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck0B4B13E_at0:
    rts

; @routine 0B4B:15E SubStepPickupStart0B4B15E
; @run 70:B15E
; WHAT IT DOES: Starts the pickup (Secret stage 06): its record, a timer of $C0, its place, list channel $82 and script
; $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart0B4B15E:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$C0
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$30
    sta a:OBJ_Y_LO,x
    phx
    lda #$82
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart0B4B15E_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart0B4B15E_at0:
    plx
    rts

; @routine 0B4B:1A3 SubStepPickupWait0B4B1A3
; @run 70:B1A3
; WHAT IT DOES: The pickup waits (Secret stage 06): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $B1B7; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait0B4B1A3:
    lda #$B1
    sta z:zTemp11
    lda #$B7
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait0B4B1A3_at0
    inc a:OBJ_STATE,x
SubStepPickupWait0B4B1A3_at0:
    rts

; @routine 0B4B:1C0 SubStepPickupFirstScripts0B4B1C0
; @run 70:B1C0
; WHAT IT DOES: The pickup's first round (Secret stage 06): each time its frame list ends, script $83 plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts0B4B1C0:
    lda #$B1
    sta z:zTemp11
    lda #$B7
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts0B4B1C0_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$83
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts0B4B1C0_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts0B4B1C0_at0:
    rts

; @routine 0B4B:1F0 SubStepPickupSecondScripts0B4B1F0
; @run 70:B1F0
; WHAT IT DOES: The pickup's second round (Secret stage 06): every fourth frame script $89 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn0B4B219.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts0B4B1F0:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts0B4B1F0_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$89
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts0B4B1F0_at0
    jsr SubStepPickupSpawn0B4B219
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts0B4B1F0_at0:
    rts

; @routine 0B4B:219 SubStepPickupSpawn0B4B219
; @run 70:B219
; WHAT IT DOES: Starts up to eight objects of kind $22 in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn0B4B219:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn0B4B219_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn0B4B219_at0
    lda #$22
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn0B4B219_at1
SubStepPickupSpawn0B4B219_at0:
    plx
    rts

; @routine 0B4B:233 SubStepPickupFall0B4B233
; @run 70:B233
; WHAT IT DOES: The pickup falls (Secret stage 06): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall0B4B233:
    lda #$B2
    sta z:zTemp11
    lda #$66
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall0B4B233_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall0B4B233_at0:
    rts

; @routine 0B4B:24E SubStepPickupRest0B4B24E
; @run 70:B24E
; WHAT IT DOES: The landed pickup (Secret stage 06): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest0B4B24E:
    lda #$B2
    sta z:zTemp11
    lda #$66
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest0B4B24E_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest0B4B24E_at0:
    rts

; @routine 0B4B:2A5 SubStepPickupErase0B4B2A5
; @run 70:B2A5
; WHAT IT DOES: The pickup's last state (Secret stage 06): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns0B4A328, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase0B4B2A5:
    jsr ErodePatterns0B4A328
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase0B4B2A5_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase0B4B2A5_at0:
    rts

; @routine 0B4D:19E ErodePattern0B4D19E
; @run 71:C19E
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $C1AF into zBiosCl-Ch, then ErodeVramWords0B4D1BD.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern0B4D19EPatterns = $C1AF ; the VRAM addresses of the eight patterns
ErodePattern0B4D19E:
    txa
    asl a
    tay
    lda a:ErodePattern0B4D19EPatterns,y
    sta z:zBiosCl
    lda a:ErodePattern0B4D19EPatterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords0B4D1BD
    rts

; @routine 0B4D:1BD ErodeVramWords0B4D1BD
; @run 71:C1BD
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords0B4D1BD:
    phy
    cly
ErodeVramWords0B4D1BD_at1:
    tya
    and #$03
    cmp z:zTemp00
    bne ErodeVramWords0B4D1BD_at0
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
ErodeVramWords0B4D1BD_at0:
    lda z:zBiosCl
    clc
    adc #$01
    sta z:zBiosCl
    lda z:zBiosCh
    adc #$00
    sta z:zBiosCh
    iny
    cpy #$40
    bcc ErodeVramWords0B4D1BD_at1
    ply
    rts

; @routine 0CAA:33A ErodePatterns0CAA33A
; @run 70:AB3A
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern0CAA346, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns0CAA33A:
    phx
    clx
ErodePatterns0CAA33A_at0:
    jsr ErodePattern0CAA346
    inx
    cpx #$08
    bcc ErodePatterns0CAA33A_at0
    plx
    rts

; @routine 0CAA:346 ErodePattern0CAA346
; @run 70:AB46
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $AB57 into zBiosCl-Ch, then ErodeVramWords0CAA367.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern0CAA346Patterns = $AB57 ; the VRAM addresses of the eight patterns
ErodePattern0CAA346:
    txa
    asl a
    tay
    lda a:ErodePattern0CAA346Patterns,y
    sta z:zBiosCl
    lda a:ErodePattern0CAA346Patterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords0CAA367
    rts

; @routine 0CAA:367 ErodeVramWords0CAA367
; @run 70:AB67
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords0CAA367:
    phy
    cly
ErodeVramWords0CAA367_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords0CAA367_at0
    ply
    rts

; @routine 0CAA:6DC SubStepPickup0CAA6DC
; @run 70:AEDC
; WHAT IT DOES: The pickup of Secret stage 07: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck0CAA6FF, then JumpByStateTableBody with the table at $AEF1.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup0CAA6DCStates = $AEF1 ; the handlers by state
SubStepPickup0CAA6DC:
    jsr SubStepPickupHitCheck0CAA6FF
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup0CAA6DCStates,y
    sta z:zTemp00
    lda a:SubStepPickup0CAA6DCStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAA:6FF SubStepPickupHitCheck0CAA6FF
; @run 70:AEFF
; WHAT IT DOES: The pickup's hit (Secret stage 07): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck0CAA6FF:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck0CAA6FF_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck0CAA6FF_at0
    cmp #$05
    bcs SubStepPickupHitCheck0CAA6FF_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck0CAA6FF_at0:
    rts

; @routine 0CAA:71F SubStepPickupStart0CAA71F
; @run 70:AF1F
; WHAT IT DOES: Starts the pickup (Secret stage 07): its record, a timer of $C0, its place, list channel $81 and script
; $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart0CAA71F:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$80
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$30
    sta a:OBJ_Y_LO,x
    phx
    lda #$81
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart0CAA71F_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart0CAA71F_at0:
    plx
    rts

; @routine 0CAA:764 SubStepPickupWait0CAA764
; @run 70:AF64
; WHAT IT DOES: The pickup waits (Secret stage 07): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $AF78; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait0CAA764:
    lda #$AF
    sta z:zTemp11
    lda #$78
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait0CAA764_at0
    inc a:OBJ_STATE,x
SubStepPickupWait0CAA764_at0:
    rts

; @routine 0CAA:781 SubStepPickupFirstScripts0CAA781
; @run 70:AF81
; WHAT IT DOES: The pickup's first round (Secret stage 07): each time its frame list ends, script $83 plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts0CAA781:
    lda #$AF
    sta z:zTemp11
    lda #$78
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts0CAA781_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$83
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts0CAA781_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts0CAA781_at0:
    rts

; @routine 0CAA:7B1 SubStepPickupSecondScripts0CAA7B1
; @run 70:AFB1
; WHAT IT DOES: The pickup's second round (Secret stage 07): every fourth frame script $89 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn0CAA7DA.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts0CAA7B1:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts0CAA7B1_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$89
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts0CAA7B1_at0
    jsr SubStepPickupSpawn0CAA7DA
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts0CAA7B1_at0:
    rts

; @routine 0CAA:7DA SubStepPickupSpawn0CAA7DA
; @run 70:AFDA
; WHAT IT DOES: Starts up to eight objects of kind $2F in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn0CAA7DA:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn0CAA7DA_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn0CAA7DA_at0
    lda #$2F
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn0CAA7DA_at1
SubStepPickupSpawn0CAA7DA_at0:
    plx
    rts

; @routine 0CAA:7F4 SubStepPickupFall0CAA7F4
; @run 70:AFF4
; WHAT IT DOES: The pickup falls (Secret stage 07): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall0CAA7F4:
    lda #$B0
    sta z:zTemp11
    lda #$27
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall0CAA7F4_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall0CAA7F4_at0:
    rts

; @routine 0CAB:00F SubStepPickupRest0CAB00F
; @run 70:B00F
; WHAT IT DOES: The landed pickup (Secret stage 07): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest0CAB00F:
    lda #$B0
    sta z:zTemp11
    lda #$27
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest0CAB00F_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest0CAB00F_at0:
    rts

; @routine 0CAB:066 SubStepPickupErase0CAB066
; @run 70:B066
; WHAT IT DOES: The pickup's last state (Secret stage 07): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns0CAA33A, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase0CAB066:
    jsr ErodePatterns0CAA33A
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase0CAB066_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase0CAB066_at0:
    rts

; @routine 0E0A:371 ErodePatterns0E0A371
; @run 70:AB71
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern0E0A37D, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns0E0A371:
    phx
    clx
ErodePatterns0E0A371_at0:
    jsr ErodePattern0E0A37D
    inx
    cpx #$08
    bcc ErodePatterns0E0A371_at0
    plx
    rts

; @routine 0E0A:37D ErodePattern0E0A37D
; @run 70:AB7D
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $AB8E into zBiosCl-Ch, then ErodeVramWords0E0A39E.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern0E0A37DPatterns = $AB8E ; the VRAM addresses of the eight patterns
ErodePattern0E0A37D:
    txa
    asl a
    tay
    lda a:ErodePattern0E0A37DPatterns,y
    sta z:zBiosCl
    lda a:ErodePattern0E0A37DPatterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords0E0A39E
    rts

; @routine 0E0A:39E ErodeVramWords0E0A39E
; @run 70:AB9E
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords0E0A39E:
    phy
    cly
ErodeVramWords0E0A39E_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords0E0A39E_at0
    ply
    rts

; @routine 0E0A:4A0 SubStepPickup0E0A4A0
; @run 70:ACA0
; WHAT IT DOES: The pickup of Secret stage 08: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck0E0A4C3, then JumpByStateTableBody with the table at $ACB5.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup0E0A4A0States = $ACB5 ; the handlers by state
SubStepPickup0E0A4A0:
    jsr SubStepPickupHitCheck0E0A4C3
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup0E0A4A0States,y
    sta z:zTemp00
    lda a:SubStepPickup0E0A4A0States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0E0A:4C3 SubStepPickupHitCheck0E0A4C3
; @run 70:ACC3
; WHAT IT DOES: The pickup's hit (Secret stage 08): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck0E0A4C3:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck0E0A4C3_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck0E0A4C3_at0
    cmp #$05
    bcs SubStepPickupHitCheck0E0A4C3_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck0E0A4C3_at0:
    rts

; @routine 0E0A:4E3 SubStepPickupStart0E0A4E3
; @run 70:ACE3
; WHAT IT DOES: Starts the pickup (Secret stage 08): its record, a timer of $C0, its place, list channel $81 and script
; $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart0E0A4E3:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$C0
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$30
    sta a:OBJ_Y_LO,x
    phx
    lda #$81
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart0E0A4E3_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart0E0A4E3_at0:
    plx
    rts

; @routine 0E0A:528 SubStepPickupWait0E0A528
; @run 70:AD28
; WHAT IT DOES: The pickup waits (Secret stage 08): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $AD3C; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait0E0A528:
    lda #$AD
    sta z:zTemp11
    lda #$3C
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait0E0A528_at0
    inc a:OBJ_STATE,x
SubStepPickupWait0E0A528_at0:
    rts

; @routine 0E0A:545 SubStepPickupFirstScripts0E0A545
; @run 70:AD45
; WHAT IT DOES: The pickup's first round (Secret stage 08): each time its frame list ends, script $83 plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts0E0A545:
    lda #$AD
    sta z:zTemp11
    lda #$3C
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts0E0A545_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$83
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts0E0A545_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts0E0A545_at0:
    rts

; @routine 0E0A:575 SubStepPickupSecondScripts0E0A575
; @run 70:AD75
; WHAT IT DOES: The pickup's second round (Secret stage 08): every fourth frame script $89 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn0E0A59E.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts0E0A575:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts0E0A575_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$89
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts0E0A575_at0
    jsr SubStepPickupSpawn0E0A59E
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts0E0A575_at0:
    rts

; @routine 0E0A:59E SubStepPickupSpawn0E0A59E
; @run 70:AD9E
; WHAT IT DOES: Starts up to eight objects of kind $22 in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn0E0A59E:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn0E0A59E_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn0E0A59E_at0
    lda #$22
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn0E0A59E_at1
SubStepPickupSpawn0E0A59E_at0:
    plx
    rts

; @routine 0E0A:5B8 SubStepPickupFall0E0A5B8
; @run 70:ADB8
; WHAT IT DOES: The pickup falls (Secret stage 08): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall0E0A5B8:
    lda #$AD
    sta z:zTemp11
    lda #$EB
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall0E0A5B8_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall0E0A5B8_at0:
    rts

; @routine 0E0A:5D3 SubStepPickupRest0E0A5D3
; @run 70:ADD3
; WHAT IT DOES: The landed pickup (Secret stage 08): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest0E0A5D3:
    lda #$AD
    sta z:zTemp11
    lda #$EB
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest0E0A5D3_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest0E0A5D3_at0:
    rts

; @routine 0E0A:62A SubStepPickupErase0E0A62A
; @run 70:AE2A
; WHAT IT DOES: The pickup's last state (Secret stage 08): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns0E0A371, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase0E0A62A:
    jsr ErodePatterns0E0A371
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase0E0A62A_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase0E0A62A_at0:
    rts

; @routine 0F6A:5CC SubStepPickup0F6A5CC
; @run 70:ADCC
; WHAT IT DOES: The pickup of Secret stage 09: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck0F6A5EF, then JumpByStateTableBody with the table at $ADE1.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup0F6A5CCStates = $ADE1 ; the handlers by state
SubStepPickup0F6A5CC:
    jsr SubStepPickupHitCheck0F6A5EF
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup0F6A5CCStates,y
    sta z:zTemp00
    lda a:SubStepPickup0F6A5CCStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0F6A:5EF SubStepPickupHitCheck0F6A5EF
; @run 70:ADEF
; WHAT IT DOES: The pickup's hit (Secret stage 09): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck0F6A5EF:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck0F6A5EF_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck0F6A5EF_at0
    cmp #$05
    bcs SubStepPickupHitCheck0F6A5EF_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck0F6A5EF_at0:
    rts

; @routine 0F6A:60F SubStepPickupStart0F6A60F
; @run 70:AE0F
; WHAT IT DOES: Starts the pickup (Secret stage 09): its record, a timer of $C0, its place, list channel $81 and script
; $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart0F6A60F:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$C0
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$30
    sta a:OBJ_Y_LO,x
    phx
    lda #$81
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$82
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart0F6A60F_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart0F6A60F_at0:
    plx
    rts

; @routine 0F6A:654 SubStepPickupWait0F6A654
; @run 70:AE54
; WHAT IT DOES: The pickup waits (Secret stage 09): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $AE68; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait0F6A654:
    lda #$AE
    sta z:zTemp11
    lda #$68
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait0F6A654_at0
    inc a:OBJ_STATE,x
SubStepPickupWait0F6A654_at0:
    rts

; @routine 0F6A:671 SubStepPickupFirstScripts0F6A671
; @run 70:AE71
; WHAT IT DOES: The pickup's first round (Secret stage 09): each time its frame list ends, script $83 plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts0F6A671:
    lda #$AE
    sta z:zTemp11
    lda #$68
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts0F6A671_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$83
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts0F6A671_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts0F6A671_at0:
    rts

; @routine 0F6A:6A1 SubStepPickupSecondScripts0F6A6A1
; @run 70:AEA1
; WHAT IT DOES: The pickup's second round (Secret stage 09): every fourth frame script $89 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn0F6A6CA.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts0F6A6A1:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts0F6A6A1_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$89
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts0F6A6A1_at0
    jsr SubStepPickupSpawn0F6A6CA
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts0F6A6A1_at0:
    rts

; @routine 0F6A:6CA SubStepPickupSpawn0F6A6CA
; @run 70:AECA
; WHAT IT DOES: Starts up to eight objects of kind $23 in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn0F6A6CA:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn0F6A6CA_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn0F6A6CA_at0
    lda #$23
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn0F6A6CA_at1
SubStepPickupSpawn0F6A6CA_at0:
    plx
    rts

; @routine 0F6A:6E4 SubStepPickupFall0F6A6E4
; @run 70:AEE4
; WHAT IT DOES: The pickup falls (Secret stage 09): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall0F6A6E4:
    lda #$AF
    sta z:zTemp11
    lda #$17
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall0F6A6E4_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall0F6A6E4_at0:
    rts

; @routine 0F6A:6FF SubStepPickupRest0F6A6FF
; @run 70:AEFF
; WHAT IT DOES: The landed pickup (Secret stage 09): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest0F6A6FF:
    lda #$AF
    sta z:zTemp11
    lda #$17
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest0F6A6FF_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest0F6A6FF_at0:
    rts

; @routine 0F6A:756 SubStepPickupErase0F6A756
; @run 70:AF56
; WHAT IT DOES: The pickup's last state (Secret stage 09): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns0F6D66D, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase0F6A756:
    jsr ErodePatterns0F6D66D
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase0F6A756_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase0F6A756_at0:
    rts

; @routine 0F6D:66D ErodePatterns0F6D66D
; @run 71:C66D
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern0F6D679, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns0F6D66D:
    phx
    clx
ErodePatterns0F6D66D_at0:
    jsr ErodePattern0F6D679
    inx
    cpx #$08
    bcc ErodePatterns0F6D66D_at0
    plx
    rts

; @routine 0F6D:679 ErodePattern0F6D679
; @run 71:C679
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $C68A into zBiosCl-Ch, then ErodeVramWords0F6D69A.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern0F6D679Patterns = $C68A ; the VRAM addresses of the eight patterns
ErodePattern0F6D679:
    txa
    asl a
    tay
    lda a:ErodePattern0F6D679Patterns,y
    sta z:zBiosCl
    lda a:ErodePattern0F6D679Patterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords0F6D69A
    rts

; @routine 0F6D:69A ErodeVramWords0F6D69A
; @run 71:C69A
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords0F6D69A:
    phy
    cly
ErodeVramWords0F6D69A_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords0F6D69A_at0
    ply
    rts

; @routine 10CA:2E7 ErodePatterns10CA2E7
; @run 70:AAE7
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern10CA2F3, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns10CA2E7:
    phx
    clx
ErodePatterns10CA2E7_at0:
    jsr ErodePattern10CA2F3
    inx
    cpx #$08
    bcc ErodePatterns10CA2E7_at0
    plx
    rts

; @routine 10CA:2F3 ErodePattern10CA2F3
; @run 70:AAF3
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $AB04 into zBiosCl-Ch, then ErodeVramWords10CA314.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern10CA2F3Patterns = $AB04 ; the VRAM addresses of the eight patterns
ErodePattern10CA2F3:
    txa
    asl a
    tay
    lda a:ErodePattern10CA2F3Patterns,y
    sta z:zBiosCl
    lda a:ErodePattern10CA2F3Patterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords10CA314
    rts

; @routine 10CA:314 ErodeVramWords10CA314
; @run 70:AB14
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords10CA314:
    phy
    cly
ErodeVramWords10CA314_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords10CA314_at0
    ply
    rts

; @routine 10CA:723 SubStepPickup10CA723
; @run 70:AF23
; WHAT IT DOES: The pickup of Secret stage 0A: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck10CA746, then JumpByStateTableBody with the table at $AF38.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup10CA723States = $AF38 ; the handlers by state
SubStepPickup10CA723:
    jsr SubStepPickupHitCheck10CA746
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup10CA723States,y
    sta z:zTemp00
    lda a:SubStepPickup10CA723States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 10CA:746 SubStepPickupHitCheck10CA746
; @run 70:AF46
; WHAT IT DOES: The pickup's hit (Secret stage 0A): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck10CA746:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck10CA746_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck10CA746_at0
    cmp #$05
    bcs SubStepPickupHitCheck10CA746_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck10CA746_at0:
    rts

; @routine 10CA:766 SubStepPickupStart10CA766
; @run 70:AF66
; WHAT IT DOES: Starts the pickup (Secret stage 0A): its record, a timer of $C0, its place, list channel $81 and script
; $A0, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart10CA766:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    lda #$70
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$04
    sbc a:ScrollXHiScreen
    sta a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$28
    sta a:OBJ_Y_LO,x
    phx
    lda #$81
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$A0
    ldx #$08
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart10CA766_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart10CA766_at0:
    plx
    rts

; @routine 10CA:7B0 SubStepPickupWait10CA7B0
; @run 70:AFB0
; WHAT IT DOES: The pickup waits (Secret stage 0A): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $AFC4; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait10CA7B0:
    lda #$AF
    sta z:zTemp11
    lda #$C4
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait10CA7B0_at0
    inc a:OBJ_STATE,x
SubStepPickupWait10CA7B0_at0:
    rts

; @routine 10CA:7CD SubStepPickupFirstScripts10CA7CD
; @run 70:AFCD
; WHAT IT DOES: The pickup's first round (Secret stage 0A): each time its frame list ends, script $A1 plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts10CA7CD:
    lda #$AF
    sta z:zTemp11
    lda #$C4
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts10CA7CD_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$A1
    ldx #$08
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts10CA7CD_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts10CA7CD_at0:
    rts

; @routine 10CA:7FD SubStepPickupSecondScripts10CA7FD
; @run 70:AFFD
; WHAT IT DOES: The pickup's second round (Secret stage 0A): every fourth frame script $A7 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn10CB026.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts10CA7FD:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts10CA7FD_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$A7
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts10CA7FD_at0
    jsr SubStepPickupSpawn10CB026
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts10CA7FD_at0:
    rts

; @routine 10CB:026 SubStepPickupSpawn10CB026
; @run 70:B026
; WHAT IT DOES: Starts up to eight objects of kind $2A in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn10CB026:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn10CB026_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn10CB026_at0
    lda #$2A
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn10CB026_at1
SubStepPickupSpawn10CB026_at0:
    plx
    rts

; @routine 10CB:040 SubStepPickupFall10CB040
; @run 70:B040
; WHAT IT DOES: The pickup falls (Secret stage 0A): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall10CB040:
    lda #$B0
    sta z:zTemp11
    lda #$73
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall10CB040_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall10CB040_at0:
    rts

; @routine 10CB:05B SubStepPickupRest10CB05B
; @run 70:B05B
; WHAT IT DOES: The landed pickup (Secret stage 0A): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest10CB05B:
    lda #$B0
    sta z:zTemp11
    lda #$73
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest10CB05B_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest10CB05B_at0:
    rts

; @routine 10CB:0B2 SubStepPickupErase10CB0B2
; @run 70:B0B2
; WHAT IT DOES: The pickup's last state (Secret stage 0A): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns10CA2E7, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase10CB0B2:
    jsr ErodePatterns10CA2E7
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase10CB0B2_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase10CB0B2_at0:
    rts

; @routine 13FA:6E6 SubStepPickup13FA6E6
; @run 70:AEE6
; WHAT IT DOES: The pickup of Secret stage 0C: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck13FA709, then JumpByStateTableBody with the table at $AEFB.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup13FA6E6States = $AEFB ; the handlers by state
SubStepPickup13FA6E6:
    jsr SubStepPickupHitCheck13FA709
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup13FA6E6States,y
    sta z:zTemp00
    lda a:SubStepPickup13FA6E6States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FA:709 SubStepPickupHitCheck13FA709
; @run 70:AF09
; WHAT IT DOES: The pickup's hit (Secret stage 0C): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck13FA709:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck13FA709_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck13FA709_at0
    cmp #$05
    bcs SubStepPickupHitCheck13FA709_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck13FA709_at0:
    rts

; @routine 13FA:729 SubStepPickupStart13FA729
; @run 70:AF29
; WHAT IT DOES: Starts the pickup (Secret stage 0C): its record, a timer of $C0, its place, list channel $85 and script
; $91, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart13FA729:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$C0
    sec
    sbc a:ScrollXLo
    sta a:OBJ_X_LO,x
    lda #$30
    sta a:OBJ_Y_LO,x
    phx
    lda #$85
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$91
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart13FA729_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart13FA729_at0:
    plx
    rts

; @routine 13FA:76E SubStepPickupWait13FA76E
; @run 70:AF6E
; WHAT IT DOES: The pickup waits (Secret stage 0C): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $AF82; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait13FA76E:
    lda #$AF
    sta z:zTemp11
    lda #$82
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait13FA76E_at0
    inc a:OBJ_STATE,x
SubStepPickupWait13FA76E_at0:
    rts

; @routine 13FA:78B SubStepPickupFirstScripts13FA78B
; @run 70:AF8B
; WHAT IT DOES: The pickup's first round (Secret stage 0C): each time its frame list ends, script $92 plus its count is
; started; after 6, the count is cleared, the frame set to $02 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts13FA78B:
    lda #$AF
    sta z:zTemp11
    lda #$82
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts13FA78B_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$92
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts13FA78B_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$02
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts13FA78B_at0:
    rts

; @routine 13FA:7BB SubStepPickupSecondScripts13FA7BB
; @run 70:AFBB
; WHAT IT DOES: The pickup's second round (Secret stage 0C): every fourth frame script $98 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn13FA7E8.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts13FA7BB:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts13FA7BB_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$98
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts13FA7BB_at0
    jsr SubStepPickupSpawn13FA7E8
    stz z:zRasterEffect
    stz z:zRasterArg
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts13FA7BB_at0:
    rts

; @routine 13FA:7E8 SubStepPickupSpawn13FA7E8
; @run 70:AFE8
; WHAT IT DOES: Starts up to eight objects of kind $44 in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn13FA7E8:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn13FA7E8_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn13FA7E8_at0
    lda #$44
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn13FA7E8_at1
SubStepPickupSpawn13FA7E8_at0:
    plx
    rts

; @routine 13FB:002 SubStepPickupFall13FB002
; @run 70:B002
; WHAT IT DOES: The pickup falls (Secret stage 0C): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall13FB002:
    lda #$B0
    sta z:zTemp11
    lda #$35
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall13FB002_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall13FB002_at0:
    rts

; @routine 13FB:01D SubStepPickupRest13FB01D
; @run 70:B01D
; WHAT IT DOES: The landed pickup (Secret stage 0C): its frames play; on frame $0E sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest13FB01D:
    lda #$B0
    sta z:zTemp11
    lda #$35
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0E
    bne SubStepPickupRest13FB01D_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest13FB01D_at0:
    rts

; @routine 13FB:074 SubStepPickupErase13FB074
; @run 70:B074
; WHAT IT DOES: The pickup's last state (Secret stage 0C): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns13FD5E8, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase13FB074:
    jsr ErodePatterns13FD5E8
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase13FB074_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase13FB074_at0:
    rts

; @routine 13FD:5E8 ErodePatterns13FD5E8
; @run 71:C5E8
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern13FD5F4, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns13FD5E8:
    phx
    clx
ErodePatterns13FD5E8_at0:
    jsr ErodePattern13FD5F4
    inx
    cpx #$08
    bcc ErodePatterns13FD5E8_at0
    plx
    rts

; @routine 13FD:5F4 ErodePattern13FD5F4
; @run 71:C5F4
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $C605 into zBiosCl-Ch, then ErodeVramWords13FD615.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern13FD5F4Patterns = $C605 ; the VRAM addresses of the eight patterns
ErodePattern13FD5F4:
    txa
    asl a
    tay
    lda a:ErodePattern13FD5F4Patterns,y
    sta z:zBiosCl
    lda a:ErodePattern13FD5F4Patterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords13FD615
    rts

; @routine 13FD:615 ErodeVramWords13FD615
; @run 71:C615
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords13FD615:
    phy
    cly
ErodeVramWords13FD615_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords13FD615_at0
    ply
    rts

; @routine 16CA:2AA ErodePatterns16CA2AA
; @run 70:AAAA
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern16CA2B6, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns16CA2AA:
    phx
    clx
ErodePatterns16CA2AA_at0:
    jsr ErodePattern16CA2B6
    inx
    cpx #$08
    bcc ErodePatterns16CA2AA_at0
    plx
    rts

; @routine 16CA:2B6 ErodePattern16CA2B6
; @run 70:AAB6
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $AAC7 into zBiosCl-Ch, then ErodeVramWords16CA2D7.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern16CA2B6Patterns = $AAC7 ; the VRAM addresses of the eight patterns
ErodePattern16CA2B6:
    txa
    asl a
    tay
    lda a:ErodePattern16CA2B6Patterns,y
    sta z:zBiosCl
    lda a:ErodePattern16CA2B6Patterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords16CA2D7
    rts

; @routine 16CA:2D7 ErodeVramWords16CA2D7
; @run 70:AAD7
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords16CA2D7:
    phy
    cly
ErodeVramWords16CA2D7_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords16CA2D7_at0
    ply
    rts

; @routine 16CA:6FC SubStepPickup16CA6FC
; @run 70:AEFC
; WHAT IT DOES: The pickup of Secret stage 0E: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck16CA71F, then JumpByStateTableBody with the table at $AF11.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup16CA6FCStates = $AF11 ; the handlers by state
SubStepPickup16CA6FC:
    jsr SubStepPickupHitCheck16CA71F
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup16CA6FCStates,y
    sta z:zTemp00
    lda a:SubStepPickup16CA6FCStates+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CA:71F SubStepPickupHitCheck16CA71F
; @run 70:AF1F
; WHAT IT DOES: The pickup's hit (Secret stage 0E): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck16CA71F:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck16CA71F_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck16CA71F_at0
    cmp #$05
    bcs SubStepPickupHitCheck16CA71F_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck16CA71F_at0:
    rts

; @routine 16CA:73F SubStepPickupStart16CA73F
; @run 70:AF3F
; WHAT IT DOES: Starts the pickup (Secret stage 0E): its record, a timer of $C0, its place, list channel $81 and script
; $87, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart16CA73F:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    lda #$18
    sta a:OBJ_Y_LO,x
    phx
    lda #$81
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$87
    ldx #$05
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart16CA73F_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart16CA73F_at0:
    plx
    rts

; @routine 16CA:780 SubStepPickupWait16CA780
; @run 70:AF80
; WHAT IT DOES: The pickup waits (Secret stage 0E): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $AF94; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait16CA780:
    lda #$AF
    sta z:zTemp11
    lda #$94
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait16CA780_at0
    inc a:OBJ_STATE,x
SubStepPickupWait16CA780_at0:
    rts

; @routine 16CA:79D SubStepPickupFirstScripts16CA79D
; @run 70:AF9D
; WHAT IT DOES: The pickup's first round (Secret stage 0E): each time its frame list ends, script $88 plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts16CA79D:
    lda #$AF
    sta z:zTemp11
    lda #$94
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts16CA79D_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$88
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts16CA79D_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts16CA79D_at0:
    rts

; @routine 16CA:7CD SubStepPickupSecondScripts16CA7CD
; @run 70:AFCD
; WHAT IT DOES: The pickup's second round (Secret stage 0E): every fourth frame script $8E plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn16CA7F6.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts16CA7CD:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts16CA7CD_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$8E
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts16CA7CD_at0
    jsr SubStepPickupSpawn16CA7F6
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts16CA7CD_at0:
    rts

; @routine 16CA:7F6 SubStepPickupSpawn16CA7F6
; @run 70:AFF6
; WHAT IT DOES: Starts up to eight objects of kind $2A in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn16CA7F6:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn16CA7F6_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn16CA7F6_at0
    lda #$2A
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn16CA7F6_at1
SubStepPickupSpawn16CA7F6_at0:
    plx
    rts

; @routine 16CB:010 SubStepPickupFall16CB010
; @run 70:B010
; WHAT IT DOES: The pickup falls (Secret stage 0E): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall16CB010:
    lda #$B0
    sta z:zTemp11
    lda #$43
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall16CB010_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall16CB010_at0:
    rts

; @routine 16CB:02B SubStepPickupRest16CB02B
; @run 70:B02B
; WHAT IT DOES: The landed pickup (Secret stage 0E): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest16CB02B:
    lda #$B0
    sta z:zTemp11
    lda #$43
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest16CB02B_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest16CB02B_at0:
    rts

; @routine 16CB:082 SubStepPickupErase16CB082
; @run 70:B082
; WHAT IT DOES: The pickup's last state (Secret stage 0E): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns16CA2AA, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase16CB082:
    jsr ErodePatterns16CA2AA
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase16CB082_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase16CB082_at0:
    rts

; @routine 18DA:272 ErodePatterns18DA272
; @run 70:AA72
; WHAT IT DOES: Erodes the eight patterns.
; HOW: X 0 to 7 through ErodePattern18DA27E, X kept on the stack.
; WAITS: no.
; Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
ErodePatterns18DA272:
    phx
    clx
ErodePatterns18DA272_at0:
    jsr ErodePattern18DA27E
    inx
    cpx #$08
    bcc ErodePatterns18DA272_at0
    plx
    rts

; @routine 18DA:27E ErodePattern18DA27E
; @run 70:AA7E
; WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
; HOW: The address from the table at $AA8F into zBiosCl-Ch, then ErodeVramWords18DA29F.
; WAITS: no.
; Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
ErodePattern18DA27EPatterns = $AA8F ; the VRAM addresses of the eight patterns
ErodePattern18DA27E:
    txa
    asl a
    tay
    lda a:ErodePattern18DA27EPatterns,y
    sta z:zBiosCl
    lda a:ErodePattern18DA27EPatterns+1,y
    sta z:zBiosCh
    jsr ErodeVramWords18DA29F
    rts

; @routine 18DA:29F ErodeVramWords18DA29F
; @run 70:AA9F
; WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
; its high byte right.
; HOW: For each word, the write and read addresses set to it, then read and written back.
; WAITS: no.
; Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
; N and Z from Y restored; X, Y and V kept.
ErodeVramWords18DA29F:
    phy
    cly
ErodeVramWords18DA29F_at0:
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
    asl a
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lsr a
    sta a:VDC_DATA_HI
    inc z:zBiosCl
    iny
    cpy #$40
    bcc ErodeVramWords18DA29F_at0
    ply
    rts

; @routine 18DA:704 SubStepPickup18DA704
; @run 70:AF04
; WHAT IT DOES: The pickup of Stage 10: the hit check, then the handler of its state (start, wait, two rounds of
; scripts, fall, rest, erase).
; HOW: SubStepPickupHitCheck18DA727, then JumpByStateTableBody with the table at $AF19.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
SubStepPickup18DA704States = $AF19 ; the handlers by state
SubStepPickup18DA704:
    jsr SubStepPickupHitCheck18DA727
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SubStepPickup18DA704States,y
    sta z:zTemp00
    lda a:SubStepPickup18DA704States+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DA:727 SubStepPickupHitCheck18DA727
; @run 70:AF27
; WHAT IT DOES: The pickup's hit (Stage 10): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
; to state 6 for 16 frames.
; HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
SubStepPickupHitCheck18DA727:
    lda a:OBJ_STATE,x
    cmp #$06
    bcs SubStepPickupHitCheck18DA727_at0
    lda a:OBJ_HIT_TAKEN,x
    beq SubStepPickupHitCheck18DA727_at0
    cmp #$05
    bcs SubStepPickupHitCheck18DA727_at0
    lda #$0B
    jsr VecSoundCommand
    lda #$10
    sta a:OBJ_PICKUP_TIMER,x
    lda #$06
    sta a:OBJ_STATE,x
SubStepPickupHitCheck18DA727_at0:
    rts

; @routine 18DA:747 SubStepPickupStart18DA747
; @run 70:AF47
; WHAT IT DOES: Starts the pickup (Stage 10): its record, a timer of $C0, its place, list channel $90 and script
; $89, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
; HOW: The calls through the kernel's vectors and cd_fade.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupStart18DA747:
    jsr VecLoadObjectRecord
    lda #$C0
    sta a:OBJ_PICKUP_TIMER,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_Y_HI,x
    lda #$80
    sta a:OBJ_X_LO,x
    lda #$18
    sta a:OBJ_Y_LO,x
    phx
    lda #$90
    ldx #$05
    jsr VecStartListChannelKind2
    lda #$89
    ldx #$08
    jsr VecStartScriptSlotKind1
    lda #$0C
    jsr BiosCdFade
    smb3 z:zField8E
    lda #$5C
    sta z:zFadeSound
    ldx a:TimeSlowMode
    bne SubStepPickupStart18DA747_at0
    jsr VecStopSound
    lda #$5C
    jsr VecSoundCommand
SubStepPickupStart18DA747_at0:
    plx
    rts

; @routine 18DA:788 SubStepPickupWait18DA788
; @run 70:AF88
; WHAT IT DOES: The pickup waits (Stage 10): its frames play while its timer counts down, then the next state.
; HOW: StepObjectFrames on the list at $AF9C; DEC; INC of the state at zero.
; WAITS: no.
; Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
SubStepPickupWait18DA788:
    lda #$AF
    sta z:zTemp11
    lda #$9C
    sta z:zTemp10
    jsr VecStepObjectFrames
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupWait18DA788_at0
    inc a:OBJ_STATE,x
SubStepPickupWait18DA788_at0:
    rts

; @routine 18DA:7A5 SubStepPickupFirstScripts18DA7A5
; @run 70:AFA5
; WHAT IT DOES: The pickup's first round (Stage 10): each time its frame list ends, script $8A plus its count is
; started; after 6, the count is cleared, the frame set to $01 and the next state.
; HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupFirstScripts18DA7A5:
    lda #$AF
    sta z:zTemp11
    lda #$9C
    sta z:zTemp10
    jsr VecStepObjectFrames
    bne SubStepPickupFirstScripts18DA7A5_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$8A
    ldx #$08
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$06
    bcc SubStepPickupFirstScripts18DA7A5_at0
    stz a:OBJ_PICKUP_COUNT,x
    lda #$01
    sta a:OBJ_FRAME,x
    inc a:OBJ_STATE,x
SubStepPickupFirstScripts18DA7A5_at0:
    rts

; @routine 18DA:7D5 SubStepPickupSecondScripts18DA7D5
; @run 70:AFD5
; WHAT IT DOES: The pickup's second round (Stage 10): every fourth frame script $90 plus its count; after 8,
; the pieces are spawned, the object made hittable and the next state.
; HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn18DA7FE.
; WAITS: no.
; Entry: X object. Exit: as the last call or compare; X kept.
SubStepPickupSecondScripts18DA7D5:
    lda z:zWaveCounter
    and #$03
    bne SubStepPickupSecondScripts18DA7D5_at0
    phx
    lda a:OBJ_PICKUP_COUNT,x
    clc
    adc #$90
    ldx #$05
    jsr VecStartScriptSlotKind1
    plx
    inc a:OBJ_PICKUP_COUNT,x
    lda a:OBJ_PICKUP_COUNT,x
    cmp #$08
    bcc SubStepPickupSecondScripts18DA7D5_at0
    jsr SubStepPickupSpawn18DA7FE
    lda #$01
    sta a:OBJ_HITBOX_ON,x
    inc a:OBJ_STATE,x
SubStepPickupSecondScripts18DA7D5_at0:
    rts

; @routine 18DA:7FE SubStepPickupSpawn18DA7FE
; @run 70:AFFE
; WHAT IT DOES: Starts up to eight objects of kind $31 in free slots from $20, each holding this object's slot.
; HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
; WAITS: no.
; Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
SubStepPickupSpawn18DA7FE:
    stx z:zTemp00
    phx
    clx
SubStepPickupSpawn18DA7FE_at1:
    jsr VecFindFreeSlotFrom20
    bcc SubStepPickupSpawn18DA7FE_at0
    lda #$31
    jsr VecStartObjectAtPlace
    lda z:zTemp00
    sta a:OBJ_PICKUP_TIMER,y
    inx
    cpx #$08
    bcc SubStepPickupSpawn18DA7FE_at1
SubStepPickupSpawn18DA7FE_at0:
    plx
    rts

; @routine 18DB:018 SubStepPickupFall18DB018
; @run 70:B018
; WHAT IT DOES: The pickup falls (Stage 10): its frames play; on landing sound $2C and the next state.
; HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
; WAITS: no.
; Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
SubStepPickupFall18DB018:
    lda #$B0
    sta z:zTemp11
    lda #$4B
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda #$00
    jsr VecStopOnLanding
    bcc SubStepPickupFall18DB018_at0
    lda #$2C
    jsr VecSoundCommand
    inc a:OBJ_STATE,x
SubStepPickupFall18DB018_at0:
    rts

; @routine 18DB:033 SubStepPickupRest18DB033
; @run 70:B033
; WHAT IT DOES: The landed pickup (Stage 10): its frames play; on frame $0D sound $51.
; HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
; WAITS: no.
; Entry: X object. Exit: as the compare or the sound.
SubStepPickupRest18DB033:
    lda #$B0
    sta z:zTemp11
    lda #$4B
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_FRAME,x
    cmp #$0D
    bne SubStepPickupRest18DB033_at0
    lda #$51
    jsr VecSoundCommand
SubStepPickupRest18DB033_at0:
    rts

; @routine 18DB:08A SubStepPickupErase18DB08A
; @run 70:B08A
; WHAT IT DOES: The pickup's last state (Stage 10): its graphics erode while its count runs; then the stage's
; sub-step becomes 6 and the object is cleared.
; HOW: ErodePatterns18DA272, DEC of the count, and a tail JMP to ClearObject.
; WAITS: no.
; Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
SubStepPickupErase18DB08A:
    jsr ErodePatterns18DA272
    dec a:OBJ_PICKUP_TIMER,x
    bne SubStepPickupErase18DB08A_at0
    lda #$06
    sta z:zSubStep
    jmp VecClearObject
SubStepPickupErase18DB08A_at0:
    rts
