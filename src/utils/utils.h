#ifndef AMN_UTILS_UTILS_H
#define AMN_UTILS_UTILS_H
#pragma once

#include <pxr/imaging/glf/glew.h>
#include "../common.h"
#include "../ui/style.h"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_impl_glfw.h"
#include "icons.h"
#include <bitset>

#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>
#include <pxr/base/gf/vec2d.h>
#include <pxr/base/gf/vec3i.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/base/gf/vec3d.h>
#include <pxr/base/gf/vec4i.h>
#include <pxr/base/gf/vec4f.h>
#include <pxr/base/gf/vec4d.h>

AMN_NAMESPACE_OPEN_SCOPE

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using
// a merged icon fonts (see docs/FONTS.txt)
static void HelpMarker(const char* desc)
{
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered())
  {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

// from https://github.com/aoterodelaroza/imgui-goodies/blob/master/imgui_widgets.cpp
static ImGuiID tooltip_shownid = 0;
static ImGuiID tooltip_hoveredid = 0;
static int tooltip_thisframe = -1;
static bool tooltip_washovered = false;
static float tooltip_time = 0.f;
static float tooltip_lastactive = 0.f;

static void AttachTooltip(const char* desc, float delay, float maxwidth, ImFont* font)
{
	ImGuiContext *g = ImGui::GetCurrentContext();
  ImGuiID id = g->CurrentWindow->DC.LastItemId;
  float time = ImGui::GetTime();
  int thisframe = ImGui::GetFrameCount();

  if (thisframe != tooltip_thisframe){
    // run once every frame, in the first call
    if (!tooltip_washovered){
      tooltip_time = time;
      tooltip_shownid = 0;
      tooltip_hoveredid = 0;
    }
    tooltip_thisframe = thisframe;
    tooltip_washovered = false;
  }

  if (g->HoveredId == id){
    // If no tooltip is being shown and the mouse moves from one tooltip element
    // to another, this is the same as if it moved from a zone without any
    // tooltip elements.
    if (id != tooltip_hoveredid && tooltip_shownid == 0)
      tooltip_time = time;

    if (tooltip_lastactive != 0.f)
      delay = fmin(delay,fmax(tooltip_time - tooltip_lastactive,0.f));

    tooltip_washovered = true;
    tooltip_hoveredid = id;

    if (time - tooltip_time > delay){
      tooltip_shownid = id;
      tooltip_lastactive = time;
      ImGui::BeginTooltip();
      ImGui::PushTextWrapPos(maxwidth);
      ImGui::PushFont(font);
      ImGui::TextUnformatted(desc);
      ImGui::PopFont();
      ImGui::PopTextWrapPos();
      ImGui::EndTooltip();
    }
  }
}

// callback prototype
typedef void(*IconPressedFunc)(...);

template<typename FuncT, typename ...ArgsT>
static void IconButton(Icon* icon, FuncT func, ArgsT... args)
{
  ImGui::BeginGroup();
  ImGui::Image(
    (ImTextureID)(intptr_t)icon->tex,
    ImVec2(icon->size, icon->size)
  );
  ImGui::EndGroup();
}

template<typename FuncT, typename ...ArgsT>
bool AddIconButton(Icon* icon, FuncT func, ArgsT... args)
{
  if (ImGui::ImageButton(
    (ImTextureID)(intptr_t)icon->tex,
    ImVec2(icon->size, icon->size),
    ImVec2(0, 0),
    ImVec2(1, 1),
    -1))
  {
    func(args...);
    ImGui::SameLine();
    return true;
  }
  ImGui::SameLine();
  return false;
}

// print vectors (debug)
static void PrintVector(const pxr::GfVec2i& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] << std::endl;
}

static void PrintVector(const pxr::GfVec2f& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] << std::endl;
}

static void PrintVector(const pxr::GfVec2d& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] << std::endl;
}

static void PrintVector(const pxr::GfVec3f& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] << "," << v[2] << std::endl;
}

static void PrintVector(const pxr::GfVec3d& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] << "," << v[2] << std::endl;
}

static void PrintVector(const pxr::GfVec4f& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] << "," << v[2] << "," << v[3] <<std::endl;
}

static void PrintVector(const pxr::GfVec4d& v, const char* t)
{
  std::cerr << t << ": " << v[0] << "," << v[1] << "," << v[2] << "," << v[3] <<std::endl;
}

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UTILS_UTILS_H