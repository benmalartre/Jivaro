#ifndef AMN_APPLICATION_POPUP_H
#define AMN_APPLICATION_POPUP_H

#include "../common.h"
#include "view.h"

AMN_NAMESPACE_OPEN_SCOPE

class Window;
class BaseUI;
class Popup : public View
{
public:
 
  Popup(View* parent, const pxr::GfVec2f& min, const pxr::GfVec2f& max);
  Popup(View* parent, int x, int y, int w, int h);
  ~Popup();

  void MouseButton(int x, int y);

private:

};

AMN_NAMESPACE_CLOSE_SCOPE

#endif // AMN_APPLICATION_VIEW_H