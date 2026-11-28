/*
 * Maria's objects (her overlay, bank $6E): item 4's object (kind 4, slot 5), item 7's part (kind 7, which follows
 * the attack's object in its OBJ_WORK_D), and the four pieces of kind 9 in slots 5 to 8 (MariaClearSlot5AsKind9
 * starts the first).
 *
 * These routines hold the processor (see maria_objects_slots.c).
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"
#include "maria_objects.h"

enum {
    OBJECT_PLAYER = 0, SLOT_FIRST_PIECE = 5, SLOT_LAST_PIECE = 8, KIND_PIECE = 9,
    PART_SPRITE_SET = 3, PART_FRAME = 0x13, PART_DOWN = 0x07, PART_HITBOX_DY = 0x06, PART_HALF = 0x08,
    PART_WORK_N = 0x0C,
    ITEM4_HALF_H = 0x10, ITEM4_HALF_W = 0x04, ITEM4_HITBOX_DY = 0x0E, ITEM4_WORK_N = 0x0D, PLAYER_LOW_SCREEN = 1,
    PIECE_TIMER = 0x20, PIECE_FIRST_FRAME = 0x0D, PIECE_SPRITE_SET = 1, PIECE_FALL = 0x38,
    KIND12_FALL_LO = 0x38, KIND12_ANIMATION = 7
};

#define MariaPieceSpeedsX_TBL MariaPieceSpeedsX

Sum RichterObjectSetFall(uint8_t object, uint8_t low, bool carry); /* object/richter_objects.c */
void MariaObjectAtPlayerPlace(void);                                /* object/maria_objects_slots.c */

static void p_lda(uint8_t value)
{
    cpu.a = value;
    abi_nz(value);
}

static void p_sum(Sum s)
{
    p_lda(s.value);
    abi_flag(P_C, s.carry);
    abi_flag(P_V, s.overflow);
}

static void p_cmp(uint8_t reg, uint8_t operand)
{
    abi_nz((uint8_t)(reg - operand));
    abi_flag(P_C, reg >= operand);
}

static uint8_t p_ldx_object(void)
{
    cpu.x = RD8(ZP->richter_object);
    abi_nz(cpu.x);
    return cpu.x;
}

/*
 * @routine 01E1:31D MariaObjectSetFall
 * @run 6E:C31D
 * WHAT IT DOES: RichterObjectSetFall at her address, the same bytes: an object's Y speed low byte A, its high byte
 * plus the carry.
 * HOW: RichterObjectSetFall's body.
 * WAITS: no.
 * Entry: A the low byte, C the carry, X the object. Exit: A the high byte, N and Z from it, C and V from its add.
 * X and Y kept.
 */
void MariaObjectSetFall(void)
{
    p_sum(RichterObjectSetFall(cpu.x, cpu.a, (cpu.p & P_C) != 0));
}

void abi_MariaObjectSetFall(void)
{
    MariaObjectSetFall();
}

/*
 * @routine 01E2:497 MariaItem7Part
 * @run 6E:CC97
 * WHAT IT DOES: The handler of kind 7, item 7's part: while the attack's object in its OBJ_WORK_D is out, sits 7 rows
 * below it, as it faces and flips, sprite set 3 frame $13, an 8 by 8 hitbox 6 down, live, OBJ_WORK_N $0C. When the
 * attack's object is gone it makes sure her kind-3 companion is out (MariaClaimKind3Slot) and is cleared. With
 * PlayerKind7Caught set it is cleared at once, and zField93 (the item in hand) with it; PlayerKind7Caught is cleared.
 * HOW: The attack's slot in Y; the copies; MariaClaimKind3Slot; the tail JMPs to ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaItem7Part(void)
{
    uint8_t object = p_ldx_object();
    uint8_t attack = RD8(OBJ->work_d[object]);
    uint8_t value;
    Sum s;

    cpu.y = attack;
    abi_nz(attack);
    value = RD8_IDX(COL(OBJ->kind), attack);
    p_lda(value);
    if (value == 0) {
        value = RD8(PVARS2->kind7_caught);
        p_lda(value);
        if (value == 0) {
            jsr_push(0xCCFB);
            regs_exit(MariaClaimKind3Slot(object));
            jsr_pop();
            WR8(PVARS2->kind7_caught, 0);
            ClearObject(cpu.x);
            return;
        }
        WR8(PVARS2->kind7_caught, 0);
        WR8(ZP->field93, 0);
        ClearObject(cpu.x);
        return;
    }
    p_lda(PART_SPRITE_SET);
    WR8(OBJ->sprite_set[object], PART_SPRITE_SET);
    p_lda(PART_FRAME);
    WR8(OBJ->frame[object], PART_FRAME);
    value = RD8_IDX(COL(OBJ->x_lo), attack);
    p_lda(value);
    WR8(OBJ->x_lo[object], value);
    value = RD8_IDX(COL(OBJ->x_hi), attack);
    p_lda(value);
    WR8(OBJ->x_hi[object], value);
    value = RD8_IDX(COL(OBJ->y_lo), attack);
    s = add_bytes(value, PART_DOWN, false);
    p_sum(s);
    WR8(OBJ->y_lo[object], s.value);
    value = RD8_IDX(COL(OBJ->y_hi), attack);
    s = add_bytes(value, 0, s.carry);
    p_sum(s);
    WR8(OBJ->y_hi[object], s.value);
    value = RD8_IDX(COL(OBJ->sprite_flip), attack);
    p_lda(value);
    WR8(OBJ->sprite_flip[object], value);
    value = RD8_IDX(COL(OBJ->facing_left), attack);
    p_lda(value);
    WR8(OBJ->facing_left[object], value);
    p_lda(PART_HITBOX_DY);
    WR8(OBJ->hitbox_dy[object], PART_HITBOX_DY);
    p_lda(PART_HALF);
    WR8(OBJ->hitbox_half_h[object], PART_HALF);
    p_lda(PART_HALF);
    WR8(OBJ->hitbox_half_w[object], PART_HALF);
    p_lda(1);
    WR8(OBJ->hitbox_on[object], 1);
    p_lda(PART_WORK_N);
    WR8(OBJ->work_n[object], PART_WORK_N);
    value = RD8(PVARS2->kind7_caught);
    p_lda(value);
    if (value == 0) {
        return;
    }
    WR8(PVARS2->kind7_caught, 0);
    WR8(ZP->field93, 0);
    ClearObject(cpu.x);
}

void abi_MariaItem7Part(void)
{
    MariaItem7Part();
}

/*
 * @routine 01E2:509 MariaItem4Object
 * @run 6E:CD09
 * WHAT IT DOES: The handler of kind 4, item 4's object: where she is, with a hitbox $10 high, 4 wide and $0E down,
 * OBJ_WORK_N $0D; cleared once her Y's high byte is 1 (she is below the first screen).
 * HOW: The tail JMPs to MariaObjectAtPlayerPlace and ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: as MariaObjectAtPlayerPlace, or with the processor as the clear found it.
 */
void MariaItem4Object(void)
{
    uint8_t object = p_ldx_object();
    uint8_t value = RD8(OBJ->y_hi[OBJECT_PLAYER]);

    p_lda((uint8_t)(value - 1));
    if (value == PLAYER_LOW_SCREEN) {
        ClearObject(object);
        return;
    }
    p_lda(ITEM4_HALF_H);
    WR8(OBJ->hitbox_half_h[object], ITEM4_HALF_H);
    p_lda(ITEM4_HALF_W);
    WR8(OBJ->hitbox_half_w[object], ITEM4_HALF_W);
    p_lda(ITEM4_HITBOX_DY);
    WR8(OBJ->hitbox_dy[object], ITEM4_HITBOX_DY);
    p_lda(ITEM4_WORK_N);
    WR8(OBJ->work_n[object], ITEM4_WORK_N);
    MariaObjectAtPlayerPlace();
}

void abi_MariaItem4Object(void)
{
    MariaItem4Object();
}

/*
 * @routine 01E2:52B MariaObjectKind9
 * @run 6E:CD2B
 * WHAT IT DOES: The handler of kind 9, four pieces in slots 5 to 8. State 0: the piece in slot 5 makes slots 6 to 8
 * pieces too; each starts where she is, from her, facing right, with its X speed by slot (MariaPieceSpeedsX), the
 * even slots also Y speed entry 0, its animation timer $20, frame $0D on by slot, sprite set 1, state 1. State 1:
 * moves, falls by $38 a frame, and is cleared when its timer runs out.
 * HOW: ClearObject for the new slots; MariaObjectFromPlayer, MariaObjectAtPlayerPlace, MariaSetVelocityYFromTable;
 * MariaObjectMove, MariaObjectSetFall; the tail JMPs to MariaObjectNextState and ClearObject.
 * WAITS: no.
 * Entry: zRichterObject the slot. Exit: the processor as the last step left it.
 */
void MariaObjectKind9(void)
{
    uint8_t object = p_ldx_object();
    uint8_t value, index;
    ObjectFromPlayer from;
    Sum s;

    value = RD8(OBJ->state[object]);
    p_lda(value);
    if (value != 0) {
        jsr_push(0xCD84);
        abi_ended(ended_of_axis_move(MariaObjectMove(object)));
        jsr_pop();
        value = RD8(OBJ->vy_lo[object]);
        s = add_bytes(value, PIECE_FALL, false);
        p_sum(s);
        jsr_push(0xCD8D);
        MariaObjectSetFall();
        jsr_pop();
        value = DEC8(OBJ->hurt_timer[object]);
        abi_nz(value);
        if (value == 0) {
            ClearObject(object);
        }
        return;
    }
    p_cmp(object, SLOT_FIRST_PIECE);
    if (object == SLOT_FIRST_PIECE) {
        push8(object);
        do {
            cpu.x = (uint8_t)(cpu.x + 1);
            abi_nz(cpu.x);
            jsr_push(0xCD3B);
            ClearObject(cpu.x);
            jsr_pop();
            p_lda(KIND_PIECE);
            WR8(OBJ->kind[cpu.x], KIND_PIECE);
            p_cmp(cpu.x, SLOT_LAST_PIECE);
        } while (cpu.x != SLOT_LAST_PIECE);
        cpu.x = pull8();
        abi_nz(cpu.x);
    }
    jsr_push(0xCD48);
    from = MariaObjectFromPlayer(object);
    jsr_pop();
    p_lda(from.facing);
    abi_flag(P_C, (from.player_2fe8 & 0x80) != 0);
    WR8(OBJ->facing_left[object], 0);
    jsr_push(0xCD4E);
    MariaObjectAtPlayerPlace();
    jsr_pop();
    p_lda(object);
    s = sub_bytes(object, SLOT_FIRST_PIECE, true);
    p_sum(s);
    p_lda((uint8_t)(s.value << 1));
    abi_flag(P_C, (s.value & 0x80) != 0);
    index = cpu.a;
    cpu.y = index;
    value = TBL8(MariaPieceSpeedsX, index);
    p_lda(value);
    WR8(OBJ->vx_lo[object], value);
    value = TBL8(MariaPieceSpeedsX, index + 1);
    p_lda(value);
    WR8(OBJ->vx_hi[object], value);
    p_lda((uint8_t)(object & 0x01));
    if (cpu.a == 0) {
        p_lda(0);
        jsr_push(0xCD6A);
        regs_exit(regs_of_velocity_table(MariaSetVelocityYFromTable(object, 0)));
        jsr_pop();
    }
    p_lda(PIECE_TIMER);
    WR8(OBJ->hurt_timer[object], PIECE_TIMER);
    s = sub_bytes(object, SLOT_FIRST_PIECE, true);
    p_sum(s);
    s = add_bytes(s.value, PIECE_FIRST_FRAME, false);
    p_sum(s);
    WR8(OBJ->frame[object], s.value);
    p_lda(PIECE_SPRITE_SET);
    WR8(OBJ->sprite_set[object], PIECE_SPRITE_SET);
    abi_nz(MariaObjectNextState(object));
}

void abi_MariaObjectKind9(void)
{
    MariaObjectKind9();
}

/*
 * @routine 01E2:59E MariaObjectClearAfterSpeeds
 * @run 6E:CD9E
 * WHAT IT DOES: Empties the object's slot. Nothing is known to reach it: it sits after MariaPieceSpeedsX and no call,
 * jump or table found goes to it.
 * HOW: The tail JMP to ClearObject (the kernel's vector at $4054).
 * WAITS: no.
 * Entry: X the object. Exit: as ClearObject: everything kept.
 */
void MariaObjectClearAfterSpeeds(void)
{
    ClearObject(cpu.x);
}

void abi_MariaObjectClearAfterSpeeds(void)
{
    MariaObjectClearAfterSpeeds();
}

/*
 * @routine 01E2:5A1 MariaObjectFallStart7
 * @run 6E:CDA1
 * WHAT IT DOES: An object falls by $0038 a frame, with animation 7 and its next state.
 * HOW: The stores, then the tail JMP to MariaObjectStartAnimation.
 * WAITS: no.
 * Entry: X the object. Exit: as MariaObjectStartAnimation.
 */
void MariaObjectFallStart7(void)
{
    uint8_t object = cpu.x;

    p_lda(KIND12_FALL_LO);
    WR8(OBJ->fall_lo[object], KIND12_FALL_LO);
    p_lda(0);
    WR8(OBJ->fall_hi[object], 0);
    p_lda(KIND12_ANIMATION);
    mx_exit(MariaObjectStartAnimation(object, KIND12_ANIMATION));
}

void abi_MariaObjectFallStart7(void)
{
    MariaObjectFallStart7();
}
