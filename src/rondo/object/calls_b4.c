/*
 * More calling routines of stages $09 to $10 and the secret stages: frame lists, moves, spawns, scripts, sounds.
 */
#include "machine.h"
#include "ram.h"
#include "access.h"
#include "routines.h"
#include "enemy_objects.h"
#include "stage_enemies_l7.h"
#include "stage_enemies_l8.h"
#include "richter.h"
#include "calls_b4.h"

static void lda(uint8_t value) { cpu.a = value; abi_nz(value); }

static void cmp(uint8_t reg, uint8_t value)
{
    abi_flag(P_C, reg >= value);
    abi_nz((uint8_t)(reg - value));
}

/* JSR LoadObjectRecord's vector. */
static void load_record(uint8_t slot, uint16_t ret)
{
    uint8_t nz, last;
    bool carry;

    jsr_push(ret);
    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
    jsr_pop();
}

/* LDA #command and JSR SoundCommand's vector. */
static void sound(uint8_t command, uint16_t ret)
{
    CommandCall call;

    lda(command);
    jsr_push(ret);
    call = SoundCommand(command, cpu.x, cpu.y);
    sound_command_exit_shim(command, call);
    jsr_pop();
}

/* The frame list's address into zTemp11 then zTemp10 (or the other way round), and JSR StepObjectFrames. */
static void step_list(uint8_t slot, uint16_t list, bool low_first, uint16_t ret)
{
    uint8_t y = cpu.y;
    FrameStep step;

    if (low_first) {
        lda((uint8_t)list);
        WR8(ZP->temp10, cpu.a);
        lda((uint8_t)(list >> 8));
        WR8(ZP->temp11, cpu.a);
    } else {
        lda((uint8_t)(list >> 8));
        WR8(ZP->temp11, cpu.a);
        lda((uint8_t)list);
        WR8(ZP->temp10, cpu.a);
    }
    jsr_push(ret);
    step = StepObjectFrames(slot, &y);
    step_frames_exit_shim(step, y);
    jsr_pop();
}

static void restart_animation(uint8_t slot)
{
    WR8(OBJ->anim_step[slot], 0);
    WR8(OBJ->anim_timer[slot], 0);
}

static void clear_slot_in_y(uint16_t ret)
{
    jsr_push(ret);
    ClearObjectInY(cpu.y);
    jsr_pop();
}

/* LDA #kind and JSR StartObjectAtPlace: the object at X's place, in slot Y. */
static void start_at_place(uint8_t kind, uint16_t ret)
{
    lda(kind);
    jsr_push(ret);
    copy_place_exit_shim(StartObjectAtPlace(cpu.a, cpu.x, cpu.y));
    jsr_pop();
}

/* A free slot looked for from $30 (or $20) and, when found, an object of a kind started in it. */
static void start_in_free_slot(bool from20, uint8_t kind, uint16_t find_ret, uint16_t start_ret)
{
    uint8_t found_a, to;
    bool found;

    jsr_push(find_ret);
    to = from20 ? FindFreeSlotFrom20(&found_a, &found) : FindFreeSlotFrom30(&found_a, &found);
    find_slot_exit_shim(to, found_a, found);
    jsr_pop();
    if (found) {
        start_at_place(kind, start_ret);
    }
}

/* LDA #script, LDX #slot, JSR StartScriptSlotKind1, between PHX and PLX. */
static void script_kept_x(uint8_t number, uint8_t at_slot, uint16_t ret)
{
    push8(cpu.x);
    lda(number);
    cpu.x = at_slot;
    abi_nz(at_slot);
    jsr_push(ret);
    script_slot_exit_shim(StartScriptSlotKind1(cpu.a, cpu.x));
    jsr_pop();
    cpu.x = pull8();
    abi_nz(cpu.x);
}

static void move_x(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityX(slot));
    jsr_pop();
}

static void move_y(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    axis_move_exit_shim(ApplyVelocityY(slot));
    jsr_pop();
}

static void gravity(uint8_t slot, uint16_t ret)
{
    jsr_push(ret);
    gravity_exit_shim(ApplyGravity(slot));
    jsr_pop();
}

static void load_record_tail_(uint8_t slot)
{
    uint8_t nz, last;
    bool carry;

    last = LoadObjectRecord(slot, &nz, &carry);
    load_record_exit_shim(last, nz, carry);
}

/* DEC of a column: true when it reached zero. */
static bool counted_out(uint8_t left)
{
    abi_nz(left);
    return left == 0;
}

/* BNE after StepObjectFrames: true when the list ended. */
static bool list_ended(void)
{
    return (cpu.p & P_Z) != 0;
}

/*
 * @routine 12DE:1A3 MoveYThenClearOffScreen12DE
 * @run 71:C9A3
 * WHAT IT DOES: Moves along Y; off the screen above, or below and still falling, the object is cleared (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void MoveYThenClearOffScreen12DE(uint8_t slot)
{
    move_y(slot, 0xC9A6);
    lda(RD8(OBJ->y_hi[slot]));
    if (cpu.a == 0) {
        return;
    }
    if ((cpu.a & 0x80) != 0) {
        ClearObject(slot);
        return;
    }
    lda(RD8(OBJ->vy_hi[slot]));
    if ((cpu.a & 0x80) == 0) {
        ClearObject(slot);
    }
}

void abi_MoveYThenClearOffScreen12DE(void)
{
    MoveYThenClearOffScreen12DE(cpu.x);
}

/*
 * @routine 13FB:651 ListThenState213FB_651
 * @run 70:B651
 * WHAT IT DOES: Plays the frame list at $B667; when it ends, state 2 with substate 0 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenState213FB_651(uint8_t slot)
{
    step_list(slot, 0xB667, true, 0xB65C);
    if (!list_ended()) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenState213FB_651(void)
{
    ListThenState213FB_651(cpu.x);
}

/*
 * @routine 13FB:6F0 ListThenState213FB_6F0
 * @run 70:B6F0
 * WHAT IT DOES: Plays the frame list at $B706; when it ends, state 2 with substate 0 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenState213FB_6F0(uint8_t slot)
{
    step_list(slot, 0xB706, true, 0xB6FB);
    if (!list_ended()) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenState213FB_6F0(void)
{
    ListThenState213FB_6F0(cpu.x);
}

/*
 * @routine 13FC:515 ListThenState213FC_515
 * @run 70:BD15
 * WHAT IT DOES: Plays the frame list at $BD2B; when it ends, state 2 with substate 0 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenState213FC_515(uint8_t slot)
{
    step_list(slot, 0xBD2B, true, 0xBD20);
    if (!list_ended()) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
    WR8(OBJ->substate[slot], 0);
}

void abi_ListThenState213FC_515(void)
{
    ListThenState213FC_515(cpu.x);
}

/*
 * @routine 161B:303 FollowSource161B
 * @run 70:B303
 * WHAT IT DOES: Keeps the place of the slot OBJ_WORK_D names: cleared once it is empty, state on once it is not kind $2C (Stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FollowSource161B(uint8_t slot)
{
    cpu.y = RD8(OBJ->work_d[slot]);
    abi_nz(cpu.y);
    jsr_push(0xB309);
    copy_place_exit_shim(CopyObjectPlaceBack(cpu.y, slot));
    jsr_pop();
    lda(RD8(OBJ->kind[cpu.y]));
    if (cpu.a == 0) {
        ClearObject(slot);
        return;
    }
    cmp(cpu.a, 0x2C);
    if (cpu.a != 0x2C) {
        abi_nz(INC8(OBJ->state[slot]));
    }
}

void abi_FollowSource161B(void)
{
    FollowSource161B(cpu.x);
}

/*
 * @routine 18DB:4CA ListThenWait18DB
 * @run 70:B4CA
 * WHAT IT DOES: Plays the frame list at $B4E0; when it ends, OBJ_WORK_E 48 and substate on (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenWait18DB(uint8_t slot)
{
    step_list(slot, 0xB4E0, true, 0xB4D5);
    if (!list_ended()) {
        return;
    }
    lda(0x30);
    WR8(OBJ->work_e[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenWait18DB(void)
{
    ListThenWait18DB(cpu.x);
}

/*
 * @routine 0E0C:6C1 SpawnWithSoundAtRandom0E0C
 * @run 70:BEC1
 * WHAT IT DOES: One frame in 16: sound $13 and a kind $9B object in a free slot (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnWithSoundAtRandom0E0C(uint8_t slot)
{
    jsr_push(0xBEC4);
    random_exit_shim(NextRandom());
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x0F));
    if (cpu.a != 0) {
        return;
    }
    {
        uint8_t found_a, to;
        bool found;

        jsr_push(0xBECB);
        to = FindFreeSlotFrom30(&found_a, &found);
        find_slot_exit_shim(to, found_a, found);
        jsr_pop();
        if (!found) {
            return;
        }
    }
    sound(0x13, 0xBED2);
    start_at_place(0x9B, 0xBED7);
}

void abi_SpawnWithSoundAtRandom0E0C(void)
{
    SpawnWithSoundAtRandom0E0C(cpu.x);
}

/*
 * @routine 117D:13E ListThenSubstate117D
 * @run 71:C13E
 * WHAT IT DOES: Plays the frame list at $C155; when it ends, the animation restarted and substate on (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenSubstate117D(uint8_t slot)
{
    step_list(slot, 0xC155, false, 0xC149);
    if (!list_ended()) {
        return;
    }
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenSubstate117D(void)
{
    ListThenSubstate117D(cpu.x);
}

/*
 * @routine 117F:694 SoundE2AndClearNext117F
 * @run 71:D694
 * WHAT IT DOES: The hit box off, sound $E2, the next slot cleared, OBJ_WORK_D 64, substate on (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SoundE2AndClearNext117F(uint8_t slot)
{
    WR8(OBJ->hitbox_on[slot], 0);
    sound(0xE2, 0xD69C);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xD6A2);
    lda(0x40);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_SoundE2AndClearNext117F(void)
{
    SoundE2AndClearNext117F(cpu.x);
}

/*
 * @routine 12DC:50F FaceByBitThenNear12DC
 * @run 70:BD0F
 * WHAT IT DOES: Faces by A's bit 0; unless $BCDB leaves C set, an object $70 dots or more from the player goes to state 2 (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FaceByBitThenNear12DC(uint8_t slot)
{
    lda((uint8_t)(cpu.a & 0x01));
    WR8(OBJ->facing_left[slot], cpu.a);
    call_routine(0xBD17, 0xBCDB);
    if ((cpu.p & P_C) != 0) {
        return;
    }
    jsr_push(0xBD1C);
    abi_ended(ended_of_distance(DistanceXToPlayer(slot)));
    jsr_pop();
    cmp(cpu.a, 0x70);
    if (cpu.a < 0x70) {
        return;
    }
    lda(0x02);
    WR8(OBJ->state[slot], cpu.a);
}

void abi_FaceByBitThenNear12DC(void)
{
    FaceByBitThenNear12DC(cpu.x);
}

/*
 * @routine 13FD:2D4 LoadThenStateOrDrop13FD
 * @run 71:C2D4
 * WHAT IT DOES: Loads the record; a start argument means state 3, else the object is put at Y $70 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadThenStateOrDrop13FD(uint8_t slot)
{
    load_record(slot, 0xC2D7);
    lda(RD8(OBJ->start_arg[slot]));
    if (cpu.a != 0) {
        lda(0x03);
        WR8(OBJ->state[slot], cpu.a);
        return;
    }
    WR8(OBJ->y_hi[slot], 0);
    lda(0x70);
    WR8(OBJ->y_lo[slot], cpu.a);
}

void abi_LoadThenStateOrDrop13FD(void)
{
    LoadThenStateOrDrop13FD(cpu.x);
}

/*
 * @routine 16CB:66F CountThenScript97_16CB
 * @run 70:B66F
 * WHAT IT DOES: Counts OBJ_WORK_D; at zero script $97 starts, the count 192, substate on (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenScript97_16CB(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    script_kept_x(0x97, 0x05, 0xB67C);
    lda(0xC0);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_CountThenScript97_16CB(void)
{
    CountThenScript97_16CB(cpu.x);
}

/*
 * @routine 18DB:1D2 LoadSpawnScript18DB
 * @run 70:B1D2
 * WHAT IT DOES: Loads the record, starts a kind $25 object in slot $11 and script $82 (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadSpawnScript18DB(uint8_t slot)
{
    load_record(slot, 0xB1D5);
    cpu.y = 0x11;
    abi_nz(cpu.y);
    clear_slot_in_y(0xB1DA);
    start_at_place(0x25, 0xB1DF);
    script_kept_x(0x82, 0x08, 0xB1E7);
}

void abi_LoadSpawnScript18DB(void)
{
    LoadSpawnScript18DB(cpu.x);
}

/*
 * @routine 18DB:327 CountThenSound91_18DB
 * @run 70:B327
 * WHAT IT DOES: Counts OBJ_WORK_E; at zero sound $91 and the substate the table gives for OBJ_WORK_F (Stage 10's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenSound91_18DB(uint8_t slot)
{
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    sound(0x91, 0xB331);
    lda(RD8(OBJ->work_f[slot]));
    lda((uint8_t)(cpu.a & 0x0F));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB33E + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
}

void abi_CountThenSound91_18DB(void)
{
    CountThenSound91_18DB(cpu.x);
}

/*
 * @routine 088B:543 StartRow088B_543
 * @run 70:B543
 * WHAT IT DOES: Starts 3 kind $2B objects at this place, each told its number in OBJ_WORK_D (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartRow088B_543(uint8_t slot)
{
    uint8_t found_a, to;
    bool found;

    cpu.y = 0;
    do {
        WR8(ZP->spread_count, cpu.y);
        jsr_push(0xB549);
        to = FindFreeSlotFrom30(&found_a, &found);
        find_slot_exit_shim(to, found_a, found);
        jsr_pop();
        start_at_place(0x2B, 0xB54E);
        lda(RD8(ZP->spread_count));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        cpu.y = RD8(ZP->spread_count);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 3);
    } while (cpu.y < 3);
}

void abi_StartRow088B_543(void)
{
    StartRow088B_543(cpu.x);
}

/*
 * @routine 088B:55B StartRow088B_55B
 * @run 70:B55B
 * WHAT IT DOES: Starts 16 kind $2C objects at this place, each told its number in OBJ_WORK_D (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartRow088B_55B(uint8_t slot)
{
    uint8_t found_a, to;
    bool found;

    cpu.y = 0;
    do {
        WR8(ZP->spread_count, cpu.y);
        jsr_push(0xB561);
        to = FindFreeSlotFrom20(&found_a, &found);
        find_slot_exit_shim(to, found_a, found);
        jsr_pop();
        start_at_place(0x2C, 0xB566);
        lda(RD8(ZP->spread_count));
        WR8(OBJ->work_d[cpu.y], cpu.a);
        cpu.y = RD8(ZP->spread_count);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 16);
    } while (cpu.y < 16);
}

void abi_StartRow088B_55B(void)
{
    StartRow088B_55B(cpu.x);
}

/*
 * @routine 088C:017 LoadSpeedByWorkD088C
 * @run 70:B817
 * WHAT IT DOES: Loads the record, takes the X speed and OBJ_WORK_F the table gives for OBJ_WORK_D, and moves (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadSpeedByWorkD088C(uint8_t slot)
{
    load_record(slot, 0xB81A);
    lda(RD8(OBJ->work_d[slot]));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB82F + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB830 + cpu.y)));
    WR8(OBJ->work_f[slot], cpu.a);
    move_x(slot, 0xB82E);
}

void abi_LoadSpeedByWorkD088C(void)
{
    LoadSpeedByWorkD088C(cpu.x);
}

/*
 * @routine 0B4D:0CD CountThenDraw0B4D
 * @run 71:C0CD
 * WHAT IT DOES: Counts OBJ_WORK_D; at zero frame 5, tile script 6 plus the start argument, state on (secret stage 06's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void CountThenDraw0B4D(uint8_t slot)
{
    Sum sum;
    TileRun run;

    if (!counted_out(DEC8(OBJ->work_d[slot]))) {
        return;
    }
    lda(0x05);
    WR8(OBJ->frame[slot], cpu.a);
    lda(RD8(OBJ->start_arg[slot]));
    sum = add_bytes(cpu.a, 0x06, false);
    lda(sum.value);
    abi_flag(P_C, sum.carry);
    abi_flag(P_V, sum.overflow);
    cpu.y = 0;
    jsr_push(0xC0E1);
    run = DrawTileScript(sum.value, 0);
    abi_ended(ended_of_tile_run(run, sum.value));
    jsr_pop();
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_CountThenDraw0B4D(void)
{
    CountThenDraw0B4D(cpu.x);
}

/*
 * @routine 0E0B:769 WaitForSlotSpawn0E0B
 * @run 70:B769
 * WHAT IT DOES: Frame $43; once slot $50 is empty, slot $11 cleared, OBJ_WORK_D 96, substate on (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void WaitForSlotSpawn0E0B(uint8_t slot)
{
    lda(0x43);
    WR8(OBJ->frame[slot], cpu.a);
    lda(RD8(OBJ->kind[0x50]));
    if (cpu.a != 0) {
        return;
    }
    cpu.y = 0x11;
    abi_nz(cpu.y);
    clear_slot_in_y(0xB778);
    lda(0x60);
    WR8(OBJ->work_d[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_WaitForSlotSpawn0E0B(void)
{
    WaitForSlotSpawn0E0B(cpu.x);
}

/*
 * @routine 101E:36C StartRowAndClear101E
 * @run 71:CB6C
 * WHAT IT DOES: Starts kind $C3 objects at this place in slots $11 to $18 with its flip bits, then clears itself (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void StartRowAndClear101E(uint8_t slot)
{
    cpu.y = 0x11;
    abi_nz(cpu.y);
    do {
        clear_slot_in_y(0xCB71);
        start_at_place(0xC3, 0xCB76);
        lda(RD8(OBJ->sprite_flip[slot]));
        WR8(OBJ->sprite_flip[cpu.y], cpu.a);
        cpu.y = (uint8_t)(cpu.y + 1);
        cmp(cpu.y, 0x19);
    } while (cpu.y < 0x19);
    ClearObject(slot);
}

void abi_StartRowAndClear101E(void)
{
    StartRowAndClear101E(cpu.x);
}

/*
 * @routine 117D:029 LandInFront117D
 * @run 71:C029
 * WHAT IT DOES: Runs $BBD8 and falls with a front probe; on landing OBJ_WORK_G 12, frame $3B, substate on (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LandInFront117D(uint8_t slot)
{
    call_routine(0xC02C, 0xBBD8);
    lda(0x0C);
    jsr_push(0xC031);
    mover_exit_shim(StopOnLandingFrontColumn(cpu.a, slot, cpu.y, (cpu.p & P_C) != 0), 0x01);
    jsr_pop();
    if ((cpu.p & P_C) == 0) {
        return;
    }
    lda(0x0C);
    WR8(OBJ->work_g[slot], cpu.a);
    lda(0x3B);
    WR8(OBJ->frame[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_LandInFront117D(void)
{
    LandInFront117D(cpu.x);
}

/*
 * @routine 088B:4F6 AimAtPlayer088B
 * @run 70:B4F6
 * WHAT IT DOES: Aims at the player at speed $0480, OBJ_WORK_B 36, substate on (secret stage 04's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void AimAtPlayer088B(uint8_t slot)
{
    AngleExit angle;

    jsr_push(0xB4F9);
    angle = AngleToPlayer(slot);
    angle_exit_shim(angle);
    jsr_pop();
    push8(cpu.a);
    lda(0x80);
    WR8(ZP->temp00, cpu.a);
    lda(0x04);
    WR8(ZP->temp01, cpu.a);
    lda(pull8());
    jsr_push(0xB506);
    scale_exit_shim(SetVelocityFromAngle(cpu.a, slot));
    jsr_pop();
    lda(0x24);
    WR8(OBJ->work_b[slot], cpu.a);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_AimAtPlayer088B(void)
{
    AimAtPlayer088B(cpu.x);
}

/*
 * @routine 0E0C:478 LoadOrListThenClear0E0C
 * @run 70:BC78
 * WHAT IT DOES: In state 0 loads the record; later plays the frame list at $BC91 and is cleared when it ends (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadOrListThenClear0E0C(uint8_t slot)
{
    lda(RD8(OBJ->state[slot]));
    if (cpu.a == 0) {
        load_record_tail_(slot);
        return;
    }
    step_list(slot, 0xBC91, true, 0xBC8B);
    if (!list_ended()) {
        return;
    }
    ClearObject(slot);
}

void abi_LoadOrListThenClear0E0C(void)
{
    LoadOrListThenClear0E0C(cpu.x);
}

/*
 * @routine 0EC0:1DC FallListThenClear0EC0
 * @run 71:D9DC
 * WHAT IT DOES: Attribute $8B, moves along Y, plays the frame list at $D9F5 and is cleared when it ends (Stage 09's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void FallListThenClear0EC0(uint8_t slot)
{
    lda(0x8B);
    WR8(OBJ->sprite_attr[slot], cpu.a);
    move_y(slot, 0xD9E4);
    step_list(slot, 0xD9F5, true, 0xD9EF);
    if (!list_ended()) {
        return;
    }
    ClearObject(slot);
}

void abi_FallListThenClear0EC0(void)
{
    FallListThenClear0EC0(cpu.x);
}

/*
 * @routine 117F:05A LoadHitBoxAndSpawn117F
 * @run 71:D05A
 * WHAT IT DOES: Loads the record, hit-tested, OBJ_WORK_E 96, a kind $3D object in the next slot (Stage 0B's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void LoadHitBoxAndSpawn117F(uint8_t slot)
{
    load_record(slot, 0xD05D);
    lda(0x02);
    WR8(OBJ->hitbox_on[slot], cpu.a);
    lda(0x60);
    WR8(OBJ->work_e[slot], cpu.a);
    lda(cpu.x);
    cpu.y = cpu.a;
    cpu.y = (uint8_t)(cpu.y + 1);
    abi_nz(cpu.y);
    clear_slot_in_y(0xD06D);
    start_at_place(0x3D, 0xD072);
}

void abi_LoadHitBoxAndSpawn117F(void)
{
    LoadHitBoxAndSpawn117F(cpu.x);
}

/*
 * @routine 12DC:5AF SpawnWhileEarly12DC
 * @run 70:BDAF
 * WHAT IT DOES: In the first five animation steps, every eighth count of OBJ_WORK_H starts a kind $2E object (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnWhileEarly12DC(uint8_t slot)
{
    lda(RD8(OBJ->anim_step[slot]));
    cmp(cpu.a, 0x05);
    if (cpu.a >= 0x05) {
        return;
    }
    lda(RD8(OBJ->work_h[slot]));
    lda((uint8_t)(cpu.a & 0x07));
    if (cpu.a != 0) {
        return;
    }
    start_in_free_slot(false, 0x2E, 0xBDC0, 0xBDC7);
}

void abi_SpawnWhileEarly12DC(void)
{
    SpawnWhileEarly12DC(cpu.x);
}

/*
 * @routine 13FC:405 NextSubstateByTable13FC
 * @run 70:BC05
 * WHAT IT DOES: The animation restarted, the substate the table gives for OBJ_WORK_L; substate 1 sounds $26 (secret stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void NextSubstateByTable13FC(uint8_t slot)
{
    restart_animation(slot);
    cpu.y = RD8(OBJ->work_l[slot]);
    abi_nz(cpu.y);
    lda(rd8((uint16_t)(0xBC1E + cpu.y)));
    WR8(OBJ->substate[slot], cpu.a);
    cmp(cpu.a, 0x01);
    if (cpu.a != 0x01) {
        return;
    }
    sound(0x26, 0xBC1D);
}

void abi_NextSubstateByTable13FC(void)
{
    NextSubstateByTable13FC(cpu.x);
}

/*
 * @routine 16CB:58F SpawnFromCount32_16CB
 * @run 70:B58F
 * WHAT IT DOES: From OBJ_WORK_D 32, on even counts of OBJ_WORK_J starts a kind $22 object (secret stage 0E's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpawnFromCount32_16CB(uint8_t slot)
{
    lda(RD8(OBJ->work_d[slot]));
    cmp(cpu.a, 0x20);
    if (cpu.a < 0x20) {
        return;
    }
    lda(RD8(OBJ->work_j[slot]));
    lda((uint8_t)(cpu.a & 0x01));
    if (cpu.a != 0) {
        return;
    }
    start_in_free_slot(false, 0x22, 0xB5A0, 0xB5A7);
}

void abi_SpawnFromCount32_16CB(void)
{
    SpawnFromCount32_16CB(cpu.x);
}

/*
 * @routine 0CAC:15E ListThenWhenSlot24Gone0CAC
 * @run 70:B95E
 * WHAT IT DOES: Plays the frame list at $B8AB; once slot $18 is empty, the animation restarted and substate on (secret stage 07's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListThenWhenSlot24Gone0CAC(uint8_t slot)
{
    step_list(slot, 0xB8AB, true, 0xB969);
    lda(RD8(OBJ->kind[0x18]));
    if (cpu.a != 0) {
        return;
    }
    restart_animation(slot);
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListThenWhenSlot24Gone0CAC(void)
{
    ListThenWhenSlot24Gone0CAC(cpu.x);
}

/*
 * @routine 0CAC:145 ListAndSink0CAC
 * @run 70:B945
 * WHAT IT DOES: Plays the frame list at $B8AB while sinking a dot a frame; from Y $70 the substate on (secret stage 07's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListAndSink0CAC(uint8_t slot)
{
    step_list(slot, 0xB8AB, true, 0xB950);
    INC8(OBJ->y_lo[slot]);
    lda(RD8(OBJ->y_lo[slot]));
    cmp(cpu.a, 0x70);
    if (cpu.a < 0x70) {
        return;
    }
    abi_nz(INC8(OBJ->substate[slot]));
}

void abi_ListAndSink0CAC(void)
{
    ListAndSink0CAC(cpu.x);
}

/*
 * @routine 0E0C:32D ListMoveCount0E0C_32D
 * @run 70:BB2D
 * WHAT IT DOES: Plays the frame list at $B972, moves with gravity and counts OBJ_WORK_E; at zero cleared (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListMoveCount0E0C_32D(uint8_t slot)
{
    step_list(slot, 0xB972, true, 0xBB38);
    gravity(slot, 0xBB3B);
    move_x(slot, 0xBB3E);
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    ClearObject(slot);
}

void abi_ListMoveCount0E0C_32D(void)
{
    ListMoveCount0E0C_32D(cpu.x);
}

/*
 * @routine 0E0C:4E0 ListMoveCount0E0C_4E0
 * @run 70:BCE0
 * WHAT IT DOES: Plays the frame list at $B972, moves with gravity and counts OBJ_WORK_E; at zero the state counted on (secret stage 08's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void ListMoveCount0E0C_4E0(uint8_t slot)
{
    step_list(slot, 0xB972, true, 0xBCEB);
    move_x(slot, 0xBCEE);
    gravity(slot, 0xBCF1);
    if (!counted_out(DEC8(OBJ->work_e[slot]))) {
        return;
    }
    abi_nz(INC8(OBJ->state[slot]));
}

void abi_ListMoveCount0E0C_4E0(void)
{
    ListMoveCount0E0C_4E0(cpu.x);
}

/*
 * @routine 101B:73B SpeedByPlayerSide101B
 * @run 70:B73B
 * WHAT IT DOES: Faces the player and takes the X speed of that side and OBJ_WORK_E (Stage 0A's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void SpeedByPlayerSide101B(uint8_t slot)
{
    jsr_push(0xB73E);
    side_of_player_exit_shim(SideOfPlayer(slot));
    jsr_pop();
    lda((uint8_t)(cpu.a & 0x01));
    WR8(OBJ->facing_left[slot], cpu.a);
    lda((uint8_t)(cpu.a ^ RD8(OBJ->work_e[slot])));
    abi_flag(P_C, (cpu.a & 0x80) != 0);
    lda((uint8_t)(cpu.a << 1));
    cpu.y = cpu.a;
    lda(rd8((uint16_t)(0xB756 + cpu.y)));
    WR8(OBJ->vx_hi[slot], cpu.a);
    lda(rd8((uint16_t)(0xB755 + cpu.y)));
    WR8(OBJ->vx_lo[slot], cpu.a);
}

void abi_SpeedByPlayerSide101B(void)
{
    SpeedByPlayerSide101B(cpu.x);
}

/*
 * @routine 12DE:648 RunThenCountClear12DE
 * @run 71:CE48
 * WHAT IT DOES: Runs $CE5E over patterns 4 to $11 while OBJ_WORK_G counts; at zero the object is cleared (Stage 0C's overlay).
 * HOW: The calls in turn.
 * WAITS: no.
 * Entry: X object. Exit: as the last call, compare or load left them.
 */
void RunThenCountClear12DE(uint8_t slot)
{
    lda(0x04);
    wr8(WorkByteA, cpu.a);
    lda(0x12);
    wr8(WorkByteB, cpu.a);
    call_routine(0xCE55, 0xCE5E);
    if (!counted_out(DEC8(OBJ->work_g[slot]))) {
        return;
    }
    jsr_push(0xCE5D);
    ClearObject(slot);
    jsr_pop();
}

void abi_RunThenCountClear12DE(void)
{
    RunThenCountClear12DE(cpu.x);
}
