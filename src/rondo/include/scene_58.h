/* The cutscene overlay of sectors $267B to $267E (menu/scene_58.c). */
#ifndef RONDO_SCENE_58_H
#define RONDO_SCENE_58_H

#include "routines.h"
#include "front_end.h"

Exit Scene58Step(uint8_t kept_y);
void abi_Scene58Step(void);
Exit Scene58Begin(uint8_t kept_x, uint8_t kept_y);
void abi_Scene58Begin(void);
Exit Scene58Frame(void);
void abi_Scene58Frame(void);
Exit Scene58WaitForRun(void);
void abi_Scene58WaitForRun(void);
Exit Scene58ScriptAdvance(void);
void abi_Scene58ScriptAdvance(void);
Exit Scene58ScriptStep(void);
void abi_Scene58ScriptStep(void);
Exit Scene58CommandEnd(uint8_t index, bool carry);
void abi_Scene58CommandEnd(void);
Exit Scene58CommandSubModeTwo(void);
void abi_Scene58CommandSubModeTwo(void);
Exit Scene58CommandSetScroll(void);
void abi_Scene58CommandSetScroll(void);
Exit Scene58CommandStartListAndClear(void);
void abi_Scene58CommandStartListAndClear(void);
Exit Scene58CommandStartList(void);
void abi_Scene58CommandStartList(void);
Exit Scene58CommandStartScript1(void);
void abi_Scene58CommandStartScript1(void);
Exit Scene58CommandStartScript2(void);
void abi_Scene58CommandStartScript2(void);
Exit Scene58CommandStartScript3(void);
void abi_Scene58CommandStartScript3(void);
Exit Scene58CommandPlayAdpcm(void);
void abi_Scene58CommandPlayAdpcm(void);
Exit Scene58CommandCdPlay(uint8_t index);
void abi_Scene58CommandCdPlay(void);
Exit Scene58CommandPlaceKind(void);
void abi_Scene58CommandPlaceKind(void);
Exit Scene58CommandPlaceNext(void);
void abi_Scene58CommandPlaceNext(void);
Exit Scene58CommandPlaceFrame(void);
void abi_Scene58CommandPlaceFrame(void);
Exit Scene58CommandPlaceTimed(void);
void abi_Scene58CommandPlaceTimed(void);
Exit Scene58CommandPlaceWorked(void);
void abi_Scene58CommandPlaceWorked(void);
Exit Scene58CommandPlaceTitleObjectALow(void);
void abi_Scene58CommandPlaceTitleObjectALow(void);
Exit Scene58CommandPlaceTitleObjectB(void);
void abi_Scene58CommandPlaceTitleObjectB(void);
Exit Scene58CommandBranchOnProgress(void);
void abi_Scene58CommandBranchOnProgress(void);
Exit Scene58CommandLoadBanks(void);
void abi_Scene58CommandLoadBanks(void);
Exit Scene58ObjectsStep(void);
void abi_Scene58ObjectsStep(void);
ObjExit Scene58DispatchKind(uint8_t kind, uint8_t object);
void abi_Scene58DispatchKind(void);
bool scene_58_handler(uint16_t target, uint8_t object, ObjExit *out);

#endif
