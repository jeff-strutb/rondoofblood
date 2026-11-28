/* The cutscene overlay of sector $1E61 (menu/scene_0e.c). */
#ifndef RONDO_SCENE_0E_H
#define RONDO_SCENE_0E_H

#include "routines.h"
#include "front_end.h"

Exit Scene0EFrame(void);
void abi_Scene0EFrame(void);
Exit Scene0EScriptStep(void);
void abi_Scene0EScriptStep(void);
Exit Scene0ECommandStartListAndClear(void);
void abi_Scene0ECommandStartListAndClear(void);
Exit Scene0ECommandStartScript1(void);
void abi_Scene0ECommandStartScript1(void);
Exit Scene0ECommandStartScript2(void);
void abi_Scene0ECommandStartScript2(void);
Exit Scene0ECommandStartScript3(void);
void abi_Scene0ECommandStartScript3(void);
Exit Scene0ECommandPlayAdpcm(void);
void abi_Scene0ECommandPlayAdpcm(void);
Exit Scene0ECommandCdPlay(uint8_t index);
void abi_Scene0ECommandCdPlay(void);
Exit Scene0ECommandPlaceKind(void);
void abi_Scene0ECommandPlaceKind(void);
Exit Scene0ECommandPlaceNext(void);
void abi_Scene0ECommandPlaceNext(void);
Exit Scene0ECommandPlaceFrame(void);
void abi_Scene0ECommandPlaceFrame(void);
Exit Scene0ECommandPlaceTimed(void);
void abi_Scene0ECommandPlaceTimed(void);
Exit Scene0ECommandPlaceTitleObjectA(void);
void abi_Scene0ECommandPlaceTitleObjectA(void);
Exit Scene0ECommandPlaceTitleObjectB(void);
void abi_Scene0ECommandPlaceTitleObjectB(void);
Exit Scene0ECommandLoadBanks(void);
void abi_Scene0ECommandLoadBanks(void);
Exit Scene0EObjectsStep(void);
void abi_Scene0EObjectsStep(void);
ObjExit Scene0EDispatchKind(uint8_t kind, uint8_t object);
void abi_Scene0EDispatchKind(void);
ObjExit Scene0ERiseScroll(uint8_t object);
void abi_Scene0ERiseScroll(void);
ObjExit Scene0EPieceStart(uint8_t object);
void abi_Scene0EPieceStart(void);
ObjExit Scene0EPieceSwingA(uint8_t object);
void abi_Scene0EPieceSwingA(void);
ObjExit Scene0EPieceSwingB(uint8_t object);
void abi_Scene0EPieceSwingB(void);
ObjExit Scene0EPieceStartB(uint8_t object);
void abi_Scene0EPieceStartB(void);
ObjExit Scene0EBandsA(uint8_t object);
void abi_Scene0EBandsA(void);
ObjExit Scene0EBandsB(uint8_t object);
void abi_Scene0EBandsB(void);
ObjExit Scene0EPlayListOnce(uint8_t object);
void abi_Scene0EPlayListOnce(void);
ObjExit Scene0ESlideForCount(uint8_t object);
void abi_Scene0ESlideForCount(void);
ObjExit Scene0EFrameList(uint8_t list, uint8_t object);
void abi_Scene0EFrameList(void);
ObjExit Scene0EStepsA(uint8_t object);
void abi_Scene0EStepsA(void);
ObjExit Scene0EStepsB(uint8_t object);
void abi_Scene0EStepsB(void);

#endif
