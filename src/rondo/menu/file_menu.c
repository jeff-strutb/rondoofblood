/*
 * The file menu's pages and steps (mode 4, the title overlay's $800C entry; bank $6B at $8000 and $6F at $A000):
 * zGameSubMode is the page and zField80 the step of the page. Each page, and each step of a page, is picked through
 * MenuStepDispatch, whose table of handlers is the words after its JSR.
 *
 * The handlers here end with the registers that a chain of callees leaves, some of which (the card's backup calls, the
 * sound driver, the CD) give their registers back only through their exit shims. A handler keeps them in a MenuRegs:
 * the register file as the original holds it, the registers the routine's own code set since the last callee whose
 * exit is replayed, and the replayed callees in order. menu_regs_shim gives the CPU exactly what the original leaves.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"
#include "syscard.h"

/* What a step of the menu leaves: the registers its own code set, over the exits of the callees it replays. */
enum { REG_A = 1, REG_X = 2, REG_Y = 4, REG_NZ = 8, REG_C = 16, REG_V = 32 };
typedef enum { REPLAY_RESET, REPLAY_BACKUP_READ, REPLAY_BACKUP_WRITE, REPLAY_FORMAT, REPLAY_TRACK, REPLAY_SOUND,
    REPLAY_CD_PAUSE, REPLAY_CD_FADE } ReplayKind;
typedef struct {
    ReplayKind kind;
    uint8_t set;                    /* the registers set before this callee, in a, x, ... of the record */
    uint8_t a, x, y, nz;
    bool c, v;
    uint8_t command;
    union { BackupAccess backup; BackupWriteRun write; BackupFormat format; TrackPlay track; CommandCall sound; CdPause pause; } call;
} MenuReplay;
typedef struct {
    uint8_t a, x, y, nz;
    bool c, v;
    uint8_t set;                    /* the registers this routine's code set since the last replay */
    uint8_t replays;
    MenuReplay replay[6];
} MenuRegs;

/* The registers a routine is entered with from original code. */
static MenuRegs menu_regs_cpu(void)
{
    MenuRegs r = { 0 };

    r.a = cpu.a;
    r.x = cpu.x;
    r.y = cpu.y;
    r.nz = (cpu.p & P_Z) ? 0 : (cpu.p & P_N) ? 0x80 : 1;
    r.c = (cpu.p & P_C) != 0;
    r.v = (cpu.p & P_V) != 0;
    return r;
}

static void set_regs_shim(uint8_t set, uint8_t a, uint8_t x, uint8_t y, uint8_t nz, bool c, bool v)
{
    if (set & REG_A) {
        cpu.a = a;
    }
    if (set & REG_X) {
        cpu.x = x;
    }
    if (set & REG_Y) {
        cpu.y = y;
    }
    if (set & REG_NZ) {
        abi_nz(nz);
    }
    if (set & REG_C) {
        abi_flag(P_C, c);
    }
    if (set & REG_V) {
        abi_flag(P_V, v);
    }
}

/* The CPU as the original leaves it: each replayed callee's exit in order, with what the code set around them. */
static void menu_regs_shim(const MenuRegs *r)
{
    for (uint8_t i = 0; i < r->replays; i++) {
        const MenuReplay *p = &r->replay[i];

        set_regs_shim(p->set, p->a, p->x, p->y, p->nz, p->c, p->v);
        switch (p->kind) {
        case REPLAY_RESET:
            reset_game_state_exit_shim();
            break;
        case REPLAY_BACKUP_READ:
            backup_read_exit_shim(p->call.backup);
            break;
        case REPLAY_BACKUP_WRITE:
            backup_write_exit_shim(&p->call.write);
            break;
        case REPLAY_FORMAT:
            cpu.a = 0x6C;                           /* BackupFormat's LDA of the blank name's high byte */
            abi_nz(0x6C);
            bm_format_exit_shim(p->call.format);
            cpu.x = cpu.a;
            abi_nz(cpu.a);
            break;
        case REPLAY_TRACK:
            play_track_exit_shim(p->call.track);
            break;
        case REPLAY_SOUND:
            sound_command_exit_shim(p->command, p->call.sound);
            break;
        case REPLAY_CD_PAUSE:
            cd_pause_exit_shim(p->call.pause);
            break;
        case REPLAY_CD_FADE:
            cd_fade_exit_shim(p->command);
            break;
        }
    }
    set_regs_shim(r->set, r->a, r->x, r->y, r->nz, r->c, r->v);
}

/* A callee whose registers come back only through its exit shim: what the code set so far goes before it. */
static MenuReplay *menu_replay(MenuRegs *r, ReplayKind kind)
{
    MenuReplay *p = &r->replay[r->replays++];

    p->kind = kind;
    p->set = r->set;
    p->a = r->a;
    p->x = r->x;
    p->y = r->y;
    p->nz = r->nz;
    p->c = r->c;
    p->v = r->v;
    r->set = 0;
    return p;
}

/* Setting one register or flag, as an instruction does. */
static void ra(MenuRegs *r, uint8_t a)
{
    r->a = a;
    r->nz = a;
    r->set |= REG_A | REG_NZ;
}

static void rx(MenuRegs *r, uint8_t x)
{
    r->x = x;
    r->nz = x;
    r->set |= REG_X | REG_NZ;
}

static void ry(MenuRegs *r, uint8_t y)
{
    r->y = y;
    r->nz = y;
    r->set |= REG_Y | REG_NZ;
}

static void ra_keep(MenuRegs *r, uint8_t a)            /* CLA, or a load whose flags are overwritten unseen */
{
    r->a = a;
    r->set |= REG_A;
}

static void ry_keep(MenuRegs *r, uint8_t y)            /* CLY */
{
    r->y = y;
    r->set |= REG_Y;
}

static void rnz(MenuRegs *r, uint8_t nz)
{
    r->nz = nz;
    r->set |= REG_NZ;
}

static void rc(MenuRegs *r, bool c)
{
    r->c = c;
    r->set |= REG_C;
}

static void rv(MenuRegs *r, bool v)
{
    r->v = v;
    r->set |= REG_V;
}

/* An Exit merged into the register file: what it sets over what stood. */
static void rexit(MenuRegs *r, Exit e)
{
    if (e.from_cpu) {
        return;
    }
    if (!e.a_kept) {
        r->a = e.a;
        r->set |= REG_A;
    }
    if (!e.x_kept) {
        r->x = e.x;
        r->set |= REG_X;
    }
    if (!e.y_kept) {
        r->y = e.y;
        r->set |= REG_Y;
    }
    rnz(r, e.nz);
    if (!e.c_kept) {
        rc(r, e.c);
    }
    if (e.v_set) {
        rv(r, e.v);
    }
}

/* A compare, CMP, CPX or CPY: N and Z from the difference, C no borrow. */
static void rcmp(MenuRegs *r, uint8_t reg, uint8_t value)
{
    rnz(r, (uint8_t)(reg - value));
    rc(r, reg >= value);
}

/* SBC with the carry set before it (SEC): A, N, Z, C and V. */
static uint8_t rsub(MenuRegs *r, uint8_t a, uint8_t value)
{
    uint8_t diff = (uint8_t)(a - value);

    ra(r, diff);
    rc(r, a >= value);
    rv(r, ((a ^ value) & (a ^ diff) & 0x80) != 0);
    return diff;
}

/* ADC with the carry clear before it (CLC). */
static uint8_t radd(MenuRegs *r, uint8_t a, uint8_t value)
{
    unsigned sum = (unsigned)a + value;

    ra(r, (uint8_t)sum);
    rc(r, sum > 0xFF);
    rv(r, ((a ^ sum) & (value ^ sum) & 0x80) != 0);
    return (uint8_t)sum;
}

/* What a callee with a typed exit leaves. */
static void r_event(MenuRegs *r, ByteIndex b)          /* EventBitTest, EventBitSet, EventBitClear */
{
    ra(r, b.value);
    r->y = b.index;
    r->set |= REG_Y;
    rc(r, false);
}

static bool move_carry(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;

    return move.before + added > 0xFFFFFF;
}

static bool move_overflow(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;
    uint32_t sum = move.before + added;
    uint8_t sign = (uint8_t)(added >> 16);
    uint8_t high = (uint8_t)(move.before >> 16);
    uint8_t result = (uint8_t)(sum >> 16);

    return (~(sign ^ high) & (sign ^ result) & 0x80) != 0;
}

static void r_glide(MenuRegs *r, GlideExit g, uint8_t x)  /* GlideCursorObjects (X 3), JitterCursorObjects (X 1) */
{
    if (g.moved) {
        rc(r, move_carry(g.move));
        rv(r, move_overflow(g.move));
    }
    rexit(r, g.exit);
    r->x = x;
    r->set |= REG_X;
}

static void r_pad(MenuRegs *r, PadMoveExit p)           /* CursorMoveByPad */
{
    if (p.moved) {
        r->x = p.x;
        r->set |= REG_X;
    }
    r->y = p.exit.y;
    r->set |= REG_Y;
    ra(r, p.exit.a);
    rnz(r, p.exit.nz);
    if (!p.exit.c_kept) {
        rc(r, p.exit.c);
    }
}

static void r_found(MenuRegs *r, SlotFound f)           /* SlotLastClearedFlag */
{
    ra(r, f.a);
    rx(r, f.x);
    r->y = f.y;
    r->set |= REG_Y;
    rnz(r, f.found ? f.a : f.x);
    rc(r, true);
    rv(r, f.v);
}

/* Y as the original holds it: set by the code, or by the last replayed callee that sets it. */
static uint8_t card_y(CardExit e, uint8_t before)
{
    return e.set_y ? e.y : before;
}

static uint8_t menu_y(const MenuRegs *r)
{
    uint8_t y;
    int i;

    if (r->set & REG_Y) {
        return r->y;
    }
    for (i = r->replays - 1; i >= 0; i--) {
        const MenuReplay *p = &r->replay[i];

        y = (p->set & REG_Y) ? p->y : cpu.y;
        switch (p->kind) {
        case REPLAY_RESET:
            return 0xFF;
        case REPLAY_BACKUP_READ:
            if (p->call.backup.exit.set_y) {
                return p->call.backup.exit.y;
            }
            break;
        case REPLAY_FORMAT:
            if (p->call.format.exit.set_y) {
                return p->call.format.exit.y;
            }
            break;
        case REPLAY_BACKUP_WRITE: {
            const BackupWriteRun *w = &p->call.write;

            y = card_y(w->removal.exit, y);
            y = card_y(w->free_space.exit, y);
            if (w->fits) {
                y = card_y(w->write.exit, y);
            }
            if (w->removal.exit.set_y || w->free_space.exit.set_y || (w->fits && w->write.exit.set_y)) {
                return y;
            }
            break;
        }
        default:
            break;
        }
        if (p->set & REG_Y) {
            return p->y;
        }
    }
    return cpu.y;
}

/* The registers a JSR to code that waits hands it. */
static WaitRegs menu_wait_regs(const MenuRegs *r)
{
    WaitRegs w;

    w.a = (r->set & REG_A) || r->replays ? r->a : cpu.a;
    w.x = r->x;
    w.y = (r->set & REG_Y) || r->replays ? r->y : cpu.y;
    w.n = (r->nz & 0x80) != 0;
    w.z = r->nz == 0;
    w.c = r->c;
    w.v = (r->set & REG_V) ? r->v : (cpu.p & P_V) != 0;
    return w;
}

/* The menu's own RAM: the backup file's three slots ($30 bytes from SaveBuffer, at the offsets of SLOT_OFFSETS), and
 * beside them, by slot number 1 to 3, what the menu keeps for each slot. */
#define SLOT_OFFSETS_BY_NUMBER (SLOT_OFFSETS - 1)   /* by slot number 1 to 3 */
#define SLOT_PROGRESS_BY_NUMBER (MenuSlotProgress - 1)
#define SLOT_CHARACTER_BY_NUMBER (MenuSlotCharacter - 1)
#define SLOT_MUSIC_BY_NUMBER (MenuSlotMusic - 1)
#define ERASE_TIMER 0x7DFE                           /* frames left before a slot is erased */
#define SLOT_BYTES 0x30
#define MENU_DISPATCH_OPERAND 0x9E00                /* the operand of MenuStepDispatch's JMP, rewritten each call */
#define SCROLL_TARGET 0x7DFB                        /* the scroll high byte a menu page moves to */

/*
 * @routine 00C3:265 FileMenuReadSave
 * @run 6B:9265
 * WHAT IT DOES: Page 0, step 0: the game state wiped, the backup file read. Read: the menu's music (track $0E) starts,
 * the slots' characters and marks are cleared and each slot's progress found, and the step goes on. Not formatted: the
 * memory is formatted, then as for no file. No file: an empty file ($90 zero bytes) is written; written, as for read;
 * no room, page $0D with ContinueChoice 0. A bad file (2): page $0D with ContinueChoice 1. Any other answer to the
 * write: the game stops in a loop that never ends.
 * HOW: ResetGameState, BackupRead, its answer through TAX, DEC and INX; BackupFormat; a loop of STZ over SaveBuffer;
 * BackupWrite; CdPlayRepeat; SlotLastClearedFlag for zSaveSlot 0, 1 and 2; MenuStepAdvance or SetMenuPage.
 * WAITS: yes: in ResetGameState (the vertical blank) and CdPlayRepeat (the drive); the stack there is this routine's
 * caller's return and the call's two bytes. A write that fails other than for room waits for ever (BRA to itself).
 * Entry: from MenuStepDispatch. Exit, read or written: as MenuStepAdvance, X the third slot's progress, Y, C and V as
 * SlotLastClearedFlag leaves them. Page $0D: A 0, N and Z from $0D, X the answer plus one (bad file) or the write's
 * answer (no room), Y, C and V as BackupRead or BackupWrite leaves them.
 */
static MenuRegs save_ready(MenuRegs r)
{
    SlotFound found;
    TrackPlay track;
    MenuReplay *p;
    uint8_t step;

    ra(&r, 0x0E);
    jsr_push(0x9283);
    track = CdPlayRepeat(0x0E, r.x, menu_y(&r));
    jsr_pop();
    p = menu_replay(&r, REPLAY_TRACK);
    p->call.track = track;
    wr8(MenuSlotCharacter, 0);
    wr8(MenuSlotCharacter + 1, 0);
    wr8(MenuSlotCharacter + 2, 0);
    wr8(MenuSlotMusic, 0);
    wr8(MenuSlotMusic + 1, 0);
    wr8(MenuSlotMusic + 2, 0);
    WR8(ZP->save_slot, 0);
    jsr_push(0x929A);
    found = SlotLastClearedFlag();
    jsr_pop();
    r_found(&r, found);
    wr8(MenuSlotProgress, found.x);
    rnz(&r, INC8(ZP->save_slot));
    jsr_push(0x92A2);
    found = SlotLastClearedFlag();
    jsr_pop();
    r_found(&r, found);
    wr8(MenuSlotProgress + 1, found.x);
    rnz(&r, INC8(ZP->save_slot));
    jsr_push(0x92AA);
    found = SlotLastClearedFlag();
    jsr_pop();
    r_found(&r, found);
    wr8(MenuSlotProgress + 2, found.x);
    step = MenuStepAdvance();
    ra(&r, step);
    return r;
}

static MenuRegs save_fresh(MenuRegs r)
{
    BackupWriteRun write;
    MenuReplay *p;
    uint8_t x, result;

    x = 0;
    do {
        wr8((uint16_t)(SaveBuffer + x), 0);
        x++;
    } while (x != 0x90);
    ra_keep(&r, 0);
    rx(&r, 0x90);
    rcmp(&r, 0x90, 0x90);
    jsr_push(0x92BF);
    write = BackupWrite();
    jsr_pop();
    p = menu_replay(&r, REPLAY_BACKUP_WRITE);
    p->call.write = write;
    result = write.fits ? bm_write_result(write.write) : 1;
    rx(&r, result);
    if (result == 0) {
        return save_ready(r);
    }
    if ((uint8_t)(result - 1) != 0) {
        for (;;) {
            wait_interrupt();
        }
    }
    wr8(ContinueChoice, 0);
    SetMenuPage(0x0D);
    ra(&r, 0);
    rnz(&r, 0x0D);
    return r;
}

MenuRegs FileMenuReadSave(MenuRegs r)
{
    BackupAccess read;
    BackupFormat format;
    MenuReplay *p;
    uint8_t result;

    jsr_push(0x9268);
    ResetGameState(menu_wait_regs(&r));
    jsr_pop();
    (void)menu_replay(&r, REPLAY_RESET);
    jsr_push(0x926B);
    read = BackupRead();
    jsr_pop();
    p = menu_replay(&r, REPLAY_BACKUP_READ);
    p->call.backup = read;
    result = bm_read_result(read);
    rx(&r, result);
    if (result == 0) {
        return save_ready(r);
    }
    ra(&r, (uint8_t)(result - 1));
    if (r.a == 0) {
        return save_fresh(r);
    }
    rx(&r, (uint8_t)(result + 1));
    if (r.x == 0) {
        jsr_push(0x92B3);
        format = BackupFormatMemory();
        jsr_pop();
        p = menu_replay(&r, REPLAY_FORMAT);
        p->call.format = format;
        return save_fresh(r);
    }
    wr8(ContinueChoice, 1);
    SetMenuPage(0x0D);
    ra(&r, 0);
    rnz(&r, 0x0D);
    return r;
}

void abi_FileMenuReadSave(void)
{
    MenuRegs r = FileMenuReadSave(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:2CF FileMenuStartScreen
 * @run 6B:92CF
 * WHAT IT DOES: Page 0, step 1: every event flag cleared, list $0D started on channel 0, lists 0 and 1 on channels 7
 * and 6 as kind 3, and the menu goes on to page 1.
 * HOW: EventFlagsClear, StartListChannel0Kind2, StartListChannelKind3 twice, MenuPageAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: A 0, X 6, N and Z from the new page; Y, C and V kept.
 */
MenuRegs FileMenuStartScreen(MenuRegs r)
{
    uint8_t page;

    jsr_push(0x92D2);
    EventFlagsClear();
    jsr_pop();
    jsr_push(0x92D7);
    (void)StartListChannel0Kind2(0x0D);
    jsr_pop();
    jsr_push(0x92DD);
    (void)StartListChannelKind3(0, 7);
    jsr_pop();
    jsr_push(0x92E4);
    (void)StartListChannelKind3(1, 6);
    jsr_pop();
    page = MenuPageAdvance();
    ra(&r, 0);
    rx(&r, 6);
    rnz(&r, page);
    return r;
}

void abi_FileMenuStartScreen(void)
{
    MenuRegs r = FileMenuStartScreen(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* What MenuPickSlot leaves: dropped when it pulled its caller's return address and ended the caller's step itself. */
typedef struct { MenuRegs r; bool dropped; } MenuPick;

/* The end both drops of MenuPickSlot share: PLA twice, then MenuBackOnButtonII, or MenuBackToPageEight. */
static MenuPick pick_dropped(MenuRegs r, bool to_page_eight)
{
    MenuPick pick;
    uint8_t high;

    (void)pull8();
    high = pull8();
    ra(&r, high);
    if (to_page_eight || MenuBackOnButtonII()) {
        if (!to_page_eight) {
            ra(&r, 2);
        }
        if (to_page_eight) {
            MenuBackToPageEight();
        }
        ra(&r, 0);
        rnz(&r, 8);
    } else {
        ra(&r, 0);
    }
    pick.r = r;
    pick.dropped = true;
    return pick;
}

/*
 * @routine 00C3:2F5 MenuPickSlot
 * @run 6B:92F5
 * WHAT IT DOES: The pick of a slot on the pages that choose one: the cursor moves by the pad and shakes; on button I over
 * a slot (cursor values 9 to 11) it answers whether the slot has data. Button I over item 8, or no button I, ends the
 * caller's step: back to page 8 on button II. Button I over item $0C: back to page 8.
 * HOW: CursorMoveByPad (menu 1), JitterCursorObjects, the trigger's bit 0, the cursor value (OBJ->work_d of object 0)
 * compared; EventBitTest with the slot number as a tail jump. The ends that leave the step pull the caller's return
 * address (two PLA) and go to MenuBackOnButtonII or MenuBackToPageEight, so they return to the caller's caller.
 * WAITS: no.
 * Entry: nothing. Exit, a slot: as EventBitTest (Z set: no data), V from the slot number's subtract. Otherwise the
 * caller's frame is dropped: as MenuBackOnButtonII or MenuBackToPageEight, X, Y, C and V as JitterCursorObjects
 * left them (C set by the compare with $0C on its path).
 */
MenuPick MenuPickSlot(MenuRegs r)
{
    MenuPick pick;
    PadMoveExit pad;
    GlideExit jitter;
    uint8_t trigger, value;

    ry(&r, 1);
    jsr_push(0x92FA);
    pad = CursorMoveByPad(1);
    jsr_pop();
    r_pad(&r, pad);
    jsr_push(0x92FD);
    jitter = JitterCursorObjects();
    jsr_pop();
    r_glide(&r, jitter, 1);
    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x01));
    if (r.a == 0) {
        return pick_dropped(r, false);
    }
    value = RD8(OBJ->work_d[0]);
    ra(&r, value);
    rcmp(&r, value, 0x08);
    if (value == 0x08) {
        return pick_dropped(r, false);
    }
    rcmp(&r, value, 0x0C);
    if (value == 0x0C) {
        return pick_dropped(r, true);
    }
    value = rsub(&r, value, 0x08);
    r_event(&r, EventBitTest(value));
    pick.r = r;
    pick.dropped = false;
    return pick;
}

void abi_MenuPickSlot(void)
{
    MenuPick pick = MenuPickSlot(menu_regs_cpu());

    menu_regs_shim(&pick.r);
}

/* MenuBackOnButtonII as a tail jump from a step: its registers over the step's. */
static MenuRegs back_on_button_ii(MenuRegs r)
{
    uint8_t trigger = rd8(BiosJoyTrigger);

    ra(&r, (uint8_t)(trigger & 0x02));
    if (r.a == 0) {
        return r;
    }
    MenuBackToPageEight();
    ra(&r, 0);
    rnz(&r, 8);
    return r;
}

/* The end FileMenuPickEmpty and FileMenuPickUsed share ($932C): the slot chosen, and page 9. */
static MenuRegs select_slot(MenuRegs r)
{
    uint8_t value = RD8(OBJ->work_d[0]);

    value = rsub(&r, value, 0x08);
    wr8(MenuSelectedSlot, value);
    SetMenuPage(0x09);
    ra(&r, 0);
    rnz(&r, 0x09);
    return r;
}

/*
 * @routine 00C3:327 FileMenuPickEmpty
 * @run 6B:9327
 * WHAT IT DOES: Page 3: a slot with no data is picked (one with data is refused), and becomes the selected file; page 9.
 * HOW: MenuPickSlot; on Z clear MenuBackOnButtonII; the cursor value less 8 into MenuSelectedSlot; SetMenuPage 9.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuPickSlot's dropped ends, as MenuBackOnButtonII, or A 0, N and Z from 9, C
 * and V from the subtract; X and Y as MenuPickSlot left them.
 */
MenuRegs FileMenuPickEmpty(MenuRegs r)
{
    MenuPick pick;

    jsr_push(0x932A);
    pick = MenuPickSlot(r);
    if (pick.dropped) {
        return pick.r;
    }
    jsr_pop();
    if (pick.r.nz != 0) {
        return back_on_button_ii(pick.r);
    }
    return select_slot(pick.r);
}

void abi_FileMenuPickEmpty(void)
{
    MenuRegs r = FileMenuPickEmpty(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:33A FileMenuPickUsed
 * @run 6B:933A
 * WHAT IT DOES: Page 4: a slot with data is picked (an empty one is refused), and becomes the selected file; page 9.
 * HOW: MenuPickSlot; on Z set MenuBackOnButtonII; else the end of FileMenuPickEmpty ($932C).
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as FileMenuPickEmpty.
 */
MenuRegs FileMenuPickUsed(MenuRegs r)
{
    MenuPick pick;

    jsr_push(0x933D);
    pick = MenuPickSlot(r);
    if (pick.dropped) {
        return pick.r;
    }
    jsr_pop();
    if (pick.r.nz == 0) {
        return back_on_button_ii(pick.r);
    }
    return select_slot(pick.r);
}

void abi_FileMenuPickUsed(void)
{
    MenuRegs r = FileMenuPickUsed(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:34A FileMenuEraseStart
 * @run 6B:934A
 * WHAT IT DOES: Page 5, step 0: a slot with data is picked for erasing; its text row is overwritten with eight tiles
 * $10FF, sound $3C plays, the erase count is set to $80 frames and the step goes on.
 * HOW: MenuPickSlot; on Z set MenuBackOnButtonII. The slot's row address from TEXT_ROW_ADDRESSES (rows 3 to 5) is
 * written to the VDC after SelectVramWriteAddress; SelectVramData; eight ST1 $FF, ST2 $10; SoundCommand; ERASE_TIMER;
 * MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuPickSlot's dropped ends, as MenuBackOnButtonII, or as MenuStepAdvance
 * with Y the row index doubled, X, C and V as SoundCommand leaves them.
 */
MenuRegs FileMenuEraseStart(MenuRegs r)
{
    MenuPick pick;
    CommandCall sound;
    MenuReplay *p;
    uint8_t value, index, low, high, count;

    jsr_push(0x934D);
    pick = MenuPickSlot(r);
    if (pick.dropped) {
        return pick.r;
    }
    jsr_pop();
    r = pick.r;
    if (r.nz == 0) {
        return back_on_button_ii(r);
    }
    value = RD8(OBJ->work_d[0]);
    value = rsub(&r, value, 0x09);
    index = (uint8_t)(value << 1);
    ra(&r, index);
    rc(&r, (value & 0x80) != 0);
    ry(&r, index);
    jsr_push(0x935A);
    SelectVramWriteAddress();
    jsr_pop();
    low = rd8((uint16_t)(TEXT_ROW_ADDRESSES + 6 + index));
    WR8(VDC->data_lo, low);
    high = rd8((uint16_t)(TEXT_ROW_ADDRESSES + 7 + index));
    WR8(VDC->data_hi, high);
    jsr_push(0x9369);
    SelectVramData();
    jsr_pop();
    count = 8;
    do {
        st1(0xFF);
        st2(0x10);
        count--;
    } while (count != 0);
    rx(&r, 0);
    ra(&r, 0x3C);
    jsr_push(0x9377);
    sound = SoundCommand(0x3C, 0, index);
    jsr_pop();
    p = menu_replay(&r, REPLAY_SOUND);
    p->command = 0x3C;
    p->call.sound = sound;
    wr8(ERASE_TIMER, 0x80);
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuEraseStart(void)
{
    MenuRegs r = FileMenuEraseStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* The end both arms of FileMenuEraseWait share ($93C4): zField80 back one, the step before. */
static MenuRegs erase_step_back(MenuRegs r)
{
    rnz(&r, DEC8(ZP->field80));
    return r;
}

/*
 * @routine 00C3:37F FileMenuEraseWait
 * @run 6B:937F
 * WHAT IT DOES: Page 5, step 1: the cursor shakes while the erase count runs down. Button II stops it: the slot's name
 * is drawn back. At zero the slot is erased: its character and mark go to zero, its $30 bytes of the backup file are
 * cleared, its event bit is taken out, its row's colours set back and its text row blanked. Either way the menu goes
 * back to step 0.
 * HOW: JitterCursorObjects; the trigger's bit 1; ERASE_TIMER counted down; the slot number from the cursor value;
 * STZ over its bytes; EventBitClear, RecolorTextRun, BlankTextRow; or DrawSlotName; zField80 counted down.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit, counting: N and Z from the count, the rest as JitterCursorObjects left them.
 * Otherwise N and Z from the step, A, X, Y, C and V as BlankTextRow or DrawSlotName leaves them.
 */
MenuRegs FileMenuEraseWait(MenuRegs r)
{
    GlideExit jitter;
    uint8_t trigger, value, slot, x, y, row;

    jsr_push(0x9382);
    jitter = JitterCursorObjects();
    jsr_pop();
    r_glide(&r, jitter, 1);
    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x02));
    if (r.a != 0) {
        value = RD8(OBJ->work_d[0]);
        row = rsub(&r, value, 0x06);
        jsr_push(0x93D0);
        rexit(&r, DrawSlotName(row, r.x));
        jsr_pop();
        return erase_step_back(r);
    }
    rnz(&r, (uint8_t)(rd8(ERASE_TIMER) - 1));
    wr8(ERASE_TIMER, r.nz);
    if (r.nz != 0) {
        return r;
    }
    value = RD8(OBJ->work_d[0]);
    slot = rsub(&r, value, 0x08);
    ry(&r, slot);
    ra_keep(&r, 0);
    wr8((uint16_t)(SLOT_PROGRESS_BY_NUMBER + slot), 0);
    wr8((uint16_t)(SLOT_CHARACTER_BY_NUMBER + slot), 0);
    x = rd8((uint16_t)(SLOT_OFFSETS_BY_NUMBER + slot));
    y = 0;
    do {
        wr8((uint16_t)(SaveBuffer + x), 0);
        x++;
        y++;
    } while (y != SLOT_BYTES);
    rx(&r, x);
    ry(&r, y);
    rcmp(&r, y, SLOT_BYTES);
    value = RD8(OBJ->work_d[0]);
    value = rsub(&r, value, 0x08);
    jsr_push(0x93B2);
    r_event(&r, EventBitClear(value));
    jsr_pop();
    value = RD8(OBJ->work_d[0]);
    row = (uint8_t)(value - 2);
    ra(&r, row);
    ry_keep(&r, 0);
    jsr_push(0x93BB);
    rexit(&r, RecolorTextRun(row, 0));
    jsr_pop();
    value = RD8(OBJ->work_d[0]);
    row = rsub(&r, value, 0x06);
    jsr_push(0x93C4);
    rexit(&r, BlankTextRow(row, r.x));
    jsr_pop();
    return erase_step_back(r);
}

void abi_FileMenuEraseWait(void)
{
    MenuRegs r = FileMenuEraseWait(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:3E2 FileMenuCopyPickFrom
 * @run 6B:93E2
 * WHAT IT DOES: Page 6, step 0: the cursor moves by the pad over the copy page's items; button I over a slot with data
 * (cursor values $0E to $10) makes it the slot copied from, draws the copy list for it and goes on a step. Item $11, or
 * an empty slot, or no button I: back to page 8 (an empty slot and no press only on button II).
 * HOW: CursorMoveByPad (menu 2), the trigger's bit 0, the cursor value compared with $11; less $0D to MenuCopyFrom;
 * EventBitTest; StartListChannelKind3 with the value less $0C on channel 0; MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuBackOnButtonII, MenuBackToPageEight (C set from the compare) or
 * MenuStepAdvance (X 0, Y the event byte, C and V from the subtract).
 */
MenuRegs FileMenuCopyPickFrom(MenuRegs r)
{
    PadMoveExit pad;
    uint8_t trigger, value, list;

    ry(&r, 2);
    jsr_push(0x93E7);
    pad = CursorMoveByPad(2);
    jsr_pop();
    r_pad(&r, pad);
    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x01));
    if (r.a == 0) {
        return back_on_button_ii(r);
    }
    value = RD8(OBJ->work_d[0]);
    ra(&r, value);
    rcmp(&r, value, 0x11);
    if (value == 0x11) {
        MenuBackToPageEight();
        ra(&r, 0);
        rnz(&r, 8);
        return r;
    }
    value = rsub(&r, value, 0x0D);
    wr8(MenuCopyFrom, value);
    jsr_push(0x9404);
    r_event(&r, EventBitTest(value));
    jsr_pop();
    if (r.nz == 0) {
        return back_on_button_ii(r);
    }
    rx(&r, 0);
    value = RD8(OBJ->work_d[0]);
    list = rsub(&r, value, 0x0C);
    jsr_push(0x9413);
    (void)StartListChannelKind3(list, 0);
    jsr_pop();
    ra(&r, 3);
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuCopyPickFrom(void)
{
    MenuRegs r = FileMenuCopyPickFrom(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DD:7A0 DrawSlotRows
 * @run 6F:A7A0
 * WHAT IT DOES: Draws the three slots' rows of the copy page, from the third up: a slot with data gets its name and its
 * percentage; each row is coloured with palette 1 or 0 by whether the slot has data, swapped when the argument is 1.
 * HOW: zTemp01 the argument, zTemp00 the slot counted down from 3; EventBitTest; DrawSlotName and DrawSlotPercent at
 * the slot plus 8; RecolorTextRun at the slot plus 9 with the data bit EOR the argument.
 * WAITS: no.
 * Entry: A 0 or 1. Exit: A $30, Y and C as RecolorTextRun leaves them, Z set, N clear (the last count); X kept.
 */
MenuRegs DrawSlotRows(MenuRegs r, uint8_t swap)
{
    uint8_t slot, value, row;

    WR8(ZP->temp01, swap);
    ra(&r, 3);
    WR8(ZP->temp00, 3);
    do {
        slot = RD8(ZP->temp00);
        jsr_push(0xA7AB);
        r_event(&r, EventBitTest(slot));
        jsr_pop();
        if (r.nz != 0) {
            slot = RD8(ZP->temp00);
            row = radd(&r, slot, 0x08);
            jsr_push(0xA7B5);
            rexit(&r, DrawSlotName(row, r.x));
            jsr_pop();
            slot = RD8(ZP->temp00);
            row = radd(&r, slot, 0x08);
            jsr_push(0xA7BD);
            rexit(&r, DrawSlotPercent(row, r.x));
            jsr_pop();
            ra(&r, 1);
        } else {
            ra_keep(&r, 0);
        }
        value = (uint8_t)(r.a ^ RD8(ZP->temp01));
        ra(&r, value);
        ry(&r, value);
        slot = RD8(ZP->temp00);
        row = radd(&r, slot, 0x09);
        jsr_push(0xA7CD);
        rexit(&r, RecolorTextRun(row, value));
        jsr_pop();
        rnz(&r, DEC8(ZP->temp00));
    } while (r.nz != 0);
    return r;
}

void abi_DrawSlotRows(void)
{
    MenuRegs r = DrawSlotRows(menu_regs_cpu(), cpu.a);

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:416 FileMenuCopyDrawRows
 * @run 6B:9416
 * WHAT IT DOES: Page 6, step 1: the slots' rows drawn with the colours swapped, and the step goes on.
 * HOW: DrawSlotRows with 1; MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance, the rest as DrawSlotRows leaves them.
 */
MenuRegs FileMenuCopyDrawRows(MenuRegs r)
{
    ra(&r, 1);
    jsr_push(0x941B);
    r = DrawSlotRows(r, 1);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuCopyDrawRows(void)
{
    MenuRegs r = FileMenuCopyDrawRows(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* The end of a copy, made or given up ($945B): the rows drawn again, list $17 on channel 0, the page's step 0. */
static MenuRegs copy_end(MenuRegs r)
{
    ra_keep(&r, 0);
    jsr_push(0x945F);
    r = DrawSlotRows(r, 0);
    jsr_pop();
    ra(&r, 0x17);
    jsr_push(0x9464);
    (void)StartListChannel0Kind2(0x17);
    jsr_pop();
    ra(&r, 2);
    rnz(&r, 2);
    rx(&r, 0);
    rnz(&r, 2);
    ra_keep(&r, 0);
    jsr_push(0x9468);
    SetMenuStep(0);
    jsr_pop();
    return r;
}

/*
 * @routine 00C3:454 FileMenuCopyCancel
 * @run 6B:9454
 * WHAT IT DOES: The end of the copy page's second pick: on button II the copy is given up (the rows drawn again, list
 * $17, the page's step 0); otherwise nothing.
 * HOW: The trigger's bit 1; DrawSlotRows with 0, StartListChannel0Kind2, SetMenuStep 0 by JSR.
 * WAITS: no.
 * Entry: from FileMenuCopyPickTo. Exit, no button: A 0, Z set, N clear. Given up: A 0, X 0, N and Z clear (from the
 * list start), Y and C as DrawSlotRows leaves them.
 */
MenuRegs FileMenuCopyCancel(MenuRegs r)
{
    uint8_t trigger = rd8(BiosJoyTrigger);

    ra(&r, (uint8_t)(trigger & 0x02));
    if (r.a == 0) {
        return r;
    }
    return copy_end(r);
}

void abi_FileMenuCopyCancel(void)
{
    MenuRegs r = FileMenuCopyCancel(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:41E FileMenuCopyPickTo
 * @run 6B:941E
 * WHAT IT DOES: Page 6, step 2: the cursor moves by the pad; button I over an empty slot (cursor values $0E to $10)
 * makes it the slot copied to, draws the copy list for it, sets the copy's wait to $80 frames and goes on a step.
 * Item $11: back to page 8. A slot with data, or no button I: FileMenuCopyCancel.
 * HOW: CursorMoveByPad (menu 2), the trigger's bit 0, the cursor value compared with $11; less $0D to MenuCopyTo;
 * EventBitTest; StartListChannelKind3 with the value less 9 on channel 0; OBJ->work_d of object $26 the wait;
 * MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as FileMenuCopyCancel, MenuBackToPageEight, or MenuStepAdvance (X 0, Y the
 * event byte, C and V from the subtract).
 */
MenuRegs FileMenuCopyPickTo(MenuRegs r)
{
    PadMoveExit pad;
    uint8_t trigger, value, list;

    ry(&r, 2);
    jsr_push(0x9423);
    pad = CursorMoveByPad(2);
    jsr_pop();
    r_pad(&r, pad);
    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x01));
    if (r.a == 0) {
        return FileMenuCopyCancel(r);
    }
    value = RD8(OBJ->work_d[0]);
    ra(&r, value);
    rcmp(&r, value, 0x11);
    if (value == 0x11) {
        MenuBackToPageEight();
        ra(&r, 0);
        rnz(&r, 8);
        return r;
    }
    value = rsub(&r, value, 0x0D);
    wr8(MenuCopyTo, value);
    jsr_push(0x943D);
    r_event(&r, EventBitTest(value));
    jsr_pop();
    if (r.nz != 0) {
        return FileMenuCopyCancel(r);
    }
    rx(&r, 0);
    value = RD8(OBJ->work_d[0]);
    list = rsub(&r, value, 0x09);
    jsr_push(0x944C);
    (void)StartListChannelKind3(list, 0);
    jsr_pop();
    ra(&r, 0x80);
    WR8(OBJ->work_d[0x26], 0x80);
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuCopyPickTo(void)
{
    MenuRegs r = FileMenuCopyPickTo(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:469 FileMenuCopyMake
 * @run 6B:9469
 * WHAT IT DOES: Page 6, step 3: when the copy's wait has run out, the slot copied from is copied to the slot copied to:
 * its $30 bytes of the backup file, its progress, its character and its mark; list $18 starts on channel 1, the slot
 * copied to gets its event bit, and the copy ends as FileMenuCopyCancel's (the rows, list $17, step 0).
 * HOW: OBJ->work_d of object $26 counted down; the slots' offsets from SLOT_OFFSETS; zTemp00 counts the bytes; the three
 * per-slot bytes; StartListChannelKind2, EventBitSet; the end at $945B.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit, waiting: N and Z from the count; A, X, Y and C kept. Copied: as FileMenuCopyCancel.
 */
MenuRegs FileMenuCopyMake(MenuRegs r)
{
    uint8_t from, to, x, y, value, slot;

    rnz(&r, DEC8(OBJ->work_d[0x26]));
    if (r.nz != 0) {
        return r;
    }
    slot = rd8(MenuCopyFrom);
    x = rd8((uint16_t)(SLOT_OFFSETS_BY_NUMBER + slot));
    slot = rd8(MenuCopyTo);
    y = rd8((uint16_t)(SLOT_OFFSETS_BY_NUMBER + slot));
    WR8(ZP->temp00, SLOT_BYTES);
    do {
        value = rd8((uint16_t)(SaveBuffer + x));
        wr8((uint16_t)(SaveBuffer + y), value);
        x++;
        y++;
    } while (DEC8(ZP->temp00) != 0);
    from = rd8(MenuCopyFrom);
    to = rd8(MenuCopyTo);
    value = rd8((uint16_t)(SLOT_PROGRESS_BY_NUMBER + from));
    wr8((uint16_t)(SLOT_PROGRESS_BY_NUMBER + to), value);
    value = rd8((uint16_t)(SLOT_CHARACTER_BY_NUMBER + from));
    wr8((uint16_t)(SLOT_CHARACTER_BY_NUMBER + to), value);
    value = rd8((uint16_t)(SLOT_MUSIC_BY_NUMBER + from));
    wr8((uint16_t)(SLOT_MUSIC_BY_NUMBER + to), value);
    rx(&r, 1);
    ra(&r, 0x18);
    jsr_push(0x94AA);
    (void)StartListChannelKind2(0x18, 1);
    jsr_pop();
    value = rd8(MenuCopyTo);
    jsr_push(0x94B0);
    r_event(&r, EventBitSet(value));
    jsr_pop();
    return copy_end(r);
}

void abi_FileMenuCopyMake(void)
{
    MenuRegs r = FileMenuCopyMake(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* SoundCommand as a tail jump: A the command, its exit replayed over what stood. X and Y are the caller's. */
static MenuRegs sound_tail(MenuRegs r, uint8_t command, uint8_t x)
{
    CommandCall sound;
    MenuReplay *p;

    ra(&r, command);
    sound = SoundCommand(command, x, menu_y(&r));
    p = menu_replay(&r, REPLAY_SOUND);
    p->command = command;
    p->call.sound = sound;
    return r;
}

/* StepObjectFrames' A, C and flags: $FF and C for a changed frame, $FF held, 0 at the list's end. */
static uint8_t r_frames(MenuRegs *r, FrameStep step)
{
    uint8_t a = step == FRAME_LIST_ENDED ? 0 : 0xFF;

    ra(r, a);
    rc(r, step == FRAME_CHANGED);
    return a;
}

/* The character picture's animation lists by OBJ->start_arg (0 and 1 at rest, 2 and 3 turning to them), and the
 * second object's lists by the character. */
#define PORTRAIT_LISTS 0xBC90
#define PORTRAIT_FLASH_LISTS 0xBC98

/*
 * @routine 00E0:443 AnimateCharacterPortrait
 * @run 6F:BC43
 * WHAT IT DOES: Steps the animation of the menu's character picture (the object whose start argument the character
 * page sets to the slot's character). When the start argument has changed from the one shown (OBJ->work_d), the
 * turning animation of the new one (start argument plus 2) starts; at its end the picture rests on its character's
 * own (start argument less 2). Frames $31 and $37 of the turn play sounds $79 and $78.
 * HOW: OBJ->start_arg against OBJ->work_d; OBJ->anim_timer and OBJ->anim_step cleared; the list from PORTRAIT_LISTS
 * through zTemp10 and zTemp11; StepObjectFrames; OBJ->frame compared; SoundCommand as a tail jump.
 * WAITS: no.
 * Entry: X the object. Exit: Y A as StepObjectFrames left it (TAY); then held: A $FF, C clear, N set; ended: A the
 * start argument less 2, N and Z from it, C clear; changed: A the frame, N, Z and C from its last compare, or as
 * SoundCommand leaves them. X and V kept (V as StepObjectFrames keeps it).
 */
MenuRegs AnimateCharacterPortrait(MenuRegs r, uint8_t object)
{
    FrameStep step;
    uint8_t arg, shown, index, a, frame, step_y;

    arg = RD8(OBJ->start_arg[object]);
    ra(&r, arg);
    shown = RD8(OBJ->work_d[object]);
    rcmp(&r, arg, shown);
    if (arg != shown) {
        WR8(OBJ->anim_timer[object], 0);
        WR8(OBJ->anim_step[object], 0);
        arg = (uint8_t)(arg + 2);
        ra(&r, arg);
        WR8(OBJ->work_d[object], arg);
        WR8(OBJ->start_arg[object], arg);
    }
    index = (uint8_t)(arg << 1);
    ra(&r, index);
    rc(&r, (arg & 0x80) != 0);
    ry(&r, index);
    WR8(ZP->temp10, rd8((uint16_t)(PORTRAIT_LISTS + index)));
    WR8(ZP->temp11, rd8((uint16_t)(PORTRAIT_LISTS + 1 + index)));
    jsr_push(0xBC68);
    step = StepObjectFrames(object, &step_y);
    jsr_pop();
    a = r_frames(&r, step);
    ry(&r, a);
    if (a == 0) {
        arg = RD8(OBJ->start_arg[object]);
        arg = (uint8_t)(arg - 2);
        ra(&r, arg);
        if (arg & 0x80) {
            return r;
        }
        WR8(OBJ->start_arg[object], arg);
        WR8(OBJ->work_d[object], arg);
        return r;
    }
    if (step == FRAME_HELD) {
        return r;
    }
    frame = RD8(OBJ->frame[object]);
    ra(&r, frame);
    rcmp(&r, frame, 0x31);
    if (frame == 0x31) {
        return sound_tail(r, 0x79, object);
    }
    rcmp(&r, frame, 0x37);
    if (frame != 0x37) {
        return r;
    }
    return sound_tail(r, 0x78, object);
}

void abi_AnimateCharacterPortrait(void)
{
    MenuRegs r = AnimateCharacterPortrait(menu_regs_cpu(), cpu.x);

    menu_regs_shim(&r);
}

/*
 * @routine 00E0:40A AnimateCharacterChoice
 * @run 6F:BC0A
 * WHAT IT DOES: The character page's two objects for a frame: object $33 (the character picture) is animated while it
 * is on the screen (its Y low byte not zero); object $34 plays the list for the picture's character (bit 0 of $33's
 * start argument) while its OBJ->work_d is not zero: 2 starts the list afresh, 1 goes on, and the list's end sets 0.
 * HOW: PHX and PLX around it; AnimateCharacterPortrait; OBJ->work_d of $34 counted down from 2, its animation cleared;
 * the list from PORTRAIT_FLASH_LISTS through zTemp10 and zTemp11; StepObjectFrames.
 * WAITS: no.
 * Entry: nothing. Exit: X kept, N and Z from it (the PLX); A, Y, C and V as the last step left them.
 */
MenuRegs AnimateCharacterChoice(MenuRegs r)
{
    FrameStep step;
    uint8_t kept_x, low, count, index, a, step_y, object;

    kept_x = r.x;
    push8(kept_x);
    rx(&r, 0x33);
    low = RD8(OBJ->y_lo[0x33]);
    ra(&r, low);
    if (low != 0) {
        jsr_push(0xBC15);
        r = AnimateCharacterPortrait(r, 0x33);
        jsr_pop();
        object = (uint8_t)(r.x + 1);
        rx(&r, object);
        count = RD8(OBJ->work_d[object]);
        ra(&r, count);
        if (count != 0) {
            count = (uint8_t)(count - 1);
            ra(&r, count);
            if (count != 0) {
                WR8(OBJ->anim_timer[object], 0);
                WR8(OBJ->anim_step[object], 0);
                rnz(&r, DEC8(OBJ->work_d[object]));
            }
            a = RD8(OBJ->start_arg[(uint8_t)(object - 1)]);
            index = (uint8_t)((a & 0x01) << 1);
            ra(&r, index);
            rc(&r, false);
            ry(&r, index);
            WR8(ZP->temp10, rd8((uint16_t)(PORTRAIT_FLASH_LISTS + index)));
            WR8(ZP->temp11, rd8((uint16_t)(PORTRAIT_FLASH_LISTS + 1 + index)));
            jsr_push(0xBC3B);
            step = StepObjectFrames(object, &step_y);
            jsr_pop();
            a = r_frames(&r, step);
            ry(&r, a);
            if (a == 0) {
                WR8(OBJ->work_d[object], 0);
            }
        }
    }
    rx(&r, pull8());
    return r;
}

void abi_AnimateCharacterChoice(void)
{
    MenuRegs r = AnimateCharacterChoice(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* The stage a slot starts on, by its progress (MenuSlotProgress), and the stages the second pad's held buttons pick. */
#define START_STAGES 0x9574
#define START_STAGES_HELD 0x9581
#define SLOT_INTRO_SEEN 0x24                          /* a slot's byte: 1 once its first game has been started */
#define CHARACTER_OBJECT 0x33                         /* the character picture; $34 beside it */

/* NextRandom's A, N, Z, C and V. */
static uint8_t r_random(MenuRegs *r, RandomResult x)
{
    ra(r, x.value);
    rc(r, x.carry);
    rv(r, x.overflow);
    return x.value;
}

/*
 * @routine 00C3:4B2 FileMenuCharacterPage
 * @run 6B:94B2
 * WHAT IT DOES: Page 7, the slot's game page: the character objects animate; without button I the cursor moves and
 * shakes, now and then one of objects $2F to $32 turns round (every 32 frames, one time in 16), and button II goes back
 * to page 8. Button I picks the item: $14 swaps the slot's character (and the picture's), $15 page $0C, $16 and $17
 * set and clear MenuSlotMusic's first byte and go to page 9, $13 starts the game: Maria on a slot with no progress is
 * refused (sound $41); otherwise zCharacter is the slot's, zStage the start stage for its progress (from the other
 * table when pads 1 and 3 hold nothing but button I and pad 2 holds $8E), a slot's first game asks for the intro
 * scenes (bit 3 of zModeEvents), and the menu goes to page $10. Any other item is taken as no press.
 * HOW: AnimateCharacterChoice; the trigger's bit 0; CursorMoveByPad (menu 3), JitterCursorObjects; zFrameTick, two
 * NextRandom; OBJ->facing_left EOR 1; MenuBackOnButtonII by JSR. The item compared; MenuSlotCharacter, OBJ->start_arg of
 * the picture, OBJ->work_d of $34; SetMenuPage; SoundCommand; START_STAGES; BiosPadState; the slot's byte $24.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as the arm that ran last (MenuBackOnButtonII, SetMenuPage, SoundCommand, or the
 * swap: A the character, X 2, N and Z from 2).
 */
MenuRegs FileMenuCharacterPage(MenuRegs r)
{
    PadMoveExit pad;
    GlideExit jitter;
    uint8_t trigger, tick, value, item, slot, object, offset, held;

    jsr_push(0x94B5);
    r = AnimateCharacterChoice(r);
    jsr_pop();
    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x01));
    if (r.a != 0) {
        item = RD8(OBJ->work_d[0]);
        ra(&r, item);
        rcmp(&r, item, 0x15);
        if (item == 0x15) {
            SetMenuPage(0x0C);
            ra(&r, 0);
            rnz(&r, 0x0C);
            return r;
        }
        rcmp(&r, item, 0x16);
        if (item == 0x16) {
            wr8(MenuShowRecords, 1);
            SetMenuPage(0x09);
            ra(&r, 0);
            rnz(&r, 0x09);
            return r;
        }
        rcmp(&r, item, 0x17);
        if (item == 0x17) {
            wr8(MenuShowRecords, 0);
            SetMenuPage(0x09);
            ra(&r, 0);
            rnz(&r, 0x09);
            return r;
        }
        rcmp(&r, item, 0x13);
        if (item == 0x13) {
            slot = RD8(ZP->save_slot);
            ry(&r, slot);
            value = rd8((uint16_t)(MenuSlotCharacter + slot));
            ra(&r, value);
            if (value != 0) {
                value = rd8((uint16_t)(MenuSlotProgress + slot));
                ra(&r, value);
                if (value == 0) {
                    return sound_tail(r, 0x41, r.x);
                }
            }
            slot = RD8(ZP->save_slot);
            ry(&r, slot);
            value = rd8((uint16_t)(MenuSlotCharacter + slot));
            ra(&r, value);
            WR8(ZP->character, value);
            value = rd8((uint16_t)(MenuSlotProgress + slot));
            rx(&r, value);
            value = rd8((uint16_t)(START_STAGES + value));
            ra(&r, value);
            WR8(ZP->stage, value);
            held = rd8(BiosPadState);
            held = (uint8_t)(held | rd8(BiosPadState + 2));
            held = (uint8_t)(held & 0xFE);
            ra(&r, held);
            if (held == 0) {
                held = rd8(BiosPadState + 1);
                ra(&r, held);
                rcmp(&r, held, 0x8E);
                if (held == 0x8E) {
                    value = rd8((uint16_t)(MenuSlotProgress + slot));
                    rx(&r, value);
                    value = rd8((uint16_t)(START_STAGES_HELD + value));
                    ra(&r, value);
                    WR8(ZP->stage, value);
                }
            }
            slot = RD8(ZP->save_slot);
            ry(&r, slot);
            offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
            rx(&r, offset);
            value = rd8((uint16_t)(SaveBuffer + SLOT_INTRO_SEEN + offset));
            ra(&r, value);
            if (value == 0) {
                value = (uint8_t)(rd8((uint16_t)(SaveBuffer + SLOT_INTRO_SEEN + offset)) + 1);
                wr8((uint16_t)(SaveBuffer + SLOT_INTRO_SEEN + offset), value);
                RMW8(ZP->mode_events, events_, events_ | 0x08);
            }
            SetMenuPage(0x10);
            ra(&r, 0);
            rnz(&r, 0x10);
            return r;
        }
        rcmp(&r, item, 0x14);
        if (item == 0x14) {
            slot = RD8(ZP->save_slot);
            rx(&r, slot);
            value = (uint8_t)(rd8((uint16_t)(MenuSlotCharacter + slot)) ^ 0x01);
            ra(&r, value);
            wr8((uint16_t)(MenuSlotCharacter + slot), value);
            WR8(OBJ->start_arg[CHARACTER_OBJECT], value);
            rx(&r, 2);
            WR8(OBJ->work_d[CHARACTER_OBJECT + 1], 2);
            return r;
        }
    }
    ry(&r, 3);
    jsr_push(0x94C1);
    pad = CursorMoveByPad(3);
    jsr_pop();
    r_pad(&r, pad);
    jsr_push(0x94C4);
    jitter = JitterCursorObjects();
    jsr_pop();
    r_glide(&r, jitter, 1);
    tick = RD8(ZP->frame_tick);
    ra(&r, (uint8_t)(tick & 0x1F));
    if (r.a == 0) {
        jsr_push(0x94CD);
        value = r_random(&r, NextRandom());
        jsr_pop();
        ra(&r, (uint8_t)(value & 0x0F));
        if (r.a == 0) {
            jsr_push(0x94D4);
            value = r_random(&r, NextRandom());
            jsr_pop();
            object = radd(&r, (uint8_t)(value & 0x03), 0x2F);
            rx(&r, object);
            value = (uint8_t)(RD8(OBJ->facing_left[object]) ^ 0x01);
            ra(&r, value);
            WR8(OBJ->facing_left[object], value);
        }
    }
    jsr_push(0x94E5);
    r = back_on_button_ii(r);
    jsr_pop();
    return r;
}

void abi_FileMenuCharacterPage(void)
{
    MenuRegs r = FileMenuCharacterPage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* The name typed to open everything on a new slot, and the three default names. */
#define NAME_CHEAT 0x97EF
#define DEFAULT_NAMES 0x980D
#define SLOT_NAME 0x15                      /* a slot's eight name characters */
#define NAME_OBJECT 2                       /* the grid cursor, with object 3 */

static void r_name(MenuRegs *r, NameExit n)             /* ShowNameCursor, NameBackspace, NameTypeCharacter */
{
    ra(r, n.a);
    r->y = n.y;
    r->set |= REG_Y;
    rnz(r, n.full ? 0 : n.a);
    rc(r, n.carry);
    rv(r, n.overflow);
}

static void r_name_target(MenuRegs *r, NameCursorExit n)   /* NameCursorTarget */
{
    ra(r, 1);
    r->y = n.y;
    r->set |= REG_Y;
    rnz(r, 1);
    rc(r, n.carry);
}

/* PadRepeatUpdate: A the value it ends with, X 8, Z and C set, N clear. */
static void r_pad_repeat(MenuRegs *r, uint8_t a)
{
    ra(r, a);
    rx(r, 8);
    rnz(r, 0);
    rc(r, true);
}

/*
 * @routine 00C3:599 NameEntryStart
 * @run 6B:9599
 * WHAT IT DOES: The name page's first step: the grid cursor (objects 2 and 3) is emptied, given its picture set and put
 * at the middle of the screen, sent to its grid place, the key repeat started afresh, the name buffer filled with
 * spaces, the count cleared and the first blank tile drawn; then the step goes on.
 * HOW: ClearObject twice, six stores, NameCursorTarget, PadRepeatReset, a loop over MenuName, ShowNameCursor,
 * MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: A the new step, N and Z from it, X $FF, Y the count, C and V from ShowNameCursor.
 */
MenuRegs NameEntryStart(MenuRegs r)
{
    uint8_t i;

    rx(&r, 3);
    jsr_push(0x959E);
    ClearObject(3);
    jsr_pop();
    rx(&r, NAME_OBJECT);
    jsr_push(0x95A3);
    ClearObject(NAME_OBJECT);
    jsr_pop();
    ra(&r, 4);
    WR8(OBJ->sprite_set[NAME_OBJECT], 4);
    WR8(OBJ->sprite_set[NAME_OBJECT + 1], 4);
    ra(&r, 0x80);
    WR8(OBJ->x_lo[NAME_OBJECT], 0x80);
    WR8(OBJ->y_lo[NAME_OBJECT], 0x80);
    WR8(OBJ->x_lo[NAME_OBJECT + 1], 0x80);
    WR8(OBJ->y_lo[NAME_OBJECT + 1], 0x80);
    jsr_push(0x95BC);
    r_name_target(&r, NameCursorTarget(NAME_OBJECT));
    jsr_pop();
    jsr_push(0x95BF);
    PadRepeatReset();
    jsr_pop();
    ra(&r, 0x20);
    rx(&r, 7);
    i = 7;
    do {
        wr8((uint16_t)(MenuName + i), 0x20);
        i--;
    } while (i != 0xFF);
    rx(&r, 0xFF);
    wr8(MenuNameLength, 0);
    jsr_push(0x95CF);
    r_name(&r, ShowNameCursor());
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_NameEntryStart(void)
{
    MenuRegs r = NameEntryStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:7F7 NameUseDefault
 * @run 6B:97F7
 * WHAT IT DOES: Fills the name buffer with the selected slot's default name.
 * HOW: Slot less one times eight indexes DEFAULT_NAMES; eight bytes copied.
 * WAITS: no.
 * Entry: nothing. Exit: A the last character, X 8, Y past it, Z and C set, N clear.
 */
MenuRegs NameUseDefault(MenuRegs r)
{
    uint8_t slot, y, x, value = 0;

    slot = rd8(MenuSelectedSlot);
    y = (uint8_t)((uint8_t)(slot - 1) << 3);
    x = 0;
    do {
        value = rd8((uint16_t)(DEFAULT_NAMES + y));
        wr8((uint16_t)(MenuName + x), value);
        y++;
        x++;
    } while (x != NAME_LENGTH);
    ra(&r, value);
    rx(&r, x);
    r.y = y;
    r.set |= REG_Y;
    rcmp(&r, x, NAME_LENGTH);
    return r;
}

void abi_NameUseDefault(void)
{
    MenuRegs r = NameUseDefault(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DD:76D DrawUsedSlotRows
 * @run 6F:A76D
 * WHAT IT DOES: Draws the three slots' rows of the slot list: a slot with data gets its name; each row is coloured
 * with palette 0 or 1 by whether it has data, swapped unless the page row (MenuChosenItem) is 4.
 * HOW: zTemp01 0 or 1 from MenuChosenItem; zTemp00 counted down from 3; EventBitTest; DrawSlotName at the slot plus 2;
 * RecolorTextRun at the slot plus 6.
 * WAITS: no.
 * Entry: nothing. Exit: as RecolorTextRun leaves the last row, Z set, N clear (the count).
 */
MenuRegs DrawUsedSlotRows(MenuRegs r)
{
    uint8_t value, slot, row;

    WR8(ZP->temp01, 0);
    value = rd8(MenuChosenItem);
    ra(&r, value);
    rcmp(&r, value, 0x04);
    if (value != 0x04) {
        rnz(&r, INC8(ZP->temp01));
    }
    ra(&r, 3);
    WR8(ZP->temp00, 3);
    do {
        slot = RD8(ZP->temp00);
        ra(&r, slot);
        jsr_push(0xA781);
        r_event(&r, EventBitTest(slot));
        jsr_pop();
        if (r.nz != 0) {
            slot = RD8(ZP->temp00);
            row = radd(&r, slot, 0x02);
            jsr_push(0xA78B);
            rexit(&r, DrawSlotName(row, r.x));
            jsr_pop();
            ra_keep(&r, 0);
        } else {
            ra(&r, 1);
        }
        value = (uint8_t)(r.a ^ RD8(ZP->temp01));
        ra(&r, value);
        ry(&r, value);
        slot = RD8(ZP->temp00);
        row = radd(&r, slot, 0x06);
        jsr_push(0xA79B);
        rexit(&r, RecolorTextRun(row, value));
        jsr_pop();
        rnz(&r, DEC8(ZP->temp00));
    } while (r.nz != 0);
    return r;
}

void abi_DrawUsedSlotRows(void)
{
    MenuRegs r = DrawUsedSlotRows(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* The end of the name page ($97C1): the grid cursor and objects 0 and 1's attributes cleared, and page $0A. */
static MenuRegs name_leave(MenuRegs r)
{
    WR8(OBJ->sprite_attr[0], 0);
    WR8(OBJ->sprite_attr[1], 0);
    rx(&r, NAME_OBJECT);
    jsr_push(0x97CC);
    ClearObject(NAME_OBJECT);
    jsr_pop();
    rx(&r, NAME_OBJECT + 1);
    jsr_push(0x97D0);
    ClearObject(NAME_OBJECT + 1);
    jsr_pop();
    ra(&r, 0x0A);
    SetMenuPage(0x0A);
    ra(&r, 0);
    rnz(&r, 0x0A);
    return r;
}

/*
 * The name accepted ($9763): the default name when nothing was typed, the name put in the slot's bytes ($15 on), a
 * byte at 1 set unless the page row is 5; a name equal to the cheat code fills the slot's bytes 1 to 5 with $C5, $FF,
 * $7F, $45 and 1; the slot's event is set and the rows drawn.
 */
static MenuRegs name_accept(MenuRegs r)
{
    uint8_t value, slot, y, x;

    value = rd8(MenuNameLength);
    ra(&r, value);
    if (value == 0) {
        jsr_push(0x976B);
        r = NameUseDefault(r);
        jsr_pop();
    }
    slot = rd8(MenuSelectedSlot);
    rx(&r, slot);
    y = rd8((uint16_t)(SLOT_OFFSETS_BY_NUMBER + slot));
    ry(&r, y);
    value = rd8(MenuChosenItem);
    ra(&r, value);
    rcmp(&r, value, 0x05);
    if (value != 0x05) {
        ra(&r, 1);
        wr8((uint16_t)(SaveBuffer + 1 + y), 1);
    }
    WR8(ZP->bios_a.lo, 0);
    x = 0;
    do {
        value = rd8((uint16_t)(MenuName + x));
        wr8((uint16_t)(SaveBuffer + SLOT_NAME + y), value);
        if (value == rd8((uint16_t)(NAME_CHEAT + x))) {
            INC8(ZP->bios_a.lo);
        }
        y++;
        x++;
    } while (x != NAME_LENGTH);
    rx(&r, x);
    r.y = y;
    r.set |= REG_Y;
    rcmp(&r, x, NAME_LENGTH);
    value = RD8(ZP->bios_a.lo);
    ra(&r, value);
    rcmp(&r, value, NAME_LENGTH);
    if (value == NAME_LENGTH) {
        slot = rd8(MenuSelectedSlot);
        rx(&r, slot);
        y = rd8((uint16_t)(SLOT_OFFSETS_BY_NUMBER + slot));
        ry(&r, y);
        wr8((uint16_t)(SaveBuffer + 1 + y), 0xC5);
        wr8((uint16_t)(SaveBuffer + 2 + y), 0xFF);
        wr8((uint16_t)(SaveBuffer + 3 + y), 0x7F);
        wr8((uint16_t)(SaveBuffer + 4 + y), 0x45);
        ra(&r, 1);
        wr8((uint16_t)(SaveBuffer + 5 + y), 1);
    }
    value = rd8(MenuSelectedSlot);
    ra(&r, value);
    jsr_push(0x97BE);
    r_event(&r, EventBitSet(value));
    jsr_pop();
    jsr_push(0x97C1);
    r = DrawUsedSlotRows(r);
    jsr_pop();
    return name_leave(r);
}

/*
 * @routine 00C3:6A3 NameEntryType
 * @run 6B:96A3
 * WHAT IT DOES: The name page's typing step: the pad with repeat moves the grid cursor (left and right by column,
 * except on the last row; up and down by row, five rows), the cursor objects glide and shake. Button I types the
 * character (sound $3B; a full name moves the cursor to the last row, keeping the place it came from, and goes on a
 * step), or on the last row accepts the name; button II erases the last character, or with an empty name leaves.
 * HOW: PadRepeatUpdate; MenuPadRepeatBits shifted out a bit at a time with PHA and PLA; NameCursorTarget on any
 * direction; GlideCursorObjects, JitterCursorObjects; NameTypeCharacter, NameBackspace, the ends at $9763 and $97C1.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as the arm that ran last leaves them.
 */
MenuRegs NameEntryType(MenuRegs r)
{
    CommandCall sound;
    MenuReplay *p;
    uint8_t value, a, row, column, trigger;
    bool carry;

    jsr_push(0x96A6);
    r_pad_repeat(&r, PadRepeatUpdate());
    jsr_pop();
    rx(&r, NAME_OBJECT);
    a = (uint8_t)(rd8(MenuPadRepeatBits) & 0xF0);
    ra(&r, a);
    if (a != 0) {
        carry = (a & 0x80) != 0;
        a = (uint8_t)(a << 1);
        ra(&r, a);
        rc(&r, carry);
        if (carry) {
            push8(a);
            row = RD8(OBJ->work_h[NAME_OBJECT]);
            rcmp(&r, row, 4);
            if (row != 4) {
                column = (uint8_t)(RD8(OBJ->work_d[NAME_OBJECT]) - 1);
                WR8(OBJ->work_d[NAME_OBJECT], (uint8_t)(column & 7));
            }
            a = pull8();
            ra(&r, a);
        }
        carry = (a & 0x80) != 0;
        a = (uint8_t)(a << 1);
        ra(&r, a);
        rc(&r, carry);
        if (carry) {
            push8(a);
            row = (uint8_t)(RD8(OBJ->work_h[NAME_OBJECT]) + 1);
            rcmp(&r, row, 5);
            if (row == 5) {
                row = 0;
            }
            WR8(OBJ->work_h[NAME_OBJECT], row);
            a = pull8();
            ra(&r, a);
        }
        carry = (a & 0x80) != 0;
        a = (uint8_t)(a << 1);
        ra(&r, a);
        rc(&r, carry);
        if (carry) {
            push8(a);
            row = RD8(OBJ->work_h[NAME_OBJECT]);
            rcmp(&r, row, 4);
            if (row != 4) {
                column = (uint8_t)(RD8(OBJ->work_d[NAME_OBJECT]) + 1);
                WR8(OBJ->work_d[NAME_OBJECT], (uint8_t)(column & 7));
            }
            a = pull8();
            ra(&r, a);
        }
        carry = (a & 0x80) != 0;
        a = (uint8_t)(a << 1);
        ra(&r, a);
        rc(&r, carry);
        if (carry) {
            row = (uint8_t)(RD8(OBJ->work_h[NAME_OBJECT]) - 1);
            ra(&r, row);
            if (row & 0x80) {
                row = 4;
                ra(&r, row);
            }
            WR8(OBJ->work_h[NAME_OBJECT], row);
        }
        jsr_push(0x96FB);
        r_name_target(&r, NameCursorTarget(NAME_OBJECT));
        jsr_pop();
    }
    jsr_push(0x96FE);
    r_glide(&r, GlideCursorObjects(), 3);
    jsr_pop();
    jsr_push(0x9701);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x01));
    if (r.a != 0) {
        row = RD8(OBJ->work_h[NAME_OBJECT]);
        ra(&r, row);
        rcmp(&r, row, 4);
        if (row == 4) {
            return name_accept(r);
        }
        jsr_push(0x9712);
        r_name(&r, NameTypeCharacter());
        jsr_pop();
        ra(&r, 0x3B);
        jsr_push(0x9717);
        sound = SoundCommand(0x3B, r.x, menu_y(&r));
        jsr_pop();
        p = menu_replay(&r, REPLAY_SOUND);
        p->command = 0x3B;
        p->call.sound = sound;
        value = rd8(MenuNameLength);
        ra(&r, value);
        rcmp(&r, value, NAME_LENGTH);
        if (value != NAME_LENGTH) {
            return r;
        }
        rx(&r, NAME_OBJECT);
        value = RD8(OBJ->work_d[NAME_OBJECT]);
        wr8(MenuCopyFrom, value);
        value = RD8(OBJ->work_h[NAME_OBJECT]);
        ra(&r, value);
        wr8(MenuCopyTo, value);
        ra(&r, 4);
        WR8(OBJ->work_h[NAME_OBJECT], 4);
        jsr_push(0x9734);
        r_name_target(&r, NameCursorTarget(NAME_OBJECT));
        jsr_pop();
        ra(&r, MenuStepAdvance());
        return r;
    }
    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x02));
    if (r.a == 0) {
        return r;
    }
    row = RD8(OBJ->work_h[NAME_OBJECT]);
    ra(&r, row);
    rcmp(&r, row, 4);
    if (row == 4) {
        return r;
    }
    value = rd8(MenuNameLength);
    ra(&r, value);
    if (value == 0) {
        return name_leave(r);
    }
    jsr_push(0x974D);
    r_name(&r, NameBackspace());
    jsr_pop();
    return r;
}

void abi_NameEntryType(void)
{
    MenuRegs r = NameEntryType(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:74E NameEntryEnd
 * @run 6B:974E
 * WHAT IT DOES: The name page's last step, with the cursor on the last row: button I accepts the name; button II goes
 * back to typing: the cursor returns to the grid place it came from, the last character is dropped and the step goes
 * back one.
 * HOW: PadRepeatUpdate, GlideCursorObjects, JitterCursorObjects, the trigger's bits 0 and 1 (LSR gives I); the end at
 * $9763, or MenuCopyFrom and MenuCopyTo restored, NameCursorTarget, MenuNameLength counted down, ShowNameCursor, zField80
 * counted down.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as the arm that ran last leaves them.
 */
MenuRegs NameEntryEnd(MenuRegs r)
{
    uint8_t trigger, value;

    jsr_push(0x9751);
    r_pad_repeat(&r, PadRepeatUpdate());
    jsr_pop();
    rx(&r, NAME_OBJECT);
    jsr_push(0x9756);
    r_glide(&r, GlideCursorObjects(), 3);
    jsr_pop();
    jsr_push(0x9759);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x03));
    if (r.a == 0) {
        return r;
    }
    rc(&r, (r.a & 0x01) != 0);
    ra(&r, (uint8_t)(r.a >> 1));
    if (r.c) {
        return name_accept(r);
    }
    rx(&r, NAME_OBJECT);
    value = rd8(MenuCopyFrom);
    WR8(OBJ->work_d[NAME_OBJECT], value);
    value = rd8(MenuCopyTo);
    ra(&r, value);
    WR8(OBJ->work_h[NAME_OBJECT], value);
    jsr_push(0x97E6);
    r_name_target(&r, NameCursorTarget(NAME_OBJECT));
    jsr_pop();
    rnz(&r, (uint8_t)(rd8(MenuNameLength) - 1));
    wr8(MenuNameLength, r.nz);
    jsr_push(0x97EC);
    r_name(&r, ShowNameCursor());
    jsr_pop();
    rnz(&r, DEC8(ZP->field80));
    return r;
}

void abi_NameEntryEnd(void)
{
    MenuRegs r = NameEntryEnd(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * The slot's record pages. A slot's bytes: $01 to $05 flags, $1F to $21 its total score (BCD), $22 and $23 the bits of
 * the entries bought with score (FileMenuShopStep).
 */
#define RECORD_ENTRIES 14
#define MARK_TEXTS 0xAA7B                   /* a string pointer for each value of MenuSlotMusic */
#define RECORD_SCORES 0xAD01                /* a two-byte BCD price for each entry, and one more at index 14 */
#define ENTRY_OFFSETS 0xB90A                /* the flag byte pair of each entry, from the slot's byte $01 */
#define ENTRY_MASKS 0xB917                  /* its bit there */
#define BOUGHT_MASKS 0xB993                /* the entry's bought bit in the slot's byte $22 (entries 0 to 7) or $23 */
#define ENTRY_PLACES 0xBA2F                 /* the VRAM address of each entry's name */
#define ENTRY_NAMES 0xBA4B                  /* a string pointer for each entry */
#define UNKNOWN_NAME 0xBA67                 /* the string of an entry not reached */

static void r_arith(MenuRegs *r, ArithExit e)           /* DrawSelectedFileDigit, DrawCursorRowDigit */
{
    ra(r, e.a);
    rnz(r, e.nz);
    rc(r, e.carry);
    if (e.overflow_set) {
        rv(r, e.overflow);
    }
}

/* SetCursorValue: A and X 1, Y the table index, C. */
static void r_cursor(MenuRegs *r, CursorTarget t)
{
    ra(r, 1);
    r->x = 1;
    r->y = t.index;
    r->set |= REG_X | REG_Y;
    rnz(r, 1);
    rc(r, t.carry);
}

static void r_select_write(MenuRegs *r)                 /* SelectVramWriteAddress: A 0, flags kept */
{
    ra_keep(r, 0);
}

static void r_select_data(MenuRegs *r)                  /* SelectVramData: A 2, N and Z clear */
{
    ra(r, 2);
}

/* WriteHexByteTiles: A the low digit's tile, N and Z clear, C bit 3 of the byte. */
static void r_hex(MenuRegs *r, uint8_t byte)
{
    ra(r, (uint8_t)((byte & 0x0F) | 0x10));
    rc(r, (byte & 0x08) != 0);
}

/*
 * @routine 00C4:0F1 DrawChosenItemDigit
 * @run 6B:98F1
 * WHAT IT DOES: Draws MenuChosenItem as a digit tile at the screen place $01C7.
 * HOW: Sets the VDC's write address and runs into DrawCursorRowDigit's second ST ($98E7, the high byte $01).
 * WAITS: no.
 * Entry: nothing. Exit: as DrawSelectedFileDigit.
 */
ArithExit DrawChosenItemDigit(void)
{
    ArithExit exit = { 0 };
    uint8_t value, sum;

    jsr_push(0x98F4);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0xC7);
    st2(0x01);
    jsr_push(0x98EC);
    SelectVramData();
    jsr_pop();
    value = rd8(MenuChosenItem);
    sum = (uint8_t)(value + 0x10);
    WR8(VDC->data_lo, sum);
    st2(0x11);
    exit.a = sum;
    exit.nz = sum;
    exit.carry = value >= 0xF0;
    exit.overflow = ((value ^ sum) & (0x10 ^ sum) & 0x80) != 0;
    exit.overflow_set = true;
    return exit;
}

void abi_DrawChosenItemDigit(void)
{
    arith_exit_shim(DrawChosenItemDigit());
}

/*
 * @routine 00DE:11D DrawSlotMark
 * @run 6F:A91D
 * WHAT IT DOES: Draws the selected slot's MenuSlotMusic plus one as a BCD number at $02C4, and the mark's string.
 * HOW: The mark plus one, adjusted past 9 by subtracting $FA with the carry set; WriteHexByteTiles; the string pointer
 * from MARK_TEXTS in zTemp00 and zTemp01; DrawGlyphText.
 * WAITS: no.
 * Entry: nothing. Exit: as DrawGlyphText leaves them.
 */
MenuRegs DrawSlotMark(MenuRegs r)
{
    uint8_t slot, mark, number, index;

    jsr_push(0xA920);
    SelectVramWriteAddress();
    jsr_pop();
    r_select_write(&r);
    st1(0xC4);
    st2(0x02);
    jsr_push(0xA927);
    SelectVramData();
    jsr_pop();
    r_select_data(&r);
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    mark = rd8((uint16_t)(MenuSlotMusic + slot));
    number = (uint8_t)(mark + 1);
    ra(&r, number);
    rcmp(&r, number, 0x0A);
    if (number >= 0x0A) {
        number = rsub(&r, number, 0xFA);
    }
    jsr_push(0xA935);
    WriteHexByteTiles(number);
    jsr_pop();
    r_hex(&r, number);
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    mark = rd8((uint16_t)(MenuSlotMusic + slot));
    index = (uint8_t)(mark << 1);
    rc(&r, (mark & 0x80) != 0);
    rx(&r, index);
    WR8(ZP->temp00, rd8((uint16_t)(MARK_TEXTS + index)));
    ra(&r, rd8((uint16_t)(MARK_TEXTS + 1 + index)));
    WR8(ZP->temp01, r.a);
    jsr_push(0xA949);
    rexit(&r, DrawGlyphText());
    jsr_pop();
    return r;
}

void abi_DrawSlotMark(void)
{
    MenuRegs r = DrawSlotMark(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DE:14A DrawSlotScore
 * @run 6F:A94A
 * WHAT IT DOES: Draws the selected slot's total score, six digits, at $01D4.
 * HOW: Sets the VDC's write address and runs into DrawSlotDetails at $A95C: the score's three bytes, high first,
 * through WriteHexByteTiles, the last as a tail.
 * WAITS: no.
 * Entry: nothing. Exit: A the last digit tile, N and Z clear, C bit 3 of the low score byte, X the slot's offset, Y the
 * slot.
 */
MenuRegs DrawSlotScore(MenuRegs r)
{
    uint8_t slot, offset, value;

    jsr_push(0xA94D);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0xD4);
    st2(0x01);
    jsr_push(0xA95F);
    SelectVramData();
    jsr_pop();
    r_select_data(&r);
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    rx(&r, offset);
    value = rd8((uint16_t)(SaveScoreTotal + 2 + offset));
    jsr_push(0xA969);
    WriteHexByteTiles(value);
    jsr_pop();
    value = rd8((uint16_t)(SaveScoreTotal + 1 + offset));
    jsr_push(0xA96E);
    WriteHexByteTiles(value);
    jsr_pop();
    value = rd8((uint16_t)(SaveScoreTotal + offset));
    WriteHexByteTiles(value);
    r_hex(&r, value);
    return r;
}

void abi_DrawSlotScore(void)
{
    MenuRegs r = DrawSlotScore(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DE:463 SlotScoreReaches
 * @run 6F:AC63
 * WHAT IT DOES: Tells whether the selected slot's total score reaches the score of entry Y/2 in RECORD_SCORES.
 * HOW: PHX, PHY, SXY (X becomes the entry's byte index); the slot's offset; the score's third byte non-zero answers
 * yes; else the second and first bytes compared with the entry's high and low. PLY and PLX.
 * WAITS: no.
 * Entry: Y the entry times two. Exit: C set when reached; A the last byte read; X and Y kept, N and Z from X.
 */
MenuRegs SlotScoreReaches(MenuRegs r)
{
    uint8_t x = r.x, y = r.y, index, slot, offset, value, limit;
    bool reached;

    push8(x);
    push8(y);
    index = y;
    slot = RD8(ZP->save_slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    value = rd8((uint16_t)(SaveScoreTotal + 2 + offset));
    reached = value != 0;
    if (!reached) {
        value = rd8((uint16_t)(SaveScoreTotal + 1 + offset));
        limit = rd8((uint16_t)(RECORD_SCORES + 1 + index));
        if (value == limit) {
            value = rd8((uint16_t)(SaveScoreTotal + offset));
            reached = value >= rd8((uint16_t)(RECORD_SCORES + index));
        } else {
            reached = value > limit;
        }
    }
    y = pull8();
    x = pull8();
    ra(&r, value);
    r.x = x;
    r.y = y;
    r.set |= REG_X | REG_Y;
    rnz(&r, x);
    rc(&r, reached);
    return r;
}

void abi_SlotScoreReaches(void)
{
    MenuRegs r = SlotScoreReaches(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* Two digit tiles of a byte with palette 6 ($ACEF): the high one, then the low one. */
static void record_digit(uint8_t value)
{
    WR8(VDC->data_lo, (uint8_t)(value | 0x10));
    st2(0x61);
}

/*
 * @routine 00DE:4EF WriteDimHexByte
 * @run 6F:ACEF
 * WHAT IT DOES: Writes a byte as two digit tiles with palette 6.
 * HOW: PHA; the high nibble through the second half by BSR; PLA; the low nibble falls into it.
 * WAITS: no.
 * Entry: A the byte, the VDC's write address set. Exit: A the low digit's tile, N and Z clear, C bit 3 of the byte;
 * X and Y kept.
 */
void WriteDimHexByte(uint8_t byte)
{
    push8(byte);
    jsr_push(0xACF6);
    record_digit((uint8_t)(byte >> 4));
    jsr_pop();
    byte = pull8();
    record_digit((uint8_t)(byte & 0x0F));
}

void abi_WriteDimHexByte(void)
{
    uint8_t byte = cpu.a;

    WriteDimHexByte(byte);
    cpu.a = (uint8_t)((byte & 0x0F) | 0x10);
    abi_nz(cpu.a);
    abi_flag(P_C, (byte & 0x08) != 0);
}

/* The three bytes of a record line drawn by WriteHexByteTiles ($ACCA): "00" and the entry's score. */
static MenuRegs record_line_lit(MenuRegs r)
{
    uint8_t value, index = r.y;

    ra_keep(&r, 0);
    jsr_push(0xACCE);
    WriteHexByteTiles(0);
    jsr_pop();
    value = rd8((uint16_t)(RECORD_SCORES + 1 + index));
    jsr_push(0xACD4);
    WriteHexByteTiles(value);
    jsr_pop();
    value = rd8((uint16_t)(RECORD_SCORES + index));
    WriteHexByteTiles(value);
    r_hex(&r, value);
    return r;
}

/*
 * @routine 00DE:48D DrawSlotRecord
 * @run 6F:AC8D
 * WHAT IT DOES: Draws the score line at $0556 for the entry the cursor is on: the object's page (OBJ->work_h) plus 7
 * when its line (OBJ->work_d) is not 0, or 14 for page 7. An entry the slot has cleared (its bit in bytes $22 and
 * $23) shows "00" and the score at index 14; else the entry's score, bright when the slot's total reaches it and with
 * palette 6 when it does not.
 * HOW: The entry number in zTemp (bios_b low); the slot's bought bit tested with BOUGHT_MASKS; PHX, PLX around it;
 * SlotScoreReaches; WriteHexByteTiles or WriteDimHexByte.
 * WAITS: no.
 * Entry: X the cursor object. Exit: A the last digit tile, N and Z clear, C bit 3 of the last byte, X kept, Y the
 * entry's byte index, V from the page add when it was made.
 */
MenuRegs DrawSlotRecord(MenuRegs r)
{
    uint8_t object = r.x, entry, line, slot, offset, bits, index;

    jsr_push(0xAC90);
    SelectVramWriteAddress();
    jsr_pop();
    st1(0x56);
    st2(0x05);
    jsr_push(0xAC97);
    SelectVramData();
    jsr_pop();
    r_select_data(&r);
    entry = RD8(OBJ->work_h[object]);
    ra(&r, entry);
    rcmp(&r, entry, 0x07);
    if (entry == 0x07) {
        entry = 0x0E;
        ra(&r, entry);
    } else {
        line = RD8(OBJ->work_d[object]);
        ry(&r, line);
        if (line != 0) {
            rc(&r, false);
            entry = radd(&r, entry, 0x07);
        }
    }
    WR8(ZP->bios_b.lo, entry);
    push8(object);
    rx(&r, entry);
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    ry(&r, offset);
    rcmp(&r, entry, 0x08);
    if (entry >= 0x08) {
        offset++;
        ry(&r, offset);
    }
    bits = (uint8_t)(rd8((uint16_t)(SaveBuffer + 0x22 + offset)) & rd8((uint16_t)(BOUGHT_MASKS + entry)));
    ra(&r, bits);
    if (bits != 0) {
        rx(&r, pull8());
        ry(&r, 0x1C);
        return record_line_lit(r);
    }
    rx(&r, pull8());
    index = RD8(ZP->bios_b.lo);
    rc(&r, (index & 0x80) != 0);
    index = (uint8_t)(index << 1);
    ra(&r, index);
    ry(&r, index);
    jsr_push(0xACC8);
    r = SlotScoreReaches(r);
    jsr_pop();
    if (r.c) {
        return record_line_lit(r);
    }
    ra_keep(&r, 0);
    jsr_push(0xACE3);
    WriteDimHexByte(0);
    jsr_pop();
    bits = rd8((uint16_t)(RECORD_SCORES + 1 + index));
    jsr_push(0xACE9);
    WriteDimHexByte(bits);
    jsr_pop();
    bits = rd8((uint16_t)(RECORD_SCORES + index));
    WriteDimHexByte(bits);
    r_hex(&r, bits);
    return r;
}

void abi_DrawSlotRecord(void)
{
    MenuRegs r = DrawSlotRecord(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00E0:0EB SlotEntryTest
 * @run 6F:B8EB
 * WHAT IT DOES: Tells whether the selected slot has reached entry X of the record list: entry 13 is bit 0 of the
 * slot's first byte; the others are a bit of a pair of its flag bytes.
 * HOW: The slot's offset; ENTRY_OFFSETS added, the byte at $01 ORed with the one at $06, ANDed with ENTRY_MASKS.
 * WAITS: no.
 * Entry: X the entry. Exit: A the bit or 0, N and Z from it; Y the offset used; X kept; C from the compare with 13, or
 * C and V from the offset's add.
 */
MenuRegs SlotEntryTest(MenuRegs r)
{
    uint8_t entry = r.x, slot, offset, value;

    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    ra(&r, offset);
    rcmp(&r, entry, 0x0D);
    if (entry == 0x0D) {
        ry(&r, offset);
        value = (uint8_t)(rd8((uint16_t)(SaveBuffer + offset)) & 0x01);
        ra(&r, value);
        return r;
    }
    rc(&r, false);
    offset = radd(&r, offset, rd8((uint16_t)(ENTRY_OFFSETS + entry)));
    ry(&r, offset);
    value = rd8((uint16_t)(SaveBuffer + 1 + offset));
    value = (uint8_t)(value | rd8((uint16_t)(SaveBuffer + 6 + offset)));
    value = (uint8_t)(value & rd8((uint16_t)(ENTRY_MASKS + entry)));
    ra(&r, value);
    return r;
}

void abi_SlotEntryTest(void)
{
    MenuRegs r = SlotEntryTest(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* The end DrawEntryName and DrawUnknownName share ($BA20): twelve tiles from the string at zTemp00, each with X as its
 * high byte. */
static MenuRegs draw_entry_tiles(MenuRegs r)
{
    uint8_t y, tile = 0;

    for (y = 0; y != 0x0C; y++) {
        tile = RD8_PTR_Y(ZP->temp00, y);
        WR8(VDC->data_lo, tile);
        WR8(VDC->data_hi, r.x);
    }
    ra(&r, tile);
    r.y = y;
    r.set |= REG_Y;
    rcmp(&r, y, 0x0C);
    return r;
}

/* The VDC's write address for entry index in ENTRY_PLACES; Y the index. */
static MenuRegs select_entry_place(MenuRegs r, uint8_t index, uint16_t ret_select, uint16_t ret_data)
{
    ry(&r, index);
    jsr_push(ret_select);
    SelectVramWriteAddress();
    jsr_pop();
    WR8(VDC->data_lo, rd8((uint16_t)(ENTRY_PLACES + index)));
    ra(&r, rd8((uint16_t)(ENTRY_PLACES + 1 + index)));
    WR8(VDC->data_hi, r.a);
    jsr_push(ret_data);
    SelectVramData();
    jsr_pop();
    r_select_data(&r);
    return r;
}

/*
 * @routine 00E0:1E2 DrawUnknownName
 * @run 6F:B9E2
 * WHAT IT DOES: Draws the string for an entry not reached at entry A's place.
 * HOW: The place from ENTRY_PLACES, the string pointer from UNKNOWN_NAME, then the twelve tiles at $BA20.
 * WAITS: no.
 * Entry: A the entry, X the tiles' high byte. Exit: A the last tile, Y $0C, Z and C set, N clear; X kept.
 */
MenuRegs DrawUnknownName(MenuRegs r)
{
    uint8_t entry = r.a;

    rc(&r, (entry & 0x80) != 0);
    r = select_entry_place(r, (uint8_t)(entry << 1), 0xB9E7, 0xB9F6);
    WR8(ZP->temp00, rd8(UNKNOWN_NAME));
    ra(&r, rd8(UNKNOWN_NAME + 1));
    WR8(ZP->temp01, r.a);
    return draw_entry_tiles(r);
}

void abi_DrawUnknownName(void)
{
    MenuRegs r = DrawUnknownName(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00E0:202 DrawEntryName
 * @run 6F:BA02
 * WHAT IT DOES: Draws entry A's name at its place.
 * HOW: As DrawUnknownName, the string pointer from ENTRY_NAMES by the entry.
 * WAITS: no.
 * Entry, exit: as DrawUnknownName.
 */
MenuRegs DrawEntryName(MenuRegs r)
{
    uint8_t entry = r.a, index;

    rc(&r, (entry & 0x80) != 0);
    index = (uint8_t)(entry << 1);
    r = select_entry_place(r, index, 0xBA07, 0xBA16);
    WR8(ZP->temp00, rd8((uint16_t)(ENTRY_NAMES + index)));
    ra(&r, rd8((uint16_t)(ENTRY_NAMES + 1 + index)));
    WR8(ZP->temp01, r.a);
    return draw_entry_tiles(r);
}

void abi_DrawEntryName(void)
{
    MenuRegs r = DrawEntryName(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00E0:1A1 DrawRecordList
 * @run 6F:B9A1
 * WHAT IT DOES: Draws the selected slot's list of 14 entries and the score line: an entry not reached shows the unknown
 * string (attribute $41), entry 0 its name with attribute $11, a reached entry its name with $51 when the slot's
 * bought bit for it is set and $41 when not. Then the score line (DrawSlotRecord) with the X the last entry left.
 * HOW: The entry counted in the card's zero page word A (bios_a low); SlotEntryTest; BOUGHT_MASKS against bytes $22
 * and $23; DrawUnknownName or DrawEntryName; a tail jump to DrawSlotRecord.
 * WAITS: no.
 * Entry: nothing. Exit: as DrawSlotRecord, X the attribute of entry 13.
 */
MenuRegs DrawRecordList(MenuRegs r)
{
    uint8_t entry, slot, offset, bits;

    WR8(ZP->bios_a.lo, 0);
    do {
        entry = RD8(ZP->bios_a.lo);
        rx(&r, entry);
        jsr_push(0xB9A8);
        r = SlotEntryTest(r);
        jsr_pop();
        if (r.nz == 0) {
            rx(&r, 0x41);
            entry = RD8(ZP->bios_a.lo);
            ra(&r, entry);
            jsr_push(0xB9D1);
            r = DrawUnknownName(r);
            jsr_pop();
        } else {
            entry = RD8(ZP->bios_a.lo);
            ra(&r, entry);
            if (entry == 0) {
                rx(&r, 0x11);
            } else {
                slot = RD8(ZP->save_slot);
                ry(&r, slot);
                offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
                ry(&r, offset);
                rcmp(&r, r.x, 0x08);
                if (r.x >= 0x08) {
                    offset++;
                    ry(&r, offset);
                }
                bits = (uint8_t)(rd8((uint16_t)(SaveBuffer + 0x22 + offset)) & rd8((uint16_t)(BOUGHT_MASKS + r.x)));
                ra(&r, bits);
                rx(&r, bits != 0 ? 0x51 : 0x41);
                entry = RD8(ZP->bios_a.lo);
                ra(&r, entry);
            }
            jsr_push(0xB9D7);
            r = DrawEntryName(r);
            jsr_pop();
        }
        rnz(&r, INC8(ZP->bios_a.lo));
        entry = RD8(ZP->bios_a.lo);
        ra(&r, entry);
        rcmp(&r, entry, RECORD_ENTRIES);
    } while (entry != RECORD_ENTRIES);
    return DrawSlotRecord(r);
}

void abi_DrawRecordList(void)
{
    MenuRegs r = DrawRecordList(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:065 FileMenuListStart
 * @run 6B:9865
 * WHAT IT DOES: Page 9, step 0: starts the list for the chosen item (list $11 for items 4 and up, else $1A plus the
 * page's first mark flag), keeps the scroll's high byte plus one in SCROLL_TARGET, puts the cursor on item 0 and goes
 * on a step.
 * HOW: StartListChannel0Kind2; SCROLL x_hi plus one; SetCursorValue; JitterCursorObjects; MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuListStart(MenuRegs r)
{
    uint8_t chosen, list, scroll;

    chosen = rd8(MenuChosenItem);
    ra(&r, chosen);
    rcmp(&r, chosen, 0x04);
    if (chosen < 0x04) {
        rc(&r, false);
        list = radd(&r, 0x1A, rd8(MenuShowRecords));
    } else {
        list = 0x11;
        ra(&r, list);
    }
    jsr_push(0x9879);
    (void)StartListChannel0Kind2(list);
    jsr_pop();
    ra(&r, 2);
    rx(&r, 0);
    rnz(&r, 2);
    scroll = (uint8_t)(RD8(SCROLL->x_hi[0]) + 1);
    ra(&r, scroll);
    wr8(SCROLL_TARGET, scroll);
    ra_keep(&r, 0);
    jsr_push(0x9884);
    r_cursor(&r, SetCursorValue(0));
    jsr_pop();
    jsr_push(0x9887);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuListStart(void)
{
    MenuRegs r = FileMenuListStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:08A FileMenuListDraw
 * @run 6B:988A
 * WHAT IT DOES: Page 9, step 1: draws what the chosen item shows: items 4 and up the selected slot's number (object
 * list 4); else the slot's mark (list 6), or, with the first mark flag set, the item number, the slot's name, its
 * score and its record list (list 7). Then the scroll's step is cleared and the step goes on.
 * HOW: DrawSelectedFileDigit, DrawSlotMark, or DrawChosenItemDigit, DrawSlotName at the slot plus 12, DrawSlotScore,
 * the cursor object's line and page zeroed and DrawRecordList; PlaceObjectList; two stores to the scroll step;
 * JitterCursorObjects; MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuListDraw(MenuRegs r)
{
    uint8_t value, row, list;

    value = rd8(MenuChosenItem);
    ra(&r, value);
    rcmp(&r, value, 0x04);
    if (value >= 0x04) {
        jsr_push(0x9894);
        r_arith(&r, DrawSelectedFileDigit());
        jsr_pop();
        list = 4;
    } else {
        value = rd8(MenuShowRecords);
        ra(&r, value);
        if (value == 0) {
            jsr_push(0x98A0);
            r = DrawSlotMark(r);
            jsr_pop();
            list = 6;
        } else {
            jsr_push(0x98A7);
            r_arith(&r, DrawChosenItemDigit());
            jsr_pop();
            value = RD8(ZP->save_slot);
            ra(&r, value);
            rc(&r, false);
            row = radd(&r, value, 0x0C);
            jsr_push(0x98AF);
            rexit(&r, DrawSlotName(row, r.x));
            jsr_pop();
            jsr_push(0x98B2);
            r = DrawSlotScore(r);
            jsr_pop();
            WR8(OBJ->work_d[0], 0);
            WR8(OBJ->work_h[0], 0);
            jsr_push(0x98BB);
            r = DrawRecordList(r);
            jsr_pop();
            list = 7;
        }
    }
    ra(&r, list);
    jsr_push(0x98C0);
    rexit(&r, PlaceObjectList(list));
    jsr_pop();
    WR8(SCROLLE->step_x_lo[0], 0);
    WR8(SCROLLE->step_x_hi[0], 0);
    jsr_push(0x98C9);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuListDraw(void)
{
    MenuRegs r = FileMenuListDraw(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:0F8 FileMenuListLeave
 * @run 6B:98F8
 * WHAT IT DOES: Page 9, step 3: where the chosen item leads: items 4 and up to page $0B (cursor on item $18, objects 0
 * and 1 given attribute $88); else page $0E (cursor on item $1C, $7DDA cleared) or, with the first mark flag set, page
 * $0F (cursor on item $1D).
 * HOW: JitterCursorObjects, SetCursorValue, the stores, SetMenuPage.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: A 0, N and Z from the page, X 1, Y and C as SetCursorValue leaves them.
 */
MenuRegs FileMenuListLeave(MenuRegs r)
{
    uint8_t value, page;

    jsr_push(0x98FB);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    value = rd8(MenuChosenItem);
    ra(&r, value);
    rcmp(&r, value, 0x04);
    if (value >= 0x04) {
        ra(&r, 0x18);
        jsr_push(0x9907);
        r_cursor(&r, SetCursorValue(0x18));
        jsr_pop();
        ra(&r, 0x88);
        WR8(OBJ->sprite_attr[0], 0x88);
        WR8(OBJ->sprite_attr[1], 0x88);
        page = 0x0B;
    } else {
        value = rd8(MenuShowRecords);
        ra(&r, value);
        if (value == 0) {
            ra(&r, 0x1C);
            jsr_push(0x991E);
            r_cursor(&r, SetCursorValue(0x1C));
            jsr_pop();
            wr8(MenuMusicPlaying, 0);
            page = 0x0E;
        } else {
            ra(&r, 0x1D);
            jsr_push(0x992B);
            r_cursor(&r, SetCursorValue(0x1D));
            jsr_pop();
            page = 0x0F;
        }
    }
    ra(&r, page);
    SetMenuPage(page);
    ra(&r, 0);
    rnz(&r, page);
    return r;
}

void abi_FileMenuListLeave(void)
{
    MenuRegs r = FileMenuListLeave(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* Tables of the slot list pages, indexed by the chosen item (1 to 7). */
#define LIST_BY_ITEM 0x9964                 /* the list a page starts, 0: by the slot's first byte */
#define KIND_BY_ITEM 0x9A30                 /* 0: a slot (items 1 to 3); 1 or 2 otherwise */
#define PICTURE_BY_CHARACTER 0x9A38         /* object $34's frame by the character */
#define CURSOR_BY_ITEM 0x9AFF               /* the cursor value a page returns to */
#define SLOT_NOT_FULL 0x7DDB                /* set when the slot chosen has no bit 2 in its byte $01 */

/*
 * @routine 00C4:13D FileMenuSlideStart
 * @run 6B:993D
 * WHAT IT DOES: The first step of the page that slides the screen on: starts the item's list (LIST_BY_ITEM; where that
 * is 0, list $13 when the slot has data and $12 when not), keeps the scroll's high byte plus one as the target, puts
 * the cursor on item 0 and goes on.
 * HOW: LIST_BY_ITEM, the slot's first byte shifted right; StartListChannel0Kind2; SetCursorValue; JitterCursorObjects;
 * MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuSlideStart(MenuRegs r)
{
    uint8_t chosen, list, offset, scroll;

    chosen = rd8(MenuChosenItem);
    ry(&r, chosen);
    list = rd8((uint16_t)(LIST_BY_ITEM + chosen));
    ra(&r, list);
    if (list == 0) {
        offset = rd8((uint16_t)(SLOT_OFFSETS_BY_NUMBER + chosen));
        rx(&r, offset);
        rc(&r, (rd8((uint16_t)(SaveBuffer + offset)) & 0x01) != 0);
        list = r.c ? 0x13 : 0x12;
        ra(&r, list);
    }
    jsr_push(0x9954);
    (void)StartListChannel0Kind2(list);
    jsr_pop();
    ra(&r, 2);
    rx(&r, 0);
    rnz(&r, 2);
    scroll = (uint8_t)(RD8(SCROLL->x_hi[0]) + 1);
    ra(&r, scroll);
    wr8(SCROLL_TARGET, scroll);
    ra_keep(&r, 0);
    jsr_push(0x995F);
    r_cursor(&r, SetCursorValue(0));
    jsr_pop();
    jsr_push(0x9962);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuSlideStart(void)
{
    MenuRegs r = FileMenuSlideStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:16C FileMenuSlideDraw
 * @run 6B:996C
 * WHAT IT DOES: Draws the next screen of the chosen item off to the side: item 7 the copy rows, items 4 to 6 their list
 * ($14 to $16) and the used slots' rows, items 1 to 3 the slot (its number made zSaveSlot): list $1C when its byte $01
 * has no bit 2, its details, name and percentage; then the item's objects (list 3 or 5 for a slot with and without
 * data, or the item's kind), and the slot's character picture and marks. The scroll's step is cleared and the step
 * goes on.
 * HOW: StartListChannel0Kind2, DrawUsedSlotRows, DrawSlotRows, DrawSlotDetails, DrawCursorRowDigit, DrawSlotName,
 * DrawSlotPercent, PlaceObjectList; the object frames of $2B and $2F to $34 and the start arguments of $33 and $34.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuSlideDraw(MenuRegs r)
{
    uint8_t chosen, list, offset, bits, slot, row, kind, character, byte;

    wr8(SLOT_NOT_FULL, 0);
    chosen = rd8(MenuChosenItem);
    ra(&r, chosen);
    rcmp(&r, chosen, 0x07);
    if (chosen == 0x07) {
        ra_keep(&r, 0);
        jsr_push(0x9989);
        r = DrawSlotRows(r, 0);
        jsr_pop();
    } else {
        rcmp(&r, chosen, 0x04);
        if (chosen >= 0x04) {
            rc(&r, false);
            list = radd(&r, chosen, 0x10);
            jsr_push(0x9980);
            (void)StartListChannel0Kind2(list);
            jsr_pop();
            ra(&r, 2);
            rx(&r, 0);
            rnz(&r, 2);
            jsr_push(0x9983);
            r = DrawUsedSlotRows(r);
            jsr_pop();
        } else {
            chosen = rd8(MenuChosenItem);
            slot = (uint8_t)(chosen - 1);
            ry(&r, slot);
            offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
            rx(&r, offset);
            bits = (uint8_t)(rd8((uint16_t)(SaveBuffer + 1 + offset)) & 0x04);
            ra(&r, bits);
            if (bits == 0) {
                ra(&r, 0x1C);
                jsr_push(0x999E);
                (void)StartListChannel0Kind2(0x1C);
                jsr_pop();
                ra(&r, 2);
                rx(&r, 0);
                rnz(&r, 2);
                byte = (uint8_t)(rd8(SLOT_NOT_FULL) + 1);
                wr8(SLOT_NOT_FULL, byte);
                rnz(&r, byte);
            }
        }
    }
    chosen = rd8(MenuChosenItem);
    ra(&r, chosen);
    rcmp(&r, chosen, 0x04);
    if (chosen < 0x04) {
        chosen = rd8(MenuChosenItem);
        slot = (uint8_t)(chosen - 1);
        ra(&r, slot);
        WR8(ZP->save_slot, slot);
        jsr_push(0x99B1);
        rexit(&r, DrawSlotDetails());
        jsr_pop();
        jsr_push(0x99B4);
        r_arith(&r, DrawCursorRowDigit());
        jsr_pop();
        slot = RD8(ZP->save_slot);
        rc(&r, false);
        row = radd(&r, slot, 0x06);
        jsr_push(0x99BC);
        rexit(&r, DrawSlotName(row, r.x));
        jsr_pop();
        slot = RD8(ZP->save_slot);
        rc(&r, false);
        row = radd(&r, slot, 0x06);
        jsr_push(0x99C4);
        rexit(&r, DrawSlotPercent(row, r.x));
        jsr_pop();
    }
    chosen = rd8(MenuChosenItem);
    ry(&r, chosen);
    kind = rd8((uint16_t)(KIND_BY_ITEM + chosen));
    ra(&r, kind);
    if (kind == 0) {
        offset = rd8((uint16_t)(SLOT_OFFSETS_BY_NUMBER + chosen));
        rx(&r, offset);
        rc(&r, (rd8((uint16_t)(SaveBuffer + offset)) & 0x01) != 0);
        kind = r.c ? 3 : 5;
        ra(&r, kind);
    }
    jsr_push(0x99DC);
    rexit(&r, PlaceObjectList(kind));
    jsr_pop();
    byte = rd8(SLOT_NOT_FULL);
    ra(&r, byte);
    if (byte != 0) {
        WR8(OBJ->frame[0x2B], 0);
    }
    chosen = rd8(MenuChosenItem);
    ry(&r, chosen);
    kind = rd8((uint16_t)(KIND_BY_ITEM + chosen));
    ra(&r, kind);
    if (kind == 0) {
        character = rd8((uint16_t)(SLOT_CHARACTER_BY_NUMBER + chosen));
        WR8(OBJ->start_arg[0x33], character);
        WR8(OBJ->work_d[0x33], character);
        rx(&r, character);
        push8(character);
        offset = rd8((uint16_t)(SLOT_OFFSETS_BY_NUMBER + chosen));
        byte = rd8((uint16_t)(SaveBuffer + offset));
        rx(&r, pull8());
        ra(&r, (uint8_t)(byte >> 1));
        rc(&r, (byte & 0x01) != 0);
        if (byte & 0x01) {
            byte = rd8((uint16_t)(PICTURE_BY_CHARACTER + r.x));
            ra(&r, byte);
            WR8(OBJ->frame[0x34], byte);
        }
        offset = rd8((uint16_t)(SLOT_OFFSETS_BY_NUMBER + chosen));
        rx(&r, offset);
        byte = rd8((uint16_t)(SaveBuffer + offset));
        rc(&r, (byte & 0x01) != 0);
        byte >>= 1;
        ra(&r, byte);
        if (!r.c) {
            WR8(OBJ->frame[0x31], 0);
        }
        rc(&r, (byte & 0x01) != 0);
        byte >>= 1;
        ra(&r, byte);
        if (!r.c) {
            WR8(OBJ->frame[0x32], 0);
        }
        rc(&r, (byte & 0x01) != 0);
        byte >>= 1;
        ra(&r, byte);
        if (!r.c) {
            WR8(OBJ->frame[0x2F], 0);
        }
        rc(&r, (byte & 0x01) != 0);
        byte >>= 1;
        ra(&r, byte);
        if (!r.c) {
            WR8(OBJ->frame[0x30], 0);
        }
    }
    WR8(SCROLLE->step_x_lo[0], 0);
    WR8(SCROLLE->step_x_hi[0], 0);
    jsr_push(0x9A2E);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuSlideDraw(void)
{
    MenuRegs r = FileMenuSlideDraw(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* ShiftObjectsByScrollChange: A and V from its last sum, X $40, Z and C set, N clear. */
static void r_shift(MenuRegs *r, ArithExit e)
{
    r_arith(r, e);
    r->x = 0x40;
    r->set |= REG_X;
}

/* The scroll's step along X grown by $40, or shrunk by $40 with the high byte counted up on a borrow too ($9A45 and
 * $9A55; the same bytes at $9B72 and $9B82). */
static void scroll_step_ramp(MenuRegs *r, bool shrink)
{
    uint8_t low, sum;

    low = RD8(SCROLLE->step_x_lo[0]);
    if (!shrink) {
        sum = (uint8_t)(low + 0x40);
        ra(r, sum);
        rc(r, low > 0xBF);
        rv(r, ((low ^ sum) & (0x40 ^ sum) & 0x80) != 0);
        WR8(SCROLLE->step_x_lo[0], sum);
        if (r->c) {
            rnz(r, INC8(SCROLLE->step_x_hi[0]));
        }
        return;
    }
    (void)rsub(r, low, 0x40);
    WR8(SCROLLE->step_x_lo[0], r->a);
    if (!r->c) {
        rnz(r, INC8(SCROLLE->step_x_hi[0]));
    }
}

/* ADC with the carry as it stands. */
static uint8_t radc(MenuRegs *r, uint8_t a, uint8_t value)
{
    unsigned sum = (unsigned)a + value + (r->c ? 1u : 0u);

    ra(r, (uint8_t)sum);
    rc(r, sum > 0xFF);
    rv(r, ((a ^ sum) & (value ^ sum) & 0x80) != 0);
    return (uint8_t)sum;
}

/*
 * @routine 00C4:23A FileMenuSlideLeft
 * @run 6B:9A3A
 * WHAT IT DOES: Slides the screen to the next page a frame: the step along X ramps by $40 a frame (up while the
 * scroll's middle byte is positive, down after), the 24-bit scroll moves by it, and at the target page the scroll's low
 * bytes are zeroed and the step goes on. The objects move with the scroll.
 * HOW: AnimateCharacterChoice, JitterCursorObjects; the ramp; SCROLL x_frac, x_lo and x_hi added (x_lo and x_hi kept
 * in the speed bytes); x_hi compared with SCROLL_TARGET; ShiftObjectsByScrollChange; MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: not there: N, Z and C from the compare with the target; there: as
 * MenuStepAdvance.
 */
MenuRegs FileMenuSlideLeft(MenuRegs r)
{
    uint8_t low, middle, high;

    jsr_push(0x9A3D);
    r = AnimateCharacterChoice(r);
    jsr_pop();
    jsr_push(0x9A40);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    middle = RD8(SCROLL->x_lo[0]);
    ra(&r, middle);
    scroll_step_ramp(&r, (middle & 0x80) != 0);
    low = RD8(SCROLL->x_frac[0]);
    rc(&r, false);
    low = radd(&r, low, RD8(SCROLLE->step_x_lo[0]));
    WR8(SCROLL->x_frac[0], low);
    middle = RD8(SCROLL->x_lo[0]);
    WR8(SCROLLE->speed_x[0], middle);
    middle = radc(&r, middle, RD8(SCROLLE->step_x_hi[0]));
    WR8(SCROLL->x_lo[0], middle);
    high = RD8(SCROLL->x_hi[0]);
    WR8(SCROLLE->speed_y[0], high);
    high = radc(&r, high, 0);
    WR8(SCROLL->x_hi[0], high);
    rcmp(&r, high, rd8(SCROLL_TARGET));
    if (r.nz == 0) {
        WR8(SCROLL->x_lo[0], 0);
        WR8(SCROLL->x_frac[0], 0);
    }
    jsr_push(0x9A92);
    r_shift(&r, ShiftObjectsByScrollChange());
    jsr_pop();
    high = RD8(SCROLL->x_hi[0]);
    ra(&r, high);
    rcmp(&r, high, rd8(SCROLL_TARGET));
    if (r.nz != 0) {
        return r;
    }
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuSlideLeft(void)
{
    MenuRegs r = FileMenuSlideLeft(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:2F0 FileMenuReturnPage
 * @run 6B:9AF0
 * WHAT IT DOES: Goes to the page an item returns to: page 7 for the slots (items 1 to 3), else the page the item less
 * one.
 * HOW: JitterCursorObjects, MenuChosenItem compared with 4, DEC, SetMenuPage.
 * WAITS: no.
 * Entry: nothing. Exit: A 0, N and Z from the page, C from the compare; the rest as JitterCursorObjects leaves them.
 */
MenuRegs FileMenuReturnPage(MenuRegs r)
{
    uint8_t chosen, page;

    jsr_push(0x9AF3);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    chosen = rd8(MenuChosenItem);
    rcmp(&r, chosen, 0x04);
    if (chosen < 0x04) {
        chosen = 8;
    }
    page = (uint8_t)(chosen - 1);
    SetMenuPage(page);
    ra(&r, 0);
    rnz(&r, page);
    return r;
}

void abi_FileMenuReturnPage(void)
{
    MenuRegs r = FileMenuReturnPage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:29E FileMenuSlideEnd
 * @run 6B:9A9E
 * WHAT IT DOES: After the slide: puts the cursor where the item's page has it (CURSOR_BY_ITEM; for value $13 as it is,
 * else moved down to the first slot with data, or for item 4 to the first empty one) and goes to the item's page.
 * HOW: AnimateCharacterChoice; the table; EventBitTest on slots 1 and 2, counted in zTemp00; SetCursorValue; runs into
 * FileMenuReturnPage.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as FileMenuReturnPage.
 */
MenuRegs FileMenuSlideEnd(MenuRegs r)
{
    uint8_t chosen, value, slot;
    bool want_empty;

    jsr_push(0x9AA1);
    r = AnimateCharacterChoice(r);
    jsr_pop();
    chosen = rd8(MenuChosenItem);
    ry(&r, chosen);
    value = rd8((uint16_t)(CURSOR_BY_ITEM + chosen));
    ra(&r, value);
    rcmp(&r, value, 0x13);
    if (value != 0x13) {
        rcmp(&r, chosen, 0x04);
        want_empty = chosen == 0x04;
        ra(&r, 1);
        WR8(ZP->temp00, 1);
        jsr_push(want_empty ? 0x9AD6 : 0x9AB6);
        r_event(&r, EventBitTest(1));
        jsr_pop();
        if ((r.nz == 0) != want_empty) {
            rnz(&r, INC8(ZP->temp00));
            slot = RD8(ZP->temp00);
            ra(&r, slot);
            jsr_push(want_empty ? 0x9ADF : 0x9ABF);
            r_event(&r, EventBitTest(slot));
            jsr_pop();
            if ((r.nz == 0) != want_empty) {
                rnz(&r, INC8(ZP->temp00));
            }
        }
        chosen = rd8(MenuChosenItem);
        ry(&r, chosen);
        value = (uint8_t)(rd8((uint16_t)(CURSOR_BY_ITEM + chosen)) - 1);
        ra(&r, value);
        rc(&r, false);
        value = radd(&r, value, RD8(ZP->temp00));
    }
    jsr_push(0x9AF0);
    r_cursor(&r, SetCursorValue(value));
    jsr_pop();
    return FileMenuReturnPage(r);
}

void abi_FileMenuSlideEnd(void)
{
    MenuRegs r = FileMenuSlideEnd(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:312 FileMenuBackStart
 * @run 6B:9B12
 * WHAT IT DOES: Leaves a slot page without sliding: list $0F, the cursor on item $17 or $16 by the first mark flag
 * (slots), or on the selected slot plus 8, and the item's page.
 * HOW: StartListChannel0Kind2; SetCursorValue; a tail jump to FileMenuReturnPage.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as FileMenuReturnPage.
 */
MenuRegs FileMenuBackStart(MenuRegs r)
{
    uint8_t chosen, flag, value;

    ra(&r, 0x0F);
    jsr_push(0x9B17);
    (void)StartListChannel0Kind2(0x0F);
    jsr_pop();
    ra(&r, 2);
    rx(&r, 0);
    rnz(&r, 2);
    chosen = rd8(MenuChosenItem);
    ra(&r, chosen);
    rcmp(&r, chosen, 0x04);
    if (chosen < 0x04) {
        flag = rd8(MenuShowRecords);
        value = flag != 0 ? 0x16 : 0x17;
        ra(&r, value);
    } else {
        value = rd8(MenuSelectedSlot);
        rc(&r, false);
        value = radd(&r, value, 0x08);
    }
    jsr_push(0x9B34);
    r_cursor(&r, SetCursorValue(value));
    jsr_pop();
    return FileMenuReturnPage(r);
}

void abi_FileMenuBackStart(void)
{
    MenuRegs r = FileMenuBackStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

#define SCROLL_TRAVEL_FRAC 0x7DFE           /* the distance slid back, 24 bits */
#define SCROLL_TRAVEL_LO 0x7DFD
#define SCROLL_TRAVEL_HI 0x7DFC

/*
 * @routine 00C4:348 FileMenuSlideBackStart
 * @run 6B:9B48
 * WHAT IT DOES: Starts the slide back: the scroll's step and the distance travelled cleared, the target the page the
 * scroll is on, the cursor on item 0, and the step goes on.
 * HOW: Five STZ, SCROLL x_hi to SCROLL_TARGET, SetCursorValue, JitterCursorObjects, MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuSlideBackStart(MenuRegs r)
{
    uint8_t high;

    WR8(SCROLLE->step_x_lo[0], 0);
    WR8(SCROLLE->step_x_hi[0], 0);
    wr8(SCROLL_TRAVEL_FRAC, 0);
    wr8(SCROLL_TRAVEL_LO, 0);
    wr8(SCROLL_TRAVEL_HI, 0);
    high = RD8(SCROLL->x_hi[0]);
    wr8(SCROLL_TARGET, high);
    ra_keep(&r, 0);
    jsr_push(0x9B61);
    r_cursor(&r, SetCursorValue(0));
    jsr_pop();
    jsr_push(0x9B64);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuSlideBackStart(void)
{
    MenuRegs r = FileMenuSlideBackStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:367 FileMenuSlideBack
 * @run 6B:9B67
 * WHAT IT DOES: Slides the screen back a page a frame: the step ramps by $40 (up while the distance's middle byte is
 * positive, down after) and adds to the 24-bit distance, cleared to a whole page once it reaches one; the scroll is the
 * target less the distance; at a whole page the step goes on. The objects move with the scroll.
 * HOW: AnimateCharacterChoice, JitterCursorObjects; the ramp; the distance; SCROLL x_lo and x_hi kept in the speed
 * bytes; the scroll as a subtraction; ShiftObjectsByScrollChange; MenuStepAdvance when the distance's high byte is set.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: not there: A 0, Z set; there: as MenuStepAdvance.
 */
MenuRegs FileMenuSlideBack(MenuRegs r)
{
    uint8_t middle, value, high;

    jsr_push(0x9B6A);
    r = AnimateCharacterChoice(r);
    jsr_pop();
    jsr_push(0x9B6D);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    middle = rd8(SCROLL_TRAVEL_LO);
    ra(&r, middle);
    scroll_step_ramp(&r, (middle & 0x80) != 0);
    value = rd8(SCROLL_TRAVEL_FRAC);
    rc(&r, false);
    value = radd(&r, value, RD8(SCROLLE->step_x_lo[0]));
    wr8(SCROLL_TRAVEL_FRAC, value);
    value = rd8(SCROLL_TRAVEL_LO);
    value = radc(&r, value, RD8(SCROLLE->step_x_hi[0]));
    wr8(SCROLL_TRAVEL_LO, value);
    value = rd8(SCROLL_TRAVEL_HI);
    value = radc(&r, value, 0);
    wr8(SCROLL_TRAVEL_HI, value);
    if (value != 0) {
        wr8(SCROLL_TRAVEL_LO, 0);
        wr8(SCROLL_TRAVEL_FRAC, 0);
    }
    middle = RD8(SCROLL->x_lo[0]);
    WR8(SCROLLE->speed_x[0], middle);
    high = RD8(SCROLL->x_hi[0]);
    ra(&r, high);
    WR8(SCROLLE->speed_y[0], high);
    ra_keep(&r, 0);
    value = rsub(&r, 0, rd8(SCROLL_TRAVEL_FRAC));
    WR8(SCROLL->x_frac[0], value);
    ra_keep(&r, 0);
    value = rd8(SCROLL_TRAVEL_LO);
    {
        uint8_t borrow = r.c ? 0 : 1;
        uint8_t diff = (uint8_t)(0 - value - borrow);

        ra(&r, diff);
        rv(&r, ((0 ^ value) & (0 ^ diff) & 0x80) != 0);
        rc(&r, 0 >= (unsigned)value + borrow);
        WR8(SCROLL->x_lo[0], diff);
        value = rd8(SCROLL_TARGET);
        high = rd8(SCROLL_TRAVEL_HI);
        borrow = r.c ? 0 : 1;
        diff = (uint8_t)(value - high - borrow);
        ra(&r, diff);
        rv(&r, ((value ^ high) & (value ^ diff) & 0x80) != 0);
        rc(&r, value >= (unsigned)high + borrow);
        WR8(SCROLL->x_hi[0], diff);
    }
    jsr_push(0x9BDA);
    r_shift(&r, ShiftObjectsByScrollChange());
    jsr_pop();
    value = rd8(SCROLL_TRAVEL_HI);
    ra(&r, value);
    if (value == 0) {
        return r;
    }
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuSlideBack(void)
{
    MenuRegs r = FileMenuSlideBack(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:3E3 FileMenuBackToStart
 * @run 6B:9BE3
 * WHAT IT DOES: After the slide back to the first page: the cursor on the item chosen, page 1 at its step 2.
 * HOW: AnimateCharacterChoice, SetCursorValue, SetMenuPage by JSR, then SetMenuStep.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: A 2, N and Z clear, X 1, Y and C as SetCursorValue leaves them.
 */
MenuRegs FileMenuBackToStart(MenuRegs r)
{
    uint8_t chosen;

    jsr_push(0x9BE6);
    r = AnimateCharacterChoice(r);
    jsr_pop();
    chosen = rd8(MenuChosenItem);
    ra(&r, chosen);
    jsr_push(0x9BEC);
    r_cursor(&r, SetCursorValue(chosen));
    jsr_pop();
    jsr_push(0x9BF1);
    SetMenuPage(1);
    jsr_pop();
    SetMenuStep(2);
    ra(&r, 2);
    return r;
}

void abi_FileMenuBackToStart(void)
{
    MenuRegs r = FileMenuBackToStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* RecolorTextRun with Y the palette and A the row; the event routines with A the event. */
static MenuRegs recolor(MenuRegs r, uint8_t row, uint8_t palette, uint16_t ret)
{
    jsr_push(ret);
    rexit(&r, RecolorTextRun(row, palette));
    jsr_pop();
    return r;
}

static MenuRegs set_event(MenuRegs r, uint8_t event, uint16_t ret)
{
    ra(&r, event);
    jsr_push(ret);
    r_event(&r, EventBitSet(event));
    jsr_pop();
    return r;
}

static MenuRegs clear_event(MenuRegs r, uint8_t event, uint16_t ret)
{
    ra(&r, event);
    jsr_push(ret);
    r_event(&r, EventBitClear(event));
    jsr_pop();
    return r;
}

/*
 * @routine 00DD:731 OfferSlotCommands
 * @run 6F:A731
 * WHAT IT DOES: Draws menu item 3 in palette 1 and makes event 4 pickable.
 * HOW: RecolorTextRun with Y 1 on row 3; EventBitSet 4 as a tail.
 * WAITS: no.
 * Entry: nothing. Exit: as EventBitSet; Y the event's byte.
 */
MenuRegs OfferSlotCommands(MenuRegs r)
{
    ry(&r, 1);
    ra(&r, 3);
    r = recolor(r, 3, 1, 0xA738);
    ra(&r, 4);
    r_event(&r, EventBitSet(4));
    return r;
}

void abi_OfferSlotCommands(void)
{
    MenuRegs r = OfferSlotCommands(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DD:73D DrawSlotMenuGreyed
 * @run 6F:A73D
 * WHAT IT DOES: DrawSlotMenu's two arms that grey commands: with three slots used ($A73D), item 3 greyed and event 4
 * cleared; with none ($A74A), OfferSlotCommands first and then items 4 and 5 greyed and events 5 and 6 cleared. Both
 * end greying item 6 and clearing event 7.
 * HOW: RecolorTextRun with palette 0, EventBitClear; the last as a tail jump.
 * WAITS: no.
 * Entry: branched to from DrawSlotMenu. Exit: as EventBitClear.
 */
static MenuRegs DrawSlotMenuGreyed(MenuRegs r, bool none_used)
{
    if (!none_used) {
        ry_keep(&r, 0);
        ra(&r, 3);
        r = recolor(r, 3, 0, 0xA743);
        r = clear_event(r, 4, 0xA748);
    } else {
        ry_keep(&r, 0);
        ra(&r, 4);
        r = recolor(r, 4, 0, 0xA752);
        r = clear_event(r, 5, 0xA757);
        ry_keep(&r, 0);
        ra(&r, 5);
        r = recolor(r, 5, 0, 0xA75D);
        r = clear_event(r, 6, 0xA762);
    }
    ry_keep(&r, 0);
    ra(&r, 6);
    r = recolor(r, 6, 0, 0xA768);
    ra(&r, 7);
    r_event(&r, EventBitClear(7));
    return r;
}

/*
 * @routine 00DD:6D0 DrawSlotMenu
 * @run 6F:A6D0
 * WHAT IT DOES: Draws the slot list and its commands: each of the three slots with data gets its event, name and
 * percentage, one without has its event cleared and its row blanked, and the row is coloured by it. Then the command
 * rows: with no slot used, only item 3 is offered and items 4 to 6 are greyed with their events cleared; with one or
 * two used, items 4, 5, 6 and 3 are offered; with all three, items 4 and 5 are offered and 3 is greyed.
 * HOW: bios_b low counts the slots with data (bit 0 of each slot's byte $01); per slot EventBitSet or EventBitClear,
 * DrawSlotName or BlankTextRow, DrawSlotPercent, RecolorTextRun; then the commands.
 * WAITS: no.
 * Entry: nothing. Exit: as the last EventBitSet or EventBitClear.
 */
MenuRegs DrawSlotMenu(MenuRegs r)
{
    uint8_t x, offset, used, event, count;

    WR8(ZP->bios_b.lo, 0);
    x = 0;
    rx(&r, 0);
    do {
        offset = rd8((uint16_t)(SLOT_OFFSETS + x));
        ry(&r, offset);
        used = (uint8_t)(rd8((uint16_t)(SaveBuffer + 1 + offset)) & 0x01);
        ra(&r, used);
        ry(&r, used);
        push8(used);
        event = (uint8_t)(x + 1);
        if (used == 0) {
            ra(&r, x);
            r = clear_event(r, event, 0xA6E4);
            ra(&r, x);
            jsr_push(0xA6E8);
            rexit(&r, BlankTextRow(x, x));
            jsr_pop();
        } else {
            ra(&r, x);
            r = set_event(r, event, 0xA6EF);
            ra(&r, x);
            jsr_push(0xA6F3);
            rexit(&r, DrawSlotName(x, x));
            jsr_pop();
            rnz(&r, INC8(ZP->bios_b.lo));
        }
        ra(&r, x);
        jsr_push(0xA6F9);
        rexit(&r, DrawSlotPercent(x, x));
        jsr_pop();
        used = pull8();
        ry(&r, used);
        ra(&r, x);
        r = recolor(r, x, used, 0xA6FE);
        x++;
        rx(&r, x);
        rcmp(&r, x, 0x03);
    } while (x != 3);
    count = RD8(ZP->bios_b.lo);
    ra(&r, count);
    if (count != 0) {
        ry(&r, 1);
        ra(&r, 4);
        r = recolor(r, 4, 1, 0xA70E);
        r = set_event(r, 5, 0xA713);
        ry(&r, 1);
        ra(&r, 5);
        r = recolor(r, 5, 1, 0xA71A);
        r = set_event(r, 6, 0xA71F);
        count = RD8(ZP->bios_b.lo);
        ra(&r, count);
        rcmp(&r, count, 0x03);
        if (count != 3) {
            ry(&r, 1);
            ra(&r, 6);
            r = recolor(r, 6, 1, 0xA72C);
            r = set_event(r, 7, 0xA731);
            return OfferSlotCommands(r);
        }
        return DrawSlotMenuGreyed(r, false);
    }
    jsr_push(0xA74C);
    r = OfferSlotCommands(r);
    jsr_pop();
    return DrawSlotMenuGreyed(r, true);
}

void abi_DrawSlotMenu(void)
{
    MenuRegs r = DrawSlotMenu(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:42F FileMenuOpenStart
 * @run 6B:9C2F
 * WHAT IT DOES: The opening page's first step: list $0F, the objects of object list 0, the cursor objects put on item 1,
 * and the step goes on.
 * HOW: StartListChannel0Kind2, PlaceObjectList, PlaceCursorObjects, MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuOpenStart(MenuRegs r)
{
    CursorTarget place;

    ra(&r, 0x0F);
    jsr_push(0x9C34);
    (void)StartListChannel0Kind2(0x0F);
    jsr_pop();
    ra(&r, 2);
    rx(&r, 0);
    rnz(&r, 2);
    ra_keep(&r, 0);
    jsr_push(0x9C38);
    rexit(&r, PlaceObjectList(0));
    jsr_pop();
    ra(&r, 1);
    jsr_push(0x9C3D);
    place = PlaceCursorObjects(1);
    jsr_pop();
    ra(&r, place.value);
    r.x = 1;
    r.y = place.index;
    r.set |= REG_X | REG_Y;
    rc(&r, place.carry);
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuOpenStart(void)
{
    MenuRegs r = FileMenuOpenStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:440 FileMenuOpenSlots
 * @run 6B:9C40
 * WHAT IT DOES: The opening page's second step: script $0E, the slot list drawn, and the step goes on.
 * HOW: StartScriptSlot0Kind1, DrawSlotMenu, MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuOpenSlots(MenuRegs r)
{
    ra(&r, 0x0E);
    jsr_push(0x9C45);
    (void)StartScriptSlot0Kind1(0x0E);
    jsr_pop();
    ry(&r, 0x0E);
    rx(&r, 0);
    rnz(&r, 0x0E);
    jsr_push(0x9C48);
    r = DrawSlotMenu(r);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuOpenSlots(void)
{
    MenuRegs r = FileMenuOpenSlots(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:44B FileMenuOpenPick
 * @run 6B:9C4B
 * WHAT IT DOES: The opening page's pick: button I on an item whose event is set makes it MenuChosenItem and goes to
 * the next page; otherwise the cursor moves by the pad and shakes.
 * HOW: The trigger's bit 0, EventBitTest, MenuPageAdvance; or CursorMoveByPad (menu 0) and a tail jump to
 * JitterCursorObjects.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuPageAdvance, or as JitterCursorObjects.
 */
MenuRegs FileMenuOpenPick(MenuRegs r)
{
    uint8_t trigger, value, page;

    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x01));
    if (r.a != 0) {
        value = RD8(OBJ->work_d[0]);
        ra(&r, value);
        jsr_push(0x9C58);
        r_event(&r, EventBitTest(value));
        jsr_pop();
        if (r.nz != 0) {
            value = RD8(OBJ->work_d[0]);
            ra(&r, value);
            wr8(MenuChosenItem, value);
            page = MenuPageAdvance();
            ra_keep(&r, 0);
            rnz(&r, page);
            return r;
        }
    }
    ry_keep(&r, 0);
    jsr_push(0x9C67);
    r_pad(&r, CursorMoveByPad(0));
    jsr_pop();
    r_glide(&r, JitterCursorObjects(), 1);
    return r;
}

void abi_FileMenuOpenPick(void)
{
    MenuRegs r = FileMenuOpenPick(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:342 FileMenuSlidePrepare
 * @run 6B:9B42
 * WHAT IT DOES: The first step of the page that slides back to the opening page: the character objects animate and the
 * slot list is drawn; then, as FileMenuSlideBackStart, which it runs into.
 * HOW: AnimateCharacterChoice, DrawSlotMenu, the bytes of FileMenuSlideBackStart.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuSlidePrepare(MenuRegs r)
{
    jsr_push(0x9B45);
    r = AnimateCharacterChoice(r);
    jsr_pop();
    jsr_push(0x9B48);
    r = DrawSlotMenu(r);
    jsr_pop();
    return FileMenuSlideBackStart(r);
}

void abi_FileMenuSlidePrepare(void)
{
    MenuRegs r = FileMenuSlidePrepare(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:5F4 MenuStepDispatch
 * @run 6B:9DF4
 * WHAT IT DOES: Goes to the handler the table after the caller's JSR names for step A: the file menu's pages and their steps.
 * HOW: A doubled plus one in X; the return address pulled into the operand of its own JMP, which then reads the word at that address plus X.
 * WAITS: no.
 * Entry: A the step, the table's address less one on the stack. Exit: the target; A the operand's high byte, N and Z from it, X twice the step plus one, C bit 7 of the step.
 */
uint16_t MenuStepDispatch(uint8_t step, uint8_t *operand_high)
{
    uint8_t index, low, high;
    uint16_t at;

    index = (uint8_t)((uint8_t)(step << 1) + 1);
    low = pull8();
    wr8(MENU_DISPATCH_OPERAND, low);
    high = pull8();
    wr8(MENU_DISPATCH_OPERAND + 1, high);
    *operand_high = high;
    at = (uint16_t)((low | (high << 8)) + index);
    low = rd8(at);
    high = rd8((uint16_t)(at + 1));
    return (uint16_t)(low | (high << 8));
}

void abi_MenuStepDispatch(void)
{
    uint8_t step = cpu.a, high;
    uint16_t target = MenuStepDispatch(step, &high);

    cpu.a = high;
    cpu.x = (uint8_t)((uint8_t)(step << 1) + 1);
    abi_nz(cpu.a);
    abi_flag(P_C, (step & 0x80) != 0);
    run_routine(target);
}

/* A page's own jump to MenuStepDispatch: the step in A, the registers its handler is entered with. */
static uint16_t menu_page(MenuRegs *r, uint16_t table)
{
    uint8_t step, high;
    uint16_t target;

    step = RD8(ZP->field80);
    ra(r, step);
    jsr_push(table);
    target = MenuStepDispatch(step, &high);
    ra(r, high);
    r->x = (uint8_t)((uint8_t)(step << 1) + 1);
    r->set |= REG_X;
    rc(r, (step & 0x80) != 0);
    return target;
}

/* A word the page's table does not hold in C: to the C of whatever is at that address. */
static MenuRegs menu_elsewhere(MenuRegs r, uint16_t target)
{
    menu_regs_shim(&r);
    run_routine(target);
    return menu_regs_cpu();
}

/*
 * @routine 00C3:255 FileMenuReadSavePage
 * @run 6B:9255
 * @stop 925A
 * WHAT IT DOES: Page 0: the save read, then the start screen.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: yes, in the steps that wait (the page of the save read).
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuReadSavePage(MenuRegs r)
{
    uint16_t target;

    target = menu_page(&r, 0x925A);
    switch (target) {
    case 0x9265: return FileMenuReadSave(r);
    case 0x92CF: return FileMenuStartScreen(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuReadSavePage(void)
{
    MenuRegs r = FileMenuReadSavePage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:341 FileMenuErasePage
 * @run 6B:9341
 * @stop 9346
 * WHAT IT DOES: Page 5: erasing a slot.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: no.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuErasePage(MenuRegs r)
{
    uint16_t target;

    target = menu_page(&r, 0x9346);
    switch (target) {
    case 0x934A: return FileMenuEraseStart(r);
    case 0x937F: return FileMenuEraseWait(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuErasePage(void)
{
    MenuRegs r = FileMenuErasePage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:58E FileMenuNamePage
 * @run 6B:958E
 * @stop 9593
 * WHAT IT DOES: Page 11: the name entry.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: no.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuNamePage(MenuRegs r)
{
    uint16_t target;

    target = menu_page(&r, 0x9593);
    switch (target) {
    case 0x9599: return NameEntryStart(r);
    case 0x96A3: return NameEntryType(r);
    case 0x974E: return NameEntryEnd(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuNamePage(void)
{
    MenuRegs r = FileMenuNamePage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:058 FileMenuListPage
 * @run 6B:9858
 * @stop 985D
 * WHAT IT DOES: Page 9: a slot's list.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: no.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuListPage(MenuRegs r)
{
    uint16_t target;

    target = menu_page(&r, 0x985D);
    switch (target) {
    case 0x9865: return FileMenuListStart(r);
    case 0x988A: return FileMenuListDraw(r);
    case 0x9A3A: return FileMenuSlideLeft(r);
    case 0x98F8: return FileMenuListLeave(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuListPage(void)
{
    MenuRegs r = FileMenuListPage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:130 FileMenuSlidePage
 * @run 6B:9930
 * @stop 9935
 * WHAT IT DOES: Page 2: sliding to the item chosen.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: no.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuSlidePage(MenuRegs r)
{
    uint16_t target;

    target = menu_page(&r, 0x9935);
    switch (target) {
    case 0x993D: return FileMenuSlideStart(r);
    case 0x996C: return FileMenuSlideDraw(r);
    case 0x9A3A: return FileMenuSlideLeft(r);
    case 0x9A9E: return FileMenuSlideEnd(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuSlidePage(void)
{
    MenuRegs r = FileMenuSlidePage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:307 FileMenuBackPage
 * @run 6B:9B07
 * @stop 9B0C
 * WHAT IT DOES: Page 10: sliding back.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: no.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuBackPage(MenuRegs r)
{
    uint16_t target;

    target = menu_page(&r, 0x9B0C);
    switch (target) {
    case 0x9B48: return FileMenuSlideBackStart(r);
    case 0x9B67: return FileMenuSlideBack(r);
    case 0x9B12: return FileMenuBackStart(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuBackPage(void)
{
    MenuRegs r = FileMenuBackPage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:337 FileMenuBackToStartPage
 * @run 6B:9B37
 * @stop 9B3C
 * WHAT IT DOES: Page 8: sliding back to the opening page.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: no.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuBackToStartPage(MenuRegs r)
{
    uint16_t target;

    target = menu_page(&r, 0x9B3C);
    switch (target) {
    case 0x9B42: return FileMenuSlidePrepare(r);
    case 0x9B67: return FileMenuSlideBack(r);
    case 0x9BE3: return FileMenuBackToStart(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuBackToStartPage(void)
{
    MenuRegs r = FileMenuBackToStartPage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C4:424 FileMenuOpenPage
 * @run 6B:9C24
 * @stop 9C29
 * WHAT IT DOES: Page 1: the opening page.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: no.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuOpenPage(MenuRegs r)
{
    uint16_t target;

    target = menu_page(&r, 0x9C29);
    switch (target) {
    case 0x9C2F: return FileMenuOpenStart(r);
    case 0x9C40: return FileMenuOpenSlots(r);
    case 0x9C4B: return FileMenuOpenPick(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuOpenPage(void)
{
    MenuRegs r = FileMenuOpenPage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:3D2 FileMenuCopyPage
 * @run 6B:93D2
 * @stop 93DA
 * WHAT IT DOES: Page 6: copying a slot; the cursor shakes each frame.
 * HOW: JitterCursorObjects, then zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: no.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuCopyPage(MenuRegs r)
{
    uint16_t target;

    jsr_push(0x93D5);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    target = menu_page(&r, 0x93DA);
    switch (target) {
    case 0x93E2: return FileMenuCopyPickFrom(r);
    case 0x9416: return FileMenuCopyDrawRows(r);
    case 0x941E: return FileMenuCopyPickTo(r);
    case 0x9469: return FileMenuCopyMake(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuCopyPage(void)
{
    MenuRegs r = FileMenuCopyPage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* The music select's CD track for each of the 13 choices, and the strings of the menu's three notices. */
#define MUSIC_TRACKS 0xB705
#define MENU_MESSAGES 0xBD6D                /* string pointers: ContinueChoice 0 and 1, then $BD71 and $BD73 */
#define PORTRAIT_X 0xB049                   /* the X of the two objects $0E and $0F */
#define MARK_X 0xB066                       /* the X of the two objects $0C and $0D */
#define FADE_OBJECT 0x10                    /* work_d: the fade's steps left; work_e: frames to the next */
enum { MUSIC_CHOICES = 13 };

/* CdPlayRepeat or CdPlayOnce, its exit replayed. */
static MenuRegs play_track(MenuRegs r, uint8_t track, bool repeat, uint16_t ret)
{
    TrackPlay play;
    MenuReplay *p;

    if (ret != 0) {
        jsr_push(ret);
    }
    play = repeat ? CdPlayRepeat(track, r.x, menu_y(&r)) : CdPlayOnce(track, r.x, menu_y(&r));
    if (ret != 0) {
        jsr_pop();
    }
    p = menu_replay(&r, REPLAY_TRACK);
    p->call.track = play;
    return r;
}

static MenuRegs pause_cd(MenuRegs r, uint16_t ret)
{
    CdPause pause;
    MenuReplay *p;

    if (ret != 0) {
        jsr_push(ret);
    }
    pause = vec_cd_pause(r.x);
    if (ret != 0) {
        jsr_pop();
    }
    p = menu_replay(&r, REPLAY_CD_PAUSE);
    p->call.pause = pause;
    return r;
}

/*
 * @routine 00DF:68B MusicSelectStep
 * @run 6F:B68B
 * WHAT IT DOES: The slot page's frame. Button II, or button I on item $1E, leaves for page $0A (the track playing
 * started again, repeating, if one was). Button I on item $1D pauses the CD and marks it paused; on item $1C pauses it
 * and plays the slot's chosen track once. Left and right change the slot's choice among 13 and draw it. Otherwise the
 * cursor moves by the pad and shakes.
 * HOW: The trigger's bits one at a time; MenuSlotMusic of zSaveSlot; DrawSlotMark; CdPlayRepeat, cd_pause, CdPlayOnce
 * with the track from $B705; CursorMoveByPad (menu 5), JitterCursorObjects.
 * WAITS: yes, on the drive, in the CD calls.
 * Entry: from the menu's page table. Exit: as the arm that ran last leaves them.
 */
MenuRegs MusicSelectStep(MenuRegs r)
{
    uint8_t trigger, item, slot, choice, playing, track;

    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x01));
    if (r.a != 0) {
        item = RD8(OBJ->work_d[0]);
        ra(&r, item);
        rcmp(&r, item, 0x1E);
        if (item != 0x1E) {
            rcmp(&r, item, 0x1D);
            if (item == 0x1D) {
                ra(&r, 0xFF);
                wr8(MenuMusicPlaying, 0xFF);
                return pause_cd(r, 0);
            }
            rcmp(&r, item, 0x1C);
            if (item != 0x1C) {
                return r;
            }
            r = pause_cd(r, 0xB6F7);
            slot = RD8(ZP->save_slot);
            rx(&r, slot);
            choice = rd8((uint16_t)(MenuSlotMusic + slot));
            ry(&r, choice);
            wr8(MenuMusicPlaying, choice);
            track = rd8((uint16_t)(MUSIC_TRACKS + choice));
            ra(&r, track);
            return play_track(r, track, false, 0);
        }
    } else {
        trigger = rd8(BiosJoyTrigger);
        ra(&r, (uint8_t)(trigger & 0x02));
        if (r.a == 0) {
            trigger = rd8(BiosJoyTrigger);
            ra(&r, (uint8_t)(trigger & 0x80));
            if (r.a == 0) {
                trigger = rd8(BiosJoyTrigger);
                ra(&r, (uint8_t)(trigger & 0x20));
                if (r.a == 0) {
                    ry(&r, 5);
                    jsr_push(0xB6AC);
                    r_pad(&r, CursorMoveByPad(5));
                    jsr_pop();
                    jsr_push(0xB6AF);
                    r_glide(&r, JitterCursorObjects(), 1);
                    jsr_pop();
                    return r;
                }
                slot = RD8(ZP->save_slot);
                ry(&r, slot);
                choice = (uint8_t)(rd8((uint16_t)(MenuSlotMusic + slot)) + 1);
                ra(&r, choice);
                rcmp(&r, choice, MUSIC_CHOICES);
                if (choice == MUSIC_CHOICES) {
                    choice = 0;
                    ra_keep(&r, 0);
                }
            } else {
                slot = RD8(ZP->save_slot);
                ry(&r, slot);
                choice = (uint8_t)(rd8((uint16_t)(MenuSlotMusic + slot)) - 1);
                ra(&r, choice);
                if (choice & 0x80) {
                    choice = MUSIC_CHOICES - 1;
                    ra(&r, choice);
                }
            }
            wr8((uint16_t)(MenuSlotMusic + slot), choice);
            return DrawSlotMark(r);
        }
    }
    playing = rd8(MenuMusicPlaying);
    ra(&r, playing);
    if (playing != 0) {
        ra(&r, 0x0E);
        r = play_track(r, 0x0E, true, 0xB6E7);
    }
    ra(&r, 0x0A);
    SetMenuPage(0x0A);
    ra(&r, 0);
    rnz(&r, 0x0A);
    return r;
}

void abi_MusicSelectStep(void)
{
    MenuRegs r = MusicSelectStep(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* One of the pairs of objects the notice pages show: emptied, given picture set 5 and a place ($B02E, $B052). */
static uint8_t notice_object(uint8_t object, uint8_t y_lo, uint16_t xs, uint8_t index, uint16_t ret)
{
    uint8_t x;

    jsr_push(ret);
    ClearObject(object);
    jsr_pop();
    WR8(OBJ->sprite_set[object], 5);
    WR8(OBJ->y_lo[object], y_lo);
    x = rd8((uint16_t)(xs + index));
    WR8(OBJ->x_lo[object], x);
    return x;
}

/*
 * @routine 00DF:027 PlaceMenuPortraits
 * @run 6F:B027
 * WHAT IT DOES: Puts objects $0E and $0F on the screen at Y $D8, X from $B049, frames 2 and 1.
 * HOW: ClearObject, three stores and TYA, INC, EOR 3 to the frame, twice (the second falls through).
 * WAITS: no.
 * Entry: nothing. Exit: A 1, N and Z from it, X $0F, Y 1, C kept.
 */
MenuRegs PlaceMenuPortraits(MenuRegs r)
{
    uint8_t i, frame = 0;

    for (i = 0; i < 2; i++) {
        uint8_t object = (uint8_t)(0x0E + i);

        if (i == 0) {
            jsr_push(0xB02C);
        }
        (void)notice_object(object, 0xD8, PORTRAIT_X, i, 0xB031);
        frame = (uint8_t)((uint8_t)(i + 1) ^ 0x03);
        WR8(OBJ->frame[object], frame);
        if (i == 0) {
            jsr_pop();
        }
    }
    ra(&r, frame);
    r.x = 0x0F;
    r.y = 1;
    r.set |= REG_X | REG_Y;
    return r;
}

void abi_PlaceMenuPortraits(void)
{
    MenuRegs r = PlaceMenuPortraits(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:04B PlaceMenuMarks
 * @run 6F:B04B
 * WHAT IT DOES: Puts objects $0C and $0D on the screen at Y $C0, X from $B066.
 * HOW: ClearObject and three stores, twice (the second falls through).
 * WAITS: no.
 * Entry: nothing. Exit: A the second X, N and Z from it, X $0D, Y 1, C kept.
 */
MenuRegs PlaceMenuMarks(MenuRegs r)
{
    uint8_t value;

    jsr_push(0xB050);
    (void)notice_object(0x0C, 0xC0, MARK_X, 0, 0xB055);
    jsr_pop();
    value = notice_object(0x0D, 0xC0, MARK_X, 1, 0xB055);
    ra(&r, value);
    r.x = 0x0D;
    r.y = 1;
    r.set |= REG_X | REG_Y;
    return r;
}

void abi_PlaceMenuMarks(void)
{
    MenuRegs r = PlaceMenuMarks(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:17C AnimateMenuMarks
 * @run 6F:B17C
 * WHAT IT DOES: Turns objects $0C and $0D through frames 5 to 12, one every four frames.
 * HOW: zFrameTick shifted right twice, ANDed with 7, plus 5, into both frames.
 * WAITS: no.
 * Entry: nothing. Exit: A the frame, N and Z from it, C and V clear; X and Y kept.
 */
MenuRegs AnimateMenuMarks(MenuRegs r)
{
    uint8_t frame = (uint8_t)(((RD8(ZP->frame_tick) >> 2) & 0x07) + 5);

    ra(&r, frame);
    rc(&r, false);
    rv(&r, false);
    WR8(OBJ->frame[0x0C], frame);
    WR8(OBJ->frame[0x0D], frame);
    return r;
}

void abi_AnimateMenuMarks(void)
{
    MenuRegs r = AnimateMenuMarks(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* One of the notice's strings, by the table's byte index ($BD60). */
static MenuRegs menu_message(MenuRegs r, uint8_t index)
{
    WR8(ZP->temp00, rd8((uint16_t)(MENU_MESSAGES + index)));
    ra(&r, rd8((uint16_t)(MENU_MESSAGES + 1 + index)));
    WR8(ZP->temp01, r.a);
    rexit(&r, DrawGlyphText());
    return r;
}

/*
 * @routine 00E0:553 DrawMenuNotice
 * @run 6F:BD53
 * WHAT IT DOES: Draws the notice of the backup memory: the string for ContinueChoice (0 or 1), then two more.
 * HOW: The string pointers at $BD6D, drawn by two BSR and a fall through, the Y index 0 or 2, then 4, then 6.
 * WAITS: no.
 * Entry: nothing. Exit: as the last DrawGlyphText leaves them.
 */
MenuRegs DrawMenuNotice(MenuRegs r)
{
    uint8_t choice, index;

    choice = rd8(ContinueChoice);
    index = (uint8_t)(choice << 1);
    ra(&r, index);
    rc(&r, (choice & 0x80) != 0);
    ry(&r, index);
    jsr_push(0xBD5A);
    r = menu_message(r, index);
    jsr_pop();
    ry(&r, 4);
    jsr_push(0xBD5E);
    r = menu_message(r, 4);
    jsr_pop();
    ry(&r, 6);
    return menu_message(r, 6);
}

void abi_DrawMenuNotice(void)
{
    MenuRegs r = DrawMenuNotice(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00E0:51F FileMenuNoticeStart
 * @run 6F:BD1F
 * WHAT IT DOES: The backup notice page's first step: the game state wiped, list $19 and script $0E started, the page's
 * objects placed.
 * HOW: ResetGameState, StartListChannel0Kind2, StartScriptSlot0Kind1, PlaceMenuMarks, PlaceMenuPortraits,
 * MenuStepAdvance.
 * WAITS: yes, in ResetGameState (the vertical blank).
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuNoticeStart(MenuRegs r)
{
    jsr_push(0xBD22);
    ResetGameState(menu_wait_regs(&r));
    jsr_pop();
    (void)menu_replay(&r, REPLAY_RESET);
    ra(&r, 0x19);
    jsr_push(0xBD27);
    (void)StartListChannel0Kind2(0x19);
    jsr_pop();
    ra(&r, 2);
    rx(&r, 0);
    rnz(&r, 2);
    ra(&r, 0x0E);
    jsr_push(0xBD2C);
    (void)StartScriptSlot0Kind1(0x0E);
    jsr_pop();
    ry(&r, 0x0E);
    rx(&r, 0);
    rnz(&r, 0x0E);
    jsr_push(0xBD2F);
    r = PlaceMenuMarks(r);
    jsr_pop();
    jsr_push(0xBD32);
    r = PlaceMenuPortraits(r);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuNoticeStart(void)
{
    MenuRegs r = FileMenuNoticeStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00E0:535 FileMenuNoticeDraw
 * @run 6F:BD35
 * WHAT IT DOES: The notice page's second step: script $0F and the strings drawn.
 * HOW: StartScriptSlot0Kind1, DrawMenuNotice, MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuNoticeDraw(MenuRegs r)
{
    ra(&r, 0x0F);
    jsr_push(0xBD3A);
    (void)StartScriptSlot0Kind1(0x0F);
    jsr_pop();
    ry(&r, 0x0F);
    rx(&r, 0);
    rnz(&r, 0x0F);
    jsr_push(0xBD3D);
    r = DrawMenuNotice(r);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuNoticeDraw(void)
{
    MenuRegs r = FileMenuNoticeDraw(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00E0:540 FileMenuNoticeWait
 * @run 6F:BD40
 * WHAT IT DOES: The notice stays up until button I: then the mode is done with no save slot, and the intro scenes are
 * asked for.
 * HOW: AnimateMenuMarks, the trigger's bit 0; SMB3 zModeEvents, zSaveSlot $FF, zModeDone counted up.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: no button: A 0, Z set; else A $FF, N and Z from zModeDone.
 */
MenuRegs FileMenuNoticeWait(MenuRegs r)
{
    uint8_t trigger;

    jsr_push(0xBD43);
    r = AnimateMenuMarks(r);
    jsr_pop();
    trigger = rd8(BiosJoyTrigger);
    ra(&r, (uint8_t)(trigger & 0x01));
    if (r.a == 0) {
        return r;
    }
    RMW8(ZP->mode_events, events_, events_ | 0x08);
    ra(&r, 0xFF);
    WR8(ZP->save_slot, 0xFF);
    rnz(&r, INC8(ZP->mode_done));
    return r;
}

void abi_FileMenuNoticeWait(void)
{
    MenuRegs r = FileMenuNoticeWait(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00E0:514 FileMenuNoticePage
 * @run 6F:BD14
 * @stop BD19
 * WHAT IT DOES: Page 13: the backup notice.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: yes, in the first step.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuNoticePage(MenuRegs r)
{
    uint16_t target = menu_page(&r, 0xBD19);

    switch (target) {
    case 0xBD1F: return FileMenuNoticeStart(r);
    case 0xBD35: return FileMenuNoticeDraw(r);
    case 0xBD40: return FileMenuNoticeWait(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuNoticePage(void)
{
    MenuRegs r = FileMenuNoticePage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* One colour of the saved palette at (zTemp00),Y faded one step: blue, red and green each down by one unless zero. */
static void fade_saved_colour(MenuRegs *r, uint8_t y)
{
    uint8_t part, byte, high, carry;

    byte = RD8_PTR_Y(ZP->temp00, y);
    part = (uint8_t)(byte & 0x07);
    if (part != 0) {
        part--;
    }
    WR8(ZP->temp02, part);
    byte = RD8_PTR_Y(ZP->temp00, y);
    part = (uint8_t)(byte & 0x38);
    if (part != 0) {
        rv(r, ((part ^ 0x08) & (part ^ (uint8_t)(part - 0x08)) & 0x80) != 0);
        part = (uint8_t)(part - 0x08);
    }
    part = (uint8_t)(part | RD8(ZP->temp02));
    WR8(ZP->temp02, part);
    byte = RD8_PTR_Y(ZP->temp00, y);
    WR8(ZP->temp03, byte);
    high = RD8_PTR_Y(ZP->temp00, (uint8_t)(y + 1));
    carry = (uint8_t)(high & 1);                    /* LSR A, then ROR of the low byte */
    ROR8(ZP->temp03, carry != 0);
    byte = (uint8_t)(RD8(ZP->temp03) & 0xE0);
    if (byte != 0) {
        rv(r, ((byte ^ 0x20) & (byte ^ (uint8_t)(byte - 0x20)) & 0x80) != 0);
        byte = (uint8_t)(byte - 0x20);
    }
    WR8(ZP->temp03, 0);
    carry = (uint8_t)(byte >> 7);
    byte = (uint8_t)(byte << 1);                    /* ASL A, then ROL of the cleared byte */
    ROL8(ZP->temp03, carry != 0);
    byte = (uint8_t)(byte | RD8(ZP->temp02));
    WR8_PTR_Y(ZP->temp00, y, byte);
    byte = RD8(ZP->temp03);
    WR8_PTR_Y(ZP->temp00, (uint8_t)(y + 1), byte);
}

/*
 * @routine 00E0:31D FileMenuLeaveStep
 * @run 6F:BB1D
 * WHAT IT DOES: The page that leaves the file menu for the game. First: the saved colours cleared, all colours saved
 * to super RAM (script slot 0 as kind 6), nine fade steps set up and the CD faded. Then, a frame at a time: when the
 * step's frames are out, the 512 saved colours are faded one step and written back (kind 7); after the ninth step the
 * screen's first row at $1100 is blanked, the game state wiped, the slot copied to the work area, the backup written
 * (unless there is no slot), and the mode is done.
 * HOW: Two TAIs from the zero bytes at $4003; StartScriptSlotKind6B and 7B; cd_fade; bank $12 (or $30 when bit 0 of
 * zIdleFlag is set) plus the base at MPR6; the fade of $D000 to $D3FF; SelectVramWriteAddress and ST; ResetGameState,
 * SlotToWorkArea, BackupWrite.
 * WAITS: yes, in ResetGameState (the vertical blank) on the last step.
 * Entry: from the menu's page table. Exit: as the arm that ran last leaves them.
 */
MenuRegs FileMenuLeaveStep(MenuRegs r)
{
    BackupWriteRun write;
    SlotCopy copy;
    MenuReplay *p;
    uint8_t step, timer, bank, base, y, high, count;
    unsigned total;

    jsr_push(0xBB20);
    r_glide(&r, JitterCursorObjects(), 1);
    jsr_pop();
    step = RD8(ZP->field80);
    ra(&r, step);
    if (step == 0) {
        block_move_tai(KernelZeros, ListChannelKind, 0x007F, r.a, r.x, menu_y(&r));
        block_move_tai(KernelZeros, ScriptSlotKind, 0x0060, r.a, r.x, menu_y(&r));
        rx(&r, 0);
        jsr_push(0xBB36);
        y = menu_y(&r);
        (void)StartScriptSlotKind6B(y, 0);
        jsr_pop();
        ra(&r, y);
        ra(&r, 0x09);
        WR8(OBJ->work_d[FADE_OBJECT], 0x09);
        ra(&r, 0x01);
        WR8(OBJ->work_e[FADE_OBJECT], 0x01);
        ra(&r, 0x0C);
        jsr_push(0xBB45);
        (void)vec_cd_fade(0x0C);
        jsr_pop();
        p = menu_replay(&r, REPLAY_CD_FADE);
        p->command = 0x0C;
        rnz(&r, INC8(ZP->field80));
        return r;
    }
    timer = DEC8(OBJ->work_e[FADE_OBJECT]);
    rnz(&r, timer);
    if (timer != 0) {
        return r;
    }
    bank = 0x12;
    if (RD8(ZP->idle_flag) & 0x01) {
        bank = 0x30;
    }
    base = RD8(ZP->ram_base_bank);
    total = (unsigned)bank + base;
    ra(&r, (uint8_t)total);
    rc(&r, total > 0xFF);
    rv(&r, ((bank ^ total) & (base ^ total) & 0x80) != 0);
    tam(0x40, (uint8_t)total);
    WR8(ZP->temp00, 0);
    ra(&r, 0xD0);
    WR8(ZP->temp01, 0xD0);
    do {
        uint8_t low, sum;

        y = 0;
        do {
            fade_saved_colour(&r, y);
            y = (uint8_t)(y + 2);
        } while (y < 0x20);
        low = RD8(ZP->temp00);
        sum = (uint8_t)(low + 0x20);
        rv(&r, ((0x20 ^ sum) & (low ^ sum) & 0x80) != 0);
        WR8(ZP->temp00, sum);
        if (low > 0xDF) {
            INC8(ZP->temp01);
        }
        high = RD8(ZP->temp01);
        ra(&r, high);
        rcmp(&r, high, 0xD4);
    } while (high != 0xD4);
    r.y = y;
    r.set |= REG_Y;
    rx(&r, 0);
    jsr_push(0xBBB2);
    (void)StartScriptSlotKind7B(y, 0);
    jsr_pop();
    ra(&r, y);
    ra(&r, 0x08);
    WR8(OBJ->work_e[FADE_OBJECT], 0x08);
    count = DEC8(OBJ->work_d[FADE_OBJECT]);
    rnz(&r, count);
    if (count != 0) {
        return r;
    }
    jsr_push(0xBBBF);
    SelectVramWriteAddress();
    jsr_pop();
    r_select_write(&r);
    st1(0x00);
    st2(0x11);
    jsr_push(0xBBC6);
    SelectVramData();
    jsr_pop();
    r_select_data(&r);
    ry(&r, 0x10);
    for (count = 0x10; count != 0; count--) {
        st1(0x00);
        st2(0x00);
    }
    ry(&r, 0);
    jsr_push(0xBBD2);
    ResetGameState(menu_wait_regs(&r));
    jsr_pop();
    (void)menu_replay(&r, REPLAY_RESET);
    jsr_push(0xBBD5);
    copy = SlotToWorkArea();
    jsr_pop();
    ra(&r, copy.a);
    r.x = copy.x;
    r.y = 5;
    r.set |= REG_X | REG_Y;
    rnz(&r, 0);
    rc(&r, true);
    high = RD8(ZP->save_slot);
    ra(&r, high);
    if ((high & 0x80) == 0) {
        jsr_push(0xBBDC);
        write = BackupWrite();
        jsr_pop();
        p = menu_replay(&r, REPLAY_BACKUP_WRITE);
        p->call.write = write;
    }
    rnz(&r, INC8(ZP->mode_done));
    return r;
}

void abi_FileMenuLeaveStep(void)
{
    MenuRegs r = FileMenuLeaveStep(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * The shop: the record list's entries bought with the slot's total score, and the stage each starts. Two cursor
 * objects (0 and 1) mark the row (OBJ->work_h, 7 the last) and the column (OBJ->work_d); entry = row plus 7 for the
 * right column.
 */
#define SHOP_FRAMES 0xB8E9                  /* the cursor's frame by whether it is in the lower part (overwritten) */
#define SHOP_STAGES 0xB881                  /* the stage each entry starts */
enum { SHOP_ROWS = 8, SHOP_LAST_ROW = 7, SHOP_ENTRY_MARIA = 0x0D, SHOP_SOUND_REFUSED = 0x41, SHOP_SOUND_TAKEN = 0x3F };

/*
 * @routine 00DF:739 PlaceShopCursor
 * @run 6F:B739
 * WHAT IT DOES: Sends the shop's two cursor objects to the row's places: Y $88 and X $08 and $F8, or, on the last
 * row, Y $C8 and X $60 and $A0; both marked moving.
 * HOW: OBJ->work_h compared with 7; the targets in OBJ->work_f (X) and OBJ->work_g (Y); OBJ->work_e 1.
 * WAITS: no.
 * Entry: X the first object. Exit: A 1, N and Z clear, Y the row, C from the compare; X kept.
 */
MenuRegs PlaceShopCursor(MenuRegs r)
{
    uint8_t object = r.x, row;

    row = RD8(OBJ->work_h[object]);
    ry(&r, row);
    rcmp(&r, row, SHOP_LAST_ROW);
    if (row == SHOP_LAST_ROW) {
        ra(&r, 0xC8);
        WR8(OBJ->work_g[object], 0xC8);
        WR8(OBJ->work_g[object + 1], 0xC8);
        ra(&r, 0x60);
        WR8(OBJ->work_f[object], 0x60);
        ra(&r, 0xA0);
        WR8(OBJ->work_f[object + 1], 0xA0);
    } else {
        ra(&r, 0x88);
        WR8(OBJ->work_g[object], 0x88);
        WR8(OBJ->work_g[object + 1], 0x88);
        ra(&r, 0x08);
        WR8(OBJ->work_f[object], 0x08);
        ra(&r, 0xF8);
        WR8(OBJ->work_f[object + 1], 0xF8);
    }
    ra(&r, 1);
    WR8(OBJ->work_e[object], 1);
    WR8(OBJ->work_e[object + 1], 1);
    return r;
}

void abi_PlaceShopCursor(void)
{
    MenuRegs r = PlaceShopCursor(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* One cursor object's move towards its place ($B893), and its frame from the tick. */
static MenuRegs move_shop_object(MenuRegs r, uint8_t object)
{
    uint8_t moving, target, pos, diff, value, frame_index;
    ArithExit halved;
    AxisMove move;
    ValueCarry tick;
    bool carry, out;

    moving = RD8(OBJ->work_e[object]);
    ra(&r, moving);
    if (moving != 0) {
        target = RD8(OBJ->work_f[object]);
        pos = RD8(OBJ->x_lo[object]);
        diff = (uint8_t)(target - pos);
        carry = target >= pos;
        rv(&r, ((target ^ pos) & (target ^ diff) & 0x80) != 0);
        out = (diff & 1) != 0;                       /* ROR A, EOR #$80 */
        value = (uint8_t)(((diff >> 1) | (carry ? 0x80 : 0)) ^ 0x80);
        WR8(OBJ->vx_hi[object], value);
        WR8(OBJ->vx_lo[object], 0);
        (void)ROR8(OBJ->vx_lo[object], out);
        target = RD8(OBJ->work_g[object]);
        pos = RD8(OBJ->y_lo[object]);
        diff = (uint8_t)(target - pos);
        carry = target >= pos;
        rv(&r, ((target ^ pos) & (target ^ diff) & 0x80) != 0);
        out = (diff & 1) != 0;
        value = (uint8_t)(((diff >> 1) | (carry ? 0x80 : 0)) ^ 0x80);
        WR8(OBJ->vy_hi[object], value);
        WR8(OBJ->vy_lo[object], 0);
        (void)ROR8(OBJ->vy_lo[object], out);
        jsr_push(0xB8C1);
        halved = HalveObjectVelocity(object);
        jsr_pop();
        if (halved.overflow_set) {
            rv(&r, halved.overflow);
        }
        jsr_push(0xB8C4);
        move = ApplyVelocity(object);
        jsr_pop();
        rv(&r, move_overflow(move));
        pos = RD8(OBJ->y_lo[object]);
        frame_index = (uint8_t)(pos >= 0xB0 ? 1 : 0);  /* CMP #$B0, CLA, ROL */
        ry(&r, frame_index);
        value = rd8((uint16_t)(SHOP_FRAMES + frame_index));
        WR8(OBJ->frame[object], value);
        value = RD8(OBJ->x_lo[object]);
        ra(&r, value);
        rc(&r, false);
        target = RD8(OBJ->work_f[object]);
        rcmp(&r, value, target);
        if (value == target) {
            value = RD8(OBJ->y_lo[object]);
            ra(&r, value);
            target = RD8(OBJ->work_g[object]);
            rcmp(&r, value, target);
            if (value == target) {
                WR8(OBJ->work_e[object], 0);
            }
        }
    }
    jsr_push(0xB8E8);
    tick = ObjectFrameFromTick(object);
    jsr_pop();
    ra(&r, tick.value);
    rc(&r, tick.carry);
    return r;
}

/*
 * @routine 00E0:08F MoveShopCursor
 * @run 6F:B88F
 * WHAT IT DOES: Moves the two cursor objects: each, while moving, takes a velocity of half the distance left (signed
 * halving by ROR and EOR), halved again, applies it, and stops on its place; both take their frame from the tick.
 * HOW: BSR for object 0, INX, and the same code falls through for object 1.
 * WAITS: no.
 * Entry: nothing. Exit: A object 1's frame, N and Z from it, C bit 1 of the tick, X 1; Y and V as the moves left them.
 */
MenuRegs MoveShopCursor(MenuRegs r)
{
    rx(&r, 0);
    jsr_push(0xB892);
    r = move_shop_object(r, 0);
    jsr_pop();
    r.x = 1;
    r.set |= REG_X;
    return move_shop_object(r, 1);
}

void abi_MoveShopCursor(void)
{
    MenuRegs r = MoveShopCursor(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* The entry the cursor is on, as ShowShopLine and HideShopLine work it out: false on the last row. */
static bool shop_entry(MenuRegs *r, uint8_t object, uint8_t *entry)
{
    uint8_t row, column;

    row = RD8(OBJ->work_h[object]);
    ra(r, row);
    rcmp(r, row, SHOP_LAST_ROW);
    if (row == SHOP_LAST_ROW) {
        return false;
    }
    column = RD8(OBJ->work_d[object]);
    ry(r, column);
    if (column != 0) {
        rc(r, false);
        row = radd(r, row, SHOP_LAST_ROW);
    }
    WR8(ZP->bios_a.lo, row);
    *entry = row;
    return true;
}

/*
 * @routine 00E0:124 ShowShopLine
 * @run 6F:B924
 * WHAT IT DOES: Draws the score line for the cursor's entry and its name highlighted (attribute $11); an entry not
 * reached shows the unknown string.
 * HOW: PHX; DrawSlotRecord; the entry; SlotEntryTest; DrawUnknownName or DrawEntryName; PLX.
 * WAITS: no.
 * Entry: X the cursor object. Exit: X kept, N and Z from it; A, Y, C and V as the last call left them.
 */
MenuRegs ShowShopLine(MenuRegs r)
{
    uint8_t object = r.x, entry;

    push8(object);
    jsr_push(0xB928);
    r = DrawSlotRecord(r);
    jsr_pop();
    if (shop_entry(&r, object, &entry)) {
        rx(&r, entry);
        jsr_push(0xB93D);
        r = SlotEntryTest(r);
        jsr_pop();
        if (r.nz == 0) {
            rx(&r, 0x11);
            entry = RD8(ZP->bios_a.lo);
            ra(&r, entry);
            jsr_push(0xB946);
            r = DrawUnknownName(r);
            jsr_pop();
        } else {
            entry = RD8(ZP->bios_a.lo);
            ra(&r, entry);
            rx(&r, 0x11);
            jsr_push(0xB94F);
            r = DrawEntryName(r);
            jsr_pop();
        }
    }
    rx(&r, pull8());
    return r;
}

void abi_ShowShopLine(void)
{
    MenuRegs r = ShowShopLine(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00E0:151 HideShopLine
 * @run 6F:B951
 * WHAT IT DOES: Draws the cursor's entry back in its plain look: the unknown string when not reached, else its name,
 * $51 when bought and $41 when not.
 * HOW: PHX; the entry; SlotEntryTest; the bought bit; SAX to put the attribute in X and the entry in A; DrawEntryName;
 * PLX.
 * WAITS: no.
 * Entry: X the cursor object. Exit: as ShowShopLine.
 */
MenuRegs HideShopLine(MenuRegs r)
{
    uint8_t object = r.x, entry, slot, offset, bits, attribute;

    push8(object);
    if (shop_entry(&r, object, &entry)) {
        rx(&r, entry);
        jsr_push(0xB967);
        r = SlotEntryTest(r);
        jsr_pop();
        if (r.nz == 0) {
            rx(&r, 0x41);
            entry = RD8(ZP->bios_a.lo);
            ra(&r, entry);
            jsr_push(0xB970);
            r = DrawUnknownName(r);
            jsr_pop();
        } else {
            entry = RD8(ZP->bios_a.lo);
            ra(&r, entry);
            rx(&r, entry);
            slot = RD8(ZP->save_slot);
            ry(&r, slot);
            offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
            ry(&r, offset);
            rcmp(&r, entry, 0x08);
            if (entry >= 0x08) {
                offset++;
                ry(&r, offset);
            }
            bits = (uint8_t)(rd8((uint16_t)(SaveBuffer + 0x22 + offset)) & rd8((uint16_t)(BOUGHT_MASKS + entry)));
            ra(&r, bits);
            attribute = bits != 0 ? 0x51 : 0x41;
            ra(&r, attribute);
            /* SAX: A the entry, X the attribute; the flags stay. */
            r.a = entry;
            r.x = attribute;
            r.set |= REG_A | REG_X;
            jsr_push(0xB991);
            r = DrawEntryName(r);
            jsr_pop();
        }
    }
    rx(&r, pull8());
    return r;
}

void abi_HideShopLine(void)
{
    MenuRegs r = HideShopLine(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:71B FileMenuShopStart
 * @run 6F:B71B
 * WHAT IT DOES: The shop page's first step: the cursor objects to the screen's middle, row and column 0, sent to the
 * row's places, the entry's line shown; the step goes on.
 * HOW: Four stores, two STZ, PlaceShopCursor, ShowShopLine, MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuShopStart(MenuRegs r)
{
    rx(&r, 0);
    ra(&r, 0x80);
    WR8(OBJ->x_lo[0], 0x80);
    WR8(OBJ->y_lo[0], 0x80);
    WR8(OBJ->x_lo[1], 0x80);
    WR8(OBJ->y_lo[1], 0x80);
    WR8(OBJ->work_d[0], 0);
    WR8(OBJ->work_h[0], 0);
    jsr_push(0xB733);
    r = PlaceShopCursor(r);
    jsr_pop();
    jsr_push(0xB736);
    r = ShowShopLine(r);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuShopStart(void)
{
    MenuRegs r = FileMenuShopStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* Decimal mode set and cleared, and a decimal SBC as the HuC6280 makes it (V left alone). */
static void decimal_shim(bool on)
{
    cpu.p = (uint8_t)(on ? (cpu.p | P_D) : (cpu.p & ~P_D));
}

static uint8_t sbc_decimal(uint8_t a, uint8_t value, bool *carry)
{
    uint8_t low = (uint8_t)((a & 0x0F) - (value & 0x0F) - (*carry ? 0 : 1));
    uint8_t high = (uint8_t)((a >> 4) - (value >> 4) - ((low >> 4) & 1));
    uint8_t result = (uint8_t)((high << 4) | (low & 0x0F));

    if (low & 0x10) {
        result = (uint8_t)(result - 0x06);
    }
    if (high & 0x10) {
        result = (uint8_t)(result - 0x60);
    }
    *carry = !((high >> 4) & 1);
    return result;
}

/* The slot's offset in Y, plus one for entries 8 and up. */
static uint8_t shop_flag_offset(MenuRegs *r, uint8_t entry)
{
    uint8_t slot, offset;

    slot = RD8(ZP->save_slot);
    ry(r, slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    ry(r, offset);
    rcmp(r, entry, 0x08);
    if (entry >= 0x08) {
        offset++;
        ry(r, offset);
    }
    return offset;
}

/* An entry taken: its stage, Maria for the last entry, the screens asked for, and page $10. */
static MenuRegs shop_take(MenuRegs r)
{
    CommandCall sound;
    MenuReplay *p;
    uint8_t entry, stage;

    WR8(ZP->character, 0);
    entry = RD8(ZP->bios_a.lo);
    rc(&r, (entry & 1) != 0);
    entry >>= 1;
    ra(&r, entry);
    ry(&r, entry);
    rcmp(&r, entry, SHOP_ENTRY_MARIA);
    if (entry == SHOP_ENTRY_MARIA) {
        rnz(&r, INC8(ZP->character));
    }
    stage = rd8((uint16_t)(SHOP_STAGES + entry));
    ra(&r, stage);
    WR8(ZP->stage, stage);
    RMW8(ZP->mode_events, events_, events_ | 0x10);
    ra(&r, SHOP_SOUND_TAKEN);
    jsr_push(0xB86A);
    sound = SoundCommand(SHOP_SOUND_TAKEN, r.x, menu_y(&r));
    jsr_pop();
    p = menu_replay(&r, REPLAY_SOUND);
    p->command = SHOP_SOUND_TAKEN;
    p->call.sound = sound;
    ra(&r, 0x10);
    SetMenuPage(0x10);
    ra(&r, 0);
    rnz(&r, 0x10);
    return r;
}

/*
 * @routine 00DF:76F FileMenuShopStep
 * @run 6F:B76F
 * WHAT IT DOES: The shop's frame. The pad moves the cursor (left and right change column, except on the last row; up
 * and down change row) and the old and new entries' lines are redrawn; the objects move. Button I: on the last row,
 * back to page $0A; else an entry already bought starts its stage; one not bought is bought if the slot's total score
 * reaches its price and the slot has reached it (the price taken from the score in decimal, the slot's record redrawn),
 * else sound $41. Button II: back to page $0A.
 * HOW: HideShopLine, the pad's bits, PlaceShopCursor, ShowShopLine; MoveShopCursor; the entry doubled in the card's
 * zero page A; SlotScoreReaches, SlotEntryTest; ORA into the bought bits; SED, three SBCs, CLD; DrawSlotScore; the
 * stage from $B881, sound $3F, SMB4 zModeEvents, page $10.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as the arm that ran last.
 */
MenuRegs FileMenuShopStep(MenuRegs r)
{
    uint8_t trigger, row, column, entry, offset, bits, value;
    bool carry, take;
    CommandCall sound;
    MenuReplay *p;

    rx(&r, 0);
    trigger = (uint8_t)(rd8(BiosJoyTrigger) & 0xF0);
    ra(&r, trigger);
    if (trigger != 0) {
        jsr_push(0xB77A);
        r = HideShopLine(r);
        jsr_pop();
        trigger = (uint8_t)(rd8(BiosJoyTrigger) & 0xA0);
        ra(&r, trigger);
        if (trigger != 0) {
            row = RD8(OBJ->work_h[0]);
            ra(&r, row);
            rcmp(&r, row, SHOP_LAST_ROW);
            if (row != SHOP_LAST_ROW) {
                column = (uint8_t)(RD8(OBJ->work_d[0]) ^ 0x01);
                ra(&r, column);
                WR8(OBJ->work_d[0], column);
            }
        }
        trigger = (uint8_t)(rd8(BiosJoyTrigger) & 0x40);
        ra(&r, trigger);
        if (trigger != 0) {
            row = (uint8_t)(RD8(OBJ->work_h[0]) + 1);
            ra(&r, row);
            rcmp(&r, row, SHOP_ROWS);
            if (row == SHOP_ROWS) {
                row = 0;
                ra_keep(&r, 0);
            }
            WR8(OBJ->work_h[0], row);
        }
        trigger = (uint8_t)(rd8(BiosJoyTrigger) & 0x10);
        ra(&r, trigger);
        if (trigger != 0) {
            row = (uint8_t)(RD8(OBJ->work_h[0]) - 1);
            ra(&r, row);
            if (row & 0x80) {
                row = SHOP_LAST_ROW;
                ra(&r, row);
            }
            WR8(OBJ->work_h[0], row);
        }
        row = RD8(OBJ->work_h[0]);
        ra(&r, row);
        rcmp(&r, row, SHOP_LAST_ROW);
        if (row != SHOP_LAST_ROW) {
            column = RD8(OBJ->work_d[0]);
            ry(&r, column);
            if (column != 0) {
                rc(&r, false);
                row = radd(&r, row, SHOP_LAST_ROW);
            }
        }
        jsr_push(0xB7C7);
        r = PlaceShopCursor(r);
        jsr_pop();
        jsr_push(0xB7CA);
        r = ShowShopLine(r);
        jsr_pop();
    }
    jsr_push(0xB7CD);
    r = MoveShopCursor(r);
    jsr_pop();
    trigger = (uint8_t)(rd8(BiosJoyTrigger) & 0x01);
    ra(&r, trigger);
    if (trigger == 0) {
        trigger = (uint8_t)(rd8(BiosJoyTrigger) & 0x02);
        ra(&r, trigger);
        if (trigger == 0) {
            return r;
        }
        goto leave;
    }
    rx(&r, 0);
    row = RD8(OBJ->work_h[0]);
    ra(&r, row);
    rcmp(&r, row, SHOP_LAST_ROW);
    if (row == SHOP_LAST_ROW) {
        goto leave;
    }
    column = RD8(OBJ->work_d[0]);
    ry(&r, column);
    if (column != 0) {
        rc(&r, false);
        row = radd(&r, row, SHOP_LAST_ROW);
    }
    rc(&r, (row & 0x80) != 0);
    value = (uint8_t)(row << 1);
    ra(&r, value);
    WR8(ZP->bios_a.lo, value);
    value = RD8(ZP->bios_a.lo);
    rc(&r, (value & 1) != 0);
    entry = (uint8_t)(value >> 1);
    ra(&r, entry);
    rx(&r, entry);
    offset = shop_flag_offset(&r, entry);
    bits = (uint8_t)(rd8((uint16_t)(SaveBuffer + 0x22 + offset)) & rd8((uint16_t)(BOUGHT_MASKS + entry)));
    ra(&r, bits);
    take = bits != 0;
    if (!take) {
        value = RD8(ZP->bios_a.lo);
        ry(&r, value);
        jsr_push(0xB809);
        r = SlotScoreReaches(r);
        jsr_pop();
        if (!r.c) {
            goto refused;
        }
        value = RD8(ZP->bios_a.lo);
        ry(&r, value);
        jsr_push(0xB810);
        r = SlotEntryTest(r);
        jsr_pop();
        if (r.nz == 0) {
            goto refused;
        }
        value = RD8(ZP->bios_a.lo);
        rc(&r, (value & 1) != 0);
        entry = (uint8_t)(value >> 1);
        ra(&r, entry);
        rx(&r, entry);
        offset = shop_flag_offset(&r, entry);
        bits = (uint8_t)(rd8((uint16_t)(SaveBuffer + 0x22 + offset)) | rd8((uint16_t)(BOUGHT_MASKS + entry)));
        ra(&r, bits);
        wr8((uint16_t)(SaveBuffer + 0x22 + offset), bits);
        entry = RD8(ZP->bios_a.lo);
        rx(&r, entry);
        value = RD8(ZP->save_slot);
        ry(&r, value);
        offset = rd8((uint16_t)(SLOT_OFFSETS + value));
        ry(&r, offset);
        decimal_shim(true);
        carry = true;
        value = sbc_decimal(rd8((uint16_t)(SaveScoreTotal + offset)), rd8((uint16_t)(RECORD_SCORES + entry)), &carry);
        wr8((uint16_t)(SaveScoreTotal + offset), value);
        value = sbc_decimal(rd8((uint16_t)(SaveScoreTotal + 1 + offset)), rd8((uint16_t)(RECORD_SCORES + 1 + entry)), &carry);
        wr8((uint16_t)(SaveScoreTotal + 1 + offset), value);
        value = sbc_decimal(rd8((uint16_t)(SaveScoreTotal + 2 + offset)), 0, &carry);
        wr8((uint16_t)(SaveScoreTotal + 2 + offset), value);
        decimal_shim(false);
        ra(&r, value);
        rc(&r, carry);
        jsr_push(0xB852);
        r = DrawSlotScore(r);
        jsr_pop();
    }
    return shop_take(r);
refused:
    ra(&r, SHOP_SOUND_REFUSED);
    jsr_push(0xB880);
    sound = SoundCommand(SHOP_SOUND_REFUSED, r.x, menu_y(&r));
    jsr_pop();
    p = menu_replay(&r, REPLAY_SOUND);
    p->command = SHOP_SOUND_REFUSED;
    p->call.sound = sound;
    return r;
leave:
    ra(&r, 0x0A);
    SetMenuPage(0x0A);
    ra(&r, 0);
    rnz(&r, 0x0A);
    return r;
}

void abi_FileMenuShopStep(void)
{
    MenuRegs r = FileMenuShopStep(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:712 FileMenuShopPage
 * @run 6F:B712
 * @stop B717
 * WHAT IT DOES: Page 15: the shop.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: no.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuShopPage(MenuRegs r)
{
    uint16_t target = menu_page(&r, 0xB717);

    switch (target) {
    case 0xB71B: return FileMenuShopStart(r);
    case 0xB76F: return FileMenuShopStep(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuShopPage(void)
{
    MenuRegs r = FileMenuShopPage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * The stage map (page 12): the stages the selected slot has reached, each marked on a map; the pad moves between
 * neighbouring ones that are open; button I takes the stage chosen. MenuSlotProgress holds the position, bit 7 set
 * when it was left by button II. SlotClearedFlags writes the open positions to $3D00 and up.
 */
#define MAP_FLAGS 0x3D00                    /* nonzero: the position can be moved to (SlotClearedFlags) */
#define MAP_START 0x3D10                    /* the position the map was entered with */
#define MAP_NEIGHBOURS 0xAF49               /* a pointer to a neighbour table for left, down, right, up, and two more */
#define MAP_MARK_X 0xAFD0                   /* marks' X and Y by object ($20 to $2D) */
#define MAP_MARK_Y 0xAFDE
#define MAP_CURSOR_X 0xAFF0                 /* the cursor's place by position */
#define MAP_CURSOR_Y 0xAFFE
#define MAP_PATH_X 0xB0FC
#define MAP_PATH_Y 0xB109
#define MAP_PATHS 0xB0EF                    /* the path animation by position, 0 none */
#define MAP_LINKS 0xB13B                    /* the mark lit beside a position */
#define MAP_CURSOR_LIST 0xB177              /* the cursor's frame list */
#define MAP_BLINK_FRAMES 0xB198             /* a frame by object ($10 to $1D) */
#define MAP_PATH_LISTS 0xB1CA               /* frame list pointers by path animation */
#define MAP_NAMES 0xB23A                    /* string pointers by position */
#define MAP_TEXTS_A 0xB365                  /* and by position and character */
#define MAP_TEXTS_B 0xB4F8
#define MAP_OBJECT_FRAMES 0xAEDF            /* object $34's frame by character */
enum { MAP_MARK_FIRST = 0x20, MAP_MARK_END = 0x2E, MAP_PATH_OBJECT = 0x30, MAP_MOVE_SOUND = 0x22 };

/* DrawGlyphText through a pointer from a word table: LDA table,x; STA zTemp00; LDA table+1,x; STA zTemp01. */
static MenuRegs draw_map_string(MenuRegs r, uint16_t table, uint8_t index, uint16_t ret)
{
    WR8(ZP->temp00, rd8((uint16_t)(table + index)));
    ra(&r, rd8((uint16_t)(table + 1 + index)));
    WR8(ZP->temp01, r.a);
    jsr_push(ret);
    rexit(&r, DrawGlyphText());
    jsr_pop();
    return r;
}

/*
 * @routine 00DF:1F9 DrawMapStrings
 * @run 6F:B1F9
 * WHAT IT DOES: Draws the strings of the map's position: its name, then the character's text for it.
 * HOW: The position doubled indexes MAP_NAMES; DrawGlyphText; then MAP_TEXTS_B or MAP_TEXTS_A by MenuSlotCharacter.
 * WAITS: no.
 * Entry: nothing. Exit: as DrawGlyphText.
 */
MenuRegs DrawMapStrings(MenuRegs r)
{
    uint8_t slot, position, index, character;

    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    position = rd8((uint16_t)(MenuSlotProgress + slot));
    index = (uint8_t)(position << 1);
    ra(&r, index);
    rc(&r, (position & 0x80) != 0);
    rx(&r, index);
    r = draw_map_string(r, MAP_NAMES, index, 0xB20D);
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    character = rd8((uint16_t)(MenuSlotCharacter + slot));
    ra(&r, character);
    position = rd8((uint16_t)(MenuSlotProgress + slot));
    index = (uint8_t)(position << 1);
    ra(&r, index);
    rc(&r, (position & 0x80) != 0);
    rx(&r, index);
    if (character != 0) {
        return draw_map_string(r, MAP_TEXTS_B, index, 0xB226);
    }
    return draw_map_string(r, MAP_TEXTS_A, index, 0xB239);
}

void abi_DrawMapStrings(void)
{
    MenuRegs r = DrawMapStrings(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:01A PlaceMapBackground
 * @run 6F:B01A
 * WHAT IT DOES: Puts object 0 on the map screen: picture set 5, frame 1.
 * HOW: ClearObject, a store and an INC.
 * WAITS: no.
 * Entry: nothing. Exit: A 5, X 0, N and Z from the frame, C kept.
 */
MenuRegs PlaceMapBackground(MenuRegs r)
{
    rx(&r, 0);
    jsr_push(0xB01E);
    ClearObject(0);
    jsr_pop();
    ra(&r, 5);
    WR8(OBJ->sprite_set[0], 5);
    rnz(&r, INC8(OBJ->frame[0]));
    return r;
}

void abi_PlaceMapBackground(void)
{
    MenuRegs r = PlaceMapBackground(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:068 PlaceMapMarks
 * @run 6F:B068
 * WHAT IT DOES: Puts the map's 14 marks (objects $20 to $2D) and their shadows (objects $10 to $1D) at their places;
 * a mark whose position is open gets frame $0D.
 * HOW: A loop over X; ClearObject; stores with X and X less $10; the flag at $3CE0+X.
 * WAITS: no.
 * Entry: nothing. Exit: A the last flag or $0D, X $2E, Z and C set, N clear.
 */
MenuRegs PlaceMapMarks(MenuRegs r)
{
    uint8_t object, value, flag;

    for (object = MAP_MARK_FIRST; object != MAP_MARK_END; object++) {
        jsr_push(0xB06D);
        ClearObject(object);
        jsr_pop();
        WR8(OBJ->sprite_set[object], 5);
        WR8(OBJ->sprite_set[(uint8_t)(object - 0x10)], 5);
        value = rd8((uint16_t)(MAP_MARK_X + object));
        WR8(OBJ->x_lo[object], value);
        WR8(OBJ->x_lo[(uint8_t)(object - 0x10)], value);
        value = rd8((uint16_t)(MAP_MARK_Y + object));
        WR8(OBJ->y_lo[object], value);
        WR8(OBJ->y_lo[(uint8_t)(object - 0x10)], value);
        flag = rd8((uint16_t)(MAP_FLAGS - MAP_MARK_FIRST + object));
        ra(&r, flag);
        if (flag != 0) {
            ra(&r, 0x0D);
            WR8(OBJ->frame[object], 0x0D);
        }
    }
    rx(&r, object);
    rcmp(&r, object, MAP_MARK_END);
    return r;
}

void abi_PlaceMapMarks(void)
{
    MenuRegs r = PlaceMapMarks(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* A mark lit (or put out): its start argument counted up (or cleared), and, when its position is closed, frame $0D
 * and attribute $10 (or frame 0). */
static void light_mark(MenuRegs *r, uint8_t x, bool light)
{
    uint8_t flag;

    if (light) {
        rnz(r, INC8(OBJ->start_arg[(uint8_t)(x + 0x10)]));
    } else {
        WR8(OBJ->start_arg[(uint8_t)(x + 0x10)], 0);
    }
    flag = rd8((uint16_t)(MAP_FLAGS + x));
    ra(r, flag);
    if (flag == 0) {
        if (light) {
            ra(r, 0x0D);
            WR8(OBJ->frame[(uint8_t)(x + 0x20)], 0x0D);
            ra(r, 0x10);
            WR8(OBJ->sprite_attr[(uint8_t)(x + 0x20)], 0x10);
        } else {
            WR8(OBJ->frame[(uint8_t)(x + 0x20)], 0);
        }
    }
}

/* The marks of the position the slot is on: position 1 also has the mark of position 3. */
static MenuRegs position_marks(MenuRegs r, bool light)
{
    uint8_t slot, position, link;

    slot = RD8(ZP->save_slot);
    rx(&r, slot);
    position = rd8((uint16_t)(MenuSlotProgress + slot));
    ry(&r, position);
    rcmp(&r, position, 0x01);
    if (position == 0x01) {
        rx(&r, 0x03);
        light_mark(&r, 0x03, light);
    }
    link = rd8((uint16_t)(MAP_LINKS + position));
    rx(&r, link);
    light_mark(&r, link, light);
    return r;
}

/*
 * @routine 00DF:097 LightMapMarks
 * @run 6F:B097
 * WHAT IT DOES: Lights the marks beside the slot's position.
 * HOW: The position's link from $B13B (position 1 the mark 3 too): INC of start_arg, frame $0D and attribute $10 on a
 * closed one.
 * WAITS: no.
 * Entry: nothing. Exit: X the link, Y the position, A and the flags as the last test or store left them.
 */
MenuRegs LightMapMarks(MenuRegs r)
{
    return position_marks(r, true);
}

void abi_LightMapMarks(void)
{
    MenuRegs r = LightMapMarks(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:116 PutOutMapMarks
 * @run 6F:B116
 * WHAT IT DOES: Puts out the marks beside the slot's position.
 * HOW: STZ of start_arg, and of the frame on a closed one.
 * WAITS: no.
 * Entry: nothing. Exit: as LightMapMarks.
 */
MenuRegs PutOutMapMarks(MenuRegs r)
{
    return position_marks(r, false);
}

void abi_PutOutMapMarks(void)
{
    MenuRegs r = PutOutMapMarks(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:0CA PlaceMapPath
 * @run 6F:B0CA
 * WHAT IT DOES: Puts object $30, the path from the slot's position, on the map, when the position has one.
 * HOW: ClearObject; $B0EF by position to start_arg; the place from $B0FC and $B109; picture set 5.
 * WAITS: no.
 * Entry: nothing. Exit: X $30, Y the position; none: A 0, Z set; else A 5, N and Z clear. C kept.
 */
MenuRegs PlaceMapPath(MenuRegs r)
{
    uint8_t slot, position, value;

    rx(&r, MAP_PATH_OBJECT);
    jsr_push(0xB0CF);
    ClearObject(MAP_PATH_OBJECT);
    jsr_pop();
    slot = RD8(ZP->save_slot);
    position = rd8((uint16_t)(MenuSlotProgress + slot));
    ry(&r, position);
    value = rd8((uint16_t)(MAP_PATHS + position));
    ra(&r, value);
    r.y = position;
    if (value != 0) {
        WR8(OBJ->start_arg[MAP_PATH_OBJECT], value);
        value = rd8((uint16_t)(MAP_PATH_X + position));
        WR8(OBJ->x_lo[MAP_PATH_OBJECT], value);
        value = rd8((uint16_t)(MAP_PATH_Y + position));
        WR8(OBJ->y_lo[MAP_PATH_OBJECT], value);
        ra(&r, 5);
        WR8(OBJ->sprite_set[MAP_PATH_OBJECT], 5);
    }
    return r;
}

void abi_PlaceMapPath(void)
{
    MenuRegs r = PlaceMapPath(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* StepObjectFrames' exit with the Y it leaves. */
static void r_step_frames(MenuRegs *r, uint8_t object, uint16_t ret)
{
    FrameStep step;
    uint8_t y = 0;

    jsr_push(ret);
    step = StepObjectFrames(object, &y);
    jsr_pop();
    (void)r_frames(r, step);
    if (step != FRAME_HELD) {
        r->y = y;
        r->set |= REG_Y;
    }
}

/*
 * @routine 00DF:148 MoveMapCursor
 * @run 6F:B148
 * WHAT IT DOES: Animates object 0, the map's cursor, and puts it at the slot's position with a vertical wave.
 * HOW: StepObjectFrames with the list at $B177; ScaleBySine of 5 by the frame counter times four; the Y of the
 * position plus the product's middle byte, and the X.
 * WAITS: no.
 * Entry: nothing. Exit: A the X place, N and Z from it, Y the position, X 0; C and V from the Y add.
 */
MenuRegs MoveMapCursor(MenuRegs r)
{
    ScaleResult wave;
    uint8_t slot, position, base, sum, value;
    unsigned total;

    rx(&r, 0);
    WR8(ZP->temp10, (uint8_t)MAP_CURSOR_LIST);
    ra(&r, (uint8_t)(MAP_CURSOR_LIST >> 8));
    WR8(ZP->temp11, (uint8_t)(MAP_CURSOR_LIST >> 8));
    r_step_frames(&r, 0, 0xB154);
    WR8(ZP->temp00, 0x05);
    WR8(ZP->temp01, 0x00);
    value = (uint8_t)(RD8(ZP->frame_tick) << 2);
    ra(&r, value);
    jsr_push(0xB161);
    wave = ScaleBySine(value);
    jsr_pop();
    (void)wave;
    ry(&r, 0);
    slot = RD8(ZP->save_slot);
    position = rd8((uint16_t)(MenuSlotProgress + slot));
    ry(&r, position);
    base = rd8((uint16_t)(MAP_CURSOR_Y + position));
    total = (unsigned)base + RD8(ZP->product.mid);
    sum = (uint8_t)total;
    rc(&r, total > 0xFF);
    rv(&r, ((base ^ sum) & ((uint8_t)(total - base) ^ sum) & 0x80) != 0);
    WR8(OBJ->y_lo[0], sum);
    value = rd8((uint16_t)(MAP_CURSOR_X + position));
    ra(&r, value);
    WR8(OBJ->x_lo[0], value);
    return r;
}

void abi_MoveMapCursor(void)
{
    MenuRegs r = MoveMapCursor(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:18C BlinkMapMarks
 * @run 6F:B18C
 * WHAT IT DOES: Blinks the lit marks (objects $10 to $1D): an object with a start argument shows its frame on every
 * other 16 frames.
 * HOW: STZ of the frame; start_arg and bit 4 of zFrameTick; the frame from $B198+X.
 * WAITS: no.
 * Entry: nothing. Exit: X $1E, Z and C set, N clear, A the last value read.
 */
MenuRegs BlinkMapMarks(MenuRegs r)
{
    uint8_t object, value;

    for (object = 0x10; object != 0x1E; object++) {
        WR8(OBJ->frame[object], 0);
        value = RD8(OBJ->start_arg[object]);
        ra(&r, value);
        if (value != 0) {
            value = (uint8_t)(RD8(ZP->frame_tick) & 0x10);
            ra(&r, value);
            if (value != 0) {
                value = rd8((uint16_t)(MAP_BLINK_FRAMES + object));
                ra(&r, value);
                WR8(OBJ->frame[object], value);
            }
        }
    }
    rx(&r, object);
    rcmp(&r, object, 0x1E);
    return r;
}

void abi_BlinkMapMarks(void)
{
    MenuRegs r = BlinkMapMarks(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DF:1B6 AnimateMapPath
 * @run 6F:B1B6
 * WHAT IT DOES: Steps the path object's animation, when it has one.
 * HOW: start_arg doubled indexes the list pointers at $B1CA; a tail jump to StepObjectFrames; none returns.
 * WAITS: no.
 * Entry: nothing. Exit: none: A 0, Z set, X $30; else as StepObjectFrames.
 */
MenuRegs AnimateMapPath(MenuRegs r)
{
    FrameStep step;
    uint8_t value, index, y = 0;

    rx(&r, MAP_PATH_OBJECT);
    value = RD8(OBJ->start_arg[MAP_PATH_OBJECT]);
    ra(&r, value);
    if (value == 0) {
        return r;
    }
    index = (uint8_t)(value << 1);
    ra(&r, index);
    rc(&r, (value & 0x80) != 0);
    ry(&r, index);
    WR8(ZP->temp10, rd8((uint16_t)(MAP_PATH_LISTS + index)));
    ra(&r, rd8((uint16_t)(MAP_PATH_LISTS + 1 + index)));
    WR8(ZP->temp11, r.a);
    step = StepObjectFrames(MAP_PATH_OBJECT, &y);
    (void)r_frames(&r, step);
    if (step != FRAME_HELD) {
        r.y = y;
        r.set |= REG_Y;
    }
    return r;
}

void abi_AnimateMapPath(void)
{
    MenuRegs r = AnimateMapPath(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* The neighbour of the slot's position in the table pointed to at MAP_NEIGHBOURS + index; X the neighbour. */
static MenuRegs map_neighbour(MenuRegs r, uint8_t index)
{
    uint8_t slot, position, neighbour, flag;

    WR8(ZP->temp00, rd8((uint16_t)(MAP_NEIGHBOURS + index)));
    WR8(ZP->temp01, rd8((uint16_t)(MAP_NEIGHBOURS + 1 + index)));
    slot = RD8(ZP->save_slot);
    rx(&r, slot);
    position = rd8((uint16_t)(MenuSlotProgress + slot));
    ry(&r, position);
    neighbour = RD8_PTR_Y(ZP->temp00, position);
    ra(&r, neighbour);
    rx(&r, neighbour);
    flag = rd8((uint16_t)(MAP_FLAGS + neighbour));
    ra(&r, flag);
    return r;
}

/* To the neighbour in X ($AF08): marks put out, the position moved, path, marks and strings drawn, a sound. */
static MenuRegs map_move(MenuRegs r)
{
    CommandCall sound;
    MenuReplay *p;
    uint8_t neighbour = r.x, slot;

    push8(neighbour);
    jsr_push(0xAF0C);
    r = PutOutMapMarks(r);
    jsr_pop();
    neighbour = pull8();
    rx(&r, neighbour);
    ra(&r, neighbour);
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    wr8((uint16_t)(MenuSlotProgress + slot), neighbour);
    jsr_push(0xAF16);
    r = PlaceMapPath(r);
    jsr_pop();
    jsr_push(0xAF19);
    r = LightMapMarks(r);
    jsr_pop();
    jsr_push(0xAF1C);
    r = DrawMapStrings(r);
    jsr_pop();
    ra(&r, MAP_MOVE_SOUND);
    jsr_push(0xAF21);
    sound = SoundCommand(MAP_MOVE_SOUND, r.x, menu_y(&r));
    jsr_pop();
    p = menu_replay(&r, REPLAY_SOUND);
    p->command = MAP_MOVE_SOUND;
    p->call.sound = sound;
    return r;
}

/*
 * @routine 00DE:6E1 MoveOnMap
 * @run 6F:AEE1
 * WHAT IT DOES: Moves the slot's position by the pad: to the neighbour in the direction pressed when it is open; for
 * left or right, when that one is closed, to the one the fifth or sixth table gives, if open.
 * HOW: The first set bit of the pad's top four (a loop of ASL) picks the table; ($00),Y; PutOutMapMarks, the new
 * position, PlaceMapPath, LightMapMarks, DrawMapStrings, sound $22.
 * WAITS: no.
 * Entry: nothing. Exit: as the arm that ran last.
 */
MenuRegs MoveOnMap(MenuRegs r)
{
    uint8_t pad, direction, index;

    pad = (uint8_t)(rd8(BiosJoyTrigger) & 0xF0);
    ra(&r, pad);
    if (pad == 0) {
        return r;
    }
    direction = 0xFF;
    do {
        direction++;
        rc(&r, (pad & 0x80) != 0);
        pad = (uint8_t)(pad << 1);
    } while (!r.c);
    index = (uint8_t)(direction << 1);
    rc(&r, (direction & 0x80) != 0);
    ra(&r, index);
    ry(&r, index);
    r = map_neighbour(r, index);
    if (r.a != 0) {
        return map_move(r);
    }
    pad = (uint8_t)(rd8(BiosJoyTrigger) & 0xA0);
    ra(&r, pad);
    if (pad == 0) {
        return r;
    }
    ry(&r, 0x08);
    ra(&r, (uint8_t)(pad & 0x80));
    index = 0x08;
    if (r.a == 0) {
        index = 0x0A;
        ry(&r, 0x0A);
    }
    r = map_neighbour(r, index);
    if (r.a != 0) {
        return map_move(r);
    }
    return r;
}

void abi_MoveOnMap(void)
{
    MenuRegs r = MoveOnMap(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DE:5AE FileMenuMapStart
 * @run 6F:ADAE
 * WHAT IT DOES: The map page's first step: the game state wiped, list $19 and script $0F, the position kept in
 * MAP_START, the open positions found, and the map's objects placed.
 * HOW: ResetGameState and the calls in order; MenuStepAdvance.
 * WAITS: yes, in ResetGameState (the vertical blank).
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuMapStart(MenuRegs r)
{
    SlotFlags flags;
    uint8_t slot, position;

    jsr_push(0xADB1);
    ResetGameState(menu_wait_regs(&r));
    jsr_pop();
    (void)menu_replay(&r, REPLAY_RESET);
    ra(&r, 0x19);
    jsr_push(0xADB6);
    (void)StartListChannel0Kind2(0x19);
    jsr_pop();
    ra(&r, 2);
    rx(&r, 0);
    rnz(&r, 2);
    ra(&r, 0x0F);
    jsr_push(0xADBB);
    (void)StartScriptSlot0Kind1(0x0F);
    jsr_pop();
    ry(&r, 0x0F);
    rx(&r, 0);
    rnz(&r, 0x0F);
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    position = rd8((uint16_t)(MenuSlotProgress + slot));
    ra(&r, position);
    wr8(MAP_START, position);
    jsr_push(0xADC6);
    flags = SlotClearedFlags();
    jsr_pop();
    ra(&r, flags.a);
    rx(&r, 13);
    r.y = flags.y;
    r.set |= REG_Y;
    rnz(&r, 0);
    rc(&r, true);
    rv(&r, flags.v);
    jsr_push(0xADC9);
    r = PlaceMapBackground(r);
    jsr_pop();
    jsr_push(0xADCC);
    r = PlaceMenuMarks(r);
    jsr_pop();
    jsr_push(0xADCF);
    r = PlaceMenuPortraits(r);
    jsr_pop();
    jsr_push(0xADD2);
    r = PlaceMapMarks(r);
    jsr_pop();
    jsr_push(0xADD5);
    r = LightMapMarks(r);
    jsr_pop();
    jsr_push(0xADD8);
    r = PlaceMapPath(r);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuMapStart(void)
{
    MenuRegs r = FileMenuMapStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DE:5DB FileMenuMapDraw
 * @run 6F:ADDB
 * WHAT IT DOES: The map page's second step: the position's strings drawn.
 * HOW: DrawMapStrings, MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuMapDraw(MenuRegs r)
{
    jsr_push(0xADDE);
    r = DrawMapStrings(r);
    jsr_pop();
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuMapDraw(void)
{
    MenuRegs r = FileMenuMapDraw(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DE:5E1 FileMenuMapStep
 * @run 6F:ADE1
 * WHAT IT DOES: The map's frame: the objects animate and the pad moves the position. Button I goes on a step with sound
 * $3F; button II goes on a step with the entry position put back, bit 7 set.
 * HOW: MoveMapCursor, AnimateMenuMarks, BlinkMapMarks, AnimateMapPath, MoveOnMap; the trigger's bits 0 and 1.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as the arm that ran last.
 */
MenuRegs FileMenuMapStep(MenuRegs r)
{
    CommandCall sound;
    MenuReplay *p;
    uint8_t trigger, position, slot;

    jsr_push(0xADE4);
    r = MoveMapCursor(r);
    jsr_pop();
    jsr_push(0xADE7);
    r = AnimateMenuMarks(r);
    jsr_pop();
    jsr_push(0xADEA);
    r = BlinkMapMarks(r);
    jsr_pop();
    jsr_push(0xADED);
    r = AnimateMapPath(r);
    jsr_pop();
    jsr_push(0xADF0);
    r = MoveOnMap(r);
    jsr_pop();
    trigger = (uint8_t)(rd8(BiosJoyTrigger) & 0x03);
    ra(&r, trigger);
    if (trigger == 0) {
        return r;
    }
    jsr_push(0xADFA);
    ra(&r, MenuStepAdvance());
    jsr_pop();
    trigger = (uint8_t)(rd8(BiosJoyTrigger) & 0x01);
    ra(&r, trigger);
    if (trigger == 0) {
        position = rd8(MAP_START);
        slot = RD8(ZP->save_slot);
        ry(&r, slot);
        position = (uint8_t)(position | 0x80);
        ra(&r, position);
        wr8((uint16_t)(MenuSlotProgress + slot), position);
        return r;
    }
    ra(&r, 0x3F);
    jsr_push(0xAE12);
    sound = SoundCommand(0x3F, r.x, menu_y(&r));
    jsr_pop();
    p = menu_replay(&r, REPLAY_SOUND);
    p->command = 0x3F;
    p->call.sound = sound;
    return r;
}

void abi_FileMenuMapStep(void)
{
    MenuRegs r = FileMenuMapStep(menu_regs_cpu());

    menu_regs_shim(&r);
}

/* StartListChannel0Kind2: A 2, X 0. */
static MenuRegs start_list(MenuRegs r, uint8_t list, uint16_t ret)
{
    ra(&r, list);
    jsr_push(ret);
    (void)StartListChannel0Kind2(list);
    jsr_pop();
    ra(&r, 2);
    rx(&r, 0);
    rnz(&r, 2);
    return r;
}

/*
 * @routine 00DE:613 FileMenuMapLeaveStart
 * @run 6F:AE13
 * WHAT IT DOES: The map page's step after the pick: the game state wiped, the start screen's lists again.
 * HOW: ResetGameState, list $0D on channel 0, lists 0 and 1 as kind 3 on channels 7 and 6, MenuStepAdvance.
 * WAITS: yes, in ResetGameState (the vertical blank).
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuMapLeaveStart(MenuRegs r)
{
    jsr_push(0xAE16);
    ResetGameState(menu_wait_regs(&r));
    jsr_pop();
    (void)menu_replay(&r, REPLAY_RESET);
    r = start_list(r, 0x0D, 0xAE1B);
    rx(&r, 0x07);
    ra_keep(&r, 0);
    jsr_push(0xAE21);
    (void)StartListChannelKind3(0, 7);
    jsr_pop();
    ra(&r, 3);
    rx(&r, 0x06);
    ra(&r, 1);
    jsr_push(0xAE28);
    (void)StartListChannelKind3(1, 6);
    jsr_pop();
    ra(&r, 3);
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuMapLeaveStart(void)
{
    MenuRegs r = FileMenuMapLeaveStart(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DE:62B FileMenuMapLeaveObjects
 * @run 6F:AE2B
 * WHAT IT DOES: Puts the slot page's objects back; the cursor on item $15 when button II left the map (the bit
 * cleared), else on $13.
 * HOW: List $0F, object list 0, bit 7 of MenuSlotProgress, PlaceCursorObjects, MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuMapLeaveObjects(MenuRegs r)
{
    CursorTarget place;
    uint8_t slot, position, item;

    r = start_list(r, 0x0F, 0xAE30);
    ra_keep(&r, 0);
    jsr_push(0xAE34);
    rexit(&r, PlaceObjectList(0));
    jsr_pop();
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    position = rd8((uint16_t)(MenuSlotProgress + slot));
    ra(&r, position);
    if (position & 0x80) {
        position &= 0x7F;
        ra(&r, position);
        wr8((uint16_t)(MenuSlotProgress + slot), position);
        item = 0x15;
    } else {
        item = 0x13;
    }
    ra(&r, item);
    jsr_push(0xAE49);
    place = PlaceCursorObjects(item);
    jsr_pop();
    ra(&r, place.value);
    r.x = 1;
    r.y = place.index;
    r.set |= REG_X | REG_Y;
    rc(&r, place.carry);
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuMapLeaveObjects(void)
{
    MenuRegs r = FileMenuMapLeaveObjects(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DE:64C FileMenuMapLeaveList
 * @run 6F:AE4C
 * WHAT IT DOES: Draws the slot menu and starts list $13 for a slot with data, $12 without.
 * HOW: DrawSlotMenu, bit 0 of the slot's first byte, StartListChannel0Kind2, MenuStepAdvance.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: as MenuStepAdvance.
 */
MenuRegs FileMenuMapLeaveList(MenuRegs r)
{
    uint8_t slot, offset, list;

    jsr_push(0xAE4F);
    r = DrawSlotMenu(r);
    jsr_pop();
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    rx(&r, offset);
    rc(&r, (rd8((uint16_t)(SaveBuffer + offset)) & 0x01) != 0);
    list = r.c ? 0x13 : 0x12;
    r = start_list(r, list, 0xAE60);
    ra(&r, MenuStepAdvance());
    return r;
}

void abi_FileMenuMapLeaveList(void)
{
    MenuRegs r = FileMenuMapLeaveList(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DE:663 FileMenuMapLeaveDraw
 * @run 6F:AE63
 * WHAT IT DOES: Draws the slot page a screen to the left of the map and goes back to page 7: script $0E, the slot's
 * details, name and percentage, its objects and character picture, then objects $10 to $3F moved a screen left and the
 * scroll a screen right.
 * HOW: As the end of FileMenuSlideDraw for a slot, with frame $34 from $AEDF; DEC of x_hi over objects $10 to $3F, INC
 * of the scroll's high byte; SetMenuPage 7.
 * WAITS: no.
 * Entry: from MenuStepDispatch. Exit: A 0, N and Z from 7, X $40, the rest as the draws leave them.
 */
MenuRegs FileMenuMapLeaveDraw(MenuRegs r)
{
    uint8_t slot, row, offset, byte, kind, character, object;

    ra(&r, 0x0E);
    jsr_push(0xAE68);
    (void)StartScriptSlot0Kind1(0x0E);
    jsr_pop();
    ry(&r, 0x0E);
    rx(&r, 0);
    rnz(&r, 0x0E);
    jsr_push(0xAE6B);
    rexit(&r, DrawSlotDetails());
    jsr_pop();
    jsr_push(0xAE6E);
    r_arith(&r, DrawCursorRowDigit());
    jsr_pop();
    slot = RD8(ZP->save_slot);
    rc(&r, false);
    row = radd(&r, slot, 0x06);
    jsr_push(0xAE76);
    rexit(&r, DrawSlotName(row, r.x));
    jsr_pop();
    slot = RD8(ZP->save_slot);
    rc(&r, false);
    row = radd(&r, slot, 0x06);
    jsr_push(0xAE7E);
    rexit(&r, DrawSlotPercent(row, r.x));
    jsr_pop();
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    rx(&r, offset);
    byte = rd8((uint16_t)(SaveBuffer + offset));
    rc(&r, (byte & 0x01) != 0);
    kind = r.c ? 3 : 5;
    ra(&r, kind);
    jsr_push(0xAE90);
    rexit(&r, PlaceObjectList(kind));
    jsr_pop();
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    character = rd8((uint16_t)(MenuSlotCharacter + slot));
    WR8(OBJ->start_arg[0x33], character);
    WR8(OBJ->work_d[0x33], character);
    rx(&r, character);
    push8(character);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    byte = rd8((uint16_t)(SaveBuffer + offset));
    rx(&r, pull8());
    ra(&r, (uint8_t)(byte >> 1));
    rc(&r, (byte & 0x01) != 0);
    if (r.c) {
        byte = rd8((uint16_t)(MAP_OBJECT_FRAMES + r.x));
        ra(&r, byte);
        WR8(OBJ->frame[0x34], byte);
    }
    slot = RD8(ZP->save_slot);
    ry(&r, slot);
    offset = rd8((uint16_t)(SLOT_OFFSETS + slot));
    rx(&r, offset);
    byte = rd8((uint16_t)(SaveBuffer + offset));
    rc(&r, (byte & 0x01) != 0);
    byte >>= 1;
    ra(&r, byte);
    if (!r.c) {
        WR8(OBJ->frame[0x31], 0);
    }
    rc(&r, (byte & 0x01) != 0);
    byte >>= 1;
    ra(&r, byte);
    if (!r.c) {
        WR8(OBJ->frame[0x32], 0);
    }
    rc(&r, (byte & 0x01) != 0);
    byte >>= 1;
    ra(&r, byte);
    if (!r.c) {
        WR8(OBJ->frame[0x2F], 0);
    }
    rc(&r, (byte & 0x01) != 0);
    byte >>= 1;
    ra(&r, byte);
    if (!r.c) {
        WR8(OBJ->frame[0x30], 0);
    }
    for (object = 0x10; object != 0x40; object++) {
        DEC8(OBJ->x_hi[object]);
    }
    rx(&r, object);
    rcmp(&r, object, 0x40);
    rnz(&r, INC8(SCROLL->x_hi[0]));
    ra(&r, 0x07);
    SetMenuPage(0x07);
    ra(&r, 0);
    rnz(&r, 0x07);
    return r;
}

void abi_FileMenuMapLeaveDraw(void)
{
    MenuRegs r = FileMenuMapLeaveDraw(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00DE:59B FileMenuMapPage
 * @run 6F:AD9B
 * @stop ADA0
 * WHAT IT DOES: Page 12: the stage map.
 * HOW: zField80 in A and a JSR to MenuStepDispatch, the table after it.
 * WAITS: yes, in the steps that wipe the game state.
 * Entry: from the menu's page table. Exit: as the step leaves them.
 */
MenuRegs FileMenuMapPage(MenuRegs r)
{
    uint16_t target = menu_page(&r, 0xADA0);

    switch (target) {
    case 0xADAE: return FileMenuMapStart(r);
    case 0xADDB: return FileMenuMapDraw(r);
    case 0xADE1: return FileMenuMapStep(r);
    case 0xAE13: return FileMenuMapLeaveStart(r);
    case 0xAE2B: return FileMenuMapLeaveObjects(r);
    case 0xAE4C: return FileMenuMapLeaveList(r);
    case 0xAE63: return FileMenuMapLeaveDraw(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuMapPage(void)
{
    MenuRegs r = FileMenuMapPage(menu_regs_cpu());

    menu_regs_shim(&r);
}

/*
 * @routine 00C3:1E2 FileMenuFrame
 * @run 6B:91E2
 * @stop 9230
 * WHAT IT DOES: A frame of the file menu (mode 4): the menu's second overlay bank mapped; away from pages 0 and 12,
 * when object $26 shows, objects $26 and $27 given attribute $88 and the tick's frame, and likewise $3A to $3D when
 * $3A shows; then the handler of page zGameSubMode.
 * HOW: Bank 7 plus zRamBaseBank at MPR5; ObjectFrameFromTick; zGameSubMode in A and a JSR to MenuStepDispatch, the 17
 * pages' words after it.
 * WAITS: yes, where a page waits.
 * Entry: from the overlay's $800C. Exit: as the page leaves them.
 */
MenuRegs FileMenuFrame(MenuRegs r)
{
    uint8_t base, page, value, x, high;
    uint16_t target;
    unsigned total;

    base = RD8(ZP->ram_base_bank);
    total = (unsigned)7 + base;
    ra(&r, (uint8_t)total);
    rc(&r, total > 0xFF);
    rv(&r, ((7 ^ total) & (base ^ total) & 0x80) != 0);
    tam(0x20, (uint8_t)total);
    page = RD8(ZP->game_sub_mode);
    ra(&r, page);
    if (page != 0) {
        rcmp(&r, page, 0x0C);
        if (page != 0x0C) {
            value = RD8(OBJ->frame[0x26]);
            ra(&r, value);
            if (value != 0) {
                ra(&r, 0x88);
                WR8(OBJ->sprite_attr[0x26], 0x88);
                WR8(OBJ->sprite_attr[0x27], 0x88);
                for (x = 0x26; x != 0x28; x++) {
                    ValueCarry tick;

                    rx(&r, x);
                    jsr_push(x == 0x26 ? 0x9203 : 0x9207);
                    tick = ObjectFrameFromTick(x);
                    jsr_pop();
                    ra(&r, tick.value);
                    rc(&r, tick.carry);
                }
                value = RD8(OBJ->frame[0x3A]);
                ra(&r, value);
                if (value != 0) {
                    ra(&r, 0x88);
                    WR8(OBJ->sprite_attr[0x3A], 0x88);
                    WR8(OBJ->sprite_attr[0x3B], 0x88);
                    WR8(OBJ->sprite_attr[0x3C], 0x88);
                    WR8(OBJ->sprite_attr[0x3D], 0x88);
                    for (x = 0x3A; x != 0x3E; x++) {
                        ValueCarry tick;

                        rx(&r, x);
                        jsr_push((uint16_t)(0x921F + 4 * (x - 0x3A)));
                        tick = ObjectFrameFromTick(x);
                        jsr_pop();
                        ra(&r, tick.value);
                        rc(&r, tick.carry);
                    }
                }
            }
        }
    }
    page = RD8(ZP->game_sub_mode);
    ra(&r, page);
    jsr_push(0x9230);
    target = MenuStepDispatch(page, &high);
    ra(&r, high);
    r.x = (uint8_t)((uint8_t)(page << 1) + 1);
    r.set |= REG_X;
    rc(&r, (page & 0x80) != 0);
    switch (target) {
    case 0x9255: return FileMenuReadSavePage(r);
    case 0x9C24: return FileMenuOpenPage(r);
    case 0x9930: return FileMenuSlidePage(r);
    case 0x9327: return FileMenuPickEmpty(r);
    case 0x933A: return FileMenuPickUsed(r);
    case 0x9341: return FileMenuErasePage(r);
    case 0x93D2: return FileMenuCopyPage(r);
    case 0x94B2: return FileMenuCharacterPage(r);
    case 0x9B37: return FileMenuBackToStartPage(r);
    case 0x9858: return FileMenuListPage(r);
    case 0x9B07: return FileMenuBackPage(r);
    case 0x958E: return FileMenuNamePage(r);
    case 0xAD9B: return FileMenuMapPage(r);
    case 0xBD14: return FileMenuNoticePage(r);
    case 0xB68B: return MusicSelectStep(r);
    case 0xB712: return FileMenuShopPage(r);
    case 0xBB1D: return FileMenuLeaveStep(r);
    default: return menu_elsewhere(r, target);
    }
}

void abi_FileMenuFrame(void)
{
    MenuRegs r = FileMenuFrame(menu_regs_cpu());

    menu_regs_shim(&r);
}
