#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>

#include "../ui/ui.h"
#include "../ui/popup.h"
#include "../ui/splitter.h"
#include "../utils/timer.h"
#include "../utils/keys.h"
#include "../utils/strings.h"
#include "../command/block.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/application.h"


PXR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags PopupUI::_flags = 
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoBackground;

// Popup constructor
//----------------------------------------------------------------------------
PopupUI::PopupUI(int x, int y, int w, int h)
  : BaseUI(NULL, "##popup_" + std::to_string(CurrentTime()), true)
  , _x(x)
  , _y(y)
  , _width(w)
  , _height(h)
  , _done(false)
  , _sync(false)
{
  _parent = GetApplication()->GetActiveWindow()->GetMainView();
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
  if (x >= _x && y >= _y && x <= (_x + _width) && y <= (_y + _height)) {
    std::cout << "POPUP MOUSE BUTTON : (INSIDE) = " << x << "," << y << std::endl;
  }
  else {
    std::cout << "POPUP MOUSE BUTTON : (OUTSIDE) = " << x << "," << y << std::endl;
  }
  _done = true;
  //MouseButton(button, action, mods);
}

void
PopupUI::MouseMove(int x, int y)
{
}

void 
PopupUI::Keyboard(int key, int scancode, int action, int mods)
{
}

bool 
PopupUI::Draw()
{
  bool opened;

  ImGui::Begin(_name.c_str(), &opened, _flags);

  ImGui::SetWindowSize(pxr::GfVec2f(_width, _height));
  ImGui::SetWindowPos(pxr::GfVec2f(_x, _y));

  pxr::GfVec4f color(
    RANDOM_0_1,
    RANDOM_0_1,
    RANDOM_0_1, 
    1.f
  );

  ImDrawList* drawList = ImGui::GetForegroundDrawList();
  drawList->AddRectFilled(
    ImVec2(_x, _y),
    ImVec2(_x + _width, _y + _height),
    ImColor(color[0], color[1], color[2], color[3])
  );
  ImGui::End();
  return true;
};

//===========================================================================================
// ColorPopupUI
//===========================================================================================
ColorPopupUI::ColorPopupUI(int x, int y, int width, int height, 
  const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode)
  : PopupUI(x, y, width, height)
  , _attribute(attribute)
  , _time(timeCode)
  , _isArray(false)
{
  _sync = true;
  pxr::VtValue value;
  attribute.Get(&value, timeCode);
  if (value.IsHolding<pxr::GfVec3f>()) {
    _color = _original = pxr::GfVec3f(value.Get<pxr::GfVec3f>());
  }
  else if (value.IsArrayValued() &&
    value.GetArraySize() == 1 &&
    value.IsHolding<pxr::VtArray<pxr::GfVec3f>>()) {
    pxr::VtArray<pxr::GfVec3f> array = value.Get<pxr::VtArray<pxr::GfVec3f>>();
    _color = _original = array[0];
    _isArray = true;
  }
}

ColorPopupUI::~ColorPopupUI()
{
}


void
ColorPopupUI::MouseButton(int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(GetWindow()->GetGlfwWindow(), &x, &y);
  if (!(x >= _x && y >= _y && x <= (_x + _width) && y <= (_y + _height))) {
    if (_isArray) {
      pxr::VtArray<pxr::GfVec3f> result;
      result = { _original };
      _attribute.Set(result, _time);
      result = { _color };
      UndoBlock editBlock(true);
      _attribute.Set(result, _time);
    }
    else {
      _attribute.Set(_original, _time);
      UndoBlock editBlock(true);
      _attribute.Set(_color, _time);
    }
    _done = true;
  }
}

bool
ColorPopupUI::Draw()
{
  bool opened;

  ImGui::Begin(_name.c_str(), &opened, _flags);

  ImGui::SetWindowSize(pxr::GfVec2f(_width, _height));
  ImGui::SetWindowPos(pxr::GfVec2f(_x, _y));

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    ImVec2(_x, _y),
    ImVec2(_x + _width, _y + _height),
    ImColor(BACKGROUND_COLOR)
  );

  static ImGuiColorEditFlags picker_flags = 
    ImGuiColorEditFlags_HDR | 
    ImGuiColorEditFlags_NoDragDrop |
    ImGuiColorEditFlags_AlphaPreviewHalf | 
    ImGuiColorEditFlags_NoOptions;

  ImGui::SetNextItemWidth(_width);
  ImGui::ColorPicker4("##picker", (float*)&_color, picker_flags);
  if(_color != _original) {
    if (_isArray) {
      pxr::VtArray<pxr::GfVec3f> result;
      result = { _color };
      _attribute.Set(result, _time);
    }
    else {
      _attribute.Set(_original, _time);
      _attribute.Set(_color, _time);
    }
  }
 
  ImGui::End();
  return true;
};

//===========================================================================================
// NodePopupUI
//===========================================================================================
NodePopupUI::NodePopupUI(int x, int y, int width, int height)
  : PopupUI(x, y, width, height)
{
  _sync = true;
}

NodePopupUI::~NodePopupUI()
{
}

void
NodePopupUI::BuildNodeList()
{
  _nodes.push_back("wrap");
  _nodes.push_back("collide");
  _nodes.push_back("skin");
  _nodes.push_back("bend");
}

void
NodePopupUI::MouseButton(int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(GetWindow()->GetGlfwWindow(), &x, &y);
  std::cout << x << "," << y << std::endl;
  /*
  if (!(x >= _x && y >= _y && x <= (_x + _width) && y <= (_y + _height))) {
    if (_isArray) {
      pxr::VtArray<pxr::GfVec3f> result;
      result = { _original };
      _attribute.Set(result, _time);
      result = { _color };
      UndoBlock editBlock(true);
      _attribute.Set(result, _time);
    }
    else {
      _attribute.Set(_original, _time);
      UndoBlock editBlock(true);
      _attribute.Set(_color, _time);
    }
    _done = true;
  }
  */
  if(button == GLFW_MOUSE_BUTTON_RIGHT)
    _done = true;
}


void
NodePopupUI::Input(int key)
{
  _filter += ConvertCodePointToUtf8(key);
  std::cout << "FILTER : " << _filter << std::endl;
}

bool
NodePopupUI::Draw()
{
  bool opened;

  ImGui::Begin(_name.c_str(), &opened, _flags);

  ImGui::SetWindowSize(pxr::GfVec2f(_width, _height));
  ImGui::SetWindowPos(pxr::GfVec2f(_x, _y));

  ImDrawList* drawList = ImGui::GetForegroundDrawList();
  ImGui::PushFont(GetWindow()->GetMediumFont(2));
  for(auto& node : _nodes) {
    ImGui::Text(node.c_str());
  }

  ImGui::InputText("##filter", (char*)_filter.c_str(), _filter.capacity() + 1);
  if (ImGui::IsItemEdited()) {
    std::cout << "FILTER : " << _filter << std::endl;
  }

  ImGui::End();
  return true;
};

PXR_NAMESPACE_CLOSE_SCOPE