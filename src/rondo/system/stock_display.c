/* The player's stock (lives) shown on the stage's tile rows, and the extra life that changes it. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/*
 * @routine 05BF:151 BlendStockRows
 * @run 6B:9151
 * WHAT IT DOES: Maps the stage's banks and puts the player's stock on the stage's tile rows: with the blend off ($366F clear) it clears the blend step $366E, notes the stock in StageVar3616 and
 * blends the first row pair with the stock as the byte; with the blend on, it only asks for the pause bit (zPaused bit 1) and the value $C0 in StageVar3617 when the sub-mode is 1 and the noted stock differs;
 * then puts the banks back.
 * HOW: SaveAndMapStageBanksLong, BlendStageRowPair, a tail jump into RestoreStageBanksLong.
 * Untraced: the arm that sets bit 1 of zPaused (9166).
 * WAITS: no.
 * Entry: X kept, Y the second row of the blend. Exit: A the bank restored, N and Z from it; C and V as the last blend or the bank add left them.
 */
/* The arms BlendStockRows ends on. */
enum { BS_BLENDED, BS_UNCHANGED, BS_OTHER_MODE, BS_PAUSE_ASKED };

ObjArm BlendStockRows(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    TileRowsExit blend;

    jsr_push(0x9154);
    r.sum = SaveAndMapStageBanks();
    jsr_pop();
    r.first = RD8(STAGEVARS->blend_on[1]);
    if (r.first == 0) {
        r.arm = BS_BLENDED;
        push8(slot);
        WR8(STAGEVARS->blend_on[0], 0);
        r.second = RD8(ZP->stock);
        WR8(STAGEVARS->state_3616[0], r.second);
        jsr_push(0x917D);
        blend = BlendStageRowPair(0, r.second, y);
        jsr_pop();
        r.third = blend.a;
        r.flag = blend.v;
        r.sixth = pull8();
    } else {
        r.fourth = RD8(STAGEVARS->state_3616[0]);
        r.second = RD8(ZP->stock);
        r.arm = BS_UNCHANGED;
        if (r.fourth != r.second) {
            r.fifth = RD8(ZP->game_sub_mode);
            r.arm = BS_OTHER_MODE;
            if (r.fifth == 1) {
                RMW8(ZP->paused, old, old | 0x02);
                WR8(STAGEVARS->state_3616[1], 0xC0);
                r.arm = BS_PAUSE_ASKED;
            }
        }
    }
    r.seventh = RestoreStageBanks();
    return r;
}

/* BlendStockRows' exit: A the bank restored, N and Z from it; C and V as the last blend or the bank add left them. */
Ended blend_stock_rows_exit_shim(ObjArm r)
{
    Ended e = ended_with_a(ended_with_overflow(ended_with_carry(ended_nz((uint8_t)(r.sum.value + 1)), r.sum.carry), r.sum.overflow),
                           (uint8_t)(r.sum.value + 1));

    e = ended_then(e, ended_load(r.first));
    if (r.arm == BS_BLENDED) {
        e = ended_then(e, ended_load(r.second));
        e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(0), r.third), true), r.flag), 5));
        e = ended_then(e, ended_nz(r.sixth));
    } else {
        e = ended_then(e, ended_with_carry(ended_load(r.fourth), r.fourth >= r.second));
        if (r.arm != BS_UNCHANGED) {
            e = ended_then(e, ended_with_carry(ended_load(r.fifth), r.fifth >= 1));
            if (r.arm == BS_PAUSE_ASKED) {
                e = ended_then(e, ended_load(0xC0));
            }
        }
    }
    return ended_then(e, ended_load(r.seventh));
}

void abi_BlendStockRows(void)
{
    abi_ended(blend_stock_rows_exit_shim(BlendStockRows(cpu.x, cpu.y)));
}

/*
 * @routine 0519:6EC GiveExtraLife
 * @run 70:A6EC
 * WHAT IT DOES: Plays sound $52, adds one to the player's stock (zStock, decimal, at most 99) and shows it (BlendStockRows, by a tail jump).
 * HOW: SoundCommand, a decimal add.
 * Untraced: the whole routine (no script ran it); the draft does not exist.
 * WAITS: no.
 * Entry: X kept, Y the sound driver's. Exit: as BlendStockRows.
 */
ObjArm GiveExtraLife(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t stock;
    bool carry;
    unsigned sum;

    r.sounded = true;
    r.command = 0x52;
    jsr_push(0xA6F1);
    r.call = SoundCommand(0x52, slot, y);
    jsr_pop();
    stock = RD8(ZP->stock);
    sum = (unsigned)((stock & 0x0F) + 1);
    if (sum >= 0x0A) {
        sum += 0x06;
    }
    sum += (unsigned)(stock & 0xF0);
    if (sum >= 0xA0) {
        sum += 0x60;
    }
    carry = (sum & 0xFF00u) != 0;
    stock = (uint8_t)sum;
    WR8(ZP->stock, stock);
    if (carry) {
        WR8(ZP->stock, 0x99);
    }
    r.sub = obj_sub(BlendStockRows(slot, y));
    return r;
}

/* GiveExtraLife's exit: the sound's flags, then what BlendStockRows left. */
SoundEnd give_extra_life_exit_shim(ObjArm r)
{
    SoundEnd e = { { 0 }, { 0 }, false, 0, { 0 } };

    e.before = ended_load(0x52);
    e.sounded = r.sounded;
    e.command = r.command;
    e.call = r.call;
    e.end = blend_stock_rows_exit_shim(obj_arm(r.sub));
    return e;
}

void abi_GiveExtraLife(void)
{
    abi_sound_end(give_extra_life_exit_shim(GiveExtraLife(cpu.x, cpu.y)));
}
