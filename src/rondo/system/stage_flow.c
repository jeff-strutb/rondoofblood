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

/* Entries that run into one another, declared ahead of their callers. */
StageRegs StageStartPlain(StageRegs r);
StageRegs StageStartMusicByStage(StageRegs r);
StageRegs StageStartMusicKeep(StageRegs r);
StageRegs StageStartMusicBoss(StageRegs r);
StageRegs StageStartDone(StageRegs r);
StageRegs StageStartSound(StageRegs r);
StageRegs StageStartBossMusic(StageRegs r);
StageRegs StageFallRightLand(StageRegs r);
StageRegs StageFallRightDone(StageRegs r);
StageRegs StageFallLeftLand(StageRegs r);
StageRegs StageFallLeftDone(StageRegs r);
StageRegs StageFallPose(StageRegs r);

/* Defined elsewhere, typed, and not declared in a header. */
ObjArm MapOverlayBank7(void);                       /* object/player_leaves.c */
Ended MapOverlayBanks5And6(void);                   /* object/player_leaves.c */

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
    return StageScrollSpeedsOffAt(r, 0x80C8);
}

/* StageScrollSpeedsOff's body, ret the address after its JSR to MapOverlayBank7 (where the JMP that follows starts):
 * the builds of the overlay that hold these instructions at another address (stage_programs_census.c) pass their own. */
StageRegs StageScrollSpeedsOffAt(StageRegs r, uint16_t ret)
{
    ObjArm bank;

    jsr_push(ret);
    bank = MapOverlayBank7();
    jsr_pop();
    r = loaded(r, bank.sum.value);
    r.c = bank.sum.carry;
    r.v = bank.sum.overflow;
    return tail_to(r, ret, 0xA067);
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
    return StagePauseToggleAt(r, 0x81B9);
}

/* StagePauseToggle's body, ret the address after its JSR to StagePauseSet: the builds of the overlay that hold these
 * instructions at another address (stage_programs_census.c) pass their own. */
StageRegs StagePauseToggleAt(StageRegs r, uint16_t ret)
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
    jsr_push(ret);
    r = StagePauseSet(r);
    jsr_pop();
    return r;
}

void abi_StagePauseToggle(void)
{
    stage_regs_load_shim(StagePauseToggle(entry_shim()));
}

/* The sub-steps' start: zGameSubMode and zField80 set, zField89 cleared, StageScrollSpeedsOff (at scroll_off) by JSR
 * at ret. */
static Regs sub_step_start(Regs r, uint8_t sub_mode, uint16_t ret, uint16_t scroll_off)
{
    r = loaded(r, sub_mode);
    WR8(ZP->game_sub_mode, sub_mode);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    return call_to(r, ret, scroll_off);
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
    return StageSubStepDeathAt(r, 0x8248, SCROLL_SPEEDS_OFF_RUN);
}

/* StageSubStepDeath's body, ret the address after its JSR to StageScrollSpeedsOff and scroll_off where that JSR goes:
 * the builds of the overlay that hold these instructions at other addresses (stage_programs_census.c) pass their own. */
StageRegs StageSubStepDeathAt(StageRegs r, uint16_t ret, uint16_t scroll_off)
{
    r = sub_step_start(r, SUB_MODE_PLAY, ret, scroll_off);
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
    return StageSubStepFadeOutAt(r, 0x82AA, SCROLL_SPEEDS_OFF_RUN);
}

/* StageSubStepFadeOut's body, site the address of its tail JMP to StageScrollSpeedsOff and scroll_off where that JMP goes:
 * the builds of the overlay that hold these instructions at other addresses (stage_programs_census.c) pass their own. */
StageRegs StageSubStepFadeOutAt(StageRegs r, uint16_t site, uint16_t scroll_off)
{
    r = loaded(r, SUB_MODE_FADE);
    WR8(ZP->game_sub_mode, SUB_MODE_FADE);
    WR8(ZP->field80, 0);
    WR8(ZP->field89, 0);
    return tail_to(r, site, scroll_off);
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
    return StageSubStepFadeOutLateAt(r, 0x82B7, SCROLL_SPEEDS_OFF_RUN);
}

/* StageSubStepFadeOutLate's body, site the address of its tail JMP to StageScrollSpeedsOff and scroll_off where that JMP goes:
 * the builds of the overlay that hold these instructions at other addresses (stage_programs_census.c) pass their own. */
StageRegs StageSubStepFadeOutLateAt(StageRegs r, uint16_t site, uint16_t scroll_off)
{
    r = loaded(r, SUB_MODE_FADE);
    WR8(ZP->game_sub_mode, SUB_MODE_FADE);
    r = loaded(r, FADE_STEP_LATE);
    WR8(ZP->field80, FADE_STEP_LATE);
    WR8(ZP->field89, 0);
    return tail_to(r, site, scroll_off);
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
    return StageSubStepTallyAt(r, 0x82C5, SCROLL_SPEEDS_OFF_RUN);
}

/* StageSubStepTally's body, ret the address after its JSR to StageScrollSpeedsOff and scroll_off where that JSR goes:
 * the builds of the overlay that hold these instructions at other addresses (stage_programs_census.c) pass their own. */
StageRegs StageSubStepTallyAt(StageRegs r, uint16_t ret, uint16_t scroll_off)
{
    r = sub_step_start(r, SUB_MODE_TALLY, ret, scroll_off);
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    WR8(ZP->fall_step, 0);
    WR8(OBJ->tally_step[0], 0);
    r = loaded(r, TALLY_WAIT);
    WR8(OBJ->tally_wait[0], TALLY_WAIT);
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

/*
 * @routine 024D:0BF StageCameraStep
 * @run 6B:80BF
 * WHAT IT DOES: The camera's frame with the overlay's bank 7 mapped.
 * HOW: MapOverlayBank7, then the tail JMP to $A08D, the C of the routine loaded there.
 * WAITS: no.
 * Entry: nothing. Exit: as CameraStep, entered with A the bank, N, Z, C and V from the map's add.
 */
StageRegs StageCameraStep(StageRegs r)
{
    return StageCameraStepAt(r, 0x80C2);
}

/* StageCameraStep's body, ret the address after its JSR to MapOverlayBank7 (where the JMP that follows starts):
 * the builds of the overlay that hold these instructions at another address (stage_programs_census.c) pass their own. */
StageRegs StageCameraStepAt(StageRegs r, uint16_t ret)
{
    ObjArm bank;

    jsr_push(ret);
    bank = MapOverlayBank7();
    jsr_pop();
    r = loaded(r, bank.sum.value);
    r.c = bank.sum.carry;
    r.v = bank.sum.overflow;
    return tail_to(r, ret, 0xA08D);
}

void abi_StageCameraStep(void)
{
    stage_regs_load_shim(StageCameraStep(entry_shim()));
}

/* JSR MapOverlayBanks5And6 at ret: A the second bank, N and Z from it, C and V from the add. */
static Regs map_banks_5_6(Regs r, uint16_t ret)
{
    Ended e;

    jsr_push(ret);
    e = MapOverlayBanks5And6();
    jsr_pop();
    if (e.a_set) {
        r.a = e.a;
    }
    if (e.nz_set) {
        r.nz = e.nz;
    }
    if (e.c_set) {
        r.c = e.c;
    }
    if (e.v_set) {
        r.v = e.v;
    }
    return r;
}

/* The body of StageForcedWalk and StageForcedWalkB, whose bytes are the same (both read the table at $833E): init_ret
 * the address after the JSR to InitStage, init where it goes, pads the table of forced pads. The builds of the overlay
 * that hold these instructions at other addresses (stage_programs_census.c) pass their own. */
StageRegs StageForcedWalkAt(StageRegs r, uint16_t init_ret, uint16_t init, uint16_t pads)
{
    uint8_t side;

    r = call_to(r, init_ret, init);
    r = loaded(r, 0x01);
    WR8(ZP->game_sub_mode, 0x01);
    r = loaded(r, RD8(ZP->field8c));
    if (r.a != 0) {
        WR8(ZP->field8c, 0);
        r = loaded(r, 0x18);
        WR8(OBJ->pad_force_frames[0], 0x18);
    }
    side = (uint8_t)((RD8(OBJ->f2ac0[0]) ^ 0x01) & 0x01);
    r.x = side;
    r = loaded(r, rd8((uint16_t)(pads + side)));
    WR8(OBJ->pad_forced[0], r.a);
    return r;
}

/*
 * @routine 024D:31D StageForcedWalk
 * @run 6B:831D
 * WHAT IT DOES: A sub-mode 3 start (zField87 0): the stage set up (InitStage), sub-mode 1, and the player walked by a
 * forced pad away from the side he faces: $18 frames when zField8C is set (it is cleared).
 * HOW: JSR InitStage; zGameSubMode 1; OBJ_PAD_FORCE_FRAMES; facing XOR 1 picks the pad from ForcedWalkPads.
 * WAITS: where InitStage waits.
 * Entry: nothing. Exit: A the pad, N and Z from it, X the table index; Y, C and V as InitStage left them.
 */
StageRegs StageForcedWalk(StageRegs r)
{
    return StageForcedWalkAt(r, 0x8320, 0x88EC, ForcedWalkPads);
}

void abi_StageForcedWalk(void)
{
    stage_regs_load_shim(StageForcedWalk(entry_shim()));
}

/*
 * @routine 024D:340 StageForcedWalkB
 * @run 6B:8340
 * WHAT IT DOES: A sub-mode 3 start (zField87 1): as StageForcedWalk, the same bytes at their own address.
 * HOW: As StageForcedWalk.
 * WAITS: where InitStage waits.
 * Entry, exit: as StageForcedWalk.
 */
StageRegs StageForcedWalkB(StageRegs r)
{
    return StageForcedWalkAt(r, 0x8343, 0x88EC, ForcedWalkPads);
}

void abi_StageForcedWalkB(void)
{
    stage_regs_load_shim(StageForcedWalkB(entry_shim()));
}

/* The forced-pad ends of the fall starts: InitStage, sub-mode 1, zField8C cleared, $20 frames of the pad `pad`. */
static Regs fall_start_done(Regs r, uint16_t init_ret, uint8_t pad)
{
    r = call_to(r, init_ret, 0x88EC);
    r = loaded(r, 0x01);
    WR8(ZP->game_sub_mode, 0x01);
    WR8(ZP->field8c, 0);
    r = loaded(r, 0x20);
    WR8(OBJ->pad_force_frames[0], 0x20);
    r = loaded(r, pad);
    WR8(OBJ->pad_forced[0], pad);
    return r;
}

/*
 * @routine 024D:39C StageFallRightDone
 * @run 6B:839C
 * WHAT IT DOES: The last step of the fall to the right: the stage set up again and the player walked on by pad $20.
 * HOW: JSR InitStage; sub-mode 1, zField8C cleared; $20 frames of pad $20.
 * WAITS: where InitStage waits.
 * Entry: nothing. Exit: A $20, N and Z from it; X, Y, C and V as InitStage left them.
 */
StageRegs StageFallRightDone(StageRegs r)
{
    return fall_start_done(r, 0x839F, 0x20);
}

void abi_StageFallRightDone(void)
{
    stage_regs_load_shim(StageFallRightDone(entry_shim()));
}

/*
 * @routine 024D:3EC StageFallLeftDone
 * @run 6B:83EC
 * WHAT IT DOES: The last step of the fall to the left: as StageFallRightDone with pad $80.
 * HOW: As StageFallRightDone.
 * WAITS: where InitStage waits.
 * Entry: nothing. Exit: A $80, N and Z from it; X, Y, C and V as InitStage left them.
 */
StageRegs StageFallLeftDone(StageRegs r)
{
    return fall_start_done(r, 0x83EF, 0x80);
}

void abi_StageFallLeftDone(void)
{
    stage_regs_load_shim(StageFallLeftDone(entry_shim()));
}

/* The player's X against `limit` past which the fall goes on to its landing ($A01E, X 0, by the tail JMP at site);
 * else zFallStep counts on. */
static Regs fall_until(Regs r, bool right, uint8_t limit, uint16_t site)
{
    r = loaded(r, RD8(OBJ->x_hi[0]));
    if (r.a != 0) {
        r = loaded(r, RD8(OBJ->x_lo[0]));
        r = regs_compared(r, r.a, limit);
        if (right ? r.c : !r.c) {
            return StageFallStepNext(r);
        }
    }
    r.x = 0;
    return tail_to(r, site, 0xA01E);
}

/*
 * @routine 024D:380 StageFallRightLand
 * @run 6B:8380
 * WHAT IT DOES: The fall to the right's middle step: the player idle, his animation sound, and until his X passes $10
 * on a screen past the first the step counts on; else his landing.
 * HOW: MapOverlayBanks5And6, $A01B, $A012; OBJ_X_HI and OBJ_X_LO; the tail JMP to $A01E with X 0, or INC zFallStep.
 * WAITS: no.
 * Entry: nothing. Exit: counting: N and Z from the new step; else as $A01E.
 */
StageRegs StageFallRightLand(StageRegs r)
{
    return StageFallRightLandAt(r, 0x8380);
}

/* StageFallRightLand's body, at the address it runs at (its JSRs return to at + 3, + 6 and + 9, its tail JMP is at
 * at + $16): the builds of the overlay that hold these instructions at another address (stage_programs_census.c) pass
 * their own. */
StageRegs StageFallRightLandAt(StageRegs r, uint16_t at)
{
    r = map_banks_5_6(r, (uint16_t)(at + 3));
    r = call_to(r, (uint16_t)(at + 6), 0xA01B);
    r = call_to(r, (uint16_t)(at + 9), 0xA012);
    return fall_until(r, true, 0x10, (uint16_t)(at + 0x16));
}

void abi_StageFallRightLand(void)
{
    stage_regs_load_shim(StageFallRightLand(entry_shim()));
}

/*
 * @routine 024D:3D0 StageFallLeftLand
 * @run 6B:83D0
 * WHAT IT DOES: The fall to the left's middle step: as StageFallRightLand, counting on while his X is below $F0.
 * HOW: As StageFallRightLand.
 * WAITS: no.
 * Entry, exit: as StageFallRightLand.
 */
StageRegs StageFallLeftLand(StageRegs r)
{
    r = map_banks_5_6(r, 0x83D3);
    r = call_to(r, 0x83D6, 0xA01B);
    r = call_to(r, 0x83D9, 0xA012);
    return fall_until(r, false, 0xF0, 0x8396);
}

void abi_StageFallLeftLand(void)
{
    stage_regs_load_shim(StageFallLeftLand(entry_shim()));
}

/*
 * @routine 024D:373 StageFallPose
 * @run 6B:8373
 * WHAT IT DOES: The fall's first step: the player's animation 0 set and its sound, then the step counted on.
 * HOW: MapOverlayBanks5And6, $A018 with A 0, $A012, BRA to the INC of zFallStep.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the new step; the rest as $A012 left them.
 */
StageRegs StageFallPose(StageRegs r)
{
    return StageFallPoseAt(r, 0x8373);
}

/* StageFallPose's body, at the address it runs at (its JSRs return to at + 3, + 8 and + $0B): the builds of the
 * overlay that hold these instructions at another address (stage_programs_census.c) pass their own. */
StageRegs StageFallPoseAt(StageRegs r, uint16_t at)
{
    r = map_banks_5_6(r, (uint16_t)(at + 3));
    r = loaded(r, 0x00);
    r = call_to(r, (uint16_t)(at + 8), 0xA018);
    r = call_to(r, (uint16_t)(at + 0x0B), 0xA012);
    r.nz = INC8(ZP->fall_step);
    return r;
}

void abi_StageFallPose(void)
{
    stage_regs_load_shim(StageFallPose(entry_shim()));
}

/*
 * @routine 024D:361 StageFallRight
 * @run 6B:8361
 * @stop 8373
 * WHAT IT DOES: The fall to the right (zField87 2), a step a frame: the first sets the player's speed $0140 and runs
 * into StageFallPose; the second StageFallRightLand; the third StageFallRightDone.
 * HOW: zFallStep decremented twice with BEQs.
 * WAITS: as the step taken.
 * Entry: nothing. Exit: as the step taken.
 */
StageRegs StageFallRight(StageRegs r)
{
    r = loaded(r, RD8(ZP->fall_step));
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return StageFallRightLand(r);
    }
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return StageFallRightDone(r);
    }
    r = loaded(r, 0x40);
    WR8(OBJ->vx_lo[0], 0x40);
    r = loaded(r, 0x01);
    WR8(OBJ->vx_hi[0], 0x01);
    return StageFallPose(r);
}

void abi_StageFallRight(void)
{
    stage_regs_load_shim(StageFallRight(entry_shim()));
}

/*
 * @routine 024D:3B0 StageFallLeft
 * @run 6B:83B0
 * @stop 83D0
 * WHAT IT DOES: The fall to the left (zField87 3): the second step StageFallLeftLand, the third StageFallLeftDone; the
 * first, with the object in slot $50 of kind $30, counts the step on twice, else sets the player's speed -$0140, poses
 * him (StageFallPose) and runs into StageFallLeftLand.
 * HOW: zFallStep decremented twice; OBJ_KIND of slot $50; BSR and BRA to StageFallStepNext; JSR StageFallPose.
 * WAITS: as the step taken.
 * Entry: nothing. Exit: as the step taken.
 */
StageRegs StageFallLeft(StageRegs r)
{
    r = loaded(r, RD8(ZP->fall_step));
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return StageFallLeftLand(r);
    }
    r = loaded(r, (uint8_t)(r.a - 1));
    if (r.a == 0) {
        return StageFallLeftDone(r);
    }
    r = loaded(r, RD8(OBJ->kind[0x50]));
    r = regs_compared(r, r.a, 0x30);
    if (r.a == 0x30) {
        jsr_push(0x83C1);
        r = StageFallStepNext(r);
        jsr_pop();
        return StageFallStepNext(r);
    }
    r = loaded(r, 0xC0);
    WR8(OBJ->vx_lo[0], 0xC0);
    r = loaded(r, 0xFE);
    WR8(OBJ->vx_hi[0], 0xFE);
    jsr_push(0x83D0);
    r = StageFallPose(r);
    jsr_pop();
    return StageFallLeftLand(r);
}

void abi_StageFallLeft(void)
{
    stage_regs_load_shim(StageFallLeft(entry_shim()));
}

/*
 * @routine 024D:413 StageRestartInPlace
 * @run 6B:8413
 * WHAT IT DOES: Sets the stage up anew in place: the screen cleared, its lists, VRAM runs, set-up, objects and the
 * player's start, then sub-mode 1 with zField86 from zField83.
 * HOW: ClearScreenState, StartStageLists, ExpandStageVramRuns, InitStage, SpawnStageObjectsRows01, zField8A 2,
 * MapOverlayBanks5And6, $A003, SetUpStageObjects.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: A zField83, N and Z from it; X, Y, C and V as SetUpStageObjects left them.
 */
StageRegs StageRestartInPlace(StageRegs r)
{
    r = call_to(r, 0x8416, 0x4009);
    r = call_to(r, 0x8419, 0x88B1);
    r = call_to(r, 0x841C, 0x8882);
    r = call_to(r, 0x841F, 0x88EC);
    r = call_to(r, 0x8422, 0x8B53);
    r = loaded(r, 0x02);
    WR8(ZP->field8a, 0x02);
    r = map_banks_5_6(r, 0x8429);
    r = call_to(r, 0x842C, 0xA003);
    r = call_to(r, 0x842F, 0x8A5F);
    WR8(ZP->fade_active, 0);
    WR8(ZP->paused, 0);
    r = loaded(r, 0x01);
    WR8(ZP->game_sub_mode, 0x01);
    r = loaded(r, RD8(ZP->field83));
    WR8(ZP->field86, r.a);
    return r;
}

void abi_StageRestartInPlace(void)
{
    stage_regs_load_shim(StageRestartInPlace(entry_shim()));
}

/*
 * @routine 024D:448 StageShowStartFrame
 * @run 6B:8448
 * WHAT IT DOES: The player's frame by character set, then his animation sound.
 * HOW: MapOverlayBanks5And6; OBJ_FRAME of slot 0 from StartFrameByCharacter; the tail JMP to $A012.
 * WAITS: no.
 * Entry: nothing. Exit: as $A012, entered with X zCharacter, A the frame, N and Z from it.
 */
StageRegs StageShowStartFrame(StageRegs r)
{
    r = map_banks_5_6(r, 0x844B);
    r.x = RD8(ZP->character);
    r = loaded(r, rd8((uint16_t)(StartFrameByCharacter + r.x)));
    WR8(OBJ->frame[0], r.a);
    return tail_to(r, 0x8453, 0xA012);
}

void abi_StageShowStartFrame(void)
{
    stage_regs_load_shim(StageShowStartFrame(entry_shim()));
}

/*
 * @routine 024D:260 StageChooseClearKind
 * @run 6B:8260
 * WHAT IT DOES: Chooses how the cleared stage plays out (zClearKind) from zField8C: 0 gives 2, negative 4, else 1 for
 * 1, 2, $0D, $0E and 7 up to $0A but $0B and $0C, and 3 otherwise; then sub-mode 3 with the scroll stopped and the
 * forced presses and time-slowing cleared.
 * HOW: CMP chain on zField8C read once; StageScrollSpeedsOff by the C at $80C5.
 * WAITS: no.
 * Entry: nothing. Exit: as the call at $80C5 and the stores leave them; A, N and Z as that left them.
 */
StageRegs StageChooseClearKind(StageRegs r)
{
    return StageChooseClearKindAt(r, 0x8298, SCROLL_SPEEDS_OFF_RUN);
}

/* StageChooseClearKind's body, ret the address after its JSR to StageScrollSpeedsOff and scroll_off where that JSR
 * goes: the builds of the overlay that hold these instructions at other addresses (stage_programs_census.c) pass their own. */
StageRegs StageChooseClearKindAt(StageRegs r, uint16_t ret, uint16_t scroll_off)
{
    uint8_t kind;

    r = loaded(r, RD8(ZP->field8c));
    if (r.a == 0) {
        kind = 0x02;
    } else if (r.a & 0x80) {
        kind = 0x04;
    } else {
        r.x = 1;
        wr8(HitAttackFlags, 0x01);
        r = regs_compared(r, r.a, 0x03);
        if (!r.c) {
            kind = 0x01;
        } else {
            r = regs_compared(r, r.a, 0x07);
            if (!r.c) {
                kind = 0x03;
            } else {
                r = regs_compared(r, r.a, 0x0B);
                if (r.a == 0x0B) {
                    kind = 0x03;
                } else {
                    r = regs_compared(r, r.a, 0x0C);
                    if (r.a == 0x0C) {
                        kind = 0x03;
                    } else {
                        r = regs_compared(r, r.a, 0x0F);
                        kind = r.c ? 0x03 : 0x01;
                    }
                }
            }
        }
    }
    r = loaded(r, kind);
    WR8(ZP->clear_kind, kind);
    r = loaded(r, 0x03);
    WR8(ZP->game_sub_mode, 0x03);
    WR8(ZP->field85, 0);
    r = call_to(r, ret, scroll_off);
    wr8(PadForcedPress, 0);
    wr8(PadForcedPressB, 0);
    wr8(TimeSlowMode, 0);
    return r;
}

void abi_StageChooseClearKind(void)
{
    stage_regs_load_shim(StageChooseClearKind(entry_shim()));
}

/*
 * @routine 024D:082 StageStartSound
 * @run 6B:8082
 * WHAT IT DOES: Keeps the stage's start sound as zFadeSound and sends it.
 * HOW: STA zFadeSound, the tail JMP to the sound hook.
 * WAITS: no.
 * Entry: A the sound, X and Y the caller's. Exit: as SoundCommand.
 */
StageRegs StageStartSound(StageRegs r)
{
    WR8(ZP->fade_sound, r.a);
    return sound_tail(r, r.a);
}

void abi_StageStartSound(void)
{
    stage_regs_load_shim(StageStartSound(entry_shim()));
}

/*
 * @routine 024D:079 StageStartMusicByStage
 * @run 6B:8079
 * WHAT IT DOES: For a stage of $80 or more: stage $80 itself nothing; else the music stopped and the sound $5A.
 * HOW: CMP #$80; JSR the stop vector; running into StageStartSound.
 * WAITS: no.
 * Entry: A zStage. Exit: stage $80: N and Z from the compare, C 1; else as StageStartSound.
 */
StageRegs StageStartMusicByStage(StageRegs r)
{
    r = regs_compared(r, r.a, 0x80);
    if (r.a == 0x80) {
        return r;
    }
    r = call_to(r, 0x8080, 0x4087);
    r = loaded(r, 0x5A);
    return StageStartSound(r);
}

void abi_StageStartMusicByStage(void)
{
    stage_regs_load_shim(StageStartMusicByStage(entry_shim()));
}

/*
 * @routine 024D:077 StageStartMusicKeep
 * @run 6B:8077
 * @stop 8079
 * WHAT IT DOES: StageStartMusicByStage with zField8E's bit 3 set, so the fade's end sends the sound.
 * HOW: SMB3, running into StageStartMusicByStage.
 * WAITS: no.
 * Entry: A zStage. Exit: as StageStartMusicByStage.
 */
StageRegs StageStartMusicKeep(StageRegs r)
{
    RMW8(ZP->field8e, old, old | 0x08);
    return StageStartMusicByStage(r);
}

void abi_StageStartMusicKeep(void)
{
    stage_regs_load_shim(StageStartMusicKeep(entry_shim()));
}

/*
 * @routine 024D:06E StageStartMusicBoss
 * @run 6B:806E
 * WHAT IT DOES: The music stopped and the sound $5B kept and sent, with zField8E's bit 3 set.
 * HOW: SMB3, the stop vector, BRA to StageStartSound.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as StageStartSound.
 */
StageRegs StageStartMusicBoss(StageRegs r)
{
    RMW8(ZP->field8e, old, old | 0x08);
    r = call_to(r, 0x8073, 0x4087);
    r = loaded(r, 0x5B);
    return StageStartSound(r);
}

void abi_StageStartMusicBoss(void)
{
    stage_regs_load_shim(StageStartMusicBoss(entry_shim()));
}

/*
 * @routine 024D:093 StageStartDone
 * @run 6B:8093
 * WHAT IT DOES: The stage start's end: sub-mode 5, the forced pad's frames counted down, the player's start pose set.
 * HOW: Stores and a DEC.
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it; X, Y, C and V kept.
 */
StageRegs StageStartDone(StageRegs r)
{
    r = loaded(r, 0x05);
    WR8(ZP->game_sub_mode, 0x05);
    r.nz = DEC8(OBJ->pad_force_frames[0]);
    r = loaded(r, 0x01);
    wr8(PlayerStartPose, 0x01);
    return r;
}

void abi_StageStartDone(void)
{
    stage_regs_load_shim(StageStartDone(entry_shim()));
}

/*
 * @routine 024D:06A StageStartBossMusic
 * @run 6B:806A
 * WHAT IT DOES: The boss music, then the start's end.
 * HOW: BSR StageStartMusicBoss, BRA to StageStartDone.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as StageStartDone, the rest as the music left them.
 */
StageRegs StageStartBossMusic(StageRegs r)
{
    jsr_push(0x806C);
    r = StageStartMusicBoss(r);
    jsr_pop();
    return StageStartDone(r);
}

void abi_StageStartBossMusic(void)
{
    stage_regs_load_shim(StageStartBossMusic(entry_shim()));
}

/*
 * @routine 024D:087 StageStartPlain
 * @run 6B:8087
 * @stop 8093
 * WHAT IT DOES: The start with zField8E's bit 0 clear: stage $10 the boss music, stage $0C the by-stage music, others
 * none; then the start's end.
 * HOW: CPs of zStage read once; the BSR to StageStartMusicKeep.
 * WAITS: no.
 * Entry: nothing. Exit: as StageStartDone, the rest as the music left them.
 */
StageRegs StageStartPlain(StageRegs r)
{
    r = loaded(r, RD8(ZP->stage));
    r = regs_compared(r, r.a, 0x10);
    if (r.a == 0x10) {
        return StageStartBossMusic(r);
    }
    r = regs_compared(r, r.a, 0x0C);
    if (r.a == 0x0C) {
        jsr_push(0x8093);
        r = StageStartMusicKeep(r);
        jsr_pop();
    }
    return StageStartDone(r);
}

void abi_StageStartPlain(void)
{
    stage_regs_load_shim(StageStartPlain(entry_shim()));
}

/*
 * @routine 024D:02D StageStart
 * @run 6B:802D
 * @stop 806A
 * WHAT IT DOES: A stage's start: the game state wiped, its lists, VRAM runs, set-up, objects and player, then the
 * music by stage: with zField8E's bit 0 clear StageStartPlain; else the secret stages StageStartMusicByStage, stage $0C
 * StageStartMusicKeep, stage $10 the boss music, others the card's play-again.
 * HOW: The calls in turn; zField82 indexes SectionOrder for zField86; BBR0 and compares of zStage.
 * WAITS: where its calls wait.
 * Entry: nothing. Exit: as the music routine, or as the play-again.
 */
StageRegs StageStartAt(StageRegs r, uint16_t order_table)
{
    r = call_to(r, 0x8030, 0x4006);
    r = call_to(r, 0x8033, 0x88B1);
    r = call_to(r, 0x8036, 0x8882);
    r = call_to(r, 0x8039, 0x88EC);
    r = map_banks_5_6(r, 0x803C);
    r = call_to(r, 0x803F, 0xA003);
    r = call_to(r, 0x8042, 0x8B53);
    r = call_to(r, 0x8045, 0x8F98);
    r = call_to(r, 0x8048, 0x8A5F);
    WR8(ZP->paused, 0);
    WR8(ZP->fade_active, 0);
    r.nz = INC8(ZP->game_sub_mode);
    r.y = RD8(ZP->field82);
    r = loaded(r, rd8((uint16_t)(order_table + r.y)));
    WR8(ZP->field86, r.a);
    r = call_to(r, 0x8058, 0x80CB);
    if (!(RD8(ZP->field8e) & 0x01)) {
        return StageStartPlain(r);
    }
    r = loaded(r, RD8(ZP->stage));
    if (r.a & 0x80) {
        return StageStartMusicByStage(r);
    }
    r = regs_compared(r, r.a, 0x0C);
    if (r.a == 0x0C) {
        return StageStartMusicKeep(r);
    }
    r = regs_compared(r, r.a, 0x10);
    if (r.a == 0x10) {
        return StageStartMusicBoss(r);
    }
    return tail_to(r, 0x8067, 0x407B);
}

StageRegs StageStart(StageRegs r)
{
    return StageStartAt(r, SectionOrder);
}

void abi_StageStart(void)
{
    stage_regs_load_shim(StageStart(entry_shim()));
}
