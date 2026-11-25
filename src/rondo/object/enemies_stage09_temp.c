/*
 * TEMPORARY: the stage 09A1 enemies and their helpers (disc sectors 093C and 09E9 to 09ED), by routine until
 * each enemy kind is identified. Bodies return how the routine's last instructions left A, Y and the flags
 * (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

static Ended of_sum(Sum sum)
{
    return ended_of_sum(sum);
}

/* The steps of an animation list at `list`, with the list's address in zTemp10-11; the registers it left in `e`. */
static FrameStep step_list(uint8_t slot, uint16_t list, uint16_t ret, uint8_t *y, bool high_first)
{
    FrameStep step;

    if (high_first) {
        WR8(ZP->temp11, (uint8_t)(list >> 8));
        WR8(ZP->temp10, (uint8_t)list);
    } else {
        WR8(ZP->temp10, (uint8_t)list);
        WR8(ZP->temp11, (uint8_t)(list >> 8));
    }
    jsr_push(ret);
    step = StepObjectFrames(slot, y);
    jsr_pop();
    return step;
}


/* CPY and CPX: N, Z and C from the register less the operand; A kept. */
static Ended cpy(uint8_t reg, uint8_t operand)
{
    Ended e = ended_nz((uint8_t)(reg - operand));

    return ended_with_carry(e, reg >= operand);
}

/* ApplyGravity and ApplyGravityUnlimited's exit: A, N, Z, C and V as gravity_exit_shim leaves them. */
static Ended ended_of_gravity(GravityResult g)
{
    Ended e;

    if (g.gained) {
        e = ended_with_a(ended_nz(g.carried ? g.high : g.low), g.low);
        return ended_with_overflow(ended_with_carry(e, g.carried), g.overflow);
    }
    e = ended_with_a(ended_nz((uint8_t)(g.high - 7)), g.high);
    return ended_with_carry(e, true);
}

/* StartSoleScriptSlot's exit. */
static Ended ended_of_sole(SoleSlot r)
{
    uint8_t a = r.started ? r.script : r.busy_kind;

    return ended_with_carry(ended_with_y(ended_load(a), r.script), r.started);
}

/*
 * @routine 09EA:55A FaceByCue
 * @run 70:AD5A
 * WHAT IT DOES: While the screen is not scrolling along X, and in a state the table at $AD70 marks, the object
 * faces the way the player is (bit 0 of SideOfPlayer into its facing).
 * HOW: Reads ScrollSpeedX and the table by state, then SideOfPlayer.
 * WAITS: no.
 * Entry: X object. Exit: A and N, Z from the last load or AND; Y the state when the table was read.
 */
Ended FaceByCue(uint8_t slot)
{
    SideResult side;
    uint8_t speed, state, mark, bits;
    Ended e;

    speed = rd8(ScrollSpeedX);
    e = ended_load(speed);
    if (speed != 0) {
        return e;
    }
    state = RD8(OBJ->state[slot]);
    mark = rd8((uint16_t)(0xAD70 + state));
    e = then(e, ended_with_y(ended_load(mark), state));
    if (mark == 0) {
        return e;
    }
    jsr_push(0xAD6A);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    bits = (uint8_t)(side.sides & 0x01);
    WR8(OBJ->facing_left[slot], bits);
    return then(e, ended_load(bits));
}

void abi_FaceByCue(void)
{
    abi_ended(FaceByCue(cpu.x));
}

/*
 * @routine 09EA:6CC WalkAndTurnAtEdges
 * @run 70:AECC
 * WHAT IT DOES: Moves the object along X; when its X plus the scroll reaches an edge (page 3 from $30, or any other
 * page below $48) it turns round: the X speed is reversed and the move made again.
 * HOW: ApplyVelocityX, the add of ScrollXLo and ScrollXHiScreen, then the compare; the turn is NegateVelocityX and a
 * tail jump into ApplyVelocityX.
 * Untraced: both arms that turn, and the page 3 branch (AEE9 and AEEF).
 * WAITS: no.
 * Entry: X object. Exit: no turn: A the low byte, N, Z, C from its compare; turn: as ApplyVelocityX.
 */
Ended WalkAndTurnAtEdges(uint8_t slot)
{
    AxisMove move;
    uint8_t low, high, scroll, edge;
    Sum sum;
    Ended e;
    bool turn;

    jsr_push(0xAECF);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    low = RD8(OBJ->x_lo[slot]);
    scroll = rd8(ScrollXLo);
    sum = add_bytes(low, scroll, false);
    WR8(ZP->temp00, sum.value);
    e = then(e, of_sum(sum));
    high = RD8(OBJ->x_hi[slot]);
    scroll = rd8(ScrollXHiScreen);
    sum = add_bytes(high, scroll, sum.carry);
    e = then(e, of_sum(sum));
    e = then(e, ended_compare(sum.value, 3));
    edge = RD8(ZP->temp00);
    if (sum.value == 3) {
        e = then(e, ended_compare(edge, 0x30));
        turn = edge >= 0x30;
    } else {
        e = then(e, ended_compare(edge, 0x48));
        turn = edge < 0x48;
    }
    if (!turn) {
        return e;
    }
    jsr_push(0xAEF2);
    NegateVelocityX(slot);
    jsr_pop();
    move = ApplyVelocityX(slot);
    return ended_of_axis_move(move);
}

void abi_WalkAndTurnAtEdges(void)
{
    abi_ended(WalkAndTurnAtEdges(cpu.x));
}

/*
 * @routine 09EA:6A2 WalkSideways
 * @run 70:AEA2
 * WHAT IT DOES: Steps the walk animation (list $AEC3), sets the X speed to $FFC0 (left), reverses it when the object's
 * direction byte OBJ_3568 is set, and moves with WalkAndTurnAtEdges.
 * HOW: StepObjectFrames, NegateVelocityX, WalkAndTurnAtEdges.
 * WAITS: no.
 * Entry: X object. Exit: as WalkAndTurnAtEdges.
 */
Ended WalkSideways(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0, direction;
    Ended e;

    step = step_list(slot, 0xAEC3, 0xAEAD, &y, false);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    WR8(OBJ->vx_hi[slot], 0xFF);
    WR8(OBJ->vx_lo[slot], 0xC0);
    direction = RD8(OBJ->angle_hi[slot]);
    e = then(e, ended_load(direction));
    if (direction != 0) {
        jsr_push(0xAEBF);
        e = then(e, of_sum(NegateVelocityX(slot)));
        jsr_pop();
    }
    jsr_push(0xAEC2);
    e = then(e, WalkAndTurnAtEdges(slot));
    jsr_pop();
    return e;
}

void abi_WalkSideways(void)
{
    abi_ended(WalkSideways(cpu.x));
}

/*
 * @routine 09EA:6F5 PickNextAction
 * @run 70:AEF5
 * WHAT IT DOES: While its wait OBJ_3510 runs, counts it down. At zero it picks a random state (from the pair of bytes
 * at $AF28), plays the sound the pair's second byte names, resets its animation and picks a new wait from the table
 * at $AF24 by a random 0 to 3.
 * HOW: NextRandom twice, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept unless the picks set it. Exit: waiting: A the old wait, N and Z from the new; else A the
 * new wait, Y its index, C and V from the second random.
 */
Ended PickNextAction(uint8_t slot)
{
    RandomResult random;
    uint8_t wait, picked, index, byte, left;
    Ended e;

    wait = RD8(OBJ->f3510[slot]);
    e = ended_load(wait);
    if (wait != 0) {
        left = DEC8(OBJ->f3510[slot]);
        return then(e, ended_nz(left));
    }
    jsr_push(0xAF01);
    random = NextRandom();
    jsr_pop();
    picked = (uint8_t)(random.value & 0x01);
    index = (uint8_t)(picked << 1);
    byte = rd8((uint16_t)(0xAF28 + index));
    WR8(OBJ->state[slot], byte);
    byte = rd8((uint16_t)(0xAF29 + index));
    jsr_push(0xAF11);
    SoundCommand(byte, slot, index);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    jsr_push(0xAF1A);
    random = NextRandom();
    jsr_pop();
    e = ended_of_random(random);
    index = (uint8_t)(random.value & 0x03);
    byte = rd8((uint16_t)(0xAF24 + index));
    WR8(OBJ->f3510[slot], byte);
    return then(e, ended_with_y(ended_load(byte), index));
}

void abi_PickNextAction(void)
{
    abi_ended(PickNextAction(cpu.x));
}

/*
 * @routine 09EA:689 WalkTowardPlayer
 * @run 70:AE89
 * WHAT IT DOES: PickNextAction and WalkSideways, then the facing byte becomes the walk direction and, if the player is
 * within $40 along X, the object enters state 2.
 * HOW: DistanceXToPlayer.
 * Untraced: the state 2 arm (AE9C).
 * WAITS: no.
 * Entry: X object. Exit: the last compare, A the distance.
 */
Ended WalkTowardPlayer(uint8_t slot)
{
    PlayerDistance distance;
    uint8_t facing;
    Ended e;

    jsr_push(0xAE8C);
    e = PickNextAction(slot);
    jsr_pop();
    jsr_push(0xAE8F);
    e = then(e, WalkSideways(slot));
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_load(facing));
    WR8(OBJ->angle_hi[slot], facing);
    jsr_push(0xAE98);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_distance(distance));
    e = then(e, ended_compare(distance.distance, 0x40));
    if (distance.distance >= 0x40) {
        return e;
    }
    WR8(OBJ->state[slot], 2);
    return then(e, ended_load(2));
}

void abi_WalkTowardPlayer(void)
{
    abi_ended(WalkTowardPlayer(cpu.x));
}

/*
 * @routine 09EA:7D0 StartCueScript
 * @run 70:AFD0
 * WHAT IT DOES: Starts the script slot 5 with the script $80 plus Y (the last part of PlayCueOnFrame).
 * HOW: X is pushed and pulled around StartScriptSlotKind4.
 * WAITS: no.
 * Entry: X object, Y the cue's index. Exit: A and Y the script, N and Z from X, C and V from the add of $80.
 */
Ended StartCueScript(uint8_t slot, uint8_t y)
{
    uint8_t script;
    Sum sum;
    Ended e;

    push8(slot);
    sum = add_bytes(y, 0x80, false);
    e = of_sum(sum);
    jsr_push(0xAFDA);
    script = StartScriptSlotKind4(sum.value, 5);
    jsr_pop();
    slot = pull8();
    e = then(e, ended_with_y(ended_load(script), script));
    return then(e, ended_nz(slot));
}

void abi_StartCueScript(void)
{
    abi_ended(StartCueScript(cpu.x, cpu.y));
}

/*
 * @routine 09EA:792 PlayCueOnFrame
 * @run 70:AF92
 * @stop AFB9
 * WHAT IT DOES: An object showing frame 8 or $0A starts a script when its animation timer equals one of the cue
 * counts in a table at $AFB9 (frame 8: the first eight; frame $0A: the next fourteen); the script is $80 plus the
 * cue's index.
 * HOW: A scan of the table with the timer, ending in StartCueScript.
 * Untraced: the table (AFB9 to AFCF) is data the traced run only read.
 * WAITS: no.
 * Entry: X object. Exit: no cue: A and flags of the last compare; cue: as StartCueScript.
 */
Ended PlayCueOnFrame(uint8_t slot)
{
    uint8_t frame, y, limit, timer, cue;
    Ended e;

    frame = RD8(OBJ->frame[slot]);
    e = ended_compare(frame, 8);
    if (frame == 8) {
        y = 0;
        e = then(e, ended_with_y(ended_load(8), y));
        limit = 8;
        WR8(ZP->temp00, limit);
    } else {
        e = then(e, ended_compare(frame, 0x0A));
        if (frame != 0x0A) {
            return e;
        }
        y = 8;
        e = then(e, ended_with_y(ended_load(0x16), y));
        limit = 0x16;
        WR8(ZP->temp00, limit);
    }
    do {
        timer = RD8(OBJ->anim_timer[slot]);
        cue = rd8((uint16_t)(0xAFB9 + y));
        e = then(e, ended_compare(timer, cue));
        if (timer == cue) {
            return then(e, StartCueScript(slot, y));
        }
        y++;
        e = then(e, ended_with_y(ended_nz(y), y));
        limit = RD8(ZP->temp00);
        e = then(e, cpy(y, limit));
    } while (y < limit);
    return e;
}

void abi_PlayCueOnFrame(void)
{
    abi_ended(PlayCueOnFrame(cpu.x));
}

/*
 * @routine 09EA:7DC StepFlyingCue
 * @run 70:AFDC
 * WHAT IT DOES: Steps the animation list at $AFFE; when it ends the object enters state 2; on a new frame, at animation
 * step $1A, sound $1A plays.
 * HOW: StepObjectFrames, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: see the wrapper.
 */
SoundEnd StepFlyingCue(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    FrameStep step;
    uint8_t anim;
    Ended e;

    step = step_list(slot, 0xAFFE, 0xAFE7, &y, true);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    r.end = e;
    if (step == FRAME_LIST_ENDED) {
        WR8(OBJ->state[slot], 2);
        r.end = then(e, ended_load(2));
        return r;
    }
    if (step != FRAME_CHANGED) {
        return r;
    }
    anim = RD8(OBJ->anim_step[slot]);
    r.end = then(e, ended_compare(anim, 0x1A));
    if (anim != 0x1A) {
        return r;
    }
    r.before = then(r.end, ended_load(0x1A));
    r.end = ended_kept();
    r.sounded = true;
    r.command = 0x1A;
    jsr_push(0xAFF7);
    r.call = SoundCommand(0x1A, slot, y);
    jsr_pop();
    return r;
}

void abi_StepFlyingCue(void)
{
    abi_sound_end(StepFlyingCue(cpu.x, cpu.y));
}

/*
 * @routine 09EA:664 StartTwinSpawner
 * @run 70:AE64
 * WHAT IT DOES: Loads the object's record, clears OBJ_3618, sets its wait to $60 and sets bit 2 of zField8E; then
 * clears objects $11 and $12 and starts a kind $22 and a kind $27 at this object's place in them.
 * HOW: LoadObjectRecord, ClearObjectInY, StartObjectAtPlace; the last is a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte copied, N and Z from it, Y $12, C from the record.
 */
Ended StartTwinSpawner(uint8_t slot)
{
    uint8_t nz, last, flags, placed;
    bool carry;

    jsr_push(0xAE67);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->f3618[slot], 0);
    WR8(OBJ->f3510[slot], 0x60);
    flags = RD8(ZP->field8_e);
    flags = (uint8_t)(flags | 0x04);
    WR8(ZP->field8_e, flags);
    jsr_push(0xAE7A);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xAE7F);
    StartObjectAtPlace(0x22, slot, 0x11);
    jsr_pop();
    jsr_push(0xAE84);
    ClearObjectInY(0x12);
    jsr_pop();
    placed = StartObjectAtPlace(0x27, slot, 0x12);
    return ended_with_carry(ended_with_y(ended_load(placed), 0x12), carry);
}

void abi_StartTwinSpawner(void)
{
    abi_ended(StartTwinSpawner(cpu.x));
}

/*
 * @routine 09EB:03B StepThenQuarterWait
 * @run 70:B03B
 * WHAT IT DOES: Steps the animation list at $B060; when it ends the object's wait OBJ_3510 is quartered, it enters state 2
 * and its timer is raised to at least 3.
 * HOW: StepObjectFrames, two LSRs of memory, a compare.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: list running: as the step; ended: the last compare or load.
 */
Ended StepThenQuarterWait(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0, timer, shifted;
    bool bit;
    Ended e;

    step = step_list(slot, 0xB060, 0xB046, &y, true);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step != FRAME_LIST_ENDED) {
        return e;
    }
    shifted = LSR8_VALUE(OBJ->f3510[slot], bit);
    e = then(e, ended_with_carry(ended_nz(shifted), bit));
    shifted = LSR8_VALUE(OBJ->f3510[slot], bit);
    e = then(e, ended_with_carry(ended_nz(shifted), bit));
    WR8(OBJ->state[slot], 2);
    e = then(e, ended_load(2));
    timer = RD8(OBJ->timer[slot]);
    e = then(e, ended_compare(timer, 3));
    if (timer >= 3) {
        return e;
    }
    WR8(OBJ->timer[slot], 3);
    return then(e, ended_load(3));
}

void abi_StepThenQuarterWait(void)
{
    abi_ended(StepThenQuarterWait(cpu.x));
}

/*
 * @routine 09EB:067 StartShotRing
 * @run 70:B067
 * WHAT IT DOES: Clears objects $11 and $12, starts ten objects of kind $28 at this object's place in slots $18 to $27, each
 * with this object's facing; then resets the animation, puts the object off the left of the screen (X from $C0 less the scroll,
 * Y $FFC0), stops it and enters its next state.
 * HOW: ClearObjectInY and StartObjectAtPlace in a loop.
 * WAITS: no.
 * Entry: X object. Exit: A $C0, Y $28, N and Z from the new state, C and V from the subtract of the scroll.
 */
Ended StartShotRing(uint8_t slot)
{
    uint8_t to, facing, scroll, state;
    Sum sum;
    Ended e;

    jsr_push(0xB06C);
    ClearObjectInY(0x11);
    jsr_pop();
    jsr_push(0xB070);
    ClearObjectInY(0x12);
    jsr_pop();
    for (to = 0x18; to < 0x28; to++) {
        jsr_push(0xB075);
        ClearObjectInY(to);
        jsr_pop();
        jsr_push(0xB07A);
        StartObjectAtPlace(0x28, slot, to);
        jsr_pop();
        facing = RD8(OBJ->facing_left[slot]);
        WR8(OBJ->facing_left[to], facing);
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->x_hi[slot], 0);
    scroll = rd8(ScrollXLo);
    sum = sub_bytes(0xC0, scroll, true);
    WR8(OBJ->x_lo[slot], sum.value);
    WR8(OBJ->y_hi[slot], 0xFF);
    WR8(OBJ->y_lo[slot], 0xC0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->frame[slot], 0);
    state = INC8(OBJ->state[slot]);
    e = of_sum(sum);
    e = then(e, ended_with_y(ended_with_a(ended_nz(state), 0xC0), 0x28));
    return e;
}

void abi_StartShotRing(void)
{
    abi_ended(StartShotRing(cpu.x));
}

/*
 * @routine 09EB:0AE EnterStateWhenSlotClear
 * @run 70:B0AE
 * WHAT IT DOES: The object enters its next state once object $26's kind is 0 (the one it waits for is gone).
 * HOW: A read of that kind and an INC.
 * WAITS: no.
 * Entry: X object. Exit: A the kind read, N and Z from it or from the new state.
 */
Ended EnterStateWhenSlotClear(uint8_t slot)
{
    uint8_t kind, state;

    kind = RD8(OBJ->kind[0x26]);
    if (kind != 0) {
        return ended_load(kind);
    }
    state = INC8(OBJ->state[slot]);
    return then(ended_load(kind), ended_nz(state));
}

void abi_EnterStateWhenSlotClear(void)
{
    abi_ended(EnterStateWhenSlotClear(cpu.x));
}

/*
 * @routine 09EB:0B7 BurstOnLanding
 * @run 70:B0B7
 * WHAT IT DOES: A thing that falls (list $B101, unlimited gravity) and bursts when it reaches Y $0040 on the first page: sound
 * $A1, script slot 3 started, kind $29 objects started in slots $12 to $19, a sideways speed of $0140 and the next state.
 * HOW: StepObjectFrames, ApplyGravityUnlimited, ApplyVelocityY, SoundCommand, StartSoleScriptSlot, StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
SoundEnd BurstOnLanding(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    FrameStep step;
    GravityResult gravity;
    AxisMove move;
    SoleSlot sole;
    uint8_t y_high, y_low, to, placed, state;
    Ended e;

    step = step_list(slot, 0xB101, 0xB0C2, &y, true);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    jsr_push(0xB0C5);
    gravity = ApplyGravityUnlimited(slot);
    jsr_pop();
    e = then(e, ended_of_gravity(gravity));
    jsr_push(0xB0C8);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(y_high));
    r.end = e;
    if (y_high != 0) {
        return r;
    }
    y_low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(y_low, 0x40));
    r.end = e;
    if (y_low < 0x40) {
        return r;
    }
    r.before = then(e, ended_load(0xA1));
    r.sounded = true;
    r.command = 0xA1;
    jsr_push(0xB0D9);
    r.call = SoundCommand(0xA1, slot, y);
    jsr_pop();
    push8(slot);
    jsr_push(0xB0E0);
    sole = StartSoleScriptSlot(0, 3);
    jsr_pop();
    slot = pull8();
    e = then(ended_load(0), ended_of_sole(sole));
    e = then(e, ended_nz(slot));
    for (to = 0x12; to < 0x1A; to++) {
        jsr_push(0xB0E8);
        placed = StartObjectAtPlace(0x29, slot, to);
        jsr_pop();
        e = then(e, ended_with_y(ended_load(placed), to));
        e = then(e, ended_with_y(cpy((uint8_t)(to + 1), 0x1A), (uint8_t)(to + 1)));
    }
    WR8(OBJ->vx_hi[slot], 1);
    WR8(OBJ->vx_lo[slot], 0x40);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    state = INC8(OBJ->state[slot]);
    r.end = then(e, ended_with_a(ended_nz(state), 0x40));
    return r;
}

void abi_BurstOnLanding(void)
{
    SoundEnd r = BurstOnLanding(cpu.x, cpu.y);

    /* the sound call's leak sits before the later instructions: the wrapper shows it first, then the rest */
    abi_sound_end(r);
}

/*
 * @routine 09EB:16F InWindowRange
 * @run 70:B16F
 * WHAT IT DOES: Says whether the object is inside the page range the stage keeps its things alive in: the object's X plus the scroll,
 * high byte 3 or more with the low byte $C0 or more, or high byte 1 with the low byte under $C0.
 * HOW: The add of ScrollXLo and ScrollXHiScreen and two compares.
 * WAITS: no.
 * Entry: X object. Exit: C set when inside; A the last load, N and Z from the last compare, V from the add.
 */
ViewTest InWindowRange(uint8_t slot)
{
    ViewTest r = { false, { 0 } };
    uint8_t low, high, scroll, edge;
    Sum sum;
    Ended e;

    low = RD8(OBJ->x_lo[slot]);
    scroll = rd8(ScrollXLo);
    sum = add_bytes(low, scroll, false);
    WR8(ZP->temp00, sum.value);
    e = of_sum(sum);
    high = RD8(OBJ->x_hi[slot]);
    scroll = rd8(ScrollXHiScreen);
    sum = add_bytes(high, scroll, sum.carry);
    e = then(e, of_sum(sum));
    e = then(e, ended_compare(sum.value, 3));
    if (sum.value >= 3) {
        edge = RD8(ZP->temp00);
        e = then(e, ended_compare(edge, 0xC0));
        r.in_view = edge >= 0xC0;
        r.end = then(e, ended_with_carry(ended_kept(), r.in_view));
        return r;
    }
    e = then(e, ended_compare(sum.value, 1));
    if (sum.value != 1) {
        r.end = then(e, ended_with_carry(ended_kept(), false));
        return r;
    }
    edge = RD8(ZP->temp00);
    e = then(e, ended_compare(edge, 0xC0));
    r.in_view = edge < 0xC0;
    r.end = then(e, ended_with_carry(ended_kept(), r.in_view));
    return r;
}

void abi_InWindowRange(void)
{
    abi_ended(InWindowRange(cpu.x).end);
}

/* The start two splitters share: speeds from a random pick, Y low byte given, then the side. */
static Ended splitter_start(uint8_t slot, uint8_t carry_in, uint8_t y_low, uint16_t ret_sum, uint16_t ret_side, uint16_t ret_negate)
{
    Sum distance;
    SideResult side;
    uint8_t picked, index, byte, state, bits;
    Ended e;

    jsr_push(ret_sum);
    distance = PlayerDistanceSum(slot, carry_in != 0);
    jsr_pop();
    e = of_sum(distance);
    picked = (uint8_t)(distance.value & 0x03);
    index = (uint8_t)(picked << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), false), index));
    byte = rd8((uint16_t)(0xB1C0 + index));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xB1BF + index));
    WR8(OBJ->vx_lo[slot], byte);
    WR8(OBJ->y_lo[slot], y_low);
    WR8(OBJ->f3670[slot], 0);
    state = INC8(OBJ->f2800[slot]);
    e = then(e, ended_with_a(ended_nz(state), y_low));
    jsr_push(ret_side);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    bits = (uint8_t)(side.sides & 0x01);
    e = then(e, ended_load(bits));
    if (bits == 0) {
        return e;
    }
    jsr_push(ret_negate);
    e = then(e, of_sum(NegateVelocityX(slot)));
    jsr_pop();
    return e;
}

/*
 * @routine 09EB:196 StartSplitterHigh
 * @run 70:B196
 * WHAT IT DOES: Starts a falling splitter at Y low $48: its X speed is picked from a table at $B1BF by two bits of the
 * player's distance sum, OBJ_3670 is cleared, OBJ_2800 moves on, and when the object is on the player's left its X speed is
 * reversed.
 * HOW: PlayerDistanceSum, SideOfPlayer, NegateVelocityX.
 * Untraced: the reversal (B1BB).
 * WAITS: no.
 * Entry: X object, C passed to PlayerDistanceSum. Exit: the last load, AND or subtract.
 */
Ended StartSplitterHigh(uint8_t slot, bool carry_in)
{
    return splitter_start(slot, carry_in, 0x48, 0xB199, 0xB1B7, 0xB1BE);
}

void abi_StartSplitterHigh(void)
{
    abi_ended(StartSplitterHigh(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 09EB:1E8 StartSplitterLow
 * @run 70:B1E8
 * WHAT IT DOES: StartSplitterHigh with the Y low byte $58.
 * HOW: As StartSplitterHigh.
 * Untraced: the reversal (B20D).
 * WAITS: no.
 * Entry and exit: as StartSplitterHigh.
 */
Ended StartSplitterLow(uint8_t slot, bool carry_in)
{
    return splitter_start(slot, carry_in, 0x58, 0xB1EB, 0xB209, 0xB210);
}

void abi_StartSplitterLow(void)
{
    abi_ended(StartSplitterLow(cpu.x, (cpu.p & P_C) != 0));
}

/*
 * @routine 09EB:21A MaybeShootAhead
 * @run 70:B21A
 * WHAT IT DOES: Every 32nd count of the object's OBJ_35C0, with a chance of three in four, starts a kind $2A object at its place in
 * the first free slot from $30 and gives it the object's facing.
 * HOW: NextRandom, FindFreeSlotFrom30, StartObjectAtPlace.
 * Untraced: one byte.
 * WAITS: no.
 * Entry: X object. Exit: the last load, AND or INC.
 */
Ended MaybeShootAhead(uint8_t slot)
{
    RandomResult random;
    uint8_t count, picked, last, found_slot, placed, facing;
    bool found;
    Ended e;

    count = RD8(OBJ->angle_lo[slot]);
    count = (uint8_t)(count & 0x1F);
    e = ended_load(count);
    if (count != 0) {
        return e;
    }
    jsr_push(0xB224);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    picked = (uint8_t)(random.value & 0x03);
    e = then(e, ended_load(picked));
    if (picked == 0) {
        return e;
    }
    jsr_push(0xB22B);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = then(e, ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), last), found_slot), found));
    if (!found) {
        return e;
    }
    jsr_push(0xB232);
    placed = StartObjectAtPlace(0x2A, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], facing);
    return then(e, ended_load(facing));
}

void abi_MaybeShootAhead(void)
{
    abi_ended(MaybeShootAhead(cpu.x));
}

/*
 * @routine 09EB:51E PickWaterLevel
 * @run 70:B51E
 * WHAT IT DOES: Sets the water the stage wants (RasterWaterWanted and LevelYHi) to a two-byte pair from a table at $B531 picked by
 * two bits of the wave counter.
 * HOW: Reads the wave counter and the table (bank 70).
 * WAITS: no.
 * Entry: nothing. Exit: A the second byte, Y the index, N and Z from A, C 0.
 */
Ended PickWaterLevel(void)
{
    uint8_t wave, index, byte;

    wave = RD8(ZP->wave_counter);
    wave = (uint8_t)(wave & 0x03);
    index = (uint8_t)(wave << 1);
    byte = rd8((uint16_t)(0xB531 + index));
    wr8(RasterWaterWanted, byte);
    byte = rd8((uint16_t)(0xB532 + index));
    wr8(LevelYHi, byte);
    return ended_with_carry(ended_with_y(ended_load(byte), index), false);
}

void abi_PickWaterLevel(void)
{
    abi_ended(PickWaterLevel());
}

/*
 * @routine 09EB:55F WaitForPage2
 * @run 70:B55F
 * WHAT IT DOES: When the level position's high byte is 2, the object's timer is set to $40 and it enters its next state; the
 * object in slot $10 also plays sound $A2.
 * HOW: A compare of ScrollXHiScreen, an INC, SoundCommand.
 * WAITS: no.
 * Entry: X object, Y pushed by the sound driver. Exit: see the wrapper.
 */
SoundEnd WaitForPage2(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t page, state;
    Ended e;

    page = rd8(ScrollXHiScreen);
    e = ended_compare(page, 2);
    r.end = e;
    if (page != 2) {
        return r;
    }
    WR8(OBJ->life_timer[slot], 0x40);
    e = then(e, ended_load(0x40));
    state = INC8(OBJ->state[slot]);
    e = then(e, ended_nz(state));
    e = then(e, cpy(slot, 0x10));
    r.end = e;
    if (slot != 0x10) {
        return r;
    }
    r.before = then(e, ended_load(0xA2));
    r.end = ended_kept();
    r.sounded = true;
    r.command = 0xA2;
    jsr_push(0xB577);
    r.call = SoundCommand(0xA2, slot, y);
    jsr_pop();
    return r;
}

void abi_WaitForPage2(void)
{
    abi_sound_end(WaitForPage2(cpu.x, cpu.y));
}

/*
 * @routine 09EB:5BB RiseUntilAboveGround
 * @run 70:B5BB
 * WHAT IT DOES: Moves the object, takes $0040 off its Y speed (it climbs), and when its Y position has passed from negative
 * to a low byte under $80 it enters its next state.
 * HOW: ApplyVelocity, a 16-bit subtract.
 * WAITS: no.
 * Entry: X object. Exit: the last load or INC.
 */
Ended RiseUntilAboveGround(uint8_t slot)
{
    AxisMove move;
    uint8_t low, high, y_high, y_low, state;
    Sum sum;
    Ended e;

    jsr_push(0xB5BE);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    low = RD8(OBJ->vy_lo[slot]);
    sum = sub_bytes(low, 0x40, true);
    WR8(OBJ->vy_lo[slot], sum.value);
    e = then(e, of_sum(sum));
    high = RD8(OBJ->vy_hi[slot]);
    sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(e, of_sum(sum));
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(y_high));
    if ((y_high & 0x80) == 0) {
        return e;
    }
    y_low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_load(y_low));
    if ((y_low & 0x80) != 0) {
        return e;
    }
    state = INC8(OBJ->state[slot]);
    return then(e, ended_nz(state));
}

void abi_RiseUntilAboveGround(void)
{
    abi_ended(RiseUntilAboveGround(cpu.x));
}

/*
 * @routine 09EB:5DD WaitForSlot10State4
 * @run 70:B5DD
 * WHAT IT DOES: Once the object in slot $10 has reached state 4, this object's timer becomes $40 and it enters its next state.
 * HOW: A compare of that state.
 * WAITS: no.
 * Entry: X object. Exit: the last compare, load or INC.
 */
Ended WaitForSlot10State4(uint8_t slot)
{
    uint8_t state, mine;
    Ended e;

    state = RD8(OBJ->state[0x10]);
    e = ended_compare(state, 4);
    if (state < 4) {
        return e;
    }
    WR8(OBJ->life_timer[slot], 0x40);
    e = then(e, ended_load(0x40));
    mine = INC8(OBJ->state[slot]);
    return then(e, ended_nz(mine));
}

void abi_WaitForSlot10State4(void)
{
    abi_ended(WaitForSlot10State4(cpu.x));
}

/*
 * @routine 09EB:7EB StartNumberedPiece
 * @run 70:B7EB
 * WHAT IT DOES: Loads the object's record; its timer comes from a table at $B801 by the slot less $18; its frame is that number plus
 * $12.
 * HOW: LoadObjectRecord and an add.
 * WAITS: no.
 * Entry: X object. Exit: A the frame, N, Z, C and V from the add, Y the number.
 */
Ended StartNumberedPiece(uint8_t slot)
{
    uint8_t nz, byte;
    bool carry;
    Sum number, frame;

    jsr_push(0xB7EE);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    number = sub_bytes(slot, 0x18, true);
    byte = rd8((uint16_t)(0xB801 + number.value));
    WR8(OBJ->life_timer[slot], byte);
    frame = add_bytes(number.value, 0x12, false);
    WR8(OBJ->frame[slot], frame.value);
    return ended_with_y(of_sum(frame), number.value);
}

void abi_StartNumberedPiece(void)
{
    abi_ended(StartNumberedPiece(cpu.x));
}

/*
 * @routine 09EB:7A4 FollowPlayerHitbox
 * @run 70:B7A4
 * WHAT IT DOES: In state 0 loads the object's record; later it takes the player's place and sets its hitbox from a four-byte row of the
 * table at $BC8B picked by the player's frame less 5.
 * HOW: LoadObjectRecord (a tail jump), CopyObjectPlaceBack from slot $10, the table (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: state 0: as LoadObjectRecord; else A the last table byte, Y the row, N and Z from A, C and V from the subtract.
 */
Ended FollowPlayerHitbox(uint8_t slot)
{
    uint8_t state, nz, last, frame, index, byte;
    bool carry;
    Sum diff;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return ended_of_record(last, nz, carry);
    }
    jsr_push(0xB7B1);
    byte = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    e = ended_with_y(ended_load(byte), 0x10);
    frame = RD8(OBJ->frame[0x10]);
    diff = sub_bytes(frame, 5, true);
    index = (uint8_t)(diff.value << 2);
    e = then(e, ended_with_y(ended_with_carry(of_sum(diff), (diff.value & 0x40) != 0), index));
    byte = rd8((uint16_t)(0xBC8B + index));
    WR8(OBJ->hitbox_dx[slot], byte);
    byte = rd8((uint16_t)(0xBC8C + index));
    WR8(OBJ->hitbox_dy[slot], byte);
    byte = rd8((uint16_t)(0xBC8D + index));
    WR8(OBJ->hitbox_half_w[slot], byte);
    byte = rd8((uint16_t)(0xBC8E + index));
    WR8(OBJ->hitbox_half_h[slot], byte);
    return then(e, ended_load(byte));
}

void abi_FollowPlayerHitbox(void)
{
    abi_ended(FollowPlayerHitbox(cpu.x));
}

/*
 * @routine 09EC:01A RiseAndClear
 * @run 70:B81A
 * WHAT IT DOES: Moves the object, takes $0040 off its Y speed (it climbs) and clears it once its Y position has passed from negative
 * to a low byte under $80.
 * HOW: ApplyVelocity, a 16-bit subtract, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: the last load or subtract.
 */
Ended RiseAndClear(uint8_t slot)
{
    AxisMove move;
    uint8_t low, high, y_high, y_low;
    Sum sum;
    Ended e;

    jsr_push(0xB81D);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    low = RD8(OBJ->vy_lo[slot]);
    sum = sub_bytes(low, 0x40, true);
    WR8(OBJ->vy_lo[slot], sum.value);
    e = then(e, of_sum(sum));
    high = RD8(OBJ->vy_hi[slot]);
    sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(e, of_sum(sum));
    y_high = RD8(OBJ->y_hi[slot]);
    e = then(e, ended_load(y_high));
    if ((y_high & 0x80) == 0) {
        return e;
    }
    y_low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_load(y_low));
    if ((y_low & 0x80) != 0) {
        return e;
    }
    ClearObject(slot);
    return e;
}

void abi_RiseAndClear(void)
{
    abi_ended(RiseAndClear(cpu.x));
}

/*
 * @routine 09EC:059 RaiseStateAfterSlot10
 * @run 70:B859
 * WHAT IT DOES: Once the object in slot $10 is in state $0A or later, an object below state 3 enters state 3.
 * HOW: Two compares.
 * Untraced: the whole arm after the first compare (B864 to B86C).
 * WAITS: no.
 * Entry: X object. Exit: the last compare or load.
 */
Ended RaiseStateAfterSlot10(uint8_t slot)
{
    uint8_t lead, state;
    Ended e;

    lead = RD8(OBJ->state[0x10]);
    e = ended_compare(lead, 0x0A);
    if (lead < 0x0A) {
        return e;
    }
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_compare(state, 3));
    if (state >= 3) {
        return e;
    }
    WR8(OBJ->state[slot], 3);
    return then(e, ended_load(3));
}

void abi_RaiseStateAfterSlot10(void)
{
    abi_ended(RaiseStateAfterSlot10(cpu.x));
}

/*
 * @routine 09EC:06D StartChainLink
 * @run 70:B86D
 * WHAT IT DOES: Loads the object's record, sets OBJ_3510 to $41 and its timer from a table at $B888 by the slot less $12; the object in slot
 * $19 also moves to its next frame.
 * HOW: LoadObjectRecord and an INC.
 * WAITS: no.
 * Entry: X object. Exit: slot $19: N and Z of the new frame; else the compare of X with $19.
 */
Ended StartChainLink(uint8_t slot)
{
    uint8_t nz, byte, frame;
    bool carry;
    Sum number;
    Ended e;

    jsr_push(0xB870);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x41);
    number = sub_bytes(slot, 0x12, true);
    e = ended_with_y(of_sum(number), number.value);
    byte = rd8((uint16_t)(0xB888 + number.value));
    WR8(OBJ->life_timer[slot], byte);
    e = then(e, ended_load(byte));
    e = then(e, cpy(slot, 0x19));
    if (slot != 0x19) {
        return e;
    }
    frame = INC8(OBJ->frame[slot]);
    return then(e, ended_nz(frame));
}

void abi_StartChainLink(void)
{
    abi_ended(StartChainLink(cpu.x));
}

/*
 * @routine 09EC:0F4 ChainFollowsPrevious
 * @run 70:B8F4
 * WHAT IT DOES: A link of a chain: object $12 takes the place of slot $10 and moves sideways by a table byte ($B923) picked by that
 * slot's facing, with Y speed 8; any other link takes the place of the link before it and takes its speed from its angle OBJ_3510 with a
 * length of $0A00; both then move.
 * HOW: CopyObjectPlaceBack, SetVelocityFromAngle, and a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity, Y the table index, or 0 as the angle scale leaves it.
 */
Ended ChainFollowsPrevious(uint8_t slot)
{
    AxisMove move;
    uint8_t facing, byte, angle, before;
    Ended e;

    e = cpy(slot, 0x12);
    if (slot == 0x12) {
        jsr_push(0xB8FD);
        byte = CopyObjectPlaceBack(0x10, slot);
        jsr_pop();
        facing = RD8(OBJ->facing_left[0x10]);
        byte = rd8((uint16_t)(0xB923 + facing));
        WR8(OBJ->vx_hi[slot], byte);
        WR8(OBJ->vy_hi[slot], 8);
        move = ApplyVelocity(slot);
        return then(ended_with_y(ended_kept(), facing), ended_of_axis_move(move));
    }
    before = (uint8_t)(slot - 1);
    jsr_push(0xB914);
    CopyObjectPlaceBack(before, slot);
    jsr_pop();
    WR8(ZP->temp00, 0);
    WR8(ZP->temp01, 0x0A);
    angle = RD8(OBJ->f3510[slot]);
    jsr_push(0xB920);
    SetVelocityFromAngle(angle, slot);
    jsr_pop();
    move = ApplyVelocity(slot);
    e = then(ended_with_y(ended_kept(), 0), ended_of_axis_move(move));
    return e;
}

void abi_ChainFollowsPrevious(void)
{
    abi_ended(ChainFollowsPrevious(cpu.x));
}

/*
 * @routine 09EC:1A5 StartKickedPiece
 * @run 70:B9A5
 * WHAT IT DOES: Loads the object's record, gives it an X speed from a table at $B9BD by its facing and a Y speed of $10, moves it once and
 * stops its Y speed.
 * HOW: LoadObjectRecord, ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity with Y the facing.
 */
Ended StartKickedPiece(uint8_t slot)
{
    AxisMove move;
    uint8_t nz, facing, byte;
    bool carry;

    jsr_push(0xB9A8);
    LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    byte = rd8((uint16_t)(0xB9BD + facing));
    WR8(OBJ->vx_hi[slot], byte);
    WR8(OBJ->vy_hi[slot], 0x10);
    jsr_push(0xB9B9);
    move = ApplyVelocity(slot);
    jsr_pop();
    WR8(OBJ->vy_hi[slot], 0);
    return then(ended_with_y(ended_kept(), facing), ended_of_axis_move(move));
}

void abi_StartKickedPiece(void)
{
    abi_ended(StartKickedPiece(cpu.x));
}

/*
 * @routine 09EC:203 StepThenClear
 * @run 70:BA03
 * WHAT IT DOES: Steps the animation list at $BA14; when it ends the object is cleared.
 * HOW: StepObjectFrames and a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step.
 */
Ended StepThenClear(uint8_t slot)
{
    FrameStep step;
    uint8_t y = 0;

    step = step_list(slot, 0xBA14, 0xBA0E, &y, true);
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
    }
    return ended_of_frame_step(step, step != FRAME_HELD, y);
}

void abi_StepThenClear(void)
{
    abi_ended(StepThenClear(cpu.x));
}

/*
 * @routine 09ED:032 ClearAnimation
 * @run 71:C032
 * WHAT IT DOES: Restarts the object's animation: step and timer 0.
 * HOW: Two stores.
 * WAITS: no.
 * Entry: X object. Exit: every register and flag kept.
 */
void ClearAnimation(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

void abi_ClearAnimation(void)
{
    ClearAnimation(cpu.x);
}

/*
 * @routine 09ED:06E EnterNextAndReset
 * @run 71:C06E
 * WHAT IT DOES: Moves the object on one state and its OBJ_3510 on one, restarts its animation and gives it a countdown of $20 in
 * its angle byte.
 * HOW: Two INCs, ClearAnimation, a store.
 * WAITS: no.
 * Entry: X object. Exit: A $20, N and Z from it.
 */
Ended EnterNextAndReset(uint8_t slot)
{
    INC8(OBJ->state[slot]);
    INC8(OBJ->f3510[slot]);
    jsr_push(0xC077);
    ClearAnimation(slot);
    jsr_pop();
    WR8(OBJ->angle_hi[slot], 0x20);
    return ended_load(0x20);
}

void abi_EnterNextAndReset(void)
{
    abi_ended(EnterNextAndReset(cpu.x));
}

/*
 * @routine 09ED:07D FlipSide
 * @run 71:C07D
 * WHAT IT DOES: Flips bit 0 of the object's life timer (the side it works on).
 * HOW: An EOR.
 * WAITS: no.
 * Entry: X object. Exit: A the new value, N and Z from it.
 */
Ended FlipSide(uint8_t slot)
{
    uint8_t side;

    side = RD8(OBJ->life_timer[slot]);
    side = (uint8_t)(side ^ 0x01);
    WR8(OBJ->life_timer[slot], side);
    return ended_load(side);
}

void abi_FlipSide(void)
{
    abi_ended(FlipSide(cpu.x));
}

/*
 * @routine 09ED:039 StepListByPhase
 * @run 71:C039
 * WHAT IT DOES: The object faces as its life timer says and steps the animation list that a table at $C103 names for its OBJ_3510.
 * HOW: Reads the list's address into zTemp10-11; a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object. Exit: as StepObjectFrames, Y kept unless the step sets it (the index otherwise).
 */
Ended StepListByPhase(uint8_t slot)
{
    FrameStep step;
    uint8_t facing, phase, index, y;

    facing = RD8(OBJ->life_timer[slot]);
    WR8(OBJ->facing_left[slot], facing);
    phase = RD8(OBJ->f3510[slot]);
    index = (uint8_t)(phase << 1);
    y = index;
    WR8(ZP->temp10, rd8((uint16_t)(0xC103 + index)));
    WR8(ZP->temp11, rd8((uint16_t)(0xC104 + index)));
    step = StepObjectFrames(slot, &y);
    return ended_of_frame_step(step, true, y);
}

void abi_StepListByPhase(void)
{
    Ended e = StepListByPhase(cpu.x);

    abi_ended(e);
}

/*
 * @routine 09ED:0C3 SetSwaySpeed
 * @run 71:C0C3
 * WHAT IT DOES: Sets the object's X speed from a word of a table at $C0DE, picked by OBJ_3510 times 4 plus life timer times 2.
 * HOW: Two shifts, an add with carry, two table reads (bank 71).
 * WAITS: no.
 * Entry: X object. Exit: A the high byte, Y the index, N and Z from A, C and V from the add.
 */
Ended SetSwaySpeed(uint8_t slot)
{
    uint8_t phase, side, byte, mix;
    bool carry;
    Sum sum;

    phase = RD8(OBJ->f3510[slot]);
    mix = (uint8_t)(phase << 2);
    WR8(ZP->temp10, mix);
    side = RD8(OBJ->life_timer[slot]);
    carry = (side & 0x80) != 0;
    mix = RD8(ZP->temp10);
    sum = add_bytes((uint8_t)(side << 1), mix, carry);
    byte = rd8((uint16_t)(0xC0DE + sum.value));
    WR8(OBJ->vx_lo[slot], byte);
    byte = rd8((uint16_t)(0xC0DF + sum.value));
    WR8(OBJ->vx_hi[slot], byte);
    return ended_with_y(then(of_sum(sum), ended_load(byte)), sum.value);
}

void abi_SetSwaySpeed(void)
{
    abi_ended(SetSwaySpeed(cpu.x));
}

/*
 * @routine 09ED:086 BobAndMove
 * @run 71:C086
 * WHAT IT DOES: Makes the object bob: OBJ_3618 counts up and its sine (scaled by 3FF) added to OBJ_35C0 is its Y low byte; between
 * Y $B0 and the right place on screen (X from $80 to $9F once the scroll is added) a random wobble of -1 to 2 is added; then it moves.
 * HOW: ScaleBySine, NextRandom, a tail jump into ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity, Y 0 as the sine's exit leaves it.
 */
Ended BobAndMove(uint8_t slot)
{
    AxisMove move;
    RandomResult random;
    uint8_t phase, base, x_low, wobble;
    Sum sum;

    WR8(ZP->temp00, 0xFF);
    WR8(ZP->temp01, 3);
    INC8(OBJ->f3618[slot]);
    phase = RD8(OBJ->f3618[slot]);
    jsr_push(0xC097);
    ScaleBySine(phase);
    jsr_pop();
    base = RD8(ZP->product.hi);
    sum = add_bytes(base, RD8(OBJ->angle_lo[slot]), false);
    WR8(OBJ->y_lo[slot], sum.value);
    if (sum.value < 0xB0) {
        x_low = RD8(OBJ->x_lo[slot]);
        sum = add_bytes(x_low, rd8(ScrollXLo), false);
        if (sum.value >= 0x80 && sum.value < 0xA0) {
            jsr_push(0xC0B6);
            random = NextRandom();
            jsr_pop();
            wobble = (uint8_t)((random.value & 0x03) - 1);
            base = RD8(OBJ->y_lo[slot]);
            sum = add_bytes(base, wobble, false);
            WR8(OBJ->y_lo[slot], sum.value);
        }
    }
    move = ApplyVelocity(slot);
    return then(ended_with_y(ended_kept(), 0), ended_of_axis_move(move));
}

void abi_BobAndMove(void)
{
    abi_ended(BobAndMove(cpu.x));
}

/*
 * @routine 09ED:051 ReactAtWindowEdge
 * @run 71:C051
 * WHAT IT DOES: When the object's X plus the scroll is at the edge its side (life timer 0 or 1) names ($E0 or more for 0, under $20 for 1) and it
 * is in state 1, it goes on as EnterNextAndReset.
 * HOW: ScrollXLo and two compares; the last step runs on into the next routine's bytes.
 * Untraced: one byte (the fall-through branch).
 * WAITS: no.
 * Entry: X object. Exit: the last compare or the load; Y the side.
 */
Ended ReactAtWindowEdge(uint8_t slot)
{
    uint8_t x_low, side, state;
    Sum sum;
    Ended e;

    x_low = RD8(OBJ->x_lo[slot]);
    sum = add_bytes(x_low, rd8(ScrollXLo), false);
    e = of_sum(sum);
    side = RD8(OBJ->life_timer[slot]);
    e = then(e, ended_with_y(ended_nz(side), side));
    if (side == 0) {
        e = then(e, ended_compare(sum.value, 0xE0));
        if (sum.value < 0xE0) {
            return e;
        }
    } else {
        e = then(e, ended_compare(sum.value, 0x20));
        if (sum.value >= 0x20) {
            return e;
        }
    }
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_compare(state, 1));
    if (state != 1) {
        return e;
    }
    return then(e, EnterNextAndReset(slot));
}

void abi_ReactAtWindowEdge(void)
{
    abi_ended(ReactAtWindowEdge(cpu.x));
}

/* The three steps of a swaying object, which two routines branch into the middle of: the list, the speed, the bob. */
Ended SwayStep(uint8_t slot)
{
    Ended e;

    jsr_push(0xC006);
    e = StepListByPhase(slot);
    jsr_pop();
    jsr_push(0xC009);
    e = then(e, SetSwaySpeed(slot));
    jsr_pop();
    return then(e, BobAndMove(slot));
}

/*
 * @routine 09EC:7FE SwayOrAdvance
 * @run 70:BFFE
 * WHAT IT DOES: Counts the object's angle byte down; at zero it goes on as EnterNextAndReset, otherwise it sways (SwayStep).
 * HOW: DEC, then the tail calls.
 * WAITS: no.
 * Entry: X object. Exit: as EnterNextAndReset or BobAndMove.
 */
Ended SwayOrAdvance(uint8_t slot)
{
    uint8_t left;

    left = DEC8(OBJ->angle_hi[slot]);
    if (left == 0) {
        return EnterNextAndReset(slot);
    }
    return then(ended_nz(left), SwayStep(slot));
}

void abi_SwayOrAdvance(void)
{
    abi_ended(SwayOrAdvance(cpu.x));
}

/*
 * @routine 09EC:7F2 SwayThenReact
 * @run 70:BFF2
 * WHAT IT DOES: Steps the animation by phase, sets the sway speed, bobs and moves, then reacts at the window edge.
 * HOW: StepListByPhase, SetSwaySpeed, BobAndMove, a tail jump into ReactAtWindowEdge.
 * WAITS: no.
 * Entry: X object. Exit: as ReactAtWindowEdge.
 */
Ended SwayThenReact(uint8_t slot)
{
    Ended e;

    jsr_push(0xBFF5);
    e = StepListByPhase(slot);
    jsr_pop();
    jsr_push(0xBFF8);
    e = then(e, SetSwaySpeed(slot));
    jsr_pop();
    jsr_push(0xBFFB);
    e = then(e, BobAndMove(slot));
    jsr_pop();
    return then(e, ReactAtWindowEdge(slot));
}

void abi_SwayThenReact(void)
{
    abi_ended(SwayThenReact(cpu.x));
}

/*
 * @routine 09ED:00C SwayThenFlip
 * @run 71:C00C
 * WHAT IT DOES: Counts the angle byte down; at zero it flips the side, goes on as EnterNextAndReset and sets OBJ_3510 to 1; otherwise it sways.
 * HOW: DEC, FlipSide, EnterNextAndReset, or SwayStep.
 * WAITS: no.
 * Entry: X object. Exit: at zero A 1 from the load; else as BobAndMove.
 */
Ended SwayThenFlip(uint8_t slot)
{
    uint8_t left;
    Ended e;

    left = DEC8(OBJ->angle_hi[slot]);
    e = ended_nz(left);
    if (left != 0) {
        return then(e, SwayStep(slot));
    }
    jsr_push(0xC016);
    e = then(e, FlipSide(slot));
    jsr_pop();
    jsr_push(0xC019);
    e = then(e, EnterNextAndReset(slot));
    jsr_pop();
    WR8(OBJ->f3510[slot], 1);
    return then(e, ended_load(1));
}

void abi_SwayThenFlip(void)
{
    abi_ended(SwayThenFlip(cpu.x));
}

/*
 * @routine 09ED:01F SwayThenRestart
 * @run 71:C01F
 * WHAT IT DOES: Counts the angle byte down; at zero it goes on as EnterNextAndReset, puts the object in state 1 and OBJ_3510 at 0; otherwise it sways.
 * HOW: DEC, EnterNextAndReset, or SwayStep.
 * WAITS: no.
 * Entry: X object. Exit: at zero A 1 from the load; else as BobAndMove.
 */
Ended SwayThenRestart(uint8_t slot)
{
    uint8_t left;
    Ended e;

    left = DEC8(OBJ->angle_hi[slot]);
    e = ended_nz(left);
    if (left != 0) {
        return then(e, SwayStep(slot));
    }
    jsr_push(0xC029);
    e = then(e, EnterNextAndReset(slot));
    jsr_pop();
    WR8(OBJ->state[slot], 1);
    WR8(OBJ->f3510[slot], 0);
    return then(e, ended_load(1));
}

void abi_SwayThenRestart(void)
{
    abi_ended(SwayThenRestart(cpu.x));
}

/*
 * @routine 09EC:7BC JumpByStateTable
 * @run 70:BFBC
 * WHAT IT DOES: Goes to the code a table at $BFCE names for the object's state.
 * HOW: The table's two bytes go to zTemp10-11 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTable(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xBFCE + index));
    WR8(ZP->temp10, low);
    high = rd8((uint16_t)(0xBFCF + index));
    WR8(ZP->temp11, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp10, ZP->temp11);
    return jump;
}

void abi_JumpByStateTable(void)
{
    Jump jump = JumpByStateTable(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EC:090 ChainCountdown
 * @run 70:B890
 * WHAT IT DOES: A link of the chain follows the one before it (ChainFollowsPrevious); its timer counts down and at zero it enters its next state.
 * HOW: ChainFollowsPrevious, DEC, INC.
 * WAITS: no.
 * Entry: X object. Exit: as ChainFollowsPrevious, then N and Z of the count or of the new state.
 */
Ended ChainCountdown(uint8_t slot)
{
    uint8_t left, state;
    Ended e;

    jsr_push(0xB893);
    e = ChainFollowsPrevious(slot);
    jsr_pop();
    left = DEC8(OBJ->life_timer[slot]);
    e = then(e, ended_nz(left));
    if (left != 0) {
        return e;
    }
    state = INC8(OBJ->state[slot]);
    return then(e, ended_nz(state));
}

void abi_ChainCountdown(void)
{
    abi_ended(ChainCountdown(cpu.x));
}

/*
 * @routine 09EC:09C ChainSwings
 * @run 70:B89C
 * WHAT IT DOES: A link follows the one before it, takes slot $10's facing, and swings: the cosine of its angle OBJ_35C0 (multiplier $0200) is added to OBJ_3568 and OBJ_3510 as
 * a 16-bit pair, and a 16-bit step from a table at $B8E4 (by OBJ_3670 and 7) is added to OBJ_3618 and OBJ_35C0; a carry out of that goes up one on OBJ_3670.
 * HOW: ChainFollowsPrevious, ScaleByCosine, table reads (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: the last add or INC.
 */
Ended ChainSwings(uint8_t slot)
{
    uint8_t facing, angle, low, high, phase, index, step, next;
    Sum sum;
    Ended e;

    jsr_push(0xB89F);
    e = ChainFollowsPrevious(slot);
    jsr_pop();
    facing = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], facing);
    e = then(e, ended_load(facing));
    WR8(ZP->temp01, 2);
    WR8(ZP->temp00, 0);
    angle = RD8(OBJ->angle_lo[slot]);
    jsr_push(0xB8B3);
    ScaleByCosine(angle);
    jsr_pop();
    e = then(e, ended_with_y(ended_load(0), 0));
    low = RD8(OBJ->angle_hi[slot]);
    sum = add_bytes(low, RD8(ZP->product.mid), false);
    WR8(OBJ->angle_hi[slot], sum.value);
    high = RD8(OBJ->f3510[slot]);
    sum = add_bytes(high, RD8(ZP->product.hi), sum.carry);
    WR8(OBJ->f3510[slot], sum.value);
    e = of_sum(sum);
    phase = RD8(OBJ->f3670[slot]);
    phase = (uint8_t)(phase & 0x07);
    index = (uint8_t)(phase << 1);
    e = then(e, ended_with_y(ended_with_carry(ended_load(index), false), index));
    step = RD8(OBJ->f3618[slot]);
    sum = add_bytes(step, rd8((uint16_t)(0xB8E4 + index)), false);
    WR8(OBJ->f3618[slot], sum.value);
    e = then(e, of_sum(sum));
    low = RD8(OBJ->angle_lo[slot]);
    sum = add_bytes(low, rd8((uint16_t)(0xB8E5 + index)), sum.carry);
    WR8(OBJ->angle_lo[slot], sum.value);
    e = then(e, of_sum(sum));
    if (!sum.carry) {
        return e;
    }
    next = INC8(OBJ->f3670[slot]);
    return then(e, ended_nz(next));
}

void abi_ChainSwings(void)
{
    abi_ended(ChainSwings(cpu.x));
}

/*
 * @routine 09EA:72C WalkAndPick
 * @run 70:AF2C
 * WHAT IT DOES: WalkSideways; the walk direction OBJ_3568 becomes the facing turned round; when the player is $80 or more away along X the object enters state 1; then
 * PickNextAction (a tail jump).
 * HOW: WalkSideways, DistanceXToPlayer, PickNextAction.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: as PickNextAction.
 */
Ended WalkAndPick(uint8_t slot)
{
    PlayerDistance distance;
    uint8_t facing;
    Ended e;

    jsr_push(0xAF2F);
    e = WalkSideways(slot);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    facing = (uint8_t)(facing ^ 0x01);
    WR8(OBJ->angle_hi[slot], facing);
    e = then(e, ended_load(facing));
    jsr_push(0xAF3A);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_distance(distance));
    e = then(e, ended_compare(distance.distance, 0x80));
    if (distance.distance >= 0x80) {
        WR8(OBJ->state[slot], 1);
        e = then(e, ended_load(1));
    }
    return then(e, PickNextAction(slot));
}

void abi_WalkAndPick(void)
{
    abi_ended(WalkAndPick(cpu.x));
}

/*
 * @routine 09EA:746 StepShootOnCue
 * @run 70:AF46
 * WHAT IT DOES: Steps the animation list at $AF89; when it ends the object enters state 2; on a new frame with animation step 3 it plays sound $26; then PlayCueOnFrame runs;
 * on frame $0A with OBJ_35C0's low three bits clear it starts a kind $24 object at its place in the first free slot from $30, facing as it does.
 * HOW: StepObjectFrames, SoundCommand, PlayCueOnFrame, FindFreeSlotFrom30, StartObjectAtPlace.
 * Untraced: one byte.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: see the wrapper.
 */
SoundEnd StepShootOnCue(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    FrameStep step;
    uint8_t anim, frame, bits, last, found_slot, placed, facing;
    bool found;
    Ended e;

    step = step_list(slot, 0xAF89, 0xAF51, &y, true);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    r.end = e;
    if (step == FRAME_LIST_ENDED) {
        WR8(OBJ->state[slot], 2);
        r.end = then(e, ended_load(2));
        return r;
    }
    if (step == FRAME_CHANGED) {
        anim = RD8(OBJ->anim_step[slot]);
        e = then(e, ended_compare(anim, 3));
        if (anim == 3) {
            r.before = then(e, ended_load(0x26));
            r.sounded = true;
            r.command = 0x26;
            jsr_push(0xAF61);
            r.call = SoundCommand(0x26, slot, y);
            jsr_pop();
            e = ended_kept();
        }
    }
    jsr_push(0xAF64);
    e = then(e, PlayCueOnFrame(slot));
    jsr_pop();
    frame = RD8(OBJ->frame[slot]);
    e = then(e, ended_compare(frame, 0x0A));
    if (frame != 0x0A) {
        r.end = e;
        return r;
    }
    bits = RD8(OBJ->angle_lo[slot]);
    bits = (uint8_t)(bits & 0x07);
    e = then(e, ended_load(bits));
    if (bits != 0) {
        r.end = e;
        return r;
    }
    jsr_push(0xAF75);
    found_slot = FindFreeSlotFrom30(&last, &found);
    jsr_pop();
    e = then(e, ended_with_carry(ended_with_y(ended_with_a(ended_nz(0), last), found_slot), found));
    if (!found) {
        r.end = e;
        return r;
    }
    jsr_push(0xAF7C);
    placed = StartObjectAtPlace(0x24, slot, found_slot);
    jsr_pop();
    e = then(e, ended_load(placed));
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->facing_left[found_slot], facing);
    r.end = then(e, ended_load(facing));
    return r;
}

void abi_StepShootOnCue(void)
{
    abi_sound_end(StepShootOnCue(cpu.x, cpu.y));
}

/*
 * @routine 09EA:5CB AdvanceByHealth
 * @run 70:ADCB
 * WHAT IT DOES: An object with a state moves to its next phase (OBJ_3618) when its health OBJ_32A8 falls below the limit a table at $AE09 gives for the phase (phase 2 moves on
 * when bit 7 of the health is set): its state becomes the byte a table at $AE0B gives, the phase goes up, the animation and OBJ_2800 are reset, and from phase 2 its OBJ_3148 loses
 * bits 3 to 6.
 * HOW: Table reads (bank 70).
 * Untraced: the phase 2 test (ADD7) and the OBJ_3148 mask (AE00).
 * WAITS: no.
 * Entry: X object. Exit: the last compare, load or AND.
 */
Ended AdvanceByHealth(uint8_t slot)
{
    uint8_t state, phase, health, limit, byte, flags;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        return e;
    }
    phase = RD8(OBJ->f3618[slot]);
    e = then(e, ended_with_y(ended_nz(phase), phase));
    e = then(e, cpy(phase, 2));
    if (phase == 2) {
        health = RD8(OBJ->f32a8[slot]);
        health = (uint8_t)(health & 0x80);
        e = then(e, ended_load(health));
        if (health == 0) {
            return e;
        }
    } else {
        health = RD8(OBJ->f32a8[slot]);
        limit = rd8((uint16_t)(0xAE09 + phase));
        e = then(e, ended_compare(health, limit));
        if (health >= limit) {
            return e;
        }
    }
    byte = rd8((uint16_t)(0xAE0B + phase));
    WR8(OBJ->state[slot], byte);
    e = then(e, ended_load(byte));
    INC8(OBJ->f3618[slot]);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->f2800[slot], 0);
    phase = RD8(OBJ->f3618[slot]);
    e = then(e, ended_compare(phase, 2));
    if (phase < 2) {
        return e;
    }
    flags = RD8(OBJ->f3148[slot]);
    flags = (uint8_t)(flags & 0x87);
    WR8(OBJ->f3148[slot], flags);
    return then(e, ended_load(flags));
}

void abi_AdvanceByHealth(void)
{
    abi_ended(AdvanceByHealth(cpu.x));
}

/*
 * @routine 09EB:14D PickGroundSpeedInWindow
 * @run 70:B14D
 * WHAT IT DOES: When the object is inside the window (InWindowRange), OBJ_2800 takes a byte of a sixteen-byte table at $B15F picked at random.
 * HOW: InWindowRange, NextRandom, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: outside: the window test's flags; inside: A the table byte, Y the pick, C and V from the random.
 */
Ended PickGroundSpeedInWindow(uint8_t slot)
{
    ViewTest view;
    RandomResult random;
    uint8_t pick, byte;
    Ended e;

    jsr_push(0xB150);
    view = InWindowRange(slot);
    jsr_pop();
    e = view.end;
    if (!view.in_view) {
        return e;
    }
    jsr_push(0xB155);
    random = NextRandom();
    jsr_pop();
    e = then(e, ended_of_random(random));
    pick = (uint8_t)(random.value & 0x0F);
    e = then(e, ended_with_y(ended_load(pick), pick));
    byte = rd8((uint16_t)(0xB15F + pick));
    WR8(OBJ->f2800[slot], byte);
    return then(e, ended_load(byte));
}

void abi_PickGroundSpeedInWindow(void)
{
    abi_ended(PickGroundSpeedInWindow(cpu.x));
}

/*
 * @routine 09EB:14A MoveThenPick
 * @run 70:B14A
 * WHAT IT DOES: Moves the object along X, then PickGroundSpeedInWindow.
 * HOW: ApplyVelocityX, then the next routine's bytes.
 * WAITS: no.
 * Entry: X object. Exit: as PickGroundSpeedInWindow.
 */
Ended MoveThenPick(uint8_t slot)
{
    AxisMove move;
    Ended e;

    jsr_push(0xB14D);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    return then(e, PickGroundSpeedInWindow(slot));
}

void abi_MoveThenPick(void)
{
    abi_ended(MoveThenPick(cpu.x));
}

/*
 * @routine 09EB:1C7 SwingAndPick
 * @run 70:B1C7
 * WHAT IT DOES: Sets the object's Y speed to the cosine of its OBJ_3670 count (multiplier $0150), moves it, counts OBJ_3670 up and goes on as PickGroundSpeedInWindow.
 * HOW: ScaleByCosine, ApplyVelocity, a tail jump into PickGroundSpeedInWindow.
 * WAITS: no.
 * Entry: X object. Exit: as PickGroundSpeedInWindow.
 */
Ended SwingAndPick(uint8_t slot)
{
    AxisMove move;
    uint8_t angle, high, low;

    WR8(ZP->temp00, 0x50);
    WR8(ZP->temp01, 1);
    angle = RD8(OBJ->f3670[slot]);
    jsr_push(0xB1D5);
    ScaleByCosine(angle);
    jsr_pop();
    high = RD8(ZP->product.hi);
    WR8(OBJ->vy_hi[slot], high);
    low = RD8(ZP->product.mid);
    WR8(OBJ->vy_lo[slot], low);
    jsr_push(0xB1E2);
    move = ApplyVelocity(slot);
    jsr_pop();
    INC8(OBJ->f3670[slot]);
    (void)move;
    return then(ended_with_y(ended_kept(), 0), PickGroundSpeedInWindow(slot));
}

void abi_SwingAndPick(void)
{
    abi_ended(SwingAndPick(cpu.x));
}

/*
 * @routine 09EC:1BF FallStepAndLeave
 * @run 70:B9BF
 * WHAT IT DOES: Steps the animation list at $B9F2, adds $0010 to the Y speed and moves down; once Y low is $C0 or more the hitbox goes off, the animation resets and the
 * object enters its next state.
 * HOW: StepObjectFrames, ApplyVelocityY.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: the last compare or INC.
 */
Ended FallStepAndLeave(uint8_t slot)
{
    FrameStep step;
    AxisMove move;
    uint8_t y = 0, low, high, y_low, state;
    Sum sum;
    Ended e;

    step = step_list(slot, 0xB9F2, 0xB9CA, &y, true);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    low = RD8(OBJ->vy_lo[slot]);
    sum = add_bytes(low, 0x10, false);
    WR8(OBJ->vy_lo[slot], sum.value);
    e = then(e, of_sum(sum));
    high = RD8(OBJ->vy_hi[slot]);
    sum = add_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = then(e, of_sum(sum));
    jsr_push(0xB9DE);
    move = ApplyVelocityY(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    y_low = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(y_low, 0xC0));
    if (y_low < 0xC0) {
        return e;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    state = INC8(OBJ->state[slot]);
    return then(e, ended_nz(state));
}

void abi_FallStepAndLeave(void)
{
    abi_ended(FallStepAndLeave(cpu.x));
}

/*
 * @routine 093C:082 TouchesPlayerMark
 * @run 70:B882
 * WHAT IT DOES: Says, with C, whether a mark applies to the object: false (C clear) when the object has no state or is in state 4 or the player byte PlayerByte3BD8 is 0;
 * otherwise a table byte decides: with the player byte 2 the table at $A166, else the table at $A146, indexed by the wave counter's low three bits plus $18; a nonzero
 * byte (zero for the first table) says yes (C set).
 * HOW: Compares and a table read (bank 70).
 * Untraced: the player byte 2 arm (B88E to B89A).
 * WAITS: no.
 * Entry: X object. Exit: C the answer; A and N, Z as the last load or compare left them.
 */
ViewTest TouchesPlayerMark(uint8_t slot)
{
    ViewTest r = { false, { 0 } };
    uint8_t state, mode, wave, index, byte;
    Sum sum;
    Ended e;

    state = RD8(OBJ->state[slot]);
    e = ended_load(state);
    if (state == 0) {
        r.end = then(e, ended_with_carry(ended_kept(), false));
        return r;
    }
    e = then(e, ended_compare(state, 4));
    if (state == 4) {
        r.end = then(e, ended_with_carry(ended_kept(), false));
        return r;
    }
    mode = rd8(PlayerByte3BD8);
    e = then(e, ended_load(mode));
    if (mode == 0) {
        r.end = then(e, ended_with_carry(ended_kept(), false));
        return r;
    }
    e = then(e, ended_compare(mode, 2));
    wave = RD8(ZP->wave_counter);
    wave = (uint8_t)(wave & 0x07);
    e = then(e, ended_load(wave));
    sum = add_bytes(wave, 0x18, false);
    e = then(e, of_sum(sum));
    index = sum.value;
    e = then(e, ended_with_y(ended_kept(), index));
    if (mode == 2) {
        byte = rd8((uint16_t)(0xA166 + index));
        e = then(e, ended_load(byte));
        if (byte == 0) {
            r.end = then(e, ended_with_carry(ended_kept(), false));
            return r;
        }
        r.in_view = true;
        r.end = then(e, ended_with_carry(ended_kept(), true));
        return r;
    }
    byte = rd8((uint16_t)(0xA146 + index));
    e = then(e, ended_load(byte));
    r.in_view = byte != 0;
    r.end = then(e, ended_with_carry(ended_kept(), r.in_view));
    return r;
}

void abi_TouchesPlayerMark(void)
{
    abi_ended(TouchesPlayerMark(cpu.x).end);
}

/*
 * @routine 093C:1DD CountBothPairs
 * @run 70:B9DD
 * WHAT IT DOES: Runs CountTimerPairDownOrRepick and CountAnglePairDownOrRepick.
 * HOW: Two calls.
 * WAITS: no.
 * Entry: X object. Exit: as the second, over the first's.
 */
Ended CountBothPairs(uint8_t slot)
{
    Ended e;

    jsr_push(0xB9E0);
    e = CountTimerPairDownOrRepick(slot);
    jsr_pop();
    jsr_push(0xB9E3);
    e = then(e, CountAnglePairDownOrRepick(slot));
    jsr_pop();
    return e;
}

void abi_CountBothPairs(void)
{
    abi_ended(CountBothPairs(cpu.x));
}

/*
 * @routine 09E9:6B0 HealAndClear
 * @run 70:A6B0
 * WHAT IT DOES: An item that heals: the player's health goes up $28 (to at most $5C) and the item is cleared; when the health is already $5C the object instead plays sound $0B
 * and becomes a kind $9F in state 0 with a timer of 3.
 * HOW: A compare, an add, a tail jump into ClearObject.
 * Untraced: the full-health arm (A6B6) and the cap (A6D4).
 * WAITS: no.
 * Entry: X object. Exit: the last compare, add or load.
 */
SoundEnd HealAndClear(uint8_t slot, uint8_t y)
{
    SoundEnd r = { { 0 }, { 0 }, false, 0, { 0 } };
    uint8_t health;
    Sum sum;
    Ended e;

    health = RD8(ZP->player_health);
    e = ended_compare(health, 0x5C);
    if (health == 0x5C) {
        r.before = then(e, ended_load(0x0B));
        r.sounded = true;
        r.command = 0x0B;
        jsr_push(0xA6BB);
        r.call = SoundCommand(0x0B, slot, y);
        jsr_pop();
        WR8(OBJ->kind[slot], 0x9F);
        WR8(OBJ->life_timer[slot], 3);
        WR8(OBJ->state[slot], 0);
        r.end = ended_load(0);
        return r;
    }
    health = RD8(ZP->player_health);
    sum = add_bytes(health, 0x28, false);
    WR8(ZP->player_health, sum.value);
    e = then(e, of_sum(sum));
    e = then(e, ended_compare(sum.value, 0x5C));
    if (sum.value >= 0x5C) {
        WR8(ZP->player_health, 0x5C);
        e = then(e, ended_load(0x5C));
    }
    ClearObject(slot);
    r.end = e;
    return r;
}

void abi_HealAndClear(void)
{
    abi_sound_end(HealAndClear(cpu.x, cpu.y));
}

/*
 * @routine 09EB:6CD StartOnPlayerAnimation
 * @run 70:B6CD
 * WHAT IT DOES: In state 0 loads the object's record and sets its hitbox offsets to $15 and $14; later it takes the place and facing of slot $10 and steps the animation list that
 * a table at $B707 names for slot $10's state (the list index from the table at $B6FF).
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads (bank 70), a tail jump into StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: state 0: A $14 from the store; else as StepObjectFrames.
 */
Ended StartOnPlayerAnimation(uint8_t slot)
{
    FrameStep step;
    uint8_t state, nz, last, placed, facing, lead, row, y;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xB6D5);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        WR8(OBJ->hitbox_dx[slot], 0x15);
        WR8(OBJ->hitbox_dy[slot], 0x14);
        return then(ended_of_record(last, nz, carry), ended_load(0x14));
    }
    jsr_push(0xB6E5);
    placed = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    e = ended_with_y(ended_load(placed), 0x10);
    facing = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], facing);
    e = then(e, ended_load(facing));
    lead = RD8(OBJ->state[0x10]);
    row = rd8((uint16_t)(0xB6FF + lead));
    e = then(e, ended_with_y(ended_load(row), lead));
    y = row;
    e = then(e, ended_with_y(ended_kept(), y));
    WR8(ZP->temp10, rd8((uint16_t)(0xB707 + row)));
    WR8(ZP->temp11, rd8((uint16_t)(0xB708 + row)));
    step = StepObjectFrames(slot, &y);
    return then(e, ended_of_frame_step(step, true, y));
}

void abi_StartOnPlayerAnimation(void)
{
    abi_ended(StartOnPlayerAnimation(cpu.x));
}

/*
 * @routine 09EC:7D8 StartTracker
 * @run 70:BFD8
 * WHAT IT DOES: Loads the object's record, sets its sprite set to 6 and its frame to 2, and notes its Y low byte in OBJ_35C0 and its next kind in OBJ_3618.
 * HOW: LoadObjectRecord and four stores.
 * WAITS: no.
 * Entry: X object. Exit: A the next kind, N and Z from it, Y 11, C from the record.
 */
Ended StartTracker(uint8_t slot)
{
    uint8_t nz, last, low, kind;
    bool carry;
    Ended e;

    jsr_push(0xBFDB);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->sprite_set[slot], 6);
    WR8(OBJ->frame[slot], 2);
    low = RD8(OBJ->y_lo[slot]);
    WR8(OBJ->angle_lo[slot], low);
    kind = RD8(OBJ->next_kind[slot]);
    WR8(OBJ->f3618[slot], kind);
    return ended_then(e, ended_load(kind));
}

void abi_StartTracker(void)
{
    abi_ended(StartTracker(cpu.x));
}

/*
 * @routine 09ED:0EA StartThenStepList
 * @run 71:C0EA
 * WHAT IT DOES: In state 0 loads the object's record and sets its sprite set to 6; every frame it steps the animation list at $C112.
 * HOW: LoadObjectRecord, StepObjectFrames.
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: as the step.
 */
Ended StartThenStepList(uint8_t slot)
{
    FrameStep step;
    uint8_t state, nz, last, y = 0;
    bool carry;
    Ended e = ended_kept();

    state = RD8(OBJ->state[slot]);
    if (state == 0) {
        jsr_push(0xC0F2);
        last = LoadObjectRecord(slot, &nz, &carry);
        jsr_pop();
        WR8(OBJ->sprite_set[slot], 6);
        e = ended_then(ended_of_record(last, nz, carry), ended_load(6));
    }
    step = step_list(slot, 0xC112, 0xC102, &y, true);
    return ended_then(e, ended_of_frame_step(step, step != FRAME_HELD, y));
}

void abi_StartThenStepList(void)
{
    abi_ended(StartThenStepList(cpu.x));
}

/*
 * @routine 09EB:654 FinalSpinAndEnd
 * @run 70:B654
 * WHAT IT DOES: The last object of the stage's final scene: it is held at X $00B0, Y $0098, its speed follows its angle (OBJ_3568, with the length OBJ_3510) and it moves; the angle turns on by
 * OBJ_3618 and the length shrinks by OBJ_35C0; when the length runs out, if this is slot $10, the music plays again, script slot 3 starts, sound $A1 plays, the object becomes kind $21 in state 0,
 * the water line and the scroll limit are set ($0000 and $027E) and slots $11 to $1F are cleared.
 * HOW: SetVelocityFromAngle, ApplyVelocity, 8-bit subtract, CdPlayAgain, StartSoleScriptSlot, SoundCommand, ClearObjectInY.
 * WAITS: yes, on CdPlayAgain's drive (the stack there is the caller's return, this call's two bytes and X pushed (2) and zeros).
 * Entry: X object, Y the sound driver's. Exit: see the wrapper.
 */
void FinalSpinAndEnd(uint8_t slot)
{
    ScaleResult scale;
    AxisMove move;
    TrackPlay track;
    SoleSlot sole;
    CommandCall call;
    Ended e;
    uint8_t angle, length, add, step, sub, to, mode;
    Sum sum;

    WR8(OBJ->x_hi[slot], 0);
    WR8(OBJ->x_lo[slot], 0xB0);
    WR8(OBJ->y_hi[slot], 0);
    WR8(OBJ->y_lo[slot], 0x98);
    WR8(ZP->temp00, 0);
    length = RD8(OBJ->f3510[slot]);
    WR8(ZP->temp01, length);
    angle = RD8(OBJ->angle_hi[slot]);
    jsr_push(0xB671);
    scale = SetVelocityFromAngle(angle, slot);
    jsr_pop();
    jsr_push(0xB674);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_then(ended_with_y(ended_kept(), 0), ended_of_axis_move(move));
    angle = RD8(OBJ->angle_hi[slot]);
    add = RD8(OBJ->f3618[slot]);
    sum = add_bytes(angle, add, false);
    WR8(OBJ->angle_hi[slot], sum.value);
    e = ended_then(e, ended_of_sum(sum));
    length = RD8(OBJ->f3510[slot]);
    sub = RD8(OBJ->angle_lo[slot]);
    sum = sub_bytes(length, sub, true);
    WR8(OBJ->f3510[slot], sum.value);
    e = ended_then(e, ended_of_sum(sum));
    if (sum.carry) {
        sync(&e);
        return;
    }
    WR8(OBJ->f3510[slot], 0);
    e = ended_then(e, cpy(slot, 0x10));
    if (slot != 0x10) {
        sync(&e);
        return;
    }
    sync(&e);
    push8(slot);
    RMW8(ZP->field8_e, old, old & 0xF7);
    RMW8(ZP->field8_e, old, old & 0xEF);
    jsr_push(0xB699);
    track = CdPlayAgain(slot, 0);
    jsr_pop();
    play_track_exit_shim(track);
    jsr_push(0xB69F);
    sole = StartSoleScriptSlot(0, 3);
    jsr_pop();
    abi_ended(ended_of_sole(sole));
    slot = pull8();
    abi_ended(ended_nz(slot));
    jsr_push(0xB6A5);
    call = SoundCommand(0xA1, slot, 0);
    jsr_pop();
    sound_command_exit_shim(0xA1, call);
    WR8(OBJ->kind[slot], 0x21);
    WR8(OBJ->state[slot], 0);
    wr8(LevelYHi, 0);
    wr8(RasterWaterWanted, 0);
    wr8(ScrollXMaxHi, 2);
    wr8(ScrollXMaxLo, 0x7E);
    WR8(OBJ->frame[slot], 1);
    for (to = 0x11; to < 0x20; to++) {
        jsr_push(0xB6C7);
        ClearObjectInY(to);
        jsr_pop();
    }
    mode = 0;
    e = ended_with_y(ended_nz(0), 0x20);
    e = ended_with_carry(ended_with_a(e, 1), true);
    abi_ended(e);
    (void)scale;
    (void)mode;
}

void abi_FinalSpinAndEnd(void)
{
    FinalSpinAndEnd(cpu.x);
}

/*
 * @routine 09EB:752 StepFallingSpark
 * @run 70:B752
 * WHAT IT DOES: Steps the animation list at $B781; when it ends the object is cleared; otherwise it moves, takes $0018 off its Y speed and, from frame $10, turns its hitbox off (the part of the
 * routine at $B725 that comes after its table).
 * HOW: StepObjectFrames, ApplyVelocity, a 16-bit subtract, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: ended: as the step; else the last compare or subtract.
 */
Ended StepFallingSpark(uint8_t slot)
{
    FrameStep step;
    AxisMove move;
    uint8_t y = 0, low, high, frame;
    Sum sum;
    Ended e;

    step = step_list(slot, 0xB781, 0xB75D, &y, true);
    e = ended_of_frame_step(step, step != FRAME_HELD, y);
    if (step == FRAME_LIST_ENDED) {
        ClearObject(slot);
        return e;
    }
    jsr_push(0xB765);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_then(e, ended_of_axis_move(move));
    low = RD8(OBJ->vy_lo[slot]);
    sum = sub_bytes(low, 0x18, true);
    WR8(OBJ->vy_lo[slot], sum.value);
    e = ended_then(e, ended_of_sum(sum));
    high = RD8(OBJ->vy_hi[slot]);
    sum = sub_bytes(high, 0, sum.carry);
    WR8(OBJ->vy_hi[slot], sum.value);
    e = ended_then(e, ended_of_sum(sum));
    frame = RD8(OBJ->frame[slot]);
    e = ended_then(e, ended_compare(frame, 0x10));
    if (frame >= 0x10) {
        WR8(OBJ->hitbox_on[slot], 0);
    }
    return e;
}

void abi_StepFallingSpark(void)
{
    abi_ended(StepFallingSpark(cpu.x));
}

/*
 * @routine 09EB:725 StartFallingSpark09
 * @run 70:B725
 * @stop B74E
 * WHAT IT DOES: In state 0 loads the object's record, sets its hitbox offsets to $E0 and $F8, takes its X speed from a table at $B74E by its facing and a Y speed of 3; in any other state it
 * goes on as StepFallingSpark (the code after the table).
 * HOW: LoadObjectRecord and stores, table reads (bank 70).
 * Untraced: the table (B74E to B751).
 * WAITS: no.
 * Entry: X object, Y kept unless a callee sets it. Exit: state 0: A 3, Y the facing doubled; else as StepFallingSpark.
 */
Ended StartFallingSpark09(uint8_t slot)
{
    uint8_t state, nz, last, facing, index, byte;
    bool carry;
    Ended e;

    state = RD8(OBJ->state[slot]);
    if (state != 0) {
        return StepFallingSpark(slot);
    }
    jsr_push(0xB72D);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->hitbox_dx[slot], 0xE0);
    WR8(OBJ->hitbox_dy[slot], 0xF8);
    facing = RD8(OBJ->facing_left[slot]);
    index = (uint8_t)(facing << 1);
    e = ended_then(e, ended_with_y(ended_with_carry(ended_load(index), (facing & 0x80) != 0), index));
    byte = rd8((uint16_t)(0xB74F + index));
    WR8(OBJ->vx_hi[slot], byte);
    byte = rd8((uint16_t)(0xB74E + index));
    WR8(OBJ->vx_lo[slot], byte);
    WR8(OBJ->vy_hi[slot], 3);
    return ended_then(e, ended_load(3));
}

void abi_StartFallingSpark09(void)
{
    abi_ended(StartFallingSpark09(cpu.x));
}

/*
 * @routine 09EB:211 ShootMoveAndPickSpeed
 * @run 70:B211
 * WHAT IT DOES: MaybeShootAhead, a move in X (ApplyVelocityX), and a tail jump into PickGroundSpeedInWindow.
 * HOW: Two calls and a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as PickGroundSpeedInWindow.
 */
Ended ShootMoveAndPickSpeed(uint8_t slot)
{
    AxisMove move;
    Ended e;

    jsr_push(0xB214);
    e = MaybeShootAhead(slot);
    jsr_pop();
    jsr_push(0xB217);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    return then(e, PickGroundSpeedInWindow(slot));
}

void abi_ShootMoveAndPickSpeed(void)
{
    abi_ended(ShootMoveAndPickSpeed(cpu.x));
}

/*
 * @routine 09EB:539 JumpByStateTableE
 * @run 70:B539
 * WHAT IT DOES: Goes to the code a table at $B54B names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTableE(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xB54B + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xB54C + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTableE(void)
{
    Jump jump = JumpByStateTableE(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EA:630 JumpByStateTableF
 * @run 70:AE30
 * WHAT IT DOES: Goes to the code a table at $AE42 names for the object's state.
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled state.
 */
Jump JumpByStateTableF(uint8_t slot)
{
    Jump jump = { { 0 }, 0, true };
    uint8_t state, index, low, high;

    state = RD8(OBJ->state[slot]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(0xAE42 + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(0xAE43 + index));
    WR8(ZP->temp01, high);
    jump.end = ended_with_carry(ended_with_y(ended_load(high), index), (state & 0x80) != 0);
    jump.target = RD16_LH2(ZP->temp00, ZP->temp01);
    return jump;
}

void abi_JumpByStateTableF(void)
{
    Jump jump = JumpByStateTableF(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09E9:28D HitRowByKindAndHits
 * @run 70:A28D
 * WHAT IT DOES: Puts the byte of a table at $AB9F picked by 16 times OBJ_2EE0 plus the hits taken (OBJ_33B0) into zTemp00.
 * HOW: Four shifts, an add, a table read (bank 70).
 * WAITS: no.
 * Entry: X object. Exit: A the table byte, Y its index, C and V from the add.
 */
Ended HitRowByKindAndHits(uint8_t slot)
{
    uint8_t group, hits, shifted, byte;
    Sum sum;
    Ended e;

    group = RD8(OBJ->fields_2ee0[slot]);
    shifted = (uint8_t)(group << 4);
    e = ended_with_carry(ended_nz(shifted), (group & 0x10) != 0);
    WR8(ZP->temp00, shifted);
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_load(hits));
    sum = add_bytes(hits, RD8(ZP->temp00), false);
    e = then(e, of_sum(sum));
    byte = rd8((uint16_t)(0xAB9F + sum.value));
    WR8(ZP->temp00, byte);
    return then(e, ended_with_y(ended_load(byte), sum.value));
}

void abi_HitRowByKindAndHits(void)
{
    abi_ended(HitRowByKindAndHits(cpu.x));
}
