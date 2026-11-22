; TEMPORARY: object routines every stage overlay carries its own copy of (they differ only in the
; disc tables they read), kept together here until each has a home.
; The disc tables are named here by their overlay address until the typed data layer reads them.
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
; TEMPORARY: object routines every stage overlay carries its own copy of (they differ only in the
; disc tables they read), kept together here until each has a home.
; The disc tables are named here by their overlay address until the typed data layer reads them.
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
; TEMPORARY: object routines every stage overlay carries its own copy of (they differ only in the
; disc tables they read), kept together here until each has a home.
; The disc tables are named here by their overlay address until the typed data layer reads them.
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
;
; @routine 07D9:2C5 DropItemWhenHitA
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $AB3D
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AB45,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
; reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
; coverage (PROCEDURE section 6).
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
; free slot (or the table index); X kept. Clobbers zTemp00.
DropItemWhenHitA:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq RichterStandStep1
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:$AB3D,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$AB45,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 0939:2C5 DropItemWhenHitB
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $ABFD
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AC05,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
; reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
; coverage (PROCEDURE section 6).
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
; free slot (or the table index); X kept. Clobbers zTemp00.
DropItemWhenHitB:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq RichterStandStep1
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:$ABFD,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$AC05,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 0BF9:2C5 DropItemWhenHitC
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $AC08
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AC10,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
; reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
; coverage (PROCEDURE section 6).
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
; free slot (or the table index); X kept. Clobbers zTemp00.
DropItemWhenHitC:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq RichterStandStep1
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:$AC08,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$AC10,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 0D59:2C5 DropItemWhenHitD
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $ACD2
; (indexed by those bits minus one) picks the chance; the item comes from a table at $ACDA,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
; reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
; coverage (PROCEDURE section 6).
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
; free slot (or the table index); X kept. Clobbers zTemp00.
DropItemWhenHitD:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq RichterStandStep1
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:$ACD2,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$ACDA,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 1179:2C5 DropItemWhenHitE
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $AC56
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AC5E,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
; reaches as untraced bytes (about a quarter of the cases) are NOT CHECKED; they wait for
; coverage (PROCEDURE section 6).
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
; free slot (or the table index); X kept. Clobbers zTemp00.
DropItemWhenHitE:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq RichterStandStep1
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:$AC56,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$AC5E,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 088A:5AF FaceThePlayerInSomeStatesC
; @run 70:ADAF
; WHAT IT DOES: FaceThePlayerInSomeStates in the overlay at sector 088A, with its table at $ADC0.
; HOW: The shared body.
; WAITS: no.
; Entry, exit: as FaceThePlayerInSomeStates.
FaceThePlayerInSomeStatesC:
    ldy a:OBJ_27A8,x
    lda a:$ADC0,y
    beq FaceThePlayerInSomeStatesC_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
FaceThePlayerInSomeStatesC_at0:
    rts

; @routine 0A9C:7FE FaceThePlayerInSomeStatesD
; @run 71:BFFE
; WHAT IT DOES: FaceThePlayerInSomeStates in the overlay at sector 0A9C, with its table at $C00F.
; HOW: The shared body.
; WAITS: no.
; Entry, exit: as FaceThePlayerInSomeStates.
FaceThePlayerInSomeStatesD:
    ldy a:OBJ_27A8,x
    lda a:$C00F,y
    beq FaceThePlayerInSomeStatesD_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_2AC0,x
FaceThePlayerInSomeStatesD_at0:
    rts

; @routine 0A9E:591 ThrowByNextKindB
; @run 71:CD91
; WHAT IT DOES: ThrowByNextKind in the overlay at sector 0A9E, with its X speed table at $CDB4.
; HOW: The shared body.
; WAITS: no.
; Entry, exit: as ThrowByNextKind.
ThrowByNextKindB:
    lda a:OBJ_3040,x
    and #$01
    tay
    lda a:$CDB4,y
    sta a:OBJ_VX_HI,x
    lda a:OBJ_3040,x
    asl a
    asl a
    asl a
    asl a
    eor #$FF
    inc a
    sta a:OBJ_VY_HI,x
    jsr VecApplyVelocity
    stz a:OBJ_VX_HI,x
    stz a:OBJ_VY_HI,x
    rts

; @routine 0A9E:5B6 PickTimersAtRandomB
; @run 71:CDB6
; WHAT IT DOES: PickTimersAtRandom in the overlay at sector 0A9E, with its table at $CDCA.
; HOW: The shared body.
; WAITS: no.
; Entry, exit: as PickTimersAtRandom.
PickTimersAtRandomB:
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$CDCA,y
    sta a:OBJ_3510,x
    lda a:$CDCB,y
    sta a:OBJ_34B8,x
    rts

; @routine 0A9E:5D2 PickAngleAtRandomB
; @run 71:CDD2
; WHAT IT DOES: PickAngleAtRandom in the overlay at sector 0A9E, with its table at $CDE6.
; HOW: The shared body.
; WAITS: no.
; Entry, exit: as PickAngleAtRandom.
PickAngleAtRandomB:
    jsr VecNextRandom
    and #$03
    asl a
    tay
    lda a:$CDE6,y
    sta a:OBJ_35C0,x
    lda a:$CDE7,y
    sta a:OBJ_3568,x
    rts

; @routine 117C:73A WalkWhileFrameChangesD
; @run 70:BF3A
; WHAT IT DOES: WalkWhileFrameChangesB in the overlay at sector 117C: lists at $BF56 plus Y, X
; speeds at $BF5A by f3510.
; HOW: StepObjectFrames, then ApplyVelocityX.
; WAITS: no.
; Entry, exit: as WalkWhileFrameChangesB.
WalkWhileFrameChangesD:
    lda a:$BF56,y
    sta z:zTemp10
    lda a:$BF57,y
    sta z:zTemp11
    jsr VecStepObjectFrames
    bcc WalkWhileFrameChangesD_at0
    ldy a:OBJ_3510,x
    lda a:$BF5A,y
    sta a:OBJ_VX_HI,x
    jsr VecApplyVelocityX
WalkWhileFrameChangesD_at0:
    rts
