/*
 * The checks stages $09 to $10 and the secret stages make on their objects before the state's handler: most change
 * the state once bit 7 of OBJ_HIT_STATUS is set, some spawn or count.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "hit_checks_b.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }
static void ldy(uint8_t value) { cpu.y = value; abi_nz(value); }
static void iny(void) { ldy((uint8_t)(cpu.y + 1)); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

static void sum_to_a(Sum sum)
{
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
}

/* LDA of OBJ_HIT_STATUS and AND #$80: true when bit 7 is set. */
static bool hit_bit(uint8_t slot)
{
    lda(RD8(OBJ->hit_status[slot]));
    lda((uint8_t)(cpu.a & 0x80));
    return cpu.a != 0;
}

/* LDA of OBJ_STATE and CMP: true when the state is under the limit. */
static bool state_under(uint8_t slot, uint8_t limit)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, limit);
    return cpu.a < limit;
}

static void sta_state(uint8_t slot, uint8_t value)
{
    lda(value);
    WR8(OBJ->state[slot], cpu.a);
}

static void sound(uint8_t command, uint16_t ret)
{
    CommandCall call;

    lda(command);
    jsr_push(ret);
    call = SoundCommand(command, cpu.x, cpu.y);
    sound_command_exit_shim(command, call);
    jsr_pop();
}

static void clear_slot_in_y(uint16_t ret)
{
    jsr_push(ret);
    ClearObjectInY(cpu.y);
    jsr_pop();
}

/* LDA #type and StartObjectAtPlace at X, in slot Y. */
static void start_at_place(uint8_t type)
{
    lda(type);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
}

/* Frame, no vertical speed, state, substate 0 and the hit box off, when under the state and hit. */
static void hit_to_state(uint8_t slot, uint8_t frame, uint8_t state)
{
    if (!state_under(slot, state) || !hit_bit(slot)) {
        return;
    }
    lda(frame);
    WR8(OBJ->frame[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    sta_state(slot, state);
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
}

/* The state and substate 0, when under the state and hit. */
static void hit_to_state_only(uint8_t slot, uint8_t state)
{
    if (!state_under(slot, state) || !hit_bit(slot)) {
        return;
    }
    sta_state(slot, state);
    WR8(OBJ->substate[slot], 0);
}

/*
 * @routine 0EBD:45F HitToState4Frame0EBD
 * @run 71:C45F
 * WHAT IT DOES: Under state 4, once hit: frame $05, no vertical speed, state 4, substate 0 and the hit box off.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
 */
void HitToState4Frame0EBD(uint8_t slot)
{
    hit_to_state(slot, 0x05, 4);
}

void abi_HitToState4Frame0EBD(void)
{
    HitToState4Frame0EBD(cpu.x);
}

/*
 * @routine 101E:68F HitToState2Frame101E
 * @run 71:CE8F
 * WHAT IT DOES: Under state 2, once hit: frame $21, no vertical speed, state 2, substate 0 and the hit box off.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
 */
void HitToState2Frame101E(uint8_t slot)
{
    hit_to_state(slot, 0x21, 2);
}

void abi_HitToState2Frame101E(void)
{
    HitToState2Frame101E(cpu.x);
}

/*
 * @routine 101B:088 HitToState4_101B
 * @run 70:B088
 * WHAT IT DOES: Once hit, an object under state 4 goes to state 4 (Stage 0A's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
 */
void HitToState4_101B(uint8_t slot)
{
    if (!hit_bit(slot) || !state_under(slot, 4)) {
        return;
    }
    sta_state(slot, 4);
}

void abi_HitToState4_101B(void)
{
    HitToState4_101B(cpu.x);
}

/*
 * @routine 101B:46C HitToState11_101B
 * @run 70:B46C
 * WHAT IT DOES: Under state 11, once hit: state 11 and substate 0.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
 */
void HitToState11_101B(uint8_t slot)
{
    hit_to_state_only(slot, 0x0B);
}

void abi_HitToState11_101B(void)
{
    HitToState11_101B(cpu.x);
}

/*
 * @routine 12DB:43A HitToState4_12DB
 * @run 70:B43A
 * WHAT IT DOES: Under state 4, once hit: state 4 and substate 0.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
 */
void HitToState4_12DB(uint8_t slot)
{
    hit_to_state_only(slot, 0x04);
}

void abi_HitToState4_12DB(void)
{
    HitToState4_12DB(cpu.x);
}

/*
 * @routine 161B:341 HitSpawn92_161B
 * @run 70:B341
 * WHAT IT DOES: Under state 4, once hit (Stage 0E's overlay): the next slot cleared and started as kind $92 at this place, rising,
 * not hit-tested for 22 frames; this object takes frame 9, state 4 and no hit box.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A 4 or as the test; Y the next slot; X kept.
 */
void HitSpawn92_161B(uint8_t slot)
{
    if (!state_under(slot, 4) || !hit_bit(slot)) {
        return;
    }
    lda(cpu.x);
    ldy(cpu.a);
    iny();
    clear_slot_in_y(0xB355);
    lda(0x92);
    jsr_push(0xB35A);
    start_at_place(cpu.a);
    jsr_pop();
    lda(0xFC);
    WR8(OBJ->vy_hi[cpu.y], cpu.a);
    cpu.a = 0;
    WR8(OBJ->vy_lo[cpu.y], cpu.a);
    lda(0x16);
    WR8(OBJ->invuln_timer[cpu.y], cpu.a);
    lda(0x09);
    WR8(OBJ->frame[slot], cpu.a);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    sta_state(slot, 4);
    WR8(OBJ->substate[slot], 0);
    WR8(OBJ->hitbox_on[slot], 0);
}

void abi_HitSpawn92_161B(void)
{
    HitSpawn92_161B(cpu.x);
}

/*
 * @routine 161B:625 HitHold32_161B
 * @run 70:B625
 * WHAT IT DOES: Once hit (Stage 0E's overlay): the hit box off, OBJ_WORK_H 32, frame 0, state 3.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A 3 or 0; X, Y, C and V kept.
 */
void HitHold32_161B(uint8_t slot)
{
    if (!hit_bit(slot)) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    lda(0x20);
    WR8(OBJ->work_h[slot], cpu.a);
    WR8(OBJ->frame[slot], 0);
    sta_state(slot, 3);
}

void abi_HitHold32_161B(void)
{
    HitHold32_161B(cpu.x);
}

/*
 * @routine 0EBD:44D MarkWhenPlayerNear0EBD
 * @run 71:C44D
 * WHAT IT DOES: While OBJ_WORK_G's bit 7 is clear, sets it to 1 once the player is within $30 dots along X (Stage 09's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load; X kept.
 */
void MarkWhenPlayerNear0EBD(uint8_t slot)
{
    PlayerDistance near;

    lda(RD8(OBJ->work_g[slot]));
    if ((cpu.a & 0x80) != 0) {
        return;
    }
    jsr_push(0xC455);
    near = DistanceXToPlayer(slot);
    abi_ended(ended_of_distance(near));
    jsr_pop();
    cmp(cpu.a, 0x30);
    if (cpu.a < 0x30) {
        lda(1);
        WR8(OBJ->work_g[slot], cpu.a);
    }
}

void abi_MarkWhenPlayerNear0EBD(void)
{
    MarkWhenPlayerNear0EBD(cpu.x);
}

/*
 * @routine 101C:2AE HitSoundToState4_101C
 * @run 70:BAAE
 * WHAT IT DOES: Once hit (Stage 0A's overlay): sound $DD and state 4.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A 4 or 0; X kept.
 */
void HitSoundToState4_101C(uint8_t slot)
{
    if (!hit_bit(slot)) {
        return;
    }
    sound(0xDD, 0xBABA);
    sta_state(slot, 4);
}

void abi_HitSoundToState4_101C(void)
{
    HitSoundToState4_101C(cpu.x);
}

/*
 * @routine 101C:2C0 HitBoxSizeByFrame101C
 * @run 70:BAC0
 * WHAT IT DOES: Gives the object the hit box size of its frame, from $74 on (Stage 0A's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A the half height, N and Z from it, Y the size's offset, C from the shift, V from the subtract.
 */
void HitBoxSizeByFrame101C(uint8_t slot)
{
    lda(RD8(OBJ->frame[slot]));
    sum_to_a(sub_bytes(cpu.a, 0x74, true));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xBAD9 + cpu.y)));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xBB01 + cpu.y)));
    WR8(OBJ->hitbox_half_w[slot], cpu.a);
    lda(rd8((uint16_t)(0xBB02 + cpu.y)));
    WR8(OBJ->hitbox_half_h[slot], cpu.a);
}

void abi_HitBoxSizeByFrame101C(void)
{
    HitBoxSizeByFrame101C(cpu.x);
}

/*
 * @routine 101C:790 CycleWhileUnderState2_101C
 * @run 70:BF90
 * WHAT IT DOES: The first of its kind (OBJ_START_ARG 0), from state 2 (Stage 0A's overlay): every 224 frames OBJ_WORK_M names the
 * next of 14 slots from the table at $BFD2 (at once again when slot $11 plus it is not kind $36), else $FF.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load; X kept.
 */
void CycleWhileUnderState2_101C(uint8_t slot)
{
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a != 0) {
        return;
    }
    if (state_under(slot, 2)) {
        return;
    }
    lda(0xFF);
    WR8(OBJ->work_m[slot], cpu.a);
    abi_nz(DEC8(OBJ->work_i[slot]));
    if ((cpu.p & P_Z) == 0) {
        return;
    }
    lda(0xE0);
    WR8(OBJ->work_i[slot], cpu.a);
    ldy(RD8(OBJ->work_l[slot]));
    lda(rd8((uint16_t)(0xBFD2 + cpu.y)));
    WR8(OBJ->work_m[slot], cpu.a);
    abi_flag(P_C, false);
    sum_to_a(add_bytes(cpu.a, 0x11, false));
    cpu.y = cpu.a;
    abi_nz(cpu.a);
    lda(RD8(OBJ->kind[cpu.y]));
    cmp(cpu.a, 0x36);
    if (cpu.a != 0x36) {
        lda(1);
        WR8(OBJ->work_i[slot], cpu.a);
    }
    INC8(OBJ->work_l[slot]);
    lda(RD8(OBJ->work_l[slot]));
    cmp(cpu.a, 0x0E);
    if (cpu.a >= 0x0E) {
        WR8(OBJ->work_l[slot], 0);
    }
}

void abi_CycleWhileUnderState2_101C(void)
{
    CycleWhileUnderState2_101C(cpu.x);
}

/*
 * @routine 101D:40F ToState4WhenPlayerClose101D
 * @run 71:C40F
 * WHAT IT DOES: Under state 4, with the player within 8 dots along X and $20 along Y (Stage 0A's overlay): sound $D7, the animation
 * restarted, state 4.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call or compare; X kept.
 */
void ToState4WhenPlayerClose101D(uint8_t slot)
{
    PlayerDistance distance;

    if (!state_under(slot, 4)) {
        return;
    }
    jsr_push(0xC419);
    distance = DistanceXToPlayer(slot);
    abi_ended(ended_of_distance(distance));
    jsr_pop();
    cmp(cpu.a, 0x08);
    if (cpu.a >= 0x08) {
        return;
    }
    jsr_push(0xC420);
    distance = DistanceYToPlayer(slot);
    abi_ended(ended_of_distance(distance));
    jsr_pop();
    cmp(cpu.a, 0x20);
    if (cpu.a >= 0x20) {
        return;
    }
    sound(0xD7, 0xC429);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    sta_state(slot, 4);
    WR8(OBJ->substate[slot], 0);
}

void abi_ToState4WhenPlayerClose101D(void)
{
    ToState4WhenPlayerClose101D(cpu.x);
}

/*
 * @routine 101D:438 HitToState5_101D
 * @run 71:C438
 * WHAT IT DOES: Under state 5, once hit (Stage 0A's overlay): the hit box off, OBJ_WORK_G $60, frame 1, state 5, substate 0.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load left them; C from the state's compare; X and V kept.
 */
void HitToState5_101D(uint8_t slot)
{
    if (!state_under(slot, 5) || !hit_bit(slot)) {
        return;
    }
    WR8(OBJ->hitbox_on[slot], 0);
    lda(0x60);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(0x01);
    WR8(OBJ->frame[slot], cpu.a);
    sta_state(slot, 5);
    WR8(OBJ->substate[slot], 0);
}

void abi_HitToState5_101D(void)
{
    HitToState5_101D(cpu.x);
}

/*
 * @routine 101C:7E0 HitBringDownSlots101C
 * @run 70:BFE0
 * WHAT IT DOES: Under state 4, once hit (Stage 0A's overlay): slot $11 plays sounds $DE and $FE and marks slots $12 to $1D hit
 * (OBJ_HIT_TAKEN 5, OBJ_WORK_N 1); the object stops, its hit box off, state 4.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load; X kept.
 */
void HitBringDownSlots101C(uint8_t slot)
{
    if (!state_under(slot, 4) || !hit_bit(slot)) {
        return;
    }
    cmp(cpu.x, 0x11);
    if (cpu.x == 0x11) {
        sound(0xDE, 0xBFF7);
        sound(0xFE, 0xBFFC);
        ldy(0x12);
        do {
            lda(0x05);
            WR8(OBJ->hit_taken[cpu.y], cpu.a);
            lda(0x01);
            WR8(OBJ->work_n[cpu.y], cpu.a);
            iny();
            cmp(cpu.y, 0x1E);
        } while (cpu.y != 0x1E);
    }
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    sta_state(slot, 4);
    WR8(OBJ->substate[slot], 0);
}

void abi_HitBringDownSlots101C(void)
{
    HitBringDownSlots101C(cpu.x);
}

/*
 * @routine 12DD:37F DropAndShake12DD
 * @run 71:C37F
 * WHAT IT DOES: While OBJ_WORK_I counts, every fourth frame of OBJ_WORK_D a kind $36 object starts at its place; else a hit of 5
 * or more under state 5 starts the count and halves OBJ_WORK_F (Stage 0C's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test, call or shift; X kept.
 */
void DropAndShake12DD(uint8_t slot)
{
    uint8_t found_a, to;
    bool found;

    lda(RD8(OBJ->work_i[slot]));
    if (cpu.a != 0) {
        DEC8(OBJ->work_i[slot]);
        lda(RD8(OBJ->work_d[slot]));
        lda((uint8_t)(cpu.a & 0x03));
        if (cpu.a != 0) {
            return;
        }
        jsr_push(0xC391);
        to = FindFreeSlotFrom30(&found_a, &found);
        find_slot_exit_shim(to, found_a, found);
        jsr_pop();
        if (!found) {
            return;
        }
        start_at_place(0x36);
        return;
    }
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a >= 0x05) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    lda(0x08);
    WR8(OBJ->work_i[slot], cpu.a);
    {
        uint8_t old = RD8(OBJ->work_f[slot]);

        WR8(OBJ->work_f[slot], (uint8_t)(old >> 1));
        abi_flag(P_C, (old & 1) != 0);
        abi_nz((uint8_t)(old >> 1));
    }
}

void abi_DropAndShake12DD(void)
{
    DropAndShake12DD(cpu.x);
}

/*
 * @routine 12DE:273 HalveWhenHit12DE
 * @run 71:CA73
 * WHAT IT DOES: Under state 5, a hit of 5 or more halves OBJ_WORK_F (Stage 0C's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare or the shift; X and Y kept.
 */
void HalveWhenHit12DE(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a >= 0x05) {
        return;
    }
    lda(RD8(OBJ->hit_taken[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    {
        uint8_t old = RD8(OBJ->work_f[slot]);

        WR8(OBJ->work_f[slot], (uint8_t)(old >> 1));
        abi_flag(P_C, (old & 1) != 0);
        abi_nz((uint8_t)(old >> 1));
    }
}

void abi_HalveWhenHit12DE(void)
{
    HalveWhenHit12DE(cpu.x);
}

/*
 * @routine 161C:2EF HitToState7_161C
 * @run 70:BAEF
 * WHAT IT DOES: Once hit, under state 7 (Stage 0E's overlay): the sound OBJ_WORK_K names (table at $BB18) when on the first screen,
 * the hit box off, the animation restarted, state 7.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or load; X kept.
 */
void HitToState7_161C(uint8_t slot)
{
    if (!hit_bit(slot) || !state_under(slot, 7)) {
        return;
    }
    ldy(RD8(OBJ->work_k[slot]));
    lda(rd8((uint16_t)(0xBB18 + cpu.y)));
    jsr_push(0xBB06);
    sound_if_on_first_screen_exit_shim(SoundIfOnFirstScreen(cpu.a, cpu.x, cpu.y));
    jsr_pop();
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    sta_state(slot, 7);
    WR8(OBJ->substate[slot], 0);
}

void abi_HitToState7_161C(void)
{
    HitToState7_161C(cpu.x);
}

/*
 * @routine 12DD:3AF HitStopAll12DD
 * @run 71:C3AF
 * WHAT IT DOES: Under state 5, once hit (Stage 0C's overlay): slot $11 cleared, the raster effect off, the object stopped
 * with no hit box, its animation restarted, state 5.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A 5 or as the test; Y $11 when taken; X kept.
 */
void HitStopAll12DD(uint8_t slot)
{
    if (!state_under(slot, 5) || !hit_bit(slot)) {
        return;
    }
    ldy(0x11);
    clear_slot_in_y(0xC3C2);
    WR8(ZP->raster_effect, 0);
    WR8(ZP->raster_arg, 0);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    sta_state(slot, 5);
    WR8(OBJ->substate[slot], 0);
}

void abi_HitStopAll12DD(void)
{
    HitStopAll12DD(cpu.x);
}

/*
 * @routine 12DE:285 HitStopAll12DE
 * @run 71:CA85
 * WHAT IT DOES: Under state 5, once hit (Stage 0C's overlay): slot $11 cleared, the water line off, the object stopped
 * with no hit box, its animation restarted, state 5.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A 5 or as the test; Y $11 when taken; X kept.
 */
void HitStopAll12DE(uint8_t slot)
{
    if (!state_under(slot, 5) || !hit_bit(slot)) {
        return;
    }
    ldy(0x11);
    clear_slot_in_y(0xCA98);
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->vx_hi[slot], 0);
    WR8(OBJ->vx_lo[slot], 0);
    WR8(OBJ->vy_hi[slot], 0);
    WR8(OBJ->vy_lo[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    wr8(RasterWaterWanted + 1, 0);
    wr8(RasterWaterWanted, 0);
    sta_state(slot, 5);
    WR8(OBJ->substate[slot], 0);
}

void abi_HitStopAll12DE(void)
{
    HitStopAll12DE(cpu.x);
}

/*
 * @routine 18DB:6FE SpawnAtRandom18DB
 * @run 70:B6FE
 * WHAT IT DOES: One time in 32, starts a kind $32 object at this place in the first empty slot from $40 to $4F (Stage 10's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A 0 and as the last call, or as the random's mask or the last compare; X kept.
 */
void SpawnAtRandom18DB(uint8_t slot)
{
    jsr_push(0xB701);
    random_exit_shim(NextRandom());
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x1F));
    if (cpu.a != 0) {
        return;
    }
    ldy(0x40);
    for (;;) {
        lda(RD8(OBJ->kind[cpu.y]));
        if (cpu.a == 0) {
            break;
        }
        iny();
        cmp(cpu.y, 0x50);
        if (cpu.y >= 0x50) {
            return;
        }
    }
    clear_slot_in_y(0xB715);
    jsr_push(0xB71A);
    start_at_place(0x32);
    jsr_pop();
}

void abi_SpawnAtRandom18DB(void)
{
    SpawnAtRandom18DB(cpu.x);
}

/*
 * @routine 18DB:71B HealthByFirstHit18DB
 * @run 70:B71B
 * WHAT IT DOES: The first of slots $11 to $13 with a hit of 5 or more gives this object's OBJ_HURT_TIMER half the table's byte for
 * it (Stage 10's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last test or the shift; X kept.
 */
void HealthByFirstHit18DB(uint8_t slot)
{
    ldy(0x11);
    for (;;) {
        lda(RD8(OBJ->hit_taken[cpu.y]));
        if (cpu.a != 0) {
            break;
        }
        iny();
        cmp(cpu.y, 0x14);
        if (cpu.y >= 0x14) {
            return;
        }
    }
    cmp(cpu.a, 0x05);
    if (cpu.a < 0x05) {
        return;
    }
    cpu.y = cpu.a;
    abi_nz(cpu.a);
    lda(rd8((uint16_t)(0xA27D + cpu.y)));
    abi_flag(P_C, (cpu.a & 1) != 0);
    lda((uint8_t)(cpu.a >> 1));
    WR8(OBJ->hurt_timer[slot], cpu.a);
}

void abi_HealthByFirstHit18DB(void)
{
    HealthByFirstHit18DB(cpu.x);
}

/*
 * @routine 0EBF:110 StartSixAt0EBF
 * @run 71:D110
 * WHAT IT DOES: Starts six kind $3E objects at this place in free slots from $30, each with its OBJ_WORK_D, OBJ_WORK_E and speed
 * high bytes from the table at $D146 (Stage 09's overlay).
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: X kept; as the last compare, or C clear with no free slot.
 */
void StartSixAt0EBF(uint8_t slot)
{
    uint8_t found_a, to;
    bool found;

    cpu.y = 0;
    do {
        WR8(ZP->spread_count, cpu.y);
        jsr_push(0xD116);
        to = FindFreeSlotFrom30(&found_a, &found);
        find_slot_exit_shim(to, found_a, found);
        jsr_pop();
        if (!found) {
            return;
        }
        jsr_push(0xD11D);
        start_at_place(0x3E);
        jsr_pop();
        WR8(ZP->spread_source, cpu.x);
        lda(RD8(ZP->spread_count));
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        abi_flag(P_C, (cpu.a & 0x80) != 0);
        lda((uint8_t)(cpu.a << 1));
        cpu.x = cpu.a;
        lda(rd8((uint16_t)(0xD146 + cpu.x)));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xD147 + cpu.x)));
        WR8(OBJ->work_e[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xD148 + cpu.x)));
        WR8(OBJ->vx_hi[cpu.y], cpu.a);
        lda(rd8((uint16_t)(0xD149 + cpu.x)));
        WR8(OBJ->vy_hi[cpu.y], cpu.a);
        cpu.x = RD8(ZP->spread_source);
        ldy(RD8(ZP->spread_count));
        iny();
        cmp(cpu.y, 0x06);
    } while (cpu.y != 0x06);
}

void abi_StartSixAt0EBF(void)
{
    StartSixAt0EBF(cpu.x);
}

/*
 * @routine 0EBF:0EB HitSoundAndSix0EBF
 * @run 71:D0EB
 * WHAT IT DOES: Once hit, not yet in state 6 (Stage 09's overlay): sound $C7, six pieces, the hit box off, the animation restarted,
 * state 6.
 * HOW: The tests and stores in turn.
 * WAITS: no.
 * Entry: X object. Exit: A 6 or as the test; C from the state's compare; X kept.
 */
void HitSoundAndSix0EBF(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    cmp(cpu.a, 0x06);
    if (cpu.a == 0x06) {
        return;
    }
    if (!hit_bit(slot)) {
        return;
    }
    sound(0xC7, 0xD0FE);
    jsr_push(0xD101);
    StartSixAt0EBF(slot);
    jsr_pop();
    WR8(OBJ->hitbox_on[slot], 0);
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
    sta_state(slot, 6);
}

void abi_HitSoundAndSix0EBF(void)
{
    HitSoundAndSix0EBF(cpu.x);
}
