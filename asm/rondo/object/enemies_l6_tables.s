; @routine 051A:190 JumpByStateTable051A190
; @run 70:A990
; WHAT IT DOES: Goes to the code a table at $A9A2 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable051A190:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$A9A2,y
    sta z:zTemp00
    lda a:$A9A3,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 072B:1B1 JumpByStateTable072B1B1
; @run 70:B1B1
; WHAT IT DOES: Goes to the code a table at $B1C3 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable072B1B1:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$B1C3,y
    sta z:zTemp00
    lda a:$B1C4,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 072B:24C JumpByStateTable072B24C
; @run 70:B24C
; WHAT IT DOES: Goes to the code a table at $B25E names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable072B24C:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:SoundDrvStopAll+4,y
    sta z:zTemp00
    lda a:SoundDrvStopAll+5,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 072C:004 JumpByPhaseTable072C004
; @run 70:B804
; WHAT IT DOES: Goes to the code a table at $B816 names for the object's OBJ_2800.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
JumpByPhaseTable072C004:
    lda a:OBJ_2800,x
    asl a
    tay
    lda a:$B816,y
    sta z:zTemp00
    lda a:$B817,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 072C:763 JumpByStateTable072C763
; @run 70:BF63
; WHAT IT DOES: Goes to the code a table at $BF75 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable072C763:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$BF75,y
    sta z:zTemp00
    lda a:$BF76,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093C:3FF JumpByStateTable093C3FF
; @run 70:BBFF
; WHAT IT DOES: Goes to the code a table at $BC11 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable093C3FF:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$BC11,y
    sta z:zTemp00
    lda a:$BC12,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093C:4E5 JumpByStateTable093C4E5
; @run 70:BCE5
; WHAT IT DOES: Goes to the code a table at $BCF7 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable093C4E5:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$BCF7,y
    sta z:zTemp00
    lda a:$BCF8,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 09EB:7D3 JumpByStateTable09EB7D3
; @run 70:B7D3
; WHAT IT DOES: Goes to the code a table at $B7E5 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable09EB7D3:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$B7E5,y
    sta z:zTemp00
    lda a:$B7E6,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 09EC:18D JumpByStateTable09EC18D
; @run 70:B98D
; WHAT IT DOES: Goes to the code a table at $B99F names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable09EC18D:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$B99F,y
    sta z:zTemp00
    lda a:$B9A0,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFC:12B JumpByStateTable0BFC12B
; @run 70:B92B
; WHAT IT DOES: Goes to the code a table at $B93D names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0BFC12B:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$B93D,y
    sta z:zTemp00
    lda a:$B93E,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAB:12F JumpByStateTable0CAB12F
; @run 70:B12F
; WHAT IT DOES: Goes to the code a table at $B141 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0CAB12F:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$B141,y
    sta z:zTemp00
    lda a:$B142,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAB:4D7 JumpByPhaseTable0CAB4D7
; @run 70:B4D7
; WHAT IT DOES: Goes to the code a table at $B4E9 names for the object's OBJ_2800.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_2800.
JumpByPhaseTable0CAB4D7:
    lda a:OBJ_2800,x
    asl a
    tay
    lda a:$B4E9,y
    sta z:zTemp00
    lda a:$B4EA,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAC:47A JumpByStateTable0CAC47A
; @run 70:BC7A
; WHAT IT DOES: Goes to the code a table at $BC8C names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0CAC47A:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$BC8C,y
    sta z:zTemp00
    lda a:$BC8D,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAC:59D JumpByStateTable0CAC59D
; @run 70:BD9D
; WHAT IT DOES: Goes to the code a table at $BDAF names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0CAC59D:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$BDAF,y
    sta z:zTemp00
    lda a:$BDB0,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAC:6C3 JumpByStateTable0CAC6C3
; @run 70:BEC3
; WHAT IT DOES: Goes to the code a table at $BED5 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0CAC6C3:
    lda a:OBJ_27A8,x
    asl a
    tay
    lda a:$BED5,y
    sta z:zTemp00
    lda a:$BED6,y
    sta z:zTemp01
    jmp (ZeroPage)
