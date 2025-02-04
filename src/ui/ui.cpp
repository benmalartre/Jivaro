#include "../ui/ui.h"
#include "../ui/tab.h"
#include "../app/window.h"
#include "../app/view.h"
#include "../app/application.h"

JVR_NAMESPACE_OPEN_SCOPE

static int GLOBAL_UI_TYPE_COUNTER[UIType::COUNT];


// constructor
BaseUI::BaseUI(View* parent, short type, bool popup)
  : _parent(parent) 
  , _type(type)
  , _name(ComputeUniqueName(type))
  , _initialized(false)
  , _interacting(false)
  , _model(Application::Get()->GetModel())
{
  if (_parent && !popup)
  {
    _parent->AddUI(this);
    _parent->SetCurrentUI(this);
    _parent->SetFlag(View::LEAF);
    _parent->SetDirty();
  }

  TfWeakPtr<BaseUI> me(this);
  //TfNotice::Register(me, &BaseUI::OnAllNotices);
  TfNotice::Register(me, &BaseUI::OnNewSceneNotice);
  TfNotice::Register(me, &BaseUI::OnSceneChangedNotice);
  TfNotice::Register(me, &BaseUI::OnSelectionChangedNotice);
  TfNotice::Register(me, &BaseUI::OnAttributeChangedNotice);
  TfNotice::Register(me, &BaseUI::OnToolChangedNotice);
};

void
BaseUI::SetModel(Model* model)
{
  _model = model;
  _initialized = false;
}

std::string 
BaseUI::ComputeUniqueName(short type)
{
  if(_parent)return _parent->GetWindow()->ComputeUniqueUIName(type);
  std::string baseName = UITypeName[type];
  return baseName + std::to_string(GLOBAL_UI_TYPE_COUNTER[type]++);
}

void BaseUI::OnNewSceneNotice(const NewSceneNotice& n)
{
  _initialized = false;
}

void BaseUI::OnSelectionChangedNotice(const SelectionChangedNotice& n)
{
}

void BaseUI::OnSceneChangedNotice(const SceneChangedNotice& n)
{
}

void BaseUI::OnAttributeChangedNotice(const AttributeChangedNotice& n)
{
}

void BaseUI::OnToolChangedNotice(const ToolChangedNotice& n)
{
}

void BaseUI::OnAllNotices(const TfNotice& n)
{
}

// mouse positon relative to the view
void BaseUI::GetRelativeMousePosition(const float inX, const float inY, 
  float& outX, float& outY)
{
  GfVec2f parentPosition = _parent->GetMin();
  float parentX = parentPosition[0];
  float parentY = parentPosition[1];
  if (_parent->GetTab()) parentY += _parent->GetTab()->GetHeight();
  outX = inX - parentX;
  outY = inY - parentY;
}

void BaseUI::DiscardEventsIfMouseInsideBox(const GfVec2f& min, const GfVec2f& max)
{
  const GfVec2f mousePos = ImGui::GetMousePos() - _parent->GetMin();
  if (mousePos[0] > min[0] && mousePos[0] < max[0] && mousePos[1] > min[1] && mousePos[1] < max[1]) {
    _parent->SetFlag(View::DISCARDMOUSEBUTTON | View::DISCARDMOUSEMOVE);
  }
}

// parent window
Window* 
BaseUI::GetWindow()
{
  if(_parent)return _parent->GetWindow();
  return NULL;
};

// parent window height
int 
BaseUI::GetWindowHeight()
{
  return _parent->GetWindow()->GetHeight();
};

ImFont* 
BaseUI::GetFont(size_t size, size_t index)
{
  return _parent->GetWindow()->GetFont(size, index);
}

// ui dimensions
GfVec2f BaseUI::GetPosition()
{
  return GfVec2f(GetX(), GetY());
}

GfVec2f BaseUI::GetSize()
{
  return GfVec2f(GetWidth(), GetHeight());
}

int BaseUI::GetX()
{
  return _parent->GetMin()[0];
}

int BaseUI::GetY()
{ 
  ImGuiStyle& style = ImGui::GetStyle();
  if (_parent->GetTab()) {
    return (_parent->GetMin()[1] + _parent->GetTab()->GetHeight()) - 1;
  }
  else {
    return (_parent->GetMin()[1]) - 1;
  }
  return _parent->GetMin()[1];
}

int BaseUI::GetWidth()
{ 
  return _parent->GetWidth();
}

int BaseUI::GetHeight()
{
  ImGuiStyle& style = ImGui::GetStyle();
  if (_parent->GetTab()) {
    return (_parent->GetHeight() - _parent->GetTab()->GetHeight()) + 2;
  }
  else {
    return _parent->GetHeight() + 2;
  }
}

void 
BaseUI::SetInteracting(bool state)
{
  if(state) {
    _interacting = true;
    _parent->SetFlag(View::INTERACTING);
  } else {
    _interacting = false;
   _parent->ClearFlag(View::INTERACTING);
  }
}

void BaseUI::AttachTooltip(const char* tooltip)
{
  ImGui::SetTooltip("%s", tooltip);
  ImGuiContext& g = *GImGui;
  const GfVec2i min = GfVec2i(g.IO.MousePos.x, g.IO.MousePos.y) +
    GfVec2i(16 * g.Style.MouseCursorScale, 8 * g.Style.MouseCursorScale);
  ImVec2 size(ImGui::CalcTextSize(tooltip));
  GetWindow()->DirtyViewsUnderBox(min, GfVec2i(size.x, size.y));
}


ViewEventData 
BaseUI::_MouseButtonEventData(int button, int action, int mods, int x, int y)
{
  ViewEventData data;
  data.type = ViewEventData::MOUSE_BUTTON;
  data.button = button;
  data.action = action;
  data.mods = mods;
  data.x = x - GetX();
  data.y = y - GetY();
  data.width = GetWidth();
  data.height = GetHeight();
  return data;
}

ViewEventData 
BaseUI::_MouseMoveEventData(int x, int y)
{
  ViewEventData data;
  data.type = ViewEventData::MOUSE_MOVE;

  data.x = x - GetX();
  data.y = y - GetY();
  data.width = GetWidth();
  data.height = GetHeight();
  return data;
}

JVR_NAMESPACE_CLOSE_SCOPE
