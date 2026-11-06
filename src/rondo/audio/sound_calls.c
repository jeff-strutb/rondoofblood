/* Calling the sound driver, which lives in banks of its own. */
#include "machine.h"
#include "ram.h"

/* The driver's three banks, as offsets from the first Super RAM bank. */
enum { SOUND_BANK_OFFSET = 0x0D, MPR4 = 0x10, MPR5 = 0x20, MPR6 = 0x40, SOUND_CALL_BUSY = 0x80 };

/* Maps the driver, and gives the last of its banks. */
static uint8_t map_sound_banks(void)
{
    uint8_t bank = (uint8_t)(SOUND_BANK_OFFSET + rd8(zRamBaseBank));

    tam(MPR4, bank);
    tam(MPR5, (uint8_t)(bank + 1));
    tam(MPR6, (uint8_t)(bank + 2));
    return (uint8_t)(bank + 2);
}

/*
 * Enters the driver, which is original code, with A and the flags the assembly gives it:
 * C and V from the sum of the banks, since its entries do not all overwrite them.
 */
static void enter_sound_driver_shim(uint16_t entry, uint8_t a)
{
    Alu sum = adc8(SOUND_BANK_OFFSET, rd8(zRamBaseBank), false);

    sum.a = a;
    sum.nz = a;
    abi_alu(sum);
    call_original(entry);
}

/*
 * @routine 0043:33F SoundStopAll
 * @run 69:433F
 * WHAT IT DOES: Stops every sound channel.
 * HOW: Maps the driver's banks, calls its stop entry, restores the banks from
 * the stack.
 * Entry: nothing. Exit: A the bank restored to MPR4, X and Y kept, N and Z from
 * X, C and V as the driver left them.
 */
uint8_t SoundStopAll(void)
{
    uint8_t mpr4 = tma(MPR4);
    uint8_t mpr5 = tma(MPR5);
    uint8_t mpr6 = tma(MPR6);

    enter_sound_driver_shim(SoundDrvStopAll, map_sound_banks());
    tam(MPR6, mpr6);
    tam(MPR5, mpr5);
    tam(MPR4, mpr4);
    return mpr4;
}

void abi_SoundStopAll(void)
{
    uint8_t x = cpu.x;
    uint8_t y = cpu.y;

    cpu.a = SoundStopAll();
    cpu.x = x;
    cpu.y = y;
    abi_nz(x);
}

/*
 * @routine 0043:366 SoundFrameTick
 * @run 69:4366
 * WHAT IT DOES: Runs one frame of the sound driver, unless a command is going
 * in from the main program at this moment.
 * HOW: zSoundBusy nonzero returns. Otherwise sets it, maps the driver's banks,
 * calls its tick, restores the banks and clears it.
 * Entry: frame interrupt. Exit: busy, A zSoundBusy, N and Z from it. Else A the
 * bank restored to MPR4, N and Z from it; X, Y, C and V as the driver left them.
 */
uint8_t SoundFrameTick(void)
{
    uint8_t busy = rd8(zSoundBusy);
    uint8_t mpr4;
    uint8_t mpr5;
    uint8_t mpr6;

    if (busy != 0)
        return busy;
    wr8(zSoundBusy, SOUND_CALL_BUSY);
    mpr4 = tma(MPR4);
    mpr5 = tma(MPR5);
    mpr6 = tma(MPR6);
    enter_sound_driver_shim(SoundDrvTick, map_sound_banks());
    tam(MPR6, mpr6);
    tam(MPR5, mpr5);
    tam(MPR4, mpr4);
    wr8(zSoundBusy, 0);
    return mpr4;
}

void abi_SoundFrameTick(void)
{
    cpu.a = SoundFrameTick();
    abi_nz(cpu.a);
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
 * Entry: A command. Exit, dropped: A kept, N and V bits 7 and 6 of zSoundBusy,
 * Z from A and it. Exit, taken: A the MPR4 bank restored, N and Z from it; X, Y,
 * C and V as the driver left them.
 */
bool SoundCommand(uint8_t command, uint8_t *busy_or_bank)
{
    uint8_t byte = rd8(zSoundBusy);

    if (byte & 0x80) {
        *busy_or_bank = byte;
        return false;
    }
    wr8(zSoundBusy, SOUND_CALL_BUSY);
    byte = tma(MPR4);
    wr8(SavedMprA, byte);
    byte = tma(MPR5);
    wr8(SavedMprB, byte);
    byte = tma(MPR6);
    wr8(SavedMprC, byte);
    map_sound_banks();
    enter_sound_driver_shim(SoundDrvCommand, command);
    byte = rd8(SavedMprC);
    tam(MPR6, byte);
    byte = rd8(SavedMprB);
    tam(MPR5, byte);
    byte = rd8(SavedMprA);
    tam(MPR4, byte);
    wr8(zSoundBusy, 0);
    *busy_or_bank = byte;
    return true;
}

void abi_SoundCommand(void)
{
    uint8_t command = cpu.a;
    uint8_t value;

    if (SoundCommand(command, &value)) {
        cpu.a = value;
        abi_nz(value);
    } else {
        abi_flag(P_N, (value & 0x80) != 0);
        abi_flag(P_V, (value & 0x40) != 0);
        abi_flag(P_Z, (command & value) == 0);
    }
}
