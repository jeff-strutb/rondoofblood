; @routine 0519:28D HitDamage0519
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABDB: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0519 = $ABDB ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0519:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0519,y
    sta z:zTemp00
    rts

; @routine 05C9:28D HitDamage05C9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB8F: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack05C9 = $AB8F ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage05C9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack05C9,y
    sta z:zTemp00
    rts

; @routine 0729:28D HitDamage0729
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB97: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0729 = $AB97 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0729:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0729,y
    sta z:zTemp00
    rts

; @routine 07D9:28D HitDamage07D9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB0D: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack07D9 = $AB0D ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage07D9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack07D9,y
    sta z:zTemp00
    rts

; @routine 0889:28D HitDamage0889
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AA94: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0889 = $AA94 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0889:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0889,y
    sta z:zTemp00
    rts

; @routine 0939:28D HitDamage0939
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABCD: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0939 = $ABCD ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0939:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0939,y
    sta z:zTemp00
    rts

; @routine 0A99:28D HitDamage0A99
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC14: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0A99 = $AC14 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0A99:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0A99,y
    sta z:zTemp00
    rts

; @routine 0B49:28D HitDamage0B49
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AA98: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0B49 = $AA98 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0B49:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0B49,y
    sta z:zTemp00
    rts

; @routine 0BF9:28D HitDamage0BF9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABE8: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0BF9 = $ABE8 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0BF9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0BF9,y
    sta z:zTemp00
    rts

; @routine 0CA9:28D HitDamage0CA9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABFC: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0CA9 = $ABFC ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0CA9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0CA9,y
    sta z:zTemp00
    rts

; @routine 0D59:28D HitDamage0D59
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ACA2: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0D59 = $ACA2 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0D59:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0D59,y
    sta z:zTemp00
    rts

; @routine 0E09:28D HitDamage0E09
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB01: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0E09 = $AB01 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0E09:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0E09,y
    sta z:zTemp00
    rts

; @routine 0EB9:28D HitDamage0EB9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC50: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0EB9 = $AC50 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0EB9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0EB9,y
    sta z:zTemp00
    rts

; @routine 1019:28D HitDamage1019
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC0C: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack1019 = $AC0C ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage1019:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack1019,y
    sta z:zTemp00
    rts

; @routine 10C9:28D HitDamage10C9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $ABAB: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack10C9 = $ABAB ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage10C9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack10C9,y
    sta z:zTemp00
    rts

; @routine 1179:28D HitDamage1179
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC16: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack1179 = $AC16 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage1179:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack1179,y
    sta z:zTemp00
    rts

; @routine 16C9:28D HitDamage16C9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB66: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack16C9 = $AB66 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage16C9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack16C9,y
    sta z:zTemp00
    rts


; @routine 0F69:28D HitDamage0F69
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AA98: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack0F69 = $AA98 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage0F69:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack0F69,y
    sta z:zTemp00
    rts

; @routine 12D9:28D HitDamage12D9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC28: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack12D9 = $AC28 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage12D9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack12D9,y
    sta z:zTemp00
    rts

; @routine 13F9:28D HitDamage13F9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC38: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack13F9 = $AC38 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage13F9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack13F9,y
    sta z:zTemp00
    rts

; @routine 1619:28D HitDamage1619
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AC22: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack1619 = $AC22 ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage1619:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack1619,y
    sta z:zTemp00
    rts

; @routine 18D9:28D HitDamage18D9
; @run 70:A28D
; WHAT IT DOES: HitDamage with the overlay's own table at $AB3A: the damage the object takes from the hit it has taken, into zTemp00.
; HOW: hit_damage_from.
; WAITS: no.
; Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add.
HitDamageByAttack18D9 = $AB3A ; damage by OBJ_2EE0 times sixteen plus the hit taken
HitDamage18D9:
    lda a:OBJ_WORK_A,x
    asl a
    asl a
    asl a
    asl a
    sta z:zTemp00
    lda a:OBJ_HIT_TAKEN,x
    clc
    adc z:zTemp00
    tay
    lda a:HitDamageByAttack18D9,y
    sta z:zTemp00
    rts

