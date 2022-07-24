#include <memory>
#include <vector>
#include "../common.h"
#include "../ui/style.h"
#include "../ui/head.h"
#include "../ui/viewport.h"
#include "../ui/contentBrowser.h"
#include "../ui/graphEditor.h"
#include "../ui/propertyEditor.h"
#include "../ui/curveEditor.h"
#include "../ui/layerEditor.h"
#include "../ui/debug.h"
#include "../ui/demo.h"

#include "../app/view.h"
#include "../app/window.h"


JVR_NAMESPACE_OPEN_SCOPE

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
  case UIType::CONTENTBROWSER:
    new ContentBrowserUI(_parent);
    break;
  case UIType::GRAPHEDITOR:
    new GraphEditorUI(_parent);
    break;
  case UIType::CURVEEDITOR:
    new CurveEditorUI(_parent);
    break;
  case UIType::LAYEREDITOR:
    new LayerEditorUI(_parent);
    break;
  case UIType::DEBUG:
    new DebugUI(_parent);
    break;
  case UIType::DEMO:
    new DemoUI(_parent);
    break;
  case UIType::PROPERTYEDITOR:
    new PropertyUI(_parent);
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

bool
ViewHead::Draw()
{
  ImGuiStyle& style = ImGui::GetStyle();
  const pxr::GfVec2f min(_parent->GetMin());
  const pxr::GfVec2f size(_parent->GetWidth(), GetHeight());
  static bool open;

  ImGui::Begin(("##" + _name).c_str(), &open, ViewHead::_flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);
  //ImGui::PushFont(_parent->GetWindow()->GetMediumFont(1));

  _height = ImGui::GetTextLineHeight() + style.FramePadding[1] * 2 + style.WindowPadding[1];

  const ImVec4* colors = style.Colors;
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  
  drawList->AddRectFilled(
    min,
    min + size,
    ImColor(style.Colors[ImGuiCol_FrameBgHovered])
  );

  if (_parent->IsActive()) {
    drawList->AddRectFilled(
      min + pxr::GfVec2f(0, size[1] - 4),
      min + size - pxr::GfVec2f(0, 3),
      ImColor(pxr::GfVec4f(1.f, 1.f, 1.f, 0.5f))
    );
  }

  static ImGuiTabBarFlags tabBarFlags = 
    ImGuiTabBarFlags_AutoSelectNewTabs | 
    ImGuiTabBarFlags_Reorderable | 
    ImGuiTabBarFlags_FittingPolicyScroll;

  if (ImGui::BeginTabBar((_name + "TabBar").c_str(), tabBarFlags))
  {
    const std::string popupName = _name + "Popup";
    if (ImGui::TabItemButton(" + ", ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip)) {
      ImGui::SetNextWindowPos(min + pxr::GfVec2i(12, 12));
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

    // Submit our regular tabs
    for (int n = 0; n < _childrens.size(); ++n)
    {
      bool open = true;
      const char* name = UITypeName[_childrens[n]->GetType()];
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

    if (ImGui::TabItemButton(" x ", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
      std::cout << "DELETE CURRENT TAB ITEM" << std::endl;
    }

    ImGui::EndTabBar();
  }

  //ImGui::PopFont();
  ImGui::End();
  return _invade;
}

void ViewHead::MouseMove(int x, int y)
{

}
void ViewHead::MouseButton(int button, int action, int mods)
{
  if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
    if (_invade) {
      _parent->SetDirty();
      _parent->GetWindow()->ForceRedraw();
    }
    _invade = false;
  }
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
HeadedUI::HeadedUI(View* parent, short type)
  : BaseUI(parent, type)
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
  ImGuiStyle& style = ImGui::GetStyle();
  if (_head) {
    return (_parent->GetMin()[1] + _head->GetHeight()) - 1;
  } else {
    return (_parent->GetMin()[1]) - 1;
  }
}

int HeadedUI::GetWidth()
{
  return _parent->GetWidth() + 2;
}

int HeadedUI::GetHeight()
{
  ImGuiStyle& style = ImGui::GetStyle();
  if (_head) {
    return (_parent->GetHeight() - _head->GetHeight()) + 2;
  } else {
    return _parent->GetHeight() + 2;
  }
}

// mouse positon relative to the view
void HeadedUI::GetRelativeMousePosition(const float inX, const float inY,
  float& outX, float& outY)
{
  pxr::GfVec2f parentPosition = _parent->GetMin();
  float parentX = parentPosition[0];
  float parentY = parentPosition[1];
  if(_head) parentY += _head->GetHeight();
  outX = inX - parentX;
  outY = inY - parentY;
}


JVR_NAMESPACE_CLOSE_SCOPE