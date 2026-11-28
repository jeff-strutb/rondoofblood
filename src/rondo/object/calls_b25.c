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
#include "calls_b25.h"

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

/* BNE after StepObjectFrames: true when the list ended. */
static bool list_ended(void)
{
    return (cpu.p & P_Z) != 0;
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
/* ROL of A through the carry. */
static void rol_a(void)
{
    bool out = (cpu.a & 0x80) != 0;

    lda((uint8_t)((cpu.a << 1) | ((cpu.p & P_C) != 0 ? 1 : 0)));
    abi_flag(P_C, out);
}

/*
 * @routine 117D:1E4 ShatterEight117D
 * @run 71:C1E4
 * WHAT IT DOES: Clears the neighbouring parts and scatters 8 pieces with their waits and speeds, then becomes kind $9C.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ShatterEight117D(uint8_t slot)
{
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xC1EA);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        ClearObject(slot);
        return;
    }
    cpu.y = 0;
    do {
        wr8(zHitSlot, cpu.y);
        if (!find_from30(0xC1F8)) {
            break;
        }
        start_at_place(0x2D, 0xC1FF);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        wr8(zSpreadSource, cpu.x);
        lda(rd8(zHitSlot));
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        WR8(ZP->temp00, cpu.a);
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        add_a(RD8(ZP->temp00), false);
        cpu.x = cpu.a;
        abi_nz(cpu.x);
        lda(rd8((uint16_t)(0xC247 + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC248 + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC249 + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC24A + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC24B + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC24C + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        cpu.x = rd8(zSpreadSource);
        abi_nz(cpu.x);
        ldy(rd8(zHitSlot));
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x08);
    } while (cpu.y < 0x08);
    lda(0x9C);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_ShatterEight117D(void)
{
    ShatterEight117D(cpu.x);
}

/*
 * @routine 0EBD:000 ShatterEleven0EBD
 * @run 71:C000
 * WHAT IT DOES: Clears the neighbouring parts and scatters 11 pieces with their waits and speeds, then becomes kind $9C.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ShatterEleven0EBD(uint8_t slot)
{
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xC006);
    cpu.y = (uint8_t)(cpu.y - 1);
    cpu.y = (uint8_t)(cpu.y - 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xC00B);
    cpu.y = 0;
    do {
        wr8(zHitSlot, cpu.y);
        call_routine(0xC011, 0xC0A7);
        if ((cpu.p & P_C) == 0) {
            break;
        }
        start_at_place(0x37, 0xC018);
        lda(RD8(OBJ->facing_left[slot]));
        WR8(OBJ->facing_left[cpu.y], cpu.a);
        wr8(zSpreadSource, cpu.x);
        lda(rd8(zHitSlot));
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        WR8(ZP->temp00, cpu.a);
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        add_a(RD8(ZP->temp00), false);
        cpu.x = cpu.a;
        abi_nz(cpu.x);
        lda(rd8((uint16_t)(0xC065 + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC066 + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC067 + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC068 + cpu.x)));
        WR8(OBJ->vx_lo[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC069 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xC06A + cpu.x)));
        WR8(OBJ->vy_lo[cpu.y], cpu.a);
        cpu.x = rd8(zSpreadSource);
        abi_nz(cpu.x);
        ldy(rd8(zHitSlot));
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x0B);
    } while (cpu.y < 0x0B);
    lda(0x30);
    WR8(OBJ->work_l[0x11], cpu.a);
    lda(0x9C);
    WR8(OBJ->kind[slot], cpu.a);
    WR8(OBJ->state[slot], 0);
}

void abi_ShatterEleven0EBD(void)
{
    ShatterEleven0EBD(cpu.x);
}

/*
 * @routine 161D:7AA SlideWhileList161D
 * @run 71:C7AA
 * WHAT IT DOES: Slides during steps 9-13 (sound at 9), speeding by facing until a wall; at the list's end state 3.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SlideWhileList161D(uint8_t slot)
{
    step_list(slot, 0xC80C, true, 0xC7B5);
    if (list_ended()) {
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x09);
    if (cpu.a < 0x09) {
        return;
    }
    cmp(cpu.a, 0x0E);
    if (cpu.a >= 0x0E) {
        return;
    }
    cmp(cpu.a, 0x09);
    if (cpu.a == 0x09) {
        lda(RD8(OBJ->anim_timer[slot]));
        cmp(cpu.a, 0x10);
        if (cpu.a == 0x10) {
            lda(0x1F);
            jsr_push(0xC7D2);
            sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, cpu.x, cpu.y));
            jsr_pop();
        }
    }
    lda(RD8(OBJ->vx_hi[slot]));
    lda((uint8_t)(cpu.a | RD8(OBJ->vx_lo[slot])));
    if (cpu.a == 0) {
        return;
    }
    lda(0x0C);
    WR8(ZP->product.mid, cpu.a);
    call_routine(0xC7E1, 0xAB3A);
    if ((cpu.p & P_C) != 0) {
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        return;
    }
    lda(RD8(OBJ->facing_left[slot]));
    word_index(cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    add_a(rd8((uint16_t)(0xC7A6 + cpu.y)), false);
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    add_a(rd8((uint16_t)(0xC7A7 + cpu.y)), (cpu.p & P_C) != 0);
    WR8(OBJ->vx_hi[slot], cpu.a);
}

void abi_SlideWhileList161D(void)
{
    SlideWhileList161D(cpu.x);
}

/*
 * @routine 161E:650 BobbingLift161E
 * @run 71:CE50
 * WHAT IT DOES: A lift that carries the player, swinging OBJ_WORK_E between 0 and $A0 and turning at each end.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BobbingLift161E(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xCE58);
        lda(0x50);
        WR8(OBJ->work_e[slot], cpu.a);
        goto speed;
    }
    move_y(slot, 0xCE70);
    call_routine(0xCE73, 0xCEB8);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a != 0) {
        call_routine(0xCE7B, 0xD03A);
    } else {
        call_routine(0xCE80, 0xD052);
    }
    lda(RD8(OBJ->work_d[slot]));
    if (cpu.a != 0) {
        lda(RD8(OBJ->work_f[slot]));
        lda((uint8_t)(cpu.a ^ 0x80));
        WR8(OBJ->work_f[slot], cpu.a);
        if (cpu.a == 0) {
            return;
        }
        INC8(OBJ->work_e[slot]);
        lda(RD8(OBJ->work_e[slot]));
        cmp(cpu.a, 0xA0);
        if (cpu.a != 0xA0) {
            return;
        }
        WR8(OBJ->work_d[slot], 0);
        goto speed;
    }
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a ^ 0x80));
    WR8(OBJ->work_f[slot], cpu.a);
    if (cpu.a == 0) {
        return;
    }
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0x02);
    WR8(OBJ->work_d[slot], cpu.a);
speed:
    ldy(RD8(OBJ->work_d[slot]));
    lda(rd8((uint16_t)(0xCEB4 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xCEB5 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
}

void abi_BobbingLift161E(void)
{
    BobbingLift161E(cpu.x);
}

/*
 * @routine 101D:4E8 OrbitSlot1F_101D
 * @run 71:C4E8
 * WHAT IT DOES: Orbits slot $1F at a widening radius until a hard hit; then state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void OrbitSlot1F_101D(uint8_t slot)
{
    lda(RD8(OBJ->work_e[slot]));
    cmp(cpu.a, 0x78);
    if (cpu.a < 0x78) {
        lda(RD8(OBJ->work_f[slot]));
        add_a(0x40, false);
        WR8(OBJ->work_f[slot], cpu.a);
        lda(RD8(OBJ->work_e[slot]));
        add_a(0x00, (cpu.p & P_C) != 0);
        WR8(OBJ->work_e[slot], cpu.a);
    }
    step_list(slot, 0xC54D, true, 0xC50B);
    ldy(0x1F);
    jsr_push(0xC510);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8(OBJ->work_f[slot]));
    WR8(ZP->temp00, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    velocity_from_a(slot, 0xC520);
    lda(RD8(OBJ->vy_hi[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    {
        uint8_t old = RD8(OBJ->vy_hi[slot]);

        WR8(OBJ->vy_hi[slot], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_flag(P_C, (old & 0x01) != 0);
        old = RD8(OBJ->vy_lo[slot]);
        WR8(OBJ->vy_lo[slot], (uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_nz((uint8_t)((old >> 1) | ((cpu.p & P_C) != 0 ? 0x80 : 0)));
        abi_flag(P_C, (old & 0x01) != 0);
    }
    move(slot, 0xC52D);
    lda(RD8(OBJ->work_d[slot]));
    add_a((uint8_t)~0x01, true);
    WR8(OBJ->work_d[slot], cpu.a);
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    restart_animation(slot);
    INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
}

void abi_OrbitSlot1F_101D(void)
{
    OrbitSlot1F_101D(cpu.x);
}

/*
 * @routine 072C:3D9 RaiseColumn072C
 * @run 70:BBD9
 * WHAT IT DOES: After OBJ_WORK_J: placed under slot 0, the parts in slots $11-$26 spread down 16 apart, then launched upward.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RaiseColumn072C(uint8_t slot)
{
    abi_nz(DEC8(OBJ->work_j[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    sound(0x8A, 0xBBE3);
    ldy(0x00);
    jsr_push(0xBBE8);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(0x01);
    WR8(OBJ->y_hi[slot], cpu.a);
    lda(0x40);
    WR8(OBJ->y_lo[slot], cpu.a);
    call_routine(0xBBF5, 0xB42E);
    lda(0x10);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    ldy(0x11);
    do {
        lda(RD8(OBJ->y_lo[cpu.y]));
        add_a(RD8(ZP->temp00), false);
        WR8(OBJ->y_lo[cpu.y], cpu.a);
        lda(RD8(OBJ->y_hi[cpu.y]));
        add_a(RD8(ZP->temp01), (cpu.p & P_C) != 0);
        WR8(OBJ->y_hi[cpu.y], cpu.a);
        lda(RD8(ZP->temp00));
        add_a(0x10, false);
        WR8(ZP->temp00, cpu.a);
        lda(RD8(ZP->temp01));
        add_a(0x00, (cpu.p & P_C) != 0);
        WR8(ZP->temp01, cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x27);
    } while (cpu.y < 0x27);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(0xF6);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_RaiseColumn072C(void)
{
    RaiseColumn072C(cpu.x);
}

/*
 * @routine 0EC0:2F7 HoverThenSwoop0EC0
 * @run 71:DAF7
 * WHAT IT DOES: Bobs along; near the player, now and then swoops at one of eight speeds (state on).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void HoverThenSwoop0EC0(uint8_t slot)
{
    step_list(slot, 0xDB5D, false, 0xDB02);
    lda(0x40);
    WR8(ZP->temp00, cpu.a);
    lda(0x00);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_d[slot]));
    jsr_push(0xDB10);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct2));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8(zProduct1));
    WR8(OBJ->vy_lo[slot], cpu.a);
    move_y(slot, 0xDB1D);
    INC8(OBJ->work_d[slot]);
    move_x(slot, 0xDB23);
    near_x(slot, 0xDB26);
    cmp(cpu.a, 0x60);
    if (cpu.a >= 0x60) {
        return;
    }
    random_a(0xDB2D);
    cmp(cpu.a, 0x02);
    if (cpu.a >= 0x02) {
        return;
    }
    random_a(0xDB34);
    lda((uint8_t)(cpu.a & 0x07));
    word_index((uint8_t)(cpu.a << 1));
    lda(rd8((uint16_t)(0xDB66 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xDB67 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xDB68 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xDB69 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->facing_left[slot]));
    if (cpu.a != 0) {
        jsr_push(0xDB59);
        negate_exit_shim(NegateVelocityX(slot));
        jsr_pop();
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_HoverThenSwoop0EC0(void)
{
    HoverThenSwoop0EC0(cpu.x);
}

/*
 * @routine 0EC0:4BF StepOffThenWake0EC0
 * @run 71:DCBF
 * WHAT IT DOES: Steps forward four times at frame 3 of its list, slowing; then, when the player's flags allow, wakes.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StepOffThenWake0EC0(uint8_t slot)
{
    lda(RD8(OBJ->work_f[slot]));
    cmp(cpu.a, 0x04);
    if (cpu.a < 0x04) {
        step_list(slot, 0xDD25, false, 0xDCD1);
        if ((cpu.p & P_C) != 0) {
            lda(RD8(OBJ->frame[slot]));
            cmp(cpu.a, 0x03);
            if (cpu.a == 0x03) {
                INC8(OBJ->work_f[slot]);
                lda(0x01);
                WR8(OBJ->vx_hi[slot], cpu.a);
                lda(0x70);
                WR8(OBJ->vx_lo[slot], cpu.a);
                goto step;
            }
        }
    }
    lda(RD8(OBJ->vx_lo[slot]));
    add_a(0xFC, false);
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    add_a(0xFF, (cpu.p & P_C) != 0);
    WR8(OBJ->vx_hi[slot], cpu.a);
    if ((cpu.p & P_N) != 0) {
        WR8(OBJ->vx_hi[slot], 0);
        WR8(OBJ->vx_lo[slot], 0);
        lda(RD8(OBJ->work_f[slot]));
        cmp(cpu.a, 0x04);
        if (cpu.a == 0x04) {
            lda(rd8(RichterColumnHit));
            lda((uint8_t)(cpu.a | rd8(RichterColumnHit + 1)));
            lda((uint8_t)(cpu.a | rd8(RichterColumnHit + 2)));
            lda((uint8_t)(cpu.a & 0x01));
            if (cpu.a != 0) {
                lda(RD8(OBJ->loop_flags[slot]));
                lda((uint8_t)(cpu.a & 0xFE));
                WR8(OBJ->loop_flags[slot], cpu.a);
                INC8(OBJ->state[slot]);
            }
        }
    }
step:
    move_x(slot, 0xDD24);
}

void abi_StepOffThenWake0EC0(void)
{
    StepOffThenWake0EC0(cpu.x);
}

/*
 * @routine 0EBF:308 SwoopAndClimb0EBF
 * @run 71:D308
 * WHAT IT DOES: Swoops toward the player: first falling (turning frame while moving the other way), then climbing to Y OBJ_WORK_J.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwoopAndClimb0EBF(uint8_t slot)
{
    move(slot, 0xD30B);
    jsr_push(0xD30E);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    WR8(ZP->temp00, cpu.a);
    word_index(cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    add_a(rd8((uint16_t)(0xD36F + cpu.y)), false);
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    add_a(rd8((uint16_t)(0xD370 + cpu.y)), (cpu.p & P_C) != 0);
    WR8(OBJ->vx_hi[slot], cpu.a);
    call_routine(0xD32A, 0xD373);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a((uint8_t)~0x10, true);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    if ((cpu.p & P_N) == 0) {
        ldy(0x04);
        call_routine(0xD342, 0xD19C);
        lda(RD8(OBJ->vx_hi[slot]));
        rol_a();
        rol_a();
        lda((uint8_t)(cpu.a & 0x01));
        lda((uint8_t)(cpu.a ^ RD8(ZP->temp00)));
        if (cpu.a == 0) {
            return;
        }
            lda(0x4F);
            WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    call_routine(0xD356, 0xD398);
    ldy(0x04);
    call_routine(0xD35B, 0xD19C);
    call_routine(0xD35E, 0xD3A8);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, RD8(OBJ->work_j[slot]));
    if (cpu.a >= RD8(OBJ->work_j[slot])) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SwoopAndClimb0EBF(void)
{
    SwoopAndClimb0EBF(cpu.x);
}

/*
 * @routine 117B:761 SwoopAndClimb117B
 * @run 70:B761
 * WHAT IT DOES: Swoops toward the player: first falling (turning frame while moving the other way), then climbing to Y OBJ_WORK_J.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwoopAndClimb117B(uint8_t slot)
{
    move(slot, 0xB764);
    jsr_push(0xB767);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    WR8(ZP->temp00, cpu.a);
    word_index(cpu.a);
    lda(RD8(OBJ->vx_lo[slot]));
    add_a(rd8((uint16_t)(0xB7C8 + cpu.y)), false);
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(RD8(OBJ->vx_hi[slot]));
    add_a(rd8((uint16_t)(0xB7C9 + cpu.y)), (cpu.p & P_C) != 0);
    WR8(OBJ->vx_hi[slot], cpu.a);
    call_routine(0xB783, 0xB7CC);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a((uint8_t)~0x10, true);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    if ((cpu.p & P_N) == 0) {
        ldy(0x04);
        call_routine(0xB79B, 0xB5F5);
        lda(RD8(OBJ->vx_hi[slot]));
        rol_a();
        rol_a();
        lda((uint8_t)(cpu.a & 0x01));
        lda((uint8_t)(cpu.a ^ RD8(ZP->temp00)));
        if (cpu.a == 0) {
            return;
        }
            lda(0x04);
            WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    call_routine(0xB7AF, 0xB7F1);
    ldy(0x04);
    call_routine(0xB7B4, 0xB5F5);
    call_routine(0xB7B7, 0xB801);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, RD8(OBJ->work_j[slot]));
    if (cpu.a >= RD8(OBJ->work_j[slot])) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SwoopAndClimb117B(void)
{
    SwoopAndClimb117B(cpu.x);
}

/*
 * @routine 0EBD:78F PlayerRidesBell0EBD
 * @run 71:C78F
 * WHAT IT DOES: Under it, pressing down: the player is held while its animation rings (sound $3B).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PlayerRidesBell0EBD(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC797);
        lda(0x06);
        WR8(OBJ->sprite_set[slot], cpu.a);
        return;
    }
    lda(RD8(OBJ->substate[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->state[0]));
        if (cpu.a != 0) {
            return;
        }
        lda(RD8(OBJ->x_lo[0]));
        add_a((uint8_t)~RD8(OBJ->x_lo[slot]), true);
        add_a(0x0C, false);
        cmp(cpu.a, 0x18);
        if (cpu.a >= 0x18) {
            return;
        }
        lda(rd8(zPadPressed));
        lda((uint8_t)(cpu.a & 0x10));
        if (cpu.a == 0) {
            return;
        }
        INC8(OBJ->substate[slot]);
        lda(RD8(OBJ->x_lo[0]));
        WR8(OBJ->x_lo[slot], cpu.a);
        lda(RD8(OBJ->y_lo[0]));
        add_a(0xD8, false);
        WR8(OBJ->y_lo[slot], cpu.a);
        return;
    }
    lda(0xC0);
    WR8(OBJ->work_e[0], cpu.a);
    lda(0x10);
    WR8(OBJ->work_f[0], cpu.a);
    step_list(slot, 0xC780, true, 0xC7E3);
    if (list_ended()) {
        WR8(OBJ->substate[slot], 0);
        WR8(OBJ->frame[slot], 0);
        WR8(OBJ->work_e[0], 0);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    sound(0x3B, 0xC7F6);
}

void abi_PlayerRidesBell0EBD(void)
{
    PlayerRidesBell0EBD(cpu.x);
}
