/*
 * Maria's player overlay (01DD, $A000 in bank $6D): its vectors, her start, the spike check while she is pushed,
 * her death, and the moves that stand, crouch and drop her.
 *
 * Most of these are Richter's (richter_state.c) at her addresses, some with her own numbers: her start fills the
 * health to $5C and keeps no PlayerAttackKind clear in the stand; her spike check ends a state $10 through
 * MariaClearSlot5AsKind9; her death's wait holds her at Y $00FF and, while another object is out, sets its timer back
 * to one frame instead of waiting on.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "maria.h"

enum {
    OBJECT_PLAYER = 0, OBJECT_FIRST_OTHER = 1, OBJECT_END = 0x10, OBJECT_SLOT5 = 5,
    LIST_CHANNEL_PLAYER = 7, LIST_KIND_2 = 2, SCRIPT_SLOT_PLAYER = 7,
    START_NEW = 0xA139, START_HEALTH = 0xA13F, START_STAND = 0xA158, START_CLEAR = 0xA143,
    START_FIELD94 = 0x10, HEALTH_FULL = 0x5C, START_LIVE = 1, START_STANDING = 1,
    STATE_STAND = 0, STATE_CROUCH = 2, STATE_FALL = 4, STATE_DYING = 9, STATE_HIT = 9, STATE_10 = 0x10,
    STEP_CROUCH = 3, ANIMATION_STAND = 1, ANIMATION_CROUCH = 0x0E, ANIMATION_FALL = 0x0D,
    FALL_VY_HI = 2, FALL_WORK_D = 4,
    CAMERA_PUSH_IGNORED = 0x18, PUSH_ON_FLOOR = 0x01, HIT_KIND_SPIKE = 6, SPIKE_HIT_TIMER = 0x10, TILE_SPIKE = 0x02,
    SPIKE_COLUMNS = 3,
    DYING_Y_LO = 0xFF, DEATH_SUB_STEP = 5, SCRIPT_DEATH = 9, DEATH_TIMER = 0x40, DYING_HOLD = 1,
    SOUND_DEATH = 0x60, SLOT5_KIND = 9, SOUND_SLOT5 = 0x19, HIT_SCRIPT = 8
};

static AnimThen anim_then(AnimationStep animation, Regs regs)
{
    AnimThen r;

    r.animation = animation;
    r.regs = regs;
    return r;
}

static void anim_then_exit_shim(AnimThen r)
{
    animation_exit_shim(r.animation);
    regs_exit(r.regs);
}

/*
 * The JMP (table,X) of a dispatch: the word it read goes to that routine's C. A target that is none of the
 * dispatcher's own is reached by its run address, with the registers as the jump leaves them.
 */
static void jump_to_routine(uint16_t target, Regs at_jump)
{
    regs_exit(at_jump);
    run_routine(target);
}

/* ------------------------------------------------------------------------------------------------ vectors */

/*
 * @routine 01DD:003 MariaVecStart
 * @run 6D:A003
 * WHAT IT DOES: Her overlay's second vector: goes to MariaStart.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaStart.
 */
void abi_MariaVecStart(void)
{
    abi_MariaStart();
}

/*
 * @routine 01DD:012 MariaVecAnimationSound
 * @run 6D:A012
 * WHAT IT DOES: Goes to MariaAnimationSound.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaAnimationSound.
 */
Regs MariaVecAnimationSound(void)
{
    return MariaAnimationSound();
}

void abi_MariaVecAnimationSound(void)
{
    regs_exit(MariaVecAnimationSound());
}

/*
 * @routine 01DD:018 MariaVecSetAnimation
 * @run 6D:A018
 * WHAT IT DOES: Goes to MariaSetAnimation.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaSetAnimation.
 */
AnimationStep MariaVecSetAnimation(uint8_t animation)
{
    return MariaSetAnimation(animation);
}

void abi_MariaVecSetAnimation(void)
{
    animation_exit_shim(MariaVecSetAnimation(cpu.a));
}

/*
 * @routine 01DD:01B MariaVecAnimationTick
 * @run 6D:A01B
 * WHAT IT DOES: Goes to MariaAnimationTick.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaAnimationTick.
 */
AnimationStep MariaVecAnimationTick(void)
{
    return MariaAnimationTick();
}

void abi_MariaVecAnimationTick(void)
{
    animation_exit_shim(MariaVecAnimationTick());
}

/*
 * @routine 01DD:01E MariaVecObjectApplyVelocity
 * @run 6D:A01E
 * WHAT IT DOES: Goes to MariaObjectApplyVelocity.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaObjectApplyVelocity.
 */
void abi_MariaVecObjectApplyVelocity(void)
{
    abi_MariaObjectApplyVelocity();
}

/*
 * @routine 01DD:027 MariaVecApplyMoveAndPush
 * @run 6D:A027
 * WHAT IT DOES: Goes to MariaApplyMoveAndPush.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as MariaApplyMoveAndPush.
 */
void abi_MariaVecApplyMoveAndPush(void)
{
    abi_MariaApplyMoveAndPush();
}

/* ------------------------------------------------------------------------------------------------ the start */

/*
 * @routine 01DD:12A MariaStart
 * @run 6D:A12A
 * WHAT IT DOES: Starts Maria as zField8A says: a new run, the health, the stand or the clear; each goes on into the
 * next, so all end standing with her animation's list started.
 * HOW: zField8A doubled is the index into MariaStartHandlers; the JMP (table,X) reads the word there.
 * WAITS: no.
 * Entry: Y the caller's (MariaClearWork's block move saves it). Exit: as MariaStartStand. A word in the table that is
 * not one of the four goes to the routine at it with A and X the index, N and Z from it, C bit 7 of zField8A.
 */
typedef struct { bool elsewhere; AnimThen started; } MariaStartEnd;

MariaStartEnd MariaStart(uint8_t y)
{
    uint8_t how = RD8(ZP->field8a);
    uint8_t doubled = (uint8_t)(how << 1);
    uint8_t lo = TBL8(MariaStartHandlers, doubled);
    uint8_t hi = TBL8(MariaStartHandlers, (uint8_t)(doubled + 1));
    uint16_t target = (uint16_t)(lo | (hi << 8));
    MariaStartEnd end = { false, { { 0 }, { 0 } } };

    switch (target) {
    case START_NEW:
        end.started = MariaStartNew(doubled, y);
        break;
    case START_HEALTH:
        end.started = MariaStartHealth(doubled, y);
        break;
    case START_STAND:
        end.started = MariaStartStand();
        break;
    case START_CLEAR:
        end.started = MariaStartClear(doubled, doubled, y);
        break;
    default:
        end.elsewhere = true;
        jump_to_routine(target, r_c(r_ldx(r_lda(regs_kept(), doubled), doubled), (how & 0x80) != 0));
        break;
    }
    return end;
}

void abi_MariaStart(void)
{
    MariaStartEnd end = MariaStart(cpu.y);

    /* Sent elsewhere, that routine's wrapper has left the processor as its code does. */
    if (!end.elsewhere) {
        anim_then_exit_shim(end.started);
    }
}

/*
 * @routine 01DD:139 MariaStartNew
 * @run 6D:A139
 * WHAT IT DOES: Starts a new run: clears zField93 and sets zField94 to $10, then as MariaStartHealth.
 * HOW: Runs on into it.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaStartStand.
 */
AnimThen MariaStartNew(uint8_t x, uint8_t y)
{
    WR8(ZP->field93, 0);
    WR8(ZP->field94, START_FIELD94);
    return MariaStartHealth(x, y);
}

void abi_MariaStartNew(void)
{
    anim_then_exit_shim(MariaStartNew(cpu.x, cpu.y));
}

/*
 * @routine 01DD:13F MariaStartHealth
 * @run 6D:A13F
 * WHAT IT DOES: Fills her health ($5C), then as MariaStartClear.
 * HOW: Runs on into it.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaStartStand.
 */
AnimThen MariaStartHealth(uint8_t x, uint8_t y)
{
    WR8(ZP->player_health, HEALTH_FULL);
    return MariaStartClear(HEALTH_FULL, x, y);
}

void abi_MariaStartHealth(void)
{
    anim_then_exit_shim(MariaStartHealth(cpu.x, cpu.y));
}

/*
 * @routine 01DD:143 MariaStartClear
 * @run 6D:A143
 * WHAT IT DOES: Clears the player's work and her object (keeping her place and facing), makes her hitbox live, sets
 * OBJ_WORK_N and gives her a frame of the hit timer; then as MariaStartStand.
 * HOW: MariaClearWork, MariaClearKeepPlace, three stores of 1, and on into MariaStartStand.
 * WAITS: no.
 * Entry: A, X and Y as they stand (MariaClearWork's block move pushes and pulls them). Exit: as MariaStartStand.
 */
AnimThen MariaStartClear(uint8_t a, uint8_t x, uint8_t y)
{
    jsr_push(0xA146);
    MariaClearWork(a, x, y);
    jsr_pop();
    jsr_push(0xA149);
    MariaClearKeepPlace();
    jsr_pop();
    WR8(OBJ->hitbox_on[OBJECT_PLAYER], START_LIVE);
    WR8(OBJ->work_n[OBJECT_PLAYER], START_LIVE);
    WR8(OBJ->anim_timer[OBJECT_PLAYER], START_LIVE);
    return MariaStartStand();
}

void abi_MariaStartClear(void)
{
    anim_then_exit_shim(MariaStartClear(cpu.a, cpu.x, cpu.y));
}

/*
 * @routine 01DD:158 MariaStartStand
 * @run 6D:A158
 * WHAT IT DOES: Marks the item's first use to come, stands her, and runs on into MariaAnimationSound.
 * HOW: MariaStand, then the sound's instructions.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaAnimationSound; C, and V where it sets it, as MariaSetAnimation left them.
 */
AnimThen MariaStartStand(void)
{
    AnimThen r;

    WR8(PVARS->item_first_use, START_STANDING);
    jsr_push(0xA160);
    r = MariaStand();
    jsr_pop();
    r.regs = r_then(r.regs, MariaAnimationSound());
    return r;
}

void abi_MariaStartStand(void)
{
    anim_then_exit_shim(MariaStartStand());
}

/*
 * @routine 01DD:160 MariaAnimationSound
 * @run 6D:A160
 * WHAT IT DOES: Starts the list that goes with her animation on the player's list channel.
 * HOW: MariaAnimationSounds by OBJ_FRAME, then StartListChannelKind2 with X 7 (the tail JMP through the kernel's
 * vector at $400F).
 * WAITS: no.
 * Entry: nothing. Exit: A 2, N 0, Z 0, X 7, Y the animation. C and V kept.
 */
Regs MariaAnimationSound(void)
{
    uint8_t animation = RD8(OBJ->frame[OBJECT_PLAYER]);
    uint8_t list = TBL8(MariaAnimationSounds, animation);
    uint8_t kind = StartListChannelKind2(list, LIST_CHANNEL_PLAYER);

    return r_lda(r_x_only(r_y_only(regs_kept(), animation), LIST_CHANNEL_PLAYER), kind);
}

void abi_MariaAnimationSound(void)
{
    regs_exit(MariaAnimationSound());
}

/* ------------------------------------------------------------------------------------------------ stand, crouch, fall */

/*
 * @routine 01DE:040 MariaStand
 * @run 6D:A840
 * WHAT IT DOES: Stands her: may not leave the top, state 0, step 0, animation 1, all speed gone.
 * HOW: MariaSetAnimation, then the tail JMP to MariaStopMoving.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaSetAnimation; X and V kept.
 */
AnimThen MariaStand(void)
{
    AnimationStep animation;

    WR8(PVARS->may_leave_top, 0);
    WR8(OBJ->state[OBJECT_PLAYER], STATE_STAND);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    jsr_push(0xA852);
    animation = MariaSetAnimation(ANIMATION_STAND);
    jsr_pop();
    MariaStopMoving();
    return anim_then(animation, regs_kept());
}

void abi_MariaStand(void)
{
    anim_then_exit_shim(MariaStand());
}

/*
 * @routine 01DE:055 MariaCrouch
 * @run 6D:A855
 * WHAT IT DOES: Crouches her: the pad's forced presses off, state 2, step 3, animation $0E, all speed gone.
 * HOW: MariaPadForcesOff, MariaSetAnimation, and on into MariaStopMoving.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaSetAnimation; X and V kept.
 */
AnimThen MariaCrouch(void)
{
    AnimationStep animation;

    jsr_push(0xA858);
    MariaPadForcesOff();
    jsr_pop();
    WR8(OBJ->state[OBJECT_PLAYER], STATE_CROUCH);
    WR8(OBJ->substate[OBJECT_PLAYER], STEP_CROUCH);
    jsr_push(0xA867);
    animation = MariaSetAnimation(ANIMATION_CROUCH);
    jsr_pop();
    MariaStopMoving();
    return anim_then(animation, regs_kept());
}

void abi_MariaCrouch(void)
{
    anim_then_exit_shim(MariaCrouch());
}

/*
 * @routine 01DE:067 MariaStopMoving
 * @run 6D:A867
 * WHAT IT DOES: Stops her along Y and X.
 * HOW: Zeroes the Y speed and runs on into MariaStopX.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void MariaStopMoving(void)
{
    WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], 0);
    MariaStopX();
}

void abi_MariaStopMoving(void)
{
    MariaStopMoving();
}

/*
 * @routine 01DE:06D MariaStopX
 * @run 6D:A86D
 * WHAT IT DOES: Stops her along X.
 * HOW: Zeroes the X speed.
 * WAITS: no.
 * Entry: nothing. Exit: all registers and flags kept.
 */
void MariaStopX(void)
{
    WR8(OBJ->vx_lo[OBJECT_PLAYER], 0);
    WR8(OBJ->vx_hi[OBJECT_PLAYER], 0);
}

void abi_MariaStopX(void)
{
    MariaStopX();
}

/*
 * @routine 01DE:074 MariaToFall
 * @run 6D:A874
 * WHAT IT DOES: Drops her: state 4, step 0, animation $0D, Y speed $0200 down, OBJ_WORK_D 4, no X speed, and the pad's
 * forced presses off.
 * HOW: MariaSetAnimation, the stores, MariaStopX, and on into MariaPadForcesOff.
 * WAITS: no.
 * Entry: nothing. Exit: A 4, N and Z from it; C, and Y and V where it sets them, as MariaSetAnimation left them; X kept.
 */
AnimThen MariaToFall(void)
{
    AnimationStep animation;

    WR8(OBJ->state[OBJECT_PLAYER], STATE_FALL);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    jsr_push(0xA883);
    animation = MariaSetAnimation(ANIMATION_FALL);
    jsr_pop();
    WR8(OBJ->vy_lo[OBJECT_PLAYER], 0);
    WR8(OBJ->vy_hi[OBJECT_PLAYER], FALL_VY_HI);
    WR8(OBJ->work_d[OBJECT_PLAYER], FALL_WORK_D);
    jsr_push(0xA895);
    MariaStopX();
    jsr_pop();
    MariaPadForcesOff();
    return anim_then(animation, r_lda(regs_kept(), FALL_WORK_D));
}

void abi_MariaToFall(void)
{
    anim_then_exit_shim(MariaToFall());
}

/* ------------------------------------------------------------------------------------------------ the spike check */


/*
 * The registers the spike check leaves. The read leaves Y, C and V (and, when the stage's original code answered,
 * everything: the processor holds it). Otherwise Y is the state the LDY loaded.
 */
static void spike_exit_shim(MariaSpike probe)
{
    if (!(probe.on_the_map && probe.read.kind == TILE_BY_ORIGINAL)) {
        cpu.y = probe.state;
    }
    read_above_exit_shim(probe.read, probe.on_the_map);
    cpu.x = probe.column;
    if (probe.found) {
        cpu.a = TILE_SPIKE;
        abi_nz(TILE_SPIKE);
        return;
    }
    cpu.a = 0;
    abi_nz(0);
    abi_flag(P_C, true);
}

/* The Y the spike check leaves, for a call that is given it: worked out from what it read, as its wrapper gives it. */
static uint8_t spike_y(MariaSpike probe)
{
    Cpu saved = cpu;
    uint8_t y;

    spike_exit_shim(probe);
    y = cpu.y;
    cpu = saved;
    return y;
}

/*
 * @routine 01DD:0EF MariaSpikeBelow
 * @run 6D:A0EF
 * WHAT IT DOES: Probes three points at her feet for a tile with bit 1 set (a spike).
 * HOW: Each point's X from MariaColumnOffset pulled one pixel in toward her, its Y from MariaFootYByState by state;
 * the terrain from MariaReadAbove.
 * WAITS: no.
 * Entry: nothing. Exit: Y, C and V as the last read left them (Y the state when the row was off the map). Found, A 2,
 * N and Z from it, X the point. Else A 0, X 3, Z and C set, N clear.
 */
MariaSpike MariaSpikeBelow(void)
{
    MariaSpike probe = { false, 0, 0, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0, { false, 0, false, false, false } } };
    ColumnOffset offset;
    uint8_t across, base, foot;
    Sum column, row;

    for (probe.column = 0; probe.column < SPIKE_COLUMNS; probe.column++) {
        jsr_push(0xA0F3);
        offset = MariaColumnOffset(probe.column);
        jsr_pop();
        across = offset.value;
        if (across != 0) {
            across = (across & 0x80) ? (uint8_t)(across + 1) : (uint8_t)(across - 1);
        }
        base = RD8(OBJ->x_lo[OBJECT_PLAYER]);
        column = add_bytes(across, base, false);
        WR8(ZP->temp00, column.value);
        base = RD8(OBJ->y_lo[OBJECT_PLAYER]);
        probe.state = RD8(OBJ->state[OBJECT_PLAYER]);
        foot = TBL8(MariaFootYByState, probe.state);
        row = add_bytes(base, foot, false);
        probe.on_the_map = row.carry;
        /* The read may hand the tile to the stage's code, which is entered with X the point and Y the state. */
        cpu.x = probe.column;
        cpu.y = probe.state;
        jsr_push(0xA10E);
        probe.read = MariaReadAbove(row.value, row.carry);
        jsr_pop();
        if (tile_answer(probe.read) & TILE_SPIKE) {
            probe.found = true;
            return probe;
        }
    }
    return probe;
}

void abi_MariaSpikeBelow(void)
{
    spike_exit_shim(MariaSpikeBelow());
}

/*
 * @routine 01E0:0B2 MariaClearSlot5AsKind9
 * @run 6D:B8B2
 * WHAT IT DOES: Empties object slot 5 and gives it kind 9, marks the item's first use to come, and plays sound $19.
 * HOW: ClearObject on slot 5, the stores, then the tail JMP to SoundCommand through the kernel's vector at $401B.
 * WAITS: no.
 * Entry: Y the caller's (SoundCommand is given it). Exit: as SoundCommand with $19; X 5 given to it.
 */
CommandCall MariaClearSlot5AsKind9(uint8_t y)
{
    jsr_push(0xB8B7);
    ClearObject(OBJECT_SLOT5);
    jsr_pop();
    WR8(PVARS->item_first_use, START_STANDING);
    WR8(OBJ->kind[OBJECT_SLOT5], SLOT5_KIND);
    return SoundCommand(SOUND_SLOT5, OBJECT_SLOT5, y);
}

void abi_MariaClearSlot5AsKind9(void)
{
    CommandCall sound = MariaClearSlot5AsKind9(cpu.y);

    /* A and X as the tail JMP leaves them: a dropped command keeps them, a taken one leaves the driver's. */
    cpu.a = SOUND_SLOT5;
    cpu.x = OBJECT_SLOT5;
    sound_command_exit_shim(SOUND_SLOT5, sound);
}


/*
 * @routine 01DD:0BE MariaHitWhilePushed
 * @run 6D:A0BE
 * WHAT IT DOES: While she is pushed (bit 0 of zPlayerPush), outside camera mode $18, with no hit timer and not
 * above the map, a spike under her is a hit of kind 6 with timer $10; the state goes back to standing, and a state
 * $10 is ended first by MariaClearSlot5AsKind9.
 * HOW: MariaSpikeBelow, then the hit's fields.
 * WAITS: no.
 * Entry: nothing. Exit: camera mode $18, A $18, Z and C set. Not pushed, A the camera mode, Z set, N and V the push's
 * bits 7 and 6, C from comparing the mode with $18. Held, A OBJ_ANIM_TIMER or PlayerMayLeaveTop, N and Z from it, C as
 * before, V the push's bit 6. No spike, as MariaSpikeBelow. A spike: A the state, N, Z and C from comparing it with
 * $10, X, Y and V as MariaSpikeBelow left them; from state $10, as MariaClearSlot5AsKind9.
 */
MariaPushHit MariaHitWhilePushed(void)
{
    MariaPushHit hit = { PUSH_HIT_CAMERA, 0, 0, 0, 0,
                         { false, 0, 0, false, { TILE_EMPTY_PAGE, 0, 0, 0, 0, 0, { false, 0, false, false, false } } },
                         false, { false, { 0, false, false }, 0, { 0 } } };

    hit.camera = RD8(SCROLLLIM->camera_mode);
    if (hit.camera == CAMERA_PUSH_IGNORED) {
        return hit;
    }
    hit.push = RD8(ZP->player_push);
    hit.how = PUSH_HIT_NOT_PUSHED;
    if (!(hit.push & PUSH_ON_FLOOR)) {
        return hit;
    }
    hit.held = RD8(OBJ->anim_timer[OBJECT_PLAYER]);
    hit.held |= RD8(PVARS->may_leave_top);
    hit.how = PUSH_HIT_HELD;
    if (hit.held != 0) {
        return hit;
    }
    jsr_push(0xA0D5);
    hit.spike = MariaSpikeBelow();
    jsr_pop();
    hit.how = PUSH_HIT_NO_SPIKE;
    if (!hit.spike.found) {
        return hit;
    }
    hit.how = PUSH_HIT_SPIKE;
    WR8(OBJ->hit_taken[OBJECT_PLAYER], HIT_KIND_SPIKE);
    WR8(OBJ->health[OBJECT_PLAYER], SPIKE_HIT_TIMER);
    hit.state = RD8(OBJ->state[OBJECT_PLAYER]);
    if (hit.state == STATE_10) {
        hit.state10 = true;
        jsr_push(0xA0EB);
        hit.sound = MariaClearSlot5AsKind9(spike_y(hit.spike));
        jsr_pop();
    }
    WR8(OBJ->state[OBJECT_PLAYER], STATE_STAND);
    return hit;
}

void maria_push_hit_exit_shim(MariaPushHit hit)
{

    switch (hit.how) {
    case PUSH_HIT_CAMERA:
        cpu.a = CAMERA_PUSH_IGNORED;
        abi_nz(0);
        abi_flag(P_C, true);
        break;
    case PUSH_HIT_NOT_PUSHED:
        cpu.a = hit.camera;
        abi_nz(0);
        abi_flag(P_N, (hit.push & 0x80) != 0);
        abi_flag(P_V, (hit.push & 0x40) != 0);
        abi_flag(P_C, hit.camera >= CAMERA_PUSH_IGNORED);
        break;
    case PUSH_HIT_HELD:
        cpu.a = hit.held;
        abi_nz(hit.held);
        abi_flag(P_V, (hit.push & 0x40) != 0);
        abi_flag(P_C, hit.camera >= CAMERA_PUSH_IGNORED);
        break;
    case PUSH_HIT_NO_SPIKE:
        spike_exit_shim(hit.spike);
        break;
    case PUSH_HIT_SPIKE:
        spike_exit_shim(hit.spike);
        if (hit.state10) {
            cpu.a = SOUND_SLOT5;
            cpu.x = OBJECT_SLOT5;
            sound_command_exit_shim(SOUND_SLOT5, hit.sound);
        } else {
            cpu.a = hit.state;
            abi_nz((uint8_t)(hit.state - STATE_10));
            abi_flag(P_C, hit.state >= STATE_10);
        }
        break;
    }
}

void abi_MariaHitWhilePushed(void)
{
    maria_push_hit_exit_shim(MariaHitWhilePushed());
}

/* ------------------------------------------------------------------------------------------------ her death */




/*
 * @routine 01DD:197 MariaDyingStep
 * @run 6D:A197
 * WHAT IT DOES: The death's wait, from her state frame: holds her still at the top of the map (Y $00FF) with no
 * animation; when PlayerDyingTimer runs out, clears the objects from slot 1 up, and if one is still out sets the
 * timer back to one frame; with all of them gone, starts script 9 and ends the stage's step (zSubStep 5).
 * HOW: OBJ_KIND not zero is an object out. Entered by the state frame's branch.
 * WAITS: no.
 * Entry: nothing. Exit: waiting, A $FF, N and Z from the timer. An object out, A 1, N and Z from it, X its slot, C
 * clear past slot 1 (kept at slot 1). Over, A 5, N and Z from it, X 7, Y 9, C set.
 */
MariaDying MariaDyingStep(void)
{
    MariaDying step = { DYING_WAITING, 0, 0 };

    jsr_push(0xA19A);
    MariaStopX();
    jsr_pop();
    WR8(OBJ->y_lo[OBJECT_PLAYER], DYING_Y_LO);
    WR8(OBJ->y_hi[OBJECT_PLAYER], 0);
    WR8(OBJ->frame[OBJECT_PLAYER], 0);
    step.timer = DEC8(PVARS2->dying_timer);
    if (step.timer != 0) {
        return step;
    }
    for (step.object = OBJECT_FIRST_OTHER; step.object < OBJECT_END; step.object++) {
        if (RD8(OBJ->kind[step.object]) != 0) {
            WR8(PVARS2->dying_timer, DYING_HOLD);
            step.how = DYING_OBJECT_LEFT;
            return step;
        }
        jsr_push(0xA1B4);
        ClearObject(step.object);
        jsr_pop();
    }
    jsr_push(0xA1BE);
    MariaStartHitScript(SCRIPT_DEATH);
    jsr_pop();
    WR8(ZP->sub_step, DEATH_SUB_STEP);
    WR8(PVARS->door_ready, 0);
    WR8(PVARS2->dying, 0);
    step.how = DYING_OVER;
    return step;
}

void maria_dying_exit_shim(MariaDying step)
{

    switch (step.how) {
    case DYING_WAITING:
        cpu.a = DYING_Y_LO;
        abi_nz(step.timer);
        break;
    case DYING_OBJECT_LEFT:
        cpu.a = DYING_HOLD;
        cpu.x = step.object;
        abi_nz(DYING_HOLD);
        if (step.object != OBJECT_FIRST_OTHER) {
            abi_flag(P_C, false);
        }
        break;
    case DYING_OVER:
        hit_script_exit_shim(SCRIPT_DEATH);
        cpu.a = DEATH_SUB_STEP;
        abi_nz(DEATH_SUB_STEP);
        abi_flag(P_C, true);
        break;
    }
}

void abi_MariaDyingStep(void)
{
    maria_dying_exit_shim(MariaDyingStep());
}

/*
 * @routine 01DD:1E1 MariaDeathSound
 * @run 6D:A1E1
 * WHAT IT DOES: Fades the music out and plays the death's sound.
 * HOW: FadeAndStopSound, then the tail JMP to SoundCommand $60 through the kernel's vector at $401B.
 * WAITS: no.
 * Entry: X and Y the caller's (FadeAndStopSound leaves them; SoundCommand is given them). Exit: as SoundCommand
 * with $60.
 */
CommandCall MariaDeathSound(uint8_t x, uint8_t y)
{
    jsr_push(0xA1E4);
    FadeAndStopSound(x, y);
    jsr_pop();
    return SoundCommand(SOUND_DEATH, x, y);
}

void abi_MariaDeathSound(void)
{
    CommandCall sound = MariaDeathSound(cpu.x, cpu.y);

    cpu.a = SOUND_DEATH;
    sound_command_exit_shim(SOUND_DEATH, sound);
}

/*
 * @routine 01DD:1CF MariaDie
 * @run 6D:A1CF
 * WHAT IT DOES: Starts her death: no health, PlayerDying counted up, its timer $40, the dying state (9), no X speed,
 * then the death's sound.
 * HOW: Entered by the state frame's branch when she has fallen off the bottom of the map; MariaStopX, then on into
 * MariaDeathSound.
 * WAITS: no.
 * Entry: X and Y the caller's. Exit: as MariaDeathSound.
 */
CommandCall MariaDie(uint8_t x, uint8_t y)
{
    WR8(ZP->player_health, 0);
    INC8(PVARS2->dying);
    WR8(PVARS2->dying_timer, DEATH_TIMER);
    WR8(OBJ->state[OBJECT_PLAYER], STATE_DYING);
    jsr_push(0xA1E1);
    MariaStopX();
    jsr_pop();
    return MariaDeathSound(x, y);
}

void abi_MariaDie(void)
{
    CommandCall sound = MariaDie(cpu.x, cpu.y);

    cpu.a = SOUND_DEATH;
    sound_command_exit_shim(SOUND_DEATH, sound);
}

/*
 * @routine 01DD:334 MariaHitReact
 * @run 6D:A334
 * WHAT IT DOES: Her reaction to a hit: the pad's forced presses off, the hit timer by the hit's kind (MariaHitTimers),
 * the state and step she was in kept in OBJ_WORK_G and OBJ_WORK_H, state 9 step 0, and the hit's script 8.
 * HOW: The stores, then on into MariaStartHitScript.
 * WAITS: no.
 * Entry: nothing. Exit: as MariaStartHitScript with 8.
 */
void MariaHitReact(void)
{
    uint8_t kind, value;

    jsr_push(0xA337);
    MariaPadForcesOff();
    jsr_pop();
    kind = (uint8_t)(RD8(OBJ->hit_taken[OBJECT_PLAYER]) & 0x07);
    WR8(OBJ->anim_timer[OBJECT_PLAYER], TBL8(MariaHitTimers, kind));
    value = RD8(OBJ->state[OBJECT_PLAYER]);
    WR8(OBJ->work_g[OBJECT_PLAYER], value);
    value = RD8(OBJ->substate[OBJECT_PLAYER]);
    WR8(OBJ->work_h[OBJECT_PLAYER], value);
    WR8(OBJ->state[OBJECT_PLAYER], STATE_HIT);
    WR8(OBJ->substate[OBJECT_PLAYER], 0);
    MariaStartHitScript(HIT_SCRIPT);
}

void abi_MariaHitReact(void)
{
    MariaHitReact();
    hit_script_exit_shim(HIT_SCRIPT);
}
