/*
 * The object that comes down at the end of a section in the secret stages and others (kind loaded by its own start):
 * placed, it waits, runs two rounds of scripts, throws eight pieces, falls until it lands, and when it is hit its
 * graphics erode and the stage's sub-step becomes 6. Every overlay holds its own copy at its own address and with
 * its own constants, so each routine is one body here taking them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "dissolve_b.h"
#include "pickup_b.h"

/*
 * Written on the register file, as the code keeps its registers across these calls: each load, compare, push and call
 * of the assembly is here in its order, a callee's registers left by its exit shim.
 */
static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }
static void ldx(uint8_t value) { cpu.x = value; abi_nz(value); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

/* JSR SoundCommand's vector with A the command. */
static void sound(uint8_t command, uint16_t ret)
{
    CommandCall call;

    lda(command);
    jsr_push(ret);
    call = SoundCommand(command, cpu.x, cpu.y);
    sound_command_exit_shim(command, call);
    jsr_pop();
}

static void script(uint8_t number, uint8_t at_slot, uint16_t ret)
{
    jsr_push(ret);
    script_slot_exit_shim(StartScriptSlotKind1(number, at_slot));
    jsr_pop();
}

/* LDA of the frame list's two halves into zTemp11 and zTemp10, and one StepObjectFrames. */
static void step_list(uint8_t slot, uint16_t list, uint16_t ret)
{
    uint8_t y = cpu.y;
    FrameStep step;

    lda((uint8_t)(list >> 8));
    WR8(ZP->temp11, cpu.a);
    lda((uint8_t)list);
    WR8(ZP->temp10, cpu.a);
    jsr_push(ret);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
}

/* 64 VRAM words from zBiosCl-Ch on: each read, its low byte shifted left and its high byte right, and written back. */
static void erode_words(uint16_t at)
{
    uint8_t value, old;

    push8(cpu.y);
    cpu.y = 0;
    do {
        jsr_push((uint16_t)(at + 5));
        SelectVramWriteAddress();
        cpu.a = 0x00;
        jsr_pop();
        lda(RD8(ZP->bios_c.lo));
        WR8(VDC->data_lo, cpu.a);
        lda(RD8(ZP->bios_c.hi));
        WR8(VDC->data_hi, cpu.a);
        jsr_push((uint16_t)(at + 0x12));
        SelectVramReadAddress();
        lda(0x01);
        jsr_pop();
        lda(RD8(ZP->bios_c.lo));
        WR8(VDC->data_lo, cpu.a);
        lda(RD8(ZP->bios_c.hi));
        WR8(VDC->data_hi, cpu.a);
        jsr_push((uint16_t)(at + 0x1F));
        SelectVramData();
        lda(0x02);
        jsr_pop();
        value = RD8(VDC->data_lo);
        abi_flag(P_C, (value & 0x80) != 0);
        lda((uint8_t)(value << 1));
        WR8(VDC->data_lo, cpu.a);
        old = RD8(VDC->data_hi);
        abi_flag(P_C, (old & 0x01) != 0);
        lda((uint8_t)(old >> 1));
        WR8(VDC->data_hi, cpu.a);
        INC8(ZP->bios_c.lo);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x40);
    } while (cpu.y < 0x40);
    cpu.y = pull8();
    abi_nz(cpu.y);
}

/* The VRAM address of pattern X from a table of words, into zBiosCl-Ch, and its words eroded. */
static void erode_pattern(uint16_t table, void (*words)(void), uint16_t ret)
{
    cpu.a = cpu.x;
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(table + cpu.y)));
    WR8(ZP->bios_c.lo, cpu.a);
    lda(rd8((uint16_t)(table + 1 + cpu.y)));
    WR8(ZP->bios_c.hi, cpu.a);
    jsr_push(ret);
    words();
    jsr_pop();
}

/* Patterns 0 to 7, X restored. */
static void erode_eight(void (*one)(void), uint16_t ret)
{
    push8(cpu.x);
    cpu.x = 0;
    do {
        jsr_push(ret);
        one();
        jsr_pop();
        cpu.x = (uint8_t)(cpu.x + 1);
        cmp(cpu.x, 8);
    } while (cpu.x < 8);
    ldx(pull8());
}

/* In states under 6, a pending hit of 1 to 4 plays sound $0B and takes the object to state 6 for 16 frames. */
static void hit_check(uint8_t slot, uint16_t at)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 6);
    if (cpu.a >= 6) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    if (cpu.a == 0) {
        return;
    }
    cmp(cpu.a, 5);
    if (cpu.a >= 5) {
        return;
    }
    sound(0x0B, (uint16_t)(at + 0x15));
    lda(0x10);
    WR8(OBJ->pickup_timer[slot], cpu.a);
    lda(6);
    WR8(OBJ->state[slot], cpu.a);
}

/* The dispatcher: the hit check, then the handler of the state, from the table behind it. */
static Jump pickup_dispatch(uint8_t slot, void (*check)(void), uint16_t at)
{
    JumpTableLayout layout = { (uint16_t)(at + 0x15) };

    jsr_push((uint16_t)(at + 3));
    check();
    jsr_pop();
    return JumpByStateTableBody(slot, &layout);
}

/* What differs between the stages' starts. */
typedef struct {
    uint8_t sprite_set;          /* stored first when the start gives one (has_set) */
    bool has_set;
    uint8_t x;                   /* the screen X */
    bool less_scroll;            /* less the scroll's low byte */
    bool level_x;                /* the X high byte from the scroll too, else both high bytes zeroed first */
    uint8_t y;
    uint8_t list, list_at, script, script_at;
} PickupStart;

/* The object placed, its list channel and script started, the CD faded and a sound (or all sound) given. */
static void pickup_start(uint8_t slot, const PickupStart *s, uint16_t at)
{
    uint8_t nz, last;
    bool carry;
    uint16_t pc = (uint16_t)(at + 3);
    Sum diff;

    jsr_push(pc);
    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
    jsr_pop();
    if (s->has_set) {
        lda(s->sprite_set);
        WR8(OBJ->sprite_set[slot], cpu.a);
        pc = (uint16_t)(pc + 5);
    }
    lda(0xC0);
    WR8(OBJ->pickup_timer[slot], cpu.a);
    pc = (uint16_t)(pc + 5);
    if (!s->level_x) {
        WR8(OBJ->x_hi[slot], 0);
        WR8(OBJ->y_hi[slot], 0);
        pc = (uint16_t)(pc + 6);
    }
    lda(s->x);
    pc = (uint16_t)(pc + 2);
    if (s->less_scroll) {
        diff = sub_bytes(cpu.a, RD8(SCROLL->x_lo[0]), true);
        lda(diff.value);
        abi_flag(P_C, diff.carry);
        abi_flag(P_V, diff.overflow);
        pc = (uint16_t)(pc + 4);
    }
    WR8(OBJ->x_lo[slot], cpu.a);
    pc = (uint16_t)(pc + 3);
    if (s->level_x) {
        lda(0x04);
        diff = sub_bytes(cpu.a, rd8(ScrollXHiScreen), (cpu.p & P_C) != 0);
        lda(diff.value);
        abi_flag(P_C, diff.carry);
        abi_flag(P_V, diff.overflow);
        WR8(OBJ->x_hi[slot], cpu.a);
        WR8(OBJ->y_hi[slot], 0);
        pc = (uint16_t)(pc + 11);
    }
    lda(s->y);
    WR8(OBJ->y_lo[slot], cpu.a);
    pc = (uint16_t)(pc + 5);
    push8(cpu.x);
    lda(s->list);
    ldx(s->list_at);
    pc = (uint16_t)(pc + 8);
    jsr_push(pc);
    list_channel_exit_shim(StartListChannelKind2(s->list, s->list_at));
    jsr_pop();
    lda(s->script);
    ldx(s->script_at);
    pc = (uint16_t)(pc + 7);
    script(s->script, s->script_at, pc);
    lda(0x0C);
    pc = (uint16_t)(pc + 5);
    jsr_push(pc);
    (void)vec_cd_fade(0x0C);
    cd_fade_exit_shim(0x0C);
    jsr_pop();
    RMW8(ZP->field8e, old, old | 0x08);
    lda(0x5C);
    WR8(ZP->fade_sound, cpu.a);
    ldx(rd8(TimeSlowMode));
    if (cpu.x == 0) {
        jsr_push((uint16_t)(pc + 14));
        lda(0xFF);
        {
            CommandCall call = SoundCommand(0xFF, cpu.x, cpu.y);

            sound_command_exit_shim(0xFF, call);
        }
        jsr_pop();
        sound(0x5C, (uint16_t)(pc + 19));
    }
    ldx(pull8());
}

/* The frames play while the wait counts down, then the next state. */
static void pickup_wait(uint8_t slot, uint16_t list, uint16_t at)
{
    uint8_t left;

    step_list(slot, list, (uint16_t)(at + 0x0B));
    left = DEC8(OBJ->pickup_timer[slot]);
    abi_nz(left);
    if (left == 0) {
        abi_nz(INC8(OBJ->state[slot]));
    }
}

/* Each time the frame list ends, one more script; after the last, a fixed frame and the next state. */
static void pickup_first_scripts(uint8_t slot, uint16_t list, uint8_t base, uint8_t script_at, uint8_t count, uint8_t frame, uint16_t at)
{
    Sum number;

    step_list(slot, list, (uint16_t)(at + 0x0B));
    if (cpu.a != 0) {
        return;
    }
    push8(cpu.x);
    lda(RD8(OBJ->pickup_count[slot]));
    number = add_bytes(cpu.a, base, false);
    lda(number.value);
    abi_flag(P_C, number.carry);
    abi_flag(P_V, number.overflow);
    ldx(script_at);
    script(cpu.a, cpu.x, (uint16_t)(at + 0x19));
    ldx(pull8());
    INC8(OBJ->pickup_count[slot]);
    lda(RD8(OBJ->pickup_count[slot]));
    cmp(cpu.a, count);
    if (cpu.a < count) {
        return;
    }
    WR8(OBJ->pickup_count[slot], 0);
    lda(frame);
    WR8(OBJ->frame[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

/* Every fourth frame one more script; after the eighth, the pieces spawned and the object made hittable. */
static void pickup_second_scripts(uint8_t slot, uint8_t base, uint8_t script_at, uint8_t count, void (*spawn)(void), bool raster_off,
                                  uint16_t at)
{
    Sum number;

    lda(RD8(ZP->wave_counter));
    lda((uint8_t)(cpu.a & 3));
    if (cpu.a != 0) {
        return;
    }
    push8(cpu.x);
    lda(RD8(OBJ->pickup_count[slot]));
    number = add_bytes(cpu.a, base, false);
    lda(number.value);
    abi_flag(P_C, number.carry);
    abi_flag(P_V, number.overflow);
    ldx(script_at);
    script(cpu.a, cpu.x, (uint16_t)(at + 0x12));
    ldx(pull8());
    INC8(OBJ->pickup_count[slot]);
    lda(RD8(OBJ->pickup_count[slot]));
    cmp(cpu.a, count);
    if (cpu.a < count) {
        return;
    }
    jsr_push((uint16_t)(at + 0x20));
    spawn();
    jsr_pop();
    if (raster_off) {
        WR8(ZP->raster_effect, 0);
        WR8(ZP->raster_arg, 0);
    }
    lda(1);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

/* Eight pieces of one kind in free slots from $20 on, each told this object's slot. */
static void pickup_spawn(uint8_t kind, uint16_t at)
{
    uint8_t found_a, to, last;
    bool found;

    WR8(ZP->temp00, cpu.x);
    push8(cpu.x);
    cpu.x = 0;
    do {
        jsr_push((uint16_t)(at + 7));
        to = FindFreeSlotFrom20(&found_a, &found);
        find_slot_exit_shim(to, found_a, found);
        jsr_pop();
        if (!found) {
            break;
        }
        lda(kind);
        jsr_push((uint16_t)(at + 0x0E));
        last = StartObjectAtPlace(cpu.a, cpu.x, cpu.y);
        copy_place_exit_shim(last);
        jsr_pop();
        lda(RD8(ZP->temp00));
        WR8(OBJ->pickup_timer[cpu.y], cpu.a);
        cpu.x = (uint8_t)(cpu.x + 1);
        cmp(cpu.x, 8);
    } while (cpu.x < 8);
    ldx(pull8());
}

/* The frames play as it falls; on landing a sound and the next state. */
static void pickup_fall(uint8_t slot, uint16_t list, uint8_t command, uint16_t at)
{
    Mover mover;

    step_list(slot, list, (uint16_t)(at + 0x0B));
    lda(0);
    jsr_push((uint16_t)(at + 0x10));
    mover = StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0);
    mover_exit_shim(mover, 0x01);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    sound(command, (uint16_t)(at + 0x17));
    abi_nz(INC8(OBJ->state[slot]));
}

/* The frames play; at one of them a sound. */
static void pickup_rest(uint8_t slot, uint16_t list, uint8_t frame, uint8_t command, uint16_t at)
{
    step_list(slot, list, (uint16_t)(at + 0x0B));
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, frame);
    if (cpu.a != frame) {
        return;
    }
    sound(command, (uint16_t)(at + 0x17));
}

/* The graphics erode while the count runs; then the stage's sub-step 6 and the object cleared. */
static void pickup_erase(uint8_t slot, void (*erode)(void), uint16_t at)
{
    uint8_t left;

    jsr_push((uint16_t)(at + 3));
    erode();
    jsr_pop();
    left = DEC8(OBJ->pickup_timer[slot]);
    abi_nz(left);
    if (left != 0) {
        return;
    }
    lda(6);
    WR8(ZP->sub_step, cpu.a);
    ClearObject(slot);
}

/*
 * @routine 05CA:6B3 SubStepPickupHitCheck05CA6B3
 * @run 70:AEB3
 * WHAT IT DOES: The pickup's hit (Secret stage 02): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck05CA6B3(uint8_t slot)
{
    hit_check(slot, 0xAEB3);
}

void abi_SubStepPickupHitCheck05CA6B3(void)
{
    SubStepPickupHitCheck05CA6B3(cpu.x);
}

/*
 * @routine 05CA:690 SubStepPickup05CA690
 * @run 70:AE90
 * WHAT IT DOES: The pickup of Secret stage 02: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck05CA6B3, then JumpByStateTableBody with the table at $AEA5.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup05CA690(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck05CA6B3, 0xAE90);
}

void abi_SubStepPickup05CA690(void)
{
    Jump jump = SubStepPickup05CA690(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 05CA:6D3 SubStepPickupStart05CA6D3
 * @run 70:AED3
 * WHAT IT DOES: Starts the pickup (Secret stage 02): its record, a timer of $C0, its place, list channel $81 and script
 * $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart05CA6D3(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0x80, true, false, 0x30, 0x81, 5, 0x82, 5 };

    pickup_start(slot, &start, 0xAED3);
}

void abi_SubStepPickupStart05CA6D3(void)
{
    SubStepPickupStart05CA6D3(cpu.x);
}

/*
 * @routine 05CA:718 SubStepPickupWait05CA718
 * @run 70:AF18
 * WHAT IT DOES: The pickup waits (Secret stage 02): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $AF2C; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait05CA718(uint8_t slot)
{
    pickup_wait(slot, 0xAF2C, 0xAF18);
}

void abi_SubStepPickupWait05CA718(void)
{
    SubStepPickupWait05CA718(cpu.x);
}

/*
 * @routine 05CA:735 SubStepPickupFirstScripts05CA735
 * @run 70:AF35
 * WHAT IT DOES: The pickup's first round (Secret stage 02): each time its frame list ends, script $83 plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts05CA735(uint8_t slot)
{
    pickup_first_scripts(slot, 0xAF2C, 0x83, 5, 6, 0x01, 0xAF35);
}

void abi_SubStepPickupFirstScripts05CA735(void)
{
    SubStepPickupFirstScripts05CA735(cpu.x);
}

/*
 * @routine 05CA:78E SubStepPickupSpawn05CA78E
 * @run 70:AF8E
 * WHAT IT DOES: Starts up to eight objects of kind $23 in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn05CA78E(void)
{
    pickup_spawn(0x23, 0xAF8E);
}

void abi_SubStepPickupSpawn05CA78E(void)
{
    SubStepPickupSpawn05CA78E();
}

/*
 * @routine 05CA:765 SubStepPickupSecondScripts05CA765
 * @run 70:AF65
 * WHAT IT DOES: The pickup's second round (Secret stage 02): every fourth frame script $89 plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn05CA78E.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts05CA765(uint8_t slot)
{
    pickup_second_scripts(slot, 0x89, 5, 8, SubStepPickupSpawn05CA78E, false, 0xAF65);
}

void abi_SubStepPickupSecondScripts05CA765(void)
{
    SubStepPickupSecondScripts05CA765(cpu.x);
}

/*
 * @routine 05CA:7A8 SubStepPickupFall05CA7A8
 * @run 70:AFA8
 * WHAT IT DOES: The pickup falls (Secret stage 02): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall05CA7A8(uint8_t slot)
{
    pickup_fall(slot, 0xAFDB, 0x2C, 0xAFA8);
}

void abi_SubStepPickupFall05CA7A8(void)
{
    SubStepPickupFall05CA7A8(cpu.x);
}

/*
 * @routine 05CA:7C3 SubStepPickupRest05CA7C3
 * @run 70:AFC3
 * WHAT IT DOES: The landed pickup (Secret stage 02): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest05CA7C3(uint8_t slot)
{
    pickup_rest(slot, 0xAFDB, 0x0D, 0x51, 0xAFC3);
}

void abi_SubStepPickupRest05CA7C3(void)
{
    SubStepPickupRest05CA7C3(cpu.x);
}

/*
 * @routine 05CA:43C ErodeVramWords05CA43C
 * @run 70:AC3C
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords05CA43C(void)
{
    erode_words(0xAC3C);
}

void abi_ErodeVramWords05CA43C(void)
{
    ErodeVramWords05CA43C();
}

/*
 * @routine 05CA:41B ErodePattern05CA41B
 * @run 70:AC1B
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $AC2C into zBiosCl-Ch, then ErodeVramWords05CA43C.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern05CA41B(void)
{
    erode_pattern(0xAC2C, ErodeVramWords05CA43C, 0xAC2B);
}

void abi_ErodePattern05CA41B(void)
{
    ErodePattern05CA41B();
}

/*
 * @routine 05CA:40F ErodePatterns05CA40F
 * @run 70:AC0F
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern05CA41B, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns05CA40F(void)
{
    erode_eight(ErodePattern05CA41B, 0xAC14);
}

void abi_ErodePatterns05CA40F(void)
{
    ErodePatterns05CA40F();
}

/*
 * @routine 05CB:01A SubStepPickupErase05CB01A
 * @run 70:B01A
 * WHAT IT DOES: The pickup's last state (Secret stage 02): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns05CA40F, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase05CB01A(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns05CA40F, 0xB01A);
}

void abi_SubStepPickupErase05CB01A(void)
{
    SubStepPickupErase05CB01A(cpu.x);
}

/*
 * @routine 072A:6E7 SubStepPickupHitCheck072A6E7
 * @run 70:AEE7
 * WHAT IT DOES: The pickup's hit (Secret stage 03): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck072A6E7(uint8_t slot)
{
    hit_check(slot, 0xAEE7);
}

void abi_SubStepPickupHitCheck072A6E7(void)
{
    SubStepPickupHitCheck072A6E7(cpu.x);
}

/*
 * @routine 072A:6C4 SubStepPickup072A6C4
 * @run 70:AEC4
 * WHAT IT DOES: The pickup of Secret stage 03: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck072A6E7, then JumpByStateTableBody with the table at $AED9.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup072A6C4(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck072A6E7, 0xAEC4);
}

void abi_SubStepPickup072A6C4(void)
{
    Jump jump = SubStepPickup072A6C4(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 072A:707 SubStepPickupStart072A707
 * @run 70:AF07
 * WHAT IT DOES: Starts the pickup (Secret stage 03): its record, a timer of $C0, its place, list channel $81 and script
 * $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart072A707(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0x80, false, false, 0x30, 0x81, 5, 0x82, 5 };

    pickup_start(slot, &start, 0xAF07);
}

void abi_SubStepPickupStart072A707(void)
{
    SubStepPickupStart072A707(cpu.x);
}

/*
 * @routine 072A:748 SubStepPickupWait072A748
 * @run 70:AF48
 * WHAT IT DOES: The pickup waits (Secret stage 03): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $AF5C; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait072A748(uint8_t slot)
{
    pickup_wait(slot, 0xAF5C, 0xAF48);
}

void abi_SubStepPickupWait072A748(void)
{
    SubStepPickupWait072A748(cpu.x);
}

/*
 * @routine 072A:765 SubStepPickupFirstScripts072A765
 * @run 70:AF65
 * WHAT IT DOES: The pickup's first round (Secret stage 03): each time its frame list ends, script $83 plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts072A765(uint8_t slot)
{
    pickup_first_scripts(slot, 0xAF5C, 0x83, 5, 6, 0x01, 0xAF65);
}

void abi_SubStepPickupFirstScripts072A765(void)
{
    SubStepPickupFirstScripts072A765(cpu.x);
}

/*
 * @routine 072A:7BE SubStepPickupSpawn072A7BE
 * @run 70:AFBE
 * WHAT IT DOES: Starts up to eight objects of kind $23 in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn072A7BE(void)
{
    pickup_spawn(0x23, 0xAFBE);
}

void abi_SubStepPickupSpawn072A7BE(void)
{
    SubStepPickupSpawn072A7BE();
}

/*
 * @routine 072A:795 SubStepPickupSecondScripts072A795
 * @run 70:AF95
 * WHAT IT DOES: The pickup's second round (Secret stage 03): every fourth frame script $89 plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn072A7BE.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts072A795(uint8_t slot)
{
    pickup_second_scripts(slot, 0x89, 5, 8, SubStepPickupSpawn072A7BE, false, 0xAF95);
}

void abi_SubStepPickupSecondScripts072A795(void)
{
    SubStepPickupSecondScripts072A795(cpu.x);
}

/*
 * @routine 072A:7D8 SubStepPickupFall072A7D8
 * @run 70:AFD8
 * WHAT IT DOES: The pickup falls (Secret stage 03): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall072A7D8(uint8_t slot)
{
    pickup_fall(slot, 0xB00B, 0x2C, 0xAFD8);
}

void abi_SubStepPickupFall072A7D8(void)
{
    SubStepPickupFall072A7D8(cpu.x);
}

/*
 * @routine 072A:7F3 SubStepPickupRest072A7F3
 * @run 70:AFF3
 * WHAT IT DOES: The landed pickup (Secret stage 03): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest072A7F3(uint8_t slot)
{
    pickup_rest(slot, 0xB00B, 0x0D, 0x51, 0xAFF3);
}

void abi_SubStepPickupRest072A7F3(void)
{
    SubStepPickupRest072A7F3(cpu.x);
}

/*
 * @routine 072A:444 ErodeVramWords072A444
 * @run 70:AC44
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords072A444(void)
{
    erode_words(0xAC44);
}

void abi_ErodeVramWords072A444(void)
{
    ErodeVramWords072A444();
}

/*
 * @routine 072A:423 ErodePattern072A423
 * @run 70:AC23
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $AC34 into zBiosCl-Ch, then ErodeVramWords072A444.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern072A423(void)
{
    erode_pattern(0xAC34, ErodeVramWords072A444, 0xAC33);
}

void abi_ErodePattern072A423(void)
{
    ErodePattern072A423();
}

/*
 * @routine 072A:417 ErodePatterns072A417
 * @run 70:AC17
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern072A423, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns072A417(void)
{
    erode_eight(ErodePattern072A423, 0xAC1C);
}

void abi_ErodePatterns072A417(void)
{
    ErodePatterns072A417();
}

/*
 * @routine 072B:04A SubStepPickupErase072B04A
 * @run 70:B04A
 * WHAT IT DOES: The pickup's last state (Secret stage 03): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns072A417, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase072B04A(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns072A417, 0xB04A);
}

void abi_SubStepPickupErase072B04A(void)
{
    SubStepPickupErase072B04A(cpu.x);
}

/*
 * @routine 088C:7EE SubStepPickupHitCheck088C7EE
 * @run 70:BFEE
 * WHAT IT DOES: The pickup's hit (Secret stage 04): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck088C7EE(uint8_t slot)
{
    hit_check(slot, 0xBFEE);
}

void abi_SubStepPickupHitCheck088C7EE(void)
{
    SubStepPickupHitCheck088C7EE(cpu.x);
}

/*
 * @routine 088C:7CB SubStepPickup088C7CB
 * @run 70:BFCB
 * WHAT IT DOES: The pickup of Secret stage 04: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck088C7EE, then JumpByStateTableBody with the table at $BFE0.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup088C7CB(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck088C7EE, 0xBFCB);
}

void abi_SubStepPickup088C7CB(void)
{
    Jump jump = SubStepPickup088C7CB(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088D:00E SubStepPickupStart088D00E
 * @run 71:C00E
 * WHAT IT DOES: Starts the pickup (Secret stage 04): its record, a timer of $C0, its place, list channel $84 and script
 * $83, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart088D00E(uint8_t slot)
{
    static const PickupStart start = { 0x07, true, 0x80, false, false, 0x30, 0x84, 5, 0x83, 5 };

    pickup_start(slot, &start, 0xC00E);
}

void abi_SubStepPickupStart088D00E(void)
{
    SubStepPickupStart088D00E(cpu.x);
}

/*
 * @routine 088D:054 SubStepPickupWait088D054
 * @run 71:C054
 * WHAT IT DOES: The pickup waits (Secret stage 04): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $C068; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait088D054(uint8_t slot)
{
    pickup_wait(slot, 0xC068, 0xC054);
}

void abi_SubStepPickupWait088D054(void)
{
    SubStepPickupWait088D054(cpu.x);
}

/*
 * @routine 088D:071 SubStepPickupFirstScripts088D071
 * @run 71:C071
 * WHAT IT DOES: The pickup's first round (Secret stage 04): each time its frame list ends, script $84 plus its count is
 * started; after 6, the count is cleared, the frame set to $4C and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts088D071(uint8_t slot)
{
    pickup_first_scripts(slot, 0xC068, 0x84, 5, 6, 0x4C, 0xC071);
}

void abi_SubStepPickupFirstScripts088D071(void)
{
    SubStepPickupFirstScripts088D071(cpu.x);
}

/*
 * @routine 088D:0CA SubStepPickupSpawn088D0CA
 * @run 71:C0CA
 * WHAT IT DOES: Starts up to eight objects of kind $2E in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn088D0CA(void)
{
    pickup_spawn(0x2E, 0xC0CA);
}

void abi_SubStepPickupSpawn088D0CA(void)
{
    SubStepPickupSpawn088D0CA();
}

/*
 * @routine 088D:0A1 SubStepPickupSecondScripts088D0A1
 * @run 71:C0A1
 * WHAT IT DOES: The pickup's second round (Secret stage 04): every fourth frame script $8A plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn088D0CA.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts088D0A1(uint8_t slot)
{
    pickup_second_scripts(slot, 0x8A, 5, 8, SubStepPickupSpawn088D0CA, false, 0xC0A1);
}

void abi_SubStepPickupSecondScripts088D0A1(void)
{
    SubStepPickupSecondScripts088D0A1(cpu.x);
}

/*
 * @routine 088D:0E4 SubStepPickupFall088D0E4
 * @run 71:C0E4
 * WHAT IT DOES: The pickup falls (Secret stage 04): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall088D0E4(uint8_t slot)
{
    pickup_fall(slot, 0xC117, 0x2C, 0xC0E4);
}

void abi_SubStepPickupFall088D0E4(void)
{
    SubStepPickupFall088D0E4(cpu.x);
}

/*
 * @routine 088D:0FF SubStepPickupRest088D0FF
 * @run 71:C0FF
 * WHAT IT DOES: The landed pickup (Secret stage 04): its frames play; on frame $58 sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest088D0FF(uint8_t slot)
{
    pickup_rest(slot, 0xC117, 0x58, 0x51, 0xC0FF);
}

void abi_SubStepPickupRest088D0FF(void)
{
    SubStepPickupRest088D0FF(cpu.x);
}

/*
 * @routine 088C:55C ErodeVramWords088C55C
 * @run 70:BD5C
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords088C55C(void)
{
    erode_words(0xBD5C);
}

void abi_ErodeVramWords088C55C(void)
{
    ErodeVramWords088C55C();
}

/*
 * @routine 088C:53B ErodePattern088C53B
 * @run 70:BD3B
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $BD4C into zBiosCl-Ch, then ErodeVramWords088C55C.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern088C53B(void)
{
    erode_pattern(0xBD4C, ErodeVramWords088C55C, 0xBD4B);
}

void abi_ErodePattern088C53B(void)
{
    ErodePattern088C53B();
}

/*
 * @routine 088C:52F ErodePatterns088C52F
 * @run 70:BD2F
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern088C53B, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns088C52F(void)
{
    erode_eight(ErodePattern088C53B, 0xBD34);
}

void abi_ErodePatterns088C52F(void)
{
    ErodePatterns088C52F();
}

/*
 * @routine 088D:156 SubStepPickupErase088D156
 * @run 71:C156
 * WHAT IT DOES: The pickup's last state (Secret stage 04): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns088C52F, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase088D156(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns088C52F, 0xC156);
}

void abi_SubStepPickupErase088D156(void)
{
    SubStepPickupErase088D156(cpu.x);
}

/*
 * @routine 09EC:534 SubStepPickupHitCheck09EC534
 * @run 70:BD34
 * WHAT IT DOES: The pickup's hit (Secret stage 05): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck09EC534(uint8_t slot)
{
    hit_check(slot, 0xBD34);
}

void abi_SubStepPickupHitCheck09EC534(void)
{
    SubStepPickupHitCheck09EC534(cpu.x);
}

/*
 * @routine 09EC:511 SubStepPickup09EC511
 * @run 70:BD11
 * WHAT IT DOES: The pickup of Secret stage 05: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck09EC534, then JumpByStateTableBody with the table at $BD26.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup09EC511(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck09EC534, 0xBD11);
}

void abi_SubStepPickup09EC511(void)
{
    Jump jump = SubStepPickup09EC511(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EC:554 SubStepPickupStart09EC554
 * @run 70:BD54
 * WHAT IT DOES: Starts the pickup (Secret stage 05): its record, a timer of $C0, its place, list channel $81 and script
 * $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart09EC554(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0xC0, true, false, 0x30, 0x81, 5, 0x82, 5 };

    pickup_start(slot, &start, 0xBD54);
}

void abi_SubStepPickupStart09EC554(void)
{
    SubStepPickupStart09EC554(cpu.x);
}

/*
 * @routine 09EC:599 SubStepPickupWait09EC599
 * @run 70:BD99
 * WHAT IT DOES: The pickup waits (Secret stage 05): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $BDAD; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait09EC599(uint8_t slot)
{
    pickup_wait(slot, 0xBDAD, 0xBD99);
}

void abi_SubStepPickupWait09EC599(void)
{
    SubStepPickupWait09EC599(cpu.x);
}

/*
 * @routine 09EC:5B6 SubStepPickupFirstScripts09EC5B6
 * @run 70:BDB6
 * WHAT IT DOES: The pickup's first round (Secret stage 05): each time its frame list ends, script $83 plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts09EC5B6(uint8_t slot)
{
    pickup_first_scripts(slot, 0xBDAD, 0x83, 5, 6, 0x01, 0xBDB6);
}

void abi_SubStepPickupFirstScripts09EC5B6(void)
{
    SubStepPickupFirstScripts09EC5B6(cpu.x);
}

/*
 * @routine 09EC:613 SubStepPickupSpawn09EC613
 * @run 70:BE13
 * WHAT IT DOES: Starts up to eight objects of kind $32 in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn09EC613(void)
{
    pickup_spawn(0x32, 0xBE13);
}

void abi_SubStepPickupSpawn09EC613(void)
{
    SubStepPickupSpawn09EC613();
}

/*
 * @routine 09EC:5E6 SubStepPickupSecondScripts09EC5E6
 * @run 70:BDE6
 * WHAT IT DOES: The pickup's second round (Secret stage 05): every fourth frame script $89 plus its count; after 8,
 * the pieces are spawned, the raster effect turned off,
 * the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn09EC613.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts09EC5E6(uint8_t slot)
{
    pickup_second_scripts(slot, 0x89, 5, 8, SubStepPickupSpawn09EC613, true, 0xBDE6);
}

void abi_SubStepPickupSecondScripts09EC5E6(void)
{
    SubStepPickupSecondScripts09EC5E6(cpu.x);
}

/*
 * @routine 09EC:62D SubStepPickupFall09EC62D
 * @run 70:BE2D
 * WHAT IT DOES: The pickup falls (Secret stage 05): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall09EC62D(uint8_t slot)
{
    pickup_fall(slot, 0xBE60, 0x2C, 0xBE2D);
}

void abi_SubStepPickupFall09EC62D(void)
{
    SubStepPickupFall09EC62D(cpu.x);
}

/*
 * @routine 09EC:648 SubStepPickupRest09EC648
 * @run 70:BE48
 * WHAT IT DOES: The landed pickup (Secret stage 05): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest09EC648(uint8_t slot)
{
    pickup_rest(slot, 0xBE60, 0x0D, 0x51, 0xBE48);
}

void abi_SubStepPickupRest09EC648(void)
{
    SubStepPickupRest09EC648(cpu.x);
}

/*
 * @routine 09ED:150 ErodeVramWords09ED150
 * @run 71:C150
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords09ED150(void)
{
    erode_words(0xC150);
}

void abi_ErodeVramWords09ED150(void)
{
    ErodeVramWords09ED150();
}

/*
 * @routine 09ED:12F ErodePattern09ED12F
 * @run 71:C12F
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $C140 into zBiosCl-Ch, then ErodeVramWords09ED150.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern09ED12F(void)
{
    erode_pattern(0xC140, ErodeVramWords09ED150, 0xC13F);
}

void abi_ErodePattern09ED12F(void)
{
    ErodePattern09ED12F();
}

/*
 * @routine 09ED:123 ErodePatterns09ED123
 * @run 71:C123
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern09ED12F, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns09ED123(void)
{
    erode_eight(ErodePattern09ED12F, 0xC128);
}

void abi_ErodePatterns09ED123(void)
{
    ErodePatterns09ED123();
}

/*
 * @routine 09EC:69F SubStepPickupErase09EC69F
 * @run 70:BE9F
 * WHAT IT DOES: The pickup's last state (Secret stage 05): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns09ED123, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase09EC69F(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns09ED123, 0xBE9F);
}

void abi_SubStepPickupErase09EC69F(void)
{
    SubStepPickupErase09EC69F(cpu.x);
}

/*
 * @routine 0B4B:13E SubStepPickupHitCheck0B4B13E
 * @run 70:B13E
 * WHAT IT DOES: The pickup's hit (Secret stage 06): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck0B4B13E(uint8_t slot)
{
    hit_check(slot, 0xB13E);
}

void abi_SubStepPickupHitCheck0B4B13E(void)
{
    SubStepPickupHitCheck0B4B13E(cpu.x);
}

/*
 * @routine 0B4B:11B SubStepPickup0B4B11B
 * @run 70:B11B
 * WHAT IT DOES: The pickup of Secret stage 06: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck0B4B13E, then JumpByStateTableBody with the table at $B130.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup0B4B11B(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck0B4B13E, 0xB11B);
}

void abi_SubStepPickup0B4B11B(void)
{
    Jump jump = SubStepPickup0B4B11B(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4B:15E SubStepPickupStart0B4B15E
 * @run 70:B15E
 * WHAT IT DOES: Starts the pickup (Secret stage 06): its record, a timer of $C0, its place, list channel $82 and script
 * $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart0B4B15E(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0xC0, true, false, 0x30, 0x82, 5, 0x82, 5 };

    pickup_start(slot, &start, 0xB15E);
}

void abi_SubStepPickupStart0B4B15E(void)
{
    SubStepPickupStart0B4B15E(cpu.x);
}

/*
 * @routine 0B4B:1A3 SubStepPickupWait0B4B1A3
 * @run 70:B1A3
 * WHAT IT DOES: The pickup waits (Secret stage 06): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $B1B7; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait0B4B1A3(uint8_t slot)
{
    pickup_wait(slot, 0xB1B7, 0xB1A3);
}

void abi_SubStepPickupWait0B4B1A3(void)
{
    SubStepPickupWait0B4B1A3(cpu.x);
}

/*
 * @routine 0B4B:1C0 SubStepPickupFirstScripts0B4B1C0
 * @run 70:B1C0
 * WHAT IT DOES: The pickup's first round (Secret stage 06): each time its frame list ends, script $83 plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts0B4B1C0(uint8_t slot)
{
    pickup_first_scripts(slot, 0xB1B7, 0x83, 5, 6, 0x01, 0xB1C0);
}

void abi_SubStepPickupFirstScripts0B4B1C0(void)
{
    SubStepPickupFirstScripts0B4B1C0(cpu.x);
}

/*
 * @routine 0B4B:219 SubStepPickupSpawn0B4B219
 * @run 70:B219
 * WHAT IT DOES: Starts up to eight objects of kind $22 in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn0B4B219(void)
{
    pickup_spawn(0x22, 0xB219);
}

void abi_SubStepPickupSpawn0B4B219(void)
{
    SubStepPickupSpawn0B4B219();
}

/*
 * @routine 0B4B:1F0 SubStepPickupSecondScripts0B4B1F0
 * @run 70:B1F0
 * WHAT IT DOES: The pickup's second round (Secret stage 06): every fourth frame script $89 plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn0B4B219.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts0B4B1F0(uint8_t slot)
{
    pickup_second_scripts(slot, 0x89, 5, 8, SubStepPickupSpawn0B4B219, false, 0xB1F0);
}

void abi_SubStepPickupSecondScripts0B4B1F0(void)
{
    SubStepPickupSecondScripts0B4B1F0(cpu.x);
}

/*
 * @routine 0B4B:233 SubStepPickupFall0B4B233
 * @run 70:B233
 * WHAT IT DOES: The pickup falls (Secret stage 06): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall0B4B233(uint8_t slot)
{
    pickup_fall(slot, 0xB266, 0x2C, 0xB233);
}

void abi_SubStepPickupFall0B4B233(void)
{
    SubStepPickupFall0B4B233(cpu.x);
}

/*
 * @routine 0B4B:24E SubStepPickupRest0B4B24E
 * @run 70:B24E
 * WHAT IT DOES: The landed pickup (Secret stage 06): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest0B4B24E(uint8_t slot)
{
    pickup_rest(slot, 0xB266, 0x0D, 0x51, 0xB24E);
}

void abi_SubStepPickupRest0B4B24E(void)
{
    SubStepPickupRest0B4B24E(cpu.x);
}

/*
 * @routine 0B4A:355 ErodeVramWords0B4A355
 * @run 70:AB55
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords0B4A355(void)
{
    erode_words(0xAB55);
}

void abi_ErodeVramWords0B4A355(void)
{
    ErodeVramWords0B4A355();
}

/*
 * @routine 0B4A:334 ErodePattern0B4A334
 * @run 70:AB34
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $AB45 into zBiosCl-Ch, then ErodeVramWords0B4A355.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern0B4A334(void)
{
    erode_pattern(0xAB45, ErodeVramWords0B4A355, 0xAB44);
}

void abi_ErodePattern0B4A334(void)
{
    ErodePattern0B4A334();
}

/*
 * @routine 0B4A:328 ErodePatterns0B4A328
 * @run 70:AB28
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern0B4A334, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns0B4A328(void)
{
    erode_eight(ErodePattern0B4A334, 0xAB2D);
}

void abi_ErodePatterns0B4A328(void)
{
    ErodePatterns0B4A328();
}

/*
 * @routine 0B4B:2A5 SubStepPickupErase0B4B2A5
 * @run 70:B2A5
 * WHAT IT DOES: The pickup's last state (Secret stage 06): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns0B4A328, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase0B4B2A5(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns0B4A328, 0xB2A5);
}

void abi_SubStepPickupErase0B4B2A5(void)
{
    SubStepPickupErase0B4B2A5(cpu.x);
}

/*
 * @routine 0CAA:6FF SubStepPickupHitCheck0CAA6FF
 * @run 70:AEFF
 * WHAT IT DOES: The pickup's hit (Secret stage 07): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck0CAA6FF(uint8_t slot)
{
    hit_check(slot, 0xAEFF);
}

void abi_SubStepPickupHitCheck0CAA6FF(void)
{
    SubStepPickupHitCheck0CAA6FF(cpu.x);
}

/*
 * @routine 0CAA:6DC SubStepPickup0CAA6DC
 * @run 70:AEDC
 * WHAT IT DOES: The pickup of Secret stage 07: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck0CAA6FF, then JumpByStateTableBody with the table at $AEF1.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup0CAA6DC(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck0CAA6FF, 0xAEDC);
}

void abi_SubStepPickup0CAA6DC(void)
{
    Jump jump = SubStepPickup0CAA6DC(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAA:71F SubStepPickupStart0CAA71F
 * @run 70:AF1F
 * WHAT IT DOES: Starts the pickup (Secret stage 07): its record, a timer of $C0, its place, list channel $81 and script
 * $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart0CAA71F(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0x80, true, false, 0x30, 0x81, 5, 0x82, 5 };

    pickup_start(slot, &start, 0xAF1F);
}

void abi_SubStepPickupStart0CAA71F(void)
{
    SubStepPickupStart0CAA71F(cpu.x);
}

/*
 * @routine 0CAA:764 SubStepPickupWait0CAA764
 * @run 70:AF64
 * WHAT IT DOES: The pickup waits (Secret stage 07): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $AF78; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait0CAA764(uint8_t slot)
{
    pickup_wait(slot, 0xAF78, 0xAF64);
}

void abi_SubStepPickupWait0CAA764(void)
{
    SubStepPickupWait0CAA764(cpu.x);
}

/*
 * @routine 0CAA:781 SubStepPickupFirstScripts0CAA781
 * @run 70:AF81
 * WHAT IT DOES: The pickup's first round (Secret stage 07): each time its frame list ends, script $83 plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts0CAA781(uint8_t slot)
{
    pickup_first_scripts(slot, 0xAF78, 0x83, 5, 6, 0x01, 0xAF81);
}

void abi_SubStepPickupFirstScripts0CAA781(void)
{
    SubStepPickupFirstScripts0CAA781(cpu.x);
}

/*
 * @routine 0CAA:7DA SubStepPickupSpawn0CAA7DA
 * @run 70:AFDA
 * WHAT IT DOES: Starts up to eight objects of kind $2F in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn0CAA7DA(void)
{
    pickup_spawn(0x2F, 0xAFDA);
}

void abi_SubStepPickupSpawn0CAA7DA(void)
{
    SubStepPickupSpawn0CAA7DA();
}

/*
 * @routine 0CAA:7B1 SubStepPickupSecondScripts0CAA7B1
 * @run 70:AFB1
 * WHAT IT DOES: The pickup's second round (Secret stage 07): every fourth frame script $89 plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn0CAA7DA.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts0CAA7B1(uint8_t slot)
{
    pickup_second_scripts(slot, 0x89, 5, 8, SubStepPickupSpawn0CAA7DA, false, 0xAFB1);
}

void abi_SubStepPickupSecondScripts0CAA7B1(void)
{
    SubStepPickupSecondScripts0CAA7B1(cpu.x);
}

/*
 * @routine 0CAA:7F4 SubStepPickupFall0CAA7F4
 * @run 70:AFF4
 * WHAT IT DOES: The pickup falls (Secret stage 07): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall0CAA7F4(uint8_t slot)
{
    pickup_fall(slot, 0xB027, 0x2C, 0xAFF4);
}

void abi_SubStepPickupFall0CAA7F4(void)
{
    SubStepPickupFall0CAA7F4(cpu.x);
}

/*
 * @routine 0CAB:00F SubStepPickupRest0CAB00F
 * @run 70:B00F
 * WHAT IT DOES: The landed pickup (Secret stage 07): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest0CAB00F(uint8_t slot)
{
    pickup_rest(slot, 0xB027, 0x0D, 0x51, 0xB00F);
}

void abi_SubStepPickupRest0CAB00F(void)
{
    SubStepPickupRest0CAB00F(cpu.x);
}

/*
 * @routine 0CAA:367 ErodeVramWords0CAA367
 * @run 70:AB67
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords0CAA367(void)
{
    erode_words(0xAB67);
}

void abi_ErodeVramWords0CAA367(void)
{
    ErodeVramWords0CAA367();
}

/*
 * @routine 0CAA:346 ErodePattern0CAA346
 * @run 70:AB46
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $AB57 into zBiosCl-Ch, then ErodeVramWords0CAA367.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern0CAA346(void)
{
    erode_pattern(0xAB57, ErodeVramWords0CAA367, 0xAB56);
}

void abi_ErodePattern0CAA346(void)
{
    ErodePattern0CAA346();
}

/*
 * @routine 0CAA:33A ErodePatterns0CAA33A
 * @run 70:AB3A
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern0CAA346, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns0CAA33A(void)
{
    erode_eight(ErodePattern0CAA346, 0xAB3F);
}

void abi_ErodePatterns0CAA33A(void)
{
    ErodePatterns0CAA33A();
}

/*
 * @routine 0CAB:066 SubStepPickupErase0CAB066
 * @run 70:B066
 * WHAT IT DOES: The pickup's last state (Secret stage 07): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns0CAA33A, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase0CAB066(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns0CAA33A, 0xB066);
}

void abi_SubStepPickupErase0CAB066(void)
{
    SubStepPickupErase0CAB066(cpu.x);
}

/*
 * @routine 0E0A:4C3 SubStepPickupHitCheck0E0A4C3
 * @run 70:ACC3
 * WHAT IT DOES: The pickup's hit (Secret stage 08): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck0E0A4C3(uint8_t slot)
{
    hit_check(slot, 0xACC3);
}

void abi_SubStepPickupHitCheck0E0A4C3(void)
{
    SubStepPickupHitCheck0E0A4C3(cpu.x);
}

/*
 * @routine 0E0A:4A0 SubStepPickup0E0A4A0
 * @run 70:ACA0
 * WHAT IT DOES: The pickup of Secret stage 08: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck0E0A4C3, then JumpByStateTableBody with the table at $ACB5.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup0E0A4A0(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck0E0A4C3, 0xACA0);
}

void abi_SubStepPickup0E0A4A0(void)
{
    Jump jump = SubStepPickup0E0A4A0(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0E0A:4E3 SubStepPickupStart0E0A4E3
 * @run 70:ACE3
 * WHAT IT DOES: Starts the pickup (Secret stage 08): its record, a timer of $C0, its place, list channel $81 and script
 * $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart0E0A4E3(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0xC0, true, false, 0x30, 0x81, 5, 0x82, 5 };

    pickup_start(slot, &start, 0xACE3);
}

void abi_SubStepPickupStart0E0A4E3(void)
{
    SubStepPickupStart0E0A4E3(cpu.x);
}

/*
 * @routine 0E0A:528 SubStepPickupWait0E0A528
 * @run 70:AD28
 * WHAT IT DOES: The pickup waits (Secret stage 08): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $AD3C; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait0E0A528(uint8_t slot)
{
    pickup_wait(slot, 0xAD3C, 0xAD28);
}

void abi_SubStepPickupWait0E0A528(void)
{
    SubStepPickupWait0E0A528(cpu.x);
}

/*
 * @routine 0E0A:545 SubStepPickupFirstScripts0E0A545
 * @run 70:AD45
 * WHAT IT DOES: The pickup's first round (Secret stage 08): each time its frame list ends, script $83 plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts0E0A545(uint8_t slot)
{
    pickup_first_scripts(slot, 0xAD3C, 0x83, 5, 6, 0x01, 0xAD45);
}

void abi_SubStepPickupFirstScripts0E0A545(void)
{
    SubStepPickupFirstScripts0E0A545(cpu.x);
}

/*
 * @routine 0E0A:59E SubStepPickupSpawn0E0A59E
 * @run 70:AD9E
 * WHAT IT DOES: Starts up to eight objects of kind $22 in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn0E0A59E(void)
{
    pickup_spawn(0x22, 0xAD9E);
}

void abi_SubStepPickupSpawn0E0A59E(void)
{
    SubStepPickupSpawn0E0A59E();
}

/*
 * @routine 0E0A:575 SubStepPickupSecondScripts0E0A575
 * @run 70:AD75
 * WHAT IT DOES: The pickup's second round (Secret stage 08): every fourth frame script $89 plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn0E0A59E.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts0E0A575(uint8_t slot)
{
    pickup_second_scripts(slot, 0x89, 5, 8, SubStepPickupSpawn0E0A59E, false, 0xAD75);
}

void abi_SubStepPickupSecondScripts0E0A575(void)
{
    SubStepPickupSecondScripts0E0A575(cpu.x);
}

/*
 * @routine 0E0A:5B8 SubStepPickupFall0E0A5B8
 * @run 70:ADB8
 * WHAT IT DOES: The pickup falls (Secret stage 08): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall0E0A5B8(uint8_t slot)
{
    pickup_fall(slot, 0xADEB, 0x2C, 0xADB8);
}

void abi_SubStepPickupFall0E0A5B8(void)
{
    SubStepPickupFall0E0A5B8(cpu.x);
}

/*
 * @routine 0E0A:5D3 SubStepPickupRest0E0A5D3
 * @run 70:ADD3
 * WHAT IT DOES: The landed pickup (Secret stage 08): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest0E0A5D3(uint8_t slot)
{
    pickup_rest(slot, 0xADEB, 0x0D, 0x51, 0xADD3);
}

void abi_SubStepPickupRest0E0A5D3(void)
{
    SubStepPickupRest0E0A5D3(cpu.x);
}

/*
 * @routine 0E0A:39E ErodeVramWords0E0A39E
 * @run 70:AB9E
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords0E0A39E(void)
{
    erode_words(0xAB9E);
}

void abi_ErodeVramWords0E0A39E(void)
{
    ErodeVramWords0E0A39E();
}

/*
 * @routine 0E0A:37D ErodePattern0E0A37D
 * @run 70:AB7D
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $AB8E into zBiosCl-Ch, then ErodeVramWords0E0A39E.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern0E0A37D(void)
{
    erode_pattern(0xAB8E, ErodeVramWords0E0A39E, 0xAB8D);
}

void abi_ErodePattern0E0A37D(void)
{
    ErodePattern0E0A37D();
}

/*
 * @routine 0E0A:371 ErodePatterns0E0A371
 * @run 70:AB71
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern0E0A37D, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns0E0A371(void)
{
    erode_eight(ErodePattern0E0A37D, 0xAB76);
}

void abi_ErodePatterns0E0A371(void)
{
    ErodePatterns0E0A371();
}

/*
 * @routine 0E0A:62A SubStepPickupErase0E0A62A
 * @run 70:AE2A
 * WHAT IT DOES: The pickup's last state (Secret stage 08): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns0E0A371, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase0E0A62A(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns0E0A371, 0xAE2A);
}

void abi_SubStepPickupErase0E0A62A(void)
{
    SubStepPickupErase0E0A62A(cpu.x);
}

/*
 * @routine 0F6A:5EF SubStepPickupHitCheck0F6A5EF
 * @run 70:ADEF
 * WHAT IT DOES: The pickup's hit (Secret stage 09): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck0F6A5EF(uint8_t slot)
{
    hit_check(slot, 0xADEF);
}

void abi_SubStepPickupHitCheck0F6A5EF(void)
{
    SubStepPickupHitCheck0F6A5EF(cpu.x);
}

/*
 * @routine 0F6A:5CC SubStepPickup0F6A5CC
 * @run 70:ADCC
 * WHAT IT DOES: The pickup of Secret stage 09: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck0F6A5EF, then JumpByStateTableBody with the table at $ADE1.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup0F6A5CC(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck0F6A5EF, 0xADCC);
}

void abi_SubStepPickup0F6A5CC(void)
{
    Jump jump = SubStepPickup0F6A5CC(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0F6A:60F SubStepPickupStart0F6A60F
 * @run 70:AE0F
 * WHAT IT DOES: Starts the pickup (Secret stage 09): its record, a timer of $C0, its place, list channel $81 and script
 * $82, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart0F6A60F(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0xC0, true, false, 0x30, 0x81, 5, 0x82, 5 };

    pickup_start(slot, &start, 0xAE0F);
}

void abi_SubStepPickupStart0F6A60F(void)
{
    SubStepPickupStart0F6A60F(cpu.x);
}

/*
 * @routine 0F6A:654 SubStepPickupWait0F6A654
 * @run 70:AE54
 * WHAT IT DOES: The pickup waits (Secret stage 09): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $AE68; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait0F6A654(uint8_t slot)
{
    pickup_wait(slot, 0xAE68, 0xAE54);
}

void abi_SubStepPickupWait0F6A654(void)
{
    SubStepPickupWait0F6A654(cpu.x);
}

/*
 * @routine 0F6A:671 SubStepPickupFirstScripts0F6A671
 * @run 70:AE71
 * WHAT IT DOES: The pickup's first round (Secret stage 09): each time its frame list ends, script $83 plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts0F6A671(uint8_t slot)
{
    pickup_first_scripts(slot, 0xAE68, 0x83, 5, 6, 0x01, 0xAE71);
}

void abi_SubStepPickupFirstScripts0F6A671(void)
{
    SubStepPickupFirstScripts0F6A671(cpu.x);
}

/*
 * @routine 0F6A:6CA SubStepPickupSpawn0F6A6CA
 * @run 70:AECA
 * WHAT IT DOES: Starts up to eight objects of kind $23 in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn0F6A6CA(void)
{
    pickup_spawn(0x23, 0xAECA);
}

void abi_SubStepPickupSpawn0F6A6CA(void)
{
    SubStepPickupSpawn0F6A6CA();
}

/*
 * @routine 0F6A:6A1 SubStepPickupSecondScripts0F6A6A1
 * @run 70:AEA1
 * WHAT IT DOES: The pickup's second round (Secret stage 09): every fourth frame script $89 plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn0F6A6CA.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts0F6A6A1(uint8_t slot)
{
    pickup_second_scripts(slot, 0x89, 5, 8, SubStepPickupSpawn0F6A6CA, false, 0xAEA1);
}

void abi_SubStepPickupSecondScripts0F6A6A1(void)
{
    SubStepPickupSecondScripts0F6A6A1(cpu.x);
}

/*
 * @routine 0F6A:6E4 SubStepPickupFall0F6A6E4
 * @run 70:AEE4
 * WHAT IT DOES: The pickup falls (Secret stage 09): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall0F6A6E4(uint8_t slot)
{
    pickup_fall(slot, 0xAF17, 0x2C, 0xAEE4);
}

void abi_SubStepPickupFall0F6A6E4(void)
{
    SubStepPickupFall0F6A6E4(cpu.x);
}

/*
 * @routine 0F6A:6FF SubStepPickupRest0F6A6FF
 * @run 70:AEFF
 * WHAT IT DOES: The landed pickup (Secret stage 09): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest0F6A6FF(uint8_t slot)
{
    pickup_rest(slot, 0xAF17, 0x0D, 0x51, 0xAEFF);
}

void abi_SubStepPickupRest0F6A6FF(void)
{
    SubStepPickupRest0F6A6FF(cpu.x);
}

/*
 * @routine 0F6D:69A ErodeVramWords0F6D69A
 * @run 71:C69A
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords0F6D69A(void)
{
    erode_words(0xC69A);
}

void abi_ErodeVramWords0F6D69A(void)
{
    ErodeVramWords0F6D69A();
}

/*
 * @routine 0F6D:679 ErodePattern0F6D679
 * @run 71:C679
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $C68A into zBiosCl-Ch, then ErodeVramWords0F6D69A.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern0F6D679(void)
{
    erode_pattern(0xC68A, ErodeVramWords0F6D69A, 0xC689);
}

void abi_ErodePattern0F6D679(void)
{
    ErodePattern0F6D679();
}

/*
 * @routine 0F6D:66D ErodePatterns0F6D66D
 * @run 71:C66D
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern0F6D679, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns0F6D66D(void)
{
    erode_eight(ErodePattern0F6D679, 0xC672);
}

void abi_ErodePatterns0F6D66D(void)
{
    ErodePatterns0F6D66D();
}

/*
 * @routine 0F6A:756 SubStepPickupErase0F6A756
 * @run 70:AF56
 * WHAT IT DOES: The pickup's last state (Secret stage 09): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns0F6D66D, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase0F6A756(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns0F6D66D, 0xAF56);
}

void abi_SubStepPickupErase0F6A756(void)
{
    SubStepPickupErase0F6A756(cpu.x);
}

/*
 * @routine 10CA:746 SubStepPickupHitCheck10CA746
 * @run 70:AF46
 * WHAT IT DOES: The pickup's hit (Secret stage 0A): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck10CA746(uint8_t slot)
{
    hit_check(slot, 0xAF46);
}

void abi_SubStepPickupHitCheck10CA746(void)
{
    SubStepPickupHitCheck10CA746(cpu.x);
}

/*
 * @routine 10CA:723 SubStepPickup10CA723
 * @run 70:AF23
 * WHAT IT DOES: The pickup of Secret stage 0A: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck10CA746, then JumpByStateTableBody with the table at $AF38.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup10CA723(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck10CA746, 0xAF23);
}

void abi_SubStepPickup10CA723(void)
{
    Jump jump = SubStepPickup10CA723(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 10CA:766 SubStepPickupStart10CA766
 * @run 70:AF66
 * WHAT IT DOES: Starts the pickup (Secret stage 0A): its record, a timer of $C0, its place, list channel $81 and script
 * $A0, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart10CA766(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0x70, true, true, 0x28, 0x81, 5, 0xA0, 8 };

    pickup_start(slot, &start, 0xAF66);
}

void abi_SubStepPickupStart10CA766(void)
{
    SubStepPickupStart10CA766(cpu.x);
}

/*
 * @routine 10CA:7B0 SubStepPickupWait10CA7B0
 * @run 70:AFB0
 * WHAT IT DOES: The pickup waits (Secret stage 0A): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $AFC4; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait10CA7B0(uint8_t slot)
{
    pickup_wait(slot, 0xAFC4, 0xAFB0);
}

void abi_SubStepPickupWait10CA7B0(void)
{
    SubStepPickupWait10CA7B0(cpu.x);
}

/*
 * @routine 10CA:7CD SubStepPickupFirstScripts10CA7CD
 * @run 70:AFCD
 * WHAT IT DOES: The pickup's first round (Secret stage 0A): each time its frame list ends, script $A1 plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts10CA7CD(uint8_t slot)
{
    pickup_first_scripts(slot, 0xAFC4, 0xA1, 8, 6, 0x01, 0xAFCD);
}

void abi_SubStepPickupFirstScripts10CA7CD(void)
{
    SubStepPickupFirstScripts10CA7CD(cpu.x);
}

/*
 * @routine 10CB:026 SubStepPickupSpawn10CB026
 * @run 70:B026
 * WHAT IT DOES: Starts up to eight objects of kind $2A in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn10CB026(void)
{
    pickup_spawn(0x2A, 0xB026);
}

void abi_SubStepPickupSpawn10CB026(void)
{
    SubStepPickupSpawn10CB026();
}

/*
 * @routine 10CA:7FD SubStepPickupSecondScripts10CA7FD
 * @run 70:AFFD
 * WHAT IT DOES: The pickup's second round (Secret stage 0A): every fourth frame script $A7 plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn10CB026.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts10CA7FD(uint8_t slot)
{
    pickup_second_scripts(slot, 0xA7, 5, 8, SubStepPickupSpawn10CB026, false, 0xAFFD);
}

void abi_SubStepPickupSecondScripts10CA7FD(void)
{
    SubStepPickupSecondScripts10CA7FD(cpu.x);
}

/*
 * @routine 10CB:040 SubStepPickupFall10CB040
 * @run 70:B040
 * WHAT IT DOES: The pickup falls (Secret stage 0A): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall10CB040(uint8_t slot)
{
    pickup_fall(slot, 0xB073, 0x2C, 0xB040);
}

void abi_SubStepPickupFall10CB040(void)
{
    SubStepPickupFall10CB040(cpu.x);
}

/*
 * @routine 10CB:05B SubStepPickupRest10CB05B
 * @run 70:B05B
 * WHAT IT DOES: The landed pickup (Secret stage 0A): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest10CB05B(uint8_t slot)
{
    pickup_rest(slot, 0xB073, 0x0D, 0x51, 0xB05B);
}

void abi_SubStepPickupRest10CB05B(void)
{
    SubStepPickupRest10CB05B(cpu.x);
}

/*
 * @routine 10CA:314 ErodeVramWords10CA314
 * @run 70:AB14
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords10CA314(void)
{
    erode_words(0xAB14);
}

void abi_ErodeVramWords10CA314(void)
{
    ErodeVramWords10CA314();
}

/*
 * @routine 10CA:2F3 ErodePattern10CA2F3
 * @run 70:AAF3
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $AB04 into zBiosCl-Ch, then ErodeVramWords10CA314.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern10CA2F3(void)
{
    erode_pattern(0xAB04, ErodeVramWords10CA314, 0xAB03);
}

void abi_ErodePattern10CA2F3(void)
{
    ErodePattern10CA2F3();
}

/*
 * @routine 10CA:2E7 ErodePatterns10CA2E7
 * @run 70:AAE7
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern10CA2F3, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns10CA2E7(void)
{
    erode_eight(ErodePattern10CA2F3, 0xAAEC);
}

void abi_ErodePatterns10CA2E7(void)
{
    ErodePatterns10CA2E7();
}

/*
 * @routine 10CB:0B2 SubStepPickupErase10CB0B2
 * @run 70:B0B2
 * WHAT IT DOES: The pickup's last state (Secret stage 0A): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns10CA2E7, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase10CB0B2(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns10CA2E7, 0xB0B2);
}

void abi_SubStepPickupErase10CB0B2(void)
{
    SubStepPickupErase10CB0B2(cpu.x);
}

/*
 * @routine 13FA:709 SubStepPickupHitCheck13FA709
 * @run 70:AF09
 * WHAT IT DOES: The pickup's hit (Secret stage 0C): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck13FA709(uint8_t slot)
{
    hit_check(slot, 0xAF09);
}

void abi_SubStepPickupHitCheck13FA709(void)
{
    SubStepPickupHitCheck13FA709(cpu.x);
}

/*
 * @routine 13FA:6E6 SubStepPickup13FA6E6
 * @run 70:AEE6
 * WHAT IT DOES: The pickup of Secret stage 0C: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck13FA709, then JumpByStateTableBody with the table at $AEFB.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup13FA6E6(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck13FA709, 0xAEE6);
}

void abi_SubStepPickup13FA6E6(void)
{
    Jump jump = SubStepPickup13FA6E6(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FA:729 SubStepPickupStart13FA729
 * @run 70:AF29
 * WHAT IT DOES: Starts the pickup (Secret stage 0C): its record, a timer of $C0, its place, list channel $85 and script
 * $91, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart13FA729(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0xC0, true, false, 0x30, 0x85, 5, 0x91, 5 };

    pickup_start(slot, &start, 0xAF29);
}

void abi_SubStepPickupStart13FA729(void)
{
    SubStepPickupStart13FA729(cpu.x);
}

/*
 * @routine 13FA:76E SubStepPickupWait13FA76E
 * @run 70:AF6E
 * WHAT IT DOES: The pickup waits (Secret stage 0C): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $AF82; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait13FA76E(uint8_t slot)
{
    pickup_wait(slot, 0xAF82, 0xAF6E);
}

void abi_SubStepPickupWait13FA76E(void)
{
    SubStepPickupWait13FA76E(cpu.x);
}

/*
 * @routine 13FA:78B SubStepPickupFirstScripts13FA78B
 * @run 70:AF8B
 * WHAT IT DOES: The pickup's first round (Secret stage 0C): each time its frame list ends, script $92 plus its count is
 * started; after 6, the count is cleared, the frame set to $02 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts13FA78B(uint8_t slot)
{
    pickup_first_scripts(slot, 0xAF82, 0x92, 5, 6, 0x02, 0xAF8B);
}

void abi_SubStepPickupFirstScripts13FA78B(void)
{
    SubStepPickupFirstScripts13FA78B(cpu.x);
}

/*
 * @routine 13FA:7E8 SubStepPickupSpawn13FA7E8
 * @run 70:AFE8
 * WHAT IT DOES: Starts up to eight objects of kind $44 in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn13FA7E8(void)
{
    pickup_spawn(0x44, 0xAFE8);
}

void abi_SubStepPickupSpawn13FA7E8(void)
{
    SubStepPickupSpawn13FA7E8();
}

/*
 * @routine 13FA:7BB SubStepPickupSecondScripts13FA7BB
 * @run 70:AFBB
 * WHAT IT DOES: The pickup's second round (Secret stage 0C): every fourth frame script $98 plus its count; after 8,
 * the pieces are spawned, the raster effect turned off,
 * the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn13FA7E8.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts13FA7BB(uint8_t slot)
{
    pickup_second_scripts(slot, 0x98, 5, 8, SubStepPickupSpawn13FA7E8, true, 0xAFBB);
}

void abi_SubStepPickupSecondScripts13FA7BB(void)
{
    SubStepPickupSecondScripts13FA7BB(cpu.x);
}

/*
 * @routine 13FB:002 SubStepPickupFall13FB002
 * @run 70:B002
 * WHAT IT DOES: The pickup falls (Secret stage 0C): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall13FB002(uint8_t slot)
{
    pickup_fall(slot, 0xB035, 0x2C, 0xB002);
}

void abi_SubStepPickupFall13FB002(void)
{
    SubStepPickupFall13FB002(cpu.x);
}

/*
 * @routine 13FB:01D SubStepPickupRest13FB01D
 * @run 70:B01D
 * WHAT IT DOES: The landed pickup (Secret stage 0C): its frames play; on frame $0E sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest13FB01D(uint8_t slot)
{
    pickup_rest(slot, 0xB035, 0x0E, 0x51, 0xB01D);
}

void abi_SubStepPickupRest13FB01D(void)
{
    SubStepPickupRest13FB01D(cpu.x);
}

/*
 * @routine 13FD:615 ErodeVramWords13FD615
 * @run 71:C615
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords13FD615(void)
{
    erode_words(0xC615);
}

void abi_ErodeVramWords13FD615(void)
{
    ErodeVramWords13FD615();
}

/*
 * @routine 13FD:5F4 ErodePattern13FD5F4
 * @run 71:C5F4
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $C605 into zBiosCl-Ch, then ErodeVramWords13FD615.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern13FD5F4(void)
{
    erode_pattern(0xC605, ErodeVramWords13FD615, 0xC604);
}

void abi_ErodePattern13FD5F4(void)
{
    ErodePattern13FD5F4();
}

/*
 * @routine 13FD:5E8 ErodePatterns13FD5E8
 * @run 71:C5E8
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern13FD5F4, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns13FD5E8(void)
{
    erode_eight(ErodePattern13FD5F4, 0xC5ED);
}

void abi_ErodePatterns13FD5E8(void)
{
    ErodePatterns13FD5E8();
}

/*
 * @routine 13FB:074 SubStepPickupErase13FB074
 * @run 70:B074
 * WHAT IT DOES: The pickup's last state (Secret stage 0C): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns13FD5E8, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase13FB074(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns13FD5E8, 0xB074);
}

void abi_SubStepPickupErase13FB074(void)
{
    SubStepPickupErase13FB074(cpu.x);
}

/*
 * @routine 16CA:71F SubStepPickupHitCheck16CA71F
 * @run 70:AF1F
 * WHAT IT DOES: The pickup's hit (Secret stage 0E): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck16CA71F(uint8_t slot)
{
    hit_check(slot, 0xAF1F);
}

void abi_SubStepPickupHitCheck16CA71F(void)
{
    SubStepPickupHitCheck16CA71F(cpu.x);
}

/*
 * @routine 16CA:6FC SubStepPickup16CA6FC
 * @run 70:AEFC
 * WHAT IT DOES: The pickup of Secret stage 0E: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck16CA71F, then JumpByStateTableBody with the table at $AF11.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup16CA6FC(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck16CA71F, 0xAEFC);
}

void abi_SubStepPickup16CA6FC(void)
{
    Jump jump = SubStepPickup16CA6FC(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 16CA:73F SubStepPickupStart16CA73F
 * @run 70:AF3F
 * WHAT IT DOES: Starts the pickup (Secret stage 0E): its record, a timer of $C0, its place, list channel $81 and script
 * $87, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart16CA73F(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0x80, false, false, 0x18, 0x81, 5, 0x87, 5 };

    pickup_start(slot, &start, 0xAF3F);
}

void abi_SubStepPickupStart16CA73F(void)
{
    SubStepPickupStart16CA73F(cpu.x);
}

/*
 * @routine 16CA:780 SubStepPickupWait16CA780
 * @run 70:AF80
 * WHAT IT DOES: The pickup waits (Secret stage 0E): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $AF94; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait16CA780(uint8_t slot)
{
    pickup_wait(slot, 0xAF94, 0xAF80);
}

void abi_SubStepPickupWait16CA780(void)
{
    SubStepPickupWait16CA780(cpu.x);
}

/*
 * @routine 16CA:79D SubStepPickupFirstScripts16CA79D
 * @run 70:AF9D
 * WHAT IT DOES: The pickup's first round (Secret stage 0E): each time its frame list ends, script $88 plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts16CA79D(uint8_t slot)
{
    pickup_first_scripts(slot, 0xAF94, 0x88, 5, 6, 0x01, 0xAF9D);
}

void abi_SubStepPickupFirstScripts16CA79D(void)
{
    SubStepPickupFirstScripts16CA79D(cpu.x);
}

/*
 * @routine 16CA:7F6 SubStepPickupSpawn16CA7F6
 * @run 70:AFF6
 * WHAT IT DOES: Starts up to eight objects of kind $2A in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn16CA7F6(void)
{
    pickup_spawn(0x2A, 0xAFF6);
}

void abi_SubStepPickupSpawn16CA7F6(void)
{
    SubStepPickupSpawn16CA7F6();
}

/*
 * @routine 16CA:7CD SubStepPickupSecondScripts16CA7CD
 * @run 70:AFCD
 * WHAT IT DOES: The pickup's second round (Secret stage 0E): every fourth frame script $8E plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn16CA7F6.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts16CA7CD(uint8_t slot)
{
    pickup_second_scripts(slot, 0x8E, 5, 8, SubStepPickupSpawn16CA7F6, false, 0xAFCD);
}

void abi_SubStepPickupSecondScripts16CA7CD(void)
{
    SubStepPickupSecondScripts16CA7CD(cpu.x);
}

/*
 * @routine 16CB:010 SubStepPickupFall16CB010
 * @run 70:B010
 * WHAT IT DOES: The pickup falls (Secret stage 0E): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall16CB010(uint8_t slot)
{
    pickup_fall(slot, 0xB043, 0x2C, 0xB010);
}

void abi_SubStepPickupFall16CB010(void)
{
    SubStepPickupFall16CB010(cpu.x);
}

/*
 * @routine 16CB:02B SubStepPickupRest16CB02B
 * @run 70:B02B
 * WHAT IT DOES: The landed pickup (Secret stage 0E): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest16CB02B(uint8_t slot)
{
    pickup_rest(slot, 0xB043, 0x0D, 0x51, 0xB02B);
}

void abi_SubStepPickupRest16CB02B(void)
{
    SubStepPickupRest16CB02B(cpu.x);
}

/*
 * @routine 16CA:2D7 ErodeVramWords16CA2D7
 * @run 70:AAD7
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords16CA2D7(void)
{
    erode_words(0xAAD7);
}

void abi_ErodeVramWords16CA2D7(void)
{
    ErodeVramWords16CA2D7();
}

/*
 * @routine 16CA:2B6 ErodePattern16CA2B6
 * @run 70:AAB6
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $AAC7 into zBiosCl-Ch, then ErodeVramWords16CA2D7.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern16CA2B6(void)
{
    erode_pattern(0xAAC7, ErodeVramWords16CA2D7, 0xAAC6);
}

void abi_ErodePattern16CA2B6(void)
{
    ErodePattern16CA2B6();
}

/*
 * @routine 16CA:2AA ErodePatterns16CA2AA
 * @run 70:AAAA
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern16CA2B6, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns16CA2AA(void)
{
    erode_eight(ErodePattern16CA2B6, 0xAAAF);
}

void abi_ErodePatterns16CA2AA(void)
{
    ErodePatterns16CA2AA();
}

/*
 * @routine 16CB:082 SubStepPickupErase16CB082
 * @run 70:B082
 * WHAT IT DOES: The pickup's last state (Secret stage 0E): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns16CA2AA, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase16CB082(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns16CA2AA, 0xB082);
}

void abi_SubStepPickupErase16CB082(void)
{
    SubStepPickupErase16CB082(cpu.x);
}

/*
 * @routine 18DA:727 SubStepPickupHitCheck18DA727
 * @run 70:AF27
 * WHAT IT DOES: The pickup's hit (Stage 10): in states under 6, a pending hit of 1 to 4 plays sound $0B and takes it
 * to state 6 for 16 frames.
 * HOW: Compares of the state and OBJ_HIT_TAKEN; SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, or A 6 with the flags of its load.
 */
void SubStepPickupHitCheck18DA727(uint8_t slot)
{
    hit_check(slot, 0xAF27);
}

void abi_SubStepPickupHitCheck18DA727(void)
{
    SubStepPickupHitCheck18DA727(cpu.x);
}

/*
 * @routine 18DA:704 SubStepPickup18DA704
 * @run 70:AF04
 * WHAT IT DOES: The pickup of Stage 10: the hit check, then the handler of its state (start, wait, two rounds of
 * scripts, fall, rest, erase).
 * HOW: SubStepPickupHitCheck18DA727, then JumpByStateTableBody with the table at $AF19.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the handler's high byte, Y the doubled state, C the state's bit 7.
 */
Jump SubStepPickup18DA704(uint8_t slot)
{
    return pickup_dispatch(slot, abi_SubStepPickupHitCheck18DA727, 0xAF04);
}

void abi_SubStepPickup18DA704(void)
{
    Jump jump = SubStepPickup18DA704(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 18DA:747 SubStepPickupStart18DA747
 * @run 70:AF47
 * WHAT IT DOES: Starts the pickup (Stage 10): its record, a timer of $C0, its place, list channel $90 and script
 * $89, the CD faded, zField8E bit 3 set, zFadeSound $5C, and with no time slowing all sound stopped and $5C played.
 * HOW: The calls through the kernel's vectors and cd_fade.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupStart18DA747(uint8_t slot)
{
    static const PickupStart start = { 0x00, false, 0x80, false, false, 0x18, 0x90, 5, 0x89, 8 };

    pickup_start(slot, &start, 0xAF47);
}

void abi_SubStepPickupStart18DA747(void)
{
    SubStepPickupStart18DA747(cpu.x);
}

/*
 * @routine 18DA:788 SubStepPickupWait18DA788
 * @run 70:AF88
 * WHAT IT DOES: The pickup waits (Stage 10): its frames play while its timer counts down, then the next state.
 * HOW: StepObjectFrames on the list at $AF9C; DEC; INC of the state at zero.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, N and Z from the count or the new state.
 */
void SubStepPickupWait18DA788(uint8_t slot)
{
    pickup_wait(slot, 0xAF9C, 0xAF88);
}

void abi_SubStepPickupWait18DA788(void)
{
    SubStepPickupWait18DA788(cpu.x);
}

/*
 * @routine 18DA:7A5 SubStepPickupFirstScripts18DA7A5
 * @run 70:AFA5
 * WHAT IT DOES: The pickup's first round (Stage 10): each time its frame list ends, script $8A plus its count is
 * started; after 6, the count is cleared, the frame set to $01 and the next state.
 * HOW: StepObjectFrames, StartScriptSlotKind1 through its vector, INC and compare.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupFirstScripts18DA7A5(uint8_t slot)
{
    pickup_first_scripts(slot, 0xAF9C, 0x8A, 8, 6, 0x01, 0xAFA5);
}

void abi_SubStepPickupFirstScripts18DA7A5(void)
{
    SubStepPickupFirstScripts18DA7A5(cpu.x);
}

/*
 * @routine 18DA:7FE SubStepPickupSpawn18DA7FE
 * @run 70:AFFE
 * WHAT IT DOES: Starts up to eight objects of kind $31 in free slots from $20, each holding this object's slot.
 * HOW: FindFreeSlotFrom20, StartObjectAtPlace and the slot stored, for X 0 to 7 while a slot is free.
 * WAITS: no.
 * Entry: X object. Exit: X restored, N and Z from it; A, Y, C and V as the calls left them.
 */
void SubStepPickupSpawn18DA7FE(void)
{
    pickup_spawn(0x31, 0xAFFE);
}

void abi_SubStepPickupSpawn18DA7FE(void)
{
    SubStepPickupSpawn18DA7FE();
}

/*
 * @routine 18DA:7D5 SubStepPickupSecondScripts18DA7D5
 * @run 70:AFD5
 * WHAT IT DOES: The pickup's second round (Stage 10): every fourth frame script $90 plus its count; after 8,
 * the pieces are spawned, the object made hittable and the next state.
 * HOW: zWaveCounter's bits, StartScriptSlotKind1, SubStepPickupSpawn18DA7FE.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void SubStepPickupSecondScripts18DA7D5(uint8_t slot)
{
    pickup_second_scripts(slot, 0x90, 5, 8, SubStepPickupSpawn18DA7FE, false, 0xAFD5);
}

void abi_SubStepPickupSecondScripts18DA7D5(void)
{
    SubStepPickupSecondScripts18DA7D5(cpu.x);
}

/*
 * @routine 18DB:018 SubStepPickupFall18DB018
 * @run 70:B018
 * WHAT IT DOES: The pickup falls (Stage 10): its frames play; on landing sound $2C and the next state.
 * HOW: StepObjectFrames, StopOnLanding, SoundCommand through their vectors.
 * WAITS: no.
 * Entry: X object. Exit: as StopOnLanding, or as the sound with N and Z from the new state.
 */
void SubStepPickupFall18DB018(uint8_t slot)
{
    pickup_fall(slot, 0xB04B, 0x2C, 0xB018);
}

void abi_SubStepPickupFall18DB018(void)
{
    SubStepPickupFall18DB018(cpu.x);
}

/*
 * @routine 18DB:033 SubStepPickupRest18DB033
 * @run 70:B033
 * WHAT IT DOES: The landed pickup (Stage 10): its frames play; on frame $0D sound $51.
 * HOW: StepObjectFrames, a compare of the frame, SoundCommand through its vector.
 * WAITS: no.
 * Entry: X object. Exit: as the compare or the sound.
 */
void SubStepPickupRest18DB033(uint8_t slot)
{
    pickup_rest(slot, 0xB04B, 0x0D, 0x51, 0xB033);
}

void abi_SubStepPickupRest18DB033(void)
{
    SubStepPickupRest18DB033(cpu.x);
}

/*
 * @routine 18DA:29F ErodeVramWords18DA29F
 * @run 70:AA9F
 * WHAT IT DOES: Erodes 64 words of VRAM from the address in zBiosCl-Ch: each word's low byte is shifted left and
 * its high byte right.
 * HOW: For each word, the write and read addresses set to it, then read and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address (zBiosCl steps through the 64 words). Exit: A the last word's new high byte, C 1,
 * N and Z from Y restored; X, Y and V kept.
 */
void ErodeVramWords18DA29F(void)
{
    erode_words(0xAA9F);
}

void abi_ErodeVramWords18DA29F(void)
{
    ErodeVramWords18DA29F();
}

/*
 * @routine 18DA:27E ErodePattern18DA27E
 * @run 70:AA7E
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $AA8F into zBiosCl-Ch, then ErodeVramWords18DA29F.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern18DA27E(void)
{
    erode_pattern(0xAA8F, ErodeVramWords18DA29F, 0xAA8E);
}

void abi_ErodePattern18DA27E(void)
{
    ErodePattern18DA27E();
}

/*
 * @routine 18DA:272 ErodePatterns18DA272
 * @run 70:AA72
 * WHAT IT DOES: Erodes the eight patterns.
 * HOW: X 0 to 7 through ErodePattern18DA27E, X kept on the stack.
 * WAITS: no.
 * Entry: nothing. Exit: X restored, N and Z from it; C 1; else as the last pattern.
 */
void ErodePatterns18DA272(void)
{
    erode_eight(ErodePattern18DA27E, 0xAA77);
}

void abi_ErodePatterns18DA272(void)
{
    ErodePatterns18DA272();
}

/*
 * @routine 18DB:08A SubStepPickupErase18DB08A
 * @run 70:B08A
 * WHAT IT DOES: The pickup's last state (Stage 10): its graphics erode while its count runs; then the stage's
 * sub-step becomes 6 and the object is cleared.
 * HOW: ErodePatterns18DA272, DEC of the count, and a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: counting: N and Z from the count; else A 6; X kept, the rest as the erosion.
 */
void SubStepPickupErase18DB08A(uint8_t slot)
{
    pickup_erase(slot, ErodePatterns18DA272, 0xB08A);
}

void abi_SubStepPickupErase18DB08A(void)
{
    SubStepPickupErase18DB08A(cpu.x);
}

/*
 * @routine 05CC:09F ErodeVramWords05CC09F
 * @run 70:B89F
 * WHAT IT DOES: Of the 64 VRAM words from zBiosCl-Ch on, ANDs every fourth one, from the row zTemp00 names, with
 * zTemp01 (low byte) and zTemp02 (high byte); a copy of DissolveWords0E0A260.
 * HOW: For each such word, the write and read addresses set to it, then read, masked and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address, zTemp00-02 the row and masks. Exit: zBiosCl-Ch 64 on; Y kept.
 */
void ErodeVramWords05CC09F(void)
{
    DissolveWordsAt(0xB89F);
}

void abi_ErodeVramWords05CC09F(void)
{
    ErodeVramWords05CC09F();
}

/*
 * @routine 05CC:058 ErodePattern05CC058
 * @run 70:B858
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $B869 into zBiosCl-Ch, then ErodeVramWords05CC09F.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern05CC058(void)
{
    erode_pattern(0xB869, ErodeVramWords05CC09F, 0xB868);
}

void abi_ErodePattern05CC058(void)
{
    ErodePattern05CC058();
}

/*
 * @routine 072C:541 ErodeVramWords072C541
 * @run 70:BD41
 * WHAT IT DOES: Of the 64 VRAM words from zBiosCl-Ch on, ANDs every fourth one, from the row zTemp00 names, with
 * zTemp01 (low byte) and zTemp02 (high byte); a copy of DissolveWords0E0A260.
 * HOW: For each such word, the write and read addresses set to it, then read, masked and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address, zTemp00-02 the row and masks. Exit: zBiosCl-Ch 64 on; Y kept.
 */
void ErodeVramWords072C541(void)
{
    DissolveWordsAt(0xBD41);
}

void abi_ErodeVramWords072C541(void)
{
    ErodeVramWords072C541();
}

/*
 * @routine 072C:520 ErodePattern072C520
 * @run 70:BD20
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $BD31 into zBiosCl-Ch, then ErodeVramWords072C541.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern072C520(void)
{
    erode_pattern(0xBD31, ErodeVramWords072C541, 0xBD30);
}

void abi_ErodePattern072C520(void)
{
    ErodePattern072C520();
}

/*
 * @routine 0B4D:1BD ErodeVramWords0B4D1BD
 * @run 71:C1BD
 * WHAT IT DOES: Of the 64 VRAM words from zBiosCl-Ch on, ANDs every fourth one, from the row zTemp00 names, with
 * zTemp01 (low byte) and zTemp02 (high byte); a copy of DissolveWords0E0A260.
 * HOW: For each such word, the write and read addresses set to it, then read, masked and written back.
 * WAITS: no.
 * Entry: zBiosCl-Ch the address, zTemp00-02 the row and masks. Exit: zBiosCl-Ch 64 on; Y kept.
 */
void ErodeVramWords0B4D1BD(void)
{
    DissolveWordsAt(0xC1BD);
}

void abi_ErodeVramWords0B4D1BD(void)
{
    ErodeVramWords0B4D1BD();
}

/*
 * @routine 0B4D:19E ErodePattern0B4D19E
 * @run 71:C19E
 * WHAT IT DOES: Erodes pattern X of the eight in the table behind it.
 * HOW: The address from the table at $C1AF into zBiosCl-Ch, then ErodeVramWords0B4D1BD.
 * WAITS: no.
 * Entry: X the pattern (0 to 7). Exit: as the erosion; X kept, Y twice X.
 */
void ErodePattern0B4D19E(void)
{
    erode_pattern(0xC1AF, ErodeVramWords0B4D1BD, 0xC1AE);
}

void abi_ErodePattern0B4D19E(void)
{
    ErodePattern0B4D19E();
}
