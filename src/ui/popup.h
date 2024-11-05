#ifndef JVR_APPLICATION_POPUP_H
#define JVR_APPLICATION_POPUP_H

#include <pxr/base/vt/array.h>
#include <pxr/base/vt/value.h>
#include <pxr/base/gf/vec3f.h>
#include <pxr/usd/usd/attribute.h>
#include <pxr/usd/usd/timeCode.h>

#include "../common.h"
#include "../ui/ui.h"


JVR_NAMESPACE_OPEN_SCOPE


class PopupUI : public BaseUI
{
public:
  inline static const float Sensitivity = 4.f;
  PopupUI(const std::string &name, int x, int y, int width, int height);
  ~PopupUI() override;

  void MouseButton(int button, int action, int mods) override;
  void MouseMove(int x, int y) override;
  void Keyboard(int key, int scancode, int action, int mods) override;
  bool Draw() override;
  virtual bool Terminate() { return true; };

  // mouse position in the view space
  // (0, 0) left top corner
  // (width, height) right bottom corner
  void GetRelativeMousePosition(const float inX, const float inY, 
    float& outX, float& outY) override;

  // get the (x,y) position in window space (left top corner)
  GfVec2f GetPosition() override { return GfVec2f(_x, _y); };;

  // get the x position in window space (x-coordinate of left top corner)
  int GetX() override {return _x;};

  // get the y position in window space (y-coordinate of left top corner)
  int GetY() override { return _y; };

  // get the width of the parent view
  int GetWidth() override { return _width; };
  
  // get the height of the parent view
  int GetHeight() override { return _height; };

  // is popup done, if true popup will be destroyed
  bool IsDone() { return _done; };

  // is popup sync, if true it will update background window
  bool IsSync() { return _sync; };

  // is popup dimmer, if true it will dim background window
  bool IsDimmer() { return _dimmer; };

  // is popup cancel
  bool IsCancel() { return _cancel; };

protected:

  void _DrawTitle();

  std::string                 _name;
  bool                        _done;
  bool                        _cancel;
  bool                        _sync;
  bool                        _dimmer;
  bool                        _drag;
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
    const UsdAttribute& attribute, const UsdTimeCode& timeCode);
  ~ColorPopupUI() override;

  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
  bool Terminate() override;
private:
  UsdAttribute _attribute;
  UsdTimeCode  _time;
  GfVec3f      _color;
  GfVec3f      _original;
  bool         _isArray;
};


class ListPopupUI : public PopupUI
{
public:
  using Callback = std::function<void(const TfToken &token)>;
 
  ListPopupUI(const char* name, int x, int y, int width, int height, Callback callback);
  ~ListPopupUI() override;

  bool Draw() override;
  void MouseButton(int button, int action, int mods) override;
  void Keyboard(int key, int scancode, int action, int mods) override;
  void Input(int key) override;

private:
  void _BuildList();
  void _FilterItems();
  std::vector<std::string> _items;
  std::vector<std::string> _filteredItems;
  char                     _filter[256];
  short                    _p; // cursor position
  short                    _i; // index in filterded items

  Callback                 _callback;
};

class InputPopupUI : public PopupUI
{
public:
  using Callback = std::function<void(const TfToken& token)>;

  InputPopupUI(int x, int y, int width, int height, Callback callback);
  InputPopupUI(int x, int y, int width, int height, Callback callback, const std::string& value);
  ~InputPopupUI() override;

  void SetName(const std::string& name);
  bool Draw() override;

  bool Terminate() override;

private:
  char        _value[255];
  Callback    _callback;
};


JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_APPLICATION_POPUP_H