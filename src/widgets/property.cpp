#include "property.h"
#include "../app/view.h"

AMN_NAMESPACE_OPEN_SCOPE

PropertyUI::PropertyUI(View* parent, const std::string& name):BaseUI(parent, name){}

PropertyUI::~PropertyUI(){}

bool PropertyUI::Draw()
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

  ImGui::SetWindowSize(_parent->GetSize());
  ImGui::SetWindowPos(_parent->GetMin());

  ImDrawList* drawList = ImGui::GetWindowDrawList();

  drawList->AddRectFilled(
    _parent->GetMin(),
    _parent->GetMax(),
    ImColor(color[0], color[1], color[2], color[3])
  );

  ImGui::End();
  return true;
};
  

AMN_NAMESPACE_CLOSE_SCOPE