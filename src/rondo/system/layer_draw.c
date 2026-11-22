/* Drawing: the sprites of the frame's objects, written into the VDC's sprite table. */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "layer_draw.h"

/* The sprite table the VDC is writing (word 0 Y, 1 X, 2 pattern, 3 attributes), the parts' limits. */
enum {
    SPRITE_Y_BIAS = 0x40, SPRITE_X_BIAS = 0x20, SPRITE_PART_BYTES = 5,
    SPRITE_Y_FAR_LOW = 0x30, SPRITE_X_FAR_LOW = 0x20, SPRITE_X_WIDE_MIRROR = 0x819F,
    SPRITE_X_MIN = 0x81A1, SPRITE_SET_TABLE = 0x81A3, TAIL_NO_PARTS = 0x819E
};
/* Zero page bytes with no name yet (rows requested from the owner of the records). */
enum { ZpFlipMask = 0x2014, ZpPartsLeft = 0x2016, ZpSpritesLeft = 0x2017 };

/*
 * @routine 00D1:066 EmitObjectSprites
 * @run 6C:8066
 * WHAT IT DOES: Writes the sprites of one object into the VDC's sprite table, a part at a
 * time, leaving out the parts off the screen, until the object's parts or the frame's room
 * for sprites run out.
 * HOW: The object's position plus a bias is the screen origin. Its kind picks a table of
 * frames, its frame picks a description: a count and five bytes a part (flags, Y and X offsets
 * from the origin, pattern, attributes). A part whose Y or X falls outside the screen is
 * skipped; one that shows has its four words (Y, X, pattern, attributes) written to the VDC.
 * A mirrored object has each X offset taken from the width and bit 3 of the flags flipped.
 * The attribute's palette comes from the object's flags (bit 7: the part's own bit 7 and the
 * object's palette; bit 6 with bit 7 clear, untraced: the part's attribute plus the palette).
 * The scratch is zTemp00 to zTemp11 and three more zero page bytes, left as the assembly leaves them.
 * An object with no parts runs on at $819E (untraced, original code).
 * WAITS: no.
 * Entry: X the object. Exit: N set when the frame's room ran out, else Z set (the parts ran out);
 * A, X, Y, C and V as the last part's work leaves them (SpriteExit).
 * Untraced bytes (no script ran them; the draft ends at them, so liftcmp cannot compare them): 00D1:059
 * (the jump to $819E), 00D1:0F1 to 0F9 (the bit 6 palette arm), and the jumps to the skip at 00D1:07A,
 * 00D1:081 and the mirrored arm's path: a mirrored object, an object off the top or the left.
 * liftcmp cases: tools/rondo/curated/layer_draw.txt; A5 attract_82: one call in 76486 differs in
 * RAM byte $51B2 only, and the draft differs at the same call, so it is the harness (a poke).
 * Names: the object fields f2a10, f2a68, f2ac0, f2b18 and f2fe8 hold the sprite set, frame,
 * mirror flag, palette byte and flip mask (renames requested); the three zero page bytes
 * and the disc tables are by address until the owner of the records adds them.
 */
SpriteExit EmitObjectSprites(uint8_t object)
{
    SpriteExit out = { 0 };
    Alu sum;
    uint8_t value, x = object, y;
    bool carry, overflow;

    value = RD8(OBJ->y_lo[object]);
    sum = adc8(value, SPRITE_Y_BIAS, false);
    WR8(ZP->temp08, sum.a);
    value = RD8(OBJ->y_hi[object]);
    sum = adc8(value, 0, sum.c);
    WR8(ZP->temp09, sum.a);
    value = RD8(OBJ->x_lo[object]);
    sum = adc8(value, SPRITE_X_BIAS, false);
    WR8(ZP->temp0_a, sum.a);
    value = RD8(OBJ->x_hi[object]);
    sum = adc8(value, 0, sum.c);
    WR8(ZP->temp0_b, sum.a);
    carry = sum.c;
    overflow = sum.v;
    value = RD8(OBJ->f2ac0[object]);
    WR8(ZP->temp0_d, value);
    value = RD8(OBJ->f2fe8[object]);
    wr8(ZpFlipMask, value);
    value = RD8(OBJ->f2b18[object]);
    WR8(ZP->temp0_e, value);
    WR8(ZP->temp0_f, (uint8_t)(value & 0x0F));
    value = RD8(OBJ->f2a10[object]);
    y = (uint8_t)(value << 1);
    carry = (value & 0x80) != 0;
    value = rd8((uint16_t)(SPRITE_SET_TABLE + y));
    WR8(ZP->temp00, value);
    value = rd8((uint16_t)(SPRITE_SET_TABLE + 1 + y));
    WR8(ZP->temp01, value);
    value = RD8(OBJ->f2a68[object]);
    y = (uint8_t)(value << 1);
    carry = (value & 0x80) != 0;
    if (carry) {
        INC8(ZP->temp01);
    }
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(ZP->temp10, value);
    y++;
    value = RD8_PTR_Y(ZP->temp00, y);
    WR8(ZP->temp11, value);
    value = RD8_PTR(ZP->temp10);
    wr8(ZpPartsLeft, value);
    if (value == 0) {
        out.tail = true;
        out.a = 0;
        out.x = object;
        out.y = y;
        out.nz = 0;
        out.c = carry;
        out.v = overflow;
        return out;
    }
    value = RD8(ZP->temp10);
    WR8(ZP->temp10, (uint8_t)(value + 1));
    if ((uint8_t)(value + 1) == 0) {
        INC8(ZP->temp11);
    }
    for (;;) {
        bool shown = true;
        uint8_t hi, lo, edge;

        x = 0;
        y = 0;
        value = RD8_PTR_Y(ZP->temp10, y);
        WR8(ZP->temp00, value);
        y++;
        WR8(ZP->temp06, 0);
        value = RD8_PTR_Y(ZP->temp10, y);
        if (value & 0x80) {
            DEC8(ZP->temp06);
        }
        edge = RD8(ZP->temp08);
        sum = adc8(value, edge, false);
        WR8(ZP->temp07, sum.a);
        hi = RD8(ZP->temp06);
        edge = RD8(ZP->temp09);
        sum = adc8(hi, edge, sum.c);
        WR8(ZP->temp06, sum.a);
        carry = sum.c;
        overflow = sum.v;
        if (sum.a != 0) {
            if (sum.a & 0x80) {
                shown = false;
            } else {
                carry = true;
                if (sum.a != 1) {
                    shown = false;
                } else {
                    lo = RD8(ZP->temp07);
                    carry = lo >= SPRITE_Y_FAR_LOW;
                    if (carry) {
                        shown = false;
                    }
                }
            }
        }
        if (shown) {
            y++;
            value = RD8(ZP->temp00);
            x = value & 1;
            WR8(ZP->temp04, 0);
            value = RD8(ZP->temp0_d);
            if (value != 0) {
                value = RD8(ZP->temp00);
                WR8(ZP->temp00, (uint8_t)(value ^ 0x08));
                value = rd8((uint16_t)(SPRITE_X_WIDE_MIRROR + x));
                edge = RD8_PTR_Y(ZP->temp10, y);
                sum = sbc8(value, edge, true);
                value = sum.a;
                carry = sum.c;
                overflow = sum.v;
            } else {
                value = RD8_PTR_Y(ZP->temp10, y);
            }
            if (value & 0x80) {
                DEC8(ZP->temp04);
            }
            edge = RD8(ZP->temp0_a);
            sum = adc8(value, edge, false);
            WR8(ZP->temp05, sum.a);
            hi = RD8(ZP->temp04);
            edge = RD8(ZP->temp0_b);
            sum = adc8(hi, edge, sum.c);
            WR8(ZP->temp04, sum.a);
            carry = sum.c;
            overflow = sum.v;
            if (sum.a == 0) {
                lo = RD8(ZP->temp05);
                edge = rd8((uint16_t)(SPRITE_X_MIN + x));
                carry = lo >= edge;
                if (!carry) {
                    shown = false;
                }
            } else if (sum.a & 0x80) {
                shown = false;
            } else {
                hi = RD8(ZP->temp04);
                carry = hi >= 1;
                if (hi != 1) {
                    shown = false;
                } else {
                    lo = RD8(ZP->temp05);
                    carry = lo >= SPRITE_X_FAR_LOW;
                    if (carry) {
                        shown = false;
                    }
                }
            }
            if (shown) {
                y++;
                value = RD8_PTR_Y(ZP->temp10, y);
                WR8(ZP->temp03, value);
                y++;
                value = RD8_PTR_Y(ZP->temp10, y);
                WR8(ZP->temp02, (uint8_t)((value & 0x70) >> 4));
                value = RD8(ZP->temp0_e);
                carry = (value & 0x80) != 0;
                if (carry) {
                    value = RD8_PTR_Y(ZP->temp10, y);
                    edge = RD8(ZP->temp0_f);
                    value = (uint8_t)((value & 0x80) | edge);
                } else if (value & 0x40) {
                    value = RD8_PTR_Y(ZP->temp10, y);
                    edge = RD8(ZP->temp0_f);
                    sum = adc8(value, edge, false);
                    value = (uint8_t)(sum.a & 0x8F);
                    carry = sum.c;
                    overflow = sum.v;
                } else {
                    value = RD8_PTR_Y(ZP->temp10, y);
                    value = (uint8_t)(value & 0x8F);
                }
                WR8(ZP->temp01, value);
                value = rd8(ZpFlipMask);
                edge = RD8(ZP->temp01);
                WR8(ZP->temp01, (uint8_t)((value & 0x80) ^ edge));
                y = 7;
                for (;;) {
                    value = RD8_IDX(ADDR_OF(ZP->temp00), y);
                    WR8(VDC->data_lo, value);
                    y--;
                    value = RD8_IDX(ADDR_OF(ZP->temp00), y);
                    WR8(VDC->data_hi, value);
                    y--;
                    if (y & 0x80) {
                        break;
                    }
                }
                out.a = value;
                value = rd8(ZpSpritesLeft);
                wr8(ZpSpritesLeft, (uint8_t)(value - 1));
                if ((uint8_t)(value - 1) & 0x80) {
                    out.x = x;
                    out.y = y;
                    out.nz = (uint8_t)(value - 1);
                    out.c = carry;
                    out.v = overflow;
                    return out;
                }
            }
        }
        value = RD8(ZP->temp10);
        sum = adc8(5, value, false);
        WR8(ZP->temp10, sum.a);
        if (sum.c) {
            INC8(ZP->temp11);
        }
        value = rd8(ZpPartsLeft);
        wr8(ZpPartsLeft, (uint8_t)(value - 1));
        if ((uint8_t)(value - 1) == 0) {
            out.a = sum.a;
            out.x = x;
            out.y = y;
            out.nz = 0;
            out.c = sum.c;
            out.v = sum.v;
            return out;
        }
    }
}

void abi_EmitObjectSprites(void)
{
    SpriteExit out = EmitObjectSprites(cpu.x);

    cpu.a = out.a;
    cpu.x = out.x;
    cpu.y = out.y;
    abi_nz(out.nz);
    abi_flag(P_C, out.c);
    abi_flag(P_V, out.v);
    if (out.tail) {
        call_original(TAIL_NO_PARTS);
    }
}
