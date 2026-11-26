# L5 recheck list (2026-11-25)

Everything below is on main unless stated. Verdict words are those of the gates at the commit named.

| Routine / item | State | Commit / evidence |
|---|---|---|
| SoundLoadWave (004E:654) | C, NOT CHECKED. Block move done in C (stub bytes, JSR frame, TIN push Y,A,X, 32 reads/writes, pulls). A5 sound_test: EXTRA 21E1/21E0/21DF, read of the wave byte, MOVED 1FE806, because `place()` stops at a CALL to an untagged target. Checker change approved by Jeff; the drafter builds it. Rerun A5 after it lands. | d9773ac; note `side-session-notes/.../soundloadwave-a5.md` |
| ReadSequence calls 114/115 | Frames fixed earlier; calls 113/115 checked, the remaining two from a trace of the original not re-verified here. Recheck with A5 sound_test. | sound_driver.c |
| ModeStage (0045:265) | liftcmp DIFFERS (173 match, 123 differ): all differences begin inside callee EndingStep (8000), a run-address twin. Called a harness limit by me; the master wants typed-access to confirm from evidence. Not accepted. | game_mode.c |
| ModeAttract (0045:161) | liftcmp 581 match, 12 differ, 7 untraced: differences inside overlay code (AttractStep). | game_mode.c |
| Camera routines waiting on PlayerWall/PlayerFloor (player_terrain.c, L2 a29ff94) | After rebase: PlayerWallRight 297/24 differ, PlayerWallLeft 343/30, MovePlayerYIfMoving 383/1; MovePlayerRight/LeftOrWall, PlayerXOnScreen (20), CameraModeVertical (20) differ only through those wall cases. L2 has the list. | 6306827, 5fb5306 |
| camera.c not yet to the standard | PlayerAndCameraFrame (0/399), CameraModeFollow (0/307), CameraModeLeftWall (no draft). Direction choice was changed for them but their frames/order are unverified. | camera.c |
| ad_play patch | Not landed; for the card writer (local_14f40d86) to check. Fixes the 3 flagged calls of SoundCommand, SoundDriverCommand, SoundStartVoice. | docs/rondo/decomp/l5-held/held_ad_play.patch.txt |
| Other divergents | Sound: SoundStartMusic (liftcmp 0/385), give_back_channel Y placeholder, SoundCheckPause. game_mode.c: SaveProgress, SaveScore, NextSubMode NOT CHECKED; FrameModeDispatch 1 differ. Held (off build): Halve/Clamp copy callers in docs/rondo/decomp/l5-held. Commits labeled "wip" 25400d9, 377d0ef cannot be rewritten. | |

Landed and liftcmp-clean this session: ModeBackToTitle (ac2ba5a), ModeDead/ModeGameOverWait (09f463f), FadeAndStopSound (59b2cdc), ModeAttractPlay (866eee3), camera leaves (b962b26), MovePlayerY etc. (12c355c, b9b0e63, d284ea0, e073e7c). MovePlayerUp: 263 of 400 reach untraced bytes.
