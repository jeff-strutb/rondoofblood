/* The routines of menu/scene_text_c.c. */
#ifndef RONDO_SCENE_TEXT_C_H
#define RONDO_SCENE_TEXT_C_H

#include <stdint.h>

void SceneTextObjectAt(uint16_t at, uint8_t slot);
void Scene58TextObject(uint8_t slot);
void abi_Scene58TextObject(void);
void SceneTextStartAt(uint16_t at);
void Scene58TextStart(void);
void abi_Scene58TextStart(void);
void SceneTextSkipSpaceAt(uint16_t at, uint8_t slot);
void Scene58TextSkipSpace(uint8_t slot);
void abi_Scene58TextSkipSpace(void);
void SceneTextPatternsAt(uint16_t at, uint8_t slot);
void Scene58TextPatterns(uint8_t slot);
void abi_Scene58TextPatterns(void);
void SceneTextWriteAt(uint16_t at);
void Scene58TextWrite(void);
void abi_Scene58TextWrite(void);
void SceneTextNextGlyphAt(uint16_t at);
void Scene58TextNextGlyph(void);
void abi_Scene58TextNextGlyph(void);
void SceneTextLineDoneAt(uint16_t at, uint8_t slot);
void Scene58TextLineDone(uint8_t slot);
void abi_Scene58TextLineDone(void);
void SceneTextReturnAt(uint16_t at);
void Scene58TextReturn(void);
void abi_Scene58TextReturn(void);
void SceneTextBlankGlyphAt(uint16_t at);
void Scene58TextBlankGlyph(void);
void abi_Scene58TextBlankGlyph(void);
void SceneTextDrawGlyphAt(uint16_t at);
void Scene58TextDrawGlyph(void);
void abi_Scene58TextDrawGlyph(void);
void SceneTextShiftGlyphAt(uint16_t at);
void Scene58TextShiftGlyph(void);
void abi_Scene58TextShiftGlyph(void);
void SceneTextGlyphVramAddressAt(uint16_t at, uint8_t slot);
void Scene58TextGlyphVramAddress(uint8_t slot);
void abi_Scene58TextGlyphVramAddress(void);
void SceneTextRiseAt(uint16_t at);
void Scene58TextRise(void);
void abi_Scene58TextRise(void);
void SceneTextShowAt(uint16_t at, uint8_t slot);
void Scene58TextShow(uint8_t slot);
void abi_Scene58TextShow(void);
void SceneTextEndAt(uint16_t at);
void Scene58TextEnd(void);
void abi_Scene58TextEnd(void);
void SceneEffectsVecStep(void);
void abi_SceneEffectsVecStep(void);
void Scene5AQuakeScroll(uint8_t slot);
void abi_Scene5AQuakeScroll(void);
void Scene5AJitterScroll(void);
void abi_Scene5AJitterScroll(void);
void Scene5ATextObject(uint8_t slot);
void abi_Scene5ATextObject(void);
void Scene5ATextStart(void);
void abi_Scene5ATextStart(void);
void Scene5ATextSkipSpace(uint8_t slot);
void abi_Scene5ATextSkipSpace(void);
void Scene5ATextPatterns(uint8_t slot);
void abi_Scene5ATextPatterns(void);
void Scene5ATextWrite(void);
void abi_Scene5ATextWrite(void);
void Scene5ATextNextGlyph(void);
void abi_Scene5ATextNextGlyph(void);
void Scene5ATextLineDone(uint8_t slot);
void abi_Scene5ATextLineDone(void);
void Scene5ATextReturn(void);
void abi_Scene5ATextReturn(void);
void Scene5ATextBlankGlyph(void);
void abi_Scene5ATextBlankGlyph(void);
void Scene5ATextDrawGlyph(void);
void abi_Scene5ATextDrawGlyph(void);
void Scene5ATextShiftGlyph(void);
void abi_Scene5ATextShiftGlyph(void);
void Scene5ATextGlyphVramAddress(uint8_t slot);
void abi_Scene5ATextGlyphVramAddress(void);
void Scene5ATextRise(void);
void abi_Scene5ATextRise(void);
void Scene5ATextShow(uint8_t slot);
void abi_Scene5ATextShow(void);
void Scene5ATextEnd(void);
void abi_Scene5ATextEnd(void);

#endif
