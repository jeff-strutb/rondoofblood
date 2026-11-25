/*
 * The loop that runs every object once a frame: for each slot from $10 to $57 it moves the object with the scroll, clears it when it
 * is far off screen, runs its hit handler, decides whether this frame it runs its own code, and then dispatches by its kind.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/* What the hold window of an object with flags bit 2 says: true when it is on the first screen in the window or beyond it (held). */
static bool in_hold_window(uint8_t x, Ended *e)
{
    uint8_t high, low;

    high = RD8(OBJ->x_hi[x]);
    *e = ended_then(*e, ended_load(high));
    if (high == 0) {
        return true;
    }
    if ((high & 0x80) == 0) {
        *e = ended_then(*e, ended_compare(high, 1));
        if (high != 1) {
            return false;
        }
        low = RD8(OBJ->x_lo[x]);
        *e = ended_then(*e, ended_compare(low, 0x40));
        return low < 0x40;
    }
    *e = ended_then(*e, ended_compare(high, 0xFF));
    if (high != 0xFF) {
        return false;
    }
    low = RD8(OBJ->x_lo[x]);
    *e = ended_then(*e, ended_compare(low, 0xC0));
    return low >= 0xC0;
}

/* One axis of the scroll: the object's position takes off the scroll speed's sign-extended word. */
static void scroll_axis(uint8_t x, uint8_t y, uint16_t speed_table, uint16_t low_col, uint16_t high_col, Ended *e)
{
    uint8_t speed, low, high, extend;
    Sum sum;

    WR8(ZP->temp00, 0);
    speed = rd8((uint16_t)(speed_table + y));
    *e = ended_then(*e, ended_load(speed));
    if ((speed & 0x80) != 0) {
        extend = DEC8(ZP->temp00);
        *e = ended_then(*e, ended_nz(extend));
    }
    low = RD8_IDX(low_col, x);
    sum = sub_bytes(low, rd8((uint16_t)(speed_table + y)), true);
    WR8_IDX(low_col, x, sum.value);
    *e = ended_then(*e, ended_of_sum(sum));
    high = RD8_IDX(high_col, x);
    sum = sub_bytes(high, RD8(ZP->temp00), sum.carry);
    WR8_IDX(high_col, x, sum.value);
    *e = ended_then(*e, ended_of_sum(sum));
}

/* The far-off-screen test of one axis: true when the object is past the edge by its high byte and the low byte's top bit. */
static bool past_edge(uint8_t high_value, uint8_t low_value)
{
    if ((high_value & 0x80) != 0) {
        return low_value < 0x80;
    }
    return low_value >= 0x80;
}

/*
 * @routine 0259:000 RunObjects
 * @run 70:A000
 * @stop A13C
 * WHAT IT DOES: Runs every object slot from $10 to $57 once. For each slot with a kind and a state: unless its flags bit 0 says it stays put, it moves by the scroll speeds (the table
 * entry by its flags); an object whose flags bit 1 is clear and which is far off the first screen is cleared; the hit handler runs (ObjectHit); an object whose flags bit 2 is
 * set is held while it is outside the window; one with a hurt timer counts it down; the player's mode (PlayerByte3BD8) may skip the object on some frames by a wave-counter table;
 * the rest dispatch by kind (DispatchByKind0259). A slot with no state dispatches at once. At the end, every fourth wave tick it starts the next script of a cue table at $A13C.
 * HOW: A loop of X from $10 over the pieces of the original (000, 044, 05E, 090, 0C0, 0D1, 112), which are one function here; each place the original code is entered, the registers the
 * typed steps so far left are put into the machine first.
 * Untraced: parts of the position tests and of the player modes' tables.
 * WAITS: no.
 * Entry: X pushed and pulled back. Exit: A and the flags as the last instruction left them.
 */
Ended RunObjects(uint8_t entry_x)
{
    ExitChain c;
    Ended e = ended_kept();
    uint8_t x, y, kind, state, flags, high, low, timer, mode, wave, table, count, script, index;
    bool next, dispatch;
    Sum sum;

    push8(entry_x);
    x = 0x10;
    y = 0;
    do {
        next = false;
        dispatch = false;
        kind = RD8(OBJ->kind[x]);
        e = ended_then(e, ended_load(kind));
        if (kind == 0) {
            next = true;
        } else {
            state = RD8(OBJ->state[x]);
            e = ended_then(e, ended_load(state));
            dispatch = state == 0;
        }
        if (!next && !dispatch) {
            flags = RD8(OBJ->f30f0[x]);
            flags = (uint8_t)(flags & 0x01);
            e = ended_then(e, ended_load(flags));
            if (flags == 0) {
                y = RD8(OBJ->flags[x]);
                y = (uint8_t)(y & 0x7F);
                e = ended_then(e, ended_with_y(ended_load(y), y));
                scroll_axis(x, y, ScrollSpeedX, COL(OBJ->x_lo), COL(OBJ->x_hi), &e);
                scroll_axis(x, y, ScrollSpeedY, COL(OBJ->y_lo), COL(OBJ->y_hi), &e);
            }
            state = RD8(OBJ->state[x]);
            e = ended_then(e, ended_load(state));
            dispatch = state == 0;
        }
        if (!next && !dispatch) {
            flags = RD8(OBJ->f30f0[x]);
            flags = (uint8_t)(flags & 0x02);
            e = ended_then(e, ended_load(flags));
            if (flags == 0) {
                bool clear = false;

                high = RD8(OBJ->x_hi[x]);
                e = ended_then(e, ended_load(high));
                if (high != 0) {
                    low = RD8(OBJ->x_lo[x]);
                    e = ended_then(e, ended_compare(low, 0x80));
                    clear = past_edge(high, low);
                }
                if (!clear) {
                    high = RD8(OBJ->y_hi[x]);
                    e = ended_then(e, ended_load(high));
                    if (high != 0) {
                        low = RD8(OBJ->y_lo[x]);
                        e = ended_then(e, ended_compare(low, 0x80));
                        clear = past_edge(high, low);
                    }
                }
                if (clear) {
                    sync(&e);
                    jsr_push(0xA076);
                    ClearObject(x);
                    jsr_pop();
                    next = true;
                }
            }
        }
        if (!next && !dispatch) {
            sync(&e);
            jsr_push(0xA09A);
            c = ObjectHit(x, y);
            jsr_pop();
            abi_chain(&c);
            flags = RD8(OBJ->f30f0[x]);
            flags = (uint8_t)(flags & 0x04);
            e = ended_load(flags);
            if (flags != 0 && !in_hold_window(x, &e)) {
                next = true;
            }
        }
        if (!next && !dispatch) {
            timer = RD8(OBJ->f2f90[x]);
            e = ended_then(e, ended_load(timer));
            if (timer != 0) {
                timer = DEC8(OBJ->f2f90[x]);
                e = ended_then(e, ended_nz(timer));
                flags = RD8(OBJ->f3148[x]);
                flags = (uint8_t)(flags & 0x04);
                e = ended_then(e, ended_load(flags));
                if (flags == 0) {
                    next = true;
                }
            }
        }
        if (!next && !dispatch) {
            mode = rd8(PlayerByte3BD8);
            e = ended_then(e, ended_load(mode));
            if (mode == 0) {
                dispatch = true;
            } else if (mode == 3) {
                e = ended_then(e, ended_compare(mode, 3));
                wave = RD8(ZP->wave_counter);
                wave = (uint8_t)(wave & 0x03);
                e = ended_then(e, ended_load(wave));
                next = wave != 0;
                dispatch = !next;
            } else {
                e = ended_then(e, ended_compare(mode, 2));
                flags = RD8(OBJ->f3148[x]);
                flags = (uint8_t)(flags & (mode == 2 ? 0x60 : 0x18));
                e = ended_then(e, ended_load(flags));
                if (mode == 2) {
                    flags = (uint8_t)(flags >> 2);
                    e = ended_then(e, ended_with_carry(ended_load(flags), false));
                }
                WR8(ZP->temp00, flags);
                wave = RD8(ZP->wave_counter);
                wave = (uint8_t)(wave & 0x07);
                e = ended_then(e, ended_load(wave));
                sum = add_bytes(wave, RD8(ZP->temp00), false);
                e = ended_then(e, ended_with_y(ended_of_sum(sum), sum.value));
                y = sum.value;
                table = rd8((uint16_t)((mode == 2 ? 0xA166 : 0xA146) + y));
                e = ended_then(e, ended_load(table));
                next = table != 0;
                dispatch = !next;
            }
        }
        if (dispatch) {
            Jump jump;

            sync(&e);
            jsr_push(0xA115);
            jump = DispatchByKind0259(x);
            x = enter_handler(x, jump);
            jsr_pop();
            e = ended_kept();
        }
        x++;
        e = ended_then(e, ended_with_carry(ended_nz((uint8_t)(x - 0x58)), x >= 0x58));
    } while (x < 0x58);
    wave = RD8(ZP->wave_counter);
    wave = (uint8_t)(wave & 0x03);
    e = ended_then(e, ended_load(wave));
    if (wave == 0) {
        index = rd8(ObjectCueIndex);
        script = rd8((uint16_t)(0xA13C + index));
        e = ended_then(e, ended_with_y(ended_load(script), index));
        sync(&e);
        jsr_push(0xA12D);
        script = StartScriptSlotKind4(script, 0);
        jsr_pop();
        e = ended_with_y(ended_load(script), script);
        count = rd8(ObjectCueIndex);
        count++;
        wr8(ObjectCueIndex, count);
        e = ended_then(e, ended_nz(count));
        count = rd8(ObjectCueIndex);
        e = ended_then(e, ended_compare(count, 0x0A));
        if (count >= 0x0A) {
            wr8(ObjectCueIndex, 0);
        }
    }
    count = pull8();
    e = ended_then(e, ended_nz(count));
    return e;
}

void abi_RunObjects(void)
{
    uint8_t entry_x = cpu.x;

    abi_ended(RunObjects(entry_x));
    cpu.x = entry_x;
}
