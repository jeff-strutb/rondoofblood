; @routine 0441:000 ListEntry0441
; @run 6C:8000
; @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as RunAllListChannels.
ListEntry0441:
    jmp RunAllListChannels0441

; @routine 0441:003 EffectEntry0441
; @run 6C:8003
; @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as EffectStep.
EffectEntry0441:
    jmp EffectStep0441

; @routine 0441:006 VramEntry0441
; @run 6C:8006
; @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as VramListRun.
VramEntry0441:
    jmp VramListRun

; @routine 0441:009 RunAllListChannels0441
; @run 6C:8009
; @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 0441, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
; RunListChannel.
; HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
; last channel left them.
RunAllListChannels0441:
    ldx #$07
RunAllListChannels0441_at1:
    lda a:ListChannelKind,x
    beq RunAllListChannels0441_at0
    phx
    bsr RunListChannel0441
    plx
RunAllListChannels0441_at0:
    dex
    bpl RunAllListChannels0441_at1
    rts

; @routine 0651:000 ListEntry0651
; @run 6C:8000
; @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as RunAllListChannels.
ListEntry0651:
    jmp RunAllListChannels0651

; @routine 0651:003 EffectEntry0651
; @run 6C:8003
; @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as EffectStep.
EffectEntry0651:
    jmp EffectStep0651

; @routine 0651:006 VramEntry0651
; @run 6C:8006
; @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as VramListRun.
VramEntry0651:
    jmp VramListRun

; @routine 0651:009 RunAllListChannels0651
; @run 6C:8009
; @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 0651, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
; RunListChannel.
; HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
; last channel left them.
RunAllListChannels0651:
    ldx #$07
RunAllListChannels0651_at1:
    lda a:ListChannelKind,x
    beq RunAllListChannels0651_at0
    phx
    bsr RunListChannel0651
    plx
RunAllListChannels0651_at0:
    dex
    bpl RunAllListChannels0651_at1
    rts

; @routine 1231:000 ListEntry1231
; @run 6C:8000
; @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as RunAllListChannels.
ListEntry1231:
    jmp RunAllListChannels1231

; @routine 1231:003 EffectEntry1231
; @run 6C:8003
; @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as EffectStep.
EffectEntry1231:
    jmp EffectStep1231

; @routine 1231:006 VramEntry1231
; @run 6C:8006
; @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as VramListRun.
VramEntry1231:
    jmp VramListRun

; @routine 1231:009 RunAllListChannels1231
; @run 6C:8009
; @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 1231, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
; RunListChannel.
; HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
; last channel left them.
RunAllListChannels1231:
    ldx #$07
RunAllListChannels1231_at1:
    lda a:ListChannelKind,x
    beq RunAllListChannels1231_at0
    phx
    bsr RunListChannel1231
    plx
RunAllListChannels1231_at0:
    dex
    bpl RunAllListChannels1231_at1
    rts

; @routine 1A31:000 ListEntry1A31
; @run 6C:8000
; @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as RunAllListChannels.
ListEntry1A31:
    jmp RunAllListChannels1A31

; @routine 1A31:003 EffectEntry1A31
; @run 6C:8003
; @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as EffectStep.
EffectEntry1A31:
    jmp EffectStep1A31

; @routine 1A31:006 VramEntry1A31
; @run 6C:8006
; @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as VramListRun.
VramEntry1A31:
    jmp VramListRun

; @routine 1A31:009 RunAllListChannels1A31
; @run 6C:8009
; @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 1A31, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
; RunListChannel.
; HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
; last channel left them.
RunAllListChannels1A31:
    ldx #$07
RunAllListChannels1A31_at1:
    lda a:ListChannelKind,x
    beq RunAllListChannels1A31_at0
    phx
    bsr RunListChannel1A31
    plx
RunAllListChannels1A31_at0:
    dex
    bpl RunAllListChannels1A31_at1
    rts

; @routine 1B91:000 ListEntry1B91
; @run 6C:8000
; @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as RunAllListChannels.
ListEntry1B91:
    jmp RunAllListChannels1B91

; @routine 1B91:003 EffectEntry1B91
; @run 6C:8003
; @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as EffectStep.
EffectEntry1B91:
    jmp EffectStep1B91

; @routine 1B91:006 VramEntry1B91
; @run 6C:8006
; @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as VramListRun.
VramEntry1B91:
    jmp VramListRun

; @routine 1B91:009 RunAllListChannels1B91
; @run 6C:8009
; @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 1B91, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
; RunListChannel.
; HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
; last channel left them.
RunAllListChannels1B91:
    ldx #$07
RunAllListChannels1B91_at1:
    lda a:ListChannelKind,x
    beq RunAllListChannels1B91_at0
    phx
    bsr RunListChannel1B91
    plx
RunAllListChannels1B91_at0:
    dex
    bpl RunAllListChannels1B91_at1
    rts

; @routine 1CF1:000 ListEntry1CF1
; @run 6C:8000
; @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as RunAllListChannels.
ListEntry1CF1:
    jmp RunAllListChannels1CF1

; @routine 1CF1:003 EffectEntry1CF1
; @run 6C:8003
; @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as EffectStep.
EffectEntry1CF1:
    jmp EffectStep1CF1

; @routine 1CF1:006 VramEntry1CF1
; @run 6C:8006
; @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as VramListRun.
VramEntry1CF1:
    jmp VramListRun

; @routine 1CF1:009 RunAllListChannels1CF1
; @run 6C:8009
; @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 1CF1, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
; RunListChannel.
; HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
; last channel left them.
RunAllListChannels1CF1:
    ldx #$07
RunAllListChannels1CF1_at1:
    lda a:ListChannelKind,x
    beq RunAllListChannels1CF1_at0
    phx
    bsr RunListChannel1CF1
    plx
RunAllListChannels1CF1_at0:
    dex
    bpl RunAllListChannels1CF1_at1
    rts

; @routine 25F7:000 ListEntry25F7
; @run 6C:8000
; @unreachable 6C:8000-6C:8002 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8000 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8000: goes to RunAllListChannels.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as RunAllListChannels.
ListEntry25F7:
    jmp RunAllListChannels25F7

; @routine 25F7:003 EffectEntry25F7
; @run 6C:8003
; @unreachable 6C:8003-6C:8005 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8003 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8003: goes to EffectStep, the palette effects' frame.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as EffectStep.
EffectEntry25F7:
    jmp EffectStep25F7

; @routine 25F7:006 VramEntry25F7
; @run 6C:8006
; @unreachable 6C:8006-6C:8008 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8006 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: The overlay's entry at $8006: goes to VramListRun.
; HOW: One jump, no frame of its own.
; WAITS: no.
; Entry, exit: as VramListRun.
VramEntry25F7:
    jmp VramListRun

; @routine 25F7:009 RunAllListChannels25F7
; @run 6C:8009
; @unreachable 6C:8009-6C:8017 no read the game makes loads disc sector 25F7, so these bytes are never in memory.
; Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites included;
; the boot block's three reads; the card's reads of sectors 0 to 2; the old-card program's 20 sectors from $23; the
; five cd_read calls no script runs; LoadSceneScript for every value of A (the records past scene $5E name no sector
; on the disc); SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, to memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8009 runs whatever overlay another scene
; left there, not these bytes.
;
; WHAT IT DOES: Runs the list channels: every channel from 7 down to 0 whose kind is not zero goes through
; RunListChannel.
; HOW: The kind read at $3C80 by X; the channel kept on the stack across the call.
; WAITS: no.
; Entry: nothing. Exit: X $FF, N set, Z clear; A the last kind loaded or what the last channel left; Y, C and V as the
; last channel left them.
RunAllListChannels25F7:
    ldx #$07
RunAllListChannels25F7_at1:
    lda a:ListChannelKind,x
    beq RunAllListChannels25F7_at0
    phx
    bsr RunListChannel25F7
    plx
RunAllListChannels25F7_at0:
    dex
    bpl RunAllListChannels25F7_at1
    rts
