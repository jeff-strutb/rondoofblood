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
#include "calls_b26.h"

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

static void aim_at_player(uint16_t ret)
{
    AngleExit angle;

    jsr_push(ret);
    angle = AngleToPlayer(cpu.x);
    angle_exit_shim(angle);
    jsr_pop();
}

static void velocity_from_a(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
}

/* ROL of A through the carry. */
static void rol_a(void)
{
    bool out = (cpu.a & 0x80) != 0;

    lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
    abi_flag(P_C, out);
}

/*
 * @routine 0EBE:508 BurstIntoSix0EBE
 * @run 71:CD08
 * WHAT IT DOES: From frame $41 pops off by the facing; then breaks into six kind $3A pieces and goes.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BurstIntoSix0EBE(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x41);
    if (cpu.a >= 0x41) {
        add_a((uint8_t)~0x41, true);
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xCB53 + cpu.y)));
        WR8(OBJ->vx_hi[slot], cpu.a);
        lda(0x20);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(RD8(OBJ->facing_left[slot]));
        if (cpu.a != 0) {
            jsr_push(0xCD26);
            negate_exit_shim(NegateVelocityX(slot));
            jsr_pop();
        }
        move(slot, 0xCD29);
    }
    cpu.y = 0;
    do {
        wr8(zHitSlot, cpu.y);
        if (!find_from30(0xCD2F)) {
            break;
        }
        start_at_place(0x3A, 0xCD36);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        wr8(zSpreadSource, cpu.x);
        lda(rd8(zHitSlot));
        for (int i = 0; i < 3; i++) {
            abi_flag(P_C, (cpu.a & 0x80) != 0);
            lda((uint8_t)(cpu.a << 1));
        }
        cpu.x = cpu.a;
        abi_nz(cpu.x);
        lda(rd8((uint16_t)(0xCD74 + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xCD75 + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xCD76 + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xCD77 + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xCD78 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xCD79 + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        cpu.x = rd8(zSpreadSource);
        abi_nz(cpu.x);
        ldy(rd8(zHitSlot));
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x06);
    } while (cpu.y < 0x06);
    ClearObject(slot);
}

void abi_BurstIntoSix0EBE(void)
{
    BurstIntoSix0EBE(cpu.x);
}

/*
 * @routine 161B:4BE SpitThenRest161B
 * @run 70:B4BE
 * WHAT IT DOES: Faces and waits for OBJ_WORK_D counts, then spits kind $2F objects (sound 7) every eighth count of OBJ_WORK_E.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpitThenRest161B(uint8_t slot)
{
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->work_e[slot]));
        lda((uint8_t)(cpu.a & 0x1F));
        if (cpu.a == 0) {
            face_by_side(slot, COL(OBJ->facing_left), 0xB4CD);
        }
        step_list(slot, 0xB3AE, true, 0xB4DD);
        abi_nz(DEC8(OBJ->work_d[slot]));
        if ((cpu.p & P_Z) == 0) {
            return;
        }
        sound(0x89, 0xB4E7);
        restart_animation(slot);
        lda(0x40);
        WR8(OBJ->work_d[slot], cpu.a);
        abi_nz(INC8(OBJ->substate[slot]));
        return;
    }
    step_list(slot, 0xB3AE, true, 0xB501);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) != 0) {
        DEC8(OBJ->state[slot]);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    if (cpu.a != 0) {
        return;
    }
    if (!find_from30(0xB517)) {
        return;
    }
    sound(0x07, 0xB51E);
    start_at_place(0x2F, 0xB523);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_SpitThenRest161B(void)
{
    SpitThenRest161B(cpu.x);
}

/*
 * @routine 12DD:769 PlatformCarry12DD
 * @run 71:C769
 * WHAT IT DOES: Stood on carries the player (clearing eight bytes of $7D00); struck, pushes by its speed against the player's.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PlatformCarry12DD(uint8_t slot)
{
    lda(0x17);
    WR8(ZP->temp08, cpu.a);
    lda(0x24);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xC774, 0xAA35);
    if ((cpu.p & P_C) == 0) {
        lda(RD8(OBJ->work_f[slot]));
        if (cpu.a != 0) {
            abi_nz(DEC8(OBJ->work_f[slot]));
            return;
        }
        lda(RD8(OBJ->hit_taken[slot]));
        if (cpu.a == 0) {
            return;
        }
        lda(rd8(zPlayerPush));
        lda((uint8_t)(cpu.a | 0x10));
        wr8(zPlayerPush, cpu.a);
        lda(RD8(OBJ->vx_lo[slot]));
        add_a((uint8_t)~RD8(OBJ->vx_lo[0]), true);
        wr8(zPushXLo, cpu.a);
        lda(RD8(OBJ->vx_hi[slot]));
        add_a((uint8_t)~RD8(OBJ->vx_hi[0]), (cpu.p & P_C) != 0);
        wr8(zPushXHi, cpu.a);
        lda(RD8(OBJ->state[slot]));
        cmp(cpu.a, 0x03);
        if (cpu.a < 0x03) {
            return;
        }
        lda(0x08);
        WR8(ZP->product.mid, cpu.a);
        lda(0x02);
        jsr_push(0xC7AB);
        probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
        jsr_pop();
        lda((uint8_t)(cpu.a & 0x02));
        if (cpu.a == 0) {
            return;
        }
        lda(0x02);
        WR8(OBJ->work_c[slot], cpu.a);
        return;
    }
    lda(rd8(zPlayerPush));
    lda((uint8_t)(cpu.a | 0x30));
    wr8(zPlayerPush, cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    wr8(zPushXLo, cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    wr8(zPushXHi, cpu.a);
    lda(0x03);
    WR8(OBJ->work_f[slot], cpu.a);
    ldy(0x2C);
    cpu.a = 0;
    do {
        wr8((uint16_t)(SaveBuffer + cpu.y), cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x34);
    } while (cpu.y < 0x34);
}

void abi_PlatformCarry12DD(void)
{
    PlatformCarry12DD(cpu.x);
}

/*
 * @routine 0EBD:484 LoadAtSideWithPartner0EBD
 * @run 71:C484
 * WHAT IT DOES: Loads at one side of the screen with a kind $2D partner in the next slot; with that slot taken, gone.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAtSideWithPartner0EBD(uint8_t slot)
{
    load_record(slot, 0xC487);
    lda(RD8(OBJ->x_lo[0]));
    cmp(cpu.a, 0xC0);
    if (cpu.a >= 0xC0) {
        lda(RD8(OBJ->x_lo[0]));
        rol_a();
        rol_a();
    } else {
        cmp(cpu.a, 0x40);
        if (cpu.a >= 0x40) {
            random_a(0xC495);
        } else {
            lda(RD8(OBJ->x_lo[0]));
            rol_a();
            rol_a();
        }
    }
    lda((uint8_t)(cpu.a & 0x01));
    word_index((uint8_t)(cpu.a << 1));
    lda(rd8((uint16_t)(0xC4F3 + cpu.y)));
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC4F2 + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xC4F5 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xC4F4 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    WR8(OBJ->y_hi[slot], 0);
    random_a(0xC4BF);
    lda((uint8_t)(cpu.a & 0x07));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xC4FA + cpu.y)));
    WR8(OBJ->y_lo[slot], cpu.a);
    face_by_side(slot, COL(OBJ->facing_left), 0xC4CB);
    lda(cpu.x);
    lda((uint8_t)(cpu.a + 1));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(RD8(OBJ->kind[cpu.y]));
    if (cpu.a != 0) {
        ClearObject(slot);
        return;
    }
    clear_slot_in_y(0xC4DE);
    start_at_place(0x2D, 0xC4E3);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
    lda(cpu.x);
    WR8(OBJ->start_arg[cpu.y], cpu.a);
    lda(cpu.y);
    WR8(OBJ->work_f[slot], cpu.a);
}

void abi_LoadAtSideWithPartner0EBD(void)
{
    LoadAtSideWithPartner0EBD(cpu.x);
}

/*
 * @routine 09EB:2AD SinkOnWater09EB
 * @run 70:B2AD
 * WHAT IT DOES: Carries the player up while rising out of the water; at Y $A4: sound $A3, the water effects off, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SinkOnWater09EB(uint8_t slot)
{
    lda(0x20);
    WR8(ZP->temp08, cpu.a);
    lda(0x2C);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB2B8, 0xAA35);
    if ((cpu.p & P_C) != 0) {
        DEC8(OBJ->y_lo[0]);
        abi_nz(DEC8(OBJ->y_lo[0]));
    }
    lda(rd8(zPlayerPush));
    lda((uint8_t)(cpu.a & 0xF9));
    wr8(zPlayerPush, cpu.a);
    call_routine(0xB2C9, 0xB51E);
    step_list(slot, 0xB31C, true, 0xB2D4);
    call_routine(0xB2D7, 0xB3C4);
    move_y(slot, 0xB2DA);
    lda(RD8(OBJ->work_g[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a == 0) {
        if (find_from30(0xB2E4)) {
            start_at_place(0x2F, 0xB2EB);
        }
    }
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xA4);
    if (cpu.a >= 0xA4) {
        return;
    }
    lda(0x40);
    WR8(OBJ->work_l[slot], cpu.a);
    sound(0xA3, 0xB301);
    lda(0xA4);
    WR8(OBJ->y_lo[slot], cpu.a);
    lda(0x03);
    WR8(OBJ->work_c[slot], cpu.a);
    wr8(LevelYHi, 0);
    wr8(RasterWaterWanted, 0);
    wr8(zRasterEffect, 0);
    wr8(zRasterArg, 0);
    WR8(OBJ->work_i[slot], 0);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_SinkOnWater09EB(void)
{
    SinkOnWater09EB(cpu.x);
}

/*
 * @routine 117B:200 CarrierPlatform117B
 * @run 70:B200
 * WHAT IT DOES: A platform: stood on it goes down and carries the player; the player's flags keep it still.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CarrierPlatform117B(uint8_t slot)
{
    lda(rd8(RichterColumnHit));
    lda((uint8_t)(cpu.a | rd8(RichterColumnHit + 1)));
    lda((uint8_t)(cpu.a | rd8(RichterColumnHit + 2)));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        WR8(OBJ->work_i[slot], 0);
        goto leave;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->work_i[slot]));
        if (cpu.a != 0) {
            abi_nz(DEC8(OBJ->work_i[slot]));
            return;
        }
    }
    lda(0x18);
    WR8(ZP->temp08, cpu.a);
    lda(0x1D);
    WR8(ZP->temp09, cpu.a);
    call_routine(0xB22B, 0xAA35);
    if ((cpu.p & P_C) == 0) {
        goto leave;
    }
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->loop_flags[slot]));
        lda((uint8_t)(cpu.a | 0x01));
        WR8(OBJ->loop_flags[slot], cpu.a);
        lda(0x01);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(0xC0);
        WR8(OBJ->vy_lo[slot], cpu.a);
        INC8(OBJ->y_lo[slot]);
        INC8(OBJ->work_d[slot]);
        lda(0x10);
        WR8(OBJ->work_i[slot], cpu.a);
    }
    call_routine(0xB252, 0xB299);
    lda(rd8(zPlayerPush));
    lda((uint8_t)(cpu.a | 0x30));
    wr8(zPlayerPush, cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    wr8(zPushXLo, cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    wr8(zPushXHi, cpu.a);
    return;
leave:
    lda(RD8(OBJ->loop_flags[slot]));
    lda((uint8_t)(cpu.a & 0xFE));
    WR8(OBJ->loop_flags[slot], cpu.a);
    WR8(OBJ->work_d[slot], 0);
}

void abi_CarrierPlatform117B(void)
{
    CarrierPlatform117B(cpu.x);
}

/*
 * @routine 0F6D:382 RiseSwayThenGo0F6D
 * @run 71:C382
 * WHAT IT DOES: Rises swaying to Y $C4, then waits $10 counts and goes.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RiseSwayThenGo0F6D(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC38A);
        lda(0x08);
        WR8(OBJ->sprite_set[slot], cpu.a);
        lda(0x04);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    lda((uint8_t)(cpu.a - 1));
    if (cpu.a != 0) {
        abi_nz(DEC8(OBJ->work_e[slot]));
        if ((cpu.p & P_Z) == 0) {
            return;
        }
        ClearObject(slot);
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0xC4);
    if (cpu.a >= 0xC4) {
        lda(0xC4);
        WR8(OBJ->y_lo[slot], cpu.a);
        lda(0x10);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        lda(0x10);
        WR8(OBJ->work_e[slot], cpu.a);
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    lda(0x00);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_l[slot]));
    jsr_push(0xC3AD);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct2));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8(zProduct1));
    WR8(OBJ->vx_lo[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->work_m[slot]));
    add_a(0x80, false);
    WR8(OBJ->work_m[slot], cpu.a);
    lda(RD8(OBJ->work_l[slot]));
    add_a(0x02, (cpu.p & P_C) != 0);
    WR8(OBJ->work_l[slot], cpu.a);
    move_x(slot, 0xC3D3);
    move_y(slot, 0xC3D6);
}

void abi_RiseSwayThenGo0F6D(void)
{
    RiseSwayThenGo0F6D(cpu.x);
}

/*
 * @routine 13FD:30D SeekingOrb13FD
 * @run 71:C30D
 * WHAT IT DOES: Turns toward the player as it moves, for its 16-bit lifetime, pulsing OBJ_WORK_G-H (sound $46 each pulse).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SeekingOrb13FD(uint8_t slot)
{
    lda(0xC0);
    WR8(ZP->temp00, cpu.a);
    lda(0x01);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    velocity_from_a(slot, 0xC31B);
    move(slot, 0xC31E);
    aim_at_player(0xC321);
    WR8(ZP->temp02, cpu.a);
    lda(0x01);
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_j[slot]));
    WR8(ZP->temp01, cpu.a);
    jsr_push(0xC32F);
    {
        TurnAngleResult turned = TurnAngleToward();

        turn_angle_exit_shim(turned);
    }
    jsr_pop();
    WR8(OBJ->work_j[slot], cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    add_a((uint8_t)~0x01, true);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->work_d[slot], cpu.a);
    lda((uint8_t)(cpu.a | RD8(OBJ->work_e[slot])));
    if (cpu.a == 0) {
        ClearObject(slot);
        return;
    }
    lda(RD8(OBJ->work_f[slot]));
    add_a(0x04, false);
    WR8(OBJ->work_f[slot], cpu.a);
    WR8(ZP->temp00, 0);
    lda(0x0A);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_k[slot]));
    jsr_push(0xC360);
    scale_exit_shim(ScaleBySine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct1));
    WR8(OBJ->work_h[slot], cpu.a);
    lda(rd8(zProduct2));
    add_a(0x0B, false);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(RD8(OBJ->work_k[slot]));
    add_a(0x02, false);
    WR8(OBJ->work_k[slot], cpu.a);
    if ((cpu.p & P_C) == 0) {
        return;
    }
    sound(0x46, 0xC37D);
}

void abi_SeekingOrb13FD(void)
{
    SeekingOrb13FD(cpu.x);
}
