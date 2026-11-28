/*
 * Maria's objects (her overlay, bank $6E): the frame's run of them, her companions in slots 1 and 2 and then every
 * object of slots 3 to 15 by its kind (MariaObjectHandlers, 25 kinds); and two pieces of her object code that nothing
 * is known to reach.
 *
 * These routines hold the processor (maria_objects_p.h).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"
#include "maria_objects_p.h"

enum {
    OBJECT_PLAYER = 0, SLOT_FIRST_OBJECT = 3, SLOT_END = 0x10,
    KIND_NONE = 0xC703, KIND_1 = 0xC76D, KIND_2 = 0xC94B, KIND_3 = 0xCF3B, KIND_4 = 0xCD09, KIND_5 = 0xC8DF,
    KIND_6 = 0xD00A, KIND_7 = 0xCC97, KIND_8 = 0xD182, KIND_9 = 0xCD2B, KIND_10 = 0xCE71, KIND_11 = 0xD9B3,
    KIND_12 = 0xCDB0, KIND_13 = 0xD3D3, KIND_14 = 0xD273, KIND_15 = 0xD77D, KIND_16 = 0xD5FD, KIND_17 = 0xDBDD,
    KIND_18 = 0xDA25, KIND_19 = 0xDA7D, KIND_20 = 0xD107, KIND_21 = 0xD5C0, KIND_22 = 0xD71B, KIND_23 = 0xDC64,
    KIND_24 = 0xDD13
};

#define MariaObjectHandlers_TBL MariaObjectHandlers

/* The handlers, each in its kind's file. */
void MariaRunCompanions(void);                                       /* object/maria_objects_slots.c */
void MariaAttackObject(void);                                        /* object/maria_objects_attack.c */
void MariaItem5(void);                                               /* object/maria_objects_attack.c */
void MariaKind2(void);                                               /* object/maria_objects_kind2.c */
void MariaItem3(void);                                               /* object/maria_objects_items2.c */
void MariaItem6(void);                                               /* object/maria_objects_items2.c */
void MariaItem8(void);                                               /* object/maria_objects_items2.c */
void MariaItem6Piece(void);                                          /* object/maria_objects_items2.c */
void MariaItem4Object(void);                                         /* object/maria_objects_items.c */
void MariaItem7Part(void);                                           /* object/maria_objects_items.c */
void MariaObjectKind9(void);                                         /* object/maria_objects_items.c */
void MariaSequenceObject(void);                                      /* object/maria_objects_kind10.c */
void MariaItem4Extra(void);                                          /* object/maria_objects_kind10.c */
void MariaCrash0Object(void);                                        /* object/maria_objects_crash0.c */
void MariaCrash0Piece(void);                                         /* object/maria_objects_crash0.c */
void MariaCrash1Object(void);                                        /* object/maria_objects_crash.c */
void MariaKind11(void);                                              /* object/maria_objects_kind19.c */
void MariaKind18(void);                                              /* object/maria_objects_kind19.c */
void MariaKind19(void);                                              /* object/maria_objects_kind19.c */
void MariaKind15(void);                                              /* object/maria_objects_kind15.c */
void MariaKind16(void);                                              /* object/maria_objects_kind15.c */
void MariaKind22(void);                                              /* object/maria_objects_kind15.c */
void MariaKind17(void);                                              /* object/maria_objects_kind23.c */
void MariaKind23(void);                                              /* object/maria_objects_kind23.c */
void MariaKind24(void);                                              /* object/maria_objects_kind23.c */

/*
 * @routine 01E1:731 MariaRunObjectSlot
 * @run 6E:C731
 * WHAT IT DOES: Runs one object: zRichterObject its slot, then its kind's handler from MariaObjectHandlers (kind 0
 * ends at once).
 * HOW: OBJ_KIND doubled, the JMP (table,X).
 * WAITS: no.
 * Entry: X the slot. Exit: the processor as the handler left it.
 */
void MariaRunObjectSlot(void)
{
    uint8_t kind, lo, hi;
    uint16_t target;

    WR8(ZP->richter_object, cpu.x);
    kind = RD8(OBJ->kind[cpu.x]);
    p_lda(kind);
    p_asl();
    p_ldx(cpu.a);
    lo = TBL8(MariaObjectHandlers, cpu.x);
    hi = TBL8(MariaObjectHandlers, cpu.x + 1);
    target = (uint16_t)(lo | (hi << 8));
    switch (target) {
    case KIND_NONE:
        cpu.x = MariaObjectSlot();
        abi_nz(cpu.x);
        break;
    case KIND_1:
        MariaAttackObject();
        break;
    case KIND_2:
        MariaKind2();
        break;
    case KIND_3:
        MariaItem3();
        break;
    case KIND_4:
        MariaItem4Object();
        break;
    case KIND_5:
        MariaItem5();
        break;
    case KIND_6:
        MariaItem6();
        break;
    case KIND_7:
        MariaItem7Part();
        break;
    case KIND_8:
        MariaItem8();
        break;
    case KIND_9:
        MariaObjectKind9();
        break;
    case KIND_10:
        MariaSequenceObject();
        break;
    case KIND_11:
        MariaKind11();
        break;
    case KIND_12:
        MariaItem4Extra();
        break;
    case KIND_13:
        MariaCrash0Object();
        break;
    case KIND_14:
        MariaCrash1Object();
        break;
    case KIND_15:
        MariaKind15();
        break;
    case KIND_16:
        MariaKind16();
        break;
    case KIND_17:
        MariaKind17();
        break;
    case KIND_18:
        MariaKind18();
        break;
    case KIND_19:
        MariaKind19();
        break;
    case KIND_20:
        MariaItem6Piece();
        break;
    case KIND_21:
        MariaCrash0Piece();
        break;
    case KIND_22:
        MariaKind22();
        break;
    case KIND_23:
        MariaKind23();
        break;
    case KIND_24:
        MariaKind24();
        break;
    default:
        p_jump_elsewhere(target);
        break;
    }
}

void abi_MariaRunObjectSlot(void)
{
    MariaRunObjectSlot();
}

/*
 * @routine 01E1:724 MariaRunObjects
 * @run 6E:C724
 * WHAT IT DOES: A frame of her objects: her companions in slots 1 and 2, then the objects of slots 3 to 15.
 * HOW: MariaRunCompanions; a BSR to MariaRunObjectSlot for each slot, X as the handler left it, one up, until $10.
 * WAITS: no.
 * Entry: nothing. Exit: X $10, Z and C set, N clear; A, Y and V as the last handler left them.
 */
void MariaRunObjects(void)
{
    jsr_push(0xC727);
    MariaRunCompanions();
    jsr_pop();
    p_ldx(SLOT_FIRST_OBJECT);
    do {
        jsr_push(0xC72B);
        MariaRunObjectSlot();
        jsr_pop();
        p_ldx((uint8_t)(cpu.x + 1));
        p_cmp(cpu.x, SLOT_END);
    } while (!p_z());
}

void abi_MariaRunObjects(void)
{
    MariaRunObjects();
}

/* ------------------------------------------------------------------------------------------------ unreached */

/*
 * @routine 01E1:385 MariaObjectMarkFromPlayer
 * @run 6E:C385
 * WHAT IT DOES: Marks an object on (sprite attribute bit 4), then gives it her flip and facing. Nothing is known to
 * reach it: the code map follows no path into $C385.
 * HOW: A BSR to MariaObjectMarkOn, then on into MariaObjectFromPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: as MariaObjectFromPlayer.
 */
void MariaObjectMarkFromPlayer(void)
{
    pj_mark_on(0xC387);
    pj_from_player(0);
}

void abi_MariaObjectMarkFromPlayer(void)
{
    MariaObjectMarkFromPlayer();
}

/*
 * @routine 01E1:558 MariaObjectAddPlayerHitbox
 * @run 6E:C558
 * WHAT IT DOES: Moves an object by her hitbox's offsets: her OBJ_HITBOX_DX added to its X and her OBJ_HITBOX_DY to its
 * Y, each sign-extended to 16 bits. Nothing is known to reach it: the code map follows no path into $C558.
 * HOW: zTemp00 the high byte ($FF for a negative offset), the 16-bit adds.
 * WAITS: no.
 * Entry: X the object. Exit: A its Y's high byte, N, Z, C and V from the add. X and Y kept.
 */
void MariaObjectAddPlayerHitbox(void)
{
    WR8(ZP->temp00, 0);
    p_lda(RD8(OBJ->hitbox_dx[OBJECT_PLAYER]));
    if (p_n()) {
        abi_nz(DEC8(ZP->temp00));
    }
    abi_flag(P_C, false);
    p_adc(RD8(OBJ->x_lo[cpu.x]));
    WR8(OBJ->x_lo[cpu.x], cpu.a);
    p_lda(RD8(ZP->temp00));
    p_adc(RD8(OBJ->x_hi[cpu.x]));
    WR8(OBJ->x_hi[cpu.x], cpu.a);
    WR8(ZP->temp00, 0);
    p_lda(RD8(OBJ->hitbox_dy[OBJECT_PLAYER]));
    if (p_n()) {
        abi_nz(DEC8(ZP->temp00));
    }
    abi_flag(P_C, false);
    p_adc(RD8(OBJ->y_lo[cpu.x]));
    WR8(OBJ->y_lo[cpu.x], cpu.a);
    p_lda(RD8(ZP->temp00));
    p_adc(RD8(OBJ->y_hi[cpu.x]));
    WR8(OBJ->y_hi[cpu.x], cpu.a);
}

void abi_MariaObjectAddPlayerHitbox(void)
{
    MariaObjectAddPlayerHitbox();
}
