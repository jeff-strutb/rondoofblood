; @routine 024E:236 StageScrollStep
; @run 6B:8A36
; WHAT IT DOES: Maps the stage's two bank pairs, the first at offset 8 above the RAM base
; bank and the second at the offset the overlay keeps, into MPR5 and MPR6, then steps the
; stream with the scroll.
; HOW: Each pair is the base bank plus an offset into MPR5 and the next bank into MPR6. The
; step is StreamScrollStep, called through its vector.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as StreamScrollStep.
StageScrollStep:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC95
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
StageScrollStepB:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ACCA
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
StageScrollStepC:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC0E
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
StageScrollStepD:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC16
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
StageScrollStepE:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ABDC
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
StageScrollStepF:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AB03
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
StageScrollStepG:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC8E
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
StageScrollStepH:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC5E
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
StageScrollStepI:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ACE3
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
StageScrollStepJ:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AB27
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
StageScrollStepK:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ACA7
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
StageScrollStepL:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC8B
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
StageScrollStepM:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD91
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
StageScrollStepN:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AB70
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
StageScrollStepO:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD3F
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
StageScrollStepP:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ABA7
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
StageScrollStepQ:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD1B
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
StageScrollStepR:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ACC9
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
StageScrollStepS:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD36
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
StageScrollStepT:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$ACF7
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
StageScrollStepU:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AD31
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
StageScrollStepV:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC65
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
StageScrollStepW:
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    lda a:$AC49
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jsr VecStreamScrollStep
    rts
