; @routine 00C3:5D2 NameTypeCharacter
; @run 6B:95D2
; WHAT IT DOES: Types the character the cursor is on: it is drawn at the next place, put in the
; buffer, the count goes up by one, and unless the name is full the cursor tile follows it.
; HOW: The character is read from the grid table at the row (OBJ->f3618 of object 2) times 8
; plus the column (OBJ->f34b8 of object 2), with the carry of the row's third shift added in as
; the original does; written to the VDC as a tile with palette 1 and to the buffer at the count.
; The count rises; at 8 the routine stops, otherwise it runs on into ShowNameCursor.
; WAITS: no.
; Entry: nothing. Exit: name full: A 8, Z and C set, N clear, Y the count before, V from the
; grid index add; else as ShowNameCursor.
NameTypeCharacter:
    jsr VecSelectVramWriteAddress
    lda a:$7DE8
    clc
    adc #$D0
    sta a:VDC_DATA_LO
    st2 #$00
    jsr VecSelectVramData
    lda a:OBJ_3618+2
NameTypeCharacter_at0:
    asl a
    asl a
    asl a
    adc a:OBJ_34B8+2
    tay
    lda a:$9645,y
    sta a:VDC_DATA_LO
    ldy a:$7DE8
    sta a:$7DE9,y
    st2 #$11
    inc a:$7DE8
    lda a:$7DE8
    cmp #$08
    beq ShowNameCursor_at1

; @routine 00C3:605 ShowNameCursor
; @run 6B:9605
; WHAT IT DOES: Draws the blank tile where the next character of the name will go, and keeps a
; space in the name buffer there.
; HOW: Sets the VDC's write address to the tile row's place for the typed count, writes the
; blank tile, and stores $20 in the buffer at the count.
; WAITS: no.
; Entry: nothing. Exit: A $20, N and Z clear, C and V from the add of the place, Y the count; X
; kept.
ShowNameCursor:
    jsr VecSelectVramWriteAddress
    lda a:$7DE8
    clc
    adc #$D0
    sta a:VDC_DATA_LO
    st2 #$00
    jsr VecSelectVramData
    st1 #$FF
    st2 #$10
    ldy a:$7DE8
    lda #$20
ShowNameCursor_at0:
    sta a:$7DE9,y
ShowNameCursor_at1:
    rts

; @routine 00C3:623 NameBackspace
; @run 6B:9623
; WHAT IT DOES: Takes back the last character of the name: its tile is redrawn as the erased
; tile, the count goes down by one, and the cursor tile goes where the character was.
; HOW: Sets the VDC's write address to the last tile's place, writes the erased tile ($113D),
; subtracts one from the count, stores a space at the new count and goes to ShowNameCursor.
; WAITS: no.
; Entry: the count at least 1. Exit: as ShowNameCursor.
NameBackspace:
    jsr VecSelectVramWriteAddress
    lda a:$7DE8
    clc
    adc #$D0
    sta a:VDC_DATA_LO
    st2 #$00
    jsr VecSelectVramData
    st1 #$3D
    st2 #$11
    dec a:$7DE8
    ldy a:$7DE8
    lda #$20
    sta a:$7DE9,y
    bra ShowNameCursor

; @routine 00C3:665 NameCursorTarget
; @run 6B:9665
; WHAT IT DOES: Sends the name cursor object (and its pair, the next object) to the place of the
; grid character it is on: the last row, the "end" key, is wide, from $60 to $A0.
; HOW: The object's row (OBJ->f3618) picks the Y place from a table, written to both objects;
; on row 4 the X places are $60 and $A0, otherwise the column (OBJ->f34b8) picks a place from a
; table for both; both are marked as moving.
; WAITS: no.
; Entry: X the object. Exit: A 1, N and Z clear, Y the row or the column; X kept; C as the last
; compare left it.
NameCursorTarget:
    ldy a:OBJ_3618,x
    lda a:$969E,y
    sta a:OBJ_35C0,x
    sta a:OBJ_35C0+1,x
    cpy #$04
    bne NameCursorTarget_at1
    lda #$60
    sta a:OBJ_3568,x
    lda #$A0
    sta a:OBJ_3568+1,x
    bra NameCursorTarget_at0
NameCursorTarget_at1:
    ldy a:OBJ_34B8,x
    lda a:$9696,y
    sta a:OBJ_3568,x
    sta a:OBJ_3568+1,x
NameCursorTarget_at0:
    lda #$01
    sta a:OBJ_3510,x
    sta a:OBJ_3510+1,x
    rts
