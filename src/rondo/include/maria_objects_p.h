/*
 * Maria's object handlers that hold the processor (maria_objects_kind*.c): the register file is `cpu`, set where the
 * assembly sets a register, and each JSR to a typed callee goes through one of the pj_ helpers below, which frame the
 * call with the disc's return address, give the callee the registers it takes from `cpu`, and give its exit back to
 * `cpu` as its own wrapper does. A tail JMP is the same without the frame (pt_ helpers: ret 0).
 */
#ifndef RONDO_MARIA_OBJECTS_P_H
#define RONDO_MARIA_OBJECTS_P_H

#include "maria_objects.h"
#include "richter.h"

/* LDA and the like: A, with N and Z from it. */
static inline void p_lda(uint8_t value)
{
    cpu.a = value;
    abi_nz(value);
}

static inline void p_ldx(uint8_t value)
{
    cpu.x = value;
    abi_nz(value);
}

static inline void p_ldy(uint8_t value)
{
    cpu.y = value;
    abi_nz(value);
}

/* ADC, SBC: A, N and Z, C and V. */
static inline void p_sum(Sum s)
{
    p_lda(s.value);
    abi_flag(P_C, s.carry);
    abi_flag(P_V, s.overflow);
}

static inline void p_adc(uint8_t operand)
{
    p_sum(add_bytes(cpu.a, operand, (cpu.p & P_C) != 0));
}

static inline void p_sbc(uint8_t operand)
{
    p_sum(sub_bytes(cpu.a, operand, (cpu.p & P_C) != 0));
}

/* CMP, CPX, CPY. */
static inline void p_cmp(uint8_t reg, uint8_t operand)
{
    abi_nz((uint8_t)(reg - operand));
    abi_flag(P_C, reg >= operand);
}

/* TST #mask, memory. */
static inline void p_tst(uint8_t mask, uint8_t memory)
{
    abi_nz((mask & memory) == 0 ? 0 : (uint8_t)((memory & 0x80) | 0x01));
    abi_flag(P_N, (memory & 0x80) != 0);
    abi_flag(P_V, (memory & 0x40) != 0);
}

/* ASL A, LSR A. */
static inline void p_asl(void)
{
    bool c = (cpu.a & 0x80) != 0;

    p_lda((uint8_t)(cpu.a << 1));
    abi_flag(P_C, c);
}

static inline void p_lsr(void)
{
    bool c = (cpu.a & 0x01) != 0;

    p_lda((uint8_t)(cpu.a >> 1));
    abi_flag(P_C, c);
}

static inline bool p_c(void)
{
    return (cpu.p & P_C) != 0;
}

static inline bool p_z(void)
{
    return (cpu.p & P_Z) != 0;
}

static inline bool p_n(void)
{
    return (cpu.p & P_N) != 0;
}

/* LDX zRichterObject: the object being run. */
static inline uint8_t p_ldx_object(void)
{
    p_ldx(RD8(ZP->richter_object));
    return cpu.x;
}

/* The frame of a JSR (ret the address after it), or none for a tail JMP (ret 0). */
static inline void pj_in(uint16_t ret)
{
    if (ret != 0) {
        jsr_push(ret);
    }
}

static inline void pj_out(uint16_t ret)
{
    if (ret != 0) {
        jsr_pop();
    }
}

/* Defined elsewhere, typed, and not declared in a header these files include. */
Sum MariaObjectAgainstScroll(uint8_t object);                        /* object/richter_objects.c */
uint8_t MariaObjectSlot(void);                                       /* object/richter_objects.c */
void MariaObjectProbe(void);                                         /* object/maria_objects_probe.c */
void MariaObjectProbeAt(void);                                       /* object/maria_objects_probe.c */
void MariaObjectProbeAtBit1(void);                                   /* object/maria_objects_probe.c */
void MariaObjectProbeAtSolid(void);                                  /* object/maria_objects_probe.c */
void MariaObjectProbeBelow(void);                                    /* object/maria_objects_probe.c */
void MariaObjectProbeBelowAny(void);                                 /* object/maria_objects_probe.c */
void MariaObjectAtPlayerPlace(void);                                 /* object/maria_objects_slots.c */
void MariaObjectSetFall(void);                                       /* object/maria_objects_items.c */
ObjArm MariaStateDoubled(void);                                      /* object/player_leaves.c */

static inline void pj_velocity_y(uint16_t ret)
{
    pj_in(ret);
    regs_exit(regs_of_velocity_table(MariaSetVelocityYFromTable(cpu.x, cpu.a)));
    pj_out(ret);
}

static inline void pj_velocity_x(uint16_t ret)
{
    pj_in(ret);
    regs_exit(regs_of_velocity_table(MariaSetVelocityXFromTable(cpu.x, cpu.a)));
    pj_out(ret);
}

static inline void pj_fall_capped(uint16_t ret)
{
    pj_in(ret);
    abi_ended(MariaObjectFallCapped(cpu.x));
    pj_out(ret);
}

static inline void pj_move(uint16_t ret)
{
    pj_in(ret);
    abi_ended(ended_of_axis_move(MariaObjectMove(cpu.x)));
    pj_out(ret);
}

static inline void pj_apply_velocity(uint16_t ret)
{
    pj_in(ret);
    abi_ended(ended_of_axis_move(MariaObjectApplyVelocity(cpu.x)));
    pj_out(ret);
}

static inline void pj_facing_velocity(uint16_t ret)
{
    pj_in(ret);
    abi_ended(MariaObjectFacingVelocity(cpu.x));
    pj_out(ret);
}

static inline void pj_accelerate(uint16_t ret)
{
    pj_in(ret);
    abi_ended(MariaObjectAccelerate(cpu.x));
    pj_out(ret);
}

static inline void pj_near_player(uint16_t ret)
{
    pj_in(ret);
    abi_ended(MariaObjectNearPlayer(cpu.x));
    pj_out(ret);
}

static inline void pj_mark_on(uint16_t ret)
{
    pj_in(ret);
    p_lda(MariaObjectMarkOn(cpu.x));
    pj_out(ret);
}

static inline void pj_mark_off(uint16_t ret)
{
    pj_in(ret);
    regs_exit(MariaObjectMarkOff(cpu.x));
    pj_out(ret);
}

static inline void pj_from_player(uint16_t ret)
{
    ObjectFromPlayer from;

    pj_in(ret);
    from = MariaObjectFromPlayer(cpu.x);
    p_lda(from.facing);
    abi_flag(P_C, (from.player_2fe8 & 0x80) != 0);
    pj_out(ret);
}

static inline void pj_at_player(uint16_t ret)
{
    pj_in(ret);
    p_sum(MariaObjectAtPlayer(cpu.x));
    pj_out(ret);
}

static inline void pj_against_scroll(uint16_t ret)
{
    pj_in(ret);
    p_sum(MariaObjectAgainstScroll(cpu.x));
    pj_out(ret);
}

static inline void pj_next_state(uint16_t ret)
{
    pj_in(ret);
    abi_nz(MariaObjectNextState(cpu.x));
    pj_out(ret);
}

/* MariaStateDoubled: A and X the doubled state of zRichterObject's object, N and Z from it, C its bit 7. */
static inline void pj_state_doubled(uint16_t ret)
{
    ObjArm r;

    pj_in(ret);
    r = MariaStateDoubled();
    p_lda(r.first);
    abi_flag(P_C, r.flag);
    cpu.x = r.first;
    pj_out(ret);
}

static inline void pj_animation_tick(uint16_t ret)
{
    pj_in(ret);
    animation_exit_shim(MariaObjectAnimationTick(cpu.x));
    pj_out(ret);
}

static inline void pj_animation_running(uint16_t ret)
{
    pj_in(ret);
    animation_exit_shim(MariaObjectAnimationRunning(cpu.x));
    pj_out(ret);
}

static inline void pj_set_animation(uint16_t ret)
{
    pj_in(ret);
    animation_exit_shim(MariaObjectSetAnimation(cpu.x, cpu.a));
    pj_out(ret);
}

static inline void pj_start_animation(uint16_t ret)
{
    pj_in(ret);
    mx_exit(MariaObjectStartAnimation(cpu.x, cpu.a));
    pj_out(ret);
}

static inline void pj_clear_object(uint16_t ret)
{
    pj_in(ret);
    ClearObject(cpu.x);
    pj_out(ret);
}

static inline void pj_sound(uint16_t ret)
{
    uint8_t command = cpu.a;
    CommandCall call;

    pj_in(ret);
    call = SoundCommand(command, cpu.x, cpu.y);
    sound_command_exit_shim(command, call);
    pj_out(ret);
}

static inline void pj_hit_script(uint16_t ret)
{
    uint8_t script = cpu.a;

    pj_in(ret);
    MariaStartHitScript(script);
    hit_script_exit_shim(script);
    pj_out(ret);
}

/* The Regs routines of maria_objects.c, each by X (and A where it takes one). */
#define PJ_REGS_X(name, fn) \
    static inline void name(uint16_t ret) { pj_in(ret); regs_exit(fn(cpu.x)); pj_out(ret); }
PJ_REGS_X(pj_hit_by_checked, MariaObjectHitByChecked)
PJ_REGS_X(pj_above_player, MariaObjectAbovePlayer)
PJ_REGS_X(pj_frame_from_player, MariaObjectFrameFromPlayer)
PJ_REGS_X(pj_velocity_from_work, MariaObjectVelocityFromWork)
PJ_REGS_X(pj_far_off_screen, MariaObjectFarOffScreen)
PJ_REGS_X(pj_frame24_next, MariaObjectFrame24Next)
PJ_REGS_X(pj_frame_by_player_frame, MariaObjectFrameByPlayerFrame)
PJ_REGS_X(pj_frame25, MariaObjectFrame25)
PJ_REGS_X(pj_claim_kind3, MariaClaimKind3Slot)

static inline void pj_script_slot3(uint16_t ret)
{
    pj_in(ret);
    regs_exit(MariaStartScriptSlot3(cpu.x, cpu.a));
    pj_out(ret);
}

/* The void steps that keep every register and flag, and the processor-holding routines of the other files. */
static inline void pj_void_x(uint16_t ret, void (*fn)(uint8_t))
{
    pj_in(ret);
    fn(cpu.x);
    pj_out(ret);
}

static inline void pj_proc(uint16_t ret, void (*fn)(void))
{
    pj_in(ret);
    fn();
    pj_out(ret);
}

/*
 * JMP (table,X): the word read from the table at X, lo then hi (RAM reads), for the caller's switch on it. A word
 * that is none of the caller's goes to the routine at it by its address (p_jump_elsewhere).
 */
static inline uint16_t p_table_word(uint16_t table)
{
    uint8_t lo = rd8((uint16_t)(table + cpu.x));
    uint8_t hi = rd8((uint16_t)(table + cpu.x + 1));

    return (uint16_t)(lo | (hi << 8));
}

static inline void p_jump_elsewhere(uint16_t target)
{
    run_routine(target);
}

/* object/maria_objects_kind2.c */
void MariaKind2(void);
void MariaKind2Start(void);
void MariaKind2Walk(void);
void MariaKind2Turn(void);
void MariaKind2Edge(void);
void MariaKind2Land(void);
void MariaKind2State2(void);
void MariaKind2Clear(void);
void MariaKind2Walk2(void);
void MariaKind2Leap(void);
void MariaKind2Drop(void);
void MariaKind2Air(void);
void MariaKind2Step(void);
void MariaKind2State5(void);
void MariaKind2Special(void);

#endif
