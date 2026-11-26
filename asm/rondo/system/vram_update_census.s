; @routine 0441:142 VramListFromTable0441
; @run 6C:8142
; @stop 8151
; @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 0441, so these bytes are never in
; memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
; included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
; to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
; from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
; left there, not these bytes. The C is optimistic: no script reaches it.
;
; WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
; of lists, at $8E6A, gives its pointer.
; HOW: A doubled indexes the table at $8E6A, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
; WAITS: no.
; Entry: A the list number ($80 or more). Exit: as VramListRun.
; liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_vram_list_0441.txt). The exit shim's last line is not reached there: a case ends in the tail-called
; VramListRun, as VramListFromTableC's does.
VramListFromTable0441:
    asl a
    tay
    lda a:$8E6A,y
    sta z:SpriteFlipMask
    lda a:$8E6B,y
    sta z:zProduct2
    cly
    bra VramListRun

; @routine 0651:142 VramListFromTable0651
; @run 6C:8142
; @stop 8151
; @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 0651, so these bytes are never in
; memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
; included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
; to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
; from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
; left there, not these bytes. The C is optimistic: no script reaches it.
;
; WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
; of lists, at $8E65, gives its pointer.
; HOW: A doubled indexes the table at $8E65, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
; WAITS: no.
; Entry: A the list number ($80 or more). Exit: as VramListRun.
; liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_vram_list_0651.txt). The exit shim's last line is not reached there: a case ends in the tail-called
; VramListRun, as VramListFromTableC's does.
VramListFromTable0651:
    asl a
    tay
    lda a:$8E65,y
    sta z:SpriteFlipMask
    lda a:$8E66,y
    sta z:zProduct2
    cly
    bra VramListRun

; @routine 1231:142 VramListFromTable1231
; @run 6C:8142
; @stop 8151
; @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 1231, so these bytes are never in
; memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
; included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
; to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
; from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
; left there, not these bytes. The C is optimistic: no script reaches it.
;
; WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
; of lists, at $9118, gives its pointer.
; HOW: A doubled indexes the table at $9118, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
; WAITS: no.
; Entry: A the list number ($80 or more). Exit: as VramListRun.
; liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_vram_list_1231.txt). The exit shim's last line is not reached there: a case ends in the tail-called
; VramListRun, as VramListFromTableC's does.
VramListFromTable1231:
    asl a
    tay
    lda a:$9118,y
    sta z:SpriteFlipMask
    lda a:$9119,y
    sta z:zProduct2
    cly
    bra VramListRun

; @routine 1A31:142 VramListFromTable1A31
; @run 6C:8142
; @stop 8151
; @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 1A31, so these bytes are never in
; memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
; included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
; to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
; from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
; left there, not these bytes. The C is optimistic: no script reaches it.
;
; WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
; of lists, at $8ED2, gives its pointer.
; HOW: A doubled indexes the table at $8ED2, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
; WAITS: no.
; Entry: A the list number ($80 or more). Exit: as VramListRun.
; liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_vram_list_1A31.txt). The exit shim's last line is not reached there: a case ends in the tail-called
; VramListRun, as VramListFromTableC's does.
VramListFromTable1A31:
    asl a
    tay
    lda a:$8ED2,y
    sta z:SpriteFlipMask
    lda a:$8ED3,y
    sta z:zProduct2
    cly
    bra VramListRun

; @routine 1B91:142 VramListFromTable1B91
; @run 6C:8142
; @stop 8151
; @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 1B91, so these bytes are never in
; memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
; included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
; to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
; from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
; left there, not these bytes. The C is optimistic: no script reaches it.
;
; WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
; of lists, at $8D82, gives its pointer.
; HOW: A doubled indexes the table at $8D82, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
; WAITS: no.
; Entry: A the list number ($80 or more). Exit: as VramListRun.
; liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_vram_list_1B91.txt). The exit shim's last line is not reached there: a case ends in the tail-called
; VramListRun, as VramListFromTableC's does.
VramListFromTable1B91:
    asl a
    tay
    lda a:$8D82,y
    sta z:SpriteFlipMask
    lda a:$8D83,y
    sta z:zProduct2
    cly
    bra VramListRun

; @routine 25F7:142 VramListFromTable25F7
; @run 6C:8142
; @stop 8151
; @unreachable 6C:8142-6C:8150 no read the game can make loads disc sector 25F7, so these bytes are never in
; memory. Read in full: the 95 scene records at $74D1 and every op of their lists, the reads a later op overwrites
; included; the boot block's three reads ($43 for 10, $4D for 12, $59 for 8 sectors); the card's reads of sectors 0
; to 2; the old-card program's 20 sectors from $23; the five cd_read calls no script runs (sectors 0 to 3 and $23,
; from constants); LoadSceneScript for every value of A (the records past scene $5E name no sector on the disc);
; SceneReadSectors, entered only from the op loop at $7001; the ADPCM loads, which go to ADPCM memory the CPU cannot
; run. No input or poke of work RAM can load the sector, and a unit call at $8142 runs whatever overlay another scene
; left there, not these bytes. The C is optimistic: no script reaches it.
;
; WHAT IT DOES: Runs the VRAM list whose number is in A, for a number of $80 or more: this overlay's second table
; of lists, at $829F, gives its pointer.
; HOW: A doubled indexes the table at $829F, the pointer goes to zp $14, Y 0, and the loop runs on at VramListRun.
; WAITS: no.
; Entry: A the list number ($80 or more). Exit: as VramListRun.
; liftcmp: 402 of 402 cases match its census draft (census_draft.py; callees as filed wrappers; curated
; census_vram_list_25F7.txt). The exit shim's last line is not reached there: a case ends in the tail-called
; VramListRun, as VramListFromTableC's does.
VramListFromTable25F7:
    asl a
    tay
    lda a:$829F,y
    sta z:SpriteFlipMask
    lda a:$82A0,y
    sta z:zProduct2
    cly
    bra VramListRun
