; @routine 05CB:680 JumpByPhaseTable05CB680
; @run 70:B680
; WHAT IT DOES: Goes to the code a table at $B692 names for the object's OBJ_SUBSTATE (Secret stage 02's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable05CB680Table = $B692 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable05CB680:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable05CB680Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable05CB680Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088A:37F JumpByStateTable088A37F
; @run 70:AB7F
; WHAT IT DOES: Goes to the code a table at $AB91 names for the object's state (Secret stage 04's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable088A37FTable = $AB91 ; a handler's address per OBJ_STATE value
JumpByStateTable088A37F:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable088A37FTable,y
    sta z:zTemp00
    lda a:JumpByStateTable088A37FTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088A:661 JumpByStateTable088A661
; @run 70:AE61
; WHAT IT DOES: Goes to the code a table at $AE73 names for the object's state (Secret stage 04's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable088A661Table = $AE73 ; a handler's address per OBJ_STATE value
JumpByStateTable088A661:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable088A661Table,y
    sta z:zTemp00
    lda a:JumpByStateTable088A661Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088B:26F JumpByPhaseTable088B26F
; @run 70:B26F
; WHAT IT DOES: Goes to the code a table at $B281 names for the object's OBJ_SUBSTATE (Secret stage 04's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable088B26FTable = $B281 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable088B26F:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable088B26FTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable088B26FTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088B:5B9 JumpByPhaseTable088B5B9
; @run 70:B5B9
; WHAT IT DOES: Goes to the code a table at $B5CB names for the object's OBJ_SUBSTATE (Secret stage 04's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable088B5B9Table = $B5CB ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable088B5B9:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable088B5B9Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable088B5B9Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 088B:7FF JumpByStateTable088B7FF
; @run 70:B7FF
; WHAT IT DOES: Goes to the code a table at $B811 names for the object's state (Secret stage 04's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable088B7FFTable = $B811 ; a handler's address per OBJ_STATE value
JumpByStateTable088B7FF:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable088B7FFTable,y
    sta z:zTemp00
    lda a:JumpByStateTable088B7FFTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 09EB:431 JumpByPhaseTable09EB431
; @run 70:B431
; WHAT IT DOES: Goes to the code a table at $B443 names for the object's OBJ_SUBSTATE (Secret stage 05's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable09EB431Table = $B443 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable09EB431:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable09EB431Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable09EB431Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 09EC:223 JumpByStateTable09EC223
; @run 70:BA23
; WHAT IT DOES: Goes to the code a table at $BA35 names for the object's state (Secret stage 05's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable09EC223Table = $BA35 ; a handler's address per OBJ_STATE value
JumpByStateTable09EC223:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable09EC223Table,y
    sta z:zTemp00
    lda a:JumpByStateTable09EC223Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4C:1DC JumpByPhaseTable0B4C1DC
; @run 70:B9DC
; WHAT IT DOES: Goes to the code a table at $B9EE names for the object's OBJ_SUBSTATE (Secret stage 06's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable0B4C1DCTable = $B9EE ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable0B4C1DC:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable0B4C1DCTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable0B4C1DCTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4D:013 JumpByStateTable0B4D013
; @run 71:C013
; WHAT IT DOES: Goes to the code a table at $C025 names for the object's state (Secret stage 06's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable0B4D013Table = $C025 ; a handler's address per OBJ_STATE value
JumpByStateTable0B4D013:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable0B4D013Table,y
    sta z:zTemp00
    lda a:JumpByStateTable0B4D013Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0B4D:0FF JumpByStateTable0B4D0FF
; @run 71:C0FF
; WHAT IT DOES: Goes to the code a table at $C111 names for the object's state (Secret stage 06's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable0B4D0FFTable = $C111 ; a handler's address per OBJ_STATE value
JumpByStateTable0B4D0FF:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable0B4D0FFTable,y
    sta z:zTemp00
    lda a:JumpByStateTable0B4D0FFTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0CAC:7BE JumpByStateTable0CAC7BE
; @run 70:BFBE
; WHAT IT DOES: Goes to the code a table at $BFD0 names for the object's state (Secret stage 07's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable0CAC7BETable = $BFD0 ; a handler's address per OBJ_STATE value
JumpByStateTable0CAC7BE:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable0CAC7BETable,y
    sta z:zTemp00
    lda a:JumpByStateTable0CAC7BETable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0E0B:389 JumpByPhaseTable0E0B389
; @run 70:B389
; WHAT IT DOES: Goes to the code a table at $B39B names for the object's OBJ_SUBSTATE (Secret stage 08's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable0E0B389Table = $B39B ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable0E0B389:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable0E0B389Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable0E0B389Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0E0C:5C0 JumpByStateTable0E0C5C0
; @run 70:BDC0
; WHAT IT DOES: Goes to the code a table at $BDD2 names for the object's state (Secret stage 08's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable0E0C5C0Table = $BDD2 ; a handler's address per OBJ_STATE value
JumpByStateTable0E0C5C0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable0E0C5C0Table,y
    sta z:zTemp00
    lda a:JumpByStateTable0E0C5C0Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBB:063 JumpByStateTable0EBB063
; @run 70:B063
; WHAT IT DOES: Goes to the code a table at $B075 names for the object's state (Stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable0EBB063Table = $B075 ; a handler's address per OBJ_STATE value
JumpByStateTable0EBB063:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable0EBB063Table,y
    sta z:zTemp00
    lda a:JumpByStateTable0EBB063Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBB:154 JumpByStateTable0EBB154
; @run 70:B154
; WHAT IT DOES: Goes to the code a table at $B166 names for the object's state (Stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable0EBB154Table = $B166 ; a handler's address per OBJ_STATE value
JumpByStateTable0EBB154:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable0EBB154Table,y
    sta z:zTemp00
    lda a:JumpByStateTable0EBB154Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBB:38A JumpByStateTable0EBB38A
; @run 70:B38A
; WHAT IT DOES: Goes to the code a table at $B39C names for the object's state (Stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable0EBB38ATable = $B39C ; a handler's address per OBJ_STATE value
JumpByStateTable0EBB38A:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable0EBB38ATable,y
    sta z:zTemp00
    lda a:JumpByStateTable0EBB38ATable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBD:7F7 JumpByStateTable0EBD7F7
; @run 71:C7F7
; WHAT IT DOES: Goes to the code a table at $C809 names for the object's state (Stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable0EBD7F7Table = $C809 ; a handler's address per OBJ_STATE value
JumpByStateTable0EBD7F7:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable0EBD7F7Table,y
    sta z:zTemp00
    lda a:JumpByStateTable0EBD7F7Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBE:210 JumpByPhaseTable0EBE210
; @run 71:CA10
; WHAT IT DOES: Goes to the code a table at $CA22 names for the object's OBJ_SUBSTATE (Stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable0EBE210Table = $CA22 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable0EBE210:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable0EBE210Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable0EBE210Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBE:49E JumpByPhaseTable0EBE49E
; @run 71:CC9E
; WHAT IT DOES: Goes to the code a table at $CCB0 names for the object's OBJ_SUBSTATE (Stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable0EBE49ETable = $CCB0 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable0EBE49E:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable0EBE49ETable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable0EBE49ETable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBE:6D8 JumpByPhaseTable0EBE6D8
; @run 71:CED8
; WHAT IT DOES: Goes to the code a table at $CEEA names for the object's OBJ_SUBSTATE (Stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable0EBE6D8Table = $CEEA ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable0EBE6D8:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable0EBE6D8Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable0EBE6D8Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EBF:21A JumpByPhaseTable0EBF21A
; @run 71:D21A
; WHAT IT DOES: Goes to the code a table at $D22C names for the object's OBJ_SUBSTATE (Stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable0EBF21ATable = $D22C ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable0EBF21A:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable0EBF21ATable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable0EBF21ATable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0EC0:3A3 JumpByStateTable0EC03A3
; @run 71:DBA3
; WHAT IT DOES: Goes to the code a table at $DBB5 names for the object's state (Stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable0EC03A3Table = $DBB5 ; a handler's address per OBJ_STATE value
JumpByStateTable0EC03A3:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable0EC03A3Table,y
    sta z:zTemp00
    lda a:JumpByStateTable0EC03A3Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 0F6B:633 JumpByPhaseTable0F6B633
; @run 70:B633
; WHAT IT DOES: Goes to the code a table at $B645 names for the object's OBJ_SUBSTATE (Secret stage 09's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable0F6B633Table = $B645 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable0F6B633:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable0F6B633Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable0F6B633Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101B:662 JumpByPhaseTable101B662
; @run 70:B662
; WHAT IT DOES: Goes to the code a table at $B674 names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable101B662Table = $B674 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable101B662:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable101B662Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable101B662Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101B:76D JumpByPhaseTable101B76D
; @run 70:B76D
; WHAT IT DOES: Goes to the code a table at $B77F names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable101B76DTable = $B77F ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable101B76D:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable101B76DTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable101B76DTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101C:381 JumpByPhaseTable101C381
; @run 70:BB81
; WHAT IT DOES: Goes to the code a table at $BB93 names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable101C381Table = $BB93 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable101C381:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable101C381Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable101C381Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101C:52B JumpByStateTable101C52B
; @run 70:BD2B
; WHAT IT DOES: Goes to the code a table at $BD3D names for the object's state (Stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable101C52BTable = $BD3D ; a handler's address per OBJ_STATE value
JumpByStateTable101C52B:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable101C52BTable,y
    sta z:zTemp00
    lda a:JumpByStateTable101C52BTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101D:46A JumpByPhaseTable101D46A
; @run 71:C46A
; WHAT IT DOES: Goes to the code a table at $C47C names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable101D46ATable = $C47C ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable101D46A:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable101D46ATable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable101D46ATable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101E:094 JumpByStateTable101E094
; @run 71:C894
; WHAT IT DOES: Goes to the code a table at $C8A6 names for the object's state (Stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable101E094Table = $C8A6 ; a handler's address per OBJ_STATE value
JumpByStateTable101E094:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable101E094Table,y
    sta z:zTemp00
    lda a:JumpByStateTable101E094Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101E:236 JumpByPhaseTable101E236
; @run 71:CA36
; WHAT IT DOES: Goes to the code a table at $CA48 names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable101E236Table = $CA48 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable101E236:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable101E236Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable101E236Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 101F:675 JumpByStateTable101F675
; @run 71:D675
; WHAT IT DOES: Goes to the code a table at $D687 names for the object's state (Stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable101F675Table = $D687 ; a handler's address per OBJ_STATE value
JumpByStateTable101F675:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable101F675Table,y
    sta z:zTemp00
    lda a:JumpByStateTable101F675Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 10CB:26D JumpByStateTable10CB26D
; @run 70:B26D
; WHAT IT DOES: Goes to the code a table at $B27F names for the object's state (Secret stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable10CB26DTable = $B27F ; a handler's address per OBJ_STATE value
JumpByStateTable10CB26D:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable10CB26DTable,y
    sta z:zTemp00
    lda a:JumpByStateTable10CB26DTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 10CB:2AB JumpByPhaseTable10CB2AB
; @run 70:B2AB
; WHAT IT DOES: Goes to the code a table at $B2BD names for the object's OBJ_SUBSTATE (Secret stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable10CB2ABTable = $B2BD ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable10CB2AB:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable10CB2ABTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable10CB2ABTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 10CB:65E JumpByPhaseTable10CB65E
; @run 70:B65E
; WHAT IT DOES: Goes to the code a table at $B670 names for the object's OBJ_SUBSTATE (Secret stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable10CB65ETable = $B670 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable10CB65E:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable10CB65ETable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable10CB65ETable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 10CC:4E1 JumpByPhaseTable10CC4E1
; @run 70:BCE1
; WHAT IT DOES: Goes to the code a table at $BCF3 names for the object's OBJ_SUBSTATE (Secret stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable10CC4E1Table = $BCF3 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable10CC4E1:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable10CC4E1Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable10CC4E1Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 10CD:019 JumpByStateTable10CD019
; @run 71:C019
; WHAT IT DOES: Goes to the code a table at $C02B names for the object's state (Secret stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable10CD019Table = $C02B ; a handler's address per OBJ_STATE value
JumpByStateTable10CD019:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable10CD019Table,y
    sta z:zTemp00
    lda a:JumpByStateTable10CD019Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 10CD:319 JumpByStateTable10CD319
; @run 71:C319
; WHAT IT DOES: Goes to the code a table at $C32B names for the object's state (Secret stage 0A's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable10CD319Table = $C32B ; a handler's address per OBJ_STATE value
JumpByStateTable10CD319:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable10CD319Table,y
    sta z:zTemp00
    lda a:JumpByStateTable10CD319Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117B:2E6 JumpByStateTable117B2E6
; @run 70:B2E6
; WHAT IT DOES: Goes to the code a table at $B2F8 names for the object's state (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable117B2E6Table = $B2F8 ; a handler's address per OBJ_STATE value
JumpByStateTable117B2E6:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable117B2E6Table,y
    sta z:zTemp00
    lda a:JumpByStateTable117B2E6Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117B:673 JumpByPhaseTable117B673
; @run 70:B673
; WHAT IT DOES: Goes to the code a table at $B685 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable117B673Table = $B685 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable117B673:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable117B673Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable117B673Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117C:7E2 JumpByPhaseTable117C7E2
; @run 70:BFE2
; WHAT IT DOES: Goes to the code a table at $BFF4 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable117C7E2Table = $BFF4 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable117C7E2:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable117C7E2Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable117C7E2Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117D:052 JumpByPhaseTable117D052
; @run 71:C052
; WHAT IT DOES: Goes to the code a table at $C064 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable117D052Table = $C064 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable117D052:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable117D052Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable117D052Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117D:4F7 JumpByStateTable117D4F7
; @run 71:C4F7
; WHAT IT DOES: Goes to the code a table at $C509 names for the object's state (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable117D4F7Table = $C509 ; a handler's address per OBJ_STATE value
JumpByStateTable117D4F7:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable117D4F7Table,y
    sta z:zTemp00
    lda a:JumpByStateTable117D4F7Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117D:5E3 JumpByPhaseTable117D5E3
; @run 71:C5E3
; WHAT IT DOES: Goes to the code a table at $C5F5 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable117D5E3Table = $C5F5 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable117D5E3:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable117D5E3Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable117D5E3Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117D:7EB JumpByStateTable117D7EB
; @run 71:C7EB
; WHAT IT DOES: Goes to the code a table at $C7FD names for the object's state (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable117D7EBTable = $C7FD ; a handler's address per OBJ_STATE value
JumpByStateTable117D7EB:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable117D7EBTable,y
    sta z:zTemp00
    lda a:JumpByStateTable117D7EBTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117E:11D JumpByStateTable117E11D
; @run 71:C91D
; WHAT IT DOES: Goes to the code a table at $C92F names for the object's state (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable117E11DTable = $C92F ; a handler's address per OBJ_STATE value
JumpByStateTable117E11D:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable117E11DTable,y
    sta z:zTemp00
    lda a:JumpByStateTable117E11DTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117F:13A JumpByStateTable117F13A
; @run 71:D13A
; WHAT IT DOES: Goes to the code a table at $D14C names for the object's state (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable117F13ATable = $D14C ; a handler's address per OBJ_STATE value
JumpByStateTable117F13A:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable117F13ATable,y
    sta z:zTemp00
    lda a:JumpByStateTable117F13ATable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117F:278 JumpByStateTable117F278
; @run 71:D278
; WHAT IT DOES: Goes to the code a table at $D28A names for the object's state (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable117F278Table = $D28A ; a handler's address per OBJ_STATE value
JumpByStateTable117F278:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable117F278Table,y
    sta z:zTemp00
    lda a:JumpByStateTable117F278Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117F:3A6 JumpByPhaseTable117F3A6
; @run 71:D3A6
; WHAT IT DOES: Goes to the code a table at $D3B8 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable117F3A6Table = $D3B8 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable117F3A6:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable117F3A6Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable117F3A6Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117F:4BC JumpByPhaseTable117F4BC
; @run 71:D4BC
; WHAT IT DOES: Goes to the code a table at $D4CE names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable117F4BCTable = $D4CE ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable117F4BC:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable117F4BCTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable117F4BCTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117F:67C JumpByPhaseTable117F67C
; @run 71:D67C
; WHAT IT DOES: Goes to the code a table at $D68E names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable117F67CTable = $D68E ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable117F67C:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable117F67CTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable117F67CTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 117F:7C7 JumpByStateTable117F7C7
; @run 71:D7C7
; WHAT IT DOES: Goes to the code a table at $D7D9 names for the object's state (Stage 0B's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable117F7C7Table = $D7D9 ; a handler's address per OBJ_STATE value
JumpByStateTable117F7C7:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable117F7C7Table,y
    sta z:zTemp00
    lda a:JumpByStateTable117F7C7Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12D0:4CD JumpByStateTable12D04CD
; @run 6B:9CCD
; WHAT IT DOES: Goes to the code a table at $9CDF names for the object's state (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable12D04CDTable = $9CDF ; a handler's address per OBJ_STATE value
JumpByStateTable12D04CD:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable12D04CDTable,y
    sta z:zTemp00
    lda a:JumpByStateTable12D04CDTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DA:71E JumpByStateTable12DA71E
; @run 70:AF1E
; WHAT IT DOES: Goes to the code a table at $AF30 names for the object's state (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable12DA71ETable = $AF30 ; a handler's address per OBJ_STATE value
JumpByStateTable12DA71E:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable12DA71ETable,y
    sta z:zTemp00
    lda a:JumpByStateTable12DA71ETable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DA:793 JumpByStateTable12DA793
; @run 70:AF93
; WHAT IT DOES: Goes to the code a table at $AFA5 names for the object's state (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable12DA793Table = $AFA5 ; a handler's address per OBJ_STATE value
JumpByStateTable12DA793:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable12DA793Table,y
    sta z:zTemp00
    lda a:JumpByStateTable12DA793Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DB:120 JumpByStateTable12DB120
; @run 70:B120
; WHAT IT DOES: Goes to the code a table at $B132 names for the object's state (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable12DB120Table = $B132 ; a handler's address per OBJ_STATE value
JumpByStateTable12DB120:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable12DB120Table,y
    sta z:zTemp00
    lda a:JumpByStateTable12DB120Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DB:2AC JumpByStateTable12DB2AC
; @run 70:B2AC
; WHAT IT DOES: Goes to the code a table at $B2BE names for the object's state (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable12DB2ACTable = $B2BE ; a handler's address per OBJ_STATE value
JumpByStateTable12DB2AC:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable12DB2ACTable,y
    sta z:zTemp00
    lda a:JumpByStateTable12DB2ACTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DB:533 JumpByPhaseTable12DB533
; @run 70:B533
; WHAT IT DOES: Goes to the code a table at $B545 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable12DB533Table = $B545 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable12DB533:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable12DB533Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable12DB533Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DB:772 JumpByPhaseTable12DB772
; @run 70:B772
; WHAT IT DOES: Goes to the code a table at $B784 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable12DB772Table = $B784 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable12DB772:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable12DB772Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable12DB772Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DC:388 JumpByStateTable12DC388
; @run 70:BB88
; WHAT IT DOES: Goes to the code a table at $BB9A names for the object's state (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable12DC388Table = $BB9A ; a handler's address per OBJ_STATE value
JumpByStateTable12DC388:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable12DC388Table,y
    sta z:zTemp00
    lda a:JumpByStateTable12DC388Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DC:526 JumpByPhaseTable12DC526
; @run 70:BD26
; WHAT IT DOES: Goes to the code a table at $BD38 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable12DC526Table = $BD38 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable12DC526:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable12DC526Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable12DC526Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DD:199 JumpByStateTable12DD199
; @run 71:C199
; WHAT IT DOES: Goes to the code a table at $C1AB names for the object's state (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable12DD199Table = $C1AB ; a handler's address per OBJ_STATE value
JumpByStateTable12DD199:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable12DD199Table,y
    sta z:zTemp00
    lda a:JumpByStateTable12DD199Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DD:534 JumpByPhaseTable12DD534
; @run 71:C534
; WHAT IT DOES: Goes to the code a table at $C546 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable12DD534Table = $C546 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable12DD534:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable12DD534Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable12DD534Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DE:3D2 JumpByPhaseTable12DE3D2
; @run 71:CBD2
; WHAT IT DOES: Goes to the code a table at $CBE4 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable12DE3D2Table = $CBE4 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable12DE3D2:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable12DE3D2Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable12DE3D2Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 12DE:5E8 JumpByPhaseTable12DE5E8
; @run 71:CDE8
; WHAT IT DOES: Goes to the code a table at $CDFA names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable12DE5E8Table = $CDFA ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable12DE5E8:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable12DE5E8Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable12DE5E8Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FB:13D JumpByStateTable13FB13D
; @run 70:B13D
; WHAT IT DOES: Goes to the code a table at $B14F names for the object's state (Secret stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable13FB13DTable = $B14F ; a handler's address per OBJ_STATE value
JumpByStateTable13FB13D:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable13FB13DTable,y
    sta z:zTemp00
    lda a:JumpByStateTable13FB13DTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FB:366 JumpByStateTable13FB366
; @run 70:B366
; WHAT IT DOES: Goes to the code a table at $B378 names for the object's state (Secret stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable13FB366Table = $B378 ; a handler's address per OBJ_STATE value
JumpByStateTable13FB366:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable13FB366Table,y
    sta z:zTemp00
    lda a:JumpByStateTable13FB366Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FB:3A0 JumpByPhaseTable13FB3A0
; @run 70:B3A0
; WHAT IT DOES: Goes to the code a table at $B3B2 names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable13FB3A0Table = $B3B2 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable13FB3A0:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable13FB3A0Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable13FB3A0Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FB:70F JumpByPhaseTable13FB70F
; @run 70:B70F
; WHAT IT DOES: Goes to the code a table at $B721 names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable13FB70FTable = $B721 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable13FB70F:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable13FB70FTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable13FB70FTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FC:3DF JumpByPhaseTable13FC3DF
; @run 70:BBDF
; WHAT IT DOES: Goes to the code a table at $BBF1 names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable13FC3DFTable = $BBF1 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable13FC3DF:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable13FC3DFTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable13FC3DFTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FC:779 JumpByPhaseTable13FC779
; @run 70:BF79
; WHAT IT DOES: Goes to the code a table at $BF8B names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable13FC779Table = $BF8B ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable13FC779:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable13FC779Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable13FC779Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FD:1E2 JumpByPhaseTable13FD1E2
; @run 71:C1E2
; WHAT IT DOES: Goes to the code a table at $C1F4 names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable13FD1E2Table = $C1F4 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable13FD1E2:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable13FD1E2Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable13FD1E2Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 13FD:2B6 JumpByStateTable13FD2B6
; @run 71:C2B6
; WHAT IT DOES: Goes to the code a table at $C2C8 names for the object's state (Secret stage 0C's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable13FD2B6Table = $C2C8 ; a handler's address per OBJ_STATE value
JumpByStateTable13FD2B6:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable13FD2B6Table,y
    sta z:zTemp00
    lda a:JumpByStateTable13FD2B6Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161B:1C6 JumpByStateTable161B1C6
; @run 70:B1C6
; WHAT IT DOES: Goes to the code a table at $B1D8 names for the object's state (Stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable161B1C6Table = $B1D8 ; a handler's address per OBJ_STATE value
JumpByStateTable161B1C6:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable161B1C6Table,y
    sta z:zTemp00
    lda a:JumpByStateTable161B1C6Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161C:134 JumpByPhaseTable161C134
; @run 70:B934
; WHAT IT DOES: Goes to the code a table at $B946 names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable161C134Table = $B946 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable161C134:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable161C134Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable161C134Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161C:4B0 JumpByPhaseTable161C4B0
; @run 70:BCB0
; WHAT IT DOES: Goes to the code a table at $BCC2 names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable161C4B0Table = $BCC2 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable161C4B0:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable161C4B0Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable161C4B0Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161D:1DB JumpByPhaseTable161D1DB
; @run 71:C1DB
; WHAT IT DOES: Goes to the code a table at $C1ED names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable161D1DBTable = $C1ED ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable161D1DB:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable161D1DBTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable161D1DBTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161D:366 JumpByPhaseTable161D366
; @run 71:C366
; WHAT IT DOES: Goes to the code a table at $C378 names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable161D366Table = $C378 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable161D366:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable161D366Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable161D366Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161D:6E7 JumpByPhaseTable161D6E7
; @run 71:C6E7
; WHAT IT DOES: Goes to the code a table at $C6F9 names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable161D6E7Table = $C6F9 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable161D6E7:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable161D6E7Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable161D6E7Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161E:257 JumpByStateTable161E257
; @run 71:CA57
; WHAT IT DOES: Goes to the code a table at $CA69 names for the object's state (Stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable161E257Table = $CA69 ; a handler's address per OBJ_STATE value
JumpByStateTable161E257:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable161E257Table,y
    sta z:zTemp00
    lda a:JumpByStateTable161E257Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161F:3E3 JumpByStateTable161F3E3
; @run 71:D3E3
; WHAT IT DOES: Goes to the code a table at $D3F5 names for the object's state (Stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable161F3E3Table = $D3F5 ; a handler's address per OBJ_STATE value
JumpByStateTable161F3E3:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable161F3E3Table,y
    sta z:zTemp00
    lda a:JumpByStateTable161F3E3Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 161F:5C0 JumpByStateTable161F5C0
; @run 71:D5C0
; WHAT IT DOES: Goes to the code a table at $D5D2 names for the object's state (Stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable161F5C0Table = $D5D2 ; a handler's address per OBJ_STATE value
JumpByStateTable161F5C0:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable161F5C0Table,y
    sta z:zTemp00
    lda a:JumpByStateTable161F5C0Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CB:226 JumpByPhaseTable16CB226
; @run 70:B226
; WHAT IT DOES: Goes to the code a table at $B238 names for the object's OBJ_SUBSTATE (Secret stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable16CB226Table = $B238 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable16CB226:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable16CB226Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable16CB226Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CB:539 JumpByPhaseTable16CB539
; @run 70:B539
; WHAT IT DOES: Goes to the code a table at $B54B names for the object's OBJ_SUBSTATE (Secret stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable16CB539Table = $B54B ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable16CB539:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable16CB539Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable16CB539Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CB:76A JumpByStateTable16CB76A
; @run 70:B76A
; WHAT IT DOES: Goes to the code a table at $B77C names for the object's state (Secret stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable16CB76ATable = $B77C ; a handler's address per OBJ_STATE value
JumpByStateTable16CB76A:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable16CB76ATable,y
    sta z:zTemp00
    lda a:JumpByStateTable16CB76ATable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CC:06E JumpByStateTable16CC06E
; @run 70:B86E
; WHAT IT DOES: Goes to the code a table at $B880 names for the object's state (Secret stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable16CC06ETable = $B880 ; a handler's address per OBJ_STATE value
JumpByStateTable16CC06E:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable16CC06ETable,y
    sta z:zTemp00
    lda a:JumpByStateTable16CC06ETable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CC:36D JumpByStateTable16CC36D
; @run 70:BB6D
; WHAT IT DOES: Goes to the code a table at $BB7F names for the object's state (Secret stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable16CC36DTable = $BB7F ; a handler's address per OBJ_STATE value
JumpByStateTable16CC36D:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable16CC36DTable,y
    sta z:zTemp00
    lda a:JumpByStateTable16CC36DTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 16CC:3F4 JumpByStateTable16CC3F4
; @run 70:BBF4
; WHAT IT DOES: Goes to the code a table at $BC06 names for the object's state (Secret stage 0E's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable16CC3F4Table = $BC06 ; a handler's address per OBJ_STATE value
JumpByStateTable16CC3F4:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable16CC3F4Table,y
    sta z:zTemp00
    lda a:JumpByStateTable16CC3F4Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DB:1AE JumpByStateTable18DB1AE
; @run 70:B1AE
; WHAT IT DOES: Goes to the code a table at $B1C0 names for the object's state (Stage 10's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable18DB1AETable = $B1C0 ; a handler's address per OBJ_STATE value
JumpByStateTable18DB1AE:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable18DB1AETable,y
    sta z:zTemp00
    lda a:JumpByStateTable18DB1AETable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DB:1E9 JumpByPhaseTable18DB1E9
; @run 70:B1E9
; WHAT IT DOES: Goes to the code a table at $B1FB names for the object's OBJ_SUBSTATE (Stage 10's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable18DB1E9Table = $B1FB ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable18DB1E9:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable18DB1E9Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable18DB1E9Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DB:463 JumpByPhaseTable18DB463
; @run 70:B463
; WHAT IT DOES: Goes to the code a table at $B475 names for the object's OBJ_SUBSTATE (Stage 10's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable18DB463Table = $B475 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable18DB463:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable18DB463Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable18DB463Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DB:5AF JumpByPhaseTable18DB5AF
; @run 70:B5AF
; WHAT IT DOES: Goes to the code a table at $B5C1 names for the object's OBJ_SUBSTATE (Stage 10's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable18DB5AFTable = $B5C1 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable18DB5AF:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable18DB5AFTable,y
    sta z:zTemp00
    lda a:JumpByPhaseTable18DB5AFTable+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DC:1A1 JumpByPhaseTable18DC1A1
; @run 70:B9A1
; WHAT IT DOES: Goes to the code a table at $B9B3 names for the object's OBJ_SUBSTATE (Stage 10's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByPhaseTable18DC1A1Table = $B9B3 ; a handler's address per OBJ_SUBSTATE value
JumpByPhaseTable18DC1A1:
    lda a:OBJ_SUBSTATE,x
    asl a
    tay
    lda a:JumpByPhaseTable18DC1A1Table,y
    sta z:zTemp00
    lda a:JumpByPhaseTable18DC1A1Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)

; @routine 18DD:134 JumpByStateTable18DD134
; @run 71:C134
; WHAT IT DOES: Goes to the code a table at $C146 names for the object's state (Stage 10's overlay).
; HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
; WAITS: no.
; Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
JumpByStateTable18DD134Table = $C146 ; a handler's address per OBJ_STATE value
JumpByStateTable18DD134:
    lda a:OBJ_STATE,x
    asl a
    tay
    lda a:JumpByStateTable18DD134Table,y
    sta z:zTemp00
    lda a:JumpByStateTable18DD134Table+1,y
    sta z:zTemp01
    jmp (ZeroPage)
