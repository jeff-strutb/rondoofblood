; @card 00:F858 bm_format
; WHAT IT DOES: Formats the backup memory, given the password that guards it.
; HOW: Compares eleven bytes with the password; measures how much RAM answers; writes the
; header: signature, end, next free byte, and no files.
; Entry: _AX the password's address. Exit, wrong password: A 1, N 0, Z 0, Y where it differed,
; C from that compare. Exit: A 0, N 0, Z 1, X $FF, Y the bytes short of a page. V kept.
bm_format:
    ldy #$0A
bm_format_at0:
    lda a:BackupFormatPassword,y
    cmp (CardAl),y
    bne bm_format_at4
    dey
    bpl bm_format_at0
    jsr bm_open
    lda #$80
    sta z:CardPointerHi
    stz z:CardPointerLo
    ldx #$20
bm_format_at1:
    cly
    cla
bm_format_at2:
    sta (CardPointerLo),y
    cmp (CardPointerLo),y
    bne bm_format_at3
    iny
    bne bm_format_at2
    inc z:CardPointerHi
    dex
    bne bm_format_at1
bm_format_at3:
    sty a:BackupEnd
    lda z:CardPointerHi
    sta a:BackupEnd+1
    ldx #$03
bm_format_at5:
    lda a:BackupSignature,x
    sta a:BackupHeader,x
    dex
    bpl bm_format_at5
    lda #$10
    sta a:BackupNextFree
    lda #$80
    sta a:BackupNextFree+1
    stz a:BackupFirstFile
    stz a:BackupFirstFile+1
    cla
    jmp bm_close
bm_format_at4:
    lda #$01
    rts

; @card 00:F8B8 bm_free
; WHAT IT DOES: Gives how many bytes of the backup memory are free for another file.
; HOW: Its end less its next free byte, less a file's header of $12 bytes, and zero if that
; is below zero. The answer goes in _CX.
; Entry: nothing. Exit: A 0, N 0, Z 1, X $FF, C and V from the subtract's high byte; or $FF as
; bm_open_formatted if not formatted. Y kept.
bm_free:
    jsr bm_open_formatted
    lda a:BackupEnd
    sec
    sbc a:BackupNextFree
    sta z:CardCl
    lda a:BackupEnd+1
    sbc a:BackupNextFree+1
    sta z:CardCh
    lda z:CardCl
    sec
    sbc #$12
    sta z:CardCl
    lda z:CardCh
    sbc #$00
    sta z:CardCh
    bpl bm_free_at0
    stz z:CardCl
    stz z:CardCh
bm_free_at0:
    cla
    jmp bm_close

; @card 00:FB96 bm_find_file
; WHAT IT DOES: Looks through the backup memory's files for the name at _AX.
; HOW: Each file starts with its length; a length of zero ends the list. The name is compared
; through the window, and the file reached is left in CardFileLo.
; Entry: _AX the name's address, backup memory open. Exit, found: C clear, A the file's high
; byte, N and Z from it, Y $0C, V from taking the name's offset off again. Exit, not found: C
; set, A 0, N 0, Z 1, Y 1, V from the last step, or kept if there were no files. X the saved MPR.
bm_find_file:
    lda z:CardAl
    sta z:CardCopyLo
    lda z:CardAh
    sta z:CardCopyHi
    jsr bm_map_name_window
    lda #$10
    sta z:CardFileLo
    lda #$80
    sta z:CardFileHi
bm_find_file_at0:
    ldy #$01
    lda (CardFileLo)
    ora (CardFileLo),y
    beq bm_find_file_at3
    lda z:CardFileLo
    clc
    adc #$04
    sta z:CardFileLo
    lda z:CardFileHi
    adc #$00
    sta z:CardFileHi
    cly
bm_find_file_at1:
    lda (CardCopyLo),y
    cmp (CardFileLo),y
    bne bm_find_file_at2
    iny
    cpy #$0C
    bcc bm_find_file_at1
    lda z:CardFileLo
    sec
    sbc #$04
    sta z:CardFileLo
    lda z:CardFileHi
    sbc #$00
    sta z:CardFileHi
    clc
    rts
bm_find_file_at2:
    lda z:CardFileLo
    sec
    sbc #$04
    sta z:CardFileLo
    lda z:CardFileHi
    sbc #$00
    sta z:CardFileHi
    ldy #$01
    lda (CardFileLo),y
    pha
    lda (CardFileLo)
    clc
    adc z:CardFileLo
    sta z:CardFileLo
    pla
    adc z:CardFileHi
    sta z:CardFileHi
    jmp bm_find_file_at0
bm_find_file_at3:
    sec
    rts

; @card 00:FC32 bm_open_formatted
; WHAT IT DOES: Opens the backup memory if it has been formatted; if not, closes it and has
; its caller return $FF.
; HOW: Compares the header's first four bytes with the signature, from the last.
; Entry: nothing. Exit, formatted: A the first byte, N 1, Z 0, C 1, X $FF. Exit, not: its
; caller's caller is next, A $FF, N 1, Z 0, X where it differed, C from that compare.
bm_open_formatted:
    bsr bm_open
    ldx #$03
bm_open_formatted_at0:
    lda a:BackupHeader,x
    cmp a:BackupSignature,x
    bne bm_open_formatted_at1
    dex
    bpl bm_open_formatted_at0
    rts
bm_open_formatted_at1:
    bsr bm_close
    pla
    pla
    lda #$FF
    rts

; @card 00:FC49 bm_open
; WHAT IT DOES: Opens the backup memory at $8000, keeping every MPR.
; HOW: The eight MPRs go to CardSavedMprs, its bank to MPR4, slow speed, interrupts off, and
; the unlock port.
; Entry: nothing. Exit: A kept, N and Z from it, I set, the CPU on its slow clock. X, Y, C, V kept.
bm_open:
    pha
    tma #$01
    sta a:CardSavedMprs
    tma #$02
    sta a:CardSavedMprs+1
    tma #$04
    sta a:CardSavedMprs+2
    tma #$08
    sta a:CardSavedMprs+3
    tma #$10
    sta a:CardSavedMprs+4
    tma #$20
    sta a:CardSavedMprs+5
    tma #$40
    sta a:CardSavedMprs+6
    tma #$80
    sta a:CardSavedMprs+7
    lda #$F7
    tam #$10
    csl
    sei
    lda #$80
    sta a:BackupUnlock
    pla
    rts

; @card 00:FC7F bm_close
; WHAT IT DOES: Closes the backup memory and puts back every MPR.
; HOW: The eight MPRs back; a read of the status port locks it; full speed; then waits out a
; vertical blank with its interrupt on, so that none was lost, and puts the VDC back.
; Entry: nothing. Exit: A kept, N and Z from it, I clear, the CPU on its fast clock. X, Y, C, V
; kept.
bm_close:
    pha
    lda a:CardSavedMprs
    tam #$01
    lda a:CardSavedMprs+1
    tam #$02
    lda a:CardSavedMprs+2
    tam #$04
    lda a:CardSavedMprs+3
    tam #$08
    lda a:CardSavedMprs+4
    tam #$10
    lda a:CardSavedMprs+5
    tam #$20
    lda a:CardSavedMprs+6
    tam #$40
    lda a:CardSavedMprs+7
    tam #$80
    lda a:CdIrqStatus
    csh
    lda #$05
    sta a:VdcAddress
    lda z:CardVdcCrLo
    ora #$08
    sta a:VdcDataLo
bm_close_at0:
    lda a:VdcAddress
    and #$20
    bne bm_close_at0
bm_close_at1:
    lda a:VdcAddress
    and #$20
    beq bm_close_at1
    lda z:CardVdcCrLo
    sta a:VdcDataLo
    lda z:CardVdcReg
    sta a:VdcAddress
    pla
    cli
    rts

; @card 00:FD7B bm_map_name_window
; WHAT IT DOES: Makes the pointer in CardCopyLo reach what it pointed to before the backup
; memory was opened.
; HOW: Its top three bits choose the saved MPR; the banks there and after it go at $A000, and
; the pointer is rebased there.
; Entry: CardCopyLo-Hi. Exit: A the pointer's new high byte, N and Z from it, X the saved MPR,
; C bit 4 of the old high byte. Y, V kept.
bm_map_name_window:
    lda z:CardCopyHi
    rol a
    rol a
    rol a
    rol a
    and #$07
    tax
    lda a:CardSavedMprs,x
    tam #$20
    lda a:CardSavedMprs+1,x
    tam #$40
    lda z:CardCopyHi
    and #$1F
    ora #$A0
    sta z:CardCopyHi
    rts

; @card 00:FD97 bm_count_down
; WHAT IT DOES: Takes one from the count of bytes left.
; HOW: A 16-bit decrement of CardByteCount.
; Entry: CardByteCount. Exit: A its two bytes ORed, N and Z from that. X, Y, C, V kept.
bm_count_down:
    lda a:CardByteCount
    bne bm_count_down_at0
    dec a:CardByteCount+1
bm_count_down_at0:
    dec a:CardByteCount
    lda a:CardByteCount
    ora a:CardByteCount+1
    rts

; @card 00:F8E3 bm_read
; WHAT IT DOES: Reads _CX bytes at offset _DX of the backup file named by _AX into _BX.
; HOW: Finds the file; refuses an offset at or past its end; sums the file against its
; checksum; cuts the count at the file's end and gives it back in _CX; copies through the
; window. A is 0, 1 for no such file, or 2 for a bad checksum (the bytes are still copied).
; Entry: _AX, _BX, _CX, _DX. Exit: I clear, A the result, N and Z from it; not formatted, as
; bm_open_formatted. No file: X the saved MPR, Y 1, C 1, V as bm_find_file. Offset past the end:
; _CX 0, the same with A 0. Read: bytes copied, X the saved MPR, Y the count's low byte, C bit 4
; of _BH; nothing copied, X as bm_find_file, Y 1, C from the cut or the place; V from the cut or
; the place.
bm_read:
    jsr bm_open_formatted
    jsr bm_find_file
    bcs bm_read_at0
    ldy #$01
    lda z:CardDh
    cmp (CardFileLo),y
    bne bm_read_at1
    lda z:CardDl
    cmp (CardFileLo)
bm_read_at1:
    bcs bm_read_at2
    jsr bm_sum_file
    ldy #$02
    lda (CardFileLo),y
    iny
    clc
    adc z:CardCopyLo
    sta z:CardCopyLo
    lda (CardFileLo),y
    adc z:CardCopyHi
    ora z:CardCopyLo
    pha
    jsr bm_place_span
    bmi bm_read_at3
    lda a:CardByteCount
    sec
    sbc z:CardCopyLo
    sta a:CardByteCount
    lda a:CardByteCount+1
    sbc z:CardCopyHi
    sta a:CardByteCount+1
bm_read_at3:
    lda a:CardByteCount
    sta z:CardCl
    lda a:CardByteCount+1
    sta z:CardCh
    ora a:CardByteCount
    beq bm_read_at4
    lda z:CardBl
    sta z:CardCopyLo
    lda z:CardBh
    sta z:CardCopyHi
    jsr bm_copy_out
bm_read_at4:
    pla
    bne bm_read_at5
    jmp bm_close
bm_read_at2:
    stz z:CardCh
    stz z:CardCl
    cla
    jmp bm_close
bm_read_at0:
    lda #$01
    jmp bm_close
bm_read_at5:
    lda #$02
    jmp bm_close

; @card 00:F955 bm_write
; WHAT IT DOES: Writes _CX bytes from _BX at offset _DX of the backup file named by _AX, making
; the file if there is none, and growing it if the write runs past its end.
; HOW: A file that must grow moves every file after it up by the growth, if the memory has room
; for that and a closing zero length; then the bytes are copied in through the window and the
; file's checksum is made the negative of its sum.
; Entry: _AX, _BX, _CX, _DX. Exit: I clear, A the result, N and Z from it; not formatted, as
; bm_open_formatted. No room for a header: A 1, X as bm_find_file, Y 1, C, V as bm_new_file. No
; room to grow: A 1, X the saved MPR, Y 1, C 0, V from that compare. Written: A 0, X the saved
; MPR, Y 3, C and V from negating the sum's high byte.
bm_write:
    jsr bm_open_formatted
    jsr bm_find_file
    bcc bm_write_at0
    jsr bm_new_file
    bcc bm_write_at0
    jmp bm_write_at1
bm_write_at0:
    jsr bm_place_span
    bpl bm_write_at2
    jmp bm_write_at3
bm_write_at2:
    lda z:CardCopyLo
    sta a:CardByteCount
    lda z:CardCopyHi
    sta a:CardByteCount+1
    lda a:BackupNextFree
    clc
    adc a:CardByteCount
    sta z:CardCopyLo
    lda a:BackupNextFree+1
    adc a:CardByteCount+1
    sta z:CardCopyHi
    lda z:CardCopyLo
    clc
    adc #$02
    sta z:CardPointerLo
    lda z:CardCopyHi
    adc #$00
    sta z:CardPointerHi
    lda a:BackupEnd
    cmp z:CardPointerLo
    lda a:BackupEnd+1
    sbc z:CardPointerHi
    bcc bm_write_at1
    lda z:CardCopyLo
    sta a:BackupNextFree
    lda z:CardCopyHi
    sta a:BackupNextFree+1
    ldy #$01
    lda (CardFileLo)
    clc
    adc z:CardFileLo
    sta z:CardCopyLo
    sta z:CardPointerLo
    lda (CardFileLo),y
    adc z:CardFileHi
    sta z:CardCopyHi
    sta z:CardPointerHi
    ldy #$01
    lda a:CardByteCount
    clc
    adc (CardFileLo)
    sta (CardFileLo)
    lda a:CardByteCount+1
    adc (CardFileLo),y
    sta (CardFileLo),y
    lda a:CardByteCount
    clc
    adc z:CardCopyLo
    sta z:CardCopyLo
    lda a:CardByteCount+1
    adc z:CardCopyHi
    sta z:CardCopyHi
    lda a:BackupNextFree
    sec
    sbc z:CardPointerLo
    sta a:CardByteCount
    lda a:BackupNextFree+1
    sbc z:CardPointerHi
    sta a:CardByteCount+1
    jsr bm_move
    jsr bm_place_span
bm_write_at3:
    lda z:CardBl
    sta z:CardCopyLo
    lda z:CardBh
    sta z:CardCopyHi
    jsr bm_copy_in
    jsr bm_sum_file
    ldy #$02
    cla
    sec
    sbc z:CardCopyLo
    sta (CardFileLo),y
    iny
    cla
    sbc z:CardCopyHi
    sta (CardFileLo),y
    cla
    jmp bm_close
bm_write_at1:
    lda #$01
    jmp bm_close

; @card 00:FA1A bm_delete
; WHAT IT DOES: Deletes the backup file named by _AX.
; HOW: Moves every file after it down over it, takes its length off the next free byte, and
; marks the list's new end.
; Entry: _AX. Exit: I clear, A the result, N and Z from it; not formatted, as bm_open_formatted.
; No file: A 1, X the saved MPR, Y 1, C 1, V as bm_find_file. Deleted: A 0, Y 1; X, C and V as
; bm_move leaves them, or, with nothing after it, X the saved MPR and C and V from taking the
; length off.
bm_delete:
    jsr bm_open_formatted
    jsr bm_find_file
    bcs bm_delete_at0
    lda z:CardFileLo
    sta z:CardCopyLo
    lda z:CardFileHi
    sta z:CardCopyHi
    lda (CardFileLo)
    sta a:CardByteCount
    ldy #$01
    lda (CardFileLo),y
    sta a:CardByteCount+1
    lda z:CardFileLo
    clc
    adc a:CardByteCount
    sta z:CardPointerLo
    lda z:CardFileHi
    adc a:CardByteCount+1
    sta z:CardPointerHi
    lda a:BackupNextFree
    sec
    sbc z:CardPointerLo
    sta a:CardByteCount
    lda a:BackupNextFree+1
    sbc z:CardPointerHi
    sta a:CardByteCount+1
    lda a:BackupNextFree
    sec
    sbc (CardFileLo)
    sta a:BackupNextFree
    lda a:BackupNextFree+1
    sbc (CardFileLo),y
    sta a:BackupNextFree+1
    jsr bm_move
    jmp bm_close
bm_delete_at0:
    lda #$01
    jmp bm_close

; @card 00:FAE2 bm_new_file
; WHAT IT DOES: Starts a new, empty file named by _AX at the backup memory's next free byte.
; HOW: Works out the room left into CardPointerLo, but tests CardCopyLo-Hi against $10 instead,
; which the name's pointer always passes. Then copies the twelve bytes of the name through the
; window, moves the next free byte on by a header, and gives the file a length of $10.
; Entry: _AX the name, CardCopyLo-Hi. Exit, made: C clear, A 0, N 0, Z 1, Y 1, X the saved MPR, V
; from moving the next free byte on. Exit, refused: C set, A the high byte of the test, N and Z
; from it, V from it. X and Y kept.
bm_new_file:
    lda a:BackupNextFree
    sta z:CardFileLo
    lda a:BackupNextFree+1
    sta z:CardFileHi
    lda a:BackupEnd
    sec
    sbc z:CardFileLo
    sta z:CardPointerLo
    lda a:BackupEnd+1
    sbc z:CardFileHi
    sta z:CardPointerHi
    lda z:CardCopyLo
    cmp #$10
    lda z:CardCopyHi
    sbc #$00
    bcc bm_new_file_at0
    lda z:CardFileLo
    clc
    adc #$04
    sta z:CardPointerLo
    lda z:CardFileHi
    adc #$00
    sta z:CardPointerHi
    lda z:CardAl
    sta z:CardCopyLo
    lda z:CardAh
    sta z:CardCopyHi
    jsr bm_map_name_window
    cly
bm_new_file_at1:
    lda (CardCopyLo),y
    sta (CardPointerLo),y
    iny
    cpy #$0C
    bne bm_new_file_at1
    lda a:BackupNextFree
    clc
    adc #$10
    sta a:BackupNextFree
    lda a:BackupNextFree+1
    adc #$00
    sta a:BackupNextFree+1
    ldy #$01
    lda #$10
    sta (CardFileLo)
    lda #$00
    sta (CardFileLo),y
    clc
    rts
bm_new_file_at0:
    sec
    rts

; @card 00:FB46 bm_place_span
; WHAT IT DOES: Places a read or write of _CX bytes at offset _DX in the file at CardFileLo.
; HOW: How far it runs past the file's end goes to CardCopyLo (negative when it fits), the
; address in the file it starts at to CardPointerLo, and _CX to CardByteCount.
; Entry: _CX, _DX, CardFileLo. Exit: A the high byte of the overrun, N and Z from it, Y 1, C and V
; from adding _DX to the file's data address. X kept.
bm_place_span:
    lda z:CardDl
    clc
    adc z:CardCl
    sta z:CardCopyLo
    lda z:CardDh
    adc z:CardCh
    sta z:CardCopyHi
    lda z:CardCopyLo
    clc
    adc #$10
    sta z:CardCopyLo
    lda z:CardCopyHi
    adc #$00
    sta z:CardCopyHi
    ldy #$01
    lda z:CardCopyLo
    sec
    sbc (CardFileLo)
    sta z:CardCopyLo
    lda z:CardCopyHi
    sbc (CardFileLo),y
    sta z:CardCopyHi
    lda z:CardFileLo
    clc
    adc #$10
    sta z:CardPointerLo
    lda z:CardFileHi
    adc #$00
    sta z:CardPointerHi
    lda z:CardDl
    clc
    adc z:CardPointerLo
    sta z:CardPointerLo
    lda z:CardDh
    adc z:CardPointerHi
    sta z:CardPointerHi
    lda z:CardCl
    sta a:CardByteCount
    lda z:CardCh
    sta a:CardByteCount+1
    lda z:CardCopyHi
    rts

; @card 00:FBFC bm_sum_file
; WHAT IT DOES: Sums the name and data of the file at CardFileLo, for its checksum.
; HOW: Adds every byte after the length and checksum words into a 16-bit sum in CardCopyLo,
; counting the file's length less four down in CardByteCount. A length of four sums $10000
; bytes.
; Entry: CardFileLo. Exit: A 0, N 0, Z 1, Y the low byte of four plus the bytes summed, C and V
; from the last add's high byte. X kept. CardPointerLo-Hi the file, its high byte moved on by
; the pages crossed.
bm_sum_file:
    lda z:CardFileLo
    sta z:CardPointerLo
    lda z:CardFileHi
    sta z:CardPointerHi
    ldy #$01
    lda (CardPointerLo)
    sec
    sbc #$04
    sta a:CardByteCount
    lda (CardPointerLo),y
    sbc #$00
    sta a:CardByteCount+1
    stz z:CardCopyLo
    stz z:CardCopyHi
    ldy #$04
bm_sum_file_at1:
    lda (CardPointerLo),y
    clc
    adc z:CardCopyLo
    sta z:CardCopyLo
    cla
    adc z:CardCopyHi
    sta z:CardCopyHi
    iny
    bne bm_sum_file_at0
    inc z:CardPointerHi
bm_sum_file_at0:
    jsr bm_count_down
    bne bm_sum_file_at1
    rts

; @card 00:FCD3 bm_move
; WHAT IT DOES: Moves CardByteCount bytes inside the backup memory, then marks the end of the
; file list at its next free byte.
; HOW: bm_move_direction picks the way; bytes go one at a time, each pointer moved a page at a
; time; then a zero length is written at the next free byte.
; Entry: CardPointerLo the source, CardCopyLo the destination, CardByteCount. Exit: A 0, Y 1, N
; and Z from loading Y (CLA leaves them), CardPointerLo the next free byte. Nothing to
; move: X, C, V kept. Else X, C and V as bm_move_direction leaves them.
bm_move:
    lda a:CardByteCount
    ora a:CardByteCount+1
    beq bm_move_at0
    jsr bm_move_direction
    cly
    txa
    bne bm_move_at1
bm_move_at3:
    lda (CardPointerLo),y
    sta (CardCopyLo),y
    tya
    bne bm_move_at2
    dec z:CardPointerHi
    dec z:CardCopyHi
bm_move_at2:
    dey
    jsr bm_count_down
    bne bm_move_at3
    beq bm_move_at0
bm_move_at1:
    lda (CardPointerLo),y
    sta (CardCopyLo),y
    iny
    bne bm_move_at4
    inc z:CardPointerHi
    inc z:CardCopyHi
bm_move_at4:
    jsr bm_count_down
    bne bm_move_at1
bm_move_at0:
    lda a:BackupNextFree
    sta z:CardPointerLo
    lda a:BackupNextFree+1
    sta z:CardPointerHi
    ldy #$01
    cla
    sta (CardPointerLo)
    sta (CardPointerLo),y
    rts

; @card 00:FD17 bm_copy_out
; WHAT IT DOES: Copies bytes from the backup memory to the caller's memory.
; HOW: The caller's pointer in CardCopyLo is put through the window, then CardByteCount bytes go
; one at a time from CardPointerLo; a count of zero copies $10000.
; Entry: CardPointerLo, CardCopyLo, CardByteCount. Exit: A 0, N 0, Z 1, X the saved MPR, Y the
; low byte of the count, C bit 4 of the caller's old high byte. V kept.
bm_copy_out:
    jsr bm_map_name_window
    cly
bm_copy_out_at1:
    lda (CardPointerLo),y
    sta (CardCopyLo),y
    iny
    bne bm_copy_out_at0
    inc z:CardPointerHi
    inc z:CardCopyHi
bm_copy_out_at0:
    jsr bm_count_down
    bne bm_copy_out_at1
    rts

; @card 00:FD2C bm_copy_in
; WHAT IT DOES: Copies bytes from the caller's memory into the backup memory.
; HOW: As bm_copy_out, the other way.
; Entry: CardPointerLo, CardCopyLo, CardByteCount. Exit: as bm_copy_out.
bm_copy_in:
    jsr bm_map_name_window
    cly
bm_copy_in_at1:
    lda (CardCopyLo),y
    sta (CardPointerLo),y
    iny
    bne bm_copy_in_at0
    inc z:CardPointerHi
    inc z:CardCopyHi
bm_copy_in_at0:
    jsr bm_count_down
    bne bm_copy_in_at1
    rts

; @card 00:FD41 bm_move_direction
; WHAT IT DOES: Chooses which way to move bytes inside the backup memory, so that a move onto
; itself is not spoiled.
; HOW: A source at or above its destination goes first byte first. One below goes last byte
; first: both pointers are moved on to their last byte.
; Entry: CardPointerLo the source, CardCopyLo the destination, CardByteCount the bytes.
; Exit, forward: X 2, A the high bytes' difference, N 0, Z 0, C 1, V from that subtract.
; Exit, backward: X 0, A the destination's new high byte, N 0, Z 1, C and V from moving it on.
; Y kept.
bm_move_direction:
    lda z:CardPointerLo
    sec
    sbc z:CardCopyLo
    lda z:CardPointerHi
    sbc z:CardCopyHi
    bcs bm_move_direction_at0
    jsr bm_count_down
    lda z:CardPointerLo
    clc
    adc a:CardByteCount
    sta z:CardPointerLo
    lda z:CardPointerHi
    adc a:CardByteCount+1
    sta z:CardPointerHi
    lda z:CardCopyLo
    clc
    adc a:CardByteCount
    sta z:CardCopyLo
    lda z:CardCopyHi
    adc a:CardByteCount+1
    sta z:CardCopyHi
    inc a:CardByteCount
    bne bm_move_direction_at1
    inc a:CardByteCount+1
bm_move_direction_at1:
    ldx #$00
    rts
bm_move_direction_at0:
    ldx #$02
    rts

