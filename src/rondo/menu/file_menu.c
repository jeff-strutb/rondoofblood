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
typedef enum { REPLAY_RESET, REPLAY_BACKUP_READ, REPLAY_BACKUP_WRITE, REPLAY_FORMAT, REPLAY_TRACK, REPLAY_SOUND } ReplayKind;
typedef struct {
    ReplayKind kind;
    uint8_t set;                    /* the registers set before this callee, in a, x, ... of the record */
    uint8_t a, x, y, nz;
    bool c, v;
    uint8_t command;
    union { BackupAccess backup; BackupWriteRun write; BackupFormat format; TrackPlay track; CommandCall sound; } call;
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
#define SLOT_MARK 0x7DDD                             /* a byte the menu keeps for each slot, by slot 0 to 2: not settled */
#define SLOT_MARK_BY_NUMBER (SLOT_MARK - 1)
#define ERASE_TIMER 0x7DFE                           /* frames left before a slot is erased */
#define SLOT_BYTES 0x30
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
    wr8(SLOT_MARK, 0);
    wr8(SLOT_MARK + 1, 0);
    wr8(SLOT_MARK + 2, 0);
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
    value = rd8((uint16_t)(SLOT_MARK_BY_NUMBER + from));
    wr8((uint16_t)(SLOT_MARK_BY_NUMBER + to), value);
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
 * set and clear SLOT_MARK's first byte and go to page 9, $13 starts the game: Maria on a slot with no progress is
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
            wr8(SLOT_MARK - 1, 1);
            SetMenuPage(0x09);
            ra(&r, 0);
            rnz(&r, 0x09);
            return r;
        }
        rcmp(&r, item, 0x17);
        if (item == 0x17) {
            wr8(SLOT_MARK - 1, 0);
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
 * the entries cleared.
 */
#define RECORD_ENTRIES 14
#define MARK_TEXTS 0xAA7B                   /* a string pointer for each value of SLOT_MARK */
#define RECORD_SCORES 0xAD01                /* a two-byte BCD score for each entry, and one more at index 14 */
#define ENTRY_OFFSETS 0xB90A                /* the flag byte pair of each entry, from the slot's byte $01 */
#define ENTRY_MASKS 0xB917                  /* its bit there */
#define CLEARED_MASKS 0xB993                /* the entry's bit in the slot's byte $22 (entries 0 to 7) or $23 */
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
 * WHAT IT DOES: Draws the selected slot's SLOT_MARK plus one as a BCD number at $02C4, and the mark's string.
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
    mark = rd8((uint16_t)(SLOT_MARK + slot));
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
    mark = rd8((uint16_t)(SLOT_MARK + slot));
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
 * HOW: The entry number in zTemp (bios_b low); the slot's cleared bit tested with CLEARED_MASKS; PHX, PLX around it;
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
    bits = (uint8_t)(rd8((uint16_t)(SaveBuffer + 0x22 + offset)) & rd8((uint16_t)(CLEARED_MASKS + entry)));
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
 * cleared bit for it is set and $41 when not. Then the score line (DrawSlotRecord) with the X the last entry left.
 * HOW: The entry counted in the card's zero page word A (bios_a low); SlotEntryTest; CLEARED_MASKS against bytes $22
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
                bits = (uint8_t)(rd8((uint16_t)(SaveBuffer + 0x22 + offset)) & rd8((uint16_t)(CLEARED_MASKS + r.x)));
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
        list = radd(&r, 0x1A, rd8(SLOT_MARK - 1));
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
        value = rd8(SLOT_MARK - 1);
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
        value = rd8(SLOT_MARK - 1);
        ra(&r, value);
        if (value == 0) {
            ra(&r, 0x1C);
            jsr_push(0x991E);
            r_cursor(&r, SetCursorValue(0x1C));
            jsr_pop();
            wr8(0x7DDA, 0);
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
        flag = rd8(SLOT_MARK - 1);
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
