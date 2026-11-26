/*
 * TEMPORARY: the stage object routines of lot L8 that run the jump tables of lot L6 (overlays 025A, 025B, 0B4B and 0F6B), by routine. Bodies return the
 * values behind what they ended on (and the table's jump); an exit shim per routine builds how the routine's last instructions left A, Y and the flags
 * (Ended, enemy_objects.h) and the abi_ wrappers show it. The registers a table's code is entered with are set by the shims too. The code a table names
 * runs as the original.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_map.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"

static Ended then(Ended e, Ended next)
{
    return ended_then(e, next);
}

/* The arms the routines end on. */
enum { ST27_ROWS_SCROLLING, ST27_ROWS_AT_ZERO };
enum { ST27_C_AIM, ST27_C_NO_FRAME, ST27_C_NO_ENTRY, ST27_C_LISTED };
enum { ST27_D_UNMARKED, ST27_D_MARKED };

/* What the count and the four callees of BossFrameRoutines left before the hit sound. */
static Ended boss_before_hit(const L8St27BossArm *r)
{
    Ended e = ended_nz(r->counted);

    e = then(e, l8_swing_by_cosine_exit_shim(r->swing));
    e = then(e, l8_enter_state0b_exit_shim(r->enter));
    e = then(e, l8_random_script_exit_shim(r->script));
    return then(e, l8_push_player_exit_shim(r->push));
}

/* The Y the hit sound takes: the one a callee left, else the one the routine was entered with. */
static uint8_t boss_y(const L8St27BossArm *r, uint8_t y_entry)
{
    Ended e = boss_before_hit(r);

    return e.y_set ? e.y : y_entry;
}

/*
 * @routine 025B:471 BossFrameRoutines
 * @run 70:B471
 * WHAT IT DOES: One frame of the boss of overlay 025B: counts OBJ_3568 up, then runs SwingByCosineFromState0B, EnterState0BWhenFlagged, StartRandomScriptWhenFlagged, PushPlayerWhenFlagged and HitSoundThenSpawnPieces, and the code the state table names (JumpByStateTable025B545).
 * HOW: An INC, five calls, a call into the table's jump.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the code the table names (boss_exit_shim).
 */
L8St27BossArm BossFrameRoutines(uint8_t slot, uint8_t y_entry)
{
    L8St27BossArm r = { 0 };

    r.counted = INC8(OBJ->f3568[slot]);
    jsr_push(0xB477);
    r.swing = SwingByCosineFromState0B(slot);
    jsr_pop();
    jsr_push(0xB47A);
    r.enter = EnterState0BWhenFlagged(slot);
    jsr_pop();
    jsr_push(0xB47D);
    r.script = StartRandomScriptWhenFlagged(slot);
    jsr_pop();
    jsr_push(0xB480);
    r.push = PushPlayerWhenFlagged(slot);
    jsr_pop();
    jsr_push(0xB483);
    r.hit = HitSoundThenSpawnPieces(slot, boss_y(&r, y_entry));
    jsr_pop();
    abi_sound_end(sound_end_after(boss_before_hit(&r), l8_hit_sound_spawn_exit_shim(r.hit)));
    jsr_push(0xB486);
    r.jump = JumpByStateTable025B545(slot);
    abi_ended(r.jump.end);
    slot = enter_handler(slot, r.jump);
    jsr_pop();
    return r;
}

/* What BossFrameRoutines leaves after the table's code: nothing of its own. */
static Ended boss_exit_shim(L8St27BossArm r)
{
    (void)r;
    return ended_kept();
}

void abi_BossFrameRoutines(void)
{
    abi_ended(boss_exit_shim(BossFrameRoutines(cpu.x, cpu.y)));
}

/*
 * @routine 0F6B:38E HurtSoundsThenRunTable
 * @run 70:B38E
 * WHAT IT DOES: Runs SoundWhenHurtThenFlag and SoundAndState6WhenFlagged and then the code the state table names (JumpByStateTable0F6B3EC).
 * HOW: Three calls.
 * WAITS: no.
 * Entry: X object, Y kept for the sounds. Exit: as the code the table names (l8_hurt_sounds_exit_shim).
 */
L8St27HurtArm HurtSoundsThenRunTable(uint8_t slot, uint8_t y_entry)
{
    L8St27HurtArm r = { 0 };

    jsr_push(0xB390);
    r.hurt = SoundWhenHurtThenFlag(slot, y_entry);
    jsr_pop();
    abi_sound_end(l8_hurt_then_flag_exit_shim(r.hurt));
    jsr_push(0xB392);
    r.state6 = SoundAndState6WhenFlagged(slot, y_entry);
    jsr_pop();
    abi_sound_end(l8_state6_when_flagged_exit_shim(r.state6));
    jsr_push(0xB394);
    r.jump = JumpByStateTable0F6B3EC(slot);
    abi_ended(r.jump.end);
    slot = enter_handler(slot, r.jump);
    jsr_pop();
    return r;
}

/* What HurtSoundsThenRunTable leaves after the table's code: nothing of its own. */
static Ended l8_hurt_sounds_exit_shim(L8St27HurtArm r)
{
    (void)r;
    return ended_kept();
}

void abi_HurtSoundsThenRunTable(void)
{
    abi_ended(l8_hurt_sounds_exit_shim(HurtSoundsThenRunTable(cpu.x, cpu.y)));
}

/* What the count and the two callees of BossFrameRoutinesB left before the list start of state 0F. */
static Ended bossb_before_enter(const L8St27BossBArm *r)
{
    Ended e = ended_nz(r->counted);

    e = then(e, r->face);
    return then(e, r->hurt);
}

/* The Y the state 0F start takes: the one a callee left, else the one the routine was entered with. */
static uint8_t bossb_y(const L8St27BossBArm *r, uint8_t y_entry)
{
    Ended e = bossb_before_enter(r);

    return e.y_set ? e.y : y_entry;
}

/* What the flag, the X and the sums of the list start leave before StartListChannelKind2. */
static Ended bossb_list_regs(const L8St27BossBArm *r)
{
    Ended e = then(ended_load(r->flag), ended_nz(r->x));

    return then(e, then(ended_of_sum(r->diff), ended_of_sum(r->list)));
}

/*
 * @routine 0B4B:556 BossFrameRoutinesB
 * @run 70:B556
 * WHAT IT DOES: One frame of the boss of overlay 0B4B: counts OBJ_3618 up, runs FaceThePlayerByStateTable, StateTableHurtFlag, EnterState0FWhenFlagged, HitBoxByFrameTable and the code the state table names (JumpByStateTable0B4B64C); then, when OBJ_3670 is set, starts the list $83 plus the frame less 1 on channel 5.
 * HOW: An INC, five calls, StartListChannelKind2 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (bossb_exit_shim).
 */
L8St27BossBArm BossFrameRoutinesB(uint8_t slot, uint8_t y_entry)
{
    L8St27BossBArm r = { 0 };

    r.counted = INC8(OBJ->f3618[slot]);
    jsr_push(0xB55C);
    r.face = l8_face_state_table_exit_shim(FaceThePlayerByStateTable(slot));
    jsr_pop();
    jsr_push(0xB55F);
    r.hurt = l8_state_table_hurt_exit_shim(StateTableHurtFlag(slot));
    jsr_pop();
    abi_ended(bossb_before_enter(&r));
    jsr_push(0xB562);
    r.enter = l8_enter_state0f_exit_shim(EnterState0FWhenFlagged(slot, bossb_y(&r, y_entry)));
    jsr_pop();
    abi_sound_end(r.enter);
    jsr_push(0xB565);
    r.box = l8_hitbox_frame_table_exit_shim(HitBoxByFrameTable(slot));
    jsr_pop();
    abi_ended(r.box);
    jsr_push(0xB568);
    r.jump = JumpByStateTable0B4B64C(slot);
    abi_ended(r.jump.end);
    slot = enter_handler(slot, r.jump);
    jsr_pop();
    r.flag = RD8(OBJ->f3670[slot]);
    if (r.flag == 0) {
        return r;
    }
    r.x = slot;
    push8(slot);
    r.frame = RD8(OBJ->frame[slot]);
    r.diff = sub_bytes(r.frame, 1, true);
    r.list = add_bytes(r.diff.value, 0x83, false);
    abi_ended(bossb_list_regs(&r));
    jsr_push(0xB57C);
    StartListChannelKind2(r.list.value, 5);
    jsr_pop();
    r.pulled = pull8();
    r.listed = true;
    return r;
}

/* What BossFrameRoutinesB leaves in A, Y and the flags, from the arm it ended on. */
static Ended bossb_exit_shim(L8St27BossBArm r)
{
    if (!r.listed) {
        return ended_load(r.flag);
    }
    return ended_with_a(ended_nz(r.pulled), 2);
}

void abi_BossFrameRoutinesB(void)
{
    abi_ended(bossb_exit_shim(BossFrameRoutinesB(cpu.x, cpu.y)));
}

/*
 * @routine 025B:09F SpawnRowsWhenScrollAtZero
 * @run 70:B09F
 * WHAT IT DOES: Runs SetBossFixedFields and StepVerticalScrollTable; when the scroll's bytes ScrollXHi+1 and ScrollXLo+1 are both 0 it sets the game's sub-mode 5, counts slot 0's OBJ_3510 down, sets the byte at $3BF0 to 1, clears bit 0 of zField8E, spawns the stage's rows 3 and 4 (SpawnStageObjectsRows34) and goes on to SetTimerThenNextState; otherwise it sets ScrollStepXHi+1 to 1.
 * HOW: Two calls, an ORA, stores, RMWs, SpawnStageObjectsRows34 between a PHX and a PLX, a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (rows_exit_shim). The byte at $3BF0 keeps its address form until it has a name.
 */
L8St27RowsArm SpawnRowsWhenScrollAtZero(uint8_t slot)
{
    L8St27RowsArm r = { 0 };
    StageSpawnEnd spawn;
    uint8_t a;

    jsr_push(0xB0A2);
    r.fixed = SetBossFixedFields();
    jsr_pop();
    jsr_push(0xB0A5);
    r.table = StepVerticalScrollTable(slot);
    jsr_pop();
    WR8(SCROLLE->step_x_hi[1], 0);
    r.high = RD8(SCROLL->x_hi[1]);
    r.low = RD8(SCROLL->x_lo[1]);
    if ((r.high | r.low) != 0) {
        r.counted = INC8(SCROLLE->step_x_hi[1]);
        r.arm = ST27_ROWS_SCROLLING;
        return r;
    }
    WR8(ZP->game_sub_mode, 5);
    r.dec = DEC8(OBJ->f3510[0]);
    wr8(0x3BF0, 1);
    RMW8(ZP->field8e, old, old & 0xFE);
    push8(slot);
    jsr_push(0xB0C2);
    spawn = SpawnStageObjectsRows34();
    jsr_pop();
    r.pulled = pull8();
    slot = r.pulled;
    a = 1;
    r.ran = spawn.ran;
    if (spawn.ran) {
        a = spawn.a;
        r.spawn_a = spawn.a;
        r.spawn_y = spawn.y;
    }
    r.state = SetTimerThenNextState(slot, a);
    r.arm = ST27_ROWS_AT_ZERO;
    return r;
}

/* What SpawnRowsWhenScrollAtZero leaves in A, Y and the flags, from the arm it ended on. */
static Ended rows_exit_shim(L8St27RowsArm r)
{
    Ended e = then(l8_boss_fixed_exit_shim(r.fixed), l8_vertical_scroll_table_exit_shim(r.table));

    e = then(e, ended_load((uint8_t)(r.high | r.low)));
    if (r.arm == ST27_ROWS_SCROLLING) {
        return then(e, ended_nz(r.counted));
    }
    e = then(e, ended_load(5));
    e = then(e, ended_nz(r.dec));
    e = then(e, ended_load(1));
    if (r.ran) {
        e = then(e, ended_with_y(ended_with_a(ended_nz(r.spawn_a), r.spawn_a), r.spawn_y));
    }
    e = then(e, ended_nz(r.pulled));
    return then(e, l8_next_state_exit_shim(r.state));
}

void abi_SpawnRowsWhenScrollAtZero(void)
{
    abi_ended(rows_exit_shim(SpawnRowsWhenScrollAtZero(cpu.x)));
}

/* What the count and the two callees of BossFrameRoutinesC left before the table. */
static Ended bossc_before_jump(const L8St27BossCArm *r)
{
    Ended e = ended_nz(r->counted);

    e = then(e, l8_flag_then_hits_exit_shim(r->flag));
    return then(e, l8_enter_state5_exit_shim(r->enter));
}

/* What BossFrameRoutinesC left after the table's code, as far as the arm it ended on (the list start's arm: through the table entry). */
static Ended bossc_tail(const L8St27BossCArm *r)
{
    Ended e = ended_load(r->aim);

    if (r->arm == ST27_C_AIM) {
        return e;
    }
    e = then(e, ended_load(r->frame));
    if (r->arm == ST27_C_NO_FRAME) {
        return e;
    }
    e = then(then(e, ended_of_sum(r->diff)), ended_with_y(ended_nz(r->y), r->y));
    return then(e, ended_load(r->entry));
}

/*
 * @routine 05CB:13F BossFrameRoutinesC
 * @run 70:B13F
 * WHAT IT DOES: One frame of the boss of overlay 05CB: counts OBJ_34B8 up, runs FlagThenHitsCompare, EnterState5WhenFlagged and the code the state table names (JumpByStateTable05CB1CF); then, when OBJ_35C0 and the frame are not 0, starts the list that the table at $B169 names for the frame less 1 (plus $81) on channel 5.
 * HOW: An INC, three calls, a table read, StartListChannelKind2 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran (bossc_exit_shim).
 */
L8St27BossCArm BossFrameRoutinesC(uint8_t slot, L8Layout lay)
{
    L8St27BossCArm r = { 0 };

    r.counted = INC8(OBJ->f34b8[slot]);
    jsr_push(l8_at(lay, 0xB145));
    r.flag = FlagThenHitsCompare(slot);
    jsr_pop();
    jsr_push(l8_at(lay, 0xB148));
    r.enter = EnterState5WhenFlagged(slot);
    jsr_pop();
    abi_ended(bossc_before_jump(&r));
    jsr_push(l8_at(lay, 0xB14B));
    r.jump = JumpByStateTable05CB1CF(slot);
    abi_ended(r.jump.end);
    slot = enter_handler(slot, r.jump);
    jsr_pop();
    r.aim = RD8(OBJ->f35c0[slot]);
    if (r.aim != 0) {
        r.arm = ST27_C_AIM;
        return r;
    }
    r.frame = RD8(OBJ->frame[slot]);
    if (r.frame == 0) {
        r.arm = ST27_C_NO_FRAME;
        return r;
    }
    r.diff = sub_bytes(r.frame, 1, true);
    r.y = r.diff.value;
    r.entry = rd8((uint16_t)(l8_at(lay, 0xB169) + r.y));
    if (r.entry == 0) {
        r.arm = ST27_C_NO_ENTRY;
        return r;
    }
    r.x = slot;
    push8(slot);
    r.list = add_bytes(r.entry, 0x81, false);
    r.arm = ST27_C_LISTED;
    abi_ended(then(bossc_tail(&r), then(ended_nz(r.x), ended_of_sum(r.list))));
    jsr_push(l8_at(lay, 0xB167));
    StartListChannelKind2(r.list.value, 5);
    jsr_pop();
    r.pulled = pull8();
    return r;
}

/* What BossFrameRoutinesC leaves in A, Y and the flags, from the arm it ended on. */
static Ended bossc_exit_shim(L8St27BossCArm r)
{
    if (r.arm == ST27_C_LISTED) {
        return ended_with_a(ended_nz(r.pulled), 2);
    }
    return bossc_tail(&r);
}

void abi_BossFrameRoutinesC(void)
{
    abi_ended(bossc_exit_shim(BossFrameRoutinesC(cpu.x, L8_LOADED)));
}

/*
 * @routine 0F6D:523 HitBoxRowByFrameKind
 * @run 71:C523
 * WHAT IT DOES: Takes the hit box (OBJ_3988, OBJ_3A38, OBJ_3930, OBJ_39E0) from a four byte row of the table at $C575, the row the table at $C545 names for the object's frame (OBJ_2A68).
 * HOW: A read of the frame as Y, a table read, two shifts, four table reads and stores.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte read, Y the row, C bit 6 of the kind, N and Z from A (l8_hitbox_row_exit_shim). Called by BossFrameRoutinesD from the states $BBB0 marks, which ran in no script (docs/rondo/l8-untraced-arms.md). The C reads the tables HitBoxKindByFrame ($C545) and HitBoxRows ($C575) by address until their reader exists (no script reaches the routine yet).
 */
L8HitBoxRowArm HitBoxRowByFrameKind(uint8_t slot)
{
    L8HitBoxRowArm r;
    uint8_t value;

    r.frame = RD8(OBJ->frame[slot]);
    r.kind = rd8((uint16_t)(0xC545 + r.frame));
    r.row = (uint8_t)(r.kind << 2);
    value = rd8((uint16_t)(0xC575 + r.row));
    WR8(OBJ->hitbox_half_w[slot], value);
    value = rd8((uint16_t)(0xC576 + r.row));
    WR8(OBJ->hitbox_half_h[slot], value);
    value = rd8((uint16_t)(0xC577 + r.row));
    WR8(OBJ->hitbox_dx[slot], value);
    r.last = rd8((uint16_t)(0xC578 + r.row));
    WR8(OBJ->hitbox_dy[slot], r.last);
    return r;
}

/* What HitBoxRowByFrameKind leaves in A, Y and the flags: the frame as Y, the kind, two shifts (C the kind's bit 6), the row as A and Y, then the last byte. */
static Ended l8_hitbox_row_exit_shim(L8HitBoxRowArm r)
{
    Ended e = ended_with_y(ended_nz(r.frame), r.frame);

    e = then(e, ended_load(r.kind));
    e = then(e, ended_with_carry(ended_load(r.row), (r.kind & 0x40) != 0));
    e = ended_with_y(e, r.row);
    return then(e, ended_load(r.last));
}

void abi_HitBoxRowByFrameKind(void)
{
    abi_ended(l8_hitbox_row_exit_shim(HitBoxRowByFrameKind(cpu.x)));
}

/* What BossFrameRoutinesD left after the table's code when the state was marked or not: the state as Y and the mark. */
static Ended bossd_tail(const L8St27BossDArm *r)
{
    return then(ended_with_y(ended_nz(r->y), r->y), ended_load(r->marked));
}

/*
 * @routine 0F6C:39C BossFrameRoutinesD
 * @run 70:BB9C
 * WHAT IT DOES: One frame of the boss of overlay 0F6C: runs EnterState0EWhenFlagged and FlashWhenHurtThenSound, the code the state table names (JumpByStateTable0F6C3C0) and, in the states the table at $BBB0 marks, HitBoxRowByFrameKind.
 * HOW: Three calls, a table read, a call into the original code.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran (bossd_exit_shim). The call into HitBoxRowByFrameKind (0F6D:523) ran in no script; the draft stops there.
 */
L8St27BossDArm BossFrameRoutinesD(uint8_t slot, uint8_t y_entry)
{
    L8St27BossDArm r = { 0 };

    jsr_push(0xBB9F);
    r.enter = EnterState0EWhenFlagged(slot);
    jsr_pop();
    abi_ended(l8_enter_0e_exit_shim(r.enter));
    jsr_push(0xBBA2);
    r.flash = FlashWhenHurtThenSound(slot, y_entry);
    jsr_pop();
    abi_sound_end(l8_flash_hurt_exit_shim(r.flash));
    jsr_push(0xBBA4);
    r.jump = JumpByStateTable0F6C3C0(slot);
    abi_ended(r.jump.end);
    slot = enter_handler(slot, r.jump);
    jsr_pop();
    r.y = RD8(OBJ->state[slot]);
    r.marked = rd8((uint16_t)(0xBBB0 + r.y));
    if (r.marked == 0) {
        r.arm = ST27_D_UNMARKED;
        return r;
    }
    abi_ended(bossd_tail(&r));
    jsr_push(0xBBAF);
    r.row = HitBoxRowByFrameKind(slot);
    jsr_pop();
    r.arm = ST27_D_MARKED;
    return r;
}

/* What BossFrameRoutinesD leaves in A, Y and the flags, from the arm it ended on. */
static Ended bossd_exit_shim(L8St27BossDArm r)
{
    if (r.arm == ST27_D_UNMARKED) {
        return bossd_tail(&r);
    }
    return l8_hitbox_row_exit_shim(r.row);
}

void abi_BossFrameRoutinesD(void)
{
    abi_ended(bossd_exit_shim(BossFrameRoutinesD(cpu.x, cpu.y)));
}
