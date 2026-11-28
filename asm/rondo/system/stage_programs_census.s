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

; @routine 026E:53A BlendStageTileRows026E
; @run 6B:8D3A
; WHAT IT DOES: Writes five tile colour values into the VRAM words of one tile row step of the stage, from one row of
; StageTileRows or, while a blend runs, from that row and a second one (as BlendStageTileRows, 024E:584). These are
; 024E:584's instructions $4A lower. Programs 031E, 043E, 064E, 10DE, 122E, 1A2E, 1B8E
; and 1CEE hold the same bytes, their tile address setter at $8E3D their own.
; HOW: As BlendStageTileRows, with the tile address setter at $8E3D.
; WAITS: no.
; Entry: X the step (zTemp04), A the first row, Y the second row. Exit: A the last byte written, X the blend step, Y 5,
; N 0, Z 1, C 1, V from the last add.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BlendStageTileRows026E:
    stx z:zTemp04
    sta z:zTemp00
    asl a
    asl a
    adc z:zTemp00
    adc #$34
    sta z:zTemp00
    cla
    adc #$C5
    sta z:zTemp01
    tya
    sta z:zTemp06
    asl a
    asl a
    adc z:zTemp06
    adc #$34
    sta z:zTemp06
    cla
    adc #$C5
    sta z:zTemp07
    jsr SetStageTileVramAddress026E
    lda z:zTemp04
    lsr a
    bcs BlendStageTileRows026E_at0
    cly
    ldx a:StageBlendStep
BlendStageTileRows026E_at4:
    cpx #$05
    beq BlendStageTileRows026E_at1
    lda a:StageBlendOn
    beq BlendStageTileRows026E_at1
    sty z:zTemp05
    txa
    clc
    adc z:zTemp05
    cmp #$05
    bcs BlendStageTileRows026E_at2
    phy
    tay
    lda (zTemp06),y
    ply
    bra BlendStageTileRows026E_at3
BlendStageTileRows026E_at2:
    sbc #$05
    phy
    tay
    lda (zTemp00),y
    ply
    bra BlendStageTileRows026E_at3
BlendStageTileRows026E_at1:
    lda (zTemp00),y
BlendStageTileRows026E_at3:
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
    bne BlendStageTileRows026E_at4
    rts
BlendStageTileRows026E_at0:
    cly
    ldx a:StageBlendStep
BlendStageTileRows026E_at8:
    cpx #$05
    beq BlendStageTileRows026E_at5
    lda a:StageBlendOn
    beq BlendStageTileRows026E_at5
    sty z:zTemp05
    txa
    clc
    adc z:zTemp05
    cmp #$05
    bcs BlendStageTileRows026E_at6
    phy
    tay
    lda (zTemp06),y
    ply
    bra BlendStageTileRows026E_at7
BlendStageTileRows026E_at6:
    sbc #$05
    phy
    tay
    lda (zTemp00),y
    ply
    bra BlendStageTileRows026E_at7
BlendStageTileRows026E_at5:
    lda (zTemp00),y
BlendStageTileRows026E_at7:
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
    bne BlendStageTileRows026E_at8
    inc z:zTemp04
    bsr SetStageTileVramAddress026E
    cly
    ldx a:StageBlendStep
BlendStageTileRows026E_at12:
    cpx #$05
    beq BlendStageTileRows026E_at9
    lda a:StageBlendOn
    beq BlendStageTileRows026E_at9
    sty z:zTemp05
    txa
    clc
    adc z:zTemp05
    cmp #$05
    bcs BlendStageTileRows026E_at10
    phy
    tay
    lda (zTemp06),y
    ply
    bra BlendStageTileRows026E_at11
BlendStageTileRows026E_at10:
    sbc #$05
    phy
    tay
    lda (zTemp00),y
    ply
    bra BlendStageTileRows026E_at11
BlendStageTileRows026E_at9:
    lda (zTemp00),y
BlendStageTileRows026E_at11:
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
    bne BlendStageTileRows026E_at12
    rts

; @routine 026F:224 BlendStageRowPair026F
; @run 6B:9224
; WHAT IT DOES: Blends the two rows a byte names: its low nibble's row at the step X, its high nibble's row at the step after
; (as BlendStageRowPair, 024F:266). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: BlendStageTileRows026E for the low nibble, called, then the same for the high nibble with the step one on, a
; tail jump.
; WAITS: no.
; Entry: A the byte, X the step, Y the second row of the first blend. Exit: as BlendStageTileRows026E.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BlendStageRowPair026F:
    phx
    pha
    and #$0F
    jsr BlendStageTileRows026E
    pla
    lsr a
    lsr a
    lsr a
    lsr a
    plx
    inx
    jmp BlendStageTileRows026E

; @routine 031F:218 BlendStageRowPair031F
; @run 6B:9218
; WHAT IT DOES: Blends the two rows a byte names: its low nibble's row at the step X, its high nibble's row at the step after
; (as BlendStageRowPair, 024F:266). Programs 043F and 10DF hold the same bytes; these are 026F:224's at $9218, filed again
; because the JSR's return address is its own.
; HOW: BlendStageTileRows026E for the low nibble, called, then the same for the high nibble with the step one on, a
; tail jump.
; WAITS: no.
; Entry: A the byte, X the step, Y the second row of the first blend. Exit: as BlendStageTileRows026E.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BlendStageRowPair031F:
    phx
    pha
    and #$0F
    jsr BlendStageTileRows026E
    pla
    lsr a
    lsr a
    lsr a
    lsr a
    plx
    inx
    jmp BlendStageTileRows026E

; @routine 026F:1F4 BlendStageRowChanges026F
; @run 6B:91F4
; WHAT IT DOES: Blends the rows a byte changed: for each nibble of the old byte (zBiosAl) that differs from the new (A), the old
; nibble's row is blended in at the step X or the step after (as BlendStageRowChanges, 024F:236). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: The low nibbles compared, BlendStageTileRows026E called when they differ; then the high nibbles one step on.
; WAITS: no.
; Entry: A the new byte, zBiosAl the old, X the step. Exit: as BlendStageRowChanges.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BlendStageRowChanges026F:
    phx
    sta z:zBiosBl
    lda z:zBiosAl
    and #$0F
    sta z:zBiosAh
    lda z:zBiosBl
    and #$0F
    cmp z:zBiosAh
    beq BlendStageRowChanges026F_at0
    ldy z:zBiosAh
    jsr BlendStageTileRows026E
BlendStageRowChanges026F_at0:
    lda z:zBiosAl
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zBiosAh
    lda z:zBiosBl
    lsr a
    lsr a
    lsr a
    lsr a
    plx
    inx
    cmp z:zBiosAh
    beq BlendStageRowChanges026F_at1
    ldy z:zBiosAh
    jsr BlendStageTileRows026E
BlendStageRowChanges026F_at1:
    rts

; @routine 031F:1E8 BlendStageRowChanges031F
; @run 6B:91E8
; WHAT IT DOES: Blends the rows a byte changed: for each nibble of the old byte (zBiosAl) that differs from the new (A), the old
; nibble's row is blended in at the step X or the step after (as BlendStageRowChanges, 024F:236). Programs 043F and 10DF hold the same bytes; these are 026F:1F4's at $91E8, filed again
; because the JSRs' return addresses are their own.
; HOW: The low nibbles compared, BlendStageTileRows026E called when they differ; then the high nibbles one step on.
; WAITS: no.
; Entry: A the new byte, zBiosAl the old, X the step. Exit: as BlendStageRowChanges.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BlendStageRowChanges031F:
    phx
    sta z:zBiosBl
    lda z:zBiosAl
    and #$0F
    sta z:zBiosAh
    lda z:zBiosBl
    and #$0F
    cmp z:zBiosAh
    beq BlendStageRowChanges031F_at0
    ldy z:zBiosAh
    jsr BlendStageTileRows026E
BlendStageRowChanges031F_at0:
    lda z:zBiosAl
    lsr a
    lsr a
    lsr a
    lsr a
    sta z:zBiosAh
    lda z:zBiosBl
    lsr a
    lsr a
    lsr a
    lsr a
    plx
    inx
    cmp z:zBiosAh
    beq BlendStageRowChanges031F_at1
    ldy z:zBiosAh
    jsr BlendStageTileRows026E
BlendStageRowChanges031F_at1:
    rts

; @routine 026F:10F BlendStageStockRows026F
; @run 6B:910F
; WHAT IT DOES: Shows the player's stock on the stage's tile rows when it changed (as BlendStageStockRows, 024F:151). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: SaveAndMapStageBanks' bytes, BlendStageRowPair026F, a tail jump into RestoreStageBanks' bytes.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as BlendStageStockRows.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SaveAndMapStageBanks_026F = $92FD ; SaveAndMapStageBanks' bytes (024F:33F) in the 026D programs
RestoreStageBanks_026F = $9312 ; RestoreStageBanks' bytes (024F:354) in the 026D programs
BlendStageStockRows026F:
    jsr SaveAndMapStageBanks_026F
    lda a:StageBlendOn+1
    beq BlendStageStockRows026F_at0
    lda a:StageVar3616
    cmp z:zStock
    beq BlendStageStockRows026F_at1
    lda z:zGameSubMode
    cmp #$01
    bne BlendStageStockRows026F_at1
    smb1 z:zPaused
    lda #$C0
    sta a:StageVar3616+1
    jmp RestoreStageBanks_026F
BlendStageStockRows026F_at0:
    phx
    stz a:StageBlendOn
    lda z:zStock
    sta a:StageVar3616
    clx
    jsr BlendStageRowPair026F
    plx
BlendStageStockRows026F_at1:
    jmp RestoreStageBanks_026F

; @routine 031F:103 BlendStageStockRows031F
; @run 6B:9103
; WHAT IT DOES: Shows the player's stock on the stage's tile rows when it changed (as BlendStageStockRows, 024F:151). Programs 043F and 10DF hold the same bytes.
; HOW: SaveAndMapStageBanks' bytes, BlendStageRowPair031F, a tail jump into RestoreStageBanks' bytes.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as BlendStageStockRows.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
SaveAndMapStageBanks_031F = $95DE ; SaveAndMapStageBanks' bytes (024F:33F) in the 031D programs
RestoreStageBanks_031F = $95F3 ; RestoreStageBanks' bytes (024F:354) in the 031D programs
BlendStageStockRows031F:
    jsr SaveAndMapStageBanks_031F
    lda a:StageBlendOn+1
    beq BlendStageStockRows031F_at0
    lda a:StageVar3616
    cmp z:zStock
    beq BlendStageStockRows031F_at1
    lda z:zGameSubMode
    cmp #$01
    bne BlendStageStockRows031F_at1
    smb1 z:zPaused
    lda #$C0
    sta a:StageVar3616+1
    jmp RestoreStageBanks_031F
BlendStageStockRows031F_at0:
    phx
    stz a:StageBlendOn
    lda z:zStock
    sta a:StageVar3616
    clx
    jsr BlendStageRowPair031F
    plx
BlendStageStockRows031F_at1:
    jmp RestoreStageBanks_031F

; @routine 026F:13F BlendStageScoreAddRows026F
; @run 6B:913F
; WHAT IT DOES: Shows the score being added on the stage's tile rows while the blend is off (as BlendStageScoreAddRows,
; 024F:181). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: SaveAndMapStageBanks' bytes, BlendStageRowPair026F twice, a tail jump into RestoreStageBanks' bytes.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as BlendStageScoreAddRows.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BlendStageScoreAddRows026F:
    jsr SaveAndMapStageBanks_026F
    lda a:StageBlendOn+1
    bne BlendStageStockRows026F_at1
    phx
    stz a:StageBlendOn
    lda z:zScoreAddLo
    ldx #$03
    jsr BlendStageRowPair026F
    lda z:zScoreAddHi
    ldx #$05
    jsr BlendStageRowPair026F
    plx
    jmp RestoreStageBanks_026F

; @routine 031F:133 BlendStageScoreAddRows031F
; @run 6B:9133
; WHAT IT DOES: Shows the score being added on the stage's tile rows while the blend is off (as BlendStageScoreAddRows,
; 024F:181). Programs 043F and 10DF hold the same bytes.
; HOW: SaveAndMapStageBanks' bytes, BlendStageRowPair031F twice, a tail jump into RestoreStageBanks' bytes.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as BlendStageScoreAddRows.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BlendStageScoreAddRows031F:
    jsr SaveAndMapStageBanks_031F
    lda a:StageBlendOn+1
    bne BlendStageStockRows031F_at1
    phx
    stz a:StageBlendOn
    lda z:zScoreAddLo
    ldx #$03
    jsr BlendStageRowPair031F
    lda z:zScoreAddHi
    ldx #$05
    jsr BlendStageRowPair031F
    plx
    jmp RestoreStageBanks_031F

; @routine 026F:16A BlendStageScoreRows026F
; @run 6B:916A
; WHAT IT DOES: Shows the score on the stage's tile rows, a row of changes a step, when it changed (as BlendStageScoreRows,
; 024F:1AC). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: SaveAndMapStageBanks' bytes, BlendStageRowChanges026F for the score's three bytes, a tail jump into
; RestoreStageBanks' bytes.
; WAITS: no.
; Entry: X the caller's. Exit: as BlendStageScoreRows.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BlendStageScoreRows026F:
    jsr SaveAndMapStageBanks_026F
    lda a:StageBlendOn+1
    beq BlendStageScoreRows026F_at0
    sta a:StageBlendOn
    lda a:StageBlendStep
    bne BlendStageScoreRows026F_at1
    lda z:zGameSubMode
    cmp #$06
    beq BlendStageScoreRows026F_at2
    bbr1 zPaused,BlendStageScoreRows026F_at3
BlendStageScoreRows026F_at2:
    lda #$05
    sta a:StageBlendStep
BlendStageScoreRows026F_at3:
    lda a:StageVar350E
    cmp z:zScore0
    bne BlendStageScoreRows026F_at1
    lda a:StageVar3566
    cmp z:zScore1
    bne BlendStageScoreRows026F_at1
    lda a:StageVar35BE
    cmp z:zScore2
    beq BlendStageScoreRows026F_at0
    inc a:StageBlendStep
    lda z:zScore0
    sta a:StageVar371E
    lda z:zScore1
    sta a:$3776
    lda z:zScore2
    sta a:$37CE
BlendStageScoreRows026F_at1:
    phx
    lda a:StageVar350E
    sta z:zBiosAl
    lda z:zScore0
    ldx #$01
    jsr BlendStageRowChanges026F
    lda a:StageVar3566
    sta z:zBiosAl
    lda z:zScore1
    ldx #$03
    jsr BlendStageRowChanges026F
    lda a:StageVar35BE
    sta z:zBiosAl
    lda z:zScore2
    ldx #$05
    jsr BlendStageRowChanges026F
    plx
    inc a:StageBlendStep
    lda a:StageBlendStep
    cmp #$06
    bne BlendStageScoreRows026F_at0
    stz a:StageBlendStep
    lda z:zScore0
    sta a:StageVar350E
    lda z:zScore1
    sta a:StageVar3566
    lda z:zScore2
    sta a:StageVar35BE
BlendStageScoreRows026F_at0:
    jmp RestoreStageBanks_026F

; @routine 031F:15E BlendStageScoreRows031F
; @run 6B:915E
; WHAT IT DOES: Shows the score on the stage's tile rows, a row of changes a step, when it changed (as BlendStageScoreRows,
; 024F:1AC). Programs 043F and 10DF hold the same bytes.
; HOW: SaveAndMapStageBanks' bytes, BlendStageRowChanges031F for the score's three bytes, a tail jump into
; RestoreStageBanks' bytes.
; WAITS: no.
; Entry: X the caller's. Exit: as BlendStageScoreRows.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
BlendStageScoreRows031F:
    jsr SaveAndMapStageBanks_031F
    lda a:StageBlendOn+1
    beq BlendStageScoreRows031F_at0
    sta a:StageBlendOn
    lda a:StageBlendStep
    bne BlendStageScoreRows031F_at1
    lda z:zGameSubMode
    cmp #$06
    beq BlendStageScoreRows031F_at2
    bbr1 zPaused,BlendStageScoreRows031F_at3
BlendStageScoreRows031F_at2:
    lda #$05
    sta a:StageBlendStep
BlendStageScoreRows031F_at3:
    lda a:StageVar350E
    cmp z:zScore0
    bne BlendStageScoreRows031F_at1
    lda a:StageVar3566
    cmp z:zScore1
    bne BlendStageScoreRows031F_at1
    lda a:StageVar35BE
    cmp z:zScore2
    beq BlendStageScoreRows031F_at0
    inc a:StageBlendStep
    lda z:zScore0
    sta a:StageVar371E
    lda z:zScore1
    sta a:$3776
    lda z:zScore2
    sta a:$37CE
BlendStageScoreRows031F_at1:
    phx
    lda a:StageVar350E
    sta z:zBiosAl
    lda z:zScore0
    ldx #$01
    jsr BlendStageRowChanges031F
    lda a:StageVar3566
    sta z:zBiosAl
    lda z:zScore1
    ldx #$03
    jsr BlendStageRowChanges031F
    lda a:StageVar35BE
    sta z:zBiosAl
    lda z:zScore2
    ldx #$05
    jsr BlendStageRowChanges031F
    plx
    inc a:StageBlendStep
    lda a:StageBlendStep
    cmp #$06
    bne BlendStageScoreRows031F_at0
    stz a:StageBlendStep
    lda z:zScore0
    sta a:StageVar350E
    lda z:zScore1
    sta a:StageVar3566
    lda z:zScore2
    sta a:StageVar35BE
BlendStageScoreRows031F_at0:
    jmp RestoreStageBanks_031F

; @routine 026F:06F MaskVramRowsRandom026F
; @run 6B:906F
; WHAT IT DOES: Masks a region of VRAM with random bits: copied into the mask buffer, masked row by row, and written back (as
; MaskVramRowsRandom, 024F:0B1). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: As MaskVramRowsRandom, with this program's VRAM address setter and row masks.
; WAITS: no.
; Entry: zTemp10 and zTemp11 the VRAM address. Exit: as MaskVramRowsRandom.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
MaskVramRowsRandom026F:
    ldx z:zTemp10
    ldy z:zTemp11
    jsr SetVramAddresses026F
    ldx #$80
MaskVramRowsRandom026F_at0:
    lda a:VDC_DATA_LO
    sta a:$3B3F,x
    lda a:VDC_DATA_HI
    sta a:$3B3E,x
    dex
    dex
    bne MaskVramRowsRandom026F_at0
    ldy z:SpritesLeft
    cpy #$10
    bcc MaskVramRowsRandom026F_at1
    ldx #$7F
MaskVramRowsRandom026F_at2:
    stz a:StageMaskBuffer,x
    dex
    bpl MaskVramRowsRandom026F_at2
    jmp MaskVramRowsRandom026F_at3
MaskVramRowsRandom026F_at1:
    dey
    beq MaskVramRowsRandom026F_at4
    clx
MaskVramRowsRandom026F_at5:
    lda z:zRandom2
    sec
    ror a
    ora z:zRandom2
    sta z:zTemp00
    jsr MaskStageRowCopy026F
    inx
    inx
    dey
    bne MaskVramRowsRandom026F_at5
MaskVramRowsRandom026F_at4:
    ldy z:SpritesLeft
    clx
MaskVramRowsRandom026F_at6:
    lda z:zRandom2
    ora z:zRandom3
    sta z:zTemp00
    jsr MaskStageRow026F
    inx
    inx
    dey
    bne MaskVramRowsRandom026F_at6
MaskVramRowsRandom026F_at3:
    ldx #$80
MaskVramRowsRandom026F_at7:
    lda a:$3B3F,x
    sta a:VDC_DATA_LO
    lda a:$3B3E,x
    sta a:VDC_DATA_HI
    dex
    dex
    bne MaskVramRowsRandom026F_at7
    rts

; @routine 031F:063 MaskVramRowsRandom031F
; @run 6B:9063
; WHAT IT DOES: Masks a region of VRAM with random bits: copied into the mask buffer, masked row by row, and written back (as
; MaskVramRowsRandom, 024F:0B1). Programs 043F and 10DF hold the same bytes.
; HOW: As MaskVramRowsRandom, with this program's VRAM address setter and row masks.
; WAITS: no.
; Entry: zTemp10 and zTemp11 the VRAM address. Exit: as MaskVramRowsRandom.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
MaskVramRowsRandom031F:
    ldx z:zTemp10
    ldy z:zTemp11
    jsr SetVramAddresses031F
    ldx #$80
MaskVramRowsRandom031F_at0:
    lda a:VDC_DATA_LO
    sta a:$3B3F,x
    lda a:VDC_DATA_HI
    sta a:$3B3E,x
    dex
    dex
    bne MaskVramRowsRandom031F_at0
    ldy z:SpritesLeft
    cpy #$10
    bcc MaskVramRowsRandom031F_at1
    ldx #$7F
MaskVramRowsRandom031F_at2:
    stz a:StageMaskBuffer,x
    dex
    bpl MaskVramRowsRandom031F_at2
    jmp MaskVramRowsRandom031F_at3
MaskVramRowsRandom031F_at1:
    dey
    beq MaskVramRowsRandom031F_at4
    clx
MaskVramRowsRandom031F_at5:
    lda z:zRandom2
    sec
    ror a
    ora z:zRandom2
    sta z:zTemp00
    jsr MaskStageRowCopy031F
    inx
    inx
    dey
    bne MaskVramRowsRandom031F_at5
MaskVramRowsRandom031F_at4:
    ldy z:SpritesLeft
    clx
MaskVramRowsRandom031F_at6:
    lda z:zRandom2
    ora z:zRandom3
    sta z:zTemp00
    jsr MaskStageRow031F
    inx
    inx
    dey
    bne MaskVramRowsRandom031F_at6
MaskVramRowsRandom031F_at3:
    ldx #$80
MaskVramRowsRandom031F_at7:
    lda a:$3B3F,x
    sta a:VDC_DATA_LO
    lda a:$3B3E,x
    sta a:VDC_DATA_HI
    dex
    dex
    bne MaskVramRowsRandom031F_at7
    rts

; @routine 026F:29D ClearStageBitRows026F
; @run 6B:929D
; WHAT IT DOES: Clears the stage's bit in four rows of VRAM words (as ClearStageBitRows, 024F:2DF). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: As ClearStageBitRows, with SetVramAddresses026F and StageClearBitLo026F/StageClearBitHi026F.
; WAITS: no.
; Entry: X kept for the caller; zTemp08-09 the base, zTemp0F the place. Exit: as ClearStageBitRows.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearBitLo026F = $92E5 ; the low byte of the stage's bit in the cleared word, by stage index, in the 026D programs
StageClearBitHi026F = $92ED ; and its high byte
ClearStageBitRows026F:
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
ClearStageBitRows026F_at0:
    phx
    phy
    ldx z:zTemp0A
    ldy z:zTemp0B
    jsr SetVramAddresses026F
    ply
    lda a:VDC_DATA_LO
    and a:StageClearBitLo026F,y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and a:StageClearBitHi026F,y
    sta a:VDC_DATA_HI
    plx
    lda z:zTemp0A
    clc
    adc #$10
    sta z:zTemp0A
    dec z:zTemp0E
    bne ClearStageBitRows026F_at0
    lda z:zTemp0F
    clc
    adc #$23
    sta z:zTemp0F
    rts

; @routine 031F:291 ClearStageBitRows031F
; @run 6B:9291
; WHAT IT DOES: Clears the stage's bit in four rows of VRAM words (as ClearStageBitRows, 024F:2DF). Programs 043F and 10DF hold the same bytes.
; HOW: As ClearStageBitRows, with SetVramAddresses031F and StageClearBitLo031F/StageClearBitHi031F.
; WAITS: no.
; Entry: X kept for the caller; zTemp08-09 the base, zTemp0F the place. Exit: as ClearStageBitRows.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearBitLo031F = $92D9 ; the low byte of the stage's bit in the cleared word, by stage index, in the 031D programs
StageClearBitHi031F = $92E1 ; and its high byte
ClearStageBitRows031F:
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
ClearStageBitRows031F_at0:
    phx
    phy
    ldx z:zTemp0A
    ldy z:zTemp0B
    jsr SetVramAddresses031F
    ply
    lda a:VDC_DATA_LO
    and a:StageClearBitLo031F,y
    sta a:VDC_DATA_LO
    lda a:VDC_DATA_HI
    and a:StageClearBitHi031F,y
    sta a:VDC_DATA_HI
    plx
    lda z:zTemp0A
    clc
    adc #$10
    sta z:zTemp0A
    dec z:zTemp0E
    bne ClearStageBitRows031F_at0
    lda z:zTemp0F
    clc
    adc #$23
    sta z:zTemp0F
    rts

; @routine 026F:297 ClearStageBitRowsFour026F
; @run 6B:9297
; WHAT IT DOES: Clears the stage's bit in four rows of VRAM words, four times over (as ClearStageBitRowsFour, 024F:2D9). Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: Three BSRs to ClearStageBitRows026F and the fall into it.
; WAITS: no.
; Entry: as ClearStageBitRows026F. Exit: as the last ClearStageBitRows026F.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ClearStageBitRowsFour026F:
    bsr ClearStageBitRows026F
    bsr ClearStageBitRows026F
    bsr ClearStageBitRows026F

; @routine 031F:28B ClearStageBitRowsFour031F
; @run 6B:928B
; WHAT IT DOES: Clears the stage's bit in four rows of VRAM words, four times over (as ClearStageBitRowsFour, 024F:2D9). Programs 043F and 10DF hold the same bytes.
; HOW: Three BSRs to ClearStageBitRows031F and the fall into it.
; WAITS: no.
; Entry: as ClearStageBitRows031F. Exit: as the last ClearStageBitRows031F.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ClearStageBitRowsFour031F:
    bsr ClearStageBitRows031F
    bsr ClearStageBitRows031F
    bsr ClearStageBitRows031F

; @routine 026F:560 HitStoreHandler026F
; @run 6B:9560
; WHAT IT DOES: The hit store handler of the stage programs (as HitStoreHandler, 024F:543): the hit slot's value into
; the object's hit-taken byte; when the first object's hit-taken byte is empty and the byte at $2745 is 0, the first
; object's hit-taken byte and health take the object's two stored bytes. Programs 064F, 122F, 1A2F, 1B8F and 1CEF
; hold the same bytes, and 031F, 043F and 10DF at $97E2.
; HOW: HitStoreHandler's code with a test of $2745 before the two copies.
; WAITS: no.
; Entry: X the object, Y the hit slot. Exit: A the last byte loaded, N and Z from it. X, Y, C and V kept.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
HitStoreHandler026F:
    lda a:OBJ_WORK_N,y
    sta a:OBJ_HIT_TAKEN,x
    lda a:OBJ_HIT_TAKEN
    bne HitStoreHandler026F_at0
    lda a:HitStoreGate
    bne HitStoreHandler026F_at0
    lda a:OBJ_WORK_C,x
    sta a:OBJ_HIT_TAKEN
    lda a:OBJ_WORK_B,x
    sta a:OBJ_HEALTH
HitStoreHandler026F_at0:
    rts

; @routine 026F:483 HitTestObject026F
; @run 6B:9483
; @stop 9542
; WHAT IT DOES: The stage programs' hit test (as HitTestObject, 024F:466): the object X's hitbox against the other
; side's slots and, on the first overlap, the handler its kind names in this program's table at $9542. Programs 064F,
; 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: HitTestObject's code with this program's table; with no handler for the kind it returns through
; HitStoreHandler026F's RTS.
; WAITS: no.
; Entry: X the object. Exit: with no hit A the last value, Y $FF, N set; with one, as the handler.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
HitHandlers026F = $9542 ; the hit handler of each kind from 1, a word each
HitTestObject026F:
    stz a:OBJ_HIT_TAKEN,x
    lda a:OBJ_HITBOX_ON,x
    bne HitTestObject026F_at0
    jmp HitTestObject026F_at1
HitTestObject026F_at0:
    lda a:OBJ_HITBOX_HALF_W,x
    ora a:OBJ_HITBOX_HALF_H,x
    bne HitTestObject026F_at2
    jmp HitTestObject026F_at1
HitTestObject026F_at2:
    lda a:OBJ_HITBOX_DX,x
    sta z:zTemp01
    lda a:OBJ_FACING_LEFT,x
    beq HitTestObject026F_at3
    lda z:zTemp01
    eor #$FF
    inc a
    sta z:zTemp01
HitTestObject026F_at3:
    stz z:zTemp00
    lda z:zTemp01
    bpl HitTestObject026F_at4
    dec z:zTemp00
HitTestObject026F_at4:
    clc
    adc a:OBJ_X_LO,x
    sta a:WorkByteA
    lda a:OBJ_X_HI,x
    adc z:zTemp00
    bne HitTestObject026F_at1
    stz z:zTemp00
    lda a:OBJ_HITBOX_DY,x
    bpl HitTestObject026F_at5
    dec z:zTemp00
HitTestObject026F_at5:
    clc
    adc a:OBJ_Y_LO,x
    sta a:WorkByteB
    lda a:OBJ_Y_HI,x
    adc z:zTemp00
    bne HitTestObject026F_at1
    lda a:OBJ_HITBOX_ON,x
    bpl HitTestObject026F_at6
    cly
    bra HitTestObject026F_at7
HitTestObject026F_at6:
    ldy #$0F
HitTestObject026F_at7:
    lda a:OBJ_HITBOX_ON,y
    beq HitTestObject026F_at8
    lda a:HitboxHi,y
    bne HitTestObject026F_at8
    lda a:OBJ_HITBOX_HALF_W,x
    clc
    adc a:OBJ_HITBOX_HALF_W,y
    sta a:StreamStepLo
    lda a:OBJ_HITBOX_HALF_H,x
    clc
    adc a:OBJ_HITBOX_HALF_H,y
    sta a:StreamStepHi
    lda a:WorkByteA
    sec
    sbc a:HitboxXLo,y
    bcs HitTestObject026F_at9
    eor #$FF
    inc a
HitTestObject026F_at9:
    cmp a:StreamStepLo
    bcs HitTestObject026F_at8
    lda a:WorkByteB
    sec
    sbc a:HitboxYLo,y
    bcs HitTestObject026F_at10
    eor #$FF
    inc a
HitTestObject026F_at10:
    cmp a:StreamStepHi
    bcc HitTestObject026F_at11
HitTestObject026F_at8:
    dey
    bpl HitTestObject026F_at7
HitTestObject026F_at1:
    rts
HitTestObject026F_at11:
    txa
    sta a:OBJ_WORK_O,y
    sty z:zHitSlot
    lda a:OBJ_WORK_N,y
    beq HitStoreHandler026F_at0
    dec a
    asl a
    tay
    lda a:HitHandlers026F,y
    sta z:zTemp00
    lda a:HitHandlers026F+1,y
    sta z:zTemp01
    ldy z:zHitSlot
    jmp (ZeroPage)

; @routine 031F:705 HitTestObject031F
; @run 6B:9705
; @stop 97C4
; WHAT IT DOES: The hit test of programs 031F, 043F and 10DF (as HitTestObject, 024F:466), with the table at $97C4.
; HOW: HitTestObject's code with this program's table; with no handler for the kind it returns through the RTS of the
; store handler's bytes at $97E2 (HitStoreHandler026F's).
; WAITS: no.
; Entry, exit: as HitTestObject026F.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
HitHandlers031F = $97C4 ; the hit handler of each kind from 1, a word each
HitStoreRts031F = HitTestObject031F+$F9 ; the RTS of the store handler's bytes at $97E2
HitTestObject031F:
    stz a:OBJ_HIT_TAKEN,x
    lda a:OBJ_HITBOX_ON,x
    bne HitTestObject031F_at0
    jmp HitTestObject031F_at1
HitTestObject031F_at0:
    lda a:OBJ_HITBOX_HALF_W,x
    ora a:OBJ_HITBOX_HALF_H,x
    bne HitTestObject031F_at2
    jmp HitTestObject031F_at1
HitTestObject031F_at2:
    lda a:OBJ_HITBOX_DX,x
    sta z:zTemp01
    lda a:OBJ_FACING_LEFT,x
    beq HitTestObject031F_at3
    lda z:zTemp01
    eor #$FF
    inc a
    sta z:zTemp01
HitTestObject031F_at3:
    stz z:zTemp00
    lda z:zTemp01
    bpl HitTestObject031F_at4
    dec z:zTemp00
HitTestObject031F_at4:
    clc
    adc a:OBJ_X_LO,x
    sta a:WorkByteA
    lda a:OBJ_X_HI,x
    adc z:zTemp00
    bne HitTestObject031F_at1
    stz z:zTemp00
    lda a:OBJ_HITBOX_DY,x
    bpl HitTestObject031F_at5
    dec z:zTemp00
HitTestObject031F_at5:
    clc
    adc a:OBJ_Y_LO,x
    sta a:WorkByteB
    lda a:OBJ_Y_HI,x
    adc z:zTemp00
    bne HitTestObject031F_at1
    lda a:OBJ_HITBOX_ON,x
    bpl HitTestObject031F_at6
    cly
    bra HitTestObject031F_at7
HitTestObject031F_at6:
    ldy #$0F
HitTestObject031F_at7:
    lda a:OBJ_HITBOX_ON,y
    beq HitTestObject031F_at8
    lda a:HitboxHi,y
    bne HitTestObject031F_at8
    lda a:OBJ_HITBOX_HALF_W,x
    clc
    adc a:OBJ_HITBOX_HALF_W,y
    sta a:StreamStepLo
    lda a:OBJ_HITBOX_HALF_H,x
    clc
    adc a:OBJ_HITBOX_HALF_H,y
    sta a:StreamStepHi
    lda a:WorkByteA
    sec
    sbc a:HitboxXLo,y
    bcs HitTestObject031F_at9
    eor #$FF
    inc a
HitTestObject031F_at9:
    cmp a:StreamStepLo
    bcs HitTestObject031F_at8
    lda a:WorkByteB
    sec
    sbc a:HitboxYLo,y
    bcs HitTestObject031F_at10
    eor #$FF
    inc a
HitTestObject031F_at10:
    cmp a:StreamStepHi
    bcc HitTestObject031F_at11
HitTestObject031F_at8:
    dey
    bpl HitTestObject031F_at7
HitTestObject031F_at1:
    rts
HitTestObject031F_at11:
    txa
    sta a:OBJ_WORK_O,y
    sty z:zHitSlot
    lda a:OBJ_WORK_N,y
    beq HitStoreRts031F
    dec a
    asl a
    tay
    lda a:HitHandlers031F,y
    sta z:zTemp00
    lda a:HitHandlers031F+1,y
    sta z:zTemp01
    ldy z:zHitSlot
    jmp (ZeroPage)

; @routine 026F:415 HitTestAllObjects026F
; @run 6B:9415
; WHAT IT DOES: One frame of hit testing in the stage programs (as HitTestAllObjects, 024F:3F8): places the first
; sixteen hitboxes and tests the objects from $57 down to $10, clearing the hit-taken byte of each whose timer is
; running and counting that timer down. Programs 064F, 122F, 1A2F, 1B8F and 1CEF hold the same bytes.
; HOW: A JSR to ComputeHitboxPositions' bytes at $9430, then HitTestObject026F for each object.
; WAITS: no.
; Entry: X kept on the stack. Exit: as the last object left it; X kept, C 0.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
HitboxPositions026F = $9430 ; ComputeHitboxPositions' bytes in these programs
HitTestAllObjects026F:
    phx
    jsr HitboxPositions026F
    ldx #$57
HitTestAllObjects026F_at1:
    jsr HitTestObject026F
    lda a:OBJ_INVULN_TIMER,x
    beq HitTestAllObjects026F_at0
    stz a:OBJ_HIT_TAKEN,x
    dec a:OBJ_INVULN_TIMER,x
HitTestAllObjects026F_at0:
    dex
    cpx #$10
    bcs HitTestAllObjects026F_at1
    plx
    rts

; @routine 031F:697 HitTestAllObjects031F
; @run 6B:9697
; WHAT IT DOES: One frame of hit testing in programs 031F, 043F and 10DF (as HitTestAllObjects, 024F:3F8).
; HOW: A JSR to ComputeHitboxPositions' bytes at $96B2, then HitTestObject031F for each object.
; WAITS: no.
; Entry, exit: as HitTestAllObjects026F.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
HitboxPositions031F = $96B2 ; ComputeHitboxPositions' bytes in these programs
HitTestAllObjects031F:
    phx
    jsr HitboxPositions031F
    ldx #$57
HitTestAllObjects031F_at1:
    jsr HitTestObject031F
    lda a:OBJ_INVULN_TIMER,x
    beq HitTestAllObjects031F_at0
    stz a:OBJ_HIT_TAKEN,x
    dec a:OBJ_INVULN_TIMER,x
HitTestAllObjects031F_at0:
    dex
    cpx #$10
    bcs HitTestAllObjects031F_at1
    plx
    rts

; @routine 026F:6A1 TerrainTileRead026F
; @run 6B:96A1
; WHAT IT DOES: The terrain tile read of the stage programs (as TerrainTileRead, 050F:67F), with this program's
; handler table at $973C. Programs 064F, 122F, 1A2F, 1B8F, 1CEF hold the same bytes.
; HOW: TerrainTileRead's code at $96A1.
; WAITS: no.
; Entry: zTemp00-01 X, zTemp02-03 Y. Exit: as TerrainTileRead.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileHandlers026F = $973C ; the handlers for attributes $80-$9B, a word each
TerrainTileRead026F:
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
    lda z:zTemp05
    sta z:zTemp06
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
    bmi TerrainTileRead026F_at0
    lda z:zTemp08
    clc
    adc #$00
    sta z:zTemp08
    lda a:SaveBuffer,y
    adc #$7D
    sta z:zTemp09
    lda (zTemp08)
    bmi TerrainTileRead026F_at1
    rts
TerrainTileRead026F_at0:
    lda #$00
    rts
TerrainTileRead026F_at1:
    cmp #$9C
    bcs TerrainTileRead026F_at0+2
    pha
    sbc #$7F
    asl a
    tay
    lda a:TileHandlers026F,y
    sta z:zTemp04
    lda a:TileHandlers026F+1,y
    sta z:zTemp05
    pla
    jmp (zTemp04+ZeroPage)

; @routine 026F:5CD TerrainLookup026F
; @run 6B:95CD
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB), from
; this program's probe table at $D2BB.
; HOW: TerrainLookupB's code with this program's table, reading with the read at $96A1.
; WAITS: no.
; Entry: A probe, X object. Exit: as TerrainLookupB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ProbeTable026F = $D2BB ; the probe table: a pointer to each probe's four columns of offsets
TerrainLookup026F:
    asl a
    tay
    lda a:ProbeTable026F,y
    sta z:zTemp0C
    lda a:ProbeTable026F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup026F_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup026F_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TerrainTileRead026F
TerrainLookup026F_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup026F_at1
    rts
TerrainLookup026F_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup026F_at2

; @routine 026F:62D TerrainLookupAlt026F
; @run 6B:962D
; WHAT IT DOES: TerrainLookup026F, then a second look at each column that found floor (as TerrainLookupAltB, 024F:60B).
; HOW: TerrainLookupAltB's code with this program's table.
; WAITS: no.
; Entry and exit: as TerrainLookup026F.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TerrainLookupAlt026F:
    pha
    jsr TerrainLookup026F
    pla
    asl a
    tay
    lda a:ProbeTable026F,y
    sta z:zTemp0C
    lda a:ProbeTable026F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt026F_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt026F_at0
    beq TerrainLookupAlt026F_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
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
    jsr TerrainTileRead026F
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt026F_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt026F_at1:
    ply
TerrainLookupAlt026F_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt026F_at2
    rts
TerrainLookupAlt026F_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt026F_at3

; @routine 064F:5CD TerrainLookup064F
; @run 6B:95CD
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB), from
; this program's probe table at $B63B.
; HOW: TerrainLookupB's code with this program's table, reading with the read at $96A1.
; WAITS: no.
; Entry: A probe, X object. Exit: as TerrainLookupB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ProbeTable064F = $B63B ; the probe table: a pointer to each probe's four columns of offsets
TileRead064F = $96A1 ; TerrainTileRead026F's bytes in this program
TerrainLookup064F:
    asl a
    tay
    lda a:ProbeTable064F,y
    sta z:zTemp0C
    lda a:ProbeTable064F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup064F_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup064F_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TileRead064F
TerrainLookup064F_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup064F_at1
    rts
TerrainLookup064F_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup064F_at2

; @routine 064F:62D TerrainLookupAlt064F
; @run 6B:962D
; WHAT IT DOES: TerrainLookup064F, then a second look at each column that found floor (as TerrainLookupAltB, 024F:60B).
; HOW: TerrainLookupAltB's code with this program's table.
; WAITS: no.
; Entry and exit: as TerrainLookup064F.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TerrainLookupAlt064F:
    pha
    jsr TerrainLookup064F
    pla
    asl a
    tay
    lda a:ProbeTable064F,y
    sta z:zTemp0C
    lda a:ProbeTable064F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt064F_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt064F_at0
    beq TerrainLookupAlt064F_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
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
    jsr TileRead064F
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt064F_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt064F_at1:
    ply
TerrainLookupAlt064F_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt064F_at2
    rts
TerrainLookupAlt064F_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt064F_at3

; @routine 122F:5CD TerrainLookup122F
; @run 6B:95CD
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB), from
; this program's probe table at $D4C4.
; HOW: TerrainLookupB's code with this program's table, reading with the read at $96A1.
; WAITS: no.
; Entry: A probe, X object. Exit: as TerrainLookupB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ProbeTable122F = $D4C4 ; the probe table: a pointer to each probe's four columns of offsets
TileRead122F = $96A1 ; TerrainTileRead026F's bytes in this program
TerrainLookup122F:
    asl a
    tay
    lda a:ProbeTable122F,y
    sta z:zTemp0C
    lda a:ProbeTable122F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup122F_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup122F_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TileRead122F
TerrainLookup122F_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup122F_at1
    rts
TerrainLookup122F_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup122F_at2

; @routine 122F:62D TerrainLookupAlt122F
; @run 6B:962D
; WHAT IT DOES: TerrainLookup122F, then a second look at each column that found floor (as TerrainLookupAltB, 024F:60B).
; HOW: TerrainLookupAltB's code with this program's table.
; WAITS: no.
; Entry and exit: as TerrainLookup122F.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TerrainLookupAlt122F:
    pha
    jsr TerrainLookup122F
    pla
    asl a
    tay
    lda a:ProbeTable122F,y
    sta z:zTemp0C
    lda a:ProbeTable122F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt122F_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt122F_at0
    beq TerrainLookupAlt122F_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
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
    jsr TileRead122F
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt122F_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt122F_at1:
    ply
TerrainLookupAlt122F_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt122F_at2
    rts
TerrainLookupAlt122F_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt122F_at3

; @routine 1A2F:5CD TerrainLookup1A2F
; @run 6B:95CD
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB), from
; this program's probe table at $BCDF.
; HOW: TerrainLookupB's code with this program's table, reading with the read at $96A1.
; WAITS: no.
; Entry: A probe, X object. Exit: as TerrainLookupB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ProbeTable1A2F = $BCDF ; the probe table: a pointer to each probe's four columns of offsets
TileRead1A2F = $96A1 ; TerrainTileRead026F's bytes in this program
TerrainLookup1A2F:
    asl a
    tay
    lda a:ProbeTable1A2F,y
    sta z:zTemp0C
    lda a:ProbeTable1A2F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup1A2F_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup1A2F_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TileRead1A2F
TerrainLookup1A2F_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup1A2F_at1
    rts
TerrainLookup1A2F_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup1A2F_at2

; @routine 1A2F:62D TerrainLookupAlt1A2F
; @run 6B:962D
; WHAT IT DOES: TerrainLookup1A2F, then a second look at each column that found floor (as TerrainLookupAltB, 024F:60B).
; HOW: TerrainLookupAltB's code with this program's table.
; WAITS: no.
; Entry and exit: as TerrainLookup1A2F.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TerrainLookupAlt1A2F:
    pha
    jsr TerrainLookup1A2F
    pla
    asl a
    tay
    lda a:ProbeTable1A2F,y
    sta z:zTemp0C
    lda a:ProbeTable1A2F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt1A2F_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt1A2F_at0
    beq TerrainLookupAlt1A2F_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
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
    jsr TileRead1A2F
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt1A2F_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt1A2F_at1:
    ply
TerrainLookupAlt1A2F_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt1A2F_at2
    rts
TerrainLookupAlt1A2F_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt1A2F_at3

; @routine 1B8F:5CD TerrainLookup1B8F
; @run 6B:95CD
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB), from
; this program's probe table at $D85F.
; HOW: TerrainLookupB's code with this program's table, reading with the read at $96A1.
; WAITS: no.
; Entry: A probe, X object. Exit: as TerrainLookupB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ProbeTable1B8F = $D85F ; the probe table: a pointer to each probe's four columns of offsets
TileRead1B8F = $96A1 ; TerrainTileRead026F's bytes in this program
TerrainLookup1B8F:
    asl a
    tay
    lda a:ProbeTable1B8F,y
    sta z:zTemp0C
    lda a:ProbeTable1B8F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup1B8F_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup1B8F_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TileRead1B8F
TerrainLookup1B8F_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup1B8F_at1
    rts
TerrainLookup1B8F_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup1B8F_at2

; @routine 1B8F:62D TerrainLookupAlt1B8F
; @run 6B:962D
; WHAT IT DOES: TerrainLookup1B8F, then a second look at each column that found floor (as TerrainLookupAltB, 024F:60B).
; HOW: TerrainLookupAltB's code with this program's table.
; WAITS: no.
; Entry and exit: as TerrainLookup1B8F.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TerrainLookupAlt1B8F:
    pha
    jsr TerrainLookup1B8F
    pla
    asl a
    tay
    lda a:ProbeTable1B8F,y
    sta z:zTemp0C
    lda a:ProbeTable1B8F+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt1B8F_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt1B8F_at0
    beq TerrainLookupAlt1B8F_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
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
    jsr TileRead1B8F
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt1B8F_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt1B8F_at1:
    ply
TerrainLookupAlt1B8F_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt1B8F_at2
    rts
TerrainLookupAlt1B8F_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt1B8F_at3

; @routine 1CEF:5CD TerrainLookup1CEF
; @run 6B:95CD
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB), from
; this program's probe table at $DA45.
; HOW: TerrainLookupB's code with this program's table, reading with the read at $96A1.
; WAITS: no.
; Entry: A probe, X object. Exit: as TerrainLookupB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ProbeTable1CEF = $DA45 ; the probe table: a pointer to each probe's four columns of offsets
TileRead1CEF = $96A1 ; TerrainTileRead026F's bytes in this program
TerrainLookup1CEF:
    asl a
    tay
    lda a:ProbeTable1CEF,y
    sta z:zTemp0C
    lda a:ProbeTable1CEF+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup1CEF_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup1CEF_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TileRead1CEF
TerrainLookup1CEF_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup1CEF_at1
    rts
TerrainLookup1CEF_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup1CEF_at2

; @routine 1CEF:62D TerrainLookupAlt1CEF
; @run 6B:962D
; WHAT IT DOES: TerrainLookup1CEF, then a second look at each column that found floor (as TerrainLookupAltB, 024F:60B).
; HOW: TerrainLookupAltB's code with this program's table.
; WAITS: no.
; Entry and exit: as TerrainLookup1CEF.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TerrainLookupAlt1CEF:
    pha
    jsr TerrainLookup1CEF
    pla
    asl a
    tay
    lda a:ProbeTable1CEF,y
    sta z:zTemp0C
    lda a:ProbeTable1CEF+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt1CEF_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt1CEF_at0
    beq TerrainLookupAlt1CEF_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
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
    jsr TileRead1CEF
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt1CEF_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt1CEF_at1:
    ply
TerrainLookupAlt1CEF_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt1CEF_at2
    rts
TerrainLookupAlt1CEF_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt1CEF_at3

; @routine 0320:123 TerrainTileRead0320
; @run 6B:9923
; WHAT IT DOES: The terrain tile read of the stage programs (as TerrainTileRead, 050F:67F), with this program's
; handler table at $99BE. Programs 0440, 10E0 hold the same bytes.
; HOW: TerrainTileRead's code at $9923.
; WAITS: no.
; Entry: zTemp00-01 X, zTemp02-03 Y. Exit: as TerrainTileRead.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileHandlers0320 = $99BE ; the handlers for attributes $80-$9B, a word each
TerrainTileRead0320:
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
    lda z:zTemp05
    sta z:zTemp06
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
    bmi TerrainTileRead0320_at0
    lda z:zTemp08
    clc
    adc #$00
    sta z:zTemp08
    lda a:SaveBuffer,y
    adc #$7D
    sta z:zTemp09
    lda (zTemp08)
    bmi TerrainTileRead0320_at1
    rts
TerrainTileRead0320_at0:
    lda #$00
    rts
TerrainTileRead0320_at1:
    cmp #$9C
    bcs TerrainTileRead0320_at0+2
    pha
    sbc #$7F
    asl a
    tay
    lda a:TileHandlers0320,y
    sta z:zTemp04
    lda a:TileHandlers0320+1,y
    sta z:zTemp05
    pla
    jmp (zTemp04+ZeroPage)

; @routine 0320:04F TerrainLookup0320
; @run 6B:984F
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB), from
; this program's probe table at $BDF5.
; HOW: TerrainLookupB's code with this program's table, reading with the read at $9923.
; WAITS: no.
; Entry: A probe, X object. Exit: as TerrainLookupB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ProbeTable0320 = $BDF5 ; the probe table: a pointer to each probe's four columns of offsets
TerrainLookup0320:
    asl a
    tay
    lda a:ProbeTable0320,y
    sta z:zTemp0C
    lda a:ProbeTable0320+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup0320_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup0320_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TerrainTileRead0320
TerrainLookup0320_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup0320_at1
    rts
TerrainLookup0320_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup0320_at2

; @routine 0320:0AF TerrainLookupAlt0320
; @run 6B:98AF
; WHAT IT DOES: TerrainLookup0320, then a second look at each column that found floor (as TerrainLookupAltB, 024F:60B).
; HOW: TerrainLookupAltB's code with this program's table.
; WAITS: no.
; Entry and exit: as TerrainLookup0320.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TerrainLookupAlt0320:
    pha
    jsr TerrainLookup0320
    pla
    asl a
    tay
    lda a:ProbeTable0320,y
    sta z:zTemp0C
    lda a:ProbeTable0320+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt0320_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt0320_at0
    beq TerrainLookupAlt0320_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
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
    jsr TerrainTileRead0320
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt0320_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt0320_at1:
    ply
TerrainLookupAlt0320_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt0320_at2
    rts
TerrainLookupAlt0320_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt0320_at3

; @routine 0440:04F TerrainLookup0440
; @run 6B:984F
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB), from
; this program's probe table at $C1C6.
; HOW: TerrainLookupB's code with this program's table, reading with the read at $9923.
; WAITS: no.
; Entry: A probe, X object. Exit: as TerrainLookupB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ProbeTable0440 = $C1C6 ; the probe table: a pointer to each probe's four columns of offsets
TileRead0440 = $9923 ; TerrainTileRead0320's bytes in this program
TerrainLookup0440:
    asl a
    tay
    lda a:ProbeTable0440,y
    sta z:zTemp0C
    lda a:ProbeTable0440+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup0440_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup0440_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TileRead0440
TerrainLookup0440_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup0440_at1
    rts
TerrainLookup0440_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup0440_at2

; @routine 0440:0AF TerrainLookupAlt0440
; @run 6B:98AF
; WHAT IT DOES: TerrainLookup0440, then a second look at each column that found floor (as TerrainLookupAltB, 024F:60B).
; HOW: TerrainLookupAltB's code with this program's table.
; WAITS: no.
; Entry and exit: as TerrainLookup0440.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TerrainLookupAlt0440:
    pha
    jsr TerrainLookup0440
    pla
    asl a
    tay
    lda a:ProbeTable0440,y
    sta z:zTemp0C
    lda a:ProbeTable0440+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt0440_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt0440_at0
    beq TerrainLookupAlt0440_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
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
    jsr TileRead0440
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt0440_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt0440_at1:
    ply
TerrainLookupAlt0440_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt0440_at2
    rts
TerrainLookupAlt0440_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt0440_at3

; @routine 10E0:04F TerrainLookup10E0
; @run 6B:984F
; WHAT IT DOES: Reads the terrain at the four columns of a probe around an object (as TerrainLookupB, 024F:5AB), from
; this program's probe table at $C55F.
; HOW: TerrainLookupB's code with this program's table, reading with the read at $9923.
; WAITS: no.
; Entry: A probe, X object. Exit: as TerrainLookupB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
ProbeTable10E0 = $C55F ; the probe table: a pointer to each probe's four columns of offsets
TileRead10E0 = $9923 ; TerrainTileRead0320's bytes in this program
TerrainLookup10E0:
    asl a
    tay
    lda a:ProbeTable10E0,y
    sta z:zTemp0C
    lda a:ProbeTable10E0+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookup10E0_at1:
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    php
    lda (zTemp0C),y
    cmp #$80
    beq TerrainLookup10E0_at0
    plp
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
    sta z:zTemp03
    iny
    phy
    jsr TileRead10E0
TerrainLookup10E0_at2:
    ldy z:zTemp0A
    sta a:TileHit,y
    lda z:zTemp00
    sta a:TileDepthX,y
    lda z:zTemp02
    sta a:TileDepthY,y
    inc z:zTemp0A
    ply
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookup10E0_at1
    rts
TerrainLookup10E0_at0:
    plp
    iny
    iny
    iny
    phy
    stz z:zTemp00
    stz z:zTemp02
    cla
    bra TerrainLookup10E0_at2

; @routine 10E0:0AF TerrainLookupAlt10E0
; @run 6B:98AF
; WHAT IT DOES: TerrainLookup10E0, then a second look at each column that found floor (as TerrainLookupAltB, 024F:60B).
; HOW: TerrainLookupAltB's code with this program's table.
; WAITS: no.
; Entry and exit: as TerrainLookup10E0.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TerrainLookupAlt10E0:
    pha
    jsr TerrainLookup10E0
    pla
    asl a
    tay
    lda a:ProbeTable10E0,y
    sta z:zTemp0C
    lda a:ProbeTable10E0+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
TerrainLookupAlt10E0_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit,y
    ply
    and #$A1
    bmi TerrainLookupAlt10E0_at0
    beq TerrainLookupAlt10E0_at0
    lda (zTemp0C),y
    clc
    adc a:OBJ_X_LO,x
    sta z:zTemp00
    iny
    lda (zTemp0C),y
    adc a:OBJ_X_HI,x
    sta z:zTemp01
    iny
    lda (zTemp0C),y
    clc
    adc a:OBJ_Y_LO,x
    sta z:zTemp02
    iny
    lda (zTemp0C),y
    adc a:OBJ_Y_HI,x
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
    jsr TileRead10E0
    ldy z:zTemp0A
    and #$01
    beq TerrainLookupAlt10E0_at1
    ldy z:zTemp0A
    lda a:TileHit,y
    and #$5E
    sta a:TileHit,y
TerrainLookupAlt10E0_at1:
    ply
TerrainLookupAlt10E0_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne TerrainLookupAlt10E0_at2
    rts
TerrainLookupAlt10E0_at0:
    iny
    iny
    iny
    iny
    bra TerrainLookupAlt10E0_at3

; @routine 026F:774 TileStepC026F
; @run 6B:9774
; WHAT IT DOES: A slope tile: moves the point's row by the table at $980F on its column (as TileStepC, 050F:752).
; HOW: TileStepC's code with this program's table.
; WAITS: no.
; Entry and exit: as TileStepC.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileStepsC026F = $980F ; the tile's table in this program
TileStepC026F:
    ldy z:zTemp00
    lda a:TileStepsC026F,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 026F:781 TileRowA026F
; @run 6B:9781
; WHAT IT DOES: A slope tile: the point's new row from the eight by eight table at $9817 on its row and column (as TileRowA, 050F:75F).
; HOW: TileRowA's code with this program's table.
; WAITS: no.
; Entry and exit: as TileRowA.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowsA026F = $9817 ; the tile's table in this program
TileRowA026F:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsA026F,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 026F:792 TileRowB026F
; @run 6B:9792
; WHAT IT DOES: As TileRowB (050F:770), with the table at $9857.
; HOW: TileRowB's code with this program's table.
; WAITS: no.
; Entry and exit: as TileRowB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowsB026F = $9857 ; the tile's table in this program
TileRowB026F:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsB026F,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 026F:7A3 TileHalfStepA026F
; @run 6B:97A3
; WHAT IT DOES: The tiles $87-$8A: a step from the pairs at $97FF that differs between the tile's halves (as TileHalfStepA, 050F:781).
; HOW: TileHalfStepA's code with this program's table.
; WAITS: no.
; Entry and exit: as TileHalfStepA.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileHalfStepsA026F = $97FF ; the tile's table in this program
TileHalfStepA026F:
    sec
    sbc #$83
    asl a
    tay
    bbr2 zTemp00,TileHalfStepA026F_at0
    iny
TileHalfStepA026F_at0:
    lda a:TileHalfStepsA026F,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 026F:7B7 TileHalfStepB026F
; @run 6B:97B7
; WHAT IT DOES: The tiles $8C-$8F: a step from the pairs at $9807 that differs between the tile's halves (as TileHalfStepB, 050F:795).
; HOW: TileHalfStepB's code with this program's table.
; WAITS: no.
; Entry and exit: as TileHalfStepB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileHalfStepsB026F = $9807 ; the tile's table in this program
TileHalfStepB026F:
    sec
    sbc #$8C
    asl a
    tay
    bbs2 zTemp00,TileHalfStepB026F_at0
    iny
TileHalfStepB026F_at0:
    lda a:TileHalfStepsB026F,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 026F:7D0 TileStepD026F
; @run 6B:97D0
; WHAT IT DOES: As TileStepD (050F:7AE), with the table at $9897.
; HOW: TileStepD's code with this program's table.
; WAITS: no.
; Entry and exit: as TileStepD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileStepsD026F = $9897 ; the tile's table in this program
TileStepD026F:
    ldy z:zTemp00
    lda a:TileStepsD026F,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 026F:7DD TileRowC026F
; @run 6B:97DD
; WHAT IT DOES: As TileRowC (050F:7BB), with the table at $989F.
; HOW: TileRowC's code with this program's table.
; WAITS: no.
; Entry and exit: as TileRowC.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowsC026F = $989F ; the tile's table in this program
TileRowC026F:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsC026F,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 026F:7EE TileRowD026F
; @run 6B:97EE
; WHAT IT DOES: As TileRowD (050F:7CC), with the table at $98DF.
; HOW: TileRowD's code with this program's table.
; WAITS: no.
; Entry and exit: as TileRowD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowsD026F = $98DF ; the tile's table in this program
TileRowD026F:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsD026F,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 0320:1F6 TileStepC0320
; @run 6B:99F6
; WHAT IT DOES: A slope tile: moves the point's row by the table at $9A91 on its column (as TileStepC, 050F:752).
; HOW: TileStepC's code with this program's table.
; WAITS: no.
; Entry and exit: as TileStepC.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileStepsC0320 = $9A91 ; the tile's table in this program
TileStepC0320:
    ldy z:zTemp00
    lda a:TileStepsC0320,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 0320:203 TileRowA0320
; @run 6B:9A03
; WHAT IT DOES: A slope tile: the point's new row from the eight by eight table at $9A99 on its row and column (as TileRowA, 050F:75F).
; HOW: TileRowA's code with this program's table.
; WAITS: no.
; Entry and exit: as TileRowA.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowsA0320 = $9A99 ; the tile's table in this program
TileRowA0320:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsA0320,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 0320:214 TileRowB0320
; @run 6B:9A14
; WHAT IT DOES: As TileRowB (050F:770), with the table at $9AD9.
; HOW: TileRowB's code with this program's table.
; WAITS: no.
; Entry and exit: as TileRowB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowsB0320 = $9AD9 ; the tile's table in this program
TileRowB0320:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsB0320,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 0320:225 TileHalfStepA0320
; @run 6B:9A25
; WHAT IT DOES: The tiles $87-$8A: a step from the pairs at $9A81 that differs between the tile's halves (as TileHalfStepA, 050F:781).
; HOW: TileHalfStepA's code with this program's table.
; WAITS: no.
; Entry and exit: as TileHalfStepA.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileHalfStepsA0320 = $9A81 ; the tile's table in this program
TileHalfStepA0320:
    sec
    sbc #$83
    asl a
    tay
    bbr2 zTemp00,TileHalfStepA0320_at0
    iny
TileHalfStepA0320_at0:
    lda a:TileHalfStepsA0320,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 0320:239 TileHalfStepB0320
; @run 6B:9A39
; WHAT IT DOES: The tiles $8C-$8F: a step from the pairs at $9A89 that differs between the tile's halves (as TileHalfStepB, 050F:795).
; HOW: TileHalfStepB's code with this program's table.
; WAITS: no.
; Entry and exit: as TileHalfStepB.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileHalfStepsB0320 = $9A89 ; the tile's table in this program
TileHalfStepB0320:
    sec
    sbc #$8C
    asl a
    tay
    bbs2 zTemp00,TileHalfStepB0320_at0
    iny
TileHalfStepB0320_at0:
    lda a:TileHalfStepsB0320,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 0320:252 TileStepD0320
; @run 6B:9A52
; WHAT IT DOES: As TileStepD (050F:7AE), with the table at $9B19.
; HOW: TileStepD's code with this program's table.
; WAITS: no.
; Entry and exit: as TileStepD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileStepsD0320 = $9B19 ; the tile's table in this program
TileStepD0320:
    ldy z:zTemp00
    lda a:TileStepsD0320,y
    clc
    adc z:zTemp02
    sta z:zTemp02
    lda #$81
    rts

; @routine 0320:25F TileRowC0320
; @run 6B:9A5F
; WHAT IT DOES: As TileRowC (050F:7BB), with the table at $9B21.
; HOW: TileRowC's code with this program's table.
; WAITS: no.
; Entry and exit: as TileRowC.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowsC0320 = $9B21 ; the tile's table in this program
TileRowC0320:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsC0320,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 0320:270 TileRowD0320
; @run 6B:9A70
; WHAT IT DOES: As TileRowD (050F:7CC), with the table at $9B61.
; HOW: TileRowD's code with this program's table.
; WAITS: no.
; Entry and exit: as TileRowD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowsD0320 = $9B61 ; the tile's table in this program
TileRowD0320:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
    tay
    lda a:TileRowsD0320,y
    sta z:zTemp02
    lda #$81
    rts

; @routine 0270:11F TileStepTwice026F
; @run 6B:991F
; WHAT IT DOES: As TileStepTwice (0510:0FD), with this program's tables.
; HOW: TileStepTwice's code, entering TileStepOffset026F past its LDY.
; WAITS: no.
; Entry: as TileStepTwice.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowOffsets026F = $998A ; this program's offsets by column
TileRowsE026F = $9952 ; this program's new rows by row and column
TileStepTwice026F:
    ldy z:zTemp00
    lda a:TileRowOffsets026F,y
    inc a
    tay
    bra TileStepOffset026F_at0

; @routine 0270:128 TileRowOffset026F
; @run 6B:9928
; WHAT IT DOES: As TileRowOffset (0510:106), with this program's tables.
; HOW: TileRowOffset's code, entering TileRowE026F at its TAY.
; WAITS: no.
; Entry: as TileRowOffset.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowOffset026F:
    lda z:zTemp02
    asl a
    asl a
    asl a
    ldy z:zTemp00
    clc
    adc a:TileRowOffsets026F,y
    inc a
    bra TileRowE026F_at0

; @routine 0270:136 TileStepOffset026F
; @run 6B:9936
; WHAT IT DOES: As TileStepOffset (0510:114), with this program's tables.
; HOW: TileStepOffset's code, storing through TileRowE026F's end.
; WAITS: no.
; Entry: as TileStepOffset.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileStepOffset026F:
    ldy z:zTemp00
TileStepOffset026F_at0:
    lda z:zTemp02
    clc
    adc a:TileRowOffsets026F,y
    inc a
    bra TileRowE026F_at1

; @routine 0270:141 TileRowE026F
; @run 6B:9941
; WHAT IT DOES: As TileRowE (0510:11F), with this program's tables.
; HOW: TileRowE's code with this program's table.
; WAITS: no.
; Entry: as TileRowE.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowE026F:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
TileRowE026F_at0:
    tay
    lda a:TileRowsE026F,y
TileRowE026F_at1:
    sta z:zTemp02
    lda #$81
    rts

; @routine 0320:3A1 TileStepTwice0320
; @run 6B:9BA1
; WHAT IT DOES: As TileStepTwice (0510:0FD), with this program's tables.
; HOW: TileStepTwice's code, entering TileStepOffset0320 past its LDY.
; WAITS: no.
; Entry: as TileStepTwice.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowOffsets0320 = $9C0C ; this program's offsets by column
TileRowsE0320 = $9BD4 ; this program's new rows by row and column
TileStepTwice0320:
    ldy z:zTemp00
    lda a:TileRowOffsets0320,y
    inc a
    tay
    bra TileStepOffset0320_at0

; @routine 0320:3AA TileRowOffset0320
; @run 6B:9BAA
; WHAT IT DOES: As TileRowOffset (0510:106), with this program's tables.
; HOW: TileRowOffset's code, entering TileRowE0320 at its TAY.
; WAITS: no.
; Entry: as TileRowOffset.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowOffset0320:
    lda z:zTemp02
    asl a
    asl a
    asl a
    ldy z:zTemp00
    clc
    adc a:TileRowOffsets0320,y
    inc a
    bra TileRowE0320_at0

; @routine 0320:3B8 TileStepOffset0320
; @run 6B:9BB8
; WHAT IT DOES: As TileStepOffset (0510:114), with this program's tables.
; HOW: TileStepOffset's code, storing through TileRowE0320's end.
; WAITS: no.
; Entry: as TileStepOffset.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileStepOffset0320:
    ldy z:zTemp00
TileStepOffset0320_at0:
    lda z:zTemp02
    clc
    adc a:TileRowOffsets0320,y
    inc a
    bra TileRowE0320_at1

; @routine 0320:3C3 TileRowE0320
; @run 6B:9BC3
; WHAT IT DOES: As TileRowE (0510:11F), with this program's tables.
; HOW: TileRowE's code with this program's table.
; WAITS: no.
; Entry: as TileRowE.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
TileRowE0320:
    lda z:zTemp02
    asl a
    asl a
    asl a
    clc
    adc z:zTemp00
TileRowE0320_at0:
    tay
    lda a:TileRowsE0320,y
TileRowE0320_at1:
    sta z:zTemp02
    lda #$81
    rts

; @routine 026F:5B5 StageHitReportTop026F
; @run 6B:95B5
; WHAT IT DOES: As StageHitReportTop (024F:593), but side 2 gives the attack power 6.
; HOW: StageHitReportTop's code with the power 6; both branches go to the report at $958A, which holds HitReportHandler's bytes.
; WAITS: no.
; Entry: as StageHitReportTop.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageHitReportTop026F:
    lda a:OBJ_PACE,x
    and #$80
    rol a
    rol a
    and #$01
    inc a
    sta a:OBJ_ATTACK_SIDE,y
    cmp #$02
    bne *-58 ; the report at $958A
    lda #$06
    sta a:OBJ_ATTACK_POWER,y
    bra *-65 ; the report at $958A

; @routine 14DF:035 StageUnpackList14DD
; @run 6B:9035
; WHAT IT DOES: StageUnpackList3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageUnpackList3CD's code with this build's table addresses.
; WAITS: where StageUnpackList3CD waits.
; Entry: as StageUnpackList3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageListPointers14DD = $9B6C ; this build's table
StageRowMapPointers14DD = $9B6E ; this build's table
StageRouteSounds14DD = $9B68 ; this build's table
StageRouteScripts14DD = $9B69 ; this build's table
StageScreenPointers14DD = $9B77 ; this build's table
StageLevelRecords14DD = $9B97 ; this build's table
StageNextByRoute14DD = $9B6B ; this build's table
StageProbeLists14DD = $C3B5 ; this build's table
StageUnpackList14DD:
    tai KernelZeros,SaveBuffer,$02FF
    lda z:zField82
    asl a
    tay
    lda a:StageListPointers14DD,y
    sta z:zTemp00
    lda a:StageListPointers14DD+1,y
    sta z:zTemp01
    cly
StageUnpackList14DD_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne StageUnpackList14DD_at0
    lda #$08
    bra StageUnpackList14DD_at1
StageUnpackList14DD_at7:
    lda (zTemp00)
    bne StageUnpackList14DD_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra StageUnpackList14DD_at1
StageUnpackList14DD_at2:
    cmp #$FF
    bne StageUnpackList14DD_at3
    rts
StageUnpackList14DD_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
StageUnpackList14DD_at4:
    sta (zTemp02),y
    iny
    dex
    bne StageUnpackList14DD_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc StageUnpackList14DD_at5
    inc z:zTemp03
StageUnpackList14DD_at5:
    lda #$02
StageUnpackList14DD_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc StageUnpackList14DD_at6
    inc z:zTemp01
StageUnpackList14DD_at6:
    bra StageUnpackList14DD_at7


; @routine 14DE:01E StageLoadRowMap14DD
; @run 6B:881E
; WHAT IT DOES: StageLoadRowMap3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageLoadRowMap3CD's code with this build's table addresses.
; WAITS: where StageLoadRowMap3CD waits.
; Entry: as StageLoadRowMap3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageLoadRowMap14DD:
    tai KernelZeros,$3A00,$00FF
    lda z:zField82
    asl a
    tay
    lda a:StageRowMapPointers14DD,y
    sta z:zTemp00
    lda a:StageRowMapPointers14DD+1,y
    sta z:zTemp01
    cly
StageLoadRowMap14DD_at1:
    lda (zTemp00),y
    tax
    iny
StageLoadRowMap14DD_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne StageLoadRowMap14DD_at0
    rts
StageLoadRowMap14DD_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:$3A00,x
    inx
    bbs0 zTemp02,StageLoadRowMap14DD_at1
    bra StageLoadRowMap14DD_at2


; @routine 14DE:04D StageRouteSetUpA14DD
; @run 6B:884D
; WHAT IT DOES: StageRouteSetUpA3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageRouteSetUpA3CD's code with this build's table addresses.
; WAITS: where StageRouteSetUpA3CD waits.
; Entry: as StageRouteSetUpA3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageRouteSetUpA14DD:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageRouteSounds14DD,x
    jsr VecStartListChannel0Kind2
    jmp StageUnpackList14DD


; @routine 14DE:05B StageRouteSetUpB14DD
; @run 6B:885B
; WHAT IT DOES: StageRouteSetUpB3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageRouteSetUpB3CD's code with this build's table addresses.
; WAITS: where StageRouteSetUpB3CD waits.
; Entry: as StageRouteSetUpB3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageRouteSetUpB14DD:
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageRouteScripts14DD,x
    ldx #$07
    jmp VecStartScriptSlotKind1


; @routine 14DE:0D2 StageLoadScreen14DD
; @run 6B:88D2
; WHAT IT DOES: StageLoadScreen3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageLoadScreen3CD's code with this build's table addresses.
; WAITS: where StageLoadScreen3CD waits.
; Entry: as StageLoadScreen3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageLoadScreen14DD:
    stz z:zRasterStage
    lda z:zField83
    asl a
    tay
    lda a:StageScreenPointers14DD,y
    sta z:zTemp00
    lda a:StageScreenPointers14DD+1,y
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
    sta a:Scroll3E66
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
    bbr1 zField8E,StageLoadScreen14DD_at5
    rmb1 z:zField8E
    iny
    iny
    bra StageLoadScreen14DD_at0
StageLoadScreen14DD_at5:
    lda (zTemp00),y
    bne StageLoadScreen14DD_at1
    lda z:zBiosAl
    sta a:ScrollXHiScreen
    and #$01
    sta a:ScrollXHi
    lda z:zBiosAh
    sta a:ScrollXLo
    iny
    bra StageLoadScreen14DD_at2
StageLoadScreen14DD_at1:
    iny
    sta a:OBJ_X_HI+8
StageLoadScreen14DD_at2:
    lda (zTemp00),y
    iny
    sta a:$2C50
    stz a:$2C08
    stz a:$2B30
    lda z:zCharacter
    beq StageLoadScreen14DD_at0
    lda a:$2818
    cmp #$05
    bcc StageLoadScreen14DD_at0
    sec
    sbc #$05
    cmp #$04
    bcs StageLoadScreen14DD_at0
    tax
    lda a:OBJ_X_HI+8
    clc
    adc a:$899A,x
    sta a:OBJ_X_HI+8
StageLoadScreen14DD_at0:
    lda (zTemp00),y
    iny
    sta z:zRasterEffect
    stz z:zRasterArg
    lda (zTemp00),y
    bmi StageLoadScreen14DD_at3
    iny
    sta a:$2AA0
    bra StageLoadScreen14DD_at4
StageLoadScreen14DD_at3:
    iny
StageLoadScreen14DD_at4:
    rts


; @routine 14DE:078 StageResetScroll14DD
; @run 6B:8878
; WHAT IT DOES: StageResetScroll3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageResetScroll3CD's code with this build's table addresses.
; WAITS: where StageResetScroll3CD waits.
; Entry: as StageResetScroll3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageResetScroll14DD:
    lda a:ScrollXHiScreen
    sta z:zBiosAl
    lda a:ScrollXLo
    sta z:zBiosAh
    jsr VecClearScreenState
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    stz a:$2ED8
    lda z:zField8E
    pha
    bsr StageLoadScreen14DD
    pla
    sta z:zField8E
    jsr $8044
    jsr SetBlinkFlash
    ldy z:zRasterEffect
    lda a:$886A,y
    beq StageResetScroll14DD_at0
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    bsr StageLoadScreen14DD
    pla
    sta z:zField8E
    jsr $A79D
StageResetScroll14DD_at0:
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088


; @routine 14DE:1B2 StageLevelStart14DD
; @run 6B:89B2
; WHAT IT DOES: StageLevelStart3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageLevelStart3CD's code with this build's table addresses.
; WAITS: where StageLevelStart3CD waits.
; Entry: as StageLevelStart3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageLevelStart14DD:
    jsr VecClearEnemySlots
    jsr $8044
    lda z:zField83
    asl a
    asl a
    tay
    lda a:StageLevelRecords14DD,y
    sta a:LevelStreamLo
    lda a:StageLevelRecords14DD+1,y
    sta a:LevelStreamHi
    lda a:StageLevelRecords14DD+2,y
    sta a:LevelAxis
    lda a:StageLevelRecords14DD+3,y
    beq StageLevelStart14DD_at0
    phy
    cly
StageLevelStart14DD_at1:
    lda #$00
    sta a:EventFlags,y
    iny
    cpy #$20
    bcc StageLevelStart14DD_at1
    ply
StageLevelStart14DD_at0:
    jsr VecStreamFullWalk
    jsr StageOverlayFrame3CD
    rts


; @routine 14DD:619 StageNextRouteStage14DD
; @run 6B:8619
; WHAT IT DOES: StageNextRouteStage3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageNextRouteStage3CD's code with this build's table addresses.
; WAITS: where StageNextRouteStage3CD waits.
; Entry: as StageNextRouteStage3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageNextRouteStage14DD:
    lda #$02
    sta z:zDisplayDelay
    stz a:$3310
    stz a:Field2742+1
    stz a:OBJ_WORK_C
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageNextByRoute14DD,y
    sta z:zField83
    cla
    sta z:$84
    stz a:$2A58
    rts


; @routine 14DD:5FD StageBossWalkOut14DD
; @run 6B:85FD
; WHAT IT DOES: StageBossWalkOut3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageBossWalkOut3CD's code with this build's table addresses.
; WAITS: where StageBossWalkOut3CD waits.
; Entry: as StageBossWalkOut3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageBossWalkOut14DD:
    lda #$B8
    sta a:$3310
    jsr StageFrameA0003CD
    jsr VecPlayerAndCameraFrame
    jsr RichterVecC3A7
    jsr StageFrameA08D3CD
    jsr StageOverlayFrame3CD
    lda a:OBJ_X_HI+8
    cmp #$F0
    beq StageNextRouteStage14DD
    rts


; @routine 14DD:511 StageClearedWalkOn14DD
; @run 6B:8511
; WHAT IT DOES: StageClearedWalkOn3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageClearedWalkOn3CD's code with this build's table addresses.
; WAITS: where StageClearedWalkOn3CD waits.
; Entry: as StageClearedWalkOn3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearedWalkOn14DD:
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
    beq StageClearedWalkOn14DD_at0
    rts
StageClearedWalkOn14DD_at0:
    lda z:zField8C
    cmp #$07
    bcs StageClearedWalkOn14DD_at1
    inc z:zField82
    lda z:zField82
    tay
    lda a:StageNextByRoute14DD,y
    sta z:zField83
    sta z:zField86
    bra StageClearedWalkOn14DD_at2
StageClearedWalkOn14DD_at1:
    inc z:zField83
    smb4 z:zField8E
    lda #$FF
    jsr VecSoundCommand
StageClearedWalkOn14DD_at2:
    smb1 z:zField8E
    lda z:zField8C
    cmp #$07
    bcs StageClearedWalkOn14DD_at3
    jsr StageRouteSetUpA14DD
    jsr StageRouteSetUpB14DD
StageClearedWalkOn14DD_at3:
    jsr StageLoadRowMap14DD
    tai KernelZeros,ScrollXFrac,$0030
    tai KernelZeros,Scroll3E66,$0088
    lda z:zField8E
    pha
    jsr StageLoadScreen14DD
    pla
    sta z:zField8E
    lda a:CameraMode
    cmp #$0D
    bne StageClearedWalkOn14DD_at4
    jsr $8044
    jsr SetBlinkFlash
    jsr StageLoadScreen14DD
StageClearedWalkOn14DD_at4:
    rmb1 z:zField8E
    lda z:zField8C
    cmp #$09
    bcs StageClearedWalkOn14DD_at6
    cmp #$07
    bcs StageClearedWalkOn14DD_at5
StageClearedWalkOn14DD_at6:
    jsr StageLevelStart14DD
StageClearedWalkOn14DD_at5:
    stz z:zField8C
    stz z:zPaused
    lda #$01
    sta z:zGameSubMode
    rts


; @routine 14DF:68D StageProbePoints14DD
; @run 6B:968D
; WHAT IT DOES: StageProbePoints3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageProbePoints3CD's code with this build's table addresses.
; WAITS: where StageProbePoints3CD waits.
; Entry: as StageProbePoints3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageProbePoints14DD:
    asl a
    tay
    lda a:StageProbeLists14DD,y
    sta z:zTemp0C
    lda a:StageProbeLists14DD+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
StageProbePoints14DD_at0:
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
    bne StageProbePoints14DD_at0
    rts


; @routine 14DF:6DB StageProbeAgain14DD
; @run 6B:96DB
; WHAT IT DOES: StageProbeAgain3CD's code in the copy of its program at 14DD (and 15BD, 177D), which reads its own tables.
; HOW: StageProbeAgain3CD's code with this build's table addresses.
; WAITS: where StageProbeAgain3CD waits.
; Entry: as StageProbeAgain3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageProbeAgain14DD:
    pha
    jsr StageProbePoints14DD
    pla
    asl a
    tay
    lda a:StageProbeLists14DD,y
    sta z:zTemp0C
    lda a:StageProbeLists14DD+1,y
    sta z:zTemp0D
    cly
    sty z:zTemp0A
StageProbeAgain14DD_at2:
    phy
    ldy z:zTemp0A
    lda a:TileHit+3,y
    ply
    and #$81
    bmi StageProbeAgain14DD_at0
    beq StageProbeAgain14DD_at0
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
    jsr ProbeTile3CD
    ldy z:zTemp0A
    and #$01
    beq StageProbeAgain14DD_at1
    ldy z:zTemp0A
    lda a:TileHit+3,y
    and #$7E
    sta a:TileHit+3,y
StageProbeAgain14DD_at1:
    ply
StageProbeAgain14DD_at3:
    inc z:zTemp0A
    lda z:zTemp0A
    cmp #$04
    bne StageProbeAgain14DD_at2
    rts
StageProbeAgain14DD_at0:
    iny
    iny
    iny
    iny
    bra StageProbeAgain14DD_at3


; @routine 026D:018 StageModeStep026D
; @run 6B:8018
; WHAT IT DOES: Goes to the sub-mode's routine by zGameSubMode, through this program's table at $801F.
; HOW: LDA, ASL, TAX, a jump through the table.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
MapOverlayBanks5And6At026D = $8089 ; the bank mapping at this address of the program at 026D
StageFadeTimer026D = $8161 ; the fade timer at this address of this program
StageScrollStepAt026D = $89EC ; the scroll step this program runs
StepStageFrameAt026D = $8B23 ; the stage frame this program runs
SetUpStageObjectsAt026D = $8A15 ; the object set up this program runs
FrameBankEntryA000 = $A000 ; the player bank's entries
FrameBankEntryA009 = $A009
FrameBankEntryA00C = $A00C
FrameBankEntryA00F = $A00F
FrameBankEntryA012 = $A012
FrameBankEntryA015 = $A015
StageModeSteps026D = $801F ; this program's table of sub-modes
StageDeathPhases026D = $82ED ; its table of the death's phases
StageClearedSteps026D = $843D ; its tables of the cleared stage's steps
StageBossSteps026D = $85B2
StageClearedStepsB026D = $864C
StageModeStep026D:
    lda z:zGameSubMode
    asl a
    tax
    jmp (StageModeSteps026D,x)

; @routine 026D:094 StagePlayerFrame026D
; @run 6B:8094
; WHAT IT DOES: Maps banks 5 and 6 and goes to the entry at $A000 there.
; HOW: A JSR and a JMP.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StagePlayerFrame026D:
    jsr MapOverlayBanks5And6At026D
    jmp FrameBankEntryA000

; @routine 026D:09A StagePlayerFrameB026D
; @run 6B:809A
; WHAT IT DOES: Maps banks 5 and 6 and goes to the entry at $A009 there.
; HOW: A JSR and a JMP.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StagePlayerFrameB026D:
    jsr MapOverlayBanks5And6At026D
    jmp FrameBankEntryA009

; @routine 026D:2CE StageSubModeDeath026D
; @run 6B:82CE
; WHAT IT DOES: The death sub-mode: sound $FF, the death's phase, and once the sub-mode is 1 again the stage's objects set up afresh.
; HOW: As the C.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageSubModeDeath026D:
    lda #$FF
    jsr VecSoundCommand
    jsr StageDeathPhase026D
    lda z:zGameSubMode
    cmp #$01
    beq StageSubModeDeath026D_at0
    rts
StageSubModeDeath026D_at0:
    jsr MapOverlayBanks5And6At026D
    jsr FrameBankEntryA015
    jmp SetUpStageObjectsAt026D

; @routine 026D:2E6 StageDeathPhase026D
; @run 6B:82E6
; WHAT IT DOES: Goes to the death's phase by zField87 through this program's table at $82ED.
; HOW: LDA, ASL, TAX, a jump through the table.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageDeathPhase026D:
    lda z:zField87
    asl a
    tax
    jmp (StageDeathPhases026D,x)

; @routine 026D:430 StageClearedWalkOut026D
; @run 6B:8430
; WHAT IT DOES: Runs the scroll step and the stage frame, then goes to the step zField85 names in the table at $843D.
; HOW: Two JSRs; LDA, ASL, TAX, a jump through the table.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearedWalkOut026D:
    jsr StageScrollStepAt026D
    jsr StepStageFrameAt026D
    lda z:zField85
    asl a
    tax
    jmp (StageClearedSteps026D,x)

; @routine 026D:5A5 StageClearedBossWalk026D
; @run 6B:85A5
; WHAT IT DOES: Runs the stage frame and the fade timer, then goes to the step zField85 names in the table at $85B2.
; HOW: Two JSRs; LDA, ASL, TAX, a jump through the table.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearedBossWalk026D:
    jsr StepStageFrameAt026D
    jsr StageFadeTimer026D
    lda z:zField85
    asl a
    tax
    jmp (StageBossSteps026D,x)

; @routine 026D:63F StageClearedWalkOutB026D
; @run 6B:863F
; WHAT IT DOES: Runs the scroll step and the stage frame, then goes to the step zField85 names in the table at $864C.
; HOW: Two JSRs; LDA, ASL, TAX, a jump through the table.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearedWalkOutB026D:
    jsr StageScrollStepAt026D
    jsr StepStageFrameAt026D
    lda z:zField85
    asl a
    tax
    jmp (StageClearedStepsB026D,x)

; @routine 026D:7A5 StageFadeWait026D
; @run 6B:87A5
; WHAT IT DOES: Runs the bank's entry at $A00F, calls $A012 if the player's frame changed, and counts zField80 while zField89 is not 0.
; HOW: As the C; the count is in StageFadeStart026D.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFadeWait026D:
    lda a:OBJ_FRAME
    pha
    jsr FrameBankEntryA00F
    pla
    cmp a:OBJ_FRAME
    beq StageFadeWait026D_at0
    jsr FrameBankEntryA012
StageFadeWait026D_at0:
    lda z:zField89
    bne *-21
    rts

; @routine 026D:7BA StagePauseCdThenStock026D
; @run 6B:87BA
; WHAT IT DOES: StagePauseCdThenStock's code in the program at 026D.
; HOW: StagePauseCdThenStock's code at this address.
; WAITS: where the routines it calls wait.
; Entry: as StagePauseCdThenStock.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StagePauseCdThenStock026D:
    jsr BiosCdPause
    tax
    bne StagePauseCdThenStock026D
    lda z:zStock
    bne *+16 ; this program's StageStockTake at $87D2
    lda #$80
    sta z:zModeDone
    rts

; @routine 026D:6F6 StagePlayerToCentre026D
; @run 6B:86F6
; WHAT IT DOES: StagePlayerToCentre's code in the program at 026D, calling its own routines.
; HOW: StagePlayerToCentre's code at this address.
; WAITS: where the routines it calls wait.
; Entry: as StagePlayerToCentre.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StagePlayerToCentre026D:
    lda a:OBJ_SUBSTATE
    bne StagePlayerToCentre026D_at0
    lda a:OBJ_STATE
    cmp #$02
    bcc StagePlayerToCentre026D_at1
StagePlayerToCentre026D_at0:
    lda #$80
    sta a:OBJ_PAD_FORCE_FRAMES
    stz a:OBJ_PAD_FORCED
    jsr StagePlayerFrameB026D
    rts
StagePlayerToCentre026D_at1:
    lda #$80
    sta a:OBJ_PAD_FORCE_FRAMES
    stz a:OBJ_PAD_FORCED
    lda #$01
    sta a:PlayerStartPose
    jsr StagePlayerFrameB026D
    lda a:OBJ_X_LO
    clc
    adc #$02
    and #$FC
    cmp #$80
    beq StagePlayerToCentre026D_at2
    lda a:OBJ_X_LO
    bmi StagePlayerToCentre026D_at3
    lda #$FC
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    lda a:OBJ_X_LO
    clc
    adc #$04
    sta a:OBJ_X_LO
    bra StagePlayerToCentre026D_at4
StagePlayerToCentre026D_at3:
    lda #$04
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    lda a:OBJ_X_LO
    clc
    adc #$FC
    sta a:OBJ_X_LO
StagePlayerToCentre026D_at4:
    jsr StageCameraStep026D
    lda a:OBJ_X_LO
    clc
    adc #$02
    and #$F8
    cmp #$80
    bne StagePlayerToCentre026D_end
StagePlayerToCentre026D_at2:
    rmb7 z:zField8E
    stz a:OBJ_PAD_FORCE_FRAMES
    stz a:PlayerStartPose
StagePlayerToCentre026D_end:
    rts


; @routine 026D:79C StageFadeStart026D
; @run 6B:879C
; WHAT IT DOES: StageFadeStart3CD's code in the program at 026D.
; HOW: StageFadeStart3CD's code at this address.
; WAITS: where the routines it calls wait.
; Entry: as StageFadeStart3CD.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFadeStart026D:
    jsr RichterVecC023
    jsr RichterVecAnimationSound
    inc z:zField80
    rts


; @routine 026D:5E8 StageBossWaitDoor026D
; @run 6B:85E8
; WHAT IT DOES: The wait at the boss's door in this program: its own two calls and bytes.
; HOW: StageBossWaitDoor3CD's code with this program's calls and bytes.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageBossWaitDoor026D:
    jsr StagePlayerFrameB026D
    jsr StepScrollAndRunObjects026E
    lda a:OBJ_STATE+34
    cmp #$04
    bne StageBossWaitDoor026D_at0
    lda a:OBJ_WORK_H+34
    bne StageBossWaitDoor026D_at0
    lda #$20
    sta a:OBJ_WORK_F
    inc z:zField85
StageBossWaitDoor026D_at0:
    rts

; @routine 026D:7F2 EnterStageState026D
; @run 6B:87F2
; WHAT IT DOES: EnterStageState's code in the program at 026D.
; HOW: EnterStageState's code at this address.
; WAITS: where the routines it calls wait.
; Entry: as EnterStageState.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
EnterStageState026D:
    lda z:zField86
    sta z:zField83
    stz z:zField8A
    stz z:zField8F
    rmb2 z:zField8E
    lda z:zStage
    bpl EnterStageState026D_at0
EnterStageState026D_at1:
    jsr BiosCdPause
    tax
    bne EnterStageState026D_at1
    smb3 z:zField8E
EnterStageState026D_at0:
    lda z:zGameRunning
    beq EnterStageState026D_at2
    inc z:zModeDone
    rts
EnterStageState026D_at2:
    stz z:zRasterEffect
    lda #$02
    sta z:zDisplayDelay
    stz z:zGameSubMode
    rts


; @routine 026D:447 StageClearedWaitObjects026D
; @run 6B:8447
; WHAT IT DOES: The cleared stage's wait for its objects in this program.
; HOW: StageClearedWaitObjects3CD's code with this program's calls and bytes.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StagePlayerBankEntryA003 = $A003 ; the player bank's entry
ClearedWalkSides026D = $8416 ; the walk's step by side
ClearedWalkEnds026D = $841A ; where the walk back ends, by side
ClearedWalkTimers026D = $841C ; the walk back's OBJ_WORK_F, by side
StageClearedWaitObjects026D:
    jsr StageShowStartFrame026D
    jsr StagePlayerFrameB026D
    jsr StageCameraStep026D
    jsr StepScrollAndRunObjects026E
    ldx #$50
StageClearedWaitObjects026D_at0:
    lda a:OBJ_KIND,x
    bmi StageClearedWaitObjects026D_at1
    bne *+78 ; the RTS at $84A8, StageClearedWaitScroll026D's
StageClearedWaitObjects026D_at1:
    dex
    bne StageClearedWaitObjects026D_at0
    stz a:HitAttackFlags
    inc z:zField85
    rts

; @routine 026D:4A9 StageClearedWalkBack026D
; @run 6B:84A9
; WHAT IT DOES: The player walks back until he reaches the side's end, then the walk's bytes are cleared, the start mode 2 is set, the player bank's $A003 runs and the step counted.
; HOW: As the C.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearedWalkBack026D:
    jsr StepScrollAndRunObjects026E
    lda #$E0
    sta a:OBJ_TALLY_WAIT
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:ClearedWalkTimers026D,y
    sta a:OBJ_WORK_F
    jsr StagePlayerFrame026D
    lda z:zField8C
    dec a
    and #$01
    tay
    lda a:OBJ_X_LO
    sec
    sbc a:ClearedWalkSides026D,y
    sta a:OBJ_X_LO
    cmp a:ClearedWalkEnds026D,y
    beq StageClearedWalkBack026D_at0
    rts
StageClearedWalkBack026D_at0:
    rmb7 z:zField8C
    stz a:OBJ_TALLY_WAIT
    stz a:OBJ_WORK_F
    lda #$02
    sta z:zField8A
    jsr StagePlayerBankEntryA003
    jsr StageShowStartFrame026D
    inc z:zField85
    rts

; @routine 026D:5B8 StageBossScrollToDoor026D
; @run 6B:85B8
; WHAT IT DOES: Scrolls to the boss's door a pixel a frame; at screen 4 the scroll stops, sound $82, the step counted.
; HOW: As the C.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageBossScrollToDoor026D:
    jsr StageShowStartFrame026D
    jsr StagePlayerFrameB026D
    lda #$01
    sta a:ScrollStepXHi
    stz a:ScrollStepXLo
    dec a:OBJ_X_LO
    jsr StageCameraStep026D
    jsr StepScrollAndRunObjects026E
    lda a:ScrollXHi
    cmp #$04
    beq StageBossScrollToDoor026D_at0
    rts
StageBossScrollToDoor026D_at0:
    jsr StageScrollSpeedsOff026D
    stz a:ScrollStepXHi
    stz a:ScrollStepXLo+2
    lda #$82
    jsr VecSoundCommand
    inc z:zField85
    rts

; @routine 026D:602 StageBossWalkOut026D
; @run 6B:8602
; WHAT IT DOES: The player walks out of the boss's room; at X $F0 the walk's bytes cleared, the route's next section and the start frame reset (boss_walk_end_026d, $861E).
; HOW: As the C.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StagePlayerBankEntryA009At026D = $A009 ; the player bank's entry
StageBossWalkOut026D:
    lda #$B8
    sta a:OBJ_TALLY_WAIT
    jsr StagePlayerFrame026D
    jsr VecPlayerAndCameraFrame
    jsr StagePlayerBankEntryA009At026D
    jsr StageCameraStep026D
    jsr StepScrollAndRunObjects026E
    lda a:OBJ_X_LO
    cmp #$F0
    beq StageBossWalkOut026D_at0
    rts
StageBossWalkOut026D_at0:
    lda #$02
    sta z:zDisplayDelay
    stz a:OBJ_TALLY_WAIT
    stz a:HitAttackFlags
    stz a:OBJ_WORK_F
    jsr StageSectionNext026D
    stz z:zClearKind
    stz a:OBJ_FRAME
    rts

; @routine 026D:6D4 StageClearedEnding026D
; @run 6B:86D4
; WHAT IT DOES: The ending of a cleared stage: sound $FF, the player bank's $A015, then on while zField8C's low bits are not 0 zModeDone counted; else the walk out's end.
; HOW: As the C.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearedEnding026D:
    lda #$FF
    jsr VecSoundCommand
    jsr MapOverlayBanks5And6At026D
    jsr FrameBankEntryA015
    lda z:zField8C
    and #$7F
    bne StageClearedEnding026D_at0
    stz z:zField8C
    jmp StageBossWalkOut026D_at0
StageClearedEnding026D_at0:
    sta z:zField8C
    inc z:zModeDone
    lda #$02
    sta z:zDisplayDelay
    stz a:OBJ_FRAME
    rts

; @routine 026D:3D8 StageSubModeCleared026D
; @run 6B:83D8
; WHAT IT DOES: The cleared sub-mode: zWaveCounter counted, then the way zClearKind names through this program's table at $83E1.
; HOW: INC, LDA, ASL, TAX, a jump through the table.
; WAITS: where the routines it jumps to wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageClearKinds026D = $83E1 ; this program's table of the cleared ways
StageSubModeCleared026D:
    inc z:zWaveCounter
    lda z:zClearKind
    asl a
    tax
    jmp (StageClearKinds026D,x)

; @routine 026D:76B StageSubModeFade026D
; @run 6B:876B
; WHAT IT DOES: The fade sub-mode: zWaveCounter counted, the stage frame; until step 3 the camera and scroll run with TimeSlowMode 3; then banks 5 and 6 and the step zField80 names through this program's table at $878E.
; HOW: As the C.
; WAITS: where the routines it calls wait.
; Entry: nothing.
; Nothing the game does loads this program (codemap.unloaded_programs), so nothing is known to reach it.
StageFadeSteps026D = $878E ; this program's table of the fade's steps
StageSubModeFade026D:
    inc z:zWaveCounter
    jsr StepStageFrameAt026D
    lda z:zField80
    cmp #$03
    beq StageSubModeFade026D_at0
    lda #$03
    sta a:TimeSlowMode
    jsr StageCameraStep026D
    jsr StepScrollAndRunObjects026E
    stz a:TimeSlowMode
StageSubModeFade026D_at0:
    jsr MapOverlayBanks5And6At026D
    lda z:zField80
    asl a
    tax
    jmp (StageFadeSteps026D,x)
