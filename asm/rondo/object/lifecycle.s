; @routine 0046:1E7 ClearObjectInY
; @run 69:59E7
; WHAT IT DOES: ClearObject for a caller holding the slot in Y.
; HOW: Swaps X and Y around the call.
; Entry: Y object. Exit: A, X, Y and flags kept.
ClearObjectInY:
    sxy
    jsr ClearObject
    sxy
    rts

; @routine 0046:1ED ClearObject
; @run 69:59ED
; WHAT IT DOES: Empties an object slot altogether.
; HOW: Zeroes the identity and position fields, then runs into
; ClearObjectExceptPlace.
; Entry: X object. Exit: A, X, Y and flags kept.
ClearObject:
    stz a:OBJ_KIND,x
    stz a:OBJ_STATE,x
    stz a:OBJ_2A10,x
    stz a:OBJ_FRAME,x
    stz a:OBJ_X_HI,x
    stz a:OBJ_X_LO,x
    stz a:OBJ_X_FRAC,x
    stz a:OBJ_Y_HI,x
    stz a:OBJ_Y_LO,x
    stz a:OBJ_Y_FRAC,x

; @routine 0046:20B ClearObjectExceptPlace
; @run 69:5A0B
; WHAT IT DOES: Empties an object slot but leaves where it is: its position and
; the fields ClearObject clears before this one stay.
; HOW: Stores zero to each field in turn: three fields, then the velocities and
; every field from there up to the last at $3AE8, $58 apart.
; Entry: X object. Exit: A, X, Y and flags kept.
ClearObjectExceptPlace:
    stz a:OBJ_FACING_LEFT,x
    stz a:OBJ_2B18,x
    stz a:OBJ_SUBSTATE,x
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    stz a:OBJ_VY_LO,x
    stz a:OBJ_FIELDS_2EE0,x
    stz a:OBJ_FLAGS_2F38,x
    stz a:OBJ_2F90,x
    stz a:OBJ_SPRITE_FLIP,x
    stz a:OBJ_3040,x
    stz a:OBJ_EVENT_ID,x
    stz a:OBJ_30F0,x
    stz a:OBJ_3148,x
    stz a:OBJ_31A0,x
    stz a:OBJ_HIT_REACTION,x
    stz a:OBJ_3250,x
    stz a:OBJ_32A8,x
    stz a:OBJ_3300,x
    stz a:OBJ_3358,x
    stz a:OBJ_HIT_TAKEN,x
    stz a:OBJ_ANIM_STEP,x
    stz a:OBJ_ANIM_TIMER,x
    stz a:OBJ_34B8,x
    stz a:OBJ_3510,x
    stz a:OBJ_3568,x
    stz a:OBJ_35C0,x
    stz a:OBJ_3618,x
    stz a:OBJ_3670,x
    stz a:OBJ_36C8,x
    stz a:OBJ_3720,x
    stz a:OBJ_3778,x
    stz a:OBJ_37D0,x
    stz a:OBJ_3828,x
    stz a:OBJ_3880,x
    stz a:OBJ_38D8,x
    stz a:OBJ_HITBOX_DX,x
    stz a:OBJ_HITBOX_HALF_W,x
    stz a:OBJ_HITBOX_DY,x
    stz a:OBJ_HITBOX_HALF_H,x
    stz a:OBJ_3A90,x
    stz a:OBJ_3AE8,x
    rts

; @routine 0046:28D ClearEnemySlots
; @run 69:5A8D
; WHAT IT DOES: Empties every object slot from $10 to $53, the slots below $10
; staying: the player and what travels with it.
; HOW: ClearObject in a loop.
; Entry: nothing. Exit: X kept, N and Z from X, C 1. A and Y kept.
ClearEnemySlots:
    phx
    ldx #$10
ClearEnemySlots_at0:
    jsr ClearObject
    inx
    cpx #$54
    bcc ClearEnemySlots_at0
    plx
    rts
