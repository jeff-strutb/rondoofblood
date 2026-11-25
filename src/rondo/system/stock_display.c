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
Ended BlendStockRows(uint8_t slot, uint8_t y)
{
    TileRowsExit blend;
    Sum mapped;
    uint8_t flag, noted, stock, mode, bank;
    Ended e;

    jsr_push(0x9154);
    mapped = SaveAndMapStageBanks();
    jsr_pop();
    e = ended_with_a(ended_with_overflow(ended_with_carry(ended_nz((uint8_t)(mapped.value + 1)), mapped.carry), mapped.overflow),
                     (uint8_t)(mapped.value + 1));
    flag = RD8(STAGEVARS->blend_on[1]);
    e = ended_then(e, ended_load(flag));
    if (flag == 0) {
        push8(slot);
        WR8(STAGEVARS->blend_on[0], 0);
        stock = RD8(ZP->stock);
        WR8(STAGEVARS->state_3616[0], stock);
        e = ended_then(e, ended_load(stock));
        jsr_push(0x917D);
        blend = BlendStageRowPair(0, stock, y);
        jsr_pop();
        e = ended_then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(0), blend.a), true), blend.v), 5));
        slot = pull8();
        e = ended_then(e, ended_nz(slot));
    } else {
        noted = RD8(STAGEVARS->state_3616[0]);
        stock = RD8(ZP->stock);
        e = ended_then(e, ended_with_carry(ended_load(noted), noted >= stock));
        if (noted != stock) {
            mode = RD8(ZP->game_sub_mode);
            e = ended_then(e, ended_with_carry(ended_load(mode), mode >= 1));
            if (mode == 1) {
                RMW8(ZP->paused, old, old | 0x02);
                WR8(STAGEVARS->state_3616[1], 0xC0);
                e = ended_then(e, ended_load(0xC0));
            }
        }
    }
    bank = RestoreStageBanks();
    return ended_then(e, ended_load(bank));
}

void abi_BlendStockRows(void)
{
    abi_ended(BlendStockRows(cpu.x, cpu.y));
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
SoundEnd GiveExtraLife(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t stock;
    bool carry;
    unsigned sum;

    r.before = ended_load(0x52);
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
    r.end = BlendStockRows(slot, y);
    return r;
}

void abi_GiveExtraLife(void)
{
    abi_sound_end(GiveExtraLife(cpu.x, cpu.y));
}
