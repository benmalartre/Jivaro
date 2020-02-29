#include "ui.h"
#include "window.h"
#include "view.h"


namespace AMN {

  UI::UI(View* parent, const std::string& name):
    _parent(parent), _name(name)
  {
  };

  void
  UI::GetRelativeMousePosition(const int inX, const int inY, int& outX, int& outY)
  {
    pxr::GfVec2i parentPosition = _parent->GetMin();
    int parentX = parentPosition[0];
    int parentY = parentPosition[1];
    outX = inX - parentX;
    outY = inY - parentY;
  }

  float 
  UI::GetWindowHeight()
  {
    return _parent->GetWindow()->GetHeight();
  }

  void
  UI::SetWindowContext()
  {
    _parent->GetWindow()->SetContext();
  }
} // namespace AMN