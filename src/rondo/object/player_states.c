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

enum { FLASH_FRAMES = 0x9CFD, FLASH_LISTS = 0x9CFF, FLASH_CHANNEL = 5, FLASH_STEPS = 4, FLASH_SOUND = 0x71, FLASH_FRAME_TIME = 4 };

/* The step that starts the flash's next list: reload the timer, take the frame and the list from the tables by the kind. */
static Ended flash_restart(uint8_t slot, uint16_t ret)
{
    uint8_t kind, index, base, step, list, started;
    Sum frame, sum;
    Ended e;

    WR8(OBJ->life_timer[slot], FLASH_FRAME_TIME);
    kind = RD8(OBJ->next_kind[slot]);
    index = (uint8_t)(kind & 1);
    base = rd8((uint16_t)(FLASH_FRAMES + index));
    step = RD8(OBJ->f3510[slot]);
    frame = add_bytes(base, step, false);
    WR8(OBJ->frame[slot], frame.value);
    sum = add_bytes(frame.value, rd8((uint16_t)(FLASH_LISTS + index)), false);
    list = sum.value;
    push8(slot);
    jsr_push(ret);
    started = StartListChannelKind2(list, FLASH_CHANNEL);
    jsr_pop();
    slot = pull8();
    e = ended_with_y(ended_with_overflow(ended_with_carry(ended_with_a(ended_nz(slot), started), sum.carry), sum.overflow), index);
    return e;
}

/*
 * @routine 0720:567 WaitForField85Two
 * @run 6B:9D67
 * WHAT IT DOES: Moves the object on to its next state when zField85 is 2.
 * HOW: CMP #2; equal increments the state.
 * WAITS: no.
 * Entry: X object. Exit: A zField85; N, Z, C from the compare, or N and Z from the new state. X, Y kept.
 */
Ended WaitForField85Two(uint8_t slot)
{
    uint8_t value = RD8(ZP->field85);
    Ended e = ended_compare(value, 2);

    if (value != 2) {
        return e;
    }
    return ended_then(e, ended_nz(INC8(OBJ->state[slot])));
}

void abi_WaitForField85Two(void)
{
    abi_ended(WaitForField85Two(cpu.x));
}

/*
 * @routine 0720:571 StepFlashTimer
 * @run 6B:9D71
 * WHAT IT DOES: Counts the flash's frame timer down; at zero steps its list counter, and at four steps ends the flash
 * (next state, bit 7 of zField8C, sound $71); before that it starts the next list.
 * HOW: DEC of the timer, INC of OBJ_3510 read again for the CMP, flash_restart.
 * WAITS: no.
 * Entry: X object, Y kept. Exit: timer running: N and Z from it. Else as the flash's restart (A 2, N and Z the object's
 * slot, C and V from the second add) or the sound. The end of the flash (four steps) is untraced (4 of 400 cases reach it
 * and the draft stops there): written from the disc's bytes.
 */
SoundEnd StepFlashTimer(uint8_t slot, uint8_t y)
{
    SoundEnd r = { ended_kept(), ended_kept(), false, 0, { 0 } };
    uint8_t timer = DEC8(OBJ->life_timer[slot]);
    uint8_t count, flags;

    r.end = ended_nz(timer);
    if (timer != 0) {
        return r;
    }
    INC8(OBJ->f3510[slot]);
    count = RD8(OBJ->f3510[slot]);
    r.end = ended_compare(count, FLASH_STEPS);
    if (count != FLASH_STEPS) {
        r.end = flash_restart(slot, 0x9DAD);
        return r;
    }
    INC8(OBJ->state[slot]);
    flags = RD8(ZP->field8c);
    WR8(ZP->field8c, (uint8_t)(flags | 0x80));
    r.end = ended_kept();
    r.before = ended_load(FLASH_SOUND);
    r.sounded = true;
    r.command = FLASH_SOUND;
    r.call = SoundCommand(FLASH_SOUND, slot, y);
    return r;
}

void abi_StepFlashTimer(void)
{
    abi_sound_end(StepFlashTimer(cpu.x, cpu.y));
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
Ended NextStateByKindBits(uint8_t slot)
{
    uint8_t flags = RD8(ZP->field8c);
    uint8_t kind;
    Ended e = ended_load(flags);
    bool bump;

    if (flags & 0x80) {
        return e;
    }
    INC8(OBJ->state[slot]);
    WR8(OBJ->life_timer[slot], FLASH_FRAME_TIME);
    kind = (uint8_t)(RD8(OBJ->next_kind[slot]) & 0x0F);
    e = ended_compare(kind, 2);
    if (kind < 2) {
        return e;
    }
    e = ended_compare(kind, 6);
    bump = kind < 6;
    if (!bump) {
        e = ended_compare(kind, 0x0A);
        bump = kind == 0x0A;
    }
    if (!bump) {
        e = ended_compare(kind, 0x0B);
        bump = kind == 0x0B;
    }
    if (bump) {
        e = ended_then(e, ended_nz(INC8(OBJ->state[slot])));
    }
    return e;
}

void abi_NextStateByKindBits(void)
{
    abi_ended(NextStateByKindBits(cpu.x));
}

/*
 * @routine 0720:5D6 StepFlashTimerDown
 * @run 6B:9DD6
 * WHAT IT DOES: The flash's other step: counts its frame timer down; at zero counts the list counter down; below zero
 * the flash ends (next state, sound $71), else the next list starts.
 * HOW: DEC of the timer, DEC of OBJ_3510, flash_restart.
 * WAITS: no.
 * Entry: X object, Y kept. Exit: as StepFlashTimer.
 */
SoundEnd StepFlashTimerDown(uint8_t slot, uint8_t y)
{
    SoundEnd r = { ended_kept(), ended_kept(), false, 0, { 0 } };
    uint8_t timer = DEC8(OBJ->life_timer[slot]);
    uint8_t count;

    r.end = ended_nz(timer);
    if (timer != 0) {
        return r;
    }
    count = DEC8(OBJ->f3510[slot]);
    r.end = ended_nz(count);
    if ((count & 0x80) == 0) {
        r.end = flash_restart(slot, 0x9E07);
        return r;
    }
    INC8(OBJ->state[slot]);
    r.end = ended_kept();
    r.before = ended_load(FLASH_SOUND);
    r.sounded = true;
    r.command = FLASH_SOUND;
    r.call = SoundCommand(FLASH_SOUND, slot, y);
    return r;
}

void abi_StepFlashTimerDown(void)
{
    abi_sound_end(StepFlashTimerDown(cpu.x, cpu.y));
}
