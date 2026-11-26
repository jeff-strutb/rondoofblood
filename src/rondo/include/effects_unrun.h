/* The ending's palette fade in the overlay versions no script runs (menu/effects_unrun.c). */
#ifndef RONDO_EFFECTS_UNRUN_H
#define RONDO_EFFECTS_UNRUN_H

#include "front_end.h"

Exit EndingEffectFadeA(uint8_t slot);
void abi_EndingEffectFadeA(void);
Exit EndingEffectFadeB(uint8_t slot);
void abi_EndingEffectFadeB(void);
Exit EndingEffectFadeC(uint8_t slot);
void abi_EndingEffectFadeC(void);
Exit EndingEffectFadeD(uint8_t slot);
void abi_EndingEffectFadeD(void);

#endif
