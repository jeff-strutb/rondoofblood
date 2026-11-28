; @routine 152D:000 StageEntryA152D
; @run 6B:8000
; @unreachable 6B:8000-6B:8002 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8000 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8000: goes to StageSubModeStep152D, the stage's frame by sub-mode.
; HOW: One jump, no frame of its own.
; WAITS: as the sub-mode's handler.
; Entry, exit: as StageSubModeStep152D.
StageEntryA152D:
    jmp StageSubModeStep152D

; @routine 152D:003 StageEntryB152D
; @run 6B:8003
; @unreachable 6B:8003-6B:8005 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8003 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8003: goes to ContinueStep152D, the continue screen's frame by zGameSubMode.
; HOW: One jump, no frame of its own.
; WAITS: as the handler.
; Entry, exit: as ContinueStep152D.
StageEntryB152D:
    jmp ContinueStep152D

; @routine 152D:006 StageEntryC152D
; @run 6B:8006
; @unreachable 6B:8006-6B:8008 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8006 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8006: goes to ProbeTile152D, the tile probe at $96C6.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as ProbeTile152D.
StageEntryC152D:
    jmp ProbeTile152D

; @routine 152D:009 StageEntryD152D
; @run 6B:8009
; @unreachable 6B:8009-6B:800B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8009 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8009: goes to ClearBitRowsFour152D at $8E72.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as ClearBitRowsFour152D.
StageEntryD152D:
    jmp ClearBitRowsFour152D

; @routine 152D:00C StageSubModeStep152D
; @run 6B:800C
; @stop 8013
; @unreachable 6B:800C-6B:8012 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $800C runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The stage's frame: runs the handler of the sub-mode zGameSubMode holds, from the table of seven at
; $8013 (0 the start, 1 play, 2 the player's death, 3 the stage cleared, 4 the fade out, 5 and 6).
; HOW: The sub-mode doubled indexes the table, whose word is jumped through.
; WAITS: as the handler.
; Entry: nothing. Exit: as the handler leaves them; it is entered with A and X twice the sub-mode, N and Z from it,
; C the sub-mode's bit 7.
StageSubModeStep152D:
    lda z:zGameSubMode
    asl a
    tax
    jmp (OverlayObjectRecords+1,x)

; @routine 152D:034 StageSubModeStart152D
; @run 6B:8034
; @unreachable 6B:8034-6B:806E no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8034 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-mode 0, the stage's start: sets the stage up (the kernel's piece at $4006, the route's pieces, the
; row map, the screen, the overlay at $A000's start, the display, the opening, the level), clears the pause and the
; fade, and goes on to sub-mode 1; zField86 takes zField83. When zField8E's bit 0 is clear the stage goes straight to
; sub-mode 5 instead, with the count at $3310 one less; otherwise, unless zStage is negative, the kernel's piece at
; $407B runs last.
; HOW: Calls in turn $4006, StageRouteSetUpA152D, StageRouteSetUpB152D, StageLoadRowMap152D, StageResetScroll152D,
; MapOverlayBanks5And6, $A003, StageHudStart152D, StageOpeningStart152D and StageLevelStart152D.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit, sub-mode 5: A 5, N and Z from the count's decrement. Else: A zStage, N and Z from it, or as
; $407B leaves them; X, Y, C and V as the last piece left them.
StageSubModeStart152D:
    jsr VecResetGameState
    jsr StageRouteSetUpA152D
    jsr StageRouteSetUpB152D
    jsr StageLoadRowMap152D
    jsr StageResetScroll152D
    jsr $8021
    jsr RichterVecStart
    jsr StageHudStart152D
    jsr StageOpeningStart152D
    jsr StageLevelStart152D
    stz z:zPaused
    stz z:$3C
    inc z:zGameSubMode
    lda z:zField83
    sta z:zField86
    bbr0 zField8E,StageSubModeStart152D_at0
    lda z:zStage
    bmi StageSubModeStart152D_at1
    jsr VecCdPlayAgain
StageSubModeStart152D_at1:
    rts
StageSubModeStart152D_at0:
    lda #$05
    sta z:zGameSubMode
    dec a:$3310
    rts

; @routine 152D:06F StageFadeTimer152D
; @run 6B:806F
; @unreachable 6B:806F-6B:8099 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $806F runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Counts down the stage's fade at $9B by two a frame. On the step that reaches one it marks the fade as
; running ($3C 1) and, unless zField8E's bits 3 or 4 are set, starts the card's CD fade of type $0C; on the steps after
; it clears the mark and, unless bit 4 is set, sends the sound at $9D (bit 3 set) or the CD fade of type 0.
; HOW: Two decrements of $9B with a test after each; bit tests of zField8E; the sound hook at $401B or cd_fade, reached
; by a tail jump.
; WAITS: no.
; Entry: nothing. Exit: A the count first read (0: N and Z from it), or the value the last test loaded; N and Z from
; the last decrement or test; after the sound or the fade, as they leave them.
StageFadeTimer152D:
    lda z:$9B
    beq StageFadeTimer152D_at0
    dec z:$9B
    beq StageFadeTimer152D_at1
    dec z:$9B
    stz z:$3C
    bbs4 zField8E,StageFadeTimer152D_at0
    bbs3 zField8E,StageFadeTimer152D_at2
    cla
    jmp BiosCdFade
StageFadeTimer152D_at2:
    lda z:$9D
    jmp VecSoundCommand
StageFadeTimer152D_at1:
    lda #$01
    sta z:$3C
    lda z:zField8E
    and #$18
    bne StageFadeTimer152D_at0
    lda #$0C
    jmp BiosCdFade
StageFadeTimer152D_at0:
    rts

; @routine 152D:09A StagePauseToggle152D
; @run 6B:809A
; @unreachable 6B:809A-6B:80B0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $809A runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Pauses or unpauses the stage when the first pad's button 8 (Run) is pressed, unless the fade is
; running or zPaused's bit 1 holds the pause.
; HOW: $3C XOR 1 must not be zero; BiosJoyTrigger's bit 3; zPaused's bit 1 clear; zPaused XOR 3.
; WAITS: no.
; Entry: nothing. Exit: A the last value loaded or computed, N and Z from it (the bit test leaves A the pad's bit).
; X, Y, C and V kept.
StagePauseToggle152D:
    lda z:$3C
    eor #$01
    beq StagePauseToggle152D_at0
    lda a:BiosJoyTrigger
    and #$08
    beq StagePauseToggle152D_at0
    bbs1 zPaused,StagePauseToggle152D_at0
    lda z:zPaused
    eor #$03
    sta z:zPaused
StagePauseToggle152D_at0:
    rts

; @routine 152D:0B1 StageModeEventsStep152D
; @run 6B:80B1
; @unreachable 6B:80B1-6B:80D6 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $80B1 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Counts the wave and steps zModeEvents: $11 added, wrapped below $50 by its low nibble; a low pair that
; reaches 3 carries into the next pair, and a second pair that reaches 3 clears the low nibble.
; HOW: An add and compares on the byte, written back after each change.
; WAITS: no.
; Entry: nothing. Exit: A the last value masked, N and Z from the last compare or mask; C from the last compare; V
; from the add. X and Y kept.
StageModeEventsStep152D:
    inc z:zWaveCounter
    lda z:zModeEvents
    clc
    adc #$11
    cmp #$50
    bcc StageModeEventsStep152D_at0
    and #$0F
StageModeEventsStep152D_at0:
    sta z:zModeEvents
    and #$03
    cmp #$03
    bne StageModeEventsStep152D_at1
    inc z:zModeEvents
    lda z:zModeEvents
    and #$0C
    cmp #$0C
    bne StageModeEventsStep152D_at1
    lda z:zModeEvents
    and #$F0
    sta z:zModeEvents
StageModeEventsStep152D_at1:
    rts

; @routine 152D:0D7 StageSubModePlay152D
; @run 6B:80D7
; @stop 8112
; @unreachable 6B:80D7-6B:8111 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $80D7 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-mode 1, the stage in play: the fade and the pause; paused, the paused frame (StagePausedFrame152D)
; runs instead. Else the events step and the overlay at $A000's frame; unless a sub-step is asked for, the overlay's
; pieces, the overlays' frame, the objects' hits and the display; then the sub-step zSubStep held (cleared) runs from
; the table at $8112.
; HOW: Calls StageFadeTimer152D and StagePauseToggle152D; jumps to StagePausedFrame152D; calls StageModeEventsStep152D,
; MapOverlayBanks5And6, $A000, then $A006, MapOverlayBank7, $A08D, MapOverlayBanks5And6, $A009, StageOverlayFrame152D,
; StageObjectHits152D and StageHudStep152D; the sub-step doubled indexes the table.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit: as the sub-step's handler leaves them; it is entered with A and X twice the sub-step, N and Z
; from it, C its bit 7. Paused: as StagePausedFrame152D.
StageSubModePlay152D:
    jsr StageFadeTimer152D
    jsr StagePauseToggle152D
    lda z:zPaused
    beq StageSubModePlay152D_at0
    jmp StagePausedFrame152D
StageSubModePlay152D_at0:
    jsr StageModeEventsStep152D
    jsr $8021
    jsr RunObjects1619
    lda z:zSubStep
    bne StageSubModePlay152D_at1
    jsr RichterVecApplyMove
    jsr $802C
    jsr $A08D
    jsr $8021
    jsr RichterVecC3A7
    jsr StageOverlayFrame152D
    jsr StageObjectHits152D
    jsr StageHudStep152D
StageSubModePlay152D_at1:
    lda z:zSubStep
    stz z:zSubStep
    asl a
    tax
    jmp ($8112,x)

; @routine 152D:120 StageSubStepNone152D
; @run 6B:8120
; @unreachable 6B:8120-6B:8120 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8120 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-step 0: nothing.
; HOW: RTS.
; WAITS: no.
; Entry: nothing. Exit: everything kept.
StageSubStepNone152D:
    rts

; @routine 152D:121 StageSubStepNoneB152D
; @run 6B:8121
; @unreachable 6B:8121-6B:8121 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8121 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-step 1: nothing.
; HOW: RTS.
; WAITS: no.
; Entry: nothing. Exit: everything kept.
StageSubStepNoneB152D:
    rts

; @routine 152D:122 StageSubStepDeath152D
; @run 6B:8122
; @unreachable 6B:8122-6B:8147 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8122 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-step 2, the player died: sub-mode 2 with zField80, zField89 and the fall's step cleared, the
; overlay at $A000's piece at $A067, the forced presses cleared; when PlayerByte3BD8 is set it is cleared and the fade
; starts again from 2.
; HOW: Stores; MapOverlayBank7 and $A067; a jump into StageFadeTimer152D.
; WAITS: no.
; Entry: nothing. Exit: A PlayerByte3BD8 (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer152D.
StageSubStepDeath152D:
    lda #$02
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jsr $802C
    jsr ClearScrollSpeeds
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz z:$88
    lda a:PlayerByte3BD8
    bne StageSubStepDeath152D_at0
    rts
StageSubStepDeath152D_at0:
    stz a:PlayerByte3BD8
    lda #$02
    sta z:$9B
    jmp StageFadeTimer152D

; @routine 152D:148 StageSubStepCleared152D
; @run 6B:8148
; @unreachable 6B:8148-6B:817A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8148 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-step 3, the stage cleared: sub-mode 3 with its step 0, its table at $84 picked by zField8C (none:
; 2; 3 or 4: 3; else 1, with $2743 set to 1); the overlay at $A000's piece at $A067, the forced presses and
; PlayerByte3BD8 cleared.
; HOW: Compares of zField8C; stores; MapOverlayBank7 and $A067.
; WAITS: no.
; Entry: nothing. Exit: as $A067 left them.
StageSubStepCleared152D:
    lda z:zField8C
    beq StageSubStepCleared152D_at0
    ldx #$01
    stx a:Field2742+1
    cmp #$03
    beq StageSubStepCleared152D_at1
    cmp #$04
    beq StageSubStepCleared152D_at1
    lda #$01
    bra StageSubStepCleared152D_at2
StageSubStepCleared152D_at1:
    lda #$03
    bra StageSubStepCleared152D_at2
StageSubStepCleared152D_at0:
    lda #$02
StageSubStepCleared152D_at2:
    sta z:$84
    lda #$03
    sta z:zGameSubMode
    stz z:zField85
    jsr $802C
    jsr ClearScrollSpeeds
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz a:PlayerByte3BD8
    rts

; @routine 152D:17B StageSubStepFadeOut152D
; @run 6B:817B
; @unreachable 6B:817B-6B:8189 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $817B runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-step 4: sub-mode 4 with zField80 and zField89 cleared, and the overlay at $A000's piece at $A067.
; HOW: Stores; MapOverlayBank7 and $A067.
; WAITS: no.
; Entry: nothing. Exit: as $A067 left them.
StageSubStepFadeOut152D:
    lda #$04
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jsr $802C
    jsr ClearScrollSpeeds
    rts

; @routine 152D:18A StageSubStepFadeOutB152D
; @run 6B:818A
; @unreachable 6B:818A-6B:819A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $818A runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-step 5: sub-mode 4 entered at its step 5 (zField80 5, zField89 cleared), and the overlay at $A000's
; piece at $A067.
; HOW: Stores; MapOverlayBank7 and $A067.
; WAITS: no.
; Entry: nothing. Exit: as $A067 left them.
StageSubStepFadeOutB152D:
    lda #$04
    sta z:zGameSubMode
    lda #$05
    sta z:zField80
    stz z:zField89
    jsr $802C
    jsr ClearScrollSpeeds
    rts

; @routine 152D:19B StageSubStepBoss152D
; @run 6B:819B
; @unreachable 6B:819B-6B:81C8 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $819B runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-step 6: sub-mode 6 with zField80, zField89 and the fall's step cleared, the overlay at $A000's
; piece at $A067, the forced presses cleared, the player's step at $3358 0 and its count at $3310 $40; when
; PlayerByte3BD8 is set it is cleared and the fade starts again from 2.
; HOW: Stores; MapOverlayBank7 and $A067; a jump into StageFadeTimer152D.
; WAITS: no.
; Entry: nothing. Exit: A PlayerByte3BD8 (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer152D.
StageSubStepBoss152D:
    lda #$06
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jsr $802C
    jsr ClearScrollSpeeds
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz z:$88
    stz a:OBJ_3358
    lda #$40
    sta a:$3310
    lda a:PlayerByte3BD8
    bne StageSubStepBoss152D_at0
    rts
StageSubStepBoss152D_at0:
    stz a:PlayerByte3BD8
    lda #$02
    sta z:$9B
    jmp StageFadeTimer152D

; @routine 152D:1C9 StageSubModeDeath152D
; @run 6B:81C9
; @unreachable 6B:81C9-6B:81E0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $81C9 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-mode 2, the player's death: sound command $FF, then the death's phase; once the phase has put the
; stage back in sub-mode 1, the overlay at $A000's piece at $A015 and the level's start.
; HOW: The sound hook at $401B; StageDeathPhase152D; a compare of zGameSubMode; MapOverlayBanks5And6, $A015 and a jump
; to StageLevelStart152D.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit: A zGameSubMode, N, Z and C from its compare with 1; else as StageLevelStart152D.
StageSubModeDeath152D:
    lda #$FF
    jsr VecSoundCommand
    jsr StageDeathPhase152D
    lda z:zGameSubMode
    cmp #$01
    beq StageSubModeDeath152D_at0
    rts
StageSubModeDeath152D_at0:
    jsr $8021
    jsr RichterVecCE1D
    jmp StageLevelStart152D

; @routine 152D:1E1 StageDeathPhase152D
; @run 6B:81E1
; @stop 81E8
; @unreachable 6B:81E1-6B:81E7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $81E1 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The death's phase by zField87, from the table of four at $81E8.
; HOW: zField87 doubled indexes the table, whose word is jumped through.
; WAITS: as the phase.
; Entry: nothing. Exit: as the phase leaves them; it is entered with A and X twice zField87, N and Z from it, C its
; bit 7.
StageDeathPhase152D:
    lda z:zField87
    asl a
    tax
    jmp ($81E8,x)

; @routine 152D:1F0 StageDeathRespawn152D
; @run 6B:81F0
; @unreachable 6B:81F0-6B:8208 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $81F0 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Phase 0 of the death: the player put back and the stage in play again (sub-mode 1), the player's count
; at $3310 $18 and its step at $3358 from the two at $8209 by the facing bit of the byte at $2818.
; HOW: StageResetScroll152D; stores; one table read.
; WAITS: no.
; Entry: nothing. Exit: A the step, X the facing bit, N and Z from A; Y, C and V as StageResetScroll152D left them.
StageDeathRespawn152D:
    jsr StageResetScroll152D
    lda #$01
    sta z:zGameSubMode
    lda #$18
    sta a:$3310
    lda a:$2818
    and #$01
    tax
    lda a:$8209,x
    sta a:OBJ_3358
    rts

; @routine 152D:20B StageDeathRespawnB152D
; @run 6B:820B
; @unreachable 6B:820B-6B:8223 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $820B runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Phase 1 of the death: as phase 0 (the same bytes, the same table at $8209).
; HOW: StageResetScroll152D; stores; one table read.
; WAITS: no.
; Entry: nothing. Exit: as StageDeathRespawn152D.
StageDeathRespawnB152D:
    jsr StageResetScroll152D
    lda #$01
    sta z:zGameSubMode
    lda #$18
    sta a:$3310
    lda a:$2818
    and #$01
    tax
    lda a:$8209,x
    sta a:OBJ_3358
    rts

; @routine 152D:224 StageDeathFallLeft152D
; @run 6B:8224
; @unreachable 6B:8224-6B:8235 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8224 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Phase 2 of the death, the fall to the left, by the fall's step at $88: step 0 sets the player's Y
; ($2D28 $40, $2CE0 1) and runs on into StageDeathFallStart152D; step 1 moves the player while it is shown and left
; of X $10, else ends through $A01E; step 2 puts the player back (StageResetScroll152D, sub-mode 1, the count at $3310
; $20, the step at $3358 $20).
; HOW: Two decrements of the step's copy in A; the pieces of the overlay at $A000 at $A01B, $A012 and $A01E.
; WAITS: no.
; Entry: nothing. Exit, step 0: as StageDeathFallStart152D. Step 1, the step counted: A the player's X, N and Z from
; the increment, C from the compare; else as $A01E leaves them. Step 2: A $20, N and Z from it, the rest as
; StageResetScroll152D left them.
StageDeathFallLeft152D:
    lda z:$88
    dec a
    beq StageDeathFallStart152D_at2
    dec a
    beq StageDeathFallBackLeft152D
    lda #$40
    sta a:OBJ_Y_FRAC
    lda #$01
    sta a:$2CE0

; @routine 152D:236 StageDeathFallStart152D
; @run 6B:8236
; @unreachable 6B:8236-6B:825E no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8236 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The fall's first step: the overlay at $A000's pieces at $A018 (with A 0) and $A012, and the fall's
; step at $88 counted.
; HOW: MapOverlayBanks5And6, $A018, $A012; an increment of $88.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the step's increment; A, X, Y, C and V as $A012 left them.
StageDeathFallStart152D:
    jsr $8021
    lda #$00
    jsr RichterVecSetAnimation
    jsr RichterVecAnimationSound
    bra StageDeathFallStart152D_at0
StageDeathFallStart152D_at2:
    jsr $8021
    jsr RichterVecSetIdle
    jsr RichterVecAnimationSound
    lda a:$2B30
    beq StageDeathFallStart152D_at1
    lda a:OBJ_X_HI+8
    cmp #$10
    bcs StageDeathFallStart152D_at0
StageDeathFallStart152D_at1:
    clx
    jmp RichterVecCE73
StageDeathFallStart152D_at0:
    inc z:$88
    rts

; @routine 152D:25F StageDeathFallBackLeft152D
; @run 6B:825F
; @unreachable 6B:825F-6B:8270 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $825F runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The left fall's last step, reached from StageDeathFallLeft152D: the screen and the player put back
; (StageResetScroll152D), sub-mode 1, the count at $3310 $20 and the step at $3358 $20.
; HOW: StageResetScroll152D; stores.
; WAITS: no.
; Entry: nothing. Exit: A $20, N and Z from it; X, Y, C and V as StageResetScroll152D left them.
StageDeathFallBackLeft152D:
    jsr StageResetScroll152D
    lda #$01
    sta z:zGameSubMode
    lda #$20
    sta a:$3310
    lda #$20
    sta a:OBJ_3358
    rts

; @routine 152D:271 StageDeathFallRight152D
; @run 6B:8271
; @unreachable 6B:8271-6B:82B3 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8271 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Phase 3 of the death, the fall to the right, by the fall's step at $88: step 0 sets the player's Y
; ($2D28 $C0, $2CE0 $FE) and calls StageDeathFallStart152D, then moves the player as step 1 does; step 1 moves the
; player while it is shown and left of X $F0, else ends through $A01E; step 2 puts the player back
; (StageResetScroll152D, sub-mode 1, the count at $3310 $20, the step at $3358 $80).
; HOW: Two decrements of the step's copy in A; StageDeathFallStart152D; the pieces of the overlay at $A000 at $A01B,
; $A012 and $A01E.
; WAITS: no.
; Entry: nothing. Exit, steps 0 and 1, the step counted: A the player's X, N and Z from the increment, C from the
; compare; else as $A01E leaves them. Step 2: A $80, N and Z from it, the rest as StageResetScroll152D left them.
StageDeathFallRight152D:
    lda z:$88
    dec a
    beq StageDeathFallRight152D_at0
    dec a
    beq StageDeathFallRight152D_at1
    lda #$C0
    sta a:OBJ_Y_FRAC
    lda #$FE
    sta a:$2CE0
    jsr StageDeathFallStart152D
StageDeathFallRight152D_at0:
    jsr $8021
    jsr RichterVecSetIdle
    jsr RichterVecAnimationSound
    lda a:$2B30
    beq StageDeathFallRight152D_at2
    lda a:OBJ_X_HI+8
    cmp #$F0
    bcc StageDeathFallRight152D_at3
StageDeathFallRight152D_at2:
    clx
    jmp RichterVecCE73
StageDeathFallRight152D_at3:
    inc z:$88
    rts
StageDeathFallRight152D_at1:
    jsr StageResetScroll152D
    lda #$01
    sta z:zGameSubMode
    lda #$20
    sta a:$3310
    lda #$80
    sta a:OBJ_3358
    rts

; @routine 152D:2B4 StageSubModeCleared152D
; @run 6B:82B4
; @stop 82BE
; @unreachable 6B:82B4-6B:82BD no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $82B4 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-mode 3, the stage cleared: the events step, then the step of the table $84 picked (the next stage's
; start, the walk out, the boss's walk out, the walk out after the boss) from the four at $82BE.
; HOW: StageModeEventsStep152D; $84 doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $84, N and Z from it, C its bit 7.
StageSubModeCleared152D:
    jsr StageModeEventsStep152D
    lda z:$84
    asl a
    tax
    jmp ($82BE,x)

; @routine 152D:2C6 StageClearedNextStage152D
; @run 6B:82C6
; @unreachable 6B:82C6-6B:82EF no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $82C6 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The cleared stage's first table: the next stage set up as the start does (the kernel's piece at $4009,
; the route's pieces, the row map, the screen, the display, zField8A 2, the overlay at $A000's start, the level) and
; put in play, sub-mode 1,
; unpaused, zField86 zField83.
; HOW: Calls in turn $4009, StageRouteSetUpA152D, StageRouteSetUpB152D, StageLoadRowMap152D, StageResetScroll152D,
; StageHudStart152D, MapOverlayBanks5And6, $A003 and StageLevelStart152D.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit: A zField83, N and Z from it; X, Y, C and V as StageLevelStart152D left them.
StageClearedNextStage152D:
    jsr VecClearScreenState
    jsr StageRouteSetUpA152D
    jsr StageRouteSetUpB152D
    jsr StageLoadRowMap152D
    jsr StageResetScroll152D
    jsr StageHudStart152D
    lda #$02
    sta z:zField8A
    jsr $8021
    jsr RichterVecStart
    jsr StageLevelStart152D
    stz z:zPaused
    lda #$01
    sta z:zGameSubMode
    lda z:zField83
    sta z:zField86
    rts

; @routine 152D:2FC StageClearedWalkOut152D
; @run 6B:82FC
; @stop 8303
; @unreachable 6B:82FC-6B:8302 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $82FC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The cleared stage's second table: its step at $85 from the five at $8303 (the wait for the objects,
; the walk, the scroll's wait, the walk back, the walk on to the next stage).
; HOW: $85 doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $85, N and Z from it, C its bit 7.
StageClearedWalkOut152D:
    lda z:zField85
    asl a
    tax
    jmp ($8303,x)

; @routine 152D:30D StageClearedWaitObjects152D
; @run 6B:830D
; @unreachable 6B:830D-6B:832D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $830D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A step of the cleared stage: the overlay at $A000's pieces at $A009 and $A08D and the scroll, then,
; once no object slot from $40 down to 1 holds a state from 1 to $7F in the column at $27D0, $2743 cleared and the
; step counted.
; HOW: MapOverlayBanks5And6, $A009, MapOverlayBank7, $A08D, StageOverlayFrame152D; a loop down X; a store and an
; increment.
; WAITS: no.
; Entry: nothing. Exit, an object still busy: A its state, X its slot, N and Z from A. Else: X 0, N and Z from the
; step's increment; A, Y, C and V as StageOverlayFrame152D left them.
StageClearedWaitObjects152D:
    jsr $8021
    jsr RichterVecC3A7
    jsr $802C
    jsr $A08D
    jsr StageOverlayFrame152D
    ldx #$40
StageClearedWaitObjects152D_at1:
    lda a:$27D0,x
    bmi StageClearedWaitObjects152D_at0
    bne StageClearedWaitScroll152D_at0
StageClearedWaitObjects152D_at0:
    dex
    bne StageClearedWaitObjects152D_at1
    stz a:Field2742+1
    inc z:zField85
    rts

; @routine 152D:32E StageClearedWalk152D
; @run 6B:832E
; @unreachable 6B:832E-6B:8370 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $832E runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A step of the cleared stage: the player walks the scroll ($3E74 the step from $82F0 by the side,
; $3E6C and $3E00 cleared, its X moved by $82F2); when its X reaches the side's end ($82F4) the overlay at $A000's
; piece at $A067 runs, $3E74 is cleared and the step counted.
; HOW: The side from zField8C; table reads, an add; MapOverlayBank7, $A08D, StageOverlayFrame152D, $A321; a compare.
; WAITS: no.
; Entry: nothing. Exit, short of the end: A the X, N, Z and C from the compare, Y the side. At the end: N and Z from
; the step's increment, the rest as $A067 left them.
StageClearedWalk152D:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$82F0,y
    sta a:ScrollStepYLo+6
    stz a:Scroll3E66+6
    stz a:ScrollXFrac
    lda a:OBJ_X_HI+8
    clc
    adc a:$82F2,y
    sta a:OBJ_X_HI+8
    jsr $802C
    jsr $A08D
    jsr StageOverlayFrame152D
    jsr $A321
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_HI+8
    cmp a:$82F4,y
    beq StageClearedWalk152D_at0
    rts
StageClearedWalk152D_at0:
    jsr $802C
    jsr ClearScrollSpeeds
    stz a:ScrollStepYLo+6
    inc z:zField85
    rts

; @routine 152D:371 StageClearedWaitScroll152D
; @run 6B:8371
; @unreachable 6B:8371-6B:837D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8371 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A step of the cleared stage: the scroll and the overlay at $A000's piece at $A321; once zField8C is
; negative the step is counted.
; HOW: StageOverlayFrame152D, $A321; a test of zField8C; an increment.
; WAITS: no.
; Entry: nothing. Exit: A zField8C, N and Z from it, or from the step's increment; X, Y, C and V as $A321 left them.
StageClearedWaitScroll152D:
    jsr StageOverlayFrame152D
    jsr $A321
    lda z:zField8C
    bpl StageClearedWaitScroll152D_at0
    inc z:zField85
StageClearedWaitScroll152D_at0:
    rts

; @routine 152D:37E StageClearedWalkBack152D
; @run 6B:837E
; @unreachable 6B:837E-6B:83C0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $837E runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A step of the cleared stage: the player walks back (its count at $3310 $E0, its step at $3358 from
; $82F8 by the side, the overlay at $A000's frame at $A000, its X moved back by $82F2); when the X reaches the side's
; mark ($82F6) zField8C loses bit 7, the player stops, zField8A is 2, the overlay's piece at $A003 runs and the step is
; counted.
; HOW: The side from zField8C; table reads; MapOverlayBanks5And6, $A000; a subtract and a compare; stores, $A003.
; WAITS: no.
; Entry: nothing. Exit, short of the mark: A the X, N, Z and C from the compare, V from the subtract, Y the side. At
; the mark: N and Z from the step's increment, the rest as $A003 left them.
StageClearedWalkBack152D:
    lda #$E0
    sta a:$3310
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$82F8,y
    sta a:OBJ_3358
    jsr $8021
    jsr RunObjects1619
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_HI+8
    sec
    sbc a:$82F2,y
    sta a:OBJ_X_HI+8
    cmp a:$82F6,y
    beq StageClearedWalkBack152D_at0
    rts
StageClearedWalkBack152D_at0:
    lda z:zField8C
    and #$7F
    sta z:zField8C
    stz a:$3310
    stz a:OBJ_3358
    lda #$02
    sta z:zField8A
    jsr RichterVecStart
    inc z:zField85
    rts

; @routine 152D:3C1 StageClearedWalkOn152D
; @run 6B:83C1
; @unreachable 6B:83C1-6B:8435 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $83C1 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The cleared stage's last step: the player walks on with the scroll until its X low byte at $3E08 is 0;
; then the next stage: below zField8C 5 the next of the route's stages from the table at $9A5F (zField82 counted, into
; zField83 and zField86), else zField83 counted; zField8E's bit 1 set; the stage's tables, rows and objects set up
; again (the first and the last only below 5), the scroll's bytes at $3E00 and $3E54 cleared from KernelZeros, the
; screen loaded; zField8C and the pause cleared and the stage in play, sub-mode 1.
; HOW: The walk's push and frame as StageClearedWalk152D; StageRouteSetUpA152D, StageRouteSetUpB152D,
; StageLoadRowMap152D, two TAI, StageLoadScreen152D, StageLevelStart152D.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit, walking: A $3E08, N and Z from it, the rest as $A321 left them. Else: A 1, N and Z from it;
; X, Y, C and V as the last piece left them, C from the compare when StageLevelStart152D did not run.
StageClearedWalkOn152D:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$82F0,y
    sta a:ScrollStepYLo+6
    stz a:Scroll3E66+6
    lda a:OBJ_X_HI+8
    clc
    adc a:$82F2,y
    sta a:OBJ_X_HI+8
    jsr $802C
    jsr $A08D
    jsr StageOverlayFrame152D
    jsr $A321
    lda a:ScrollXLo
    beq StageClearedWalkOn152D_at0
    rts
StageClearedWalkOn152D_at0:
    lda z:zField8C
    cmp #$05
    bcs StageClearedWalkOn152D_at1
    inc z:zField82
    lda z:zField82
    tay
    lda a:$9A5F,y
    sta z:zField83
    sta z:zField86
    bra StageClearedWalkOn152D_at2
StageClearedWalkOn152D_at1:
    inc z:zField83
StageClearedWalkOn152D_at2:
    smb1 z:zField8E
    lda z:zField8C
    cmp #$05
    bcs StageClearedWalkOn152D_at3
    jsr StageRouteSetUpA152D
StageClearedWalkOn152D_at3:
    jsr StageRouteSetUpB152D
    jsr StageLoadRowMap152D
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollBgyLo,$0078
    jsr StageLoadScreen152D
    lda z:zField8C
    cmp #$05
    bcs StageClearedWalkOn152D_at4
    jsr StageLevelStart152D
StageClearedWalkOn152D_at4:
    stz z:zField8C
    stz z:zPaused
    lda #$01
    sta z:zGameSubMode
    rts

; @routine 152D:436 StageClearedBossWalk152D
; @run 6B:8436
; @stop 8440
; @unreachable 6B:8436-6B:843F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8436 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The cleared stage's third table, after a boss: the fade's countdown, then its step at $85 from the
; three at $8440 (the scroll to the boss's door, the wait for the door, the walk out).
; HOW: StageFadeTimer152D; $85 doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $85, N and Z from it, C its bit 7.
StageClearedBossWalk152D:
    jsr StageFadeTimer152D
    lda z:zField85
    asl a
    tax
    jmp ($8440,x)

; @routine 152D:446 StageBossScrollToDoor152D
; @run 6B:8446
; @unreachable 6B:8446-6B:847B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8446 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A step after the boss: the overlay at $A000's piece at $A009, the scroll stepped by 1 with the player's
; X one less, the scroll frame; once ScrollXHi is 4 the overlay's piece at $A067 runs, the scroll's steps are cleared,
; sound command $3D is sent and the step counted.
; HOW: MapOverlayBanks5And6, $A009; stores and a decrement; MapOverlayBank7, $A08D, StageOverlayFrame152D; a compare;
; MapOverlayBank7, $A067, the sound hook at $401B.
; WAITS: no.
; Entry: nothing. Exit, short of 4: A ScrollXHi, N, Z and C from the compare, the rest as StageOverlayFrame152D left
; them. Else: N and Z from the step's increment, the rest as the sound hook left them.
StageBossScrollToDoor152D:
    jsr $8021
    jsr RichterVecC3A7
    lda #$01
    sta a:ScrollStepYLo+6
    stz a:Scroll3E66+6
    dec a:OBJ_X_HI+8
    jsr $802C
    jsr $A08D
    jsr StageOverlayFrame152D
    lda a:ScrollXHi
    cmp #$04
    beq StageBossScrollToDoor152D_at0
    rts
StageBossScrollToDoor152D_at0:
    jsr $802C
    jsr ClearScrollSpeeds
    stz a:ScrollStepYLo+6
    stz a:ScrollStepYLo
    lda #$3D
    jsr VecSoundCommand
    inc z:zField85
    rts

; @routine 152D:47C StageBossWaitDoor152D
; @run 6B:847C
; @unreachable 6B:847C-6B:8498 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $847C runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A step after the boss: the overlay at $A000's piece at $A009 and the scroll; once the door object's
; byte at $283A is 4 and its animation step at $340A is 0, the player's step at $3358 is $20 and the step is counted.
; HOW: MapOverlayBanks5And6, $A009, StageOverlayFrame152D; two tests; a store and an increment.
; WAITS: no.
; Entry: nothing. Exit: A the last byte tested, N and Z from its test (C from the compare with 4), or $20 with N and Z
; from the step's increment; X, Y and V as StageOverlayFrame152D left them.
StageBossWaitDoor152D:
    jsr $8021
    jsr RichterVecC3A7
    jsr StageOverlayFrame152D
    lda a:$283A
    cmp #$04
    bne StageBossWaitDoor152D_at0
    lda a:OBJ_ANIM_STEP+2
    bne StageBossWaitDoor152D_at0
    lda #$20
    sta a:OBJ_3358
    inc z:zField85
StageBossWaitDoor152D_at0:
    rts

; @routine 152D:499 StageBossWalkOut152D
; @run 6B:8499
; @unreachable 6B:8499-6B:84D7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8499 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The last step after the boss: the player walks out (its count at $3310 $B8) with the overlay at $A000's
; frame; when its X reaches $F0 the player stops, $2743 is cleared, the next of the route's stages is taken from the
; table at $9A5F (zField82 counted, into zField83), $84 and the byte at $2A58 are cleared and the level starts.
; HOW: MapOverlayBanks5And6, $A000, $A006, $A009, MapOverlayBank7, $A08D, StageOverlayFrame152D; a compare;
; stores; a table read; StageLevelStart152D.
; WAITS: no.
; Entry: nothing. Exit, short of $F0: A the X, N, Z and C from the compare, the rest as StageOverlayFrame152D left
; them. Else: as StageLevelStart152D.
StageBossWalkOut152D:
    lda #$B8
    sta a:$3310
    jsr $8021
    jsr RunObjects1619
    jsr RichterVecApplyMove
    jsr RichterVecC3A7
    jsr $802C
    jsr $A08D
    jsr StageOverlayFrame152D
    lda a:OBJ_X_HI+8
    cmp #$F0
    beq StageBossWalkOut152D_at0
    rts
StageBossWalkOut152D_at0:
    stz a:$3310
    stz a:Field2742+1
    stz a:OBJ_3358
    inc z:zField82
    lda z:zField82
    tay
    lda a:$9A5F,y
    sta z:zField83
    cla
    sta z:$84
    stz a:$2A58
    jsr StageLevelStart152D
    rts

; @routine 152D:4D8 StageClearedLastWalk152D
; @run 6B:84D8
; @stop 84DF
; @unreachable 6B:84D8-6B:84DE no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $84D8 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The cleared stage's fourth table: its step at $85 from the four at $84DF (the wait for the objects and
; the scroll's wait of the walk out, a step that only counts, and the walk off the screen).
; HOW: $85 doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $85, N and Z from it, C its bit 7.
StageClearedLastWalk152D:
    lda z:zField85
    asl a
    tax
    jmp ($84DF,x)

; @routine 152D:4E7 StageClearedCount152D
; @run 6B:84E7
; @unreachable 6B:84E7-6B:84E9 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $84E7 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A step of the last walk that only counts the step.
; HOW: An increment of $85.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the increment; A, X, Y, C and V kept.
StageClearedCount152D:
    inc z:zField85
    rts

; @routine 152D:4EA StageClearedWalkOff152D
; @run 6B:84EA
; @unreachable 6B:84EA-6B:852B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $84EA runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The last walk off the screen: the player's step and count at $3358 and $3310 from $82F8 by the side,
; the overlay at $A000's frame, the player's X moved by $82F0 (its page at $2B30 too when the X wraps to 0); when the
; X reaches the side's mark at $82FA, zModeDone is counted.
; HOW: The side from zField8C; table reads; MapOverlayBanks5And6, $A000; adds and a compare.
; WAITS: no.
; Entry: nothing. Exit: A the X, N, Z and C from the compare, Y the side, V from the last add; N and Z from
; zModeDone's increment when it is counted. X as $A000 left it.
StageClearedWalkOff152D:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$82F8,y
    sta a:OBJ_3358
    sta a:$3310
    jsr $8021
    jsr RunObjects1619
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$82F0,y
    clc
    adc a:OBJ_X_HI+8
    sta a:OBJ_X_HI+8
    bne StageClearedWalkOff152D_at0
    lda a:$82F0,y
    clc
    adc a:$2B30
    sta a:$2B30
StageClearedWalkOff152D_at0:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_HI+8
    cmp a:$82FA,y
    bne StageClearedWalkOff152D_at1
    inc z:zModeDone
StageClearedWalkOff152D_at1:
    rts

; @routine 152D:52C StageSubModeFade152D
; @run 6B:852C
; @stop 8542
; @unreachable 6B:852C-6B:8541 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $852C runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-mode 4, the fade out after the death or the stage: the events step and the rows, then the step at
; zField80 from the seven at $8542 (the fade's start, its wait, the CD's stop and the stock taken, the waits, the
; restart).
; HOW: StageModeEventsStep152D, StageHudStep152D; a compare that branches to the next instruction either way;
; MapOverlayBanks5And6; zField80 doubled indexes the table, whose word is jumped through. Step 3, at $85A1, is the
; bytes of StageWaitTimerStep (stage_mode.c, 024E:023), a census duplicate run through its C.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zField80, N and Z from it, C its
; bit 7.
StageSubModeFade152D:
    jsr StageModeEventsStep152D
    jsr StageHudStep152D
    lda z:zField80
    cmp #$03
    beq StageSubModeFade152D_at0
StageSubModeFade152D_at0:
    jsr $8021
    lda z:zField80
    asl a
    tax
    jmp ($8542,x)

; @routine 152D:550 StageFadeStart152D
; @run 6B:8550
; @unreachable 6B:8550-6B:8558 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8550 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The fade's first step: the overlay at $A000's pieces at $A00C and $A012, and the step counted.
; HOW: Two calls and an increment of zField80.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the increment; A, X, Y, C and V as $A012 left them.
StageFadeStart152D:
    jsr RichterVecC023
    jsr RichterVecAnimationSound
    inc z:zField80
    rts

; @routine 152D:559 StageFadeWait152D
; @run 6B:8559
; @unreachable 6B:8559-6B:8570 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8559 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The fade's second step: the overlay at $A000's piece at $A00F, and $A012 again when it changed the byte
; at $2A58; once zField89 is set the step is counted.
; HOW: The byte kept on the stack across $A00F and compared after it; a test of zField89; an increment.
; WAITS: no.
; Entry: nothing. Exit: A zField89, N and Z from it or from zField80's increment; C from the compare, or as $A012 left
; it; X, Y and V as the last call left them.
StageFadeWait152D:
    lda a:$2A58
    pha
    jsr RichterVecC076
    pla
    cmp a:$2A58
    beq StageFadeWait152D_at0
    jsr RichterVecAnimationSound
StageFadeWait152D_at0:
    lda z:zField89
    bne StageFadeWait152D_at1
    rts
StageFadeWait152D_at1:
    inc z:zField80
    rts

; @routine 152D:571 StageFadeStopCd152D
; @run 6B:8571
; @unreachable 6B:8571-6B:857F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8571 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The fade's third step: the CD paused (cd_pause until it answers 0); with no stock left zModeDone is
; $80 (the game over), else a life is taken from zStock, the wait at zWaitTimer3 set to $60 and the step counted.
; HOW: cd_pause in a loop; a test of zStock; a decimal subtract (SED, SBC, CLD) and stores, at $8589 in
; StageFadeTakeStock152D's bytes.
; WAITS: in cd_pause, as the card's routine does.
; Entry: D clear, as the game keeps it. Exit, no stock: A $80, N and Z from it. Else: A $60, N and Z from zField80's
; increment, C from the subtract; X 0; Y and V as cd_pause left them.
StageFadeStopCd152D:
    jsr BiosCdPause
    tax
    bne StageFadeStopCd152D
    lda z:zStock
    bne StageFadeTakeStock152D_at1
    lda #$80
    sta z:zModeDone
    rts

; @routine 152D:580 StageFadeTakeStock152D
; @run 6B:8580
; @unreachable 6B:8580-6B:8596 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8580 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A fade step: with stock left the fade goes back to its third step (zField80 2); with none the wait at
; zWaitTimer3 is set to $60 and the step counted. Its bytes from $8589 also take a life for StageFadeStopCd152D.
; HOW: A test of zStock; stores and an increment.
; WAITS: no.
; Entry: nothing. Exit, stock left: A 2, N and Z from it. Else: A $60, N and Z from zField80's increment. X, Y, C and V
; kept.
StageFadeTakeStock152D:
    lda z:zStock
    beq StageFadeTakeStock152D_at0
    lda #$02
    sta z:zField80
    rts
StageFadeTakeStock152D_at1:
    sed
    sec
    sbc #$01
    cld
    sta z:zStock
StageFadeTakeStock152D_at0:
    lda #$60
    sta z:zWaitTimer3
    inc z:zField80
    rts

; @routine 152D:597 StageFadeWaitLast152D
; @run 6B:8597
; @unreachable 6B:8597-6B:85A0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8597 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A fade step: counts zWaitTimer3 down, and when it reaches 0 the fade goes back to its third step
; (zField80 2).
; HOW: A decrement; a store.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the decrement, A kept; at 0, A 2 with N and Z from it. X, Y, C and V kept.
StageFadeWaitLast152D:
    dec z:zWaitTimer3
    beq StageFadeWaitLast152D_at0
    rts
StageFadeWaitLast152D_at0:
    lda #$02
    sta z:zField80
    rts

; @routine 152D:5A9 StageFadeRestart152D
; @run 6B:85A9
; @unreachable 6B:85A9-6B:85C3 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $85A9 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The fade's last step: the stage starts again from sub-mode 0, zField83 back to zField86, zField8A and
; zField8F cleared, zField8E's bit 2 cleared; when zStage is negative the CD is paused first and zField8E's bit 3 set.
; HOW: Stores and bit changes; cd_pause in a loop.
; WAITS: in cd_pause, as the card's routine does.
; Entry: nothing. Exit: A 0, N and Z from it; X 0 after cd_pause; Y, C and V as cd_pause left them.
StageFadeRestart152D:
    lda z:zField86
    sta z:zField83
    stz z:zField8A
    stz z:zField8F
    rmb2 z:zField8E
    lda z:zStage
    bpl StageFadeRestart152D_at1
StageFadeRestart152D_at0:
    jsr BiosCdPause
    tax
    bne StageFadeRestart152D_at0
    smb3 z:zField8E
StageFadeRestart152D_at1:
    lda #$00
    sta z:zGameSubMode
    rts

; @routine 152D:5C4 StageSubModeOpening152D
; @run 6B:85C4
; @unreachable 6B:85C4-6B:85E9 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $85C4 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-mode 5, the stage's opening: the events step, the rows, the overlay at $A000's frame, the scroll,
; the opening's piece at $8BD5; once zField8E's bit 0 is set PlayerByte3BD8 and the player's count at $3310 are
; cleared and the stage is in play, sub-mode 1.
; HOW: StageModeEventsStep152D, StageHudStep152D, MapOverlayBanks5And6, $A000, StageOverlayFrame152D, MapOverlayBank7,
; $A08D, StageOpeningStep152D; a bit test; stores.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z from it when the stage goes into play; else as StageOpeningStep152D left them.
StageSubModeOpening152D:
    jsr StageModeEventsStep152D
    jsr StageHudStep152D
    jsr $8021
    jsr RunObjects1619
    jsr StageOverlayFrame152D
    jsr $802C
    jsr $A08D
    jsr StageOpeningStep152D
    bbr0 zField8E,StageSubModeOpening152D_at0
    stz a:PlayerByte3BD8
    lda #$01
    sta z:zGameSubMode
    stz a:$3310
StageSubModeOpening152D_at0:
    rts

; @routine 152D:5EA StageLoadRowMap152D
; @run 6B:85EA
; @unreachable 6B:85EA-6B:8618 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $85EA runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Fills the stage's row map at $3A00: cleared from KernelZeros (255 bytes), then unpacked from the
; route's list (the table at $9A68 by zField82): a byte that starts a run gives the map's index, and each byte after
; it is a row's value in its low seven bits, its bit 7 saying that the next byte starts a new run; $FF ends the list.
; HOW: TAI; the list's address in zTemp00-01; two loops, the run bit rotated into zTemp02.
; WAITS: no.
; Entry: nothing. Exit: A $FF, Z set, C set; X the map index reached, Y the list offset after the end. V kept.
; Clobbers zTemp00-02.
StageLoadRowMap152D:
    tai KernelZeros,$3A00,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9A68,y
    sta z:zTemp00
    lda a:$9A69,y
    sta z:zTemp01
    cly
StageLoadRowMap152D_at1:
    lda (zTemp00),y
    tax
    iny
StageLoadRowMap152D_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne StageLoadRowMap152D_at0
    rts
StageLoadRowMap152D_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:$3A00,x
    inx
    bbs0 zTemp02,StageLoadRowMap152D_at1
    bra StageLoadRowMap152D_at2

; @routine 152D:619 StageRouteSetUpA152D
; @run 6B:8619
; @unreachable 6B:8619-6B:8626 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8619 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The kernel's pieces at $403F and at $400C (with A the route's byte from the table at $9A59 by
; zField82), then the route's list unpacked into SaveBuffer (StageUnpackList152D).
; HOW: Two calls into the kernel; a jump to StageUnpackList152D.
; WAITS: where the kernel's pieces wait.
; Entry: nothing. Exit: as StageUnpackList152D.
StageRouteSetUpA152D:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9A59,x
    jsr VecStartListChannel0Kind2
    jmp StageUnpackList152D

; @routine 152D:627 StageRouteSetUpB152D
; @run 6B:8627
; @unreachable 6B:8627-6B:8630 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8627 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The kernel's piece at $4015 with A the route's byte from the table at $9A5C by zField82 and X 7.
; HOW: A table read; a jump into the kernel.
; WAITS: where the kernel's piece waits.
; Entry: nothing. Exit: as $4015 leaves them.
StageRouteSetUpB152D:
    ldx z:zField82
    lda a:$9A5C,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 152D:631 StageResetScroll152D
; @run 6B:8631
; @unreachable 6B:8631-6B:8684 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8631 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sets the stage's screen up: the kernel's piece at $4009, the scroll cleared and loaded for the stage
; (zField8E kept), the overlay at $A000's piece at $A5EB; for the raster effects 3, 5 and 6 the screen is loaded a
; second time and $A626 runs; then the scroll is cleared and it runs on into StageLoadScreen152D.
; HOW: $4009; TAI twice and StageLoadScreen152D by BSR; MapOverlayBank7, $A5EB; compares of zRasterEffect.
; WAITS: where the kernel's piece waits.
; Entry: nothing. Exit: as StageLoadScreen152D.
StageResetScroll152D:
    jsr VecClearScreenState
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollBgyLo,$0078
    lda z:zField8E
    pha
    bsr StageLoadScreen152D
    pla
    sta z:zField8E
    jsr $802C
    jsr $A5EB
    lda z:zRasterEffect
    cmp #$05
    beq StageResetScroll152D_at1
    cmp #$06
    beq StageResetScroll152D_at1
    cmp #$03
    bne StageResetScroll152D_at0
StageResetScroll152D_at1:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollBgyLo,$0078
    lda z:zField8E
    pha
    bsr StageLoadScreen152D
    pla
    sta z:zField8E
    jsr $A626
StageResetScroll152D_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollBgyLo,$0078

; @routine 152D:685 StageLoadScreen152D
; @run 6B:8685
; @unreachable 6B:8685-6B:872F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8685 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Loads the stage's screen from its record (the table at $9A90 by zField83): the scroll's pages, X and Y,
; steps, speeds and limits; unless zField8E's bit 1 asks to keep the player (it is then cleared), the player's X and
; the two bytes at $2C50 and $2C08 and its page; for a character other than 0 the player's X is moved by the table at
; $8730 by the byte at $2818 less 5; then zRasterEffect (zRasterArg cleared) and the byte at $2AA0.
; HOW: The record's address in zTemp00-01; a byte at a time through it, each stored where it goes.
; WAITS: no.
; Entry: nothing. Exit: A the record's last byte, N and Z from it, Y past it; X, C and V as the moves of the player
; left them. Clobbers zTemp00-01.
StageLoadScreen152D:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9A90,y
    sta z:zTemp00
    lda a:$9A91,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollStepXLo+6
    sta a:ScrollStepXLo+7
    sta a:ScrollStepXHi
    sta a:ScrollStepXHi+1
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    lda (zTemp00),y
    iny
    sta a:ScrollStepYHi+6
    and #$01
    sta a:ScrollYHi
    lda (zTemp00),y
    iny
    sta a:ScrollXLo
    lda (zTemp00),y
    iny
    sta a:ScrollYLo
    lda (zTemp00),y
    iny
    sta a:ScrollSpeedX+6
    lda (zTemp00),y
    iny
    sta a:ScrollBgyLo
    lda (zTemp00),y
    iny
    sta a:ScrollSpeedY+6
    lda (zTemp00),y
    iny
    sta a:$3EB4
    lda (zTemp00),y
    iny
    sta a:RasterWaterLine
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi+6
    bbr1 zField8E,StageLoadScreen152D_at0
    rmb1 z:zField8E
    iny
    iny
    bra StageLoadScreen152D_at1
StageLoadScreen152D_at0:
    lda (zTemp00),y
    iny
    sta a:OBJ_X_HI+8
    lda (zTemp00),y
    iny
    sta a:$2C50
    stz a:$2C08
    stz a:$2B30
    lda z:zCharacter
    beq StageLoadScreen152D_at1
    lda a:$2818
    cmp #$05
    bcc StageLoadScreen152D_at1
    sec
    sbc #$05
    tax
    lda a:OBJ_X_HI+8
    clc
    adc a:$8730,x
    sta a:OBJ_X_HI+8
StageLoadScreen152D_at1:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    iny
    sta a:$2AA0
    rts

; @routine 152D:734 StageOverlayFrame152D
; @run 6B:8734
; @unreachable 6B:8734-6B:8747 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8734 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The frame of the overlays above: the piece at $AE47 under bank 7, then the piece at $A000 under banks
; 8 and 9 above zRamBaseBank.
; HOW: MapOverlayBank7, $AE47; the add and two TAM; $A000.
; WAITS: where the pieces wait.
; Entry: nothing. Exit: as $A000 leaves them.
StageOverlayFrame152D:
    jsr $802C
    jsr $AE47
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr RunObjects1619
    rts

; @routine 152D:748 StageLevelStart152D
; @run 6B:8748
; @unreachable 6B:8748-6B:877A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8748 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts the stage's level: the kernel's piece at $4057 under bank 7, the level's stream and axis from
; the four bytes at $9B20 by zField83, the 32 event flags cleared when the fourth byte is not 0, then the overlay at
; $A000's piece at $ADAF.
; HOW: MapOverlayBank7, $4057; table reads; a loop of stores with Y kept on the stack; $ADAF.
; WAITS: where the pieces wait.
; Entry: nothing. Exit: as $ADAF leaves them.
StageLevelStart152D:
    jsr $802C
    jsr VecClearEnemySlots
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9B20,y
    sta a:LevelStreamLo
    lda a:$9B21,y
    sta a:LevelStreamHi
    lda a:$9B22,y
    sta a:LevelAxis
    lda a:$9B23,y
    beq StageLevelStart152D_at0
    phy
    cly
StageLevelStart152D_at1:
    lda #$00
    sta a:EventFlags,y
    iny
    cpy #$20
    bcc StageLevelStart152D_at1
    ply
StageLevelStart152D_at0:
    jsr FaceThePlayerInSomeStatesC
    rts

; @routine 152D:77B DoorHandler152D
; @run 6B:877B
; @unreachable 6B:877B-6B:87C5 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $877B runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: An object handler for a door the pad's override opens. Not yet placed (its byte at $2818 0): $A36C, and
; the object's packed byte at $2F20 unpacked into its room ($3310), the room's entry ($3358) and the pad override's
; index ($32C8). Placed: the byte at $3790 cleared, and when the override at PadOverrides by that index is set and the
; object's state at $31F0 is 1 to 4, the stage goes through the door (zField83 the room, zField87 the entry, zSubStep
; 2); otherwise the state is cleared.
; HOW: Loads, shifts through the carry and stores; the tests and the stores of the door.
; WAITS: no.
; Entry: X the object. Exit: A the last value loaded or masked, N and Z from it, C from the shifts or the compare, Y
; the packed byte shifted or the override's index; X kept.
DoorHandler152D:
    lda a:$2818,x
    bne DoorHandler152D_at0
    jsr $A36C
    lda a:$2F20,x
    tay
    and #$0F
    sta a:$3310,x
    tya
    rol a
    rol a
    rol a
    tay
    and #$03
    sta a:OBJ_3358,x
    tya
    rol a
    rol a
    and #$03
    sta a:$32C8,x
    rts
DoorHandler152D_at0:
    stz a:$3790,x
    ldy a:$32C8,x
    lda a:PadOverrides,y
    beq DoorHandler152D_at1
    lda a:$31F0,x
    beq DoorHandler152D_at1
    cmp #$05
    bcs DoorHandler152D_at1
    lda a:$3310,x
    sta z:zField83
    lda a:OBJ_3358,x
    sta z:zField87
    lda #$02
    sta z:zSubStep
    rts
DoorHandler152D_at1:
    stz a:$31F0,x
    rts

; @routine 152D:7C6 DoorHandlerB152D
; @run 6B:87C6
; @unreachable 6B:87C6-6B:8800 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $87C6 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: An object handler for a door PlayerByte3BD7 opens: as DoorHandler152D, with no pad override (the
; packed byte gives the room and the entry only), the gate PlayerByte3BD7. Its fall-back that clears the state is
; DoorHandler152D's bytes at $87C2.
; HOW: Loads, shifts through the carry and stores; the tests and the stores of the door.
; WAITS: no.
; Entry: X the object. Exit: as DoorHandler152D, Y the packed byte when it was unpacked.
DoorHandlerB152D:
    lda a:$2818,x
    bne DoorHandlerB152D_at0
    jsr $A36C
    lda a:$2F20,x
    tay
    and #$0F
    sta a:$3310,x
    tya
    rol a
    rol a
    rol a
    and #$03
    sta a:OBJ_3358,x
    rts
DoorHandlerB152D_at0:
    stz a:$3790,x
    lda a:PlayerByte3BD7
    beq DoorHandler152D_at1
    lda a:$31F0,x
    beq DoorHandler152D_at1
    cmp #$05
    bcs DoorHandler152D_at1
    lda a:$3310,x
    sta z:zField83
    lda a:OBJ_3358,x
    sta z:zField87
    lda #$02
    sta z:zSubStep
    rts

; @routine 152E:001 StageHudStart152D
; @run 6B:8801
; @unreachable 6B:8801-6B:8810 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8801 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Draws the stage's display at its start: the pieces 0 and 1 of StageDrawPiece152D, the stock and the
; points to add (StageDrawStock152D, StageDrawScoreAdd152D); zField8E's bit 5 cleared; then runs on into
; StageHudStep152D.
; HOW: Calls with X 0 and 1; a bit cleared.
; WAITS: no.
; Entry: nothing. Exit: as StageHudStep152D.
StageHudStart152D:
    clx
    jsr StageDrawPiece152D
    inx
    jsr StageDrawPiece152D
    jsr StageDrawStock152D
    jsr StageDrawScoreAdd152D
    rmb5 z:zField8E

; @routine 152E:011 StageHudStep152D
; @run 6B:8811
; @stop 8878
; @unreachable 6B:8811-6B:8877 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8811 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The stage's display each frame: the item sound's step, the item's frame at $2A9E from the table at
; $8878 by zField93; the health bar moved one step toward zPlayerHealth (zField99 the shown health: down, the bar's
; piece cleared; up, set, with sound $0B once when the bar was full and zField8E's bit 6 not yet set), sound $2C when
; it reaches it after rising; the hearts' two digits when zField94 changed; and while zField8E's bit 2 is set the
; boss's bar (StageBossBar152D).
; HOW: StageHudSound152D; compares of zPlayerHealth with zField99; StageHealthBarAddress152D by BSR with AndVramData
; or OrVramData (census duplicates at $942A and $9439, through map_draw.c's C); the sound hook; StageDrawDigit152D for
; each digit; a jump to StageBossBar152D.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them.
StageHudStep152D:
    jsr StageHudSound152D
    ldx z:zField93
    lda a:$8878,x
    sta a:$2A9E
    lda z:zPlayerHealth
    cmp z:zField99
    beq StageHudStep152D_at0
    bcs StageHudStep152D_at1
    bsr StageHealthBarAddress152D
    ldx #$FF
    ldy #$F0
    jsr $942A
    dec z:zField99
    bra StageHudStep152D_at6
StageHudStep152D_at1:
    bbr5 zField8E,StageHudStep152D_at2
    bbs6 zField8E,StageHudStep152D_at2
    lda #$0B
    jsr VecSoundCommand
    smb6 z:zField8E
StageHudStep152D_at2:
    inc z:zField99
    bsr StageHealthBarAddress152D
    ldx #$00
    ldy #$0F
    jsr $9439
    bra StageHudStep152D_at3
StageHudStep152D_at0:
    smb5 z:zField8E
StageHudStep152D_at6:
    bbr6 zField8E,StageHudStep152D_at3
    rmb6 z:zField8E
    lda #$2C
    jsr VecSoundCommand
StageHudStep152D_at3:
    lda z:zField94
    cmp z:zField95
    beq StageHudStep152D_at4
    sta z:zField95
    and #$0F
    ldx #$08
    jsr StageDrawDigit152D
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr StageDrawDigit152D
StageHudStep152D_at4:
    bbs2 zField8E,StageHudStep152D_at5
    rts
StageHudStep152D_at5:
    jmp StageBossBar152D

; @routine 152E:081 StageHudSound152D
; @run 6B:8881
; @unreachable 6B:8881-6B:8890 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8881 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The item's glow: the kernel's piece at $4018 with X 1 and A the colour of the step the wave counter's
; low three bits (masked by RichterItemReady) pick from the eight at $8891.
; HOW: Two ANDs and a table read; a jump into the kernel.
; WAITS: no.
; Entry: nothing. Exit: as $4018 leaves them.
StageHudSound152D:
    lda z:zWaveCounter
    and #$07
    and a:RichterItemReady
    tax
    lda a:$8891,x
    ldx #$01
    jmp VecStartScriptSlotKind4

; @routine 152E:099 StageHealthBarAddress152D
; @run 6B:8899
; @unreachable 6B:8899-6B:88C1 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8899 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Points the VDC at the health bar's piece for the shown health zField99 ($5E less it, in the rows from
; $6500), unless zField99 is 0.
; HOW: A subtract, nibbles and shifts into zTemp00-01, an add; SetVramAddresses152D with the address in X and
; Y.
; WAITS: no.
; Entry: nothing. Exit, zField99 0: A 0, N and Z from it. Else: A 2, N and Z clear; X and Y the address, C from the
; add, V from it. Clobbers zTemp00-01.
StageHealthBarAddress152D:
    lda z:zField99
    beq StageHealthBarAddress152D_at0
    lda #$5E
    sec
    sbc z:zField99
    pha
    and #$0F
    sta z:zTemp00
    pla
    and #$F0
    stz z:zTemp01
    asl a
    rol z:zTemp01
    asl a
    rol z:zTemp01
    asl a
    rol z:zTemp01
    clc
    adc z:zTemp00
    tax
    lda z:zTemp01
    adc #$65
    tay
    jsr SetVramAddresses152D
StageHealthBarAddress152D_at0:
    rts

; @routine 152E:0C2 StageBossBarReset152D
; @run 6B:88C2
; @unreachable 6B:88C2-6B:88CB no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $88C2 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The boss's bar drawn anew (StageDrawPiece152D 2) and its length at $2745 set to $5C; then runs on into
; StageBossBar152D. Reached from StageBossBar152D while the boss's frame at $2A9D is 0.
; HOW: A call with X 2; a store.
; WAITS: no.
; Entry: nothing. Exit: as StageBossBar152D.
StageBossBarReset152D:
    ldx #$02
    jsr StageDrawPiece152D
    lda #$5C
    sta a:LevelFullWalk+1

; @routine 152E:0CC StageBossBar152D
; @run 6B:88CC
; @unreachable 6B:88CC-6B:88DA no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $88CC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The boss's bar each frame: while the boss's frame at $2A9D is 0 the bar is drawn anew
; (StageBossBarReset152D); the boss's X at $2BBD is moved left until it reaches $D8; then StageBossBarStep152D.
; HOW: Tests; a decrement; a branch or run into StageBossBarStep152D.
; WAITS: no.
; Entry: nothing. Exit: as StageBossBarStep152D.
StageBossBar152D:
    lda a:$2A9D
    beq StageBossBarReset152D
    lda a:$2BBD
    cmp #$D8
    beq StageBossBarStep152D
    dec a:$2BBD

; @routine 152E:0DB StageBossBarStep152D
; @run 6B:88DB
; @unreachable 6B:88DB-6B:891B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $88DB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Moves the boss's bar at $2745 one step toward the boss's health at $3128: down, the piece cleared; up,
; set. A negative health is the boss's end: the bar is stepped with a health of 0 (by a call of this routine) and the
; health put back; once the bar is empty, the count at $330D takes the health. While the count at $330D is not 0 the
; bar flashes instead (StageBossBarFlash152D).
; HOW: Compares; StageBossBarAddress152D by BSR with AndVramData or OrVramData (map_draw.c's C, census duplicates at
; $942A and $9439); the health kept on the stack across the call of itself.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them; at the bar, A the health, N, Z and C from the compare.
StageBossBarStep152D:
    lda a:$330D
    beq StageBossBarStep152D_at0
    jmp StageBossBarFlash152D
StageBossBarStep152D_at0:
    lda a:$3128
    bmi StageBossBarStep152D_at1
    cmp a:LevelFullWalk+1
    beq StageBossBarStep152D_at2
    bcs StageBossBarStep152D_at3
    bsr StageBossBarAddress152D
    ldx #$0F
    ldy #$FF
    jsr $942A
    dec a:LevelFullWalk+1
    rts
StageBossBarStep152D_at1:
    pha
    stz a:$3128
    bsr StageBossBarStep152D
    pla
    sta a:$3128
    ldy a:LevelFullWalk+1
    bne StageBossBarStep152D_at2
    sta a:$330D
    rts
StageBossBarStep152D_at3:
    inc a:LevelFullWalk+1
    bsr StageBossBarAddress152D
    ldx #$F0
    ldy #$00
    jsr $9439
StageBossBarStep152D_at2:
    rts

; @routine 152E:11C StageBossBarAddress152D
; @run 6B:891C
; @unreachable 6B:891C-6B:8964 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $891C runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Points the VDC at the boss's bar's piece for the length at $2745 ($5E less it, from $5D40, a page on
; at $3F and more), unless the length is 0.
; HOW: The page in zTemp08-09; a subtract, nibbles and shifts into zTemp00-01, adds; SetVramAddresses152D with the
; address in X and Y.
; WAITS: no.
; Entry: nothing. Exit, length 0: A 0, N and Z from it. Else: A 2, N and Z clear; X and Y the address, C and V from the
; last add. Clobbers zTemp00-01 and zTemp08-09.
StageBossBarAddress152D:
    lda a:LevelFullWalk+1
    beq StageBossBarStep152D_at2
    stz z:zTemp08
    stz z:zTemp09
    cmp #$3F
    bcc StageBossBarAddress152D_at0
    lda #$C0
    sta z:zTemp08
    inc z:zTemp09
StageBossBarAddress152D_at0:
    lda #$5E
    sec
    sbc a:LevelFullWalk+1
    pha
    and #$0F
    clc
    adc #$40
    sta z:zTemp00
    pla
    and #$F0
    stz z:zTemp01
    asl a
    rol z:zTemp01
    asl a
    rol z:zTemp01
    asl a
    rol z:zTemp01
    clc
    adc z:zTemp00
    sta z:zTemp00
    lda z:zTemp01
    adc #$5D
    sta z:zTemp01
    lda z:zTemp08
    clc
    adc z:zTemp00
    tax
    lda z:zTemp09
    adc z:zTemp01
    tay
    jsr SetVramAddresses152D
StageBossBarAddress152D_at1:
    rts

; @routine 152E:17D StageBossBarFlash152D
; @run 6B:897D
; @unreachable 6B:897D-6B:89BD no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $897D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The boss's bar at its end: every other piece of the four (by the wave counter's bit 0, from the table
; of addresses at $8965) blanked by StageBarErase152D; the flash's step at $339D counted; the boss's speed at $2DFD and
; $2DB5 raised by $0C; the kernel's piece at $4045 with X $45; once the boss's page at $2C4D is 0 the boss's frame at
; $2A9D and zField8E's bit 2 are cleared.
; HOW: A loop of four in zTemp01; adds; the kernel; stores.
; WAITS: no.
; Entry: nothing. Exit: A the boss's page, N and Z from it, the rest as $4045 left them.
StageBossBarFlash152D:
    lda z:zWaveCounter
    and #$01
    sta z:zTemp01
StageBossBarFlash152D_at0:
    lda z:zTemp01
    asl a
    tax
    ldy a:$8966,x
    lda a:$8965,x
    tax
    jsr StageBarErase152D
    inc z:zTemp01
    inc z:zTemp01
    lda z:zTemp01
    cmp #$08
    bcc StageBossBarFlash152D_at0
    inc a:$339D
    lda a:$2DFD
    clc
    adc #$0C
    sta a:$2DFD
    cla
    adc a:$2DB5
    sta a:$2DB5
    ldx #$45
    jsr VecApplyVelocity
    lda a:$2C4D
    beq StageBossBarAddress152D_at1
    stz a:$2A9D
    rmb2 z:zField8E
    rts

; @routine 152E:1BE StageBarErase152D
; @run 6B:89BE
; @unreachable 6B:89BE-6B:89E8 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $89BE runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Blanks a bar's piece in VRAM at the address in X and Y, in 64 words: each word ANDed with a mask of
; the kernel's random piece at $4051, zRandom2 and zRandom1, and the flash step's row of the table at $8975.
; HOW: SetVramAddresses152D; a loop down X of reads and writes of the VDC's
; data port.
; WAITS: no.
; Entry: X, Y the VRAM address. Exit: X 0, N and Z from it; A the word's high byte, Y the flash step's row; C and V
; as $4051 left them. Clobbers zTemp00.
StageBarErase152D:
    jsr SetVramAddresses152D
    ldx #$40
StageBarErase152D_at0:
    lda a:$339D
    and #$07
    tay
    jsr VecNextRandom
    ora z:zRandom2
    ora z:zRandom1
    ora a:$8975,y
    sta z:zTemp00
    lda a:VDC_DATA_LO
    and z:zTemp00
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and z:zTemp00
    sta a:VDC_DATA_HI
    dex
    bne StageBarErase152D_at0
    rts

; @routine 152E:1E9 StageDrawDigit152D
; @run 6B:89E9
; @unreachable 6B:89E9-6B:8A68 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $89E9 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Draws the digit in A into the display at the place in X (StageDigitAddress152D): the digit's five rows
; (five bytes from $8A84 up) laid into the VRAM words' bit planes; an even place takes bits 4 to 7 of the low byte and
; bits 0 and 1 of the high, an odd place bits 4 to 7 of the high byte, then the next place's low two bits.
; HOW: The glyph's address in zTemp00-01; StageDigitAddress152D by BSR; loops of five reads and writes of the VDC's
; data port with shifts through zTemp02-03.
; WAITS: no.
; Entry: A the digit, X the place. Exit: Y 5, Z set from the compare, C set; A the last high byte written. Clobbers
; zTemp00-04.
StageDrawDigit152D:
    stx z:zTemp04
    sta z:zTemp00
    asl a
    asl a
    adc z:zTemp00
    adc #$84
    sta z:zTemp00
    cla
    adc #$8A
    sta z:zTemp01
    bsr StageDigitAddress152D
    lda z:zTemp04
    lsr a
    bcs StageDrawDigit152D_at0
    cly
StageDrawDigit152D_at1:
    lda (zTemp00),y
    stz z:zTemp03
    asl a
    rol z:zTemp03
    asl a
    rol z:zTemp03
    sta z:zTemp02
    lda a:VDC_DATA_LO
    and #$0F
    ora z:zTemp02
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and #$FC
    ora z:zTemp03
    sta a:VDC_DATA_HI
    iny
    cpy #$05
    bne StageDrawDigit152D_at1
    rts
StageDrawDigit152D_at0:
    cly
StageDrawDigit152D_at2:
    lda (zTemp00),y
    asl a
    asl a
    sta z:zTemp02
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and #$0F
    ora z:zTemp02
    sta a:VDC_DATA_HI
    iny
    cpy #$05
    bne StageDrawDigit152D_at2
    inc z:zTemp04
    bsr StageDigitAddress152D
    cly
StageDrawDigit152D_at3:
    lda (zTemp00),y
    stz z:zTemp02
    asl a
    rol z:zTemp02
    asl a
    rol z:zTemp02
    lda a:VDC_DATA_LO
    and #$FC
    ora z:zTemp02
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    iny
    cpy #$05
    bne StageDrawDigit152D_at3
    rts

; @routine 152E:269 StageDigitAddress152D
; @run 6B:8A69
; @unreachable 6B:8A69-6B:8A77 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8A69 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Points the VDC at the display's place zTemp04 (its even pair) from the table of addresses at $8A78.
; HOW: Two table reads; a jump to SetVramAddresses152D.
; WAITS: no.
; Entry: zTemp04 the place. Exit: A 2, N and Z clear; X and Y the address.
StageDigitAddress152D:
    lda z:zTemp04
    and #$FE
    tax
    lda a:$8A78,x
    ldy a:$8A79,x
    tax
    jmp SetVramAddresses152D

; @routine 152E:2CC StagePausedSelect152D
; @run 6B:8ACC
; @unreachable 6B:8ACC-6B:8AF0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8ACC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The pause screen's choice, reached from StagePausedFrame152D: on the first pad's left or right (bits 4
; and 6 of BiosJoyTrigger) the sub-weapon at zField93 moves to the next or the one before (the table at $8AC0, two on
; for right), its frame at $2A9E from the table at $8878, PlayerByte3BD6 1 and zField94 $99.
; HOW: Pad bits; table reads; stores.
; WAITS: no.
; Entry: nothing. Exit, no press: A 0, N and Z from it. Else: A $99, N and Z from it, X the new choice. Y, C and V
; kept.
StagePausedSelect152D:
    lda a:BiosJoyTrigger
    and #$50
    beq StagePausedFrame152D_at4
    ldx z:zField93
    and #$40
    beq StagePausedSelect152D_at0
    inx
    inx
StagePausedSelect152D_at0:
    lda a:$8AC0,x
    sta z:zField93
    tax
    lda a:$8878,x
    sta a:$2A9E
    lda #$01
    sta a:PlayerByte3BD6
    lda #$99
    sta z:zField94
    rts

; @routine 152E:2F1 StagePausedFrame152D
; @run 6B:8AF1
; @unreachable 6B:8AF1-6B:8B6F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8AF1 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The paused frame. zPaused's bit 1 clear: the choice (StagePausedSelect152D). Else zPaused counts up by
; 4 a frame (its bits 2 up the menu's step, bit 0 the side it scrolls from) through the menu's rows (the table of four
; bytes a row at $8B70, from $19 down or up to $1A); past them only bit 0 is kept. Steps $25 and $27 at the end of the
; scroll redraw the score, or the stock and the points to add.
; HOW: Shifts of zPaused through zTemp00; a subtract; the row's step through the kernel (pause_menu_row); jumps to
; StageDrawScore152D or StageDrawScoreAdd152D, a call of StageDrawStock152D.
; WAITS: no.
; Entry: nothing. Exit: as the step that ran left them.
StagePausedFrame152D:
    lda z:zPaused
    lsr a
    ror z:zTemp00
    lsr a
    bcc StagePausedSelect152D
    rol z:zTemp00
    bcc StagePausedFrame152D_at0
    cmp #$1A
    bcc StagePausedFrame152D_at1
StagePausedFrame152D_at2:
    lda z:zPaused
    and #$01
    sta z:zPaused
StagePausedFrame152D_at4:
    rts
StagePausedFrame152D_at0:
    sta z:zTemp00
    lda #$19
    sec
    sbc z:zTemp00
    bcc StagePausedFrame152D_at2
StagePausedFrame152D_at1:
    lsr a
    php
    asl a
    asl a
    tax
    lda z:zPaused
    clc
    adc #$04
    sta z:zPaused
    plp
    bcs StagePausedFrame152D_at3
    stx z:zTemp00
    lda a:$8B70,x
    ldx #$02
    jsr VecStartScriptSlotKind4
    ldx z:zTemp00
    lda a:$8B71,x
    ldx #$03
    jsr VecStartScriptSlotKind4
    ldx z:zTemp00
    lda a:$8B73,x
    sta a:$2A9F
    lda z:zPaused
    sta z:zTemp01
    ldx z:zTemp00
    lda a:$8B72,x
    beq StagePausedFrame152D_at4
    lsr z:zTemp01
    bcc StagePausedFrame152D_at5
    cmp #$65
    beq StagePausedFrame152D_at4
    bne StagePausedFrame152D_at6
StagePausedFrame152D_at5:
    cmp #$64
    beq StagePausedFrame152D_at4
StagePausedFrame152D_at6:
    ldx #$04
    jmp VecStartListChannelKind2
StagePausedFrame152D_at3:
    lda z:zPaused
    lsr a
    bcc StagePausedFrame152D_at7
    cmp #$25
    bne StagePausedFrame152D_at4
    jmp StageDrawScore152D
StagePausedFrame152D_at7:
    cmp #$27
    bne StagePausedFrame152D_at4
    jsr StageDrawStock152D
    jmp StageDrawScoreAdd152D

; @routine 152E:3A4 StageOpeningStart152D
; @run 6B:8BA4
; @unreachable 6B:8BA4-6B:8BD4 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8BA4 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Starts the stage's opening when zField8E's bit 0 is clear: the display's pieces 3 and 4
; (StageDrawPiece152D), the opening's objects placed (pages at $2B74 and $2C4D one less, speeds at $2D24 and $2DB5 4,
; counts at $3354 and $3355 $20), PlayerByte3BD8 2, sound $3B, the CD's wait at $33E4 cleared.
; HOW: A bit test; calls with X 3 and 4; stores; the sound hook.
; WAITS: no.
; Entry: nothing. Exit, bit 0 set: everything kept. Else: as the sound hook left them.
StageOpeningStart152D:
    bbr0 zField8E,StageOpeningStart152D_at0
    rts
StageOpeningStart152D_at0:
    ldx #$03
    jsr StageDrawPiece152D
    inx
    jsr StageDrawPiece152D
    dec a:OBJ_X_HI+4
    dec a:$2C4D
    lda #$04
    sta a:$2D24
    sta a:$2DB5
    lda #$20
    sta a:$3354
    sta a:$3355
    lda #$02
    sta a:PlayerByte3BD8
    lda #$3B
    jsr VecSoundCommand
    stz a:$33E4
    rts

; @routine 152E:3D5 StageOpeningStep152D
; @run 6B:8BD5
; @stop 8C87
; @unreachable 6B:8BD5-6B:8C86 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8BD5 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One frame of the stage's opening: once the ADPCM is idle the kernel's piece at $407B runs once ($33E4
; set). Two objects move in steps: each step's count ($3355, $3354) runs down, then the step's number ($330D, $330C)
; counts on and its speed and count come from the tables at $8C86 and $8C8C; at the first's step 7 the opening ends
; (the boss's frame at $2A9D cleared, zField8E's bit 0 set, $407B if not yet run). At the first's step 6 the count is
; kept at $339C ($2A9C cleared at $1F); at the second's step 6 the screen's row fade runs with it
; (StageOpeningFade152D).
; HOW: ad_stat; decrements and table reads; 24-bit adds of the speeds into the positions.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them.
StageOpeningStep152D:
    lda a:$33E4
    bne StageOpeningStep152D_at0
    jsr BiosAdStat
    tax
    bne StageOpeningStep152D_at0
    jsr VecCdPlayAgain
    inc a:$33E4
StageOpeningStep152D_at0:
    dec a:$3355
    bne StageOpeningStep152D_at1
    inc a:$330D
    ldy a:$330D
    cpy #$07
    bne StageOpeningStep152D_at2
    stz a:$2A9D
    smb0 z:zField8E
    lda a:$33E4
    bne StageOpeningStep152D_at3
    jsr VecCdPlayAgain
StageOpeningStep152D_at3:
    rts
StageOpeningStep152D_at2:
    lda a:$8C86,y
    lsr a
    sta a:$2DB5
    cla
    ror a
    sta a:$2DFD
    lda a:$8C8C,y
    sta a:$3355
StageOpeningStep152D_at1:
    lda a:$330D
    cmp #$06
    bne StageOpeningStep152D_at4
    lda a:$3355
    cmp #$1F
    bne StageOpeningStep152D_at5
    stz a:$2A9C
StageOpeningStep152D_at5:
    sta a:$339C
StageOpeningStep152D_at4:
    lda a:$2DFD
    clc
    adc a:$2CDD
    sta a:$2CDD
    lda a:$2DB5
    adc a:$2C95
    sta a:$2C95
    cla
    adc a:$2C4D
    sta a:$2C4D
    dec a:$3354
    bne StageOpeningStep152D_at6
    inc a:$330C
    ldy a:$330C
    lda a:$8C86,y
    lsr a
    sta a:$2D24
    cla
    ror a
    sta a:$2D6C
    lda a:$8C8C,y
    sta a:$3354
StageOpeningStep152D_at6:
    lda a:$2D6C
    clc
    adc a:$2C04
    sta a:$2C04
    lda a:$2D24
    adc a:$2BBC
    sta a:$2BBC
    cla
    adc a:OBJ_X_HI+4
    sta a:OBJ_X_HI+4
    lda a:$330C
    cmp #$06
    beq StageOpeningStep152D_at7
    rts
StageOpeningStep152D_at7:
    lda a:$339C
    bra StageOpeningFade152D

; @routine 152E:4A3 StageOpeningFade152D
; @run 6B:8CA3
; @unreachable 6B:8CA3-6B:8CEE no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8CA3 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The opening's fade of the screen's rows, with A the count kept at $339C: from it the rows' fade level
; (zMultiplierHi, $10 less the low nibble) and the group of rows (the table of four bytes a group at $8C93: the first
; row's VRAM address, the count of rows, the step between them); each row is faded by StageOpeningCopyRow152D after
; the kernel's piece at $4051.
; HOW: Masks, a subtract and table reads into zTemp10-12 and zProduct0; a loop of rows with a 16-bit add.
; WAITS: no.
; Entry: A the count. Exit: N and Z from the last row count's decrement (Z set); A the address's high byte, C and V
; from its add; X and Y as StageOpeningCopyRow152D left them.
StageOpeningFade152D:
    sta z:SpritesLeft
    cmp #$20
    bcc StageOpeningFade152D_at0
    and #$1F
    lsr a
    bcc StageOpeningFade152D_at1
    clc
    adc #$10
StageOpeningFade152D_at1:
    adc #$20
StageOpeningFade152D_at0:
    pha
    and #$0F
    sta z:zTemp10
    lda #$10
    sec
    sbc z:zTemp10
    sta z:SpritesLeft
    pla
    lsr a
    lsr a
    and #$0C
    tax
    lda a:$8C93,x
    sta z:zTemp10
    lda a:$8C94,x
    sta z:zTemp11
    lda a:$8C95,x
    sta z:zTemp12
    lda a:$8C96,x
    sta z:zProduct0
StageOpeningFade152D_at2:
    jsr VecNextRandom
    bsr StageOpeningCopyRow152D
    lda z:zTemp10
    clc
    adc z:zProduct0
    sta z:zTemp10
    cla
    adc z:zTemp11
    sta z:zTemp11
    dec z:zTemp12
    bne StageOpeningFade152D_at2
    rts

; @routine 152E:4EF StageOpeningCopyRow152D
; @run 6B:8CEF
; @unreachable 6B:8CEF-6B:8DB7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8CEF runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Fades one row of the screen in VRAM: the row's 64 words (at the address in zTemp10-11) read into the
; buffer at $397E; at a level of $10 or more cleared; else, for each level step below the last, every word ANDed with
; the next word under a random mask (zRandom2 rotated with bit 7 set, ORed with itself), then every word ANDed with
; zRandom2 OR zRandom3; then runs on into StageOpeningWriteRow152D, which writes the buffer back.
; HOW: SetVramAddresses152D; loops of the VDC's data port and of the buffer.
; WAITS: no.
; Entry: zTemp10-11 the row's address, zMultiplierHi the level. Exit: as StageOpeningWriteRow152D. Clobbers zTemp00.
StageOpeningCopyRow152D:
    ldx z:zTemp10
    ldy z:zTemp11
    jsr SetVramAddresses152D
    ldx #$80
StageOpeningCopyRow152D_at1:
    lda a:VDC_DATA_LO
    sta a:$397F,x
    lda a:VDC_DATA_HI
    sta a:$397E,x
    dex
    dex
    bne StageOpeningCopyRow152D_at1
    ldy z:SpritesLeft
    cpy #$10
    bcc StageOpeningCopyRow152D_at2
    ldx #$7F
StageOpeningCopyRow152D_at3:
    stz a:$3980,x
    dex
    bpl StageOpeningCopyRow152D_at3
    jmp StageOpeningWriteRow152D
StageOpeningCopyRow152D_at2:
    dey
    beq StageOpeningCopyRow152D_at4
    clx
StageOpeningCopyRow152D_at5:
    lda z:zRandom2
    sec
    ror a
    ora z:zRandom2
    sta z:zTemp00
StageOpeningCopyRow152D_at0:
    lda a:$3982,x
    and z:zTemp00
    sta a:$3980,x
    lda a:$3983,x
    and z:zTemp00
    sta a:$3981,x
    lda a:$39A2,x
    and z:zTemp00
    sta a:$39A0,x
    lda a:$39A3,x
    and z:zTemp00
    sta a:$39A1,x
    lda a:$39C2,x
    and z:zTemp00
    sta a:$39C0,x
    lda a:$39C3,x
    and z:zTemp00
    sta a:$39C1,x
    lda a:OBJ_HITBOX_DY+2,x
    and z:zTemp00
    sta a:OBJ_HITBOX_DY,x
    lda a:OBJ_HITBOX_DY+3,x
    and z:zTemp00
    sta a:OBJ_HITBOX_DY+1,x
    inx
    inx
    dey
    bne StageOpeningCopyRow152D_at5
StageOpeningCopyRow152D_at4:
    ldy z:SpritesLeft
    clx
StageOpeningCopyRow152D_at6:
    lda z:zRandom2
    ora z:zRandom3
    sta z:zTemp00
    lda a:$3980,x
    and z:zTemp00
    sta a:$3980,x
    lda a:$3981,x
    and z:zTemp00
    sta a:$3981,x
    lda a:$39A0,x
    and z:zTemp00
    sta a:$39A0,x
    lda a:$39A1,x
    and z:zTemp00
    sta a:$39A1,x
    lda a:$39C0,x
    and z:zTemp00
    sta a:$39C0,x
    lda a:$39C1,x
    and z:zTemp00
    sta a:$39C1,x
    lda a:OBJ_HITBOX_DY,x
    and z:zTemp00
    sta a:OBJ_HITBOX_DY,x
    lda a:OBJ_HITBOX_DY+1,x
    and z:zTemp00
    sta a:OBJ_HITBOX_DY+1,x
    inx
    inx
    dey
    bne StageOpeningCopyRow152D_at6

; @routine 152E:5B8 StageOpeningWriteRow152D
; @run 6B:8DB8
; @unreachable 6B:8DB8-6B:8DCA no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8DB8 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Writes the row's 64 words from the buffer at $397E back through the VDC's data port.
; HOW: A loop down X by two.
; WAITS: no.
; Entry: the VDC pointed at the row. Exit: X 0, Z set; A the last high byte written. Y, C and V kept.
StageOpeningWriteRow152D:
    ldx #$80
StageOpeningWriteRow152D_at0:
    lda a:$397F,x
    sta a:VDC_DATA_LO
    lda a:$397E,x
    sta a:VDC_DATA_HI
    dex
    dex
    bne StageOpeningWriteRow152D_at0
    rts

; @routine 152E:5CB StageDrawStock152D
; @run 6B:8DCB
; @unreachable 6B:8DCB-6B:8DD3 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8DCB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Draws the stock zStock (two digits at places 0 and 1).
; HOW: StageDrawNumber152D with X 0, X kept on the stack across it.
; WAITS: no.
; Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber152D left them.
StageDrawStock152D:
    phx
    lda z:zStock
    clx
    jsr StageDrawNumber152D
    plx
    rts

; @routine 152E:5D4 StageDrawScoreAdd152D
; @run 6B:8DD4
; @unreachable 6B:8DD4-6B:8DE4 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8DD4 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Draws the points to add, zScoreAddLo and zScoreAddHi (four digits from place 3).
; HOW: StageDrawNumber152D with X 3 and 5, X kept on the stack across them.
; WAITS: no.
; Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber152D left them.
StageDrawScoreAdd152D:
    phx
    lda z:zScoreAddLo
    ldx #$03
    jsr StageDrawNumber152D
    lda z:zScoreAddHi
    ldx #$05
    jsr StageDrawNumber152D
    plx
    rts

; @routine 152E:5E5 StageDrawScore152D
; @run 6B:8DE5
; @unreachable 6B:8DE5-6B:8DFC no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8DE5 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Draws the score zScore0-2 (six digits from place 1).
; HOW: StageDrawNumber152D with X 1, 3 and 5, X kept on the stack across them.
; WAITS: no.
; Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber152D left them.
StageDrawScore152D:
    phx
    lda z:zScore0
    ldx #$01
    jsr StageDrawNumber152D
    lda z:zScore1
    ldx #$03
    jsr StageDrawNumber152D
    lda z:zScore2
    ldx #$05
    jsr StageDrawNumber152D
    plx
    rts

; @routine 152E:5FD StageDrawNumber152D
; @run 6B:8DFD
; @unreachable 6B:8DFD-6B:8E0D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8DFD runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Draws the two decimal digits of A at the places X and X + 1, the low digit first.
; HOW: StageDrawDigit152D with the low nibble, X and A kept on the stack; then a jump to it with the high nibble.
; WAITS: no.
; Entry: A the number, X the place. Exit: as StageDrawDigit152D.
StageDrawNumber152D:
    phx
    pha
    and #$0F
    jsr StageDrawDigit152D
    pla
    lsr a
    lsr a
    lsr a
    lsr a
    plx
    inx
    jmp StageDrawDigit152D

; @routine 152E:60E StageUnpackList152D
; @run 6B:8E0E
; @unreachable 6B:8E0E-6B:8E71 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8E0E runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Unpacks the route's list (the table at $9A62 by zField82) into memory: SaveBuffer's 767 bytes cleared
; from KernelZeros and its first 8 taken from the list; then records: 0 sets the target address (the next two bytes),
; $FF ends, any other count fills that many bytes from the target with the next byte and moves the target on.
; HOW: TAI; the list in zTemp00-01, the target in zTemp02-03, the count in zTemp04; loops of reads and writes
; through the pointers.
; WAITS: no.
; Entry: nothing. Exit: A $FF, Z set, C set; X 0 or as it was, Y as the last record left it. Clobbers zTemp00-04.
StageUnpackList152D:
    tai KernelZeros,SaveBuffer,$02FF
    lda z:zField82
    asl a
    tay
    lda a:$9A62,y
    sta z:zTemp00
    lda a:$9A63,y
    sta z:zTemp01
    cly
StageUnpackList152D_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne StageUnpackList152D_at0
    lda #$08
    bra StageUnpackList152D_at1
StageUnpackList152D_at5:
    lda (zTemp00)
    bne StageUnpackList152D_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra StageUnpackList152D_at1
StageUnpackList152D_at2:
    cmp #$FF
    bne StageUnpackList152D_at3
    rts
StageUnpackList152D_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
StageUnpackList152D_at4:
    sta (zTemp02),y
    iny
    dex
    bne StageUnpackList152D_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    cla
    adc z:zTemp03
    sta z:zTemp03
    lda #$02
StageUnpackList152D_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    cla
    adc z:zTemp01
    sta z:zTemp01
    bra StageUnpackList152D_at5

; @routine 152E:672 ClearBitRowsFour152D
; @run 6B:8E72
; @unreachable 6B:8E72-6B:8E77 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8E72 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs StageClearBitRows152D four times: three calls and the run into it.
; HOW: Three BSR and the bytes of StageClearBitRows152D after them.
; WAITS: no.
; Entry, exit: as StageClearBitRows152D.
ClearBitRowsFour152D:
    bsr StageClearBitRows152D
    bsr StageClearBitRows152D
    bsr StageClearBitRows152D

; @routine 152E:678 StageClearBitRows152D
; @run 6B:8E78
; @unreachable 6B:8E78-6B:8ED1 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8E78 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Clears one bit of four VRAM words $10 apart, for the map entry zTemp0F names: its low nibble picks the
; bit (the masks at $8ED2 and $8EDA), its high nibble is added to the base in zTemp08-09; then zTemp0F moves on by
; $23. Each word is pointed at by the kernel's pieces at $4024, $4021 and $4027 and ANDed in place.
; HOW: The address in zTemp0A-0B, the count in zTemp0E; the VDC's data port.
; WAITS: no.
; Entry: zTemp08-09 the base, zTemp0F the entry. Exit: A the new zTemp0F, N, Z, C and V from its add; X as $4027
; left it; Y the bit.
StageClearBitRows152D:
    lda z:zTemp0F
    and #$0F
    tay
    lda z:zTemp0F
    lsr a
    lsr a
    lsr a
    lsr a
    clc
    adc z:zTemp08
    sta z:zTemp0A
    lda z:zTemp09
    sta z:zTemp0B
    lda #$04
    sta z:zTemp0E
StageClearBitRows152D_at0:
    jsr VecSelectVramReadAddress
    lda z:zTemp0A
    sta a:VDC_DATA_LO
    lda z:zTemp0B
    sta a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    lda z:zTemp0A
    sta a:VDC_DATA_LO
    lda z:zTemp0B
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    and a:$8ED2,y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and a:$8EDA,y
    sta a:VDC_DATA_HI
    lda z:zTemp0A
    clc
    adc #$10
    sta z:zTemp0A
    dec z:zTemp0E
    bne StageClearBitRows152D_at0
    lda z:zTemp0F
    clc
    adc #$23
    sta z:zTemp0F
    rts

; @routine 152E:6EA ContinueStep152D
; @run 6B:8EEA
; @stop 8EF1
; @unreachable 6B:8EEA-6B:8EF0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8EEA runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The continue screen's frame: the step zGameSubMode holds, from the four at $8EF1 (the start, the
; reveal, the menu's start, the menu).
; HOW: zGameSubMode doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zGameSubMode, N and Z from it, C its
; bit 7.
ContinueStep152D:
    lda z:zGameSubMode
    asl a
    tax
    jmp ($8EF1,x)

; @routine 152E:6F9 ContinueStart152D
; @run 6B:8EF9
; @unreachable 6B:8EF9-6B:8F5A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8EF9 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The continue screen's start: sound $36, zRasterStage cleared; for each of the eight rows (7 down to 0)
; its 64 VRAM words at $6E00 + row x $40 cleared, the kernel's piece at $4054 with X the row, and the row's object
; placed (its count at $32C8 twice the row, sprite $0D up at $2A58, set 5, X $48, $2C50 $59); then the kernel's piece
; at $4012 with A $0D, and the next step.
; HOW: The row in zTemp10; the address through zTemp00-01 and the kernel's pieces at $4021 and $4027; ST1 and ST2
; in a loop.
; WAITS: no.
; Entry: nothing. Exit: N and Z from zGameSubMode's increment; A, X, Y, C and V as $4012 left them.
ContinueStart152D:
    lda #$36
    jsr VecSoundCommand
    stz z:zRasterStage
    lda #$07
    sta z:zTemp10
ContinueStart152D_at1:
    stz z:zTemp00
    lda z:zTemp10
    lsr a
    ror z:zTemp00
    lsr a
    ror z:zTemp00
    sta z:zTemp01
    jsr VecSelectVramWriteAddress
    lda #$00
    clc
    adc z:zTemp00
    sta a:VDC_DATA_LO
    lda #$6E
    adc z:zTemp01
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    cly
ContinueStart152D_at0:
    st1 #$00
    st2 #$00
    iny
    cpy #$40
    bne ContinueStart152D_at0
    ldx z:zTemp10
    jsr VecClearObject
    txa
    asl a
    sta a:$32C8,x
    txa
    clc
    adc #$0D
    sta a:$2A58,x
    lda #$05
    sta a:OBJ_SPRITE_SET,x
    lda #$48
    sta a:OBJ_X_HI+8,x
    lda #$59
    sta a:$2C50,x
    dec z:zTemp10
    bpl ContinueStart152D_at1
    lda #$0D
    jsr VecStartScriptSlot0Kind1
    inc z:zGameSubMode
    rts

; @routine 152E:75B ContinueReveal152D
; @run 6B:8F5B
; @unreachable 6B:8F5B-6B:8F7D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8F5B runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The continue screen's reveal: Run pressed once the last row's count at $32CF is $11 ends it; else each
; row from 7 down steps (ContinueRowStep152D), and the last row's count is moved from $10 to $11.
; HOW: A pad bit and a compare; BSR in a loop down X.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them, X $FF after the rows; A the count, N, Z and C from its
; compare.
ContinueReveal152D:
    lda a:BiosJoyTrigger
    and #$08
    beq ContinueReveal152D_at0
    lda a:$32CF
    cmp #$11
    bne ContinueReveal152D_at0
    inc z:zGameSubMode
    rts
ContinueReveal152D_at0:
    ldx #$07
ContinueReveal152D_at1:
    bsr ContinueRowStep152D
    dex
    bpl ContinueReveal152D_at1
    lda a:$32CF
    cmp #$10
    bne ContinueRowStep152D_at2
    inc a:$32CF
    rts

; @routine 152E:77E ContinueDeadFade152D
; @run 6B:8F7E
; @unreachable 6B:8F7E-6B:8F82 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8F7E runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The card's CD fade of type $0C. No code of the overlay reaches these bytes (they follow the RTS of
; ContinueReveal152D); the C is the bytes' own.
; HOW: A load and a jump to cd_fade.
; WAITS: no.
; Entry: nothing. Exit: as cd_fade: A the nibble, N and Z from it; X, Y, C and V kept.
ContinueDeadFade152D:
    lda #$0C
    jmp BiosCdFade

; @routine 152E:783 ContinueRowStep152D
; @run 6B:8F83
; @unreachable 6B:8F83-6B:9014 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $8F83 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One row of the continue screen's reveal, X the row. While its byte at $2818 is 0 its object slides
; right by 8 a frame as its count at $32C8 runs down, and then the byte is set. After that, while the count is below
; $10: the row's word at $6E0E less the count is cleared (not at $0F), the stage's banks $10 and $11 mapped, four
; columns of the row's picture (from $B29E, a row's 128 bytes) drawn by ContinueRowDraw152D, $2C50 and the count
; counted.
; HOW: Tests and stores in the row's columns; the address in zTemp00-01, the picture in zTemp08-09; the kernel's
; pieces at $4021 and $4027; TAM.
; WAITS: no.
; Entry: X the row. Exit: as the last instruction that ran left them; X kept.
ContinueRowStep152D:
    lda a:$2818,x
    bne ContinueRowStep152D_at0
    lda a:$32C8,x
    bne ContinueRowStep152D_at1
    inc a:$2818,x
    rts
ContinueRowStep152D_at1:
    lda a:OBJ_X_HI+8,x
    clc
    adc #$08
    sta a:OBJ_X_HI+8,x
    dec a:$32C8,x
ContinueRowStep152D_at2:
    rts
ContinueRowStep152D_at0:
    lda a:$32C8,x
    cmp #$10
    bcs ContinueRowStep152D_at2
    stz z:zTemp00
    txa
    lsr a
    ror z:zTemp00
    lsr a
    ror z:zTemp00
    sta z:zTemp01
    lda #$00
    clc
    adc z:zTemp00
    sta z:zTemp00
    lda #$6E
    adc z:zTemp01
    sta z:zTemp01
    lda a:$32C8,x
    cmp #$0F
    beq ContinueRowStep152D_at3
    jsr VecSelectVramWriteAddress
    lda z:zTemp00
    clc
    adc #$0E
    sec
    sbc a:$32C8,x
    sta a:VDC_DATA_LO
    lda z:zTemp01
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    st1 #$00
    st2 #$00
ContinueRowStep152D_at3:
    lda #$10
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    stz z:zTemp08
    txa
    lsr a
    ror z:zTemp08
    sta z:zTemp09
    lda #$9E
    clc
    adc z:zTemp08
    sta z:zTemp08
    lda #$B2
    adc z:zTemp09
    sta z:zTemp09
    lda #$0F
    bsr ContinueRowDraw152D
    lda #$1F
    bsr ContinueRowDraw152D
    lda #$2F
    bsr ContinueRowDraw152D
    lda #$3F
    bsr ContinueRowDraw152D
    inc a:$2C50,x
    inc a:$32C8,x
    rts

; @routine 152F:015 ContinueRowDraw152D
; @run 6B:9015
; @unreachable 6B:9015-6B:9041 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9015 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: One word of a continue row's picture: the VDC pointed at the row's word A less its count, and the
; picture's word there (from zTemp08-09, two bytes a word) written.
; HOW: The column in zTemp10; the kernel's pieces at $4021 and $4027; reads through the picture's pointer.
; WAITS: no.
; Entry: A the column, X the row, zTemp00-01 the row's address, zTemp08-09 the picture. Exit: A the word's high byte,
; N and Z from it, Y its offset; C and V from the subtract. X as $4027 left it.
ContinueRowDraw152D:
    sta z:zTemp10
    jsr VecSelectVramWriteAddress
    lda z:zTemp00
    clc
    adc z:zTemp10
    sec
    sbc a:$32C8,x
    sta a:VDC_DATA_LO
    lda z:zTemp01
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda z:zTemp10
    sec
    sbc a:$32C8,x
    asl a
    tay
    lda (zTemp08),y
    sta a:VDC_DATA_LO
    iny
    lda (zTemp08),y
    sta a:VDC_DATA_HI
    rts

; @routine 152F:042 ContinueMenuStart152D
; @run 6B:9042
; @unreachable 6B:9042-6B:908A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9042 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The continue menu's start: the kernel's piece at $4057, zField90, zField91, zRasterEffect and the
; scroll's X and Y cleared; the kernel's piece at $4054 for object 8 (its loop ends after one) and for $44 to $47;
; the next step; the kernel's pieces at $4012 (A $0E) and $400C (A $6C); the menu's objects (sprites 4 and 3, X $4F)
; and the choice ContinueChoice 0, the pointer placed.
; HOW: Stores; two loops of the kernel's piece; a branch into ContinueMenu152D's pointer code at $90B6.
; WAITS: no.
; Entry: nothing. Exit: A the pointer's X, N and Z from it, X the choice; Y, C and V as $400C left them.
ContinueMenuStart152D:
    jsr VecClearEnemySlots
    stz z:zField90
    stz z:zField91
    stz z:zRasterEffect
    stz a:ScrollXLo
    stz a:ScrollXHi
    stz a:ScrollYLo
    stz a:ScrollYHi
    ldx #$08
ContinueMenuStart152D_at0:
    jsr VecClearObject
    inx
    cpx #$10
    bpl ContinueMenuStart152D_at0
    ldx #$44
ContinueMenuStart152D_at1:
    jsr VecClearObject
    inx
    cpx #$48
    bne ContinueMenuStart152D_at1
    inc z:zGameSubMode
    lda #$0E
    jsr VecStartScriptSlot0Kind1
    lda #$6C
    jsr VecStartListChannel0Kind2
    lda #$04
    sta a:$2A60
    lda #$03
    sta a:OBJ_SPRITE_SET+8
    lda #$4F
    sta a:$2B80
    stz a:ContinueChoice
    bra ContinueMenu152D_at3

; @routine 152F:08B ContinueMenu152D
; @run 6B:908B
; @unreachable 6B:908B-6B:90C7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $908B runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The continue menu: the cursor's blink; button I or Run takes the choice (sound $FF, zModeDone
; counted); up, down or Select moves ContinueChoice (the table at $90C8, two on for down), with the kernel's piece at
; $4012 for the choice's text and sound 1, and the pointer moved.
; HOW: ContinueBlink152D; pad bits; a table read and an add; the kernel and the sound hook.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction that ran left them.
ContinueMenu152D:
    jsr ContinueBlink152D
    lda a:BiosJoyTrigger
    and #$09
    bne ContinueMenu152D_at0
    lda a:BiosJoyTrigger
    and #$54
    beq ContinueMenu152D_at1
    ldx a:ContinueChoice
    and #$10
    bne ContinueMenu152D_at2
    inx
    inx
ContinueMenu152D_at2:
    lda a:$90C8,x
    sta a:ContinueChoice
    clc
    adc #$0E
    jsr VecStartScriptSlot0Kind1
    lda #$01
    jsr VecSoundCommand
ContinueMenu152D_at3:
    ldx a:ContinueChoice
    lda a:$90CD,x
    sta a:$2C58
    rts
ContinueMenu152D_at0:
    lda #$FF
    jsr VecSoundCommand
    inc z:zModeDone
ContinueMenu152D_at1:
    rts

; @routine 152F:0D0 ContinueBlink152D
; @run 6B:90D0
; @unreachable 6B:90D0-6B:90ED no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $90D0 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Every fourth frame the kernel's piece at $4018 with X 0 and A the next of the ten blink colours at
; $90EE (the step at $32D0, back to 0 after 9).
; HOW: zFrameTick's low two bits; a table read; the kernel; an increment and a compare.
; WAITS: no.
; Entry: nothing. Exit: A the step or zFrameTick's bits, N, Z and C from the last test; X and Y as $4018 left them.
ContinueBlink152D:
    lda z:zFrameTick
    and #$03
    bne ContinueBlink152D_at0
    ldy a:$32D0
    lda a:$90EE,y
    clx
    jsr VecStartScriptSlotKind4
    inc a:$32D0
    lda a:$32D0
    cmp #$0A
    bcc ContinueBlink152D_at0
    stz a:$32D0
ContinueBlink152D_at0:
    rts

; @routine 152F:0F8 StageSubModeTally152D
; @run 6B:90F8
; @stop 9108
; @unreachable 6B:90F8-6B:9107 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $90F8 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Sub-mode 6, the tally after the boss: the events step, the display, the overlays' frame, then the
; tally's step at zField80 from the eight at $9108 (the start, the board, the board's reveal, a pause, the health and
; hearts counted into the score, a wait, the stock bonus, the end).
; HOW: StageModeEventsStep152D, StageHudStep152D, StageOverlayFrame152D; zField80 doubled indexes the table, whose
; word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zField80, N and Z from it, C its
; bit 7.
StageSubModeTally152D:
    jsr StageModeEventsStep152D
    jsr StageHudStep152D
    jsr StageOverlayFrame152D
    lda z:zField80
    asl a
    tax
    jmp ($9108,x)

; @routine 152F:118 TallyStart152D
; @run 6B:9118
; @unreachable 6B:9118-6B:9122 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9118 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The tally's first step, after the wait: the kernel's piece at $4087, sound $32, and the next step
; (runs on into TallyNext152D).
; HOW: TallyWait152D; $4087; the sound hook.
; WAITS: no.
; Entry: nothing. Exit: as TallyWait152D when it ends the step; else as TallyNext152D.
TallyStart152D:
    jsr TallyWait152D
    jsr VecStopSound
    lda #$32
    jsr VecSoundCommand

; @routine 152F:123 TallyNext152D
; @run 6B:9123
; @unreachable 6B:9123-6B:9125 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9123 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The tally's next step: zField80 counted.
; HOW: An increment.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the increment; A, X, Y, C and V kept.
TallyNext152D:
    inc z:zField80
    rts

; @routine 152F:126 TallyBoard152D
; @run 6B:9126
; @unreachable 6B:9126-6B:916F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9126 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The tally's board: the kernel's piece at $4012 (A $16), the display's pieces 5 to 7
; (StageDrawPiece152D), the hearts kept at $330D, the full-health mark ($2A9B $18 at zPlayerHealth $5C), three
; blocks of VRAM words cleared (StageClearWords152D: $0E at $5000, $0C at $5400, $0A at $5D80), the board's cells
; at $3980 from the 62 at $93B9; the next step (TallyNext152D), then runs on into TallyDigits152D.
; HOW: Calls; a copy loop.
; WAITS: no.
; Entry: nothing. Exit: as TallyDigits152D.
TallyBoard152D:
    lda #$16
    jsr VecStartScriptSlot0Kind1
    ldx #$05
    jsr StageDrawPiece152D
    inx
    jsr StageDrawPiece152D
    inx
    jsr StageDrawPiece152D
    lda z:zField94
    sta a:$330D
    lda z:zPlayerHealth
    cmp #$5C
    bne TallyBoard152D_at0
    lda #$18
    sta a:$2A9B
TallyBoard152D_at0:
    clx
    ldy #$50
    lda #$0E
    jsr StageClearWords152D
    clx
    ldy #$54
    lda #$0C
    jsr StageClearWords152D
    ldx #$80
    ldy #$5D
    lda #$0A
    jsr StageClearWords152D
    clx
TallyBoard152D_at1:
    lda a:$93B9,x
    sta a:$3980,x
    inx
    cpx #$3E
    bne TallyBoard152D_at1
    jsr TallyNext152D

; @routine 152F:170 TallyDigits152D
; @run 6B:9170
; @unreachable 6B:9170-6B:9197 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9170 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Puts the health and the hearts on the board as digits: zPlayerHealth's tens and units
; (StageSplitTens152D) at $3991 and $3992, zField94's two BCD digits at $39A1 and $39A2, each one more than the digit;
; a leading zero is left out.
; HOW: StageSplitTens152D; shifts and masks; stores.
; WAITS: no.
; Entry: nothing. Exit: A the hearts' last cell, N and Z from its increment; X the health's tens cell; C from the
; compare, V from the split's add. Y kept.
TallyDigits152D:
    lda z:zPlayerHealth
    jsr StageSplitTens152D
    inx
    cpx #$01
    beq TallyDigits152D_at0
    stx a:$3991
TallyDigits152D_at0:
    inc a
    sta a:$3992
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    inc a
    cmp #$01
    beq TallyDigits152D_at1
    sta a:$39A1
TallyDigits152D_at1:
    lda z:zField94
    and #$0F
    inc a
    sta a:$39A2
    rts

; @routine 152F:198 TallyReveal152D
; @run 6B:9198
; @unreachable 6B:9198-6B:922B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9198 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The board's reveal, under the stage's banks: once all 62 cells at $3980 are 0 the reveal is done
; (zPaused's bit 1 set, the wait at $3310 $40, the next step). Else a random cell that is not yet 0 is stepped by $40
; (cleared when it carries), and drawn: a cell of the first ten columns as its 16-word glyph from the picture at $B77E
; (rows from $5D80); the others by TallyCell152D.
; HOW: MapStageBanks; a loop down X; the kernel's random piece at $4051 until it gives a live cell; shifts into
; zTemp08-09, the glyph's address in zTemp10-11; SetVramWriteAddress152D; a loop of 16 words with SXY.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction that ran left them.
TallyReveal152D:
    lda #$10
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    ldx #$3D
TallyReveal152D_at1:
    lda a:$3980,x
    bne TallyReveal152D_at0
    dex
    bpl TallyReveal152D_at1
    smb1 z:zPaused
    lda #$40
    sta a:$3310
    jmp TallyNext152D
TallyReveal152D_at0:
    jsr VecNextRandom
    and #$3F
    cmp #$3E
    bcs TallyReveal152D_at0
    tax
    lda a:$3980,x
    beq TallyReveal152D_at0
    sta z:zTemp00
    adc #$40
    sta a:$3980,x
    bcc TallyReveal152D_at2
    stz a:$3980,x
TallyReveal152D_at2:
    txa
    and #$3F
    cmp #$0A
    bcs TallyCell152D
    stz z:zTemp08
    lsr a
    ror z:zTemp08
    lsr a
    ror z:zTemp08
    sta z:zTemp09
    lda z:zTemp00
    lsr a
    lsr a
    and #$30
    clc
    adc #$80
    clc
    adc z:zTemp08
    tax
    lda #$5D
    adc z:zTemp09
    tay
    lda z:zTemp00
    dec a
    and #$3F
    stz z:zTemp08
    lsr a
    ror z:zTemp08
    lsr a
    ror z:zTemp08
    lsr a
    ror z:zTemp08
    sta z:zTemp09
    lda #$7E
    clc
    adc z:zTemp08
    sta z:zTemp10
    lda #$B7
    adc z:zTemp09
    sta z:zTemp11
    jsr SetVramWriteAddress152D
    cly
    ldx #$10
TallyReveal152D_at3:
    sxy
    lda (zTemp10),y
    sta a:VDC_DATA_LO
    sxy
    lda (zTemp10),y
    sta a:VDC_DATA_HI
    iny
    inx
    cpy #$10
    bne TallyReveal152D_at3
    rts

; @routine 152F:22C TallyCell152D
; @run 6B:922C
; @unreachable 6B:922C-6B:92B0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $922C runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Draws one of the board's cells past the first ten columns: A the cell's index, zTemp00 its value
; (its top two bits the shade, its low six the glyph). The cell's place gives the VRAM word (rows of eight from $5008,
; odd and even halves) and which of the word's bytes it fills; eight bytes of the glyph from the picture at $B69E
; (eight a glyph) go into that byte, the other byte written back as it was.
; HOW: A subtract and shifts into zTemp01-02 and zTemp08-09, the glyph's address in zTemp10-11; SetVramAddresses152D;
; a loop of eight reads and writes of the VDC's data port.
; WAITS: no.
; Entry: A the cell's index ($0A up), C set; zTemp00 the cell's value. Exit: Y 8, Z set, C set; A the last byte read
; or written; X the VRAM address's low byte. Clobbers zTemp01-02, zTemp08-11.
TallyCell152D:
    sec
    sbc #$0A
    tax
    sta z:zTemp01
    lsr a
    and #$1C
    bbr2 zTemp01,TallyCell152D_at0
    ora #$20
TallyCell152D_at0:
    sta z:zTemp02
    lda z:zTemp01
    and #$03
    ora z:zTemp02
    sta z:zTemp01
    stz z:zTemp08
    lsr a
    lsr z:zTemp08
    lsr a
    ror z:zTemp08
    lsr a
    ror z:zTemp08
    sta z:zTemp09
    lda z:zTemp00
    lsr a
    lsr a
    and #$30
    clc
    adc #$08
    clc
    adc z:zTemp08
    tax
    lda #$50
    adc z:zTemp09
    tay
    lda z:zTemp00
    dec a
    and #$3F
    stz z:zTemp09
    asl a
    rol z:zTemp09
    asl a
    rol z:zTemp09
    asl a
    rol z:zTemp09
    sta z:zTemp08
    lda #$9E
    clc
    adc z:zTemp08
    sta z:zTemp10
    lda #$B6
    adc z:zTemp09
    sta z:zTemp11
    jsr SetVramAddresses152D
    cly
    bbs0 zTemp01,TallyCell152D_at1
TallyCell152D_at2:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lda (zTemp10),y
    sta a:VDC_DATA_HI
    iny
    cpy #$08
    bne TallyCell152D_at2
    rts
TallyCell152D_at1:
    lda a:VDC_DATA_LO
    lda (zTemp10),y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    iny
    cpy #$08
    bne TallyCell152D_at1
TallyCell152D_at3:
    rts

; @routine 152F:2B1 TallyPause152D
; @run 6B:92B1
; @unreachable 6B:92B1-6B:92BB no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $92B1 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A pause of the tally: after the wait, the next wait at $3310 set to $40 and the next step.
; HOW: TallyWait152D; a store; a jump to TallyNext152D.
; WAITS: no.
; Entry: nothing. Exit: as TallyWait152D when it ends the step; else as TallyNext152D, A $40.
TallyPause152D:
    jsr TallyWait152D
    lda #$40
    sta a:$3310
    jmp TallyNext152D

; @routine 152F:2BC TallyCount152D
; @run 6B:92BC
; @unreachable 6B:92BC-6B:9311 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $92BC runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The tally's count: the paused frame's menu step with zPaused's bit 0 set for it; while zPaused's bit 1
; is clear the health is counted into the score (zPlayerHealth down by one, its cells redrawn by TallyHealth152D, 1
; point by the kernel's piece at $408A), then after the wait the hearts (zField94 down by one in BCD, TallyHearts152D,
; $10 points); the score redrawn and sound 1 every fourth event. With both at 0 the tally goes on two steps, or, when
; the full-health mark at $2A9B is set, stays with zPaused's bit 1 set and the wait at $3310 $40.
; HOW: Bit changes; StagePausedFrame152D; decrements; $408A; StageDrawScore152D; the sound hook.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them.
TallyCount152D:
    smb0 z:zPaused
    jsr StagePausedFrame152D
    rmb0 z:zPaused
    bbs1 zPaused,TallyCell152D_at3
    lda z:zPlayerHealth
    beq TallyCount152D_at0
    dec z:zPlayerHealth
    jsr TallyHealth152D
    lda #$01
    bra TallyCount152D_at1
TallyCount152D_at0:
    jsr TallyWait152D
    lda z:zField94
    beq TallyCount152D_at2
    sed
    sec
    sbc #$01
    sta z:zField94
    cld
    jsr TallyHearts152D
    lda #$10
TallyCount152D_at1:
    jsr VecAddScoreByte
    jsr StageDrawScore152D
    lda z:zModeEvents
    and #$03
    bne TallyCount152D_at3
    lda #$01
    jsr VecSoundCommand
TallyCount152D_at3:
    rts
TallyCount152D_at2:
    dec a:$3310
    inc z:zField80
    inc z:zField80
    lda a:$2A9B
    beq TallyCount152D_at4
    dec z:zField80
    dec z:zField80
    smb1 z:zPaused
    lda #$40
    sta a:$3310
TallyCount152D_at4:
    jmp TallyNext152D

; @routine 152F:312 TallyWaitPress152D
; @run 6B:9312
; @unreachable 6B:9312-6B:9323 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9312 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: A wait of the tally: after the wait, the paused frame's menu step; once zPaused's bit 1 is clear the
; wait at $3310 is $20 and the next step.
; HOW: TallyWait152D; StagePausedFrame152D; a bit test; a store and a jump to TallyNext152D.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them.
TallyWaitPress152D:
    jsr TallyWait152D
    jsr StagePausedFrame152D
    bbr1 zPaused,TallyWaitPress152D_at0
    rts
TallyWaitPress152D_at0:
    lda #$20
    sta a:$3310
    jmp TallyNext152D

; @routine 152F:324 TallyStockBonus152D
; @run 6B:9324
; @unreachable 6B:9324-6B:9341 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9324 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The stock bonus: after the wait, zStock up by one in BCD (kept at 99), redrawn (StageDrawStock152D),
; sound $2A, the wait at $3310 one less and the next step.
; HOW: TallyWait152D; SED, ADC, CLD; StageDrawStock152D; the sound hook; a decrement; a jump to TallyNext152D.
; WAITS: no.
; Entry: D clear, as the game keeps it. Exit: as TallyNext152D.
TallyStockBonus152D:
    jsr TallyWait152D
    sed
    lda z:zStock
    clc
    adc #$01
    bcc TallyStockBonus152D_at0
    lda #$99
TallyStockBonus152D_at0:
    sta z:zStock
    cld
    jsr StageDrawStock152D
    lda #$2A
    jsr VecSoundCommand
    dec a:$3310
    jmp TallyNext152D

; @routine 152F:342 TallyEnd152D
; @run 6B:9342
; @unreachable 6B:9342-6B:934C no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9342 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The tally's end: after the wait, zField94 back from the hearts kept at $330D and zModeDone counted.
; HOW: TallyWait152D; a store; an increment.
; WAITS: no.
; Entry: nothing. Exit: A the hearts, N and Z from zModeDone's increment.
TallyEnd152D:
    jsr TallyWait152D
    lda a:$330D
    sta z:zField94
    inc z:zModeDone
    rts

; @routine 152F:34D TallyWait152D
; @run 6B:934D
; @unreachable 6B:934D-6B:9357 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $934D runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The tally's wait: while the count at $3310 is not 0 it is counted down and the caller's step ends here,
; this routine dropping its caller's return address so its RTS ends the caller too.
; HOW: A test; a decrement; two PLA.
; WAITS: no.
; Entry: nothing. Exit, count 0: A 0, N and Z from it, the rest kept. Else: the frame dropped, A the return address's
; high byte less one, N and Z from it.
TallyWait152D:
    lda a:$3310
    beq TallyWait152D_at0
    dec a:$3310
    pla
    pla
TallyWait152D_at0:
    rts

; @routine 152F:358 TallyHealth152D
; @run 6B:9358
; @unreachable 6B:9358-6B:937B no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9358 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Redraws the board's health: the stage's banks mapped, the digits put in (TallyDigits152D), the units'
; cell $12 stepped (TallyDigitCell152D); from 10 up the tens' cell $11 too, below it the tens' place cleared.
; HOW: MapStageBanks; TallyDigits152D; TallyDigitCell152D by BSR; a compare; a branch to TallyDigitCell152D or a jump
; to StageClearWords152D.
; WAITS: no.
; Entry: nothing. Exit: as TallyDigitCell152D or StageClearWords152D.
TallyHealth152D:
    lda #$10
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr TallyDigits152D
    ldx #$12
    bsr TallyDigitCell152D
    lda z:zPlayerHealth
    cmp #$0A
    bcc TallyHealth152D_at0
    ldx #$11
    bra TallyDigitCell152D
TallyHealth152D_at0:
    ldx #$40
    ldy #$54
    lda #$01
    jmp StageClearWords152D

; @routine 152F:37C TallyHearts152D
; @run 6B:937C
; @unreachable 6B:937C-6B:939F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $937C runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Redraws the board's hearts as TallyHealth152D does the health: the units' cell $22, and from $10 up
; (BCD) the tens' cell $21, below it the tens' place cleared.
; HOW: MapStageBanks; TallyDigits152D; TallyDigitCell152D by BSR; a compare; a branch to TallyDigitCell152D or a jump
; to StageClearWords152D.
; WAITS: no.
; Entry: nothing. Exit: as TallyDigitCell152D or StageClearWords152D.
TallyHearts152D:
    lda #$10
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr TallyDigits152D
    ldx #$22
    bsr TallyDigitCell152D
    lda z:zField94
    cmp #$10
    bcc TallyHearts152D_at0
    ldx #$21
    bra TallyDigitCell152D
TallyHearts152D_at0:
    ldx #$40
    ldy #$55
    lda #$01
    jmp StageClearWords152D

; @routine 152F:3A0 TallyDigitCell152D
; @run 6B:93A0
; @unreachable 6B:93A0-6B:93B8 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $93A0 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Draws the board's cell X at every shade from its value up: TallyCell152D for the cell, then the cell
; stepped by $40 until it carries.
; HOW: X kept on the stack across TallyCell152D; an add; a loop.
; WAITS: no.
; Entry: X the cell. Exit: A the cell plus $40, C set, N and Z from it; X the cell; Y 8.
TallyDigitCell152D:
    phx
    lda a:$3980,x
    sta z:zTemp00
    txa
    jsr TallyCell152D
    plx
    lda a:$3980,x
    clc
    adc #$40
    bcs TallyDigitCell152D_at0
    sta a:$3980,x
    bra TallyDigitCell152D
TallyDigitCell152D_at0:
    rts

; @routine 152F:3F7 StageClearWords152D
; @run 6B:93F7
; @unreachable 6B:93F7-6B:9409 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $93F7 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Clears A rows of 64 VRAM words from the address in X and Y.
; HOW: The count in zTemp00; SetVramWriteAddress152D; ST1 and ST2 in loops.
; WAITS: no.
; Entry: A the rows, X and Y the address. Exit: X 0, A as SetVramWriteAddress152D left it, N and Z from the count's
; decrement (Z set). Y, C and V as SetVramWriteAddress152D left them. Clobbers zTemp00.
StageClearWords152D:
    sta z:zTemp00
    jsr SetVramWriteAddress152D
StageClearWords152D_at2:
    ldx #$40
StageClearWords152D_at1:
    st1 #$00
    st2 #$00
    dex
    bne StageClearWords152D_at1
StageClearWords152D_at0:
    dec z:zTemp00
    bne StageClearWords152D_at2
    rts

; @routine 152F:40A StageSplitTens152D
; @run 6B:940A
; @unreachable 6B:940A-6B:9414 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $940A runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Splits A into its tens (X) and its units (A) by repeated subtracts of ten.
; HOW: A loop of SBC #$0A, X counted; the last subtract undone.
; WAITS: no.
; Entry: A the number. Exit: A the units, X the tens, N and Z from X's decrement; C and V from the add that undid the
; last subtract. Y kept.
StageSplitTens152D:
    clx
StageSplitTens152D_at0:
    inx
    sec
    sbc #$0A
    bcs StageSplitTens152D_at0
    adc #$0A
    dex
    rts

; @routine 152F:415 SetVramAddresses152D
; @run 6B:9415
; @unreachable 6B:9415-6B:941D no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9415 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Points both of the VDC's VRAM addresses at the word in X and Y, then runs on into
; SetVramWriteAddress152D. The bytes are those of SetVramAddresses (map_draw.c) at this overlay's address, where
; $941E is an entry of its own.
; HOW: The kernel's piece at $4024 (the read address), X and Y into the data port; then SetVramWriteAddress152D.
; WAITS: no.
; Entry: X low byte, Y high byte of the VRAM address. Exit: as SetVramWriteAddress152D.
SetVramAddresses152D:
    jsr VecSelectVramReadAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI

; @routine 152F:41E SetVramWriteAddress152D
; @run 6B:941E
; @unreachable 6B:941E-6B:9429 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $941E runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Points the VDC's VRAM write address at the word in X and Y and selects the data port.
; HOW: The kernel's piece at $4021, X and Y into the data port; a jump to the kernel's piece at $4027.
; WAITS: no.
; Entry: X low byte, Y high byte of the VRAM address. Exit: as $4027 leaves them.
SetVramWriteAddress152D:
    jsr VecSelectVramWriteAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jmp VecSelectVramData

; @routine 152F:448 StageDrawPiece152D
; @run 6B:9448
; @unreachable 6B:9448-6B:9467 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9448 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Places one of the display's pieces, X from 0 to 7: the kernel's piece at $405D gives a slot for the
; object the table at $9468 names, and the slot takes its sprite, set, X and Y from the tables at $9470, $9478,
; $9480 and $9488.
; HOW: Table reads by X; $405D; stores by the slot in Y.
; WAITS: no.
; Entry: X the piece. Exit: A the Y written, N and Z from it; X and Y as $405D left them; C and V as it left them.
StageDrawPiece152D:
    lda a:$9468,x
    tay
    jsr VecClearObjectInY
    lda a:$9470,x
    sta a:$2A58,y
    lda a:$9478,x
    sta a:OBJ_SPRITE_SET,y
    lda a:$9480,x
    sta a:OBJ_X_HI+8,y
    lda a:$9488,x
    sta a:$2C50,y
    rts

; @routine 152F:490 StageObjectHits152D
; @run 6B:9490
; @unreachable 6B:9490-6B:949A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9490 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The objects' hits each frame: the hit boxes of slots 0 to 15 placed (StagePlaceHitBoxes152D), then
; for objects $47 down to $10 the test of each object that has an event against those boxes and the step of its hit
; timer; runs on into the loop's timer step at $949B for the first object without an event.
; HOW: X kept on the stack; StagePlaceHitBoxes152D; the loop (hit_loop), shared with StageHitTimer152D,
; StageHitNext152D and StageHitEvent152D, which are its entries the loop's branches and jumps reach.
; WAITS: no.
; Entry: nothing. Exit: X kept, N and Z from it; C clear; A, Y and V as the last object's step left them.
StageObjectHits152D:
    phx
    jsr StagePlaceHitBoxes152D
    ldx #$47
StageObjectHits152D_at0:
    lda a:$30D0,x
    bne StageHitEvent152D

; @routine 152F:49B StageHitTimer152D
; @run 6B:949B
; @unreachable 6B:949B-6B:94A5 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $949B runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The hit loop's step of object X's hit timer at $3790 (while it runs, the object's state at $31F0 is
; cleared and the timer counted down), then the loop goes on (StageHitNext152D).
; HOW: A test, a store and a decrement in the hit loop.
; WAITS: no.
; Entry: X the object, the loop's X on the stack. Exit: as StageObjectHits152D.
StageHitTimer152D:
    lda a:$3790,x
    beq StageHitNext152D
    stz a:$31F0,x
    dec a:$3790,x

; @routine 152F:4A6 StageHitNext152D
; @run 6B:94A6
; @unreachable 6B:94A6-6B:94AC no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $94A6 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The hit loop's next object: X down by one; from $10 down the loop goes on, below it X comes back from
; the stack.
; HOW: DEX, CPX and the loop's branch; PLX.
; WAITS: no.
; Entry: X the object, the loop's X on the stack. Exit: as StageObjectHits152D.
StageHitNext152D:
    dex
    cpx #$10
    bcs StageObjectHits152D_at0
    plx
    rts

; @routine 152F:4AD StagePlaceHitBoxes152D
; @run 6B:94AD
; @unreachable 6B:94AD-6B:94B7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $94AD runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Places the hit boxes of slots 0 to 15 (StagePlaceHitBox152D for each).
; HOW: A loop up X.
; WAITS: no.
; Entry: nothing. Exit: X $10, N and Z from the compare (Z set), C set; A, Y and V as the last box left them.
StagePlaceHitBoxes152D:
    ldx #$00
StagePlaceHitBoxes152D_at0:
    jsr StagePlaceHitBox152D
    inx
    cpx #$10
    bcc StagePlaceHitBoxes152D_at0
    rts

; @routine 152F:4B8 StagePlaceHitBox152D
; @run 6B:94B8
; @unreachable 6B:94B8-6B:94FC no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $94B8 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Places slot X's hit box: with no event the box is off the page ($2733 1); else its X and Y (the
; slot's position moved by the signed offsets at $3670 and $3700) at $2713 and $2723, and the two high bytes ORed
; into $2733 (0 when the box is on the screen's page).
; HOW: 16-bit adds with the sign in zTemp00.
; WAITS: no.
; Entry: X the slot. Exit: A the page byte, N and Z from the ORA; C and V from the last add. X and Y kept.
StagePlaceHitBox152D:
    lda a:$30D0,x
    bne StagePlaceHitBox152D_at0
    lda #$01
    sta a:HitboxHi+3,x
    rts
StagePlaceHitBox152D_at0:
    stz z:zTemp00
    lda a:OBJ_3670,x
    bpl StagePlaceHitBox152D_at1
    dec z:zTemp00
StagePlaceHitBox152D_at1:
    lda a:OBJ_X_HI+8,x
    clc
    adc a:OBJ_3670,x
    sta a:HitboxXLo+3,x
    lda a:$2B30,x
    adc z:zTemp00
    sta a:HitboxHi+3,x
    stz z:zTemp00
    lda a:$3700,x
    bpl StagePlaceHitBox152D_at2
    dec z:zTemp00
StagePlaceHitBox152D_at2:
    lda a:$2C50,x
    clc
    adc a:$3700,x
    sta a:HitboxYLo+3,x
    lda a:$2C08,x
    adc z:zTemp00
    ora a:HitboxHi+3,x
    sta a:HitboxHi+3,x
    rts

; @routine 152F:4FD StageHitEvent152D
; @run 6B:94FD
; @unreachable 6B:94FD-6B:958A no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $94FD runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The hit loop's test of object X that has an event: its state at $31F0 cleared; its point (its X and Y
; moved by the signed offsets at $3670 and $3700) must be on the screen's page; then each hit box of slots 15 down to
; 0 that is on the page and within the two objects' reaches ($36B8 and $3748 added) is a hit: StageHitHandler152D runs
; and the object's hit timer steps. Then the loop goes on.
; HOW: 16-bit adds with the sign in zTemp00; the point in $2702-$2703, the reaches in $2704-$2705; distances as
; absolute differences; the loop.
; WAITS: no.
; Entry: X the object, the loop's X on the stack. Exit: as StageObjectHits152D.
StageHitEvent152D:
    stz a:$31F0,x
    stz z:zTemp00
    lda a:OBJ_3670,x
    bpl StageHitEvent152D_at0
    dec z:zTemp00
StageHitEvent152D_at0:
    clc
    adc a:OBJ_X_HI+8,x
    sta a:StreamStepHi
    lda a:$2B30,x
    adc z:zTemp00
    beq StageHitEvent152D_at1
    jmp StageHitTimer152D
StageHitEvent152D_at1:
    stz z:zTemp00
    lda a:$3700,x
    bpl StageHitEvent152D_at2
    dec z:zTemp00
StageHitEvent152D_at2:
    clc
    adc a:$2C50,x
    sta a:StreamStepHi+1
    lda a:$2C08,x
    adc z:zTemp00
    beq StageHitEvent152D_at3
    jmp StageHitTimer152D
StageHitEvent152D_at3:
    ldy #$0F
StageHitEvent152D_at8:
    lda a:HitboxHi+3,y
    bne StageHitEvent152D_at4
    lda a:$36B8,x
    clc
    adc a:$36B8,y
    sta a:StreamStepHi+2
    lda a:$3748,x
    clc
    adc a:$3748,y
    sta a:StreamStepHi+3
    lda a:StreamStepHi
    sec
    sbc a:HitboxXLo+3,y
    bcs StageHitEvent152D_at5
    eor #$FF
    inc a
StageHitEvent152D_at5:
    cmp a:StreamStepHi+2
    bcs StageHitEvent152D_at4
    lda a:StreamStepHi+1
    sec
    sbc a:HitboxYLo+3,y
    bcs StageHitEvent152D_at6
    eor #$FF
    inc a
StageHitEvent152D_at6:
    cmp a:StreamStepHi+3
    bcs StageHitEvent152D_at4
    jsr StageHitHandler152D
    lda a:$3790,x
    bne StageHitEvent152D_at7
    jmp StageHitNext152D
StageHitEvent152D_at7:
    dec a:$3790,x
    stz a:$31F0,x
    jmp StageHitNext152D
StageHitEvent152D_at4:
    dey
    bpl StageHitEvent152D_at8
    jmp StageHitTimer152D

; @routine 152F:58B StageHitHandler152D
; @run 6B:958B
; @unreachable 6B:958B-6B:95A8 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $958B runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Object X hit slot Y's box: the object kept as the slot's hitter at $35E0, zHitSlot the slot; when the
; slot's hit kind at $3598 is not 0 its handler (the table of 15 at $95A9, kind 1 first) runs with Y the slot.
; HOW: Stores; a table read into zTemp00-01 and a jump through it.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: kind 0: A 0, N and Z from it. Else: as the handler leaves them; it is entered
; with A the handler's high byte and Y the slot.
StageHitHandler152D:
    txa
    sta a:$35E0,y
    sty z:zHitSlot
    lda a:$3598,y
    beq StageHitHandler152D_at0
    dec a
    asl a
    tay
    lda a:$95A9,y
    sta z:zTemp00
    lda a:$95AA,y
    sta z:zTemp01
    ldy z:zHitSlot
    jmp (ZeroPage)
StageHitHandler152D_at0:
    rts

; @routine 152F:5C7 HitKindSpawn152D
; @run 6B:95C7
; @unreachable 6B:95C7-6B:95E3 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $95C7 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Hit kinds 1 to 4: the object's state at $31F0 takes the slot's kind; when the first object's state at
; $31F0 is 0 and the gate at $2748 is 0, the first object takes this object's bytes at $31A8 and $3160 (its state at
; $31F0 and its byte at $3118).
; HOW: Loads and stores; two tests.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: A the last byte loaded, N and Z from it. X, Y, C and V kept.
HitKindSpawn152D:
    lda a:$3598,y
    sta a:$31F0,x
    lda a:$31F0
    bne HitKindSpawn152D_at0
    lda a:LevelFullWalk+4
    bne HitKindSpawn152D_at0
    lda a:OBJ_HIT_STATUS+8,x
    sta a:$31F0
    lda a:$3160,x
    sta a:$3118
HitKindSpawn152D_at0:
    rts

; @routine 152F:5E4 HitKindFacing152D
; @run 6B:95E4
; @unreachable 6B:95E4-6B:95F0 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $95E4 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Hit kind 6: the slot's side at $3628 from the object's facing (bit 0 of $2FF8, plus one); unless it is
; 2, runs on into HitKindState152D.
; HOW: A mask and an increment; a compare.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: side 2: A 2, Z set, C set. Else as HitKindState152D.
HitKindFacing152D:
    lda a:$2FF8,x
    and #$01
    inc a
    sta a:$3628,y
HitKindFacing152D_at0:
    cmp #$02
    beq HitKindState152D_at0

; @routine 152F:5F1 HitKindState152D
; @run 6B:95F1
; @unreachable 6B:95F1-6B:95F7 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $95F1 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Hit kinds 5, 7 and 10 to 15: the object's state at $31F0 takes the slot's kind.
; HOW: A load and a store.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: A the kind, N and Z from it. X, Y, C and V kept.
HitKindState152D:
    lda a:$3598,y
    sta a:$31F0,x
HitKindState152D_at0:
    rts

; @routine 152F:5F8 HitKindSide152D
; @run 6B:95F8
; @unreachable 6B:95F8-6B:9603 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $95F8 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Hit kind 9: the slot's side at $3628 from bit 1 of the object's $2FF8 (plus one); unless it is 2, the
; object takes the slot's kind. The compare and what follows are HitKindFacing152D's bytes at $95ED.
; HOW: A mask, a shift and an increment; a branch into HitKindFacing152D's bytes.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: as HitKindFacing152D.
HitKindSide152D:
    lda a:$2FF8,x
    and #$02
    lsr a
    inc a
    sta a:$3628,y
    bra HitKindFacing152D_at0

; @routine 152F:604 StageProbePoints152D
; @run 6B:9604
; @unreachable 6B:9604-6B:9651 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9604 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Probes the map at an object's four points (the list the index in A names, from the table at $DB54:
; four offsets of X and Y a point from the object's position): for each, ProbeTile152D's answer at $26F6 and the
; tile's column and row at $26FA and $26FE.
; HOW: The list in zTemp0C-0D, the count in zTemp0A; the point into zTemp00-03; Y kept on the stack across
; ProbeTile152D.
; WAITS: no.
; Entry: A the list, X the object. Exit: A 4, Z set, C set; Y past the last point; X kept as ProbeTile152D keeps it.
StageProbePoints152D:
    asl a
    tay
    lda a:$DB54,y
    sta z:zTemp0C
    lda a:$DB55,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
StageProbePoints152D_at0:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_HI+8,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:$2B30,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:$2C50,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:$2C08,x
    sta z:zTemp03
    iny
    phy
    jsr ProbeTile152D
    ldy z:zTemp0A
    sta a:TileHit+3,y
    lda z:zTemp00
    sta a:TileDepthX+3,y
    lda z:zTemp02
    sta a:TileDepthY+3,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne StageProbePoints152D_at0
    rts

; @routine 152F:652 StageProbeAgain152D
; @run 6B:9652
; @unreachable 6B:9652-6B:96C5 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9652 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Probes an object's four points (StageProbePoints152D), then each point whose answer has bit 0 set and
; bit 7 clear again 8 pixels higher; when the second probe's answer has bit 0 set the first answer keeps only bits 1
; to 6.
; HOW: A kept on the stack across StageProbePoints152D; the list again; for each point a test of its answer, the point
; into zTemp00-03 and $FFF8 added to its Y, ProbeTile152D with Y kept on the stack.
; WAITS: no.
; Entry: A the list, X the object. Exit: A 4, Z set, C set; Y past the last point.
StageProbeAgain152D:
    pha
    jsr StageProbePoints152D
    pla
    asl a
    tay
    lda a:$DB54,y
    sta z:zTemp0C
    lda a:$DB55,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
StageProbeAgain152D_at3:
    phy
    ldy z:zTemp0A
    lda a:TileHit+3,y
    ply
    and #$81
    bmi StageProbeAgain152D_at0
    beq StageProbeAgain152D_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_HI+8,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:$2B30,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:$2C50,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:$2C08,x
    sta z:zTemp03
    iny
    lda #$F8
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$FF
    adc z:zTemp02
    sta z:zTemp02
    phy
    jsr ProbeTile152D
    ldy z:zTemp0A
    and #$01
    beq StageProbeAgain152D_at1
    ldy z:zTemp0A
    lda a:TileHit+3,y
    and #$7E
    sta a:TileHit+3,y
StageProbeAgain152D_at1:
    ply
StageProbeAgain152D_at2:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne StageProbeAgain152D_at3
    rts
StageProbeAgain152D_at0:
    iny
    iny
    iny
    iny
    bra StageProbeAgain152D_at2

; @routine 152F:6C6 ProbeTile152D
; @run 6B:96C6
; @unreachable 6B:96C6-6B:97D2 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $96C6 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Says what the map holds at a point: the background's entry there (its place from the point plus the
; scroll, read from VRAM), the entry's page in SaveBuffer (pages of 256 bytes from $7D00) and its attribute there.
; A page of bit 7 or an attribute below $80 gives 0 or the attribute; $80 to $85 are slopes and steps that set the
; row in the tile (zTemp02) from the tables at $97DA up and answer $81; $86 to $89 move the row by the table at $97D3
; and $8A by one, answering 1.
; HOW: 9-bit adds of the point and the scroll into zTemp04-07; the kernel's pieces at $4024 and $4027 around the VRAM
; address; the entry in zTemp08-09; compares of the attribute.
; WAITS: no.
; Entry: zTemp00-01 X, zTemp02-03 Y. Exit: A the answer, N and Z from it; zTemp00 the column in the tile, zTemp02 the
; row; Y and C as the last load or compare left them. X kept as the kernel keeps it. Clobbers zTemp04-09.
ProbeTile152D:
    lda z:zTemp00
    clc
    adc a:ScrollXLo
    sta z:zTemp04
    lda z:zTemp01
    adc a:ScrollXHi
    and #$01
    lsr a
    lda z:zTemp04
    ror a
    lsr a
    lsr a
    and #$3F
    sta z:zTemp05
    lda z:zTemp04
    and #$07
    sta z:zTemp00
    lda z:zTemp02
    clc
    adc a:ScrollYLo
    sta z:zTemp06
    lda z:zTemp03
    adc a:ScrollYHi
    and #$01
    sta z:zTemp07
    lda z:zTemp06
    asl a
    rol z:zTemp07
    asl a
    rol z:zTemp07
    asl a
    rol z:zTemp07
    and #$C0
    clc
    adc z:zTemp05
    sta z:zTemp05
    lda z:zTemp06
    and #$07
    sta z:zTemp02
    jsr VecSelectVramReadAddress
    lda z:zTemp05
    sta a:VDC_DATA_LO
    lda z:zTemp07
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:VDC_DATA_LO
    sta z:zTemp08
    lda a:VDC_DATA_HI
    and #$07
    sta z:zTemp09
    tay
    lda a:SaveBuffer,y
    bmi ProbeTile152D_at0
    lda z:zTemp08
    clc
    adc #$00
    sta z:zTemp08
    lda a:SaveBuffer,y
    adc #$7D
    sta z:zTemp09
    lda (zTemp08)
    bmi ProbeTile152D_at1
    rts
ProbeTile152D_at0:
    lda #$00
    rts
ProbeTile152D_at1:
    cmp #$80
    beq ProbeTile152D_at2
    cmp #$81
    beq ProbeTile152D_at3
    cmp #$82
    beq ProbeTile152D_at4
    cmp #$83
    beq ProbeTile152D_at5
    cmp #$84
    beq ProbeTile152D_at6
    cmp #$85
    beq ProbeTile152D_at7
    cmp #$8A
    bcc ProbeTile152D_at8
    cmp #$8A
    beq ProbeTile152D_at9
    rts
ProbeTile152D_at2:
    ldy z:zTemp00
    lda a:$97DA,y
    sta z:zTemp02
    lda #$81
    rts
ProbeTile152D_at3:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileHalfStepsA+5,y
    sta z:zTemp02
    lda #$81
    rts
ProbeTile152D_at4:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:$9822,y
    sta z:zTemp02
    lda #$81
    rts
ProbeTile152D_at8:
    sec
    sbc #$83
    tay
    lda a:$97D3,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$01
    rts
ProbeTile152D_at9:
    inc z:zTemp02
    lda #$01
    rts
ProbeTile152D_at5:
    ldy z:zTemp00
    lda a:$9862,y
    sta z:zTemp02
    lda #$81
    rts
ProbeTile152D_at6:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:$986A,y
    sta z:zTemp02
    lda #$81
    rts
ProbeTile152D_at7:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:$98AA,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 1530:0EA GateObjectStep152D
; @run 6B:98EA
; @stop 98FC
; @unreachable 6B:98EA-6B:98FB no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $98EA runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: An object handler for the stage's gate: its step at $2818 runs from the table of seven at $98FC (the
; start, the wait for the player, the wait for the cleared stage's step, the opening, the wait, the closing, nothing).
; No code of the overlay reaches it (another overlay's tables do); the census counts none of its bytes.
; HOW: The step doubled indexes the table; its word goes through zTemp00-01 and is jumped through.
; WAITS: no.
; Entry: X the object. Exit: as the step leaves them; it is entered with A the word's high byte and Y the index.
GateObjectStep152D:
    lda a:$2818,x
    asl a
    tay
    lda a:$98FC,y
    sta z:zTemp00
    lda a:$98FD,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 1530:10A GateStart152D
; @run 6B:990A
; @stop 9926
; @unreachable 6B:990A-6B:9925 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $990A runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The gate's first step: the overlay at $A000's piece at $A36C, the gate's Y moved up by ScrollYLo, its
; sprite at $2A58 from the two at $9927 by bit 0 of its packed byte at $2F20; then the RTS at $9926 (the bytes of
; StageSubStepNone152D's kind, run on into).
; HOW: $A36C; a negate and an add; a table read.
; WAITS: no.
; Entry: X the object. Exit: A the sprite, N and Z from it, Y the bit; C and V from the add.
GateStart152D:
    jsr $A36C
    lda a:ScrollYLo
    eor #$FF
    inc a
    clc
    adc a:$2C50,x
    sta a:$2C50,x
    lda a:$2F20,x
    and #$01
    tay
    lda a:$9927,y
    sta a:$2A58,x

; @routine 1530:12B GateWaitPlayer152D
; @run 6B:992B
; @unreachable 6B:992B-6B:9975 no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $992B runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The gate waits for the player: with PlayerByte3BD7 set and the byte at $2746 clear, and the gate's
; state at $31F0 1 or 2, the stage is cleared through it (zField8C its packed byte's low nibble plus one, zSubStep 3),
; the gate goes on to its next step (its count 0, its timer 4); a packed byte's high nibble names a sound (raster
; effect and $3EA4 cleared, the kernel's piece at $400F with X 6 and A the nibble with bit 7).
; HOW: Tests; stores; shifts; X kept on the stack across the kernel.
; WAITS: no.
; Entry: X the object. Exit: as the last test or the kernel left them; X kept.
GateWaitPlayer152D:
    lda a:LevelFullWalk+2
    eor #$01
    and a:PlayerByte3BD7
    beq GateWaitPlayer152D_at0
    lda a:$31F0,x
    beq GateWaitPlayer152D_at0
    cmp #$03
    bcs GateWaitPlayer152D_at0
    lda a:$2F20,x
    and #$0F
    inc a
    sta z:zField8C
    lda #$03
    sta z:zSubStep
    inc a:$2818,x
    stz a:$3310,x
    lda #$04
    sta a:$32C8,x
    lda a:$2F20,x
    and #$F0
    beq GateWaitPlayer152D_at0
    stz z:zRasterEffect
    stz a:ScrollSpeedX+6
    lda a:$2F20,x
    and #$F0
    beq GateWaitPlayer152D_at0
    lsr a
    lsr a
    lsr a
    lsr a
    ora #$80
    phx
    ldx #$06
    jsr VecStartListChannelKind2
    plx
GateWaitPlayer152D_at0:
    rts

; @routine 1530:176 GateWaitStep152D
; @run 6B:9976
; @unreachable 6B:9976-6B:997F no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9976 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The gate waits for the cleared stage's step at $85 to reach 2, then goes on.
; HOW: A compare; an increment.
; WAITS: no.
; Entry: X the object. Exit: A $85, N, Z and C from the compare, or N and Z from the step's increment.
GateWaitStep152D:
    lda z:zField85
    cmp #$02
    bne GateWaitClear152D_at0
    inc a:$2818,x
    rts

; @routine 1530:180 GateOpen152D
; @run 6B:9980
; @unreachable 6B:9980-6B:99BD no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $9980 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The gate opens: every fourth frame its count at $3310 steps up and its frame is drawn; at 4 it goes
; on, zField8C gets bit 7 and sound $3A is sent.
; HOW: A countdown at $32C8; a compare; gate_frame; the sound hook by a jump.
; WAITS: no.
; Entry: X the object. Exit: as the last instruction, the sound hook or the kernel left them; X kept.
GateOpen152D:
    dec a:$32C8,x
    bne GateOpen152D_at0
    inc a:$3310,x
    lda a:$3310,x
    cmp #$04
    bne GateOpen152D_at1
    inc a:$2818,x
    lda z:zField8C
    ora #$80
    sta z:zField8C
    lda #$3A
    jmp VecSoundCommand
GateOpen152D_at1:
    lda #$04
    sta a:$32C8,x
    lda a:$2F20,x
    and #$01
    tay
    lda a:$9927,y
    clc
    adc a:$3310,x
    sta a:$2A58,x
    clc
    adc a:$9929,y
    phx
    ldx #$05
    jsr VecStartListChannelKind2
    plx
GateOpen152D_at0:
    rts

; @routine 1530:1BE GateWaitClear152D
; @run 6B:99BE
; @unreachable 6B:99BE-6B:99CA no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $99BE runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The gate waits until zField8C loses bit 7, then goes on with its timer 4.
; HOW: A test; stores.
; WAITS: no.
; Entry: X the object. Exit: A zField8C, N and Z from it, or A 4.
GateWaitClear152D:
    lda z:zField8C
    bmi GateWaitClear152D_at0
    inc a:$2818,x
    lda #$04
    sta a:$32C8,x
GateWaitClear152D_at0:
    rts

; @routine 1530:1CB GateClose152D
; @run 6B:99CB
; @unreachable 6B:99CB-6B:99FD no read the game makes loads disc sectors 152D to 1530, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sectors, and a unit call at $99CB runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The gate closes: every fourth frame its count at $3310 steps down and its frame is drawn; below 0 it
; goes on and sound $3A is sent.
; HOW: A countdown at $32C8; a decrement; gate_frame; the sound hook by a jump.
; WAITS: no.
; Entry: X the object. Exit: as the last instruction, the sound hook or the kernel left them; X kept.
GateClose152D:
    dec a:$32C8,x
    bne GateClose152D_at0
    dec a:$3310,x
    bpl GateClose152D_at1
    inc a:$2818,x
    lda #$3A
    jmp VecSoundCommand
GateClose152D_at1:
    lda #$04
    sta a:$32C8,x
    lda a:$2F20,x
    and #$01
    tay
    lda a:$9927,y
    clc
    adc a:$3310,x
    sta a:$2A58,x
    clc
    adc a:$9929,y
    phx
    ldx #$05
    jsr VecStartListChannelKind2
    plx
GateClose152D_at0:
    rts
