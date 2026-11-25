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
StartStageLists:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9B86,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageLists
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9B8E,x
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
StartStageListsB:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9C12,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsB
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9C1A,x
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
StartStageListsC:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9E6D,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsC
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9E75,x
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
StartStageListsD:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9F31,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsD
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9F39,x
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
StartStageListsE:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9CC6,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsE
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9CCA,x
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
StartStageListsF:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9E61,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsF
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9E62,x
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
StartStageListsG:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9CC3,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsG
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9CCB,x
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
StartStageListsH:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9EA8,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsH
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9EA9,x
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
StartStageListsI:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9C9C,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsI
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9CA4,x
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
StartStageListsJ:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9E73,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsJ
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9E74,x
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
StartStageListsK:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9CBB,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsK
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9CBE,x
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
StartStageListsL:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9E76,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsL
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9E77,x
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
StartStageListsM:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9CDB,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsM
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9CDF,x
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
StartStageListsN:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9EA4,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsN
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9EA5,x
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
StartStageListsO:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9DEA,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsO
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9DEE,x
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
StartStageListsP:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9F19,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsP
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9F1A,x
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
StartStageListsQ:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9C6C,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsQ
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9C6F,x
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
StartStageListsR:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9F03,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsR
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9F04,x
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
StartStageListsS:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9C65,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsS
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9C6B,x
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
StartStageListsT:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9F27,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsT
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9F2F,x
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
StartStageListsU:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9C9C,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsU
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9C9F,x
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
StartStageListsV:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9F3F,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsU
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9F40,x
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
StartStageListsW:
    jsr VecClearListChannels
    ldx z:zField82
    lda a:$9EA0,x
    jsr VecStartListChannel0Kind2
    jsr ExpandStageListsV
    lda z:zField82
    asl a
    clc
    adc z:zCharacter
    tax
    lda a:$9EA1,x
    ldx #$06
    jmp VecStartScriptSlotKind1
