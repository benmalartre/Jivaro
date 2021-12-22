#ifndef JVR_APPLICATION_POPUP_H
#define JVR_APPLICATION_POPUP_H

#include "../common.h"
#include "../ui/ui.h"
#include <pxr/base/vt/array.h>
#include <pxr/base/vt/value.h>

PXR_NAMESPACE_OPEN_SCOPE

class PopupUI : public BaseUI
{
public:
  PopupUI(View* parent, int x, int y, int width, int height);
  ~PopupUI() override;

  void MouseButton(int button, int action, int mods) override;
  void MouseMove(int x, int y) override;
  bool Draw() override;

  // mouse position in the view space
  // (0, 0) left top corner
  // (width, height) right bottom corner
  void GetRelativeMousePosition(const float inX, const float inY, 
    float& outX, float& outY) override;

  // get the (x,y) position in window space (left top corner)
  pxr::GfVec2f GetPosition() override { return pxr::GfVec2f(_x, _y); };;

  // get the x position in window space (x-coordinate of left top corner)
  int GetX() override {return _x;};

  // get the y position in window space (y-coordinate of left top corner)
  int GetY() override { return _y; };

  // get the width of the parent view
  int GetWidth() override { return _width; };
  
  // get the height of the parent view
  int GetHeight() override { return _height; };;

private:
  int                         _x;
  int                         _y;
  int                         _width;
  int                         _height;
  static ImGuiWindowFlags     _flags;
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_POPUP_H