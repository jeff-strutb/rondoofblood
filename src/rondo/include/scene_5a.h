/* The cutscene overlay of sectors $2703 to $2706 (menu/scene_5a.c). */
#ifndef RONDO_SCENE_5A_H
#define RONDO_SCENE_5A_H

#include "routines.h"
#include "front_end.h"

Exit Scene5ABegin(uint8_t kept_x, uint8_t kept_y);
void abi_Scene5ABegin(void);
Exit Scene5AFrame(void);
void abi_Scene5AFrame(void);
Exit Scene5AScriptAdvance(void);
void abi_Scene5AScriptAdvance(void);
Exit Scene5AScriptStep(void);
void abi_Scene5AScriptStep(void);
Exit Scene5ACommandSubModeTwo(void);
void abi_Scene5ACommandSubModeTwo(void);
Exit Scene5ACommandSetScroll(void);
void abi_Scene5ACommandSetScroll(void);
Exit Scene5ACommandStartListAndClear(void);
void abi_Scene5ACommandStartListAndClear(void);
Exit Scene5ACommandStartList(void);
void abi_Scene5ACommandStartList(void);
Exit Scene5ACommandStartScript1(void);
void abi_Scene5ACommandStartScript1(void);
Exit Scene5ACommandStartScript2(void);
void abi_Scene5ACommandStartScript2(void);
Exit Scene5ACommandStartScript3(void);
void abi_Scene5ACommandStartScript3(void);
Exit Scene5ACommandPlayAdpcm(void);
void abi_Scene5ACommandPlayAdpcm(void);
Exit Scene5ACommandCdPlay(uint8_t index);
void abi_Scene5ACommandCdPlay(void);
Exit Scene5ACommandPlaceKind(void);
void abi_Scene5ACommandPlaceKind(void);
Exit Scene5ACommandPlaceNext(void);
void abi_Scene5ACommandPlaceNext(void);
Exit Scene5ACommandPlaceFrame(void);
void abi_Scene5ACommandPlaceFrame(void);
Exit Scene5ACommandPlaceTimed(void);
void abi_Scene5ACommandPlaceTimed(void);
Exit Scene5ACommandPlaceTitleObjectA(void);
void abi_Scene5ACommandPlaceTitleObjectA(void);
Exit Scene5ACommandPlaceTitleObjectB(void);
void abi_Scene5ACommandPlaceTitleObjectB(void);
Exit Scene5ACommandLoadBanks(void);
void abi_Scene5ACommandLoadBanks(void);
ObjExit Scene5ACycleLists(uint8_t object);
ObjExit Scene5AShake(uint8_t object);
ObjExit Scene5ASlideThenList(uint8_t object);
ObjExit Scene5APlayListOnce(uint8_t object);
ObjExit Scene5ASlide(uint8_t object);
ObjExit Scene5AFrameList(uint8_t list, uint8_t object);
void abi_Scene5ACycleLists(void);
void abi_Scene5AShake(void);
void abi_Scene5ASlideThenList(void);
void abi_Scene5APlayListOnce(void);
void abi_Scene5ASlide(void);
void abi_Scene5AFrameList(void);
bool scene_5a_handler(uint16_t target, uint8_t object, ObjExit *out);

#endif
