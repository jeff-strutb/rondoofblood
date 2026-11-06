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

/* What reading bytes from the drive saw. */
typedef struct {
    bool acked;            /* a byte was taken */
    uint8_t last_control;  /* the control port at the last acknowledge */
    uint8_t answer;        /* the phase at the end, its answer bits */
    bool drained;          /* the drive had more than was asked for */
    bool drained_block;    /* and a block of it was read and dropped */
} CdDataIn;

/* A command from sending it to taking its status. */
typedef struct {
    CdSent sent;
    bool read_data;
    CdDataIn data;
    CdFinish finish;
} CdExchange;

/* What the BIOS decided about a failed command. */
typedef struct {
    bool give_up;
    uint8_t error;
    bool asked;      /* the error was one that asks the drive why */
    uint8_t sense;
    CdExchange sense_exchange;
    uint8_t sense_status;
} CdRetry;

/* What testing the unit found. */
typedef struct {
    CdExchange exchange;
    uint8_t status;
    bool failed;
    CdRetry retry;
} CdTest;

/* What cd_stat found: its own read of the bus, or a test of the unit. */
typedef struct {
    bool tested;
    uint8_t busy;
    CdTest test;
} CdStat;

/* What an earlier failed try left behind, the last that mattered. */
typedef enum { CD_LEFT_NOTHING, CD_LEFT_CLEARED, CD_LEFT_ASKED } CdLeftOver;

/* A command tried until it worked or the BIOS gave up: the last try's story. */
typedef struct {
    CdExchange exchange;
    uint8_t status;
    bool failed;
    CdRetry retry;
    CdLeftOver left_over;
} CdAttempt;

/* What cd_contnts gathered: it stops at the first answer that failed. */
typedef struct {
    CdAttempt attempt;
} CdContents;

/* What cd_pause found and did. */
typedef struct {
    CdAttempt subq;
    uint8_t playing;
    bool told_to_pause;
    CdExchange pause;
    uint8_t status;
    bool failed;
    CdRetry retry;
} CdPause;

/* How cd_search ended: left running on an interrupt, or tried to its end. */
typedef struct {
    bool on_interrupt;
    CdSent sent;
    uint8_t irq_control;
    uint8_t copied_to;
    CdAttempt attempt;
} CdSearch;

/* How taking a frame off the disc's end went. */
typedef struct {
    bool borrowed_minute;
    bool unused;
    uint8_t minutes_before;
} DiscEndStep;

/* How cd_play ended, and the values it worked with. */
typedef enum {
    CD_PLAY_FINISHED,
    CD_PLAY_GAVE_UP_SEARCHING,
    CD_PLAY_SEEK_ONLY,
    CD_PLAY_RETURNED,
    CD_PLAY_ON_INTERRUPT
} CdPlayHow;

typedef struct {
    CdPlayHow how;
    uint8_t start_mode;
    bool searched;
    CdAttempt search;
    CdSent sent;
    uint8_t sub;
    uint8_t irq_control;
    CdFinish finish;
    uint8_t status;
    bool failed;
    CdRetry retry;
    CdLeftOver left_over;
} CdPlay;

/* How a block read to memory ended. */
typedef struct {
    bool at_status;
    bool moved;
    uint8_t phase;
    uint8_t pages_left;
    uint8_t page_offset;
} CdBlockRead;

/* How a read to VRAM ended. */
typedef struct {
    CdDataIn in;
    bool at_status;
    bool moved;
    bool counted;
    uint16_t count_before;
    uint8_t pages_left;
    uint8_t words_left;
} CdVramRead;

/* What adding a disc base worked with. */
typedef struct {
    uint32_t address;
    uint32_t base;
    uint8_t offset;
} CdBaseSum;

/* How cd_read ended, and the values it worked with. */
typedef struct {
    bool bad_request;
    bool mapped;
    bool again;
    uint8_t mode;
    uint8_t count;
    uint16_t bytes_asked;
    CdBaseSum added;
    CdSent sent;
    bool to_memory;
    CdBlockRead block;
    bool dropped;
    bool to_vram;
    bool vram_slow;
    CdVramRead vram;
    bool moved_through_bank;
    uint8_t pages_left;
    uint8_t page_offset;
    CdFinish finish;
    uint8_t status;
    bool failed;
    CdRetry retry;
} CdRead;

/* What checking the backup memory's header found. */
typedef struct {
    bool formatted;
    uint8_t read;
    uint8_t expected;
    uint8_t where;
} BackupCheck;

typedef struct {
    BackupCheck check;
    uint16_t room;
    uint16_t after_header;
} BackupFree;

typedef struct {
    uint8_t page;
    uint8_t offset;
    uint8_t pages_left;
    bool whole;
} BackupSize;

typedef struct {
    bool accepted;
    uint8_t read;
    uint8_t expected;
    uint8_t where;
    BackupSize size;
} BackupFormat;

/* What a look for a file found: where it stopped, and its last step through the list. */
typedef struct {
    bool found;
    uint16_t file;
    uint8_t window;
    bool stepped;
    uint16_t last_file;
    uint16_t last_length;
} BackupFind;

uint8_t bm_map_name_window(void);
uint16_t bm_count_down(void);
BackupFind bm_find_file(void);
void bm_open(void);
void bm_close(void);
BackupCheck bm_open_formatted(void);
BackupFree bm_free(void);
BackupFormat bm_format(void);

/* How ad_trans ended. */
typedef struct {
    bool refused;
    uint8_t busy_port;
    CdExchange exchange;
    uint8_t status;
    bool failed;
    CdRetry retry;
} AdTrans;

AdTrans ad_trans(void);
void ad_set_address(uint8_t low, uint8_t high);
uint8_t ad_latch_read_address(void);
uint8_t ad_latch_write_address(void);
uint8_t ad_latch_length(void);
uint8_t ad_read_byte(void);
CdBaseSum add_cd_base(void);
bool cd_base_sum_overflowed(CdBaseSum added);
uint8_t write_vram_word_pending(void);
CdRead cd_read(void);
CdBlockRead cd_read_to_memory(void);
CdVramRead cd_read_to_vram_fast(void);
CdVramRead cd_read_to_vram_slow(void);
uint8_t cd_bytes_to_sectors(void);
DiscEndStep disc_end_minus_one_frame(void);
CdPlay cd_play(void);
CdAttempt cd_subq(void);
CdAttempt cd_dinfo(void);
CdContents cd_contnts(void);
CdPause cd_pause(void);
CdSearch cd_search(void);
void clear_cd_command(void);
void clear_cd_result(void);
void save_card_args(void);
void restore_card_args(void);
uint8_t copy_args_to_command(uint8_t from, uint8_t to);
bool cd_drop_extra_data(void);
CdTest cd_test_unit(void);
CdDataIn cd_read_data_in(void);
uint8_t cd_request_sense(CdExchange *exchange);
CdRetry cd_retry_after_error(uint8_t error);
CdStat cd_stat(uint8_t what);
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
