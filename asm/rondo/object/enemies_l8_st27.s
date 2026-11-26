; @routine 025B:09F SpawnRowsWhenScrollAtZero
; @run 70:B09F
; WHAT IT DOES: Runs SetBossFixedFields and StepVerticalScrollTable; when the scroll's bytes ScrollXHi+1 and ScrollXLo+1 are both 0 it sets the game's sub-mode 5, counts slot 0's OBJ_3510 down, sets the byte at $3BF0 to 1, clears bit 0 of zField8E, spawns the stage's rows 3 and 4 (SpawnStageObjectsRows34) and goes on to SetTimerThenNextState; otherwise it sets ScrollStepXHi+1 to 1.
; HOW: Two calls, an ORA, stores, RMWs, SpawnStageObjectsRows34 between a PHX and a PLX, a tail jump.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran. The byte at $3BF0 keeps its address form until it has a name.
SpawnRowsWhenScrollAtZero:
    jsr SetBossFixedFields
    jsr StepVerticalScrollTable
    stz a:ScrollStepXHi+1
    lda a:ScrollXHi+1
    ora a:ScrollXLo+1
    bne SpawnRowsWhenScrollAtZero_at0
    lda #$05
    sta z:zGameSubMode
    dec a:OBJ_3510
    lda #$01
    sta a:PlayerByte3BF0
    rmb0 z:zField8E
    phx
    jsr SpawnStageObjectsRows34
    plx
    jmp SetTimerThenNextState
SpawnRowsWhenScrollAtZero_at0:
    inc a:ScrollStepXHi+1
    rts

; @routine 025B:471 BossFrameRoutines
; @run 70:B471
; WHAT IT DOES: One frame of the boss of overlay 025B: counts OBJ_3568 up, then runs SwingByCosineFromState0B, EnterState0BWhenFlagged, StartRandomScriptWhenFlagged, PushPlayerWhenFlagged and HitSoundThenSpawnPieces, and the code the state table names (JumpByStateTable025B545).
; HOW: An INC, five calls, a call into the table's jump.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the code the table names.
BossFrameRoutines:
    inc a:OBJ_3568,x
    jsr SwingByCosineFromState0B
    jsr EnterState0BWhenFlagged
    jsr StartRandomScriptWhenFlagged
    jsr PushPlayerWhenFlagged
    jsr HitSoundThenSpawnPieces
    jsr JumpByStateTable025B545
    rts

; @routine 05CB:13F BossFrameRoutinesC
; @run 70:B13F
; WHAT IT DOES: One frame of the boss of overlay 05CB: counts OBJ_34B8 up, runs FlagThenHitsCompare, EnterState5WhenFlagged and the code the state table names (JumpByStateTable05CB1CF); then, when OBJ_35C0 and the frame are not 0, starts the list that the table at $B169 names for the frame less 1 (plus $81) on channel 5.
; HOW: An INC, three calls, a table read, StartListChannelKind2 between a PHX and a PLX.
; WAITS: no.
; Entry: X object. Exit: as the last instruction that ran.
BossFrameRoutinesC:
    inc a:OBJ_34B8,x
    jsr FlagThenHitsCompare
    jsr EnterState5WhenFlagged
    jsr JumpByStateTable05CB1CF
    lda a:OBJ_35C0,x
    bne BossFrameRoutinesC_at0
    lda a:OBJ_2A68,x
    beq BossFrameRoutinesC_at0
    sec
    sbc #$01
    tay
    lda a:$B169,y
    beq BossFrameRoutinesC_at0
    phx
    clc
    adc #$81
    ldx #$05
    jsr VecStartListChannelKind2
    plx
BossFrameRoutinesC_at0:
    rts

; @routine 0B4B:556 BossFrameRoutinesB
; @run 70:B556
; WHAT IT DOES: One frame of the boss of overlay 0B4B: counts OBJ_3618 up, runs FaceThePlayerByStateTable, StateTableHurtFlag, EnterState0FWhenFlagged, HitBoxByFrameTable and the code the state table names (JumpByStateTable0B4B64C); then, when OBJ_3670 is set, starts the list $83 plus the frame less 1 on channel 5.
; HOW: An INC, five calls, StartListChannelKind2 between a PHX and a PLX.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
BossFrameRoutinesB:
    inc a:OBJ_3618,x
    jsr FaceThePlayerByStateTable
    jsr StateTableHurtFlag
    jsr EnterState0FWhenFlagged
    jsr HitBoxByFrameTable
    jsr JumpByStateTable0B4B64C
    lda a:OBJ_3670,x
    beq BossFrameRoutinesB_at0
    phx
    lda a:OBJ_2A68,x
    sec
    sbc #$01
    clc
    adc #$83
    ldx #$05
    jsr VecStartListChannelKind2
    plx
BossFrameRoutinesB_at0:
    rts

; @routine 0F6B:38E HurtSoundsThenRunTable
; @run 70:B38E
; WHAT IT DOES: Runs SoundWhenHurtThenFlag and SoundAndState6WhenFlagged and then the code the state table names (JumpByStateTable0F6B3EC).
; HOW: Three calls.
; WAITS: no.
; Entry: X object, Y kept for the sounds. Exit: as the code the table names.
HurtSoundsThenRunTable:
    bsr SoundWhenHurtThenFlag
    bsr SoundAndState6WhenFlagged
    bsr JumpByStateTable0F6B3EC
    rts

; @routine 0F6C:39C BossFrameRoutinesD
; @run 70:BB9C
; WHAT IT DOES: One frame of the boss of overlay 0F6C: runs EnterState0EWhenFlagged and FlashWhenHurtThenSound, the code the state table names (JumpByStateTable0F6C3C0) and, in the states the table at $BBB0 marks, HitBoxRowByFrameKind.
; HOW: Three calls, a table read, a call into the original code.
; WAITS: no.
; Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The call into HitBoxRowByFrameKind (0F6D:523) ran in no script; the draft stops there.
BossFrameRoutinesD:
    jsr EnterState0EWhenFlagged
    jsr FlashWhenHurtThenSound
    bsr JumpByStateTable0F6C3C0
    ldy a:OBJ_27A8,x
    lda a:$BBB0,y
    beq BossFrameRoutinesD_at0
    jsr HitBoxRowByFrameKind
BossFrameRoutinesD_at0:
    rts
    brk
    brk
    brk
    brk
    brk
    brk
    brk
    brk
    brk
    brk
    ora (zTemp01,x)
    ora (zTemp00,x)
    brk
    brk

; @routine 0F6D:523 HitBoxRowByFrameKind
; @run 71:C523
; WHAT IT DOES: Takes the hit box (OBJ_3988, OBJ_3A38, OBJ_3930, OBJ_39E0) from a four byte row of the table at $C575, the row the table at $C545 names for the object's frame (OBJ_2A68).
; HOW: A read of the frame as Y, a table read, two shifts, four table reads and stores.
; WAITS: no.
; Entry: X object. Exit: A the last byte read, Y the row, C bit 6 of the kind, N and Z from A. The tables at $C545 and $C575 keep their address form until they have a name.
HitBoxRowByFrameKind:
    ldy a:OBJ_2A68,x
    lda a:$C545,y
    asl a
    asl a
    tay
    lda a:$C575,y
    sta a:OBJ_3988,x
    lda a:$C576,y
    sta a:OBJ_3A38,x
    lda a:$C577,y
    sta a:OBJ_3930,x
    lda a:$C578,y
    sta a:OBJ_39E0,x
    rts
