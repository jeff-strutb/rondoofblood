/* The cutscene overlay of scenes $5C and $5D (menu/scene_5c.c). */
#ifndef RONDO_SCENE_5C_H
#define RONDO_SCENE_5C_H

#include "routines.h"
#include "front_end.h"

Exit Scene5CBegin(uint8_t kept_x, uint8_t kept_y);
void abi_Scene5CBegin(void);
Exit Scene5CFrame(void);
void abi_Scene5CFrame(void);
Exit Scene5CScriptAdvance(void);
void abi_Scene5CScriptAdvance(void);
Exit Scene5CScriptStep(void);
void abi_Scene5CScriptStep(void);
Exit Scene5CCommandSubModeTwo(void);
void abi_Scene5CCommandSubModeTwo(void);
Exit Scene5CCommandSetScroll(void);
void abi_Scene5CCommandSetScroll(void);
Exit Scene5CCommandStartListAndClear(void);
void abi_Scene5CCommandStartListAndClear(void);
Exit Scene5CCommandStartList(void);
void abi_Scene5CCommandStartList(void);
Exit Scene5CCommandStartScript1(void);
void abi_Scene5CCommandStartScript1(void);
Exit Scene5CCommandStartScript2(void);
void abi_Scene5CCommandStartScript2(void);
Exit Scene5CCommandStartScript3(void);
void abi_Scene5CCommandStartScript3(void);
Exit Scene5CCommandPlayAdpcm(void);
void abi_Scene5CCommandPlayAdpcm(void);
Exit Scene5CCommandCdPlay(uint8_t index);
void abi_Scene5CCommandCdPlay(void);
Exit Scene5CCommandPlaceKind(void);
void abi_Scene5CCommandPlaceKind(void);
Exit Scene5CCommandPlaceNext(void);
void abi_Scene5CCommandPlaceNext(void);
Exit Scene5CCommandPlaceFrame(void);
void abi_Scene5CCommandPlaceFrame(void);
Exit Scene5CCommandPlaceTimed(void);
void abi_Scene5CCommandPlaceTimed(void);
void Scene5CClearObjects(void);
void abi_Scene5CClearObjects(void);
Exit Scene5CPlaceObject(uint8_t object, uint8_t y);
void abi_Scene5CPlaceObject(void);
Exit Scene5CCommandPlaceTitleObjectA(void);
void abi_Scene5CCommandPlaceTitleObjectA(void);
Exit Scene5CCommandPlaceTitleObjectB(void);
void abi_Scene5CCommandPlaceTitleObjectB(void);
Exit Scene5CCommandLoadBanks(void);
void abi_Scene5CCommandLoadBanks(void);
Exit Scene5CObjectsStep(void);
void abi_Scene5CObjectsStep(void);
ObjExit Scene5CDispatchKind(uint8_t kind, uint8_t object);
void abi_Scene5CDispatchKind(void);
ObjExit Scene5CShowFirstObject(uint8_t object);
void abi_Scene5CShowFirstObject(void);
ObjExit Scene5CScrollUpAndClear(uint8_t object);
void abi_Scene5CScrollUpAndClear(void);
ObjExit Scene5CDriftFiveObjects(uint8_t object);
void abi_Scene5CDriftFiveObjects(void);
ObjExit Scene5CPlayListOnce(uint8_t object);
void abi_Scene5CPlayListOnce(void);
ObjExit Scene5CSlideForCount(uint8_t object);
void abi_Scene5CSlideForCount(void);
ObjExit Scene5CFrameList(uint8_t list, uint8_t object);
void abi_Scene5CFrameList(void);
Exit Scene5CStep(uint8_t kept_y);
void abi_Scene5CStep(void);

#endif
