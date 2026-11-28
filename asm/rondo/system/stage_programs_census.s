; @routine 026D:0B4 ResetStageVariables026D
; @run 6B:80B4
; WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $D541.
; HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D5 and N set. X, Y, C, V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ResetStageVariables026D:
    lda z:zGameRunning
    beq ResetStageVariables026D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$41
    sta z:zScriptPtr
    lda #$D5
    sta z:zScriptPtrHi
ResetStageVariables026D_at0:
    rts

; @routine 026E:03D ExpandStageVramRuns026E
; @run 6B:883D
; WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9C52.
; HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageVramRunTable026E = $9C52 ; the program's run list of each section, a word per section
ExpandStageVramRuns026E:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageVramRunTable026E,y
    sta z:zTemp00
    lda a:StageVramRunTable026E+1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns026E_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns026E_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns026E_at0
    rts
ExpandStageVramRuns026E_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns026E_at1
    bra ExpandStageVramRuns026E_at2

; @routine 026F:235 ExpandStageLists026F
; @run 6B:9235
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9C4A.
; HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListTable026F = $9C4A ; the program's list record of each section, a word per section
ExpandStageLists026F:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListTable026F,y
    sta z:zTemp00
    lda a:StageListTable026F+1,y
    sta z:zTemp01
    cly
ExpandStageLists026F_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists026F_at0
    lda #$08
    bra ExpandStageLists026F_at1
ExpandStageLists026F_at7:
    lda (zTemp00)
    bne ExpandStageLists026F_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists026F_at1
ExpandStageLists026F_at2:
    cmp #$FF
    bne ExpandStageLists026F_at3
    rts
ExpandStageLists026F_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists026F_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists026F_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists026F_at5
    inc z:zTemp03
ExpandStageLists026F_at5:
    lda #$02
ExpandStageLists026F_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists026F_at6
    inc z:zTemp01
ExpandStageLists026F_at6:
    bra ExpandStageLists026F_at7

; @routine 031D:0B4 ResetStageVariables031D
; @run 6B:80B4
; WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $BF1E.
; HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $BF and N set. X, Y, C, V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ResetStageVariables031D:
    lda z:zGameRunning
    beq ResetStageVariables031D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$1E
    sta z:zScriptPtr
    lda #$BF
    sta z:zScriptPtrHi
ResetStageVariables031D_at0:
    rts

; @routine 031E:03D ExpandStageVramRuns031E
; @run 6B:883D
; WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9E19.
; HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageVramRunTable031E = $9E19 ; the program's run list of each section, a word per section
ExpandStageVramRuns031E:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageVramRunTable031E,y
    sta z:zTemp00
    lda a:StageVramRunTable031E+1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns031E_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns031E_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns031E_at0
    rts
ExpandStageVramRuns031E_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns031E_at1
    bra ExpandStageVramRuns031E_at2

; @routine 031F:229 ExpandStageLists031F
; @run 6B:9229
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9E11.
; HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListTable031F = $9E11 ; the program's list record of each section, a word per section
ExpandStageLists031F:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListTable031F,y
    sta z:zTemp00
    lda a:StageListTable031F+1,y
    sta z:zTemp01
    cly
ExpandStageLists031F_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists031F_at0
    lda #$08
    bra ExpandStageLists031F_at1
ExpandStageLists031F_at7:
    lda (zTemp00)
    bne ExpandStageLists031F_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists031F_at1
ExpandStageLists031F_at2:
    cmp #$FF
    bne ExpandStageLists031F_at3
    rts
ExpandStageLists031F_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists031F_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists031F_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists031F_at5
    inc z:zTemp03
ExpandStageLists031F_at5:
    lda #$02
ExpandStageLists031F_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists031F_at6
    inc z:zTemp01
ExpandStageLists031F_at6:
    bra ExpandStageLists031F_at7

; @routine 043D:0B4 ResetStageVariables043D
; @run 6B:80B4
; WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $C340.
; HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C3 and N set. X, Y, C, V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ResetStageVariables043D:
    lda z:zGameRunning
    beq ResetStageVariables043D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$40
    sta z:zScriptPtr
    lda #$C3
    sta z:zScriptPtrHi
ResetStageVariables043D_at0:
    rts

; @routine 043E:03D ExpandStageVramRuns043E
; @run 6B:883D
; WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9EDD.
; HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageVramRunTable043E = $9EDD ; the program's run list of each section, a word per section
ExpandStageVramRuns043E:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageVramRunTable043E,y
    sta z:zTemp00
    lda a:StageVramRunTable043E+1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns043E_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns043E_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns043E_at0
    rts
ExpandStageVramRuns043E_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns043E_at1
    bra ExpandStageVramRuns043E_at2

; @routine 043F:229 ExpandStageLists043F
; @run 6B:9229
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9ED5.
; HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListTable043F = $9ED5 ; the program's list record of each section, a word per section
ExpandStageLists043F:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListTable043F,y
    sta z:zTemp00
    lda a:StageListTable043F+1,y
    sta z:zTemp01
    cly
ExpandStageLists043F_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists043F_at0
    lda #$08
    bra ExpandStageLists043F_at1
ExpandStageLists043F_at7:
    lda (zTemp00)
    bne ExpandStageLists043F_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists043F_at1
ExpandStageLists043F_at2:
    cmp #$FF
    bne ExpandStageLists043F_at3
    rts
ExpandStageLists043F_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists043F_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists043F_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists043F_at5
    inc z:zTemp03
ExpandStageLists043F_at5:
    lda #$02
ExpandStageLists043F_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists043F_at6
    inc z:zTemp01
ExpandStageLists043F_at6:
    bra ExpandStageLists043F_at7

; @routine 064D:0B4 ResetStageVariables064D
; @run 6B:80B4
; WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $DC6A.
; HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DC and N set. X, Y, C, V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ResetStageVariables064D:
    lda z:zGameRunning
    beq ResetStageVariables064D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$6A
    sta z:zScriptPtr
    lda #$DC
    sta z:zScriptPtrHi
ResetStageVariables064D_at0:
    rts

; @routine 064E:03D ExpandStageVramRuns064E
; @run 6B:883D
; WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9D01.
; HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageVramRunTable064E = $9D01 ; the program's run list of each section, a word per section
ExpandStageVramRuns064E:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageVramRunTable064E,y
    sta z:zTemp00
    lda a:StageVramRunTable064E+1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns064E_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns064E_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns064E_at0
    rts
ExpandStageVramRuns064E_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns064E_at1
    bra ExpandStageVramRuns064E_at2

; @routine 064F:235 ExpandStageLists064F
; @run 6B:9235
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9CFB.
; HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListTable064F = $9CFB ; the program's list record of each section, a word per section
ExpandStageLists064F:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListTable064F,y
    sta z:zTemp00
    lda a:StageListTable064F+1,y
    sta z:zTemp01
    cly
ExpandStageLists064F_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists064F_at0
    lda #$08
    bra ExpandStageLists064F_at1
ExpandStageLists064F_at7:
    lda (zTemp00)
    bne ExpandStageLists064F_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists064F_at1
ExpandStageLists064F_at2:
    cmp #$FF
    bne ExpandStageLists064F_at3
    rts
ExpandStageLists064F_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists064F_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists064F_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists064F_at5
    inc z:zTemp03
ExpandStageLists064F_at5:
    lda #$02
ExpandStageLists064F_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists064F_at6
    inc z:zTemp01
ExpandStageLists064F_at6:
    bra ExpandStageLists064F_at7

; @routine 10DD:0B4 ResetStageVariables10DD
; @run 6B:80B4
; WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $C6F1.
; HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C6 and N set. X, Y, C, V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ResetStageVariables10DD:
    lda z:zGameRunning
    beq ResetStageVariables10DD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$F1
    sta z:zScriptPtr
    lda #$C6
    sta z:zScriptPtrHi
ResetStageVariables10DD_at0:
    rts

; @routine 10DE:03D ExpandStageVramRuns10DE
; @run 6B:883D
; WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9ED7.
; HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageVramRunTable10DE = $9ED7 ; the program's run list of each section, a word per section
ExpandStageVramRuns10DE:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageVramRunTable10DE,y
    sta z:zTemp00
    lda a:StageVramRunTable10DE+1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns10DE_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns10DE_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns10DE_at0
    rts
ExpandStageVramRuns10DE_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns10DE_at1
    bra ExpandStageVramRuns10DE_at2

; @routine 10DF:229 ExpandStageLists10DF
; @run 6B:9229
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9ECF.
; HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListTable10DF = $9ECF ; the program's list record of each section, a word per section
ExpandStageLists10DF:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListTable10DF,y
    sta z:zTemp00
    lda a:StageListTable10DF+1,y
    sta z:zTemp01
    cly
ExpandStageLists10DF_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists10DF_at0
    lda #$08
    bra ExpandStageLists10DF_at1
ExpandStageLists10DF_at7:
    lda (zTemp00)
    bne ExpandStageLists10DF_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists10DF_at1
ExpandStageLists10DF_at2:
    cmp #$FF
    bne ExpandStageLists10DF_at3
    rts
ExpandStageLists10DF_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists10DF_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists10DF_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists10DF_at5
    inc z:zTemp03
ExpandStageLists10DF_at5:
    lda #$02
ExpandStageLists10DF_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists10DF_at6
    inc z:zTemp01
ExpandStageLists10DF_at6:
    bra ExpandStageLists10DF_at7

; @routine 122D:0B4 ResetStageVariables122D
; @run 6B:80B4
; WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $D712.
; HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D7 and N set. X, Y, C, V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ResetStageVariables122D:
    lda z:zGameRunning
    beq ResetStageVariables122D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$12
    sta z:zScriptPtr
    lda #$D7
    sta z:zScriptPtrHi
ResetStageVariables122D_at0:
    rts

; @routine 122E:03D ExpandStageVramRuns122E
; @run 6B:883D
; WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9CD0.
; HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageVramRunTable122E = $9CD0 ; the program's run list of each section, a word per section
ExpandStageVramRuns122E:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageVramRunTable122E,y
    sta z:zTemp00
    lda a:StageVramRunTable122E+1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns122E_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns122E_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns122E_at0
    rts
ExpandStageVramRuns122E_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns122E_at1
    bra ExpandStageVramRuns122E_at2

; @routine 122F:235 ExpandStageLists122F
; @run 6B:9235
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9CCA.
; HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListTable122F = $9CCA ; the program's list record of each section, a word per section
ExpandStageLists122F:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListTable122F,y
    sta z:zTemp00
    lda a:StageListTable122F+1,y
    sta z:zTemp01
    cly
ExpandStageLists122F_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists122F_at0
    lda #$08
    bra ExpandStageLists122F_at1
ExpandStageLists122F_at7:
    lda (zTemp00)
    bne ExpandStageLists122F_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists122F_at1
ExpandStageLists122F_at2:
    cmp #$FF
    bne ExpandStageLists122F_at3
    rts
ExpandStageLists122F_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists122F_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists122F_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists122F_at5
    inc z:zTemp03
ExpandStageLists122F_at5:
    lda #$02
ExpandStageLists122F_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists122F_at6
    inc z:zTemp01
ExpandStageLists122F_at6:
    bra ExpandStageLists122F_at7

; @routine 1A2D:0B4 ResetStageVariables1A2D
; @run 6B:80B4
; WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $DCA3.
; HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DC and N set. X, Y, C, V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ResetStageVariables1A2D:
    lda z:zGameRunning
    beq ResetStageVariables1A2D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$A3
    sta z:zScriptPtr
    lda #$DC
    sta z:zScriptPtrHi
ResetStageVariables1A2D_at0:
    rts

; @routine 1A2E:03D ExpandStageVramRuns1A2E
; @run 6B:883D
; WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9CCD.
; HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageVramRunTable1A2E = $9CCD ; the program's run list of each section, a word per section
ExpandStageVramRuns1A2E:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageVramRunTable1A2E,y
    sta z:zTemp00
    lda a:StageVramRunTable1A2E+1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns1A2E_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns1A2E_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns1A2E_at0
    rts
ExpandStageVramRuns1A2E_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns1A2E_at1
    bra ExpandStageVramRuns1A2E_at2

; @routine 1A2F:235 ExpandStageLists1A2F
; @run 6B:9235
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9CC5.
; HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListTable1A2F = $9CC5 ; the program's list record of each section, a word per section
ExpandStageLists1A2F:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListTable1A2F,y
    sta z:zTemp00
    lda a:StageListTable1A2F+1,y
    sta z:zTemp01
    cly
ExpandStageLists1A2F_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists1A2F_at0
    lda #$08
    bra ExpandStageLists1A2F_at1
ExpandStageLists1A2F_at7:
    lda (zTemp00)
    bne ExpandStageLists1A2F_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists1A2F_at1
ExpandStageLists1A2F_at2:
    cmp #$FF
    bne ExpandStageLists1A2F_at3
    rts
ExpandStageLists1A2F_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists1A2F_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists1A2F_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists1A2F_at5
    inc z:zTemp03
ExpandStageLists1A2F_at5:
    lda #$02
ExpandStageLists1A2F_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists1A2F_at6
    inc z:zTemp01
ExpandStageLists1A2F_at6:
    bra ExpandStageLists1A2F_at7

; @routine 1B8D:0B4 ResetStageVariables1B8D
; @run 6B:80B4
; WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $D9C4.
; HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D9 and N set. X, Y, C, V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ResetStageVariables1B8D:
    lda z:zGameRunning
    beq ResetStageVariables1B8D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$C4
    sta z:zScriptPtr
    lda #$D9
    sta z:zScriptPtrHi
ResetStageVariables1B8D_at0:
    rts

; @routine 1B8E:03D ExpandStageVramRuns1B8E
; @run 6B:883D
; WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9D15.
; HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageVramRunTable1B8E = $9D15 ; the program's run list of each section, a word per section
ExpandStageVramRuns1B8E:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageVramRunTable1B8E,y
    sta z:zTemp00
    lda a:StageVramRunTable1B8E+1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns1B8E_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns1B8E_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns1B8E_at0
    rts
ExpandStageVramRuns1B8E_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns1B8E_at1
    bra ExpandStageVramRuns1B8E_at2

; @routine 1B8F:235 ExpandStageLists1B8F
; @run 6B:9235
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9D0D.
; HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListTable1B8F = $9D0D ; the program's list record of each section, a word per section
ExpandStageLists1B8F:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListTable1B8F,y
    sta z:zTemp00
    lda a:StageListTable1B8F+1,y
    sta z:zTemp01
    cly
ExpandStageLists1B8F_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists1B8F_at0
    lda #$08
    bra ExpandStageLists1B8F_at1
ExpandStageLists1B8F_at7:
    lda (zTemp00)
    bne ExpandStageLists1B8F_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists1B8F_at1
ExpandStageLists1B8F_at2:
    cmp #$FF
    bne ExpandStageLists1B8F_at3
    rts
ExpandStageLists1B8F_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists1B8F_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists1B8F_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists1B8F_at5
    inc z:zTemp03
ExpandStageLists1B8F_at5:
    lda #$02
ExpandStageLists1B8F_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists1B8F_at6
    inc z:zTemp01
ExpandStageLists1B8F_at6:
    bra ExpandStageLists1B8F_at7

; @routine 1CED:0B4 ResetStageVariables1CED
; @run 6B:80B4
; WHAT IT DOES: While the game runs, clears the random bytes, zWaveCounter, zFrameTick and $207C-$207D, and points $207E at $DC06.
; HOW: reset_stage_variables (player_stage_start.c), the body of 024D:0CB, with this program's pointer.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DC and N set. X, Y, C, V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ResetStageVariables1CED:
    lda z:zGameRunning
    beq ResetStageVariables1CED_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$06
    sta z:zScriptPtr
    lda #$DC
    sta z:zScriptPtrHi
ResetStageVariables1CED_at0:
    rts

; @routine 1CEE:03D ExpandStageVramRuns1CEE
; @run 6B:883D
; WHAT IT DOES: Clears the stage's VRAM copy buffer and fills the runs of the current section's list, from this program's table of lists at $9CAB.
; HOW: expand_vram_runs (stage_vram_runs.c), the body of 024E:082, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list position, N 0, Z 1, C 1. Clobbers zTemp00-02.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageVramRunTable1CEE = $9CAB ; the program's run list of each section, a word per section
ExpandStageVramRuns1CEE:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageVramRunTable1CEE,y
    sta z:zTemp00
    lda a:StageVramRunTable1CEE+1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns1CEE_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns1CEE_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns1CEE_at0
    rts
ExpandStageVramRuns1CEE_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns1CEE_at1
    bra ExpandStageVramRuns1CEE_at2

; @routine 1CEF:235 ExpandStageLists1CEF
; @run 6B:9235
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the current section's record, from this program's table of records at $9C9F.
; HOW: expand_stage_lists (stage_lists.c), the body of 024F:277, with this program's table.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers zTemp00-04.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListTable1CEF = $9C9F ; the program's list record of each section, a word per section
ExpandStageLists1CEF:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListTable1CEF,y
    sta z:zTemp00
    lda a:StageListTable1CEF+1,y
    sta z:zTemp01
    cly
ExpandStageLists1CEF_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists1CEF_at0
    lda #$08
    bra ExpandStageLists1CEF_at1
ExpandStageLists1CEF_at7:
    lda (zTemp00)
    bne ExpandStageLists1CEF_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists1CEF_at1
ExpandStageLists1CEF_at2:
    cmp #$FF
    bne ExpandStageLists1CEF_at3
    rts
ExpandStageLists1CEF_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists1CEF_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists1CEF_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists1CEF_at5
    inc z:zTemp03
ExpandStageLists1CEF_at5:
    lda #$02
ExpandStageLists1CEF_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists1CEF_at6
    inc z:zTemp01
ExpandStageLists1CEF_at6:
    bra ExpandStageLists1CEF_at7

; @routine 026F:33B SetVramAddresses026F
; @run 6B:933B
; WHAT IT DOES: Points the VDC's read and write addresses at X and Y and selects the data register: this program's SetVramAddresses.
; HOW: set_vram_addresses (map_draw.c), the body of 024F:37D, with this copy's return addresses.
; WAITS: no.
; Entry: X the address low, Y the high. Exit: A 2, N and Z from it. X, Y kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SetVramAddresses026F:
    jsr VecSelectVramReadAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jmp VecSelectVramData

; @routine 031F:61C SetVramAddresses031F
; @run 6B:961C
; WHAT IT DOES: Points the VDC's read and write addresses at X and Y and selects the data register: this program's SetVramAddresses.
; HOW: set_vram_addresses (map_draw.c), the body of 024F:37D, with this copy's return addresses.
; WAITS: no.
; Entry: X the address low, Y the high. Exit: A 2, N and Z from it. X, Y kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SetVramAddresses031F:
    jsr VecSelectVramReadAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jmp VecSelectVramData

; @routine 026D:634 StageSectionNext026D
; @run 6B:8634
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
; HOW: As StageSectionNext (024D:66A), with this program's table.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SectionOrder026D = $9C42 ; the section record by the step through the stage
StageSectionNext026D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder026D,y
    sta z:zField83
    rts

; @routine 031D:634 StageSectionNext031D
; @run 6B:8634
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
; HOW: As StageSectionNext (024D:66A), with this program's table.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SectionOrder031D = $9E09 ; the section record by the step through the stage
StageSectionNext031D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder031D,y
    sta z:zField83
    rts

; @routine 043D:634 StageSectionNext043D
; @run 6B:8634
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
; HOW: As StageSectionNext (024D:66A), with this program's table.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SectionOrder043D = $9ECD ; the section record by the step through the stage
StageSectionNext043D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder043D,y
    sta z:zField83
    rts

; @routine 064D:634 StageSectionNext064D
; @run 6B:8634
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
; HOW: As StageSectionNext (024D:66A), with this program's table.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SectionOrder064D = $9CF3 ; the section record by the step through the stage
StageSectionNext064D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder064D,y
    sta z:zField83
    rts

; @routine 10DD:634 StageSectionNext10DD
; @run 6B:8634
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
; HOW: As StageSectionNext (024D:66A), with this program's table.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SectionOrder10DD = $9EC7 ; the section record by the step through the stage
StageSectionNext10DD:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder10DD,y
    sta z:zField83
    rts

; @routine 122D:634 StageSectionNext122D
; @run 6B:8634
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
; HOW: As StageSectionNext (024D:66A), with this program's table.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SectionOrder122D = $9CC7 ; the section record by the step through the stage
StageSectionNext122D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder122D,y
    sta z:zField83
    rts

; @routine 1A2D:634 StageSectionNext1A2D
; @run 6B:8634
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
; HOW: As StageSectionNext (024D:66A), with this program's table.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SectionOrder1A2D = $9CBD ; the section record by the step through the stage
StageSectionNext1A2D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder1A2D,y
    sta z:zField83
    rts

; @routine 1B8D:634 StageSectionNext1B8D
; @run 6B:8634
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
; HOW: As StageSectionNext (024D:66A), with this program's table.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SectionOrder1B8D = $9D09 ; the section record by the step through the stage
StageSectionNext1B8D:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder1B8D,y
    sta z:zField83
    rts

; @routine 1CED:634 StageSectionNext1CED
; @run 6B:8634
; WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from this program's section order by it.
; HOW: As StageSectionNext (024D:66A), with this program's table.
; WAITS: no.
; Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SectionOrder1CED = $9C99 ; the section record by the step through the stage
StageSectionNext1CED:
    inc z:zField82
    lda z:zField82
    tay
    lda a:SectionOrder1CED,y
    sta z:zField83
    rts

; @routine 026F:0D1 MaskStageRowCopy026F
; @run 6B:90D1
; WHAT IT DOES: Masks one row of the stage's mask buffer from the cells two on: each of the row's eight cells ANDed with the mask in zTemp00.
; HOW: As MaskStageRowCopy (024F:113), with this program's offset table.
; WAITS: no.
; Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last cell written, N and Z from X, C and V from the last offset add. X and Y kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageMaskOffsets026F = $9107 ; the eight offsets of the mask buffer's cells one step touches
MaskStageRowCopy026F:
    stx z:zTemp08
    phx
    phy
    ldx #$07
MaskStageRowCopy026F_at0:
    lda a:StageMaskOffsets026F,x
    clc
    adc z:zTemp08
    tay
    lda a:StageMaskBuffer+2,y
    and z:zTemp00
    sta a:StageMaskBuffer,y
    dex
    bpl MaskStageRowCopy026F_at0
    ply
    plx
    rts

; @routine 026F:0EC MaskStageRow026F
; @run 6B:90EC
; WHAT IT DOES: Masks one row of the stage's mask buffer from the cells in place: each of the row's eight cells ANDed with the mask in zTemp00.
; HOW: As MaskStageRow (024F:12E), with this program's offset table.
; WAITS: no.
; Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last cell written, N and Z from X, C and V from the last offset add. X and Y kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
MaskStageRow026F:
    stx z:zTemp08
    phx
    phy
    ldx #$07
MaskStageRow026F_at0:
    lda a:StageMaskOffsets026F,x
    clc
    adc z:zTemp08
    tay
    lda a:StageMaskBuffer,y
    and z:zTemp00
    sta a:StageMaskBuffer,y
    dex
    bpl MaskStageRow026F_at0
    ply
    plx
    rts

; @routine 031F:0C5 MaskStageRowCopy031F
; @run 6B:90C5
; WHAT IT DOES: Masks one row of the stage's mask buffer from the cells two on: each of the row's eight cells ANDed with the mask in zTemp00.
; HOW: As MaskStageRowCopy (024F:113), with this program's offset table.
; WAITS: no.
; Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last cell written, N and Z from X, C and V from the last offset add. X and Y kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageMaskOffsets031F = $90FB ; the eight offsets of the mask buffer's cells one step touches
MaskStageRowCopy031F:
    stx z:zTemp08
    phx
    phy
    ldx #$07
MaskStageRowCopy031F_at0:
    lda a:StageMaskOffsets031F,x
    clc
    adc z:zTemp08
    tay
    lda a:StageMaskBuffer+2,y
    and z:zTemp00
    sta a:StageMaskBuffer,y
    dex
    bpl MaskStageRowCopy031F_at0
    ply
    plx
    rts

; @routine 031F:0E0 MaskStageRow031F
; @run 6B:90E0
; WHAT IT DOES: Masks one row of the stage's mask buffer from the cells in place: each of the row's eight cells ANDed with the mask in zTemp00.
; HOW: As MaskStageRow (024F:12E), with this program's offset table.
; WAITS: no.
; Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last cell written, N and Z from X, C and V from the last offset add. X and Y kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
MaskStageRow031F:
    stx z:zTemp08
    phx
    phy
    ldx #$07
MaskStageRow031F_at0:
    lda a:StageMaskOffsets031F,x
    clc
    adc z:zTemp08
    tay
    lda a:StageMaskBuffer,y
    and z:zTemp00
    sta a:StageMaskBuffer,y
    dex
    bpl MaskStageRow031F_at0
    ply
    plx
    rts

; @routine 026E:63D SetStageTileVramAddress026E
; @run 6B:8E3D
; WHAT IT DOES: Sets the VRAM address of the tile pair zTemp04 names, from this program's table of addresses.
; HOW: As SetStageTileVramAddress (024E:687): zTemp04 less bit 0 indexes the table; a tail JMP to the setter loaded at that address (several programs hold these bytes).
; WAITS: no.
; Entry: zTemp04. Exit: A 2, N and Z from it, X the address low, Y the high.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageTileVramAddress026E = $8E4C ; VRAM addresses of the stage's tile pairs, a word each
SetStageTileVramAddress026E:
    lda z:zTemp04
    and #$FE
    tax
    lda a:StageTileVramAddress026E,x
    ldy a:StageTileVramAddress026E+1,x
    tax
    jmp SetVramAddresses026F

; @routine 026E:3F9 SetHealthBarAddress026E
; @run 6B:8BF9
; WHAT IT DOES: While zField99 is not 0, sets the VRAM address of the bar at the cell $5E less it counts.
; HOW: As SetHealthBarAddress (024E:443), calling the setter loaded at its JSR's address (several programs hold these bytes).
; WAITS: no.
; Entry: nothing. Exit, zField99 0: A 0, Z set. Else: A 2, X and Y the address, C and V from the last add.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SetHealthBarAddress026E:
    lda z:zField99
    beq SetHealthBarAddress026E_at0
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
    jsr SetVramAddresses026F
SetHealthBarAddress026E_at0:
    rts

; @routine 031E:63D SetStageTileVramAddress031E
; @run 6B:8E3D
; WHAT IT DOES: Sets the VRAM address of the tile pair zTemp04 names, from this program's table of addresses.
; HOW: As SetStageTileVramAddress (024E:687): zTemp04 less bit 0 indexes the table; a tail JMP to the setter loaded at that address (several programs hold these bytes).
; WAITS: no.
; Entry: zTemp04. Exit: A 2, N and Z from it, X the address low, Y the high.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageTileVramAddress031E = $8E4C ; VRAM addresses of the stage's tile pairs, a word each
SetStageTileVramAddress031E:
    lda z:zTemp04
    and #$FE
    tax
    lda a:StageTileVramAddress031E,x
    ldy a:StageTileVramAddress031E+1,x
    tax
    jmp SetVramAddresses031F

; @routine 031E:3F9 SetHealthBarAddress031E
; @run 6B:8BF9
; WHAT IT DOES: While zField99 is not 0, sets the VRAM address of the bar at the cell $5E less it counts.
; HOW: As SetHealthBarAddress (024E:443), calling the setter loaded at its JSR's address (several programs hold these bytes).
; WAITS: no.
; Entry: nothing. Exit, zField99 0: A 0, Z set. Else: A 2, X and Y the address, C and V from the last add.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SetHealthBarAddress031E:
    lda z:zField99
    beq SetHealthBarAddress031E_at0
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
    jsr SetVramAddresses031F
SetHealthBarAddress031E_at0:
    rts

; @routine 026E:06C StartStageLists026E
; @run 6B:886C
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
; HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists026F, script slot 7 (the loaded routine: 6) and its return addresses.
; WAITS: no.
; Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListNumbers026E = $9C34 ; the list channel's list by section
StageScriptNumbers026E = $9C3C ; the script by section and character
StartStageLists026E:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListNumbers026E,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists026F
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptNumbers026E,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 031E:06C StartStageLists031E
; @run 6B:886C
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
; HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists031F, script slot 7 (the loaded routine: 6) and its return addresses.
; WAITS: no.
; Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListNumbers031E = $9DFF ; the list channel's list by section
StageScriptNumbers031E = $9E07 ; the script by section and character
StartStageLists031E:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListNumbers031E,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists031F
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptNumbers031E,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 043E:06C StartStageLists043E
; @run 6B:886C
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
; HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists043F, script slot 7 (the loaded routine: 6) and its return addresses.
; WAITS: no.
; Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListNumbers043E = $9EC3 ; the list channel's list by section
StageScriptNumbers043E = $9ECB ; the script by section and character
StartStageLists043E:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListNumbers043E,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists043F
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptNumbers043E,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 064E:06C StartStageLists064E
; @run 6B:886C
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
; HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists064F, script slot 7 (the loaded routine: 6) and its return addresses.
; WAITS: no.
; Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListNumbers064E = $9CE5 ; the list channel's list by section
StageScriptNumbers064E = $9CED ; the script by section and character
StartStageLists064E:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListNumbers064E,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists064F
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptNumbers064E,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 10DE:06C StartStageLists10DE
; @run 6B:886C
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
; HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists10DF, script slot 7 (the loaded routine: 6) and its return addresses.
; WAITS: no.
; Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListNumbers10DE = $9EB9 ; the list channel's list by section
StageScriptNumbers10DE = $9EC1 ; the script by section and character
StartStageLists10DE:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListNumbers10DE,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists10DF
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptNumbers10DE,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 122E:06C StartStageLists122E
; @run 6B:886C
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
; HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists122F, script slot 7 (the loaded routine: 6) and its return addresses.
; WAITS: no.
; Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListNumbers122E = $9CBE ; the list channel's list by section
StageScriptNumbers122E = $9CC1 ; the script by section and character
StartStageLists122E:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListNumbers122E,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists122F
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptNumbers122E,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 1A2E:06C StartStageLists1A2E
; @run 6B:886C
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
; HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists1A2F, script slot 7 (the loaded routine: 6) and its return addresses.
; WAITS: no.
; Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListNumbers1A2E = $9CAD ; the list channel's list by section
StageScriptNumbers1A2E = $9CB5 ; the script by section and character
StartStageLists1A2E:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListNumbers1A2E,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists1A2F
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptNumbers1A2E,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 1B8E:06C StartStageLists1B8E
; @run 6B:886C
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
; HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists1B8F, script slot 7 (the loaded routine: 6) and its return addresses.
; WAITS: no.
; Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListNumbers1B8E = $9CFD ; the list channel's list by section
StageScriptNumbers1B8E = $9D01 ; the script by section and character
StartStageLists1B8E:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListNumbers1B8E,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists1B8F
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptNumbers1B8E,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 1CEE:06C StartStageLists1CEE
; @run 6B:886C
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage lists, and the section's script for the character in script slot 7.
; HOW: As StartStageLists (024E:0B1), with this program's tables, ExpandStageLists1CEF, script slot 7 (the loaded routine: 6) and its return addresses.
; WAITS: no.
; Entry: nothing. Exit: as StartScriptSlotKind1, X 7; C and V from the script index's add. Y as the list expansion left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListNumbers1CEE = $9C87 ; the list channel's list by section
StageScriptNumbers1CEE = $9C8D ; the script by section and character
StartStageLists1CEE:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListNumbers1CEE,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists1CEF
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptNumbers1CEE,x
    ldx #$07
    jmp VecStartScriptSlotKind1

; @routine 026F:36E SpawnStageObject026F
; @run 6B:936E
; WHAT IT DOES: Puts a new object in the slot a row of this program's spawn tables names, with the row's frame, sprite set and X and Y low bytes.
; HOW: As SpawnStageObject (024F:3B0), with this program's tables and return address.
; WAITS: no.
; Entry: X the row. Exit: A the Y low byte, N and Z from it, Y the slot. X kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageSpawnSlot026F = $938E ; by spawn row
StageSpawnFrame026F = $9396 ; by spawn row
StageSpawnKind026F = $939E ; by spawn row
StageSpawnXLo026F = $93A6 ; by spawn row
StageSpawnYLo026F = $93AE ; by spawn row
SpawnStageObject026F:
    lda a:StageSpawnSlot026F,x
    tay
    jsr VecClearObjectInY
    lda a:StageSpawnFrame026F,x
    sta a:OBJ_FRAME,y
    lda a:StageSpawnKind026F,x
    sta a:OBJ_SPRITE_SET,y
    lda a:StageSpawnXLo026F,x
    sta a:OBJ_X_LO,y
    lda a:StageSpawnYLo026F,x
    sta a:OBJ_Y_LO,y
    rts

; @routine 031F:64F SpawnStageObject031F
; @run 6B:964F
; WHAT IT DOES: Puts a new object in the slot a row of this program's spawn tables names, with the row's frame, sprite set and X and Y low bytes.
; HOW: As SpawnStageObject (024F:3B0), with this program's tables and return address.
; WAITS: no.
; Entry: X the row. Exit: A the Y low byte, N and Z from it, Y the slot. X kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageSpawnSlot031F = $966F ; by spawn row
StageSpawnFrame031F = $9677 ; by spawn row
StageSpawnKind031F = $967F ; by spawn row
StageSpawnXLo031F = $9687 ; by spawn row
StageSpawnYLo031F = $968F ; by spawn row
SpawnStageObject031F:
    lda a:StageSpawnSlot031F,x
    tay
    jsr VecClearObjectInY
    lda a:StageSpawnFrame031F,x
    sta a:OBJ_FRAME,y
    lda a:StageSpawnKind031F,x
    sta a:OBJ_SPRITE_SET,y
    lda a:StageSpawnXLo031F,x
    sta a:OBJ_X_LO,y
    lda a:StageSpawnYLo031F,x
    sta a:OBJ_Y_LO,y
    rts

; @routine 026E:75A SpawnStageObjectsRows34026E
; @run 6B:8F5A
; WHAT IT DOES: Unless zField8E bit 0 is set, spawns rows 3 and 4 and sets up the two anchor objects and the player byte.
; HOW: SpawnSecondPairBody (stage_spawn.c), the body of 024E:798, spawning with the row spawn loaded at its JSR's address (several programs hold these bytes), with this copy's return addresses.
; WAITS: no.
; Entry: nothing. Exit when it spawned: A 2, X 4, Y the second slot, N and Z from A; else as entered.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SpawnStageObjectsRows34026E:
    bbr0 zField8E,SpawnStageObjectsRows34026E_at0
    rts
SpawnStageObjectsRows34026E_at0:
    ldx #$03
    jsr SpawnStageObject026F
    inx
    jsr SpawnStageObject026F
    dec a:OBJ_X_HI+84
    dec a:OBJ_Y_HI+85
    lda #$04
    sta a:OBJ_VX_HI+84
    sta a:OBJ_VY_HI+85
    lda #$20
    sta a:OBJ_WORK_E+84
    sta a:OBJ_WORK_E+85
    lda #$02
    sta a:TimeSlowMode
    stz a:OBJ_WORK_G+84
    rts

; @routine 031E:74E SpawnStageObjectsRows34031E
; @run 6B:8F4E
; WHAT IT DOES: Unless zField8E bit 0 is set, spawns rows 3 and 4 and sets up the two anchor objects and the player byte.
; HOW: SpawnSecondPairBody (stage_spawn.c), the body of 024E:798, spawning with the row spawn loaded at its JSR's address (several programs hold these bytes), with this copy's return addresses.
; WAITS: no.
; Entry: nothing. Exit when it spawned: A 2, X 4, Y the second slot, N and Z from A; else as entered.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SpawnStageObjectsRows34031E:
    bbr0 zField8E,SpawnStageObjectsRows34031E_at0
    rts
SpawnStageObjectsRows34031E_at0:
    ldx #$03
    jsr SpawnStageObject031F
    inx
    jsr SpawnStageObject031F
    dec a:OBJ_X_HI+84
    dec a:OBJ_Y_HI+85
    lda #$04
    sta a:OBJ_VX_HI+84
    sta a:OBJ_VY_HI+85
    lda #$20
    sta a:OBJ_WORK_E+84
    sta a:OBJ_WORK_E+85
    lda #$02
    sta a:TimeSlowMode
    stz a:OBJ_WORK_G+84
    rts

; @routine 026F:3DF FillMarkedTiles026F
; @run 6B:93DF
; WHAT IT DOES: For each non-zero byte of a 256-byte table from index Y on, fills one 16-word VRAM tile with $55AA: the
; tile at address X*$100 + index*16.
; HOW: The table's address (A its high byte) in zTemp00-01. For a marked index: the write address selected, the address
; set (the index shifted four times through zTemp03, X ORed into the high byte), the data register selected, sixteen
; words by ST1 and ST2.
; WAITS: no.
; Entry: A the table's page, X the VRAM address high bits, Y the first index. Exit: Y 0, N 0, Z 1. If any tile was
; filled: X 0, C 0; else X and C kept. A 2 if index $FF was filled, else 0. V kept. Clobbers zTemp00-03.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
FillMarkedTiles026F:
    stz z:zTemp00
    sta z:zTemp01
    stx z:zTemp02
FillMarkedTiles026F_at0:
    lda (zTemp00),y
    beq FillMarkedTiles026F_at2
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
FillMarkedTiles026F_at1:
    st1 #$AA
    st2 #$55
    dex
    bne FillMarkedTiles026F_at1
FillMarkedTiles026F_at2:
    iny
    bne FillMarkedTiles026F_at0
    rts

; @routine 1531:2FC ClearEffectSlot1531
; @run 6B:82FC
; WHAT IT DOES: Empties a palette effect slot of the list program at sector 1531: its eight fields, 8 bytes apart from $3C00.
; HOW: Eight STZ of the slot's columns, the kind first. This build has 8 slots where the later ones have 12.
; WAITS: no.
; Entry: X the slot. Exit: A, X, Y and the flags kept.
; Nothing the game does loads sector 1531, and nothing is known to reach this routine.
ClearEffectSlot1531:
    stz a:ScriptSlotKind,x
    stz a:ScriptSlotKind+8,x
    stz a:ScriptSlotKind+16,x
    stz a:ScriptSlotKind+24,x
    stz a:ScriptSlotKind+32,x
    stz a:ScriptSlotKind+40,x
    stz a:ScriptSlotKind+48,x
    stz a:ScriptSlotKind+56,x
    rts

; @routine 1531:55C EffectRestoreColours1531
; @run 6B:855C
; WHAT IT DOES: Ends a palette effect of the list program at sector 1531: clears zFadeActive, sends the program's 256 background and 128 sprite colours to the VCE, and empties the slot.
; HOW: Two TIA, from $8632 to colour 0 and from $8832 to colour $180, then a jump to ClearEffectSlot1531.
; WAITS: no.
; Entry: X the slot; Y pushed by the block moves. Exit: A $80, N set, Z clear. X, Y, C, V kept.
; Nothing the game does loads sector 1531, and nothing is known to reach this routine.
EffectColours1531 = $8632 ; the program's 256 background colours
EffectSpriteColours1531 = $8832 ; and 128 sprite colours
EffectRestoreColours1531:
    stz z:zFadeActive
    lda #$00
    sta a:VCE_ADDR_HI
    lda #$00
    sta a:VCE_ADDR_LO
    tia EffectColours1531,VCE_DATA_LO,$0200
    lda #$01
    sta a:VCE_ADDR_HI
    lda #$80
    sta a:VCE_ADDR_LO
    tia EffectSpriteColours1531,VCE_DATA_LO,$0100
    jmp ClearEffectSlot1531

; @routine 026E:205 StepScrollAndRunObjects026E
; @run 6B:8A05
; WHAT IT DOES: Steps the stage scroll, maps the banks eight and nine above zRamBaseBank at $A000 and $C000, and goes on into the overlay at $A000.
; HOW: A JSR to the scroll step at $89EC (the one loaded there: the nine programs hold these bytes), an add, two TAM, and a JMP to $A000.
; WAITS: as the code at $A000.
; Entry: X as the scroll step takes it. Exit: as the code at $A000, entered with A the second bank, N and Z from it, C and V from the add.
; Nothing the game does loads these programs (codemap.unloaded_programs), so nothing is known to reach it.
StageObjectsEntry026E = $A000 ; the overlay loaded at $A000, its first entry
StepScrollAndRunObjects026E:
    jsr StageScrollStep026E
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jmp StageObjectsEntry026E

; @routine 026D:18B StagePauseToggle026D
; @run 6B:818B
; WHAT IT DOES: Pauses the stage when RUN is pressed, unless the fade is running, the display is still off, or
; zPaused's bit 1 holds the pause (as StagePauseToggle, 024D:1A2). Programs 064D, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: zFadeActive XOR 1 must not be zero; BiosJoyTrigger's bit 3; zDisplayDelay zero; BBS1 of zPaused (a read);
; StagePauseSet's bytes at $8E96.
; WAITS: no.
; Entry: nothing. Exit: A the last value loaded or computed, N and Z from it; or as StagePauseSet. X, Y, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StagePauseSet026E = $8E96 ; StagePauseSet's bytes (024E:6D4) in this program
StagePauseToggle026D:
    lda z:zFadeActive
    eor #$01
    beq StagePauseToggle026D_at0
    lda a:BiosJoyTrigger
    and #$08
    beq StagePauseToggle026D_at0
    lda z:zDisplayDelay
    bne StagePauseToggle026D_at0
    bbs1 zPaused,StagePauseToggle026D_at0
    jsr StagePauseSet026E
StagePauseToggle026D_at0:
    rts

; @routine 031D:18B StagePauseToggle031D
; @run 6B:818B
; WHAT IT DOES: Pauses the stage when RUN is pressed, unless the fade is running, the display is still off, or
; zPaused's bit 1 holds the pause (as StagePauseToggle, 024D:1A2). Programs 043D and 10DD hold the same bytes.
; HOW: zFadeActive XOR 1 must not be zero; BiosJoyTrigger's bit 3; zDisplayDelay zero; BBS1 of zPaused (a read);
; StagePauseSet's bytes at $8E8A.
; WAITS: no.
; Entry: nothing. Exit: A the last value loaded or computed, N and Z from it; or as StagePauseSet. X, Y, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StagePauseSet031E = $8E8A ; StagePauseSet's bytes (024E:6D4) in this program
StagePauseToggle031D:
    lda z:zFadeActive
    eor #$01
    beq StagePauseToggle031D_at0
    lda a:BiosJoyTrigger
    and #$08
    beq StagePauseToggle031D_at0
    lda z:zDisplayDelay
    bne StagePauseToggle031D_at0
    bbs1 zPaused,StagePauseToggle031D_at0
    jsr StagePauseSet031E
StagePauseToggle031D_at0:
    rts

MapOverlayBank7_026D = $80A0 ; MapOverlayBank7's bytes (024D:0B7) in the 026D programs

; @routine 026D:0A8 StageCameraStep026D
; @run 6B:80A8
; WHAT IT DOES: The camera's frame with the overlay's bank 7 mapped (as StageCameraStep, 024D:0BF). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same
; bytes.
; HOW: MapOverlayBank7's bytes at $80A0, then the tail JMP to CameraStep.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as CameraStep, entered with A the bank, N, Z, C and V from the map's add.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageCameraStep026D:
    jsr MapOverlayBank7_026D
    jmp CameraStep


; @routine 026D:0AE StageScrollSpeedsOff026D
; @run 6B:80AE
; WHAT IT DOES: Stops the layers' scroll: maps the overlay's bank 7 and runs its ClearScrollSpeeds (as StageScrollSpeedsOff, 024D:0C5). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same
; bytes.
; HOW: MapOverlayBank7's bytes at $80A0, then the tail JMP to ClearScrollSpeeds.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as ClearScrollSpeeds leaves them, entered with A the bank and N, Z, C and V from the map's add.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageScrollSpeedsOff026D:
    jsr MapOverlayBank7_026D
    jmp ClearScrollSpeeds

; @routine 026D:249 StageChooseClearKind026D
; @run 6B:8249
; WHAT IT DOES: Chooses how the cleared stage plays out (zClearKind) from zField8C: 0 gives 2, negative 4; else 1 for 1, 2, 7 to
; $0A, $0D and $0E, and 3 for 3 to 6, $0B, $0C and $0F up; then sub-mode 3 with the scroll stopped and the forced
; presses and time-slowing cleared (as StageChooseClearKind, 024D:260). Programs 031D, 043D, 064D,
; 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: CMP chain on zField8C read once; StageScrollSpeedsOff026D.
; WAITS: no.
; Entry: nothing. Exit: as StageScrollSpeedsOff026D and the stores leave them; A, N and Z as that left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageChooseClearKind026D:
    lda z:zField8C
    beq StageChooseClearKind026D_at0
    bmi StageChooseClearKind026D_at1
    ldx #$01
    stx a:HitAttackFlags
    cmp #$03
    bcc StageChooseClearKind026D_at2
    cmp #$07
    bcc StageChooseClearKind026D_at3
    cmp #$0B
    beq StageChooseClearKind026D_at3
    cmp #$0C
    beq StageChooseClearKind026D_at3
    cmp #$0F
    bcs StageChooseClearKind026D_at3
StageChooseClearKind026D_at2:
    lda #$01
    bra StageChooseClearKind026D_at4
StageChooseClearKind026D_at3:
    lda #$03
    bra StageChooseClearKind026D_at4
StageChooseClearKind026D_at1:
    lda #$04
    bra StageChooseClearKind026D_at4
StageChooseClearKind026D_at0:
    lda #$02
StageChooseClearKind026D_at4:
    sta z:zClearKind
    lda #$03
    sta z:zGameSubMode
    stz z:zField85
    jsr StageScrollSpeedsOff026D
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz a:TimeSlowMode
    rts

; @routine 026D:226 StageSubStepDeath026D
; @run 6B:8226
; WHAT IT DOES: Sub-step for the player's death: sub-mode 2 with zField80 and zField89 cleared, the scroll stopped, the forced
; presses and the fall's step cleared; when the time-slowing mode is set it is cleared and the fade starts again from
; 2 (as StageSubStepDeath, 024D:23D). Programs 031D, 043D, 064D,
; 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: Stores; StageScrollSpeedsOff026D; TimeSlowMode; the tail JMP to StageFadeTimer's bytes at $8161.
; WAITS: no.
; Entry: X and Y the caller's. Exit: A TimeSlowMode (0), N and Z from it, the rest as StageScrollSpeedsOff026D left
; them; else as StageFadeTimer.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFadeTimer_026D = $8161 ; StageFadeTimer's bytes (024D:178) in the 026D programs
StageSubStepDeath026D:
    lda #$02
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jsr StageScrollSpeedsOff026D
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz z:zFallStep
    lda a:TimeSlowMode
    bne StageSubStepDeath026D_at0
    rts
StageSubStepDeath026D_at0:
    stz a:TimeSlowMode
    lda #$02
    sta z:zFadeCount
    jmp StageFadeTimer_026D

; @routine 026D:2A3 StageSubStepTally026D
; @run 6B:82A3
; WHAT IT DOES: Sub-step to the tally: sub-mode 6 with zField80 and zField89 cleared, the scroll stopped, the forced presses and
; the fall's step cleared, the player's work bytes F cleared and E $40; when the time-slowing mode is set it is cleared
; and the fade starts again from 2 (as StageSubStepTally, 024D:2BA). Programs 031D, 043D, 064D,
; 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: Stores; StageScrollSpeedsOff026D; TimeSlowMode; the tail JMP to StageFadeTimer's bytes at $8161.
; WAITS: no.
; Entry: X and Y the caller's. Exit: A TimeSlowMode (0), N and Z from it, the rest as StageScrollSpeedsOff026D left
; them; else as StageFadeTimer.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageSubStepTally026D:
    lda #$06
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jsr StageScrollSpeedsOff026D
    stz a:PadForcedPress
    stz a:PadForcedPressB
    stz z:zFallStep
    stz a:OBJ_TALLY_STEP
    lda #$40
    sta a:OBJ_TALLY_WAIT
    lda a:TimeSlowMode
    bne StageSubStepTally026D_at0
    rts
StageSubStepTally026D_at0:
    stz a:TimeSlowMode
    lda #$02
    sta z:zFadeCount
    jmp StageFadeTimer_026D

; @routine 026D:28B StageSubStepFadeOut026D
; @run 6B:828B
; WHAT IT DOES: Sub-step to the fade: sub-mode 4 from its first step, zField89 cleared, the scroll stopped (as StageSubStepFadeOut, 024D:2A2). Programs 031D, 043D, 064D,
; 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: Stores, then the tail JMP to StageScrollSpeedsOff026D.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as StageScrollSpeedsOff026D.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageSubStepFadeOut026D:
    lda #$04
    sta z:zGameSubMode
    stz z:zField80
    stz z:zField89
    jmp StageScrollSpeedsOff026D

; @routine 026D:296 StageSubStepFadeOutLate026D
; @run 6B:8296
; WHAT IT DOES: Sub-step to the fade at its step 5: sub-mode 4, zField80 5, zField89 cleared, the scroll stopped (as StageSubStepFadeOutLate, 024D:2AD). Programs 031D, 043D, 064D,
; 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: Stores, then the tail JMP to StageScrollSpeedsOff026D.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as StageScrollSpeedsOff026D.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageSubStepFadeOutLate026D:
    lda #$04
    sta z:zGameSubMode
    lda #$05
    sta z:zField80
    stz z:zField89
    jmp StageScrollSpeedsOff026D

; @routine 026D:2F5 StageForcedWalk026D
; @run 6B:82F5
; WHAT IT DOES: A sub-mode 3 start: the stage set up (InitStage026E), sub-mode 1, and the player walked by a forced pad away from
; the side he faces: $18 frames when zField8C is set (it is cleared). As StageForcedWalk, 024D:31D. Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: JSR InitStage026E; zGameSubMode 1; OBJ_PAD_FORCE_FRAMES; facing XOR 1 picks the pad from ForcedWalkPads026D.
; WAITS: where InitStage026E waits.
; Entry: nothing. Exit: A the pad, N and Z from it, X the table index; Y, C and V as InitStage026E left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ForcedWalkPads026D = $8316 ; the forced pad by the side the player faces, $80 then $20, in the 026D programs
StageForcedWalk026D:
    jsr InitStage026E
    lda #$01
    sta z:zGameSubMode
    lda z:zField8C
    beq StageForcedWalk026D_at0
    stz z:zField8C
    lda #$18
    sta a:OBJ_PAD_FORCE_FRAMES
StageForcedWalk026D_at0:
    lda a:OBJ_FACING_LEFT
    eor #$01
    and #$01
    tax
    lda a:ForcedWalkPads026D,x
    sta a:OBJ_PAD_FORCED
    rts

; @routine 026D:318 StageForcedWalkB026D
; @run 6B:8318
; WHAT IT DOES: A sub-mode 3 start: the stage set up (InitStage026E), sub-mode 1, and the player walked by a forced pad away from
; the side he faces: $18 frames when zField8C is set (it is cleared). As StageForcedWalk, 024D:31D. These bytes are 026D:2F5's at
; $8318; their C is filed again because the JSR's return address is its own. Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: JSR InitStage026E; zGameSubMode 1; OBJ_PAD_FORCE_FRAMES; facing XOR 1 picks the pad from ForcedWalkPads026D.
; WAITS: where InitStage026E waits.
; Entry: nothing. Exit: A the pad, N and Z from it, X the table index; Y, C and V as InitStage026E left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageForcedWalkB026D:
    jsr InitStage026E
    lda #$01
    sta z:zGameSubMode
    lda z:zField8C
    beq StageForcedWalkB026D_at0
    stz z:zField8C
    lda #$18
    sta a:OBJ_PAD_FORCE_FRAMES
StageForcedWalkB026D_at0:
    lda a:OBJ_FACING_LEFT
    eor #$01
    and #$01
    tax
    lda a:ForcedWalkPads026D,x
    sta a:OBJ_PAD_FORCED
    rts

; @routine 026D:358 StageFallRightLand026D
; @run 6B:8358
; WHAT IT DOES: The fall to the right's middle step: the player idle, his animation sound, and until his X passes $10 on a
; screen past the first the step counts on; else his landing (as StageFallRightLand, 024D:380). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: MapOverlayBanks5And6's bytes at $8089, $A01B, $A012; OBJ_X_HI and OBJ_X_LO; the tail JMP to $A01E with X 0, or
; INC zFallStep.
; WAITS: no.
; Entry: nothing. Exit: counting: N and Z from the new step; else as $A01E.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
MapOverlayBanks5And6_026D = $8089 ; MapOverlayBanks5And6's bytes (024D:0A0) in the 026D programs
StageFallRightLand026D:
    jsr MapOverlayBanks5And6_026D
    jsr MariaVecAnimationTick
    jsr MariaVecAnimationSound
    lda a:OBJ_X_HI
    beq StageFallRightLand026D_at1
    lda a:OBJ_X_LO
    cmp #$10
    bcs StageFallRightLand026D_at0
StageFallRightLand026D_at1:
    clx
    jmp MariaVecObjectApplyVelocity
StageFallRightLand026D_at0:
    inc z:zFallStep
    rts

; @routine 026D:34B StageFallPose026D
; @run 6B:834B
; WHAT IT DOES: The fall's first step: the player's animation 0 set and its sound, then the step counted on (as StageFallPose,
; 024D:373). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: MapOverlayBanks5And6's bytes at $8089, $A018 with A 0, $A012, BRA to the INC of zFallStep.
; WAITS: no.
; Entry: nothing. Exit: N and Z from the new step; the rest as $A012 left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFallPose026D:
    jsr MapOverlayBanks5And6_026D
    lda #$00
    jsr MariaVecSetAnimation
    jsr MariaVecAnimationSound
    bra StageFallRightLand026D_at0

; @routine 026D:374 StageFallRightDone026D
; @run 6B:8374
; WHAT IT DOES: The last step of the fall to the right: the stage set up again and the player walked on by pad $20 (as
; StageFallRightDone, 024D:39C). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: JSR InitStage026E; sub-mode 1, zField8C cleared; $20 frames of pad $20.
; WAITS: where InitStage026E waits.
; Entry: nothing. Exit: A $20, N and Z from it; X, Y, C and V as InitStage026E left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFallRightDone026D:
    jsr InitStage026E
    lda #$01
    sta z:zGameSubMode
    stz z:zField8C
    lda #$20
    sta a:OBJ_PAD_FORCE_FRAMES
    lda #$20
    sta a:OBJ_PAD_FORCED
    rts

; @routine 026D:3C4 StageFallLeftDone026D
; @run 6B:83C4
; WHAT IT DOES: The last step of the fall to the left: the stage set up again and the player walked on by pad $80 (as
; StageFallLeftDone, 024D:3EC). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: JSR InitStage026E; sub-mode 1, zField8C cleared; $20 frames of pad $80.
; WAITS: where InitStage026E waits.
; Entry: nothing. Exit: A $80, N and Z from it; X, Y, C and V as InitStage026E left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFallLeftDone026D:
    jsr InitStage026E
    lda #$01
    sta z:zGameSubMode
    stz z:zField8C
    lda #$20
    sta a:OBJ_PAD_FORCE_FRAMES
    lda #$80
    sta a:OBJ_PAD_FORCED
    rts

; @routine 026D:3A8 StageFallLeftLand026D
; @run 6B:83A8
; WHAT IT DOES: The fall to the left's middle step: the player idle, his animation sound, and while his X is below $F0 on a
; screen past the first the step counts on; else his landing (as StageFallLeftLand, 024D:3D0). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: MapOverlayBanks5And6's bytes at $8089, $A01B, $A012; OBJ_X_HI and OBJ_X_LO; the tail JMP to $A01E with X 0, or
; StageFallStepNext's bytes at $83C1.
; WAITS: no.
; Entry: nothing. Exit: counting: N and Z from the new step; else as $A01E.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFallStepNext_026D = $83C1 ; StageFallStepNext's bytes (024D:3E9) in the 026D programs
StageFallLeftLand026D:
    jsr MapOverlayBanks5And6_026D
    jsr MariaVecAnimationTick
    jsr MariaVecAnimationSound
    lda a:OBJ_X_HI
    beq StageFallLeftLand026D_at0
    lda a:OBJ_X_LO
    cmp #$F0
    bcc StageFallStepNext_026D
StageFallLeftLand026D_at0:
    clx
    jmp MariaVecObjectApplyVelocity

; @routine 026D:339 StageFallRight026D
; @run 6B:8339
; WHAT IT DOES: The fall to the right, a step a frame: the first sets the player's speed $0140 and runs into
; StageFallPose026D; the second StageFallRightLand026D; the third StageFallRightDone026D (as StageFallRight,
; 024D:361). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: zFallStep decremented twice with BEQs.
; WAITS: as the step taken.
; Entry: nothing. Exit: as the step taken.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFallRight026D:
    lda z:zFallStep
    dec a
    beq StageFallRightLand026D
    dec a
    beq StageFallRightDone026D
    lda #$40
    sta a:OBJ_VX_LO
    lda #$01
    sta a:OBJ_VX_HI

; @routine 026D:388 StageFallLeft026D
; @run 6B:8388
; WHAT IT DOES: The fall to the left: the second step StageFallLeftLand026D, the third StageFallLeftDone026D; the first, with
; the object in slot $50 of kind $30, counts the step on twice, else sets the player's speed -$0140, poses him
; (StageFallPose026D) and runs into StageFallLeftLand026D (as StageFallLeft, 024D:3B0). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: zFallStep decremented twice; OBJ_KIND of slot $50; BSR and BRA to StageFallStepNext's bytes; JSR
; StageFallPose026D.
; WAITS: as the step taken.
; Entry: nothing. Exit: as the step taken.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFallLeft026D:
    lda z:zFallStep
    dec a
    beq StageFallLeftLand026D
    dec a
    beq StageFallLeftDone026D
    lda a:OBJ_KIND+80
    cmp #$30
    bne StageFallLeft026D_at0
    bsr StageFallStepNext_026D
    bra StageFallStepNext_026D
StageFallLeft026D_at0:
    lda #$C0
    sta a:OBJ_VX_LO
    lda #$FE
    sta a:OBJ_VX_HI
    jsr StageFallPose026D

; @routine 026D:3EB StageRestartInPlace026D
; @run 6B:83EB
; WHAT IT DOES: Sets the stage up anew in place: the screen cleared, its lists, VRAM runs, set-up, objects and the player's start,
; then sub-mode 1 with zField86 from zField83 (as StageRestartInPlace, 024D:413). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes, their calls going to the routines
; each loads at the same addresses.
; HOW: ClearScreenState, StartStageLists026E, ExpandStageVramRuns026E, InitStage026E, SpawnStageObjectsRows01026E,
; zField8A 2, MapOverlayBanks5And6's bytes, $A003, SetUpStageObjects026E.
; WAITS: where its calls wait.
; Entry: nothing. Exit: A zField83, N and Z from it; X, Y, C and V as SetUpStageObjects026E left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageRestartInPlace026D:
    jsr VecClearScreenState
    jsr StartStageLists026E
    jsr ExpandStageVramRuns026E
    jsr InitStage026E
    jsr SpawnStageObjectsRows01026E
    lda #$02
    sta z:zField8A
    jsr MapOverlayBanks5And6_026D
    jsr MariaVecStart
    jsr SetUpStageObjects026E
    stz z:zFadeActive
    stz z:zPaused
    lda #$01
    sta z:zGameSubMode
    lda z:zField83
    sta z:zField86
    rts

; @routine 026D:420 StageShowStartFrame026D
; @run 6B:8420
; WHAT IT DOES: The player's frame by character set, then his animation sound (as StageShowStartFrame, 024D:448). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: MapOverlayBanks5And6's bytes; OBJ_FRAME of slot 0 from StartFrameByCharacter026D; the tail JMP to $A012.
; WAITS: no.
; Entry: nothing. Exit: as $A012, entered with X zCharacter, A the frame, N and Z from it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StartFrameByCharacter026D = $842E ; the player's frame at a start by zCharacter, in the 026D programs
StageShowStartFrame026D:
    jsr MapOverlayBanks5And6_026D
    ldx z:zCharacter
    lda a:StartFrameByCharacter026D,x
    sta a:OBJ_FRAME
    jmp MariaVecAnimationSound

; @routine 026D:49F StageClearedWaitScroll026D
; @run 6B:849F
; WHAT IT DOES: A cleared stage's step: the scroll and the objects run, and once zField8C's bit 7 is set the next step comes (as
; StageClearedWaitScroll, 024D:4C7). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: StepScrollAndRunObjects026E; zField8C negative counts zField85 on.
; WAITS: as StepScrollAndRunObjects026E.
; Entry: X the caller's. Exit: A zField8C, N and Z from it or from zField85's INC; X, Y, C and V as the objects left
; them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearedWaitScroll026D:
    jsr StepScrollAndRunObjects026E
    lda z:zField8C
    bpl StageClearedWaitScroll026D_at0
    inc z:zField85
StageClearedWaitScroll026D_at0:
    rts

; @routine 026D:65F StageClearedScrollOn026D
; @run 6B:865F
; WHAT IT DOES: A cleared stage's step: the scroll and the objects run, zField8C's bit 7 is cleared and the next step comes (as
; StageClearedScrollOn, 024D:695). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: StepScrollAndRunObjects026E; RMB7 of zField8C; INC zField85.
; WAITS: as StepScrollAndRunObjects026E.
; Entry: X the caller's. Exit: N and Z from zField85's INC; A, X, Y, C and V as the objects left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearedScrollOn026D:
    jsr StepScrollAndRunObjects026E
    rmb7 z:zField8C
    inc z:zField85
    rts

; @routine 026D:465 StageWalkOutScroll026D
; @run 6B:8465
; WHAT IT DOES: A frame of walking the player out of a cleared room: the scroll stepped by the route's direction, the player
; moved with it, the camera and objects run; at the route's stop the scroll halts and the next step comes (as
; StageWalkOutScroll, 024D:48D). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: (zField8C - 1) AND 1 picks the route; WalkScrollStepByRoute026D into entry 0's X step, WalkPlayerStepByRoute026D
; added to the player's X; StageCameraStep026D and StepScrollAndRunObjects026E; the X compared with
; WalkStopXByRoute026D.
; WAITS: as the calls.
; Entry: nothing. Exit: walking: A the player's X, N, Z and C from the compare, Y the route; stopped: N and Z from
; zField85's INC.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
WalkScrollStepByRoute026D = $8414 ; the scroll's X step high byte in a walk-out by route, in the 026D programs
WalkPlayerStepByRoute026D = $8416 ; what the player's X low gets each frame of a walk-out by route
WalkStopXByRoute026D = $8418 ; the player's X low a walk-out stops at by route
StageWalkOutScroll026D:
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:WalkScrollStepByRoute026D,y
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    stz a:ScrollXFrac
    lda a:OBJ_X_LO
    clc
    adc a:WalkPlayerStepByRoute026D,y
    sta a:OBJ_X_LO
    jsr StageCameraStep026D
    jsr StepScrollAndRunObjects026E
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_LO
    cmp a:WalkStopXByRoute026D,y
    beq StageWalkOutScroll026D_at0
    rts
StageWalkOutScroll026D_at0:
    jsr StageScrollSpeedsOff026D
    stz a:ScrollStepXHi
    inc z:zField85
    rts

; @routine 026D:656 StageStopAndStep026D
; @run 6B:8656
; WHAT IT DOES: The scroll stopped, a frame of the objects, the next step (as StageStopAndStep, 024D:68C). Programs 031D, 043D, 064D, 10DD, 122D, 1A2D, 1B8D and 1CED hold the same bytes.
; HOW: StageScrollSpeedsOff026D and StepScrollAndRunObjects026E, INC zField85.
; WAITS: as the calls.
; Entry: nothing. Exit: N and Z from zField85's INC; the rest as the objects left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageStopAndStep026D:
    jsr StageScrollSpeedsOff026D
    jsr StepScrollAndRunObjects026E
    inc z:zField85
    rts

; @routine 026E:3D8 StageScriptByWave026E
; @run 6B:8BD8
; WHAT IT DOES: Starts the stage's script in slot 1 that the wave counter names, when it is not the one already shown (as
; StageScriptByWave, 024E:422). Programs 031E, 043E, 064E, 10DE, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
; HOW: The counter's low three bits, masked with RichterItemReady, index WaveScripts026E; when the one found differs from
; StageVar3566's second byte, it goes there and StartScriptSlotKind4 starts it in slot 1.
; WAITS: no.
; Entry: nothing. Exit: unchanged script: A the script, X the table index, Z 1, C 1; else as StartScriptSlotKind4 with
; X 1, C as the compare left it.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
WaveScripts026E = $8BF1 ; the eight script numbers by wave, in the 026D programs
StageScriptByWave026E:
    lda z:zWaveCounter
    and #$07
    and a:RichterItemReady
    tax
    lda a:WaveScripts026E,x
    cmp a:StageVar3566+1
    beq StageScriptByWave026E_at0
    sta a:StageVar3566+1
    ldx #$01
    jmp VecStartScriptSlotKind4
StageScriptByWave026E_at0:
    rts

; @routine 026E:51A BossFlashMaskCell026E
; @run 6B:8D1A
; WHAT IT DOES: Masks one cell of VRAM, $40 words, with random bits and a pattern by the flash's step (as BossFlashMaskCell,
; 024E:564). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
; HOW: The VRAM address from X and Y (SetVramAddresses026F); for each word, BossFlashMasks026E by the step's low three bits OR a
; random byte OR zRandom2 OR zRandom1 is both masks of AndVramData, X kept on the stack.
; WAITS: no.
; Entry: X and Y the address. Exit: X 0, N 0, Z 1; A, Y, C and V as the last AndVramData left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BossFlashMasks026E = $8CD3 ; the boss gauge flash's mask patterns by its step's low three bits, in the 026D programs
AndVramData_026F = $9350 ; AndVramData's bytes (024F:392) in the 026D programs
BossFlashMaskCell026E:
    jsr SetVramAddresses026F
    ldx #$40
BossFlashMaskCell026E_at0:
    lda a:BossFlashStep
    and #$07
    tay
    jsr VecNextRandom
    ora z:zRandom2
    ora z:zRandom1
    ora a:BossFlashMasks026E,y
    phx
    tax
    tay
    jsr AndVramData_026F
    plx
    dex
    bne BossFlashMaskCell026E_at0
    rts

; @routine 031E:51A BossFlashMaskCell031E
; @run 6B:8D1A
; WHAT IT DOES: Masks one cell of VRAM, $40 words, with random bits and a pattern by the flash's step (as BossFlashMaskCell,
; 024E:564). Programs 043E and 10DE hold the same bytes.
; HOW: The VRAM address from X and Y (SetVramAddresses031F); for each word, BossFlashMasks026E by the step's low three bits OR a
; random byte OR zRandom2 OR zRandom1 is both masks of AndVramData, X kept on the stack.
; WAITS: no.
; Entry: X and Y the address. Exit: X 0, N 0, Z 1; A, Y, C and V as the last AndVramData left them.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
AndVramData_031F = $9631 ; AndVramData's bytes (024F:392) in the 031D programs
BossFlashMaskCell031E:
    jsr SetVramAddresses031F
    ldx #$40
BossFlashMaskCell031E_at0:
    lda a:BossFlashStep
    and #$07
    tay
    jsr VecNextRandom
    ora z:zRandom2
    ora z:zRandom1
    ora a:BossFlashMasks026E,y
    phx
    tax
    tay
    jsr AndVramData_031F
    plx
    dex
    bne BossFlashMaskCell031E_at0
    rts

; @routine 026E:439 GaugeTimerCheckB026E
; @run 6B:8C39
; WHAT IT DOES: While the life timer of slot 85 is zero, the gauge steps (GaugeStepB026E); else the boss gauge's flash
; at $8CDB (as GaugeTimerCheckB, 024E:483). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
; HOW: LDA and the two jumps.
; WAITS: no.
; Entry: nothing. Exit: as the step or the flash taken.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
OrVramData_026F = $935F ; OrVramData's bytes (024F:3A1) in the 026D programs
GaugeTimerCheckB026E:
    lda a:BossGaugeEnding
    beq GaugeStepB026E
    jmp BossGaugeFlash026E

; @routine 026E:441 GaugeStepB026E
; @run 6B:8C41
; WHAT IT DOES: Moves the gauge one step towards its target, as GaugeStep with this program's VRAM routines (024E:48B). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
; HOW: As GaugeStepB, the VRAM routines at $933B, $9350 and $935F.
; WAITS: no.
; Entry: nothing. Exit: as GaugeStep.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
GaugeStepB026E:
    lda a:OBJ_HEALTH+16
    bmi GaugeStepB026E_at0
    cmp a:BossGaugeShown
    beq GaugeStepUpB026E_at0
    bcs GaugeStepUpB026E
    bsr SetGaugeAddressB026E
    ldx #$0F
    ldy #$FF
    jsr AndVramData_026F
    dec a:BossGaugeShown
    rts
GaugeStepB026E_at0:
    pha
    stz a:OBJ_HEALTH+16
    bsr GaugeTimerCheckB026E
    pla
    sta a:OBJ_HEALTH+16
    ldy a:BossGaugeShown
    bne GaugeStepUpB026E_at0
    sta a:BossGaugeEnding
    rts

; @routine 026E:46D GaugeStepUpB026E
; @run 6B:8C6D
; WHAT IT DOES: Grows the gauge one, as GaugeStepUp with this program's VRAM routines (024E:4B7). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
; HOW: As GaugeStepUpB, the VRAM routines at $933B, $9350 and $935F.
; WAITS: no.
; Entry: nothing. Exit: as GaugeStepUp.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
GaugeStepUpB026E:
    inc a:BossGaugeShown
    bsr SetGaugeAddressB026E
    ldx #$F0
    ldy #$00
    jsr OrVramData_026F
GaugeStepUpB026E_at0:
    rts

; @routine 026E:47A SetGaugeAddressB026E
; @run 6B:8C7A
; WHAT IT DOES: Points the VDC at the gauge's cell for its value, as SetGaugeAddress with this program's VRAM routine (024E:4C4). Programs 064E, 122E, 1A2E, 1B8E and 1CEE hold the same bytes.
; HOW: As SetGaugeAddressB, the VRAM routines at $933B, $9350 and $935F.
; WAITS: no.
; Entry: nothing. Exit: as SetGaugeAddress.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SetGaugeAddressB026E:
    lda a:BossGaugeShown
    beq GaugeStepUpB026E_at0
    stz z:zTemp08
    stz z:zTemp09
    cmp #$3F
    bcc SetGaugeAddressB026E_at0
    lda #$C0
    sta z:zTemp08
    inc z:zTemp09
SetGaugeAddressB026E_at0:
    lda #$5E
    sec
    sbc a:BossGaugeShown
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
    jsr SetVramAddresses026F
SetGaugeAddressB026E_ret:
    rts

; @routine 031E:439 GaugeTimerCheckB031E
; @run 6B:8C39
; WHAT IT DOES: While the life timer of slot 85 is zero, the gauge steps (GaugeStepB031E); else the boss gauge's flash
; at $8CDB (as GaugeTimerCheckB, 024E:483). Programs 043E and 10DE hold the same bytes.
; HOW: LDA and the two jumps.
; WAITS: no.
; Entry: nothing. Exit: as the step or the flash taken.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BossGaugeFlash_031E = $8CDB ; the boss gauge's flash (BossGaugeFlash026E's bytes), in the 031D programs
OrVramData_031F = $9640 ; OrVramData's bytes (024F:3A1) in the 031D programs
GaugeTimerCheckB031E:
    lda a:BossGaugeEnding
    beq GaugeStepB031E
    jmp BossGaugeFlash_031E

; @routine 031E:441 GaugeStepB031E
; @run 6B:8C41
; WHAT IT DOES: Moves the gauge one step towards its target, as GaugeStep with this program's VRAM routines (024E:48B). Programs 043E and 10DE hold the same bytes.
; HOW: As GaugeStepB, the VRAM routines at $961C, $9631 and $9640.
; WAITS: no.
; Entry: nothing. Exit: as GaugeStep.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
GaugeStepB031E:
    lda a:OBJ_HEALTH+16
    bmi GaugeStepB031E_at0
    cmp a:BossGaugeShown
    beq GaugeStepUpB031E_at0
    bcs GaugeStepUpB031E
    bsr SetGaugeAddressB031E
    ldx #$0F
    ldy #$FF
    jsr AndVramData_031F
    dec a:BossGaugeShown
    rts
GaugeStepB031E_at0:
    pha
    stz a:OBJ_HEALTH+16
    bsr GaugeTimerCheckB031E
    pla
    sta a:OBJ_HEALTH+16
    ldy a:BossGaugeShown
    bne GaugeStepUpB031E_at0
    sta a:BossGaugeEnding
    rts

; @routine 031E:46D GaugeStepUpB031E
; @run 6B:8C6D
; WHAT IT DOES: Grows the gauge one, as GaugeStepUp with this program's VRAM routines (024E:4B7). Programs 043E and 10DE hold the same bytes.
; HOW: As GaugeStepUpB, the VRAM routines at $961C, $9631 and $9640.
; WAITS: no.
; Entry: nothing. Exit: as GaugeStepUp.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
GaugeStepUpB031E:
    inc a:BossGaugeShown
    bsr SetGaugeAddressB031E
    ldx #$F0
    ldy #$00
    jsr OrVramData_031F
GaugeStepUpB031E_at0:
    rts

; @routine 031E:47A SetGaugeAddressB031E
; @run 6B:8C7A
; WHAT IT DOES: Points the VDC at the gauge's cell for its value, as SetGaugeAddress with this program's VRAM routine (024E:4C4). Programs 043E and 10DE hold the same bytes.
; HOW: As SetGaugeAddressB, the VRAM routines at $961C, $9631 and $9640.
; WAITS: no.
; Entry: nothing. Exit: as SetGaugeAddress.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SetGaugeAddressB031E:
    lda a:BossGaugeShown
    beq GaugeStepUpB031E_at0
    stz z:zTemp08
    stz z:zTemp09
    cmp #$3F
    bcc SetGaugeAddressB031E_at0
    lda #$C0
    sta z:zTemp08
    inc z:zTemp09
SetGaugeAddressB031E_at0:
    lda #$5E
    sec
    sbc a:BossGaugeShown
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
    jsr SetVramAddresses031F
SetGaugeAddressB031E_ret:
    rts

; @routine 026E:4DB BossGaugeFlash026E
; @run 6B:8CDB
; WHAT IT DOES: A frame of the boss gauge's flash: masks four cells, counts the flash's step, makes the object in slot 85 fall
; faster and move, and at the bottom hides it and clears bit 2 of zField8E (as BossGaugeFlash, 024E:525). These are
; 024E:525's instructions $4A lower, every operand into the code moved alike; their return addresses are their own. Programs 031E, 043E, 064E, 10DE, 122E, 1A2E,
; 1B8E and 1CEE hold the same bytes, their call at $8CEC going to their own routine there.
; HOW: BossFlashAddresses026E by zTemp01 (from the wave count's bit 0, two at a time below 8), the routine at $8D1A;
; OBJ_VY of slot 85 plus $0C; ApplyVelocity.
; WAITS: no.
; Entry: nothing. Exit: A the object's Y high, N and Z from it (RMB sets none); X 85.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BossFlashAddresses026E = $8CC3 ; the boss gauge flash's VRAM cells, a word each, in the 026D programs
BossGaugeFlash026E:
    lda z:zWaveCounter
    and #$01
    sta z:zTemp01
BossGaugeFlash026E_at0:
    lda z:zTemp01
    asl a
    tax
    ldy a:BossFlashAddresses026E+1,x
    lda a:BossFlashAddresses026E,x
    tax
    jsr BossFlashMaskCell026E
    inc z:zTemp01
    inc z:zTemp01
    lda z:zTemp01
    cmp #$08
    bcc BossGaugeFlash026E_at0
    inc a:BossFlashStep
    lda a:OBJ_VY_LO+85
    clc
    adc #$0C
    sta a:OBJ_VY_LO+85
    bcc BossGaugeFlash026E_at1
    inc a:OBJ_VY_HI+85
BossGaugeFlash026E_at1:
    ldx #$55
    jsr VecApplyVelocity
    lda a:OBJ_Y_HI+85
    beq SetGaugeAddressB026E_ret
    stz a:OBJ_FRAME+85
    rmb2 z:zField8E
    rts
