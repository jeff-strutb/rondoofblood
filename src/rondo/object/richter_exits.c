/*
 * The exit log of Richter's overlay routines (richter.h): each step is what an instruction or a
 * callee left in the registers and flags, kept in the order the assembly ran them, and replayed in
 * that order by the routine's abi_ wrapper. No routine of the original is here: this is the C's
 * bookkeeping for the registers a caller sees.
 */
#include "machine.h"
#include "ram.h"
#include "routines.h"
#include "access.h"
#include "enemy_objects.h"
#include "player_objects.h"
#include "richter.h"

static RegStep *rl_next(RegLog *log, RegKind kind)
{
    RegStep *s = &log->step[log->count < 24 ? log->count++ : 23];

    s->kind = kind;
    return s;
}

void rl_ended(RegLog *log, Ended e)
{
    RegStep *s = rl_next(log, RX_ENDED);

    s->ended.e = e;
    s->ended.x_set = false;
    s->ended.x = 0;
}

void rl_x(RegLog *log, uint8_t x)
{
    rl_ended_x(log, ended_kept(), x);
}

void rl_ended_x(RegLog *log, Ended e, uint8_t x)
{
    RegStep *s = rl_next(log, RX_ENDED);

    s->ended.e = e;
    s->ended.x_set = true;
    s->ended.x = x;
}

/* TST #mask, memory: Z from the AND, N and V the memory's bits 7 and 6; A and C stand. */
void rl_tst(RegLog *log, uint8_t mask, uint8_t memory)
{
    RegStep *s = rl_next(log, RX_TST);

    s->tst.mask = mask;
    s->tst.memory = memory;
}

void rl_columns(RegLog *log, ColumnsEnd r) { rl_next(log, RX_COLUMNS)->columns = r; }
void rl_floor(RegLog *log, FloorResult r) { rl_next(log, RX_FLOOR)->floor = r; }
void rl_landing(RegLog *log, LandingLift r) { rl_next(log, RX_LANDING)->landing = r; }
void rl_animation(RegLog *log, AnimationStep r) { rl_next(log, RX_ANIMATION)->animation = r; }
void rl_script(RegLog *log, uint8_t script) { rl_next(log, RX_SCRIPT)->script = script; }
void rl_lift(RegLog *log, Lift r) { rl_next(log, RX_LIFT)->lift = r; }
void rl_nudge(RegLog *log, Nudge r) { rl_next(log, RX_NUDGE)->nudge = r; }
void rl_fall(RegLog *log, AnimationStep r) { rl_next(log, RX_FALL)->animation = r; }
void rl_gravity(RegLog *log, Sum r) { rl_next(log, RX_GRAVITY)->gravity = r; }
void rl_crash_end(RegLog *log, CrashEnd r) { rl_next(log, RX_CRASH_END)->crash_end = r; }
void rl_column(RegLog *log, ColumnRead r) { rl_next(log, RX_COLUMN_READ)->column = r; }

void rl_solid(RegLog *log, ColumnRead r) { rl_next(log, RX_SOLID)->column = r; }
void rl_hit_effects(RegLog *log, HitEffects r) { rl_next(log, RX_HIT_EFFECTS)->hit_effects = r; }
void rl_hit_sound(RegLog *log, HitSound r) { rl_next(log, RX_HIT_SOUND)->hit_sound = r; }
void rl_slots(RegLog *log, Ended r) { rl_next(log, RX_SLOTS)->slots = r; }
void rl_debris(RegLog *log, DebrisColumns r) { rl_next(log, RX_DEBRIS)->debris = r; }

void rl_sound(RegLog *log, uint8_t command, CommandCall r)
{
    RegStep *s = rl_next(log, RX_SOUND);

    s->sound.command = command;
    s->sound.call = r;
}

void reg_log_replay(const RegLog *log)
{
    for (uint8_t i = 0; i < log->count; i++) {
        const RegStep *s = &log->step[i];

        switch (s->kind) {
        case RX_ENDED:
            abi_ended(s->ended.e);
            if (s->ended.x_set) {
                cpu.x = s->ended.x;
            }
            break;
        case RX_TST:
            abi_flag(P_Z, (s->tst.mask & s->tst.memory) == 0);
            abi_flag(P_N, (s->tst.memory & 0x80) != 0);
            abi_flag(P_V, (s->tst.memory & 0x40) != 0);
            break;
        case RX_COLUMNS:
            columns_exit_shim(s->columns);
            break;
        case RX_FLOOR:
            floor_check_exit_shim(s->floor);
            break;
        case RX_LANDING:
            landing_exit_shim(s->landing);
            break;
        case RX_ANIMATION:
            animation_exit_shim(s->animation);
            break;
        case RX_SOUND:
            cpu.a = s->sound.command;
            sound_command_exit_shim(s->sound.command, s->sound.call);
            break;
        case RX_SCRIPT:
            hit_script_exit_shim(s->script);
            break;
        case RX_LIFT:
            lift_exit_shim(s->lift);
            break;
        case RX_NUDGE:
            nudge_exit_shim(s->nudge);
            break;
        case RX_FALL:
            to_fall_exit_shim(s->animation);
            break;
        case RX_GRAVITY:
            fall_gravity_exit_shim(s->gravity);
            break;
        case RX_CRASH_END:
            crash_end_exit_shim(s->crash_end);
            break;
        case RX_COLUMN_READ:
            column_read_exit_shim(s->column);
            break;
        case RX_SOLID:
            column_read_exit_shim(s->column);
            cpu.a = solid_answer(s->column);
            abi_nz(cpu.a);
            break;
        case RX_HIT_EFFECTS:
            hit_effects_exit_shim(s->hit_effects);
            break;
        case RX_HIT_SOUND:
            hit_sound_exit_shim(s->hit_sound);
            break;
        case RX_SLOTS:
            abi_ended(s->slots);
            cpu.x = 2;
            break;
        case RX_DEBRIS:
            debris_columns_exit_shim(s->debris);
            break;
        case RX_LOG:
            break;
        }
    }
}

void rl_xy(const RegLog *log, uint8_t x_in, uint8_t y_in, uint8_t *x, uint8_t *y)
{
    Cpu saved = cpu;

    cpu.x = x_in;
    cpu.y = y_in;
    reg_log_replay(log);
    *x = cpu.x;
    *y = cpu.y;
    cpu = saved;
}
