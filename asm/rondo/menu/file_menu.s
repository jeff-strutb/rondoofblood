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
FileMenuPickEmpty_at0:
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
    bra FileMenuPickEmpty_at0

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
    sta a:$7DFE
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
    dec a:$7DFE
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
    stz a:SaveBuffer,x
    inx
    iny
    cpy #$30
    bne TallyDigitCell152D
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
FileMenuCopyPickFrom_at3:
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
FileMenuCopyPickTo_at2:
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
    bne FileMenuCopyCancel_at0
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
    beq StartTimerWhenSlot10Ready
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_ANIM_STEP,x
    inc a
    inc a
    sta a:OBJ_WORK_D,x
    sta a:OBJ_START_ARG,x
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
