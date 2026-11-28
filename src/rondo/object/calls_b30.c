/*
 * More calling routines of stages $09 to $10 and the secret stages.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "calls_b.h"
#include "calls_b3.h"
#include "calls_b22.h"
#include "calls_b30.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

/* JSR LoadObjectRecord's vector. */
static void load_record(uint8_t slot, uint16_t ret)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(ret);
    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
    jsr_pop();
}

/* LDA #command and JSR SoundCommand's vector. */
static void sound(uint8_t command, uint16_t ret)
{
    CommandCall call;

    lda(command);
    jsr_push(ret);
    call = SoundCommand(command, cpu.x, cpu.y);
    sound_command_exit_shim(command, call);
    jsr_pop();
}

/* The frame list's address into zTemp11 then zTemp10 (or the other way round), and JSR StepObjectFrames. */
static void step_list(uint8_t slot, uint16_t list, bool low_first, uint16_t ret)
{
    uint8_t y = cpu.y;
    FrameStep step;

    if (low_first) {
        lda((uint8_t)list);
        WR8(ZP->temp10, cpu.a);
        lda((uint8_t)(list >> 8));
        WR8(ZP->temp11, cpu.a);
    } else {
        lda((uint8_t)(list >> 8));
        WR8(ZP->temp11, cpu.a);
        lda((uint8_t)list);
        WR8(ZP->temp10, cpu.a);
    }
    jsr_push(ret);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
}

static void restart_animation(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

static void clear_slot_in_y(uint16_t ret)
{
    jsr_push(ret);
    ClearObjectInY(cpu.y);
    jsr_pop();
}

/* LDA #kind and JSR StartObjectAtPlace: the object at X's place, in slot Y. */
static void start_at_place(uint8_t kind, uint16_t ret)
{
    lda(kind);
    jsr_push(ret);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
}

static void move_y(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
}

/* BNE after StepObjectFrames: true when the list ended. */
static bool list_ended(void)
{
    return (cpu.p & P_Z) != 0;
}
static void add_a(uint8_t value, bool carry_in)
{
    Sum sum = add_bytes(cpu.a, value, carry_in);

    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

static void random_a(uint16_t ret)
{
    jsr_push(ret);
    random_exit_shim(NextRandom());
    jsr_pop();
}

static void face_by_side(uint8_t slot, uint16_t column, uint16_t ret)
{
    jsr_push(ret);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    WR8_IDX(column, slot, cpu.a);
}

static void ldy(uint8_t value)
{
    cpu.y = value;
    abi_nz(value);
}

/* LDA, ASL, TAY: a word index from an object column. */
static void word_index(uint8_t value)
{
    abi_flag(P_C, (value & 0x80) != 0);
    lda((uint8_t)(value << 1));
    cpu.y = cpu.a;
}

static void record_event(uint8_t slot, uint16_t ret)
{
    uint8_t stored, event, byte;
    bool marked;

    jsr_push(ret);
    marked = RecordObjectEvent(slot, &stored, &event, &byte);
    abi_ended(l8_of_event(marked, stored, event, byte));
    jsr_pop();
}

/*
 * @routine 18DD:0D1 BubbleAtRandom18DD
 * @run 71:C0D1
 * WHAT IT DOES: A bubble at a random spot with one of four lists, gone when it ends.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BubbleAtRandom18DD(uint8_t slot)
{
    uint8_t y;
    FrameStep step;

    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC0D9);
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        random_a(0xC0E1);
        lda((uint8_t)(cpu.a & 0x03));
        WR8(OBJ->start_arg[slot], cpu.a);
        WR8(OBJ->x_hi[slot], 0);
        WR8(OBJ->y_hi[slot], 0);
        lda(0x78);
        WR8(OBJ->y_lo[slot], cpu.a);
        random_a(0xC0F4);
        lda((uint8_t)(cpu.a & 0x1F));
        WR8(ZP->temp00, cpu.a);
        random_a(0xC0FB);
        add_a((uint8_t)~RD8(ZP->temp00), true);
        add_a(0x10, false);
        WR8(OBJ->x_lo[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xC11D + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xC11E + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    y = cpu.y;
    jsr_push(0xC117);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
    if (!list_ended()) {
        return;
    }
    ClearObject(slot);
}

void abi_BubbleAtRandom18DD(void)
{
    BubbleAtRandom18DD(cpu.x);
}

/*
 * @routine 0EBA:75C WeightPlatform0EBA
 * @run 70:AF5C
 * WHAT IT DOES: Falls to the line its argument gives; stepped on there, dips.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WeightPlatform0EBA(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xAF64);
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x06);
        WR8(OBJ->frame[slot], cpu.a);
        lda(RD8(OBJ->y_lo[slot]));
        WR8(OBJ->work_e[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    call_routine(0xAF7B, 0xAFA9);
    lda(0x18);
    WR8(ZP->temp08, cpu.a);
    lda(0x1D);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xAF86, 0xAA35);
    if ((cpu.p & P_C) == 0) {
        WR8(OBJ->work_d[slot], 0);
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->y_lo[slot]));
        cmp(cpu.a, RD8(OBJ->start_arg[slot]));
        if (cpu.a == RD8(OBJ->start_arg[slot])) {
                lda(0x01);
                WR8(OBJ->vy_hi[slot], cpu.a);
                lda(0xC0);
                WR8(OBJ->vy_lo[slot], cpu.a);
        }
    }
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_WeightPlatform0EBA(void)
{
    WeightPlatform0EBA(cpu.x);
}

/*
 * @routine 18DB:648 SinkThenTurn18DB
 * @run 70:B648
 * WHAT IT DOES: Sinks to Y $78 while its list runs; then allows the player to be, faces the player and goes on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SinkThenTurn18DB(uint8_t slot)
{
    step_list(slot, 0xB695, true, 0xB653);
    if (list_ended()) {
        lda(RD8(ZP->field8e));
        lda((uint8_t)(cpu.a | 0x04));
        WR8(ZP->field8e, cpu.a);
        jsr_push(0xB684);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        lda((uint8_t)(cpu.a & 0x01));
        WR8(OBJ->facing_left[slot], cpu.a);
        lda(0x20);
        WR8(OBJ->work_e[slot], cpu.a);
        INC8(OBJ->state[slot]);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    move_y(slot, 0xB658);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x0C, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x78);
    if (cpu.a < 0x78) {
        return;
    }
    lda(0x78);
    WR8(OBJ->y_lo[slot], cpu.a);
}

void abi_SinkThenTurn18DB(void)
{
    SinkThenTurn18DB(cpu.x);
}

/*
 * @routine 101D:647 HoldAndSwallow101D
 * @run 71:C647
 * WHAT IT DOES: Holds the player hidden at its place; at step 19 starts nine kind $BE objects and drops the player below.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void HoldAndSwallow101D(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a != 0) {
        return;
    }
    WR8(OBJ->frame[0], 0);
    lda(0x02);
    wr8(PlayerStartPose, cpu.a);
    lda(0x03);
    WR8(OBJ->work_e[0], cpu.a);
    WR8(OBJ->work_f[0], 0);
    ldy(0x00);
    jsr_push(0xC661);
    copy_place_exit_shim(CopyObjectPlace(cpu.x, cpu.y));
    jsr_pop();
    step_list(slot, 0xC695, true, 0xC66C);
    if (list_ended()) {
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x13);
    if (cpu.a != 0x13) {
        return;
    }
    ldy(0x10);
    do {
        clear_slot_in_y(0xC67C);
        start_at_place(0xBE, 0xC681);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x19);
    } while (cpu.y < 0x19);
    lda(0x01);
    WR8(OBJ->y_hi[0], cpu.a);
    lda(0x80);
    WR8(OBJ->y_lo[0], cpu.a);
}

void abi_HoldAndSwallow101D(void)
{
    HoldAndSwallow101D(cpu.x);
}

/*
 * @routine 10CC:21C ArmedPace10CC
 * @run 70:BA1C
 * WHAT IT DOES: Paces toward the player, readying an attack when it can (sound $46) or every $60 counts.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ArmedPace10CC(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    if (cpu.a == 0) {
        lda(0x60);
        WR8(OBJ->work_f[slot], cpu.a);
        lda(0x08);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    DEC8(OBJ->work_f[slot]);
    call_routine(0xBA32, 0xBB50);
    if ((cpu.p & P_C) != 0) {
        sound(0x46, 0xBA39);
        restart_animation(slot);
        lda(0x07);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a >= 0x05) {
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    call_routine(0xBA53, 0xBA73);
    face_by_side(slot, COL(OBJ->facing_left), 0xBA56);
    step_list(slot, 0xBA6A, true, 0xBA66);
    call_routine(0xBA69, 0xBAF9);
}

void abi_ArmedPace10CC(void)
{
    ArmedPace10CC(cpu.x);
}

/*
 * @routine 0EBB:014 BellStruck0EBB
 * @run 70:B014
 * WHAT IT DOES: A bell: a hard hit rings it (sound $3A) and records it; $C0 counts on, the state counts on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BellStruck0EBB(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xB01C);
        lda(0x20);
        WR8(OBJ->vy_lo[slot], cpu.a);
        return;
    }
    lda((uint8_t)(cpu.a - 1));
    if (cpu.a == 0) {
        lda(RD8(OBJ->hit_taken[slot]));
        cmp(cpu.a, 0x05);
        if (cpu.a < 0x05) {
            return;
        }
        sound(0x3A, 0xB031);
        record_event(slot, 0xB034);
        lda(0xC0);
        WR8(OBJ->work_d[slot], cpu.a);
        ldy(RD8(OBJ->start_arg[slot]));
        lda(RD8(OBJ->kind[cpu.y]));
        cmp(cpu.a, 0xA1);
        if (cpu.a == 0xA1) {
            lda(RD8(OBJ->state[cpu.y]));
            cmp(cpu.a, 0x02);
            if (cpu.a >= 0x02) {
                lda(0x14);
                wr8((uint16_t)(StageVramBuffer + 2), cpu.a);
                sound(0xCC, 0xB054);
            }
        }
        goto next;
    }
    lda((uint8_t)(cpu.a - 1));
    if (cpu.a != 0) {
        return;
    }
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) != 0) {
        goto next;
    }
    axis_move_exit_shim(ApplyVelocityY(slot));
    return;
next:
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_BellStruck0EBB(void)
{
    BellStruck0EBB(cpu.x);
}

/*
 * @routine 0E0B:11E OpenArenaWithRaster0E0B
 * @run 70:B11E
 * WHAT IT DOES: OpenArena0E0B with raster effect $0B in A: the entry two bytes before it.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void OpenArenaWithRaster0E0B(uint8_t slot)
{
    lda(0x0B);
    OpenArena0E0B(slot);
}

void abi_OpenArenaWithRaster0E0B(void)
{
    OpenArenaWithRaster0E0B(cpu.x);
}
