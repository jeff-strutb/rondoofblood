/*
 * hit_copies.c: the copies of ScoreForKind ($A2A3), DropItemWhenHit ($A2C5) and PlayHitSound ($A2FE) that every
 * stage overlay carries, each the same code as sector 0259's with its own table (the operands found by comparing each
 * copy's instructions with 0259's on the disc), and the calls that reach the copy loaded there, as ObjectHit's
 * JSRs do. A copy filed elsewhere under its own name is not filed again here.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"

/*
 * @routine 0519:2A3 ScoreForKind0519
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0519: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC2A gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0519(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC2A);
}

void abi_ScoreForKind0519(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0519(cpu.x, cpu.y)));
}

/*
 * @routine 05C9:2A3 ScoreForKind05C9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 05C9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $ABEE gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind05C9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xABEE);
}

void abi_ScoreForKind05C9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind05C9(cpu.x, cpu.y)));
}

/*
 * @routine 0729:2A3 ScoreForKind0729
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0729: with HitAttackFlags bit 0 clear, adds the score its table at
 * $ABF6 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0729(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xABF6);
}

void abi_ScoreForKind0729(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0729(cpu.x, cpu.y)));
}

/*
 * @routine 07D9:2A3 ScoreForKind07D9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 07D9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AB7C gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind07D9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAB7C);
}

void abi_ScoreForKind07D9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind07D9(cpu.x, cpu.y)));
}

/*
 * @routine 0889:2A3 ScoreForKind0889
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0889: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AAE3 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0889(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAAE3);
}

void abi_ScoreForKind0889(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0889(cpu.x, cpu.y)));
}

/*
 * @routine 0939:2A3 ScoreForKind0939
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0939: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC3C gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0939(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC3C);
}

void abi_ScoreForKind0939(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0939(cpu.x, cpu.y)));
}

/*
 * @routine 09E9:2A3 ScoreForKind09E9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 09E9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC1E gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind09E9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC1E);
}

void abi_ScoreForKind09E9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind09E9(cpu.x, cpu.y)));
}

/*
 * @routine 0A99:2A3 ScoreForKind0A99
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0A99: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC83 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0A99(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC83);
}

void abi_ScoreForKind0A99(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0A99(cpu.x, cpu.y)));
}

/*
 * @routine 0B49:2A3 ScoreForKind0B49
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0B49: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AAE7 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0B49(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAAE7);
}

void abi_ScoreForKind0B49(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0B49(cpu.x, cpu.y)));
}

/*
 * @routine 0BF9:2A3 ScoreForKind0BF9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0BF9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC47 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0BF9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC47);
}

void abi_ScoreForKind0BF9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0BF9(cpu.x, cpu.y)));
}

/*
 * @routine 0CA9:2A3 ScoreForKind0CA9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0CA9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC6B gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0CA9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC6B);
}

void abi_ScoreForKind0CA9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0CA9(cpu.x, cpu.y)));
}

/*
 * @routine 0D59:2A3 ScoreForKind0D59
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0D59: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AD11 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0D59(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAD11);
}

void abi_ScoreForKind0D59(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0D59(cpu.x, cpu.y)));
}

/*
 * @routine 0E09:2A3 ScoreForKind0E09
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0E09: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AB50 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0E09(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAB50);
}

void abi_ScoreForKind0E09(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0E09(cpu.x, cpu.y)));
}

/*
 * @routine 0EB9:2A3 ScoreForKind0EB9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0EB9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC9F gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0EB9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC9F);
}

void abi_ScoreForKind0EB9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0EB9(cpu.x, cpu.y)));
}

/*
 * @routine 0F69:2A3 ScoreForKind0F69
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 0F69: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AB07 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind0F69(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAB07);
}

void abi_ScoreForKind0F69(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind0F69(cpu.x, cpu.y)));
}

/*
 * @routine 1019:2A3 ScoreForKind1019
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 1019: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC7B gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind1019(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC7B);
}

void abi_ScoreForKind1019(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind1019(cpu.x, cpu.y)));
}

/*
 * @routine 10C9:2A3 ScoreForKind10C9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 10C9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC29 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind10C9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC29);
}

void abi_ScoreForKind10C9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind10C9(cpu.x, cpu.y)));
}

/*
 * @routine 1179:2A3 ScoreForKind1179
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 1179: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC96 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind1179(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC96);
}

void abi_ScoreForKind1179(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind1179(cpu.x, cpu.y)));
}

/*
 * @routine 12D9:2A3 ScoreForKind12D9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 12D9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $ACB7 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind12D9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xACB7);
}

void abi_ScoreForKind12D9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind12D9(cpu.x, cpu.y)));
}

/*
 * @routine 13F9:2A3 ScoreForKind13F9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 13F9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC97 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind13F9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC97);
}

void abi_ScoreForKind13F9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind13F9(cpu.x, cpu.y)));
}

/*
 * @routine 1619:2A3 ScoreForKind1619
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 1619: with HitAttackFlags bit 0 clear, adds the score its table at
 * $AC91 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind1619(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xAC91);
}

void abi_ScoreForKind1619(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind1619(cpu.x, cpu.y)));
}

/*
 * @routine 16C9:2A3 ScoreForKind16C9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 16C9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $ABC5 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind16C9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xABC5);
}

void abi_ScoreForKind16C9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind16C9(cpu.x, cpu.y)));
}

/*
 * @routine 18D9:2A3 ScoreForKind18D9
 * @run 70:A2A3
 * WHAT IT DOES: ScoreForKind in the overlay at sector 18D9: with HitAttackFlags bit 0 clear, adds the score its table at
 * $ABA9 gives for the object's kind less $20, and the extra life when that carries over the mark.
 * HOW: score_for_kind_at with this overlay's table.
 * WAITS: no.
 * Entry: X object, Y the sound driver's. Exit: as ScoreForKind (score_for_kind_exit_shim).
 */
ObjArm ScoreForKind18D9(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, 0xABA9);
}

void abi_ScoreForKind18D9(void)
{
    abi_sound_end(score_for_kind_exit_shim(ScoreForKind18D9(cpu.x, cpu.y)));
}

/*
 * @routine 0519:2C5 DropItemWhenHit0519
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 0519: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $ABFB and the item from $AC03.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit0519(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xABFB, 0xAC03);
}

void abi_DropItemWhenHit0519(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit0519(cpu.x)));
}

/*
 * @routine 05C9:2C5 DropItemWhenHit05C9
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 05C9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $ABBF and the item from $ABC7.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit05C9(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xABBF, 0xABC7);
}

void abi_DropItemWhenHit05C9(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit05C9(cpu.x)));
}

/*
 * @routine 0729:2C5 DropItemWhenHit0729
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 0729: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $ABC7 and the item from $ABCF.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit0729(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xABC7, 0xABCF);
}

void abi_DropItemWhenHit0729(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit0729(cpu.x)));
}

/*
 * @routine 0889:2C5 DropItemWhenHit0889
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 0889: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $AAC4 and the item from $AACC.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit0889(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xAAC4, 0xAACC);
}

void abi_DropItemWhenHit0889(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit0889(cpu.x)));
}

/*
 * @routine 0B49:2C5 DropItemWhenHit0B49
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 0B49: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $AAC8 and the item from $AAD0.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit0B49(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xAAC8, 0xAAD0);
}

void abi_DropItemWhenHit0B49(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit0B49(cpu.x)));
}

/*
 * @routine 0E09:2C5 DropItemWhenHit0E09
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 0E09: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $AB31 and the item from $AB39.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit0E09(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xAB31, 0xAB39);
}

void abi_DropItemWhenHit0E09(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit0E09(cpu.x)));
}

/*
 * @routine 0F69:2C5 DropItemWhenHit0F69
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 0F69: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $AAD8 and the item from $AAE0.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit0F69(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xAAD8, 0xAAE0);
}

void abi_DropItemWhenHit0F69(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit0F69(cpu.x)));
}

/*
 * @routine 10C9:2C5 DropItemWhenHit10C9
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 10C9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $ABFA and the item from $AC02.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit10C9(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xABFA, 0xAC02);
}

void abi_DropItemWhenHit10C9(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit10C9(cpu.x)));
}

/*
 * @routine 12D9:2C5 DropItemWhenHit12D9
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 12D9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $AC88 and the item from $AC90.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit12D9(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xAC88, 0xAC90);
}

void abi_DropItemWhenHit12D9(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit12D9(cpu.x)));
}

/*
 * @routine 13F9:2C5 DropItemWhenHit13F9
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 13F9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $AC68 and the item from $AC70.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit13F9(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xAC68, 0xAC70);
}

void abi_DropItemWhenHit13F9(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit13F9(cpu.x)));
}

/*
 * @routine 1619:2C5 DropItemWhenHit1619
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 1619: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $AC62 and the item from $AC6A.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit1619(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xAC62, 0xAC6A);
}

void abi_DropItemWhenHit1619(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit1619(cpu.x)));
}

/*
 * @routine 16C9:2C5 DropItemWhenHit16C9
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 16C9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $AB96 and the item from $AB9E.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit16C9(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xAB96, 0xAB9E);
}

void abi_DropItemWhenHit16C9(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit16C9(cpu.x)));
}

/*
 * @routine 18D9:2C5 DropItemWhenHit18D9
 * @run 70:A2C5
 * WHAT IT DOES: DropItemWhenHit in the overlay at sector 18D9: with a hit of 5 or more and f31f8 bits 2 to 4 set, may drop
 * an item, the chance under a limit from $AB7A and the item from $AB82.
 * HOW: drop_item_when_hit with this overlay's tables.
 * WAITS: no.
 * Entry: X object. Exit: as DropItemWhenHitF (drop_item_when_hit_exit_shim).
 */
ObjArm DropItemWhenHit18D9(uint8_t slot)
{
    return drop_item_when_hit(slot, 0xAB7A, 0xAB82);
}

void abi_DropItemWhenHit18D9(void)
{
    abi_ended(drop_item_when_hit_exit_shim(DropItemWhenHit18D9(cpu.x)));
}

/*
 * @routine 05C9:2FE PlayHitSound05C9
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 05C9: plays the sound its table at $ABE7 gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound05C9(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xABE7);
}

void abi_PlayHitSound05C9(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound05C9(cpu.x)));
}

/*
 * @routine 0729:2FE PlayHitSound0729
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 0729: plays the sound its table at $ABEF gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound0729(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xABEF);
}

void abi_PlayHitSound0729(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound0729(cpu.x)));
}

/*
 * @routine 0889:2FE PlayHitSound0889
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 0889: plays the sound its table at $AADC gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound0889(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xAADC);
}

void abi_PlayHitSound0889(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound0889(cpu.x)));
}

/*
 * @routine 0B49:2FE PlayHitSound0B49
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 0B49: plays the sound its table at $AAE0 gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound0B49(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xAAE0);
}

void abi_PlayHitSound0B49(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound0B49(cpu.x)));
}

/*
 * @routine 0E09:2FE PlayHitSound0E09
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 0E09: plays the sound its table at $AB49 gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound0E09(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xAB49);
}

void abi_PlayHitSound0E09(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound0E09(cpu.x)));
}

/*
 * @routine 0F69:2FE PlayHitSound0F69
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 0F69: plays the sound its table at $AB00 gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound0F69(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xAB00);
}

void abi_PlayHitSound0F69(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound0F69(cpu.x)));
}

/*
 * @routine 10C9:2FE PlayHitSound10C9
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 10C9: plays the sound its table at $AC22 gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound10C9(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xAC22);
}

void abi_PlayHitSound10C9(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound10C9(cpu.x)));
}

/*
 * @routine 12D9:2FE PlayHitSound12D9
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 12D9: plays the sound its table at $ACB0 gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound12D9(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xACB0);
}

void abi_PlayHitSound12D9(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound12D9(cpu.x)));
}

/*
 * @routine 13F9:2FE PlayHitSound13F9
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 13F9: plays the sound its table at $AC90 gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound13F9(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xAC90);
}

void abi_PlayHitSound13F9(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound13F9(cpu.x)));
}

/*
 * @routine 1619:2FE PlayHitSound1619
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 1619: plays the sound its table at $AC8A gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound1619(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xAC8A);
}

void abi_PlayHitSound1619(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound1619(cpu.x)));
}

/*
 * @routine 18D9:2FE PlayHitSound18D9
 * @run 70:A2FE
 * WHAT IT DOES: PlayHitSound in the overlay at sector 18D9: plays the sound its table at $ABA2 gives for bits 5 to 7
 * of the object's f31f8, when they are not zero and the object is on the first screen.
 * HOW: sound_from_hit_code with this overlay's table.
 * WAITS: no.
 * Entry: X object, D clear. Exit: as PlayHitSoundA (play_hit_sound_exit_shim).
 */
ObjArm PlayHitSound18D9(uint8_t slot)
{
    return sound_from_hit_code(slot, 0xABA2);
}

void abi_PlayHitSound18D9(void)
{
    abi_screen_sound_end(play_hit_sound_exit_shim(PlayHitSound18D9(cpu.x)));
}

/* The sector of the overlay whose copy of a routine at run address `run` is loaded there, 0 if none of these. */
static uint16_t loaded_sector(uint16_t run)
{
    uint32_t at = resident_at(run);

    return at && (at % 2048u) == (uint32_t)(run & 0x7FF) ? (uint16_t)(at / 2048u) : 0;
}

/* The score table of the copy loaded at $A2A3 (sector 0259's when none of the 24 is). */
static uint16_t score_table(void)
{
    switch (loaded_sector(0xA2A3)) {
    case 0x0259: return 0xABF5;
    case 0x0519: return 0xAC2A;
    case 0x05C9: return 0xABEE;
    case 0x0729: return 0xABF6;
    case 0x07D9: return 0xAB7C;
    case 0x0889: return 0xAAE3;
    case 0x0939: return 0xAC3C;
    case 0x09E9: return 0xAC1E;
    case 0x0A99: return 0xAC83;
    case 0x0B49: return 0xAAE7;
    case 0x0BF9: return 0xAC47;
    case 0x0CA9: return 0xAC6B;
    case 0x0D59: return 0xAD11;
    case 0x0E09: return 0xAB50;
    case 0x0EB9: return 0xAC9F;
    case 0x0F69: return 0xAB07;
    case 0x1019: return 0xAC7B;
    case 0x10C9: return 0xAC29;
    case 0x1179: return 0xAC96;
    case 0x12D9: return 0xACB7;
    case 0x13F9: return 0xAC97;
    case 0x1619: return 0xAC91;
    case 0x16C9: return 0xABC5;
    case 0x18D9: return 0xABA9;
    default: return 0xABF5;
    }
}

/* The drop table of the copy loaded at $A2C5 (sector 0259's when none of the 24 is). */
static uint16_t drop_limits(void)
{
    switch (loaded_sector(0xA2C5)) {
    case 0x0259: return 0xABC6;
    case 0x0519: return 0xABFB;
    case 0x05C9: return 0xABBF;
    case 0x0729: return 0xABC7;
    case 0x07D9: return 0xAB3D;
    case 0x0889: return 0xAAC4;
    case 0x0939: return 0xABFD;
    case 0x09E9: return 0xABEF;
    case 0x0A99: return 0xAC44;
    case 0x0B49: return 0xAAC8;
    case 0x0BF9: return 0xAC08;
    case 0x0CA9: return 0xAC3C;
    case 0x0D59: return 0xACD2;
    case 0x0E09: return 0xAB31;
    case 0x0EB9: return 0xAC70;
    case 0x0F69: return 0xAAD8;
    case 0x1019: return 0xAC4C;
    case 0x10C9: return 0xABFA;
    case 0x1179: return 0xAC56;
    case 0x12D9: return 0xAC88;
    case 0x13F9: return 0xAC68;
    case 0x1619: return 0xAC62;
    case 0x16C9: return 0xAB96;
    case 0x18D9: return 0xAB7A;
    default: return 0xABC6;
    }
}

/* The sound table of the copy loaded at $A2FE (sector 0259's when none of the 24 is). */
static uint16_t sound_table(void)
{
    switch (loaded_sector(0xA2FE)) {
    case 0x0259: return 0xABEE;
    case 0x0519: return 0xAC23;
    case 0x05C9: return 0xABE7;
    case 0x0729: return 0xABEF;
    case 0x07D9: return 0xAB75;
    case 0x0889: return 0xAADC;
    case 0x0939: return 0xAC35;
    case 0x09E9: return 0xAC17;
    case 0x0A99: return 0xAC7C;
    case 0x0B49: return 0xAAE0;
    case 0x0BF9: return 0xAC40;
    case 0x0CA9: return 0xAC64;
    case 0x0D59: return 0xAD0A;
    case 0x0E09: return 0xAB49;
    case 0x0EB9: return 0xAC98;
    case 0x0F69: return 0xAB00;
    case 0x1019: return 0xAC74;
    case 0x10C9: return 0xAC22;
    case 0x1179: return 0xAC8E;
    case 0x12D9: return 0xACB0;
    case 0x13F9: return 0xAC90;
    case 0x1619: return 0xAC8A;
    case 0x16C9: return 0xABBE;
    case 0x18D9: return 0xABA2;
    default: return 0xABEE;
    }
}

ObjArm score_for_kind_loaded(uint8_t slot, uint8_t y)
{
    return score_for_kind_at(slot, y, score_table());
}

ObjArm drop_item_when_hit_loaded(uint8_t slot)
{
    uint16_t limits = drop_limits();

    return drop_item_when_hit(slot, limits, (uint16_t)(limits + 8));
}

ObjArm play_hit_sound_loaded(uint8_t slot)
{
    return sound_from_hit_code(slot, sound_table());
}
