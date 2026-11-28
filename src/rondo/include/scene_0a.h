/* The cutscene overlay of sector $23F6 (menu/scene_0a.c). */
#ifndef RONDO_SCENE_0A_H
#define RONDO_SCENE_0A_H

#include "routines.h"
#include "front_end.h"

Exit Scene0ABegin(uint8_t kept_x, uint8_t kept_y);
void abi_Scene0ABegin(void);
ObjExit Scene0AOrbit(uint8_t object);
void abi_Scene0AOrbit(void);
ObjExit Scene0ASwoop(uint8_t object);
void abi_Scene0ASwoop(void);
ObjExit Scene0AScrollStart(uint8_t object);
void abi_Scene0AScrollStart(void);
ObjExit Scene0AScrollRight(uint8_t object);
void abi_Scene0AScrollRight(void);
ObjExit Scene0APlayListOnce(uint8_t object);
void abi_Scene0APlayListOnce(void);
ObjExit Scene0ASlideForCount(uint8_t object);
void abi_Scene0ASlideForCount(void);
ObjExit Scene0AFrameList(uint8_t list, uint8_t object);
void abi_Scene0AFrameList(void);
ObjExit Scene0ASteps(uint8_t object);
void abi_Scene0ASteps(void);
bool scene_0a_handler(uint16_t target, uint8_t object, ObjExit *out);

#endif
