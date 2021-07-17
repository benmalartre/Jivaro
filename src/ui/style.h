#ifndef AMN_UI_STYLE_H
#define AMN_UI_STYLE_H

#include "../common.h"
#include "../imgui/imgui.h"

AMN_NAMESPACE_OPEN_SCOPE

static ImVec4 AMN_BACKGROUND_COLOR(0.2f, 0.2f, 0.2f, 1.f);
static ImVec4 AMN_ALTERNATE_COLOR(0.25f, 0.25f, 0.25f, 1.f);
static ImVec4 AMN_SELECTED_COLOR(0.9f, 0.9f, 0.9f, 1.f);
static ImVec4 AMN_HIGHLIGHTED_COLOR(0.4f, 0.4f, 0.4f, 1.f);
static ImVec4 AMN_HOVERED_COLOR(0.75f, 0.75f, 0.75f, 1.f);
static ImVec4 AMN_TRANSPARENT_COLOR(0.f, 0.f, 0.f, 0.f);
static ImVec4 AMN_TRANSPARENT_HOVERED_COLOR(1.f, 1.f, 1.f, 0.1f);
static ImVec4 AMN_BUTTON_COLOR(0.25f, 0.25f, 0.25f, 1.f);
static ImVec4 AMN_BUTTON_HOVERED_COLOR(0.33f, 0.33f, 0.33f, 1.f);
static ImVec4 AMN_BUTTON_ACTIVE_COLOR(0.9f, 0.9f, 0.9f, 1.f);
static ImVec4 AMN_TEXT_DEFAULT_COLOR(0.9f, 0.9f, 0.9f, 1.f);
static ImVec4 AMN_TEXT_SELECTED_COLOR(0.1f, 0.1f, 0.1f, 1.f);
static ImVec4 AMN_TEXT_DISABLED_COLOR(0.5f, 0.5f, 0.5f, 1.f);

static void AMNStyle(ImGuiStyle* dst)
{
  ImGuiStyle* style = dst ? dst : &ImGui::GetStyle();
  ImVec4* colors = style->Colors;
  
  style->Alpha = 1.f;
  style->WindowRounding = 0.0f;
  style->ChildRounding = 0.0f;
  style->FrameRounding = 0.0f;
  style->GrabRounding = 0.0f;
  style->PopupRounding = 0.0f;
  style->ScrollbarRounding = 0.0f;
  style->TabRounding = 0.0f;
  style->WindowPadding = pxr::GfVec2f(4.f, 0.f);
  style->FramePadding = pxr::GfVec2f(0.f, 0.f);
  style->ChildBorderSize = 0.f;
  style->FrameBorderSize = 0.f;
  style->IndentSpacing = 2.f;
  style->ItemSpacing = pxr::GfVec2f(4.f,4.f);
  style->ItemInnerSpacing = pxr::GfVec2f(0.f, 4.f);
  style->FrameBorderSize = 1.0f;
  style->FrameRounding = 2.f;
  style->AntiAliasedLines = true;
  style->AntiAliasedFill = true;

  colors[ImGuiCol_Text] = ImVec4(0.75f, 0.75f, 0.75f, 1.f);
  colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.f);
  colors[ImGuiCol_WindowBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
  //colors[ImGuiCol_ChildWindowBg]         = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_ChildBg] = ImVec4(0.25f, 0.25f, 0.25f, 1.0f);
  colors[ImGuiCol_PopupBg] = ImVec4(0.35f, 0.35f, 0.35f, 1.0f);
  colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
  colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
  colors[ImGuiCol_FrameBg] = ImVec4(0.15f, 0.15f, 0.15f, 1.0f);
  colors[ImGuiCol_FrameBgHovered] = ImVec4(0.1f, 0.1f, 0.1f, 1.0f);
  colors[ImGuiCol_FrameBgActive] = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);
  colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.82f, 0.78f, 0.78f, 0.51f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
  colors[ImGuiCol_MenuBarBg] = ImVec4(0.2f, 0.2f, 0.2f, 1.f);
  colors[ImGuiCol_ScrollbarBg] = ImVec4(0.20f, 0.25f, 0.30f, 0.61f);
  colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.90f, 0.90f, 0.90f, 0.30f);
  colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.92f, 0.92f, 0.92f, 0.78f);
  colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
  colors[ImGuiCol_CheckMark] = ImVec4(0.184f, 0.407f, 0.193f, 1.00f);
  colors[ImGuiCol_SliderGrab] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  colors[ImGuiCol_SliderGrabActive] = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
  colors[ImGuiCol_Button] = AMN_BUTTON_COLOR;
  colors[ImGuiCol_ButtonHovered] = AMN_BUTTON_HOVERED_COLOR;
  colors[ImGuiCol_ButtonActive] = AMN_BUTTON_ACTIVE_COLOR;
  colors[ImGuiCol_Header] = ImVec4(0.35f, 0.35f, 0.35f, 0.25f);
  colors[ImGuiCol_HeaderHovered] = ImVec4(0.35f, 0.35f, 0.35f, 0.5f);
  colors[ImGuiCol_HeaderActive] = ImVec4(0.35f, 0.35f, 0.35f, 0.75f);
  colors[ImGuiCol_Separator] = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
  colors[ImGuiCol_SeparatorHovered] = ImVec4(0.5f, 0.5f, 0.5f, 1.f);
  colors[ImGuiCol_SeparatorActive] = ImVec4(0.6f, 0.6f, 0.6f, 1.f);
  colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.00f);
  colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.26f, 0.59f, 0.98f, 0.45f);
  colors[ImGuiCol_ResizeGripActive] = ImVec4(0.26f, 0.59f, 0.98f, 0.78f);
  colors[ImGuiCol_PlotLines] = ImVec4(0.39f, 0.39f, 0.39f, 1.00f);
  colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
  colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
  colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
  colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
  colors[ImGuiCol_ModalWindowDarkening] = ImVec4(0.20f, 0.20f, 0.20f, 0.35f);
  colors[ImGuiCol_DragDropTarget] = ImVec4(0.26f, 0.59f, 0.98f, 0.95f);
  colors[ImGuiCol_NavHighlight] = colors[ImGuiCol_HeaderHovered];
  colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.70f, 0.70f, 0.70f, 0.70f);
  colors[ImGuiCol_Tab] = ImVec4(0.52f, 0.52f, 0.52f, 1.00f);
  colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
  colors[ImGuiCol_TabHovered] = ImVec4(0.62f, 0.62f, 0.62f, 1.00f);
}

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_UI_STYLE_H