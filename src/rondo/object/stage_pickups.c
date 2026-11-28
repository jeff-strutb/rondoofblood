/*
 * Stage objects shared by the stages' object banks (sectors 0250, 0259, 025A and 025C as the first stage loads them):
 * short handlers and the pieces of the one-at-a-time flash object. Each takes the object in X and the registers it is
 * entered with, and gives back those it leaves.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_map.h"
#include "stage_regs.h"

/* Defined elsewhere, typed, and not declared in a header. */
void ClearObject(uint8_t object);                    /* object/lifecycle.c */
void ClearObjectInY(uint8_t object);                 /* object/lifecycle.c */

typedef StageRegs Regs;

/* A JSR to the routine loaded at run: the registers as they stand, and what it leaves. */
static Regs call_to(Regs r, uint16_t ret, uint16_t run)
{
    stage_regs_load_shim(r);
    call_routine(ret, run);
    return stage_regs_from_cpu_shim(r);
}

/* A JSR at ret to the sound hook with A the command: what it leaves. */
static Regs sound_jsr(Regs r, uint8_t command, uint16_t ret)
{
    CommandCall call;

    r = regs_loaded(r, command);
    stage_regs_load_shim(r);
    jsr_push(ret);
    call = SoundCommand(command, r.x, r.y);
    jsr_pop();
    sound_command_exit_shim(command, call);
    return stage_regs_from_cpu_shim(r);
}

/* ApplyVelocity's exit: C and V from the last add, A the position's high byte, N and Z from it. */
static Regs moved_regs(Regs r, AxisMove moved)
{
    stage_regs_load_shim(r);
    cpu.a = axis_move_flags_shim(moved);
    abi_nz(cpu.a);
    return stage_regs_from_cpu_shim(r);
}

/* A tail JMP to the sound hook with A the command: what it leaves. */
static Regs sound_tail(Regs r, uint8_t command)
{
    CommandCall call;

    stage_regs_load_shim(r);
    call = SoundCommand(command, r.x, r.y);
    sound_command_exit_shim(command, call);
    return stage_regs_from_cpu_shim(r);
}

/*
 * @routine 0259:36A StageObjectRemove
 * @run 70:A36A
 * WHAT IT DOES: Empties the object's slot.
 * HOW: The tail JMP to the kernel's ClearObject vector.
 * WAITS: no.
 * Entry: X object. Exit: A, X, Y and flags kept.
 */
StageRegs StageObjectRemove(StageRegs r)
{
    ClearObject(r.x);
    return r;
}

void abi_StageObjectRemove(void)
{
    stage_regs_load_shim(StageObjectRemove(regs_entry_shim()));
}

/*
 * @routine 0259:5DA StageObjectBecomeScore
 * @run 70:A5DA
 * WHAT IT DOES: Turns the object into the score marker (kind $9F) after blending the score added: its OBJ_MARKER_VALUE from
 * the value in Y, its state cleared.
 * HOW: PHY, BlendStageScoreAddRows by its $9181 C, PLA.
 * WAITS: no.
 * Entry: X object, Y the marker's value. Exit: A $9F, N and Z from it; X, Y, C and V as the blend left them.
 */
StageRegs StageObjectBecomeScore(StageRegs r)
{
    uint8_t value;

    push8(r.y);
    r = call_to(r, 0xA5DE, 0x9181);
    value = pull8();
    r = regs_loaded(r, value);
    WR8(OBJ->marker_value[r.x], value);
    r = regs_loaded(r, 0x9F);
    WR8(OBJ->kind[r.x], 0x9F);
    WR8(OBJ->state[r.x], 0);
    return r;
}

void abi_StageObjectBecomeScore(void)
{
    stage_regs_load_shim(StageObjectBecomeScore(regs_entry_shim()));
}

/*
 * @routine 0259:675 StageFlashStart
 * @run 70:A675
 * WHAT IT DOES: Starts the screen flash unless one is running, in which case the object is cleared: the sound $50, the
 * sole script slot 3 with script 0, $12 frames, bit 1 of HitAttackFlags set, the object's frame cleared and its state
 * counted on.
 * HOW: StageFlashActive; the sound hook; StartSoleScriptSlot with X kept on the stack.
 * WAITS: no.
 * Entry: X object. Exit: running: as ClearObject (A the flag); else N and Z from the state's INC, A HitAttackFlags.
 */
StageRegs StageFlashStart(StageRegs r)
{
    uint8_t value;

    r = regs_loaded(r, rd8(StageFlashActive));
    if (r.a != 0) {
        ClearObject(r.x);
        return r;
    }
    r = regs_loaded(r, 0x01);
    wr8(StageFlashActive, 0x01);
    r = sound_jsr(r, 0x50, 0xA687);
    push8(r.x);
    r.a = 0;                                   /* CLA: no flags */
    r.x = 3;
    r.nz = 3;                                  /* LDX */
    r = call_to(r, 0xA68E, 0x4042);
    value = pull8();
    r.x = value;
    r.nz = value;
    r = regs_loaded(r, 0x12);
    WR8(OBJ->flash_frames[r.x], 0x12);
    value = (uint8_t)(rd8(HitAttackFlags) | 0x02);
    r = regs_loaded(r, value);
    wr8(HitAttackFlags, value);
    WR8(OBJ->frame[r.x], 0);
    r.nz = INC8(OBJ->state[r.x]);
    return r;
}

void abi_StageFlashStart(void)
{
    stage_regs_load_shim(StageFlashStart(regs_entry_shim()));
}

/*
 * @routine 025A:009 StageFlashEnd
 * @run 70:A809
 * WHAT IT DOES: The flash's countdown: bit 1 of HitAttackFlags cleared; when the object's OBJ_FLASH_FRAMES runs out the
 * flash is over and the object cleared.
 * HOW: AND and a store, DEC, the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: A HitAttackFlags' new value; counting N and Z from the DEC, else as ClearObject.
 */
StageRegs StageFlashEnd(StageRegs r)
{
    uint8_t value = (uint8_t)(rd8(HitAttackFlags) & 0xFD);

    r = regs_loaded(r, value);
    wr8(HitAttackFlags, value);
    r.nz = DEC8(OBJ->flash_frames[r.x]);
    if (r.nz != 0) {
        return r;
    }
    wr8(StageFlashActive, 0);
    ClearObject(r.x);
    return r;
}

void abi_StageFlashEnd(void)
{
    stage_regs_load_shim(StageFlashEnd(regs_entry_shim()));
}

/*
 * @routine 0259:6A3 StagePickupPotion
 * @run 70:A6A3
 * WHAT IT DOES: The potion taken: sound $0B, the player's invincibility timer $80, the object cleared.
 * HOW: The sound hook, a store, the tail JMP to ClearObject.
 * WAITS: no.
 * Entry: X object. Exit: A $80, N and Z from it; X, Y, C and V as the sound left them.
 */
StageRegs StagePickupPotion(StageRegs r)
{
    r = sound_jsr(r, 0x0B, 0xA6A8);
    r = regs_loaded(r, 0x80);
    wr8(PlayerInvincibleTimer, 0x80);
    ClearObject(r.x);
    return r;
}

void abi_StagePickupPotion(void)
{
    stage_regs_load_shim(StagePickupPotion(regs_entry_shim()));
}

/*
 * @routine 025C:126 StageObjectRiseToTop
 * @run 70:B926
 * WHAT IT DOES: Moves the object up a pixel while its Y low byte is $30 or more.
 * HOW: CMP, DEC.
 * WAITS: no.
 * Entry: X object. Exit: A its Y low, C 1 when it moved, N and Z from the compare or the DEC.
 */
StageRegs StageObjectRiseToTop(StageRegs r)
{
    r = regs_loaded(r, RD8(OBJ->y_lo[r.x]));
    r = regs_compared(r, r.a, 0x30);
    if (r.c) {
        r.nz = DEC8(OBJ->y_lo[r.x]);
    }
    return r;
}

void abi_StageObjectRiseToTop(void)
{
    stage_regs_load_shim(StageObjectRiseToTop(regs_entry_shim()));
}

/*
 * @routine 0250:29D StageGateAdvance
 * @run 6B:9A9D
 * WHAT IT DOES: Unless zField8C is negative, advances the object's state with $04 in its OBJ_GATE_WAIT, twice for the
 * start arguments whose low nibble is 2 to 5, $0A or $0B.
 * HOW: CMP chain on OBJ_START_ARG's low nibble.
 * WAITS: no.
 * Entry: X object. Exit: negative: A zField8C; else A the nibble, N, Z and C from the last compare or INC.
 */
StageRegs StageGateAdvance(StageRegs r)
{
    uint8_t value;
    bool again;

    r = regs_loaded(r, RD8(ZP->field8c));
    if (r.a & 0x80) {
        return r;
    }
    r.nz = INC8(OBJ->state[r.x]);
    r = regs_loaded(r, 0x04);
    WR8(OBJ->gate_wait[r.x], 0x04);
    value = (uint8_t)(RD8(OBJ->start_arg[r.x]) & 0x0F);
    r = regs_loaded(r, value);
    r = regs_compared(r, value, 0x02);
    again = false;
    if (r.c) {
        r = regs_compared(r, value, 0x06);
        if (!r.c) {
            again = true;
        } else {
            r = regs_compared(r, value, 0x0A);
            if (value == 0x0A) {
                again = true;
            } else {
                r = regs_compared(r, value, 0x0B);
                again = value == 0x0B;
            }
        }
    }
    if (again) {
        r.nz = INC8(OBJ->state[r.x]);
    }
    return r;
}

void abi_StageGateAdvance(void)
{
    stage_regs_load_shim(StageGateAdvance(regs_entry_shim()));
}

/* A JSR at ret to StartListChannelKind2 on channel 5 with list A, X kept on the stack around it. */
static Regs start_list_5(Regs r, uint16_t ret)
{
    uint8_t value;

    push8(r.x);
    r.x = 5;
    jsr_push(ret);
    value = StartListChannelKind2(r.a, 5);
    jsr_pop();
    r.a = value;
    value = pull8();
    r.x = value;
    r.nz = value;
    return r;
}

/* The gate's frame by its side (the start argument's bit 0) and step, and that frame's list. */
static Regs gate_frame(Regs r, uint16_t list_ret)
{
    uint8_t side;
    Sum sum;

    r = regs_loaded(r, 0x04);
    WR8(OBJ->gate_wait[r.x], 0x04);
    side = (uint8_t)(RD8(OBJ->start_arg[r.x]) & 0x01);
    r.y = side;
    sum = add_bytes(rd8((uint16_t)(StageGateFrames + side)), RD8(OBJ->gate_step[r.x]), false);
    WR8(OBJ->frame[r.x], sum.value);
    sum = add_bytes(sum.value, rd8((uint16_t)(StageGateLists + side)), false);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    return start_list_5(r, list_ret);
}

/*
 * @routine 0250:1DB StageGateShow
 * @run 6B:99DB
 * WHAT IT DOES: Sets a gate up: its object record loaded and its frame by side.
 * HOW: LoadObjectRecord; StageGateFrames by the start argument's bit 0.
 * WAITS: no.
 * Entry: X object. Exit: A the frame, N and Z from it, Y the side; C as the record load left it, V and X kept.
 */
StageRegs StageGateShow(StageRegs r)
{
    uint8_t last, state;
    bool carry;

    stage_regs_load_shim(r);
    jsr_push(0x99DE);
    last = LoadObjectRecord(r.x, &state, &carry);
    jsr_pop();
    load_record_exit_shim(last, state, carry);
    r = stage_regs_from_cpu_shim(r);
    r.y = (uint8_t)(RD8(OBJ->start_arg[r.x]) & 0x01);
    r = regs_loaded(r, rd8((uint16_t)(StageGateFrames + r.y)));
    WR8(OBJ->frame[r.x], r.a);
    return r;
}

void abi_StageGateShow(void)
{
    stage_regs_load_shim(StageGateShow(regs_entry_shim()));
}

/*
 * @routine 0250:255 StageGateWaitRoom
 * @run 6B:9A55
 * WHAT IT DOES: Waits for the stage's step 2 (zField85), then advances the gate's state.
 * HOW: CMP, INC.
 * WAITS: no.
 * Entry: X object. Exit: A zField85; C and Z from the compare, or N and Z from the INC.
 */
StageRegs StageGateWaitRoom(StageRegs r)
{
    r = regs_loaded(r, RD8(ZP->field85));
    r = regs_compared(r, r.a, 0x02);
    if (r.a == 0x02) {
        r.nz = INC8(OBJ->state[r.x]);
    }
    return r;
}

void abi_StageGateWaitRoom(void)
{
    stage_regs_load_shim(StageGateWaitRoom(regs_entry_shim()));
}

/*
 * @routine 0250:25F StageGateRise
 * @run 6B:9A5F
 * WHAT IT DOES: Opens a gate a step every four frames; at the fourth step the gate's state advances, bit 7 of zField8C
 * is set and the sound $71 goes out.
 * HOW: OBJ_GATE_WAIT counts the frames, OBJ_GATE_STEP the steps; the frame and its list as gate_frame.
 * WAITS: no.
 * Entry: X object. Exit: waiting: N and Z from the DEC; the last step as SoundCommand; else as the list start, X
 * kept.
 */
StageRegs StageGateRise(StageRegs r)
{
    r.nz = DEC8(OBJ->gate_wait[r.x]);
    if (r.nz != 0) {
        return r;
    }
    r.nz = INC8(OBJ->gate_step[r.x]);
    r = regs_loaded(r, RD8(OBJ->gate_step[r.x]));
    r = regs_compared(r, r.a, 0x04);
    if (r.a != 0x04) {
        return gate_frame(r, 0x9A9B);
    }
    r.nz = INC8(OBJ->state[r.x]);
    r = regs_loaded(r, (uint8_t)(RD8(ZP->field8c) | 0x80));
    WR8(ZP->field8c, r.a);
    r = regs_loaded(r, 0x71);
    return sound_tail(r, 0x71);
}

void abi_StageGateRise(void)
{
    stage_regs_load_shim(StageGateRise(regs_entry_shim()));
}

/*
 * @routine 0250:2C4 StageGateFall
 * @run 6B:9AC4
 * WHAT IT DOES: Closes a gate a step every four frames; past step 0 the gate's state advances with the sound $71.
 * HOW: As StageGateRise, counting the step down.
 * WAITS: no.
 * Entry: X object. Exit: as StageGateRise.
 */
StageRegs StageGateFall(StageRegs r)
{
    r.nz = DEC8(OBJ->gate_wait[r.x]);
    if (r.nz != 0) {
        return r;
    }
    r.nz = DEC8(OBJ->gate_step[r.x]);
    if (!(r.nz & 0x80)) {
        return gate_frame(r, 0x9AF5);
    }
    r.nz = INC8(OBJ->state[r.x]);
    r = regs_loaded(r, 0x71);
    return sound_tail(r, 0x71);
}

void abi_StageGateFall(void)
{
    stage_regs_load_shim(StageGateFall(regs_entry_shim()));
}

/*
 * @routine 025C:0B6 StageDropPickup
 * @run 70:B8B6
 * WHAT IT DOES: Orders the pickup in slot $50: if one of kind $29 is there, order 2; else, unless this object has
 * OBJ_SPAWN_HELD, a kind $29 started at this object's place with order 1.
 * HOW: ClearObjectInY and StartObjectAtPlace with Y $50.
 * WAITS: no.
 * Entry: X object. Exit: Y $50; ordering: A the order, N and Z from it, C 1 when slot $50 held kind $29; held: A the
 * hold.
 */
StageRegs StageDropPickup(StageRegs r)
{
    uint8_t kind;
    uint8_t placed;

    r.y = 0x50;
    kind = RD8(OBJ->kind[0x50]);
    r = regs_loaded(r, kind);
    r = regs_compared(r, kind, 0x29);
    if (kind == 0x29) {
        r = regs_loaded(r, 0x02);
        WR8(OBJ->child_order[r.y], 0x02);
        return r;
    }
    r = regs_loaded(r, RD8(OBJ->spawn_held[r.x]));
    if (r.a != 0) {
        return r;
    }
    jsr_push(0xB8C7);
    ClearObjectInY(r.y);
    jsr_pop();
    r = regs_loaded(r, 0x29);
    stage_regs_load_shim(r);
    jsr_push(0xB8CC);
    placed = StartObjectAtPlace(0x29, r.x, r.y);
    jsr_pop();
    copy_place_exit_shim(placed);
    r = stage_regs_from_cpu_shim(r);
    r = regs_loaded(r, 0x01);
    WR8(OBJ->child_order[r.y], 0x01);
    return r;
}

void abi_StageDropPickup(void)
{
    stage_regs_load_shim(StageDropPickup(regs_entry_shim()));
}

/*
 * @routine 025C:092 StageDropStart
 * @run 70:B892
 * WHAT IT DOES: Starts the thing flying: the pickup ordered, list $83 on channel 5, no hitbox, its animation reset,
 * speed 3 along X, the next substate.
 * HOW: StageDropPickup, StartListChannelKind2 with X kept, stores.
 * WAITS: no.
 * Entry: X object. Exit: A 3, N and Z from the substate's INC, X kept.
 */
StageRegs StageDropStart(StageRegs r)
{
    jsr_push(0xB895);
    r = StageDropPickup(r);
    jsr_pop();
    r.a = 0x83;
    push8(r.x);
    r = regs_loaded(r, 0x83);
    r.x = 5;
    r.nz = 5;
    jsr_push(0xB89D);
    r.a = StartListChannelKind2(0x83, 5);
    jsr_pop();
    r.x = pull8();
    WR8(OBJ->hitbox_on[r.x], 0);
    WR8(OBJ->anim_step[r.x], 0);
    WR8(OBJ->anim_timer[r.x], 0);
    r = regs_loaded(r, 0x03);
    WR8(OBJ->vx_hi[r.x], 0x03);
    WR8(OBJ->vx_lo[r.x], 0);
    WR8(OBJ->fly_phase[r.x], 0);
    r.nz = INC8(OBJ->substate[r.x]);
    return r;
}

void abi_StageDropStart(void)
{
    stage_regs_load_shim(StageDropStart(regs_entry_shim()));
}

/* The frame list at `list` in zTemp10-11, stepped by StepObjectFrames at a JSR ending at ret. */
static Regs step_frames(Regs r, uint16_t list, uint16_t ret)
{
    FrameStep step;
    uint8_t y = r.y;

    r = regs_loaded(r, (uint8_t)list);
    WR8(ZP->temp10, (uint8_t)list);
    r = regs_loaded(r, (uint8_t)(list >> 8));
    WR8(ZP->temp11, (uint8_t)(list >> 8));
    stage_regs_load_shim(r);
    jsr_push(ret);
    step = StepObjectFrames(r.x, &y);
    jsr_pop();
    step_frames_exit_shim(step, y);
    return stage_regs_from_cpu_shim(r);
}

/* OBJ_VX less `less`, low byte then high. */
static Regs slow_x(Regs r, uint8_t less)
{
    Sum sum;

    sum = add_bytes(RD8(OBJ->vx_lo[r.x]), (uint8_t)~less, true);
    WR8(OBJ->vx_lo[r.x], sum.value);
    sum = add_bytes(RD8(OBJ->vx_hi[r.x]), 0xFF, sum.carry);
    r = regs_loaded(r, sum.value);
    r.c = sum.carry;
    r.v = sum.overflow;
    WR8(OBJ->vx_hi[r.x], sum.value);
    return r;
}

/*
 * @routine 025C:0D8 StageDropFly
 * @run 70:B8D8
 * WHAT IT DOES: The flying thing: its frames stepped, moved along X and up to the top, slowed by $18 a frame; once
 * it flies back past X $E0 on the first screen: the sound $73, its animation reset, the next substate and a wait.
 * HOW: StepObjectFrames on the list at $B921, ApplyVelocityX, StageObjectRiseToTop, the two-byte subtract.
 * WAITS: no.
 * Entry: X object. Exit: as the last compare, subtract or INC.
 */
StageRegs StageDropFly(StageRegs r)
{
    AxisMove moved;

    r = step_frames(r, 0xB921, 0xB8E3);
    jsr_push(0xB8E6);
    moved = ApplyVelocityX(r.x);
    jsr_pop();
    r = moved_regs(r, moved);
    jsr_push(0xB8E9);
    r = StageObjectRiseToTop(r);
    jsr_pop();
    r = slow_x(r, 0x18);
    if (!(r.a & 0x80)) {
        return r;
    }
    r = regs_loaded(r, RD8(OBJ->x_hi[r.x]));
    if (r.a != 0) {
        return r;
    }
    r = regs_loaded(r, RD8(OBJ->x_lo[r.x]));
    r = regs_compared(r, r.a, 0xE0);
    if (r.c) {
        return r;
    }
    r = sound_jsr(r, 0x73, 0xB90D);
    WR8(OBJ->anim_step[r.x], 0);
    WR8(OBJ->anim_timer[r.x], 0);
    r = regs_loaded(r, 0x04);
    WR8(OBJ->fly_phase[r.x], 0x04);
    r = regs_loaded(r, 0xB0);
    WR8(OBJ->flyer_wait[r.x], 0xB0);
    r.nz = INC8(OBJ->substate[r.x]);
    return r;
}

void abi_StageDropFly(void)
{
    stage_regs_load_shim(StageDropFly(regs_entry_shim()));
}

/*
 * @routine 025C:4FD StageFlyerSlow
 * @run 70:BCFD
 * WHAT IT DOES: A flyer's frames stepped, its X speed less $30, and moved.
 * HOW: StepObjectFrames on the list at $BB69, the subtract, the tail JMP to ApplyVelocity.
 * WAITS: no.
 * Entry: X object. Exit: as ApplyVelocity.
 */
StageRegs StageFlyerSlow(StageRegs r)
{
    AxisMove moved;

    r = step_frames(r, 0xBB69, 0xBD08);
    r = slow_x(r, 0x30);
    moved = ApplyVelocity(r.x);
    return moved_regs(r, moved);
}

void abi_StageFlyerSlow(void)
{
    stage_regs_load_shim(StageFlyerSlow(regs_entry_shim()));
}

/*
 * @routine 0259:6DB StagePickupExtraLife
 * @run 70:A6DB
 * WHAT IT DOES: The 1-up taken: the object becomes the score marker, zField90 1, the sound $52, one added to the stock
 * in decimal (held at 99), and the stock rows blended.
 * HOW: SED, CLC, ADC #1, CLD; BCC; the tail JMP to $9151 (BlendStageStockRows).
 * WAITS: no.
 * Entry: X object. Exit: as the blend.
 */
StageRegs StagePickupExtraLife(StageRegs r)
{
    Alu sum;

    r = regs_loaded(r, 0x9F);
    WR8(OBJ->kind[r.x], 0x9F);
    r = regs_loaded(r, 0x04);
    WR8(OBJ->marker_value[r.x], 0x04);
    WR8(OBJ->state[r.x], 0);
    r = regs_loaded(r, 0x01);
    WR8(ZP->field90, 0x01);
    r = sound_jsr(r, 0x52, 0xA6F1);
    sum = adc8_bcd(RD8(ZP->stock), 0x01, false);
    r = regs_loaded(r, sum.a);
    r.c = sum.c;
    WR8(ZP->stock, sum.a);
    if (r.c) {
        r = regs_loaded(r, 0x99);
        WR8(ZP->stock, 0x99);
    }
    stage_regs_load_shim(r);
    run_routine_from(0xA700, 0x9151);
    return stage_regs_from_cpu_shim(r);
}

void abi_StagePickupExtraLife(void)
{
    stage_regs_load_shim(StagePickupExtraLife(regs_entry_shim()));
}

/*
 * @routine 025B:0ED StageScrollTableVector
 * @run 70:B0ED
 * @stop B0F0
 * WHAT IT DOES: Goes to StepVerticalScrollTable.
 * HOW: One jump.
 * WAITS: no.
 * Entry, exit: as the routine at $B0F0.
 */
void abi_StageScrollTableVector(void)
{
    run_routine_from(0xB0ED, 0xB0F0);
}
