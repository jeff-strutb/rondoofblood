/* My stages' stage flow copies (system/stage_flow_b.c). */
#ifndef RONDO_STAGE_FLOW_B_H
#define RONDO_STAGE_FLOW_B_H

#include "stage_map.h"

/* system/stage_flow.c: StageSectionNext's body with the overlay's section order. */
StageRegs StageSectionNextAt(StageRegs r, uint16_t order);

StageRegs StageSectionNext05BD(StageRegs r);
void abi_StageSectionNext05BD(void);
StageRegs StageSectionNext071D(StageRegs r);
void abi_StageSectionNext071D(void);
StageRegs StageSectionNext087D(StageRegs r);
void abi_StageSectionNext087D(void);
StageRegs StageSectionNext09DD(StageRegs r);
void abi_StageSectionNext09DD(void);
StageRegs StageSectionNext0B3D(StageRegs r);
void abi_StageSectionNext0B3D(void);
StageRegs StageSectionNext0C9D(StageRegs r);
void abi_StageSectionNext0C9D(void);
StageRegs StageSectionNext0DFD(StageRegs r);
void abi_StageSectionNext0DFD(void);
StageRegs StageSectionNext0EAD(StageRegs r);
void abi_StageSectionNext0EAD(void);
StageRegs StageSectionNext0F5D(StageRegs r);
void abi_StageSectionNext0F5D(void);
StageRegs StageSectionNext100D(StageRegs r);
void abi_StageSectionNext100D(void);
StageRegs StageSectionNext10BD(StageRegs r);
void abi_StageSectionNext10BD(void);
StageRegs StageSectionNext116D(StageRegs r);
void abi_StageSectionNext116D(void);
StageRegs StageSectionNext12CD(StageRegs r);
void abi_StageSectionNext12CD(void);
StageRegs StageSectionNext160D(StageRegs r);
void abi_StageSectionNext160D(void);
StageRegs StageSectionNext16BD(StageRegs r);
void abi_StageSectionNext16BD(void);
StageRegs StageSectionNext18CD(StageRegs r);
void abi_StageSectionNext18CD(void);

#endif
