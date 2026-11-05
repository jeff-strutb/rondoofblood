/* Routines called across files. */
#ifndef RONDO_ROUTINES_H
#define RONDO_ROUTINES_H

#include "machine.h"

Alu ApplyVelocity(uint8_t object);
void ClearObject(uint8_t object);
void SelectVdcRegister(uint8_t reg);
void SelectVramWriteAddress(void);
void SelectVramData(void);
void ClearVdcControlHigh(void);
void SelectVdcControl(void);
uint8_t WriteScreenTiming(uint8_t index);
uint8_t WriteBandScroll(uint8_t band);
void ResetDisplayControl(void);
uint8_t SoundFrameTick(void);
Alu NextRandom(void);
bool RasterStart(uint8_t *effect, uint8_t *last);
void RasterInterrupt(void);
uint8_t BitOfIndex(uint8_t index, uint8_t *byte);
uint8_t RecordEvent(uint8_t event, uint8_t *byte);
void StartScriptSlotKind1(uint8_t script, uint8_t slot);
void StartListChannelKind2(uint8_t list, uint8_t channel);
void StartListChannelKind3(uint8_t list, uint8_t channel);

#endif
