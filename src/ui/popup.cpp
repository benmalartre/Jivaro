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


JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags PopupUI::_flags = 
  ImGuiWindowFlags_NoResize |
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoBackground;

// Popup constructor
//----------------------------------------------------------------------------
PopupUI::PopupUI(int x, int y, int w, int h)
  : BaseUI(NULL, UIType::POPUP, true)
  , _x(x)
  , _y(y)
  , _width(w)
  , _height(h)
  , _initialized(false)
  , _done(false)
  , _sync(false)
  , _cancel(false)
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
  if (x < _x && y < _y && x > (_x + _width) && y > (_y + _height)) {
    _cancel = true;
  }
}

void
PopupUI::MouseMove(int x, int y)
{
}

void 
PopupUI::Keyboard(int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS) {
    switch (GetMappedKey(key)) {
    case GLFW_KEY_ENTER:
      _done = true;
      break;
    case GLFW_KEY_ESCAPE:
      _cancel = true;
      break;
    }
  }
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
  const ImGuiStyle& style = ImGui::GetStyle();
  ImGui::Begin(_name.c_str(), &opened, _flags);

  ImGui::SetWindowSize(pxr::GfVec2f(_width, _height));
  ImGui::SetWindowPos(pxr::GfVec2f(_x, _y));

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    ImVec2(_x, _y),
    ImVec2(_x + _width, _y + _height),
    ImColor(style.Colors[ImGuiCol_WindowBg])
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
// NamePopupUI
//===========================================================================================
NamePopupUI::NamePopupUI(int x, int y, int width, int height)
  : PopupUI(x, y, width, height)
{
  _sync = true;
  memset(&_value[0], 0, 255 * sizeof(char));
}

NamePopupUI::NamePopupUI(int x, int y, int width, int height, const std::string& name)
  : PopupUI(x, y, width, height)
{
  _sync = true;
  strcpy(&_value[0], name.c_str());
}

NamePopupUI::~NamePopupUI()
{
}

void 
NamePopupUI::SetName(const std::string& name)
{
  strcpy(&_value[0], name.c_str());
}

bool
NamePopupUI::Draw()
{
  bool opened;
  ImGui::Begin(_name.c_str(), &opened, _flags);

  ImGui::SetWindowSize(pxr::GfVec2f(_width, _height));
  ImGui::SetWindowPos(pxr::GfVec2f(_x, _y));

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const ImGuiStyle& style = ImGui::GetStyle();
  drawList->AddRectFilled(
    _parent->GetMin(), _parent->GetMax(), ImColor(style.Colors[ImGuiCol_WindowBg]));

  drawList = ImGui::GetForegroundDrawList();

  ImGui::SetCursorPos(ImVec2(20, 20));
  ImGui::Text("Name : ");
  ImGui::SameLine();

  ImGui::InputText("##name", &_value[0], 255);
  if (!_initialized) {
    ImGui::SetKeyboardFocusHere(-1);
    _initialized = true;
  }

  if (ImGui::Button("OK", ImVec2(GetWidth() / 3, 32))) {
    _done = true;
  }
  ImGui::SameLine();
  if (ImGui::Button("Cancel", ImVec2(GetWidth() / 3, 32))) {
    _cancel = true;
  }

  ImGui::End();
  return true;
};



//===========================================================================================
// NodePopupUI
//===========================================================================================
NodePopupUI::NodePopupUI(int x, int y, int width, int height)
  : PopupUI(x, y, width, height)
  , _p(0)
  , _i(0)
{
  _sync = true;
  memset(&_filter[0], (char)0, NODE_FILTER_SIZE * sizeof(char));
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
      UndoBlock editBlock;
      _attribute.Set(result, _time);
    }
    else {
      _attribute.Set(_original, _time);
      UndoBlock editBlock;
      _attribute.Set(_color, _time);
    }
    _done = true;
  }
  */
  if(button == GLFW_MOUSE_BUTTON_RIGHT)
    _done = true;
}


void 
NodePopupUI::Keyboard(int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    switch (GetMappedKey(key)) {
    case GLFW_KEY_BACKSPACE:
      _filter[--_p] = (char)0;
      break;
    case GLFW_KEY_DOWN:
      _i++;
      if (_i >= _filteredNodes.size()) _i = 0;
      break;
    case GLFW_KEY_UP:
      _i--;
      if (_i < 0) _i = _filteredNodes.size() - 1;
      break;
    case GLFW_KEY_ENTER:
      const std::string name = _filteredNodes[_i];
      std::cout << "INSTANCIATE NODE : " << name << std::endl;
      _done = true;
      break;
    }
  }
}

void
NodePopupUI::Input(int key)
{
  _filter[_p++] = ConvertCodePointToUtf8(key).c_str()[0];
}

static bool _FilterNodeName(const std::string& name, const char* filter)
{
  return CountString(name, (std::string)filter) > 0;
}

void
NodePopupUI::_FilterNodes()
{
  _filteredNodes.clear();
  for (auto& node : _nodes) {
    if (_FilterNodeName(node, _filter)) {
      _filteredNodes.push_back(node);
    }
  }
}

bool
NodePopupUI::Draw()
{
  bool opened;

  ImGui::Begin(_name.c_str(), &opened, _flags);

  ImGui::SetWindowSize(pxr::GfVec2f(_width, _height));
  ImGui::SetWindowPos(pxr::GfVec2f(_x, _y));

  ImDrawList* drawList = ImGui::GetForegroundDrawList();
  
  if (!strcmp(_filter, "")) {
    _filteredNodes = _nodes;
  } else {
    _FilterNodes();
  }
  size_t idx = 0;
  for(auto& node : _filteredNodes) {
    if (idx == _i) {
      //ImGui::PushFont(GetWindow()->GetBoldFont(2));
      ImGui::TextColored(ImVec4(1, 0, 0, 1), node.c_str());
    } else {
      //ImGui::PushFont(GetWindow()->GetMediumFont(2));
      ImGui::TextColored(ImVec4(0, 1, 0, 1), node.c_str());
    }
    idx++;
  }

  ImGui::TextColored(ImVec4(0, 0, 1, 1), _filter);
  ImGui::SameLine();

  ImGui::End();
  return true;
};

JVR_NAMESPACE_CLOSE_SCOPE