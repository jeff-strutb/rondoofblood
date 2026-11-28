/*
 * Object state handlers of the player's overlays, lot L2: the four step routines of a stage's flash object (a short
 * animation from two tables at $9CFD-$9D00 that ends with sound $71), shared across the seven overlay copies that hold it.
 *
 * Tables read by address here keep the old address form until the typed access owner lands rows for them.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "player_objects.h"

enum { PS_W_WAIT, PS_W_NEXT };
enum { PS_SF_TIMER, PS_SF_RESTART, PS_SF_SOUND };
enum { PS_NK_LOCKED, PS_NK_LOW, PS_NK_PLAIN, PS_NK_BUMPED };
enum { PS_SD_TIMER, PS_SD_RESTART, PS_SD_SOUND };
enum { PS_OT_X_HIGH, PS_OT_X_FAR, PS_OT_Y_HIGH, PS_OT_Y_FAR, PS_OT_TOUCH };
enum { FLASH_FRAMES = 0x9CFD, FLASH_LISTS = 0x9CFF, FLASH_CHANNEL = 5, FLASH_STEPS = 4, FLASH_SOUND = 0x71, FLASH_FRAME_TIME = 4 };

/* The step that starts the flash's next list: reload the timer, take the frame and the list from the tables by the kind. */
static ObjArm flash_restart(uint8_t slot, uint16_t ret)
{
    ObjArm r = { 0 };
    uint8_t kind, base, step, list;
    Sum frame;

    WR8(OBJ->life_timer[slot], FLASH_FRAME_TIME);
    kind = RD8(OBJ->start_arg[slot]);
    r.first = (uint8_t)(kind & 1);
    base = rd8((uint16_t)(FLASH_FRAMES + r.first));
    step = RD8(OBJ->work_e[slot]);
    frame = add_bytes(base, step, false);
    WR8(OBJ->frame[slot], frame.value);
    r.sum = add_bytes(frame.value, rd8((uint16_t)(FLASH_LISTS + r.first)), false);
    list = r.sum.value;
    push8(slot);
    jsr_push(ret);
    r.second = StartListChannelKind2(list, FLASH_CHANNEL);
    jsr_pop();
    r.third = pull8();
    return r;
}

static Ended flash_restart_exit_shim(ObjArm r)
{
    return ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(r.third), r.second), r.sum.carry), r.sum.overflow), r.first);
}

/*
 * @routine 0720:567 WaitForField85Two
 * @run 6B:9D67
 * WHAT IT DOES: Moves the object on to its next state when zField85 is 2.
 * HOW: CMP #2; equal increments the state.
 * WAITS: no.
 * Entry: X object. Exit: A zField85; N, Z, C from the compare, or N and Z from the new state. X, Y kept.
 */
ObjArm WaitForField85Two(uint8_t slot)
{
    ObjArm r = { 0 };

    r.first = RD8(ZP->field85);
    r.arm = PS_W_WAIT;
    if (r.first != 2) {
        return r;
    }
    r.second = INC8(OBJ->state[slot]);
    r.arm = PS_W_NEXT;
    return r;
}

static Ended wait_for_field85_two_exit_shim(ObjArm r)
{
    Ended e = ended_compare(r.first, 2);

    if (r.arm == PS_W_WAIT) {
        return e;
    }
    return ended_then(e, ended_nz(r.second));
}

void abi_WaitForField85Two(void)
{
    abi_ended(wait_for_field85_two_exit_shim(WaitForField85Two(cpu.x)));
}

/*
 * @routine 0720:571 StepFlashTimer
 * @run 6B:9D71
 * WHAT IT DOES: Counts the flash's frame timer down; at zero steps its list counter, and at four steps ends the flash
 * (next state, bit 7 of zField8C, sound $71); before that it starts the next list.
 * HOW: DEC of the timer, INC of OBJ_WORK_E read again for the CMP, flash_restart.
 * WAITS: no.
 * Entry: X object, Y kept. Exit: timer running: N and Z from it. Else as the flash's restart (A 2, N and Z the object's
 * slot, C and V from the second add) or the sound. The end of the flash (four steps) is untraced (4 of 400 cases reach it
 * and the draft stops there): written from the disc's bytes.
 */
ObjArm StepFlashTimer(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };
    uint8_t flags;

    r.first = DEC8(OBJ->life_timer[slot]);
    r.arm = PS_SF_TIMER;
    if (r.first != 0) {
        return r;
    }
    INC8(OBJ->work_e[slot]);
    r.second = RD8(OBJ->work_e[slot]);
    if (r.second != FLASH_STEPS) {
        r.sub = obj_sub(flash_restart(slot, 0x9DAD));
        r.arm = PS_SF_RESTART;
        return r;
    }
    INC8(OBJ->state[slot]);
    flags = RD8(ZP->field8c);
    WR8(ZP->field8c, (uint8_t)(flags | 0x80));
    r.sounded = true;
    r.command = FLASH_SOUND;
    r.call = SoundCommand(FLASH_SOUND, slot, y);
    r.arm = PS_SF_SOUND;
    return r;
}

static SoundEnd step_flash_timer_exit_shim(ObjArm r)
{
    SoundEnd s = { ended_kept(), ended_kept(), false, 0, { 0 } };

    if (r.arm == PS_SF_TIMER) {
        s.end = ended_nz(r.first);
        return s;
    }
    if (r.arm == PS_SF_RESTART) {
        s.end = flash_restart_exit_shim(obj_arm(r.sub));
        return s;
    }
    s.before = ended_load(FLASH_SOUND);
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    return s;
}

void abi_StepFlashTimer(void)
{
    abi_sound_end(step_flash_timer_exit_shim(StepFlashTimer(cpu.x, cpu.y)));
}

/*
 * @routine 0720:5AF NextStateByKindBits
 * @run 6B:9DAF
 * WHAT IT DOES: While zField8C bit 7 is clear, takes the object to the next state, sets its frame timer to 4, and
 * takes it one state further when the low nibble of its next kind is 2 to 5, 10 or 11.
 * HOW: INC of the state, one store, then compares of the nibble.
 * WAITS: no.
 * Entry: X object. Exit: A the last value loaded or compared, N, Z, C from the compare or N and Z from the last INC.
 * The arms for the nibbles 6 to 11 and the second INC are untraced (89 of 400 cases stop the draft): written from the
 * disc's bytes.
 */
ObjArm NextStateByKindBits(uint8_t slot)
{
    ObjArm r = { 0 };
    bool bump;

    r.first = RD8(ZP->field8c);
    r.arm = PS_NK_LOCKED;
    if (r.first & 0x80) {
        return r;
    }
    INC8(OBJ->state[slot]);
    WR8(OBJ->life_timer[slot], FLASH_FRAME_TIME);
    r.second = (uint8_t)(RD8(OBJ->start_arg[slot]) & 0x0F);
    r.arm = PS_NK_LOW;
    if (r.second < 2) {
        return r;
    }
    r.fourth = 6;
    bump = r.second < 6;
    if (!bump) {
        r.fourth = 0x0A;
        bump = r.second == 0x0A;
    }
    if (!bump) {
        r.fourth = 0x0B;
        bump = r.second == 0x0B;
    }
    r.arm = PS_NK_PLAIN;
    if (bump) {
        r.third = INC8(OBJ->state[slot]);
        r.arm = PS_NK_BUMPED;
    }
    return r;
}

/* Each compare replaces the one before it: the last one made, then the second INC's flags when it bumped. */
static Ended next_state_by_kind_bits_exit_shim(ObjArm r)
{
    Ended e;

    if (r.arm == PS_NK_LOCKED) {
        return ended_load(r.first);
    }
    if (r.arm == PS_NK_LOW) {
        return ended_compare(r.second, 2);
    }
    e = ended_compare(r.second, r.fourth);
    if (r.arm == PS_NK_BUMPED) {
        e = ended_then(e, ended_nz(r.third));
    }
    return e;
}

void abi_NextStateByKindBits(void)
{
    abi_ended(next_state_by_kind_bits_exit_shim(NextStateByKindBits(cpu.x)));
}

/*
 * @routine 0720:5D6 StepFlashTimerDown
 * @run 6B:9DD6
 * WHAT IT DOES: The flash's other step: counts its frame timer down; at zero counts the list counter down; below zero
 * the flash ends (next state, sound $71), else the next list starts.
 * HOW: DEC of the timer, DEC of OBJ_WORK_E, flash_restart.
 * WAITS: no.
 * Entry: X object, Y kept. Exit: as StepFlashTimer.
 */
ObjArm StepFlashTimerDown(uint8_t slot, uint8_t y)
{
    ObjArm r = { 0 };

    r.first = DEC8(OBJ->life_timer[slot]);
    r.arm = PS_SD_TIMER;
    if (r.first != 0) {
        return r;
    }
    r.second = DEC8(OBJ->work_e[slot]);
    if ((r.second & 0x80) == 0) {
        r.sub = obj_sub(flash_restart(slot, 0x9E07));
        r.arm = PS_SD_RESTART;
        return r;
    }
    INC8(OBJ->state[slot]);
    r.sounded = true;
    r.command = FLASH_SOUND;
    r.call = SoundCommand(FLASH_SOUND, slot, y);
    r.arm = PS_SD_SOUND;
    return r;
}

static SoundEnd step_flash_timer_down_exit_shim(ObjArm r)
{
    SoundEnd s = { ended_kept(), ended_kept(), false, 0, { 0 } };

    if (r.arm == PS_SD_TIMER) {
        s.end = ended_nz(r.first);
        return s;
    }
    if (r.arm == PS_SD_RESTART) {
        s.end = flash_restart_exit_shim(obj_arm(r.sub));
        return s;
    }
    s.before = ended_load(FLASH_SOUND);
    s.sounded = true;
    s.command = r.command;
    s.call = r.call;
    return s;
}

void abi_StepFlashTimerDown(void)
{
    abi_sound_end(step_flash_timer_down_exit_shim(StepFlashTimerDown(cpu.x, cpu.y)));
}

/*
 * @routine 01BD:7C9 ObjectTouchesPlayerBox
 * @run 6E:C7C9
 * WHAT IT DOES: Says whether the object lies within a box around the player: 8 either side of his X low byte and, from
 * a point his Y low plus the box offset OBJ_HITBOX_DY, $14 either side. The answer is 0 for in and 1 for out.
 * HOW: The first screen only (both high bytes zero); two add and compare windows of $10 and $28.
 * WAITS: no.
 * Entry: X object. Exit: A 0 or 1, N and Z from it, C and V from the last window's add and compare. Clobbers zTemp00.
 */
ObjArm ObjectTouchesPlayerBox(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t player_y = RD8(OBJ->y_lo[0]);
    uint8_t offset = RD8(OBJ->hitbox_dy[0]);
    uint8_t mine, theirs, taken;
    Sum diff;

    r.sum = add_bytes(player_y, offset, false);
    WR8(ZP->temp00, r.sum.value);
    r.first = RD8(OBJ->x_hi[slot]);
    r.arm = PS_OT_X_HIGH;
    if (r.first != 0) {
        return r;
    }
    mine = RD8(OBJ->x_lo[slot]);
    theirs = RD8(OBJ->x_lo[0]);
    diff = sub_bytes(mine, theirs, true);
    r.sum2 = add_bytes(diff.value, 8, false);
    r.arm = PS_OT_X_FAR;
    if (r.sum2.value >= 0x10) {
        return r;
    }
    r.second = RD8(OBJ->y_hi[slot]);
    r.arm = PS_OT_Y_HIGH;
    if (r.second != 0) {
        return r;
    }
    mine = RD8(OBJ->y_lo[slot]);
    taken = RD8(ZP->temp00);
    diff = sub_bytes(mine, taken, true);
    r.sum3 = add_bytes(diff.value, 0x14, false);
    r.arm = PS_OT_Y_FAR;
    if (r.sum3.value >= 0x28) {
        return r;
    }
    r.arm = PS_OT_TOUCH;
    return r;
}

static Ended object_touches_player_box_exit_shim(ObjArm r)
{
    Ended e = ended_then(ended_of_sum(r.sum), ended_load(r.first));

    if (r.arm == PS_OT_X_HIGH) {
        return ended_then(e, ended_load(1));
    }
    e = ended_then(e, ended_then(ended_of_sum(r.sum2), ended_compare(r.sum2.value, 0x10)));
    if (r.arm == PS_OT_X_FAR) {
        return ended_then(e, ended_load(1));
    }
    e = ended_then(e, ended_load(r.second));
    if (r.arm == PS_OT_Y_HIGH) {
        return ended_then(e, ended_load(1));
    }
    e = ended_then(e, ended_then(ended_of_sum(r.sum3), ended_compare(r.sum3.value, 0x28)));
    if (r.arm == PS_OT_Y_FAR) {
        return ended_then(e, ended_load(1));
    }
    return ended_then(e, ended_load(0));
}

void abi_ObjectTouchesPlayerBox(void)
{
    abi_ended(object_touches_player_box_exit_shim(ObjectTouchesPlayerBox(cpu.x)));
}

/*
 * @routine 01BE:4E6 NudgeAngleByVelocityAndScroll
 * @run 6E:CCE6
 * WHAT IT DOES: Moves the object's angle pair OBJ_WORK_F and OBJ_WORK_G (and the counter OBJ_WORK_H): by two tables when it
 * moves sideways (by its X speed's sign), then by the scroll speed ScrollSpeedX negated, the table at $CE79 by its sign.
 * HOW: Chains of 8-bit adds with the carry carried from one to the next.
 * WAITS: no.
 * Entry: X object. Exit: A the angle low byte stored, N and Z from it, C and V from its add. X kept, Y the sign index.
 * Tables at $CD2D, $CD2F and $CE79 have no rows yet (the old address form).
 */
ObjArm NudgeAngleByVelocityAndScroll(uint8_t slot)
{
    ObjArm r = { 0 };
    uint8_t vx_hi = RD8(OBJ->vx_hi[slot]);
    uint8_t vx_lo = RD8(OBJ->vx_lo[slot]);
    uint8_t index, scroll, negated, value;
    Sum sum;

    if ((uint8_t)(vx_hi | vx_lo) != 0) {
        vx_hi = RD8(OBJ->vx_hi[slot]);
        index = (uint8_t)((vx_hi >> 7) & 1);
        value = rd8((uint16_t)(0xCD2D + index));
        sum = add_bytes(value, RD8(OBJ->work_h[slot]), false);
        WR8(OBJ->work_h[slot], sum.value);
        value = rd8((uint16_t)(0xCD2F + index));
        sum = add_bytes(value, RD8(OBJ->angle_hi[slot]), sum.carry);
        WR8(OBJ->angle_hi[slot], sum.value);
        value = rd8((uint16_t)(0xCD2F + index));
        sum = add_bytes(value, RD8(OBJ->angle_lo[slot]), sum.carry);
        WR8(OBJ->angle_lo[slot], sum.value);
    }
    index = 0;
    scroll = rd8(ScrollSpeedX);
    negated = (uint8_t)(~scroll + 1);
    if (negated & 0x80) {
        index = 1;
    }
    sum = add_bytes(negated, RD8(OBJ->angle_hi[slot]), false);
    WR8(OBJ->angle_hi[slot], sum.value);
    value = rd8((uint16_t)(0xCE79 + index));
    r.sum = add_bytes(value, RD8(OBJ->angle_lo[slot]), sum.carry);
    WR8(OBJ->angle_lo[slot], r.sum.value);
    r.first = index;
    return r;
}

static Ended nudge_angle_by_velocity_and_scroll_exit_shim(ObjArm r)
{
    return ended_with_y(ended_of_sum(r.sum), r.first);
}

void abi_NudgeAngleByVelocityAndScroll(void)
{
    abi_ended(nudge_angle_by_velocity_and_scroll_exit_shim(NudgeAngleByVelocityAndScroll(cpu.x)));
}
