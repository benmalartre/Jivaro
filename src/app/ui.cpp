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
    _parent->SetFlag(View::LEAF);
    _parent->SetFlag(View::DIRTY);
  }
};

// mouse positon relative to the view
void BaseUI::GetRelativeMousePosition(const float inX, const float inY, float& outX, float& outY)
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
int BaseUI::GetWindowHeight(){return _parent->GetWindow()->GetHeight();};
//void BaseUI::SetWindowContext(){_parent->GetWindow()->SetContext();};

// ui dimensions
ImVec2 BaseUI::GetPosition()
{
  if (_docked)return _parent->GetMin();
  else return ImGui::GetWindowPos();
}

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

Application* BaseUI::GetApplication()
{
  return _parent->GetWindow()->GetApplication();
}

AMN_NAMESPACE_CLOSE_SCOPE
