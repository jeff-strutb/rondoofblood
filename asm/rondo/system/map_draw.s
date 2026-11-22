; @routine 024F:392 AndVramData
; @run 6B:9392
; WHAT IT DOES: Clears bits of the VRAM word under the VDC's address: the word the caller
; has selected for reading and writing at once is ANDed with a mask in place.
; HOW: Each byte of the data port is read, ANDed with its mask byte, and written back, low
; byte first, then high byte. The caller has selected the port's register.
; WAITS: no.
; Entry: X low mask, Y high mask. Exit: A the high byte written, N and Z from it. X and Y kept.
AndVramData:
    txa
    and a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    tya
    and a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    rts

; @routine 024F:3A1 OrVramData
; @run 6B:93A1
; WHAT IT DOES: Sets bits of the VRAM word under the VDC's address: the word the caller
; has selected for reading and writing at once is ORed with a mask in place.
; HOW: As AndVramData, with OR.
; WAITS: no.
; Entry: X low mask, Y high mask. Exit: A the high byte written, N and Z from it. X and Y kept.
OrVramData:
    txa
    ora a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    tya
    ora a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    rts
