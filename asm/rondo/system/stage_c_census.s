; @routine 03CD:000 StageEntryA3CD
; @run 6B:8000
; @unreachable 6B:8000-6B:8002 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8000 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay's entry at $8000: goes to StageSubModeStep3CD, the stage's frame by sub-mode.
; HOW: One jump, no frame of its own.
; WAITS: as the sub-mode's handler.
; Entry, exit: as StageSubModeStep3CD.
AndVramDataAt9350 = $9350 ; AndVramData's bytes (024F:392) at this address
AndVramDataAt9425 = $9425 ; AndVramData's bytes (024F:392) at this address
AndVramDataAt942A = $942A ; AndVramData's bytes (024F:392) at this address
AndVramDataAt9631 = $9631 ; AndVramData's bytes (024F:392) at this address
MapOverlayBank7At802C = $802C ; MapOverlayBank7's bytes (024D:0B7) at this address
MapOverlayBank7At8044 = $8044 ; MapOverlayBank7's bytes (024D:0B7) at this address
MapOverlayBank7At80A0 = $80A0 ; MapOverlayBank7's bytes (024D:0B7) at this address
MapOverlayBanks5And6At8021 = $8021 ; MapOverlayBanks5And6's bytes (024D:0A0) at this address
MapOverlayBanks5And6At802D = $802D ; MapOverlayBanks5And6's bytes (024D:0A0) at this address
MapStageBanksAt9307 = $9307 ; MapStageBanks's bytes (024F:349) at this address
MapStageBanksAt93E7 = $93E7 ; MapStageBanks's bytes (024F:349) at this address
MapStageBanksAt95E8 = $95E8 ; MapStageBanks's bytes (024F:349) at this address
OrVramDataAt935F = $935F ; OrVramData's bytes (024F:3A1) at this address
OrVramDataAt9434 = $9434 ; OrVramData's bytes (024F:3A1) at this address
OrVramDataAt9439 = $9439 ; OrVramData's bytes (024F:3A1) at this address
OrVramDataAt9640 = $9640 ; OrVramData's bytes (024F:3A1) at this address
StageOpeningWriteRow152DAt8FA1 = $8FA1 ; StageOpeningWriteRow152D's bytes (152E:5B8) at this address
StageSplitTens152DAt9405 = $9405 ; StageSplitTens152D's bytes (152F:40A) at this address
StageEntryA3CD:
    jmp StageSubModeStep3CD

; @routine 03CD:003 StageEntryB3CD
; @run 6B:8003
; @unreachable 6B:8003-6B:8005 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8003 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay's entry at $8003: a jump to $0000, the hardware page, where no code is (the entry of this
; build that has no routine).
; HOW: One jump, no frame of its own.
; WAITS: as what runs there.
; Entry, exit: as the code at $0000.
StageEntryB3CD:
    jmp VDC_ADDR

; @routine 03CD:006 StageEntryC3CD
; @run 6B:8006
; @unreachable 6B:8006-6B:8008 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8006 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay's entry at $8006: goes to ProbeTile3CD, the tile probe at $974F.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as ProbeTile3CD.
StageEntryC3CD:
    jmp ProbeTile3CD

; @routine 03CD:009 StageEntryD3CD
; @run 6B:8009
; @unreachable 6B:8009-6B:800B no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8009 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay's entry at $8009: goes to ClearBitRowsFour3CD at $9097.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as ClearBitRowsFour3CD.
StageEntryD3CD:
    jmp ClearBitRowsFour3CD

; @routine 03CD:00C StageEntryE3CD
; @run 6B:800C
; @unreachable 6B:800C-6B:800E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $800C runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay's entry at $800C: goes to StageProbePoints3CD at $968D.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as StageProbePoints3CD.
StageEntryE3CD:
    jmp StageProbePoints3CD

; @routine 03CD:00F StageEntryF3CD
; @run 6B:800F
; @unreachable 6B:800F-6B:8011 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $800F runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay's entry at $800F: goes to StageProbeAgain3CD at $96DB.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as StageProbeAgain3CD.
StageEntryF3CD:
    jmp StageProbeAgain3CD

; @routine 03CD:018 StageSubModeStep3CD
; @run 6B:8018
; @stop 801F
; @unreachable 6B:8018-6B:801E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8018 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The stage's frame: runs the handler of the sub-mode zGameSubMode holds, from the table of seven at
; $801F (0 the start, 1 play, 2 the player's death, 3 the stage cleared, 4 the fade out, 5 the opening, 6 the tally).
; HOW: The sub-mode doubled indexes the table, whose word is jumped through.
; WAITS: as the handler.
; Entry: nothing. Exit: as the handler leaves them; it is entered with A and X twice the sub-mode, N and Z from it,
; C the sub-mode's bit 7.
StageSubModeStep3CD:
    lda z:zGameSubMode
    asl a
    tax
    jmp ($801F,x)

; @routine 03CD:038 StageFrameA0003CD
; @run 6B:8038
; @unreachable 6B:8038-6B:803D no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8038 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay at $A000's frame at $A000 under banks 5 and 6.
; HOW: MapOverlayBanks5And6; a jump to $A000.
; WAITS: as $A000.
; Entry: nothing. Exit: as $A000 leaves them.
StageFrameA0003CD:
    jsr MapOverlayBanks5And6At802D
    jmp RunObjects

; @routine 03CD:03E StageFrameA0093CD
; @run 6B:803E
; @unreachable 6B:803E-6B:8043 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $803E runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay at $A000's piece at $A009 under banks 5 and 6.
; HOW: MapOverlayBanks5And6; a jump to $A009.
; WAITS: as $A009.
; Entry: nothing. Exit: as $A009 leaves them.
StageFrameA0093CD:
    jsr MapOverlayBanks5And6At802D
    jmp RichterVecC3A7

; @routine 03CD:04C StageFrameA08D3CD
; @run 6B:804C
; @unreachable 6B:804C-6B:8051 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $804C runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay at $A000's piece at $A08D under bank 7.
; HOW: MapOverlayBank7; a jump to $A08D.
; WAITS: as $A08D.
; Entry: nothing. Exit: as $A08D leaves them.
StageFrameA08D3CD:
    jsr MapOverlayBank7At8044
    jmp $A08D

; @routine 03CD:052 StageFrameA0673CD
; @run 6B:8052
; @unreachable 6B:8052-6B:8057 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8052 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The overlay at $A000's piece at $A067 under bank 7.
; HOW: MapOverlayBank7; a jump to $A067.
; WAITS: as $A067.
; Entry: nothing. Exit: as $A067 leaves them.
StageFrameA0673CD:
    jsr MapOverlayBank7At8044
    jmp ClearScrollSpeeds

; @routine 03CD:058 DemoStart3CD
; @run 6B:8058
; @unreachable 6B:8058-6B:8090 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8058 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Starts the demo recorder by the pad held at the stage's start: bit 4 records (mode 1, the buffer's
; first byte at $A000 cleared under bank $40 above zRamBaseBank), bit 6 plays back (mode $FF); either way the random
; seed, the wave counter, zModeEvents, zFrameTick and the recorder's bytes are cleared and its pointer set to $A000.
; HOW: Pad bits at $2229; the mode in zWaitTimer; stores.
; WAITS: no.
; Entry: nothing. Exit: A the pad's bit or $A0, N and Z from it; C and V from the add when recording. X and Y kept.
DemoStart3CD:
    stz z:zWaitTimer
    lda a:BiosPadState+1
    and #$10
    beq DemoStart3CD_at1
    inc z:zWaitTimer
    lda #$40
    clc
    adc z:zRamBaseBank
    tam #$20
    stz a:$A000
    bra DemoStart3CD_at0
DemoStart3CD_at1:
    lda a:BiosPadState+1
    and #$40
    beq DemoStart3CD_at2
    dec z:zWaitTimer
DemoStart3CD_at0:
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zModeEvents
    stz z:zFrameTick
    stz z:zDemoPadLast
    stz z:zDemoPadFrames
    stz z:zScriptPtr
    lda #$A0
    sta z:zScriptPtrHi
DemoStart3CD_at2:
    rts

; @routine 03CD:091 DemoStep3CD
; @run 6B:8091
; @unreachable 6B:8091-6B:80D5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8091 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The demo recorder's frame, under banks $40 and $41: mode 0 does nothing; a negative mode plays back
; (DemoPlay3CD); a positive one records BiosPadState into the buffer (a full record of $FF moves on first).
; HOW: The mode in zWaitTimer; the add and two TAM; reads and writes through the pointer at $7E; DemoAdvance3CD by BSR.
; WAITS: no.
; Entry: nothing. Exit: as the step that ran left them; mode 0: A 0, N and Z from it.
DemoStep3CD:
    lda z:zWaitTimer
    beq DemoStart3CD_at2
    lda #$40
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda z:zWaitTimer
    bmi DemoPlay3CD
    lda (zScriptPtr)
    cmp #$FF
    bne DemoStep3CD_at2
    bsr DemoAdvance3CD
DemoStep3CD_at2:
    lda a:BiosJoyTrigger+1
    and #$04
    bne DemoStep3CD_at0
    lda (zScriptPtr)
    beq DemoStep3CD_at1
    ldy #$01
    lda a:BiosPadState
    cmp (zScriptPtr),y
    bne DemoStep3CD_at3
    lda (zScriptPtr)
    inc a
    sta (zScriptPtr)
    rts
DemoStep3CD_at3:
    bsr DemoAdvance3CD
DemoStep3CD_at1:
    lda #$01
    sta (zScriptPtr)
    tay
    lda a:BiosPadState
    sta (zScriptPtr),y
    rts
DemoStep3CD_at0:
    bsr DemoAdvance3CD
    bra DemoAdvance3CD_at1

; @routine 03CD:0D6 DemoAdvance3CD
; @run 6B:80D6
; @unreachable 6B:80D6-6B:80EE no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $80D6 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Moves the demo recorder's pointer to the next record (two bytes); at $E000 the recording ends (mode
; 0); else the new record is cleared.
; HOW: A 16-bit increment of the pointer at $7E; a compare of its page.
; WAITS: no.
; Entry: nothing. Exit, at the end: N and Z from the page's compare, C set. Else: A 0, Y 1, N and Z from Y.
DemoAdvance3CD:
    inc z:zScriptPtr
    inc z:zScriptPtr
    bne DemoAdvance3CD_at0
    inc z:zScriptPtrHi
    lda z:zScriptPtrHi
    cmp #$E0
    bne DemoAdvance3CD_at0
DemoAdvance3CD_at1:
    stz z:zWaitTimer
    rts
DemoAdvance3CD_at0:
    cla
    sta (zScriptPtr)
    ldy #$01
    sta (zScriptPtr),y
    rts

; @routine 03CD:0EF DemoPlay3CD
; @run 6B:80EF
; @unreachable 6B:80EF-6B:8124 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $80EF runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The demo's playback: each record's pad is put in BiosPadState for its count of frames, its new
; presses into BiosJoyTrigger; a record with a count of 0 ends the playback, and so does the buffer's end at $E000.
; HOW: The count at $7D, the last pad at $7C; reads through the pointer at $7E; a 16-bit increment.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction that ran left them.
DemoPlay3CD:
    lda z:zDemoPadFrames
    bne DemoPlay3CD_at0
    lda (zScriptPtr)
    beq DemoAdvance3CD_at1
    sta z:zDemoPadFrames
DemoPlay3CD_at0:
    ldy #$01
    lda (zScriptPtr),y
    sta a:BiosPadState
    lda z:zDemoPadLast
    eor #$FF
    and a:BiosPadState
    sta a:BiosJoyTrigger
    lda a:BiosPadState
    sta z:zDemoPadLast
    dec z:zDemoPadFrames
    beq DemoPlay3CD_at1
    rts
DemoPlay3CD_at1:
    inc z:zScriptPtr
    inc z:zScriptPtr
    bne DemoPlay3CD_at2
    inc z:zScriptPtrHi
    lda z:zScriptPtrHi
    cmp #$E0
    bne DemoPlay3CD_at2
    stz z:zWaitTimer
DemoPlay3CD_at2:
    rts

; @routine 03CD:125 StageSubModeStart3CD
; @run 6B:8125
; @unreachable 6B:8125-6B:8167 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8125 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-mode 0, the stage's start: as StageSubModeStart152D (the kernel's piece at $4006, the route's
; pieces, the row map, the screen, the overlay at $A000's start, the display, the opening, the level), the pause and
; the fade cleared, sub-mode 1, zField86 zField83; the demo recorder started (DemoStart3CD). zField8E's bit 0 clear
; sends the stage to sub-mode 5 with the count at $3310 one less and PlayerStartPose 1; else $407B runs unless zStage is
; negative.
; HOW: Calls in turn $4006, StageRouteSetUpA3CD, StageRouteSetUpB3CD, StageLoadRowMap3CD, StageResetScroll3CD,
; MapOverlayBanks5And6, $A003, StageHudStart3CD, StageOpeningStart3CD, StageLevelStart3CD and DemoStart3CD.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit, sub-mode 5: A 1, N and Z from it. Else: A zStage, N and Z from it, or as $407B leaves them.
StageSubModeStart3CD:
    jsr VecResetGameState
    jsr StageRouteSetUpA3CD
    jsr StageRouteSetUpB3CD
    jsr StageLoadRowMap3CD
    jsr StageResetScroll3CD
    jsr MapOverlayBanks5And6At802D
    jsr RichterVecStart
    jsr StageHudStart3CD
    jsr StageOpeningStart3CD
    jsr StageLevelStart3CD
    stz z:zPaused
    stz z:zFadeActive
    inc z:zGameSubMode
    lda z:zField83
    sta z:zField86
    jsr DemoStart3CD
    bbr0 zField8E,StageSubModeStart3CD_at0
    lda z:zStage
    bmi StageSubModeStart3CD_at1
    jsr VecCdPlayAgain
StageSubModeStart3CD_at1:
    rts
StageSubModeStart3CD_at0:
    lda #$05
    sta z:zGameSubMode
    dec a:OBJ_WORK_B+16
    lda #$01
    sta a:PlayerStartPose
    rts

; @routine 03CD:168 StageFadeTimer3CD
; @run 6B:8168
; @unreachable 6B:8168-6B:8191 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8168 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The fade's countdown at $9B, as StageFadeTimer152D does it (the same code at this address).
; HOW: Two decrements of $9B; bit tests of zField8E; the sound hook or cd_fade by a tail jump.
; WAITS: no.
; Entry: nothing. Exit: as StageFadeTimer152D.
StageFadeTimer3CD:
    lda z:zFadeCount
    beq StagePauseToggle3CD_at0
    dec z:zFadeCount
    beq StageFadeTimer3CD_at0
    dec z:zFadeCount
    stz z:zFadeActive
    bbs4 zField8E,StagePauseToggle3CD_at0
    bbs3 zField8E,StageFadeTimer3CD_at1
    cla
    jmp BiosCdFade
StageFadeTimer3CD_at1:
    lda z:zFadeSound
    jmp VecSoundCommand
StageFadeTimer3CD_at0:
    lda #$01
    sta z:zFadeActive
    lda z:zField8E
    and #$18
    bne StagePauseToggle3CD_at0
    lda #$0C
    jmp BiosCdFade

; @routine 03CD:192 StagePauseToggle3CD
; @run 6B:8192
; @unreachable 6B:8192-6B:81AC no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8192 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Pauses or unpauses on Run, as StagePauseToggle152D, and not while zDisplayDelay is set.
; HOW: $3C XOR 1; BiosJoyTrigger's bit 3; zDisplayDelay; zPaused's bit 1; zPaused XOR 3.
; WAITS: no.
; Entry: nothing. Exit: A the last value loaded or computed, N and Z from it. X, Y, C and V kept.
StagePauseToggle3CD:
    lda z:zFadeActive
    eor #$01
    beq StagePauseToggle3CD_at0
    lda a:BiosJoyTrigger
    and #$08
    beq StagePauseToggle3CD_at0
    lda z:zDisplayDelay
    bne StagePauseToggle3CD_at0
    bbs1 zPaused,StagePauseToggle3CD_at0
    lda z:zPaused
    eor #$03
    sta z:zPaused
StagePauseToggle3CD_at0:
    rts

; @routine 03CD:1D4 StageFrameSkip3CD
; @run 6B:81D4
; @unreachable 6B:81D4-6B:81DD no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $81D4 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Lets its caller run one frame in eight: on the others it drops its caller's return address, so its
; RTS ends the caller too. No code of the overlay calls it.
; HOW: zFrameTick's low three bits; two PLA.
; WAITS: no.
; Entry: nothing. Exit, the eighth frame: A 0, Z set. Else: the frame dropped, A the return address's high byte less
; one, N and Z from it.
StageFrameSkip3CD:
    lda z:zFrameTick
    and #$07
    beq StageFrameSkip3CD_at0
    pla
    pla
    rts
StageFrameSkip3CD_at0:
    rts

; @routine 03CD:1DE StageSubModePlay3CD
; @run 6B:81DE
; @stop 8250
; @unreachable 6B:81DE-6B:824F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $81DE runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-mode 1, the stage in play: the demo recorder, the fade and the pause; paused, the paused frame.
; Else the events step; the stage's timer PlayerSlowTimerLo-3BE9 counted down (at its end the kernel's piece at $4015
; with A 9 and X 7), and while it runs the scroll's steps cleared and, on odd counts, the overlay's frame skipped;
; unless a sub-step is asked for the kernel's piece at $40D8; then for a stage with zField8E's bit 7 the scroll by
; StageCenterPlayer3CD, else the overlay's piece at $A08D (and $A009 on even counts); the overlays' frame, the
; objects' hits and the display; then the sub-step zSubStep held (cleared) from the table at $8250.
; HOW: Calls; TST of the timer's bit 0; the sub-step doubled indexes the table, whose word is jumped through.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit: as the sub-step's handler leaves them; paused: as StagePausedFrame3CD.
StageSubModePlay3CD:
    jsr $81D3
    jsr DemoStep3CD
    jsr StageFadeTimer3CD
    jsr StagePauseToggle3CD
    lda z:zPaused
    beq StageSubModePlay3CD_at1
    jmp StagePausedFrame3CD
StageSubModePlay3CD_at1:
    jsr StageModeEventsStep3CD
    lda a:PlayerSlowTimerLo
    ora a:PlayerSlowTimerHi
    beq StageSubModePlay3CD_at2
    dec a:PlayerSlowTimerLo
    bne StageSubModePlay3CD_at3
    dec a:PlayerSlowTimerHi
    bne StageSubModePlay3CD_at3
    lda #$09
    ldx #$07
    jsr VecStartScriptSlotKind1
StageSubModePlay3CD_at3:
    stz a:ScrollStepXHi
    stz a:ScrollStepXLo
    stz a:ScrollStepYHi
    stz a:ScrollStepYLo
    tst #$01,a:PlayerSlowTimerLo
    bne StageSubModePlay3CD_at4
StageSubModePlay3CD_at2:
    jsr StageFrameA0003CD
    lda z:zSubStep
    bne StageSubModePlay3CD_at5
    jsr VecPlayerAndCameraFrame
StageSubModePlay3CD_at4:
    lda z:zField8E
StageSubModePlay3CD_at0:
    bpl StageSubModePlay3CD_at6
    jsr StageCenterPlayer3CD
    bra StageSubModePlay3CD_at7
StageSubModePlay3CD_at6:
    jsr StageFrameA08D3CD
    tst #$01,a:PlayerSlowTimerLo
    bne StageSubModePlay3CD_at7
    jsr StageFrameA0093CD
StageSubModePlay3CD_at7:
    jsr StageOverlayFrame3CD
    jsr StageObjectHits3CD
    jsr StageHudStep3CD
StageSubModePlay3CD_at5:
    lda z:zSubStep
    stz z:zSubStep
    asl a
    tax
    jmp ($8250,x)

; @routine 03CD:260 StageSubStepDeath3CD
; @run 6B:8260
; @unreachable 6B:8260-6B:8282 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8260 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-step 2, the player died: sub-mode 2 with zField80, zField89 and the fall's step cleared, the
; overlay at $A000's piece at $A067 (StageFrameA0673CD), the forced presses cleared; when TimeSlowMode is set it is
; cleared and the fade starts again from 2.
; HOW: Stores; StageFrameA0673CD; a jump into StageFadeTimer3CD.
; WAITS: no.
; Entry: nothing. Exit: A TimeSlowMode (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer3CD.
StageSubStepDeath3CD:
    lda #$02
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jsr StageFrameA0673CD
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz z:zFallStep
    lda a:TimeSlowMode
    bne StageSubStepDeath3CD_at0
    rts
StageSubStepDeath3CD_at0:
    stz a:TimeSlowMode
    lda #$02
    sta z:zFadeCount
    jmp StageFadeTimer3CD

; @routine 03CD:283 StageSubStepCleared3CD
; @run 6B:8283
; @unreachable 6B:8283-6B:82BC no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8283 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-step 3, the stage cleared: sub-mode 3 with its step 0 and its table at $84 picked by zField8C
; (none: 2; negative: 4; 3 to 6 or $0F up: 3; else 1, with $2743 set to 1); the overlay at $A000's piece at $A067, the
; forced presses and TimeSlowMode cleared.
; HOW: Tests and compares of zField8C; stores; StageFrameA0673CD.
; WAITS: no.
; Entry: nothing. Exit: as $A067 left them.
StageSubStepCleared3CD:
    lda z:zField8C
    beq StageSubStepCleared3CD_at0
    bmi StageSubStepCleared3CD_at1
    ldx #$01
    stx a:Field2742+1
    cmp #$03
    bcc StageSubStepCleared3CD_at2
    cmp #$07
    bcc StageSubStepCleared3CD_at3
    cmp #$0F
    bcs StageSubStepCleared3CD_at3
StageSubStepCleared3CD_at2:
    lda #$01
    bra StageSubStepCleared3CD_at4
StageSubStepCleared3CD_at3:
    lda #$03
    bra StageSubStepCleared3CD_at4
StageSubStepCleared3CD_at1:
    lda #$04
    bra StageSubStepCleared3CD_at4
StageSubStepCleared3CD_at0:
    lda #$02
StageSubStepCleared3CD_at4:
    sta z:zClearKind
    lda #$03
    sta z:zGameSubMode
    stz z:zField85
    jsr StageFrameA0673CD
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz a:TimeSlowMode
    rts

; @routine 03CD:2BD StageSubStepFadeOut3CD
; @run 6B:82BD
; @unreachable 6B:82BD-6B:82C7 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $82BD runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-step 4: sub-mode 4 with zField80 and zField89 cleared, then the overlay at $A000's piece at $A067.
; HOW: Stores; a jump to StageFrameA0673CD.
; WAITS: no.
; Entry: nothing. Exit: as $A067 leaves them.
StageSubStepFadeOut3CD:
    lda #$04
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jmp StageFrameA0673CD

; @routine 03CD:2C8 StageSubStepFadeOutB3CD
; @run 6B:82C8
; @unreachable 6B:82C8-6B:82D4 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $82C8 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-step 5: sub-mode 4 entered at its step 5 (zField80 5, zField89 cleared), then the overlay at
; $A000's piece at $A067.
; HOW: Stores; a jump to StageFrameA0673CD.
; WAITS: no.
; Entry: nothing. Exit: as $A067 leaves them.
StageSubStepFadeOutB3CD:
    lda #$04
    sta z:zGameSubMode
    lda #$05
    sta z:zField80
    stz z:zField89
    jmp StageFrameA0673CD

; @routine 03CD:2D5 StageSubStepTally3CD
; @run 6B:82D5
; @unreachable 6B:82D5-6B:82FF no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $82D5 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-step 6: sub-mode 6 with zField80, zField89 and the fall's step cleared, the overlay at $A000's
; piece at $A067, the forced presses cleared, the player's step at $3358 0 and its count at $3310 $40; when
; TimeSlowMode is set it is cleared and the fade starts again from 2.
; HOW: Stores; StageFrameA0673CD; a jump into StageFadeTimer3CD.
; WAITS: no.
; Entry: nothing. Exit: A TimeSlowMode (0), N and Z from it, the rest as $A067 left them; else as StageFadeTimer3CD.
StageSubStepTally3CD:
    lda #$06
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jsr StageFrameA0673CD
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz z:zFallStep
    stz a:OBJ_WORK_C
    lda #$40
    sta a:OBJ_WORK_B+16
    lda a:TimeSlowMode
    bne StageSubStepTally3CD_at0
    rts
StageSubStepTally3CD_at0:
    stz a:TimeSlowMode
    lda #$02
    sta z:zFadeCount
    jmp StageFadeTimer3CD

; @routine 03CD:300 StageSubModeDeath3CD
; @run 6B:8300
; @unreachable 6B:8300-6B:8317 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8300 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-mode 2, the player's death: sound command $FF, the death's phase; once the phase has put the stage
; back in sub-mode 1, the overlay at $A000's piece at $A015 and the level's start.
; HOW: The sound hook; StageDeathPhase3CD; a compare; MapOverlayBanks5And6, $A015 and a jump to StageLevelStart3CD.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit: A zGameSubMode, N, Z and C from its compare with 1; else as StageLevelStart3CD.
StageSubModeDeath3CD:
    lda #$FF
    jsr VecSoundCommand
    jsr StageDeathPhase3CD
    lda z:zGameSubMode
    cmp #$01
    beq StageSubModeDeath3CD_at0
    rts
StageSubModeDeath3CD_at0:
    jsr MapOverlayBanks5And6At802D
    jsr RichterVecCE1D
    jmp StageLevelStart3CD

; @routine 03CD:318 StageDeathPhase3CD
; @run 6B:8318
; @stop 831F
; @unreachable 6B:8318-6B:831E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8318 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The death's phase by zField87, from the table of four at $831F.
; HOW: zField87 doubled indexes the table, whose word is jumped through.
; WAITS: as the phase.
; Entry: nothing. Exit: as the phase leaves them; it is entered with A and X twice zField87, N and Z from it, C its
; bit 7.
StageDeathPhase3CD:
    lda z:zField87
    asl a
    tax
    jmp ($831F,x)

; @routine 03CD:327 StageDeathRespawn3CD
; @run 6B:8327
; @unreachable 6B:8327-6B:8345 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8327 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Phase 0 of the death: the screen and the player put back, sub-mode 1; when zField8C is set it is
; cleared and the player's count at $3310 is $18; the step at $3358 from the two at $8346 by the facing bit.
; HOW: StageResetScroll3CD; stores; one table read.
; WAITS: no.
; Entry: nothing. Exit: A the step, X the facing bit, N and Z from A; Y, C and V as StageResetScroll3CD left them.
StageDeathRespawn3CD:
    jsr StageResetScroll3CD
    lda #$01
    sta z:zGameSubMode
    lda z:zField8C
    beq StageDeathRespawn3CD_at0
    stz z:zField8C
    lda #$18
    sta a:OBJ_WORK_B+16
StageDeathRespawn3CD_at0:
    lda a:OBJ_SUBSTATE+24
    and #$01
    tax
    lda a:$8346,x
    sta a:OBJ_WORK_C
    rts

; @routine 03CD:348 StageDeathRespawnB3CD
; @run 6B:8348
; @unreachable 6B:8348-6B:8366 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8348 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Phase 1 of the death: as phase 0 (the same bytes, the same table at $8346).
; HOW: StageResetScroll3CD; stores; one table read.
; WAITS: no.
; Entry: nothing. Exit: as StageDeathRespawn3CD.
StageDeathRespawnB3CD:
    jsr StageResetScroll3CD
    lda #$01
    sta z:zGameSubMode
    lda z:zField8C
    beq StageDeathRespawnB3CD_at0
    stz z:zField8C
    lda #$18
    sta a:OBJ_WORK_B+16
StageDeathRespawnB3CD_at0:
    lda a:OBJ_SUBSTATE+24
    and #$01
    tax
    lda a:$8346,x
    sta a:OBJ_WORK_C
    rts

; @routine 03CD:367 StageDeathFallLeft3CD
; @run 6B:8367
; @unreachable 6B:8367-6B:8378 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8367 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Phase 2 of the death, the fall to the left, as StageDeathFallLeft152D (its bytes, here running into
; this build's StageDeathFallStart3CD): step 0 sets the player's Y and runs on into StageDeathFallStart3CD; step 1
; moves the player; step 2 puts it back (StageDeathFallBackLeft3CD).
; HOW: Two decrements of the step's copy in A; the pieces of the overlay at $A000.
; WAITS: no.
; Entry: nothing. Exit: as the step that ran left them.
StageDeathFallLeft3CD:
    lda z:zFallStep
    dec a
    beq StageDeathFallStart3CD_at2
    dec a
    beq StageDeathFallBackLeft3CD
    lda #$40
    sta a:OBJ_Y_FRAC
    lda #$01
    sta a:OBJ_Y_LO+16

; @routine 03CD:379 StageDeathFallStart3CD
; @run 6B:8379
; @unreachable 6B:8379-6B:83A1 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8379 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The fall's first step: the overlay at $A000's pieces at $A018 (with A 0) and $A012, and the fall's step
; at $88 counted.
; HOW: MapOverlayBanks5And6, $A018, $A012; an increment of $88.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the step's increment; A, X, Y, C and V as $A012 left them.
StageDeathFallStart3CD:
    jsr MapOverlayBanks5And6At802D
    lda #$00
    jsr RichterVecSetAnimation
    jsr RichterVecAnimationSound
    bra StageDeathFallStart3CD_at0
StageDeathFallStart3CD_at2:
    jsr MapOverlayBanks5And6At802D
    jsr RichterVecSetIdle
    jsr RichterVecAnimationSound
    lda a:OBJ_SPRITE_ATTR+24
    beq StageDeathFallStart3CD_at1
    lda a:OBJ_X_HI+8
    cmp #$10
    bcs StageDeathFallStart3CD_at0
StageDeathFallStart3CD_at1:
    clx
    jmp RichterVecCE73
StageDeathFallStart3CD_at0:
    inc z:zFallStep
    rts

; @routine 03CD:3A2 StageDeathFallBackLeft3CD
; @run 6B:83A2
; @unreachable 6B:83A2-6B:83B5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $83A2 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The left fall's last step: the screen and the player put back (StageResetScroll3CD), sub-mode 1,
; zField8C cleared, the count at $3310 $20 and the step at $3358 $20.
; HOW: StageResetScroll3CD; stores.
; WAITS: no.
; Entry: nothing. Exit: A $20, N and Z from it; X, Y, C and V as StageResetScroll3CD left them.
StageDeathFallBackLeft3CD:
    jsr StageResetScroll3CD
    lda #$01
    sta z:zGameSubMode
    stz z:zField8C
    lda #$20
    sta a:OBJ_WORK_B+16
    lda #$20
    sta a:OBJ_WORK_C
    rts

; @routine 03CD:3B6 StageDeathFallRight3CD
; @run 6B:83B6
; @unreachable 6B:83B6-6B:83FA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $83B6 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Phase 3 of the death, the fall to the right: step 0 sets the player's Y ($C0, $FE), calls
; StageDeathFallStart3CD and moves the player as step 1 does; step 1 moves it while it is shown and left of X $F0;
; step 2 puts it back (StageResetScroll3CD, sub-mode 1, zField8C cleared, the count $20, the step $80).
; HOW: Two decrements of the step's copy in A; the pieces of the overlay at $A000.
; WAITS: no.
; Entry: nothing. Exit: as the step that ran left them.
StageDeathFallRight3CD:
    lda z:zFallStep
    dec a
    beq StageDeathFallRight3CD_at0
    dec a
    beq StageDeathFallRight3CD_at1
    lda #$C0
    sta a:OBJ_Y_FRAC
    lda #$FE
    sta a:OBJ_Y_LO+16
    jsr StageDeathFallStart3CD
StageDeathFallRight3CD_at0:
    jsr MapOverlayBanks5And6At802D
    jsr RichterVecSetIdle
    jsr RichterVecAnimationSound
    lda a:OBJ_SPRITE_ATTR+24
    beq StageDeathFallRight3CD_at2
    lda a:OBJ_X_HI+8
    cmp #$F0
    bcc StageDeathFallRight3CD_at3
StageDeathFallRight3CD_at2:
    clx
    jmp RichterVecCE73
StageDeathFallRight3CD_at3:
    inc z:zFallStep
    rts
StageDeathFallRight3CD_at1:
    jsr StageResetScroll3CD
    lda #$01
    sta z:zGameSubMode
    stz z:zField8C
    lda #$20
    sta a:OBJ_WORK_B+16
    lda #$80
    sta a:OBJ_WORK_C
    rts

; @routine 03CD:3FB StageSubModeCleared3CD
; @run 6B:83FB
; @stop 8405
; @unreachable 6B:83FB-6B:8404 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $83FB runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-mode 3, the stage cleared: the events step, then the step of the table $84 picked from the five at
; $8405 (the next stage's start, the walk out, the walk out after the boss, the last walk, the stage's end).
; HOW: StageModeEventsStep152D (its bytes at $81AD); $84 doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice $84, N and Z from it, C its bit 7.
StageSubModeCleared3CD:
    jsr StageModeEventsStep3CD
    lda z:zClearKind
    asl a
    tax
    jmp ($8405,x)

; @routine 03CD:40F StageClearedNextStage3CD
; @run 6B:840F
; @unreachable 6B:840F-6B:8438 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $840F runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The cleared stage's first table: the next stage set up as the start does (the kernel's piece at $4009,
; the route's pieces, the row map, the screen, the display, zField8A 2, the overlay at $A000's start, the level) and
; put in play, sub-mode 1, unpaused, zField86 zField83.
; HOW: Calls in turn $4009, StageRouteSetUpA3CD, StageRouteSetUpB3CD, StageLoadRowMap3CD, StageResetScroll3CD,
; StageHudStart3CD, MapOverlayBanks5And6, $A003 and StageLevelStart3CD.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit: A zField83, N and Z from it; X, Y, C and V as StageLevelStart3CD left them.
StageClearedNextStage3CD:
    jsr VecClearScreenState
    jsr StageRouteSetUpA3CD
    jsr StageRouteSetUpB3CD
    jsr StageLoadRowMap3CD
    jsr StageResetScroll3CD
    jsr StageHudStart3CD
    lda #$02
    sta z:zField8A
    jsr MapOverlayBanks5And6At802D
    jsr RichterVecStart
    jsr StageLevelStart3CD
    stz z:zPaused
    lda #$01
    sta z:zGameSubMode
    lda z:zField83
    sta z:zField86
    rts

; @routine 03CD:445 StageWalkSprite3CD
; @run 6B:8445
; @unreachable 6B:8445-6B:8452 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8445 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The walking player's sprite at $2A58 by the character (the two at $8453), then the overlay at $A000's
; piece at $A012.
; HOW: MapOverlayBanks5And6; a table read; a jump to $A012.
; WAITS: no.
; Entry: nothing. Exit: as $A012 leaves them.
StageWalkSprite3CD:
    jsr MapOverlayBanks5And6At802D
    ldx z:zCharacter
    lda a:$8453,x
    sta a:OBJ_SPRITE_SET+72
    jmp RichterVecAnimationSound

; @routine 03CD:455 StageClearedWalkOut3CD
; @run 6B:8455
; @stop 8462
; @unreachable 6B:8455-6B:8461 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8455 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The cleared stage's second table: the kernel's piece at $4096 under bank 7, then its step at $85 from
; the five at $8462 (the wait for the objects, the walk, the scroll's wait, the walk back, the walk on).
; HOW: MapOverlayBank7; $4096; $85 doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them.
StageClearedWalkOut3CD:
    jsr MapOverlayBank7At8044
    jsr VecStreamScrollStep
    lda z:zField85
    asl a
    tax
    jmp ($8462,x)

; @routine 03CD:46C StageClearedWaitObjects3CD
; @run 6B:846C
; @unreachable 6B:846C-6B:8489 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $846C runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A step of the cleared stage: the walking sprite, the overlay at $A000's pieces at $A009 and $A08D and
; the overlays' frame; once no object slot from $40 down to 1 holds a state from 1 to $7F at $27D0, $2743 cleared and
; the step counted.
; HOW: StageWalkSprite3CD, StageFrameA0093CD, StageFrameA08D3CD, StageOverlayFrame3CD; a loop down X.
; WAITS: no.
; Entry: nothing. Exit, an object busy: A its state, X its slot, N and Z from A. Else: X 0, N and Z from the step's
; increment.
StageClearedWaitObjects3CD:
    jsr StageWalkSprite3CD
    jsr StageFrameA0093CD
    jsr StageFrameA08D3CD
    jsr StageOverlayFrame3CD
    ldx #$40
StageClearedWaitObjects3CD_at1:
    lda a:OBJ_STATE+40,x
    bmi StageClearedWaitObjects3CD_at0
    bne StageClearedWaitScroll3CD_at0
StageClearedWaitObjects3CD_at0:
    dex
    bne StageClearedWaitObjects3CD_at1
    stz a:Field2742+1
    inc z:zField85
    rts

; @routine 03CD:48A StageClearedWalk3CD
; @run 6B:848A
; @unreachable 6B:848A-6B:84C3 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $848A runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A step of the cleared stage: the player walks the scroll ($3E86 the step from $8439 by the side, $3E7E
; and $3E00 cleared, its X moved by $843B); at the side's end ($843D) the overlay's piece at $A067 runs, $3E86 is
; cleared and the step counted.
; HOW: The side from zField8C; table reads, an add; StageFrameA08D3CD, StageOverlayFrame3CD; a compare.
; WAITS: no.
; Entry: nothing. Exit, short of the end: A the X, N, Z and C from the compare, Y the side. At the end: N and Z from
; the step's increment, the rest as $A067 left them.
StageClearedWalk3CD:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$8439,y
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    stz a:ScrollXFrac
    lda a:OBJ_X_HI+8
    clc
    adc a:$843B,y
    sta a:OBJ_X_HI+8
    jsr StageFrameA08D3CD
    jsr StageOverlayFrame3CD
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_HI+8
    cmp a:$843D,y
    beq StageClearedWalk3CD_at0
    rts
StageClearedWalk3CD_at0:
    jsr StageFrameA0673CD
    stz a:ScrollStepXHi
    inc z:zField85
    rts

; @routine 03CD:4C4 StageClearedWaitScroll3CD
; @run 6B:84C4
; @unreachable 6B:84C4-6B:84CD no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $84C4 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A step of the cleared stage: the overlays' frame; once zField8C is negative the step is counted.
; HOW: StageOverlayFrame3CD; a test; an increment.
; WAITS: no.
; Entry: nothing. Exit: A zField8C, N and Z from it or from the step's increment.
StageClearedWaitScroll3CD:
    jsr StageOverlayFrame3CD
    lda z:zField8C
    bpl StageClearedWaitScroll3CD_at0
    inc z:zField85
StageClearedWaitScroll3CD_at0:
    rts

; @routine 03CD:4CE StageClearedWalkBack3CD
; @run 6B:84CE
; @unreachable 6B:84CE-6B:8510 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $84CE runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A step of the cleared stage: the player walks back (count $E0, step from $8441 by the side, the
; overlay at $A000's frame, its X moved back by $843B); at the side's mark ($843F) zField8C loses bit 7, the player
; stops, zField8A is 2, the overlay's piece at $A003 and the walking sprite run and the step is counted.
; HOW: The side from zField8C; table reads; StageFrameA0003CD; a subtract and a compare; stores; $A003,
; StageWalkSprite3CD.
; WAITS: no.
; Entry: nothing. Exit, short of the mark: A the X, N, Z and C from the compare, V from the subtract, Y the side. At
; the mark: N and Z from the step's increment.
StageClearedWalkBack3CD:
    lda #$E0
    sta a:OBJ_WORK_B+16
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$8441,y
    sta a:OBJ_WORK_C
    jsr StageFrameA0003CD
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_HI+8
    sec
    sbc a:$843B,y
    sta a:OBJ_X_HI+8
    cmp a:$843F,y
    beq StageClearedWalkBack3CD_at0
    rts
StageClearedWalkBack3CD_at0:
    lda z:zField8C
    and #$7F
    sta z:zField8C
    stz a:OBJ_WORK_B+16
    stz a:OBJ_WORK_C
    lda #$02
    sta z:zField8A
    jsr RichterVecStart
    jsr StageWalkSprite3CD
    inc z:zField85
    rts

; @routine 03CD:511 StageClearedWalkOn3CD
; @run 6B:8511
; @unreachable 6B:8511-6B:85A2 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8511 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The cleared stage's last step: the player walks on with the scroll until ScrollXLo is 0; then the next
; stage: below zField8C 7 the route's next stage from the table at $9D60 (zField82 counted, into zField83 and
; zField86), else zField83 counted with zField8E's bit 4 set and sound $FF; zField8E's bit 1 set; the route's pieces
; below 7, the row map, the scroll's bytes at $3E00 (48) and $3E66 (136) cleared, the screen loaded (with zField8E
; kept, and again under bank 7 after $A74E for camera mode $0D); bit 1 cleared; the level started unless zField8C is 7
; or 8; zField8C and the pause cleared, sub-mode 1.
; HOW: As StageClearedWalk3CD for the walk; calls, TAI, bit changes.
; WAITS: where the pieces it calls wait.
; Entry: nothing. Exit, walking: A ScrollXLo, N and Z from it. Else: A 1, N and Z from it.
StageClearedWalkOn3CD:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$8439,y
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    lda a:OBJ_X_HI+8
    clc
    adc a:$843B,y
    sta a:OBJ_X_HI+8
    jsr StageFrameA08D3CD
    jsr StageOverlayFrame3CD
    lda a:ScrollXLo
    beq StageClearedWalkOn3CD_at0
    rts
StageClearedWalkOn3CD_at0:
    lda z:zField8C
    cmp #$07
    bcs StageClearedWalkOn3CD_at1
    inc z:zField82
    lda z:zField82
    tay
    lda a:$9D60,y
    sta z:zField83
    sta z:zField86
    bra StageClearedWalkOn3CD_at2
StageClearedWalkOn3CD_at1:
    inc z:zField83
    smb4 z:zField8E
    lda #$FF
    jsr VecSoundCommand
StageClearedWalkOn3CD_at2:
    smb1 z:zField8E
    lda z:zField8C
    cmp #$07
    bcs StageClearedWalkOn3CD_at3
    jsr StageRouteSetUpA3CD
    jsr StageRouteSetUpB3CD
StageClearedWalkOn3CD_at3:
    jsr StageLoadRowMap3CD
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    lda z:zField8E
    pha
    jsr StageLoadScreen3CD
    pla
    sta z:zField8E
    lda a:CameraMode
    cmp #$0D
    bne StageClearedWalkOn3CD_at4
    jsr MapOverlayBank7At8044
    jsr SetBlinkFlash
    jsr StageLoadScreen3CD
StageClearedWalkOn3CD_at4:
    rmb1 z:zField8E
    lda z:zField8C
    cmp #$09
    bcs StageClearedWalkOn3CD_at6
    cmp #$07
    bcs StageClearedWalkOn3CD_at5
StageClearedWalkOn3CD_at6:
    jsr StageLevelStart3CD
StageClearedWalkOn3CD_at5:
    stz z:zField8C
    stz z:zPaused
    lda #$01
    sta z:zGameSubMode
    rts

; @routine 03CD:5A3 StageClearedBossWalk3CD
; @run 6B:85A3
; @stop 85AD
; @unreachable 6B:85A3-6B:85AC no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $85A3 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The cleared stage's third table, after a boss: the fade's countdown, then its step at $85 from the
; three at $85AD (the scroll to the boss's door, the wait for the door, the walk out).
; HOW: StageFadeTimer3CD; $85 doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them.
StageClearedBossWalk3CD:
    jsr StageFadeTimer3CD
    lda z:zField85
    asl a
    tax
    jmp ($85AD,x)

; @routine 03CD:5B3 StageBossScrollToDoor3CD
; @run 6B:85B3
; @unreachable 6B:85B3-6B:85E2 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $85B3 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A step after the boss: the walking sprite and the overlay at $A000's piece at $A009, the scroll stepped
; by 1 with the player's X one less, the overlays' frame; once ScrollXHi is 4 the overlay's piece at $A067 runs, the
; scroll's steps are cleared, sound $58 is sent and the step counted.
; HOW: StageWalkSprite3CD, StageFrameA0093CD; stores and a decrement; StageFrameA08D3CD, StageOverlayFrame3CD; a
; compare; StageFrameA0673CD, the sound hook.
; WAITS: no.
; Entry: nothing. Exit, short of 4: A ScrollXHi, N, Z and C from the compare. Else: N and Z from the step's increment.
StageBossScrollToDoor3CD:
    jsr StageWalkSprite3CD
    jsr StageFrameA0093CD
    lda #$01
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    dec a:OBJ_X_HI+8
    jsr StageFrameA08D3CD
    jsr StageOverlayFrame3CD
    lda a:ScrollXHi
    cmp #$04
    beq StageBossScrollToDoor3CD_at0
    rts
StageBossScrollToDoor3CD_at0:
    jsr StageFrameA0673CD
    stz a:ScrollStepXHi
    stz a:ScrollStepXLo+2
    lda #$58
    jsr VecSoundCommand
    inc z:zField85
    rts

; @routine 03CD:5E3 StageBossWaitDoor3CD
; @run 6B:85E3
; @unreachable 6B:85E3-6B:85FC no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $85E3 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A step after the boss: the overlay at $A000's piece at $A009 and the overlays' frame; once the door's
; byte at $283A is 4 and its animation step at $340A is 0, the player's step at $3358 is $20 and the step counted.
; HOW: StageFrameA0093CD, StageOverlayFrame3CD; two tests; a store and an increment.
; WAITS: no.
; Entry: nothing. Exit: A the last byte tested, N and Z from its test, or $20 with N and Z from the increment.
StageBossWaitDoor3CD:
    jsr StageFrameA0093CD
    jsr StageOverlayFrame3CD
    lda a:OBJ_SUBSTATE+58
    cmp #$04
    bne StageBossWaitDoor3CD_at0
    lda a:OBJ_ANIM_STEP+2
    bne StageBossWaitDoor3CD_at0
    lda #$20
    sta a:OBJ_WORK_C
    inc z:zField85
StageBossWaitDoor3CD_at0:
    rts

; @routine 03CD:5FD StageBossWalkOut3CD
; @run 6B:85FD
; @unreachable 6B:85FD-6B:8618 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $85FD runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The last step after the boss: the player walks out (count $B8) with the overlay at $A000's frame, the
; kernel's piece at $40D8 and the piece at $A009; at X $F0 the route's next stage (StageNextRouteStage3CD).
; HOW: StageFrameA0003CD, $40D8, $A009, StageFrameA08D3CD, StageOverlayFrame3CD; a compare; a branch to
; StageNextRouteStage3CD.
; WAITS: no.
; Entry: nothing. Exit, short of $F0: A the X, N, Z and C from the compare. Else: as StageNextRouteStage3CD.
StageBossWalkOut3CD:
    lda #$B8
    sta a:OBJ_WORK_B+16
    jsr StageFrameA0003CD
    jsr VecPlayerAndCameraFrame
    jsr RichterVecC3A7
    jsr StageFrameA08D3CD
    jsr StageOverlayFrame3CD
    lda a:OBJ_X_HI+8
    cmp #$F0
    beq StageNextRouteStage3CD
    rts

; @routine 03CD:619 StageNextRouteStage3CD
; @run 6B:8619
; @unreachable 6B:8619-6B:8636 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8619 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The route's next stage: zDisplayDelay 2, the player stopped, $2743 cleared, zField82 counted and the
; stage from the table at $9D60 into zField83, $84 and the byte at $2A58 cleared.
; HOW: Stores; an increment; a table read.
; WAITS: no.
; Entry: nothing. Exit: A 0 (CLA keeps the flags), N and Z from the table's load, Y zField82.
StageNextRouteStage3CD:
    lda #$02
    sta z:zDisplayDelay
    stz a:OBJ_WORK_B+16
    stz a:Field2742+1
    stz a:OBJ_WORK_C
    inc z:zField82
    lda z:zField82
    tay
    lda a:$9D60,y
    sta z:zField83
    cla
    sta z:zClearKind
    stz a:OBJ_SPRITE_SET+72
    rts

; @routine 03CD:637 StageClearedLastWalk3CD
; @run 6B:8637
; @stop 8644
; @unreachable 6B:8637-6B:8643 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8637 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The cleared stage's fourth table: the kernel's piece at $4096 under bank 7, then its step at $85 from
; the five at $8644 (the wait for the objects, a frame, the scroll's wait, zField8C's bit 7 cleared, the walk off).
; HOW: MapOverlayBank7; $4096; $85 doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them.
StageClearedLastWalk3CD:
    jsr MapOverlayBank7At8044
    jsr VecStreamScrollStep
    lda z:zField85
    asl a
    tax
    jmp ($8644,x)

; @routine 03CD:64E StageClearedFrame3CD
; @run 6B:864E
; @unreachable 6B:864E-6B:8653 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $864E runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A step of the last walk: the overlays' frame and the step counted.
; HOW: StageOverlayFrame3CD; an increment.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the increment; the rest as StageOverlayFrame3CD left them.
StageClearedFrame3CD:
    jsr StageOverlayFrame3CD
    inc z:zField85
    rts

; @routine 03CD:654 StageClearedClearBit3CD
; @run 6B:8654
; @unreachable 6B:8654-6B:865F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8654 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A step of the last walk: the overlays' frame, zField8C's bit 7 cleared and the step counted.
; HOW: StageOverlayFrame3CD; a mask; an increment.
; WAITS: no.
; Entry: nothing. Exit: A zField8C, N and Z from the step's increment.
StageClearedClearBit3CD:
    jsr StageOverlayFrame3CD
    lda z:zField8C
    and #$7F
    sta z:zField8C
    inc z:zField85
    rts

; @routine 03CD:660 StageClearedWalkOff3CD
; @run 6B:8660
; @unreachable 6B:8660-6B:86BA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8660 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The last walk off the screen: the overlays' frame; the player's step and count from $8441 by the side,
; the overlay at $A000's frame, its X moved by $8439 (its page at $2B30 too when it wraps); at the side's mark ($8443):
; below zField8C $0F zModeDone is counted; from $0F up the stage goes on to the room zField8C less one over 16 and the
; entry 2 or 3 by the side, through StageSubStepDeath3CD.
; HOW: StageOverlayFrame3CD; table reads; StageFrameA0003CD; adds and a compare; shifts; a jump to
; StageSubStepDeath3CD.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction or piece that ran left them.
StageClearedWalkOff3CD:
    jsr StageOverlayFrame3CD
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$8441,y
    sta a:OBJ_WORK_C
    sta a:OBJ_WORK_B+16
    jsr StageFrameA0003CD
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:$8439,y
    clc
    adc a:OBJ_X_HI+8
    sta a:OBJ_X_HI+8
    bne StageClearedWalkOff3CD_at0
    lda a:$8439,y
    clc
    adc a:OBJ_SPRITE_ATTR+24
    sta a:OBJ_SPRITE_ATTR+24
StageClearedWalkOff3CD_at0:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_HI+8
    cmp a:$8443,y
    bne StageClearedWalkOff3CD_at1
    lda z:zField8C
    cmp #$0F
    bcs StageClearedWalkOff3CD_at2
    inc z:zModeDone
StageClearedWalkOff3CD_at1:
    rts
StageClearedWalkOff3CD_at2:
    dec a
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zField83
    lda z:zField8C
    dec a
    and #$01
    ora #$02
    sta z:zField87
    jmp StageSubStepDeath3CD

; @routine 03CD:6BB StageClearedEnding3CD
; @run 6B:86BB
; @unreachable 6B:86BB-6B:86D5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $86BB runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The cleared stage's fifth table: sound $FF and the overlay at $A000's piece at $A015; zField8C keeps
; only its low seven bits; when they are 0 the route goes on (StageNextRouteStage3CD), else zModeDone is counted.
; HOW: The sound hook; MapOverlayBanks5And6, $A015; a mask; a jump to StageNextRouteStage3CD.
; WAITS: no.
; Entry: nothing. Exit: as StageNextRouteStage3CD, or A zField8C's bits, N and Z from zModeDone's increment.
StageClearedEnding3CD:
    lda #$FF
    jsr VecSoundCommand
    jsr MapOverlayBanks5And6At802D
    jsr RichterVecCE1D
    lda z:zField8C
    and #$7F
    bne StageClearedEnding3CD_at0
    stz z:zField8C
    jmp StageNextRouteStage3CD
StageClearedEnding3CD_at0:
    sta z:zField8C
    inc z:zModeDone
    rts

; @routine 03CD:6D6 StageCenterPlayer3CD
; @run 6B:86D6
; @unreachable 6B:86D6-6B:874A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $86D6 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The play frame's scroll while zField8E's bit 7 is set: the player's count at $3310 $80 and its step
; cleared, the overlay at $A000's piece at $A009. Unless the byte at $2860 is set or the player's state at $2818 is 2
; or more, PlayerStartPose is 1 and the player is walked to the screen's middle ($80) by 4 a frame with the scroll
; moving the other way and the overlay's piece at $A08D; there the centring ends.
; HOW: Tests; StageFrameA0093CD; adds, masks and compares; StageFrameA08D3CD; stores and a bit cleared.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction or piece that ran left them.
StageCenterPlayer3CD:
    lda a:$2860
    bne StageCenterPlayer3CD_at0
    lda a:OBJ_SUBSTATE+24
    cmp #$02
    bcc StageCenterPlayer3CD_at1
StageCenterPlayer3CD_at0:
    lda #$80
    sta a:OBJ_WORK_B+16
    stz a:OBJ_WORK_C
    jsr StageFrameA0093CD
    rts
StageCenterPlayer3CD_at1:
    lda #$80
    sta a:OBJ_WORK_B+16
    stz a:OBJ_WORK_C
    lda #$01
    sta a:PlayerStartPose
    jsr StageFrameA0093CD
    lda a:OBJ_X_HI+8
    clc
    adc #$02
    and #$FC
    cmp #$80
    beq StageCenterPlayer3CD_at5
    lda a:OBJ_X_HI+8
    bmi StageCenterPlayer3CD_at2
    lda #$FC
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    lda a:OBJ_X_HI+8
    clc
    adc #$04
    sta a:OBJ_X_HI+8
    bra StageCenterPlayer3CD_at3
StageCenterPlayer3CD_at2:
    lda #$04
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    lda a:OBJ_X_HI+8
    clc
    adc #$FC
    sta a:OBJ_X_HI+8
StageCenterPlayer3CD_at3:
    jsr StageFrameA08D3CD
    lda a:OBJ_X_HI+8
    clc
    adc #$02
    and #$F8
    cmp #$80
    bne StageCenterPlayer3CD_at4
StageCenterPlayer3CD_at5:
    rmb7 z:zField8E
    stz a:OBJ_WORK_B+16
    stz a:PlayerStartPose
StageCenterPlayer3CD_at4:
    rts

; @routine 03CD:74B StageSubModeFade3CD
; @run 6B:874B
; @stop 8779
; @unreachable 6B:874B-6B:8778 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $874B runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-mode 4, the fade out: the events step and the display; unless the fade is at its step 3, the
; overlay's piece at $A08D for the raster effects $0C and $0D, the overlays' frame on every fourth frame, and
; TimeSlowMode cleared; then the step at zField80 from the seven at $8779, under banks 5 and 6.
; HOW: StageModeEventsStep152D, StageHudStep3CD; compares; StageFrameA08D3CD; TST of zFrameTick;
; StageOverlayFrame3CD; MapOverlayBanks5And6; zField80 doubled indexes the table, whose word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them.
StageSubModeFade3CD:
    jsr StageModeEventsStep3CD
    jsr StageHudStep3CD
    lda z:zField80
    cmp #$03
    beq StageSubModeFade3CD_at3
    lda z:zRasterEffect
    cmp #$0C
    beq StageSubModeFade3CD_at0
    cmp #$0D
    bne StageSubModeFade3CD_at1
StageSubModeFade3CD_at0:
    jsr StageFrameA08D3CD
StageSubModeFade3CD_at1:
    tst #$03,z:zFrameTick
    bne StageSubModeFade3CD_at2
    jsr StageOverlayFrame3CD
StageSubModeFade3CD_at2:
    stz a:TimeSlowMode
StageSubModeFade3CD_at3:
    jsr MapOverlayBanks5And6At802D
    lda z:zField80
    asl a
    tax
    jmp ($8779,x)

; @routine 03CD:787 StageFadeStart3CD
; @run 6B:8787
; @unreachable 6B:8787-6B:878F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8787 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The fade's first step: the overlay at $A000's pieces at $A00C and $A012, and the step counted.
; HOW: Two calls and an increment of zField80.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the increment; A, X, Y, C and V as $A012 left them.
StageFadeStart3CD:
    jsr RichterVecC023
    jsr RichterVecAnimationSound
    inc z:zField80
    rts

; @routine 03CD:790 StageFadeWait3CD
; @run 6B:8790
; @unreachable 6B:8790-6B:87A7 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8790 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The fade's second step: the overlay at $A000's piece at $A00F, and $A012 again when it changed the byte
; at $2A58; once zField89 is set the step is counted.
; HOW: The byte kept on the stack across $A00F and compared after it; a test; an increment.
; WAITS: no.
; Entry: nothing. Exit: A zField89, N and Z from it or from zField80's increment.
StageFadeWait3CD:
    lda a:OBJ_SPRITE_SET+72
    pha
    jsr RichterVecC076
    pla
    cmp a:OBJ_SPRITE_SET+72
    beq StageFadeWait3CD_at1
    jsr RichterVecAnimationSound
StageFadeWait3CD_at1:
    lda z:zField89
    bne StageFadeWait3CD_at0
    rts
StageFadeWait3CD_at0:
    inc z:zField80
    rts

; @routine 03CD:7A8 StageFadeStopCd3CD
; @run 6B:87A8
; @stop 87B7
; @unreachable 6B:87A8-6B:87B6 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $87A8 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The fade's third step: the CD paused (cd_pause until it answers 0); with no stock left zModeDone is
; $80 (the game over), else a life is taken from zStock (at $87C0 in StageFadeTakeStock152D's bytes), the wait at
; zWaitTimer3 set to $60 and the step counted.
; HOW: cd_pause in a loop; a test of zStock; a decimal subtract and stores.
; WAITS: in cd_pause, as the card's routine does.
; Entry: D clear, as the game keeps it. Exit, no stock: A $80, N and Z from it. Else: A $60, N and Z from zField80's
; increment, C from the subtract; X 0.
StageFadeStopCd3CD:
    jsr BiosCdPause
    tax
    bne StageFadeStopCd3CD
    lda z:zStock
    bne $87C0
    lda #$80
    sta z:zModeDone
    rts

; @routine 03CD:7E0 StageFadeRestart3CD
; @run 6B:87E0
; @unreachable 6B:87E0-6B:87FA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $87E0 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The fade's last step: the stage starts again from sub-mode 0, zField83 back to zField86, zField8A and
; zField8F cleared, zField8E's bit 2 cleared; when zStage is negative the CD is paused first and bit 3 set.
; HOW: Stores and bit changes; cd_pause in a loop.
; WAITS: in cd_pause, as the card's routine does.
; Entry: nothing. Exit: A 0, N and Z from it.
StageFadeRestart3CD:
    lda z:zField86
    sta z:zField83
    stz z:zField8A
    stz z:zField8F
    rmb2 z:zField8E
    lda z:zStage
    bpl StageFadeRestart3CD_at0
StageFadeRestart3CD_at1:
    jsr BiosCdPause
    tax
    bne StageFadeRestart3CD_at1
    smb3 z:zField8E
StageFadeRestart3CD_at0:
    lda #$00
    sta z:zGameSubMode
    rts

; @routine 03CD:7FB StageSubModeOpening3CD
; @run 6B:87FB
; @unreachable 6B:87FB-6B:881D no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $87FB runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-mode 5, the stage's opening: the events step, the display, the overlay at $A000's frame and its
; piece at $A08D, the overlays' frame, the opening's step; once zField8E's bit 0 is set TimeSlowMode is cleared,
; sub-mode 1 (the player's count at $3310 1) and PlayerStartPose cleared.
; HOW: StageModeEventsStep152D, StageHudStep3CD, StageFrameA0003CD, StageFrameA08D3CD, StageOverlayFrame3CD,
; StageOpeningStep3CD; a bit test; stores.
; WAITS: no.
; Entry: nothing. Exit: A 1, N and Z from it when the stage goes into play; else as StageOpeningStep3CD left them.
StageSubModeOpening3CD:
    jsr StageModeEventsStep3CD
    jsr StageHudStep3CD
    jsr StageFrameA0003CD
    jsr StageFrameA08D3CD
    jsr StageOverlayFrame3CD
    jsr StageOpeningStep3CD
    bbr0 zField8E,StageSubModeOpening3CD_at0
    stz a:TimeSlowMode
    lda #$01
    sta z:zGameSubMode
    sta a:OBJ_WORK_B+16
    stz a:PlayerStartPose
StageSubModeOpening3CD_at0:
    rts

; @routine 03CE:01E StageLoadRowMap3CD
; @run 6B:881E
; @unreachable 6B:881E-6B:884C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $881E runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Fills the stage's row map at $3A00 as StageLoadRowMap152D does, from the route's list in the table
; at $9D69 by zField82.
; HOW: TAI; the list's address in zTemp00-01; two loops, the run bit rotated into zTemp02.
; WAITS: no.
; Entry: nothing. Exit: A $FF, Z set, C set; X the map index reached, Y the list offset after the end. Clobbers
; zTemp00-02.
StageLoadRowMap3CD:
    tai KernelZeros,$3A00,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9D69,y
    sta z:zTemp00
    lda a:$9D6A,y
    sta z:zTemp01
    cly
StageLoadRowMap3CD_at1:
    lda (zTemp00),y
    tax
    iny
StageLoadRowMap3CD_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne StageLoadRowMap3CD_at0
    rts
StageLoadRowMap3CD_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:OBJ_HITBOX_DY+32,x
    inx
    bbs0 zTemp02,StageLoadRowMap3CD_at1
    bra StageLoadRowMap3CD_at2

; @routine 03CE:04D StageRouteSetUpA3CD
; @run 6B:884D
; @unreachable 6B:884D-6B:885A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $884D runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The kernel's pieces at $403F and at $400C (A the route's byte from the table at $9D57 by zField82),
; then the route's list unpacked into SaveBuffer (StageUnpackList3CD).
; HOW: Two calls into the kernel; a jump to StageUnpackList3CD.
; WAITS: where the kernel's pieces wait.
; Entry: nothing. Exit: as StageUnpackList3CD.
StageRouteSetUpA3CD:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9D57,x
    jsr VecStartListChannel0Kind2
    jmp StageUnpackList3CD

; @routine 03CE:05B StageRouteSetUpB3CD
; @run 6B:885B
; @unreachable 6B:885B-6B:8869 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $885B runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The kernel's piece at $4015 with X 7 and A the route's byte from the table at $9D5A by zField82 twice
; plus the character.
; HOW: A shift and an add; a table read; a jump into the kernel.
; WAITS: where the kernel's piece waits.
; Entry: nothing. Exit: as $4015 leaves them.
StageRouteSetUpB3CD:
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9D5A,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 03CE:078 StageResetScroll3CD
; @run 6B:8878
; @unreachable 6B:8878-6B:88D1 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8878 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sets the stage's screen up: the scroll's page and X kept in zBiosAl and zBiosAh, the kernel's piece at
; $4009, the scroll cleared, the byte at $2ED8 cleared and the screen loaded (zField8E kept); the overlay at $A000's
; piece at $A74E under bank 7; for a raster effect the table at $886A marks, the screen is loaded again and $A79D runs;
; then the scroll cleared and it runs on into StageLoadScreen3CD.
; HOW: Stores; $4009; TAI; StageLoadScreen3CD by BSR; MapOverlayBank7, $A74E; a table read by zRasterEffect.
; WAITS: where the kernel's piece waits.
; Entry: nothing. Exit: as StageLoadScreen3CD.
StageResetScroll3CD:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    stz a:OBJ_VY_LO+80
    lda z:zField8E
    pha
    bsr StageLoadScreen3CD
    pla
    sta z:zField8E
    jsr MapOverlayBank7At8044
    jsr SetBlinkFlash
    ldy z:zRasterEffect
    lda a:$886A,y
    beq StageResetScroll3CD_at0
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088
    lda z:zField8E
    pha
    bsr StageLoadScreen3CD
    pla
    sta z:zField8E
    jsr $A79D
StageResetScroll3CD_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,ScrollColumnBlocks,$0088

; @routine 03CE:0D2 StageLoadScreen3CD
; @run 6B:88D2
; @unreachable 6B:88D2-6B:8999 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $88D2 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Loads the stage's screen from its record (the table at $9D91 by zField83): the scroll's screen pages, X
; and Y, the camera mode, the band's byte at $3E66 and the scroll's limits; unless zField8E's bit 1 keeps the player
; (it is then cleared): a first byte of 0 restores the scroll's page and X kept in zBiosAl and zBiosAh, else it is the
; player's X; then the player's Y at $2C50, the bytes at $2C08 and $2B30 cleared, and for a character other than 0 in
; the states 5 to 8 at $2818 the player's X moved by the table at $899A; then zRasterEffect (zRasterArg cleared) and,
; unless negative, the byte at $2AA0.
; HOW: The record's address in zTemp00-01; a byte at a time through it, each stored where it goes.
; WAITS: no.
; Entry: nothing. Exit: A the record's last byte read, N and Z from it, Y past it. Clobbers zTemp00-01.
StageLoadScreen3CD:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:$9D91,y
    sta z:zTemp00
    lda a:$9D92,y
    sta z:zTemp01
    cly
    lda (zTemp00),y
    iny
    sta a:ScrollXHiScreen
    sta a:ScrollXHiScreen+1
    sta a:ScrollXHiScreen+2
    sta a:ScrollXHiScreen+3
    and #$01
    sta a:ScrollXHi
    sta a:ScrollXHi+1
    sta a:ScrollXHi+2
    sta a:ScrollXHi+3
    lda (zTemp00),y
    iny
    sta a:ScrollYHiScreen
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
    sta a:CameraMode
    lda (zTemp00),y
    iny
    sta a:ScrollColumnBlocks
    lda (zTemp00),y
    iny
    sta a:ScrollXMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollXMaxHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMinHi
    lda (zTemp00),y
    iny
    sta a:ScrollYMaxHi
    bbr1 zField8E,StageLoadScreen3CD_at5
    rmb1 z:zField8E
    iny
    iny
    bra StageLoadScreen3CD_at0
StageLoadScreen3CD_at5:
    lda (zTemp00),y
    bne StageLoadScreen3CD_at1
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra StageLoadScreen3CD_at2
StageLoadScreen3CD_at1:
    iny
    sta a:OBJ_X_HI+8
StageLoadScreen3CD_at2:
    lda (zTemp00),y
    iny
    sta a:OBJ_X_FRAC+48
    stz a:OBJ_X_LO+64
    stz a:OBJ_SPRITE_ATTR+24
    lda z:zCharacter
    beq StageLoadScreen3CD_at0
    lda a:OBJ_SUBSTATE+24
    cmp #$05
    bcc StageLoadScreen3CD_at0
    sec
    sbc #$05
    cmp #$04
    bcs StageLoadScreen3CD_at0
    tax
    lda a:OBJ_X_HI+8
    clc
    adc a:$899A,x
    sta a:OBJ_X_HI+8
StageLoadScreen3CD_at0:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi StageLoadScreen3CD_at3
    iny
    sta a:OBJ_FRAME+56
    bra StageLoadScreen3CD_at4
StageLoadScreen3CD_at3:
    iny
StageLoadScreen3CD_at4:
    rts

; @routine 03CE:19E StageOverlayFrame3CD
; @run 6B:899E
; @unreachable 6B:899E-6B:89B1 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $899E runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The frame of the overlays above: the kernel's piece at $4096 under bank 7, then the piece at $A000
; under banks 8 and 9 above zRamBaseBank.
; HOW: MapOverlayBank7, $4096; the add and two TAM; $A000.
; WAITS: where the pieces wait.
; Entry: nothing. Exit: as $A000 leaves them.
StageOverlayFrame3CD:
    jsr MapOverlayBank7At8044
    jsr VecStreamScrollStep
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr RunObjects
    rts

; @routine 03CE:1B2 StageLevelStart3CD
; @run 6B:89B2
; @unreachable 6B:89B2-6B:89E7 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $89B2 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Starts the stage's level: the kernel's piece at $4057, then under bank 7 the level's stream and axis
; from the four bytes at $9E21 by zField83, the 32 event flags cleared when the fourth byte is not 0, the kernel's
; piece at $4093 and the overlays' frame.
; HOW: $4057, MapOverlayBank7; table reads; a loop of stores with Y kept on the stack; $4093; StageOverlayFrame3CD.
; WAITS: where the pieces wait.
; Entry: nothing. Exit: as StageOverlayFrame3CD.
StageLevelStart3CD:
    jsr VecClearEnemySlots
    jsr MapOverlayBank7At8044
    lda z:zField83
    asl a
    asl a
    tay
    lda a:$9E21,y
    sta a:LevelStreamLo
    lda a:$9E22,y
    sta a:LevelStreamHi
    lda a:$9E23,y
    sta a:LevelAxis
    lda a:$9E24,y
    beq StageLevelStart3CD_at0
    phy
    cly
StageLevelStart3CD_at1:
    lda #$00
    sta a:EventFlags,y
    iny
    cpy #$20
    bcc StageLevelStart3CD_at1
    ply
StageLevelStart3CD_at0:
    jsr VecStreamFullWalk
    jsr StageOverlayFrame3CD
    rts

; @routine 03CE:1E8 DoorHandler3CD
; @run 6B:89E8
; @unreachable 6B:89E8-6B:8A36 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $89E8 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: An object handler for a door the pad's override opens, as DoorHandler152D with the kernel's piece at
; $40D2 where that has $A36C, and zField8C set to 1 when the door is taken.
; HOW: Loads, shifts through the carry and stores; the tests and the stores of the door.
; WAITS: no.
; Entry: X the object. Exit: A the last value loaded or masked, N and Z from it; X kept.
DoorHandler3CD:
    lda a:OBJ_SUBSTATE+24,x
    bne DoorHandler3CD_at0
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_A+64,x
    tay
    and #$0F
    sta a:OBJ_WORK_B+16,x
    tya
    rol a
    rol a
    rol a
    tay
    and #$03
    sta a:OBJ_WORK_C,x
    tya
    rol a
    rol a
    and #$03
    sta a:OBJ_HEALTH+32,x
    rts
DoorHandler3CD_at0:
    stz a:OBJ_WORK_L+24,x
    ldy a:OBJ_HEALTH+32,x
    lda a:PadOverrides,y
    beq DoorHandler3CD_at1
    lda a:OBJ_HIT_STATUS+80,x
    beq DoorHandler3CD_at1
    cmp #$05
    bcs DoorHandler3CD_at1
    lda a:OBJ_WORK_B+16,x
    sta z:zField83
    lda a:OBJ_WORK_C,x
    sta z:zField87
    lda #$01
    sta z:zField8C
    lda #$02
    sta z:zSubStep
    rts
DoorHandler3CD_at1:
    stz a:OBJ_HIT_STATUS+80,x
    rts

; @routine 03CE:237 DoorHandlerB3CD
; @run 6B:8A37
; @unreachable 6B:8A37-6B:8A84 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8A37 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: An object handler for a door: not yet placed, $40D2 and the packed byte unpacked (its room in five
; bits). Placed: the byte at $3790 cleared; a door whose packed byte has bit 7 needs PlayerDoorReady; with the byte at
; $3BD3 other than 1 and the object's state at $31F0 from 1 to 4 the stage goes through it (zField83 the room,
; zField87 the entry, zField8C 0, zSubStep 2, the player's Y bytes at $2CE0 and $2D28 cleared); else the state is
; cleared (at $3BD3 1 nothing is done).
; HOW: Loads, shifts and stores; the tests and the stores of the door.
; WAITS: no.
; Entry: X the object. Exit: A the last value loaded or computed, N and Z from it; X kept.
DoorHandlerB3CD:
    lda a:OBJ_SUBSTATE+24,x
    bne DoorHandlerB3CD_at1
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_A+64,x
    tay
    and #$1F
    sta a:OBJ_WORK_B+16,x
    tya
    rol a
    rol a
    rol a
    and #$03
    sta a:OBJ_WORK_C,x
DoorHandlerB3CD_at0:
    rts
DoorHandlerB3CD_at1:
    stz a:OBJ_WORK_L+24,x
    lda a:OBJ_WORK_A+64,x
    bpl DoorHandlerB3CD_at2
    lda a:PlayerDoorReady
    beq DoorHandler3CD_at1
DoorHandlerB3CD_at2:
    lda a:PlayerMayLeaveTop
    dec a
    beq DoorHandlerB3CD_at0
    lda a:OBJ_HIT_STATUS+80,x
    beq DoorHandler3CD_at1
    cmp #$05
    bcs DoorHandler3CD_at1
    lda a:OBJ_WORK_B+16,x
    sta z:zField83
    lda a:OBJ_WORK_C,x
    sta z:zField87
    stz z:zField8C
    lda #$02
    sta z:zSubStep
    stz a:OBJ_Y_LO+16
    stz a:OBJ_Y_FRAC
    rts

; @routine 03CE:285 StageHudStart3CD
; @run 6B:8A85
; @unreachable 6B:8A85-6B:8A94 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8A85 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Draws the stage's display at its start: the pieces 0 and 1 (StageDrawPiece3CD), the stock and the
; points to add; zField8E's bit 5 cleared; then runs on into StageHudStep3CD.
; HOW: Calls with X 0 and 1; a bit cleared.
; WAITS: no.
; Entry: nothing. Exit: as StageHudStep3CD.
StageHudStart3CD:
    clx
    jsr StageDrawPiece3CD
    inx
    jsr StageDrawPiece3CD
    jsr StageDrawStock3CD
    jsr StageDrawScoreAdd3CD
    rmb5 z:zField8E

; @routine 03CE:295 StageHudStep3CD
; @run 6B:8A95
; @stop 8B18
; @unreachable 6B:8A95-6B:8B17 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8A95 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The stage's display each frame, as StageHudStep152D: the item's glow; the item's frame at $2A9E from
; the table at $8B18 by zField93 (for the characters, frames $72 for item $0D and $71 for $11, with $2C96 $28 instead
; of $24); the health bar a step toward zPlayerHealth (sounds $0E and $40); the hearts' digits; the boss's bar while
; zField8E's bit 2 is set.
; HOW: StageHudSound3CD; compares; StageHealthBarAddress3CD by BSR with AndVramData or OrVramData (census duplicates
; at $9425 and $9434, through map_draw.c's C); the sound hook; StageDrawDigit3CD; a jump to StageBossBar3CD.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them.
StageHudStep3CD:
    jsr StageHudSound3CD
    lda #$24
    sta a:OBJ_Y_HI+30
    ldx z:zField93
    lda a:$8B18,x
    ldx z:zCharacter
    beq StageHudStep3CD_at0
    cmp #$0D
    beq StageHudStep3CD_at1
    cmp #$11
    bne StageHudStep3CD_at0
    ldx #$28
    stx a:OBJ_Y_HI+30
    lda #$71
    bra StageHudStep3CD_at0
StageHudStep3CD_at1:
    lda #$72
StageHudStep3CD_at0:
    sta a:OBJ_FRAME+54
    lda z:zPlayerHealth
    cmp z:zField99
    beq StageHudStep3CD_at2
    bcs StageHudStep3CD_at8
    bsr StageHealthBarAddress3CD
    ldx #$FF
    ldy #$F0
    jsr AndVramDataAt9425
    dec z:zField99
    bra StageHudStep3CD_at3
StageHudStep3CD_at8:
    bbr5 zField8E,StageHudStep3CD_at4
    bbs6 zField8E,StageHudStep3CD_at4
    lda #$0E
    jsr VecSoundCommand
    smb6 z:zField8E
StageHudStep3CD_at4:
    inc z:zField99
    bsr StageHealthBarAddress3CD
    ldx #$00
    ldy #$0F
    jsr OrVramDataAt9434
    bra StageHudStep3CD_at5
StageHudStep3CD_at2:
    smb5 z:zField8E
StageHudStep3CD_at3:
    bbr6 zField8E,StageHudStep3CD_at5
    rmb6 z:zField8E
    lda #$40
    jsr VecSoundCommand
StageHudStep3CD_at5:
    lda z:zField94
    cmp z:zField95
    beq StageHudStep3CD_at6
    sta z:zField95
    and #$0F
    ldx #$08
    jsr StageDrawDigit3CD
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    ldx #$09
    jsr StageDrawDigit3CD
StageHudStep3CD_at6:
    bbs2 zField8E,StageHudStep3CD_at7
    rts
StageHudStep3CD_at7:
    jmp StageBossBar3CD

; @routine 03CE:321 StageHudSound3CD
; @run 6B:8B21
; @unreachable 6B:8B21-6B:8B30 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8B21 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The item's glow: the kernel's piece at $4018 with X 1 and A the colour of the step the wave counter's
; low three bits (masked by RichterItemReady) pick from the eight at $8B31.
; HOW: Two ANDs and a table read; a jump into the kernel.
; WAITS: no.
; Entry: nothing. Exit: as $4018 leaves them.
StageHudSound3CD:
    lda z:zWaveCounter
    and #$07
    and a:RichterItemReady
    tax
    lda a:$8B31,x
    ldx #$01
    jmp VecStartScriptSlotKind4

; @routine 03CE:339 StageHealthBarAddress3CD
; @run 6B:8B39
; @unreachable 6B:8B39-6B:8B61 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8B39 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Points the VDC at the health bar's piece for the shown health zField99, as StageHealthBarAddress152D.
; HOW: A subtract, nibbles and shifts into zTemp00-01, an add; SetVramAddresses3CD with the address in X and Y.
; WAITS: no.
; Entry: nothing. Exit, zField99 0: A 0, N and Z from it. Else: as SetVramAddresses3CD. Clobbers zTemp00-01.
StageHealthBarAddress3CD:
    lda z:zField99
    beq StageHealthBarAddress3CD_at0
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
    jsr SetVramAddresses3CD
StageHealthBarAddress3CD_at0:
    rts

; @routine 03CE:362 StageBossBarReset3CD
; @run 6B:8B62
; @unreachable 6B:8B62-6B:8B69 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8B62 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The boss's bar drawn anew (StageDrawPiece3CD 2) and its length at $2745 cleared; then runs on into
; StageBossBar3CD. Reached from StageBossBar3CD while the boss's frame at $2A9D is 0.
; HOW: A call with X 2; a store.
; WAITS: no.
; Entry: nothing. Exit: as StageBossBar3CD.
StageBossBarReset3CD:
    ldx #$02
    jsr StageDrawPiece3CD
    stz a:BossBarLength

; @routine 03CE:36A StageBossBar3CD
; @run 6B:8B6A
; @unreachable 6B:8B6A-6B:8B78 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8B6A runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The boss's bar each frame, as StageBossBar152D.
; HOW: Tests; a decrement; a branch or run into StageBossBarStep3CD.
; WAITS: no.
; Entry: nothing. Exit: as StageBossBarStep3CD.
StageBossBar3CD:
    lda a:OBJ_FRAME+53
    beq StageBossBarReset3CD
    lda a:OBJ_X_HI+77
    cmp #$D8
    beq StageBossBarStep3CD
    dec a:OBJ_X_HI+77

; @routine 03CE:379 StageBossBarStep3CD
; @run 6B:8B79
; @unreachable 6B:8B79-6B:8BB9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8B79 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Moves the boss's bar at $2745 one step toward the boss's health at $3128, as StageBossBarStep152D; the
; flash (StageBossBarFlash3CD) while the count at $330D is not 0.
; HOW: Compares; StageBossBarAddress3CD by BSR with AndVramData or OrVramData; the health on the stack across the call
; of itself.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them.
StageBossBarStep3CD:
    lda a:OBJ_WORK_B+13
    beq StageBossBarStep3CD_at0
    jmp StageBossBarFlash3CD
StageBossBarStep3CD_at0:
    lda a:OBJ_LOOP_FLAGS+56
    bmi StageBossBarStep3CD_at1
    cmp a:BossBarLength
    beq StageBossBarStep3CD_at2
    bcs StageBossBarStep3CD_at3
    bsr StageBossBarAddress3CD
    ldx #$0F
    ldy #$FF
    jsr AndVramDataAt9425
    dec a:BossBarLength
    rts
StageBossBarStep3CD_at1:
    pha
    stz a:OBJ_LOOP_FLAGS+56
    bsr StageBossBarStep3CD
    pla
    sta a:OBJ_LOOP_FLAGS+56
    ldy a:BossBarLength
    bne StageBossBarStep3CD_at2
    sta a:OBJ_WORK_B+13
    rts
StageBossBarStep3CD_at3:
    inc a:BossBarLength
    bsr StageBossBarAddress3CD
    ldx #$F0
    ldy #$00
    jsr OrVramDataAt9434
StageBossBarStep3CD_at2:
    rts

; @routine 03CE:3BA StageBossBarAddress3CD
; @run 6B:8BBA
; @unreachable 6B:8BBA-6B:8C02 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8BBA runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Points the VDC at the boss's bar's piece for the length at $2745, as StageBossBarAddress152D, from
; $7D40.
; HOW: The page in zTemp08-09; a subtract, nibbles and shifts into zTemp00-01, adds; SetVramAddresses3CD.
; WAITS: no.
; Entry: nothing. Exit, length 0: A 0, N and Z from it. Else: as SetVramAddresses3CD. Clobbers zTemp00-01, zTemp08-09.
StageBossBarAddress3CD:
    lda a:BossBarLength
    beq StageBossBarStep3CD_at2
    stz z:zTemp08
    stz z:zTemp09
    cmp #$3F
    bcc StageBossBarAddress3CD_at0
    lda #$C0
    sta z:zTemp08
    inc z:zTemp09
StageBossBarAddress3CD_at0:
    lda #$5E
    sec
    sbc a:BossBarLength
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
    adc #$7D
    sta z:zTemp01
    lda z:zTemp08
    clc
    adc z:zTemp00
    tax
    lda z:zTemp09
    adc z:zTemp01
    tay
    jsr SetVramAddresses3CD
StageBossBarAddress3CD_at1:
    rts

; @routine 03CE:41B StageBossBarFlash3CD
; @run 6B:8C1B
; @unreachable 6B:8C1B-6B:8C59 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8C1B runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The boss's bar at its end, as StageBossBarFlash152D (the addresses at $8C03, StageBarErase3CD); the
; speed's high byte at $2DB5 counted only when its low byte carries.
; HOW: A loop of four in zTemp01; an add and an increment; the kernel; stores.
; WAITS: no.
; Entry: nothing. Exit: A the boss's page, N and Z from it, the rest as $4045 left them.
StageBossBarFlash3CD:
    lda z:zWaveCounter
    and #$01
    sta z:zTemp01
StageBossBarFlash3CD_at0:
    lda z:zTemp01
    asl a
    tax
    ldy a:$8C04,x
    lda a:$8C03,x
    tax
    jsr StageBarErase3CD
    inc z:zTemp01
    inc z:zTemp01
    lda z:zTemp01
    cmp #$08
    bcc StageBossBarFlash3CD_at0
    inc a:OBJ_WORK_C+69
    lda a:OBJ_VX_LO+37
    clc
    adc #$0C
    sta a:OBJ_VX_LO+37
    bcc StageBossBarFlash3CD_at1
    inc a:OBJ_VX_HI+53
StageBossBarFlash3CD_at1:
    ldx #$45
    jsr VecApplyVelocity
    lda a:OBJ_X_FRAC+45
    beq StageBossBarAddress3CD_at1
    stz a:OBJ_FRAME+53
    rmb2 z:zField8E
    rts

; @routine 03CE:45A StageBarErase3CD
; @run 6B:8C5A
; @unreachable 6B:8C5A-6B:8C79 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8C5A runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Blanks a bar's piece in VRAM at the address in X and Y, in 64 words: each word ANDed (AndVramData,
; through its C) with a mask of the kernel's random piece at $4051, zRandom2, zRandom1 and the flash step's row of
; the table at $8C13, X kept on the stack.
; HOW: SetVramAddresses3CD; a loop down X.
; WAITS: no.
; Entry: X, Y the VRAM address. Exit: X 0, N and Z from it; A the word's high byte, Y the mask.
StageBarErase3CD:
    jsr SetVramAddresses3CD
    ldx #$40
StageBarErase3CD_at0:
    lda a:OBJ_WORK_C+69
    and #$07
    tay
    jsr VecNextRandom
    ora z:zRandom2
    ora z:zRandom1
    ora a:$8C13,y
    phx
    tax
    tay
    jsr AndVramDataAt9425
    plx
    dex
    bne StageBarErase3CD_at0
    rts

; @routine 03CE:47A StageDrawDigit3CD
; @run 6B:8C7A
; @unreachable 6B:8C7A-6B:8CF9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8C7A runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Draws the digit in A into the display at the place in X, as StageDrawDigit152D, from the glyphs at
; $8D15 up (StageDigitAddress3CD).
; HOW: The glyph's address in zTemp00-01; StageDigitAddress3CD by BSR; loops of five reads and writes of the VDC's data
; port with shifts through zTemp02-03.
; WAITS: no.
; Entry: A the digit, X the place. Exit: Y 5, Z set, C set. Clobbers zTemp00-04.
StageDrawDigit3CD:
    stx z:zTemp04
    sta z:zTemp00
    asl a
    asl a
    adc z:zTemp00
    adc #$15
    sta z:zTemp00
    cla
    adc #$8D
    sta z:zTemp01
    bsr StageDigitAddress3CD
    lda z:zTemp04
    lsr a
    bcs StageDrawDigit3CD_at0
    cly
StageDrawDigit3CD_at1:
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
    bne StageDrawDigit3CD_at1
    rts
StageDrawDigit3CD_at0:
    cly
StageDrawDigit3CD_at2:
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
    bne StageDrawDigit3CD_at2
    inc z:zTemp04
    bsr StageDigitAddress3CD
    cly
StageDrawDigit3CD_at3:
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
    bne StageDrawDigit3CD_at3
    rts

; @routine 03CE:4FA StageDigitAddress3CD
; @run 6B:8CFA
; @unreachable 6B:8CFA-6B:8D08 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8CFA runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Points the VDC at the display's place zTemp04 (its even pair) from the table of addresses at $8D09.
; HOW: Two table reads; a jump to SetVramAddresses3CD.
; WAITS: no.
; Entry: zTemp04 the place. Exit: as SetVramAddresses3CD.
StageDigitAddress3CD:
    lda z:zTemp04
    and #$FE
    tax
    lda a:$8D09,x
    ldy a:$8D0A,x
    tax
    jmp SetVramAddresses3CD

; @routine 03CE:55D StagePausedSelect3CD
; @run 6B:8D5D
; @unreachable 6B:8D5D-6B:8D8D no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8D5D runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The pause screen's choice, reached from StagePausedFrame3CD: Select with Run (bits 7 and 2 of
; BiosJoyTrigger together) draws the map's debug marks (StageMapDebug3CD); else as StagePausedSelect152D, the choices
; from the table at $8D51 and the frames from $8B18.
; HOW: Pad bits; table reads; stores; a jump to StageMapDebug3CD.
; WAITS: no.
; Entry: nothing. Exit, no press: A 0, N and Z from it. Else: A $99, N and Z from it, X the new choice.
StagePausedSelect3CD:
    lda a:BiosJoyTrigger
    and #$84
    cmp #$84
    bne StagePausedSelect3CD_at0
    jmp StageMapDebug3CD
StagePausedSelect3CD_at0:
    lda a:BiosJoyTrigger
    and #$50
    beq StagePausedFrame3CD_at4
    ldx z:zField93
    and #$40
    beq StagePausedSelect3CD_at1
    inx
    inx
StagePausedSelect3CD_at1:
    lda a:SoundRatePresets,x
    sta z:zField93
    tax
    lda a:$8B18,x
    sta a:OBJ_FRAME+54
    lda #$01
    sta a:PlayerItemFirstUse
    lda #$99
    sta z:zField94
    rts

; @routine 03CE:58E StagePausedFrame3CD
; @run 6B:8D8E
; @unreachable 6B:8D8E-6B:8E0C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8D8E runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The paused frame. zPaused's bit 1 clear: the choice (StagePausedSelect3CD). Else zPaused counts up by
; 4 a frame (its bits 2 up the menu's step, bit 0 the side it scrolls from) through the menu's rows (the table of four
; bytes a row at $8E0D, from $19 down or up to $1A); past them only bit 0 is kept. Steps $25 and $27 at the end of the
; scroll redraw the score, or the stock and the points to add.
; HOW: Shifts of zPaused through zTemp00; a subtract; the row's step through the kernel (pause_menu_row); jumps to
; StageDrawScore3CD or StageDrawScoreAdd3CD, a call of StageDrawStock3CD.
; WAITS: no.
; Entry: nothing. Exit: as the step that ran left them.
StagePausedFrame3CD:
    lda z:zPaused
    lsr a
    ror z:zTemp00
    lsr a
    bcc StagePausedSelect3CD
    rol z:zTemp00
    bcc StagePausedFrame3CD_at0
    cmp #$1A
    bcc StagePausedFrame3CD_at1
StagePausedFrame3CD_at2:
    lda z:zPaused
    and #$01
    sta z:zPaused
StagePausedFrame3CD_at4:
    rts
StagePausedFrame3CD_at0:
    sta z:zTemp00
    lda #$19
    sec
    sbc z:zTemp00
    bcc StagePausedFrame3CD_at2
StagePausedFrame3CD_at1:
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
    bcs StagePausedFrame3CD_at3
    stx z:zTemp00
    lda a:$8E0D,x
    ldx #$02
    jsr VecStartScriptSlotKind4
    ldx z:zTemp00
    lda a:$8E0E,x
    ldx #$03
    jsr VecStartScriptSlotKind4
    ldx z:zTemp00
    lda a:$8E10,x
    sta a:OBJ_FRAME+55
    lda z:zPaused
    sta z:zTemp01
    ldx z:zTemp00
    lda a:$8E0F,x
    beq StagePausedFrame3CD_at4
    lsr z:zTemp01
    bcc StagePausedFrame3CD_at5
    cmp #$65
    beq StagePausedFrame3CD_at4
    bne StagePausedFrame3CD_at6
StagePausedFrame3CD_at5:
    cmp #$64
    beq StagePausedFrame3CD_at4
StagePausedFrame3CD_at6:
    ldx #$04
    jmp VecStartListChannelKind2
StagePausedFrame3CD_at3:
    lda z:zPaused
    lsr a
    bcc StagePausedFrame3CD_at7
    cmp #$25
    bne StagePausedFrame3CD_at4
    jmp StageDrawScore3CD
StagePausedFrame3CD_at7:
    cmp #$27
    bne StagePausedFrame3CD_at4
    jsr StageDrawStock3CD
    jmp StageDrawScoreAdd3CD

; @routine 03CE:641 StageOpeningStart3CD
; @run 6B:8E41
; @unreachable 6B:8E41-6B:8E6C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8E41 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Starts the stage's opening when zField8E's bit 0 is clear, as StageOpeningStart152D without its sound:
; the display's pieces 3 and 4, the opening's objects placed, TimeSlowMode 2, the wait at $33E4 cleared.
; HOW: A bit test; calls with X 3 and 4; stores.
; WAITS: no.
; Entry: nothing. Exit, bit 0 set: everything kept. Else: A 2, N and Z from it, X 4, Y and the flags as
; StageDrawPiece3CD left them.
StageOpeningStart3CD:
    bbr0 zField8E,StageOpeningStart3CD_at0
    rts
StageOpeningStart3CD_at0:
    ldx #$03
    jsr StageDrawPiece3CD
    inx
    jsr StageDrawPiece3CD
    dec a:OBJ_X_HI+4
    dec a:OBJ_X_FRAC+45
    lda #$04
    sta a:OBJ_Y_LO+84
    sta a:OBJ_VX_HI+53
    lda #$20
    sta a:OBJ_WORK_B+84
    sta a:OBJ_WORK_B+85
    lda #$02
    sta a:TimeSlowMode
    stz a:OBJ_HIT_TAKEN+52
    rts

; @routine 03CE:66D StageOpeningStep3CD
; @run 6B:8E6D
; @stop 8EEB
; @unreachable 6B:8E6D-6B:8EEA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8E6D runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: One frame of the stage's opening: the kernel's piece at $407B once (for a stage other than 0), the two
; objects stepped as in StageOpeningStep152D (their steps from $8EEA and $8EF0), but moved by the kernel's pieces at
; $4063 (object $45) and $4060 (object $44); at the first's step 7 the opening ends; at the second's step 6 the
; screen's row fade runs (StageOpeningFade3CD).
; HOW: Decrements and table reads; the kernel; a branch to StageOpeningFade3CD.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them.
StageOpeningStep3CD:
    lda a:OBJ_HIT_TAKEN+52
    bne StageOpeningStep3CD_at0
    lda z:zStage
    beq StageOpeningStep3CD_at1
    jsr VecCdPlayAgain
StageOpeningStep3CD_at1:
    inc a:OBJ_HIT_TAKEN+52
StageOpeningStep3CD_at0:
    dec a:OBJ_WORK_B+85
    bne StageOpeningStep3CD_at2
    inc a:OBJ_WORK_B+13
    ldy a:OBJ_WORK_B+13
    cpy #$07
    bne StageOpeningStep3CD_at3
    stz a:OBJ_FRAME+53
    smb0 z:zField8E
    rts
StageOpeningStep3CD_at3:
    lda a:$8EEA,y
    lsr a
    sta a:OBJ_VX_HI+53
    cla
    ror a
    sta a:OBJ_VX_LO+37
    lda a:$8EF0,y
    sta a:OBJ_WORK_B+85
StageOpeningStep3CD_at2:
    lda a:OBJ_WORK_B+13
    cmp #$06
    bne StageOpeningStep3CD_at4
    lda a:OBJ_WORK_B+85
    cmp #$1F
    bne StageOpeningStep3CD_at5
    stz a:OBJ_FRAME+52
StageOpeningStep3CD_at5:
    sta a:OBJ_WORK_C+68
StageOpeningStep3CD_at4:
    ldx #$45
    jsr VecApplyVelocityY
    dec a:OBJ_WORK_B+84
    bne StageOpeningStep3CD_at6
    inc a:OBJ_WORK_B+12
    ldy a:OBJ_WORK_B+12
    lda a:$8EEA,y
    lsr a
    sta a:OBJ_Y_LO+84
    cla
    ror a
    sta a:OBJ_Y_FRAC+68
    lda a:$8EF0,y
    sta a:OBJ_WORK_B+84
StageOpeningStep3CD_at6:
    ldx #$44
    jsr VecApplyVelocityX
    lda a:OBJ_WORK_B+12
    cmp #$06
    beq StageOpeningStep3CD_at7
    rts
StageOpeningStep3CD_at7:
    lda a:OBJ_WORK_C+68
    bra StageOpeningFade3CD

; @routine 03CE:707 StageOpeningFade3CD
; @run 6B:8F07
; @unreachable 6B:8F07-6B:8F51 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8F07 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The opening's fade of the screen's rows with A the count, as StageOpeningFade152D: the level and the
; group of rows from the table at $8EF7, each row faded by StageOpeningCopyRow3CD after the kernel's piece at $4051;
; the row's address moves on with its high byte counted only on a carry.
; HOW: Masks, a subtract and table reads into zTemp10-12 and zProduct0; a loop of rows.
; WAITS: no.
; Entry: A the count. Exit: N and Z from the row count's decrement (Z set); C and V from the address's add.
StageOpeningFade3CD:
    sta z:SpritesLeft
    cmp #$20
    bcc StageOpeningFade3CD_at0
    and #$1F
    lsr a
    bcc StageOpeningFade3CD_at1
    clc
    adc #$10
StageOpeningFade3CD_at1:
    adc #$20
StageOpeningFade3CD_at0:
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
    lda a:$8EF7,x
    sta z:zTemp10
    lda a:$8EF8,x
    sta z:zTemp11
    lda a:$8EF9,x
    sta z:zTemp12
    lda a:$8EFA,x
    sta z:zProduct0
StageOpeningFade3CD_at3:
    jsr VecNextRandom
    bsr StageOpeningCopyRow3CD
    lda z:zTemp10
    clc
    adc z:zProduct0
    sta z:zTemp10
    bcc StageOpeningFade3CD_at2
    inc z:zTemp11
StageOpeningFade3CD_at2:
    dec z:zTemp12
    bne StageOpeningFade3CD_at3
    rts

; @routine 03CE:752 StageOpeningCopyRow3CD
; @run 6B:8F52
; @stop 8FA1
; @unreachable 6B:8F52-6B:8FA0 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8F52 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Fades one row of the screen in VRAM, as StageOpeningCopyRow152D, the passes over the buffer at $3980 by
; StageFadeRowNext3CD and StageFadeRowSelf3CD; then runs on into the write back (StageOpeningWriteRow152D, whose bytes
; are at $8FA1).
; HOW: SetVramAddresses3CD; loops of the VDC's data port and of the buffer.
; WAITS: no.
; Entry: zTemp10-11 the row's address, zMultiplierHi the level. Exit: as StageOpeningWriteRow152D. Clobbers zTemp00.
StageOpeningCopyRow3CD:
    ldx z:zTemp10
    ldy z:zTemp11
    jsr SetVramAddresses3CD
    ldx #$80
StageOpeningCopyRow3CD_at5:
    lda a:VDC_DATA_LO
    sta a:OBJ_HITBOX_DX+79,x
    lda a:VDC_DATA_HI
    sta a:OBJ_HITBOX_DX+78,x
    dex
    dex
    bne StageOpeningCopyRow3CD_at5
    ldy z:SpritesLeft
    cpy #$10
    bcc StageOpeningCopyRow3CD_at0
    ldx #$7F
StageOpeningCopyRow3CD_at1:
    stz a:OBJ_HITBOX_DX+80,x
    dex
    bpl StageOpeningCopyRow3CD_at1
    jmp StageOpeningWriteRow152DAt8FA1
StageOpeningCopyRow3CD_at0:
    dey
    beq StageOpeningCopyRow3CD_at2
    clx
StageOpeningCopyRow3CD_at3:
    lda z:zRandom2
    sec
    ror a
    ora z:zRandom2
    sta z:zTemp00
    jsr StageFadeRowNext3CD
    inx
    inx
    dey
    bne StageOpeningCopyRow3CD_at3
StageOpeningCopyRow3CD_at2:
    ldy z:SpritesLeft
    clx
StageOpeningCopyRow3CD_at4:
    lda z:zRandom2
    ora z:zRandom3
    sta z:zTemp00
    jsr StageFadeRowSelf3CD
    inx
    inx
    dey
    bne StageOpeningCopyRow3CD_at4

; @routine 03CE:7B4 StageFadeRowNext3CD
; @run 6B:8FB4
; @unreachable 6B:8FB4-6B:8FCE no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8FB4 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: One word of the row's fade: its eight bytes (the four planes' pairs, offsets from $8FEA) at $3980 + X
; each the next word's byte ANDed with the mask in zTemp00.
; HOW: X in zTemp08; X and Y kept on the stack; a loop down X.
; WAITS: no.
; Entry: X the word's offset, zTemp00 the mask. Exit: X and Y kept, N and Z from X; A the last byte written; C and V
; from the last add.
StageFadeRowNext3CD:
    stx z:zTemp08
    phx
    phy
    ldx #$07
StageFadeRowNext3CD_at0:
    lda a:$8FEA,x
    clc
    adc z:zTemp08
    tay
    lda a:OBJ_HITBOX_DX+82,y
    and z:zTemp00
    sta a:OBJ_HITBOX_DX+80,y
    dex
    bpl StageFadeRowNext3CD_at0
    ply
    plx
    rts

; @routine 03CE:7CF StageFadeRowSelf3CD
; @run 6B:8FCF
; @unreachable 6B:8FCF-6B:8FE9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8FCF runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: One word of the row's fade: its eight bytes ANDed in place with the mask in zTemp00.
; HOW: X in zTemp08; X and Y kept on the stack; a loop down X.
; WAITS: no.
; Entry: X the word's offset, zTemp00 the mask. Exit: as StageFadeRowNext3CD.
StageFadeRowSelf3CD:
    stx z:zTemp08
    phx
    phy
    ldx #$07
StageFadeRowSelf3CD_at0:
    lda a:$8FEA,x
    clc
    adc z:zTemp08
    tay
    lda a:OBJ_HITBOX_DX+80,y
    and z:zTemp00
    sta a:OBJ_HITBOX_DX+80,y
    dex
    bpl StageFadeRowSelf3CD_at0
    ply
    plx
    rts

; @routine 03CE:7F2 StageDrawStock3CD
; @run 6B:8FF2
; @unreachable 6B:8FF2-6B:8FFA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8FF2 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Draws the stock zStock (two digits at places 0 and 1).
; HOW: StageDrawNumber3CD with X 0, X kept on the stack across it.
; WAITS: no.
; Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber3CD left them.
StageDrawStock3CD:
    phx
    lda z:zStock
    clx
    jsr StageDrawNumber3CD
    plx
    rts

; @routine 03CE:7FB StageDrawScoreAdd3CD
; @run 6B:8FFB
; @unreachable 6B:8FFB-6B:900B no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $8FFB runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Draws the points to add, zScoreAddLo and zScoreAddHi (four digits from place 3).
; HOW: StageDrawNumber3CD with X 3 and 5, X kept on the stack across them.
; WAITS: no.
; Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber3CD left them.
StageDrawScoreAdd3CD:
    phx
    lda z:zScoreAddLo
    ldx #$03
    jsr StageDrawNumber3CD
    lda z:zScoreAddHi
    ldx #$05
    jsr StageDrawNumber3CD
    plx
    rts

; @routine 03CF:00C StageDrawScore3CD
; @run 6B:900C
; @unreachable 6B:900C-6B:9023 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $900C runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Draws the score zScore0-2 (six digits from place 1).
; HOW: StageDrawNumber3CD with X 1, 3 and 5, X kept on the stack across them.
; WAITS: no.
; Entry: nothing. Exit: X kept, N and Z from it; A, Y, C and V as StageDrawNumber3CD left them.
StageDrawScore3CD:
    phx
    lda z:zScore0
    ldx #$01
    jsr StageDrawNumber3CD
    lda z:zScore1
    ldx #$03
    jsr StageDrawNumber3CD
    lda z:zScore2
    ldx #$05
    jsr StageDrawNumber3CD
    plx
    rts

; @routine 03CF:024 StageDrawNumber3CD
; @run 6B:9024
; @unreachable 6B:9024-6B:9034 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9024 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Draws the two decimal digits of A at the places X and X + 1, the low digit first.
; HOW: StageDrawDigit3CD with the low nibble, X and A kept on the stack; then a jump to it with the high nibble.
; WAITS: no.
; Entry: A the number, X the place. Exit: as StageDrawDigit3CD.
StageDrawNumber3CD:
    phx
    pha
    and #$0F
    jsr StageDrawDigit3CD
    pla
    lsr a
    lsr a
    lsr a
    lsr a
    plx
    inx
    jmp StageDrawDigit3CD

; @routine 03CF:035 StageUnpackList3CD
; @run 6B:9035
; @unreachable 6B:9035-6B:9096 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9035 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Unpacks the route's list (the table at $9D63 by zField82) into memory, as StageUnpackList152D; the
; pointers' high bytes are counted only on a carry.
; HOW: TAI; the list in zTemp00-01, the target in zTemp02-03, the count in zTemp04.
; WAITS: no.
; Entry: nothing. Exit: A $FF, Z set, C set. Clobbers zTemp00-04.
StageUnpackList3CD:
    tai KernelZeros,SaveBuffer,$02FF
    lda z:zField82
    asl a
    tay
    lda a:$9D63,y
    sta z:zTemp00
    lda a:$9D64,y
    sta z:zTemp01
    cly
StageUnpackList3CD_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne StageUnpackList3CD_at0
    lda #$08
    bra StageUnpackList3CD_at1
StageUnpackList3CD_at7:
    lda (zTemp00)
    bne StageUnpackList3CD_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra StageUnpackList3CD_at1
StageUnpackList3CD_at2:
    cmp #$FF
    bne StageUnpackList3CD_at3
    rts
StageUnpackList3CD_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
StageUnpackList3CD_at4:
    sta (zTemp02),y
    iny
    dex
    bne StageUnpackList3CD_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc StageUnpackList3CD_at5
    inc z:zTemp03
StageUnpackList3CD_at5:
    lda #$02
StageUnpackList3CD_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc StageUnpackList3CD_at6
    inc z:zTemp01
StageUnpackList3CD_at6:
    bra StageUnpackList3CD_at7

; @routine 03CF:097 ClearBitRowsFour3CD
; @run 6B:9097
; @unreachable 6B:9097-6B:909C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9097 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Runs StageClearBitRows3CD four times: three calls and the run into it.
; HOW: Three BSR and the bytes of StageClearBitRows3CD after them.
; WAITS: no.
; Entry, exit: as StageClearBitRows3CD.
ClearBitRowsFour3CD:
    bsr StageClearBitRows3CD
    bsr StageClearBitRows3CD
    bsr StageClearBitRows3CD

; @routine 03CF:09D StageClearBitRows3CD
; @run 6B:909D
; @unreachable 6B:909D-6B:90E4 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $909D runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Clears one bit of four VRAM words $10 apart for the map entry zTemp0F names, as StageClearBitRows152D;
; each word pointed at by SetVramAddresses3CD (X and Y kept on the stack) and ANDed with the masks at $90E5 and $90ED.
; HOW: The address in zTemp0A-0B, the count in zTemp0E; the VDC's data port.
; WAITS: no.
; Entry: zTemp08-09 the base, zTemp0F the entry. Exit: A the new zTemp0F, N, Z, C and V from its add; X and Y kept.
StageClearBitRows3CD:
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
StageClearBitRows3CD_at0:
    phx
    phy
    ldx z:zTemp0A
    ldy z:zTemp0B
    jsr SetVramAddresses3CD
    ply
    lda a:VDC_DATA_LO
    and a:$90E5,y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and a:$90ED,y
    sta a:VDC_DATA_HI
    plx
    lda z:zTemp0A
    clc
    adc #$10
    sta z:zTemp0A
    dec z:zTemp0E
    bne StageClearBitRows3CD_at0
    lda z:zTemp0F
    clc
    adc #$23
    sta z:zTemp0F
    rts

; @routine 03CF:0FD StageSubModeTally3CD
; @run 6B:90FD
; @stop 910D
; @unreachable 6B:90FD-6B:910C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $90FD runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Sub-mode 6, the tally after the boss: the events step, the display, the overlays' frame, then the
; tally's step at zField80 from the eight at $910D (the start, the board, the board's reveal, a pause, the health and
; hearts counted into the score, a wait, the stock bonus, the end).
; HOW: StageModeEventsStep152D, StageHudStep3CD, StageOverlayFrame3CD; zField80 doubled indexes the table, whose
; word is jumped through.
; WAITS: as the step.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zField80, N and Z from it, C its
; bit 7.
StageSubModeTally3CD:
    jsr StageModeEventsStep3CD
    jsr StageHudStep3CD
    jsr StageOverlayFrame3CD
    lda z:zField80
    asl a
    tax
    jmp ($910D,x)

; @routine 03CF:11D TallyStart3CD
; @run 6B:911D
; @stop 9128
; @unreachable 6B:911D-6B:9127 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $911D runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The tally's first step, after the wait: the kernel's piece at $4087, sound $46, and the next step
; (runs on into TallyNext152D).
; HOW: TallyWait152D; $4087; the sound hook.
; WAITS: no.
; Entry: nothing. Exit: as TallyWait152D when it ends the step; else as TallyNext152D.
TallyStart3CD:
    jsr TallyWait3CD
    jsr VecStopSound
    lda #$46
    jsr VecSoundCommand

; @routine 03CF:12B TallyBoard3CD
; @run 6B:912B
; @unreachable 6B:912B-6B:9174 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $912B runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The tally's board: the kernel's piece at $4012 (A $16), the display's pieces 5 to 7
; (StageDrawPiece3CD), the hearts kept at $330D, the full-health mark ($2A9B $18 at zPlayerHealth $5C), three
; blocks of VRAM words cleared (StageClearWords3CD: $0E at $5000, $0C at $5400, $0A at $5D80), the board's cells
; at $3980 from the 62 at $93A9; the next step (TallyNext152D), then runs on into TallyDigits3CD.
; HOW: Calls; a copy loop.
; WAITS: no.
; Entry: nothing. Exit: as TallyDigits3CD.
TallyBoard3CD:
    lda #$16
    jsr VecStartScriptSlot0Kind1
    ldx #$05
    jsr StageDrawPiece3CD
    inx
    jsr StageDrawPiece3CD
    inx
    jsr StageDrawPiece3CD
    lda z:zField94
    sta a:OBJ_WORK_B+13
    lda z:zPlayerHealth
    cmp #$5C
    bne TallyBoard3CD_at0
    lda #$18
    sta a:OBJ_FRAME+51
TallyBoard3CD_at0:
    clx
    ldy #$50
    lda #$0E
    jsr StageClearWords3CD
    clx
    ldy #$54
    lda #$0C
    jsr StageClearWords3CD
    ldx #$80
    ldy #$5D
    lda #$0A
    jsr StageClearWords3CD
    clx
TallyBoard3CD_at1:
    lda a:$93A9,x
    sta a:OBJ_HITBOX_DX+80,x
    inx
    cpx #$3E
    bne TallyBoard3CD_at1
    jsr $9128

; @routine 03CF:175 TallyDigits3CD
; @run 6B:9175
; @unreachable 6B:9175-6B:919C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9175 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Puts the health and the hearts on the board as digits: zPlayerHealth's tens and units
; (StageSplitTens152D) at $3991 and $3992, zField94's two BCD digits at $39A1 and $39A2, each one more than the digit;
; a leading zero is left out.
; HOW: StageSplitTens152D; shifts and masks; stores.
; WAITS: no.
; Entry: nothing. Exit: A the hearts' last cell, N and Z from its increment; X the health's tens cell; C from the
; compare, V from the split's add. Y kept.
TallyDigits3CD:
    lda z:zPlayerHealth
    jsr StageSplitTens152DAt9405
    inx
    cpx #$01
    beq TallyDigits3CD_at0
    stx a:OBJ_HITBOX_HALF_W+9
TallyDigits3CD_at0:
    inc a
    sta a:OBJ_HITBOX_HALF_W+10
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    inc a
    cmp #$01
    beq TallyDigits3CD_at1
    sta a:OBJ_HITBOX_HALF_W+25
TallyDigits3CD_at1:
    lda z:zField94
    and #$0F
    inc a
    sta a:OBJ_HITBOX_HALF_W+26
    rts

; @routine 03CF:19D TallyReveal3CD
; @run 6B:919D
; @unreachable 6B:919D-6B:9229 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $919D runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The board's reveal, under the stage's banks: once all 62 cells at $3980 are 0 the reveal is done
; (zPaused's bit 1 set, the wait at $3310 $40, the next step). Else a random cell that is not yet 0 is stepped by $40
; (cleared when it carries), and drawn: a cell of the first ten columns as its 16-word glyph from the picture at $C034
; (rows from $5D80); the others by TallyCell3CD.
; HOW: MapStageBanks (a census duplicate at $93E7); a loop down X; the kernel's random piece at $4051 until it gives a
; live cell; shifts into zTemp08-09, the glyph's address in zTemp10-11; SetVramWriteAddress3CD; a loop of 16 words
; with SXY.
; WAITS: no.
; Entry: nothing. Exit: as the last instruction that ran left them.
TallyReveal3CD:
    jsr MapStageBanksAt93E7
    ldx #$3D
TallyReveal3CD_at1:
    lda a:OBJ_HITBOX_DX+80,x
    bne TallyReveal3CD_at0
    dex
    bpl TallyReveal3CD_at1
    smb1 z:zPaused
    lda #$40
    sta a:OBJ_WORK_B+16
    jmp $9128
TallyReveal3CD_at0:
    jsr VecNextRandom
    and #$3F
    cmp #$3E
    bcs TallyReveal3CD_at0
    tax
    lda a:OBJ_HITBOX_DX+80,x
    beq TallyReveal3CD_at0
    sta z:zTemp00
    adc #$40
    sta a:OBJ_HITBOX_DX+80,x
    bcc TallyReveal3CD_at2
    stz a:OBJ_HITBOX_DX+80,x
TallyReveal3CD_at2:
    txa
    and #$3F
    cmp #$0A
    bcs TallyCell3CD
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
    lda #$34
    clc
    adc z:zTemp08
    sta z:zTemp10
    lda #$C0
    adc z:zTemp09
    sta z:zTemp11
    jsr SetVramWriteAddress3CD
    cly
    ldx #$10
TallyReveal3CD_at3:
    sxy
    lda (zTemp10),y
    sta a:VDC_DATA_LO
    sxy
    lda (zTemp10),y
    sta a:VDC_DATA_HI
    iny
    inx
    cpy #$10
    bne TallyReveal3CD_at3
    rts

; @routine 03CF:22A TallyCell3CD
; @run 6B:922A
; @unreachable 6B:922A-6B:92AE no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $922A runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Draws one of the board's cells past the first ten columns: A the cell's index, zTemp00 its value
; (its top two bits the shade, its low six the glyph). The cell's place gives the VRAM word (rows of eight from $5008,
; odd and even halves) and which of the word's bytes it fills; eight bytes of the glyph from the picture at $BF54
; (eight a glyph) go into that byte, the other byte written back as it was.
; HOW: A subtract and shifts into zTemp01-02 and zTemp08-09, the glyph's address in zTemp10-11; SetVramAddresses3CD;
; a loop of eight reads and writes of the VDC's data port.
; WAITS: no.
; Entry: A the cell's index ($0A up), C set; zTemp00 the cell's value. Exit: Y 8, Z set, C set; A the last byte read
; or written; X the VRAM address's low byte. Clobbers zTemp01-02, zTemp08-11.
TallyCell3CD:
    sec
    sbc #$0A
    tax
    sta z:zTemp01
    lsr a
    and #$1C
    bbr2 zTemp01,TallyCell3CD_at0
    ora #$20
TallyCell3CD_at0:
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
    lda #$54
    clc
    adc z:zTemp08
    sta z:zTemp10
    lda #$BF
    adc z:zTemp09
    sta z:zTemp11
    jsr SetVramAddresses3CD
    cly
    bbs0 zTemp01,TallyCell3CD_at1
TallyCell3CD_at2:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lda (zTemp10),y
    sta a:VDC_DATA_HI
    iny
    cpy #$08
    bne TallyCell3CD_at2
    rts
TallyCell3CD_at1:
    lda a:VDC_DATA_LO
    lda (zTemp10),y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    iny
    cpy #$08
    bne TallyCell3CD_at1
TallyCell3CD_at3:
    rts

; @routine 03CF:2AF TallyPause3CD
; @run 6B:92AF
; @unreachable 6B:92AF-6B:92B9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $92AF runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A pause of the tally: after the wait, the next wait at $3310 set to $40 and the next step.
; HOW: TallyWait152D; a store; a jump to TallyNext152D.
; WAITS: no.
; Entry: nothing. Exit: as TallyWait152D when it ends the step; else as TallyNext152D, A $40.
TallyPause3CD:
    jsr TallyWait3CD
    lda #$40
    sta a:OBJ_WORK_B+16
    jmp $9128

; @routine 03CF:2BA TallyCount3CD
; @run 6B:92BA
; @unreachable 6B:92BA-6B:930F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $92BA runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The tally's count: the paused frame's menu step with zPaused's bit 0 set for it; while zPaused's bit 1
; is clear the health is counted into the score (zPlayerHealth down by one, its cells redrawn by TallyHealth3CD, 1
; point by the kernel's piece at $408A), then after the wait the hearts (zField94 down by one in BCD, TallyHearts3CD,
; $10 points); the score redrawn and sound 1 every fourth event. With both at 0 the tally goes on two steps, or, when
; the full-health mark at $2A9B is set, stays with zPaused's bit 1 set and the wait at $3310 $40.
; HOW: Bit changes; StagePausedFrame3CD; decrements; $408A; StageDrawScore3CD; the sound hook.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them.
TallyCount3CD:
    smb0 z:zPaused
    jsr StagePausedFrame3CD
    rmb0 z:zPaused
    bbs1 zPaused,TallyCell3CD_at3
    lda z:zPlayerHealth
    beq TallyCount3CD_at0
    dec z:zPlayerHealth
    jsr TallyHealth3CD
    lda #$01
    bra TallyCount3CD_at1
TallyCount3CD_at0:
    jsr TallyWait3CD
    lda z:zField94
    beq TallyCount3CD_at2
    sed
    sec
    sbc #$01
    sta z:zField94
    cld
    jsr TallyHearts3CD
    lda #$10
TallyCount3CD_at1:
    jsr VecAddScoreByte
    jsr StageDrawScore3CD
    lda z:zModeEvents
    and #$03
    bne TallyCount3CD_at3
    lda #$01
    jsr VecSoundCommand
TallyCount3CD_at3:
    rts
TallyCount3CD_at2:
    dec a:OBJ_WORK_B+16
    inc z:zField80
    inc z:zField80
    lda a:OBJ_FRAME+51
    beq TallyCount3CD_at4
    dec z:zField80
    dec z:zField80
    smb1 z:zPaused
    lda #$40
    sta a:OBJ_WORK_B+16
TallyCount3CD_at4:
    jmp $9128

; @routine 03CF:310 TallyWaitPress3CD
; @run 6B:9310
; @unreachable 6B:9310-6B:9321 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9310 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: A wait of the tally: after the wait, the paused frame's menu step; once zPaused's bit 1 is clear the
; wait at $3310 is $20 and the next step.
; HOW: TallyWait152D; StagePausedFrame3CD; a bit test; a store and a jump to TallyNext152D.
; WAITS: no.
; Entry: nothing. Exit: as the last piece that ran left them.
TallyWaitPress3CD:
    jsr TallyWait3CD
    jsr StagePausedFrame3CD
    bbr1 zPaused,TallyWaitPress3CD_at0
    rts
TallyWaitPress3CD_at0:
    lda #$20
    sta a:OBJ_WORK_B+16
    jmp $9128

; @routine 03CF:322 TallyStockBonus3CD
; @run 6B:9322
; @unreachable 6B:9322-6B:933F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9322 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The stock bonus: after the wait, zStock up by one in BCD (kept at 99), redrawn (StageDrawStock3CD),
; sound $3E, the wait at $3310 one less and the next step.
; HOW: TallyWait152D; SED, ADC, CLD; StageDrawStock3CD; the sound hook; a decrement; a jump to TallyNext152D.
; WAITS: no.
; Entry: D clear, as the game keeps it. Exit: as TallyNext152D.
TallyStockBonus3CD:
    jsr TallyWait3CD
    sed
    lda z:zStock
    clc
    adc #$01
    bcc TallyStockBonus3CD_at0
    lda #$99
TallyStockBonus3CD_at0:
    sta z:zStock
    cld
    jsr StageDrawStock3CD
    lda #$3E
    jsr VecSoundCommand
    dec a:OBJ_WORK_B+16
    jmp $9128

; @routine 03CF:340 TallyEnd3CD
; @run 6B:9340
; @unreachable 6B:9340-6B:934A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9340 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The tally's end: after the wait, zField94 back from the hearts kept at $330D and zModeDone counted.
; HOW: TallyWait152D; a store; an increment.
; WAITS: no.
; Entry: nothing. Exit: A the hearts, N and Z from zModeDone's increment.
TallyEnd3CD:
    jsr TallyWait3CD
    lda a:OBJ_WORK_B+13
    sta z:zField94
    inc z:zModeDone
    rts

; @routine 03CF:356 TallyHealth3CD
; @run 6B:9356
; @unreachable 6B:9356-6B:9372 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9356 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Redraws the board's health: the stage's banks mapped, the digits put in (TallyDigits3CD), the units'
; cell $12 stepped (TallyDigitCell3CD); from 10 up the tens' cell $11 too, below it the tens' place cleared.
; HOW: MapStageBanks (a census duplicate at $93E7); TallyDigits3CD; TallyDigitCell3CD by BSR; a compare; a branch to
; TallyDigitCell3CD or a jump to StageClearWords3CD.
; WAITS: no.
; Entry: nothing. Exit: as TallyDigitCell3CD or StageClearWords3CD.
TallyHealth3CD:
    jsr MapStageBanksAt93E7
    jsr TallyDigits3CD
    ldx #$12
    bsr TallyDigitCell3CD
    lda z:zPlayerHealth
    cmp #$0A
    bcc TallyHealth3CD_at0
    ldx #$11
    bra TallyDigitCell3CD
TallyHealth3CD_at0:
    ldx #$40
    ldy #$54
    lda #$01
    jmp StageClearWords3CD

; @routine 03CF:373 TallyHearts3CD
; @run 6B:9373
; @unreachable 6B:9373-6B:938F no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9373 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Redraws the board's hearts as TallyHealth3CD does the health: the units' cell $22, and from $10 up
; (BCD) the tens' cell $21, below it the tens' place cleared.
; HOW: MapStageBanks (a census duplicate at $93E7); TallyDigits3CD; TallyDigitCell3CD by BSR; a compare; a branch to
; TallyDigitCell3CD or a jump to StageClearWords3CD.
; WAITS: no.
; Entry: nothing. Exit: as TallyDigitCell3CD or StageClearWords3CD.
TallyHearts3CD:
    jsr MapStageBanksAt93E7
    jsr TallyDigits3CD
    ldx #$22
    bsr TallyDigitCell3CD
    lda z:zField94
    cmp #$10
    bcc TallyHearts3CD_at0
    ldx #$21
    bra TallyDigitCell3CD
TallyHearts3CD_at0:
    ldx #$40
    ldy #$55
    lda #$01
    jmp StageClearWords3CD

; @routine 03CF:390 TallyDigitCell3CD
; @run 6B:9390
; @unreachable 6B:9390-6B:93A8 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9390 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Draws the board's cell X at every shade from its value up: TallyCell3CD for the cell, then the cell
; stepped by $40 until it carries.
; HOW: X kept on the stack across TallyCell3CD; an add; a loop.
; WAITS: no.
; Entry: X the cell. Exit: A the cell plus $40, C set, N and Z from it; X the cell; Y 8.
TallyDigitCell3CD:
    phx
    lda a:OBJ_HITBOX_DX+80,x
    sta z:zTemp00
    txa
    jsr TallyCell3CD
    plx
    lda a:OBJ_HITBOX_DX+80,x
    clc
    adc #$40
    bcs TallyDigitCell3CD_at0
    sta a:OBJ_HITBOX_DX+80,x
    bra TallyDigitCell3CD
TallyDigitCell3CD_at0:
    rts

; @routine 03CF:3F2 StageClearWords3CD
; @run 6B:93F2
; @unreachable 6B:93F2-6B:9404 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $93F2 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Clears A rows of 64 VRAM words from the address in X and Y.
; HOW: The count in zTemp00; SetVramWriteAddress3CD; ST1 and ST2 in loops.
; WAITS: no.
; Entry: A the rows, X and Y the address. Exit: X 0, A as SetVramWriteAddress3CD left it, N and Z from the count's
; decrement (Z set). Y, C and V as SetVramWriteAddress3CD left them. Clobbers zTemp00.
StageClearWords3CD:
    sta z:zTemp00
    jsr SetVramWriteAddress3CD
StageClearWords3CD_at1:
    ldx #$40
StageClearWords3CD_at0:
    st1 #$00
    st2 #$00
    dex
    bne StageClearWords3CD_at0
    dec z:zTemp00
    bne StageClearWords3CD_at1
    rts

; @routine 03CF:410 SetVramAddresses3CD
; @run 6B:9410
; @unreachable 6B:9410-6B:9418 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9410 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Points both of the VDC's VRAM addresses at the word in X and Y, then runs on into
; SetVramWriteAddress3CD. The bytes are those of SetVramAddresses (map_draw.c) at this overlay's address, where
; $9419 is an entry of its own.
; HOW: The kernel's piece at $4024 (the read address), X and Y into the data port; then SetVramWriteAddress3CD.
; WAITS: no.
; Entry: X low byte, Y high byte of the VRAM address. Exit: as SetVramWriteAddress3CD.
SetVramAddresses3CD:
    jsr VecSelectVramReadAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI

; @routine 03CF:419 SetVramWriteAddress3CD
; @run 6B:9419
; @unreachable 6B:9419-6B:9424 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9419 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Points the VDC's VRAM write address at the word in X and Y and selects the data port.
; HOW: The kernel's piece at $4021, X and Y into the data port; a jump to the kernel's piece at $4027.
; WAITS: no.
; Entry: X low byte, Y high byte of the VRAM address. Exit: as $4027 leaves them.
SetVramWriteAddress3CD:
    jsr VecSelectVramWriteAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jmp VecSelectVramData

; @routine 03CF:443 StageDrawPiece3CD
; @run 6B:9443
; @unreachable 6B:9443-6B:9462 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9443 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Places one of the display's pieces, X from 0 to 7: the kernel's piece at $405D gives a slot for the
; object the table at $9463 names, and the slot takes its sprite, set, X and Y from the tables at $946B, $9473,
; $947B and $9483.
; HOW: Table reads by X; $405D; stores by the slot in Y.
; WAITS: no.
; Entry: X the piece. Exit: A the Y written, N and Z from it; X and Y as $405D left them; C and V as it left them.
StageDrawPiece3CD:
    lda a:$9463,x
    tay
    jsr VecClearObjectInY
    lda a:$946B,x
    sta a:OBJ_SPRITE_SET+72,y
    lda a:$9473,x
    sta a:OBJ_SPRITE_SET,y
    lda a:$947B,x
    sta a:OBJ_X_HI+8,y
    lda a:$9483,x
    sta a:OBJ_X_FRAC+48,y
    rts

; @routine 03CF:48B StageMapDebug3CD
; @run 6B:948B
; @unreachable 6B:948B-6B:94B3 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $948B runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The map's debug marks, from the pause screen: for each of SaveBuffer's pages 1 to 3 that is in use (its
; byte at $7D01 to $7D03 not negative) the page's cells are marked on the screen (StageMapDebugPage3CD; page 1 from its
; cell $10, at the rows $10, $20 and $30).
; HOW: Three tests and calls.
; WAITS: no.
; Entry: nothing. Exit: as the last test or call left them.
StageMapDebug3CD:
    lda a:SaveBuffer+1
    bmi StageMapDebug3CD_at0
    ldy #$10
    lda #$7D
    ldx #$10
    jsr StageMapDebugPage3CD
StageMapDebug3CD_at0:
    lda a:SaveBuffer+2
    bmi StageMapDebug3CD_at2
    cly
    lda #$7E
    ldx #$20
    jsr StageMapDebugPage3CD
StageMapDebug3CD_at2:
    lda a:SaveBuffer+3
    bmi StageMapDebug3CD_at1
    cly
    lda #$7F
    ldx #$30
    jsr StageMapDebugPage3CD
StageMapDebug3CD_at1:
    rts

; @routine 03CF:4B4 StageMapDebugPage3CD
; @run 6B:94B4
; @unreachable 6B:94B4-6B:94E9 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $94B4 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Marks a page's cells on the screen: for each cell from Y to $FF of the page at A00 that is not 0, the
; 16 VRAM words at the cell's place (Y times 8, the high byte ORed with X) filled with $55AA.
; HOW: The page in zTemp00-01, the row in zTemp02; the kernel's pieces at $4021 and $4027; shifts into zTemp03; ST1
; and ST2 in a loop.
; WAITS: no.
; Entry: A the page's high byte, X the row's high byte, Y the first cell. Exit: Y 0, Z set; A, X, C and V as the last
; cell left them. Clobbers zTemp00-03.
StageMapDebugPage3CD:
    stz z:zTemp00
    sta z:zTemp01
    stx z:zTemp02
StageMapDebugPage3CD_at2:
    lda (zTemp00),y
    beq StageMapDebugPage3CD_at0
    jsr VecSelectVramWriteAddress
    stz z:zTemp03
    tya
    asl a
    rol z:zTemp03
    asl a
    rol z:zTemp03
    asl a
    rol z:zTemp03
    asl a
    rol z:zTemp03
    sta a:VDC_DATA_LO
    lda z:zTemp03
    ora z:zTemp02
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    ldx #$10
StageMapDebugPage3CD_at1:
    st1 #$AA
    st2 #$55
    dex
    bne StageMapDebugPage3CD_at1
StageMapDebugPage3CD_at0:
    iny
    bne StageMapDebugPage3CD_at2
    rts

; @routine 03CF:4EA StageObjectHits3CD
; @run 6B:94EA
; @unreachable 6B:94EA-6B:9504 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $94EA runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The objects' hits each frame: the hit boxes of slots 0 to 15 placed (StagePlaceHitBoxes3CD), then for
; objects $47 down to $10 the object's test against them (StageHitTestObject3CD) and the step of its hit timer at $3790
; (while it runs the object's state at $31F0 is cleared).
; HOW: X kept on the stack; calls in a loop down X.
; WAITS: no.
; Entry: nothing. Exit: X kept, N and Z from it; C clear; A, Y and V as the last object's step left them.
StageObjectHits3CD:
    phx
    jsr StagePlaceHitBoxes3CD
    ldx #$47
StageObjectHits3CD_at1:
    jsr StageHitTestObject3CD
    lda a:OBJ_WORK_L+24,x
    beq StageObjectHits3CD_at0
    stz a:OBJ_HIT_STATUS+80,x
    dec a:OBJ_WORK_L+24,x
StageObjectHits3CD_at0:
    dex
    cpx #$10
    bcs StageObjectHits3CD_at1
    plx
    rts

; @routine 03CF:505 StagePlaceHitBoxes3CD
; @run 6B:9505
; @unreachable 6B:9505-6B:955E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9505 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Places the hit boxes of slots 0 to 15: with no event a box is off the page ($2733 1); else its X (the
; slot's X moved by its offset at $3670, negated when it faces the other way) and Y (moved by the offset at $3700) at
; $2713 and $2723, and the two high bytes ORed into $2733.
; HOW: A loop up X; 16-bit adds with the sign in zTemp00, the offset in zTemp01.
; WAITS: no.
; Entry: nothing. Exit: X $10, Z set, C set; A the last page byte.
StagePlaceHitBoxes3CD:
    ldx #$00
StagePlaceHitBoxes3CD_at5:
    lda a:OBJ_EVENT_ID+56,x
    bne StagePlaceHitBoxes3CD_at0
    lda #$01
    sta a:HitboxHi+3,x
    bra StagePlaceHitBoxes3CD_at1
StagePlaceHitBoxes3CD_at0:
    lda a:OBJ_WORK_I,x
    sta z:zTemp01
    lda a:OBJ_FRAME+56,x
    beq StagePlaceHitBoxes3CD_at2
    lda z:zTemp01
    eor #$FF
    inc a
    sta z:zTemp01
StagePlaceHitBoxes3CD_at2:
    stz z:zTemp00
    lda z:zTemp01
    bpl StagePlaceHitBoxes3CD_at3
    dec z:zTemp00
StagePlaceHitBoxes3CD_at3:
    clc
    adc a:OBJ_X_HI+8,x
    sta a:HitboxXLo+3,x
    lda a:OBJ_SPRITE_ATTR+24,x
    adc z:zTemp00
    sta a:HitboxHi+3,x
    stz z:zTemp00
    lda a:OBJ_WORK_J+56,x
    bpl StagePlaceHitBoxes3CD_at4
    dec z:zTemp00
StagePlaceHitBoxes3CD_at4:
    lda a:OBJ_X_FRAC+48,x
    clc
    adc a:OBJ_WORK_J+56,x
    sta a:HitboxYLo+3,x
    lda a:OBJ_X_LO+64,x
    adc z:zTemp00
    ora a:HitboxHi+3,x
    sta a:HitboxHi+3,x
StagePlaceHitBoxes3CD_at1:
    inx
    cpx #$10
    bcc StagePlaceHitBoxes3CD_at5
    rts

; @routine 03CF:55F StageHitTestObject3CD
; @run 6B:955F
; @stop 95FB
; @unreachable 6B:955F-6B:95FA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $955F runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tests object X against the hit boxes: its state at $31F0 cleared; with an event and a reach, its point
; (its X moved by its facing offset, its Y by the offset at $3700) on the screen's page is tested against the boxes of
; slots 15 down to 0 (only slot 0 for an event with bit 7); a box within both reaches is a hit (StageHitHandler3CD).
; HOW: Tests; 16-bit adds with the sign in zTemp00; the point in $2702-$2703, the reaches in $2704-$2705; a loop
; down Y; jumps to the RTS at $95FB and a branch to StageHitHandler3CD.
; WAITS: no.
; Entry: X the object. Exit: as the last test or the handler left them.
StageHitTestObject3CD:
    stz a:OBJ_HIT_STATUS+80,x
    lda a:OBJ_EVENT_ID+56,x
    bne StageHitTestObject3CD_at0
    jmp $95FB
StageHitTestObject3CD_at0:
    lda a:OBJ_WORK_I+72,x
    ora a:OBJ_WORK_K+40,x
    bne StageHitTestObject3CD_at1
    jmp $95FB
StageHitTestObject3CD_at1:
    lda a:OBJ_WORK_I,x
    sta z:zTemp01
    lda a:OBJ_FRAME+56,x
    beq StageHitTestObject3CD_at2
    lda z:zTemp01
    eor #$FF
    inc a
    sta z:zTemp01
StageHitTestObject3CD_at2:
    stz z:zTemp00
    lda z:zTemp01
    bpl StageHitTestObject3CD_at3
    dec z:zTemp00
StageHitTestObject3CD_at3:
    clc
    adc a:OBJ_X_HI+8,x
    sta a:StreamStepHi
    lda a:OBJ_SPRITE_ATTR+24,x
    adc z:zTemp00
    bne $95FB
    stz z:zTemp00
    lda a:OBJ_WORK_J+56,x
    bpl StageHitTestObject3CD_at4
    dec z:zTemp00
StageHitTestObject3CD_at4:
    clc
    adc a:OBJ_X_FRAC+48,x
    sta a:StreamStepHi+1
    lda a:OBJ_X_LO+64,x
    adc z:zTemp00
    bne $95FB
    lda a:OBJ_EVENT_ID+56,x
    bpl StageHitTestObject3CD_at5
    cly
    bra StageHitTestObject3CD_at6
StageHitTestObject3CD_at5:
    ldy #$0F
StageHitTestObject3CD_at6:
    lda a:HitboxHi+3,y
    bne StageHitTestObject3CD_at9
    lda a:OBJ_WORK_I+72,x
    clc
    adc a:OBJ_WORK_I+72,y
    sta a:StreamStepHi+2
    lda a:OBJ_WORK_K+40,x
    clc
    adc a:OBJ_WORK_K+40,y
    sta a:StreamStepHi+3
    lda a:StreamStepHi
    sec
    sbc a:HitboxXLo+3,y
    bcs StageHitTestObject3CD_at7
    eor #$FF
    inc a
StageHitTestObject3CD_at7:
    cmp a:StreamStepHi+2
    bcs StageHitTestObject3CD_at9
    lda a:StreamStepHi+1
    sec
    sbc a:HitboxYLo+3,y
    bcs StageHitTestObject3CD_at8
    eor #$FF
    inc a
StageHitTestObject3CD_at8:
    cmp a:StreamStepHi+3
    bcc StageHitHandler3CD
StageHitTestObject3CD_at9:
    dey
    bpl StageHitTestObject3CD_at6

; @routine 03CF:5FC StageHitHandler3CD
; @run 6B:95FC
; @unreachable 6B:95FC-6B:9619 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $95FC runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Object X hit slot Y's box, as StageHitHandler152D: the hitter kept at $35E0, zHitSlot the slot, the
; slot's kind's handler from the table of 15 at $961A.
; HOW: Stores; a table read into zTemp00-01 and a jump through it.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: kind 0: A 0, N and Z from it. Else: as the handler leaves them.
StageHitHandler3CD:
    txa
    sta a:OBJ_WORK_G+32,y
    sty z:zHitSlot
    lda a:OBJ_WORK_F+48,y
    beq StageHitHandler3CD_at0
    dec a
    asl a
    tay
    lda a:$961A,y
    sta z:zTemp00
    lda a:$961B,y
    sta z:zTemp01
    ldy z:zHitSlot
    jmp (ZeroPage)
StageHitHandler3CD_at0:
    rts

; @routine 03CF:655 HitKindFacing3CD
; @run 6B:9655
; @unreachable 6B:9655-6B:9661 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9655 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Hit kind 6: the slot's side at $3628 from the object's facing (bit 0 of $2FF8, plus one); unless it is
; 2, runs on into HitKindReport3CD.
; HOW: A mask and an increment; a compare.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: side 2: A 2, Z set, C set. Else as HitKindReport3CD.
HitKindFacing3CD:
    lda a:OBJ_SPRITE_FLIP+16,x
    and #$01
    inc a
    sta a:OBJ_WORK_H+16,y
HitKindFacing3CD_at0:
    cmp #$02
    beq $9680

; @routine 03CF:662 HitKindReport3CD
; @run 6B:9662
; @stop 967A
; @unreachable 6B:9662-6B:9679 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9662 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Hit kinds 5, 10, 11, 13, 14 and 15: the hit's point ($2702-$2703) and hit box 3's X and Y ($2716,
; $2726) kept in StageHitReport's four bytes, then the object takes the slot's kind (runs on into the bytes of
; HitKindState152D at $967A).
; HOW: Loads and stores.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: as HitKindState152D.
HitKindReport3CD:
    lda a:StreamStepHi
    sta a:StageHitReport
    lda a:StreamStepHi+1
    sta a:StageHitReport+1
    lda a:HitboxXLo+6
    sta a:StageHitReport+2
    lda a:HitboxYLo+6
    sta a:StageHitReport+3

; @routine 03CF:681 HitKindSide3CD
; @run 6B:9681
; @unreachable 6B:9681-6B:968C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9681 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Hit kind 9: the slot's side at $3628 from bit 1 of the object's $2FF8 (plus one); unless it is 2, the
; hit is reported. The compare and what follows are HitKindFacing3CD's bytes at $965E.
; HOW: A mask, a shift and an increment; a branch into HitKindFacing3CD's bytes.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: as HitKindFacing3CD.
HitKindSide3CD:
    lda a:OBJ_SPRITE_FLIP+16,x
    and #$02
    lsr a
    inc a
    sta a:OBJ_WORK_H+16,y
    bra HitKindFacing3CD_at0

; @routine 03CF:68D StageProbePoints3CD
; @run 6B:968D
; @unreachable 6B:968D-6B:96DA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $968D runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Probes the map at an object's four points (the list the index in A names, from the table at $D756:
; four offsets of X and Y a point from the object's position): for each, ProbeTile3CD's answer at $26F6 and the
; tile's column and row at $26FA and $26FE.
; HOW: The list in zTemp0C-0D, the count in zTemp0A; the point into zTemp00-03; Y kept on the stack across
; ProbeTile3CD.
; WAITS: no.
; Entry: A the list, X the object. Exit: A 4, Z set, C set; Y past the last point; X kept as ProbeTile3CD keeps it.
StageProbePoints3CD:
    asl a
    tay
    lda a:$D756,y
    sta z:zTemp0C
    lda a:$D757,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
StageProbePoints3CD_at0:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_HI+8,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_SPRITE_ATTR+24,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_FRAC+48,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_LO+64,x
    sta z:zTemp03
    iny
    phy
    jsr ProbeTile3CD
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
    bne StageProbePoints3CD_at0
    rts

; @routine 03CF:6DB StageProbeAgain3CD
; @run 6B:96DB
; @unreachable 6B:96DB-6B:974E no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $96DB runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Probes an object's four points (StageProbePoints3CD), then each point whose answer has bit 0 set and
; bit 7 clear again 8 pixels higher; when the second probe's answer has bit 0 set the first answer keeps only bits 1
; to 6.
; HOW: A kept on the stack across StageProbePoints3CD; the list again; for each point a test of its answer, the point
; into zTemp00-03 and $FFF8 added to its Y, ProbeTile3CD with Y kept on the stack.
; WAITS: no.
; Entry: A the list, X the object. Exit: A 4, Z set, C set; Y past the last point.
StageProbeAgain3CD:
    pha
    jsr StageProbePoints3CD
    pla
    asl a
    tay
    lda a:$D756,y
    sta z:zTemp0C
    lda a:$D757,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
StageProbeAgain3CD_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit+3,y
    ply
    and #$81
    bmi StageProbeAgain3CD_at0
    beq StageProbeAgain3CD_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_HI+8,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_SPRITE_ATTR+24,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_FRAC+48,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_LO+64,x
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
    jsr ProbeTile3CD
    ldy z:zTemp0A
    and #$01
    beq StageProbeAgain3CD_at1
    ldy z:zTemp0A
    lda a:TileHit+3,y
    and #$7E
    sta a:TileHit+3,y
StageProbeAgain3CD_at1:
    ply
StageProbeAgain3CD_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne StageProbeAgain3CD_at2
    rts
StageProbeAgain3CD_at0:
    iny
    iny
    iny
    iny
    bra StageProbeAgain3CD_at3

; @routine 03CF:74F ProbeTile3CD
; @run 6B:974F
; @unreachable 6B:974F-6B:97E5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $974F runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Says what the map holds at a point, as ProbeTile152D up to the attribute: a page of bit 7 gives 0, an
; attribute below $80 or from $90 up is the answer; $80 to $8F go to their handlers (the table of 16 at $97E6), with A
; the attribute, which set the row in the tile (zTemp02) and answer $81.
; HOW: 9-bit adds of the point and the scroll into zTemp04-07; the kernel's pieces at $4024 and $4027; the entry in
; zTemp08-09; the handler's address in zTemp04-05 and a jump through it.
; WAITS: no.
; Entry: zTemp00-01 X, zTemp02-03 Y. Exit: A the answer, N and Z from it; zTemp00 the column, zTemp02 the row.
; Clobbers zTemp04-09.
ProbeTile3CD:
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
    bmi ProbeTile3CD_at2
    lda z:zTemp08
    clc
    adc #$00
    sta z:zTemp08
    lda a:SaveBuffer,y
    adc #$7D
    sta z:zTemp09
    lda (zTemp08)
    bmi ProbeTile3CD_at0
    rts
ProbeTile3CD_at2:
    lda #$00
ProbeTile3CD_at1:
    rts
ProbeTile3CD_at0:
    cmp #$90
    bcs ProbeTile3CD_at1
    pha
    sbc #$7F
    asl a
    tay
    lda a:TileHalfStepsB+1,y
    sta z:zTemp04
    lda a:TileHalfStepsB+2,y
    sta z:zTemp05
    pla
    jmp (zTemp04+$2000)

; @routine 03D0:006 TileSlopeA3CD
; @run 6B:9806
; @unreachable 6B:9806-6B:9812 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9806 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tile attribute $80: the row moved by the table at $989C by the column; answers $81.
; HOW: A table read and an add.
; WAITS: no.
; Entry: zTemp00 the column, zTemp02 the row. Exit: A $81, N set; Y the column; C and V from the add.
TileSlopeA3CD:
    ldy z:zTemp00
    lda a:$989C,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 03D0:013 TileSlopeB3CD
; @run 6B:9813
; @unreachable 6B:9813-6B:9823 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9813 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tile attribute $81: the row from the 8 by 8 table at $98A4; answers $81.
; HOW: Shifts, an add and a table read.
; WAITS: no.
; Entry: zTemp00 the column, zTemp02 the row. Exit: A $81, N set; Y the index; C and V from the add.
TileSlopeB3CD:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:$98A4,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 03D0:024 TileSlopeC3CD
; @run 6B:9824
; @unreachable 6B:9824-6B:9834 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9824 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tile attribute $82: the row from the 8 by 8 table at $98E4; answers $81.
; HOW: Shifts, an add and a table read.
; WAITS: no.
; Entry: zTemp00 the column, zTemp02 the row. Exit: as TileSlopeB3CD.
TileSlopeC3CD:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:$98E4,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 03D0:035 TileStepRow3CD
; @run 6B:9835
; @unreachable 6B:9835-6B:9843 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9835 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tile attributes $87 to $8A: the row moved by the table at $988C by the attribute less $83; answers $81.
; HOW: A subtract, a table read and an add.
; WAITS: no.
; Entry: A the attribute, zTemp02 the row. Exit: A $81, N set; Y the index; C and V from the add.
TileStepRow3CD:
    sec
    sbc #$83
    tay
    lda a:$988C,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 03D0:044 TileStepPair3CD
; @run 6B:9844
; @unreachable 6B:9844-6B:9857 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9844 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tile attributes $8C to $8F: the row moved by one of a pair in the table at $9894 (the attribute less
; $8C doubled, the second when the column's bit 2 is clear); answers $81.
; HOW: A subtract and a shift; BBS2 of zTemp00; a table read and an add.
; WAITS: no.
; Entry: A the attribute, zTemp00 the column, zTemp02 the row. Exit: as TileStepRow3CD.
TileStepPair3CD:
    sec
    sbc #$8C
    asl a
    tay
    bbs2 zTemp00,TileStepPair3CD_at0
    iny
TileStepPair3CD_at0:
    lda a:$9894,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 03D0:058 TileRowDown3CD
; @run 6B:9858
; @unreachable 6B:9858-6B:985C no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9858 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tile attributes $86 and $8B: the row one lower; answers $81.
; HOW: An increment.
; WAITS: no.
; Entry: zTemp02 the row. Exit: A $81, N set. X, Y, C and V kept.
TileRowDown3CD:
    inc z:zTemp02
    lda #$81
    rts

; @routine 03D0:05D TileSlopeD3CD
; @run 6B:985D
; @unreachable 6B:985D-6B:9869 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $985D runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tile attribute $83: the row moved by the table at $9924 by the column; answers $81.
; HOW: A table read and an add.
; WAITS: no.
; Entry: zTemp00 the column, zTemp02 the row. Exit: as TileSlopeA3CD.
TileSlopeD3CD:
    ldy z:zTemp00
    lda a:$9924,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 03D0:06A TileSlopeE3CD
; @run 6B:986A
; @unreachable 6B:986A-6B:987A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $986A runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tile attribute $84: the row from the 8 by 8 table at $992C; answers $81.
; HOW: Shifts, an add and a table read.
; WAITS: no.
; Entry: zTemp00 the column, zTemp02 the row. Exit: as TileSlopeB3CD.
TileSlopeE3CD:
    lda z:zTemp02
TileSlopeE3CD_at0:
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:$992C,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 03D0:07B TileSlopeF3CD
; @run 6B:987B
; @unreachable 6B:987B-6B:988B no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $987B runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: Tile attribute $85: the row from the 8 by 8 table at $996C; answers $81.
; HOW: Shifts, an add and a table read.
; WAITS: no.
; Entry: zTemp00 the column, zTemp02 the row. Exit: as TileSlopeB3CD.
TileSlopeF3CD:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowOffsets+4,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 03D0:1AC GateObjectStep3CD
; @run 6B:99AC
; @stop 99BE
; @unreachable 6B:99AC-6B:99BD no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $99AC runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: An object handler for the stage's gate, as GateObjectStep152D: its step at $2818 from the table of
; seven at $99BE. No code of the overlay reaches it; the census counts none of its bytes.
; HOW: The step doubled indexes the table; its word goes through zTemp00-01 and is jumped through.
; WAITS: no.
; Entry: X the object. Exit: as the step leaves them.
GateObjectStep3CD:
    lda a:OBJ_SUBSTATE+24,x
    asl a
    tay
    lda a:$99BE,y
    sta z:zTemp00
    lda a:$99BF,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 03D0:1CC GateStart3CD
; @run 6B:99CC
; @stop 99DB
; @unreachable 6B:99CC-6B:99DA no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $99CC runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The gate's first step: the kernel's piece at $40D2 and its sprite at $2A58 from the two at $99DC by
; bit 0 of its packed byte; then the RTS at $99DB (StageSubStepNone152D's kind, run on into).
; HOW: $40D2; a table read.
; WAITS: no.
; Entry: X the object. Exit: A the sprite, N and Z from it, Y the bit.
GateStart3CD:
    jsr VecLoadObjectRecord
    lda a:OBJ_WORK_A+64,x
    and #$01
    tay
    lda a:$99DC,y
    sta a:OBJ_SPRITE_SET+72,x

; @routine 03D0:1E0 GateWaitPlayer3CD
; @run 6B:99E0
; @unreachable 6B:99E0-6B:9A40 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $99E0 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The gate waits for the player, as GateWaitPlayer152D, with camera mode 1 cleared when the stage is
; cleared through it; a packed byte with a high nibble other than $F names the sound, and camera mode is cleared;
; with no high nibble, packed bytes 2 to 5 start the card's CD fade of type $0C.
; HOW: Tests; stores; shifts; the kernel's piece at $400F with X 6 (X on the stack); a jump to cd_fade.
; WAITS: no.
; Entry: X the object. Exit: as the last test, the kernel or cd_fade left them; X kept.
GateWaitPlayer3CD:
    lda a:LevelFullWalk+2
    eor #$01
    and a:PlayerDoorReady
    beq GateWaitPlayer3CD_at0
    lda a:OBJ_HIT_STATUS+80,x
    beq GateWaitPlayer3CD_at0
    cmp #$03
    bcs GateWaitPlayer3CD_at0
    lda a:OBJ_WORK_A+64,x
    and #$0F
    inc a
    sta z:zField8C
    lda a:CameraMode
    dec a
    bne GateWaitPlayer3CD_at1
    stz a:CameraMode
GateWaitPlayer3CD_at1:
    lda #$03
    sta z:zSubStep
    inc a:OBJ_SUBSTATE+24,x
    stz a:OBJ_WORK_B+16,x
    lda #$04
    sta a:OBJ_HEALTH+32,x
    lda a:OBJ_WORK_A+64,x
    and #$F0
    beq GateWaitPlayer3CD_at2
    stz z:zRasterEffect
    stz a:CameraMode
    cmp #$F0
    beq GateWaitPlayer3CD_at0
    lsr a
    lsr a
    lsr a
    lsr a
    ora #$80
    phx
    ldx #$06
    jsr VecStartListChannelKind2
    plx
GateWaitPlayer3CD_at0:
    rts
GateWaitPlayer3CD_at2:
    lda a:OBJ_WORK_A+64,x
    cmp #$02
    bcc GateWaitPlayer3CD_at0
    cmp #$06
    bcs GateWaitPlayer3CD_at0
    lda #$0C
    jmp BiosCdFade

; @routine 03D0:241 GateWaitStep3CD
; @run 6B:9A41
; @unreachable 6B:9A41-6B:9A4A no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9A41 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The gate waits for the cleared stage's step at $85 to reach 2, then goes on.
; HOW: A compare; an increment.
; WAITS: no.
; Entry: X the object. Exit: A $85, N, Z and C from the compare, or N and Z from the step's increment.
GateWaitStep3CD:
    lda z:zField85
    cmp #$02
    bne GateWaitClear3CD_at0
    inc a:OBJ_SUBSTATE+24,x
    rts

; @routine 03D0:24B GateOpen3CD
; @run 6B:9A4B
; @unreachable 6B:9A4B-6B:9A88 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9A4B runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The gate opens, as GateOpen152D, with sound $56 at the end.
; HOW: A countdown at $32C8; a compare; gate_frame; the sound hook by a jump.
; WAITS: no.
; Entry: X the object. Exit: as the last instruction, the sound hook or the kernel left them; X kept.
GateOpen3CD:
    dec a:OBJ_HEALTH+32,x
    bne GateOpen3CD_at0
    inc a:OBJ_WORK_B+16,x
    lda a:OBJ_WORK_B+16,x
    cmp #$04
    bne GateOpen3CD_at1
    inc a:OBJ_SUBSTATE+24,x
    lda z:zField8C
    ora #$80
    sta z:zField8C
    lda #$56
    jmp VecSoundCommand
GateOpen3CD_at1:
    lda #$04
    sta a:OBJ_HEALTH+32,x
    lda a:OBJ_WORK_A+64,x
    and #$01
    tay
    lda a:$99DC,y
    clc
    adc a:OBJ_WORK_B+16,x
    sta a:OBJ_SPRITE_SET+72,x
    clc
    adc a:$99DE,y
    phx
    ldx #$05
    jsr VecStartListChannelKind2
    plx
GateOpen3CD_at0:
    rts

; @routine 03D0:289 GateWaitClear3CD
; @run 6B:9A89
; @unreachable 6B:9A89-6B:9AA5 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9A89 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The gate waits until zField8C loses bit 7, then goes on with its timer 4; a gate whose packed byte is 2
; to 5 skips its closing.
; HOW: A test; stores; compares; a second increment.
; WAITS: no.
; Entry: X the object. Exit: A zField8C, N and Z from it; or A the packed byte, N, Z and C from its compare, or N and Z
; from the second increment.
GateWaitClear3CD:
    lda z:zField8C
    bmi GateWaitClear3CD_at0
    inc a:OBJ_SUBSTATE+24,x
    lda #$04
    sta a:OBJ_HEALTH+32,x
    lda a:OBJ_WORK_A+64,x
    cmp #$02
    bcc GateWaitClear3CD_at1
    cmp #$06
    bcc GateWaitClear3CD_at2
GateWaitClear3CD_at1:
    bra GateWaitClear3CD_at0
GateWaitClear3CD_at2:
    inc a:OBJ_SUBSTATE+24,x
GateWaitClear3CD_at0:
    rts

; @routine 03D0:2A6 GateClose3CD
; @run 6B:9AA6
; @unreachable 6B:9AA6-6B:9AD8 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9AA6 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: The gate closes, as GateClose152D, with sound $56 at the end.
; HOW: A countdown at $32C8; a decrement; gate_frame; the sound hook by a jump.
; WAITS: no.
; Entry: X the object. Exit: as the last instruction, the sound hook or the kernel left them; X kept.
GateClose3CD:
    dec a:OBJ_HEALTH+32,x
    bne GateClose3CD_at0
    dec a:OBJ_WORK_B+16,x
    bpl GateClose3CD_at1
    inc a:OBJ_SUBSTATE+24,x
    lda #$56
    jmp VecSoundCommand
GateClose3CD_at1:
    lda #$04
    sta a:OBJ_HEALTH+32,x
    lda a:OBJ_WORK_A+64,x
    and #$01
    tay
    lda a:$99DC,y
    clc
    adc a:OBJ_WORK_B+16,x
    sta a:OBJ_SPRITE_SET+72,x
    clc
    adc a:$99DE,y
    phx
    ldx #$05
    jsr VecStartListChannelKind2
    plx
GateClose3CD_at0:
    rts

; @routine 03CD:1AD StageModeEventsStep3CD
; @run 6B:81AD
; @unreachable 6B:81AD-6B:81D2 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $81AD runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: As StageModeEventsStep152D (152D:0B1), whose bytes these are.
; HOW: The same instructions; none of its absolute operands points into its own bytes, so it runs the same here.
; WAITS: no.
; Entry: nothing. Exit: as StageModeEventsStep152D: A the last value masked, N and Z from the last compare or mask; C
; from the last compare; V from the add. X and Y kept.
StageModeEventsStep3CD:
    inc z:zWaveCounter
    lda z:zModeEvents
    clc
    adc #$11
    cmp #$50
    bcc StageModeEventsStep3CD_at0
    and #$0F
StageModeEventsStep3CD_at0:
    sta z:zModeEvents
    and #$03
    cmp #$03
    bne StageModeEventsStep3CD_at1
    inc z:zModeEvents
    lda z:zModeEvents
    and #$0C
    cmp #$0C
    bne StageModeEventsStep3CD_at1
    lda z:zModeEvents
    and #$F0
    sta z:zModeEvents
StageModeEventsStep3CD_at1:
    rts

; @routine 03CF:34B TallyWait3CD
; @run 6B:934B
; @unreachable 6B:934B-6B:9355 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $934B runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: As TallyWait152D (152F:34D), whose bytes these are.
; HOW: The same instructions; none of its absolute operands points into its own bytes, so it runs the same here.
; WAITS: no.
; Entry: nothing. Exit: as TallyWait152D: count 0, A 0, N and Z from it, the rest kept; else the caller's frame
; dropped, A the last byte pulled, N and Z from it.
TallyWait3CD:
    lda a:OBJ_WORK_B+16
    beq TallyWait3CD_at0
    dec a:OBJ_WORK_B+16
    pla
    pla
TallyWait3CD_at0:
    rts

; @routine 03CF:638 HitKindSpawn3CD
; @run 6B:9638
; @unreachable 6B:9638-6B:9654 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $9638 runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: As HitKindSpawn152D (152F:5C7), whose bytes these are.
; HOW: The same instructions; none of its absolute operands points into its own bytes, so it runs the same here.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: as HitKindSpawn152D: A the last byte loaded, N and Z from it. X, Y, C and
; V kept.
HitKindSpawn3CD:
    lda a:OBJ_WORK_F+48,y
    sta a:OBJ_HIT_STATUS+80,x
    lda a:OBJ_HIT_STATUS+80
    bne HitKindSpawn3CD_at0
    lda a:HitStoreGateB
    bne HitKindSpawn3CD_at0
    lda a:OBJ_HIT_STATUS+8,x
    sta a:OBJ_HIT_STATUS+80
    lda a:OBJ_PACE+24,x
    sta a:OBJ_LOOP_FLAGS+40
HitKindSpawn3CD_at0:
    rts

; @routine 03CF:67A HitKindState3CD
; @run 6B:967A
; @unreachable 6B:967A-6B:9680 no read the game makes loads sectors 03CD to 03D0 or their six copies, so these
; bytes are never in memory (the copies are at 134D, 14DD, 154D, 15BD, 162D and 177D). Read in full: the 95 scene
; records at $74D1 and every op of their lists, the reads a later op overwrites included; the boot block's three
; reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the five cd_read calls no
; script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot run. No input or
; poke of work RAM can load the sectors, and a unit call at $967A runs whatever overlay another scene left there, not
; these bytes.
;
; WHAT IT DOES: As HitKindState152D (152F:5F1), whose bytes these are.
; HOW: The same instructions; none of its absolute operands points into its own bytes, so it runs the same here.
; WAITS: no.
; Entry: X the object, Y the slot. Exit: as HitKindState152D: A the kind, N and Z from it. X, Y, C and V kept.
HitKindState3CD:
    lda a:OBJ_WORK_F+48,y
    sta a:OBJ_HIT_STATUS+80,x
    rts


; @routine 031F:310 TallyStart031F
; @run 6B:9310
; WHAT IT DOES: The tally screen's first step: after its wait, the sound stopped, sound $52, the next step.
; HOW: TallyStart3CD's code with this program's wait and sound.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StagePausedStep031F = $8EA0 ; the paused step in this program
TallyNext031F = $931B ; the INC and RTS TallyStart031F runs into
MapStageBanks031F = $95E8 ; MapStageBanks' bytes (024F:349) in this program
StageSplitTens031F = $9611 ; StageSplitTens152D's bytes (152F:40A) in this program
SetVramAddressXY031F = $9625 ; SetVramAddressXY's bytes (004D:4DB) in this program
TallyCellSource031F = $95A0 ; the tally screen's first cells, $3E of them
TallyStart031F:
    jsr TallyWait031F
    jsr VecStopSound
    lda #$52
    jsr VecSoundCommand

; @routine 031F:31E TallyBoard031F
; @run 6B:931E
; WHAT IT DOES: As TallyBoard3CD: the tally board set up, then run on into its digits.
; HOW: TallyBoard3CD's code with this program's cells, bytes and routines.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyBoard031F:
    lda #$16
    jsr VecStartScriptSlot0Kind1
    ldx #$05
    jsr SpawnStageObject031F
    inx
    jsr SpawnStageObject031F
    inx
    jsr SpawnStageObject031F
    lda z:zField94
    sta a:OBJ_WORK_D+85
    lda z:zPlayerHealth
    cmp #$5C
    bne TallyBoard031F_at0
    lda #$18
    sta a:OBJ_FRAME+83
TallyBoard031F_at0:
    clx
    ldy #$50
    lda #$0E
    jsr ClearVramRows031F
    clx
    ldy #$54
    lda #$0C
    jsr ClearVramRows031F
    ldx #$80
    ldy #$5D
    lda #$0A
    jsr ClearVramRows031F
    clx
TallyBoard031F_at1:
    lda a:TallyCellSource031F,x
    sta a:StageMaskBuffer,x
    inx
    cpx #$3E
    bne TallyBoard031F_at1
    jsr TallyNext031F

; @routine 031F:368 TallyDigits031F
; @run 6B:9368
; WHAT IT DOES: As TallyDigits3CD: the health's and the hearts' digits put in the board's cells.
; HOW: TallyDigits3CD's code with this program's cells, bytes and routines.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyDigits031F:
    lda z:zPlayerHealth
    jsr StageSplitTens031F
    inx
    cpx #$01
    beq TallyDigits031F_at0
    stx a:StageMaskBuffer+17
TallyDigits031F_at0:
    inc a
    sta a:StageMaskBuffer+18
    lda z:zField94
    lsr a
    lsr a
    lsr a
    lsr a
    inc a
    cmp #$01
    beq TallyDigits031F_at1
    sta a:StageMaskBuffer+33
TallyDigits031F_at1:
    lda z:zField94
    and #$0F
    inc a
    sta a:StageMaskBuffer+34
    rts

; @routine 031F:41B TallyCell031F
; @run 6B:941B
; WHAT IT DOES: As TallyCell3CD: one cell of the board drawn.
; HOW: TallyCell3CD's code with this program's cells, bytes and routines.
; WAITS: no.
; Entry: A the cell, zTemp00 its value.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyCell031F:
    sec
    sbc #$0A
    tax
    sta z:zTemp01
    lsr a
    and #$1C
    bbr2 zTemp01,TallyCell031F_at0
    ora #$20
TallyCell031F_at0:
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
    lda #$54
    clc
    adc z:zTemp08
    sta z:zTemp10
    lda #$BF
    adc z:zTemp09
    sta z:zTemp11
    jsr SetVramAddresses031F
    cly
    bbs0 zTemp01,TallyCell031F_at1
TallyCell031F_at2:
    lda a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    lda (zTemp10),y
    sta a:VDC_DATA_HI
    iny
    cpy #$08
    bne TallyCell031F_at2
    rts
TallyCell031F_at1:
    lda a:VDC_DATA_LO
    lda (zTemp10),y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    iny
    cpy #$08
    bne TallyCell031F_at1
TallyCell031F_at3:
    rts

; @routine 031F:4A0 TallyPause031F
; @run 6B:94A0
; WHAT IT DOES: As TallyPause3CD: after its wait, a wait of $40 and the next step.
; HOW: TallyPause3CD's code with this program's cells, bytes and routines.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyPause031F:
    jsr TallyWait031F
    lda #$40
    sta a:OBJ_TALLY_WAIT
    jmp TallyNext031F

; @routine 031F:54D TallyHealth031F
; @run 6B:954D
; WHAT IT DOES: As TallyHealth3CD: the board's health redrawn.
; HOW: TallyHealth3CD's code with this program's cells, bytes and routines.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyHealth031F:
    jsr MapStageBanks031F
    jsr TallyDigits031F
    ldx #$12
    bsr TallyDigitCell031F
    lda z:zPlayerHealth
    cmp #$0A
    bcc TallyHealth031F_at0
    ldx #$11
    bra TallyDigitCell031F
TallyHealth031F_at0:
    ldx #$40
    ldy #$54
    lda #$01
    jmp ClearVramRows031F

; @routine 031F:56A TallyHearts031F
; @run 6B:956A
; WHAT IT DOES: As TallyHearts3CD: the board's hearts redrawn.
; HOW: TallyHearts3CD's code with this program's cells, bytes and routines.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyHearts031F:
    jsr MapStageBanks031F
    jsr TallyDigits031F
    ldx #$22
    bsr TallyDigitCell031F
    lda z:zField94
    cmp #$10
    bcc TallyHearts031F_at0
    ldx #$21
    bra TallyDigitCell031F
TallyHearts031F_at0:
    ldx #$40
    ldy #$55
    lda #$01
    jmp ClearVramRows031F

; @routine 031F:587 TallyDigitCell031F
; @run 6B:9587
; WHAT IT DOES: As TallyDigitCell3CD: a cell stepped to its end.
; HOW: TallyDigitCell3CD's code with this program's cells, bytes and routines.
; WAITS: no.
; Entry: X the cell.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyDigitCell031F:
    phx
    lda a:StageMaskBuffer,x
    sta z:zTemp00
    txa
    jsr TallyCell031F
    plx
    lda a:StageMaskBuffer,x
    clc
    adc #$40
    bcs TallyDigitCell031F_at0
    sta a:StageMaskBuffer,x
    bra TallyDigitCell031F
TallyDigitCell031F_at0:
    rts

; @routine 031F:537 TallyEnd031F
; @run 6B:9537
; WHAT IT DOES: As TallyEnd3CD: after its wait, the kept health to zField94, zModeDone counted.
; HOW: TallyEnd3CD's code with this program's cells, bytes and routines.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyEnd031F:
    jsr TallyWait031F
    lda a:OBJ_WORK_D+85
    sta z:zField94
    inc z:zModeDone
    rts

; @routine 031F:390 TallyReveal031F
; @run 6B:9390
; WHAT IT DOES: As TallyReveal3CD, but the reveal's end does not set bit 1 of zPaused: one cell of the board revealed a call.
; HOW: TallyReveal3CD's code without its SMB1.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyReveal031F:
    jsr MapStageBanks031F
    ldx #$3D
TallyReveal031F_at1:
    lda a:StageMaskBuffer,x
    bne TallyReveal031F_at0
    dex
    bpl TallyReveal031F_at1
    lda #$40
    sta a:OBJ_TALLY_WAIT
    jmp TallyNext031F
TallyReveal031F_at0:
    jsr VecNextRandom
    and #$3F
    cmp #$3E
    bcs TallyReveal031F_at0
    tax
    lda a:StageMaskBuffer,x
    beq TallyReveal031F_at0
    sta z:zTemp00
    adc #$40
    sta a:StageMaskBuffer,x
    bcc TallyReveal031F_at2
    stz a:StageMaskBuffer,x
TallyReveal031F_at2:
    txa
    and #$3F
    cmp #$0A
    bcs TallyCell031F
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
    lda #$34
    clc
    adc z:zTemp08
    sta z:zTemp10
    lda #$C0
    adc z:zTemp09
    sta z:zTemp11
    jsr SetVramAddressXY031F
    cly
    ldx #$10
TallyReveal031F_at3:
    sxy
    lda (zTemp10),y
    sta a:VDC_DATA_LO
    sxy
    lda (zTemp10),y
    sta a:VDC_DATA_HI
    iny
    inx
    cpy #$10
    bne TallyReveal031F_at3
    rts

; @routine 031F:500 TallyWaitPress031F
; @run 6B:9500
; WHAT IT DOES: After its wait, bit 1 of zPaused set and the paused step run until it clears that bit; then a wait of $20 and the next step.
; HOW: TallyWaitPress3CD with an SMB1, the call to $8EA0 and its own wait byte.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyWaitPress031F:
    jsr TallyWait031F
    smb1 z:zPaused
    jsr StagePausedStep031F
    bbr1 zPaused,TallyWaitPress031F_at0
    rts
TallyWaitPress031F_at0:
    lda #$20
    sta a:OBJ_TALLY_WAIT
    jmp TallyNext031F

; @routine 031F:514 TallyStockBonus031F
; @run 6B:9514
; WHAT IT DOES: After its wait, an extra life (TallyExtraLife031F), the routine at $9103, a wait of $80 and the next step.
; HOW: A BSR, a JSR, a store and a jump.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyStockBonus031F:
    jsr TallyWait031F
    bsr TallyExtraLife031F
    jsr BlendStageStockRows031F
    lda #$80
    sta a:OBJ_TALLY_WAIT
    jmp TallyNext031F

; @routine 031F:524 TallyExtraLife031F
; @run 6B:9524
; WHAT IT DOES: A life more: the player's stock one up in decimal, 99 at most, and sound $48.
; HOW: SED, CLC, ADC, CLD; a store; the sound.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyExtraLife031F:
    sed
    lda z:zStock
    clc
    adc #$01
    bcc TallyExtraLife031F_at0
    lda #$99
TallyExtraLife031F_at0:
    sta z:zStock
    cld
    lda #$48
    jsr VecSoundCommand
    rts

; @routine 031F:542 TallyWait031F
; @run 6B:9542
; WHAT IT DOES: The tally steps' wait: while OBJ_TALLY_WAIT is not 0, one less and the caller's return dropped.
; HOW: TallyWait152D's code on OBJ_TALLY_WAIT.
; WAITS: no.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyWait031F:
    lda a:OBJ_TALLY_WAIT
    beq TallyWait031F_at0
    dec a:OBJ_TALLY_WAIT
    pla
    pla
TallyWait031F_at0:
    rts

; @routine 031F:5FE ClearVramRows031F
; @run 6B:95FE
; WHAT IT DOES: ClearVramRows' bytes (004D:4C8) in this program.
; HOW: ClearVramRows' code, calling the copy of SetVramAddressXY at $9625.
; WAITS: no.
; Entry: A rows, X the address low, Y high.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ClearVramRows031F:
    sta z:zTemp00
    jsr SetVramAddressXY031F
ClearVramRows031F_at1:
    ldx #$40
ClearVramRows031F_at0:
    st1 #$00
    st2 #$00
    dex
    bne ClearVramRows031F_at0
    dec z:zTemp00
    bne ClearVramRows031F_at1
    rts

; @routine 031F:2F1 StageSubModeTally031F
; @run 6B:92F1
; WHAT IT DOES: Sub-mode 6 in the 031D program, the tally after the boss: zWaveCounter counted, the stage's frame, the scroll and objects,
; then the tally's step at zField80 from the eight at $9300.
; HOW: An increment; JSRs to the routines loaded at $8B23 and $8A05; zField80 doubled indexes the table, whose word is jumped
; through.
; WAITS: no.
; Entry: nothing. Exit: as the step leaves them; it is entered with A and X twice zField80, N and Z from it, C its bit 7.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallySteps031F = $9300 ; the tally's step table in this program
StageSubModeTally031F:
    inc z:zWaveCounter
    jsr StepStageFrame031E
    jsr StepScrollAndRunObjects026E
    lda z:zField80
    asl a
    tax
    jmp (TallySteps031F,x)

; @routine 031F:4AB TallyCount031F
; @run 6B:94AB
; WHAT IT DOES: The tally's count: while there is health it is counted into the score (zPlayerHealth down by one, the board's health
; redrawn, 1 point); then after the wait the hearts (zField94 down by one in BCD, redrawn, $10 points). A carry from the
; score gives a life more and puts zStock in StageShownStock. The score's rows are blended, and sound 1 plays every fourth frame. With
; both at 0 the tally goes on two steps with a wait of $C0, or, with the full-health mark at $2ABB set, stays with a wait
; of $40.
; HOW: Decrements; TallyHealth031F, TallyWait031F, TallyHearts031F; the kernel's score add; TallyExtraLife031F by BSR on a
; carry; the routine loaded at $915E.
; WAITS: no.
; Entry: D clear, as the game keeps it. Exit: as the last piece that ran left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TallyCount031F:
    lda z:zPlayerHealth
    beq TallyCount031F_at0
    dec z:zPlayerHealth
    jsr TallyHealth031F
    lda #$01
    bra TallyCount031F_at1
TallyCount031F_at0:
    jsr TallyWait031F
    lda z:zField94
    beq TallyCount031F_at2
    sed
    sec
    sbc #$01
    sta z:zField94
    cld
    jsr TallyHearts031F
    lda #$10
TallyCount031F_at1:
    jsr VecAddScoreByte
    bcc TallyCount031F_at3
    bsr TallyExtraLife031F
    lda z:zStock
    sta a:StageShownStock
TallyCount031F_at3:
    jsr BlendStageScoreRows031F
    lda z:zWaveCounter
    and #$03
    bne TallyCount031F_at4
    lda #$01
    jsr VecSoundCommand
TallyCount031F_at4:
    rts
TallyCount031F_at2:
    lda #$C0
    sta a:OBJ_TALLY_WAIT
    inc z:zField80
    inc z:zField80
    lda a:OBJ_FRAME+83
    beq TallyCount031F_at5
    dec z:zField80
    dec z:zField80
    lda #$40
    sta a:OBJ_TALLY_WAIT
TallyCount031F_at5:
    jmp TallyNext031F
