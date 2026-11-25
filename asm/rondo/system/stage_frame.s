; @routine 024E:24F StepScrollAndRunObjects
; @run 6B:8A4F
; WHAT IT DOES: Finishes a stage's frame set-up: steps the stream with the scroll, maps the
; object banks, and runs the stage's objects.
; HOW: Calls the overlay's scroll step ($8A36), maps the bank pair 8 above the RAM base bank
; into MPR5 and MPR6, and jumps to the code at $A000 (RunObjects), which returns to the caller.
; Every stage overlay holds the same code here.
; WAITS: it can wait where a handler's code does.
; Entry: X the caller's. Exit: as RunObjects.
StepScrollAndRunObjects:
    jsr StageScrollStep
    lda #$08
    clc
    adc z:zRamBaseBank
    tam #$20
    inc a
    tam #$40
    jmp CopyTileBlockToVram
