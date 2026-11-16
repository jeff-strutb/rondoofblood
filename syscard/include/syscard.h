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

/* A file's name and data summed as bm_sum_file sums them, with its last step. */
typedef struct {
    uint16_t sum;
    uint16_t before_last;  /* the sum before the last byte was added */
    uint8_t last_byte;
    uint32_t bytes;        /* bytes summed: $10000 when the count started at zero */
} BackupSum;

/* Where a read or write of _CX bytes at offset _DX falls in the file at CardFileLo. */
typedef struct {
    uint16_t overrun;      /* bytes past the file's end, negative when it fits */
    uint16_t data;         /* the file's address plus its header */
    uint16_t offset;       /* _DX */
} BackupSpan;

/* Which way a move of bytes inside the backup memory went. */
typedef struct {
    bool forward;          /* from a source at or above its destination */
    uint16_t source;
    uint16_t destination;
    uint16_t count;
} BackupMoveWay;

/* A copy between the caller's memory and the backup memory: the window and the bytes. */
typedef struct {
    uint8_t window;
    uint8_t old_high;      /* the caller's pointer's high byte before it was rebased */
    uint32_t bytes;
} BackupCopy;

/* What adding a file's header for a new name found. */
typedef struct {
    bool made;
    uint16_t pointer;      /* CardCopyLo-Hi, compared with $10 */
    uint16_t next_free;    /* where the file went */
    BackupCopy copy;
} BackupNewFile;

typedef struct {
    bool moved;            /* there were bytes to move */
    BackupMoveWay way;
} BackupMove;

/* How bm_read, bm_write and bm_delete ended, and the values their exits show. */
typedef enum {
    BM_ENDED_NOT_FORMATTED,
    BM_ENDED_NOT_FOUND,     /* no such file, or no room for its header */
    BM_ENDED_NO_ROOM,       /* a write that would run past the memory's end */
    BM_ENDED_PAST_END,      /* a read starting at or past the file's end */
    BM_ENDED_DONE
} BackupEnding;

typedef struct {
    BackupEnding ending;
    uint8_t result;         /* what A gives: 0 done, 1 not found or no room, 2 bad checksum */
    BackupCheck check;
    BackupFind find;
    BackupNewFile new_file;
    BackupSpan span;
    bool truncated;         /* a read cut short at the file's end */
    uint16_t count_before;  /* the count it was cut from */
    uint16_t count;         /* bytes copied */
    BackupCopy copy;
    BackupSum sum;
    uint16_t end;           /* write: the memory's end, compared with the room needed */
    uint16_t needed;
    BackupMove move;
    uint16_t next_free;     /* delete: before it was taken down */
    uint16_t length;        /* delete: the file's length */
} BackupAccess;

/* How ex_scrmod set the screen, and the values its exit shows. */
typedef struct {
    bool accepted;            /* false: the fallback mode was set instead */
    uint8_t columns;
    uint8_t width_register;   /* the memory width register ex_dotmod left */
    uint8_t dot_bits;         /* what ex_dotmod was given */
    uint8_t end_inverse;      /* the horizontal end's inverse, before its base was added */
    uint8_t end_base;
} ScreenMode;

/* What setting the vertical display left. */
typedef struct {
    uint8_t rows_dots;        /* the rows times eight, as a byte */
    uint8_t vce;              /* what went to the VCE */
} ScreenRows;

ScreenRows set_screen_rows(void);
ScreenMode ex_scrmod(uint8_t mode, uint8_t columns, uint8_t rows);
ScreenMode ex_scrmod_fallback(void);
void ex_satclr(void);

/* The card's version, as ex_getver gives it. */
typedef struct {
    uint8_t major;
    uint8_t minor;
} CardVersion;

/* How ad_play ended. */
typedef enum {
    AD_PLAY_BUSY,       /* ADPCM was already playing: nothing done */
    AD_PLAY_BAD_RATE,   /* a rate of $10 or more: nothing played */
    AD_PLAY_AGAIN,      /* played from what is already latched */
    AD_PLAY_STARTED     /* latched _BX and _AX, then played */
} AdPlayHow;

typedef struct {
    AdPlayHow how;
    AdStat stat;
    uint8_t rate;
    uint8_t irq_before;  /* the CD unit's interrupt enables before playing turned one on */
    uint8_t length_low;  /* what was latched as the length, which X and Y give back */
    uint8_t length_high;
} AdPlay;

CardVersion ex_getver(void);
uint8_t map_boot_ram(void);
void init_card_vectors(void);
CdSubcodeRead reset_cd_unit(void);
void clear_vram(void);
uint8_t load_font(void);
uint8_t ad_reset(void);
AdPlay ad_play(void);
/* What ad_read did, and what its exit shows. */
typedef struct {
    AdStat stat;
    uint8_t latch;        /* the control port as ad_latch_read_address read it */
    uint8_t mode;         /* _DH */
    bool refused;
    uint8_t mask;         /* the MPR bit, reading to banks */
    uint8_t saved_bank;
    uint8_t pages_left;
    uint8_t bytes_left;
} AdRead;

AdRead ad_read(void);
void ad_read_to_banks(AdRead *read);
void aim_bx_at_vram_port(void);
uint8_t ad_read_byte(void);
uint8_t write_vram_word_pending(void);

/*
 * The exits of ad_play, ad_stat and ad_stop: the registers and flags each leaves, for its abi_
 * wrapper and for a caller's wrapper whose routine ends in it. They touch the CPU, so only abi_
 * wrappers call them.
 */
void ad_play_exit_shim(AdPlay play);
void ad_stat_exit_shim(AdStat stat);
void ad_stop_exit_shim(uint8_t control);
AdStat ad_stat(void);
uint8_t ad_latch_read_address(void);
uint8_t ad_latch_length(void);

/* A byte converted between binary and BCD, and whether it could be. */
typedef struct {
    bool converted;
    uint8_t value;      /* the converted byte, or the one given */
    uint8_t tens;       /* ex_bcdbin: the high digit, or ten times it once it was checked */
} BcdConvert;

BcdConvert ex_bcdbin(uint8_t bcd);
BcdConvert ex_binbcd(uint8_t value);

/* The last add of a sector number's terms, which the exit's flags come from. */
typedef struct {
    uint32_t before;    /* the sum before it, 24 bits */
    uint32_t term;      /* what was added, 24 bits */
} LbaAdd;

void lba_load_term(uint8_t value);
bool lba_double_term(void);
LbaAdd lba_add_term(void);
LbaAdd lba_double_and_add(void);
LbaAdd msf_to_lba(void);

/* How cd_base read the base it was given, and what its exit shows. */
typedef struct {
    bool converted;      /* a position or a track was made into a sector number */
    bool from_track;     /* the position came from the table of contents */
    CdAttempt dinfo;
    LbaAdd add;
    uint8_t saved_mpr6;  /* what MPR6 held while bank 1 was mapped */
    uint8_t which;       /* _CL: bit 1 keeps the first base, bit 0 the second */
} CdBase;

CdBase cd_base(void);

/* What ex_vsync did: returned at once, or waited for the frame count to move. */
typedef struct {
    bool waited;
    uint8_t before;
    uint8_t after;
} VsyncWait;

uint8_t ex_imode(uint8_t mode);
uint8_t ex_vmode(void);
uint8_t ex_hmode(void);
VsyncWait ex_vsync(void);
uint8_t ex_rcron(void);
uint8_t ex_rcroff(void);
uint8_t ex_irqon(void);
uint8_t ex_irqoff(void);
void ex_bgon(void);
void ex_bgoff(void);
void ex_spron(void);
void ex_sproff(void);
void ex_dspon(void);
void ex_dspoff(void);
uint8_t ex_dmamod(uint8_t mode);
uint8_t ex_sprdma(void);
void ex_setrcr(uint8_t low, uint8_t high);
void ex_setred(uint8_t low, uint8_t high);
void ex_setwrt(uint8_t low, uint8_t high);
uint8_t ex_setdma(void);

/* Where sjis_to_font_index gave up, if it did. */
typedef enum {
    FONT_FOUND,
    FONT_LEAD_HIGH,     /* the first byte past $EF */
    FONT_LEAD_GAP,      /* the first byte in $A0-$BF */
    FONT_TRAIL_HIGH,    /* the second byte past $FC */
    FONT_TRAIL_GAP,     /* the second byte $7F */
    FONT_ROW_UNUSED,    /* a JIS row with no glyphs: $28-$2F */
    FONT_ROW_HIGH       /* a JIS row past $4F */
} FontFailure;

/* A Shift-JIS code made into a glyph number, with what the exits' flags come from. */
typedef struct {
    FontFailure failure;
    uint8_t lead;        /* _AH as given */
    uint8_t trail;       /* _AL as given */
    uint8_t row;         /* the JIS row */
    uint8_t column;      /* the column after its first step, before the row's adjustments */
    bool second_half;    /* the code's second byte was in the lead's second row */
    uint8_t table_index;
    uint8_t carry_in;    /* the carry into the index's high byte */
} FontIndex;

/* What ex_getfnt did. */
typedef struct {
    FontIndex index;
    bool twelve_dot;
    uint8_t restored_high;  /* the second MPR it put back */
} FontGet;

FontIndex sjis_to_font_index(void);
void put_font_byte(uint8_t byte);
void expand_12dot_glyph(void);
FontGet ex_getfnt(void);

/* What ex_colorcmd moved: the command, and each set's count of 16-colour palettes. */
typedef struct {
    uint8_t command;
    uint8_t background_sets;
    uint8_t sprite_sets;
} ColorCommand;

ColorCommand ex_colorcmd(void);

BackupSum bm_sum_file(void);
BackupSpan bm_place_span(void);
BackupMoveWay bm_move_direction(void);
BackupMove bm_move(void);
BackupCopy bm_copy_out(void);
BackupCopy bm_copy_in(void);
BackupNewFile bm_new_file(void);
BackupAccess bm_read(void);
BackupAccess bm_write(void);
BackupAccess bm_delete(void);

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
