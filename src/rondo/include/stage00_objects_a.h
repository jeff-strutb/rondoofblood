/* The routines of object/stage00_objects_a.c. */
#ifndef RONDO_STAGE00_OBJECTS_A_H
#define RONDO_STAGE00_OBJECTS_A_H

#include <stdint.h>

void StageGateByState(uint8_t slot);
void abi_StageGateByState(void);
void ByState025AAEFC(uint8_t slot);
void abi_ByState025AAEFC(void);
void FlyInAndAnnounce025B(uint8_t slot);
void abi_FlyInAndAnnounce025B(void);
void HoldByWorkE025B(uint8_t slot);
void abi_HoldByWorkE025B(void);
void HoldByWorkD025B(uint8_t slot);
void abi_HoldByWorkD025B(void);
void SoundWhenHurtThenByStateBADE(uint8_t slot);
void abi_SoundWhenHurtThenByStateBADE(void);
void ScriptByWaveThenByStateBECE(uint8_t slot);
void abi_ScriptByWaveThenByStateBECE(void);

#endif
