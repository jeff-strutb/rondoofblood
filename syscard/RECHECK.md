# Card routines landed unchecked

Nothing is unchecked at present. Earlier entries cleared: ad_write, ad_read_to_banks and aim_bx_at_vram_port (unit calls,
card_unit_adpcm), ex_colorcmd, the banks arms of ad_read and ad_write and cd_read's MPR arm (A5 identical on boot,
card_unit_adpcm and card_unit_read once the checker compared RAM-built code with its caller, d249cb1).

## Card code the traces run that has no C yet

From the catalog (tools/rondo/catalog.py, System Card: 169 entries): the entries without C are 00:E736 (interrupt, 12 B),
00:E742 (call, 293 B), 00:E870 (interrupt, 115 B) and 00:F6CE (13 B, a named entry into ad_stop). Done: 01:CB0F, 01:CB64 and
00:E14C.

## Original code the card's C still calls

| call | from | why |
|---|---|---|
| 00:E905 (the CD command send with its waits) | cd_interrupt | no script runs it: no C |
| 00:E6CF, which calls 02:CD12 and 02:C2EF in card bank 2 | card_irq1, when the reset mark is 1 | no script runs it, nor anything in bank 2: no C for E6CF or for bank 2 |
| the program at $2B26 | check_boot_disc | the disc's own boot program, not card code |
| the program's interrupt vectors at $2200 to $220B | the four jumps (E733, E867, E86A, E86D) | the program's code |
