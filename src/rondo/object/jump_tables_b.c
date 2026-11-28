/*
 * The state and phase jump tables of the overlays of stages $09, $0A, $0B, $0C, $0E, $10 and the secret stages:
 * the same 18 bytes as JumpByStateTable025B545 and JumpByPhaseTable025C068 but for the table's address, so each
 * runs the shared body with its own table.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "jump_tables_b.h"

/*
 * @routine 05CB:680 JumpByPhaseTable05CB680
 * @run 70:B680
 * WHAT IT DOES: Goes to the code a table at $B692 names for the object's OBJ_SUBSTATE (Secret stage 02's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable05CB680(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB692 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable05CB680(void)
{
    Jump jump = JumpByPhaseTable05CB680(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088A:37F JumpByStateTable088A37F
 * @run 70:AB7F
 * WHAT IT DOES: Goes to the code a table at $AB91 names for the object's state (Secret stage 04's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable088A37F(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xAB91 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable088A37F(void)
{
    Jump jump = JumpByStateTable088A37F(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088A:661 JumpByStateTable088A661
 * @run 70:AE61
 * WHAT IT DOES: Goes to the code a table at $AE73 names for the object's state (Secret stage 04's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable088A661(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xAE73 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable088A661(void)
{
    Jump jump = JumpByStateTable088A661(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088B:26F JumpByPhaseTable088B26F
 * @run 70:B26F
 * WHAT IT DOES: Goes to the code a table at $B281 names for the object's OBJ_SUBSTATE (Secret stage 04's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable088B26F(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB281 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable088B26F(void)
{
    Jump jump = JumpByPhaseTable088B26F(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088B:5B9 JumpByPhaseTable088B5B9
 * @run 70:B5B9
 * WHAT IT DOES: Goes to the code a table at $B5CB names for the object's OBJ_SUBSTATE (Secret stage 04's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable088B5B9(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB5CB };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable088B5B9(void)
{
    Jump jump = JumpByPhaseTable088B5B9(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 088B:7FF JumpByStateTable088B7FF
 * @run 70:B7FF
 * WHAT IT DOES: Goes to the code a table at $B811 names for the object's state (Secret stage 04's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable088B7FF(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB811 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable088B7FF(void)
{
    Jump jump = JumpByStateTable088B7FF(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EB:431 JumpByPhaseTable09EB431
 * @run 70:B431
 * WHAT IT DOES: Goes to the code a table at $B443 names for the object's OBJ_SUBSTATE (Secret stage 05's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable09EB431(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB443 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable09EB431(void)
{
    Jump jump = JumpByPhaseTable09EB431(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 09EC:223 JumpByStateTable09EC223
 * @run 70:BA23
 * WHAT IT DOES: Goes to the code a table at $BA35 names for the object's state (Secret stage 05's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable09EC223(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBA35 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable09EC223(void)
{
    Jump jump = JumpByStateTable09EC223(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4C:1DC JumpByPhaseTable0B4C1DC
 * @run 70:B9DC
 * WHAT IT DOES: Goes to the code a table at $B9EE names for the object's OBJ_SUBSTATE (Secret stage 06's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable0B4C1DC(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB9EE };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable0B4C1DC(void)
{
    Jump jump = JumpByPhaseTable0B4C1DC(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4D:013 JumpByStateTable0B4D013
 * @run 71:C013
 * WHAT IT DOES: Goes to the code a table at $C025 names for the object's state (Secret stage 06's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable0B4D013(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC025 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable0B4D013(void)
{
    Jump jump = JumpByStateTable0B4D013(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0B4D:0FF JumpByStateTable0B4D0FF
 * @run 71:C0FF
 * WHAT IT DOES: Goes to the code a table at $C111 names for the object's state (Secret stage 06's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable0B4D0FF(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC111 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable0B4D0FF(void)
{
    Jump jump = JumpByStateTable0B4D0FF(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0CAC:7BE JumpByStateTable0CAC7BE
 * @run 70:BFBE
 * WHAT IT DOES: Goes to the code a table at $BFD0 names for the object's state (Secret stage 07's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable0CAC7BE(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBFD0 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable0CAC7BE(void)
{
    Jump jump = JumpByStateTable0CAC7BE(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0E0B:389 JumpByPhaseTable0E0B389
 * @run 70:B389
 * WHAT IT DOES: Goes to the code a table at $B39B names for the object's OBJ_SUBSTATE (Secret stage 08's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable0E0B389(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB39B };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable0E0B389(void)
{
    Jump jump = JumpByPhaseTable0E0B389(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0E0C:5C0 JumpByStateTable0E0C5C0
 * @run 70:BDC0
 * WHAT IT DOES: Goes to the code a table at $BDD2 names for the object's state (Secret stage 08's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable0E0C5C0(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBDD2 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable0E0C5C0(void)
{
    Jump jump = JumpByStateTable0E0C5C0(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBB:063 JumpByStateTable0EBB063
 * @run 70:B063
 * WHAT IT DOES: Goes to the code a table at $B075 names for the object's state (Stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable0EBB063(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB075 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable0EBB063(void)
{
    Jump jump = JumpByStateTable0EBB063(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBB:154 JumpByStateTable0EBB154
 * @run 70:B154
 * WHAT IT DOES: Goes to the code a table at $B166 names for the object's state (Stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable0EBB154(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB166 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable0EBB154(void)
{
    Jump jump = JumpByStateTable0EBB154(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBB:38A JumpByStateTable0EBB38A
 * @run 70:B38A
 * WHAT IT DOES: Goes to the code a table at $B39C names for the object's state (Stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable0EBB38A(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB39C };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable0EBB38A(void)
{
    Jump jump = JumpByStateTable0EBB38A(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBD:7F7 JumpByStateTable0EBD7F7
 * @run 71:C7F7
 * WHAT IT DOES: Goes to the code a table at $C809 names for the object's state (Stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable0EBD7F7(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC809 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable0EBD7F7(void)
{
    Jump jump = JumpByStateTable0EBD7F7(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBE:210 JumpByPhaseTable0EBE210
 * @run 71:CA10
 * WHAT IT DOES: Goes to the code a table at $CA22 names for the object's OBJ_SUBSTATE (Stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable0EBE210(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xCA22 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable0EBE210(void)
{
    Jump jump = JumpByPhaseTable0EBE210(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBE:49E JumpByPhaseTable0EBE49E
 * @run 71:CC9E
 * WHAT IT DOES: Goes to the code a table at $CCB0 names for the object's OBJ_SUBSTATE (Stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable0EBE49E(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xCCB0 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable0EBE49E(void)
{
    Jump jump = JumpByPhaseTable0EBE49E(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBE:6D8 JumpByPhaseTable0EBE6D8
 * @run 71:CED8
 * WHAT IT DOES: Goes to the code a table at $CEEA names for the object's OBJ_SUBSTATE (Stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable0EBE6D8(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xCEEA };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable0EBE6D8(void)
{
    Jump jump = JumpByPhaseTable0EBE6D8(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EBF:21A JumpByPhaseTable0EBF21A
 * @run 71:D21A
 * WHAT IT DOES: Goes to the code a table at $D22C names for the object's OBJ_SUBSTATE (Stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable0EBF21A(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD22C };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable0EBF21A(void)
{
    Jump jump = JumpByPhaseTable0EBF21A(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0EC0:3A3 JumpByStateTable0EC03A3
 * @run 71:DBA3
 * WHAT IT DOES: Goes to the code a table at $DBB5 names for the object's state (Stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable0EC03A3(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xDBB5 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable0EC03A3(void)
{
    Jump jump = JumpByStateTable0EC03A3(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 0F6B:633 JumpByPhaseTable0F6B633
 * @run 70:B633
 * WHAT IT DOES: Goes to the code a table at $B645 names for the object's OBJ_SUBSTATE (Secret stage 09's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable0F6B633(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB645 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable0F6B633(void)
{
    Jump jump = JumpByPhaseTable0F6B633(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101B:662 JumpByPhaseTable101B662
 * @run 70:B662
 * WHAT IT DOES: Goes to the code a table at $B674 names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable101B662(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB674 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable101B662(void)
{
    Jump jump = JumpByPhaseTable101B662(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101B:76D JumpByPhaseTable101B76D
 * @run 70:B76D
 * WHAT IT DOES: Goes to the code a table at $B77F names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable101B76D(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB77F };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable101B76D(void)
{
    Jump jump = JumpByPhaseTable101B76D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101C:381 JumpByPhaseTable101C381
 * @run 70:BB81
 * WHAT IT DOES: Goes to the code a table at $BB93 names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable101C381(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBB93 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable101C381(void)
{
    Jump jump = JumpByPhaseTable101C381(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101C:52B JumpByStateTable101C52B
 * @run 70:BD2B
 * WHAT IT DOES: Goes to the code a table at $BD3D names for the object's state (Stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable101C52B(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBD3D };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable101C52B(void)
{
    Jump jump = JumpByStateTable101C52B(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101D:46A JumpByPhaseTable101D46A
 * @run 71:C46A
 * WHAT IT DOES: Goes to the code a table at $C47C names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable101D46A(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC47C };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable101D46A(void)
{
    Jump jump = JumpByPhaseTable101D46A(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101E:094 JumpByStateTable101E094
 * @run 71:C894
 * WHAT IT DOES: Goes to the code a table at $C8A6 names for the object's state (Stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable101E094(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC8A6 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable101E094(void)
{
    Jump jump = JumpByStateTable101E094(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101E:236 JumpByPhaseTable101E236
 * @run 71:CA36
 * WHAT IT DOES: Goes to the code a table at $CA48 names for the object's OBJ_SUBSTATE (Stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable101E236(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xCA48 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable101E236(void)
{
    Jump jump = JumpByPhaseTable101E236(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 101F:675 JumpByStateTable101F675
 * @run 71:D675
 * WHAT IT DOES: Goes to the code a table at $D687 names for the object's state (Stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable101F675(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD687 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable101F675(void)
{
    Jump jump = JumpByStateTable101F675(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 10CB:26D JumpByStateTable10CB26D
 * @run 70:B26D
 * WHAT IT DOES: Goes to the code a table at $B27F names for the object's state (Secret stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable10CB26D(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB27F };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable10CB26D(void)
{
    Jump jump = JumpByStateTable10CB26D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 10CB:2AB JumpByPhaseTable10CB2AB
 * @run 70:B2AB
 * WHAT IT DOES: Goes to the code a table at $B2BD names for the object's OBJ_SUBSTATE (Secret stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable10CB2AB(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB2BD };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable10CB2AB(void)
{
    Jump jump = JumpByPhaseTable10CB2AB(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 10CB:65E JumpByPhaseTable10CB65E
 * @run 70:B65E
 * WHAT IT DOES: Goes to the code a table at $B670 names for the object's OBJ_SUBSTATE (Secret stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable10CB65E(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB670 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable10CB65E(void)
{
    Jump jump = JumpByPhaseTable10CB65E(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 10CC:4E1 JumpByPhaseTable10CC4E1
 * @run 70:BCE1
 * WHAT IT DOES: Goes to the code a table at $BCF3 names for the object's OBJ_SUBSTATE (Secret stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable10CC4E1(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBCF3 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable10CC4E1(void)
{
    Jump jump = JumpByPhaseTable10CC4E1(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 10CD:019 JumpByStateTable10CD019
 * @run 71:C019
 * WHAT IT DOES: Goes to the code a table at $C02B names for the object's state (Secret stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable10CD019(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC02B };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable10CD019(void)
{
    Jump jump = JumpByStateTable10CD019(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 10CD:319 JumpByStateTable10CD319
 * @run 71:C319
 * WHAT IT DOES: Goes to the code a table at $C32B names for the object's state (Secret stage 0A's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable10CD319(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC32B };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable10CD319(void)
{
    Jump jump = JumpByStateTable10CD319(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117B:2E6 JumpByStateTable117B2E6
 * @run 70:B2E6
 * WHAT IT DOES: Goes to the code a table at $B2F8 names for the object's state (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable117B2E6(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB2F8 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable117B2E6(void)
{
    Jump jump = JumpByStateTable117B2E6(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117B:673 JumpByPhaseTable117B673
 * @run 70:B673
 * WHAT IT DOES: Goes to the code a table at $B685 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable117B673(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB685 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable117B673(void)
{
    Jump jump = JumpByPhaseTable117B673(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117C:7E2 JumpByPhaseTable117C7E2
 * @run 70:BFE2
 * WHAT IT DOES: Goes to the code a table at $BFF4 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable117C7E2(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBFF4 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable117C7E2(void)
{
    Jump jump = JumpByPhaseTable117C7E2(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117D:052 JumpByPhaseTable117D052
 * @run 71:C052
 * WHAT IT DOES: Goes to the code a table at $C064 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable117D052(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC064 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable117D052(void)
{
    Jump jump = JumpByPhaseTable117D052(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117D:4F7 JumpByStateTable117D4F7
 * @run 71:C4F7
 * WHAT IT DOES: Goes to the code a table at $C509 names for the object's state (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable117D4F7(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC509 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable117D4F7(void)
{
    Jump jump = JumpByStateTable117D4F7(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117D:5E3 JumpByPhaseTable117D5E3
 * @run 71:C5E3
 * WHAT IT DOES: Goes to the code a table at $C5F5 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable117D5E3(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC5F5 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable117D5E3(void)
{
    Jump jump = JumpByPhaseTable117D5E3(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117D:7EB JumpByStateTable117D7EB
 * @run 71:C7EB
 * WHAT IT DOES: Goes to the code a table at $C7FD names for the object's state (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable117D7EB(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC7FD };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable117D7EB(void)
{
    Jump jump = JumpByStateTable117D7EB(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117E:11D JumpByStateTable117E11D
 * @run 71:C91D
 * WHAT IT DOES: Goes to the code a table at $C92F names for the object's state (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable117E11D(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC92F };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable117E11D(void)
{
    Jump jump = JumpByStateTable117E11D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117F:13A JumpByStateTable117F13A
 * @run 71:D13A
 * WHAT IT DOES: Goes to the code a table at $D14C names for the object's state (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable117F13A(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD14C };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable117F13A(void)
{
    Jump jump = JumpByStateTable117F13A(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117F:278 JumpByStateTable117F278
 * @run 71:D278
 * WHAT IT DOES: Goes to the code a table at $D28A names for the object's state (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable117F278(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD28A };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable117F278(void)
{
    Jump jump = JumpByStateTable117F278(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117F:3A6 JumpByPhaseTable117F3A6
 * @run 71:D3A6
 * WHAT IT DOES: Goes to the code a table at $D3B8 names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable117F3A6(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD3B8 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable117F3A6(void)
{
    Jump jump = JumpByPhaseTable117F3A6(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117F:4BC JumpByPhaseTable117F4BC
 * @run 71:D4BC
 * WHAT IT DOES: Goes to the code a table at $D4CE names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable117F4BC(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD4CE };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable117F4BC(void)
{
    Jump jump = JumpByPhaseTable117F4BC(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117F:67C JumpByPhaseTable117F67C
 * @run 71:D67C
 * WHAT IT DOES: Goes to the code a table at $D68E names for the object's OBJ_SUBSTATE (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable117F67C(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD68E };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable117F67C(void)
{
    Jump jump = JumpByPhaseTable117F67C(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 117F:7C7 JumpByStateTable117F7C7
 * @run 71:D7C7
 * WHAT IT DOES: Goes to the code a table at $D7D9 names for the object's state (Stage 0B's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable117F7C7(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD7D9 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable117F7C7(void)
{
    Jump jump = JumpByStateTable117F7C7(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12D0:4CD JumpByStateTable12D04CD
 * @run 6B:9CCD
 * WHAT IT DOES: Goes to the code a table at $9CDF names for the object's state (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable12D04CD(uint8_t slot)
{
    static const JumpTableLayout layout = { 0x9CDF };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable12D04CD(void)
{
    Jump jump = JumpByStateTable12D04CD(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DA:71E JumpByStateTable12DA71E
 * @run 70:AF1E
 * WHAT IT DOES: Goes to the code a table at $AF30 names for the object's state (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable12DA71E(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xAF30 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable12DA71E(void)
{
    Jump jump = JumpByStateTable12DA71E(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DA:793 JumpByStateTable12DA793
 * @run 70:AF93
 * WHAT IT DOES: Goes to the code a table at $AFA5 names for the object's state (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable12DA793(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xAFA5 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable12DA793(void)
{
    Jump jump = JumpByStateTable12DA793(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DB:120 JumpByStateTable12DB120
 * @run 70:B120
 * WHAT IT DOES: Goes to the code a table at $B132 names for the object's state (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable12DB120(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB132 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable12DB120(void)
{
    Jump jump = JumpByStateTable12DB120(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DB:2AC JumpByStateTable12DB2AC
 * @run 70:B2AC
 * WHAT IT DOES: Goes to the code a table at $B2BE names for the object's state (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable12DB2AC(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB2BE };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable12DB2AC(void)
{
    Jump jump = JumpByStateTable12DB2AC(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DB:533 JumpByPhaseTable12DB533
 * @run 70:B533
 * WHAT IT DOES: Goes to the code a table at $B545 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable12DB533(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB545 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable12DB533(void)
{
    Jump jump = JumpByPhaseTable12DB533(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DB:772 JumpByPhaseTable12DB772
 * @run 70:B772
 * WHAT IT DOES: Goes to the code a table at $B784 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable12DB772(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB784 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable12DB772(void)
{
    Jump jump = JumpByPhaseTable12DB772(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DC:388 JumpByStateTable12DC388
 * @run 70:BB88
 * WHAT IT DOES: Goes to the code a table at $BB9A names for the object's state (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable12DC388(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBB9A };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable12DC388(void)
{
    Jump jump = JumpByStateTable12DC388(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DC:526 JumpByPhaseTable12DC526
 * @run 70:BD26
 * WHAT IT DOES: Goes to the code a table at $BD38 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable12DC526(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBD38 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable12DC526(void)
{
    Jump jump = JumpByPhaseTable12DC526(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DD:199 JumpByStateTable12DD199
 * @run 71:C199
 * WHAT IT DOES: Goes to the code a table at $C1AB names for the object's state (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable12DD199(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC1AB };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable12DD199(void)
{
    Jump jump = JumpByStateTable12DD199(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DD:534 JumpByPhaseTable12DD534
 * @run 71:C534
 * WHAT IT DOES: Goes to the code a table at $C546 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable12DD534(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC546 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable12DD534(void)
{
    Jump jump = JumpByPhaseTable12DD534(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DE:3D2 JumpByPhaseTable12DE3D2
 * @run 71:CBD2
 * WHAT IT DOES: Goes to the code a table at $CBE4 names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable12DE3D2(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xCBE4 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable12DE3D2(void)
{
    Jump jump = JumpByPhaseTable12DE3D2(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 12DE:5E8 JumpByPhaseTable12DE5E8
 * @run 71:CDE8
 * WHAT IT DOES: Goes to the code a table at $CDFA names for the object's OBJ_SUBSTATE (Stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable12DE5E8(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xCDFA };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable12DE5E8(void)
{
    Jump jump = JumpByPhaseTable12DE5E8(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FB:13D JumpByStateTable13FB13D
 * @run 70:B13D
 * WHAT IT DOES: Goes to the code a table at $B14F names for the object's state (Secret stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable13FB13D(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB14F };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable13FB13D(void)
{
    Jump jump = JumpByStateTable13FB13D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FB:366 JumpByStateTable13FB366
 * @run 70:B366
 * WHAT IT DOES: Goes to the code a table at $B378 names for the object's state (Secret stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable13FB366(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB378 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable13FB366(void)
{
    Jump jump = JumpByStateTable13FB366(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FB:3A0 JumpByPhaseTable13FB3A0
 * @run 70:B3A0
 * WHAT IT DOES: Goes to the code a table at $B3B2 names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable13FB3A0(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB3B2 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable13FB3A0(void)
{
    Jump jump = JumpByPhaseTable13FB3A0(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FB:70F JumpByPhaseTable13FB70F
 * @run 70:B70F
 * WHAT IT DOES: Goes to the code a table at $B721 names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable13FB70F(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB721 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable13FB70F(void)
{
    Jump jump = JumpByPhaseTable13FB70F(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FC:3DF JumpByPhaseTable13FC3DF
 * @run 70:BBDF
 * WHAT IT DOES: Goes to the code a table at $BBF1 names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable13FC3DF(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBBF1 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable13FC3DF(void)
{
    Jump jump = JumpByPhaseTable13FC3DF(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FC:779 JumpByPhaseTable13FC779
 * @run 70:BF79
 * WHAT IT DOES: Goes to the code a table at $BF8B names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable13FC779(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBF8B };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable13FC779(void)
{
    Jump jump = JumpByPhaseTable13FC779(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FD:1E2 JumpByPhaseTable13FD1E2
 * @run 71:C1E2
 * WHAT IT DOES: Goes to the code a table at $C1F4 names for the object's OBJ_SUBSTATE (Secret stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable13FD1E2(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC1F4 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable13FD1E2(void)
{
    Jump jump = JumpByPhaseTable13FD1E2(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 13FD:2B6 JumpByStateTable13FD2B6
 * @run 71:C2B6
 * WHAT IT DOES: Goes to the code a table at $C2C8 names for the object's state (Secret stage 0C's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable13FD2B6(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC2C8 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable13FD2B6(void)
{
    Jump jump = JumpByStateTable13FD2B6(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161B:1C6 JumpByStateTable161B1C6
 * @run 70:B1C6
 * WHAT IT DOES: Goes to the code a table at $B1D8 names for the object's state (Stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable161B1C6(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB1D8 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable161B1C6(void)
{
    Jump jump = JumpByStateTable161B1C6(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161C:134 JumpByPhaseTable161C134
 * @run 70:B934
 * WHAT IT DOES: Goes to the code a table at $B946 names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable161C134(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB946 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable161C134(void)
{
    Jump jump = JumpByPhaseTable161C134(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161C:4B0 JumpByPhaseTable161C4B0
 * @run 70:BCB0
 * WHAT IT DOES: Goes to the code a table at $BCC2 names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable161C4B0(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBCC2 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable161C4B0(void)
{
    Jump jump = JumpByPhaseTable161C4B0(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161D:1DB JumpByPhaseTable161D1DB
 * @run 71:C1DB
 * WHAT IT DOES: Goes to the code a table at $C1ED names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable161D1DB(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC1ED };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable161D1DB(void)
{
    Jump jump = JumpByPhaseTable161D1DB(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161D:366 JumpByPhaseTable161D366
 * @run 71:C366
 * WHAT IT DOES: Goes to the code a table at $C378 names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable161D366(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC378 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable161D366(void)
{
    Jump jump = JumpByPhaseTable161D366(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161D:6E7 JumpByPhaseTable161D6E7
 * @run 71:C6E7
 * WHAT IT DOES: Goes to the code a table at $C6F9 names for the object's OBJ_SUBSTATE (Stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable161D6E7(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC6F9 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable161D6E7(void)
{
    Jump jump = JumpByPhaseTable161D6E7(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161E:257 JumpByStateTable161E257
 * @run 71:CA57
 * WHAT IT DOES: Goes to the code a table at $CA69 names for the object's state (Stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable161E257(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xCA69 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable161E257(void)
{
    Jump jump = JumpByStateTable161E257(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161F:3E3 JumpByStateTable161F3E3
 * @run 71:D3E3
 * WHAT IT DOES: Goes to the code a table at $D3F5 names for the object's state (Stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable161F3E3(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD3F5 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable161F3E3(void)
{
    Jump jump = JumpByStateTable161F3E3(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 161F:5C0 JumpByStateTable161F5C0
 * @run 71:D5C0
 * WHAT IT DOES: Goes to the code a table at $D5D2 names for the object's state (Stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable161F5C0(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xD5D2 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable161F5C0(void)
{
    Jump jump = JumpByStateTable161F5C0(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 16CB:226 JumpByPhaseTable16CB226
 * @run 70:B226
 * WHAT IT DOES: Goes to the code a table at $B238 names for the object's OBJ_SUBSTATE (Secret stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable16CB226(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB238 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable16CB226(void)
{
    Jump jump = JumpByPhaseTable16CB226(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 16CB:539 JumpByPhaseTable16CB539
 * @run 70:B539
 * WHAT IT DOES: Goes to the code a table at $B54B names for the object's OBJ_SUBSTATE (Secret stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable16CB539(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB54B };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable16CB539(void)
{
    Jump jump = JumpByPhaseTable16CB539(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 16CB:76A JumpByStateTable16CB76A
 * @run 70:B76A
 * WHAT IT DOES: Goes to the code a table at $B77C names for the object's state (Secret stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable16CB76A(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB77C };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable16CB76A(void)
{
    Jump jump = JumpByStateTable16CB76A(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 16CC:06E JumpByStateTable16CC06E
 * @run 70:B86E
 * WHAT IT DOES: Goes to the code a table at $B880 names for the object's state (Secret stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable16CC06E(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB880 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable16CC06E(void)
{
    Jump jump = JumpByStateTable16CC06E(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 16CC:36D JumpByStateTable16CC36D
 * @run 70:BB6D
 * WHAT IT DOES: Goes to the code a table at $BB7F names for the object's state (Secret stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable16CC36D(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBB7F };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable16CC36D(void)
{
    Jump jump = JumpByStateTable16CC36D(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 16CC:3F4 JumpByStateTable16CC3F4
 * @run 70:BBF4
 * WHAT IT DOES: Goes to the code a table at $BC06 names for the object's state (Secret stage 0E's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable16CC3F4(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xBC06 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable16CC3F4(void)
{
    Jump jump = JumpByStateTable16CC3F4(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 18DB:1AE JumpByStateTable18DB1AE
 * @run 70:B1AE
 * WHAT IT DOES: Goes to the code a table at $B1C0 names for the object's state (Stage 10's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable18DB1AE(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB1C0 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable18DB1AE(void)
{
    Jump jump = JumpByStateTable18DB1AE(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 18DB:1E9 JumpByPhaseTable18DB1E9
 * @run 70:B1E9
 * WHAT IT DOES: Goes to the code a table at $B1FB names for the object's OBJ_SUBSTATE (Stage 10's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable18DB1E9(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB1FB };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable18DB1E9(void)
{
    Jump jump = JumpByPhaseTable18DB1E9(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 18DB:463 JumpByPhaseTable18DB463
 * @run 70:B463
 * WHAT IT DOES: Goes to the code a table at $B475 names for the object's OBJ_SUBSTATE (Stage 10's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable18DB463(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB475 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable18DB463(void)
{
    Jump jump = JumpByPhaseTable18DB463(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 18DB:5AF JumpByPhaseTable18DB5AF
 * @run 70:B5AF
 * WHAT IT DOES: Goes to the code a table at $B5C1 names for the object's OBJ_SUBSTATE (Stage 10's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable18DB5AF(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB5C1 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable18DB5AF(void)
{
    Jump jump = JumpByPhaseTable18DB5AF(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 18DC:1A1 JumpByPhaseTable18DC1A1
 * @run 70:B9A1
 * WHAT IT DOES: Goes to the code a table at $B9B3 names for the object's OBJ_SUBSTATE (Stage 10's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByPhaseTable18DC1A1(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xB9B3 };

    return JumpByPhaseTableBody(slot, &layout);
}

void abi_JumpByPhaseTable18DC1A1(void)
{
    Jump jump = JumpByPhaseTable18DC1A1(cpu.x);

    jump_shim(jump.end, jump.target);
}

/*
 * @routine 18DD:134 JumpByStateTable18DD134
 * @run 71:C134
 * WHAT IT DOES: Goes to the code a table at $C146 names for the object's state (Stage 10's overlay).
 * HOW: The table's two bytes go to zTemp00-01 and the code is jumped through them.
 * WAITS: no.
 * Entry: X object. Exit: as the code jumped to; A the high byte, Y the doubled value.
 */
Jump JumpByStateTable18DD134(uint8_t slot)
{
    static const JumpTableLayout layout = { 0xC146 };

    return JumpByStateTableBody(slot, &layout);
}

void abi_JumpByStateTable18DD134(void)
{
    Jump jump = JumpByStateTable18DD134(cpu.x);

    jump_shim(jump.end, jump.target);
}
