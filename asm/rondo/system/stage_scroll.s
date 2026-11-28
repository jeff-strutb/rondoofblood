; @routine 024E:236 StageScrollStep
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffset = $AC95 ; the second bank pair's offset above the RAM base bank
StageScrollStep:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffset
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 050E:236 StageScrollStepB
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACCA.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetB = $ACCA ; the second bank pair's offset above the RAM base bank
StageScrollStepB:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetB
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 05BE:236 StageScrollStepC
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC0E.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetC = $AC0E ; the second bank pair's offset above the RAM base bank
StageScrollStepC:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetC
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 071E:236 StageScrollStepD
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC16.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetD = $AC16 ; the second bank pair's offset above the RAM base bank
StageScrollStepD:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetD
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 07CE:236 StageScrollStepE
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $ABDC.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetE = $ABDC ; the second bank pair's offset above the RAM base bank
StageScrollStepE:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetE
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 087E:236 StageScrollStepF
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AB03.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetF = $AB03 ; the second bank pair's offset above the RAM base bank
StageScrollStepF:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetF
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 092E:236 StageScrollStepG
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC8E.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetG = $AC8E ; the second bank pair's offset above the RAM base bank
StageScrollStepG:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetG
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 09DE:236 StageScrollStepH
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC5E.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetH = $AC5E ; the second bank pair's offset above the RAM base bank
StageScrollStepH:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetH
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 0A8E:236 StageScrollStepI
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACE3.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetI = $ACE3 ; the second bank pair's offset above the RAM base bank
StageScrollStepI:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetI
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 0B3E:236 StageScrollStepJ
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AB27.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetJ = $AB27 ; the second bank pair's offset above the RAM base bank
StageScrollStepJ:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetJ
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 0BEE:236 StageScrollStepK
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACA7.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetK = $ACA7 ; the second bank pair's offset above the RAM base bank
StageScrollStepK:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetK
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 0C9E:236 StageScrollStepL
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC8B.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetL = $AC8B ; the second bank pair's offset above the RAM base bank
StageScrollStepL:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetL
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 0D4E:236 StageScrollStepM
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD91.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetM = $AD91 ; the second bank pair's offset above the RAM base bank
StageScrollStepM:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetM
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 0DFE:236 StageScrollStepN
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AB70.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetN = $AB70 ; the second bank pair's offset above the RAM base bank
StageScrollStepN:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetN
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 0EAE:236 StageScrollStepO
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD3F.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetO = $AD3F ; the second bank pair's offset above the RAM base bank
StageScrollStepO:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetO
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 0F5E:236 StageScrollStepP
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $ABA7.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetP = $ABA7 ; the second bank pair's offset above the RAM base bank
StageScrollStepP:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetP
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 100E:236 StageScrollStepQ
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD1B.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetQ = $AD1B ; the second bank pair's offset above the RAM base bank
StageScrollStepQ:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetQ
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 10BE:236 StageScrollStepR
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACC9.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetR = $ACC9 ; the second bank pair's offset above the RAM base bank
StageScrollStepR:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetR
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 116E:236 StageScrollStepS
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD36.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetS = $AD36 ; the second bank pair's offset above the RAM base bank
StageScrollStepS:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetS
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 12CE:236 StageScrollStepT
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $ACF7.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetT = $ACF7 ; the second bank pair's offset above the RAM base bank
StageScrollStepT:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetT
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 160E:236 StageScrollStepU
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AD31.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetU = $AD31 ; the second bank pair's offset above the RAM base bank
StageScrollStepU:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetU
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 16BE:236 StageScrollStepV
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC65.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetV = $AC65 ; the second bank pair's offset above the RAM base bank
StageScrollStepV:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetV
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts

; @routine 18CE:236 StageScrollStepW
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; The same code as StageScrollStep; this overlay's second bank offset is the byte at $AC49.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageStreamBankOffsetW = $AC49 ; the second bank pair's offset above the RAM base bank
StageScrollStepW:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:StageStreamBankOffsetW
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts
