#ifndef JVR_APPLICATION_POPUP_H
#define JVR_APPLICATION_POPUP_H

#include <pxr/base/vt/array.h>
#include <pxr/base/vt/value.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/timeCode.h>

#include "../common.h"
#include "../ui/ui.h"


PXR_NAMESPACE_OPEN_SCOPE

class PopupUI : public BaseUI
{
public:
  PopupUI(int x, int y, int width, int height);
  ~PopupUI() override;

  void MouseButton(int button, int action, int mods) override;
  void MouseMove(int x, int y) override;
  void Keyboard(int key, int scancode, int action, int mods) override;
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
  int GetHeight() override { return _height; };

  // is popup done
  bool IsDone() { return _done; };
  bool IsSync() { return _sync; };

protected:
  bool                        _done;
  bool                        _sync;
  int                         _x;
  int                         _y;
  int                         _width;
  int                         _height;
  static ImGuiWindowFlags     _flags;
};

class ColorPopupUI : public PopupUI
{
public:
  ColorPopupUI(int  x, int y, int width, int height, 
    const pxr::UsdAttribute& attribute, const pxr::UsdTimeCode& timeCode);
  ~ColorPopupUI() override;

  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
private:
  pxr::UsdAttribute _attribute;
  pxr::UsdTimeCode  _time;
  pxr::GfVec3f      _color;
  pxr::GfVec3f      _original;
  bool              _isArray;
};

#define NODE_FILTER_SIZE 64
class NodePopupUI : public PopupUI
{
public:

  NodePopupUI(int x, int y, int width, int height);
  ~NodePopupUI() override;

  void BuildNodeList();

  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
  void Keyboard(int key, int scancode, int action, int mods) override;
  void Input(int key) override;
private:
  void _FilterNodes();
  std::vector<std::string> _nodes;
  std::vector<std::string> _filteredNodes;
  char                     _filter[NODE_FILTER_SIZE];
  short                    _p; // cursor position
  short                    _i; // index in filterded nodes
};

PXR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_POPUP_H