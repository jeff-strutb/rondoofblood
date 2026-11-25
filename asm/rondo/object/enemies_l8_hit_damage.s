; @routine 0519:28D HitDamage0519
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABDB: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0519:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$ABDB,y
    sta z:zTemp00
    rts

; @routine 05C9:28D HitDamage05C9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB8F: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage05C9:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AB8F,y
    sta z:zTemp00
    rts

; @routine 0729:28D HitDamage0729
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB97: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0729:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AB97,y
    sta z:zTemp00
    rts

; @routine 07D9:28D HitDamage07D9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB0D: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage07D9:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AB0D,y
    sta z:zTemp00
    rts

; @routine 0889:28D HitDamage0889
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AA94: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0889:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AA94,y
    sta z:zTemp00
    rts

; @routine 0939:28D HitDamage0939
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABCD: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0939:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$ABCD,y
    sta z:zTemp00
    rts

; @routine 0A99:28D HitDamage0A99
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC14: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0A99:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AC14,y
    sta z:zTemp00
    rts

; @routine 0B49:28D HitDamage0B49
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AA98: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0B49:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AA98,y
    sta z:zTemp00
    rts

; @routine 0BF9:28D HitDamage0BF9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABE8: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0BF9:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$ABE8,y
    sta z:zTemp00
    rts

; @routine 0CA9:28D HitDamage0CA9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABFC: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0CA9:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$ABFC,y
    sta z:zTemp00
    rts

; @routine 0D59:28D HitDamage0D59
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ACA2: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0D59:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$ACA2,y
    sta z:zTemp00
    rts

; @routine 0E09:28D HitDamage0E09
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB01: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0E09:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AB01,y
    sta z:zTemp00
    rts

; @routine 0EB9:28D HitDamage0EB9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC50: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage0EB9:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AC50,y
    sta z:zTemp00
    rts

; @routine 1019:28D HitDamage1019
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC0C: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage1019:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AC0C,y
    sta z:zTemp00
    rts

; @routine 10C9:28D HitDamage10C9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABAB: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage10C9:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$ABAB,y
    sta z:zTemp00
    rts

; @routine 1179:28D HitDamage1179
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC16: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage1179:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AC16,y
    sta z:zTemp00
    rts

; @routine 16C9:28D HitDamage16C9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB66: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamage16C9:
    lda a:OBJ_FIELDS_2EE0,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_33B0,x
    clc
    adc z:zTemp00
    tay
    lda a:$AB66,y
    sta z:zTemp00
    rts
