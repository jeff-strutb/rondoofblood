# Coverage for the card routines no script reaches

The runner's A5 at be14573 (34 scripts) had 87 card routines identical, 7 divergent and 41 uncovered. Since then the 7 divergent
are identical (40452e3, 1128d6c, 9b1e65f/e5d5907 for ex_colorcmd) and the uncovered ones are run by unit calls (route 3 of
PROCEDURE section 6), each script in tools/rondo/scripts:

| script | routines it reaches (A5 identical unless said) |
|---|---|
| card_unit_vdc | ex_imode, ex_vmode, ex_rcron, ex_rcroff, ex_bgoff, ex_spron, ex_sproff, ex_dspon, ex_dspoff, ex_setrcr, ex_setred, ex_setwrt, ex_setdma, ex_binbcd, ex_rnd, ex_scrmod_fallback |
| card_unit_misc | cd_bytes_to_sectors, disc_end_minus_one_frame, clear_cd_result, write_vram_word_pending, cd_pcmrd, cplay_advance, ex_sprput |
| card_unit_adpcm | ad_read and ad_write to memory and VRAM, aim_bx_at_vram_port, ad_read_byte; the banks arms and ad_read_to_banks differ only in the order of the RAM stub's frame: not checked |
| card_unit_backup | bm_format, bm_free, bm_files, bm_write, bm_read, bm_delete, bm_new_file, bm_place_span, bm_sum_file, bm_move, bm_copy_in, bm_copy_out, bm_move_direction, bm_count_down |
| card_unit_cd | cd_seek, cd_request_sense, cd_drop_extra_data, cd_retry_after_error |
| card_unit_trans | ad_trans_begin, ad_trans_step, ad_trans_select, ad_trans_command, ad_trans_start_dma, ad_trans_finish |
| card_unit_cplay | ad_cplay |
| card_unit_read | cd_read to VRAM, cd_read_to_vram_fast, cd_read_to_vram_slow, the bad request; the MPR arm: not checked |

Defects the unit calls found, now fixed: ex_rnd (no flag push, accesses out of order), ad_poll.c (the five ad_trans routines'
accesses out of order), ad_trans_begin (the ADPCM address kept in the wrong bytes), the carry out of the window shift in the
banks arms, cd_read_to_vram_fast's V.

Not covered, and why: the arms of bm_* routines named in their headers (a copy crossing a page, a refused header), the ad_read
and ad_write banks arms and cd_read's MPR arm (RAM stubs done in C, not checked: RECHECK.md), cd_subq, cd_play, cd_search,
cd_pause, cd_fade as the sweep covers them. A routine not listed here and not in RECHECK.md was run by a game script.
