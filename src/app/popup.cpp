#include "popup.h"
#include "window.h"
#include "../ui/ui.h"
#include "../ui/splitter.h"
#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec2f.h>


AMN_NAMESPACE_OPEN_SCOPE

// View constructor
//----------------------------------------------------------------------------
Popup::Popup(View* parent, const pxr::GfVec2f& min, const pxr::GfVec2f& max) :
  View(parent, min, max)
{

}

Popup::Popup(View* parent, int x, int y, int w, int h):
  View(parent, x, y, w, h)
{
}

Popup::~Popup()
{
}

void Popup::MouseButton(int x, int y)
{
  std::cout << x << "," << y << std::endl;
}

AMN_NAMESPACE_CLOSE_SCOPE