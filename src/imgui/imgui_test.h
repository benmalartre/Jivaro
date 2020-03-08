#pragma once

#include "imgui.h"
#include <iostream>

#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec4i.h>

namespace ImGui {
  IMGUI_API  bool 
  TabLabels(int numTabs, const char** tabLabels, int& selectedIndex, 
    const char** tabLabelTooltips, bool autoLayout, int *pOptionalHoveredIndex) ;

  IMGUI_API void
  FillBackground();

  IMGUI_API  void 
  TestDummyView(bool* p_open, const pxr::GfVec2f& vmin, const pxr::GfVec2f& vmax,
    const pxr::GfVec4f& color);

  IMGUI_API  void 
  TestGraphNodes(bool* p_open, const pxr::GfVec2f& vmin, const pxr::GfVec2f& vmax); 

} // namespace ImGui