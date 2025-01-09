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
#define NUM_FONT_FACTOR 3

enum FontSize { FONT_SMALL, FONT_MEDIUM, FONT_LARGE};

extern ImFont* FONTS_SMALL[NUM_FONT_FACTOR];
extern ImFont* FONTS_MEDIUM[NUM_FONT_FACTOR];
extern ImFont* FONTS_LARGE[NUM_FONT_FACTOR];
extern ImFont* DEFAULT_FONT;
static float FONT_SIZE_SMALL = 10.f;
static float FONT_SIZE_MEDIUM = 12.f;
static float FONT_SIZE_LARGE = 14.f;
static float FONT_SIZE_FACTOR[NUM_FONT_FACTOR] = { 1.f, 2.f, 4.f };


void CreateFontAtlas();
void DeleteFontAtlas();

JVR_NAMESPACE_CLOSE_SCOPE

#endif