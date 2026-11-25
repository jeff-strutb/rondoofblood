/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0B4B (enemy kinds not yet identified), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/*
 * @routine 0B4B:01E LoadRecordInStateZero
 * @run 70:B01E
 * WHAT IT DOES: In state 0 loads the object's record; later nothing.
 * HOW: A test and LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: the state's N and Z, or as LoadObjectRecord.
 */
Ended LoadRecordInStateZero(uint8_t slot)
{
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xB026, &first);

    return e;
}

void abi_LoadRecordInStateZero(void)
{
    abi_ended(LoadRecordInStateZero(cpu.x));
}

/*
 * @routine 0B4B:57E FaceThePlayerByStateTable
 * @run 70:B57E
 * WHAT IT DOES: Every eighth frame of OBJ_3618, in the states the table at $B596 marks, the object takes the facing of the player's side.
 * HOW: An AND, a table read by the state, SideOfPlayer, a store.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended FaceThePlayerByStateTable(uint8_t slot)
{
    uint8_t bits, y, marked, facing;
    SideResult side;
    Ended e;

    bits = (uint8_t)(RD8(OBJ->f3618[slot]) & 0x07);
    e = ended_load(bits);
    if (bits != 0) {
        return e;
    }
    y = RD8(OBJ->state[slot]);
    e = then(e, ended_with_y(ended_nz(y), y));
    marked = rd8((uint16_t)(0xB596 + y));
    e = then(e, ended_load(marked));
    if (marked == 0) {
        return e;
    }
    jsr_push(0xB590);
    side = SideOfPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_side(side));
    facing = side.sides & 0x01;
    WR8(OBJ->facing_left[slot], facing);
    return then(e, ended_load(facing));
}

void abi_FaceThePlayerByStateTable(void)
{
    abi_ended(FaceThePlayerByStateTable(cpu.x));
}

/*
 * @routine 0B4B:5AE StateTableHurtFlag
 * @run 70:B5AE
 * WHAT IT DOES: Sets OBJ_2EE0 from the table at $B5D2 by the state; in the states the table at $B5E2 marks, an object that has taken 5 hits goes to state $0E and clears its OBJ_2800 and animation.
 * HOW: Two table reads by the state, a compare, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended StateTableHurtFlag(uint8_t slot)
{
    uint8_t y, value, marked, hits;
    Ended e;

    y = RD8(OBJ->state[slot]);
    e = ended_with_y(ended_nz(y), y);
    value = rd8((uint16_t)(0xB5D2 + y));
    WR8(OBJ->fields_2ee0[slot], value);
    e = then(e, ended_load(value));
    marked = rd8((uint16_t)(0xB5E2 + y));
    e = then(e, ended_load(marked));
    if (marked == 0) {
        return e;
    }
    hits = RD8(OBJ->hit_taken[slot]);
    e = then(e, ended_compare(hits, 5));
    if (hits < 5) {
        return e;
    }
    WR8(OBJ->state[slot], 0x0E);
    WR8(OBJ->f2800[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    return then(e, ended_load(0x0E));
}

void abi_StateTableHurtFlag(void)
{
    abi_ended(StateTableHurtFlag(cpu.x));
}

/*
 * @routine 0B4B:5F2 EnterState0FWhenFlagged
 * @run 70:B5F2
 * WHAT IT DOES: When bit 7 of OBJ_31A0 is set and the state is not $0F, plays sound $B7, clears the animation and the speed and goes to state $0F with OBJ_2800 0.
 * HOW: Two tests, SoundCommand, stores.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The arm past the flag test ran in no script; the draft stops there.
 */
SoundEnd EnterState0FWhenFlagged(uint8_t slot, uint8_t y_entry)
{
    uint8_t flags, state;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    flags = RD8(OBJ->status_flags[slot]);
    e = ended_load((uint8_t)(flags & 0x80));
    if ((flags & 0x80) == 0) {
        r.end = e;
        return r;
    }
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_compare(state, 0x0F));
    if (state == 0x0F) {
        r.end = e;
        return r;
    }
    r.sounded = true;
    r.command = 0xB7;
    r.before = then(e, ended_load(0xB7));
    jsr_push(0xB605);
    r.call = SoundCommand(0xB7, slot, y_entry);
    jsr_pop();
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->state[slot], 0x0F);
    WR8(OBJ->f2800[slot], 0);
    r.end = ended_load(0x0F);
    return r;
}

void abi_EnterState0FWhenFlagged(void)
{
    abi_sound_end(EnterState0FWhenFlagged(cpu.x, cpu.y));
}

/*
 * @routine 0B4B:620 HitBoxByFrameTable
 * @run 70:B620
 * WHAT IT DOES: When OBJ_3250 is set, sets it 2 and takes the hit box (OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38) from a four byte row of the table at $C207 picked by the frame less 1.
 * HOW: A test, a subtract, two shifts, four table reads, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended HitBoxByFrameTable(uint8_t slot)
{
    uint8_t on, frame, row, value;
    Sum diff;
    Ended e;

    on = RD8(OBJ->hitbox_on[slot]);
    e = ended_load(on);
    if (on == 0) {
        return e;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    frame = RD8(OBJ->frame[slot]);
    diff = sub_bytes(frame, 1, true);
    row = (uint8_t)(diff.value << 2);
    e = then(then(e, ended_load(2)), ended_with_overflow(ended_with_carry(ended_load(row), (diff.value & 0x40) != 0), diff.overflow));
    e = ended_with_y(e, row);
    value = rd8((uint16_t)(0xC207 + row));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(0xC208 + row));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(0xC209 + row));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(0xC20A + row));
    WR8(OBJ->hitbox_half_h[slot], value);
    return then(e, ended_load(value));
}

void abi_HitBoxByFrameTable(void)
{
    abi_ended(HitBoxByFrameTable(cpu.x));
}

/*
 * @routine 0B4B:67E LoadRecordStartPieceAtPlace
 * @run 70:B67E
 * WHAT IT DOES: Loads the record, sets OBJ_34B8 $60, clears slot $11 and starts an object of type $26 there at the object's place.
 * HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: A the start's last byte, Y $11.
 */
Ended LoadRecordStartPieceAtPlace(uint8_t slot)
{
    uint8_t last, nz, placed;
    bool carry;
    Ended e;

    jsr_push(0xB681);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    e = ended_of_record(last, nz, carry);
    WR8(OBJ->f34b8[slot], 0x60);
    e = then(e, ended_load(0x60));
    jsr_push(0xB68B);
    ClearObjectInY(0x11);
    jsr_pop();
    e = then(e, ended_with_y(ended_nz(0x11), 0x11));
    placed = StartObjectAtPlace(0x26, slot, 0x11);
    return then(then(e, ended_load(0x26)), ended_load(placed));
}

void abi_LoadRecordStartPieceAtPlace(void)
{
    abi_ended(LoadRecordStartPieceAtPlace(cpu.x));
}

/* MusicThenState's result: whether the music was restarted, with the exit before it and after it. */
typedef struct {
    Ended pre;
    bool played;
    TrackPlay track;
    Ended post;
} MusicState;

/*
 * @routine 0B4B:6BD CountDownThenMusicAgain
 * @run 70:B6BD
 * WHAT IT DOES: Counts OBJ_34B8 down; at zero clears bits 3 and 4 of zField8E, plays the track again (CdPlayAgain), sets OBJ_3670 1 and OBJ_34B8 1 and goes to the next state.
 * HOW: A memory decrement, two RMWs of zField8E, CdPlayAgain, stores, an INC.
 * WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
 * Entry: X object, Y kept for the track call. Exit: as the last instruction that ran.
 */
static MusicState count_down_then_music_again(uint8_t slot, uint8_t y_entry)
{
    uint8_t timer;
    MusicState r;

    r.played = false;
    r.post = ended_kept();
    timer = DEC8(OBJ->f34b8[slot]);
    r.pre = ended_nz(timer);
    if (timer != 0) {
        return r;
    }
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    jsr_push(0xB6C9);
    r.track = CdPlayAgain(slot, y_entry);
    jsr_pop();
    r.played = true;
    WR8(OBJ->f3670[slot], 1);
    WR8(OBJ->f34b8[slot], 1);
    r.post = then(ended_load(1), ended_nz(INC8(OBJ->state[slot])));
    return r;
}

void abi_CountDownThenMusicAgain(void)
{
    MusicState r = count_down_then_music_again(cpu.x, cpu.y);

    abi_ended(r.pre);
    if (r.played) {
        play_track_exit_shim(r.track);
        abi_ended(r.post);
    }
}

/*
 * @routine 0B4B:6D7 StartScriptsFromTable
 * @run 70:B6D7
 * WHAT IT DOES: Counts OBJ_34B8 down; at zero steps OBJ_36C8 and takes OBJ_34B8 from the table at $B72B. Each frame it takes the frame and the script from the four byte row of the table at $B729 picked by OBJ_36C8 and starts the script as kind 1 in slot 5; at the marker $FF it sets bit 2 of zField8E, frame $2D, the speed X $0100 and Y -$0600, OBJ_3250 2, OBJ_34B8 $30 and goes to the next state.
 * HOW: A memory decrement, table reads, StartScriptSlotKind1 between a PHX and a PLX, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended StartScriptsFromTable(uint8_t slot)
{
    uint8_t timer, step, y, value, script, flags;
    Ended e;

    timer = DEC8(OBJ->f34b8[slot]);
    e = ended_nz(timer);
    if (timer == 0) {
        e = then(e, ended_nz(INC8(OBJ->f36c8[slot])));
        step = RD8(OBJ->f36c8[slot]);
        y = (uint8_t)(step << 2);
        e = then(e, ended_with_y(ended_with_carry(ended_load(y), (uint8_t)(step << 1) >= 0x80), y));
        value = rd8((uint16_t)(0xB72B + y));
        WR8(OBJ->f34b8[slot], value);
        e = then(e, ended_load(value));
    }
    step = RD8(OBJ->f36c8[slot]);
    y = (uint8_t)(step << 2);
    e = then(e, ended_with_y(ended_with_carry(ended_load(y), (uint8_t)(step << 1) >= 0x80), y));
    value = rd8((uint16_t)(0xB729 + y));
    e = then(e, ended_compare(value, 0xFF));
    if (value == 0xFF) {
        flags = RD8(ZP->field8e);
        flags = (uint8_t)(flags | 0x04);
        WR8(ZP->field8e, flags);
        e = then(e, ended_load(flags));
        WR8(OBJ->frame[slot], 0x2D);
        WR8(OBJ->vx_hi[slot], 1);
        WR8(OBJ->vy_hi[slot], 0xFA);
        WR8(OBJ->hitbox_on[slot], 2);
        WR8(OBJ->f34b8[slot], 0x30);
        e = then(e, ended_load(0x30));
        return then(e, ended_nz(INC8(OBJ->state[slot])));
    }
    WR8(OBJ->frame[slot], value);
    push8(slot);
    script = rd8((uint16_t)(0xB72A + y));
    jsr_push(0xB704);
    StartScriptSlotKind1(script, 5);
    jsr_pop();
    slot = pull8();
    return then(e, ended_with_y(ended_with_a(ended_nz(slot), script), script));
}

void abi_StartScriptsFromTable(void)
{
    abi_ended(StartScriptsFromTable(cpu.x));
}

/* What a routine that ends with C 1 for "yes" and C 0 for "no" leaves: the exits in order, the answer and the Y it ends with. */
typedef struct {
    ExitChain chain;
    bool yes;
    uint8_t y;
} Answer;

/*
 * @routine 0B4B:77C FallToGroundAt98
 * @run 70:B77C
 * WHAT IT DOES: Probes the terrain below the object (request $22, probe height 4), moves it by its speed and under gravity; at Y low $98 or more it stops there and says yes (C 1), else no (C 0).
 * HOW: ProbeTerrain, ApplyVelocity, ApplyGravity, a compare.
 * WAITS: no.
 * Entry: X object, Y the probe column, C into the probe. Exit: C 1 landed, 0 not; the compare's A, N and Z.
 */
static Answer fall_to_ground_at_98(uint8_t slot, uint8_t y_entry, bool carry_in)
{
    Answer a = { { { 0 }, 0 }, false, y_entry };
    Probe probe;
    AxisMove move;
    GravityResult gravity;
    uint8_t ground;
    Ended e;

    WR8(ZP->product.mid, 4);
    chain_ended(&a.chain, ended_load(0x22));
    jsr_push(0xB785);
    probe = ProbeTerrain(0x22, slot, y_entry, carry_in);
    jsr_pop();
    chain_probe(&a.chain, probe);
    a.y = probe.column;
    jsr_push(0xB788);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    jsr_push(0xB78B);
    gravity = ApplyGravity(slot);
    jsr_pop();
    if (gravity.gained) {
        e = then(e, ended_with_overflow(ended_kept(), gravity.overflow));
    }
    ground = RD8(OBJ->y_lo[slot]);
    e = then(e, ended_compare(ground, 0x98));
    if (ground >= 0x98) {
        WR8(OBJ->y_lo[slot], 0x98);
        e = then(e, ended_load(0x98));
        e = ended_with_carry(e, true);
        a.yes = true;
    } else {
        e = ended_with_carry(e, false);
    }
    chain_ended(&a.chain, e);
    return a;
}

ExitChain FallToGroundAt98(uint8_t slot, uint8_t y_entry, bool carry_in)
{
    return fall_to_ground_at_98(slot, y_entry, carry_in).chain;
}

void abi_FallToGroundAt98(void)
{
    ExitChain c = FallToGroundAt98(cpu.x, cpu.y, (cpu.p & P_C) != 0);

    abi_chain(&c);
}

/*
 * @routine 0B4B:76E FallThenSoundAtGround
 * @run 70:B76E
 * WHAT IT DOES: Falls (FallToGroundAt98); on landing plays sound $4A and goes to the next state.
 * HOW: FallToGroundAt98, SoundCommand, an INC.
 * WAITS: no.
 * Entry: X object, Y the probe column, C into the probe. Exit: as the last instruction that ran.
 */
ExitChain FallThenSoundAtGround(uint8_t slot, uint8_t y_entry, bool carry_in)
{
    Answer a;
    SoundEnd r = l8_sound_none(ended_kept());

    jsr_push(0xB771);
    a = fall_to_ground_at_98(slot, y_entry, carry_in);
    jsr_pop();
    if (!a.yes) {
        return a.chain;
    }
    r.sounded = true;
    r.command = 0x4A;
    r.before = ended_load(0x4A);
    jsr_push(0xB778);
    r.call = SoundCommand(0x4A, slot, a.y);
    jsr_pop();
    r.end = ended_nz(INC8(OBJ->state[slot]));
    chain_sound(&a.chain, r);
    return a.chain;
}

void abi_FallThenSoundAtGround(void)
{
    ExitChain c = FallThenSoundAtGround(cpu.x, cpu.y, (cpu.p & P_C) != 0);

    abi_chain(&c);
}

/*
 * @routine 0B4B:7CC MoveXAndProbeWallBounce
 * @run 70:B7CC
 * WHAT IT DOES: Moves the object along X, probes the walls on the side it moves to (request 2, probe height 4) and, when the probe's attribute bytes at $26F3 and $26F4 for that side show bit 1, reverses the speed and moves again and says yes (C 1); otherwise no (C 0).
 * HOW: ApplyVelocityX, ProbeTerrain, two ROLs, a table read, NegateVelocityX, ApplyVelocityX. The bytes at $26F3 and $26F4 keep their address form until they have names.
 * WAITS: no.
 * Entry: X object, Y the probe column, C into the move. Exit: C 1 bounced, 0 not. The bounce ran in no script; the draft stops there.
 */
static Answer move_x_and_probe_wall_bounce(uint8_t slot, uint8_t y_entry)
{
    Answer a = { { { 0 }, 0 }, false, y_entry };
    AxisMove move;
    Probe probe;
    Sum negated;
    uint8_t speed, side, y, bits;
    bool carry;
    Ended e;

    jsr_push(0xB7CF);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = ended_of_axis_move(move);
    chain_ended(&a.chain, e);
    carry = e.c;
    WR8(ZP->product.mid, 4);
    chain_ended(&a.chain, ended_load(2));
    jsr_push(0xB7D8);
    probe = ProbeTerrain(2, slot, y_entry, carry);
    jsr_pop();
    chain_probe(&a.chain, probe);
    carry = probe_carry_out(probe, carry);
    speed = RD8(OBJ->vx_hi[slot]);
    side = (uint8_t)(((speed >> 7) & 0x01) ^ 0x01);
    y = (uint8_t)(side << 1);
    a.y = y;
    e = ended_with_y(ended_with_carry(ended_load(y), false), y);
    bits = (uint8_t)(rd8((uint16_t)(0x26F3 + y)));
    bits = (uint8_t)(bits | rd8((uint16_t)(0x26F4 + y)));
    bits = bits & 0x02;
    e = then(e, ended_load(bits));
    if (bits == 0) {
        chain_ended(&a.chain, ended_with_carry(e, false));
        return a;
    }
    jsr_push(0xB7F0);
    negated = NegateVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_sum(negated));
    jsr_push(0xB7F3);
    move = ApplyVelocityX(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    chain_ended(&a.chain, ended_with_carry(e, true));
    a.yes = true;
    return a;
}

ExitChain MoveXAndProbeWallBounce(uint8_t slot, uint8_t y_entry)
{
    return move_x_and_probe_wall_bounce(slot, y_entry).chain;
}

void abi_MoveXAndProbeWallBounce(void)
{
    ExitChain c = MoveXAndProbeWallBounce(cpu.x, cpu.y);

    abi_chain(&c);
}

/*
 * @routine 0B4B:7B6 StepListThenWallBounce
 * @run 70:B7B6
 * WHAT IT DOES: Steps the animation list at $B7F8; when the graphic changes it sets the X speed from the table at $B805 by OBJ_3568 and goes on to MoveXAndProbeWallBounce; otherwise says no (C 0).
 * HOW: StepObjectFrames, a table read, a store, a fall into the next routine.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: C 1 bounced, 0 not.
 */
static Answer step_list_then_wall_bounce(uint8_t slot, uint8_t y_entry)
{
    Answer a = { { { 0 }, 0 }, false, y_entry };
    uint8_t y = y_entry, aim, value;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB7F8, 0xB7C1, true, &y, &step);
    Answer next;

    if (step != FRAME_CHANGED) {
        chain_ended(&a.chain, ended_with_carry(e, false));
        a.y = y;
        return a;
    }
    chain_ended(&a.chain, e);
    aim = RD8(OBJ->f3568[slot]);
    y = aim;
    value = rd8((uint16_t)(0xB805 + y));
    WR8(OBJ->vx_hi[slot], value);
    chain_ended(&a.chain, ended_with_y(ended_load(value), y));
    next = move_x_and_probe_wall_bounce(slot, y);
    for (unsigned i = 0; i < next.chain.count; i++) {
        a.chain.step[a.chain.count++] = next.chain.step[i];
    }
    a.yes = next.yes;
    a.y = next.y;
    return a;
}

ExitChain StepListThenWallBounce(uint8_t slot, uint8_t y_entry)
{
    return step_list_then_wall_bounce(slot, y_entry).chain;
}

void abi_StepListThenWallBounce(void)
{
    ExitChain c = StepListThenWallBounce(cpu.x, cpu.y);

    abi_chain(&c);
}

/*
 * @routine 0B4B:79B CountDownThenHopTowardPlayer
 * @run 70:B79B
 * WHAT IT DOES: Runs CountDownOrPickNextTimer; when it says no (C 0) runs StepListThenWallBounce, copies the facing to OBJ_3568 and, when the player is within $28 in X, goes to state 6.
 * HOW: Two calls, DistanceXToPlayer, a compare, a store.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
ExitChain CountDownThenHopTowardPlayer(uint8_t slot, uint8_t y_entry)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    Answer a;
    Ended e, ran;
    PlayerDistance distance;
    uint8_t facing;

    jsr_push(0xB79E);
    ran = CountDownOrPickNextTimer(slot);
    jsr_pop();
    chain_ended(&c, ran);
    if (ran.c) {
        return c;
    }
    jsr_push(0xB7A3);
    a = step_list_then_wall_bounce(slot, y_entry);
    jsr_pop();
    for (unsigned i = 0; i < a.chain.count; i++) {
        c.step[c.count++] = a.chain.step[i];
    }
    facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->f3568[slot], facing);
    e = ended_load(facing);
    jsr_push(0xB7AC);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_distance(distance));
    e = then(e, ended_compare(distance.distance, 0x28));
    if (distance.distance < 0x28) {
        WR8(OBJ->state[slot], 6);
        e = then(e, ended_load(6));
    }
    chain_ended(&c, e);
    return c;
}

void abi_CountDownThenHopTowardPlayer(void)
{
    ExitChain c = CountDownThenHopTowardPlayer(cpu.x, cpu.y);

    abi_chain(&c);
}

/* Appends the steps of one exit chain to another. */
static void chain_append(ExitChain *to, const ExitChain *from)
{
    for (unsigned i = 0; i < from->count; i++) {
        to->step[to->count++] = from->step[i];
    }
}

/*
 * @routine 0B4C:034 CountDownThenHopAwayFromPlayer
 * @run 70:B834
 * WHAT IT DOES: Runs CountDownOrPickNextTimer; when it says no (C 0) runs StepListThenWallBounce, sets OBJ_3568 to the opposite of the facing and, when the player is $40 or more away in X, goes to state 5.
 * HOW: Two calls, DistanceXToPlayer, a compare, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
ExitChain CountDownThenHopAwayFromPlayer(uint8_t slot, uint8_t y_entry)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    Answer a;
    Ended e, ran;
    PlayerDistance distance;
    uint8_t facing;

    jsr_push(0xB837);
    ran = CountDownOrPickNextTimer(slot);
    jsr_pop();
    chain_ended(&c, ran);
    if (ran.c) {
        return c;
    }
    jsr_push(0xB83C);
    a = step_list_then_wall_bounce(slot, y_entry);
    jsr_pop();
    chain_append(&c, &a.chain);
    facing = RD8(OBJ->facing_left[slot]);
    facing = (uint8_t)(facing ^ 0x01);
    WR8(OBJ->f3568[slot], facing);
    e = ended_load(facing);
    jsr_push(0xB847);
    distance = DistanceXToPlayer(slot);
    jsr_pop();
    e = then(e, ended_of_distance(distance));
    e = then(e, ended_compare(distance.distance, 0x40));
    if (distance.distance >= 0x40) {
        WR8(OBJ->state[slot], 5);
        e = then(e, ended_load(5));
    }
    chain_ended(&c, e);
    return c;
}

void abi_CountDownThenHopAwayFromPlayer(void)
{
    ExitChain c = CountDownThenHopAwayFromPlayer(cpu.x, cpu.y);

    abi_chain(&c);
}

/*
 * @routine 0B4C:15F StepListFallThenSoundAtGround
 * @run 70:B95F
 * WHAT IT DOES: Steps the animation list at $B97D and falls (FallToGroundAt98); on landing plays sound $4A, sets OBJ_3510 $28 and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, FallToGroundAt98, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
ExitChain StepListFallThenSoundAtGround(uint8_t slot, uint8_t y_entry)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    uint8_t y = y_entry;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xB97D, 0xB96A, true, &y, &step);
    Answer a;
    SoundEnd r = l8_sound_none(ended_kept());

    chain_ended(&c, e);
    jsr_push(0xB96D);
    a = fall_to_ground_at_98(slot, y, step == FRAME_CHANGED);
    jsr_pop();
    chain_append(&c, &a.chain);
    if (!a.yes) {
        return c;
    }
    r.sounded = true;
    r.command = 0x4A;
    r.before = ended_load(0x4A);
    jsr_push(0xB974);
    r.call = SoundCommand(0x4A, slot, a.y);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x28);
    r.end = then(ended_load(0x28), ended_nz(INC8(OBJ->f2800[slot])));
    chain_sound(&c, r);
    return c;
}

void abi_StepListFallThenSoundAtGround(void)
{
    ExitChain c = StepListFallThenSoundAtGround(cpu.x, cpu.y);

    abi_chain(&c);
}

/*
 * @routine 0B4C:304 StepListThenTileWriteAndSpawn
 * @run 70:BB04
 * WHAT IT DOES: Steps the animation list at $BB1F; when it ends goes to state 5; when the graphic changes runs SoundAndTileWriteAtFrame24 and SoundAndSpawnFiveAtFrame27.
 * HOW: StepObjectFrames, two calls, the second a tail jump.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
ExitChain StepListThenTileWriteAndSpawn(uint8_t slot, uint8_t y_entry)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    uint8_t y = y_entry;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xBB1F, 0xBB0F, true, &y, &step);

    chain_ended(&c, e);
    if (step == FRAME_LIST_ENDED) {
        WR8(OBJ->state[slot], 5);
        chain_ended(&c, ended_load(5));
        return c;
    }
    if (step == FRAME_HELD) {
        return c;
    }
    jsr_push(0xBB16);
    chain_sound(&c, SoundAndTileWriteAtFrame24(slot, y));
    jsr_pop();
    chain_sound(&c, SoundAndSpawnFiveAtFrame27(slot, y));
    return c;
}

void abi_StepListThenTileWriteAndSpawn(void)
{
    ExitChain c = StepListThenTileWriteAndSpawn(cpu.x, cpu.y);

    abi_chain(&c);
}

/*
 * @routine 0B4C:3B8 WaitThenStepListWithSoundAndBounce
 * @run 70:BBB8
 * WHAT IT DOES: In OBJ_2800 0 goes to state 9 when the player is $58 or more away in X, and on to the next OBJ_2800. Later it steps the animation list at $BC00: when it ends it clears the animation and goes to state 5; at frame $12 it plays sound $2C, sets the X speed from the table at $BC07 by its facing and goes on to MoveXAndProbeWallBounce.
 * HOW: DistanceXToPlayer, StepObjectFrames, SoundCommand, a table read, a tail jump.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran.
 */
ExitChain WaitThenStepListWithSoundAndBounce(uint8_t slot, uint8_t y_entry)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    uint8_t phase, y = y_entry, frame, facing, value;
    FrameStep step;
    PlayerDistance distance;
    Answer a;
    SoundEnd r = l8_sound_none(ended_kept());
    Ended e;

    phase = RD8(OBJ->f2800[slot]);
    e = ended_load(phase);
    if (phase == 0) {
        jsr_push(0xBBC0);
        distance = DistanceXToPlayer(slot);
        jsr_pop();
        e = then(e, ended_of_distance(distance));
        e = then(e, ended_compare(distance.distance, 0x58));
        if (distance.distance >= 0x58) {
            WR8(OBJ->state[slot], 9);
            e = then(e, ended_load(9));
        }
        e = then(e, ended_nz(INC8(OBJ->f2800[slot])));
        chain_ended(&c, e);
        return c;
    }
    e = then(e, l8_step_list(slot, 0xBC00, 0xBBD8, true, &y, &step));
    chain_ended(&c, e);
    if (step == FRAME_LIST_ENDED) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 5);
        chain_ended(&c, ended_load(5));
        return c;
    }
    if (step == FRAME_HELD) {
        return c;
    }
    frame = RD8(OBJ->frame[slot]);
    e = ended_compare(frame, 0x12);
    if (frame != 0x12) {
        chain_ended(&c, e);
        return c;
    }
    r.sounded = true;
    r.command = 0x2C;
    r.before = then(e, ended_load(0x2C));
    jsr_push(0xBBE8);
    r.call = SoundCommand(0x2C, slot, y);
    jsr_pop();
    facing = RD8(OBJ->facing_left[slot]);
    y = facing;
    value = rd8((uint16_t)(0xBC07 + y));
    WR8(OBJ->vx_hi[slot], value);
    r.end = ended_with_y(ended_load(value), y);
    chain_sound(&c, r);
    a = move_x_and_probe_wall_bounce(slot, y);
    chain_append(&c, &a.chain);
    return c;
}

void abi_WaitThenStepListWithSoundAndBounce(void)
{
    ExitChain c = WaitThenStepListWithSoundAndBounce(cpu.x, cpu.y);

    abi_chain(&c);
}

/*
 * @routine 0B4C:46F StepListFallThenCountLandings
 * @run 70:BC6F
 * WHAT IT DOES: Steps the animation list at $BC9B and falls (FallToGroundAt98); on landing clears the animation, goes to state 6, halves OBJ_34B8 and counts OBJ_3778 up; at 3 clears OBJ_34B8.
 * HOW: StepObjectFrames, FallToGroundAt98, an LSR and an INC in memory, a compare.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran. Clearing OBJ_34B8 ran in no script; the draft stops there.
 */
ExitChain StepListFallThenCountLandings(uint8_t slot, uint8_t y_entry)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    uint8_t y = y_entry, count;
    FrameStep step;
    Ended e = l8_step_list(slot, 0xBC9B, 0xBC7A, true, &y, &step);
    Answer a;
    bool shifted;

    chain_ended(&c, e);
    jsr_push(0xBC7D);
    a = fall_to_ground_at_98(slot, y, step == FRAME_CHANGED);
    jsr_pop();
    chain_append(&c, &a.chain);
    if (!a.yes) {
        return c;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 6);
    e = ended_load(6);
    shifted = LSR8(OBJ->f34b8[slot]) != 0;
    e = then(e, ended_with_carry(ended_kept(), shifted));
    e = then(e, ended_nz(INC8(OBJ->f3778[slot])));
    count = RD8(OBJ->f3778[slot]);
    e = then(e, ended_load(count));
    e = then(e, ended_compare(count, 3));
    if (count >= 3) {
        WR8(OBJ->f34b8[slot], 0);
    }
    chain_ended(&c, e);
    return c;
}

void abi_StepListFallThenCountLandings(void)
{
    ExitChain c = StepListFallThenCountLandings(cpu.x, cpu.y);

    abi_chain(&c);
}

/*
 * @routine 0B4C:755 LoadRecordLeapByNumberThenFall
 * @run 70:BF55
 * WHAT IT DOES: In state 0 loads the record, gives it the X speed from the table at $BF9B by its facing and the Y speed high $E0, moves it once and sets the speed from a four byte row of the table at $BF9D by OBJ_34B8; facing left reverses the X speed. Later it falls (ApplyGravityUnlimited) and moves.
 * HOW: LoadObjectRecord, ApplyVelocity, table reads, NegateVelocityX, ApplyGravityUnlimited.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. Reversing the speed ran in no script; the draft stops there.
 */
Ended LoadRecordLeapByNumberThenFall(uint8_t slot)
{
    uint8_t y, number, value, facing;
    AxisMove move;
    GravityResult gravity;
    Sum negated;
    uint8_t first;
    Ended e = l8_load_if_new(slot, 0xBF5D, &first);

    if (first != 0) {
        jsr_push(0xBF98);
        gravity = ApplyGravityUnlimited(slot);
        jsr_pop();
        (void)gravity;
        move = ApplyVelocity(slot);
        return ended_of_axis_move(move);
    }
    y = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_with_y(ended_nz(y), y));
    value = rd8((uint16_t)(0xBF9B + y));
    WR8(OBJ->vx_hi[slot], value);
    WR8(OBJ->vy_hi[slot], 0xE0);
    e = then(e, ended_load(0xE0));
    jsr_push(0xBF6E);
    move = ApplyVelocity(slot);
    jsr_pop();
    e = then(e, ended_of_axis_move(move));
    number = RD8(OBJ->f34b8[slot]);
    y = (uint8_t)(number << 2);
    e = then(e, ended_with_y(ended_with_carry(ended_load(y), (uint8_t)(number << 1) >= 0x80), y));
    value = rd8((uint16_t)(0xBF9E + y));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xBF9D + y));
    WR8(OBJ->vx_lo[slot], value);
    value = rd8((uint16_t)(0xBFA0 + y));
    WR8(OBJ->vy_hi[slot], value);
    value = rd8((uint16_t)(0xBF9F + y));
    WR8(OBJ->vy_lo[slot], value);
    e = then(e, ended_load(value));
    facing = RD8(OBJ->facing_left[slot]);
    e = then(e, ended_load(facing));
    if (facing == 0) {
        return e;
    }
    jsr_push(0xBF94);
    negated = NegateVelocityX(slot);
    jsr_pop();
    return then(e, ended_of_sum(negated));
}

void abi_LoadRecordLeapByNumberThenFall(void)
{
    abi_ended(LoadRecordLeapByNumberThenFall(cpu.x));
}

/*
 * @routine 0B4C:7B1 FollowObject10HitBoxByFrame
 * @run 70:BFB1
 * WHAT IT DOES: Counts OBJ_34B8 up. In state 0 loads the record. Later, when OBJ_3670 of slot $10 is 0 it clears its hit box width and height; otherwise it takes the place and facing of slot $10 and the hit box from a four byte row of the table at $C33B by slot $10's frame; a slot $10 that is not kind $24 clears the object.
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads, stores, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. Clearing the object ran in no script; the draft stops there.
 */
Ended FollowObject10HitBoxByFrame(uint8_t slot)
{
    uint8_t state, last, nz, mode, frame, row, value, kind, facing;
    bool carry;
    Sum diff;
    Ended e;

    e = ended_nz(INC8(OBJ->f34b8[slot]));
    state = RD8(OBJ->state[slot]);
    e = then(e, ended_load(state));
    if (state == 0) {
        last = LoadObjectRecord(slot, &nz, &carry);
        return then(e, ended_of_record(last, nz, carry));
    }
    mode = RD8(OBJ->f3670[0x10]);
    e = then(e, ended_load(mode));
    if (mode == 0) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        return e;
    }
    jsr_push(0xBFCD);
    value = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    e = then(then(e, ended_with_y(ended_nz(0x10), 0x10)), ended_load(value));
    frame = RD8(OBJ->frame[0x10]);
    diff = sub_bytes(frame, 1, true);
    row = (uint8_t)(diff.value << 2);
    e = then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(row), (diff.value & 0x40) != 0), diff.overflow), row));
    value = rd8((uint16_t)(0xC33B + row));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(0xC33C + row));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(0xC33D + row));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(0xC33E + row));
    WR8(OBJ->hitbox_half_h[slot], value);
    facing = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], facing);
    kind = RD8(OBJ->kind[0x10]);
    e = then(then(e, ended_load(facing)), ended_compare(kind, 0x24));
    if (kind != 0x24) {
        ClearObject(slot);
    }
    return e;
}

void abi_FollowObject10HitBoxByFrame(void)
{
    abi_ended(FollowObject10HitBoxByFrame(cpu.x));
}
