; @routine 05CA:01D TilePieceObject05CA
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 02), as TilePieceObject072A with this
; overlay's tile piece records at $AE61.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject05CARecords = $AE61 ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject05CA:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject05CA_at0
    jmp VecClearObject
TilePieceObject05CA_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject05CARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject05CARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject05CARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 088A:01D TilePieceObject088A
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 04), as TilePieceObject072A with this
; overlay's tile piece records at $C222.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject088ARecords = $C222 ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject088A:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject088A_at0
    jmp VecClearObject
TilePieceObject088A_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject088ARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject088ARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject088ARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 09EA:01D TilePieceObject09EA
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 05), as TilePieceObject072A with this
; overlay's tile piece records at $BCE2.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject09EARecords = $BCE2 ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject09EA:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject09EA_at0
    jmp VecClearObject
TilePieceObject09EA_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject09EARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject09EARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject09EARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 0B4A:01D TilePieceObject0B4A
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 06), as TilePieceObject072A with this
; overlay's tile piece records at $B36F.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject0B4ARecords = $B36F ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject0B4A:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject0B4A_at0
    jmp VecClearObject
TilePieceObject0B4A_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject0B4ARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject0B4ARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject0B4ARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 0CAA:01D TilePieceObject0CAA
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 07), as TilePieceObject072A with this
; overlay's tile piece records at $AE9D.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject0CAARecords = $AE9D ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject0CAA:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject0CAA_at0
    jmp VecClearObject
TilePieceObject0CAA_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject0CAARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject0CAARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject0CAARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 0E0A:01D TilePieceObject0E0A
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 08), as TilePieceObject072A with this
; overlay's tile piece records at $AEF4.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject0E0ARecords = $AEF4 ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject0E0A:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject0E0A_at0
    jmp VecClearObject
TilePieceObject0E0A_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject0E0ARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject0E0ARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject0E0ARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 0EBA:01D TilePieceObject0EBA
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 09), as TilePieceObject072A with this
; overlay's tile piece records at $AF1D.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject0EBARecords = $AF1D ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject0EBA:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject0EBA_at0
    jmp VecClearObject
TilePieceObject0EBA_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject0EBARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject0EBARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject0EBARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 0F6A:01D TilePieceObject0F6A
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 09), as TilePieceObject072A with this
; overlay's tile piece records at $ADBD.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject0F6ARecords = $ADBD ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject0F6A:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject0F6A_at0
    jmp VecClearObject
TilePieceObject0F6A_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject0F6ARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject0F6ARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject0F6ARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 101A:01D TilePieceObject101A
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 0A), as TilePieceObject072A with this
; overlay's tile piece records at $B017.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject101ARecords = $B017 ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject101A:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject101A_at0
    jmp VecClearObject
TilePieceObject101A_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject101ARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject101ARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject101ARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 117A:01D TilePieceObject117A
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 0B), as TilePieceObject072A with this
; overlay's tile piece records at $B073.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject117ARecords = $B073 ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject117A:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject117A_at0
    jmp VecClearObject
TilePieceObject117A_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject117ARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject117ARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject117ARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 12DA:01D TilePieceObject12DA
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 0C), as TilePieceObject072A with this
; overlay's tile piece records at $AEDF.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject12DARecords = $AEDF ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject12DA:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject12DA_at0
    jmp VecClearObject
TilePieceObject12DA_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject12DARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject12DARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject12DARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 13FA:01D TilePieceObject13FA
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 0C), as TilePieceObject072A with this
; overlay's tile piece records at $AEA7.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject13FARecords = $AEA7 ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject13FA:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject13FA_at0
    jmp VecClearObject
TilePieceObject13FA_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject13FARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject13FARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject13FARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 161A:01D TilePieceObject161A
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 0E), as TilePieceObject072A with this
; overlay's tile piece records at $B0A3.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject161ARecords = $B0A3 ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject161A:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject161A_at0
    jmp VecClearObject
TilePieceObject161A_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject161ARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject161ARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject161ARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 16CA:01D TilePieceObject16CA
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 0E), as TilePieceObject072A with this
; overlay's tile piece records at $AE7D.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject16CARecords = $AE7D ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject16CA:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject16CA_at0
    jmp VecClearObject
TilePieceObject16CA_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject16CARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject16CARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject16CARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts

; @routine 18DA:01D TilePieceObject18DA
; @run 70:A81D
; @stop A855
; WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 10), as TilePieceObject072A with this
; overlay's tile piece records at $AE85.
; HOW: TilePieceObjectAt.
; WAITS: no.
; Entry: X object. Exit: as TilePieceObject072A.
TilePieceObject18DARecords = $AE85 ; 16 bytes per start argument: the hit box's half sizes, then the tile script
TilePieceObject18DA:
    lda a:OBJ_STATE,x
    bne TilePieceInRange072A
    ldy a:OBJ_START_ARG,x
    jsr BitMaskFromIndex
    and z:zField8F
    beq TilePieceObject18DA_at0
    jmp VecClearObject
TilePieceObject18DA_at0:
    jsr VecLoadObjectRecord
    lda a:OBJ_START_ARG,x
    asl a
    asl a
    asl a
    asl a
    sta a:OBJ_WORK_E,x
    tay
    lda a:TilePieceObject18DARecords,y
    sta a:OBJ_HITBOX_HALF_W,x
    lda a:TilePieceObject18DARecords+1,y
    sta a:OBJ_HITBOX_HALF_H,x
    lda a:TilePieceObject18DARecords+2,y
    clc
    adc a:OBJ_WORK_D,x
    cly
    jsr VecDrawTileScript
    rts
