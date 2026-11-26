/*
 * TEMPORARY: the stage object routines of lot L8 for the overlay at sector 0B4B (enemy kinds not yet identified), by routine. Bodies return the
 * arm they ended on and the values behind it; each routine's exit shim builds how its last instructions left A, Y and the flags (Ended,
 * enemy_objects.h), and the abi_ wrappers show it.
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

/* Appends the steps of one exit chain to another. */
static void chain_append(ExitChain *to, const ExitChain *from)
{
    for (unsigned i = 0; i < from->count; i++) {
        to->step[to->count++] = from->step[i];
    }
}

/* The carry out of the 24-bit position add ApplyVelocityX ended with. */
static bool axis_move_carry(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;

    return (move.before + added) > 0xFFFFFF;
}

/* The arms the routines end on (the routine's own struct). */
enum { ST19_FACE_BITS, ST19_FACE_UNMARKED, ST19_FACE_TURNED };
enum { ST19_HURT_UNMARKED, ST19_HURT_HEALTHY, ST19_HURT_HURT };
enum { ST19_ENTER_NOT_FLAGGED, ST19_ENTER_ALREADY, ST19_ENTER_ENTERED };
enum { ST19_SCRIPTS_STARTED, ST19_SCRIPTS_ENDMARK };
enum { ST19_WAIT_WAITED, ST19_WAIT_ENDED, ST19_WAIT_HELD, ST19_WAIT_NOT_FRAME, ST19_WAIT_BOUNCED };
enum { ST19_LEAP_FALLING, ST19_LEAP_LEAPT, ST19_LEAP_REVERSED };
enum { ST19_FOLLOW_LOADED, ST19_FOLLOW_NO_MODE, ST19_FOLLOW_FOLLOWED };

/*
 * @routine 0B4B:01E LoadRecordInStateZero
 * @run 70:B01E
 * WHAT IT DOES: In state 0 loads the object's record; later nothing.
 * HOW: A test and LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: the state's N and Z, or as LoadObjectRecord (l8_load_zero_exit_shim).
 */
L8Load LoadRecordInStateZero(uint8_t slot, L8Layout lay)
{
    return l8_load_new(slot, l8_at(lay, 0xB026));
}

/* What LoadRecordInStateZero leaves in A, Y and the flags: the state, then the record when it was loaded. */
static Ended l8_load_zero_exit_shim(L8Load r)
{
    return l8_load_ended(r);
}

void abi_LoadRecordInStateZero(void)
{
    abi_ended(l8_load_zero_exit_shim(LoadRecordInStateZero(cpu.x, L8_LOADED)));
}

/*
 * @routine 0B4B:57E FaceThePlayerByStateTable
 * @run 70:B57E
 * WHAT IT DOES: Every eighth frame of OBJ_3618, in the states the table at $B596 marks, the object takes the facing of the player's side.
 * HOW: An AND, a table read by the state, SideOfPlayer, a store.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_face_state_table_exit_shim).
 */
L8St19FaceArm FaceThePlayerByStateTable(uint8_t slot)
{
    L8St19FaceArm r = { 0 };

    r.bits = (uint8_t)(RD8(OBJ->f3618[slot]) & 0x07);
    if (r.bits != 0) {
        r.arm = ST19_FACE_BITS;
        return r;
    }
    r.state = RD8(OBJ->state[slot]);
    r.marked = rd8((uint16_t)(0xB596 + r.state));
    if (r.marked == 0) {
        r.arm = ST19_FACE_UNMARKED;
        return r;
    }
    jsr_push(0xB590);
    r.side = SideOfPlayer(slot);
    jsr_pop();
    r.facing = r.side.sides & 0x01;
    WR8(OBJ->facing_left[slot], r.facing);
    r.arm = ST19_FACE_TURNED;
    return r;
}

/* What FaceThePlayerByStateTable leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_face_state_table_exit_shim(L8St19FaceArm r)
{
    Ended e = ended_load(r.bits);

    if (r.arm == ST19_FACE_BITS) {
        return e;
    }
    e = then(e, ended_with_y(ended_nz(r.state), r.state));
    e = then(e, ended_load(r.marked));
    if (r.arm == ST19_FACE_UNMARKED) {
        return e;
    }
    e = then(e, ended_of_side(r.side));
    return then(e, ended_load(r.facing));
}

void abi_FaceThePlayerByStateTable(void)
{
    abi_ended(l8_face_state_table_exit_shim(FaceThePlayerByStateTable(cpu.x)));
}

/*
 * @routine 0B4B:5AE StateTableHurtFlag
 * @run 70:B5AE
 * WHAT IT DOES: Sets OBJ_2EE0 from the table at $B5D2 by the state; in the states the table at $B5E2 marks, an object that has taken 5 hits goes to state $0E and clears its OBJ_2800 and animation.
 * HOW: Two table reads by the state, a compare, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_state_table_hurt_exit_shim).
 */
L8St19HurtFlagArm StateTableHurtFlag(uint8_t slot)
{
    L8St19HurtFlagArm r = { 0 };

    r.state = RD8(OBJ->state[slot]);
    r.value = rd8((uint16_t)(0xB5D2 + r.state));
    WR8(OBJ->fields_2ee0[slot], r.value);
    r.marked = rd8((uint16_t)(0xB5E2 + r.state));
    if (r.marked == 0) {
        r.arm = ST19_HURT_UNMARKED;
        return r;
    }
    r.hits = RD8(OBJ->hit_taken[slot]);
    if (r.hits < 5) {
        r.arm = ST19_HURT_HEALTHY;
        return r;
    }
    WR8(OBJ->state[slot], 0x0E);
    WR8(OBJ->f2800[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    r.arm = ST19_HURT_HURT;
    return r;
}

/* What StateTableHurtFlag leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_state_table_hurt_exit_shim(L8St19HurtFlagArm r)
{
    Ended e = ended_with_y(ended_nz(r.state), r.state);

    e = then(e, ended_load(r.value));
    e = then(e, ended_load(r.marked));
    if (r.arm == ST19_HURT_UNMARKED) {
        return e;
    }
    e = then(e, ended_compare(r.hits, 5));
    if (r.arm == ST19_HURT_HEALTHY) {
        return e;
    }
    return then(e, ended_load(0x0E));
}

void abi_StateTableHurtFlag(void)
{
    abi_ended(l8_state_table_hurt_exit_shim(StateTableHurtFlag(cpu.x)));
}

/*
 * @routine 0B4B:5F2 EnterState0FWhenFlagged
 * @run 70:B5F2
 * WHAT IT DOES: When bit 7 of OBJ_31A0 is set and the state is not $0F, plays sound $B7, clears the animation and the speed and goes to state $0F with OBJ_2800 0.
 * HOW: Two tests, SoundCommand, stores.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (l8_enter_state0f_exit_shim). The arm past the flag test ran in no script; the draft stops there.
 */
L8St19EnterArm EnterState0FWhenFlagged(uint8_t slot, uint8_t y_entry)
{
    L8St19EnterArm r = { 0 };

    r.flags = RD8(OBJ->status_flags[slot]);
    if ((r.flags & 0x80) == 0) {
        r.arm = ST19_ENTER_NOT_FLAGGED;
        return r;
    }
    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0x0F) {
        r.arm = ST19_ENTER_ALREADY;
        return r;
    }
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
    r.arm = ST19_ENTER_ENTERED;
    return r;
}

/* What EnterState0FWhenFlagged leaves, the sound call's included, from the arm it ended on. */
SoundEnd l8_enter_state0f_exit_shim(L8St19EnterArm r)
{
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_load((uint8_t)(r.flags & 0x80));

    if (r.arm == ST19_ENTER_NOT_FLAGGED) {
        s.end = e;
        return s;
    }
    e = then(e, ended_compare(r.state, 0x0F));
    if (r.arm == ST19_ENTER_ALREADY) {
        s.end = e;
        return s;
    }
    s.sounded = true;
    s.command = 0xB7;
    s.before = then(e, ended_load(0xB7));
    s.call = r.call;
    s.end = ended_load(0x0F);
    return s;
}

void abi_EnterState0FWhenFlagged(void)
{
    abi_sound_end(l8_enter_state0f_exit_shim(EnterState0FWhenFlagged(cpu.x, cpu.y)));
}

/*
 * @routine 0B4B:620 HitBoxByFrameTable
 * @run 70:B620
 * WHAT IT DOES: When OBJ_3250 is set, sets it 2 and takes the hit box (OBJ_3930, OBJ_39E0, OBJ_3988, OBJ_3A38) from a four byte row of the table at $C207 picked by the frame less 1.
 * HOW: A test, a subtract, two shifts, four table reads, stores.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_hitbox_frame_table_exit_shim).
 */
L8St19HitBoxArm HitBoxByFrameTable(uint8_t slot)
{
    uint8_t frame, value;
    L8St19HitBoxArm r = { 0 };

    r.on = RD8(OBJ->hitbox_on[slot]);
    if (r.on == 0) {
        r.set = false;
        return r;
    }
    WR8(OBJ->hitbox_on[slot], 2);
    frame = RD8(OBJ->frame[slot]);
    r.diff = sub_bytes(frame, 1, true);
    r.row = (uint8_t)(r.diff.value << 2);
    value = rd8((uint16_t)(0xC207 + r.row));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(0xC208 + r.row));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(0xC209 + r.row));
    WR8(OBJ->hitbox_half_w[slot], value);
    r.value = rd8((uint16_t)(0xC20A + r.row));
    WR8(OBJ->hitbox_half_h[slot], r.value);
    r.set = true;
    return r;
}

/* What HitBoxByFrameTable leaves in A, Y and the flags, from the arm it ended on. */
Ended l8_hitbox_frame_table_exit_shim(L8St19HitBoxArm r)
{
    Ended e = ended_load(r.on);

    if (!r.set) {
        return e;
    }
    e = then(then(e, ended_load(2)), ended_with_overflow(ended_with_carry(ended_load(r.row), (r.diff.value & 0x40) != 0), r.diff.overflow));
    e = ended_with_y(e, r.row);
    return then(e, ended_load(r.value));
}

void abi_HitBoxByFrameTable(void)
{
    abi_ended(l8_hitbox_frame_table_exit_shim(HitBoxByFrameTable(cpu.x)));
}

/*
 * @routine 0B4B:67E LoadRecordStartPieceAtPlace
 * @run 70:B67E
 * WHAT IT DOES: Loads the record, sets OBJ_34B8 $60, clears slot $11 and starts an object of type $26 there at the object's place.
 * HOW: LoadObjectRecord, ClearObjectInY, a tail jump into StartObjectAtPlace.
 * WAITS: no.
 * Entry: X object. Exit: A the start's last byte, Y $11 (l8_start_piece_exit_shim).
 */
L8St19PieceArm LoadRecordStartPieceAtPlace(uint8_t slot)
{
    L8St19PieceArm r;

    jsr_push(0xB681);
    r.record = l8_record(slot);
    jsr_pop();
    WR8(OBJ->f34b8[slot], 0x60);
    jsr_push(0xB68B);
    ClearObjectInY(0x11);
    jsr_pop();
    r.placed = StartObjectAtPlace(0x26, slot, 0x11);
    return r;
}

/* What LoadRecordStartPieceAtPlace leaves in A, Y and the flags. */
static Ended l8_start_piece_exit_shim(L8St19PieceArm r)
{
    Ended e = l8_record_ended(r.record);

    e = then(e, ended_load(0x60));
    e = then(e, ended_with_y(ended_nz(0x11), 0x11));
    return then(then(e, ended_load(0x26)), ended_load(r.placed));
}

void abi_LoadRecordStartPieceAtPlace(void)
{
    abi_ended(l8_start_piece_exit_shim(LoadRecordStartPieceAtPlace(cpu.x)));
}

/* MusicThenState's result: whether the music was restarted, with the exit before it and after it. */
typedef struct {
    Ended pre;
    bool played;
    TrackPlay track;
    Ended post;
} MusicState;

/* CountDownThenMusicAgain: the timer after the DEC, and when it ran out the track call and the state after the INC. */
typedef struct {
    bool played;
    uint8_t timer, count;
    TrackPlay track;
} L8St19MusicArm;

/*
 * @routine 0B4B:6BD CountDownThenMusicAgain
 * @run 70:B6BD
 * WHAT IT DOES: Counts OBJ_34B8 down; at zero clears bits 3 and 4 of zField8E, plays the track again (CdPlayAgain), sets OBJ_3670 1 and OBJ_34B8 1 and goes to the next state.
 * HOW: A memory decrement, two RMWs of zField8E, CdPlayAgain, stores, an INC.
 * WAITS: no. (CdPlayAgain starts a disc command; it does not wait.)
 * Entry: X object, Y kept for the track call. Exit: as the last instruction that ran (l8_music_again_exit_shim).
 */
static L8St19MusicArm count_down_then_music_again(uint8_t slot, uint8_t y_entry)
{
    L8St19MusicArm r = { 0 };

    r.timer = DEC8(OBJ->f34b8[slot]);
    if (r.timer != 0) {
        r.played = false;
        return r;
    }
    RMW8(ZP->field8e, old, old & 0xF7);
    RMW8(ZP->field8e, old, old & 0xEF);
    jsr_push(0xB6C9);
    r.track = CdPlayAgain(slot, y_entry);
    jsr_pop();
    WR8(OBJ->f3670[slot], 1);
    WR8(OBJ->f34b8[slot], 1);
    r.count = INC8(OBJ->state[slot]);
    r.played = true;
    return r;
}

/* What CountDownThenMusicAgain leaves: the exit before the track call, the call, the exit after it. */
static MusicState l8_music_again_exit_shim(L8St19MusicArm a)
{
    MusicState r;

    r.played = a.played;
    r.post = ended_kept();
    r.pre = ended_nz(a.timer);
    if (!a.played) {
        return r;
    }
    r.track = a.track;
    r.post = then(ended_load(1), ended_nz(a.count));
    return r;
}

void abi_CountDownThenMusicAgain(void)
{
    MusicState r = l8_music_again_exit_shim(count_down_then_music_again(cpu.x, cpu.y));

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
 * Entry: X object. Exit: as the last instruction that ran (l8_start_scripts_exit_shim).
 */
L8St19ScriptsArm StartScriptsFromTable(uint8_t slot)
{
    uint8_t flags;
    L8St19ScriptsArm r = { 0 };

    r.timer = DEC8(OBJ->f34b8[slot]);
    if (r.timer == 0) {
        r.inc = INC8(OBJ->f36c8[slot]);
        r.step = RD8(OBJ->f36c8[slot]);
        r.row = (uint8_t)(r.step << 2);
        r.value = rd8((uint16_t)(0xB72B + r.row));
        WR8(OBJ->f34b8[slot], r.value);
    }
    r.step2 = RD8(OBJ->f36c8[slot]);
    r.row2 = (uint8_t)(r.step2 << 2);
    r.value2 = rd8((uint16_t)(0xB729 + r.row2));
    if (r.value2 == 0xFF) {
        flags = RD8(ZP->field8e);
        flags = (uint8_t)(flags | 0x04);
        WR8(ZP->field8e, flags);
        r.flags = flags;
        WR8(OBJ->frame[slot], 0x2D);
        WR8(OBJ->vx_hi[slot], 1);
        WR8(OBJ->vy_hi[slot], 0xFA);
        WR8(OBJ->hitbox_on[slot], 2);
        WR8(OBJ->f34b8[slot], 0x30);
        r.state_inc = INC8(OBJ->state[slot]);
        r.arm = ST19_SCRIPTS_ENDMARK;
        return r;
    }
    WR8(OBJ->frame[slot], r.value2);
    push8(slot);
    r.script = rd8((uint16_t)(0xB72A + r.row2));
    jsr_push(0xB704);
    StartScriptSlotKind1(r.script, 5);
    jsr_pop();
    slot = pull8();
    r.pulled = slot;
    r.arm = ST19_SCRIPTS_STARTED;
    return r;
}

/* What StartScriptsFromTable leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_start_scripts_exit_shim(L8St19ScriptsArm r)
{
    Ended e = ended_nz(r.timer);

    if (r.timer == 0) {
        e = then(e, ended_nz(r.inc));
        e = then(e, ended_with_y(ended_with_carry(ended_load(r.row), (uint8_t)(r.step << 1) >= 0x80), r.row));
        e = then(e, ended_load(r.value));
    }
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.row2), (uint8_t)(r.step2 << 1) >= 0x80), r.row2));
    e = then(e, ended_compare(r.value2, 0xFF));
    if (r.arm == ST19_SCRIPTS_ENDMARK) {
        e = then(e, ended_load(r.flags));
        e = then(e, ended_load(0x30));
        return then(e, ended_nz(r.state_inc));
    }
    return then(e, ended_with_y(ended_with_a(ended_nz(r.pulled), r.script), r.script));
}

void abi_StartScriptsFromTable(void)
{
    abi_ended(l8_start_scripts_exit_shim(StartScriptsFromTable(cpu.x)));
}

/*
 * @routine 0B4B:77C FallToGroundAt98
 * @run 70:B77C
 * WHAT IT DOES: Probes the terrain below the object (request $22, probe height 4), moves it by its speed and under gravity; at Y low $98 or more it stops there and says yes (C 1), else no (C 0).
 * HOW: ProbeTerrain, ApplyVelocity, ApplyGravity, a compare.
 * WAITS: no.
 * Entry: X object, Y the probe column, C into the probe. Exit: C 1 landed, 0 not; the compare's A, N and Z (l8_fall_ground_exit_shim).
 */
L8St19FallArm FallToGroundAt98(uint8_t slot, uint8_t y_entry, bool carry_in)
{
    L8St19FallArm r = { 0 };

    WR8(ZP->product.mid, 4);
    jsr_push(0xB785);
    r.probe = ProbeTerrain(0x22, slot, y_entry, carry_in);
    jsr_pop();
    r.y = r.probe.column;
    jsr_push(0xB788);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    jsr_push(0xB78B);
    r.gravity = ApplyGravity(slot);
    jsr_pop();
    r.ground = RD8(OBJ->y_lo[slot]);
    if (r.ground >= 0x98) {
        WR8(OBJ->y_lo[slot], 0x98);
        r.yes = true;
    }
    return r;
}

/* What FallToGroundAt98 leaves, the probe's included: the exits in order. */
static ExitChain l8_fall_ground_exit_shim(L8St19FallArm r)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    Ended e;

    chain_ended(&c, ended_load(0x22));
    chain_probe(&c, r.probe);
    e = ended_of_axis_move(r.move);
    if (r.gravity.gained) {
        e = then(e, ended_with_overflow(ended_kept(), r.gravity.overflow));
    }
    e = then(e, ended_compare(r.ground, 0x98));
    if (r.yes) {
        e = then(e, ended_load(0x98));
        e = ended_with_carry(e, true);
    } else {
        e = ended_with_carry(e, false);
    }
    chain_ended(&c, e);
    return c;
}

void abi_FallToGroundAt98(void)
{
    ExitChain c = l8_fall_ground_exit_shim(FallToGroundAt98(cpu.x, cpu.y, (cpu.p & P_C) != 0));

    abi_chain(&c);
}

/*
 * @routine 0B4B:76E FallThenSoundAtGround
 * @run 70:B76E
 * WHAT IT DOES: Falls (FallToGroundAt98); on landing plays sound $4A and goes to the next state.
 * HOW: FallToGroundAt98, SoundCommand, an INC.
 * WAITS: no.
 * Entry: X object, Y the probe column, C into the probe. Exit: as the last instruction that ran (l8_fall_sound_exit_shim).
 */
L8St19FallSoundArm FallThenSoundAtGround(uint8_t slot, uint8_t y_entry, bool carry_in)
{
    L8St19FallSoundArm r = { 0 };

    jsr_push(0xB771);
    r.fall = FallToGroundAt98(slot, y_entry, carry_in);
    jsr_pop();
    if (!r.fall.yes) {
        return r;
    }
    jsr_push(0xB778);
    r.call = SoundCommand(0x4A, slot, r.fall.y);
    jsr_pop();
    r.count = INC8(OBJ->state[slot]);
    return r;
}

/* What FallThenSoundAtGround leaves: the fall's exits, and on landing the sound call and the state's INC. */
static ExitChain l8_fall_sound_exit_shim(L8St19FallSoundArm a)
{
    ExitChain c = l8_fall_ground_exit_shim(a.fall);
    SoundEnd r = l8_sound_none(ended_kept());

    if (!a.fall.yes) {
        return c;
    }
    r.sounded = true;
    r.command = 0x4A;
    r.before = ended_load(0x4A);
    r.call = a.call;
    r.end = ended_nz(a.count);
    chain_sound(&c, r);
    return c;
}

void abi_FallThenSoundAtGround(void)
{
    ExitChain c = l8_fall_sound_exit_shim(FallThenSoundAtGround(cpu.x, cpu.y, (cpu.p & P_C) != 0));

    abi_chain(&c);
}

/*
 * @routine 0B4B:7CC MoveXAndProbeWallBounce
 * @run 70:B7CC
 * WHAT IT DOES: Moves the object along X, probes the walls on the side it moves to (request 2, probe height 4) and, when the probe's attribute bytes at $26F3 and $26F4 for that side show bit 1, reverses the speed and moves again and says yes (C 1); otherwise no (C 0).
 * HOW: ApplyVelocityX, ProbeTerrain, two ROLs, a table read, NegateVelocityX, ApplyVelocityX. The bytes at $26F3 and $26F4 keep their address form until they have names.
 * WAITS: no.
 * Entry: X object, Y the probe column, C into the move. Exit: C 1 bounced, 0 not (l8_wall_bounce_exit_shim). The bounce ran in no script; the draft stops there.
 */
L8St19BounceArm MoveXAndProbeWallBounce(uint8_t slot, uint8_t y_entry)
{
    uint8_t speed, side, bits;
    L8St19BounceArm r = { 0 };

    jsr_push(0xB7CF);
    r.move = ApplyVelocityX(slot);
    jsr_pop();
    WR8(ZP->product.mid, 4);
    jsr_push(0xB7D8);
    r.probe = ProbeTerrain(2, slot, y_entry, axis_move_carry(r.move));
    jsr_pop();
    speed = RD8(OBJ->vx_hi[slot]);
    side = (uint8_t)(((speed >> 7) & 0x01) ^ 0x01);
    r.y = (uint8_t)(side << 1);
    bits = (uint8_t)(rd8((uint16_t)(0x26F3 + r.y)));
    bits = (uint8_t)(bits | rd8((uint16_t)(0x26F4 + r.y)));
    r.bits = bits & 0x02;
    if (r.bits == 0) {
        return r;
    }
    jsr_push(0xB7F0);
    r.negated = NegateVelocityX(slot);
    jsr_pop();
    jsr_push(0xB7F3);
    r.move2 = ApplyVelocityX(slot);
    jsr_pop();
    r.yes = true;
    return r;
}

/* What MoveXAndProbeWallBounce leaves, the probe's included: the exits in order. */
static ExitChain l8_wall_bounce_exit_shim(L8St19BounceArm r)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    Ended e;

    chain_ended(&c, ended_of_axis_move(r.move));
    chain_ended(&c, ended_load(2));
    chain_probe(&c, r.probe);
    e = ended_with_y(ended_with_carry(ended_load(r.y), false), r.y);
    e = then(e, ended_load(r.bits));
    if (!r.yes) {
        chain_ended(&c, ended_with_carry(e, false));
        return c;
    }
    e = then(e, ended_of_sum(r.negated));
    e = then(e, ended_of_axis_move(r.move2));
    chain_ended(&c, ended_with_carry(e, true));
    return c;
}

void abi_MoveXAndProbeWallBounce(void)
{
    ExitChain c = l8_wall_bounce_exit_shim(MoveXAndProbeWallBounce(cpu.x, cpu.y));

    abi_chain(&c);
}

/*
 * @routine 0B4B:7B6 StepListThenWallBounce
 * @run 70:B7B6
 * WHAT IT DOES: Steps the animation list at $B7F8; when the graphic changes it sets the X speed from the table at $B805 by OBJ_3568 and goes on to MoveXAndProbeWallBounce; otherwise says no (C 0).
 * HOW: StepObjectFrames, a table read, a store, a fall into the next routine.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: C 1 bounced, 0 not (l8_step_wall_bounce_exit_shim).
 */
L8St19StepBounceArm StepListThenWallBounce(uint8_t slot, uint8_t y_entry)
{
    L8St19StepBounceArm r = { 0 };

    r.step = l8_list_step(slot, 0xB7F8, 0xB7C1, true, y_entry);
    r.y = r.step.y;
    if (r.step.step != FRAME_CHANGED) {
        return r;
    }
    r.aim = RD8(OBJ->f3568[slot]);
    r.value = rd8((uint16_t)(0xB805 + r.aim));
    WR8(OBJ->vx_hi[slot], r.value);
    r.next = MoveXAndProbeWallBounce(slot, r.aim);
    r.yes = r.next.yes;
    r.y = r.next.y;
    return r;
}

/* What StepListThenWallBounce leaves: the step's exit, and when the graphic changed the speed's load and the bounce's exits. */
static ExitChain l8_step_wall_bounce_exit_shim(L8St19StepBounceArm r)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    ExitChain next;
    Ended e = l8_step_ended(r.step);

    if (r.step.step != FRAME_CHANGED) {
        chain_ended(&c, ended_with_carry(e, false));
        return c;
    }
    chain_ended(&c, e);
    chain_ended(&c, ended_with_y(ended_load(r.value), r.aim));
    next = l8_wall_bounce_exit_shim(r.next);
    chain_append(&c, &next);
    return c;
}

void abi_StepListThenWallBounce(void)
{
    ExitChain c = l8_step_wall_bounce_exit_shim(StepListThenWallBounce(cpu.x, cpu.y));

    abi_chain(&c);
}

/*
 * @routine 0B4B:79B CountDownThenHopTowardPlayer
 * @run 70:B79B
 * WHAT IT DOES: Runs CountDownOrPickNextTimer; when it says no (C 0) runs StepListThenWallBounce, copies the facing to OBJ_3568 and, when the player is within $28 in X, goes to state 6.
 * HOW: Two calls, DistanceXToPlayer, a compare, a store.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_hop_toward_exit_shim).
 */
L8St19HopArm CountDownThenHopTowardPlayer(uint8_t slot, uint8_t y_entry)
{
    L8St19HopArm r = { 0 };

    jsr_push(0xB79E);
    r.timer = CountDownOrPickNextTimer(slot);
    jsr_pop();
    /* The timer ran out (it answered C 1) when the value it read was 0. */
    r.ran_out = r.timer.first == 0;
    if (r.ran_out) {
        return r;
    }
    jsr_push(0xB7A3);
    r.hop = StepListThenWallBounce(slot, y_entry);
    jsr_pop();
    r.facing = RD8(OBJ->facing_left[slot]);
    WR8(OBJ->f3568[slot], r.facing);
    jsr_push(0xB7AC);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    if (r.distance.distance < 0x28) {
        WR8(OBJ->state[slot], 6);
        r.entered = true;
    }
    return r;
}

/* What CountDownThenHopTowardPlayer leaves: the timer's exit, and when it said no the hop's exits and the distance's. */
static ExitChain l8_hop_toward_exit_shim(L8St19HopArm r)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    ExitChain hop;
    Ended e;

    chain_ended(&c, l8_count_down_or_pick_exit_shim(r.timer));
    if (r.ran_out) {
        return c;
    }
    hop = l8_step_wall_bounce_exit_shim(r.hop);
    chain_append(&c, &hop);
    e = ended_load(r.facing);
    e = then(e, ended_of_distance(r.distance));
    e = then(e, ended_compare(r.distance.distance, 0x28));
    if (r.entered) {
        e = then(e, ended_load(6));
    }
    chain_ended(&c, e);
    return c;
}

void abi_CountDownThenHopTowardPlayer(void)
{
    ExitChain c = l8_hop_toward_exit_shim(CountDownThenHopTowardPlayer(cpu.x, cpu.y));

    abi_chain(&c);
}

/*
 * @routine 0B4C:034 CountDownThenHopAwayFromPlayer
 * @run 70:B834
 * WHAT IT DOES: Runs CountDownOrPickNextTimer; when it says no (C 0) runs StepListThenWallBounce, sets OBJ_3568 to the opposite of the facing and, when the player is $40 or more away in X, goes to state 5.
 * HOW: Two calls, DistanceXToPlayer, a compare, stores.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_hop_away_exit_shim).
 */
L8St19HopArm CountDownThenHopAwayFromPlayer(uint8_t slot, uint8_t y_entry)
{
    L8St19HopArm r = { 0 };

    jsr_push(0xB837);
    r.timer = CountDownOrPickNextTimer(slot);
    jsr_pop();
    /* The timer ran out (it answered C 1) when the value it read was 0. */
    r.ran_out = r.timer.first == 0;
    if (r.ran_out) {
        return r;
    }
    jsr_push(0xB83C);
    r.hop = StepListThenWallBounce(slot, y_entry);
    jsr_pop();
    r.facing = RD8(OBJ->facing_left[slot]);
    r.facing = (uint8_t)(r.facing ^ 0x01);
    WR8(OBJ->f3568[slot], r.facing);
    jsr_push(0xB847);
    r.distance = DistanceXToPlayer(slot);
    jsr_pop();
    if (r.distance.distance >= 0x40) {
        WR8(OBJ->state[slot], 5);
        r.entered = true;
    }
    return r;
}

/* What CountDownThenHopAwayFromPlayer leaves: the timer's exit, and when it said no the hop's exits and the distance's. */
static ExitChain l8_hop_away_exit_shim(L8St19HopArm r)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    ExitChain hop;
    Ended e;

    chain_ended(&c, l8_count_down_or_pick_exit_shim(r.timer));
    if (r.ran_out) {
        return c;
    }
    hop = l8_step_wall_bounce_exit_shim(r.hop);
    chain_append(&c, &hop);
    e = ended_load(r.facing);
    e = then(e, ended_of_distance(r.distance));
    e = then(e, ended_compare(r.distance.distance, 0x40));
    if (r.entered) {
        e = then(e, ended_load(5));
    }
    chain_ended(&c, e);
    return c;
}

void abi_CountDownThenHopAwayFromPlayer(void)
{
    ExitChain c = l8_hop_away_exit_shim(CountDownThenHopAwayFromPlayer(cpu.x, cpu.y));

    abi_chain(&c);
}

/*
 * @routine 0B4C:15F StepListFallThenSoundAtGround
 * @run 70:B95F
 * WHAT IT DOES: Steps the animation list at $B97D and falls (FallToGroundAt98); on landing plays sound $4A, sets OBJ_3510 $28 and goes to the next OBJ_2800.
 * HOW: StepObjectFrames, FallToGroundAt98, SoundCommand, stores, an INC.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_step_fall_sound_exit_shim).
 */
L8St19StepFallSoundArm StepListFallThenSoundAtGround(uint8_t slot, uint8_t y_entry)
{
    L8St19StepFallSoundArm r = { 0 };

    r.step = l8_list_step(slot, 0xB97D, 0xB96A, true, y_entry);
    jsr_push(0xB96D);
    r.fall = FallToGroundAt98(slot, r.step.y, r.step.step == FRAME_CHANGED);
    jsr_pop();
    if (!r.fall.yes) {
        return r;
    }
    jsr_push(0xB974);
    r.call = SoundCommand(0x4A, slot, r.fall.y);
    jsr_pop();
    WR8(OBJ->f3510[slot], 0x28);
    r.count = INC8(OBJ->f2800[slot]);
    return r;
}

/* What StepListFallThenSoundAtGround leaves: the step's exit, the fall's exits, and on landing the sound call and the counts. */
static ExitChain l8_step_fall_sound_exit_shim(L8St19StepFallSoundArm a)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    ExitChain fall;
    SoundEnd r = l8_sound_none(ended_kept());

    chain_ended(&c, l8_step_ended(a.step));
    fall = l8_fall_ground_exit_shim(a.fall);
    chain_append(&c, &fall);
    if (!a.fall.yes) {
        return c;
    }
    r.sounded = true;
    r.command = 0x4A;
    r.before = ended_load(0x4A);
    r.call = a.call;
    r.end = then(ended_load(0x28), ended_nz(a.count));
    chain_sound(&c, r);
    return c;
}

void abi_StepListFallThenSoundAtGround(void)
{
    ExitChain c = l8_step_fall_sound_exit_shim(StepListFallThenSoundAtGround(cpu.x, cpu.y));

    abi_chain(&c);
}

/*
 * @routine 0B4C:304 StepListThenTileWriteAndSpawn
 * @run 70:BB04
 * WHAT IT DOES: Steps the animation list at $BB1F; when it ends goes to state 5; when the graphic changes runs SoundAndTileWriteAtFrame24 and SoundAndSpawnFiveAtFrame27.
 * HOW: StepObjectFrames, two calls, the second a tail jump.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_tile_spawn_exit_shim).
 */
L8St19TileSpawnArm StepListThenTileWriteAndSpawn(uint8_t slot, uint8_t y_entry)
{
    L8St19TileSpawnArm r = { 0 };

    r.step = l8_list_step(slot, 0xBB1F, 0xBB0F, true, y_entry);
    if (r.step.step == FRAME_LIST_ENDED) {
        WR8(OBJ->state[slot], 5);
        return r;
    }
    if (r.step.step == FRAME_HELD) {
        return r;
    }
    jsr_push(0xBB16);
    r.tile = SoundAndTileWriteAtFrame24(slot, r.step.y);
    jsr_pop();
    r.five = SoundAndSpawnFiveAtFrame27(slot, r.step.y);
    return r;
}

/* What StepListThenTileWriteAndSpawn leaves: the step's exit, then the state's load or the two routines' exits. */
static ExitChain l8_tile_spawn_exit_shim(L8St19TileSpawnArm r)
{
    ExitChain c = { { { 0 }, 0 }, 0 };

    chain_ended(&c, l8_step_ended(r.step));
    if (r.step.step == FRAME_LIST_ENDED) {
        chain_ended(&c, ended_load(5));
        return c;
    }
    if (r.step.step == FRAME_HELD) {
        return c;
    }
    chain_sound(&c, l8_tile_write_24_exit_shim(r.tile));
    chain_sound(&c, l8_spawn_five_27_exit_shim(r.five));
    return c;
}

void abi_StepListThenTileWriteAndSpawn(void)
{
    ExitChain c = l8_tile_spawn_exit_shim(StepListThenTileWriteAndSpawn(cpu.x, cpu.y));

    abi_chain(&c);
}

/*
 * @routine 0B4C:3B8 WaitThenStepListWithSoundAndBounce
 * @run 70:BBB8
 * WHAT IT DOES: In OBJ_2800 0 goes to state 9 when the player is $58 or more away in X, and on to the next OBJ_2800. Later it steps the animation list at $BC00: when it ends it clears the animation and goes to state 5; at frame $12 it plays sound $2C, sets the X speed from the table at $BC07 by its facing and goes on to MoveXAndProbeWallBounce.
 * HOW: DistanceXToPlayer, StepObjectFrames, SoundCommand, a table read, a tail jump.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_wait_bounce_exit_shim).
 */
L8St19WaitBounceArm WaitThenStepListWithSoundAndBounce(uint8_t slot, uint8_t y_entry)
{
    L8St19WaitBounceArm r = { 0 };

    r.phase = RD8(OBJ->f2800[slot]);
    if (r.phase == 0) {
        jsr_push(0xBBC0);
        r.distance = DistanceXToPlayer(slot);
        jsr_pop();
        if (r.distance.distance >= 0x58) {
            WR8(OBJ->state[slot], 9);
            r.entered = true;
        }
        r.inc = INC8(OBJ->f2800[slot]);
        r.arm = ST19_WAIT_WAITED;
        return r;
    }
    r.step = l8_list_step(slot, 0xBC00, 0xBBD8, true, y_entry);
    if (r.step.step == FRAME_LIST_ENDED) {
        WR8(OBJ->anim_step[slot], 0);
        WR8(OBJ->anim_timer[slot], 0);
        WR8(OBJ->state[slot], 5);
        r.arm = ST19_WAIT_ENDED;
        return r;
    }
    if (r.step.step == FRAME_HELD) {
        r.arm = ST19_WAIT_HELD;
        return r;
    }
    r.frame = RD8(OBJ->frame[slot]);
    if (r.frame != 0x12) {
        r.arm = ST19_WAIT_NOT_FRAME;
        return r;
    }
    jsr_push(0xBBE8);
    r.call = SoundCommand(0x2C, slot, r.step.y);
    jsr_pop();
    r.facing = RD8(OBJ->facing_left[slot]);
    r.value = rd8((uint16_t)(0xBC07 + r.facing));
    WR8(OBJ->vx_hi[slot], r.value);
    r.bounce = MoveXAndProbeWallBounce(slot, r.facing);
    r.arm = ST19_WAIT_BOUNCED;
    return r;
}

/* What WaitThenStepListWithSoundAndBounce leaves: the exits in order, the sound call's and the bounce's included. */
static ExitChain l8_wait_bounce_exit_shim(L8St19WaitBounceArm r)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    ExitChain bounce;
    SoundEnd s = l8_sound_none(ended_kept());
    Ended e = ended_load(r.phase);

    if (r.arm == ST19_WAIT_WAITED) {
        e = then(e, ended_of_distance(r.distance));
        e = then(e, ended_compare(r.distance.distance, 0x58));
        if (r.entered) {
            e = then(e, ended_load(9));
        }
        e = then(e, ended_nz(r.inc));
        chain_ended(&c, e);
        return c;
    }
    e = then(e, l8_step_ended(r.step));
    chain_ended(&c, e);
    if (r.arm == ST19_WAIT_ENDED) {
        chain_ended(&c, ended_load(5));
        return c;
    }
    if (r.arm == ST19_WAIT_HELD) {
        return c;
    }
    e = ended_compare(r.frame, 0x12);
    if (r.arm == ST19_WAIT_NOT_FRAME) {
        chain_ended(&c, e);
        return c;
    }
    s.sounded = true;
    s.command = 0x2C;
    s.before = then(e, ended_load(0x2C));
    s.call = r.call;
    s.end = ended_with_y(ended_load(r.value), r.facing);
    chain_sound(&c, s);
    bounce = l8_wall_bounce_exit_shim(r.bounce);
    chain_append(&c, &bounce);
    return c;
}

void abi_WaitThenStepListWithSoundAndBounce(void)
{
    ExitChain c = l8_wait_bounce_exit_shim(WaitThenStepListWithSoundAndBounce(cpu.x, cpu.y));

    abi_chain(&c);
}

/*
 * @routine 0B4C:46F StepListFallThenCountLandings
 * @run 70:BC6F
 * WHAT IT DOES: Steps the animation list at $BC9B and falls (FallToGroundAt98); on landing clears the animation, goes to state 6, halves OBJ_34B8 and counts OBJ_3778 up; at 3 clears OBJ_34B8.
 * HOW: StepObjectFrames, FallToGroundAt98, an LSR and an INC in memory, a compare.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the last instruction that ran (l8_landings_exit_shim). Clearing OBJ_34B8 ran in no script; the draft stops there.
 */
L8St19LandingsArm StepListFallThenCountLandings(uint8_t slot, uint8_t y_entry)
{
    L8St19LandingsArm r = { 0 };

    r.step = l8_list_step(slot, 0xBC9B, 0xBC7A, true, y_entry);
    jsr_push(0xBC7D);
    r.fall = FallToGroundAt98(slot, r.step.y, r.step.step == FRAME_CHANGED);
    jsr_pop();
    if (!r.fall.yes) {
        return r;
    }
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    WR8(OBJ->state[slot], 6);
    r.shifted = LSR8(OBJ->f34b8[slot]) != 0;
    r.inc = INC8(OBJ->f3778[slot]);
    r.count = RD8(OBJ->f3778[slot]);
    if (r.count >= 3) {
        WR8(OBJ->f34b8[slot], 0);
    }
    return r;
}

/* What StepListFallThenCountLandings leaves: the step's exit, the fall's exits, and on landing the state's load, the shift's carry and the counts. */
static ExitChain l8_landings_exit_shim(L8St19LandingsArm r)
{
    ExitChain c = { { { 0 }, 0 }, 0 };
    ExitChain fall;
    Ended e;

    chain_ended(&c, l8_step_ended(r.step));
    fall = l8_fall_ground_exit_shim(r.fall);
    chain_append(&c, &fall);
    if (!r.fall.yes) {
        return c;
    }
    e = ended_load(6);
    e = then(e, ended_with_carry(ended_kept(), r.shifted));
    e = then(e, ended_nz(r.inc));
    e = then(e, ended_load(r.count));
    e = then(e, ended_compare(r.count, 3));
    chain_ended(&c, e);
    return c;
}

void abi_StepListFallThenCountLandings(void)
{
    ExitChain c = l8_landings_exit_shim(StepListFallThenCountLandings(cpu.x, cpu.y));

    abi_chain(&c);
}

/*
 * @routine 0B4C:755 LoadRecordLeapByNumberThenFall
 * @run 70:BF55
 * WHAT IT DOES: In state 0 loads the record, gives it the X speed from the table at $BF9B by its facing and the Y speed high $E0, moves it once and sets the speed from a four byte row of the table at $BF9D by OBJ_34B8; facing left reverses the X speed. Later it falls (ApplyGravityUnlimited) and moves.
 * HOW: LoadObjectRecord, ApplyVelocity, table reads, NegateVelocityX, ApplyGravityUnlimited.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_leap_by_number_exit_shim). Reversing the speed ran in no script; the draft stops there.
 */
L8St19LeapArm LoadRecordLeapByNumberThenFall(uint8_t slot)
{
    uint8_t value;
    GravityResult gravity;
    L8St19LeapArm r = { 0 };

    r.load = l8_load_new(slot, 0xBF5D);
    if (r.load.state != 0) {
        jsr_push(0xBF98);
        gravity = ApplyGravityUnlimited(slot);
        jsr_pop();
        (void)gravity;
        r.move = ApplyVelocity(slot);
        r.arm = ST19_LEAP_FALLING;
        return r;
    }
    r.facing_index = RD8(OBJ->facing_left[slot]);
    value = rd8((uint16_t)(0xBF9B + r.facing_index));
    WR8(OBJ->vx_hi[slot], value);
    WR8(OBJ->vy_hi[slot], 0xE0);
    jsr_push(0xBF6E);
    r.move = ApplyVelocity(slot);
    jsr_pop();
    r.number = RD8(OBJ->f34b8[slot]);
    r.row = (uint8_t)(r.number << 2);
    value = rd8((uint16_t)(0xBF9E + r.row));
    WR8(OBJ->vx_hi[slot], value);
    value = rd8((uint16_t)(0xBF9D + r.row));
    WR8(OBJ->vx_lo[slot], value);
    value = rd8((uint16_t)(0xBFA0 + r.row));
    WR8(OBJ->vy_hi[slot], value);
    r.value = rd8((uint16_t)(0xBF9F + r.row));
    WR8(OBJ->vy_lo[slot], r.value);
    r.facing = RD8(OBJ->facing_left[slot]);
    if (r.facing == 0) {
        r.arm = ST19_LEAP_LEAPT;
        return r;
    }
    jsr_push(0xBF94);
    r.negated = NegateVelocityX(slot);
    jsr_pop();
    r.arm = ST19_LEAP_REVERSED;
    return r;
}

/* What LoadRecordLeapByNumberThenFall leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_leap_by_number_exit_shim(L8St19LeapArm r)
{
    Ended e = l8_load_ended(r.load);

    if (r.arm == ST19_LEAP_FALLING) {
        return ended_of_axis_move(r.move);
    }
    e = then(e, ended_with_y(ended_nz(r.facing_index), r.facing_index));
    e = then(e, ended_load(0xE0));
    e = then(e, ended_of_axis_move(r.move));
    e = then(e, ended_with_y(ended_with_carry(ended_load(r.row), (uint8_t)(r.number << 1) >= 0x80), r.row));
    e = then(e, ended_load(r.value));
    e = then(e, ended_load(r.facing));
    if (r.arm == ST19_LEAP_LEAPT) {
        return e;
    }
    return then(e, ended_of_sum(r.negated));
}

void abi_LoadRecordLeapByNumberThenFall(void)
{
    abi_ended(l8_leap_by_number_exit_shim(LoadRecordLeapByNumberThenFall(cpu.x)));
}

/*
 * @routine 0B4C:7B1 FollowObject10HitBoxByFrame
 * @run 70:BFB1
 * WHAT IT DOES: Counts OBJ_34B8 up. In state 0 loads the record. Later, when OBJ_3670 of slot $10 is 0 it clears its hit box width and height; otherwise it takes the place and facing of slot $10 and the hit box from a four byte row of the table at $C33B by slot $10's frame; a slot $10 that is not kind $24 clears the object.
 * HOW: LoadObjectRecord, CopyObjectPlaceBack, table reads, stores, a tail jump into ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (l8_follow_hitbox_exit_shim). Clearing the object ran in no script; the draft stops there.
 */
L8St19FollowArm FollowObject10HitBoxByFrame(uint8_t slot)
{
    uint8_t frame, value;
    L8St19FollowArm r = { 0 };

    r.inc = INC8(OBJ->f34b8[slot]);
    r.state = RD8(OBJ->state[slot]);
    if (r.state == 0) {
        r.record = l8_record(slot);
        r.arm = ST19_FOLLOW_LOADED;
        return r;
    }
    r.mode = RD8(OBJ->f3670[0x10]);
    if (r.mode == 0) {
        WR8(OBJ->hitbox_half_w[slot], 0);
        WR8(OBJ->hitbox_half_h[slot], 0);
        r.arm = ST19_FOLLOW_NO_MODE;
        return r;
    }
    jsr_push(0xBFCD);
    r.copied = CopyObjectPlaceBack(0x10, slot);
    jsr_pop();
    frame = RD8(OBJ->frame[0x10]);
    r.diff = sub_bytes(frame, 1, true);
    r.row = (uint8_t)(r.diff.value << 2);
    value = rd8((uint16_t)(0xC33B + r.row));
    WR8(OBJ->hitbox_dx[slot], value);
    value = rd8((uint16_t)(0xC33C + r.row));
    WR8(OBJ->hitbox_dy[slot], value);
    value = rd8((uint16_t)(0xC33D + r.row));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(0xC33E + r.row));
    WR8(OBJ->hitbox_half_h[slot], value);
    r.facing = RD8(OBJ->facing_left[0x10]);
    WR8(OBJ->facing_left[slot], r.facing);
    r.kind = RD8(OBJ->kind[0x10]);
    if (r.kind != 0x24) {
        ClearObject(slot);
    }
    r.arm = ST19_FOLLOW_FOLLOWED;
    return r;
}

/* What FollowObject10HitBoxByFrame leaves in A, Y and the flags, from the arm it ended on. */
static Ended l8_follow_hitbox_exit_shim(L8St19FollowArm r)
{
    Ended e = ended_nz(r.inc);

    e = then(e, ended_load(r.state));
    if (r.arm == ST19_FOLLOW_LOADED) {
        return then(e, l8_record_ended(r.record));
    }
    e = then(e, ended_load(r.mode));
    if (r.arm == ST19_FOLLOW_NO_MODE) {
        return e;
    }
    e = then(then(e, ended_with_y(ended_nz(0x10), 0x10)), ended_load(r.copied));
    e = then(e, ended_with_y(ended_with_overflow(ended_with_carry(ended_load(r.row), (r.diff.value & 0x40) != 0), r.diff.overflow), r.row));
    return then(then(e, ended_load(r.facing)), ended_compare(r.kind, 0x24));
}

void abi_FollowObject10HitBoxByFrame(void)
{
    abi_ended(l8_follow_hitbox_exit_shim(FollowObject10HitBoxByFrame(cpu.x)));
}
