/*
 * TilePieceObject072A in the overlays of stages $09 to $10 and the secret stages: the same code but for the address
 * of the overlay's tile piece records, so each runs the body in enemies_stage06_temp.c with its own.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "stage_enemies_l7.h"
#include "tile_piece_b.h"


/*
 * @routine 05CA:01D TilePieceObject05CA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 02), as TilePieceObject072A with this
 * overlay's tile piece records at $AE61.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject05CA(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xAE61);
}

void abi_TilePieceObject05CA(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject05CA(cpu.x));
}

/*
 * @routine 088A:01D TilePieceObject088A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 04), as TilePieceObject072A with this
 * overlay's tile piece records at $C222.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject088A(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xC222);
}

void abi_TilePieceObject088A(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject088A(cpu.x));
}

/*
 * @routine 09EA:01D TilePieceObject09EA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 05), as TilePieceObject072A with this
 * overlay's tile piece records at $BCE2.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject09EA(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xBCE2);
}

void abi_TilePieceObject09EA(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject09EA(cpu.x));
}

/*
 * @routine 0B4A:01D TilePieceObject0B4A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 06), as TilePieceObject072A with this
 * overlay's tile piece records at $B36F.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject0B4A(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xB36F);
}

void abi_TilePieceObject0B4A(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject0B4A(cpu.x));
}

/*
 * @routine 0CAA:01D TilePieceObject0CAA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 07), as TilePieceObject072A with this
 * overlay's tile piece records at $AE9D.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject0CAA(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xAE9D);
}

void abi_TilePieceObject0CAA(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject0CAA(cpu.x));
}

/*
 * @routine 0E0A:01D TilePieceObject0E0A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 08), as TilePieceObject072A with this
 * overlay's tile piece records at $AEF4.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject0E0A(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xAEF4);
}

void abi_TilePieceObject0E0A(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject0E0A(cpu.x));
}

/*
 * @routine 0EBA:01D TilePieceObject0EBA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 09), as TilePieceObject072A with this
 * overlay's tile piece records at $AF1D.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject0EBA(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xAF1D);
}

void abi_TilePieceObject0EBA(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject0EBA(cpu.x));
}

/*
 * @routine 0F6A:01D TilePieceObject0F6A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 09), as TilePieceObject072A with this
 * overlay's tile piece records at $ADBD.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject0F6A(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xADBD);
}

void abi_TilePieceObject0F6A(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject0F6A(cpu.x));
}

/*
 * @routine 101A:01D TilePieceObject101A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 0A), as TilePieceObject072A with this
 * overlay's tile piece records at $B017.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject101A(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xB017);
}

void abi_TilePieceObject101A(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject101A(cpu.x));
}

/*
 * @routine 117A:01D TilePieceObject117A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 0B), as TilePieceObject072A with this
 * overlay's tile piece records at $B073.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject117A(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xB073);
}

void abi_TilePieceObject117A(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject117A(cpu.x));
}

/*
 * @routine 12DA:01D TilePieceObject12DA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 0C), as TilePieceObject072A with this
 * overlay's tile piece records at $AEDF.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject12DA(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xAEDF);
}

void abi_TilePieceObject12DA(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject12DA(cpu.x));
}

/*
 * @routine 13FA:01D TilePieceObject13FA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 0C), as TilePieceObject072A with this
 * overlay's tile piece records at $AEA7.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject13FA(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xAEA7);
}

void abi_TilePieceObject13FA(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject13FA(cpu.x));
}

/*
 * @routine 161A:01D TilePieceObject161A
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 0E), as TilePieceObject072A with this
 * overlay's tile piece records at $B0A3.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject161A(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xB0A3);
}

void abi_TilePieceObject161A(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject161A(cpu.x));
}

/*
 * @routine 16CA:01D TilePieceObject16CA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Secret stage 0E), as TilePieceObject072A with this
 * overlay's tile piece records at $AE7D.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject16CA(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xAE7D);
}

void abi_TilePieceObject16CA(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject16CA(cpu.x));
}

/*
 * @routine 18DA:01D TilePieceObject18DA
 * @run 70:A81D
 * @stop A855
 * WHAT IT DOES: A piece of the scenery drawn from a tile script (Stage 10), as TilePieceObject072A with this
 * overlay's tile piece records at $AE85.
 * HOW: TilePieceObjectAt.
 * WAITS: no.
 * Entry: X object. Exit: as TilePieceObject072A.
 */
PieceObjArm TilePieceObject18DA(uint8_t slot)
{
    return TilePieceObjectAt(slot, 0xAE85);
}

void abi_TilePieceObject18DA(void)
{
    tile_piece_object_at_exit_shim(TilePieceObject18DA(cpu.x));
}
