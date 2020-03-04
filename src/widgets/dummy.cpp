#include "dummy.h"
#include "../view.h"

namespace AMN {

DummyUI::DummyUI(AmnView* parent, const std::string& name):AmnUI(parent, name){}

DummyUI::~DummyUI(){}

void DummyUI::Event()
{
  std::cerr << "DummyUI EVENT!" << std::endl;
};

void DummyUI::Draw()
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

  //ImGui::TestDummyView(&opened, _parent->GetMin(), _parent->GetMax(), color);
  ImGui::TestGraphNodes(&opened, _parent->GetMin(), _parent->GetMax());
  ImGui::SetWindowSize(_parent->GetMax() - _parent->GetMin());
  ImGui::SetWindowPos(_parent->GetMin());
  ImGui::End();
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

} // namespace AMN