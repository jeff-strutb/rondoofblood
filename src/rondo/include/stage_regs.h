/* The registers and flags a stage routine is entered with and leaves (StageRegs), for the stage writer's files. */
#ifndef RONDO_STAGE_REGS_H
#define RONDO_STAGE_REGS_H

#include "machine.h"
#include "stage_map.h"

/* The registers and flags the routine is entered with, N and Z as the flags hold them. */
static inline StageRegs regs_entry_shim(void)
{
    StageRegs r = { cpu.a, cpu.x, cpu.y, (cpu.p & P_C) != 0, (cpu.p & P_V) != 0, 0 };

    r.nz = (cpu.p & P_Z) != 0 ? 0 : ((cpu.p & P_N) != 0 ? 0x80 : 0x01);
    return r;
}

/* A load into A: N and Z from the value. */
static inline StageRegs regs_loaded(StageRegs r, uint8_t value)
{
    r.a = value;
    r.nz = value;
    return r;
}

/* A compare with A, X or Y: C from no borrow, N and Z from the difference. */
static inline StageRegs regs_compared(StageRegs r, uint8_t reg, uint8_t value)
{
    r.c = reg >= value;
    r.nz = (uint8_t)(reg - value);
    return r;
}

/* system/stage_flow.c: StageStart (024D:02D) with the section order table at order_table, for the copies whose
 * only difference is that operand. */
StageRegs StageStartAt(StageRegs r, uint16_t order_table);

/* system/stage_flow.c: StagePauseToggle (024D:1A2) with ret the address after its JSR to StagePauseSet. */
StageRegs StagePauseToggleAt(StageRegs r, uint16_t ret);

/* system/stage_flow.c: StageScrollSpeedsOff (024D:0C5) and StageCameraStep (024D:0BF) with ret the address after
 * their JSR to MapOverlayBank7. */
StageRegs StageScrollSpeedsOffAt(StageRegs r, uint16_t ret);
StageRegs StageCameraStepAt(StageRegs r, uint16_t ret);


/* system/stage_flow.c: StageChooseClearKind (024D:260) with ret the address after its JSR to StageScrollSpeedsOff
 * and scroll_off the address that JSR goes to. */
StageRegs StageChooseClearKindAt(StageRegs r, uint16_t ret, uint16_t scroll_off);


/* system/stage_flow.c: StageSubStepDeath (024D:23D) with ret the address after its JSR to StageScrollSpeedsOff and
 * scroll_off the address that JSR goes to. */
StageRegs StageSubStepDeathAt(StageRegs r, uint16_t ret, uint16_t scroll_off);


/* system/stage_flow.c: StageSubStepTally (024D:2BA) with ret the address after its JSR to StageScrollSpeedsOff and
 * scroll_off the address that JSR goes to. */
StageRegs StageSubStepTallyAt(StageRegs r, uint16_t ret, uint16_t scroll_off);


/* system/stage_flow.c: StageSubStepFadeOut (024D:2A2) with site the address of its tail JMP to StageScrollSpeedsOff and
 * scroll_off the address that JMP goes to. */
StageRegs StageSubStepFadeOutAt(StageRegs r, uint16_t site, uint16_t scroll_off);


/* system/stage_flow.c: StageSubStepFadeOutLate (024D:2AD) with site the address of its tail JMP to StageScrollSpeedsOff and
 * scroll_off the address that JMP goes to. */
StageRegs StageSubStepFadeOutLateAt(StageRegs r, uint16_t site, uint16_t scroll_off);


/* system/stage_flow.c: StageForcedWalk and StageForcedWalkB (024D:31D, 024D:340) with init_ret the address after their
 * JSR to InitStage, init the address it goes to, and pads their table of forced pads. */
StageRegs StageForcedWalkAt(StageRegs r, uint16_t init_ret, uint16_t init, uint16_t pads);


/* system/stage_flow.c: StageFallPose (024D:373) run at `at`. */
StageRegs StageFallPoseAt(StageRegs r, uint16_t at);


/* system/stage_flow.c: StageFallRightLand (024D:380) run at `at`. */
StageRegs StageFallRightLandAt(StageRegs r, uint16_t at);


/* system/stage_flow.c: StageFallRightDone (024D:39C) with init_ret the address after its JSR to InitStage and init the
 * address it goes to. */
StageRegs StageFallRightDoneAt(StageRegs r, uint16_t init_ret, uint16_t init);


/* system/stage_flow.c: StageFallLeftDone (024D:3EC) with init_ret the address after its JSR to InitStage and init the
 * address it goes to. */
StageRegs StageFallLeftDoneAt(StageRegs r, uint16_t init_ret, uint16_t init);

#endif
