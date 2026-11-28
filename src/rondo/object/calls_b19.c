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
#include "calls_b19.h"

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

static void move(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocity(slot));
    jsr_pop();
}
/* LDA #list, LDX #slot, JSR StartListChannelKind2. */
static void list_channel2(uint8_t number, uint8_t at_slot, uint16_t ret)
{
    lda(number);
    cpu.x = at_slot;
    abi_nz(at_slot);
    jsr_push(ret);
    list_channel_exit_shim(StartListChannelKind2(cpu.a, cpu.x));
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

/* A tail JMP to LoadObjectRecord's vector. */
static void load_record_tail_(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
}

/*
 * @routine 12DB:25E CountThenFire12DB
 * @run 70:B25E
 * WHAT IT DOES: Runs the list and $B297; after OBJ_WORK_D counts: script $86, a kind $27 object in slot $40, sound $72, rising, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenFire12DB(uint8_t slot)
{
    step_list(slot, 0xB168, true, 0xB269);
    call_routine(0xB26C, 0xB297);
    abi_nz(DEC8(OBJ->work_d[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    push8(cpu.x);
    script_at(0x86, 0x05, 0xB279);
    pull_x();
    ldy(0x40);
    clear_slot_in_y(0xB27F);
    start_at_place(0x27, 0xB284);
    sound(0x72, 0xB289);
    lda(0xFF);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(0x80);
    WR8(OBJ->vy_lo[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_CountThenFire12DB(void)
{
    CountThenFire12DB(cpu.x);
}

/*
 * @routine 12DE:348 MoveBounceByTable12DE
 * @run 71:CB48
 * WHAT IT DOES: Moves across at the speed OBJ_WORK_E picks, turning round at a wall.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void MoveBounceByTable12DE(uint8_t slot)
{
    lda(RD8(OBJ->work_e[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xCB82 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xCB81 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    move_x(slot, 0xCB5C);
    lda(0x0A);
    WR8(ZP->product.mid, cpu.a);
    lda(0x02);
    jsr_push(0xCB65);
    probe_exit_shim(ProbeTerrain(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0));
    jsr_pop();
    lda(RD8(OBJ->vx_hi[slot]));
    rol_a();
    rol_a();
    lda((uint8_t)(cpu.a & 0x01));
    lda((uint8_t)(cpu.a ^ 0x01));
    word_index(cpu.a);
    lda(rd8((uint16_t)(TileHit + cpu.y)));
    lda((uint8_t)(cpu.a | rd8((uint16_t)(TileHit + 1 + cpu.y))));
    lda((uint8_t)(cpu.a & 0x02));
    if (cpu.a == 0) {
        return;
    }
    jsr_push(0xCB7E);
    negate_exit_shim(NegateVelocityX(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocityX(slot));
}

void abi_MoveBounceByTable12DE(void)
{
    MoveBounceByTable12DE(cpu.x);
}

/*
 * @routine 13FB:74E StrikeSoundsByStep13FB
 * @run 70:B74E
 * WHAT IT DOES: Runs the list at $B787, the sound for OBJ_WORK_I at step 10; at its end state 2.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StrikeSoundsByStep13FB(uint8_t slot)
{
    step_list(slot, 0xB787, true, 0xB759);
    if (list_ended()) {
        lda(0x02);
        WR8(OBJ->state[slot], cpu.a);
        WR8(OBJ->substate[slot], 0);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x0A);
    if (cpu.a == 0x0A) {
        ldy(RD8(OBJ->work_i[slot]));
        cpu.y = (uint8_t)(cpu.y - 1);
        abi_nz(cpu.y);
        sound(rd8((uint16_t)(0xB7AC + cpu.y)), 0xB76E);
    }
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x0B);
    if (cpu.a != 0x0B) {
        return;
    }
    WR8(OBJ->work_i[slot], 0);
    lda(0x01);
    WR8(OBJ->work_l[slot], cpu.a);
}

void abi_StrikeSoundsByStep13FB(void)
{
    StrikeSoundsByStep13FB(cpu.x);
}

/*
 * @routine 161C:283 TossThenBecome9B161C
 * @run 70:BA83
 * WHAT IT DOES: Loads moving away from or toward the player; falls animated for OBJ_WORK_E counts and then becomes kind $9B.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void TossThenBecome9B161C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xBA8B);
        jsr_push(0xBA8E);
        side_of_player_exit_shim(SideOfPlayer(slot));
        jsr_pop();
        lda((uint8_t)(cpu.a & 0x01));
        if (cpu.a != 0) {
            jsr_push(0xBA95);
            negate_exit_shim(NegateVelocityX(slot));
            jsr_pop();
        }
        return;
    }
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) != 0) {
        lda(0x9B);
        WR8(OBJ->kind[slot], cpu.a);
        WR8(OBJ->state[slot], 0);
        return;
    }
    lda(RD8(OBJ->work_d[slot]));
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xBABC + cpu.y)));
    WR8(ZP->temp10, cpu.a);
    lda(rd8((uint16_t)(0xBABD + cpu.y)));
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step;

        jsr_push(0xBAB6);
        step = StepObjectFrames(slot, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    jsr_push(0xBAB9);
    gravity_exit_shim(ApplyGravityUnlimited(slot));
    jsr_pop();
    axis_move_exit_shim(ApplyVelocity(slot));
}

void abi_TossThenBecome9B161C(void)
{
    TossThenBecome9B161C(cpu.x);
}

/*
 * @routine 161D:3F4 FallDroppingKind9C161D
 * @run 71:C3F4
 * WHAT IT DOES: Falls, cleared below the first screen; every eighth count of OBJ_WORK_E leaves a kind $9C object.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallDroppingKind9C161D(uint8_t slot)
{
    step_list(slot, 0xC42D, true, 0xC3FF);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x14, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xC413);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        ClearObject(slot);
        return;
    }
    lda(RD8(OBJ->work_e[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    if (cpu.a != 0) {
        return;
    }
    if (!find_from30(0xC425)) {
        return;
    }
    start_at_place(0x9C, 0xC42C);
}

void abi_FallDroppingKind9C161D(void)
{
    FallDroppingKind9C161D(cpu.x);
}

/*
 * @routine 18DB:53C SwingThenScript18DB
 * @run 70:B53C
 * WHAT IT DOES: Bobs by the cosine of OBJ_WORK_G; after OBJ_WORK_E counts script $83, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwingThenScript18DB(uint8_t slot)
{
    lda(0xE0);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    lda(RD8(OBJ->work_g[slot]));
    jsr_push(0xB548);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct1));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(rd8(zProduct2));
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xB555);
    lda(RD8(OBJ->work_g[slot]));
    add_a(0x04, false);
    WR8(OBJ->work_g[slot], cpu.a);
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    push8(cpu.x);
    script_at(0x83, 0x04, 0xB56B);
    pull_x();
    lda(0x80);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SwingThenScript18DB(void)
{
    SwingThenScript18DB(cpu.x);
}

/*
 * @routine 18DB:575 SwingThenChannel18DB
 * @run 70:B575
 * WHAT IT DOES: Bobs by the cosine of OBJ_WORK_G; after OBJ_WORK_E counts list channel $82, state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SwingThenChannel18DB(uint8_t slot)
{
    lda(0xE0);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    lda(RD8(OBJ->work_g[slot]));
    jsr_push(0xB581);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct1));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(rd8(zProduct2));
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xB58E);
    lda(RD8(OBJ->work_g[slot]));
    add_a(0x04, false);
    WR8(OBJ->work_g[slot], cpu.a);
    abi_nz(DEC8(OBJ->work_e[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    push8(cpu.x);
    list_channel2(0x82, 0x05, 0xB5A4);
    pull_x();
    WR8(OBJ->frame[slot], 0);
    INC8(OBJ->state[slot]);
    WR8(OBJ->substate[slot], 0);
}

void abi_SwingThenChannel18DB(void)
{
    SwingThenChannel18DB(cpu.x);
}

/*
 * @routine 18DC:100 WalkInsideScreen18DC
 * @run 70:B900
 * WHAT IT DOES: Between X $40 and $C0 on the first screen walks; outside it the substate counts on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WalkInsideScreen18DC(uint8_t slot)
{
    lda(RD8(OBJ->x_hi[slot]));
    if (cpu.a == 0) {
        lda(RD8(OBJ->x_lo[slot]));
        cmp(cpu.a, 0xC0);
        if (cpu.a < 0xC0) {
            cmp(cpu.a, 0x40);
            if (cpu.a >= 0x40) {
                step_list(slot, 0xB6E9, true, 0xB91B);
                lda(RD8(OBJ->facing_left[slot]));
                word_index(cpu.a);
                lda(rd8((uint16_t)(0xB6FA + cpu.y)));
                WR8(OBJ->vx_lo[slot], cpu.a);
                lda(rd8((uint16_t)(0xB6FB + cpu.y)));
                WR8(OBJ->vx_hi[slot], cpu.a);
                axis_move_exit_shim(ApplyVelocityX(slot));
                return;
            }
        }
    }
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_WalkInsideScreen18DC(void)
{
    WalkInsideScreen18DC(cpu.x);
}

/*
 * @routine 0B4C:19B ListSpit0B4C
 * @run 70:B99B
 * WHAT IT DOES: Runs the list at $B9D5, spitting kind $25 at frame $15; at its end clears it and goes to state 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListSpit0B4C(uint8_t slot)
{
    step_list(slot, 0xB9D5, false, 0xB9A6);
    if (list_ended()) {
        ldy(0x51);
        clear_slot_in_y(0xB9C9);
        restart_animation(slot);
        lda(0x05);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(RD8(OBJ->frame[slot]));
    cmp(cpu.a, 0x15);
    if (cpu.a != 0x15) {
        return;
    }
    sound(0x20, 0xB9B6);
    ldy(0x51);
    start_at_place(0x25, 0xB9BD);
    lda(RD8(OBJ->facing_left[slot]));
    WR8(OBJ->facing_left[cpu.y], cpu.a);
}

void abi_ListSpit0B4C(void)
{
    ListSpit0B4C(cpu.x);
}

/*
 * @routine 0EBE:460 ThrowThenWait0EBE
 * @run 71:CC60
 * WHAT IT DOES: Animates, throwing kind $39 at frame $2C; at the list's end (or off the first screen) state 3 with the next wait.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ThrowThenWait0EBE(uint8_t slot)
{
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        ldy(0x02);
        call_routine(0xCC6A, 0xCB86);
        if (!list_ended()) {
            if ((cpu.p & P_C) == 0) {
                return;
            }
            lda(RD8(OBJ->frame[slot]));
            cmp(cpu.a, 0x2C);
            if (cpu.a != 0x2C) {
                return;
            }
            if (!find_from30(0xCC78)) {
                return;
            }
            lda(0x39);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
            return;
        }
    }
    restart_animation(slot);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    INC8(OBJ->work_f[slot]);
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xCC9A + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_ThrowThenWait0EBE(void)
{
    ThrowThenWait0EBE(cpu.x);
}

/*
 * @routine 161C:0F6 ThrowThenWait161C
 * @run 70:B8F6
 * WHAT IT DOES: Animates, throwing kind $36 at frame $21; at the list's end (or off the first screen) state 3 with the next wait.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ThrowThenWait161C(uint8_t slot)
{
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        ldy(0x02);
        call_routine(0xB900, 0xB81C);
        if (!list_ended()) {
            if ((cpu.p & P_C) == 0) {
                return;
            }
            lda(RD8(OBJ->frame[slot]));
            cmp(cpu.a, 0x21);
            if (cpu.a != 0x21) {
                return;
            }
            if (!find_from30(0xB90E)) {
                return;
            }
            lda(0x36);
            copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
            return;
        }
    }
    restart_animation(slot);
    lda(0x03);
    WR8(OBJ->state[slot], cpu.a);
    INC8(OBJ->work_f[slot]);
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xB930 + cpu.y)));
    WR8(OBJ->work_d[slot], cpu.a);
}

void abi_ThrowThenWait161C(void)
{
    ThrowThenWait161C(cpu.x);
}

/*
 * @routine 12DC:0E1 LoadSpreadBySlot12DC
 * @run 70:B8E1
 * WHAT IT DOES: Loads by the slot from $20: a count, a first displacement, then a speed up or down.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadSpreadBySlot12DC(uint8_t slot)
{
    load_record(slot, 0xB8E4);
    lda(cpu.x);
    add_a((uint8_t)~0x20, true);
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xB963 + cpu.y)));
    WR8(OBJ->work_e[slot], cpu.a);
    lda(cpu.y);
    word_index(cpu.a);
    lda(rd8((uint16_t)(0xB91B + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB91C + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    move(slot, 0xB901);
    lda(cpu.x);
    add_a((uint8_t)~0x20, true);
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    word_index(cpu.a);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    lda(rd8((uint16_t)(0xB936 + cpu.y)));
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB935 + cpu.y)));
    WR8(OBJ->vy_lo[slot], cpu.a);
}

void abi_LoadSpreadBySlot12DC(void)
{
    LoadSpreadBySlot12DC(cpu.x);
}

/*
 * @routine 12DE:1E5 PopThenRise12DE
 * @run 71:C9E5
 * WHAT IT DOES: Loads at a random height below; then rises animated until its list ends.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void PopThenRise12DE(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xC9ED);
        random_a(0xC9F0);
        lda((uint8_t)(cpu.a & 0x0F));
        add_a((uint8_t)~0x1C, true);
        WR8(OBJ->vy_hi[slot], cpu.a);
        move_y(slot, 0xC9FB);
        lda(0xFE);
        WR8(OBJ->vy_hi[slot], cpu.a);
        lda(0x80);
        WR8(OBJ->vy_lo[slot], cpu.a);
        return;
    }
    lda(0x8C);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    move_y(slot, 0xCA0E);
    step_list(slot, 0xCA1F, true, 0xCA19);
    if (!list_ended()) {
        return;
    }
    ClearObject(slot);
}

void abi_PopThenRise12DE(void)
{
    PopThenRise12DE(cpu.x);
}

/*
 * @routine 16CB:7C3 FallThenGone16CB
 * @run 70:B7C3
 * WHAT IT DOES: Falls animated (sound $8E at step 3); below the first screen the raster effect off and cleared.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallThenGone16CB(uint8_t slot)
{
    step_list(slot, 0xB7FD, true, 0xB7CE);
    if ((cpu.p & P_C) != 0) {
        lda(RD8(OBJ->anim_step[slot]));
        cmp(cpu.a, 0x03);
        if (cpu.a == 0x03) {
            sound(0x8E, 0xB7DC);
        }
    }
    move(slot, 0xB7DF);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a(0x09, false);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a(0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        return;
    }
    wr8(zRasterArg, 0);
    wr8(zRasterEffect, 0);
    ClearObject(slot);
}

void abi_FallThenGone16CB(void)
{
    FallThenGone16CB(cpu.x);
}

/*
 * @routine 18DD:173 MarkInPool18DD
 * @run 71:C173
 * WHAT IT DOES: Marks (bit 7) one unmarked entry of the 112 at $3F90; with none left script $85, a wait and the state on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void MarkInPool18DD(uint8_t slot)
{
    lda(0x10);
    WR8(ZP->temp00, cpu.a);
    do {
        do {
            random_a(0xC17A);
            lda((uint8_t)(cpu.a & 0x7F));
            cmp(cpu.a, 0x70);
        } while (cpu.a >= 0x70);
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0x3F90 + cpu.y)));
        if ((cpu.p & P_N) == 0) {
            goto mark;
        }
        abi_nz(DEC8(ZP->temp00));
    } while ((cpu.p & P_Z) == 0);
    cpu.y = 0;
    do {
        lda(rd8((uint16_t)(0x3F90 + cpu.y)));
        if ((cpu.p & P_N) == 0) {
            goto mark;
        }
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x70);
    } while (cpu.y != 0x70);
    push8(cpu.x);
    cpu.x = 0x0B;
    abi_nz(cpu.x);
    lda(0x85);
    jsr_push(0xC19D);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
    pull_x();
    lda(0x40);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->state[slot]));
    return;
mark:
    lda((uint8_t)(cpu.a | 0x80));
    wr8((uint16_t)(0x3F90 + cpu.y), cpu.a);
}

void abi_MarkInPool18DD(void)
{
    MarkInPool18DD(cpu.x);
}

/*
 * @routine 0B4A:679 BlinkThenWakeOther0B4A
 * @run 70:AE79
 * WHAT IT DOES: State 2 blinks for 4 counts; state 3 sets the slot in the start argument to state 2 and goes.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BlinkThenWakeOther0B4A(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record(slot, 0xAE81);
        WR8(OBJ->frame[slot], 0);
        lda(0x04);
        WR8(OBJ->work_d[slot], cpu.a);
        return;
    }
    lda((uint8_t)(cpu.a - 1));
    if (cpu.a == 0) {
        return;
    }
    lda((uint8_t)(cpu.a - 1));
    if (cpu.a == 0) {
        lda(RD8(OBJ->work_d[slot]));
        lda((uint8_t)(cpu.a & 0x01));
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        lda(rd8((uint16_t)(0xAEB4 + cpu.y)));
        WR8(OBJ->frame[slot], cpu.a);
        abi_nz(DEC8(OBJ->work_d[slot]));
        if ((cpu.p & P_Z) == 0) {
            return;
        }
        abi_nz(INC8(OBJ->state[slot]));
        return;
    }
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a == 0) {
        return;
    }
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(0x02);
    WR8(OBJ->state[cpu.y], cpu.a);
    ClearObject(slot);
}

void abi_BlinkThenWakeOther0B4A(void)
{
    BlinkThenWakeOther0B4A(cpu.x);
}

/*
 * @routine 0CAC:0CE CircleCentre0CAC
 * @run 70:B8CE
 * WHAT IT DOES: Circles the screen's centre at radius $50 by OBJ_WORK_E; at angle $C0 the substate counts on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CircleCentre0CAC(uint8_t slot)
{
    step_list(slot, 0xB8AB, true, 0xB8D9);
    WR8(OBJ->x_hi[slot], 0);
    lda(0x80);
    WR8(OBJ->x_lo[slot], cpu.a);
    WR8(OBJ->y_hi[slot], 0);
    lda(0x70);
    WR8(OBJ->y_lo[slot], cpu.a);
    WR8(ZP->temp00, 0);
    lda(0x50);
    WR8(ZP->temp01, cpu.a);
    lda(RD8(OBJ->work_e[slot]));
    velocity_from_a(slot, 0xB8F5);
    move(slot, 0xB8F8);
    INC8(OBJ->work_e[slot]);
    call_routine(0xB8FE, 0xB909);
    lda(RD8(OBJ->work_e[slot]));
    cmp(cpu.a, 0xC0);
    if (cpu.a != 0xC0) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CircleCentre0CAC(void)
{
    CircleCentre0CAC(cpu.x);
}

/*
 * @routine 0EBC:2BE SprinkleKind9A0EBC
 * @run 70:BABE
 * WHAT IT DOES: Sound and load; then each time a kind $9A object scattered 0-15 to the right.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SprinkleKind9A0EBC(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        sound(0x59, 0xBAC8);
        load_record_tail_(slot);
        return;
    }
    if (!find_from20(0xBACE)) {
        return;
    }
    start_at_place(0x9A, 0xBAD5);
    lda(RD8(OBJ->work_d[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    WR8(OBJ->work_e[cpu.y], cpu.a);
    lda(0x10);
    WR8(OBJ->sprite_attr[cpu.y], cpu.a);
    INC8(OBJ->work_d[slot]);
    random_a(0xBAE8);
    lda((uint8_t)(cpu.a & 0x0F));
    add_a(RD8(OBJ->x_lo[cpu.y]), false);
    WR8(OBJ->x_lo[cpu.y], cpu.a);
    cpu.a = 0;
    add_a(RD8(OBJ->x_hi[cpu.y]), (cpu.p & P_C) != 0);
    WR8(OBJ->x_hi[cpu.y], cpu.a);
}

void abi_SprinkleKind9A0EBC(void)
{
    SprinkleKind9A0EBC(cpu.x);
}

/*
 * @routine 0EC0:2B0 LoadAtRandomSide0EC0
 * @run 71:DAB0
 * WHAT IT DOES: Loads at one side of the screen, at one of four heights, with that side's speed.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadAtRandomSide0EC0(uint8_t slot)
{
    load_record(slot, 0xDAB3);
    random_a(0xDAB6);
    WR8(OBJ->work_d[slot], cpu.a);
    lda((uint8_t)(cpu.a & 0x01));
    word_index((uint8_t)(cpu.a << 1));
    lda(rd8((uint16_t)(0xDAEC + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xDAEB + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
    lda(rd8((uint16_t)(0xDAEE + cpu.y)));
    WR8(OBJ->x_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xDAED + cpu.y)));
    WR8(OBJ->x_lo[slot], cpu.a);
    random_a(0xDAD9);
    lda((uint8_t)(cpu.a & 0x03));
    cpu.y = cpu.a;
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xDAF3 + cpu.y)));
    WR8(OBJ->y_lo[slot], cpu.a);
    face_by_side(slot, COL(OBJ->facing_left), 0xDAE5);
}

void abi_LoadAtRandomSide0EC0(void)
{
    LoadAtRandomSide0EC0(cpu.x);
}

/*
 * @routine 10CC:02D BobWhileList10CC
 * @run 70:B82D
 * WHAT IT DOES: Bobs while its list runs; then starts upward, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void BobWhileList10CC(uint8_t slot)
{
    lda(0x30);
    WR8(ZP->temp00, cpu.a);
    WR8(ZP->temp01, 0);
    lda(RD8(OBJ->work_g[slot]));
    jsr_push(0xB839);
    scale_exit_shim(ScaleByCosine(cpu.a));
    jsr_pop();
    lda(rd8(zProduct1));
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(rd8(zProduct2));
    WR8(OBJ->vy_hi[slot], cpu.a);
    move_y(slot, 0xB846);
    lda(RD8(OBJ->work_g[slot]));
    add_a(0x02, false);
    WR8(OBJ->work_g[slot], cpu.a);
    step_list(slot, 0xB868, true, 0xB85A);
    if (!list_ended()) {
        return;
    }
    lda(0xFC);
    WR8(OBJ->vy_hi[slot], cpu.a);
    WR8(OBJ->vy_lo[slot], 0);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_BobWhileList10CC(void)
{
    BobWhileList10CC(cpu.x);
}

/*
 * @routine 117C:38C LandAnimateByRise117C
 * @run 70:BB8C
 * WHAT IT DOES: Fades its attribute by OBJ_WORK_H, stops on landing, and shows frame $1B falling or the list rising.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandAnimateByRise117C(uint8_t slot)
{
    lda(RD8(OBJ->work_h[slot]));
    if (cpu.a != 0) {
        for (int i = 0; i < 4; i++) {
            abi_flag(P_C, (cpu.a & 0x01) != 0);
            lda((uint8_t)(cpu.a >> 1));
        }
        cpu.y = cpu.a;
        abi_nz(cpu.y);
        add_a(0x8C, false);
        WR8(OBJ->sprite_attr[slot], cpu.a);
        abi_nz(DEC8(OBJ->work_h[slot]));
    }
    call_routine(0xBBA2, 0xBBD8);
    lda(0x08);
    WR8(ZP->product.mid, cpu.a);
    jsr_push(0xBBA9);
    mover_exit_shim(StopOnLanding(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) != 0) {
        WR8(OBJ->substate[slot], 0);
        INC8(OBJ->state[slot]);
    }
    lda(RD8(OBJ->vy_hi[slot]));
    if ((cpu.p & P_N) == 0) {
        lda(0x1B);
        WR8(OBJ->frame[slot], cpu.a);
        return;
    }
    lda(0xC7);
    WR8(ZP->temp10, cpu.a);
    lda(0xBB);
    WR8(ZP->temp11, cpu.a);
    {
        uint8_t y = cpu.y;
        FrameStep step = StepObjectFrames(slot, &y);

        step_frames_exit_shim(step, y);
    }
}

void abi_LandAnimateByRise117C(void)
{
    LandAnimateByRise117C(cpu.x);
}

/*
 * @routine 12DB:70B RiseToTop12DB
 * @run 70:B70B
 * WHAT IT DOES: Rises faster each count; at Y $40 stops there and the substate counts on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RiseToTop12DB(uint8_t slot)
{
    step_list(slot, 0xB682, true, 0xB716);
    move(slot, 0xB719);
    call_routine(0xB71C, 0xB687);
    call_routine(0xB71F, 0xB6A1);
    lda(RD8(OBJ->vy_lo[slot]));
    add_a((uint8_t)~0x18, true);
    WR8(OBJ->vy_lo[slot], cpu.a);
    lda(RD8(OBJ->vy_hi[slot]));
    add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
    WR8(OBJ->vy_hi[slot], cpu.a);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x40);
    if (cpu.a >= 0x40) {
        return;
    }
    lda(0x40);
    WR8(OBJ->y_lo[slot], cpu.a);
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_RiseToTop12DB(void)
{
    RiseToTop12DB(cpu.x);
}

/*
 * @routine 12DE:4D6 ListFallThenCrash12DE
 * @run 71:CCD6
 * WHAT IT DOES: Runs the list and $CAF4; carry from it: two sounds, substate on.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListFallThenCrash12DE(uint8_t slot)
{
    step_list(slot, 0xCD11, true, 0xCCE1);
    call_routine(0xCCE4, 0xCAF4);
    if ((cpu.p & P_C) == 0) {
        lda(RD8(OBJ->vy_lo[slot]));
        add_a((uint8_t)~0x18, true);
        WR8(OBJ->vy_lo[slot], cpu.a);
        lda(RD8(OBJ->vy_hi[slot]));
        add_a((uint8_t)~0x00, (cpu.p & P_C) != 0);
        WR8(OBJ->vy_hi[slot], cpu.a);
        return;
    }
    sound(0x4A, 0xCCFD);
    sound(0x7D, 0xCD02);
    restart_animation(slot);
    lda(0x01);
    WR8(OBJ->work_k[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListFallThenCrash12DE(void)
{
    ListFallThenCrash12DE(cpu.x);
}

/*
 * @routine 0EBF:15E DiveWhenNear0EBF
 * @run 71:D15E
 * WHAT IT DOES: On screen and within $60 across: dives (state 4, substate 2) with a random time.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void DiveWhenNear0EBF(uint8_t slot)
{
    cpu.y = 0;
    call_routine(0xD162, 0xD19C);
    lda(RD8(OBJ->x_hi[slot]));
    lda((uint8_t)(cpu.a | RD8(OBJ->y_hi[slot])));
    if (cpu.a != 0) {
        return;
    }
    near_x(slot, 0xD16D);
    cmp(cpu.a, 0x60);
    if (cpu.a >= 0x60) {
        return;
    }
    restart_animation(slot);
    lda(RD8(OBJ->scroll_band[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    WR8(OBJ->scroll_band[slot], cpu.a);
    lda(0x03);
    WR8(OBJ->vy_hi[slot], cpu.a);
    random_a(0xD187);
    lda((uint8_t)(cpu.a & 0x7F));
    add_a(0x44, false);
    WR8(OBJ->work_j[slot], cpu.a);
    lda(0x04);
    WR8(OBJ->state[slot], cpu.a);
    lda(0x02);
    WR8(OBJ->substate[slot], cpu.a);
}

void abi_DiveWhenNear0EBF(void)
{
    DiveWhenNear0EBF(cpu.x);
}

/*
 * @routine 0EBF:3B5 FlyTillLevel0EBF
 * @run 71:D3B5
 * WHAT IT DOES: Flies until its Y speed levels off (0 or $FFFF) or it is above Y $10; then state 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FlyTillLevel0EBF(uint8_t slot)
{
    ldy(0x04);
    call_routine(0xD3BA, 0xD19C);
    call_routine(0xD3BD, 0xD3A8);
    move(slot, 0xD3C0);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x10);
    if (cpu.a >= 0x10) {
        lda(RD8(OBJ->work_h[slot]));
        lda((uint8_t)(cpu.a & 0x01));
        if (cpu.a != 0) {
            return;
        }
        call_routine(0xD3D6, 0xD2C9);
        lda(RD8(OBJ->vy_hi[slot]));
        lda((uint8_t)(cpu.a | RD8(OBJ->vy_lo[slot])));
        if (cpu.a != 0) {
            lda(RD8(OBJ->vy_hi[slot]));
            lda((uint8_t)(cpu.a & RD8(OBJ->vy_lo[slot])));
            cmp(cpu.a, 0xFF);
            if (cpu.a != 0xFF) {
                return;
            }
        }
    }
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_FlyTillLevel0EBF(void)
{
    FlyTillLevel0EBF(cpu.x);
}

/*
 * @routine 117C:00E FlyTillLevel117C
 * @run 70:B80E
 * WHAT IT DOES: Flies until its Y speed levels off (0 or $FFFF) or it is above Y $10; then state 5.
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FlyTillLevel117C(uint8_t slot)
{
    ldy(0x04);
    call_routine(0xB813, 0xB5F5);
    call_routine(0xB816, 0xB801);
    move(slot, 0xB819);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a != 0) {
        return;
    }
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x10);
    if (cpu.a >= 0x10) {
        lda(RD8(OBJ->work_h[slot]));
        lda((uint8_t)(cpu.a & 0x01));
        if (cpu.a != 0) {
            return;
        }
        call_routine(0xB82F, 0xB722);
        lda(RD8(OBJ->vy_hi[slot]));
        lda((uint8_t)(cpu.a | RD8(OBJ->vy_lo[slot])));
        if (cpu.a != 0) {
            lda(RD8(OBJ->vy_hi[slot]));
            lda((uint8_t)(cpu.a & RD8(OBJ->vy_lo[slot])));
            cmp(cpu.a, 0xFF);
            if (cpu.a != 0xFF) {
                return;
            }
        }
    }
    lda(0x05);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_FlyTillLevel117C(void)
{
    FlyTillLevel117C(cpu.x);
}
