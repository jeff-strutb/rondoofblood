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
#include "calls_b24.h"

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

/* LDA #kind and JSR StartObjectAtPlace: the object at X's place, in slot Y. */
static void start_at_place(uint8_t kind, uint16_t ret)
{
    lda(kind);
    jsr_push(ret);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
}

static void move_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityX(slot));
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

static void velocity_from_a(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
}

static void near_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
}
/* A tail JMP to LoadObjectRecord's vector. */
static void load_record_tail_(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
}

/*
 * @routine 16CB:315 SwingBack16CB
 * @run 70:B315
 * WHAT IT DOES: Swings by angle OBJ_WORK_E, falling to $E0; reaching Y $B6 the sound is cut and substate 7.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwingBack16CB(uint8_t slot)
{
    step_list(slot, 0xB36A, true, 0xB320);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xB6);
    if (cpu.a >= 0xB6) {
        RMW8(ZP->field8e, old, old | 0x10);
        sound(0xFE, 0xB32E);
        sound(0x72, 0xB333);
        lda(0x07);
        WR8(OBJ->substate[slot], cpu.a);
        lda(0xB6);
        WR8(OBJ->y_lo[slot], cpu.a);
        lda(0x20);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    lda(0x20);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    velocity_from_a(slot, 0xB351);
    move(slot, 0xB354);
    lda(RD8(OBJ->work_e[slot]));
    add_a((uint8_t)~0x02, true);
    WR8(OBJ->work_e[slot], cpu.a);
    cmp(cpu.a, 0xE0);
    if (cpu.a != 0xE0) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SwingBack16CB(void)
{
    SwingBack16CB(cpu.x);
}

/*
 * @routine 16CB:373 SwingForth16CB
 * @run 70:B373
 * WHAT IT DOES: Swings by angle OBJ_WORK_E, rising to $A0; reaching Y $B6 the sound is cut and substate 7.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwingForth16CB(uint8_t slot)
{
    step_list(slot, 0xB36A, true, 0xB37E);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xB6);
    if (cpu.a >= 0xB6) {
        RMW8(ZP->field8e, old, old | 0x10);
        sound(0xFE, 0xB38C);
        sound(0x72, 0xB391);
        lda(0x07);
        WR8(OBJ->substate[slot], cpu.a);
        lda(0xB6);
        WR8(OBJ->y_lo[slot], cpu.a);
        lda(0x20);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    lda(0x20);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    velocity_from_a(slot, 0xB3AF);
    move(slot, 0xB3B2);
    lda(RD8(OBJ->work_e[slot]));
    add_a(0x02, false);
    WR8(OBJ->work_e[slot], cpu.a);
    cmp(cpu.a, 0xA0);
    if (cpu.a != 0xA0) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(DEC8(OBJ->substate[slot]));
}

void abi_SwingForth16CB(void)
{
    SwingForth16CB(cpu.x);
}

/*
 * @routine 161C:399 PaceUntilHit161C
 * @run 70:BB99
 * WHAT IT DOES: Paces, turning at walls and at the screen's edges; a hard hit on an argument with bit 1 goes on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PaceUntilHit161C(uint8_t slot)
{
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a >= 0x05) {
        lda(RD8(OBJ->start_arg[slot]));
        lda((uint8_t)(cpu.a & 0x02));
        if (cpu.a != 0) {
            lda(0x01);
            WR8(OBJ->work_k[slot], cpu.a);
            restart_animation(slot);
            abi_nz(INC8(OBJ->state[slot]));
            return;
        }
    }
    step_list(slot, 0xBBEF, true, 0xBBC1);
    ldy(RD8(OBJ->facing_left[slot]));
    lda(rd8((uint16_t)(0xBBFE + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x08);
    WR8(ZP->product.mid, cpu.a);
    call_routine(0xBBD1, 0xAB3A);
    if ((cpu.p & P_C) == 0) {
        lda(RD8(OBJ->work_j[slot]));
        if (cpu.a != 0) {
            abi_nz(DEC8(OBJ->work_j[slot]));
            return;
        }
        lda(RD8(OBJ->x_hi[slot]));
        if (cpu.a == 0) {
            return;
        }
    }
    lda(RD8(OBJ->facing_left[slot]));
    lda((uint8_t)(cpu.a ^ 0x01));
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->work_j[slot], cpu.a);
}

void abi_PaceUntilHit161C(void)
{
    PaceUntilHit161C(cpu.x);
}

/*
 * @routine 0EBD:0ED FollowHolder0EBD
 * @run 71:C0ED
 * WHAT IT DOES: Held by the previous slot at an offset by its frame; gone when that slot is empty.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FollowHolder0EBD(uint8_t slot)
{
    lda(RD8_IDX((uint16_t)(COL(OBJ->kind) - 1), slot));
    if (cpu.a == 0) {
        ClearObject(slot);
        return;
    }
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC0FD);
    }
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a >= 0x05) {
        sound(0x12, 0xC109);
    }
    lda(RD8_IDX((uint16_t)(COL(OBJ->frame) - 1), slot));
    cmp(cpu.a, 0x13);
    if (cpu.a != 0x13) {
        lda(0x16);
        WR8(ZP->temp00, cpu.a);
        lda(0x04);
        WR8(ZP->temp01, cpu.a);
    } else {
        lda(0x24);
        WR8(ZP->temp00, cpu.a);
        lda(0x14);
        WR8(ZP->temp01, cpu.a);
    }
    lda(RD8(ZP->temp00));
    add_a(RD8_IDX((uint16_t)(COL(OBJ->x_lo) - 1), slot), false);
    WR8(OBJ->x_lo[slot], cpu.a);
    cpu.a = 0;
    add_a(RD8_IDX((uint16_t)(COL(OBJ->x_hi) - 1), slot), (cpu.p & P_C) != 0);
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(RD8(ZP->temp01));
    add_a(RD8_IDX((uint16_t)(COL(OBJ->y_lo) - 1), slot), false);
    WR8(OBJ->y_lo[slot], cpu.a);
    cpu.a = 0;
    add_a(RD8_IDX((uint16_t)(COL(OBJ->y_hi) - 1), slot), (cpu.p & P_C) != 0);
    WR8(OBJ->y_hi[slot], cpu.a);
}

void abi_FollowHolder0EBD(void)
{
    FollowHolder0EBD(cpu.x);
}

/*
 * @routine 0E0C:5D8 LoadShardByArg0E0C
 * @run 70:BDD8
 * WHAT IT DOES: Loads a shard: placed by the start argument (mirrored by facing), then flying at its speeds.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadShardByArg0E0C(uint8_t slot)
{
    load_record(slot, 0xBDDB);
    lda(RD8(OBJ->sprite_attr[slot]));
    lda((uint8_t)(cpu.a | 0x10));
    WR8(OBJ->sprite_attr[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    add_a(0x4E, false);
    WR8(OBJ->frame[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBE2F + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBE30 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    if (cpu.a != 0) {
        jsr_push(0xBE05);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    move(slot, 0xBE08);
    lda(RD8(OBJ->start_arg[slot]));
    word_index((uint8_t)(cpu.a << 1));
    lda(rd8((uint16_t)(0xBE4E + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBE4D + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xBE50 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xBE4F + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(0xBE2E);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
}

void abi_LoadShardByArg0E0C(void)
{
    LoadShardByArg0E0C(cpu.x);
}

/*
 * @routine 117E:521 StrikeByFrame117E
 * @run 71:CD21
 * WHAT IT DOES: Lunges on frames $0C-$0D (sound at $0E) with the next slot's OBJ_WORK_C 3; at the list's end 2 and state 4.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StrikeByFrame117E(uint8_t slot)
{
    lda(0x03);
    WR8(OBJ->work_c[slot + 1], cpu.a);
    step_list(slot, 0xCD80, false, 0xCD31);
    if (list_ended()) {
        lda(0x02);
        WR8(OBJ->work_c[slot + 1], cpu.a);
        restart_animation(slot);
        lda(0x04);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x0E);
    if (cpu.a == 0x0E) {
        sound(0x1A, 0xCD41);
    }
    lda(RD8(OBJ->frame[slot]));
    add_a((uint8_t)~0x0C, true);
    cmp(cpu.a, 0x02);
    if (cpu.a >= 0x02) {
        return;
    }
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    add_a(RD8(ZP->temp00), false);
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xCD79 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xCD78 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    call_routine(0xCD66, 0xCAE7);
}

void abi_StrikeByFrame117E(void)
{
    StrikeByFrame117E(cpu.x);
}

/*
 * @routine 13FB:30F DecideWhenNearOrHit13FB
 * @run 70:B30F
 * WHAT IT DOES: Hit, or the player near for $18 counts: halves OBJ_WORK_G and goes to state 3, OBJ_WORK_K 1 now and then at the right end.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DecideWhenNearOrHit13FB(uint8_t slot)
{
    lda(RD8(OBJ->work_l[slot]));
    if (cpu.a == 0) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    if (cpu.a == 0) {
        near_x(slot, 0xB31C);
        cmp(cpu.a, 0x50);
        if (cpu.a >= 0x50) {
            return;
        }
        INC8(OBJ->work_f[slot]);
        lda(RD8(OBJ->work_f[slot]));
        cmp(cpu.a, 0x18);
        if (cpu.a < 0x18) {
            return;
        }
    }
    {
        uint8_t old = RD8(OBJ->work_g[slot]);

        WR8(OBJ->work_g[slot], (uint8_t)(old >> 1));
        abi_nz((uint8_t)(old >> 1));
        abi_flag(P_C, (old & 1) != 0);
    }
    WR8(OBJ->work_k[slot], 0);
    random_a(0xB334);
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        lda(RD8(OBJ->x_lo[slot]));
        add_a(rd8(ScrollXLo), false);
        WR8(ZP->temp00, cpu.a);
        lda(RD8(OBJ->x_hi[slot]));
        add_a(rd8(ScrollXHiScreen), (cpu.p & P_C) != 0);
        cmp(cpu.a, 0x03);
        if (cpu.a == 0x03) {
            lda(RD8(ZP->temp00));
            if ((cpu.p & P_N) != 0) {
                lda(0x01);
                WR8(OBJ->work_k[slot], cpu.a);
            }
        }
    }
    restart_animation(slot);
    WR8(OBJ->work_f[slot], 0);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_DecideWhenNearOrHit13FB(void)
{
    DecideWhenNearOrHit13FB(cpu.x);
}

/*
 * @routine 12DC:6F6 ChargeCountHits12DC
 * @run 70:BEF6
 * WHAT IT DOES: Charges by the facing; at the list's end counts it, state 2, or state 4 substate 6 on the third.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ChargeCountHits12DC(uint8_t slot)
{
    step_list(slot, 0xBF4E, true, 0xBF01);
    if (!list_ended()) {
        lda(RD8(OBJ->facing_left[slot]));
        word_index(cpu.a);
        lda(rd8((uint16_t)(0xBF56 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xBF55 + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        call_routine(0xBF17, 0xBC5C);
        lda(RD8(OBJ->work_d[slot]));
        add_a(0x04, false);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    lda(0x08);
    WR8(OBJ->invuln_timer[slot], cpu.a);
    {
        uint8_t old = RD8(OBJ->work_k[slot]);

        WR8(OBJ->work_k[slot], (uint8_t)(old >> 1));
        abi_nz((uint8_t)(old >> 1));
        abi_flag(P_C, (old & 1) != 0);
    }
    INC8(OBJ->work_p[slot]);
    lda(RD8(OBJ->work_p[slot]));
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    lda(RD8(OBJ->work_p[slot]));
    cmp(cpu.a, 0x03);
    if (cpu.a < 0x03) {
        return;
    }
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    lda(0x06);
    WR8(OBJ->substate[slot], cpu.a);
    lda(0x20);
    WR8(OBJ->work_k[slot], cpu.a);
    WR8(OBJ->work_p[slot], 0);
}

void abi_ChargeCountHits12DC(void)
{
    ChargeCountHits12DC(cpu.x);
}

/*
 * @routine 09EC:340 ScatterBit09EC
 * @run 70:BB40
 * WHAT IT DOES: A bit thrown at random from the foot of the screen, one of eight frames, cleared below.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ScatterBit09EC(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xBB48);
        random_a(0xBB4B);
        lda((uint8_t)(cpu.a & 0x1F));
        add_a((uint8_t)~0x10, true);
        WR8(OBJ->vx_hi[slot], cpu.a);
        move_x(slot, 0xBB56);
        WR8(OBJ->y_hi[slot], 0);
        lda(0xC0);
        WR8(OBJ->y_lo[slot], cpu.a);
        random_a(0xBB61);
        lda((uint8_t)(cpu.a & 0x0F));
        word_index((uint8_t)(cpu.a << 1));
        lda(rd8((uint16_t)(0xBB9A + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xBB99 + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(rd8((uint16_t)(0xBB9C + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xBB9B + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        random_a(0xBB81);
        lda((uint8_t)(cpu.a & 0x07));
        add_a(0x46, false);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    jsr_push(0xBB8D);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    move(slot, 0xBB90);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        return;
    }
    ClearObject(slot);
}

void abi_ScatterBit09EC(void)
{
    ScatterBit09EC(cpu.x);
}

/*
 * @routine 101E:618 SpawnerPastStart101E
 * @run 71:CE18
 * WHAT IT DOES: Past the first $C0 of the level, starts kind $45 objects in slots $20-$22 at random intervals.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnerPastStart101E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    lda(rd8(ScrollXHiScreen));
    if (cpu.a == 0) {
        lda(rd8(ScrollXLo));
        cmp(cpu.a, 0xC0);
        if (cpu.a < 0xC0) {
            return;
        }
    }
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a | RD8(OBJ->work_e[slot])));
    if (cpu.a != 0) {
        lda(RD8(OBJ->work_e[slot]));
        add_a((uint8_t)~0x01, true);
        WR8(OBJ->work_e[slot], cpu.a);
        lda(RD8(OBJ->work_d[slot]));
        add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    ldy(0x20);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (cpu.a == 0) {
            break;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x23);
        if (cpu.y >= 0x23) {
            random_a(0xCE55);
            lda((uint8_t)(cpu.a & 0x3F));
            add_a(0x40, false);
            WR8(OBJ->work_e[slot], cpu.a);
            return;
        }
    }
    start_at_place(0x45, 0xCE63);
    random_a(0xCE66);
    lda((uint8_t)(cpu.a & 0x3F));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_SpawnerPastStart101E(void)
{
    SpawnerPastStart101E(cpu.x);
}
