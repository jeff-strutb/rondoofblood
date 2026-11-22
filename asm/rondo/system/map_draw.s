; @routine 024E:687 SetStageTileVramAddress
; @run 6B:8E87
; WHAT IT DOES: Points the VDC's VRAM addresses at the tile row the stage's tile step names:
; one of the eight words of StageTileVramAddress, picked by zTemp04 rounded down to even.
; HOW: The word into SetVramAddresses, which this routine jumps to with no frame of its own.
; WAITS: no.
; Entry: zTemp04 the step. Exit: as SetVramAddresses. X and Y the address bytes.
SetStageTileVramAddress:
    lda z:zTemp04
    and #$FE
    tax
    lda a:StageTileVramAddress,x
    ldy a:StageTileVramAddress+1,x
    tax
    jmp SetVramAddresses

; @routine 024F:37D SetVramAddresses
; @run 6B:937D
; WHAT IT DOES: Points both of the VDC's VRAM addresses, the one it reads from and the one it
; writes to, at the same word, and selects the data port, so a word can be read and written
; back in place (AndVramData, OrVramData).
; HOW: Read address register, then X and Y into the data port; the write address register the
; same; then the data port register, which this routine runs on into with no return of its own.
; WAITS: no.
; Entry: X low byte, Y high byte of the VRAM address. Exit: A 2, N 0, Z 0. X and Y kept.
SetVramAddresses:
    jsr VecSelectVramReadAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jmp VecSelectVramData

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

; @routine 05BE:687 SetStageTileVramAddressLong
; @run 6B:8E87
; WHAT IT DOES: SetStageTileVramAddress for the stages whose overlay has its SetVramAddresses at
; $968F.
; HOW: The same code, jumping to SetVramAddressesLong.
; WAITS: no.
; Entry: zTemp04 the step. Exit: as SetVramAddressesLong. X and Y the address bytes.
SetStageTileVramAddressLong:
    lda z:zTemp04
    and #$FE
    tax
    lda a:StageTileVramAddress,x
    ldy a:StageTileVramAddress+1,x
    tax
    jmp SetVramAddressesLong

; @routine 05BF:68F SetVramAddressesLong
; @run 6B:968F
; WHAT IT DOES: SetVramAddresses, as the stages whose overlay runs it at $968F have it.
; HOW: The same code at another run address, so its two calls return to $9692 and $969B.
; WAITS: no.
; Entry: X low byte, Y high byte of the VRAM address. Exit: as SetVramAddresses.
SetVramAddressesLong:
    jsr VecSelectVramReadAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jsr VecSelectVramWriteAddress
    stx a:VDC_DATA_LO
    sty a:VDC_DATA_HI
    jmp VecSelectVramData

; @routine 05BF:6A4 AndVramDataLong
; @run 6B:96A4
; WHAT IT DOES: AndVramData, as the stages whose overlay runs it at $96A4 have it.
; HOW: The same bytes as AndVramData at another run address. Its C is AndVramData.
; WAITS: no.
; Entry: X low mask, Y high mask. Exit: as AndVramData.
AndVramDataLong:
    txa
    and a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    tya
    and a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    rts

; @routine 05BF:6B3 OrVramDataLong
; @run 6B:96B3
; WHAT IT DOES: OrVramData, as the stages whose overlay runs it at $96B3 have it.
; HOW: The same bytes as OrVramData at another run address. Its C is OrVramData.
; WAITS: no.
; Entry: X low mask, Y high mask. Exit: as OrVramData.
OrVramDataLong:
    txa
    ora a:VDC_DATA_LO
    sta a:VDC_DATA_LO
    tya
    ora a:VDC_DATA_HI
    sta a:VDC_DATA_HI
    rts
