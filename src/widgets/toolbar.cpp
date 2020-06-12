#include "toolbar.h"
#include "../app/view.h"
#include "../widgets/icon.h"

AMN_NAMESPACE_OPEN_SCOPE

ToolbarUI::ToolbarUI(View* parent, const std::string& name) :BaseUI(parent, name) 
{
  IconButtonUI icon;
  icon.Build(&PlayIcon[0], PlayIconN);
  _icons.push_back(icon);
}

ToolbarUI::~ToolbarUI() {}

bool ToolbarUI::Draw()
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
  /*
  drawList->AddRectFilled(
    ImVec2(0, 0),
    ImVec2(_parent->GetSize()),
    ImColor(color[0], color[1], color[2], color[3])
  );
  */
  for (auto& icon : _icons)icon.Draw(drawList);

  ImGui::End();
  return true;
};


AMN_NAMESPACE_CLOSE_SCOPE