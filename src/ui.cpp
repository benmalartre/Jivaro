#include "ui.h"
#include "window.h"
#include "view.h"


namespace AMN {
  // constructor
  UI::UI(View* parent, const std::string& name):
    _parent(parent), _name(name)
  {
  };

  // mouse positon relative to the view
  void UI::GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY)
  {
    pxr::GfVec2i parentPosition = _parent->GetMin();
    int parentX = parentPosition[0];
    int parentY = parentPosition[1];
    outX = inX - parentX;
    outY = inY - parentY;
  }

  // parent window
  int UI::GetWindowHeight(){return _parent->GetWindow()->GetHeight();};
  //void UI::SetWindowContext(){_parent->GetWindow()->SetContext();};

  // ui dimensions
  int UI::GetX(){return _parent->GetMin()[0];};
  int UI::GetY(){return GetWindowHeight() - (_parent->GetMin()[1] + _parent->GetHeight());};
  int UI::GetWidth(){return _parent->GetWidth();};
  int UI::GetHeight(){return _parent->GetHeight();};

} // namespace AMN