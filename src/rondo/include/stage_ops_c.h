/*
 * stage_ops_c.h: the flag tests the C of stages 06 to 08 branches on. A branch of the original tests one flag as
 * the instruction before it left it in P; the C tests the same flag in cpu.p, which the ops6280.h steps and the
 * callees' exit shims keep as the CPU does.
 */
#ifndef RONDO_STAGE_OPS_C_H
#define RONDO_STAGE_OPS_C_H

#include <stdbool.h>
#include "machine.h"

static inline bool flag_z(void) { return (cpu.p & P_Z) != 0; }   /* BEQ taken; BNE when clear */
static inline bool flag_c(void) { return (cpu.p & P_C) != 0; }   /* BCS taken; BCC when clear */
static inline bool flag_n(void) { return (cpu.p & P_N) != 0; }   /* BMI taken; BPL when clear */
static inline bool flag_v(void) { return (cpu.p & P_V) != 0; }   /* BVS taken; BVC when clear */

#endif
