/* Calling the sound driver, which lives in banks of its own. */
#include "machine.h"
#include "ram.h"
#include "syscard.h"
#include "routines.h"

/* The driver's three banks, as offsets from the first Super RAM bank. */
enum { SOUND_BANK_OFFSET = 0x0D, MPR4 = 0x10, MPR5 = 0x20, MPR6 = 0x40, SOUND_CALL_BUSY = 0x80 };

/* Maps the driver: the add that finds its first bank, whose C and V the driver is entered with. */
static Sum map_sound_banks(void)
{
    Sum bank = add_bytes(SOUND_BANK_OFFSET, rd8(zRamBaseBank), false);

    tam(MPR4, bank.value);
    tam(MPR5, (uint8_t)(bank.value + 1));
    tam(MPR6, (uint8_t)(bank.value + 2));
    return bank;
}

/* The registers the driver is entered with: A as given, N and Z from it, C and V from the map. */
static void driver_entry_shim(uint8_t a, Sum bank)
{
    cpu.a = a;
    abi_nz(a);
    abi_flag(P_C, bank.carry);
    abi_flag(P_V, bank.overflow);
}

/*
 * @routine 0043:33F SoundStopAll
 * @run 69:433F
 * WHAT IT DOES: Stops every sound channel.
 * HOW: Maps the driver's banks, calls its stop entry, restores the banks from
 * the stack.
 * WAITS: no.
 * Entry: X and Y are pushed first and pulled last (a typed caller passes its own). Exit: A the
 * bank restored to MPR4, X and Y kept, N and Z from X, C and V as the driver left them. liftcmp
 * agrees for the first 32 accesses; its first difference is inside SoundDriverStopAll (the
 * driver's own call frames).
 */
StopCall SoundStopAll(uint8_t kept_x, uint8_t kept_y)
{
    StopCall call;
    uint8_t mpr4, mpr5, mpr6;

    push8(kept_x);
    push8(kept_y);
    mpr4 = tma(MPR4);
    push8(mpr4);
    mpr5 = tma(MPR5);
    push8(mpr5);
    mpr6 = tma(MPR6);
    push8(mpr6);
    call.bank = map_sound_banks();
    jsr_push(0x435A);
    SoundDriverStopAll();
    jsr_pop();
    mpr6 = pull8();
    tam(MPR6, mpr6);
    mpr5 = pull8();
    tam(MPR5, mpr5);
    call.mpr4 = pull8();
    tam(MPR4, call.mpr4);
    call.y = pull8();
    call.x = pull8();
    return call;
}

void stop_call_exit_shim(StopCall call)
{
    driver_entry_shim((uint8_t)(call.bank.value + 2), call.bank);
    cpu.a = call.mpr4;
    cpu.x = call.x;
    cpu.y = call.y;
    abi_nz(call.x);
}

void abi_SoundStopAll(void)
{
    stop_call_exit_shim(SoundStopAll(cpu.x, cpu.y));
}

/*
 * @routine 0043:366 SoundFrameTick
 * @run 69:4366
 * WHAT IT DOES: Runs one frame of the sound driver, unless a command is going
 * in from the main program at this moment.
 * HOW: zSoundBusy nonzero returns. Otherwise sets it, maps the driver's banks,
 * calls its tick, restores the banks and clears it.
 * WAITS: no.
 * Entry: frame interrupt. Exit: busy, A zSoundBusy, N and Z from it. Else A the
 * bank restored to MPR4, N and Z from it; X, Y, C and V as the driver left them.
 */
TickCall SoundFrameTick(void)
{
    TickCall call;
    uint8_t mpr4, mpr5, mpr6;

    call.busy = rd8(zSoundBusy);
    if (call.busy != 0)
        return call;
    wr8(zSoundBusy, SOUND_CALL_BUSY);
    mpr4 = tma(MPR4);
    push8(mpr4);
    mpr5 = tma(MPR5);
    push8(mpr5);
    mpr6 = tma(MPR6);
    push8(mpr6);
    call.bank = map_sound_banks();
    jsr_push(0x4387);
    call.tick = SoundDriverTick();
    jsr_pop();
    mpr6 = pull8();
    tam(MPR6, mpr6);
    mpr5 = pull8();
    tam(MPR5, mpr5);
    call.mpr4 = pull8();
    tam(MPR4, call.mpr4);
    wr8(zSoundBusy, 0);
    return call;
}

void abi_SoundFrameTick(void)
{
    TickCall call = SoundFrameTick();

    if (call.busy != 0) {
        cpu.a = call.busy;
        abi_nz(call.busy);
        return;
    }
    driver_entry_shim((uint8_t)(call.bank.value + 2), call.bank);
    driver_tick_exit_shim(call.tick);
    cpu.a = call.mpr4;
    abi_nz(call.mpr4);
}

/*
 * @routine 0043:393 SoundCommand
 * @run 69:4393
 * WHAT IT DOES: Gives the sound driver a command: a sound to play, or $FF stop,
 * $FE fade, $FD and $FC to set and clear a flag of the driver's. Dropped if
 * another command is going in.
 * HOW: Bit 7 of zSoundBusy set returns. Otherwise sets it, which also holds off
 * the frame tick; keeps the three MPRs in RAM, maps the driver and calls its
 * command entry with the command in A.
 * WAITS: no.
 * Entry: A command. Exit, dropped: A kept, N and V bits 7 and 6 of zSoundBusy,
 * Z from A and it. Exit, taken: A the MPR4 bank restored, N and Z from it; X, Y,
 * C and V as the driver left them.
 */
CommandCall SoundCommand(uint8_t command)
{
    CommandCall call;
    uint8_t byte = rd8(zSoundBusy);

    call.taken = !(byte & SOUND_CALL_BUSY);
    if (!call.taken) {
        call.mpr4 = byte;
        return call;
    }
    push8(command);
    wr8(zSoundBusy, SOUND_CALL_BUSY);
    wr8(SavedMprA, tma(MPR4));
    wr8(SavedMprB, tma(MPR5));
    wr8(SavedMprC, tma(MPR6));
    call.bank = map_sound_banks();
    command = pull8();
    jsr_push(0x43BC);
    call.command = SoundDriverCommand(command);
    jsr_pop();
    tam(MPR6, rd8(SavedMprC));
    tam(MPR5, rd8(SavedMprB));
    call.mpr4 = rd8(SavedMprA);
    tam(MPR4, call.mpr4);
    wr8(zSoundBusy, 0);
    return call;
}

void sound_command_exit_shim(uint8_t command, CommandCall call)
{
    if (!call.taken) {
        abi_flag(P_N, (call.mpr4 & 0x80) != 0);
        abi_flag(P_V, (call.mpr4 & 0x40) != 0);
        abi_flag(P_Z, (command & call.mpr4) == 0);
        return;
    }
    driver_entry_shim(command, call.bank);
    driver_command_exit_shim(call.command);
    cpu.a = call.mpr4;
    abi_nz(call.mpr4);
}

void abi_SoundCommand(void)
{
    uint8_t command = cpu.a;

    sound_command_exit_shim(command, SoundCommand(command));
}
