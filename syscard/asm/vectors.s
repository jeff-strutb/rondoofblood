; @card 00:E006 vec_cd_base
; WHAT IT DOES: The jump table's entry for cd_base: goes to it.
; HOW: A JMP to cd_base, no frame of its own: the caller's JSR frame is the one cd_base returns through.
; WAITS: no.
; Entry, exit: as cd_base.
vec_cd_base:
    jmp cd_base

; @card 00:E009 vec_cd_read
; WHAT IT DOES: The jump table's entry for cd_read: goes to it.
; HOW: A JMP to cd_read, no frame of its own: the caller's JSR frame is the one cd_read returns through.
; WAITS: no.
; Entry, exit: as cd_read.
vec_cd_read:
    jmp cd_read

; @card 00:E012 vec_cd_play
; WHAT IT DOES: The jump table's entry for cd_play: goes to it.
; HOW: A JMP to cd_play, no frame of its own: the caller's JSR frame is the one cd_play returns through.
; WAITS: no.
; Entry, exit: as cd_play.
vec_cd_play:
    jmp cd_play

; @card 00:E015 vec_cd_search
; WHAT IT DOES: The jump table's entry for cd_search: goes to it.
; HOW: A JMP to cd_search, no frame of its own: the caller's JSR frame is the one cd_search returns through.
; WAITS: no.
; Entry, exit: as cd_search.
vec_cd_search:
    jmp cd_search

; @card 00:E018 vec_cd_pause
; WHAT IT DOES: The jump table's entry for cd_pause: goes to it.
; HOW: A JMP to cd_pause, no frame of its own: the caller's JSR frame is the one cd_pause returns through.
; WAITS: no.
; Entry, exit: as cd_pause.
vec_cd_pause:
    jmp cd_pause

; @card 00:E01B vec_cd_stat
; WHAT IT DOES: The jump table's entry for cd_stat: goes to it.
; HOW: A JMP to cd_stat, no frame of its own: the caller's JSR frame is the one cd_stat returns through.
; WAITS: no.
; Entry, exit: as cd_stat.
vec_cd_stat:
    jmp cd_stat

; @card 00:E02D vec_cd_fade
; WHAT IT DOES: The jump table's entry for cd_fade: goes to it.
; HOW: A JMP to cd_fade, no frame of its own: the caller's JSR frame is the one cd_fade returns through.
; WAITS: no.
; Entry, exit: as cd_fade.
vec_cd_fade:
    jmp cd_fade

; @card 00:E033 vec_ad_trans
; WHAT IT DOES: The jump table's entry for ad_trans: goes to it.
; HOW: A JMP to ad_trans, no frame of its own: the caller's JSR frame is the one ad_trans returns through.
; WAITS: no.
; Entry, exit: as ad_trans.
vec_ad_trans:
    jmp ad_trans

; @card 00:E036 vec_ad_read
; WHAT IT DOES: The jump table's entry for ad_read: goes to it.
; HOW: A JMP to ad_read, no frame of its own: the caller's JSR frame is the one ad_read returns through.
; WAITS: no.
; Entry, exit: as ad_read.
vec_ad_read:
    jmp ad_read

; @card 00:E03C vec_ad_play
; WHAT IT DOES: The jump table's entry for ad_play: goes to it.
; HOW: A JMP to ad_play, no frame of its own: the caller's JSR frame is the one ad_play returns through.
; WAITS: no.
; Entry, exit: as ad_play.
vec_ad_play:
    jmp ad_play

; @card 00:E03F vec_ad_cplay
; WHAT IT DOES: The jump table's entry for ad_cplay: goes to it.
; HOW: A JMP to ad_cplay, no frame of its own: the caller's JSR frame is the one ad_cplay returns through.
; WAITS: only through ad_cplay.
; Entry, exit: as ad_cplay.
vec_ad_cplay:
    jmp ad_cplay

; @card 00:E042 vec_ad_stop
; WHAT IT DOES: The jump table's entry for ad_stop: goes to it.
; HOW: A JMP to ad_stop, no frame of its own: the caller's JSR frame is the one ad_stop returns through.
; WAITS: no.
; Entry, exit: as ad_stop.
vec_ad_stop:
    jmp ad_stop

; @card 00:E045 vec_ad_stat
; WHAT IT DOES: The jump table's entry for ad_stat: goes to it.
; HOW: A JMP to ad_stat, no frame of its own: the caller's JSR frame is the one ad_stat returns through.
; WAITS: no.
; Entry, exit: as ad_stat.
vec_ad_stat:
    jmp ad_stat

; @card 00:E04B vec_bm_free
; WHAT IT DOES: The jump table's entry for bm_free: goes to it.
; HOW: A JMP to bm_free, no frame of its own: the caller's JSR frame is the one bm_free returns through.
; WAITS: no.
; Entry, exit: as bm_free.
vec_bm_free:
    jmp bm_free

; @card 00:E04E vec_bm_read
; WHAT IT DOES: The jump table's entry for bm_read: goes to it.
; HOW: A JMP to bm_read, no frame of its own: the caller's JSR frame is the one bm_read returns through.
; WAITS: no.
; Entry, exit: as bm_read.
vec_bm_read:
    jmp bm_read

; @card 00:E051 vec_bm_write
; WHAT IT DOES: The jump table's entry for bm_write: goes to it.
; HOW: A JMP to bm_write, no frame of its own: the caller's JSR frame is the one bm_write returns through.
; WAITS: no.
; Entry, exit: as bm_write.
vec_bm_write:
    jmp bm_write

; @card 00:E054 vec_bm_delete
; WHAT IT DOES: The jump table's entry for bm_delete: goes to it.
; HOW: A JMP to bm_delete, no frame of its own: the caller's JSR frame is the one bm_delete returns through.
; WAITS: no.
; Entry, exit: as bm_delete.
vec_bm_delete:
    jmp bm_delete

; @card 00:E05A vec_ex_getver
; WHAT IT DOES: The jump table's entry for ex_getver: goes to it.
; HOW: A JMP to ex_getver, no frame of its own: the caller's JSR frame is the one ex_getver returns through.
; WAITS: no.
; Entry, exit: as ex_getver.
vec_ex_getver:
    jmp ex_getver

; @card 00:E05D vec_ex_setvec
; WHAT IT DOES: The jump table's entry for ex_setvec: goes to it.
; HOW: A JMP to ex_setvec, no frame of its own: the caller's JSR frame is the one ex_setvec returns through.
; WAITS: no.
; Entry, exit: as ex_setvec.
vec_ex_setvec:
    jmp ex_setvec

; @card 00:E060 vec_ex_getfnt
; WHAT IT DOES: The jump table's entry for ex_getfnt: goes to it.
; HOW: A JMP to ex_getfnt, no frame of its own: the caller's JSR frame is the one ex_getfnt returns through.
; WAITS: no.
; Entry, exit: as ex_getfnt.
vec_ex_getfnt:
    jmp ex_getfnt

; @card 00:E069 vec_ex_scrsiz
; WHAT IT DOES: The jump table's entry for ex_scrsiz: goes to it.
; HOW: A JMP to ex_scrsiz, no frame of its own: the caller's JSR frame is the one ex_scrsiz returns through.
; WAITS: no.
; Entry, exit: as ex_scrsiz.
vec_ex_scrsiz:
    jmp ex_scrsiz

; @card 00:E06C vec_ex_dotmod
; WHAT IT DOES: The jump table's entry for ex_dotmod: goes to it.
; HOW: A JMP to ex_dotmod, no frame of its own: the caller's JSR frame is the one ex_dotmod returns through.
; WAITS: no.
; Entry, exit: as ex_dotmod.
vec_ex_dotmod:
    jmp ex_dotmod

; @card 00:E06F vec_ex_scrmod
; WHAT IT DOES: The jump table's entry for ex_scrmod: goes to it.
; HOW: A JMP to ex_scrmod, no frame of its own: the caller's JSR frame is the one ex_scrmod returns through.
; WAITS: no.
; Entry, exit: as ex_scrmod.
vec_ex_scrmod:
    jmp ex_scrmod

; @card 00:E0A2 vec_ex_satclr
; WHAT IT DOES: The jump table's entry for ex_satclr: goes to it.
; HOW: A JMP to ex_satclr, no frame of its own: the caller's JSR frame is the one ex_satclr returns through.
; WAITS: no.
; Entry, exit: as ex_satclr.
vec_ex_satclr:
    jmp ex_satclr

; @card 00:E0BD vec_ma_mul8u
; WHAT IT DOES: The jump table's entry for ma_mul8u: goes to it.
; HOW: A JMP to ma_mul8u, no frame of its own: the caller's JSR frame is the one ma_mul8u returns through.
; WAITS: no.
; Entry, exit: as ma_mul8u.
vec_ma_mul8u:
    jmp ma_mul8u

; @card 00:E0DE vec_ex_memopen
; WHAT IT DOES: The jump table's entry for ex_memopen: goes to it.
; HOW: A JMP to ex_memopen, no frame of its own: the caller's JSR frame is the one ex_memopen returns through.
; WAITS: no.
; Entry, exit: as ex_memopen.
vec_ex_memopen:
    jmp ex_memopen

; @card 00:E063 vec_ex_joysns
; WHAT IT DOES: The jump table's entry for ex_joysns: goes to it.
; HOW: A JMP to ex_joysns, no frame of its own: the caller's JSR frame is the one ex_joysns returns through.
; WAITS: no.
; Entry, exit: as ex_joysns.
vec_ex_joysns:
    jmp ex_joysns
