; @routine 07DD:00E FallStep07DE
; @run 71:C00E
; WHAT IT DOES: Falls without the speed limit and moves.
; HOW: ApplyGravityUnlimited and a tail jump to ApplyVelocity.
; WAITS: no.
; Entry: X object. Exit: as the last instruction, call or jump left them.
FallStep07DE:
    jsr VecApplyGravityUnlimited
    jmp VecApplyVelocity
