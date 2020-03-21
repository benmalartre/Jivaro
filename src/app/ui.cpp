#include "ui.h"
#include "window.h"
#include "view.h"

AMN_NAMESPACE_OPEN_SCOPE

// constructor
BaseUI::BaseUI(View* parent, const std::string& name, bool docked):
  _parent(parent), _name(name), _docked(docked)
{
  if(_parent)
  {
    _parent->SetContent(this);
    _parent->SetLeaf();
  }
};

// mouse positon relative to the view
void BaseUI::GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY)
{
  pxr::GfVec2f parentPosition = _parent->GetMin();
  int parentX = parentPosition[0];
  int parentY = parentPosition[1];
  outX = inX - parentX;
  outY = inY - parentY;
}

// parent window
int BaseUI::GetWindowHeight(){return _parent->GetWindow()->GetHeight();};
//void BaseUI::SetWindowContext(){_parent->GetWindow()->SetContext();};

// ui dimensions
int BaseUI::GetX()
{
  if(_docked)return _parent->GetMin()[0];
  else return ImGui::GetWindowPos().x;
};

int BaseUI::GetY()
{ 
  if(_docked)return GetWindowHeight() - (_parent->GetMin()[1] + _parent->GetHeight());
  else return ImGui::GetWindowPos().y;
};

int BaseUI::GetWidth()
{ 
  if(_docked)return _parent->GetWidth();
  else return ImGui::GetWindowSize().x;
};

int BaseUI::GetHeight()
{
  if(_docked)return _parent->GetHeight();
  else return ImGui::GetWindowSize().y;
};

AMN_NAMESPACE_CLOSE_SCOPE
