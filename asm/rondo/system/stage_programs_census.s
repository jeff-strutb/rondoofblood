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
