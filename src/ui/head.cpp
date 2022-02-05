#include <memory>
#include <vector>
#include "../common.h"
#include "../ui/style.h"
#include "../ui/head.h"
#include "../ui/viewport.h"
#include "../ui/graphEditor.h"

#include "../app/view.h"
#include "../app/window.h"


PXR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags ViewHead::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoBackground |
  ImGuiWindowFlags_NoCollapse |
  ImGuiWindowFlags_NoNav |
  ImGuiWindowFlags_NoScrollWithMouse |
  ImGuiWindowFlags_NoScrollbar;

// constructor
ViewHead::ViewHead(View* parent)
  : _parent(parent)
  , _invade(false)
  , _name(parent->GetName() + "Head")
{
}

// destructor
ViewHead::~ViewHead()
{
}

void
ViewHead::AddChild(BaseUI* child)
{
  _childrens.push_back(child);
}

void
ViewHead::CreateChild(UIType type)
{
  switch (type) {
  case UIType::VIEWPORT:
    new ViewportUI(_parent);
    break;
  case UIType::GRAPHEDITOR:
    new GraphEditorUI(_parent);
    break;
  }
}

void
ViewHead::SetCurrentChild(int index)
{
  if (index >= 0 && index < _childrens.size())
  {
    _childrens[index]->Resize();
    _parent->SetContent(_childrens[index]);
  }
}

void
ViewHead::RemoveChild(BaseUI* child)
{
  for (size_t i = 0; i < _childrens.size(); ++i) {
    if (_childrens[i] == child) {
      _childrens.erase(_childrens.begin() + i);
      delete child;
    }
  }
}

void
ViewHead::Draw()
{
  const pxr::GfVec2f min(_parent->GetMin());
  const pxr::GfVec2f size(_parent->GetWidth(), VIEW_HEAD_HEIGHT);
  static bool open;

  ImGui::Begin(("##" + _name).c_str(), &open, ViewHead::_flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);
  ImGui::PushFont(_parent->GetWindow()->GetMediumFont(1));

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    min,
    min + size,
    ImColor(BACKGROUND_COLOR)
  );

  static ImGuiTabBarFlags tabBarFlags = 
    ImGuiTabBarFlags_AutoSelectNewTabs | 
    ImGuiTabBarFlags_Reorderable | 
    ImGuiTabBarFlags_FittingPolicyScroll;

  if (ImGui::BeginTabBar((_name + "TabBar").c_str(), tabBarFlags))
  {
    const std::string popupName = _name + "Popup";
    if (ImGui::TabItemButton(" + ", ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip)) {
      std::cout << "FUCK" << std::endl;
      ImGui::OpenPopup(popupName.c_str());
      _invade = true;
    }
    if (ImGui::BeginPopup(popupName.c_str()))
    {
      if (_invade)_parent->SetFlag(View::DISCARDMOUSEBUTTON);
      for (size_t n = UIType::VIEWPORT; n < UIType::COUNT; ++n) {
        ImGui::Selectable(UITypeName[n]);
        if (ImGui::IsItemClicked()) {
          CreateChild(UIType(n));
          _invade = false;
        }
      }
      ImGui::EndPopup();
    }

    if (ImGui::TabItemButton(" x ", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip))
      std::cout << "DELETE CURRENT TAB ITEM" << std::endl;

    // Submit our regular tabs
    for (int n = 0; n < _childrens.size(); ++n)
    {
      bool open = true;
      const char* name = _childrens[n]->GetName().c_str();
      if (ImGui::BeginTabItem(name, &open,
        ImGuiTabItemFlags_NoCloseButton | ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoPushId))
      {
        if (n != _current) {
          _current = n;
          SetCurrentChild(n);
        }
        
        ImGui::EndTabItem();
      }
    }
    ImGui::EndTabBar();
  }

  ImGui::PopFont();
  ImGui::End();
}

void ViewHead::MouseMove(int x, int y)
{

}
void ViewHead::MouseButton(int button, int action, int mods)
{

}

ImGuiWindowFlags HeadedUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_MenuBar |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoBringToFrontOnFocus |
  ImGuiWindowFlags_NoDecoration;


// constructor
HeadedUI::HeadedUI(View* parent, const std::string& name)
  : BaseUI(parent, name)
{
  _head = _parent->GetHead();
  if (!_head) _head = _parent->CreateHead();
  _head->AddChild(this);
}

// destructor
HeadedUI::~HeadedUI()
{
}

int HeadedUI::GetX()
{
  return _parent->GetMin()[0] - 1;
}

int HeadedUI::GetY()
{
  return (_parent->GetMin()[1] + VIEW_HEAD_HEIGHT) - 1;
}

int HeadedUI::GetWidth()
{
  return _parent->GetWidth() + 2;
}

int HeadedUI::GetHeight()
{
  return (_parent->GetHeight() - VIEW_HEAD_HEIGHT) + 2;
}

// mouse positon relative to the view
void HeadedUI::GetRelativeMousePosition(const float inX, const float inY,
  float& outX, float& outY)
{
  pxr::GfVec2f parentPosition = _parent->GetMin();
  float parentX = parentPosition[0];
  float parentY = parentPosition[1] + VIEW_HEAD_HEIGHT;
  outX = inX - parentX;
  outY = inY - parentY;
}


PXR_NAMESPACE_CLOSE_SCOPE