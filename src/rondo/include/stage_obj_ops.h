/*
 * The instruction forms and kernel calls of the stage 02, 04 and 05 object code, as the C writes them. Each k_ is
 * the kernel routine through its typed C with the registers it takes and leaves; a JSR is JSR(ret, k_...()) and
 * a tail JMP the k_ alone.
 */
#ifndef RONDO_STAGE_OBJ_OPS_H
#define RONDO_STAGE_OBJ_OPS_H

#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "common_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "maria.h"

/* The probes the landing and wall movers ask for, and the VDC registers the kernel's selects leave in A. */
enum { STAGE_PROBE_FLOOR = 0x01, STAGE_PROBE_WALL = 0x02, STAGE_VDC_MAWR = 0x00, STAGE_VDC_MARR = 0x01, STAGE_VDC_VRAM = 0x02 };

#define JSR(ret, call) do { jsr_push(ret); call; jsr_pop(); } while (0)

#define CC ((cpu.p & P_C) == 0)
#define CS ((cpu.p & P_C) != 0)
#define EQ ((cpu.p & P_Z) != 0)
#define NE ((cpu.p & P_Z) == 0)
#define MI ((cpu.p & P_N) != 0)
#define PL ((cpu.p & P_N) == 0)

static inline void lda(uint8_t value) { cpu.a = value; abi_nz(value); }
static inline void ldx(uint8_t value) { cpu.x = value; abi_nz(value); }
static inline void ldy(uint8_t value) { cpu.y = value; abi_nz(value); }

/* CMP, CPX, CPY. */
static inline void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

static inline void and_a(uint8_t value) { lda((uint8_t)(cpu.a & value)); }
static inline void ora_a(uint8_t value) { lda((uint8_t)(cpu.a | value)); }
static inline void eor_a(uint8_t value) { lda((uint8_t)(cpu.a ^ value)); }

static inline void asl_a(void)
{
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
}

static inline void lsr_a(void)
{
    abi_flag(P_C, (cpu.a & 0x01) != 0);
    lda((uint8_t)(cpu.a >> 1));
}

static inline void rol_a(void)
{
    bool out = (cpu.a & 0x80) != 0;

    lda((uint8_t)((cpu.a << 1) | (CS ? 0x01 : 0)));
    abi_flag(P_C, out);
}

static inline void ror_a(void)
{
    bool out = (cpu.a & 0x01) != 0;

    lda((uint8_t)((cpu.a >> 1) | (CS ? 0x80 : 0)));
    abi_flag(P_C, out);
}

static inline void sum_to_a(Sum sum)
{
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

/* ADC and SBC with the carry as it stands; CLC and SEC are clc() and sec(). */
static inline void clc(void) { abi_flag(P_C, false); }
static inline void sec(void) { abi_flag(P_C, true); }
static inline void adc_a(uint8_t value) { sum_to_a(add_bytes(cpu.a, value, CS)); }
static inline void sbc_a(uint8_t value) { sum_to_a(sub_bytes(cpu.a, value, CS)); }

static inline void inc_a(void) { lda((uint8_t)(cpu.a + 1)); }
static inline void dec_a(void) { lda((uint8_t)(cpu.a - 1)); }
static inline void tax(void) { ldx(cpu.a); }
static inline void tay(void) { ldy(cpu.a); }
static inline void txa(void) { lda(cpu.x); }
static inline void tya(void) { lda(cpu.y); }
static inline void inx(void) { ldx((uint8_t)(cpu.x + 1)); }
static inline void iny(void) { ldy((uint8_t)(cpu.y + 1)); }
static inline void dex(void) { ldx((uint8_t)(cpu.x - 1)); }
static inline void dey(void) { ldy((uint8_t)(cpu.y - 1)); }

static inline void sxy(void)
{
    uint8_t x = cpu.x;

    cpu.x = cpu.y;
    cpu.y = x;
}

static inline void phx(void) { push8(cpu.x); }
static inline void phy(void) { push8(cpu.y); }
static inline void pha(void) { push8(cpu.a); }
static inline void plx(void) { ldx(pull8()); }
static inline void ply(void) { ldy(pull8()); }
static inline void pla(void) { lda(pull8()); }

/* ROR of a byte in memory: the carry in at the top, the carry out from bit 0, N and Z from the result. */
#define ROR_MEM(field) do { \
    bool in_ = CS, out_; \
    uint8_t v_ = RMW8(field, old_, (out_ = (old_ & 0x01) != 0, (old_ >> 1) | (in_ ? 0x80 : 0))); \
    abi_nz(v_); \
    abi_flag(P_C, out_); \
} while (0)

/* The two bytes of a list or script address into zTemp10-11, low byte first (LDA, STA, LDA, STA). */
static inline void list_to_temp10(uint16_t list)
{
    lda((uint8_t)list);
    WR8(ZP->temp10, cpu.a);
    lda((uint8_t)(list >> 8));
    WR8(ZP->temp11, cpu.a);
}

/* JMP (ZeroPage) through a table of handlers by the object's state or substate. */
static inline void jump_by_state(uint8_t slot, uint16_t table)
{
    JumpTableLayout layout = { table };
    Jump jump = JumpByStateTableBody(slot, &layout);

    jump_shim(jump.end, jump.target);
}

static inline void jump_by_substate(uint8_t slot, uint16_t table)
{
    JumpTableLayout layout = { table };
    Jump jump = JumpByPhaseTableBody(slot, &layout);

    jump_shim(jump.end, jump.target);
}

/* The kernel. */
static inline void k_step_frames(void)
{
    uint8_t y = cpu.y;
    FrameStep step = StepObjectFrames(cpu.x, &y);

    step_frames_exit_shim(step, y);
}

static inline void k_load_record(void)
{
    uint8_t nz;
    bool carry;
    uint8_t last = LoadObjectRecord(cpu.x, &nz, &carry);

    load_record_exit_shim(last, nz, carry);
}

static inline void k_clear(void) { ClearObject(cpu.x); }
static inline void k_clear_in_y(void) { ClearObjectInY(cpu.y); }
static inline void k_move(void) { axis_move_exit_shim(ApplyVelocity(cpu.x)); }
static inline void k_move_x(void) { axis_move_exit_shim(ApplyVelocityX(cpu.x)); }
static inline void k_move_y(void) { axis_move_exit_shim(ApplyVelocityY(cpu.x)); }
static inline void k_gravity(void) { gravity_exit_shim(ApplyGravity(cpu.x)); }
static inline void k_gravity_unlimited(void) { gravity_exit_shim(ApplyGravityUnlimited(cpu.x)); }
static inline void k_start_at_place(void) { copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y)); }
static inline void k_copy_place(void) { copy_place_exit_shim(CopyObjectPlace(cpu.x, cpu.y)); }
static inline void k_copy_place_back(void) { copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, cpu.x)); }
static inline void k_random(void) { random_exit_shim(NextRandom()); }
static inline void k_side(void) { side_of_player_exit_shim(SideOfPlayer(cpu.x)); }
static inline void k_distance_x(void) { distance_exit_shim(DistanceXToPlayer(cpu.x)); }
static inline void k_distance_y(void) { distance_exit_shim(DistanceYToPlayer(cpu.x)); }
static inline void k_velocity_from_angle(void) { scale_exit_shim(SetVelocityFromAngle(cpu.a, cpu.x)); }
static inline void k_cosine(void) { scale_exit_shim(ScaleByCosine(cpu.a)); }
static inline void k_sine(void) { scale_exit_shim(ScaleBySine(cpu.a)); }
static inline void k_negate_x(void) { negate_exit_shim(NegateVelocityX(cpu.x)); }
static inline void k_negate_y(void) { negate_exit_shim(NegateVelocityY(cpu.x)); }
static inline void k_probe(void) { probe_exit_shim(ProbeTerrain(cpu.a, cpu.x, cpu.y, CS)); }
static inline void k_stop_on_landing(void) { mover_exit_shim(StopOnLanding(cpu.a, cpu.x, cpu.y, CS), STAGE_PROBE_FLOOR); }
static inline void k_stop_on_landing_front(void)
{
    mover_exit_shim(StopOnLandingFrontColumn(cpu.a, cpu.x, cpu.y, CS), STAGE_PROBE_FLOOR);
}
static inline void k_move_and_land(void) { mover_exit_shim(MoveAndLand(cpu.a, cpu.x, cpu.y, CS), STAGE_PROBE_WALL); }
static inline void k_angle_to_player(void) { angle_exit_shim(AngleToPlayer(cpu.x)); }
static inline void k_angle_to_point(void) { angle_exit_shim(AngleToPoint(cpu.x)); }
static inline void k_angle_to_player_offset(void) { angle_exit_shim(AngleToPlayerOffset(cpu.x)); }
static inline void k_distance_sum(void) { distance_sum_exit_shim(PlayerDistanceSum(cpu.x, CS)); }
static inline void k_turn_angle(void) { turn_angle_exit_shim(TurnAngleToward()); }
static inline void k_script1(void) { script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x)); }
static inline void k_script4(void) { script_slot_exit_shim(StartScriptSlotKind4(cpu.a, cpu.x)); }
static inline void k_list2(void) { list_channel_exit_shim(StartListChannelKind2(cpu.a, cpu.x)); }
static inline void k_list3(void) { list_channel_exit_shim(StartListChannelKind3(cpu.a, cpu.x)); }
static inline void k_sound_first_screen(void)
{
    sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, cpu.x, cpu.y));
}

static inline void k_sound(void)
{
    CommandCall call;
    uint8_t command = cpu.a;

    call = SoundCommand(command, cpu.x, cpu.y);
    sound_command_exit_shim(command, call);
}

static inline void k_free_from20(void)
{
    uint8_t a;
    bool found;
    uint8_t slot = FindFreeSlotFrom20(&a, &found);

    find_slot_exit_shim(slot, a, found);
}

static inline void k_free_from30(void)
{
    uint8_t a;
    bool found;
    uint8_t slot = FindFreeSlotFrom30(&a, &found);

    find_slot_exit_shim(slot, a, found);
}

static inline void k_free_from50(void)
{
    uint8_t a;
    bool found;
    uint8_t slot = FindFreeSlotFrom50(&a, &found);

    find_slot_exit_shim(slot, a, found);
}

static inline void k_record_event(void)
{
    uint8_t stored, event, byte;

    if (!RecordObjectEvent(cpu.x, &stored, &event, &byte)) {
        lda(0);
        return;
    }
    record_event_exit_shim(event, stored, byte);
}

static inline void k_draw_tiles(void)
{
    uint8_t item = cpu.a;
    uint8_t group = cpu.y;

    abi_exit_tile_run(DrawTileScript(item, group), (uint8_t)((uint8_t)(item << 1) + 1));
}

static inline void k_select_vram_write(void) { SelectVramWriteAddress(); cpu.a = STAGE_VDC_MAWR; }
static inline void k_select_vram_read(void) { SelectVramReadAddress(); lda(STAGE_VDC_MARR); }
static inline void k_select_vram_data(void) { SelectVramData(); lda(STAGE_VDC_VRAM); }
static inline void k_clear_vdc_high(void) { ClearVdcControlHigh(); lda(0); }

#endif
