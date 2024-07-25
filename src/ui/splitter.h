#ifndef JVR_UI_SPLITTER_H
#define JVR_UI_SPLITTER_H

#include <pxr/base/gf/vec2i.h>
#include <pxr/base/gf/vec3f.h>

#include "../common.h"
#include "../ui/ui.h"
#include "../ui/utils.h"


JVR_NAMESPACE_OPEN_SCOPE

#define SPLITTER_THICKNESS 2.0

class Window;
class SplitterUI : public BaseUI
{
public:
  enum Border
  {
    TOP = 1,
    RIGHT = 2,
    BOTTOM = 4,
    LEFT = 8
  };
 
  SplitterUI() = delete;
  SplitterUI(Window* window);
  ~SplitterUI();

  const int* GetPixels(){return _pixels;};
  int GetWidth() override {return _width;};
  int GetHeight() override {return _height;};
  void BuildMap(int width, int height);
  View* GetViewByIndex(int index);
  int Pick(int x, int y);
  View* GetHovered() { return _hovered; };
  void SetHorizontalCursor(){_cursor = ImGuiMouseCursor_ResizeEW;};
  void SetVerticalCursor(){_cursor = ImGuiMouseCursor_ResizeNS;};
  void SetDefaultCursor(){_cursor = ImGuiMouseCursor_Arrow;};

  Window* GetWindow() override;
  void Resize(int width, int height);
  bool Draw() override;
  
private:
  int*                    _pixels;
  unsigned                _width;
  unsigned                _height;
  unsigned                _lastX;
  unsigned                _lastY;
  bool                    _drag;
  bool                    _valid;
  int                     _cursor;
  View*                   _hovered;
  Window*                 _window;
  static ImGuiWindowFlags _flags;
};

JVR_NAMESPACE_CLOSE_SCOPE

#endif // JVR_UI_SPLITTER_H
