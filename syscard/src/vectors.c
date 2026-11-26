/* The jump table at $E000: each entry the game calls is a JMP to a BIOS routine. */
#include "card.h"
#include "syscard.h"

/* The wrappers the vectors go through: each is defined with its routine. */
void abi_cd_base(void);
void abi_cd_read(void);
void abi_cd_play(void);
void abi_cd_search(void);
void abi_cd_pause(void);
void abi_cd_stat(void);
void abi_cd_fade(void);
void abi_ad_trans(void);
void abi_ad_read(void);
void abi_ad_play(void);
void abi_ad_stop(void);
void abi_ad_stat(void);
void abi_bm_free(void);
void abi_bm_read(void);
void abi_bm_write(void);
void abi_bm_delete(void);
void abi_ex_getver(void);
void abi_ex_setvec(void);
void abi_ex_getfnt(void);
void abi_ex_scrsiz(void);
void abi_ex_dotmod(void);
void abi_ex_scrmod(void);
void abi_ex_satclr(void);
void abi_ma_mul8u(void);
void abi_ex_memopen(void);


/*
 * @card 00:E006 vec_cd_base
 * WHAT IT DOES: The jump table's entry for cd_base: goes to it.
 * HOW: A JMP to cd_base, no frame of its own: the caller's JSR frame is the one cd_base returns through.
 * WAITS: no.
 * Entry, exit: as cd_base.
 */
CardExit vec_cd_base(uint8_t a, uint8_t x)
{
    return cd_base(a, x);
}

void abi_vec_cd_base(void)
{
    abi_cd_base();
}

/*
 * @card 00:E009 vec_cd_read
 * WHAT IT DOES: The jump table's entry for cd_read: goes to it.
 * HOW: A JMP to cd_read, no frame of its own: the caller's JSR frame is the one cd_read returns through.
 * WAITS: no.
 * Entry, exit: as cd_read.
 */
CdRead vec_cd_read(uint8_t a, uint8_t x)
{
    return cd_read(a, x);
}

void abi_vec_cd_read(void)
{
    abi_cd_read();
}

/*
 * @card 00:E012 vec_cd_play
 * WHAT IT DOES: The jump table's entry for cd_play: goes to it.
 * HOW: A JMP to cd_play, no frame of its own: the caller's JSR frame is the one cd_play returns through.
 * WAITS: no.
 * Entry, exit: as cd_play.
 */
CdPlay vec_cd_play(uint8_t a, uint8_t x)
{
    return cd_play(a, x);
}

void abi_vec_cd_play(void)
{
    abi_cd_play();
}

/*
 * @card 00:E015 vec_cd_search
 * WHAT IT DOES: The jump table's entry for cd_search: goes to it.
 * HOW: A JMP to cd_search, no frame of its own: the caller's JSR frame is the one cd_search returns through.
 * WAITS: no.
 * Entry, exit: as cd_search.
 */
CdSearch vec_cd_search(uint8_t a, uint8_t x)
{
    return cd_search(a, x);
}

void abi_vec_cd_search(void)
{
    abi_cd_search();
}

/*
 * @card 00:E018 vec_cd_pause
 * WHAT IT DOES: The jump table's entry for cd_pause: goes to it.
 * HOW: A JMP to cd_pause, no frame of its own: the caller's JSR frame is the one cd_pause returns through.
 * WAITS: no.
 * Entry, exit: as cd_pause.
 */
CdPause vec_cd_pause(uint8_t x)
{
    return cd_pause(x);
}

void abi_vec_cd_pause(void)
{
    abi_cd_pause();
}

/*
 * @card 00:E01B vec_cd_stat
 * WHAT IT DOES: The jump table's entry for cd_stat: goes to it.
 * HOW: A JMP to cd_stat, no frame of its own: the caller's JSR frame is the one cd_stat returns through.
 * WAITS: no.
 * Entry, exit: as cd_stat.
 */
CdStat vec_cd_stat(uint8_t what, uint8_t x)
{
    return cd_stat(what, x);
}

void abi_vec_cd_stat(void)
{
    abi_cd_stat();
}

/*
 * @card 00:E02D vec_cd_fade
 * WHAT IT DOES: The jump table's entry for cd_fade: goes to it.
 * HOW: A JMP to cd_fade, no frame of its own: the caller's JSR frame is the one cd_fade returns through.
 * WAITS: no.
 * Entry, exit: as cd_fade.
 */
uint8_t vec_cd_fade(uint8_t type)
{
    return cd_fade(type);
}

void abi_vec_cd_fade(void)
{
    abi_cd_fade();
}

/*
 * @card 00:E033 vec_ad_trans
 * WHAT IT DOES: The jump table's entry for ad_trans: goes to it.
 * HOW: A JMP to ad_trans, no frame of its own: the caller's JSR frame is the one ad_trans returns through.
 * WAITS: no.
 * Entry, exit: as ad_trans.
 */
AdTrans vec_ad_trans(uint8_t a, uint8_t x)
{
    return ad_trans(a, x);
}

void abi_vec_ad_trans(void)
{
    abi_ad_trans();
}

/*
 * @card 00:E036 vec_ad_read
 * WHAT IT DOES: The jump table's entry for ad_read: goes to it.
 * HOW: A JMP to ad_read, no frame of its own: the caller's JSR frame is the one ad_read returns through.
 * WAITS: no.
 * Entry, exit: as ad_read.
 */
CardExit vec_ad_read(void)
{
    return ad_read();
}

void abi_vec_ad_read(void)
{
    abi_ad_read();
}

/*
 * @card 00:E03C vec_ad_play
 * WHAT IT DOES: The jump table's entry for ad_play: goes to it.
 * HOW: A JMP to ad_play, no frame of its own: the caller's JSR frame is the one ad_play returns through.
 * WAITS: no.
 * Entry, exit: as ad_play.
 */
AdPlay vec_ad_play(void)
{
    return ad_play();
}

void abi_vec_ad_play(void)
{
    abi_ad_play();
}

/*
 * @card 00:E042 vec_ad_stop
 * WHAT IT DOES: The jump table's entry for ad_stop: goes to it.
 * HOW: A JMP to ad_stop, no frame of its own: the caller's JSR frame is the one ad_stop returns through.
 * WAITS: no.
 * Entry, exit: as ad_stop.
 */
uint8_t vec_ad_stop(void)
{
    return ad_stop();
}

void abi_vec_ad_stop(void)
{
    abi_ad_stop();
}

/*
 * @card 00:E045 vec_ad_stat
 * WHAT IT DOES: The jump table's entry for ad_stat: goes to it.
 * HOW: A JMP to ad_stat, no frame of its own: the caller's JSR frame is the one ad_stat returns through.
 * WAITS: no.
 * Entry, exit: as ad_stat.
 */
AdStat vec_ad_stat(void)
{
    return ad_stat();
}

void abi_vec_ad_stat(void)
{
    abi_ad_stat();
}

/*
 * @card 00:E04B vec_bm_free
 * WHAT IT DOES: The jump table's entry for bm_free: goes to it.
 * HOW: A JMP to bm_free, no frame of its own: the caller's JSR frame is the one bm_free returns through.
 * WAITS: no.
 * Entry, exit: as bm_free.
 */
BackupAccess vec_bm_free(uint8_t a)
{
    return bm_free(a);
}

void abi_vec_bm_free(void)
{
    abi_bm_free();
}

/*
 * @card 00:E04E vec_bm_read
 * WHAT IT DOES: The jump table's entry for bm_read: goes to it.
 * HOW: A JMP to bm_read, no frame of its own: the caller's JSR frame is the one bm_read returns through.
 * WAITS: no.
 * Entry, exit: as bm_read.
 */
BackupAccess vec_bm_read(uint8_t a)
{
    return bm_read(a);
}

void abi_vec_bm_read(void)
{
    abi_bm_read();
}

/*
 * @card 00:E051 vec_bm_write
 * WHAT IT DOES: The jump table's entry for bm_write: goes to it.
 * HOW: A JMP to bm_write, no frame of its own: the caller's JSR frame is the one bm_write returns through.
 * WAITS: no.
 * Entry, exit: as bm_write.
 */
BackupAccess vec_bm_write(uint8_t a)
{
    return bm_write(a);
}

void abi_vec_bm_write(void)
{
    abi_bm_write();
}

/*
 * @card 00:E054 vec_bm_delete
 * WHAT IT DOES: The jump table's entry for bm_delete: goes to it.
 * HOW: A JMP to bm_delete, no frame of its own: the caller's JSR frame is the one bm_delete returns through.
 * WAITS: no.
 * Entry, exit: as bm_delete.
 */
BackupAccess vec_bm_delete(uint8_t a)
{
    return bm_delete(a);
}

void abi_vec_bm_delete(void)
{
    abi_bm_delete();
}

/*
 * @card 00:E05A vec_ex_getver
 * WHAT IT DOES: The jump table's entry for ex_getver: goes to it.
 * HOW: A JMP to ex_getver, no frame of its own: the caller's JSR frame is the one ex_getver returns through.
 * WAITS: no.
 * Entry, exit: as ex_getver.
 */
CardVersion vec_ex_getver(void)
{
    return ex_getver();
}

void abi_vec_ex_getver(void)
{
    abi_ex_getver();
}

/*
 * @card 00:E05D vec_ex_setvec
 * WHAT IT DOES: The jump table's entry for ex_setvec: goes to it.
 * HOW: A JMP to ex_setvec, no frame of its own: the caller's JSR frame is the one ex_setvec returns through.
 * WAITS: no.
 * Entry, exit: as ex_setvec.
 */
SetVec vec_ex_setvec(uint8_t which, uint8_t low, uint8_t high, uint8_t flags)
{
    return ex_setvec(which, low, high, flags);
}

void abi_vec_ex_setvec(void)
{
    abi_ex_setvec();
}

/*
 * @card 00:E060 vec_ex_getfnt
 * WHAT IT DOES: The jump table's entry for ex_getfnt: goes to it.
 * HOW: A JMP to ex_getfnt, no frame of its own: the caller's JSR frame is the one ex_getfnt returns through.
 * WAITS: no.
 * Entry, exit: as ex_getfnt.
 */
CardExit vec_ex_getfnt(void)
{
    return ex_getfnt();
}

void abi_vec_ex_getfnt(void)
{
    abi_ex_getfnt();
}

/*
 * @card 00:E069 vec_ex_scrsiz
 * WHAT IT DOES: The jump table's entry for ex_scrsiz: goes to it.
 * HOW: A JMP to ex_scrsiz, no frame of its own: the caller's JSR frame is the one ex_scrsiz returns through.
 * WAITS: no.
 * Entry, exit: as ex_scrsiz.
 */
CardExit vec_ex_scrsiz(uint8_t size)
{
    return ex_scrsiz(size);
}

void abi_vec_ex_scrsiz(void)
{
    abi_ex_scrsiz();
}

/*
 * @card 00:E06C vec_ex_dotmod
 * WHAT IT DOES: The jump table's entry for ex_dotmod: goes to it.
 * HOW: A JMP to ex_dotmod, no frame of its own: the caller's JSR frame is the one ex_dotmod returns through.
 * WAITS: no.
 * Entry, exit: as ex_dotmod.
 */
CardExit vec_ex_dotmod(uint8_t mode)
{
    return ex_dotmod(mode);
}

void abi_vec_ex_dotmod(void)
{
    abi_ex_dotmod();
}

/*
 * @card 00:E06F vec_ex_scrmod
 * WHAT IT DOES: The jump table's entry for ex_scrmod: goes to it.
 * HOW: A JMP to ex_scrmod, no frame of its own: the caller's JSR frame is the one ex_scrmod returns through.
 * WAITS: no.
 * Entry, exit: as ex_scrmod.
 */
CardExit vec_ex_scrmod(uint8_t mode, uint8_t columns, uint8_t rows)
{
    return ex_scrmod(mode, columns, rows);
}

void abi_vec_ex_scrmod(void)
{
    abi_ex_scrmod();
}

/*
 * @card 00:E0A2 vec_ex_satclr
 * WHAT IT DOES: The jump table's entry for ex_satclr: goes to it.
 * HOW: A JMP to ex_satclr, no frame of its own: the caller's JSR frame is the one ex_satclr returns through.
 * WAITS: no.
 * Entry, exit: as ex_satclr.
 */
CardExit vec_ex_satclr(void)
{
    return ex_satclr();
}

void abi_vec_ex_satclr(void)
{
    abi_ex_satclr();
}

/*
 * @card 00:E0BD vec_ma_mul8u
 * WHAT IT DOES: The jump table's entry for ma_mul8u: goes to it.
 * HOW: A JMP to ma_mul8u, no frame of its own: the caller's JSR frame is the one ma_mul8u returns through.
 * WAITS: no.
 * Entry, exit: as ma_mul8u.
 */
MaMul8u vec_ma_mul8u(uint8_t a, uint8_t flags)
{
    return ma_mul8u(a, flags);
}

void abi_vec_ma_mul8u(void)
{
    abi_ma_mul8u();
}

/*
 * @card 00:E0DE vec_ex_memopen
 * WHAT IT DOES: The jump table's entry for ex_memopen: goes to it.
 * HOW: A JMP to ex_memopen, no frame of its own: the caller's JSR frame is the one ex_memopen returns through.
 * WAITS: no.
 * Entry, exit: as ex_memopen.
 */
MemOpen vec_ex_memopen(void)
{
    return ex_memopen();
}

void abi_vec_ex_memopen(void)
{
    abi_ex_memopen();
}
