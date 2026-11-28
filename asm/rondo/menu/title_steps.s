; @routine 00C1:000 TitleEntry
; @run 6B:8000
; WHAT IT DOES: The title overlay's entry for a frame: runs TitleStep.
; HOW: A jump to TitleStep.
; WAITS: no.
; Entry: Y kept. Exit: as TitleStep.
; not checked: as TitleStep.
TitleEntry:
    jmp TitleStep

; @routine 00C1:00F TitleStep
; @run 6B:800F
; WHAT IT DOES: One frame of the title: the sub-mode's handler, by zGameSubMode's word in the table at
; $802E (sub-mode 0 is TitleSetUp, then the scroll in, the blinks, the wipe, a wait, the logo, its
; fades, a wait, its cycles, a wait, the last script, a wait and the end).
; HOW: Past the first sub-mode, with the backup file read and the title seen before, START (the joypad
; byte's bit 3) skips the title: zModeDone is counted down and it returns (A 8, N and Z from zModeDone,
; X, Y and C kept). Otherwise it reads zGameSubMode again, doubles it and jumps through the table. A word
; this overlay does not hold runs as the original code.
; WAITS: no.
; Entry: Y kept. Exit: as the handler leaves them.
; not checked: the START skip arm (the 3 bytes at 8024) has no draft code.
TitleStep:
    lda z:zGameSubMode
    beq TitleStep_at0
    lda a:TitleSaveRead
    beq TitleStep_at0
    lda a:TitleSeen
    beq TitleStep_at0
    lda a:BiosJoyTrigger
    and #$08
    beq TitleStep_at0
    dec z:zModeDone
    rts
TitleStep_at0:
    lda z:zGameSubMode
    asl a
    tax
    jmp (TitleSteps,x)

; @routine 00C1:048 TitleSetUp
; @run 6B:8048
; WHAT IT DOES: The title's first sub-mode: the music stopped, the backup file read (TitleSaveRead
; is set when it read without trouble), the screen's size set, the title's first list and script
; started, the raster effect on at line $10, and on to the next sub-mode.
; HOW: SoundStopAll, BackupRead, ex_scrsiz with 3, StartListChannel0Kind2 and StartScriptSlot0Kind1
; with 0, six stores, zGameSubMode counted up. A and X read back from the backup call reach no
; branch that a result other than 0 makes differ.
; WAITS: no.
; Entry: X the doubled sub-mode, Y kept. Exit: A $10 (the last load), X and Y 0, N and Z from the new
; sub-mode, C and V as ex_scrsiz leaves them.
; not checked: the draft only ran a backup read that returns 0; the other results are untraced.
TitleSetUp:
    jsr VecSoundStopAll
    jsr VecBackupRead
    tax
    beq TitleSetUp_at0
    dec a
    bne TitleSetUp_at1
    inx
    beq TitleSetUp_at0
    bra TitleSetUp_at1
TitleSetUp_at0:
    lda #$01
    sta a:TitleSaveRead
TitleSetUp_at1:
    lda #$03
    jsr BiosExScrSiz
    cla
    jsr VecStartListChannel0Kind2
    cla
    jsr VecStartScriptSlot0Kind1
    lda #$01
    sta z:zRasterEffect
    stz a:ScrollXLo+1
    lda #$03
    sta a:ScrollXHi+1
    lda #$10
    sta z:zRasterArg
    stz a:ScrollYLo+1
    stz a:ScrollYHi+1
    inc z:zGameSubMode
    rts

; @routine 00C1:1F8 TitleScreenStep
; @run 6B:81F8
; WHAT IT DOES: One frame of the screen's setup: the handler zGameSubMode's word names in the table
; at $81FF (sub-mode 0 is TitleScreenBegin, 1 is TitleSoundBegin).
; HOW: Reads zGameSubMode, doubles it and jumps through the table. A word this overlay does not
; hold runs as the original code.
; WAITS: no.
; Entry: Y kept. Exit: as the handler leaves them.
; not checked: one case reaches the skip arm of TitleStep.
TitleScreenStep:
    lda z:zGameSubMode
    asl a
    tax
    jmp ($81FF,x)

; @routine 00C1:205 TitleScreenBegin
; @run 6B:8205
; WHAT IT DOES: The screen's first sub-mode: the screen state cleared, the objects' fields at
; $2A68, $2A8A, $2A8B, $2A70 and $2A71 cleared, list 1 started on channel 0 as kind 2 and script 3
; on slot 0 as kind 1, zTemp3F and the script's step cleared, and on to the next sub-mode.
; HOW: ClearScreenState, five stores, the two starts, two stores and zGameSubMode counted up.
; WAITS: no.
; Entry: X the doubled sub-mode and Y kept (ClearScreenState saves them). Exit: A and Y 3, X 0, N and Z from the new sub-mode; C and V kept.
TitleScreenBegin:
    jsr VecClearScreenState
    stz a:OBJ_FRAME
    stz a:$2A8A
    stz a:$2A8B
    stz a:OBJ_FRAME+8
    stz a:$2A71
    lda #$01
    jsr VecStartListChannel0Kind2
    lda #$03
    jsr VecStartScriptSlot0Kind1
    stz z:$3F
    stz z:zWaitTimer4
    inc z:zGameSubMode
    rts

; @routine 00C1:22F TitleSoundBegin
; @run 6B:822F
; WHAT IT DOES: The screen's second sub-mode: the sound reset pair at $3B80 cleared, sound command
; $FD sent, zGameSubMode counted up, zStage cleared, and zModeDone counted up (ModeDoneAdd).
; HOW: Two stores, SoundCommand, two more, then it branches into ModeDoneAdd.
; WAITS: no.
; Entry: X and Y as the caller's (passed to the sound command). Exit: as SoundCommand left A, X and Y,
; N and Z from zModeDone's new value, C and V as the command left them.
TitleSoundBegin:
    stz a:$3B80
    stz a:$3B81
    lda #$FD
    jsr VecSoundCommand
    inc z:zGameSubMode
    stz z:zStage
    bra ModeDoneAdd
    lda a:BiosJoyTrigger
    and #$09
    beq $826A
    lda a:$3B81
    eor #$01
    ora a:$3B80
    beq $826A
    ldx a:$3B80
    lda a:$82BE,x
    sta z:zStage
    lda a:$3B81
    sta z:zCharacter
    tax
    lda a:$8268,x
    jsr VecSoundCommand

; @routine 00C1:700 AttractStep
; @run 6B:8700
; WHAT IT DOES: One frame of the attract scene: bank $07 plus zRamBaseBank mapped at MPR5, then the
; handler zGameSubMode's word names in the table at $870E: begin, wait for start, check start, (a sub-mode
; no script reached), the end count and the blink.
; HOW: tam, then reads zGameSubMode, doubles it and jumps through the table. A word this overlay does not
; hold runs as the original code.
; WAITS: yes, through AttractBegin and AttractEndCountdown, which reach ResetGameState.
; Entry: Y kept. Exit: as the handler leaves them.
; not checked: two cases reach the skip arm of TitleStep.
AttractStep:
    lda #$07
    clc
    adc z:zRamBaseBank
    tam #$20
    lda z:zGameSubMode
    asl a
    tax
    jmp ($870E,x)

; @routine 00C1:742 AttractWaitStart
; @run 6B:8742
; WHAT IT DOES: An attract frame that waits for the file to have been read and START (the joypad byte's
; bit 3) pressed: the frame counted and the scene's script stepped; then, once the backup file is read
; and the button is down, the scene's script goes back to $9192, its counter to $FFFF, the sub-mode on,
; and the CD fades with type $0C. (The arm after the button is untraced.)
; HOW: AttractFrameTick, ScriptStep, TitleSaveRead and the joypad byte tested, the arm's stores, then a jump
; to cd_fade.
; WAITS: no.
; Entry: nothing. Exit, no start: A 0, N and Z set, the rest as ScriptStep left them.
; With start: as cd_fade leaves them.
AttractWaitStart:
    jsr AttractFrameTick
    jsr ScriptStep
    lda a:TitleSaveRead
    beq $8791
    lda a:BiosJoyTrigger
    and #$08
    beq $8791
    lda #$92
    sta z:zScriptPtr
    lda #$91
    sta z:zScriptPtrHi
    lda #$FF
    sta z:zWaitTimer3
    sta z:zWaitTimer4
    inc z:zGameSubMode
    lda #$0C
    jmp BiosCdFade

; @routine 00C1:769 AttractStartCheck
; @run 6B:8769
; WHAT IT DOES: An attract frame in its second sub-mode: the frame counted and the script stepped; on the
; start button the scene's music is cut (sound command $7A), the counter set to $0080 with the blink's
; fields at $2758 to $2A71 cleared, and the sub-mode set to 5.
; HOW: AttractFrameTick, ScriptStep, the joypad byte's bit 3, SoundCommand, seven stores.
; WAITS: no.
; Entry: X the doubled sub-mode, Y kept. Exit, no start: A 0, N and Z set, the rest as ScriptStep left them.
; With start: A 5, N and Z clear, as SoundCommand leaves the rest.
; not checked: one case differs inside SoundDriverCommand (the sound driver's own frames).
AttractStartCheck:
    jsr AttractFrameTick
    jsr ScriptStep
    lda a:BiosJoyTrigger
    and #$08
    beq AttractStartCheck_at0
    lda #$7A
    jsr VecSoundCommand
    lda #$80
    sta z:zWaitTimer3
    stz z:zWaitTimer4
    stz a:OBJ_KIND+8
    stz a:OBJ_FRAME+8
    stz a:$2759
    stz a:$2A71
    lda #$05
    sta z:zGameSubMode
AttractStartCheck_at0:
    rts

; @routine 00C1:792 AttractBlink
; @run 6B:8792
; WHAT IT DOES: The attract scene's last state before the end: every 16th frame, while its step is 0,
; flips the six low bits of the blink colour at $2A68; counts the frame counter down, and on 0 goes through
; three steps: the first clears the colour and starts script 8, the second starts script 9 (each with $40
; frames), the third counts zModeDone.
; HOW: zFrameTick's low nibble, the counter's low and high bytes (zWaitTimer3 and 4), StartScriptSlot0Kind2.
; WAITS: no.
; Entry: nothing. Exit: while counting, A the last byte loaded (or the colour), N and Z from the counter's
; new value, C kept. At a step, A and Y the script, X 0, C set. Third, A the step, N and Z from zModeDone's
; new value, C set when the step is 2 or more.
AttractBlink:
    lda z:zFrameTick
    and #$0F
    bne AttractBlink_at0
    lda z:zWaitTimer4
    bne AttractBlink_at0
    lda a:OBJ_FRAME
    eor #$3F
    sta a:OBJ_FRAME
AttractBlink_at0:
    dec z:zWaitTimer3
    bne $8791
    inc z:zWaitTimer4
    lda z:zWaitTimer4
    cmp #$01
    beq AttractBlink_at1
    cmp #$02
    beq AttractBlink_at2
    inc z:zModeDone
    rts
AttractBlink_at1:
    lda #$40
    sta z:zWaitTimer3
    stz a:OBJ_FRAME
    lda #$08
    jmp VecStartScriptSlot0Kind2
AttractBlink_at2:
    lda #$40
    sta z:zWaitTimer3
    lda #$09
    jmp VecStartScriptSlot0Kind2
