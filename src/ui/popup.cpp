#include "../app/window.h"
#include "../app/view.h"
#include "../ui/ui.h"
#include "../ui/popup.h"
#include "../ui/splitter.h"
#include "../utils/timer.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>


AMN_NAMESPACE_OPEN_SCOPE

PopupUIItem::PopupUIItem(BaseUI* ui, const std::string& lbl, bool tgl, bool enb, 
  PopupItemPressedFunc func, const pxr::VtArray<pxr::VtValue> args)
  : ui(ui)
  , label(lbl)
  , toggable(tgl)
  , enabled(enb)
  , func(func)
  , args(args)
{
}

bool PopupUIItem::Draw()
{
  Window* window = ui->GetView()->GetWindow();
  ImGui::PushFont(window->GetRegularFont(0));
  
  /*
  if(toggable) {
    AddCheckableIconButton<IconPressedFunc>(
      icon,
      (window->GetActiveTool() == tool) ? AMN_ICON_SELECTED : AMN_ICON_DEFAULT,
      func
    );
  } else {
    AddIconButton<IconPressedFunc>(
      icon,
      AMN_ICON_DEFAULT,
      func
    );
  }
  */
  ImGui::PopFont(); 
  return false;
}

static void _FillBackground()
{
  ImVec2 vMin = ImGui::GetWindowContentRegionMin();
  ImVec2 vMax = ImGui::GetWindowContentRegionMax();

  vMin.x += ImGui::GetWindowPos().x;
  vMin.y += ImGui::GetWindowPos().y;
  vMax.x += ImGui::GetWindowPos().x;
  vMax.y += ImGui::GetWindowPos().y;

  ImGui::GetForegroundDrawList()->AddRect( vMin, vMax, IM_COL32( 255, 255, 0, 255 ) );
}

// Popup constructor
//----------------------------------------------------------------------------
PopupUI::PopupUI(View* parent, int x, int y, int w, int h)
  : BaseUI(parent, "popup_" + std::to_string(ns()))
  , _x(x)
  , _y(y)
  , _width(w)
  , _height(h)
{
}

PopupUI::~PopupUI()
{
}

// mouse positon relative to the view
void PopupUI::GetRelativeMousePosition(const float inX, const float inY,
  float& outX, float& outY)
{
  outX = inX - _x;
  outY = inY - _y;
}


void 
PopupUI::MouseButton(int button, int action, int mods)
{
  std::cout << "POPUP MOUSE BUTTON : " << button << "," << action << "," << mods << std::endl;
  //MouseButton(button, action, mods);
}

void
PopupUI::MouseMove(int x, int y)
{

}

bool 
PopupUI::Draw()
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

  ImGui::SetWindowSize(ImVec2(_width, _height));
  ImGui::SetWindowPos(ImVec2(_x, _y));
  ImDrawList* drawList = ImGui::GetForegroundDrawList();
  drawList->AddRectFilled(
    ImVec2(_x, _y),
    ImVec2(_x + _width, _y + _height),
    ImColor(color[0], color[1], color[2], color[3])
  );
  ImGui::End();
  return true;
};


AMN_NAMESPACE_CLOSE_SCOPE