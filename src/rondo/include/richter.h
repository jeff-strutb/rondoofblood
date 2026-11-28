/*
 * Richter's player overlay (sector $1B9, run at $A000): the routines the files filed from it in
 * name order declare for each other, beside player_objects.h. The defining file is named at each.
 */
#ifndef RONDO_RICHTER_H
#define RONDO_RICHTER_H

#include "machine.h"
#include "routines.h"
#include "enemy_objects.h"

/* object/richter_state.c */
typedef struct { uint8_t state; uint8_t index; } CrashEnd;
CrashEnd RichterEndItemCrash(void);
void crash_end_exit_shim(CrashEnd end);
void RichterStopMoving(void);
void RichterStopX(void);
void RichterStartHitScript(uint8_t script);
void hit_script_exit_shim(uint8_t script);
void sound_command_exit_shim(uint8_t command, CommandCall call);   /* audio/sound_calls.c */

/* object/richter_objects.c */
void RichterObjectStopX(uint8_t slot);
void RichterObjectStop(uint8_t slot);
void RichterObjectStopY(uint8_t slot);

/* object/richter_hurt.c */
typedef struct { Ended slots; CommandCall sound; } HurtLandSound;
typedef struct { uint8_t side; uint8_t high; } KnockSpeed;
typedef enum { HIT_EFFECT_NONE, HIT_EFFECT_SLOTS, HIT_EFFECT_COUNTED, HIT_EFFECT_MARKED } HitEffectHow;
/* What RichterHitEffects did: the OBJ_HIT_TAKEN its last TST read, the script, the sound. */
typedef struct { HitEffectHow how; uint8_t taken; uint8_t script; uint8_t command; CommandCall sound; } HitEffects;
typedef struct { bool sounded; CommandCall sound; HitEffects effects; } HitSound;
/* RichterHurtResume: the state before the hit and its frame, the count, and what was given back. */
typedef struct {
    bool given_back; bool ended; uint8_t state; uint8_t index; uint8_t frame; uint8_t count;
    uint8_t resumed_state; uint8_t step; CrashEnd end;
} HurtResume;

void RichterClearSlotKindFour(uint8_t slot);
Ended RichterReplaceSlotsOneTwo(void);
HurtLandSound RichterHurtLandSound(uint8_t y);
void hurt_land_sound_exit_shim(HurtLandSound r);
uint8_t RichterKnockSpeedX(uint8_t side);
void knock_speed_x_exit_shim(uint8_t side, uint8_t high);
KnockSpeed RichterKnockSpeedXFromHit(void);
void RichterHurtToCrouch(void);
HitEffects RichterHitEffects(uint8_t x, uint8_t y);
void hit_effects_exit_shim(HitEffects r);
HitSound RichterHitSoundAndEffects(uint8_t x, uint8_t y);
HurtResume RichterHurtResume(void);

/* object/debris_burst.c */
ObjArm RichterStartDebrisBurst(uint8_t slot, uint8_t timer, uint8_t y_entry);
Ended start_debris_burst_exit_shim(ObjArm r);
void abi_RichterStartDebrisBurst(void);

/* object/richter_fall.c */
/* RichterDebrisOnMarkedColumns: whether the burst ran, its result, the V of the Y add, his Y pulled back. */
typedef struct { bool burst_started; ObjArm burst; bool overflow; uint8_t y_low; } DebrisColumns;
DebrisColumns RichterDebrisOnMarkedColumns(uint8_t x, uint8_t y);
void debris_columns_exit_shim(DebrisColumns r);
void abi_RichterDebrisOnMarkedColumns(void);

void abi_RichterObjectStopX(void);
void abi_RichterObjectStop(void);
void abi_RichterObjectStopY(void);
void abi_RichterClearSlotKindFour(void);
void abi_RichterReplaceSlotsOneTwo(void);
void abi_RichterHurtLandSound(void);
void abi_RichterKnockSpeedX(void);
void abi_RichterKnockSpeedXFromHit(void);
void abi_RichterHurtToCrouch(void);
void abi_RichterHitEffects(void);
void abi_RichterHitSoundAndEffects(void);
void abi_RichterHurtResume(void);

#endif
