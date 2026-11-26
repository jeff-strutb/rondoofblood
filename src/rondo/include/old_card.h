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

/* How DrawObjectSprite ended: the object had no parts, the 64 sprites ran out (after a part was written), or the
 * object's parts were all done. The registers it leaves: A, X, Y, the value N and Z come from, and C and V where the
 * arm set them (no parts: C the frame's bit 7, V kept). */
enum { SPRITE_NO_PARTS, SPRITE_OUT_OF_SPRITES, SPRITE_PARTS_DONE };

typedef struct {
    int arm;
    uint8_t a, x, y, nz;
    bool carry;
    bool v_set;
    bool overflow;
} SpriteArm;

/* How DrawSprites ended: out of sprites inside DrawObjectSprite, every sprite used (X 0), or the rest cleared
 * (X $FF). A is the last frame byte read or the last DrawObjectSprite's A; drew says whether any object was drawn,
 * whose last arm gives Y, C and V. */
enum { SPRITES_OUT, SPRITES_ALL_USED, SPRITES_CLEARED };

typedef struct {
    int arm;
    uint8_t a;
    bool drew;
    SpriteArm last;
} SpritesArm;

/* PlayCdTrack: the card's play tried until it answered zero, the last tries' results kept for the exit. */
typedef struct {
    uint8_t dl;
    uint8_t x, y;     /* X and Y as pulled back */
    unsigned tries;
    uint8_t results[CD_PLAY_KEPT];
    CdPlay attempts[CD_PLAY_KEPT];
} OldCardPlay;

void OldCardMain(void);
void abi_OldCardMain(void);
void OldCardSoftReset(void);
void abi_OldCardSoftReset(void);
void StepDemoFrame(void);
void abi_StepDemoFrame(void);
SpritesArm DrawSprites(void);
void abi_DrawSprites(void);
SpriteArm DrawObjectSprite(uint8_t object);
void abi_DrawObjectSprite(void);
OldCardPlay PlayCdTrack(uint8_t kept_x, uint8_t kept_y);
void abi_PlayCdTrack(void);
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

/* How a routine of object 1 (the program's second character) ended. The values behind the registers, applied in the
 * order the original made them: the last animation step (its FrameStep and the Y it left, Y to A after a TAY), the
 * move's V, the random byte's C and V; then by arm: LOADED, A first and N and Z from second; COMPARED, A first
 * compared with second; NZ_ONLY, N and Z from second (a decrement of memory), A kept. Registers not named are kept. */
enum { OBJ1_ANIMATED, OBJ1_LOADED, OBJ1_COMPARED, OBJ1_NZ_ONLY, OBJ1_ORIGINAL };

typedef struct {
    int arm;
    uint8_t first, second;
    bool stepped;
    FrameStep step;
    uint8_t step_y;
    bool through_tay;
    bool moved;
    AxisMove move;
    bool randomized;
    RandomResult random;
} Obj1Arm;

PlayerArm PlayerHurtFromAttack(uint8_t object);
void abi_PlayerHurtFromAttack(void);
PlayerArm PlayerAttack(uint8_t object);
void abi_PlayerAttack(void);
PlayerArm PlayerKnockedBack(uint8_t object);
void abi_PlayerKnockedBack(void);
void InitObject1(void);
void abi_InitObject1(void);
Obj1Arm RunObject1(void);
void abi_RunObject1(void);
Obj1Arm Object1WaitForScroll(uint8_t object);
void abi_Object1WaitForScroll(void);
Obj1Arm Object1WalkIn(uint8_t object);
void abi_Object1WalkIn(void);
Obj1Arm Object1FacePlayer(uint8_t object);
void abi_Object1FacePlayer(void);
Obj1Arm Object1Wait(uint8_t object);
void abi_Object1Wait(void);
Obj1Arm Object1Jump(uint8_t object);
void abi_Object1Jump(void);
Obj1Arm Object1Hold(uint8_t object);
void abi_Object1Hold(void);
Obj1Arm Object1StepLeft(uint8_t object);
void abi_Object1StepLeft(void);
Obj1Arm Object1StepRight(uint8_t object);
void abi_Object1StepRight(void);

#endif
