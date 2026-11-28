; @routine 024F:277 ExpandStageLists
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecords = $9B98 ; the list buffer's record by the section step (zField82)
ExpandStageLists:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecords,y
    sta z:zTemp00
    lda a:StageListRecords+1,y
    sta z:zTemp01
    cly
ExpandStageLists_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageLists_at0
    lda #$08
    bra ExpandStageLists_at1
ExpandStageLists_at7:
    lda (zTemp00)
    bne ExpandStageLists_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageLists_at1
ExpandStageLists_at2:
    cmp #$FF
    bne ExpandStageLists_at3
    rts
ExpandStageLists_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageLists_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageLists_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageLists_at5
    inc z:zTemp03
ExpandStageLists_at5:
    lda #$02
ExpandStageLists_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageLists_at6
    inc z:zTemp01
ExpandStageLists_at6:
    bra ExpandStageLists_at7

; @routine 050F:277 ExpandStageListsB
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9C28.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsB = $9C28 ; the list buffer's record by the section step (zField82)
ExpandStageListsB:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsB,y
    sta z:zTemp00
    lda a:StageListRecordsB+1,y
    sta z:zTemp01
    cly
ExpandStageListsB_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsB_at0
    lda #$08
    bra ExpandStageListsB_at1
ExpandStageListsB_at7:
    lda (zTemp00)
    bne ExpandStageListsB_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsB_at1
ExpandStageListsB_at2:
    cmp #$FF
    bne ExpandStageListsB_at3
    rts
ExpandStageListsB_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsB_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsB_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsB_at5
    inc z:zTemp03
ExpandStageListsB_at5:
    lda #$02
ExpandStageListsB_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsB_at6
    inc z:zTemp01
ExpandStageListsB_at6:
    bra ExpandStageListsB_at7

; @routine 05BF:277 ExpandStageListsC
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9E7F.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsC = $9E7F ; the list buffer's record by the section step (zField82)
ExpandStageListsC:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsC,y
    sta z:zTemp00
    lda a:StageListRecordsC+1,y
    sta z:zTemp01
    cly
ExpandStageListsC_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsC_at0
    lda #$08
    bra ExpandStageListsC_at1
ExpandStageListsC_at7:
    lda (zTemp00)
    bne ExpandStageListsC_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsC_at1
ExpandStageListsC_at2:
    cmp #$FF
    bne ExpandStageListsC_at3
    rts
ExpandStageListsC_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsC_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsC_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsC_at5
    inc z:zTemp03
ExpandStageListsC_at5:
    lda #$02
ExpandStageListsC_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsC_at6
    inc z:zTemp01
ExpandStageListsC_at6:
    bra ExpandStageListsC_at7

; @routine 071F:277 ExpandStageListsD
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9F43.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsD = $9F43 ; the list buffer's record by the section step (zField82)
ExpandStageListsD:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsD,y
    sta z:zTemp00
    lda a:StageListRecordsD+1,y
    sta z:zTemp01
    cly
ExpandStageListsD_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsD_at0
    lda #$08
    bra ExpandStageListsD_at1
ExpandStageListsD_at7:
    lda (zTemp00)
    bne ExpandStageListsD_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsD_at1
ExpandStageListsD_at2:
    cmp #$FF
    bne ExpandStageListsD_at3
    rts
ExpandStageListsD_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsD_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsD_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsD_at5
    inc z:zTemp03
ExpandStageListsD_at5:
    lda #$02
ExpandStageListsD_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsD_at6
    inc z:zTemp01
ExpandStageListsD_at6:
    bra ExpandStageListsD_at7

; @routine 07CF:277 ExpandStageListsE
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9CDA.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsE = $9CDA ; the list buffer's record by the section step (zField82)
ExpandStageListsE:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsE,y
    sta z:zTemp00
    lda a:StageListRecordsE+1,y
    sta z:zTemp01
    cly
ExpandStageListsE_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsE_at0
    lda #$08
    bra ExpandStageListsE_at1
ExpandStageListsE_at7:
    lda (zTemp00)
    bne ExpandStageListsE_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsE_at1
ExpandStageListsE_at2:
    cmp #$FF
    bne ExpandStageListsE_at3
    rts
ExpandStageListsE_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsE_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsE_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsE_at5
    inc z:zTemp03
ExpandStageListsE_at5:
    lda #$02
ExpandStageListsE_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsE_at6
    inc z:zTemp01
ExpandStageListsE_at6:
    bra ExpandStageListsE_at7

; @routine 087F:277 ExpandStageListsF
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9E65.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsF = $9E65 ; the list buffer's record by the section step (zField82)
ExpandStageListsF:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsF,y
    sta z:zTemp00
    lda a:StageListRecordsF+1,y
    sta z:zTemp01
    cly
ExpandStageListsF_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsF_at0
    lda #$08
    bra ExpandStageListsF_at1
ExpandStageListsF_at7:
    lda (zTemp00)
    bne ExpandStageListsF_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsF_at1
ExpandStageListsF_at2:
    cmp #$FF
    bne ExpandStageListsF_at3
    rts
ExpandStageListsF_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsF_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsF_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsF_at5
    inc z:zTemp03
ExpandStageListsF_at5:
    lda #$02
ExpandStageListsF_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsF_at6
    inc z:zTemp01
ExpandStageListsF_at6:
    bra ExpandStageListsF_at7

; @routine 092F:277 ExpandStageListsG
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9CD9.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsG = $9CD9 ; the list buffer's record by the section step (zField82)
ExpandStageListsG:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsG,y
    sta z:zTemp00
    lda a:StageListRecordsG+1,y
    sta z:zTemp01
    cly
ExpandStageListsG_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsG_at0
    lda #$08
    bra ExpandStageListsG_at1
ExpandStageListsG_at7:
    lda (zTemp00)
    bne ExpandStageListsG_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsG_at1
ExpandStageListsG_at2:
    cmp #$FF
    bne ExpandStageListsG_at3
    rts
ExpandStageListsG_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsG_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsG_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsG_at5
    inc z:zTemp03
ExpandStageListsG_at5:
    lda #$02
ExpandStageListsG_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsG_at6
    inc z:zTemp01
ExpandStageListsG_at6:
    bra ExpandStageListsG_at7

; @routine 09DF:277 ExpandStageListsH
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9EAC.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsH = $9EAC ; the list buffer's record by the section step (zField82)
ExpandStageListsH:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsH,y
    sta z:zTemp00
    lda a:StageListRecordsH+1,y
    sta z:zTemp01
    cly
ExpandStageListsH_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsH_at0
    lda #$08
    bra ExpandStageListsH_at1
ExpandStageListsH_at7:
    lda (zTemp00)
    bne ExpandStageListsH_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsH_at1
ExpandStageListsH_at2:
    cmp #$FF
    bne ExpandStageListsH_at3
    rts
ExpandStageListsH_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsH_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsH_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsH_at5
    inc z:zTemp03
ExpandStageListsH_at5:
    lda #$02
ExpandStageListsH_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsH_at6
    inc z:zTemp01
ExpandStageListsH_at6:
    bra ExpandStageListsH_at7

; @routine 0A8F:277 ExpandStageListsI
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9CB0.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsI = $9CB0 ; the list buffer's record by the section step (zField82)
ExpandStageListsI:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsI,y
    sta z:zTemp00
    lda a:StageListRecordsI+1,y
    sta z:zTemp01
    cly
ExpandStageListsI_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsI_at0
    lda #$08
    bra ExpandStageListsI_at1
ExpandStageListsI_at7:
    lda (zTemp00)
    bne ExpandStageListsI_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsI_at1
ExpandStageListsI_at2:
    cmp #$FF
    bne ExpandStageListsI_at3
    rts
ExpandStageListsI_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsI_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsI_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsI_at5
    inc z:zTemp03
ExpandStageListsI_at5:
    lda #$02
ExpandStageListsI_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsI_at6
    inc z:zTemp01
ExpandStageListsI_at6:
    bra ExpandStageListsI_at7

; @routine 0B3F:277 ExpandStageListsJ
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9E77.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsJ = $9E77 ; the list buffer's record by the section step (zField82)
ExpandStageListsJ:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsJ,y
    sta z:zTemp00
    lda a:StageListRecordsJ+1,y
    sta z:zTemp01
    cly
ExpandStageListsJ_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsJ_at0
    lda #$08
    bra ExpandStageListsJ_at1
ExpandStageListsJ_at7:
    lda (zTemp00)
    bne ExpandStageListsJ_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsJ_at1
ExpandStageListsJ_at2:
    cmp #$FF
    bne ExpandStageListsJ_at3
    rts
ExpandStageListsJ_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsJ_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsJ_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsJ_at5
    inc z:zTemp03
ExpandStageListsJ_at5:
    lda #$02
ExpandStageListsJ_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsJ_at6
    inc z:zTemp01
ExpandStageListsJ_at6:
    bra ExpandStageListsJ_at7

; @routine 0BEF:277 ExpandStageListsK
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9CC7.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsK = $9CC7 ; the list buffer's record by the section step (zField82)
ExpandStageListsK:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsK,y
    sta z:zTemp00
    lda a:StageListRecordsK+1,y
    sta z:zTemp01
    cly
ExpandStageListsK_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsK_at0
    lda #$08
    bra ExpandStageListsK_at1
ExpandStageListsK_at7:
    lda (zTemp00)
    bne ExpandStageListsK_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsK_at1
ExpandStageListsK_at2:
    cmp #$FF
    bne ExpandStageListsK_at3
    rts
ExpandStageListsK_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsK_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsK_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsK_at5
    inc z:zTemp03
ExpandStageListsK_at5:
    lda #$02
ExpandStageListsK_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsK_at6
    inc z:zTemp01
ExpandStageListsK_at6:
    bra ExpandStageListsK_at7

; @routine 0C9F:277 ExpandStageListsL
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9E7A.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsL = $9E7A ; the list buffer's record by the section step (zField82)
ExpandStageListsL:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsL,y
    sta z:zTemp00
    lda a:StageListRecordsL+1,y
    sta z:zTemp01
    cly
ExpandStageListsL_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsL_at0
    lda #$08
    bra ExpandStageListsL_at1
ExpandStageListsL_at7:
    lda (zTemp00)
    bne ExpandStageListsL_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsL_at1
ExpandStageListsL_at2:
    cmp #$FF
    bne ExpandStageListsL_at3
    rts
ExpandStageListsL_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsL_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsL_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsL_at5
    inc z:zTemp03
ExpandStageListsL_at5:
    lda #$02
ExpandStageListsL_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsL_at6
    inc z:zTemp01
ExpandStageListsL_at6:
    bra ExpandStageListsL_at7

; @routine 0D4F:277 ExpandStageListsM
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9CEB.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsM = $9CEB ; the list buffer's record by the section step (zField82)
ExpandStageListsM:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsM,y
    sta z:zTemp00
    lda a:StageListRecordsM+1,y
    sta z:zTemp01
    cly
ExpandStageListsM_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsM_at0
    lda #$08
    bra ExpandStageListsM_at1
ExpandStageListsM_at7:
    lda (zTemp00)
    bne ExpandStageListsM_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsM_at1
ExpandStageListsM_at2:
    cmp #$FF
    bne ExpandStageListsM_at3
    rts
ExpandStageListsM_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsM_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsM_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsM_at5
    inc z:zTemp03
ExpandStageListsM_at5:
    lda #$02
ExpandStageListsM_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsM_at6
    inc z:zTemp01
ExpandStageListsM_at6:
    bra ExpandStageListsM_at7

; @routine 0DFF:277 ExpandStageListsN
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9EA8.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsN = $9EA8 ; the list buffer's record by the section step (zField82)
ExpandStageListsN:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsN,y
    sta z:zTemp00
    lda a:StageListRecordsN+1,y
    sta z:zTemp01
    cly
ExpandStageListsN_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsN_at0
    lda #$08
    bra ExpandStageListsN_at1
ExpandStageListsN_at7:
    lda (zTemp00)
    bne ExpandStageListsN_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsN_at1
ExpandStageListsN_at2:
    cmp #$FF
    bne ExpandStageListsN_at3
    rts
ExpandStageListsN_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsN_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsN_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsN_at5
    inc z:zTemp03
ExpandStageListsN_at5:
    lda #$02
ExpandStageListsN_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsN_at6
    inc z:zTemp01
ExpandStageListsN_at6:
    bra ExpandStageListsN_at7

; @routine 0EAF:277 ExpandStageListsO
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9DFA.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsO = $9DFA ; the list buffer's record by the section step (zField82)
ExpandStageListsO:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsO,y
    sta z:zTemp00
    lda a:StageListRecordsO+1,y
    sta z:zTemp01
    cly
ExpandStageListsO_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsO_at0
    lda #$08
    bra ExpandStageListsO_at1
ExpandStageListsO_at7:
    lda (zTemp00)
    bne ExpandStageListsO_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsO_at1
ExpandStageListsO_at2:
    cmp #$FF
    bne ExpandStageListsO_at3
    rts
ExpandStageListsO_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsO_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsO_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsO_at5
    inc z:zTemp03
ExpandStageListsO_at5:
    lda #$02
ExpandStageListsO_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsO_at6
    inc z:zTemp01
ExpandStageListsO_at6:
    bra ExpandStageListsO_at7

; @routine 0F5F:277 ExpandStageListsP
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9F1D.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsP = $9F1D ; the list buffer's record by the section step (zField82)
ExpandStageListsP:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsP,y
    sta z:zTemp00
    lda a:StageListRecordsP+1,y
    sta z:zTemp01
    cly
ExpandStageListsP_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsP_at0
    lda #$08
    bra ExpandStageListsP_at1
ExpandStageListsP_at7:
    lda (zTemp00)
    bne ExpandStageListsP_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsP_at1
ExpandStageListsP_at2:
    cmp #$FF
    bne ExpandStageListsP_at3
    rts
ExpandStageListsP_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsP_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsP_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsP_at5
    inc z:zTemp03
ExpandStageListsP_at5:
    lda #$02
ExpandStageListsP_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsP_at6
    inc z:zTemp01
ExpandStageListsP_at6:
    bra ExpandStageListsP_at7

; @routine 100F:277 ExpandStageListsQ
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9C78.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsQ = $9C78 ; the list buffer's record by the section step (zField82)
ExpandStageListsQ:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsQ,y
    sta z:zTemp00
    lda a:StageListRecordsQ+1,y
    sta z:zTemp01
    cly
ExpandStageListsQ_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsQ_at0
    lda #$08
    bra ExpandStageListsQ_at1
ExpandStageListsQ_at7:
    lda (zTemp00)
    bne ExpandStageListsQ_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsQ_at1
ExpandStageListsQ_at2:
    cmp #$FF
    bne ExpandStageListsQ_at3
    rts
ExpandStageListsQ_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsQ_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsQ_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsQ_at5
    inc z:zTemp03
ExpandStageListsQ_at5:
    lda #$02
ExpandStageListsQ_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsQ_at6
    inc z:zTemp01
ExpandStageListsQ_at6:
    bra ExpandStageListsQ_at7

; @routine 10BF:277 ExpandStageListsR
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9F07.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsR = $9F07 ; the list buffer's record by the section step (zField82)
ExpandStageListsR:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsR,y
    sta z:zTemp00
    lda a:StageListRecordsR+1,y
    sta z:zTemp01
    cly
ExpandStageListsR_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsR_at0
    lda #$08
    bra ExpandStageListsR_at1
ExpandStageListsR_at7:
    lda (zTemp00)
    bne ExpandStageListsR_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsR_at1
ExpandStageListsR_at2:
    cmp #$FF
    bne ExpandStageListsR_at3
    rts
ExpandStageListsR_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsR_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsR_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsR_at5
    inc z:zTemp03
ExpandStageListsR_at5:
    lda #$02
ExpandStageListsR_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsR_at6
    inc z:zTemp01
ExpandStageListsR_at6:
    bra ExpandStageListsR_at7

; @routine 116F:277 ExpandStageListsS
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9C7D.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsS = $9C7D ; the list buffer's record by the section step (zField82)
ExpandStageListsS:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsS,y
    sta z:zTemp00
    lda a:StageListRecordsS+1,y
    sta z:zTemp01
    cly
ExpandStageListsS_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsS_at0
    lda #$08
    bra ExpandStageListsS_at1
ExpandStageListsS_at7:
    lda (zTemp00)
    bne ExpandStageListsS_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsS_at1
ExpandStageListsS_at2:
    cmp #$FF
    bne ExpandStageListsS_at3
    rts
ExpandStageListsS_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsS_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsS_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsS_at5
    inc z:zTemp03
ExpandStageListsS_at5:
    lda #$02
ExpandStageListsS_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsS_at6
    inc z:zTemp01
ExpandStageListsS_at6:
    bra ExpandStageListsS_at7

; @routine 12CF:277 ExpandStageListsT
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9F3D.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsT = $9F3D ; the list buffer's record by the section step (zField82)
ExpandStageListsT:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsT,y
    sta z:zTemp00
    lda a:StageListRecordsT+1,y
    sta z:zTemp01
    cly
ExpandStageListsT_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsT_at0
    lda #$08
    bra ExpandStageListsT_at1
ExpandStageListsT_at7:
    lda (zTemp00)
    bne ExpandStageListsT_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsT_at1
ExpandStageListsT_at2:
    cmp #$FF
    bne ExpandStageListsT_at3
    rts
ExpandStageListsT_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsT_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsT_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsT_at5
    inc z:zTemp03
ExpandStageListsT_at5:
    lda #$02
ExpandStageListsT_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsT_at6
    inc z:zTemp01
ExpandStageListsT_at6:
    bra ExpandStageListsT_at7

; @routine 160F:277 ExpandStageListsU
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9CA8.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsU = $9CA8 ; the list buffer's record by the section step (zField82)
ExpandStageListsU:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsU,y
    sta z:zTemp00
    lda a:StageListRecordsU+1,y
    sta z:zTemp01
    cly
ExpandStageListsU_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsU_at0
    lda #$08
    bra ExpandStageListsU_at1
ExpandStageListsU_at7:
    lda (zTemp00)
    bne ExpandStageListsU_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsU_at1
ExpandStageListsU_at2:
    cmp #$FF
    bne ExpandStageListsU_at3
    rts
ExpandStageListsU_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsU_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsU_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsU_at5
    inc z:zTemp03
ExpandStageListsU_at5:
    lda #$02
ExpandStageListsU_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsU_at6
    inc z:zTemp01
ExpandStageListsU_at6:
    bra ExpandStageListsU_at7

; @routine 18CF:277 ExpandStageListsV
; @run 6B:9277
; WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
; current stage section (zField82): the record's first eight bytes, then runs of one value
; stored at the places the record names.
; HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
; two bytes per section. After the first eight bytes, a zero byte sets the destination
; (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
; value after it, the destination moving on by the count; $FF ends the record.
; The same code as ExpandStageLists; this overlay's table of records is at $9EA4.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
; the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
; zTemp00-04.
StageListRecordsV = $9EA4 ; the list buffer's record by the section step (zField82)
ExpandStageListsV:
    tai KernelZeros,SaveBuffer,$0300
    lda z:zField82
    asl a
    tay
    lda a:StageListRecordsV,y
    sta z:zTemp00
    lda a:StageListRecordsV+1,y
    sta z:zTemp01
    cly
ExpandStageListsV_at0:
    lda (zTemp00),y
    sta a:SaveBuffer,y
    iny
    cpy #$08
    bne ExpandStageListsV_at0
    lda #$08
    bra ExpandStageListsV_at1
ExpandStageListsV_at7:
    lda (zTemp00)
    bne ExpandStageListsV_at2
    ldy #$01
    lda (zTemp00),y
    sta z:zTemp02
    iny
    lda (zTemp00),y
    sta z:zTemp03
    lda #$03
    bra ExpandStageListsV_at1
ExpandStageListsV_at2:
    cmp #$FF
    bne ExpandStageListsV_at3
    rts
ExpandStageListsV_at3:
    sta z:zTemp04
    tax
    ldy #$01
    lda (zTemp00),y
    cly
ExpandStageListsV_at4:
    sta (zTemp02),y
    iny
    dex
    bne ExpandStageListsV_at4
    lda z:zTemp04
    clc
    adc z:zTemp02
    sta z:zTemp02
    bcc ExpandStageListsV_at5
    inc z:zTemp03
ExpandStageListsV_at5:
    lda #$02
ExpandStageListsV_at1:
    clc
    adc z:zTemp00
    sta z:zTemp00
    bcc ExpandStageListsV_at6
    inc z:zTemp01
ExpandStageListsV_at6:
    bra ExpandStageListsV_at7
