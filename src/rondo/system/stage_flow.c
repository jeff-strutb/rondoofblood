/*
 * The stage overlay's frame steps (sector 024D and on, bank $6B at $8000, as the first stage loads it): the fade's
 * countdown, the pause, and the sub-steps that send the stage to its next sub-mode. Each routine takes the registers
 * and flags it is entered with and gives back those it leaves (StageRegs), as the stage overlay's other routines do.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "syscard.h"
#include "enemy_objects.h"
#include "stage_map.h"
#include "stage_regs.h"
#include "stage_flow_b.h"

typedef StageRegs Regs;

/* Defined elsewhere, typed, and not declared in a header. */
ObjArm MapOverlayBank7(void);                       /* object/player_leaves.c */

enum {
    PAD_RUN = 0x08, FIELD8E_SOUND = 0x08, FIELD8E_KEEP_CD = 0x10, FIELD8E_NO_FADE = 0x18,
    FADE_TYPE_OUT = 0x0C, FADE_TYPE_NONE = 0x00, FADE_RESTART = 2, PAUSED_HELD = 0x02, PAUSED_LOCK = 0x03,
    SCROLL_SPEEDS_OFF_RUN = 0x80C5, SUB_MODE_PLAY = 2, SUB_MODE_FADE = 4, SUB_MODE_TALLY = 6, FADE_STEP_LATE = 5, TALLY_WAIT = 0x40
};

#define loaded regs_loaded
#define entry_shim regs_entry_shim

/* A tail JMP to the routine loaded at run, from the jump at site: the registers as they stand, and what it leaves. */
static Regs tail_to(Regs r, uint16_t site, uint16_t run)
{
    stage_regs_load_shim(r);
    run_routine_from(site, run);
    return stage_regs_from_cpu_shim(r);
}

/* A JSR to the routine loaded at run: the registers as they stand, and what it leaves. The overlays holding these
 * sub-steps differ at $80C5 (10BD and 12CD jump to $A3F0, the others to $A067), so the call goes to the one loaded. */
static Regs call_to(Regs r, uint16_t ret, uint16_t run)
{
    stage_regs_load_shim(r);
    call_routine(ret, run);
    return stage_regs_from_cpu_shim(r);
}

/* The sound hook ($401B, SoundCommand) reached by a tail JMP with A the command: what it leaves. */
static Regs sound_tail(Regs r, uint8_t command)
{
    CommandCall call;

    r = loaded(r, command);
    stage_regs_load_shim(r);
    call = SoundCommand(command, r.x, r.y);
    sound_command_exit_shim(command, call);
    return stage_regs_from_cpu_shim(r);
}

/* The card's cd_fade, reached by a tail JMP: A the nibble it returns, N and Z from it; X, Y, C and V kept. */
static Regs fade_tail(Regs r, uint8_t type)
{
    return loaded(r, vec_cd_fade(type));
}

/*
 * @routine 024D:009 StageVec8009_024D
 * @run 6B:8009
 * WHAT IT DOES: Goes to ClearStageBitRowsFour.
 * HOW: One jump.
 * WAITS: where the routine at $92D9 waits; the jump adds nothing to the stack.
 * Entry, exit: as the routine at $92D9.
 */
void abi_StageVec8009_024D(void)
{
    run_routine(0x92D9);
}

/*
 * @routine 024D:0C5 StageScrollSpeedsOff
 * @run 6B:80C5
 * WHAT IT DOES: Stops the layers' scroll: maps the overlay's bank 7 and runs its ClearScrollSpeeds.
 * HOW: MapOverlayBank7, then the tail JMP to $A067, the C of the routine loaded there.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as ClearScrollSpeeds leaves them, entered with A the bank and N, Z, C and V
 * from the map's add.
 */
StageRegs StageScrollSpeedsOff(StageRegs r)
{
    ObjArm bank;

    jsr_push(0x80C8);
    bank = MapOverlayBank7();
    jsr_pop();
    r = loaded(r, bank.sum.value);
    r.c = bank.sum.carry;
    r.v = bank.sum.overflow;
    return tail_to(r, 0x80C8, 0xA067);
}

void abi_StageScrollSpeedsOff(void)
{
    stage_regs_load_shim(StageScrollSpeedsOff(entry_shim()));
}

/*
 * @routine 024D:178 StageFadeTimer
 * @run 6B:8178
 * WHAT IT DOES: The fade's countdown, two a frame. When the count reaches zero on the first step the fade is running
 * (zFadeActive 1) and, unless zField8E's bits 3 or 4 are set, the card starts its CD fade of type $0C; when it
 * reaches zero on the second the mark is cleared and, unless bit 4 is set, the sound zFadeSound goes out (bit 3 set)
 * or the CD fade of type 0.
 * HOW: Two DECs of zFadeCount with a test after each; BBS4 and BBS3 of zField8E, each a read of it; the sound hook
 * at $401B or the card's cd_fade, reached by a tail JMP.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: count zero: A 0, N and Z from it; else N and Z from the last DEC or test, A the
 * value the last test loaded; after the sound or the fade, as they leave them. X and Y kept.
 */
StageRegs StageFadeTimer(StageRegs r)
{
    uint8_t left;

    r = loaded(r, RD8(ZP->fade_count));
    if (r.a == 0) {
        return r;
    }
    left = DEC8(ZP->fade_count);
    r.nz = left;
    if (left == 0) {
        r = loaded(r, 1);
        WR8(ZP->fade_active, 1);
        r = loaded(r, (uint8_t)(RD8(ZP->field8e) & FIELD8E_NO_FADE));
        if (r.a != 0) {
            return r;
        }
        return fade_tail(r, FADE_TYPE_OUT);
    }
    r.nz = DEC8(ZP->fade_count);
    WR8(ZP->fade_active, 0);
    if (RD8(ZP->field8e) & FIELD8E_KEEP_CD) {
        return r;
    }
    if (RD8(ZP->field8e) & FIELD8E_SOUND) {
        return sound_tail(r, RD8(ZP->fade_sound));
    }
    r.a = 0;
    return fade_tail(r, FADE_TYPE_NONE);
}

void abi_StageFadeTimer(void)
{
    stage_regs_load_shim(StageFadeTimer(entry_shim()));
}

/*
 * @routine 024E:6D4 StagePauseSet
 * @run 6B:8ED4
 * WHAT IT DOES: Pauses the stage. While the opening sequence's toggle is set, or the stage is paused already, the
 * pause is held (zPaused bits 0 and 1 set); otherwise zPaused is 1 and the opening sequence's frame count is cleared.
 * HOW: StageSequenceToggle, BBS0 of zPaused (a read), STZ StageSequenceDelay; one store to zPaused.
 * WAITS: no.
 * Entry: nothing. Exit: A zPaused's new value, N and Z from it. X, Y, C and V kept.
 */
StageRegs StagePauseSet(StageRegs r)
{
    uint8_t value;

    r = loaded(r, rd8(StageSequenceToggle));
    if (r.a == 0 && !(RD8(ZP->paused) & 0x01)) {
        wr8(StageSequenceDelay, 0);
        r = loaded(r, 1);
    } else {
        value = (uint8_t)(RD8(ZP->paused) | PAUSED_LOCK);
        r = loaded(r, value);
    }
    WR8(ZP->paused, r.a);
    return r;
}

void abi_StagePauseSet(void)
{
    stage_regs_load_shim(StagePauseSet(entry_shim()));
}

/*
 * @routine 024D:1A2 StagePauseToggle
 * @run 6B:81A2
 * WHAT IT DOES: Pauses the stage when RUN is pressed, unless the fade is running, the display is still off, or
 * zPaused's bit 1 holds the pause.
 * HOW: zFadeActive XOR 1 must not be zero; BiosJoyTrigger's bit 3; zDisplayDelay zero; BBS1 of zPaused (a read);
 * StagePauseSet.
 * WAITS: no.
 * Entry: nothing. Exit: A the last value loaded or computed, N and Z from it; or as StagePauseSet. X, Y, C and V kept.
 */
StageRegs StagePauseToggle(StageRegs r)
{
    r = loaded(r, (uint8_t)(RD8(ZP->fade_active) ^ 0x01));
    if (r.a == 0) {
        return r;
    }
    r = loaded(r, (uint8_t)(rd8(BiosJoyTrigger) & PAD_RUN));
    if (r.a == 0) {
        return r;
    }
    r = loaded(r, RD8(ZP->display_delay));
    if (r.a != 0) {
        return r;
    }
    if (RD8(ZP->paused) & PAUSED_HELD) {
        return r;
    }
    jsr_push(0x81B9);
    r = StagePauseSet(r);
    jsr_pop();
    return r;
}

void abi_StagePauseToggle(void)
{
    stage_regs_load_shim(StagePauseToggle(entry_shim()));
}

/* The sub-steps' start: zGameSubMode and zField80 set, zField89 cleared, StageScrollSpeedsOff by JSR at ret. */
static Regs sub_step_start(Regs r, uint8_t sub_mode, uint16_t ret)
{
    r = loaded(r, sub_mode);
    WR8(ZP->game_sub_mode, sub_mode);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    return call_to(r, ret, SCROLL_SPEEDS_OFF_RUN);
}

/* With the time-slowing mode set it is cleared and the fade runs again from 2, by the tail JMP to StageFadeTimer. */
static Regs restart_fade_if_slowed(Regs r)
{
    r = loaded(r, rd8(TimeSlowMode));
    if (r.a == 0) {
        return r;
    }
    wr8(TimeSlowMode, 0);
    r = loaded(r, FADE_RESTART);
    WR8(ZP->fade_count, FADE_RESTART);
    return StageFadeTimer(r);
}

/*
 * @routine 024D:23D StageSubStepDeath
 * @run 6B:823D
 * WHAT IT DOES: Sub-step for the player's death: sub-mode 2 with zField80 and zField89 cleared, the scroll stopped,
 * the forced presses and the fall's step cleared; when the time-slowing mode is set it is cleared and the fade
 * starts again from 2.
 * HOW: Stores; StageScrollSpeedsOff; TimeSlowMode; the tail JMP to StageFadeTimer.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: A TimeSlowMode (0), N and Z from it, the rest as StageScrollSpeedsOff left them;
 * else as StageFadeTimer.
 */
StageRegs StageSubStepDeath(StageRegs r)
{
    r = sub_step_start(r, SUB_MODE_PLAY, 0x8248);
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    WR8(ZP->fall_step, 0);
    return restart_fade_if_slowed(r);
}

void abi_StageSubStepDeath(void)
{
    stage_regs_load_shim(StageSubStepDeath(entry_shim()));
}

/*
 * @routine 024D:2A2 StageSubStepFadeOut
 * @run 6B:82A2
 * WHAT IT DOES: Sub-step to the fade: sub-mode 4 from its first step, zField89 cleared, the scroll stopped.
 * HOW: Stores, then the tail JMP to $80C5 (StageScrollSpeedsOff here), the C of the routine loaded there.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as StageScrollSpeedsOff.
 */
StageRegs StageSubStepFadeOut(StageRegs r)
{
    r = loaded(r, SUB_MODE_FADE);
    WR8(ZP->game_sub_mode, SUB_MODE_FADE);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    return tail_to(r, 0x82AA, SCROLL_SPEEDS_OFF_RUN);
}

void abi_StageSubStepFadeOut(void)
{
    stage_regs_load_shim(StageSubStepFadeOut(entry_shim()));
}

/*
 * @routine 024D:2AD StageSubStepFadeOutLate
 * @run 6B:82AD
 * WHAT IT DOES: Sub-step to the fade at its step 5: sub-mode 4, zField80 5, zField89 cleared, the scroll stopped.
 * HOW: Stores, then the tail JMP to $80C5 (StageScrollSpeedsOff here), the C of the routine loaded there.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as StageScrollSpeedsOff.
 */
StageRegs StageSubStepFadeOutLate(StageRegs r)
{
    r = loaded(r, SUB_MODE_FADE);
    WR8(ZP->game_sub_mode, SUB_MODE_FADE);
    r = loaded(r, FADE_STEP_LATE);
    WR8(ZP->field80, FADE_STEP_LATE);
    WR8(ZP->field89, 0);
    return tail_to(r, 0x82B7, SCROLL_SPEEDS_OFF_RUN);
}

void abi_StageSubStepFadeOutLate(void)
{
    stage_regs_load_shim(StageSubStepFadeOutLate(entry_shim()));
}

/*
 * @routine 024D:2BA StageSubStepTally
 * @run 6B:82BA
 * WHAT IT DOES: Sub-step to the tally: sub-mode 6 with zField80 and zField89 cleared, the scroll stopped, the forced
 * presses and the fall's step cleared, the player's work bytes F cleared and E $40; when the time-slowing mode is
 * set it is cleared and the fade starts again from 2.
 * HOW: Stores; StageScrollSpeedsOff; TimeSlowMode; the tail JMP to StageFadeTimer.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: A TimeSlowMode (0), N and Z from it, the rest as StageScrollSpeedsOff left them;
 * else as StageFadeTimer.
 */
StageRegs StageSubStepTally(StageRegs r)
{
    r = sub_step_start(r, SUB_MODE_TALLY, 0x82C5);
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    WR8(ZP->fall_step, 0);
    WR8(OBJ->work_f[0], 0);
    r = loaded(r, TALLY_WAIT);
    WR8(OBJ->work_e[0], TALLY_WAIT);
    return restart_fade_if_slowed(r);
}

void abi_StageSubStepTally(void)
{
    stage_regs_load_shim(StageSubStepTally(entry_shim()));
}

/*
 * @routine 024D:3E9 StageFallStepNext
 * @run 6B:83E9
 * WHAT IT DOES: Counts the fall's step on.
 * HOW: INC zFallStep.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the new step. A, X, Y, C and V kept.
 */
StageRegs StageFallStepNext(StageRegs r)
{
    r.nz = INC8(ZP->fall_step);
    return r;
}

void abi_StageFallStepNext(void)
{
    stage_regs_load_shim(StageFallStepNext(entry_shim()));
}

/* JSR $8A4F, StepScrollAndRunObjects: it ends in the code at $A000, so what it leaves is the machine's. */
static Regs step_and_run_objects(Regs r, uint16_t ret)
{
    stage_regs_load_shim(r);
    jsr_push(ret);
    StepScrollAndRunObjects(r.x);
    jsr_pop();
    return stage_regs_from_cpu_shim(r);
}

/*
 * @routine 024D:4C7 StageClearedWaitScroll
 * @run 6B:84C7
 * WHAT IT DOES: A cleared stage's step: the scroll and the objects run, and once zField8C's bit 7 is set the next
 * step comes.
 * HOW: StepScrollAndRunObjects; zField8C negative counts zField85 on.
 * WAITS: no.
 * Entry: X the caller's. Exit: A zField8C, N and Z from it or from zField85's INC; X, Y, C and V as the objects left
 * them.
 */
StageRegs StageClearedWaitScroll(StageRegs r)
{
    r = step_and_run_objects(r, 0x84CA);
    r = loaded(r, RD8(ZP->field8c));
    if (r.a & 0x80) {
        r.nz = INC8(ZP->field85);
    }
    return r;
}

void abi_StageClearedWaitScroll(void)
{
    stage_regs_load_shim(StageClearedWaitScroll(entry_shim()));
}

/*
 * @routine 024D:66A StageSectionNext
 * @run 6B:866A
 * WHAT IT DOES: Moves the stage on to its next section: zField82 counted on and zField83 set from SectionOrder by it.
 * HOW: INC zField82, read again as the index, one table read, one store.
 * WAITS: no.
 * Entry: nothing. Exit: A zField83's new value, N and Z from it, Y the index. X, C and V kept.
 */
StageRegs StageSectionNextAt(StageRegs r, uint16_t order)
{
    INC8(ZP->field82);
    r.y = RD8(ZP->field82);
    r = loaded(r, rd8((uint16_t)(order + r.y)));
    WR8(ZP->field83, r.a);
    return r;
}

StageRegs StageSectionNext(StageRegs r)
{
    return StageSectionNextAt(r, SectionOrder);
}

void abi_StageSectionNext(void)
{
    stage_regs_load_shim(StageSectionNext(entry_shim()));
}

/*
 * @routine 024D:695 StageClearedScrollOn
 * @run 6B:8695
 * WHAT IT DOES: A cleared stage's step: the scroll and the objects run, zField8C's bit 7 is cleared and the next step
 * comes.
 * HOW: StepScrollAndRunObjects; RMB7 of zField8C; INC zField85.
 * WAITS: no.
 * Entry: X the caller's. Exit: N and Z from zField85's INC; A, X, Y, C and V as the objects left them.
 */
StageRegs StageClearedScrollOn(StageRegs r)
{
    r = step_and_run_objects(r, 0x8698);
    RMW8(ZP->field8c, old, old & 0x7F);
    r.nz = INC8(ZP->field85);
    return r;
}

void abi_StageClearedScrollOn(void)
{
    stage_regs_load_shim(StageClearedScrollOn(entry_shim()));
}
