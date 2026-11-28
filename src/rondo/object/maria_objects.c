/*
 * Maria's objects (her overlay's $C000-$DFFF, bank $6E): the small steps her objects' handlers share.
 *
 * The handlers run with X the object's slot (zRichterObject keeps it for them, as it does for Richter's). Slots 1
 * and 2 are her companions, 3 to 15 her attacks and items.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"
#include "maria_objects.h"

enum {
    OBJECT_PLAYER = 0, OBJECT_FIRST_OTHER = 1, OBJECT_END = 0x10, SLOT_COMPANION_1 = 1, SLOT_COMPANION_2 = 2,
    SLOT_SCRIPT = 3,
    KIND_ITEM4 = 4, KIND_ITEM4_EXTRA = 0x0C, KIND_COMPANION = 3, ITEM4_EXTRA_KEPT_STATE = 2,
    MARK_OFF = 0xEF, HIT_SLOT_FIRST = 0x10, HITBOX_HITTING = 2,
    ABOVE_PLAYER = 0xF6, ABOVE_PLAYER_MORE = 0xFD, FRAME_HIGHER = 0x42,
    FRAME_FIRST = 0x45, FRAME_OWN_FIRST = 0x11, FRAME_TABLE_FIRST = 0x53, FRAME_24 = 0x24, FRAME_25 = 0x25,
    EDGE_RIGHT_X = 0x30, EDGE_LEFT_X = 0xD0, SPRITE_SET_OBJECT = 1
};

#define MariaSlotPlaces_TBL MariaSlotPlaces
#define MariaSlotPlaceByState_TBL MariaSlotPlaceByState
#define MariaFrameByPlayerFrame_TBL MariaFrameByPlayerFrame

/*
 * @routine 01E1:30D MariaObjectStopX
 * @run 6E:C30D
 * WHAT IT DOES: RichterObjectStopX at her address, the same bytes: stops an object along X.
 * HOW: RichterObjectStopX's body.
 * WAITS: no.
 * Entry: X the object. Exit: all registers and flags kept.
 */
void MariaObjectStopX(uint8_t object)
{
    RichterObjectStopX(object);
}

void abi_MariaObjectStopX(void)
{
    MariaObjectStopX(cpu.x);
}

/*
 * @routine 01E1:314 MariaObjectStop
 * @run 6E:C314
 * WHAT IT DOES: Stops an object along X and Y.
 * HOW: A BSR to MariaObjectStopX, then on into MariaObjectStopY.
 * WAITS: no.
 * Entry: X the object. Exit: all registers and flags kept.
 */
void MariaObjectStop(uint8_t object)
{
    jsr_push(0xC316);
    MariaObjectStopX(object);
    jsr_pop();
    MariaObjectStopY(object);
}

void abi_MariaObjectStop(void)
{
    MariaObjectStop(cpu.x);
}

/*
 * @routine 01E1:316 MariaObjectStopY
 * @run 6E:C316
 * WHAT IT DOES: RichterObjectStopY at her address, the same bytes: stops an object along Y.
 * HOW: RichterObjectStopY's body.
 * WAITS: no.
 * Entry: X the object. Exit: all registers and flags kept.
 */
void MariaObjectStopY(uint8_t object)
{
    RichterObjectStopY(object);
}

void abi_MariaObjectStopY(void)
{
    MariaObjectStopY(cpu.x);
}

/*
 * @routine 01E1:375 MariaObjectMarkOff
 * @run 6E:C375
 * WHAT IT DOES: Clears bit 4 of an object's sprite attribute (the bit MariaObjectMarkOn sets).
 * HOW: The AND, then a branch to MariaObjectMarkOn's store.
 * WAITS: no.
 * Entry: X the object. Exit: A the new attribute, N and Z from it. X, Y, C and V kept.
 */
Regs MariaObjectMarkOff(uint8_t object)
{
    uint8_t value = (uint8_t)(RD8(OBJ->sprite_attr[object]) & MARK_OFF);

    WR8(OBJ->sprite_attr[object], value);
    return r_lda(regs_kept(), value);
}

void abi_MariaObjectMarkOff(void)
{
    regs_exit(MariaObjectMarkOff(cpu.x));
}

/*
 * @routine 01E1:4BE MariaClearObjectsBut
 * @run 6E:C4BE
 * WHAT IT DOES: Empties object slots 1 to 15, except an object of kind 4 and one of kind $0C in state 2 (item 4's);
 * time back to normal and the pad's forced presses off.
 * HOW: OBJ_KIND, and OBJ_STATE for kind $0C; ClearObject.
 * WAITS: no.
 * Entry: nothing. Exit: A the last byte read (slot 15's kind, or its state), X $10, Z and C set, N clear. Y and V
 * kept.
 */
Regs MariaClearObjectsBut(void)
{
    uint8_t object, value = 0;

    for (object = OBJECT_FIRST_OTHER; object < OBJECT_END; object++) {
        value = RD8(OBJ->kind[object]);
        if (value == KIND_ITEM4) {
            continue;
        }
        if (value == KIND_ITEM4_EXTRA) {
            value = RD8(OBJ->state[object]);
            if (value == ITEM4_EXTRA_KEPT_STATE) {
                continue;
            }
        }
        jsr_push(0xC4D5);
        ClearObject(object);
        jsr_pop();
    }
    WR8(PVARS->time_slow_mode, 0);
    WR8(PVARS->forced_press, 0);
    WR8(PVARS->forced_press_b, 0);
    return r_cmp(r_x_only(r_a_only(regs_kept(), value), OBJECT_END), OBJECT_END, OBJECT_END);
}

void abi_MariaClearObjectsBut(void)
{
    regs_exit(MariaClearObjectsBut());
}

/*
 * @routine 01E1:6A5 MariaObjectPlaceByState
 * @run 6E:C6A5
 * WHAT IT DOES: Puts a companion (slot 1 or 2) at its place beside her for the state she is in: the offsets from
 * MariaSlotPlaces, the pair picked by MariaSlotPlaceByState and the slot.
 * HOW: (slot - 1) * 2 plus the state's entry; the offsets to zTemp04 and zTemp06; the tail JMP to MariaObjectAtPlayer.
 * WAITS: no.
 * Entry: X the object. Exit: Y the pair's index; as MariaObjectAtPlayer. X kept.
 */
MExit MariaObjectPlaceByState(uint8_t object)
{
    MExit e = mx_none();
    uint8_t base = (uint8_t)((uint8_t)(object - 1) << 1);
    uint8_t state = RD8(OBJ->state[OBJECT_PLAYER]);
    uint8_t index = (uint8_t)(base + TBL8(MariaSlotPlaceByState, state));
    uint8_t value;

    value = TBL8(MariaSlotPlaces, index);
    WR8(ZP->temp04, value);
    value = TBL8(MariaSlotPlaces, index + 1);
    WR8(ZP->temp06, value);
    mx_regs(&e, r_y_only(regs_kept(), index));
    mx_regs(&e, regs_of_sum(MariaObjectAtPlayer(object)));
    return e;
}

void abi_MariaObjectPlaceByState(void)
{
    mx_exit(MariaObjectPlaceByState(cpu.x));
}

/*
 * @routine 01E1:706 MariaClaimKind3Slot
 * @run 6E:C706
 * WHAT IT DOES: Makes sure a companion of kind 3 is out: slot 1 when it is empty; nothing when slot 1 holds kind 3;
 * else slot 2 when it is empty.
 * HOW: OBJ_KIND; ClearObject, kind 3. X is kept on the stack.
 * WAITS: no.
 * Entry: X kept. Exit: A the last kind read or 3, N and Z from X; C from comparing slot 1's kind with 3 when it was
 * out, else kept. Y and V kept.
 */
Regs claim_kind3_slot(uint8_t object, uint16_t clear_ret)
{
    Regs r = regs_kept();
    uint8_t slot = SLOT_COMPANION_1;
    uint8_t kind;

    push8(object);
    kind = RD8(OBJ->kind[SLOT_COMPANION_1]);
    r = r_a_only(r, kind);
    if (kind != 0) {
        r = r_cmp(r, kind, KIND_COMPANION);
        if (kind == KIND_COMPANION) {
            goto done;
        }
        slot = SLOT_COMPANION_2;
        kind = RD8(OBJ->kind[SLOT_COMPANION_2]);
        r = r_a_only(r, kind);
        if (kind != 0) {
            goto done;
        }
    }
    jsr_push(clear_ret);
    ClearObject(slot);
    jsr_pop();
    WR8(OBJ->kind[slot], KIND_COMPANION);
    r = r_a_only(r, KIND_COMPANION);
done:
    object = pull8();
    return r_ldx(r, object);
}

Regs MariaClaimKind3Slot(uint8_t object)
{
    return claim_kind3_slot(object, 0xC71D);
}

void abi_MariaClaimKind3Slot(void)
{
    regs_exit(MariaClaimKind3Slot(cpu.x));
}

/*
 * @routine 01E1:7CF MariaObjectStartAnimation
 * @run 6E:C7CF
 * WHAT IT DOES: Starts an object's animation, sprite set 1, and moves it to its next state.
 * HOW: MariaObjectSetAnimation, the store, the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: A the animation, X the object. Exit: A 1, N and Z from the new state; Y and C as MariaObjectSetAnimation left
 * them. X and V kept.
 */
MExit MariaObjectStartAnimation(uint8_t object, uint8_t animation)
{
    MExit e = mx_none();

    jsr_push(0xC7D2);
    mx_animation(&e, MariaObjectSetAnimation(object, animation));
    jsr_pop();
    WR8(OBJ->sprite_set[object], SPRITE_SET_OBJECT);
    mx_regs(&e, r_nz(r_a_only(regs_kept(), SPRITE_SET_OBJECT), MariaObjectNextState(object)));
    return e;
}

void abi_MariaObjectStartAnimation(void)
{
    mx_exit(MariaObjectStartAnimation(cpu.x, cpu.a));
}

/*
 * @routine 01E2:40B MariaObjectHitByChecked
 * @run 6E:CC0B
 * WHAT IT DOES: Whether an object takes the hit its hit test found: the object that hit it (OBJ_HIT_BY_SLOT) must be
 * slot 16 or more, hitting (OBJ_HITBOX_ON 2) and of health not below zero; then its hitbox goes off and its
 * OBJ_WORK_D counts one down (C set). Otherwise its OBJ_WORK_P is cleared (C clear). OBJ_HIT_BY_SLOT is cleared
 * either way.
 * HOW: The hitter's slot in Y; its columns read by Y.
 * WAITS: no.
 * Entry: X the object. Exit: Y the hitter's slot. Taken, A the hitter's health, N and Z from the new OBJ_WORK_D, C
 * set. Not, C clear, with A and N, Z from the last read or compare (A kept when the slot was below 16). X and V kept.
 */
Regs MariaObjectHitByChecked(uint8_t object)
{
    uint8_t hitter = RD8(OBJ->hit_by_slot[object]);
    Regs r = r_cmp(r_ldy(regs_kept(), hitter), hitter, HIT_SLOT_FIRST);
    uint8_t value;

    if (hitter >= HIT_SLOT_FIRST) {
        value = RD8_IDX(COL(OBJ->hitbox_on), hitter);
        r = r_cmp(r_lda(r, value), value, HITBOX_HITTING);
        if (value == HITBOX_HITTING) {
            value = RD8_IDX(COL(OBJ->health), hitter);
            r = r_lda(r, value);
            if (!(value & 0x80)) {
                WR8(OBJ->hitbox_on[object], 0);
                WR8(OBJ->hit_by_slot[object], 0);
                r = r_nz(r, DEC8(OBJ->work_d[object]));
                return r_c(r, true);
            }
        }
    }
    WR8(OBJ->hit_by_slot[object], 0);
    WR8(OBJ->work_p[object], 0);
    return r_c(r, false);
}

void abi_MariaObjectHitByChecked(void)
{
    regs_exit(MariaObjectHitByChecked(cpu.x));
}

/*
 * @routine 01E2:616 MariaObjectAbovePlayer
 * @run 6E:CE16
 * WHAT IT DOES: Puts an object's Y ten rows above hers, and three more unless her frame is $42.
 * HOW: 16-bit adds of $FFF6 and $FFFD.
 * WAITS: no.
 * Entry: X the object. Exit: frame $42, A $42, Z and C set. Else A the object's Y high byte, N and Z from it, C and
 * V from the add. X and Y kept.
 */
Regs MariaObjectAbovePlayer(uint8_t object)
{
    uint8_t value = RD8(OBJ->y_lo[OBJECT_PLAYER]);
    Sum lo = add_bytes(value, ABOVE_PLAYER, false);
    Sum hi;
    uint8_t frame;

    WR8(OBJ->y_lo[object], lo.value);
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    hi = add_bytes(value, 0xFF, lo.carry);
    WR8(OBJ->y_hi[object], hi.value);
    frame = RD8(OBJ->frame[OBJECT_PLAYER]);
    if (frame == FRAME_HIGHER) {
        return r_v(r_cmp(r_lda(regs_kept(), frame), frame, FRAME_HIGHER), hi.overflow);
    }
    value = RD8(OBJ->y_lo[object]);
    lo = add_bytes(value, ABOVE_PLAYER_MORE, false);
    WR8(OBJ->y_lo[object], lo.value);
    value = RD8(OBJ->y_hi[OBJECT_PLAYER]);
    hi = add_bytes(value, 0xFF, lo.carry);
    WR8(OBJ->y_hi[object], hi.value);
    return regs_of_sum(hi);
}

void abi_MariaObjectAbovePlayer(void)
{
    regs_exit(MariaObjectAbovePlayer(cpu.x));
}

/*
 * @routine 01E2:6D0 MariaObjectFrameFromPlayer
 * @run 6E:CED0
 * WHAT IT DOES: Gives an object the frame that goes with hers (her frame less $45, plus $11), shown on odd counts of
 * zWaveCounter only (frame 0 on even ones).
 * HOW: A subtract and an add; bit 0 of zWaveCounter.
 * WAITS: no.
 * Entry: X the object. Exit: A bit 0 of zWaveCounter, N and Z from it; C and V from the add. X and Y kept.
 */
Regs MariaObjectFrameFromPlayer(uint8_t object)
{
    uint8_t value = RD8(OBJ->frame[OBJECT_PLAYER]);
    Sum less = sub_bytes(value, FRAME_FIRST, true);
    Sum own = add_bytes(less.value, FRAME_OWN_FIRST, false);
    uint8_t odd;

    WR8(OBJ->frame[object], own.value);
    odd = (uint8_t)(RD8(ZP->wave_counter) & 0x01);
    if (odd == 0) {
        WR8(OBJ->frame[object], 0);
    }
    return r_v(r_c(r_lda(regs_kept(), odd), own.carry), own.overflow);
}

void abi_MariaObjectFrameFromPlayer(void)
{
    regs_exit(MariaObjectFrameFromPlayer(cpu.x));
}

/*
 * @routine 01E2:7B5 MariaObjectVelocityFromWork
 * @run 6E:CFB5
 * WHAT IT DOES: Points an object along the angle in its OBJ_WORK_D at the speed in OBJ_WORK_E (low) and OBJ_WORK_F
 * (high).
 * HOW: The speed to zTemp00-01, then SetVelocityFromAngle (through the kernel's vector at $4048).
 * WAITS: no.
 * Entry: X the object. Exit: as SetVelocityFromAngle.
 */
Regs MariaObjectVelocityFromWork(uint8_t object)
{
    uint8_t value = RD8(OBJ->work_e[object]);
    uint8_t angle;
    ScaleResult r;

    WR8(ZP->temp00, value);
    value = RD8(OBJ->work_f[object]);
    WR8(ZP->temp01, value);
    angle = RD8(OBJ->work_d[object]);
    jsr_push(0xCFC5);
    r = SetVelocityFromAngle(angle, object);
    jsr_pop();
    return regs_of_scale(r);
}

void abi_MariaObjectVelocityFromWork(void)
{
    regs_exit(MariaObjectVelocityFromWork(cpu.x));
}

/*
 * @routine 01E3:26B MariaStartScriptSlot3
 * @run 6E:D26B
 * WHAT IT DOES: Starts a script in script slot 3 as kind 5, unless slot 3 is in use.
 * HOW: StartSoleScriptSlot (through the kernel's vector at $4042) with X 3; X kept on the stack.
 * WAITS: no.
 * Entry: A the script, X kept. Exit: A, Y and C as StartSoleScriptSlot left them, N and Z from X. V kept.
 */
Regs MariaStartScriptSlot3(uint8_t object, uint8_t script)
{
    Regs r;

    push8(object);
    jsr_push(0xD271);
    r = regs_of_sole_slot(StartSoleScriptSlot(script, SLOT_SCRIPT));
    jsr_pop();
    object = pull8();
    return r_ldx(r, object);
}

void abi_MariaStartScriptSlot3(void)
{
    regs_exit(MariaStartScriptSlot3(cpu.x, cpu.a));
}

/*
 * @routine 01E3:33F MariaObjectFarOffScreen
 * @run 6E:D33F
 * WHAT IT DOES: Says whether an object is more than $30 pixels off the screen: 1 when it is, 0 when it is on the
 * screen (X high byte 0) or near its edge.
 * HOW: OBJ_X_HI 0 is on the screen, $FF the screen to the left (off when X is below $D0), any other past the right
 * (off from $30 on).
 * WAITS: no.
 * Entry: X the object. Exit: A 0 or 1, N and Z from it; C from the compare when one was made. X, Y and V kept.
 */
Regs MariaObjectFarOffScreen(uint8_t object)
{
    Regs r = regs_kept();
    uint8_t high = RD8(OBJ->x_hi[object]);
    uint8_t low;

    if (high != 0) {
        if ((uint8_t)(high + 1) == 0) {
            low = RD8(OBJ->x_lo[object]);
            r = r_cmp(r, low, EDGE_LEFT_X);
            if (low < EDGE_LEFT_X) {
                return r_lda(r, 1);
            }
        } else {
            low = RD8(OBJ->x_lo[object]);
            r = r_cmp(r, low, EDGE_RIGHT_X);
            if (low >= EDGE_RIGHT_X) {
                return r_lda(r, 1);
            }
        }
    }
    return r_lda(r, 0);
}

void abi_MariaObjectFarOffScreen(void)
{
    regs_exit(MariaObjectFarOffScreen(cpu.x));
}

/*
 * @routine 01E3:35D MariaObjectFrame24Next
 * @run 6E:D35D
 * WHAT IT DOES: Gives an object frame $24, clears zFieldD2, and moves the object to its next state.
 * HOW: The stores, the tail JMP to MariaObjectNextState.
 * WAITS: no.
 * Entry: X the object. Exit: A $24, N and Z from the new state. X, Y, C and V kept.
 */
Regs MariaObjectFrame24Next(uint8_t object)
{
    WR8(OBJ->frame[object], FRAME_24);
    WR8(ZP->fieldd2, 0);
    return r_nz(r_a_only(regs_kept(), FRAME_24), MariaObjectNextState(object));
}

void abi_MariaObjectFrame24Next(void)
{
    regs_exit(MariaObjectFrame24Next(cpu.x));
}

/*
 * @routine 01E3:38C MariaObjectFrameByPlayerFrame
 * @run 6E:D38C
 * WHAT IT DOES: Gives an object the frame MariaFrameByPlayerFrame pairs with hers (from frame $53).
 * HOW: Her frame less $53 indexes the table.
 * WAITS: no.
 * Entry: X the object. Exit: A the frame, N and Z from it; Y the index; C and V from the subtract. X kept.
 */
Regs MariaObjectFrameByPlayerFrame(uint8_t object)
{
    uint8_t value = RD8(OBJ->frame[OBJECT_PLAYER]);
    Sum index = sub_bytes(value, FRAME_TABLE_FIRST, true);
    uint8_t frame = TBL8(MariaFrameByPlayerFrame, index.value);

    WR8(OBJ->frame[object], frame);
    return r_v(r_c(r_lda(r_y_only(regs_kept(), index.value), frame), index.carry), index.overflow);
}

void abi_MariaObjectFrameByPlayerFrame(void)
{
    regs_exit(MariaObjectFrameByPlayerFrame(cpu.x));
}

/*
 * @routine 01E3:3BE MariaObjectFrame25
 * @run 6E:D3BE
 * WHAT IT DOES: Gives an object frame $25.
 * HOW: One store.
 * WAITS: no.
 * Entry: X the object. Exit: A $25, N and Z from it. X, Y, C and V kept.
 */
Regs MariaObjectFrame25(uint8_t object)
{
    WR8(OBJ->frame[object], FRAME_25);
    return r_lda(regs_kept(), FRAME_25);
}

void abi_MariaObjectFrame25(void)
{
    regs_exit(MariaObjectFrame25(cpu.x));
}
