# L1 routines landed with part of their behaviour unchecked

Each routine's header says `not checked: <what>`. How each is to be checked:

| Routines | Unchecked | Why | How it will be checked |
|---|---|---|---|
| EndingHoldTimer (010D:5F3) | all of it alone | pulls its caller's frame, so it cannot run standalone | its callers below, in a unit call with the caller's frame on the stack |
| EndingScrollWindowSteps, EndingStartList3, EndingStartScript18, EndingSplitScrolls, EndingScrollsOn, EndingRiseFrame31, EndingRiseFrame32A and B, EndingRiseWithList, EndingHoldThenScript19, EndingCreditScrollStart | the arm where the hold timer has not run out | the original drops the caller's frame, the draft goes on (FRAME DROPPED); 22 to 373 of 400 cases compare, every difference seen is that arm | a unit call that lays the dispatch's frame (return $81E3 less one) under the routine and compares the stack and registers on return; or A5 on the ending script |
| EndingObjectsStep, EndingFrame, StepSceneObjects, SceneObjectsEntry | the loop over handlers | the jump to a handler is computed; through mode cannot follow it, only the idle loop matches (curated/ending_objects_idle.txt, scene_objects_idle.txt) | A5 on the ending and attract scripts (they run every handler) |
| EndingStep, EndingEffectDispatch, EndingDispatchKind, DispatchSceneObject | one to two cases of 400 | a table word outside this overlay's C runs as original code | none needed beyond A5 |
| EndingEffectStep, EndingEffectEntry | the paused path | untraced in the draft | A5 on a script that pauses in the ending, or a curated case with zPaused bit 0 set once the draft has the path |
| TitleSetUp | backup results other than 0 | the draft only ran result 0 | curated case with a formatted backup memory (BackupRead returns 0, then a non-zero result); needs the BackupRead case set (HUBM at $8000, file entry after $8010) |
| TitleStep, TitleEntry, TitleScreenStep, AttractStep | the START skip arm (8024) | no draft code | A5 on a script that presses START in the title |
| AttractStartCheck | one case in SoundDriverCommand | the sound driver's own frames | the sound driver's checks (L5) |
| EndingCommandLoadBanks | whole routine | differs inside the card's ad_read, whose draft pushes frames its C does not | card ad_read frames fixed by the card writer, then rerun with curated/ending_script_y3.txt |
| DrawGlyphPair | past the font call | differs inside the card's sjis_to_font_index | card font frames fixed, then rerun with curated/glyph_pair.txt |
| DrawGlyphText | with the real callees | matches only with callees as filed wrappers on both sides | rerun after DrawGlyphPair is checked |
| LoadSceneScript | its A5 divergence | 1013 of 1200 comparable cases match; the rest differ in the overlay call's tail flags; A5 shows an extra read of zBiosCl in the first call | the runner's A5 record for attract after the next landing; the original's accesses in call 1 are needed |

Curated files: `script_handlers_y3.txt` (renamed from attract.txt; Y 3 for the 00C2 script handlers, by name only), `ending_script_y3.txt`, `ending_script.txt`, `ending_objects.txt`, `ending_objects_idle.txt`, `scene_objects_idle.txt`, `glyph_pair.txt`, `glyph_text.txt`.
