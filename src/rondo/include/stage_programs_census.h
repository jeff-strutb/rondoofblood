/* The unloaded stage programs' routines (system/stage_programs_census.c). */
#ifndef RONDO_STAGE_PROGRAMS_CENSUS_H
#define RONDO_STAGE_PROGRAMS_CENSUS_H

#include "stage_map.h"
#include "enemy_objects.h"

ObjArm ResetStageVariables026D(void);
void abi_ResetStageVariables026D(void);
StageRunsEnd ExpandStageVramRuns026E(void);
void abi_ExpandStageVramRuns026E(void);
StageListEnd ExpandStageLists026F(void);
void abi_ExpandStageLists026F(void);
ObjArm ResetStageVariables031D(void);
void abi_ResetStageVariables031D(void);
StageRunsEnd ExpandStageVramRuns031E(void);
void abi_ExpandStageVramRuns031E(void);
StageListEnd ExpandStageLists031F(void);
void abi_ExpandStageLists031F(void);
ObjArm ResetStageVariables043D(void);
void abi_ResetStageVariables043D(void);
StageRunsEnd ExpandStageVramRuns043E(void);
void abi_ExpandStageVramRuns043E(void);
StageListEnd ExpandStageLists043F(void);
void abi_ExpandStageLists043F(void);
ObjArm ResetStageVariables064D(void);
void abi_ResetStageVariables064D(void);
StageRunsEnd ExpandStageVramRuns064E(void);
void abi_ExpandStageVramRuns064E(void);
StageListEnd ExpandStageLists064F(void);
void abi_ExpandStageLists064F(void);
ObjArm ResetStageVariables10DD(void);
void abi_ResetStageVariables10DD(void);
StageRunsEnd ExpandStageVramRuns10DE(void);
void abi_ExpandStageVramRuns10DE(void);
StageListEnd ExpandStageLists10DF(void);
void abi_ExpandStageLists10DF(void);
ObjArm ResetStageVariables122D(void);
void abi_ResetStageVariables122D(void);
StageRunsEnd ExpandStageVramRuns122E(void);
void abi_ExpandStageVramRuns122E(void);
StageListEnd ExpandStageLists122F(void);
void abi_ExpandStageLists122F(void);
ObjArm ResetStageVariables1A2D(void);
void abi_ResetStageVariables1A2D(void);
StageRunsEnd ExpandStageVramRuns1A2E(void);
void abi_ExpandStageVramRuns1A2E(void);
StageListEnd ExpandStageLists1A2F(void);
void abi_ExpandStageLists1A2F(void);
ObjArm ResetStageVariables1B8D(void);
void abi_ResetStageVariables1B8D(void);
StageRunsEnd ExpandStageVramRuns1B8E(void);
void abi_ExpandStageVramRuns1B8E(void);
StageListEnd ExpandStageLists1B8F(void);
void abi_ExpandStageLists1B8F(void);
ObjArm ResetStageVariables1CED(void);
void abi_ResetStageVariables1CED(void);
StageRunsEnd ExpandStageVramRuns1CEE(void);
void abi_ExpandStageVramRuns1CEE(void);
StageListEnd ExpandStageLists1CEF(void);
void abi_ExpandStageLists1CEF(void);

void SetVramAddresses026F(uint8_t lo, uint8_t hi);
void abi_SetVramAddresses026F(void);
void SetVramAddresses031F(uint8_t lo, uint8_t hi);
void abi_SetVramAddresses031F(void);
StageRegs StageSectionNext026D(StageRegs r);
void abi_StageSectionNext026D(void);
StageRegs StageSectionNext031D(StageRegs r);
void abi_StageSectionNext031D(void);
StageRegs StageSectionNext043D(StageRegs r);
void abi_StageSectionNext043D(void);
StageRegs StageSectionNext064D(StageRegs r);
void abi_StageSectionNext064D(void);
StageRegs StageSectionNext10DD(StageRegs r);
void abi_StageSectionNext10DD(void);
StageRegs StageSectionNext122D(StageRegs r);
void abi_StageSectionNext122D(void);
StageRegs StageSectionNext1A2D(StageRegs r);
void abi_StageSectionNext1A2D(void);
StageRegs StageSectionNext1B8D(StageRegs r);
void abi_StageSectionNext1B8D(void);
StageRegs StageSectionNext1CED(StageRegs r);
void abi_StageSectionNext1CED(void);
MaskExit MaskStageRowCopy026F(uint8_t x, uint8_t y);
void abi_MaskStageRowCopy026F(void);
MaskExit MaskStageRow026F(uint8_t x, uint8_t y);
void abi_MaskStageRow026F(void);
MaskExit MaskStageRowCopy031F(uint8_t x, uint8_t y);
void abi_MaskStageRowCopy031F(void);
MaskExit MaskStageRow031F(uint8_t x, uint8_t y);
void abi_MaskStageRow031F(void);
uint16_t SetStageTileVramAddress026E(void);
void abi_SetStageTileVramAddress026E(void);
StageExit SetHealthBarAddress026E(void);
void abi_SetHealthBarAddress026E(void);
uint16_t SetStageTileVramAddress031E(void);
void abi_SetStageTileVramAddress031E(void);
StageExit SetHealthBarAddress031E(void);
void abi_SetHealthBarAddress031E(void);

StageStartEnd StartStageLists026E(void);
void abi_StartStageLists026E(void);
StageStartEnd StartStageLists031E(void);
void abi_StartStageLists031E(void);
StageStartEnd StartStageLists043E(void);
void abi_StartStageLists043E(void);
StageStartEnd StartStageLists064E(void);
void abi_StartStageLists064E(void);
StageStartEnd StartStageLists10DE(void);
void abi_StartStageLists10DE(void);
StageStartEnd StartStageLists122E(void);
void abi_StartStageLists122E(void);
StageStartEnd StartStageLists1A2E(void);
void abi_StartStageLists1A2E(void);
StageStartEnd StartStageLists1B8E(void);
void abi_StartStageLists1B8E(void);
StageStartEnd StartStageLists1CEE(void);
void abi_StartStageLists1CEE(void);
SpawnedObject SpawnStageObject026F(uint8_t row);
void abi_SpawnStageObject026F(void);
SpawnedObject SpawnStageObject031F(uint8_t row);
void abi_SpawnStageObject031F(void);
StageSpawnEnd SpawnStageObjectsRows34026E(void);
void abi_SpawnStageObjectsRows34026E(void);
StageSpawnEnd SpawnStageObjectsRows34031E(void);
void abi_SpawnStageObjectsRows34031E(void);

/* What FillMarkedTiles026F leaves: A, and whether it filled any tile (then X 0, C 0). */
typedef struct { uint8_t a; bool filled; } TileFillEnd;
TileFillEnd FillMarkedTiles026F(uint8_t page, uint8_t high, uint8_t y);
void abi_FillMarkedTiles026F(void);

void ClearEffectSlot1531(uint8_t slot);
void abi_ClearEffectSlot1531(void);
void EffectRestoreColours1531(uint8_t slot, uint8_t y);
void abi_EffectRestoreColours1531(void);

void StepScrollAndRunObjects026E(void);
void abi_StepScrollAndRunObjects026E(void);

StageRegs StagePauseToggle026D(StageRegs r);
void abi_StagePauseToggle026D(void);
StageRegs StagePauseToggle031D(StageRegs r);
void abi_StagePauseToggle031D(void);
StageRegs StageCameraStep026D(StageRegs r);
void abi_StageCameraStep026D(void);
StageRegs StageScrollSpeedsOff026D(StageRegs r);
void abi_StageScrollSpeedsOff026D(void);

StageRegs StageChooseClearKind026D(StageRegs r);
void abi_StageChooseClearKind026D(void);

StageRegs StageSubStepDeath026D(StageRegs r);
void abi_StageSubStepDeath026D(void);

StageRegs StageSubStepTally026D(StageRegs r);
void abi_StageSubStepTally026D(void);

StageRegs StageSubStepFadeOut026D(StageRegs r);
void abi_StageSubStepFadeOut026D(void);

StageRegs StageSubStepFadeOutLate026D(StageRegs r);
void abi_StageSubStepFadeOutLate026D(void);

StageRegs StageForcedWalk026D(StageRegs r);
void abi_StageForcedWalk026D(void);

StageRegs StageForcedWalkB026D(StageRegs r);
void abi_StageForcedWalkB026D(void);

StageRegs StageFallRightLand026D(StageRegs r);
void abi_StageFallRightLand026D(void);

StageRegs StageFallPose026D(StageRegs r);
void abi_StageFallPose026D(void);

StageRegs StageFallRightDone026D(StageRegs r);
void abi_StageFallRightDone026D(void);

StageRegs StageFallLeftDone026D(StageRegs r);
void abi_StageFallLeftDone026D(void);

StageRegs StageFallLeftLand026D(StageRegs r);
void abi_StageFallLeftLand026D(void);

StageRegs StageFallRight026D(StageRegs r);
void abi_StageFallRight026D(void);

StageRegs StageFallLeft026D(StageRegs r);
void abi_StageFallLeft026D(void);

StageRegs StageRestartInPlace026D(StageRegs r);
void abi_StageRestartInPlace026D(void);

StageRegs StageShowStartFrame026D(StageRegs r);
void abi_StageShowStartFrame026D(void);

StageRegs StageClearedWaitScroll026D(StageRegs r);
void abi_StageClearedWaitScroll026D(void);

StageRegs StageClearedScrollOn026D(StageRegs r);
void abi_StageClearedScrollOn026D(void);

StageRegs StageWalkOutScroll026D(StageRegs r);
void abi_StageWalkOutScroll026D(void);

StageRegs StageStopAndStep026D(StageRegs r);
void abi_StageStopAndStep026D(void);

StageExit StageScriptByWave026E(void);
void abi_StageScriptByWave026E(void);

StageRegs BossFlashMaskCell026E(StageRegs r);
void abi_BossFlashMaskCell026E(void);

StageRegs BossFlashMaskCell031E(StageRegs r);
void abi_BossFlashMaskCell031E(void);

void abi_GaugeTimerCheckB026E(void);

StageExit GaugeStepB026E(void);
void abi_GaugeStepB026E(void);

StageExit GaugeStepUpB026E(void);
void abi_GaugeStepUpB026E(void);

StageExit SetGaugeAddressB026E(void);
void abi_SetGaugeAddressB026E(void);

void abi_GaugeTimerCheckB031E(void);

StageExit GaugeStepB031E(void);
void abi_GaugeStepB031E(void);

StageExit GaugeStepUpB031E(void);
void abi_GaugeStepUpB031E(void);

StageExit SetGaugeAddressB031E(void);
void abi_SetGaugeAddressB031E(void);

StageRegs BossGaugeFlash026E(StageRegs r);
void abi_BossGaugeFlash026E(void);

TileRowsExit BlendStageTileRows026E(uint8_t phase, uint8_t row_a, uint8_t row_b);
void abi_BlendStageTileRows026E(void);

TileRowsExit BlendStageRowPair026F(uint8_t x, uint8_t a, uint8_t y);
void abi_BlendStageRowPair026F(void);

TileRowsExit BlendStageRowPair031F(uint8_t x, uint8_t a, uint8_t y);
void abi_BlendStageRowPair031F(void);

RowChangesExit BlendStageRowChanges026F(uint8_t x, uint8_t new_byte);
void abi_BlendStageRowChanges026F(void);

RowChangesExit BlendStageRowChanges031F(uint8_t x, uint8_t new_byte);
void abi_BlendStageRowChanges031F(void);


ObjArm BlendStageStockRows026F(uint8_t slot, uint8_t y);
void abi_BlendStageStockRows026F(void);

ObjArm BlendStageStockRows031F(uint8_t slot, uint8_t y);
void abi_BlendStageStockRows031F(void);

ObjArm BlendStageScoreAddRows026F(uint8_t slot, uint8_t y);
void abi_BlendStageScoreAddRows026F(void);

ObjArm BlendStageScoreAddRows031F(uint8_t slot, uint8_t y);
void abi_BlendStageScoreAddRows031F(void);

#endif
