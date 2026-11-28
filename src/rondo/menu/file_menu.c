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
