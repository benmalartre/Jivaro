#pragma once

#include "exporer.h"
#include "utils.h"
#include <pxr/usd/usd/prim.h>

namespace AMN {
IMGUI_API void
FillBackground()
{
  ImVec2 vMin = ImGui::GetWindowContentRegionMin();
  ImVec2 vMax = ImGui::GetWindowContentRegionMax();

  

  vMin.x += ImGui::GetWindowPos().x;
  vMin.y += ImGui::GetWindowPos().y;
  vMax.x += ImGui::GetWindowPos().x;
  vMax.y += ImGui::GetWindowPos().y;

  ImGui::GetForegroundDrawList()->AddRect( vMin, vMax, IM_COL32( 255, 255, 0, 255 ) );
}

IMGUI_API  void 
TestDummyView(bool* p_open, const pxr::GfVec2i& vmin, const pxr::GfVec2i& vmax,
  const pxr::GfVec4f& color)
{
  ImGui::SetCursorScreenPos(vmin);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  const ImU32 col = ImColor(color);
  draw_list->AddRectFilled(vmin, vmax, col, 0,  0); 
}

} // namespace AMN