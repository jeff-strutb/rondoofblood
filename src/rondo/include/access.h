/*
 * The typed access layer: how a routine names a byte of RAM.
 *
 * A routine never writes an address. It names a field of a record (ramtypes.h, written from
 * config/rondo/records.csv): RD8(OBJ->vx_hi[slot]), WR8(ZP->temp10, 0). Each macro makes
 * exactly the rd8 or wr8 calls the original makes, in the order the macro says, so the checker
 * sees the same accesses as it does for the untyped form (docs/rondo/PLAN.md, section 1). The
 * records are placed at their original addresses and never dereferenced: a field's address is
 * all a macro uses of it. For the native port only the bodies of RD8 and WR8 change.
 *
 * What a macro checks at compile time: a byte macro takes a one-byte field (a wide field or a
 * whole column is a compile error, not a quiet read of its first byte); a wide macro takes a
 * wide field of the right size. What it cannot check is listed in docs/rondo/PROCEDURE.md.
 *
 * Order of evaluation: a macro's field argument is evaluated once per access it makes, in
 * order. An index in a field (the `slot` in OBJ->vx_hi[slot]) is a plain variable, never an
 * expression that reads memory; RMW8 evaluates the field twice, once for the read and once for
 * the write, as the original does.
 */
#ifndef RONDO_ACCESS_H
#define RONDO_ACCESS_H

#include "machine.h"
#include "ramtypes.h"
#include "tables.h"

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

/* Memory shifts, as lsr_mem and the others in machine.h: the bit shifted out. */
#define LSR8(field) lsr_mem(ADDR_OF8(field))
#define ASL8(field) asl_mem(ADDR_OF8(field))
#define ROR8(field, carry_in) ror_mem(ADDR_OF8(field), (carry_in))
#define ROL8(field, carry_in) rol_mem(ADDR_OF8(field), (carry_in))

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

/*
 * Through a zero-page pointer: the original reads the pointer's two bytes, then the byte they
 * point at (rd8_zp in machine.h). The field is the pointer's low byte; its high byte is the next.
 */
#define RD8_PTR(pointer_lo) rd8_zp(ADDR_OF8(pointer_lo))
#define RD8_PTR_Y(pointer_lo, y) rd8_zp_y(ADDR_OF8(pointer_lo), (y))
#define WR8_PTR_Y(pointer_lo, y, value) wr8_zp_y(ADDR_OF8(pointer_lo), (y), (value))

/*
 * A disc table (config/rondo/tables.csv, settled from the traces by tools/rondo/tables.py): data in code or in a block
 * the loader brought in. TBL8 reads the byte at a byte index; TBL16_LH and TBL16_HL read the element at an element index
 * of a word table, low byte first or high byte first (the order is the assembly's, as for RD16_LH). Each is the same
 * checked read as rd8 of the table's address plus the index. A table no script has read in one bank has no
 * NAME_TBL in tables.h, and using it here is a compile error: a reader exists only for a table the traces settled.
 */
#define TBL8(name, index) rd8((uint16_t)(name##_TBL + (index)))
#define TBL16_LH(name, index) \
    ({ (void)sizeof(char[name##_TBL_ELEM == 2 ? 1 : -1]); uint16_t at_ = (uint16_t)(2 * (index)); \
       uint8_t lo_ = rd8((uint16_t)(name##_TBL + at_)); uint8_t hi_ = rd8((uint16_t)(name##_TBL + at_ + 1)); \
       (uint16_t)(lo_ | (hi_ << 8)); })
#define TBL16_HL(name, index) \
    ({ (void)sizeof(char[name##_TBL_ELEM == 2 ? 1 : -1]); uint16_t at_ = (uint16_t)(2 * (index)); \
       uint8_t hi_ = rd8((uint16_t)(name##_TBL + at_ + 1)); uint8_t lo_ = rd8((uint16_t)(name##_TBL + at_)); \
       (uint16_t)(lo_ | (hi_ << 8)); })

#endif
