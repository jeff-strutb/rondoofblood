; Overlay entry jumps: each overlay begins with a table of JMPs that the kernel and the dispatchers call
; through. One routine per distinct jump; its copies in other overlays have the same bytes, and the C goes
; to whichever routine that overlay has at the target.

; @routine 050D:000 StageVec8000_050D
; @run 6B:8000
; WHAT IT DOES: Goes to the stage overlay's frame at $8018.
; HOW: One jump.
; WAITS: where the routine at $8018 waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $8018.
StageVec8000_050D:
    jmp $8018

; @routine 050D:006 StageVec8006_050D
; @run 6B:8006
; WHAT IT DOES: Goes to TerrainTileRead.
; HOW: One jump.
; WAITS: where the routine at $967F waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $967F.
StageVec8006_050D:
    jmp TerrainTileRead

; @routine 050D:00C StageVec800C_050D
; @run 6B:800C
; WHAT IT DOES: Goes to TerrainLookup.
; HOW: One jump.
; WAITS: where the routine at $95AB waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $95AB.
StageVec800C_050D:
    jmp TerrainLookup

; @routine 050D:00F StageVec800F_050D
; @run 6B:800F
; WHAT IT DOES: Goes to TerrainLookupAlt.
; HOW: One jump.
; WAITS: where the routine at $960B waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $960B.
StageVec800F_050D:
    jmp TerrainLookupAlt

; @routine 071D:006 StageVec8006_071D
; @run 6B:8006
; WHAT IT DOES: Goes to the overlay's tile read at $9991.
; HOW: One jump.
; WAITS: where the routine at $9991 waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $9991.
StageVec8006_071D:
    jmp $9991

; @routine 071D:00C StageVec800C_071D
; @run 6B:800C
; WHAT IT DOES: Goes to ProbeBoxes0720.
; HOW: One jump.
; WAITS: where the routine at $98BD waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $98BD.
StageVec800C_071D:
    jmp ProbeBoxes0720

; @routine 071D:00F StageVec800F_071D
; @run 6B:800F
; WHAT IT DOES: Goes to ProbeBoxesFlagged0720.
; HOW: One jump.
; WAITS: where the routine at $991D waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $991D.
StageVec800F_071D:
    jmp ProbeBoxesFlagged0720

; @routine 0111:006 SceneVec8006_0111
; @run 6C:8006
; WHAT IT DOES: Goes to the scene overlay's call at $8169.
; HOW: One jump.
; WAITS: where the routine at $8169 waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $8169.
SceneVec8006_0111:
    jmp VramListRun0111

; @routine 00C1:00C StageLoadVec800C_00C1
; @run 6B:800C
; WHAT IT DOES: Goes to the stage-load overlay's code at $91E2.
; HOW: One jump.
; WAITS: where the routine at $91E2 waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $91E2.
StageLoadVec800C_00C1:
    jmp FileMenuFrame

; @routine 072C:66E JumpToFollowPathAndShow
; @run 70:BE6E
; WHAT IT DOES: Goes to FollowPathAndShow.
; HOW: One jump.
; WAITS: where the routine at $BDD0 waits; the jump adds nothing to the stack.
; Entry, exit: as the routine at $BDD0.
JumpToFollowPathAndShow:
    jmp FollowPathAndShow
