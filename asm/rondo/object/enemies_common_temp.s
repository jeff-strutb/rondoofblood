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
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
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
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
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
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
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
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
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
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
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
    ldy a:OBJ_STATE,x
    lda a:$ADC0,y
    beq FaceThePlayerInSomeStatesC_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
FaceThePlayerInSomeStatesC_at0:
    rts

; @routine 0A9C:7FE FaceThePlayerInSomeStatesD
; @run 71:BFFE
; WHAT IT DOES: FaceThePlayerInSomeStates in the overlay at sector 0A9C, with its table at $C00F.
; HOW: The shared body.
; WAITS: no.
; Entry, exit: as FaceThePlayerInSomeStates.
FaceThePlayerInSomeStatesD:
    ldy a:OBJ_STATE,x
    lda a:$C00F,y
    beq FaceThePlayerInSomeStatesD_at0
    jsr VecSideOfPlayer
    and #$01
    sta a:OBJ_FACING_LEFT,x
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
; @routine 0259:2FE PlayHitSoundA
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $ABEE, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundA:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundA_at0
    dec a
    tay
    lda a:$ABEE,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundA_at0:
    rts

; @routine 0519:2FE PlayHitSoundB
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AC23, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundB:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundB_at0
    dec a
    tay
    lda a:$AC23,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundB_at0:
    rts

; @routine 07D9:2FE PlayHitSoundC
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AB75, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundC:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundC_at0
    dec a
    tay
    lda a:$AB75,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundC_at0:
    rts

; @routine 0939:2FE PlayHitSoundD
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AC35, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundD:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundD_at0
    dec a
    tay
    lda a:$AC35,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundD_at0:
    rts

; @routine 09E9:2FE PlayHitSoundE
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AC17, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundE:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundE_at0
    dec a
    tay
    lda a:$AC17,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundE_at0:
    rts

; @routine 0BF9:2FE PlayHitSoundF
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AC40, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundF:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundF_at0
    dec a
    tay
    lda a:$AC40,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundF_at0:
    rts

; @routine 0CA9:2FE PlayHitSoundG
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AC64, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundG:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundG_at0
    dec a
    tay
    lda a:$AC64,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundG_at0:
    rts

; @routine 0D59:2FE PlayHitSoundH
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AD0A, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundH:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundH_at0
    dec a
    tay
    lda a:$AD0A,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundH_at0:
    rts

; @routine 0EB9:2FE PlayHitSoundI
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AC98, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundI:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundI_at0
    dec a
    tay
    lda a:$AC98,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundI_at0:
    rts

; @routine 1019:2FE PlayHitSoundJ
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AC74, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundJ:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundJ_at0
    dec a
    tay
    lda a:$AC74,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundJ_at0:
    rts

; @routine 1179:2FE PlayHitSoundK
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AC8E, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundK:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundK_at0
    dec a
    tay
    lda a:$AC8E,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundK_at0:
    rts

; @routine 16C9:2FE PlayHitSoundL
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $ABBE, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundL:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundL_at0
    dec a
    tay
    lda a:$ABBE,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundL_at0:
    rts

; @routine 0259:2C5 DropItemWhenHitF
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $ABC6
; (indexed by those bits minus one) picks the chance; the item comes from a table at $ABCE,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
; reaches as untraced bytes (most of the cases here) are NOT CHECKED; they wait for coverage
; (PROCEDURE section 6).
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
; free slot (or the table index); X kept. Clobbers zTemp00.
DropItemWhenHitF:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
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
    cmp a:$ABC6,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$ABCE,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 09E9:2C5 DropItemWhenHitG
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $ABEF
; (indexed by those bits minus one) picks the chance; the item comes from a table at $ABF7,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
; reaches as untraced bytes (most of the cases here) are NOT CHECKED; they wait for coverage
; (PROCEDURE section 6).
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
; free slot (or the table index); X kept. Clobbers zTemp00.
DropItemWhenHitG:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
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
    cmp a:$ABEF,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$ABF7,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 0A99:2C5 DropItemWhenHitH
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $AC44
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AC4C,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
; reaches as untraced bytes (most of the cases here) are NOT CHECKED; they wait for coverage
; (PROCEDURE section 6).
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
; free slot (or the table index); X kept. Clobbers zTemp00.
DropItemWhenHitH:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
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
    cmp a:$AC44,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$AC4C,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

; @routine 0A99:2FE PlayHitSoundM
; @run 70:A2FE
; WHAT IT DOES: Plays the sound that bits 5 to 7 of the object's f31f8 ask for, when they are not
; zero and the object is on the first screen: a table of sounds at overlay address $AC7C, indexed
; by those bits minus one. Every stage overlay has its own copy with its own table.
; HOW: Shifts the byte right five times, masks three bits, indexes the table, and calls
; SoundIfOnFirstScreen.
; WAITS: no.
; Entry: X object, D clear. Exit: not played: A the masked bits, N, Z from them, C bit 4 of f31f8;
; played: as SoundIfOnFirstScreen leaves it (Y the table index).
PlayHitSoundM:
    lda a:OBJ_HIT_REACTION,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSoundM_at0
    dec a
    tay
    lda a:$AC7C,y
    jsr VecSoundIfOnFirstScreen
PlayHitSoundM_at0:
    rts

; @routine 0CA9:2C5 DropItemWhenHitI
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $AC3C
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AC44,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace. The arms the draft
; reaches as untraced bytes (most of the cases here) are NOT CHECKED; they wait for coverage
; (PROCEDURE section 6).
; WAITS: no.
; Entry: X object, D clear. Exit: A, N, Z, C, V from the last compare, add or load that ran; Y the
; free slot (or the table index); X kept. Clobbers zTemp00.
DropItemWhenHitI:
    lda a:OBJ_HIT_TAKEN,x
    cmp #$05
    bcc RichterStandStep1
    lda a:OBJ_HIT_REACTION,x
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
    cmp a:$AC3C,y
    bcs RichterStandStep1
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:$AC44,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc RichterStandStep1
    lda z:zTemp00
    jsr VecStartObjectAtPlace
    rts

