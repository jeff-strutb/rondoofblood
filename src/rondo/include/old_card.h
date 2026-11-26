/*
 * The old-card program (system/old_card.c): the 20 sectors from $23 the boot program reads to $4000 and jumps to
 * when the System Card is older than 3.0 or ex_memopen returns carry set (docs/rondo/HANDBOOK.md, Code). Declared
 * here, not in routines.h.
 */
#ifndef RONDO_OLD_CARD_H
#define RONDO_OLD_CARD_H

#include "machine.h"
#include "routines.h"

/*
 * How a routine of the player object (object 0 of this program) ended: through the animation step at $4276 (its
 * FrameStep and the Y it left), at the hurt arm (the hurt byte the arm halved into Y), or through original code no
 * C covers yet (the registers as that code left them). A jump's arms also carry the move it made, whose V the step
 * keeps, and whether the step's A went through the TAY at $47FF.
 */
enum { PLAYER_ANIMATED, PLAYER_HURT, PLAYER_ORIGINAL };

typedef struct {
    int arm;
    FrameStep step;
    uint8_t y;
    uint8_t hurt;
    bool moved;
    AxisMove move;
    bool through_tay;
} PlayerArm;

/* How ScrollWithPlayer ended: the arm, the A it leaves and the value its N and Z come from, the Y it holds where
 * it set Y, and the carry of its last compare where it made one. */
enum {
    FOLLOW_CLAMP_LEFT, FOLLOW_CLAMP_RIGHT, FOLLOW_FRACTION, FOLLOW_CENTRE, FOLLOW_RIGHT_END, FOLLOW_LEFT_END,
    FOLLOW_RECENTRED
};

typedef struct {
    int arm;
    uint8_t a, y, nz;
    bool y_set;
    bool c_set;
    bool carry;
} FollowArm;

/* How StepTitleTimer ended: returned with the timer's low seven bits in A and the carry of their compare with $40,
 * or tail-called the card at $E0E4 (whose exit is the routine's). Y is what the second animation step left. */
enum { TITLE_RETURNED, TITLE_CARD_CALLED };

typedef struct {
    int arm;
    uint8_t a, nz;
    bool carry;
    FrameStep step;
    uint8_t y;
} TitleArm;

uint8_t SetBackgroundScroll(void);
void abi_SetBackgroundScroll(void);
void SelectVdcRegisterOldCard(uint8_t reg);
void abi_SelectVdcRegisterOldCard(void);
FrameStep StartObjectAnimation(uint8_t list, uint8_t object, uint8_t *exit_y);
void abi_StartObjectAnimation(void);
FollowArm ClampObjectLeft(void);
void abi_ClampObjectLeft(void);
FollowArm ClampObjectRight(void);
void abi_ClampObjectRight(void);
FollowArm ScrollWithPlayer(void);
void abi_ScrollWithPlayer(void);
uint8_t SpawnTitleObjects(void);
void abi_SpawnTitleObjects(void);
TitleArm StepTitleTimer(void);
void abi_StepTitleTimer(void);
void InitPlayerObject(void);
void abi_InitPlayerObject(void);
PlayerArm RunPlayerObject(void);
void abi_RunPlayerObject(void);
PlayerArm PlayerStand(uint8_t object);
void abi_PlayerStand(void);
PlayerArm PlayerRestFromWalk(uint8_t object);
void abi_PlayerRestFromWalk(void);
PlayerArm PlayerWalk(uint8_t object);
void abi_PlayerWalk(void);
PlayerArm PlayerJump(uint8_t object);
void abi_PlayerJump(void);

#endif
