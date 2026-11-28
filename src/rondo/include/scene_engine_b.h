/* The script engine of the cutscene overlays after a stage (menu/scene_engine_b.c): scenes $58-$59 (sector $267B)
 * and $5A-$5B (sector $2703). The same engine as scene_engine.h's, grown by commands, at places that do not move by
 * one shift: each overlay gives the address of every piece, and each piece's C takes the address it runs at. */
#ifndef RONDO_SCENE_ENGINE_B_H
#define RONDO_SCENE_ENGINE_B_H

#include "routines.h"
#include "front_end.h"

typedef struct {
    uint16_t step;          /* the script step (Scene58ScriptStep's place) */
    uint16_t objects;       /* the objects' loop */
    Exit (*objects_step)(void);   /* the C of the routine loaded at objects */
    uint16_t lengths;       /* the commands' lengths */
    uint16_t commands;      /* the commands' handlers */
    uint16_t kinds;         /* the object handlers by kind */
    uint16_t clear;         /* the copy of EndingClearObjects */
    uint16_t place;         /* the copy of EndingPlaceObject */
    /* The overlay's command handlers by address (index the doubled command, carry the doubling's); false for an
     * address it does not hold. */
    bool (*command)(uint16_t target, uint8_t index, bool carry, Exit *out);
    /* The overlay's object handlers by address; false for an address it does not hold. */
    bool (*handler)(uint16_t target, uint8_t object, ObjExit *out);
} SceneLayoutB;

Exit SceneBBegin(uint16_t at, uint8_t script_lo, uint8_t script_hi, uint8_t kept_x, uint8_t kept_y);
Exit SceneBFrame(const SceneLayoutB *ov, uint16_t at);
Exit SceneBWaitForRun(void);
Exit SceneBScriptAdvance(const SceneLayoutB *ov);
Exit SceneBScriptStep(const SceneLayoutB *ov);
Exit SceneBCommandEnd(uint8_t index, bool carry);
Exit SceneBCommandSubModeTwo(const SceneLayoutB *ov);
Exit SceneBCommandSetScroll(const SceneLayoutB *ov);
Exit SceneBCommandStartListAndClear(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandStartList(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandStartScript1(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandStartScript2(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandStartScript3(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandPlayAdpcm(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandCdPlay(const SceneLayoutB *ov, uint16_t at, uint8_t index);
Exit SceneBCommandPlaceKind(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandPlaceNext(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandPlaceFrame(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandPlaceTimed(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandPlaceWorked(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandPlaceTitleObjectA(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandPlaceTitleObjectALow(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandPlaceTitleObjectB(const SceneLayoutB *ov, uint16_t at);
Exit SceneBCommandBranchOnProgress(const SceneLayoutB *ov);
Exit SceneBCommandLoadBanks(const SceneLayoutB *ov, uint16_t at);
Exit SceneBObjectsStep(const SceneLayoutB *ov);
ObjExit SceneBDispatchKind(const SceneLayoutB *ov, uint8_t kind, uint8_t object);
ObjExit SceneBAfter(ObjExit second, ObjExit first);
ObjExit SceneBCycleLists(uint16_t ret, uint8_t delay, uint8_t limit, uint8_t object);
ObjExit SceneBShake(uint16_t xs, uint16_t ys, uint8_t object);
ObjExit SceneBSpawnCycle(uint16_t slots, uint16_t steps, uint16_t ret, uint8_t object);
ObjExit SceneBSwing(uint16_t at, const uint8_t lo[6], const uint8_t hi[6], uint8_t object);
ObjExit SceneBLoadPatterns(uint16_t at, uint8_t y, uint8_t object);
ObjExit SceneBSpawnSparkle(uint16_t at, uint16_t slots, uint8_t base, uint8_t object);
ObjExit SceneBSparkles(uint16_t at, uint16_t spawn, uint16_t slots, uint8_t base, uint8_t object);
ObjExit SceneBSparkleFall(uint16_t at, uint8_t object);
ObjExit SceneBScrollLayers(uint16_t lo, uint16_t mid, uint16_t hi, uint8_t raster, uint8_t end, uint8_t object);

#endif
