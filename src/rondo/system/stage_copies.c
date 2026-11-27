/*
 * stage_copies.c: the stage helpers a shared caller reaches at a fixed address, taken from the group of stage overlays
 * that is loaded. The 024F group (11 overlays) and the 05BF group (13) hold the same blends at $9151, $9181 and $91AC,
 * whose bank calls differ, and the same VRAM masks and bank mapping $312 apart ($9392, $93A1, $9349 against $96A4,
 * $96B3, $965B). A caller whose bytes are the same in both groups (StepStageSequence, the item pickups in the enemy
 * overlays) or whose body is shared (StepStageFrameBody) asks resident_at which copy is there and calls that copy's C,
 * as its JSR reaches it. A copy of neither group runs as 024F's, as before these were told apart.
 */
#include "machine.h"
#include "routines.h"
#include "stage_map.h"
#include "stage_enemies_l7.h"

/* True when 05BF's group's copy of the routine at run address `run` (disc offset `offset` in sector 05BF) is loaded. */
static bool group_05bf_at(uint16_t run, uint16_t offset)
{
    return resident_at(run) == DISC(0x05BF, offset);
}

ObjArm blend_stock_rows_loaded(uint8_t slot, uint8_t y)
{
    if (group_05bf_at(0x9151, 0x151)) {
        return BlendStockRows(slot, y);
    }
    return BlendStageStockRows(slot, y);
}

ObjArm blend_rows_of_player_loaded(uint8_t slot, uint8_t y)
{
    if (group_05bf_at(0x9181, 0x181)) {
        return BlendStageRowsOfPlayer(slot, y);
    }
    return BlendStageScoreAddRows(slot, y);
}

BlendStepEnd blend_score_rows_loaded(uint8_t x)
{
    if (group_05bf_at(0x91AC, 0x1AC)) {
        return BlendStageScoreRowsLong(x);
    }
    return BlendStageScoreRows(x);
}

BlendStepEnd blend_stock_step_loaded(uint8_t x, uint8_t y)
{
    return blend_stock_rows_step(blend_stock_rows_loaded(x, y));
}

BlendStepEnd blend_score_add_step_loaded(uint8_t x, uint8_t y)
{
    return blend_rows_of_player_step(blend_rows_of_player_loaded(x, y));
}

uint8_t and_vram_data_loaded(uint8_t lo_mask, uint8_t hi_mask)
{
    if (group_05bf_at(0x96A4, 0x6A4)) {
        return AndVramDataLong(lo_mask, hi_mask);
    }
    return AndVramData(lo_mask, hi_mask);
}

uint8_t or_vram_data_loaded(uint8_t lo_mask, uint8_t hi_mask)
{
    if (group_05bf_at(0x96B3, 0x6B3)) {
        return OrVramDataLong(lo_mask, hi_mask);
    }
    return OrVramData(lo_mask, hi_mask);
}

Sum map_stage_banks_loaded(void)
{
    if (group_05bf_at(0x965B, 0x65B)) {
        return MapStageBanksLong();
    }
    return MapStageBanks();
}
