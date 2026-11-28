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
#include "calls_b27.h"

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

static void move_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
}

static void move_y(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
}

static void gravity(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
}

/* JSR FindFreeSlotFrom30's vector: true when a slot was found, in Y. */
static bool find_from30(uint16_t ret)
{
    uint8_t found_a, to;
    bool found;

    jsr_push(ret);
    to = FindFreeSlotFrom30(&found_a, &found);
    find_slot_exit_shim(to, found_a, found);
    jsr_pop();
    return found;
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

static bool find_from20(uint16_t ret)
{
    uint8_t found_a, to;
    bool found;

    jsr_push(ret);
    to = FindFreeSlotFrom20(&found_a, &found);
    find_slot_exit_shim(to, found_a, found);
    jsr_pop();
    return found;
}

static void script_at(uint8_t number, uint8_t at_slot, uint16_t ret)
{
    lda(number);
    cpu.x = at_slot;
    abi_nz(at_slot);
    jsr_push(ret);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
}

static void pull_x(void)
{
    cpu.x = pull8();
    abi_nz(cpu.x);
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

static void near_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
}
/*
 * @routine 0F6C:135 ShardByArgArc0F6C
 * @run 70:B935
 * WHAT IT DOES: A shard flung by its start argument, falling at most 3 a count.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ShardByArgArc0F6C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xB93D);
        lda(0x10);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        ldy(RD8(OBJ->start_arg[slot]));
        lda(rd8((uint16_t)(0xBA03 + cpu.y)));
        WR8(OBJ->frame[slot], cpu.a);
        lda(rd8((uint16_t)(0xB9D9 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xB9E0 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        move_x(slot, 0xB95A);
        move_y(slot, 0xB95D);
        lda(RD8(OBJ->start_arg[slot]));
        word_index(cpu.a);
        lda(rd8((uint16_t)(0xB9E7 + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(rd8((uint16_t)(0xB9E8 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xB9F5 + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(rd8((uint16_t)(0xB9F6 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->vy_hi[slot]));
    if ((cpu.p & P_N) == 0) {
        cmp(cpu.a, 0x01);
        if ((cpu.p & P_N) == 0) {
            call_routine(0xB987, 0xB9A8);
        }
    }
    move_x(slot, 0xB98A);
    move_y(slot, 0xB98D);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x10, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    if ((cpu.p & P_N) != 0) {
        return;
    }
    cmp(cpu.a, 0x03);
    if (cpu.a < 0x03) {
        return;
    }
    WR8(OBJ->vy_lo[slot], 0);
}

void abi_ShardByArgArc0F6C(void)
{
    ShardByArgArc0F6C(cpu.x);
}

/*
 * @routine 161B:1DE LoadFlyInByArg161B
 * @run 70:B1DE
 * WHAT IT DOES: Flies in from the right, or now and then from the left, above slot 0, sometimes with a kind $2D rider.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFlyInByArg161B(uint8_t slot)
{
    load_record(slot, 0xB1E1);
    lda(0x04);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(0xF0);
    WR8(OBJ->work_f[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a == 0) {
        random_a(0xB1F5);
        lda((uint8_t)(cpu.a & 0x03));
    } else {
        goto right;
    }
    if (cpu.a == 0) {
        goto right;
    }
    lda(0x01);
    WR8(OBJ->facing_left[slot], cpu.a);
    lda(0xFF);
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(0xF0);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0x00);
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->vx_lo[slot], cpu.a);
    goto both;
right:
    lda(0x01);
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(0xFE);
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->vx_lo[slot], cpu.a);
both:
    lda(RD8(OBJ->y_lo[0]));
    add_a((uint8_t)~0x10, true);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(RD8(OBJ->y_hi[0]));
    add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->y_hi[slot], cpu.a);
    random_a(0xB23C);
    lda((uint8_t)(cpu.a & 0x03));
    if (cpu.a != 0) {
        return;
    }
    if (!find_from30(0xB243)) {
        return;
    }
    start_at_place(0x2D, 0xB24A);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    lda(cpu.x);
    WR8(OBJ->work_d[cpu.y], cpu.a);
}

void abi_LoadFlyInByArg161B(void)
{
    LoadFlyInByArg161B(cpu.x);
}

/*
 * @routine 18DB:5C7 RaiseArmed18DB
 * @run 70:B5C7
 * WHAT IT DOES: Arms itself with its hit box and three numbered kind $25 parts, two scripts, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RaiseArmed18DB(uint8_t slot)
{
    lda(0x59);
    WR8(OBJ->frame[slot], cpu.a);
    lda(0x5C);
    WR8(OBJ->health[slot], cpu.a);
    WR8(OBJ->pace[slot], 0);
    lda(0x03);
    WR8(OBJ->work_a[slot], cpu.a);
    WR8(OBJ->hit_status[slot], 0);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(0xE8);
    WR8(OBJ->hitbox_dx[slot], cpu.a);
    lda(0xF0);
    WR8(OBJ->hitbox_dy[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
    ldy(0x11);
    clear_slot_in_y(0xB5FA);
    start_at_place(0x25, 0xB5FF);
    lda(0x01);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xB608);
    start_at_place(0x25, 0xB60D);
    lda(0x02);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xB616);
    start_at_place(0x25, 0xB61B);
    lda(0x03);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    push8(cpu.x);
    script_at(0x86, 0x05, 0xB628);
    script_at(0x84, 0x04, 0xB62F);
    pull_x();
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_RaiseArmed18DB(void)
{
    RaiseArmed18DB(cpu.x);
}

/*
 * @routine 12DC:16F DebrisFallThenGround12DC
 * @run 70:B96F
 * WHAT IT DOES: Debris spreading by slot until its time or the top of the screen; then kind $9C, with a kind $83 left there.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DebrisFallThenGround12DC(uint8_t slot)
{
    step_list(slot, 0xB9EA, true, 0xB97A);
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) != 0) {
        goto ground;
    }
    lda(RD8(OBJ->y_hi[slot]));
    if ((cpu.p & P_N) != 0) {
        goto ground;
    }
    if (cpu.a == 0) {
        lda(RD8(OBJ->y_lo[slot]));
        cmp(cpu.a, 0x18);
        if (cpu.a < 0x18) {
            goto ground;
        }
    }
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        return;
    }
    move(slot, 0xB997);
    lda(cpu.x);
    add_a((uint8_t)~0x20, true);
    word_index(cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    add_a(rd8((uint16_t)(0xB9EF + cpu.y)), false);
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    add_a(rd8((uint16_t)(0xB9F0 + cpu.y)), (cpu.p & P_C) != 0);
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a((uint8_t)~rd8((uint16_t)(0xBA07 + cpu.y)), true);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    return;
ground:
    lda(0x9C);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
    if (!find_from30(0xB9CE)) {
        return;
    }
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->y_lo[slot]));
        cmp(cpu.a, 0x18);
        if (cpu.a >= 0x18) {
            goto start;
        }
    }
    WR8(OBJ->y_hi[slot], 0);
    lda(0x18);
    WR8(OBJ->y_lo[slot], cpu.a);
start:
    start_at_place(0x83, 0xB9E9);
}

void abi_DebrisFallThenGround12DC(void)
{
    DebrisFallThenGround12DC(cpu.x);
}

/*
 * @routine 0EBF:6CA WeaveThenDive0EBF
 * @run 71:D6CA
 * WHAT IT DOES: Weaves for OBJ_WORK_G counts; unless the player is near then, leaps (sound $CB) and goes on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WeaveThenDive0EBF(uint8_t slot)
{
    step_list(slot, 0xD757, true, 0xD6D5);
    lda(0x80);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_i[slot]));
    jsr_push(0xD6E3);
    scale_exit_shim(ScaleBySine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct2));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8(zProduct1));
    WR8(OBJ->vy_lo[slot], cpu.a);
    move_y(slot, 0xD6F0);
    lda(0xA0);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    jsr_push(0xD6FE);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct2));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8(zProduct1));
    WR8(OBJ->vx_lo[slot], cpu.a);
    move_x(slot, 0xD70B);
    INC8(OBJ->work_i[slot]);
    INC8(OBJ->work_j[slot]);
    abi_nz(DEC8(OBJ->work_g[slot]));
    if ((cpu.p & P_Z) != 0) {
        return;
    }
    near_x(slot, 0xD719);
    cmp(cpu.a, 0x38);
    if (cpu.a < 0x38) {
        return;
    }
    lda(0xF6);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->work_h[slot], cpu.a);
    random_a(0xD72F);
    lda((uint8_t)(cpu.a & 0x0E));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xD748 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    lda(rd8((uint16_t)(0xD747 + cpu.y)));
    WR8(OBJ->work_g[slot], cpu.a);
    sound(0xCB, 0xD743);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_WeaveThenDive0EBF(void)
{
    WeaveThenDive0EBF(cpu.x);
}

/*
 * @routine 0F6D:42D PieceArcThenRest0F6D
 * @run 71:C42D
 * WHAT IT DOES: A piece arcing by its start argument toward the player's side; landing, leaves a kind $9B object and goes.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PieceArcThenRest0F6D(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC435);
        WR8(OBJ->hitbox_on[slot], 0);
        ldy(RD8(OBJ->start_arg[slot]));
        lda(rd8((uint16_t)(0xC4AA + cpu.y)));
        WR8(OBJ->frame[slot], cpu.a);
        lda(RD8(OBJ->start_arg[slot]));
        word_index(cpu.a);
        lda(rd8((uint16_t)(0xC4AC + cpu.y)));
        WR8(OBJ->vx_lo[slot], cpu.a);
        lda(rd8((uint16_t)(0xC4AD + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(rd8((uint16_t)(0xC4B0 + cpu.y)));
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(rd8((uint16_t)(0xC4B1 + cpu.y)));
        WR8(OBJ->vy_hi[slot], cpu.a);
        jsr_push(0xC461);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        lda((uint8_t)(cpu.a & 0x01));
        if (cpu.a != 0) {
            negate_exit_shim(NegateVelocityX(slot));
        }
        return;
    }
    lda((uint8_t)(cpu.a - 1));
    if (cpu.a != 0) {
        abi_nz(DEC8(OBJ->work_d[slot]));
        if ((cpu.p & P_Z) == 0) {
            return;
        }
        ClearObject(slot);
        return;
    }
    move_x(slot, 0xC46F);
    move_y(slot, 0xC472);
    gravity(slot, 0xC475);
    lda(RD8(OBJ->vy_hi[slot]));
    if ((cpu.p & P_N) != 0) {
        return;
    }
    ldy(RD8(OBJ->start_arg[slot]));
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, rd8((uint16_t)(0xC4B4 + cpu.y)));
    if (cpu.a < rd8((uint16_t)(0xC4B4 + cpu.y))) {
        return;
    }
    lda(rd8((uint16_t)(0xC4B4 + cpu.y)));
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(0x10);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    INC8(OBJ->state[slot]);
    (void)find_from20(0xC49B);
    start_at_place(0x9B, 0xC4A0);
}

void abi_PieceArcThenRest0F6D(void)
{
    PieceArcThenRest0F6D(cpu.x);
}

/*
 * @routine 0EBD:351 LoadFromSide0EBD
 * @run 71:C351
 * WHAT IT DOES: Comes in from behind the player at its height, from screen 2 until the scroll's limit; other states go to $C3C9.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadFromSide0EBD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a != 0) {
        run_routine(0xC3C9);
        return;
    }
    load_record(slot, 0xC35C);
    lda(rd8(ScrollXHiScreen));
    cmp(cpu.a, rd8(ScrollXMaxHi));
    if (cpu.a == rd8(ScrollXMaxHi)) {
        ClearObject(slot);
        return;
    }
    lda(rd8(ScrollXHiScreen));
    cmp(cpu.a, 0x02);
    if (cpu.a < 0x02) {
        ClearObject(slot);
        return;
    }
    lda(RD8(OBJ->facing_left[0]));
    if (cpu.a != 0) {
        lda(0x01);
        WR8(OBJ->facing_left[slot], cpu.a);
        lda(0xFF);
        WR8(OBJ->x_hi[slot], cpu.a);
        lda(0xC0);
        WR8(OBJ->x_lo[slot], cpu.a);
        lda(0x01);
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(0x40);
        WR8(OBJ->vx_lo[slot], cpu.a);
    } else {
        lda(0x01);
        WR8(OBJ->x_hi[slot], cpu.a);
        lda(0x40);
        WR8(OBJ->x_lo[slot], cpu.a);
        lda(0xFE);
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(0xC0);
        WR8(OBJ->vx_lo[slot], cpu.a);
    }
    ldy(RD8(OBJ->work_g[slot]));
    lda(rd8((uint16_t)(0xC420 + cpu.y)));
    WR8(OBJ->work_h[slot], cpu.a);
    INC8(OBJ->work_f[slot]);
    lda(RD8(OBJ->y_hi[0]));
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(RD8(OBJ->y_lo[0]));
    WR8(OBJ->y_lo[slot], cpu.a);
    ldy(RD8(OBJ->work_i[slot]));
    lda(rd8((uint16_t)(0xC424 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    axis_move_exit_shim(ApplyVelocityY(slot));
}

void abi_LoadFromSide0EBD(void)
{
    LoadFromSide0EBD(cpu.x);
}
