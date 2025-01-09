
#include <iostream>
#include <array>
#include <utility>
#include "../imgui/imgui_impl_glfw.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_internal.h"

#include "../ui/demo.h"
#include "../app/view.h"

JVR_NAMESPACE_OPEN_SCOPE


ImGuiWindowFlags DemoUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoCollapse |
  ImGuiWindowFlags_NoNav |
  ImGuiWindowFlags_NoScrollWithMouse |
  ImGuiWindowFlags_NoScrollbar;


DemoUI::DemoUI(View* parent)
  : BaseUI(parent, UIType::DEMO)
{
}

DemoUI::~DemoUI()
{
}

bool DemoUI::Draw()
{
  static bool opened = false;
  const GfVec2f pos(GetX(), GetY());
  const GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos(pos);

  ImGui::Begin(_name.c_str(), &opened, _flags);

  ImGui::ShowDemoWindow();

  ImGui::End();

  return ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
};
  

JVR_NAMESPACE_CLOSE_SCOPE