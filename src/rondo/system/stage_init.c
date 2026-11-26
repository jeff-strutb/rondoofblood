/* A stage's start: the screen cleared, the scroll and the object banks set up, the stage's own code started. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    SCROLL_CLEAR = 0x0030, SCROLL_ENTRIES_CLEAR = 0x0088, SCROLL_ENTRIES = 0x3E66, SCROLL_MAIN = 0x3E00,
    CLEAR_SCREEN_RETURN = 0x88F9, FIRST_LOAD_RETURN = 0x891B, MAP_RETURN = 0x8921, FIRST_CODE_RETURN = 0x8924,
    SECOND_LOAD_RETURN = 0x893E, SECOND_CODE_RETURN = 0x8944, EFFECT_TABLE = 0x88CE
};

/* 024E's layout of the start: the return addresses of its calls, the effect table, and the overlay's two set-ups. */
static StageExit init_stage(uint8_t x, uint8_t y, uint16_t first_code, uint16_t second_code)
{
    InitStageLayout layout = { CLEAR_SCREEN_RETURN, FIRST_LOAD_RETURN, MAP_RETURN, FIRST_CODE_RETURN,
                               SECOND_LOAD_RETURN, SECOND_CODE_RETURN, EFFECT_TABLE, first_code, second_code };

    return InitStageBody(x, y, &layout);
}

void abi_ClearScreenState(void);
void abi_MapOverlayBank7(void);

/* The two block moves that clear the scroll state, made with the registers as the code holds them. */
static void clear_scroll_shim(StageRegs r)
{
    stage_regs_load_shim(r);
    tai(KernelZeros, SCROLL_MAIN, SCROLL_CLEAR);
    tai(KernelZeros, SCROLL_ENTRIES, SCROLL_ENTRIES_CLEAR);
}

/* The loader's two calls, with the machine flags kept on the stack around them as the code keeps them. */
static StageRegs load_entry(StageRegs r, uint16_t ret)
{
    uint8_t flags = RD8(ZP->field8e);

    push8(flags);
    jsr_push(ret);
    r = stage_regs_after(LoadStageEntryResident(), r);
    jsr_pop();
    flags = pull8();
    WR8(ZP->field8e, flags);
    r.a = flags;
    r.nz = flags;
    return r;
}

/*
 * The stage start: clears the screen with the scroll saved in zBiosAl, clears the scroll state,
 * loads the section's entry, maps the object bank and runs the overlay's first set-up; when the
 * overlay's table says the section has a second set-up, clears and loads again and runs that;
 * clears once more and ends as the entry loader.
 */
StageExit InitStageBody(uint8_t x, uint8_t y, const InitStageLayout *layout)
{
    StageRegs r = { 0, x, y, false, false, 0 };
    StageExit end;
    uint8_t a;

    a = RD8(SCROLLE->x_hi_screen[0]);
    WR8(ZP->bios_a.lo, a);
    a = RD8(SCROLL->x_lo[0]);
    WR8(ZP->bios_a.hi, a);
    r.a = a;
    r.nz = a;
    stage_regs_load_shim(r);
    jsr_push(layout->clear_screen_return);
    abi_ClearScreenState();
    jsr_pop();
    r = stage_regs_from_cpu_shim(r);
    clear_scroll_shim(r);
    WR8(OBJ->sprite_flip[0], 0);
    r = load_entry(r, layout->first_load_return);
    stage_regs_load_shim(r);
    jsr_push(layout->map_return);
    abi_MapOverlayBank7();
    jsr_pop();
    r = stage_regs_from_cpu_shim(r);
    stage_regs_load_shim(r);
    call_orig(layout->first_code_return, layout->first_code);
    r = stage_regs_from_cpu_shim(r);
    r.y = RD8(ZP->raster_effect);
    a = rd8((uint16_t)(layout->effect_table + r.y));
    r.a = a;
    r.nz = a;
    if (a != 0) {
        clear_scroll_shim(r);
        r = load_entry(r, layout->second_load_return);
        stage_regs_load_shim(r);
        call_orig(layout->second_code_return, layout->second_code);
        r = stage_regs_from_cpu_shim(r);
    }
    clear_scroll_shim(r);
    end = LoadStageEntryResident();
    if ((end.set & EXIT_X) == 0) {
        end.x = r.x;
        end.set |= EXIT_X;
    }
    if ((end.set & EXIT_V) == 0) {
        end.v = r.v;
        end.set |= EXIT_V;
    }
    return end;
}

/*
 * @routine 024E:0EC InitStage
 * @run 6B:88EC
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry
 * record, maps the object bank and runs the overlay's set-up at $A94C; when the section's
 * effect byte has a second set-up, clears and loads again and runs the one at $A9B1; then
 * clears the scroll state once more and runs on into the entry loader.
 * Twenty-one stage overlays hold this code.
 * HOW: ClearScreenState with the scroll kept in zBiosAl, block moves of zeros over SCROLL and
 * SCROLLE, LoadStageEntry through the overlay's copy, MapOverlayBank7, and the overlay's own code.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStage(uint8_t x, uint8_t y)
{
    return init_stage(x, y, 0xA94C, 0xA9B1);
}

void abi_InitStage(void)
{
    stage_exit_shim(InitStage(cpu.x, cpu.y));
}

/*
 * @routine 10BE:0EC InitStageB
 * @run 6B:88EC
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry
 * record, maps the object bank and runs the overlay's set-up at $ACDC; when the section's
 * effect byte has a second set-up, clears and loads again and runs the one at $AD41; then
 * clears the scroll state once more and runs on into the entry loader.
 * The same code as InitStage; this overlay's set-ups are at different addresses.
 * HOW: ClearScreenState with the scroll kept in zBiosAl, block moves of zeros over SCROLL and
 * SCROLLE, LoadStageEntry through the overlay's copy, MapOverlayBank7, and the overlay's own code.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStageB(uint8_t x, uint8_t y)
{
    return init_stage(x, y, 0xACDC, 0xAD41);
}

void abi_InitStageB(void)
{
    stage_exit_shim(InitStageB(cpu.x, cpu.y));
}

/*
 * @routine 12CE:0EC InitStageC
 * @run 6B:88EC
 * WHAT IT DOES: Starts a stage: clears the screen and the scroll state, loads the section's entry
 * record, maps the object bank and runs the overlay's set-up at $ACD5; when the section's
 * effect byte has a second set-up, clears and loads again and runs the one at $AD3A; then
 * clears the scroll state once more and runs on into the entry loader.
 * The same code as InitStage; this overlay's set-ups are at different addresses.
 * HOW: ClearScreenState with the scroll kept in zBiosAl, block moves of zeros over SCROLL and
 * SCROLLE, LoadStageEntry through the overlay's copy, MapOverlayBank7, and the overlay's own code.
 * WAITS: no.
 * Entry: X and Y as the block moves push them. Exit: as LoadStageEntry; X and V as the set-up code left them.
 */
StageExit InitStageC(uint8_t x, uint8_t y)
{
    return init_stage(x, y, 0xACD5, 0xAD3A);
}

void abi_InitStageC(void)
{
    stage_exit_shim(InitStageC(cpu.x, cpu.y));
}
