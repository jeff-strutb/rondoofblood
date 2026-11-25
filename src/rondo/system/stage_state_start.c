/* A stage's first state: the section taken over, the card polled until it is ready, the stage's game mode ended. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum { CARD_RETURN = 0x883C, CARD_CALL = 0xE018, DISPLAY_DELAY = 2, FIELD8E_ENDING = 0x04, FIELD8E_READY = 0x08 };

/*
 * @routine 024E:02B EnterStageState
 * @run 6B:882B
 * WHAT IT DOES: The state a stage starts in: the section (zField86) becomes the one the stage loads
 * (zField83), two stage bytes are cleared, bit 2 of zField8E is cleared and, when the stage number
 * is negative, the System Card routine at $E018 is called until it answers zero and bit 3 of
 * zField8E is set. Then, with the game running, the mode is counted done; otherwise the raster
 * effect is cleared, the display delay set to 2 and the game sub mode cleared.
 * HOW: A loop of the card call; the rest is stores.
 * WAITS: it waits for the card as long as it answers.
 * Entry: nothing. Exit: A the last value loaded, X the card's last answer when it was called.
 * Seventeen stage overlays hold this code.
 */
StageExit EnterStageState(void)
{
    StageExit end = { 0, 0, 0, false, false, 0, 0 };
    StageRegs r = { 0, 0, 0, false, false, 0 };
    StageExit card;
    uint8_t a;

    a = RD8(ZP->field86);
    WR8(ZP->field83, a);
    WR8(ZP->field8a, 0);
    WR8(ZP->field8f, 0);
    RMW8(ZP->field8e, flags, flags & ~FIELD8E_ENDING);
    a = RD8(ZP->stage);
    end.a = a;
    end.nz = a;
    if ((a & 0x80) != 0) {
        r.a = a;
        r.nz = a;
        do {
            stage_regs_load_shim(r);
            call_orig(CARD_RETURN, CARD_CALL);
            card = stage_exit_from_cpu_shim();
            r = stage_regs_after(card, r);
            r.x = r.a;
            r.nz = r.a;
        } while (r.a != 0);
        RMW8(ZP->field8e, flags, flags | FIELD8E_READY);
        end.a = r.a;
        end.x = r.x;
        end.y = r.y;
        end.c = r.c;
        end.v = r.v;
        end.nz = r.nz;
        end.set = EXIT_X | EXIT_Y | EXIT_C | EXIT_V;
    }
    a = RD8(ZP->game_running);
    end.a = a;
    end.nz = a;
    if (a != 0) {
        end.nz = INC8(ZP->mode_done);
        return end;
    }
    WR8(ZP->raster_effect, 0);
    WR8(ZP->display_delay, DISPLAY_DELAY);
    WR8(ZP->game_sub_mode, 0);
    end.a = DISPLAY_DELAY;
    end.nz = DISPLAY_DELAY;
    return end;
}

void abi_EnterStageState(void)
{
    stage_exit_shim(EnterStageState());
}
