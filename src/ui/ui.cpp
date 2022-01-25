#include "../ui/ui.h"
#include "../ui/head.h"
#include "../app/window.h"
#include "../app/view.h"


PXR_NAMESPACE_OPEN_SCOPE

// constructor
BaseUI::BaseUI(View* parent, const std::string& name, bool popup)
  : _parent(parent) 
  , _name(name)
  , _initialized(false)
  , _interacting(false)
{
  if(_parent && !popup)
  {
    _parent->SetContent(this);
    _parent->AddChild(this);
    _parent->SetFlag(View::LEAF);
  }
  pxr::TfWeakPtr<BaseUI> me(this);
  //pxr::TfNotice::Register(me, &BaseUI::OnAllNotices);
  pxr::TfNotice::Register(me, &BaseUI::OnNewSceneNotice);
  pxr::TfNotice::Register(me, &BaseUI::OnSceneChangedNotice);
  pxr::TfNotice::Register(me, &BaseUI::OnSelectionChangedNotice);
};

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
  outX = inX - parentX;
  outY = inY - parentY;
}

// parent window
Window* BaseUI::GetWindow(){return _parent->GetWindow();};

// parent window height
int BaseUI::GetWindowHeight(){
  return _parent->GetWindow()->GetHeight();
};
//void BaseUI::SetWindowContext(){_parent->GetWindow()->SetContext();};

// ui dimensions
pxr::GfVec2f BaseUI::GetPosition()
{
  return pxr::GfVec2f(GetX(), GetY());
}

int BaseUI::GetX()
{
  return _parent->GetMin()[0] - 1;
}

int BaseUI::GetY()
{ 
  return _parent->GetMin()[1] - 1;
}

int BaseUI::GetWidth()
{ 
  return _parent->GetWidth() + 2;
}

int BaseUI::GetHeight()
{
  return _parent->GetHeight() + 2;
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

/*
bool BaseUI::DrawHead() 
{
  ImDrawList* drawList = ImGui::GetWindowDrawList();
  drawList->AddRectFilled(
    ImVec2(GetX(), _parent->GetMin()[1]),
    ImVec2(GetX() + GetWidth(), UI_HEADER_HEIGHT),
    ImColor(RANDOM_0_1, RANDOM_0_1, RANDOM_0_1, 1.f)
  );
  return true;
}
*/


PXR_NAMESPACE_CLOSE_SCOPE
