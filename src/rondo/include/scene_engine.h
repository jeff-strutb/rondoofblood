/* The cutscene overlays' shared script engine (menu/scene_engine.c). */
#ifndef RONDO_SCENE_ENGINE_H
#define RONDO_SCENE_ENGINE_H

#include "routines.h"
#include "front_end.h"

/* One overlay's engine: its shift from the $5C overlay's addresses, and its object handlers by address. */
typedef struct {
    uint16_t shift;
    bool (*handler)(uint16_t target, uint8_t object, ObjExit *out);
} SceneOverlay;

void scene_obj_shim(ObjExit result);

/* What a move leaves in A and V (axis_move_flags_shim's sums, without touching the CPU): a handler that ends on its
 * move shows them. */
static inline uint8_t scene_move_high(AxisMove move)
{
    uint32_t sum = move.before + ((uint32_t)(int32_t)move.velocity & 0xFFFFFF);

    return (uint8_t)(sum >> 16);
}

static inline bool scene_move_v(AxisMove move)
{
    uint32_t added = (uint32_t)(int32_t)move.velocity & 0xFFFFFF;
    uint8_t sign = (uint8_t)(added >> 16);
    uint8_t high = (uint8_t)(move.before >> 16);
    uint8_t result = (uint8_t)((move.before + added) >> 16);

    return (~(sign ^ high) & (sign ^ result) & 0x80) != 0;
}
ObjExit scene_kept(uint8_t a, uint8_t nz);
ObjExit ScenePlayListOnce(uint16_t lists, uint16_t ret, uint8_t object);
ObjExit SceneSlide(const uint16_t speeds[4], uint16_t ret, uint8_t object);
uint16_t SceneStateJump(uint16_t table, uint8_t object, uint8_t *index, uint8_t *high);
uint8_t SceneFrameTick(void);
Exit SceneFrame(const SceneOverlay *ov);
Exit SceneScriptAdvance(const SceneOverlay *ov);
Exit SceneScriptStep(const SceneOverlay *ov);
Exit SceneCommandSubModeTwo(const SceneOverlay *ov);
Exit SceneCommandSetScroll(const SceneOverlay *ov);
Exit SceneCommandStartListAndClear(const SceneOverlay *ov);
Exit SceneCommandStartList(const SceneOverlay *ov);
Exit SceneCommandStartScript1(const SceneOverlay *ov);
Exit SceneCommandStartScript2(const SceneOverlay *ov);
Exit SceneCommandStartScript3(const SceneOverlay *ov);
Exit SceneCommandPlayAdpcm(const SceneOverlay *ov);
Exit SceneCommandCdPlay(const SceneOverlay *ov, uint8_t index);
Exit SceneCommandPlaceKind(const SceneOverlay *ov);
Exit SceneCommandPlaceNext(const SceneOverlay *ov);
Exit SceneCommandPlaceFrame(const SceneOverlay *ov);
Exit SceneCommandPlaceTimed(const SceneOverlay *ov);
void SceneClearObjects(const SceneOverlay *ov);
Exit ScenePlaceObject(const SceneOverlay *ov, uint8_t object, uint8_t y);
Exit SceneCommandPlaceTitleObjectA(const SceneOverlay *ov);
Exit SceneCommandPlaceTitleObjectB(const SceneOverlay *ov);
Exit SceneCommandLoadBanks(const SceneOverlay *ov);
Exit SceneObjectsStep(const SceneOverlay *ov);
ObjExit SceneDispatchKind(const SceneOverlay *ov, uint8_t kind, uint8_t object);

#endif
