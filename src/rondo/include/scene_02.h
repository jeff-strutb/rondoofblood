/* The cutscene overlay of sector $224B (menu/scene_02.c). */
#ifndef RONDO_SCENE_02_H
#define RONDO_SCENE_02_H

#include "routines.h"
#include "front_end.h"

Exit Scene02Frame(void);
void abi_Scene02Frame(void);
Exit Scene02ScriptStep(void);
void abi_Scene02ScriptStep(void);
Exit Scene02CommandStartListAndClear(void);
void abi_Scene02CommandStartListAndClear(void);
Exit Scene02CommandStartScript1(void);
void abi_Scene02CommandStartScript1(void);
Exit Scene02CommandStartScript2(void);
void abi_Scene02CommandStartScript2(void);
Exit Scene02CommandStartScript3(void);
void abi_Scene02CommandStartScript3(void);
Exit Scene02CommandPlayAdpcm(void);
void abi_Scene02CommandPlayAdpcm(void);
Exit Scene02CommandCdPlay(uint8_t index);
void abi_Scene02CommandCdPlay(void);
Exit Scene02CommandPlaceKind(void);
void abi_Scene02CommandPlaceKind(void);
Exit Scene02CommandPlaceNext(void);
void abi_Scene02CommandPlaceNext(void);
Exit Scene02CommandPlaceFrame(void);
void abi_Scene02CommandPlaceFrame(void);
Exit Scene02CommandPlaceTimed(void);
void abi_Scene02CommandPlaceTimed(void);
Exit Scene02CommandPlaceTitleObjectA(void);
void abi_Scene02CommandPlaceTitleObjectA(void);
Exit Scene02CommandPlaceTitleObjectB(void);
void abi_Scene02CommandPlaceTitleObjectB(void);
Exit Scene02CommandLoadBanks(void);
void abi_Scene02CommandLoadBanks(void);
Exit Scene02ObjectsStep(void);
void abi_Scene02ObjectsStep(void);
ObjExit Scene02DispatchKind(uint8_t kind, uint8_t object);
void abi_Scene02DispatchKind(void);
Exit Scene02Begin(uint8_t kept_x, uint8_t kept_y);
void abi_Scene02Begin(void);
ObjExit Scene02Descend(uint8_t object);
void abi_Scene02Descend(void);
ObjExit Scene02ScrollSet(uint8_t object);
void abi_Scene02ScrollSet(void);
ObjExit Scene02ScrollUp(uint8_t object);
void abi_Scene02ScrollUp(void);
ObjExit Scene02RaiseSecond(uint8_t object);
void abi_Scene02RaiseSecond(void);
ObjExit Scene02Bob(uint8_t object);
void abi_Scene02Bob(void);
ObjExit Scene02PlayListOnce(uint8_t object);
void abi_Scene02PlayListOnce(void);
ObjExit Scene02SlideForCount(uint8_t object);
void abi_Scene02SlideForCount(void);
ObjExit Scene02FrameList(uint8_t list, uint8_t object);
void abi_Scene02FrameList(void);
ObjExit Scene02StepsA(uint8_t object);
void abi_Scene02StepsA(void);
ObjExit Scene02StepsB(uint8_t object);
void abi_Scene02StepsB(void);

#endif
