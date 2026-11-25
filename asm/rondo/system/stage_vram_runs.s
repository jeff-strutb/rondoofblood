; @routine 024E:082 ExpandStageVramRuns
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRuns:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9BA0,y
    sta z:zTemp00
    lda a:$9BA1,y
    sta z:zTemp01
    cly
ExpandStageVramRuns_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRuns_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRuns_at0
    rts
ExpandStageVramRuns_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRuns_at1
    bra ExpandStageVramRuns_at2

; @routine 050E:082 ExpandStageVramRunsB
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9C30.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsB:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9C30,y
    sta z:zTemp00
    lda a:$9C31,y
    sta z:zTemp01
    cly
ExpandStageVramRunsB_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsB_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsB_at0
    rts
ExpandStageVramRunsB_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsB_at1
    bra ExpandStageVramRunsB_at2

; @routine 05BE:082 ExpandStageVramRunsC
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E87.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsC:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9E87,y
    sta z:zTemp00
    lda a:$9E88,y
    sta z:zTemp01
    cly
ExpandStageVramRunsC_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsC_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsC_at0
    rts
ExpandStageVramRunsC_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsC_at1
    bra ExpandStageVramRunsC_at2

; @routine 071E:082 ExpandStageVramRunsD
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9F4B.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsD:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9F4B,y
    sta z:zTemp00
    lda a:$9F4C,y
    sta z:zTemp01
    cly
ExpandStageVramRunsD_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsD_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsD_at0
    rts
ExpandStageVramRunsD_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsD_at1
    bra ExpandStageVramRunsD_at2

; @routine 07CE:082 ExpandStageVramRunsE
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CE2.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsE:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9CE2,y
    sta z:zTemp00
    lda a:$9CE3,y
    sta z:zTemp01
    cly
ExpandStageVramRunsE_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsE_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsE_at0
    rts
ExpandStageVramRunsE_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsE_at1
    bra ExpandStageVramRunsE_at2

; @routine 087E:082 ExpandStageVramRunsF
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E67.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsF:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9E67,y
    sta z:zTemp00
    lda a:$9E68,y
    sta z:zTemp01
    cly
ExpandStageVramRunsF_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsF_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsF_at0
    rts
ExpandStageVramRunsF_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsF_at1
    bra ExpandStageVramRunsF_at2

; @routine 092E:082 ExpandStageVramRunsG
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CDF.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsG:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9CDF,y
    sta z:zTemp00
    lda a:$9CE0,y
    sta z:zTemp01
    cly
ExpandStageVramRunsG_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsG_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsG_at0
    rts
ExpandStageVramRunsG_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsG_at1
    bra ExpandStageVramRunsG_at2

; @routine 09DE:082 ExpandStageVramRunsH
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9EAE.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsH:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9EAE,y
    sta z:zTemp00
    lda a:$9EAF,y
    sta z:zTemp01
    cly
ExpandStageVramRunsH_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsH_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsH_at0
    rts
ExpandStageVramRunsH_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsH_at1
    bra ExpandStageVramRunsH_at2

; @routine 0A8E:082 ExpandStageVramRunsI
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CB8.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsI:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9CB8,y
    sta z:zTemp00
    lda a:$9CB9,y
    sta z:zTemp01
    cly
ExpandStageVramRunsI_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsI_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsI_at0
    rts
ExpandStageVramRunsI_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsI_at1
    bra ExpandStageVramRunsI_at2

; @routine 0B3E:082 ExpandStageVramRunsJ
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E79.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsJ:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9E79,y
    sta z:zTemp00
    lda a:$9E7A,y
    sta z:zTemp01
    cly
ExpandStageVramRunsJ_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsJ_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsJ_at0
    rts
ExpandStageVramRunsJ_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsJ_at1
    bra ExpandStageVramRunsJ_at2

; @routine 0BEE:082 ExpandStageVramRunsK
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CCD.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsK:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9CCD,y
    sta z:zTemp00
    lda a:$9CCE,y
    sta z:zTemp01
    cly
ExpandStageVramRunsK_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsK_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsK_at0
    rts
ExpandStageVramRunsK_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsK_at1
    bra ExpandStageVramRunsK_at2

; @routine 0C9E:082 ExpandStageVramRunsL
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E7C.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsL:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9E7C,y
    sta z:zTemp00
    lda a:$9E7D,y
    sta z:zTemp01
    cly
ExpandStageVramRunsL_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsL_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsL_at0
    rts
ExpandStageVramRunsL_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsL_at1
    bra ExpandStageVramRunsL_at2

; @routine 0D4E:082 ExpandStageVramRunsM
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CF3.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsM:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9CF3,y
    sta z:zTemp00
    lda a:$9CF4,y
    sta z:zTemp01
    cly
ExpandStageVramRunsM_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsM_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsM_at0
    rts
ExpandStageVramRunsM_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsM_at1
    bra ExpandStageVramRunsM_at2

; @routine 0DFE:082 ExpandStageVramRunsN
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9EAA.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsN:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9EAA,y
    sta z:zTemp00
    lda a:$9EAB,y
    sta z:zTemp01
    cly
ExpandStageVramRunsN_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsN_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsN_at0
    rts
ExpandStageVramRunsN_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsN_at1
    bra ExpandStageVramRunsN_at2

; @routine 0EAE:082 ExpandStageVramRunsO
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9E04.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsO:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9E04,y
    sta z:zTemp00
    lda a:$9E05,y
    sta z:zTemp01
    cly
ExpandStageVramRunsO_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsO_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsO_at0
    rts
ExpandStageVramRunsO_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsO_at1
    bra ExpandStageVramRunsO_at2

; @routine 0F5E:082 ExpandStageVramRunsP
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9F1F.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsP:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9F1F,y
    sta z:zTemp00
    lda a:$9F20,y
    sta z:zTemp01
    cly
ExpandStageVramRunsP_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsP_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsP_at0
    rts
ExpandStageVramRunsP_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsP_at1
    bra ExpandStageVramRunsP_at2

; @routine 100E:082 ExpandStageVramRunsQ
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9C7E.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsQ:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9C7E,y
    sta z:zTemp00
    lda a:$9C7F,y
    sta z:zTemp01
    cly
ExpandStageVramRunsQ_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsQ_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsQ_at0
    rts
ExpandStageVramRunsQ_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsQ_at1
    bra ExpandStageVramRunsQ_at2

; @routine 10BE:082 ExpandStageVramRunsR
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9F09.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsR:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9F09,y
    sta z:zTemp00
    lda a:$9F0A,y
    sta z:zTemp01
    cly
ExpandStageVramRunsR_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsR_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsR_at0
    rts
ExpandStageVramRunsR_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsR_at1
    bra ExpandStageVramRunsR_at2

; @routine 116E:082 ExpandStageVramRunsS
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9C89.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsS:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9C89,y
    sta z:zTemp00
    lda a:$9C8A,y
    sta z:zTemp01
    cly
ExpandStageVramRunsS_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsS_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsS_at0
    rts
ExpandStageVramRunsS_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsS_at1
    bra ExpandStageVramRunsS_at2

; @routine 12CE:082 ExpandStageVramRunsT
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9F45.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsT:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9F45,y
    sta z:zTemp00
    lda a:$9F46,y
    sta z:zTemp01
    cly
ExpandStageVramRunsT_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsT_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsT_at0
    rts
ExpandStageVramRunsT_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsT_at1
    bra ExpandStageVramRunsT_at2

; @routine 160E:082 ExpandStageVramRunsU
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9CAE.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsU:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9CAE,y
    sta z:zTemp00
    lda a:$9CAF,y
    sta z:zTemp01
    cly
ExpandStageVramRunsU_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsU_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsU_at0
    rts
ExpandStageVramRunsU_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsU_at1
    bra ExpandStageVramRunsU_at2

; @routine 18CE:082 ExpandStageVramRunsV
; @run 6B:8882
; WHAT IT DOES: Builds the stage's VRAM copy buffer ($3D00, 256 bytes) from a compressed list:
; clears it, then fills the runs the list for the current stage section (zField82) holds.
; HOW: A TAI of zeros clears the buffer; the list's address comes from the overlay's table,
; two bytes per section. Each run gives its first buffer index, then its bytes with the last
; one marked by bit 7; an $FF byte ends the list.
; The same code as ExpandStageVramRuns; this overlay's table of run lists is at $9EA6.
; WAITS: no.
; Entry: zField82 the stage section. Exit: A $FF, X the buffer index reached, Y the list
; position, N 0, Z 1, C 1. Clobbers zTemp00-02.
ExpandStageVramRunsV:
    tai KernelZeros,StageVramBuffer,$00FF
    lda z:zField82
    asl a
    tay
    lda a:$9EA6,y
    sta z:zTemp00
    lda a:$9EA7,y
    sta z:zTemp01
    cly
ExpandStageVramRunsV_at1:
    lda (zTemp00),y
    tax
    iny
ExpandStageVramRunsV_at2:
    lda (zTemp00),y
    iny
    cmp #$FF
    bne ExpandStageVramRunsV_at0
    rts
ExpandStageVramRunsV_at0:
    asl a
    rol z:zTemp02
    lsr a
    sta a:StageVramBuffer,x
    inx
    bbs0 zTemp02,ExpandStageVramRunsV_at1
    bra ExpandStageVramRunsV_at2
