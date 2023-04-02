#include "../ui/ui.h"
#include "../ui/tab.h"
#include "../app/window.h"
#include "../app/view.h"


JVR_NAMESPACE_OPEN_SCOPE

static int GLOBAL_UI_TYPE_COUNTER[UIType::COUNT];


// constructor
BaseUI::BaseUI(View* parent, short type, bool popup)
  : _parent(parent) 
  , _type(type)
  , _name(ComputeUniqueName(type))
  , _initialized(false)
  , _interacting(false)
{
  if (_parent && !popup)
  {
    _parent->AddUI(this);
    _parent->SetCurrent(this);
    _parent->SetFlag(View::LEAF);
    _parent->SetDirty();
  }

  pxr::TfWeakPtr<BaseUI> me(this);
  //pxr::TfNotice::Register(me, &BaseUI::OnAllNotices);
  pxr::TfNotice::Register(me, &BaseUI::OnNewSceneNotice);
  pxr::TfNotice::Register(me, &BaseUI::OnSceneChangedNotice);
  pxr::TfNotice::Register(me, &BaseUI::OnSelectionChangedNotice);
  pxr::TfNotice::Register(me, &BaseUI::OnAttributeChangedNotice);
};

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

void BaseUI::OnAllNotices(const pxr::TfNotice& n)
{
}

// mouse positon relative to the view
void BaseUI::GetRelativeMousePosition(const float inX, const float inY, 
  float& outX, float& outY)
{
  pxr::GfVec2f parentPosition = _parent->GetMin();
  float parentX = parentPosition[0];
  float parentY = parentPosition[1];
  if (_parent->GetTab()) parentY += _parent->GetTab()->GetHeight();
  outX = inX - parentX;
  outY = inY - parentY;
}

void BaseUI::DiscardEventsIfMouseInsideBox(const pxr::GfVec2f& min, const pxr::GfVec2f& max)
{
  const pxr::GfVec2f mousePos = ImGui::GetMousePos() - _parent->GetMin();
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

// ui dimensions
pxr::GfVec2f BaseUI::GetPosition()
{
  return pxr::GfVec2f(GetX(), GetY());
}

pxr::GfVec2f BaseUI::GetSize()
{
  return pxr::GfVec2f(GetWidth(), GetHeight());
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
  const pxr::GfVec2i min = pxr::GfVec2i(g.IO.MousePos.x, g.IO.MousePos.y) +
    pxr::GfVec2i(16 * g.Style.MouseCursorScale, 8 * g.Style.MouseCursorScale);
  ImVec2 size(ImGui::CalcTextSize(tooltip));
  GetWindow()->DirtyViewsUnderBox(min, pxr::GfVec2i(size.x, size.y));
}

JVR_NAMESPACE_CLOSE_SCOPE
