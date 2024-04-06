#ifndef JVR_UI_FONTS_H
#define JVR_UI_FONTS_H

#include <map>

#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../common.h"
#include "../fonts/fonts/IBMPlexMonoFree.h"
#include "../fonts/fonts/IBMPlexSansMediumFree.h"
#include "../fonts/iconfontcppheaders/IconsFontAwesome6.h"
#include "../fonts/fontawesome/FontAwesomeFree6.h"

JVR_NAMESPACE_OPEN_SCOPE  

extern ImFontAtlas* SHARED_ATLAS;

#define NUM_FONT_SIZE 3
extern ImFont* FONTS[NUM_FONT_SIZE];
extern ImFont* DEFAULT_FONT;
static float FONT_SIZE[NUM_FONT_SIZE] = { 32.f, 48.f, 64.f };


void CreateFontAtlas();
void DeleteFontAtlas();

JVR_NAMESPACE_CLOSE_SCOPE

#endif