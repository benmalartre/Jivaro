#include "../ui/icon.h"
#include "../app/view.h"
#include "../app/window.h"

JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags IconUI::_flags =
  ImGuiWindowFlags_None |
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoBackground;


IconUI::IconUI(View* parent)
  : BaseUI(parent, UIType::DEMO)
  , _offset(0,0)
  , _scale(1.f)
{
}

IconUI::~IconUI()
{
}

void 
IconUI::MouseButton(int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(GetWindow()->GetGlfwWindow(), &x, &y);
  if (button == GLFW_MOUSE_BUTTON_LEFT) {
    switch (action) {
    case GLFW_PRESS:
      _drag = true;
      _origin[0] = x;
      _origin[1] = y;
      break;
    case GLFW_RELEASE:
      _drag = false;
      break;
    }
  }
}

void 
IconUI::MouseMove(int x, int y)
{
  if (_drag) {
    _offset = _origin - GfVec2f(x, y) * _scale;
  }
  _parent->SetDirty();
}

void IconUI::MouseWheel(int x, int y)
{
  _scale += y * 0.01;
  _parent->SetDirty();
}

bool IconUI::Draw()
{
  static bool opened = false;
  const GfVec2f pos(GetX(), GetY());
  const GfVec2f size(GetWidth(), GetHeight());

  ImGui::SetNextWindowSize(size);
  ImGui::SetNextWindowPos(pos);

  ImGui::Begin(_name.c_str(), &opened, _flags);

  
  ImGuiIO& io = ImGui::GetIO();
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(pos, pos+size, ImColor(0,0,0,255));

  const float ratio = (float)GetHeight() / (float)GetWidth();
  GfVec2f minUV = _offset * 0.01;
  GfVec2f maxUV = _offset * 0.01 + GfVec2f(_scale, _scale * ratio);
  drawList->AddImage(io.Fonts->TexID, pos, pos+size, minUV, maxUV);

  ImGui::End();

  return ImGui::IsAnyItemActive() || ImGui::IsAnyItemFocused() || ImGui::IsAnyItemHovered();
};
  

JVR_NAMESPACE_CLOSE_SCOPE