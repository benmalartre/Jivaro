#include <memory>
#include <vector>
#include "../common.h"
#include "../ui/style.h"
#include "../ui/tab.h"
#include "../app/view.h"
#include "../app/window.h"


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
  const pxr::GfVec2f min(_parent->GetMin());
  const pxr::GfVec2f size(_parent->GetWidth(), GetHeight());
  static bool open;

  ImGui::Begin(_ComputeName(_id).c_str(), &open, ViewTabUI::_flags);
  ImGui::SetWindowPos(min);
  ImGui::SetWindowSize(size);

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

  int button_state = 0;

  if (ImGui::BeginTabBar(_ComputeName(_id, "TabBar").c_str(), tabBarFlags))
  {
    const char* popupName = _ComputeName(_id, "Popup").c_str();
    if (ImGui::TabItemButton(" + ", ImGuiTabItemFlags_Leading | ImGuiTabItemFlags_NoTooltip)) {
      ImGui::SetNextWindowPos(min + pxr::GfVec2i(12, 12));
      ImGui::OpenPopup(popupName);
      _invade = true;
    }
    if (ImGui::BeginPopup(popupName))
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
      bool open = true;
      const char* name = UITypeName[uis[n]->GetType()];
      if (ImGui::BeginTabItem(name, &open,
        ImGuiTabItemFlags_NoCloseButton | ImGuiTabItemFlags_NoCloseWithMiddleMouseButton | ImGuiTabItemFlags_NoPushId))
      {
        if (n != _current) {
          _parent->SetCurrentUI(n);
          _current = n;
        }
        
        ImGui::EndTabItem();
      }
    }

    if (ImGui::TabItemButton(" x ", ImGuiTabItemFlags_Trailing | ImGuiTabItemFlags_NoTooltip)) {
      _parent->RemoveUI(current);
    }

    ImGui::EndTabBar();

    ImGui::SetWindowFontScale(0.5);
    ImGui::SetCursorPos(
      ImVec2(
        _parent->GetWidth() - (3 * BUTTON_MINI_SIZE[0] + 2 * style.ItemSpacing[0] + style.FramePadding[0]), 
        6
      ));

    Window* window = _parent->GetWindow();
    if (ImGui::Button(ICON_FA_GRIP_LINES, BUTTON_MINI_SIZE)) {
      _parent->Split(0.5, true);
      window->Resize(window->GetWidth(), window->GetHeight());
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_GRIP_LINES_VERTICAL, BUTTON_MINI_SIZE)) {
      _parent->Split(0.5, false);
      window->Resize(window->GetWidth(), window->GetHeight());
    }
    ImGui::SameLine();
    if (ImGui::Button(ICON_FA_XMARK, BUTTON_MINI_SIZE)) {
      /*
      View* parent = _parent->GetParent();
      View* other = _parent->GetSibling();
      ViewTabUI* head = _parent->GetHead();
      BaseUI* content = other->GetContent();
      parent->SetHead(head);
      parent->DeleteChildren();
      //parent->SetContent(content);
      _parent->GetWindow()->ForceRedraw();
      */
      std::cout << "not implemented yet..." << std::endl;

    };
    ImGui::SameLine();
    ImGui::SetWindowFontScale(1.0);
  }

  ImGui::End();
  return _invade;

}

void ViewTabUI::MouseMove(int x, int y)
{

}

void ViewTabUI::MouseButton(int button, int action, int mods)
{
  if (action == GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT) {
    if (_invade) {
      _parent->SetDirty();
      _parent->GetWindow()->ForceRedraw();
    }
    _invade = false;
  }
}

JVR_NAMESPACE_CLOSE_SCOPE