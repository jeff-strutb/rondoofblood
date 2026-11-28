/*
 * The stage's scroll set up before its first frame: stages 02, 04 and 05, whose copies are the same bytes here.
 */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_scroll_entry.h"

enum { SCROLL_SLOT_BYTES = 0x30, SCROLL_COLUMN_BYTES = 0x88 };

static void load_a(uint8_t value)
{
    cpu.a = value;
    abi_nz(value);
}

static void clear_scroll(void)
{
    tai(KernelZeros, ScrollXFrac, SCROLL_SLOT_BYTES);
    tai(KernelZeros, ScrollColumnBlocks, SCROLL_COLUMN_BYTES);
}

/* LoadStageEntry with zField8E kept across it. */
static void load_entry_keeping_flags(uint16_t ret)
{
    load_a(RD8(ZP->field8e));
    push8(cpu.a);
    call_routine(ret, 0x8952);
    load_a(pull8());
    WR8(ZP->field8e, cpu.a);
}

/*
 * @routine 050E:0FB SetUpStageScrollTwice
 * @run 6B:88FB
 * @stop 8952
 * WHAT IT DOES: Sets up the stage's scroll from the section's record: keeps the camera's X for the entry, clears
 * the scroll state, loads the entry and sets up the scroll; for a raster effect with a second set up, does it again
 * for the second plane; then clears the state once more and goes on into the entry load.
 * HOW: Block clears, then LoadStageEntry ($8952) around MapOverlayBank7, StageSetUpScroll and
 * StageSetUpScrollByEffect in the overlay.
 * WAITS: no.
 * Entry: zField83 the section. Exit: as LoadStageEntry at $8952.
 */
void SetUpStageScrollTwice(void)
{
    load_a(RD8(SCROLLE->x_hi_screen[0]));
    WR8(ZP->bios_a.lo, cpu.a);
    load_a(RD8(SCROLL->x_lo[0]));
    WR8(ZP->bios_a.hi, cpu.a);
    clear_scroll();
    WR8(OBJ->sprite_flip[0], 0);
    load_entry_keeping_flags(0x891B);
    call_routine(0x8921, 0x80B7);
    call_routine(0x8924, 0xA94C);
    cpu.y = RD8(ZP->raster_effect);
    abi_nz(cpu.y);
    load_a(rd8((uint16_t)(EffectSecondSetUp + cpu.y)));
    if (cpu.a != 0) {
        clear_scroll();
        load_entry_keeping_flags(0x893E);
        call_routine(0x8944, 0xA9B1);
    }
    clear_scroll();
    run_routine(0x8952);
}

void abi_SetUpStageScrollTwice(void)
{
    SetUpStageScrollTwice();
}
