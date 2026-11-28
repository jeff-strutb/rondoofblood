/*
 * The scene objects of the title, attract and ending-less scenes (overlay sector $00C2, bank $6B at $8800): every slot with a kind is sent to the
 * handler its kind names, once a frame, and the overlay's four entry stubs jump to the sub-mode dispatchers.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "front_end.h"

enum { OBJECT_SLOTS = 0x58, KIND_TABLE = 0x898A };

/* The V a move leaves, and the high byte it leaves in A. */
static uint8_t move_high(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;

    return (uint8_t)((move.before + added) >> 16);
}

static bool move_overflow(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;
    uint8_t sign = (uint8_t)(added >> 16);
    uint8_t high = (uint8_t)(move.before >> 16);
    uint8_t result = move_high(move);

    return (~(sign ^ high) & (sign ^ result) & 0x80) != 0;
}

/* The registers a handler is entered with from the kind dispatch: A the table's high byte, X the object, Y the index, N and Z from A, C the doubling's. */
static void kind_entry_shim(uint8_t high, uint8_t object, uint8_t index, bool carry)
{
    cpu.a = high;
    cpu.x = object;
    cpu.y = index;
    abi_nz(high);
    abi_flag(P_C, carry);
}

/*
 * @routine 00C2:17A DispatchSceneObject
 * @run 6B:897A
 * WHAT IT DOES: Sends a scene object to the handler its kind names: the table at $898A holds one address per kind, kind 1 first.
 * HOW: Kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01; the jump is made with A the word's high byte,
 * Y the index, C bit 7 of kind less one, X the object. A handler this overlay's C does not hold runs as the original code.
 * WAITS: no.
 * Entry: A the kind (not zero), X the object. Exit: as the handler leaves them.
 */
ObjExit DispatchSceneObject(uint8_t kind, uint8_t object)
{
    ObjExit result = { 0 };
    uint8_t less, index, low, high, high_read;
    uint16_t target;
    bool carry;

    less = (uint8_t)(kind - 1);
    index = (uint8_t)(less << 1);
    carry = (less & 0x80) != 0;
    low = rd8((uint16_t)(KIND_TABLE + index));
    WR8(ZP->temp00, low);
    high = rd8((uint16_t)(KIND_TABLE + 1 + index));
    WR8(ZP->temp01, high);
    low = RD8(ZP->temp00);
    high_read = RD8(ZP->temp01);
    target = (uint16_t)(low | (high_read << 8));
    switch (target) {
    case 0x89B0: result = PlayFrameListOnce(object); break;
    case 0x89C1: result = SlideForCount(object); break;
    case 0x8A86: result = MirrorVramBlock(object); break;
    case 0x8AF0: result = NudgeScrollIn(object); break;
    case 0x8B19: result = DropAndShakeScreen(object); break;
    case 0x8B85: result = BreakApartOnFrame(object); break;
    case 0x8C6A: result = FallAndClear(object); break;
    case 0x8C95: result = IntroSceneObject(object); break;
    case 0x8D1C: result = RiseAndFallObject(object); break;
    case 0x8D81: result = DriftAndClearObject(object); break;
    case 0x8DBB: result = SplitScreenObject(object); break;
    case 0x8A1F: result = AnimateAndCueNine(object); break;
    case 0x8A52: result = AnimateList9AndCueNine(object); break;
    case 0x8E34: result = FlingObjectRight(object); break;
    case 0x8E86: result = FlingObjectLeft(object); break;
    case 0x89EC: result = AnimateAndCueSeven(object); break;
    case 0x8A0E: result = AnimateList1AndCueSeven(object); break;
    case 0x8A6E: result = BlinkScrollFlag(object); break;
    default:
        result.exit.from_cpu = true;
        kind_entry_shim(high, object, index, carry);
        run_routine(target);
        return result;
    }
    if (result.exit.a_kept) {
        result.exit.a = result.moved ? move_high(result.move) : high;
        result.exit.a_kept = false;
    }
    if (result.exit.y_kept) {
        result.exit.y = index;
        result.exit.y_kept = false;
    }
    if (result.exit.c_kept && !result.moved) {
        result.exit.c = carry;
        result.exit.c_kept = false;
    }
    return result;
}

void abi_DispatchSceneObject(void)
{
    ObjExit result = DispatchSceneObject(cpu.a, cpu.x);
    uint8_t moved_a = 0;

    if (result.moved) {
        moved_a = axis_move_flags_shim(result.move);
        if (result.exit.a_kept) {
            result.exit.a = moved_a;
            result.exit.a_kept = false;
        }
    }
    exit_shim(result.exit);
}

/* After the last object: X $58 and the compare's flags; A what the last slot left. */
static void objects_end_shim(uint8_t a)
{
    cpu.a = a;
    cpu.x = OBJECT_SLOTS;
    abi_nz(0);
    abi_flag(P_C, true);
}

/*
 * @routine 00C2:16B StepSceneObjects
 * @run 6B:896B
 * WHAT IT DOES: Runs every scene object that has a kind: a loop over the 88 slots, each with a kind sent to its handler.
 * HOW: OBJ->kind read for each slot; DispatchSceneObject for the ones that are not zero.
 * WAITS: no.
 * Entry: nothing. Exit: X $58, Z and C set, N clear (the loop's last compare); A the last slot's kind or what its handler left;
 * Y and V as the last handler left them.
 * not checked: as EndingObjectsStep: only the idle loop is compared (curated/scene_objects_idle.txt).
 */
Exit StepSceneObjects(void)
{
    Exit exit = { 0 };
    ObjExit result;
    uint8_t slot, kind, a = 0, y = 0;
    bool y_set = false, v_set = false, v = false, via_cpu = false;

    for (slot = 0; slot != OBJECT_SLOTS; slot++) {
        kind = RD8(OBJ->kind[slot]);
        a = kind;
        if (kind == 0) {
            continue;
        }
        jsr_push(0x8974);
        result = DispatchSceneObject(kind, slot);
        jsr_pop();
        if (result.exit.from_cpu) {
            via_cpu = true;
            continue;
        }
        via_cpu = false;
        a = result.exit.a;
        if (!result.exit.x_kept) {
            slot = result.exit.x;
        }
        if (!result.exit.y_kept) {
            y = result.exit.y;
            y_set = true;
        }
        if (result.moved) {
            v = move_overflow(result.move);
            v_set = true;
        }
        if (result.exit.v_set) {
            v = result.exit.v;
            v_set = true;
        }
    }
    if (via_cpu) {
        exit.from_cpu = true;
        objects_end_shim(a);
        return exit;
    }
    exit.a = a;
    exit.x = OBJECT_SLOTS;
    exit.y = y;
    exit.y_kept = !y_set;
    exit.nz = 0;
    exit.c = true;
    exit.v = v;
    exit.v_set = v_set;
    return exit;
}

void abi_StepSceneObjects(void)
{
    exit_shim(StepSceneObjects());
}

/*
 * @routine 00C1:003 TitleScreenEntry
 * @run 6B:8003
 * WHAT IT DOES: The title overlay's entry for the logo mode's frame: runs TitleScreenStep.
 * HOW: A jump to TitleScreenStep.
 * WAITS: no.
 * Entry: Y kept. Exit: as TitleScreenStep.
 */
Exit TitleScreenEntry(uint8_t kept_y)
{
    return TitleScreenStep(kept_y);
}

void abi_TitleScreenEntry(void)
{
    exit_shim(TitleScreenEntry(cpu.y));
}

/*
 * @routine 00C1:006 AttractEntry
 * @run 6B:8006
 * WHAT IT DOES: The overlay's entry for the attract mode's frame: runs AttractStep.
 * HOW: A jump to AttractStep.
 * WAITS: yes, through AttractStep.
 * Entry: Y kept. Exit: as AttractStep.
 */
Exit AttractEntry(uint8_t kept_y)
{
    return AttractStep(kept_y);
}

void abi_AttractEntry(void)
{
    exit_shim(AttractEntry(cpu.y));
}

/*
 * @routine 00C1:009 SceneObjectsEntry
 * @run 6B:8009
 * WHAT IT DOES: The overlay's entry for a frame of the scene's objects: runs StepSceneObjects.
 * HOW: A jump to StepSceneObjects.
 * WAITS: no.
 * Entry: nothing. Exit: as StepSceneObjects.
 * not checked: as StepSceneObjects.
 */
Exit SceneObjectsEntry(void)
{
    return StepSceneObjects();
}

void abi_SceneObjectsEntry(void)
{
    exit_shim(SceneObjectsEntry());
}
