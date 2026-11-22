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
























/*
 * What a routine leaves in the registers and flags, for its exit shim: each register or flag it sets has its
 * set_ flag, and one that is not set is left as the caller had it. nz is the value N and Z come from.
 */
typedef struct {
    uint8_t a, x, y, nz;
    bool c, v;
    bool n, z;      /* N and Z on their own, for what a TSB, TRB or TST leaves: from the byte, not from a result */
    bool set_a, set_x, set_y, set_nz, set_c, set_v, set_n, set_z;
} CardExit;

/* The CD calls the game makes keep the registers and flags they leave, as a CardExit; A is the call's answer. */
typedef CardExit CdPlay;
typedef CardExit CdRead;
typedef CardExit CdSearch;
typedef CardExit CdStat;
typedef CardExit CdPause;
typedef CardExit CdSeek;
typedef CardExit AdTrans;


/* What checking the backup memory's header found, and what it leaves in the registers. */
typedef struct {
    bool formatted;
    CardExit exit;
} BackupCheck;

/* How a backup call ended, and the values its exit shows. */
typedef enum {
    BM_ENDED_NOT_FORMATTED,
    BM_ENDED_NOT_FOUND,     /* no such file, no room for its header, or a wrong password */
    BM_ENDED_NO_ROOM,       /* a write that would run past the memory's end */
    BM_ENDED_PAST_END,      /* a read starting at or past the file's end */
    BM_ENDED_DONE
} BackupEnding;

typedef struct {
    BackupEnding ending;
    uint8_t result;         /* what A gives: 0 done, 1 not found, no room or a wrong password, 2 bad checksum, $FF not formatted */
    CardExit exit;          /* X, Y, C and V as the call leaves them (A and N, Z come from result) */
} BackupAccess;

typedef BackupAccess BackupFree;
typedef BackupAccess BackupFormat;
typedef BackupAccess BackupFiles;

uint8_t bm_open(uint8_t a);
uint8_t bm_close(uint8_t a);
BackupCheck bm_open_formatted(uint8_t a);
uint8_t bm_count_down(void);
CardExit bm_find_file(void);
CardExit bm_sum_file(void);
CardExit bm_place_span(void);
CardExit bm_move_direction(void);
CardExit bm_move(void);
CardExit bm_copy_out(void);
CardExit bm_copy_in(void);
CardExit bm_new_file(void);
BackupAccess bm_read(uint8_t a);
BackupAccess bm_write(uint8_t a);
BackupAccess bm_delete(uint8_t a);
BackupAccess bm_free(uint8_t a);
BackupAccess bm_format(uint8_t a);
BackupAccess bm_files(uint8_t a);
uint8_t bm_files_result(BackupFiles files);
void bm_files_exit_shim(BackupFiles files);

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

/* What ex_sprput did. */
typedef struct {
    bool put;           /* false: the table already held 64 sprites */
    uint8_t count;      /* CardSatFlag as it was read */
    uint16_t sat;       /* CardSatAddress */
    uint8_t last;       /* the last byte written to the VDC */
} SpritePut;

SpritePut ex_sprput(void);

/* A byte from ex_rnd, and the bit its last shift carried out. */
typedef struct {
    uint8_t value;
    bool carry;
} RandomByte;

RandomByte ex_rnd(void);

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
void cd_subrd_exit_shim(CdSubcodeRead read);

void clear_vram(void);
uint8_t load_font(void);
uint8_t boot_display(void);

/* What a run of print_text's list did: where its end mark was, and the last text placed. */
typedef struct {
    uint8_t end;
    bool placed;
    uint8_t column;
    uint8_t row;
} TextRun;

uint16_t text_cursor(uint8_t column, uint8_t row);
TextRun print_text(void);
TextRun print_text_run(uint8_t index);

/* What the start screen ended with: run (with the drive's test) or select, and any wait. */
enum { START_RUN = 0, START_SELECT = 1 };

typedef struct {
    uint8_t choice;
    CdStat drive;
    bool waited;
} StartScreen;

StartScreen start_screen(void);
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

/* What ad_write did, and what its exit shows. */
typedef struct {
    uint8_t busy;         /* the transfer bits of the port: not zero refuses */
    bool addressed;       /* _CX was not $FFFF: it was latched */
    uint8_t address_low;  /* _CL */
    uint8_t address_high; /* _CH */
    uint8_t latch;        /* the control port as ad_latch_write_address read it */
    uint8_t mode;         /* _DH */
    bool refused;
    uint8_t mask;         /* the MPR bit, writing from banks */
    uint8_t saved_bank;
    uint8_t pages_left;
    uint8_t bytes_left;
    uint8_t last_status;  /* the status port as the last wait read it */
} AdWrite;

AdWrite ad_write(void);
void ad_write_exit_shim(AdWrite write);


CardExit ad_trans_begin(void);
void aim_bx_at_vram_port(void);
uint8_t ad_read_byte(void);
uint8_t write_vram_word_pending(void);

/* ma_mul8u's result: the product, and the A and flags it pulled back (the product's low byte, the flags as pushed). */
typedef struct {
    uint16_t product;
    uint8_t a;
    uint8_t flags;
} MaMul8u;

/*
 * The exits of ad_play, ad_stat and ad_stop: the registers and flags each leaves, for its abi_
 * wrapper and for a caller's wrapper whose routine ends in it. They touch the CPU, so only abi_
 * wrappers call them.
 */
void ad_play_exit_shim(AdPlay play);
void ad_stat_exit_shim(AdStat stat);
void ad_stop_exit_shim(uint8_t control);
void cd_play_exit_shim(CdPlay play);
void cd_read_exit_shim(CdRead read);
void cd_fade_exit_shim(uint8_t nibble);
void ma_mul8u_exit_shim(MaMul8u result);
void bm_read_exit_shim(BackupAccess read);
void bm_write_exit_shim(BackupAccess write);

/* The A each leaves, which the game's callers branch on: the BIOS's status, 0 for success. */
uint8_t cd_play_result(CdPlay play);
uint8_t cd_read_result(CdRead read);
uint8_t bm_read_result(BackupAccess read);
uint8_t bm_write_result(BackupAccess write);
void bm_delete_exit_shim(BackupAccess removal);
uint8_t bm_delete_result(BackupAccess removal);
void bm_free_exit_shim(BackupFree free_space);
uint8_t bm_free_result(BackupFree free_space);
void bm_format_exit_shim(BackupFormat format);
uint8_t bm_format_result(BackupFormat format);
void cd_pause_exit_shim(CdPause pause);
void cd_seek_exit_shim(CdSeek seek);
uint8_t cd_seek_result(CdSeek seek);
uint8_t cd_pause_result(CdPause pause);
void cd_search_exit_shim(CdSearch search);
uint8_t cd_search_result(CdSearch search);
void cd_stat_exit_shim(CdStat stat);
uint8_t cd_stat_result(CdStat stat);
AdStat ad_stat(void);
uint8_t ad_latch_read_address(void);
uint8_t ad_latch_length(void);

/* A byte converted between binary and BCD, and whether it could be. */
typedef struct {
    bool converted;
    uint8_t value;      /* the converted byte, or the one given */
    uint8_t tens;       /* ex_bcdbin: the high digit, or ten times it once it was checked */
} BcdConvert;

BcdConvert ex_bcdbin(uint8_t bcd, uint8_t x);
BcdConvert ex_binbcd(uint8_t value, uint8_t x);


void lba_load_term(uint8_t value);
CardExit lba_double_term(void);
CardExit lba_add_term(void);
CardExit lba_double_and_add(void);
CardExit msf_to_lba(uint8_t x);


CardExit cd_base(uint8_t a, uint8_t x);

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
void ex_getfnt_exit_shim(FontGet get);

/* What ex_colorcmd moved: the command, and each set's count of 16-colour palettes. */
typedef struct {
    uint8_t command;
    uint8_t background_sets;
    uint8_t sprite_sets;
} ColorCommand;

ColorCommand ex_colorcmd(void);


AdTrans ad_trans(uint8_t a, uint8_t x);
void ad_trans_exit_shim(AdTrans trans);
uint8_t ad_trans_result(AdTrans trans);
/* What one step of continuous ADPCM play did to its counts. */
typedef struct {
    uint32_t left_before;  /* the sectors left, before the step */
    uint8_t step;
} CplayStep;

/* How ad_cplay ended. */
typedef enum { AD_CPLAY_BUSY, AD_CPLAY_BAD_RATE, AD_CPLAY_TRANSFER_FAILED, AD_CPLAY_STARTED } AdCplayHow;

typedef CardExit AdCplay;

CardExit cplay_advance(void);
AdCplay ad_cplay(uint8_t a, uint8_t x);
uint8_t ad_cplay_result(AdCplay play);
void ad_cplay_exit_shim(AdCplay play);
void ad_set_address(uint8_t low, uint8_t high);
uint8_t ad_latch_read_address(void);
uint8_t ad_latch_write_address(void);
uint8_t ad_latch_length(void);
uint8_t ad_read_byte(void);
CardExit add_cd_base(void);
uint8_t write_vram_word_pending(void);
CdRead cd_read(uint8_t a, uint8_t x);
CdSeek cd_seek(uint8_t a, uint8_t x);
uint8_t cd_seek_result(CdSeek seek);
void cd_seek_exit_shim(CdSeek seek);
CardExit cd_read_to_memory(void);
CardExit cd_read_to_vram_fast(void);
CardExit cd_read_to_vram_slow(void);
CardExit cd_bytes_to_sectors(void);
CardExit disc_end_minus_one_frame(uint8_t a);
CdPlay cd_play(uint8_t a, uint8_t x);
CardExit cd_subq(uint8_t a, uint8_t x);
CardExit cd_dinfo(uint8_t a, uint8_t x);
CardExit cd_contnts(uint8_t a, uint8_t x);
CdPause cd_pause(uint8_t x);
CdSearch cd_search(uint8_t a, uint8_t x);
void clear_cd_command(uint8_t x);
void clear_cd_result(uint8_t x);
void save_card_args(uint8_t a, uint8_t x);
void restore_card_args(uint8_t a, uint8_t x);
uint8_t copy_args_to_command(uint8_t from, uint8_t to);
CardExit cd_drop_extra_data(void);
CardExit cd_test_unit(uint8_t x);
CardExit cd_read_data_in(void);
CardExit cd_request_sense(uint8_t x);
CardExit cd_retry_after_error(uint8_t error, uint8_t x);
CdStat cd_stat(uint8_t what, uint8_t x);
CardExit cd_send_command(void);
CardExit cd_read_status(void);
CardExit cd_finish_command(void);
uint8_t cd_ack_pulse(void);
uint8_t cd_ack_on(void);
uint8_t cd_ack_off(void);
CardExit cd_reset(void);
CdSubcodeRead cd_subrd(void);
CdPcmRead cd_pcmrd(uint8_t latch);
MaMul8u ma_mul8u(uint8_t a, uint8_t flags);
uint8_t cd_fade(uint8_t type);
uint8_t ad_stop(void);
AdStat ad_stat(void);
uint8_t ex_scrsiz(uint8_t size);
void ex_scrsiz_exit_shim(uint8_t given, uint8_t width);
uint8_t ex_dotmod(uint8_t mode);
MemOpen ex_memopen(void);
void ex_setvec(uint8_t which, uint8_t low, uint8_t high);

uint8_t cd_ack_on(void);
uint8_t cd_ack_off(void);

/* ad_trans_step's states, as CardAdTransStep holds them. */
enum { AD_TRANS_IDLE, AD_TRANS_SELECT, AD_TRANS_COMMAND, AD_TRANS_DATA, AD_TRANS_STATUS };

/* How ad_trans_select's try at the drive went. */
typedef enum { AD_SELECT_BUSY, AD_SELECT_TIMED_OUT, AD_SELECTED } AdSelectHow;

typedef struct {
    AdSelectHow how;
    uint8_t bus;         /* the bus as the busy test read it */
    uint8_t tries_left;  /* X: the count of tries, down from 0, when the drive answered */
} AdTransSelect;

/* What ad_trans_command did with the phase it found. */
typedef enum { AD_COMMAND_WAITING, AD_COMMAND_SENT, AD_COMMAND_DONE } AdCommandHow;

typedef struct {
    AdCommandHow how;
    uint8_t index;    /* the command byte sent */
    uint8_t control;  /* the CD control port as the acknowledge found it */
    uint8_t answer;   /* the phase and $B8, when not the command phase */
} AdTransCommand;

typedef struct {
    bool done;
    uint8_t phase;     /* the phase first read */
    uint8_t status;    /* the drive's status byte */
    uint8_t last_bus;  /* the bus's last read, once free */
} AdTransFinish;

/* One step of ad_trans_step: the state it found, and what that state's step did. */
typedef struct {
    uint8_t state;
    AdTransSelect select;
    AdTransCommand command;
    uint8_t control;   /* the ADPCM control port, starting the transfer */
    AdTransFinish finish;
} AdTransStep;

AdTransSelect ad_trans_select(void);
AdTransCommand ad_trans_command(void);
uint8_t ad_trans_start_dma(void);
AdTransFinish ad_trans_finish(void);
AdTransStep ad_trans_step(void);

#endif
