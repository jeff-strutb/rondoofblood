# Card routines landed unchecked

Each is filed to the access standard from its assembly, but no script reaches it, so no A5 and no liftcmp case ran it.
They stay here until their coverage (PROCEDURE section 6) runs.

| routine | where | landed |
|---|---|---|
| ad_write | 00:F4D8 | 1128d6c |
| ad_read_to_banks | 00:F46B | 1128d6c |
| aim_bx_at_vram_port | 00:F6F5 | 1128d6c |
| ex_colorcmd | 00:E509 | the block move it writes in RAM is done in C; A5 not run on it until the checker compares RAM-built code with its caller (9b1e65f ran it as original code: reverted to C) |
| ad_read (banks arm), ad_write (banks arm), ad_read_to_banks | 00:F407, 00:F4D8, 00:F46B | the TMA/TAM stubs they write in RAM are done in C; unit calls (card_unit_adpcm) judge every other arm identical, the banks arms only differ in the order the stub's frame is made, until the checker compares RAM-built code with its caller |
| cd_read (MPR arm) | 00:EC05 | the TMA/TAM stubs it writes in RAM are done in C; the unit call (card_unit_read) judges every other arm identical |

## Card code the traces run that has no C yet

From the catalog (tools/rondo/catalog.py, System Card: 169 entries, 162 with C): seven entries have none. They are filed
nowhere yet; their bytes come from the card's image.

| entry | kind | bytes | bytes run |
|---|---|---|---|
| 00:E14C | call | 191 | 182 |
| 00:E736 | interrupt | 12 | 11 |
| 00:E742 | call | 293 | 123 |
| 00:E870 | interrupt | 115 | 78 |
| 00:F6CE | call | 13 | 13 |
| 01:CB0F | call | 33 | 33 |
| 01:CB64 | call | 95 | 95 |

The master's figure of 31 raw-intake card routines is not what the catalog shows; the other 24 are not traced code.

