/*
 * Stage objects shared by the stages' object banks (sectors 0250, 0259, 025A and 025C as the first stage loads them):
 * short handlers and the pieces of the one-at-a-time flash object. Each takes the object in X and the registers it is
 * entered with, and gives back those it leaves.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "stage_regs.h"

/* Defined elsewhere, typed, and not declared in a header. */
void ClearObject(uint8_t object);                    /* object/lifecycle.c */

typedef StageRegs Regs;

/* A JSR to the routine loaded at run: the registers as they stand, and what it leaves. */
static Regs call_to(Regs r, uint16_t ret, uint16_t run)
{
    stage_regs_load_shim(r);
    call_routine(ret, run);
    return stage_regs_from_cpu_shim(r);
}

/* A JSR at ret to the sound hook with A the command: what it leaves. */
static Regs sound_jsr(Regs r, uint8_t command, uint16_t ret)
{
    CommandCall call;

    r = regs_loaded(r, command);
    stage_regs_load_shim(r);
    jsr_push(ret);
    call = SoundCommand(command, r.x, r.y);
    jsr_pop();
    sound_command_exit_shim(command, call);
    return stage_regs_from_cpu_shim(r);
}

/*
 * @routine 0259:36A StageObjectRemove
 * @run 70:A36A
 * WHAT IT DOES: Empties the object's slot.
 * HOW: The tail JMP to the kernel's ClearObject vector.
 * WAITS: no.
 * Entry: X object. Exit: A, X, Y and flags kept.
 */
StageRegs StageObjectRemove(StageRegs r)
{
    ClearObject(r.x);
    return r;
}

void abi_StageObjectRemove(void)
{
    stage_regs_load_shim(StageObjectRemove(regs_entry_shim()));
}

/*
 * @routine 0259:5DA StageObjectBecomeScore
 * @run 70:A5DA
 * WHAT IT DOES: Turns the object into the score marker (kind $9F) after blending the score added: its OBJ_MARKER_VALUE from
 * the value in Y, its state cleared.
 * HOW: PHY, BlendStageScoreAddRows by its $9181 C, PLA.
 * WAITS: no.
 * Entry: X object, Y the marker's value. Exit: A $9F, N and Z from it; X, Y, C and V as the blend left them.
 */
StageRegs StageObjectBecomeScore(StageRegs r)
{
    uint8_t value;

    push8(r.y);
    r = call_to(r, 0xA5DE, 0x9181);
    value = pull8();
    r = regs_loaded(r, value);
    WR8(OBJ->marker_value[r.x], value);
    r = regs_loaded(r, 0x9F);
    WR8(OBJ->kind[r.x], 0x9F);
    WR8(OBJ->state[r.x], 0);
    return r;
}

void abi_StageObjectBecomeScore(void)
{
    stage_regs_load_shim(StageObjectBecomeScore(regs_entry_shim()));
}

/*
 * @routine 0259:675 StageFlashStart
 * @run 70:A675
 * WHAT IT DOES: Starts the screen flash unless one is running, in which case the object is cleared: the sound $50, the
 * sole script slot 3 with script 0, $12 frames, bit 1 of HitAttackFlags set, the object's frame cleared and its state
 * counted on.
 * HOW: StageFlashActive; the sound hook; StartSoleScriptSlot with X kept on the stack.
 * WAITS: no.
 * Entry: X object. Exit: running: as ClearObject (A the flag); else N and Z from the state's INC, A HitAttackFlags.
 */
StageRegs StageFlashStart(StageRegs r)
{
    uint8_t value;

    r = regs_loaded(r, rd8(StageFlashActive));
    if (r.a != 0) {
        ClearObject(r.x);
        return r;
    }
    r = regs_loaded(r, 0x01);
    wr8(StageFlashActive, 0x01);
    r = sound_jsr(r, 0x50, 0xA687);
    push8(r.x);
    r.a = 0;                                   /* CLA: no flags */
    r.x = 3;
    r.nz = 3;                                  /* LDX */
    r = call_to(r, 0xA68E, 0x4042);
    value = pull8();
    r.x = value;
    r.nz = value;
    r = regs_loaded(r, 0x12);
    WR8(OBJ->flash_frames[r.x], 0x12);
    value = (uint8_t)(rd8(HitAttackFlags) | 0x02);
    r = regs_loaded(r, value);
    wr8(HitAttackFlags, value);
    WR8(OBJ->frame[r.x], 0);
    r.nz = INC8(OBJ->state[r.x]);
    return r;
}

void abi_StageFlashStart(void)
{
    stage_regs_load_shim(StageFlashStart(regs_entry_shim()));
}

/*
 * @routine 025A:009 StageFlashEnd
 * @run 70:A809
 * WHAT IT DOES: The flash's countdown: bit 1 of HitAttackFlags cleared; when the object's OBJ_FLASH_FRAMES runs out the
 * flash is over and the object cleared.
 * HOW: AND and a store, DEC, the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: A HitAttackFlags' new value; counting N and Z from the DEC, else as ClearObject.
 */
StageRegs StageFlashEnd(StageRegs r)
{
    uint8_t value = (uint8_t)(rd8(HitAttackFlags) & 0xFD);

    r = regs_loaded(r, value);
    wr8(HitAttackFlags, value);
    r.nz = DEC8(OBJ->flash_frames[r.x]);
    if (r.nz != 0) {
        return r;
    }
    wr8(StageFlashActive, 0);
    ClearObject(r.x);
    return r;
}

void abi_StageFlashEnd(void)
{
    stage_regs_load_shim(StageFlashEnd(regs_entry_shim()));
}

/*
 * @routine 0259:6A3 StagePickupPotion
 * @run 70:A6A3
 * WHAT IT DOES: The potion taken: sound $0B, the player's invincibility timer $80, the object cleared.
 * HOW: The sound hook, a store, the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: A $80, N and Z from it; X, Y, C and V as the sound left them.
 */
StageRegs StagePickupPotion(StageRegs r)
{
    r = sound_jsr(r, 0x0B, 0xA6A8);
    r = regs_loaded(r, 0x80);
    wr8(PlayerInvincibleTimer, 0x80);
    ClearObject(r.x);
    return r;
}

void abi_StagePickupPotion(void)
{
    stage_regs_load_shim(StagePickupPotion(regs_entry_shim()));
}

/*
 * @routine 025C:126 StageObjectRiseToTop
 * @run 70:B926
 * WHAT IT DOES: Moves the object up a pixel while its Y low byte is $30 or more.
 * HOW: CMP, DEC.
 * WAITS: no.
 * Entry: X object. Exit: A its Y low, C 1 when it moved, N and Z from the compare or the DEC.
 */
StageRegs StageObjectRiseToTop(StageRegs r)
{
    r = regs_loaded(r, RD8(OBJ->y_lo[r.x]));
    r = regs_compared(r, r.a, 0x30);
    if (r.c) {
        r.nz = DEC8(OBJ->y_lo[r.x]);
    }
    return r;
}

void abi_StageObjectRiseToTop(void)
{
    stage_regs_load_shim(StageObjectRiseToTop(regs_entry_shim()));
}

/*
 * @routine 0250:29D StageGateAdvance
 * @run 6B:9A9D
 * WHAT IT DOES: Unless zField8C is negative, advances the object's state with $04 in its OBJ_GATE_WAIT, twice for the
 * start arguments whose low nibble is 2 to 5, $0A or $0B.
 * HOW: CMP chain on OBJ_START_ARG's low nibble.
 * WAITS: no.
 * Entry: X object. Exit: negative: A zField8C; else A the nibble, N, Z and C from the last compare or INC.
 */
StageRegs StageGateAdvance(StageRegs r)
{
    uint8_t value;
    bool again;

    r = regs_loaded(r, RD8(ZP->field8c));
    if (r.a & 0x80) {
        return r;
    }
    r.nz = INC8(OBJ->state[r.x]);
    r = regs_loaded(r, 0x04);
    WR8(OBJ->gate_wait[r.x], 0x04);
    value = (uint8_t)(RD8(OBJ->start_arg[r.x]) & 0x0F);
    r = regs_loaded(r, value);
    r = regs_compared(r, value, 0x02);
    again = false;
    if (r.c) {
        r = regs_compared(r, value, 0x06);
        if (!r.c) {
            again = true;
        } else {
            r = regs_compared(r, value, 0x0A);
            if (value == 0x0A) {
                again = true;
            } else {
                r = regs_compared(r, value, 0x0B);
                again = value == 0x0B;
            }
        }
    }
    if (again) {
        r.nz = INC8(OBJ->state[r.x]);
    }
    return r;
}

void abi_StageGateAdvance(void)
{
    stage_regs_load_shim(StageGateAdvance(regs_entry_shim()));
}
