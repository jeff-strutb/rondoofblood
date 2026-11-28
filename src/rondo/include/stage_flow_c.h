/* The routines of system/stage_flow_c.c. */
#ifndef RONDO_STAGE_FLOW_C_H
#define RONDO_STAGE_FLOW_C_H

#include <stdint.h>

void StageEntranceStep(void);
void abi_StageEntranceStep(void);
void StageEntranceByKind(void);
void abi_StageEntranceByKind(void);
void StageClearedByKind(void);
void abi_StageClearedByKind(void);
void StageClearedWalkByStep(void);
void abi_StageClearedWalkByStep(void);
void StageClearedWaitEmpty(void);
void abi_StageClearedWaitEmpty(void);
void StageClearedWalkToStop(void);
void abi_StageClearedWalkToStop(void);
void StageClearedNextRoom(void);
void abi_StageClearedNextRoom(void);
void StageClearedFinishByStep(void);
void abi_StageClearedFinishByStep(void);
void StageClearedScrollToEnd(void);
void abi_StageClearedScrollToEnd(void);
void StageClearedWaitObject22(void);
void abi_StageClearedWaitObject22(void);
void StageClearedWalkOnScreen(void);
void abi_StageClearedWalkOnScreen(void);
void StageNextSectionAfterWalk(void);
void abi_StageNextSectionAfterWalk(void);
void StageClearedDoorByStep(void);
void abi_StageClearedDoorByStep(void);
void StageClearedWalkOut(void);
void abi_StageClearedWalkOut(void);
void StageClearedLeaveScene(void);
void abi_StageClearedLeaveScene(void);
void StageDeathByStep(void);
void abi_StageDeathByStep(void);
void StageDeathLeapStart(void);
void abi_StageDeathLeapStart(void);
void StageNextDeathStep(void);
void abi_StageNextDeathStep(void);
void StageDeathFall(void);
void abi_StageDeathFall(void);

#endif
