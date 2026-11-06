/*
 * The System Card's BIOS, as typed functions: the one header a caller includes. Names are
 * Hudson's where public notes give them, not yet confirmed from anything but the code.
 */
#ifndef SYSCARD_H
#define SYSCARD_H

#include <stdbool.h>
#include <stdint.h>

/* What ex_memopen found: the Super System Card's RAM, and its first bank. */
typedef struct {
    bool present;
    uint8_t bank;
    uint8_t last_read;  /* the signature byte a failed check last read */
    uint8_t expected;   /* what it was compared with */
} MemOpen;

/* What ad_stat reads from the ADPCM hardware. */
typedef struct {
    uint8_t busy;
    uint8_t playing;
} AdStat;

/* What cd_subrd read: a subcode byte, if one was waiting. */
typedef struct {
    bool ready;
    uint8_t status;
    uint8_t byte;
} CdSubcodeRead;

/* What cd_pcmrd latched: the sample's two bytes, and whether the port has more. */
typedef struct {
    uint8_t low;
    uint8_t high;
    uint8_t more;
} CdPcmRead;

/* The reset port before cd_reset's pulse, and what it was left holding. */
typedef struct {
    uint8_t before;
    uint8_t after;
} CdReset;

/* How sending a command ended, and the values it worked with. */
typedef enum {
    CD_SENT_PENDING,   /* something else is running: it does not wait on the drive */
    CD_SENT_NO_WAIT,   /* a command that does not wait on the drive */
    CD_SENT_ANSWERED,  /* the drive's phase moved on */
    CD_SENT_TIMED_OUT
} CdSentHow;

typedef struct {
    CdSentHow how;
    uint8_t sent;        /* command bytes taken by the drive */
    uint8_t pending;     /* what said something else is running */
    uint8_t flag_byte;   /* the port last tested or changed: its bits 7 and 6 */
    uint8_t phase;       /* the phase that ended the wait */
    uint8_t first_tick;  /* the frame count when the wait began */
    uint8_t last_tick;   /* and when it last looked */
    bool waited_again;
} CdSent;

/* The status byte a command ended with, and the bus's last state. */
typedef struct {
    uint8_t status;
    uint8_t last_bus;
} CdStatusRead;

/* How finishing a command ended. */
typedef struct {
    bool timed_out;
    CdReset reset;
    CdStatusRead read;
} CdFinish;

CdSent cd_send_command(void);
CdStatusRead cd_read_status(void);
CdFinish cd_finish_command(void);
uint8_t cd_ack_pulse(void);
CdReset cd_reset(void);
CdSubcodeRead cd_subrd(void);
CdPcmRead cd_pcmrd(uint8_t latch);
uint16_t ma_mul8u(uint8_t al, uint8_t bl);
void cd_fade(uint8_t type);
uint8_t ad_stop(void);
AdStat ad_stat(void);
uint8_t ex_scrsiz(uint8_t size);
uint8_t ex_dotmod(uint8_t mode);
MemOpen ex_memopen(void);
void ex_setvec(uint8_t which, uint8_t low, uint8_t high);

#endif
