; @routine 024D:0CB ResetStageVariables024D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C20D.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C2 and N set. X, Y, C, V kept.
ResetStageVariables024D:
    lda z:zGameRunning
    beq ResetStageVariables024D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$0D
    sta z:$7E
    lda #$C2
    sta z:$7F
ResetStageVariables024D_at0:
    rts

; @routine 050D:0CB ResetStageVariables050D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $CE66.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $CE and N set. X, Y, C, V kept.
ResetStageVariables050D:
    lda z:zGameRunning
    beq ResetStageVariables050D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$66
    sta z:$7E
    lda #$CE
    sta z:$7F
ResetStageVariables050D_at0:
    rts

; @routine 05BD:0CB ResetStageVariables05BD
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $BE15.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $BE and N set. X, Y, C, V kept.
ResetStageVariables05BD:
    lda z:zGameRunning
    beq ResetStageVariables05BD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$15
    sta z:$7E
    lda #$BE
    sta z:$7F
ResetStageVariables05BD_at0:
    rts

; @routine 071D:0CB ResetStageVariables071D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C24E.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C2 and N set. X, Y, C, V kept.
ResetStageVariables071D:
    lda z:zGameRunning
    beq ResetStageVariables071D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$4E
    sta z:$7E
    lda #$C2
    sta z:$7F
ResetStageVariables071D_at0:
    rts

; @routine 07CD:0CB ResetStageVariables07CD
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DC50.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DC and N set. X, Y, C, V kept.
ResetStageVariables07CD:
    lda z:zGameRunning
    beq ResetStageVariables07CD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$50
    sta z:$7E
    lda #$DC
    sta z:$7F
ResetStageVariables07CD_at0:
    rts

; @routine 087D:0CB ResetStageVariables087D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C3AA.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C3 and N set. X, Y, C, V kept.
ResetStageVariables087D:
    lda z:zGameRunning
    beq ResetStageVariables087D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$AA
    sta z:$7E
    lda #$C3
    sta z:$7F
ResetStageVariables087D_at0:
    rts

; @routine 092D:0CB ResetStageVariables092D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DB8F.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DB and N set. X, Y, C, V kept.
ResetStageVariables092D:
    lda z:zGameRunning
    beq ResetStageVariables092D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$8F
    sta z:$7E
    lda #$DB
    sta z:$7F
ResetStageVariables092D_at0:
    rts

; @routine 09DD:0CB ResetStageVariables09DD
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C274.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C2 and N set. X, Y, C, V kept.
ResetStageVariables09DD:
    lda z:zGameRunning
    beq ResetStageVariables09DD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$74
    sta z:$7E
    lda #$C2
    sta z:$7F
ResetStageVariables09DD_at0:
    rts

; @routine 0A8D:0CB ResetStageVariables0A8D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DDD7.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DD and N set. X, Y, C, V kept.
ResetStageVariables0A8D:
    lda z:zGameRunning
    beq ResetStageVariables0A8D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$D7
    sta z:$7E
    lda #$DD
    sta z:$7F
ResetStageVariables0A8D_at0:
    rts

; @routine 0B3D:0CB ResetStageVariables0B3D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D46F.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D4 and N set. X, Y, C, V kept.
ResetStageVariables0B3D:
    lda z:zGameRunning
    beq ResetStageVariables0B3D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$6F
    sta z:$7E
    lda #$D4
    sta z:$7F
ResetStageVariables0B3D_at0:
    rts

; @routine 0BED:0CB ResetStageVariables0BED
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DA06.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DA and N set. X, Y, C, V kept.
ResetStageVariables0BED:
    lda z:zGameRunning
    beq ResetStageVariables0BED_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$06
    sta z:$7E
    lda #$DA
    sta z:$7F
ResetStageVariables0BED_at0:
    rts

; @routine 0C9D:0CB ResetStageVariables0C9D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C31A.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C3 and N set. X, Y, C, V kept.
ResetStageVariables0C9D:
    lda z:zGameRunning
    beq ResetStageVariables0C9D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$1A
    sta z:$7E
    lda #$C3
    sta z:$7F
ResetStageVariables0C9D_at0:
    rts

; @routine 0D4D:0CB ResetStageVariables0D4D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D937.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D9 and N set. X, Y, C, V kept.
ResetStageVariables0D4D:
    lda z:zGameRunning
    beq ResetStageVariables0D4D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$37
    sta z:$7E
    lda #$D9
    sta z:$7F
ResetStageVariables0D4D_at0:
    rts

; @routine 0DFD:0CB ResetStageVariables0DFD
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C15C.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C1 and N set. X, Y, C, V kept.
ResetStageVariables0DFD:
    lda z:zGameRunning
    beq ResetStageVariables0DFD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$5C
    sta z:$7E
    lda #$C1
    sta z:$7F
ResetStageVariables0DFD_at0:
    rts

; @routine 0EAD:0CB ResetStageVariables0EAD
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $B389.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $B3 and N set. X, Y, C, V kept.
ResetStageVariables0EAD:
    lda z:zGameRunning
    beq ResetStageVariables0EAD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$89
    sta z:$7E
    lda #$B3
    sta z:$7F
ResetStageVariables0EAD_at0:
    rts

; @routine 0F5D:0CB ResetStageVariables0F5D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $B1F3.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $B1 and N set. X, Y, C, V kept.
ResetStageVariables0F5D:
    lda z:zGameRunning
    beq ResetStageVariables0F5D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$F3
    sta z:$7E
    lda #$B1
    sta z:$7F
ResetStageVariables0F5D_at0:
    rts

; @routine 100D:0CB ResetStageVariables100D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D556.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D5 and N set. X, Y, C, V kept.
ResetStageVariables100D:
    lda z:zGameRunning
    beq ResetStageVariables100D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$56
    sta z:$7E
    lda #$D5
    sta z:$7F
ResetStageVariables100D_at0:
    rts

; @routine 10BD:0CB ResetStageVariables10BD
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C58C.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C5 and N set. X, Y, C, V kept.
ResetStageVariables10BD:
    lda z:zGameRunning
    beq ResetStageVariables10BD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$8C
    sta z:$7E
    lda #$C5
    sta z:$7F
ResetStageVariables10BD_at0:
    rts

; @routine 116D:0CB ResetStageVariables116D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $DB63.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $DB and N set. X, Y, C, V kept.
ResetStageVariables116D:
    lda z:zGameRunning
    beq ResetStageVariables116D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$63
    sta z:$7E
    lda #$DB
    sta z:$7F
ResetStageVariables116D_at0:
    rts

; @routine 12CD:0CB ResetStageVariables12CD
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D25A.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D2 and N set. X, Y, C, V kept.
ResetStageVariables12CD:
    lda z:zGameRunning
    beq ResetStageVariables12CD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$5A
    sta z:$7E
    lda #$D2
    sta z:$7F
ResetStageVariables12CD_at0:
    rts

; @routine 160D:0CB ResetStageVariables160D
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $D916.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $D9 and N set. X, Y, C, V kept.
ResetStageVariables160D:
    lda z:zGameRunning
    beq ResetStageVariables160D_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$16
    sta z:$7E
    lda #$D9
    sta z:$7F
ResetStageVariables160D_at0:
    rts

; @routine 16BD:0CB ResetStageVariables16BD
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $BF26.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $BF and N set. X, Y, C, V kept.
ResetStageVariables16BD:
    lda z:zGameRunning
    beq ResetStageVariables16BD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$26
    sta z:$7E
    lda #$BF
    sta z:$7F
ResetStageVariables16BD_at0:
    rts

; @routine 18CD:0CB ResetStageVariables18CD
; @run 6B:80CB
; WHAT IT DOES: While the game runs, clears the random bytes, the work bytes zWaveCounter and zFrameTick and $207C-$207D, and points $207E at $C373.
; HOW: One test of zGameRunning, then the stores; the shared body is reset_stage_variables.
; WAITS: no.
; Entry: nothing. Exit: A zGameRunning, N and Z from it when it is zero; otherwise A $C3 and N set. X, Y, C, V kept.
ResetStageVariables18CD:
    lda z:zGameRunning
    beq ResetStageVariables18CD_at0
    stz z:zRandom0
    stz z:zRandom1
    stz z:zRandom2
    stz z:zRandom3
    stz z:zWaveCounter
    stz z:zFrameTick
    stz z:$7C
    stz z:$7D
    lda #$73
    sta z:$7E
    lda #$C3
    sta z:$7F
ResetStageVariables18CD_at0:
    rts
