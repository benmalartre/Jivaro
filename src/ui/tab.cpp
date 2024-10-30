#include <memory>
#include <vector>
#include "../common.h"
#include "../ui/style.h"
#include "../ui/tab.h"
#include "../app/view.h"
#include "../app/window.h"
#include "../app/application.h"


JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags ViewTabUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoBackground |
  ImGuiWindowFlags_NoCollapse |
  ImGuiWindowFlags_NoNav |
  ImGuiWindowFlags_NoScrollWithMouse |
  ImGuiWindowFlags_NoScrollbar;

int ViewTabUI::ViewTabUIID = 0;

// constructor
ViewTabUI::ViewTabUI(View* parent)
  : _parent(parent)
  , _invade(false)
  , _current(-1)
{
  _id = ViewTabUIID++;
}


// destructor
ViewTabUI::~ViewTabUI()
{
}

void ViewTabUI::SetView(View* view)
{
  _parent = view;
}

std::string 
ViewTabUI::_ComputeName(int index, const char* suffix)
{
  return VIEW_TAB_NAME + std::to_string(index) + suffix;
}


bool
ViewTabUI::Draw()
{
  ImGuiStyle& style = ImGui::GetStyle();
  const GfVec2f min(_parent->GetMin());
  const GfVec2f size(_parent->GetWidth(), GetHeight());
  
  ImGui::SetNextWindowPos(min);
  ImGui::SetNextWindowSize(size);

  ImGui::Begin(_ComputeName(_id).c_str(), NULL, ViewTabUI::_flags);

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
      min + GfVec2f(0, size[1] - 4),
      min + size,
      ImColor(GfVec4f(1.f, 1.f, 1.f, 0.25f))
    );
  }
  
  static ImGuiTabBarFlags tabBarFlags = 
    ImGuiTabBarFlags_AutoSelectNewTabs | 
    ImGuiTabBarFlags_Reorderable | 
    ImGuiTabBarFlags_FittingPolicyScroll;

  if (ImGui::BeginTabBar(_ComputeName(_id, "TabBar").c_str(), tabBarFlags))
  {
    if (ImGui::TabItemButton(ICON_FA_CARET_DOWN, ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip)) {
      ImGui::SetNextWindowPos(min + GfVec2i(12, 12));
      ImGui::OpenPopup(_ComputeName(_id, "Uis").c_str());
      _invade = true;
    }
    if (ImGui::BeginPopup(_ComputeName(_id, "Uis").c_str()))
    {
      if (_invade)_parent->SetFlag(View::DISCARDMOUSEBUTTON);
      for (size_t n = UIType::VIEWPORT; n < UIType::COUNT; ++n) {
        ImGui::Selectable(UITypeName[n]);
        if (ImGui::IsItemClicked()) {
          _parent->CreateUI(UIType(n));
          _current = _parent->GetUIs().size() - 1;
          _invade = false;
        }
      }
      ImGui::EndPopup();
    }
    
    // Submit our regular tabs
    BaseUI* current = _parent->GetCurrentUI();
    std::vector<BaseUI*>& uis = _parent->GetUIs();
    for (int n = 0; n < uis.size(); ++n)
    {
      const char* name = UITypeName[uis[n]->GetType()];
      if (ImGui::BeginTabItem(name, NULL,
        ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | 
        ImGuiTabItemFlags_NoPushId))
      {
        if (n != _current) {
          _parent->SetCurrentUI(n);
          _current = n;
        }
        
        ImGui::EndTabItem();
      }
    }

    ImGui::EndTabBar();

    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_Button, TRANSPARENT_COLOR);
    ImGui::SetCursorPos(
      ImVec2(
        _parent->GetWidth() - 
        (BUTTON_MINI_SIZE[0] + 2 * style.ItemSpacing[0] + style.FramePadding[0]),
        0
      ));

    if (ImGui::Button(ICON_FA_BARS, BUTTON_MINI_SIZE)) {
      ImGui::SetNextWindowPos(min + GfVec2i(12, 12));
      ImGui::OpenPopup(_ComputeName(_id, "View").c_str());
      _invade = true;
    }
    if (ImGui::BeginPopup(_ComputeName(_id, "View").c_str()))
    {
      if (_invade)_parent->SetFlag(View::DISCARDMOUSEBUTTON);
      ImGui::Selectable("Split Horizontaly");
      if (ImGui::IsItemClicked()) {
        Application::Get()->AddDeferredCommand(
          std::bind(&View::Split, _parent, 0.5, true, false, 0));
        _invade = false;
      }

      ImGui::Selectable("Split Verticaly");
      if (ImGui::IsItemClicked()) {
        Application::Get()->AddDeferredCommand(
          std::bind(&View::Split, _parent, 0.5, false, false, 0));
        _invade = false;
      }

      ImGui::Selectable("Delete View");
      if (ImGui::IsItemClicked()) {
        Window* window = _parent->GetWindow();
        Application::Get()->AddDeferredCommand(
          std::bind(&Window::RemoveView, window, _parent));
        _invade = false;
      }
      
      ImGui::EndPopup();
    }
  };
    

  ImGui::PopStyleVar();
  ImGui::PopStyleColor();

  ImGui::End();
  return _invade;

}

void ViewTabUI::MouseMove(int x, int y)
{

}

void ViewTabUI::Focus(bool state)
{
  if (_invade && !state)_invade = false;
}

void ViewTabUI::MouseButton(int button, int action, int mods)
{
  if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
    if (_invade)
      _parent->GetWindow()->DirtyAllViews(false);
    
    _invade = false;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE