; @routine 024F:113 MaskStageRowCopy
; @run 6B:9113
; WHAT IT DOES: Masks one row of the stage's mask buffer from the cells two on: each of the
; row's eight cells takes the cell two bytes after it, ANDed with the mask in zTemp00.
; HOW: The row's base offset is added to each of the eight offsets of StageMaskOffsets, from
; the last to the first. X and Y go on the stack and come back.
; WAITS: no.
; Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: A the last
; cell written, N and Z from X, C and V from the last offset add. X and Y kept.
MaskStageRowCopy:
    stx z:zTemp08
    phx
    phy
    ldx #$07
MaskStageRowCopy_at0:
    lda a:StageMaskOffsets,x
    clc
    adc z:zTemp08
    tay
    lda a:StageMaskBuffer+2,y
    and z:zTemp00
    sta a:StageMaskBuffer,y
    dex
    bpl MaskStageRowCopy_at0
    ply
    plx
    rts

; @routine 024F:12E MaskStageRow
; @run 6B:912E
; WHAT IT DOES: Masks one row of the stage's mask buffer in place: each of the row's eight
; cells is ANDed with the mask in zTemp00.
; HOW: As MaskStageRowCopy, from the cell itself.
; WAITS: no.
; Entry: X the row's base offset, zTemp00 the mask, Y kept for the caller. Exit: as
; MaskStageRowCopy.
MaskStageRow:
    stx z:zTemp08
    phx
    phy
    ldx #$07
MaskStageRow_at0:
    lda a:StageMaskOffsets,x
    clc
    adc z:zTemp08
    tay
    lda a:StageMaskBuffer,y
    and z:zTemp00
    sta a:StageMaskBuffer,y
    dex
    bpl MaskStageRow_at0
    ply
    plx
    rts
