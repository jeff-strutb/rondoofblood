# Coverage for the 41 card routines no script reaches

Measured by the runner's A5 at be14573 (34 scripts), and unchanged by this branch's landings: 87 identical, 7 divergent, 41 uncovered.
The 7 divergent (ex_colorcmd, ex_getfnt, sjis_to_font_index, expand_12dot_glyph, put_font_byte, ad_read, ad_play) are now identical on the
scripts that reach them: 40452e3, 1128d6c, 9b1e65f. Nothing below is verified; each stays UNVERIFIED until its route has run.

Order of routes, PROCEDURE section 6: 1 a script with real input, 2 a poke of work RAM where the game then runs the code, 3 a unit call at the
routine's own entry, 4 exempt by proof (byte range and proof to the master first).

| routine(s) | what reaches it in the game | first route to try |
|---|---|---|
| ad_write, ad_read_to_banks, aim_bx_at_vram_port, ad_read (VRAM and bank arms) | the game does not call them (it loads ADPCM by ad_trans) | 3: unit calls at each entry, with A, _AX to _DH and the ADPCM latch set; the stub-built TMA/TAM calls need the harness to run the RAM stub |
| ad_cplay, cplay_advance, ad_trans_begin, ad_trans_step, ad_trans_select, ad_trans_command, ad_trans_start_dma, ad_trans_finish | continuous ADPCM play from disc and the stepped transfer; no game path found | 3: unit calls; ad_trans_step's callees poll the CD unit, so the unit call needs the unit's ports in a state the checker's core gives |
| cd_seek, cd_bytes_to_sectors, disc_end_minus_one_frame, cd_request_sense, cd_retry_after_error, cd_drop_extra_data, cd_read_to_vram_fast, cd_read_to_vram_slow, clear_cd_result, cd_pcmrd | CD seek, the error retry path, reads into VRAM, PCM read: not run by the 34 scripts | 1 if a script plays a CD-DA seek or provokes a read error, else 3 |
| bm_files, bm_format | the backup-memory menu's file list and format; BackupFormatMemory calls bm_format but no script formats | 1: a menu script that formats a blank memory (menu_files reaches bm_files' neighbours but not these) |
| ex_imode, ex_vmode, ex_rcron, ex_rcroff, ex_bgoff, ex_spron, ex_sproff, ex_dspon, ex_dspoff, ex_setrcr, ex_setred, ex_setwrt, ex_setdma, ex_sprput, ex_scrmod_fallback, write_vram_word_pending | VDC mode and display setters the game calls through the BIOS vectors only on paths no script takes | 3: unit calls at the entry (each is a few instructions); ex_setdma and ex_sprput need VRAM and a table |
| ex_binbcd, ex_rnd | the BIOS's binary to BCD and random number | 3: unit calls |

Not run by any script is not the same as dead: ledger rows stay gate_a5 = 0 for these until a route has run. The unit call's script syntax
is not filled in yet in PROCEDURE ("filled in when the checker lands"); until it is, route 3 cannot be written and these wait.
