/*
 * TEMPORARY: the hit damage variants of lot L8, one for each overlay that carries its own table, by routine. Bodies return the values behind their
 * exit (L8HitDamage); l8_hit_damage_exit_shim builds how the last instructions left A, Y and the flags, and the abi_ wrappers show it.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "common_objects.h"
#include "enemy_objects.h"
#include "stage_enemies_l8.h"

/* HitDamage with the table of the overlay: the damage is the table's byte at the object's OBJ_2EE0 times sixteen plus its hit; it goes to zTemp00. */
static L8HitDamage hit_damage_from(uint8_t slot, uint16_t table)
{
    L8HitDamage r;
    uint8_t row, hit;

    row = RD8(OBJ->fields_2ee0[slot]);
    row = (uint8_t)(row << 4);
    r.row = row;
    WR8(ZP->temp00, row);
    hit = RD8(OBJ->hit_taken[slot]);
    r.index = add_bytes(hit, RD8(ZP->temp00), false);
    r.damage = rd8((uint16_t)(table + r.index.value));
    WR8(ZP->temp00, r.damage);
    return r;
}

/* What every HitDamage variant leaves in A, Y and the flags (one path): the ASL's A and C, the add, then the damage loaded and the index in Y. */
static Ended l8_hit_damage_exit_shim(L8HitDamage r)
{
    Ended e = ended_with_carry(ended_load(r.row), false);

    e = ended_then(e, ended_of_sum(r.index));
    return ended_then(e, ended_with_y(ended_load(r.damage), r.index.value));
}

/*
 * @routine 0519:28D HitDamage0519
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $ABDB: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0519(uint8_t slot)
{
    return hit_damage_from(slot, 0xABDB);
}

void abi_HitDamage0519(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0519(cpu.x)));
}

/*
 * @routine 05C9:28D HitDamage05C9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AB8F: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage05C9(uint8_t slot)
{
    return hit_damage_from(slot, 0xAB8F);
}

void abi_HitDamage05C9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage05C9(cpu.x)));
}

/*
 * @routine 0729:28D HitDamage0729
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AB97: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0729(uint8_t slot)
{
    return hit_damage_from(slot, 0xAB97);
}

void abi_HitDamage0729(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0729(cpu.x)));
}

/*
 * @routine 07D9:28D HitDamage07D9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AB0D: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage07D9(uint8_t slot)
{
    return hit_damage_from(slot, 0xAB0D);
}

void abi_HitDamage07D9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage07D9(cpu.x)));
}

/*
 * @routine 0889:28D HitDamage0889
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AA94: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0889(uint8_t slot)
{
    return hit_damage_from(slot, 0xAA94);
}

void abi_HitDamage0889(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0889(cpu.x)));
}

/*
 * @routine 0939:28D HitDamage0939
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $ABCD: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0939(uint8_t slot)
{
    return hit_damage_from(slot, 0xABCD);
}

void abi_HitDamage0939(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0939(cpu.x)));
}

/*
 * @routine 0A99:28D HitDamage0A99
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AC14: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0A99(uint8_t slot)
{
    return hit_damage_from(slot, 0xAC14);
}

void abi_HitDamage0A99(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0A99(cpu.x)));
}

/*
 * @routine 0B49:28D HitDamage0B49
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AA98: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0B49(uint8_t slot)
{
    return hit_damage_from(slot, 0xAA98);
}

void abi_HitDamage0B49(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0B49(cpu.x)));
}

/*
 * @routine 0BF9:28D HitDamage0BF9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $ABE8: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0BF9(uint8_t slot)
{
    return hit_damage_from(slot, 0xABE8);
}

void abi_HitDamage0BF9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0BF9(cpu.x)));
}

/*
 * @routine 0CA9:28D HitDamage0CA9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $ABFC: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0CA9(uint8_t slot)
{
    return hit_damage_from(slot, 0xABFC);
}

void abi_HitDamage0CA9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0CA9(cpu.x)));
}

/*
 * @routine 0D59:28D HitDamage0D59
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $ACA2: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0D59(uint8_t slot)
{
    return hit_damage_from(slot, 0xACA2);
}

void abi_HitDamage0D59(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0D59(cpu.x)));
}

/*
 * @routine 0E09:28D HitDamage0E09
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AB01: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0E09(uint8_t slot)
{
    return hit_damage_from(slot, 0xAB01);
}

void abi_HitDamage0E09(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0E09(cpu.x)));
}

/*
 * @routine 0EB9:28D HitDamage0EB9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AC50: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0EB9(uint8_t slot)
{
    return hit_damage_from(slot, 0xAC50);
}

void abi_HitDamage0EB9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0EB9(cpu.x)));
}

/*
 * @routine 1019:28D HitDamage1019
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AC0C: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage1019(uint8_t slot)
{
    return hit_damage_from(slot, 0xAC0C);
}

void abi_HitDamage1019(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage1019(cpu.x)));
}

/*
 * @routine 10C9:28D HitDamage10C9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $ABAB: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage10C9(uint8_t slot)
{
    return hit_damage_from(slot, 0xABAB);
}

void abi_HitDamage10C9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage10C9(cpu.x)));
}

/*
 * @routine 1179:28D HitDamage1179
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AC16: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage1179(uint8_t slot)
{
    return hit_damage_from(slot, 0xAC16);
}

void abi_HitDamage1179(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage1179(cpu.x)));
}

/*
 * @routine 16C9:28D HitDamage16C9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AB66: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage16C9(uint8_t slot)
{
    return hit_damage_from(slot, 0xAB66);
}

void abi_HitDamage16C9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage16C9(cpu.x)));
}


/*
 * @routine 0F69:28D HitDamage0F69
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AA98: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage0F69(uint8_t slot)
{
    return hit_damage_from(slot, 0xAA98);
}

void abi_HitDamage0F69(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage0F69(cpu.x)));
}

/*
 * @routine 12D9:28D HitDamage12D9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AC28: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage12D9(uint8_t slot)
{
    return hit_damage_from(slot, 0xAC28);
}

void abi_HitDamage12D9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage12D9(cpu.x)));
}

/*
 * @routine 13F9:28D HitDamage13F9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AC38: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage13F9(uint8_t slot)
{
    return hit_damage_from(slot, 0xAC38);
}

void abi_HitDamage13F9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage13F9(cpu.x)));
}

/*
 * @routine 1619:28D HitDamage1619
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AC22: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage1619(uint8_t slot)
{
    return hit_damage_from(slot, 0xAC22);
}

void abi_HitDamage1619(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage1619(cpu.x)));
}

/*
 * @routine 18D9:28D HitDamage18D9
 * @run 70:A28D
 * WHAT IT DOES: HitDamage with the overlay's own table at $AB3A: the damage the object takes from the hit it has taken, into zTemp00.
 * HOW: hit_damage_from.
 * WAITS: no.
 * Entry: X object. Exit: A the damage, Y the table index, N and Z from A, C and V from the add (l8_hit_damage_exit_shim).
 */
L8HitDamage HitDamage18D9(uint8_t slot)
{
    return hit_damage_from(slot, 0xAB3A);
}

void abi_HitDamage18D9(void)
{
    abi_ended(l8_hit_damage_exit_shim(HitDamage18D9(cpu.x)));
}

