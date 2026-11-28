/*
 * Three routines of the overlays of stages 06 and 07 that store into RAM under $6000-$7FFF which no reached code reads:
 * $7E1A-$7E20 and $7E70-$7E77 have no name and stay raw operands (these two routines count for M1 only until code that reads
 * the bytes settles a name); $7D4C is written as SaveBuffer plus $4C, inside the backup file in RAM.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "ops6280.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "stage_ops_c.h"
#include "stage0607_ram_c.h"

/*
 * @routine 0A9C:31D FillSevenWithThreeThenClear0A9C
 * @run 70:BB1D
 * WHAT IT DOES: Stores 3 into the seven bytes from $7E1A and clears the object (Stage 06's overlay). Its RAM
 * operand ($7E1A + Y, or $7E70 + Y) has no name: no reached code reads those bytes, so it stays a raw operand and
 * the routine counts for M1 only until code that reads them settles a name.
 * HOW: A loop of seven stores by Y; a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FillSevenWithThreeThenClear0A9C(void)
{
    op_lda(0x03);
    op_cly();
    do {
        wr8((uint16_t)(0x7E1A + cpu.y), cpu.a);
        op_iny();
        op_cpy(0x07);
    } while (!flag_z());
    ClearObject(cpu.x);
}

void abi_FillSevenWithThreeThenClear0A9C(void)
{
    FillSevenWithThreeThenClear0A9C();
}

/*
 * @routine 0BFF:1C3 FillEightWithArgThenClear0BFF
 * @run 71:D1C3
 * WHAT IT DOES: Stores the object's OBJ_START_ARG into the eight bytes from $7E70, records its event and clears it
 * (Stage 07's overlay). Its RAM operand ($7E1A + Y, or $7E70 + Y) has no name: no reached code reads those bytes,
 * so it stays a raw operand and the routine counts for M1 only until code that reads them settles a name.
 * HOW: A loop of eight stores by Y down to 0; RecordObjectEvent; a tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void FillEightWithArgThenClear0BFF(uint8_t slot)
{
    op_lda(RD8(OBJ->start_arg[slot]));
    op_ldy(0x07);
    do {
        wr8((uint16_t)(0x7E70 + cpu.y), cpu.a);
        op_dey();
    } while (!flag_n());
    {
        uint8_t stored;
        uint8_t event;
        uint8_t byte;

        jsr_push(0xD1D1);
        if (!RecordObjectEvent(slot, &stored, &event, &byte)) {
            cpu.a = 0;
            abi_nz(0);
        } else {
            record_event_exit_shim(event, stored, byte);
        }
        jsr_pop();
    }
    ClearObject(slot);
}

void abi_FillEightWithArgThenClear0BFF(void)
{
    FillEightWithArgThenClear0BFF(cpu.x);
}

/*
 * @routine 0BFF:3DA TileGateBySaveByte0BFF
 * @run 71:D3DA
 * WHAT IT DOES: Waits on CheckTileAndPartner0BFF's carry by state: state 0 loads the record (sprite set 6, frame 1)
 * and, once it reports C, sets frame and state 3; state 1, once it reports C, steps the state on; state 3 and on,
 * once it reports C, does nothing more; each of these then clears SaveBuffer+$4C and draws tile script $0C. State 2
 * plays the frame list at $D44A and steps the state on when it ends (Stage 07's overlay).
 * HOW: A chain of DEC A tests of the state; CheckTileAndPartner0BFF's carry; DrawTileScript as a tail;
 * StepObjectFrames.
 * WAITS: no.
 * Entry: X the object. Exit: A, X, Y and the flags as its last instruction or call left them.
 */
void TileGateBySaveByte0BFF(void)
{
    op_lda(RD8(OBJ->state[cpu.x]));
    if (flag_z()) {
        {
            uint8_t nz;
            bool carry;
            uint8_t last;

            jsr_push(0xD3E2);
            last = LoadObjectRecord(cpu.x, &nz, &carry);
            load_record_exit_shim(last, nz, carry);
            jsr_pop();
        }
        op_lda(0x06);
        WR8(OBJ->sprite_set[cpu.x], cpu.a);
        op_lda(0x01);
        WR8(OBJ->frame[cpu.x], cpu.a);
        call_routine(0xD3EF, 0xD42A);
        if (!flag_c()) {
            goto at_D422;
        }
        op_lda(0x03);
        WR8(OBJ->frame[cpu.x], cpu.a);
        WR8(OBJ->state[cpu.x], cpu.a);
    } else {
        cpu.a = f_dec(cpu.a);
        if (!flag_z()) {
            goto at_D40F;
        }
        call_routine(0xD401, 0xD42A);
        if (!flag_c()) {
            goto at_D422;
        }
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
    }
at_D406:
    wr8((uint16_t)(SaveBuffer + 0x4C), 0);
    op_cly();
    op_lda(0x0C);
    {
        uint8_t item;
        uint8_t group;

        item = cpu.a;
        group = cpu.y;
        abi_exit_tile_run(DrawTileScript(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
    }
    return;
at_D40F:
    cpu.a = f_dec(cpu.a);
    if (!flag_z()) {
        goto at_D423;
    }
    op_lda(0xD4);
    WR8(ZP->temp11, cpu.a);
    op_lda(0x4A);
    WR8(ZP->temp10, cpu.a);
    {
        uint8_t y;
        FrameStep step;

        jsr_push(0xD41D);
        y = cpu.y;
        step = StepObjectFrames(cpu.x, &y);
        step_frames_exit_shim(step, y);
        jsr_pop();
    }
    if (flag_z()) {
        RMW8(OBJ->state[cpu.x], v, f_inc(v));
    }
at_D422:
    return;
at_D423:
    call_routine(0xD426, 0xD42A);
    if (!flag_c()) {
        goto at_D422;
    }
    goto at_D406;
}

void abi_TileGateBySaveByte0BFF(void)
{
    TileGateBySaveByte0BFF();
}
