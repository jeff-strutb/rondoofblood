/* Calling the sound driver, which lives in banks of its own. */
#include "machine.h"
#include "ram.h"

/* The driver's three banks, as offsets from the first Super RAM bank. */
enum { SOUND_BANK_OFFSET = 0x0D, MPR4 = 0x10, MPR5 = 0x20, MPR6 = 0x40, SOUND_CALL_BUSY = 0x80 };

/*
 * Maps the driver. The driver is entered with what this leaves in A and the
 * flags: its entries do not all overwrite C and V, so they reach the caller.
 */
static void map_sound_banks(void)
{
    uint8_t base = rd8(zRamBaseBank);
    Alu bank = adc8(SOUND_BANK_OFFSET, base, false);

    tam(MPR4, bank.a);
    tam(MPR5, (uint8_t)(bank.a + 1));
    tam(MPR6, (uint8_t)(bank.a + 2));
    cpu.a = (uint8_t)(bank.a + 2);
    abi_alu((Alu){ cpu.a, cpu.a, bank.c, bank.v, bank.v_set, false });
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
    uint8_t x = cpu.x;
    uint8_t y = cpu.y;

    map_sound_banks();
    call_original(SoundDrvStopAll);
    tam(MPR6, mpr6);
    tam(MPR5, mpr5);
    tam(MPR4, mpr4);
    cpu.x = x;
    cpu.y = y;
    return mpr4;
}

void abi_SoundStopAll(void)
{
    cpu.a = SoundStopAll();
    abi_nz(cpu.x);
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
    map_sound_banks();
    call_original(SoundDrvTick);
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
    cpu.a = command;
    abi_nz(command);
    call_original(SoundDrvCommand);
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
