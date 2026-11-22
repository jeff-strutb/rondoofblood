/* Stage setup: the state bytes a stage keeps in the last slots of the object table's columns. */
#include "machine.h"
#include "access.h"
#include "stage_map.h"

enum { STAGE_STATE_SLOT = 0x56 };

/*
 * @routine 024F:19F ClearStageStateBytes
 * @run 6B:919F
 * WHAT IT DOES: Zeroes four of the stage's state bytes, the ones a stage restart or its
 * start-of-stage handler clears together.
 * HOW: Four stores of zero, each into slot $56 of one column of the object table (columns
 * $34B8, $3510, $3568 and $3670). The columns are the object table's, the use is the stage
 * overlay's own; the field names are the table's until the typed layer names them.
 * WAITS: no.
 * Entry: nothing. Exit: A, X, Y and flags kept.
 */
void ClearStageStateBytes(void)
{
    WR8(OBJ->f34b8[STAGE_STATE_SLOT], 0);
    WR8(OBJ->f3510[STAGE_STATE_SLOT], 0);
    WR8(OBJ->f3568[STAGE_STATE_SLOT], 0);
    WR8(OBJ->f3670[STAGE_STATE_SLOT], 0);
}

void abi_ClearStageStateBytes(void)
{
    ClearStageStateBytes();
}
