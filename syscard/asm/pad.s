; @card 00:E49A ex_joysns
; WHAT IT DOES: Reads the five pads, and works out which buttons each has newly pressed. Run and select
; together on pad 1 (or any pad the mask asks for) is a soft reset.
; HOW: Per pad, the port is written to select the high nibble, read (shifted up, with the pad's previous buttons kept),
; written to select the low nibble, read, and the buttons made active high; what is new is what was not held before. Then the
; pads in the mask are tested for run and select newly pressed together.
; WAITS: no.
; Entry: the mask of the pads asked for, at $2227. Exit: A the last value it loaded, Y 5, N 0, Z 1, C 1. X, V kept. Soft reset: the
; reset mark set to $80 and a jump through CardSoftResetLo.
; The pad port reads are by their address, with the stack's push and pull the original makes to wait on the port.
CardPadPort = $1000
CardPadNow = $2228
CardPadBefore = $2232
CardPadAsked = $2227
CardPadBits = $E504
ex_joysns:
    lda #$01
    sta a:CardPadPort
    lda #$03
    sta a:CardPadPort
    cly
ex_joysns_at0:
    lda #$01
    sta a:CardPadPort
    pha
    pla
    nop
    nop
    lda a:CardPadNow,y
    sta a:CardPadBefore,y
    lda a:CardPadPort
    asl a
    asl a
    asl a
    asl a
    sta a:CardPadNow,y
    stz a:CardPadPort
    pha
    pla
    nop
    nop
    lda a:CardPadPort
    and #$0F
    ora a:CardPadNow,y
    eor #$FF
    sta a:CardPadNow,y
    eor a:CardPadBefore,y
    and a:CardPadNow,y
    sta a:CardPadPressed,y
    iny
    cpy #$05
    bcc ex_joysns_at0
    cly
ex_joysns_at1:
    lda a:CardPadAsked
    and a:CardPadBits,y
    beq ex_joysns_at2
    lda a:CardPadPressed,y
    cmp #$04
    bne ex_joysns_at2
    lda a:CardPadNow,y
    cmp #$0C
    bne ex_joysns_at2
    lda #$80
    sta z:CardResetMark
    jmp (CardSoftResetLo)
ex_joysns_at2:
    iny
    cpy #$05
    bcc ex_joysns_at1
    rts
