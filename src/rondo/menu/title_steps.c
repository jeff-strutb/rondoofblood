/*
 * The title and attract overlay's sub-mode dispatch (bank $6B, sectors $C1 to $C5): the entry
 * stub, the title's own step table at $802E (zGameSubMode's word), the screen's setup table at
 * $81FF, the attract table at $870E, and the sub-modes of those tables that title.c and
 * attract.c do not hold. Each table is entered by a jump, so a handler's registers are the
 * dispatch's: A and X the doubled sub-mode, C its top bit, N and Z from it, Y whatever the mode
 * loop had (kept_y below).
 *
 * Fields named by their address are placeholders until the names for them land: the
 * title's screen state at $3E09 to $3E29 and $2A68 to $2A8B, the sound reset pair at $3B80.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "syscard.h"

enum {
    TITLE_STEPS = 0x802E, SETUP_STEPS = 0x81FF, ATTRACT_STEPS = 0x870E,
    SCREEN_SIZE_TITLE = 3, RASTER_ON = 1, RASTER_LINE = 0x10, SOUND_FADE_ALL = 0xFD,
    SOUND_ATTRACT_END = 0x7A, LOGO_SOUND_COMMAND = 0x55, ATTRACT_HOLD = 0x80, ATTRACT_END_STATE = 5, FADE_TYPE = 0x0C,
    BLINK_FLIP = 0x3F, BLINK_MASK = 0x0F, BLINK_HOLD = 0x40, SCRIPT_BLINK_OFF = 8, SCRIPT_BLINK_ON = 9,
    MPR5 = 0x20, VDC_REG_VRAM = 0x02, ATTRACT_BANK = 7, RESTART_LOW = 0x92, RESTART_HIGH = 0x91, JOY_RUN = 0x08
};

/* Placeholders (the names land with the typed access session). */
#define F2A68 0x2A68
#define F2A8A 0x2A8A
#define F2A8B 0x2A8B
#define F2A70 0x2A70
#define F2A71 0x2A71
#define F2758 0x2758
#define F2759 0x2759
#define F3E09 0x3E09
#define F3E11 0x3E11
#define F3E21 0x3E21
#define F3E29 0x3E29
#define F3B80 0x3B80
#define F3B81 0x3B81

static uint8_t next_step(void)
{
    return INC8(ZP->game_sub_mode);
}

/* The registers a handler entered through a table has: A and X the doubled sub-mode, C its top
 * bit, N and Z from it; Y as the caller had it. */
static void table_entry_shim(uint8_t index, uint8_t state, uint8_t kept_y, int v_in)
{
    if (v_in >= 0) {
        abi_flag(P_V, v_in != 0);
    }
    cpu.a = index;
    cpu.x = index;
    cpu.y = kept_y;
    abi_nz(index);
    abi_flag(P_C, (state & 0x80) != 0);
}

/* What a handler left, put in the dispatch's terms: a register it kept is the table's, and a
 * carry it kept is the doubling's. */
static Exit through_table(Exit exit, uint8_t index, uint8_t state, uint8_t kept_y, int v_in)
{
    if (exit.from_cpu) {
        return exit;
    }
    if (v_in >= 0 && !exit.v_set) {
        exit.v = v_in != 0;
        exit.v_set = true;
    }
    if (exit.a_kept) {
        exit.a = index;
        exit.a_kept = false;
    }
    if (exit.x_kept) {
        exit.x = index;
        exit.x_kept = false;
    }
    if (exit.y_kept) {
        exit.y = kept_y;
        exit.y_kept = false;
    }
    if (exit.c_kept) {
        exit.c = (state & 0x80) != 0;
        exit.c_kept = false;
    }
    return exit;
}

/* The flags a card call leaves, as an Exit's. */
static void card_flags(Exit *exit, CardExit card)
{
    exit->c_kept = !card.set_c;
    exit->c = card.c;
    exit->v_set = card.set_v;
    exit->v = card.v;
}

static void load_exit_shim(uint8_t value)
{
    cpu.a = value;
    abi_nz(value);
}

/*
 * @routine 00C1:048 TitleSetUp
 * @run 6B:8048
 * WHAT IT DOES: The title's first sub-mode: the music stopped, the backup file read (TitleSaveRead
 * is set when it read without trouble), the screen's size set, the title's first list and script
 * started, the raster effect on at line $10, and on to the next sub-mode.
 * HOW: SoundStopAll, BackupRead, ex_scrsiz with 3, StartListChannel0Kind2 and StartScriptSlot0Kind1
 * with 0, six stores, zGameSubMode counted up. A and X read back from the backup call reach no
 * branch that a result other than 0 makes differ.
 * WAITS: no.
 * Entry: X the doubled sub-mode, Y kept. Exit: A, X and Y 0, N and Z from the new sub-mode, C and V
 * as ex_scrsiz leaves them.
 */
Exit TitleSetUp(uint8_t kept_x, uint8_t kept_y)
{
    Exit exit = { 0 };
    BackupAccess read;
    CardExit card;

    jsr_push(0x804B);
    (void)SoundStopAll(kept_x, kept_y);
    jsr_pop();
    jsr_push(0x804E);
    read = BackupRead();
    jsr_pop();
    if (read.result == 0) {
        wr8(TitleSaveRead, 1);
    }
    jsr_push(0x8063);
    card = ex_scrsiz(SCREEN_SIZE_TITLE);
    jsr_pop();
    jsr_push(0x8067);
    (void)StartListChannel0Kind2(0);
    jsr_pop();
    jsr_push(0x806B);
    (void)StartScriptSlot0Kind1(0);
    jsr_pop();
    WR8(ZP->raster_effect, RASTER_ON);
    wr8(F3E09, 0);
    wr8(F3E11, 3);
    WR8(ZP->raster_arg, RASTER_LINE);
    wr8(F3E21, 0);
    wr8(F3E29, 0);
    exit.nz = next_step();
    card_flags(&exit, card);
    return exit;
}

void abi_TitleSetUp(void)
{
    exit_shim(TitleSetUp(cpu.x, cpu.y));
}

/*
 * @routine 00C1:205 TitleScreenBegin
 * @run 6B:8205
 * WHAT IT DOES: The screen's first sub-mode: the screen state cleared, the objects' fields at
 * $2A68, $2A8A, $2A8B, $2A70 and $2A71 cleared, list 1 started on channel 0 as kind 2 and script 3
 * on slot 0 as kind 1, zTemp3F and the script's step cleared, and on to the next sub-mode.
 * HOW: ClearScreenState, five stores, the two starts, two stores and zGameSubMode counted up.
 * WAITS: no.
 * Entry: nothing. Exit: A and Y 3, X 0, N and Z from the new sub-mode; C and V kept.
 */
Exit TitleScreenBegin(void)
{
    Exit exit = { 0 };

    jsr_push(0x8208);
    ClearScreenState();
    jsr_pop();
    wr8(F2A68, 0);
    wr8(F2A8A, 0);
    wr8(F2A8B, 0);
    wr8(F2A70, 0);
    wr8(F2A71, 0);
    jsr_push(0x821C);
    (void)StartListChannel0Kind2(1);
    jsr_pop();
    jsr_push(0x8221);
    (void)StartScriptSlot0Kind1(3);
    jsr_pop();
    WR8(ZP->_gap_003F[0], 0);
    WR8(ZP->_gap_007C[0], 0);
    exit.a = 3;
    exit.x = 0;
    exit.y = 3;
    exit.nz = next_step();
    exit.c_kept = true;
    return exit;
}

void abi_TitleScreenBegin(void)
{
    exit_shim(TitleScreenBegin());
}

/*
 * @routine 00C1:22F TitleSoundBegin
 * @run 6B:822F
 * WHAT IT DOES: The screen's second sub-mode: the sound reset pair at $3B80 cleared, sound command
 * $FD sent, zGameSubMode counted up, zStage cleared, and zModeDone counted up (ModeDoneAdd).
 * HOW: Two stores, SoundCommand, two more, then it branches into ModeDoneAdd.
 * WAITS: no.
 * Entry: X and Y as the caller's (passed to the sound command). Exit: as SoundCommand left A, X and Y,
 * N and Z from zModeDone's new value, C and V as the command left them.
 */
TitleSound TitleSoundBegin(uint8_t kept_x, uint8_t kept_y)
{
    TitleSound sound;

    wr8(F3B80, 0);
    wr8(F3B81, 0);
    jsr_push(0x823A);
    sound.call = SoundCommand(SOUND_FADE_ALL, kept_x, kept_y);
    jsr_pop();
    next_step();
    WR8(ZP->stage, 0);
    sound.done = ModeDoneAdd();
    return sound;
}

void title_sound_exit_shim(TitleSound sound)
{
    cpu.a = SOUND_FADE_ALL;
    sound_command_exit_shim(SOUND_FADE_ALL, sound.call);
    abi_nz(sound.done);
}

void abi_TitleSoundBegin(void)
{
    title_sound_exit_shim(TitleSoundBegin(cpu.x, cpu.y));
}

/* The handlers that leave something other than an Exit, as one: the registers they keep are the table's. */
static Exit kept_exit(uint8_t nz)
{
    Exit exit = { 0 };

    exit.nz = nz;
    exit.a_kept = exit.x_kept = exit.y_kept = exit.c_kept = true;
    return exit;
}

static Exit script_start_exit(uint8_t script, uint8_t nz)
{
    Exit exit = { 0 };

    exit.a = script;
    exit.x = 0;
    exit.y = script;
    exit.nz = nz;
    exit.c_kept = true;
    return exit;
}

static void title_logo_start_exit_shim(LogoStart start)
{
    cpu.a = LOGO_SOUND_COMMAND;
    cpu.x = 0;
    cpu.y = 1;
    sound_command_exit_shim(LOGO_SOUND_COMMAND, start.sound);
    abi_nz(start.step);
}

/*
 * @routine 00C1:00F TitleStep
 * @run 6B:800F
 * WHAT IT DOES: One frame of the title: the sub-mode's handler, by zGameSubMode's word in the table at
 * $802E (sub-mode 0 is TitleSetUp, then the scroll in, the blinks, the wipe, a wait, the logo, its
 * fades, a wait, its cycles, a wait, the last script, a wait and the end).
 * HOW: Past the first sub-mode, with the backup file read and the title seen before, START (the joypad
 * byte's bit 3) skips the title: zModeDone is counted down and it returns (A 8, N and Z from zModeDone,
 * X, Y and C kept). Otherwise it reads zGameSubMode again, doubles it and jumps through the table. A word
 * this overlay does not hold runs as the original code.
 * WAITS: no.
 * Entry: Y kept. Exit: as the handler leaves them.
 */
Exit TitleStep(uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t state, index, low, high;
    uint16_t target;
    int v_in = -1;

    state = RD8(ZP->game_sub_mode);
    if (state != 0 && rd8(TitleSaveRead) != 0 && rd8(TitleSeen) != 0
        && (rd8(BiosJoyTrigger) & JOY_RUN) != 0) {
        exit.a = JOY_RUN;
        exit.nz = DEC8(ZP->mode_done);
        exit.x_kept = exit.y_kept = exit.c_kept = true;
        return exit;
    }
    state = RD8(ZP->game_sub_mode);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(TITLE_STEPS + index));
    high = rd8((uint16_t)(TITLE_STEPS + 1 + index));
    target = (uint16_t)(low | (high << 8));
    switch (target) {
    case 0x8048: return through_table(TitleSetUp(index, kept_y), index, state, kept_y, v_in);
    case 0x8084: return through_table(TitleScrollIn(), index, state, kept_y, v_in);
    case 0x809E: return through_table(TitleBlink(kept_y), index, state, kept_y, v_in);
    case 0x80AF: return through_table(TitleWipe(), index, state, kept_y, v_in);
    case 0x8182: return through_table(kept_exit(TitleWait()), index, state, kept_y, v_in);
    case 0x80C6:
        table_entry_shim(index, state, kept_y, v_in);
        title_logo_start_exit_shim(TitleLogoStart());
        exit.from_cpu = true;
        return exit;
    case 0x80DB: return through_table(TitleLogoFade(kept_y), index, state, kept_y, v_in);
    case 0x8111: return through_table(TitleCycle(), index, state, kept_y, v_in);
    case 0x8170:
        TitleLastScript();
        return through_table(script_start_exit(0, 0), index, state, kept_y, v_in);
    case 0x817A: {
        Exit end = kept_exit(TitleEnd());

        end.a = 1;
        end.a_kept = false;
        return through_table(end, index, state, kept_y, v_in);
    }
    default:
        break;
    }
    table_entry_shim(index, state, kept_y, v_in);
    call_original(target);
    exit.from_cpu = true;
    return exit;
}

void abi_TitleStep(void)
{
    exit_shim(TitleStep(cpu.y));
}

/*
 * @routine 00C1:000 TitleEntry
 * @run 6B:8000
 * WHAT IT DOES: The title overlay's entry for a frame: runs TitleStep.
 * HOW: A jump to TitleStep.
 * WAITS: no.
 * Entry: Y kept. Exit: as TitleStep.
 */
Exit TitleEntry(uint8_t kept_y)
{
    return TitleStep(kept_y);
}

void abi_TitleEntry(void)
{
    exit_shim(TitleEntry(cpu.y));
}

/*
 * @routine 00C1:1F8 TitleScreenStep
 * @run 6B:81F8
 * WHAT IT DOES: One frame of the screen's setup: the handler zGameSubMode's word names in the table
 * at $81FF (sub-mode 0 is TitleScreenBegin, 1 is TitleSoundBegin).
 * HOW: Reads zGameSubMode, doubles it and jumps through the table. A word this overlay does not
 * hold runs as the original code.
 * WAITS: no.
 * Entry: Y kept. Exit: as the handler leaves them.
 */
Exit TitleScreenStep(uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t state, index, low, high;
    uint16_t target;
    int v_in = -1;

    state = RD8(ZP->game_sub_mode);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(SETUP_STEPS + index));
    high = rd8((uint16_t)(SETUP_STEPS + 1 + index));
    target = (uint16_t)(low | (high << 8));
    switch (target) {
    case 0x8205: return through_table(TitleScreenBegin(), index, state, kept_y, v_in);
    case 0x822F:
        table_entry_shim(index, state, kept_y, v_in);
        title_sound_exit_shim(TitleSoundBegin(index, kept_y));
        exit.from_cpu = true;
        return exit;
    default:
        break;
    }
    table_entry_shim(index, state, kept_y, v_in);
    call_original(target);
    exit.from_cpu = true;
    return exit;
}

void abi_TitleScreenStep(void)
{
    exit_shim(TitleScreenStep(cpu.y));
}

/* After a call, a load of A: the registers and flags of the call stand but A and N and Z. */
static Exit after_load(Exit exit, uint8_t value)
{
    if (exit.from_cpu) {
        load_exit_shim(value);
        return exit;
    }
    exit.a = value;
    exit.a_kept = false;
    exit.nz = value;
    return exit;
}

/*
 * @routine 00C1:742 AttractWaitStart
 * @run 6B:8742
 * WHAT IT DOES: An attract frame that waits for the file to have been read and START (the joypad byte's
 * bit 3) pressed: the frame counted and the scene's script stepped; then, once the backup file is read
 * and the button is down, the scene's script goes back to $9192, its counter to $FFFF, the sub-mode on,
 * and the CD fades with type $0C. (The arm after the button is untraced.)
 * HOW: AttractFrameTick, ScriptStep, TitleSaveRead and the joypad byte tested, the arm's stores, then a jump
 * to cd_fade.
 * WAITS: no.
 * Entry: nothing. Exit, no start: A 0, N and Z set, the rest as ScriptStep left them.
 * With start: as cd_fade leaves them.
 */
Exit AttractWaitStart(void)
{
    Exit exit;
    uint8_t saved, joy;

    jsr_push(0x8745);
    AttractFrameTick();
    jsr_pop();
    jsr_push(0x8748);
    exit = ScriptStep();
    jsr_pop();
    saved = rd8(TitleSaveRead);
    if (saved == 0) {
        return after_load(exit, saved);
    }
    joy = (uint8_t)(rd8(BiosJoyTrigger) & JOY_RUN);
    if (joy == 0) {
        return after_load(exit, joy);
    }
    WR8(ZP->_gap_007C[2], RESTART_LOW);
    WR8(ZP->_gap_007C[3], RESTART_HIGH);
    WR8(ZP->wait_timer3, 0xFF);
    WR8(ZP->wait_timer4, 0xFF);
    next_step();
    return after_load(exit, cd_fade(FADE_TYPE));
}

void abi_AttractWaitStart(void)
{
    exit_shim(AttractWaitStart());
}

/*
 * @routine 00C1:769 AttractStartCheck
 * @run 6B:8769
 * WHAT IT DOES: An attract frame in its second sub-mode: the frame counted and the script stepped; on the
 * start button the scene's music is cut (sound command $7A), the counter set to $0080 with the blink's
 * fields at $2758 to $2A71 cleared, and the sub-mode set to 5.
 * HOW: AttractFrameTick, ScriptStep, the joypad byte's bit 3, SoundCommand, seven stores.
 * WAITS: no.
 * Entry: X the doubled sub-mode, Y kept. Exit, no start: A 0, N and Z set, the rest as ScriptStep left them.
 * With start: A 5, N and Z clear, as SoundCommand leaves the rest.
 */
AttractStart AttractStartCheck(uint8_t index, uint8_t kept_y)
{
    AttractStart start = { 0 };
    uint8_t joy;

    jsr_push(0x876C);
    AttractFrameTick();
    jsr_pop();
    jsr_push(0x876F);
    start.exit = ScriptStep();
    jsr_pop();
    joy = (uint8_t)(rd8(BiosJoyTrigger) & JOY_RUN);
    if (joy == 0) {
        start.exit = after_load(start.exit, joy);
        return start;
    }
    start.x = start.exit.x_kept ? index : start.exit.x;
    start.y = start.exit.y_kept ? kept_y : start.exit.y;
    jsr_push(0x877B);
    start.call = SoundCommand(SOUND_ATTRACT_END, start.x, start.y);
    jsr_pop();
    start.sounded = true;
    WR8(ZP->wait_timer3, ATTRACT_HOLD);
    WR8(ZP->wait_timer4, 0);
    wr8(F2758, 0);
    wr8(F2A70, 0);
    wr8(F2759, 0);
    wr8(F2A71, 0);
    WR8(ZP->game_sub_mode, ATTRACT_END_STATE);
    return start;
}

void attract_start_exit_shim(AttractStart start)
{
    if (!start.sounded) {
        exit_shim(start.exit);
        return;
    }
    exit_shim(start.exit);
    cpu.a = SOUND_ATTRACT_END;
    cpu.x = start.x;
    cpu.y = start.y;
    sound_command_exit_shim(SOUND_ATTRACT_END, start.call);
    load_exit_shim(ATTRACT_END_STATE);
}

void abi_AttractStartCheck(void)
{
    attract_start_exit_shim(AttractStartCheck(cpu.x, cpu.y));
}

/*
 * @routine 00C1:792 AttractBlink
 * @run 6B:8792
 * WHAT IT DOES: The attract scene's last state before the end: every 16th frame, while its step is 0,
 * flips the six low bits of the blink colour at $2A68; counts the frame counter down, and on 0 goes through
 * three steps: the first clears the colour and starts script 8, the second starts script 9 (each with $40
 * frames), the third counts zModeDone.
 * HOW: zFrameTick's low nibble, the counter's low and high bytes (zWaitTimer3 and 4), StartScriptSlot0Kind2.
 * WAITS: no.
 * Entry: nothing. Exit: while counting, A the last byte loaded (or the colour), N and Z from the counter's
 * new value, C kept. At a step, A and Y the script, X 0, C set. Third, A the step, N and Z from zModeDone's
 * new value, C set when the step is 2 or more.
 */
Exit AttractBlink(void)
{
    Exit exit = { 0 };
    uint8_t tick, a, left, step;

    exit.x_kept = true;
    exit.y_kept = true;
    exit.c_kept = true;
    tick = (uint8_t)(RD8(ZP->frame_tick) & BLINK_MASK);
    a = tick;
    if (tick == 0) {
        a = RD8(ZP->wait_timer4);
        if (a == 0) {
            a = (uint8_t)(rd8(F2A68) ^ BLINK_FLIP);
            wr8(F2A68, a);
        }
    }
    left = DEC8(ZP->wait_timer3);
    exit.a = a;
    exit.nz = left;
    if (left != 0) {
        return exit;
    }
    INC8(ZP->wait_timer4);
    step = RD8(ZP->wait_timer4);
    exit.a = step;
    exit.c = step >= 1;
    exit.c_kept = false;
    if (step == 1) {
        WR8(ZP->wait_timer3, BLINK_HOLD);
        wr8(F2A68, 0);
        (void)StartScriptSlot0Kind2(SCRIPT_BLINK_OFF);
        exit = script_start_exit(SCRIPT_BLINK_OFF, SCRIPT_BLINK_OFF);
        exit.c = true;
        exit.c_kept = false;
        return exit;
    }
    exit.c = step >= 2;
    if (step == 2) {
        WR8(ZP->wait_timer3, BLINK_HOLD);
        (void)StartScriptSlot0Kind2(SCRIPT_BLINK_ON);
        exit = script_start_exit(SCRIPT_BLINK_ON, SCRIPT_BLINK_ON);
        exit.c = true;
        exit.c_kept = false;
        return exit;
    }
    exit.nz = INC8(ZP->mode_done);
    return exit;
}

void abi_AttractBlink(void)
{
    exit_shim(AttractBlink());
}

/*
 * @routine 00C1:700 AttractStep
 * @run 6B:8700
 * WHAT IT DOES: One frame of the attract scene: bank $07 plus zRamBaseBank mapped at MPR5, then the
 * handler zGameSubMode's word names in the table at $870E: begin, wait for start, check start, (a sub-mode
 * no script reached), the end count and the blink.
 * HOW: tam, then reads zGameSubMode, doubles it and jumps through the table. A word this overlay does not
 * hold runs as the original code.
 * WAITS: yes, through AttractBegin and AttractEndCountdown, which reach ResetGameState.
 * Entry: Y kept. Exit: as the handler leaves them.
 */
Exit AttractStep(uint8_t kept_y)
{
    Exit exit = { 0 };
    uint8_t state, index, low, high;
    uint16_t target;
    uint8_t base, bank;
    int v_in;

    base = RD8(ZP->ram_base_bank);
    bank = (uint8_t)(ATTRACT_BANK + base);
    v_in = ((ATTRACT_BANK ^ bank) & (base ^ bank) & 0x80) != 0;     /* the add's V stands through the dispatch */
    tam(MPR5, bank);
    state = RD8(ZP->game_sub_mode);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(ATTRACT_STEPS + index));
    high = rd8((uint16_t)(ATTRACT_STEPS + 1 + index));
    target = (uint16_t)(low | (high << 8));
    switch (target) {
    case 0x871A: {
        Exit begin = { 0 };

        begin.a = 0xFF;
        begin.x = 0;
        begin.y = 0xFF;
        begin.nz = AttractBegin();
        begin.c_kept = true;
        return through_table(begin, index, state, kept_y, v_in);
    }
    case 0x8742: return through_table(AttractWaitStart(), index, state, kept_y, v_in);
    case 0x8769:
        table_entry_shim(index, state, kept_y, v_in);
        attract_start_exit_shim(AttractStartCheck(index, kept_y));
        exit.from_cpu = true;
        return exit;
    case 0x87D3: {
        AttractEnd end = AttractEndCountdown();
        Exit done = kept_exit(end.value);

        if (end.reset) {
            done.a = VDC_REG_VRAM;
            done.x = 0;
            done.y = 0xFF;
            done.a_kept = done.x_kept = done.y_kept = false;
        }
        return through_table(done, index, state, kept_y, v_in);
    }
    case 0x8792: return through_table(AttractBlink(), index, state, kept_y, v_in);
    default:
        break;
    }
    table_entry_shim(index, state, kept_y, v_in);
    call_original(target);
    exit.from_cpu = true;
    return exit;
}

void abi_AttractStep(void)
{
    exit_shim(AttractStep(cpu.y));
}
