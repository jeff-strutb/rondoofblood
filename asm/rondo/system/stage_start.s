; @routine 024E:0B1 StartStageLists
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStep = $9B86 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStep = $9B8E ; slot 6's script, by twice the section step plus the character
StartStageLists:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStep,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStep,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 050E:0B1 StartStageListsB
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9C12 and $9C1A.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepB = $9C12 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepB = $9C1A ; slot 6's script, by twice the section step plus the character
StartStageListsB:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepB,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsB
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepB,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 05BE:0B1 StartStageListsC
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9E6D and $9E75.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepC = $9E6D ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepC = $9E75 ; slot 6's script, by twice the section step plus the character
StartStageListsC:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepC,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsC
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepC,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 071E:0B1 StartStageListsD
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9F31 and $9F39.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepD = $9F31 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepD = $9F39 ; slot 6's script, by twice the section step plus the character
StartStageListsD:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepD,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsD
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepD,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 07CE:0B1 StartStageListsE
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9CC6 and $9CCA.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepE = $9CC6 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepE = $9CCA ; slot 6's script, by twice the section step plus the character
StartStageListsE:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepE,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsE
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepE,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 087E:0B1 StartStageListsF
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9E61 and $9E62.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepF = $9E61 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepF = $9E62 ; slot 6's script, by twice the section step plus the character
StartStageListsF:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepF,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsF
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepF,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 092E:0B1 StartStageListsG
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9CC3 and $9CCB.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepG = $9CC3 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepG = $9CCB ; slot 6's script, by twice the section step plus the character
StartStageListsG:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepG,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsG
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepG,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 09DE:0B1 StartStageListsH
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9EA8 and $9EA9.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepH = $9EA8 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepH = $9EA9 ; slot 6's script, by twice the section step plus the character
StartStageListsH:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepH,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsH
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepH,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 0A8E:0B1 StartStageListsI
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9C9C and $9CA4.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepI = $9C9C ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepI = $9CA4 ; slot 6's script, by twice the section step plus the character
StartStageListsI:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepI,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsI
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepI,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 0B3E:0B1 StartStageListsJ
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9E73 and $9E74.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepJ = $9E73 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepJ = $9E74 ; slot 6's script, by twice the section step plus the character
StartStageListsJ:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepJ,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsJ
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepJ,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 0BEE:0B1 StartStageListsK
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9CBB and $9CBE.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepK = $9CBB ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepK = $9CBE ; slot 6's script, by twice the section step plus the character
StartStageListsK:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepK,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsK
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepK,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 0C9E:0B1 StartStageListsL
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9E76 and $9E77.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepL = $9E76 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepL = $9E77 ; slot 6's script, by twice the section step plus the character
StartStageListsL:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepL,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsL
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepL,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 0D4E:0B1 StartStageListsM
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9CDB and $9CDF.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepM = $9CDB ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepM = $9CDF ; slot 6's script, by twice the section step plus the character
StartStageListsM:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepM,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsM
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepM,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 0DFE:0B1 StartStageListsN
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9EA4 and $9EA5.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepN = $9EA4 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepN = $9EA5 ; slot 6's script, by twice the section step plus the character
StartStageListsN:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepN,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsN
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepN,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 0EAE:0B1 StartStageListsO
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9DEA and $9DEE.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepO = $9DEA ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepO = $9DEE ; slot 6's script, by twice the section step plus the character
StartStageListsO:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepO,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsO
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepO,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 0F5E:0B1 StartStageListsP
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9F19 and $9F1A.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepP = $9F19 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepP = $9F1A ; slot 6's script, by twice the section step plus the character
StartStageListsP:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepP,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsP
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepP,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 100E:0B1 StartStageListsQ
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9C6C and $9C6F.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepQ = $9C6C ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepQ = $9C6F ; slot 6's script, by twice the section step plus the character
StartStageListsQ:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepQ,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsQ
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepQ,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 10BE:0B1 StartStageListsR
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9F03 and $9F04.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepR = $9F03 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepR = $9F04 ; slot 6's script, by twice the section step plus the character
StartStageListsR:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepR,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsR
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepR,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 116E:0B1 StartStageListsS
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9C65 and $9C6B.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepS = $9C65 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepS = $9C6B ; slot 6's script, by twice the section step plus the character
StartStageListsS:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepS,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsS
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepS,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 12CE:0B1 StartStageListsT
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9F27 and $9F2F.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepT = $9F27 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepT = $9F2F ; slot 6's script, by twice the section step plus the character
StartStageListsT:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepT,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsT
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepT,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 160E:0B1 StartStageListsU
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9C9C and $9C9F.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepU = $9C9C ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepU = $9C9F ; slot 6's script, by twice the section step plus the character
StartStageListsU:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepU,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsU
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepU,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 16BE:0B1 StartStageListsV
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9F3F and $9F40.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepV = $9F3F ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepV = $9F40 ; slot 6's script, by twice the section step plus the character
StartStageListsV:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepV,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsU
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepV,x
    ldx #$06
    jmp VecStartScriptSlotKind1

; @routine 18CE:0B1 StartStageListsW
; @run 6B:88B1
; WHAT IT DOES: Starts the stage's lists: the list channel for the section, the section's stage
; lists built into the list buffer, and the section's script in script slot 6.
; HOW: Clears the list channels, starts channel 0 as kind 2 with the list the overlay's table
; names for zField82, builds the section's lists, then starts slot 6 with the script from the
; overlay's second table at twice zField82 plus the character.
; The same code as StartStageLists; this overlay's tables are at $9EA0 and $9EA1.
; WAITS: no.
; Entry: nothing. Exit: A and Y the script, X 6, N and Z from it, C and V from the table
; index add.
StageListByStepW = $9EA0 ; the list channel 0 starts, by the section step (zField82)
StageScriptByStepW = $9EA1 ; slot 6's script, by twice the section step plus the character
StartStageListsW:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:StageListByStepW,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsV
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:StageScriptByStepW,x
    ldx #$06
    jmp VecStartScriptSlotKind1
