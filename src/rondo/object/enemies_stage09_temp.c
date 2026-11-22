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
