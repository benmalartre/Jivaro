#include "ui.h"
#include "window.h"
#include "view.h"

PXR_NAMESPACE_OPEN_SCOPE

// constructor
AmnUI::AmnUI(AmnView* parent, const std::string& name):
  _parent(parent), _name(name)
{
};

// mouse positon relative to the view
void AmnUI::GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY)
{
  pxr::GfVec2i parentPosition = _parent->GetMin();
  int parentX = parentPosition[0];
  int parentY = parentPosition[1];
  outX = inX - parentX;
  outY = inY - parentY;
}

// parent window
int AmnUI::GetWindowHeight(){return _parent->GetWindow()->GetHeight();};
//void AmnUI::SetWindowContext(){_parent->GetWindow()->SetContext();};

// ui dimensions
int AmnUI::GetX(){return _parent->GetMin()[0];};
int AmnUI::GetY(){return GetWindowHeight() - (_parent->GetMin()[1] + _parent->GetHeight());};
int AmnUI::GetWidth(){return _parent->GetWidth();};
int AmnUI::GetHeight(){return _parent->GetHeight();};

PXR_NAMESPACE_CLOSE_SCOPE
