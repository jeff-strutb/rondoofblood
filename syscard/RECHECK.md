# Card routines landed unchecked

Each is filed to the access standard from its assembly, but no script reaches it, so no A5 and no liftcmp case ran it.
They stay here until their coverage (PROCEDURE section 6) runs.

| routine | where | landed |
|---|---|---|
| ad_write | 00:F4D8 | 1128d6c |
| ad_read_to_banks | 00:F46B | 1128d6c |
| aim_bx_at_vram_port | 00:F6F5 | 1128d6c |
| ex_colorcmd | 00:E509 | the block move it writes in RAM is done in C; A5 not run on it until the checker compares RAM-built code with its caller (9b1e65f ran it as original code: reverted to C) |
