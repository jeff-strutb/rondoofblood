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
