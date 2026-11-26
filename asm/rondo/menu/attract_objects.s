; @routine 00C1:003 TitleScreenEntry
; @run 6B:8003
; WHAT IT DOES: The title overlay's entry for the logo mode's frame: runs TitleScreenStep.
; HOW: A jump to TitleScreenStep.
; WAITS: no.
; Entry: Y kept. Exit: as TitleScreenStep.
TitleScreenEntry:
    jmp TitleScreenStep

; @routine 00C1:006 AttractEntry
; @run 6B:8006
; WHAT IT DOES: The overlay's entry for the attract mode's frame: runs AttractStep.
; HOW: A jump to AttractStep.
; WAITS: yes, through AttractStep.
; Entry: Y kept. Exit: as AttractStep.
AttractEntry:
    jmp AttractStep

; @routine 00C1:009 SceneObjectsEntry
; @run 6B:8009
; WHAT IT DOES: The overlay's entry for a frame of the scene's objects: runs StepSceneObjects.
; HOW: A jump to StepSceneObjects.
; WAITS: no.
; Entry: nothing. Exit: as StepSceneObjects.
; not checked: as StepSceneObjects.
SceneObjectsEntry:
    jmp StepSceneObjects

; @routine 00C2:16B StepSceneObjects
; @run 6B:896B
; WHAT IT DOES: Runs every scene object that has a kind: a loop over the 88 slots, each with a kind sent to its handler.
; HOW: OBJ->kind read for each slot; DispatchSceneObject for the ones that are not zero.
; WAITS: no.
; Entry: nothing. Exit: X $58, Z and C set, N clear (the loop's last compare); A the last slot's kind or what its handler left;
; Y and V as the last handler left them.
; not checked: as EndingObjectsStep: only the idle loop is compared (curated/scene_objects_idle.txt).
StepSceneObjects:
    clx
StepSceneObjects_at1:
    lda a:OBJ_2750,x
    beq StepSceneObjects_at0
    jsr DispatchSceneObject
StepSceneObjects_at0:
    inx
    cpx #$58
    bne StepSceneObjects_at1
    rts

; @routine 00C2:17A DispatchSceneObject
; @run 6B:897A
; WHAT IT DOES: Sends a scene object to the handler its kind names: the table at $898A holds one address per kind, kind 1 first.
; HOW: Kind less one doubled indexes the table; the word goes through zTemp00 and zTemp01; the jump is made with A the word's high byte,
; Y the index, C bit 7 of kind less one, X the object. A handler this overlay's C does not hold runs as the original code.
; WAITS: no.
; Entry: A the kind (not zero), X the object. Exit: as the handler leaves them.
DispatchSceneObject:
    dec a
    asl a
    tay
    lda a:$898A,y
    sta z:zTemp00
    lda a:$898B,y
    sta z:zTemp01
    jmp (ZeroPage)
