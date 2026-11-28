/*
 * The stage overlay's play sub-step and what it runs (sector 024D and on, bank $6B at $8000, as the first stage loads
 * it): the demo's pad record, played back or recorded, the player program's frame and objects through the banks 5 and
 * 6 above zRamBaseBank, the walk that brings the player to the screen's middle, and the paused frame.
 *
 * The player program (Richter's at 01B9, Maria's at 01DD) is mapped at $A000 under MPR5: its vectors at $A000 (the
 * frame), $A009 (the run of its objects) and $A027 (the move it applies). A JSR or JMP into the overlay's window or the
 * player's goes to the C of the routine loaded there (routine_at), as the overlays that hold these bytes differ in what
 * they hold there; a JSR into the kernel goes to the callee's typed C.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_map.h"
#include "stage_regs.h"

typedef StageRegs Regs;

/* Defined elsewhere, typed, and not declared in a header. */
ObjArm MapOverlayBanks5And6(void);                   /* object/player_leaves.c */
ColumnsEnd PlayerAndCameraFrame(void);              /* object/camera.c */

enum {
    PLAYER_BANK = 5, DEMO_BANK = 8, RECORD_END_COUNT = 0xFF, PAD_SELECT = 0x04, RECORD_LAST_PAGE = 0xE0,
    PAUSED_ON = 0x01, PAUSED_MENU = 0x02, SLOW_TIMER_SCRIPT = 0x09, SLOW_TIMER_SLOT = 0x07, FIELD8E_CENTRE = 0x80,
    CENTRE_X = 0x80, CENTRE_STEP = 0x04, CENTRE_PAD_FRAMES = 0x80,
    PLAY_HANDLERS = 0x822D, PLAYER_FRAME_RUN = 0xA000, PLAYER_OBJECTS_RUN = 0xA009, PLAYER_MOVE_RUN = 0xA027
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

/* A JSR to the routine loaded at run, returning to ret: the registers as they stand, and what it leaves. */
static Regs call_to(Regs r, uint16_t ret, uint16_t run)
{
    stage_regs_load_shim(r);
    call_routine(ret, run);
    return stage_regs_from_cpu_shim(r);
}

/* JSR MapOverlayBanks5And6 ($80A0), returning to ret: A the second bank, N and Z from it, C and V from the add. */
static Regs map_banks_5_6(Regs r, uint16_t ret)
{
    ObjArm e;

    jsr_push(ret);
    e = MapOverlayBanks5And6();
    jsr_pop();
    r = loaded(r, e.first);
    r.c = e.sum.carry;
    r.v = e.sum.overflow;
    return r;
}

/* TST #imm, field: Z from the byte AND the mask, N and V from the byte's bits 7 and 6. N and Z are held as one byte,
 * so N is kept only where Z is clear; where both are set no instruction before the next flag write tests N. */
static Regs tested(Regs r, uint8_t byte, uint8_t mask)
{
    r.v = (byte & 0x40) != 0;
    r.nz = (uint8_t)((byte & mask) == 0 ? 0x00 : ((byte & 0x80) != 0 ? 0x80 : 0x01));
    return r;
}

/* A word of the overlay's jump table, as JMP [table,X] reads it: the low byte, then the high. */
static uint16_t table_word(uint16_t table, uint8_t index)
{
    uint8_t low = rd8((uint16_t)(table + index));
    uint8_t high = rd8((uint16_t)(table + index + 1));

    return (uint16_t)(low | (high << 8));
}

/*
 * @routine 024D:0AB StagePlayerFrame
 * @run 6B:80AB
 * WHAT IT DOES: Runs the player program's frame: maps the banks 5 and 6 above zRamBaseBank and goes to the vector at
 * $A000 of the program loaded there.
 * HOW: MapOverlayBanks5And6, then the tail JMP to $A000.
 * WAITS: where the player's frame waits.
 * Entry: X and Y the caller's. Exit: as the player's frame leaves them, entered with A the second bank, N and Z from
 * it, C and V from the map's add.
 */
StageRegs StagePlayerFrame(StageRegs r)
{
    return StagePlayerFrameAt(r, 0x80AE);
}

/* StagePlayerFrame's body, ret the address after its JSR to MapOverlayBanks5And6 (where the JMP that follows starts):
 * a build of the overlay that holds these instructions at another address passes its own. */
StageRegs StagePlayerFrameAt(StageRegs r, uint16_t ret)
{
    r = map_banks_5_6(r, ret);
    return tail_to(r, ret, PLAYER_FRAME_RUN);
}

void abi_StagePlayerFrame(void)
{
    stage_regs_load_shim(StagePlayerFrame(entry_shim()));
}

/*
 * @routine 024D:0B1 StagePlayerObjects
 * @run 6B:80B1
 * WHAT IT DOES: Runs the player program's objects: maps the banks 5 and 6 above zRamBaseBank and goes to the vector at
 * $A009 of the program loaded there.
 * HOW: MapOverlayBanks5And6, then the tail JMP to $A009.
 * WAITS: where the player's objects wait.
 * Entry: X and Y the caller's. Exit: as the player's objects leave them, entered with A the second bank, N and Z from
 * it, C and V from the map's add.
 */
StageRegs StagePlayerObjects(StageRegs r)
{
    return StagePlayerObjectsAt(r, 0x80B4);
}

/* StagePlayerObjects' body, ret the address after its JSR to MapOverlayBanks5And6 (where the JMP that follows starts). */
StageRegs StagePlayerObjectsAt(StageRegs r, uint16_t ret)
{
    r = map_banks_5_6(r, ret);
    return tail_to(r, ret, PLAYER_OBJECTS_RUN);
}

void abi_StagePlayerObjects(void)
{
    stage_regs_load_shim(StagePlayerObjects(entry_shim()));
}

/* The end at $813C: the record or the playback is over, zModeDone counted on. N and Z from it; the rest kept. */
static Regs demo_done(Regs r)
{
    r.nz = INC8(ZP->mode_done);
    return r;
}

/*
 * @routine 024D:147 StageDemoPadPlay
 * @run 6B:8147
 * WHAT IT DOES: Plays a demo's pad record back, one frame: the record (at zScriptPtr) is pairs of a count of frames
 * and a pad. The pad goes to BiosPadState, its buttons not held the frame before to BiosJoyTrigger; when its count runs
 * out the next pair comes. A count of zero ends the demo: zModeDone is counted on.
 * HOW: zDemoPadFrames is the count left; at zero the pair's count is read through zScriptPtr. The pad is read at
 * [zScriptPtr],1; zDemoPadLast keeps the pad given, its complement ANDed with the new one for the new presses. DEC
 * zDemoPadFrames; at zero zScriptPtr steps by 2, its high byte counted on when the low carries.
 * WAITS: no.
 * Entry: the record mapped (StageDemoPadStep). Exit: the record over: A 0, N and Z from zModeDone's INC. A pad given
 * with frames left: A the pad, Y 1, N and Z from the DEC. The pair's last frame: A zScriptPtr's new low byte, Y 1, C and
 * V from the add, N and Z from it, or from the high byte's INC when the low carried (C then 1). X kept; V kept but where
 * the pointer steps.
 */
StageRegs StageDemoPadPlay(StageRegs r)
{
    uint8_t frames = RD8(ZP->demo_pad_frames);
    Sum step;

    r = loaded(r, frames);
    if (frames == 0) {
        uint8_t count = RD8_PTR(ZP->script_ptr);

        r = loaded(r, count);
        if (count == 0) {
            return demo_done(r);
        }
        WR8(ZP->demo_pad_frames, count);
    }
    r.y = 0x01;
    r = loaded(r, RD8_PTR_Y(ZP->script_ptr, r.y));
    WR8(PADS->now, r.a);
    {
        uint8_t last = RD8(ZP->demo_pad_last);
        uint8_t pad = RD8(PADS->now);

        r = loaded(r, (uint8_t)((uint8_t)(last ^ 0xFF) & pad));
    }
    WR8(PADS->trigger, r.a);
    r = loaded(r, RD8(PADS->now));
    WR8(ZP->demo_pad_last, r.a);
    r.nz = DEC8(ZP->demo_pad_frames);
    if (r.nz != 0) {
        return r;
    }
    step = add_bytes(RD8(ZP->script_ptr), 0x02, false);
    r = loaded(r, step.value);
    r.c = step.carry;
    r.v = step.overflow;
    WR8(ZP->script_ptr, step.value);
    if (!step.carry) {
        return r;
    }
    r.nz = INC8(ZP->script_ptr_hi);
    return r;
}

void abi_StageDemoPadPlay(void)
{
    stage_regs_load_shim(StageDemoPadPlay(entry_shim()));
}

/*
 * @routine 024D:0E8 StageDemoPadStep
 * @run 6B:80E8
 * WHAT IT DOES: While a demo runs, maps its pad record (the banks 8 and 9 above zRamBaseBank, into MPR5 and MPR6) and
 * plays its frame's pad back.
 * HOW: zGameRunning zero ends at once, at the RTS before it ($80E7); else the ADC, TAM #$20, INC A, TAM #$40, and the
 * BRA into StageDemoPadPlay.
 * WAITS: no.
 * Entry: nothing. Exit: no demo: A 0, Z 1, the rest kept. Else as StageDemoPadPlay, entered with A the second bank, N
 * and Z from it, C and V from the add.
 */
StageRegs StageDemoPadStep(StageRegs r)
{
    uint8_t running = RD8(ZP->game_running);
    Sum bank;

    r = loaded(r, running);
    if (running == 0) {
        return r;
    }
    bank = add_bytes(DEMO_BANK, RD8(ZP->ram_base_bank), false);
    r.c = bank.carry;
    r.v = bank.overflow;
    tam(0x20, bank.value);
    r = loaded(r, (uint8_t)(bank.value + 1));
    tam(0x40, r.a);
    return StageDemoPadPlay(r);
}

void abi_StageDemoPadStep(void)
{
    stage_regs_load_shim(StageDemoPadStep(entry_shim()));
}

/*
 * @routine 024D:12B StageDemoPadRecordNext
 * @run 6B:812B
 * WHAT IT DOES: Starts a demo record's next pair: zScriptPtr steps by 2 and the pair there is cleared; a pointer that
 * reaches $E000 ends the record (zModeDone counted on) instead.
 * HOW: ADC #2 to zScriptPtr's low byte; on its carry the high byte's INC and a CMP of it with $E0. CLA, STA
 * [zScriptPtr], LDY #1, STA [zScriptPtr],Y.
 * WAITS: no.
 * Nothing is known to reach it but StageDemoPadRecord, and nothing to reach that.
 * Entry: nothing. Exit: the pair cleared: A 0, Y 1, N 0 and Z 0, C and V from the add (C from the CMP when the high byte
 * stepped). The record over: A $E0, C 1, N and Z from zModeDone's INC, V from the add, Y kept. X kept.
 */
StageRegs StageDemoPadRecordNext(StageRegs r)
{
    Sum step = add_bytes(RD8(ZP->script_ptr), 0x02, false);

    r = loaded(r, step.value);
    r.c = step.carry;
    r.v = step.overflow;
    WR8(ZP->script_ptr, step.value);
    if (step.carry) {
        INC8(ZP->script_ptr_hi);
        r.a = RD8(ZP->script_ptr_hi);
        r = regs_compared(r, r.a, RECORD_LAST_PAGE);
        if (r.a == RECORD_LAST_PAGE) {
            return demo_done(r);
        }
    }
    r.a = 0x00;                                     /* CLA: the flags kept */
    WR8_PTR_Y(ZP->script_ptr, 0, r.a);              /* STA [zScriptPtr]: the same accesses as with Y 0 */
    r.y = 0x01;
    r.nz = r.y;
    WR8_PTR_Y(ZP->script_ptr, r.y, r.a);
    return r;
}

void abi_StageDemoPadRecordNext(void)
{
    stage_regs_load_shim(StageDemoPadRecordNext(entry_shim()));
}

/* BSR StageDemoPadRecordNext, returning to ret. */
static Regs record_next(Regs r, uint16_t ret)
{
    jsr_push(ret);
    r = StageDemoPadRecordNext(r);
    jsr_pop();
    return r;
}

/*
 * @routine 024D:0F8 StageDemoPadRecord
 * @run 6B:80F8
 * WHAT IT DOES: Records a demo's pad, one frame, in the form StageDemoPadPlay plays back: while the pad is the one the
 * pair at zScriptPtr holds its count is counted on; a new pad, or a count of $FF, starts the next pair with a count of
 * 1. The second pad's SELECT ends the record (zModeDone counted on).
 * HOW: LDA [zScriptPtr] against $FF; BiosJoyTrigger2 bit 2; the pad compared with [zScriptPtr],1; BSRs to
 * StageDemoPadRecordNext.
 * WAITS: no.
 * Nothing is known to reach it: no JSR, JMP or table of the overlay names $80F8, and the BRA before it goes past it.
 * Entry: the record mapped. Exit: the count counted on: A the count, N and Z from it, Y 1 when the pad was compared. A
 * new pair: A the pad, Y 1, N and Z from it. The record over: as StageDemoPadRecordNext's end, then zModeDone's INC.
 * X kept.
 */
StageRegs StageDemoPadRecord(StageRegs r)
{
    return StageDemoPadRecordAt(r, 0x80F8);
}

/* StageDemoPadRecord's body run at `at`: its BSRs push their return addresses from there. The builds that hold these
 * instructions at another address ($80E1 in 026D and its kin) pass their own. */
StageRegs StageDemoPadRecordAt(StageRegs r, uint16_t at)
{
    uint8_t count = RD8_PTR(ZP->script_ptr);

    r = regs_compared(loaded(r, count), count, RECORD_END_COUNT);
    if (count == RECORD_END_COUNT) {
        r = record_next(r, (uint16_t)(at + 0x08));
    }
    r = loaded(r, (uint8_t)(RD8(PADS->trigger2) & PAD_SELECT));
    if (r.a != 0) {
        r = record_next(r, (uint16_t)(at + 0x31));
        return demo_done(r);
    }
    count = RD8_PTR(ZP->script_ptr);
    r = loaded(r, count);
    if (count != 0) {
        uint8_t pad;

        r.y = 0x01;
        pad = RD8(PADS->now);
        r.a = pad;
        r = regs_compared(r, pad, RD8_PTR_Y(ZP->script_ptr, r.y));
        if (r.nz == 0) {
            r = loaded(r, (uint8_t)(RD8_PTR(ZP->script_ptr) + 1));
            WR8_PTR_Y(ZP->script_ptr, 0, r.a);      /* STA [zScriptPtr] */
            return r;
        }
        r = record_next(r, (uint16_t)(at + 0x24));
    }
    r = loaded(r, 0x01);
    WR8_PTR_Y(ZP->script_ptr, 0, r.a);              /* STA [zScriptPtr] */
    r.y = r.a;
    r = loaded(r, RD8(PADS->now));
    WR8_PTR_Y(ZP->script_ptr, r.y, r.a);
    return r;
}

void abi_StageDemoPadRecord(void)
{
    stage_regs_load_shim(StageDemoPadRecord(entry_shim()));
}

/*
 * @routine 024D:72C StagePlayerToCentre
 * @run 6B:872C
 * WHAT IT DOES: The walk that brings the player to the screen's middle while zField8E's bit 7 is set: his pad held
 * forced for $80 frames and his objects run; standing (state 0 or 1, step 0) he is shown in his start pose and moved 4
 * pixels a frame towards X $80 while the layers scroll 4 the other way and the camera steps. Within 2 of $80 the walk
 * ends: zField8E's bit 7, the forced frames and the start pose cleared.
 * HOW: OBJ_SUBSTATE and OBJ_STATE of slot 0; StagePlayerObjects; OBJ_X_LO plus 2 masked with $FC (before the move) and
 * $F8 (after it) against $80; ScrollStepXHi $FC or 4 by the side of $80 he is on, ScrollStepXLo 0; StageCameraStep.
 * WAITS: no.
 * Entry: nothing. Exit: not standing, or standing and his objects run: as StagePlayerObjects leaves them. Moved: A his
 * X plus 2 masked with $F8, N, Z and C from the CMP with $80, V from the add; X and Y as the camera step left them. At
 * the middle: A $80, Z 1, C 1, V from the last add.
 */
StageRegs StagePlayerToCentre(StageRegs r)
{
    uint8_t step = RD8(OBJ->substate[0]);
    Sum near;

    r = loaded(r, step);
    if (step == 0) {
        uint8_t state = RD8(OBJ->state[0]);

        r = regs_compared(loaded(r, state), state, 0x02);
        if (state < 0x02) {
            r = loaded(r, CENTRE_PAD_FRAMES);
            WR8(OBJ->pad_force_frames[0], r.a);
            WR8(OBJ->pad_forced[0], 0x00);
            r = loaded(r, 0x01);
            WR8(PVARS2->start_pose, r.a);
            r = call_to(r, 0x8754, 0x80B1);
            near = add_bytes(RD8(OBJ->x_lo[0]), 0x02, false);
            r.v = near.overflow;
            r = loaded(r, (uint8_t)(near.value & 0xFC));
            r = regs_compared(r, r.a, CENTRE_X);
            if (r.a != CENTRE_X) {
                uint8_t x = RD8(OBJ->x_lo[0]);
                Sum moved;

                r = loaded(r, x);
                if ((x & 0x80) == 0) {
                    r = loaded(r, (uint8_t)(0x100 - CENTRE_STEP));
                    WR8(SCROLLE->step_x_hi[0], r.a);
                    WR8(SCROLLE->step_x_lo[0], 0x00);
                    moved = add_bytes(RD8(OBJ->x_lo[0]), CENTRE_STEP, false);
                } else {
                    r = loaded(r, CENTRE_STEP);
                    WR8(SCROLLE->step_x_hi[0], r.a);
                    WR8(SCROLLE->step_x_lo[0], 0x00);
                    moved = add_bytes(RD8(OBJ->x_lo[0]), (uint8_t)(0x100 - CENTRE_STEP), false);
                }
                r = loaded(r, moved.value);
                r.c = moved.carry;
                r.v = moved.overflow;
                WR8(OBJ->x_lo[0], moved.value);
                r = call_to(r, 0x878C, 0x80BF);
                near = add_bytes(RD8(OBJ->x_lo[0]), 0x02, false);
                r.v = near.overflow;
                r = loaded(r, (uint8_t)(near.value & 0xF8));
                r = regs_compared(r, r.a, CENTRE_X);
                if (r.a != CENTRE_X) {
                    return r;
                }
            }
            WR8(ZP->field8e, (uint8_t)(RD8(ZP->field8e) & (uint8_t)~FIELD8E_CENTRE));    /* RMB7 */
            WR8(OBJ->pad_force_frames[0], 0x00);
            WR8(PVARS2->start_pose, 0x00);
            return r;
        }
    }
    r = loaded(r, CENTRE_PAD_FRAMES);
    WR8(OBJ->pad_force_frames[0], r.a);
    WR8(OBJ->pad_forced[0], 0x00);
    return call_to(r, 0x8743, 0x80B1);
}

void abi_StagePlayerToCentre(void)
{
    stage_regs_load_shim(StagePlayerToCentre(entry_shim()));
}

/*
 * @routine 024E:6AE StagePausedIdle
 * @run 6B:8EAE
 * WHAT IT DOES: Returns: the paused frame's end while the pause screen is not up.
 * HOW: One RTS.
 * WAITS: no.
 * Entry: anything. Exit: all registers and flags kept.
 */
StageRegs StagePausedIdle(StageRegs r)
{
    return r;
}

void abi_StagePausedIdle(void)
{
}

/*
 * @routine 024E:6EA StagePausedStep
 * @run 6B:8EEA
 * WHAT IT DOES: The paused game's frame: nothing until the pause screen is up (zPaused bit 1), then its sequence.
 * HOW: BBR1 of zPaused to the RTS at $8EAE; else on into StepStageSequence.
 * WAITS: as StepStageSequence.
 * Entry: X and Y the caller's. Exit: bit 1 clear: all kept. Else as StepStageSequence leaves them.
 */
StageRegs StagePausedStep(StageRegs r)
{
    if ((RD8(ZP->paused) & PAUSED_MENU) == 0) {
        return StagePausedIdle(r);
    }
    return stage_regs_after(StepStageSequence(r.x, r.y), r);
}

void abi_StagePausedStep(void)
{
    stage_regs_load_shim(StagePausedStep(entry_shim()));
}

/*
 * @routine 024D:1BA StageSubStepPlay
 * @run 6B:81BA
 * @stop 822D
 * WHAT IT DOES: The stage's play sub-step, one frame: the demo's pad, the fade's countdown and the pause. Paused, the
 * paused frame runs instead. Else: the wave counter steps; the player's slow timer counts down (script 9 in slot 7 at
 * its end) and while it runs the layers' scroll steps are cleared and the player runs on even counts only; the
 * player's frame, and unless a sub-step is asked for, the player and the camera (PlayerAndCameraFrame), the camera step
 * and the player's objects (or the walk to the middle while zField8E's bit 7 is set), the scroll and the objects, the
 * hit tests, the stage's frame and the player's move. Last, the sub-step asked for in zSubStep (cleared) runs from the
 * table at $822D.
 * HOW: Three JSRs into the overlay; BBR0 of zPaused; INC zWaveCounter; PlayerSlowTimerLo ORA Hi, two DECs; four STZs
 * of the scroll steps; TST #1 of the timer's low byte twice; the JSRs; zSubStep doubled indexes the table, whose word
 * is jumped through.
 * WAITS: where its callees wait; the handler is reached by a jump.
 * Entry: X and Y the caller's. Exit: as the handler leaves them, entered with A and X twice zSubStep, N and Z from it,
 * C its bit 7.
 */
StageRegs StageSubStepPlay(StageRegs r)
{
    uint8_t step;
    uint8_t twice;
    bool player_frame = true;

    r = call_to(r, 0x81BD, 0x80E8);
    r = call_to(r, 0x81C0, 0x8178);
    r = call_to(r, 0x81C3, 0x81A2);
    if ((RD8(ZP->paused) & PAUSED_ON) != 0) {
        return tail_to(r, 0x81C6, 0x8EEA);
    }
    r.nz = INC8(ZP->wave_counter);
    {
        uint8_t lo = RD8(PVARS->slow_timer_lo);
        uint8_t hi = RD8(PVARS->slow_timer_hi);

        r = loaded(r, (uint8_t)(lo | hi));
    }
    if (r.a != 0) {
        r.nz = DEC8(PVARS->slow_timer_lo);
        if (r.nz == 0) {
            r.nz = DEC8(PVARS->slow_timer_hi);
            if (r.nz == 0) {
                r = loaded(r, SLOW_TIMER_SCRIPT);
                r.x = SLOW_TIMER_SLOT;
                jsr_push(0x81E4);
                r.y = StartScriptSlotKind1(r.a, r.x);
                jsr_pop();
                r.a = r.y;
                r.nz = r.y;
            }
        }
        WR8(SCROLLE->step_x_hi[0], 0x00);
        WR8(SCROLLE->step_x_lo[0], 0x00);
        WR8(SCROLLE->step_y_hi[0], 0x00);
        WR8(SCROLLE->step_y_lo[0], 0x00);
        r = tested(r, RD8(PVARS->slow_timer_lo), 0x01);
        player_frame = r.nz == 0;                   /* BNE $8200 on an odd count */
    }
    if (player_frame) {
        r = call_to(r, 0x81F9, 0x80AB);
        r = loaded(r, RD8(ZP->sub_step));
        if (r.a != 0) {
            goto handler;
        }
        stage_regs_load_shim(r);
        jsr_push(0x8200);
        columns_exit_shim(PlayerAndCameraFrame());
        jsr_pop();
        r = stage_regs_from_cpu_shim(r);
    }
    r = loaded(r, RD8(ZP->field8e));
    if ((r.a & 0x80) != 0) {
        r = call_to(r, 0x8207, 0x872C);
    } else {
        r = call_to(r, 0x820C, 0x80BF);
        r = tested(r, RD8(PVARS->slow_timer_lo), 0x01);
        if (r.nz == 0) {
            r = call_to(r, 0x8215, 0x80B1);
        }
    }
    r = call_to(r, 0x8218, 0x8A4F);
    r = call_to(r, 0x821B, 0x93F8);
    r = call_to(r, 0x821E, 0x8B6D);
    r = call_to(r, 0x8221, 0x80A0);
    r = call_to(r, 0x8224, PLAYER_MOVE_RUN);
handler:
    step = RD8(ZP->sub_step);
    WR8(ZP->sub_step, 0x00);
    twice = (uint8_t)(step << 1);
    r.c = (step & 0x80) != 0;
    r = loaded(r, twice);
    r.x = twice;
    return tail_to(r, 0x822A, table_word(PLAY_HANDLERS, twice));
}

void abi_StageSubStepPlay(void)
{
    stage_regs_load_shim(StageSubStepPlay(entry_shim()));
}
