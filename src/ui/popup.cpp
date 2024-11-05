#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>

#include "../ui/ui.h"
#include "../ui/popup.h"
#include "../ui/splitter.h"
#include "../utils/timer.h"
#include "../utils/keys.h"
#include "../utils/usd.h"
#include "../utils/strings.h"
#include "../command/block.h"
#include "../command/manager.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/application.h"
#include "../app/commands.h"

JVR_NAMESPACE_OPEN_SCOPE

ImGuiWindowFlags PopupUI::_flags = 
  ImGuiWindowFlags_NoTitleBar |
  ImGuiWindowFlags_NoScrollbar |
  ImGuiWindowFlags_NoMove |
  ImGuiWindowFlags_NoResize;

// Popup constructor
//----------------------------------------------------------------------------
PopupUI::PopupUI(const std::string &name, int x, int y, int w, int h)
  : BaseUI(NULL, UIType::POPUP, true)
  , _name(name)
  , _x(x)
  , _y(y)
  , _width(w)
  , _height(h)
  , _done(false)
  , _sync(false)
  , _cancel(false)
{
  _parent = WindowRegistry::GetActiveWindow()->GetMainView();
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


static double _TouchEdge(const GfVec2f& p, const GfVec2f& min, 
  const GfVec2f& max, double eps=PopupUI::Sensitivity)
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
    if (x < (_x - Sensitivity) && y < (_y - Sensitivity) && 
      x >(_x + _width + Sensitivity) && y >(_y + _height + Sensitivity)) {
      _cancel = true;
    }
    std::cout << "------------------------------" << std::endl;
    if(_TouchEdge(GfVec2f(x, y), GfVec2f(_x, _y), GfVec2f(_x+_width, _y+_height)))
        std::cout << "Touch Fuckin Edge" << std::endl;

    if (button == GLFW_MOUSE_BUTTON_LEFT)_drag = true;
  }
  else if (action == GLFW_RELEASE) {
    _drag = false;
  } 
}

void
PopupUI::MouseMove(int x, int y)
{
  if(_TouchEdge(GfVec2f(x, y), GfVec2f(_x, _y), GfVec2f(_x+_width, _y+_height)))
        std::cout << "Touch Fuckin Edge" << std::endl;
  if (_drag) {

}
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

  ImGui::SetNextWindowSize(GfVec2f(_width, _height));
  ImGui::SetNextWindowPos(GfVec2f(_x, _y));

  ImGui::Begin(_name.c_str(), NULL, _flags);

  GfVec4f color(
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
  const UsdAttribute& attribute, const UsdTimeCode& timeCode)
  : PopupUI("Color Chooser", x, y, width, height)
  , _attribute(attribute)
  , _time(timeCode)
  , _isArray(false)
{
  _sync = true;
  _dimmer = false;
  VtValue value;
  attribute.Get(&value, timeCode);
  if (value.IsHolding<GfVec3f>()) {
    _color = _original = GfVec3f(value.Get<GfVec3f>());
  }
  else if (value.IsArrayValued() &&
    value.GetArraySize() == 1 &&
    value.IsHolding<VtArray<GfVec3f>>()) {
    VtArray<GfVec3f> array = value.Get<VtArray<GfVec3f>>();
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
      VtArray<GfVec3f> value = { _color };
      VtArray<GfVec3f> previous = { _original };
      UsdAttributeVector attributes = { _attribute };
      ADD_COMMAND(SetAttributeCommand, attributes, 
        VtValue(value), VtValue(previous), UsdTimeCode::Default());
    }
    else {
      UsdAttributeVector attributes = { _attribute };
       ADD_COMMAND(SetAttributeCommand, attributes,
         VtValue(_color), VtValue(_original), UsdTimeCode::Default());
    }
  }
  else {
    if (_isArray) {
      _attribute.Set(VtArray<GfVec3f>({ _color }), 
        UsdTimeCode::Default());
    }
    else {
      _attribute.Set(_color, UsdTimeCode::Default());
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

  ImGui::SetNextWindowSize(GfVec2f(_width, _height));
  ImGui::SetNextWindowPos(GfVec2f(_x, _y));

  ImGui::Begin(_name.c_str(), &opened, _flags);

  

  ImDrawList* drawList = ImGui::GetBackgroundDrawList();
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
// InputPopupUI
//===========================================================================================
InputPopupUI::InputPopupUI(int x, int y, int width, int height, Callback callback)
  : PopupUI("inputpopup", x, y, width, height)
  , _callback(callback)
{
  _sync = false;
  _dimmer = false;
  memset(&_value[0], 0, 255 * sizeof(char));
}

InputPopupUI::InputPopupUI(int x, int y, int width, int height, Callback callback, const std::string& value)
  : PopupUI("inputpopup", x, y, width, height)
  , _callback(callback)
{
  _sync = false;
  _dimmer = false;
  strcpy(&_value[0], value.c_str());
}

InputPopupUI::~InputPopupUI()
{
}

void 
InputPopupUI::SetName(const std::string& name)
{
  strcpy(&_value[0], name.c_str());
}

bool
InputPopupUI::Draw()
{
  ImGui::SetNextWindowSize(GfVec2f(_width, _height));
  ImGui::SetNextWindowPos(GfVec2f(_x, _y));

  ImGui::Begin(_name.c_str(), NULL, _flags);

  ImGui::SetNextItemWidth(_width);
  ImGui::InputText("##input", &_value[0], 255);
  if (!_initialized) {
    ImGui::SetKeyboardFocusHere(-1);
    _initialized = true;
  }

  ImGui::End();
  return true;
};

bool 
InputPopupUI::Terminate()
{
  if(_callback)
    _callback(TfToken(_value));
  return true;
}



//===========================================================================================
// ListPopupUI
//===========================================================================================
ListPopupUI::ListPopupUI(const char* name, int x, int y, int width, int height, Callback callback)
  : PopupUI(name, x, y, width, height)
  , _p(0)
  , _i(0)
  , _callback(callback)
{
  _sync = false;
  memset(&_filter[0], (char)0, 256 * sizeof(char));
  _BuildList();
}

ListPopupUI::~ListPopupUI()
{
}

void
ListPopupUI::_BuildList()
{

  for(auto& specTypeName: GetAllSpecTypeNames() )
    _items.push_back(specTypeName);
  /*
  _items.push_back("wrap");
  _items.push_back("collide");
  _items.push_back("skin");
  _items.push_back("bend");
  _items.push_back("twist");
  _items.push_back("perlin");
  _items.push_back("wire");
  _items.push_back("curve");
  _items.push_back("warp");
  */
}

void
ListPopupUI::MouseButton(int button, int action, int mods)
{
  double x, y;
  glfwGetCursorPos(GetWindow()->GetGlfwWindow(), &x, &y);

  if(x < _x || x > (_x + _width) || y < _y || y > (_y + _height))
    _cancel = true;
  else if(button == GLFW_MOUSE_BUTTON_RIGHT)
    _done = true;
}


void 
ListPopupUI::Keyboard(int key, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS || action == GLFW_REPEAT) {
    switch (GetMappedKey(key)) {
    case GLFW_KEY_BACKSPACE:
      _filter[--_p] = (char)0;
      break;
    case GLFW_KEY_DOWN:
      _i++;
      if (_i >= _filteredItems.size()) _i = 0;
      break;
    case GLFW_KEY_UP:
      _i--;
      if (_i < 0) _i = _filteredItems.size() - 1;
      break;
    case GLFW_KEY_ENTER:
      if(_filteredItems.size()){
        const std::string name = _filteredItems[_i];
        std::cout << "NodePopupUI result : " << name << std::endl;
        if(_callback)_callback(TfToken(name));
      }
      _done = true;
      break;
    }
  }
}

void
ListPopupUI::Input(int key)
{
  _filter[_p++] = ConvertCodePointToUtf8(key).c_str()[0];
}

static bool _FilterItemName(const std::string& name, const char* filter)
{
  return CountString(name, (std::string)filter) > 0;
}

void
ListPopupUI::_FilterItems()
{
  _filteredItems.clear();
  for (auto& node : _items) {
    if (_FilterItemName(node, _filter)) {
      _filteredItems.push_back(node);
    }
  }
}

bool
ListPopupUI::Draw()
{
  const ImGuiStyle& style = ImGui::GetStyle();

  if (!strcmp(_filter, "")) {
    _filteredItems = _items;
  } else {
    _FilterItems();
  }

  ImGui::SetNextWindowPos(GfVec2f(_x, _y));
  ImGui::SetNextWindowSize(GfVec2f(_width, _height));

  ImGui::Begin(_name.c_str(), NULL, _flags);

  ImDrawList* drawList = ImGui::GetForegroundDrawList();

  
  ImGui::PushFont(GetWindow()->GetFont(FONT_MEDIUM, 0));
  ImGui::TextColored(ImVec4(1, 0.5, 0.25, 1.0),_name.c_str());
  ImGui::PopFont();

  ImGui::SameLine();
  ImGui::InputText("##filter", _filter, IM_ARRAYSIZE(_filter));

  ImGui::PushFont(GetWindow()->GetFont(FONT_MEDIUM, 0));
  if (ImGui::BeginListBox("##listbox", ImVec2(_width, _height - ImGui::GetFontSize())))
  {
    for (int n = 0; n < _filteredItems.size(); ++n)
    {
      const bool isSelected = (_i == n);
      if (ImGui::Selectable(_filteredItems[n].c_str(), isSelected, ImGuiSelectableFlags_AllowDoubleClick)) {
        _i = n;
        if (ImGui::IsMouseDoubleClicked(0)) {
          _done = true;
          std::cout << "NodePopupUI result : " << _filteredItems[n] << std::endl;
           if(_callback)_callback(TfToken(_filteredItems[n]));
        }
      }

      // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
      if (isSelected)
        ImGui::SetItemDefaultFocus();
    }
    ImGui::EndListBox();
  }
  
  
  ImGui::PopFont();


  ImGui::End();
  return true;

};



JVR_NAMESPACE_CLOSE_SCOPE