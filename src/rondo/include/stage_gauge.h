/* The stage gauge's bodies (object/stage_gauge.c), for the builds of the stage overlay that hold them elsewhere. */
#ifndef RONDO_STAGE_GAUGE_H
#define RONDO_STAGE_GAUGE_H

#include "stage_map.h"

/* What differs between the builds holding the same gauge: the VRAM routines it draws through, and how far its code
 * sits from the loaded overlays' ($8C83 to $8D63), every return address and jump target inside it moved alike. */
typedef struct {
    void (*point)(uint8_t lo, uint8_t hi);
    uint8_t (*clear_bits)(uint8_t lo_mask, uint8_t hi_mask);
    uint8_t (*set_bits)(uint8_t lo_mask, uint8_t hi_mask);
    uint16_t moved;
} GaugeVram;

StageExit SetGaugeAddressAt(const GaugeVram *vram);
StageExit GaugeStepUpAt(const GaugeVram *vram);
StageExit GaugeStepAt(const GaugeVram *vram);
void GaugeTimerCheckAt(uint16_t moved_by);
/* The 024E program's gauge frame, and its entry at the frame test ($8C74). */
StageExit RunStageGaugeB(void);
void abi_RunStageGaugeB(void);
StageExit RunStageGaugeFromFrameB(void);
void abi_RunStageGaugeFromFrameB(void);

#endif
