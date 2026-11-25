/*
 * TEMPORARY: the stage 0841 object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return how the routine's last
 * instructions left A, Y and the flags (Ended, enemy_objects.h); the abi_ wrappers show it.
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

/*
 * @routine 088B:6C5 ClearFrame
 * @run 70:B6C5
 * WHAT IT DOES: Sets the object's frame to 0.
 * HOW: A store.
 * WAITS: no.
 * Entry: X object. Exit: all kept.
 */
Ended ClearFrame(uint8_t slot)
{
    WR8(OBJ->frame[slot], 0);
    return ended_kept();
}

void abi_ClearFrame(void)
{
    abi_ended(ClearFrame(cpu.x));
}

/*
 * @routine 088B:21D TurnAroundInState9
 * @run 70:B21D
 * WHAT IT DOES: Sets the object's state to 9 and reverses its X speed (a tail jump into NegateVelocityX).
 * HOW: A store, NegateVelocityX.
 * WAITS: no.
 * Entry: X object. Exit: as NegateVelocityX.
 */
Ended TurnAroundInState9(uint8_t slot)
{
    WR8(OBJ->state[slot], 9);
    return then(ended_load(9), ended_of_sum(NegateVelocityX(slot)));
}

void abi_TurnAroundInState9(void)
{
    abi_ended(TurnAroundInState9(cpu.x));
}

/*
 * @routine 088B:225 FallBackInState9
 * @run 70:B225
 * WHAT IT DOES: Sets the object's state to 9 and reverses its Y speed (a tail jump into NegateVelocityY).
 * HOW: A store, NegateVelocityY.
 * WAITS: no.
 * Entry: X object. Exit: as NegateVelocityY.
 */
Ended FallBackInState9(uint8_t slot)
{
    WR8(OBJ->state[slot], 9);
    return then(ended_load(9), ended_of_sum(NegateVelocityY(slot)));
}

void abi_FallBackInState9(void)
{
    abi_ended(FallBackInState9(cpu.x));
}

/*
 * @routine 088A:71E PlaySoundTwo
 * @run 70:AF1E
 * WHAT IT DOES: Plays sound 2.
 * HOW: SoundCommand.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: see abi_sound_end.
 */
SoundEnd PlaySoundTwo(uint8_t slot, uint8_t y_entry)
{
    SoundEnd r = { { 0 }, { 0 }, true, 2, { 0 } };

    r.before = ended_load(2);
    jsr_push(0xAF23);
    r.call = SoundCommand(2, slot, y_entry);
    jsr_pop();
    r.end = ended_kept();
    return r;
}

void abi_PlaySoundTwo(void)
{
    abi_sound_end(PlaySoundTwo(cpu.x, cpu.y));
}
