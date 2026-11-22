/*
 * The typed access layer: how a routine names a byte of RAM.
 *
 * A routine never writes an address. It names a field of a record (card_ramtypes.h, written from
 * syscard/config/records.csv): RD8(CRAM->frame_tick), WR8(CZP->vdc_reg, reg). Each macro makes
 * exactly the rd8 or wr8 calls the original makes, in the order the macro says, so the checker
 * sees the same accesses as it does for the untyped form. The
 * records are placed at their original addresses and never dereferenced: a field's address is
 * all a macro uses of it. For the native port only the bodies of RD8 and WR8 change.
 *
 * What a macro checks at compile time: a byte macro takes a one-byte field (a wide field or a
 * whole column is a compile error, not a quiet read of its first byte); a wide macro takes a
 * wide field of the right size. What it cannot check: the order a wide macro names must be the assembly's, a value kept in a C variable is not a read, and two fields given to a pair macro are not checked to be adjacent.
 *
 * Order of evaluation: a macro's field argument is evaluated once per access it makes, in
 * order. An index in a field  is a plain variable, never an
 * expression that reads memory; RMW8 evaluates the field twice, once for the read and once for
 * the write, as the original does.
 */
#ifndef CARD_ACCESS_H
#define CARD_ACCESS_H

#include "card.h"
#include "card_ramtypes.h"

/* The original address of a field. Not a read. */
#define ADDR_OF(field) ((uint16_t)(uintptr_t) & (field))
/* The same, for a one-byte field only. */
#define ADDR_OF8(field) ((void)sizeof(char[sizeof(field) == 1 ? 1 : -1]), ADDR_OF(field))
/* A column's first byte, for a body shared by several columns (see RD8_IDX below). */
#define COL(column) ADDR_OF((column)[0])

/* One byte. */
#define RD8(field) rd8(ADDR_OF8(field))
#define RDS8(field) ((int8_t)rd8(ADDR_OF8(field)))
#define WR8(field, value) wr8(ADDR_OF8(field), (value))

/*
 * A column picked at run time: a base from COL(OBJ->vx_hi) kept in a variable or a table (an
 * AxisFields), and the slot. One read or write at base + index.
 */
#define RD8_IDX(base, index) rd8((uint16_t)((base) + (index)))
#define WR8_IDX(base, index, value) wr8((uint16_t)((base) + (index)), (value))

/*
 * A read, modify and write of one byte, as INC, DEC, ASL, ORA-to-memory and the like make it: one
 * read, then one write of the new value. `old` names the value read, inside `expr`, and the
 * macro gives the value written. RMW8(ZP->temp10, v, v - 1) is DEC zTemp10.
 */
#define RMW8(field, old, expr) \
    ({ uint8_t old = RD8(field); uint8_t new_ = (uint8_t)(expr); WR8(field, new_); new_; })
#define INC8(field) RMW8(field, old_, old_ + 1)
#define DEC8(field) RMW8(field, old_, old_ - 1)

/*
 * Two bytes in one field (U16LE or U16BE) or in a pair of fields. The name is the order of the
 * accesses, not of the storage: _LH reads or writes the low byte first, _HL the high byte first.
 * There is no macro without an order. A write evaluates its value before the first store; a
 * routine that stores a byte before it has the next one's value writes the bytes with WR8.
 * The two-field forms take the low byte's field and the high byte's: two columns of a record,
 * or two bytes of zero page that are a pointer for the routine's duration.
 */
#define RD16_LH(field) \
    ({ uint8_t lo_ = RD8((field).lo); uint8_t hi_ = RD8((field).hi); (uint16_t)(lo_ | (hi_ << 8)); })
#define RD16_HL(field) \
    ({ uint8_t hi_ = RD8((field).hi); uint8_t lo_ = RD8((field).lo); (uint16_t)(lo_ | (hi_ << 8)); })
#define WR16_LH(field, value) \
    do { uint16_t v_ = (value); WR8((field).lo, (uint8_t)v_); WR8((field).hi, (uint8_t)(v_ >> 8)); } while (0)
#define WR16_HL(field, value) \
    do { uint16_t v_ = (value); WR8((field).hi, (uint8_t)(v_ >> 8)); WR8((field).lo, (uint8_t)v_); } while (0)

#define RD16_LH2(lo_field, hi_field) \
    ({ uint8_t lo_ = RD8(lo_field); uint8_t hi_ = RD8(hi_field); (uint16_t)(lo_ | (hi_ << 8)); })
#define RD16_HL2(lo_field, hi_field) \
    ({ uint8_t hi_ = RD8(hi_field); uint8_t lo_ = RD8(lo_field); (uint16_t)(lo_ | (hi_ << 8)); })
#define WR16_LH2(lo_field, hi_field, value) \
    do { uint16_t v_ = (value); WR8(lo_field, (uint8_t)v_); WR8(hi_field, (uint8_t)(v_ >> 8)); } while (0)
#define WR16_HL2(lo_field, hi_field, value) \
    do { uint16_t v_ = (value); WR8(hi_field, (uint8_t)(v_ >> 8)); WR8(lo_field, (uint8_t)v_); } while (0)

/* Three bytes in one field (U24LE or U24BE): low, middle, high, or the reverse. */
#define RD24_LMH(field) \
    ({ uint8_t lo_ = RD8((field).lo); uint8_t mid_ = RD8((field).mid); uint8_t hi_ = RD8((field).hi); \
       (uint32_t)lo_ | ((uint32_t)mid_ << 8) | ((uint32_t)hi_ << 16); })
#define RD24_HML(field) \
    ({ uint8_t hi_ = RD8((field).hi); uint8_t mid_ = RD8((field).mid); uint8_t lo_ = RD8((field).lo); \
       (uint32_t)lo_ | ((uint32_t)mid_ << 8) | ((uint32_t)hi_ << 16); })
#define WR24_LMH(field, value) \
    do { uint32_t v_ = (value); WR8((field).lo, (uint8_t)v_); WR8((field).mid, (uint8_t)(v_ >> 8)); \
         WR8((field).hi, (uint8_t)(v_ >> 16)); } while (0)
#define WR24_HML(field, value) \
    do { uint32_t v_ = (value); WR8((field).hi, (uint8_t)(v_ >> 16)); WR8((field).mid, (uint8_t)(v_ >> 8)); \
         WR8((field).lo, (uint8_t)v_); } while (0)

#endif
