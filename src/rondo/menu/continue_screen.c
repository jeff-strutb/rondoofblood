/*
 * The continue screen: the program at the start of the sound bank (sector 004D, bank $75 at $8000), which ModeDead
 * maps into MPR4 and calls once a frame. By zGameSubMode it sets the screen up, spells the character's message in
 * letters that step or turn into place, fades the palette out, clears the stage's objects and runs the menu of
 * ContinueChoice. Richter's and Maria's first two steps differ; the rest are shared.
 *
 * The letters: each is eight rows of a 64-word tile pattern in the bank mapped at $A000 (Richter's from $B754, Maria's
 * from $BB54, $80 bytes a letter), written to VRAM from $6E00, $40 words a letter. Maria's letters turn into place: each
 * frame a letter's pattern is copied into StageMaskBuffer as two planes of eight 16-byte rows, stretched about its
 * middle column by the letter's step count, its 8 by 8 tiles transposed before and after, and written out.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"

/* Defined elsewhere, typed, and not declared in routines.h. */
void ClearEnemySlots(uint8_t entry_x);              /* object/lifecycle.c */

enum {
    MPR5 = 0x20, MPR6 = 0x40,
    JOY_I = 0x01, JOY_RUN = 0x08, JOY_UP = 0x10, JOY_MOVES = 0x54, JOY_CHOOSE = 0x09,
    LETTERS = 8, LETTER_LAST = 7, LETTER_DONE_RICHTER = 0x11, LETTER_LAST_STEP = 0x10, LETTER_DONE_MARIA = 0x20,
    LETTER_VRAM_HI = 0x6E, LETTER_BANK_OFFSET = 0x10, MESSAGE_ROWS = 8,
    RICHTER_LETTERS_LO = 0x54, RICHTER_LETTERS_HI = 0xB7, MARIA_LETTERS_LO = 0x54, MARIA_LETTERS_HI = 0xBB,
    LETTER_FIRST_FRAME = 0x0D, LETTER_SPRITE_SET = 5, RICHTER_LETTER_X = 0x48, RICHTER_LETTER_Y = 0x59,
    MARIA_LETTER_Y = 0x68, MESSAGE_SCRIPT = 0x0D, START_SOUND = 0x61,
    PLANE_HI = 0x00, PLANE_LO = 0x10, PLANE_ROW = 0x10, PLANE_BYTES = 0x80, TILE_ROWS = 8,
    FADE_STEPS = 9, FADE_BANK = 0x12, FADE_BANK_IDLE = 0x30, PALETTE_HI = 0xB0, PALETTE_END_HI = 0xB4,
    PALETTE_SKIP_LO = 0xA0, PALETTE_SKIP_HI = 0xB2, PALETTE_ROW = 0x20, FADE_WAIT = 4,
    CURSOR = 8, CURSOR_FRAME = 4, CURSOR_SPRITE_SET = 3, CURSOR_X = 0x4F, MENU_LIST = 0x6C, MENU_SCRIPT = 0x0E,
    THIRD_CHOICE_VRAM_LO = 0x40, THIRD_CHOICE_VRAM_HI = 0x05, THIRD_CHOICE_WORDS = 0x20, BLANK_LO = 0x10, BLANK_HI = 0x01,
    MOVE_SOUND = 0x01, SOUND_STOP = 0xFF, BLINK_STEPS = 0x0A
};

/* The registers and flags after `after`, where `after` keeps what `before` left. */
static Exit over(Exit before, Exit after)
{
    Exit e = after;

    if (after.a_kept) {
        e.a = before.a;
        e.a_kept = before.a_kept;
    }
    if (after.x_kept) {
        e.x = before.x;
        e.x_kept = before.x_kept;
    }
    if (after.y_kept) {
        e.y = before.y;
        e.y_kept = before.y_kept;
    }
    if (after.c_kept) {
        e.c = before.c;
        e.c_kept = before.c_kept;
    }
    if (!after.v_set) {
        e.v = before.v;
        e.v_set = before.v_set;
    }
    return e;
}

/* Nothing changed but N and Z. */
static Exit kept_nz(uint8_t nz)
{
    Exit e = { 0 };

    e.nz = nz;
    e.a_kept = e.x_kept = e.y_kept = e.c_kept = true;
    return e;
}

static Exit with_c(Exit e, bool c)
{
    e.c = c;
    e.c_kept = false;
    return e;
}

static Exit with_v(Exit e, bool v)
{
    e.v = v;
    e.v_set = true;
    return e;
}

static Exit with_a(Exit e, uint8_t a)
{
    e.a = a;
    e.a_kept = false;
    return e;
}

static Exit with_x(Exit e, uint8_t x)
{
    e.x = x;
    e.x_kept = false;
    return e;
}

static Exit with_y(Exit e, uint8_t y)
{
    e.y = y;
    e.y_kept = false;
    return e;
}

/* A byte of the work buffer the letters are turned in: plane PLANE_HI holds the high bytes, PLANE_LO the low. */
static uint16_t plane(uint8_t index)
{
    return ADDR_OF(STAGEWORK->mask_buffer[index]);
}

/* The work bytes zTemp08 to zTemp0F, as LDA, ROL and ROR <$08,X reach them. */
static uint16_t column_byte(uint8_t x)
{
    return (uint16_t)(0x2000 | (uint8_t)(ADDR_OF(ZP->temp08) + x));
}

/*
 * @routine 004D:4DB SetVramAddressXY
 * @run 75:84DB
 * WHAT IT DOES: Points the VDC's VRAM writes at an address and selects the data port.
 * HOW: SelectVramWriteAddress, the address's two bytes to the data ports, then the tail JMP to SelectVramData.
 * WAITS: no.
 * Entry: X the address's low byte, Y its high byte. Exit: A 2, N 0, Z 0. X, Y, C and V kept.
 */
Exit SetVramAddressXY(uint8_t lo, uint8_t hi)
{
    jsr_push(0x84DE);
    SelectVramWriteAddress();
    jsr_pop();
    WR8(VDC->data_lo, lo);
    WR8(VDC->data_hi, hi);
    SelectVramData();
    return with_a(kept_nz(2), 2);
}

void abi_SetVramAddressXY(void)
{
    exit_shim(SetVramAddressXY(cpu.x, cpu.y));
}

/*
 * @routine 004D:4C8 ClearVramRows
 * @run 75:84C8
 * WHAT IT DOES: Clears VRAM from an address, $40 zero words a row.
 * HOW: The row count in zTemp00; SetVramAddressXY; ST1 and ST2 of zero 64 times a row; DEC zTemp00 until it is zero
 * (a count of 0 clears 256 rows).
 * WAITS: no.
 * Entry: A rows, X the address's low byte, Y its high byte. Exit: A 2, X 0, N 0, Z 1. Y, C and V kept.
 */
Exit ClearVramRows(uint8_t rows, uint8_t lo, uint8_t hi)
{
    WR8(ZP->temp00, rows);
    jsr_push(0x84CD);
    SetVramAddressXY(lo, hi);
    jsr_pop();
    do {
        for (uint8_t words = 0x40; words != 0; words--) {
            st1(0x00);
            st2(0x00);
        }
    } while (DEC8(ZP->temp00) != 0);
    return with_x(with_a(kept_nz(0), 2), 0);
}

void abi_ClearVramRows(void)
{
    exit_shim(ClearVramRows(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 004D:026 ContinueBegin
 * @run 75:8026
 * WHAT IT DOES: The continue screen's first step for either character: its music, the stage value cleared, the
 * letters' VRAM ($6E00, eight rows) cleared, and zTemp10 set to the last letter.
 * HOW: SoundCommand $61, STZ zRasterStage, ClearVramRows, zTemp10 7.
 * WAITS: no.
 * Entry: X and Y the caller's (SoundCommand is called with them). Exit: A 7, X 0, Y $6E, N and Z from 7; C and V as
 * SoundCommand left them.
 */
CommandCall ContinueBegin(uint8_t kept_x, uint8_t kept_y)
{
    CommandCall sound;

    jsr_push(0x802B);
    sound = SoundCommand(START_SOUND, kept_x, kept_y);
    jsr_pop();
    WR8(ZP->raster_stage, 0);
    jsr_push(0x8036);
    ClearVramRows(MESSAGE_ROWS, 0x00, LETTER_VRAM_HI);
    jsr_pop();
    WR8(ZP->temp10, LETTER_LAST);
    return sound;
}

void abi_ContinueBegin(void)
{
    sound_command_exit_shim(START_SOUND, ContinueBegin(cpu.x, cpu.y));
    cpu.a = LETTER_LAST;
    cpu.x = 0;
    cpu.y = LETTER_VRAM_HI;
    abi_nz(LETTER_LAST);
}

/*
 * @routine 004D:11F LetterPlace
 * @run 75:811F
 * WHAT IT DOES: Finds a letter's places: its VRAM address in zTemp00 and zTemp01 ($6E00 plus $40 words a letter) and
 * its pattern's offset in zTemp08 and zTemp09 ($80 bytes a letter), and maps the letters' bank pair ($10 above the
 * RAM base bank) into MPR5 and MPR6.
 * HOW: The letter's bits shifted into the low bytes with LSR and ROR; the high bytes added.
 * WAITS: no.
 * Entry: X the letter. Exit: A the letter halved, N and Z from zTemp08, C 0, V from the bank add. X and Y kept.
 */
Exit LetterPlace(uint8_t letter)
{
    uint8_t a;
    bool c;
    Sum sum, bank;

    WR8(ZP->temp00, 0);
    a = (uint8_t)(letter >> 1);
    ROR8(ZP->temp00, (letter & 0x01) != 0);
    c = (a & 0x01) != 0;
    a = (uint8_t)(a >> 1);
    ROR8(ZP->temp00, c);
    WR8(ZP->temp01, a);
    sum = add_bytes(0x00, RD8(ZP->temp00), false);
    WR8(ZP->temp00, sum.value);
    sum = add_bytes(LETTER_VRAM_HI, RD8(ZP->temp01), sum.carry);
    WR8(ZP->temp01, sum.value);
    bank = add_bytes(LETTER_BANK_OFFSET, RD8(ZP->ram_base_bank), false);
    tam(MPR5, bank.value);
    tam(MPR6, (uint8_t)(bank.value + 1));
    WR8(ZP->temp08, 0);
    a = (uint8_t)(letter >> 1);
    ROR8(ZP->temp08, (letter & 0x01) != 0);
    WR8(ZP->temp09, a);
    return with_v(with_c(with_a(kept_nz((uint8_t)((letter & 0x01) << 7)), a), false), bank.overflow);
}

void abi_LetterPlace(void)
{
    exit_shim(LetterPlace(cpu.x));
}

/* LDA #lo, CLC, ADC zTemp08, STA; LDA #hi, ADC zTemp09, STA: the pattern's address. V from the high add. */
static bool letter_pattern_base(uint8_t lo, uint8_t hi)
{
    Sum sum;

    sum = add_bytes(lo, RD8(ZP->temp08), false);
    WR8(ZP->temp08, sum.value);
    sum = add_bytes(hi, RD8(ZP->temp09), sum.carry);
    WR8(ZP->temp09, sum.value);
    return sum.overflow;
}

/*
 * @routine 004D:0F2 WriteLetterColumn
 * @run 75:80F2
 * WHAT IT DOES: Writes one word of a stepping letter: at the letter's VRAM address plus the column less its step
 * count, the word of its pattern at that index.
 * HOW: The column in zTemp10; SelectVramWriteAddress, the address (the low byte added, the high byte as it is);
 * SelectVramData; the word read through zTemp08, low byte then high.
 * WAITS: no.
 * Entry: A the column, X the letter. Exit: A the word's high byte, N and Z from it; Y the index of that byte; C the
 * index's carry out of the doubling, V from the second subtraction. X kept.
 */
Exit WriteLetterColumn(uint8_t column, uint8_t letter)
{
    uint8_t value, index, place, timer;
    Alu step;
    Sum sum;

    WR8(ZP->temp10, column);
    jsr_push(0x80F7);
    SelectVramWriteAddress();
    jsr_pop();
    place = RD8(ZP->temp00);
    sum = add_bytes(place, RD8(ZP->temp10), false);
    timer = RD8(OBJ->life_timer[letter]);
    step = sbc8(sum.value, timer, true);
    WR8(VDC->data_lo, step.a);
    value = RD8(ZP->temp01);
    WR8(VDC->data_hi, value);
    jsr_push(0x810B);
    SelectVramData();
    jsr_pop();
    value = RD8(ZP->temp10);
    step = sbc8(value, RD8(OBJ->life_timer[letter]), true);
    index = (uint8_t)(step.a << 1);
    value = RD8_PTR_Y(ZP->temp08, index);
    WR8(VDC->data_lo, value);
    index++;
    value = RD8_PTR_Y(ZP->temp08, index);
    WR8(VDC->data_hi, value);
    return with_v(with_c(with_y(with_a(kept_nz(value), value), index), (step.a & 0x80) != 0), step.v);
}

void abi_WriteLetterColumn(void)
{
    exit_shim(WriteLetterColumn(cpu.a, cpu.x));
}

/*
 * @routine 004D:0A9 RichterLetterStep
 * @run 75:80A9
 * WHAT IT DOES: One frame of one of Richter's letters. A letter not started waits out its step count, moving 8
 * pixels right a step, and starts at zero; a started one, until 16 steps, writes four words of its pattern (columns
 * $0F, $1F, $2F, $3F less the steps), moves a pixel down and counts a step.
 * HOW: OBJ state, life_timer, x_lo and y_lo of the letter's slot; LetterPlace; the pattern from $B754 in the letters'
 * bank; WriteLetterColumn four times.
 * WAITS: no.
 * Entry: X the letter. Exit: not started, count zero: A 0, N and Z from the state's INC; counting: A the new X, C and
 * V from its add, N and Z from the count's DEC; started, 16 steps or more: A the count, N, Z and C from CMP #$10;
 * writing: as the last WriteLetterColumn, N and Z from the count's INC. X kept; Y kept but where it writes.
 */
Exit RichterLetterStep(uint8_t letter)
{
    uint8_t state, count;
    Exit e;
    Sum sum;

    state = RD8(OBJ->state[letter]);
    if (state == 0) {
        count = RD8(OBJ->life_timer[letter]);
        if (count == 0) {
            return with_a(kept_nz(INC8(OBJ->state[letter])), 0);
        }
        sum = add_bytes(RD8(OBJ->x_lo[letter]), 8, false);
        WR8(OBJ->x_lo[letter], sum.value);
        return with_v(with_c(with_a(kept_nz(DEC8(OBJ->life_timer[letter])), sum.value), sum.carry), sum.overflow);
    }
    count = RD8(OBJ->life_timer[letter]);
    if (count >= LETTER_LAST_STEP) {
        return with_c(with_a(kept_nz((uint8_t)(count - LETTER_LAST_STEP)), count), true);
    }
    jsr_push(0x80CE);
    LetterPlace(letter);
    jsr_pop();
    letter_pattern_base(RICHTER_LETTERS_LO, RICHTER_LETTERS_HI);
    jsr_push(0x80DF);
    WriteLetterColumn(0x0F, letter);
    jsr_pop();
    jsr_push(0x80E3);
    WriteLetterColumn(0x1F, letter);
    jsr_pop();
    jsr_push(0x80E7);
    WriteLetterColumn(0x2F, letter);
    jsr_pop();
    jsr_push(0x80EB);
    e = WriteLetterColumn(0x3F, letter);
    jsr_pop();
    INC8(OBJ->y_lo[letter]);
    e.nz = INC8(OBJ->life_timer[letter]);
    return e;
}

void abi_RichterLetterStep(void)
{
    exit_shim(RichterLetterStep(cpu.x));
}

/* A set-up's end: A the timer's last value, X 0 and Y $0D from the script start, C 0 and V 0 from letter 0's add. */
static Exit start_exit(uint8_t nz, uint8_t a)
{
    return with_v(with_c(with_y(with_x(with_a(kept_nz(nz), a), 0), MESSAGE_SCRIPT), false), false);
}

/* LDA #1, STA OBJ angle_hi of slot 0; LDA #wait, STA its f3510: the screen's timer, in frames and in rounds. */
static void set_screen_timer(uint8_t wait)
{
    WR8(OBJ->angle_hi[0], 1);
    WR8(OBJ->f3510[0], wait);
}

/*
 * @routine 004D:03B ContinueStartRichter
 * @run 75:803B
 * WHAT IT DOES: Richter's set-up: ContinueBegin, then his eight letters as objects (frame $0D up, sprite set 5, at
 * X $48, Y $59), each letter's step count twice its number, the message script started, the screen's timer set ($C0
 * frames, one round) and the next sub-mode.
 * HOW: zTemp10 counts the letters down from 7 and is read again for each; ClearObject; StartScriptSlot0Kind1.
 * WAITS: no.
 * Entry: X and Y the dispatch's (for ContinueBegin). Exit: A $C0, X 0, Y $0D, N and Z from the sub-mode's INC, C 0, V 0.
 */
Exit ContinueStartRichter(uint8_t kept_x, uint8_t kept_y)
{
    uint8_t letter;

    jsr_push(0x803E);
    ContinueBegin(kept_x, kept_y);
    jsr_pop();
    do {
        letter = RD8(ZP->temp10);
        jsr_push(0x8043);
        ClearObject(letter);
        jsr_pop();
        WR8(OBJ->life_timer[letter], (uint8_t)(letter << 1));
        WR8(OBJ->frame[letter], (uint8_t)(letter + LETTER_FIRST_FRAME));
        WR8(OBJ->sprite_set[letter], LETTER_SPRITE_SET);
        WR8(OBJ->x_lo[letter], RICHTER_LETTER_X);
        WR8(OBJ->y_lo[letter], RICHTER_LETTER_Y);
    } while (!(DEC8(ZP->temp10) & 0x80));
    jsr_push(0x8067);
    StartScriptSlot0Kind1(MESSAGE_SCRIPT);
    jsr_pop();
    set_screen_timer(0xC0);
    return start_exit(INC8(ZP->game_sub_mode), 0xC0);
}

void abi_ContinueStartRichter(void)
{
    exit_shim(ContinueStartRichter(cpu.x, cpu.y));
}

/*
 * The screen's timer, from $8087: f3510 of slot 0 counted down, then angle_hi; when both run out, the next sub-mode
 * as RUN gives it. at: the registers as the code that went there left them.
 */
static Exit continue_count_down(Exit at)
{
    uint8_t left;

    left = DEC8(OBJ->f3510[0]);
    if (left != 0) {
        return over(at, kept_nz(left));
    }
    left = DEC8(OBJ->angle_hi[0]);
    if (left != 0) {
        return over(at, kept_nz(left));
    }
    WR8(ZP->field80, 0);
    return over(at, kept_nz(INC8(ZP->game_sub_mode)));
}

/*
 * @routine 004D:074 ContinueWaitRichter
 * @run 75:8074
 * WHAT IT DOES: Richter's message, a frame: RUN once the last letter is done goes on at once; otherwise every letter
 * steps, the last one's count goes from $10 to $11 (done), and once it is done the screen's timer runs (at $8087,
 * which Maria's step shares); its end goes on.
 * HOW: BiosJoyTrigger's RUN bit and the last letter's life_timer; RichterLetterStep for letters 7 down to 0.
 * WAITS: no.
 * Entry: nothing. Exit: on: A $11, C 1, N and Z from the sub-mode's INC, X, Y and V kept; else X $FF and A, Y, C and V
 * as the last letter's step and what followed leave them (the count's compare, its INC or the timer's DEC).
 */
Exit ContinueWaitRichter(void)
{
    Exit e;
    uint8_t pressed, last;
    int8_t letter;

    pressed = (uint8_t)(rd8(BiosJoyTrigger) & JOY_RUN);
    e = with_a(kept_nz(pressed), pressed);
    if (pressed != 0) {
        last = RD8(OBJ->life_timer[LETTER_LAST]);
        e = with_c(with_a(kept_nz((uint8_t)(last - LETTER_DONE_RICHTER)), last), last >= LETTER_DONE_RICHTER);
        if (last == LETTER_DONE_RICHTER) {
            WR8(ZP->field80, 0);
            e.nz = INC8(ZP->game_sub_mode);
            return e;
        }
    }
    for (letter = LETTER_LAST; letter >= 0; letter--) {
        jsr_push(0x8097);
        e = over(e, RichterLetterStep((uint8_t)letter));
        jsr_pop();
    }
    e = with_x(e, 0xFF);
    last = RD8(OBJ->life_timer[LETTER_LAST]);
    e = with_a(e, last);
    if (last == LETTER_DONE_RICHTER) {
        return continue_count_down(with_c(e, true));
    }
    e = with_c(e, last >= LETTER_LAST_STEP);
    if (last != LETTER_LAST_STEP) {
        e.nz = (uint8_t)(last - LETTER_LAST_STEP);
        return e;
    }
    e.nz = INC8(OBJ->life_timer[LETTER_LAST]);
    return e;
}

void abi_ContinueWaitRichter(void)
{
    exit_shim(ContinueWaitRichter());
}

/*
 * @routine 004D:14A ContinueStartMaria
 * @run 75:814A
 * WHAT IT DOES: Maria's set-up: ContinueBegin, then her eight letters as objects (frame $0D up, sprite set 5, 16
 * pixels apart from X $48, at Y $68), the message script started, the screen's timer set ($10 frames, one round),
 * the first letter started and the next sub-mode.
 * HOW: zTemp10 counts the letters down from 7 and is read again for each; ClearObject; StartScriptSlot0Kind1.
 * WAITS: no.
 * Entry: X and Y the dispatch's (for ContinueBegin). Exit: A $10, X 0, Y $0D, N and Z from the sub-mode's INC, C 0, V 0.
 */
Exit ContinueStartMaria(uint8_t kept_x, uint8_t kept_y)
{
    uint8_t letter;

    jsr_push(0x814D);
    ContinueBegin(kept_x, kept_y);
    jsr_pop();
    do {
        letter = RD8(ZP->temp10);
        jsr_push(0x8152);
        ClearObject(letter);
        jsr_pop();
        WR8(OBJ->frame[letter], (uint8_t)(letter + LETTER_FIRST_FRAME));
        WR8(OBJ->x_lo[letter], (uint8_t)((uint8_t)(letter << 4) + RICHTER_LETTER_X));
        WR8(OBJ->sprite_set[letter], LETTER_SPRITE_SET);
        WR8(OBJ->y_lo[letter], MARIA_LETTER_Y);
    } while (!(DEC8(ZP->temp10) & 0x80));
    jsr_push(0x8177);
    StartScriptSlot0Kind1(MESSAGE_SCRIPT);
    jsr_pop();
    set_screen_timer(0x10);
    INC8(OBJ->state[0]);
    return start_exit(INC8(ZP->game_sub_mode), 0x10);
}

void abi_ContinueStartMaria(void)
{
    exit_shim(ContinueStartMaria(cpu.x, cpu.y));
}

/*
 * @routine 004D:4A0 ContinueCursorBlink
 * @run 75:84A0
 * WHAT IT DOES: Every fourth frame, starts the cursor's next blink script (0 to 5 and back, ten steps) in script slot 0.
 * HOW: zFrameTick's low two bits; the step in the cursor's life_timer (slot 8) indexes ContinueBlinkScripts;
 * StartScriptSlotKind4; the step counted and wrapped at 10.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: off frames: A zFrameTick's low bits, N and Z from them, X, Y, C and V kept; else A
 * the step, N, Z and C from CMP #$0A, X 0, Y the script, V kept.
 */
Exit ContinueCursorBlink(void)
{
    uint8_t tick, step, script;

    tick = (uint8_t)(RD8(ZP->frame_tick) & 0x03);
    if (tick != 0) {
        return with_a(kept_nz(tick), tick);
    }
    step = RD8(OBJ->life_timer[CURSOR]);
    script = rd8((uint16_t)(ContinueBlinkScripts + step));
    jsr_push(0x84B0);
    StartScriptSlotKind4(script, 0);
    jsr_pop();
    INC8(OBJ->life_timer[CURSOR]);
    step = RD8(OBJ->life_timer[CURSOR]);
    if (step >= BLINK_STEPS) {
        WR8(OBJ->life_timer[CURSOR], 0);
    }
    return with_c(with_y(with_x(with_a(kept_nz((uint8_t)(step - BLINK_STEPS)), step), 0), script),
                  step >= BLINK_STEPS);
}

void abi_ContinueCursorBlink(void)
{
    exit_shim(ContinueCursorBlink());
}

/*
 * @routine 004D:323 StoreTileRows
 * @run 75:8323
 * WHAT IT DOES: Puts the eight bytes zTemp08 to zTemp0F back into StageMaskBuffer from Y on.
 * HOW: LDA <$08,X and STA abs,Y for X 0 to 7.
 * WAITS: no.
 * Entry: Y the first row. Exit: A zTemp0F, X 8, Y eight on, N 0, Z 1, C 1. V kept.
 */
Exit StoreTileRows(uint8_t y)
{
    uint8_t x, value = 0;

    for (x = 0; x != TILE_ROWS; x++) {
        value = rd8(column_byte(x));
        wr8(plane(y), value);
        y++;
    }
    return with_c(with_y(with_x(with_a(kept_nz(0), value), x), y), true);
}

void abi_StoreTileRows(void)
{
    exit_shim(StoreTileRows(cpu.y));
}

/*
 * @routine 004D:2EE TransposeTile
 * @run 75:82EE
 * WHAT IT DOES: Transposes the 8 by 8 bit tile of StageMaskBuffer at Y: row r's bit 7 - c goes to row c's bit r.
 * HOW: Each row shifted out with ASL into zTemp08 to zTemp0F by ROL <$08,X, X up; StoreTileRows puts them back.
 * zTransposeRowsLeft counts the rows; Y is kept on the stack.
 * WAITS: no.
 * Entry: Y the first row. Exit: as StoreTileRows.
 */
Exit TransposeTile(uint8_t y)
{
    uint8_t first = y, x, row;
    bool c;

    push8(first);
    WR8(ZP->transpose_rows_left, TILE_ROWS);
    do {
        x = 0;
        row = rd8(plane(y));
        do {
            c = (row & 0x80) != 0;
            row = (uint8_t)(row << 1);
            rol_mem(column_byte(x), c);
            x++;
        } while (x != TILE_ROWS);
        y++;
    } while (DEC8(ZP->transpose_rows_left) != 0);
    y = pull8();
    jsr_push(0x8308);
    {
        Exit e = StoreTileRows(y);

        jsr_pop();
        return e;
    }
}

void abi_TransposeTile(void)
{
    exit_shim(TransposeTile(cpu.y));
}

/*
 * @routine 004D:309 TransposeTileMirrored
 * @run 75:8309
 * WHAT IT DOES: As TransposeTile, the bits gathered the other way: row r's bit 7 - c goes to row 7 - c's bit 7 - r.
 * HOW: Each row shifted out with ASL into zTemp0F down to zTemp08 by ROR <$08,X, X down; StoreTileRows.
 * WAITS: no.
 * Entry: Y the first row. Exit: as StoreTileRows.
 */
Exit TransposeTileMirrored(uint8_t y)
{
    uint8_t first = y, row;
    int8_t x;
    bool c;

    push8(first);
    WR8(ZP->transpose_rows_left, TILE_ROWS);
    do {
        x = TILE_ROWS - 1;
        row = rd8(plane(y));
        do {
            c = (row & 0x80) != 0;
            row = (uint8_t)(row << 1);
            ror_mem(column_byte((uint8_t)x), c);
            x--;
        } while (x >= 0);
        y++;
    } while (DEC8(ZP->transpose_rows_left) != 0);
    y = pull8();
    jsr_push(0x8322);
    {
        Exit e = StoreTileRows(y);

        jsr_pop();
        return e;
    }
}

void abi_TransposeTileMirrored(void)
{
    exit_shim(TransposeTileMirrored(cpu.y));
}

/*
 * @routine 004D:2E2 TransposeTileGroup
 * @run 75:82E2
 * WHAT IT DOES: Transposes four tiles from Y: the first and last straight, the middle two mirrored.
 * HOW: TransposeTile, TransposeTileMirrored twice, then the tail JMP to TransposeTile. Each leaves Y eight on.
 * WAITS: no.
 * Entry: Y the first row. Exit: as StoreTileRows, Y 32 on.
 */
Exit TransposeTileGroup(uint8_t y)
{
    jsr_push(0x82E5);
    y = TransposeTile(y).y;
    jsr_pop();
    jsr_push(0x82E8);
    y = TransposeTileMirrored(y).y;
    jsr_pop();
    jsr_push(0x82EB);
    y = TransposeTileMirrored(y).y;
    jsr_pop();
    return TransposeTile(y);
}

void abi_TransposeTileGroup(void)
{
    exit_shim(TransposeTileGroup(cpu.y));
}

/*
 * @routine 004D:2D8 TransposeLetterTiles
 * @run 75:82D8
 * WHAT IT DOES: Transposes all sixteen tiles of StageMaskBuffer, four groups of four.
 * HOW: Y 0; TransposeTileGroup three times by JSR and the fourth by running into it.
 * WAITS: no.
 * Entry: nothing. Exit: A zTemp0F, X 8, Y $80, N 0, Z 1, C 1. V kept.
 */
Exit TransposeLetterTiles(void)
{
    uint8_t y = 0;

    jsr_push(0x82DC);
    y = TransposeTileGroup(y).y;
    jsr_pop();
    jsr_push(0x82DF);
    y = TransposeTileGroup(y).y;
    jsr_pop();
    jsr_push(0x82E2);
    y = TransposeTileGroup(y).y;
    jsr_pop();
    return TransposeTileGroup(y);
}

void abi_TransposeLetterTiles(void)
{
    exit_shim(TransposeLetterTiles());
}

/*
 * @routine 004D:2A4 ClearLetterPlaneByte
 * @run 75:82A4
 * WHAT IT DOES: Clears one byte of StageMaskBuffer and moves X a row (16) on.
 * HOW: STZ abs,X; SAX, CLC, ADC #$10, SAX.
 * WAITS: no.
 * Entry: X the byte. Exit: X 16 on, N, Z, C and V from that add. A and Y kept.
 */
Exit ClearLetterPlaneByte(uint8_t x)
{
    Sum sum;

    wr8(plane(x), 0);
    sum = add_bytes(x, PLANE_ROW, false);
    return with_v(with_c(with_x(kept_nz(sum.value), sum.value), sum.carry), sum.overflow);
}

void abi_ClearLetterPlaneByte(void)
{
    exit_shim(ClearLetterPlaneByte(cpu.x));
}

/*
 * @routine 004D:2AD CopyLetterPlaneColumn
 * @run 75:82AD
 * WHAT IT DOES: Copies a column of StageMaskBuffer's eight 16-byte rows to another column.
 * HOW: X and Y kept on the stack; zPlaneRowsLeft counts the rows; both move a row with CLC, ADC #$10.
 * WAITS: no.
 * Entry: X the source column, Y the destination's. Exit: A Y plus $80, C and V from the last add, N and Z from X. X
 * and Y kept.
 */
Exit CopyLetterPlaneColumn(uint8_t x, uint8_t y)
{
    uint8_t value;
    Sum sx, sy;

    push8(x);
    push8(y);
    WR8(ZP->plane_rows_left, TILE_ROWS);
    do {
        value = rd8(plane(x));
        wr8(plane(y), value);
        sx = add_bytes(x, PLANE_ROW, false);
        x = sx.value;
        sy = add_bytes(y, PLANE_ROW, false);
        y = sy.value;
    } while (DEC8(ZP->plane_rows_left) != 0);
    y = pull8();
    x = pull8();
    return with_v(with_c(with_y(with_x(with_a(kept_nz(x), sy.value), x), y), sy.carry), sy.overflow);
}

void abi_CopyLetterPlaneColumn(void)
{
    exit_shim(CopyLetterPlaneColumn(cpu.x, cpu.y));
}

/*
 * @routine 004D:260 StretchLetterPlanes
 * @run 75:8260
 * WHAT IT DOES: Stretches the letter in StageMaskBuffer out from its middle: for columns 7 down to 0, a running sum
 * of the step that carries copies the column to the next one out (and its mirror, 15 less each), and the columns
 * left over at the middle are cleared.
 * HOW: The step in zPlaneStep; CopyLetterPlaneColumn twice a carry, the registers kept on the stack across;
 * ClearLetterPlaneByte for both halves of each column left, zPlaneRowsLeft counting the rows.
 * WAITS: no.
 * Entry: A the step, X the caller's (kept). Exit: X kept, N and Z from it; Y $FF; A $FF when no column was left,
 * else 8; C and V from the last add.
 */
Exit StretchLetterPlanes(uint8_t step, uint8_t kept_x)
{
    uint8_t a = step, x = 7, y = 7;
    bool c = false, v = false;
    Exit e;
    Sum sum;

    push8(kept_x);
    WR8(ZP->plane_step, step);
    do {
        sum = add_bytes(a, RD8(ZP->plane_step), false);
        a = sum.value;
        c = sum.carry;
        v = sum.overflow;
        if (sum.carry) {
            push8(a);
            jsr_push(0x8270);
            CopyLetterPlaneColumn(x, y);
            jsr_pop();
            push8(x);
            push8(y);
            jsr_push(0x827D);
            e = CopyLetterPlaneColumn((uint8_t)(x ^ 0x0F), (uint8_t)(y ^ 0x0F));
            jsr_pop();
            c = e.c;
            v = e.v;
            y = pull8();
            x = pull8();
            y--;
            a = pull8();
        }
        x--;
    } while (!(x & 0x80));
    a = y;
    if (!(y & 0x80)) {
        x = (uint8_t)(y ^ 0x0F);
        do {
            push8(x);
            push8(y);
            a = TILE_ROWS;
            WR8(ZP->plane_rows_left, TILE_ROWS);
            do {
                jsr_push(0x8293);
                e = ClearLetterPlaneByte(x);
                jsr_pop();
                x = y;
                y = e.x;
                jsr_push(0x8297);
                e = ClearLetterPlaneByte(x);
                jsr_pop();
                x = y;
                y = e.x;
                c = e.c;
                v = e.v;
            } while (DEC8(ZP->plane_rows_left) != 0);
            y = pull8();
            x = pull8();
            x++;
            y--;
        } while (!(y & 0x80));
    }
    x = pull8();
    return with_v(with_c(with_y(with_x(with_a(kept_nz(x), a), x), y), c), v);
}

void abi_StretchLetterPlanes(void)
{
    exit_shim(StretchLetterPlanes(cpu.a, cpu.x));
}

/*
 * @routine 004D:2CA TurnLetterPlanes
 * @run 75:82CA
 * WHAT IT DOES: Stretches the letter's tiles along the other axis: transposes them, stretches, transposes back.
 * HOW: X and the step kept on the stack; TransposeLetterTiles, StretchLetterPlanes, TransposeLetterTiles.
 * WAITS: no.
 * Entry: A the step, X the caller's (kept). Exit: X kept, N and Z from it; A zTemp0F, Y $80, C 1, V from the stretch.
 */
Exit TurnLetterPlanes(uint8_t step, uint8_t kept_x)
{
    Exit e, stretch;
    uint8_t x;

    push8(kept_x);
    push8(step);
    jsr_push(0x82CF);
    e = TransposeLetterTiles();
    jsr_pop();
    step = pull8();
    jsr_push(0x82D3);
    stretch = StretchLetterPlanes(step, e.x);
    jsr_pop();
    jsr_push(0x82D6);
    e = TransposeLetterTiles();
    jsr_pop();
    x = pull8();
    e = with_v(e, stretch.v);
    e.nz = x;
    return with_x(e, x);
}

void abi_TurnLetterPlanes(void)
{
    exit_shim(TurnLetterPlanes(cpu.a, cpu.x));
}

/*
 * @routine 004D:208 MariaLetterReveal
 * @run 75:8208
 * WHAT IT DOES: One step of one of Maria's turning letters: its pattern copied into StageMaskBuffer's two planes,
 * its step counted, the planes stretched and turned by eight times the count (not when that is a multiple of 256),
 * and the planes written to its VRAM.
 * HOW: The pattern read through zTemp08 (its words' low bytes to the PLANE_LO rows, high bytes to PLANE_HI); the
 * count in life_timer; StretchLetterPlanes, TurnLetterPlanes; SetVramAddressXY from zTemp00 and zTemp01; X kept on
 * the stack.
 * WAITS: no.
 * Entry: X the letter, zTemp00-01 its VRAM address, zTemp08-09 its pattern. Exit: A $80, X kept, N and Z from it,
 * Y zTemp01, C 1, V 1.
 */
Exit MariaLetterReveal(uint8_t letter)
{
    uint8_t x = 0, y = 0, value, amount, lo, hi;

    push8(letter);
    do {
        value = RD8_PTR_Y(ZP->temp08, y);
        wr8(plane((uint8_t)(PLANE_LO + x)), value);
        y++;
        value = RD8_PTR_Y(ZP->temp08, y);
        wr8(plane((uint8_t)(PLANE_HI + x)), value);
        y++;
        x++;
        if ((y & 0x1F) == 0) {
            x = (uint8_t)(x + PLANE_ROW);
        }
    } while (y != PLANE_BYTES);
    letter = pull8();
    push8(letter);
    INC8(OBJ->life_timer[letter]);
    amount = (uint8_t)(RD8(OBJ->life_timer[letter]) << 3);
    if (amount != 0) {
        push8(amount);
        jsr_push(0x8237);
        StretchLetterPlanes(amount, letter);
        jsr_pop();
        amount = pull8();
        jsr_push(0x823B);
        TurnLetterPlanes(amount, letter);
        jsr_pop();
    }
    lo = RD8(ZP->temp00);
    hi = RD8(ZP->temp01);
    jsr_push(0x8242);
    SetVramAddressXY(lo, hi);
    jsr_pop();
    x = 0;
    do {
        value = rd8(plane((uint8_t)(PLANE_LO + x)));
        WR8(VDC->data_lo, value);
        value = rd8(plane((uint8_t)(PLANE_HI + x)));
        WR8(VDC->data_hi, value);
        x++;
        if ((x & 0x0F) == 0) {
            x = (uint8_t)(x + PLANE_ROW);
        }
    } while (x != PLANE_BYTES);
    letter = pull8();
    return with_v(with_c(with_y(with_x(with_a(kept_nz(letter), PLANE_BYTES), letter), hi), true), true);
}

void abi_MariaLetterReveal(void)
{
    exit_shim(MariaLetterReveal(cpu.x));
}

/*
 * @routine 004D:1E0 MariaLetterStep
 * @run 75:81E0
 * WHAT IT DOES: One frame of one of Maria's letters: a letter in state 1 turns a step (MariaLetterReveal of its
 * pattern from $BB54), and at its 32nd step it is done (state 2) and the next letter starts.
 * HOW: OBJ state and life_timer; LetterPlace; the pattern's address added; the next slot's state counted up.
 * WAITS: no.
 * Entry: X the letter. Exit: not turning: A the state, or the state less one, N and Z from it, Y, C and V kept;
 * turning: A the count, N, Z and C from CMP #$20, Y and V as MariaLetterReveal left them; done: A $20, C 1, N and Z
 * from the next letter's INC. X kept.
 */
Exit MariaLetterStep(uint8_t letter)
{
    uint8_t state, count;
    Exit e;

    state = RD8(OBJ->state[letter]);
    if (state == 0) {
        return with_a(kept_nz(0), 0);
    }
    state--;
    if (state != 0) {
        return with_a(kept_nz(state), state);
    }
    jsr_push(0x81EB);
    LetterPlace(letter);
    jsr_pop();
    letter_pattern_base(MARIA_LETTERS_LO, MARIA_LETTERS_HI);
    jsr_push(0x81FA);
    e = MariaLetterReveal(letter);
    jsr_pop();
    count = RD8(OBJ->life_timer[letter]);
    e = with_c(with_a(e, count), count >= LETTER_DONE_MARIA);
    e.x_kept = true;
    if (count != LETTER_DONE_MARIA) {
        e.nz = (uint8_t)(count - LETTER_DONE_MARIA);
        return e;
    }
    INC8(OBJ->state[letter]);
    e.nz = INC8(OBJ->state[(uint8_t)(letter + 1)]);
    return e;
}

void abi_MariaLetterStep(void)
{
    exit_shim(MariaLetterStep(cpu.x));
}

/*
 * @routine 004D:187 ContinueWaitMaria
 * @run 75:8187
 * WHAT IT DOES: Maria's message, a frame. RUN: once the last letter is done goes on; before, writes every letter
 * whole at once and goes on. Otherwise, once the last letter is done the screen's timer runs (Richter's, at $8087);
 * until then every letter steps.
 * HOW: BiosJoyTrigger's RUN bit, the last letter's state; LetterPlace, SetVramAddressXY and the pattern from $BB54
 * through zTemp08 for the whole writes; MariaLetterStep for letters 7 down to 0.
 * WAITS: no.
 * Entry: nothing. Exit: RUN: A 2 (done) or the last pattern byte (written), C 1, N and Z from the
 * sub-mode's INC, X kept or $FF, Y kept or $80, V kept or 0; timer: as continue_count_down from A 2, C 1; stepping: X
 * $FF, N 1, Z 0, the rest as the last MariaLetterStep left them.
 */
Exit ContinueWaitMaria(void)
{
    Exit e = kept_nz(0);
    uint8_t state, value = 0, lo, hi, y;
    int8_t letter;
    bool v = false;

    if (rd8(BiosJoyTrigger) & JOY_RUN) {
        state = RD8(OBJ->state[LETTER_LAST]);
        if (state == 2) {
            WR8(ZP->field80, 0);
            e = with_c(with_a(e, state), true);
            e.nz = INC8(ZP->game_sub_mode);
            return e;
        }
        for (letter = LETTER_LAST; letter >= 0; letter--) {
            jsr_push(0x819F);
            LetterPlace((uint8_t)letter);
            jsr_pop();
            v = letter_pattern_base(MARIA_LETTERS_LO, MARIA_LETTERS_HI);
            push8((uint8_t)letter);
            lo = RD8(ZP->temp00);
            hi = RD8(ZP->temp01);
            jsr_push(0x81B4);
            SetVramAddressXY(lo, hi);
            jsr_pop();
            letter = (int8_t)pull8();
            y = 0;
            do {
                value = RD8_PTR_Y(ZP->temp08, y);
                WR8(VDC->data_lo, value);
                y++;
                value = RD8_PTR_Y(ZP->temp08, y);
                WR8(VDC->data_hi, value);
                y++;
            } while (y != PLANE_BYTES);
        }
        WR8(ZP->field80, 0);
        e = with_v(with_c(with_y(with_x(with_a(e, value), 0xFF), PLANE_BYTES), true), v);
        e.nz = INC8(ZP->game_sub_mode);
        return e;
    }
    state = RD8(OBJ->state[LETTER_LAST]);
    if (state == 2) {
        Exit at = with_c(with_a(kept_nz(0), state), true);

        return continue_count_down(at);
    }
    e = with_c(with_a(kept_nz((uint8_t)(state - 2)), state), state >= 2);
    for (letter = LETTER_LAST; letter >= 0; letter--) {
        jsr_push(0x81DC);
        e = over(e, MariaLetterStep((uint8_t)letter));
        jsr_pop();
    }
    e = with_x(e, 0xFF);
    e.nz = 0xFF;
    return e;
}

void abi_ContinueWaitMaria(void)
{
    exit_shim(ContinueWaitMaria());
}

/* TAI from KernelZeros: Y, A and X pushed, the zeros moved, then pulled. */
static void clear_by_tai(uint16_t dst, uint16_t len, uint8_t a, uint8_t x, uint8_t y)
{
    push8(y);
    push8(a);
    push8(x);
    for (uint16_t i = 0; i < len; i++) {
        uint8_t byte = rd8((uint16_t)(KernelZeros + (i & 1)));

        wr8((uint16_t)(dst + i), byte);
    }
    (void)pull8();
    (void)pull8();
    (void)pull8();
}

/* One palette word through zTemp00 at Y: each of its three colour fields one less, down to zero. */
static void fade_palette_word(uint8_t y)
{
    uint8_t value, field, high;
    bool c;

    value = RD8_PTR_Y(ZP->temp00, y);
    field = (uint8_t)(value & 0x07);
    if (field != 0) {
        field--;
    }
    WR8(ZP->temp02, field);
    value = RD8_PTR_Y(ZP->temp00, y);
    field = (uint8_t)(value & 0x38);
    if (field != 0) {
        field = (uint8_t)(field - 0x08);
    }
    field = (uint8_t)(field | RD8(ZP->temp02));
    WR8(ZP->temp02, field);
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(ZP->temp03, value);
    y++;
    value = RD8_PTR_Y(ZP->temp00, y);
    ROR8(ZP->temp03, (value & 0x01) != 0);
    field = (uint8_t)(RD8(ZP->temp03) & 0xE0);
    if (field != 0) {
        field = (uint8_t)(field - 0x20);
    }
    WR8(ZP->temp03, 0);
    c = (field & 0x80) != 0;
    field = (uint8_t)(field << 1);
    ROL8(ZP->temp03, c);
    field = (uint8_t)(field | RD8(ZP->temp02));
    y--;
    WR8_PTR_Y(ZP->temp00, y, field);
    y++;
    high = RD8(ZP->temp03);
    WR8_PTR_Y(ZP->temp00, y, high);
}

/*
 * @routine 004D:330 ContinueFade
 * @run 75:8330
 * WHAT IT DOES: Fades the screen out. Its first frame clears the list channels and script slots and starts script
 * slot 0 as kind 6 with the script in Y; then every fourth frame, nine times, every palette word in the bank's
 * $B000-$B3FF (but $B2A0-$B2BF) loses one step of each colour, with script slot 0 restarted as kind 7, script $20;
 * the ninth goes on.
 * HOW: zField80 says the fade has begun; TAI from KernelZeros; the steps in slot 0's life_timer, the frames in its
 * f3510; the palette's bank ($12, or $30 with zIdleFlag bit 0, above the RAM base bank) mapped into MPR5.
 * WAITS: no.
 * Entry: X and Y the dispatch's (Y the script for kind 6). Exit: first frame: A 1, X 0, Y the script, N and Z from
 * zField80's INC; waiting: A zField80, N and Z from the DEC, X, Y, C and V kept; a step: A 4, X 0, Y $20, C 1, V 0,
 * N and Z from the step's DEC or the sub-mode's INC.
 */
Exit ContinueFade(uint8_t kept_x, uint8_t script)
{
    uint8_t begun, left, bank_offset, y;
    Sum bank, next;

    begun = RD8(ZP->field80);
    if (begun == 0) {
        clear_by_tai(ADDR_OF(LCH->kind[0]), 0x7F, begun, kept_x, script);
        clear_by_tai(ADDR_OF(SLOTS->kind[0]), 0x60, begun, kept_x, script);
        jsr_push(0x8346);
        StartScriptSlotKind6(script, 0);
        jsr_pop();
        WR8(OBJ->life_timer[0], FADE_STEPS);
        WR8(OBJ->f3510[0], 1);
        return with_y(with_x(with_a(kept_nz(INC8(ZP->field80)), 1), 0), script);
    }
    left = DEC8(OBJ->f3510[0]);
    if (left != 0) {
        return with_a(kept_nz(left), begun);
    }
    bank_offset = (RD8(ZP->idle_flag) & 0x01) ? FADE_BANK_IDLE : FADE_BANK;
    bank = add_bytes(bank_offset, RD8(ZP->ram_base_bank), false);
    tam(MPR5, bank.value);
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, PALETTE_HI);
    for (bool faded = false; !faded;) {
        for (y = 0; y < PALETTE_ROW; y = (uint8_t)(y + 2)) {
            fade_palette_word(y);
        }
        for (;;) {
            next = add_bytes(PALETTE_ROW, RD8(ZP->temp00), false);
            WR8(ZP->temp00, next.value);
            if (next.carry) {
                INC8(ZP->temp01);
            }
            if (RD8(ZP->temp01) == PALETTE_END_HI) {
                faded = true;
                break;
            }
            if (RD8(ZP->temp00) != PALETTE_SKIP_LO) {
                break;
            }
            if (RD8(ZP->temp01) != PALETTE_SKIP_HI) {
                break;
            }
        }
    }
    jsr_push(0x83CB);
    StartScriptSlotKind7(PALETTE_ROW, 0);
    jsr_pop();
    WR8(OBJ->f3510[0], FADE_WAIT);
    left = DEC8(OBJ->life_timer[0]);
    if (left != 0) {
        return with_v(with_c(with_y(with_x(with_a(kept_nz(left), FADE_WAIT), 0), PALETTE_ROW), true), false);
    }
    return with_v(with_c(with_y(with_x(with_a(kept_nz(INC8(ZP->game_sub_mode)), FADE_WAIT), 0), PALETTE_ROW), true),
                  false);
}

void abi_ContinueFade(void)
{
    exit_shim(ContinueFade(cpu.x, cpu.y));
}

/* LDX ContinueChoice, LDA ContinueCursorRows,X, STA the cursor's y_lo: the cursor at the row of the choice. */
static uint8_t place_cursor(uint8_t *choice)
{
    uint8_t row;

    *choice = rd8(ContinueChoice);
    row = rd8((uint16_t)(ContinueCursorRows + *choice));
    WR8(OBJ->y_lo[CURSOR], row);
    return row;
}

/*
 * @routine 004D:3D8 ContinueClear
 * @run 75:83D8
 * WHAT IT DOES: Clears the stage away for the menu: the enemy slots, zField90 and zField91, the raster effect, the
 * scroll, object 8 and objects $54-$57; then the menu's list channel and its cursor (object 8: frame 4, sprite set
 * 3, X $4F) on the first choice.
 * HOW: ClearEnemySlots; ClearObject; a CPX #$10 and BPL after slot 8, which ends the loop at once (9 less 16 is
 * negative); StartListChannel0Kind2 $6C; ContinueChoice 0, and the cursor's row from ContinueCursorRows.
 * WAITS: no.
 * Entry: X the dispatch's (ClearEnemySlots keeps it on the stack), Y the dispatch's. Exit: A the cursor's row, N and Z
 * from it, X the choice, C 1. Y and V kept.
 */
Exit ContinueClear(uint8_t kept_x)
{
    uint8_t x, choice, row;

    jsr_push(0x83DB);
    ClearEnemySlots(kept_x);
    jsr_pop();
    WR8(ZP->field90, 0);
    WR8(ZP->field91, 0);
    WR8(ZP->raster_effect, 0);
    WR8(SCROLL->x_lo[0], 0);
    WR8(SCROLL->x_hi[0], 0);
    WR8(SCROLL->y_lo[0], 0);
    WR8(SCROLL->y_hi[0], 0);
    x = CURSOR;
    do {
        jsr_push(0x83F2);
        ClearObject(x);
        jsr_pop();
        x++;
    } while (!((uint8_t)(x - 0x10) & 0x80));
    for (x = 0x54; x != 0x58; x++) {
        jsr_push(0x83FC);
        ClearObject(x);
        jsr_pop();
    }
    INC8(ZP->game_sub_mode);
    jsr_push(0x8408);
    StartListChannel0Kind2(MENU_LIST);
    jsr_pop();
    WR8(OBJ->frame[CURSOR], CURSOR_FRAME);
    WR8(OBJ->sprite_set[CURSOR], CURSOR_SPRITE_SET);
    WR8(OBJ->x_lo[CURSOR], CURSOR_X);
    wr8(ContinueChoice, 0);
    row = place_cursor(&choice);
    return with_c(with_x(with_a(kept_nz(row), row), choice), true);
}

void abi_ContinueClear(void)
{
    exit_shim(ContinueClear(cpu.x));
}

/* zStage not zero and ProgressCharacter bit 0 set: the menu has its third choice. The LSR's carry. */
static bool third_choice(void)
{
    if (RD8(ZP->stage) == 0) {
        return false;
    }
    return (rd8(ProgressCharacter) & 0x01) != 0;
}

/*
 * @routine 004D:41C ContinueShowMenu
 * @run 75:841C
 * WHAT IT DOES: Shows the menu: its script, and, when there is no third choice, the third choice's line blanked
 * (VRAM $0540, $20 words of tile $0110).
 * HOW: StartScriptSlot0Kind1 $0E; zStage and ProgressCharacter's bit 0; SetVramAddressXY, ST1 and ST2.
 * WAITS: no.
 * Entry: nothing. Exit: N and Z from the sub-mode's INC, X 0; third choice: A ProgressCharacter halved, Y $0E, C 1;
 * else A 2, Y 5, C 0 (or kept when zStage is zero). V kept.
 */
Exit ContinueShowMenu(void)
{
    Exit e = kept_nz(0);
    uint8_t stage, progress;

    jsr_push(0x8421);
    StartScriptSlot0Kind1(MENU_SCRIPT);
    jsr_pop();
    e = with_y(with_x(e, 0), MENU_SCRIPT);
    stage = RD8(ZP->stage);
    if (stage != 0) {
        progress = rd8(ProgressCharacter);
        e = with_c(with_a(e, (uint8_t)(progress >> 1)), (progress & 0x01) != 0);
        if (progress & 0x01) {
            e.nz = INC8(ZP->game_sub_mode);
            return e;
        }
    }
    jsr_push(0x8432);
    SetVramAddressXY(THIRD_CHOICE_VRAM_LO, THIRD_CHOICE_VRAM_HI);
    jsr_pop();
    for (uint8_t words = THIRD_CHOICE_WORDS; words != 0; words--) {
        st1(BLANK_LO);
        st2(BLANK_HI);
    }
    e = with_y(with_x(with_a(e, 2), 0), THIRD_CHOICE_VRAM_HI);
    e.nz = INC8(ZP->game_sub_mode);
    return e;
}

void abi_ContinueShowMenu(void)
{
    exit_shim(ContinueShowMenu());
}

/* What ContinueMenu did, for its wrapper. */
typedef enum { MENU_IDLE, MENU_MOVED, MENU_CHOSEN } MenuPath;

typedef struct {
    MenuPath path;
    Exit blink;       /* ContinueCursorBlink's exit */
    uint8_t y;        /* Y at the sound calls */
    uint8_t choice;   /* the new choice, as LDX ContinueChoice read it back */
    Sum script;       /* the choice plus $0E, as the add made it */
    uint8_t row;
    uint8_t character;
    uint8_t voice;
    CommandCall sound;
    CommandCall stop;
    uint8_t done;
} MenuRun;

/*
 * @routine 004D:43E ContinueMenu
 * @run 75:843E
 * WHAT IT DOES: The menu, a frame: the cursor blinks; I or RUN chooses (the character's voice, the music stopped, and
 * zModeDone counted up for ModeDead); UP, DOWN or SELECT moves the choice (through two choices, or three when there
 * is a third), restarts the menu's script for it, sounds the move and puts the cursor on its row.
 * HOW: ContinueCursorBlink; BiosJoyTrigger; ContinueMovesTwo or ContinueMovesThree by the choice (two on for any key
 * but UP); StartScriptSlot0Kind1 $0E plus the choice; SoundCommand 1; ContinueVoices by zCharacter; SoundCommand $FF.
 * SoundCommand keeps X and Y, so the second call has the first one's.
 * WAITS: no.
 * Entry: Y the dispatch's. Exit: idle: A 0, N 0, Z 1, X, Y, C and V as ContinueCursorBlink left them; moved: A the
 * row, N and Z from it, X the choice, Y the choice plus $0E, C and V as SoundCommand left them; chosen: as
 * SoundCommand $FF left them, X zCharacter, N and Z from zModeDone's INC.
 */
MenuRun ContinueMenu(uint8_t kept_y)
{
    MenuRun run = { 0 };
    uint8_t keys, x;

    run.path = MENU_IDLE;
    jsr_push(0x8441);
    run.blink = ContinueCursorBlink();
    jsr_pop();
    run.y = run.blink.y_kept ? kept_y : run.blink.y;
    if (rd8(BiosJoyTrigger) & JOY_CHOOSE) {
        run.path = MENU_CHOSEN;
        run.character = RD8(ZP->character);
        run.voice = rd8((uint16_t)(ContinueVoices + run.character));
        jsr_push(0x848A);
        run.sound = SoundCommand(run.voice, run.character, run.y);
        jsr_pop();
        jsr_push(0x848F);
        run.stop = SoundCommand(SOUND_STOP, run.character, run.y);
        jsr_pop();
        run.done = INC8(ZP->mode_done);
        return run;
    }
    keys = (uint8_t)(rd8(BiosJoyTrigger) & JOY_MOVES);
    if (keys == 0) {
        return run;
    }
    run.path = MENU_MOVED;
    x = rd8(ContinueChoice);
    if (!(keys & JOY_UP)) {
        x = (uint8_t)(x + 2);
    }
    if (third_choice()) {
        run.choice = rd8((uint16_t)(ContinueMovesThree + x));
    } else {
        run.choice = rd8((uint16_t)(ContinueMovesTwo + x));
    }
    wr8(ContinueChoice, run.choice);
    run.script = add_bytes(run.choice, MENU_SCRIPT, false);
    jsr_push(0x8473);
    StartScriptSlot0Kind1(run.script.value);
    jsr_pop();
    run.y = run.script.value;
    jsr_push(0x8478);
    run.sound = SoundCommand(MOVE_SOUND, 0, run.y);
    jsr_pop();
    run.row = place_cursor(&run.choice);
    return run;
}

void abi_ContinueMenu(void)
{
    MenuRun run = ContinueMenu(cpu.y);

    exit_shim(run.blink);
    switch (run.path) {
    case MENU_IDLE:
        cpu.a = 0;
        abi_nz(0);
        return;
    case MENU_MOVED:
        abi_flag(P_C, run.script.carry);
        abi_flag(P_V, run.script.overflow);
        cpu.x = 0;
        cpu.y = run.y;
        cpu.a = MOVE_SOUND;
        abi_nz(MOVE_SOUND);
        sound_command_exit_shim(MOVE_SOUND, run.sound);
        cpu.x = run.choice;
        cpu.a = run.row;
        abi_nz(run.row);
        return;
    case MENU_CHOSEN:
        cpu.x = run.character;
        cpu.y = run.y;
        cpu.a = run.voice;
        abi_nz(run.voice);
        sound_command_exit_shim(run.voice, run.sound);
        cpu.a = SOUND_STOP;
        abi_nz(SOUND_STOP);
        sound_command_exit_shim(SOUND_STOP, run.stop);
        abi_nz(run.done);
        return;
    }
}

/*
 * @routine 004D:017 ContinueFrameMaria
 * @run 75:8017
 * WHAT IT DOES: The continue screen's step for Maria: the JMP through ContinueStepsMaria by the doubled sub-mode.
 * HOW: JMP (ContinueStepsMaria,X): the word's two bytes read, low then high, and the C of the routine there run.
 * WAITS: as the step does (no step waits).
 * Entry: X the doubled sub-mode. Exit: as the step leaves them.
 */
void ContinueFrameMaria(uint8_t twice)
{
    uint8_t lo, hi;

    lo = rd8((uint16_t)(ContinueStepsMaria + twice));
    hi = rd8((uint16_t)(ContinueStepsMaria + twice + 1));
    run_routine_from(0x8017, (uint16_t)(lo | (hi << 8)));
}

void abi_ContinueFrameMaria(void)
{
    ContinueFrameMaria(cpu.x);
}

/* At the JMP: A zCharacter, X the doubled sub-mode, N and Z from zCharacter, C the sub-mode's bit 7. */
static void continue_step_shim(uint8_t character, uint8_t twice, bool carry)
{
    cpu.a = character;
    cpu.x = twice;
    abi_nz(character);
    abi_flag(P_C, carry);
}

/*
 * @routine 004D:000 ContinueFrame
 * @run 75:8000
 * @stop 800B
 * WHAT IT DOES: The continue screen's frame, which ModeDead calls: runs the step of zGameSubMode, Richter's or Maria's.
 * HOW: The sub-mode doubled indexes ContinueStepsRichter ($800B) or, when zCharacter is not zero, ContinueStepsMaria
 * ($801A, through ContinueFrameMaria): Richter's set-up and message, or Maria's; then the fade, the clearing, the
 * menu's showing and the menu, the same for both.
 * WAITS: no; no step waits.
 * Entry: Y the caller's (ContinueFade takes it as a script). Exit: as the step leaves them.
 */
void ContinueFrame(void)
{
    uint8_t sub, twice, character, lo, hi;

    sub = RD8(ZP->game_sub_mode);
    twice = (uint8_t)(sub << 1);
    character = RD8(ZP->character);
    continue_step_shim(character, twice, (sub & 0x80) != 0);
    if (character != 0) {
        ContinueFrameMaria(twice);
        return;
    }
    lo = rd8((uint16_t)(ContinueStepsRichter + twice));
    hi = rd8((uint16_t)(ContinueStepsRichter + twice + 1));
    run_routine_from(0x8008, (uint16_t)(lo | (hi << 8)));
}

void abi_ContinueFrame(void)
{
    ContinueFrame();
}
