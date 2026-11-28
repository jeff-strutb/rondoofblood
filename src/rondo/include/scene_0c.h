/* The cutscene overlay of sector $20BC (menu/scene_0c.c). */
#ifndef RONDO_SCENE_0C_H
#define RONDO_SCENE_0C_H

#include "routines.h"
#include "front_end.h"

Exit Scene0CFrame(void);
void abi_Scene0CFrame(void);
Exit Scene0CScriptStep(void);
void abi_Scene0CScriptStep(void);
Exit Scene0CCommandStartListAndClear(void);
void abi_Scene0CCommandStartListAndClear(void);
Exit Scene0CCommandStartScript1(void);
void abi_Scene0CCommandStartScript1(void);
Exit Scene0CCommandStartScript2(void);
void abi_Scene0CCommandStartScript2(void);
Exit Scene0CCommandStartScript3(void);
void abi_Scene0CCommandStartScript3(void);
Exit Scene0CCommandPlayAdpcm(void);
void abi_Scene0CCommandPlayAdpcm(void);
Exit Scene0CCommandCdPlay(uint8_t index);
void abi_Scene0CCommandCdPlay(void);
Exit Scene0CCommandPlaceKind(void);
void abi_Scene0CCommandPlaceKind(void);
Exit Scene0CCommandPlaceNext(void);
void abi_Scene0CCommandPlaceNext(void);
Exit Scene0CCommandPlaceFrame(void);
void abi_Scene0CCommandPlaceFrame(void);
Exit Scene0CCommandPlaceTimed(void);
void abi_Scene0CCommandPlaceTimed(void);
Exit Scene0CCommandPlaceTitleObjectA(void);
void abi_Scene0CCommandPlaceTitleObjectA(void);
Exit Scene0CCommandPlaceTitleObjectB(void);
void abi_Scene0CCommandPlaceTitleObjectB(void);
Exit Scene0CCommandLoadBanks(void);
void abi_Scene0CCommandLoadBanks(void);
Exit Scene0CObjectsStep(void);
void abi_Scene0CObjectsStep(void);
ObjExit Scene0CDispatchKind(uint8_t kind, uint8_t object);
void abi_Scene0CDispatchKind(void);
Exit Scene0CBegin(uint8_t kept_x, uint8_t kept_y);
void abi_Scene0CBegin(void);
ObjExit Scene0CDriftThree(uint8_t object);
void abi_Scene0CDriftThree(void);
ObjExit Scene0CPlayListOnce(uint8_t object);
void abi_Scene0CPlayListOnce(void);
ObjExit Scene0CSlideForCount(uint8_t object);
void abi_Scene0CSlideForCount(void);
ObjExit Scene0CFrameList(uint8_t list, uint8_t object);
void abi_Scene0CFrameList(void);

#endif
