; @routine 00C3:265 FileMenuReadSave
; @run 6B:9265
; WHAT IT DOES: Page 0, step 0: the game state wiped, the backup file read. Read: the menu's music (track $0E) starts,
; the slots' characters and marks are cleared and each slot's progress found, and the step goes on. Not formatted: the
; memory is formatted, then as for no file. No file: an empty file ($90 zero bytes) is written; written, as for read;
; no room, page $0D with ContinueChoice 0. A bad file (2): page $0D with ContinueChoice 1. Any other answer to the
; write: the game stops in a loop that never ends.
; HOW: ResetGameState, BackupRead, its answer through TAX, DEC and INX; BackupFormat; a loop of STZ over SaveBuffer;
; BackupWrite; CdPlayRepeat; SlotLastClearedFlag for zSaveSlot 0, 1 and 2; MenuStepAdvance or SetMenuPage.
; WAITS: yes: in ResetGameState (the vertical blank) and CdPlayRepeat (the drive); the stack there is this routine's
; caller's return and the call's two bytes. A write that fails other than for room waits for ever (BRA to itself).
; Entry: from MenuStepDispatch. Exit, read or written: as MenuStepAdvance, X the third slot's progress, Y, C and V as
; SlotLastClearedFlag leaves them. Page $0D: A 0, N and Z from $0D, X the answer plus one (bad file) or the write's
; answer (no room), Y, C and V as BackupRead or BackupWrite leaves them.
SlotOffsets = $9252 ; the three slots' offsets in SaveBuffer
TextRowAddresses = $A894 ; the VRAM address of each text row of the menu
StartStages = $9574 ; the stage a slot starts on, by its progress
StartStagesHeld = $9581 ; the stages the second pad's held buttons pick instead
PortraitLists = $BC90 ; the character picture's animation lists by its start argument
PortraitFlashLists = $BC98 ; object $34's lists by the picture's character
NameCheat = $97EF ; the name that opens everything on a new slot
DefaultNames = $980D ; the three slots' default names, 8 bytes each
MarkTexts = $AA7B ; a string pointer for each value of a slot's mark
RecordScores = $AD01 ; a two-byte BCD score for each record entry, and one more
EntryOffsets = $B90A ; each record entry's flag byte pair
EntryMasks = $B917 ; each record entry's bit
ClearedMasks = $B993 ; each record entry's cleared bit
EntryPlaces = $BA2F ; the VRAM address of each entry's name
EntryNames = $BA4B ; a string pointer for each entry's name
UnknownName = $BA67 ; the string of an entry not reached
ListByItem = $9964 ; the list a slot page starts, by chosen item (from 1)
KindByItem = $9A30 ; 0 for the slot items, by chosen item
PictureByCharacter = $9A38 ; object $34's frame by character
CursorByItem = $9AFF ; the cursor value a page returns to, by chosen item
FileMenuReadSave:
    jsr VecResetGameState
    jsr VecBackupRead
    tax
    beq FileMenuReadSave_at0
    dec a
    beq FileMenuReadSave_at1
    inx
    beq FileMenuReadSave_at2
    lda #$01
    sta a:ContinueChoice
    lda #$0D
    jmp SetMenuPage
FileMenuReadSave_at0:
    lda #$0E
    jsr VecCdPlayRepeat
    stz a:MenuSlotCharacter
    stz a:MenuSlotCharacter+1
    stz a:MenuSlotCharacter+2
    stz a:$7DDD
    stz a:$7DDE
    stz a:$7DDF
    stz z:zSaveSlot
    jsr SlotLastClearedFlag
    stx a:MenuSlotProgress
    inc z:zSaveSlot
    jsr SlotLastClearedFlag
    stx a:MenuSlotProgress+1
    inc z:zSaveSlot
    jsr SlotLastClearedFlag
    stx a:MenuSlotProgress+2
    jmp MenuStepAdvance
FileMenuReadSave_at2:
    jsr VecBackupFormat
FileMenuReadSave_at1:
    clx
FileMenuReadSave_at3:
    stz a:SaveBuffer,x
    inx
    cpx #$90
    bne FileMenuReadSave_at3
    jsr VecBackupWrite
    tax
    beq FileMenuReadSave_at0
    dec a
    beq FileMenuReadSave_at4
FileMenuReadSave_at5:
    bra FileMenuReadSave_at5
FileMenuReadSave_at4:
    stz a:ContinueChoice
    lda #$0D
    jmp SetMenuPage

; @routine 00C3:2CF FileMenuStartScreen
; @run 6B:92CF
; WHAT IT DOES: Page 0, step 1: every event flag cleared, list $0D started on channel 0, lists 0 and 1 on channels 7
; and 6 as kind 3, and the menu goes on to page 1.
; HOW: EventFlagsClear, StartListChannel0Kind2, StartListChannelKind3 twice, MenuPageAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: A 0, X 6, N and Z from the new page; Y, C and V kept.
FileMenuStartScreen:
    jsr EventFlagsClear
    lda #$0D
    jsr VecStartListChannel0Kind2
    ldx #$07
    cla
    jsr VecStartListChannelKind3
    ldx #$06
    lda #$01
    jsr VecStartListChannelKind3
    jmp MenuPageAdvance

; @routine 00C3:2F5 MenuPickSlot
; @run 6B:92F5
; WHAT IT DOES: The pick of a slot on the pages that choose one: the cursor moves by the pad and shakes; on button I over
; a slot (cursor values 9 to 11) it answers whether the slot has data. Button I over item 8, or no button I, ends the
; caller's step: back to page 8 on button II. Button I over item $0C: back to page 8.
; HOW: CursorMoveByPad (menu 1), JitterCursorObjects, the trigger's bit 0, the cursor value (OBJ->work_d of object 0)
; compared; EventBitTest with the slot number as a tail jump. The ends that leave the step pull the caller's return
; address (two PLA) and go to MenuBackOnButtonII or MenuBackToPageEight, so they return to the caller's caller.
; WAITS: no.
; Entry: nothing. Exit, a slot: as EventBitTest (Z set: no data), V from the slot number's subtract. Otherwise the
; caller's frame is dropped: as MenuBackOnButtonII or MenuBackToPageEight, X, Y, C and V as JitterCursorObjects
; left them (C set by the compare with $0C on its path).
MenuPickSlot:
    ldy #$01
    jsr CursorMoveByPad
    jsr JitterCursorObjects
    lda a:BiosJoyTrigger
    and #$01
    beq MenuPickSlot_at0
    lda a:OBJ_WORK_D
    cmp #$08
    beq MenuPickSlot_at0
    cmp #$0C
    beq MenuPickSlot_at1
    sec
    sbc #$08
    jmp EventBitTest
MenuPickSlot_at1:
    pla
    pla
    bra MenuBackToPageEight
MenuPickSlot_at0:
    pla
    pla

; @routine 00C3:327 FileMenuPickEmpty
; @run 6B:9327
; WHAT IT DOES: Page 3: a slot with no data is picked (one with data is refused), and becomes the selected file; page 9.
; HOW: MenuPickSlot; on Z clear MenuBackOnButtonII; the cursor value less 8 into MenuSelectedSlot; SetMenuPage 9.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuPickSlot's dropped ends, as MenuBackOnButtonII, or A 0, N and Z from 9, C
; and V from the subtract; X and Y as MenuPickSlot left them.
FileMenuPickEmpty:
    jsr MenuPickSlot
    bne MenuBackOnButtonII
    lda a:OBJ_WORK_D
    sec
    sbc #$08
    sta a:MenuSelectedSlot
    lda #$09
    jmp SetMenuPage

; @routine 00C3:33A FileMenuPickUsed
; @run 6B:933A
; WHAT IT DOES: Page 4: a slot with data is picked (an empty one is refused), and becomes the selected file; page 9.
; HOW: MenuPickSlot; on Z set MenuBackOnButtonII; else the end of FileMenuPickEmpty ($932C).
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as FileMenuPickEmpty.
FileMenuPickUsed:
    jsr MenuPickSlot
    beq MenuBackOnButtonII
    bra FileMenuPickEmpty+5

; @routine 00C3:34A FileMenuEraseStart
; @run 6B:934A
; WHAT IT DOES: Page 5, step 0: a slot with data is picked for erasing; its text row is overwritten with eight tiles
; $10FF, sound $3C plays, the erase count is set to $80 frames and the step goes on.
; HOW: MenuPickSlot; on Z set MenuBackOnButtonII. The slot's row address from TEXT_ROW_ADDRESSES (rows 3 to 5) is
; written to the VDC after SelectVramWriteAddress; SelectVramData; eight ST1 $FF, ST2 $10; SoundCommand; ERASE_TIMER;
; MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuPickSlot's dropped ends, as MenuBackOnButtonII, or as MenuStepAdvance
; with Y the row index doubled, X, C and V as SoundCommand leaves them.
FileMenuEraseStart:
    jsr MenuPickSlot
    beq MenuBackOnButtonII
    lda a:OBJ_WORK_D
    sec
    sbc #$09
    asl a
    tay
    jsr VecSelectVramWriteAddress
    lda a:TextRowAddresses+6,y
    sta a:VDC_DATA_LO
    lda a:TextRowAddresses+7,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    ldx #$08
FileMenuEraseStart_at0:
    st1 #$FF
    st2 #$10
    dex
    bne FileMenuEraseStart_at0
    lda #$3C
    jsr VecSoundCommand
    lda #$80
    sta a:MenuPadRepeatBits+4
    jmp MenuStepAdvance

; @routine 00C3:37F FileMenuEraseWait
; @run 6B:937F
; WHAT IT DOES: Page 5, step 1: the cursor shakes while the erase count runs down. Button II stops it: the slot's name
; is drawn back. At zero the slot is erased: its character and mark go to zero, its $30 bytes of the backup file are
; cleared, its event bit is taken out, its row's colours set back and its text row blanked. Either way the menu goes
; back to step 0.
; HOW: JitterCursorObjects; the trigger's bit 1; ERASE_TIMER counted down; the slot number from the cursor value;
; STZ over its bytes; EventBitClear, RecolorTextRun, BlankTextRow; or DrawSlotName; zField80 counted down.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit, counting: N and Z from the count, the rest as JitterCursorObjects left them.
; Otherwise N and Z from the step, A, X, Y, C and V as BlankTextRow or DrawSlotName leaves them.
FileMenuEraseWait:
    jsr JitterCursorObjects
    lda a:BiosJoyTrigger
    and #$02
    bne FileMenuEraseWait_at0
    dec a:MenuPadRepeatBits+4
    bne FileMenuEraseWait_at1
    lda a:OBJ_WORK_D
    sec
    sbc #$08
    tay
    cla
    sta a:MenuSlotProgress-1,y
    sta a:MenuSlotCharacter-1,y
    ldx a:SlotOffsets-1,y
    cly
FileMenuEraseWait_t93A0:
    stz a:SaveBuffer,x
    inx
    iny
    cpy #$30
    bne FileMenuEraseWait_t93A0
    lda a:OBJ_WORK_D
    sec
    sbc #$08
    jsr EventBitClear
    lda a:OBJ_WORK_D
    dec a
    dec a
    cly
    jsr RecolorTextRun
    lda a:OBJ_WORK_D
    sec
    sbc #$06
    jsr BlankTextRow
FileMenuEraseWait_at2:
    dec z:zField80
FileMenuEraseWait_at1:
    rts
FileMenuEraseWait_at0:
    lda a:OBJ_WORK_D
    sec
    sbc #$06
    jsr DrawSlotName
    bra FileMenuEraseWait_at2

; @routine 00C3:3E2 FileMenuCopyPickFrom
; @run 6B:93E2
; WHAT IT DOES: Page 6, step 0: the cursor moves by the pad over the copy page's items; button I over a slot with data
; (cursor values $0E to $10) makes it the slot copied from, draws the copy list for it and goes on a step. Item $11, or
; an empty slot, or no button I: back to page 8 (an empty slot and no press only on button II).
; HOW: CursorMoveByPad (menu 2), the trigger's bit 0, the cursor value compared with $11; less $0D to MenuCopyFrom;
; EventBitTest; StartListChannelKind3 with the value less $0C on channel 0; MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuBackOnButtonII, MenuBackToPageEight (C set from the compare) or
; MenuStepAdvance (X 0, Y the event byte, C and V from the subtract).
FileMenuCopyPickFrom:
    ldy #$02
    jsr CursorMoveByPad
    lda a:BiosJoyTrigger
    and #$01
    bne FileMenuCopyPickFrom_at0
    jmp MenuBackOnButtonII
FileMenuCopyPickFrom_at0:
    lda a:OBJ_WORK_D
    cmp #$11
    bne FileMenuCopyPickFrom_at1
    jmp MenuBackToPageEight
FileMenuCopyPickFrom_at1:
    sec
    sbc #$0D
    sta a:MenuCopyFrom
    jsr EventBitTest
    bne FileMenuCopyPickFrom_at2
    jmp MenuBackOnButtonII
FileMenuCopyPickFrom_at2:
    clx
    lda a:OBJ_WORK_D
    sec
    sbc #$0C
    jsr VecStartListChannelKind3
    jmp MenuStepAdvance

; @routine 00C3:416 FileMenuCopyDrawRows
; @run 6B:9416
; WHAT IT DOES: Page 6, step 1: the slots' rows drawn with the colours swapped, and the step goes on.
; HOW: DrawSlotRows with 1; MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuStepAdvance, the rest as DrawSlotRows leaves them.
FileMenuCopyDrawRows:
    lda #$01
    jsr DrawSlotRows
    jmp MenuStepAdvance

; @routine 00C3:41E FileMenuCopyPickTo
; @run 6B:941E
; WHAT IT DOES: Page 6, step 2: the cursor moves by the pad; button I over an empty slot (cursor values $0E to $10)
; makes it the slot copied to, draws the copy list for it, sets the copy's wait to $80 frames and goes on a step.
; Item $11: back to page 8. A slot with data, or no button I: FileMenuCopyCancel.
; HOW: CursorMoveByPad (menu 2), the trigger's bit 0, the cursor value compared with $11; less $0D to MenuCopyTo;
; EventBitTest; StartListChannelKind3 with the value less 9 on channel 0; OBJ->work_d of object $26 the wait;
; MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as FileMenuCopyCancel, MenuBackToPageEight, or MenuStepAdvance (X 0, Y the
; event byte, C and V from the subtract).
FileMenuCopyPickTo:
    ldy #$02
    jsr CursorMoveByPad
    lda a:BiosJoyTrigger
    and #$01
    beq FileMenuCopyCancel
    lda a:OBJ_WORK_D
    cmp #$11
    bne FileMenuCopyPickTo_at0
    jmp MenuBackToPageEight
FileMenuCopyPickTo_at0:
    sec
    sbc #$0D
    sta a:MenuCopyTo
    jsr EventBitTest
    beq FileMenuCopyPickTo_at1
    jmp FileMenuCopyCancel
FileMenuCopyPickTo_at1:
    clx
    lda a:OBJ_WORK_D
    sec
    sbc #$09
    jsr VecStartListChannelKind3
    lda #$80
    sta a:OBJ_WORK_D+$26
    jmp MenuStepAdvance

; @routine 00C3:454 FileMenuCopyCancel
; @run 6B:9454
; WHAT IT DOES: The end of the copy page's second pick: on button II the copy is given up (the rows drawn again, list
; $17, the page's step 0); otherwise nothing.
; HOW: The trigger's bit 1; DrawSlotRows with 0, StartListChannel0Kind2, SetMenuStep 0 by JSR.
; WAITS: no.
; Entry: from FileMenuCopyPickTo. Exit, no button: A 0, Z set, N clear. Given up: A 0, X 0, N and Z clear (from the
; list start), Y and C as DrawSlotRows leaves them.
FileMenuCopyCancel:
    lda a:BiosJoyTrigger
    and #$02
    beq FileMenuCopyCancel_at0
    cla
    jsr DrawSlotRows
    lda #$17
    jsr VecStartListChannel0Kind2
    cla
    jsr SetMenuStep
FileMenuCopyCancel_at0:
    rts

; @routine 00C3:469 FileMenuCopyMake
; @run 6B:9469
; WHAT IT DOES: Page 6, step 3: when the copy's wait has run out, the slot copied from is copied to the slot copied to:
; its $30 bytes of the backup file, its progress, its character and its mark; list $18 starts on channel 1, the slot
; copied to gets its event bit, and the copy ends as FileMenuCopyCancel's (the rows, list $17, step 0).
; HOW: OBJ->work_d of object $26 counted down; the slots' offsets from SLOT_OFFSETS; zTemp00 counts the bytes; the three
; per-slot bytes; StartListChannelKind2, EventBitSet; the end at $945B.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit, waiting: N and Z from the count; A, X, Y and C kept. Copied: as FileMenuCopyCancel.
FileMenuCopyMake:
    dec a:OBJ_WORK_D+$26
    bne FileMenuCopyCancel+20
    ldy a:MenuCopyFrom
    ldx a:SlotOffsets-1,y
    ldy a:MenuCopyTo
    lda a:SlotOffsets-1,y
    tay
    lda #$30
    sta z:zTemp00
FileMenuCopyMake_at0:
    lda a:SaveBuffer,x
    sta a:SaveBuffer,y
    inx
    iny
    dec z:zTemp00
    bne FileMenuCopyMake_at0
    ldx a:MenuCopyFrom
    ldy a:MenuCopyTo
    lda a:MenuSlotProgress-1,x
    sta a:MenuSlotProgress-1,y
    lda a:MenuSlotCharacter-1,x
    sta a:MenuSlotCharacter-1,y
    lda a:$7DDC,x
    sta a:$7DDC,y
    ldx #$01
    lda #$18
    jsr VecStartListChannelKind2
    lda a:MenuCopyTo
    jsr EventBitSet
    bra FileMenuCopyCancel+7

; @routine 00C3:4B2 FileMenuCharacterPage
; @run 6B:94B2
; WHAT IT DOES: Page 7, the slot's game page: the character objects animate; without button I the cursor moves and
; shakes, now and then one of objects $2F to $32 turns round (every 32 frames, one time in 16), and button II goes back
; to page 8. Button I picks the item: $14 swaps the slot's character (and the picture's), $15 page $0C, $16 and $17
; set and clear SLOT_MARK's first byte and go to page 9, $13 starts the game: Maria on a slot with no progress is
; refused (sound $41); otherwise zCharacter is the slot's, zStage the start stage for its progress (from the other
; table when pads 1 and 3 hold nothing but button I and pad 2 holds $8E), a slot's first game asks for the intro
; scenes (bit 3 of zModeEvents), and the menu goes to page $10. Any other item is taken as no press.
; HOW: AnimateCharacterChoice; the trigger's bit 0; CursorMoveByPad (menu 3), JitterCursorObjects; zFrameTick, two
; NextRandom; OBJ->facing_left EOR 1; MenuBackOnButtonII by JSR. The item compared; MenuSlotCharacter, OBJ->start_arg of
; the picture, OBJ->work_d of $34; SetMenuPage; SoundCommand; START_STAGES; BiosPadState; the slot's byte $24.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as the arm that ran last (MenuBackOnButtonII, SetMenuPage, SoundCommand, or the
; swap: A the character, X 2, N and Z from 2).
FileMenuCharacterPage:
    jsr AnimateCharacterChoice
    lda a:BiosJoyTrigger
    and #$01
    bne FileMenuCharacterPage_at0
FileMenuCharacterPage_at6:
    ldy #$03
    jsr CursorMoveByPad
    jsr JitterCursorObjects
    lda z:zFrameTick
    and #$1F
    bne FileMenuCharacterPage_at1
    jsr VecNextRandom
    and #$0F
    bne FileMenuCharacterPage_at1
    jsr VecNextRandom
    and #$03
    clc
    adc #$2F
    tax
    lda a:OBJ_FACING_LEFT,x
    eor #$01
    sta a:OBJ_FACING_LEFT,x
FileMenuCharacterPage_at1:
    jsr MenuBackOnButtonII
    rts
FileMenuCharacterPage_at0:
    lda a:OBJ_WORK_D
    cmp #$15
    beq FileMenuCharacterPage_at2
    cmp #$16
    beq FileMenuCharacterPage_at3
    cmp #$17
    beq FileMenuCharacterPage_at4
    cmp #$13
    beq FileMenuCharacterPage_at5
    cmp #$14
    bne FileMenuCharacterPage_at6
    ldx z:zSaveSlot
    lda a:MenuSlotCharacter,x
    eor #$01
    sta a:MenuSlotCharacter,x
    sta a:OBJ_START_ARG+$33
    ldx #$02
    stx a:OBJ_WORK_D+$34
    rts
FileMenuCharacterPage_at2:
    lda #$0C
    jmp SetMenuPage
FileMenuCharacterPage_at3:
    lda #$01
    sta a:$7DDC
    lda #$09
    jmp SetMenuPage
FileMenuCharacterPage_at4:
    stz a:$7DDC
    lda #$09
    jmp SetMenuPage
FileMenuCharacterPage_at5:
    ldy z:zSaveSlot
    lda a:MenuSlotCharacter,y
    beq FileMenuCharacterPage_at7
    lda a:MenuSlotProgress,y
    bne FileMenuCharacterPage_at7
    lda #$41
    jmp VecSoundCommand
FileMenuCharacterPage_at7:
    ldy z:zSaveSlot
    lda a:MenuSlotCharacter,y
    sta z:zCharacter
    ldx a:MenuSlotProgress,y
    lda a:StartStages,x
    sta z:zStage
    lda a:BiosPadState
    ora a:BiosPadState+2
    and #$FE
    bne FileMenuCharacterPage_at8
    lda a:BiosPadState+1
    cmp #$8E
    bne FileMenuCharacterPage_at8
    ldx a:MenuSlotProgress,y
    lda a:StartStagesHeld,x
    sta z:zStage
FileMenuCharacterPage_at8:
    ldy z:zSaveSlot
    ldx a:SlotOffsets,y
    lda a:SaveScoreTotal+5,x
    bne FileMenuCharacterPage_at9
    inc a:SaveScoreTotal+5,x
    smb3 z:zModeEvents
FileMenuCharacterPage_at9:
    lda #$10
    jmp SetMenuPage

; @routine 00C3:599 NameEntryStart
; @run 6B:9599
; WHAT IT DOES: The name page's first step: the grid cursor (objects 2 and 3) is emptied, given its picture set and put
; at the middle of the screen, sent to its grid place, the key repeat started afresh, the name buffer filled with
; spaces, the count cleared and the first blank tile drawn; then the step goes on.
; HOW: ClearObject twice, six stores, NameCursorTarget, PadRepeatReset, a loop over MenuName, ShowNameCursor,
; MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: A the new step, N and Z from it, X $FF, Y the count, C and V from ShowNameCursor.
NameEntryStart:
    ldx #$03
    jsr VecClearObject
    ldx #$02
    jsr VecClearObject
    lda #$04
    sta a:OBJ_SPRITE_SET,x
    sta a:OBJ_SPRITE_SET+1,x
    lda #$80
    sta a:OBJ_X_LO,x
    sta a:OBJ_Y_LO,x
    sta a:OBJ_X_LO+1,x
    sta a:OBJ_Y_LO+1,x
    jsr NameCursorTarget
    jsr PadRepeatReset
    lda #$20
    ldx #$07
NameEntryStart_at0:
    sta a:MenuName,x
    dex
    bpl NameEntryStart_at0
    stz a:MenuNameLength
    jsr ShowNameCursor
    jmp MenuStepAdvance

; @routine 00C3:6A3 NameEntryType
; @run 6B:96A3
; WHAT IT DOES: The name page's typing step: the pad with repeat moves the grid cursor (left and right by column,
; except on the last row; up and down by row, five rows), the cursor objects glide and shake. Button I types the
; character (sound $3B; a full name moves the cursor to the last row, keeping the place it came from, and goes on a
; step), or on the last row accepts the name; button II erases the last character, or with an empty name leaves.
; HOW: PadRepeatUpdate; MenuPadRepeatBits shifted out a bit at a time with PHA and PLA; NameCursorTarget on any
; direction; GlideCursorObjects, JitterCursorObjects; NameTypeCharacter, NameBackspace, the ends at $9763 and $97C1.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as the arm that ran last leaves them.
NameEntryType:
    jsr PadRepeatUpdate
    ldx #$02
    lda a:MenuPadRepeatBits
    and #$F0
    beq NameEntryType_at0
    asl a
    bcc NameEntryType_at1
    pha
    lda a:OBJ_WORK_H,x
    cmp #$04
    beq NameEntryType_at2
    lda a:OBJ_WORK_D,x
    dec a
    and #$07
    sta a:OBJ_WORK_D,x
NameEntryType_at2:
    pla
NameEntryType_at1:
    asl a
    bcc NameEntryType_at3
    pha
    lda a:OBJ_WORK_H,x
    inc a
    cmp #$05
    bne NameEntryType_at4
    cla
NameEntryType_at4:
    sta a:OBJ_WORK_H,x
    pla
NameEntryType_at3:
    asl a
    bcc NameEntryType_t96EA
    pha
    lda a:OBJ_WORK_H,x
    cmp #$04
    beq NameEntryType_at5
    lda a:OBJ_WORK_D,x
    inc a
    and #$07
    sta a:OBJ_WORK_D,x
NameEntryType_at5:
    pla
NameEntryType_t96EA:
    asl a
    bcc NameEntryType_at6
    lda a:OBJ_WORK_H,x
    dec a
    bpl NameEntryType_at7
    lda #$04
NameEntryType_at7:
    sta a:OBJ_WORK_H,x
NameEntryType_at6:
    jsr NameCursorTarget
NameEntryType_at0:
    jsr GlideCursorObjects
    jsr JitterCursorObjects
    lda a:BiosJoyTrigger
    and #$01
    beq NameEntryType_at8
    lda a:OBJ_WORK_H+2
    cmp #$04
    beq NameEntryEnd+21
    jsr NameTypeCharacter
    lda #$3B
    jsr VecSoundCommand
    lda a:MenuNameLength
    cmp #$08
    bne NameEntryType_at9
    ldx #$02
    lda a:OBJ_WORK_D,x
    sta a:MenuCopyFrom
    lda a:OBJ_WORK_H,x
    sta a:MenuCopyTo
    lda #$04
    sta a:OBJ_WORK_H,x
    jsr NameCursorTarget
    jmp MenuStepAdvance
NameEntryType_at8:
    lda a:BiosJoyTrigger
    and #$02
    beq NameEntryType_at9
    lda a:OBJ_WORK_H+2
    cmp #$04
    beq NameEntryType_at9
    lda a:MenuNameLength
    beq NameEntryEnd+115
    jsr NameBackspace
NameEntryType_at9:
    rts

; @routine 00C3:74E NameEntryEnd
; @run 6B:974E
; WHAT IT DOES: The name page's last step, with the cursor on the last row: button I accepts the name; button II goes
; back to typing: the cursor returns to the grid place it came from, the last character is dropped and the step goes
; back one.
; HOW: PadRepeatUpdate, GlideCursorObjects, JitterCursorObjects, the trigger's bits 0 and 1 (LSR gives I); the end at
; $9763, or MenuCopyFrom and MenuCopyTo restored, NameCursorTarget, MenuNameLength counted down, ShowNameCursor, zField80
; counted down.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as the arm that ran last leaves them.
NameEntryEnd:
    jsr PadRepeatUpdate
    ldx #$02
    jsr GlideCursorObjects
    jsr JitterCursorObjects
    lda a:BiosJoyTrigger
    and #$03
    beq NameEntryType+170
    lsr a
    bcc NameEntryEnd_at0
    lda a:MenuNameLength
    bne NameEntryEnd_at1
    jsr NameUseDefault
NameEntryEnd_at1:
    ldx a:MenuSelectedSlot
    ldy a:SlotOffsets-1,x
    lda a:MenuChosenItem
    cmp #$05
    beq NameEntryEnd_at2
    lda #$01
    sta a:SaveBuffer+1,y
NameEntryEnd_at2:
    stz z:zBiosAl
    clx
NameEntryEnd_at4:
    lda a:MenuName,x
    sta a:SaveBuffer+$15,y
    cmp a:TileStepsC+2,x
    bne NameEntryEnd_at3
    inc z:zBiosAl
NameEntryEnd_at3:
    iny
    inx
    cpx #$08
    bne NameEntryEnd_at4
    lda z:zBiosAl
    cmp #$08
    bne NameEntryEnd_at5
    ldx a:MenuSelectedSlot
    ldy a:SlotOffsets-1,x
    lda #$C5
    sta a:SaveBuffer+1,y
    lda #$FF
    sta a:SaveBuffer+2,y
    lda #$7F
    sta a:SaveBuffer+3,y
    lda #$45
    sta a:SaveBuffer+4,y
    lda #$01
    sta a:SaveBuffer+5,y
NameEntryEnd_at5:
    lda a:MenuSelectedSlot
    jsr EventBitSet
    jsr DrawUsedSlotRows
    stz a:OBJ_SPRITE_ATTR
    stz a:OBJ_SPRITE_ATTR+1
    ldx #$02
    jsr VecClearObject
    inx
    jsr VecClearObject
    lda #$0A
    jmp SetMenuPage
NameEntryEnd_at0:
    ldx #$02
    lda a:MenuCopyFrom
    sta a:OBJ_WORK_D,x
    lda a:MenuCopyTo
    sta a:OBJ_WORK_H,x
    jsr NameCursorTarget
    dec a:MenuNameLength
    jsr ShowNameCursor
    dec z:zField80
    rts

; @routine 00C3:7F7 NameUseDefault
; @run 6B:97F7
; WHAT IT DOES: Fills the name buffer with the selected slot's default name.
; HOW: Slot less one times eight indexes DEFAULT_NAMES; eight bytes copied.
; WAITS: no.
; Entry: nothing. Exit: A the last character, X 8, Y past it, Z and C set, N clear.
NameUseDefault:
    lda a:MenuSelectedSlot
    dec a
    asl a
    asl a
    asl a
    tay
    clx
NameUseDefault_t9800:
    lda a:DefaultNames,y
    sta a:MenuName,x
    iny
    inx
    cpx #$08
    bne NameUseDefault_t9800
    rts

; @routine 00C4:065 FileMenuListStart
; @run 6B:9865
; WHAT IT DOES: Page 9, step 0: starts the list for the chosen item (list $11 for items 4 and up, else $1A plus the
; page's first mark flag), keeps the scroll's high byte plus one in SCROLL_TARGET, puts the cursor on item 0 and goes
; on a step.
; HOW: StartListChannel0Kind2; SCROLL x_hi plus one; SetCursorValue; JitterCursorObjects; MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
FileMenuListStart:
    lda a:MenuChosenItem
    cmp #$04
    bcc FileMenuListStart_at0
    lda #$11
    bra FileMenuListStart_t9876
FileMenuListStart_at0:
    lda #$1A
    clc
    adc a:$7DDC
FileMenuListStart_t9876:
    jsr VecStartListChannel0Kind2
    lda a:ScrollXHi
    inc a
    sta a:MenuPadRepeatBits+1
    cla
    jsr SetCursorValue
    jsr JitterCursorObjects
    jmp MenuStepAdvance

; @routine 00C4:08A FileMenuListDraw
; @run 6B:988A
; WHAT IT DOES: Page 9, step 1: draws what the chosen item shows: items 4 and up the selected slot's number (object
; list 4); else the slot's mark (list 6), or, with the first mark flag set, the item number, the slot's name, its
; score and its record list (list 7). Then the scroll's step is cleared and the step goes on.
; HOW: DrawSelectedFileDigit, DrawSlotMark, or DrawChosenItemDigit, DrawSlotName at the slot plus 12, DrawSlotScore,
; the cursor object's line and page zeroed and DrawRecordList; PlaceObjectList; two stores to the scroll step;
; JitterCursorObjects; MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
FileMenuListDraw:
    lda a:MenuChosenItem
    cmp #$04
    bcc FileMenuListDraw_at0
    jsr DrawSelectedFileDigit
    lda #$04
    bra FileMenuListDraw_t98BD
FileMenuListDraw_at0:
    lda a:$7DDC
    bne FileMenuListDraw_at1
    jsr DrawSlotMark
    lda #$06
    bra FileMenuListDraw_t98BD
FileMenuListDraw_at1:
    jsr DrawChosenItemDigit
    lda z:zSaveSlot
    clc
    adc #$0C
    jsr DrawSlotName
    jsr DrawSlotScore
    stz a:OBJ_WORK_D
    stz a:OBJ_WORK_H
    jsr DrawRecordList
    lda #$07
FileMenuListDraw_t98BD:
    jsr PlaceObjectList
    stz a:ScrollStepXLo
    stz a:ScrollStepXHi
    jsr JitterCursorObjects
    jmp MenuStepAdvance

; @routine 00C4:0F1 DrawChosenItemDigit
; @run 6B:98F1
; WHAT IT DOES: Draws MenuChosenItem as a digit tile at the screen place $01C7.
; HOW: Sets the VDC's write address and runs into DrawCursorRowDigit's second ST ($98E7, the high byte $01).
; WAITS: no.
; Entry: nothing. Exit: as DrawSelectedFileDigit.
DrawChosenItemDigit:
    jsr VecSelectVramWriteAddress
    st1 #$C7
    bra DrawCursorRowDigit+5

; @routine 00C4:0F8 FileMenuListLeave
; @run 6B:98F8
; WHAT IT DOES: Page 9, step 3: where the chosen item leads: items 4 and up to page $0B (cursor on item $18, objects 0
; and 1 given attribute $88); else page $0E (cursor on item $1C, $7DDA cleared) or, with the first mark flag set, page
; $0F (cursor on item $1D).
; HOW: JitterCursorObjects, SetCursorValue, the stores, SetMenuPage.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: A 0, N and Z from the page, X 1, Y and C as SetCursorValue leaves them.
FileMenuListLeave:
    jsr JitterCursorObjects
    lda a:MenuChosenItem
    cmp #$04
    bcc FileMenuListLeave_t9914
    lda #$18
    jsr SetCursorValue
    lda #$88
    sta a:OBJ_SPRITE_ATTR
    sta a:OBJ_SPRITE_ATTR+1
    lda #$0B
    jmp SetMenuPage
FileMenuListLeave_t9914:
    lda a:$7DDC
    bne FileMenuListLeave_at0
    lda #$1C
    jsr SetCursorValue
    stz a:$7DDA
    lda #$0E
    jmp SetMenuPage
FileMenuListLeave_at0:
    lda #$1D
    jsr SetCursorValue
    lda #$0F
    jmp SetMenuPage

; @routine 00C4:13D FileMenuSlideStart
; @run 6B:993D
; WHAT IT DOES: The first step of the page that slides the screen on: starts the item's list (LIST_BY_ITEM; where that
; is 0, list $13 when the slot has data and $12 when not), keeps the scroll's high byte plus one as the target, puts
; the cursor on item 0 and goes on.
; HOW: LIST_BY_ITEM, the slot's first byte shifted right; StartListChannel0Kind2; SetCursorValue; JitterCursorObjects;
; MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
FileMenuSlideStart:
    ldy a:MenuChosenItem
    lda a:ListByItem,y
    bne FileMenuSlideStart_at0
    ldx a:SlotOffsets-1,y
    lda a:SaveBuffer,x
    lsr a
    lda #$13
    bcs FileMenuSlideStart_at0
    dec a
FileMenuSlideStart_at0:
    jsr VecStartListChannel0Kind2
    lda a:ScrollXHi
    inc a
    sta a:MenuPadRepeatBits+1
    cla
    jsr SetCursorValue
    jsr JitterCursorObjects
    jmp MenuStepAdvance

; @routine 00C4:16C FileMenuSlideDraw
; @run 6B:996C
; WHAT IT DOES: Draws the next screen of the chosen item off to the side: item 7 the copy rows, items 4 to 6 their list
; ($14 to $16) and the used slots' rows, items 1 to 3 the slot (its number made zSaveSlot): list $1C when its byte $01
; has no bit 2, its details, name and percentage; then the item's objects (list 3 or 5 for a slot with and without
; data, or the item's kind), and the slot's character picture and marks. The scroll's step is cleared and the step
; goes on.
; HOW: StartListChannel0Kind2, DrawUsedSlotRows, DrawSlotRows, DrawSlotDetails, DrawCursorRowDigit, DrawSlotName,
; DrawSlotPercent, PlaceObjectList; the object frames of $2B and $2F to $34 and the start arguments of $33 and $34.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
FileMenuSlideDraw:
    stz a:$7DDB
    lda a:MenuChosenItem
    cmp #$07
    beq FileMenuSlideDraw_at0
    cmp #$04
    bcc FileMenuSlideDraw_at1
    clc
    adc #$10
    jsr VecStartListChannel0Kind2
    jsr DrawUsedSlotRows
    bra FileMenuSlideDraw_at2
FileMenuSlideDraw_at0:
    cla
    jsr DrawSlotRows
    bra FileMenuSlideDraw_at2
FileMenuSlideDraw_at1:
    ldy a:MenuChosenItem
    dey
    ldx a:SlotOffsets,y
    lda a:SaveBuffer+1,x
    and #$04
    bne FileMenuSlideDraw_at2
    lda #$1C
    jsr VecStartListChannel0Kind2
    inc a:$7DDB
FileMenuSlideDraw_at2:
    lda a:MenuChosenItem
    cmp #$04
    bcs FileMenuSlideDraw_at3
    lda a:MenuChosenItem
    dec a
    sta z:zSaveSlot
    jsr DrawSlotDetails
    jsr DrawCursorRowDigit
    lda z:zSaveSlot
    clc
    adc #$06
    jsr DrawSlotName
    lda z:zSaveSlot
    clc
    adc #$06
    jsr DrawSlotPercent
FileMenuSlideDraw_at3:
    ldy a:MenuChosenItem
    lda a:KindByItem,y
    bne FileMenuSlideDraw_at4
    ldx a:SlotOffsets-1,y
    lda a:SaveBuffer,x
    lsr a
    lda #$03
    bcs FileMenuSlideDraw_at4
    lda #$05
FileMenuSlideDraw_at4:
    jsr PlaceObjectList
    lda a:$7DDB
    beq FileMenuSlideDraw_at5
    stz a:OBJ_FRAME+$2B
FileMenuSlideDraw_at5:
    ldy a:MenuChosenItem
    lda a:KindByItem,y
    bne FileMenuSlideDraw_at6
    lda a:MenuSlotCharacter-1,y
    sta a:OBJ_START_ARG+$33
    sta a:OBJ_WORK_D+$33
    tax
    phx
    ldx a:SlotOffsets-1,y
    lda a:SaveBuffer,x
    plx
    lsr a
    bcc FileMenuSlideDraw_at7
    lda a:PictureByCharacter,x
    sta a:OBJ_FRAME+$34
FileMenuSlideDraw_at7:
    ldx a:SlotOffsets-1,y
    lda a:SaveBuffer,x
    lsr a
    bcs FileMenuSlideDraw_at8
    stz a:OBJ_FRAME+$31
FileMenuSlideDraw_at8:
    lsr a
    bcs FileMenuSlideDraw_at9
    stz a:OBJ_FRAME+$32
FileMenuSlideDraw_at9:
    lsr a
    bcs FileMenuSlideDraw_at10
    stz a:OBJ_FRAME+$2F
FileMenuSlideDraw_at10:
    lsr a
    bcs FileMenuSlideDraw_at6
    stz a:OBJ_FRAME+$30
FileMenuSlideDraw_at6:
    stz a:ScrollStepXLo
    stz a:ScrollStepXHi
    jsr JitterCursorObjects
    jmp MenuStepAdvance

; @routine 00C4:23A FileMenuSlideLeft
; @run 6B:9A3A
; WHAT IT DOES: Slides the screen to the next page a frame: the step along X ramps by $40 a frame (up while the
; scroll's middle byte is positive, down after), the 24-bit scroll moves by it, and at the target page the scroll's low
; bytes are zeroed and the step goes on. The objects move with the scroll.
; HOW: AnimateCharacterChoice, JitterCursorObjects; the ramp; SCROLL x_frac, x_lo and x_hi added (x_lo and x_hi kept
; in the speed bytes); x_hi compared with SCROLL_TARGET; ShiftObjectsByScrollChange; MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: not there: N, Z and C from the compare with the target; there: as
; MenuStepAdvance.
FileMenuSlideLeft:
    jsr AnimateCharacterChoice
    jsr JitterCursorObjects
    lda a:ScrollXLo
    bmi FileMenuSlideLeft_at0
    lda a:ScrollStepXLo
    clc
    adc #$40
    sta a:ScrollStepXLo
    bcc FileMenuSlideLeft_at1
    inc a:ScrollStepXHi
FileMenuSlideLeft_at1:
    bra FileMenuSlideLeft_at2
FileMenuSlideLeft_at0:
    lda a:ScrollStepXLo
    sec
    sbc #$40
    sta a:ScrollStepXLo
    bcs FileMenuSlideLeft_at2
    inc a:ScrollStepXHi
FileMenuSlideLeft_at2:
    lda a:ScrollXFrac
    clc
    adc a:ScrollStepXLo
    sta a:ScrollXFrac
    lda a:ScrollXLo
    sta a:ScrollSpeedX
    adc a:ScrollStepXHi
    sta a:ScrollXLo
    lda a:ScrollXHi
    sta a:ScrollSpeedY
    adc #$00
    sta a:ScrollXHi
    cmp a:MenuPadRepeatBits+1
    bne FileMenuSlideLeft_at3
    stz a:ScrollXLo
    stz a:ScrollXFrac
FileMenuSlideLeft_at3:
    jsr ShiftObjectsByScrollChange
    lda a:ScrollXHi
    cmp a:MenuPadRepeatBits+1
    bne FileMenuSlideLeft_at4
    jmp MenuStepAdvance
FileMenuSlideLeft_at4:
    rts

; @routine 00C4:29E FileMenuSlideEnd
; @run 6B:9A9E
; WHAT IT DOES: After the slide: puts the cursor where the item's page has it (CURSOR_BY_ITEM; for value $13 as it is,
; else moved down to the first slot with data, or for item 4 to the first empty one) and goes to the item's page.
; HOW: AnimateCharacterChoice; the table; EventBitTest on slots 1 and 2, counted in zTemp00; SetCursorValue; runs into
; FileMenuReturnPage.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as FileMenuReturnPage.
FileMenuSlideEnd:
    jsr AnimateCharacterChoice
    ldy a:MenuChosenItem
    lda a:CursorByItem,y
    cmp #$13
    beq FileMenuSlideEnd_at0
    cpy #$04
    beq FileMenuSlideEnd_at1
    lda #$01
    sta z:zTemp00
    jsr EventBitTest
    bne FileMenuSlideEnd_at2
    inc z:zTemp00
    lda z:zTemp00
    jsr EventBitTest
    bne FileMenuSlideEnd_at2
    inc z:zTemp00
FileMenuSlideEnd_at2:
    ldy a:MenuChosenItem
    lda a:CursorByItem,y
    dec a
    clc
    adc z:zTemp00
    bra FileMenuSlideEnd_at0
FileMenuSlideEnd_at1:
    lda #$01
    sta z:zTemp00
    jsr EventBitTest
    beq FileMenuSlideEnd_at3
    inc z:zTemp00
    lda z:zTemp00
    jsr EventBitTest
    beq FileMenuSlideEnd_at3
    inc z:zTemp00
FileMenuSlideEnd_at3:
    ldy a:MenuChosenItem
    lda a:CursorByItem,y
    dec a
    clc
    adc z:zTemp00
FileMenuSlideEnd_at0:
    jsr SetCursorValue

; @routine 00C4:2F0 FileMenuReturnPage
; @run 6B:9AF0
; WHAT IT DOES: Goes to the page an item returns to: page 7 for the slots (items 1 to 3), else the page the item less
; one.
; HOW: JitterCursorObjects, MenuChosenItem compared with 4, DEC, SetMenuPage.
; WAITS: no.
; Entry: nothing. Exit: A 0, N and Z from the page, C from the compare; the rest as JitterCursorObjects leaves them.
FileMenuReturnPage:
    jsr JitterCursorObjects
    lda a:MenuChosenItem
    cmp #$04
    bcs FileMenuReturnPage_at0
    lda #$08
FileMenuReturnPage_at0:
    dec a
    jmp SetMenuPage

; @routine 00C4:312 FileMenuBackStart
; @run 6B:9B12
; WHAT IT DOES: Leaves a slot page without sliding: list $0F, the cursor on item $17 or $16 by the first mark flag
; (slots), or on the selected slot plus 8, and the item's page.
; HOW: StartListChannel0Kind2; SetCursorValue; a tail jump to FileMenuReturnPage.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as FileMenuReturnPage.
FileMenuBackStart:
    lda #$0F
    jsr VecStartListChannel0Kind2
    lda a:MenuChosenItem
    cmp #$04
    bcs FileMenuBackStart_at0
    lda a:$7DDC
    bne FileMenuBackStart_at1
    lda #$17
    bra FileMenuBackStart_at2
FileMenuBackStart_at1:
    lda #$16
    bra FileMenuBackStart_at2
FileMenuBackStart_at0:
    lda a:MenuSelectedSlot
    clc
    adc #$08
FileMenuBackStart_at2:
    jsr SetCursorValue
    jmp FileMenuReturnPage

; @routine 00C4:342 FileMenuSlidePrepare
; @run 6B:9B42
; WHAT IT DOES: The first step of the page that slides back to the opening page: the character objects animate and the
; slot list is drawn; then, as FileMenuSlideBackStart, which it runs into.
; HOW: AnimateCharacterChoice, DrawSlotMenu, the bytes of FileMenuSlideBackStart.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
FileMenuSlidePrepare:
    jsr AnimateCharacterChoice
    jsr DrawSlotMenu

; @routine 00C4:348 FileMenuSlideBackStart
; @run 6B:9B48
; WHAT IT DOES: Starts the slide back: the scroll's step and the distance travelled cleared, the target the page the
; scroll is on, the cursor on item 0, and the step goes on.
; HOW: Five STZ, SCROLL x_hi to SCROLL_TARGET, SetCursorValue, JitterCursorObjects, MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
FileMenuSlideBackStart:
    stz a:ScrollStepXLo
    stz a:ScrollStepXHi
    stz a:MenuPadRepeatBits+4
    stz a:MenuPadRepeatBits+3
    stz a:MenuPadRepeatBits+2
    lda a:ScrollXHi
    sta a:MenuPadRepeatBits+1
    cla
    jsr SetCursorValue
    jsr JitterCursorObjects
    jmp MenuStepAdvance

; @routine 00C4:367 FileMenuSlideBack
; @run 6B:9B67
; WHAT IT DOES: Slides the screen back a page a frame: the step ramps by $40 (up while the distance's middle byte is
; positive, down after) and adds to the 24-bit distance, cleared to a whole page once it reaches one; the scroll is the
; target less the distance; at a whole page the step goes on. The objects move with the scroll.
; HOW: AnimateCharacterChoice, JitterCursorObjects; the ramp; the distance; SCROLL x_lo and x_hi kept in the speed
; bytes; the scroll as a subtraction; ShiftObjectsByScrollChange; MenuStepAdvance when the distance's high byte is set.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: not there: A 0, Z set; there: as MenuStepAdvance.
FileMenuSlideBack:
    jsr AnimateCharacterChoice
    jsr JitterCursorObjects
    lda a:MenuPadRepeatBits+3
    bmi FileMenuSlideBack_at0
    lda a:ScrollStepXLo
    clc
    adc #$40
    sta a:ScrollStepXLo
    bcc FileMenuSlideBack_at1
    inc a:ScrollStepXHi
FileMenuSlideBack_at1:
    bra FileMenuSlideBack_t9B90
FileMenuSlideBack_at0:
    lda a:ScrollStepXLo
    sec
    sbc #$40
    sta a:ScrollStepXLo
    bcs FileMenuSlideBack_t9B90
    inc a:ScrollStepXHi
FileMenuSlideBack_t9B90:
    lda a:MenuPadRepeatBits+4
    clc
    adc a:ScrollStepXLo
    sta a:MenuPadRepeatBits+4
    lda a:MenuPadRepeatBits+3
    adc a:ScrollStepXHi
    sta a:MenuPadRepeatBits+3
    lda a:MenuPadRepeatBits+2
    adc #$00
    sta a:MenuPadRepeatBits+2
    beq FileMenuSlideBack_at2
    stz a:MenuPadRepeatBits+3
    stz a:MenuPadRepeatBits+4
FileMenuSlideBack_at2:
    lda a:ScrollXLo
    sta a:ScrollSpeedX
    lda a:ScrollXHi
    sta a:ScrollSpeedY
    cla
    sec
    sbc a:MenuPadRepeatBits+4
    sta a:ScrollXFrac
    cla
    sbc a:MenuPadRepeatBits+3
    sta a:ScrollXLo
    lda a:MenuPadRepeatBits+1
    sbc a:MenuPadRepeatBits+2
    sta a:ScrollXHi
    jsr ShiftObjectsByScrollChange
    lda a:MenuPadRepeatBits+2
    beq FileMenuSlideBack_at3
    jmp MenuStepAdvance
FileMenuSlideBack_at3:
    rts

; @routine 00C4:3E3 FileMenuBackToStart
; @run 6B:9BE3
; WHAT IT DOES: After the slide back to the first page: the cursor on the item chosen, page 1 at its step 2.
; HOW: AnimateCharacterChoice, SetCursorValue, SetMenuPage by JSR, then SetMenuStep.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: A 2, N and Z clear, X 1, Y and C as SetCursorValue leaves them.
FileMenuBackToStart:
    jsr AnimateCharacterChoice
    lda a:MenuChosenItem
    jsr SetCursorValue
    lda #$01
    jsr SetMenuPage
    lda #$02
    jmp SetMenuStep

; @routine 00C4:42F FileMenuOpenStart
; @run 6B:9C2F
; WHAT IT DOES: The opening page's first step: list $0F, the objects of object list 0, the cursor objects put on item 1,
; and the step goes on.
; HOW: StartListChannel0Kind2, PlaceObjectList, PlaceCursorObjects, MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
FileMenuOpenStart:
    lda #$0F
    jsr VecStartListChannel0Kind2
    cla
    jsr PlaceObjectList
    lda #$01
    jsr PlaceCursorObjects
    jmp MenuStepAdvance

; @routine 00C4:440 FileMenuOpenSlots
; @run 6B:9C40
; WHAT IT DOES: The opening page's second step: script $0E, the slot list drawn, and the step goes on.
; HOW: StartScriptSlot0Kind1, DrawSlotMenu, MenuStepAdvance.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
FileMenuOpenSlots:
    lda #$0E
    jsr VecStartScriptSlot0Kind1
    jsr DrawSlotMenu
    jmp MenuStepAdvance

; @routine 00C4:44B FileMenuOpenPick
; @run 6B:9C4B
; WHAT IT DOES: The opening page's pick: button I on an item whose event is set makes it MenuChosenItem and goes to
; the next page; otherwise the cursor moves by the pad and shakes.
; HOW: The trigger's bit 0, EventBitTest, MenuPageAdvance; or CursorMoveByPad (menu 0) and a tail jump to
; JitterCursorObjects.
; WAITS: no.
; Entry: from MenuStepDispatch. Exit: as MenuPageAdvance, or as JitterCursorObjects.
FileMenuOpenPick:
    lda a:BiosJoyTrigger
    and #$01
    beq FileMenuOpenPick_at0
    lda a:OBJ_WORK_D
    jsr EventBitTest
    beq FileMenuOpenPick_at0
    lda a:OBJ_WORK_D
    sta a:MenuChosenItem
    jmp MenuPageAdvance
FileMenuOpenPick_at0:
    cly
    jsr CursorMoveByPad
    jmp JitterCursorObjects

; @routine 00DD:6D0 DrawSlotMenu
; @run 6F:A6D0
; WHAT IT DOES: Draws the slot list and its commands: each of the three slots with data gets its event, name and
; percentage, one without has its event cleared and its row blanked, and the row is coloured by it. Then the command
; rows: with no slot used, only item 3 is offered and items 4 to 6 are greyed with their events cleared; with one or
; two used, items 4, 5, 6 and 3 are offered; with all three, items 4 and 5 are offered and 3 is greyed.
; HOW: bios_b low counts the slots with data (bit 0 of each slot's byte $01); per slot EventBitSet or EventBitClear,
; DrawSlotName or BlankTextRow, DrawSlotPercent, RecolorTextRun; then the commands.
; WAITS: no.
; Entry: nothing. Exit: as the last EventBitSet or EventBitClear.
DrawSlotMenu:
    stz z:zBiosBl
    clx
DrawSlotMenu_at2:
    ldy a:SlotOffsets,x
    lda a:SaveBuffer+1,y
    and #$01
    tay
    phy
    bne DrawSlotMenu_at0
    txa
    inc a
    jsr EventBitClear
    txa
    jsr BlankTextRow
    bra DrawSlotMenu_at1
DrawSlotMenu_at0:
    txa
    inc a
    jsr EventBitSet
    txa
    jsr DrawSlotName
    inc z:zBiosBl
DrawSlotMenu_at1:
    txa
    jsr DrawSlotPercent
    ply
    txa
    jsr RecolorTextRun
    inx
    cpx #$03
    bne DrawSlotMenu_at2
    lda z:zBiosBl
    beq DrawSlotMenuGreyed+13
    ldy #$01
    lda #$04
    jsr RecolorTextRun
    lda #$05
    jsr EventBitSet
    ldy #$01
    lda #$05
    jsr RecolorTextRun
    lda #$06
    jsr EventBitSet
    lda z:zBiosBl
    cmp #$03
    beq DrawSlotMenuGreyed
    ldy #$01
    lda #$06
    jsr RecolorTextRun
    lda #$07
    jsr EventBitSet

; @routine 00DD:731 OfferSlotCommands
; @run 6F:A731
; WHAT IT DOES: Draws menu item 3 in palette 1 and makes event 4 pickable.
; HOW: RecolorTextRun with Y 1 on row 3; EventBitSet 4 as a tail.
; WAITS: no.
; Entry: nothing. Exit: as EventBitSet; Y the event's byte.
OfferSlotCommands:
    ldy #$01
    lda #$03
    jsr RecolorTextRun
    lda #$04
    jmp EventBitSet

; @routine 00DD:73D DrawSlotMenuGreyed
; @run 6F:A73D
; WHAT IT DOES: DrawSlotMenu's two arms that grey commands: with three slots used ($A73D), item 3 greyed and event 4
; cleared; with none ($A74A), OfferSlotCommands first and then items 4 and 5 greyed and events 5 and 6 cleared. Both
; end greying item 6 and clearing event 7.
; HOW: RecolorTextRun with palette 0, EventBitClear; the last as a tail jump.
; WAITS: no.
; Entry: branched to from DrawSlotMenu. Exit: as EventBitClear.
DrawSlotMenuGreyed:
    cly
    lda #$03
    jsr RecolorTextRun
    lda #$04
    jsr EventBitClear
    bra DrawSlotMenuGreyed_at0
    bsr OfferSlotCommands
    cly
    lda #$04
    jsr RecolorTextRun
    lda #$05
    jsr EventBitClear
    cly
    lda #$05
    jsr RecolorTextRun
    lda #$06
    jsr EventBitClear
DrawSlotMenuGreyed_at0:
    cly
    lda #$06
    jsr RecolorTextRun
    lda #$07
    jmp EventBitClear

; @routine 00DD:76D DrawUsedSlotRows
; @run 6F:A76D
; WHAT IT DOES: Draws the three slots' rows of the slot list: a slot with data gets its name; each row is coloured
; with palette 0 or 1 by whether it has data, swapped unless the page row (MenuChosenItem) is 4.
; HOW: zTemp01 0 or 1 from MenuChosenItem; zTemp00 counted down from 3; EventBitTest; DrawSlotName at the slot plus 2;
; RecolorTextRun at the slot plus 6.
; WAITS: no.
; Entry: nothing. Exit: as RecolorTextRun leaves the last row, Z set, N clear (the count).
DrawUsedSlotRows:
    stz z:zTemp01
    lda a:MenuChosenItem
    cmp #$04
    beq DrawUsedSlotRows_at0
    inc z:zTemp01
DrawUsedSlotRows_at0:
    lda #$03
    sta z:zTemp00
DrawUsedSlotRows_at3:
    lda z:zTemp00
    jsr EventBitTest
    beq DrawUsedSlotRows_at1
    lda z:zTemp00
    clc
    adc #$02
    jsr DrawSlotName
    cla
    bra DrawUsedSlotRows_at2
DrawUsedSlotRows_at1:
    lda #$01
DrawUsedSlotRows_at2:
    eor z:zTemp01
    tay
    lda z:zTemp00
    clc
    adc #$06
    jsr RecolorTextRun
    dec z:zTemp00
    bne DrawUsedSlotRows_at3
    rts

; @routine 00DD:7A0 DrawSlotRows
; @run 6F:A7A0
; WHAT IT DOES: Draws the three slots' rows of the copy page, from the third up: a slot with data gets its name and its
; percentage; each row is coloured with palette 1 or 0 by whether the slot has data, swapped when the argument is 1.
; HOW: zTemp01 the argument, zTemp00 the slot counted down from 3; EventBitTest; DrawSlotName and DrawSlotPercent at
; the slot plus 8; RecolorTextRun at the slot plus 9 with the data bit EOR the argument.
; WAITS: no.
; Entry: A 0 or 1. Exit: A $30, Y and C as RecolorTextRun leaves them, Z set, N clear (the last count); X kept.
DrawSlotRows:
    sta z:zTemp01
    lda #$03
    sta z:zTemp00
DrawSlotRows_at2:
    lda z:zTemp00
    jsr EventBitTest
    beq DrawSlotRows_at0
    lda z:zTemp00
    clc
    adc #$08
    jsr DrawSlotName
    lda z:zTemp00
    clc
    adc #$08
    jsr DrawSlotPercent
    lda #$01
    bra DrawSlotRows_at1
DrawSlotRows_at0:
    cla
DrawSlotRows_at1:
    eor z:zTemp01
    tay
    lda z:zTemp00
    clc
    adc #$09
    jsr RecolorTextRun
    dec z:zTemp00
    bne DrawSlotRows_at2
    rts

; @routine 00DE:11D DrawSlotMark
; @run 6F:A91D
; WHAT IT DOES: Draws the selected slot's SLOT_MARK plus one as a BCD number at $02C4, and the mark's string.
; HOW: The mark plus one, adjusted past 9 by subtracting $FA with the carry set; WriteHexByteTiles; the string pointer
; from MARK_TEXTS in zTemp00 and zTemp01; DrawGlyphText.
; WAITS: no.
; Entry: nothing. Exit: as DrawGlyphText leaves them.
DrawSlotMark:
    jsr VecSelectVramWriteAddress
    st1 #$C4
    st2 #$02
    jsr VecSelectVramData
    ldy z:zSaveSlot
    lda a:$7DDD,y
    inc a
    cmp #$0A
    bcc DrawSlotMark_at0
    sbc #$FA
DrawSlotMark_at0:
    bsr WriteHexByteTiles
    ldy z:zSaveSlot
    lda a:$7DDD,y
    asl a
    tax
    lda a:MarkTexts,x
    sta z:zTemp00
    lda a:MarkTexts+1,x
    sta z:zTemp01
    jsr DrawGlyphText
    rts

; @routine 00DE:14A DrawSlotScore
; @run 6F:A94A
; WHAT IT DOES: Draws the selected slot's total score, six digits, at $01D4.
; HOW: Sets the VDC's write address and runs into DrawSlotDetails at $A95C: the score's three bytes, high first,
; through WriteHexByteTiles, the last as a tail.
; WAITS: no.
; Entry: nothing. Exit: A the last digit tile, N and Z clear, C bit 3 of the low score byte, X the slot's offset, Y the
; slot.
DrawSlotScore:
    jsr VecSelectVramWriteAddress
    st1 #$D4
    st2 #$01
    bra DrawSlotDetails+9

; @routine 00DE:463 SlotScoreReaches
; @run 6F:AC63
; WHAT IT DOES: Tells whether the selected slot's total score reaches the score of entry Y/2 in RECORD_SCORES.
; HOW: PHX, PHY, SXY (X becomes the entry's byte index); the slot's offset; the score's third byte non-zero answers
; yes; else the second and first bytes compared with the entry's high and low. PLY and PLX.
; WAITS: no.
; Entry: Y the entry times two. Exit: C set when reached; A the last byte read; X and Y kept, N and Z from X.
SlotScoreReaches:
    phx
    phy
    sxy
    ldy z:zSaveSlot
    lda a:SlotOffsets,y
    tay
    lda a:SaveScoreTotal+2,y
    bne SlotScoreReaches_at0
    lda a:SaveScoreTotal+1,y
    cmp a:RecordScores+1,x
    beq SlotScoreReaches_at1
    bcs SlotScoreReaches_at0
    bra SlotScoreReaches_at2
SlotScoreReaches_at1:
    lda a:SaveScoreTotal,y
    cmp a:RecordScores,x
    bcs SlotScoreReaches_at0
SlotScoreReaches_at2:
    ply
    plx
    clc
    rts
SlotScoreReaches_at0:
    ply
    plx
    sec
    rts

; @routine 00DE:48D DrawSlotRecord
; @run 6F:AC8D
; WHAT IT DOES: Draws the score line at $0556 for the entry the cursor is on: the object's page (OBJ->work_h) plus 7
; when its line (OBJ->work_d) is not 0, or 14 for page 7. An entry the slot has cleared (its bit in bytes $22 and
; $23) shows "00" and the score at index 14; else the entry's score, bright when the slot's total reaches it and with
; palette 6 when it does not.
; HOW: The entry number in zTemp (bios_b low); the slot's cleared bit tested with CLEARED_MASKS; PHX, PLX around it;
; SlotScoreReaches; WriteHexByteTiles or WriteDimHexByte.
; WAITS: no.
; Entry: X the cursor object. Exit: A the last digit tile, N and Z clear, C bit 3 of the last byte, X kept, Y the
; entry's byte index, V from the page add when it was made.
DrawSlotRecord:
    jsr VecSelectVramWriteAddress
    st1 #$56
    st2 #$05
    jsr VecSelectVramData
    lda a:OBJ_WORK_H,x
    cmp #$07
    bne DrawSlotRecord_at0
    lda #$0E
    bra DrawSlotRecord_at1
DrawSlotRecord_at0:
    ldy a:OBJ_WORK_D,x
    beq DrawSlotRecord_at1
    clc
    adc #$07
DrawSlotRecord_at1:
    sta z:zBiosBl
    phx
    tax
    ldy z:zSaveSlot
    lda a:SlotOffsets,y
    tay
    cpx #$08
    bcc DrawSlotRecord_tACB9
    iny
DrawSlotRecord_tACB9:
    lda a:SaveScoreTotal+3,y
    and a:ClearedMasks,x
    bne DrawSlotRecord_at2
    plx
    lda z:zBiosBl
    asl a
    tay
    bsr SlotScoreReaches
    bcc DrawSlotRecord_at3
DrawSlotRecord_at4:
    cla
    jsr WriteHexByteTiles
    lda a:RecordScores+1,y
    jsr WriteHexByteTiles
    lda a:RecordScores,y
    jmp WriteHexByteTiles
DrawSlotRecord_at2:
    plx
    ldy #$1C
    bra DrawSlotRecord_at4
DrawSlotRecord_at3:
    cla
    jsr WriteDimHexByte
    lda a:RecordScores+1,y
    jsr WriteDimHexByte
    lda a:RecordScores,y
    jmp WriteDimHexByte

; @routine 00DE:4EF WriteDimHexByte
; @run 6F:ACEF
; WHAT IT DOES: Writes a byte as two digit tiles with palette 6.
; HOW: PHA; the high nibble through the second half by BSR; PLA; the low nibble falls into it.
; WAITS: no.
; Entry: A the byte, the VDC's write address set. Exit: A the low digit's tile, N and Z clear, C bit 3 of the byte;
; X and Y kept.
WriteDimHexByte:
    pha
    lsr a
    lsr a
    lsr a
    lsr a
    bsr WriteDimHexByte_at0
    pla
    and #$0F
WriteDimHexByte_at0:
    ora #$10
    sta a:VDC_DATA_LO
    st2 #$61
    rts

; @routine 00E0:0EB SlotEntryTest
; @run 6F:B8EB
; WHAT IT DOES: Tells whether the selected slot has reached entry X of the record list: entry 13 is bit 0 of the
; slot's first byte; the others are a bit of a pair of its flag bytes.
; HOW: The slot's offset; ENTRY_OFFSETS added, the byte at $01 ORed with the one at $06, ANDed with ENTRY_MASKS.
; WAITS: no.
; Entry: X the entry. Exit: A the bit or 0, N and Z from it; Y the offset used; X kept; C from the compare with 13, or
; C and V from the offset's add.
SlotEntryTest:
    ldy z:zSaveSlot
    lda a:SlotOffsets,y
    cpx #$0D
    beq SlotEntryTest_tB903
    clc
    adc a:EntryOffsets,x
    tay
    lda a:SaveBuffer+1,y
    ora a:SaveBuffer+6,y
    and a:SoundVoicesFirst+3,x
    rts
SlotEntryTest_tB903:
    tay
    lda a:SaveBuffer,y
    and #$01
    rts

; @routine 00E0:1A1 DrawRecordList
; @run 6F:B9A1
; WHAT IT DOES: Draws the selected slot's list of 14 entries and the score line: an entry not reached shows the unknown
; string (attribute $41), entry 0 its name with attribute $11, a reached entry its name with $51 when the slot's
; cleared bit for it is set and $41 when not. Then the score line (DrawSlotRecord) with the X the last entry left.
; HOW: The entry counted in the card's zero page word A (bios_a low); SlotEntryTest; CLEARED_MASKS against bytes $22
; and $23; DrawUnknownName or DrawEntryName; a tail jump to DrawSlotRecord.
; WAITS: no.
; Entry: nothing. Exit: as DrawSlotRecord, X the attribute of entry 13.
DrawRecordList:
    stz z:zBiosAl
DrawRecordList_at5:
    ldx z:zBiosAl
    jsr SlotEntryTest
    beq DrawRecordList_at0
    lda z:zBiosAl
    beq DrawRecordList_at1
    ldy z:zSaveSlot
    lda a:SlotOffsets,y
    tay
    cpx #$08
    bcc DrawRecordList_tB9B9
    iny
DrawRecordList_tB9B9:
    lda a:SaveScoreTotal+3,y
    and a:ClearedMasks,x
    beq DrawRecordList_at2
    ldx #$51
    bra DrawRecordList_tB9C7
DrawRecordList_at2:
    ldx #$41
DrawRecordList_tB9C7:
    lda z:zBiosAl
    bra DrawRecordList_at3
DrawRecordList_at0:
    ldx #$41
    lda z:zBiosAl
    bsr DrawUnknownName
    bra DrawRecordList_at4
DrawRecordList_at1:
    ldx #$11
DrawRecordList_at3:
    bsr DrawEntryName
DrawRecordList_at4:
    inc z:zBiosAl
    lda z:zBiosAl
    cmp #$0E
    bne DrawRecordList_at5
    jmp DrawSlotRecord

; @routine 00E0:1E2 DrawUnknownName
; @run 6F:B9E2
; WHAT IT DOES: Draws the string for an entry not reached at entry A's place.
; HOW: The place from ENTRY_PLACES, the string pointer from UNKNOWN_NAME, then the twelve tiles at $BA20.
; WAITS: no.
; Entry: A the entry, X the tiles' high byte. Exit: A the last tile, Y $0C, Z and C set, N clear; X kept.
DrawUnknownName:
    asl a
    tay
    jsr VecSelectVramWriteAddress
    lda a:RichterItemUseScripts,y
    sta a:VDC_DATA_LO
    lda a:RichterItemUseScripts+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:UnknownName
    sta z:zTemp00
    lda a:UnknownName+1
    sta z:zTemp01
    bra DrawEntryName+30

; @routine 00E0:202 DrawEntryName
; @run 6F:BA02
; WHAT IT DOES: Draws entry A's name at its place.
; HOW: As DrawUnknownName, the string pointer from ENTRY_NAMES by the entry.
; WAITS: no.
; Entry, exit: as DrawUnknownName.
DrawEntryName:
    asl a
    tay
    jsr VecSelectVramWriteAddress
    lda a:RichterItemUseScripts,y
    sta a:VDC_DATA_LO
    lda a:RichterItemUseScripts+1,y
    sta a:VDC_DATA_HI
    jsr VecSelectVramData
    lda a:EntryNames,y
    sta z:zTemp00
    lda a:EntryNames+1,y
    sta z:zTemp01
    cly
DrawEntryName_at0:
    lda (zTemp00),y
    sta a:VDC_DATA_LO
    stx a:VDC_DATA_HI
    iny
    cpy #$0C
    bne DrawEntryName_at0
    rts

; @routine 00E0:40A AnimateCharacterChoice
; @run 6F:BC0A
; WHAT IT DOES: The character page's two objects for a frame: object $33 (the character picture) is animated while it
; is on the screen (its Y low byte not zero); object $34 plays the list for the picture's character (bit 0 of $33's
; start argument) while its OBJ->work_d is not zero: 2 starts the list afresh, 1 goes on, and the list's end sets 0.
; HOW: PHX and PLX around it; AnimateCharacterPortrait; OBJ->work_d of $34 counted down from 2, its animation cleared;
; the list from PORTRAIT_FLASH_LISTS through zTemp10 and zTemp11; StepObjectFrames.
; WAITS: no.
; Entry: nothing. Exit: X kept, N and Z from it (the PLX); A, Y, C and V as the last step left them.
AnimateCharacterChoice:
    phx
    ldx #$33
    lda a:OBJ_Y_LO,x
    beq AnimateCharacterChoice_at0
    jsr AnimateCharacterPortrait
    inx
    lda a:OBJ_WORK_D,x
    beq AnimateCharacterChoice_at0
    dec a
    beq AnimateCharacterChoice_at1
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_ANIM_STEP,x
    dec a:OBJ_WORK_D,x
AnimateCharacterChoice_at1:
    lda a:OBJ_START_ARG-1,x
    and #$01
    asl a
    tay
    lda a:PortraitFlashLists,y
    sta z:zTemp10
    lda a:PortraitFlashLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    tay
    bne AnimateCharacterChoice_at0
    stz a:OBJ_WORK_D,x
AnimateCharacterChoice_at0:
    plx
    rts

; @routine 00E0:443 AnimateCharacterPortrait
; @run 6F:BC43
; WHAT IT DOES: Steps the animation of the menu's character picture (the object whose start argument the character
; page sets to the slot's character). When the start argument has changed from the one shown (OBJ->work_d), the
; turning animation of the new one (start argument plus 2) starts; at its end the picture rests on its character's
; own (start argument less 2). Frames $31 and $37 of the turn play sounds $79 and $78.
; HOW: OBJ->start_arg against OBJ->work_d; OBJ->anim_timer and OBJ->anim_step cleared; the list from PORTRAIT_LISTS
; through zTemp10 and zTemp11; StepObjectFrames; OBJ->frame compared; SoundCommand as a tail jump.
; WAITS: no.
; Entry: X the object. Exit: Y A as StepObjectFrames left it (TAY); then held: A $FF, C clear, N set; ended: A the
; start argument less 2, N and Z from it, C clear; changed: A the frame, N, Z and C from its last compare, or as
; SoundCommand leaves them. X and V kept (V as StepObjectFrames keeps it).
AnimateCharacterPortrait:
    lda a:OBJ_START_ARG,x
    cmp a:OBJ_WORK_D,x
    beq AnimateCharacterPortrait_tBC59
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_ANIM_STEP,x
    inc a
    inc a
    sta a:OBJ_WORK_D,x
    sta a:OBJ_START_ARG,x
AnimateCharacterPortrait_tBC59:
    asl a
    tay
    lda a:PortraitLists,y
    sta z:zTemp10
    lda a:PortraitLists+1,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    tay
    beq AnimateCharacterPortrait_at0
    bcc AnimateCharacterPortrait_at1
    lda a:OBJ_FRAME,x
    cmp #$31
    beq AnimateCharacterPortrait_at2
    cmp #$37
    bne AnimateCharacterPortrait_at1
    lda #$78
    jmp VecSoundCommand
AnimateCharacterPortrait_at0:
    lda a:OBJ_START_ARG,x
    dec a
    dec a
    bmi AnimateCharacterPortrait_at1
    sta a:OBJ_START_ARG,x
    sta a:OBJ_WORK_D,x
AnimateCharacterPortrait_at1:
    rts
AnimateCharacterPortrait_at2:
    lda #$79
    jmp VecSoundCommand

; @routine 00C3:255 FileMenuReadSavePage
; @run 6B:9255
; @stop 925A
; WHAT IT DOES: Page 0: the save read, then the start screen.
; HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
; WAITS: yes, in the steps that wait (the page of the save read).
; Entry: from the menu's page table. Exit: as the step leaves them.
FileMenuReadSavePage:
    lda z:zField80
    jsr MenuStepDispatch

; @routine 00C3:341 FileMenuErasePage
; @run 6B:9341
; @stop 9346
; WHAT IT DOES: Page 5: erasing a slot.
; HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
; WAITS: no.
; Entry: from the menu's page table. Exit: as the step leaves them.
FileMenuErasePage:
    lda z:zField80
    jsr MenuStepDispatch

; @routine 00C3:3D2 FileMenuCopyPage
; @run 6B:93D2
; @stop 93DA
; WHAT IT DOES: Page 6: copying a slot; the cursor shakes each frame.
; HOW: JitterCursorObjects, then zField80 in A and a JSR to MenuStepDispatch, the table after it.
; WAITS: no.
; Entry: from the menu's page table. Exit: as the step leaves them.
FileMenuCopyPage:
    jsr JitterCursorObjects
    lda z:zField80
    jsr MenuStepDispatch

; @routine 00C3:58E FileMenuNamePage
; @run 6B:958E
; @stop 9593
; WHAT IT DOES: Page 11: the name entry.
; HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
; WAITS: no.
; Entry: from the menu's page table. Exit: as the step leaves them.
FileMenuNamePage:
    lda z:zField80
    jsr MenuStepDispatch

; @routine 00C4:058 FileMenuListPage
; @run 6B:9858
; @stop 985D
; WHAT IT DOES: Page 9: a slot's list.
; HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
; WAITS: no.
; Entry: from the menu's page table. Exit: as the step leaves them.
FileMenuListPage:
    lda z:zField80
    jsr MenuStepDispatch

; @routine 00C4:130 FileMenuSlidePage
; @run 6B:9930
; @stop 9935
; WHAT IT DOES: Page 2: sliding to the item chosen.
; HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
; WAITS: no.
; Entry: from the menu's page table. Exit: as the step leaves them.
FileMenuSlidePage:
    lda z:zField80
    jsr MenuStepDispatch

; @routine 00C4:307 FileMenuBackPage
; @run 6B:9B07
; @stop 9B0C
; WHAT IT DOES: Page 10: sliding back.
; HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
; WAITS: no.
; Entry: from the menu's page table. Exit: as the step leaves them.
FileMenuBackPage:
    lda z:zField80
    jsr MenuStepDispatch

; @routine 00C4:337 FileMenuBackToStartPage
; @run 6B:9B37
; @stop 9B3C
; WHAT IT DOES: Page 8: sliding back to the opening page.
; HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
; WAITS: no.
; Entry: from the menu's page table. Exit: as the step leaves them.
FileMenuBackToStartPage:
    lda z:zField80
    jsr MenuStepDispatch

; @routine 00C4:424 FileMenuOpenPage
; @run 6B:9C24
; @stop 9C29
; WHAT IT DOES: Page 1: the opening page.
; HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
; WAITS: no.
; Entry: from the menu's page table. Exit: as the step leaves them.
FileMenuOpenPage:
    lda z:zField80
    jsr MenuStepDispatch

; @routine 00C4:5F4 MenuStepDispatch
; @run 6B:9DF4
; WHAT IT DOES: Goes to the handler the table after the caller's JSR names for step A: the file menu's pages and their steps.
; HOW: A doubled plus one in X; the return address pulled into the operand of its own JMP, which then reads the word at that address plus X.
; WAITS: no.
; Entry: A the step, the table's address less one on the stack. Exit: the target; A the operand's high byte, N and Z from it, X twice the step plus one, C bit 7 of the step.
MenuDispatchOperand = $9E00 ; the operand of the JMP below
MenuStepDispatch:
    asl a
    inc a
    tax
    pla
    sta a:MenuDispatchOperand
    pla
    sta a:MenuDispatchOperand+1
    jmp (MenuDispatchOperand,x)

