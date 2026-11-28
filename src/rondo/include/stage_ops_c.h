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

/* The probe request bits the landing and walking callees' exit shims take (sensing.c's PROBE_FLOOR and PROBE_WALL). */
enum { STAGE_C_PROBE_FLOOR = 0x01, STAGE_C_PROBE_WALL = 0x02 };
/* The VDC register numbers the kernel's VDC selectors leave in A (video.c's VDC_REG_). */
enum { STAGE_C_VDC_MAWR = 0x00, STAGE_C_VDC_MARR = 0x01, STAGE_C_VDC_VRAM = 0x02, STAGE_C_VDC_CR = 0x05 };

#endif
