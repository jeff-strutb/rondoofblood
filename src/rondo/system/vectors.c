/*
 * The kernel's jump table: overlays call the kernel through these, so that the kernel
 * can change under them. Each is one JMP, and its registers and flags are its target's.
 */
#include "machine.h"
void abi_KernelStart(void);
void abi_ResetGameState(void);
void abi_ClearScreenState(void);
void abi_StartListChannel0Kind2(void);
void abi_StartListChannelKind2(void);
void abi_StartScriptSlot0Kind1(void);
void abi_StartScriptSlotKind1(void);
void abi_StartScriptSlotKind4(void);
void abi_SoundCommand(void);
void abi_SoundStopAll(void);
void abi_SelectVramWriteAddress(void);
void abi_SelectVramReadAddress(void);
void abi_SelectVramData(void);
void abi_SelectVdcControl(void);
void abi_ClearVdcControlHigh(void);
void abi_StartScriptSlot0Kind2(void);
void abi_StartScriptSlotKind2(void);
void abi_StartScriptSlot0Kind3(void);
void abi_StartScriptSlotKind3(void);
void abi_StartListChannelKind3(void);
void abi_ClearListChannels(void);
void abi_StartSoleScriptSlot(void);
void abi_ApplyVelocity(void);
void abi_SetVelocityFromAngle(void);
void abi_AngleToPlayer(void);
void abi_AngleFromDelta(void);
void abi_NextRandom(void);
void abi_ClearObject(void);
void abi_ClearEnemySlots(void);
void abi_ClearObjectExceptPlace(void);
void abi_ClearObjectInY(void);
void abi_ApplyVelocityX(void);
void abi_ApplyVelocityY(void);
void abi_ScaleBySine(void);
void abi_ScaleByCosine(void);
void abi_TurnAngleToward(void);
void abi_RecordObjectEvent(void);
void abi_RecordEvent(void);
void abi_BitOfIndex(void);
void abi_ClearScriptSlot(void);
void abi_CdPlayAgain(void);
void abi_NegateVelocityX(void);
void abi_NegateVelocityY(void);
void abi_FadeAndStopSound(void);
void abi_StopSound(void);
void abi_AddScoreByte(void);
void abi_AddScoreWord(void);
void abi_AdpcmLoad(void);
void abi_StreamFullWalk(void);
void abi_StreamScrollStep(void);
void abi_AngleToPlayerOffset(void);
void abi_AngleToPoint(void);
void abi_StartObjectAtPlace(void);
void abi_CopyObjectPlace(void);
void abi_CopyObjectPlaceBack(void);
void abi_StepObjectFrames(void);
void abi_ProbeTerrain(void);
void abi_DistanceXToPlayer(void);
void abi_DistanceYToPlayer(void);
void abi_SideOfPlayer(void);
void abi_MoveAndLand(void);
void abi_StopOnLanding(void);
void abi_StopOnLandingFrontColumn(void);
void abi_FindFreeSlotFrom30(void);
void abi_FindFreeSlotFrom20(void);
void abi_FindFreeSlotFrom50(void);
void abi_DrawTileScript(void);
void abi_ApplyGravity(void);
void abi_ApplyGravityUnlimited(void);
void abi_LoadObjectRecord(void);
void abi_PlayerDistanceSum(void);
void abi_PlayerAndCameraFrame(void);
void abi_PlayerWallLeft(void);
void abi_PlayerWallRight(void);
void abi_CdPlayRepeat(void);
void abi_CdPlayOnce(void);
void abi_BackupRead(void);
void abi_BackupWrite(void);
void abi_BackupFormat(void);
void abi_AdpcmPlayFromDisc(void);
void abi_StopOnLandingNoMove(void);
void abi_SoundIfOnFirstScreen(void);

/*
 * @routine 0043:000 VecKernelStart
 * @run 69:4000
 * WHAT IT DOES: Goes to KernelStart.
 * HOW: One jump.
 * Entry, exit: as KernelStart.
 */
void abi_VecKernelStart(void)
{
    abi_KernelStart();
}

/*
 * @routine 0043:006 VecResetGameState
 * @run 69:4006
 * WHAT IT DOES: Goes to ResetGameState.
 * HOW: One jump.
 * Entry, exit: as ResetGameState.
 */
void abi_VecResetGameState(void)
{
    abi_ResetGameState();
}

/*
 * @routine 0043:009 VecClearScreenState
 * @run 69:4009
 * WHAT IT DOES: Goes to ClearScreenState.
 * HOW: One jump.
 * Entry, exit: as ClearScreenState.
 */
void abi_VecClearScreenState(void)
{
    abi_ClearScreenState();
}

/*
 * @routine 0043:00C VecStartListChannel0Kind2
 * @run 69:400C
 * WHAT IT DOES: Goes to StartListChannel0Kind2.
 * HOW: One jump.
 * Entry, exit: as StartListChannel0Kind2.
 */
void abi_VecStartListChannel0Kind2(void)
{
    abi_StartListChannel0Kind2();
}

/*
 * @routine 0043:00F VecStartListChannelKind2
 * @run 69:400F
 * WHAT IT DOES: Goes to StartListChannelKind2.
 * HOW: One jump.
 * Entry, exit: as StartListChannelKind2.
 */
void abi_VecStartListChannelKind2(void)
{
    abi_StartListChannelKind2();
}

/*
 * @routine 0043:012 VecStartScriptSlot0Kind1
 * @run 69:4012
 * WHAT IT DOES: Goes to StartScriptSlot0Kind1.
 * HOW: One jump.
 * Entry, exit: as StartScriptSlot0Kind1.
 */
void abi_VecStartScriptSlot0Kind1(void)
{
    abi_StartScriptSlot0Kind1();
}

/*
 * @routine 0043:015 VecStartScriptSlotKind1
 * @run 69:4015
 * WHAT IT DOES: Goes to StartScriptSlotKind1.
 * HOW: One jump.
 * Entry, exit: as StartScriptSlotKind1.
 */
void abi_VecStartScriptSlotKind1(void)
{
    abi_StartScriptSlotKind1();
}

/*
 * @routine 0043:018 VecStartScriptSlotKind4
 * @run 69:4018
 * WHAT IT DOES: Goes to StartScriptSlotKind4.
 * HOW: One jump.
 * Entry, exit: as StartScriptSlotKind4.
 */
void abi_VecStartScriptSlotKind4(void)
{
    abi_StartScriptSlotKind4();
}

/*
 * @routine 0043:01B VecSoundCommand
 * @run 69:401B
 * WHAT IT DOES: Goes to SoundCommand.
 * HOW: One jump.
 * Entry, exit: as SoundCommand.
 */
void abi_VecSoundCommand(void)
{
    abi_SoundCommand();
}

/*
 * @routine 0043:01E VecSoundStopAll
 * @run 69:401E
 * WHAT IT DOES: Goes to SoundStopAll.
 * HOW: One jump.
 * Entry, exit: as SoundStopAll.
 */
void abi_VecSoundStopAll(void)
{
    abi_SoundStopAll();
}

/*
 * @routine 0043:021 VecSelectVramWriteAddress
 * @run 69:4021
 * WHAT IT DOES: Goes to SelectVramWriteAddress.
 * HOW: One jump.
 * Entry, exit: as SelectVramWriteAddress.
 */
void abi_VecSelectVramWriteAddress(void)
{
    abi_SelectVramWriteAddress();
}

/*
 * @routine 0043:024 VecSelectVramReadAddress
 * @run 69:4024
 * WHAT IT DOES: Goes to SelectVramReadAddress.
 * HOW: One jump.
 * Entry, exit: as SelectVramReadAddress.
 */
void abi_VecSelectVramReadAddress(void)
{
    abi_SelectVramReadAddress();
}

/*
 * @routine 0043:027 VecSelectVramData
 * @run 69:4027
 * WHAT IT DOES: Goes to SelectVramData.
 * HOW: One jump.
 * Entry, exit: as SelectVramData.
 */
void abi_VecSelectVramData(void)
{
    abi_SelectVramData();
}

/*
 * @routine 0043:02A VecSelectVdcControl
 * @run 69:402A
 * WHAT IT DOES: Goes to SelectVdcControl.
 * HOW: One jump.
 * Entry, exit: as SelectVdcControl.
 */
void abi_VecSelectVdcControl(void)
{
    abi_SelectVdcControl();
}

/*
 * @routine 0043:02D VecClearVdcControlHigh
 * @run 69:402D
 * WHAT IT DOES: Goes to ClearVdcControlHigh.
 * HOW: One jump.
 * Entry, exit: as ClearVdcControlHigh.
 */
void abi_VecClearVdcControlHigh(void)
{
    abi_ClearVdcControlHigh();
}

/*
 * @routine 0043:030 VecStartScriptSlot0Kind2
 * @run 69:4030
 * WHAT IT DOES: Goes to StartScriptSlot0Kind2.
 * HOW: One jump.
 * Entry, exit: as StartScriptSlot0Kind2.
 */
void abi_VecStartScriptSlot0Kind2(void)
{
    abi_StartScriptSlot0Kind2();
}

/*
 * @routine 0043:033 VecStartScriptSlotKind2
 * @run 69:4033
 * WHAT IT DOES: Goes to StartScriptSlotKind2.
 * HOW: One jump.
 * Entry, exit: as StartScriptSlotKind2.
 */
void abi_VecStartScriptSlotKind2(void)
{
    abi_StartScriptSlotKind2();
}

/*
 * @routine 0043:036 VecStartScriptSlot0Kind3
 * @run 69:4036
 * WHAT IT DOES: Goes to StartScriptSlot0Kind3.
 * HOW: One jump.
 * Entry, exit: as StartScriptSlot0Kind3.
 */
void abi_VecStartScriptSlot0Kind3(void)
{
    abi_StartScriptSlot0Kind3();
}

/*
 * @routine 0043:039 VecStartScriptSlotKind3
 * @run 69:4039
 * WHAT IT DOES: Goes to StartScriptSlotKind3.
 * HOW: One jump.
 * Entry, exit: as StartScriptSlotKind3.
 */
void abi_VecStartScriptSlotKind3(void)
{
    abi_StartScriptSlotKind3();
}

/*
 * @routine 0043:03C VecStartListChannelKind3
 * @run 69:403C
 * WHAT IT DOES: Goes to StartListChannelKind3.
 * HOW: One jump.
 * Entry, exit: as StartListChannelKind3.
 */
void abi_VecStartListChannelKind3(void)
{
    abi_StartListChannelKind3();
}

/*
 * @routine 0043:03F VecClearListChannels
 * @run 69:403F
 * WHAT IT DOES: Goes to ClearListChannels.
 * HOW: One jump.
 * Entry, exit: as ClearListChannels.
 */
void abi_VecClearListChannels(void)
{
    abi_ClearListChannels();
}

/*
 * @routine 0043:042 VecStartSoleScriptSlot
 * @run 69:4042
 * WHAT IT DOES: Goes to StartSoleScriptSlot.
 * HOW: One jump.
 * Entry, exit: as StartSoleScriptSlot.
 */
void abi_VecStartSoleScriptSlot(void)
{
    abi_StartSoleScriptSlot();
}

/*
 * @routine 0043:045 VecApplyVelocity
 * @run 69:4045
 * WHAT IT DOES: Goes to ApplyVelocity.
 * HOW: One jump.
 * Entry, exit: as ApplyVelocity.
 */
void abi_VecApplyVelocity(void)
{
    abi_ApplyVelocity();
}

/*
 * @routine 0043:048 VecSetVelocityFromAngle
 * @run 69:4048
 * WHAT IT DOES: Goes to SetVelocityFromAngle.
 * HOW: One jump.
 * Entry, exit: as SetVelocityFromAngle.
 */
void abi_VecSetVelocityFromAngle(void)
{
    abi_SetVelocityFromAngle();
}

/*
 * @routine 0043:04B VecAngleToPlayer
 * @run 69:404B
 * WHAT IT DOES: Goes to AngleToPlayer.
 * HOW: One jump.
 * Entry, exit: as AngleToPlayer.
 */
void abi_VecAngleToPlayer(void)
{
    abi_AngleToPlayer();
}

/*
 * @routine 0043:04E VecAngleFromDelta
 * @run 69:404E
 * WHAT IT DOES: Goes to AngleFromDelta.
 * HOW: One jump.
 * Entry, exit: as AngleFromDelta.
 */
void abi_VecAngleFromDelta(void)
{
    abi_AngleFromDelta();
}

/*
 * @routine 0043:051 VecNextRandom
 * @run 69:4051
 * WHAT IT DOES: Goes to NextRandom.
 * HOW: One jump.
 * Entry, exit: as NextRandom.
 */
void abi_VecNextRandom(void)
{
    abi_NextRandom();
}

/*
 * @routine 0043:054 VecClearObject
 * @run 69:4054
 * WHAT IT DOES: Goes to ClearObject.
 * HOW: One jump.
 * Entry, exit: as ClearObject.
 */
void abi_VecClearObject(void)
{
    abi_ClearObject();
}

/*
 * @routine 0043:057 VecClearEnemySlots
 * @run 69:4057
 * WHAT IT DOES: Goes to ClearEnemySlots.
 * HOW: One jump.
 * Entry, exit: as ClearEnemySlots.
 */
void abi_VecClearEnemySlots(void)
{
    abi_ClearEnemySlots();
}

/*
 * @routine 0043:05A VecClearObjectExceptPlace
 * @run 69:405A
 * WHAT IT DOES: Goes to ClearObjectExceptPlace.
 * HOW: One jump.
 * Entry, exit: as ClearObjectExceptPlace.
 */
void abi_VecClearObjectExceptPlace(void)
{
    abi_ClearObjectExceptPlace();
}

/*
 * @routine 0043:05D VecClearObjectInY
 * @run 69:405D
 * WHAT IT DOES: Goes to ClearObjectInY.
 * HOW: One jump.
 * Entry, exit: as ClearObjectInY.
 */
void abi_VecClearObjectInY(void)
{
    abi_ClearObjectInY();
}

/*
 * @routine 0043:060 VecApplyVelocityX
 * @run 69:4060
 * WHAT IT DOES: Goes to ApplyVelocityX.
 * HOW: One jump.
 * Entry, exit: as ApplyVelocityX.
 */
void abi_VecApplyVelocityX(void)
{
    abi_ApplyVelocityX();
}

/*
 * @routine 0043:063 VecApplyVelocityY
 * @run 69:4063
 * WHAT IT DOES: Goes to ApplyVelocityY.
 * HOW: One jump.
 * Entry, exit: as ApplyVelocityY.
 */
void abi_VecApplyVelocityY(void)
{
    abi_ApplyVelocityY();
}

/*
 * @routine 0043:066 VecScaleBySine
 * @run 69:4066
 * WHAT IT DOES: Goes to ScaleBySine.
 * HOW: One jump.
 * Entry, exit: as ScaleBySine.
 */
void abi_VecScaleBySine(void)
{
    abi_ScaleBySine();
}

/*
 * @routine 0043:069 VecScaleByCosine
 * @run 69:4069
 * WHAT IT DOES: Goes to ScaleByCosine.
 * HOW: One jump.
 * Entry, exit: as ScaleByCosine.
 */
void abi_VecScaleByCosine(void)
{
    abi_ScaleByCosine();
}

/*
 * @routine 0043:06C VecTurnAngleToward
 * @run 69:406C
 * WHAT IT DOES: Goes to TurnAngleToward.
 * HOW: One jump.
 * Entry, exit: as TurnAngleToward.
 */
void abi_VecTurnAngleToward(void)
{
    abi_TurnAngleToward();
}

/*
 * @routine 0043:06F VecRecordObjectEvent
 * @run 69:406F
 * WHAT IT DOES: Goes to RecordObjectEvent.
 * HOW: One jump.
 * Entry, exit: as RecordObjectEvent.
 */
void abi_VecRecordObjectEvent(void)
{
    abi_RecordObjectEvent();
}

/*
 * @routine 0043:072 VecRecordEvent
 * @run 69:4072
 * WHAT IT DOES: Goes to RecordEvent.
 * HOW: One jump.
 * Entry, exit: as RecordEvent.
 */
void abi_VecRecordEvent(void)
{
    abi_RecordEvent();
}

/*
 * @routine 0043:075 VecBitOfIndex
 * @run 69:4075
 * WHAT IT DOES: Goes to BitOfIndex.
 * HOW: One jump.
 * Entry, exit: as BitOfIndex.
 */
void abi_VecBitOfIndex(void)
{
    abi_BitOfIndex();
}

/*
 * @routine 0043:078 VecClearScriptSlot
 * @run 69:4078
 * WHAT IT DOES: Goes to ClearScriptSlot.
 * HOW: One jump.
 * Entry, exit: as ClearScriptSlot.
 */
void abi_VecClearScriptSlot(void)
{
    abi_ClearScriptSlot();
}

/*
 * @routine 0043:07B VecCdPlayAgain
 * @run 69:407B
 * WHAT IT DOES: Goes to CdPlayAgain.
 * HOW: One jump.
 * Entry, exit: as CdPlayAgain.
 */
void abi_VecCdPlayAgain(void)
{
    abi_CdPlayAgain();
}

/*
 * @routine 0043:07E VecNegateVelocityX
 * @run 69:407E
 * WHAT IT DOES: Goes to NegateVelocityX.
 * HOW: One jump.
 * Entry, exit: as NegateVelocityX.
 */
void abi_VecNegateVelocityX(void)
{
    abi_NegateVelocityX();
}

/*
 * @routine 0043:081 VecNegateVelocityY
 * @run 69:4081
 * WHAT IT DOES: Goes to NegateVelocityY.
 * HOW: One jump.
 * Entry, exit: as NegateVelocityY.
 */
void abi_VecNegateVelocityY(void)
{
    abi_NegateVelocityY();
}

/*
 * @routine 0043:084 VecFadeAndStopSound
 * @run 69:4084
 * WHAT IT DOES: Goes to FadeAndStopSound.
 * HOW: One jump.
 * Entry, exit: as FadeAndStopSound.
 */
void abi_VecFadeAndStopSound(void)
{
    abi_FadeAndStopSound();
}

/*
 * @routine 0043:087 VecStopSound
 * @run 69:4087
 * WHAT IT DOES: Goes to StopSound.
 * HOW: One jump.
 * Entry, exit: as StopSound.
 */
void abi_VecStopSound(void)
{
    abi_StopSound();
}

/*
 * @routine 0043:08A VecAddScoreByte
 * @run 69:408A
 * WHAT IT DOES: Goes to AddScoreByte.
 * HOW: One jump.
 * Entry, exit: as AddScoreByte.
 */
void abi_VecAddScoreByte(void)
{
    abi_AddScoreByte();
}

/*
 * @routine 0043:08D VecAddScoreWord
 * @run 69:408D
 * WHAT IT DOES: Goes to AddScoreWord.
 * HOW: One jump.
 * Entry, exit: as AddScoreWord.
 */
void abi_VecAddScoreWord(void)
{
    abi_AddScoreWord();
}

/*
 * @routine 0043:090 VecAdpcmLoad
 * @run 69:4090
 * WHAT IT DOES: Goes to AdpcmLoad.
 * HOW: One jump.
 * Entry, exit: as AdpcmLoad.
 */
void abi_VecAdpcmLoad(void)
{
    abi_AdpcmLoad();
}

/*
 * @routine 0043:093 VecStreamFullWalk
 * @run 69:4093
 * WHAT IT DOES: Goes to StreamFullWalk.
 * HOW: One jump.
 * Entry, exit: as StreamFullWalk.
 */
void abi_VecStreamFullWalk(void)
{
    abi_StreamFullWalk();
}

/*
 * @routine 0043:096 VecStreamScrollStep
 * @run 69:4096
 * WHAT IT DOES: Goes to StreamScrollStep.
 * HOW: One jump.
 * Entry, exit: as StreamScrollStep.
 */
void abi_VecStreamScrollStep(void)
{
    abi_StreamScrollStep();
}

/*
 * @routine 0043:099 VecAngleToPlayerOffset
 * @run 69:4099
 * WHAT IT DOES: Goes to AngleToPlayerOffset.
 * HOW: One jump.
 * Entry, exit: as AngleToPlayerOffset.
 */
void abi_VecAngleToPlayerOffset(void)
{
    abi_AngleToPlayerOffset();
}

/*
 * @routine 0043:09C VecAngleToPoint
 * @run 69:409C
 * WHAT IT DOES: Goes to AngleToPoint.
 * HOW: One jump.
 * Entry, exit: as AngleToPoint.
 */
void abi_VecAngleToPoint(void)
{
    abi_AngleToPoint();
}

/*
 * @routine 0043:09F VecStartObjectAtPlace
 * @run 69:409F
 * WHAT IT DOES: Goes to StartObjectAtPlace.
 * HOW: One jump.
 * Entry, exit: as StartObjectAtPlace.
 */
void abi_VecStartObjectAtPlace(void)
{
    abi_StartObjectAtPlace();
}

/*
 * @routine 0043:0A2 VecCopyObjectPlace
 * @run 69:40A2
 * WHAT IT DOES: Goes to CopyObjectPlace.
 * HOW: One jump.
 * Entry, exit: as CopyObjectPlace.
 */
void abi_VecCopyObjectPlace(void)
{
    abi_CopyObjectPlace();
}

/*
 * @routine 0043:0A5 VecCopyObjectPlaceBack
 * @run 69:40A5
 * WHAT IT DOES: Goes to CopyObjectPlaceBack.
 * HOW: One jump.
 * Entry, exit: as CopyObjectPlaceBack.
 */
void abi_VecCopyObjectPlaceBack(void)
{
    abi_CopyObjectPlaceBack();
}

/*
 * @routine 0043:0A8 VecStepObjectFrames
 * @run 69:40A8
 * WHAT IT DOES: Goes to StepObjectFrames.
 * HOW: One jump.
 * Entry, exit: as StepObjectFrames.
 */
void abi_VecStepObjectFrames(void)
{
    abi_StepObjectFrames();
}

/*
 * @routine 0043:0AB VecProbeTerrain
 * @run 69:40AB
 * WHAT IT DOES: Goes to ProbeTerrain.
 * HOW: One jump.
 * Entry, exit: as ProbeTerrain.
 */
void abi_VecProbeTerrain(void)
{
    abi_ProbeTerrain();
}

/*
 * @routine 0043:0AE VecDistanceXToPlayer
 * @run 69:40AE
 * WHAT IT DOES: Goes to DistanceXToPlayer.
 * HOW: One jump.
 * Entry, exit: as DistanceXToPlayer.
 */
void abi_VecDistanceXToPlayer(void)
{
    abi_DistanceXToPlayer();
}

/*
 * @routine 0043:0B1 VecDistanceYToPlayer
 * @run 69:40B1
 * WHAT IT DOES: Goes to DistanceYToPlayer.
 * HOW: One jump.
 * Entry, exit: as DistanceYToPlayer.
 */
void abi_VecDistanceYToPlayer(void)
{
    abi_DistanceYToPlayer();
}

/*
 * @routine 0043:0B4 VecSideOfPlayer
 * @run 69:40B4
 * WHAT IT DOES: Goes to SideOfPlayer.
 * HOW: One jump.
 * Entry, exit: as SideOfPlayer.
 */
void abi_VecSideOfPlayer(void)
{
    abi_SideOfPlayer();
}

/*
 * @routine 0043:0B7 VecMoveAndLand
 * @run 69:40B7
 * WHAT IT DOES: Goes to MoveAndLand.
 * HOW: One jump.
 * Entry, exit: as MoveAndLand.
 */
void abi_VecMoveAndLand(void)
{
    abi_MoveAndLand();
}

/*
 * @routine 0043:0BA VecStopOnLanding
 * @run 69:40BA
 * WHAT IT DOES: Goes to StopOnLanding.
 * HOW: One jump.
 * Entry, exit: as StopOnLanding.
 */
void abi_VecStopOnLanding(void)
{
    abi_StopOnLanding();
}

/*
 * @routine 0043:0BD VecStopOnLandingFrontColumn
 * @run 69:40BD
 * WHAT IT DOES: Goes to StopOnLandingFrontColumn.
 * HOW: One jump.
 * Entry, exit: as StopOnLandingFrontColumn.
 */
void abi_VecStopOnLandingFrontColumn(void)
{
    abi_StopOnLandingFrontColumn();
}

/*
 * @routine 0043:0C0 VecFindFreeSlotFrom30
 * @run 69:40C0
 * WHAT IT DOES: Goes to FindFreeSlotFrom30.
 * HOW: One jump.
 * Entry, exit: as FindFreeSlotFrom30.
 */
void abi_VecFindFreeSlotFrom30(void)
{
    abi_FindFreeSlotFrom30();
}

/*
 * @routine 0043:0C3 VecFindFreeSlotFrom20
 * @run 69:40C3
 * WHAT IT DOES: Goes to FindFreeSlotFrom20.
 * HOW: One jump.
 * Entry, exit: as FindFreeSlotFrom20.
 */
void abi_VecFindFreeSlotFrom20(void)
{
    abi_FindFreeSlotFrom20();
}

/*
 * @routine 0043:0C6 VecFindFreeSlotFrom50
 * @run 69:40C6
 * WHAT IT DOES: Goes to FindFreeSlotFrom50.
 * HOW: One jump.
 * Entry, exit: as FindFreeSlotFrom50.
 */
void abi_VecFindFreeSlotFrom50(void)
{
    abi_FindFreeSlotFrom50();
}

/*
 * @routine 0043:0C9 VecDrawTileScript
 * @run 69:40C9
 * WHAT IT DOES: Goes to DrawTileScript.
 * HOW: One jump.
 * Entry, exit: as DrawTileScript.
 */
void abi_VecDrawTileScript(void)
{
    abi_DrawTileScript();
}

/*
 * @routine 0043:0CC VecApplyGravity
 * @run 69:40CC
 * WHAT IT DOES: Goes to ApplyGravity.
 * HOW: One jump.
 * Entry, exit: as ApplyGravity.
 */
void abi_VecApplyGravity(void)
{
    abi_ApplyGravity();
}

/*
 * @routine 0043:0CF VecApplyGravityUnlimited
 * @run 69:40CF
 * WHAT IT DOES: Goes to ApplyGravityUnlimited.
 * HOW: One jump.
 * Entry, exit: as ApplyGravityUnlimited.
 */
void abi_VecApplyGravityUnlimited(void)
{
    abi_ApplyGravityUnlimited();
}

/*
 * @routine 0043:0D2 VecLoadObjectRecord
 * @run 69:40D2
 * WHAT IT DOES: Goes to LoadObjectRecord.
 * HOW: One jump.
 * Entry, exit: as LoadObjectRecord.
 */
void abi_VecLoadObjectRecord(void)
{
    abi_LoadObjectRecord();
}

/*
 * @routine 0043:0D5 VecPlayerDistanceSum
 * @run 69:40D5
 * WHAT IT DOES: Goes to PlayerDistanceSum.
 * HOW: One jump.
 * Entry, exit: as PlayerDistanceSum.
 */
void abi_VecPlayerDistanceSum(void)
{
    abi_PlayerDistanceSum();
}

/*
 * @routine 0043:0D8 VecPlayerAndCameraFrame
 * @run 69:40D8
 * WHAT IT DOES: Goes to PlayerAndCameraFrame.
 * HOW: One jump.
 * Entry, exit: as PlayerAndCameraFrame.
 */
void abi_VecPlayerAndCameraFrame(void)
{
    abi_PlayerAndCameraFrame();
}

/*
 * @routine 0043:0DB VecPlayerWallLeft
 * @run 69:40DB
 * WHAT IT DOES: Goes to PlayerWallLeft.
 * HOW: One jump.
 * Entry, exit: as PlayerWallLeft.
 */
void abi_VecPlayerWallLeft(void)
{
    abi_PlayerWallLeft();
}

/*
 * @routine 0043:0DE VecPlayerWallRight
 * @run 69:40DE
 * WHAT IT DOES: Goes to PlayerWallRight.
 * HOW: One jump.
 * Entry, exit: as PlayerWallRight.
 */
void abi_VecPlayerWallRight(void)
{
    abi_PlayerWallRight();
}

/*
 * @routine 0043:0E1 VecCdPlayRepeat
 * @run 69:40E1
 * WHAT IT DOES: Goes to CdPlayRepeat.
 * HOW: One jump.
 * Entry, exit: as CdPlayRepeat.
 */
void abi_VecCdPlayRepeat(void)
{
    abi_CdPlayRepeat();
}

/*
 * @routine 0043:0E4 VecCdPlayOnce
 * @run 69:40E4
 * WHAT IT DOES: Goes to CdPlayOnce.
 * HOW: One jump.
 * Entry, exit: as CdPlayOnce.
 */
void abi_VecCdPlayOnce(void)
{
    abi_CdPlayOnce();
}

/*
 * @routine 0043:0E7 VecBackupRead
 * @run 69:40E7
 * WHAT IT DOES: Goes to BackupRead.
 * HOW: One jump.
 * Entry, exit: as BackupRead.
 */
void abi_VecBackupRead(void)
{
    abi_BackupRead();
}

/*
 * @routine 0043:0EA VecBackupWrite
 * @run 69:40EA
 * WHAT IT DOES: Goes to BackupWrite.
 * HOW: One jump.
 * Entry, exit: as BackupWrite.
 */
void abi_VecBackupWrite(void)
{
    abi_BackupWrite();
}

/*
 * @routine 0043:0ED VecBackupFormat
 * @run 69:40ED
 * WHAT IT DOES: Goes to BackupFormat.
 * HOW: One jump.
 * Entry, exit: as BackupFormat.
 */
void abi_VecBackupFormat(void)
{
    abi_BackupFormat();
}

/*
 * @routine 0043:0F0 VecAdpcmPlayFromDisc
 * @run 69:40F0
 * WHAT IT DOES: Goes to AdpcmPlayFromDisc.
 * HOW: One jump.
 * Entry, exit: as AdpcmPlayFromDisc.
 */
void abi_VecAdpcmPlayFromDisc(void)
{
    abi_AdpcmPlayFromDisc();
}

/*
 * @routine 0043:0F3 VecStopOnLandingNoMove
 * @run 69:40F3
 * WHAT IT DOES: Goes to StopOnLandingNoMove.
 * HOW: One jump.
 * Entry, exit: as StopOnLandingNoMove.
 */
void abi_VecStopOnLandingNoMove(void)
{
    abi_StopOnLandingNoMove();
}

/*
 * @routine 0043:0F6 VecSoundIfOnFirstScreen
 * @run 69:40F6
 * WHAT IT DOES: Goes to SoundIfOnFirstScreen.
 * HOW: One jump.
 * Entry, exit: as SoundIfOnFirstScreen.
 */
void abi_VecSoundIfOnFirstScreen(void)
{
    abi_SoundIfOnFirstScreen();
}
