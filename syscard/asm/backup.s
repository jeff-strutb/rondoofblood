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
; Entry: nothing. Exit: A kept, N and Z from it, I set. X, Y, C, V kept.
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
; Entry: nothing. Exit: A kept, N and Z from it, I clear. X, Y, C, V kept.
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
