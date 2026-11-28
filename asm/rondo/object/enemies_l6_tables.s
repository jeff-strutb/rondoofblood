; @routine 025B:545 JumpByStateTable025B545
; @run 70:B545
; WHAT IT DOES: Goes to the code a table at $B557 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable025B545:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$B557,y
    sta z:zTemp00
    lda a:$B558,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 025C:068 JumpByPhaseTable025C068
; @run 70:B868
; WHAT IT DOES: Goes to the code a table at $B87A names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable025C068:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B87A,y
    sta z:zTemp00
    lda a:$B87B,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 025C:40D JumpByPhaseTable025C40D
; @run 70:BC0D
; WHAT IT DOES: Goes to the code a table at $BC1F names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable025C40D:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:AnimationLists+4,y
    sta z:zTemp00
    lda a:AnimationLists+5,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 025C:51C JumpByStateTable025C51C
; @run 70:BD1C
; WHAT IT DOES: Goes to the code a table at $BD2E names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable025C51C:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$BD2E,y
    sta z:zTemp00
    lda a:$BD2F,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051A:190 JumpByStateTable051A190
; @run 70:A990
; WHAT IT DOES: Goes to the code a table at $A9A2 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable051A190:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$A9A2,y
    sta z:zTemp00
    lda a:$A9A3,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051B:225 JumpByStateTable051B225
; @run 70:B225
; WHAT IT DOES: Goes to the code a table at $B237 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable051B225:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SoundDrvInit+4,y
    sta z:zTemp00
    lda a:SoundDrvInit+5,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051B:350 JumpByPhaseTable051B350
; @run 70:B350
; WHAT IT DOES: Goes to the code a table at $B362 names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable051B350:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B362,y
    sta z:zTemp00
    lda a:$B363,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 051B:5B5 JumpByPhaseTable051B5B5
; @run 70:B5B5
; WHAT IT DOES: Goes to the code a table at $B5C7 names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable051B5B5:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B5C7,y
    sta z:zTemp00
    lda a:$B5C8,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 05CB:1CF JumpByStateTable05CB1CF
; @run 70:B1CF
; WHAT IT DOES: Goes to the code a table at $B1E1 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable05CB1CF:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$B1E1,y
    sta z:zTemp00
    lda a:$B1E2,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 05CB:212 JumpByPhaseTable05CB212
; @run 70:B212
; WHAT IT DOES: Goes to the code a table at $B224 names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable05CB212:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B224,y
    sta z:zTemp00
    lda a:$B225,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 05CB:4B8 JumpByPhaseTable05CB4B8
; @run 70:B4B8
; WHAT IT DOES: Goes to the code a table at $B4CA names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable05CB4B8:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B4CA,y
    sta z:zTemp00
    lda a:$B4CB,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 05CC:0E9 JumpByStateTable05CC0E9
; @run 70:B8E9
; WHAT IT DOES: Goes to the code a table at $B8FB names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable05CC0E9:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SoundMusicTable+1,y
    sta z:zTemp00
    lda a:SoundMusicTable+2,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 072B:1B1 JumpByStateTable072B1B1
; @run 70:B1B1
; WHAT IT DOES: Goes to the code a table at $B1C3 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable072B1B1:
    lda a:OBJ_STATE,x
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
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:SoundDrvStopAll+4,y
    sta z:zTemp00
    lda a:SoundDrvStopAll+5,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 072C:004 JumpByPhaseTable072C004
; @run 70:B804
; WHAT IT DOES: Goes to the code a table at $B816 names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable072C004:
    lda a:OBJ_SUBSTATE,x
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
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$BF75,y
    sta z:zTemp00
    lda a:$BF76,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088A:304 JumpByStateTable088A304
; @run 70:AB04
; WHAT IT DOES: Goes to the code a table at $AB16 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable088A304:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$AB16,y
    sta z:zTemp00
    lda a:$AB17,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088A:4E4 JumpByStateTable088A4E4
; @run 70:ACE4
; WHAT IT DOES: Goes to the code a table at $ACF6 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable088A4E4:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$ACF6,y
    sta z:zTemp00
    lda a:$ACF7,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088B:06C JumpByPhaseTable088B06C
; @run 70:B06C
; WHAT IT DOES: Goes to the code a table at $B07E names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable088B06C:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B07E,y
    sta z:zTemp00
    lda a:$B07F,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 093C:3FF JumpByStateTable093C3FF
; @run 70:BBFF
; WHAT IT DOES: Goes to the code a table at $BC11 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable093C3FF:
    lda a:OBJ_STATE,x
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
    lda a:OBJ_STATE,x
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
    lda a:OBJ_STATE,x
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
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$B99F,y
    sta z:zTemp00
    lda a:$B9A0,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4A:4DC JumpByStateTable0B4A4DC
; @run 70:ACDC
; WHAT IT DOES: Goes to the code a table at $ACEE names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0B4A4DC:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$ACEE,y
    sta z:zTemp00
    lda a:$ACEF,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4A:573 JumpByStateTable0B4A573
; @run 70:AD73
; WHAT IT DOES: Goes to the code a table at $AD85 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0B4A573:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$AD85,y
    sta z:zTemp00
    lda a:$AD86,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4B:64C JumpByStateTable0B4B64C
; @run 70:B64C
; WHAT IT DOES: Goes to the code a table at $B65E names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0B4B64C:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$B65E,y
    sta z:zTemp00
    lda a:$B65F,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4C:0D7 JumpByPhaseTable0B4C0D7
; @run 70:B8D7
; WHAT IT DOES: Goes to the code a table at $B8E9 names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable0B4C0D7:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B8E9,y
    sta z:zTemp00
    lda a:$B8EA,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4C:42F JumpByPhaseTable0B4C42F
; @run 70:BC2F
; WHAT IT DOES: Goes to the code a table at $BC41 names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable0B4C42F:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$BC41,y
    sta z:zTemp00
    lda a:$BC42,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4D:08F JumpByStateTable0B4D08F
; @run 70:C08F
; WHAT IT DOES: Goes to the code a table at $C0A1 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0B4D08F:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$C0A1,y
    sta z:zTemp00
    lda a:$C0A2,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0BFC:12B JumpByStateTable0BFC12B
; @run 70:B92B
; WHAT IT DOES: Goes to the code a table at $B93D names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0BFC12B:
    lda a:OBJ_STATE,x
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
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$B141,y
    sta z:zTemp00
    lda a:$B142,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAB:4D7 JumpByPhaseTable0CAB4D7
; @run 70:B4D7
; WHAT IT DOES: Goes to the code a table at $B4E9 names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable0CAB4D7:
    lda a:OBJ_SUBSTATE,x
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
    lda a:OBJ_STATE,x
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
    lda a:OBJ_STATE,x
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
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$BED5,y
    sta z:zTemp00
    lda a:$BED6,y
    sta z:zTemp01
    jmp (ZeroPage)
; @routine 0E0B:16C JumpByPhaseTable0E0B16C
; @run 70:B16C
; WHAT IT DOES: Goes to the code a table at $B17E names for the object's OBJ_SUBSTATE.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled OBJ_SUBSTATE.
JumpByPhaseTable0E0B16C:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:$B17E,y
    sta z:zTemp00
    lda a:$B17F,y
    sta z:zTemp01
    jmp (ZeroPage)
; @routine 0F6B:3EC JumpByStateTable0F6B3EC
; @run 70:B3EC
; WHAT IT DOES: Goes to the code a table at $B3FE names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0F6B3EC:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$B3FE,y
    sta z:zTemp00
    lda a:$B3FF,y
    sta z:zTemp01
    jmp (ZeroPage)
; @routine 0F6C:3C0 JumpByStateTable0F6C3C0
; @run 70:BBC0
; WHAT IT DOES: Goes to the code a table at $BBD2 names for the object's state.
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
JumpByStateTable0F6C3C0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:$BBD2,y
    sta z:zTemp00
    lda a:$BBD3,y
    sta z:zTemp01
    jmp (ZeroPage)
