; @routine 01DD:009 MariaVecRunObjects
; @run 6D:A009
; WHAT IT DOES: Her overlay's vector to the run of her objects.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaRunObjects.
MariaVecRunObjects:
    jmp MariaRunObjects

; @routine 01DD:00F MariaVecDeathObjects
; @run 6D:A00F
; WHAT IT DOES: Her overlay's vector to her death's objects frame.
; HOW: One jump.
; WAITS: no.
; Entry, exit: as MariaDeathObjects.
MariaVecDeathObjects:
    jmp MariaDeathObjects

; @routine 01E1:39F MariaDeathObjects
; @run 6E:C39F
; WHAT IT DOES: The frame of her death's objects: her death's frame, the player's and the camera's frame, then the
; run of her objects.
; HOW: MariaDeathFrame, PlayerAndCameraFrame (through the kernel's vector at $40D8), MariaRunObjects.
; WAITS: no.
; Entry: X and Y the caller's. Exit: as MariaRunObjects.
MariaDeathObjects:
    jsr MariaDeathFrame
    jsr VecPlayerAndCameraFrame
    jsr MariaRunObjects
    rts
