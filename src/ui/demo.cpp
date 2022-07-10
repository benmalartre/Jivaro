
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
  : HeadedUI(parent, "DemoXXX")
{
}

DemoUI::~DemoUI()
{
}

bool DemoUI::Draw()
{
  std::cout << "PARENT : " << _parent->GetName() << std::endl;
  static bool opened = false;
  const pxr::GfVec2f pos(GetX(), GetY());
  const pxr::GfVec2f size(GetWidth(), GetHeight());
  std::cout << "DEMO POS " << pos << std::endl;
  std::cout << "DEMO SIZE : " << size << std::endl;
  ImGui::Begin(_name.c_str(), &opened, _flags);
  ImGui::SetWindowSize(size);
  ImGui::SetWindowPos(pos);
 
  ImGui::ShowDemoWindow();

  ImGui::End();

  return true;
};
  

JVR_NAMESPACE_CLOSE_SCOPE