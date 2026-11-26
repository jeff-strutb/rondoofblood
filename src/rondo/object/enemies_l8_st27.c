/*
 * TEMPORARY: the stage object routines of lot L8 that run the jump tables of lot L6 (overlays 025A, 025B, 0B4B and 0F6B), by routine. Bodies return how the
 * routine's last instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it. The code a table names runs as the original.
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

/*
 * @routine 025B:471 BossFrameRoutines
 * @run 70:B471
 * WHAT IT DOES: One frame of the boss of overlay 025B: counts OBJ_3568 up, then runs SwingByCosineFromState0B, EnterState0BWhenFlagged, StartRandomScriptWhenFlagged, PushPlayerWhenFlagged and HitSoundThenSpawnPieces, and the code the state table names (JumpByStateTable025B545).
 * HOW: An INC, five calls, a call into the table's jump.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the code the table names.
 */
Ended BossFrameRoutines(uint8_t slot, uint8_t y_entry)
{
    Ended e;
    SoundEnd sound;
    Jump jump;

    e = ended_nz(INC8(OBJ->f3568[slot]));
    jsr_push(0xB477);
    e = then(e, SwingByCosineFromState0B(slot));
    jsr_pop();
    jsr_push(0xB47A);
    e = then(e, EnterState0BWhenFlagged(slot));
    jsr_pop();
    jsr_push(0xB47D);
    e = then(e, StartRandomScriptWhenFlagged(slot));
    jsr_pop();
    jsr_push(0xB480);
    e = then(e, PushPlayerWhenFlagged(slot));
    jsr_pop();
    jsr_push(0xB483);
    sound = HitSoundThenSpawnPieces(slot, e.y_set ? e.y : y_entry);
    jsr_pop();
    abi_sound_end(sound_end_after(e, sound));
    jsr_push(0xB486);
    jump = JumpByStateTable025B545(slot);
    e = jump.end;
    abi_ended(e);
    slot = enter_handler(slot, jump);
    jsr_pop();
    return ended_kept();
}

void abi_BossFrameRoutines(void)
{
    abi_ended(BossFrameRoutines(cpu.x, cpu.y));
}

/*
 * @routine 0F6B:38E HurtSoundsThenRunTable
 * @run 70:B38E
 * WHAT IT DOES: Runs SoundWhenHurtThenFlag and SoundAndState6WhenFlagged and then the code the state table names (JumpByStateTable0F6B3EC).
 * HOW: Three calls.
 * WAITS: no.
 * Entry: X object, Y kept for the sounds. Exit: as the code the table names.
 */
Ended HurtSoundsThenRunTable(uint8_t slot, uint8_t y_entry)
{
    SoundEnd sound;
    Jump jump;

    jsr_push(0xB390);
    sound = SoundWhenHurtThenFlag(slot, y_entry);
    jsr_pop();
    abi_sound_end(sound);
    jsr_push(0xB392);
    sound = SoundAndState6WhenFlagged(slot, y_entry);
    jsr_pop();
    abi_sound_end(sound);
    jsr_push(0xB394);
    jump = JumpByStateTable0F6B3EC(slot);
    abi_ended(jump.end);
    slot = enter_handler(slot, jump);
    jsr_pop();
    return ended_kept();
}

void abi_HurtSoundsThenRunTable(void)
{
    abi_ended(HurtSoundsThenRunTable(cpu.x, cpu.y));
}

/*
 * @routine 0B4B:556 BossFrameRoutinesB
 * @run 70:B556
 * WHAT IT DOES: One frame of the boss of overlay 0B4B: counts OBJ_3618 up, runs FaceThePlayerByStateTable, StateTableHurtFlag, EnterState0FWhenFlagged, HitBoxByFrameTable and the code the state table names (JumpByStateTable0B4B64C); then, when OBJ_3670 is set, starts the list $83 plus the frame less 1 on channel 5.
 * HOW: An INC, five calls, StartListChannelKind2 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran.
 */
Ended BossFrameRoutinesB(uint8_t slot, uint8_t y_entry)
{
    uint8_t flag, frame;
    Sum diff, list;
    Ended e;
    SoundEnd sound;
    Jump jump;

    e = ended_nz(INC8(OBJ->f3618[slot]));
    jsr_push(0xB55C);
    e = then(e, FaceThePlayerByStateTable(slot));
    jsr_pop();
    jsr_push(0xB55F);
    e = then(e, StateTableHurtFlag(slot));
    jsr_pop();
    abi_ended(e);
    jsr_push(0xB562);
    sound = EnterState0FWhenFlagged(slot, e.y_set ? e.y : y_entry);
    jsr_pop();
    abi_sound_end(sound);
    jsr_push(0xB565);
    e = HitBoxByFrameTable(slot);
    jsr_pop();
    abi_ended(e);
    jsr_push(0xB568);
    jump = JumpByStateTable0B4B64C(slot);
    abi_ended(jump.end);
    slot = enter_handler(slot, jump);
    jsr_pop();
    flag = RD8(OBJ->f3670[slot]);
    e = ended_load(flag);
    if (flag == 0) {
        return e;
    }
    push8(slot);
    frame = RD8(OBJ->frame[slot]);
    diff = sub_bytes(frame, 1, true);
    list = add_bytes(diff.value, 0x83, false);
    e = then(then(e, ended_nz(slot)), then(ended_of_sum(diff), ended_of_sum(list)));
    abi_ended(e);
    jsr_push(0xB57C);
    StartListChannelKind2(list.value, 5);
    jsr_pop();
    slot = pull8();
    return ended_with_a(ended_nz(slot), 2);
}

void abi_BossFrameRoutinesB(void)
{
    abi_ended(BossFrameRoutinesB(cpu.x, cpu.y));
}

/*
 * @routine 025B:09F SpawnRowsWhenScrollAtZero
 * @run 70:B09F
 * WHAT IT DOES: Runs SetBossFixedFields and StepVerticalScrollTable; when the scroll's bytes ScrollXHi+1 and ScrollXLo+1 are both 0 it sets the game's sub-mode 5, counts slot 0's OBJ_3510 down, sets the byte at $3BF0 to 1, clears bit 0 of zField8E, spawns the stage's rows 3 and 4 (SpawnStageObjectsRows34) and goes on to SetTimerThenNextState; otherwise it sets ScrollStepXHi+1 to 1.
 * HOW: Two calls, an ORA, stores, RMWs, SpawnStageObjectsRows34 between a PHX and a PLX, a tail jump.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran. The byte at $3BF0 keeps its address form until it has a name.
 */
Ended SpawnRowsWhenScrollAtZero(uint8_t slot)
{
    uint8_t high, low, a;
    StageSpawnEnd spawn;
    Ended e;

    jsr_push(0xB0A2);
    e = SetBossFixedFields();
    jsr_pop();
    jsr_push(0xB0A5);
    e = then(e, StepVerticalScrollTable(slot));
    jsr_pop();
    WR8(SCROLLE->step_x_hi[1], 0);
    high = RD8(SCROLL->x_hi[1]);
    low = RD8(SCROLL->x_lo[1]);
    e = then(e, ended_load((uint8_t)(high | low)));
    if ((high | low) != 0) {
        return then(e, ended_nz(INC8(SCROLLE->step_x_hi[1])));
    }
    WR8(ZP->game_sub_mode, 5);
    e = then(e, ended_load(5));
    e = then(e, ended_nz(DEC8(OBJ->f3510[0])));
    wr8(0x3BF0, 1);
    e = then(e, ended_load(1));
    RMW8(ZP->field8e, old, old & 0xFE);
    push8(slot);
    jsr_push(0xB0C2);
    spawn = SpawnStageObjectsRows34();
    jsr_pop();
    slot = pull8();
    a = 1;
    if (spawn.ran) {
        a = spawn.a;
        e = then(e, ended_with_y(ended_with_a(ended_nz(spawn.a), spawn.a), spawn.y));
    }
    e = then(e, ended_nz(slot));
    return then(e, SetTimerThenNextState(slot, a));
}

void abi_SpawnRowsWhenScrollAtZero(void)
{
    abi_ended(SpawnRowsWhenScrollAtZero(cpu.x));
}

/*
 * @routine 05CB:13F BossFrameRoutinesC
 * @run 70:B13F
 * WHAT IT DOES: One frame of the boss of overlay 05CB: counts OBJ_34B8 up, runs FlagThenHitsCompare, EnterState5WhenFlagged and the code the state table names (JumpByStateTable05CB1CF); then, when OBJ_35C0 and the frame are not 0, starts the list that the table at $B169 names for the frame less 1 (plus $81) on channel 5.
 * HOW: An INC, three calls, a table read, StartListChannelKind2 between a PHX and a PLX.
 * WAITS: no.
 * Entry: X object. Exit: as the last instruction that ran.
 */
Ended BossFrameRoutinesC(uint8_t slot)
{
    uint8_t aim, frame, y, entry;
    Sum diff, list;
    Ended e;
    Jump jump;

    e = ended_nz(INC8(OBJ->f34b8[slot]));
    jsr_push(0xB145);
    e = then(e, FlagThenHitsCompare(slot));
    jsr_pop();
    jsr_push(0xB148);
    e = then(e, EnterState5WhenFlagged(slot));
    jsr_pop();
    abi_ended(e);
    jsr_push(0xB14B);
    jump = JumpByStateTable05CB1CF(slot);
    abi_ended(jump.end);
    slot = enter_handler(slot, jump);
    jsr_pop();
    aim = RD8(OBJ->f35c0[slot]);
    e = ended_load(aim);
    if (aim != 0) {
        return e;
    }
    frame = RD8(OBJ->frame[slot]);
    e = then(e, ended_load(frame));
    if (frame == 0) {
        return e;
    }
    diff = sub_bytes(frame, 1, true);
    y = diff.value;
    e = then(then(e, ended_of_sum(diff)), ended_with_y(ended_nz(y), y));
    entry = rd8((uint16_t)(0xB169 + y));
    e = then(e, ended_load(entry));
    if (entry == 0) {
        return e;
    }
    push8(slot);
    list = add_bytes(entry, 0x81, false);
    abi_ended(then(e, then(ended_nz(slot), ended_of_sum(list))));
    jsr_push(0xB167);
    StartListChannelKind2(list.value, 5);
    jsr_pop();
    slot = pull8();
    return ended_with_a(ended_nz(slot), 2);
}

void abi_BossFrameRoutinesC(void)
{
    abi_ended(BossFrameRoutinesC(cpu.x));
}

/*
 * @routine 0F6C:39C BossFrameRoutinesD
 * @run 70:B39C
 * WHAT IT DOES: One frame of the boss of overlay 0F6C: runs EnterState0EWhenFlagged and FlashWhenHurtThenSound, the code the state table names (JumpByStateTable0F6C3C0) and, in the states the table at $BBB0 marks, the code at $C523.
 * HOW: Three calls, a table read, a call into the original code.
 * WAITS: no.
 * Entry: X object, Y kept for the sound. Exit: as the last instruction that ran. The call into $C523 ran in no script; the draft stops there.
 */
Ended BossFrameRoutinesD(uint8_t slot, uint8_t y_entry)
{
    uint8_t y, marked;
    Ended e;
    SoundEnd sound;
    Jump jump, extra = { { 0 }, 0xC523, true };

    jsr_push(0xBB9F);
    e = l8_enter_0e_exit_shim(EnterState0EWhenFlagged(slot));
    jsr_pop();
    abi_ended(e);
    jsr_push(0xBBA2);
    sound = l8_flash_hurt_exit_shim(FlashWhenHurtThenSound(slot, y_entry));
    jsr_pop();
    abi_sound_end(sound);
    jsr_push(0xBBA4);
    jump = JumpByStateTable0F6C3C0(slot);
    abi_ended(jump.end);
    slot = enter_handler(slot, jump);
    jsr_pop();
    y = RD8(OBJ->state[slot]);
    e = ended_with_y(ended_nz(y), y);
    marked = rd8((uint16_t)(0xBBB0 + y));
    e = then(e, ended_load(marked));
    if (marked == 0) {
        return e;
    }
    abi_ended(e);
    jsr_push(0xBBAF);
    slot = enter_handler(slot, extra);
    jsr_pop();
    return ended_kept();
}

void abi_BossFrameRoutinesD(void)
{
    abi_ended(BossFrameRoutinesD(cpu.x, cpu.y));
}
