; @routine 0519:2A3 ScoreForKind0519
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0519: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC2A gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0519:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0519_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0519,y
    sta z:zTemp00
    lda a:KindScores0519+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0519_at0
    jsr GiveExtraLife
ScoreForKind0519_at0:
    rts

; @routine 0519:2C5 DropItemWhenHit0519
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 0519: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $ABFB and the item from $AC03.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit0519:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit0519_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit0519_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances0519,y
    bcs DropItemWhenHit0519_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems0519,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit0519_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit0519_ret:
    rts

; @routine 05C9:2A3 ScoreForKind05C9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 05C9: with HitAttackFlags bit 0 clear, adds the score its table at
; $ABEE gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind05C9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind05C9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores05C9,y
    sta z:zTemp00
    lda a:KindScores05C9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind05C9_at0
    jsr GiveExtraLife
ScoreForKind05C9_at0:
    rts

; @routine 05C9:2C5 DropItemWhenHit05C9
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 05C9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $ABBF and the item from $ABC7.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit05C9:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit05C9_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit05C9_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances05C9,y
    bcs DropItemWhenHit05C9_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems05C9,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit05C9_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit05C9_ret:
    rts

; @routine 05C9:2FE PlayHitSound05C9
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 05C9: plays the sound its table at $ABE7 gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound05C9:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound05C9_at0
    dec a
    tay
    lda a:HitSounds05C9,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound05C9_at0:
    rts

; @routine 0729:2A3 ScoreForKind0729
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0729: with HitAttackFlags bit 0 clear, adds the score its table at
; $ABF6 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0729:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0729_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0729,y
    sta z:zTemp00
    lda a:KindScores0729+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0729_at0
    jsr GiveExtraLife
ScoreForKind0729_at0:
    rts

; @routine 0729:2C5 DropItemWhenHit0729
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 0729: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $ABC7 and the item from $ABCF.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit0729:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit0729_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit0729_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances0729,y
    bcs DropItemWhenHit0729_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems0729,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit0729_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit0729_ret:
    rts

; @routine 0729:2FE PlayHitSound0729
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 0729: plays the sound its table at $ABEF gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound0729:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound0729_at0
    dec a
    tay
    lda a:HitSounds0729,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound0729_at0:
    rts

; @routine 07D9:2A3 ScoreForKind07D9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 07D9: with HitAttackFlags bit 0 clear, adds the score its table at
; $AB7C gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind07D9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind07D9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores07D9,y
    sta z:zTemp00
    lda a:KindScores07D9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind07D9_at0
    jsr GiveExtraLife
ScoreForKind07D9_at0:
    rts

; @routine 0889:2A3 ScoreForKind0889
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0889: with HitAttackFlags bit 0 clear, adds the score its table at
; $AAE3 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0889:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0889_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0889,y
    sta z:zTemp00
    lda a:KindScores0889+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0889_at0
    jsr GiveExtraLife
ScoreForKind0889_at0:
    rts

; @routine 0889:2C5 DropItemWhenHit0889
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 0889: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $AAC4 and the item from $AACC.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit0889:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit0889_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit0889_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances0889,y
    bcs DropItemWhenHit0889_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems0889,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit0889_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit0889_ret:
    rts

; @routine 0889:2FE PlayHitSound0889
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 0889: plays the sound its table at $AADC gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound0889:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound0889_at0
    dec a
    tay
    lda a:HitSounds0889,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound0889_at0:
    rts

; @routine 0939:2A3 ScoreForKind0939
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0939: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC3C gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0939:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0939_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0939,y
    sta z:zTemp00
    lda a:KindScores0939+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0939_at0
    jsr GiveExtraLife
ScoreForKind0939_at0:
    rts

; @routine 09E9:2A3 ScoreForKind09E9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 09E9: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC1E gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind09E9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind09E9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores09E9,y
    sta z:zTemp00
    lda a:KindScores09E9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind09E9_at0
    jsr GiveExtraLife
ScoreForKind09E9_at0:
    rts

; @routine 0A99:2A3 ScoreForKind0A99
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0A99: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC83 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0A99:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0A99_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0A99,y
    sta z:zTemp00
    lda a:KindScores0A99+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0A99_at0
    jsr GiveExtraLife
ScoreForKind0A99_at0:
    rts

; @routine 0B49:2A3 ScoreForKind0B49
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0B49: with HitAttackFlags bit 0 clear, adds the score its table at
; $AAE7 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0B49:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0B49_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0B49,y
    sta z:zTemp00
    lda a:KindScores0B49+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0B49_at0
    jsr GiveExtraLife
ScoreForKind0B49_at0:
    rts

; @routine 0B49:2C5 DropItemWhenHit0B49
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 0B49: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $AAC8 and the item from $AAD0.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit0B49:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit0B49_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit0B49_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances0B49,y
    bcs DropItemWhenHit0B49_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems0B49,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit0B49_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit0B49_ret:
    rts

; @routine 0B49:2FE PlayHitSound0B49
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 0B49: plays the sound its table at $AAE0 gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound0B49:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound0B49_at0
    dec a
    tay
    lda a:HitSounds0B49,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound0B49_at0:
    rts

; @routine 0BF9:2A3 ScoreForKind0BF9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0BF9: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC47 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0BF9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0BF9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0BF9,y
    sta z:zTemp00
    lda a:KindScores0BF9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0BF9_at0
    jsr GiveExtraLife
ScoreForKind0BF9_at0:
    rts

; @routine 0CA9:2A3 ScoreForKind0CA9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0CA9: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC6B gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0CA9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0CA9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0CA9,y
    sta z:zTemp00
    lda a:KindScores0CA9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0CA9_at0
    jsr GiveExtraLife
ScoreForKind0CA9_at0:
    rts

; @routine 0D59:2A3 ScoreForKind0D59
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0D59: with HitAttackFlags bit 0 clear, adds the score its table at
; $AD11 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0D59:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0D59_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0D59,y
    sta z:zTemp00
    lda a:KindScores0D59+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0D59_at0
    jsr GiveExtraLife
ScoreForKind0D59_at0:
    rts

; @routine 0E09:2A3 ScoreForKind0E09
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0E09: with HitAttackFlags bit 0 clear, adds the score its table at
; $AB50 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0E09:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0E09_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0E09,y
    sta z:zTemp00
    lda a:KindScores0E09+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0E09_at0
    jsr GiveExtraLife
ScoreForKind0E09_at0:
    rts

; @routine 0E09:2C5 DropItemWhenHit0E09
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 0E09: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $AB31 and the item from $AB39.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit0E09:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit0E09_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit0E09_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances0E09,y
    bcs DropItemWhenHit0E09_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems0E09,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit0E09_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit0E09_ret:
    rts

; @routine 0E09:2FE PlayHitSound0E09
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 0E09: plays the sound its table at $AB49 gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound0E09:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound0E09_at0
    dec a
    tay
    lda a:HitSounds0E09,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound0E09_at0:
    rts

; @routine 0EB9:2A3 ScoreForKind0EB9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0EB9: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC9F gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0EB9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0EB9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0EB9,y
    sta z:zTemp00
    lda a:KindScores0EB9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0EB9_at0
    jsr GiveExtraLife
ScoreForKind0EB9_at0:
    rts

; @routine 0F69:2A3 ScoreForKind0F69
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 0F69: with HitAttackFlags bit 0 clear, adds the score its table at
; $AB07 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind0F69:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind0F69_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores0F69,y
    sta z:zTemp00
    lda a:KindScores0F69+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind0F69_at0
    jsr GiveExtraLife
ScoreForKind0F69_at0:
    rts

; @routine 0F69:2C5 DropItemWhenHit0F69
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 0F69: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $AAD8 and the item from $AAE0.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit0F69:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit0F69_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit0F69_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances0F69,y
    bcs DropItemWhenHit0F69_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems0F69,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit0F69_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit0F69_ret:
    rts

; @routine 0F69:2FE PlayHitSound0F69
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 0F69: plays the sound its table at $AB00 gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound0F69:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound0F69_at0
    dec a
    tay
    lda a:HitSounds0F69,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound0F69_at0:
    rts

; @routine 1019:2A3 ScoreForKind1019
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 1019: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC7B gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind1019:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind1019_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores1019,y
    sta z:zTemp00
    lda a:KindScores1019+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind1019_at0
    jsr GiveExtraLife
ScoreForKind1019_at0:
    rts

; @routine 10C9:2A3 ScoreForKind10C9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 10C9: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC29 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind10C9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind10C9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores10C9,y
    sta z:zTemp00
    lda a:KindScores10C9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind10C9_at0
    jsr GiveExtraLife
ScoreForKind10C9_at0:
    rts

; @routine 10C9:2C5 DropItemWhenHit10C9
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 10C9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $ABFA and the item from $AC02.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit10C9:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit10C9_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit10C9_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances10C9,y
    bcs DropItemWhenHit10C9_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems10C9,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit10C9_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit10C9_ret:
    rts

; @routine 10C9:2FE PlayHitSound10C9
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 10C9: plays the sound its table at $AC22 gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound10C9:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound10C9_at0
    dec a
    tay
    lda a:HitSounds10C9,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound10C9_at0:
    rts

; @routine 1179:2A3 ScoreForKind1179
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 1179: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC96 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind1179:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind1179_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores1179,y
    sta z:zTemp00
    lda a:KindScores1179+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind1179_at0
    jsr GiveExtraLife
ScoreForKind1179_at0:
    rts

; @routine 12D9:2A3 ScoreForKind12D9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 12D9: with HitAttackFlags bit 0 clear, adds the score its table at
; $ACB7 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind12D9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind12D9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores12D9,y
    sta z:zTemp00
    lda a:KindScores12D9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind12D9_at0
    jsr GiveExtraLife
ScoreForKind12D9_at0:
    rts

; @routine 12D9:2C5 DropItemWhenHit12D9
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 12D9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $AC88 and the item from $AC90.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit12D9:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit12D9_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit12D9_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances12D9,y
    bcs DropItemWhenHit12D9_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems12D9,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit12D9_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit12D9_ret:
    rts

; @routine 12D9:2FE PlayHitSound12D9
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 12D9: plays the sound its table at $ACB0 gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound12D9:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound12D9_at0
    dec a
    tay
    lda a:HitSounds12D9,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound12D9_at0:
    rts

; @routine 13F9:2A3 ScoreForKind13F9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 13F9: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC97 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind13F9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind13F9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores13F9,y
    sta z:zTemp00
    lda a:KindScores13F9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind13F9_at0
    jsr GiveExtraLife
ScoreForKind13F9_at0:
    rts

; @routine 13F9:2C5 DropItemWhenHit13F9
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 13F9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $AC68 and the item from $AC70.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit13F9:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit13F9_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit13F9_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances13F9,y
    bcs DropItemWhenHit13F9_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems13F9,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit13F9_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit13F9_ret:
    rts

; @routine 13F9:2FE PlayHitSound13F9
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 13F9: plays the sound its table at $AC90 gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound13F9:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound13F9_at0
    dec a
    tay
    lda a:HitSounds13F9,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound13F9_at0:
    rts

; @routine 1619:2A3 ScoreForKind1619
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 1619: with HitAttackFlags bit 0 clear, adds the score its table at
; $AC91 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind1619:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind1619_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores1619,y
    sta z:zTemp00
    lda a:KindScores1619+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind1619_at0
    jsr GiveExtraLife
ScoreForKind1619_at0:
    rts

; @routine 1619:2C5 DropItemWhenHit1619
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 1619: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $AC62 and the item from $AC6A.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit1619:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit1619_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit1619_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances1619,y
    bcs DropItemWhenHit1619_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems1619,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit1619_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit1619_ret:
    rts

; @routine 1619:2FE PlayHitSound1619
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 1619: plays the sound its table at $AC8A gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound1619:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound1619_at0
    dec a
    tay
    lda a:HitSounds1619,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound1619_at0:
    rts

; @routine 16C9:2A3 ScoreForKind16C9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 16C9: with HitAttackFlags bit 0 clear, adds the score its table at
; $ABC5 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind16C9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind16C9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores16C9,y
    sta z:zTemp00
    lda a:KindScores16C9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind16C9_at0
    jsr GiveExtraLife
ScoreForKind16C9_at0:
    rts

; @routine 16C9:2C5 DropItemWhenHit16C9
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 16C9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $AB96 and the item from $AB9E.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit16C9:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit16C9_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit16C9_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances16C9,y
    bcs DropItemWhenHit16C9_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems16C9,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit16C9_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit16C9_ret:
    rts

; @routine 18D9:2A3 ScoreForKind18D9
; @run 70:A2A3
; WHAT IT DOES: ScoreForKind in the overlay at sector 18D9: with HitAttackFlags bit 0 clear, adds the score its table at
; $ABA9 gives for the object's kind less $20, and the extra life when that carries over the mark.
; HOW: score_for_kind_at with this overlay's table.
; WAITS: no.
; Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
ScoreForKind18D9:
    lda a:HitAttackFlags
    and #$01
    bne ScoreForKind18D9_at0
    lda a:OBJ_2750,x
    sec
    sbc #$20
    asl a
    tay
    lda a:KindScores18D9,y
    sta z:zTemp00
    lda a:KindScores18D9+1,y
    sta z:zTemp01
    jsr VecAddScoreWord
    bcc ScoreForKind18D9_at0
    jsr GiveExtraLife
ScoreForKind18D9_at0:
    rts

; @routine 18D9:2C5 DropItemWhenHit18D9
; @run 70:A2C5
; WHAT IT DOES: DropItemWhenHit in the overlay at sector 18D9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
; an item, the chance under a limit from $AB7A and the item from $AB82.
; HOW: drop_item_when_hit with this overlay's tables.
; WAITS: no.
; Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
DropItemWhenHit18D9:
    lda a:OBJ_33B0,x
    cmp #$05
    bcc DropItemWhenHit18D9_ret
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    and #$07
    beq DropItemWhenHit18D9_ret
    dec a
    tay
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    jsr VecNextRandom
    cmp a:DropChances18D9,y
    bcs DropItemWhenHit18D9_ret
    jsr VecPlayerDistanceSum
    and #$0F
    clc
    adc z:zTemp00
    tay
    lda a:DropItems18D9,y
    sta z:zTemp00
    jsr VecFindFreeSlotFrom30
    bcc DropItemWhenHit18D9_ret
    lda z:zTemp00
    jsr VecStartObjectAtPlace
DropItemWhenHit18D9_ret:
    rts

; @routine 18D9:2FE PlayHitSound18D9
; @run 70:A2FE
; WHAT IT DOES: PlayHitSound in the overlay at sector 18D9: plays the sound its table at $ABA2 gives for bits 5 to 7
; of the object's f31f8, when they are not zero and the object is on the first screen.
; HOW: sound_from_hit_code with this overlay's table.
; WAITS: no.
; Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
PlayHitSound18D9:
    lda a:OBJ_31F8,x
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a
    and #$07
    beq PlayHitSound18D9_at0
    dec a
    tay
    lda a:HitSounds18D9,y
    jsr VecSoundIfOnFirstScreen
PlayHitSound18D9_at0:
    rts
