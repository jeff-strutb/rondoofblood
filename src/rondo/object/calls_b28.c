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
#include "calls_b28.h"

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

/* BNE after StepObjectFrames: true when the list ended. */
static bool list_ended(void)
{
    return (cpu.p & P_Z) != 0;
}
static void move(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
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

/*
 * @routine 0E0B:42E LungeFromList0E0B
 * @run 70:B42E
 * WHAT IT DOES: At frame $17 lunges (sounds $CD, $1F); at the list's end state 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LungeFromList0E0B(uint8_t slot)
{
    step_list(slot, 0xB463, true, 0xB439);
    if (list_ended()) {
        lda(0x05);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x17);
    if (cpu.a != 0x17) {
        return;
    }
    sound(0xCD, 0xB449);
    sound(0x1F, 0xB44E);
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xB470 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    run_routine(0xB302);
}

void abi_LungeFromList0E0B(void)
{
    LungeFromList0E0B(cpu.x);
}

/*
 * @routine 0E0B:29F WalkByStateTurning0E0B
 * @run 70:B29F
 * WHAT IT DOES: Walks the state's list at OBJ_WORK_E's speed; on turning at a wall takes the next state from a table.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkByStateTurning0E0B(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    add_a((uint8_t)~0x04, true);
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xB2DA + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xB2DB + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step;

        jsr_push(0xB2B4);
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xB2FF + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB2FE + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    call_routine(0xB2CA, 0xB302);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->state[slot]));
    add_a((uint8_t)~0x04, true);
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xB2FC + cpu.y)));
    WR8(OBJ->state[slot], cpu.a);
}

void abi_WalkByStateTurning0E0B(void)
{
    WalkByStateTurning0E0B(cpu.x);
}

/*
 * @routine 12DC:569 SpitWhileList12DC
 * @run 70:BD69
 * WHAT IT DOES: Steers and runs the list, spitting kind $2F at frame $25; at its end state 2.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpitWhileList12DC(uint8_t slot)
{
    call_routine(0xBD6C, 0xBC91);
    call_routine(0xBD6F, 0xBDAF);
    step_list(slot, 0xBDA4, true, 0xBD7A);
    if (list_ended()) {
        lda(0x02);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x25);
    if (cpu.a != 0x25) {
        return;
    }
    sound(0x2E, 0xBD8A);
    ldy(0x50);
    clear_slot_in_y(0xBD8F);
    start_at_place(0x2F, 0xBD94);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_SpitWhileList12DC(void)
{
    SpitWhileList12DC(cpu.x);
}

/*
 * @routine 12DB:45F FlickerIn12DB
 * @run 70:B45F
 * WHAT IT DOES: Flickers in, showing more often as OBJ_WORK_E runs down; then hit-tested.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FlickerIn12DB(uint8_t slot)
{
    step_list(slot, 0xB4A3, true, 0xB46A);
    lda(RD8(OBJ->frame[slot]));
    if (cpu.a != 0) {
        WR8(OBJ->work_g[slot], cpu.a);
    }
    WR8(OBJ->frame[slot], 0);
    lda(RD8(OBJ->work_e[slot]));
    for (int i = 0; i < 4; i++) {
        abi_flag(P_C, (cpu.a & 0x01) != 0);
        lda((uint8_t)(cpu.a >> 1));
    }
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    lda((uint8_t)(cpu.a & rd8((uint16_t)(0xB49B + cpu.y))));
    if (cpu.a != 0) {
        lda(RD8(OBJ->work_g[slot]));
        WR8(OBJ->frame[slot], cpu.a);
    }
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_FlickerIn12DB(void)
{
    FlickerIn12DB(cpu.x);
}

/*
 * @routine 0CAD:066 SparkPop0CAD
 * @run 71:C066
 * WHAT IT DOES: A spark thrown at random, rising until its list ends.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SparkPop0CAD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC06E);
        random_a(0xC071);
        lda((uint8_t)(cpu.a & 0x1F));
        add_a((uint8_t)~0x10, true);
        WR8(OBJ->vx_hi[slot], cpu.a);
        random_a(0xC07C);
        lda((uint8_t)(cpu.a & 0x1F));
        add_a((uint8_t)~0x10, true);
        WR8(OBJ->vy_hi[slot], cpu.a);
        move(slot, 0xC087);
        lda(0xFE);
        WR8(OBJ->vy_hi[slot], cpu.a);
        WR8(OBJ->vy_lo[slot], 0);
        return;
    }
    step_list(slot, 0xC0A3, true, 0xC09B);
    if (list_ended()) {
        ClearObject(slot);
        return;
    }
    axis_move_exit_shim(ApplyVelocityY(slot));
}

void abi_SparkPop0CAD(void)
{
    SparkPop0CAD(cpu.x);
}

/*
 * @routine 10CC:499 BurstRow10CC
 * @run 70:BC99
 * WHAT IT DOES: At step 2 shakes (raster stage 3) and starts six kind $2E objects; at the list's end substate 1.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BurstRow10CC(uint8_t slot)
{
    step_list(slot, 0xBCD6, true, 0xBCA4);
    if (list_ended()) {
        wr8(zRasterStage, 0);
        lda(0x01);
        WR8(OBJ->substate[slot], cpu.a);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x02);
    if (cpu.a != 0x02) {
        return;
    }
    lda(0x03);
    wr8(zRasterStage, cpu.a);
    ldy(0x13);
    do {
        clear_slot_in_y(0xBCB8);
        start_at_place(0x2E, 0xBCBD);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        sound(0xE5, 0xBCC8);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x19);
    } while (cpu.y < 0x19);
}

void abi_BurstRow10CC(void)
{
    BurstRow10CC(cpu.x);
}

/*
 * @routine 117E:45E ListByWorkIStomp117E
 * @run 71:CC5E
 * WHAT IT DOES: Runs the list OBJ_WORK_I picks, stomping (sound $1F) at two frames; at its end state 7 with a random wait.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListByWorkIStomp117E(uint8_t slot)
{
    lda(RD8(OBJ->work_i[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xCC9F + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xCCA0 + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step;

        jsr_push(0xCC70);
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (!list_ended()) {
        if ((cpu.p & P_C) == 0) {
            return;
        }
        lda(RD8(OBJ->frame[slot]));
        cmp(cpu.a, 0x26);
        if (cpu.a != 0x26) {
            cmp(cpu.a, 0x2D);
            if (cpu.a != 0x2D) {
                return;
            }
        }
        lda(0x1F);
        {
            CommandCall call = SoundCommand(cpu.a, cpu.x, cpu.y);

            sound_command_exit_shim(0x1F, call);
        }
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(0x07);
    WR8(OBJ->state[slot], cpu.a);
    jsr_push(0xCC91);
    distance_sum_exit_shim(PlayerDistanceSum(slot, (cpu.p & P_C) != 0));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xCC9B + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
}

void abi_ListByWorkIStomp117E(void)
{
    ListByWorkIStomp117E(cpu.x);
}
