/*
 * TEMPORARY: the stage 04D1 object routines of lot L6 (enemy kinds not yet identified), by routine. Bodies return how the routine's last
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
 * @routine 051B:23D LoadRecordA04
 * @run 70:B23D
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
Ended LoadRecordA04(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(0xB240);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    return ended_of_record(last, nz, carry);
}

void abi_LoadRecordA04(void)
{
    abi_ended(LoadRecordA04(cpu.x));
}

/*
 * @routine 051B:76A LoadRecordB04
 * @run 70:B76A
 * WHAT IT DOES: Loads the object's record.
 * HOW: LoadObjectRecord.
 * WAITS: no.
 * Entry: X object. Exit: as LoadObjectRecord.
 */
Ended LoadRecordB04(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(0xB76D);
    last = LoadObjectRecord(slot, &nz, &carry);
    jsr_pop();
    return ended_of_record(last, nz, carry);
}

void abi_LoadRecordB04(void)
{
    abi_ended(LoadRecordB04(cpu.x));
}

/*
 * @routine 051B:7C4 StepListB819
 * @run 70:B7C4
 * WHAT IT DOES: Steps the animation list at $B819, then goes on into the code that follows it ($B7CF).
 * HOW: StepObjectFrames; the code after it is entered by a tail jump.
 * WAITS: no.
 * Entry: X object, Y kept unless the step sets it. Exit: as the step, then the code at $B7CF.
 */
Jump StepListB819(uint8_t slot, uint8_t y_entry)
{
    FrameStep step;
    uint8_t y = y_entry;
    Jump jump = { ended_kept(), 0xB7CF, true };

    WR8(ZP->temp10, 0x19);
    WR8(ZP->temp11, 0xB8);
    jsr_push(0xB7CF);
    step = StepObjectFrames(slot, &y);
    jsr_pop();
    jump.end = ended_of_frame_step(step, step != FRAME_HELD, y);
    return jump;
}

void abi_StepListB819(void)
{
    Jump jump = StepListB819(cpu.x, cpu.y);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 051E:109 RandomTimer10
 * @run 71:C909
 * WHAT IT DOES: Sets the object's timer OBJ_34B8 to a random number from 0 to 15 plus 16.
 * HOW: NextRandom.
 * WAITS: no.
 * Entry: X object. Exit: A the timer, N, Z, C and V from the add.
 */
Ended RandomTimer10(uint8_t slot)
{
    RandomResult random;
    Sum sum;
    Ended e;

    jsr_push(0xC90C);
    random = NextRandom();
    jsr_pop();
    e = ended_of_random(random);
    e = ended_then(e, ended_load((uint8_t)(random.value & 0x0F)));
    sum = add_bytes((uint8_t)(random.value & 0x0F), 0x10, false);
    WR8(OBJ->f34b8[slot], sum.value);
    return ended_then(e, ended_of_sum(sum));
}

void abi_RandomTimer10(void)
{
    abi_ended(RandomTimer10(cpu.x));
}

/*
 * @routine 051D:4E8 CopyScrollToTimer
 * @run 71:C4E8
 * WHAT IT DOES: Sets the object's OBJ_34B8 to the scroll's X fraction of slot 4 and then to its X low of slot 4 (the second store stays).
 * HOW: Two reads and two stores.
 * WAITS: no.
 * Entry: X object. Exit: A the scroll X low, N and Z from it.
 */
Ended CopyScrollToTimer(uint8_t slot)
{
    uint8_t value;

    value = RD8(SCROLL->x_frac[4]);
    WR8(OBJ->f34b8[slot], value);
    value = RD8(SCROLL->x_lo[4]);
    WR8(OBJ->f34b8[slot], value);
    return ended_load(value);
}

void abi_CopyScrollToTimer(void)
{
    abi_ended(CopyScrollToTimer(cpu.x));
}

/*
 * @routine 051B:4BF SetHitBox0C18
 * @run 70:B4BF
 * WHAT IT DOES: Sets the object's hit box: X offset 0, half width $0C, half height $18.
 * HOW: Three stores.
 * WAITS: no.
 * Entry: X object. Exit: A $18, N and Z from it.
 */
Ended SetHitBox0C18(uint8_t slot)
{
    WR8(OBJ->hitbox_dx[slot], 0);
    WR8(OBJ->hitbox_half_w[slot], 0x0C);
    WR8(OBJ->hitbox_half_h[slot], 0x18);
    return ended_load(0x18);
}

void abi_SetHitBox0C18(void)
{
    abi_ended(SetHitBox0C18(cpu.x));
}

/*
 * @routine 051B:5CD StartPieceAtNextSlot
 * @run 70:B5CD
 * WHAT IT DOES: Clears the object in the slot after this one, starts a kind $32 object there at this object's place and goes on to the next OBJ_2800.
 * HOW: ClearObjectInY, StartObjectAtPlace, an INC.
 * WAITS: no.
 * Entry: X object. Exit: A the last byte copied, Y the slot after; N and Z from the new OBJ_2800.
 */
Ended StartPieceAtNextSlot(uint8_t slot)
{
    uint8_t to = (uint8_t)(slot + 1), placed, phase;

    jsr_push(0xB5D3);
    ClearObjectInY(to);
    jsr_pop();
    jsr_push(0xB5D8);
    placed = StartObjectAtPlace(0x32, slot, to);
    jsr_pop();
    phase = INC8(OBJ->f2800[slot]);
    return ended_with_y(ended_with_a(ended_nz(phase), placed), to);
}

void abi_StartPieceAtNextSlot(void)
{
    abi_ended(StartPieceAtNextSlot(cpu.x));
}

/*
 * @routine 051B:690 DrawTileScriptByHighBits
 * @run 70:B690
 * WHAT IT DOES: Draws the tile script of group 0 numbered by bits 4 to 6 of OBJ_3040 (as 0 to 7) and goes on to the next OBJ_2800.
 * HOW: DrawTileScript, an INC.
 * WAITS: no.
 * Entry: X object. Exit: the tile script's registers, then N and Z from the new OBJ_2800.
 */
Ended DrawTileScriptByHighBits(uint8_t slot)
{
    TileRun run;
    uint8_t item, phase;
    Ended e;

    item = (uint8_t)((RD8(OBJ->f3040[slot]) & 0x70) >> 4);
    e = ended_with_carry(ended_load(item), false);
    jsr_push(0xB69D);
    run = DrawTileScript(item, 0);
    jsr_pop();
    e = then(e, ended_of_tile_run(run, item));
    phase = INC8(OBJ->f2800[slot]);
    return then(e, ended_nz(phase));
}

void abi_DrawTileScriptByHighBits(void)
{
    abi_ended(DrawTileScriptByHighBits(cpu.x));
}
