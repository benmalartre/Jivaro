#include "dummy.h"
#include "../app/view.h"

JVR_NAMESPACE_OPEN_SCOPE

DummyUI::DummyUI(View* parent, const std::string& name):BaseUI(parent, name){}

DummyUI::~DummyUI(){}

bool DummyUI::Draw()
{
  bool opened;
  int flags = 0;
  flags |= ImGuiWindowFlags_NoResize;
  flags |= ImGuiWindowFlags_NoTitleBar;
  flags |= ImGuiWindowFlags_NoMove;
  

  ImGui::Begin(_name.c_str(), &opened, flags);
  pxr::GfVec4f color(
    RANDOM_0_1,
    RANDOM_0_1,
    RANDOM_0_1, 
    1.f
  );

  ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetWindowPos(_parent->GetMin());
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    ImVec2(0, 0),
    ImVec2(_parent->GetSize()),
    ImColor(color[0], color[1], color[2], color[3])
  );
  Demo();
  ImGui::End();
  return true;
};
  
void DummyUI::FillBackground()
{
  ImVec2 vMin = ImGui::GetWindowContentRegionMin();
  ImVec2 vMax = ImGui::GetWindowContentRegionMax();

  vMin.x += ImGui::GetWindowPos().x;
  vMin.y += ImGui::GetWindowPos().y;
  vMax.x += ImGui::GetWindowPos().x;
  vMax.y += ImGui::GetWindowPos().y;

  ImGui::GetForegroundDrawList()->AddRect( vMin, vMax, IM_COL32( 255, 255, 0, 255 ) );
}

void DummyUI::Demo()
{
  ImGui::ShowDemoWindow();
}

JVR_NAMESPACE_CLOSE_SCOPE