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
    lda a:OBJ_31F8,x
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
    lda a:OBJ_31F8,x
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

; @routine 07D9:2C5 DropItemWhenHitA
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $AB3D
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AB45,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace.
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
    lda a:OBJ_31F8,x
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

; @routine 0939:2C5 DropItemWhenHitB
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $ABFD
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AC05,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace.
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
    lda a:OBJ_31F8,x
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
    lda a:OBJ_31F8,x
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

; @routine 0BF9:2C5 DropItemWhenHitC
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $AC08
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AC10,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace.
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
    lda a:OBJ_31F8,x
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
    lda a:OBJ_31F8,x
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

; @routine 0D59:2C5 DropItemWhenHitD
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $ACD2
; (indexed by those bits minus one) picks the chance; the item comes from a table at $ACDA,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace.
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
    lda a:OBJ_31F8,x
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
    lda a:OBJ_31F8,x
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
    lda a:OBJ_31F8,x
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

; @routine 1179:2C5 DropItemWhenHitE
; @run 70:A2C5
; WHAT IT DOES: When the object has taken a hit of 5 or more and its f31f8 bits 2 to 4 are not
; zero, may drop an item: a random number under the limit in a table at overlay address $AC56
; (indexed by those bits minus one) picks the chance; the item comes from a table at $AC5E,
; indexed by the player distance sum's low nibble plus the bits minus one times $10; it starts in
; a free slot from $30 at the object's place. Every stage overlay has its own copy and tables.
; HOW: NextRandom, PlayerDistanceSum, FindFreeSlotFrom30, StartObjectAtPlace.
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
    lda a:OBJ_31F8,x
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
    lda a:OBJ_31F8,x
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
