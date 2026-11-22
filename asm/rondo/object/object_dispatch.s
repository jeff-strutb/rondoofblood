; @routine 0519:329 StepReleaseOnHit
; @run 70:A329
; WHAT IT DOES: An object that waits to be hit. It starts from its record; each frame after, it
; steps its animation list (the list at $A365); once it has taken a hit of 5 or more it records
; the event, takes its next kind and a timer of $10, goes back to state 0, and starts an object of
; kind $9B at its place in the first free slot from $50 to $53.
; Untraced: the arm where no slot from $50 to $53 is free (FindFreeSlotFrom50 ran in no script that far).
; HOW: LoadObjectRecord and a frame of 1 on state 0; else StepObjectFrames with the list's pointer
; put in zTemp10-11, then the hit byte compared with 5. The spawn is a tail jump into
; StartObjectAtPlace. The list is read from the disc's table at $A365 by the step (bank 70).
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper (started: A 1, C from
; the record; stepped: A the hit byte, C clear and N, Z from hit less 5; no slot: as
; FindFreeSlotFrom50; spawned: as StartObjectAtPlace with the slot found in Y).
StepReleaseOnHit:
    lda a:OBJ_27A8,x
    bne StepReleaseOnHit_at0
    jsr VecLoadObjectRecord
    lda #$01
    sta a:OBJ_2A68,x
    rts
StepReleaseOnHit_at0:
    lda #$A3
    sta z:zTemp11
    lda #$65
    sta z:zTemp10
    jsr VecStepObjectFrames
    lda a:OBJ_33B0,x
    cmp #$05
    bcc StepReleaseOnHit_at1
    jsr VecRecordObjectEvent
    lda a:OBJ_3040,x
    sta a:OBJ_2750,x
    lda #$10
    sta a:OBJ_3A90,x
    stz a:OBJ_27A8,x
    jsr VecFindFreeSlotFrom50
    bcc StepReleaseOnHit_at1
    lda #$9B
    jmp VecStartObjectAtPlace
StepReleaseOnHit_at1:
    rts

; @routine 0519:36D StepFallingItem
; @run 70:A36D
; WHAT IT DOES: A small thing that falls and then goes away. It starts from its record with its
; speed set from a two-byte row of the table at $A3B1 picked by its kind less $9B; each frame after,
; it moves down by its Y speed, clears its flash flag and steps the animation list the second table
; ($A3B9, by the same index) names; when the list ends the object is cleared.
; HOW: ApplyVelocityY, then StepObjectFrames with zTemp10-11 loaded from the table (bank 70); the
; clear is a tail jump into ClearObject, which leaves every register as it was.
; WAITS: no.
; Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper (started: A the speed's low
; byte, Y the table index, C bit 7 of kind less $9B; else as StepObjectFrames).
StepFallingItem:
    lda a:OBJ_27A8,x
    bne StepFallingItem_at0
    jsr VecLoadObjectRecord
    stz a:OBJ_3408,x
    stz a:OBJ_3460,x
    lda a:OBJ_2750,x
    sec
    sbc #$9B
    asl a
    tay
    lda a:$A3B2,y
    sta a:OBJ_VY_HI,x
    lda a:$A3B1,y
    sta a:OBJ_VY_LO,x
    rts
StepFallingItem_at0:
    jsr VecApplyVelocityY
    stz a:OBJ_2B18,x
    lda a:OBJ_2750,x
    sec
    sbc #$9B
    asl a
    tay
    lda a:$A3B9,y
    sta z:zTemp10
    lda a:$A3BA,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bne StepFallingItem_at1
    jmp VecClearObject
StepFallingItem_at1:
    rts

; @routine 0519:46A EnterHurtStateOnHit
; @run 70:A46A
; WHAT IT DOES: Sends an object that is in one of its first four states and has taken a hit
; of 1 to 4 into state 5, the state its handler table uses for being hurt.
; HOW: Reads the state and the hit byte, each once, and writes the state only when both are
; from 1 to 4.
; WAITS: no.
; Entry: X object. Exit: A and flags as the last compare or load left them; X and Y kept.
EnterHurtStateOnHit:
    lda a:OBJ_27A8,x
    beq EnterHurtStateOnHit_out
    cmp #$05
    bcs EnterHurtStateOnHit_out
    lda a:OBJ_33B0,x
    beq EnterHurtStateOnHit_out
    cmp #$05
    bcs EnterHurtStateOnHit_out
    lda #$05
    sta a:OBJ_27A8,x
EnterHurtStateOnHit_out:
    rts

; @routine 0519:482 InitObjectFromRecord
; @run 70:A482
; WHAT IT DOES: Starts a newly placed object: loads its record, copies slot 0's byte of the
; OBJ_2FE8 column into the object's, sets two of its counters and its animation delay to 4, and
; its timer to $60.
; HOW: Calls LoadObjectRecord through its vector (which keeps X), then stores each byte.
; WAITS: no.
; Entry: X object. Exit: A $60, N and Z from it, C as LoadObjectRecord left it; Y 11; X kept. D clear.
InitObjectFromRecord:
    jsr VecLoadObjectRecord
    lda a:OBJ_2FE8
    sta a:OBJ_2FE8,x
    lda #$04
    sta a:OBJ_3988,x
    sta a:OBJ_3A38,x
    lda #$04
    sta a:OBJ_3510,x
    lda #$60
    sta a:OBJ_34B8,x
    rts

; @routine 0519:4BD FallAndBounce
; @run 70:A4BD
; WHAT IT DOES: Moves an object by its velocity and asks the ground under it; on floor the
; object enters its next state. Unless the ground is deadly, the object's X speed is set
; from its angle (the full cosine) and its Y speed to $80, and its angle is turned on by
; $0280.
; HOW: Calls ApplyVelocity and ProbeTerrain (floor, height 0), then ScaleByCosine with a
; multiplier of 256; the angle is the two bytes OBJ_35C0 (low) and OBJ_3568 (high).
; WAITS: no.
; Entry: X object. Exit: A and flags from the last AND or add; X kept; Y as ScaleByCosine
; left it.
FallAndBounce:
    jsr VecApplyVelocity
    lda #$00
    sta z:zProduct1
    lda #$01
    jsr VecProbeTerrain
    and #$01
    beq FallAndBounce_at0
    inc a:OBJ_27A8,x
FallAndBounce_at0:
    lda z:zProduct0
    and #$10
    bne FallAndBounce_out
    lda #$00
    sta z:zTemp00
    lda #$01
    sta z:zTemp01
    lda a:OBJ_3568,x
    jsr VecScaleByCosine
    lda z:zProduct2
    sta a:OBJ_VX_HI,x
    lda z:zProduct1
    sta a:OBJ_VX_LO,x
    stz a:OBJ_VY_HI,x
    lda #$80
    sta a:OBJ_VY_LO,x
    lda a:OBJ_35C0,x
    clc
    adc #$80
    sta a:OBJ_35C0,x
    lda a:OBJ_3568,x
    adc #$02
    sta a:OBJ_3568,x
FallAndBounce_out:
    rts

; @routine 0519:5EF ChangeKindFromTable
; @run 70:A5EF
; WHAT IT DOES: An object changes into another kind. Sound $0B plays, the player's byte at $3BD6
; becomes 1, and zField93 takes a byte of the table at $A650 picked by the kind less $80 (its old
; value is the index into the table at $A668 that names the new kind). A zero there clears the object.
; Otherwise the object takes the new kind and state 0 with a timer of $60, is put at the player's place,
; moved 8 up with a borrow, and thrown up at a speed of $FEC0 and sideways by a table pair picked by the
; player's facing.
; HOW: The old zField93 is kept on the stack across the table read; the place copy is made with X
; and Y swapped (source slot 0, destination the object); the clear is a tail jump into ClearObject.
; Tables are read from the disc (bank 70) by their addresses until the data layer names them; $3BD6
; has no name yet (a row is requested).
; WAITS: no.
; Entry: X object. Exit: see the wrapper (cleared: A 0, Y the old zField93, C and V from the kind
; less $80; changed: A the sideways speed's low byte, Y the facing's table index, C the facing's top
; bit, V from the borrow into the Y position's high byte).
ChangeKindFromTable:
    lda #$0B
    jsr VecSoundCommand
    lda z:zField93
    pha
    lda #$01
    sta a:PlayerByte3BD6
    lda a:OBJ_2750,x
    sec
    sbc #$80
    tay
    lda a:$A650,y
    sta z:zField93
    ply
    lda a:$A668,y
    bne ChangeKindFromTable_at0
    jmp VecClearObject
ChangeKindFromTable_at0:
    sta a:OBJ_2750,x
    stz a:OBJ_27A8,x
    lda #$60
    sta a:OBJ_3A90,x
    ldy #$00
    sxy
    jsr VecCopyObjectPlace
    sxy
    lda a:OBJ_Y_LO,x
    sec
    sbc #$08
    sta a:OBJ_Y_LO,x
    lda a:OBJ_Y_HI,x
    sbc #$00
    sta a:OBJ_Y_HI,x
    lda #$FE
    sta a:OBJ_VY_HI,x
    lda #$C0
    sta a:OBJ_VY_LO,x
    lda a:OBJ_2AC0
    asl a
    tay
    lda a:$A672,y
    sta a:OBJ_VX_HI,x
    lda a:$A671,y
    sta a:OBJ_VX_LO,x
    rts

; @routine 0519:74E SetBlinkFlash
; @run 70:A74E
; WHAT IT DOES: Sets an object's flash flag (the palette flash its sprite shows) to $86 on
; alternate beats of the wave counter once its timer has reached $18, and to 0 otherwise.
; HOW: Clears the flag first, then reads the timer and, if it is $18 or more and bit 1 of the
; wave counter is clear, writes $86.
; WAITS: no.
; Entry: X object. Exit: A and flags as the last compare, AND or load left them; X and Y kept.
SetBlinkFlash:
    stz a:OBJ_2B18,x
    lda a:OBJ_3A90,x
    cmp #$18
    bcc SetBlinkFlash_out
    lda z:zWaveCounter
    and #$02
    bne SetBlinkFlash_out
    lda #$86
    sta a:OBJ_2B18,x
SetBlinkFlash_out:
    rts

; @routine 051A:055 BitMaskFromIndex
; @run 70:A855
; WHAT IT DOES: Gives the byte with only bit Y set (1 shifted left Y times; 0 if Y is 8 or more).
; HOW: Shifts a 1 left, counting Y down to 0.
; WAITS: no.
; Entry: Y the bit number. Exit: A the mask; Y 0; Z and C set, N clear (the last compare).
BitMaskFromIndex:
    lda #$01
BitMaskFromIndex_at0:
    cpy #$00
    beq BitMaskFromIndex_out
    asl a
    dey
    bra BitMaskFromIndex_at0
BitMaskFromIndex_out:
    rts
