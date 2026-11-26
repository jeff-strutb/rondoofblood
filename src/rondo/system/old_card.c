/*
 * The old-card program: the 20 sectors from $23 that the boot program reads to $4000 and jumps to when the System
 * Card is older than 3.0 or ex_memopen returns carry set (docs/rondo/HANDBOOK.md, Code). It is a small program of its
 * own: a title with animated objects, then a screen where the player object (slot 0) stands, walks, jumps and is
 * knocked back, on a scrolling background. Its banks are those the boot block maps, MPR2 to MPR6 $80 to $84 (the
 * remap at $4062 writes $FFF5's byte, $80 on card 3.0, so the code keeps running from the same banks).
 *
 * Reached only on that path: no script of ours runs it, and none can on the Super System Card 3.0 the scripts use
 * unless a poke makes ex_memopen fail. So this C is filed, not certified; it is not marked @unreachable, because the
 * read that loads it is one the game makes.
 *
 * Several pieces of the program are byte for byte copies of kernel routines filed elsewhere (ClearObject with
 * ClearObjectExceptPlace at $4584, ApplyVelocity at $4624, StepObjectFrames at $449E, NextRandom at $4692; the
 * census's proven duplicates). They are not filed again: this C calls those routines' bodies where the program
 * calls its copies, with the copies' return addresses.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "old_card.h"

enum {
    VDC_REG_BXR = 0x07, VDC_REG_BYR = 0x08,
    ANIMATION_LISTS = 0x52DF,          /* this program's table of animation list pointers, a word each */
    PLAYER_HANDLERS = 0x46F3,          /* RunPlayerObject's table: a handler address per player state */
    HURT_SPEEDS = 0x478D,              /* the knock-back's X speed, by the hurt byte halved */
    TITLE_FRAMES = 0x44F0, TITLE_X = 0x44F9, TITLE_Y = 0x4502, /* the nine title objects' first frame and place */
    TITLE_OBJECTS = 9,
    PAD_LEFT = 0x80, PAD_LEFT_RIGHT = 0xA0, PAD_II = 0x02, TRIGGER_I = 0x01, TRIGGER_II = 0x02,
    PLAYER = 0,                        /* the player object's slot */
    PLAYER_STANDING = 0, PLAYER_WALKING = 1, PLAYER_JUMPING = 2, PLAYER_ATTACKING = 3, PLAYER_HURT_STATE = 4,
    PLAYER_FLOOR = 0xA0,               /* the Y the player lands at */
    FOLLOW_LEFT_EDGE = 0x28, FOLLOW_RIGHT_EDGE = 0xD8, FOLLOW_CENTRE_X = 0x80, FOLLOW_SCROLL_END = 0x03,
    CARD_E0E4 = 0xE0E4,                /* a card entry with no typed C yet; it takes its arguments at $221F to $2225 */
    CARD_ARG_221F = 0x221F, CARD_ARG_2220 = 0x2220, CARD_ARG_2221 = 0x2221, CARD_ARG_2222 = 0x2222,
    CARD_ARG_2225 = 0x2225,            /* the card's RAM: no names yet, so their address form */
    CARD_E0AE = 0xE0AE,                /* a card entry with no typed C yet (DrawSprites calls it with A 0, X $10) */
    DEMO_TIMER = 0x20,                 /* the object whose $34B8 and $3510 bytes the frame step counts down */
    SPRITE_SETS = 0x4C9D,              /* the program's table of sprite part lists, a word per frame */
    MIRROR_OFFSETS = 0x4453,           /* $F0 and $E0: a part's X mirrored, by its width bit; then $10 and 0, the
                                        * left edge a part at X high byte 0 must reach, by the same bit */
    OPENING_TRACK = 0x4498,            /* PlayCdTrack's six bytes of the card's play arguments */
    CD_PLAY_FROM = 0x40, CD_PLAY_REPEAT_MODE = 0x41
};

static void follow_exit_shim(FollowArm r);
static void player_exit_shim(PlayerArm r);

/* The registers StepDemoFrame hands the original code it calls after RunPlayerObject: what the player left. */
static void demo_player_regs_shim(PlayerArm r)
{
    if (r.arm != PLAYER_ORIGINAL) {
        cpu.x = PLAYER;
    }
    player_exit_shim(r);
}

/*
 * @routine 0023:228 StepDemoFrame
 * @run 80:4228
 * WHAT IT DOES: One frame of the player screen: counts down object $20's timer (its $34B8 byte, then its $3510 byte
 * when that reaches zero) and clears its frame when both have; runs the player, the program's routines at $4863,
 * $4B11 and $49AD, ScrollWithPlayer, and the routines at $497B and $4BE8.
 * HOW: Two decrements, then seven calls; the four with no C yet are run as original with the registers the call
 * before left.
 * WAITS: where the original code it calls does (not read yet).
 * Entry: nothing. Exit: as the routine at $4BE8 leaves them.
 */
void StepDemoFrame(void)
{
    PlayerArm player;
    FollowArm follow;

    if (DEC8(OBJ->f34b8[DEMO_TIMER]) == 0) {
        if (DEC8(OBJ->f3510[DEMO_TIMER]) == 0) {
            WR8(OBJ->frame[DEMO_TIMER], 0);
        }
    }
    jsr_push(0x4238);
    player = RunPlayerObject();
    jsr_pop();
    demo_player_regs_shim(player);
    call_orig(0x423B, 0x4863);
    call_orig(0x423E, 0x4B11);
    call_orig(0x4241, 0x49AD);
    jsr_push(0x4244);
    follow = ScrollWithPlayer();
    jsr_pop();
    follow_exit_shim(follow);
    call_orig(0x4247, 0x497B);
    call_orig(0x424A, 0x4BE8);
}

void abi_StepDemoFrame(void)
{
    StepDemoFrame();
}

/*
 * @routine 0023:24B SetBackgroundScroll
 * @run 80:424B
 * WHAT IT DOES: Writes the scroll the program wants, band 0's background X and Y, to the VDC's BXR and BYR.
 * HOW: With interrupts off, selects BXR and writes ScrollBgxLo and Hi, selects BYR and writes ScrollBgyLo and Hi,
 * then turns interrupts back on.
 * WAITS: no.
 * Entry: nothing. Exit: A ScrollBgyHi as read, N and Z from it, I clear. X, Y, C and V kept.
 */
uint8_t SetBackgroundScroll(void)
{
    uint8_t value;

    /* SEI: the original takes no interrupt from here to the CLI. */
    jsr_push(0x4251);
    SelectVdcRegisterOldCard(VDC_REG_BXR);
    jsr_pop();
    value = RD8(BANDS->bgx_lo[0]);
    WR8(VDC->data_lo, value);
    value = RD8(BANDS->bgx_hi[0]);
    WR8(VDC->data_hi, value);
    jsr_push(0x4262);
    SelectVdcRegisterOldCard(VDC_REG_BYR);
    jsr_pop();
    value = RD8(BANDS->bgy_lo[0]);
    WR8(VDC->data_lo, value);
    value = RD8(BANDS->bgy_hi[0]);
    WR8(VDC->data_hi, value);
    cli();
    return value;
}

void abi_SetBackgroundScroll(void)
{
    uint8_t value = SetBackgroundScroll();

    cpu.a = value;
    abi_nz(value);
    abi_flag(P_I, false);
}

/*
 * @routine 0023:270 SelectVdcRegisterOldCard
 * @run 80:4270
 * WHAT IT DOES: Selects a VDC register, remembering which in zVdcReg: this program's copy of SelectVdcRegister.
 * HOW: The same two stores (SelectVdcRegister's body).
 * WAITS: no.
 * Entry: A register. Exit: A, X, Y and flags kept.
 */
void SelectVdcRegisterOldCard(uint8_t reg)
{
    SelectVdcRegister(reg);
}

void abi_SelectVdcRegisterOldCard(void)
{
    SelectVdcRegisterOldCard(cpu.a);
}

/*
 * @routine 0023:276 StartObjectAnimation
 * @run 80:4276
 * WHAT IT DOES: Steps an object's animation on the list whose number is in A: the list pointer from this program's
 * table at $52DF goes to zTemp10-11, then the step is the one StepObjectFrames makes.
 * HOW: A doubled indexes the table (its bit 7 is dropped), the word goes to zTemp10 and zTemp11, and a jump to the
 * program's copy of StepObjectFrames at $449E.
 * WAITS: no.
 * Entry: A the list, X the object. Exit: as StepObjectFrames, Y the doubled list number where it keeps Y.
 */
FrameStep StartObjectAnimation(uint8_t list, uint8_t object, uint8_t *exit_y)
{
    uint8_t index = (uint8_t)(list << 1);
    uint8_t byte;

    *exit_y = index;
    byte = rd8((uint16_t)(ANIMATION_LISTS + index));
    WR8(ZP->temp10, byte);
    byte = rd8((uint16_t)(ANIMATION_LISTS + 1 + index));
    WR8(ZP->temp11, byte);
    return StepObjectFrames(object, exit_y);   /* JMP to the copy at $449E */
}

void abi_StartObjectAnimation(void)
{
    uint8_t y = cpu.y;
    FrameStep step = StartObjectAnimation(cpu.a, cpu.x, &y);

    step_frames_exit_shim(step, y);
}

/* What ScrollWithPlayer and its two clamps leave: A, N and Z from the arm, Y and C where the arm set them. */
static void follow_exit_shim(FollowArm r)
{
    cpu.a = r.a;
    abi_nz(r.nz);
    if (r.y_set) {
        cpu.y = r.y;
    }
    if (r.c_set) {
        abi_flag(P_C, r.carry);
    }
}

/*
 * @routine 0023:285 ClampObjectLeft
 * @run 80:4285
 * WHAT IT DOES: Puts the player back at the left edge of the screen's walking room, X $28.
 * HOW: One store; ScrollWithPlayer branches here.
 * WAITS: no.
 * Entry: nothing. Exit: A $28, N and Z from it. X, Y, C and V kept.
 */
FollowArm ClampObjectLeft(void)
{
    FollowArm r = { 0 };

    WR8(OBJ->x_lo[PLAYER], FOLLOW_LEFT_EDGE);
    r.arm = FOLLOW_CLAMP_LEFT;
    r.a = FOLLOW_LEFT_EDGE;
    r.nz = FOLLOW_LEFT_EDGE;
    return r;
}

void abi_ClampObjectLeft(void)
{
    follow_exit_shim(ClampObjectLeft());
}

/*
 * @routine 0023:28B ClampObjectRight
 * @run 80:428B
 * WHAT IT DOES: Puts the player back at the right edge of the screen's walking room, X $D8.
 * HOW: One store; ScrollWithPlayer branches here, and to its RTS.
 * WAITS: no.
 * Entry: nothing. Exit: A $D8, N and Z from it. X, Y, C and V kept.
 */
FollowArm ClampObjectRight(void)
{
    FollowArm r = { 0 };

    WR8(OBJ->x_lo[PLAYER], FOLLOW_RIGHT_EDGE);
    r.arm = FOLLOW_CLAMP_RIGHT;
    r.a = FOLLOW_RIGHT_EDGE;
    r.nz = FOLLOW_RIGHT_EDGE;
    return r;
}

void abi_ClampObjectRight(void)
{
    follow_exit_shim(ClampObjectRight());
}

/*
 * @routine 0023:291 ScrollWithPlayer
 * @run 80:4291
 * WHAT IT DOES: Keeps the player between X $28 and $D8, and while the background is not at an end, scrolls it a
 * pixel toward the player and puts the player back at the centre, X $80. The scroll runs from 0 to $301; reaching
 * $301 sets $3E01 (the second byte of ScrollXFrac), which stops the scroll for good, and going below 0 holds it at 0.
 * HOW: Compares of the player's X; ScrollXLo and ScrollXHi as a word stepped up or down, read back after each step.
 * WAITS: no.
 * Entry: nothing. Exit: by arm (FollowArm): the clamps as theirs with C from the compare that sent them (0 left,
 * 1 right); otherwise A, N, Z, Y and C as the arm's last load, store or compare left them. X and V kept.
 */
FollowArm ScrollWithPlayer(void)
{
    FollowArm r = { 0 };
    uint8_t x, stop, low, high;

    x = RD8(OBJ->x_lo[PLAYER]);
    if (x < FOLLOW_LEFT_EDGE) {
        r = ClampObjectLeft();
        r.c_set = true;
        r.carry = false;
        return r;
    }
    if (x >= FOLLOW_RIGHT_EDGE) {
        r = ClampObjectRight();
        r.c_set = true;
        r.carry = true;
        return r;
    }
    stop = RD8(SCROLL->x_frac[1]);
    r.y = stop;
    r.y_set = true;
    r.c_set = true;
    if (stop != 0) {
        r.arm = FOLLOW_FRACTION;
        r.a = x;
        r.nz = stop;
        r.carry = false;
        return r;
    }
    if (x == FOLLOW_CENTRE_X) {
        r.arm = FOLLOW_CENTRE;
        r.a = x;
        r.nz = 0;
        r.carry = true;
        return r;
    }
    if (x > FOLLOW_CENTRE_X) {
        INC8(SCROLL->x_lo[0]);
        low = RD8(SCROLL->x_lo[0]);
        if (low == 1) {
            high = RD8(SCROLL->x_hi[0]);
            if (high == FOLLOW_SCROLL_END) {
                WR8(SCROLL->x_lo[0], 0);
                WR8(SCROLL->x_hi[0], FOLLOW_SCROLL_END);
                r.arm = FOLLOW_RIGHT_END;
                r.a = FOLLOW_SCROLL_END;
                r.nz = INC8(SCROLL->x_frac[1]);
                r.carry = true;
                return r;
            }
            r.carry = high >= FOLLOW_SCROLL_END;
        } else {
            r.y = low;                         /* TAY */
            r.carry = low >= 1;
            if (low == 0) {
                INC8(SCROLL->x_hi[0]);
            }
        }
    } else {
        DEC8(SCROLL->x_lo[0]);
        low = RD8(SCROLL->x_lo[0]);
        r.carry = low == 0xFF;
        if (low == 0xFF) {
            high = DEC8(SCROLL->x_hi[0]);
            if ((high & 0x80) != 0) {
                WR8(SCROLL->x_lo[0], 0);
                WR8(SCROLL->x_hi[0], 0);
                r.arm = FOLLOW_LEFT_END;
                r.a = low;
                r.nz = high;
                return r;
            }
        }
    }
    WR8(OBJ->x_lo[PLAYER], FOLLOW_CENTRE_X);
    r.arm = FOLLOW_RECENTRED;
    r.a = FOLLOW_CENTRE_X;
    r.nz = FOLLOW_CENTRE_X;
    return r;
}

void abi_ScrollWithPlayer(void)
{
    follow_exit_shim(ScrollWithPlayer());
}

/* What DrawObjectSprite leaves: every register from the arm; V where the arm made an add. */
static void sprite_exit_shim(SpriteArm r)
{
    cpu.a = r.a;
    cpu.x = r.x;
    cpu.y = r.y;
    abi_nz(r.nz);
    abi_flag(P_C, r.carry);
    if (r.v_set) {
        abi_flag(P_V, r.overflow);
    }
}

/*
 * @routine 0023:321 DrawObjectSprite
 * @run 80:4321
 * WHAT IT DOES: Writes an object's sprites to the VDC's data port, a part at a time from the list its frame names
 * in the table at $4C9D: each part is five bytes (attributes, Y and X offsets, the pattern's two bytes). A part off
 * the screen (Y plus $40 outside 0 to $12F, X plus $20 outside the part's edge to $11F) is passed over; a part's X
 * is mirrored when the object faces left, its palette replaced (flash bit 7) or offset (bit 6) by the low nibble
 * of the object's flash byte, and its Y flip set from the object's flip byte. Each part written counts one of the
 * sprites left (zp $17); when they run out it stops.
 * HOW: The object's place plus the screen's offsets into zTemp06 to 0B, the part's four words built in zTemp00
 * to 07 and written high word first, from zTemp07 down, through absolute reads of the zero page.
 * WAITS: no.
 * Entry: X the object. Exit: by arm (SpriteArm): A, X, Y, N, Z, C and V as the arm's last instructions left them.
 */
SpriteArm DrawObjectSprite(uint8_t object)
{
    SpriteArm r = { 0 };
    uint8_t value, index, parts, width, high, low, edge, flash, attr;
    bool visible;
    Sum sum;

    value = RD8(OBJ->y_lo[object]);
    sum = add_bytes(value, 0x40, false);
    WR8(ZP->temp08, sum.value);
    value = RD8(OBJ->y_hi[object]);
    sum = add_bytes(value, 0, sum.carry);
    WR8(ZP->temp09, sum.value);
    value = RD8(OBJ->x_lo[object]);
    sum = add_bytes(value, 0x20, false);
    WR8(ZP->temp0a, sum.value);
    value = RD8(OBJ->x_hi[object]);
    sum = add_bytes(value, 0, sum.carry);
    WR8(ZP->temp0b, sum.value);
    value = RD8(OBJ->facing_left[object]);
    WR8(ZP->temp0d, value);
    value = RD8(OBJ->sprite_flip[object]);
    WR8(ZP->sprite_flip_mask, value);
    value = RD8(OBJ->flash_flags[object]);
    WR8(ZP->temp0e, value);
    WR8(ZP->temp0f, (uint8_t)(value & 0x0F));
    WR8(ZP->temp00, (uint8_t)SPRITE_SETS);
    WR8(ZP->temp01, (uint8_t)(SPRITE_SETS >> 8));
    value = RD8(OBJ->frame[object]);
    index = (uint8_t)(value << 1);
    r.carry = (value & 0x80) != 0;
    if (r.carry) {
        INC8(ZP->temp01);
    }
    value = RD8_PTR_Y(ZP->temp00, index);
    WR8(ZP->temp10, value);
    index++;
    value = RD8_PTR_Y(ZP->temp00, index);
    WR8(ZP->temp11, value);
    parts = RD8_PTR(ZP->temp10);
    WR8(ZP->sprite_parts_left, parts);
    if (parts == 0) {
        r.arm = SPRITE_NO_PARTS;
        r.a = 0;
        r.x = object;
        r.y = index;
        r.nz = 0;
        return r;
    }
    if (INC8(ZP->temp10) == 0) {
        INC8(ZP->temp11);
    }
    for (;;) {
        r.x = 0;                                   /* CLX, CLY at $437B */
        attr = RD8_PTR_Y(ZP->temp10, 0);
        WR8(ZP->temp00, attr);
        r.y = 1;
        WR8(ZP->temp06, 0);
        value = RD8_PTR_Y(ZP->temp10, 1);
        if ((value & 0x80) != 0) {
            DEC8(ZP->temp06);
        }
        sum = add_bytes(value, RD8(ZP->temp08), false);
        WR8(ZP->temp07, sum.value);
        high = RD8(ZP->temp06);
        sum = add_bytes(high, RD8(ZP->temp09), sum.carry);
        WR8(ZP->temp06, sum.value);
        visible = sum.value == 0 || (sum.value == 1 && RD8(ZP->temp07) < 0x30);
        if (visible) {
            r.y = 2;
            attr = RD8(ZP->temp00);
            width = (uint8_t)(attr & 0x01);
            r.x = width;
            WR8(ZP->temp04, 0);
            if (RD8(ZP->temp0d) != 0) {
                attr = RD8(ZP->temp00);
                WR8(ZP->temp00, (uint8_t)(attr ^ 0x08));
                value = rd8((uint16_t)(MIRROR_OFFSETS + width));
                value = (uint8_t)(value - RD8_PTR_Y(ZP->temp10, 2));
            } else {
                value = RD8_PTR_Y(ZP->temp10, 2);
            }
            if ((value & 0x80) != 0) {
                DEC8(ZP->temp04);
            }
            sum = add_bytes(value, RD8(ZP->temp0a), false);
            WR8(ZP->temp05, sum.value);
            high = RD8(ZP->temp04);
            sum = add_bytes(high, RD8(ZP->temp0b), sum.carry);
            WR8(ZP->temp04, sum.value);
            r.overflow = sum.overflow;
            if (sum.value == 0) {
                low = RD8(ZP->temp05);
                edge = rd8((uint16_t)(MIRROR_OFFSETS + 2 + width));
                visible = low >= edge;
            } else if ((sum.value & 0x80) != 0) {
                visible = false;
            } else {
                high = RD8(ZP->temp04);
                visible = high == 1 && RD8(ZP->temp05) < 0x20;
            }
        }
        if (visible) {
            r.y = 3;
            value = RD8_PTR_Y(ZP->temp10, 3);
            WR8(ZP->temp03, value);
            r.y = 4;
            value = RD8_PTR_Y(ZP->temp10, 4);
            WR8(ZP->temp02, (uint8_t)((value & 0x70) >> 4));
            flash = RD8(ZP->temp0e);
            if ((flash & 0x80) != 0) {
                value = RD8_PTR_Y(ZP->temp10, 4);
                value = (uint8_t)((value & 0x80) | RD8(ZP->temp0f));
                r.carry = true;
            } else if ((flash & 0x40) != 0) {
                value = RD8_PTR_Y(ZP->temp10, 4);
                sum = add_bytes(value, RD8(ZP->temp0f), false);
                value = (uint8_t)(sum.value & 0x8F);
                r.carry = sum.carry;
                r.overflow = sum.overflow;
            } else {
                value = (uint8_t)(RD8_PTR_Y(ZP->temp10, 4) & 0x8F);
                r.carry = false;
            }
            WR8(ZP->temp01, value);
            value = (uint8_t)(RD8(ZP->sprite_flip_mask) & 0x80);
            value = (uint8_t)(value ^ RD8(ZP->temp01));
            WR8(ZP->temp01, value);
            index = 7;
            for (;;) {
                value = rd8((uint16_t)(ADDR_OF(ZP->temp00) + index));
                WR8(VDC->data_lo, value);
                index--;
                value = rd8((uint16_t)(ADDR_OF(ZP->temp00) + index));
                WR8(VDC->data_hi, value);
                index--;
                if ((index & 0x80) != 0) {
                    break;
                }
            }
            r.y = index;
            r.a = value;
            low = DEC8(ZP->sprites_left);
            if ((low & 0x80) != 0) {
                r.arm = SPRITE_OUT_OF_SPRITES;
                r.nz = low;
                r.v_set = true;
                return r;
            }
        }
        sum = add_bytes(0x05, RD8(ZP->temp10), false);
        WR8(ZP->temp10, sum.value);
        if (sum.carry) {
            INC8(ZP->temp11);
        }
        parts = DEC8(ZP->sprite_parts_left);
        if (parts == 0) {
            r.arm = SPRITE_PARTS_DONE;
            r.a = sum.value;
            r.nz = 0;
            r.carry = sum.carry;
            r.overflow = sum.overflow;
            r.v_set = true;
            return r;
        }
    }
}

void abi_DrawObjectSprite(void)
{
    sprite_exit_shim(DrawObjectSprite(cpu.x));
}

/* The registers DrawSprites hands the card at $E0AE: A 0 (CLA, flags kept), X $10, N and Z from the LDX. */
static void sprites_card_regs_shim(void)
{
    cpu.a = 0x00;
    cpu.x = 0x10;
    abi_nz(0x10);
}

/* What DrawSprites leaves: the last object's arm for Y, C and V (the card's when none was drawn), then A and X and
 * N and Z by the arm. */
static void sprites_exit_shim(SpritesArm r)
{
    if (r.drew) {
        sprite_exit_shim(r.last);
    }
    switch (r.arm) {
    case SPRITES_OUT:
        return;
    case SPRITES_ALL_USED:
        cpu.a = r.a;
        cpu.x = 0x00;
        abi_nz(0x00);
        return;
    default:
        cpu.a = r.a;
        cpu.x = 0xFF;
        abi_nz(0xFF);
        return;
    }
}

/*
 * @routine 0023:2E8 DrawSprites
 * @run 80:42E8
 * WHAT IT DOES: Draws the frame's sprites: after the card's entry at $E0AE (A 0, X $10), every object of the 88
 * with a frame is drawn by DrawObjectSprite, 64 sprites at most; the sprites left over are written as zeros.
 * HOW: zp $17 the sprites left, zp $15 the objects left, zp $D9 the slot across the call; four zero words a sprite
 * by ST1 and ST2.
 * WAITS: where the card's entry does.
 * Entry: nothing. Exit: by arm (SpritesArm).
 */
SpritesArm DrawSprites(void)
{
    SpritesArm r = { 0 };
    uint8_t slot, frame, count;

    sprites_card_regs_shim();
    call_orig(0x42EE, CARD_E0AE);
    WR8(ZP->sprites_left, 0x3F);
    WR8(ZP->product.hi, 0x58);
    slot = 0;
    do {
        frame = RD8(OBJ->frame[slot]);
        r.a = frame;
        if (frame != 0) {
            WR8(ZP->richter_object, slot);
            jsr_push(0x4300);
            r.last = DrawObjectSprite(slot);
            jsr_pop();
            r.drew = true;
            r.a = r.last.a;
            if (r.last.arm == SPRITE_OUT_OF_SPRITES) {
                r.arm = SPRITES_OUT;
                return r;
            }
            slot = RD8(ZP->richter_object);
        }
        slot++;
    } while (DEC8(ZP->product.hi) != 0);
    count = RD8(ZP->sprites_left);
    if (count == 0) {
        r.arm = SPRITES_ALL_USED;
        return r;
    }
    do {
        st1(0);
        st2(0);
        st1(0);
        st2(0);
        st1(0);
        st2(0);
        st1(0);
        st2(0);
        count--;
    } while ((count & 0x80) == 0);
    r.arm = SPRITES_CLEARED;
    return r;
}

void abi_DrawSprites(void)
{
    sprites_exit_shim(DrawSprites());
}

/*
 * @routine 0023:457 PlayCdTrack
 * @run 80:4457
 * WHAT IT DOES: Plays the program's CD track, repeating (mode $41), from the six bytes at $4498, asking the card's
 * cd_play until it answers zero; then the card's cd_fade with A 0.
 * HOW: The kernel's CdPlayRepeat without its track table: zTemp00-01 point at the bytes, which go to the card's
 * argument bytes in its order, zBiosBh $40 first; X the mode, then each failed try's answer.
 * WAITS: yes, on the drive, as CdPlayRepeat: the stack there is the caller's return, this call's two bytes and X and
 * Y pushed.
 * Entry: nothing. Exit: A 0 from the fade, C and V as the card leaves them, X and Y kept, N and Z from X. The C
 * replays the last 32 tries' exits.
 */
OldCardPlay PlayCdTrack(uint8_t kept_x, uint8_t kept_y)
{
    OldCardPlay play = { 0 };
    uint8_t result;

    push8(kept_x);
    push8(kept_y);
    WR8(ZP->cd_play_mode, CD_PLAY_REPEAT_MODE);
    WR8(ZP->temp00, (uint8_t)OPENING_TRACK);
    WR8(ZP->temp01, (uint8_t)(OPENING_TRACK >> 8));
    do {
        CdPlay tried;

        wr8(zBiosBh, CD_PLAY_FROM);
        wr8(zBiosAl, RD8_PTR(ZP->temp00));
        wr8(zBiosAh, RD8_PTR_Y(ZP->temp00, 1));
        wr8(zBiosBl, RD8_PTR_Y(ZP->temp00, 2));
        wr8(zBiosDh, RD8(ZP->cd_play_mode));
        wr8(zBiosCl, RD8_PTR_Y(ZP->temp00, 3));
        wr8(zBiosCh, RD8_PTR_Y(ZP->temp00, 4));
        play.dl = RD8_PTR_Y(ZP->temp00, 5);
        wr8(zBiosDl, play.dl);
        jsr_push(0x448E);
        tried = vec_cd_play(play.dl, play.tries == 0 ? CD_PLAY_REPEAT_MODE
                                                     : play.results[(play.tries - 1) % CD_PLAY_KEPT]);
        jsr_pop();
        result = cd_play_result(tried);
        play.results[play.tries % CD_PLAY_KEPT] = result;
        play.attempts[play.tries % CD_PLAY_KEPT] = tried;
        play.tries++;
    } while (result != 0);
    jsr_push(0x4495);
    vec_cd_fade(0);
    jsr_pop();
    play.y = pull8();
    play.x = pull8();
    return play;
}

/* As the card's calls leave the registers (each try's with A the last argument, X the mode or the answer before,
 * Y 5), then X and Y back as they came, N and Z from X. */
static void play_exit_shim(OldCardPlay play)
{
    unsigned first = play.tries > CD_PLAY_KEPT ? play.tries - CD_PLAY_KEPT : 0;

    for (unsigned i = first; i < play.tries; i++) {
        cpu.x = i == 0 ? CD_PLAY_REPEAT_MODE : play.results[(i - 1) % CD_PLAY_KEPT];
        cpu.a = play.dl;
        cpu.y = 5;
        abi_nz(play.dl);
        cd_play_exit_shim(play.attempts[i % CD_PLAY_KEPT]);
    }
    cpu.x = 0;
    cd_fade_exit_shim(0);
    cpu.x = play.x;
    cpu.y = play.y;
    abi_nz(play.x);
}

void abi_PlayCdTrack(void)
{
    play_exit_shim(PlayCdTrack(cpu.x, cpu.y));
}

/*
 * @routine 0023:4D0 SpawnTitleObjects
 * @run 80:44D0
 * WHAT IT DOES: Empties objects 0 to 8 and gives each its first frame and place from the program's three tables
 * ($44F0 frames, $44F9 X, $4502 Y); then zeroes the title timer, a word at zp $7A-$7B (zWaitTimer3 and 4 in the
 * game's own use).
 * HOW: A loop over the nine slots through the program's copy of ClearObject at $4584, then three table reads.
 * WAITS: no.
 * Entry: nothing. Exit: A the last Y byte read (the table's ninth), X 9, N 0, Z 1, C 1 (the CPX). Y and V kept.
 */
uint8_t SpawnTitleObjects(void)
{
    uint8_t slot = 0, value;

    do {
        jsr_push(0x44D4);
        ClearObject(slot);
        jsr_pop();
        value = rd8((uint16_t)(TITLE_FRAMES + slot));
        WR8(OBJ->frame[slot], value);
        value = rd8((uint16_t)(TITLE_X + slot));
        WR8(OBJ->x_lo[slot], value);
        value = rd8((uint16_t)(TITLE_Y + slot));
        WR8(OBJ->y_lo[slot], value);
        slot++;
    } while (slot != TITLE_OBJECTS);
    WR8(ZP->wait_timer3, 0);
    WR8(ZP->wait_timer4, 0);
    return value;
}

void abi_SpawnTitleObjects(void)
{
    cpu.a = SpawnTitleObjects();
    cpu.x = TITLE_OBJECTS;
    abi_nz(0);
    abi_flag(P_C, true);
}

/* The registers StepTitleTimer holds at its JMP to the card: A 2 (N 0, Z 0), X 5, Y what the step left, C 1 (the
 * equal compare that chose the arm); V as the routine was entered. */
static void title_card_regs_shim(uint8_t y)
{
    cpu.a = 0x02;
    cpu.x = 0x05;
    cpu.y = y;
    abi_nz(0x02);
    abi_flag(P_C, true);
}

/*
 * @routine 0023:50B StepTitleTimer
 * @run 80:450B
 * WHAT IT DOES: One frame of the title: steps the animations of objects 4 (list $0A) and 5 (list $0B) and the title
 * timer at zp $7A-$7B. At $1E0 it sets object 0's frame to $2F. When the timer's low seven bits reach $44 it clears
 * the frames of objects 7 and 8, at $40 it gives object 8 (timer bit 7 set) or object 7 (clear) the frame $36; at
 * either it hands the card at $E0E4 the arguments at $221F to $2225 ($DF and $57, $5B or $5D at $2220-$2221, $10 at
 * $2222, 0 at $2225, 2 at $221F) and jumps to it.
 * HOW: Two calls of StartObjectAnimation, the word's increment, compares of its bytes read again each time.
 * WAITS: no; the card's routine is entered by a JMP and returns to this routine's caller.
 * Entry: nothing. Exit: returned: A the timer's low seven bits, N and Z from their compare with $40, C from it,
 * X 5, Y as the second step left it. Card arm: as the card's routine leaves them. V kept.
 */
TitleArm StepTitleTimer(void)
{
    TitleArm r = { 0 };
    uint8_t low, high;

    jsr_push(0x4512);
    (void)StartObjectAnimation(0x0A, 4, &r.y);
    jsr_pop();
    jsr_push(0x4518);
    r.step = StartObjectAnimation(0x0B, 5, &r.y);
    jsr_pop();
    low = INC8(ZP->wait_timer3);
    if (low == 0) {
        INC8(ZP->wait_timer4);
    }
    high = RD8(ZP->wait_timer4);
    if (high == 0x01) {
        low = RD8(ZP->wait_timer3);
        if (low == 0xE0) {
            WR8(OBJ->frame[0], 0x2F);
        }
    }
    low = RD8(ZP->wait_timer3);
    low = (uint8_t)(low & 0x7F);
    if (low == 0x44) {
        WR8(OBJ->frame[7], 0);
        WR8(OBJ->frame[8], 0);
        wr8(CARD_ARG_2220, 0xDF);
        wr8(CARD_ARG_2221, 0x57);
    } else if (low == 0x40) {
        if ((RD8(ZP->wait_timer3) & 0x80) != 0) {
            WR8(OBJ->frame[8], 0x36);
            wr8(CARD_ARG_2220, 0xDF);
            wr8(CARD_ARG_2221, 0x5B);
        } else {
            WR8(OBJ->frame[7], 0x36);
            wr8(CARD_ARG_2220, 0xDF);
            wr8(CARD_ARG_2221, 0x5D);
        }
    } else {
        r.arm = TITLE_RETURNED;
        r.a = low;
        r.nz = (uint8_t)(low - 0x40);
        r.carry = low >= 0x40;
        return r;
    }
    wr8(CARD_ARG_2222, 0x10);
    wr8(CARD_ARG_2225, 0);
    wr8(CARD_ARG_221F, 0x02);
    title_card_regs_shim(r.y);
    call_original(CARD_E0E4);                /* JMP: the card returns to this routine's caller */
    r.arm = TITLE_CARD_CALLED;
    return r;
}

void abi_StepTitleTimer(void)
{
    TitleArm r = StepTitleTimer();

    if (r.arm == TITLE_CARD_CALLED) {
        return;
    }
    step_frames_exit_shim(r.step, r.y);
    cpu.a = r.a;
    cpu.x = 0x05;
    abi_nz(r.nz);
    abi_flag(P_C, r.carry);
}

/*
 * @routine 0023:6CA InitPlayerObject
 * @run 80:46CA
 * WHAT IT DOES: Empties object 0, the player, and puts it at X $40, Y $A0 with frame 3.
 * HOW: The program's copy of ClearObject, then three stores.
 * WAITS: no.
 * Entry: nothing. Exit: A $40, X 0, N and Z from A. Y, C and V kept.
 */
void InitPlayerObject(void)
{
    jsr_push(0x46CF);
    ClearObject(PLAYER);
    jsr_pop();
    WR8(OBJ->frame[PLAYER], 0x03);
    WR8(OBJ->y_lo[PLAYER], PLAYER_FLOOR);
    WR8(OBJ->x_lo[PLAYER], 0x40);
}

void abi_InitPlayerObject(void)
{
    InitPlayerObject();
    cpu.a = 0x40;
    cpu.x = PLAYER;
    abi_nz(0x40);
}

/* What a player routine leaves. Animated: the step's A, C, N, Z and Y (after the TAY at $47FF Y is A), V that of
 * the move when the arm made one. Hurt: A 4, N 0, Z 0, Y the hurt byte halved, C its bit 0. X kept. */
static void player_exit_shim(PlayerArm r)
{
    switch (r.arm) {
    case PLAYER_ORIGINAL:
        return;
    case PLAYER_HURT:
        cpu.a = PLAYER_HURT_STATE;
        cpu.y = (uint8_t)(r.hurt >> 1);
        abi_nz(PLAYER_HURT_STATE);
        abi_flag(P_C, (r.hurt & 0x01) != 0);
        return;
    default:
        if (r.moved) {
            (void)axis_move_flags_shim(r.move);
        }
        step_frames_exit_shim(r.step, r.y);
        if (r.through_tay) {
            cpu.y = cpu.a;
        }
        return;
    }
}

/* $4729: the animation step on the list the player's state names, read again here. */
static PlayerArm player_animate(uint8_t object)
{
    PlayerArm r = { 0 };
    uint8_t state;

    state = RD8(OBJ->f34b8[object]);
    r.arm = PLAYER_ANIMATED;
    r.step = StartObjectAnimation(state, object, &r.y);   /* JMP $4276 */
    return r;
}

/* $4720: back to standing, the animation from its start. */
static PlayerArm player_rest(uint8_t object)
{
    WR8(OBJ->anim_step[object], 0);
    WR8(OBJ->anim_timer[object], 0);
    WR8(OBJ->f34b8[object], PLAYER_STANDING);
    return player_animate(object);
}

/* $472F: a jump: Y velocity $FB00, X velocity 0, or a pixel a frame toward the side held. */
static PlayerArm player_jump_start(uint8_t object)
{
    uint8_t pad;

    WR8(OBJ->anim_step[object], 0);
    WR8(OBJ->anim_timer[object], 0);
    WR8(OBJ->vy_hi[object], 0xFB);
    WR8(OBJ->vy_lo[object], 0);
    WR8(OBJ->vx_lo[object], 0);
    WR8(OBJ->vx_hi[object], 0);
    pad = rd8(BiosPadState);
    pad = (uint8_t)(pad & PAD_LEFT_RIGHT);
    if (pad != 0) {
        if ((pad & PAD_LEFT) != 0) {
            DEC8(OBJ->vx_hi[object]);
        } else {
            INC8(OBJ->vx_hi[object]);
        }
    }
    WR8(OBJ->f34b8[object], PLAYER_JUMPING);
    return player_animate(object);
}

/* $475C: the attack. */
static PlayerArm player_attack(uint8_t object)
{
    WR8(OBJ->anim_step[object], 0);
    WR8(OBJ->anim_timer[object], 0);
    WR8(OBJ->f34b8[object], PLAYER_ATTACKING);
    return player_animate(object);
}

/* $4769: knocked back: X velocity from the hurt byte (the table at $478D), Y velocity $FC00, state 4. */
static PlayerArm player_hurt(uint8_t object)
{
    PlayerArm r = { 0 };
    uint8_t speed;

    WR8(OBJ->anim_step[object], 0);
    WR8(OBJ->anim_timer[object], 0);
    WR8(OBJ->vx_lo[object], 0x80);
    r.hurt = RD8(OBJ->f3250[object]);
    speed = rd8((uint16_t)(HURT_SPEEDS + (r.hurt >> 1)));
    WR8(OBJ->vx_hi[object], speed);
    WR8(OBJ->vy_lo[object], 0);
    WR8(OBJ->vy_hi[object], 0xFC);
    WR8(OBJ->f34b8[object], PLAYER_HURT_STATE);
    r.arm = PLAYER_HURT;
    return r;
}

/* $47A7: a pixel toward the side held; facing turns with it unless II is held. */
static PlayerArm player_walk_steer(uint8_t object)
{
    uint8_t pad;

    pad = rd8(BiosPadState);
    if ((pad & PAD_LEFT) != 0) {
        pad = rd8(BiosPadState);
        if ((pad & PAD_II) == 0) {
            WR8(OBJ->facing_left[object], 1);
        }
        DEC8(OBJ->x_lo[object]);
    } else {
        pad = rd8(BiosPadState);
        if ((pad & PAD_II) == 0) {
            WR8(OBJ->facing_left[object], 0);
        }
        INC8(OBJ->x_lo[object]);
    }
    return player_animate(object);
}

/* A handler with no C yet (states 3 and 4, at $4812 and $4824), run with the registers RunPlayerObject holds at its
 * JMP: A the table's high byte (N and Z from it), X 0, Y the doubled state, C the state's bit 7. */
static void player_handler_shim(uint16_t handler, uint8_t high, uint8_t index, bool carry)
{
    cpu.a = high;
    cpu.x = PLAYER;
    cpu.y = index;
    abi_nz(high);
    abi_flag(P_C, carry);
    call_original(handler);
}

/*
 * @routine 0023:6DF RunPlayerObject
 * @run 80:46DF
 * WHAT IT DOES: Runs the player object's handler for its state (the byte at $34B8): stand, walk, jump, and two the
 * program has at $4812 and $4824.
 * HOW: The state doubled indexes the table at $46F3; the word goes to zTemp00-01 and an indirect jump takes it.
 * WAITS: no.
 * Entry: nothing. Exit: X 0; the rest as the handler leaves them.
 */
PlayerArm RunPlayerObject(void)
{
    PlayerArm r = { 0 };
    uint8_t state, index, low, high;
    uint16_t handler;

    state = RD8(OBJ->f34b8[PLAYER]);
    index = (uint8_t)(state << 1);
    low = rd8((uint16_t)(PLAYER_HANDLERS + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(PLAYER_HANDLERS + 1 + index));
    WR8(ZP->temp01, high);
    handler = RD16_LH2(ZP->temp00, ZP->temp01);  /* JMP [$2000] */
    switch (handler) {
    case 0x46FD:
        return PlayerStand(PLAYER);
    case 0x4792:
        return PlayerWalk(PLAYER);
    case 0x47CF:
        return PlayerJump(PLAYER);
    default:
        player_handler_shim(handler, high, index, (state & 0x80) != 0);
        r.arm = PLAYER_ORIGINAL;
        return r;
    }
}

void abi_RunPlayerObject(void)
{
    PlayerArm r = RunPlayerObject();

    if (r.arm != PLAYER_ORIGINAL) {
        cpu.x = PLAYER;
    }
    player_exit_shim(r);
}

/*
 * @routine 0023:6FD PlayerStand
 * @run 80:46FD
 * WHAT IT DOES: The player standing: knocked back when hurt ($3250 set); I jumps, II attacks, left or right walks
 * (a first step at once); otherwise it stays standing. Then the animation steps on the state's list.
 * HOW: The pad bytes the card keeps (BiosJoyTrigger shifted bit by bit, BiosPadState), and the arms at $4720,
 * $4729, $472F, $475C and $4769 that the walk and the jump share.
 * WAITS: no.
 * Entry: X the object. Exit: as player_exit_shim's arms. X kept.
 */
PlayerArm PlayerStand(uint8_t object)
{
    uint8_t trigger, pad;

    if (RD8(OBJ->f3250[object]) != 0) {
        return player_hurt(object);
    }
    trigger = rd8(BiosJoyTrigger);
    if ((trigger & TRIGGER_I) != 0) {
        return player_jump_start(object);
    }
    if ((trigger & TRIGGER_II) != 0) {
        return player_attack(object);
    }
    pad = rd8(BiosPadState);
    if ((pad & PAD_LEFT_RIGHT) == 0) {
        return player_rest(object);
    }
    WR8(OBJ->anim_step[object], 0);
    WR8(OBJ->anim_timer[object], 0);
    WR8(OBJ->f34b8[object], PLAYER_WALKING);
    return player_walk_steer(object);        /* JMP $47A7 */
}

void abi_PlayerStand(void)
{
    player_exit_shim(PlayerStand(cpu.x));
}

/*
 * @routine 0023:78F PlayerRestFromWalk
 * @run 80:478F
 * WHAT IT DOES: The walk's way back to standing.
 * HOW: A jump to $4720 in PlayerStand.
 * WAITS: no.
 * Entry: X the object. Exit: as player_exit_shim's animated arm. X kept.
 */
PlayerArm PlayerRestFromWalk(uint8_t object)
{
    return player_rest(object);
}

void abi_PlayerRestFromWalk(void)
{
    player_exit_shim(PlayerRestFromWalk(cpu.x));
}

/*
 * @routine 0023:792 PlayerWalk
 * @run 80:4792
 * WHAT IT DOES: The player walking: hurt, I and II as standing; with neither left nor right held it stands again;
 * otherwise a pixel toward the side held, turning to face it unless II is held.
 * HOW: The same pad reads as PlayerStand; the step is at $47A7.
 * WAITS: no.
 * Entry: X the object. Exit: as player_exit_shim's arms. X kept.
 */
PlayerArm PlayerWalk(uint8_t object)
{
    uint8_t trigger, pad;

    if (RD8(OBJ->f3250[object]) != 0) {
        return player_hurt(object);
    }
    trigger = rd8(BiosJoyTrigger);
    if ((trigger & TRIGGER_I) != 0) {
        return player_jump_start(object);
    }
    if ((trigger & TRIGGER_II) != 0) {
        return player_attack(object);
    }
    pad = rd8(BiosPadState);
    if ((pad & PAD_LEFT_RIGHT) == 0) {
        return PlayerRestFromWalk(object);   /* BEQ $478F */
    }
    return player_walk_steer(object);
}

void abi_PlayerWalk(void)
{
    player_exit_shim(PlayerWalk(cpu.x));
}

/*
 * @routine 0023:7CF PlayerJump
 * @run 80:47CF
 * WHAT IT DOES: The player in the air: hurt as standing; otherwise gravity ($20 a frame on the Y velocity), the move
 * by the velocity, a landing at Y $A0 that stops the X speed, and the animation; when the jump's list has ended the
 * velocity is zeroed and the player stands.
 * HOW: The velocity word's add, the program's copy of ApplyVelocity at $4624, StartObjectAnimation, whose A the TAY
 * at $47FF tests.
 * WAITS: no.
 * Entry: X the object. Exit: as player_exit_shim's arms, V that of the move. X kept.
 */
PlayerArm PlayerJump(uint8_t object)
{
    PlayerArm r = { 0 };
    AxisMove move;
    uint8_t value, state;
    Sum sum;

    if (RD8(OBJ->f3250[object]) != 0) {
        return player_hurt(object);
    }
    value = RD8(OBJ->vy_lo[object]);
    sum = add_bytes(0x20, value, false);
    WR8(OBJ->vy_lo[object], sum.value);
    value = RD8(OBJ->vy_hi[object]);
    sum = add_bytes(0, value, sum.carry);
    WR8(OBJ->vy_hi[object], sum.value);
    jsr_push(0x47E7);
    move = ApplyVelocity(object);
    jsr_pop();
    value = RD8(OBJ->y_lo[object]);
    if (value >= PLAYER_FLOOR) {
        WR8(OBJ->y_lo[object], PLAYER_FLOOR);
        WR8(OBJ->vx_hi[object], 0);
        WR8(OBJ->vx_lo[object], 0);
    }
    state = RD8(OBJ->f34b8[object]);
    jsr_push(0x47FF);
    r.step = StartObjectAnimation(state, object, &r.y);
    jsr_pop();
    if (r.step != FRAME_LIST_ENDED) {
        r.arm = PLAYER_ANIMATED;
        r.moved = true;
        r.move = move;
        r.through_tay = true;
        return r;
    }
    WR8(OBJ->vx_hi[object], 0);
    WR8(OBJ->vx_lo[object], 0);
    WR8(OBJ->vy_hi[object], 0);
    r = player_rest(object);                 /* JMP $4720 */
    r.moved = true;
    r.move = move;
    return r;
}

void abi_PlayerJump(void)
{
    player_exit_shim(PlayerJump(cpu.x));
}
