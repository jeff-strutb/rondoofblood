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

void rl_note_press(RegLog *log, ObjArm r) { rl_next(log, RX_NOTE_PRESS)->arm = r; }
void rl_item_use(RegLog *log, ItemUse r) { rl_next(log, RX_ITEM_USE)->use = r; }
void rl_under_feet(RegLog *log, ColumnRead r) { rl_next(log, RX_UNDER_FEET)->column = r; }
void rl_column_lift(RegLog *log, ColumnLift r) { rl_next(log, RX_COLUMN_LIFT)->column_lift = r; }
void rl_attack(RegLog *log, AttackStart r) { rl_next(log, RX_ATTACK)->attack = r; }
void rl_knock(RegLog *log, Knock r) { rl_next(log, RX_KNOCK)->knock = r; }
void rl_marked(RegLog *log, MarkedFloor r) { rl_next(log, RX_MARKED)->marked = r; }
void rl_footing(RegLog *log, FootingCheck r) { rl_next(log, RX_FOOTING)->footing = r; }
void rl_ledge(RegLog *log, LedgeCheck r) { rl_next(log, RX_LEDGE)->ledge = r; }
void rl_hurt_land(RegLog *log, HurtLandSound r) { rl_next(log, RX_HURT_LAND)->hurt_land = r; }
void rl_hurt_resume(RegLog *log, HurtResume r) { rl_next(log, RX_HURT_RESUME)->hurt_resume = r; }

void rl_move_stopped(RegLog *log, Sum moved, uint8_t facing)
{
    RegStep *s = rl_next(log, RX_MOVE_STOPPED);

    s->move_stopped.moved = moved;
    s->move_stopped.facing = facing;
}

void rl_read_below(RegLog *log, TileRead r, bool read_made)
{
    RegStep *s = rl_next(log, RX_READ_BELOW);

    s->read_below.read = r;
    s->read_below.read_made = read_made;
}

void rl_offset_read(RegLog *log, OffsetRead r) { rl_next(log, RX_OFFSET_READ)->offset_read = r; }
void rl_item_crash(RegLog *log, ItemCrash r) { rl_next(log, RX_ITEM_CRASH)->item_crash = r; }
void rl_ledge_hurt(RegLog *log, LedgeHurt r) { rl_next(log, RX_LEDGE_HURT)->ledge_hurt = r; }
void rl_up_whip(RegLog *log, UpWhip r) { rl_next(log, RX_UP_WHIP)->up_whip = r; }
void rl_jump_climb(RegLog *log, JumpOrClimb r) { rl_next(log, RX_JUMP_CLIMB)->jump_climb = r; }
void rl_turn_pad(RegLog *log, ObjArm r) { rl_next(log, RX_TURN_PAD)->arm = r; }
void rl_crash_frame(RegLog *log, CrashFrame r) { rl_next(log, RX_CRASH_FRAME)->crash_frame = r; }
void rl_speed(RegLog *log, FacingSpeed r) { rl_next(log, RX_SPEED)->speed = r; }
void rl_air(RegLog *log, AirStart r) { rl_next(log, RX_AIR)->air = r; }
void rl_crash_stand(RegLog *log, CrashStand r) { rl_next(log, RX_CRASH_STAND)->crash_stand = r; }
void rl_up_press(RegLog *log, UpPressUse r) { rl_next(log, RX_UP_PRESS)->up_press = r; }
void rl_jump_pressed(RegLog *log, bool r) { rl_next(log, RX_JUMP_PRESSED)->jump_pressed = r; }
void rl_countdown(RegLog *log, CountDown r) { rl_next(log, RX_COUNTDOWN)->countdown = r; }

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
        case RX_NOTE_PRESS:
            abi_ended_tst(richter_note_item_press_exit_shim(s->arm));
            break;
        case RX_ITEM_USE:
            item_use_exit_shim(s->use);
            break;
        case RX_UNDER_FEET:
            under_feet_exit_shim(s->column);
            break;
        case RX_COLUMN_LIFT:
            column_lift_exit_shim(s->column_lift);
            break;
        case RX_MOVE_STOPPED:
            cpu.a = 0xFF;
            cpu.x = s->move_stopped.facing;
            abi_nz(s->move_stopped.facing);
            abi_flag(P_C, s->move_stopped.moved.carry);
            abi_flag(P_V, s->move_stopped.moved.overflow);
            break;
        case RX_READ_BELOW:
            read_or_nothing_exit_shim(s->read_below.read, s->read_below.read_made);
            break;
        case RX_ATTACK:
            attack_exit_shim(s->attack);
            break;
        case RX_KNOCK:
            knock_exit_shim(s->knock);
            break;
        case RX_MARKED:
            marked_floor_exit_shim(s->marked);
            break;
        case RX_FOOTING:
            footing_exit_shim(s->footing);
            break;
        case RX_LEDGE:
            ledge_check_exit_shim(s->ledge);
            break;
        case RX_HURT_LAND:
            hurt_land_sound_exit_shim(s->hurt_land);
            break;
        case RX_HURT_RESUME:
            hurt_resume_exit_shim(s->hurt_resume);
            break;
        case RX_OFFSET_READ:
            offset_read_exit_shim(s->offset_read);
            break;
        case RX_ITEM_CRASH:
            item_crash_exit_shim(s->item_crash);
            break;
        case RX_LEDGE_HURT:
            ledge_hurt_exit_shim(s->ledge_hurt);
            break;
        case RX_UP_WHIP:
            up_whip_exit_shim(s->up_whip);
            break;
        case RX_JUMP_CLIMB:
            jump_or_climb_exit_shim(s->jump_climb);
            break;
        case RX_TURN_PAD:
            abi_ended_tst(richter_turn_on_pad_exit_shim(s->arm));
            break;
        case RX_CRASH_FRAME:
            crash_frame_exit_shim(s->crash_frame);
            break;
        case RX_SPEED:
            speed_exit_shim(s->speed);
            break;
        case RX_AIR:
            air_exit_shim(s->air);
            break;
        case RX_CRASH_STAND:
            crash_stand_exit_shim(s->crash_stand);
            break;
        case RX_UP_PRESS:
            up_press_use_exit_shim(s->up_press);
            break;
        case RX_JUMP_PRESSED:
            abi_flag(P_C, s->jump_pressed);
            break;
        case RX_COUNTDOWN:
            count_down_stand_exit_shim(s->countdown);
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
