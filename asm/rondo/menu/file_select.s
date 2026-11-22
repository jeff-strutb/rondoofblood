; @routine 00C4:3F6 ShiftObjectsByScrollChange
; @run 6B:9BF6
; WHAT IT DOES: Moves the menu's objects 16 to 63 sideways by what the menu's scroll has moved
; since the last time, so they stay where they are on the screen.
; HOW: The X scroll word kept last time (the menu keeps it in the first X and Y speed bytes,
; which it has no other use for) less the scroll's X word now is kept in their place as the shift, and added, low byte then high byte, to the X
; place of each of the objects.
; WAITS: no.
; Entry: nothing. Exit: A the last high byte sum, V from that sum, X $40, Z and C set, N clear.
ShiftObjectsByScrollChange:
    lda a:ScrollSpeedX
    sec
    sbc a:ScrollXLo
    sta a:ScrollSpeedX
    lda a:ScrollSpeedY
    sbc a:ScrollXHi
    sta a:ScrollSpeedY
    ldx #$10
ShiftObjectsByScrollChange_at0:
    lda a:OBJ_X_LO,x
    clc
    adc a:ScrollSpeedX
    sta a:OBJ_X_LO,x
    lda a:OBJ_X_HI,x
    adc a:ScrollSpeedY
    sta a:OBJ_X_HI,x
    inx
    cpx #$40
    bne ShiftObjectsByScrollChange_at0
    rts

; @routine 00C4:46A SetCursorValue
; @run 6B:9C6A
; WHAT IT DOES: Puts both cursors on the cursor value A and sends them to its place.
; HOW: Stores A in both cursor objects' value and goes to SetCursorTargets.
; WAITS: no.
; Entry: A the cursor value. Exit: as SetCursorTargets.
SetCursorValue:
    sta a:OBJ_34B8
    sta a:OBJ_34B8+1
    jmp SetCursorTargets

; @routine 00C4:4CA PlaceCursorObjects
; @run 6B:9CCA
; WHAT IT DOES: PlaceCursorObject for both cursor objects, 0 then 1, with the same value.
; HOW: Keeps A on the stack; a call for object 0; the PLA and INX after it run on into
; PlaceCursorObject for object 1.
; WAITS: no.
; Entry: A the cursor value. Exit: as PlaceCursorObject for object 1, X 1.
PlaceCursorObjects:
    clx
    pha
    bsr PlaceCursorObject
    pla
    inx

; @routine 00C4:4D0 PlaceCursorObject
; @run 6B:9CD0
; WHAT IT DOES: Makes a cursor object afresh at the place its cursor value names.
; HOW: Keeps A on the stack across ClearObject, gives the object the value, frame 4 and the
; place from the same table SetCursorTarget reads (index as there).
; WAITS: no.
; Entry: A the cursor value, X the object (0 or 1). Exit: A the Y place, N and Z from it, C
; bit 6 of the cursor value, Y the table index. X kept.
PlaceCursorObject:
    pha
    jsr VecClearObject
    lda #$04
    sta a:OBJ_2A10,x
    txa
    lsr a
    pla
    sta a:OBJ_34B8,x
    rol a
    asl a
    tay
    lda a:$AD1F,y
    sta a:OBJ_X_LO,x
    lda a:$AD20,y
    sta a:OBJ_Y_LO,x
    rts

; @routine 00C4:5B6 SetCursorTargets
; @run 6B:9DB6
; WHAT IT DOES: SetCursorTarget for both cursor objects, 0 then 1.
; HOW: A call for object 0; the one byte after it, INX, runs on into SetCursorTarget for 1.
; WAITS: no.
; Entry: nothing. Exit: as SetCursorTarget for object 1, X 1.
SetCursorTargets:
    clx
    bsr SetCursorTarget
    inx

; @routine 00C4:5BA SetCursorTarget
; @run 6B:9DBA
; WHAT IT DOES: Sends a cursor object to the place its cursor value names: the target is
; read from a table of places, one pair for each value and each of the two cursors.
; HOW: The object's cursor value (OBJ->f34b8) doubled with the object's low bit under it, doubled
; again, is the table index; the pair at that index goes to the object's target (X and Y) and
; the object is marked as on its way (OBJ->f3510 is 1).
; WAITS: no.
; Entry: X the cursor object (0 or 1). Exit: A 1, N and Z clear, C bit 6 of the cursor value, Y
; the table index. X kept.
SetCursorTarget:
    txa
    lsr a
    lda a:OBJ_34B8,x
    rol a
    asl a
    tay
    lda a:$AD1F,y
    sta a:OBJ_3568,x
    lda a:$AD20,y
    sta a:OBJ_35C0,x
    lda #$01
    sta a:OBJ_3510,x
    rts

; @routine 00C4:5D4 HalveObjectVelocity
; @run 6B:9DD4
; WHAT IT DOES: Halves an object's speed along both axes, keeping the sign.
; HOW: The X speed (high byte, then low byte) and then the Y speed, each as a 16-bit
; arithmetic shift right: the sign is copied into the carry first so the high byte keeps it.
; WAITS: no.
; Entry: X the object. Exit: A the Y speed's high byte shifted left once, N and Z from the Y
; speed's new low byte, C the bit it lost. X and Y kept.
HalveObjectVelocity:
    lda a:OBJ_VX_HI,x
    asl a
    ror a:OBJ_VX_HI,x
    ror a:OBJ_VX_LO,x
    lda a:OBJ_VY_HI,x
    asl a
    ror a:OBJ_VY_HI,x
    ror a:OBJ_VY_LO,x
    rts

; @routine 00C4:5E9 ObjectFrameFromTick
; @run 6B:9DE9
; WHAT IT DOES: Gives an object the picture frame (1 to 8) that the frame counter says: a
; frame lasts four ticks, and the eight frames go round.
; HOW: Reads zFrameTick, shifts it right twice, keeps three bits, adds one, stores it in the
; object's frame byte.
; WAITS: no.
; Entry: X the object. Exit: A the frame, N and Z clear, C bit 1 of the tick. X and Y kept.
ObjectFrameFromTick:
    lda z:zFrameTick
    lsr a
    lsr a
    and #$07
    inc a
    sta a:OBJ_2A68,x
    rts

; @routine 00C4:602 MenuPageAdvance
; @run 6B:9E02
; WHAT IT DOES: Moves the menu on to the first step of the next page.
; HOW: Reads zGameSubMode, adds one and runs on into SetMenuPage.
; WAITS: no.
; Entry: nothing. Exit: A 0, N and Z from the new page, C kept.
MenuPageAdvance:
    lda z:zGameSubMode
    inc a

; @routine 00C4:605 SetMenuPage
; @run 6B:9E05
; WHAT IT DOES: Puts the menu on page A, at its first step.
; HOW: Stores A in zGameSubMode, then runs on into SetMenuStep with the step 0.
; WAITS: no.
; Entry: A the page. Exit: A 0, the flags kept.
SetMenuPage:
    sta z:zGameSubMode
    cla
    bra SetMenuStep

; @routine 00C4:60A MenuStepAdvance
; @run 6B:9E0A
; WHAT IT DOES: Moves the menu on to the next step of its page.
; HOW: Reads zField80, adds one and runs on into SetMenuStep.
; WAITS: no.
; Entry: nothing. Exit: A the new step, N and Z from it, C kept.
MenuStepAdvance:
    lda z:zField80
    inc a

; @routine 00C4:60D SetMenuStep
; @run 6B:9E0D
; WHAT IT DOES: Puts the menu in step A of its page.
; HOW: One store into zField80.
; WAITS: no.
; Entry: A the step. Exit: A and the flags kept.
SetMenuStep:
    sta z:zField80
    rts
