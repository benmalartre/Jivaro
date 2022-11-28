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
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoBackground;

// Popup constructor
//----------------------------------------------------------------------------
PopupUI::PopupUI(int x, int y, int w, int h)
  : BaseUI(NULL, UIType::POPUP, true)
  , _x(x)
  , _y(y)
  , _width(w)
  , _height(h)
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


static double _TouchEdge(const pxr::GfVec2f& p, const pxr::GfVec2f& min, 
  const pxr::GfVec2f& max, double eps=2.0)
{
  const double l = p[0] - min[0];
  const double r = max[0] - p[0];
  const double t = p[1] - min[1];
  const double b = max[1] - p[1];

  const double closest = std::min(l, std::min(r, std::min(t, b)));
  return closest < eps;
}

void 
PopupUI::MouseButton(int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(GetWindow()->GetGlfwWindow(), &x, &y);
  if (action == GLFW_PRESS) {
    if (x < _x && y < _y && x >(_x + _width) && y >(_y + _height)) {
      _cancel = true;
    }
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

  ImGui::Begin(_name.c_str(), NULL, _flags);

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

bool ColorPopupUI::Terminate()
{
  if (_done) {
    if (_isArray) {
      pxr::VtArray<pxr::GfVec3f> result;
      result = { _original };
      _attribute.Set(result, pxr::UsdTimeCode::Default());
      result = { _color };
      UndoBlock editBlock(true);
      _attribute.Set(result, pxr::UsdTimeCode::Default());
    }
    else {
      _attribute.Set(_original, pxr::UsdTimeCode::Default());
      UndoBlock editBlock(true);
      _attribute.Set(_color, pxr::UsdTimeCode::Default());
    }
    AttributeChangedNotice().Send();
  }
  return _done;
}

void
ColorPopupUI::MouseButton(int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(GetWindow()->GetGlfwWindow(), &x, &y);
  if (!(x >= _x && y >= _y && x <= (_x + _width) && y <= (_y + _height))) {
    _done = true;
    Terminate();
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
    Terminate();
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
  _sync = false;
  memset(&_value[0], 0, 255 * sizeof(char));
}

NamePopupUI::NamePopupUI(int x, int y, int width, int height, const std::string& name)
  : PopupUI(x, y, width, height)
{
  _sync = false;
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
  static bool initialized = false;
  ImGui::Begin(_name.c_str(), NULL, _flags);

  if (!initialized) {
    ImGui::SetWindowSize(pxr::GfVec2f(_width, _height));
    ImGui::SetWindowPos(pxr::GfVec2f(_x, _y));
    initialized = true;
  }

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
  BuildNodeList();
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
  _nodes.push_back("twist");
  _nodes.push_back("perlin");
  _nodes.push_back("wire");
  _nodes.push_back("curve");
  _nodes.push_back("warp");
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
  ImGui::Begin(_name.c_str(), NULL, _flags);
  ImGui::SetWindowSize(pxr::GfVec2f(_width, _height));
  ImGui::SetWindowPos(pxr::GfVec2f(_x, _y));


  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const ImGuiStyle& style = ImGui::GetStyle();
  
  drawList->AddRectFilled(
    _parent->GetMin(), _parent->GetMax(), ImColor(style.Colors[ImGuiCol_ChildBg]));

  drawList = ImGui::GetForegroundDrawList();
  
  if (!strcmp(_filter, "")) {
    _filteredNodes = _nodes;
  } else {
    _FilterNodes();
  }

  size_t idx = 0;
  for(auto& node : _filteredNodes) {
    std::cout << node.c_str() << std::endl;
    if (idx == _i) {
      ImGui::PushFont(GetWindow()->GetFont(1));
      ImGui::TextColored(ImVec4(1.0,1.0,1.0,1.0), node.c_str());
    } else {
      ImGui::PushFont(GetWindow()->GetFont(1));
      ImGui::TextColored(ImVec4(0.75,0.75,0.75,1.0), node.c_str());
    }
    ImGui::PopFont();
    idx++;
  }

  ImGui::TextColored(ImVec4(0, 0, 1, 1), _filter);
  ImGui::SameLine();

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
// SdfPathPopupUI
//===========================================================================================
SdfPathPopupUI::SdfPathPopupUI(int x, int y, int width, int height,
  const pxr::SdfPrimSpecHandle& primSpec)
  : PopupUI(x, y, width, height)
  , _primSpec(primSpec)
{
  
}

SdfPathPopupUI::~SdfPathPopupUI()
{
}

bool
SdfPathPopupUI::Draw()
{
  bool opened;

  ImGui::Begin(_name.c_str(), &opened, _flags);

  ImGui::SetWindowSize(pxr::GfVec2f(_width, _height));
  ImGui::SetWindowPos(pxr::GfVec2f(_x, _y));

  // TODO: We will probably want to browse in the scene hierarchy to select the path
    //   create a selection tree, one day
  ImGui::Text("%s", _primSpec->GetPath().GetString().c_str());
  if (ImGui::BeginCombo("Operation", GetListEditorOperationName(_operation))) {
    for (int n = 0; n < GetListEditorOperationSize(); n++) {
      if (ImGui::Selectable(GetListEditorOperationName(n))) {
        _operation = n;
      }
    }
    ImGui::EndCombo();
  }
  ImGui::InputText("Target prim path", &_primPath);

  ImGui::End();
  return true;
};

/*
/// Create a standard UI for entering a SdfPath.
/// This is used for inherit and specialize
struct SdfPathPopupUI : public ModalDialog {

  CreateSdfPathModalDialog(const SdfPrimSpecHandle& primSpec) : _primSpec(primSpec) {};
  ~CreateSdfPathModalDialog() override {}

  void Draw() override {
    if (!_primSpec) {
      CloseModal();
      return;
    }
    // TODO: We will probably want to browse in the scene hierarchy to select the path
    //   create a selection tree, one day
    ImGui::Text("%s", _primSpec->GetPath().GetString().c_str());
    if (ImGui::BeginCombo("Operation", GetListEditorOperationName(_operation))) {
      for (int n = 0; n < GetListEditorOperationSize(); n++) {
        if (ImGui::Selectable(GetListEditorOperationName(n))) {
          _operation = n;
        }
      }
      ImGui::EndCombo();
    }
    ImGui::InputText("Target prim path", &_primPath);
    DrawOkCancelModal([=]() { OnOkCallBack(); });
  }

  virtual void OnOkCallBack() = 0;

  const char* DialogId() const override { return "Sdf path"; }

  SdfPrimSpecHandle _primSpec;
  std::string _primPath;
  int _operation = 0;
};
*/

JVR_NAMESPACE_CLOSE_SCOPE