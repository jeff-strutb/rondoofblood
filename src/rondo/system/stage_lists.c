/* The stage overlay's object and tile lists: a table of records expanded into the stage buffer. */
#include "machine.h"
#include "access.h"
#include "routines.h"
#include "ram.h"
#include "stage_map.h"

enum {
    BUFFER_BYTES = 0x0300, HEADER_BYTES = 8, LIST_END = 0xFF, SET_DESTINATION = 0,
    DESTINATION_STEP = 3, FILL_STEP = 2
};

/*
 * Clears the stage buffer, copies the first eight bytes of the stage section's record into it,
 * then walks the rest of the record: a zero byte followed by a word sets where the next fill
 * goes (zTemp02-03); any other byte is a count, followed by the value the fill stores that many
 * times, moving the destination on past them; $FF ends the record.
 */
StageListEnd expand_stage_lists(uint16_t table)
{
    uint8_t y;
    uint8_t x = 0;
    bool x_set = false;
    uint8_t step;
    uint8_t kind;
    uint8_t count;
    uint8_t value;
    Sum sum;
    Sum last_add;

    tai(KernelZeros, SaveBuffer, BUFFER_BYTES);
    y = (uint8_t)(RD8(ZP->field82) << 1);
    WR8(ZP->temp00, rd8((uint16_t)(table + y)));
    WR8(ZP->temp01, rd8((uint16_t)(table + 1 + y)));
    for (y = 0; y != HEADER_BYTES; y++) {
        value = RD8_PTR_Y(ZP->temp00, y);
        wr8((uint16_t)(SaveBuffer + y), value);
    }
    step = HEADER_BYTES;
    for (;;) {
        last_add = add_bytes(step, RD8(ZP->temp00), false);
        WR8(ZP->temp00, last_add.value);
        if (last_add.carry) {
            INC8(ZP->temp01);
        }
        kind = RD8_PTR(ZP->temp00);
        if (kind == SET_DESTINATION) {
            y = 1;
            value = RD8_PTR_Y(ZP->temp00, y);
            WR8(ZP->temp02, value);
            y++;
            value = RD8_PTR_Y(ZP->temp00, y);
            WR8(ZP->temp03, value);
            step = DESTINATION_STEP;
            continue;
        }
        if (kind == LIST_END) {
            StageListEnd end;

            end.x = x;
            end.x_set = x_set;
            end.y = y;
            end.v = last_add.overflow;
            return end;
        }
        WR8(ZP->temp04, kind);
        x = kind;
        x_set = true;
        y = 1;
        value = RD8_PTR_Y(ZP->temp00, y);
        for (y = 0; ; ) {
            WR8_PTR_Y(ZP->temp02, y, value);
            y++;
            x--;
            if (x == 0) {
                break;
            }
        }
        count = RD8(ZP->temp04);
        sum = add_bytes(count, RD8(ZP->temp02), false);
        WR8(ZP->temp02, sum.value);
        if (sum.carry) {
            INC8(ZP->temp03);
        }
        step = FILL_STEP;
    }
}

/* The exit the list's end leaves: A $FF compared equal to $FF (N 0, Z 1, C 1), V from the last add. */
void lists_exit_shim(StageListEnd end)
{
    cpu.a = LIST_END;
    if (end.x_set) {
        cpu.x = end.x;
    }
    cpu.y = end.y;
    abi_nz(0);
    abi_flag(P_C, true);
    abi_flag(P_V, end.v);
}

/*
 * @routine 024F:277 ExpandStageLists
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageLists(void)
{
    return expand_stage_lists(0x9B98);
}

void abi_ExpandStageLists(void)
{
    lists_exit_shim(ExpandStageLists());
}

/*
 * @routine 050F:277 ExpandStageListsB
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9C28.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsB(void)
{
    return expand_stage_lists(0x9C28);
}

void abi_ExpandStageListsB(void)
{
    lists_exit_shim(ExpandStageListsB());
}

/*
 * @routine 05BF:277 ExpandStageListsC
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9E7F.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsC(void)
{
    return expand_stage_lists(0x9E7F);
}

void abi_ExpandStageListsC(void)
{
    lists_exit_shim(ExpandStageListsC());
}

/*
 * @routine 071F:277 ExpandStageListsD
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9F43.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsD(void)
{
    return expand_stage_lists(0x9F43);
}

void abi_ExpandStageListsD(void)
{
    lists_exit_shim(ExpandStageListsD());
}

/*
 * @routine 07CF:277 ExpandStageListsE
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9CDA.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsE(void)
{
    return expand_stage_lists(0x9CDA);
}

void abi_ExpandStageListsE(void)
{
    lists_exit_shim(ExpandStageListsE());
}

/*
 * @routine 087F:277 ExpandStageListsF
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9E65.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsF(void)
{
    return expand_stage_lists(0x9E65);
}

void abi_ExpandStageListsF(void)
{
    lists_exit_shim(ExpandStageListsF());
}

/*
 * @routine 092F:277 ExpandStageListsG
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9CD9.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsG(void)
{
    return expand_stage_lists(0x9CD9);
}

void abi_ExpandStageListsG(void)
{
    lists_exit_shim(ExpandStageListsG());
}

/*
 * @routine 09DF:277 ExpandStageListsH
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9EAC.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsH(void)
{
    return expand_stage_lists(0x9EAC);
}

void abi_ExpandStageListsH(void)
{
    lists_exit_shim(ExpandStageListsH());
}

/*
 * @routine 0A8F:277 ExpandStageListsI
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9CB0.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsI(void)
{
    return expand_stage_lists(0x9CB0);
}

void abi_ExpandStageListsI(void)
{
    lists_exit_shim(ExpandStageListsI());
}

/*
 * @routine 0B3F:277 ExpandStageListsJ
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9E77.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsJ(void)
{
    return expand_stage_lists(0x9E77);
}

void abi_ExpandStageListsJ(void)
{
    lists_exit_shim(ExpandStageListsJ());
}

/*
 * @routine 0BEF:277 ExpandStageListsK
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9CC7.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsK(void)
{
    return expand_stage_lists(0x9CC7);
}

void abi_ExpandStageListsK(void)
{
    lists_exit_shim(ExpandStageListsK());
}

/*
 * @routine 0C9F:277 ExpandStageListsL
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9E7A.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsL(void)
{
    return expand_stage_lists(0x9E7A);
}

void abi_ExpandStageListsL(void)
{
    lists_exit_shim(ExpandStageListsL());
}

/*
 * @routine 0D4F:277 ExpandStageListsM
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9CEB.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsM(void)
{
    return expand_stage_lists(0x9CEB);
}

void abi_ExpandStageListsM(void)
{
    lists_exit_shim(ExpandStageListsM());
}

/*
 * @routine 0DFF:277 ExpandStageListsN
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9EA8.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsN(void)
{
    return expand_stage_lists(0x9EA8);
}

void abi_ExpandStageListsN(void)
{
    lists_exit_shim(ExpandStageListsN());
}

/*
 * @routine 0EAF:277 ExpandStageListsO
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9DFA.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsO(void)
{
    return expand_stage_lists(0x9DFA);
}

void abi_ExpandStageListsO(void)
{
    lists_exit_shim(ExpandStageListsO());
}

/*
 * @routine 0F5F:277 ExpandStageListsP
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9F1D.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsP(void)
{
    return expand_stage_lists(0x9F1D);
}

void abi_ExpandStageListsP(void)
{
    lists_exit_shim(ExpandStageListsP());
}

/*
 * @routine 100F:277 ExpandStageListsQ
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9C78.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsQ(void)
{
    return expand_stage_lists(0x9C78);
}

void abi_ExpandStageListsQ(void)
{
    lists_exit_shim(ExpandStageListsQ());
}

/*
 * @routine 10BF:277 ExpandStageListsR
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9F07.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsR(void)
{
    return expand_stage_lists(0x9F07);
}

void abi_ExpandStageListsR(void)
{
    lists_exit_shim(ExpandStageListsR());
}

/*
 * @routine 116F:277 ExpandStageListsS
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9C7D.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsS(void)
{
    return expand_stage_lists(0x9C7D);
}

void abi_ExpandStageListsS(void)
{
    lists_exit_shim(ExpandStageListsS());
}

/*
 * @routine 12CF:277 ExpandStageListsT
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9F3D.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsT(void)
{
    return expand_stage_lists(0x9F3D);
}

void abi_ExpandStageListsT(void)
{
    lists_exit_shim(ExpandStageListsT());
}

/*
 * @routine 160F:277 ExpandStageListsU
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9CA8.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsU(void)
{
    return expand_stage_lists(0x9CA8);
}

void abi_ExpandStageListsU(void)
{
    lists_exit_shim(ExpandStageListsU());
}

/*
 * @routine 18CF:277 ExpandStageListsV
 * @run 6B:9277
 * WHAT IT DOES: Builds the stage's list buffer ($7D00, $300 bytes) from the record of the
 * current stage section (zField82): the record's first eight bytes, then runs of one value
 * stored at the places the record names.
 * HOW: A TAI of zeros clears the buffer; the record's address comes from the overlay's table,
 * two bytes per section. After the first eight bytes, a zero byte sets the destination
 * (zTemp02-03) from the word that follows, and any other byte is a count of times to store the
 * value after it, the destination moving on by the count; $FF ends the record.
 * The same code as ExpandStageLists; this overlay's table of records is at $9EA4.
 * WAITS: no.
 * Entry: zField82 the stage section. Exit: A $FF, N 0, Z 1, C 1, V from the last step added to
 * the record's place; Y the last index used; X 0 if a run was stored, else kept. Clobbers
 * zTemp00-04.
 */
StageListEnd ExpandStageListsV(void)
{
    return expand_stage_lists(0x9EA4);
}

void abi_ExpandStageListsV(void)
{
    lists_exit_shim(ExpandStageListsV());
}
