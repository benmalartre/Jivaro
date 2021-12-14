#include "../ui/ui.h"
#include "../ui/popup.h"
#include "../ui/splitter.h"
#include "../utils/timer.h"
#include "../app/window.h"
#include "../app/view.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>


AMN_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags PopupUI::_flags = 
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoBackground;

// Popup constructor
//----------------------------------------------------------------------------
PopupUI::PopupUI(View* parent, int x, int y, int w, int h)
  : BaseUI(parent, "##popup_" + std::to_string(CurrentTime()), true)
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
  double x, y;
  glfwGetCursorPos(GetWindow()->GetGlfwWindow(), &x, &y);
  if (x > _x && y > y && x < (_x + _width) && y < (_y + _height)) {
    std::cout << "POPUP MOUSE BUTTON : (INSIDE) = " << x << "," << y << std::endl;
  }
  else {
    std::cout << "POPUP MOUSE BUTTON : (OUTSIDE) = " << x << "," << y << std::endl;
  }
  
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

  ImGui::Begin(_name.c_str(), &opened, _flags);
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