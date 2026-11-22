; @routine 024F:19F ClearStageStateBytes
; @run 6B:919F
; WHAT IT DOES: Zeroes four of the stage's state bytes, the ones a stage restart or its
; start-of-stage handler clears together.
; HOW: Four stores of zero, each into slot $56 of one column of the object table (columns
; $34B8, $3510, $3568 and $3670). The columns are the object table's, the use is the stage
; overlay's own; the field names are the table's until the typed layer names them.
; WAITS: no.
; Entry: nothing. Exit: A, X, Y and flags kept.
ClearStageStateBytes:
    stz a:OBJ_34B8+$56
    stz a:OBJ_3510+$56
    stz a:OBJ_3568+$56
    stz a:OBJ_3670+$56
    rts

; @routine 024F:33F SaveAndMapStageBanks
; @run 6B:933F
; WHAT IT DOES: Keeps the banks in MPR5 and MPR6 and then maps the stage's two banks there.
; HOW: Each MPR into its saved byte (slot $57 of columns $3670 and $36C8 of the object
; table, the stage's own use), then MapStageBanks, which this routine runs on into with no
; return of its own. RestoreStageBanks puts the saved ones back.
; WAITS: no.
; Entry: nothing. Exit: as MapStageBanks.
SaveAndMapStageBanks:
    tma #$20
    sta a:OBJ_3670+$57
    tma #$40
    sta a:OBJ_36C8+$57

; @routine 024F:349 MapStageBanks
; @run 6B:9349
; WHAT IT DOES: Maps the stage's two Super RAM banks into MPR5 and MPR6 ($A000 and $C000):
; the banks $10 and $11 above the card's first Super RAM bank.
; HOW: $10 plus zRamBaseBank into MPR5, that plus one into MPR6.
; WAITS: no.
; Entry: nothing. Exit: A the MPR6 bank, flags from the add and the increment (C and V from
; the add, N and Z from A). X and Y kept.
MapStageBanks:
    lda #$10
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    rts

; @routine 024F:354 RestoreStageBanks
; @run 6B:9354
; WHAT IT DOES: Puts back the banks SaveAndMapStageBanks kept in MPR5 and MPR6.
; HOW: MPR6 from the byte at slot $57 of column $36C8, then MPR5 from slot $57 of $3670.
; WAITS: no.
; Entry: nothing. Exit: A the MPR5 bank, N and Z from it. X and Y kept.
RestoreStageBanks:
    lda a:OBJ_36C8+$57
    tam #$40
    lda a:OBJ_3670+$57
    tam #$20
    rts

; @routine 05BF:651 SaveAndMapStageBanksLong
; @run 6B:9651
; WHAT IT DOES: SaveAndMapStageBanks, as the stages whose overlay runs it at $9651 have it.
; HOW: The same bytes as SaveAndMapStageBanks at another run address; runs on into
; MapStageBanksLong. Its C is SaveAndMapStageBanks.
; WAITS: no.
; Entry: nothing. Exit: as MapStageBanks.
SaveAndMapStageBanksLong:
    tma #$20
    sta a:OBJ_3670+$57
    tma #$40
    sta a:OBJ_36C8+$57

; @routine 05BF:65B MapStageBanksLong
; @run 6B:965B
; WHAT IT DOES: MapStageBanks, as the stages whose overlay runs it at $965B have it.
; HOW: The same bytes as MapStageBanks at another run address. Its C is MapStageBanks.
; WAITS: no.
; Entry: nothing. Exit: as MapStageBanks.
MapStageBanksLong:
    lda #$10
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    rts

; @routine 05BF:666 RestoreStageBanksLong
; @run 6B:9666
; WHAT IT DOES: RestoreStageBanks, as the stages whose overlay runs it at $9666 have it.
; HOW: The same bytes as RestoreStageBanks at another run address. Its C is RestoreStageBanks.
; WAITS: no.
; Entry: nothing. Exit: as RestoreStageBanks.
RestoreStageBanksLong:
    lda a:OBJ_36C8+$57
    tam #$40
    lda a:OBJ_3670+$57
    tam #$20
RestoreStageBanksLong_at0:
    rts
