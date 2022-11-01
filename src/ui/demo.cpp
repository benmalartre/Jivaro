
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
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove;


DemoUI::DemoUI(View* parent)
  : BaseUI(parent, UIType::DEMO)
{
}

DemoUI::~DemoUI()
{
}

bool DemoUI::Draw()
{
  /*
  static bool opened = false;
  const pxr::GfVec2f pos(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());

  ImGui::Begin(_name.c_str(), &opened, _flags);
  ImGui::SetWindowSize(size);
  ImGui::SetWindowPos(pos);
 
  ImGui::ShowDemoWindow();

  ImGui::End();

  return ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
  */
  return false;
};
  

JVR_NAMESPACE_CLOSE_SCOPE